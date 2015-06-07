// license:BSD-3-Clause
// copyright-holders:Raphael Nabet
/*
    cpu/apexc/apexcsm.c : APE(X)C CPU disassembler

    By Raphael Nabet

    see cpu/apexc.c for background and tech info
*/


#include "emu.h"
#include "debugger.h"

#include "apexc.h"

/*
    Here is the format used for debugger output.

    Since the only assembler for the APEXC that I know of uses numerical data
    (yes, mnemonics are numbers), I do not know if there is an official way of writing
    APEXC assembly on a text terminal.  The format I chose is closely inspired by
    the assembly format found in Booth, but was slightly adapted to accommodate
    the lack of subscripts and of a 'greater or equal' character.

        Printed format                      Name
    0         1         2
    012345678901234567890123456
    Stop                (##/##)         one_address
    I                   (##/##)         one_address
    P                   (##/##)         one_address
    B        <(##/##) >=(##/##)         branch
    l (##)              (##/##)         shiftl
    r (##)              (##/##)         shiftr
    Illegal             (##/##)         one_address
    X (##)    (##)      (##/##)         multiply
    X         (##)      (##/##)         multiply
    +c        (##/##)   (##/##)         two_address
    -c        (##/##)   (##/##)         two_address
    +         (##/##)   (##/##)         two_address
    -         (##/##)   (##/##)         two_address
    T         (##/##)   (##/##)         two_address
    R  (1-##) (##/##)   (##/##)         store
    R (##-32) (##/##)   (##/##)         store
    R         (##/##)   (##/##)         store
    A  (1-##) (##/##)   (##/##)         store
    A (##-32) (##/##)   (##/##)         store
    A         (##/##)   (##/##)         store
    S         (##)      (##/##)         swap
    +--------++--------++-----+
    mnemonic   X field  Y field
     field

    For vector instructions, replace the first space on the right of the mnemonic
    with a 'v'.

    01-Feb-2010 (Robbbert):
    I've added the actual address, (as shown in the extreme left of the debugger
    output), so that you can see much easier how the program will flow. Example:

    +C  XXX(##/##)   XXX(##/##)

    The X value shows where the data word is located, and the Y value is the
    address of the next instruction.
*/
enum format_type {branch, shiftl, shiftr, multiply, store, swap, one_address, two_address};

struct instr_desc
{
	const char *mnemonic;
	format_type format; /* -> X and Y are format */
};

static const instr_desc instructions[16] =
{
	{ "Stop",   one_address },  { "I",      one_address },
	{ "P",      one_address },  { "B",      branch },
	{ "l",      shiftl },       { "r",      shiftr },
	{ "Illegal",one_address },  { "X",      multiply },
	{ "+c",     two_address },  { "-c",     two_address },
	{ "+",      two_address },  { "-",      two_address },
	{ "T",      two_address },  { "R",      store },
	{ "A",      store },        { "S",      swap }
};

CPU_DISASSEMBLE( apexc )
{
	UINT32 instruction;         /* 32-bit machine instruction */
	int x, y, function, c6, vector; /* instruction fields */
	int n;                      /* 'friendly', instruction-dependant interpretation of C6 */
	const instr_desc *the_desc; /* pointer to the relevant entry in the instructions array */
	char mnemonic[9];           /* storage for generated mnemonic */

	/* read the instruction to disassemble */
	instruction = oprom[0] << 24 | oprom[1] << 16 | oprom[2] << 8 | oprom[3];

	/* isolate the instruction fields */
	x = (instruction >> 22) & 0x3FF;
	y = (instruction >> 12) & 0x3FF;
	function = (instruction >> 7) & 0x1F;
	c6 = (instruction >> 1) & 0x3F;
	vector = instruction & 1;

	/* get the relevant entry in instructions */
	the_desc = & instructions[function >> 1];

	/* generate mnemonic : append a 'v' to the basic mnemonic if it is a vector instruction */
	sprintf(mnemonic, "%.*s%c", (int)sizeof(mnemonic)-2, the_desc->mnemonic, vector ? 'v' : ' ');

	/* print mnemonic and n immediate */
	switch (the_desc->format)
	{
	case one_address:
	case two_address:
	case branch:
	case swap:
		buffer += sprintf(buffer, "   %-10s", mnemonic);    /* 10 chars*/
		break;

	case shiftl:
	case shiftr:
		if (the_desc->format == shiftl)
			n = c6;
		else
			n = 64-c6;
		buffer += sprintf(buffer, "   %-2s(%2d)    ", mnemonic, n); /* 10 chars */
		break;

	case multiply:
		n = 33-c6;
		if (n == 32)
			/* case "32" : do not show bit specifier */
			buffer += sprintf(buffer, "   %-10s", mnemonic);    /* 10 chars */
		else
			buffer += sprintf(buffer, "   %-2s(%2d)    ", mnemonic, n); /* 10 chars */
		break;

	case store:
		if (c6 == 0)
		{   /* case "1-32" : do not show bit specifier */
			buffer += sprintf(buffer, "   %-10s", mnemonic);    /* 10 chars*/
		}
		else if (c6 & 0x20)
		{   /* case "1-n" */
			n = c6-32;
			buffer += sprintf(buffer, "   %-2s (1-%02d) ", mnemonic, n);    /* 10 chars */
		}
		else
		{   /* case "n-32" */
			n = c6+1;
			buffer += sprintf(buffer, "   %-2s(%02d-32) ", mnemonic, n);    /* 8 chars */
		}
	}

	/* print X address */
	switch (the_desc->format)
	{
	case branch:
		buffer--;   /* eat last char */
		buffer += sprintf(buffer, "<%03X(%02d/%02d) >=", x<<2, (x >> 5) & 0x1f, x & 0x1f);  /* 10+1 chars */
		break;

	case multiply:
	case swap:
		buffer += sprintf(buffer, "   (%02d)      ", (x >> 5) & 0x1f);  /* 10 chars */
		break;

	case one_address:
	case shiftl:
	case shiftr:
		buffer += sprintf(buffer, "             "); /* 10 chars */
		break;

	case two_address:
	case store:
		buffer += sprintf(buffer, "%03X(%02d/%02d)   ", x<<2, (x >> 5) & 0x1f, x & 0x1f);   /* 10 chars */
		break;
	}

	/* print Y address */
	buffer += sprintf(buffer, "%03X(%02d/%02d)", y<<2, (y >> 5) & 0x1f, y & 0x1f);  /* 7 chars */

	return 4;
}
