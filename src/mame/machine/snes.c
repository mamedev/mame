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
#include "includes/snes.h"
#include "cpu/superfx/superfx.h"
#include "cpu/g65816/g65816.h"


/* -- Globals -- */
UINT8  *snes_ram = NULL;		/* 65816 ram */
UINT8  *spc_ram = NULL;			/* spc700 ram */
UINT8  *snes_vram = NULL;		/* Video RAM (Should be 16-bit, but it's easier this way) */
UINT16 *snes_cgram = NULL;		/* Colour RAM */
UINT16 *snes_oam = NULL;		/* Object Attribute Memory */
static UINT16 cgram_address;	/* CGRAM address */
static UINT8  vram_read_offset;	/* VRAM read offset */
UINT8  spc_port_in[4];	/* Port for sending data to the SPC700 */
UINT8  spc_port_out[4];	/* Port for receiving data from the SPC700 */
static UINT8 snes_hdma_chnl;	/* channels enabled for HDMA */
static UINT8 read_ophct = 0, read_opvct = 0;
static emu_timer *snes_scanline_timer;
static emu_timer *snes_hblank_timer;
static emu_timer *snes_nmi_timer;
static emu_timer *snes_hirq_timer;
static emu_timer *snes_div_timer;
static emu_timer *snes_mult_timer;
static UINT16 hblank_offset;
UINT16 snes_htmult;	/* in 512 wide, we run HTOTAL double and halve it on latching */
UINT8 snes_has_addon_chip;
UINT32 snes_rom_size;

// full graphic variables
static UINT16 vram_fgr_high, vram_fgr_increment, vram_fgr_count, vram_fgr_mask, vram_fgr_shift, vram_read_buffer;
static const UINT16 vram_fgr_inctab[4] = { 1, 32, 128, 128 };
static const UINT16 vram_fgr_inccnts[4] = { 0, 32, 64, 128 };
static const UINT16 vram_fgr_shiftab[4] = { 0, 5, 6, 7 };

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
void snes_latch_counters(running_machine *machine)
{
	snes_ppu.beam.current_horz = video_screen_get_hpos(machine->primary_screen) / snes_htmult;
	snes_ppu.beam.latch_vert = video_screen_get_vpos(machine->primary_screen);
	snes_ppu.beam.latch_horz = snes_ppu.beam.current_horz;
	snes_ram[STAT78] |= 0x40;	// indicate we latched
//  read_ophct = read_opvct = 0;    // clear read flags - 2009-08: I think we must clear these when STAT78 is read...

//  printf("latched @ H %d V %d\n", snes_ppu.beam.latch_horz, snes_ppu.beam.latch_vert);
}

static TIMER_CALLBACK( snes_nmi_tick )
{
	// pull NMI
	cputag_set_input_line(machine, "maincpu", G65816_LINE_NMI, ASSERT_LINE );

	// don't happen again
	timer_adjust_oneshot(snes_nmi_timer, attotime_never, 0);
}

static void snes_hirq_tick(running_machine *machine)
{
	// latch the counters and pull IRQ
	// (don't need to switch to the 65816 context, we don't do anything dependant on it)
	snes_latch_counters(machine);
	snes_ram[TIMEUP] = 0x80;	/* Indicate that irq occured */
	cputag_set_input_line(machine, "maincpu", G65816_LINE_IRQ, ASSERT_LINE );

	// don't happen again
	timer_adjust_oneshot(snes_hirq_timer, attotime_never, 0);
}

static TIMER_CALLBACK( snes_hirq_tick_callback )
{
	snes_hirq_tick(machine);
}

static TIMER_CALLBACK( snes_reset_oam_address )
{
	// make sure we're in the 65816's context since we're messing with the OAM and stuff
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);

	if (!(snes_ppu.screen_disabled)) //Reset OAM address, byuu says it happens at H=10
	{
		memory_write_byte(space, OAMADDL, snes_ppu.oam.saved_address_low ); /* Reset oam address */
		memory_write_byte(space, OAMADDH, snes_ppu.oam.saved_address_high );
	}
}

static TIMER_CALLBACK( snes_reset_hdma )
{
	snes_hdma_init();
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
			cputag_set_input_line(machine, "maincpu", G65816_LINE_IRQ, ASSERT_LINE );
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
//          printf("HIRQ @ %d, %d\n", pixel*snes_htmult, snes_ppu.beam.current_vert);
			if (pixel == 0)
			{
				snes_hirq_tick(machine);
			}
			else
			{
				timer_adjust_oneshot(snes_hirq_timer, video_screen_get_time_until_pos(machine->primary_screen, snes_ppu.beam.current_vert, pixel*snes_htmult), 0);
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
			timer_adjust_oneshot(snes_nmi_timer, cputag_clocks_to_attotime(machine, "maincpu", 12), 0);
		}
	}

	// hdma reset happens at scanline 0, H=~6
	if (snes_ppu.beam.current_vert == 0)
	{
		snes_hdma_init();
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

		cputag_set_input_line(machine, "maincpu", G65816_LINE_NMI, CLEAR_LINE );
	}

	timer_adjust_oneshot(snes_scanline_timer, attotime_never, 0);
	timer_adjust_oneshot(snes_hblank_timer, video_screen_get_time_until_pos(machine->primary_screen, snes_ppu.beam.current_vert, hblank_offset*snes_htmult), 0);
}

