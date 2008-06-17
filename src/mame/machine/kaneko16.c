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
			return watchdog_reset_r(machine,0);

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
			return (mame_rand(machine) & 0xffff);

		default:
			logerror("CPU #0 PC %06x: warning - read unmapped calc address %06x\n",activecpu_get_pc(),offset<<1);
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
			logerror("CPU #0 PC %06x: warning - write unmapped hit address %06x\n",activecpu_get_pc(),offset<<1);
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
			logerror("CPU #0 PC %06x: warning - write unmapped hit address %06x\n",activecpu_get_pc(),offset<<1);
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
			return (mame_rand(machine) & 0xffff);

		case 0x20/2: return hit.x1p;
		case 0x22/2: return hit.x1s;
		case 0x24/2: return hit.y1p;
		case 0x26/2: return hit.y1s;

		case 0x2c/2: return hit.x2p;
		case 0x2e/2: return hit.x2s;
		case 0x30/2: return hit.y2p;
		case 0x32/2: return hit.y2s;

		default:
			logerror("CPU #0 PC %06x: warning - read unmapped calc address %06x\n",activecpu_get_pc(),offset<<1);
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
019E : ??? -> $102e14, compared with -1 once, very interesting, see $1063e6 - IT2
030A : location where MCU will get its parameters from now on
FFFE : probably polled by MCU, needs to be kept alive (cleared by main cpu - IT2)
0042 : MCU writes its checksum
00207FE0 : may serves for relocating code (written as .l)
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
	calc3_mcu_run(machine);
}

