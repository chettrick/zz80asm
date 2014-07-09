BINDIR?=	/usr/local/bin

PROG=		zz80asm
NOMAN=

SRCS=		zz80asm.c glb.c num.c opc.c out.c pfun.c rfun.c tab.c

CFLAGS+=	-g
CFLAGS+=	-Werror -Wextra -std=c99 -Wcast-qual -Wformat
#CFLAGS+=	-Wmissing-declarations -pedantic-errors

#WARNINGS=	yes

.include <bsd.prog.mk>
