#!/bin/sh

# build.sh - build script for Morelia

set -f
set -eu
# set -x

compiler=gcc
analyzer=clang

copt='-O1 -march=native'
copt_t='-O1 -march=native'

# -fanalyzer
cdiag='-Wall -Wextra -Wshadow -Wundef -Wno-unused -Wno-padded -Wno-char-subscripts -Werror -Wstack-usage=65536'

cfmt='-fmax-errors=20 -fno-diagnostics-show-caret -fno-diagnostics-color'

#cdbg='-g1 -fsanitize=undefined,signed-integer-overflow,bounds -fno-sanitize-recover=all -ftrapv -fstack-protector'
cdbg='-g1 -fno-stack-protector -fno-wrapv -fcf-protection=none -fno-stack-clash-protection -fno-asynchronous-unwind-tables'
# UBSAN_OPTIONS=print_stacktrace=1

#cxtra='-std=c11 -funsigned-char -static -specs /home/oem/lib/musl-gcc.specs'
cxtra='-std=c11 -funsigned-char -fno-common'

cflags="$copt $cdiag $cfmt $cdbg"
cflags_t="$copt_t $cdiag $cfmt $cdbg"

lflags="-O1 -fuse-ld=gold $cdbg"

anacdia='--analyze --analyzer-output text -Weverything -Wno-implicit-int-conversion -Wunused -Wno-sign-conversion -Wno-padded -Wno-char-subscripts -Werror=format'

anacfmt='-fno-caret-diagnostics -fno-color-diagnostics -fno-diagnostics-show-option -fno-diagnostics-fixit-info -fno-diagnostics-show-note-include-stack -std=c11 -funsigned-char'

anacflags="$anacdia $anacfmt"

asmcflags='-fverbose-asm -frandon-seed=0'

# egrep -o -h -E 'enum [A-Za-z]+ {' *.h | sort

lang=minipython

dryrun=0
always=0
ana=0
map=0
dogen=0
vrb=0
target=''

usage()
{
  echo 'usage: build [-nuh] [target]'
  echo
  echo '-a - analyze'
  echo '-g - run generators'
  echo '-n - dryrun'
  echo '-m - create map file'
  echo '-u - unconditional'
  echo '-v - verbose'
  echo '-h - help'
  echo '-l - select language'
  echo '-L - build license'
  echo
  echo 'target - only build given target'
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

#  mtime=$(stat -c '%Y' "$src")
#  echo $mtime

  newer=$always

  shift

  if [ -f "$tgt" ]; then
    for dep in "$@"; do
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
      $compiler -c $cflags $src
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
    echo "$compiler -c $src"
    if [ $dryrun -eq 0 ]; then
      $compiler -c -D$def $cflags_t $src
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
        nm -S -r --size-sort $tgt > $tgt.map
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
#    dogen=1
    compiler=$analyzer
    cflags=$anacflags
    cflags_t=$anacflags ;;
  '-g') dogen=1 ;;
  '-h'|'-?') usage ;;
  '-l') lang=$2; shift ;;
  '-L') mklic ;;
  '-n') dryrun=1 ;;
  '-m') map=1 ;;
  '-u') always=1 ;;
  '-v') vrb=1 ;;
  *) target="$1" ;;
  esac
  shift
done

cc base.o base.c base.h
cc os.o   os.c base.h mem.h msg.h fmt.h os.h
cc mem.o  mem.c base.h os.h mem.h msg.h
cc fmt.o  fmt.c base.h fmt.h chr.h os.h
cc chr.o  chr.c base.h
cc math.o math.c base.h math.h
cc msg.o  msg.c base.h fmt.h msg.h mem.h os.h util.h
cc dia.o  dia.c base.h dia.h msg.h
cc util.o util.c base.h mem.h os.h fmt.h msg.h tim.h util.h
cc tim.o  tim.c base.h mem.h fmt.h msg.h tim.h

if [ $dogen -eq 1 ]; then
  tc Genlex genlex.o genlex.c base.h chr.h os.h msg.h math.h mem.h util.h tim.h hash.h
  ld        genlex   genlex.o base.o chr.o os.o msg.o math.o mem.o util.o tim.o fmt.o

  run tok.h    $lang.lex $always genlex "-1 $lang.lex lextb1.i lexdef.h tok.h"
  run lexdef.h $lang.lex 0 genlex "-1 $lang.lex lextb1.i lexdef.h tok.h"
  run lextb1.i $lang.lex 0 genlex "-1 $lang.lex lextb1.i lexdef.h tok.h"

  run lextb2.i $lang.lex $always genlex "-2 $lang.lex lextb2.i"

  tc Gensyn gensyn.o gensyn.c base.h chr.h os.h fmt.h msg.h mem.h util.h tim.h syn.h tok.h
  ld        gensyn   gensyn.o base.o chr.o os.o fmt.o msg.o mem.o util.o tim.o

  run syntab.i $lang.syn $always gensyn "$lang.syn syntab.i syndef.h"
  run syndef.h $lang.syn $always gensyn "$lang.syn syntab.i syndef.h"

  tc Genir genir.o genir.c base.h os.h fmt.h msg.h mem.h util.h tim.h irtyp.h
  ld       genir   genir.o base.o os.o fmt.o msg.o mem.o util.o tim.o

  run irdef.h irtyp.h $always genir "irdef.h"
fi

if [ $ana -eq 0 ]; then
  cc lex.o lex.c lextb1.i lextb2.i tok.h lexdef.h base.h chr.h dia.h mem.h msg.h fmt.h os.h lexsyn.h hash.h
fi

cc syn.o syn.c base.h chr.h mem.h msg.h fmt.h syn.h lexsyn.h syndef.h syntab.i astyp.h synast.h tok.h

cc ast.o ast.c base.h chr.h mem.h msg.h fmt.h lexsyn.h astyp.h synast.h syndef.h ast.h

cc vm.o vm.c base.h irtyp.h irdef.h
cc vmrun.o vmrun.c base.h mem.h os.h msg.h fmt.h util.h tim.h irtyp.h irdef.h

ld vmrun vmrun.o base.o mem.o os.o fmt.o msg.o util.o tim.o vm.o

cc morelia.o morelia.c base.h dia.h mem.h os.h msg.h lexsyn.h syndef.h synast.h astyp.h util.h

ld morelia morelia.o base.o chr.o dia.o fmt.o lex.o math.o mem.o msg.o os.o syn.o ast.o util.o tim.o -lm