/* This is called at the start of hblank *before* the scanline indicated in current_vert! */
static TIMER_CALLBACK( snes_hblank_tick )
{
	const address_space *cpu0space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);
	int nextscan;

	snes_ppu.beam.current_vert = video_screen_get_vpos(machine->primary_screen);

	/* make sure we halt */
	timer_adjust_oneshot(snes_hblank_timer, attotime_never, 0);

	/* draw a scanline */
	if (snes_ppu.beam.current_vert <= snes_ppu.beam.last_visible_line)
	{
		if (video_screen_get_vpos(machine->primary_screen) > 0)
		{
			/* Do HDMA */
			if (snes_ram[HDMAEN])
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

	timer_adjust_oneshot(snes_scanline_timer, video_screen_get_time_until_pos(machine->primary_screen, nextscan, 0), 0);
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
	rectangle visarea = *video_screen_get_visible_area(machine->primary_screen);
	attoseconds_t refresh;

	visarea.min_x = visarea.min_y = 0;
	visarea.max_y = snes_ppu.beam.last_visible_line*snes_ppu.interlace - 1;
	visarea.max_x = (SNES_SCR_WIDTH * 2) - 1;

	// fixme: should compensate for SNES_DBG_video
	if (snes_ppu.mode == 5 || snes_ppu.mode == 6 )
		snes_htmult = 2;
	else
		snes_htmult = 1;

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
	result = memory_read_byte_8le(space, cpu_get_pc(space->cpu)-1); //LAST opcode that's fetched on the bus
	recurse = 0;
	return result;
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
		return spc_port_out[offset & 0x3];
	}

	if (snes_has_addon_chip == HAS_SUPERFX && devtag_get_device(space->machine, "superfx") != NULL)
	{
		if (offset >= 0x3000 && offset < 0x3300)
		{
			return superfx_mmio_read(devtag_get_device(space->machine, "superfx"), offset);
		}
	}
	else if (snes_has_addon_chip == HAS_RTC)
	{
		if (offset == 0x2800 || offset == 0x2801)
		{
			return srtc_mmio_read(space->machine, offset);
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

// According to BSNES, these should return snes_open_bus_r!
//      case OAMADDL:
//      case OAMADDH:
//      case VMADDH:
//      case CGADD:
//      case CGDATA:
//          return snes_ram[offset];
		case MPYL:		/* Multiplication result (low) */
			{
				/* Perform 16bit * 8bit multiply */
				UINT32 c = snes_ppu.mode7.matrix_a * (INT8)(snes_ppu.mode7.matrix_b >> 8);
				snes_ppu.ppu1_open_bus = c & 0xff;
				return snes_ppu.ppu1_open_bus;
			}
		case MPYM:		/* Multiplication result (mid) */
			{
				/* Perform 16bit * 8bit multiply */
				UINT32 c = snes_ppu.mode7.matrix_a * (INT8)(snes_ppu.mode7.matrix_b >> 8);
				snes_ppu.ppu1_open_bus = (c >> 8) & 0xff;
				return snes_ppu.ppu1_open_bus;
			}
		case MPYH:		/* Multiplication result (high) */
			{
				/* Perform 16bit * 8bit multiply */
				UINT32 c = snes_ppu.mode7.matrix_a * (INT8)(snes_ppu.mode7.matrix_b >> 8);
				snes_ppu.ppu1_open_bus = (c >> 16) & 0xff;
				return snes_ppu.ppu1_open_bus;
			}
		case SLHV:		/* Software latch for H/V counter */
			snes_latch_counters(space->machine);
			return snes_open_bus_r(space,0);		/* Return value is meaningless */
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
				if (snes_ram[OAMDATA] == 0)
				{
					snes_ppu.oam.address++;
					snes_ppu.oam.address_low = snes_ram[OAMADDL] = snes_ppu.oam.address & 0xff;
					snes_ppu.oam.address_high = snes_ram[OAMADDH] = (snes_ppu.oam.address >> 8) & 0x1;
				}
				return snes_ppu.ppu1_open_bus;
			}
		case RVMDATAL:	/* Read data from VRAM (low) */
			{
				UINT32 addr = (snes_ram[VMADDH] << 8) | snes_ram[VMADDL];

				snes_ppu.ppu1_open_bus = vram_read_buffer & 0xff;

				if (!vram_fgr_high)
				{
					if (vram_fgr_count)
					{
						UINT32 rem = addr & vram_fgr_mask;
						UINT32 faddr = (addr & ~vram_fgr_mask) + (rem >> vram_fgr_shift) +
							       ((rem & (vram_fgr_count - 1)) << 3);

						vram_read_buffer = snes_vram[(faddr<<1)&0x1ffff] | snes_vram[((faddr<<1)+1) & 0x1ffff]<<8;
					}
					else
					{
						vram_read_buffer = snes_vram[(addr<<1)&0x1ffff] | snes_vram[((addr<<1)+1) & 0x1ffff]<<8;
					}

					addr += vram_fgr_increment;
					snes_ram[VMADDL] = addr&0xff;
					snes_ram[VMADDH] = (addr>>8)&0xff;
				}
			}
			return snes_ppu.ppu1_open_bus;
		case RVMDATAH:	/* Read data from VRAM (high) */
			{
				UINT32 addr = (snes_ram[VMADDH] << 8) | snes_ram[VMADDL];

				snes_ppu.ppu1_open_bus = (vram_read_buffer>>8) & 0xff;

				if (vram_fgr_high)
				{
					if (vram_fgr_count)
					{
						UINT32 rem = addr & vram_fgr_mask;
						UINT32 faddr = (addr & ~vram_fgr_mask) + (rem >> vram_fgr_shift) +
							       ((rem & (vram_fgr_count - 1)) << 3);

						vram_read_buffer = snes_vram[(faddr<<1)&0x1ffff] | snes_vram[((faddr<<1)+1) & 0x1ffff]<<8;
					}
					else
					{
						vram_read_buffer = snes_vram[(addr<<1)&0x1ffff] | snes_vram[((addr<<1)+1) & 0x1ffff]<<8;
					}

					addr += vram_fgr_increment;
					snes_ram[VMADDL] = addr&0xff;
					snes_ram[VMADDH] = (addr>>8)&0xff;
				}
			}
			return snes_ppu.ppu1_open_bus;
		case RCGDATA:	/* Read data from CGRAM */
				if (!(cgram_address & 0x01))
				{
					snes_ppu.ppu2_open_bus = ((UINT8 *)snes_cgram)[cgram_address] & 0xff;
				}
				else
				{
					snes_ppu.ppu2_open_bus &= 0x80;
					snes_ppu.ppu2_open_bus |= ((UINT8 *)snes_cgram)[cgram_address] & 0x7f;
				}

				cgram_address = (cgram_address + 1) % (SNES_CGRAM_SIZE - 2);
				return snes_ppu.ppu2_open_bus;
		case OPHCT:		/* Horizontal counter data by ext/soft latch */
			{
				if (read_ophct)
				{
					snes_ppu.ppu2_open_bus &= 0xfe;
					snes_ppu.ppu2_open_bus |= (snes_ppu.beam.latch_horz >> 8) & 0x01;
				}
				else
				{
					snes_ppu.ppu2_open_bus = snes_ppu.beam.latch_horz & 0xff;
				}
				read_ophct ^= 1;
				return snes_ppu.ppu2_open_bus;
			}
		case OPVCT:		/* Vertical counter data by ext/soft latch */
			{
				if (read_opvct)
				{
					snes_ppu.ppu2_open_bus &= 0xfe;
					snes_ppu.ppu2_open_bus |= (snes_ppu.beam.latch_vert >> 8) & 0x01;
				}
				else
				{
					snes_ppu.ppu2_open_bus = snes_ppu.beam.latch_vert & 0xff;
				}
				read_opvct ^= 1;
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
			read_ophct = 0;
			read_opvct = 0;
			value = snes_ram[offset];
			value |= (snes_ppu.ppu1_open_bus & 0x20);
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
			value = (snes_ram[offset] & 0x8f) | (snes_open_bus_r(space,0) & 0x70);
			snes_ram[offset] &= 0x7f;	/* NMI flag is reset on read */
			return value;
		case TIMEUP:		/* IRQ flag by H/V count timer */
			value = (snes_open_bus_r(space,0) & 0x7f) | (snes_ram[TIMEUP] & 0x80);
			cputag_set_input_line(space->machine, "maincpu", G65816_LINE_IRQ, CLEAR_LINE );
			snes_ram[TIMEUP] = 0;
			return value;
		case HVBJOY:		/* H/V blank and joypad controller enable */
			// electronics test says hcounter 272 is start of hblank, which is beampos 363
//          if (video_screen_get_hpos(space->machine->primary_screen) >= 363) snes_ram[offset] |= 0x40;
//              else snes_ram[offset] &= ~0x40;
			return (snes_ram[offset] & 0xc1) | (snes_open_bus_r(space,0) & 0x3e);
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
		case DMAP0:
		case DMAP1:
		case DMAP2:
		case DMAP3:
		case DMAP4:
		case DMAP5:
		case DMAP6:
		case DMAP7:
//          return (snes_ram[offset] & 0xdf) | (snes_open_bus_r(space,0) & 0x20);
		case BBAD0: case A1T0L: case A1T0H: case A1B0: case DAS0L:
		case DAS0H: case DSAB0: case A2A0L: case A2A0H: case NTRL0:
		case BBAD1: case A1T1L: case A1T1H: case A1B1: case DAS1L:
		case DAS1H: case DSAB1: case A2A1L: case A2A1H: case NTRL1:
		case BBAD2: case A1T2L: case A1T2H: case A1B2: case DAS2L:
		case DAS2H: case DSAB2: case A2A2L: case A2A2H: case NTRL2:
		case BBAD3: case A1T3L: case A1T3H: case A1B3: case DAS3L:
		case DAS3H: case DSAB3: case A2A3L: case A2A3H: case NTRL3:
		case BBAD4: case A1T4L: case A1T4H: case A1B4: case DAS4L:
		case DAS4H: case DSAB4: case A2A4L: case A2A4H: case NTRL4:
		case BBAD5: case A1T5L: case A1T5H: case A1B5: case DAS5L:
		case DAS5H: case DSAB5: case A2A5L: case A2A5H: case NTRL5:
		case BBAD6: case A1T6L: case A1T6H: case A1B6: case DAS6L:
		case DAS6H: case DSAB6: case A2A6L: case A2A6H: case NTRL6:
		case BBAD7: case A1T7L: case A1T7H: case A1B7: case DAS7L:
		case DAS7H: case DSAB7: case A2A7L: case A2A7H: case NTRL7:
			return snes_ram[offset];

#ifndef MESS
		case 0x4100:		/* NSS Dip-Switches */
#ifdef MAME_DEBUG
			return input_port_read_safe(space->machine, "DEBUG1", 0);
#else
			return input_port_read(space->machine, "DSW");
#endif	/* MAME_DEBUG */
//      case 0x4101: //PC: a104 - a10e - a12a   //only nss_actr
//      case 0x420c: //PC: 9c7d - 8fab          //only nss_ssoc

		default:
			mame_printf_debug("snes_r: offset = %x pc = %x\n",offset,cpu_get_pc(space->cpu));
#endif	/* MESS */

	}

	//printf("unsupported read: offset == %08x\n", offset);

	/* Unsupported reads returns open bus */
//  printf("%02x %02x\n",offset,snes_open_bus_r(space,0));
	return snes_open_bus_r(space,0);
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
//      printf("816: %02x to APU @ %d\n", data, offset&3);
		spc_port_in[offset & 0x3] = data;
		cpuexec_boost_interleave(space->machine, attotime_zero, ATTOTIME_IN_USEC(20));
		return;
	}

	if (snes_has_addon_chip == HAS_SUPERFX && devtag_get_device(space->machine, "superfx") != NULL)
	{
		if (offset >= 0x3000 && offset < 0x3300)
		{
			superfx_mmio_write(devtag_get_device(space->machine, "superfx"), offset, data);
			return;
		}
	}
	else if (snes_has_addon_chip == HAS_RTC)
	{
		if (offset == 0x2800 || offset == 0x2801)
		{
			srtc_mmio_write(space->machine, offset, data);
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
			snes_ppu.oam.address_low = data;
			snes_ppu.oam.saved_address_low = data;
			snes_ppu.oam.address = ((snes_ppu.oam.address_high & 0x1) << 8) + data;
			snes_ram[OAMDATA] = 0;
			break;
		case OAMADDH:	/* Address for accessing OAM (high) */
			snes_ppu.oam.address_high = data & 0x1;
			snes_ppu.oam.saved_address_high = data;
			snes_ppu.oam.address = ((data & 0x1) << 8) + snes_ppu.oam.address_low;
			snes_ppu.oam.priority_rotation = BIT(data, 7);
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
					snes_ppu.oam.address_low = snes_ram[OAMADDL] = snes_ppu.oam.address & 0xff;
					snes_ppu.oam.address_high = snes_ram[OAMADDH] = (snes_ppu.oam.address >> 8) & 0x1;
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
			/* FIXME: We support horizontal mosaic only partially */
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
			vram_fgr_high = (data & 0x80);
			vram_fgr_increment = vram_fgr_inctab[data & 3];

			if (data & 0xc)
			{
				int md = (data & 0xc) >> 2;

				vram_fgr_count = vram_fgr_inccnts[md];
				vram_fgr_mask = (vram_fgr_count * 8) - 1;
				vram_fgr_shift = vram_fgr_shiftab[md];
			}
			else
			{
				vram_fgr_count = 0;
			}
//          printf("VMAIN: high %x inc %x count %x mask %x shift %x\n", vram_fgr_high, vram_fgr_increment, vram_fgr_count, vram_fgr_mask, vram_fgr_shift);
			break;
		case VMADDL:	/* Address for VRAM read/write (low) */
		case VMADDH:	/* Address for VRAM read/write (high) */
			{
				UINT32 addr;

				snes_ram[offset] = data;
				addr = (snes_ram[VMADDH] << 8) | snes_ram[VMADDL];

				if (vram_fgr_count)
				{
					UINT32 rem = addr & vram_fgr_mask;
					UINT32 faddr = (addr & ~vram_fgr_mask) + (rem >> vram_fgr_shift) + ((rem & (vram_fgr_count - 1)) << 3);

					vram_read_buffer = snes_vram[(faddr << 1) & 0x1ffff] | snes_vram[((faddr << 1) + 1) & 0x1ffff] << 8;
				}
				else
				{
					vram_read_buffer = snes_vram[(addr << 1) & 0x1ffff] | snes_vram[((addr << 1) + 1) & 0x1ffff] << 8;
				}

			}
			break;
		case VMDATAL:	/* 2118: Data for VRAM write (low) */
			{
				UINT32 addr = (snes_ram[VMADDH] << 8) | snes_ram[VMADDL];

				if (vram_fgr_count)
				{
					UINT32 rem = addr & vram_fgr_mask;
					UINT32 faddr = (addr & ~vram_fgr_mask) + (rem >> vram_fgr_shift) + ((rem & (vram_fgr_count - 1)) << 3);

					snes_vram[(faddr << 1) & 0x1ffff] = data;
				}
				else
				{
					snes_vram[(addr << 1) & 0x1ffff] = data;
				}

				if (!vram_fgr_high)
				{
					addr += vram_fgr_increment;
					snes_ram[VMADDL] = addr & 0xff;
					snes_ram[VMADDH] = (addr >> 8) & 0xff;
				}
			}
			return;
		case VMDATAH:	/* 2119: Data for VRAM write (high) */
			{
				UINT32 addr = (snes_ram[VMADDH] << 8) | snes_ram[VMADDL];

				if (vram_fgr_count)
				{
					UINT32 rem = addr & vram_fgr_mask;
					UINT32 faddr = (addr & ~vram_fgr_mask) + (rem >> vram_fgr_shift) + ((rem & (vram_fgr_count - 1)) << 3);

					snes_vram[((faddr << 1) + 1) & 0x1ffff] = data;
				}
				else
				{
					snes_vram[((addr << 1) + 1) & 0x1ffff] = data;
				}

				if (vram_fgr_high)
				{
					addr += vram_fgr_increment;
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
			cgram_address = data << 1;
			break;
		case CGDATA:	/* Data for colour RAM */
			((UINT8 *)snes_cgram)[cgram_address] = data;
			cgram_address = (cgram_address + 1) % (SNES_CGRAM_SIZE - 2);
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
			snes_ppu.main_bg_enabled[0] = BIT(data, 0);
			snes_ppu.main_bg_enabled[1] = BIT(data, 1);
			snes_ppu.main_bg_enabled[2] = BIT(data, 2);
			snes_ppu.main_bg_enabled[3] = BIT(data, 3);
			snes_ppu.main_bg_enabled[4] = BIT(data, 4);
			break;
		case TS:		/* Subscreen designation */
			snes_ppu.sub_bg_enabled[0] = BIT(data, 0);
			snes_ppu.sub_bg_enabled[1] = BIT(data, 1);
			snes_ppu.sub_bg_enabled[2] = BIT(data, 2);
			snes_ppu.sub_bg_enabled[3] = BIT(data, 3);
			snes_ppu.sub_bg_enabled[4] = BIT(data, 4);
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
			/* FIXME: We don't support direct select for modes 3 & 4 or subscreen window stuff */
			snes_ppu.main_color_mask = (data >> 6) & 0x03;
			snes_ppu.sub_color_mask = (data >> 4) & 0x03;
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
			/* FIXME: We only support line count and interlace here */
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
//          timer_adjust_oneshot(snes_mult_timer, cputag_clocks_to_attotime(space->machine, "maincpu", 8), 0);
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
//          timer_adjust_oneshot(snes_div_timer, cputag_clocks_to_attotime(space->machine, "maincpu", 16), 0);
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
		case MDMAEN:	/* GDMA channel designation and trigger */
			snes_gdma(space, data);
			data = 0;	/* Once DMA is done we need to reset all bits to 0 */
			break;
		case HDMAEN:	/* HDMA channel designation */
			if (data) //if a HDMA is enabled, data is inited at the next scanline
				timer_set(space->machine, video_screen_get_time_until_pos(space->machine->primary_screen, snes_ppu.beam.current_vert+1, 0), NULL, 0, snes_reset_hdma);
			break;
		case MEMSEL:	/* Access cycle designation in memory (2) area */
			/* FIXME: Need to adjust the speed only during access of banks 0x80+
             * Currently we are just increasing it no matter what */
//          cpu_set_clockscale(devtag_get_device(space->machine, "maincpu"), (data & 0x1) ? 1.335820896 : 1.0 );
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
		case DMAP0: case BBAD0: case A1T0L: case A1T0H: case A1B0: case DAS0L:
		case DAS0H: case DSAB0: case A2A0L: case A2A0H: case NTRL0:
		case DMAP1: case BBAD1: case A1T1L: case A1T1H: case A1B1: case DAS1L:
		case DAS1H: case DSAB1: case A2A1L: case A2A1H: case NTRL1:
		case DMAP2: case BBAD2: case A1T2L: case A1T2H: case A1B2: case DAS2L:
		case DAS2H: case DSAB2: case A2A2L: case A2A2H: case NTRL2:
		case DMAP3: case BBAD3: case A1T3L: case A1T3H: case A1B3: case DAS3L:
		case DAS3H: case DSAB3: case A2A3L: case A2A3H: case NTRL3:
		case DMAP4: case BBAD4: case A1T4L: case A1T4H: case A1B4: case DAS4L:
		case DAS4H: case DSAB4: case A2A4L: case A2A4H: case NTRL4:
		case DMAP5: case BBAD5: case A1T5L: case A1T5H: case A1B5: case DAS5L:
		case DAS5H: case DSAB5: case A2A5L: case A2A5H: case NTRL5:
		case DMAP6: case BBAD6: case A1T6L: case A1T6H: case A1B6: case DAS6L:
		case DAS6H: case DSAB6: case A2A6L: case A2A6H: case NTRL6:
		case DMAP7: case BBAD7: case A1T7L: case A1T7H: case A1B7: case DAS7L:
		case DAS7H: case DSAB7: case A2A7L: case A2A7H: case NTRL7:
			break;
	}

	snes_ram[offset] = data;
}

WRITE_LINE_DEVICE_HANDLER( snes_extern_irq_w )
{
	cputag_set_input_line(device->machine, "maincpu", G65816_LINE_IRQ, state);
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
	UINT8 value;
	UINT16 address = offset & 0xffff;

	if (address < 0x2000)											/* Mirror of Low RAM */
		value = memory_read_byte(space, 0x7e0000 + address);
	else if (address < 0x6000)										/* I/O */
		value = snes_r_io(space, address);
	else if (address < 0x8000)
	{
		if (snes_has_addon_chip == HAS_SUPERFX && devtag_get_device(space->machine, "superfx") != NULL)
			value = snes_ram[0x700000 + (address & 0x1fff)];
		else if (snes_has_addon_chip == HAS_OBC1)
			value = obc1_read(space, offset);
		else if ((snes_has_addon_chip == HAS_DSP2) && (offset >= 0x200000))
			value = (address < 0x7000) ? DSP2_read() : 0x00;
		else if ((snes_cart.mode == SNES_MODE_21) && (snes_has_addon_chip == HAS_DSP1) && (offset < 0x100000))
			value = (address < 0x7000) ? DSP1_getDr() : DSP1_getSr();
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
		value = (address < 0xc000) ? DSP1_getDr() : DSP1_getSr();
	else if ((snes_cart.mode == SNES_MODE_20) && (snes_has_addon_chip == HAS_DSP2) && (offset >= 0x200000))
		value = (address < 0xc000) ? DSP2_read() : 0x00;
	else if ((snes_has_addon_chip == HAS_DSP3) && (offset >= 0x200000))
		value = DSP3_read(address);
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
			value = (address < 0x7000) ? DSP2_read() : 0x00;
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
		value = (address < 0xc000) ? DSP1_getDr() : DSP1_getSr();
	else if ((snes_cart.mode == SNES_MODE_20) && (snes_has_addon_chip == HAS_DSP2))
		value = (address < 0xc000) ? DSP2_read() : 0x00;
	else if (snes_has_addon_chip == HAS_DSP3)
		value = DSP3_read(address);
	else if (snes_has_addon_chip == HAS_DSP4)
		value = (address < 0xc000) ? DSP4_read() : 0x80;
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
	UINT8 value = 0xff;
	UINT16 address = offset & 0xffff;

	if (snes_has_addon_chip == HAS_SUPERFX && devtag_get_device(space->machine, "superfx") != NULL)
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
			value = (address >= 0x4000) ? DSP1_getSr() : DSP1_getDr();
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
	UINT8 value;
	UINT16 address = offset & 0xffff;

	if (snes_has_addon_chip == HAS_SUPERFX && devtag_get_device(space->machine, "superfx") != NULL)
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
	UINT8 value = 0;
	UINT16 address = offset & 0xffff;

	if (address < 0x8000)
	{
		if (address >= 0x6000 && snes_has_addon_chip == HAS_SUPERFX && devtag_get_device(space->machine, "superfx") != NULL)
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
		value = (address < 0xc000) ? DSP1_getDr() : DSP1_getSr();
	else if ((snes_cart.mode == SNES_MODE_20) && (snes_has_addon_chip == HAS_DSP2) && (offset >= 0x200000))
		value = (address < 0xc000) ? DSP2_read() : 0x00;
	else if ((snes_has_addon_chip == HAS_DSP3) && (offset >= 0x200000))
		value = DSP3_read(address);
	else if ((snes_has_addon_chip == HAS_DSP4) && (offset >= 0x300000))
		value = (address < 0xc000) ? DSP4_read() : 0x80;
	else
		value = snes_ram[0x800000 + offset];

	return value;
}

/* 0xc00000 - 0xffffff */
READ8_HANDLER( snes_r_bank7 )
{
	UINT8 value = 0;
	UINT16 address = offset & 0xffff;

	if (snes_has_addon_chip == HAS_SDD1)
	{
		return sdd1_read(space->machine, offset);
	}
	else if (snes_has_addon_chip == HAS_SUPERFX && devtag_get_device(space->machine, "superfx") != NULL)
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
			DSP2_write(data);
		else if ((snes_cart.mode == SNES_MODE_21) && (snes_has_addon_chip == HAS_DSP1) && (offset < 0x100000))
			DSP1_setDr(data);
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
		DSP1_setDr(data);
	else if ((snes_cart.mode == SNES_MODE_20) && (snes_has_addon_chip == HAS_DSP2) && (offset >= 0x200000) && (address < 0xc000))
		DSP2_write(data);
	else if ((snes_has_addon_chip == HAS_DSP3) && (offset >= 0x200000))
		DSP3_write(address, data);
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
			DSP2_write(data);
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
		DSP1_setDr(data);
	else if ((snes_cart.mode == SNES_MODE_20) && (snes_has_addon_chip == HAS_DSP2) && (address < 0xc000))
		DSP2_write(data);
	else if (snes_has_addon_chip == HAS_DSP3)
		DSP3_write(address, data);
	else if ((snes_has_addon_chip == HAS_DSP4) && (address < 0xc000))
		DSP4_write(data);
	else
		logerror("Attempt to write to ROM address: %X\n", offset + 0x300000);
}

/* 0x600000 - 0x6fffff */
WRITE8_HANDLER( snes_w_bank4 )
{
	UINT16 address = offset & 0xffff;

	if (snes_has_addon_chip == HAS_SUPERFX && devtag_get_device(space->machine, "superfx") != NULL)
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
			DSP1_setDr(data);
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
	UINT16 address = offset & 0xffff;

	if (snes_has_addon_chip == HAS_SUPERFX && devtag_get_device(space->machine, "superfx") != NULL)
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
	UINT16 address = offset & 0xffff;

	if (address < 0x8000)
	{
		if (address >= 0x6000 && snes_has_addon_chip == HAS_SUPERFX && devtag_get_device(space->machine, "superfx") != NULL)
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
		DSP1_setDr(data);
	else if ((snes_cart.mode == SNES_MODE_20) && (snes_has_addon_chip == HAS_DSP2) && (offset >= 0x200000) && (address < 0xc000))
		DSP2_write(data);
	else if ((snes_has_addon_chip == HAS_DSP3) && (offset >= 0x200000))
		DSP3_write(address, data);
	else if ((snes_has_addon_chip == HAS_DSP4) && (offset >= 0x300000) && (address < 0xc000))
		DSP4_write(data);
	else if (snes_has_addon_chip == HAS_SUPERFX && devtag_get_device(space->machine, "superfx") != NULL)
		logerror( "snes_w_bank6 hit (ROM) in Super FX mode, please fix me\n" );
	else
		logerror("Attempt to write to ROM address: %X\n", offset + 0x800000);
}


/* 0xc00000 - 0xffffff */
WRITE8_HANDLER( snes_w_bank7 )
{
	UINT16 address = offset & 0xffff;

	if (snes_has_addon_chip == HAS_SUPERFX && devtag_get_device(space->machine, "superfx") != NULL)
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
	int i;

	state->joypad[0].low  = input_port_read(machine, "SERIAL1_DATA1_L");
	state->joypad[0].high = input_port_read(machine, "SERIAL1_DATA1_H");
	state->joypad[1].low  = input_port_read(machine, "SERIAL2_DATA1_L");
	state->joypad[1].high = input_port_read(machine, "SERIAL2_DATA1_H");
	state->joypad[2].low  = input_port_read(machine, "SERIAL1_DATA2_L");
	state->joypad[2].high = input_port_read(machine, "SERIAL1_DATA2_H");
	state->joypad[3].low  = input_port_read(machine, "SERIAL2_DATA2_L");
	state->joypad[3].high = input_port_read(machine, "SERIAL2_DATA2_H");

	// avoid sending signals that could crash games
	for (i = 0; i < 4; i++)
	{
		// if left, no right
		if (state->joypad[i].high & 2)
			state->joypad[i].high &= ~1;
		// if up, no down
		if (state->joypad[i].high & 8)
			state->joypad[i].high &= ~4;
	}

	// is automatic reading on?
	if (snes_ram[NMITIMEN] & 1)
	{
		state->joy1l = state->joypad[0].low;
		state->joy1h = state->joypad[0].high;
		state->joy2l = state->joypad[1].low;
		state->joy2h = state->joypad[1].high;
		state->joy3l = state->joypad[2].low;
		state->joy3h = state->joypad[2].high;
		state->joy4l = state->joypad[3].low;
		state->joy4h = state->joypad[3].high;

		// make sure read_idx starts returning all 1s because the auto-read reads it :-)
		state->read_idx[0] = 16;
		state->read_idx[1] = 16;
	}

}

static UINT8 nss_oldjoy1_read( running_machine *machine )
{
	snes_state *state = (snes_state *)machine->driver_data;
	UINT8 res;

	// joysticks
	if (state->read_idx[0] >= 16)
		res = 0x01;
	else
		res = ((state->joypad[0].low | (state->joypad[0].high << 8)) >> (15 - state->read_idx[0]++)) & 0x01;

	return res;
}

static UINT8 nss_oldjoy2_read( running_machine *machine )
{
	snes_state *state = (snes_state *)machine->driver_data;
	UINT8 res;

	if (state->read_idx[1] >= 16)
		res = 0x01;
	else
		res = ((state->joypad[1].low | (state->joypad[1].high << 8)) >> (15 - state->read_idx[1]++)) & 0x01;

	return res;
}

/*************************************

    Driver Init

*************************************/

static void snes_init_timers(running_machine *machine)
{
	/* init timers and stop them */
	snes_scanline_timer = timer_alloc(machine, snes_scanline_tick, NULL);
	timer_adjust_oneshot(snes_scanline_timer, attotime_never, 0);
	snes_hblank_timer = timer_alloc(machine, snes_hblank_tick, NULL);
	timer_adjust_oneshot(snes_hblank_timer, attotime_never, 0);
	snes_nmi_timer = timer_alloc(machine, snes_nmi_tick, NULL);
	timer_adjust_oneshot(snes_nmi_timer, attotime_never, 0);
	snes_hirq_timer = timer_alloc(machine, snes_hirq_tick_callback, NULL);
	timer_adjust_oneshot(snes_hirq_timer, attotime_never, 0);
	snes_div_timer = timer_alloc(machine, snes_div_callback, NULL);
	timer_adjust_oneshot(snes_div_timer, attotime_never, 0);
	snes_mult_timer = timer_alloc(machine, snes_mult_callback, NULL);
	timer_adjust_oneshot(snes_mult_timer, attotime_never, 0);

	// SNES hcounter has a 0-339 range.  hblank starts at counter 260.
	// clayfighter sets an HIRQ at 260, apparently it wants it to be before hdma kicks off, so we'll delay 2 pixels.
	hblank_offset = 268;
	timer_adjust_oneshot(snes_hblank_timer, video_screen_get_time_until_pos(machine->primary_screen, ((snes_ram[STAT78] & 0x10) == SNES_NTSC) ? SNES_VTOTAL_NTSC-1 : SNES_VTOTAL_PAL-1, hblank_offset), 0);

}

static void snes_init_ram(running_machine *machine)
{
	snes_state *state = (snes_state *)machine->driver_data;
	const address_space *cpu0space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);
	int i, j;

	/* Init VRAM */
	memset(snes_vram, 0, SNES_VRAM_SIZE );

	/* Init Colour RAM */
	memset((UINT8 *)snes_cgram, 0, SNES_CGRAM_SIZE);

	/* Init oam RAM */
	memset(snes_oam, 0xff, SNES_OAM_SIZE );

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
	cgram_address = 0;
	vram_read_offset = 2;

	state->joy1l = state->joy1h = state->joy2l = state->joy2h = state->joy3l = state->joy3h = 0;
	state->joypad[0].low = state->joypad[1].low = state->joypad[2].low = state->joypad[3].low = 0;
	state->joypad[0].high = state->joypad[1].high = state->joypad[2].high = state->joypad[3].high = 0;

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
			DSP1_reset(machine);
			break;

		case HAS_DSP2:
			DSP2_reset();
			break;

		case HAS_DSP3:
			InitDSP3(machine);
			break;

		case HAS_DSP4:
			InitDSP4();
			break;

		case HAS_RTC:
			srtc_reset(machine);
			break;

		case HAS_SDD1:
			sdd1_reset(machine);
			break;

		case HAS_OBC1:
			obc1_init();
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

void snes_hdma_init()
{
	UINT8 mask = 1, dma = 0, i;

	snes_hdma_chnl = snes_ram[HDMAEN];
	for (i = 0; i < 8; i++)
	{
		if (snes_ram[HDMAEN] & mask)
		{
			snes_ram[SNES_DMA_BASE + dma + 8] = snes_ram[SNES_DMA_BASE + dma + 2];
			snes_ram[SNES_DMA_BASE + dma + 9] = snes_ram[SNES_DMA_BASE + dma + 3];
			snes_ram[SNES_DMA_BASE + dma + 0xa] = 0;
		}
		dma += 0x10;
		mask <<= 1;
	}
}


/* should we treat this as nvram in MAME? */
static DIRECT_UPDATE_HANDLER(spc_direct)
{
	direct->raw = direct->decrypted = spc_ram;
	return ~0;
}

static DIRECT_UPDATE_HANDLER(snes_direct)
{
	direct->raw = direct->decrypted = snes_ram;
	return ~0;
}

MACHINE_START( snes )
{
	snes_vram = auto_alloc_array(machine, UINT8, SNES_VRAM_SIZE);
	snes_cgram = auto_alloc_array(machine, UINT16, SNES_CGRAM_SIZE/2);
	snes_oam = auto_alloc_array(machine, UINT16, SNES_OAM_SIZE/2);
	memory_set_direct_update_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), snes_direct);
	memory_set_direct_update_handler(cputag_get_address_space(machine, "soundcpu", ADDRESS_SPACE_PROGRAM), spc_direct);

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

	snes_htmult = 1;
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


void snes_hdma(const address_space *space)
{
	UINT8 mask = 1, dma = 0, i, contmode;
	UINT16 bbus;
	UINT32 abus;

	/* Assume priority of the 8 DMA channels is 0-7 */
	for (i = 0; i < 8; i++)
	{
		if (snes_hdma_chnl & mask)
		{
			/* Check if we need to read a new line from the table */
			if (!(snes_ram[SNES_DMA_BASE + dma + 0xa] & 0x7f))
			{
				abus = (snes_ram[SNES_DMA_BASE + dma + 4] << 16) + (snes_ram[SNES_DMA_BASE + dma + 9] << 8) + snes_ram[SNES_DMA_BASE + dma + 8];

				/* Get the number of lines */
				snes_ram[SNES_DMA_BASE + dma + 0xa] = memory_read_byte(space, abus);
				if (!snes_ram[SNES_DMA_BASE + dma + 0xa])
				{
					/* No more lines so clear HDMA */
					snes_hdma_chnl &= ~mask;
					continue;
				}
				abus++;
				snes_ram[SNES_DMA_BASE + dma + 8] = abus & 0xff;
				snes_ram[SNES_DMA_BASE + dma + 9] = (abus >> 8) & 0xff;
				if (snes_ram[SNES_DMA_BASE + dma] & 0x40)
				{
					snes_ram[SNES_DMA_BASE + dma + 5] = memory_read_byte(space, abus++);
					snes_ram[SNES_DMA_BASE + dma + 6] = memory_read_byte(space, abus++);
					snes_ram[SNES_DMA_BASE + dma + 8] = abus & 0xff;
					snes_ram[SNES_DMA_BASE + dma + 9] = (abus >> 8) & 0xff;
				}
			}

			contmode = (--snes_ram[SNES_DMA_BASE + dma + 0xa]) & 0x80;

			/* Transfer addresses */
			if (snes_ram[SNES_DMA_BASE + dma] & 0x40)	/* Indirect */
				abus = (snes_ram[SNES_DMA_BASE + dma + 7] << 16) + (snes_ram[SNES_DMA_BASE + dma + 6] << 8) + snes_ram[SNES_DMA_BASE + dma + 5];
			else									/* Absolute */
				abus = (snes_ram[SNES_DMA_BASE + dma + 4] << 16) + (snes_ram[SNES_DMA_BASE + dma + 9] << 8) + snes_ram[SNES_DMA_BASE + dma + 8];
			bbus = 0x2100 + snes_ram[SNES_DMA_BASE + dma + 1];

#ifdef SNES_DBG_HDMA
			mame_printf_debug( "HDMA-Ch: %d(%s) abus: %X bbus: %X type: %d(%X %X)\n", i, snes_ram[SNES_DMA_BASE + dma] & 0x40 ? "Indirect" : "Absolute", abus, bbus, snes_ram[SNES_DMA_BASE + dma] & 0x7, snes_ram[SNES_DMA_BASE + dma + 8],snes_ram[SNES_DMA_BASE + dma + 9] );
#endif

			switch (snes_ram[SNES_DMA_BASE + dma] & 0x7)
			{
				case 0:		/* 1 address */
				{
					memory_write_byte(space, bbus, memory_read_byte(space, abus++));
				} break;
				case 5:		/* 4 bytes to 2 addresses (l,h,l,h) */
				{
					memory_write_byte(space, bbus, memory_read_byte(space, abus++));
					memory_write_byte(space, bbus + 1, memory_read_byte(space, abus++));
					memory_write_byte(space, bbus, memory_read_byte(space, abus++));
					memory_write_byte(space, bbus + 1, memory_read_byte(space, abus++));
				} break;
				case 1:		/* 2 addresses (l,h) */
				{
					memory_write_byte(space, bbus, memory_read_byte(space, abus++));
					memory_write_byte(space, bbus + 1, memory_read_byte(space, abus++));
				} break;
				case 2:		/* Write twice (l,l) */
				case 6:
				{
					memory_write_byte(space, bbus, memory_read_byte(space, abus++));
					memory_write_byte(space, bbus, memory_read_byte(space, abus++));
				} break;
				case 3:		/* 2 addresses/Write twice (l,l,h,h) */
				case 7:
				{
					memory_write_byte(space, bbus, memory_read_byte(space, abus++));
					memory_write_byte(space, bbus, memory_read_byte(space, abus++));
					memory_write_byte(space, bbus + 1, memory_read_byte(space, abus++));
					memory_write_byte(space, bbus + 1, memory_read_byte(space, abus++));
				} break;
				case 4:		/* 4 addresses (l,h,l,h) */
				{
					memory_write_byte(space, bbus, memory_read_byte(space, abus++));
					memory_write_byte(space, bbus + 1, memory_read_byte(space, abus++));
					memory_write_byte(space, bbus + 2, memory_read_byte(space, abus++));
					memory_write_byte(space, bbus + 3, memory_read_byte(space, abus++));
				} break;
				default:
#ifdef MAME_DEBUG
					mame_printf_debug( "  HDMA of unsupported type: %d\n", snes_ram[SNES_DMA_BASE + dma] & 0x7 );
#endif
					break;
			}

			/* Update address */
			if (contmode)
			{
				if (snes_ram[SNES_DMA_BASE + dma] & 0x40)	/* Indirect */
				{
					snes_ram[SNES_DMA_BASE + dma + 5] = abus & 0xff;
					snes_ram[SNES_DMA_BASE + dma + 6] = (abus >> 8) & 0xff;
				}
				else									/* Absolute */
				{
					snes_ram[SNES_DMA_BASE + dma + 8] = abus & 0xff;
					snes_ram[SNES_DMA_BASE + dma + 9] = (abus >> 8) & 0xff;
				}
			}

			if (!(snes_ram[SNES_DMA_BASE + dma + 0xa] & 0x7f))
			{
				if (!(snes_ram[SNES_DMA_BASE + dma] & 0x40))	/* Absolute */
				{
					if (!contmode)
					{
						snes_ram[SNES_DMA_BASE + dma + 8] = abus & 0xff;
						snes_ram[SNES_DMA_BASE + dma + 9] = (abus >> 8) & 0xff;
					}
				}
			}
		}
		dma += 0x10;
		mask <<= 1;
	}
}

void snes_gdma( const address_space *space, UINT8 channels )
{
	UINT8 mask = 1, dma = 0, i;
	INT8 increment;
	UINT16 bbus;
	UINT32 abus, abus_bank, length;

	/* Assume priority of the 8 DMA channels is 0-7 */
	for (i = 0; i < 8; i++)
	{
		if (channels & mask)
		{
			//printf( "Making a transfer on channel %d\n", i );
			/* Find transfer addresses */
			abus = (snes_ram[SNES_DMA_BASE + dma + 3] << 8) + snes_ram[SNES_DMA_BASE + dma + 2];
			abus_bank = (snes_ram[SNES_DMA_BASE + dma + 4] << 16);
			bbus = 0x2100 + snes_ram[SNES_DMA_BASE + dma + 1];

			//printf( "Address: %06x\n", abus | abus_bank );
			/* Auto increment */
			if (snes_ram[SNES_DMA_BASE + dma] & 0x8)
			{
				increment = 0;
			}
			else
			{
				if (snes_ram[SNES_DMA_BASE + dma] & 0x10)
					increment = -1;
				else
					increment = 1;
			}

			/* Number of bytes to transfer */
			length = (snes_ram[SNES_DMA_BASE + dma + 6] << 8) + snes_ram[SNES_DMA_BASE + dma + 5];
			if (!length)
				length = 0x10000;	/* 0x0000 really means 0x10000 */

//          printf( "GDMA-Ch %d: len: %X, abus: %X, bbus: %X, incr: %d, dir: %s, type: %d\n", i, length, abus, bbus, increment, snes_ram[SNES_DMA_BASE + dma] & 0x80 ? "PPU->CPU" : "CPU->PPU", snes_ram[SNES_DMA_BASE + dma] & 0x7 );

#ifdef SNES_DBG_GDMA
			mame_printf_debug( "GDMA-Ch %d: len: %X, abus: %X, bbus: %X, incr: %d, dir: %s, type: %d\n", i, length, abus, bbus, increment, snes_ram[SNES_DMA_BASE + dma] & 0x80 ? "PPU->CPU" : "CPU->PPU", snes_ram[SNES_DMA_BASE + dma] & 0x7 );
#endif

			switch (snes_ram[SNES_DMA_BASE + dma] & 0x7)
			{
				case 0:		/* 1 address */
				case 2:		/* 1 address ?? */
				case 6:
				{
					while (length--)
					{
						if (snes_ram[SNES_DMA_BASE + dma] & 0x80)	/* PPU->CPU */
							memory_write_byte(space, ((abus & 0xffff) | (abus_bank)), memory_read_byte(space, bbus) );
						else									/* CPU->PPU */
							memory_write_byte(space, bbus, memory_read_byte(space, ((abus & 0xffff) | (abus_bank))) );
						abus += increment;
					}
				} break;
				case 1:		/* 2 addresses (l,h) */
				case 5:
				{
					while (length--)
					{
						if (snes_ram[SNES_DMA_BASE + dma] & 0x80)	/* PPU->CPU */
							memory_write_byte(space, ((abus & 0xffff) | (abus_bank)), memory_read_byte(space, bbus) );
						else									/* CPU->PPU */
							memory_write_byte(space, bbus, memory_read_byte(space, ((abus & 0xffff) | (abus_bank))) );
						abus += increment;
						if (!(length--))
							break;
						if (snes_ram[SNES_DMA_BASE + dma] & 0x80)	/* PPU->CPU */
							memory_write_byte(space, ((abus & 0xffff) | (abus_bank)), memory_read_byte(space, bbus + 1) );
						else									/* CPU->PPU */
							memory_write_byte(space, bbus + 1, memory_read_byte(space, ((abus & 0xffff) | (abus_bank))) );
						abus += increment;
					}
				} break;
				case 3:		/* 2 addresses/write twice (l,l,h,h) */
				case 7:
				{
					while (length--)
					{
						if (snes_ram[SNES_DMA_BASE + dma] & 0x80)	/* PPU->CPU */
							memory_write_byte(space, ((abus & 0xffff) | (abus_bank)), memory_read_byte(space, bbus) );
						else									/* CPU->PPU */
							memory_write_byte(space, bbus, memory_read_byte(space, ((abus & 0xffff) | (abus_bank))) );
						abus += increment;
						if (!(length--))
							break;
						if (snes_ram[SNES_DMA_BASE + dma] & 0x80)	/* PPU->CPU */
							memory_write_byte(space, ((abus & 0xffff) | (abus_bank)), memory_read_byte(space, bbus) );
						else									/* CPU->PPU */
							memory_write_byte(space, bbus, memory_read_byte(space, ((abus & 0xffff) | (abus_bank))) );
						abus += increment;
						if (!(length--))
							break;
						if (snes_ram[SNES_DMA_BASE + dma] & 0x80)	/* PPU->CPU */
							memory_write_byte(space, ((abus & 0xffff) | (abus_bank)), memory_read_byte(space, bbus + 1) );
						else									/* CPU->PPU */
							memory_write_byte(space, bbus + 1, memory_read_byte(space, ((abus & 0xffff) | (abus_bank))) );
						abus += increment;
						if (!(length--))
							break;
						if (snes_ram[SNES_DMA_BASE + dma] & 0x80)	/* PPU->CPU */
							memory_write_byte(space, ((abus & 0xffff) | (abus_bank)), memory_read_byte(space, bbus + 1) );
						else									/* CPU->PPU */
							memory_write_byte(space, bbus + 1, memory_read_byte(space, ((abus & 0xffff) | (abus_bank))) );
						abus += increment;
					}
				} break;
				case 4:		/* 4 addresses (l,h,l,h) */
				{
					while (length--)
					{
						if (snes_ram[SNES_DMA_BASE + dma] & 0x80)	/* PPU->CPU */
							memory_write_byte(space, ((abus & 0xffff) | (abus_bank)), memory_read_byte(space, bbus) );
						else									/* CPU->PPU */
							memory_write_byte(space, bbus, memory_read_byte(space, ((abus & 0xffff) | (abus_bank))) );
						abus += increment;
						if (!(length--))
							break;
						if (snes_ram[SNES_DMA_BASE + dma] & 0x80)	/* PPU->CPU */
							memory_write_byte(space, ((abus & 0xffff) | (abus_bank)), memory_read_byte(space, bbus + 1) );
						else									/* CPU->PPU */
							memory_write_byte(space, bbus + 1, memory_read_byte(space, ((abus & 0xffff) | (abus_bank))) );
						abus += increment;
						if (!(length--))
							break;
						if (snes_ram[SNES_DMA_BASE + dma] & 0x80)	/* PPU->CPU */
							memory_write_byte(space, ((abus & 0xffff) | (abus_bank)), memory_read_byte(space, bbus + 2) );
						else									/* CPU->PPU */
							memory_write_byte(space, bbus + 2, memory_read_byte(space, ((abus & 0xffff) | (abus_bank))) );
						abus += increment;
						if (!(length--))
							break;
						if (snes_ram[SNES_DMA_BASE + dma] & 0x80)	/* PPU->CPU */
							memory_write_byte(space, ((abus & 0xffff) | (abus_bank)), memory_read_byte(space, bbus + 3) );
						else									/* CPU->PPU */
							memory_write_byte(space, bbus + 3, memory_read_byte(space, ((abus & 0xffff) | (abus_bank))) );
						abus += increment;
					}
				} break;
				default:
#ifdef MAME_DEBUG
					mame_printf_debug( "  GDMA of unsupported type: %d\n", snes_ram[SNES_DMA_BASE + dma] & 0x7 );
#endif
					break;
			}

			/* We're done so write the new abus back to the registers */
			snes_w_io(space, SNES_DMA_BASE + dma + 2, abus & 0xff);
			snes_w_io(space, SNES_DMA_BASE + dma + 3, (abus >> 8) & 0xff);
			snes_w_io(space, SNES_DMA_BASE + dma + 5, 0);
			snes_w_io(space, SNES_DMA_BASE + dma + 6, 0);
			//snes_ram[SNES_DMA_BASE + dma + 2] = abus & 0xff;
			//snes_ram[SNES_DMA_BASE + dma + 3] = (abus >> 8) & 0xff;
			//snes_ram[SNES_DMA_BASE + dma + 5] = 0;
			//snes_ram[SNES_DMA_BASE + dma + 6] = 0;
		}
		dma += 0x10;
		mask <<= 1;
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
