
/* PGM System (c)1997 IGS

Based on Information from ElSemi

A flexible cartridge based platform some would say was designed to compete with
SNK's NeoGeo and Capcom's CPS Hardware systems, despite its age it only uses a
68000 for the main processor and a Z80 to drive the sound, just like the two
previously mentioned systems in that respect..

Resolution is 448x224, 15 bit colour

Sound system is ICS WaveFront 2115 Wavetable midi synthesizer, used in some
actual sound cards (Turtle Beach)

Later games are encrypted.  Latest games (kov2, ddp2) include an arm7
coprocessor with an internal rom and an encrypted external rom.

Roms Contain the Following Data

Pxxxx - 68k Program
Txxxx - TX & BG Graphics (2 formats within the same rom)
Mxxxx - Music samples (8 bit mono 11025Hz)
Axxxx - Colour Data (for sprites)
Bxxxx - Masks & A Rom Colour Indexes (for sprites)

There is no rom for the Z80, the program is uploaded by the 68k

Known Games on this Platform
----------------------------


010x  - 1997  - Oriental Legend
020x  - 1997  - Dragon World 2
030x  - 1998  - The Killing Blade
040x  - 1998  - Dragon World 3
050x  - 1999? - Oriental Legend Super
060x  - 1999  - Knights of Valor, Knights of Valor Plus, Knights of Valor Superheroes
070x  - 1999  - Photo Y2k
080x  - 1999  - Puzzle Star
090x  - 2001  - Puzzli II
100x  - 2001  - Martial Masters

120x  - 2001  - Knights of Valor 2 Plus (9 Dragons?)
130x  - 2001  - DoDonpachi II

0450x - 2002  - Demon Front

---

unknown codes:
Dragon World 2001
Photo Y2k2
The Gladiator

---


Oriental Legend
Oriental Legend Super
Sengoku Senki / Knights of Valour Series
-
Sangoku Senki (c)1999 IGS
Sangoku Senki Super Heroes (c)1999 IGS
Sangoku Senki 2 Knights of Valour (c)2000 IGS
Sangoku Senki Busyou Souha (c)2001 IGS
-
DoDonPachi II (Bee Storm)
Photo Y2K
Photo Y2K II
Martial Masters
The Killing Blade
Dragon World 2
Dragon World 3
Dragon World 2001
Demon Front
The Gladiator
Puzzli II

There is also a single board version of the PGM system used by

Demon Front
Some Later (2002/2003) Cave shooters (Uses a Custom CAVE BIOS)

To Do / Notes:  (Revised December 2005)

Missing Sprite Features
  Zooming (table is uploaded to Video Ram)
  It is possible sprites should be transfered out of RAM with a DMA device.
  Priority?

Protection in Mnny Games
  It is possible to read the Internal Rom of the ASIC27A games with external data
  rom, but not the ones with no External Rom.
  Some of the other protection devices aren't understood at all yet, for example
  the ones on Dragon World 3, Oriental Legend Super and The Killing Blade.
  an ARM core with thumbs support is required to emulate the ASIC27A based games
  even with the internal rom.  At the current time the MAME core does not support
  this, Nebula does.

fix sound comms, several games fail prior to their protection checks due to the
current sound implementation.

Fix IRQs, maybe the protection device generates one of them on DW2 as I believe
it's the only game that needs IRQ4 and Puzzli2 explicitly doesn't want IRQ4 to be
active.

Some dumps are suspicious (orlegend super clones are missing roms, drgw3k sets
might not have the right protection rom)  In many cases the external protection
data roms change with each revision of the game.


General Notes:
--------------

Tit makes more sense to name them kov since the roms are probably the same on the various
boards.  The current sets were taken from taiwan boards incase somebody finds
it not to be the case however due to the previous note.

As we can't dump the internal rom of rhte protection devices (which contain the region
information the only way we can support multiple regions is with a fake dipswitch, this
isn't idea as it gives the false impression that the board contain a region dipswitch)

Dragon World 2 still has strange protection issues, we have to patch the code for now, what
should really happen, it jumps to invalid code, should the protection device cause the 68k
to see valid code there or something?  The English version of Dragon World 2 still appears
to have some problems which the current patching doesn't cover.

kov superheroes uses a different protection chip / different protection commands and doesn't
work, some of the gfx also need redumping to check they're the same as kov, its using invalid
codes for the ones we have (could just be protection tho)


Protection Devices / Co-processors
----------------------------------

IGS used a variety of additional ASIC chips on the game boards, these act as protection and
also give additional power to the board to make up for the limited power of the 68000
processor.  Some protection devices use external data roms, others have internal code only.
Most of these are not emulated correctly. In most cases the protection device supplies the
game region code..

ASIC 3:
    used by:
    different per region, supplies region code
    used by:
    Oriental Legend
    function:

ASIC 12 + ASIC 25
    these seem to be used together
    ASIC 25 appears to perform some kind of bitswap operations
    used by:
    Dragon World 2

ASIC 22 + ASIC 25
    these seem to be used together, ASIC25 has an external software decrypted? data rom
    ASIC 22 might be an updated version of ASIC12 ?
    used by:
    Dragon World 3
    The Killing Blade

ASIC 25 + ASIC 28
    Oriental Legend Super

ASIC 28:
    performs a variety of calculations, quite complex, different per region, supplies region code
    used by:
    Knights of Valour 1 / Plus / Superheroes (plus & superheroes seems to use extra functions, emulation issues reported in places in plus)
    Photo Y2k / Real and Fake (maybe..)
    This could be an ARM chip like the 27A below, but without the support for an external ROM (or nothing uses it..)

ASIC 27A:
    arm9? cpu with (max?) 64kb internal rom (different per game / revision) + optional external data rom
    probably used to give extra power to the system, lots of calculations are offloaded to it
    used by:
    DoDonPachi II
    Knights of Valor 2 / 2 Plus
    Martial Masters
    Demon Front
    Puzzli II
    The Gladiator

there are probably more...

PCB Layout
----------

IGS PCB NO-0133-2 (Main Board)
|-------------------------------------------------------------------------------------|
|   |----------------------------|   |----------|   |----------------------------|    |
|   |----------------------------|   |----------|   |----------------------------|    |
|                                      PGM_T01S.U29  UM61256    SRM2B61256  SRM2B61256|
| |---------|  33.8688MHz   |----------|                        SRM2B61256  SRM2B61256|
| |WAVEFRONT|               |L8A0290   |   UM6164  UM6164                             |
| |ICS2115V |               |IGS023    |                 PGM_P01S.U20              SW2|
| |(PLCC84) |               |(QFP256)  |                                              |
| |         |               |          |                                              |
| |---------|        50MHz  |----------|                                              |
|    UPD6379  PGM_M01S.U18                             |----------|                   |
|VOL                                                   |MC68HC000 |          74HC132  |
|                                                      |FN20      |   20MHz  74HC132  |
|  UPC844C    |------|                                 |(PLCC68)  |                   |
|             |Z80   |                                 |          |          V3021    |
|             |PLCC44|                  PAL            |----------|                   |
|             |------|    |--------|                                      32.768kHz   |-|
|                         |IGS026  |                                                    |
|                         |(QFP144)|           |--------|                              I|
|                         |        |           |IGS026  |                              D|
|                         |--------|           |(QFP144)|                              C|
|TDA1519A    UM61256 UM61256                   |        |                              3|
|                              TD62064         |--------|                              4|
|                                                                          3.6V_BATT    |
|                                                                                     |-|
|              |----|                                           |-----|     SW3       |
|              |    |               J  A  M  M  A               |     | SW1           |
|--------------|    |-------------------------------------------|     |---------------|


IGS PCB NO-0136 (Riser)
|-------------------------------------------------------------------------------------|
|      |---------------------------------|  |---------------------------------|       |
|      |---------------------------------|  |---------------------------------|       |
|                                                                                     |
|      |---------------------------------|  |---------------------------------|       |
|      |---------------------------------|  |---------------------------------|       |
|                                                                                     |
|   |----------------------------|   |----------|   |----------------------------|    |
|---|                            |---|          |---|                            |----|
    |----------------------------|   |----------|   |----------------------------|

Notes:
      All IC's are shown.

      CPU's
      -----
         68HC000FN20 - Motorola 68000 processor, clocked at 20.000MHz (PLCC68)
         Z80         - Zilog Z0840008VSC Z80 processor, clocked at 8.468MHz (PLCC44)

      SOUND
      -----
         ICS2115     - ICS WaveFront ICS2115V Wavetable Midi Synthesizer, clocked at 33.8688MHz (PLCC84)

      RAM
      ---
         SRM2B256 - Epson SRM2B256SLMX55 8K x8 SRAM (x4, SOP28)
         UM6164   - Unicorn Microelectronics UM6164DS-12 8K x8 SRAM (x2, SOJ28)
         UM61256  - Unicorn Microelectronics UM61256FS-15 32K x8 SRAM (x3, SOJ28)

      ROMs
      ----
         PGM_M01S.U18 - 16MBit MASKROM (TSOP48)
         PGM_P01S.U20 - 1MBit  MASKROM (DIP40, socketed, equivalent to 27C1024 EPROM)
         PGM_T01S.U29 - 16MBit MASKROM (SOP44)

      CUSTOM IC's
      -----------
         IGS023 (QFP256)
         IGS026 (x2, QFP144)

      OTHER
      -----
         3.6V_BATT - 3.6V NICad battery, connected to the V3021 RTC
         IDC34     - IDC34 way flat cable plug, doesn't appear to be used for any games. Might be for
                     re-programming some of the custom IC's or on-board surface mounted ROMs?
         PAL       - Atmel ATF16V8B PAL (DIP20)
         SW1       - Push button switch to enter Test Mode
         SW2       - 8 position DIP Switch (for configuration of PCB/game options)
         SW3       - SPDT switch (purpose unknown)
         TD62064   - Toshiba NPN 50V 1.5A Quad Darlinton Switch; for driving coin meters (DIP16)
         TDA1519A  - Philips 2x 6W Stereo Power AMP (SIL9)
         uPD6379   - NEC 2-channel 16-bit D/A converter 10mW typ. (SOIC8)
         uPC844C   - NEC Quad High Speed Wide Band Operational Amplifier (DIP14)
         V3021     - EM Microelectronic-Marin SA Ultra Low Power 32kHz CMOS Real Time Clock (DIP8)
         VOL       - Volume potentiometer

*/

#define PGMLOGERROR 0
#define PGMARM7LOGERROR 1
#define PGMARM7SPEEDHACK 1

#include "driver.h"
#include "deprecat.h"
#include "sound/ics2115.h"
#include "cpu/arm7/arm7core.h"
#include "includes/pgm.h"


UINT16 *pgm_mainram, *pgm_bg_videoram, *pgm_tx_videoram, *pgm_videoregs, *pgm_rowscrollram;
static UINT8 *z80_mainram;
static UINT32 *arm7_shareram;
static UINT32 arm7_latch;


static READ16_HANDLER ( z80_ram_r )
{
	return (z80_mainram[offset*2] << 8)|z80_mainram[offset*2+1];
}

static READ32_HANDLER( arm7_latch_arm_r )
{
	if (PGMARM7LOGERROR) logerror("ARM7: Latch read: %08x (%08x) (%06x)\n", arm7_latch, mem_mask, cpu_get_pc(space->cpu) );
	return arm7_latch;
}


#ifdef PGMARM7SPEEDHACK
static TIMER_CALLBACK( arm_irq )
{
	cpu_set_input_line(machine->cpu[2], ARM7_FIRQ_LINE, PULSE_LINE);
}
#endif

//static emu_timer *   arm_comms_timer;
static WRITE32_HANDLER( arm7_latch_arm_w )
{
	if (PGMARM7LOGERROR) logerror("ARM7: Latch write: %08x (%08x) (%06x)\n", data, mem_mask, cpu_get_pc(space->cpu) );
	COMBINE_DATA(&arm7_latch);

#ifdef PGMARM7SPEEDHACK
//  cpuexec_boost_interleave(space->machine, attotime_zero, ATTOTIME_IN_USEC(100));
	if (data!=0xaa) cpu_spinuntil_trigger(space->cpu, 1000);
	cpuexec_trigger(space->machine, 1002);
#else
	cpuexec_boost_interleave(space->machine, attotime_zero, ATTOTIME_IN_USEC(100));
	cpu_spinuntil_time(space->cpu, cpu_clocks_to_attotime(space->cpu, 100));
#endif
}

static READ32_HANDLER( arm7_shareram_r )
{
	if (PGMARM7LOGERROR) logerror("ARM7: ARM7 Shared RAM Read: %04x = %08x (%08x) (%06x)\n", offset << 2, arm7_shareram[offset], mem_mask, cpu_get_pc(space->cpu) );
	return arm7_shareram[offset];
}

static WRITE32_HANDLER( arm7_shareram_w )
{
	if (PGMARM7LOGERROR) logerror("ARM7: ARM7 Shared RAM Write: %04x = %08x (%08x) (%06x)\n", offset << 2, data, mem_mask, cpu_get_pc(space->cpu) );
	COMBINE_DATA(&arm7_shareram[offset]);
}

static READ16_HANDLER( arm7_latch_68k_r )
{
	if (PGMARM7LOGERROR) logerror("M68K: Latch read: %04x (%04x) (%06x)\n", arm7_latch & 0x0000ffff, mem_mask, cpu_get_pc(space->cpu) );
	return arm7_latch;
}

static WRITE16_HANDLER( arm7_latch_68k_w )
{
	if (PGMARM7LOGERROR) logerror("M68K: Latch write: %04x (%04x) (%06x)\n", data & 0x0000ffff, mem_mask, cpu_get_pc(space->cpu) );
	COMBINE_DATA(&arm7_latch);

#ifdef PGMARM7SPEEDHACK
	cpuexec_trigger(space->machine, 1000);
	timer_set(space->machine, ATTOTIME_IN_USEC(50), NULL, 0, arm_irq); // i don't know how long..
	cpu_spinuntil_trigger(space->cpu, 1002);
#else
	cpu_set_input_line(space->machine->cpu[2], ARM7_FIRQ_LINE, PULSE_LINE);
	cpuexec_boost_interleave(space->machine, attotime_zero, ATTOTIME_IN_USEC(200));
	cpu_spinuntil_time(space->cpu, cpu_clocks_to_attotime(space->machine->cpu[2], 200)); // give the arm time to respond (just boosting the interleave doesn't help
#endif
}

static READ16_HANDLER( arm7_ram_r )
{
	UINT16 *share16 = (UINT16 *)arm7_shareram;
	if (PGMARM7LOGERROR) logerror("M68K: ARM7 Shared RAM Read: %04x = %04x (%08x) (%06x)\n", BYTE_XOR_LE(offset), share16[BYTE_XOR_LE(offset)], mem_mask, cpu_get_pc(space->cpu) );
	return share16[BYTE_XOR_LE(offset)];
}

static WRITE16_HANDLER( arm7_ram_w )
{
	UINT16 *share16 = (UINT16 *)arm7_shareram;
	if (PGMARM7LOGERROR) logerror("M68K: ARM7 Shared RAM Write: %04x = %04x (%04x) (%06x)\n", BYTE_XOR_LE(offset), data, mem_mask, cpu_get_pc(space->cpu) );

	COMBINE_DATA(&share16[BYTE_XOR_LE(offset)]);
}

static WRITE16_HANDLER ( z80_ram_w )
{
	int pc = cpu_get_pc(space->cpu);
	if(ACCESSING_BITS_8_15)
		z80_mainram[offset*2] = data >> 8;
	if(ACCESSING_BITS_0_7)
		z80_mainram[offset*2+1] = data;

	if(pc != 0xf12 && pc != 0xde2 && pc != 0x100c50 && pc != 0x100b20)
		if (PGMLOGERROR) logerror("Z80: write %04x, %04x @ %04x (%06x)\n", offset*2, data, mem_mask, cpu_get_pc(space->cpu));
}

static WRITE16_HANDLER ( z80_reset_w )
{
	if (PGMLOGERROR) logerror("Z80: reset %04x @ %04x (%06x)\n", data, mem_mask, cpu_get_pc(space->cpu));

	if(data == 0x5050) {
		sndti_reset(SOUND_ICS2115, 0);
		cpu_set_input_line(space->machine->cpu[1], INPUT_LINE_HALT, CLEAR_LINE);
		cpu_set_input_line(space->machine->cpu[1], INPUT_LINE_RESET, PULSE_LINE);
		if(0) {
			FILE *out;
			out = fopen("z80ram.bin", "wb");
			fwrite(z80_mainram, 1, 65536, out);
			fclose(out);
		}
	}
	else
	{
		/* this might not be 100% correct, but several of the games (ddp2, puzzli2 etc. expect the z80 to be turned
           off during data uploads, they write here before the upload */
		cpu_set_input_line(space->machine->cpu[1], INPUT_LINE_HALT, ASSERT_LINE);
	}
}

static WRITE16_HANDLER ( z80_ctrl_w )
{
	if (PGMLOGERROR) logerror("Z80: ctrl %04x @ %04x (%06x)\n", data, mem_mask, cpu_get_pc(space->cpu));
}

static WRITE16_HANDLER ( m68k_l1_w )
{
	if(ACCESSING_BITS_0_7) {
		if (PGMLOGERROR) logerror("SL 1 m68.w %02x (%06x) IRQ\n", data & 0xff, cpu_get_pc(space->cpu));
		soundlatch_w(space, 0, data);
		cpu_set_input_line(space->machine->cpu[1], INPUT_LINE_NMI, PULSE_LINE );
	}
}

static WRITE8_HANDLER( z80_l3_w )
{
	if (PGMLOGERROR) logerror("SL 3 z80.w %02x (%04x)\n", data, cpu_get_pc(space->cpu));
	soundlatch3_w(space, 0, data);
}

static void sound_irq(running_machine *machine, int level)
{
	cpu_set_input_line(machine->cpu[1], 0, level);
}

static const ics2115_interface pgm_ics2115_interface = {
	sound_irq
};

/* Calendar Emulation */

static UINT8 CalVal, CalMask, CalCom=0, CalCnt=0;

static UINT8 bcd(UINT8 data)
{
	return ((data / 10) << 4) | (data % 10);
}

static READ16_HANDLER( pgm_calendar_r )
{
	UINT8 calr;
	calr = (CalVal & CalMask) ? 1 : 0;
	CalMask <<= 1;
	return calr;
}

static WRITE16_HANDLER( pgm_calendar_w )
{
	static mame_system_time systime;

	mame_get_base_datetime(space->machine, &systime);

	// initialize the time, otherwise it crashes
	if( !systime.time )
		mame_get_base_datetime(space->machine, &systime);

	CalCom <<= 1;
	CalCom |= data & 1;
	++CalCnt;
	if(CalCnt==4)
	{
		CalMask = 1;
		CalVal = 1;
		CalCnt = 0;
		switch(CalCom & 0xf)
		{
			case 1: case 3: case 5: case 7: case 9: case 0xb: case 0xd:
				CalVal++;
				break;

			case 0:
				CalVal=bcd(systime.local_time.weekday); //??
				break;

			case 2:  //Hours
				CalVal=bcd(systime.local_time.hour);
				break;

			case 4:  //Seconds
				CalVal=bcd(systime.local_time.second);
				break;

			case 6:  //Month
				CalVal=bcd(systime.local_time.month + 1); //?? not bcd in MVS
				break;

			case 8:
				CalVal=0; //Controls blinking speed, maybe milliseconds
				break;

			case 0xa: //Day
				CalVal=bcd(systime.local_time.mday);
				break;

			case 0xc: //Minute
				CalVal=bcd(systime.local_time.minute);
				break;

			case 0xe:  //Year
				CalVal=bcd(systime.local_time.year % 100);
				break;

			case 0xf:  //Load Date
				mame_get_base_datetime(space->machine, &systime);
				break;
		}
	}
}

static NVRAM_HANDLER( pgm )
{
	if (read_or_write)
		/* save the SRAM settings */
		mame_fwrite(file, pgm_mainram, 0x20000);
	else
	{
		/* load the SRAM settings */
		if (file)
			mame_fread(file, pgm_mainram, 0x20000);
		else
			memset(pgm_mainram, 0, 0x20000);
	}
}

/*** Memory Maps *************************************************************/

static ADDRESS_MAP_START( pgm_mem, ADDRESS_SPACE_PROGRAM, 16)
	AM_RANGE(0x000000, 0x01ffff) AM_ROM   /* BIOS ROM */
	AM_RANGE(0x100000, 0x5fffff) AM_ROMBANK(1) /* Game ROM */

	AM_RANGE(0x700006, 0x700007) AM_WRITENOP // Watchdog?

	AM_RANGE(0x800000, 0x81ffff) AM_RAM AM_MIRROR(0x0e0000) AM_BASE(&pgm_mainram) /* Main Ram */

	AM_RANGE(0x900000, 0x903fff) AM_RAM_WRITE(pgm_bg_videoram_w) AM_BASE(&pgm_bg_videoram) /* Backgrounds */
	AM_RANGE(0x904000, 0x905fff) AM_RAM_WRITE(pgm_tx_videoram_w) AM_BASE(&pgm_tx_videoram) /* Text Layer */
	AM_RANGE(0x907000, 0x9077ff) AM_RAM AM_BASE(&pgm_rowscrollram)
	AM_RANGE(0xa00000, 0xa011ff) AM_RAM_WRITE(paletteram16_xRRRRRGGGGGBBBBB_word_w) AM_BASE(&paletteram16)
	AM_RANGE(0xb00000, 0xb0ffff) AM_RAM AM_BASE(&pgm_videoregs) /* Video Regs inc. Zoom Table */

	AM_RANGE(0xc00002, 0xc00003) AM_READWRITE(soundlatch_word_r, m68k_l1_w)
	AM_RANGE(0xc00004, 0xc00005) AM_READWRITE(soundlatch2_word_r, soundlatch2_word_w)
	AM_RANGE(0xc00006, 0xc00007) AM_READWRITE(pgm_calendar_r, pgm_calendar_w)
	AM_RANGE(0xc00008, 0xc00009) AM_WRITE(z80_reset_w)
	AM_RANGE(0xc0000a, 0xc0000b) AM_WRITE(z80_ctrl_w)
	AM_RANGE(0xc0000c, 0xc0000d) AM_READWRITE(soundlatch3_word_r, soundlatch3_word_w)

	AM_RANGE(0xc08000, 0xc08001) AM_READ_PORT("P1P2")
	AM_RANGE(0xc08002, 0xc08003) AM_READ_PORT("P3P4")
	AM_RANGE(0xc08004, 0xc08005) AM_READ_PORT("Service")
	AM_RANGE(0xc08006, 0xc08007) AM_READ_PORT("DSW")

	AM_RANGE(0xc10000, 0xc1ffff) AM_READWRITE(z80_ram_r, z80_ram_w) /* Z80 Program */
ADDRESS_MAP_END

static UINT16 *killbld_sharedprotram;

