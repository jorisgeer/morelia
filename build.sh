#!/bin/bash

# build.sh - build script for morelia

set -f
set -eu

compiler=gcc
analyzer=clang

lang=python

copt='-O1 -march=native'

# -fanalyzer
cdiag='-Wall -Wextra -Wshadow -Wundef -Wno-unused -Wno-padded -Wno-char-subscripts -Werror'
cxdiag='-Wstack-usage=8192'
ctdiag='-Wstack-usage=32768'

cfmt='-fmax-errors=60 -fno-diagnostics-show-caret -fno-diagnostics-show-option -fno-diagnostics-color -fcompare-debug-second'

#cdbg='-g1 -fsanitize=address,undefined,signed-integer-overflow,bounds -fno-sanitize-recover=all -ftrapv -fstack-protector'
cdbg='-g1 -fno-stack-protector -fno-wrapv -fcf-protection=none -fno-stack-clash-protection -fno-asynchronous-unwind-tables -U_FORTIFY_SOURCE'
#UBSAN_OPTIONS=print_stacktrace=1

#cxtra='-std=c11 -funsigned-char -static -specs /home/joris/lib/musl-gcc.specs'
cxtra='-std=c11 -funsigned-char'

cflags="$copt $cdiag $cfmt $cdbg $cxtra"

lflags="-O1 -fuse-ld=gold $cdbg $cxtra -lm"
# --gc-sections --no-ld-generated-unwind-info -z stack-size=1234

anacdia='--analyze --analyzer-output text -Weverything -Wno-implicit-int-conversion -Wunused -Wno-sign-conversion -Wno-padded -Wno-char-subscripts -Werror=format'

anacfmt='-fno-caret-diagnostics -fno-color-diagnostics -fno-diagnostics-show-option -fno-diagnostics-fixit-info -fno-diagnostics-show-note-include-stack -std=c11 -funsigned-char'

anacflags="$anacdia $anacfmt"

asmcflags='-fverbose-asm -frandon-seed=0'

# egrep -o -h -E 'enum [A-Za-z]+ {' *.h | sort

dryrun=0
always=0
ana=0
map=0
dolgen=0
dosgen=0
vrb=0
valgrind=0
target=''

usage()
{
  echo 'usage: build [options] [target]'
  echo
  echo '-a  - analyze'
  echo '-g  - run generators'
  echo '-n  - dryrun'
  echo '-m  - create map file'
  echo '-u  - unconditional'
  echo '-v  - verbose'
  echo '-vg - valgrind'
  echo '-h  - help'
  echo '-L  - build license'
  echo
  echo 'target - only build given target'
}

error()
{
  echo $1
  exit 1
}

verbose()
{
  if [ $vrb -eq 0 ]; then
    echo $1
  else
    echo $2
  fi
}

cc()
{
  local src
  local tgt
  local dep
  local mtime
  local newer

  tgt="$1"
  src="$2"

  newer=$always

  shift

  if [ -f "$tgt" ]; then
    for dep in "$@"; do
      [ -f $dep ] || error "$dep does not exist"
      if [ "$dep" -nt "$tgt" ]; then
        newer=1
      fi
    done
  else
    newer=1
  fi

  if [ $newer -eq 1 ]; then
    verbose "$compiler -c $src" "$compiler -c $cflags $src"
    if [ $dryrun -eq 0 ]; then
      $compiler -c $cflags $cxdiag $src
    fi
  fi
  if [ "$tgt" = "$target" ]; then
    exit 0
  fi
}

tc()
{
  local src
  local tgt
  local def
  local dep
  local mtime
  local newer

  def="$1"
  tgt="$2"
  src="$3"

  newer=$always

  shift 2

  if [ -f "$tgt" ]; then
    for dep in "$@"; do
      if [ -f "$dep" -a "$dep" -nt "$tgt" ]; then
        newer=1
      fi
    done
  else
    newer=1
  fi

  if [ $newer -eq 1 ]; then
    echo "$def $compiler -c $src"
    if [ $dryrun -eq 0 ]; then
      $compiler -c $cflags $ctdiag $src
    fi
  fi
  if [ "$tgt" = "$target" ]; then
    exit 0
  fi
}

ld()
{
  local tgt
  local dep
  local mtime
  local newer

  tgt="$1"

  if [ $ana -eq 1 ]; then
    return 0
  fi

  newer=$always

  shift

  if [ $newer -eq 0 -a -f "$tgt" ]; then
    for dep in "$@"; do
      if [ "$dep" = "-lm" ]; then
          continue;
      fi
      if [ -f "$dep" -a "$dep" -nt "$tgt" ]; then
        newer=1
      fi
    done
  else
    newer=1
  fi

  if [ $newer -eq 1 ]; then
    echo "ld -o $tgt $@"
    if [ $dryrun -eq 0 ]; then
      $compiler -o $tgt $lflags $@
      if [ $map -eq 1 ]; then
        nm --line-numbers -S -r --size-sort $tgt > $tgt.map
      fi
    fi
  fi
  if [ "$tgt" = "$target" ]; then
    exit 0
  fi
}

run()
{
  local tgt
  local dep
  local alw
  local cmd
  local mtime
  local newer
  local args

  tgt="$1"
  dep="$2"
  alw="$3"
  cmd="$4"
  args="$5"
  uncond=""

  newer=$alw
  if [ $alw -eq 1 ]; then
    uncond="-u"
  fi

  if [ $ana -eq 1 ]; then
    return 0
  fi

  if [ $newer -eq 0 -a -f "$tgt" ]; then
    if [ -f "$dep" -a "$dep" -nt "$tgt" ]; then
      echo "$dep > $tgt"
      newer=1
    fi

    if [ -f "$cmd" -a "$cmd" -nt "$tgt" ]; then
      echo "  $cmd > $tgt"
      newer=1
    fi
  else
    newer=1
  fi

  if [ $newer -eq 1 ]; then
    echo "run $cmd $uncond $args"
    if [ $dryrun -eq 0 ]; then
      $cmd $uncond $args
    fi
  fi
  if [ "$tgt" = "$target" ]; then
    exit 0
  fi
}

mklic()
{
   echo 'objcopy --add-section license=License.txt'
}

while [ $# -ge 1 ]; do
  case "$1" in
  '-a') ana=1
#    always=1
    compiler=$analyzer
    cflags=$anacflags
    cflags_t=$anacflags ;;
  '-g') dolgen=1; dosgen=1 ;;
  '-h'|'-?') usage ;;
  '-l') lang=$2; shift ;;
  '-L') mklic ;;
  '-n') dryrun=1 ;;
  '-m') map=1 ;;
  '-u') always=1 ;;
  '-v') vrb=1 ;;
  '-vg') valgrind=1; cflags="$cflags -DVALGRIND" ;;
  *) target="$1" ;;
  esac
  shift
done

cc morelia.o morelia.c base.h

ld morelia   morelia.o

# ~/bin/valgrind -s --redzone-size=4096 --exit-on-first-error=yes --error-exitcode=1 --track-fds=yes --leak-check=no --partial-loads-ok=no --track-origins=yes --malloc-fill=55 $*
