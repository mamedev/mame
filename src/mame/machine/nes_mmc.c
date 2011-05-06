/*****************************************************************************************

    NES MMC Emulation


    MESS source file to handle various Multi-Memory Controllers (aka Mappers) used by NES carts.

    Many information about the mappers below come from the wonderful doc written by Disch.
    Current info (when used) are based on v0.6.1 of his docs.
    You can find the latest version of the doc at http://www.romhacking.net/docs/362/


    Missing components:
    * Currently, we are not emulating 4-screen mirroring (only 3 games use it)
    * We need a better way to implement CPU cycle based IRQ. Currently we update their counter once every scanline
      but, with this approach, every time 114 clocks have passed and for certain games it is too much!
    * We need to emulate additional sound hardware in mappers 019, 024, 026, 069, 085 (and possibly other ones)
    * We need to emulate EEPROM in mapper 016 and 159
    * Barcode World (mapper 069) and Datach games (mapper 157) need Barcode Reader emulation
      Notice that for mapper 157, the barcode reader enters in the irq, so glitches may be related to this!
    * Karaoke Studio (mapper 188) misses input devices

    Known issues on specific mappers:

    * 000 F1 Race requires more precise PPU timing. It currently has plenty of 1-line glitches.
    * 001 Yoshi flashes in-game. Back to the Future have heavily corrupted graphics (since forever).
    * 001 SOROM boards do not properly handle the two WRAM/Battery banks
    * 002, 003, 094, 097, 152 Bus conflict?
    * 003 Firehouse Rescue has flashing graphics
    * 004 Mendel Palace has never worked properly
    * 004 Ninja Gaiden 2 has flashing bg graphics in the second level
    * 005 has issues (see e.g. Just Breed or Metal Slader Glory), RAM banking needs hardware flags to determine size
    * 007 Marble Madness has small graphics corruptions
    * 014 in-game graphics is glitched
    * 015 Shanghai Tycoon has corrupted graphics
    * 033 has still some graphics problem (e.g. missing text in Akira)
    * 034 Impossible Mission II does not show graphics
    * 038 seems to miss inputs. separate reads?
    * 042 Ai Senshi Nicol has broken graphics (our Mapper 42 implementation does not handle CHR banks)
    * 048 Don Doko Don 2 freezes when you reach the first boss
    * 051 only half of the games work
    * 064 has many IRQ problems (due to the way we implement CPU based IRQ) - see Skull & Crossbones.
          Klax has problems as well (even if it uses scanline based IRQ, according to Disch's docs).
    * 067 some 1-line glitches that cannot be solved without a better implementation for cycle-based IRQs
    * 071 Fire Hawk is flashing all the times.
    * 072, 086, 092 lack samples support (maybe others as well)
    * 073 16 bit IRQ mode is not implemented
    * 077 Requires 4-screen mirroring. Currently, it is very glitchy
    * 083 has serious glitches
    * 088 Quinty has never worked properly
    * 096 is preliminary (no correct chr switch in connection to PPU)
    * 104 Not all the games work
    * 107 Are the scrolling glitches (check status bar) correct? NEStopia behaves similarly
    * 112 Master Shooter is not working and misses some graphics
    * 117 In-game glitches
    * 119 Pin Bot has glitches when the ball is in the upper half of the screen
    * 133 Qi Wang starts with corrupted graphics (ingame seems better)
    * 143 are Dancing Block borders (in the intro) correct?
    * 158 In addition to IRQ problems (same as 64), mirroring was just a guess (wrong?). info needed!
    * 164 preliminary - no sprites?
    * 176 has some graphics problem
    * 178 Fan Kong Jin Ying is not working (but not even in NEStopia)
    * 180 Crazy Climber controller?
    * 187, 198, 208, 215 have some PRG banking issues - preliminary!
    * 188 needs mic input (reads from 0x6000-0x7fff)
    * 197 Super Fighter 3 has some glitch in-game (maybe mirroring?)
    * 222 is only preliminar (wrong IRQ, mirroring or CHR banking?)
    * 225 115-in-1 has glitches in the menu (games seem fine)
    * 229 is preliminary
    * 230 not working yet (needs a value to flip at reset)
    * 232 has graphics glitches
    * 241 Commandos is not working (but not even in NEStopia)
    * 242 In Dragon Quest VIII graphics of the main character is not drawn (it seems similar to Shanghai Tycoon [map15]
          because in place of the missing graphics we get glitches in the left border)
    * 249 only half of the games work (and Du Bao Ying Hao seems to suffer the same problem as DQ8 and Shanghai Tycoon)
    * 255 does not really select game (same in NEStopia apparently)

    A few Mappers suffer of hardware conflict: original dumpers have used the same mapper number for more than
    a kind of boards. In these cases (and only in these cases) we exploit nes.hsi to set up accordingly
    emulation. Games which requires this hack are the following:
    * 032 - Major League needs hardwired mirroring (missing windows and glitched field in top view, otherwise)
    * 034 - Impossible Mission II is not like BxROM games (it writes to 0x7ffd-0x7fff, instead of 0x8000). It is still
          unplayable though (see above)
    * 071 - Fire Hawk is different from other Camerica games (no hardwired mirroring). Without crc_hack no helicopter graphics
    * 078 - Cosmo Carrier needs a different mirroring than Holy Diver
    * 113 - HES 6-in-1 requires mirroring (check Bookyman playfield), while other games break with this (check AV Soccer)
    * 153 - Famicom Jump II uses a different board (or the same in a very different way)
    * 242 - DQ8 has no mirroring (missing graphics is due to other reasons though)

    crc_hacks have been added also to handle a few wiring settings which would require submappers:
    * CHR protection pins for mapper 185
    * VRC-2, VRC-4 and VRC-6 line wiring

    Known issues on specific UNIF boards:
    * BMC-GS2004 is not working
    * BMC-GS2013 is not working
    * BMC-WS some games have corrupted graphics (e.g. Galaxian)
    * UNL-8237 is not working
    * UNL-KOF97 is not working

    Details to investigate:
    * 034 writes to 0x8000-0xffff should not be used for NINA-001 and BNROM, only unlicensed BxROM...
    * 144 we ignore writes to 0x8000 while NEStopia does not. is it a problem?
    * 240 we do not map writes to 0x4020-0x40ff. is this a problem?
    * some other emus uses mapper 210 for mapper 019 games without additional sound hardware and mirroring capabilities
      (in 210 mirroring is hardwired). However, simply initializing mirroring to Vertical in 019 seems to fix all glitches
      in 210 games, so there seems to be no reason to have this duplicate mapper

    Some mappers copy PRG rom in SRAM region. Below is a list of those which does it (in MESS) to further
    investigate if we are supporting them in the right way (e.g. do any of the following have conflicts with SRAM?):
    * 065 (Gimmick! breaks badly without it)
    * 040, 042, 050

    Remember that the MMC # does not equal the mapper #. In particular, Mapper 4 is
    in fact MMC3, Mapper 9 is MMC2 and Mapper 10 is MMC4. Makes perfect sense, right?

    TODO:
    - add more info
    - add missing mappers

****************************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "video/ppu2c0x.h"
#include "includes/nes.h"
#include "nes_mmc.h"
#include "sound/nes_apu.h"

#ifdef MAME_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)
#define LOG_FDS(x) do { if (VERBOSE) logerror x; } while (0)

static void ffe_irq( device_t *device, int scanline, int vblank, int blanked );
static WRITE8_HANDLER( mapper6_l_w );
static WRITE8_HANDLER( mapper6_w );
static WRITE8_HANDLER( mapper8_w );
static WRITE8_HANDLER( mapper17_l_w );

/*************************************************************

    Base emulation (see drivers/nes.c):
    memory 0x0000-0x1fff RAM
           0x2000-0x3fff PPU
           0x4000-0x4017 APU
           0x4018-0x5fff Expansion Area
           0x6000-0x7fff PRG RAM
           0x8000-0xffff PRG ROM

    nes_chr_r/w take care of RAM, nes_nt_r/w take care of PPU.
    for mapper specific emulation, we generically setup the
    following handlers:
    - nes_low_mapper_r/w take care of accesses to 0x4100-0x5fff
      (calling state->m_mmc_write_low/state->m_mmc_read_low)
    - state->m_mmc_write/read_mid take care of 0x6000-0x7fff
    - state->m_mmc_write/read take care of access to 0x8000-0xffff
      (most mappers only writes in this area)
    some mappers may access 0x4018-0x4100: this must be taken
    care separately in init_nes_core

*************************************************************/

