/***************************************************************************

  jrcrypt.c

  This file is not part of MAME. It is here to provide detailed
  documentation of the encryption used by Jr. Pac Man ROMs.

    David Caldwell 6-1-97
    bug reports and comments to:
    david@indigita.com

    This code is published under the GNU Public License. (GPL)

***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "mame.h"
#include "driver.h"
#include "osdepend.h"

static int interrupt_enable;

typedef UINT16 word;
typedef UINT8 byte;

#define PreDecryptedRoms

#ifndef PreDecryptedRoms
static int s0,s1,s2,s3; /* 1 bit registers inside decoder PAL */
static UINT8 shadowROM[0xffff];
static UINT8 used[0xFFFF];
UINT32 numberUsed = 0;
#else
struct {
    int count;
    int value;
} Jr_PacManTable[] = {
    { 0x00C1, 0x00 },
    { 0x0002, 0x80 },
    { 0x0004, 0x00 },
    { 0x0006, 0x80 },
    { 0x0003, 0x00 },
    { 0x0002, 0x80 },
    { 0x0009, 0x00 },
    { 0x0004, 0x80 },
    { 0x9968, 0x00 },
    { 0x0001, 0x80 },
    { 0x0002, 0x00 },
    { 0x0001, 0x80 },
    { 0x0009, 0x00 },
    { 0x0002, 0x80 },
    { 0x0009, 0x00 },
    { 0x0001, 0x80 },
    { 0x00AF, 0x00 },
    { 0x000E, 0x04 },
    { 0x0002, 0x00 },
    { 0x0004, 0x04 },
    { 0x001E, 0x00 },
    { 0x0001, 0x80 },
    { 0x0002, 0x00 },
    { 0x0001, 0x80 },
    { 0x0002, 0x00 },
    { 0x0002, 0x80 },
    { 0x0009, 0x00 },
    { 0x0002, 0x80 },
    { 0x0009, 0x00 },
    { 0x0002, 0x80 },
    { 0x0083, 0x00 },
    { 0x0001, 0x04 },
    { 0x0001, 0x01 },
    { 0x0001, 0x00 },
    { 0x0002, 0x05 },
    { 0x0001, 0x00 },
    { 0x0003, 0x04 },
    { 0x0003, 0x01 },
    { 0x0002, 0x00 },
    { 0x0001, 0x04 },
    { 0x0003, 0x01 },
    { 0x0003, 0x00 },
    { 0x0003, 0x04 },
    { 0x0001, 0x01 },
    { 0x002E, 0x00 },
    { 0x0078, 0x01 },
    { 0x0001, 0x04 },
    { 0x0001, 0x05 },
    { 0x0001, 0x00 },
    { 0x0001, 0x01 },
    { 0x0001, 0x04 },
    { 0x0002, 0x00 },
    { 0x0001, 0x01 },
    { 0x0001, 0x04 },
    { 0x0002, 0x00 },
    { 0x0001, 0x01 },
    { 0x0001, 0x04 },
    { 0x0002, 0x00 },
    { 0x0001, 0x01 },
    { 0x0001, 0x04 },
    { 0x0001, 0x05 },
    { 0x0001, 0x00 },
    { 0x0001, 0x01 },
    { 0x0001, 0x04 },
    { 0x0002, 0x00 },
    { 0x0001, 0x01 },
    { 0x0001, 0x04 },
    { 0x0002, 0x00 },
    { 0x0001, 0x01 },
    { 0x0001, 0x04 },
    { 0x0001, 0x05 },
    { 0x0001, 0x00 },
    { 0x01B0, 0x01 },
    { 0x0001, 0x00 },
    { 0x0002, 0x01 },
    { 0x00AD, 0x00 },
    { 0x0031, 0x01 },
    { 0x005C, 0x00 },
    { 0x0005, 0x01 },
    { 0x604E, 0x00 },
    { 0,0 }
};
#endif

MACHINE_RESET( jrpacman )
{
#ifndef PreDecryptedRoms
	s0 = 1;
	s1 = 1;
	s2 = 0;
	s3 = 0;

	memset(shadowROM,0,sizeof(shadowROM));
	memset(used,0,sizeof(used));
#endif
}