#define CALC3_MCU_COM_W(_n_)				\
WRITE16_HANDLER( calc3_mcu_com##_n_##_w )	\
{											\
	calc3_mcu_status |= (1 << _n_);			\
	calc3_mcu_run(machine);					\
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

static void calc3_mcu_run(running_machine *machine)
{
	UINT16 mcu_command;

	if ( calc3_mcu_status != (1|2|4|8) )	return;

	mcu_command = kaneko16_mcu_ram[calc3_mcu_command_offset + 0];

	if (mcu_command == 0) return;

	logerror("CPU #0 PC %06X : MCU executed command at %04X: %04X\n",
	 	activecpu_get_pc(),calc3_mcu_command_offset*2,mcu_command);

	switch (mcu_command)
	{

		case 0x00ff:
		{
			int param1 = kaneko16_mcu_ram[calc3_mcu_command_offset + 1];
			int param2 = kaneko16_mcu_ram[calc3_mcu_command_offset + 2];
			int param3 = kaneko16_mcu_ram[calc3_mcu_command_offset + 3];
			//int param4 = kaneko16_mcu_ram[calc3_mcu_command_offset + 4];
			int param5 = kaneko16_mcu_ram[calc3_mcu_command_offset + 5];
			//int param6 = kaneko16_mcu_ram[calc3_mcu_command_offset + 6];
			//int param7 = kaneko16_mcu_ram[calc3_mcu_command_offset + 7];

			// clear old command (handshake to main cpu)
			kaneko16_mcu_ram[calc3_mcu_command_offset] = 0x0000;

			// execute the command:

			kaneko16_mcu_ram[param1 / 2] = ~input_port_read(machine, "DSW1");	// DSW
			kaneko16_mcu_ram[param2 / 2] = 0xffff;				// ? -1 / anything else

			calc3_mcu_command_offset = param3 / 2;	// where next command will be written?
			// param 4?
			kaneko16_mcu_ram[param5 / 2] = 0x8ee4;				// MCU Rom Checksum!
			// param 6&7 = address.l

/*

First code snippet provided by the MCU:

207FE0: 48E7 FFFE                movem.l D0-D7/A0-A6, -(A7)

207FE4: 3039 00A8 0000           move.w  $a80000.l, D0
207FEA: 4279 0020 FFFE           clr.w   $20fffe.l

207FF0: 41F9 0020 0000           lea     $200000.l, A0
207FF6: 7000                     moveq   #$0, D0

207FF8: 43E8 01C6                lea     ($1c6,A0), A1
207FFC: 7E02                     moveq   #$2, D7
207FFE: D059                     add.w   (A1)+, D0
208000: 51CF FFFC                dbra    D7, 207ffe

208004: 43E9 0002                lea     ($2,A1), A1
208008: 7E04                     moveq   #$4, D7
20800A: D059                     add.w   (A1)+, D0
20800C: 51CF FFFC                dbra    D7, 20800a

208010: 4640                     not.w   D0
208012: 5340                     subq.w  #1, D0
208014: 0068 0030 0216           ori.w   #$30, ($216,A0)

20801A: B07A 009A                cmp.w   ($9a,PC), D0; ($2080b6)
20801E: 670A                     beq     20802a

208020: 0268 000F 0216           andi.w  #$f, ($216,A0)
208026: 4268 0218                clr.w   ($218,A0)

20802A: 5468 0216                addq.w  #2, ($216,A0)
20802E: 42A8 030C                clr.l   ($30c,A0)
208032: 117C 0020 030C           move.b  #$20, ($30c,A0)

208038: 3E3C 0001                move.w  #$1, D7

20803C: 0C68 0008 0218           cmpi.w  #$8, ($218,A0)
208042: 6C00 0068                bge     2080ac

208046: 117C 0080 0310           move.b  #$80, ($310,A0)
20804C: 117C 0008 0311           move.b  #$8, ($311,A0)
208052: 317C 7800 0312           move.w  #$7800, ($312,A0)
208058: 5247                     addq.w  #1, D7
20805A: 0C68 0040 0216           cmpi.w  #$40, ($216,A0)
208060: 6D08                     blt     20806a

208062: 5468 0218                addq.w  #2, ($218,A0)
208066: 6000 0044                bra     2080ac

20806A: 117C 0041 0314           move.b  #$41, ($314,A0)

208070: 0C39 0001 0010 2E12      cmpi.b  #$1, $102e12.l
208078: 6606                     bne     208080

20807A: 117C 0040 0314           move.b  #$40, ($314,A0)

208080: 117C 000C 0315           move.b  #$c, ($315,A0)
208086: 317C 7000 0316           move.w  #$7000, ($316,A0)
20808C: 5247                     addq.w  #1, D7

20808E: 0839 0001 0010 2E15      btst    #$1, $102e15.l ; service mode
208096: 6714                     beq     2080ac

208098: 117C 0058 0318           move.b  #$58, ($318,A0)
20809E: 117C 0006 0319           move.b  #$6, ($319,A0)
2080A4: 317C 6800 031A           move.w  #$6800, ($31a,A0)
2080AA: 5247                     addq.w  #1, D7

2080AC: 3147 030A                move.w  D7, ($30a,A0)
2080B0: 4CDF 7FFF                movem.l (A7)+, D0-D7/A0-A6
2080B4: 4E73                     rte

2080B6: C747
*/
		}
		break;


		case 0x0001:
		{
			//int param1 = kaneko16_mcu_ram[calc3_mcu_command_offset + 1];
			int param2 = kaneko16_mcu_ram[calc3_mcu_command_offset + 2];

			// clear old command (handshake to main cpu)
			kaneko16_mcu_ram[calc3_mcu_command_offset] = 0x0000;

			// execute the command:

			// param1 ?
			kaneko16_mcu_ram[param2/2 + 0] = 0x0000;		// ?
			kaneko16_mcu_ram[param2/2 + 1] = 0x0000;		// ?
			kaneko16_mcu_ram[param2/2 + 2] = 0x0000;		// ?
			kaneko16_mcu_ram[param2/2 + 3] = 0x0000;		// ? addr.l
			kaneko16_mcu_ram[param2/2 + 4] = 0x00e0;		// 0000e0: 4e73 rte

		}
		break;


		case 0x0002:
		{
			//int param1 = kaneko16_mcu_ram[calc3_mcu_command_offset + 1];
			//int param2 = kaneko16_mcu_ram[calc3_mcu_command_offset + 2];
			//int param3 = kaneko16_mcu_ram[calc3_mcu_command_offset + 3];
			//int param4 = kaneko16_mcu_ram[calc3_mcu_command_offset + 4];
			//int param5 = kaneko16_mcu_ram[calc3_mcu_command_offset + 5];
			//int param6 = kaneko16_mcu_ram[calc3_mcu_command_offset + 6];
			//int param7 = kaneko16_mcu_ram[calc3_mcu_command_offset + 7];

			// clear old command (handshake to main cpu)
			kaneko16_mcu_ram[calc3_mcu_command_offset] = 0x0000;

			// execute the command:

		}
		break;

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
	toybox_mcu_run(machine);							\
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
	logerror("CPU #%d (PC=%06X) : read MCU status\n", cpu_getactivecpu(), activecpu_get_previouspc());
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
			logerror("PC=%06X : MCU executed command: %04X %04X (load NVRAM settings)\n", activecpu_get_pc(), mcu_command, mcu_offset*2);
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
			logerror("PC=%06X : MCU executed command: %04X %04X (save NVRAM settings)\n", activecpu_get_pc(), mcu_command, mcu_offset*2);
		}
		break;

		case 0x03:	// DSW
		{
			kaneko16_mcu_ram[mcu_offset] = input_port_read(machine, "DSW1");
			logerror("PC=%06X : MCU executed command: %04X %04X (read DSW)\n", activecpu_get_pc(), mcu_command, mcu_offset*2);
		}
		break;

		case 0x04:	// Protection
		{
			logerror("PC=%06X : MCU executed command: %04X %04X %04X\n", activecpu_get_pc(), mcu_command, mcu_offset*2, mcu_data);

			switch(mcu_data)
			{
				// unknown purpose data
				case 0x01: MCU_RESPONSE(bloodwar_mcu_4_01); break; // Warrior 1
				case 0x02: MCU_RESPONSE(bloodwar_mcu_4_02); break; // Warrior 2
				case 0x03: MCU_RESPONSE(bloodwar_mcu_4_03); break; // Warrior 3
				case 0x04: MCU_RESPONSE(bloodwar_mcu_4_04); break; // Warrior 4
				case 0x05: MCU_RESPONSE(bloodwar_mcu_4_05); break; // Warrior 5
				case 0x06: MCU_RESPONSE(bloodwar_mcu_4_06); break; // Warrior 6
				case 0x07: MCU_RESPONSE(bloodwar_mcu_4_07); break; // Warrior 7
				case 0x08: MCU_RESPONSE(bloodwar_mcu_4_08); break; // Warrior 8
				case 0x09: MCU_RESPONSE(bloodwar_mcu_4_09); break; // Warrior 9

				// palette data
				case 0x0a: MCU_RESPONSE(bloodwar_mcu_4_0a); break; // Warrior 1 Player 1
				case 0x0b: MCU_RESPONSE(bloodwar_mcu_4_0b); break; // Warrior 1 Player 2
				case 0x0c: MCU_RESPONSE(bloodwar_mcu_4_0c); break; // Warrior 5 Player 1
				case 0x0d: MCU_RESPONSE(bloodwar_mcu_4_0d); break; // Warrior 5 Player 2
				case 0x0e: MCU_RESPONSE(bloodwar_mcu_4_0e); break; // Warrior 4 Player 2
				case 0x0f: MCU_RESPONSE(bloodwar_mcu_4_0f); break; // Warrior 4 Player 1
				case 0x10: MCU_RESPONSE(bloodwar_mcu_4_10); break; // Warrior 6 Player 1
				case 0x11: MCU_RESPONSE(bloodwar_mcu_4_11); break; // Warrior 6 Player 2
				case 0x12: MCU_RESPONSE(bloodwar_mcu_4_12); break; // Warrior 9 Player 1
				case 0x13: MCU_RESPONSE(bloodwar_mcu_4_13); break; // Warrior 9 Player 2
				case 0x14: MCU_RESPONSE(bloodwar_mcu_4_14); break; // Warrior 7 Player 1
				case 0x15: MCU_RESPONSE(bloodwar_mcu_4_15); break; // Warrior 7 Player 2
				case 0x16: MCU_RESPONSE(bloodwar_mcu_4_16); break; // Warrior 8 Player 1
				case 0x17: MCU_RESPONSE(bloodwar_mcu_4_17); break; // Warrior 8 Player 2
				case 0x18: MCU_RESPONSE(bloodwar_mcu_4_18); break; // Warrior 2 Player 2
				case 0x19: MCU_RESPONSE(bloodwar_mcu_4_19); break; // Warrior 2 Player 1
				case 0x1a: MCU_RESPONSE(bloodwar_mcu_4_1a); break; // Warrior 3 Player 1
				case 0x1b: MCU_RESPONSE(bloodwar_mcu_4_1b); break; // Warrior 3 Player 2

				// tilemap data
				case 0x1c: MCU_RESPONSE(bloodwar_mcu_4_1c); break; // Warrior 8
				case 0x1d: MCU_RESPONSE(bloodwar_mcu_4_1d); break; // Warrior 2
				case 0x1e: MCU_RESPONSE(bloodwar_mcu_4_1e); break; // Warrior 3
				case 0x1f: MCU_RESPONSE(bloodwar_mcu_4_1f); break; // Warrior 5
				case 0x20: MCU_RESPONSE(bloodwar_mcu_4_20); break; // Warrior 4
				case 0x21: MCU_RESPONSE(bloodwar_mcu_4_21); break; // Warrior 6
				case 0x22: MCU_RESPONSE(bloodwar_mcu_4_22); break; // Warrior 1
				case 0x23: MCU_RESPONSE(bloodwar_mcu_4_23); break; // Warrior 9
				case 0x24: MCU_RESPONSE(bloodwar_mcu_4_24); break; // Warrior 7

				// fighter data: pointers to ROM data
				case 0x25: MCU_RESPONSE(bloodwar_mcu_4_25); break; // Warrior 1
				case 0x26: MCU_RESPONSE(bloodwar_mcu_4_26); break; // Warrior 2
				case 0x27: MCU_RESPONSE(bloodwar_mcu_4_27); break; // Warrior 3
				case 0x28: MCU_RESPONSE(bloodwar_mcu_4_28); break; // Warrior 4
				case 0x29: MCU_RESPONSE(bloodwar_mcu_4_29); break; // Warrior 5
				case 0x2a: MCU_RESPONSE(bloodwar_mcu_4_2a); break; // Warrior 6
				case 0x2b: MCU_RESPONSE(bloodwar_mcu_4_2b); break; // Warrior 7
				case 0x2c: MCU_RESPONSE(bloodwar_mcu_4_2c); break; // Warrior 8
				case 0x2d: MCU_RESPONSE(bloodwar_mcu_4_2d); break; // Warrior 9

				default:
					logerror(" (UNKNOWN PARAMETER %02X)\n", mcu_data);
			}
		}
		break;

		default:
			logerror("PC=%06X : MCU executed command: %04X %04X %04X (UNKNOWN COMMAND)\n", activecpu_get_pc(), mcu_command, mcu_offset*2, mcu_data);
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
			logerror("PC=%06X : MCU executed command: %04X %04X (load NVRAM settings)\n", activecpu_get_pc(), mcu_command, mcu_offset*2);
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
			logerror("PC=%06X : MCU executed command: %04X %04X (save NVRAM settings)\n", activecpu_get_pc(), mcu_command, mcu_offset*2);
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
			logerror("PC=%06X : MCU executed command: %04X %04X (restore default NVRAM settings)\n", activecpu_get_pc(), mcu_command, mcu_offset*2);
		}
		break;

		case 0x03:	// DSW
		{
			kaneko16_mcu_ram[mcu_offset] = input_port_read(machine, "DSW1");
			logerror("PC=%06X : MCU executed command: %04X %04X (read DSW)\n", activecpu_get_pc(), mcu_command, mcu_offset*2);
		}
		break;

		case 0x04:	// Protection
		{
			logerror("PC=%06X : MCU executed command: %04X %04X %04X\n", activecpu_get_pc(), mcu_command, mcu_offset*2, mcu_data);

			switch(mcu_data)
			{
				// static, in this order, at boot/reset
				case 0x34: MCU_RESPONSE(bonkadv_mcu_4_34); break;
				case 0x30: MCU_RESPONSE(bonkadv_mcu_4_30); break;
				case 0x31: MCU_RESPONSE(bonkadv_mcu_4_31); break;
				case 0x32: MCU_RESPONSE(bonkadv_mcu_4_32); break;
				case 0x33: MCU_RESPONSE(bonkadv_mcu_4_33); break;

				// dynamic, per-level (29), in level order
				case 0x00: MCU_RESPONSE(bonkadv_mcu_4_00); break;
				case 0x02: MCU_RESPONSE(bonkadv_mcu_4_02); break;
				case 0x01: MCU_RESPONSE(bonkadv_mcu_4_01); break;
				case 0x05: MCU_RESPONSE(bonkadv_mcu_4_05); break;
				case 0x07: MCU_RESPONSE(bonkadv_mcu_4_07); break;
				case 0x06: MCU_RESPONSE(bonkadv_mcu_4_06); break;
				case 0x09: MCU_RESPONSE(bonkadv_mcu_4_09); break;
				case 0x0D: MCU_RESPONSE(bonkadv_mcu_4_0D); break;
				case 0x03: MCU_RESPONSE(bonkadv_mcu_4_03); break;
				case 0x08: MCU_RESPONSE(bonkadv_mcu_4_08); break;
				case 0x04: MCU_RESPONSE(bonkadv_mcu_4_04); break;
				case 0x0C: MCU_RESPONSE(bonkadv_mcu_4_0C); break;
				case 0x0A: MCU_RESPONSE(bonkadv_mcu_4_0A); break;
				case 0x0B: MCU_RESPONSE(bonkadv_mcu_4_0B); break;
				case 0x10: MCU_RESPONSE(bonkadv_mcu_4_10); break;
				case 0x0E: MCU_RESPONSE(bonkadv_mcu_4_0E); break;
				case 0x13: MCU_RESPONSE(bonkadv_mcu_4_13); break;
				case 0x0F: MCU_RESPONSE(bonkadv_mcu_4_0F); break;
				case 0x11: MCU_RESPONSE(bonkadv_mcu_4_11); break;
				case 0x14: MCU_RESPONSE(bonkadv_mcu_4_14); break;
				case 0x12: MCU_RESPONSE(bonkadv_mcu_4_12); break;
				case 0x17: MCU_RESPONSE(bonkadv_mcu_4_17); break;
				case 0x1A: MCU_RESPONSE(bonkadv_mcu_4_1A); break;
				case 0x15: MCU_RESPONSE(bonkadv_mcu_4_15); break;
				case 0x18: MCU_RESPONSE(bonkadv_mcu_4_18); break;
				case 0x16: MCU_RESPONSE(bonkadv_mcu_4_16); break;
				case 0x19: MCU_RESPONSE(bonkadv_mcu_4_19); break;
				case 0x1B: MCU_RESPONSE(bonkadv_mcu_4_1B); break;
				case 0x1C: MCU_RESPONSE(bonkadv_mcu_4_1C); break;

				default:
					logerror(" (UNKNOWN PARAMETER %02X)\n", mcu_data);
			}
		}
		break;

		default:
			logerror("PC=%06X : MCU executed command: %04X %04X %04X (UNKNOWN COMMAND)\n", activecpu_get_pc(), mcu_command, mcu_offset*2, mcu_data);
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

	logerror("CPU #0 PC %06X : MCU executed command: %04X %04X %04X\n", activecpu_get_pc(), mcu_command, mcu_offset*2, mcu_data);

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
			if (strcmp(machine->gamedrv->name, "gtmr") == 0 ||
				strcmp(machine->gamedrv->name, "gtmra") == 0)
			{
				/* MCU writes the string "MM0525-TOYBOX199" to shared ram */
				kaneko16_mcu_ram[mcu_offset+0] = 0x4d4d;
				kaneko16_mcu_ram[mcu_offset+1] = 0x3035;
				kaneko16_mcu_ram[mcu_offset+2] = 0x3235;
				kaneko16_mcu_ram[mcu_offset+3] = 0x2d54;
				kaneko16_mcu_ram[mcu_offset+4] = 0x4f59;
				kaneko16_mcu_ram[mcu_offset+5] = 0x424f;
				kaneko16_mcu_ram[mcu_offset+6] = 0x5831;
				kaneko16_mcu_ram[mcu_offset+7] = 0x3939;
			}
			else
			{
				/* MCU writes the string "USMM0713-TB1994 " to shared ram */
				kaneko16_mcu_ram[mcu_offset+0] = 0x5553;
				kaneko16_mcu_ram[mcu_offset+1] = 0x4d4d;
				kaneko16_mcu_ram[mcu_offset+2] = 0x3037;
				kaneko16_mcu_ram[mcu_offset+3] = 0x3133;
				kaneko16_mcu_ram[mcu_offset+4] = 0x2d54;
				kaneko16_mcu_ram[mcu_offset+5] = 0x4231;
				kaneko16_mcu_ram[mcu_offset+6] = 0x3939;
				kaneko16_mcu_ram[mcu_offset+7] = 0x3420;
			}
		}
		break;
	}

}

