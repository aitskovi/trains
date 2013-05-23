CC      = /usr/bin/gcc
AS		= /usr/bin/as
LD      = /usr/bin/gcc
CFLAGS  = -g -c -fPIC -fno-builtin -Wall -I. -I../include
# -g: include hooks for gdb
# -c: only compile
# -mcpu=arm920t: generate code for the 920t architecture
# -fpic: emit position-independent code
# -Wall: report all warnings