#ifdef PreDecryptedRoms
unsigned jrpacman_decode_roms(int address)
{
	int i;
	int cumulative=0;
	for (i=0;Jr_PacManTable[i].count;i++)
	{
		cumulative += Jr_PacManTable[i].count;
		if (cumulative > address)
			return RAM[address] ^ Jr_PacManTable[i].value;
	}
	return RAM[address]; // this should never happen!
}
#else
INLINE WordBit(word theWord, int theBit)
{
	return (theWord >> theBit)&1;
}
INLINE ByteBit(byte theByte, int theBit)
{
	return (theByte >> theBit)&1;
}

int jrpacman_romdecode(int offset);
int jrpacman_romdecodeA(int offset)
{
	jrpacman_romdecode(offset);
}

int jrpacman_romdecodeB(int offset)
{
	jrpacman_romdecode(offset + 0x8000);
}


/****************************************************************************

    This function would look suprisingly similiar to the source code to
the encoder pals, I imagine. I wrote is based on the jedec file that I
got from VideoDoctor. It needs to be run during the game, since the
logic is kind of wierd and state based. This is slow. So you'll see
that I made it store its "decrypted" opcodes in another array. When I
ran the program and was satisfied that it had decrypted enough code, I
dumped this new array out into some ROM files.  These ROM files are
effectively decrypted. But nobody else has them, so I'd have to upload
them to some ROM archive somewhere and hope for the best. Besides, If
I ever found a bug in them its not easy to upgrade every ROM archive
site out there. So I created a table that has the decrypted ROMs and
the encrypted ROMs xor-ed with each other.  So if something wrong is
ever discovered , all that is needed to update is the table.  Jr. Pac
only messes with 3 bits (d0, d2 and d7) so the table doesn't look too
excited.  Also I run length encoded it so its not unwieldly.

    A lot of the functions that are ifdef-ed out here were debugging
functions that I used to help me during the decoding process.

 -David Caldwell
 david@indigita.com

***************************************************************************/

