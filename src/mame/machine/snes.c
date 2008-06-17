/***************************************************************************

  snes.c

  Machine file to handle emulation of the Nintendo Super NES

  R. Belmont
  Anthony Kruize
  Based on the original code by Lee Hammerton (aka Savoury Snax)
  Thanks to Anomie for invaluable technical information.

***************************************************************************/
#define __MACHINE_SNES_C

#include "driver.h"
#include "includes/snes.h"
#include "cpu/g65816/g65816.h"
#ifdef MESS
#include "image.h"
#endif

// add-on chip emulators
#include "machine/snesdsp1.c"

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
static UINT8 ppu_last_scroll;	/* as per Theme Park, this is shared by all scroll regs */
static UINT8 snes_hdma_chnl;	/* channels enabled for HDMA */
static UINT8 joy1l, joy1h, joy2l, joy2h, joy3l, joy3h, joy4l, joy4h;
static UINT8 read_ophct = 0, read_opvct = 0;
static emu_timer *snes_scanline_timer;
static emu_timer *snes_hblank_timer;
static emu_timer *snes_nmi_timer;
static emu_timer *snes_hirq_timer;
static UINT16 hblank_offset;
static UINT16 snes_htmult;	/* in 512 wide, we run HTOTAL double and halve it on latching */
static UINT8 has_dsp1;

// full graphic variables
static UINT16 vram_fgr_high, vram_fgr_increment, vram_fgr_count, vram_fgr_mask, vram_fgr_shift, vram_read_buffer;
static const UINT16 vram_fgr_inctab[4] = { 1, 32, 128, 128 };
static const UINT16 vram_fgr_inccnts[4] = { 0, 32, 64, 128 };
static const UINT16 vram_fgr_shiftab[4] = { 0, 5, 6, 7 };

struct snes_cart_info snes_cart = { SNES_MODE_20, 0x40000, 0x40000 };

static struct
{
	UINT8 low;
	UINT8 high;
	UINT32 value;
	UINT8 oldrol;
} joypad[4];

// utility function - latches the H/V counters.  Used by IRQ, writes to WRIO, etc.
static void snes_latch_counters(running_machine *machine)
{
	snes_ppu.beam.current_horz = video_screen_get_hpos(machine->primary_screen) / snes_htmult;
	snes_ppu.beam.latch_vert = video_screen_get_vpos(machine->primary_screen);
	snes_ppu.beam.latch_horz = snes_ppu.beam.current_horz;
	snes_ram[STAT78] |= 0x40;	// indicate we latched
	read_ophct = read_opvct = 0;	// clear read flags

//  printf("latched @ H %d V %d\n", snes_ppu.beam.latch_horz, snes_ppu.beam.latch_vert);
}

static TIMER_CALLBACK( snes_nmi_tick )
{
	// pull NMI
	cpunum_set_input_line(machine, 0, G65816_LINE_NMI, HOLD_LINE );

	// don't happen again
	timer_adjust_oneshot(snes_nmi_timer, attotime_never, 0);
}

static void snes_hirq_tick(running_machine *machine)
{
	// latch the counters and pull IRQ
	// (don't need to switch to the 65816 context, we don't do anything dependant on it)
	snes_latch_counters(machine);
	snes_ram[TIMEUP] = 0x80;	/* Indicate that irq occured */
	cpunum_set_input_line(machine, 0, G65816_LINE_IRQ, HOLD_LINE );

	// don't happen again
	timer_adjust_oneshot(snes_hirq_timer, attotime_never, 0);
}

static TIMER_CALLBACK( snes_hirq_tick_callback )
{
	snes_hirq_tick(machine);
}

static TIMER_CALLBACK( snes_scanline_tick )
{
	// make sure we're in the 65816's context since we're messing with the OAM and stuff
	cpuintrf_push_context(0);

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
			cpunum_set_input_line(machine, 0, G65816_LINE_IRQ, HOLD_LINE );
		}
	}
	/* Horizontal IRQ timer */
	if( snes_ram[NMITIMEN] & 0x10 )
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
	if( snes_ppu.beam.current_vert == snes_ppu.beam.last_visible_line )
	{
		program_write_byte(OAMADDL, snes_ppu.oam.address_low ); /* Reset oam address */
		program_write_byte(OAMADDH, snes_ppu.oam.address_high );
		snes_ram[HVBJOY] |= 0x81;		/* Set vblank bit to on & indicate controllers being read */
		snes_ram[RDNMI] |= 0x80;		/* Set NMI occured bit */

		if( snes_ram[NMITIMEN] & 0x80 )	/* NMI only signaled if this bit set */
		{
			// NMI goes off about 12 cycles after this (otherwise Chrono Trigger, NFL QB Club, etc. lock up)
			timer_adjust_oneshot(snes_nmi_timer, ATTOTIME_IN_CYCLES(12, 0), 0);
		}
	}

	// hdma reset happens at scanline 0, H=~6
	if (snes_ppu.beam.current_vert == 0)
	{
		snes_hdma_init();
	}

	/* three lines after start of vblank we update the controllers (value from snes9x) */
	if (snes_ppu.beam.current_vert == snes_ppu.beam.last_visible_line + 3)
	{
		int i;

		joypad[0].low  = input_port_read(machine, "PAD1L");
		joypad[0].high = input_port_read(machine, "PAD1H");
		joypad[1].low  = input_port_read(machine, "PAD2L");
		joypad[1].high = input_port_read(machine, "PAD2H");
		joypad[2].low  = input_port_read(machine, "PAD3L");
		joypad[2].high = input_port_read(machine, "PAD3H");
		joypad[3].low  = input_port_read(machine, "PAD4L");
		joypad[3].high = input_port_read(machine, "PAD4H");

		// avoid sending signals that could crash games
		for (i = 0; i < 4; i++)
		{
			// if left, no right
			if (joypad[i].high & 2)	joypad[i].high &= ~1;
			// if up, no down
			if (joypad[i].high & 8)	joypad[i].high &= ~4;
		}

		// is automatic reading on?
		if (snes_ram[NMITIMEN] & 1)
		{
			joy1l = joypad[0].low;
			joy1h = joypad[0].high;
			joy2l = joypad[1].low;
			joy2h = joypad[1].high;
			joy3l = joypad[2].low;
			joy3h = joypad[2].high;
			joy4l = joypad[3].low;
			joy4h = joypad[3].high;

			// make sure oldrol starts returning all 1s because the auto-read reads it :-)
			joypad[0].oldrol = 16;
			joypad[1].oldrol = 16;
		}

		snes_ram[HVBJOY] &= 0xfe;		/* Clear busy bit */
	}

	if( snes_ppu.beam.current_vert == 0 )
	{	/* VBlank is over, time for a new frame */
		snes_ram[HVBJOY] &= 0x7f;		/* Clear vblank bit */
		snes_ram[RDNMI]  &= 0x7f;		/* Clear nmi occured bit */
		snes_ram[STAT77] &= 0x3f;		/* Clear Time Over and Range Over bits */
		snes_ram[STAT78] ^= 0x80;		/* Toggle field flag */

		cpunum_set_input_line(machine, 0, G65816_LINE_NMI, CLEAR_LINE );
	}

	cpuintrf_pop_context();

	timer_adjust_oneshot(snes_scanline_timer, attotime_never, 0);
	timer_adjust_oneshot(snes_hblank_timer, video_screen_get_time_until_pos(machine->primary_screen, snes_ppu.beam.current_vert, hblank_offset*snes_htmult), 0);
}

