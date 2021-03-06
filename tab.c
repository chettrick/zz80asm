/*
 * Copyright (c) 1987-2014 Udo Munk
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/*
 *	module with table operations on opcode and symbol tables
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zz80asm.h"
#include "tab.h"

struct sym	 *symtab[HASHSIZE];	/* symbol table */
struct sym	**symarray;		/* sorted symbol table */

static int 	hash(const char *);
static int 	numcmp(const int, const int);

/*
 *	binary search in sorted table opctab
 *
 *	Input: pointer to string with opcode
 *
 *	Output: pointer to table element, or NULL if not found
 */
struct opc *
search_op(const char * const op_name)
{
	int		 cond;
	struct opc	*low, *mid, *high;

	low = &opctab[0];
	high = &opctab[no_opcodes - 1];
	while (low <= high) {
		mid = low + (high - low) / 2;
		if ((cond = strcmp(op_name, mid->op_name)) < 0)
			high = mid - 1;
		else if (cond > 0)
			low = mid + 1;
		else
			return (mid);
	}
	return (NULL);
}

/*
 *	binary search on sorted table opetab
 *
 *	Input: pointer to string with operand
 *
 *	Output: symbol for operand, NOOPERA if empty operand,
 *		NOREG if operand not found
 */
int
get_reg(const char * const s)
{
	int		 cond;
	struct ope	*low, *mid, *high;

	if (s == NULL || *s == '\0')
		return (NOOPERA);
	low = &opetab[0];
	high = &opetab[no_operands - 1];
	while (low <= high) {
		mid = low + (high - low) / 2;
		if ((cond = strcmp(s, mid->ope_name)) < 0)
			high = mid - 1;
		else if (cond > 0)
			low = mid + 1;
		else
			return (mid->ope_sym);
	}
	return (NOREG);
}

/*
 *	hash search on symbol table symtab
 *
 *	Input: pointer to string with symbol
 *
 *	Output: pointer to table element, or NULL if not found
 */
struct sym *
get_sym(const char * const sym_name)
{
	struct sym	*np;

	for (np = symtab[hash(sym_name)]; np != NULL; np = np->sym_next)
		if (strcmp(sym_name, np->sym_name) == 0)
			return (np);
	return (NULL);
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
int
put_sym(const char * const sym_name, const int sym_val)
{
	int		 hashval;
	struct sym	*np;

	if (!gencode)
		return (0);
	if ((np = get_sym(sym_name)) == NULL) {
		np = malloc(sizeof(struct sym));
		if (np == NULL)
			return (1);
		if ((np->sym_name = strdup(sym_name)) == NULL)
			return (1);
		hashval = hash(sym_name);
		np->sym_next = symtab[hashval];
		symtab[hashval] = np;
	}
	np->sym_val = sym_val;
	return (0);
}

/*
 *	add label to symbol table, error if symbol already exists
 */
void
put_label(void)
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
static int
hash(const char * name)
{
	int 	hashval;

	for (hashval = 0; *name;)
		hashval += *name++;
	return (hashval % HASHSIZE);
}

/*
 *	copy whole symbol hash table into allocated pointer array
 *	used for sorting the symbol table later
 */
size_t
copy_sym(void)
{
	size_t		  i, j;
	size_t		  symsize;	/* size of symarray */
	struct sym	 *np;
	size_t		  newsize;	/* new size of symarray */
	struct sym	**newarray;	/* new sorted symbol table */

	symsize = SYMINC;
	symarray = calloc(symsize, sizeof(struct sym *));
	if (symarray == NULL)
		fatal(F_OUTMEM, "sorting symbol table");
	for (i = 0, j = 0; i < HASHSIZE; i++) {
		if (symtab[i] != NULL) {
			for (np = symtab[i]; np != NULL; np = np->sym_next) {
				symarray[j++] = np;
				if (j == symsize) {
					newsize = symsize + SYMINC;

					if (sizeof(struct sym *) && newsize >
					    SIZE_MAX / sizeof(struct sym *))
						fatal(F_INTERN, "overflow");

					newarray = realloc(symarray,
					    newsize * sizeof(struct sym *));
					if (newarray == NULL)
						fatal(F_OUTMEM,
						    "sorting symbol table");
					symarray = newarray;
					symsize = newsize;
				}
			}
		}
	}
	return (j);
}

/*
 *	sort symbol table by address or name
 */
void
sort_sym(const size_t len, int flag)
{
	int		 gap, i, j;
	struct sym	*temp;

	for (gap = (int)(len / 2); gap > 0; gap /= 2) {
		for (i = gap; i < (int)len; i++) {
			for (j = i - gap; j >= 0; j -= gap) {
				if (flag == 'a') {
					if (numcmp(symarray[j]->sym_val,
					    symarray[j + gap]->sym_val) <= 0)
						break;
				} else if (flag == 'n') {
					if (strcmp(symarray[j]->sym_name,
					    symarray[j + gap]->sym_name) <= 0)
						break;
				} else
					fatal(F_INTERN, "illegal flag");
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
static int
numcmp(const int n1, const int n2)
{
	if ((unsigned int)(n1 & 0xffff) < (unsigned int)(n2 & 0xffff))
		return (-1);
	else if ((unsigned int)(n1 & 0xffff) > (unsigned int)(n2 & 0xffff))
		return (1);
	else
		return (0);
}
