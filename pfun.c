/*
 *	Z80 - Assembler
 *	Copyright (C) 1987-2014 by Udo Munk
 */

/*
 *	processing of all PSEUDO ops
 */

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "zz80asm.h"

int prg_flag = 0;			/* flag for prg_adr valid */

/*
 *	ORG
 */
int op_org(void)
{
	int i;

	if (!gencode)
		return(0);
	i = eval(operand);
	if (i < pc) {
		asmerr(E_MEMOVR);
		return(0);
	}
	if (pass == 1) {		/* PASS 1 */
		if (!prg_flag) {
			prg_adr = i;
			prg_flag++;
		}
	} else {			/* PASS 2 */
		if (++prg_flag > 2)
			obj_fill(i - pc);
		sd_flag = 2;
	}
	pc = i;
	return(0);
}

/*
 *	EQU
 */
int op_equ(void)
{
	if (!gencode)
		return(0);
	if (pass == 1) {		/* Pass 1 */
		if (get_sym(label) == NULL) {
			sd_val = eval(operand);
			if (put_sym(label, sd_val))
				fatal(F_OUTMEM, "symbols");
		} else
			asmerr(E_MULSYM);
	} else {			/* Pass 2 */
		sd_flag = 1;
		sd_val = eval(operand);
	}
	return(0);
}

/*
 *	DEFL
 */
int op_dl(void)
{
	if (!gencode)
		return(0);
	sd_flag = 1;
	sd_val = eval(operand);
	if (put_sym(label, sd_val))
		fatal(F_OUTMEM, "symbols");
	return(0);
}

/*
 *	DEFS
 */
int op_ds(void)
{
	int val;

	if (!gencode)
		return(0);
	if ((pass == 1) && *label)
		put_label();
	sd_val = pc;
	sd_flag = 3;
	val = eval(operand);
	if ((pass == 2) && dump_flag)
		obj_fill(val);
	pc += val;
	return(0);
}

/*
 *	DEFB
 */
int op_db(void)
{
	int i;
	char *p;
	char *s;

	if (!gencode)
		return(0);
	i = 0;
	p = operand;
	if ((pass == 1) && *label)
		put_label();
	while (*p) {
		if (*p == STRSEP) {
			p++;
			while (*p != STRSEP) {
				if (*p == '\n' || *p == '\0') {
					asmerr(E_MISHYP);
					goto hyp_error;
				}
				ops[i++] = *p++;
				if (i >= OPCARRAY)
				    fatal(F_INTERN, "Op-Code buffer overflow");
			}
			p++;
		} else {
			s = tmp;
			while (*p != ',' && *p != '\0')
				*s++ = *p++;
			*s = '\0';
			ops[i++] = eval(tmp);
			if (i >= OPCARRAY)
				fatal(F_INTERN, "Op-Code buffer overflow");
		}
		if (*p == ',')
			p++;
	}
hyp_error:
	return(i);
}

/*
 *	DEFM
 */
int op_dm(void)
{
	int i;
	char *p;

	if (!gencode)
		return(0);
	i = 0;
	p = operand;
	if ((pass == 1) && *label)
		put_label();
	if (*p != STRSEP) {
		asmerr(E_MISHYP);
		return(0);
	}
	p++;
	while (*p != STRSEP) {
		if (*p == '\n' || *p == '\0') {
			asmerr(E_MISHYP);
			break;
		}
		ops[i++] = *p++;
		if (i >= OPCARRAY)
			fatal(F_INTERN, "Op-Code buffer overflow");
	}
	return(i);
}

/*
 *	DEFW
 */
int op_dw(void)
{
	int i, len, temp;
	char *p;
	char *s;

	if (!gencode)
		return(0);
	p = operand;
	i = len = 0;
	if ((pass == 1) && *label)
		put_label();
	while (*p) {
		s = tmp;
		while (*p != ',' && *p != '\0')
			*s++ = *p++;
		*s = '\0';
		if (pass == 2) {
			temp = eval(tmp);
			ops[i++] = temp & 0xff;
			ops[i++] = temp >> 8;
			if (i >= OPCARRAY)
				fatal(F_INTERN, "Op-Code buffer overflow");
		}
		len += 2;
		if (*p == ',')
			p++;
	}
	return(len);
}

/*
 *	EJECT, LIST, NOLIST, PAGE, PRINT, TITLE, INCLUDE
 */