static ADDRESS_MAP_START( killbld_mem, ADDRESS_SPACE_PROGRAM, 16)
	AM_RANGE(0x000000, 0x01ffff) AM_ROM   /* BIOS ROM */
	AM_RANGE(0x100000, 0x2fffff) AM_ROMBANK(1) /* Game ROM */
	AM_RANGE(0x300000, 0x303fff) AM_RAM AM_BASE(&killbld_sharedprotram) // Shared with protection device

	AM_RANGE(0x700006, 0x700007) AM_WRITENOP // Watchdog?

	AM_RANGE(0x800000, 0x81ffff) AM_RAM AM_MIRROR(0x0e0000) AM_BASE(&pgm_mainram) /* Main Ram */

	AM_RANGE(0x900000, 0x903fff) AM_RAM_WRITE(pgm_bg_videoram_w) AM_BASE(&pgm_bg_videoram) /* Backgrounds */
	AM_RANGE(0x904000, 0x905fff) AM_RAM_WRITE(pgm_tx_videoram_w) AM_BASE(&pgm_tx_videoram) /* Text Layer */
	AM_RANGE(0x907000, 0x9077ff) AM_RAM AM_BASE(&pgm_rowscrollram)
	AM_RANGE(0xa00000, 0xa011ff) AM_RAM_WRITE(paletteram16_xRRRRRGGGGGBBBBB_word_w) AM_BASE(&paletteram16)
	AM_RANGE(0xb00000, 0xb0ffff) AM_RAM AM_BASE(&pgm_videoregs) /* Video Regs inc. Zoom Table */

	AM_RANGE(0xc00002, 0xc00003) AM_READWRITE(soundlatch_word_r, m68k_l1_w)
	AM_RANGE(0xc00004, 0xc00005) AM_READWRITE(soundlatch2_word_r, soundlatch2_word_w)
	AM_RANGE(0xc00006, 0xc00007) AM_READWRITE(pgm_calendar_r, pgm_calendar_w)
	AM_RANGE(0xc00008, 0xc00009) AM_WRITE(z80_reset_w)
	AM_RANGE(0xc0000a, 0xc0000b) AM_WRITE(z80_ctrl_w)
	AM_RANGE(0xc0000c, 0xc0000d) AM_READWRITE(soundlatch3_word_r, soundlatch3_word_w)

	AM_RANGE(0xc08000, 0xc08001) AM_READ_PORT("P1P2")
	AM_RANGE(0xc08002, 0xc08003) AM_READ_PORT("P3P4")
	AM_RANGE(0xc08004, 0xc08005) AM_READ_PORT("Service")
	AM_RANGE(0xc08006, 0xc08007) AM_READ_PORT("DSW")

	AM_RANGE(0xc10000, 0xc1ffff) AM_READWRITE(z80_ram_r, z80_ram_w) /* Z80 Program */
ADDRESS_MAP_END

static UINT16 *olds_sharedprotram;

static ADDRESS_MAP_START( olds_mem, ADDRESS_SPACE_PROGRAM, 16)
	AM_RANGE(0x000000, 0x01ffff) AM_ROM   /* BIOS ROM */
	AM_RANGE(0x100000, 0x3fffff) AM_ROMBANK(1) /* Game ROM */
	AM_RANGE(0x400000, 0x4fffff) AM_RAM AM_BASE(&olds_sharedprotram) // Shared with protection device

	AM_RANGE(0x700006, 0x700007) AM_WRITENOP // Watchdog?

	AM_RANGE(0x800000, 0x81ffff) AM_RAM AM_MIRROR(0x0e0000) AM_BASE(&pgm_mainram) /* Main Ram */

	AM_RANGE(0x900000, 0x903fff) AM_RAM_WRITE(pgm_bg_videoram_w) AM_BASE(&pgm_bg_videoram) /* Backgrounds */
	AM_RANGE(0x904000, 0x905fff) AM_RAM_WRITE(pgm_tx_videoram_w) AM_BASE(&pgm_tx_videoram) /* Text Layer */
	AM_RANGE(0x907000, 0x9077ff) AM_RAM AM_BASE(&pgm_rowscrollram)
	AM_RANGE(0xa00000, 0xa011ff) AM_RAM_WRITE(paletteram16_xRRRRRGGGGGBBBBB_word_w) AM_BASE(&paletteram16)
	AM_RANGE(0xb00000, 0xb0ffff) AM_RAM AM_BASE(&pgm_videoregs) /* Video Regs inc. Zoom Table */

	AM_RANGE(0xc00002, 0xc00003) AM_READWRITE(soundlatch_word_r, m68k_l1_w)
	AM_RANGE(0xc00004, 0xc00005) AM_READWRITE(soundlatch2_word_r, soundlatch2_word_w)
	AM_RANGE(0xc00006, 0xc00007) AM_READWRITE(pgm_calendar_r, pgm_calendar_w)
	AM_RANGE(0xc00008, 0xc00009) AM_WRITE(z80_reset_w)
	AM_RANGE(0xc0000a, 0xc0000b) AM_WRITE(z80_ctrl_w)
	AM_RANGE(0xc0000c, 0xc0000d) AM_READWRITE(soundlatch3_word_r, soundlatch3_word_w)

	AM_RANGE(0xc08000, 0xc08001) AM_READ_PORT("P1P2")
	AM_RANGE(0xc08002, 0xc08003) AM_READ_PORT("P3P4")
	AM_RANGE(0xc08004, 0xc08005) AM_READ_PORT("Service")
	AM_RANGE(0xc08006, 0xc08007) AM_READ_PORT("DSW")

	AM_RANGE(0xc10000, 0xc1ffff) AM_READWRITE(z80_ram_r, z80_ram_w) /* Z80 Program */
ADDRESS_MAP_END

static ADDRESS_MAP_START( kov2_mem, ADDRESS_SPACE_PROGRAM, 16)
	AM_RANGE(0x000000, 0x01ffff) AM_ROM   /* BIOS ROM */
	AM_RANGE(0x100000, 0x5fffff) AM_ROMBANK(1) /* Game ROM */

	AM_RANGE(0x700006, 0x700007) AM_WRITENOP // Watchdog?

	AM_RANGE(0x800000, 0x81ffff) AM_RAM AM_MIRROR(0x0e0000) AM_BASE(&pgm_mainram) /* Main Ram */

	AM_RANGE(0x900000, 0x903fff) AM_RAM_WRITE(pgm_bg_videoram_w) AM_BASE(&pgm_bg_videoram) /* Backgrounds */
	AM_RANGE(0x904000, 0x905fff) AM_RAM_WRITE(pgm_tx_videoram_w) AM_BASE(&pgm_tx_videoram) /* Text Layer */
	AM_RANGE(0x907000, 0x9077ff) AM_RAM AM_BASE(&pgm_rowscrollram)
	AM_RANGE(0xa00000, 0xa011ff) AM_RAM_WRITE(paletteram16_xRRRRRGGGGGBBBBB_word_w) AM_BASE(&paletteram16)
	AM_RANGE(0xb00000, 0xb0ffff) AM_RAM AM_BASE(&pgm_videoregs) /* Video Regs inc. Zoom Table */

	AM_RANGE(0xc00002, 0xc00003) AM_READWRITE(soundlatch_word_r, m68k_l1_w)
	AM_RANGE(0xc00004, 0xc00005) AM_READWRITE(soundlatch2_word_r, soundlatch2_word_w)
	AM_RANGE(0xc00006, 0xc00007) AM_READWRITE(pgm_calendar_r, pgm_calendar_w)
	AM_RANGE(0xc00008, 0xc00009) AM_WRITE(z80_reset_w)
	AM_RANGE(0xc0000a, 0xc0000b) AM_WRITE(z80_ctrl_w)
	AM_RANGE(0xc0000c, 0xc0000d) AM_READWRITE(soundlatch3_word_r, soundlatch3_word_w)

	AM_RANGE(0xc08000, 0xc08001) AM_READ_PORT("P1P2")
	AM_RANGE(0xc08002, 0xc08003) AM_READ_PORT("P3P4")
	AM_RANGE(0xc08004, 0xc08005) AM_READ_PORT("Service")
	AM_RANGE(0xc08006, 0xc08007) AM_READ_PORT("DSW")

	AM_RANGE(0xc10000, 0xc1ffff) AM_READWRITE(z80_ram_r, z80_ram_w) /* Z80 Program */
	AM_RANGE(0xd00000, 0xd0ffff) AM_READWRITE(arm7_ram_r, arm7_ram_w) /* ARM7 Shared RAM */
	AM_RANGE(0xd10000, 0xd10001) AM_READWRITE(arm7_latch_68k_r, arm7_latch_68k_w) /* ARM7 Latch */
ADDRESS_MAP_END

#if 0
static ADDRESS_MAP_START( cavepgm_mem, ADDRESS_SPACE_PROGRAM, 16)
	AM_RANGE(0x000000, 0x07ffff) AM_ROM   /* larger BIOS ROM */
	AM_RANGE(0xfffffe, 0xffffff) AM_ROMBANK(1) /* Game ROM (unmapped for now, might not even have it) */
	AM_RANGE(0x400000, 0x4fffff) AM_RAM AM_BASE(&olds_sharedprotram) // Shared with protection device

	AM_RANGE(0x700006, 0x700007) AM_WRITENOP // Watchdog?

	AM_RANGE(0x800000, 0x81ffff) AM_RAM AM_MIRROR(0x0e0000) AM_BASE(&pgm_mainram) /* Main Ram */

	AM_RANGE(0x900000, 0x903fff) AM_RAM_WRITE(pgm_bg_videoram_w) AM_BASE(&pgm_bg_videoram) /* Backgrounds */
	AM_RANGE(0x904000, 0x905fff) AM_RAM_WRITE(pgm_tx_videoram_w) AM_BASE(&pgm_tx_videoram) /* Text Layer */
	AM_RANGE(0x907000, 0x9077ff) AM_RAM AM_BASE(&pgm_rowscrollram)
	AM_RANGE(0xa00000, 0xa011ff) AM_RAM_WRITE(paletteram16_xRRRRRGGGGGBBBBB_word_w) AM_BASE(&paletteram16)
	AM_RANGE(0xb00000, 0xb0ffff) AM_RAM AM_BASE(&pgm_videoregs) /* Video Regs inc. Zoom Table */

	AM_RANGE(0xc00002, 0xc00003) AM_READWRITE(soundlatch_word_r, m68k_l1_w)
	AM_RANGE(0xc00004, 0xc00005) AM_READWRITE(soundlatch2_word_r, soundlatch2_word_w)
	AM_RANGE(0xc00006, 0xc00007) AM_READWRITE(pgm_calendar_r, pgm_calendar_w)
	AM_RANGE(0xc00008, 0xc00009) AM_WRITE(z80_reset_w)
	AM_RANGE(0xc0000a, 0xc0000b) AM_WRITE(z80_ctrl_w)
	AM_RANGE(0xc0000c, 0xc0000d) AM_READWRITE(soundlatch3_word_r, soundlatch3_word_w)

	AM_RANGE(0xc08000, 0xc08001) AM_READ_PORT("P1P2")
	AM_RANGE(0xc08002, 0xc08003) AM_READ_PORT("P3P4")
	AM_RANGE(0xc08004, 0xc08005) AM_READ_PORT("Service")
	AM_RANGE(0xc08006, 0xc08007) AM_READ_PORT("DSW")

	AM_RANGE(0xc10000, 0xc1ffff) AM_READWRITE(z80_ram_r, z80_ram_w) /* Z80 Program */
ADDRESS_MAP_END
#endif

static ADDRESS_MAP_START( z80_mem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xffff) AM_RAM AM_BASE(&z80_mainram)
ADDRESS_MAP_END

static ADDRESS_MAP_START( z80_io, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x8000, 0x8003) AM_READWRITE(ics2115_r, ics2115_w)
	AM_RANGE(0x8100, 0x81ff) AM_READWRITE(soundlatch3_r, z80_l3_w)
	AM_RANGE(0x8200, 0x82ff) AM_READWRITE(soundlatch_r, soundlatch_w)
	AM_RANGE(0x8400, 0x84ff) AM_READWRITE(soundlatch2_r, soundlatch2_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( arm7_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x00003fff) AM_ROM
	AM_RANGE(0x08000000, 0x083fffff) AM_ROM AM_REGION("user1", 0)
	AM_RANGE(0x10000000, 0x100003ff) AM_RAM
	AM_RANGE(0x18000000, 0x1800ffff) AM_RAM
	AM_RANGE(0x38000000, 0x38000003) AM_READWRITE(arm7_latch_arm_r, arm7_latch_arm_w) /* 68k Latch */
	AM_RANGE(0x48000000, 0x4800ffff) AM_READWRITE(arm7_shareram_r, arm7_shareram_w) AM_BASE(&arm7_shareram)
	AM_RANGE(0x50000000, 0x500003ff) AM_RAM
ADDRESS_MAP_END


/* Kov Superheroes */

static UINT16 kovsh_highlatch, kovsh_lowlatch;

static READ32_HANDLER( kovsh_arm7_protlatch_r )
{
//  return 0x00880000;
//  return 0x0088ff66;

	return (kovsh_highlatch << 16) | (kovsh_lowlatch);
}

static WRITE32_HANDLER( kovsh_arm7_protlatch_w )
{
	if (ACCESSING_BITS_16_31)
	{
		kovsh_highlatch = data>>16;
	}
	if (ACCESSING_BITS_0_15)
	{
		kovsh_lowlatch = data;
	}

//  cpuexec_boost_interleave(space->machine, attotime_zero, ATTOTIME_IN_USEC(100));
//  cpu_spinuntil_time(space->cpu, cpu_clocks_to_attotime(space->machine->cpu[0], 100));
}

static READ16_HANDLER( kovsh_68k_protlatch_r )
{
	//cpuexec_boost_interleave(space->machine, attotime_zero, ATTOTIME_IN_USEC(200));
	cpu_spinuntil_time(space->cpu, cpu_clocks_to_attotime(space->machine->cpu[0], 600));

	switch (offset)
	{
		case 1: return kovsh_highlatch;
		case 0: return kovsh_lowlatch;
	}
	return -1;
}

static WRITE16_HANDLER( kovsh_68k_protlatch_w )
{
	switch (offset)
	{
		case 1: kovsh_highlatch = data; break;
		case 0: kovsh_lowlatch = data; break;
	}
}

static ADDRESS_MAP_START( kovsh_mem, ADDRESS_SPACE_PROGRAM, 16)
	AM_RANGE(0x000000, 0x01ffff) AM_ROM   /* BIOS ROM */
	AM_RANGE(0x100000, 0x37ffff) AM_ROMBANK(1) /* Game ROM */

	AM_RANGE(0x700006, 0x700007) AM_WRITENOP // Watchdog?

	AM_RANGE(0x800000, 0x81ffff) AM_RAM AM_MIRROR(0x0e0000) AM_BASE(&pgm_mainram) /* Main Ram */

	AM_RANGE(0x900000, 0x903fff) AM_RAM_WRITE(pgm_bg_videoram_w) AM_BASE(&pgm_bg_videoram) /* Backgrounds */
	AM_RANGE(0x904000, 0x905fff) AM_RAM_WRITE(pgm_tx_videoram_w) AM_BASE(&pgm_tx_videoram) /* Text Layer */
	AM_RANGE(0x907000, 0x9077ff) AM_RAM AM_BASE(&pgm_rowscrollram)
	AM_RANGE(0xa00000, 0xa011ff) AM_RAM_WRITE(paletteram16_xRRRRRGGGGGBBBBB_word_w) AM_BASE(&paletteram16)
	AM_RANGE(0xb00000, 0xb0ffff) AM_RAM AM_BASE(&pgm_videoregs) /* Video Regs inc. Zoom Table */

	AM_RANGE(0xc00002, 0xc00003) AM_READWRITE(soundlatch_word_r, m68k_l1_w)
	AM_RANGE(0xc00004, 0xc00005) AM_READWRITE(soundlatch2_word_r, soundlatch2_word_w)
	AM_RANGE(0xc00006, 0xc00007) AM_READWRITE(pgm_calendar_r, pgm_calendar_w)
	AM_RANGE(0xc00008, 0xc00009) AM_WRITE(z80_reset_w)
	AM_RANGE(0xc0000a, 0xc0000b) AM_WRITE(z80_ctrl_w)
	AM_RANGE(0xc0000c, 0xc0000d) AM_READWRITE(soundlatch3_word_r, soundlatch3_word_w)

	AM_RANGE(0xc08000, 0xc08001) AM_READ_PORT("P1P2")
	AM_RANGE(0xc08002, 0xc08003) AM_READ_PORT("P3P4")
	AM_RANGE(0xc08004, 0xc08005) AM_READ_PORT("Service")
	AM_RANGE(0xc08006, 0xc08007) AM_READ_PORT("DSW")

	AM_RANGE(0xc10000, 0xc1ffff) AM_READWRITE(z80_ram_r, z80_ram_w) /* Z80 Program */
	AM_RANGE(0x4f0000, 0x4f003f) AM_READWRITE(arm7_ram_r, arm7_ram_w) /* ARM7 Shared RAM */
	AM_RANGE(0x500000, 0x500003) AM_READWRITE(kovsh_68k_protlatch_r, kovsh_68k_protlatch_w) /* ARM7 Latch */
ADDRESS_MAP_END

static READ32_HANDLER( kovsh_arm7_unk_r )
{
	return 0x00000000;
}

static ADDRESS_MAP_START( kovsh_arm7_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x00003fff) AM_ROM
	AM_RANGE(0x08100000, 0x081fffff) AM_ROM AM_REGION("user1", 0) // unpopulated, returns 0 to keep checksum happy
	AM_RANGE(0x10000000, 0x100003ff) AM_RAM // main ram for asic?

	AM_RANGE(0x40000000, 0x40000003) AM_READ(kovsh_arm7_protlatch_r) AM_WRITE(kovsh_arm7_protlatch_w)
	AM_RANGE(0x40000008, 0x4000000b) AM_WRITE(SMH_NOP) // ?
	AM_RANGE(0x4000000c, 0x4000000f) AM_READ(kovsh_arm7_unk_r)
	AM_RANGE(0x50800000, 0x5080003f) AM_READWRITE(arm7_shareram_r, arm7_shareram_w) AM_BASE(&arm7_shareram)
	AM_RANGE(0x50000000, 0x500003ff) AM_RAM // uploads xor table to decrypt 68k rom here
ADDRESS_MAP_END

/*** Input Ports *************************************************************/

/* enough for 4 players, the basic dips mapped are listed in the test mode */

static INPUT_PORTS_START( pgm )
	PORT_START("P1P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_START("P3P4")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(3)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(4)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(4)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(4)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)

	PORT_START("Service")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN4 )
//  PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON5 )  // test 1p+2p
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )	// what should i use?
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 )	// service 1p+2p
//  PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON6 )  // test 3p+4p
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )	// what should i use?
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_SERVICE2 )	// service 3p+4p
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(3)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(4)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )	// unused?
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )	// unused?
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )	// unused?
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )	// unused?

	PORT_START("DSW")
	PORT_SERVICE( 0x0001, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0002, 0x0002, "Music" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Voice" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Free" )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Stop" )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("Region")
	PORT_DIPNAME( 0x0003, 0x0000, DEF_STR( Region ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( World ) )
//  PORT_DIPSETTING(      0x0001, DEF_STR( World ) ) // again?
	PORT_DIPSETTING(      0x0002, "Korea" )
	PORT_DIPSETTING(      0x0003, "China" )
INPUT_PORTS_END

static INPUT_PORTS_START( orld105k )
	PORT_INCLUDE ( pgm )

	PORT_MODIFY("Region")
	PORT_DIPNAME( 0x0003, 0x0002, DEF_STR( Unused ) )	// region switch
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )		// if enabled, game gives
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )		// "incorrect version" error
INPUT_PORTS_END

static INPUT_PORTS_START( sango )
	PORT_INCLUDE ( pgm )

	PORT_MODIFY("Region")	/* Region - supplied by protection device */
	PORT_DIPNAME( 0x000f, 0x0005, DEF_STR( Region ) )
	PORT_DIPSETTING(      0x0000, "China" )
	PORT_DIPSETTING(      0x0001, "Taiwan" )
	PORT_DIPSETTING(      0x0002, "Japan (Alta License)" )
	PORT_DIPSETTING(      0x0003, "Korea" )
	PORT_DIPSETTING(      0x0004, "Hong Kong" )
	PORT_DIPSETTING(      0x0005, DEF_STR( World ) )
INPUT_PORTS_END

static INPUT_PORTS_START( olds )
	PORT_INCLUDE ( pgm )

	PORT_MODIFY("Region")	/* Region - supplied by protection device */
	PORT_DIPNAME( 0x000f, 0x0006, DEF_STR( Region ) )
	/* includes the following regions:
    1 = taiwan, 2 = china, 3 = japan (title = orlegend special),
    4 = korea, 5 = hong kong, 6 = world */
	PORT_DIPSETTING(      0x0001, "Taiwan" )
	PORT_DIPSETTING(      0x0002, "China" )
	PORT_DIPSETTING(      0x0003, DEF_STR( Japan ) )
	PORT_DIPSETTING(      0x0004, "Korea" )
	PORT_DIPSETTING(      0x0005, "Hong Kong" )
	PORT_DIPSETTING(      0x0006, DEF_STR( World ) )
INPUT_PORTS_END

static INPUT_PORTS_START( killbld )
	PORT_INCLUDE ( pgm )

	PORT_MODIFY("Region")	/* Region - supplied by protection device */
	PORT_DIPNAME( 0x00ff, 0x0021, DEF_STR( Region ) )
	PORT_DIPSETTING(      0x0016, "Taiwan" )
	PORT_DIPSETTING(      0x0017, "China" )
	PORT_DIPSETTING(      0x0018, "Hong Kong" )
	PORT_DIPSETTING(      0x0019, DEF_STR( Japan ) )
//  PORT_DIPSETTING(      0x001a, "1a" ) // invalid
//  PORT_DIPSETTING(      0x001b, "1b" ) // invalid
//  PORT_DIPSETTING(      0x001c, "1c" ) // invalid
//  PORT_DIPSETTING(      0x001d, "1d" ) // invalid
//  PORT_DIPSETTING(      0x001e, "1e" ) // invalid
//  PORT_DIPSETTING(      0x001f, "1f" ) // invalid
	PORT_DIPSETTING(      0x0020, "Korea" )
	PORT_DIPSETTING(      0x0021, DEF_STR( World ) )
INPUT_PORTS_END

static INPUT_PORTS_START( photoy2k )
	PORT_INCLUDE ( pgm )

	PORT_MODIFY("Region")	/* Region - supplied by protection device */
	PORT_DIPNAME( 0x000f, 0x0003, DEF_STR( Region ) )
	PORT_DIPSETTING(      0x0000, "Taiwan" )
	PORT_DIPSETTING(      0x0001, "China" )
	PORT_DIPSETTING(      0x0002, "Japan (Alta License)" )
	PORT_DIPSETTING(      0x0003, DEF_STR( World ))
	PORT_DIPSETTING(      0x0004, "Korea" )
	PORT_DIPSETTING(      0x0005, "Hong Kong" )
INPUT_PORTS_END

static INPUT_PORTS_START( ddp2 )
	PORT_INCLUDE ( pgm )