int jrpacman_romdecode(int offset)
{
	int addressBus = offset;
	Z80_Regs Regs;
	Z80_GetRegs(&Regs);

	{
	int m1 = !Regs.M1;//active low (supposedly means opcode read)

	/* Pal 8C (16L8) */
	int pcbe =  !(addressBus >= 0x0000 && addressBus <= 0x3fff ||
				  addressBus >= 0x8000 && addressBus <=
0xdfff);

	int sop0 =  !(addressBus >= 0x0000 && addressBus <= 0x001f ||
				  addressBus >= 0x00e0 && addressBus <=
0x00ff ||
				  addressBus >= 0x9a60 && addressBus <=
0x9a7f ||
				  addressBus >= 0x9ac0 && addressBus <=
0x9adf ||
				  addressBus >= 0x9b60 && addressBus <=
0x9b7f ||
				  addressBus >= 0x9be0 && addressBus <=
0x9bff && m1);

	int sop1 =	!(addressBus >= 0x9be0 && addressBus <= 0x9bff && m1 ||
				  addressBus >= 0x9ca0 && addressBus <=
0x9cbf);

	int sop2 =	!(addressBus >= 0x00c0 && addressBus <= 0x00df ||
				  addressBus >= 0x9a40 && addressBus <=
0x9a5f);


	/* Pal 9c (16r4) */
	int md0  = ByteBit(RAM[addressBus],0);
	int md2  = ByteBit(RAM[addressBus],2);
	int md7  = ByteBit(RAM[addressBus],7);

	int d0 =  !( s0 &&  s1 && !md0 ||
			    !s0 &&  s1 &&  md0 ||
			     s0 && !s1 && !md0 ||
			    !s0 && !s1 && !md2);

	int d2 =  !( s0 &&  s1 && !md2 ||
			    !s0 &&  s1 && !md2 ||
			     s0 && !s1 &&  md2 ||
			    !s0 && !s1 && !md0);

	int d7 =  !( s2 &&  s3  ||
			    !s2 && !md7);

	int pb1 = !( sop0 &&  s0 ||
			    !sop0 && !s0);

	int ns0 =  ( sop1 &&  s0   ||
				!sop1 && !s0   ||
				!sop0 &&  sop1);

	int ns1 =  ( sop1 &&  s1 && !pb1 ||
				!sop1 &&  s1 && !pb1 ||
				 sop1 &&  s1 &&  pb1 ||
				!sop1 && !s1 &&  pb1 ||
				!sop0 &&  sop1);

	int ns2 =  ( sop0 &&  sop1 &&  s2 ||
				 sop0 && !sop1 &&  s2 ||
				!sop0 && !sop1 &&  s2 ||
				 sop0 && !sop2);

	int ns3 =  ( !md7 );

//  DebugPrint("%04x: %02x & %02x | %02x = %02x",addressBus,RAM[addressBus],~(1<<0) & ~(1<<2) & ~(1<<7), (d0) | (d2<<2) | (d7<<7),(RAM[addressBus] & ~(1<<0) & ~(1<<2) & ~(1<<7)) | (d0) | (d2<<2) | (d7<<7));
/*  printf("%04x: %02x & %02x | %02x = %02x\n",addressBus,RAM[addressBus],~(1<<0) & ~(1<<2) & ~(1<<7), (d0) | (d2<<2) | (d7<<7),(RAM[addressBus] & ~(1<<0) & ~(1<<2) & ~(1<<7)) | (d0) | (d2<<2) | (d7<<7));
    {static int i=0;
    if (i++>100)
    {
        while (getchar()!='\n')
            {}
    }}*/
	{
		int temp= ((int)RAM[addressBus] & 0x7A) | ((d7<<7) | (d2<<2) | (d0));

//      if (Z80_Trace==1)
			if (!used[addressBus])
			{
				used[addressBus]=1;
				shadowROM[addressBus] = temp;
				numberUsed++;
			}
			else
			{
				if (shadowROM[addressBus] != temp)
					DebugPrint("Address: %04x translates to 2 different values!!!! (%02x and %02x)",addressBus,shadowROM[addressBus],temp);
			}


		if (Z80_Trace==1)
		{
			static last = 0;
			if (last + 30 <= TickCount()) /* print bnanner if we havent been called in half a second */
				printf("m1   sop0 sop1 sop2 pcbe  md7  md2 md0   d7   d2   d0    pb1   s0   s1   s2   s3    ns0  ns1  ns2  ns3\n");
			last = TickCount();
			printf("%-4d %-4d %-4d %-4d %-4d  %-4d %-4d %-4d %-4d %-4d %-4d  %-4d  %-4d %-4d %-4d %-4d  %-4d %-4d %-4d %-4d     ",
			        m1,  sop0,sop1,sop2,pcbe, md7, md2, md0,
d7,  d2,  d0,   pb1,  s0,  s1,  s2,  s3,   ns0, ns1, ns2, ns3);
			printf("%04x: %02x & %02x | %02x = %02x\n",addressBus,RAM[addressBus],~(1<<0) & ~(1<<2) & ~(1<<7), (d0) | (d2<<2) | (d7<<7),(RAM[addressBus] & ~(1<<0) & ~(1<<2) & ~(1<<7)) | (d0) | (d2<<2) | (d7<<7));
			Z80_Trace = 1; /* stop it if it was running for a count */
		}

		/* latch new flip flops on rising edge of pcbe */
		if (!pcbe)
		{
			s0 = ns0;
			s1 = ns1;
			s2 = ns2;
			s3 = ns3;
		}
		return temp;
	}
	}
}

int Z80_Trace=0;

