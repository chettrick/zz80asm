/*
 *	Z80 - Assembler
 *	Copyright (C) 1987-2014 by Udo Munk
 */

/*
 *	module with table operations on opcode and symbol tables
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zz80asm.h"
#include "tab.h"

static int hash(const char *);
static int numcmp(const int, const int);
static char *strsave(const char * const);

/*
 *	binary search in sorted table opctab
 *
 *	Input: pointer to string with opcode
 *
 *	Output: pointer to table element, or NULL if not found
 */
struct opc *search_op(const char * const op_name)
{
	int cond;
	struct opc *low, *high, *mid;

	low = &opctab[0];
	high = &opctab[no_opcodes - 1];
	while (low <= high) {
		mid = low + (high - low) / 2;
		if ((cond = strcmp(op_name, mid->op_name)) < 0)
			high = mid - 1;
		else if (cond > 0)
			low = mid + 1;
		else
			return(mid);
	}
	return(NULL);
}

/*
 *	binary search on sorted table opetab
 *
 *	Input: pointer to string with operand
 *
 *	Output: symbol for operand, NOOPERA if empty operand,
 *		NOREG if operand not found
 */
int get_reg(const char * const s)
{
	int cond;
	struct ope *low, *high, *mid;

	if (s == NULL || *s == '\0')
		return(NOOPERA);
	low = &opetab[0];
	high = &opetab[no_operands - 1];
	while (low <= high) {
		mid = low + (high - low) / 2;
		if ((cond = strcmp(s, mid->ope_name)) < 0)
			high = mid - 1;
		else if (cond > 0)
			low = mid + 1;
		else
			return(mid->ope_sym);
	}
	return(NOREG);
}

/*
 *	hash search on symbol table symtab
 *
 *	Input: pointer to string with symbol
 *
 *	Output: pointer to table element, or NULL if not found
 */
struct sym *get_sym(const char * const sym_name)
{
	struct sym *np;

	for (np = symtab[hash(sym_name)]; np != NULL; np = np->sym_next)
		if (strcmp(sym_name, np->sym_name) == 0)
			return(np);
	return(NULL);
}

/*
 *	add symbol to symbol table symtab, or modify existing symbol
 *
 *	Input: sym_name pointer to string with symbol name
 *	       sym_val  value of symbol
 *
 *	Output: 0 symbol added/modified
 *		1 out of memory
 */
int put_sym(const char * const sym_name, const int sym_val)
{
	int hashval;
	struct sym *np;

	if (!gencode)
		return(0);
	if ((np = get_sym(sym_name)) == NULL) {
		np = (struct sym *) malloc(sizeof (struct sym));
		if (np == NULL)
			return(1);
		if ((np->sym_name = strsave(sym_name)) == NULL)
			return(1);
		hashval = hash(sym_name);
		np->sym_next = symtab[hashval];
		symtab[hashval] = np;
	}
	np->sym_val = sym_val;
	return(0);
}

/*
 *	add label to symbol table, error if symbol already exists
 */
void put_label(void)
{
	if (get_sym(label) == NULL) {
		if (put_sym(label, pc))
			fatal(F_OUTMEM, "symbols");
	} else
		asmerr(E_MULSYM);
}

/*
 *	hash algorithm
 *
 *	Input: pointer to string with name
 *
 *	Output: hash value
 */
static int hash(const char * name)
{
	int hashval;

	for (hashval = 0; *name;)
		hashval += *name++;
	return(hashval % HASHSIZE);
}

/*
 *	save string into allocated memory
 *
 *	Input: pointer to string
 *
 *	Output: pointer to allocated memory with string
 */
static char *strsave(const char * const s)
{
	char *p;

	if ((p = malloc((unsigned int)strlen(s)+1)) != NULL)
		strlcpy(p, s, sizeof(p));
	return(p);
}

/*
 *	copy whole symbol hash table into allocated pointer array
 *	used for sorting the symbol table later
 */
size_t copy_sym(void)
{
	size_t i, j;
	size_t symsize;		/* size of symarray */
	struct sym *np;

	symarray = (struct sym **) malloc(SYMINC * sizeof(struct sym *));
	if (symarray == NULL)
		fatal(F_OUTMEM, "sorting symbol table");
	symsize = SYMINC;
	for (i = 0, j = 0; i < HASHSIZE; i++) {
		if (symtab[i] != NULL) {
			for (np = symtab[i]; np != NULL; np = np->sym_next) {
				symarray[j++] = np;
				if (j == symsize) {
					symarray = (struct sym **) realloc((char *) symarray, symsize * sizeof(struct sym *) + SYMINC * sizeof(struct sym *));
					if (symarray == NULL)
						fatal(F_OUTMEM, "sorting symbol table");
					symsize += SYMINC;
				}
			}
		}
	}
	return(j);
}

/*
 *	sort symbol table by name
 */
void n_sort_sym(const size_t len)
{
	int gap, i, j;
	struct sym *temp;

	for (gap = (int)(len / 2); gap > 0; gap /= 2) {
		for (i = gap; i < (int)len; i++) {
			for (j = i - gap; j >= 0; j -= gap) {
				if (strcmp(symarray[j]->sym_name,
				    symarray[j + gap]->sym_name) <= 0)
					break;
				temp = symarray[j];
				symarray[j] = symarray[j + gap];
				symarray[j + gap] = temp;
			}
		}
	}
}

/*
 *	sort symbol table by address
 */
void a_sort_sym(const size_t len)
{
	int gap, i, j;
	struct sym *temp;

	for (gap = (int)(len / 2); gap > 0; gap /= 2) {
		for (i = gap; i < (int)len; i++) {
			for (j = i - gap; j >= 0; j -= gap) {
				if (numcmp(symarray[j]->sym_val,
				    symarray[j + gap]->sym_val) <= 0)
					break;
				temp = symarray[j];
				symarray[j] = symarray[j + gap];
				symarray[j + gap] = temp;
			}
		}
	}
}

/*
 *	compares two 16bit values, result like strcmp()
 */
static int numcmp(const int n1, const int n2)
{
	if ((unsigned int)(n1 & 0xffff) < (unsigned int)(n2 & 0xffff))
		return(-1);
	else if ((unsigned int)(n1 & 0xffff) > (unsigned int)(n2 & 0xffff))
		return(1);
	else
		return(0);
}