/* This is called at the start of hblank *before* the scanline indicated in current_vert! */
static TIMER_CALLBACK( snes_hblank_tick )
{
	int nextscan;

	snes_ppu.beam.current_vert = video_screen_get_vpos(machine->primary_screen);

	/* make sure we halt */
	timer_adjust_oneshot(snes_hblank_timer, attotime_never, 0);

	// we must guarantee the 65816's context for HDMA to work
  	cpuintrf_push_context(0);

	/* draw a scanline */
	if (snes_ppu.beam.current_vert <= snes_ppu.beam.last_visible_line)
	{
		if (video_screen_get_vpos(machine->primary_screen) > 0)
		{
			/* Do HDMA */
			if( snes_ram[HDMAEN] )
				snes_hdma();

			video_screen_update_partial(machine->primary_screen, snes_ppu.beam.current_vert-1);
		}
	}

	cpuintrf_pop_context();

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

static void snes_init_ram(running_machine *machine)
{
	int i;

	/* Init DSP1 */
	DSP1_reset();

	/* Init VRAM */
	memset( snes_vram, 0, SNES_VRAM_SIZE );

	/* Init Colour RAM */
	memset( (UINT8 *)snes_cgram, 0, SNES_CGRAM_SIZE);

	/* Init oam RAM */
	memset( snes_oam, 0xff, SNES_OAM_SIZE );

	/* Init work RAM - 0x55 isn't exactly right but it's close */
	/* make sure it happens to the 65816 (CPU 0) */
	cpuintrf_push_context(0);
	for (i = 0; i < (128*1024); i++)
	{
		program_write_byte(0x7e0000 + i, 0x55);
	}
	cpuintrf_pop_context();

	/* Inititialize registers/variables */
	snes_ppu.update_windows = 1;
	snes_ppu.beam.latch_vert = 0;
	snes_ppu.beam.latch_horz = 0;
	snes_ppu.beam.current_vert = 0;
	snes_ppu.beam.current_horz = 0;
	snes_ppu.beam.last_visible_line = 240;
	snes_ppu.mode = 0;
	cgram_address = 0;
	vram_read_offset = 2;
	joy1l = joy1h = joy2l = joy2h = joy3l = joy3h = 0;

	// set up some known register power-up defaults
	snes_ram[WRIO] = 0xff;
	snes_ram[VMAIN] = 0x80;

	/* init timers and stop them */
	snes_scanline_timer = timer_alloc(snes_scanline_tick, NULL);
	timer_adjust_oneshot(snes_scanline_timer, attotime_never, 0);
	snes_hblank_timer = timer_alloc(snes_hblank_tick, NULL);
	timer_adjust_oneshot(snes_hblank_timer, attotime_never, 0);
	snes_nmi_timer = timer_alloc(snes_nmi_tick, NULL);
	timer_adjust_oneshot(snes_nmi_timer, attotime_never, 0);
	snes_hirq_timer = timer_alloc(snes_hirq_tick_callback, NULL);
	timer_adjust_oneshot(snes_hirq_timer, attotime_never, 0);

	// SNES hcounter has a 0-339 range.  hblank starts at counter 260.
	// clayfighter sets an HIRQ at 260, apparently it wants it to be before hdma kicks off, so we'll delay 2 pixels.
	hblank_offset = 268;
	timer_adjust_oneshot(snes_hblank_timer, video_screen_get_time_until_pos(machine->primary_screen, ((snes_ram[STAT78] & 0x10) == SNES_NTSC) ? SNES_VTOTAL_NTSC-1 : SNES_VTOTAL_PAL-1, hblank_offset), 0);

	// check if DSP1 is present (maybe not 100%?)
	has_dsp1 = ((snes_r_bank1(machine,0xffd6) >= 3) && (snes_r_bank1(machine,0xffd6) <= 5)) ? 1 : 0;

	// init frame counter so first line is 0
	if( ATTOSECONDS_TO_HZ(video_screen_get_frame_period(machine->primary_screen).attoseconds) >= 59 )
	{
		snes_ppu.beam.current_vert = SNES_VTOTAL_NTSC;
	}
	else
	{
		snes_ppu.beam.current_vert = SNES_VTOTAL_PAL;
	}
}

/* should we treat this as nvram in MAME? */
static OPBASE_HANDLER(spc_opbase)
{
	opbase->rom = opbase->ram = spc_ram;
	return ~0;
}

static OPBASE_HANDLER(snes_opbase)
{
	opbase->rom = opbase->ram = snes_ram;
	return ~0;
}

MACHINE_START( snes )
{
	snes_vram = auto_malloc(SNES_VRAM_SIZE);
	snes_cgram = auto_malloc(SNES_CGRAM_SIZE);
	snes_oam = auto_malloc(SNES_OAM_SIZE);
	memory_set_opbase_handler(0, snes_opbase);
	memory_set_opbase_handler(1, spc_opbase);

	// power-on sets these registers like this
	snes_ram[WRIO] = 0xff;
	snes_ram[WRMPYA] = 0xff;
	snes_ram[WRDIVL] = 0xff;
	snes_ram[WRDIVH] = 0xff;
}

MACHINE_RESET( snes )
{
	snes_init_ram(machine);

	/* Set STAT78 to NTSC or PAL */
	if( ATTOSECONDS_TO_HZ(video_screen_get_frame_period(machine->primary_screen).attoseconds) >= 59.0f )
		snes_ram[STAT78] = SNES_NTSC;
	else /* if( ATTOSECONDS_TO_HZ(video_screen_get_frame_period(machine->primary_screen).attoseconds) == 50.0f ) */
		snes_ram[STAT78] = SNES_PAL;

	// reset does this to these registers
	snes_ram[NMITIMEN] = 0;
	snes_ram[HTIMEL] = 0xff;
	snes_ram[HTIMEH] = 0x1;
	snes_ram[VTIMEL] = 0xff;
	snes_ram[VTIMEH] = 0x1;

	snes_htmult = 1;
}

/* Handle reading of Mode 20 SRAM */
/* 0x700000 - 0x77ffff */
READ8_HANDLER( snes_r_sram )
{
	UINT8 value = 0xff;
	int mask;

	// limit SRAM size to what's actually present
	mask = (snes_cart.sram * 1024) - 1;
	offset &= mask;

	if( snes_cart.sram > 0 )
	{
		value = snes_ram[0x700000 + offset];
	}

	return value;
}

WRITE8_HANDLER( snes_w_sram )
{
	int mask;

	// limit SRAM size to what's actually present
	mask = (snes_cart.sram * 1024) - 1;
	offset &= mask;

	if( snes_cart.sram > 0 )
	{
		snes_ram[0x700000 + offset] = data;
	}
}

/* 0x000000 - 0x2fffff */
READ8_HANDLER( snes_r_bank1 )
{
	UINT16 address = offset & 0xffff;

	if ((snes_cart.mode == SNES_MODE_20) && (has_dsp1))
	{
		if ((address >= 0x8000) && (offset >= 0x200000))
		{
			if (address >= 0xc000)
				return DSP1_getSr();
			else
				return DSP1_getDr();
		}
	}

	if( address <= 0x1fff )								/* Mirror of Low RAM */
	{
		return program_read_byte(0x7e0000 + address );
	}
	else if( address >= 0x2000 && address <= 0x5fff )	/* I/O */
	{
		return snes_r_io( machine, address );
	}
	else if( address >= 0x6000 && address <= 0x7fff )	/* Reserved */
	{
		if( snes_cart.mode == SNES_MODE_20 )
		{
			return 0xff;
		}
		else
		{
			if (address >= 0x7000)
				return DSP1_getSr();
			else
				return DSP1_getDr();
		}
	}
	else
	{
		if( snes_cart.mode == SNES_MODE_20 )
		{
			return snes_ram[offset];
		}
		else	/* MODE_21 */
		{
			return snes_ram[0xc00000 + offset];
		}
	}

	return 0xff;
}

/* 0x300000 - 0x3fffff */
READ8_HANDLER( snes_r_bank2 )
{
	UINT16 address = offset & 0xffff;

	if ((snes_cart.mode == SNES_MODE_20) && (has_dsp1))
	{
		if (address >= 0x8000)
		{
			if (address >= 0xc000)
				return DSP1_getSr();
			else
				return DSP1_getDr();
		}
	}

	if( address <= 0x1fff )								/* Mirror of Low RAM */
		return program_read_byte(0x7e0000 + address );
	else if( address >= 0x2000 && address <= 0x5fff )	/* I/O */
		return snes_r_io( machine, address );
	else if( address >= 0x6000 && address <= 0x7fff )
	{
		if( snes_cart.mode == SNES_MODE_20 )
		{
			return 0xff;						/* Reserved */
		}
		else	/* MODE_21 */
		{
			int mask;

			offset -= 0x6000;

			// limit SRAM size to what's actually present
			mask = (snes_cart.sram * 1024) - 1;
			offset &= mask;
			return snes_ram[0x306000 + offset];	/* sram */
		}
	}
	else
	{
		if( snes_cart.mode == SNES_MODE_20 )
			return snes_ram[0x300000 + offset];
		else	/* MODE_21 */
			return snes_ram[0xf00000 + offset];
	}

	return 0xff;
}

/* 0x400000 - 0x5fffff */
READ8_HANDLER( snes_r_bank3 )
{
	UINT16 address = offset & 0xffff;

	if( snes_cart.mode == SNES_MODE_20 )
	{
		if( address <= 0x7fff )
			return 0xff;		/* Reserved */
		else
			return snes_ram[0x400000 + offset];
	}
	else	/* MODE_21 */
	{
		return snes_ram[0x400000 + offset];
	}

	return 0xff;
}

/* 0x600000 - 0x6fffff */
READ8_HANDLER( snes_r_bank6 )
{
	UINT16 address = offset & 0xffff;

	if (address < 0x8000)
	{
		if (address >= 0x4000)
			return DSP1_getSr();
		else
			return DSP1_getDr();
	}

	return 0xff;
}

/* 0x800000 - 0xffffff */
READ8_HANDLER( snes_r_bank4 )
{
	if( snes_cart.mode == SNES_MODE_20 )
	{
		if( offset <= 0x5fffff )
			return program_read_byte(offset );
		else
			return 0xff;
	}
	else	/* MODE_21 */
	{
		if( offset <= 0x3fffff )
			return program_read_byte(offset );
		else
			return snes_ram[offset + 0x800000];
	}

	return 0xff;
}

/* 0x000000 - 0x2fffff */
WRITE8_HANDLER( snes_w_bank1 )
{
	UINT16 address = offset & 0xffff;

	if ((address >= 0x8000) && (offset >= 0x200000))
	{
		DSP1_setDr(data);
		return;
	}

	if( address <= 0x1fff )								/* Mirror of Low RAM */
		program_write_byte(0x7e0000 + address, data );
	else if( address >= 0x2000 && address <= 0x5fff )	/* I/O */
		snes_w_io( machine, address, data );
	else if( address >= 0x6000 && address <= 0x7fff )	/* Reserved */
		if( snes_cart.mode == SNES_MODE_20 )
		logerror( "Attempt to write to reserved address: %X\n", offset );
	else
		{
			DSP1_setDr(data);
			return;
		}
	else
		logerror( "Attempt to write to ROM address: %X\n", offset );
}

/* 0x300000 - 0x3fffff */
WRITE8_HANDLER( snes_w_bank2 )
{
	UINT16 address = offset & 0xffff;

	if (address >= 0x8000)
	{
		DSP1_setDr(data);
		return;
	}

	if( address <= 0x1fff )								/* Mirror of Low RAM */
		program_write_byte(0x7e0000 + address, data );
	else if( address >= 0x2000 && address <= 0x5fff )	/* I/O */
		snes_w_io( machine, address, data );
	else if( address >= 0x6000 && address <= 0x7fff )
	{
		if( snes_cart.mode == SNES_MODE_20 )			/* Reserved */
			logerror( "Attempt to write to reserved address: %X\n", offset );
		else /* MODE_21 */
		{
			int mask;

			offset -= 0x6000;

			// limit SRAM size to what's actually present
			mask = (snes_cart.sram * 1024) - 1;
			offset &= mask;
			snes_ram[0x306000 + offset] = data;  /* sram */
		}
	}
	else
		logerror( "Attempt to write to ROM address: %X\n", offset );
}

/* 0x600000 - 0x6fffff */
WRITE8_HANDLER( snes_w_bank6 )
{
	UINT16 address = offset & 0xffff;

	if (address < 0x8000)
	{
		DSP1_setDr(data);
		return;
	}
}

/* 0x800000 - 0xffffff */
WRITE8_HANDLER( snes_w_bank4 )
{
	if( snes_cart.mode == SNES_MODE_20 )
	{
		if( offset <= 0x2fffff )
			snes_w_bank1( machine, offset, data );
		else if( offset >= 0x300000 && offset <= 0x3fffff )
			snes_w_bank2( machine, offset - 0x300000, data );
	}
	else /* MODE_21 */
	{
		if( offset <= 0x2fffff )
			snes_w_bank1( machine, offset, data );
		else if( offset >= 0x300000 && offset <= 0x3fffff )
			snes_w_bank2( machine, offset - 0x300000, data );
		else
			logerror( "Attempt to write to ROM address: %X\n", offset );
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
	UINT8 value = 0;

	// APU is mirrored from 2140 to 217f
	if (offset >= APU00 && offset < WMDATA)
	{
		return spc_port_out[offset & 0x3];
	}

	/* offset is from 0x000000 */
	switch( offset )
	{
		/* hacks for SimCity 2000 to boot - I presume openbus emulation will fix */
		case 0x221a:
		case 0x231c:
		case 0x241e:
			return 1;
		case 0x2017:
		case 0x221b:
		case 0x231d:
		case 0x241f:
			return 0;
		case 0x2016:
			return 2;

		case OAMADDL:
		case OAMADDH:
		case VMADDL:
		case VMADDH:
		case VMDATAL:
		case VMDATAH:
		case CGADD:
		case CGDATA:
			return snes_ram[offset];
		case MPYL:		/* Multiplication result (low) */
		case MPYM:		/* Multiplication result (mid) */
		case MPYH:		/* Multiplication result (high) */
			{
				/* Perform 16bit * 8bit multiply */
				INT32 c = snes_ppu.mode7.matrix_a * (snes_ppu.mode7.matrix_b >> 8);
				snes_ram[MPYL] = c & 0xff;
				snes_ram[MPYM] = (c >> 8) & 0xff;
				snes_ram[MPYH] = (c >> 16) & 0xff;
				return snes_ram[offset];
			}
		case SLHV:		/* Software latch for H/V counter */
			snes_latch_counters(machine);
			return 0x0;		/* Return value is meaningless */
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

				value = (snes_oam[oam_addr] >> (snes_ram[OAMDATA] << 3)) & 0xff;
				snes_ram[OAMDATA] = (snes_ram[OAMDATA] + 1) % 2;
				if( snes_ram[OAMDATA] == 0 )
				{
					snes_ppu.oam.address++;
					snes_ppu.oam.address_low = snes_ram[OAMADDL] = snes_ppu.oam.address & 0xff;
					snes_ppu.oam.address_high = snes_ram[OAMADDH] = (snes_ppu.oam.address >> 8) & 0x1;
				}
				return value;
			}
		case RVMDATAL:	/* Read data from VRAM (low) */
			{
				UINT32 addr = (snes_ram[VMADDH] << 8) | snes_ram[VMADDL];

				value = vram_read_buffer & 0xff;

				if (!vram_fgr_high)
				{
					if (vram_fgr_count)
					{
						UINT32 rem = addr & vram_fgr_mask;
						UINT32 faddr = (addr & ~vram_fgr_mask) + (rem >> vram_fgr_shift) +
							       ((rem & (vram_fgr_count - 1)) << 3);

						vram_read_buffer = snes_vram[(faddr<<1)&0xffff] | snes_vram[((faddr<<1)+1) & 0xffff]<<8;
					}
					else
					{
						vram_read_buffer = snes_vram[(addr<<1)&0xffff] | snes_vram[((addr<<1)+1) & 0xffff]<<8;
					}

					addr += vram_fgr_increment;
					snes_ram[VMADDL] = addr&0xff;
					snes_ram[VMADDH] = (addr>>8)&0xff;
				}
			}
			return value;
		case RVMDATAH:	/* Read data from VRAM (high) */
			{
				UINT32 addr = (snes_ram[VMADDH] << 8) | snes_ram[VMADDL];

				value = (vram_read_buffer>>8) & 0xff;

				if (vram_fgr_high)
				{
					if (vram_fgr_count)
					{
						UINT32 rem = addr & vram_fgr_mask;
						UINT32 faddr = (addr & ~vram_fgr_mask) + (rem >> vram_fgr_shift) +
							       ((rem & (vram_fgr_count - 1)) << 3);

						vram_read_buffer = snes_vram[(faddr<<1)&0xffff] | snes_vram[((faddr<<1)+1) & 0xffff]<<8;
					}
					else
					{
						vram_read_buffer = snes_vram[(addr<<1)&0xffff] | snes_vram[((addr<<1)+1) & 0xffff]<<8;
					}

					addr += vram_fgr_increment;
					snes_ram[VMADDL] = addr&0xff;
					snes_ram[VMADDH] = (addr>>8)&0xff;
				}
			}
			return value;
		case RCGDATA:	/* Read data from CGRAM */
				value = ((UINT8 *)snes_cgram)[cgram_address];
				cgram_address = (cgram_address + 1) % (SNES_CGRAM_SIZE - 2);
				return value;
		case OPHCT:		/* Horizontal counter data by ext/soft latch */
			{
				/* FIXME: need to handle STAT78 reset */
				if( read_ophct )
				{
					value = (snes_ppu.beam.latch_horz >> 8) & 0x1;
					read_ophct = 0;
				}
				else
				{
					value = snes_ppu.beam.latch_horz & 0xff;
					read_ophct = 1;
				}
				return value;
			}
		case OPVCT:		/* Vertical counter data by ext/soft latch */
			{
				/* FIXME: need to handle STAT78 reset */
				if( read_opvct )
				{
					value = (snes_ppu.beam.latch_vert >> 8) & 0x1;
					read_opvct = 0;
				}
				else
				{
					value = snes_ppu.beam.latch_vert & 0xff;
					read_opvct = 1;
				}
				return value;
			}
		case STAT77:	/* PPU status flag and version number */
			return snes_ram[offset];
		case STAT78:	/* PPU status flag and version number */
			/* FIXME: need to reset OPHCT and OPVCT */
			value = snes_ram[offset];
			snes_ram[offset] &= ~0x40;	// clear 'latched counters' flag
			return value;
		case WMDATA:	/* Data to read from WRAM */
			{
				UINT32 addr = ((snes_ram[WMADDH] & 0x1) << 16) | (snes_ram[WMADDM] << 8) | snes_ram[WMADDL];

				value = program_read_byte(0x7e0000 + addr++);
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
		case OLDJOY1:	/* Data for old NES controllers */
			{
				if( snes_ram[offset] & 0x1 )
				{
					return 0;
				}
				value = ((joypad[0].low | (joypad[0].high << 8) | 0x10000) >> (15 - (joypad[0].oldrol++ % 16))) & 0x1;
				if( !(joypad[0].oldrol % 17) )
					value = 0x1;
				return value;
			}
		case OLDJOY2:	/* Data for old NES controllers */
			{
				if( snes_ram[OLDJOY1] & 0x1 )
				{
					return 0;
				}
				value = ((joypad[1].low | (joypad[1].high << 8) | 0x10000) >> (15 - (joypad[1].oldrol++ % 16))) & 0x1;
				if( !(joypad[1].oldrol % 17) )
					value = 0x1;
				value |= 0x1c;	// bits 4, 3, and 2 are always set
				return value;
			}
		case HTIMEL:
		case HTIMEH:
		case VTIMEL:
		case VTIMEH:
			return snes_ram[offset];
		case MDMAEN:		/* GDMA channel designation and trigger */
			/* FIXME: Is this really read-only? - Villgust needs to read it */
			return snes_ram[offset];
		case RDNMI:			/* NMI flag by v-blank and version number */
			value = snes_ram[offset];
			snes_ram[offset] &= 0x7f;	/* NMI flag is reset on read */
			return value;
		case TIMEUP:		/* IRQ flag by H/V count timer */
			value = snes_ram[offset];
			snes_ram[offset] = 0;	/* Register is reset on read */
			return value;
		case HVBJOY:		/* H/V blank and joypad controller enable */
			// electronics test says hcounter 272 is start of hblank, which is beampos 363
//          if (video_screen_get_hpos(machine->primary_screen) >= 363) snes_ram[offset] |= 0x40;
//              else snes_ram[offset] &= ~0x40;
			return snes_ram[offset];
		case RDIO:			/* Programmable I/O port - echos back what's written to WRIO */
			return snes_ram[WRIO];
			break;
		case RDDIVL:		/* Quotient of divide result (low) */
		case RDDIVH:		/* Quotient of divide result (high) */
		case RDMPYL:		/* Product/Remainder of mult/div result (low) */
		case RDMPYH:		/* Product/Remainder of mult/div result (high) */
			return snes_ram[offset];
		case JOY1L:			/* Joypad 1 status register (low) */
			return joypad[0].low;
		case JOY1H:			/* Joypad 1 status register (high) */
			return joypad[0].high;
		case JOY2L:			/* Joypad 2 status register (low) */
			return joypad[1].low;
		case JOY2H:			/* Joypad 2 status register (high) */
			return joypad[1].high;
		case JOY3L:			/* Joypad 3 status register (low) */
			return joypad[2].low;
		case JOY3H:			/* Joypad 3 status register (high) */
			return joypad[2].high;
		case JOY4L:			/* Joypad 4 status register (low) */
			return joypad[3].low;
		case JOY4H:			/* Joypad 4 status register (high) */
			return joypad[3].high;
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
			return snes_ram[offset];

#ifndef MESS
		case 0x4100:		/* NSS Dip-Switches */
#ifdef MAME_DEBUG
			return input_port_read_safe(machine, "DEBUG1", 0);
#else
			return input_port_read(machine, "DSW");
#endif	/* MAME_DEBUG */
//      case 0x4101: //PC: a104 - a10e - a12a   //only nss_actr
//      case 0x420c: //PC: 9c7d - 8fab          //only nss_ssoc

		default:
			mame_printf_debug("snes_r: offset = %x pc = %x\n",offset,activecpu_get_pc());
#endif	/* MESS */

	}

	/* Unsupported reads return 0xff */
	return 0xff;
}

/*
 * DW   - Double write : address is written twice to set a 16bit value.
 * low  - This is the low byte of a 16 or 24 bit value
 * mid  - This is the middle byte of a 24 bit value
 * high - This is the high byte of a 16 or 24 bit value
 */
WRITE8_HANDLER( snes_w_io )
{
	// APU is mirrored from 2140 to 217f
	if (offset >= APU00 && offset < WMDATA)
	{
//          printf("816: %02x to APU @ %d\n", data, offset&3);
	     	spc_port_in[offset & 0x3] = data;
		cpu_boost_interleave(attotime_zero, ATTOTIME_IN_USEC(20));
		return;
	}

	/* offset is from 0x000000 */
	switch( offset )
	{
		case INIDISP:	/* Initial settings for screen */
			break;
		case OBSEL:		/* Object size and data area designation */
			snes_ppu.layer[4].data = ((data & 0x3) * 0x2000) << 1;
			snes_ppu.oam.name_select = (((data & 0x18)>>3) * 0x1000) << 1;
			/* Determine object size */
			switch( (data & 0xe0) >> 5 )
			{
				case 0:			/* 8 & 16 */
					snes_ppu.oam.size[0] = 1;
					snes_ppu.oam.size[1] = 2;
					break;
				case 1:			/* 8 & 32 */
					snes_ppu.oam.size[0] = 1;
					snes_ppu.oam.size[1] = 4;
					break;
				case 2:			/* 8 & 64 */
					snes_ppu.oam.size[0] = 1;
					snes_ppu.oam.size[1] = 8;
					break;
				case 3:			/* 16 & 32 */
					snes_ppu.oam.size[0] = 2;
					snes_ppu.oam.size[1] = 4;
					break;
				case 4:			/* 16 & 64 */
					snes_ppu.oam.size[0] = 2;
					snes_ppu.oam.size[1] = 8;
					break;
				case 5:			/* 32 & 64 */
					snes_ppu.oam.size[0] = 4;
					snes_ppu.oam.size[1] = 8;
					break;
				default:
					/* Unknown size so default to 8 & 16 */
					snes_ppu.oam.size[0] = 1;
					snes_ppu.oam.size[1] = 2;
#ifdef SNES_DBG_REG_W
					mame_printf_debug( "Object size unsupported: %d\n", (data & 0xe0) >> 5 );
#endif
			}
			break;
		case OAMADDL:	/* Address for accessing OAM (low) */
			snes_ppu.oam.address_low = data;
			snes_ppu.oam.address = ((snes_ppu.oam.address_high & 0x1) << 8) + data;
			snes_ram[OAMDATA] = 0;
			break;
		case OAMADDH:	/* Address for accessing OAM (high) */
			snes_ppu.oam.address_high = data & 0x1;
			snes_ppu.oam.address = ((data & 0x1) << 8) + snes_ppu.oam.address_low;
			snes_ppu.oam.priority_rotation = (data & 0x80) ? 1 : 0;
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
						snes_oam[oam_addr] |= (data<<8);
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
						snes_oam[oam_addr] = (data<<8) | snes_ppu.oam.write_latch;
					}
				}
				snes_ram[OAMDATA] = (snes_ram[OAMDATA] + 1) % 2;
				if( snes_ram[OAMDATA] == 0 )
				{
					snes_ram[OAMDATA] = 0;
					snes_ppu.oam.address++;
					snes_ppu.oam.address_low = snes_ram[OAMADDL] = snes_ppu.oam.address & 0xff;
					snes_ppu.oam.address_high = snes_ram[OAMADDH] = (snes_ppu.oam.address >> 8) & 0x1;
				}
				return;
			}
		case BGMODE:	/* BG mode and character size settings */
			snes_ppu.mode = data & 0x7;
			{
				rectangle visarea = *video_screen_get_visible_area(machine->primary_screen);

				visarea.min_x = visarea.min_y = 0;
				visarea.max_y = snes_ppu.beam.last_visible_line - 1;

				// fixme: should compensate for SNES_DBG_video
				if( snes_ppu.mode == 5 || snes_ppu.mode == 6 )
				{
					visarea.max_x = (SNES_SCR_WIDTH * 2) - 1;
					snes_htmult = 2;
				}
				else
				{
					visarea.max_x = SNES_SCR_WIDTH - 1;
					snes_htmult = 1;
				}

				if ((snes_ram[STAT78] & 0x10) == SNES_NTSC)
					video_screen_configure(machine->primary_screen, SNES_HTOTAL*snes_htmult, SNES_VTOTAL_NTSC, &visarea, video_screen_get_frame_period(machine->primary_screen).attoseconds);
				else
					video_screen_configure(machine->primary_screen, SNES_HTOTAL*snes_htmult, SNES_VTOTAL_PAL, &visarea, video_screen_get_frame_period(machine->primary_screen).attoseconds);
			}

			snes_ppu.layer[0].tile_size = (data >> 4) & 0x1;
			snes_ppu.layer[1].tile_size = (data >> 5) & 0x1;
			snes_ppu.layer[2].tile_size = (data >> 6) & 0x1;
			snes_ppu.layer[3].tile_size = (data >> 7) & 0x1;
			snes_ppu.update_offsets = 1;
			break;
		case MOSAIC:	/* Size and screen designation for mosaic */
			/* FIXME: We don't support horizontal mosaic yet */
			break;
		case BG1SC:		/* Address for storing SC data BG1 SC size designation */
		case BG2SC:		/* Address for storing SC data BG2 SC size designation  */
		case BG3SC:		/* Address for storing SC data BG3 SC size designation  */
		case BG4SC:		/* Address for storing SC data BG4 SC size designation  */
			snes_ppu.layer[offset - BG1SC].map = (data & 0xfc) << 9;
			snes_ppu.layer[offset - BG1SC].map_size = data & 0x3;
			break;
		case BG12NBA:	/* Address for BG 1 and 2 character data */
			snes_ppu.layer[0].data = (data & 0xf) << 13;
			snes_ppu.layer[1].data = (data & 0xf0) << 9;
			break;
		case BG34NBA:	/* Address for BG 3 and 4 character data */
			snes_ppu.layer[2].data = (data & 0xf) << 13;
			snes_ppu.layer[3].data = (data & 0xf0) << 9;
			break;

		// Anomie says "Current = (Byte<<8) | (Prev&~7) | ((Current>>8)&7);"
		case BG1HOFS:	/* BG1 - horizontal scroll (DW) */
			snes_ppu.layer[0].offset.horizontal = (data<<8) | (ppu_last_scroll & ~7) | ((snes_ppu.layer[0].offset.horizontal>>8) & 7);
			ppu_last_scroll = data;
			snes_ppu.update_offsets = 1;
			return;
		case BG1VOFS:	/* BG1 - vertical scroll (DW) */
			snes_ppu.layer[0].offset.vertical = (data<<8) | (ppu_last_scroll & ~7) | ((snes_ppu.layer[0].offset.vertical>>8) & 7);
			ppu_last_scroll = data;
			snes_ppu.update_offsets = 1;
			return;
		case BG2HOFS:	/* BG2 - horizontal scroll (DW) */
			snes_ppu.layer[1].offset.horizontal = (data<<8) | (ppu_last_scroll & ~7) | ((snes_ppu.layer[1].offset.horizontal>>8) & 7);
			ppu_last_scroll = data;
			snes_ppu.update_offsets = 1;
			return;
		case BG2VOFS:	/* BG2 - vertical scroll (DW) */
			snes_ppu.layer[1].offset.vertical = (data<<8) | (ppu_last_scroll & ~7) | ((snes_ppu.layer[1].offset.vertical>>8) & 7);
			ppu_last_scroll = data;
			snes_ppu.update_offsets = 1;
			return;
		case BG3HOFS:	/* BG3 - horizontal scroll (DW) */
			snes_ppu.layer[2].offset.horizontal = (data<<8) | (ppu_last_scroll & ~7) | ((snes_ppu.layer[2].offset.horizontal>>8) & 7);
			ppu_last_scroll = data;
			snes_ppu.update_offsets = 1;
			return;
		case BG3VOFS:	/* BG3 - vertical scroll (DW) */
			snes_ppu.layer[2].offset.vertical = (data<<8) | (ppu_last_scroll & ~7) | ((snes_ppu.layer[2].offset.vertical>>8) & 7);
			ppu_last_scroll = data;
			snes_ppu.update_offsets = 1;
			return;
		case BG4HOFS:	/* BG4 - horizontal scroll (DW) */
			snes_ppu.layer[3].offset.horizontal = (data<<8) | (ppu_last_scroll & ~7) | ((snes_ppu.layer[3].offset.horizontal>>8) & 7);
			ppu_last_scroll = data;
			snes_ppu.update_offsets = 1;
			return;
		case BG4VOFS:	/* BG4 - vertical scroll (DW) */
			snes_ppu.layer[3].offset.vertical = (data<<8) | (ppu_last_scroll & ~7) | ((snes_ppu.layer[3].offset.vertical>>8) & 7);
			ppu_last_scroll = data;
			snes_ppu.update_offsets = 1;
			return;
		case VMAIN:		/* VRAM address increment value designation */
			vram_fgr_high = (data & 0x80);
			vram_fgr_increment = vram_fgr_inctab[data & 3];

			if (data & 0xc)
			{
				int md = (data & 0xc)>>2;

				vram_fgr_count = vram_fgr_inccnts[md];
				vram_fgr_mask = (vram_fgr_count*8)-1;
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
					UINT32 faddr = (addr & ~vram_fgr_mask) + (rem >> vram_fgr_shift) +
						       ((rem & (vram_fgr_count - 1)) << 3);

					vram_read_buffer = snes_vram[(faddr<<1)&0xffff] | snes_vram[((faddr<<1)+1) & 0xffff]<<8;
				}
				else
				{
					vram_read_buffer = snes_vram[(addr<<1)&0xffff] | snes_vram[((addr<<1)+1) & 0xffff]<<8;
				}

			}
			break;
		case VMDATAL:	/* 2118: Data for VRAM write (low) */
			{
				UINT32 addr = (snes_ram[VMADDH] << 8) | snes_ram[VMADDL];

				if (vram_fgr_count)
				{
					UINT32 rem = addr & vram_fgr_mask;
					UINT32 faddr = (addr & ~vram_fgr_mask) + (rem >> vram_fgr_shift) +
						       ((rem & (vram_fgr_count - 1)) << 3);

					snes_vram[(faddr<<1)&0xffff] = data;
				}
				else
				{
					snes_vram[(addr<<1)&0xffff] = data;
				}

				if (!vram_fgr_high)
				{
					addr += vram_fgr_increment;
					snes_ram[VMADDL] = addr&0xff;
					snes_ram[VMADDH] = (addr>>8)&0xff;
				}
			}
			return;
		case VMDATAH:	/* 2119: Data for VRAM write (high) */
			{
				UINT32 addr = (snes_ram[VMADDH] << 8) | snes_ram[VMADDL];

				if (vram_fgr_count)
				{
					UINT32 rem = addr & vram_fgr_mask;
					UINT32 faddr = (addr & ~vram_fgr_mask) + (rem >> vram_fgr_shift) +
						       ((rem & (vram_fgr_count - 1)) << 3);

					snes_vram[((faddr<<1)+1)&0xffff] = data;
				}
				else
				{
					snes_vram[((addr<<1)+1)&0xffff] = data;
				}

				if (vram_fgr_high)
				{
					addr += vram_fgr_increment;
					snes_ram[VMADDL] = addr&0xff;
					snes_ram[VMADDH] = (addr>>8)&0xff;
				}
			}
			return;
		case M7SEL:		/* Mode 7 initial settings */
			break;
		case M7A:		/* Mode 7 COS angle/x expansion (DW) */
			snes_ppu.mode7.matrix_a = ((snes_ppu.mode7.matrix_a >> 8) & 0xff) + (data << 8);
			break;
		case M7B:		/* Mode 7 SIN angle/ x expansion (DW) */
			snes_ppu.mode7.matrix_b = ((snes_ppu.mode7.matrix_b >> 8) & 0xff) + (data << 8);
			break;
		case M7C:		/* Mode 7 SIN angle/y expansion (DW) */
			snes_ppu.mode7.matrix_c = ((snes_ppu.mode7.matrix_c >> 8) & 0xff) + (data << 8);
			break;
		case M7D:		/* Mode 7 COS angle/y expansion (DW) */
			snes_ppu.mode7.matrix_d = ((snes_ppu.mode7.matrix_d >> 8) & 0xff) + (data << 8);
			break;
		case M7X:		/* Mode 7 x center position (DW) */
			snes_ppu.mode7.origin_x = ((snes_ppu.mode7.origin_x >> 8) & 0xff) + (data << 8);
			break;
		case M7Y:		/* Mode 7 y center position (DW) */
			snes_ppu.mode7.origin_y = ((snes_ppu.mode7.origin_y >> 8) & 0xff) + (data << 8);
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
		case W34SEL:	/* Window mask settings for BG3-4 */
		case WOBJSEL:	/* Window mask settings for objects */
		case WH0:		/* Window 1 left position */
		case WH1:		/* Window 1 right position */
		case WH2:		/* Window 2 left position */
		case WH3:		/* Window 2 right position */
		case WBGLOG:	/* Window mask logic for BG's */
		case WOBJLOG:	/* Window mask logic for objects */
			if( data != snes_ram[offset] )
				snes_ppu.update_windows = 1;
			break;
		case TM:		/* Main screen designation */
		case TS:		/* Subscreen designation */
		case TMW:		/* Window mask for main screen designation */
		case TSW:		/* Window mask for subscreen designation */
			break;
		case CGWSEL:	/* Initial settings for Fixed colour addition or screen addition */
			/* FIXME: We don't support direct select for modes 3 & 4 or subscreen window stuff */
