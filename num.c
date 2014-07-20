/*
 *	Z80 - Assembler
 *	Copyright (C) 1987-2014 by Udo Munk
 */

/*
 *	module with numerical computation and conversion
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zz80asm.h"

/*
 *	definitions of operator symbols for expression parser
 */
enum {
	OPEDEC		= 1,	/* decimal number */
	OPEHEX		= 2,	/* hexadecimal number */
	OPEOCT		= 3,	/* octal number */
	OPEBIN		= 4,	/* binary number */
	OPESUB		= 5,	/* arithmetical - */
	OPEADD		= 6,	/* arithmetical + */
	OPEMUL		= 7,	/* arithmetical * */
	OPEDIV		= 8,	/* arithmetical / */
	OPEMOD		= 9,	/* arithmetical modulo */
	OPESHL		= 10,	/* logical shift left */
	OPESHR		= 11,	/* logical shift right */
	OPELOR		= 12,	/* logical OR */
	OPELAN		= 13,	/* logical AND */
	OPEXOR		= 14,	/* logical XOR */
	OPECOM		= 15,	/* logical complement */
	OPESYM		= 99	/* symbol */
};

static int strval(char *);
static int isari(int);
static int get_type(char *);
static int axtoi(char *);
static int abtoi(char *);
static int aotoi(char *);

/*
 *	recursive expression parser
 *
 *	Input: pointer to argument rest string
 *
 *	Output: computed value
 */
int eval(char *s)
{
	char *p;
	int val;
	char word[MAXLINE];
	struct sym *sp;

	val = 0;
	while (*s) {
		p = word;
		if (*s == '(') {
			s++;
			while (*s != ')') {
				if (*s == '\0') {
					asmerr(E_MISPAR);
					goto eval_break;
				}
				*p++ = *s++;
			}
			*p = '\0';
			s++;
			val = eval(word);
			continue;
		}
		if (*s == STRSEP) {
			s++;
			while (*s != STRSEP) {
				if (*s == '\n' || *s == '\0') {
					asmerr(E_MISHYP);
					goto hyp_error;
				}
				*p++ = *s++;
			}
			s++;
hyp_error:
			*p = '\0';
			val = strval(word);
			continue;
		}
		if (isari(*s))
			*p++ = *s++;
		else
			while (!isspace((int)*s) && !isari(*s) && (*s != '\0'))
				*p++ = *s++;
		*p = '\0';
		switch (get_type(word)) {
		case OPESYM:			/* symbol */
			if (strcmp(word, "$") == 0) {
				val = pc;
				break;
			}
			if (strlen(word) > SYMSIZE)
				word[SYMSIZE] = '\0';
			if ((sp = get_sym(word)) != NULL)
				val = sp->sym_val;
			else
				asmerr(E_UNDSYM);
			break;
		case OPEDEC:			/* decimal number */
			val = atoi(word);
			break;
		case OPEHEX:			/* hexadecimal number */
			val = axtoi(word);
			break;
		case OPEBIN:			/* binary number */
			val = abtoi(word);
			break;
		case OPEOCT:			/* octal number */
			val = aotoi(word);
			break;
		case OPESUB:			/* arithmetical - */
			val -= eval(s);
			goto eval_break;
		case OPEADD:			/* arithmetical + */
			val += eval(s);
			goto eval_break;
		case OPEMUL:			/* arithmetical * */
			val *= eval(s);
			goto eval_break;
		case OPEDIV:			/* arithmetical / */
			val /= eval(s);
			goto eval_break;
		case OPEMOD:			/* arithmetical modulo */
			val %= eval(s);
			goto eval_break;
		case OPESHL:			/* logical shift left */
			val <<= eval(s);
			goto eval_break;
		case OPESHR:			/* logical shift right */
			val >>= eval(s);
			goto eval_break;
		case OPELOR:			/* logical OR */
			val |= eval(s);
			goto eval_break;
		case OPELAN:			/* logical AND */
			val &= eval(s);
			goto eval_break;
		case OPEXOR:			/* logical XOR */
			val ^= eval(s);
			goto eval_break;
		case OPECOM:			/* logical complement */
			val = ~(eval(s));
			goto eval_break;
		}
	}
	eval_break:
	return(val);
}