/*  // probably not dsw related anyway
    PORT_START("UNK0")
    PORT_DIPNAME( 0x0001, 0x0001, "4" )
    PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0010, 0x0000, DEF_STR( Unknown ) )  // Freezes if off?
    PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

    PORT_START("UNK1")
    PORT_DIPNAME( 0x0001, 0x0001, "5" )
    PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
*/
	PORT_MODIFY("Region")	/* Region - supplied by protection device */
	PORT_DIPNAME( 0x000f, 0x0005, DEF_STR( Region ) )
	PORT_DIPSETTING(      0x0000, "China" )
	PORT_DIPSETTING(      0x0001, "Taiwan" )
	PORT_DIPSETTING(      0x0002, "Japan (Cave License)" )
	PORT_DIPSETTING(      0x0003, "Korea" )
	PORT_DIPSETTING(      0x0004, "Hong Kong" )
	PORT_DIPSETTING(      0x0005, DEF_STR( World ) )
INPUT_PORTS_END

/*** GFX Decodes *************************************************************/

/* we can't decode the sprite data like this because it isn't tile based.  the
   data decoded by pgm32_charlayout was rearranged at start-up */

static const gfx_layout pgm8_charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 4, 0, 12, 8, 20,16,  28, 24 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*32
};

static const gfx_layout pgm32_charlayout =
{
	32,32,
	RGN_FRAC(1,1),
	5,
	{ 3,4,5,6,7 },
	{ 0  , 8 ,16 ,24 ,32 ,40 ,48 ,56 ,
	  64 ,72 ,80 ,88 ,96 ,104,112,120,
	  128,136,144,152,160,168,176,184,
	  192,200,208,216,224,232,240,248 },
	{ 0*256, 1*256, 2*256, 3*256, 4*256, 5*256, 6*256, 7*256,
	  8*256, 9*256,10*256,11*256,12*256,13*256,14*256,15*256,
	 16*256,17*256,18*256,19*256,20*256,21*256,22*256,23*256,
	 24*256,25*256,26*256,27*256,28*256,29*256,30*256,31*256 },
	 32*256
};

static GFXDECODE_START( pgm )
	GFXDECODE_ENTRY( "gfx1", 0, pgm8_charlayout,    0x800, 32  ) /* 8x8x4 Tiles */
	GFXDECODE_ENTRY( "gfx2", 0, pgm32_charlayout,   0x400, 32  ) /* 32x32x5 Tiles */
GFXDECODE_END

/*** Machine Driver **********************************************************/

/* only dragon world 2 NEEDs irq4, Puzzli 2 explicitly doesn't want it, what
   is the source? maybe the protection device? */
static INTERRUPT_GEN( drgw_interrupt ) {
	if( cpu_getiloops(device) == 0 )
		cpu_set_input_line(device, 6, HOLD_LINE);
	else
		cpu_set_input_line(device, 4, HOLD_LINE);
}

static MACHINE_RESET ( pgm )
{
	cpu_set_input_line(machine->cpu[1], INPUT_LINE_HALT, ASSERT_LINE);
}

static MACHINE_DRIVER_START( pgm )

	/* basic machine hardware */
	MDRV_CPU_ADD("main", M68000, 20000000) /* 20 mhz! verified on real board */
	MDRV_CPU_PROGRAM_MAP(pgm_mem,0)
	MDRV_CPU_VBLANK_INT("main", irq6_line_hold)

	MDRV_CPU_ADD("sound", Z80, 8468000)
	MDRV_CPU_PROGRAM_MAP(z80_mem, 0)
	MDRV_CPU_IO_MAP(z80_io, 0)

	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("ics", ICS2115, 0)
	MDRV_SOUND_CONFIG(pgm_ics2115_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 5.0)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 64*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 56*8-1, 0*8, 28*8-1)

	MDRV_GFXDECODE(pgm)
	MDRV_PALETTE_LENGTH(0x1200/2)
	MDRV_MACHINE_RESET ( pgm )
	MDRV_NVRAM_HANDLER ( pgm )

	MDRV_VIDEO_START(pgm)
	MDRV_VIDEO_EOF(pgm)
	MDRV_VIDEO_UPDATE(pgm)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( drgw2 )
	MDRV_IMPORT_FROM(pgm)

	MDRV_CPU_MODIFY("main")
	MDRV_CPU_VBLANK_INT_HACK(drgw_interrupt,2) // needs an extra IRQ, puzzli2 doesn't want this irq!
MACHINE_DRIVER_END

static MACHINE_RESET( killbld );

static MACHINE_DRIVER_START( killbld )
	MDRV_IMPORT_FROM(pgm)

	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(killbld_mem,0)

	MDRV_MACHINE_RESET(killbld)

MACHINE_DRIVER_END

static MACHINE_RESET( olds );

static MACHINE_DRIVER_START( olds )
	MDRV_IMPORT_FROM(pgm)

	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(olds_mem,0)

	MDRV_MACHINE_RESET(olds)

MACHINE_DRIVER_END


static MACHINE_DRIVER_START( kov2 )
	MDRV_IMPORT_FROM(pgm)

	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(kov2_mem,0)

	/* protection CPU */
	MDRV_CPU_ADD("prot", ARM7, 20000000)	// ???
	MDRV_CPU_PROGRAM_MAP(arm7_map, 0)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( kovsh )
	MDRV_IMPORT_FROM(pgm)

	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(kovsh_mem,0)

	/* protection CPU */
	MDRV_CPU_ADD("prot", ARM7, 20000000)	// ???
	MDRV_CPU_PROGRAM_MAP(kovsh_arm7_map, 0)
MACHINE_DRIVER_END

#if 0
static MACHINE_DRIVER_START( cavepgm )
	MDRV_IMPORT_FROM(pgm)

	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(cavepgm_mem,0)

	/* protection CPU */
//  MDRV_CPU_ADD("prot", ARM7, 20000000)    // ???
//  MDRV_CPU_PROGRAM_MAP(arm7_map, 0)
MACHINE_DRIVER_END
#endif


/*** Init Stuff **************************************************************/

/* This function expands the 32x32 5-bit data into a format which is easier to
   decode in MAME */

static void expand_32x32x5bpp(running_machine *machine)
{
	UINT8 *src    = memory_region       ( machine, "gfx1" );
	UINT8 *dst    = memory_region       ( machine, "gfx2" );
	size_t  srcsize = memory_region_length( machine, "gfx1" );
	int cnt, pix;

	for (cnt = 0; cnt < srcsize/5 ; cnt ++)
	{
		pix =  ((src[0+5*cnt] >> 0)& 0x1f );							  dst[0+8*cnt]=pix;
		pix =  ((src[0+5*cnt] >> 5)& 0x07) | ((src[1+5*cnt] << 3) & 0x18);dst[1+8*cnt]=pix;
		pix =  ((src[1+5*cnt] >> 2)& 0x1f );		 					  dst[2+8*cnt]=pix;
		pix =  ((src[1+5*cnt] >> 7)& 0x01) | ((src[2+5*cnt] << 1) & 0x1e);dst[3+8*cnt]=pix;
		pix =  ((src[2+5*cnt] >> 4)& 0x0f) | ((src[3+5*cnt] << 4) & 0x10);dst[4+8*cnt]=pix;
		pix =  ((src[3+5*cnt] >> 1)& 0x1f );							  dst[5+8*cnt]=pix;
		pix =  ((src[3+5*cnt] >> 6)& 0x03) | ((src[4+5*cnt] << 2) & 0x1c);dst[6+8*cnt]=pix;
		pix =  ((src[4+5*cnt] >> 3)& 0x1f );							  dst[7+8*cnt]=pix;
	}
}

/* This function expands the sprite colour data (in the A Roms) from 3 pixels
   in each word to a byte per pixel making it easier to use */

UINT8 *pgm_sprite_a_region;
size_t	pgm_sprite_a_region_allocate;

static void expand_colourdata(running_machine *machine)
{
	UINT8 *src    = memory_region       ( machine, "gfx3" );
	size_t  srcsize = memory_region_length( machine, "gfx3" );
	int cnt;
	size_t	needed = srcsize / 2 * 3;

	/* work out how much ram we need to allocate to expand the sprites into
       and be able to mask the offset */
	pgm_sprite_a_region_allocate = 1;
	while (pgm_sprite_a_region_allocate < needed)
		pgm_sprite_a_region_allocate = pgm_sprite_a_region_allocate <<1;

	pgm_sprite_a_region = auto_malloc (pgm_sprite_a_region_allocate);


	for (cnt = 0 ; cnt < srcsize/2 ; cnt++)
	{
		UINT16 colpack;

		colpack = ((src[cnt*2]) | (src[cnt*2+1] << 8));
		pgm_sprite_a_region[cnt*3+0] = (colpack >> 0 ) & 0x1f;
		pgm_sprite_a_region[cnt*3+1] = (colpack >> 5 ) & 0x1f;
		pgm_sprite_a_region[cnt*3+2] = (colpack >> 10) & 0x1f;
	}
}

static void pgm_basic_init(running_machine *machine)
{
	UINT8 *ROM = memory_region(machine, "main");
	memory_set_bankptr(machine, 1,&ROM[0x100000]);

	expand_32x32x5bpp(machine);
	expand_colourdata(machine);
}

static DRIVER_INIT( pgm )
{
	pgm_basic_init(machine);
}

/* Oriental Legend INIT */

static DRIVER_INIT( orlegend )
{
	pgm_basic_init(machine);

	memory_install_readwrite16_handler(cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_PROGRAM), 0xC0400e, 0xC0400f, 0, 0, pgm_asic3_r, pgm_asic3_w);
	memory_install_write16_handler(cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_PROGRAM), 0xC04000, 0xC04001, 0, 0, pgm_asic3_reg_w);
}

static void drgwld2_common_init(running_machine *machine)
{
	pgm_basic_init(machine);
	pgm_dw2_decrypt(machine);
	/*
    Info from Elsemi
    Here is how to "bypass" the dw2 hang protection, it fixes the mode
    select and after failing in the 2nd stage (probably there are other checks
    out there).
    */
	memory_install_read16_handler(cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_PROGRAM), 0xd80000, 0xd80003, 0, 0, dw2_d80000_r);
}

static DRIVER_INIT( drgw2 )
{	/* incomplete? */
	UINT16 *mem16 = (UINT16 *)memory_region(machine, "main");
	drgwld2_common_init(machine);
	/* These ROM patches are not hacks, the protection device
       overlays the normal ROM code, this has been confirmed on a real PCB
       although some addresses may be missing */
	mem16[0x131098/2]=0x4e93;
	mem16[0x13113e/2]=0x4e93;
	mem16[0x1311ce/2]=0x4e93;
}

static DRIVER_INIT( drgw2c )
{
	UINT16 *mem16 = (UINT16 *)memory_region(machine, "main");
	drgwld2_common_init(machine);
	/* These ROM patches are not hacks, the protection device
       overlays the normal ROM code, this has been confirmed on a real PCB
       although some addresses may be missing */
	mem16[0x1303bc/2]=0x4e93;
	mem16[0x130462/2]=0x4e93;
	mem16[0x1304F2/2]=0x4e93;
}

static DRIVER_INIT( drgw2j )
{
	UINT16 *mem16 = (UINT16 *)memory_region(machine, "main");
	drgwld2_common_init(machine);
	/* These ROM patches are not hacks, the protection device
       overlays the normal ROM code, this has been confirmed on a real PCB
       although some addresses may be missing */
	mem16[0x1302C0/2]=0x4e93;
	mem16[0x130366/2]=0x4e93;
	mem16[0x1303F6/2]=0x4e93;
}

static DRIVER_INIT( kov )
{
	pgm_basic_init(machine);

	memory_install_readwrite16_handler(cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_PROGRAM), 0x500000, 0x500003, 0, 0, ASIC28_r16, ASIC28_w16);

	/* 0x4f0000 - ? is actually ram shared with the protection device,
      the protection device provides the region code */
	memory_install_read16_handler(cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_PROGRAM), 0x4f0000, 0x4fffff, 0, 0, sango_protram_r);

 	pgm_kov_decrypt(machine);
}

static DRIVER_INIT( kov2 )
{
	pgm_basic_init(machine);
	pgm_kov2_decrypt(machine);
}

static DRIVER_INIT( martmast )
{
	pgm_basic_init(machine);
	pgm_mm_decrypt(machine);
}


static DRIVER_INIT( pstar )
{
	pgm_basic_init(machine);
 	pgm_pstar_decrypt(machine);

	memory_install_read16_handler(cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_PROGRAM), 0x4f0000, 0x4f0025, 0, 0, PSTARS_protram_r);
	memory_install_read16_handler(cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_PROGRAM), 0x500000, 0x500003, 0, 0, PSTARS_r16);
	memory_install_write16_handler(cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_PROGRAM), 0x500000, 0x500005, 0, 0, PSTARS_w16);
}



static DRIVER_INIT( kovsh )
{
	pgm_basic_init(machine);

//  memory_install_readwrite16_handler(cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_PROGRAM), 0x500000, 0x500003, 0, 0, ASIC28_r16, ASIC28_w16);

	/* 0x4f0000 - ? is actually ram shared with the protection device,
      the protection device provides the region code */
//  memory_install_read16_handler(cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_PROGRAM), 0x4f0000, 0x4fffff, 0, 0, sango_protram_r);

 	pgm_kovsh_decrypt(machine);
}

static DRIVER_INIT( djlzz )
{
	pgm_basic_init(machine);

	memory_install_readwrite16_handler(cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_PROGRAM), 0x500000, 0x500003, 0, 0, ASIC28_r16, ASIC28_w16);

	/* 0x4f0000 - ? is actually ram shared with the protection device,
      the protection device provides the region code */
	memory_install_read16_handler(cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_PROGRAM), 0x4f0000, 0x4fffff, 0, 0, sango_protram_r);

 	pgm_djlzz_decrypt(machine);
}

static DRIVER_INIT( dw3 )
{
	pgm_basic_init(machine);

//  memory_install_readwrite16_handler(cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_PROGRAM), 0xda0000, 0xdaffff, 0, 0, dw3_prot_r, dw3_prot_w);

 	pgm_dw3_decrypt(machine);
}


/* Killing Blade uses some kind of DMA protection device which can copy data from a data rom.  The
   MCU appears to have an internal ROM as if you remove the data ROM then the shared ram is filled
   with a constant value.

   The device can perform various decryption operations on the data it copies.  for now we're just
   using a dump of the shared RAM instead.  This will be improved later.
*/

static int kb_cmd;
static int reg;
static int ptr;

static WRITE16_HANDLER( killbld_prot_w )
{
//  mame_printf_debug("killbrd prot r\n");
//  return 0;
	offset&=0xf;

	if(offset==0)
		kb_cmd=data;
	else //offset==2
	{
		logerror("%06X: ASIC25 W CMD %X  VAL %X",cpu_get_pc(space->cpu),kb_cmd,data);
		if(kb_cmd==0)
			reg=data;
		else if(kb_cmd==2)
		{

			if(data==1)	//Execute cmd
			{
				UINT16 cmd=killbld_sharedprotram[0x200/2];
				//mame_printf_debug("command %04x\n",cmd);
				if(cmd==0x6d)	//Store values to asic ram
				{
					UINT32 p1=(killbld_sharedprotram[0x298/2] << 16) | killbld_sharedprotram[0x29a/2];
					UINT32 p2=(killbld_sharedprotram[0x29c/2] << 16) | killbld_sharedprotram[0x29e/2];
					static UINT32 Regs[0x10];
					if((p2&0xFFFF)==0x9)	//Set value
					{
						int reg=(p2>>16)&0xFFFF;
						if(reg&0x200)
							Regs[reg&0xFF]=p1;
					}
					if((p2&0xFFFF)==0x6)	//Add value
					{
						int src1=(p1>>16)&0xFF;
						int src2=(p1>>0)&0xFF;
						int dst=(p2>>16)&0xFF;
						Regs[dst]=Regs[src2]-Regs[src1];
					}
					if((p2&0xFFFF)==0x1)	//Add Imm?
					{
						int reg=(p2>>16)&0xFF;
						int imm=(p1>>0)&0xFFFF;
						Regs[reg]+=imm;
					}
					if((p2&0xFFFF)==0xa)	//Get value
					{
						int reg=(p1>>16)&0xFF;
						killbld_sharedprotram[0x29c/2] = (Regs[reg]>>16)&0xffff;
						killbld_sharedprotram[0x29e/2] = Regs[reg]&0xffff;
					}
				}
				if(cmd==0x4f)	//memcpy with encryption / scrambling
				{
					UINT16 src=killbld_sharedprotram[0x290/2]>>1; // ?
					UINT32 dst=killbld_sharedprotram[0x292/2];
					UINT16 size=killbld_sharedprotram[0x294/2];
					UINT16 mode=killbld_sharedprotram[0x296/2];


				//  int a=1;
				//  if(src==0x580)
				//      int a=1;
					/*
                    P_SRC =0x300290 (offset from prot rom base)
                    P_DST =0x300292 (words from 0x300000)
                    P_SIZE=0x300294 (words)
                    P_MODE=0x300296

                    Mode 5 direct
                    Mode 6 swap nibbles and bytes

                    1,2,3 unk.
                    */

					//mame_printf_debug("src %04x dst %04x size %04x mode %04x\n",src,dst,size,mode);

					//if (src&1) mame_printf_debug("odd offset\n");

					mode &=0xf;  // what are the other bits?

					if (mode == 1 || mode == 2 || mode == 3)
					{
						/* for now, cheat -- the scramble isn't understood, it might
                           be state based */
						int x;
						UINT16 *RAMDUMP = (UINT16*)memory_region(space->machine, "user2");
						for (x=0;x<size;x++)
						{
							UINT16 dat;

							dat = RAMDUMP[dst+x];
							killbld_sharedprotram[dst+x] = dat;
						}
					}
					else if (mode == 5)
					{
						/* mode 5 seems to be a straight copy */
						int x;
						UINT16 *RAMDUMP = (UINT16*)memory_region(space->machine, "user2");
						UINT16 *PROTROM = (UINT16*)memory_region(space->machine, "user1");
						for (x=0;x<size;x++)
						{
							UINT16 dat;
							dat = PROTROM[src+x];

							if (RAMDUMP[dst+x] != dat)
								mame_printf_debug("Mismatch! %04x %04x\n", RAMDUMP[dst+x], dat);

							killbld_sharedprotram[dst+x] = dat;
						}
					}
					else if (mode == 6)
					{
						/* mode 6 seems to swap bytes and nibbles */
						int x;
						UINT16 *RAMDUMP = (UINT16*)memory_region(space->machine, "user2");
						UINT16 *PROTROM = (UINT16*)memory_region(space->machine, "user1");
						for (x=0;x<size;x++)
						{
							UINT16 dat;
							dat = PROTROM[src+x];

							dat = ((dat & 0xf000) >> 12)|
								  ((dat & 0x0f00) >> 4)|
								  ((dat & 0x00f0) << 4)|
								  ((dat & 0x000f) << 12);


							if (RAMDUMP[dst+x] != dat)
								mame_printf_debug("Mismatch! Mode 6 %04x %04x\n", RAMDUMP[dst+x], dat);

							killbld_sharedprotram[dst+x] = dat;
						}
					}
					else
					{
						mame_printf_debug("unknown copy mode!\n");
					}
					/* hack.. it jumps here but there isn't valid code even when we do
                       use what was in ram.. probably some more protection as the game
                       still doesn't behave 100% correctly :-/

                       the code is copied in 'mode 3' but even the code put here on
                       the real ram dump is corrupt??? something _very_ strange is
                       going on.. maybe more rom overlays, or ram overlays too??

                    */
					killbld_sharedprotram[0x2600/2]=0x4e75;


				}
				reg++;
			}
		}
		else if(kb_cmd==4)
			ptr=data;
		else if(kb_cmd==0x20)
			ptr++;
	}
}

static READ16_HANDLER( killbld_prot_r )
{
//  mame_printf_debug("killbld prot w\n");
	UINT16 res ;

	offset&=0xf;
	res=0;

	if(offset==1)
	{
		if(kb_cmd==1)
		{
			res=reg&0x7f;
		}
		else if(kb_cmd==5)
		{
			UINT32 protvalue;
			protvalue = 0x89911400|input_port_read(space->machine, "Region");
			res=(protvalue>>(8*(ptr-1)))&0xff;

		}
	}
	logerror("%06X: ASIC25 R CMD %X  VAL %X",cpu_get_pc(space->cpu),kb_cmd,res);
	return res;
}

static MACHINE_RESET( killbld )
{
	int i;

	MACHINE_RESET_CALL(pgm);

	/* fill the protection ram with a5 */
	for (i = 0;i < 0x4000/2;i++)
		killbld_sharedprotram[i] = 0xa5a5;

}


static DRIVER_INIT( killbld )
{
	UINT16 *mem16 = (UINT16 *)memory_region(machine, "main");

	pgm_basic_init(machine);
 	pgm_killbld_decrypt(machine);



	/* this isn't a hack.. doing a rom dump while the game is running shows the
       rom space to look like this.. there may be more overlays / enables tho */

	/* the game actually performs a CRC check of the rom during the 'Please Wait'
       screen, the checksum expected is that of the patched rom.  if the checksum
       fails the please wait screen doesn't last as long and the region supplied
       by the protection device is ignored and the attract sequence appears out
       of order */
	mem16[0x108a2c/2]=0xB6AA;
	mem16[0x108a30/2]=0x6610;
	mem16[0x108a32/2]=0x13c2;
	mem16[0x108a34/2]=0x0080;
	mem16[0x108a36/2]=0x9c76;
	mem16[0x108a38/2]=0x23c3;
	mem16[0x108a3a/2]=0x0080;
	mem16[0x108a3c/2]=0x9c78;
	mem16[0x108a3e/2]=0x1002;
	mem16[0x108a40/2]=0x6054;
	mem16[0x108a42/2]=0x5202;
	mem16[0x108a44/2]=0x0c02;

	memory_install_readwrite16_handler(cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_PROGRAM), 0xd40000, 0xd40003, 0, 0, killbld_prot_r, killbld_prot_w);
}

/* ddp2 rubbish */

static UINT16 *ddp2_protram;
static int ddp2_asic27_0xd10000 = 0;

static WRITE16_HANDLER ( ddp2_asic27_0xd10000_w )
{
	ddp2_asic27_0xd10000=data;
}

static READ16_HANDLER ( ddp2_asic27_0xd10000_r )
{
	if (PGMLOGERROR) logerror("d100000_prot_r %04x, %04x\n", offset,ddp2_asic27_0xd10000);
	ddp2_asic27_0xd10000++;
	ddp2_asic27_0xd10000&=0x7f;
	return ddp2_asic27_0xd10000;
}


static READ16_HANDLER(ddp2_protram_r)
{
	if (PGMLOGERROR) logerror("prot_r %04x, %04x\n", offset,ddp2_protram[offset]);

	if (offset == 0x02/2) return input_port_read(space->machine, "Region");

	if (offset == 0x1f00/2) return 0;

	return ddp2_protram[offset];
}

static WRITE16_HANDLER(ddp2_protram_w)
{
	if (PGMLOGERROR) logerror("prot_w %04x, %02x\n", offset,data);
	COMBINE_DATA(&ddp2_protram[offset]);

	ddp2_protram[0x10/2] = 0;
	ddp2_protram[0x20/2] = 1;
}

