/*
 *	Z80 - Assembler
 *	Copyright (C) 1987-2014 by Udo Munk
 *
 *	History:
 *	17-SEP-1987 Development under Digital Research CP/M 2.2
 *	28-JUN-1988 Switched to Unix System V.3
 *	21-OCT-2006 changed to ANSI C for modern POSIX OS's
 *	03-FEB-2007 more ANSI C conformance and reduced compiler warnings
 *	18-MAR-2007 use default output file extension dependent on format
 *	04-OCT-2008 fixed comment bug, ';' string argument now working
 */

#ifndef _ZZ80ASM_H_
#define _ZZ80ASM_H_

/*
 *	OS dependant definitions
 */
#define LENFN		2048	/* max. filename length */
#define READA		"r"	/* file open mode read ascii */
#define WRITEA		"w"	/* file open mode write ascii */
#define WRITEB		"w"	/* file open mode write binary */

/*
 *	various constants
 */
#define REL		"1.6"
#define COPYR		"Copyright (C) 1987-2014 by Udo Munk"
#define SRCEXT		".asm"	/* filename extension source */
#define OBJEXTBIN	".bin"	/* filename extension object */
#define OBJEXTHEX	".hex"	/* filename extension hex */
#define LSTEXT		".lst"	/* filename extension listing */
#define OUTBIN		1	/* format of object: binary */
#define OUTMOS		2	/*		     Mostek binary */
#define OUTHEX		3	/*		     Intel hex */
#define OUTDEF		OUTMOS	/* default object format */
#define COMMENT		';'	/* inline comment character */
#define LINCOM		'*'	/* comment line if in column 1 */
#define LABSEP		':'	/* label separator */
#define STRSEP		'\''	/* string separator */
#define ENDFILE		"END"	/* end of source */
#define MAXFN		512	/* max. no. source files */
#define MAXLINE		128	/* max. line length source */
#define PLENGTH		65	/* default lines/page in listing */
#define SYMSIZE		8	/* max. symbol length */
#define INCNEST		5	/* max. INCLUDE nesting depth */
#define IFNEST		5	/* max IF.. nesting depth */
#define HASHSIZE	500	/* max. entries in symbol hash array */
#define OPCARRAY	256	/* size of object buffer */
#define SYMINC		100	/* start size of sorted symbol array */

/*
 *	structure opcode table
 */
struct opc {
	char *op_name;		/* opcode name */
	int (*op_fun) ();	/* function pointer code generation */
	int  op_c1;		/* first base opcode */
	int  op_c2;		/* second base opcode */
};

extern struct opc opctab[];

/*
 *	structure operand table
 */
struct ope {
	char *ope_name;		/* operand name */
	int ope_sym;		/* symbol value operand */
};

extern struct ope opetab[];

/*
 *	structure symbol table entries
 */
struct sym {
	char *sym_name;		/* symbol name */
	int  sym_val;		/* symbol value */
	struct sym *sym_next;	/* next entry */
};

/*
 *	structure nested INCLUDE's
 */
struct inc {
	unsigned int inc_line;	/* line counter for listing */
	char *inc_fn;		/* filename */
	FILE *inc_fp;		/* file pointer */
};

/*
 *	definition of operand symbols
 *	definitions for registers A, B, C, D, H, L and (HL)
 *	are defined as the bits used in operands and may not
 *	be changed!
 */
#define REGB		0	/* register B */
#define REGC		1	/* register C */
#define REGD		2	/* register D */
#define REGE		3	/* register E */
#define REGH		4	/* register H */
#define REGL		5	/* register L */
#define REGIHL		6	/* register indirect HL */
#define REGA		7	/* register A */
#define REGI		8	/* register I */
#define REGR		9	/* register R */
#define REGAF		10	/* register pair AF */
#define REGBC		11	/* register pair BC */
#define REGDE		12	/* register pair DE */
#define REGHL		13	/* register pair HL */
#define REGIX		14	/* register IX */
#define REGIY		15	/* register IY */
#define REGSP		16	/* register SP */
#define REGIBC		17	/* register indirect BC */
#define REGIDE		18	/* register indirect DE */
#define REGIIX		19	/* register indirect IX */
#define REGIIY		20	/* register indirect IY */
#define REGISP		21	/* register indirect SP */
#define FLGNC		30	/* flag no carry */
#define FLGNZ		31	/* flag not zero */
#define FLGZ		32	/* flag zero */
#define FLGM		33	/* flag minus */
#define FLGP		34	/* flag plus */
#define FLGPE		35	/* flag parity even */
#define FLGPO		36	/* flag parity odd */
#define NOOPERA		98	/* no operand */
#define NOREG		99	/* operand isn't register */