void jr_monitor()
{
	int i;
	int encrypted=0;
	int last_used=-1;
	int unmapped_encrypted=0;
	int unmapped=0;
	printf("statistics: \n");
	printf("  Successfully mapped: %d\n",numberUsed);
	for (i=0;i<0xe000;i=(i==0x4000?0x8000:i+1)) /* skip hole where
hardware and RAM is */
	{
		if (used[i])
		{
			if (shadowROM[i] != RAM[i])
				encrypted = 1;
			else
				encrypted = 0;
			last_used = i;
		}
		else
		{
			if (encrypted)
				unmapped_encrypted++;
			else
				unmapped++;
		}
	}
	printf("  Non mapped, Probably not encrypted: %d\n",unmapped);
	printf("  Non mapped, but Probably encrypted:
%d\n",unmapped_encrypted);

	while (1)
	{
		void write_rom_section(char *prefix,char *suffix,int
start,int end);
		char c;
		printf(" Enter D to Merge mapped and unmapped and dump to
rom file,\n");
		printf(" Enter Q to quit.\n");
		c=tolower(getchar());
		while (getchar()!='\n') {}
		if (c=='q')
			return;
		if (c=='d')
		{
			char line[100],*l;
			int i;
			printf("Enter file prefix (files will be named
'prefix'.8d 'prefix'.8e, etc.\n");
			gets(line);
			// kill newline:
			for (l=line;*l!='\n' && *l!='\0';l++)
				{}
			*l = '\0';

			write_rom_section(line,".8d",0x0000,0x2000);
			write_rom_section(line,".8e",0x2000,0x4000);
			write_rom_section(line,".8h",0x8000,0xa000);
			write_rom_section(line,".8j",0xa000,0xc000);
			write_rom_section(line,".8k",0xc000,0xe000);
		}
	}
}

void write_rom_section(char *prefix,char *suffix,int start,int end)
{
	FILE *out;
	char file[100];
	int i;

	strcpy(file,prefix);
	strcat(file,suffix);
	out = fopen(file,"wb");
	for (i=start;i<end;i++)
		if (used[i])
			putc(shadowROM[i],out);
		else
			putc(RAM[i],out);
	fclose(out);
}
#endif

WRITE8_HANDLER( jrpacman_interrupt_enable_w )
{
	interrupt_enable = data;
}



/***************************************************************************

  Interrupt handler. This function is called at regular intervals
  (determined by IPeriod) by the CPU emulation.

***************************************************************************/

static int IntVector = 0xff;	/* Here we store the interrupt vector, if
the code performs */
						/* an OUT to port $0. Not
all games do it: many use */
						/* Interrupt Mode 1, which
doesn't use an interrupt vector */
						/* (see Z80.c for details). */

INTERRUPT_GEN( jrpacman_interrupt )
{
	irq0_line_hold();
}



/***************************************************************************

  The Pac Man machine uses OUT to port $0 to set the interrupt vector, so
  we have to remember the value passed.

***************************************************************************/
void jrpacman_out(byte Port,byte Value)
{
	/* OUT to port $0 is used to set the interrupt vector */
	if (Port == 0) IntVector = Value;
}

#if 0
/****************************************************************************
What follows is the program I used to create the JrPacMan_Table[] array at
the top of this file. It is included here for completeness.
***************************************************************************/


// CreateJrDecodeTable.c
//
// Copyright (C) 1997 David Caldwell
// This program is published under the GNU Public License.
//
// Comments, questions to: david@indigita.com

#include <stdio.h>

typedef UINT8 byte;

void CreateJrDecodeTable(byte *x, int length);
void Load(char *name,byte *buffer,int from, int length);

byte encrypted[0x10000],decrypted[0x10000];
byte xored[0x10000];
void main()
{
	int i;

	Load("jr8d",encrypted,0x0000,0x2000);
	Load("jr8e",encrypted,0x2000,0x2000);
	Load("jr8h",encrypted,0x8000,0x2000);
	Load("jr8j",encrypted,0xA000,0x2000);
	Load("jr8k",encrypted,0xC000,0x2000);

	Load("1.8d",decrypted,0x0000,0x2000);
	Load("1.8e",decrypted,0x2000,0x2000);
	Load("1.8h",decrypted,0x8000,0x2000);
	Load("1.8j",decrypted,0xA000,0x2000);
	Load("1.8k",decrypted,0xC000,0x2000);

	for (i=0;i<0x10000;i++)
		xored[i] = encrypted[i] ^ decrypted[i];

	CreateJrDecodeTable(xored,0x10000);
}

void Load(char *name,byte *buffer,int from, int length)
{
	file_error filerr;
	mame_file *file;

	filerr = mame_fopen(NULL, name, OPEN_FLAG_READ, &file);
	if (filerr != FILERR_NONE)
		return;
	while (length--)
		buffer[from++]=mame_fgetc(file);
	mame_fclose(file);
}

void CreateJrDecodeTable(byte *x, int length)
{
	int i=0;
	byte last = 0;
	int count = 0;

	printf(	"struct {\n"
			"    int count;\n"
			"    int value;\n"
			"} Jr_PacManTable[] = {\n");

	goto first;

	for (i=0;i<length;i++)
	{
		if (x[i] != last)
		{
			printf("    { 0x%04X, 0x%02X },\n",count,last);
			count = 0;
		}
first:
		last = x[i];
		count++;
	}

	printf("    { 0x%04X, 0x%02X },\n",count,last);
	printf(	"    { 0,0 }\n"
			"};\n");
}
#endif
