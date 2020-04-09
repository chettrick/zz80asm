ZZ80ASM(1) - General Commands Manual

# NAME

**zz80asm** - z80 cross assembler

# SYNOPSIS

**zz80asm**
\[**-b**&nbsp;*length*]
\[**-f**&nbsp;*b|h|m*]
\[**-l**&nbsp;\[*listfile*]]
\[**-o**&nbsp;*outfile*]
\[**-s**&nbsp;*a|n*]
\[**-v**]
\[**-x**]
*filename&nbsp;...*

# DESCRIPTION

The
**zz80asm**
utility assembles the files
*filename ...*
and writes to the output file
*outfile*,
and optionally the listing file
*listfile*.

The options are as follows:

**-b** *length*

> Set
> *length*
> bytes per record in Intel Hex formatted
> *outfile*.
> By default,
> *length*
> is interpreted as a decimal number.
> With a leading
> **0x**
> or
> **0X**,
> *length*
> is interpreted as a hexadecimal number,
> otherwise, with a leading
> **0**,
> *length*
> is interpreted as an octal number.
> The default length is 16 decimal.

**-f** *b|h|m*

> Format
> *outfile*
> as either binary, Intex Hex, or binary with Mostek header, respectively.
> Intel Hex is the default format.

**-l** \[*listfile*]

> Generate listing file as
> *listfile*,
> or as
> *filename.lst*
> by default.

**-o** *outfile*

> Set output filename to
> *outfile*
> instead of the default
> *filename.hex*
> or
> *filename.bin*.

**-s** *a|n*

> Generate symbol table at end of listing file.
> This option only works in combination with
> *-l*.
> *a*
> and
> *n*
> sort the symbol table by address or name, respectively.

**-v**

> Produce more verbose output.

**-x**

> Do not output data into
> *outfile*
> for DEFS in pass two of the assembler.

# PSEUDO OPERATIONS

## Symbol definition and memory allocation

ORG &lt;expression&gt;

> Set program address.

&lt;symbol&gt; EQU &lt;expression&gt;

> Define constant symbol.

&lt;symbol&gt; DEFL &lt;expression&gt;

> Define variable symbol.

&lt;symbol&gt; DEFB &lt;expression, 'char', ...&gt;

> Write 8 bit bytes in memory.

&lt;symbol&gt; DEFW &lt;expression, expression ...&gt;

> Write 16 bit words in memory.

&lt;symbol&gt; DEFM &lt;'string'&gt;

> Write character string in memory.

&lt;symbol&gt; DEFS &lt;expression&gt;

> Reserve space in memory.

## Conditional assembly

IFDEF &lt;symbol&gt;

> Assemble if symbol defined.

IFNDEF &lt;symbol&gt;

> Assemble if symbol not defined.

IFEQ &lt;expression 1, expression 2&gt;

> Assemble if equal.

IFNEQ &lt;expression 1, expression 2&gt;

> Assemble if not equal.

ELSE

> Else clause for all conditionals.

ENDIF

> End of conditional assembly.

## Manipulation of list file

TITLE &lt;'string'&gt;

> Title for page header.

PAGE &lt;expression&gt;

> Number of lines per page.

EJECT

> Skip to new page.

LIST

> Turn listing on.

NOLIST

> Turn listing off.

## Miscellaneous

INCLUDE &lt;filename&gt;

> Include another source file.

PRINT &lt;'string'&gt;

> Print string to stdout in pass one of the assembler.

# EXIT STATUS

The **zz80asm** utility exits&#160;0 on success, and&#160;&gt;0 if an error occurs.

# AUTHORS

**zz80asm**
was written by
Udo Munk &lt;udo.munk@freenet.de&gt;
and updated by
Chris Hettrick &lt;chris@structfoo.com&gt;.

OpenBSD 6.6 - November 12, 2017
