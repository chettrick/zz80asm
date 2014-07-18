BINDIR?=	/usr/local/bin
MANDIR?=	/usr/local/man/man

PROG=		zz80asm

SRCS=		zz80asm.c num.c out.c pfun.c rfun.c tab.c

CFLAGS+=	-g
CFLAGS+=	-Wextra -std=c99 -Wcast-qual -Wformat
CFLAGS+=	-Wmissing-declarations -pedantic-errors

WARNINGS=	yes

all: readme

readme: ${PROG}.1
	mandoc ${.CURDIR}/${PROG}.1 > ${.CURDIR}/README

.PHONY: all readme
.include <bsd.prog.mk>
