/***************************************************************************


                            MCU Code Simulation

  CALC1 is a 40 pin DIP MCU of unknown type with unknown internal rom
  CALC3 is a NEC uPD78322 series MCU with 16K internal rom & 640 bytes of ram
TBSOP01 is a NEC uPD78324 series MCU with 32K internal rom & 1024 bytes of ram
TBSOP02 is likely the same NEC uPD78324 series MCU as the TBS0P01

Currently none of the MCUs' internal roms are dumped so simulation is used

***************************************************************************/

#include "driver.h"
#include "includes/kaneko16.h"

#include "kanekotb.h"	// TOYBOX MCU trojaning results

#define MCU_RESPONSE(d) memcpy(&kaneko16_mcu_ram[mcu_offset], d, sizeof(d))

UINT16 *kaneko16_mcu_ram;

/***************************************************************************
                                Gals Panic (set 2)
                                Gals Panic (set 3)
                                Sand Scorpion
                                Bonk's Adventure
                                Blood Warrior
***************************************************************************/

/*
    - see notes about this "calculator" implementation in drivers\galpanic.c
    - bonkadv only uses Random Number, XY Overlap Collision bit and register '0x02'
*/

static struct {
	UINT16 x1p, y1p, x1s, y1s;
	UINT16 x2p, y2p, x2s, y2s;

	INT16 x12, y12, x21, y21;

	UINT16 mult_a, mult_b;
} hit;

READ16_HANDLER(galpanib_calc_r) /* Simulation of the CALC1 MCU */
{
	UINT16 data = 0;

	switch (offset)
	{
		case 0x00/2: // watchdog
			return watchdog_reset_r(space,0);

		case 0x02/2: // unknown (yet!), used by *MANY* games !!!
			//popmessage("unknown collision reg");
			break;

		case 0x04/2: // similar to the hit detection from SuperNova, but much simpler

			// X Absolute Collision
			if      (hit.x1p >  hit.x2p)	data |= 0x0200;
			else if (hit.x1p == hit.x2p)	data |= 0x0400;
			else if (hit.x1p <  hit.x2p)	data |= 0x0800;

			// Y Absolute Collision
			if      (hit.y1p >  hit.y2p)	data |= 0x2000;
			else if (hit.y1p == hit.y2p)	data |= 0x4000;
			else if (hit.y1p <  hit.y2p)	data |= 0x8000;

			// XY Overlap Collision
			hit.x12 = (hit.x1p) - (hit.x2p + hit.x2s);
			hit.y12 = (hit.y1p) - (hit.y2p + hit.y2s);
			hit.x21 = (hit.x1p + hit.x1s) - (hit.x2p);
			hit.y21 = (hit.y1p + hit.y1s) - (hit.y2p);

			if ((hit.x12 < 0) && (hit.y12 < 0) &&
				(hit.x21 >= 0) && (hit.y21 >= 0))
					data |= 0x0001;

			return data;

		case 0x10/2:
			return (((UINT32)hit.mult_a * (UINT32)hit.mult_b) >> 16);
		case 0x12/2:
			return (((UINT32)hit.mult_a * (UINT32)hit.mult_b) & 0xffff);

		case 0x14/2:
			return (mame_rand(space->machine) & 0xffff);

		default:
			logerror("CPU #0 PC %06x: warning - read unmapped calc address %06x\n",cpu_get_pc(space->cpu),offset<<1);
	}

	return 0;
}

WRITE16_HANDLER(galpanib_calc_w)
{
	switch (offset)
	{
		// p is position, s is size
		case 0x00/2: hit.x1p    = data; break;
		case 0x02/2: hit.x1s    = data; break;
		case 0x04/2: hit.y1p    = data; break;
		case 0x06/2: hit.y1s    = data; break;
		case 0x08/2: hit.x2p    = data; break;
		case 0x0a/2: hit.x2s    = data; break;
		case 0x0c/2: hit.y2p    = data; break;
		case 0x0e/2: hit.y2s    = data; break;
		case 0x10/2: hit.mult_a = data; break;
		case 0x12/2: hit.mult_b = data; break;

		default:
			logerror("CPU #0 PC %06x: warning - write unmapped hit address %06x\n",cpu_get_pc(space->cpu),offset<<1);
	}
}

WRITE16_HANDLER(bloodwar_calc_w)
{
	switch (offset)
	{
		// p is position, s is size
		case 0x20/2: hit.x1p = data; break;
		case 0x22/2: hit.x1s = data; break;
		case 0x24/2: hit.y1p = data; break;
		case 0x26/2: hit.y1s = data; break;

		case 0x2c/2: hit.x2p = data; break;
		case 0x2e/2: hit.x2s = data; break;
		case 0x30/2: hit.y2p = data; break;
		case 0x32/2: hit.y2s = data; break;

		// this register is set to zero before any computation,
		// but it has no effect on inputs or result registers
		case 0x38/2: break;

		default:
			logerror("CPU #0 PC %06x: warning - write unmapped hit address %06x\n",cpu_get_pc(space->cpu),offset<<1);
	}
}

/*
 collision detection: absolute "distance", negative if no overlap
         [one inside other] | [ normal overlap ] | [   no overlap   ]
  rect1   <-------------->  |  <----------->     |  <--->
  rect2       <----->       |     <----------->  |             <--->
  result      <---------->  |     <-------->     |       <---->
*/
static INT16 calc_compute_x(void)
{
	INT16 x_coll;

	// X distance
	if ((hit.x2p >= hit.x1p) && (hit.x2p < (hit.x1p + hit.x1s)))		// x2p inside x1
		x_coll = (hit.x1s - (hit.x2p - hit.x1p));
	else if ((hit.x1p >= hit.x2p) && (hit.x1p < (hit.x2p + hit.x2s)))	// x1p inside x2
		x_coll = (hit.x2s - (hit.x1p - hit.x2p));
	else																// normal/no overlap
	 	x_coll = ((hit.x1s + hit.x2s)/2) - abs((hit.x1p + hit.x1s/2) - (hit.x2p + hit.x2s/2));

	return x_coll;
}
static INT16 calc_compute_y(void)
{
	INT16 y_coll;

	// Y distance
	if ((hit.y2p >= hit.y1p) && (hit.y2p < (hit.y1p + hit.y1s)))		// y2p inside y1
		y_coll = (hit.y1s - (hit.y2p - hit.y1p));
	else if ((hit.y1p >= hit.y2p) && (hit.y1p < (hit.y2p + hit.y2s)))	// y1p inside y2
		y_coll = (hit.y2s - (hit.y1p - hit.y2p));
	else																// normal/no overlap
		y_coll = ((hit.y1s + hit.y2s)/2) - abs((hit.y1p + hit.y1s/2) - (hit.y2p + hit.y2s/2));

	return y_coll;
}

