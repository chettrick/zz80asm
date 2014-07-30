/*
 *	Z80 - Assembler
 *	Copyright (C) 1987-2014 by Udo Munk
 */

/*
 *	main module, handles the options and runs 2 passes over the sources
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "zz80asm.h"

static void usage(void) __attribute__((__noreturn__));
static void pass1(void);
static void pass2(void);
static int p1_line(void);
static int p2_line(void);
static void open_o_files(const char * const);
static void get_fn(char * const, char * const, const char * const);
static char *get_label(char *, char *);
static char *get_opcode(char *, char *);
static char *get_arg(char *, char *);

extern char *__progname;

FILE		*srcfp;		/* file pointer for current source */
FILE		*objfp;		/* file pointer for object code */
FILE		*lstfp;		/* file pointer for listing */
FILE		*errfp;		/* file pointer for error output */

char		*srcfn;		/* filename of current processed source file */
char		line[LINE_MAX];	/* buffer for one line source */
char		tmp[LINE_MAX];	/* temporary buffer */
char		label[SYMSIZE + 1];	/* buffer for label */
char		operand[LINE_MAX];	/* buffer for operand */

uint8_t		list_flag;	/* flag for option -l */
uint8_t		ver_flag;	/* flag for option -v */
uint8_t		dump_flag;	/* flag for option -x */
int		pc;		/* program counter */
uint8_t		pass;		/* processed pass */
int		iflevel;	/* IF nesting level */
int		gencode;	/* flag for conditional object code */
int		errors;		/* error counter */
uint8_t		sd_flag;	/* list flag for PSEUDO opcodes */
				/* = 0: addr from <val>, data from <ops> */
				/* = 1: addr from <sd_val>, data from <ops> */
				/* = 2: no addr, data from <ops> */
				/* = 3: addr from <sd_val>, no data */
				/* = 4: suppress whole line */
uint8_t		out_form;	/* format of object file */

size_t		c_line;		/* current line no. in current source */
size_t		s_line;		/* line no. counter for listing */

static char *errmsg[] = {		/* error messages for fatal() */
	"out of memory: %s",		/* 0 */
	"assembly halted",		/* 1 */
	"can't open file %s",		/* 2 */
	"internal error: %s"		/* 3 */
};

static char	*infiles[MAXFN];	/* source filenames */
static char	 objfn[PATH_MAX];	/* object filename */
static char	 lstfn[PATH_MAX];	/* listing filename */
static char	 opcode[LINE_MAX];	/* buffer for opcode */

int main(int argc, char *argv[])
{
	char *t;
	int i, ch;
	size_t len;

	int sym_flag = 0;		/* flag for option -s */

	/* program defaults */
	gencode = 1;
	out_form = OUTHEX;		/* default object format */
	dump_flag = 1;
	ver_flag = 0;
	iflevel = 0;			/* IF nesting level */
	errfp = stdout;

	while ((ch = getopt(argc, argv, "d:f:l::o:s:vx")) != -1) {
		switch (ch) {
		case 'd':
			if (optarg == '\0') {
				usage();
				/* NOTREACHED */
			}
			t = tmp;
			*t = islower(*optarg) ? (char)toupper(*optarg) :
			    *optarg;
			if (put_sym(tmp, 0) != 0)
				fatal(F_OUTMEM, "symbols");
			break;
		case 'f':
			switch (*optarg) {
			case 'b':
				out_form = OUTBIN;
				break;
			case 'm':
				out_form = OUTMOS;
				break;
			case 'h':
				out_form = OUTHEX;
				break;
			default:
				usage();
				/* NOTREACHED */
			}
			break;
		case 'l':
			if (optarg != '\0')
				get_fn(lstfn, optarg, LSTEXT);
			list_flag = 1;
			break;
		case 'o':
			if (optarg == '\0') {
				usage();
				/* NOTREACHED */
			}
			if (out_form == OUTHEX)
				get_fn(objfn, optarg, OBJEXTHEX);
			else
				get_fn(objfn, optarg, OBJEXTBIN);
			break;
		case 's':
			switch (*optarg) {
			case 'a':
				sym_flag = 'a';
				break;
			case 'n':
				sym_flag = 'n';
				break;
			default:
				usage();
				/* NOTREACHED */
			}
			break;
		case 'v':
			ver_flag = 1;
			break;
		case 'x':
			dump_flag = 0;		/* Default is on; turn off. */
			break;
		default :
			usage();
			/* NOTREACHED */
		}
	}
	argc -= optind;
	argv += optind;

	for (i = 0; (argc--) && (i < MAXFN); i++) {
		if ((infiles[i] = malloc(PATH_MAX)) == NULL) {
			while (--i >= 0)
				free(infiles[i]);
			fatal(F_OUTMEM, "filenames");
		}
		get_fn(infiles[i], *argv++, SRCEXT);
	}
	if (i == 0) {
		fprintf(errfp, "%s\n", "no input file");
		usage();
		/* NOTREACHED */
	}

	if (ver_flag)
		fprintf(stdout, "Z80 - Assembler Release %s, %s\n", REL, COPYR);
	pass1();
	pass2();
	while (i > 0)
		free(infiles[i--]);
	if (list_flag) {
		switch (sym_flag) {
		case 'a':	/* symbol table sorted by address */
			len = copy_sym();
			a_sort_sym(len);
			lst_sort_sym(len);
			break;
		case 'n':	/* symbol table sorted by name */
			len = copy_sym();
			n_sort_sym(len);
			lst_sort_sym(len);
			break;
		default:	/* no symbol table */
			break;
		}
		fclose(lstfp);
	}
	return(errors);
}