int op_misc(const int op_code)
{
	char *p, *d;
	static char fn[LENFN];
	static int incnest;
	static struct inc incl[INCNEST];

	if (!gencode)
		return(0);
	sd_flag = 2;
	switch(op_code) {
	case 1:				/* EJECT */
		if (pass == 2)
			p_line = ppl;
		break;
	case 2:				/* LIST */
		if (pass == 2)
			list_flag = 1;
		break;
	case 3:				/* NOLIST */
		if (pass == 2)
			list_flag = 0;
		break;
	case 4:				/* PAGE */
		if (pass == 2)
			ppl = (size_t)eval(operand);
		break;
	case 5:				/* PRINT */
		if (pass == 1) {
			p = operand;
			while (*p) {
				if (*p != STRSEP)
					putc(*p++, stdout);
				else
					p++;
			}
			putc('\n', stdout);
		}
		break;
	case 6:				/* INCLUDE */
		if (incnest >= INCNEST) {
			asmerr(E_INCNEST);
			break;
		}
		incl[incnest].inc_line = c_line;
		incl[incnest].inc_fn = srcfn;
		incl[incnest].inc_fp = srcfp;
		incnest++;
		p = line;
		d = fn;
		while(isspace((int)*p))	/* ignore white space until INCLUDE */
			p++;
		while(!isspace((int)*p))/* ignore INCLUDE */
			p++;
		while(isspace((int)*p))	/* ignore white space until filename */
			p++;
		while(!isspace((int)*p) && *p != COMMENT) /* get filename */
			*d++ = *p++;
		*d = '\0';
		if (pass == 1) {	/* PASS 1 */
			if (ver_flag)
				fprintf(stdout, "   Include %s\n", fn);
			p1_file(fn);
		} else {		/* PASS 2 */
			sd_flag = 2;
			lst_line(0, 0);
			if (ver_flag)
				fprintf(stdout, "   Include %s\n", fn);
			p2_file(fn);
		}
		incnest--;
		c_line = incl[incnest].inc_line;
		srcfn = incl[incnest].inc_fn;
		srcfp = incl[incnest].inc_fp;
		if (ver_flag)
			fprintf(stdout, "   Resume  %s\n", srcfn);
		if (list_flag && (pass == 2)) {
			lst_header();
			lst_attl();
		}
		sd_flag = 4;
		break;
	case 7:				/* TITLE */
		if (pass == 2) {
			p = line;
			d = title;
			while (isspace((int)*p)) /* ignore white space until TITLE */
				p++;
			while (!isspace((int)*p))/* ignore TITLE */
				p++;
			while (isspace((int)*p)) /* ignore white space until text */
				p++;
			if (*p == STRSEP)
				p++;
			while (*p != '\n' && *p != STRSEP && *p != COMMENT)
				*d++ = *p++;
			*d = '\0';
		}
		break;
	default:
		fatal(F_INTERN, "illegal opcode for function op_misc");
		/* NOTREACHED */
	}
	return(0);
}

/*
 *	IFDEF, IFNDEF, IFEQ, IFNEQ, ELSE, ENDIF
 */
int op_cond(const int op_code)
{
	char *p, *p1, *p2;
	static int condnest[IFNEST];

	switch(op_code) {
	case 1:				/* IFDEF */
		if (iflevel >= IFNEST) {
			asmerr(E_IFNEST);
			break;
		}
		condnest[iflevel++] = gencode;
		if (gencode)
			if (get_sym(operand) == NULL)
				gencode = 0;
		break;
	case 2:				/* IFNDEF */
		if (iflevel >= IFNEST) {
			asmerr(E_IFNEST);
			break;
		}
		condnest[iflevel++] = gencode;
		if (gencode)
			if (get_sym(operand) != NULL)
				gencode = 0;
		break;
	case 3:				/* IFEQ */
		if (iflevel >= IFNEST) {
			asmerr(E_IFNEST);
			break;
		}
		condnest[iflevel++] = gencode;
		p = operand;
		p1 = strchr(operand, ',');
		if ((*p == 0) || (p1 == NULL)) {
			asmerr(E_MISOPE);
			break;
		}
		if (gencode) {
			p2 = tmp;
			while (*p != ',')
				*p2++ = *p++;
			*p2 = '\0';
			if (eval(tmp) != eval(++p1))
				gencode = 0;
		}
		break;
	case 4:				/* IFNEQ */
		if (iflevel >= IFNEST) {
			asmerr(E_IFNEST);
			break;
		}
		condnest[iflevel++] = gencode;
		p = operand;
		p1 = strchr(operand, ',');
		if ((*p == 0) || (p1 == NULL)) {
			asmerr(E_MISOPE);
			break;
		}
		if (gencode) {
			p2 = tmp;
			while (*p != ',')
				*p2++ = *p++;
			*p2 = '\0';
			if (eval(tmp) == eval(++p1))
				gencode = 0;
		}
		break;
	case 98:			/* ELSE */
		if (!iflevel)
			asmerr(E_MISIFF);
		else
			if ((iflevel == 0) || (condnest[iflevel - 1] == 1))
				gencode = !gencode;
		break;
	case 99:			/* ENDIF */
		if (!iflevel)
			asmerr(E_MISIFF);
		else
			gencode = condnest[--iflevel];
		break;
	default:
		fatal(F_INTERN, "illegal opcode for function op_cond");
		/* NOTREACHED */
	}
	sd_flag = 2;
	return(0);
}

/*
 *	EXTRN and PUBLIC
 */
int op_glob(const int op_code)
{
	if (!gencode)
		return(0);
	sd_flag = 2;
	switch(op_code) {
	case 1:				/* EXTRN */
		break;
	case 2:				/* PUBLIC */
		break;
	default:
		fatal(F_INTERN, "illegal opcode for function op_glob");
		/* NOTREACHED */
	}
	return(0);
}