WRITE8_HANDLER( nes_chr_w )
{
	nes_state *state = space->machine().driver_data<nes_state>();
	int bank = offset >> 10;

	if (state->m_chr_map[bank].source == CHRRAM)
	{
		state->m_chr_map[bank].access[offset & 0x3ff] = data;
	}
}

READ8_HANDLER( nes_chr_r )
{
	nes_state *state = space->machine().driver_data<nes_state>();
	int bank = offset >> 10;

	// a few CNROM boards contained copy protection schemes through
	// suitably configured diodes, so that subsequent CHR reads can
	// give actual VROM content or open bus values.
	// For most boards, chr_open_bus remains always zero.
	if (state->m_chr_open_bus)
		return 0xff;

	return state->m_chr_map[bank].access[offset & 0x3ff];
}

WRITE8_HANDLER( nes_nt_w )
{
	nes_state *state = space->machine().driver_data<nes_state>();
	int page = ((offset & 0xc00) >> 10);

	if (state->m_nt_page[page].writable == 0)
		return;

	state->m_nt_page[page].access[offset & 0x3ff] = data;
}

READ8_HANDLER( nes_nt_r )
{
	nes_state *state = space->machine().driver_data<nes_state>();
	int page = ((offset & 0xc00) >> 10);

	if (state->m_nt_page[page].source == MMC5FILL)
	{
		if ((offset & 0x3ff) >= 0x3c0)
			return state->m_MMC5_floodattr;

		return state->m_MMC5_floodtile;
	}
	return state->m_nt_page[page].access[offset & 0x3ff];
}

WRITE8_HANDLER( nes_low_mapper_w )
{
	nes_state *state = space->machine().driver_data<nes_state>();

	if (state->m_mmc_write_low)
		(*state->m_mmc_write_low)(space, offset, data);
	else
		logerror("Unimplemented LOW mapper write, offset: %04x, data: %02x\n", offset + 0x4100, data);
}

READ8_HANDLER( nes_low_mapper_r )
{
	nes_state *state = space->machine().driver_data<nes_state>();

	if (state->m_mmc_read_low)
		return (*state->m_mmc_read_low)(space, offset);
	else
		logerror("Unimplemented LOW mapper read, offset: %04x\n", offset + 0x4100);

	return 0;
}

/*************************************************************

    Helpers to handle MMC

*************************************************************/

static void wram_bank( running_machine &machine, int bank, int source )
{
	nes_state *state = machine.driver_data<nes_state>();

	assert(state->m_battery || state->m_prg_ram);
	if (source == NES_BATTERY)
	{
		bank &= (state->m_battery_size / 0x2000) - 1;
		state->m_prg_bank[4] = state->m_battery_bank5_start + bank;
	}
	else
	{
		bank &= (state->m_wram_size / 0x2000) - 1;
		state->m_prg_bank[4] = state->m_prgram_bank5_start + bank;
	}
	memory_set_bank(machine, "bank5", state->m_prg_bank[4]);
}

INLINE void prg_bank_refresh( running_machine &machine )
{
	nes_state *state = machine.driver_data<nes_state>();
	memory_set_bank(machine, "bank1", state->m_prg_bank[0]);
	memory_set_bank(machine, "bank2", state->m_prg_bank[1]);
	memory_set_bank(machine, "bank3", state->m_prg_bank[2]);
	memory_set_bank(machine, "bank4", state->m_prg_bank[3]);
}

/* PRG ROM in 8K, 16K or 32K blocks */

static void prg32( running_machine &machine, int bank )
{
	nes_state *state = machine.driver_data<nes_state>();

	/* if there is only 16k PRG, return */
	if (!(state->m_prg_chunks >> 1))
		return;

	/* assumes that bank references a 32k chunk */
	bank &= ((state->m_prg_chunks >> 1) - 1);

	state->m_prg_bank[0] = bank * 4 + 0;
	state->m_prg_bank[1] = bank * 4 + 1;
	state->m_prg_bank[2] = bank * 4 + 2;
	state->m_prg_bank[3] = bank * 4 + 3;
	prg_bank_refresh(machine);
}

static void prg16_89ab( running_machine &machine, int bank )
{
	nes_state *state = machine.driver_data<nes_state>();

	/* assumes that bank references a 16k chunk */
	bank &= (state->m_prg_chunks - 1);

	state->m_prg_bank[0] = bank * 2 + 0;
	state->m_prg_bank[1] = bank * 2 + 1;
	prg_bank_refresh(machine);
}

static void prg16_cdef( running_machine &machine, int bank )
{
	nes_state *state = machine.driver_data<nes_state>();

	/* assumes that bank references a 16k chunk */
	bank &= (state->m_prg_chunks - 1);

	state->m_prg_bank[2] = bank * 2 + 0;
	state->m_prg_bank[3] = bank * 2 + 1;
	prg_bank_refresh(machine);
}

static void prg8_89( running_machine &machine, int bank )
{
	nes_state *state = machine.driver_data<nes_state>();

	/* assumes that bank references an 8k chunk */
	bank &= ((state->m_prg_chunks << 1) - 1);

	state->m_prg_bank[0] = bank;
	prg_bank_refresh(machine);
}

static void prg8_ab( running_machine &machine, int bank )
{
	nes_state *state = machine.driver_data<nes_state>();

	/* assumes that bank references an 8k chunk */
	bank &= ((state->m_prg_chunks << 1) - 1);

	state->m_prg_bank[1] = bank;
	prg_bank_refresh(machine);
}

static void prg8_cd( running_machine &machine, int bank )
{
	nes_state *state = machine.driver_data<nes_state>();

	/* assumes that bank references an 8k chunk */
	bank &= ((state->m_prg_chunks << 1) - 1);

	state->m_prg_bank[2] = bank;
	prg_bank_refresh(machine);
}

static void prg8_ef( running_machine &machine, int bank )
{
	nes_state *state = machine.driver_data<nes_state>();

	/* assumes that bank references an 8k chunk */
	bank &= ((state->m_prg_chunks << 1) - 1);

	state->m_prg_bank[3] = bank;
	prg_bank_refresh(machine);
}

/* We define an additional helper to map PRG-ROM to 0x6000-0x7000 */
// TODO: are we implementing this correctly in the mappers which uses it? check!

static void prg8_67( running_machine &machine, int bank )
{
	nes_state *state = machine.driver_data<nes_state>();

	/* assumes that bank references an 8k chunk */
	bank &= ((state->m_prg_chunks << 1) - 1);

	state->m_prg_bank[4] = bank;
	memory_set_bank(machine, "bank5", state->m_prg_bank[4]);
}

/* We also define an additional helper to map 8k PRG-ROM to one of the banks (passed as parameter) */
static void prg8_x( running_machine &machine, int start, int bank )
{
	nes_state *state = machine.driver_data<nes_state>();

	assert(start < 4);

	/* assumes that bank references an 8k chunk */
	bank &= ((state->m_prg_chunks << 1) - 1);

	state->m_prg_bank[start] = bank;
	prg_bank_refresh(machine);
}

/* CHR ROM in 1K, 2K, 4K or 8K blocks */

// this can be probably removed later, but it is useful while testing xml and unf handling with VRAM & VROM
INLINE void chr_sanity_check( running_machine &machine, int source )
{
	nes_state *state = machine.driver_data<nes_state>();

	if (source == CHRRAM && state->m_vram == NULL)
		fatalerror("CHRRAM bankswitch with no VRAM");

	if (source == CHRROM && state->m_vrom == NULL)
		fatalerror("CHRROM bankswitch with no VROM");
}

static void chr8( running_machine &machine, int bank, int source )
{
	nes_state *state = machine.driver_data<nes_state>();
	int i;

	chr_sanity_check(machine, source);

	if (source == CHRRAM)
	{
		bank &= (state->m_vram_chunks - 1);
		for (i = 0; i < 8; i++)
		{
			state->m_chr_map[i].source = source;
			state->m_chr_map[i].origin = (bank * 0x2000) + (i * 0x400); // for save state uses!
			state->m_chr_map[i].access = &state->m_vram[state->m_chr_map[i].origin];
		}
	}
	else
	{
		bank &= (state->m_chr_chunks - 1);
		for (i = 0; i < 8; i++)
		{
			state->m_chr_map[i].source = source;
			state->m_chr_map[i].origin = (bank * 0x2000) + (i * 0x400); // for save state uses!
			state->m_chr_map[i].access = &state->m_vrom[state->m_chr_map[i].origin];
		}
	}
}

static void chr4_x( running_machine &machine, int start, int bank, int source )
{
	nes_state *state = machine.driver_data<nes_state>();
	int i;

	chr_sanity_check(machine, source);

	if (source == CHRRAM)
	{
		bank &= ((state->m_vram_chunks << 1) - 1);
		for (i = 0; i < 4; i++)
		{
			state->m_chr_map[i + start].source = source;
			state->m_chr_map[i + start].origin = (bank * 0x1000) + (i * 0x400); // for save state uses!
			state->m_chr_map[i + start].access = &state->m_vram[state->m_chr_map[i + start].origin];
		}
	}
	else
	{
		bank &= ((state->m_chr_chunks << 1) - 1);
		for (i = 0; i < 4; i++)
		{
			state->m_chr_map[i + start].source = source;
			state->m_chr_map[i + start].origin = (bank * 0x1000) + (i * 0x400); // for save state uses!
			state->m_chr_map[i + start].access = &state->m_vrom[state->m_chr_map[i + start].origin];
		}
	}
}

static void chr4_0( running_machine &machine, int bank, int source )
{
	chr4_x(machine, 0, bank, source);
}

static void chr4_4( running_machine &machine, int bank, int source )
{
	chr4_x(machine, 4, bank, source);
}

static void chr2_x( running_machine &machine, int start, int bank, int source )
{
	nes_state *state = machine.driver_data<nes_state>();
	int i;

	chr_sanity_check(machine, source);

	if (source == CHRRAM)
	{
		bank &= ((state->m_vram_chunks << 2) - 1);
		for (i = 0; i < 2; i++)
		{
			state->m_chr_map[i + start].source = source;
			state->m_chr_map[i + start].origin = (bank * 0x800) + (i * 0x400); // for save state uses!
			state->m_chr_map[i + start].access = &state->m_vram[state->m_chr_map[i + start].origin];
		}
	}
	else
	{
		bank &= ((state->m_chr_chunks << 2) - 1);
		for (i = 0; i < 2; i++)
		{
			state->m_chr_map[i + start].source = source;
			state->m_chr_map[i + start].origin = (bank * 0x800) + (i * 0x400); // for save state uses!
			state->m_chr_map[i + start].access = &state->m_vrom[state->m_chr_map[i + start].origin];
		}
	}
}

static void chr2_0( running_machine &machine, int bank, int source )
{
	chr2_x(machine, 0, bank, source);
}

static void chr2_2( running_machine &machine, int bank, int source )
{
	chr2_x(machine, 2, bank, source);
}

static void chr2_4( running_machine &machine, int bank, int source )
{
	chr2_x(machine, 4, bank, source);
}

static void chr2_6( running_machine &machine, int bank, int source )
{
	chr2_x(machine, 6, bank, source);
}

static void chr1_x( running_machine &machine, int start, int bank, int source )
{
	nes_state *state = machine.driver_data<nes_state>();

	chr_sanity_check(machine, source);

	if (source == CHRRAM)
	{
		bank &= ((state->m_vram_chunks << 3) - 1);
		state->m_chr_map[start].source = source;
		state->m_chr_map[start].origin = (bank * 0x400); // for save state uses!
		state->m_chr_map[start].access = &state->m_vram[state->m_chr_map[start].origin];
	}
	else
	{
		bank &= ((state->m_chr_chunks << 3) - 1);
		state->m_chr_map[start].source = source;
		state->m_chr_map[start].origin = (bank * 0x400); // for save state uses!
		state->m_chr_map[start].access = &state->m_vrom[state->m_chr_map[start].origin];
	}
}

static void chr1_0 (running_machine &machine, int bank, int source)
{
	chr1_x(machine, 0, bank, source);
}

static void chr1_1( running_machine &machine, int bank, int source )
{
	chr1_x(machine, 1, bank, source);
}

static void chr1_2( running_machine &machine, int bank, int source )
{
	chr1_x(machine, 2, bank, source);
}

static void chr1_3( running_machine &machine, int bank, int source )
{
	chr1_x(machine, 3, bank, source);
}

static void chr1_4( running_machine &machine, int bank, int source )
{
	chr1_x(machine, 4, bank, source);
}