/*
 *	Pass 1:
 *	  - process all source files
 */
void pass1(void)
{
	int fi;

	pass = 1;
	pc = 0;
	fi = 0;
	if (ver_flag)
		fprintf(stdout, "%s\n", "Pass 1");
	open_o_files(infiles[fi]);
	while (infiles[fi] != NULL) {
		if (ver_flag)
			fprintf(stdout, "   Read    %s\n", infiles[fi]);
		p1_file(infiles[fi]);
		fi++;
	}
	if (errors) {
		fclose(objfp);
		unlink(objfn);
		fprintf(errfp, "%d error(s)\n", errors);
		fatal(F_HALT, NULL);
	}
}

/*
 *	Pass 1:
 *	  - process one source file
 *
 *	Input: name of source file
 */
void p1_file(char * const fn)
{
	c_line = 0;
	srcfn = fn;
	if ((srcfp = fopen(fn, "r")) == NULL)
		fatal(F_FOPEN, fn);
	while (p1_line())
		;
	fclose(srcfp);
	if (iflevel)
		asmerr(E_MISEIF);
}

/*
 *	Pass 1:
 *	  - process one line of source
 *
 *	Output: 1 line processed
 *		0 EOF
 */
int p1_line(void)
{
	char *p;
	int i;
	struct opc *op;

	if ((p = fgets(line, LINE_MAX, srcfp)) == NULL)
		return(0);
	c_line++;
	p = get_label(label, p);
	p = get_opcode(opcode, p);
	p = get_arg(operand, p);
	if (strcmp(opcode, ENDFILE) == 0)
		return(0);
	if (*opcode) {
		if ((op = search_op(opcode)) != NULL) {
			i = (*op->op_fun)(op->op_c1, op->op_c2);
			if (gencode)
				pc += i;
		} else
			asmerr(E_ILLOPC);
	} else
		if (*label)
			put_label();
	return(1);
}

/*
 *	Pass 2:
 *	  - process all source files
 */
void pass2(void)
{
	int fi;

	pass = 2;
	pc = 0;
	fi = 0;
	if (ver_flag)
		fprintf(stdout, "%s\n", "Pass 2");
	obj_header();
	while (infiles[fi] != NULL) {
		if (ver_flag)
			fprintf(stdout, "   Read    %s\n", infiles[fi]);
		p2_file(infiles[fi]);
		fi++;
	}
	obj_end();
	fclose(objfp);
	if (ver_flag)
		fprintf(stdout, "%d error(s)\n", errors);
}

/*
 *	Pass 2:
 *	  - process one source file
 *
 *	Input: name of source file
 */
void p2_file(char * const fn)
{
	c_line = 0;
	srcfn = fn;
	if ((srcfp = fopen(fn, "r")) == NULL)
		fatal(F_FOPEN, fn);
	while (p2_line())
		;
	fclose(srcfp);
}

/*
 *	Pass 2:
 *	  - process one line of source
 *
 *	Output: 1 line processed
 *		0 EOF
 */
int p2_line(void)
{
	char *p;
	int op_count;
	struct opc *op;

	if ((p = fgets(line, LINE_MAX, srcfp)) == NULL)
		return(0);
	c_line++;
	s_line++;
	p = get_label(label, p);
	p = get_opcode(opcode, p);
	p = get_arg(operand, p);
	if (strcmp(opcode, ENDFILE) == 0) {
		lst_line(pc, 0);
		return(0);
	}
	if (*opcode) {
		op = search_op(opcode);
		op_count = (*op->op_fun)(op->op_c1, op->op_c2);
		if (gencode) {
			lst_line(pc, op_count);
			obj_writeb((size_t)op_count);
			pc += op_count;
		} else {
			sd_flag = 2;
			lst_line(0, 0);
		}
	} else {
		sd_flag = 2;
		lst_line(0, 0);
	}
	return(1);
}

