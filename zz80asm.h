/*
 *	Z80 - Assembler
 *	Copyright (C) 1987-2014 by Udo Munk
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

enum {
	OUTBIN		= 1,	/* format of object: binary */
	OUTMOS		= 2,	/* format of object: Mostek binary */
	OUTHEX		= 3,	/* format of object: Intel hex */
	OUTDEF		= OUTHEX	/* default object format */
};

/*
 *	definition of operand symbols
 *	definitions for registers A, B, C, D, H, L and (HL)
 *	are defined as the bits used in operands and may not
 *	be changed!
 */
enum {
	REGB		= 0,	/* register B */
	REGC		= 1,	/* register C */
	REGD		= 2,	/* register D */
	REGE		= 3,	/* register E */
	REGH		= 4,	/* register H */
	REGL		= 5,	/* register L */
	REGIHL		= 6,	/* register indirect HL */
	REGA		= 7,	/* register A */
	REGI		= 8,	/* register I */
	REGR		= 9,	/* register R */
	REGAF		= 10,	/* register pair AF */
	REGBC		= 11,	/* register pair BC */
	REGDE		= 12,	/* register pair DE */
	REGHL		= 13,	/* register pair HL */
	REGIX		= 14,	/* register IX */
	REGIY		= 15,	/* register IY */
	REGSP		= 16,	/* register SP */
	REGIBC		= 17,	/* register indirect BC */
	REGIDE		= 18,	/* register indirect DE */
	REGIIX		= 19,	/* register indirect IX */
	REGIIY		= 20,	/* register indirect IY */
	REGISP		= 21,	/* register indirect SP */
	FLGNC		= 30,	/* flag no carry */
	FLGNZ		= 31,	/* flag not zero */
	FLGZ		= 32,	/* flag zero */
	FLGM		= 33,	/* flag minus */
	FLGP		= 34,	/* flag plus */
	FLGPE		= 35,	/* flag parity even */
	FLGPO		= 36,	/* flag parity odd */
	NOOPERA		= 98,	/* no operand */
	NOREG		= 99	/* operand isn't register */
};

/*
 *	definitions of error numbers for error messages in listfile
 */
enum err_type {
	E_ILLOPC	= 0,	/* illegal opcode */
	E_ILLOPE	= 1,	/* illegal operand */
	E_MISOPE	= 2,	/* missing operand */
	E_MULSYM	= 3,	/* multiple defined symbol */
	E_UNDSYM	= 4,	/* undefined symbol */
	E_VALOUT	= 5,	/* value out of bounds */
	E_MISPAR	= 6,	/* missing paren */
	E_MISHYP	= 7,	/* missing string separator */
	E_MEMOVR	= 8,	/* memory override (ORG) */
	E_MISIFF	= 9,	/* missing IF at ELSE or ENDIF */
	E_IFNEST	= 10,	/* to many IF's nested */
	E_MISEIF	= 11,	/* missing ENDIF */
	E_INCNEST	= 12	/* to many INCLUDE's nested */
};

/*
 *	definition fatal errors
 */
enum fatal_type {
	F_OUTMEM	= 0,	/* out of memory */
	F_USAGE		= 1,	/* usage: ... */
	F_HALT		= 2,	/* assembly halted */
	F_FOPEN		= 3,	/* can't open file */
	F_INTERN	= 4	/* internal error */
};

/*
 *	structure opcode table
 */
struct opc {
	char	*op_name;	/* opcode name */
	int	 (*op_fun)();	/* function pointer code generation */
	int	 op_c1;		/* first base opcode */
	int	 op_c2;		/* second base opcode */
};

/*
 *	structure operand table
 */
struct ope {
	char	*ope_name;	/* operand name */
	int	 ope_sym;	/* symbol value operand */
};

/*
 *	structure symbol table entries
 */
struct sym {
	struct	 sym *sym_next;	/* next entry */
	char	*sym_name;	/* symbol name */
	int	 sym_val;	/* symbol value */
};

/*
 *	structure nested INCLUDE's
 */
struct inc {
	char	*inc_fn;	/* filename */
	FILE	*inc_fp;	/* file pointer */
	size_t	 inc_line;	/* line counter for listing */
};

/*
 *	global variables other than CPU specific tables
 */
FILE		*srcfp;		/* file pointer for current source */
FILE		*objfp;		/* file pointer for object code */
FILE		*lstfp;		/* file pointer for listing */
FILE		*errfp;		/* file pointer for error output */

char		*srcfn;		/* filename of current processed source file */
char		 line[MAXLINE];	/* buffer for one line source */
char		 tmp[MAXLINE];	/* temporary buffer */
char		 label[SYMSIZE + 1];	/* buffer for label */
char		 operand[MAXLINE];	/* buffer for operand */
char		 title[MAXLINE];	/* buffer for title of source */

int		 ops[OPCARRAY];	/* buffer for generated object code */

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
int		sd_val;		/* output value for PSEUDO opcodes */
int		prg_adr;	/* start address of program */
uint8_t		out_form;	/* format of object file */

size_t		c_line;		/* current line no. in current source */
size_t		s_line;		/* line no. counter for listing */
size_t		p_line;		/* no. printed lines on page */
size_t		ppl;		/* page length */

struct sym	 *symtab[HASHSIZE];	/* symbol table */
struct sym	**symarray;	/* sorted symbol table */

/*
 *	function prototypes
 */
/* num.c */
int eval(char *);
int chk_v1(int);
int chk_v2(int);

/* out.c */
void asmerr(enum err_type);
void lst_header(void);
void lst_attl(void);
void lst_line(int, int);
void lst_sym(void);
void lst_sort_sym(size_t);
void obj_header(void);
void obj_end(void);
void obj_writeb(size_t);
void obj_fill(int);

/* pfun.c */
int op_org(void);
int op_equ(void);
int op_dl(void);
int op_ds(void);
int op_db(void);
int op_dm(void);
int op_dw(void);
int op_misc(int);
int op_cond(int);
int op_glob(int);

/* rfun.c */
int op_1b(int), op_2b(int, int), op_pupo(int);
int op_ex(void), op_ld(void), op_call(void), op_ret(void);
int op_jp(void), op_jr(void), op_djnz(void), op_rst(void);
int op_add(void), op_adc(void), op_sub(void), op_sbc(void), op_cp(void);
int op_inc(void), op_dec(void), op_or(void), op_xor(void), op_and(void);
int op_rl(void), op_rr(void), op_sla(void), op_sra(void), op_srl(void);
int op_rlc(void), op_rrc(void);
int op_out(void), op_in(void), op_im(void);
int op_set(void), op_res(void), op_bit(void);

/* tab.c */
struct opc *search_op(char *);
struct sym *get_sym(char *);
int put_sym(char *, int);
int get_reg(char *);
void put_label(void);
size_t copy_sym(void);
void n_sort_sym(size_t);
void a_sort_sym(size_t);

/* zz80asm.c */
void fatal(enum fatal_type, char *) __attribute__((noreturn));
void p1_file(char *);
void p2_file(char *);

#endif /* _ZZ80ASM_H_ */