static DRIVER_INIT( ddp2 )
{
	pgm_basic_init(machine);

	/* some kind of busy / counter */
	/* the actual protection is an arm cpu with internal rom */

	ddp2_protram = auto_malloc(0x2000);

	memory_install_readwrite16_handler(cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_PROGRAM), 0xd10000, 0xd10001, 0, 0, ddp2_asic27_0xd10000_r, ddp2_asic27_0xd10000_w);

	memory_install_readwrite16_handler(cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_PROGRAM), 0xd00000, 0xd01fff, 0, 0, ddp2_protram_r, ddp2_protram_w);
}

static DRIVER_INIT( puzzli2 )
{
	/* this protection emulation is wrong
     it uses an arm with no external rom
     an acts in a similar way to kov etc. */

	UINT16 *mem16 = (UINT16 *)memory_region(machine, "main");

	pgm_basic_init(machine);

	memory_install_readwrite16_handler(cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_PROGRAM), 0x500000, 0x500003, 0, 0, ASIC28_r16, ASIC28_w16);

	/* 0x4f0000 - ? is actually ram shared with the protection device,
      the protection device provides the region code */
	memory_install_read16_handler(cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_PROGRAM), 0x4f0000, 0x4fffff, 0, 0, sango_protram_r);

 	pgm_puzzli2_decrypt(machine);

	/* protection related? */
	mem16[0x1548ec/2]=0x4e71;
	mem16[0x1548fc/2]=0x4e71;
	mem16[0x1549FA/2]=0x4e71;
	mem16[0x154A0A/2]=0x4e71;
	mem16[0x15496A/2]=0x4e71;
	mem16[0x14cee0/2]=0x4e71;
	mem16[0x1268c0/2]=0x4e71;
	mem16[0x1268c2/2]=0x4e71;
	mem16[0x1268c4/2]=0x4e71;
	mem16[0x154948/2]=0x4e71;
	mem16[0x13877a/2]=0x662c;

	/* patch irq4 vector (irq4 should be disabled on this game? how?) */
//  mem16[0x100070/2]=0x0012;
//  mem16[0x100072/2]=0x5D78;
}

static MACHINE_RESET( olds )
{
	UINT16 *mem16 = (UINT16 *)memory_region(machine, "user2");
//  UINT16 *mem16_a = (UINT16 *)memory_region(machine, "main");
	int i;

	MACHINE_RESET_CALL(pgm);

	/* populate shared protection ram with data read from pcb .. */

//  for(i=0;i<0x100000/2;i++)
//  {
//      mem16_a[i+(0x300000/2)] = mem16[i];
//  }

	for(i=0;i<0x100000/2;i++)
	{
		olds_sharedprotram[i] = mem16[(0x100000/2)+i];

	}
}


static UINT16 olds_bs,olds_cmd3;


//UINT16 olds_r16(UINT32 addr)
static READ16_HANDLER( olds_r16 )
{
//  int offset = addr&0xf;
	UINT16 res ;
	res = 0;

	if(offset == 1)
	{
		if(kb_cmd == 1)
			res = reg&0x7f;
		if(kb_cmd == 2)
			res = olds_bs|0x80;
		if(kb_cmd == 3)
			res = olds_cmd3;
		else if(kb_cmd == 5)
		{
			UINT32 protvalue = 0x900000 | input_port_read(space->machine, "Region"); // region from protection device.
			res = (protvalue>>(8 * (ptr-1))) & 0xff; // includes region 1 = taiwan , 2 = china, 3 = japan (title = orlegend special), 4 = korea, 5 = hongkong, 6 = world

		}
	}
	logerror("%06X: ASIC25 R CMD %X  VAL %X\n",cpu_get_pc(space->cpu),kb_cmd,res);
	return res;
}

//void olds_w16(UINT32 addr,UINT16 data)
static WRITE16_HANDLER( olds_w16 )
{
//  int offset=addr&0xf;

	if(offset==0)
		kb_cmd=data;
	else //offset==2
	{
		logerror("%06X: ASIC25 W CMD %X  VAL %X\n",cpu_get_pc(space->cpu),kb_cmd,data);
		if(kb_cmd==0)
			reg=data;
		else if(kb_cmd==2)	//a bitswap=
		{
			int reg=0;
			if(data&0x01)
				reg|=0x40;
			if(data&0x02)
				reg|=0x80;
			if(data&0x04)
				reg|=0x20;
			if(data&0x08)
				reg|=0x10;
			olds_bs=reg;
		}
		else if(kb_cmd==3)
		{
			//UINT16 cmd=fast_r16(0x403026);
			UINT16 cmd = 0;
			if(cmd==0x12)	//memcpy
			{
			//  UINT16 src=fast_r16(0x40306A);
			//  UINT32 dst=0x400000+(fast_r16(0x403084)<<1);
			//  UINT16 size=fast_r16(0x4030A2);
			//  UINT16 mode=fast_r16(0x40303E)&0xf;
				//int a=1;
			}
			//else
			//  int a=1;
			olds_cmd3=((data>>4)+1)&0x3;
		}
		else if(kb_cmd==4)
			ptr=data;
		else if(kb_cmd==0x20)
		  ptr++;
	}
}




static DRIVER_INIT( olds )
{
//  UINT16 *mem16 = (UINT16 *)memory_region(machine, "main");

	pgm_basic_init(machine);

	memory_install_readwrite16_handler(cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_PROGRAM), 0xdcb400, 0xdcb403, 0, 0, olds_r16, olds_w16);

}

/*** Rom Loading *************************************************************/

/* take note of "gfx2" needed for expanding the 32x32x5bpp data and
   "gfx4" needed for expanding the Sprite Colour Data */

/* The Bios - NOT A GAME */
ROM_START( pgm )
	ROM_REGION( 0x600000, "main", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x00000, 0x20000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) )

	/* CPU2 = Z80, romless, code uploaded by 68k */

	ROM_REGION( 0x200000, "gfx1", 0 ) /* 8x8 Text Layer Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) )

	ROM_REGION( 0x200000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) )

	ROM_REGION( 0x800000/5*8, "gfx2", ROMREGION_DISPOSE | ROMREGION_ERASEFF ) /* Region for 32x32 BG Tiles */

	ROM_REGION( 0x1000000, "gfx4", ROMREGION_ERASEFF ) /* Sprite Masks + Colour Indexes */
ROM_END

ROM_START( orlegend )
	ROM_REGION( 0x600000, "main", 0 ) /* 68000 Code  */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) )// (BIOS)
	ROM_LOAD16_WORD_SWAP( "p0103.rom",    0x100000, 0x200000, CRC(d5e93543) SHA1(f081edc26514ca8354c13c7f6f89aba8e4d3e7d2) )

	/* CPU2 = Z80, romless, code uploaded by 68k */

	ROM_REGION( 0x800000, "gfx1",  ROMREGION_DISPOSE ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "t0100.rom",    0x400000, 0x400000, CRC(61425e1e) SHA1(20753b86fc12003cfd763d903f034dbba8010b32) )

	ROM_REGION( 0x800000/5*8, "gfx2", ROMREGION_DISPOSE | ROMREGION_ERASEFF ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x1800000, "gfx3", ROMREGION_DISPOSE ) /* Sprite Colour Data */
	ROM_LOAD( "a0100.rom",    0x0000000, 0x400000, CRC(8b3bd88a) SHA1(42db3a60c6ba9d83ebe2008c8047d094027f65a7) )
	ROM_LOAD( "a0101.rom",    0x0400000, 0x400000, CRC(3b9e9644) SHA1(5b95ec1d25c3bc3504c93547f5adb5ce24376405) )
	ROM_LOAD( "a0102.rom",    0x0800000, 0x400000, CRC(069e2c38) SHA1(9bddca8c2f5bd80f4abe4e1f062751736dc151dd) )
	ROM_LOAD( "a0103.rom",    0x0c00000, 0x400000, CRC(4460a3fd) SHA1(cbebdb65c17605853f7d0b298018dd8801a25a58) )
	ROM_LOAD( "a0104.rom",    0x1000000, 0x400000, CRC(5f8abb56) SHA1(6c1ddc0309862a141aa0c0f63b641aec9257aaee) )
	ROM_LOAD( "a0105.rom",    0x1400000, 0x400000, CRC(a17a7147) SHA1(44eeb43c6b0ebb829559a20ae357383fbdeecd82) )

	ROM_REGION( 0x1000000, "gfx4", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0100.rom",    0x0000000, 0x400000, CRC(69d2e48c) SHA1(5b5f759007264c07b3b39be8e03a713698e1fc2a) )
	ROM_LOAD( "b0101.rom",    0x0400000, 0x400000, CRC(0d587bf3) SHA1(5347828b0a6e4ddd7a263663d2c2604407e4d49c) )
	ROM_LOAD( "b0102.rom",    0x0800000, 0x400000, CRC(43823c1e) SHA1(e10a1a9a81b51b11044934ff702e35d8d7ab1b08) )

	ROM_REGION( 0x600000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
	ROM_LOAD( "m0100.rom",    0x400000, 0x200000, CRC(e5c36c83) SHA1(50c6f66770e8faa3df349f7d68c407a7ad021716) )
ROM_END

ROM_START( orlegnde )
	ROM_REGION( 0x600000, "main", 0 ) /* 68000 Code  */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) )// (BIOS)
	ROM_LOAD16_WORD_SWAP( "p0102.rom",    0x100000, 0x200000, CRC(4d0f6cc5) SHA1(8d41f0a712fb11a1da865f5159e5e27447b4388a) )

	/* CPU2 = Z80, romless, code uploaded by 68k */

	ROM_REGION( 0x800000, "gfx1",  ROMREGION_DISPOSE ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "t0100.rom",    0x400000, 0x400000, CRC(61425e1e) SHA1(20753b86fc12003cfd763d903f034dbba8010b32) )

	ROM_REGION( 0x800000/5*8, "gfx2", ROMREGION_DISPOSE | ROMREGION_ERASEFF ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x1800000, "gfx3", ROMREGION_DISPOSE ) /* Sprite Colour Data */
	ROM_LOAD( "a0100.rom",    0x0000000, 0x400000, CRC(8b3bd88a) SHA1(42db3a60c6ba9d83ebe2008c8047d094027f65a7) )
	ROM_LOAD( "a0101.rom",    0x0400000, 0x400000, CRC(3b9e9644) SHA1(5b95ec1d25c3bc3504c93547f5adb5ce24376405) )
	ROM_LOAD( "a0102.rom",    0x0800000, 0x400000, CRC(069e2c38) SHA1(9bddca8c2f5bd80f4abe4e1f062751736dc151dd) )
	ROM_LOAD( "a0103.rom",    0x0c00000, 0x400000, CRC(4460a3fd) SHA1(cbebdb65c17605853f7d0b298018dd8801a25a58) )
	ROM_LOAD( "a0104.rom",    0x1000000, 0x400000, CRC(5f8abb56) SHA1(6c1ddc0309862a141aa0c0f63b641aec9257aaee) )
	ROM_LOAD( "a0105.rom",    0x1400000, 0x400000, CRC(a17a7147) SHA1(44eeb43c6b0ebb829559a20ae357383fbdeecd82) )

	ROM_REGION( 0x1000000, "gfx4", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0100.rom",    0x0000000, 0x400000, CRC(69d2e48c) SHA1(5b5f759007264c07b3b39be8e03a713698e1fc2a) )
	ROM_LOAD( "b0101.rom",    0x0400000, 0x400000, CRC(0d587bf3) SHA1(5347828b0a6e4ddd7a263663d2c2604407e4d49c) )
	ROM_LOAD( "b0102.rom",    0x0800000, 0x400000, CRC(43823c1e) SHA1(e10a1a9a81b51b11044934ff702e35d8d7ab1b08) )

	ROM_REGION( 0x600000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
	ROM_LOAD( "m0100.rom",    0x400000, 0x200000, CRC(e5c36c83) SHA1(50c6f66770e8faa3df349f7d68c407a7ad021716) )
ROM_END

ROM_START( orlegndc )
	ROM_REGION( 0x600000, "main", 0 ) /* 68000 Code  */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) )// (BIOS)
	ROM_LOAD16_WORD_SWAP( "p0101.160",    0x100000, 0x200000, CRC(b24f0c1e) SHA1(a2cf75d739681f091c24ef78ed6fc13aa8cfe0c6) )

	/* CPU2 = Z80, romless, code uploaded by 68k */

	ROM_REGION( 0x800000, "gfx1",  ROMREGION_DISPOSE ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "t0100.rom",    0x400000, 0x400000, CRC(61425e1e) SHA1(20753b86fc12003cfd763d903f034dbba8010b32) )

	ROM_REGION( 0x800000/5*8, "gfx2", ROMREGION_DISPOSE | ROMREGION_ERASEFF ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x1800000, "gfx3", ROMREGION_DISPOSE ) /* Sprite Colour Data */
	ROM_LOAD( "a0100.rom",    0x0000000, 0x400000, CRC(8b3bd88a) SHA1(42db3a60c6ba9d83ebe2008c8047d094027f65a7) )
	ROM_LOAD( "a0101.rom",    0x0400000, 0x400000, CRC(3b9e9644) SHA1(5b95ec1d25c3bc3504c93547f5adb5ce24376405) )
	ROM_LOAD( "a0102.rom",    0x0800000, 0x400000, CRC(069e2c38) SHA1(9bddca8c2f5bd80f4abe4e1f062751736dc151dd) )
	ROM_LOAD( "a0103.rom",    0x0c00000, 0x400000, CRC(4460a3fd) SHA1(cbebdb65c17605853f7d0b298018dd8801a25a58) )
	ROM_LOAD( "a0104.rom",    0x1000000, 0x400000, CRC(5f8abb56) SHA1(6c1ddc0309862a141aa0c0f63b641aec9257aaee) )
	ROM_LOAD( "a0105.rom",    0x1400000, 0x400000, CRC(a17a7147) SHA1(44eeb43c6b0ebb829559a20ae357383fbdeecd82) )

	ROM_REGION( 0x1000000, "gfx4", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0100.rom",    0x0000000, 0x400000, CRC(69d2e48c) SHA1(5b5f759007264c07b3b39be8e03a713698e1fc2a) )
	ROM_LOAD( "b0101.rom",    0x0400000, 0x400000, CRC(0d587bf3) SHA1(5347828b0a6e4ddd7a263663d2c2604407e4d49c) )
	ROM_LOAD( "b0102.rom",    0x0800000, 0x400000, CRC(43823c1e) SHA1(e10a1a9a81b51b11044934ff702e35d8d7ab1b08) )

	ROM_REGION( 0x600000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
	ROM_LOAD( "m0100.rom",    0x400000, 0x200000, CRC(e5c36c83) SHA1(50c6f66770e8faa3df349f7d68c407a7ad021716) )
ROM_END

/*

Oriental Legend / Xi Yo Gi Shi Re Zuang (CHINA 111 Ver.)
(c)1997 IGS

PGM system
IGS PCB NO-0134-1
IGS PCB NO-0135


OLV111CH.U11 [b80ddd3c]
OLV111CH.U6  [5fb86373]
OLV111CH.U7  [6ee79faf]
OLV111CH.U9  [83cf09c8]

T0100.U8


A0100.U5
A0101.U6
A0102.U7
A0103.U8
A0104.U11
A0105.U12

B0100.U9
B0101.U10
B0102.U15

M0100.U1

*/

ROM_START( orld111c )
	ROM_REGION( 0x600000, "main", 0 ) /* 68000 Code  */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) )// (BIOS)
	ROM_LOAD16_BYTE( "olv111ch.u6",     0x100001, 0x080000, CRC(5fb86373) SHA1(2fc58eff1f38754c75819fde666244b867ca4f05) )
	ROM_LOAD16_BYTE( "olv111ch.u9",     0x100000, 0x080000, CRC(83cf09c8) SHA1(959780b45326059517f3008a356657f4f3d2908f) )
	ROM_LOAD16_BYTE( "olv111ch.u7",     0x200001, 0x080000, CRC(6ee79faf) SHA1(039b4b07b8577f0d3022ae01210c00375624cb3c) )
	ROM_LOAD16_BYTE( "olv111ch.u11",    0x200000, 0x080000, CRC(b80ddd3c) SHA1(55c700ce71ffdee392e03fd9d4719542c3527132) )

	/* CPU2 = Z80, romless, code uploaded by 68k */

	ROM_REGION( 0x800000, "gfx1",  ROMREGION_DISPOSE ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "t0100.rom",    0x400000, 0x400000, CRC(61425e1e) SHA1(20753b86fc12003cfd763d903f034dbba8010b32) )

	ROM_REGION( 0x800000/5*8, "gfx2", ROMREGION_DISPOSE | ROMREGION_ERASEFF ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x1800000, "gfx3", ROMREGION_DISPOSE ) /* Sprite Colour Data */
	ROM_LOAD( "a0100.rom",    0x0000000, 0x400000, CRC(8b3bd88a) SHA1(42db3a60c6ba9d83ebe2008c8047d094027f65a7) )
	ROM_LOAD( "a0101.rom",    0x0400000, 0x400000, CRC(3b9e9644) SHA1(5b95ec1d25c3bc3504c93547f5adb5ce24376405) )
	ROM_LOAD( "a0102.rom",    0x0800000, 0x400000, CRC(069e2c38) SHA1(9bddca8c2f5bd80f4abe4e1f062751736dc151dd) )
	ROM_LOAD( "a0103.rom",    0x0c00000, 0x400000, CRC(4460a3fd) SHA1(cbebdb65c17605853f7d0b298018dd8801a25a58) )
	ROM_LOAD( "a0104.rom",    0x1000000, 0x400000, CRC(5f8abb56) SHA1(6c1ddc0309862a141aa0c0f63b641aec9257aaee) )
	ROM_LOAD( "a0105.rom",    0x1400000, 0x400000, CRC(a17a7147) SHA1(44eeb43c6b0ebb829559a20ae357383fbdeecd82) )

	ROM_REGION( 0x1000000, "gfx4", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0100.rom",    0x0000000, 0x400000, CRC(69d2e48c) SHA1(5b5f759007264c07b3b39be8e03a713698e1fc2a) )
	ROM_LOAD( "b0101.rom",    0x0400000, 0x400000, CRC(0d587bf3) SHA1(5347828b0a6e4ddd7a263663d2c2604407e4d49c) )
	ROM_LOAD( "b0102.rom",    0x0800000, 0x400000, CRC(43823c1e) SHA1(e10a1a9a81b51b11044934ff702e35d8d7ab1b08) )

	ROM_REGION( 0x600000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
	ROM_LOAD( "m0100.rom",    0x400000, 0x200000, CRC(e5c36c83) SHA1(50c6f66770e8faa3df349f7d68c407a7ad021716) )
ROM_END

/*

Oriental Legend / Xi Yo Gi Shi Re Zuang (KOREA 105 Ver.)
(c)1997 IGS

PGM system
IGS PCB NO-0134-2
IGS PCB NO-0135


OLV105KO.U11 [40ae4d9e]
OLV105KO.U6  [b86703fe]
OLV105KO.U7  [5712facc]
OLV105KO.U9  [5a108e39]

T0100.U8


A0100.U5
A0101.U6
A0102.U7
A0103.U8
A0104.U11
A0105.U12

B0100.U9
B0101.U10
B0102.U15

M0100.U1

*/

ROM_START( orld105k )
	ROM_REGION( 0x600000, "main", 0 ) /* 68000 Code  */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) )// (BIOS)
	ROM_LOAD16_BYTE( "olv105ko.u6",     0x100001, 0x080000, CRC(b86703fe) SHA1(a3529b45efd400ecd5e76f764b528ebce46e24ab) )
	ROM_LOAD16_BYTE( "olv105ko.u9",     0x100000, 0x080000, CRC(5a108e39) SHA1(2033f4fe3f2dfd725dac535324f58348b9ac3914) )
	ROM_LOAD16_BYTE( "olv105ko.u7",     0x200001, 0x080000, CRC(5712facc) SHA1(2d95ebd1703874e89ac3a206f8c1f0ece6e833e0) )
	ROM_LOAD16_BYTE( "olv105ko.u11",    0x200000, 0x080000, CRC(40ae4d9e) SHA1(62d7a96438b7fe93f74753333f50e077d417971e) )

	/* CPU2 = Z80, romless, code uploaded by 68k */

	ROM_REGION( 0x800000, "gfx1",  ROMREGION_DISPOSE ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "t0100.rom",    0x400000, 0x400000, CRC(61425e1e) SHA1(20753b86fc12003cfd763d903f034dbba8010b32) )

	ROM_REGION( 0x800000/5*8, "gfx2", ROMREGION_DISPOSE | ROMREGION_ERASEFF ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x1800000, "gfx3", ROMREGION_DISPOSE ) /* Sprite Colour Data */
	ROM_LOAD( "a0100.rom",    0x0000000, 0x400000, CRC(8b3bd88a) SHA1(42db3a60c6ba9d83ebe2008c8047d094027f65a7) )
	ROM_LOAD( "a0101.rom",    0x0400000, 0x400000, CRC(3b9e9644) SHA1(5b95ec1d25c3bc3504c93547f5adb5ce24376405) )
	ROM_LOAD( "a0102.rom",    0x0800000, 0x400000, CRC(069e2c38) SHA1(9bddca8c2f5bd80f4abe4e1f062751736dc151dd) )
	ROM_LOAD( "a0103.rom",    0x0c00000, 0x400000, CRC(4460a3fd) SHA1(cbebdb65c17605853f7d0b298018dd8801a25a58) )
	ROM_LOAD( "a0104.rom",    0x1000000, 0x400000, CRC(5f8abb56) SHA1(6c1ddc0309862a141aa0c0f63b641aec9257aaee) )
	ROM_LOAD( "a0105.rom",    0x1400000, 0x400000, CRC(a17a7147) SHA1(44eeb43c6b0ebb829559a20ae357383fbdeecd82) )

	ROM_REGION( 0x1000000, "gfx4", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0100.rom",    0x0000000, 0x400000, CRC(69d2e48c) SHA1(5b5f759007264c07b3b39be8e03a713698e1fc2a) )
	ROM_LOAD( "b0101.rom",    0x0400000, 0x400000, CRC(0d587bf3) SHA1(5347828b0a6e4ddd7a263663d2c2604407e4d49c) )
	ROM_LOAD( "b0102.rom",    0x0800000, 0x400000, CRC(43823c1e) SHA1(e10a1a9a81b51b11044934ff702e35d8d7ab1b08) )

	ROM_REGION( 0x600000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
	ROM_LOAD( "m0100.rom",    0x400000, 0x200000, CRC(e5c36c83) SHA1(50c6f66770e8faa3df349f7d68c407a7ad021716) )
ROM_END

/*

Dragon World 2 (English / World Version)
IGS, 1997

This is a cart for the IGS PGM system.


PCB Layout
----------

IGS PCB NO-0162
|-----------------------------------------------|
| |------|                                      |
| |IGS012|       *1                    T0200.U7 |
| |      |                                      |
| |------|                                      |
|              |--------|                       |
|              |        |                       |
|              | IGS025 |  *2   V-110X.U2       |
| PAL    PAL   |        |                  PAL  |
|              |--------|                       |
|-|                                           |-|
  |--------------------||---------------------|
Notes:
      IGS012       - Custom IGS IC (QFP80)

      -- on english version
      IGS025       - Custom IGS IC (PLCC68, labelled "DRAGON II 0006")
      -- on china version
      IGS025       - Custom IGS IC (PLCC68, labelled "DRAGON II 0005")


      T0200.U7     - 32MBit MaskROM (SOP44)

      -- on english version
      V-110X.U2    - AM27C4096 4MBit EPROM (DIP42, labelled "DRAGON II V-110X")
      -- on china version
      V-110X.U2    - AM27C4096 4MBit EPROM (DIP42, labelled "DRAGON II V-100C")

      PALs         - x3, labelled "CZ U3", "CZ U4", "CZ U6"
      *1           - Unpopulated position for MX23C4100 SOP40 MASKROM
      *2           - Unpopulated position for MX23C4100 DIP40 EPROM/MASKROM


IGS PCB NO-0135
|-----------------------------------------------|
| U11    U12     U13      U14       U15      U16|
|                                               |
|                                               |
|A0200.U5                       B0200.U9        |
|        U6      U7       U8                 U10|
|                                               |
|                                               |
|74LS138         U1       U2             74LS139|
|                                               |
|-|                                           |-|
  |--------------------||---------------------|

Notes:
      This PCB contains only SOP44 MASKROMS and 2 logic IC's
      Only U5 and U9 are populated

      glitch on select screen exists on real board.

*/

ROM_START( drgw2 )
	ROM_REGION( 0x600000, "main", 0 ) /* 68000 Code  */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) )// (BIOS)
	ROM_LOAD16_WORD_SWAP( "v-110x.u2",    0x100000, 0x080000, CRC(1978106b) SHA1(af8a13d7783b755a58762c98bdc32cab845b2251) )

	/* CPU2 = Z80, romless, code uploaded by 68k */

	ROM_REGION( 0x800000, "gfx1",  ROMREGION_DISPOSE ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "pgmt0200.u7",    0x400000, 0x400000, CRC(b0f6534d) SHA1(174cacd81169a0e0d14790ac06d03caed737e05d) )

	ROM_REGION( 0x800000/5*8, "gfx2", ROMREGION_DISPOSE | ROMREGION_ERASEFF ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x400000, "gfx3", ROMREGION_DISPOSE ) /* Sprite Colour Data */
	ROM_LOAD( "pgma0200.u5",    0x0000000, 0x400000, CRC(13b95069) SHA1(4888b06002afb18eab81c010e9362629045767af) )

	ROM_REGION( 0x400000, "gfx4", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgmb0200.u9",    0x0000000, 0x400000, CRC(932d0f13) SHA1(4b8e008f9c617cb2b95effeb81abc065b30e5c86) )

	ROM_REGION( 0x400000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
ROM_END


ROM_START( drgw2c )
	ROM_REGION( 0x600000, "main", 0 ) /* 68000 Code  */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) )// (BIOS)
	ROM_LOAD16_WORD_SWAP( "v-100c.u2",    0x100000, 0x080000, CRC(67467981) SHA1(58af01a3871b6179fe42ff471cc39a2161940043) )

	/* CPU2 = Z80, romless, code uploaded by 68k */

	ROM_REGION( 0x800000, "gfx1",  ROMREGION_DISPOSE ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "pgmt0200.u7",    0x400000, 0x400000, CRC(b0f6534d) SHA1(174cacd81169a0e0d14790ac06d03caed737e05d) )

	ROM_REGION( 0x800000/5*8, "gfx2", ROMREGION_DISPOSE | ROMREGION_ERASEFF ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x400000, "gfx3", ROMREGION_DISPOSE ) /* Sprite Colour Data */
	ROM_LOAD( "pgma0200.u5",    0x0000000, 0x400000, CRC(13b95069) SHA1(4888b06002afb18eab81c010e9362629045767af) )

	ROM_REGION( 0x400000, "gfx4", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgmb0200.u9",    0x0000000, 0x400000, CRC(932d0f13) SHA1(4b8e008f9c617cb2b95effeb81abc065b30e5c86) )

	ROM_REGION( 0x400000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
ROM_END

ROM_START( drgw2j )
	ROM_REGION( 0x600000, "main", 0 ) /* 68000 Code  */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) )// (BIOS)
	ROM_LOAD16_WORD_SWAP( "v-100j.u2",    0x100000, 0x080000, CRC(f8f8393e) SHA1(ef0db668b4e4f661d4c1e95d57afe881bcdf13cc) )

	/* CPU2 = Z80, romless, code uploaded by 68k */

	ROM_REGION( 0x800000, "gfx1",  ROMREGION_DISPOSE ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "pgmt0200.u7",    0x400000, 0x400000, CRC(b0f6534d) SHA1(174cacd81169a0e0d14790ac06d03caed737e05d) )

	ROM_REGION( 0x800000/5*8, "gfx2", ROMREGION_DISPOSE | ROMREGION_ERASEFF ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x400000, "gfx3", ROMREGION_DISPOSE ) /* Sprite Colour Data */
	ROM_LOAD( "pgma0200.u5",    0x0000000, 0x400000, CRC(13b95069) SHA1(4888b06002afb18eab81c010e9362629045767af) )

	ROM_REGION( 0x400000, "gfx4", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgmb0200.u9",    0x0000000, 0x400000, CRC(932d0f13) SHA1(4b8e008f9c617cb2b95effeb81abc065b30e5c86) )

	ROM_REGION( 0x400000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
ROM_END

/*

Dragon World 3
Alta Co./IGS, 1998

Cart for IGS PGM system

Top board of cart contains.....
8MHz Xtal
32.768kHz Xtal
UM6164 (RAM x 2)
MACH211 CPLD
IGS022 ASIC
IGS025 ASIC
1x PAL
2x 27C040 EPROMs (main 68k program)
1x 27C512 EPROM (protection code?)
1x 32MBit smt MASKROM (T0400)

Bottom board contains.....
4x 32MBit smt MASKROMs (A0400, A0401, B0400, M0400)

*/

ROM_START( drgw3 )
	ROM_REGION( 0x600000, "main", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) )  // (BIOS)
	ROM_LOAD16_BYTE( "dw3_v100.u12",     0x100001, 0x080000,  CRC(47243906) SHA1(9cd46e3cba97f049bcb238ceb6edf27a760ef831) )
	ROM_LOAD16_BYTE( "dw3_v100.u13",     0x100000, 0x080000,  CRC(b7cded21) SHA1(c1ae2af2e42227503c81bbcd2bd6862aa416bd78) )

	/* CPU2 = Z80, romless, code uploaded by 68k */

	ROM_REGION( 0x010000, "user1", 0 ) /* Protection Data */
	ROM_LOAD( "dw3_v100.u15", 0x000000, 0x010000, CRC(03dc4fdf) SHA1(b329b04325d4f725231b1bb7862eedef2319b652) )

	ROM_REGION( 0xc00000, "gfx1", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "dw3t0400.u18",   0x400000, 0x400000, CRC(b70f3357) SHA1(8733969d7d21f540f295a9f747a4bb8f0d325cf0) )

	ROM_REGION( 0xc00000/5*8, "gfx2", ROMREGION_DISPOSE | ROMREGION_ERASEFF ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x1800000, "gfx3", ROMREGION_DISPOSE ) /* Sprite Colour Data */
	ROM_LOAD( "dw3a0400.u9",     0x0000000, 0x400000, CRC(dd7bfd40) SHA1(fb7ec5bf89a413c5208716083762a725ff63f5db) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "dw3a0401.u10",    0x0400000, 0x400000, CRC(cab6557f) SHA1(1904dd86645eea27ac1ab8a2462b20f6531356f8) ) // FIXED BITS (xxxxxxxx1xxxxxxx)

	ROM_REGION( 0x1000000, "gfx4", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "dw3b0400.u13",    0x0000000, 0x400000,  CRC(4bb87cc0) SHA1(71b2dc43fd11f7a6dffaba501e4e344b843583d8) ) // FIXED BITS (xxxxxxxx1xxxxxxx)

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
	ROM_LOAD( "dw3m0400.u1",  0x400000, 0x400000, CRC(031eb9ce) SHA1(0673ec194732becc6648c2ae1396e894aa269f9a) )
ROM_END

/*

Dragon World 3 (KOREA 106 Ver.)
(c)1998 IGS

PGM system
IGS PCB NO-0189
IGS PCB NO-0178


DW3_V106.U12 [c3f6838b]
DW3_V106.U13 [28284e22]


*/

ROM_START( drgw3k )
	ROM_REGION( 0x600000, "main", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) )  // (BIOS)
	ROM_LOAD16_BYTE( "dw3_v106.u12",     0x100001, 0x080000,  CRC(c3f6838b) SHA1(c135b1d4dd62af308139d40d03c29be7508fb1e7) )
	ROM_LOAD16_BYTE( "dw3_v106.u13",     0x100000, 0x080000,  CRC(28284e22) SHA1(4643a69881ddb7383ca10f3eb2aa2cf41be39e9f) )

	/* CPU2 = Z80, romless, code uploaded by 68k */

	ROM_REGION( 0x010000, "user1", 0 ) /* Protection Data - is it correct for this set? */
	ROM_LOAD( "dw3_v100.u15", 0x000000, 0x010000, CRC(03dc4fdf) SHA1(b329b04325d4f725231b1bb7862eedef2319b652) )

	ROM_REGION( 0xc00000, "gfx1", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "dw3t0400.u18",   0x400000, 0x400000, CRC(b70f3357) SHA1(8733969d7d21f540f295a9f747a4bb8f0d325cf0) )

	ROM_REGION( 0xc00000/5*8, "gfx2", ROMREGION_DISPOSE | ROMREGION_ERASEFF ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x1800000, "gfx3", ROMREGION_DISPOSE ) /* Sprite Colour Data */
	ROM_LOAD( "dw3a0400.u9",     0x0000000, 0x400000, CRC(dd7bfd40) SHA1(fb7ec5bf89a413c5208716083762a725ff63f5db) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "dw3a0401.u10",    0x0400000, 0x400000, CRC(cab6557f) SHA1(1904dd86645eea27ac1ab8a2462b20f6531356f8) ) // FIXED BITS (xxxxxxxx1xxxxxxx)

	ROM_REGION( 0x1000000, "gfx4", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "dw3b0400.u13",    0x0000000, 0x400000,  CRC(4bb87cc0) SHA1(71b2dc43fd11f7a6dffaba501e4e344b843583d8) ) // FIXED BITS (xxxxxxxx1xxxxxxx)

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
	ROM_LOAD( "dw3m0400.u1",  0x400000, 0x400000, CRC(031eb9ce) SHA1(0673ec194732becc6648c2ae1396e894aa269f9a) )