/*
 *	open output files:
 *	input is filename of source file
 *	list and object filenames are built from source filename if
 *	not given by options
 */
void open_o_files(const char * const source)
{
	char *p;

	if (*objfn == '\0') {
		strlcpy(objfn, source, sizeof(objfn));
		if ((p = strrchr(objfn, '.')) != NULL) {
			if (out_form == OUTHEX)
				strlcpy(p, OBJEXTHEX, sizeof(objfn));
			else
				strlcpy(p, OBJEXTBIN, sizeof(objfn));
		} else {
			if (out_form == OUTHEX)
				strlcat(objfn, OBJEXTHEX, sizeof(objfn));
			else
				strlcat(objfn, OBJEXTBIN, sizeof(objfn));
		}
	}
	if ((objfp = fopen(objfn, "w")) == NULL)
		fatal(F_FOPEN, objfn);

	if (list_flag) {
		if (*lstfn == '\0') {
			strlcpy(lstfn, source, sizeof(lstfn));
			if ((p = strrchr(lstfn, '.')) != NULL)
				strlcpy(p, LSTEXT, sizeof(lstfn));
			else
				strlcat(lstfn, LSTEXT, sizeof(lstfn));
		}
		if ((lstfp = fopen(lstfn, "w")) == NULL)
			fatal(F_FOPEN, lstfn);
		errfp = lstfp;
	}
}

/*
 *	create a filename in "dest" from "src" and "ext"
 */
void get_fn(char * const dest, char * const src, const char * const ext)
{
	int i;
	char *sp, *dp;

	i = 0;
	sp = src;
	dp = dest;
	while ((i++ < PATH_MAX) && (*sp != '\0'))
		*dp++ = *sp++;
	*dp = '\0';
	if ((strrchr(dest, '.') == NULL) &&
	    (strlen(dest) <= (PATH_MAX - strlen(ext))))
		strlcat(dest, ext, sizeof(dest));
}

/*
 *	get labels, constants and variables from source line
 *	convert names to upper case and truncate length of name
 */
char *get_label(char *s, char *l)
{
	int i;

	i = 0;
	if (*l == LINCOM)
		goto comment;
	while (!isspace(*l) && *l != COMMENT && *l != LABSEP &&
	    i < SYMSIZE) {
		*s++ = islower(*l) ? (char)toupper(*l++) : *l++;
		i++;
	}
comment:
	*s = '\0';
	return(l);
}

/*
 *	get opcode into s from source line l
 *	converts to upper case
 */
char *get_opcode(char *s, char *l)
{
	if (*l == LINCOM)
		goto comment;
	while (!isspace(*l) && *l != COMMENT && *l != LABSEP)
		l++;
	if (*l == LABSEP)
		l++;
	while (*l == ' ' || *l == '\t')
		l++;
	while (!isspace(*l) && *l != COMMENT)
		*s++ = islower(*l) ? (char)toupper(*l++) : *l++;
comment:
	*s = '\0';
	return(l);
}

/*
 *	get operand into s from source line l
 *	converts to upper case
 *	strings inside of 's are copied without changes
 */
char *get_arg(char *s, char *l)
{
	if (*l == LINCOM)
		goto comment;
	while (*l == ' ' || *l == '\t')
		l++;
	while (*l != '\n' && *l != COMMENT) {
		if (isspace(*l)) {
			l++;
			continue;
		}
		if (*l != STRSEP) {
			*s++ = islower(*l) ? (char)toupper(*l) : *l;
			l++;
			continue;
		}
		*s++ = *l++;
		if (*(s - 2) == 'F')	/* EX AF,AF' !!!!! */
			continue;
		while (*l != STRSEP) {
			if (*l == '\n' || *l == '\0')
				goto comment;
			*s++ = *l++;
		}
		*s++ = *l++;
	}
comment:
	*s = '\0';
	return(l);
}

/*
 *	print error messages and abort
 */
void fatal(enum fatal_type ft, const char * const arg)
{
	fprintf(errfp, "%s %s\n", errmsg[ft], arg);
	exit(1);
}

/*
 *	error in options, print usage
 */
static void usage(void)
{
	(void)fprintf(stderr,
	    "usage: %s [-d symbol ...] [-f b|m|h] [-l[listfile]] [-o outfile] "
	    "[-s a|n|u] [-v] [-x] filename ...\n", __progname);
	exit(1);
}