READ16_HANDLER(bloodwar_calc_r)
{
	UINT16 data = 0;
	INT16 x_coll, y_coll;

	x_coll = calc_compute_x();
	y_coll = calc_compute_y();

	switch (offset)
	{
		case 0x00/2: // X distance
			return x_coll;

		case 0x02/2: // Y distance
			return y_coll;

		case 0x04/2: // similar to the hit detection from SuperNova, but much simpler

			// 4th nibble: Y Absolute Collision -> possible values = 9,8,4,3,2
			if      (hit.y1p >  hit.y2p)	data |= 0x2000;
			else if (hit.y1p == hit.y2p)	data |= 0x4000;
			else if (hit.y1p <  hit.y2p)	data |= 0x8000;
			if (y_coll<0) data |= 0x1000;

			// 3rd nibble: X Absolute Collision -> possible values = 9,8,4,3,2
			if      (hit.x1p >  hit.x2p)	data |= 0x0200;
			else if (hit.x1p == hit.x2p)	data |= 0x0400;
			else if (hit.x1p <  hit.x2p)	data |= 0x0800;
			if (x_coll<0) data |= 0x0100;

			// 2nd nibble: always set to 4
			data |= 0x0040;

			// 1st nibble: XY Overlap Collision -> possible values = 0,2,4,f
			if (x_coll>=0) data |= 0x0004;
			if (y_coll>=0) data |= 0x0002;
			if ((x_coll>=0)&&(y_coll>=0)) data |= 0x000F;

			return data;

		case 0x14/2:
			return (mame_rand(space->machine) & 0xffff);

		case 0x20/2: return hit.x1p;
		case 0x22/2: return hit.x1s;
		case 0x24/2: return hit.y1p;
		case 0x26/2: return hit.y1s;

		case 0x2c/2: return hit.x2p;
		case 0x2e/2: return hit.x2s;
		case 0x30/2: return hit.y2p;
		case 0x32/2: return hit.y2s;

		default:
			logerror("CPU #0 PC %06x: warning - read unmapped calc address %06x\n",cpu_get_pc(space->cpu),offset<<1);
	}

	return 0;
}

/***************************************************************************
                                CALC3 MCU:

                                Shogun Warriors
                                Fujiyama Buster
                                B.Rap Boys
***************************************************************************/
/*
---------------------------------------------------------------------------
                                CALC 3

92  B.Rap Boys              KANEKO CALC3 508 (74 PIN PQFP)
92  Shogun Warriors
    Fujiyama Buster         KANEKO CALC3 508 (74 Pin PQFP)
---------------------------------------------------------------------------

MCU Initialization command:

shogwarr: CPU #0 PC 00037A : MCU executed command: 00FF 0059 019E 030A FFFE 0042 0020 7FE0
fjbuster: CPU #0 PC 00037A : MCU executed command: 00FF 0059 019E 030A FFFE 0042 0020 7FE0
brapboys: CPU #0 PC 000BAE : MCU executed command: 00FF 00C2 0042 0830 082E 00C8 0020 0872

shogwarr/fjbuster:

00FF : busy flag, MCU clears it when cmd finished (main program loops until it's cleared)
0059 : MCU writes DSW -> $102e15
019E : Eeprom data address - dumps the 0x80 bytes of Eeprom here
030A : location where MCU will get its parameters from now on
FFFE : probably polled by MCU, needs to be kept alive (cleared by main cpu - IT2)
0042 : MCU writes its checksum
00207FE0 : base of 'stack' used when transfering tables out of MCU
*/

static void calc3_mcu_run(running_machine *machine);

static int calc3_mcu_status, calc3_mcu_command_offset;


void calc3_mcu_init(void)
{
	calc3_mcu_status = 0;
	calc3_mcu_command_offset = 0;
}

WRITE16_HANDLER( calc3_mcu_ram_w )
{
	COMBINE_DATA(&kaneko16_mcu_ram[offset]);
	calc3_mcu_run(space->machine);
}

