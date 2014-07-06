BINDIR?=	/usr/local/bin

PROG=		z80asm
NOMAN=

SRCS=		z80aglb.c z80amain.c z80anum.c z80aopc.c \
		z80aout.c z80apfun.c z80arfun.c z80atab.c

CFLAGS+=	-g
CFLAGS+=	-Werror -Wextra -std=c99 -Wcast-qual -Wformat
#CFLAGS+=	-Wmissing-declarations -pedantic-errors

#WARNINGS=	yes

.include <bsd.prog.mk>
