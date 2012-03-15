
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
060x  - 1999  - Knights of Valor, Knights of Valor Plus, Knights of Valor Super Heroes
070x  - 1999  - Photo Y2k
080x  - 1999  - Puzzle Star
090x  - 2001  - Puzzli II
100x  - 2001  - Martial Masters

120x  - 2001  - Knights of Valor 2 Plus (9 Dragons?)
130x  - 2001  - DoDonpachi II

0450x - 2002  - Demon Front (also known to be produced / sold in a single PCB version)

(add more)

ToDo:

 IRQ4 generation - Puzzli 2 doens't like this, many other games require it for coins / inputs to work.


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

ASIC 25 + ASIC 12
    ASIC25 is a logic device (not MCU) which performs bitswap operations
    ASIC12 is ... ? (rom overlays?)

    used by:
    Dragon World 2

ASIC 25 + ASIC 22
    ASIC25 is a logic device (not MCU) which performs bitswap operations, and connects to the ASIC22.  It differs per region.
    ASIC22 is an MCU and acts as an encrypted DMA device

    (ASIC22 can be swapped between games with no side-effects, ASIC25 can't)

    used by:
    Dragon World 3
    The Killing Blade

ASIC 25 + ASIC 28
    ASIC25 (see above)
    ASIC28 acts as an encrypted DMA device (updated version of ASIC22 with different encryption etc.)

    used by:
    Oriental Legend Super

ASIC 27 (55857E):
    performs a variety of calculations, quite complex, different per region, supplies region code

    used by:
    Knights of Valour 1 / (Plus?)
    Photo Y2k / Real and Fake

ASIC 27A(55857F/55857G):
    arm7 cpu with 16kb internal rom (different per game / region) + optional external data rom
    probably used to give extra power to the system, lots of calculations are offloaded to it
    used by:
    Knights of Valour Super Heroes / Plus
    Puzzli II
    Oriental Legend Special Plus
    Knights of Valor 2 / 2 Plus
    DoDonPachi II
    Martial Masters
    The Gladiator
    Demon Front
    The Killing Blade Plus
    Spectral VS Generation

    The 55857G variant appears to be protected against reading even from native code.

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
         IDC34     - IDC34 way flat cable plug, PGM can support 4 players max in two cabs,
                     this is jamma connecter for another cab and the P3&P4
         PAL       - Atmel ATF16V8B PAL (DIP20)
         SW1       - Push button switch to enter Test Mode
         SW2       - 8 position DIP Switch (for configuration of PCB/game options)
         SW3       - SPDT switch , to clear the NVRAM and reset the whole system
         TD62064   - Toshiba NPN 50V 1.5A Quad Darlinton Switch; for driving coin meters (DIP16)
         TDA1519A  - Philips 2x 6W Stereo Power AMP (SIL9)
         uPD6379   - NEC 2-channel 16-bit D/A converter 10mW typ. (SOIC8)
         uPC844C   - NEC Quad High Speed Wide Band Operational Amplifier (DIP14)
         V3021     - EM Microelectronic-Marin SA Ultra Low Power 32kHz CMOS Real Time Clock (DIP8)
         VOL       - Volume potentiometer

*/

#define PGMLOGERROR 0
#define PGMARM7LOGERROR 1

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "cpu/arm7/arm7.h"
#include "sound/ics2115.h"
#include "cpu/arm7/arm7core.h"
#include "machine/nvram.h"
#include "includes/pgm.h"
#include "machine/v3021.h"

UINT16 *pgm_mainram;
static void IGS022_reset(running_machine& machine);

static READ16_HANDLER( pgm_videoram_r )
{
	pgm_state *state = space->machine().driver_data<pgm_state>();

	if (offset < 0x4000 / 2)
		return state->m_bg_videoram[offset&0x7ff];
	else if (offset < 0x7000 / 2)
		return state->m_tx_videoram[offset&0xfff];
	else
		return state->m_videoram[offset];
}


static WRITE16_HANDLER( pgm_videoram_w )
{
	pgm_state *state = space->machine().driver_data<pgm_state>();

	if (offset < 0x4000 / 2)
		pgm_bg_videoram_w(space, offset&0x7ff, data, mem_mask);
	else if (offset < 0x7000 / 2)
		pgm_tx_videoram_w(space, offset&0xfff, data, mem_mask);
	else
		COMBINE_DATA(&state->m_videoram[offset]);
}

static READ16_HANDLER ( z80_ram_r )
{
	pgm_state *state = space->machine().driver_data<pgm_state>();
	return (state->m_z80_mainram[offset * 2] << 8) | state->m_z80_mainram[offset * 2 + 1];
}

static READ32_HANDLER( arm7_latch_arm_r )
{
	pgm_state *state = space->machine().driver_data<pgm_state>();

	if (PGMARM7LOGERROR)
		logerror("ARM7: Latch read: %08x (%08x) (%06x)\n", state->m_kov2_latchdata_68k_w, mem_mask, cpu_get_pc(&space->device()));
	return state->m_kov2_latchdata_68k_w;
}

static WRITE32_HANDLER( arm7_latch_arm_w )
{
	pgm_state *state = space->machine().driver_data<pgm_state>();

	if (PGMARM7LOGERROR)
		logerror("ARM7: Latch write: %08x (%08x) (%06x)\n", data, mem_mask, cpu_get_pc(&space->device()));

	COMBINE_DATA(&state->m_kov2_latchdata_arm_w);
}

static READ32_HANDLER( arm7_shareram_r )
{
	pgm_state *state = space->machine().driver_data<pgm_state>();

	if (PGMARM7LOGERROR)
		logerror("ARM7: ARM7 Shared RAM Read: %04x = %08x (%08x) (%06x)\n", offset << 2, state->m_arm7_shareram[offset], mem_mask, cpu_get_pc(&space->device()));
	return state->m_arm7_shareram[offset];
}

static WRITE32_HANDLER( arm7_shareram_w )
{
	pgm_state *state = space->machine().driver_data<pgm_state>();

	if (PGMARM7LOGERROR)
		logerror("ARM7: ARM7 Shared RAM Write: %04x = %08x (%08x) (%06x)\n", offset << 2, data, mem_mask, cpu_get_pc(&space->device()));
	COMBINE_DATA(&state->m_arm7_shareram[offset]);
}

static READ16_HANDLER( arm7_latch_68k_r )
{
	pgm_state *state = space->machine().driver_data<pgm_state>();

	if (PGMARM7LOGERROR)
		logerror("M68K: Latch read: %04x (%04x) (%06x)\n", state->m_kov2_latchdata_arm_w & 0x0000ffff, mem_mask, cpu_get_pc(&space->device()));
	return state->m_kov2_latchdata_arm_w;
}

static WRITE16_HANDLER( arm7_latch_68k_w )
{
	pgm_state *state = space->machine().driver_data<pgm_state>();

	if (PGMARM7LOGERROR)
		logerror("M68K: Latch write: %04x (%04x) (%06x)\n", data & 0x0000ffff, mem_mask, cpu_get_pc(&space->device()));
	COMBINE_DATA(&state->m_kov2_latchdata_68k_w);

	generic_pulse_irq_line(state->m_prot, ARM7_FIRQ_LINE, 1);
	space->machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(200));
	device_spin_until_time(&space->device(), state->m_prot->cycles_to_attotime(200)); // give the arm time to respond (just boosting the interleave doesn't help)
}

static READ16_HANDLER( arm7_ram_r )
{
	pgm_state *state = space->machine().driver_data<pgm_state>();
	UINT16 *share16 = (UINT16 *)state->m_arm7_shareram;

	if (PGMARM7LOGERROR)
		logerror("M68K: ARM7 Shared RAM Read: %04x = %04x (%08x) (%06x)\n", BYTE_XOR_LE(offset), share16[BYTE_XOR_LE(offset)], mem_mask, cpu_get_pc(&space->device()));
	return share16[BYTE_XOR_LE(offset)];
}

static WRITE16_HANDLER( arm7_ram_w )
{
	pgm_state *state = space->machine().driver_data<pgm_state>();
	UINT16 *share16 = (UINT16 *)state->m_arm7_shareram;

	if (PGMARM7LOGERROR)
		logerror("M68K: ARM7 Shared RAM Write: %04x = %04x (%04x) (%06x)\n", BYTE_XOR_LE(offset), data, mem_mask, cpu_get_pc(&space->device()));
	COMBINE_DATA(&share16[BYTE_XOR_LE(offset)]);
}

static WRITE16_HANDLER ( z80_ram_w )
{
	pgm_state *state = space->machine().driver_data<pgm_state>();
	int pc = cpu_get_pc(&space->device());

	if (ACCESSING_BITS_8_15)
		state->m_z80_mainram[offset * 2] = data >> 8;
	if (ACCESSING_BITS_0_7)
		state->m_z80_mainram[offset * 2 + 1] = data;

	if (pc != 0xf12 && pc != 0xde2 && pc != 0x100c50 && pc != 0x100b20)
		if (PGMLOGERROR)
			logerror("Z80: write %04x, %04x @ %04x (%06x)\n", offset * 2, data, mem_mask, cpu_get_pc(&space->device()));
}

static WRITE16_HANDLER ( z80_reset_w )
{
	pgm_state *state = space->machine().driver_data<pgm_state>();

	if (PGMLOGERROR)
		logerror("Z80: reset %04x @ %04x (%06x)\n", data, mem_mask, cpu_get_pc(&space->device()));

	if (data == 0x5050)
	{
		state->m_ics->reset();
		device_set_input_line(state->m_soundcpu, INPUT_LINE_HALT, CLEAR_LINE);
		device_set_input_line(state->m_soundcpu, INPUT_LINE_RESET, PULSE_LINE);
		if(0)
		{
			FILE *out;
			out = fopen("z80ram.bin", "wb");
			fwrite(state->m_z80_mainram, 1, 65536, out);
			fclose(out);
		}
	}
	else
	{
		/* this might not be 100% correct, but several of the games (ddp2, puzzli2 etc. expect the z80 to be turned
           off during data uploads, they write here before the upload */
		device_set_input_line(state->m_soundcpu, INPUT_LINE_HALT, ASSERT_LINE);
	}
}

static WRITE16_HANDLER ( z80_ctrl_w )
{
	if (PGMLOGERROR)
		logerror("Z80: ctrl %04x @ %04x (%06x)\n", data, mem_mask, cpu_get_pc(&space->device()));
}

static WRITE16_HANDLER ( m68k_l1_w )
{
	pgm_state *state = space->machine().driver_data<pgm_state>();

	if(ACCESSING_BITS_0_7)
	{
		if (PGMLOGERROR)
			logerror("SL 1 m68.w %02x (%06x) IRQ\n", data & 0xff, cpu_get_pc(&space->device()));
		soundlatch_w(space, 0, data);
		device_set_input_line(state->m_soundcpu, INPUT_LINE_NMI, PULSE_LINE );
	}
}

static WRITE8_HANDLER( z80_l3_w )
{
	if (PGMLOGERROR)
		logerror("SL 3 z80.w %02x (%04x)\n", data, cpu_get_pc(&space->device()));
	soundlatch3_w(space, 0, data);
}

static void sound_irq( device_t *device, int level )
{
	pgm_state *state = device->machine().driver_data<pgm_state>();
	device_set_input_line(state->m_soundcpu, 0, level);
}

/*static const ics2115_interface pgm_ics2115_interface =
{
    sound_irq
};*/

/*** Misc Protection Handlers *************************************************************/

/* Kov Superheroes */

static READ32_HANDLER( kovsh_arm7_protlatch_r )
{
	pgm_state *state = space->machine().driver_data<pgm_state>();

	space->machine().scheduler().synchronize(); // force resync

	return (state->m_kovsh_highlatch_68k_w << 16) | (state->m_kovsh_lowlatch_68k_w);
}

static WRITE32_HANDLER( kovsh_arm7_protlatch_w )
{
	pgm_state *state = space->machine().driver_data<pgm_state>();

	space->machine().scheduler().synchronize(); // force resync

	if (ACCESSING_BITS_16_31)
	{
		state->m_kovsh_highlatch_arm_w = data >> 16;
		state->m_kovsh_highlatch_68k_w = 0;
	}
	if (ACCESSING_BITS_0_15)
	{
		state->m_kovsh_lowlatch_arm_w = data;
		state->m_kovsh_lowlatch_68k_w = 0;
	}
}

static READ16_HANDLER( kovsh_68k_protlatch_r )
{
	pgm_state *state = space->machine().driver_data<pgm_state>();

	space->machine().scheduler().synchronize(); // force resync

	switch (offset)
	{
		case 1: return state->m_kovsh_highlatch_arm_w;
		case 0: return state->m_kovsh_lowlatch_arm_w;
	}
	return -1;
}

static WRITE16_HANDLER( kovsh_68k_protlatch_w )
{
	pgm_state *state = space->machine().driver_data<pgm_state>();

	space->machine().scheduler().synchronize(); // force resync

	switch (offset)
	{
		case 1:
			state->m_kovsh_highlatch_68k_w = data;
			break;

		case 0:
			state->m_kovsh_lowlatch_68k_w = data;
			break;
	}
}

static READ16_HANDLER( kovsh_arm7_ram_r )
{
	pgm_state *state = space->machine().driver_data<pgm_state>();
	UINT16 *share16 = (UINT16 *)state->m_arm7_shareram;

	if (PGMARM7LOGERROR)
		logerror("M68K: ARM7 Shared RAM Read: %04x = %04x (%08x) (%06x)\n", BYTE_XOR_LE(offset), share16[BYTE_XOR_LE(offset)], mem_mask, cpu_get_pc(&space->device()));
	return share16[BYTE_XOR_LE(offset << 1)];
}

static WRITE16_HANDLER( kovsh_arm7_ram_w )
{
	pgm_state *state = space->machine().driver_data<pgm_state>();
	UINT16 *share16 = (UINT16 *)state->m_arm7_shareram;

	if (PGMARM7LOGERROR)
		logerror("M68K: ARM7 Shared RAM Write: %04x = %04x (%04x) (%06x)\n", BYTE_XOR_LE(offset), data, mem_mask, cpu_get_pc(&space->device()));
	COMBINE_DATA(&share16[BYTE_XOR_LE(offset << 1)]);
}




static READ32_HANDLER( kovsh_arm7_unk_r )
{
	pgm_state *state = space->machine().driver_data<pgm_state>();
	return state->m_kovsh_counter++;
}

static READ32_HANDLER( kovsh_exrom_r )
{
	return 0x00000000;
}


//55857G

static WRITE32_HANDLER( svg_arm7_ram_sel_w )
{
	pgm_state *state = space->machine().driver_data<pgm_state>();
	state->m_svg_ram_sel = data & 1;
}

static READ32_HANDLER( svg_arm7_shareram_r )
{
	pgm_state *state = space->machine().driver_data<pgm_state>();
	return state->m_svg_shareram[state->m_svg_ram_sel & 1][offset];
}

static WRITE32_HANDLER( svg_arm7_shareram_w )
{
	pgm_state *state = space->machine().driver_data<pgm_state>();
	COMBINE_DATA(&state->m_svg_shareram[state->m_svg_ram_sel & 1][offset]);
}

static READ16_HANDLER( svg_m68k_ram_r )
{
	pgm_state *state = space->machine().driver_data<pgm_state>();
	int ram_sel = (state->m_svg_ram_sel & 1) ^ 1;
	UINT16 *share16 = (UINT16 *)(state->m_svg_shareram[ram_sel & 1]);

	return share16[BYTE_XOR_LE(offset)];
}

static WRITE16_HANDLER( svg_m68k_ram_w )
{
	pgm_state *state = space->machine().driver_data<pgm_state>();
	int ram_sel = (state->m_svg_ram_sel & 1) ^ 1;
	UINT16 *share16 = (UINT16 *)(state->m_svg_shareram[ram_sel & 1]);

	COMBINE_DATA(&share16[BYTE_XOR_LE(offset)]);
}

static READ16_HANDLER( svg_68k_nmi_r )
{
	return 0;
}

static WRITE16_HANDLER( svg_68k_nmi_w )
{
	pgm_state *state = space->machine().driver_data<pgm_state>();
	generic_pulse_irq_line(state->m_prot, ARM7_FIRQ_LINE, 1);
	space->machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(200));
	device_spin_until_time(&space->device(), state->m_prot->cycles_to_attotime(200)); // give the arm time to respond (just boosting the interleave doesn't help)
}

static WRITE16_HANDLER( svg_latch_68k_w )
{
	pgm_state *state = space->machine().driver_data<pgm_state>();
	if (PGMARM7LOGERROR)
		logerror("M68K: Latch write: %04x (%04x) (%06x)\n", data & 0x0000ffff, mem_mask, cpu_get_pc(&space->device()));
	COMBINE_DATA(&state->m_kov2_latchdata_68k_w);
}

/*** Memory Maps *************************************************************/

/*** Z80 (sound CPU)**********************************************************/

static ADDRESS_MAP_START( z80_mem, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xffff) AM_RAM AM_BASE_MEMBER(pgm_state, m_z80_mainram)
ADDRESS_MAP_END

static ADDRESS_MAP_START( z80_io, AS_IO, 8 )
	AM_RANGE(0x8000, 0x8003) AM_DEVREADWRITE("ics", ics2115_device::read, ics2115_device::write)
	AM_RANGE(0x8100, 0x81ff) AM_READWRITE(soundlatch3_r, z80_l3_w)
	AM_RANGE(0x8200, 0x82ff) AM_READWRITE(soundlatch_r, soundlatch_w)
	AM_RANGE(0x8400, 0x84ff) AM_READWRITE(soundlatch2_r, soundlatch2_w)
ADDRESS_MAP_END

/*** 68000 (main CPU) + variants for protection devices **********************/

static ADDRESS_MAP_START( pgm_base_mem, AS_PROGRAM, 16)
	AM_RANGE(0x700006, 0x700007) AM_WRITENOP // Watchdog?

	AM_RANGE(0x800000, 0x81ffff) AM_RAM AM_MIRROR(0x0e0000) AM_BASE(&pgm_mainram) AM_SHARE("sram") /* Main Ram */

	AM_RANGE(0x900000, 0x907fff) AM_MIRROR(0x0f8000) AM_READWRITE(pgm_videoram_r, pgm_videoram_w) AM_BASE_MEMBER(pgm_state, m_videoram) /* IGS023 VIDEO CHIP */
	AM_RANGE(0xa00000, 0xa011ff) AM_RAM_WRITE(paletteram16_xRRRRRGGGGGBBBBB_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0xb00000, 0xb0ffff) AM_RAM AM_BASE_MEMBER(pgm_state, m_videoregs) /* Video Regs inc. Zoom Table */

	AM_RANGE(0xc00002, 0xc00003) AM_READWRITE(soundlatch_word_r, m68k_l1_w)
	AM_RANGE(0xc00004, 0xc00005) AM_READWRITE(soundlatch2_word_r, soundlatch2_word_w)
	AM_RANGE(0xc00006, 0xc00007) AM_DEVREADWRITE8_MODERN("rtc", v3021_device, read, write, 0x00ff)
	AM_RANGE(0xc00008, 0xc00009) AM_WRITE(z80_reset_w)
	AM_RANGE(0xc0000a, 0xc0000b) AM_WRITE(z80_ctrl_w)
	AM_RANGE(0xc0000c, 0xc0000d) AM_READWRITE(soundlatch3_word_r, soundlatch3_word_w)

	AM_RANGE(0xc08000, 0xc08001) AM_READ_PORT("P1P2")
	AM_RANGE(0xc08002, 0xc08003) AM_READ_PORT("P3P4")
	AM_RANGE(0xc08004, 0xc08005) AM_READ_PORT("Service")
	AM_RANGE(0xc08006, 0xc08007) AM_READ_PORT("DSW")

	AM_RANGE(0xc10000, 0xc1ffff) AM_READWRITE(z80_ram_r, z80_ram_w) /* Z80 Program */
ADDRESS_MAP_END

static ADDRESS_MAP_START( pgm_mem, AS_PROGRAM, 16)
	AM_IMPORT_FROM(pgm_base_mem)
	AM_RANGE(0x000000, 0x01ffff) AM_ROM   /* BIOS ROM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( pgm_basic_mem, AS_PROGRAM, 16)
	AM_IMPORT_FROM(pgm_mem)
	AM_RANGE(0x100000, 0x3fffff) AM_ROMBANK("bank1") /* Game ROM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( killbld_mem, AS_PROGRAM, 16)
	AM_IMPORT_FROM(pgm_mem)
	AM_RANGE(0x100000, 0x2fffff) AM_ROMBANK("bank1") /* Game ROM */
	AM_RANGE(0x300000, 0x303fff) AM_RAM AM_BASE_MEMBER(pgm_state, m_sharedprotram) // Shared with protection device
ADDRESS_MAP_END

static ADDRESS_MAP_START( olds_mem, AS_PROGRAM, 16)
	AM_IMPORT_FROM(pgm_mem)
	AM_RANGE(0x100000, 0x3fffff) AM_ROMBANK("bank1") /* Game ROM */
	AM_RANGE(0x400000, 0x403fff) AM_RAM AM_BASE_MEMBER(pgm_state, m_sharedprotram) // Shared with protection device
ADDRESS_MAP_END

/* 55857E? */
/* Knights of Valor, Photo Y2k */
/*  no execute only space? */
static ADDRESS_MAP_START( kov_map, AS_PROGRAM, 16)
	AM_IMPORT_FROM(pgm_mem)
	AM_RANGE(0x100000, 0x4effff) AM_ROMBANK("bank1") /* Game ROM */
	AM_RANGE(0x4f0000, 0x4f003f) AM_READWRITE(kovsh_arm7_ram_r, kovsh_arm7_ram_w) /* ARM7 Shared RAM */
	AM_RANGE(0x500000, 0x500005) AM_READWRITE(kovsh_68k_protlatch_r, kovsh_68k_protlatch_w) /* ARM7 Latch */
ADDRESS_MAP_END

/* 55857F? */
/* Knights of Valor 2, Martial Masters, DoDonpachi 2 */
/*  no execute only space? */
static ADDRESS_MAP_START( kov2_mem, AS_PROGRAM, 16)
	AM_IMPORT_FROM(pgm_mem)
	AM_RANGE(0x100000, 0x5fffff) AM_ROMBANK("bank1") /* Game ROM */
	AM_RANGE(0xd00000, 0xd0ffff) AM_READWRITE(arm7_ram_r, arm7_ram_w) /* ARM7 Shared RAM */
	AM_RANGE(0xd10000, 0xd10001) AM_READWRITE(arm7_latch_68k_r, arm7_latch_68k_w) /* ARM7 Latch */
ADDRESS_MAP_END

/* 55857G? */
/* Demon Front, The Gladiator, Happy 6-in-1, Spectral Vs. Generation, Killing Blade EX */
/*  the ones with an EXECUTE ONLY region of ARM space? */
static ADDRESS_MAP_START( svg_68k_mem, AS_PROGRAM, 16)
	AM_IMPORT_FROM(pgm_mem)
	AM_RANGE(0x100000, 0x1fffff) AM_ROMBANK("bank1")  /* Game ROM */

	AM_RANGE(0x500000, 0x51ffff) AM_READWRITE(svg_m68k_ram_r, svg_m68k_ram_w)    /* ARM7 Shared RAM */
	AM_RANGE(0x5c0000, 0x5c0001) AM_READWRITE(svg_68k_nmi_r, svg_68k_nmi_w)      /* ARM7 FIQ */
	AM_RANGE(0x5c0300, 0x5c0301) AM_READWRITE(arm7_latch_68k_r, svg_latch_68k_w) /* ARM7 Latch */
ADDRESS_MAP_END


static ADDRESS_MAP_START( cavepgm_mem, AS_PROGRAM, 16)
	AM_IMPORT_FROM(pgm_base_mem)
	AM_RANGE(0x000000, 0x3fffff) AM_ROM
	/* protection devices installed (simulated) later */
ADDRESS_MAP_END

/*** ARM7 (protection CPUs) **************************************************/

static ADDRESS_MAP_START( 55857E_arm7_map, AS_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x00003fff) AM_ROM
	AM_RANGE(0x08100000, 0x083fffff) AM_READ(kovsh_exrom_r) // unpopulated, returns 0 to keep checksum happy
	AM_RANGE(0x10000000, 0x100003ff) AM_RAM // internal ram for asic
	AM_RANGE(0x40000000, 0x40000003) AM_READWRITE(kovsh_arm7_protlatch_r, kovsh_arm7_protlatch_w)
	AM_RANGE(0x40000008, 0x4000000b) AM_WRITENOP // ?
	AM_RANGE(0x4000000c, 0x4000000f) AM_READ(kovsh_arm7_unk_r)
	AM_RANGE(0x50800000, 0x5080003f) AM_READWRITE(arm7_shareram_r, arm7_shareram_w) AM_BASE_MEMBER(pgm_state, m_arm7_shareram)
	AM_RANGE(0x50000000, 0x500003ff) AM_RAM // uploads xor table to decrypt 68k rom here
ADDRESS_MAP_END

static ADDRESS_MAP_START( 55857F_arm7_map, AS_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x00003fff) AM_ROM
	AM_RANGE(0x08000000, 0x083fffff) AM_ROM AM_REGION("user1", 0)
	AM_RANGE(0x10000000, 0x100003ff) AM_RAM
	AM_RANGE(0x18000000, 0x1800ffff) AM_RAM
	AM_RANGE(0x38000000, 0x38000003) AM_READWRITE(arm7_latch_arm_r, arm7_latch_arm_w) /* 68k Latch */
	AM_RANGE(0x48000000, 0x4800ffff) AM_READWRITE(arm7_shareram_r, arm7_shareram_w) AM_BASE_MEMBER(pgm_state, m_arm7_shareram)
	AM_RANGE(0x50000000, 0x500003ff) AM_RAM
ADDRESS_MAP_END


