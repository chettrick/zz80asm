BINDIR?=	/usr/local/bin
MANDIR?=	/usr/local/man/man

PROG=		zz80asm

SRCS=		zz80asm.c num.c out.c pfun.c rfun.c tab.c

CFLAGS+=	-g
CFLAGS+=	-Wextra -std=c99 -Wcast-qual -Wformat
CFLAGS+=	-Wmissing-declarations -pedantic-errors

WARNINGS=	yes

.include <bsd.prog.mk>