/*
 *	definitions of error numbers for error messages in listfile
 */
#define E_ILLOPC	0	/* illegal opcode */
#define E_ILLOPE	1	/* illegal operand */
#define E_MISOPE	2	/* missing operand */
#define E_MULSYM	3	/* multiple defined symbol */
#define E_UNDSYM	4	/* undefined symbol */
#define E_VALOUT	5	/* value out of bounds */
#define E_MISPAR	6	/* missing paren */
#define E_MISHYP	7	/* missing string separator */
#define E_MEMOVR	8	/* memory override (ORG) */
#define E_MISIFF	9	/* missing IF at ELSE or ENDIF */
#define E_IFNEST	10	/* to many IF's nested */
#define E_MISEIF	11	/* missing ENDIF */
#define E_INCNEST	12	/* to many INCLUDE's nested */

/*
 *	definition fatal errors
 */
#define F_OUTMEM	0	/* out of memory */
#define F_USAGE		1	/* usage: .... */
#define F_HALT		2	/* assembly halted */
#define F_FOPEN		3	/* can't open file */
#define F_INTERN	4	/* internal error */

/*
 *	global variables other than CPU specific tables
 */
char *infiles[MAXFN],		/* source filenames */
     objfn[LENFN + 1],		/* object filename */
     lstfn[LENFN + 1],		/* listing filename */
     *srcfn,			/* filename of current processed source file */
     line[MAXLINE],		/* buffer for one line source */
     tmp[MAXLINE],		/* temporary buffer */
     label[SYMSIZE+1],		/* buffer for label */
     opcode[MAXLINE],		/* buffer for opcode */
     operand[MAXLINE],		/* buffer for operand */
     title[MAXLINE];		/* buffer for title of source */

int  ops[OPCARRAY];		/* buffer for generated object code */

int  list_flag,			/* flag for option -l */
     sym_flag,			/* flag for option -s */
     ver_flag,			/* flag for option -v */
     dump_flag,			/* flag for option -x */
     pc,			/* program counter */
     pass,			/* processed pass */
     iflevel,			/* IF nesting level */
     gencode,			/* flag for conditional object code */
     errors,			/* error counter */
     errnum,			/* error number in pass 2 */
     sd_flag,			/* list flag for PSEUDO opcodes */
				/* = 0: address from <val>, data from <ops> */
				/* = 1: address from <sd_val>, data from <ops>*/
				/* = 2: no address, data from <ops> */
				/* = 3: address from <sd_val>, no data */
				/* = 4: suppress whole line */
     sd_val,			/* output value for PSEUDO opcodes */
     prg_adr,			/* start address of program */
     prg_flag,			/* flag for prg_adr valid */
     out_form,			/* format of object file */
     symsize;			/* size of symarray */

FILE *srcfp,			/* file pointer for current source */
     *objfp,			/* file pointer for object code */
     *lstfp,			/* file pointer for listing */
     *errfp;			/* file pointer for error output */

unsigned int
      c_line,			/* current line no. in current source */
      s_line,			/* line no. counter for listing */
      p_line,			/* no. printed lines on page */
      ppl,			/* page length */
      page;			/* no. of pages for listing */

struct sym
     *symtab[HASHSIZE],		/* symbol table */
     **symarray;		/* sorted symbol table */

/*
 *	global variable declarations
 */
extern int	no_opcodes,
		no_operands;

#endif /* _ZZ80ASM_H_ */
