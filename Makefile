BINDIR?=	/usr/local/bin
MANDIR?=	/usr/local/man/man

PROG=		zz80asm

SRCS=		zz80asm.c num.c out.c pfun.c rfun.c tab.c

CFLAGS+=	-g
CFLAGS+=	-Wall -Werror -Wextra -std=c99 -Wcast-qual -Wformat
CFLAGS+=	-Wmissing-declarations -pedantic-errors -Wunused
CFLAGS+=	-Wpointer-arith -Wuninitialized -Wmissing-prototypes
CFLAGS+=	-Wsign-compare -Wshadow -Wdeclaration-after-statement

all: readme

readme: ${PROG}.1
	mandoc ${.CURDIR}/${PROG}.1 | col -bx > ${.CURDIR}/README

.PHONY: all readme
.include <bsd.prog.mk>