ROM_END

ROM_START( kov )
	ROM_REGION( 0x600000, "main", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) )  // (BIOS)
	ROM_LOAD16_WORD_SWAP( "p0600.117",    0x100000, 0x400000, CRC(c4d19fe6) SHA1(14ef31539bfbc665e76c9703ee01b12228344052) )

	/* CPU2 = Z80, romless, code uploaded by 68k */

	ROM_REGION( 0xc00000, "gfx1", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "t0600.rom",    0x400000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	ROM_REGION( 0xc00000/5*8, "gfx2", ROMREGION_DISPOSE | ROMREGION_ERASEFF ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x1c00000, "gfx3", ROMREGION_DISPOSE ) /* Sprite Colour Data */
	ROM_LOAD( "a0600.rom",    0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0601.rom",    0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0602.rom",    0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0603.rom",    0x1800000, 0x0400000, CRC(ec31abda) SHA1(ee526655369bae63b0ef0730e9768b765c9950fc) )

	ROM_REGION( 0x1000000, "gfx4", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0600.rom",    0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "b0601.rom",    0x0800000, 0x0400000, CRC(a0bb1c2f) SHA1(0542348c6e27779e0a98de16f04f9c18158f2b28) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
	ROM_LOAD( "m0600.rom",    0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END

ROM_START( kov115 )
	ROM_REGION( 0x600000, "main", 0 ) /* 68000 Code  */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) )// (BIOS)
	ROM_LOAD16_WORD_SWAP( "p0600.115",    0x100000, 0x400000, CRC(527a2924) SHA1(7e3b166dddc5245d7b408e78437c16fd2986d1d9) )

	/* CPU2 = Z80, romless, code uploaded by 68k */

	ROM_REGION( 0xc00000, "gfx1", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "t0600.rom",    0x400000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	ROM_REGION( 0xc00000/5*8, "gfx2", ROMREGION_DISPOSE | ROMREGION_ERASEFF ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x1c00000, "gfx3", ROMREGION_DISPOSE ) /* Sprite Colour Data */
	ROM_LOAD( "a0600.rom",    0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0601.rom",    0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0602.rom",    0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0603.rom",    0x1800000, 0x0400000, CRC(ec31abda) SHA1(ee526655369bae63b0ef0730e9768b765c9950fc) )

	ROM_REGION( 0x1000000, "gfx4", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0600.rom",    0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "b0601.rom",    0x0800000, 0x0400000, CRC(a0bb1c2f) SHA1(0542348c6e27779e0a98de16f04f9c18158f2b28) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
	ROM_LOAD( "m0600.rom",    0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END

/*

Sangoku Senki / Knights of Valour (JPN 100 Ver.)
(c)1999 ALTA / IGS

PGM system
IGS PCB NO-0212-1
IGS PCB NO-0213T


SAV111.U10   [d5536107]
SAV111.U4    [ae2f1b4e]
SAV111.U5    [5fdd4aa8]
SAV111.U7    [95eedf0e]
SAV111.U8    [003cbf49]

T0600.U11


A0600.U2
A0601.U4
A0602.U6
A0603.U9

M0600.U3

B0600.U5
B0601.U7

*/

ROM_START( kovj )
	ROM_REGION( 0x600000, "main", 0 ) /* 68000 Code  */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) )// (BIOS)
	ROM_LOAD16_BYTE( "sav111.u4",      0x100001, 0x080000, CRC(ae2f1b4e) SHA1(2ac9d84f5dee52f374941cfd68e2b98ecad436a8) )
	ROM_LOAD16_BYTE( "sav111.u7",      0x100000, 0x080000, CRC(95eedf0e) SHA1(582a54e9a1eda7ff73e20f0e69d2d50141772378) )
	ROM_LOAD16_BYTE( "sav111.u5",      0x200001, 0x080000, CRC(5fdd4aa8) SHA1(43c96e21ad4f11148e1e94a59c53780b2edd43ba) )
	ROM_LOAD16_BYTE( "sav111.u8",      0x200000, 0x080000, CRC(003cbf49) SHA1(fb5bea47ecae025b1b425af52cd05e061f45e377) )
	ROM_LOAD16_WORD_SWAP( "sav111.u10",0x300000, 0x080000, CRC(d5536107) SHA1(f963e015d99c1621323eecf63e773c0b9f4b6a43) )

	/* CPU2 = Z80, romless, code uploaded by 68k */

	ROM_REGION( 0xc00000, "gfx1", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "t0600.rom",    0x400000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	ROM_REGION( 0xc00000/5*8, "gfx2", ROMREGION_DISPOSE | ROMREGION_ERASEFF ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x1c00000, "gfx3", ROMREGION_DISPOSE ) /* Sprite Colour Data */
	ROM_LOAD( "a0600.rom",    0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0601.rom",    0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0602.rom",    0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0603.rom",    0x1800000, 0x0400000, CRC(ec31abda) SHA1(ee526655369bae63b0ef0730e9768b765c9950fc) )

	ROM_REGION( 0x1000000, "gfx4", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0600.rom",    0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "b0601.rom",    0x0800000, 0x0400000, CRC(a0bb1c2f) SHA1(0542348c6e27779e0a98de16f04f9c18158f2b28) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
	ROM_LOAD( "m0600.rom",    0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END

ROM_START( kovplus )
	ROM_REGION( 0x600000, "main", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) ) // (BIOS)
	ROM_LOAD16_WORD_SWAP( "p0600.119",    0x100000, 0x400000, CRC(e4b0875d) SHA1(e8382e131b0e431406dc2a05cc1ef128302d987c) )

	/* CPU2 = Z80, romless, code uploaded by 68k */

	ROM_REGION( 0xc00000, "gfx1", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "t0600.rom",    0x400000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	ROM_REGION( 0xc00000/5*8, "gfx2", ROMREGION_DISPOSE | ROMREGION_ERASEFF ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x1c00000, "gfx3", ROMREGION_DISPOSE ) /* Sprite Colour Data */
	ROM_LOAD( "a0600.rom",    0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0601.rom",    0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0602.rom",    0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0603.rom",    0x1800000, 0x0400000, CRC(ec31abda) SHA1(ee526655369bae63b0ef0730e9768b765c9950fc) )

	ROM_REGION( 0x1000000, "gfx4", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0600.rom",    0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "b0601.rom",    0x0800000, 0x0400000, CRC(a0bb1c2f) SHA1(0542348c6e27779e0a98de16f04f9c18158f2b28) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
	ROM_LOAD( "m0600.rom",    0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END

/*

Sangoku Senki Plus / Knights of Valour Plus (Alt 119 Ver.)
(c)1999 IGS

PGM system
IGS PCB NO-0222
IGS PCB NO-0213


V119.U2      [29588ef2]
V119.U3      [6750388f]
V119.U4      [8200ece6]
V119.U5      [d4101ffd]
V119.U6      [71e28f27]

T0600.U11


A0600.U2
A0601.U4
A0602.U6
A0603.U9

M0600.U3

B0600.U5
B0601.U7

*/

ROM_START( kovplusa )
	ROM_REGION( 0x600000, "main", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) ) // (BIOS)
	ROM_LOAD16_BYTE( "v119.u3",     0x100001, 0x080000, CRC(6750388f) SHA1(869f4ad27f2992cc62baa9a78bf7984a43ec4cc5) )
	ROM_LOAD16_BYTE( "v119.u5",     0x100000, 0x080000, CRC(d4101ffd) SHA1(a327fd56eec65b07df9305cd93ef2c46bf8e40f3) )
	ROM_LOAD16_BYTE( "v119.u4",     0x200001, 0x080000, CRC(8200ece6) SHA1(97081d2e8aed2ac6fbe5951890aecea18af5ce2e) )
	ROM_LOAD16_BYTE( "v119.u6",     0x200000, 0x080000, CRC(71e28f27) SHA1(db382807e9185f0dc17124f210165fa1b36ca6ac) )
	ROM_LOAD16_WORD_SWAP( "v119.u2",0x300000, 0x080000, CRC(29588ef2) SHA1(17d1a308d44434cf65224a24360cf4b6e32d28f3) )

	/* CPU2 = Z80, romless, code uploaded by 68k */

	ROM_REGION( 0xc00000, "gfx1", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "t0600.rom",    0x400000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	ROM_REGION( 0xc00000/5*8, "gfx2", ROMREGION_DISPOSE | ROMREGION_ERASEFF ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x1c00000, "gfx3", ROMREGION_DISPOSE ) /* Sprite Colour Data */
	ROM_LOAD( "a0600.rom",    0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0601.rom",    0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0602.rom",    0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0603.rom",    0x1800000, 0x0400000, CRC(ec31abda) SHA1(ee526655369bae63b0ef0730e9768b765c9950fc) )

	ROM_REGION( 0x1000000, "gfx4", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0600.rom",    0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "b0601.rom",    0x0800000, 0x0400000, CRC(a0bb1c2f) SHA1(0542348c6e27779e0a98de16f04f9c18158f2b28) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
	ROM_LOAD( "m0600.rom",    0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END

/* is this loading the right roms? */
ROM_START( kovsh )
	ROM_REGION( 0x600000, "main", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) )  // (BIOS)
	ROM_LOAD16_WORD_SWAP( "p0600.322",    0x100000, 0x400000, CRC(7c78e5f3) SHA1(9b1e4bd63fb1294ebeb539966842273c8dc7683b) )

	/* CPU2 = Z80, romless, code uploaded by 68k */

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kovsh_v100_china.asic", 0x000000, 0x04000,  CRC(0f09a5c1) SHA1(621b38c05f33277608d58b49822aebc930ae4870) )

	ROM_REGION( 0x400000, "user1", ROMREGION_ERASE00 )
	/* unpopulated (needs to return 0) */

	ROM_REGION( 0xc00000, "gfx1", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "t0600.320",    0x400000, 0x400000, CRC(164b3c94) SHA1(f00ea66886ca6bff74bbeaa49e7f5c75c275d5d7) ) // bad? its half the size of the kov one

	ROM_REGION( 0xc00000/5*8, "gfx2", ROMREGION_DISPOSE | ROMREGION_ERASEFF ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	/* all roms below need checking to see if they're the same on this board */
	ROM_REGION( 0x1c00000, "gfx3", ROMREGION_DISPOSE ) /* Sprite Colour Data */
	ROM_LOAD( "a0600.rom",    0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0601.rom",    0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0602.rom",    0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0603.rom",    0x1800000, 0x0400000, CRC(ec31abda) SHA1(ee526655369bae63b0ef0730e9768b765c9950fc) )

	ROM_REGION( 0x1000000, "gfx4", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0600.rom",    0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "b0601.rom",    0x0800000, 0x0400000, CRC(a0bb1c2f) SHA1(0542348c6e27779e0a98de16f04f9c18158f2b28) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
	ROM_LOAD( "m0600.rom",    0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END

ROM_START( photoy2k )
	ROM_REGION( 0x600000, "main", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) )  // (BIOS)
	ROM_LOAD16_WORD_SWAP( "v104.16m",     0x100000, 0x200000, CRC(e051070f) SHA1(a5a1a8dd7542a30632501af8d02fda07475fd9aa) )

	/* CPU2 = Z80, romless, code uploaded by 68k */

	ROM_REGION( 0x4000, "cpu2", 0 ) /* ARM protection ASIC - internal rom, supplies region code etc. */
	/* not hooked up yet (simulation code used instead for now) */
	ROM_LOAD( "photoy2k_v100_china.asic", 0x000000, 0x04000,  CRC(6dd7f257) SHA1(1984f98a282d8b3264674f231c3b7def1757cf72) )

	ROM_REGION( 0x480000, "gfx1", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "t0700.rom",    0x400000, 0x080000, CRC(93943b4d) SHA1(3b439903853727d45d62c781af6073024eb3c5a3) )

	ROM_REGION( 0x480000/5*8, "gfx2", ROMREGION_DISPOSE | ROMREGION_ERASEFF ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	/* all roms below need checking to see if they're the same on this board */
	ROM_REGION( 0x1080000, "gfx3", ROMREGION_DISPOSE ) /* Sprite Colour Data */
	ROM_LOAD( "a0700.l",    0x0000000, 0x0400000, CRC(26a9ae9c) SHA1(c977c89db6fdf47ee260ff687b80375caeab975c) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0700.h",    0x0400000, 0x0400000, CRC(79bc1fc1) SHA1(a09472a9b75704c1d31ab828f92c2a5007b2b4ed) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0701.l",    0x0800000, 0x0400000, CRC(23607f81) SHA1(8b6dbcdce9b131370693847ed9771aa04b62711c) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0701.h",    0x0c00000, 0x0400000, CRC(5f2efd37) SHA1(9a5bd9751691bc085b0751b9fa8ede9eb97b1248) )
	ROM_LOAD( "a0702.rom",  0x1000000, 0x0080000, CRC(42239e1b) SHA1(2b6d20958abf8a67ce525d5c8964b6d225ccaeda) )

	ROM_REGION( 0x1000000, "gfx4", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0700.l",    0x0000000, 0x0400000, CRC(af096904) SHA1(8e86b36cc44720ece68022e409279bf9144341ba) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "b0700.h",    0x0400000, 0x0400000, CRC(6d53de26) SHA1(f3f93fd2f87adb815834ba0242b94073fbb5e333) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "cgv101.rom", 0x0800000, 0x0020000, CRC(da02ec3e) SHA1(7ee21d748c9b932f53e790a9040167f904fecefc) )

	ROM_REGION( 0x480000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
	ROM_LOAD( "m0700.rom",    0x400000, 0x080000, CRC(acc7afce) SHA1(ac2d344ebac336f0f363bb045dd8ea4e83d1fb50) )
ROM_END

/*

Real and Fake / Photo Y2K (JPN 102 Ver.)
(c)1999 ALTA / IGS

PGM system
IGS PCB NO-0220
IGS PCB NO-0221


V102.U4      [a65eda9f]
V102.U5      [9201621b]
V102.U6      [b9ca5504]
V102.U8      [3be22b8f]

T0700.U11


A0700.U2
A0701.U4

SP_V102.U5

B0700.U7

CG_V101.U3
CG_V101.U6

*/

ROM_START( raf102j )
	ROM_REGION( 0x600000, "main", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) )  // (BIOS)
	ROM_LOAD16_BYTE( "v102.u4",     0x100001, 0x080000, CRC(a65eda9f) SHA1(6307cacf4a262e781753eff14700a0455837780c) )
	ROM_LOAD16_BYTE( "v102.u6",     0x100000, 0x080000, CRC(b9ca5504) SHA1(058cf01316f233236ca9861349f515935283b75e) )
	ROM_LOAD16_BYTE( "v102.u5",     0x200001, 0x080000, CRC(9201621b) SHA1(1ca3ebe7eec40614bfa8b911657fa2b51f2c51a4) )
	ROM_LOAD16_BYTE( "v102.u8",     0x200000, 0x080000, CRC(3be22b8f) SHA1(03634fbd6a8a8369c6cb1fd6694a3784dac5bf59) )

	/* CPU2 = Z80, romless, code uploaded by 68k */

	ROM_REGION( 0x480000, "gfx1", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "t0700.rom",    0x400000, 0x080000, CRC(93943b4d) SHA1(3b439903853727d45d62c781af6073024eb3c5a3) )

	ROM_REGION( 0x480000/5*8, "gfx2", ROMREGION_DISPOSE | ROMREGION_ERASEFF ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	/* all roms below need checking to see if they're the same on this board */
	ROM_REGION( 0x1080000, "gfx3", ROMREGION_DISPOSE ) /* Sprite Colour Data */
	ROM_LOAD( "a0700.l",    0x0000000, 0x0400000, CRC(26a9ae9c) SHA1(c977c89db6fdf47ee260ff687b80375caeab975c) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0700.h",    0x0400000, 0x0400000, CRC(79bc1fc1) SHA1(a09472a9b75704c1d31ab828f92c2a5007b2b4ed) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0701.l",    0x0800000, 0x0400000, CRC(23607f81) SHA1(8b6dbcdce9b131370693847ed9771aa04b62711c) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0701.h",    0x0c00000, 0x0400000, CRC(5f2efd37) SHA1(9a5bd9751691bc085b0751b9fa8ede9eb97b1248) )
	ROM_LOAD( "a0702.rom",  0x1000000, 0x0080000, CRC(42239e1b) SHA1(2b6d20958abf8a67ce525d5c8964b6d225ccaeda) )

	ROM_REGION( 0x1000000, "gfx4", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0700.l",    0x0000000, 0x0400000, CRC(af096904) SHA1(8e86b36cc44720ece68022e409279bf9144341ba) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "b0700.h",    0x0400000, 0x0400000, CRC(6d53de26) SHA1(f3f93fd2f87adb815834ba0242b94073fbb5e333) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "cgv101.rom", 0x0800000, 0x0020000, CRC(da02ec3e) SHA1(7ee21d748c9b932f53e790a9040167f904fecefc) )

	ROM_REGION( 0x480000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
	ROM_LOAD( "m0700.rom",    0x400000, 0x080000, CRC(acc7afce) SHA1(ac2d344ebac336f0f363bb045dd8ea4e83d1fb50) )
ROM_END

/*

The Killing Blade (English / World Version)
IGS, 1998

This is a cart for the IGS PGM system.


PCB Layout
----------

IGS PCB NO-0179
|-----------------------------------------------|
|                      8MHz  |--------|         |
|            32.768kHz|----| |        |T0300.U14|
|6164  6164           |IGS | | IGS025 |         |
|                     |022 | |        |         |
|*                    |----| |--------|         |
|                                               |
|           U2     U3     U4     U5     U6      |
| PAL   PAL                                PAL  |
|                                               |
|-|                                           |-|
  |--------------------||---------------------|
Notes:
      IGS022       - Custom IGS IC (QFP100)
      IGS025       - Custom IGS IC (PLCC68, labelled "ENGLISH")
      T0300.U14    - 32MBit MaskROM (SOP44, labelled "T0300")
      6164         - x2, 8K x8 SRAM (SOJ28)
      U2           - 27C512 512KBit EPROM (DIP28, labelled "KB U2 V104")
      U3           - 27C4000 4MBit EPROM (DIP32, labelled "KB U3 V104")
      U4           - 27C4000 4MBit EPROM (DIP32, labelled "KB U4 V104")
      U5           - 27C4000 4MBit EPROM (DIP32, labelled "KB U5 V104")
      U6           - 27C4000 4MBit EPROM (DIP32, labelled "KB U6 V104")
      PALs         - x3, labelled "DH U8", "DH U1", "DH U7"
      *            - Unpopulated position for DIP42 EPROM/MASKROM (labelled "P0300")


IGS PCB NO-0178
|-----------------------------------------------|
| U9    U10   U11    U12     U13     U14     U15|
|                                               |
|                                               |
|                                               |
| U1    U2                         74LS138      |
|                                  74LS139      |
|             U3     U4      U5              U8 |
|                                               |
|                                               |
|-|                                           |-|
  |--------------------||---------------------|

Notes:
      U1           - 32MBit MASKROM (SOP44, labelled "M0300")
      U2           - 32MBit MASKROM (SOP44, labelled "A0307")
      U3           - 16MBit MASKROM (DIP42, labelled "A0302")
      U4           - 16MBit MASKROM (DIP42, labelled "A0304")
      U5           - 16MBit MASKROM (DIP42, labelled "A0305")
      U8           - 16MBit MASKROM (DIP42, labelled "B0301")
      U9           - 32MBit MASKROM (SOP44, labelled "A0300")
      U10          - 32MBit MASKROM (SOP44, labelled "A0301")
      U11          - 32MBit MASKROM (SOP44, labelled "A0303")
      U12          - 32MBit MASKROM (SOP44, labelled "A0306")
      U13          - 32MBit MASKROM (SOP44, labelled "B0300")
      U14          - 32MBit MASKROM (SOP44, labelled "B0302")
      U15          - 32MBit MASKROM (SOP44, labelled "B0303")

*/

ROM_START( killbld )
	ROM_REGION( 0x600000, "main", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) )  // (BIOS)
	ROM_LOAD16_BYTE( "kb_u3_v104.u3",     0x100001, 0x080000, CRC(6db1d719) SHA1(804002f014d275aaf0368fb7f904938fe4ac07ee) )
	ROM_LOAD16_BYTE( "kb_u6_v104.u6",     0x100000, 0x080000, CRC(31ecc978) SHA1(82666d534e4151775063af6d39f575faba0f1047) )
	ROM_LOAD16_BYTE( "kb_u4_v104.u4",     0x200001, 0x080000, CRC(1ed8b2e7) SHA1(331c037640cfc1fe743cd0e65a1156c470b3303e) ) // order?
	ROM_LOAD16_BYTE( "kb_u5_v104.u5",     0x200000, 0x080000, CRC(a0bafc29) SHA1(b20db7c16353c6f87ed3c08c9d037b07336711f1) ) // order?

	ROM_REGION( 0x4000, "user2", ROMREGION_ERASEFF )

	/* CPU2 = Z80, romless, code uploaded by 68k */

	ROM_REGION( 0x010000, "user1", 0 ) /* Protection Data */
	ROM_LOAD16_WORD_SWAP( "kb_u2_v104.u2", 0x000000, 0x010000,  CRC(c970f6d5) SHA1(399fc6f80262784c566363c847dc3fdc4fb37494) )

	ROM_REGION( 0x800000, "gfx1", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "t0300.u14",    0x400000, 0x400000, CRC(0922f7d9) SHA1(4302b4b7369e13f315fad14f7d6cad1321101d24) )

	ROM_REGION( 0x800000/5*8, "gfx2", ROMREGION_DISPOSE | ROMREGION_ERASEFF ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x2000000, "gfx3", ROMREGION_DISPOSE ) /* Sprite Colour Data */
	ROM_LOAD( "a0300.u9",   0x0000000, 0x0400000,  CRC(3f9455d3) SHA1(3718ce00ad93975383aafc14e5a74dc297b011a1) )
	ROM_LOAD( "a0301.u10",  0x0400000, 0x0400000,  CRC(92776889) SHA1(6d677837fefff47bfd1c6166322f69f89989a5e2) )
	ROM_LOAD( "a0303.u11",  0x0800000, 0x0400000,  CRC(33f5cc69) SHA1(9cacd5058d4bb25b77f71658bbbbd4b38d0a6b6a) )
	ROM_LOAD( "a0306.u12",  0x0c00000, 0x0400000,  CRC(cc018a8e) SHA1(37752d46f238fb57c0ab5a4f96b1e013f2077347) )
	ROM_LOAD( "a0307.u2",   0x1000000, 0x0400000,  CRC(bc772e39) SHA1(079cc42a190cb916f02b59bca8fa90e524acefe9) )
//  ROM_LOAD( "a0302.u3",   0x1400000, 0x0200000,  CRC(a4810e38) SHA1(c31fe641feab2c93795fc35bf71d4f37af1056d4) ) // from lord of gun! unused..
//  ROM_LOAD( "a0304.u4",   0x1600000, 0x0200000,  CRC(3096de1c) SHA1(d010990d21cfda9cb8ab5b4bc0e329c23b7719f5) ) // from lord of gun! unused..
//  ROM_LOAD( "a0305.u5",   0x1800000, 0x0200000,  CRC(2234531e) SHA1(58a82e31a1c0c1a4dd026576319f4e7ecffd140e) ) // from lord of gun! unused..

	ROM_REGION( 0x1000000, "gfx4", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0300.u13",    0x0000000, 0x0400000, CRC(7f876981) SHA1(43555a200929ad5ecc42137fc9aeb42dc4f50d20) )
	ROM_LOAD( "b0302.u14",    0x0400000, 0x0400000, CRC(eea9c502) SHA1(04b3972c7111ea59a3cceab6ad124080c4ce3520) )
	ROM_LOAD( "b0303.u15",    0x0800000, 0x0200000, CRC(77a9652e) SHA1(2342f643d37945fbda224a5034c013796e5134ca) )
//  ROM_LOAD( "b0301.u8",     0x0a00000, 0x0200000, CRC(400abe33) SHA1(20de1eb626424ea41bd55eb3cecd6b50be744ee0) ) // from lord of gun! unused..

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
	ROM_LOAD( "m0300.u1",     0x400000, 0x400000, CRC(93159695) SHA1(50c5976c9b681bd3d1ebefa3bfa9fe6e72dcb96f) )
ROM_END

ROM_START( killbldt )
	ROM_REGION( 0x600000, "main", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) )  // (BIOS)
	ROM_LOAD16_WORD_SWAP( "kb.u9", 0x100000, 0x200000, BAD_DUMP CRC(43da77d7) SHA1(f99e89da4587d6c9e3c2ae66fa139830d893fdda) ) // not verified to be correct

	ROM_REGION( 0x4000, "user2", 0 ) /* dump of RAM shared with protection device, todo, emulate protection device instead! */
	ROM_LOAD( "kb.ram", 0x000000, 0x04000,  CRC(6994c507) SHA1(8264c56709488b72282d6ddfce3a4b188c6cc109) )

	/* CPU2 = Z80, romless, code uploaded by 68k */

	ROM_REGION( 0x010000, "user1", 0 ) /* Protection Data */
	ROM_LOAD16_WORD_SWAP( "kb_u2.rom", 0x000000, 0x010000,  CRC(de3eae63) SHA1(03af767ef764055bda528b5cc6a24b9e1218cca8) )

	ROM_REGION( 0x800000, "gfx1", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "t0300.u14",    0x400000, 0x400000, CRC(0922f7d9) SHA1(4302b4b7369e13f315fad14f7d6cad1321101d24) )

	ROM_REGION( 0x800000/5*8, "gfx2", ROMREGION_DISPOSE | ROMREGION_ERASEFF ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x2000000, "gfx3", ROMREGION_DISPOSE ) /* Sprite Colour Data */
	ROM_LOAD( "a0300.u9",   0x0000000, 0x0400000,  CRC(3f9455d3) SHA1(3718ce00ad93975383aafc14e5a74dc297b011a1) )
	ROM_LOAD( "a0301.u10",  0x0400000, 0x0400000,  CRC(92776889) SHA1(6d677837fefff47bfd1c6166322f69f89989a5e2) )
	ROM_LOAD( "a0303.u11",  0x0800000, 0x0400000,  CRC(33f5cc69) SHA1(9cacd5058d4bb25b77f71658bbbbd4b38d0a6b6a) )
	ROM_LOAD( "a0306.u12",  0x0c00000, 0x0400000,  CRC(cc018a8e) SHA1(37752d46f238fb57c0ab5a4f96b1e013f2077347) )
	ROM_LOAD( "a0307.u2",   0x1000000, 0x0400000,  CRC(bc772e39) SHA1(079cc42a190cb916f02b59bca8fa90e524acefe9) )
//  ROM_LOAD( "a0302.u3",   0x1400000, 0x0200000,  CRC(a4810e38) SHA1(c31fe641feab2c93795fc35bf71d4f37af1056d4) ) // from lord of gun! unused..
//  ROM_LOAD( "a0304.u4",   0x1600000, 0x0200000,  CRC(3096de1c) SHA1(d010990d21cfda9cb8ab5b4bc0e329c23b7719f5) ) // from lord of gun! unused..
//  ROM_LOAD( "a0305.u5",   0x1800000, 0x0200000,  CRC(2234531e) SHA1(58a82e31a1c0c1a4dd026576319f4e7ecffd140e) ) // from lord of gun! unused..

	ROM_REGION( 0x1000000, "gfx4", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0300.u13",    0x0000000, 0x0400000, CRC(7f876981) SHA1(43555a200929ad5ecc42137fc9aeb42dc4f50d20) )
	ROM_LOAD( "b0302.u14",    0x0400000, 0x0400000, CRC(eea9c502) SHA1(04b3972c7111ea59a3cceab6ad124080c4ce3520) )
	ROM_LOAD( "b0303.u15",    0x0800000, 0x0200000, CRC(77a9652e) SHA1(2342f643d37945fbda224a5034c013796e5134ca) )
//  ROM_LOAD( "b0301.u8",     0x0a00000, 0x0200000, CRC(400abe33) SHA1(20de1eb626424ea41bd55eb3cecd6b50be744ee0) ) // from lord of gun! unused..

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
	ROM_LOAD( "m0300.u1",     0x400000, 0x400000, CRC(93159695) SHA1(50c5976c9b681bd3d1ebefa3bfa9fe6e72dcb96f) )
ROM_END


/*
Puzzle Star
IGS, 1999

Cart for IGS PGM system. This game is a 'columns' type game.

PCB Layout
----------

IGS PCB NO- T0236
|-----------------------------------------------|
|                        U6 U7                  |
|         |-------|                             |
|         |IGS027A|                             |
|         |       |                     T0800.U5|
|         |       |                             |
|         |-------|                             |
|          U1_V100MG.U1                         |
|          U2_V100MG.U2   U3   PAL              |
|                                               |
|-|                                           |-|
  |--------------------||---------------------|
Notes:
      IGS027A       - Custom IGS IC, ARM7/9? based CPU (QFP120, stamped 'IGS027A' & labelled 'ENGLISH')
      T0800.U5      - 16MBit MaskROM (DIP42)
      U1_V100MG.U1  - MX27C4000 512K x8 EPROM (DIP32, labelled 'PuzzleStar U1 V100MG')
      U2_V100MG.U2  - MX27C4000 512K x8 EPROM (DIP32, labelled 'PuzzleStar U2 V100MG')
      PAL           - Atmel ATF22V10B PAL (DIP24, labelled 'EA U4')
      U3            - Unpopulated position for 32MBit MASKROM (DIP42)
      U6, U7        - Unpopulated position for 74LS245 logic chip (x2)


IGS PCB NO- T0237
|-----------------------------------------------|
|                                               |
|                                               |
|                                               |
|                                               |
|       A0800.U1   M0800.U2   B0800.U3          |
|                                               |
|                                               |
|                                               |
|                                               |
|-|                                           |-|
  |--------------------||---------------------|
Notes:
      U1 - 32MBit MaskROM (DIP42)
      U2 - 32MBit MaskROM (DIP42)
      U3 - 16MBit MaskROM (DIP42)
*/

ROM_START( puzlstar )
	ROM_REGION( 0x600000, "main", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) ) // (BIOS)
	ROM_LOAD16_BYTE( "v100mg.u1",     0x100001, 0x080000, CRC(5788b77d) SHA1(7770aae6e686da92b2623c977d1bc8f019f48267) )
	ROM_LOAD16_BYTE( "v100mg.u2",     0x100000, 0x080000, CRC(4c79d979) SHA1(3b92052a35994f2b3dd164930154184c45d5e2d0) )

	/* CPU2 = Z80, romless, code uploaded by 68k */

	ROM_REGION( 0x4000, "cpu2", ROMREGION_ERASEFF ) /* ARM protection ASIC - internal rom */
	/* this has no external rom so the internal rom probably can't be dumped */
//  ROM_LOAD( "puzlstar_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xc00000, "gfx1", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "t0800.u5",    0x400000, 0x200000, CRC(f9d84e59) SHA1(80ec77025ac5bf355b1a60f2a678dd4c56071f6b) )

	ROM_REGION( 0xc00000/5*8, "gfx2", ROMREGION_DISPOSE | ROMREGION_ERASEFF ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x1c00000, "gfx3", ROMREGION_DISPOSE ) /* Sprite Colour Data */
	ROM_LOAD( "a0800.u1",    0x0000000, 0x0400000, CRC(e1e6ec40) SHA1(390432431f144ef63424a426582b311765a61771) )

	ROM_REGION( 0x1000000, "gfx4", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0800.u3",    0x0000000, 0x0200000, CRC(52e7bef5) SHA1(a678251b7e46a1016d0afc1d8d5c9928008ad5b1) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
	ROM_LOAD( "m0800.u2",    0x400000, 0x400000,  CRC(e1a46541) SHA1(6fe9de5700d8638374734d80551dcedb62975140) )
ROM_END


/*

Oriental Legend Super
IGS, 1998

This is a cart for the IGS PGM system.

PCB Layout
----------
IGS PCB NO-0191-1
|-----------------------------------------------|
|6264                 8MHz|--------|            |
|6264                     |        |   T0500.U18|
|                         | IGS025 |            |
|                 |-----| |        |   T0501.U19|
|                 | IGS | |--------|            |
|                 | 028 |                       |
|        *1       |-----|           V101.U1     |
|              V101.U2   V101.U4  PAL      PAL  |
|  V101.U6          V101.U3   V101.U5           |
|-|                                           |-|
  |--------------------||---------------------|
Notes:
      IGS028       - Custom IGS IC (QFP100)
      IGS025       - Custom IGS IC (PLCC68, labelled "KOREA")
      T0500.U18    - 32MBit MaskROM (SOP44)
      T0501.U19    - 16MBit MaskROM (SOP44)
      V101.U1      - MX27C4096 4MBit EPROM (DIP40)
      V101.U2/3/4/5- MX27C4000 4MBit EPROM (DIP32)
      PALs         - x2, labelled "CW-2 U8", "CW-2 U7"
      6264         - 8K x8 SRAM
      *1           - Unpopulated position for SOP44 MASKROM labelled "P0500"


IGS PCB NO-0135
|-----------------------------------------------|
|A0504.U11        A0506.U13     B0502.U15       |
|         A0505.U12         U14        B0503.U16|
|                                               |
|A0500.U5                       B0500.U9        |
|         A0501.U6       A0503.U8      B0501.U10|
|                 A0502.U7                      |
|                                               |
|74LS138          M0500.U1               74LS139|
|                           U2                  |
|-|                                           |-|
  |--------------------||---------------------|

Notes:
      This PCB contains only SOP44 MaskROMS and 2 logic IC's
      U2 and U14 are not populated.
      All are 32MBit except M0500 which is 16MBit.

*/

ROM_START( olds )
	ROM_REGION( 0x600000, "main", 0 ) /* 68000 Code  */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) )// (BIOS)
	ROM_LOAD16_BYTE( "sp_v101.u2",      0x100001, 0x080000,   CRC(08eb9661) SHA1(105946e72e562adb1a9fd794ca0fd2c91967eb56) )
	ROM_LOAD16_BYTE( "sp_v101.u3",      0x100000, 0x080000,   CRC(0a358c1e) SHA1(95c7c3f069c5d05001e22535750f6b3cd7de105f) )
	ROM_LOAD16_BYTE( "sp_v101.u4",      0x200001, 0x080000,   CRC(766570e0) SHA1(e7c3f5664ec69b662b82c2e1375555db7305390c) )
	ROM_LOAD16_BYTE( "sp_v101.u5",      0x200000, 0x080000,   CRC(58662e12) SHA1(2b39bd847e9c4968a8e77a2f3cec77cf323ceee3) )
	ROM_LOAD16_WORD_SWAP( "sp_v101.u1",0x300000, 0x080000,    CRC(2b2f4f1e) SHA1(67b97cf8cc7f517d67cd45588addd2ad8e24612a) )

	/* CPU2 = Z80, romless, code uploaded by 68k */

	ROM_REGION( 0x010000, "user1", 0 ) /* ASIC25? Protection Data */
	ROM_LOAD( "sp_v101.u6", 0x000000, 0x010000,  CRC(097046bc) SHA1(6d75db85cf4c79b63e837897785c253014b2126d) )

	ROM_REGION( 0x200000, "user2", ROMREGION_ERASEFF ) /* its a dump of the shared protection rom/ram from pcb. */
	// clearly not for this revision
	//ROM_LOAD16_WORD_SWAP( "ram_dump", 0x000000, 0x200000, CRC(e7b26aea) SHA1(17d101f760d790619ce4858984787b494bdbbc8a) )


	ROM_REGION( 0xc00000, "gfx1",  ROMREGION_DISPOSE ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "t0500.rom",    0x400000, 0x400000, CRC(d881726c) SHA1(a82517e665996f7b7017c940f1fcf016fccb65c2) )
	ROM_LOAD( "t0501.rom",    0x800000, 0x200000, CRC(d2106864) SHA1(65d827135b87d82196433aea3279608ee263feca) )

	ROM_REGION( 0xc00000/5*8, "gfx2", ROMREGION_DISPOSE | ROMREGION_ERASEFF ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x1c00000, "gfx3", ROMREGION_DISPOSE ) /* Sprite Colour Data */
	ROM_LOAD( "a0500.rom",    0x0000000, 0x400000, CRC(80a59197) SHA1(7d0108e2f0d0347d43ace2d96c0388202c05fdfb) )
	ROM_LOAD( "a0501.rom",    0x0400000, 0x400000, CRC(98c931b1) SHA1(9b70d1a7beb4c3a0c1436c25fd1fb76e47539538) )
	ROM_LOAD( "a0502.rom",    0x0800000, 0x400000, CRC(c3fcdf1d) SHA1(107585fd103fcd0af0fb7db283be4f7c7058aef7) )
	ROM_LOAD( "a0503.rom",    0x0c00000, 0x400000, CRC(066dffec) SHA1(f023032a7202b7090fb609a39e0f19018e664bf3) )
	ROM_LOAD( "a0504.rom",    0x1000000, 0x400000, CRC(45337583) SHA1(c954d0e5bf7fa99c90b0d154e7119d2b0c461f1c) )
	ROM_LOAD( "a0505.rom",    0x1400000, 0x400000, CRC(5b8cf3a5) SHA1(856d1e47b5d9a66dcfbdc74a51ed646fd7d96a35) )
	ROM_LOAD( "a0506.rom",    0x1800000, 0x400000, CRC(087ac60c) SHA1(3d5bf7dd40c8a3c1224cf82e12410ca904c0c5db) )

	ROM_REGION( 0x1000000, "gfx4", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0500.rom",    0x0000000, 0x400000, CRC(cde07f74) SHA1(1fe90db7c57faa28f3a054b2c07909bef89e3efb) )
	ROM_LOAD( "b0501.rom",    0x0400000, 0x400000, CRC(1546c2e9) SHA1(a7b9c8b44203db54a59d49fe469bb52bba807ba2) )
	ROM_LOAD( "b0502.rom",    0x0800000, 0x400000, CRC(e97b31c3) SHA1(1a7ca4f6c8644e84a33ae41cd4637f21046b14c5) )
	ROM_LOAD( "b0503.u16",    0x0c00000, 0x400000, CRC(e41d98e4) SHA1(f80b27fcee81762993e09bf1b3cad6e85274760c) )


	ROM_REGION( 0x600000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
	ROM_LOAD( "m0500.rom",    0x400000, 0x200000, CRC(37928cdd) SHA1(e80498cabc2a6a54d4f3ebcb097d4b3fad96fe55) )
ROM_END

ROM_START( olds100 )
	ROM_REGION( 0x600000, "main", 0 ) /* 68000 Code  */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) )// (BIOS)
	ROM_LOAD16_BYTE( "v100-u2.040",      0x100001, 0x080000,  CRC(517c2a06) SHA1(bbf5b311fac9b0bb4d4129c0561e5e24f6963fa2) )
	ROM_LOAD16_BYTE( "v100-u3.040",      0x100000, 0x080000,  CRC(d0e2b741) SHA1(2e671dbb4320d1f0c059b35efd33cdea26f12131) )
	ROM_LOAD16_BYTE( "v100-u4.040",      0x200001, 0x080000,  CRC(32a6bdbd) SHA1(a93d7f4eae722a58eca9ec351ad5890cefda56f0) )
	ROM_LOAD16_BYTE( "v100-u5.040",      0x200000, 0x080000,  CRC(b4a1cafb) SHA1(b2fccd480ede93f58ad043387b18b898152f06ef) )
	/* u1 is missing from this set? - the parent has v101 rom for u1 so it probably doesn't go with v100 main program roms */
//  ROM_LOAD16_WORD_SWAP( "v100-u1.040",0x300000, 0x080000,    CRC(1) SHA1(1) )

	/* CPU2 = Z80, romless, code uploaded by 68k */

	ROM_REGION( 0x200000, "user2", ROMREGION_ERASEFF ) /* its a dump of the shared protection rom/ram from pcb. */
	// used to simulate encrypted DMA protection device for now ..
	//ROM_LOAD16_WORD_SWAP( "ram_dump", 0x000000, 0x200000, CRC(e7b26aea) SHA1(17d101f760d790619ce4858984787b494bdbbc8a) )


	ROM_REGION( 0x010000, "user1", 0 ) /* ASIC25? Protection Data */
	ROM_LOAD( "kd-u6.512", 0x000000, 0x010000,  CRC(e7613dda) SHA1(0d7c043b90e2f9a36a45066f22e3e305dc716676) )

	ROM_REGION( 0xc00000, "gfx1",  ROMREGION_DISPOSE ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "t0500.rom",    0x400000, 0x400000, CRC(d881726c) SHA1(a82517e665996f7b7017c940f1fcf016fccb65c2) )
	ROM_LOAD( "t0501.rom",    0x800000, 0x200000, CRC(d2106864) SHA1(65d827135b87d82196433aea3279608ee263feca) )

	ROM_REGION( 0xc00000/5*8, "gfx2", ROMREGION_DISPOSE | ROMREGION_ERASEFF ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x1c00000, "gfx3", ROMREGION_DISPOSE ) /* Sprite Colour Data */
	ROM_LOAD( "a0500.rom",    0x0000000, 0x400000, CRC(80a59197) SHA1(7d0108e2f0d0347d43ace2d96c0388202c05fdfb) )
	ROM_LOAD( "a0501.rom",    0x0400000, 0x400000, CRC(98c931b1) SHA1(9b70d1a7beb4c3a0c1436c25fd1fb76e47539538) )
	ROM_LOAD( "a0502.rom",    0x0800000, 0x400000, CRC(c3fcdf1d) SHA1(107585fd103fcd0af0fb7db283be4f7c7058aef7) )
	ROM_LOAD( "a0503.rom",    0x0c00000, 0x400000, CRC(066dffec) SHA1(f023032a7202b7090fb609a39e0f19018e664bf3) )
	ROM_LOAD( "a0504.rom",    0x1000000, 0x400000, CRC(45337583) SHA1(c954d0e5bf7fa99c90b0d154e7119d2b0c461f1c) )
	ROM_LOAD( "a0505.rom",    0x1400000, 0x400000, CRC(5b8cf3a5) SHA1(856d1e47b5d9a66dcfbdc74a51ed646fd7d96a35) )
	ROM_LOAD( "a0506.rom",    0x1800000, 0x400000, CRC(087ac60c) SHA1(3d5bf7dd40c8a3c1224cf82e12410ca904c0c5db) )

	ROM_REGION( 0x1000000, "gfx4", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0500.rom",    0x0000000, 0x400000, CRC(cde07f74) SHA1(1fe90db7c57faa28f3a054b2c07909bef89e3efb) )
	ROM_LOAD( "b0501.rom",    0x0400000, 0x400000, CRC(1546c2e9) SHA1(a7b9c8b44203db54a59d49fe469bb52bba807ba2) )
	ROM_LOAD( "b0502.rom",    0x0800000, 0x400000, CRC(e97b31c3) SHA1(1a7ca4f6c8644e84a33ae41cd4637f21046b14c5) )
	ROM_LOAD( "b0503.u16",    0x0c00000, 0x400000, CRC(e41d98e4) SHA1(f80b27fcee81762993e09bf1b3cad6e85274760c) )

	ROM_REGION( 0x600000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
	ROM_LOAD( "m0500.rom",    0x400000, 0x200000, CRC(37928cdd) SHA1(e80498cabc2a6a54d4f3ebcb097d4b3fad96fe55) )
ROM_END

/* this is the set which the protection ram dump seems to be for.. */
ROM_START( olds100a )
	ROM_REGION( 0x600000, "main", 0 ) /* 68000 Code  */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) )// (BIOS)
	/* this rom had a lame hack applied to it by the dumper, this was removed, hopefully it is correct now */
	ROM_LOAD16_WORD_SWAP( "p0500.v10",    0x100000, 0x400000, CRC(8981fc87) SHA1(678d6705d06b99bca5951ff77708adadc4c4396b) )

	/* CPU2 = Z80, romless, code uploaded by 68k */

	ROM_REGION( 0x010000, "user1", ROMREGION_ERASEFF ) /* ASIC25? Protection Data */
	/* missing from this set .. */
	ROM_LOAD( "protection_data.u6", 0x000000, 0x010000, NO_DUMP )

	ROM_REGION( 0x200000, "user2", ROMREGION_ERASEFF ) /* its a dump of the shared protection rom/ram from pcb. */
	// used to simulate encrypted DMA protection device for now ..
	ROM_LOAD16_WORD_SWAP( "ram_dump", 0x000000, 0x200000, CRC(e7b26aea) SHA1(17d101f760d790619ce4858984787b494bdbbc8a) )


	ROM_REGION( 0xc00000, "gfx1",  ROMREGION_DISPOSE ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "t0500.rom",    0x400000, 0x400000, CRC(d881726c) SHA1(a82517e665996f7b7017c940f1fcf016fccb65c2) )
	ROM_LOAD( "t0501.rom",    0x800000, 0x200000, CRC(d2106864) SHA1(65d827135b87d82196433aea3279608ee263feca) )

	ROM_REGION( 0xc00000/5*8, "gfx2", ROMREGION_DISPOSE | ROMREGION_ERASEFF ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x1c00000, "gfx3", ROMREGION_DISPOSE ) /* Sprite Colour Data */
	ROM_LOAD( "a0500.rom",    0x0000000, 0x400000, CRC(80a59197) SHA1(7d0108e2f0d0347d43ace2d96c0388202c05fdfb) )
	ROM_LOAD( "a0501.rom",    0x0400000, 0x400000, CRC(98c931b1) SHA1(9b70d1a7beb4c3a0c1436c25fd1fb76e47539538) )
	ROM_LOAD( "a0502.rom",    0x0800000, 0x400000, CRC(c3fcdf1d) SHA1(107585fd103fcd0af0fb7db283be4f7c7058aef7) )
	ROM_LOAD( "a0503.rom",    0x0c00000, 0x400000, CRC(066dffec) SHA1(f023032a7202b7090fb609a39e0f19018e664bf3) )
	ROM_LOAD( "a0504.rom",    0x1000000, 0x400000, CRC(45337583) SHA1(c954d0e5bf7fa99c90b0d154e7119d2b0c461f1c) )
	ROM_LOAD( "a0505.rom",    0x1400000, 0x400000, CRC(5b8cf3a5) SHA1(856d1e47b5d9a66dcfbdc74a51ed646fd7d96a35) )
	ROM_LOAD( "a0506.rom",    0x1800000, 0x400000, CRC(087ac60c) SHA1(3d5bf7dd40c8a3c1224cf82e12410ca904c0c5db) )

	ROM_REGION( 0x1000000, "gfx4", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0500.rom",    0x0000000, 0x400000, CRC(cde07f74) SHA1(1fe90db7c57faa28f3a054b2c07909bef89e3efb) )
	ROM_LOAD( "b0501.rom",    0x0400000, 0x400000, CRC(1546c2e9) SHA1(a7b9c8b44203db54a59d49fe469bb52bba807ba2) )
	ROM_LOAD( "b0502.rom",    0x0800000, 0x400000, CRC(e97b31c3) SHA1(1a7ca4f6c8644e84a33ae41cd4637f21046b14c5) )
	ROM_LOAD( "b0503.u16",    0x0c00000, 0x400000, CRC(e41d98e4) SHA1(f80b27fcee81762993e09bf1b3cad6e85274760c) )

	ROM_REGION( 0x600000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
	ROM_LOAD( "m0500.rom",    0x400000, 0x200000, CRC(37928cdd) SHA1(e80498cabc2a6a54d4f3ebcb097d4b3fad96fe55) )
ROM_END


ROM_START( kov2 )
	ROM_REGION( 0x600000, "main", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) )  // (BIOS)
	ROM_LOAD16_WORD_SWAP( "igs_u18.rom",    0x100000, 0x400000, CRC(86205879) SHA1(f73d5b70b41d39be1cac75e474b025de2cce0b01) )

	/* CPU2 = Z80, romless, code uploaded by 68k */

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kov2_v100_hongkong.asic", 0x000000, 0x04000,  CRC(e0d7679f) SHA1(e1c2d127eba4ddbeb8ad173c55b90ac1467e1ca8) )

	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "igs_u19.rom", 0x000000, 0x200000,   CRC(edd59922) SHA1(09b14f20f685944a93292c83e5830849aade42c9) )

	ROM_REGION( 0xc00000, "gfx1", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "t1200.rom",    0x400000, 0x800000, CRC(d7e26609) SHA1(bdad810f82fcf1d50a8791bdc495374ec5a309c6) )

	ROM_REGION( 0xc00000/5*8, "gfx2", ROMREGION_DISPOSE | ROMREGION_ERASEFF ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x2800000, "gfx3", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a1200.rom",    0x0000000, 0x0800000, CRC(ceeb81d8) SHA1(5476729443fc1bc9593ae10fbf7cbc5d7290b017) )
	ROM_LOAD( "a1201.rom",    0x0800000, 0x0800000, CRC(82f0a878) SHA1(ddd13e404252a71de1b2b3b974b910f899f51c38) )
	ROM_LOAD( "a1202.rom",    0x1000000, 0x0800000, CRC(4bb92fae) SHA1(f0b6d72ed425de1c69dc8f8d5795ea760a4a59b0) )
	ROM_LOAD( "a1203.rom",    0x1800000, 0x0800000, CRC(e73cb627) SHA1(4c6e48b845a5d1e8f9899010fbf273d54c2b8899) )
	ROM_LOAD( "a1204.rom",    0x2000000, 0x0800000, CRC(27527099) SHA1(e23cf366bdeaca1e009a5cec6b13164310a34384) )

	ROM_REGION( 0x1000000, "gfx4", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b1200.rom",   0x0000000, 0x0800000,  CRC(bed7d994) SHA1(019dfba8154256d64cd249eb0fa4c451edce34b8) )
	ROM_LOAD( "b1201.rom",   0x0800000, 0x0800000,  CRC(f251eb57) SHA1(56a5fc14ab7822f83379cecb26638e5bb266349a) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
	ROM_LOAD( "m1200.rom",    0x800000, 0x800000, CRC(b0d88720) SHA1(44ab137e3f8e15b7cb5697ffbd9b1143d8210c4f) )
ROM_END


ROM_START( kov2106 )
	ROM_REGION( 0x600000, "main", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) )  // (BIOS)
	ROM_LOAD16_WORD_SWAP( "u18.106",    0x100000, 0x400000, CRC(40051ad9) SHA1(ba2ddf267fe688d5dfed575aeeccbab10135b37b) )

	/* CPU2 = Z80, romless, code uploaded by 68k */

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kov2_v100_hongkong.asic", 0x000000, 0x04000,  CRC(e0d7679f) SHA1(e1c2d127eba4ddbeb8ad173c55b90ac1467e1ca8) )

	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "u19.102", 0x000000, 0x200000,   CRC(462e2980) SHA1(3da7c3d2c65b59f50c78be1c25922b71d40f6080) )

	ROM_REGION( 0xc00000, "gfx1", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "t1200.rom",    0x400000, 0x800000, CRC(d7e26609) SHA1(bdad810f82fcf1d50a8791bdc495374ec5a309c6) )

	ROM_REGION( 0xc00000/5*8, "gfx2", ROMREGION_DISPOSE | ROMREGION_ERASEFF ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x2800000, "gfx3", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a1200.rom",    0x0000000, 0x0800000, CRC(ceeb81d8) SHA1(5476729443fc1bc9593ae10fbf7cbc5d7290b017) )
	ROM_LOAD( "a1201.rom",    0x0800000, 0x0800000, CRC(82f0a878) SHA1(ddd13e404252a71de1b2b3b974b910f899f51c38) )
	ROM_LOAD( "a1202.rom",    0x1000000, 0x0800000, CRC(4bb92fae) SHA1(f0b6d72ed425de1c69dc8f8d5795ea760a4a59b0) )
	ROM_LOAD( "a1203.rom",    0x1800000, 0x0800000, CRC(e73cb627) SHA1(4c6e48b845a5d1e8f9899010fbf273d54c2b8899) )
	ROM_LOAD( "a1204.rom",    0x2000000, 0x0800000, CRC(27527099) SHA1(e23cf366bdeaca1e009a5cec6b13164310a34384) )

	ROM_REGION( 0x1000000, "gfx4", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b1200.rom",   0x0000000, 0x0800000,  CRC(bed7d994) SHA1(019dfba8154256d64cd249eb0fa4c451edce34b8) )
	ROM_LOAD( "b1201.rom",   0x0800000, 0x0800000,  CRC(f251eb57) SHA1(56a5fc14ab7822f83379cecb26638e5bb266349a) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
	ROM_LOAD( "m1200.rom",    0x800000, 0x800000, CRC(b0d88720) SHA1(44ab137e3f8e15b7cb5697ffbd9b1143d8210c4f) )
ROM_END


ROM_START( kov2p )
	ROM_REGION( 0x600000, "main", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) )  // (BIOS)
	ROM_LOAD16_WORD_SWAP( "v204-32m.rom",    0x100000, 0x400000, CRC(583e0650) SHA1(2e5656dd9c6cba9f84af9baa3f5f70cdccf9db47) )

	/* CPU2 = Z80, romless, code uploaded by 68k */

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	/* not correct for this set, needs dumping from internal rom */
	ROM_LOAD( "kov2p.asic", 0x000000, 0x04000, BAD_DUMP CRC(e0d7679f) SHA1(e1c2d127eba4ddbeb8ad173c55b90ac1467e1ca8) ) // NOT for this version, works with a patch

	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "v200-16.rom", 0x000000, 0x200000,  CRC(16a0c11f) SHA1(ce449cef76ebd5657d49b57951e2eb0f132e203e) )

	ROM_REGION( 0xc00000, "gfx1", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "t1200.rom",    0x400000, 0x800000, CRC(d7e26609) SHA1(bdad810f82fcf1d50a8791bdc495374ec5a309c6) )

	ROM_REGION( 0xc00000/5*8, "gfx2", ROMREGION_DISPOSE | ROMREGION_ERASEFF ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x2800000, "gfx3", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a1200.rom",    0x0000000, 0x0800000, CRC(ceeb81d8) SHA1(5476729443fc1bc9593ae10fbf7cbc5d7290b017) )
	ROM_LOAD( "a1201.rom_p",  0x0800000, 0x0800000, CRC(21063ca7) SHA1(cf561b44902425a920d5cbea5bf65dd9530b2289) ) // either this or a1201.rom in kov2 is probably bad
	ROM_LOAD( "a1202.rom",    0x1000000, 0x0800000, CRC(4bb92fae) SHA1(f0b6d72ed425de1c69dc8f8d5795ea760a4a59b0) )
	ROM_LOAD( "a1203.rom",    0x1800000, 0x0800000, CRC(e73cb627) SHA1(4c6e48b845a5d1e8f9899010fbf273d54c2b8899) )
	ROM_LOAD( "a1204.rom_p",  0x2000000, 0x0200000, CRC(14b4b5bb) SHA1(d7db5740eec971f2782fb2885ee3af8f2a796550) ) // either this or a1204.rom in kov2 is probably bad

	ROM_REGION( 0x1000000, "gfx4", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b1200.rom",   0x0000000, 0x0800000,  CRC(bed7d994) SHA1(019dfba8154256d64cd249eb0fa4c451edce34b8) )
	ROM_LOAD( "b1201.rom",   0x0800000, 0x0800000,  CRC(f251eb57) SHA1(56a5fc14ab7822f83379cecb26638e5bb266349a) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
	ROM_LOAD( "m1200.rom",    0x800000, 0x800000, CRC(b0d88720) SHA1(44ab137e3f8e15b7cb5697ffbd9b1143d8210c4f) )
ROM_END

/*

Do Donpachi II
Cave, 2001

This is a PGM cart containing not a lot....
5x SOP44 mask ROMs (4x 64M, 1x 32M)
2x EPROMs (1x 4M, 1x 16M)
2x PALs (labelled FN U14 and FN U15)
1x custom IGS027A (QFP120)
3x RAMs WINBOND W24257AJ-8N
Some logic IC's, resistors, caps etc.

*/

ROM_START( ddp2 )
	ROM_REGION( 0x600000, "main", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) )  // (BIOS)
	ROM_LOAD16_WORD_SWAP( "v100.u8",    0x100000, 0x200000, CRC(0c8aa8ea) SHA1(57e33224622607a1df8daabf26ba063cf8a6d3fc) )

	/* CPU2 = Z80, romless, code uploaded by 68k */

	ROM_REGION( 0x4000, "cpu2", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "ddp2_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION32_LE( 0x20000, "user1", 0 ) /* Protection Data (encrypted external ARM data, internal missing) */
	ROM_LOAD( "v100.u23", 0x000000, 0x20000, CRC(06c3dd29) SHA1(20c9479f158467fc2037dcf162b6c6be18c91d46) )

	ROM_REGION( 0xc00000, "gfx1", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "t1300.u21",    0x400000, 0x800000, CRC(e748f0cb) SHA1(5843bee3a17c33648ce904af2b98c6a90aff7393) )

	ROM_REGION( 0xc00000/5*8, "gfx2", ROMREGION_DISPOSE | ROMREGION_ERASEFF ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x1000000, "gfx3", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a1300.u1",    0x0000000, 0x0800000, CRC(fc87a405) SHA1(115c21ecc56997652e527c92654076870bc9fa51) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a1301.u2",    0x0800000, 0x0800000, CRC(0c8520da) SHA1(390317857ae5baa94a4cc042874b00a811f06a63) ) // FIXED BITS (xxxxxxxx1xxxxxxx)

	ROM_REGION( 0x0800000, "gfx4", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b1300.u7",    0x0000000, 0x0800000,  CRC(ef646604) SHA1(d737ff513792962f18df88c2caa9dd71de449079) )

	ROM_REGION( 0x800000, "ics", ROMREGION_ERASE00 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
	ROM_LOAD( "m1300.u5",    0x400000, 0x400000, CRC(82d4015d) SHA1(d4cdc1aec1c97cf23ff7a20ccaad822962e66ffa) )
ROM_END

/*

Puzzli 2
IGS, 2001

Cart for IGS PGM system. The layout of the PCB is virtually identical to Puzzle Star.

PCB Layout
----------

IGS PCB NO- 0259
|-----------------------------------------------|
|                        U6 U7                  |
|         |-------|                             |
|         |IGS027A|                             |
|         |       |                     T0900.U9|
|         |       |                             |
|         |-------|                             |
|          2SP_V200.U3                          |
|          2SP_V200.U4    U5   PAL              |
|                                               |
|-|                                           |-|
  |--------------------||---------------------|
Notes:
      IGS027A     - Custom IGS IC, ARM7/9? based CPU (QFP120, stamped 'IGS027A')
      T0900.U9    - 16MBit MaskROM (SOP44)
      2SP_V200.U3 - MX27C4000 512K x8 EPROM (DIP32, labelled '2SP V200 U3')
      2SP_V200.U4 - MX27C4000 512K x8 EPROM (DIP32, labelled '2SP V200 U4')
      PAL         - AMD PALCE22V10 PAL (DIP24, labelled 'EL U8')
      U5          - Unpopulated position for 16MBit MaskROM (DIP42)
      U6, U7      - Unpopulated position for 74LS245 logic chip (x2)


IGS PCB NO- 0258
|-----------------------------------------------|
|                                               |
|                                               |
|                                               |
|                                               |
|   *    M0900.U2   A0900.U3   B0900.U4         |
|                                               |
|                                               |
|                                               |
|                                               |
|-|                                           |-|
  |--------------------||---------------------|
Notes:
      *  - Unpopulated position for Oki MSM27C3202CZ 32MBit MaskROM (TSOP48 Type II)
      U2 - 32MBit MaskROM (DIP42, Byte mode)
      U3 - 32MBit MaskROM (SOP44)
      U4 - 16MBit MaskROM (SOP44)

*/
ROM_START( puzzli2 )
	ROM_REGION( 0x600000, "main", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) )  // (BIOS)
	ROM_LOAD16_BYTE( "2sp_v200.u3",     0x100001, 0x080000, CRC(2a5ba8a6) SHA1(4c87b849fd6f39152e3e2ef699b78ce24b3fb6d0) )
	ROM_LOAD16_BYTE( "2sp_v200.u4",     0x100000, 0x080000, CRC(fa5c86c1) SHA1(11c219722b891b775c0f7f9bc8276cdd8f74d657) )

	/* CPU2 = Z80, romless, code uploaded by 68k */

	ROM_REGION( 0x4000, "cpu2", ROMREGION_ERASEFF ) /* ARM protection ASIC - internal rom */
	/* this has no external rom so the internal rom probably can't be dumped */
//  ROM_LOAD( "puzzli2_igs027a.bin", 0x000000, 0x04000, NO_DUMP )


	ROM_REGION( 0x600000, "gfx1", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "t0900.u9",    0x400000, 0x200000, CRC(70615611) SHA1(a46d4aa71396947b427f9ba4ba0e636876c09d6b) )

	ROM_REGION( 0x600000/5*8, "gfx2", ROMREGION_DISPOSE | ROMREGION_ERASEFF ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x400000, "gfx3", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0900.u3",    0x0000000, 0x0400000, CRC(14911251) SHA1(e0d10ef50c408dbcf0907f81d4f0e49aeb651a6c) ) // FIXED BITS (xxxxxxxx1xxxxxxx)

	ROM_REGION( 0x0200000, "gfx4", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0900.u4",    0x0000000, 0x0200000,  CRC(6f0638b6) SHA1(14b315fe9e80b3314bb63487e6ea9ce04c9703bd) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
	ROM_LOAD( "m0900.u2",    0x400000, 0x400000, CRC(9ea7af2e) SHA1(d2593d391a93c5cf5a554750c32886dea6599b3d) )
ROM_END


/*

Martial Masters
IGS, 2001

Cart for IGS PGM system. This game is a straight rip-off of any of the
late side-by-side fighting games made by SNK or Capcom such as King Of Fighters
or Super Street Fighter II etc


PCB Layout
----------

IGS PCB-0293-01
|-----------------------------------------------|
| 62256              62256         IGS027A      |
| 62256                                         |
|                      *                        |
|                                               |
| PAL                                           |
|                                               |
| PAL             V102_16M.U10  T1000.U3        |
|                                               |
|                 V104_32M.U9              22MHz|
|-|                                           |-|
  |--------------------||---------------------|
Notes:
      62256        - 32K x8 SRAM (SOJ28)
      IGS027A      - Custom IGS IC, ARM7 based CPU with internal 64K ROM (QFP120)
      T1000.U3     - 23C6410 64MBit MaskROM (SOP44)
      V102_16M.U10 - MX29F1610MC 16MBit SOP44 FlashROM mounted onto a tiny DIP42 to SOP44 adapter board
                     (manufactured by IGS) which is plugged into a standard DIP42 socket. This chip was
                     read directly on the adapter as a 27C160 EPROM. The socket is wired to accept 32MBit
                     DIP42 EPROMs.
      V104_32M.U9  - M27C3202CZ 32MBit TSOP48 Type II OTP MaskROM mounted onto a tiny DIP42 to TSOP48 Type II
                     adapter board (manufactured by IGS) which is plugged into a standard DIP42 socket. This
                     chip was read directly on the adapter as a 27C322 EPROM. The socket is wired to accept
                     32MBit DIP42 EPROMs.
      *            - Unpopulated position for 62256 SRAM


IGS PCB-0292-00
|-----------------------------------------------|
| A1000.U3         A1002.U6           A1004.U10 |
|          A1001.U4         A1003.U8            |
|                                               |
|                                               |
|                                               |
|                                               |
|                  M1001.U7           B1001.U11 |
|          M1000.U5         B1000.U9            |
|                                               |
|-|                                           |-|
  |--------------------||---------------------|



*/

ROM_START( martmast )
	ROM_REGION( 0x600000, "main", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) )  // (BIOS)
	ROM_LOAD16_WORD_SWAP( "v104_32m.u9",    0x100000, 0x400000, CRC(cfd9dff4) SHA1(328eaf6ac49a73265ee4e0f992b1b1312f49877b) )

	/* CPU2 = Z80, romless, code uploaded by 68k */

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "martial_masters_v102_usa.asic", 0x000000, 0x04000, BAD_DUMP CRC(a6c0828c) SHA1(0a5bda56dca264c3c7ff7698b8f699563f203c4d) ) // not verified, could be bad

	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "v102_16m.u10", 0x000000, 0x200000,  CRC(18b745e6) SHA1(7bcb58dd3a2d6072f492cf0dd7181cb061c1f49d) )

	ROM_REGION( 0xc00000, "gfx1", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "t1000.u3",    0x400000, 0x800000, CRC(bbf879b5) SHA1(bd9a6aea34ad4001e89e62ff4b7a2292eb833c00) )

	ROM_REGION( 0xc00000/5*8, "gfx2", ROMREGION_DISPOSE | ROMREGION_ERASEFF ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x2800000, "gfx3", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a1000.u3",    0x0000000, 0x0800000, CRC(43577ac8) SHA1(6eea8b455985d5bac74dcc9943cdc3c0902de6cc) )
	ROM_LOAD( "a1001.u4",    0x0800000, 0x0800000, CRC(fe7a476f) SHA1(a8c7f1f0dd3e53141aed6d927eb88a3ceebb81e4) )
	ROM_LOAD( "a1002.u6",    0x1000000, 0x0800000, CRC(62e33d38) SHA1(96163d583e25073594f8413ce263e56b66bd69a1) )
	ROM_LOAD( "a1003.u8",    0x1800000, 0x0800000, CRC(b2c4945a) SHA1(7b18287a2db56db3651cfd4deb607af53522fefd) )
	ROM_LOAD( "a1004.u10",   0x2000000, 0x0400000, CRC(9fd3f5fd) SHA1(057531f91062be51589c6cf8f4170089b9be6380) )

	ROM_REGION( 0x1000000, "gfx4", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b1000.u9",    0x0000000, 0x0800000,  CRC(c5961f6f) SHA1(a68060b10edbd084cbde79d2ed1c9084777beb10) )
	ROM_LOAD( "b1001.u11",   0x0800000, 0x0800000,  CRC(0b7e1c06) SHA1(545e15e0087f8621d593fecd8b4013f7ca311686) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
	ROM_LOAD( "m1000.u5",    0x400000, 0x800000, CRC(ed407ae8) SHA1(a6e9c09b39c13e8fb7fbc89fa9f823cbeb66e901) )
	ROM_LOAD( "m1001.u7",    0xc00000, 0x400000, CRC(662d2d48) SHA1(2fcc3099d9c04456cae3b13035fb28eaf709e7d8) )
ROM_END

/*

Demon Front
IGS, 2002

Cart for IGS PGM system. This game is a straight rip-off of Metal Slug.

PCB Layout
----------

IGS PCB-0387-02-FV
|-----------------------------------------------|
| BS616LV1010                      IGS027A      |
| BS616LV1010                                   |
|                                               |
|                              *     BS616LV1010|
|            PAL  PAL                           |
|                                               |
| V102_16M.U5        V101_32M.U26               |
|                                        PAL    |
|                             T04501.U29   22MHz|
|-|                                           |-|
  |--------------------||---------------------|
Notes:
      BS616LV1010  - 64K x16 SRAM (TSOP44)
      IGS027A      - Custom IGS IC, ARM7 based CPU (QFP120)
      T04501.U29   - 23C6410 64MBit MaskROM (SOP44)
      V102_16M.U5  - 27C160 16MBit EPROM (DIP42)
      V101_32M.U26 - 27C322 32MBit EPROM (DIP42)
      *            - Unpopulated position for 29F1610 16MBit SOP44 FlashROM, linked to IGS027A


IGS PCB-0390-00-FV-A
|-----------------------------------------------|
| A04501.U3  A04502.U4  A04503.U6   U8*     U10*|
|                                               |
|                                               |
|                                               |
|                                               |
|     W04501.U5   U7*    B04501.U9   B04502.U11 |
|                                               |
|                                               |
|                                               |
|-|                                           |-|
  |--------------------||---------------------|
Notes:
      *  - Unpopulated SOP44 ROM position.

*/

ROM_START( dmnfrnt )
	ROM_REGION( 0x600000, "main", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) )  // (BIOS)
	ROM_LOAD16_WORD_SWAP( "v102_16m.u5",    0x100000, 0x200000, CRC(3d4d481a) SHA1(95953b8f31343389405cc722b4177ff5adf67b62) )

	/* CPU2 = Z80, romless, code uploaded by 68k */

	ROM_REGION( 0x4000, "cpu2", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "dmnfrnt_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0x400000, "user1", 0 ) /* Protection Data (encrypted external ARM data, internal missing) */
	ROM_LOAD( "v101_32m.u26", 0x000000, 0x400000,  CRC(93965281) SHA1(89da198aaa7ca759cb96b5f18859a477e55fd590) )

	ROM_REGION( 0xc00000, "gfx1", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "t04501.u29",    0x400000, 0x800000, CRC(900eaaac) SHA1(4033cb7b28fcadb92d5af3ea7fdd1c22747618fd) )

	ROM_REGION( 0xc00000/5*8, "gfx2", ROMREGION_DISPOSE | ROMREGION_ERASEFF ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x1800000, "gfx3", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a04501.u3",    0x0000000, 0x0800000, CRC(9741bea6) SHA1(e3e904249be228628c8c2bd3495cda23586dc048) )
	ROM_LOAD( "a04502.u4",    0x0800000, 0x0800000, CRC(e104f405) SHA1(124b3deed3e838f8bae6c7d78bdd788859597585) )
	ROM_LOAD( "a04503.u6",    0x1000000, 0x0800000, CRC(bfd5cfe3) SHA1(fbe4c0a2987c2036df707b86597d78124ee2e665) )

	ROM_REGION( 0x1000000, "gfx4", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b04501.u9",    0x0000000, 0x0800000,  CRC(29320b7d) SHA1(59c78805e666f912df201c34616744f46057937b) )
	ROM_LOAD( "b04502.u11",   0x0800000, 0x0200000,  CRC(578c00e9) SHA1(14235cc8b0f8c7dd659512f017a2d4aacd91d89d) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
	ROM_LOAD( "w04501.u5",    0x400000, 0x800000, CRC(3ab58137) SHA1(b221f7e551ff0bfa3fd97b6ebedbac69442a66e9) )
ROM_END

ROM_START( dmnfrnta )
	ROM_REGION( 0x600000, "main", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) )  // (BIOS)
	ROM_LOAD16_WORD_SWAP( "v105_16m.u5",    0x100000, 0x200000, CRC(bda083bd) SHA1(58d6438737a2c43aa8bbcb7f34fb51375b781b1c) )

	/* CPU2 = Z80, romless, code uploaded by 68k */

	ROM_REGION( 0x4000, "cpu2", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "dmnfrnt_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0x400000, "user1", 0 ) /* Protection Data (encrypted external ARM data, internal missing) */
	ROM_LOAD( "v105_32m.u26", 0x000000, 0x400000,  CRC(d200ee63) SHA1(3128c27c5f5a4361d31e7b4bb006de631b3a228c) )

	ROM_REGION( 0xc00000, "gfx1", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "t04501.u29",    0x400000, 0x800000, CRC(900eaaac) SHA1(4033cb7b28fcadb92d5af3ea7fdd1c22747618fd) )

	ROM_REGION( 0xc00000/5*8, "gfx2", ROMREGION_DISPOSE | ROMREGION_ERASEFF ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x1800000, "gfx3", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a04501.u3",    0x0000000, 0x0800000, CRC(9741bea6) SHA1(e3e904249be228628c8c2bd3495cda23586dc048) )
	ROM_LOAD( "a04502.u4",    0x0800000, 0x0800000, CRC(e104f405) SHA1(124b3deed3e838f8bae6c7d78bdd788859597585) )
	ROM_LOAD( "a04503.u6",    0x1000000, 0x0800000, CRC(bfd5cfe3) SHA1(fbe4c0a2987c2036df707b86597d78124ee2e665) )

	ROM_REGION( 0xc00000, "gfx4", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b04501.u9",    0x0000000, 0x0800000,  CRC(29320b7d) SHA1(59c78805e666f912df201c34616744f46057937b) )
	ROM_LOAD( "b04502.u11",   0x0800000, 0x0200000,  CRC(578c00e9) SHA1(14235cc8b0f8c7dd659512f017a2d4aacd91d89d) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
	ROM_LOAD( "w04501.u5",    0x400000, 0x800000, CRC(3ab58137) SHA1(b221f7e551ff0bfa3fd97b6ebedbac69442a66e9) )
ROM_END

ROM_START( theglad )
	ROM_REGION( 0x600000, "main", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "pgm_p01s.rom", 0x000000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) )  // (BIOS)
	ROM_LOAD16_WORD_SWAP( "u6.rom",    0x100000, 0x080000, CRC(14c85212) SHA1(8d2489708e176a2c460498a13173be01f645b79e) )

	/* CPU2 = Z80, romless, code uploaded by 68k */

	ROM_REGION( 0x200000, "user1", 0 ) /* Protection Data (encrypted external ARM data, internal missing) */
	ROM_LOAD( "u2.rom", 0x000000, 0x200000,  CRC(c7bcf2ae) SHA1(10bc012c83987f594d5375a51bc4be2e17568a81) )

	ROM_REGION( 0xc00000, "gfx1", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // (BIOS)
	ROM_LOAD( "t04601.u33",    0x400000, 0x800000, BAD_DUMP CRC(2da3be8e) SHA1(704be0efc09bc931c71efd0b3e9f1bc4bfcdd3c1) )

	ROM_REGION( 0xc00000/5*8, "gfx2", ROMREGION_DISPOSE | ROMREGION_ERASEFF ) /* Region for 32x32 BG Tiles */
	/* 32x32 Tile Data is put here for easier Decoding */

	ROM_REGION( 0x1800000, "gfx3", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a04601.u2",    0x0000000, 0x0800000,  CRC(d9b2e004) SHA1(8e1882b800fe9f12d7d49303e7417ba5b6f8ef85) )
	ROM_LOAD( "a04602.u4",    0x0800000, 0x0800000,  CRC(14f22308) SHA1(7fad54704e8c97eab723f53dfb50fb3e7bb606d2) )
	ROM_LOAD( "a04603.u6",    0x1000000, 0x0800000,  CRC(8f621e17) SHA1(b0f87f378e0115d0c95017ca0f1b0d508827a7c6) )

	ROM_REGION( 0x1000000, "gfx4", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b04601.u11",    0x0000000, 0x0800000, CRC(ee72bccf) SHA1(73c25fe659f6c903447066e4ef83d2f580449d76) )
	ROM_LOAD( "b04602.u12",    0x0800000, 0x0400000, CRC(7dba9c38) SHA1(a03d509274e8f6a500a7ebe2da5aab8bed4e7f2f) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // (BIOS)
	ROM_LOAD( "w04601.u1",    0x400000, 0x800000, CRC(5f15ddb3) SHA1(c38dcef8e06802a84e42a7fc9fa505475fc3ac65) )
ROM_END


/*** GAME ********************************************************************/

GAME( 1997, pgm,      0,          pgm, pgm,      pgm,        ROT0,   "IGS", "PGM (Polygame Master) System BIOS", GAME_IS_BIOS_ROOT )

GAME( 1997, orlegend, pgm,        pgm, pgm,      orlegend,   ROT0,   "IGS", "Oriental Legend / Xi Yo Gi Shi Re Zuang (ver. 126)", GAME_IMPERFECT_SOUND  )
GAME( 1997, orlegnde, orlegend,   pgm, pgm,      orlegend,   ROT0,   "IGS", "Oriental Legend / Xi Yo Gi Shi Re Zuang (ver. 112)", GAME_IMPERFECT_SOUND  )
GAME( 1997, orlegndc, orlegend,   pgm, pgm,      orlegend,   ROT0,   "IGS", "Oriental Legend / Xi Yo Gi Shi Re Zuang (ver. 112, Chinese Board)", GAME_IMPERFECT_SOUND  )
GAME( 1997, orld111c, orlegend,   pgm, pgm,      orlegend,   ROT0,   "IGS", "Oriental Legend / Xi Yo Gi Shi Re Zuang (ver. 111, Chinese Board)", GAME_IMPERFECT_SOUND  )
GAME( 1997, orld105k, orlegend,   pgm, orld105k, orlegend,   ROT0,   "IGS", "Oriental Legend / Xi Yo Gi Shi Re Zuang (ver. 105, Korean Board)", GAME_IMPERFECT_SOUND  )
GAME( 1997, drgw2c,   drgw2,      drgw2, pgm,      drgw2c,     ROT0,   "IGS", "Zhong Guo Long II (ver. 100C, China)", GAME_IMPERFECT_SOUND )
GAME( 1999, kov,      pgm,        pgm, sango,    kov,        ROT0,   "IGS", "Knights of Valour / Sangoku Senki (ver. 117)", GAME_IMPERFECT_SOUND ) /* ver # provided by protection? */
GAME( 1999, kov115,   kov,        pgm, sango,    kov,        ROT0,   "IGS", "Knights of Valour / Sangoku Senki (ver. 115)", GAME_IMPERFECT_SOUND ) /* ver # provided by protection? */
GAME( 1999, kovj,     kov,        pgm, sango,    kov,        ROT0,   "IGS", "Knights of Valour / Sangoku Senki (ver. 100, Japanese Board)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND ) /* ver # provided by protection? */
GAME( 1999, photoy2k, pgm,        pgm, photoy2k,    djlzz,      ROT0,   "IGS", "Photo Y2K", GAME_IMPERFECT_SOUND ) /* region provided by protection device */
GAME( 1999, raf102j,  photoy2k,   pgm, photoy2k,    djlzz,      ROT0,   "IGS", "Real and Fake / Photo Y2K (ver. 102, Japan Board)", GAME_IMPERFECT_SOUND ) /* region provided by protection device */

/* Playable but maybe imperfect protection emulation */
GAME( 1997, drgw2,    pgm,        drgw2, pgm,      drgw2,      ROT0,   "IGS", "Dragon World II (ver. 110X, Export)", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING )
GAME( 1997, drgw2j,   drgw2,      drgw2, pgm,      drgw2j,     ROT0,   "IGS", "Chuugokuryuu II (ver. 100J, Japan)", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING )
GAME( 1999, kovplus,  kov,        pgm, sango,    kov,        ROT0,   "IGS", "Knights of Valour Plus / Sangoku Senki Plus (ver. 119)", GAME_IMPERFECT_SOUND )
GAME( 1999, kovplusa, kov,        pgm, sango,    kov,        ROT0,   "IGS", "Knights of Valour Plus / Sangoku Senki Plus (alt ver. 119)", GAME_IMPERFECT_SOUND )
GAME( 1998, killbldt, killbld, killbld,killbld,    killbld,    ROT0,   "IGS", "The Killing Blade (Chinese Board)", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION ) // it's playable, but there are some things unclear about the protection
GAME( 1999, puzlstar, pgm,        pgm, sango,    pstar,      ROT0,   "IGS", "Puzzle Star", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION )  // not playable past first few rounds


/* not working */
GAME( 1998, drgw3,    pgm,        pgm, sango,    dw3,        ROT0,   "IGS", "Dragon World 3", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING )
GAME( 1998, drgw3k,   drgw3,      pgm, sango,    dw3,        ROT0,   "IGS", "Dragon World 3 (Korean Board)", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING )
GAME( 1999, kovsh,    kov,      kovsh, sango,    kovsh,      ROT0,   "IGS", "Knights of Valour Superheroes / Sangoku Senki Superheroes (ver. 322)", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING )

GAME( 1998, killbld,  pgm,     killbld,killbld,  killbld,    ROT0,   "IGS", "The Killing Blade", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING )
GAME( 1999, olds,     pgm,        olds, olds,    olds,   ROT0,   "IGS", "Oriental Legend Super / Special (Korea 101)", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING )
GAME( 1999, olds100,  olds,       olds, olds,    olds,   ROT0,   "IGS", "Oriental Legend Super / Special (100)", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING )
GAME( 1999, olds100a, olds,       olds, olds,    olds,   ROT0,   "IGS", "Oriental Legend Super / Special (100 alt)", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING )
GAME( 2000, kov2,     pgm,       kov2, sango,    kov2,       ROT0,   "IGS", "Knights of Valour 2", GAME_IMPERFECT_SOUND )
GAME( 2000, kov2106,  kov2,      kov2, sango,    kov2,       ROT0,   "IGS", "Knights of Valour 2 (106)", GAME_IMPERFECT_SOUND )
GAME( 2000, kov2p,    kov2,      kov2, sango,    kov2,       ROT0,   "IGS", "Knights of Valour 2 Plus - Nine Dragons", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING )
GAME( 2001, ddp2,     pgm,        pgm, ddp2,     ddp2,       ROT270, "IGS", "Bee Storm - DoDonPachi II", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING )
GAME( 2001, puzzli2,  pgm,        pgm, sango,    puzzli2,    ROT0,   "IGS", "Puzzli 2 Super", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING )
GAME( 2001, martmast, pgm,       kov2, sango,    martmast,        ROT0,   "IGS", "Martial Masters", GAME_IMPERFECT_SOUND )
GAME( 2001, theglad,  pgm,        pgm, sango,    pgm,        ROT0,   "IGS", "The Gladiator", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING )
GAME( 2002, dmnfrnt,  pgm,        pgm, sango,    pgm,        ROT0,   "IGS", "Demon Front (V102)", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING )
GAME( 2002, dmnfrnta, dmnfrnt,    pgm, sango,    pgm,        ROT0,   "IGS", "Demon Front (V105)", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING )
