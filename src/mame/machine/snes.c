/***************************************************************************

  snes.c

  Machine file to handle emulation of the Nintendo Super NES

  R. Belmont
  Anthony Kruize
  Harmony
  Based on the original code by Lee Hammerton (aka Savoury Snax)
  Thanks to Anomie for invaluable technical information.
  Thanks to byuu for invaluable technical information.

***************************************************************************/
#define __MACHINE_SNES_C

#include "emu.h"
#include "cpu/superfx/superfx.h"
#include "cpu/g65816/g65816.h"
#include "includes/snes.h"
#include "audio/snes_snd.h"


/* -- Globals -- */
UINT8  *snes_ram = NULL;		/* 65816 ram */
UINT8  *snes_vram = NULL;		/* Video RAM (Should be 16-bit, but it's easier this way) */
UINT16 *snes_cgram = NULL;		/* Colour RAM */
UINT16 *snes_oam = NULL;		/* Object Attribute Memory */

UINT8 snes_has_addon_chip;
UINT32 snes_rom_size;

// full graphic variables
static const UINT16 vram_fgr_inctab[4] = { 1, 32, 128, 128 };
static const UINT16 vram_fgr_inccnts[4] = { 0, 32, 64, 128 };
static const UINT16 vram_fgr_shiftab[4] = { 0, 5, 6, 7 };

static void snes_dma(const address_space *space, UINT8 channels);
static void snes_hdma_init(running_machine *machine);
static void snes_hdma(const address_space *space);

struct snes_cart_info snes_cart;

// add-on chip emulators
#include "machine/snesdsp1.c"
#include "machine/snesdsp2.c"
#include "machine/snesdsp3.c"
#include "machine/snesdsp4.c"
#include "machine/snesobc1.c"
#include "machine/snescx4.c"
#include "machine/snesrtc.c"
#include "machine/snessdd1.c"
#include "machine/snes7110.c"
#include "machine/snesst10.c"

/*************************************

    Timers

*************************************/

// utility function - latches the H/V counters.  Used by IRQ, writes to WRIO, etc.
void snes_latch_counters( running_machine *machine )
{
	snes_state *state = (snes_state *)machine->driver_data;

	snes_ppu.beam.current_horz = video_screen_get_hpos(machine->primary_screen) / state->htmult;
	snes_ppu.beam.latch_vert = video_screen_get_vpos(machine->primary_screen);
	snes_ppu.beam.latch_horz = snes_ppu.beam.current_horz;
	snes_ram[STAT78] |= 0x40;	// indicate we latched
//  state->read_ophct = state->read_opvct = 0;    // clear read flags - 2009-08: I think we must clear these when STAT78 is read...

//  printf("latched @ H %d V %d\n", snes_ppu.beam.latch_horz, snes_ppu.beam.latch_vert);
}

static TIMER_CALLBACK( snes_nmi_tick )
{
	snes_state *state = (snes_state *)machine->driver_data;

	// pull NMI
	cpu_set_input_line(state->maincpu, G65816_LINE_NMI, ASSERT_LINE);

	// don't happen again
	timer_adjust_oneshot(state->nmi_timer, attotime_never, 0);
}

static void snes_hirq_tick( running_machine *machine )
{
	snes_state *state = (snes_state *)machine->driver_data;

	// latch the counters and pull IRQ
	// (don't need to switch to the 65816 context, we don't do anything dependant on it)
	snes_latch_counters(machine);
	snes_ram[TIMEUP] = 0x80;	/* Indicate that irq occured */
	cpu_set_input_line(state->maincpu, G65816_LINE_IRQ, ASSERT_LINE);

	// don't happen again
	timer_adjust_oneshot(state->hirq_timer, attotime_never, 0);
}

static TIMER_CALLBACK( snes_hirq_tick_callback )
{
	snes_hirq_tick(machine);
}

static TIMER_CALLBACK( snes_reset_oam_address )
{
	snes_state *state = (snes_state *)machine->driver_data;
	// make sure we're in the 65816's context since we're messing with the OAM and stuff
	const address_space *space = cpu_get_address_space(state->maincpu, ADDRESS_SPACE_PROGRAM);

	if (!(snes_ppu.screen_disabled)) //Reset OAM address, byuu says it happens at H=10
	{
		memory_write_byte(space, OAMADDL, snes_ppu.oam.saved_address_low); /* Reset oam address */
		memory_write_byte(space, OAMADDH, snes_ppu.oam.saved_address_high);
		snes_ppu.oam.first_sprite = snes_ppu.oam.priority_rotation ? (snes_ppu.oam.address >> 1) & 127 : 0;
	}
}

static TIMER_CALLBACK( snes_reset_hdma )
{
	snes_hdma_init(machine);
}

static TIMER_CALLBACK( snes_scanline_tick )
{
	snes_state *state = (snes_state *)machine->driver_data;

	/* Increase current line - we want to latch on this line during it, not after it */
	snes_ppu.beam.current_vert = video_screen_get_vpos(machine->primary_screen);

	// not in hblank
	snes_ram[HVBJOY] &= ~0x40;

	/* Vertical IRQ timer - only if horizontal isn't also enabled! */
	if ((snes_ram[NMITIMEN] & 0x20) && !(snes_ram[NMITIMEN] & 0x10))
	{
		if (snes_ppu.beam.current_vert == (((snes_ram[VTIMEH] << 8) | snes_ram[VTIMEL]) & 0x1ff))
		{
			snes_ram[TIMEUP] = 0x80;	/* Indicate that irq occured */
			// IRQ latches the counters, do it now
			snes_latch_counters(machine);
			cpu_set_input_line(state->maincpu, G65816_LINE_IRQ, ASSERT_LINE );
		}
	}
	/* Horizontal IRQ timer */
	if (snes_ram[NMITIMEN] & 0x10)
	{
		int setirq = 1;
		int pixel = ((snes_ram[HTIMEH] << 8) | snes_ram[HTIMEL]) & 0x1ff;

		// is the HIRQ on a specific scanline?
		if (snes_ram[NMITIMEN] & 0x20)
		{
			if (snes_ppu.beam.current_vert != (((snes_ram[VTIMEH] << 8) | snes_ram[VTIMEL]) & 0x1ff) )
			{
				setirq = 0;
			}
		}

		if (setirq)
		{
//          printf("HIRQ @ %d, %d\n", pixel * state->htmult, snes_ppu.beam.current_vert);
			if (pixel == 0)
			{
				snes_hirq_tick(machine);
			}
			else
			{
				timer_adjust_oneshot(state->hirq_timer, video_screen_get_time_until_pos(machine->primary_screen, snes_ppu.beam.current_vert, pixel * state->htmult), 0);
			}
		}
	}

	/* Start of VBlank */
	if (snes_ppu.beam.current_vert == snes_ppu.beam.last_visible_line)
	{
		timer_set(machine, video_screen_get_time_until_pos(machine->primary_screen, snes_ppu.beam.current_vert, 10), NULL, 0, snes_reset_oam_address);

		snes_ram[HVBJOY] |= 0x81;		/* Set vblank bit to on & indicate controllers being read */
		snes_ram[RDNMI] |= 0x80;		/* Set NMI occured bit */

		if (snes_ram[NMITIMEN] & 0x80)	/* NMI only signaled if this bit set */
		{
			// NMI goes off about 12 cycles after this (otherwise Chrono Trigger, NFL QB Club, etc. lock up)
			timer_adjust_oneshot(state->nmi_timer, cpu_clocks_to_attotime(state->maincpu, 12), 0);
		}
	}

	// hdma reset happens at scanline 0, H=~6
	if (snes_ppu.beam.current_vert == 0)
	{
		snes_hdma_init(machine);
	}

	/* three lines after start of vblank we update the controllers (value from snes9x) */
	if (snes_ppu.beam.current_vert == snes_ppu.beam.last_visible_line + 2)
	{
		state->io_read(machine);
		snes_ram[HVBJOY] &= 0xfe;		/* Clear busy bit */
	}

	if (snes_ppu.beam.current_vert == 0)
	{	/* VBlank is over, time for a new frame */
		snes_ram[HVBJOY] &= 0x7f;		/* Clear vblank bit */
		snes_ram[RDNMI]  &= 0x7f;		/* Clear nmi occured bit */
		snes_ram[STAT78] ^= 0x80;		/* Toggle field flag */
		snes_ppu.stat77_flags &= 0x3f;	/* Clear Time Over and Range Over bits */

		cpu_set_input_line(state->maincpu, G65816_LINE_NMI, CLEAR_LINE );
	}

	timer_adjust_oneshot(state->scanline_timer, attotime_never, 0);
	timer_adjust_oneshot(state->hblank_timer, video_screen_get_time_until_pos(machine->primary_screen, snes_ppu.beam.current_vert, state->hblank_offset * state->htmult), 0);
}

/* This is called at the start of hblank *before* the scanline indicated in current_vert! */
static TIMER_CALLBACK( snes_hblank_tick )
{
	snes_state *state = (snes_state *)machine->driver_data;
	const address_space *cpu0space = cpu_get_address_space(state->maincpu, ADDRESS_SPACE_PROGRAM);
	int nextscan;

	snes_ppu.beam.current_vert = video_screen_get_vpos(machine->primary_screen);

	/* make sure we halt */
	timer_adjust_oneshot(state->hblank_timer, attotime_never, 0);

	/* draw a scanline */
	if (snes_ppu.beam.current_vert <= snes_ppu.beam.last_visible_line)
	{
		if (video_screen_get_vpos(machine->primary_screen) > 0)
		{
			/* Do HDMA */
			if (state->hdmaen)
				snes_hdma(cpu0space);

			video_screen_update_partial(machine->primary_screen, (snes_ppu.interlace == 2) ? (snes_ppu.beam.current_vert*snes_ppu.interlace) : snes_ppu.beam.current_vert-1);
		}
	}

	// signal hblank
	snes_ram[HVBJOY] |= 0x40;

	/* kick off the start of scanline timer */
	nextscan = snes_ppu.beam.current_vert + 1;
	if (nextscan >= (((snes_ram[STAT78] & 0x10) == SNES_NTSC) ? SNES_VTOTAL_NTSC : SNES_VTOTAL_PAL))
	{
		nextscan = 0;
	}

	timer_adjust_oneshot(state->scanline_timer, video_screen_get_time_until_pos(machine->primary_screen, nextscan, 0), 0);
}

/* FIXME: multiplication should take 8 CPU cycles & division 16 CPU cycles, but
using these timers breaks e.g. Chrono Trigger intro and Super Tennis gameplay.
On the other hand, timers are needed for the translation of Breath of Fire 2
to work. More weirdness: we might need to leave 8 CPU cycles for division at
first, since using 16 produces bugs (see e.g. Triforce pieces in Zelda 3 intro) */

static TIMER_CALLBACK(snes_div_callback)
{
	UINT16 value, dividend, remainder;
	dividend = remainder = 0;
	value = (snes_ram[WRDIVH] << 8) + snes_ram[WRDIVL];
	if (snes_ram[WRDVDD] > 0)
	{
		dividend = value / snes_ram[WRDVDD];
		remainder = value % snes_ram[WRDVDD];
	}
	else
	{
		dividend = 0xffff;
		remainder = value;
	}
	snes_ram[RDDIVL] = dividend & 0xff;
	snes_ram[RDDIVH] = (dividend >> 8) & 0xff;
	snes_ram[RDMPYL] = remainder & 0xff;
	snes_ram[RDMPYH] = (remainder >> 8) & 0xff;
}


static TIMER_CALLBACK(snes_mult_callback)
{
	UINT32 c = snes_ram[WRMPYA] * snes_ram[WRMPYB];
	snes_ram[RDMPYL] = c & 0xff;
	snes_ram[RDMPYH] = (c >> 8) & 0xff;
}


/*************************************

    Input Handlers

*************************************/

static void snes_dynamic_res_change( running_machine *machine )
{
	snes_state *state = (snes_state *)machine->driver_data;
	rectangle visarea = *video_screen_get_visible_area(machine->primary_screen);
	attoseconds_t refresh;

	visarea.min_x = visarea.min_y = 0;
	visarea.max_y = snes_ppu.beam.last_visible_line*snes_ppu.interlace - 1;
	visarea.max_x = (SNES_SCR_WIDTH * 2) - 1;

	// fixme: should compensate for SNES_DBG_video
	if (snes_ppu.mode == 5 || snes_ppu.mode == 6 || snes_ppu.pseudo_hires)
		state->htmult = 2;
	else
		state->htmult = 1;

	/* FIXME: does the timing changes when the gfx mode is equal to 5 or 6? */
	if ((snes_ram[STAT78] & 0x10) == SNES_NTSC)
		refresh = HZ_TO_ATTOSECONDS(DOTCLK_NTSC) * SNES_HTOTAL * SNES_VTOTAL_NTSC;
	else
		refresh = HZ_TO_ATTOSECONDS(DOTCLK_PAL) * SNES_HTOTAL * SNES_VTOTAL_PAL;

	if ((snes_ram[STAT78] & 0x10) == SNES_NTSC)
		video_screen_configure(machine->primary_screen, SNES_HTOTAL*2, SNES_VTOTAL_NTSC*snes_ppu.interlace, &visarea, refresh);
	else
		video_screen_configure(machine->primary_screen, SNES_HTOTAL*2, SNES_VTOTAL_PAL*snes_ppu.interlace, &visarea, refresh);
}

static READ8_HANDLER( snes_open_bus_r )
{
	static UINT8 recurse = 0;
	UINT16 result;

	/* prevent recursion */
	if (recurse)
		return 0xff;

	recurse = 1;
	result = memory_read_byte_8le(space, cpu_get_pc(space->cpu) - 1); //LAST opcode that's fetched on the bus
	recurse = 0;
	return result;
}

/*************************************************

 SNES VRAM accesses:

 VRAM accesses during active display are invalid. 
 Unlike OAM and CGRAM, they will not be written 
 anywhere at all. Thanks to byuu's researches, 
 the ranges where writes are invalid have been 
 validated on hardware, as has the edge case where 
 the S-CPU open bus can be written if the write 
 occurs during the very last clock cycle of 
 vblank.
 Our implementation could be not 100% accurate
 when interlace is active.
*************************************************/

INLINE UINT32 snes_get_vram_address( running_machine *machine )
{
	snes_state *state = (snes_state *)machine->driver_data;
	UINT32 addr = (snes_ram[VMADDH] << 8) | snes_ram[VMADDL];

	if (state->vram_fgr_count)
	{
		UINT32 rem = addr & state->vram_fgr_mask;
		UINT32 faddr = (addr & ~state->vram_fgr_mask) + (rem >> state->vram_fgr_shift) + ((rem & (state->vram_fgr_count - 1)) << 3);
		return faddr;
	}

	return addr;
}

static READ8_HANDLER( snes_vram_read )
{
	UINT8 res = 0; 
	offset &= 0x1ffff;

	if (snes_ppu.screen_disabled)
		res = snes_vram[offset];
	else
	{
		UINT16 v = video_screen_get_vpos(space->machine->primary_screen);
		UINT16 h = video_screen_get_hpos(space->machine->primary_screen);
		UINT16 ls = (((snes_ram[STAT78] & 0x10) == SNES_NTSC ? 525 : 625) >> 1) - 1;

		if (snes_ppu.interlace == 2)
			ls++;

		if (v == ls && h == 1362) 
			res = 0;
		else if (v < snes_ppu.beam.last_visible_line - 1) 
			res = 0;
		else if (v == snes_ppu.beam.last_visible_line - 1) 
		{
			if (h == 1362) 
				res = snes_vram[offset];
			else 
				res = 0;
		} 
		else 
			res = snes_vram[offset];
	}
	return res;
}

static WRITE8_HANDLER( snes_vram_write )
{
	offset &= 0x1ffff;

	if (snes_ppu.screen_disabled)
		snes_vram[offset] = data;
	else
	{
		UINT16 v = video_screen_get_vpos(space->machine->primary_screen);
		UINT16 h = video_screen_get_hpos(space->machine->primary_screen);
		if (v == 0) 
		{
			if (h <= 4) 
				snes_vram[offset] = data;
			else if (h == 6) 
				snes_vram[offset] = snes_open_bus_r(space, 0);
			else 
			{
				//no write
			}
		} 
		else if (v < snes_ppu.beam.last_visible_line) 
		{
			//no write
		} 
		else if (v == snes_ppu.beam.last_visible_line) 
		{
			if (h <= 4) 
			{
				//no write
			} 
			else 
				snes_vram[offset] = data;
		}
		else 
			snes_vram[offset] = data;
	}
}

/*
 * DR   - Double read : address is read twice to return a 16bit value.
 * low  - This is the low byte of a 16 or 24 bit value
 * mid  - This is the middle byte of a 24 bit value
 * high - This is the high byte of a 16 or 24 bit value
 */
READ8_HANDLER( snes_r_io )
{
	snes_state *state = (snes_state *)space->machine->driver_data;
	UINT8 value = 0;

	// APU is mirrored from 2140 to 217f
	if (offset >= APU00 && offset < WMDATA)
	{
		return spc_port_out(state->spc700, offset & 0x3);
	}

	if (snes_has_addon_chip == HAS_SUPERFX && state->superfx != NULL)
	{
		if (offset >= 0x3000 && offset < 0x3300)
		{
			return superfx_mmio_read(state->superfx, offset);
		}
	}
	else if (snes_has_addon_chip == HAS_RTC)
	{
		if (offset == 0x2800 || offset == 0x2801)
		{
			return srtc_read(space->machine, offset);
		}
	}
	else if (snes_has_addon_chip == HAS_SDD1)
	{
		if (offset >= 0x4800 && offset < 0x4808)
		{
			return sdd1_mmio_read(space, (UINT32)offset);
		}
		if (offset < 0x80)
		{
			offset += 0x4300;
		}
	}
	else if (snes_has_addon_chip == HAS_SPC7110 || snes_has_addon_chip == HAS_SPC7110_RTC)
	{
		UINT16 limit = (snes_has_addon_chip == HAS_SPC7110_RTC) ? 0x4842 : 0x483f;
		if (offset >= 0x4800 && offset < limit)
		{
			return spc7110_mmio_read(space->machine, offset);
		}
	}

	/* offset is from 0x000000 */
	switch (offset)
	{
		case OAMDATA:	/* 21xy for x=0,1,2 and y=4,5,6,8,9,a returns PPU1 open bus*/
		case BGMODE:
		case MOSAIC:
		case BG2SC:
		case BG3SC:
		case BG4SC:
		case BG4VOFS:
		case VMAIN:
		case VMADDL:
		case VMDATAL:
		case VMDATAH:
		case M7SEL:
		case W34SEL:
		case WOBJSEL:
		case WH0:
		case WH2:
		case WH3:
		case WBGLOG:
			return snes_ppu.ppu1_open_bus;

		case MPYL:		/* Multiplication result (low) */
			{
				/* Perform 16bit * 8bit multiply */
				UINT32 c = (INT16)snes_ppu.mode7.matrix_a * (INT8)(snes_ppu.mode7.matrix_b >> 8);
				snes_ppu.ppu1_open_bus = c & 0xff;
				return snes_ppu.ppu1_open_bus;
			}
		case MPYM:		/* Multiplication result (mid) */
			{
				/* Perform 16bit * 8bit multiply */
				UINT32 c = (INT16)snes_ppu.mode7.matrix_a * (INT8)(snes_ppu.mode7.matrix_b >> 8);
				snes_ppu.ppu1_open_bus = (c >> 8) & 0xff;
				return snes_ppu.ppu1_open_bus;
			}
		case MPYH:		/* Multiplication result (high) */
			{
				/* Perform 16bit * 8bit multiply */
				UINT32 c = (INT16)snes_ppu.mode7.matrix_a * (INT8)(snes_ppu.mode7.matrix_b >> 8);
				snes_ppu.ppu1_open_bus = (c >> 16) & 0xff;
				return snes_ppu.ppu1_open_bus;
			}
		case SLHV:		/* Software latch for H/V counter */
			snes_latch_counters(space->machine);
			return snes_open_bus_r(space, 0);		/* Return value is meaningless */
		case ROAMDATA:	/* Read data from OAM (DR) */
			{
				int oam_addr = snes_ppu.oam.address;

				if (oam_addr & 0x100)
				{
					oam_addr &= 0x10f;
				}
				else
				{
					oam_addr &= 0x1ff;
				}

				snes_ppu.ppu1_open_bus = (snes_oam[oam_addr] >> (snes_ram[OAMDATA] << 3)) & 0xff;
				snes_ram[OAMDATA] = (snes_ram[OAMDATA] + 1) % 2;
				if (!snes_ram[OAMDATA])
				{
					snes_ppu.oam.address++;
					snes_ppu.oam.address &= 0x1ff;
					snes_ppu.oam.first_sprite = snes_ppu.oam.priority_rotation ? (snes_ppu.oam.address >> 1) & 127 : 0;
				}
				return snes_ppu.ppu1_open_bus;
			}
		case RVMDATAL:	/* Read data from VRAM (low) */
			{
				UINT32 addr = snes_get_vram_address(space->machine) << 1;
				snes_ppu.ppu1_open_bus = state->vram_read_buffer & 0xff;

				if (!state->vram_fgr_high)
				{
					state->vram_read_buffer = snes_vram_read(space, addr);
					state->vram_read_buffer |= (snes_vram_read(space, addr + 1) << 8);

					addr = ((snes_ram[VMADDH] << 8) | snes_ram[VMADDL]) + state->vram_fgr_increment;
					snes_ram[VMADDL] = addr & 0xff;
					snes_ram[VMADDH] = (addr >> 8) & 0xff;
				}

				return snes_ppu.ppu1_open_bus;
			}
		case RVMDATAH:	/* Read data from VRAM (high) */
			{
				UINT32 addr = snes_get_vram_address(space->machine) << 1;
				snes_ppu.ppu1_open_bus = (state->vram_read_buffer >> 8) & 0xff;

				if (state->vram_fgr_high)
				{
					state->vram_read_buffer = snes_vram_read(space, addr);
					state->vram_read_buffer |= (snes_vram_read(space, addr + 1) << 8);

					addr = ((snes_ram[VMADDH] << 8) | snes_ram[VMADDL]) + state->vram_fgr_increment;
					snes_ram[VMADDL] = addr & 0xff;
					snes_ram[VMADDH] = (addr >> 8) & 0xff;
				}

				return snes_ppu.ppu1_open_bus;
			}
		case RCGDATA:	/* Read data from CGRAM */
				if (!(state->cgram_address & 0x01))
				{
					snes_ppu.ppu2_open_bus = ((UINT8 *)snes_cgram)[state->cgram_address] & 0xff;
				}
				else
				{
					snes_ppu.ppu2_open_bus &= 0x80;
					snes_ppu.ppu2_open_bus |= ((UINT8 *)snes_cgram)[state->cgram_address] & 0x7f;
				}

				state->cgram_address = (state->cgram_address + 1) % (SNES_CGRAM_SIZE - 2);
				return snes_ppu.ppu2_open_bus;
		case OPHCT:		/* Horizontal counter data by ext/soft latch */
			{
				if (state->read_ophct)
				{
					snes_ppu.ppu2_open_bus &= 0xfe;
					snes_ppu.ppu2_open_bus |= (snes_ppu.beam.latch_horz >> 8) & 0x01;
				}
				else
				{
					snes_ppu.ppu2_open_bus = snes_ppu.beam.latch_horz & 0xff;
				}
				state->read_ophct ^= 1;
				return snes_ppu.ppu2_open_bus;
			}
		case OPVCT:		/* Vertical counter data by ext/soft latch */
			{
				if (state->read_opvct)
				{
					snes_ppu.ppu2_open_bus &= 0xfe;
					snes_ppu.ppu2_open_bus |= (snes_ppu.beam.latch_vert >> 8) & 0x01;
				}
				else
				{
					snes_ppu.ppu2_open_bus = snes_ppu.beam.latch_vert & 0xff;
				}
				state->read_opvct ^= 1;
				return snes_ppu.ppu2_open_bus;
			}
		case STAT77:	/* PPU status flag and version number */
			value = snes_ppu.stat77_flags & 0xc0; // 0x80 & 0x40 are Time Over / Range Over Sprite flags, set by the video code
			// 0x20 - Master/slave mode select. Little is known about this bit. We always seem to read back 0 here.
			value |= (snes_ppu.ppu1_open_bus & 0x10);
			value |= (snes_ppu.ppu1_version & 0x0f);
			snes_ppu.stat77_flags = value;	// not sure if this is needed...
			snes_ppu.ppu1_open_bus = value;
			return snes_ppu.ppu1_open_bus;
		case STAT78:	/* PPU status flag and version number */
			state->read_ophct = 0;
			state->read_opvct = 0;
			value = snes_ram[offset];
			value |= (snes_ppu.ppu2_open_bus & 0x20);
			value |= (snes_ppu.ppu2_version & 0x0f);
			snes_ram[offset] = value;	// not sure if this is needed...
			snes_ppu.ppu2_open_bus = value;
			return snes_ppu.ppu2_open_bus;
		case WMDATA:	/* Data to read from WRAM */
			{
				UINT32 addr = ((snes_ram[WMADDH] & 0x1) << 16) | (snes_ram[WMADDM] << 8) | snes_ram[WMADDL];

				value = memory_read_byte(space, 0x7e0000 + addr++);
				addr &= 0x1ffff;
				snes_ram[WMADDH] = (addr >> 16) & 0x1;
				snes_ram[WMADDM] = (addr >> 8) & 0xff;
				snes_ram[WMADDL] = addr & 0xff;
				return value;
			}
		case WMADDL:	/* Address to read/write to wram (low) */
		case WMADDM:	/* Address to read/write to wram (mid) */
		case WMADDH:	/* Address to read/write to wram (high) */
			return snes_ram[offset];
		case OLDJOY1:	/* Data for old NES controllers (JOYSER1) */
			{
				if (snes_ram[offset] & 0x1)
				{
					return 0 | (snes_open_bus_r(space, 0) & 0xfc); //correct?
				}

				value = state->oldjoy1_read(space->machine);

				return (value & 0x03) | (snes_open_bus_r(space, 0) & 0xfc); //correct?
			}
		case OLDJOY2:	/* Data for old NES controllers (JOYSER2) */
			{
				if (snes_ram[OLDJOY1] & 0x1)
				{
					return 0 | 0x1c | (snes_open_bus_r(space, 0) & 0xe0); //correct?
				}

				value = state->oldjoy2_read(space->machine);

				return value | 0x1c | (snes_open_bus_r(space, 0) & 0xe0); //correct?
			}
		case HTIMEL:
		case HTIMEH:
		case VTIMEL:
		case VTIMEH:
			return snes_ram[offset];
		case RDNMI:			/* NMI flag by v-blank and version number */
			value = (snes_ram[offset] & 0x8f) | (snes_open_bus_r(space, 0) & 0x70);
			snes_ram[offset] &= 0x7f;	/* NMI flag is reset on read */
			return value;
		case TIMEUP:		/* IRQ flag by H/V count timer */
			value = (snes_open_bus_r(space, 0) & 0x7f) | (snes_ram[TIMEUP] & 0x80);
			cpu_set_input_line(state->maincpu, G65816_LINE_IRQ, CLEAR_LINE );
			snes_ram[TIMEUP] = 0;
			return value;
		case HVBJOY:		/* H/V blank and joypad controller enable */
			// electronics test says hcounter 272 is start of hblank, which is beampos 363
//          if (video_screen_get_hpos(space->machine->primary_screen) >= 363) snes_ram[offset] |= 0x40;
//              else snes_ram[offset] &= ~0x40;
			return (snes_ram[offset] & 0xc1) | (snes_open_bus_r(space, 0) & 0x3e);
		case RDIO:			/* Programmable I/O port - echos back what's written to WRIO */
			return snes_ram[WRIO];
		case RDDIVL:		/* Quotient of divide result (low) */
		case RDDIVH:		/* Quotient of divide result (high) */
		case RDMPYL:		/* Product/Remainder of mult/div result (low) */
		case RDMPYH:		/* Product/Remainder of mult/div result (high) */
			return snes_ram[offset];
		case JOY1L:			/* Joypad 1 status register (low) */
			return state->joy1l;
		case JOY1H:			/* Joypad 1 status register (high) */
			return state->joy1h;
		case JOY2L:			/* Joypad 2 status register (low) */
			return state->joy2l;
		case JOY2H:			/* Joypad 2 status register (high) */
			return state->joy2h;
		case JOY3L:			/* Joypad 3 status register (low) */
			return state->joy3l;
		case JOY3H:			/* Joypad 3 status register (high) */
			return state->joy3h;
		case JOY4L:			/* Joypad 4 status register (low) */
			return state->joy4l;
		case JOY4H:			/* Joypad 4 status register (high) */
			return state->joy4h;
		case DMAP0:	case DMAP1: case DMAP2: case DMAP3: /*0x43n0*/
		case DMAP4: case DMAP5: case DMAP6: case DMAP7:
			return state->dma_channel[(offset & 0x70) >> 4].dmap;
		case BBAD0: case BBAD1: case BBAD2: case BBAD3: /*0x43n1*/
		case BBAD4: case BBAD5: case BBAD6: case BBAD7: 
			return state->dma_channel[(offset & 0x70) >> 4].dest_addr;
		case A1T0L: case A1T1L: case A1T2L: case A1T3L: /*0x43n2*/
		case A1T4L: case A1T5L: case A1T6L: case A1T7L: 
			return state->dma_channel[(offset & 0x70) >> 4].src_addr & 0xff;
		case A1T0H: case A1T1H: case A1T2H: case A1T3H: /*0x43n3*/
		case A1T4H: case A1T5H: case A1T6H: case A1T7H: 
			return (state->dma_channel[(offset & 0x70) >> 4].src_addr >> 8) & 0xff;
		case A1B0: case A1B1: case A1B2: case A1B3:     /*0x43n4*/
		case A1B4: case A1B5: case A1B6: case A1B7: 
			return state->dma_channel[(offset & 0x70) >> 4].bank;
		case DAS0L: case DAS1L: case DAS2L: case DAS3L: /*0x43n5*/ 
		case DAS4L: case DAS5L: case DAS6L: case DAS7L: 
			return state->dma_channel[(offset & 0x70) >> 4].trans_size & 0xff;
		case DAS0H: case DAS1H: case DAS2H: case DAS3H: /*0x43n6*/ 
		case DAS4H: case DAS5H: case DAS6H: case DAS7H: 
			return (state->dma_channel[(offset & 0x70) >> 4].trans_size >> 8) & 0xff;
		case DSAB0: case DSAB1: case DSAB2: case DSAB3: /*0x43n7*/ 
		case DSAB4: case DSAB5: case DSAB6: case DSAB7: 
			return state->dma_channel[(offset & 0x70) >> 4].ibank;
		case A2A0L: case A2A1L: case A2A2L: case A2A3L: /*0x43n8*/
		case A2A4L: case A2A5L: case A2A6L: case A2A7L: 
			return state->dma_channel[(offset & 0x70) >> 4].hdma_addr & 0xff;
		case A2A0H: case A2A1H: case A2A2H: case A2A3H: /*0x43n9*/
		case A2A4H: case A2A5H: case A2A6H: case A2A7H: 
			return (state->dma_channel[(offset & 0x70) >> 4].hdma_addr >> 8) & 0xff;
		case NTRL0: case NTRL1: case NTRL2: case NTRL3: /*0x43na*/
		case NTRL4: case NTRL5: case NTRL6: case NTRL7: 
			return state->dma_channel[(offset & 0x70) >> 4].hdma_line_counter;
		case 0x430b: case 0x431b: case 0x432b: case 0x433b: /* according to bsnes, this does not return open_bus (even if its precise effect is unknown) */
		case 0x434b: case 0x435b: case 0x436b: case 0x437b: 
			return state->dma_channel[(offset & 0x70) >> 4].unk;

#ifndef MESS
		case 0x4100:		/* NSS Dip-Switches */
			return input_port_read(space->machine, "DSW");
//      case 0x4101: //PC: a104 - a10e - a12a   //only nss_actr
//      case 0x420c: //PC: 9c7d - 8fab          //only nss_ssoc

		default:
			mame_printf_debug("snes_r: offset = %x pc = %x\n",offset,cpu_get_pc(space->cpu));
#endif	/* MESS */

	}

	//printf("unsupported read: offset == %08x\n", offset);

	/* Unsupported reads returns open bus */
//  printf("%02x %02x\n",offset,snes_open_bus_r(space, 0));
	return snes_open_bus_r(space, 0);
}

/*
 * DW   - Double write : address is written twice to set a 16bit value.
 * low  - This is the low byte of a 16 or 24 bit value
 * mid  - This is the middle byte of a 24 bit value
 * high - This is the high byte of a 16 or 24 bit value
 */
WRITE8_HANDLER( snes_w_io )
{
	snes_state *state = (snes_state *)space->machine->driver_data;

	// APU is mirrored from 2140 to 217f
	if (offset >= APU00 && offset < WMDATA)
	{
//      printf("816: %02x to APU @ %d\n", data, offset & 3);
		spc_port_in(state->spc700, offset & 0x3, data);
		cpuexec_boost_interleave(space->machine, attotime_zero, ATTOTIME_IN_USEC(20));
		return;
	}

	if (snes_has_addon_chip == HAS_SUPERFX && state->superfx != NULL)
	{
		if (offset >= 0x3000 && offset < 0x3300)
		{
			superfx_mmio_write(state->superfx, offset, data);
			return;
		}
	}
	else if (snes_has_addon_chip == HAS_RTC)
	{
		if (offset == 0x2800 || offset == 0x2801)
		{
			srtc_write(space->machine, offset, data);
			return;
		}
	}
	else if (snes_has_addon_chip == HAS_SDD1)
	{
		if ((offset >= 0x4300 && offset < 0x4380) ||
		   (offset >= 0x4800 && offset < 0x4808))
		{
			sdd1_mmio_write(space, (UINT32)offset, data);
			return;
		}
		if (offset < 0x80)
		{
			offset += 0x4300;
		}
	}
	else if (snes_has_addon_chip == HAS_SPC7110 || snes_has_addon_chip == HAS_SPC7110_RTC)
	{
		UINT16 limit = (snes_has_addon_chip == HAS_SPC7110_RTC) ? 0x4842 : 0x483f;
		if (offset >= 0x4800 && offset < limit)
		{
			spc7110_mmio_write(space->machine, (UINT32)offset, data);
			return;
		}
	}

	/* offset is from 0x000000 */
	switch (offset)
	{
		case INIDISP:	/* Initial settings for screen */
			if ((snes_ppu.screen_disabled & 0x80) && (!(data & 0x80))) //a 1->0 force blank transition causes a reset OAM address
			{
				memory_write_byte(space, OAMADDL, snes_ppu.oam.saved_address_low);
				memory_write_byte(space, OAMADDH, snes_ppu.oam.saved_address_high);
				snes_ppu.oam.first_sprite = snes_ppu.oam.priority_rotation ? (snes_ppu.oam.address >> 1) & 127 : 0;
			}
			snes_ppu.screen_disabled = data & 0x80;
			snes_ppu.screen_brightness = (data & 0x0f) + 1;
			break;
		case OBSEL:		/* Object size and data area designation */
			snes_ppu.oam.next_charmap = (data & 0x03) << 1;
			snes_ppu.oam.next_name_select = (((data & 0x18) >> 3) * 0x1000) << 1;
			snes_ppu.oam.next_size = (data & 0xe0) >> 5;
			break;
		case OAMADDL:	/* Address for accessing OAM (low) */
			snes_ppu.oam.saved_address_low = data;
			snes_ppu.oam.address = (snes_ppu.oam.address & 0xff00) + data;
			snes_ppu.oam.first_sprite = snes_ppu.oam.priority_rotation ? (snes_ppu.oam.address >> 1) & 127 : 0;
			snes_ram[OAMDATA] = 0;
			break;
		case OAMADDH:	/* Address for accessing OAM (high) */
			snes_ppu.oam.saved_address_high = data;
			snes_ppu.oam.address = (snes_ppu.oam.address & 0x00ff) | ((data & 0x01) << 8);
			snes_ppu.oam.priority_rotation = BIT(data, 7);
			snes_ppu.oam.first_sprite = snes_ppu.oam.priority_rotation ? (snes_ppu.oam.address >> 1) & 127 : 0;
			snes_ram[OAMDATA] = 0;
			break;
		case OAMDATA:	/* Data for OAM write (DW) */
			{
				int oam_addr = snes_ppu.oam.address;

				if (oam_addr >= 0x100)
				{
					oam_addr &= 0x10f;
					if (!(snes_ram[OAMDATA]))
					{
						snes_oam[oam_addr] &= 0xff00;
						snes_oam[oam_addr] |= data;
					}
					else
					{
						snes_oam[oam_addr] &= 0x00ff;
						snes_oam[oam_addr] |= (data << 8);
					}
				}
				else
				{
					oam_addr &= 0x1ff;
					if (!(snes_ram[OAMDATA]))
					{
						snes_ppu.oam.write_latch = data;
					}
					else
					{
						snes_oam[oam_addr] = (data << 8) | snes_ppu.oam.write_latch;
					}
				}
				snes_ram[OAMDATA] = (snes_ram[OAMDATA] + 1) % 2;
				if (snes_ram[OAMDATA] == 0)
				{
					snes_ram[OAMDATA] = 0;
					snes_ppu.oam.address++;
					snes_ppu.oam.address &= 0x1ff;
					snes_ppu.oam.first_sprite = snes_ppu.oam.priority_rotation ? (snes_ppu.oam.address >> 1) & 127 : 0;
				}
				return;
			}
		case BGMODE:	/* BG mode and character size settings */
			snes_ppu.mode = data & 0x07;
			snes_dynamic_res_change(space->machine);
			snes_ppu.bg3_priority_bit = BIT(data, 3);
			snes_ppu.layer[SNES_BG1].tile_size = BIT(data, 4);
			snes_ppu.layer[SNES_BG2].tile_size = BIT(data, 5);
			snes_ppu.layer[SNES_BG3].tile_size = BIT(data, 6);
			snes_ppu.layer[SNES_BG4].tile_size = BIT(data, 7);
			snes_ppu.update_offsets = 1;
			break;
		case MOSAIC:	/* Size and screen designation for mosaic */
			snes_ppu.mosaic_size = (data & 0xf0) >> 4;
			snes_ppu.layer[SNES_BG1].mosaic_enabled = BIT(data, 0);
			snes_ppu.layer[SNES_BG2].mosaic_enabled = BIT(data, 1);
			snes_ppu.layer[SNES_BG3].mosaic_enabled = BIT(data, 2);
			snes_ppu.layer[SNES_BG4].mosaic_enabled = BIT(data, 3);
			break;
		case BG1SC:		/* Address for storing SC data BG1 SC size designation */
		case BG2SC:		/* Address for storing SC data BG2 SC size designation  */
		case BG3SC:		/* Address for storing SC data BG3 SC size designation  */
		case BG4SC:		/* Address for storing SC data BG4 SC size designation  */
			snes_ppu.layer[offset - BG1SC].tilemap = data & 0xfc;
			snes_ppu.layer[offset - BG1SC].tilemap_size = data & 0x3;
			break;
		case BG12NBA:	/* Address for BG 1 and 2 character data */
			snes_ppu.layer[SNES_BG1].charmap = (data & 0x0f);
			snes_ppu.layer[SNES_BG2].charmap = (data & 0xf0) >> 4;
			break;
		case BG34NBA:	/* Address for BG 3 and 4 character data */
			snes_ppu.layer[SNES_BG3].charmap = (data & 0x0f);
			snes_ppu.layer[SNES_BG4].charmap = (data & 0xf0) >> 4;
			break;

		// Anomie says "H Current = (Byte<<8) | (Prev&~7) | ((Current>>8)&7); V Current = (Current<<8) | Prev;" and Prev is shared by all scrolls but in Mode 7!
		case BG1HOFS:	/* BG1 - horizontal scroll (DW) */
			/* In Mode 0->6 we use ppu_last_scroll as Prev */
			snes_ppu.layer[SNES_BG1].hoffs = (data << 8) | (snes_ppu.ppu_last_scroll & ~7) | ((snes_ppu.layer[SNES_BG1].hoffs >> 8) & 7);
			snes_ppu.ppu_last_scroll = data;
			/* In Mode 7 we use mode7_last_scroll as Prev */
			snes_ppu.mode7.hor_offset = (data << 8) | (snes_ppu.mode7_last_scroll & ~7) | ((snes_ppu.mode7.hor_offset >> 8) & 7);
			snes_ppu.mode7_last_scroll = data;
			snes_ppu.update_offsets = 1;
			return;
		case BG1VOFS:	/* BG1 - vertical scroll (DW) */
			/* In Mode 0->6 we use ppu_last_scroll as Prev */
			snes_ppu.layer[SNES_BG1].voffs = (data << 8) | snes_ppu.ppu_last_scroll;
			snes_ppu.ppu_last_scroll = data;
			/* In Mode 7 we use mode7_last_scroll as Prev */
			snes_ppu.mode7.ver_offset = (data << 8) | snes_ppu.mode7_last_scroll;
			snes_ppu.mode7_last_scroll = data;
			snes_ppu.update_offsets = 1;
			return;
		case BG2HOFS:	/* BG2 - horizontal scroll (DW) */
			snes_ppu.layer[SNES_BG2].hoffs = (data << 8) | (snes_ppu.ppu_last_scroll & ~7) | ((snes_ppu.layer[SNES_BG2].hoffs >> 8) & 7);
			snes_ppu.ppu_last_scroll = data;
			snes_ppu.update_offsets = 1;
			return;
		case BG2VOFS:	/* BG2 - vertical scroll (DW) */
			snes_ppu.layer[SNES_BG2].voffs = (data << 8) | (snes_ppu.ppu_last_scroll);
			snes_ppu.ppu_last_scroll = data;
			snes_ppu.update_offsets = 1;
			return;
		case BG3HOFS:	/* BG3 - horizontal scroll (DW) */
			snes_ppu.layer[SNES_BG3].hoffs = (data << 8) | (snes_ppu.ppu_last_scroll & ~7) | ((snes_ppu.layer[SNES_BG3].hoffs >> 8) & 7);
			snes_ppu.ppu_last_scroll = data;
			snes_ppu.update_offsets = 1;
			return;
		case BG3VOFS:	/* BG3 - vertical scroll (DW) */
			snes_ppu.layer[SNES_BG3].voffs = (data << 8) | (snes_ppu.ppu_last_scroll);
			snes_ppu.ppu_last_scroll = data;
			snes_ppu.update_offsets = 1;
			return;
		case BG4HOFS:	/* BG4 - horizontal scroll (DW) */
			snes_ppu.layer[SNES_BG4].hoffs = (data << 8) | (snes_ppu.ppu_last_scroll & ~7) | ((snes_ppu.layer[SNES_BG4].hoffs >> 8) & 7);
			snes_ppu.ppu_last_scroll = data;
			snes_ppu.update_offsets = 1;
			return;
		case BG4VOFS:	/* BG4 - vertical scroll (DW) */
			snes_ppu.layer[SNES_BG4].voffs = (data << 8) | (snes_ppu.ppu_last_scroll);
			snes_ppu.ppu_last_scroll = data;
			snes_ppu.update_offsets = 1;
			return;
		case VMAIN:		/* VRAM address increment value designation */
			state->vram_fgr_high = (data & 0x80);
			state->vram_fgr_increment = vram_fgr_inctab[data & 3];

			if (data & 0xc)
			{
				int md = (data & 0xc) >> 2;

				state->vram_fgr_count = vram_fgr_inccnts[md];			// 0x20, 0x40, 0x80
				state->vram_fgr_mask = (state->vram_fgr_count * 8) - 1;	// 0xff, 0x1ff, 0x2ff
				state->vram_fgr_shift = vram_fgr_shiftab[md];			// 5, 6, 7
			}
			else
			{
				state->vram_fgr_count = 0;
			}
//          printf("VMAIN: high %x inc %x count %x mask %x shift %x\n", state->vram_fgr_high, state->vram_fgr_increment, state->vram_fgr_count, state->vram_fgr_mask, state->vram_fgr_shift);
			break;
		case VMADDL:	/* Address for VRAM read/write (low) */
		case VMADDH:	/* Address for VRAM read/write (high) */
			{
				UINT32 addr;
				snes_ram[offset] = data;
				addr = snes_get_vram_address(space->machine) << 1;
				state->vram_read_buffer = snes_vram_read(space, addr);
				state->vram_read_buffer |= (snes_vram_read(space, addr + 1) << 8);
			}
			break;
		case VMDATAL:	/* 2118: Data for VRAM write (low) */
			{
				UINT32 addr = snes_get_vram_address(space->machine) << 1;
				snes_vram_write(space, addr, data);

				if (!state->vram_fgr_high)
				{
					addr = ((snes_ram[VMADDH] << 8) | snes_ram[VMADDL]) + state->vram_fgr_increment;
					snes_ram[VMADDL] = addr & 0xff;
					snes_ram[VMADDH] = (addr >> 8) & 0xff;
				}
			}
			return;
		case VMDATAH:	/* 2119: Data for VRAM write (high) */
			{
				UINT32 addr = snes_get_vram_address(space->machine) << 1;
				snes_vram_write(space, addr + 1, data);

				if (state->vram_fgr_high)
				{
					addr = ((snes_ram[VMADDH] << 8) | snes_ram[VMADDL]) + state->vram_fgr_increment;
					snes_ram[VMADDL] = addr & 0xff;
					snes_ram[VMADDH] = (addr >> 8) & 0xff;
				}
			}
			return;
		case M7SEL:		/* Mode 7 initial settings */
			snes_ppu.mode7.repeat = (data >> 6) & 3;
			snes_ppu.mode7.vflip  = BIT(data, 1);
			snes_ppu.mode7.hflip  = BIT(data, 0);
			break;
		/* As per Anomie's doc: Reg = (Current<<8) | Prev; and there is only one Prev, shared by these matrix regs and Mode 7 scroll regs */
		case M7A:		/* Mode 7 COS angle/x expansion (DW) */
			snes_ppu.mode7.matrix_a = snes_ppu.mode7_last_scroll + (data << 8);
			snes_ppu.mode7_last_scroll = data;
			break;
		case M7B:		/* Mode 7 SIN angle/ x expansion (DW) */
			snes_ppu.mode7.matrix_b = snes_ppu.mode7_last_scroll + (data << 8);
			snes_ppu.mode7_last_scroll = data;
			break;
		case M7C:		/* Mode 7 SIN angle/y expansion (DW) */
			snes_ppu.mode7.matrix_c = snes_ppu.mode7_last_scroll + (data << 8);
			snes_ppu.mode7_last_scroll = data;
			break;
		case M7D:		/* Mode 7 COS angle/y expansion (DW) */
			snes_ppu.mode7.matrix_d = snes_ppu.mode7_last_scroll + (data << 8);
			snes_ppu.mode7_last_scroll = data;
			break;
		case M7X:		/* Mode 7 x center position (DW) */
			snes_ppu.mode7.origin_x = snes_ppu.mode7_last_scroll + (data << 8);
			snes_ppu.mode7_last_scroll = data;
			break;
		case M7Y:		/* Mode 7 y center position (DW) */
			snes_ppu.mode7.origin_y = snes_ppu.mode7_last_scroll + (data << 8);
			snes_ppu.mode7_last_scroll = data;
			break;
		case CGADD:		/* Initial address for colour RAM writing */
			/* CGRAM is 16-bit, but when reading/writing we treat it as
                 * 8-bit, so we need to double the address */
			state->cgram_address = data << 1;
			break;
		case CGDATA:	/* Data for colour RAM */
			((UINT8 *)snes_cgram)[state->cgram_address] = data;
			state->cgram_address = (state->cgram_address + 1) % (SNES_CGRAM_SIZE - 2);
			break;
		case W12SEL:	/* Window mask settings for BG1-2 */
			if (data != snes_ram[offset])
			{
				snes_ppu.layer[SNES_BG1].window1_invert  = BIT(data, 0);
				snes_ppu.layer[SNES_BG1].window1_enabled = BIT(data, 1);
				snes_ppu.layer[SNES_BG1].window2_invert  = BIT(data, 2);
				snes_ppu.layer[SNES_BG1].window2_enabled = BIT(data, 3);
				snes_ppu.layer[SNES_BG2].window1_invert  = BIT(data, 4);
				snes_ppu.layer[SNES_BG2].window1_enabled = BIT(data, 5);
				snes_ppu.layer[SNES_BG2].window2_invert  = BIT(data, 6);
				snes_ppu.layer[SNES_BG2].window2_enabled = BIT(data, 7);
				snes_ppu.update_windows = 1;
			}
			break;
		case W34SEL:	/* Window mask settings for BG3-4 */
			if (data != snes_ram[offset])
			{
				snes_ppu.layer[SNES_BG3].window1_invert  = BIT(data, 0);
				snes_ppu.layer[SNES_BG3].window1_enabled = BIT(data, 1);
				snes_ppu.layer[SNES_BG3].window2_invert  = BIT(data, 2);
				snes_ppu.layer[SNES_BG3].window2_enabled = BIT(data, 3);
				snes_ppu.layer[SNES_BG4].window1_invert  = BIT(data, 4);
				snes_ppu.layer[SNES_BG4].window1_enabled = BIT(data, 5);
				snes_ppu.layer[SNES_BG4].window2_invert  = BIT(data, 6);
				snes_ppu.layer[SNES_BG4].window2_enabled = BIT(data, 7);
				snes_ppu.update_windows = 1;
			}
			break;
		case WOBJSEL:	/* Window mask settings for objects */
			if (data != snes_ram[offset])
			{
				snes_ppu.layer[SNES_OAM].window1_invert  = BIT(data, 0);
				snes_ppu.layer[SNES_OAM].window1_enabled = BIT(data, 1);
				snes_ppu.layer[SNES_OAM].window2_invert  = BIT(data, 2);
				snes_ppu.layer[SNES_OAM].window2_enabled = BIT(data, 3);
				snes_ppu.layer[SNES_COLOR].window1_invert  = BIT(data, 4);
				snes_ppu.layer[SNES_COLOR].window1_enabled = BIT(data, 5);
				snes_ppu.layer[SNES_COLOR].window2_invert  = BIT(data, 6);
				snes_ppu.layer[SNES_COLOR].window2_enabled = BIT(data, 7);
				snes_ppu.update_windows = 1;
			}
			break;
		case WH0:		/* Window 1 left position */
			if (data != snes_ram[offset])
			{
				snes_ppu.window1_left = data;
				snes_ppu.update_windows = 1;
			}
			break;
		case WH1:		/* Window 1 right position */
			if (data != snes_ram[offset])
			{
				snes_ppu.window1_right = data;
				snes_ppu.update_windows = 1;
			}
			break;
		case WH2:		/* Window 2 left position */
			if (data != snes_ram[offset])
			{
				snes_ppu.window2_left = data;
				snes_ppu.update_windows = 1;
			}
			break;
		case WH3:		/* Window 2 right position */
			if (data != snes_ram[offset])
			{
				snes_ppu.window2_right = data;
				snes_ppu.update_windows = 1;
			}
			break;
		case WBGLOG:	/* Window mask logic for BG's */
			if (data != snes_ram[offset])
			{
				snes_ppu.layer[SNES_BG1].wlog_mask = data & 0x03;
				snes_ppu.layer[SNES_BG2].wlog_mask = (data & 0x0c) >> 2;
				snes_ppu.layer[SNES_BG3].wlog_mask = (data & 0x30) >> 4;
				snes_ppu.layer[SNES_BG4].wlog_mask = (data & 0xc0) >> 6;
				snes_ppu.update_windows = 1;
			}
			break;
		case WOBJLOG:	/* Window mask logic for objects */
			if (data != snes_ram[offset])
			{
				snes_ppu.layer[SNES_OAM].wlog_mask = data & 0x03;
				snes_ppu.layer[SNES_COLOR].wlog_mask = (data & 0x0c) >> 2;
				snes_ppu.update_windows = 1;
			}
			break;
		case TM:		/* Main screen designation */
			snes_ppu.layer[SNES_BG1].main_bg_enabled = BIT(data, 0);
			snes_ppu.layer[SNES_BG2].main_bg_enabled = BIT(data, 1);
			snes_ppu.layer[SNES_BG3].main_bg_enabled = BIT(data, 2);
			snes_ppu.layer[SNES_BG4].main_bg_enabled = BIT(data, 3);
			snes_ppu.layer[SNES_OAM].main_bg_enabled = BIT(data, 4);
			break;
		case TS:		/* Subscreen designation */
			snes_ppu.layer[SNES_BG1].sub_bg_enabled = BIT(data, 0);
			snes_ppu.layer[SNES_BG2].sub_bg_enabled = BIT(data, 1);
			snes_ppu.layer[SNES_BG3].sub_bg_enabled = BIT(data, 2);
			snes_ppu.layer[SNES_BG4].sub_bg_enabled = BIT(data, 3);
			snes_ppu.layer[SNES_OAM].sub_bg_enabled = BIT(data, 4);
			break;
		case TMW:		/* Window mask for main screen designation */
			snes_ppu.layer[SNES_BG1].main_window_enabled = BIT(data, 0);
			snes_ppu.layer[SNES_BG2].main_window_enabled = BIT(data, 1);
			snes_ppu.layer[SNES_BG3].main_window_enabled = BIT(data, 2);
			snes_ppu.layer[SNES_BG4].main_window_enabled = BIT(data, 3);
			snes_ppu.layer[SNES_OAM].main_window_enabled = BIT(data, 4);
			break;
		case TSW:		/* Window mask for subscreen designation */
			snes_ppu.layer[SNES_BG1].sub_window_enabled = BIT(data, 0);
			snes_ppu.layer[SNES_BG2].sub_window_enabled = BIT(data, 1);
			snes_ppu.layer[SNES_BG3].sub_window_enabled = BIT(data, 2);
			snes_ppu.layer[SNES_BG4].sub_window_enabled = BIT(data, 3);
			snes_ppu.layer[SNES_OAM].sub_window_enabled = BIT(data, 4);
			break;
		case CGWSEL:	/* Initial settings for Fixed colour addition or screen addition */
			snes_ppu.clip_to_black = (data >> 6) & 0x03;
			snes_ppu.prevent_color_math = (data >> 4) & 0x03;
			snes_ppu.sub_add_mode = BIT(data, 1);
			snes_ppu.direct_color = BIT(data, 0);
#ifdef SNES_DBG_REG_W
			if ((data & 0x2) != (snes_ram[CGWSEL] & 0x2))
				mame_printf_debug( "Add/Sub Layer: %s\n", ((data & 0x2) >> 1) ? "Subscreen" : "Fixed colour" );
#endif
			break;
		case CGADSUB:	/* Addition/Subtraction designation for each screen */
			snes_ppu.color_modes = data & 0xc0;
			snes_ppu.layer[SNES_BG1].color_math = BIT(data, 0);
			snes_ppu.layer[SNES_BG2].color_math = BIT(data, 1);
			snes_ppu.layer[SNES_BG3].color_math = BIT(data, 2);
			snes_ppu.layer[SNES_BG4].color_math = BIT(data, 3);
			snes_ppu.layer[SNES_OAM].color_math = BIT(data, 4);
			snes_ppu.layer[SNES_COLOR].color_math = BIT(data, 5);
			break;
		case COLDATA:	/* Fixed colour data for fixed colour addition/subtraction */
			{
				/* Store it in the extra space we made in the CGRAM. It doesn't really go there, but it's as good a place as any. */
				UINT8 r, g, b;

				/* Get existing value. */
				r = snes_cgram[FIXED_COLOUR] & 0x1f;
				g = (snes_cgram[FIXED_COLOUR] & 0x3e0) >> 5;
				b = (snes_cgram[FIXED_COLOUR] & 0x7c00) >> 10;
				/* Set new value */
				if (data & 0x20)
					r = data & 0x1f;
				if (data & 0x40)
					g = data & 0x1f;
				if (data & 0x80)
					b = data & 0x1f;
				snes_cgram[FIXED_COLOUR] = (r | (g << 5) | (b << 10));
			} break;
		case SETINI:	/* Screen mode/video select */
			snes_ppu.interlace = (data & 0x01) ? 2 : 1;
			snes_ppu.obj_interlace = (data & 0x02) ? 2 : 1;
			snes_ppu.beam.last_visible_line = (data & 0x04) ? 240 : 225;
			snes_ppu.pseudo_hires = BIT(data, 3);
			snes_ppu.mode7.extbg = BIT(data, 6);
			snes_dynamic_res_change(space->machine);
#ifdef SNES_DBG_REG_W
			if ((data & 0x8) != (snes_ram[SETINI] & 0x8))
				mame_printf_debug( "Pseudo 512 mode: %s\n", (data & 0x8) ? "on" : "off" );
#endif
			break;
		case WMDATA:	/* Data to write to WRAM */
			{
				UINT32 addr = ((snes_ram[WMADDH] & 0x1) << 16) | (snes_ram[WMADDM] << 8) | snes_ram[WMADDL];
				memory_write_byte(space, 0x7e0000 + addr++, data );
				addr &= 0x1ffff;
				snes_ram[WMADDH] = (addr >> 16) & 0x1;
				snes_ram[WMADDM] = (addr >> 8) & 0xff;
				snes_ram[WMADDL] = addr & 0xff;
				return;
			}
		case WMADDL:	/* Address to read/write to wram (low) */
		case WMADDM:	/* Address to read/write to wram (mid) */
		case WMADDH:	/* Address to read/write to wram (high) */
			break;
		case OLDJOY1:	/* Old NES joystick support */
			if (((!(data & 0x1)) && (snes_ram[offset] & 0x1)))
			{
				state->read_idx[0] = 0;
				state->read_idx[1] = 0;
			}
			break;
		case NMITIMEN:	/* Flag for v-blank, timer int. and joy read */
		case OLDJOY2:	/* Old NES joystick support */
			break;
		case WRIO:		/* Programmable I/O port - latches H/V counters on a 0->1 transition */
			if (!(snes_ram[WRIO] & 0x80) && (data & 0x80))
			{
				// external latch
				snes_latch_counters(space->machine);
			}
			break;
		case WRMPYA:	/* Multiplier A */
			break;
		case WRMPYB:	/* Multiplier B */
			snes_ram[WRMPYB] = data;
//          timer_adjust_oneshot(state->mult_timer, cpu_clocks_to_attotime(state->maincpu, 8), 0);
			{
				UINT32 c = snes_ram[WRMPYA] * snes_ram[WRMPYB];
				snes_ram[RDMPYL] = c & 0xff;
				snes_ram[RDMPYH] = (c >> 8) & 0xff;
			}
			break;
		case WRDIVL:	/* Dividend (low) */
		case WRDIVH:	/* Dividend (high) */
			break;
		case WRDVDD:	/* Divisor */
			snes_ram[WRDVDD] = data;
//          timer_adjust_oneshot(state->div_timer, cpu_clocks_to_attotime(state->maincpu, 16), 0);
			{
				UINT16 value, dividend, remainder;
				dividend = remainder = 0;
				value = (snes_ram[WRDIVH] << 8) + snes_ram[WRDIVL];
				if (snes_ram[WRDVDD] > 0)
				{
					dividend = value / data;
					remainder = value % data;
				}
				else
				{
					dividend = 0xffff;
					remainder = value;
				}
				snes_ram[RDDIVL] = dividend & 0xff;
				snes_ram[RDDIVH] = (dividend >> 8) & 0xff;
				snes_ram[RDMPYL] = remainder & 0xff;
				snes_ram[RDMPYH] = (remainder >> 8) & 0xff;
			}
			break;
		case HTIMEL:	/* H-Count timer settings (low)  */
		case HTIMEH:	/* H-Count timer settings (high) */
		case VTIMEL:	/* V-Count timer settings (low)  */
		case VTIMEH:	/* V-Count timer settings (high) */
			break;
		case MDMAEN:	/* DMA channel designation and trigger */
			snes_dma(space, data);
			data = 0;	/* Once DMA is done we need to reset all bits to 0 */
			break;
		case HDMAEN:	/* HDMA channel designation */
			state->hdmaen = data;
			if (state->hdmaen) //if a HDMA is enabled, data is inited at the next scanline
				timer_set(space->machine, video_screen_get_time_until_pos(space->machine->primary_screen, snes_ppu.beam.current_vert + 1, 0), NULL, 0, snes_reset_hdma);
			break;
		case MEMSEL:	/* Access cycle designation in memory (2) area */
			/* FIXME: Need to adjust the speed only during access of banks 0x80+
             * Currently we are just increasing it no matter what */
//          cpu_set_clockscale(state->maincpu, (data & 0x1) ? 1.335820896 : 1.0 );
#ifdef SNES_DBG_REG_W
			if ((data & 0x1) != (snes_ram[MEMSEL] & 0x1))
				mame_printf_debug( "CPU speed: %f Mhz\n", (data & 0x1) ? 3.58 : 2.68 );
#endif
			break;
		case TIMEUP:
			snes_ram[TIMEUP] = data &~0x7f;
			return;
		/* Following are read-only */
		case HVBJOY:	/* H/V blank and joypad enable */
		case MPYL:		/* Multiplication result (low) */
		case MPYM:		/* Multiplication result (mid) */
		case MPYH:		/* Multiplication result (high) */
		case RDIO:
		case RDDIVL:
		case RDDIVH:
		case RDMPYL:
		case RDMPYH:
		case JOY1L:
		case JOY1H:
		case JOY2L:
		case JOY2H:
		case JOY3L:
		case JOY3H:
		case JOY4L:
		case JOY4H:
#ifdef MAME_DEBUG
			logerror( "Write to read-only register: %X value: %X", offset, data );
#endif /* MAME_DEBUG */
			return;
		/* Below is all DMA related */
		case DMAP0:	case DMAP1: case DMAP2: case DMAP3: /*0x43n0*/
		case DMAP4: case DMAP5: case DMAP6: case DMAP7:
			state->dma_channel[(offset & 0x70) >> 4].dmap = data;
			break;
		case BBAD0: case BBAD1: case BBAD2: case BBAD3: /*0x43n1*/
		case BBAD4: case BBAD5: case BBAD6: case BBAD7: 
			state->dma_channel[(offset & 0x70) >> 4].dest_addr = data;
			break;
		case A1T0L: case A1T1L: case A1T2L: case A1T3L: /*0x43n2*/
		case A1T4L: case A1T5L: case A1T6L: case A1T7L: 
			state->dma_channel[(offset & 0x70) >> 4].src_addr = (state->dma_channel[(offset & 0x70) >> 4].src_addr & 0xff00) | (data << 0);
			break;
		case A1T0H: case A1T1H: case A1T2H: case A1T3H: /*0x43n3*/
		case A1T4H: case A1T5H: case A1T6H: case A1T7H: 
			state->dma_channel[(offset & 0x70) >> 4].src_addr = (state->dma_channel[(offset & 0x70) >> 4].src_addr & 0x00ff) | (data << 8);
			break;
		case A1B0: case A1B1: case A1B2: case A1B3:     /*0x43n4*/
		case A1B4: case A1B5: case A1B6: case A1B7: 
			state->dma_channel[(offset & 0x70) >> 4].bank = data;
			break;
		case DAS0L: case DAS1L: case DAS2L: case DAS3L: /*0x43n5*/ 
		case DAS4L: case DAS5L: case DAS6L: case DAS7L: 
			state->dma_channel[(offset & 0x70) >> 4].trans_size = (state->dma_channel[(offset & 0x70) >> 4].trans_size & 0xff00) | (data << 0);
			break;
		case DAS0H: case DAS1H: case DAS2H: case DAS3H: /*0x43n6*/ 
		case DAS4H: case DAS5H: case DAS6H: case DAS7H: 
			state->dma_channel[(offset & 0x70) >> 4].trans_size = (state->dma_channel[(offset & 0x70) >> 4].trans_size & 0x00ff) | (data << 8);
			break;
		case DSAB0: case DSAB1: case DSAB2: case DSAB3: /*0x43n7*/ 
		case DSAB4: case DSAB5: case DSAB6: case DSAB7: 
			state->dma_channel[(offset & 0x70) >> 4].ibank = data;
			break;
		case A2A0L: case A2A1L: case A2A2L: case A2A3L: /*0x43n8*/
		case A2A4L: case A2A5L: case A2A6L: case A2A7L: 
			state->dma_channel[(offset & 0x70) >> 4].hdma_addr = (state->dma_channel[(offset & 0x70) >> 4].hdma_addr & 0xff00) | (data << 0);
			break;
		case A2A0H: case A2A1H: case A2A2H: case A2A3H: /*0x43n9*/
		case A2A4H: case A2A5H: case A2A6H: case A2A7H: 
			state->dma_channel[(offset & 0x70) >> 4].hdma_addr = (state->dma_channel[(offset & 0x70) >> 4].hdma_addr & 0x00ff) | (data << 8);
			break;
		case NTRL0: case NTRL1: case NTRL2: case NTRL3: /*0x43na*/
		case NTRL4: case NTRL5: case NTRL6: case NTRL7: 
			state->dma_channel[(offset & 0x70) >> 4].hdma_line_counter = data;
			break;
		case 0x430b: case 0x431b: case 0x432b: case 0x433b:
		case 0x434b: case 0x435b: case 0x436b: case 0x437b: 
			state->dma_channel[(offset & 0x70) >> 4].unk = data;
			break;
	}

	snes_ram[offset] = data;
}