static ADDRESS_MAP_START( 55857G_arm7_map, AS_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x00003fff) AM_ROM
	AM_RANGE(0x08000000, 0x087fffff) AM_ROM AM_REGION("user1", 0)
	AM_RANGE(0x10000000, 0x100003ff) AM_RAM
	AM_RANGE(0x18000000, 0x1803ffff) AM_RAM
	AM_RANGE(0x38000000, 0x3801ffff) AM_READWRITE(svg_arm7_shareram_r, svg_arm7_shareram_w)
	AM_RANGE(0x48000000, 0x48000003) AM_READWRITE(arm7_latch_arm_r, arm7_latch_arm_w) /* 68k Latch */
	AM_RANGE(0x40000018, 0x4000001b) AM_WRITE(svg_arm7_ram_sel_w) /* RAM SEL */
	AM_RANGE(0x50000000, 0x500003ff) AM_RAM
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
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( orlegend )
	PORT_INCLUDE ( pgm )

	PORT_MODIFY("Region")
	PORT_DIPNAME( 0x0003, 0x0000, DEF_STR( Region ) )
	PORT_CONFSETTING(      0x0000, DEF_STR( World ) )
    PORT_CONFSETTING(      0x0001, "World (duplicate)" ) // again?
	PORT_CONFSETTING(      0x0002, DEF_STR( Korea ) )
	PORT_CONFSETTING(      0x0003, DEF_STR( China ) )
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
	PORT_CONFNAME( 0x000f, 0x0005, DEF_STR( Region ) )
	PORT_CONFSETTING(      0x0000, DEF_STR( China ) )
	PORT_CONFSETTING(      0x0001, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(      0x0002, "Japan (Alta license)" )
	PORT_CONFSETTING(      0x0003, DEF_STR( Korea ) )
	PORT_CONFSETTING(      0x0004, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(      0x0005, DEF_STR( World ) )
INPUT_PORTS_END

static INPUT_PORTS_START( sango_ch )
	PORT_INCLUDE ( pgm )

	PORT_MODIFY("Region")	/* Region - supplied by protection device */
	PORT_CONFNAME( 0x000f, 0x0000, DEF_STR( Region ) )
	PORT_CONFSETTING(      0x0000, DEF_STR( China ) )
	PORT_CONFSETTING(      0x0001, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(      0x0002, "Japan (Alta license)" )
	PORT_CONFSETTING(      0x0003, DEF_STR( Korea ) )
	PORT_CONFSETTING(      0x0004, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(      0x0005, DEF_STR( World ) )
INPUT_PORTS_END

static INPUT_PORTS_START( dw3 )
	PORT_INCLUDE ( pgm )

	PORT_MODIFY("Region")	/* Region - supplied by protection device */
	PORT_CONFNAME( 0x000f, 0x0006, DEF_STR( Region ) )
	PORT_CONFSETTING(      0x0000, "0" )
	PORT_CONFSETTING(      0x0001, "1" )
	PORT_CONFSETTING(      0x0002, "2" )
	PORT_CONFSETTING(      0x0003, "3" )
	PORT_CONFSETTING(      0x0004, "4" )
	PORT_CONFSETTING(      0x0005, "5" )
	PORT_CONFSETTING(      0x0006, DEF_STR( World ) )
	PORT_CONFSETTING(      0x0007, "7" )

INPUT_PORTS_END


static INPUT_PORTS_START( olds )
	PORT_INCLUDE ( pgm )

	PORT_MODIFY("Region")	/* Region - supplied by protection device */
	PORT_CONFNAME( 0x000f, 0x0006, DEF_STR( Region ) )
	/* includes the following regions:
    1 = taiwan, 2 = china, 3 = japan (title = orlegend special),
    4 = korea, 5 = hong kong, 6 = world */
	PORT_CONFSETTING(      0x0001, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(      0x0002, DEF_STR( China ) )
	PORT_CONFSETTING(      0x0003, DEF_STR( Japan ) )
	PORT_CONFSETTING(      0x0004, DEF_STR( Korea ) )
	PORT_CONFSETTING(      0x0005, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(      0x0006, DEF_STR( World ) )
INPUT_PORTS_END

static INPUT_PORTS_START( killbld )
	PORT_INCLUDE ( pgm )

	PORT_MODIFY("Region")	/* Region - supplied by protection device */
	PORT_DIPNAME( 0x00ff, 0x0021, "Region (not currently working)" ) // different regions supply different protection code sequences, we only have the China one ATM
	PORT_DIPSETTING(      0x0016, DEF_STR( Taiwan ) )
	PORT_DIPSETTING(      0x0017, DEF_STR( China ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( Hong_Kong ) )
	PORT_DIPSETTING(      0x0019, DEF_STR( Japan ) )
//  PORT_DIPSETTING(      0x001a, "1a" ) // invalid
//  PORT_DIPSETTING(      0x001b, "1b" ) // invalid
//  PORT_DIPSETTING(      0x001c, "1c" ) // invalid
//  PORT_DIPSETTING(      0x001d, "1d" ) // invalid
//  PORT_DIPSETTING(      0x001e, "1e" ) // invalid
//  PORT_DIPSETTING(      0x001f, "1f" ) // invalid
	PORT_DIPSETTING(      0x0020, DEF_STR( Korea ) )
	PORT_DIPSETTING(      0x0021, DEF_STR( World ) )
INPUT_PORTS_END

static INPUT_PORTS_START( photoy2k )
	PORT_INCLUDE ( pgm )

	PORT_MODIFY("Region")	/* Region - supplied by protection device */
	PORT_CONFNAME( 0x000f, 0x0003, DEF_STR( Region ) )
	PORT_CONFSETTING(      0x0000, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(      0x0001, DEF_STR( China ) )
	PORT_CONFSETTING(      0x0002, "Japan (Alta license)" )
	PORT_CONFSETTING(      0x0003, DEF_STR( World ) )
	PORT_CONFSETTING(      0x0004, DEF_STR( Korea ) )
	PORT_CONFSETTING(      0x0005, DEF_STR( Hong_Kong ) )
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
	PORT_CONFNAME( 0x000f, 0x0005, DEF_STR( Region ) )
	PORT_CONFSETTING(      0x0000, DEF_STR( China ) )
	PORT_CONFSETTING(      0x0001, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(      0x0002, "Japan (Cave license)" )
	PORT_CONFSETTING(      0x0003, DEF_STR( Korea ) )
	PORT_CONFSETTING(      0x0004, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(      0x0005, DEF_STR( World ) )
INPUT_PORTS_END

static INPUT_PORTS_START( oldsplus )
	PORT_INCLUDE ( pgm )

	PORT_MODIFY("Region")	/* Region - supplied by protection device */
	PORT_CONFNAME( 0x000f, 0x0001, DEF_STR( Region ) )
	PORT_CONFSETTING(      0x0001, DEF_STR( China ) )
	PORT_CONFSETTING(      0x0002, DEF_STR( Japan ) )
	PORT_CONFSETTING(      0x0003, DEF_STR( Korea ) )
	PORT_CONFSETTING(      0x0004, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(      0x0005, DEF_STR( World ) )
	PORT_CONFSETTING(      0x0006, DEF_STR( Taiwan ) )
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
	0, // determined in init
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
	GFXDECODE_ENTRY( "tiles", 0, pgm8_charlayout,    0x800, 32  ) /* 8x8x4 Tiles */
	// we have to unpack the data before we can decode it as 32x32, hence we don't know how many tiles etc. in advance, see INIT
	//GFXDECODE_ENTRY( "gfx2", 0, pgm32_charlayout,   0x400, 32  ) /* 32x32x5 Tiles */
GFXDECODE_END

/*** Machine Driver **********************************************************/

/* only dragon world 2 NEEDs irq4, Puzzli 2 explicitly doesn't want it, what
   is the source? maybe the protection device? */
static TIMER_DEVICE_CALLBACK( drgw_interrupt )
{
	pgm_state *state = timer.machine().driver_data<pgm_state>();
	int scanline = param;

	if(scanline == 224)
		device_set_input_line(state->m_maincpu, 6, HOLD_LINE);

	if(scanline == 0)
		if (!state->m_irq4_disabled) device_set_input_line(state->m_maincpu, 4, HOLD_LINE);
}

static MACHINE_START( pgm )
{
	pgm_state *state = machine.driver_data<pgm_state>();

//  machine.base_datetime(state->m_systime);

	state->m_maincpu = machine.device<cpu_device>("maincpu");
	state->m_soundcpu = machine.device<cpu_device>("soundcpu");
	state->m_prot = machine.device<cpu_device>("prot");
	state->m_ics = machine.device("ics");

//  state->save_item(NAME(state->m_cal_val));
//  state->save_item(NAME(state->m_cal_mask));
//  state->save_item(NAME(state->m_cal_com));
//  state->save_item(NAME(state->m_cal_cnt));
}

static MACHINE_RESET( pgm )
{
//  pgm_state *state = machine.driver_data<pgm_state>();

	cputag_set_input_line(machine, "soundcpu", INPUT_LINE_HALT, ASSERT_LINE);

//  state->m_cal_val = 0;
//  state->m_cal_mask = 0;
//  state->m_cal_com = 0;
//  state->m_cal_cnt = 0;
}

static MACHINE_RESET( killbld );
static MACHINE_RESET( dw3 );
static MACHINE_RESET( olds );
extern MACHINE_RESET( kov );

MACHINE_CONFIG_FRAGMENT( pgmbase )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 20000000) /* 20 mhz! verified on real board */
	MCFG_CPU_PROGRAM_MAP(pgm_basic_mem)
	MCFG_CPU_VBLANK_INT("screen", irq6_line_hold)
	MCFG_TIMER_ADD_SCANLINE("scantimer", drgw_interrupt, "screen", 0, 1)

	MCFG_CPU_ADD("soundcpu", Z80, 33868800/4)
	MCFG_CPU_PROGRAM_MAP(z80_mem)
	MCFG_CPU_IO_MAP(z80_io)

	MCFG_MACHINE_START( pgm )
	MCFG_MACHINE_RESET( pgm )
	MCFG_NVRAM_ADD_0FILL("sram")

	MCFG_V3021_ADD("rtc")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60) // killing blade won't boot (just displays 'error') if this is lower than 59.9 or higher than 60.1 .. are actual PGM boards different to the Cave one?
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 56*8-1, 0*8, 28*8-1)
	MCFG_SCREEN_UPDATE_STATIC(pgm)
	MCFG_SCREEN_VBLANK_STATIC(pgm)

	MCFG_GFXDECODE(pgm)
	MCFG_PALETTE_LENGTH(0x1200/2)

	MCFG_VIDEO_START(pgm)

	/*sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
    MCFG_ICS2115_ADD("ics", 0, sound_irq)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 5.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( pgm, pgm_state )
	MCFG_FRAGMENT_ADD(pgmbase)
MACHINE_CONFIG_END




static MACHINE_CONFIG_DERIVED( killbld, pgm )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(killbld_mem)

	MCFG_MACHINE_RESET(killbld)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( dw3, pgm )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(killbld_mem)

	MCFG_MACHINE_RESET(dw3)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( olds, pgm )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(olds_mem)

	MCFG_MACHINE_RESET(olds)
MACHINE_CONFIG_END

/******* ARM 55857E *******/

static MACHINE_CONFIG_DERIVED( kov, pgm )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(kov_map)

	/* protection CPU */
	MCFG_CPU_ADD("prot", ARM7, 20000000)	// 55857E?
	MCFG_CPU_PROGRAM_MAP(55857E_arm7_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( kov_disabled_arm, pgm ) // for simulated cases
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(pgm_basic_mem)

	/* protection CPU */
	MCFG_CPU_ADD("prot", ARM7, 20000000)	// 55857E?
	MCFG_CPU_PROGRAM_MAP(55857E_arm7_map)
	MCFG_DEVICE_DISABLE()
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( kov_simulated_arm, kov_disabled_arm ) // for simulated cases
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(kov_map)

	MCFG_MACHINE_RESET(kov)
MACHINE_CONFIG_END

/******* ARM 55857F *******/

static MACHINE_CONFIG_DERIVED( kov2, pgm )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(kov2_mem)

	/* protection CPU */
	MCFG_CPU_ADD("prot", ARM7, 20000000)	// 55857F
	MCFG_CPU_PROGRAM_MAP(55857F_arm7_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( kov2_disabled_arm, pgm ) // for simulated cases
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(pgm_basic_mem)

	/* protection CPU */
	MCFG_CPU_ADD("prot", ARM7, 20000000)	// 55857F
	MCFG_CPU_PROGRAM_MAP(55857F_arm7_map)
	MCFG_DEVICE_DISABLE()
MACHINE_CONFIG_END

/******* ARM 55857G *******/

static MACHINE_CONFIG_DERIVED( svg, pgm )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(svg_68k_mem)

	/* protection CPU */
	MCFG_CPU_ADD("prot", ARM7, 20000000)	// 55857G
	MCFG_CPU_PROGRAM_MAP(55857G_arm7_map)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( svg_disabled_arm, pgm ) // for simulated cases
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(pgm_basic_mem)

	/* protection CPU */
	MCFG_CPU_ADD("prot", ARM7, 20000000)	// 55857G
	MCFG_CPU_PROGRAM_MAP(55857G_arm7_map)
	MCFG_DEVICE_DISABLE()
MACHINE_CONFIG_END

class cavepgm_state : public pgm_state
{
public:
	cavepgm_state(const machine_config &mconfig, device_type type, const char *tag)
		: pgm_state(mconfig, type, tag) {

		m_ddp3internal_slot = 0;
	}

	UINT16 m_value0;
	UINT16 m_value1;
	UINT16 m_valuekey;
	UINT16 m_ddp3lastcommand;
	UINT32 m_valueresponse;
	int m_ddp3internal_slot;
	UINT32 m_ddp3slots[0x100];
	INT16 m_ddp3thrust;
};

static MACHINE_START( cavepgm )
{
	MACHINE_START_CALL(pgm);

	cavepgm_state *state = machine.driver_data<cavepgm_state>();

	state->save_item(NAME(state->m_value0));
	state->save_item(NAME(state->m_value1));
	state->save_item(NAME(state->m_valuekey));
	state->save_item(NAME(state->m_valueresponse));
	state->save_item(NAME(state->m_ddp3internal_slot));
	state->save_item(NAME(state->m_ddp3slots));
}

static MACHINE_CONFIG_START( cavepgm, cavepgm_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 20000000)
	MCFG_CPU_PROGRAM_MAP(cavepgm_mem)
	MCFG_TIMER_ADD_SCANLINE("scantimer", drgw_interrupt, "screen", 0, 1)

	MCFG_CPU_ADD("soundcpu", Z80, 33868800/4)
	MCFG_CPU_PROGRAM_MAP(z80_mem)
	MCFG_CPU_IO_MAP(z80_io)

	MCFG_MACHINE_START( cavepgm )
	MCFG_MACHINE_RESET( pgm )
	MCFG_NVRAM_ADD_0FILL("sram")

	MCFG_V3021_ADD("rtc")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(59.17) // verified on pcb
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 56*8-1, 0*8, 28*8-1)
	MCFG_SCREEN_UPDATE_STATIC(pgm)
	MCFG_SCREEN_VBLANK_STATIC(pgm)

	MCFG_GFXDECODE(pgm)
	MCFG_PALETTE_LENGTH(0x1200/2)

	MCFG_VIDEO_START(pgm)

	/*sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
    MCFG_ICS2115_ADD("ics", 0, sound_irq)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 5.0)
MACHINE_CONFIG_END

// for now we're using a protection sim close ot the CavePgm one, so use that state.
static MACHINE_CONFIG_DERIVED( puzzli2_disabled_arm, cavepgm )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(pgm_basic_mem)

	/* protection CPU */
	MCFG_CPU_ADD("prot", ARM7, 20000000)	// 55857F?
	MCFG_CPU_PROGRAM_MAP(55857F_arm7_map)
	MCFG_DEVICE_DISABLE()
MACHINE_CONFIG_END


/*** Rom Loading *************************************************************/

/* take note of "gfx2" needed for expanding the 32x32x5bpp data and
   "sprmask" needed for expanding the Sprite Colour Data */

#define ROM_LOAD16_WORD_SWAP_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(bios+1)) /* Note '+1' */

#define PGM_68K_BIOS \
	ROM_SYSTEM_BIOS( 0, "v2",     "PGM Bios V2" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 0, "pgm_p02s.u20",    0x00000, 0x020000, CRC(78c15fa2) SHA1(885a6558e022602cc6f482ac9667ba9f61e75092) ) /* Version 2 (Label: IGS | PGM P02S | 1P0792D1 | J992438 )*/ \
	ROM_SYSTEM_BIOS( 1, "v1",     "PGM Bios V1" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 1, "pgm_p01s.u20",    0x00000, 0x020000, CRC(e42b166e) SHA1(2a9df9ec746b14b74fae48b1a438da14973702ea) ) /* Version 1 */ \

#define PGM_AUDIO_BIOS \
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) \

#define PGM_VIDEO_BIOS \
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) \

/* The Bios - NOT A GAME */
ROM_START( pgm )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS

	ROM_REGION( 0x200000, "tiles", 0 ) /* 8x8 Text Layer Tiles */
	PGM_VIDEO_BIOS

	ROM_REGION( 0x200000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS

	ROM_REGION( 0x1000000, "sprcol", ROMREGION_ERASEFF ) /* Sprite Colour Data */
	ROM_REGION( 0x1000000, "sprmask", ROMREGION_ERASEFF ) /* Sprite Masks + Colour Indexes */
ROM_END

ROM_START( orlegend )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "p0103.rom",    0x100000, 0x200000, CRC(d5e93543) SHA1(f081edc26514ca8354c13c7f6f89aba8e4d3e7d2) )

	ROM_REGION( 0x800000, "tiles",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0100.rom",    0x180000, 0x400000, CRC(61425e1e) SHA1(20753b86fc12003cfd763d903f034dbba8010b32) )

	ROM_REGION( 0x1800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0100.rom",    0x0000000, 0x400000, CRC(8b3bd88a) SHA1(42db3a60c6ba9d83ebe2008c8047d094027f65a7) )
	ROM_LOAD( "a0101.rom",    0x0400000, 0x400000, CRC(3b9e9644) SHA1(5b95ec1d25c3bc3504c93547f5adb5ce24376405) )
	ROM_LOAD( "a0102.rom",    0x0800000, 0x400000, CRC(069e2c38) SHA1(9bddca8c2f5bd80f4abe4e1f062751736dc151dd) )
	ROM_LOAD( "a0103.rom",    0x0c00000, 0x400000, CRC(4460a3fd) SHA1(cbebdb65c17605853f7d0b298018dd8801a25a58) )
	ROM_LOAD( "a0104.rom",    0x1000000, 0x400000, CRC(5f8abb56) SHA1(6c1ddc0309862a141aa0c0f63b641aec9257aaee) )
	ROM_LOAD( "a0105.rom",    0x1400000, 0x400000, CRC(a17a7147) SHA1(44eeb43c6b0ebb829559a20ae357383fbdeecd82) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0100.rom",    0x0000000, 0x400000, CRC(69d2e48c) SHA1(5b5f759007264c07b3b39be8e03a713698e1fc2a) )
	ROM_LOAD( "b0101.rom",    0x0400000, 0x400000, CRC(0d587bf3) SHA1(5347828b0a6e4ddd7a263663d2c2604407e4d49c) )
	ROM_LOAD( "b0102.rom",    0x0800000, 0x400000, CRC(43823c1e) SHA1(e10a1a9a81b51b11044934ff702e35d8d7ab1b08) )

	ROM_REGION( 0x600000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0100.rom",    0x400000, 0x200000, CRC(e5c36c83) SHA1(50c6f66770e8faa3df349f7d68c407a7ad021716) )
ROM_END

ROM_START( orlegende )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "p0102.rom",    0x100000, 0x200000, CRC(4d0f6cc5) SHA1(8d41f0a712fb11a1da865f5159e5e27447b4388a) )

	ROM_REGION( 0x800000, "tiles",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0100.rom",    0x180000, 0x400000, CRC(61425e1e) SHA1(20753b86fc12003cfd763d903f034dbba8010b32) )

	ROM_REGION( 0x1800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0100.rom",    0x0000000, 0x400000, CRC(8b3bd88a) SHA1(42db3a60c6ba9d83ebe2008c8047d094027f65a7) )
	ROM_LOAD( "a0101.rom",    0x0400000, 0x400000, CRC(3b9e9644) SHA1(5b95ec1d25c3bc3504c93547f5adb5ce24376405) )
	ROM_LOAD( "a0102.rom",    0x0800000, 0x400000, CRC(069e2c38) SHA1(9bddca8c2f5bd80f4abe4e1f062751736dc151dd) )
	ROM_LOAD( "a0103.rom",    0x0c00000, 0x400000, CRC(4460a3fd) SHA1(cbebdb65c17605853f7d0b298018dd8801a25a58) )
	ROM_LOAD( "a0104.rom",    0x1000000, 0x400000, CRC(5f8abb56) SHA1(6c1ddc0309862a141aa0c0f63b641aec9257aaee) )
	ROM_LOAD( "a0105.rom",    0x1400000, 0x400000, CRC(a17a7147) SHA1(44eeb43c6b0ebb829559a20ae357383fbdeecd82) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0100.rom",    0x0000000, 0x400000, CRC(69d2e48c) SHA1(5b5f759007264c07b3b39be8e03a713698e1fc2a) )
	ROM_LOAD( "b0101.rom",    0x0400000, 0x400000, CRC(0d587bf3) SHA1(5347828b0a6e4ddd7a263663d2c2604407e4d49c) )
	ROM_LOAD( "b0102.rom",    0x0800000, 0x400000, CRC(43823c1e) SHA1(e10a1a9a81b51b11044934ff702e35d8d7ab1b08) )

	ROM_REGION( 0x600000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0100.rom",    0x400000, 0x200000, CRC(e5c36c83) SHA1(50c6f66770e8faa3df349f7d68c407a7ad021716) )
ROM_END

ROM_START( orlegendc )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "p0101.160",    0x100000, 0x200000, CRC(b24f0c1e) SHA1(a2cf75d739681f091c24ef78ed6fc13aa8cfe0c6) )

	ROM_REGION( 0x800000, "tiles",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0100.rom",    0x180000, 0x400000, CRC(61425e1e) SHA1(20753b86fc12003cfd763d903f034dbba8010b32) )

	ROM_REGION( 0x1800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0100.rom",    0x0000000, 0x400000, CRC(8b3bd88a) SHA1(42db3a60c6ba9d83ebe2008c8047d094027f65a7) )
	ROM_LOAD( "a0101.rom",    0x0400000, 0x400000, CRC(3b9e9644) SHA1(5b95ec1d25c3bc3504c93547f5adb5ce24376405) )
	ROM_LOAD( "a0102.rom",    0x0800000, 0x400000, CRC(069e2c38) SHA1(9bddca8c2f5bd80f4abe4e1f062751736dc151dd) )
	ROM_LOAD( "a0103.rom",    0x0c00000, 0x400000, CRC(4460a3fd) SHA1(cbebdb65c17605853f7d0b298018dd8801a25a58) )
	ROM_LOAD( "a0104.rom",    0x1000000, 0x400000, CRC(5f8abb56) SHA1(6c1ddc0309862a141aa0c0f63b641aec9257aaee) )
	ROM_LOAD( "a0105.rom",    0x1400000, 0x400000, CRC(a17a7147) SHA1(44eeb43c6b0ebb829559a20ae357383fbdeecd82) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0100.rom",    0x0000000, 0x400000, CRC(69d2e48c) SHA1(5b5f759007264c07b3b39be8e03a713698e1fc2a) )
	ROM_LOAD( "b0101.rom",    0x0400000, 0x400000, CRC(0d587bf3) SHA1(5347828b0a6e4ddd7a263663d2c2604407e4d49c) )
	ROM_LOAD( "b0102.rom",    0x0800000, 0x400000, CRC(43823c1e) SHA1(e10a1a9a81b51b11044934ff702e35d8d7ab1b08) )

	ROM_REGION( 0x600000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0100.rom",    0x400000, 0x200000, CRC(e5c36c83) SHA1(50c6f66770e8faa3df349f7d68c407a7ad021716) )
ROM_END

ROM_START( orlegendca )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "p0101.102",    0x100000, 0x200000, CRC(7a22e1cb) SHA1(4fe0fde00521b0915146334ea7213f3eb7e2affc) )

	ROM_REGION( 0x800000, "tiles",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0100.rom",    0x180000, 0x400000, CRC(61425e1e) SHA1(20753b86fc12003cfd763d903f034dbba8010b32) )

	ROM_REGION( 0x1800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0100.rom",    0x0000000, 0x400000, CRC(8b3bd88a) SHA1(42db3a60c6ba9d83ebe2008c8047d094027f65a7) )
	ROM_LOAD( "a0101.rom",    0x0400000, 0x400000, CRC(3b9e9644) SHA1(5b95ec1d25c3bc3504c93547f5adb5ce24376405) )
	ROM_LOAD( "a0102.rom",    0x0800000, 0x400000, CRC(069e2c38) SHA1(9bddca8c2f5bd80f4abe4e1f062751736dc151dd) )
	ROM_LOAD( "a0103.rom",    0x0c00000, 0x400000, CRC(4460a3fd) SHA1(cbebdb65c17605853f7d0b298018dd8801a25a58) )
	ROM_LOAD( "a0104.rom",    0x1000000, 0x400000, CRC(5f8abb56) SHA1(6c1ddc0309862a141aa0c0f63b641aec9257aaee) )
	ROM_LOAD( "a0105.rom",    0x1400000, 0x400000, CRC(a17a7147) SHA1(44eeb43c6b0ebb829559a20ae357383fbdeecd82) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0100.rom",    0x0000000, 0x400000, CRC(69d2e48c) SHA1(5b5f759007264c07b3b39be8e03a713698e1fc2a) )
	ROM_LOAD( "b0101.rom",    0x0400000, 0x400000, CRC(0d587bf3) SHA1(5347828b0a6e4ddd7a263663d2c2604407e4d49c) )
	ROM_LOAD( "b0102.rom",    0x0800000, 0x400000, CRC(43823c1e) SHA1(e10a1a9a81b51b11044934ff702e35d8d7ab1b08) )

	ROM_REGION( 0x600000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0100.rom",    0x400000, 0x200000, CRC(e5c36c83) SHA1(50c6f66770e8faa3df349f7d68c407a7ad021716) )
ROM_END


/*

Oriental Legend / Xi You Shi E Zhuan (CHINA 111 Ver.)
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

ROM_START( orlegend111c )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "olv111ch.u6",     0x100001, 0x080000, CRC(5fb86373) SHA1(2fc58eff1f38754c75819fde666244b867ca4f05) )
	ROM_LOAD16_BYTE( "olv111ch.u9",     0x100000, 0x080000, CRC(83cf09c8) SHA1(959780b45326059517f3008a356657f4f3d2908f) )
	ROM_LOAD16_BYTE( "olv111ch.u7",     0x200001, 0x080000, CRC(6ee79faf) SHA1(039b4b07b8577f0d3022ae01210c00375624cb3c) )
	ROM_LOAD16_BYTE( "olv111ch.u11",    0x200000, 0x080000, CRC(b80ddd3c) SHA1(55c700ce71ffdee392e03fd9d4719542c3527132) )

	ROM_REGION( 0x800000, "tiles",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0100.rom",    0x180000, 0x400000, CRC(61425e1e) SHA1(20753b86fc12003cfd763d903f034dbba8010b32) )

	ROM_REGION( 0x1800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0100.rom",    0x0000000, 0x400000, CRC(8b3bd88a) SHA1(42db3a60c6ba9d83ebe2008c8047d094027f65a7) )
	ROM_LOAD( "a0101.rom",    0x0400000, 0x400000, CRC(3b9e9644) SHA1(5b95ec1d25c3bc3504c93547f5adb5ce24376405) )
	ROM_LOAD( "a0102.rom",    0x0800000, 0x400000, CRC(069e2c38) SHA1(9bddca8c2f5bd80f4abe4e1f062751736dc151dd) )
	ROM_LOAD( "a0103.rom",    0x0c00000, 0x400000, CRC(4460a3fd) SHA1(cbebdb65c17605853f7d0b298018dd8801a25a58) )
	ROM_LOAD( "a0104.rom",    0x1000000, 0x400000, CRC(5f8abb56) SHA1(6c1ddc0309862a141aa0c0f63b641aec9257aaee) )
	ROM_LOAD( "a0105.rom",    0x1400000, 0x400000, CRC(a17a7147) SHA1(44eeb43c6b0ebb829559a20ae357383fbdeecd82) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0100.rom",    0x0000000, 0x400000, CRC(69d2e48c) SHA1(5b5f759007264c07b3b39be8e03a713698e1fc2a) )
	ROM_LOAD( "b0101.rom",    0x0400000, 0x400000, CRC(0d587bf3) SHA1(5347828b0a6e4ddd7a263663d2c2604407e4d49c) )
	ROM_LOAD( "b0102.rom",    0x0800000, 0x400000, CRC(43823c1e) SHA1(e10a1a9a81b51b11044934ff702e35d8d7ab1b08) )

	ROM_REGION( 0x600000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0100.rom",    0x400000, 0x200000, CRC(e5c36c83) SHA1(50c6f66770e8faa3df349f7d68c407a7ad021716) )
ROM_END

/*

Oriental Legend / Xi You Shi E Zhuan (KOREA 105 Ver.)
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

ROM_START( orlegend105k )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "olv105ko.u6",     0x100001, 0x080000, CRC(b86703fe) SHA1(a3529b45efd400ecd5e76f764b528ebce46e24ab) )
	ROM_LOAD16_BYTE( "olv105ko.u9",     0x100000, 0x080000, CRC(5a108e39) SHA1(2033f4fe3f2dfd725dac535324f58348b9ac3914) )
	ROM_LOAD16_BYTE( "olv105ko.u7",     0x200001, 0x080000, CRC(5712facc) SHA1(2d95ebd1703874e89ac3a206f8c1f0ece6e833e0) )
	ROM_LOAD16_BYTE( "olv105ko.u11",    0x200000, 0x080000, CRC(40ae4d9e) SHA1(62d7a96438b7fe93f74753333f50e077d417971e) )

	ROM_REGION( 0x800000, "tiles",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0100.rom",    0x180000, 0x400000, CRC(61425e1e) SHA1(20753b86fc12003cfd763d903f034dbba8010b32) )

	ROM_REGION( 0x1800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0100.rom",    0x0000000, 0x400000, CRC(8b3bd88a) SHA1(42db3a60c6ba9d83ebe2008c8047d094027f65a7) )
	ROM_LOAD( "a0101.rom",    0x0400000, 0x400000, CRC(3b9e9644) SHA1(5b95ec1d25c3bc3504c93547f5adb5ce24376405) )
	ROM_LOAD( "a0102.rom",    0x0800000, 0x400000, CRC(069e2c38) SHA1(9bddca8c2f5bd80f4abe4e1f062751736dc151dd) )
	ROM_LOAD( "a0103.rom",    0x0c00000, 0x400000, CRC(4460a3fd) SHA1(cbebdb65c17605853f7d0b298018dd8801a25a58) )
	ROM_LOAD( "a0104.rom",    0x1000000, 0x400000, CRC(5f8abb56) SHA1(6c1ddc0309862a141aa0c0f63b641aec9257aaee) )
	ROM_LOAD( "a0105.rom",    0x1400000, 0x400000, CRC(a17a7147) SHA1(44eeb43c6b0ebb829559a20ae357383fbdeecd82) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0100.rom",    0x0000000, 0x400000, CRC(69d2e48c) SHA1(5b5f759007264c07b3b39be8e03a713698e1fc2a) )
	ROM_LOAD( "b0101.rom",    0x0400000, 0x400000, CRC(0d587bf3) SHA1(5347828b0a6e4ddd7a263663d2c2604407e4d49c) )
	ROM_LOAD( "b0102.rom",    0x0800000, 0x400000, CRC(43823c1e) SHA1(e10a1a9a81b51b11044934ff702e35d8d7ab1b08) )

	ROM_REGION( 0x600000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
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
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v-110x.u2",    0x100000, 0x080000, CRC(1978106b) SHA1(af8a13d7783b755a58762c98bdc32cab845b2251) )

	ROM_REGION( 0x800000, "tiles",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgmt0200.u7",    0x180000, 0x400000, CRC(b0f6534d) SHA1(174cacd81169a0e0d14790ac06d03caed737e05d) )

	ROM_REGION( 0x400000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgma0200.u5",    0x0000000, 0x400000, CRC(13b95069) SHA1(4888b06002afb18eab81c010e9362629045767af) )

	ROM_REGION( 0x400000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgmb0200.u9",    0x0000000, 0x400000, CRC(932d0f13) SHA1(4b8e008f9c617cb2b95effeb81abc065b30e5c86) )

	ROM_REGION( 0x400000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
ROM_END

ROM_START( dw2v100x )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "dragonv100x.bin",    0x100000, 0x080000,  CRC(5e71851d) SHA1(62052469f69daec88efd26652c1b893d6f981912) )

	ROM_REGION( 0x800000, "tiles",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgmt0200.u7",    0x180000, 0x400000, CRC(b0f6534d) SHA1(174cacd81169a0e0d14790ac06d03caed737e05d) )

	ROM_REGION( 0x400000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgma0200.u5",    0x0000000, 0x400000, CRC(13b95069) SHA1(4888b06002afb18eab81c010e9362629045767af) )

	ROM_REGION( 0x400000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgmb0200.u9",    0x0000000, 0x400000, CRC(932d0f13) SHA1(4b8e008f9c617cb2b95effeb81abc065b30e5c86) )

	ROM_REGION( 0x400000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
ROM_END

ROM_START( drgw2c )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v-100c.u2",    0x100000, 0x080000, CRC(67467981) SHA1(58af01a3871b6179fe42ff471cc39a2161940043) )

	ROM_REGION( 0x800000, "tiles",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgmt0200.u7",    0x180000, 0x400000, CRC(b0f6534d) SHA1(174cacd81169a0e0d14790ac06d03caed737e05d) )

	ROM_REGION( 0x400000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgma0200.u5",    0x0000000, 0x400000, CRC(13b95069) SHA1(4888b06002afb18eab81c010e9362629045767af) )

	ROM_REGION( 0x400000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgmb0200.u9",    0x0000000, 0x400000, CRC(932d0f13) SHA1(4b8e008f9c617cb2b95effeb81abc065b30e5c86) )

	ROM_REGION( 0x400000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
ROM_END

ROM_START( drgw2j )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v-100j.u2",    0x100000, 0x080000, CRC(f8f8393e) SHA1(ef0db668b4e4f661d4c1e95d57afe881bcdf13cc) )

	ROM_REGION( 0x800000, "tiles",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "pgmt0200.u7",    0x180000, 0x400000, CRC(b0f6534d) SHA1(174cacd81169a0e0d14790ac06d03caed737e05d) )

	ROM_REGION( 0x400000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "pgma0200.u5",    0x0000000, 0x400000, CRC(13b95069) SHA1(4888b06002afb18eab81c010e9362629045767af) )

	ROM_REGION( 0x400000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "pgmb0200.u9",    0x0000000, 0x400000, CRC(932d0f13) SHA1(4b8e008f9c617cb2b95effeb81abc065b30e5c86) )

	ROM_REGION( 0x400000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
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

ROM_START( drgw3 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "dw3_v106.u12",     0x100001, 0x080000,  CRC(c3f6838b) SHA1(c135b1d4dd62af308139d40d03c29be7508fb1e7) )
	ROM_LOAD16_BYTE( "dw3_v106.u13",     0x100000, 0x080000,  CRC(28284e22) SHA1(4643a69881ddb7383ca10f3eb2aa2cf41be39e9f) )

	ROM_REGION( 0x010000, "igs022data", 0 ) /* Protection Data - is it correct for this set? */
	ROM_LOAD16_WORD_SWAP( "dw3_v100.u15", 0x000000, 0x010000, CRC(03dc4fdf) SHA1(b329b04325d4f725231b1bb7862eedef2319b652) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "dw3t0400.u18",   0x180000, 0x400000, CRC(b70f3357) SHA1(8733969d7d21f540f295a9f747a4bb8f0d325cf0) )

	ROM_REGION( 0x1800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "dw3a0400.u9",     0x0000000, 0x400000, CRC(dd7bfd40) SHA1(fb7ec5bf89a413c5208716083762a725ff63f5db) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "dw3a0401.u10",    0x0400000, 0x400000, CRC(cab6557f) SHA1(1904dd86645eea27ac1ab8a2462b20f6531356f8) ) // FIXED BITS (xxxxxxxx1xxxxxxx)

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "dw3b0400.u13",    0x0000000, 0x400000,  CRC(4bb87cc0) SHA1(71b2dc43fd11f7a6dffaba501e4e344b843583d8) ) // FIXED BITS (xxxxxxxx1xxxxxxx)

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "dw3m0400.u1",  0x400000, 0x400000, CRC(031eb9ce) SHA1(0673ec194732becc6648c2ae1396e894aa269f9a) )
ROM_END


ROM_START( drgw3105 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "dw3_v105.u12",     0x100001, 0x080000,  CRC(c5e24318) SHA1(c6954495bbc72c3985df75aecf6afd6826c8e30e) )
	ROM_LOAD16_BYTE( "dw3_v105.u13",     0x100000, 0x080000,  CRC(8d6c9d39) SHA1(cb79303ab551e91f07e11414db4254d5b161d415) )

	ROM_REGION( 0x010000, "igs022data", 0 ) /* Protection Data - is it correct for this set? */
	ROM_LOAD16_WORD_SWAP( "dw3_v100.u15", 0x000000, 0x010000, CRC(03dc4fdf) SHA1(b329b04325d4f725231b1bb7862eedef2319b652) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "dw3t0400.u18",   0x180000, 0x400000, CRC(b70f3357) SHA1(8733969d7d21f540f295a9f747a4bb8f0d325cf0) )

	ROM_REGION( 0x1800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "dw3a0400.u9",     0x0000000, 0x400000, CRC(dd7bfd40) SHA1(fb7ec5bf89a413c5208716083762a725ff63f5db) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "dw3a0401.u10",    0x0400000, 0x400000, CRC(cab6557f) SHA1(1904dd86645eea27ac1ab8a2462b20f6531356f8) ) // FIXED BITS (xxxxxxxx1xxxxxxx)

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "dw3b0400.u13",    0x0000000, 0x400000,  CRC(4bb87cc0) SHA1(71b2dc43fd11f7a6dffaba501e4e344b843583d8) ) // FIXED BITS (xxxxxxxx1xxxxxxx)

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "dw3m0400.u1",  0x400000, 0x400000, CRC(031eb9ce) SHA1(0673ec194732becc6648c2ae1396e894aa269f9a) )
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

ROM_START( drgw3100 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "dw3_v100.u12",     0x100001, 0x080000,  CRC(47243906) SHA1(9cd46e3cba97f049bcb238ceb6edf27a760ef831) )
	ROM_LOAD16_BYTE( "dw3_v100.u13",     0x100000, 0x080000,  CRC(b7cded21) SHA1(c1ae2af2e42227503c81bbcd2bd6862aa416bd78) )

	ROM_REGION( 0x010000, "igs022data", 0 ) /* Protection Data */
	ROM_LOAD16_WORD_SWAP( "dw3_v100.u15", 0x000000, 0x010000, CRC(03dc4fdf) SHA1(b329b04325d4f725231b1bb7862eedef2319b652) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "dw3t0400.u18",   0x180000, 0x400000, CRC(b70f3357) SHA1(8733969d7d21f540f295a9f747a4bb8f0d325cf0) )

	ROM_REGION( 0x1800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "dw3a0400.u9",     0x0000000, 0x400000, CRC(dd7bfd40) SHA1(fb7ec5bf89a413c5208716083762a725ff63f5db) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "dw3a0401.u10",    0x0400000, 0x400000, CRC(cab6557f) SHA1(1904dd86645eea27ac1ab8a2462b20f6531356f8) ) // FIXED BITS (xxxxxxxx1xxxxxxx)

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "dw3b0400.u13",    0x0000000, 0x400000,  CRC(4bb87cc0) SHA1(71b2dc43fd11f7a6dffaba501e4e344b843583d8) ) // FIXED BITS (xxxxxxxx1xxxxxxx)

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "dw3m0400.u1",  0x400000, 0x400000, CRC(031eb9ce) SHA1(0673ec194732becc6648c2ae1396e894aa269f9a) )
ROM_END

/*

Dragon World EX IGS PGM cart

Top board
---------
PCB Number: IGS PCB-0189-1
8MHz XTAL
2x 27C040 EPROMs at U12, U13
27C512 EPROM at U15
PAL at U17
2x 6164 SRAM at U1, U4
MACH211 at U11
IGS022 at U14
IGS025 at U16
16M SOP44 mask ROM at U18

Bottom Board
------------
PCB Number: IGS PCB-0178
2x 16M mask ROMs at U1, U10
2x 32M mask ROMs at U9, U13

*/

// seems to be an updated version of dw3, most roms are the same, but it's a sequel, not a clone.
// the non-program roms that differ are actually the same, but in the dw3 sets they're double sized with duplicate data (overdumped, or just different roms used on pcb?)
ROM_START( dwex )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "ex_v100.u12",     0x100001, 0x080000, CRC(bc171799) SHA1(142329dffbca199f3e748a52146a03e27b36db6a) )
	ROM_LOAD16_BYTE( "ex_v100.u13",     0x100000, 0x080000, CRC(7afe6322) SHA1(a52d71af1d6de16c5a3df23eacdab3466693ba8d) )

	ROM_REGION( 0x010000, "igs022data", 0 ) /* Protection Data */
	ROM_LOAD16_WORD_SWAP( "ex_data.u15", 0x000000, 0x010000, CRC(03dc4fdf) SHA1(b329b04325d4f725231b1bb7862eedef2319b652) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "ex_t0400.u18", 0x180000, 0x200000, CRC(9ecc950d) SHA1(fd97f43818a3eb18254636166871fa09bd0d6c07) )

	ROM_REGION( 0x1800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "ex_a0400.u9",     0x0000000, 0x400000, CRC(dd7bfd40) SHA1(fb7ec5bf89a413c5208716083762a725ff63f5db) )
	ROM_LOAD( "ex_a0401.u10",    0x0400000, 0x200000, CRC(d36c06a4) SHA1(f192e8bfdfbe3d82a49d8f0d3cb0603e39719773) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "ex_b0400.u13",    0x0000000, 0x400000,  CRC(4bb87cc0) SHA1(71b2dc43fd11f7a6dffaba501e4e344b843583d8) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "ex_m0400.u1",  0x400000, 0x200000, CRC(42d54fd5) SHA1(ad915b514aa6cae6f72dea78e6208f40b08ceac0) )
ROM_END




ROM_START( kov )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "p0600.117",    0x100000, 0x400000, CRC(c4d19fe6) SHA1(14ef31539bfbc665e76c9703ee01b12228344052) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kov_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0600.rom",    0x180000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	ROM_REGION( 0x1c00000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0600.rom",    0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0601.rom",    0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0602.rom",    0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0603.rom",    0x1800000, 0x0400000, CRC(ec31abda) SHA1(ee526655369bae63b0ef0730e9768b765c9950fc) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0600.rom",    0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "b0601.rom",    0x0800000, 0x0400000, CRC(a0bb1c2f) SHA1(0542348c6e27779e0a98de16f04f9c18158f2b28) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0600.rom",    0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END

ROM_START( kov115 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "p0600.115",    0x100000, 0x400000, CRC(527a2924) SHA1(7e3b166dddc5245d7b408e78437c16fd2986d1d9) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kov_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0600.rom",    0x180000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	ROM_REGION( 0x1c00000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0600.rom",    0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0601.rom",    0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0602.rom",    0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0603.rom",    0x1800000, 0x0400000, CRC(ec31abda) SHA1(ee526655369bae63b0ef0730e9768b765c9950fc) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0600.rom",    0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "b0601.rom",    0x0800000, 0x0400000, CRC(a0bb1c2f) SHA1(0542348c6e27779e0a98de16f04f9c18158f2b28) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
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

ROM_START( kov100 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "sav111.u4",      0x100001, 0x080000, CRC(ae2f1b4e) SHA1(2ac9d84f5dee52f374941cfd68e2b98ecad436a8) )
	ROM_LOAD16_BYTE( "sav111.u7",      0x100000, 0x080000, CRC(95eedf0e) SHA1(582a54e9a1eda7ff73e20f0e69d2d50141772378) )
	ROM_LOAD16_BYTE( "sav111.u5",      0x200001, 0x080000, CRC(5fdd4aa8) SHA1(43c96e21ad4f11148e1e94a59c53780b2edd43ba) )
	ROM_LOAD16_BYTE( "sav111.u8",      0x200000, 0x080000, CRC(003cbf49) SHA1(fb5bea47ecae025b1b425af52cd05e061f45e377) )
	ROM_LOAD16_WORD_SWAP( "sav111.u10",0x300000, 0x080000, CRC(d5536107) SHA1(f963e015d99c1621323eecf63e773c0b9f4b6a43) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kov_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0600.rom",    0x180000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	ROM_REGION( 0x1c00000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0600.rom",    0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0601.rom",    0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0602.rom",    0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0603.rom",    0x1800000, 0x0400000, CRC(ec31abda) SHA1(ee526655369bae63b0ef0730e9768b765c9950fc) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0600.rom",    0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "b0601.rom",    0x0800000, 0x0400000, CRC(a0bb1c2f) SHA1(0542348c6e27779e0a98de16f04f9c18158f2b28) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0600.rom",    0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END

ROM_START( kovplus )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "p0600.119",    0x100000, 0x400000, CRC(e4b0875d) SHA1(e8382e131b0e431406dc2a05cc1ef128302d987c) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kov_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0600.rom",    0x180000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	ROM_REGION( 0x1c00000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0600.rom",    0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0601.rom",    0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0602.rom",    0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0603.rom",    0x1800000, 0x0400000, CRC(ec31abda) SHA1(ee526655369bae63b0ef0730e9768b765c9950fc) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0600.rom",    0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "b0601.rom",    0x0800000, 0x0400000, CRC(a0bb1c2f) SHA1(0542348c6e27779e0a98de16f04f9c18158f2b28) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
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
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "v119.u3",     0x100001, 0x080000, CRC(6750388f) SHA1(869f4ad27f2992cc62baa9a78bf7984a43ec4cc5) )
	ROM_LOAD16_BYTE( "v119.u5",     0x100000, 0x080000, CRC(d4101ffd) SHA1(a327fd56eec65b07df9305cd93ef2c46bf8e40f3) )
	ROM_LOAD16_BYTE( "v119.u4",     0x200001, 0x080000, CRC(8200ece6) SHA1(97081d2e8aed2ac6fbe5951890aecea18af5ce2e) )
	ROM_LOAD16_BYTE( "v119.u6",     0x200000, 0x080000, CRC(71e28f27) SHA1(db382807e9185f0dc17124f210165fa1b36ca6ac) )
	ROM_LOAD16_WORD_SWAP( "v119.u2",0x300000, 0x080000, CRC(29588ef2) SHA1(17d1a308d44434cf65224a24360cf4b6e32d28f3) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kov_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0600.rom",    0x180000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	ROM_REGION( 0x1c00000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0600.rom",    0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0601.rom",    0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0602.rom",    0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0603.rom",    0x1800000, 0x0400000, CRC(ec31abda) SHA1(ee526655369bae63b0ef0730e9768b765c9950fc) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0600.rom",    0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "b0601.rom",    0x0800000, 0x0400000, CRC(a0bb1c2f) SHA1(0542348c6e27779e0a98de16f04f9c18158f2b28) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0600.rom",    0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END


ROM_START( kovsgqyz )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "pgm_sgqyz.rom",    0x100000, 0x400000, CRC(18e1eed9) SHA1(db18d9121bb533140957e9c58dbc38211d164b01) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kovsgqyz_igs027a.bin", 0x000000, 0x04000, NO_DUMP ) // bootleg is probably a different device

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0600.rom",    0x180000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	ROM_REGION( 0x2000000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0600.rom",    0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0601.rom",    0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0602.rom",    0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "qyza0603.rom", 0x1800000, 0x0800000, CRC(c8b92220) SHA1(4f9c43970d92ac8a8f1563021022797ae8e32012) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0600.rom",    0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "qyzb0601.rom", 0x0800000, 0x0800000, CRC(64f55882) SHA1(ab9ac1396587c3d78d06f6ec83cab61d6a9faacd) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0600.rom",    0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END

ROM_START( kovsgqyza )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "pgm_sgqyza.rom",    0x100000, 0x400000, CRC(5a30dcb7) SHA1(64f34faf99a19c0a54899990695129c512d5a3c8) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kovsgqyza_igs027a.bin", 0x000000, 0x04000, NO_DUMP ) // bootleg is probably a different device

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0600.rom",    0x180000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	ROM_REGION( 0x2000000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0600.rom",    0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0601.rom",    0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0602.rom",    0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "qyza0603.rom", 0x1800000, 0x0800000, CRC(c8b92220) SHA1(4f9c43970d92ac8a8f1563021022797ae8e32012) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0600.rom",    0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "qyzb0601.rom", 0x0800000, 0x0800000, CRC(64f55882) SHA1(ab9ac1396587c3d78d06f6ec83cab61d6a9faacd) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0600.rom",    0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END

ROM_START( kovsgqyzb )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "pgm_sgqyzb.rom",    0x100000, 0x400000, CRC(18b8b9c0) SHA1(f4937aa21cd11af16fb50e7a75c8d4c4ed27c5cf) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kovsgqyzb_igs027a.bin", 0x000000, 0x04000, NO_DUMP ) // bootleg is probably a different device

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0600.rom",    0x180000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	ROM_REGION( 0x2000000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0600.rom",    0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0601.rom",    0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0602.rom",    0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "qyza0603.rom", 0x1800000, 0x0800000, CRC(c8b92220) SHA1(4f9c43970d92ac8a8f1563021022797ae8e32012) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0600.rom",    0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "qyzb0601.rom", 0x0800000, 0x0800000, CRC(64f55882) SHA1(ab9ac1396587c3d78d06f6ec83cab61d6a9faacd) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0600.rom",    0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END




ROM_START( kovsh )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "p0600.322",    0x100000, 0x400000, CRC(7c78e5f3) SHA1(9b1e4bd63fb1294ebeb539966842273c8dc7683b) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kovsh_v100_china.asic", 0x000000, 0x04000,  CRC(0f09a5c1) SHA1(621b38c05f33277608d58b49822aebc930ae4870) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0600.rom",    0x180000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	/* all roms below need checking to see if they're the same on this board */
	ROM_REGION( 0x1e00000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0600.rom",    0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0601.rom",    0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0602.rom",    0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0603.rom",    0x1800000, 0x0400000, CRC(ec31abda) SHA1(ee526655369bae63b0ef0730e9768b765c9950fc) )
	ROM_LOAD( "a0604.rom",    0x1a00000, 0x0400000, CRC(26b59fd3) SHA1(53219376056f4766dc5236735599d982ceb56b84) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0600.rom",    0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "b0601.rom",    0x0800000, 0x0400000, CRC(a0bb1c2f) SHA1(0542348c6e27779e0a98de16f04f9c18158f2b28) )
	ROM_LOAD( "b0602.rom",    0x0c00000, 0x0100000, CRC(9df77934) SHA1(99a3fe337c13702c9aa2373bcd1bb1befd0e2a13) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0600.rom",    0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END


ROM_START( kovsh103 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "p0600.103",    0x100000, 0x400000, CRC(f0b3da82) SHA1(4067beb69c049b51bce6154f4cf880600ca4de11) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kovsh_v100_china.asic", 0x000000, 0x04000,  CRC(0f09a5c1) SHA1(621b38c05f33277608d58b49822aebc930ae4870) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0600.rom",    0x180000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	/* all roms below need checking to see if they're the same on this board */
	ROM_REGION( 0x1e00000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0600.rom",    0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0601.rom",    0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0602.rom",    0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0603.rom",    0x1800000, 0x0400000, CRC(ec31abda) SHA1(ee526655369bae63b0ef0730e9768b765c9950fc) )
	ROM_LOAD( "a0604.rom",    0x1a00000, 0x0400000, CRC(26b59fd3) SHA1(53219376056f4766dc5236735599d982ceb56b84) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0600.rom",    0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "b0601.rom",    0x0800000, 0x0400000, CRC(a0bb1c2f) SHA1(0542348c6e27779e0a98de16f04f9c18158f2b28) )
	ROM_LOAD( "b0602.rom",    0x0c00000, 0x0100000, CRC(9df77934) SHA1(99a3fe337c13702c9aa2373bcd1bb1befd0e2a13) ) // this dump had the same rom 4x bigger but with the data duplicated 4x, which is correct?

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0600.rom",    0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END


/* kovsh bootlegs, these still have the ARM! */
ROM_START( kovlsqh2 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "lsqh2_prg.rom",0x100000, 0x400000, CRC(d71e3d50) SHA1(bda78648bc176b0ded74118a8e340ee661bb930d) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "qhsg_prot.c51", 0x000000, 0x04000, BAD_DUMP CRC(0f09a5c1) SHA1(621b38c05f33277608d58b49822aebc930ae4870) )

	ROM_REGION( 0xc00000, "tiles",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "lsqh2_t01.rom",0x180000, 0x800000, CRC(d498d97f) SHA1(97a7b6d2ed1170449e7c2899448af7cbbca4c94f) )
	ROM_IGNORE( 0x800000 )	// second half identical

	ROM_REGION( 0x3000000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "lsqh2_a01.rom", 0x0000000, 0x1000000, CRC(25ae3efd) SHA1(083d977602ddb5ad54fbdcba000cd4287de8d463) )
	ROM_LOAD( "lsqh2_a23.rom", 0x1000000, 0x1000000, CRC(7a779112) SHA1(0a7d36b3715063d8eac629b95a9bb3ecd8e54fca) )
	ROM_LOAD( "lsqh2_a45.rom", 0x2000000, 0x1000000, CRC(5d7de052) SHA1(7663b6cf09f65c4644661005a38f9aba84a32913) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "lsqh2_b01.rom", 0x0000000, 0x1000000, CRC(df7ca696) SHA1(7af3d27957a39de7e4873867c9972c05af7e7964) )

	ROM_REGION( 0xc00000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "lsqh2_m01.rom",0x400000, 0x400000, CRC(01af1b65) SHA1(6cf523fa8f1e03f974771611bb9a4e08a4d4443f) )
	ROM_IGNORE( 0x400000 )	// 400000-7fffff empty
	ROM_CONTINUE( 0x800000, 0x400000 )
	ROM_IGNORE( 0x400000 )	// c00000-ffffff empty
ROM_END

ROM_START( kovlsjb )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "lsjb_prg.rom", 0x100000, 0x400000, CRC(adf06b37) SHA1(be3c0af64de374046d28492ac49c01da1ec78e40) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "qhsg_prot.c51", 0x000000, 0x04000, BAD_DUMP CRC(0f09a5c1) SHA1(621b38c05f33277608d58b49822aebc930ae4870) )

	ROM_REGION( 0xc00000, "tiles",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "lsqh2_t01.rom",0x180000, 0x800000, CRC(d498d97f) SHA1(97a7b6d2ed1170449e7c2899448af7cbbca4c94f) )
	ROM_IGNORE( 0x800000 )	// second half identical

	ROM_REGION( 0x3000000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "lsqh2_a01.rom", 0x0000000, 0x1000000, CRC(25ae3efd) SHA1(083d977602ddb5ad54fbdcba000cd4287de8d463) )
	ROM_LOAD( "lsqh2_a23.rom", 0x1000000, 0x1000000, CRC(7a779112) SHA1(0a7d36b3715063d8eac629b95a9bb3ecd8e54fca) )
	ROM_LOAD( "lsqh2_a45.rom", 0x2000000, 0x1000000, CRC(5d7de052) SHA1(7663b6cf09f65c4644661005a38f9aba84a32913) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "lsqh2_b01.rom", 0x0000000, 0x1000000, CRC(df7ca696) SHA1(7af3d27957a39de7e4873867c9972c05af7e7964) )

	ROM_REGION( 0xc00000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "lsqh2_m01.rom",0x400000, 0x400000, CRC(01af1b65) SHA1(6cf523fa8f1e03f974771611bb9a4e08a4d4443f) )
	ROM_IGNORE( 0x400000 )	// 400000-7fffff empty
	ROM_CONTINUE( 0x800000, 0x400000 )
	ROM_IGNORE( 0x400000 )	// c00000-ffffff empty
ROM_END

ROM_START( kovlsjba )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "lsjba_prg.rom",0x100000, 0x400000, CRC(8b42f456) SHA1(48796e48f6f1a5f68442cf15a6b195095d443a35) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "qhsg_prot.c51", 0x000000, 0x04000, BAD_DUMP CRC(0f09a5c1) SHA1(621b38c05f33277608d58b49822aebc930ae4870) )

	ROM_REGION( 0xc00000, "tiles",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "lsqh2_t01.rom",0x180000, 0x800000, CRC(d498d97f) SHA1(97a7b6d2ed1170449e7c2899448af7cbbca4c94f) )
	ROM_IGNORE( 0x800000 )	// second half identical

	ROM_REGION( 0x3000000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "lsqh2_a01.rom", 0x0000000, 0x1000000, CRC(25ae3efd) SHA1(083d977602ddb5ad54fbdcba000cd4287de8d463) )
	ROM_LOAD( "lsqh2_a23.rom", 0x1000000, 0x1000000, CRC(7a779112) SHA1(0a7d36b3715063d8eac629b95a9bb3ecd8e54fca) )
	ROM_LOAD( "lsqh2_a45.rom", 0x2000000, 0x1000000, CRC(5d7de052) SHA1(7663b6cf09f65c4644661005a38f9aba84a32913) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "lsqh2_b01.rom", 0x0000000, 0x1000000, CRC(df7ca696) SHA1(7af3d27957a39de7e4873867c9972c05af7e7964) )

	ROM_REGION( 0xc00000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "lsqh2_m01.rom",0x400000, 0x400000, CRC(01af1b65) SHA1(6cf523fa8f1e03f974771611bb9a4e08a4d4443f) )
	ROM_IGNORE( 0x400000 )	// 400000-7fffff empty
	ROM_CONTINUE( 0x800000, 0x400000 )
	ROM_IGNORE( 0x400000 )	// c00000-ffffff empty
ROM_END

ROM_START( kovqhsgs )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "qhsg_c51.rom", 0x100000, 0x400000, CRC(e5cbac85) SHA1(4b424206387057863990b04f6d5bd0b6f754814f) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "qhsg_prot.c51", 0x000000, 0x04000, BAD_DUMP CRC(0f09a5c1) SHA1(621b38c05f33277608d58b49822aebc930ae4870) )

	ROM_REGION( 0xc00000, "tiles",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "lsqh2_t01.rom",0x180000, 0x800000, CRC(d498d97f) SHA1(97a7b6d2ed1170449e7c2899448af7cbbca4c94f) )
	ROM_IGNORE( 0x800000 )	// second half identical

	ROM_REGION( 0x3000000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "lsqh2_a01.rom", 0x0000000, 0x1000000, CRC(25ae3efd) SHA1(083d977602ddb5ad54fbdcba000cd4287de8d463) )
	ROM_LOAD( "lsqh2_a23.rom", 0x1000000, 0x1000000, CRC(7a779112) SHA1(0a7d36b3715063d8eac629b95a9bb3ecd8e54fca) )
	ROM_LOAD( "lsqh2_a45.rom", 0x2000000, 0x1000000, CRC(5d7de052) SHA1(7663b6cf09f65c4644661005a38f9aba84a32913) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "lsqh2_b01.rom", 0x0000000, 0x1000000, CRC(df7ca696) SHA1(7af3d27957a39de7e4873867c9972c05af7e7964) )

	ROM_REGION( 0xc00000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "lsqh2_m01.rom",0x400000, 0x400000, CRC(01af1b65) SHA1(6cf523fa8f1e03f974771611bb9a4e08a4d4443f) )
	ROM_IGNORE( 0x400000 )	// 400000-7fffff empty
	ROM_CONTINUE( 0x800000, 0x400000 )
	ROM_IGNORE( 0x400000 )	// c00000-ffffff empty
ROM_END


ROM_START( kovlsqh )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "lsqh_v200cn.rom", 0x100000, 0x400000, CRC(9935a27a) SHA1(3075935293172466c4bd997dcb67f864ae26493e) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "qhsg_prot.c51", 0x000000, 0x04000, BAD_DUMP CRC(0f09a5c1) SHA1(621b38c05f33277608d58b49822aebc930ae4870) )

	ROM_REGION( 0xc00000, "tiles",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "lsqh2_t01.rom",0x180000, 0x800000, CRC(d498d97f) SHA1(97a7b6d2ed1170449e7c2899448af7cbbca4c94f) )
	ROM_IGNORE( 0x800000 )	// second half identical

	ROM_REGION( 0x3000000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "lsqh2_a01.rom", 0x0000000, 0x1000000, CRC(25ae3efd) SHA1(083d977602ddb5ad54fbdcba000cd4287de8d463) )
	ROM_LOAD( "lsqh2_a23.rom", 0x1000000, 0x1000000, CRC(7a779112) SHA1(0a7d36b3715063d8eac629b95a9bb3ecd8e54fca) )
	ROM_LOAD( "lsqh2_a45.rom", 0x2000000, 0x1000000, CRC(5d7de052) SHA1(7663b6cf09f65c4644661005a38f9aba84a32913) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "lsqh2_b01.rom", 0x0000000, 0x1000000, CRC(df7ca696) SHA1(7af3d27957a39de7e4873867c9972c05af7e7964) )

	ROM_REGION( 0xc00000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "lsqh2_m01.rom",0x400000, 0x400000, CRC(01af1b65) SHA1(6cf523fa8f1e03f974771611bb9a4e08a4d4443f) )
	ROM_IGNORE( 0x400000 )	// 400000-7fffff empty
	ROM_CONTINUE( 0x800000, 0x400000 )
	ROM_IGNORE( 0x400000 )	// c00000-ffffff empty
ROM_END

/*

p0701_v105.u2

IGS
PGM P0701 V105
1B2687LC
C994746

*/

ROM_START( photoy2k )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "p0701_v105.u2",     0x100000, 0x200000, CRC(fab142e0) SHA1(8dc7e53b740ed68bac98c0ef7ca4943c517e6f5d) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "igs027a_photoy2k_v100_china.asic", 0x000000, 0x04000, CRC(1a0b68f6) SHA1(290441ed652f54b26ace8f59a26220881fb62084) ) // 3 bytes differ from the read in the other sets.  I think this one is GOOD and the other is bad.  This always gives the same read, so unless the actual chips is bad... TBC

	ROM_REGION( 0x480000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0700.rom",    0x180000, 0x080000, CRC(93943b4d) SHA1(3b439903853727d45d62c781af6073024eb3c5a3) )

	ROM_REGION( 0x1080000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0700.l",    0x0000000, 0x0400000, CRC(26a9ae9c) SHA1(c977c89db6fdf47ee260ff687b80375caeab975c) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0700.h",    0x0400000, 0x0400000, CRC(79bc1fc1) SHA1(a09472a9b75704c1d31ab828f92c2a5007b2b4ed) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0701.l",    0x0800000, 0x0400000, CRC(23607f81) SHA1(8b6dbcdce9b131370693847ed9771aa04b62711c) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0701.h",    0x0c00000, 0x0400000, CRC(5f2efd37) SHA1(9a5bd9751691bc085b0751b9fa8ede9eb97b1248) )
	ROM_LOAD( "a0702.rom",  0x1000000, 0x0080000, CRC(42239e1b) SHA1(2b6d20958abf8a67ce525d5c8964b6d225ccaeda) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0700.l",    0x0000000, 0x0400000, CRC(af096904) SHA1(8e86b36cc44720ece68022e409279bf9144341ba) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "b0700.h",    0x0400000, 0x0400000, CRC(6d53de26) SHA1(f3f93fd2f87adb815834ba0242b94073fbb5e333) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "cgv101.rom", 0x0800000, 0x0020000, CRC(da02ec3e) SHA1(7ee21d748c9b932f53e790a9040167f904fecefc) )

	ROM_REGION( 0x480000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0700.rom",    0x400000, 0x080000, CRC(acc7afce) SHA1(ac2d344ebac336f0f363bb045dd8ea4e83d1fb50) )
ROM_END


ROM_START( photoy2k104 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v104.16m",     0x100000, 0x200000, CRC(e051070f) SHA1(a5a1a8dd7542a30632501af8d02fda07475fd9aa) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "igs027a_photoy2k_v100_china_alt.asic", 0x000000, 0x04000,  CRC(6dd7f257) SHA1(1984f98a282d8b3264674f231c3b7def1757cf72) )

	ROM_REGION( 0x480000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0700.rom",    0x180000, 0x080000, CRC(93943b4d) SHA1(3b439903853727d45d62c781af6073024eb3c5a3) )

	ROM_REGION( 0x1080000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0700.l",    0x0000000, 0x0400000, CRC(26a9ae9c) SHA1(c977c89db6fdf47ee260ff687b80375caeab975c) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0700.h",    0x0400000, 0x0400000, CRC(79bc1fc1) SHA1(a09472a9b75704c1d31ab828f92c2a5007b2b4ed) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0701.l",    0x0800000, 0x0400000, CRC(23607f81) SHA1(8b6dbcdce9b131370693847ed9771aa04b62711c) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0701.h",    0x0c00000, 0x0400000, CRC(5f2efd37) SHA1(9a5bd9751691bc085b0751b9fa8ede9eb97b1248) )
	ROM_LOAD( "a0702.rom",  0x1000000, 0x0080000, CRC(42239e1b) SHA1(2b6d20958abf8a67ce525d5c8964b6d225ccaeda) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0700.l",    0x0000000, 0x0400000, CRC(af096904) SHA1(8e86b36cc44720ece68022e409279bf9144341ba) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "b0700.h",    0x0400000, 0x0400000, CRC(6d53de26) SHA1(f3f93fd2f87adb815834ba0242b94073fbb5e333) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "cgv101.rom", 0x0800000, 0x0020000, CRC(da02ec3e) SHA1(7ee21d748c9b932f53e790a9040167f904fecefc) )

	ROM_REGION( 0x480000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
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

ROM_START( photoy2k102 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "v102.u4",     0x100001, 0x080000, CRC(a65eda9f) SHA1(6307cacf4a262e781753eff14700a0455837780c) )
	ROM_LOAD16_BYTE( "v102.u6",     0x100000, 0x080000, CRC(b9ca5504) SHA1(058cf01316f233236ca9861349f515935283b75e) )
	ROM_LOAD16_BYTE( "v102.u5",     0x200001, 0x080000, CRC(9201621b) SHA1(1ca3ebe7eec40614bfa8b911657fa2b51f2c51a4) )
	ROM_LOAD16_BYTE( "v102.u8",     0x200000, 0x080000, CRC(3be22b8f) SHA1(03634fbd6a8a8369c6cb1fd6694a3784dac5bf59) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "igs027a_photoy2k_v100_china_alt.asic", 0x000000, 0x04000,  CRC(6dd7f257) SHA1(1984f98a282d8b3264674f231c3b7def1757cf72) )

	ROM_REGION( 0x480000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0700.rom",    0x180000, 0x080000, CRC(93943b4d) SHA1(3b439903853727d45d62c781af6073024eb3c5a3) )

	ROM_REGION( 0x1080000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0700.l",    0x0000000, 0x0400000, CRC(26a9ae9c) SHA1(c977c89db6fdf47ee260ff687b80375caeab975c) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0700.h",    0x0400000, 0x0400000, CRC(79bc1fc1) SHA1(a09472a9b75704c1d31ab828f92c2a5007b2b4ed) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0701.l",    0x0800000, 0x0400000, CRC(23607f81) SHA1(8b6dbcdce9b131370693847ed9771aa04b62711c) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0701.h",    0x0c00000, 0x0400000, CRC(5f2efd37) SHA1(9a5bd9751691bc085b0751b9fa8ede9eb97b1248) )
	ROM_LOAD( "a0702.rom",  0x1000000, 0x0080000, CRC(42239e1b) SHA1(2b6d20958abf8a67ce525d5c8964b6d225ccaeda) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0700.l",    0x0000000, 0x0400000, CRC(af096904) SHA1(8e86b36cc44720ece68022e409279bf9144341ba) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "b0700.h",    0x0400000, 0x0400000, CRC(6d53de26) SHA1(f3f93fd2f87adb815834ba0242b94073fbb5e333) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "cgv101.rom", 0x0800000, 0x0020000, CRC(da02ec3e) SHA1(7ee21d748c9b932f53e790a9040167f904fecefc) )

	ROM_REGION( 0x480000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0700.rom",    0x400000, 0x080000, CRC(acc7afce) SHA1(ac2d344ebac336f0f363bb045dd8ea4e83d1fb50) )
ROM_END




/*

Photo Y2K2 IGS PGM cart

Top board
---------
PCB Number: IGS PCB-0313-00T
27C160 EPROM at U1
PAL at U3
IGS027A at U4

Bottom Board
------------
PCB Number: IGS PCB-0314-00
1x 16M SOP44 mask ROM at U3
6x 64M SOP44 mask ROMs at U4, U5, U6, U7, U8, U9

*/

ROM_START( py2k2 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "y2k2_m-101xx.u1",     0x100000, 0x200000, CRC(c47795f1) SHA1(5be4af4275571932d7740c3ea0857a1f58a3f6d9) ) // 68k (encrypted) 2nd half empty...

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "igs027a_photoy2k2.asic", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0x480000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	/* no extra tilemap rom */

	ROM_REGION( 0x2000000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "y2k2_a1100.u6",    0x0000000, 0x0800000, CRC(e32ce499) SHA1(f84c7daa55c25a05da467b5654ebf432b7ce1754) )
	ROM_LOAD( "y2k2_a1101.u7",    0x0800000, 0x0800000, CRC(4e7568bc) SHA1(bf9cc453191bd5ec9fbcce62891809f253a44267) )
	ROM_LOAD( "y2k2_a1102.u8",    0x1000000, 0x0800000, CRC(6da7c143) SHA1(9408ba7722bfc8013f851aadea5e2819f5263129) )
	ROM_LOAD( "y2k2_a1103.u9",    0x1800000, 0x0800000, CRC(0ebebfdc) SHA1(4faad7f97c7e734f179ec934a37e75d8d6adccf4) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "y2k2_b1100.u4",    0x0000000, 0x0800000,  CRC(fa53d6f6) SHA1(c2da55f4b7e721fa1c63bd7f9528f261643164e8) )
	ROM_LOAD( "y2k2_b1101.u5",    0x0800000, 0x0800000, CRC(001e4c81) SHA1(21119055f8fd7f831529e73ff9c97bca3987a1dc))

	ROM_REGION( 0x880000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "y2k2_m1100.u3",    0x400000, 0x200000, CRC(fb1515f8) SHA1(90e5e5bfdac9a460445bf224952e4a536888dc1b) )
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



/*
 the text on the chip are
-----------------
IGS
PGM P0300 V109
1A0577Y3
J982846
-----------------
*/

ROM_START( killbld )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "p0300_v109.u9", 0x100000, 0x200000, CRC(2fcee215) SHA1(855281a9090bfdf3da9f4d50c121765131a13400) )

	ROM_REGION( 0x010000, "igs022data", 0 ) /* Protection Data */
	ROM_LOAD16_WORD_SWAP( "kb_u2.rom", 0x000000, 0x010000,  CRC(de3eae63) SHA1(03af767ef764055bda528b5cc6a24b9e1218cca8) )

	ROM_REGION( 0x800000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0300.u14",    0x180000, 0x400000, CRC(0922f7d9) SHA1(4302b4b7369e13f315fad14f7d6cad1321101d24) )

	ROM_REGION( 0x2000000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0300.u9",   0x0000000, 0x0400000,  CRC(3f9455d3) SHA1(3718ce00ad93975383aafc14e5a74dc297b011a1) )
	ROM_LOAD( "a0301.u10",  0x0400000, 0x0400000,  CRC(92776889) SHA1(6d677837fefff47bfd1c6166322f69f89989a5e2) )
	ROM_LOAD( "a0303.u11",  0x0800000, 0x0400000,  CRC(33f5cc69) SHA1(9cacd5058d4bb25b77f71658bbbbd4b38d0a6b6a) )
	ROM_LOAD( "a0306.u12",  0x0c00000, 0x0400000,  CRC(cc018a8e) SHA1(37752d46f238fb57c0ab5a4f96b1e013f2077347) )
	ROM_LOAD( "a0307.u2",   0x1000000, 0x0400000,  CRC(bc772e39) SHA1(079cc42a190cb916f02b59bca8fa90e524acefe9) )
//  ROM_LOAD( "a0302.u3",   0x1400000, 0x0200000,  CRC(a4810e38) SHA1(c31fe641feab2c93795fc35bf71d4f37af1056d4) ) // from lord of gun! unused..
//  ROM_LOAD( "a0304.u4",   0x1600000, 0x0200000,  CRC(3096de1c) SHA1(d010990d21cfda9cb8ab5b4bc0e329c23b7719f5) ) // from lord of gun! unused..
//  ROM_LOAD( "a0305.u5",   0x1800000, 0x0200000,  CRC(2234531e) SHA1(58a82e31a1c0c1a4dd026576319f4e7ecffd140e) ) // from lord of gun! unused..

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0300.u13",    0x0000000, 0x0400000, CRC(7f876981) SHA1(43555a200929ad5ecc42137fc9aeb42dc4f50d20) )
	ROM_LOAD( "b0302.u14",    0x0400000, 0x0400000, CRC(eea9c502) SHA1(04b3972c7111ea59a3cceab6ad124080c4ce3520) )
	ROM_LOAD( "b0303.u15",    0x0800000, 0x0200000, CRC(77a9652e) SHA1(2342f643d37945fbda224a5034c013796e5134ca) )
//  ROM_LOAD( "b0301.u8",     0x0a00000, 0x0200000, CRC(400abe33) SHA1(20de1eb626424ea41bd55eb3cecd6b50be744ee0) ) // from lord of gun! unused..

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0300.u1",     0x400000, 0x400000, CRC(93159695) SHA1(50c5976c9b681bd3d1ebefa3bfa9fe6e72dcb96f) )
ROM_END

ROM_START( killbld104 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "kb_u3_v104.u3",     0x100001, 0x080000, CRC(6db1d719) SHA1(804002f014d275aaf0368fb7f904938fe4ac07ee) )
	ROM_LOAD16_BYTE( "kb_u6_v104.u6",     0x100000, 0x080000, CRC(31ecc978) SHA1(82666d534e4151775063af6d39f575faba0f1047) )
	ROM_LOAD16_BYTE( "kb_u4_v104.u4",     0x200001, 0x080000, CRC(1ed8b2e7) SHA1(331c037640cfc1fe743cd0e65a1156c470b3303e) )
	ROM_LOAD16_BYTE( "kb_u5_v104.u5",     0x200000, 0x080000, CRC(a0bafc29) SHA1(b20db7c16353c6f87ed3c08c9d037b07336711f1) )

	ROM_REGION( 0x010000, "igs022data", 0 ) /* Protection Data */
	ROM_LOAD16_WORD_SWAP( "kb_u2_v104.u2", 0x000000, 0x010000,  CRC(c970f6d5) SHA1(399fc6f80262784c566363c847dc3fdc4fb37494) )

	ROM_REGION( 0x800000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0300.u14",    0x180000, 0x400000, CRC(0922f7d9) SHA1(4302b4b7369e13f315fad14f7d6cad1321101d24) )

	ROM_REGION( 0x2000000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0300.u9",   0x0000000, 0x0400000,  CRC(3f9455d3) SHA1(3718ce00ad93975383aafc14e5a74dc297b011a1) )
	ROM_LOAD( "a0301.u10",  0x0400000, 0x0400000,  CRC(92776889) SHA1(6d677837fefff47bfd1c6166322f69f89989a5e2) )
	ROM_LOAD( "a0303.u11",  0x0800000, 0x0400000,  CRC(33f5cc69) SHA1(9cacd5058d4bb25b77f71658bbbbd4b38d0a6b6a) )
	ROM_LOAD( "a0306.u12",  0x0c00000, 0x0400000,  CRC(cc018a8e) SHA1(37752d46f238fb57c0ab5a4f96b1e013f2077347) )
	ROM_LOAD( "a0307.u2",   0x1000000, 0x0400000,  CRC(bc772e39) SHA1(079cc42a190cb916f02b59bca8fa90e524acefe9) )
//  ROM_LOAD( "a0302.u3",   0x1400000, 0x0200000,  CRC(a4810e38) SHA1(c31fe641feab2c93795fc35bf71d4f37af1056d4) ) // from lord of gun! unused..
//  ROM_LOAD( "a0304.u4",   0x1600000, 0x0200000,  CRC(3096de1c) SHA1(d010990d21cfda9cb8ab5b4bc0e329c23b7719f5) ) // from lord of gun! unused..
//  ROM_LOAD( "a0305.u5",   0x1800000, 0x0200000,  CRC(2234531e) SHA1(58a82e31a1c0c1a4dd026576319f4e7ecffd140e) ) // from lord of gun! unused..

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0300.u13",    0x0000000, 0x0400000, CRC(7f876981) SHA1(43555a200929ad5ecc42137fc9aeb42dc4f50d20) )
	ROM_LOAD( "b0302.u14",    0x0400000, 0x0400000, CRC(eea9c502) SHA1(04b3972c7111ea59a3cceab6ad124080c4ce3520) )
	ROM_LOAD( "b0303.u15",    0x0800000, 0x0200000, CRC(77a9652e) SHA1(2342f643d37945fbda224a5034c013796e5134ca) )
//  ROM_LOAD( "b0301.u8",     0x0a00000, 0x0200000, CRC(400abe33) SHA1(20de1eb626424ea41bd55eb3cecd6b50be744ee0) ) // from lord of gun! unused..

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
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
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "v100mg.u1",     0x100001, 0x080000, CRC(5788b77d) SHA1(7770aae6e686da92b2623c977d1bc8f019f48267) )
	ROM_LOAD16_BYTE( "v100mg.u2",     0x100000, 0x080000, CRC(4c79d979) SHA1(3b92052a35994f2b3dd164930154184c45d5e2d0) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "puzlstar_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0800.u5",    0x180000, 0x200000, CRC(f9d84e59) SHA1(80ec77025ac5bf355b1a60f2a678dd4c56071f6b) )

	ROM_REGION( 0x1c00000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0800.u1",    0x0000000, 0x0400000, CRC(e1e6ec40) SHA1(390432431f144ef63424a426582b311765a61771) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0800.u3",    0x0000000, 0x0200000, CRC(52e7bef5) SHA1(a678251b7e46a1016d0afc1d8d5c9928008ad5b1) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
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
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "sp_v101.u2",      0x100001, 0x080000,   CRC(08eb9661) SHA1(105946e72e562adb1a9fd794ca0fd2c91967eb56) )
	ROM_LOAD16_BYTE( "sp_v101.u3",      0x100000, 0x080000,   CRC(0a358c1e) SHA1(95c7c3f069c5d05001e22535750f6b3cd7de105f) )
	ROM_LOAD16_BYTE( "sp_v101.u4",      0x200001, 0x080000,   CRC(766570e0) SHA1(e7c3f5664ec69b662b82c2e1375555db7305390c) )
	ROM_LOAD16_BYTE( "sp_v101.u5",      0x200000, 0x080000,   CRC(58662e12) SHA1(2b39bd847e9c4968a8e77a2f3cec77cf323ceee3) )
	ROM_LOAD16_WORD_SWAP( "sp_v101.u1",0x300000, 0x080000,    CRC(2b2f4f1e) SHA1(67b97cf8cc7f517d67cd45588addd2ad8e24612a) )

	ROM_REGION( 0x010000, "user1", 0 ) /* IGS028 Protection Data */
	ROM_LOAD( "sp_v101.u6", 0x000000, 0x010000,  CRC(097046bc) SHA1(6d75db85cf4c79b63e837897785c253014b2126d) )

	ROM_REGION( 0x4000, "user2", ROMREGION_ERASEFF ) /* its a dump of the shared protection rom/ram from pcb. */
	// clearly not for this revision
	ROM_LOAD( "ram_dump", 0x000000, 0x04000, CRC(280cfb4e) SHA1(cd2bdcaa21347952c2bf38b105a204d327fde39e) )

	ROM_REGION( 0xc00000, "tiles",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0500.rom",    0x180000, 0x400000, CRC(d881726c) SHA1(a82517e665996f7b7017c940f1fcf016fccb65c2) )
	ROM_LOAD( "t0501.rom",    0x580000, 0x200000, CRC(d2106864) SHA1(65d827135b87d82196433aea3279608ee263feca) )

	ROM_REGION( 0x1c00000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0500.rom",    0x0000000, 0x400000, CRC(80a59197) SHA1(7d0108e2f0d0347d43ace2d96c0388202c05fdfb) )
	ROM_LOAD( "a0501.rom",    0x0400000, 0x400000, CRC(98c931b1) SHA1(9b70d1a7beb4c3a0c1436c25fd1fb76e47539538) )
	ROM_LOAD( "a0502.rom",    0x0800000, 0x400000, CRC(c3fcdf1d) SHA1(107585fd103fcd0af0fb7db283be4f7c7058aef7) )
	ROM_LOAD( "a0503.rom",    0x0c00000, 0x400000, CRC(066dffec) SHA1(f023032a7202b7090fb609a39e0f19018e664bf3) )
	ROM_LOAD( "a0504.rom",    0x1000000, 0x400000, CRC(45337583) SHA1(c954d0e5bf7fa99c90b0d154e7119d2b0c461f1c) )
	ROM_LOAD( "a0505.rom",    0x1400000, 0x400000, CRC(5b8cf3a5) SHA1(856d1e47b5d9a66dcfbdc74a51ed646fd7d96a35) )
	ROM_LOAD( "a0506.rom",    0x1800000, 0x400000, CRC(087ac60c) SHA1(3d5bf7dd40c8a3c1224cf82e12410ca904c0c5db) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0500.rom",    0x0000000, 0x400000, CRC(cde07f74) SHA1(1fe90db7c57faa28f3a054b2c07909bef89e3efb) )
	ROM_LOAD( "b0501.rom",    0x0400000, 0x400000, CRC(1546c2e9) SHA1(a7b9c8b44203db54a59d49fe469bb52bba807ba2) )
	ROM_LOAD( "b0502.rom",    0x0800000, 0x400000, CRC(e97b31c3) SHA1(1a7ca4f6c8644e84a33ae41cd4637f21046b14c5) )
	ROM_LOAD( "b0503.u16",    0x0c00000, 0x400000, CRC(e41d98e4) SHA1(f80b27fcee81762993e09bf1b3cad6e85274760c) )


	ROM_REGION( 0x600000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0500.rom",    0x400000, 0x200000, CRC(37928cdd) SHA1(e80498cabc2a6a54d4f3ebcb097d4b3fad96fe55) )
ROM_END

ROM_START( olds100 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "v100-u2.040",      0x100001, 0x080000,  CRC(517c2a06) SHA1(bbf5b311fac9b0bb4d4129c0561e5e24f6963fa2) )
	ROM_LOAD16_BYTE( "v100-u3.040",      0x100000, 0x080000,  CRC(d0e2b741) SHA1(2e671dbb4320d1f0c059b35efd33cdea26f12131) )
	ROM_LOAD16_BYTE( "v100-u4.040",      0x200001, 0x080000,  CRC(32a6bdbd) SHA1(a93d7f4eae722a58eca9ec351ad5890cefda56f0) )
	ROM_LOAD16_BYTE( "v100-u5.040",      0x200000, 0x080000,  CRC(b4a1cafb) SHA1(b2fccd480ede93f58ad043387b18b898152f06ef) )
	ROM_LOAD16_WORD_SWAP( "v100-u1.040", 0x300000, 0x080000,  CRC(37ea4e75) SHA1(a94fcb89da3394a43d360f885419677f511d2580) )

	ROM_REGION( 0x4000, "user2", ROMREGION_ERASEFF ) /* its a dump of the shared protection rom/ram from pcb. */
	// used to simulate encrypted DMA protection device for now ..
	ROM_LOAD( "ram_dump", 0x000000, 0x04000, CRC(280cfb4e) SHA1(cd2bdcaa21347952c2bf38b105a204d327fde39e) )

	ROM_REGION( 0x010000, "user1", 0 ) /* IGS028 Protection Data */
	ROM_LOAD( "kd-u6.512", 0x000000, 0x010000,  CRC(e7613dda) SHA1(0d7c043b90e2f9a36a45066f22e3e305dc716676) )

	ROM_REGION( 0xc00000, "tiles",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0500.rom",    0x180000, 0x400000, CRC(d881726c) SHA1(a82517e665996f7b7017c940f1fcf016fccb65c2) )
	ROM_LOAD( "t0501.rom",    0x580000, 0x200000, CRC(d2106864) SHA1(65d827135b87d82196433aea3279608ee263feca) )

	ROM_REGION( 0x1c00000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0500.rom",    0x0000000, 0x400000, CRC(80a59197) SHA1(7d0108e2f0d0347d43ace2d96c0388202c05fdfb) )
	ROM_LOAD( "a0501.rom",    0x0400000, 0x400000, CRC(98c931b1) SHA1(9b70d1a7beb4c3a0c1436c25fd1fb76e47539538) )
	ROM_LOAD( "a0502.rom",    0x0800000, 0x400000, CRC(c3fcdf1d) SHA1(107585fd103fcd0af0fb7db283be4f7c7058aef7) )
	ROM_LOAD( "a0503.rom",    0x0c00000, 0x400000, CRC(066dffec) SHA1(f023032a7202b7090fb609a39e0f19018e664bf3) )
	ROM_LOAD( "a0504.rom",    0x1000000, 0x400000, CRC(45337583) SHA1(c954d0e5bf7fa99c90b0d154e7119d2b0c461f1c) )
	ROM_LOAD( "a0505.rom",    0x1400000, 0x400000, CRC(5b8cf3a5) SHA1(856d1e47b5d9a66dcfbdc74a51ed646fd7d96a35) )
	ROM_LOAD( "a0506.rom",    0x1800000, 0x400000, CRC(087ac60c) SHA1(3d5bf7dd40c8a3c1224cf82e12410ca904c0c5db) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0500.rom",    0x0000000, 0x400000, CRC(cde07f74) SHA1(1fe90db7c57faa28f3a054b2c07909bef89e3efb) )
	ROM_LOAD( "b0501.rom",    0x0400000, 0x400000, CRC(1546c2e9) SHA1(a7b9c8b44203db54a59d49fe469bb52bba807ba2) )
	ROM_LOAD( "b0502.rom",    0x0800000, 0x400000, CRC(e97b31c3) SHA1(1a7ca4f6c8644e84a33ae41cd4637f21046b14c5) )
	ROM_LOAD( "b0503.u16",    0x0c00000, 0x400000, CRC(e41d98e4) SHA1(f80b27fcee81762993e09bf1b3cad6e85274760c) )

	ROM_REGION( 0x600000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0500.rom",    0x400000, 0x200000, CRC(37928cdd) SHA1(e80498cabc2a6a54d4f3ebcb097d4b3fad96fe55) )
ROM_END

/* this is the set which the protection ram dump seems to be for.. */
ROM_START( olds100a )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code  */
	PGM_68K_BIOS
	/* this rom had a lame hack applied to it by the dumper, this was removed, hopefully it is correct now */
	ROM_LOAD16_WORD_SWAP( "p0500.v10",    0x100000, 0x400000, CRC(8981fc87) SHA1(678d6705d06b99bca5951ff77708adadc4c4396b) )

	ROM_REGION( 0x010000, "user1", ROMREGION_ERASEFF ) /* IGS028 Protection Data */
	/* missing from this set .. */

	ROM_REGION( 0x4000, "user2", ROMREGION_ERASEFF ) /* its a dump of the shared protection rom/ram from pcb. */
	// used to simulate encrypted DMA protection device for now ..
	ROM_LOAD( "ram_dump", 0x000000, 0x04000, CRC(280cfb4e) SHA1(cd2bdcaa21347952c2bf38b105a204d327fde39e) )

	ROM_REGION( 0xc00000, "tiles",  0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0500.rom",    0x180000, 0x400000, CRC(d881726c) SHA1(a82517e665996f7b7017c940f1fcf016fccb65c2) )
	ROM_LOAD( "t0501.rom",    0x580000, 0x200000, CRC(d2106864) SHA1(65d827135b87d82196433aea3279608ee263feca) )

	ROM_REGION( 0x1c00000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0500.rom",    0x0000000, 0x400000, CRC(80a59197) SHA1(7d0108e2f0d0347d43ace2d96c0388202c05fdfb) )
	ROM_LOAD( "a0501.rom",    0x0400000, 0x400000, CRC(98c931b1) SHA1(9b70d1a7beb4c3a0c1436c25fd1fb76e47539538) )
	ROM_LOAD( "a0502.rom",    0x0800000, 0x400000, CRC(c3fcdf1d) SHA1(107585fd103fcd0af0fb7db283be4f7c7058aef7) )
	ROM_LOAD( "a0503.rom",    0x0c00000, 0x400000, CRC(066dffec) SHA1(f023032a7202b7090fb609a39e0f19018e664bf3) )
	ROM_LOAD( "a0504.rom",    0x1000000, 0x400000, CRC(45337583) SHA1(c954d0e5bf7fa99c90b0d154e7119d2b0c461f1c) )
	ROM_LOAD( "a0505.rom",    0x1400000, 0x400000, CRC(5b8cf3a5) SHA1(856d1e47b5d9a66dcfbdc74a51ed646fd7d96a35) )
	ROM_LOAD( "a0506.rom",    0x1800000, 0x400000, CRC(087ac60c) SHA1(3d5bf7dd40c8a3c1224cf82e12410ca904c0c5db) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0500.rom",    0x0000000, 0x400000, CRC(cde07f74) SHA1(1fe90db7c57faa28f3a054b2c07909bef89e3efb) )
	ROM_LOAD( "b0501.rom",    0x0400000, 0x400000, CRC(1546c2e9) SHA1(a7b9c8b44203db54a59d49fe469bb52bba807ba2) )
	ROM_LOAD( "b0502.rom",    0x0800000, 0x400000, CRC(e97b31c3) SHA1(1a7ca4f6c8644e84a33ae41cd4637f21046b14c5) )
	ROM_LOAD( "b0503.u16",    0x0c00000, 0x400000, CRC(e41d98e4) SHA1(f80b27fcee81762993e09bf1b3cad6e85274760c) )

	ROM_REGION( 0x600000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0500.rom",    0x400000, 0x200000, CRC(37928cdd) SHA1(e80498cabc2a6a54d4f3ebcb097d4b3fad96fe55) )
ROM_END


ROM_START( kov2 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "u18.107",      0x100000, 0x400000, CRC(661a5b2c) SHA1(125054fabc93d4f4cba869c3e6adf863650d30cf) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kov2_v100_hongkong.asic", 0x000000, 0x04000,  CRC(e0d7679f) SHA1(e1c2d127eba4ddbeb8ad173c55b90ac1467e1ca8) )

	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "u19.102", 0x000000, 0x200000,   CRC(462e2980) SHA1(3da7c3d2c65b59f50c78be1c25922b71d40f6080) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t1200.rom",    0x180000, 0x800000, CRC(d7e26609) SHA1(bdad810f82fcf1d50a8791bdc495374ec5a309c6) )

	ROM_REGION( 0x2800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a1200.rom",    0x0000000, 0x0800000, CRC(ceeb81d8) SHA1(5476729443fc1bc9593ae10fbf7cbc5d7290b017) )
	ROM_LOAD( "a1201.rom",    0x0800000, 0x0800000, CRC(82f0a878) SHA1(ddd13e404252a71de1b2b3b974b910f899f51c38) )
	ROM_LOAD( "a1202.rom",    0x1000000, 0x0800000, CRC(4bb92fae) SHA1(f0b6d72ed425de1c69dc8f8d5795ea760a4a59b0) )
	ROM_LOAD( "a1203.rom",    0x1800000, 0x0800000, CRC(e73cb627) SHA1(4c6e48b845a5d1e8f9899010fbf273d54c2b8899) )
	ROM_LOAD( "a1204.rom",    0x2000000, 0x0800000, CRC(27527099) SHA1(e23cf366bdeaca1e009a5cec6b13164310a34384) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b1200.rom",   0x0000000, 0x0800000,  CRC(bed7d994) SHA1(019dfba8154256d64cd249eb0fa4c451edce34b8) )
	ROM_LOAD( "b1201.rom",   0x0800000, 0x0800000,  CRC(f251eb57) SHA1(56a5fc14ab7822f83379cecb26638e5bb266349a) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m1200.rom",    0x800000, 0x800000, CRC(b0d88720) SHA1(44ab137e3f8e15b7cb5697ffbd9b1143d8210c4f) )
ROM_END

ROM_START( kov2106 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "u18.106",    0x100000, 0x400000, CRC(40051ad9) SHA1(ba2ddf267fe688d5dfed575aeeccbab10135b37b) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kov2_v100_hongkong.asic", 0x000000, 0x04000,  CRC(e0d7679f) SHA1(e1c2d127eba4ddbeb8ad173c55b90ac1467e1ca8) )

	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "u19.102", 0x000000, 0x200000,   CRC(462e2980) SHA1(3da7c3d2c65b59f50c78be1c25922b71d40f6080) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t1200.rom",    0x180000, 0x800000, CRC(d7e26609) SHA1(bdad810f82fcf1d50a8791bdc495374ec5a309c6) )

	ROM_REGION( 0x2800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a1200.rom",    0x0000000, 0x0800000, CRC(ceeb81d8) SHA1(5476729443fc1bc9593ae10fbf7cbc5d7290b017) )
	ROM_LOAD( "a1201.rom",    0x0800000, 0x0800000, CRC(82f0a878) SHA1(ddd13e404252a71de1b2b3b974b910f899f51c38) )
	ROM_LOAD( "a1202.rom",    0x1000000, 0x0800000, CRC(4bb92fae) SHA1(f0b6d72ed425de1c69dc8f8d5795ea760a4a59b0) )
	ROM_LOAD( "a1203.rom",    0x1800000, 0x0800000, CRC(e73cb627) SHA1(4c6e48b845a5d1e8f9899010fbf273d54c2b8899) )
	ROM_LOAD( "a1204.rom",    0x2000000, 0x0800000, CRC(27527099) SHA1(e23cf366bdeaca1e009a5cec6b13164310a34384) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b1200.rom",   0x0000000, 0x0800000,  CRC(bed7d994) SHA1(019dfba8154256d64cd249eb0fa4c451edce34b8) )
	ROM_LOAD( "b1201.rom",   0x0800000, 0x0800000,  CRC(f251eb57) SHA1(56a5fc14ab7822f83379cecb26638e5bb266349a) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m1200.rom",    0x800000, 0x800000, CRC(b0d88720) SHA1(44ab137e3f8e15b7cb5697ffbd9b1143d8210c4f) )
ROM_END


ROM_START( kov2103 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "u18.103",    0x100000, 0x400000, CRC(98c32f76) SHA1(ec7e35e8071bb7097e415493be4e40be0ca19fd6) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kov2_v100_hongkong.asic", 0x000000, 0x04000,  CRC(e0d7679f) SHA1(e1c2d127eba4ddbeb8ad173c55b90ac1467e1ca8) )

	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "u19.101", 0x000000, 0x200000, CRC(8c35f2fe) SHA1(d4858f97fcfad0f342fccbc9cf1590276cc3c69c) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t1200.rom",    0x180000, 0x800000, CRC(d7e26609) SHA1(bdad810f82fcf1d50a8791bdc495374ec5a309c6) )

	ROM_REGION( 0x2800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a1200.rom",    0x0000000, 0x0800000, CRC(ceeb81d8) SHA1(5476729443fc1bc9593ae10fbf7cbc5d7290b017) )
	ROM_LOAD( "a1201.rom",    0x0800000, 0x0800000, CRC(82f0a878) SHA1(ddd13e404252a71de1b2b3b974b910f899f51c38) )
	ROM_LOAD( "a1202.rom",    0x1000000, 0x0800000, CRC(4bb92fae) SHA1(f0b6d72ed425de1c69dc8f8d5795ea760a4a59b0) )
	ROM_LOAD( "a1203.rom",    0x1800000, 0x0800000, CRC(e73cb627) SHA1(4c6e48b845a5d1e8f9899010fbf273d54c2b8899) )
	ROM_LOAD( "a1204.rom",    0x2000000, 0x0800000, CRC(27527099) SHA1(e23cf366bdeaca1e009a5cec6b13164310a34384) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b1200.rom",   0x0000000, 0x0800000,  CRC(bed7d994) SHA1(019dfba8154256d64cd249eb0fa4c451edce34b8) )
	ROM_LOAD( "b1201.rom",   0x0800000, 0x0800000,  CRC(f251eb57) SHA1(56a5fc14ab7822f83379cecb26638e5bb266349a) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m1200.rom",    0x800000, 0x800000, CRC(b0d88720) SHA1(44ab137e3f8e15b7cb5697ffbd9b1143d8210c4f) )
ROM_END


ROM_START( kov2102 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "u18.102",    0x100000, 0x400000, CRC(a2489c37) SHA1(77ea7cdec211848296dafd45bee1d042133ea2a6) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kov2_v100_hongkong.asic", 0x000000, 0x04000,  CRC(e0d7679f) SHA1(e1c2d127eba4ddbeb8ad173c55b90ac1467e1ca8) )

	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "u19.101", 0x000000, 0x200000, CRC(8c35f2fe) SHA1(d4858f97fcfad0f342fccbc9cf1590276cc3c69c) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t1200.rom",    0x180000, 0x800000, CRC(d7e26609) SHA1(bdad810f82fcf1d50a8791bdc495374ec5a309c6) )

	ROM_REGION( 0x2800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a1200.rom",    0x0000000, 0x0800000, CRC(ceeb81d8) SHA1(5476729443fc1bc9593ae10fbf7cbc5d7290b017) )
	ROM_LOAD( "a1201.rom",    0x0800000, 0x0800000, CRC(82f0a878) SHA1(ddd13e404252a71de1b2b3b974b910f899f51c38) )
	ROM_LOAD( "a1202.rom",    0x1000000, 0x0800000, CRC(4bb92fae) SHA1(f0b6d72ed425de1c69dc8f8d5795ea760a4a59b0) )
	ROM_LOAD( "a1203.rom",    0x1800000, 0x0800000, CRC(e73cb627) SHA1(4c6e48b845a5d1e8f9899010fbf273d54c2b8899) )
	ROM_LOAD( "a1204.rom",    0x2000000, 0x0800000, CRC(27527099) SHA1(e23cf366bdeaca1e009a5cec6b13164310a34384) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b1200.rom",   0x0000000, 0x0800000,  CRC(bed7d994) SHA1(019dfba8154256d64cd249eb0fa4c451edce34b8) )
	ROM_LOAD( "b1201.rom",   0x0800000, 0x0800000,  CRC(f251eb57) SHA1(56a5fc14ab7822f83379cecb26638e5bb266349a) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m1200.rom",    0x800000, 0x800000, CRC(b0d88720) SHA1(44ab137e3f8e15b7cb5697ffbd9b1143d8210c4f) )
ROM_END

ROM_START( kov2101 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "u18.101",    0x100000, 0x400000, CRC(c9926f35) SHA1(a9c72d0c5d239164107894c7d3fffe4af29ed201) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kov2_v100_hongkong.asic", 0x000000, 0x04000,  CRC(e0d7679f) SHA1(e1c2d127eba4ddbeb8ad173c55b90ac1467e1ca8) )

	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "u19.101", 0x000000, 0x200000, CRC(8c35f2fe) SHA1(d4858f97fcfad0f342fccbc9cf1590276cc3c69c) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t1200.rom",    0x180000, 0x800000, CRC(d7e26609) SHA1(bdad810f82fcf1d50a8791bdc495374ec5a309c6) )

	ROM_REGION( 0x2800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a1200.rom",    0x0000000, 0x0800000, CRC(ceeb81d8) SHA1(5476729443fc1bc9593ae10fbf7cbc5d7290b017) )
	ROM_LOAD( "a1201.rom",    0x0800000, 0x0800000, CRC(82f0a878) SHA1(ddd13e404252a71de1b2b3b974b910f899f51c38) )
	ROM_LOAD( "a1202.rom",    0x1000000, 0x0800000, CRC(4bb92fae) SHA1(f0b6d72ed425de1c69dc8f8d5795ea760a4a59b0) )
	ROM_LOAD( "a1203.rom",    0x1800000, 0x0800000, CRC(e73cb627) SHA1(4c6e48b845a5d1e8f9899010fbf273d54c2b8899) )
	ROM_LOAD( "a1204.rom",    0x2000000, 0x0800000, CRC(27527099) SHA1(e23cf366bdeaca1e009a5cec6b13164310a34384) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b1200.rom",   0x0000000, 0x0800000,  CRC(bed7d994) SHA1(019dfba8154256d64cd249eb0fa4c451edce34b8) )
	ROM_LOAD( "b1201.rom",   0x0800000, 0x0800000,  CRC(f251eb57) SHA1(56a5fc14ab7822f83379cecb26638e5bb266349a) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m1200.rom",    0x800000, 0x800000, CRC(b0d88720) SHA1(44ab137e3f8e15b7cb5697ffbd9b1143d8210c4f) )
ROM_END

ROM_START( kov2100 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "igs_u18.rom",    0x100000, 0x400000, CRC(86205879) SHA1(f73d5b70b41d39be1cac75e474b025de2cce0b01) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kov2_v100_hongkong.asic", 0x000000, 0x04000,  CRC(e0d7679f) SHA1(e1c2d127eba4ddbeb8ad173c55b90ac1467e1ca8) )

	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "u19.100", 0x000000, 0x200000,   CRC(edd59922) SHA1(09b14f20f685944a93292c83e5830849aade42c9) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t1200.rom",    0x180000, 0x800000, CRC(d7e26609) SHA1(bdad810f82fcf1d50a8791bdc495374ec5a309c6) )

	ROM_REGION( 0x2800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a1200.rom",    0x0000000, 0x0800000, CRC(ceeb81d8) SHA1(5476729443fc1bc9593ae10fbf7cbc5d7290b017) )
	ROM_LOAD( "a1201.rom",    0x0800000, 0x0800000, CRC(82f0a878) SHA1(ddd13e404252a71de1b2b3b974b910f899f51c38) )
	ROM_LOAD( "a1202.rom",    0x1000000, 0x0800000, CRC(4bb92fae) SHA1(f0b6d72ed425de1c69dc8f8d5795ea760a4a59b0) )
	ROM_LOAD( "a1203.rom",    0x1800000, 0x0800000, CRC(e73cb627) SHA1(4c6e48b845a5d1e8f9899010fbf273d54c2b8899) )
	ROM_LOAD( "a1204.rom",    0x2000000, 0x0800000, CRC(27527099) SHA1(e23cf366bdeaca1e009a5cec6b13164310a34384) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b1200.rom",   0x0000000, 0x0800000,  CRC(bed7d994) SHA1(019dfba8154256d64cd249eb0fa4c451edce34b8) )
	ROM_LOAD( "b1201.rom",   0x0800000, 0x0800000,  CRC(f251eb57) SHA1(56a5fc14ab7822f83379cecb26638e5bb266349a) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m1200.rom",    0x800000, 0x800000, CRC(b0d88720) SHA1(44ab137e3f8e15b7cb5697ffbd9b1143d8210c4f) )
ROM_END

ROM_START( kov2p )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v204-32m.rom",    0x100000, 0x400000, CRC(583e0650) SHA1(2e5656dd9c6cba9f84af9baa3f5f70cdccf9db47) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kov2p_igs027a.bin", 0x000000, 0x04000, BAD_DUMP CRC(e0d7679f) SHA1(e1c2d127eba4ddbeb8ad173c55b90ac1467e1ca8) ) // NOT for this version, works with a hack

	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "v200-16.rom", 0x000000, 0x200000,  CRC(16a0c11f) SHA1(ce449cef76ebd5657d49b57951e2eb0f132e203e) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t1200.rom",    0x180000, 0x800000, CRC(d7e26609) SHA1(bdad810f82fcf1d50a8791bdc495374ec5a309c6) )

	ROM_REGION( 0x2800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a1200.rom",    0x0000000, 0x0800000, CRC(ceeb81d8) SHA1(5476729443fc1bc9593ae10fbf7cbc5d7290b017) )
	ROM_LOAD( "a1201.rom_p",  0x0800000, 0x0800000, CRC(21063ca7) SHA1(cf561b44902425a920d5cbea5bf65dd9530b2289) ) // either this or a1201.rom in kov2 is probably bad
	ROM_LOAD( "a1202.rom",    0x1000000, 0x0800000, CRC(4bb92fae) SHA1(f0b6d72ed425de1c69dc8f8d5795ea760a4a59b0) )
	ROM_LOAD( "a1203.rom",    0x1800000, 0x0800000, CRC(e73cb627) SHA1(4c6e48b845a5d1e8f9899010fbf273d54c2b8899) )
	ROM_LOAD( "a1204.rom_p",  0x2000000, 0x0200000, CRC(14b4b5bb) SHA1(d7db5740eec971f2782fb2885ee3af8f2a796550) ) // either this or a1204.rom in kov2 is probably bad

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b1200.rom",   0x0000000, 0x0800000,  CRC(bed7d994) SHA1(019dfba8154256d64cd249eb0fa4c451edce34b8) )
	ROM_LOAD( "b1201.rom",   0x0800000, 0x0800000,  CRC(f251eb57) SHA1(56a5fc14ab7822f83379cecb26638e5bb266349a) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m1200.rom",    0x800000, 0x800000, CRC(b0d88720) SHA1(44ab137e3f8e15b7cb5697ffbd9b1143d8210c4f) )
ROM_END

ROM_START( kov2p205 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "u8-27322.rom",    0x100000, 0x400000, CRC(3a2cc0de) SHA1(d7511478b34bfb03b2fb5b8268b60502d05b9414) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kov2p_igs027a.bin", 0x000000, 0x04000, BAD_DUMP CRC(e0d7679f) SHA1(e1c2d127eba4ddbeb8ad173c55b90ac1467e1ca8) ) // NOT for this version, works with a hack

	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "v200-16.rom", 0x000000, 0x200000,  CRC(16a0c11f) SHA1(ce449cef76ebd5657d49b57951e2eb0f132e203e) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t1200.rom",    0x180000, 0x800000, CRC(d7e26609) SHA1(bdad810f82fcf1d50a8791bdc495374ec5a309c6) )

	ROM_REGION( 0x2800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a1200.rom",    0x0000000, 0x0800000, CRC(ceeb81d8) SHA1(5476729443fc1bc9593ae10fbf7cbc5d7290b017) )
	ROM_LOAD( "a1201.rom_p",  0x0800000, 0x0800000, CRC(21063ca7) SHA1(cf561b44902425a920d5cbea5bf65dd9530b2289) ) // either this or a1201.rom in kov2 is probably bad
	ROM_LOAD( "a1202.rom",    0x1000000, 0x0800000, CRC(4bb92fae) SHA1(f0b6d72ed425de1c69dc8f8d5795ea760a4a59b0) )
	ROM_LOAD( "a1203.rom",    0x1800000, 0x0800000, CRC(e73cb627) SHA1(4c6e48b845a5d1e8f9899010fbf273d54c2b8899) )
	ROM_LOAD( "a1204.rom_p",  0x2000000, 0x0200000, CRC(14b4b5bb) SHA1(d7db5740eec971f2782fb2885ee3af8f2a796550) ) // either this or a1204.rom in kov2 is probably bad

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b1200.rom",   0x0000000, 0x0800000,  CRC(bed7d994) SHA1(019dfba8154256d64cd249eb0fa4c451edce34b8) )
	ROM_LOAD( "b1201.rom",   0x0800000, 0x0800000,  CRC(f251eb57) SHA1(56a5fc14ab7822f83379cecb26638e5bb266349a) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
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
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v102.u8", 0x100000, 0x200000, CRC(5a9ea040) SHA1(51eaec46c368f7cfc5245e64896092f52b1193e0) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "ddp2_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* Protection Data (encrypted external ARM data, internal missing) */
	ROM_LOAD( "v100.u23", 0x000000, 0x20000, CRC(06c3dd29) SHA1(20c9479f158467fc2037dcf162b6c6be18c91d46) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t1300.u21",    0x180000, 0x800000, CRC(e748f0cb) SHA1(5843bee3a17c33648ce904af2b98c6a90aff7393) )

	ROM_REGION( 0x1000000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a1300.u1",    0x0000000, 0x0800000, CRC(fc87a405) SHA1(115c21ecc56997652e527c92654076870bc9fa51) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a1301.u2",    0x0800000, 0x0800000, CRC(0c8520da) SHA1(390317857ae5baa94a4cc042874b00a811f06a63) ) // FIXED BITS (xxxxxxxx1xxxxxxx)

	ROM_REGION( 0x0800000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b1300.u7",    0x0000000, 0x0800000,  CRC(ef646604) SHA1(d737ff513792962f18df88c2caa9dd71de449079) )

	ROM_REGION( 0x800000, "ics", ROMREGION_ERASE00 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m1300.u5",    0x400000, 0x400000, CRC(82d4015d) SHA1(d4cdc1aec1c97cf23ff7a20ccaad822962e66ffa) )
ROM_END

ROM_START( ddp2101 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v101_16m.u8", 0x100000, 0x200000, CRC(5e5786fd) SHA1(c6fc2956b5dc6a97c0d7d808a8c58aa21fa023b9) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "ddp2_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* Protection Data (encrypted external ARM data, internal missing) */
	ROM_LOAD( "v100.u23", 0x000000, 0x20000, CRC(06c3dd29) SHA1(20c9479f158467fc2037dcf162b6c6be18c91d46) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t1300.u21",    0x180000, 0x800000, CRC(e748f0cb) SHA1(5843bee3a17c33648ce904af2b98c6a90aff7393) )

	ROM_REGION( 0x1000000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a1300.u1",    0x0000000, 0x0800000, CRC(fc87a405) SHA1(115c21ecc56997652e527c92654076870bc9fa51) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a1301.u2",    0x0800000, 0x0800000, CRC(0c8520da) SHA1(390317857ae5baa94a4cc042874b00a811f06a63) ) // FIXED BITS (xxxxxxxx1xxxxxxx)

	ROM_REGION( 0x0800000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b1300.u7",    0x0000000, 0x0800000,  CRC(ef646604) SHA1(d737ff513792962f18df88c2caa9dd71de449079) )

	ROM_REGION( 0x800000, "ics", ROMREGION_ERASE00 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m1300.u5",    0x400000, 0x400000, CRC(82d4015d) SHA1(d4cdc1aec1c97cf23ff7a20ccaad822962e66ffa) )
ROM_END

ROM_START( ddp2100 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v100.u8", 0x100000, 0x200000, CRC(0c8aa8ea) SHA1(57e33224622607a1df8daabf26ba063cf8a6d3fc) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "ddp2_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* Protection Data (encrypted external ARM data, internal missing) */
	ROM_LOAD( "v100.u23", 0x000000, 0x20000, CRC(06c3dd29) SHA1(20c9479f158467fc2037dcf162b6c6be18c91d46) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t1300.u21",    0x180000, 0x800000, CRC(e748f0cb) SHA1(5843bee3a17c33648ce904af2b98c6a90aff7393) )

	ROM_REGION( 0x1000000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a1300.u1",    0x0000000, 0x0800000, CRC(fc87a405) SHA1(115c21ecc56997652e527c92654076870bc9fa51) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a1301.u2",    0x0800000, 0x0800000, CRC(0c8520da) SHA1(390317857ae5baa94a4cc042874b00a811f06a63) ) // FIXED BITS (xxxxxxxx1xxxxxxx)

	ROM_REGION( 0x0800000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b1300.u7",    0x0000000, 0x0800000,  CRC(ef646604) SHA1(d737ff513792962f18df88c2caa9dd71de449079) )

	ROM_REGION( 0x800000, "ics", ROMREGION_ERASE00 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m1300.u5",    0x400000, 0x400000, CRC(82d4015d) SHA1(d4cdc1aec1c97cf23ff7a20ccaad822962e66ffa) )
ROM_END


/*

Dragon World 2001 IGS PGM cart

Top board
---------
PCB Number: IGS PCB-0349-01-FL
22MHz OSC
2x 27C4096 EPROMs at U12, U22
1x 27C160 EPROM at U11
2x PALs at U4, U6
3x 62256 SRAM at U1, U23, U24
IGS027A at U7

Bottom Board
------------
PCB Number: IGS PCB-0350-00T-FL-A
4x 27C160 EPROMs at U2, U3, U7, U9

*/

ROM_START( dw2001 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "2001.u22", 0x100000, 0x80000, CRC(5cabed92) SHA1(d513e353c5c4695b16228e0bda9388c396aa4a81) )

	ROM_REGION( 0x4000, "prot", ROMREGION_ERASEFF ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "dw2001_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION32_LE( 0x4000000, "user1", ROMREGION_ERASEFF )
	ROM_LOAD( "2001.u12", 0x000000, 0x80000, CRC(973db1ab) SHA1(cc35e1a8534fa5d59d888f530769bae4e08c62ca) ) // external ARM data rom (encrypted)

	ROM_REGION( 0x600000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "2001.u11",    0x180000, 0x200000, CRC(1dd8d1e9) SHA1(13dc5d8e541bbd6eef9f477aa288978bccf7ebb3) )

	ROM_REGION( 0x400000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "2001.u2",    0x000000, 0x200000, CRC(d11c733c) SHA1(8faad32e8e215631a2263bdd51a9ae434540d028) )
	ROM_LOAD( "2001.u3",    0x200000, 0x200000, CRC(1435aef2) SHA1(582eb9f6415c89418401be7ebad041adeb600515) )

	ROM_REGION( 0x0200000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "2001.u9",    0x000000, 0x200000, CRC(ccbca572) SHA1(4d3512e82cb65e5cdfcc6cb18deec9f4a6dd350a) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "2001.u7",      0x200000, 0x200000, CRC(4ea62f21) SHA1(318f8a1ff5d4ff029a1c4133fe7acc2fc185d112) )
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
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_BYTE( "2sp_v200.u3",     0x100001, 0x080000, CRC(2a5ba8a6) SHA1(4c87b849fd6f39152e3e2ef699b78ce24b3fb6d0) )
	ROM_LOAD16_BYTE( "2sp_v200.u4",     0x100000, 0x080000, CRC(fa5c86c1) SHA1(11c219722b891b775c0f7f9bc8276cdd8f74d657) )

	ROM_REGION( 0x4000, "prot", ROMREGION_ERASEFF ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "puzzli2_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION32_LE( 0x4000000, "user1", ROMREGION_ERASEFF )
	/* not on this PCB */

	ROM_REGION( 0x600000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0900.u9",    0x180000, 0x200000, CRC(70615611) SHA1(a46d4aa71396947b427f9ba4ba0e636876c09d6b) )

	ROM_REGION( 0x400000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0900.u3",    0x0000000, 0x0400000, CRC(14911251) SHA1(e0d10ef50c408dbcf0907f81d4f0e49aeb651a6c) ) // FIXED BITS (xxxxxxxx1xxxxxxx)

	ROM_REGION( 0x0200000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0900.u4",    0x0000000, 0x0200000,  CRC(6f0638b6) SHA1(14b315fe9e80b3314bb63487e6ea9ce04c9703bd) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
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
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v104_32m.u9",    0x100000, 0x400000, CRC(cfd9dff4) SHA1(328eaf6ac49a73265ee4e0f992b1b1312f49877b) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "martial_masters_v102_usa.asic", 0x000000, 0x04000, CRC(a6c0828c) SHA1(0a5bda56dca264c3c7ff7698b8f699563f203c4d) ) // not verified, could be bad

	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "v102_16m.u10", 0x000000, 0x200000,  CRC(18b745e6) SHA1(7bcb58dd3a2d6072f492cf0dd7181cb061c1f49d) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t1000.u3",    0x180000, 0x800000, CRC(bbf879b5) SHA1(bd9a6aea34ad4001e89e62ff4b7a2292eb833c00) )

	ROM_REGION( 0x2800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a1000.u3",    0x0000000, 0x0800000, CRC(43577ac8) SHA1(6eea8b455985d5bac74dcc9943cdc3c0902de6cc) )
	ROM_LOAD( "a1001.u4",    0x0800000, 0x0800000, CRC(fe7a476f) SHA1(a8c7f1f0dd3e53141aed6d927eb88a3ceebb81e4) )
	ROM_LOAD( "a1002.u6",    0x1000000, 0x0800000, CRC(62e33d38) SHA1(96163d583e25073594f8413ce263e56b66bd69a1) )
	ROM_LOAD( "a1003.u8",    0x1800000, 0x0800000, CRC(b2c4945a) SHA1(7b18287a2db56db3651cfd4deb607af53522fefd) )
	ROM_LOAD( "a1004.u10",   0x2000000, 0x0400000, CRC(9fd3f5fd) SHA1(057531f91062be51589c6cf8f4170089b9be6380) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b1000.u9",    0x0000000, 0x0800000,  CRC(c5961f6f) SHA1(a68060b10edbd084cbde79d2ed1c9084777beb10) )
	ROM_LOAD( "b1001.u11",   0x0800000, 0x0800000,  CRC(0b7e1c06) SHA1(545e15e0087f8621d593fecd8b4013f7ca311686) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m1000.u5",    0x400000, 0x800000, CRC(ed407ae8) SHA1(a6e9c09b39c13e8fb7fbc89fa9f823cbeb66e901) )
	ROM_LOAD( "m1001.u7",    0xc00000, 0x400000, CRC(662d2d48) SHA1(2fcc3099d9c04456cae3b13035fb28eaf709e7d8) )
ROM_END

ROM_START( martmastc )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v104_32m.u9",    0x100000, 0x400000, CRC(cfd9dff4) SHA1(328eaf6ac49a73265ee4e0f992b1b1312f49877b) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "martial_masters_v101_cn.asic", 0x000000, 0x04000, CRC(b3e25b7d) SHA1(6147d7ee2e11636521df1bb96ed5da8ad21b2a57) ) // not verified, could be bad

	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "v102_16m.u10", 0x000000, 0x200000,  CRC(18b745e6) SHA1(7bcb58dd3a2d6072f492cf0dd7181cb061c1f49d) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t1000.u3",    0x180000, 0x800000, CRC(bbf879b5) SHA1(bd9a6aea34ad4001e89e62ff4b7a2292eb833c00) )

	ROM_REGION( 0x2800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a1000.u3",    0x0000000, 0x0800000, CRC(43577ac8) SHA1(6eea8b455985d5bac74dcc9943cdc3c0902de6cc) )
	ROM_LOAD( "a1001.u4",    0x0800000, 0x0800000, CRC(fe7a476f) SHA1(a8c7f1f0dd3e53141aed6d927eb88a3ceebb81e4) )
	ROM_LOAD( "a1002.u6",    0x1000000, 0x0800000, CRC(62e33d38) SHA1(96163d583e25073594f8413ce263e56b66bd69a1) )
	ROM_LOAD( "a1003.u8",    0x1800000, 0x0800000, CRC(b2c4945a) SHA1(7b18287a2db56db3651cfd4deb607af53522fefd) )
	ROM_LOAD( "a1004.u10",   0x2000000, 0x0400000, CRC(9fd3f5fd) SHA1(057531f91062be51589c6cf8f4170089b9be6380) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b1000.u9",    0x0000000, 0x0800000,  CRC(c5961f6f) SHA1(a68060b10edbd084cbde79d2ed1c9084777beb10) )
	ROM_LOAD( "b1001.u11",   0x0800000, 0x0800000,  CRC(0b7e1c06) SHA1(545e15e0087f8621d593fecd8b4013f7ca311686) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m1000.u5",    0x400000, 0x800000, CRC(ed407ae8) SHA1(a6e9c09b39c13e8fb7fbc89fa9f823cbeb66e901) )
	ROM_LOAD( "m1001.u7",    0xc00000, 0x400000, CRC(662d2d48) SHA1(2fcc3099d9c04456cae3b13035fb28eaf709e7d8) )
ROM_END

ROM_START( martmastc102 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "martmast_u9-v102.322",    0x100000, 0x400000, CRC(bb24b92a) SHA1(442cb9e3f51727be82f71c078c5c3e49dc1a23f0) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "martial_masters_v101_cn.asic", 0x000000, 0x04000, CRC(b3e25b7d) SHA1(6147d7ee2e11636521df1bb96ed5da8ad21b2a57) ) // not verified, could be bad

	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "martmast_u10-v101.160", 0x000000, 0x200000,   CRC(d5d93294) SHA1(58d0a99749f7dc05814892b508ce21b160410947) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t1000.u3",    0x180000, 0x800000, CRC(bbf879b5) SHA1(bd9a6aea34ad4001e89e62ff4b7a2292eb833c00) )

	ROM_REGION( 0x2800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a1000.u3",    0x0000000, 0x0800000, CRC(43577ac8) SHA1(6eea8b455985d5bac74dcc9943cdc3c0902de6cc) )
	ROM_LOAD( "a1001.u4",    0x0800000, 0x0800000, CRC(fe7a476f) SHA1(a8c7f1f0dd3e53141aed6d927eb88a3ceebb81e4) )
	ROM_LOAD( "a1002.u6",    0x1000000, 0x0800000, CRC(62e33d38) SHA1(96163d583e25073594f8413ce263e56b66bd69a1) )
	ROM_LOAD( "a1003.u8",    0x1800000, 0x0800000, CRC(b2c4945a) SHA1(7b18287a2db56db3651cfd4deb607af53522fefd) )
	ROM_LOAD( "a1004.u10",   0x2000000, 0x0400000, CRC(9fd3f5fd) SHA1(057531f91062be51589c6cf8f4170089b9be6380) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b1000.u9",    0x0000000, 0x0800000,  CRC(c5961f6f) SHA1(a68060b10edbd084cbde79d2ed1c9084777beb10) )
	ROM_LOAD( "b1001.u11",   0x0800000, 0x0800000,  CRC(0b7e1c06) SHA1(545e15e0087f8621d593fecd8b4013f7ca311686) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
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
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v102_16m.u5",    0x100000, 0x200000, CRC(3d4d481a) SHA1(95953b8f31343389405cc722b4177ff5adf67b62) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "dmnfrnt_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0x800000, "user1", 0 ) /* Protection Data (encrypted external ARM data, internal missing) */
	ROM_LOAD( "v101_32m.u26", 0x000000, 0x400000,  CRC(93965281) SHA1(89da198aaa7ca759cb96b5f18859a477e55fd590) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t04501.u29",    0x180000, 0x800000, CRC(900eaaac) SHA1(4033cb7b28fcadb92d5af3ea7fdd1c22747618fd) )

	ROM_REGION( 0x1800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a04501.u3",    0x0000000, 0x0800000, CRC(9741bea6) SHA1(e3e904249be228628c8c2bd3495cda23586dc048) )
	ROM_LOAD( "a04502.u4",    0x0800000, 0x0800000, CRC(e104f405) SHA1(124b3deed3e838f8bae6c7d78bdd788859597585) )
	ROM_LOAD( "a04503.u6",    0x1000000, 0x0800000, CRC(bfd5cfe3) SHA1(fbe4c0a2987c2036df707b86597d78124ee2e665) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b04501.u9",    0x0000000, 0x0800000,  CRC(29320b7d) SHA1(59c78805e666f912df201c34616744f46057937b) )
	ROM_LOAD( "b04502.u11",   0x0800000, 0x0200000,  CRC(578c00e9) SHA1(14235cc8b0f8c7dd659512f017a2d4aacd91d89d) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "w04501.u5",    0x400000, 0x800000, CRC(3ab58137) SHA1(b221f7e551ff0bfa3fd97b6ebedbac69442a66e9) )
ROM_END

ROM_START( dmnfrnta )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v105_16m.u5",    0x100000, 0x200000, CRC(bda083bd) SHA1(58d6438737a2c43aa8bbcb7f34fb51375b781b1c) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "dmnfrnt_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0x800000, "user1", 0 ) /* Protection Data (encrypted external ARM data, internal missing) */
	/* one of these is probably a bad dump, it should be obvious once progress is made because the external rom is checksummed by the internal one */
	ROM_LOAD( "v105_32m.u26",     0x000000, 0x400000,  CRC(d200ee63) SHA1(3128c27c5f5a4361d31e7b4bb006de631b3a228c) )
	ROM_LOAD( "chinese-v105.u62", 0x000000, 0x400000,  CRC(c798c2ef) SHA1(91e364c33b935293fa765ca521cdb67ac45ec70f) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t04501.u29",    0x180000, 0x800000, CRC(900eaaac) SHA1(4033cb7b28fcadb92d5af3ea7fdd1c22747618fd) )

	ROM_REGION( 0x1800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a04501.u3",    0x0000000, 0x0800000, CRC(9741bea6) SHA1(e3e904249be228628c8c2bd3495cda23586dc048) )
	ROM_LOAD( "a04502.u4",    0x0800000, 0x0800000, CRC(e104f405) SHA1(124b3deed3e838f8bae6c7d78bdd788859597585) )
	ROM_LOAD( "a04503.u6",    0x1000000, 0x0800000, CRC(bfd5cfe3) SHA1(fbe4c0a2987c2036df707b86597d78124ee2e665) )

	ROM_REGION( 0xc00000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b04501.u9",    0x0000000, 0x0800000,  CRC(29320b7d) SHA1(59c78805e666f912df201c34616744f46057937b) )
	ROM_LOAD( "b04502.u11",   0x0800000, 0x0200000,  CRC(578c00e9) SHA1(14235cc8b0f8c7dd659512f017a2d4aacd91d89d) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "w04501.u5",    0x400000, 0x800000, CRC(3ab58137) SHA1(b221f7e551ff0bfa3fd97b6ebedbac69442a66e9) )
ROM_END

ROM_START( theglad )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "u6.rom",       0x100000, 0x080000, CRC(14c85212) SHA1(8d2489708e176a2c460498a13173be01f645b79e) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "theglad_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0x800000, "user1", 0 ) /* Protection Data (encrypted external ARM data, internal missing) */
	ROM_LOAD( "u2.rom", 0x000000, 0x200000,  CRC(c7bcf2ae) SHA1(10bc012c83987f594d5375a51bc4be2e17568a81) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t04601.u33",   0x180000, 0x800000, CRC(e5dab371) SHA1(2e3c93958eb0326b6b84b95c2168626f26bbac76) )

	ROM_REGION( 0x1800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a04601.u2",    0x0000000, 0x0800000,  CRC(d9b2e004) SHA1(8e1882b800fe9f12d7d49303e7417ba5b6f8ef85) )
	ROM_LOAD( "a04602.u4",    0x0800000, 0x0800000,  CRC(14f22308) SHA1(7fad54704e8c97eab723f53dfb50fb3e7bb606d2) )
	ROM_LOAD( "a04603.u6",    0x1000000, 0x0800000,  CRC(8f621e17) SHA1(b0f87f378e0115d0c95017ca0f1b0d508827a7c6) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b04601.u11",   0x0000000, 0x0800000, CRC(ee72bccf) SHA1(73c25fe659f6c903447066e4ef83d2f580449d76) )
	ROM_LOAD( "b04602.u12",   0x0800000, 0x0400000, CRC(7dba9c38) SHA1(a03d509274e8f6a500a7ebe2da5aab8bed4e7f2f) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "w04601.u1",    0x400000, 0x800000, CRC(5f15ddb3) SHA1(c38dcef8e06802a84e42a7fc9fa505475fc3ac65) )
ROM_END

ROM_START( theglada )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v101.u6",      0x100000, 0x080000, CRC(f799e866) SHA1(dccc3c903357c40c3cf85ac0ae8fc12fb0f853a6) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "theglad_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0x800000, "user1", 0 ) /* Protection Data (encrypted external ARM data, internal missing) */
	ROM_LOAD( "v107.u26", 0x000000, 0x200000,  CRC(f7c61357) SHA1(52d31c464dfc83c5371b078cb6b73c0d0e0d57e3) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t04601.u33",   0x180000, 0x800000, CRC(e5dab371) SHA1(2e3c93958eb0326b6b84b95c2168626f26bbac76) )

	ROM_REGION( 0x1800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a04601.u2",    0x0000000, 0x0800000,  CRC(d9b2e004) SHA1(8e1882b800fe9f12d7d49303e7417ba5b6f8ef85) )
	ROM_LOAD( "a04602.u4",    0x0800000, 0x0800000,  CRC(14f22308) SHA1(7fad54704e8c97eab723f53dfb50fb3e7bb606d2) )
	ROM_LOAD( "a04603.u6",    0x1000000, 0x0800000,  CRC(8f621e17) SHA1(b0f87f378e0115d0c95017ca0f1b0d508827a7c6) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b04601.u11",   0x0000000, 0x0800000, CRC(ee72bccf) SHA1(73c25fe659f6c903447066e4ef83d2f580449d76) )
	ROM_LOAD( "b04602.u12",   0x0800000, 0x0400000, CRC(7dba9c38) SHA1(a03d509274e8f6a500a7ebe2da5aab8bed4e7f2f) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "w04601.u1",    0x400000, 0x800000, CRC(5f15ddb3) SHA1(c38dcef8e06802a84e42a7fc9fa505475fc3ac65) )
ROM_END

ROM_START( oldsplus )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "p05301.rom",   0x100000, 0x400000, CRC(923f7246) SHA1(818ade79e9724f5a2b0cc5a647ae5d4ee0374799) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "oldsplus_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0x800000, "user1", ROMREGION_ERASE00 )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t05301.rom",   0x180000, 0x800000, CRC(8257bbb0) SHA1(b48067b7e7081a15fddf21739b641d677c2df3d9) )

	ROM_REGION( 0x2000000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a05301.rom",   0x0000000, 0x0800000, CRC(57946fd2) SHA1(5d79bc71a1881f3099821a9b255a5f271e0eeff6) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a05302.rom",   0x0800000, 0x0800000, CRC(3459a0b8) SHA1(94ab6f980b5582f1db9bb12019d03f0b6e0a06df) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a05303.rom",   0x1000000, 0x0800000, CRC(13475d85) SHA1(4683a3bf304fdc15ffb1c61b7957ad68b023fa33) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a05304.rom",   0x1800000, 0x0800000, CRC(f03ef7a6) SHA1(c18b1b622b430d5e031e65daa6819b84c3e12ef5) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b05301.rom",   0x0000000, 0x0800000, CRC(fd98f503) SHA1(02046ab1aa89f63bff149003d9d61776e025a92a) )
	ROM_LOAD( "b05302.rom",   0x0800000, 0x0800000, CRC(9f6094a8) SHA1(69f6f2003ab975eae13ea6b5c2ffa40df6e6bdf6) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m05301.rom",   0x400000, 0x400000, CRC(86ec83bc) SHA1(067cb7ec449eacd1f49298f45a364368934db5dd) )
ROM_END

ROM_START( kovshp )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "p0600h.rom",   0x100000, 0x400000, CRC(e251e8e4) SHA1(af5b7c81632a39e1450d932951bed634c76b84e8) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "kovshp_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t0600.rom",    0x180000, 0x800000, CRC(4acc1ad6) SHA1(0668dbd5e856c2406910c6b7382548b37c631780) )

	ROM_REGION( 0x2000000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a0600.rom",    0x0000000, 0x0800000, CRC(d8167834) SHA1(fa55a99629d03b2ea253392352f70d2c8639a991) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0601.rom",    0x0800000, 0x0800000, CRC(ff7a4373) SHA1(7def9fca7513ad5a117da230bebd2e3c78679041) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0602.rom",    0x1000000, 0x0800000, CRC(e7a32959) SHA1(3d0ed684dc5b269238890836b2ce7ef46aa5265b) ) // FIXED BITS (xxxxxxxx1xxxxxxx)
	ROM_LOAD( "a0540.rom",    0x1800000, 0x0800000, CRC(4fd3413e) SHA1(5e8f3e421342bf558c77e59635f9b5d713e825c2) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b0600.rom",    0x0000000, 0x0800000, CRC(7d3cd059) SHA1(00cf994b63337e0e4ebe96453daf45f24192af1c) )
	ROM_LOAD( "b0540.rom",    0x0800000, 0x0800000, CRC(60999757) SHA1(118cf058e67858958bcb931e14f5d160c7de87cc) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "m0600.rom",    0x400000, 0x400000, CRC(3ada4fd6) SHA1(4c87adb25d31cbd41f04fbffe31f7bc37173da76) )
ROM_END

ROM_START( killbldp )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "v300x.u6",     0x100000, 0x080000, CRC(b7fb8ec9) SHA1(e71b2d74269a82c7155b9818821156e128b68b28) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	/* the first ~0x200 bytes of this are 'execute only' and can't be read directly, even when running code from internal/external ARM romspace */
	ROM_LOAD( "killbldp_igs027a.bin", 0x000000, 0x04000, BAD_DUMP CRC(9a73bf7d) SHA1(2ce1311b01e1124ad00af172f0670141bcb7a030) )

	ROM_REGION( 0x800000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "v300x.u26", 0x000000, 0x200000,  CRC(144388c8) SHA1(d7469df077c1a674129f18210584ba4d05a61888) )

	ROM_REGION( 0x800000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t05701w032.bin",0x180000, 0x400000, CRC(567c714f) SHA1(b25b20e1ec9f077d6f7b9d41723a68d0d461bef2) )

	ROM_REGION( 0x1800000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a05701w064.bin",   0x0000000, 0x0800000, CRC(8c0c992c) SHA1(28391e50ca4400060676f1524bd49ede373292da) )
	ROM_LOAD( "a05702w064.bin",   0x0800000, 0x0800000, CRC(7e5b0f27) SHA1(9e8d69f34c30216925fcb7af87f8b37f703317e7) )
	ROM_LOAD( "a05703w064.bin",   0x1000000, 0x0800000, CRC(accbdb44) SHA1(d59b2452c7a5b4e666473dc973b73a0f2b4edc13) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b05701w064.bin",   0x0000000, 0x0800000, CRC(a20cdcef) SHA1(029a49971adf1e72ab556a207172bdfbd0b86b03) )
	ROM_LOAD( "b05702w016.bin",   0x0800000, 0x0200000, CRC(fe7457df) SHA1(d66b1b31102b0210f9faf40e1473cd1511ccaf1f) )

	ROM_REGION( 0x800000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "w05701b032.bin",   0x400000, 0x400000, CRC(2d3ae593) SHA1(b9c1d2994be95ba974bc134a3bf115bc9c9c9c16) )
ROM_END

ROM_START( svg )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "u30.bin",      0x100000, 0x080000, CRC(34c18f3f) SHA1(42d1edd0dcfaa5e44861c6a1d4cb24f51ba23de8) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "svg_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0x800000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "u26.bin", 0x000000, 0x400000, CRC(46826ec8) SHA1(ad1daf6f615fb8d748ce7f98f19dd3bf22f79fba) )
	ROM_LOAD( "u29.bin", 0x400000, 0x400000, CRC(fa5f3901) SHA1(8ab7c6763df4f752b50ed2197063f58046b32ddb) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t05601w016.bin",0x180000, 0x200000, CRC(03e110dc) SHA1(41c8f286e9303b24ba6235b341371c298226fb6a) )

	ROM_REGION( 0x1c00000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a05601w064.bin",  0x0000000, 0x0800000, CRC(ea6453e4) SHA1(b5c82edafa8008ad59b5f2219511947d078d446e) )
	ROM_LOAD( "a05602w064.bin",  0x0800000, 0x0800000, CRC(6d00621b) SHA1(55a4bc357e14b975b0234a9cd49e2224f509dad9) )
	ROM_LOAD( "a05603w064.bin",  0x1000000, 0x0800000, CRC(7b71c64f) SHA1(bec7c7edf0634cf8351a54abb867c56af08ad2c3) )
	ROM_LOAD( "a05604w032.bin",  0x1800000, 0x0400000, CRC(9452a567) SHA1(01fdb8e1f131603843ef4c49ab76d7a56b2d6414) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b05601w064.bin",  0x0000000, 0x0800000, CRC(35c0a489) SHA1(a7d5527da01f8eaa7499fb6785b57094521bce97) )
	ROM_LOAD( "b05602w064.bin",  0x0800000, 0x0800000, CRC(8aad3f85) SHA1(da6996d901d42b3a2ba7019ad014bb938a5e328b) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "w05601b064.bin",  0x400000, 0x800000, CRC(bfe61a71) SHA1(e682ca8d57ca51c4d72f64fc091161f3dbdce871) )
	ROM_LOAD( "w05602b032.bin",  0xc00000, 0x400000, CRC(0685166d) SHA1(64dac49abd2a46d5fb58c678027aa5e23d672dc4) )
ROM_END

ROM_START( happy6 )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	PGM_68K_BIOS
	ROM_LOAD16_WORD_SWAP( "happy6in1_v100cn.u5",      0x100000, 0x080000, CRC(a25418e8) SHA1(acd7e7b69956cb4ce8e26c6420cb97bb4bf404e7) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	// data before 0x188 is read-protected and cannot be read even with a trojan (as with most 2001/2+ IGS titles)
	ROM_LOAD( "happy6_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION( 0x800000, "user1", 0 ) /* Protection Data (encrypted external ARM data) */
	ROM_LOAD( "happy6in1_v101cn.u26", 0x000000, 0x400000, CRC(4a48ca1c) SHA1(3bebc091787903d45cb84c7302046602a903f59c) )

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	PGM_VIDEO_BIOS
	ROM_LOAD( "t01w64m.u29",0x180000, 0x800000, CRC(2d3feb8b) SHA1(9832b1c46b1ee73febf5c5c8913859f4e0581665) )

	ROM_REGION( 0x1c00000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a01w64m.u5",  0x0000000, 0x0800000, CRC(bbaa3df3) SHA1(a72268d3989e96d571242279922291d0dc72db28) )
	ROM_LOAD( "a02w64m.u6",  0x0800000, 0x0800000, CRC(f8c9cd36) SHA1(d9613a83bcc2364492fa922fde1c4f0d07b3009c) )

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b01w64m.u19",  0x0000000, 0x0800000, CRC(73f5f225) SHA1(507126fa96dcec967bdbc0978d79fbce9d25db37) )

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	PGM_AUDIO_BIOS
	ROM_LOAD( "w01w64m.u17",  0x400000, 0x800000, CRC(7e23e2be) SHA1(5e920b8e480f00b6666292d4a56039b40af18141) )
ROM_END



/* all known revisions of ketsui have roms marked v100, even when the actual game revision is upgraded */

ROM_START( ket )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	/* doesn't use a separate BIOS rom */
	ROM_LOAD16_WORD_SWAP( "ketsui_v100.u38", 0x000000, 0x200000, CRC(dfe62f3b) SHA1(baa58d1ce47a707f84f65779ac0689894793e9d9) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "ket_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION32_LE( 0x400000, "user1", ROMREGION_ERASE00 )
	/* no external protection rom */

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // same as standard PGM text bios - surface scratched to remove details
	ROM_LOAD( "t04701w064.u19",   0x180000, 0x800000, CRC(2665b041) SHA1(fb1107778b66f2af0de77ac82e1ee2902f53a959) ) //text-1

	ROM_REGION( 0x1000000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a04701w064.u7",    0x0000000, 0x0800000, CRC(5ef1b94b) SHA1(f10dfa46e0a4d297c3a856aea5b49d648f98935c) ) //image-1
	ROM_LOAD( "a04702w064.u8",    0x0800000, 0x0800000, CRC(26d6da7f) SHA1(f20e07a7994f41b5ed917f8b0119dc5542f3541c) ) //image-2

	ROM_REGION( 0x0800000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b04701w064.u1",    0x0000000, 0x0800000, CRC(1bec008d) SHA1(07d117dc2eebb35727fb18a7c563acbaf25a8d36) ) //bitmap-1

	ROM_REGION( 0x800000, "ics", ROMREGION_ERASE00 ) /* Samples - (8 bit mono 11025Hz) - */
	/* there is a position for the PGM audio bios rom, but it's unpopulated, and the M of PGM has been scratched off the PCB */
	ROM_LOAD( "m04701b032.u17",    0x400000, 0x400000, CRC(b46e22d1) SHA1(670853dc485942fb96380568494bdf3235f446ee) ) //music-1

	ROM_REGION( 0x20000, "sram", 0 ) /* default settings */
	ROM_LOAD( "ket_defaults.nv",  0x0000000, 0x020000, CRC(3ca892d8) SHA1(67430df5217e453ae8140c5653deeadfad8fa684) )
ROM_END

ROM_START( keta )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	/* doesn't use a separate BIOS rom */
	ROM_LOAD16_WORD_SWAP( "ketsui_prg_revised.bin", 0x000000, 0x200000, CRC(69fcf5eb) SHA1(f726e251b4daa2f8d717e32000d4d7abc71c710d) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "ket_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION32_LE( 0x400000, "user1", ROMREGION_ERASE00 )
	/* no external protection rom */

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // same as standard PGM text bios - surface scratched to remove details
	ROM_LOAD( "t04701w064.u19",   0x180000, 0x800000, CRC(2665b041) SHA1(fb1107778b66f2af0de77ac82e1ee2902f53a959) ) //text-1

	ROM_REGION( 0x1000000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a04701w064.u7",    0x0000000, 0x0800000, CRC(5ef1b94b) SHA1(f10dfa46e0a4d297c3a856aea5b49d648f98935c) ) //image-1
	ROM_LOAD( "a04702w064.u8",    0x0800000, 0x0800000, CRC(26d6da7f) SHA1(f20e07a7994f41b5ed917f8b0119dc5542f3541c) ) //image-2

	ROM_REGION( 0x0800000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b04701w064.u1",    0x0000000, 0x0800000, CRC(1bec008d) SHA1(07d117dc2eebb35727fb18a7c563acbaf25a8d36) ) //bitmap-1

	ROM_REGION( 0x800000, "ics", ROMREGION_ERASE00 ) /* Samples - (8 bit mono 11025Hz) - */
	/* there is a position for the PGM audio bios rom, but it's unpopulated, and the M of PGM has been scratched off the PCB */
	ROM_LOAD( "m04701b032.u17",    0x400000, 0x400000, CRC(b46e22d1) SHA1(670853dc485942fb96380568494bdf3235f446ee) ) //music-1

	ROM_REGION( 0x20000, "sram", 0 ) /* default settings */
	ROM_LOAD( "ket_defaults.nv",  0x0000000, 0x020000, CRC(3ca892d8) SHA1(67430df5217e453ae8140c5653deeadfad8fa684) )
ROM_END

ROM_START( ketb )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	/* doesn't use a separate BIOS rom */
	ROM_LOAD16_WORD_SWAP( "ketsui_prg_original.bin", 0x000000, 0x200000, CRC(cca5e153) SHA1(b653feaa2004c379312def6b1613c3497f654ddf) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "ket_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION32_LE( 0x400000, "user1", ROMREGION_ERASE00 )
	/* no external protection rom */

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // same as standard PGM text bios - surface scratched to remove details
	ROM_LOAD( "t04701w064.u19",   0x180000, 0x800000, CRC(2665b041) SHA1(fb1107778b66f2af0de77ac82e1ee2902f53a959) ) //text-1

	ROM_REGION( 0x1000000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a04701w064.u7",    0x0000000, 0x0800000, CRC(5ef1b94b) SHA1(f10dfa46e0a4d297c3a856aea5b49d648f98935c) ) //image-1
	ROM_LOAD( "a04702w064.u8",    0x0800000, 0x0800000, CRC(26d6da7f) SHA1(f20e07a7994f41b5ed917f8b0119dc5542f3541c) ) //image-2

	ROM_REGION( 0x0800000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b04701w064.u1",    0x0000000, 0x0800000, CRC(1bec008d) SHA1(07d117dc2eebb35727fb18a7c563acbaf25a8d36) ) //bitmap-1

	ROM_REGION( 0x800000, "ics", ROMREGION_ERASE00 ) /* Samples - (8 bit mono 11025Hz) - */
	/* there is a position for the PGM audio bios rom, but it's unpopulated, and the M of PGM has been scratched off the PCB */
	ROM_LOAD( "m04701b032.u17",    0x400000, 0x400000, CRC(b46e22d1) SHA1(670853dc485942fb96380568494bdf3235f446ee) ) //music-1

	ROM_REGION( 0x20000, "sram", 0 ) /* default settings */
	ROM_LOAD( "ket_defaults.nv",  0x0000000, 0x020000, CRC(3ca892d8) SHA1(67430df5217e453ae8140c5653deeadfad8fa684) )
ROM_END

ROM_START( espgal )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	/* doesn't use a separate BIOS rom */
	ROM_LOAD16_WORD_SWAP( "espgaluda_v100.u38", 0x000000, 0x200000, CRC(08ecec34) SHA1(bce2e7fb9105ed51603d09cbd3a9eeb5b8f47ee2) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "espgal_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION32_LE( 0x400000, "user1", ROMREGION_ERASE00 )
	/* no external protection rom */

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "t01s.u18", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // same as standard PGM text bios - surface scratched to remove details
	ROM_LOAD( "t04801w064.u19",   0x180000, 0x800000, CRC(6021c79e) SHA1(fbc340dafb18aa3094de29b881318a5a9794e4bc) ) //text-1

	ROM_REGION( 0x1000000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a04801w064.u7",    0x0000000, 0x0800000, CRC(26dd4932) SHA1(9bbabb5a53cb5ba88397cc2c258980f3b70314ce) ) //image-1
	ROM_LOAD( "a04802w064.u8",    0x0800000, 0x0800000, CRC(0e6bf7a9) SHA1(a7541e2b5a0df2bc62a5b347e54dbc2ed1922db2) ) //image-2

	ROM_REGION( 0x0800000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b04801w064.u1",    0x0000000, 0x0800000, CRC(98dce13a) SHA1(61d48b7117459f7babc022b68231f6928177a71d) ) //bitmap-1

	ROM_REGION( 0x800000, "ics", ROMREGION_ERASE00 ) /* Samples - (8 bit mono 11025Hz) - */
	/* there is a position for the PGM audio bios rom, but it's unpopulated, and the M of PGM has been scratched off the PCB */
	ROM_LOAD( "w04801b032.u17",    0x400000, 0x400000, CRC(60298536) SHA1(6b7333f16cce778c5725dbdf75a5446f0906397a) ) //music-1
ROM_END

ROM_START( ddpdoj )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "ddp3_bios.u37",    0x00000, 0x080000, CRC(b3cc5c8f) SHA1(02d9511cf71e4a0d6ca8fd9a1ef2c79b0d001824) ) // uses a standard PGM bios with the startup logos hacked out
	ROM_LOAD16_WORD_SWAP( "ddp3_v101.u36",  0x100000, 0x200000, CRC(195b5c1e) SHA1(f18d791c034b0a3d85888a92fb5d326ee3deb04f) ) // yes this one was actually marked v101 which goes against the standard Cave marking system

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "ddp3_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION32_LE( 0x400000, "user1", ROMREGION_ERASE00 )
	/* no external protection rom */

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // same as standard PGM bios
	ROM_LOAD( "t04401w064.u19",0x180000, 0x800000, CRC(3a95f19c) SHA1(fd3c47cf0b8b1e20c6bec4be68a089fc8bbf4dbe) ) //text-1

	ROM_REGION( 0x1c00000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a04401w064.u7",  0x0000000, 0x0800000, CRC(ed229794) SHA1(1cf1863495a18c7c7d277a9be43ec116b00960b0) ) //image-1
	ROM_LOAD( "a04402w064.u8",  0x0800000, 0x0800000, CRC(752167b0) SHA1(c33c3398dd8e479c9d5bd348924958a6aecbf0fc) ) //image-2

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b04401w064.u1",  0x0000000, 0x0800000, CRC(8cbff066) SHA1(eef1cd566bc70ebf45f047e56026803d5c1dac43) ) //bitmap-1

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // same as standard PGM bios
	ROM_LOAD( "m04401b032.u17",  0x400000, 0x400000, CRC(5a0dbd76) SHA1(06ab202f6bd5ebfb35b9d8cc7a8fb83ec8840659) ) //music-1

	ROM_REGION( 0x20000, "sram", 0 ) /* default settings */
	ROM_LOAD( "ddp3_defaults.nv",  0x0000000, 0x020000, CRC(571e96c0) SHA1(348940c77ca348213331b85b9b1d3aabb96a528a) )
ROM_END


ROM_START( ddpdoja )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "ddp3_bios.u37",    0x00000, 0x080000, CRC(b3cc5c8f) SHA1(02d9511cf71e4a0d6ca8fd9a1ef2c79b0d001824) ) // uses a standard PGM bios with the startup logos hacked out
	ROM_LOAD16_WORD_SWAP( "ddp3_d_d_1_0.u36",  0x100000, 0x200000, CRC(5d3f85ba) SHA1(4c24ea206140863d456179750366921442e1d2b8) ) // marked v100

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "ddp3_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION32_LE( 0x400000, "user1", ROMREGION_ERASE00 )
	/* no external protection rom */

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // same as standard PGM bios
	ROM_LOAD( "t04401w064.u19",0x180000, 0x800000, CRC(3a95f19c) SHA1(fd3c47cf0b8b1e20c6bec4be68a089fc8bbf4dbe) ) //text-1

	ROM_REGION( 0x1c00000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a04401w064.u7",  0x0000000, 0x0800000, CRC(ed229794) SHA1(1cf1863495a18c7c7d277a9be43ec116b00960b0) ) //image-1
	ROM_LOAD( "a04402w064.u8",  0x0800000, 0x0800000, CRC(752167b0) SHA1(c33c3398dd8e479c9d5bd348924958a6aecbf0fc) ) //image-2

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b04401w064.u1",  0x0000000, 0x0800000, CRC(8cbff066) SHA1(eef1cd566bc70ebf45f047e56026803d5c1dac43) ) //bitmap-1

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // same as standard PGM bios
	ROM_LOAD( "m04401b032.u17",  0x400000, 0x400000, CRC(5a0dbd76) SHA1(06ab202f6bd5ebfb35b9d8cc7a8fb83ec8840659) ) //music-1

	ROM_REGION( 0x20000, "sram", 0 ) /* default settings */
	ROM_LOAD( "ddp3_defaults.nv",  0x0000000, 0x020000, CRC(571e96c0) SHA1(348940c77ca348213331b85b9b1d3aabb96a528a) )
ROM_END

ROM_START( ddpdojb )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "ddp3_bios.u37",    0x00000, 0x080000, CRC(b3cc5c8f) SHA1(02d9511cf71e4a0d6ca8fd9a1ef2c79b0d001824) ) // uses a standard PGM bios with the startup logos hacked out
	ROM_LOAD16_WORD_SWAP( "dd v100.bin",  0x100000, 0x200000, CRC(7da0c1e4) SHA1(aca2fe35ba0ab3628900fa2aba2d22fc4fd7046d) ) // marked v100

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "ddp3_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION32_LE( 0x400000, "user1", ROMREGION_ERASE00 )
	/* no external protection rom */

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // same as standard PGM bios
	ROM_LOAD( "t04401w064.u19",0x180000, 0x800000, CRC(3a95f19c) SHA1(fd3c47cf0b8b1e20c6bec4be68a089fc8bbf4dbe) ) //text-1

	ROM_REGION( 0x1c00000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a04401w064.u7",  0x0000000, 0x0800000, CRC(ed229794) SHA1(1cf1863495a18c7c7d277a9be43ec116b00960b0) ) //image-1
	ROM_LOAD( "a04402w064.u8",  0x0800000, 0x0800000, CRC(752167b0) SHA1(c33c3398dd8e479c9d5bd348924958a6aecbf0fc) ) //image-2

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b04401w064.u1",  0x0000000, 0x0800000, CRC(8cbff066) SHA1(eef1cd566bc70ebf45f047e56026803d5c1dac43) ) //bitmap-1

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // same as standard PGM bios
	ROM_LOAD( "m04401b032.u17",  0x400000, 0x400000, CRC(5a0dbd76) SHA1(06ab202f6bd5ebfb35b9d8cc7a8fb83ec8840659) ) //music-1

	ROM_REGION( 0x20000, "sram", 0 ) /* default settings */
	ROM_LOAD( "ddp3_defaults.nv",  0x0000000, 0x020000, CRC(571e96c0) SHA1(348940c77ca348213331b85b9b1d3aabb96a528a) )
ROM_END

/* this expects Magic values in NVRAM to boot */
ROM_START( ddpdojblk )
	ROM_REGION( 0x600000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "ddp3_bios.u37",    0x00000, 0x080000, CRC(b3cc5c8f) SHA1(02d9511cf71e4a0d6ca8fd9a1ef2c79b0d001824) ) // uses a standard PGM bios with the startup logos hacked out
	ROM_LOAD16_WORD_SWAP( "ddb10.u45",  0x100000, 0x200000, CRC(72b35510) SHA1(9a432e5e1ebe61aafd737b6acc905653e5af0d38) )

	ROM_REGION( 0x4000, "prot", 0 ) /* ARM protection ASIC - internal rom */
	ROM_LOAD( "ddp3_igs027a.bin", 0x000000, 0x04000, NO_DUMP )

	ROM_REGION32_LE( 0x400000, "user1", ROMREGION_ERASE00 )
	/* no external protection rom */

	ROM_REGION( 0xc00000, "tiles", 0 ) /* 8x8 Text Tiles + 32x32 BG Tiles */
	ROM_LOAD( "pgm_t01s.rom", 0x000000, 0x200000, CRC(1a7123a0) SHA1(cc567f577bfbf45427b54d6695b11b74f2578af3) ) // same as standard PGM bios
	ROM_LOAD( "t04401w064.u19",0x180000, 0x800000, CRC(3a95f19c) SHA1(fd3c47cf0b8b1e20c6bec4be68a089fc8bbf4dbe) ) //text-1

	ROM_REGION( 0x1c00000, "sprcol", 0 ) /* Sprite Colour Data */
	ROM_LOAD( "a04401w064.u7",  0x0000000, 0x0800000, CRC(ed229794) SHA1(1cf1863495a18c7c7d277a9be43ec116b00960b0) ) //image-1
	ROM_LOAD( "a04402w064.u8",  0x0800000, 0x0800000, CRC(752167b0) SHA1(c33c3398dd8e479c9d5bd348924958a6aecbf0fc) ) //image-2

	ROM_REGION( 0x1000000, "sprmask", 0 ) /* Sprite Masks + Colour Indexes */
	ROM_LOAD( "b04401w064.u1",  0x0000000, 0x0800000, CRC(8cbff066) SHA1(eef1cd566bc70ebf45f047e56026803d5c1dac43) ) //bitmap-1

	ROM_REGION( 0x1000000, "ics", 0 ) /* Samples - (8 bit mono 11025Hz) - */
	ROM_LOAD( "pgm_m01s.rom", 0x000000, 0x200000, CRC(45ae7159) SHA1(d3ed3ff3464557fd0df6b069b2e431528b0ebfa8) ) // same as standard PGM bios
	ROM_LOAD( "m04401b032.u17",  0x400000, 0x400000, CRC(5a0dbd76) SHA1(06ab202f6bd5ebfb35b9d8cc7a8fb83ec8840659) ) //music-1

	ROM_REGION( 0x20000, "sram", 0 ) /* NVRAM with factory programmed values - needed to boot */
	ROM_LOAD( "ddp3blk_defaults.nv",  0x0000000, 0x020000, CRC(a1651904) SHA1(5b80d3c4c764895c40953a66161d4dd84f742604) )
ROM_END


/*** Init Stuff **************************************************************/

/* This function expands the 32x32 5-bit data into a format which is easier to
   decode in MAME */

static void expand_32x32x5bpp(running_machine &machine)
{
	UINT8 *src = machine.region( "tiles" )->base();
	gfx_layout glcopy;
	glcopy = *(&pgm32_charlayout);

	size_t  srcsize = machine.region( "tiles" )->bytes();
	int cnt, pix;
	size_t gfx2_size_needed = ((srcsize/5)*8)+0x1000;
	UINT8 *dst = auto_alloc_array(machine, UINT8, gfx2_size_needed);


	for (cnt = 0; cnt < srcsize/5 ; cnt ++)
	{
		pix = ((src[0 + 5 * cnt] >> 0)& 0x1f );
			dst[0 + 8 * cnt]=pix;
		pix = ((src[0 + 5 * cnt] >> 5)& 0x07) | ((src[1 + 5 * cnt] << 3) & 0x18);
			dst[1 + 8 * cnt]=pix;
		pix = ((src[1 + 5 * cnt] >> 2)& 0x1f );
			dst[2 + 8 * cnt]=pix;
		pix = ((src[1 + 5 * cnt] >> 7)& 0x01) | ((src[2 + 5 * cnt] << 1) & 0x1e);
			dst[3 + 8 * cnt]=pix;
		pix = ((src[2 + 5 * cnt] >> 4)& 0x0f) | ((src[3 + 5 * cnt] << 4) & 0x10);
			dst[4 + 8 * cnt]=pix;
		pix = ((src[3 + 5 * cnt] >> 1)& 0x1f );
			dst[5 + 8 * cnt]=pix;
		pix = ((src[3 + 5 * cnt] >> 6)& 0x03) | ((src[4 + 5 * cnt] << 2) & 0x1c);
			dst[6 + 8 * cnt]=pix;
		pix = ((src[4 + 5 * cnt] >> 3)& 0x1f );
			dst[7 + 8 * cnt]=pix;
	}

	glcopy.total = (gfx2_size_needed / glcopy.charincrement)*8;

	machine.gfx[1] = gfx_element_alloc(machine, &glcopy, (UINT8 *)dst, 32, 0x400);


}

/* This function expands the sprite colour data (in the A Roms) from 3 pixels
   in each word to a byte per pixel making it easier to use */

static void expand_colourdata( running_machine &machine )
{
	pgm_state *state = machine.driver_data<pgm_state>();
	UINT8 *src = machine.region( "sprcol" )->base();
	size_t srcsize = machine.region( "sprcol" )->bytes();
	int cnt;
	size_t needed = srcsize / 2 * 3;

	/* work out how much ram we need to allocate to expand the sprites into
       and be able to mask the offset */
	state->m_sprite_a_region_size = 1;
	while (state->m_sprite_a_region_size < needed)
		state->m_sprite_a_region_size <<= 1;

	state->m_sprite_a_region = auto_alloc_array(machine, UINT8, state->m_sprite_a_region_size);

	for (cnt = 0 ; cnt < srcsize / 2 ; cnt++)
	{
		UINT16 colpack;

		colpack = ((src[cnt * 2]) | (src[cnt * 2 + 1] << 8));
		state->m_sprite_a_region[cnt * 3 + 0] = (colpack >> 0 ) & 0x1f;
		state->m_sprite_a_region[cnt * 3 + 1] = (colpack >> 5 ) & 0x1f;
		state->m_sprite_a_region[cnt * 3 + 2] = (colpack >> 10) & 0x1f;
	}
}





static void pgm_basic_init( running_machine &machine, bool set_bank = true )
{
	pgm_state *state = machine.driver_data<pgm_state>();

	UINT8 *ROM = machine.region("maincpu")->base();
	if (set_bank) memory_set_bankptr(machine, "bank1", &ROM[0x100000]);

	expand_32x32x5bpp(machine);
	expand_colourdata(machine);

	state->m_bg_videoram = &state->m_videoram[0];
	state->m_tx_videoram = &state->m_videoram[0x4000/2];
	state->m_rowscrollram = &state->m_videoram[0x7000/2];
}

static DRIVER_INIT( pgm )
{
	pgm_basic_init(machine);
}

/* Oriental Legend INIT */

static DRIVER_INIT( orlegend )
{
	pgm_state *state = machine.driver_data<pgm_state>();
	pgm_basic_init(machine);

	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0xC0400e, 0xC0400f, FUNC(pgm_asic3_r), FUNC(pgm_asic3_w));
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0xC04000, 0xC04001, FUNC(pgm_asic3_reg_w));

	state->m_asic3_reg = 0;
	state->m_asic3_latch[0] = 0;
	state->m_asic3_latch[1] = 0;
	state->m_asic3_latch[2] = 0;
	state->m_asic3_x = 0;
	state->m_asic3_y = 0;
	state->m_asic3_z = 0;
	state->m_asic3_h1 = 0;
	state->m_asic3_h2 = 0;
	state->m_asic3_hold = 0;

	state->save_item(NAME(state->m_asic3_reg));
	state->save_item(NAME(state->m_asic3_latch));
	state->save_item(NAME(state->m_asic3_x));
	state->save_item(NAME(state->m_asic3_y));
	state->save_item(NAME(state->m_asic3_z));
	state->save_item(NAME(state->m_asic3_h1));
	state->save_item(NAME(state->m_asic3_h2));
	state->save_item(NAME(state->m_asic3_hold));
}

static void drgwld2_common_init(running_machine &machine)
{
	pgm_basic_init(machine);
	pgm_dw2_decrypt(machine);
	/*
    Info from Elsemi
    Here is how to "bypass" the dw2 hang protection, it fixes the mode
    select and after failing in the 2nd stage (probably there are other checks
    out there).
    */
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0xd80000, 0xd80003, FUNC(dw2_d80000_r));
}

static DRIVER_INIT( drgw2 )
{	/* incomplete? */
	UINT16 *mem16 = (UINT16 *)machine.region("maincpu")->base();
	drgwld2_common_init(machine);
	/* These ROM patches are not hacks, the protection device
       overlays the normal ROM code, this has been confirmed on a real PCB
       although some addresses may be missing */
	mem16[0x131098 / 2] = 0x4e93;
	mem16[0x13113e / 2] = 0x4e93;
	mem16[0x1311ce / 2] = 0x4e93;
}

static DRIVER_INIT( dw2v100x )
{
	UINT16 *mem16 = (UINT16 *)machine.region("maincpu")->base();
	drgwld2_common_init(machine);

	mem16[0x131084 / 2] = 0x4e93;
	mem16[(0x131084+0xa6) / 2] = 0x4e93;
	mem16[(0x131084+0x136) / 2] = 0x4e93;
}

static DRIVER_INIT( drgw2c )
{
	UINT16 *mem16 = (UINT16 *)machine.region("maincpu")->base();
	drgwld2_common_init(machine);
	/* These ROM patches are not hacks, the protection device
       overlays the normal ROM code, this has been confirmed on a real PCB
       although some addresses may be missing */
	mem16[0x1303bc / 2] = 0x4e93;
	mem16[0x130462 / 2] = 0x4e93;
	mem16[0x1304f2 / 2] = 0x4e93;
}

static DRIVER_INIT( drgw2j )
{
	UINT16 *mem16 = (UINT16 *)machine.region("maincpu")->base();
	drgwld2_common_init(machine);
	/* These ROM patches are not hacks, the protection device
       overlays the normal ROM code, this has been confirmed on a real PCB
       although some addresses may be missing */
	mem16[0x1302c0 / 2] = 0x4e93;
	mem16[0x130366 / 2] = 0x4e93;
	mem16[0x1303f6 / 2] = 0x4e93;
}

static void kovsh_latch_init( running_machine &machine )
{
	pgm_state *state = machine.driver_data<pgm_state>();

	state->m_kovsh_highlatch_arm_w = 0;
	state->m_kovsh_lowlatch_arm_w = 0;
	state->m_kovsh_highlatch_68k_w = 0;
	state->m_kovsh_lowlatch_68k_w = 0;
	state->m_kovsh_counter = 1;

	state->save_item(NAME(state->m_kovsh_highlatch_arm_w));
	state->save_item(NAME(state->m_kovsh_lowlatch_arm_w));
	state->save_item(NAME(state->m_kovsh_highlatch_68k_w));
	state->save_item(NAME(state->m_kovsh_lowlatch_68k_w));
	state->save_item(NAME(state->m_kovsh_counter));
}

static DRIVER_INIT( kov )
{
	pgm_basic_init(machine);
	pgm_kov_decrypt(machine);
	kovsh_latch_init(machine);
	install_protection_asic_sim_kov(machine);
}

static DRIVER_INIT( kovboot )
{
	pgm_basic_init(machine);
//  pgm_kov_decrypt(machine);
	kovsh_latch_init(machine);
	install_protection_asic_sim_kov(machine);
}

static DRIVER_INIT( pstar )
{
	pgm_state *state = machine.driver_data<pgm_state>();

	pgm_basic_init(machine);
	pgm_pstar_decrypt(machine);
	kovsh_latch_init(machine);

	state->m_pstars_key = 0;
	state->m_pstars_int[0] = 0;
	state->m_pstars_int[1] = 0;
	state->m_pstars_val = 0;
	state->m_pstar_e7 = 0;
	state->m_pstar_b1 = 0;
	state->m_pstar_ce = 0;
	state->m_pstar_ram[0] = 0;
	state->m_pstar_ram[1] = 0;
	state->m_pstar_ram[2] = 0;
	memset(state->m_pstars_regs, 0, 16);

	state->save_item(NAME(state->m_pstars_key));
	state->save_item(NAME(state->m_pstars_int));
	state->save_item(NAME(state->m_pstars_regs));
	state->save_item(NAME(state->m_pstars_val));
	state->save_item(NAME(state->m_pstar_e7));
	state->save_item(NAME(state->m_pstar_b1));
	state->save_item(NAME(state->m_pstar_ce));
	state->save_item(NAME(state->m_pstar_ram));

}

static DRIVER_INIT( photoy2k )
{
	pgm_basic_init(machine);
	pgm_photoy2k_decrypt(machine);
	kovsh_latch_init(machine);
}

static DRIVER_INIT( py2k2 )
{
	pgm_basic_init(machine);
	pgm_py2k2_decrypt(machine);
	kovsh_latch_init(machine);
}


static DRIVER_INIT( kovsh )
{
	pgm_basic_init(machine);
	pgm_kovsh_decrypt(machine);
	kovsh_latch_init(machine);
}

static DRIVER_INIT( kovshp )
{
	pgm_basic_init(machine);
	pgm_kovshp_decrypt(machine);
	kovsh_latch_init(machine);
}


static void kov2_latch_init( running_machine &machine )
{
	pgm_state *state = machine.driver_data<pgm_state>();

	state->m_kov2_latchdata_68k_w = 0;
	state->m_kov2_latchdata_arm_w = 0;

	state->save_item(NAME(state->m_kov2_latchdata_68k_w));
	state->save_item(NAME(state->m_kov2_latchdata_arm_w));
}

static DRIVER_INIT( kov2 )
{
	pgm_basic_init(machine);
	pgm_kov2_decrypt(machine);
	kov2_latch_init(machine);
}


static DRIVER_INIT( martmast )
{
	pgm_basic_init(machine);
	pgm_mm_decrypt(machine);
	kov2_latch_init(machine);
}


static UINT16 *ddp2_protram;
static int ddp2_asic27_0xd10000 = 0;

// ARM comms latch
static WRITE16_HANDLER ( ddp2_asic27_0xd10000_w )
{
	//int pc = cpu_get_pc(&space->device());

	//logerror("%06x: ddp2_asic27_0xd10000_w %04x, %04x\n", pc, offset*2,data);

	ddp2_asic27_0xd10000=data;
}

// ARM comms latch
static READ16_HANDLER ( ddp2_asic27_0xd10000_r )
{
	//int pc = cpu_get_pc(&space->device());

	//logerror("%06x: d100000_prot_r %04x, %04x\n", pc, offset*2,ddp2_asic27_0xd10000);

	ddp2_asic27_0xd10000++;
	ddp2_asic27_0xd10000&=0x7f;
	return ddp2_asic27_0xd10000;
}

// Shared with ARM
static READ16_HANDLER(ddp2_protram_r)
{
	//int pc = cpu_get_pc(&space->device());

	//logerror("%06x prot_r %04x, %04x\n", pc, offset*2,ddp2_protram[offset]);

	if (offset == 0x02/2) return input_port_read(space->machine(), "Region");

	if (offset == 0x1f00/2) return 0;

	return ddp2_protram[offset];
}

// Shared with ARM
static WRITE16_HANDLER(ddp2_protram_w)
{
	//int pc = cpu_get_pc(&space->device());

	//logerror("%06x: prot_w %04x, %02x\n", pc, offset*2,data);

	COMBINE_DATA(&ddp2_protram[offset]);

	ddp2_protram[0x10/2] = 0;
	ddp2_protram[0x20/2] = 1;
}

static DRIVER_INIT( ddp2 )
{
	pgm_basic_init(machine);
	pgm_ddp2_decrypt(machine);
	//kov2_latch_init(machine);

	// should actually be kov2-like, but keep this simulation for now just to demonstrate it.  It will need the internal ARM rom to work properly.
	ddp2_protram = auto_alloc_array(machine, UINT16, 0x10000);
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0xd00000, 0xd0ffff, FUNC(ddp2_protram_r), FUNC(ddp2_protram_w));
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0xd10000, 0xd10001, FUNC(ddp2_asic27_0xd10000_r), FUNC(ddp2_asic27_0xd10000_w));

}

static void svg_basic_init(running_machine &machine)
{
	pgm_state *state = machine.driver_data<pgm_state>();

	pgm_basic_init(machine);
	state->m_svg_shareram[0] = auto_alloc_array(machine, UINT32, 0x10000 / 4);
	state->m_svg_shareram[1] = auto_alloc_array(machine, UINT32, 0x10000 / 4);
	state->m_svg_ram_sel = 0;

	state->save_pointer(NAME(state->m_svg_shareram[0]), 0x10000 / 4);
	state->save_pointer(NAME(state->m_svg_shareram[1]), 0x10000 / 4);
	state->save_item(NAME(state->m_svg_ram_sel));
}

static DRIVER_INIT( theglad )
{
	svg_basic_init(machine);
	pgm_theglad_decrypt(machine);
	kov2_latch_init(machine);
}

static DRIVER_INIT( svg )
{
	svg_basic_init(machine);
	pgm_svg_decrypt(machine);
	kov2_latch_init(machine);
}

static DRIVER_INIT( killbldp )
{
	svg_basic_init(machine);
	pgm_killbldp_decrypt(machine);
	kov2_latch_init(machine);
}

static DRIVER_INIT( dmnfrnt )
{
	svg_basic_init(machine);
	pgm_dfront_decrypt(machine);
	kov2_latch_init(machine);
}


/* The IGS022 is an MCU which performs encrypted DMA used by
 - The Killing Blade
 - Dragon World 3

 There is also an automatic transfer which happens on startup using params stored in the data ROM.
 This has been verified on real hardware running without any 68k game program.

*/


static void IGS022_do_dma(running_machine& machine, UINT16 src, UINT16 dst, UINT16 size, UINT16 mode)
{
	pgm_state *state = machine.driver_data<pgm_state>();
	UINT16 param;
	/*
    P_SRC =0x300290 (offset from prot rom base)
    P_DST =0x300292 (words from 0x300000)
    P_SIZE=0x300294 (words)
    P_MODE=0x300296

    Mode 5 direct
    Mode 6 swap nibbles and bytes

    1,2,3 table based ops
    */

	//mame_printf_debug("src %04x dst %04x size %04x mode %04x\n", src, dst, size, mode);

	//if (src&1) mame_printf_debug("odd offset\n");

	param = mode >> 8;
	mode &=0xf;  // what are the other bits?


	if ((mode == 0) || (mode == 1) || (mode == 2) || (mode == 3))
	{
		/* mode3 applies a xor from a 0x100 byte table to the data being
           transferred

           the table is stored at the start of the protection rom.

           the param used with the mode gives a start offset into the table

           odd offsets seem to change the table slightly (see rawDataOdd)

       */

		/*
        unsigned char rawDataOdd[256] = {
            0xB6, 0xA8, 0xB1, 0x5D, 0x2C, 0x5D, 0x4F, 0xC1,
            0xCF, 0x39, 0x3A, 0xB7, 0x65, 0x85, 0xD9, 0xEE,
            0xDB, 0x7B, 0x5F, 0x81, 0x03, 0x6D, 0xEB, 0x07,
            0x0F, 0xB5, 0x61, 0x59, 0xCD, 0x60, 0x06, 0x21,
            0xA0, 0x99, 0xDD, 0x27, 0x42, 0xD7, 0xC5, 0x5B,
            0x3B, 0xC6, 0x4F, 0xA2, 0x20, 0xF6, 0x61, 0x61,
            0x8C, 0x46, 0x8C, 0xCA, 0xE0, 0x0E, 0x2C, 0xE9,
            0xBA, 0x0F, 0x45, 0x6D, 0x36, 0x1C, 0x18, 0x37,
            0xE7, 0x85, 0x89, 0xA4, 0x94, 0x46, 0x30, 0x9B,
            0xB2, 0xF4, 0x41, 0x55, 0xA5, 0x63, 0x1C, 0xEF,
            0xB7, 0x18, 0xB3, 0xB1, 0xD4, 0x72, 0xA0, 0x1C,
            0x0B, 0x97, 0x02, 0xB6, 0xC5, 0x1F, 0x1B, 0x94,
            0xC3, 0x83, 0xAA, 0xAC, 0xD9, 0x44, 0x09, 0xD7,
            0x6C, 0xDB, 0x07, 0xA9, 0xAD, 0x64, 0x83, 0xF1,
            0x92, 0x09, 0xCD, 0x0E, 0x99, 0x2F, 0xBC, 0xF8,
            0x3C, 0x63, 0x8F, 0x0A, 0x33, 0x03, 0x84, 0x91,
            0x6C, 0xAC, 0x3A, 0x15, 0xCB, 0x67, 0xC7, 0x69,
            0xA1, 0x92, 0x99, 0x74, 0xEE, 0x90, 0x0D, 0xBE,
            0x57, 0x30, 0xD1, 0xBA, 0xE5, 0xDE, 0xFA, 0xD6,
            0x83, 0x8C, 0xE4, 0x43, 0x36, 0x5E, 0xCD, 0x84,
            0x1A, 0x18, 0x31, 0xB9, 0x20, 0x48, 0xE3, 0xA8,
            0x89, 0x32, 0xF0, 0x90, 0x21, 0x80, 0x33, 0xAE,
            0x3C, 0xA6, 0xB8, 0x8C, 0x72, 0x17, 0xD1, 0x0C,
            0x1A, 0x29, 0xFA, 0x38, 0x87, 0xC9, 0x6E, 0xC7,
            0x05, 0xDE, 0x85, 0x6E, 0x92, 0x7E, 0xD4, 0xED,
            0x5C, 0xD3, 0x03, 0xD4, 0xFE, 0xCB, 0x6C, 0x19,
            0x7A, 0x83, 0x79, 0x5B, 0xF6, 0x71, 0xBA, 0xF4,
            0x37, 0x53, 0xC9, 0xC1, 0xDE, 0xDB, 0xDE, 0xB1,
            0x64, 0x17, 0x31, 0x0E, 0xD7, 0xA2, 0x13, 0x8E,
            0x52, 0x8D, 0xCB, 0x19, 0x3D, 0x0B, 0x31, 0x58,
            0x4A, 0xDE, 0x0C, 0x01, 0x2B, 0x85, 0x2D, 0xE5,
            0x13, 0x22, 0x48, 0xB6, 0xF3, 0x2D, 0x00, 0x9A
        };
        */
		int x;
		UINT16 *PROTROM = (UINT16*)machine.region("igs022data")->base();

		for (x = 0; x < size; x++)
		{
			//UINT16 *RAMDUMP = (UINT16*)space->machine().region("user2")->base();
			//UINT16 dat = RAMDUMP[dst + x];

			UINT16 dat2 = PROTROM[src + x];

			UINT8 extraoffset = param&0xfe; // the lowest bit changed the table addressing in tests, see 'rawDataOdd' table instead.. it's still related to the main one, not identical
			UINT8* dectable = (UINT8*)machine.region("igs022data")->base();//rawDataEven; // the basic decryption table is at the start of the mcu data rom! at least in killbld
			UINT16 extraxor = ((dectable[((x*2)+0+extraoffset)&0xff]) << 8) | (dectable[((x*2)+1+extraoffset)&0xff] << 0);

			dat2 = ((dat2 & 0x00ff)<<8) | ((dat2 & 0xff00)>>8);

			//  mode==0 plain
			if (mode==3) dat2 ^= extraxor;
			if (mode==2) dat2 += extraxor;
			if (mode==1) dat2 -= extraxor;

			//if (dat!=dat2)
			//  printf("Mode %04x Param %04x Mismatch %04x %04x\n", mode, param, dat, dat2);

			state->m_sharedprotram[dst + x] = dat2;
		}

		/* Killing Blade: hack, patches out some additional security checks... we need to emulate them instead! */
		// different region IGS025 devices supply different sequences - we currently only have the china sequence for Killing Blade
		//if ((mode==3) && (param==0x54) && (src*2==0x2120) && (dst*2==0x2600)) state->m_sharedprotram[0x2600 / 2] = 0x4e75;

	}
	if (mode == 4)
	{
		mame_printf_debug("unhandled copy mode %04x!\n", mode);
		// not used by killing blade
		/* looks almost like a fixed value xor, but isn't */
	}
	else if (mode == 5)
	{
		/* mode 5 seems to be a straight copy */
		int x;
		UINT16 *PROTROM = (UINT16*)machine.region("igs022data")->base();
		for (x = 0; x < size; x++)
		{
			UINT16 dat = PROTROM[src + x];


			state->m_sharedprotram[dst + x] = dat;
		}
	}
	else if (mode == 6)
	{
		/* mode 6 seems to swap bytes and nibbles */
		int x;
		UINT16 *PROTROM = (UINT16*)machine.region("igs022data")->base();
		for (x = 0; x < size; x++)
		{
			UINT16 dat = PROTROM[src + x];

			dat = ((dat & 0xf000) >> 12)|
				  ((dat & 0x0f00) >> 4)|
				  ((dat & 0x00f0) << 4)|
				  ((dat & 0x000f) << 12);

			state->m_sharedprotram[dst + x] = dat;
		}
	}
	else if (mode == 7)
	{
		mame_printf_debug("unhandled copy mode %04x!\n", mode);
		// not used by killing blade
		/* weird mode, the params get left in memory? - maybe it's a NOP? */
	}
	else
	{
		mame_printf_debug("unhandled copy mode %04x!\n", mode);
		// not used by killing blade
		/* invalid? */

	}
}

// the internal MCU boot code automatically does this DMA
// and puts the version # of the data rom in ram
static void IGS022_reset(running_machine& machine)
{
	int i;
	UINT16 *PROTROM = (UINT16*)machine.region("igs022data")->base();
	pgm_state *state = machine.driver_data<pgm_state>();
	UINT16 tmp;

	// fill ram with A5 patern
	for (i = 0; i < 0x4000/2; i++)
		state->m_sharedprotram[i] = 0xa55a;

	// the auto-dma
	UINT16 src = PROTROM[0x100 / 2];
	UINT32 dst = PROTROM[0x102 / 2];
	UINT16 size = PROTROM[0x104/ 2];
	UINT16 mode = PROTROM[0x106 / 2];

	src = ((src & 0xff00) >> 8) | ((src & 0x00ff) << 8);
	dst = ((dst & 0xff00) >> 8) | ((dst & 0x00ff) << 8);
	size = ((size & 0xff00) >> 8) | ((size & 0x00ff) << 8);
	mode &= 0xff;

	src >>= 1;

	printf("Auto-DMA %04x %04x %04x %04x\n",src,dst,size,mode);

	IGS022_do_dma(machine,src,dst,size,mode);

	// there is also a version ID? (or is it some kind of checksum) that is stored in the data rom, and gets copied..
	// Dragon World 3 checks it
	tmp = PROTROM[0x114/2];
	tmp = ((tmp & 0xff00) >> 8) | ((tmp & 0x00ff) << 8);
	state->m_sharedprotram[0x2a2/2] = tmp;
}

static void IGS022_handle_command(running_machine& machine)
{
	pgm_state *state = machine.driver_data<pgm_state>();
	UINT16 cmd = state->m_sharedprotram[0x200/2];
	//mame_printf_debug("command %04x\n", cmd);
	if (cmd == 0x6d)	//Store values to asic ram
	{
		UINT32 p1 = (state->m_sharedprotram[0x298/2] << 16) | state->m_sharedprotram[0x29a/2];
		UINT32 p2 = (state->m_sharedprotram[0x29c/2] << 16) | state->m_sharedprotram[0x29e/2];

		if ((p2 & 0xffff) == 0x9)	//Set value
		{
			int reg = (p2 >> 16) & 0xffff;
			if (reg & 0x200)
				state->m_kb_regs[reg & 0xff] = p1;
		}
		if ((p2 & 0xffff) == 0x6)	//Add value
		{
			int src1 = (p1 >> 16) & 0xff;
			int src2 = (p1 >> 0) & 0xff;
			int dst = (p2 >> 16) & 0xff;
			state->m_kb_regs[dst] = state->m_kb_regs[src2] - state->m_kb_regs[src1];
		}
		if ((p2 & 0xffff) == 0x1)	//Add Imm?
		{
			int reg = (p2 >> 16) & 0xff;
			int imm = (p1 >> 0) & 0xffff;
			state->m_kb_regs[reg] += imm;
		}
		if ((p2 & 0xffff) == 0xa)	//Get value
		{
			int reg = (p1 >> 16) & 0xFF;
			state->m_sharedprotram[0x29c/2] = (state->m_kb_regs[reg] >> 16) & 0xffff;
			state->m_sharedprotram[0x29e/2] = state->m_kb_regs[reg] & 0xffff;
		}
	}
	if(cmd == 0x4f)	//memcpy with encryption / scrambling
	{
		UINT16 src = state->m_sharedprotram[0x290 / 2] >> 1; // ?
		UINT32 dst = state->m_sharedprotram[0x292 / 2];
		UINT16 size = state->m_sharedprotram[0x294 / 2];
		UINT16 mode = state->m_sharedprotram[0x296 / 2];

		IGS022_do_dma(machine, src,dst,size,mode);
	}

}


static WRITE16_HANDLER( killbld_igs025_prot_w )
{
//  mame_printf_debug("killbrd prot r\n");
//  return 0;
	pgm_state *state = space->machine().driver_data<pgm_state>();
	offset &= 0xf;

	if (offset == 0)
		state->m_kb_cmd = data;
	else //offset==2
	{
		logerror("%06X: ASIC25 W CMD %X  VAL %X\n", cpu_get_pc(&space->device()), state->m_kb_cmd, data);
		if (state->m_kb_cmd == 0)
			state->m_kb_reg = data;
		else if (state->m_kb_cmd == 2)
		{
			if (data == 1)	//Execute cmd
			{
				IGS022_handle_command(space->machine());
				state->m_kb_reg++;
			}
		}
		else if (state->m_kb_cmd == 4)
			state->m_kb_ptr = data;
		else if (state->m_kb_cmd == 0x20)
			state->m_kb_ptr++;
	}
}

static READ16_HANDLER( killbld_igs025_prot_r )
{
//  mame_printf_debug("killbld prot w\n");
	pgm_state *state = space->machine().driver_data<pgm_state>();
	UINT16 res ;

	offset &= 0xf;
	res = 0;

	if (offset == 1)
	{
		if (state->m_kb_cmd == 1)
		{
			res = state->m_kb_reg & 0x7f;
		}
		else if (state->m_kb_cmd == 5)
		{

			UINT8 kb_region_sequence[11] = {0x17, 0x14, 0x91, 0x89, 0x21, 0xD5, 0x7C, 0x65, 0x8F, 0x8E, 0xE1};
			UINT8 ret;

			// this isn't properly understood.. should be some kind of bitswap / xor / shift..based on values written to 0x22/0x23 etc.?
			// return hardcoded china sequence results for now, avoids rom patch
			if (state->m_kb_region_sequence_position < 11)
			{
				ret = kb_region_sequence[state->m_kb_region_sequence_position];
				state->m_kb_region_sequence_position++;
			}
			else
			{
				UINT32 protvalue = 0x89911400 | input_port_read(space->machine(), "Region");
				ret = (protvalue >> (8 * (state->m_kb_ptr - 1))) & 0xff;
			}

			res = 0x3f00 | ret;  // always 0x3fxx in logged behavior...

		}
	}
	logerror("%06X: ASIC25 R CMD %X  VAL %X\n", cpu_get_pc(&space->device()), state->m_kb_cmd, res);
	return res;
}




static MACHINE_RESET( killbld )
{
	pgm_state *state = machine.driver_data<pgm_state>();

	MACHINE_RESET_CALL(pgm);
	/* fill the protection ram with a5 + auto dma */
	IGS022_reset(machine);

	// Reset IGS025 stuff
	state->m_kb_cmd = 0;
	state->m_kb_reg = 0;
	state->m_kb_ptr = 0;
	state->m_kb_region_sequence_position = 0;
	memset(state->m_kb_regs, 0, 0x10);

}




/* ASIC025/ASIC022 don't provide rom patches like the DW2 protection does, the previous dump was bad :-) */
static DRIVER_INIT( killbld )
{
	pgm_state *state = machine.driver_data<pgm_state>();

	pgm_basic_init(machine);
	pgm_killbld_decrypt(machine);

	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0xd40000, 0xd40003, FUNC(killbld_igs025_prot_r), FUNC(killbld_igs025_prot_w));

	state->m_kb_cmd = 0;
	state->m_kb_reg = 0;
	state->m_kb_ptr = 0;
	state->m_kb_region_sequence_position = 0;
	memset(state->m_kb_regs, 0, 0x10);

	state->save_item(NAME(state->m_kb_region_sequence_position));
	state->save_item(NAME(state->m_kb_cmd));
	state->save_item(NAME(state->m_kb_reg));
	state->save_item(NAME(state->m_kb_ptr));
	state->save_item(NAME(state->m_kb_regs));
}

static MACHINE_RESET( dw3 )
{
	pgm_state *state = machine.driver_data<pgm_state>();


	MACHINE_RESET_CALL(pgm);
	/* fill the protection ram with a5 + auto dma */
	IGS022_reset(machine);

	/* game won't boot unless various values are in protection RAM
     - these should almost certainly end up there as the result of executing the protection
       commands are startup, but which, and how? */

//  state->m_sharedprotram[0x200/2] = 0x006d;
	state->m_sharedprotram[0x202/2] = 0x007c; // it cares about this, operation status flag?

//  state->m_sharedprotram[0x20c/2] = 0x0000;
//  state->m_sharedprotram[0x20e/2] = 0x0007;
//  state->m_sharedprotram[0x210/2] = 0x0000;
//  state->m_sharedprotram[0x212/2] = 0x0004;
//  state->m_sharedprotram[0x214/2] = 0x0000;
//  state->m_sharedprotram[0x216/2] = 0x0007;
//  state->m_sharedprotram[0x218/2] = 0x0000;
//  state->m_sharedprotram[0x21a/2] = 0x0004;

//  state->m_sharedprotram[0x288/2] = 0x0000;
//  state->m_sharedprotram[0x28a/2] = 0x00c2;
//  state->m_sharedprotram[0x28c/2] = 0x0000;
//  state->m_sharedprotram[0x28e/2] = 0x00c2;
//  state->m_sharedprotram[0x290/2] = 0x0500;
//  state->m_sharedprotram[0x292/2] = 0x1000;
//  state->m_sharedprotram[0x294/2] = 0x00c3;
//  state->m_sharedprotram[0x296/2] = 0x7104;
//  state->m_sharedprotram[0x298/2] = 0x0000;
//  state->m_sharedprotram[0x29a/2] = 0x0003;
//  state->m_sharedprotram[0x29c/2] = 0x0108;
//  state->m_sharedprotram[0x29e/2] = 0x0009;

//  state->m_sharedprotram[0x2a2/2] = 0x84f6; // it cares about this, it's the version number of the data rom, copied automatically!

//  state->m_sharedprotram[0x2ac/2] = 0x006d;
//  state->m_sharedprotram[0x2ae/2] = 0x0000;

//  state->m_sharedprotram[0x2b0/2] = 0xaf56;


	// Reset IGS025 stuff
	state->m_kb_cmd = 0;
	state->m_kb_reg = 0;
	state->m_kb_ptr = 0;
	state->m_kb_region_sequence_position = 0;
	memset(state->m_kb_regs, 0, 0x10);

}



static int reg;
static int ptr=0;

#define DW3BITSWAP(s,d,bs,bd)  d=((d&(~(1<<bd)))|(((s>>bs)&1)<<bd))
static UINT8 dw3_swap;
static WRITE16_HANDLER( drgw3_igs025_prot_w )
{
	pgm_state *state = space->machine().driver_data<pgm_state>();

	offset&=0xf;

	if(offset==0)
		state->m_kb_cmd=data;
	else //offset==2
	{
		printf("%06X: ASIC25 W CMD %X  VAL %X\n",cpu_get_pc(&space->device()),state->m_kb_cmd,data);
		if(state->m_kb_cmd==0)
			reg=data;
		else if(state->m_kb_cmd==3)	//??????????
		{
			dw3_swap = data;

			printf("SWAP %02x\n",dw3_swap);
		}
		//else if(kb_cmd==4)
		//  ptr=data;
		else if(state->m_kb_cmd==0x20)
			ptr++;
	}
}

static READ16_HANDLER( drgw3_igs025_prot_r )
{
//  mame_printf_debug("killbld prot w\n");
	pgm_state *state = space->machine().driver_data<pgm_state>();

	UINT16 res ;

	offset&=0xf;
	res=0;

	if(offset==1)
	{
		if(state->m_kb_cmd==0)	//swap
		{
				UINT8 v1=(dw3_swap+1)&0x7F;
				UINT8 v2=0;
				DW3BITSWAP(v1,v2,7,0);
				DW3BITSWAP(v1,v2,6,1);
				DW3BITSWAP(v1,v2,5,2);
				DW3BITSWAP(v1,v2,4,3);
				DW3BITSWAP(v1,v2,3,4);
				DW3BITSWAP(v1,v2,2,5);
				DW3BITSWAP(v1,v2,1,6);
				DW3BITSWAP(v1,v2,0,7);

				res=v2;

		}
		else if(state->m_kb_cmd==1)
		{
			res=reg&0x7f;
		}
		else if(state->m_kb_cmd==5)
		{
			UINT32 protvalue;
			protvalue = 0x60000|input_port_read(space->machine(), "Region");
			res=(protvalue>>(8*(ptr-1)))&0xff;


		}
	}
	logerror("%06X: ASIC25 R CMD %X  VAL %X\n",cpu_get_pc(&space->device()),state->m_kb_cmd,res);
	return res;
}


static DRIVER_INIT( drgw3 )
{
	pgm_basic_init(machine);

/*
    pgm_state *state = machine.driver_data<pgm_state>();

    {
        int x;
        UINT16 *RAMDUMP = (UINT16*)machine.region("user2")->base();
        for (x=0;x<(0x4000/2);x++)
        {
            state->m_sharedprotram[x] = RAMDUMP[x];
            if((x>=0x100)&&(x<0x110)) printf("data 0x%4x, offset:%x\n",state->m_sharedprotram[x],x);
        }
    }
*/
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0xDA5610, 0xDA5613, FUNC(drgw3_igs025_prot_r), FUNC(drgw3_igs025_prot_w));

	pgm_dw3_decrypt(machine);
}

static INT32 puzzli_54_trigger = 0;

// Preliminary
static WRITE16_HANDLER( puzzli2_asic_w )
{
	cavepgm_state *state = space->machine().driver_data<cavepgm_state>();

	switch (offset & 0x03)
	{
		case 0: state->m_value0 = data; return;

		case 1:
		{
			if ((data >> 8) == 0xff) state->m_valuekey = 0xffff;

			state->m_value0 ^= state->m_valuekey;

			switch ((data ^ state->m_valuekey) & 0xff)
			{
				case 0x13: // ASIC status?
					state->m_valueresponse = 0x74<<16; // 2d or 74! (based on?)
				break;

				case 0x31:
				{
					// how is this selected? command 54?

					// just a wild guess
					if (puzzli_54_trigger) {
						// pc == 1387de
						state->m_valueresponse = 0x63<<16; // ?
					} else {
						// pc == 14cf58
						state->m_valueresponse = 0xd2<<16;
					}

					puzzli_54_trigger = 0;
				}
				break;

				case 0x38: // Reset
					state->m_valueresponse = 0x78<<16;
					state->m_valuekey = 0;
					puzzli_54_trigger = 0;
				break;

				case 0x41: // ASIC status?
					state->m_valueresponse = 0x74<<16;
				break;

				case 0x47: // ASIC status?
					state->m_valueresponse = 0x74<<16;
				break;

				case 0x52: // ASIC status?
				{
					// how is this selected?

					//if (state->m_value0 == 6) {
						state->m_valueresponse = (0x74<<16)|1; // |1?
					//} else {
					//  state->m_valueresponse = 0x74<<16;
					//}
				}
				break;

				case 0x54: // ??
					puzzli_54_trigger = 1;
					state->m_valueresponse = 0x36<<16;
				break;

				case 0x61: // ??
					state->m_valueresponse = 0x36<<16;
				break;

				case 0x63: // probably read from a data table?
					state->m_valueresponse = 0; // wrong...
				break;

				case 0x67: // probably read from a data table?
					state->m_valueresponse = 0; // wrong...
				break;

				default:
					state->m_valueresponse = 0x74<<16;
				break;
			}

			state->m_valuekey = (state->m_valuekey + 0x0100) & 0xff00;
			if (state->m_valuekey == 0xff00) state->m_valuekey = 0x0100;
			state->m_valuekey |= state->m_valuekey >> 8;
		}
		return;

		case 2: return;
	}
}


static DRIVER_INIT( dw2001 )
{
	//pgm_state *state = machine.driver_data<pgm_state>();
	UINT16 *mem16 = (UINT16 *)machine.region("maincpu")->base();

	pgm_basic_init(machine);
	kovsh_latch_init(machine);
	pgm_mm_decrypt(machine); // encryption is the same as martial masters

	mem16[0x11e90c / 2] = 0x4e71;
	mem16[0x11e90e / 2] = 0x4e71;

	mem16[0x11e91a / 2] = 0x4e71;

	mem16[0x11eaf6 / 2] = 0x4e71;
	mem16[0x11eaf8 / 2] = 0x4e71;

	mem16[0x11eb04 / 2] = 0x4e71;

}

static UINT32 olds_prot_addr( UINT16 addr )
{
	UINT32 mode = addr & 0xff;
	UINT32 offset = addr >> 8;
	UINT32 realaddr;

	switch(mode)
	{
		case 0x0:
		case 0x5:
		case 0xa:
			realaddr = 0x402a00 + (offset << 2);
			break;

		case 0x2:
		case 0x8:
			realaddr = 0x402e00 + (offset << 2);
			break;

		case 0x1:
			realaddr = 0x40307e;
			break;

		case 0x3:
			realaddr = 0x403090;
			break;

		case 0x4:
			realaddr = 0x40309a;
			break;

		case 0x6:
			realaddr = 0x4030a4;
			break;

		case 0x7:
			realaddr = 0x403000;
			break;

		case 0x9:
			realaddr = 0x40306e;
			break;

		default:
			realaddr = 0;
	}
	return realaddr;
}

static UINT32 olds_read_reg( running_machine &machine, UINT16 addr )
{
	pgm_state *state = machine.driver_data<pgm_state>();
	UINT32 protaddr = (olds_prot_addr(addr) - 0x400000) / 2;
	return state->m_sharedprotram[protaddr] << 16 | state->m_sharedprotram[protaddr + 1];
}

static void olds_write_reg( running_machine &machine, UINT16 addr, UINT32 val )
{
	pgm_state *state = machine.driver_data<pgm_state>();
	state->m_sharedprotram[(olds_prot_addr(addr) - 0x400000) / 2]     = val >> 16;
	state->m_sharedprotram[(olds_prot_addr(addr) - 0x400000) / 2 + 1] = val & 0xffff;
}

static MACHINE_RESET( olds )
{
	pgm_state *state = machine.driver_data<pgm_state>();
	UINT16 *mem16 = (UINT16 *)machine.region("user2")->base();
	int i;

	MACHINE_RESET_CALL(pgm);

	/* populate shared protection ram with data read from pcb .. */
	for (i = 0; i < 0x4000 / 2; i++)
	{
		state->m_sharedprotram[i] = mem16[i];
	}

	//ROM:004008B4                 .word 0xFBA5
	for(i = 0; i < 0x4000 / 2; i++)
	{
		if (state->m_sharedprotram[i] == (0xffff - i))
			state->m_sharedprotram[i] = 0x4e75;
	}
}

static READ16_HANDLER( olds_r )
{
	pgm_state *state = space->machine().driver_data<pgm_state>();
	UINT16 res = 0;

	if (offset == 1)
	{
		if (state->m_kb_cmd == 1)
			res = state->m_kb_reg & 0x7f;
		if (state->m_kb_cmd == 2)
			res = state->m_olds_bs | 0x80;
		if (state->m_kb_cmd == 3)
			res = state->m_olds_cmd3;
		else if (state->m_kb_cmd == 5)
		{
			UINT32 protvalue = 0x900000 | input_port_read(space->machine(), "Region"); // region from protection device.
			res = (protvalue >> (8 * (state->m_kb_ptr - 1))) & 0xff; // includes region 1 = taiwan , 2 = china, 3 = japan (title = orlegend special), 4 = korea, 5 = hongkong, 6 = world

		}
	}
	logerror("%06X: ASIC25 R CMD %X  VAL %X\n", cpu_get_pc(&space->device()), state->m_kb_cmd, res);
	return res;
}

static WRITE16_HANDLER( olds_w )
{
	pgm_state *state = space->machine().driver_data<pgm_state>();
	if (offset == 0)
		state->m_kb_cmd = data;
	else //offset==2
	{
		logerror("%06X: ASIC25 W CMD %X  VAL %X\n",cpu_get_pc(&space->device()), state->m_kb_cmd, data);
		if (state->m_kb_cmd == 0)
			state->m_kb_reg = data;
		else if(state->m_kb_cmd == 2)	//a bitswap=
		{
			int reg = 0;
			if (data & 0x01)
				reg |= 0x40;
			if (data & 0x02)
				reg |= 0x80;
			if (data & 0x04)
				reg |= 0x20;
			if (data & 0x08)
				reg |= 0x10;
			state->m_olds_bs = reg;
		}
		else if (state->m_kb_cmd == 3)
		{
			UINT16 cmd = state->m_sharedprotram[0x3026 / 2];
			switch (cmd)
			{
				case 0x11:
				case 0x12:
						break;
				case 0x64:
					{
						UINT16 cmd0 = state->m_sharedprotram[0x3082 / 2];
						UINT16 val0 = state->m_sharedprotram[0x3050 / 2];	//CMD_FORMAT
						{
							if ((cmd0 & 0xff) == 0x2)
								olds_write_reg(space->machine(), val0, olds_read_reg(space->machine(), val0) + 0x10000);
						}
						break;
					}

				default:
						break;
			}
			state->m_olds_cmd3 = ((data >> 4) + 1) & 0x3;
		}
		else if (state->m_kb_cmd == 4)
			state->m_kb_ptr = data;
		else if(state->m_kb_cmd == 0x20)
		  state->m_kb_ptr++;
	}
}

static READ16_HANDLER( olds_prot_swap_r )
{
	if (cpu_get_pc(&space->device()) < 0x100000)		//bios
		return pgm_mainram[0x178f4 / 2];
	else						//game
		return pgm_mainram[0x178d8 / 2];

}

static DRIVER_INIT( olds )
{
	pgm_state *state = machine.driver_data<pgm_state>();
	pgm_basic_init(machine);

	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0xdcb400, 0xdcb403, FUNC(olds_r), FUNC(olds_w));
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x8178f4, 0x8178f5, FUNC(olds_prot_swap_r));

	state->m_kb_cmd = 0;
	state->m_kb_reg = 0;
	state->m_kb_ptr = 0;
	state->m_olds_bs = 0;
	state->m_olds_cmd3 = 0;

	state->save_item(NAME(state->m_kb_cmd));
	state->save_item(NAME(state->m_kb_reg));
	state->save_item(NAME(state->m_kb_ptr));
	state->save_item(NAME(state->m_olds_bs));
	state->save_item(NAME(state->m_olds_cmd3));
}



static void pgm_decode_kovlsqh2_tiles( running_machine &machine )
{
	int i, j;
	UINT16 *src = (UINT16 *)(machine.region("tiles")->base() + 0x180000);
	UINT16 *dst = auto_alloc_array(machine, UINT16, 0x800000);

	for (i = 0; i < 0x800000 / 2; i++)
	{
		j = BITSWAP24(i, 23, 22, 9, 8, 21, 18, 0, 1, 2, 3, 16, 15, 14, 13, 12, 11, 10, 19, 20, 17, 7, 6, 5, 4);

		dst[j] = BITSWAP16(src[i], 1, 14, 8, 7, 0, 15, 6, 9, 13, 2, 5, 10, 12, 3, 4, 11);
	}

	memcpy( src, dst, 0x800000 );

	auto_free( machine, dst );
}

static void pgm_decode_kovlsqh2_sprites( running_machine &machine, UINT8 *src )
{
	int i, j;
	UINT8 *dst = auto_alloc_array(machine, UINT8, 0x800000);

	for (i = 0; i < 0x800000; i++)
	{
		j = BITSWAP24(i, 23, 10, 9, 22, 19, 18, 20, 21, 17, 16, 15, 14, 13, 12, 11, 8, 7, 6, 5, 4, 3, 2, 1, 0);

		dst[j] = src[i];
	}

	memcpy( src, dst, 0x800000 );

	auto_free( machine, dst );
}

static void pgm_decode_kovlsqh2_samples( running_machine &machine )
{
	int i;
	UINT8 *src = (UINT8 *)(machine.region("ics")->base() + 0x400000);

	for (i = 0; i < 0x400000; i+=2) {
		src[i + 0x000001] = src[i + 0x400001];
	}

	memcpy( src + 0x400000, src, 0x400000 );
}

static void pgm_decode_kovqhsgs_program( running_machine &machine )
{
	int i;
	UINT16 *src = (UINT16 *)(machine.region("maincpu")->base() + 0x100000);
	UINT16 *dst = auto_alloc_array(machine, UINT16, 0x400000);

	for (i = 0; i < 0x400000 / 2; i++)
	{
		int j = BITSWAP24(i, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 6, 7, 5, 4, 3, 2, 1, 0);

		dst[j] = BITSWAP16(src[i], 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 4, 5, 3, 2, 1, 0);
	}

	memcpy( src, dst, 0x400000 );

	auto_free( machine, dst );
}

static void pgm_decode_kovqhsgs2_program( running_machine &machine )
{
	int i;
	UINT16 *src = (UINT16 *)(machine.region("maincpu")->base() + 0x100000);
	UINT16 *dst = auto_alloc_array(machine, UINT16, 0x400000);

	for (i = 0; i < 0x400000 / 2; i++)
	{
		int j = BITSWAP24(i, 23, 22, 21, 20, 19, 16, 15, 14, 13, 12, 11, 10, 9, 8, 0, 1, 2, 3, 4, 5, 6, 18, 17, 7);

		dst[j] = src[i];
	}

	memcpy( src, dst, 0x400000 );

	auto_free( machine, dst );
}


static DRIVER_INIT( kovlsqh2 )
{
	pgm_decode_kovqhsgs2_program(machine);
	pgm_decode_kovlsqh2_tiles(machine);

	pgm_decode_kovlsqh2_sprites(machine, machine.region("sprcol")->base() + 0x0000000);
	pgm_decode_kovlsqh2_sprites(machine, machine.region("sprcol")->base() + 0x0800000);
	pgm_decode_kovlsqh2_sprites(machine, machine.region("sprcol")->base() + 0x1000000);
	pgm_decode_kovlsqh2_sprites(machine, machine.region("sprcol")->base() + 0x1800000);
	pgm_decode_kovlsqh2_sprites(machine, machine.region("sprcol")->base() + 0x2000000);
	pgm_decode_kovlsqh2_sprites(machine, machine.region("sprcol")->base() + 0x2800000);
	pgm_decode_kovlsqh2_sprites(machine, machine.region("sprmask")->base() + 0x0000000);
	pgm_decode_kovlsqh2_sprites(machine, machine.region("sprmask")->base() + 0x0800000);

	pgm_decode_kovlsqh2_samples(machine);
	pgm_basic_init(machine);
	kovsh_latch_init(machine);
}

static DRIVER_INIT( kovqhsgs )
{
	pgm_decode_kovqhsgs_program(machine);
	pgm_decode_kovlsqh2_tiles(machine);

	pgm_decode_kovlsqh2_sprites(machine, machine.region("sprcol")->base() + 0x0000000);
	pgm_decode_kovlsqh2_sprites(machine, machine.region("sprcol")->base() + 0x0800000);
	pgm_decode_kovlsqh2_sprites(machine, machine.region("sprcol")->base() + 0x1000000);
	pgm_decode_kovlsqh2_sprites(machine, machine.region("sprcol")->base() + 0x1800000);
	pgm_decode_kovlsqh2_sprites(machine, machine.region("sprcol")->base() + 0x2000000);
	pgm_decode_kovlsqh2_sprites(machine, machine.region("sprcol")->base() + 0x2800000);
	pgm_decode_kovlsqh2_sprites(machine, machine.region("sprmask")->base() + 0x0000000);
	pgm_decode_kovlsqh2_sprites(machine, machine.region("sprmask")->base() + 0x0800000);

	pgm_decode_kovlsqh2_samples(machine);

	pgm_basic_init(machine);
	kovsh_latch_init(machine);
}

/*
 in Ketsui (ket) @ 000A719C (move.w)

 if you change D0 to 0x12
 the game will runs to "Asic27 Test" mode

 bp A71A0,1,{d0=0x12;g}
*/

static WRITE16_HANDLER( ddp3_asic_w )
{
	cavepgm_state *state = space->machine().driver_data<cavepgm_state>();

	if (offset == 0)
	{
		state->m_value0 = data;
		return;
	}
	else if (offset == 1)
	{
		UINT16 realkey;
		if ((data >> 8) == 0xff)
			state->m_valuekey = 0xff00;
		realkey = state->m_valuekey >> 8;
		realkey |= state->m_valuekey;
		{
			state->m_valuekey += 0x0100;
			state->m_valuekey &= 0xff00;
			if (state->m_valuekey == 0xff00)
				state->m_valuekey =  0x0100;
		}
		data ^= realkey;
		state->m_value1 = data;
		state->m_value0 ^= realkey;

		state->m_ddp3lastcommand = state->m_value1 & 0xff;

		/* typical frame (ddp3) (all 3 games use only these commands? for the most part of levels espgal just issues 8e)
            vbl
            145f28 command 67
            145f70 command e5
            145f28 command 67
            145f70 command e5
            1460c6 command 40
            145ec0 command 8e
            */

		switch (state->m_ddp3lastcommand)
		{
			default:
				printf("%06x command %02x | %04x\n", cpu_get_pc(&space->device()), state->m_ddp3lastcommand, state->m_value0);
				state->m_valueresponse = 0x880000;
				break;

			case 0x40:
				state->m_valueresponse = 0x880000;
			    state->m_ddp3slots[(state->m_value0>>10)&0x1F]=
					(state->m_ddp3slots[(state->m_value0>>5)&0x1F]+
					 state->m_ddp3slots[(state->m_value0>>0)&0x1F])&0xffffff;
				break;

			case 0x67: // set high bits
		//      printf("%06x command %02x | %04x\n", cpu_get_pc(&space->device()), state->m_ddp3lastcommand, state->m_value0);
				state->m_valueresponse = 0x880000;
				state->m_ddp3internal_slot = (state->m_value0 & 0xff00)>>8;
				state->m_ddp3slots[state->m_ddp3internal_slot] = (state->m_value0 & 0x00ff) << 16;
				break;

			case 0xe5: // set low bits for operation?
			//  printf("%06x command %02x | %04x\n", cpu_get_pc(&space->device()), state->m_ddp3lastcommand, state->m_value0);
				state->m_valueresponse = 0x880000;
				state->m_ddp3slots[state->m_ddp3internal_slot] |= (state->m_value0 & 0xffff);
				break;


			case 0x8e: // read back result of operations
		//      printf("%06x command %02x | %04x\n", cpu_get_pc(&space->device()), state->m_ddp3lastcommand, state->m_value0);
				state->m_valueresponse = state->m_ddp3slots[state->m_value0&0xff];
				break;


			case 0x99: // reset?
				state->m_valuekey = 0x100;
				state->m_valueresponse = 0x00880000;
				break;

		}
	}
	else if (offset==2)
	{

	}

}

static READ16_HANDLER( ddp3_asic_r )
{
	cavepgm_state *state = space->machine().driver_data<cavepgm_state>();

	if (offset == 0)
	{
		UINT16 d = state->m_valueresponse & 0xffff;
		UINT16 realkey = state->m_valuekey >> 8;
		realkey |= state->m_valuekey;
		d ^= realkey;

		return d;

	}
	else if (offset == 1)
	{
		UINT16 d = state->m_valueresponse >> 16;
		UINT16 realkey = state->m_valuekey >> 8;
		realkey |= state->m_valuekey;
		d ^= realkey;
		return d;

	}
	return 0xffff;
}


void install_asic27a_ddp3(running_machine& machine)
{
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0x500000, 0x500005, FUNC(ddp3_asic_r), FUNC(ddp3_asic_w));
}

void install_asic27a_ket(running_machine& machine)
{
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0x400000, 0x400005, FUNC(ddp3_asic_r), FUNC(ddp3_asic_w));
}

void install_asic27a_espgal(running_machine& machine)
{
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0x400000, 0x400005, FUNC(ddp3_asic_r), FUNC(ddp3_asic_w));
}

static DRIVER_INIT( ddp3 )
{
	pgm_basic_init(machine, false);
	pgm_py2k2_decrypt(machine); // yes, it's the same as photo y2k2
	install_asic27a_ddp3(machine);
}

static DRIVER_INIT( ket )
{
	pgm_basic_init(machine, false);
	pgm_ket_decrypt(machine);
	install_asic27a_ket(machine);
}

static DRIVER_INIT( espgal )
{
	pgm_basic_init(machine, false);
	pgm_espgal_decrypt(machine);
	install_asic27a_espgal(machine);
}

static DRIVER_INIT( puzzli2 )
{
	cavepgm_state *state = machine.driver_data<cavepgm_state>();

	pgm_basic_init(machine);

	pgm_puzzli2_decrypt(machine);

	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0x500000, 0x500005, FUNC(ddp3_asic_r), FUNC(puzzli2_asic_w));
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x4f0000, 0x4fffff, FUNC(sango_protram_r));

	// doesn't like this irq??
	state->m_irq4_disabled = 1;
}

static void oldsplus_latch_init( running_machine &machine )
{
	pgm_state *state = machine.driver_data<pgm_state>();

	state->m_oldsplus_key = 0;
	state->m_oldsplus_int[0] = 0;
	state->m_oldsplus_int[1] = 0;
	state->m_oldsplus_val = 0;
	memset(state->m_oldsplus_ram, 0, 0x100);
	memset(state->m_oldsplus_regs, 0, 0x100);

	state_save_register_global(machine, state->m_oldsplus_key);
	state_save_register_global(machine, state->m_oldsplus_val);
	state_save_register_global_array(machine, state->m_oldsplus_int);
	state_save_register_global_array(machine, state->m_oldsplus_ram);
	state_save_register_global_array(machine, state->m_oldsplus_regs);
}

#if 0
static DRIVER_INIT( oldsplus )
{
	pgm_basic_init(machine);
	pgm_oldsplus_decrypt(machine);
	kovsh_latch_init(machine);
}
#endif

static DRIVER_INIT( oldsplus )
{
	pgm_basic_init(machine);
	pgm_oldsplus_decrypt(machine);
	oldsplus_latch_init(machine);

	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0x500000, 0x500003, FUNC(oldsplus_r), FUNC(oldsplus_w));
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x4f0000, 0x4fffff, FUNC(oldsplus_protram_r));
}

static DRIVER_INIT( kov2p )
{
	// this hacks the identification of the kov2 rom to return the string required for kov2p
	// this isn't guaranteed to work properly (and definitely wouldn't on real hardware due to the internal
	// ROM uploading the encryption table)  The internal ROM should be dumped properly.
	UINT8 *mem8 = (UINT8 *)machine.region("user1")->base();
	pgm_basic_init(machine);
	pgm_kov2p_decrypt(machine);
	kov2_latch_init(machine);
	mem8[0xDE] = 0xC0;
	mem8[0xDF] = 0x46;
	mem8[0x4ED8] = 0xA8;// B0
	mem8[0x4EDC] = 0x9C;// A4
	mem8[0x4EE0] = 0x5C;// 64
	mem8[0x4EE4] = 0x94;// 9C
	mem8[0x4EE8] = 0xE8;// F0
	mem8[0x4EEC] = 0x6C;// 74
	mem8[0x4EF0] = 0xD4;// DC
	mem8[0x4EF4] = 0x50;// 58
	mem8[0x4EF8] = 0x80;// 88
	mem8[0x4EFC] = 0x9C;// A4
	mem8[0x4F00] = 0x28;// 30
	mem8[0x4F04] = 0x30;// 38
	mem8[0x4F08] = 0x34;// 3C
	mem8[0x4F0C] = 0x1C;// 24
	mem8[0x1FFFFC] = 0x33;
	mem8[0x1FFFFD] = 0x99;
}

/*** GAME ********************************************************************/

GAME( 1997, pgm,          0,         pgm,     pgm,      pgm,        ROT0,   "IGS", "PGM (Polygame Master) System BIOS", GAME_IS_BIOS_ROOT )

/* -----------------------------------------------------------------------------------------------------------------------
   Working (at least one set of the game is fully working)
   -----------------------------------------------------------------------------------------------------------------------*/

// the version numbering on these is a mess... date srings from ROM (and in some cases even those are missing..)
GAME( 1997, orlegend,     pgm,       pgm,     orlegend, orlegend,   ROT0,   "IGS", "Oriental Legend / Xi You Shi E Zhuan (ver. 126)", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )                // V0001 01/14/98 18:16:38 - runs as World
GAME( 1997, orlegende,    orlegend,  pgm,     orlegend, orlegend,   ROT0,   "IGS", "Oriental Legend / Xi You Shi E Zhuan (ver. 112)", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )                // V0001 07/14/97 11:19:45 - runs as World
GAME( 1997, orlegendc,    orlegend,  pgm,     orlegend, orlegend,   ROT0,   "IGS", "Oriental Legend / Xi You Shi E Zhuan (ver. 112, Chinese Board)", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE ) // V0001 05/05/97 10:08:21 - runs as World, Korea, China
GAME( 1997, orlegendca,   orlegend,  pgm,     orlegend, orlegend,   ROT0,   "IGS", "Oriental Legend / Xi You Shi E Zhuan (ver. ???, Chinese Board)", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE ) // V0001 04/02/97 13:35:43 - runs as HongKong, China, China
GAME( 1997, orlegend111c, orlegend,  pgm,     orlegend, orlegend,   ROT0,   "IGS", "Oriental Legend / Xi You Shi E Zhuan (ver. 111, Chinese Board)", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE ) // V0001 no date!          - runs as HongKong, China, China
GAME( 1997, orlegend105k, orlegend,  pgm,     orld105k, orlegend,   ROT0,   "IGS", "Oriental Legend / Xi You Shi E Zhuan (ver. 105, Korean Board)", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )  // V0000 no date!          - runs as Korea

GAME( 1997, drgw2,        pgm,       pgm,     pgm,      drgw2,      ROT0,   "IGS", "Dragon World II (ver. 110X, Export)", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING | GAME_SUPPORTS_SAVE ) // This set still has protection issues!
GAME( 1997, dw2v100x,     drgw2,     pgm,     pgm,      dw2v100x,   ROT0,   "IGS", "Dragon World II (ver. 100X, Export)", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING | GAME_SUPPORTS_SAVE ) // This set still has protection issues!
GAME( 1997, drgw2j,       drgw2,     pgm,     pgm,      drgw2j,     ROT0,   "IGS", "Chuugokuryuu II (ver. 100J, Japan)", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING | GAME_SUPPORTS_SAVE ) // This set still has protection issues!
GAME( 1997, drgw2c,       drgw2,     pgm,     pgm,      drgw2c,     ROT0,   "IGS", "Zhong Guo Long II (ver. 100C, China)", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )

GAME( 1999, photoy2k,     pgm,       kov,     photoy2k, photoy2k,   ROT0,   "IGS", "Photo Y2K (ver. 105)", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE ) /* region provided by protection device */
GAME( 1999, photoy2k104,  photoy2k,  kov,     photoy2k, photoy2k,   ROT0,   "IGS", "Photo Y2K (ver. 104)", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE ) /* region provided by protection device */
GAME( 1999, photoy2k102,  photoy2k,  kov,     photoy2k, photoy2k,   ROT0,   "IGS", "Photo Y2K (ver. 102, Japanese Board)", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE ) /* region provided by protection device */

GAME( 1999, kovsh,        pgm,       kov,     sango,    kovsh,      ROT0,   "IGS", "Knights of Valour Super Heroes / Sangoku Senki Super Heroes (ver. 104, CN)", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE ) // 68k V104, China internal ROM
GAME( 1999, kovsh103,     kovsh,     kov,     sango,    kovsh,      ROT0,   "IGS", "Knights of Valour Super Heroes / Sangoku Senki Super Heroes (ver. 103, CN)", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE ) // 68k V103, China internal ROM
// nasty modern asian bootleg of Knights of Valour Super Heroes with characters ripped from SNK's The King of Fighters series!
GAME( 1999, kovqhsgs,     kovsh,     kov,     sango,	kovqhsgs,   ROT0,   "bootleg", "Knights of Valour: Quan Huang San Guo Special / Sangoku Senki: Quan Huang San Guo Special (ver. 303CN)", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )

GAME( 2000, kov2,         pgm,       kov2,    sango,    kov2,       ROT0,   "IGS", "Knights of Valour 2 / Sangoku Senki 2 (ver. 107, 102, 100HK)", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE ) // 05/10/01 14:24:08 V107 (Ext. Arm V102, Int. Arm V100HK)
GAME( 2000, kov2106,      kov2,      kov2,    sango,    kov2,       ROT0,   "IGS", "Knights of Valour 2 / Sangoku Senki 2 (ver. 106, 102, 100KH)", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE ) // 02/27/01 13:26:46 V106 (Ext. Arm V102, Int. Arm V100HK)
GAME( 2000, kov2103,      kov2,      kov2,    sango,    kov2,       ROT0,   "IGS", "Knights of Valour 2 / Sangoku Senki 2 (ver. 103, 101, 100HK)", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE ) // 12/28/00 15:09:31 V103 (Ext. Arm V101, Int. Arm V100HK)
GAME( 2000, kov2102,      kov2,      kov2,    sango,    kov2,       ROT0,   "IGS", "Knights of Valour 2 / Sangoku Senki 2 (ver. 102, 101, 100HK)", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE ) // 12/14/00 10:33:36 V102 (Ext. Arm V101, Int. Arm V100HK)
GAME( 2000, kov2101,      kov2,      kov2,    sango,    kov2,       ROT0,   "IGS", "Knights of Valour 2 / Sangoku Senki 2 (ver. 101, 101, 100HK)", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE ) // 11/29/00 11:03:08 V100 (Ext. Arm V100, Int. Arm V100HK)
GAME( 2000, kov2100,      kov2,      kov2,    sango,    kov2,       ROT0,   "IGS", "Knights of Valour 2 / Sangoku Senki 2 (ver. 100, 100, 100HK)", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE ) // 11/29/00 11:03:08 V100 (Ext. Arm V100, Int. Arm V100HK)

GAME( 2001, kov2p,        pgm,       kov2,    sango,    kov2p,      ROT0,   "IGS", "Knights of Valour 2 Plus - Nine Dragons / Sangoku Senki 2 Plus - Nine Dragons (ver. M204XX)", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING | GAME_SUPPORTS_SAVE ) /* need internal rom of IGS027A */
GAME( 2001, kov2p205,     kov2p,     kov2,    sango,    kov2p,      ROT0,   "IGS", "Knights of Valour 2 Plus - Nine Dragons / Sangoku Senki 2 Plus - Nine Dragons (ver. M205XX)", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING | GAME_SUPPORTS_SAVE ) /* need internal rom of IGS027A */

GAME( 2001, martmast,     pgm,       kov2,    sango,    martmast,   ROT0,   "IGS", "Martial Masters (ver. 104, 102, 102US)", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE ) // 68k V104, Ext Arm 102, Int Arm 102US
GAME( 2001, martmastc,    martmast,  kov2,    sango,    martmast,   ROT0,   "IGS", "Martial Masters (ver. 104, 102, 101CN)", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE ) // 68k V104, Ext Arm 102, Int Arm 101CN
GAME( 2001, martmastc102, martmast,  kov2,    sango,    martmast,   ROT0,   "IGS", "Martial Masters (ver. 102, 101, 101CN)", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE ) // 68k V102, Ext Arm 101, Int Arm 101CN

/* -----------------------------------------------------------------------------------------------------------------------
   Partially Working, playable, but some imperfections
   -----------------------------------------------------------------------------------------------------------------------*/

 // it's playable, but the region check is still patched (different IGS025 chips return different sequences so that the game can determine the region)
GAME( 1998, killbld,      pgm,       killbld, killbld,  killbld,    ROT0,   "IGS", "The Killing Blade (ver. 109, Chinese Board)", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
GAME( 1998, killbld104,   killbld,   killbld, killbld,  killbld,    ROT0,   "IGS", "The Killing Blade (ver. 104)", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )

GAME( 1998, olds,         pgm,       olds,    olds,     olds,       ROT0,   "IGS", "Oriental Legend Special / Xi You Shi E Zhuan Super (ver. 101, Korean Board)", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING | GAME_SUPPORTS_SAVE )
GAME( 1998, olds100,      olds,      olds,    olds,     olds,       ROT0,   "IGS", "Oriental Legend Special / Xi You Shi E Zhuan Super (ver. 100, set 1)", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING | GAME_SUPPORTS_SAVE )
GAME( 1998, olds100a,     olds,      olds,    olds,     olds,       ROT0,   "IGS", "Oriental Legend Special / Xi You Shi E Zhuan Super (ver. 100, set 2)", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING | GAME_SUPPORTS_SAVE ) // crashes on some bosses, high score table etc.

GAME( 1999, kov,          pgm,       kov_simulated_arm,     sango,    kov,        ROT0,   "IGS", "Knights of Valour / Sangoku Senki (ver. 117)", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_SUPPORTS_SAVE ) /* need internal rom of IGS027A */                 // V0008 04/27/99 10:33:33
GAME( 1999, kov115,       kov,       kov_simulated_arm,     sango,    kov,        ROT0,   "IGS", "Knights of Valour / Sangoku Senki (ver. 115)", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_SUPPORTS_SAVE ) /* need internal rom of IGS027A */                 // V0006 02/22/99 11:53:18
GAME( 1999, kov100,       kov,       kov_simulated_arm,     sango,    kov,        ROT0,   "IGS", "Knights of Valour / Sangoku Senki (ver. 100, Japanese Board)", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_SUPPORTS_SAVE ) /* need internal rom of IGS027A */ // V0002 01/31/99 01:54:16

GAME( 1999, kovplus,      pgm,       kov_simulated_arm,     sango,    kov,        ROT0,   "IGS", "Knights of Valour Plus / Sangoku Senki Plus (ver. 119, set 1)", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_SUPPORTS_SAVE ) /* need internal rom of IGS027A */
GAME( 1999, kovplusa,     kovplus,   kov_simulated_arm,     sango,    kov,        ROT0,   "IGS", "Knights of Valour Plus / Sangoku Senki Plus (ver. 119, set 2)", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_SUPPORTS_SAVE ) /* need internal rom of IGS027A */

// modified title screen is only visible for china region, so use that by default.  Character select portraits don't seem quite right (different protection?)
GAME( 1999, kovsgqyz,     kovplus,   kov_simulated_arm,     sango_ch,    kovboot,    ROT0,   "bootleg", "Knights of Valour: SanGuo QunYingZhuan / Sangoku Senki: SanGuo QunYingZhuan (bootleg, set 1)", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_SUPPORTS_SAVE ) /* need internal rom of IGS027A */
GAME( 1999, kovsgqyza,    kovplus,   kov_simulated_arm,     sango_ch,    kovboot,    ROT0,   "bootleg", "Knights of Valour: SanGuo QunYingZhuan / Sangoku Senki: SanGuo QunYingZhuan (bootleg, set 2)", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_SUPPORTS_SAVE ) /* need internal rom of IGS027A */
GAME( 1999, kovsgqyzb,    kovplus,   kov_simulated_arm,     sango_ch,    kovboot,    ROT0,   "bootleg", "Knights of Valour: SanGuo QunYingZhuan / Sangoku Senki: SanGuo QunYingZhuan (bootleg, set 3)", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_SUPPORTS_SAVE ) /* need internal rom of IGS027A */




/* -----------------------------------------------------------------------------------------------------------------------
   NOT Working (mostly due to needing internal protection roms dumped)
   -----------------------------------------------------------------------------------------------------------------------*/

// should have DMA protection, like killbld, as well as the math / bitswap / memory manipulation stuff, but it never attempts to trigger the DMA? - we currently have a RAM dump to allow it to boot, but I think this stuff should be DMA copied into RAM, like killbld
GAME( 1998, drgw3,        pgm,       dw3,     dw3,      drgw3,      ROT0,   "IGS", "Dragon World 3 (ver. 106, Korean Board)", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING | GAME_SUPPORTS_SAVE )
GAME( 1998, drgw3105,     drgw3,     dw3,     dw3,      drgw3,      ROT0,   "IGS", "Dragon World 3 (ver. 105)", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING | GAME_SUPPORTS_SAVE )
GAME( 1998, drgw3100,     drgw3,     dw3,     dw3,      drgw3,      ROT0,   "IGS", "Dragon World 3 (ver. 100)", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING | GAME_SUPPORTS_SAVE ) // Japan Only?

GAME( 1998, dwex,         pgm,       dw3,     dw3,      drgw3,      ROT0,   "IGS", "Dragon World 3 EX (ver. 100)", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING | GAME_SUPPORTS_SAVE )

GAME( 1999, puzlstar,     pgm,       kov_disabled_arm,     sango,    pstar,      ROT0,   "IGS", "Puzzle Star (ver. 100MG)", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING | GAME_SUPPORTS_SAVE ) /* need internal rom of IGS027A */

GAME( 2001, py2k2,        pgm,       kov_disabled_arm,     photoy2k, py2k2,      ROT0,   "IGS", "Photo Y2K 2", GAME_NOT_WORKING )  /* need internal rom of IGS027A */

GAME( 2001, ddp2,         pgm,       kov2_disabled_arm,    ddp2,     ddp2,       ROT270, "IGS", "Bee Storm - DoDonPachi II (ver. 102)", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING | GAME_SUPPORTS_SAVE ) /* need internal rom of IGS027A */
GAME( 2001, ddp2101,      ddp2,      kov2_disabled_arm,    ddp2,     ddp2,       ROT270, "IGS", "Bee Storm - DoDonPachi II (ver. 101)", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING | GAME_SUPPORTS_SAVE ) /* need internal rom of IGS027A */
GAME( 2001, ddp2100,      ddp2,      kov2_disabled_arm,    ddp2,     ddp2,       ROT270, "IGS", "Bee Storm - DoDonPachi II (ver. 100)", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING | GAME_SUPPORTS_SAVE ) /* need internal rom of IGS027A */

GAME( 2001, dw2001,       pgm,       kov2_disabled_arm,     sango,    dw2001,    ROT0,   "IGS", "Dragon World 2001", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING | GAME_SUPPORTS_SAVE ) // V0000 02/21/01 16:05:16

GAME( 2001, puzzli2,      pgm,       puzzli2_disabled_arm,  sango,    puzzli2,    ROT0,   "IGS", "Puzzli 2 Super (ver. 200)", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING | GAME_SUPPORTS_SAVE )

GAME( 2002, dmnfrnt,      pgm,       svg,     sango,    dmnfrnt,    ROT0,   "IGS", "Demon Front (ver. 102)", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING | GAME_SUPPORTS_SAVE ) /* need internal rom of IGS027A */
GAME( 2002, dmnfrnta,     dmnfrnt,   svg,     sango,    dmnfrnt,    ROT0,   "IGS", "Demon Front (ver. 105)", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING | GAME_SUPPORTS_SAVE ) /* need internal rom of IGS027A */

/* Games below this point are known to have an 'execute only' internal ROM area covering an area at the start of the internal ROM.  This can't be read when running code from either internal or external ROM space. */

GAME( 2003, theglad,      pgm,       svg,     sango,    theglad,    ROT0,   "IGS", "The Gladiator (ver. 100)", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING | GAME_SUPPORTS_SAVE ) /* need internal rom of IGS027A */
GAME( 2003, theglada,     theglad,   svg,     sango,    theglad,    ROT0,   "IGS", "The Gladiator (ver. 101)", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING | GAME_SUPPORTS_SAVE ) /* need internal rom of IGS027A */

GAME( 2004, oldsplus,     pgm,       svg_disabled_arm,     oldsplus,     oldsplus,   ROT0,   "IGS", "Oriental Legend Special Plus / Xi You Shi E Zhuan Super Plus", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING | GAME_SUPPORTS_SAVE ) /* need internal rom of IGS027A */

GAME( 2004, kovshp,       pgm,       kov,     sango,    kovshp,     ROT0,   "IGS", "Knights of Valour Super Heroes Plus / Sangoku Senki Super Heroes Plus (ver. 100)", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING | GAME_SUPPORTS_SAVE ) /* need internal rom of IGS027A */
// these bootlegs are clones of this instead
GAME( 2004, kovlsqh,      kovshp,     kov,     sango,	kovlsqh2,   ROT0,   "bootleg", "Knights of Valour: Luan Shi Quan Huang / Sangoku Senki: Luan Shi Quan Huang (ver. 200CN)", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING | GAME_SUPPORTS_SAVE ) /* need internal rom of IGS027A */
GAME( 2004, kovlsqh2,     kovshp,     kov,     sango,	kovlsqh2,   ROT0,   "bootleg", "Knights of Valour: Luan Shi Quan Huang 2 / Sangoku Senki: Luan Shi Quan Huang 2 (ver. 200CN)", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING | GAME_SUPPORTS_SAVE ) /* need internal rom of IGS027A */
GAME( 2004, kovlsjb,      kovshp,     kov,     sango,	kovlsqh2,   ROT0,   "bootleg", "Knights of Valour: Luan Shi Jie Ba / Sangoku Senki: Luan Shi Jie Ba (ver. 200CN, set 1)", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING | GAME_SUPPORTS_SAVE ) /* need internal rom of IGS027A */
GAME( 2004, kovlsjba,     kovshp,     kov,     sango,	kovlsqh2,   ROT0,   "bootleg", "Knights of Valour: Luan Shi Jie Ba / Sangoku Senki: Luan Shi Jie Ba (ver. 200CN, set 2)", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING | GAME_SUPPORTS_SAVE ) /* need internal rom of IGS027A */

GAME( 2005, killbldp,     pgm,       svg,     sango,    killbldp,   ROT0,   "IGS", "The Killing Blade Plus (ver. 300)", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING | GAME_SUPPORTS_SAVE ) /* need internal rom of IGS027A */

GAME( 2004, happy6,       pgm,       svg,     sango,    svg,        ROT0,   "IGS", "Happy 6-in-1 (ver. 101CN)", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING | GAME_SUPPORTS_SAVE ) /* need internal rom of IGS027A */

GAME( 2005, svg,          pgm,       svg,     sango,    svg,        ROT0,   "IGS", "S.V.G. - Spectral vs Generation (ver. 200)", GAME_IMPERFECT_SOUND | GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING | GAME_SUPPORTS_SAVE ) /* need internal rom of IGS027A */

/* these don't use an External ARM rom, and don't have any weak internal functions which would allow the internal ROM to be read out */

GAME( 2002, ddpdoj,       0,         cavepgm,    pgm,     ddp3,      ROT270, "Cave", "DoDonPachi Dai-Ou-Jou V101 (2002.04.05.Master Ver)", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE ) // is there a v101 without the . after 05?
GAME( 2002, ddpdoja,    ddpdoj,      cavepgm,    pgm,     ddp3,      ROT270, "Cave", "DoDonPachi Dai-Ou-Jou V100 (2002.04.05.Master Ver)", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
GAME( 2002, ddpdojb,    ddpdoj,      cavepgm,    pgm,     ddp3,      ROT270, "Cave", "DoDonPachi Dai-Ou-Jou (2002.04.05 Master Ver)",      GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
GAME( 2002, ddpdojblk,  ddpdoj,      cavepgm,    pgm,     ddp3,      ROT270, "Cave", "DoDonPachi Dai-Ou-Jou (Black Label)",                GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE ) // Displays "2002.04.05.Master Ver" (old) or "2002.10.07 Black Ver" (new)

// the exact text of the 'version' shows which revision of the game it is; the newest has 2 '.' symbols in the string, the oldest, none.
GAME( 2002, ket,          0,         cavepgm,    pgm,     ket,       ROT270, "Cave", "Ketsui: Kizuna Jigoku Tachi (2003/01/01. Master Ver.)", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
GAME( 2002, keta,         ket,       cavepgm,    pgm,     ket,       ROT270, "Cave", "Ketsui: Kizuna Jigoku Tachi (2003/01/01 Master Ver.)",  GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
GAME( 2002, ketb,         ket,       cavepgm,    pgm,     ket,       ROT270, "Cave", "Ketsui: Kizuna Jigoku Tachi (2003/01/01 Master Ver)",   GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )

GAME( 2003, espgal,       0,         cavepgm,    pgm,     espgal,    ROT270, "Cave", "Espgaluda (2003/10/15 Master Ver)", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )

