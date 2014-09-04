PREFIX?=	/usr/local
BINDIR?=	${PREFIX}/bin
MANDIR?=	${PREFIX}/man/man

PROG=		zz80asm

SRCS=		zz80asm.c num.c out.c pfun.c rfun.c tab.c

CFLAGS+=	-g
CFLAGS+=	-Wall -Werror -Wextra -std=c99 -Wcast-qual -Wformat=2
CFLAGS+=	-Wmissing-declarations -pedantic-errors -Wundef
CFLAGS+=	-Wpointer-arith -Wuninitialized -Wmissing-prototypes
CFLAGS+=	-Wsign-compare -Wshadow -Wdeclaration-after-statement
CFLAGS+=	-Wfloat-equal -Wcast-align -Wstrict-aliasing=2

all: readme

uninstall:
	rm ${BINDIR}/${PROG}
	rm ${MANDIR}1/${PROG}.1

readme: ${PROG}.1
	mandoc ${.CURDIR}/${PROG}.1 | col -bx > ${.CURDIR}/README

.PHONY: all readme uninstall
.include <bsd.prog.mk>