WRITE_LINE_DEVICE_HANDLER( snes_extern_irq_w )
{
	snes_state *driver_state = (snes_state *)device->machine->driver_data;
	cpu_set_input_line(driver_state->maincpu, G65816_LINE_IRQ, state);
}

/*************************************

    Memory Handlers

*************************************/

/*
There are at least 4 different kind of boards for SNES carts, which we denote with
mode_20, mode_21, mode_22 and mode_25. Below is a layout of the memory for each of
them. Notice we mirror ROM at loading time where necessary (e.g. banks 0x80 to 0xff
at address 0x8000 for mode_20).

MODE_20
     banks 0x00      0x20          0x40      0x60       0x70     0x7e  0x80   0xc0   0xff
address               |             |         |          |        |     |      |      |
0xffff  ------------------------------------------------------------------------------|
             ROM      |     ROM /   |   ROM   |  ROM     |  ROM / |     | 0x00 | 0x40 |
                      |     DSP     |         |          |  SRAM? |     |  to  |  to  |
0x8000  ----------------------------------------------------------|     | 0x3f | 0x7f |
                   Reserv           |         |          |        |  W  |      |      |
                                    |         |          |   S    |  R  |  m   |  m   |
0x6000  ----------------------------|         |  DSP /   |   R    |  A  |  i   |  i   |
                     I/O            |  Reserv |          |   A    |  M  |  r   |  r   |
0x2000  ----------------------------|         |  Reserv  |   M    |     |  r   |  r   |
             Low RAM (from 0x7e)    |         |          |        |     |  o   |  o   |
                                    |         |          |        |     |  r   |  r   |
0x0000  -------------------------------------------------------------------------------

MODE_22 is the same, but banks 0x40 to 0x7e at address 0x000 to 0x7fff contain ROM (of
course mirrored also at 0xc0 to 0xff). Mode 22 is quite similar to the board SHVC-2P3B.
It is used also in SDD-1 games (only for the first blocks of data). DSP data & status
can be either at banks 0x20 to 0x40 at address 0x8000 or at banks 0x60 to 0x6f at address
0x0000.


MODE_21
     banks 0x00      0x10          0x30      0x40   0x7e  0x80      0xc0   0xff
address               |             |         |      |     |         |      |
0xffff  --------------------------------------------------------------------|
                         mirror               | 0xc0 |     | mirror  |      |
                      upper half ROM          |  to  |     | up half |      |
                     from 0xc0 to 0xff        | 0xff |     |   ROM   |      |
0x8000  --------------------------------------|      |     |---------|      |
             DSP /    |    Reserv   |  SRAM   |      |  W  |         |      |
            Reserv    |             |         |  m   |  R  |   0x00  |  R   |
0x6000  --------------------------------------|  i   |  A  |    to   |  O   |
                          I/O                 |  r   |  M  |   0x3f  |  M   |
0x2000  --------------------------------------|  r   |     |  mirror |      |
                   Low RAM (from 0x7e)        |  o   |     |         |      |
                                              |  r   |     |         |      |
0x0000  ---------------------------------------------------------------------


MODE_25 is very similar to MODE_21, but it is tought for images whose roms is larger than
the available banks at 0xc0 to 0xff.

     banks 0x00      0x20      0x3e       0x40    0x7e  0x80      0xb0     0xc0    0xff
address               |         |          |       |     |         |        |       |
0xffff  ----------------------------------------------------------------------------|
                 mirror         |   Last   |       |     |       ROM        |       |
              upper half ROM    | 0.5Mbits |       |     |      mirror      |       |
             from 0x40 to 0x7e  |   ROM    |  ROM  |     |      up half     |  ROM  |
0x8000  -----------------------------------|       |     |------------------|       |
             DSP /    |      Reserv        | next  |  W  | Reserv  |  SRAM  | first |
            Reserv    |                    |       |  R  |         |        |       |
0x6000  -----------------------------------|  31   |  A  |------------------|  32   |
                      I/O                  | Mbits |  M  |        0x00      | Mbits |
0x2000  -----------------------------------|       |     |         to       |       |
                Low RAM (from 0x7e)        |       |     |        0x3f      |       |
                                           |       |     |       mirror     |       |
0x0000  -----------------------------------------------------------------------------


*/

/* 0x000000 - 0x2fffff */
READ8_HANDLER( snes_r_bank1 )
{
	snes_state *state = (snes_state *)space->machine->driver_data;
	UINT8 value;
	UINT16 address = offset & 0xffff;

	if (address < 0x2000)											/* Mirror of Low RAM */
		value = memory_read_byte(space, 0x7e0000 + address);
	else if (address < 0x6000)										/* I/O */
		value = snes_r_io(space, address);
	else if (address < 0x8000)
	{
		if (snes_has_addon_chip == HAS_SUPERFX && state->superfx != NULL)
			value = snes_ram[0x700000 + (address & 0x1fff)];
		else if (snes_has_addon_chip == HAS_OBC1)
			value = obc1_read(space, offset);
		else if ((snes_has_addon_chip == HAS_DSP2) && (offset >= 0x200000))
			value = (address < 0x7000) ? dsp2_read() : 0x00;
		else if ((snes_cart.mode == SNES_MODE_21) && (snes_has_addon_chip == HAS_DSP1) && (offset < 0x100000))
			value = (address < 0x7000) ? dsp1_get_dr() : dsp1_get_sr();
		else if (snes_has_addon_chip == HAS_CX4)
			value = CX4_read(address - 0x6000);
		else if (snes_has_addon_chip == HAS_SPC7110 || snes_has_addon_chip == HAS_SPC7110_RTC)
		{
			value = snes_ram[0x306000 + (offset & 0x1fff)];
		}
		else
		{
			logerror( "snes_r_bank1: Unmapped external chip read: %04x\n", address );
			value = 0xff;											/* Reserved */
		}
	}
	else if ((snes_cart.mode == SNES_MODE_20) && (snes_has_addon_chip == HAS_DSP1) && (offset >= 0x200000))
		value = (address < 0xc000) ? dsp1_get_dr() : dsp1_get_sr();
	else if ((snes_cart.mode == SNES_MODE_20) && (snes_has_addon_chip == HAS_DSP2) && (offset >= 0x200000))
		value = (address < 0xc000) ? dsp2_read() : 0x00;
	else if ((snes_has_addon_chip == HAS_DSP3) && (offset >= 0x200000))
		value = dsp3_read(address);
	else
		value = snes_ram[offset];

	return value;
}

/* 0x300000 - 0x3fffff */
READ8_HANDLER( snes_r_bank2 )
{
	UINT8 value;
	UINT16 address = offset & 0xffff;

	if (address < 0x2000)											/* Mirror of Low RAM */
		value = memory_read_byte(space, 0x7e0000 + address);
	else if (address < 0x6000)										/* I/O */
		value = snes_r_io(space, address);
	else if (address < 0x8000)										/* SRAM for mode_21, Reserved othewise */
	{
		if (snes_has_addon_chip == HAS_OBC1)
			value = obc1_read (space, offset);
		else if (snes_has_addon_chip == HAS_DSP2)
			value = (address < 0x7000) ? dsp2_read() : 0x00;
		else if (snes_has_addon_chip == HAS_SPC7110 || snes_has_addon_chip == HAS_SPC7110_RTC)
		{
			value = snes_ram[0x306000 + (offset & 0x1fff)];
		}
		else if ((snes_cart.mode == SNES_MODE_21) && (snes_cart.sram > 0))
		{
			int mask = ((snes_cart.sram * 1024) - 1);				/* Limit SRAM size to what's actually present */
			offset -= 0x6000;
			value = snes_ram[0x306000 + (offset & mask)];
		}
		else
		{
			logerror( "snes_r_bank2: Unmapped external chip read: %04x\n", address );
			value = 0xff;
		}
	}
	/* some dsp1 games use these banks 0x30 to 0x3f at address 0x8000 */
	else if ((snes_cart.mode == SNES_MODE_20) && (snes_has_addon_chip == HAS_DSP1))
		value = (address < 0xc000) ? dsp1_get_dr() : dsp1_get_sr();
	else if ((snes_cart.mode == SNES_MODE_20) && (snes_has_addon_chip == HAS_DSP2))
		value = (address < 0xc000) ? dsp2_read() : 0x00;
	else if (snes_has_addon_chip == HAS_DSP3)
		value = dsp3_read(address);
	else if (snes_has_addon_chip == HAS_DSP4)
		value = (address < 0xc000) ? dsp4_read() : 0x80;
	else
		value = snes_ram[0x300000 + offset];

	return value;
}

/* 0x400000 - 0x5fffff */
READ8_HANDLER( snes_r_bank3 )
{
	UINT8 value;
	UINT16 address = offset & 0xffff;

	if (snes_has_addon_chip == HAS_SUPERFX)
	{
		//printf( "snes_r_bank3: %08x\n", offset );
		return snes_ram[0x400000 + offset];
	}
	else if (snes_cart.mode & 5)							/* Mode 20 & 22 */
	{
		if ((address < 0x8000) && (snes_cart.mode == SNES_MODE_20)) //FIXME: check this
		{
			value = 0xff;							/* Reserved */
			//value =  snes_ram[0x200000 + ((offset & ~0x8000) | 0x8000)];  // is this hack still needed?   /* Reserved */
		}
		else
			value = snes_ram[0x400000 + offset];
	}
	else											/* Mode 21 & 25 */
		value = snes_ram[0x400000 + offset];

	return value;
}

/* 0x600000 - 0x6fffff */
READ8_HANDLER( snes_r_bank4 )
{
	snes_state *state = (snes_state *)space->machine->driver_data;
	UINT8 value = 0xff;
	UINT16 address = offset & 0xffff;

	if (snes_has_addon_chip == HAS_SUPERFX && state->superfx != NULL)
	{
		value = snes_ram[0x600000 + offset];
	}
	else if (snes_has_addon_chip == HAS_ST010 && offset >= 0x80000 && address < 0x1000)
	{
		value = st010_read(address);
	}
	else if (snes_cart.mode & 5)							/* Mode 20 & 22 */
	{
		if (address >= 0x8000)
			value = snes_ram[0x600000 + offset];
		/* some other dsp1 games use these banks 0x60 to 0x6f at address 0x0000 */
		else if (snes_has_addon_chip == HAS_DSP1)
			value = (address >= 0x4000) ? dsp1_get_sr() : dsp1_get_dr();
		else
		{
			logerror( "snes_r_bank4: Unmapped external chip read: %04x\n", address );
			value = 0xff;							/* Reserved */
		}
	}
	else if (snes_cart.mode & 0x0a)					/* Mode 21 & 25 */
		value = snes_ram[0x600000 + offset];

	return value;
}

/* 0x700000 - 0x7dffff */
READ8_HANDLER( snes_r_bank5 )
{
	snes_state *state = (snes_state *)space->machine->driver_data;
	UINT8 value;
	UINT16 address = offset & 0xffff;

	if (snes_has_addon_chip == HAS_SUPERFX && state->superfx != NULL)
	{
		value = snes_ram[0x700000 + offset];
	}
	else if ((snes_cart.mode & 5) &&(address < 0x8000))		/* Mode 20 & 22 */
	{
		if (snes_cart.sram > 0)
		{
			int mask = ((snes_cart.sram * 1024) - 1) | 0xff0000;	/* Limit SRAM size to what's actually present */
			value = snes_ram[0x700000 + (offset & mask)];
		}
		else
		{
			logerror( "snes_r_bank5: Unmapped external chip read: %04x\n", address );
			value = 0xff;								/* Reserved */
		}
	}
	else
	{
		value = snes_ram[0x700000 + offset];
	}

	return value;
}

/* 0x800000 - 0xbfffff */
READ8_HANDLER( snes_r_bank6 )
{
	snes_state *state = (snes_state *)space->machine->driver_data;
	UINT8 value = 0;
	UINT16 address = offset & 0xffff;

	if (address < 0x8000)
	{
		if (address >= 0x6000 && snes_has_addon_chip == HAS_SUPERFX && state->superfx != NULL)
			logerror( "snes_r_bank6 hit in Super FX mode, please fix me\n" );
		else if (snes_cart.mode != SNES_MODE_25)
			value = memory_read_byte(space, offset);
		else if (snes_has_addon_chip == HAS_CX4)
		{
			//printf( "R: CX4 hit from 0x800000\n" );
		}
		else							/* Mode 25 has SRAM not mirrored from lower banks */
		{
			if (address < 0x6000)
				value = memory_read_byte(space, offset);
			else if ((offset >= 0x300000) && (snes_cart.sram > 0))
			{
				int mask = ((snes_cart.sram * 1024) - 1);		/* Limit SRAM size to what's actually present */
				offset -= 0x6000;
				value = snes_ram[0x806000 + (offset & mask)];	/* SRAM */
			}
			else						/* Area 0x6000-0x8000 with offset < 0x300000 is reserved */
			{
				logerror( "snes_r_bank6: Unmapped external chip read: %04x\n", address );
				value = 0xff;
			}
		}
	}
	else if ((snes_cart.mode == SNES_MODE_20) && (snes_has_addon_chip == HAS_DSP1) && (offset >= 0x200000))
		value = (address < 0xc000) ? dsp1_get_dr() : dsp1_get_sr();
	else if ((snes_cart.mode == SNES_MODE_20) && (snes_has_addon_chip == HAS_DSP2) && (offset >= 0x200000))
		value = (address < 0xc000) ? dsp2_read() : 0x00;
	else if ((snes_has_addon_chip == HAS_DSP3) && (offset >= 0x200000))
		value = dsp3_read(address);
	else if ((snes_has_addon_chip == HAS_DSP4) && (offset >= 0x300000))
		value = (address < 0xc000) ? dsp4_read() : 0x80;
	else
		value = snes_ram[0x800000 + offset];

	return value;
}

/* 0xc00000 - 0xffffff */
READ8_HANDLER( snes_r_bank7 )
{
	snes_state *state = (snes_state *)space->machine->driver_data;
	UINT8 value = 0;
	UINT16 address = offset & 0xffff;

	if (snes_has_addon_chip == HAS_SDD1)
	{
		return sdd1_read(space->machine, offset);
	}
	else if (snes_has_addon_chip == HAS_SUPERFX && state->superfx != NULL)
	{
		logerror( "snes_r_bank7 hit in Super FX mode, please fix me\n" );
	}
	else if (snes_has_addon_chip == HAS_ST010 && offset >= 0x280000 && offset < 0x300000 && address < 0x1000)
	{
		value = st010_read(address);
	}
	else if (snes_cart.mode & 5)				/* Mode 20 & 22 */
	{
		if (address < 0x8000)
		{
			value = memory_read_byte(space, 0x400000 + offset);
		}
		else
			value = snes_ram[0xc00000 + offset];
	}
	else								/* Mode 21 & 25 */
		value = snes_ram[0xc00000 + offset];

	return value;
}


/* 0x000000 - 0x2fffff */
WRITE8_HANDLER( snes_w_bank1 )
{
	UINT16 address = offset & 0xffff;

	if (address < 0x2000)							/* Mirror of Low RAM */
		memory_write_byte(space, 0x7e0000 + address, data);
	else if (address < 0x6000)						/* I/O */
		snes_w_io(space, address, data);
	else if (address < 0x8000)
	{
		if (snes_has_addon_chip == HAS_SUPERFX)
			snes_ram[0x700000 + (address & 0x1fff)] = data;
		else if (snes_has_addon_chip == HAS_OBC1)
			obc1_write(space, offset, data);
		else if ((snes_has_addon_chip == HAS_DSP2) && (offset >= 0x200000))
			dsp2_write(data);
		else if ((snes_cart.mode == SNES_MODE_21) && (snes_has_addon_chip == HAS_DSP1) && (offset < 0x100000))
			dsp1_set_dr(data);
		else if (snes_has_addon_chip == HAS_CX4)
			CX4_write(space->machine, address - 0x6000, data);
		else if (snes_has_addon_chip == HAS_SPC7110 || snes_has_addon_chip == HAS_SPC7110_RTC)
		{
			snes_ram[0x306000 + (offset & 0x1fff)] = data;
		}
		else
			logerror( "snes_w_bank1: Attempt to write to reserved address: %x = %02x\n", offset, data );
	}
	else if ((snes_cart.mode == SNES_MODE_20) && (snes_has_addon_chip == HAS_DSP1) && (offset >= 0x200000))
		dsp1_set_dr(data);
	else if ((snes_cart.mode == SNES_MODE_20) && (snes_has_addon_chip == HAS_DSP2) && (offset >= 0x200000) && (address < 0xc000))
		dsp2_write(data);
	else if ((snes_has_addon_chip == HAS_DSP3) && (offset >= 0x200000))
		dsp3_write(address, data);
	else
		logerror( "Attempt to write to ROM address: %X\n", offset );
}

/* 0x300000 - 0x3fffff */
WRITE8_HANDLER( snes_w_bank2 )
{
	UINT16 address = offset & 0xffff;

	if (address < 0x2000)							/* Mirror of Low RAM */
		memory_write_byte(space, 0x7e0000 + address, data);
	else if (address < 0x6000)						/* I/O */
		snes_w_io(space, address, data);
	else if (address < 0x8000)						/* SRAM for mode_21, Reserved othewise */
	{
		if (snes_has_addon_chip == HAS_OBC1)
			obc1_write(space, offset, data);
		else if (snes_has_addon_chip == HAS_DSP2)
			dsp2_write(data);
		else if (snes_has_addon_chip == HAS_SPC7110 || snes_has_addon_chip == HAS_SPC7110_RTC)
		{
			snes_ram[0x306000 + (offset & 0x1fff)] = data;
		}
		else if ((snes_cart.mode == SNES_MODE_21) && (snes_cart.sram > 0))
		{
			int mask = ((snes_cart.sram * 1024) - 1);			/* Limit SRAM size to what's actually present */
			logerror( "snes_w_bank2 hit in Super FX mode, please fix me if necessary\n" );
			offset -= 0x6000;
			snes_ram[0x306000 + (offset & mask)] = data;
		}
		else
			logerror("snes_w_bank2: Attempt to write to reserved address: %X = %02x\n", offset + 0x300000, data);
	}
	/* some dsp1 games use these banks 0x30 to 0x3f at address 0x8000 */
	else if ((snes_cart.mode == SNES_MODE_20) && (snes_has_addon_chip == HAS_DSP1))
		dsp1_set_dr(data);
	else if ((snes_cart.mode == SNES_MODE_20) && (snes_has_addon_chip == HAS_DSP2) && (address < 0xc000))
		dsp2_write(data);
	else if (snes_has_addon_chip == HAS_DSP3)
		dsp3_write(address, data);
	else if ((snes_has_addon_chip == HAS_DSP4) && (address < 0xc000))
		dsp4_write(data);
	else
		logerror("Attempt to write to ROM address: %X\n", offset + 0x300000);
}

/* 0x600000 - 0x6fffff */
WRITE8_HANDLER( snes_w_bank4 )
{
	snes_state *state = (snes_state *)space->machine->driver_data;
	UINT16 address = offset & 0xffff;

	if (snes_has_addon_chip == HAS_SUPERFX && state->superfx != NULL)
	{
		snes_ram[0x600000 + offset] = data;
	}
	else if (snes_has_addon_chip == HAS_ST010 && offset >= 0x80000 && address < 0x1000)
	{
		st010_write(address, data);
	}
	else if (snes_cart.mode & 5)					/* Mode 20 & 22 */
	{
		if (address >= 0x8000)
		{
			logerror("Attempt to write to ROM address: %X\n", offset + 0x600000);
		}
		else if (snes_has_addon_chip == HAS_DSP1)
		{
			dsp1_set_dr(data);
		}
		else
		{
			logerror("snes_w_bank4: Attempt to write to reserved address: %X = %02x\n", offset + 0x600000, data);
		}
	}
	else if (snes_cart.mode & 0x0a)
	{
		logerror("Attempt to write to ROM address: %X\n", offset + 0x600000);
	}
}

/* 0x700000 - 0x7dffff */
WRITE8_HANDLER( snes_w_bank5 )
{
	snes_state *state = (snes_state *)space->machine->driver_data;
	UINT16 address = offset & 0xffff;

	if (snes_has_addon_chip == HAS_SUPERFX && state->superfx != NULL)
	{
		snes_ram[0x700000 + offset] = data;
	}
	else if ((snes_cart.mode & 5) && (address < 0x8000))			/* Mode 20 & 22 */
	{
		if (snes_cart.sram > 0)
		{
			int mask = ((snes_cart.sram * 1024) - 1) | 0xff0000;	/* Limit SRAM size to what's actually present */
			snes_ram[0x700000 + (offset & mask)] = data;
		}
		else
		{
			logerror("snes_w_bank5: Attempt to write to reserved address: %X = %02x\n", offset + 0x700000, data);
		}
	}
	else if (snes_cart.mode & 0x0a)
	{
		logerror("Attempt to write to ROM address: %X\n", offset + 0x700000);
	}
}


/* 0x800000 - 0xbfffff */
WRITE8_HANDLER( snes_w_bank6 )
{
	snes_state *state = (snes_state *)space->machine->driver_data;
	UINT16 address = offset & 0xffff;

	if (address < 0x8000)
	{
		if (address >= 0x6000 && snes_has_addon_chip == HAS_SUPERFX && state->superfx != NULL)
		{
			logerror( "snes_w_bank6 hit (RAM) in Super FX mode, please fix me\n" );
		}
		else if (snes_has_addon_chip == HAS_CX4)
		{
			//printf( "R: CX4 hit from 0x800000\n" );
		}
		if (snes_cart.mode != SNES_MODE_25)
		{
			if (offset < 0x300000)
				snes_w_bank1(space, offset, data);
			else
				snes_w_bank2(space, offset - 0x300000, data);
		}
		else	/* Mode 25 has SRAM not mirrored from lower banks */
		{
			if (address < 0x6000)
			{
				if (offset < 0x300000)
					snes_w_bank1(space, offset, data);
				else
					snes_w_bank2(space, offset - 0x300000, data);
			}
			else if ((offset >= 0x300000) && (snes_cart.sram > 0))
			{
				int mask = ((snes_cart.sram * 1024) - 1);			/* Limit SRAM size to what's actually present */
				offset -= 0x6000;
				snes_ram[0xb06000 + (offset & mask)] = data;
			}
			else	/* Area in 0x6000-0x8000 && offset < 0x300000 is Reserved! */
				logerror("snes_w_bank6: Attempt to write to reserved address: %X = %02x\n", offset + 0x800000, data);
		}
	}
	else if ((snes_cart.mode == SNES_MODE_20) && (snes_has_addon_chip == HAS_DSP1) && (offset >= 0x200000))
		dsp1_set_dr(data);
	else if ((snes_cart.mode == SNES_MODE_20) && (snes_has_addon_chip == HAS_DSP2) && (offset >= 0x200000) && (address < 0xc000))
		dsp2_write(data);
	else if ((snes_has_addon_chip == HAS_DSP3) && (offset >= 0x200000))
		dsp3_write(address, data);
	else if ((snes_has_addon_chip == HAS_DSP4) && (offset >= 0x300000) && (address < 0xc000))
		dsp4_write(data);
	else if (snes_has_addon_chip == HAS_SUPERFX && state->superfx != NULL)
		logerror( "snes_w_bank6 hit (ROM) in Super FX mode, please fix me\n" );
	else
		logerror("Attempt to write to ROM address: %X\n", offset + 0x800000);
}


/* 0xc00000 - 0xffffff */
WRITE8_HANDLER( snes_w_bank7 )
{
	snes_state *state = (snes_state *)space->machine->driver_data;
	UINT16 address = offset & 0xffff;

	if (snes_has_addon_chip == HAS_SUPERFX && state->superfx != NULL)
	{
		if (offset >= 0x200000)
		{
			logerror( "snes_w_bank7 hit (ROM) in Super FX mode, please fix me\n" );
		}
		else
		{
			logerror( "snes_w_bank7 hit (RAM) in Super FX mode, please fix me\n" );
		}
	}
	else if (snes_has_addon_chip == HAS_ST010 && offset >= 0x280000 && offset < 0x300000 && address < 0x1000)
	{
		st010_write(address, data);
	}
	else if (snes_cart.mode & 5)				/* Mode 20 & 22 */
	{
		if (address < 0x8000)
		{
			if (offset >= 0x3e0000)
				logerror("Attempt to write to banks 0xfe - 0xff address: %X\n", offset);
			else if (offset >= 0x300000)
				snes_w_bank5(space, offset - 0x300000, data);
			else if (offset >= 0x200000)
				snes_w_bank4(space, offset - 0x200000, data);
		}
		else
			logerror("snes_w_bank7: Attempt to write to ROM address: %X = %02x\n", offset + 0xc00000, data);
	}
	else if (snes_cart.mode & 0x0a)
		logerror("Attempt to write to ROM address: %X\n", offset + 0xc00000);
}


/*************************************

    Input Callbacks

*************************************/

static void nss_io_read( running_machine *machine )
{
	snes_state *state = (snes_state *)machine->driver_data;
	static const char *const portnames[2][4] =
			{
				{ "SERIAL1_DATA1_L", "SERIAL1_DATA1_H", "SERIAL1_DATA2_L", "SERIAL1_DATA2_H" },
				{ "SERIAL2_DATA1_L", "SERIAL2_DATA1_H", "SERIAL2_DATA2_L", "SERIAL2_DATA2_H" },
			};
	int port;

	for (port = 0; port < 2; port++)
	{
		state->data1[port] = input_port_read(machine, portnames[port][0]) | (input_port_read(machine, portnames[port][1]) << 8);
		state->data2[port] = input_port_read(machine, portnames[port][2]) | (input_port_read(machine, portnames[port][3]) << 8);

		// avoid sending signals that could crash games
		// if left, no right
		if (state->data1[port] & 0x200)
			state->data1[port] &= ~0x100;
		// if up, no down
		if (state->data1[port] & 0x800)
			state->data1[port] &= ~0x400;

		state->joypad[port].buttons = state->data1[port];
	}

	// is automatic reading on? if so, copy port data1/data2 to joy1l->joy4h
	// this actually works like reading the first 16bits from oldjoy1/2 in reverse order
	if (snes_ram[NMITIMEN] & 1)
	{
		state->joy1l = (state->data1[0] & 0x00ff) >> 0;
		state->joy1h = (state->data1[0] & 0xff00) >> 8;
		state->joy2l = (state->data1[1] & 0x00ff) >> 0;
		state->joy2h = (state->data1[1] & 0xff00) >> 8;
		state->joy3l = (state->data2[0] & 0x00ff) >> 0;
		state->joy3h = (state->data2[0] & 0xff00) >> 8;
		state->joy4l = (state->data2[1] & 0x00ff) >> 0;
		state->joy4h = (state->data2[1] & 0xff00) >> 8;

		// make sure read_idx starts returning all 1s because the auto-read reads it :-)
		state->read_idx[0] = 16;
		state->read_idx[1] = 16;
	}

}

static UINT8 nss_oldjoy1_read( running_machine *machine )
{
	snes_state *state = (snes_state *)machine->driver_data;
	UINT8 res;

	if (state->read_idx[0] >= 16)
		res = 0x01;
	else
		res = (state->joypad[0].buttons >> (15 - state->read_idx[0]++)) & 0x01;

	return res;
}

static UINT8 nss_oldjoy2_read( running_machine *machine )
{
	snes_state *state = (snes_state *)machine->driver_data;
	UINT8 res;

	if (state->read_idx[1] >= 16)
		res = 0x01;
	else
		res = (state->joypad[1].buttons >> (15 - state->read_idx[1]++)) & 0x01;

	return res;
}

/*************************************

    Driver Init

*************************************/

static void snes_init_timers( running_machine *machine )
{
	snes_state *state = (snes_state *)machine->driver_data;

	/* init timers and stop them */
	state->scanline_timer = timer_alloc(machine, snes_scanline_tick, NULL);
	timer_adjust_oneshot(state->scanline_timer, attotime_never, 0);
	state->hblank_timer = timer_alloc(machine, snes_hblank_tick, NULL);
	timer_adjust_oneshot(state->hblank_timer, attotime_never, 0);
	state->nmi_timer = timer_alloc(machine, snes_nmi_tick, NULL);
	timer_adjust_oneshot(state->nmi_timer, attotime_never, 0);
	state->hirq_timer = timer_alloc(machine, snes_hirq_tick_callback, NULL);
	timer_adjust_oneshot(state->hirq_timer, attotime_never, 0);
	state->div_timer = timer_alloc(machine, snes_div_callback, NULL);
	timer_adjust_oneshot(state->div_timer, attotime_never, 0);
	state->mult_timer = timer_alloc(machine, snes_mult_callback, NULL);
	timer_adjust_oneshot(state->mult_timer, attotime_never, 0);

	// SNES hcounter has a 0-339 range.  hblank starts at counter 260.
	// clayfighter sets an HIRQ at 260, apparently it wants it to be before hdma kicks off, so we'll delay 2 pixels.
	state->hblank_offset = 268;
	timer_adjust_oneshot(state->hblank_timer, video_screen_get_time_until_pos(machine->primary_screen, ((snes_ram[STAT78] & 0x10) == SNES_NTSC) ? SNES_VTOTAL_NTSC - 1 : SNES_VTOTAL_PAL - 1, state->hblank_offset), 0);
}

static void snes_init_ram( running_machine *machine )
{
	snes_state *state = (snes_state *)machine->driver_data;
	const address_space *cpu0space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);
	int i, j;

	/* Init VRAM */
	memset(snes_vram, 0, SNES_VRAM_SIZE);

	/* Init Colour RAM */
	memset((UINT8 *)snes_cgram, 0, SNES_CGRAM_SIZE);

	/* Init oam RAM */
	memset(snes_oam, 0xff, SNES_OAM_SIZE);

	/* Init work RAM - 0x55 isn't exactly right but it's close */
	/* make sure it happens to the 65816 (CPU 0) */
	for (i = 0; i < (128*1024); i++)
	{
		memory_write_byte(cpu0space, 0x7e0000 + i, 0x55);
	}

	/* Inititialize registers/variables */
	snes_ppu.update_windows = 1;
	snes_ppu.beam.latch_vert = 0;
	snes_ppu.beam.latch_horz = 0;
	snes_ppu.beam.current_vert = 0;
	snes_ppu.beam.current_horz = 0;
	snes_ppu.beam.last_visible_line = 240;
	snes_ppu.mode = 0;
	snes_ppu.ppu1_version = 1;	// 5C77 chip version number, read by STAT77, only '1' is known
	snes_ppu.ppu2_version = 3;	// 5C78 chip version number, read by STAT78, only '2' & '3' encountered so far.

	state->cgram_address = 0;
	state->vram_read_offset = 2;
	state->read_ophct = 0;
	state->read_opvct = 0;

	state->joy1l = state->joy1h = state->joy2l = state->joy2h = state->joy3l = state->joy3h = 0;
	state->data1[0] = state->data2[0] = state->data1[1] = state->data2[1] = 0;

	state->io_read = nss_io_read;
	state->oldjoy1_read = nss_oldjoy1_read;
	state->oldjoy2_read = nss_oldjoy2_read;

	/* Inititialize mosaic table */
	for (j = 0; j < 16; j++)
	{
		for (i = 0; i < 4096; i++)
		{
			snes_ppu.mosaic_table[j][i] = (i / (j + 1)) * (j + 1);
		}
	}

	// set up some known register power-up defaults
	snes_ram[WRIO] = 0xff;
	snes_ram[VMAIN] = 0x80;

	switch (snes_has_addon_chip)
	{
		case HAS_DSP1:
			dsp1_init(machine);
			break;

		case HAS_DSP2:
			dsp2_init(machine);
			break;

		case HAS_DSP3:
			dsp3_init(machine);
			break;

		case HAS_DSP4:
			dsp4_init(machine);
			break;

		case HAS_RTC:
			srtc_init(machine);
			break;

		case HAS_SDD1:
			sdd1_reset(machine);
			break;

		case HAS_OBC1:
			obc1_init(machine);
			break;

		case HAS_ST010:
			st010_reset();
			break;

		default:
			break;
	}

	// init frame counter so first line is 0
	if (ATTOSECONDS_TO_HZ(video_screen_get_frame_period(machine->primary_screen).attoseconds) >= 59)
	{
		snes_ppu.beam.current_vert = SNES_VTOTAL_NTSC;
	}
	else
	{
		snes_ppu.beam.current_vert = SNES_VTOTAL_PAL;
	}
}


static DIRECT_UPDATE_HANDLER( spc_direct )
{
	direct->raw = direct->decrypted = spc_get_ram(devtag_get_device(space->machine, "spc700"));
	return ~0;
}

static DIRECT_UPDATE_HANDLER( snes_direct )
{
	direct->raw = direct->decrypted = snes_ram;
	return ~0;
}

MACHINE_START( snes )
{
	snes_state *state = (snes_state *)machine->driver_data;
	snes_vram = auto_alloc_array(machine, UINT8, SNES_VRAM_SIZE);
	snes_cgram = auto_alloc_array(machine, UINT16, SNES_CGRAM_SIZE/2);
	snes_oam = auto_alloc_array(machine, UINT16, SNES_OAM_SIZE/2);
	memory_set_direct_update_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), snes_direct);
	memory_set_direct_update_handler(cputag_get_address_space(machine, "soundcpu", ADDRESS_SPACE_PROGRAM), spc_direct);

	state->maincpu = devtag_get_device(machine, "maincpu");
	state->soundcpu = devtag_get_device(machine, "soundcpu");
	state->spc700 = devtag_get_device(machine, "spc700");
	state->superfx = devtag_get_device(machine, "superfx");

	// power-on sets these registers like this
	snes_ram[WRIO] = 0xff;
	snes_ram[WRMPYA] = 0xff;
	snes_ram[WRDIVL] = 0xff;
	snes_ram[WRDIVH] = 0xff;

	switch (snes_has_addon_chip)
	{
		case HAS_SDD1:
			sdd1_init(machine);
			break;
		case HAS_SPC7110:
		case HAS_SPC7110_RTC:
			spc7110_init(machine);
			break;
		case HAS_ST010:
			st010_init(machine);
			break;
	}

	snes_init_timers(machine);
}

MACHINE_RESET( snes )
{
	snes_state *state = (snes_state *)machine->driver_data;

	snes_init_ram(machine);

	/* Set STAT78 to NTSC or PAL */
	if (ATTOSECONDS_TO_HZ(video_screen_get_frame_period(machine->primary_screen).attoseconds) >= 59.0f)
		snes_ram[STAT78] = SNES_NTSC;
	else /* if (ATTOSECONDS_TO_HZ(video_screen_get_frame_period(machine->primary_screen).attoseconds) == 50.0f) */
		snes_ram[STAT78] = SNES_PAL;

	// reset does this to these registers
	snes_ram[NMITIMEN] = 0;
	snes_ram[HTIMEL] = 0xff;
	snes_ram[HTIMEH] = 0x1;
	snes_ram[VTIMEL] = 0xff;
	snes_ram[VTIMEH] = 0x1;

	state->htmult = 1;
	snes_ppu.interlace = 1;
	snes_ppu.obj_interlace = 1;
}


/* for mame we use an init, maybe we will need more for the different games */
DRIVER_INIT( snes )
{
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);
	UINT16 total_blocks, read_blocks;
	UINT8  *rom;

	rom = memory_region(machine, "user3");
	snes_ram = auto_alloc_array(machine, UINT8, 0x1400000);
	memset(snes_ram, 0, 0x1400000);

	/* all NSS games seem to use MODE 20 */
	snes_cart.mode = SNES_MODE_20;
	snes_cart.sram_max = 0x40000;
	snes_has_addon_chip = HAS_NONE;

	/* Find the number of blocks in this ROM */
	total_blocks = (memory_region_length(machine, "user3") / 0x8000);
	read_blocks = 0;

	/* Loading all the data blocks from cart, we only partially cover banks 0x00 to 0x7f. Therefore, we
     * have to mirror the blocks until we reach the end. E.g. for a 11Mbits image (44 blocks), we proceed
     * as follows:
     * 11 Mbits = 8 Mbits (blocks 1->32) + 2 Mbits (blocks 33->40) + 1 Mbit (blocks 41->44).
     * Hence, we fill memory up to 16 Mbits (banks 0x00 to 0x3f) mirroring the final part:
     * 8 Mbits (blocks 1->32) + 2 Mbits (blocks 33->40) + 1 Mbit (blocks 41->44) + 1 Mbit (blocks 41->44)
     *   + 2 Mbits (blocks 33->40) + 1 Mbit (blocks 41->44) + 1 Mbit (blocks 41->44).
     * And we repeat the same blocks in the second half of the banks (banks 0x40 to 0x7f).
     * This is likely what happens in the real SNES as well, because the unit cannot be aware of the exact
     * size of data in the cart (procedure confirmed by byuu)
     */

	/* LoROM carts load data in banks 0x00 to 0x7f at address 0x8000 (actually up to 0x7d, because 0x7e and
     * 0x7f are overwritten by WRAM). Each block is also mirrored in banks 0x80 to 0xff (up to 0xff for real)
     */
	while (read_blocks < 128 && read_blocks < total_blocks)
	{
		/* Loading data */
		memcpy(&snes_ram[0x008000 + read_blocks * 0x10000], &rom[read_blocks * 0x08000], 0x8000);
		/* Mirroring */
		memcpy(&snes_ram[0x808000 + read_blocks * 0x10000], &snes_ram[0x8000 + read_blocks * 0x10000], 0x8000);
		read_blocks++;
	}
	/* Filling banks up to 0x7f and their mirrors */
	while (read_blocks % 128)
	{
		int j = 0, repeat_blocks;
		while ((read_blocks % (128 >> j)) && j < 7)
			j++;
		repeat_blocks = read_blocks % (128 >> (j - 1));

		memcpy(&snes_ram[read_blocks * 0x10000], &snes_ram[(read_blocks - repeat_blocks) * 0x10000], repeat_blocks * 0x10000);
		memcpy(&snes_ram[0x800000 + read_blocks * 0x10000], &snes_ram[0x800000 + (read_blocks - repeat_blocks) * 0x10000], repeat_blocks * 0x10000);

		read_blocks += repeat_blocks;
	}

	/* Find the amount of sram */
	snes_cart.sram = snes_r_bank1(space, 0x00ffd8);
	if (snes_cart.sram > 0)
	{
		snes_cart.sram = ((1 << (snes_cart.sram + 3)) / 8);
		if (snes_cart.sram > snes_cart.sram_max)
			snes_cart.sram = snes_cart.sram_max;
	}
}