#ifdef SNES_DBG_REG_W
			if( (data & 0x2) != (snes_ram[CGWSEL] & 0x2) )
				mame_printf_debug( "Add/Sub Layer: %s\n", ((data & 0x2) >> 1) ? "Subscreen" : "Fixed colour" );
#endif
			break;
		case CGADSUB:	/* Addition/Subtraction designation for each screen */
			{
				UINT8 sub = (data & 0x80) >> 7;
				snes_ppu.layer[0].blend = (data & 0x1) << sub;
				snes_ppu.layer[1].blend = ((data & 0x2) >> 1) << sub;
				snes_ppu.layer[2].blend = ((data & 0x4) >> 2) << sub;
				snes_ppu.layer[3].blend = ((data & 0x8) >> 3) << sub;
				snes_ppu.layer[4].blend = ((data & 0x10) >> 4) << sub;
			} break;
		case COLDATA:	/* Fixed colour data for fixed colour addition/subtraction */
			{
				/* Store it in the extra space we made in the CGRAM
                 * It doesn't really go there, but it's as good a place as any. */
				UINT8 r,g,b;

				/* Get existing value. */
				r = snes_cgram[FIXED_COLOUR] & 0x1f;
				g = (snes_cgram[FIXED_COLOUR] & 0x3e0) >> 5;
				b = (snes_cgram[FIXED_COLOUR] & 0x7c00) >> 10;
				/* Set new value */
				if( data & 0x20 )
					r = data & 0x1f;
				if( data & 0x40 )
					g = data & 0x1f;
				if( data & 0x80 )
					b = data & 0x1f;
				snes_cgram[FIXED_COLOUR] = (r | (g << 5) | (b << 10));
			} break;
		case SETINI:	/* Screen mode/video select */
			/* FIXME: We only support line count here */
			snes_ppu.beam.last_visible_line = (data & 0x4) ? 240 : 225;