static void chr1_5( running_machine &machine, int bank, int source )
{
	chr1_x(machine, 5, bank, source);
}

static void chr1_6( running_machine &machine, int bank, int source )
{
	chr1_x(machine, 6, bank, source);
}

static void chr1_7( running_machine &machine, int bank, int source )
{
	chr1_x(machine, 7, bank, source);
}


/* NameTable paging and mirroring */

static void set_nt_page( running_machine &machine, int page, int source, int bank, int writable )
{
	nes_state *state = machine.driver_data<nes_state>();
	UINT8* base_ptr;

	switch (source)
	{
		case CART_NTRAM:
			base_ptr = state->m_extended_ntram;
			break;
		case MMC5FILL:
			base_ptr = NULL;
			break;
		case ROM:
			base_ptr = state->m_vrom;
			break;
		case EXRAM:
			base_ptr = state->m_mapper_ram;
			break;
		case CIRAM:
		default:
			base_ptr = state->m_ciram;
			break;
	}

	page &= 3; /* mask down to the 4 logical pages */
	state->m_nt_page[page].source = source;

	if (base_ptr != NULL)
	{
		state->m_nt_page[page].origin = bank * 0x400;
		state->m_nt_page[page].access = base_ptr + state->m_nt_page[page].origin;
	}

	state->m_nt_page[page].writable = writable;
}

void set_nt_mirroring( running_machine &machine, int mirroring )
{
	/* setup our videomem handlers based on mirroring */
	switch (mirroring)
	{
		case PPU_MIRROR_VERT:
			set_nt_page(machine, 0, CIRAM, 0, 1);
			set_nt_page(machine, 1, CIRAM, 1, 1);
			set_nt_page(machine, 2, CIRAM, 0, 1);
			set_nt_page(machine, 3, CIRAM, 1, 1);
			break;

		case PPU_MIRROR_HORZ:
			set_nt_page(machine, 0, CIRAM, 0, 1);
			set_nt_page(machine, 1, CIRAM, 0, 1);
			set_nt_page(machine, 2, CIRAM, 1, 1);
			set_nt_page(machine, 3, CIRAM, 1, 1);
			break;

		case PPU_MIRROR_HIGH:
			set_nt_page(machine, 0, CIRAM, 1, 1);
			set_nt_page(machine, 1, CIRAM, 1, 1);
			set_nt_page(machine, 2, CIRAM, 1, 1);
			set_nt_page(machine, 3, CIRAM, 1, 1);
			break;

		case PPU_MIRROR_LOW:
			set_nt_page(machine, 0, CIRAM, 0, 1);
			set_nt_page(machine, 1, CIRAM, 0, 1);
			set_nt_page(machine, 2, CIRAM, 0, 1);
			set_nt_page(machine, 3, CIRAM, 0, 1);
			break;

		case PPU_MIRROR_NONE:
		case PPU_MIRROR_4SCREEN:
		default:
			/* external RAM needs to be used somehow. */
			/* but as a default, we'll arbitrarily set vertical so as not to crash*/
			/* mappers should use set_nt_page and assign which pages are which */

			logerror("Mapper set 4-screen mirroring without supplying external nametable memory!\n");

			set_nt_page(machine, 0, CIRAM, 0, 1);
			set_nt_page(machine, 1, CIRAM, 0, 1);
			set_nt_page(machine, 2, CIRAM, 1, 1);
			set_nt_page(machine, 3, CIRAM, 1, 1);
			break;
	}
}

/*  Other custom mirroring helpers are defined below: Waixing games use waixing_set_mirror (which swaps
    MIRROR_HIGH and MIRROR_LOW compared to the above) and Sachen games use sachen_set_mirror (which has
    a slightly different MIRROR_HIGH, with page 0 set to 0) */


/*************************************************************

 Support for xml list

 *************************************************************/

/* Include emulation of NES PCBs for softlist */
#include "machine/nes_pcb.c"


/*************************************************************

    Support for .unf Files

*************************************************************/

/* Include emulation of UNIF Boards for .unf files */
#include "machine/nes_unif.c"


/*************************************************************

 Support for .nes Files

 *************************************************************/

/* Include emulation of iNES Mappers for .nes files */
#include "machine/nes_ines.c"