DRIVER_INIT( snes_hirom )
{
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);
	UINT16 total_blocks, read_blocks;
	UINT8  *rom;

	rom = memory_region(machine, "user3");
	snes_ram = auto_alloc_array(machine, UINT8, 0x1400000);
	memset(snes_ram, 0, 0x1400000);

	snes_cart.mode = SNES_MODE_21;
	snes_cart.sram_max = 0x40000;
	snes_has_addon_chip = HAS_NONE;

	/* Find the number of blocks in this ROM */
	total_blocks = (memory_region_length(machine, "user3") / 0x10000);
	read_blocks = 0;

	/* See above for details about the way we fill banks 0x00 to 0x7f */

	/* HiROM carts load data in banks 0xc0 to 0xff. Each bank is fully mirrored in banks 0x40 to 0x7f
     * (actually up to 0x7d, because 0x7e and 0x7f are overwritten by WRAM). The top half (address
     * range 0x8000 - 0xffff) of each bank is also mirrored in banks 0x00 to 0x3f and 0x80 to 0xbf.
     */
	while (read_blocks < 64 && read_blocks < total_blocks)
	{
		/* Loading data */
		memcpy(&snes_ram[0xc00000 + read_blocks * 0x10000], &rom[read_blocks * 0x10000], 0x10000);
		/* Mirroring */
		memcpy(&snes_ram[0x008000 + read_blocks * 0x10000], &snes_ram[0xc08000 + read_blocks * 0x10000], 0x8000);
		memcpy(&snes_ram[0x400000 + read_blocks * 0x10000], &snes_ram[0xc00000 + read_blocks * 0x10000], 0x10000);
		memcpy(&snes_ram[0x808000 + read_blocks * 0x10000], &snes_ram[0xc08000 + read_blocks * 0x10000], 0x8000);
		read_blocks++;
	}
	/* Filling banks up to 0x7f and their mirrors */
	while (read_blocks % 64)
	{
		int j = 0, repeat_blocks;
		while ((read_blocks % (64 >> j)) && j < 6)
			j++;
		repeat_blocks = read_blocks % (64 >> (j - 1));

		memcpy(&snes_ram[0xc00000 + read_blocks * 0x10000], &snes_ram[0xc00000 + (read_blocks - repeat_blocks) * 0x10000], repeat_blocks * 0x10000);
		memcpy(&snes_ram[read_blocks * 0x10000], &snes_ram[(read_blocks - repeat_blocks) * 0x10000], repeat_blocks * 0x10000);
		memcpy(&snes_ram[0x400000 + read_blocks * 0x10000], &snes_ram[0x400000 + (read_blocks - repeat_blocks) * 0x10000], repeat_blocks * 0x10000);
		memcpy(&snes_ram[0x800000 + read_blocks * 0x10000], &snes_ram[0x800000 + (read_blocks - repeat_blocks) * 0x10000], repeat_blocks * 0x10000);
		read_blocks += repeat_blocks;
	}

	/* Find the amount of sram */
	snes_cart.sram = snes_r_bank1(space, 0x00ffd8);
	if (snes_cart.sram > 0)
	{
		snes_cart.sram = ((1 << (snes_cart.sram + 3)) / 8);
		if (snes_cart.sram > snes_cart.sram_max)
			snes_cart.sram = snes_cart.sram_max;
	}
}


/*************************************

    HDMA

*************************************/

static int dma_abus_valid( UINT32 address )
{
	if((address & 0x40ff00) == 0x2100) return 0;  //$[00-3f|80-bf]:[2100-21ff]
	if((address & 0x40fe00) == 0x4000) return 0;  //$[00-3f|80-bf]:[4000-41ff]
	if((address & 0x40ffe0) == 0x4200) return 0;  //$[00-3f|80-bf]:[4200-421f]
	if((address & 0x40ff80) == 0x4300) return 0;  //$[00-3f|80-bf]:[4300-437f]

	return 1;
}

INLINE void snes_dma_transfer( const address_space *space, UINT8 dma, UINT32 abus, UINT16 bbus )
{
	snes_state *state = (snes_state *)space->machine->driver_data;
	UINT32 src, dst;

	if (state->dma_channel[dma].dmap & 0x80)	/* PPU->CPU */
	{
		if (bbus == 0x2180 && ((abus & 0xfe0000) == 0x7e0000 || (abus & 0x40e000) == 0x0000))
		{
			//illegal WRAM->WRAM transfer (bus conflict)
			//no read occurs; write does occur
			memory_write_byte(space, abus, 0x00);
			return;
		}
		else 
		{
			if (!dma_abus_valid(abus))
				return;

			src = bbus; dst = abus;
		}
	}
	else									/* CPU->PPU */
	{
		if (bbus == 0x2180 && ((abus & 0xfe0000) == 0x7e0000 || (abus & 0x40e000) == 0x0000))
		{
			//illegal WRAM->WRAM transfer (bus conflict)
			//read most likely occurs; no write occurs
			//read is irrelevent, as it cannot be observed by software
			return;
		}
		else 
		{
			src = abus; dst = bbus;
		}
	}

	memory_write_byte(space, dst, memory_read_byte(space, src));
}

#if 0
// FIXME: we need to use snes_state->hdma_iaddr[i] where appropriate (we now still 
// use snes_state->trans_size[i] to avoid regressions in this first wip)
// and try to make the addr recovering/updating a bit more abstract 
INLINE UINT32 snes_get_hdma_addr( running_machine *machine, int dma )
{
	snes_state *state = (snes_state *)machine->driver_data;
	return (state->dma_channel[dma].bank << 16) | (state->dma_channel[dma].hdma_addr++);
}

INLINE UINT32 snes_get_hdma_iaddr( running_machine *machine, int dma )
{
	snes_state *state = (snes_state *)machine->driver_data;
	return (state->dma_channel[dma].ibank << 16) | (state->dma_channel[dma].hdma_iaddr++);
}
#endif

static void snes_hdma_init( running_machine *machine )
{
	snes_state *state = (snes_state *)machine->driver_data;
	int i;

	for (i = 0; i < 8; i++)
	{
		if (BIT(state->hdmaen, i))
		{
			state->dma_channel[i].hdma_addr = state->dma_channel[i].src_addr;
			state->dma_channel[i].hdma_line_counter = 0;
		}
	}
}

static void snes_hdma( const address_space *space )
{
	snes_state *state = (snes_state *)space->machine->driver_data;
	UINT8 i, contmode;
	UINT16 bbus;
	UINT32 abus;

	/* Assume priority of the 8 DMA channels is 0-7 */
	for (i = 0; i < 8; i++)
	{
		if (BIT(state->hdmaen, i))
		{
			/* Check if we need to read a new line from the table */
			if (!(state->dma_channel[i].hdma_line_counter & 0x7f))
			{
				abus = (state->dma_channel[i].bank << 16) + state->dma_channel[i].hdma_addr;

				/* Get the number of lines */
				state->dma_channel[i].hdma_line_counter = memory_read_byte(space, abus);
				if (!state->dma_channel[i].hdma_line_counter)
				{
					/* No more lines so clear HDMA */
					state->hdmaen &= ~(1 << i);
					continue;
				}
				abus++;
				state->dma_channel[i].hdma_addr = abus;
				if (state->dma_channel[i].dmap & 0x40)
				{
					state->dma_channel[i].trans_size = memory_read_byte(space, abus++);
					state->dma_channel[i].trans_size |= (memory_read_byte(space, abus++) << 8);
					state->dma_channel[i].hdma_addr = abus;
				}
			}

			contmode = (--state->dma_channel[i].hdma_line_counter) & 0x80;

			/* Transfer addresses */
			if (state->dma_channel[i].dmap & 0x40)	/* Indirect */
				abus = (state->dma_channel[i].ibank << 16) + state->dma_channel[i].trans_size;
			else									/* Absolute */
				abus = (state->dma_channel[i].bank << 16) + state->dma_channel[i].hdma_addr;

			bbus = state->dma_channel[i].dest_addr + 0x2100;

#ifdef SNES_DBG_HDMA
			mame_printf_debug( "HDMA-Ch: %d(%s) abus: %X bbus: %X type: %d(%X)\n", i, state->dma_channel[i].dmap & 0x40 ? "Indirect" : "Absolute", abus, bbus, state->dma_channel[i].dmap & 0x07, state->dma_channel[i].hdma_addr);
#endif

			switch (state->dma_channel[i].dmap & 0x07)
			{
			case 0:		/* 1 register write once             (1 byte:  p               ) */
				snes_dma_transfer(space, i, abus++, bbus);
				break;
			case 5:		/* 2 registers write twice alternate (4 bytes: p, p+1, p,   p+1) */
				snes_dma_transfer(space, i, abus++, bbus);
				snes_dma_transfer(space, i, abus++, bbus + 1);
				snes_dma_transfer(space, i, abus++, bbus);
				snes_dma_transfer(space, i, abus++, bbus + 1);
				break;
			case 1:		/* 2 registers write once            (2 bytes: p, p+1          ) */
				snes_dma_transfer(space, i, abus++, bbus);
				snes_dma_transfer(space, i, abus++, bbus + 1);
				break;
			case 2:		/* 1 register write twice            (2 bytes: p, p            ) */
			case 6:
				snes_dma_transfer(space, i, abus++, bbus);
				snes_dma_transfer(space, i, abus++, bbus);
				break;
			case 3:		/* 2 registers write twice each      (4 bytes: p, p,   p+1, p+1) */
			case 7:
				snes_dma_transfer(space, i, abus++, bbus);
				snes_dma_transfer(space, i, abus++, bbus);
				snes_dma_transfer(space, i, abus++, bbus + 1);
				snes_dma_transfer(space, i, abus++, bbus + 1);
				break;
			case 4:		/* 4 registers write once            (4 bytes: p, p+1, p+2, p+3) */
				snes_dma_transfer(space, i, abus++, bbus);
				snes_dma_transfer(space, i, abus++, bbus + 1);
				snes_dma_transfer(space, i, abus++, bbus + 2);
				snes_dma_transfer(space, i, abus++, bbus + 3);
				break;
			default:
#ifdef MAME_DEBUG
				mame_printf_debug( "  HDMA of unsupported type: %d\n", state->dma_channel[i].dmap & 0x07);
#endif
				break;
			}

			/* Update address */
			if (contmode)
			{
				if (state->dma_channel[i].dmap & 0x40)	/* Indirect */
					state->dma_channel[i].trans_size = abus;
				else									/* Absolute */
					state->dma_channel[i].hdma_addr = abus;
			}

			if (!(state->dma_channel[i].hdma_line_counter & 0x7f))
			{
				if (!(state->dma_channel[i].dmap & 0x40))	/* Absolute */
				{
					if (!contmode)
						state->dma_channel[i].hdma_addr = abus;
				}
			}
		}
	}
}

static void snes_dma( const address_space *space, UINT8 channels )
{
	snes_state *state = (snes_state *)space->machine->driver_data;
	int i;
	INT8 increment;
	UINT16 bbus;
	UINT32 abus, abus_bank, length;

	/* Assume priority of the 8 DMA channels is 0-7 */
	for (i = 0; i < 8; i++)
	{
		if (BIT(channels, i))
		{
			/* FIXME: the following should be used to stop DMA if the same channel is used by HDMA (being set to 1 in snes_hdma)
            However, this cannot be implemented as is atm, because currently DMA transfers always happen as soon as they are enabled... */
			state->dma_channel[i].dma_disabled = 0;

			//printf( "Making a transfer on channel %d\n", i );
			/* Find transfer addresses */
			abus = state->dma_channel[i].src_addr;
			abus_bank = state->dma_channel[i].bank << 16;
			bbus = state->dma_channel[i].dest_addr + 0x2100;

			//printf("Address: %06x\n", abus | abus_bank);
			/* Auto increment */
			if (state->dma_channel[i].dmap & 0x8)
				increment = 0;
			else
			{
				if (state->dma_channel[i].dmap & 0x10)
					increment = -1;
				else
					increment = 1;
			}

			/* Number of bytes to transfer */
			length = state->dma_channel[i].trans_size;
			if (!length)
				length = 0x10000;	/* 0x0000 really means 0x10000 */

//          printf( "DMA-Ch %d: len: %X, abus: %X, bbus: %X, incr: %d, dir: %s, type: %d\n", i, length, abus, bbus, increment, state->dma_channel[i].dmap & 0x80 ? "PPU->CPU" : "CPU->PPU", state->dma_channel[i].dmap & 0x07);

#ifdef SNES_DBG_DMA
			mame_printf_debug( "DMA-Ch %d: len: %X, abus: %X, bbus: %X, incr: %d, dir: %s, type: %d\n", i, length, abus, bbus, increment, state->dma_channel[i].dmap & 0x80 ? "PPU->CPU" : "CPU->PPU", state->dma_channel[i].dmap & 0x07);
#endif

			switch (state->dma_channel[i].dmap & 0x07)
			{
			case 0:		/* 1 register write once */
			case 2:		/* 1 register write twice */
			case 6:		/* 1 register write twice */
				while (length-- && !state->dma_channel[i].dma_disabled)
				{
					snes_dma_transfer(space, i, (abus & 0xffff) | abus_bank, bbus);
					abus += increment;
				}
				break;
			case 1:		/* 2 registers write once */
			case 5:		/* 2 registers write twice alternate */
				while (length-- && !state->dma_channel[i].dma_disabled)
				{
					snes_dma_transfer(space, i, (abus & 0xffff) | abus_bank, bbus);
					abus += increment;
					if (!(length--) || state->dma_channel[i].dma_disabled)
						break;
					snes_dma_transfer(space, i, (abus & 0xffff) | abus_bank, bbus + 1);
					abus += increment;
				}
				break;
			case 3:		/* 2 registers write twice each */
			case 7:		/* 2 registers write twice each */
				while (length-- && !state->dma_channel[i].dma_disabled)
				{
					snes_dma_transfer(space, i, (abus & 0xffff) | abus_bank, bbus);
					abus += increment;
					if (!(length--) || state->dma_channel[i].dma_disabled)
						break;
					snes_dma_transfer(space, i, (abus & 0xffff) | abus_bank, bbus);
					abus += increment;
					if (!(length--) || state->dma_channel[i].dma_disabled)
						break;
					snes_dma_transfer(space, i, (abus & 0xffff) | abus_bank, bbus + 1);
					abus += increment;
					if (!(length--) || state->dma_channel[i].dma_disabled)
						break;
					snes_dma_transfer(space, i, (abus & 0xffff) | abus_bank, bbus + 1);
					abus += increment;
				}
				break;
			case 4:		/* 4 registers write once */
				while (length-- && !state->dma_channel[i].dma_disabled)
				{
					snes_dma_transfer(space, i, (abus & 0xffff) | abus_bank, bbus);
					abus += increment;
					if (!(length--) || state->dma_channel[i].dma_disabled)
						break;
					snes_dma_transfer(space, i, (abus & 0xffff) | abus_bank, bbus + 1);
					abus += increment;
					if (!(length--) || state->dma_channel[i].dma_disabled)
						break;
					snes_dma_transfer(space, i, (abus & 0xffff) | abus_bank, bbus + 2);
					abus += increment;
					if (!(length--) || state->dma_channel[i].dma_disabled)
						break;
					snes_dma_transfer(space, i, (abus & 0xffff) | abus_bank, bbus + 3);
					abus += increment;
				}
				break;
			default:
#ifdef MAME_DEBUG
				mame_printf_debug("  DMA of unsupported type: %d\n", state->dma_channel[i].dmap & 0x07);
#endif
				break;
			}

			/* We're done, so write the new abus back to the registers */
			state->dma_channel[i].src_addr = abus;
			state->dma_channel[i].trans_size = 0;
		}
	}
}

READ8_HANDLER( superfx_r_bank1 )
{
	return snes_ram[offset | 0x8000];
}

READ8_HANDLER( superfx_r_bank2 )
{
	return snes_ram[0x400000 + offset];
}

READ8_HANDLER( superfx_r_bank3 )
{
	//printf( "superfx_r_bank3: %08x = %02x\n", 0x600000 + offset, snes_ram[0x600000 + offset] );
	return snes_ram[0x600000 + offset];
}

WRITE8_HANDLER( superfx_w_bank1 )
{
	printf( "Attempting to write to cart ROM: %08x = %02x\n", offset, data );
	// Do nothing; can't write to cart ROM.
}

WRITE8_HANDLER( superfx_w_bank2 )
{
	printf( "Attempting to write to cart ROM: %08x = %02x\n", 0x400000 + offset, data );
	// Do nothing; can't write to cart ROM.
}

WRITE8_HANDLER( superfx_w_bank3 )
{
	//printf( "superfx_w_bank3: %08x = %02x\n", 0x600000 + offset, data );
	snes_ram[0x600000 + offset] = data;
}