#ifdef SNES_DBG_REG_W
			if( (data & 0x8) != (snes_ram[SETINI] & 0x8) )
				mame_printf_debug( "Pseudo 512 mode: %s\n", (data & 0x8) ? "on" : "off" );
#endif
			break;
		case WMDATA:	/* Data to write to WRAM */
			{
				UINT32 addr = ((snes_ram[WMADDH] & 0x1) << 16) | (snes_ram[WMADDM] << 8) | snes_ram[WMADDL];
				program_write_byte(0x7e0000 + addr++, data );
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
			if( (data & 0x1) && !(snes_ram[offset] & 0x1) )
			{
				joypad[0].oldrol = 0;
				joypad[1].oldrol = 0;
				joypad[2].oldrol = 0;
				joypad[3].oldrol = 0;
			}
			break;
		case NMITIMEN:	/* Flag for v-blank, timer int. and joy read */
		case OLDJOY2:	/* Old NES joystick support */
			break;
		case WRIO:		/* Programmable I/O port - latches H/V counters on a 1->0 transition */
			if (!(snes_ram[WRIO] & 0x80) && (data & 0x80))
			{
				// external latch
				snes_latch_counters(machine);
			}
		case WRMPYA:	/* Multiplier A */
			break;
		case WRMPYB:	/* Multiplier B */
			{
				UINT32 c = snes_ram[WRMPYA] * data;
				snes_ram[RDMPYL] = c & 0xff;
				snes_ram[RDMPYH] = (c >> 8) & 0xff;
			} break;
		case WRDIVL:	/* Dividend (low) */
		case WRDIVH:	/* Dividend (high) */
			break;
		case WRDVDD:	/* Divisor */
			{
				UINT16 value, dividend, remainder;
				dividend = remainder = 0;
				value = (snes_ram[WRDIVH] << 8) + snes_ram[WRDIVL];
				if( data > 0 )
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
			} break;
		case HTIMEL:	/* H-Count timer settings (low)  */
		case HTIMEH:	/* H-Count timer settings (high) */
		case VTIMEL:	/* V-Count timer settings (low)  */
		case VTIMEH:	/* V-Count timer settings (high) */
			break;
		case MDMAEN:	/* GDMA channel designation and trigger */
			snes_gdma( data );
			data = 0;	/* Once DMA is done we need to reset all bits to 0 */
			break;
		case HDMAEN:	/* HDMA channel designation */
			break;
		case MEMSEL:	/* Access cycle designation in memory (2) area */
			/* FIXME: Need to adjust the speed only during access of banks 0x80+
             * Currently we are just increasing it no matter what */
//          cpunum_set_clockscale(machine, 0, (data & 0x1) ? 1.335820896 : 1.0 );
#ifdef SNES_DBG_REG_W
			if( (data & 0x1) != (snes_ram[MEMSEL] & 0x1) )
				mame_printf_debug( "CPU speed: %f Mhz\n", (data & 0x1) ? 3.58 : 2.68 );
#endif
			break;
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

/* This function checks everything is in a valid range and returns how
 * 'valid' this section is as an information block. */
int snes_validate_infoblock( UINT8 *infoblock, UINT16 offset )
{
	INT8 valid = 6;

	/* Check the CRC and inverse CRC */
	if( ((infoblock[offset + 0x1c] + (infoblock[offset + 0x1d] << 8)) |
		(infoblock[offset + 0x1e] + (infoblock[offset + 0x1f] << 8))) != 0xffff )
	{
		valid -= 3;
	}
	/* Check the ROM Size is in a valid range */
	if( infoblock[offset + 0x17] > 13 )
	{
		valid--;
	}
	/* Check the SRAM size */
	if( infoblock[offset + 0x18] > 8 )
	{
		valid--;
	}
	/* Check the Country is in a valid range */
	if( infoblock[offset + 0x19] > 13 )
	{
		valid--;
	}
	/* Check the game version */
	if( infoblock[offset + 0x1b] >= 128 )
	{
		valid--;
	}

	if( valid < 0 )
	{
		valid = 0;
	}

	return valid;
}

#ifndef MESS
/* for mame we use an init, maybe we will need more for the different games */
DRIVER_INIT( snes )
{
	int i;
	UINT16 totalblocks, readblocks;
	UINT8  *rom;

	rom = memory_region( REGION_USER3 );
	snes_ram = auto_malloc(0x1000000);
	memset( snes_ram, 0, 0x1000000 );

	/* all NSS games seem to use MODE 20 */
	snes_cart.mode = SNES_MODE_20;
	snes_cart.sram_max = 0x40000;

	/* Find the number of blocks in this ROM */
	//totalblocks = ((mame_fsize(file) - offset) >> (snes_cart.mode == MODE_20 ? 15 : 16));
	totalblocks = (memory_region_length(REGION_USER3) / 0x8000) - 1;

	/* FIXME: Insert crc check here */

	readblocks = 0;
	{
		/* In mode 20, all blocks are 32kb. There are upto 96 blocks, giving a
         * total of 24mbit(3mb) of ROM.
         * The first 48 blocks are located in banks 0x00 to 0x2f at address
         * 0x8000.  They are mirrored in banks 0x80 to 0xaf.
         * The next 16 blocks are located in banks 0x30 to 0x3f at address
         * 0x8000.  They are mirrored in banks 0xb0 to 0xbf.
         * The final 32 blocks are located in banks 0x40 - 0x5f at address
         * 0x8000.  They are mirrored in banks 0xc0 to 0xdf.
         */
		i = 0;
		while( i < 96 && readblocks <= totalblocks )
		{
			//mame_fread( file, &snes_ram[(i++ * 0x10000) + 0x8000], 0x8000);
			memcpy(&snes_ram[(i * 0x10000) + 0x8000], &rom[i * 0x8000], 0x8000);
			i++;
			readblocks++;
		}
	}

	/* Find the amount of sram */
	snes_cart.sram = snes_r_bank1(machine,0x00ffd8);
	if( snes_cart.sram > 0 )
	{
		snes_cart.sram = ((1 << (snes_cart.sram + 3)) / 8);
		if( snes_cart.sram > snes_cart.sram_max )
			snes_cart.sram = snes_cart.sram_max;
	}
}

DRIVER_INIT( snes_hirom )
{
	int i;
	UINT16 totalblocks, readblocks;
	UINT8  *rom;

	rom = memory_region( REGION_USER3 );
	snes_ram = auto_malloc(0x1000000);
	memset( snes_ram, 0, 0x1000000 );

	snes_cart.mode = SNES_MODE_21;
	snes_cart.sram_max = 0x40000;

	/* Find the number of blocks in this ROM */
	//totalblocks = ((mame_fsize(file) - offset) >> (snes_cart.mode == MODE_20 ? 15 : 16));
	totalblocks = (memory_region_length(REGION_USER3) / 0x10000) - 1;

	/* FIXME: Insert crc check here */

	readblocks = 0;
	{
		i = 0;
		while( i < 64 && readblocks <= totalblocks )
		{
			memcpy( &snes_ram[0xc00000 + (i * 0x10000)],  &rom[i * 0x10000], 0x10000);
			i++;
			readblocks++;
		}
	}

	/* Find the amount of sram */
	snes_cart.sram = snes_r_bank1(machine,0x00ffd8);
	if( snes_cart.sram > 0 )
	{
		snes_cart.sram = ((1 << (snes_cart.sram + 3)) / 8);
		if( snes_cart.sram > snes_cart.sram_max )
			snes_cart.sram = snes_cart.sram_max;
	}
}
#endif	/* MESS */

void snes_hdma_init()
{
	UINT8 mask = 1, dma = 0, i;

	snes_hdma_chnl = snes_ram[HDMAEN];
	for( i = 0; i < 8; i++ )
	{
		if( snes_ram[HDMAEN] & mask )
		{
			snes_ram[SNES_DMA_BASE + dma + 8] = snes_ram[SNES_DMA_BASE + dma + 2];
			snes_ram[SNES_DMA_BASE + dma + 9] = snes_ram[SNES_DMA_BASE + dma + 3];
			snes_ram[SNES_DMA_BASE + dma + 0xa] = 0;
		}
		dma += 0x10;
		mask <<= 1;
	}
}

void snes_hdma()
{
	UINT8 mask = 1, dma = 0, i, contmode;
	UINT16 bbus;
	UINT32 abus;

	/* Assume priority of the 8 DMA channels is 0-7 */
	for( i = 0; i < 8; i++ )
	{
		if( snes_hdma_chnl & mask )
		{
			/* Check if we need to read a new line from the table */
			if( !(snes_ram[SNES_DMA_BASE + dma + 0xa] & 0x7f ) )
			{
				abus = (snes_ram[SNES_DMA_BASE + dma + 4] << 16) + (snes_ram[SNES_DMA_BASE + dma + 9] << 8) + snes_ram[SNES_DMA_BASE + dma + 8];

				/* Get the number of lines */
				snes_ram[SNES_DMA_BASE + dma + 0xa] = program_read_byte(abus);
				if( !snes_ram[SNES_DMA_BASE + dma + 0xa] )
				{
					/* No more lines so clear HDMA */
					snes_hdma_chnl &= ~mask;
					continue;
				}
				abus++;
				snes_ram[SNES_DMA_BASE + dma + 8] = abus & 0xff;
				snes_ram[SNES_DMA_BASE + dma + 9] = (abus >> 8) & 0xff;
				if( snes_ram[SNES_DMA_BASE + dma] & 0x40 )
				{
					snes_ram[SNES_DMA_BASE + dma + 5] = program_read_byte(abus++);
					snes_ram[SNES_DMA_BASE + dma + 6] = program_read_byte(abus++);
					snes_ram[SNES_DMA_BASE + dma + 8] = abus & 0xff;
					snes_ram[SNES_DMA_BASE + dma + 9] = (abus >> 8) & 0xff;
				}
			}

			contmode = (--snes_ram[SNES_DMA_BASE + dma + 0xa]) & 0x80;

			/* Transfer addresses */
			if( snes_ram[SNES_DMA_BASE + dma] & 0x40 )	/* Indirect */
				abus = (snes_ram[SNES_DMA_BASE + dma + 7] << 16) + (snes_ram[SNES_DMA_BASE + dma + 6] << 8) + snes_ram[SNES_DMA_BASE + dma + 5];
			else									/* Absolute */
				abus = (snes_ram[SNES_DMA_BASE + dma + 4] << 16) + (snes_ram[SNES_DMA_BASE + dma + 9] << 8) + snes_ram[SNES_DMA_BASE + dma + 8];
			bbus = 0x2100 + snes_ram[SNES_DMA_BASE + dma + 1];

#ifdef SNES_DBG_HDMA
			mame_printf_debug( "HDMA-Ch: %d(%s) abus: %X bbus: %X type: %d(%X %X)\n", i, snes_ram[SNES_DMA_BASE + dma] & 0x40 ? "Indirect" : "Absolute", abus, bbus, snes_ram[SNES_DMA_BASE + dma] & 0x7, snes_ram[SNES_DMA_BASE + dma + 8],snes_ram[SNES_DMA_BASE + dma + 9] );
#endif

			switch( snes_ram[SNES_DMA_BASE + dma] & 0x7 )
			{
				case 0:		/* 1 address */
				{
					program_write_byte(bbus, program_read_byte(abus++));
				} break;
				case 5:		/* 4 bytes to 2 addresses (l,h,l,h) */
				{
					program_write_byte(bbus, program_read_byte(abus++));
					program_write_byte(bbus + 1, program_read_byte(abus++));
					program_write_byte(bbus, program_read_byte(abus++));
					program_write_byte(bbus + 1, program_read_byte(abus++));
				} break;
				case 1:		/* 2 addresses (l,h) */
				{
					program_write_byte(bbus, program_read_byte(abus++));
					program_write_byte(bbus + 1, program_read_byte(abus++));
				} break;
				case 2:		/* Write twice (l,l) */
				case 6:
				{
					program_write_byte(bbus, program_read_byte(abus++));
					program_write_byte(bbus, program_read_byte(abus++));
				} break;
				case 3:		/* 2 addresses/Write twice (l,l,h,h) */
				case 7:
				{
					program_write_byte(bbus, program_read_byte(abus++));
					program_write_byte(bbus, program_read_byte(abus++));
					program_write_byte(bbus + 1, program_read_byte(abus++));
					program_write_byte(bbus + 1, program_read_byte(abus++));
				} break;
				case 4:		/* 4 addresses (l,h,l,h) */
				{
					program_write_byte(bbus, program_read_byte(abus++));
					program_write_byte(bbus + 1, program_read_byte(abus++));
					program_write_byte(bbus + 2, program_read_byte(abus++));
					program_write_byte(bbus + 3, program_read_byte(abus++));
				} break;
				default:
#ifdef MAME_DEBUG
					mame_printf_debug( "  HDMA of unsupported type: %d\n", snes_ram[SNES_DMA_BASE + dma] & 0x7 );
#endif
					break;
			}

			/* Update address */
			if( contmode )
			{
				if( snes_ram[SNES_DMA_BASE + dma] & 0x40 )	/* Indirect */
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

			if( !(snes_ram[SNES_DMA_BASE + dma + 0xa] & 0x7f) )
			{
				if( !(snes_ram[SNES_DMA_BASE + dma] & 0x40) )	/* Absolute */
				{
					if( !contmode )
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

void snes_gdma( UINT8 channels )
{
	UINT8 mask = 1, dma = 0, i;
	INT8 increment;
	UINT16 bbus;
	UINT32 abus, length;

	/* Assume priority of the 8 DMA channels is 0-7 */
	for( i = 0; i < 8; i++ )
	{
		if( channels & mask )
		{
			/* Find transfer addresses */
			abus = (snes_ram[SNES_DMA_BASE + dma + 4] << 16) + (snes_ram[SNES_DMA_BASE + dma + 3] << 8) + snes_ram[SNES_DMA_BASE + dma + 2];
			bbus = 0x2100 + snes_ram[SNES_DMA_BASE + dma + 1];

			/* Auto increment */
			if( snes_ram[SNES_DMA_BASE + dma] & 0x8 )
			{
				increment = 0;
			}
			else
			{
				if( snes_ram[SNES_DMA_BASE + dma] & 0x10 )
					increment = -1;
				else
					increment = 1;
			}

			/* Number of bytes to transfer */
			length = (snes_ram[SNES_DMA_BASE + dma + 6] << 8) + snes_ram[SNES_DMA_BASE + dma + 5];
			if( !length )
				length = 0x10000;	/* 0x0000 really means 0x10000 */

#ifdef SNES_DBG_GDMA
			mame_printf_debug( "GDMA-Ch %d: len: %X, abus: %X, bbus: %X, incr: %d, dir: %s, type: %d\n", i, length, abus, bbus, increment, snes_ram[SNES_DMA_BASE + dma] & 0x80 ? "PPU->CPU" : "CPU->PPU", snes_ram[SNES_DMA_BASE + dma] & 0x7 );
#endif

			switch( snes_ram[SNES_DMA_BASE + dma] & 0x7 )
			{
				case 0:		/* 1 address */
				case 2:		/* 1 address ?? */
				case 6:
				{
					while( length-- )
					{
						if( snes_ram[SNES_DMA_BASE + dma] & 0x80 )	/* PPU->CPU */
							program_write_byte(abus, program_read_byte(bbus) );
						else									/* CPU->PPU */
							program_write_byte(bbus, program_read_byte(abus) );
						abus += increment;
					}
				} break;
				case 1:		/* 2 addresses (l,h) */
				case 5:
				{
					while( length-- )
					{
						if( snes_ram[SNES_DMA_BASE + dma] & 0x80 )	/* PPU->CPU */
							program_write_byte(abus, program_read_byte(bbus) );
						else									/* CPU->PPU */
							program_write_byte(bbus, program_read_byte(abus) );
						abus += increment;
						if( !(length--) )
							break;
						if( snes_ram[SNES_DMA_BASE + dma] & 0x80 )	/* PPU->CPU */
							program_write_byte(abus, program_read_byte(bbus + 1) );
						else									/* CPU->PPU */
							program_write_byte(bbus + 1, program_read_byte(abus) );
						abus += increment;
					}
				} break;
				case 3:		/* 2 addresses/write twice (l,l,h,h) */
				case 7:
				{
					while( length-- )
					{
						if( snes_ram[SNES_DMA_BASE + dma] & 0x80 )	/* PPU->CPU */
							program_write_byte(abus, program_read_byte(bbus) );
						else									/* CPU->PPU */
							program_write_byte(bbus, program_read_byte(abus) );
						abus += increment;
						if( !(length--) )
							break;
						if( snes_ram[SNES_DMA_BASE + dma] & 0x80 )	/* PPU->CPU */
							program_write_byte(abus, program_read_byte(bbus) );
						else									/* CPU->PPU */
							program_write_byte(bbus, program_read_byte(abus) );
						abus += increment;
						if( !(length--) )
							break;
						if( snes_ram[SNES_DMA_BASE + dma] & 0x80 )	/* PPU->CPU */
							program_write_byte(abus, program_read_byte(bbus + 1) );
						else									/* CPU->PPU */
							program_write_byte(bbus + 1, program_read_byte(abus) );
						abus += increment;
						if( !(length--) )
							break;
						if( snes_ram[SNES_DMA_BASE + dma] & 0x80 )	/* PPU->CPU */
							program_write_byte(abus, program_read_byte(bbus + 1) );
						else									/* CPU->PPU */
							program_write_byte(bbus + 1, program_read_byte(abus) );
						abus += increment;
					}
				} break;
				case 4:		/* 4 addresses (l,h,l,h) */
				{
					while( length-- )
					{
						if( snes_ram[SNES_DMA_BASE + dma] & 0x80 )	/* PPU->CPU */
							program_write_byte(abus, program_read_byte(bbus) );
						else									/* CPU->PPU */
							program_write_byte(bbus, program_read_byte(abus) );
						abus += increment;
						if( !(length--) )
							break;
						if( snes_ram[SNES_DMA_BASE + dma] & 0x80 )	/* PPU->CPU */
							program_write_byte(abus, program_read_byte(bbus + 1) );
						else									/* CPU->PPU */
							program_write_byte(bbus + 1, program_read_byte(abus) );
						abus += increment;
						if( !(length--) )
							break;
						if( snes_ram[SNES_DMA_BASE + dma] & 0x80 )	/* PPU->CPU */
							program_write_byte(abus, program_read_byte(bbus + 2) );
						else									/* CPU->PPU */
							program_write_byte(bbus + 2, program_read_byte(abus) );
						abus += increment;
						if( !(length--) )
							break;
						if( snes_ram[SNES_DMA_BASE + dma] & 0x80 )	/* PPU->CPU */
							program_write_byte(abus, program_read_byte(bbus + 3) );
						else									/* CPU->PPU */
							program_write_byte(bbus + 3, program_read_byte(abus) );
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
			snes_ram[SNES_DMA_BASE + dma + 2] = abus & 0xff;
			snes_ram[SNES_DMA_BASE + dma + 3] = (abus >> 8) & 0xff;
			snes_ram[SNES_DMA_BASE + dma + 5] = 0;
			snes_ram[SNES_DMA_BASE + dma + 6] = 0;
		}
		dma += 0x10;
		mask <<= 1;
	}
}