#define CALC3_MCU_COM_W(_n_)				\
WRITE16_HANDLER( calc3_mcu_com##_n_##_w )	\
{											\
	logerror("calc3w %d %04x %04x\n",_n_, data,mem_mask); \
	calc3_mcu_status |= (1 << _n_);			\
	calc3_mcu_run(space->machine);					\
}

CALC3_MCU_COM_W(0)
CALC3_MCU_COM_W(1)
CALC3_MCU_COM_W(2)
CALC3_MCU_COM_W(3)

/***************************************************************************
                                Shogun Warriors
***************************************************************************/

/* Preliminary simulation: the game doesn't work */

/*
    MCU Tasks:

    - Read the DSWs
    - Supply code snippets to the 68000
*/


// protection information for brapboys / shogwarr
//
// this is just an analysis of the MCU Data rom, in reality it probably processes this when requested, not pre-decrypts
//

/*

esentially the data rom is a linked list of encrypted blocks

contains the following
ROM ADDRESS 0x0000 = the number of tables in this rom

followed by several tables in the following format

OFFSET 0 - the location of a word which specifies the size of the block
         - this is usually '3', but if it's larger than 3 it enables an 'inline encryption' mode, whereby the decryption table is stored in the
         - right before the length register

OFFSET 1 - a 'mode' register of some sort, usually 0,1,2 or 3 for used data, shogun also called a 'blank' command (length 0) with mode 8

OFFSET 2 - unknown, might be some kind of 'step' register

OFFSET 3 - decryption key - specifies which decryption table to use (ignored for inline tables, see offset 0)

(inline decryption table goes here if specified)

OFFSET 4-5 (or after the inline decryption table) - the length of the current block (so that the start of the next block can be found)

OFFSET 6-size - data for thie block

this continues for the number of blocks specified
after all the blocks there is a 0x1000 block of data which is the same between games

where games specify the same decryption key the table used is the same, I don't know where these tables come from.

*/

UINT16 calc3_mcu_crc;

/* decryption tables */

/* table 0x00 = not encrypted? */
static UINT8 calc3_table00[64] = {
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};

/* ok */
static UINT8 calc3_table01[64] = {
	0xce,0xab,0xa1,0xaa,0x20,0x20,0xcb,0x57,0x5b,0x65,0xd6,0x65,0x5e,0x92,0x6c,0x0a,
	0xf8,0xea,0xb6,0xfd,0x76,0x35,0x47,0xaf,0xed,0xe0,0xcc,0x4d,0x4c,0xd6,0x74,0x78,
	0x20,0x1c,0x1f,0xc1,0x25,0x2e,0x49,0xe7,0x90,0x1b,0xb4,0xcf,0x1e,0x61,0xd7,0x46,
	0xda,0x89,0x08,0x77,0xb1,0x81,0x6b,0x2d,0xb6,0xbc,0x99,0xc9,0x35,0x0a,0x0f,0x01
};

/* ok - shogwarr */
static UINT8 calc3_table15[64] = {
	0x0C,0xEB,0x30,0x25,0xA8,0xED,0xE3,0x23,0xAC,0x2B,0x8D,0x34,0x88,0x9F,0x55,0xB5,
	0xBF,0x15,0xE3,0x7C,0x54,0xD4,0x72,0xA9,0x7E,0x80,0x27,0x9F,0xA3,0x3F,0xA1,0x4D,
	0x84,0x1B,0xD1,0xB2,0xB5,0xA7,0x0C,0xA0,0x51,0xE6,0x5E,0xC0,0xEB,0x68,0x22,0xD6,
	0xC8,0xB6,0xCF,0x46,0x4B,0xF0,0x15,0xD7,0xB0,0xB5,0x29,0xB8,0xFD,0x43,0x5C,0xC0
};

/* ok - brapboys initial transfer */
static UINT8 calc3_table04[64] = {
    0xff,0x4a,0x62,0xa3,0xf4,0xb4,0x8c,0x2d,0x84,0xbd,0x87,0x3a,0x9e,0xe9,0xd7,0x12,
    0xff,0x50,0x40,0x39,0xa6,0x47,0xd9,0x38,0x89,0x3c,0x62,0xa0,0x86,0xe2,0xd7,0x4d,
	0x9c,0x8e,0x45,0xe8,0x5b,0xe1,0xdd,0xaf,0x99,0xa8,0x47,0x79,0x67,0x00,0x38,0xf4,
	0x43,0xbe,0x47,0x32,0x8f,0x0e,0xfd,0x64,0x42,0x59,0x2b,0xe5,0xde,0x6d,0x80,0x7a,
 };


/* partial guessed tables
unsigned char table31[64] = {
    // x     x     x    ok     x     x     x    ok     x     x     x    ok     x     x     x    ok
    0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0xaf, 0x00, 0x00, 0x00, 0x3a, 0x00, 0x00, 0x00, 0x3b,
    0x00, 0x00, 0x00, 0xbb, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0xb1, 0x00, 0x00, 0x00, 0xdc,
    0x00, 0x00, 0x00, 0xb5, 0x00, 0x00, 0x00, 0x3d, 0x00, 0x00, 0x00, 0x4f, 0x00, 0x00, 0x00, 0xde,
    0x00, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x26, 0x00, 0x00, 0x00, 0x34, 0x00, 0x00, 0x00, 0xb1
};


unsigned char tableb0[64] = {
    // x     x     x    ok     x     x     x    ok     x     x     x    ok     x     x     x    ok
    0x00, 0x00, 0x00, 0x5e, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00, 0x00, 0x68, 0x00, 0x00, 0x00, 0x81,
    0x00, 0x00, 0x00, 0x85, 0x00, 0x00, 0x00, 0x6d, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x32,
    0x00, 0x00, 0x00, 0x76, 0x00, 0x00, 0x00, 0xb2, 0x00, 0x00, 0x00, 0x2a, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xb2, 0x00, 0x00, 0x00, 0xe4, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x5b
};

unsigned char tableb7[64] = {
    // x     x     x    ok     x     x     x    ok     x     x     x    ok     x     x     x    ok
    0x00, 0x00, 0x00, 0x33, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x00, 0x00, 0xd7,
    0x00, 0x00, 0x00, 0xe7, 0x00, 0x00, 0x00, 0x54, 0x00, 0x00, 0x00, 0x2b, 0x00, 0x00, 0x00, 0x82,
    0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0xa2, 0x00, 0x00, 0x00, 0xb4, 0x00, 0x00, 0x00, 0x26,
    0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x9d, 0x00, 0x00, 0x00, 0x33
};

unsigned char tablebb[64] = {
    // x     x     x    ok     x     x     x    ok     x     x     x    ok     x     x     x    ok
    0x00, 0x00, 0x00, 0x62, 0x00, 0x00, 0x00, 0x4a, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x00, 0x00, 0x3d,
    0x00, 0x00, 0x00, 0xec, 0x00, 0x00, 0x00, 0x99, 0x00, 0x00, 0x00, 0xa0, 0x00, 0x00, 0x00, 0x69,
    0x00, 0x00, 0x00, 0xeb, 0x00, 0x00, 0x00, 0x6f, 0x00, 0x00, 0x00, 0x97, 0x00, 0x00, 0x00, 0x9e,
    0x00, 0x00, 0x00, 0xe1, 0x00, 0x00, 0x00, 0x9f, 0x00, 0x00, 0x00, 0xfd, 0x00, 0x00, 0x00, 0x4d
};
*/

// global so we can use them in the filename when we save out the data (debug..)
static UINT8 calc3_decryption_key_byte;
static UINT8 calc3_unknown;
static UINT8 calc3_mode;
static UINT8 calc3_blocksize_offset;
static UINT16 calc3_dataend;
static UINT16 calc3_database;

// endian safe? you're having a laugh
int calc3_decompress_table(running_machine* machine, int tabnum, UINT8* dstram, int dstoffset)
{
	UINT8* rom = memory_region(machine,"cpu1");
	UINT8 numregions;
	UINT16 length;
	int x;
	int offset = 0;
	numregions = rom[offset+0];

	if (tabnum > numregions)
	{
		printf("CALC3 error, requested table > num tables!\n");
		return 0;
	}

	rom++;

	// scan through the linked list to find the start of the requested table info
	for (x=0;x<tabnum;x++)
	{
		UINT8 blocksize_offset = rom[offset+0]; // location of the 'block length'
		offset+= blocksize_offset+1;
		length = rom[offset+0] | (rom[offset+1]<<8);
		offset+=length+2;
	}

	// we're at the start of the block, get the info about it
	{
		UINT16 inline_table_base = 0;
		UINT16 inline_table_size = 0;
		calc3_database = offset;
		calc3_blocksize_offset =    rom[offset+0]; // location of the 'block length'
		calc3_mode =                rom[offset+1];
		calc3_unknown =             rom[offset+2];
		calc3_decryption_key_byte = rom[offset+3];


		// if blocksize_offset > 3, it appears to specify the encryption table as 'inline' which can be of any size (odd or even) and loops over the bytes to decrypt
		// the decryption key specified seems to be ignored?
		if (calc3_blocksize_offset>3)
		{
			inline_table_base = offset+4;
			inline_table_size = calc3_blocksize_offset-3;
		}

		offset+= calc3_blocksize_offset+1;
		length = rom[offset+0] | (rom[offset+1]<<8);
		offset+=2;

		if (inline_table_size)
		{
			printf("Block %02x Found Base %04x - Inline Encryption (size %02x) - Mode? %02x Unknown %02x Key (unused?) %02x Length %04x\n", tabnum, calc3_database, inline_table_size, calc3_mode, calc3_unknown, calc3_decryption_key_byte, length);
		}
		else
		{
			printf("Block %02x Found Base %04x - Mode? %02x Unknown %02x Key %02x Length %04x\n", tabnum, calc3_database, calc3_mode, calc3_unknown, calc3_decryption_key_byte, length);
		}


		// copy + decrypt the table to the specified memory area
		if (dstram)
		{
			int i;

			if (length==0x00)
			{
				// shogwarr does this with 'mode' as 0x08, which probably has some special meaning
				//printf("CALC3: requested 0 length table!\n");
				// -- seems to be 'reset stack' to default for the protection table writes
			}

			if (inline_table_size)
			{
				for (i=0;i<length;i++)
				{
					UINT8 dat = rom[offset+i];
					UINT8 inlinet = rom[inline_table_base + (i%inline_table_size)];
					//printf("%02x, ",inlinet);
					dat -= inlinet;

					if (tabnum==0x40) // fjbuster / shogun warriors japanese character select (what enables this extra?)
					{
						// note the original, table is an odd number of words, so we can't just check if i is odd / even because the additional overlay
						// is relative to the start of the original table, and has a size of 0x11 (17) bytes which loop.
						if (((i%inline_table_size)&1)==0)
						{
							// thie gets mapped over half the inline table?!..  (inline length is 0x22, this changes odd bytes)
							// what specifies the additional overlay here?
							UINT8 extra[0x11] = { 0x14,0xf0,0xf8,0xd2,0xbe,0xfc,0xac,0x86,0x64,0x08,0x0c,0x74,0xd6,0x6a,0x24,0x12,0x1a };
							dat -= extra[(i%inline_table_size)>>1];

						}


					}

					dstram[(dstoffset+i)^1] = dat;
				}
			}
			else
			{
				UINT8* table = calc3_table00;

				// 0x00 seems to have no 'encryption'
				if (calc3_decryption_key_byte == 0x00) table = calc3_table00;

				// currently only handle 3 of the formats :-(
				if (calc3_decryption_key_byte == 0x01) table = calc3_table01;
				if (calc3_decryption_key_byte == 0x15) table = calc3_table15;
				if (calc3_decryption_key_byte == 0x04) table = calc3_table04;

				for (i=0;i<length;i++)
				{


					UINT8 dat = rom[offset+i];


					if (tabnum==0x80) // shogwarr table 80 (irq code)
					{

						unsigned char extracted80[686+2] = {
							0x00, 0x00,
							0xE7, 0x48, 0xFE, 0xFF, 0x39, 0x4A, 0x10, 0x00, 0xFE, 0x2D, 0x00, 0x67,
							0x64, 0x02, 0x39, 0x4A, 0x10, 0x00, 0x26, 0x2E, 0x00, 0x66, 0x54, 0x02,
							0x3C, 0x10, 0x4D, 0x00, 0x39, 0x4A, 0x10, 0x00, 0x10, 0x2E, 0x04, 0x67,
							0xC0, 0x08, 0x07, 0x00, 0xC0, 0x13, 0x90, 0x00, 0x00, 0x00, 0xF9, 0x43,
							0x58, 0x00, 0x00, 0x00, 0xF9, 0x41, 0x10, 0x00, 0x00, 0x18, 0xD8, 0x22,
							0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22,
							0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22,
							0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22,
							0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22,
							0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22,
							0xD8, 0x22, 0xF9, 0x43, 0x38, 0x00, 0x00, 0x00, 0x3C, 0x30, 0x0F, 0x00,
							0x79, 0x20, 0x10, 0x00, 0xA2, 0x62, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22,
							0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22,
							0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22,
							0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22,
							0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22,
							0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22,
							0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22,
							0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22,
							0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22,
							0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22,
							0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22,
							0xD8, 0x22, 0xC8, 0x51, 0x7E, 0xFF, 0x39, 0x4A, 0x10, 0x00, 0x27, 0x2E,
							0x00, 0x66, 0x64, 0x00, 0xF9, 0x45, 0x10, 0x00, 0x40, 0x32, 0xF9, 0x41,
							0x80, 0x00, 0x00, 0x00, 0xDA, 0x30, 0xDA, 0x30, 0xDA, 0x30, 0xDA, 0x30,
							0xF9, 0x41, 0x90, 0x00, 0x14, 0x00, 0x39, 0x30, 0x10, 0x00, 0x38, 0x32,
							0x40, 0x02, 0xC0, 0xFF, 0x40, 0x44, 0xC0, 0x30, 0x39, 0x30, 0x10, 0x00,
							0x3A, 0x32, 0x40, 0x02, 0xC0, 0xFF, 0x40, 0x44, 0x79, 0xD0, 0x10, 0x00,
							0x1E, 0x2E, 0xC0, 0x30, 0x39, 0x30, 0x10, 0x00, 0x3C, 0x32, 0x40, 0x02,
							0xC0, 0xFF, 0x40, 0x44, 0xC0, 0x30, 0x39, 0x30, 0x10, 0x00, 0x3E, 0x32,
							0x40, 0x02, 0xC0, 0xFF, 0x40, 0x44, 0x79, 0xD0, 0x10, 0x00, 0x1E, 0x2E,
							0xC0, 0x30, 0x00, 0x60, 0x9A, 0x00, 0xF9, 0x41, 0x80, 0x00, 0x00, 0x00,
							0x39, 0x30, 0x10, 0x00, 0x40, 0x32, 0x40, 0x31, 0x02, 0x00, 0x40, 0x31,
							0x06, 0x00, 0xF9, 0x41, 0x90, 0x00, 0x1C, 0x00, 0x39, 0x30, 0x10, 0x00,
							0x34, 0x32, 0x48, 0xED, 0x40, 0x44, 0xC0, 0x30, 0x39, 0x30, 0x10, 0x00,
							0x30, 0x32, 0x40, 0x06, 0x20, 0x00, 0x48, 0xED, 0x40, 0x44, 0x79, 0xD0,
							0x10, 0x00, 0x1E, 0x2E, 0xC0, 0x30, 0xF9, 0x41, 0x10, 0x00, 0x28, 0x2E,
							0xE8, 0x45, 0x00, 0x02, 0x79, 0x22, 0x10, 0x00, 0x22, 0x2E, 0xE9, 0x47,
							0x00, 0x10, 0x3C, 0x30, 0x07, 0x00, 0xD8, 0x22, 0xDA, 0x26, 0xD8, 0x22,
							0xDA, 0x26, 0xD8, 0x22, 0xDA, 0x26, 0xD8, 0x22, 0xDA, 0x26, 0xD8, 0x22,
							0xDA, 0x26, 0xD8, 0x22, 0xDA, 0x26, 0xD8, 0x22, 0xDA, 0x26, 0xD8, 0x22,
							0xDA, 0x26, 0xD8, 0x22, 0xDA, 0x26, 0xD8, 0x22, 0xDA, 0x26, 0xD8, 0x22,
							0xDA, 0x26, 0xD8, 0x22, 0xDA, 0x26, 0xD8, 0x22, 0xDA, 0x26, 0xD8, 0x22,
							0xDA, 0x26, 0xD8, 0x22, 0xDA, 0x26, 0xD8, 0x22, 0xDA, 0x26, 0xC8, 0x51,
							0xBE, 0xFF, 0xF9, 0x43, 0x58, 0x00, 0x80, 0x00, 0xF9, 0x41, 0x10, 0x00,
							0x80, 0x18, 0x3C, 0x30, 0x1B, 0x00, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22,
							0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22,
							0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22,
							0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22,
							0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22,
							0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xD8, 0x22, 0xC8, 0x51,
							0xBE, 0xFF, 0xD8, 0x22, 0xD8, 0x22, 0x39, 0x42, 0x10, 0x00, 0xFE, 0x2D,
							0xF9, 0x41, 0x10, 0x00, 0x6C, 0x67, 0x07, 0x70, 0x10, 0x0C, 0x03, 0x00,
							0x00, 0x66, 0x0E, 0x00, 0x68, 0x53, 0x0E, 0x00, 0x00, 0x66, 0x06, 0x00,
							0xBC, 0x10, 0x04, 0x00, 0xE8, 0x41, 0x12, 0x00, 0xC8, 0x51, 0xE6, 0xFF,
							0xC0, 0x33, 0x2C, 0x00, 0x00, 0x00, 0x3C, 0x30, 0x14, 0x00, 0xC8, 0x51,
							0xFE, 0xFF, 0xC0, 0x33, 0x2D, 0x00, 0x00, 0x00, 0xDF, 0x4C, 0xFF, 0x7F,
							0x73, 0x4E
						};

						if ((i&1)==0)
						{
							dat -= table[i&0x3f];
							dat = BITSWAP8(dat,2,1,0,7,6,5,4,3);
							// we can decode this byte... but compare it to the extract table for verification
							if (extracted80[i^1] != dat) printf("error %02x %02x %02x\n", i, extracted80[i^1],dat);
						}
						else
						{
							// I don't understand how to decode this byte :-(  - rotation + extra table?
							dat = extracted80[i^1];
						}
					}
					else if (tabnum==0x41) // shogwarr table 41  -- note fjbuster uses table 40, which looks like (almost) the same data but with better encryption...
					{
						// simple shifts...  different direction for each alternating byte..
						dat -= table[i&0x3f];

						if ((i&1)==1) dat = BITSWAP8(dat,0,7,6,5,4,3,2,1);
						else dat = BITSWAP8(dat,6,5,4,3,2,1,0,7);
					}
					else
					{
						dat -= table[i&0x3f];
					}


					dstram[(dstoffset+i)^1] = dat;
				}
			}
		}

		calc3_dataend = offset+length+1;
	}

	//printf("data base %04x data end %04x\n", calc3_database, calc3_dataend);

	return length;

}

static UINT8 calc3_decryption_key_byte;
static UINT8 calc3_unknown;
static UINT8 calc3_mode;
static UINT8 calc3_blocksize_offset;

DRIVER_INIT(calc3_scantables)
{
	UINT8* rom = memory_region(machine,"cpu1");
	UINT8 numregions;

	int x;

	calc3_mcu_crc = 0;
	for (x=0;x<0x20000;x++)
	{
		calc3_mcu_crc+=rom[x];
	}
	printf("crc %04x\n",calc3_mcu_crc);
	numregions = rom[0];

	for (x=0;x<numregions;x++)
	{
		UINT8* tmpdstram = malloc(0x2000);
		int length;
		memset(tmpdstram, 0x00,0x2000);
		length = calc3_decompress_table(machine, x, tmpdstram, 0);

		// dump to file
		if (length)
		{
			FILE *fp;
			char filename[256];

			if (calc3_blocksize_offset==3)
			{
				sprintf(filename,"data_%s_table_%04x k%02x m%02x u%02x length %04x",
						machine->gamedrv->name,
						x, calc3_decryption_key_byte, calc3_mode, calc3_unknown, length);
			}
			else
			{
				sprintf(filename,"data_%s_table_%04x k%02x (use indirect size %02x) m%02x u%02x length %04x",
					machine->gamedrv->name,
					x, calc3_decryption_key_byte, calc3_blocksize_offset-3, calc3_mode, calc3_unknown, length);
			}

			fp=fopen(filename, "w+b");
			if (fp)
			{
				fwrite(tmpdstram, length, 1, fp);
				fclose(fp);
			}
		}

		free(tmpdstram);
	}

	// there is also a 0x1000 block of data at the end.. same on both games, maybe it's related to the decryption tables??
	// the calc3_dataend points to the data after the last block processed, as we process all the blocks in the above loop, we assume this points
	// to that extra block of data

	// dump out the 0x1000 sized block at the end
	{
		FILE *fp;
		char filename[256];

		sprintf(filename,"data_%s_finalblock",
		machine->gamedrv->name);

		fp=fopen(filename, "w+b");
		if (fp)
		{
			fwrite(&rom[calc3_dataend], 0x1000, 1, fp);
			fclose(fp);
		}
	}
}

UINT32 calc3_writeaddress;
UINT16 calc3_dsw_addr;
UINT16 calc3_eeprom_addr;
UINT16 calc3_poll_addr;
UINT16 cakc3_checkumaddress;

extern UINT16 calc3_mcu_crc;
extern UINT16* kaneko16_calc3_fakeram;
// from brap boys, might be polluted with shogun warriors values tho as was running shogun code at the time
static UINT16 kaneko16_eeprom_data[0x40] =
{
	0x0001, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	0x0000, 0x0000,
	0x0000, 0x0003, 0x0002, 0x1020, 0x0002, 0x6010, 0x0101, 0x0101,
	0x0101, 0x0001,	0x0003, 0x0008, 0x4B41, 0x4E45, 0x4B4F, 0x2020,
	0x4265, 0x2052, 0x6170, 0x2042, 0x6F79, 0x7300,	0x3030, 0x302E,
	0x3038, 0x7FFF, 0xFFFF, 0xFFFF,	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0xFFFF, 0xFFFF,	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,	0xFFFF, 0xFFFF,
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,	0x003B, 0xFFFF, 0xFFFF, 0xFFFF
};

static void calc3_mcu_run(running_machine *machine)
{
	UINT16 mcu_command;
	int i;

	if ( calc3_mcu_status != (1|2|4|8) )	return;

	//calc3_mcu_status = 0;

	mcu_command = kaneko16_mcu_ram[calc3_mcu_command_offset/2 + 0];

	if (mcu_command == 0) return;

	logerror("%s : MCU executed command at %04X: %04X\n",
	 	cpuexec_describe_context(machine),calc3_mcu_command_offset,mcu_command);


	if (mcu_command>0)
	{
		/* 0xff is a special 'init' command */
		if (mcu_command == 0xff)
		{
			// clear old command (handshake to main cpu)
			kaneko16_mcu_ram[(calc3_mcu_command_offset>>1)+0] = 0x0000;


			calc3_dsw_addr =           kaneko16_mcu_ram[(0>>1) + 1];
			calc3_eeprom_addr =        kaneko16_mcu_ram[(0>>1) + 2];
			calc3_mcu_command_offset = kaneko16_mcu_ram[(0>>1) + 3];
			calc3_poll_addr =          kaneko16_mcu_ram[(0>>1) + 4];
			cakc3_checkumaddress =     kaneko16_mcu_ram[(0>>1) + 5];
			calc3_writeaddress =      (kaneko16_mcu_ram[(0>>1) + 6] << 16) |
			                          (kaneko16_mcu_ram[(0>>1) + 7]);

			printf("Calc 3 Init Command - %04x DSW addr\n",  calc3_dsw_addr);
			printf("Calc 3 Init Command - %04x Eeprom Address\n",  calc3_eeprom_addr);
			printf("Calc 3 Init Command - %04x Future Commands Base\n",  calc3_mcu_command_offset);
			printf("Calc 3 Init Command - %04x Poll / Busy Address\n",  calc3_poll_addr);
			printf("Calc 3 Init Command - %04x ROM Checksum Address\n",  cakc3_checkumaddress);
			printf("Calc 3 Init Command - %08x Data Write Address\n",  calc3_writeaddress);

			kaneko16_mcu_ram[calc3_dsw_addr/2] = ~input_port_read(machine, "DSW1");	// DSW
			kaneko16_mcu_ram[cakc3_checkumaddress / 2] = calc3_mcu_crc;				// MCU Rom Checksum!

			for (i=0;i<0x40;i++)
			{
				kaneko16_mcu_ram[(calc3_eeprom_addr / 2)+i] = kaneko16_eeprom_data[i];//((eepromData[i]&0xff00)>>8) |  ((eepromData[i]&0x00ff)<<8);
			}

		}
		/* otherwise the command number is the number of transfer operations to perform */
		else
		{
			int num_transfers = mcu_command;
			int i;

			// clear old command (handshake to main cpu)
			kaneko16_mcu_ram[calc3_mcu_command_offset>>1] = 0x0000;;

			logerror("Calc3 transfer request, %d transfers\n", num_transfers);

			for (i=0;i<num_transfers;i++)
			{
				int param1 = kaneko16_mcu_ram[(calc3_mcu_command_offset>>1) + 1 + (2*i)];
				int param2 = kaneko16_mcu_ram[(calc3_mcu_command_offset>>1) + 2 + (2*i)];
				UINT8  commandtabl = (param1&0xff00) >> 8;
				UINT16 commandaddr = (param1&0x00ff) | (param2&0xff00);
				UINT8  commandunk =  (param2&0x00ff); // brap boys sets.. seems to cause further writebasck address displacement??

				UINT32 fakeoffs;

				printf("transfer %d table %02x writeback address %04x unknown %02x\n", i, commandtabl, commandaddr, commandunk);

				// the data SHOULD be written somewhere to main ram, probably related to the calc3_writeaddress set in command 0xff
				// but I'm not sure how, for now write it back to a FAKE region instead
				fakeoffs = 0x1e00*commandtabl;
				fakeoffs+=0xf00000;

				// HACK HACK HACK, I don't know what commandunk does, but Brap Boys polls a table of addresses and doesn't do anything
				// more unless they change.  The usual writeback address doesn't touch this table, so I think this unknown value must
				// cause an additional displacement to cause the writeback address to populate that table.. maybe
				if (commandunk == 0x30)
				{
					commandaddr -= 0x60;
					calc3_decompress_table(machine, commandtabl, (UINT8*)kaneko16_mcu_ram, (calc3_writeaddress&0xffff)-2);

					printf("writing back address %08x to %08x\n", calc3_writeaddress, commandaddr);
					kaneko16_mcu_ram[(commandaddr>>1)+0] = (calc3_writeaddress>>16)&0xffff;
					kaneko16_mcu_ram[(commandaddr>>1)+1] = (calc3_writeaddress&0xffff);

				}
				else
				{
					calc3_decompress_table(machine, commandtabl, (UINT8*)kaneko16_calc3_fakeram, fakeoffs&0xfffff);

					// write back WHERE we wrote the data to the address specified so that the code can jump to it etc.
					fakeoffs+=2;// the first 2 bytes don't seem to be the offset it expects to jump to..

					printf("writing back fake address %08x to %08x\n", fakeoffs, commandaddr);
					kaneko16_mcu_ram[(commandaddr>>1)+0] = (fakeoffs>>16)&0xffff;
					kaneko16_mcu_ram[(commandaddr>>1)+1] = (fakeoffs&0xffff);
				}
			}


		}
	}
}


/***************************************************************************
                                TOYBOX MCU:

                                Bonk's Adventure
                                Blood Warrior
                                Great 1000 Miles Rally
                                ...
***************************************************************************/
/*
---------------------------------------------------------------------------
                                TOYBOX

94  Bonk's Adventure            TOYBOX?            TBSOP01
94  Blood Warrior               TOYBOX?            TBS0P01 452 9339PK001
94  Great 1000 Miles Rally      TOYBOX                                                  "MM0525-TOYBOX199","USMM0713-TB1994 "
95  Great 1000 Miles Rally 2    TOYBOX      KANEKO TBSOP02 454 9451MK002 (74 pin PQFP)  "USMM0713-TB1994 "
95  Jackie Chan                 TOYBOX                                                  "USMM0713-TB1994 "
95  Gals Panic 3                TOYBOX?            TBSOP01
---------------------------------------------------------------------------

All the considerations are based on the analysis of jchan, and to a fewer extent galpani3, and make references to the current driver sources:

MCU triggering:
---------------

the 4 JCHAN_MCU_COM_W(...) are in fact 2 groups:

AM_RANGE(0x330000, 0x330001) AM_WRITE(jchan_mcu_com0_w) // _[ these 2 are set to 0xFFFF
AM_RANGE(0x340000, 0x340001) AM_WRITE(jchan_mcu_com1_w) //  [ for MCU to execute cmd

AM_RANGE(0x350000, 0x350001) AM_WRITE(jchan_mcu_com2_w) // _[ these 2 are set to 0xFFFF
AM_RANGE(0x360000, 0x360001) AM_WRITE(jchan_mcu_com3_w) //  [ for MCU to return its status


MCU parameters:
---------------

mcu_command = kaneko16_mcu_ram[0x0010/2];    // command nb
mcu_offset  = kaneko16_mcu_ram[0x0012/2]/2;  // offset in shared RAM where MCU will write
mcu_subcmd  = kaneko16_mcu_ram[0x0014/2];    // sub-command parameter, happens only for command #4


    the only MCU commands found in program code are:
    - 0x04: protection: provide data (see below) and code <<<---!!!
    - 0x03: read DSW
    - 0x02: load game settings \ stored in ATMEL AT93C46 chip,
    - 0x42: save game settings / 128 bytes serial EEPROM


Current feeling of devs is that this EEPROM might also play a role in the protection scheme,
but I (SV) feel that it is very unlikely because of the following, which has been verified:
if the checksum test fails at most 3 times, then the initial settings, stored in main68k ROM,
are loaded in RAM then saved with cmd 0x42 (see code @ $5196 & $50d4)
Note that this is valid for jchan only, other games haven't been looked at.

Others:
-------

There is one interesting MCU cmd $4 in jchan:
-> sub-cmd $3d, MCU writes the string "USMM0713-TB1994 "

The very same string is written by gtmr games (gtmre/gtmrusa/gtmr2) but apparently with no sub-cmd: this string is
probably the MCU model string, so this one should be in internal MCU ROM (another one for gtmr is "MM0525-TOYBOX199")

TODO: look at this one since this remark is only driver-based.
*/


static const UINT8 toybox_mcu_decryption_table[0x100] = {
0x7b,0x82,0xf0,0xbc,0x7f,0x1d,0xa2,0xc5,0x2a,0xfa,0x55,0xee,0x1a,0xd0,0x59,0x76,
0x5e,0x75,0x79,0x16,0xa5,0xf6,0x84,0xed,0x0f,0x2e,0xf2,0x36,0x61,0xac,0xcd,0xab,
0x01,0x3b,0x01,0x87,0x73,0xab,0xce,0x5d,0xd4,0x1d,0x68,0x2a,0x35,0xea,0x13,0x27,
0x00,0xaa,0x46,0x36,0x6e,0x65,0x80,0x7e,0x19,0xe2,0x96,0xab,0xac,0xa5,0x6c,0x63,
0x4a,0x6f,0x87,0xf6,0x6a,0xac,0x38,0xe2,0x1f,0x87,0xf9,0xaa,0xf5,0x41,0x60,0xa6,
0x42,0xb9,0x30,0xf2,0xc3,0x1c,0x4e,0x4b,0x08,0x10,0x42,0x32,0xbf,0xb2,0xc5,0x0f,
0x7a,0xab,0x97,0xf6,0xe7,0xb3,0x46,0xf8,0xec,0x2b,0x7d,0x5f,0xb1,0x10,0x03,0xe4,
0x0f,0x22,0xdf,0x8d,0x10,0x66,0xa7,0x7e,0x96,0xbd,0x5a,0xaf,0xaa,0x43,0xdf,0x10,
0x7c,0x04,0xe2,0x9d,0x66,0xd7,0xf0,0x02,0x58,0x8a,0x55,0x17,0x16,0xe2,0xe2,0x52,
0xaf,0xd9,0xf9,0x0d,0x59,0x70,0x86,0x3c,0x05,0xd1,0x52,0xa7,0xf0,0xbf,0x17,0xd0,
0x23,0x15,0xfe,0x23,0xf2,0x80,0x60,0x6f,0x95,0x89,0x67,0x65,0xc9,0x0e,0xfc,0x16,
0xd6,0x8a,0x9f,0x25,0x2c,0x0f,0x2d,0xe4,0x51,0xb2,0xa8,0x18,0x3a,0x5d,0x66,0xa0,
0x9f,0xb0,0x58,0xea,0x78,0x72,0x08,0x6a,0x90,0xb6,0xa4,0xf5,0x08,0x19,0x60,0x4e,
0x92,0xbd,0xf1,0x05,0x67,0x4f,0x24,0x99,0x69,0x1d,0x0c,0x6d,0xe7,0x74,0x88,0x22,
0x2d,0x15,0x7a,0xa2,0x37,0xa9,0xa0,0xb0,0x2c,0xfb,0x27,0xe5,0x4f,0xb6,0xcd,0x75,
0xdc,0x39,0xce,0x6f,0x1f,0xfe,0xcc,0xb5,0xe6,0xda,0xd8,0xee,0x85,0xee,0x2f,0x04,
};


static const UINT8 toybox_mcu_decryption_table_alt[0x100] = {
0x26,0x17,0xb9,0xcf,0x1a,0xf5,0x14,0x1e,0x0c,0x35,0xb3,0x66,0xa0,0x17,0xe9,0xe4,
0x90,0xf6,0xd5,0x35,0xac,0x95,0x49,0x43,0x64,0x0c,0x03,0x75,0x4d,0xda,0xb6,0xdf,
0x06,0xcf,0x83,0x9e,0x35,0x2c,0x71,0x2a,0xab,0xcc,0x65,0xd4,0x1f,0xb0,0x88,0x3c,
0xb7,0x87,0x35,0xc0,0x41,0x65,0x9f,0xa0,0xd5,0x8c,0x3e,0x06,0x53,0xdb,0x45,0x64,
0x09,0x1e,0xc5,0x8d,0x50,0x24,0xe2,0x4a,0x9b,0x99,0x77,0x25,0x43,0xa9,0x1d,0xac,
0x99,0x31,0x75,0xb5,0x53,0xab,0xad,0x5a,0x42,0x14,0xa1,0x52,0xac,0xec,0x5f,0xf8,
0x8c,0x78,0x05,0x47,0xea,0xb8,0xde,0x69,0x98,0x2d,0x8f,0x9d,0xfc,0x05,0xea,0xee,
0x77,0xbb,0xa9,0x31,0x01,0x00,0xea,0xd8,0x9c,0x43,0xb5,0x2f,0x4e,0xb5,0x1b,0xd2,
0x01,0x4b,0xc4,0xf8,0x76,0x92,0x59,0x4f,0x20,0x52,0xd9,0x7f,0xa9,0x19,0xe9,0x7c,
0x8d,0x3b,0xec,0xe0,0x60,0x08,0x2e,0xbd,0x27,0x8b,0xb2,0xfc,0x29,0xd8,0x39,0x8a,
0x4f,0x2f,0x6b,0x04,0x10,0xbd,0xa1,0x04,0xde,0xc0,0xd5,0x0f,0x04,0x86,0xd6,0xd8,
0xfd,0xb1,0x3c,0x4c,0xd1,0xc4,0xf1,0x5b,0xf5,0x8b,0xe3,0xc4,0x89,0x3c,0x39,0x86,
0xd2,0x92,0xc9,0xe5,0x2c,0x4f,0xe2,0x2f,0x2d,0xc5,0x35,0x09,0x94,0x47,0x3c,0x04,
0x40,0x8b,0x57,0x08,0xf6,0x74,0xe9,0xb8,0x36,0x4d,0xc5,0x26,0x13,0x3d,0x75,0xa0,
0xa8,0x29,0x09,0x8c,0x87,0xf7,0x13,0xaf,0x4c,0x38,0x0b,0x8a,0x7f,0x2c,0x62,0x27,
0x47,0xaa,0xda,0x07,0x92,0x8d,0xfd,0x1f,0xee,0x48,0x1a,0x53,0x3b,0x98,0x6a,0x72,
};

// I use a byteswapped MCU data rom to make the transfers to the 68k side easier
//  not sure if it's all 100% endian safe
DRIVER_INIT( decrypt_toybox_rom )
{

	UINT8 *src = (UINT8 *)memory_region(machine, "mcudata" );

	int i;

	for (i=0;i<0x020000;i++)
	{
		src[i] = src[i] + toybox_mcu_decryption_table[(i^1)&0xff];
	}

	#if 0
	{
		FILE *fp;
		char filename[256];
		sprintf(filename,"%s.mcudata", machine->gamedrv->name);
		fp=fopen(filename, "w+b");
		if (fp)
		{
			fwrite(src, 0x20000, 1, fp);
			fclose(fp);
		}
	}
	#endif
}

DRIVER_INIT( decrypt_toybox_rom_alt )
{

	UINT8 *src = (UINT8 *)memory_region(machine, "mcudata" );

	int i;

	for (i=0;i<0x020000;i++)
	{
		src[i] = src[i] + toybox_mcu_decryption_table_alt[(i^1)&0xff];
	}
}

void toxboy_handle_04_subcommand(running_machine* machine,UINT8 mcu_subcmd, UINT16*mcu_ram)
{
	UINT8 *src = (UINT8 *)memory_region(machine, "mcudata")+0x10000;
	UINT8* dst = (UINT8 *)mcu_ram;

	int offs = (mcu_subcmd&0x3f)*8;
	int x;

	//UINT16 unused = src[offs+0] | (src[offs+1]<<8);
	UINT16 romstart = src[offs+2] | (src[offs+3]<<8);
	UINT16 romlength = src[offs+4] | (src[offs+5]<<8);
	UINT16 ramdest = mcu_ram[0x0012/2];
	//UINT16 extra = src[offs+6] | (src[offs+7]<<8); // BONK .. important :-(

	//printf("romstart %04x length %04x\n",romstart,romlength);

	for (x=0;x<romlength;x++)
	{
		dst[BYTE_XOR_LE(ramdest+x)] = src[(romstart+x)];
	}
}


void (*toybox_mcu_run)(running_machine *machine);

static UINT16 toybox_mcu_com[4];

void toybox_mcu_init(void)
{
	memset(toybox_mcu_com, 0, 4 * sizeof( UINT16) );
}

#define TOYBOX_MCU_COM_W(_n_)							\
WRITE16_HANDLER( toybox_mcu_com##_n_##_w )				\
{														\
	COMBINE_DATA(&toybox_mcu_com[_n_]);					\
	if (toybox_mcu_com[0] != 0xFFFF)	return;			\
	if (toybox_mcu_com[1] != 0xFFFF)	return;			\
	if (toybox_mcu_com[2] != 0xFFFF)	return;			\
	if (toybox_mcu_com[3] != 0xFFFF)	return;			\
														\
	memset(toybox_mcu_com, 0, 4 * sizeof( UINT16 ) );	\
	toybox_mcu_run(space->machine);							\
}

TOYBOX_MCU_COM_W(0)
TOYBOX_MCU_COM_W(1)
TOYBOX_MCU_COM_W(2)
TOYBOX_MCU_COM_W(3)

/*
    bonkadv and bloodwar test bit 0
*/
READ16_HANDLER( toybox_mcu_status_r )
{
	logerror("CPU %s (PC=%06X) : read MCU status\n", space->cpu->tag, cpu_get_previouspc(space->cpu));
	return 0; // most games test bit 0 for failure
}


/***************************************************************************
                                Blood Warrior
***************************************************************************/

void bloodwar_mcu_run(running_machine *machine)
{
	UINT16 mcu_command	=	kaneko16_mcu_ram[0x0010/2];
	UINT16 mcu_offset	=	kaneko16_mcu_ram[0x0012/2] / 2;
	UINT16 mcu_data		=	kaneko16_mcu_ram[0x0014/2];

	switch (mcu_command >> 8)
	{
		case 0x02:	// Read from NVRAM
		{
			mame_file *f;
			if ((f = nvram_fopen(machine, OPEN_FLAG_READ)) != 0)
			{
				mame_fread(f,&kaneko16_mcu_ram[mcu_offset], 128);
				mame_fclose(f);
			}
			logerror("%s : MCU executed command: %04X %04X (load NVRAM settings)\n", cpuexec_describe_context(machine), mcu_command, mcu_offset*2);
		}
		break;

		case 0x42:	// Write to NVRAM
		{
			mame_file *f;
			if ((f = nvram_fopen(machine, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS)) != 0)
			{
				mame_fwrite(f,&kaneko16_mcu_ram[mcu_offset], 128);
				mame_fclose(f);
			}
			logerror("%s : MCU executed command: %04X %04X (save NVRAM settings)\n", cpuexec_describe_context(machine), mcu_command, mcu_offset*2);
		}
		break;

		case 0x03:	// DSW
		{
			kaneko16_mcu_ram[mcu_offset] = input_port_read(machine, "DSW1");
			logerror("%s : MCU executed command: %04X %04X (read DSW)\n", cpuexec_describe_context(machine), mcu_command, mcu_offset*2);
		}
		break;

		case 0x04:	// Protection
		{
			logerror("%s : MCU executed command: %04X %04X %04X\n", cpuexec_describe_context(machine), mcu_command, mcu_offset*2, mcu_data);

			toxboy_handle_04_subcommand(machine, mcu_data, kaneko16_mcu_ram);

		}
		break;

		default:
			logerror("%s : MCU executed command: %04X %04X %04X (UNKNOWN COMMAND)\n", cpuexec_describe_context(machine), mcu_command, mcu_offset*2, mcu_data);
		break;
	}
}

/***************************************************************************
                                Bonk's Adventure
***************************************************************************/

void bonkadv_mcu_run(running_machine *machine)
{
	UINT16 mcu_command	=	kaneko16_mcu_ram[0x0010/2];
	UINT16 mcu_offset	=	kaneko16_mcu_ram[0x0012/2] / 2;
	UINT16 mcu_data		=	kaneko16_mcu_ram[0x0014/2];

	switch (mcu_command >> 8)
	{

		case 0x02:	// Read from NVRAM
		{
			mame_file *f;
			if ((f = nvram_fopen(machine, OPEN_FLAG_READ)) != 0)
			{
				mame_fread(f,&kaneko16_mcu_ram[mcu_offset], 128);
				mame_fclose(f);
			}
			logerror("%s : MCU executed command: %04X %04X (load NVRAM settings)\n", cpuexec_describe_context(machine), mcu_command, mcu_offset*2);
		}
		break;

		case 0x42:	// Write to NVRAM
		{
			mame_file *f;
			if ((f = nvram_fopen(machine, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS)) != 0)
			{
				mame_fwrite(f,&kaneko16_mcu_ram[mcu_offset], 128);
				mame_fclose(f);
			}
			logerror("%s : MCU executed command: %04X %04X (save NVRAM settings)\n", cpuexec_describe_context(machine), mcu_command, mcu_offset*2);
		}
		break;

		case 0x43:	// Initialize NVRAM - MCU writes Default Data Set directly to NVRAM
		{
			mame_file *f;
			if ((f = nvram_fopen(machine, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS)) != 0)
			{
				mame_fwrite(f, bonkadv_mcu_43, sizeof(bonkadv_mcu_43));
				mame_fclose(f);
			}
			logerror("%s : MCU executed command: %04X %04X (restore default NVRAM settings)\n", cpuexec_describe_context(machine), mcu_command, mcu_offset*2);
		}
		break;

		case 0x03:	// DSW
		{
			kaneko16_mcu_ram[mcu_offset] = input_port_read(machine, "DSW1");
			logerror("%s : MCU executed command: %04X %04X (read DSW)\n", cpuexec_describe_context(machine), mcu_command, mcu_offset*2);
		}
		break;

		case 0x04:	// Protection
		{
			logerror("%s : MCU executed command: %04X %04X %04X\n", cpuexec_describe_context(machine), mcu_command, mcu_offset*2, mcu_data);


			switch(mcu_data)
			{
				// static, in this order, at boot/reset - these aren't understood, different params in Mcu data rom, data can't be found
				case 0x34: MCU_RESPONSE(bonkadv_mcu_4_34); break;
				case 0x30: MCU_RESPONSE(bonkadv_mcu_4_30); break;
				case 0x31: MCU_RESPONSE(bonkadv_mcu_4_31); break;
				case 0x32: MCU_RESPONSE(bonkadv_mcu_4_32); break;
				case 0x33: MCU_RESPONSE(bonkadv_mcu_4_33); break;

				// dynamic, per-level (29), in level order
				default:
					toxboy_handle_04_subcommand(machine, mcu_data, kaneko16_mcu_ram);
					break;

			}
		}
		break;

		default:
			logerror("%s : MCU executed command: %04X %04X %04X (UNKNOWN COMMAND)\n", cpuexec_describe_context(machine), mcu_command, mcu_offset*2, mcu_data);
		break;
	}
}

/***************************************************************************
                            Great 1000 Miles Rally
***************************************************************************/

/*
    MCU Tasks:

    - Write and ID string to shared RAM.
    - Access the EEPROM
    - Read the DSWs
*/

void gtmr_mcu_run(running_machine *machine)
{
	UINT16 mcu_command	=	kaneko16_mcu_ram[0x0010/2];
	UINT16 mcu_offset	=	kaneko16_mcu_ram[0x0012/2] / 2;
	UINT16 mcu_data		=	kaneko16_mcu_ram[0x0014/2];

	logerror("%s : MCU executed command: %04X %04X %04X\n", cpuexec_describe_context(machine), mcu_command, mcu_offset*2, mcu_data);

	switch (mcu_command >> 8)
	{

		case 0x02:	// Read from NVRAM
		{
			mame_file *f;
			if ((f = nvram_fopen(machine, OPEN_FLAG_READ)) != 0)
			{
				mame_fread(f,&kaneko16_mcu_ram[mcu_offset], 128);
				mame_fclose(f);
			}
		}
		break;

		case 0x42:	// Write to NVRAM
		{
			mame_file *f;
			if ((f = nvram_fopen(machine, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS)) != 0)
			{
				mame_fwrite(f,&kaneko16_mcu_ram[mcu_offset], 128);
				mame_fclose(f);
			}
		}
		break;

		case 0x03:	// DSW
		{
			kaneko16_mcu_ram[mcu_offset] = input_port_read(machine, "DSW1");
		}
		break;

		case 0x04:	// TEST (2 versions)
		{
			toxboy_handle_04_subcommand(machine, mcu_data, kaneko16_mcu_ram);
		}
		break;
	}

}

