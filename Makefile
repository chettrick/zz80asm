PREFIX?=	/usr/local
BINDIR?=	${PREFIX}/bin
MANDIR?=	${PREFIX}/man/man

PROG=		zz80asm

SRCS=		zz80asm.c num.c out.c pfun.c rfun.c tab.c

MAN=		zz80asm.1

CLFAGS+=	-g
CFLAGS+=	-O2 -pipe
#CFLAGS+=	-Wall -Werror -Wextra -Wformat=2 -Wstrict-prototypes
CFLAGS+=	-Wall -Werror -Wextra -Wformat=2
CFLAGS+=	-Wmissing-declarations -pedantic -std=c99 -Wcast-qual
CFLAGS+=	-Wpointer-arith -Wuninitialized -Wmissing-prototypes
CFLAGS+=	-Wsign-compare -Wshadow -Wdeclaration-after-statement
CFLAGS+=	-Wfloat-equal -Wcast-align -Wundef -Wstrict-aliasing=2

OBJS+=		${SRCS:.c=.o}

all: ${PROG} README.md

${PROG}: ${OBJS}
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ ${OBJS}

README.md: ${MAN}
	mandoc -T markdown ${MAN} > $@

uninstall:
	rm ${BINDIR}/${PROG}
	rm ${MANDIR}1/${PROG}.1

clean:
	rm -f a.out [Ee]rrs mklog *.core y.tab.h ${PROG} *.o *.d

.PHONY: all uninstall clean