/*
 *	get type of operand
 *
 *	Input: pointer to string with operand
 *
 *	Output: operand type
 */
static int get_type(char *s)
{
	if (isdigit((int)*s)) {		/* numerical operand */
		if (isdigit((int)*(s + strlen(s) - 1)))	/* decimal number */
			return(OPEDEC);
		else if (*(s + strlen(s) - 1) == 'H')	/* hexadecimal number */
			return(OPEHEX);
		else if (*(s + strlen(s) - 1) == 'B')	/* binary number */
			return(OPEBIN);
		else if (*(s + strlen(s) - 1) == 'O')	/* octal number */
			return(OPEOCT);
	} else if (*s == '-')		/* arithmetical operand - */
		return(OPESUB);
	else if (*s == '+')		/* arithmetical operand + */
		return(OPEADD);
	else if (*s == '*')		/* arithmetical operand * */
		return(OPEMUL);
	else if (*s == '/')		/* arithmetical operand / */
		return(OPEDIV);
	else if (*s == '%')		/* arithmetical modulo */
		return(OPEMOD);
	else if (*s == '<')		/* logical shift left */
		return(OPESHL);
	else if (*s == '>')		/* logical shift right */
		return(OPESHR);
	else if (*s == '|')		/* logical OR */
		return(OPELOR);
	else if (*s == '&')		/* logical AND */
		return(OPELAN);
	else if (*s == '^')		/* logical XOR */
		return(OPEXOR);
	else if (*s == '~')		/* logical complement */
		return(OPECOM);
	return(OPESYM);			/* operand is symbol */
}

/*
 *	check a character for arithmetical operators
 *	+, -, *, /, %, <, >, |, &, ~ and ^
 */
static int isari(int c)
{
	return((c) == '+' || (c) == '-' || (c) == '*' ||
	       (c) == '/' || (c) == '%' || (c) == '<' ||
	       (c) == '>' || (c) == '|' || (c) == '&' ||
	       (c) == '~' || (c) == '^');
}

/*
 *	conversion of string with hexadecimal number to integer
 *	format: nnnnH or 0nnnnH if 1st digit > 9
 */
static int axtoi(char *str)
{
	int num;

	num = 0;
	while (isxdigit((int)*str)) {
		num *= 16;
		num += *str - ((*str <= '9') ? '0' : '7');
		str++;
	}
	return(num);
}

/*
 *	conversion of string with octal number to integer
 *	format: nnnnO
 */
static int aotoi(char *str)
{
	int num;

	num = 0;
	while ('0' <= *str && *str <= '7') {
		num *= 8;
		num += (*str++) - '0';
	}
	return(num);
}

/*
 *	conversion of string with binary number to integer
 *	format: nnnnnnnnnnnnnnnnB
 */
int abtoi(char *str)
{
	int num;

	num = 0;
	while ('0' <= *str && *str <= '1') {
		num *= 2;
		num += (*str++) - '0';
	}
	return(num);
}

/*
 *	convert ASCII string to integer
 */
static int strval(char *str)
{
	int num;

	num = 0;
	while (*str) {
		num <<= 8;
		num += (int) *str++;
	}
	return(num);
}

/*
 *	check value for range -256 < value < 256
 *	Output: value if in range, otherwise 0 and error message
 */
int chk_v1(int i)
{
	if (i >= -255 && i <= 255)
		return(i);
	else {
		asmerr(E_VALOUT);
		return(0);
	}
}

/*
 *	check value for range -128 < value < 128
 *	Output: value if in range, otherwise 0 and error message
 */
int chk_v2(int i)
{
	if (i >= -127 && i <= 127)
		return(i);
	else {
		asmerr(E_VALOUT);
		return(0);
	}
}
