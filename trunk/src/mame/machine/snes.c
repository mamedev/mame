/***************************************************************************

  snes.c

  Machine file to handle emulation of the Nintendo Super NES

  R. Belmont
  Anthony Kruize
  Angelo Salese
  Fabio Priuli
  Harmony
  Based on the original code by Lee Hammerton (aka Savoury Snax)
  Thanks to Anomie for invaluable technical information.
  Thanks to byuu for invaluable technical information.

***************************************************************************/
#define __MACHINE_SNES_C

#include "emu.h"
#include "cpu/superfx/superfx.h"
#include "cpu/g65816/g65816.h"
#include "cpu/upd7725/upd7725.h"
#include "includes/snes.h"
#include "audio/snes_snd.h"


/* -- Globals -- */
UINT8  *snes_ram = NULL;		/* 65816 ram */

static void snes_dma(address_space *space, UINT8 channels);
static void snes_hdma_init(address_space *space);
static void snes_hdma(address_space *space);

static READ8_HANDLER(snes_io_dma_r);
static WRITE8_HANDLER(snes_io_dma_w);

struct snes_cart_info snes_cart;

// DSP accessors
#define dsp_get_sr() state->m_upd7725->snesdsp_read(false)
#define dsp_get_dr() state->m_upd7725->snesdsp_read(true)
#define dsp_set_sr(data) state->m_upd7725->snesdsp_write(false, data)
#define dsp_set_dr(data) state->m_upd7725->snesdsp_write(true, data)

#define st010_get_sr() state->m_upd96050->snesdsp_read(false)
#define st010_get_dr() state->m_upd96050->snesdsp_read(true)
#define st010_set_sr(data) state->m_upd96050->snesdsp_write(false, data)
#define st010_set_dr(data) state->m_upd96050->snesdsp_write(true, data)

// add-on chip emulators
#include "machine/snesobc1.c"
#include "machine/snescx4.c"
#include "machine/snesrtc.c"
#include "machine/snessdd1.c"
#include "machine/snes7110.c"
#include "machine/snesbsx.c"

#define USE_CYCLE_STEAL 1

// ST-010 and ST-011 RAM interface
UINT8 st010_read_ram(snes_state *state, UINT16 addr)
{
	UINT16 temp = state->m_upd96050->dataram_r(addr/2);
	UINT8 res;

	if (addr & 1)
	{
		res = temp>>8;
	}
	else
	{
		res = temp & 0xff;
	}

	return res;
}

void st010_write_ram(snes_state *state, UINT16 addr, UINT8 data)
{
	UINT16 temp = state->m_upd96050->dataram_r(addr/2);

	if (addr & 1)
	{
		temp &= 0xff;
		temp |= data<<8;
	}
	else
	{
		temp &= 0xff00;
		temp |= data;
	}

	state->m_upd96050->dataram_w(addr/2, temp);
}

/*************************************

    Timers

*************************************/

static TIMER_CALLBACK( snes_nmi_tick )
{
	snes_state *state = machine.driver_data<snes_state>();

	// pull NMI
	device_set_input_line(state->m_maincpu, G65816_LINE_NMI, ASSERT_LINE);

	// don't happen again
	state->m_nmi_timer->adjust(attotime::never);
}

static void snes_hirq_tick( running_machine &machine )
{
	snes_state *state = machine.driver_data<snes_state>();

	// latch the counters and pull IRQ
	// (don't need to switch to the 65816 context, we don't do anything dependant on it)
	snes_latch_counters(machine);
	snes_ram[TIMEUP] = 0x80;	/* Indicate that irq occurred */
	device_set_input_line(state->m_maincpu, G65816_LINE_IRQ, ASSERT_LINE);

	// don't happen again
	state->m_hirq_timer->adjust(attotime::never);
}

static TIMER_CALLBACK( snes_hirq_tick_callback )
{
	snes_hirq_tick(machine);
}

static TIMER_CALLBACK( snes_reset_oam_address )
{
	snes_state *state = machine.driver_data<snes_state>();
	// make sure we're in the 65816's context since we're messing with the OAM and stuff
	address_space *space = state->m_maincpu->memory().space(AS_PROGRAM);

	if (!(snes_ppu.screen_disabled)) //Reset OAM address, byuu says it happens at H=10
	{
		space->write_byte(OAMADDL, snes_ppu.oam.saved_address_low); /* Reset oam address */
		space->write_byte(OAMADDH, snes_ppu.oam.saved_address_high);
		snes_ppu.oam.first_sprite = snes_ppu.oam.priority_rotation ? (snes_ppu.oam.address >> 1) & 127 : 0;
	}
}

static TIMER_CALLBACK( snes_reset_hdma )
{
	snes_state *state = machine.driver_data<snes_state>();
	address_space *cpu0space = state->m_maincpu->memory().space(AS_PROGRAM);
	snes_hdma_init(cpu0space);
}

static TIMER_CALLBACK( snes_update_io )
{
	snes_state *state = machine.driver_data<snes_state>();
	address_space *cpu0space = state->m_maincpu->memory().space(AS_PROGRAM);
	state->m_io_read(cpu0space->machine());
	snes_ram[HVBJOY] &= 0xfe;		/* Clear busy bit */

	state->m_io_timer->adjust(attotime::never);
}

static TIMER_CALLBACK( snes_scanline_tick )
{
	snes_state *state = machine.driver_data<snes_state>();

	/* Increase current line - we want to latch on this line during it, not after it */
	snes_ppu.beam.current_vert = machine.primary_screen->vpos();

	// not in hblank
	snes_ram[HVBJOY] &= ~0x40;

	/* Vertical IRQ timer - only if horizontal isn't also enabled! */
	if ((snes_ram[NMITIMEN] & 0x20) && !(snes_ram[NMITIMEN] & 0x10))
	{
		if (snes_ppu.beam.current_vert == state->m_vtime)
		{
			snes_ram[TIMEUP] = 0x80;	/* Indicate that irq occurred */
			// IRQ latches the counters, do it now
			snes_latch_counters(machine);
			device_set_input_line(state->m_maincpu, G65816_LINE_IRQ, ASSERT_LINE );
		}
	}
	/* Horizontal IRQ timer */
	if (snes_ram[NMITIMEN] & 0x10)
	{
		int setirq = 1;
		int pixel = state->m_htime;

		// is the HIRQ on a specific scanline?
		if (snes_ram[NMITIMEN] & 0x20)
		{
			if (snes_ppu.beam.current_vert != state->m_vtime)
			{
				setirq = 0;
			}
		}

		if (setirq)
		{
//          printf("HIRQ @ %d, %d\n", pixel * state->m_htmult, snes_ppu.beam.current_vert);
			if (pixel == 0)
			{
				snes_hirq_tick(machine);
			}
			else
			{
				state->m_hirq_timer->adjust(machine.primary_screen->time_until_pos(snes_ppu.beam.current_vert, pixel * state->m_htmult));
			}
		}
	}

	/* Start of VBlank */
	if (snes_ppu.beam.current_vert == snes_ppu.beam.last_visible_line)
	{
		machine.scheduler().timer_set(machine.primary_screen->time_until_pos(snes_ppu.beam.current_vert, 10), FUNC(snes_reset_oam_address));

		snes_ram[HVBJOY] |= 0x81;		/* Set vblank bit to on & indicate controllers being read */
		snes_ram[RDNMI] |= 0x80;		/* Set NMI occurred bit */

		if (snes_ram[NMITIMEN] & 0x80)	/* NMI only signaled if this bit set */
		{
			// NMI goes off about 12 cycles after this (otherwise Chrono Trigger, NFL QB Club, etc. lock up)
			state->m_nmi_timer->adjust(state->m_maincpu->cycles_to_attotime(12));
		}

		/* three lines after start of vblank we update the controllers (value from snes9x) */
		state->m_io_timer->adjust(machine.primary_screen->time_until_pos(snes_ppu.beam.current_vert + 2, state->m_hblank_offset * state->m_htmult));
	}

	// hdma reset happens at scanline 0, H=~6
	if (snes_ppu.beam.current_vert == 0)
	{
		address_space *cpu0space = state->m_maincpu->memory().space(AS_PROGRAM);
		snes_hdma_init(cpu0space);
	}

	if (snes_ppu.beam.current_vert == 0)
	{	/* VBlank is over, time for a new frame */
		snes_ram[HVBJOY] &= 0x7f;		/* Clear vblank bit */
		snes_ram[RDNMI]  &= 0x7f;		/* Clear nmi occurred bit */
		snes_ram[STAT78] ^= 0x80;		/* Toggle field flag */
		snes_ppu.stat77_flags &= 0x3f;	/* Clear Time Over and Range Over bits */

		device_set_input_line(state->m_maincpu, G65816_LINE_NMI, CLEAR_LINE );
	}

	state->m_scanline_timer->adjust(attotime::never);
	state->m_hblank_timer->adjust(machine.primary_screen->time_until_pos(snes_ppu.beam.current_vert, state->m_hblank_offset * state->m_htmult));

//  printf("%02x %d\n",snes_ram[HVBJOY],snes_ppu.beam.current_vert);
}

/* This is called at the start of hblank *before* the scanline indicated in current_vert! */
static TIMER_CALLBACK( snes_hblank_tick )
{
	snes_state *state = machine.driver_data<snes_state>();
	address_space *cpu0space = state->m_maincpu->memory().space(AS_PROGRAM);
	int nextscan;

	snes_ppu.beam.current_vert = machine.primary_screen->vpos();

	/* make sure we halt */
	state->m_hblank_timer->adjust(attotime::never);

	/* draw a scanline */
	if (snes_ppu.beam.current_vert <= snes_ppu.beam.last_visible_line)
	{
		if (machine.primary_screen->vpos() > 0)
		{
			/* Do HDMA */
			if (snes_ram[HDMAEN])
				snes_hdma(cpu0space);

			machine.primary_screen->update_partial((snes_ppu.interlace == 2) ? (snes_ppu.beam.current_vert * snes_ppu.interlace) : snes_ppu.beam.current_vert - 1);
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

	state->m_scanline_timer->adjust(machine.primary_screen->time_until_pos(nextscan));
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

READ8_HANDLER( snes_open_bus_r )
{
	static UINT8 recurse = 0;
	UINT16 result;

	/* prevent recursion */
	if (recurse)
		return 0xff;

	recurse = 1;
	result = space->read_byte(cpu_get_pc(&space->device()) - 1); //LAST opcode that's fetched on the bus
	recurse = 0;
	return result;
}

/* read & write to DMA addresses are defined separately, to be called by snessdd1 handlers */
static READ8_HANDLER( snes_io_dma_r )
{
	snes_state *state = space->machine().driver_data<snes_state>();

	switch (offset)
	{
		case DMAP0:	case DMAP1: case DMAP2: case DMAP3: /*0x43n0*/
		case DMAP4: case DMAP5: case DMAP6: case DMAP7:
			return state->m_dma_channel[(offset >> 4) & 0x07].dmap;
		case BBAD0: case BBAD1: case BBAD2: case BBAD3: /*0x43n1*/
		case BBAD4: case BBAD5: case BBAD6: case BBAD7:
			return state->m_dma_channel[(offset >> 4) & 0x07].dest_addr;
		case A1T0L: case A1T1L: case A1T2L: case A1T3L: /*0x43n2*/
		case A1T4L: case A1T5L: case A1T6L: case A1T7L:
			return state->m_dma_channel[(offset >> 4) & 0x07].src_addr & 0xff;
		case A1T0H: case A1T1H: case A1T2H: case A1T3H: /*0x43n3*/
		case A1T4H: case A1T5H: case A1T6H: case A1T7H:
			return (state->m_dma_channel[(offset >> 4) & 0x07].src_addr >> 8) & 0xff;
		case A1B0: case A1B1: case A1B2: case A1B3:     /*0x43n4*/
		case A1B4: case A1B5: case A1B6: case A1B7:
			return state->m_dma_channel[(offset >> 4) & 0x07].bank;
		case DAS0L: case DAS1L: case DAS2L: case DAS3L: /*0x43n5*/
		case DAS4L: case DAS5L: case DAS6L: case DAS7L:
			return state->m_dma_channel[(offset >> 4) & 0x07].trans_size & 0xff;
		case DAS0H: case DAS1H: case DAS2H: case DAS3H: /*0x43n6*/
		case DAS4H: case DAS5H: case DAS6H: case DAS7H:
			return (state->m_dma_channel[(offset >> 4) & 0x07].trans_size >> 8) & 0xff;
		case DSAB0: case DSAB1: case DSAB2: case DSAB3: /*0x43n7*/
		case DSAB4: case DSAB5: case DSAB6: case DSAB7:
			return state->m_dma_channel[(offset >> 4) & 0x07].ibank;
		case A2A0L: case A2A1L: case A2A2L: case A2A3L: /*0x43n8*/
		case A2A4L: case A2A5L: case A2A6L: case A2A7L:
			return state->m_dma_channel[(offset >> 4) & 0x07].hdma_addr & 0xff;
		case A2A0H: case A2A1H: case A2A2H: case A2A3H: /*0x43n9*/
		case A2A4H: case A2A5H: case A2A6H: case A2A7H:
			return (state->m_dma_channel[(offset >> 4) & 0x07].hdma_addr >> 8) & 0xff;
		case NTRL0: case NTRL1: case NTRL2: case NTRL3: /*0x43na*/
		case NTRL4: case NTRL5: case NTRL6: case NTRL7:
			return state->m_dma_channel[(offset >> 4) & 0x07].hdma_line_counter;
		case 0x430b: case 0x431b: case 0x432b: case 0x433b: /* according to bsnes, this does not return open_bus (even if its precise effect is unknown) */
		case 0x434b: case 0x435b: case 0x436b: case 0x437b:
			return state->m_dma_channel[(offset >> 4) & 0x07].unk;
	}

	/* we should never arrive here */
	return snes_open_bus_r(space, 0);
}

static WRITE8_HANDLER( snes_io_dma_w )
{
	snes_state *state = space->machine().driver_data<snes_state>();

	switch (offset)
	{
			/* Below is all DMA related */
		case DMAP0:	case DMAP1: case DMAP2: case DMAP3: /*0x43n0*/
		case DMAP4: case DMAP5: case DMAP6: case DMAP7:
			state->m_dma_channel[(offset >> 4) & 0x07].dmap = data;
			break;
		case BBAD0: case BBAD1: case BBAD2: case BBAD3: /*0x43n1*/
		case BBAD4: case BBAD5: case BBAD6: case BBAD7:
			state->m_dma_channel[(offset >> 4) & 0x07].dest_addr = data;
			break;
		case A1T0L: case A1T1L: case A1T2L: case A1T3L: /*0x43n2*/
		case A1T4L: case A1T5L: case A1T6L: case A1T7L:
			state->m_dma_channel[(offset >> 4) & 0x07].src_addr = (state->m_dma_channel[(offset >> 4) & 0x07].src_addr & 0xff00) | (data << 0);
			break;
		case A1T0H: case A1T1H: case A1T2H: case A1T3H: /*0x43n3*/
		case A1T4H: case A1T5H: case A1T6H: case A1T7H:
			state->m_dma_channel[(offset >> 4) & 0x07].src_addr = (state->m_dma_channel[(offset >> 4) & 0x07].src_addr & 0x00ff) | (data << 8);
			break;
		case A1B0: case A1B1: case A1B2: case A1B3:     /*0x43n4*/
		case A1B4: case A1B5: case A1B6: case A1B7:
			state->m_dma_channel[(offset >> 4) & 0x07].bank = data;
			break;
		case DAS0L: case DAS1L: case DAS2L: case DAS3L: /*0x43n5*/
		case DAS4L: case DAS5L: case DAS6L: case DAS7L:
			state->m_dma_channel[(offset >> 4) & 0x07].trans_size = (state->m_dma_channel[(offset >> 4) & 0x07].trans_size & 0xff00) | (data << 0);
			break;
		case DAS0H: case DAS1H: case DAS2H: case DAS3H: /*0x43n6*/
		case DAS4H: case DAS5H: case DAS6H: case DAS7H:
			state->m_dma_channel[(offset >> 4) & 0x07].trans_size = (state->m_dma_channel[(offset >> 4) & 0x07].trans_size & 0x00ff) | (data << 8);
			break;
		case DSAB0: case DSAB1: case DSAB2: case DSAB3: /*0x43n7*/
		case DSAB4: case DSAB5: case DSAB6: case DSAB7:
			state->m_dma_channel[(offset >> 4) & 0x07].ibank = data;
			break;
		case A2A0L: case A2A1L: case A2A2L: case A2A3L: /*0x43n8*/
		case A2A4L: case A2A5L: case A2A6L: case A2A7L:
			state->m_dma_channel[(offset >> 4) & 0x07].hdma_addr = (state->m_dma_channel[(offset >> 4) & 0x07].hdma_addr & 0xff00) | (data << 0);
			break;
		case A2A0H: case A2A1H: case A2A2H: case A2A3H: /*0x43n9*/
		case A2A4H: case A2A5H: case A2A6H: case A2A7H:
			state->m_dma_channel[(offset >> 4) & 0x07].hdma_addr = (state->m_dma_channel[(offset >> 4) & 0x07].hdma_addr & 0x00ff) | (data << 8);
			break;
		case NTRL0: case NTRL1: case NTRL2: case NTRL3: /*0x43na*/
		case NTRL4: case NTRL5: case NTRL6: case NTRL7:
			state->m_dma_channel[(offset >> 4) & 0x07].hdma_line_counter = data;
			break;
		case 0x430b: case 0x431b: case 0x432b: case 0x433b:
		case 0x434b: case 0x435b: case 0x436b: case 0x437b:
			state->m_dma_channel[(offset >> 4) & 0x07].unk = data;
			break;
	}

	snes_ram[offset] = data;
}

/*
 * DR   - Double read : address is read twice to return a 16bit value.
 * low  - This is the low byte of a 16 or 24 bit value
 * mid  - This is the middle byte of a 24 bit value
 * high - This is the high byte of a 16 or 24 bit value
 */
READ8_HANDLER( snes_r_io )
{
	snes_state *state = space->machine().driver_data<snes_state>();
	UINT8 value = 0;

	// PPU accesses are from 2100 to 213f
	if (offset >= INIDISP && offset < APU00)
	{
		return snes_ppu_read(space, offset);
	}

	// APU is mirrored from 2140 to 217f
	if (offset >= APU00 && offset < WMDATA)
	{
		return spc_port_out(state->m_spc700, offset & 0x3);
	}

	if (state->m_has_addon_chip == HAS_SUPERFX && state->m_superfx != NULL)
	{
		if (offset >= 0x3000 && offset < 0x3300)
		{
			return superfx_mmio_read(state->m_superfx, offset);
		}
	}
	else if (state->m_has_addon_chip == HAS_RTC)
	{
		if (offset == 0x2800 || offset == 0x2801)
		{
			return srtc_read(space, offset);
		}
	}
	else if (state->m_has_addon_chip == HAS_SDD1)
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
	else if (state->m_has_addon_chip == HAS_SPC7110 || state->m_has_addon_chip == HAS_SPC7110_RTC)
	{
		UINT16 limit = (state->m_has_addon_chip == HAS_SPC7110_RTC) ? 0x4842 : 0x483f;
		if (offset >= 0x4800 && offset <= limit)
		{
			return spc7110_mmio_read(space, offset);
		}
	}

	if (offset >= DMAP0 && offset < 0x4380)
	{
		return snes_io_dma_r(space, offset);
	}

	/* offset is from 0x000000 */
	switch (offset)
	{
		case WMDATA:	/* Data to read from WRAM */
			value = space->read_byte(0x7e0000 + state->m_wram_address++);
			state->m_wram_address &= 0x1ffff;
			return value;
		case OLDJOY1:	/* Data for old NES controllers (JOYSER1) */
			if (snes_ram[offset] & 0x1)
				return 0 | (snes_open_bus_r(space, 0) & 0xfc); //correct?

			value = state->m_oldjoy1_read(space->machine());

			return (value & 0x03) | (snes_open_bus_r(space, 0) & 0xfc); //correct?
		case OLDJOY2:	/* Data for old NES controllers (JOYSER2) */
			if (snes_ram[OLDJOY1] & 0x1)
				return 0 | 0x1c | (snes_open_bus_r(space, 0) & 0xe0); //correct?

			value = state->m_oldjoy2_read(space->machine());

			return value | 0x1c | (snes_open_bus_r(space, 0) & 0xe0); //correct?
		case RDNMI:			/* NMI flag by v-blank and version number */
			value = (snes_ram[offset] & 0x80) | (snes_open_bus_r(space, 0) & 0x70);
			snes_ram[offset] &= 0x70;	/* NMI flag is reset on read */
			return value | 2; //CPU version number
		case TIMEUP:		/* IRQ flag by H/V count timer */
			value = (snes_open_bus_r(space, 0) & 0x7f) | (snes_ram[TIMEUP] & 0x80);
			device_set_input_line(state->m_maincpu, G65816_LINE_IRQ, CLEAR_LINE );
			snes_ram[TIMEUP] = 0;	// flag is cleared on both read and write
			return value;
		case HVBJOY:		/* H/V blank and joypad controller enable */
			// electronics test says hcounter 272 is start of hblank, which is beampos 363
//          if (space->machine().primary_screen->hpos() >= 363) snes_ram[offset] |= 0x40;
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
			return state->m_joy1l;
		case JOY1H:			/* Joypad 1 status register (high) */
			return state->m_joy1h;
		case JOY2L:			/* Joypad 2 status register (low) */
			return state->m_joy2l;
		case JOY2H:			/* Joypad 2 status register (high) */
			return state->m_joy2h;
		case JOY3L:			/* Joypad 3 status register (low) */
			return state->m_joy3l;
		case JOY3H:			/* Joypad 3 status register (high) */
			return state->m_joy3h;
		case JOY4L:			/* Joypad 4 status register (low) */
			return state->m_joy4l;
		case JOY4H:			/* Joypad 4 status register (high) */
			return state->m_joy4h;

		case 0x4100:		/* NSS Dip-Switches */
			{
				const input_port_config *port = space->machine().port("DSW");
				if (port != NULL)
					return input_port_read(space->machine(), "DSW");
				else
					return snes_open_bus_r(space, 0);
			}
//      case 0x4101: //PC: a104 - a10e - a12a   //only nss_actr
//      case 0x420c: //PC: 9c7d - 8fab          //only nss_ssoc

		default:
//          mame_printf_debug("snes_r: offset = %x pc = %x\n",offset,cpu_get_pc(&space->device()));
// Added break; after commenting above line.  If uncommenting, drop the break;
                        break;
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
	snes_state *state = space->machine().driver_data<snes_state>();

	// PPU accesses are from 2100 to 213f
	if (offset >= INIDISP && offset < APU00)
	{
		snes_ppu_write(space, offset, data);
		return;
	}

	// APU is mirrored from 2140 to 217f
	if (offset >= APU00 && offset < WMDATA)
	{
//      printf("816: %02x to APU @ %d (PC=%06x)\n", data, offset & 3,cpu_get_pc(&space->device()));
		spc_port_in(state->m_spc700, offset & 0x3, data);
		space->machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(20));
		return;
	}

	if (state->m_has_addon_chip == HAS_SUPERFX && state->m_superfx != NULL)
	{
		if (offset >= 0x3000 && offset < 0x3300)
		{
			superfx_mmio_write(state->m_superfx, offset, data);
			return;
		}
	}
	else if (state->m_has_addon_chip == HAS_RTC)
	{
		if (offset == 0x2800 || offset == 0x2801)
		{
			srtc_write(space->machine(), offset, data);
			return;
		}
	}
	else if (state->m_has_addon_chip == HAS_SDD1)
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
	else if (state->m_has_addon_chip == HAS_SPC7110 || state->m_has_addon_chip == HAS_SPC7110_RTC)
	{
		UINT16 limit = (state->m_has_addon_chip == HAS_SPC7110_RTC) ? 0x4842 : 0x483f;
		if (offset >= 0x4800 && offset <= limit)
		{
			spc7110_mmio_write(space->machine(), (UINT32)offset, data);
			return;
		}
	}

	if (offset >= DMAP0 && offset < 0x4380)
	{
		snes_io_dma_w(space, offset, data);
		return;
	}

	/* offset is from 0x000000 */
	switch (offset)
	{
		case WMDATA:	/* Data to write to WRAM */
			space->write_byte(0x7e0000 + state->m_wram_address++, data );
			state->m_wram_address &= 0x1ffff;
			return;
		case WMADDL:	/* Address to read/write to wram (low) */
			state->m_wram_address = (state->m_wram_address & 0xffff00) | (data <<  0);
			state->m_wram_address &= 0x1ffff;
			return;
		case WMADDM:	/* Address to read/write to wram (mid) */
			state->m_wram_address = (state->m_wram_address & 0xff00ff) | (data <<  8);
			state->m_wram_address &= 0x1ffff;
			return;
		case WMADDH:	/* Address to read/write to wram (high) */
			state->m_wram_address = (state->m_wram_address & 0x00ffff) | (data << 16);
			state->m_wram_address &= 0x1ffff;
			return;
		case OLDJOY1:	/* Old NES joystick support */
			if (((!(data & 0x1)) && (snes_ram[offset] & 0x1)))
			{
				state->m_read_idx[0] = 0;
				state->m_read_idx[1] = 0;
			}
			break;
		case NMITIMEN:	/* Flag for v-blank, timer int. and joy read */
			if((data & 0x30) == 0x00)
			{
				device_set_input_line(state->m_maincpu, G65816_LINE_IRQ, CLEAR_LINE );
				snes_ram[TIMEUP] = 0;	// clear pending IRQ if irq is disabled here, 3x3 Eyes - Seima Korin Den behaves on this
			}
			break;
		case OLDJOY2:	/* Old NES joystick support */
			break;
		case WRIO:		/* Programmable I/O port - latches H/V counters on a 0->1 transition */
			if (!(snes_ram[WRIO] & 0x80) && (data & 0x80))
			{
				// external latch
				snes_latch_counters(space->machine());
			}
			break;
		case WRMPYA:	/* Multiplier A */
			break;
		case WRMPYB:	/* Multiplier B */
			snes_ram[WRMPYB] = data;
//          state->m_mult_timer->adjust(state->m_maincpu->cycles_to_attotime(8));
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
//          state->m_div_timer->adjust(state->m_maincpu->cycles_to_attotime(16));
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
			state->m_htime = (state->m_htime & 0xff00) | (data <<  0);
			state->m_htime &= 0x1ff;
			return;
		case HTIMEH:	/* H-Count timer settings (high) */
			state->m_htime = (state->m_htime & 0x00ff) | (data <<  8);
			state->m_htime &= 0x1ff;
			return;
		case VTIMEL:	/* V-Count timer settings (low)  */
			state->m_vtime = (state->m_vtime & 0xff00) | (data <<  0);
			state->m_vtime &= 0x1ff;
			return;
		case VTIMEH:	/* V-Count timer settings (high) */
			state->m_vtime = (state->m_vtime & 0x00ff) | (data <<  8);
			state->m_vtime &= 0x1ff;
			return;
		case MDMAEN:	/* DMA channel designation and trigger */
			snes_dma(space, data);
			data = 0;	/* Once DMA is done we need to reset all bits to 0 */
			break;
		case HDMAEN:	/* HDMA channel designation */
			if (data) //if a HDMA is enabled, data is inited at the next scanline
				space->machine().scheduler().timer_set(space->machine().primary_screen->time_until_pos(snes_ppu.beam.current_vert + 1), FUNC(snes_reset_hdma));
			break;
		case MEMSEL:	/* Access cycle designation in memory (2) area */
			/* FIXME: Need to adjust the speed only during access of banks 0x80+
             * Currently we are just increasing it no matter what */
//          state->m_maincpu->set_clock_scale((data & 0x1) ? 1.335820896 : 1.0 );
#ifdef SNES_DBG_REG_W
			if ((data & 0x1) != (snes_ram[MEMSEL] & 0x1))
				mame_printf_debug( "CPU speed: %f Mhz\n", (data & 0x1) ? 3.58 : 2.68 );
#endif
			break;
		case TIMEUP:	// IRQ Flag is cleared on both read and write
			snes_ram[TIMEUP] = 0;
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
	}

	snes_ram[offset] = data;
}

WRITE_LINE_DEVICE_HANDLER( snes_extern_irq_w )
{
	snes_state *driver_state = device->machine().driver_data<snes_state>();
	device_set_input_line(driver_state->m_maincpu, G65816_LINE_IRQ, state);
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

#if USE_CYCLE_STEAL
/*FIXME: missing work RAM access steal / we need to do this less "aggressive" otherwise we lose too much CPU horsepower, why? */
static int snes_bank_0x00_0x3f_cycles(running_machine &machine,UINT32 offset)
{
/*
 $00-$3F | $0000-$1FFF | Slow  | Address Bus A + /WRAM (mirror $7E:0000-$1FFF)
         | $2000-$20FF | Fast  | Address Bus A
         | $2100-$21FF | Fast  | Address Bus B
         | $2200-$3FFF | Fast  | Address Bus A
         | $4000-$41FF | XSlow | Internal CPU registers (see Note 1 below)
         | $4200-$43FF | Fast  | Internal CPU registers (see Note 1 below)
         | $4400-$5FFF | Fast  | Address Bus A
         | $6000-$7FFF | Slow  | Address Bus A
         | $8000-$FFFF | Slow  | Address Bus A + /CART
         */

	if(((offset & 0xff00) == 0x4000) || ((offset & 0xff00) == 0x4100))
		return 0; //TODO: 12
	if(((offset & 0xff00) == 0x4200) || ((offset & 0xff00) == 0x4300))
		return 0; //TODO: 6

	if(((offset & 0xff00) >= 0x0000) && ((offset & 0xff00) <= 0x1f00))
		return 0; //TODO: 8

	if((offset & 0xff00) >= 0x6000)
		return 8;

	return 0; //TODO: 6
}

static int snes_bank_0x80_0xbf_cycles(running_machine &machine,UINT32 offset)
{
/*
 $80-$BF | $0000-$1FFF | Slow  | Address Bus A + /WRAM (mirror $7E:0000-$1FFF)
         | $2000-$20FF | Fast  | Address Bus A
         | $2100-$21FF | Fast  | Address Bus B
         | $2200-$3FFF | Fast  | Address Bus A
         | $4000-$41FF | XSlow | Internal CPU registers (see Note 1 below)
         | $4200-$43FF | Fast  | Internal CPU registers (see Note 1 below)
         | $4400-$5FFF | Fast  | Address Bus A
         | $6000-$7FFF | Slow  | Address Bus A
         | $8000-$FFFF | Note2 | Address Bus A + /CART
*/


	if(((offset & 0xff00) == 0x4000) || ((offset & 0xff00) == 0x4100))
		return 0; //TODO: 12

	if(((offset & 0xff00) == 0x4200) || ((offset & 0xff00) == 0x4300))
		return 0; //TODO: 6

	if(((offset & 0xff00) >= 0x0000) && ((offset & 0xff00) <= 0x1f00))
		return 0; //TODO: 8

	if(((offset & 0xff00) >= 0x6000) && ((offset & 0xff00) <= 0x7f00))
		return 0; //TODO: 8

	if(((offset & 0xff00) >= 0x8000) && ((offset & 0xff00) <= 0xff00))
		return (snes_ram[MEMSEL] & 1) ? 6 : 8;

	return 0; //TODO: 6
}
#endif

/* 0x000000 - 0x2fffff */
READ8_HANDLER( snes_r_bank1 )
{
	snes_state *state = space->machine().driver_data<snes_state>();
	UINT8 value = 0xff;
	UINT16 address = offset & 0xffff;

	if (address < 0x2000)											/* Mirror of Low RAM */
		value = space->read_byte(0x7e0000 + address);
	else if (address < 0x6000)										/* I/O */
	{
		if (state->m_cart[0].mode == SNES_MODE_BSX && address >= 0x5000)
			value = bsx_read(space, offset);
		else
			value = snes_r_io(space, address);
	}
	else if (address < 0x8000)
	{
		if (state->m_has_addon_chip == HAS_SUPERFX && state->m_superfx != NULL)
		{
			if (superfx_access_ram(state->m_superfx))
				value = snes_ram[0xf00000 + (offset & 0x1fff)];	// here it should be 0xe00000 but there are mirroring issues
			else
				value = snes_open_bus_r(space, 0);
		}
		else if (state->m_has_addon_chip == HAS_OBC1)
			value = obc1_read(space, offset);
		else if ((state->m_cart[0].mode == SNES_MODE_21) && (state->m_has_addon_chip == HAS_DSP1) && (offset < 0x100000))
			value = (address < 0x7000) ? dsp_get_dr() : dsp_get_sr();
		else if (state->m_has_addon_chip == HAS_CX4)
			value = CX4_read(address - 0x6000);
		else if (state->m_has_addon_chip == HAS_SPC7110 || state->m_has_addon_chip == HAS_SPC7110_RTC)
		{
			if (offset < 0x10000)
				value = snes_ram[0x306000 + (offset & 0x1fff)];
		}
		else
		{
			logerror("(PC=%06x) snes_r_bank1: Unmapped external chip read: %04x\n",cpu_get_pc(&space->device()),address);
			value = snes_open_bus_r(space, 0);								/* Reserved */
		}
	}
	else if ((state->m_cart[0].mode == SNES_MODE_20) && (state->m_has_addon_chip == HAS_DSP1) && (offset >= 0x200000))
		value = (address < 0xc000) ? dsp_get_dr() : dsp_get_sr();
	else if ((state->m_cart[0].mode == SNES_MODE_20) && (state->m_has_addon_chip == HAS_DSP2) && (offset >= 0x200000))
		value = (address < 0xc000) ? dsp_get_dr() : dsp_get_sr();
	else if ((state->m_has_addon_chip == HAS_DSP3) && (offset >= 0x200000))
		value = (address < 0xc000) ? dsp_get_dr() : dsp_get_sr();
	else
		value = snes_ram[offset];

	#if USE_CYCLE_STEAL
	if(!space->debugger_access())
		device_adjust_icount(&space->device(), -snes_bank_0x00_0x3f_cycles(space->machine(), offset));
	#endif

	return value;
}

/* 0x300000 - 0x3fffff */
READ8_HANDLER( snes_r_bank2 )
{
	snes_state *state = space->machine().driver_data<snes_state>();
	UINT8 value = 0xff;
	UINT16 address = offset & 0xffff;

	if (address < 0x2000)											/* Mirror of Low RAM */
		value = space->read_byte(0x7e0000 + address);
	else if (address < 0x6000)										/* I/O */
	{
		if (state->m_cart[0].mode == SNES_MODE_BSX && address >= 0x5000)
			value = bsx_read(space, 0x300000 + offset);
		else
			value = snes_r_io(space, address);
	}
	else if (address < 0x8000)										/* SRAM for mode_21, Reserved othewise */
	{
		if (state->m_has_addon_chip == HAS_SUPERFX && state->m_superfx != NULL)
		{
			if (superfx_access_ram(state->m_superfx))
				value = snes_ram[0xf00000 + (offset & 0x1fff)];	// here it should be 0xe00000 but there are mirroring issues
			else
				value = snes_open_bus_r(space, 0);
		}
		else if (state->m_has_addon_chip == HAS_OBC1)
			value = obc1_read (space, offset);
		else if (state->m_has_addon_chip == HAS_CX4)
			value = CX4_read(address - 0x6000);
		else if (state->m_has_addon_chip == HAS_SPC7110 || state->m_has_addon_chip == HAS_SPC7110_RTC)
		{
			if (offset < 0x10000)
				value = snes_ram[0x306000 + (offset & 0x1fff)];
		}
		else if ((state->m_cart[0].mode == SNES_MODE_21) && (state->m_cart[0].sram > 0))
		{
			/* Donkey Kong Country checks this and detects a copier if 0x800 is not masked out due to sram size */
			/* OTOH Secret of Mana does not work properly if sram is not mirrored on later banks */
			int mask = (state->m_cart[0].sram - 1) | 0xffe000; /* Limit SRAM size to what's actually present */
			value = snes_ram[0x300000 + (offset & mask)];
		}
		else
		{
			logerror( "(PC=%06x) snes_r_bank2: Unmapped external chip read: %04x\n",cpu_get_pc(&space->device()),address );
			value = snes_open_bus_r(space, 0);
		}
	}
	/* some dsp1 games use these banks 0x30 to 0x3f at address 0x8000 */
	else if ((state->m_cart[0].mode == SNES_MODE_20) && (state->m_has_addon_chip == HAS_DSP1))
		value = (address < 0xc000) ? dsp_get_dr() : dsp_get_sr();
	else if ((state->m_cart[0].mode == SNES_MODE_20) && (state->m_has_addon_chip == HAS_DSP2))
		value = (address < 0xc000) ? dsp_get_dr() : dsp_get_sr();
	else if (state->m_has_addon_chip == HAS_DSP3)
		value = (address < 0xc000) ? dsp_get_dr() : dsp_get_sr();
	else if (state->m_has_addon_chip == HAS_DSP4)
		value = (address < 0xc000) ? dsp_get_dr() : dsp_get_sr();
	else
		value = snes_ram[0x300000 + offset];

	#if USE_CYCLE_STEAL
	if(!space->debugger_access())
		device_adjust_icount(&space->device(), -snes_bank_0x00_0x3f_cycles(space->machine(), offset));
	#endif

	return value;
}

/* 0x400000 - 0x5fffff */
READ8_HANDLER( snes_r_bank3 )
{
	snes_state *state = space->machine().driver_data<snes_state>();
	UINT8 value = 0xff;
	UINT16 address = offset & 0xffff;

	if (state->m_has_addon_chip == HAS_SUPERFX && state->m_superfx != NULL)
	{
		if (superfx_access_rom(state->m_superfx))
			value = snes_ram[0x400000 + offset];
		else
		{
			static const UINT8 sfx_data[16] = {
				0x00, 0x01, 0x00, 0x01, 0x04, 0x01, 0x00, 0x01,
				0x00, 0x01, 0x08, 0x01, 0x00, 0x01, 0x0c, 0x01,
			};
			return sfx_data[offset & 0x0f];
		}
	}
	else if ((state->m_has_addon_chip == HAS_SPC7110 || state->m_has_addon_chip == HAS_SPC7110_RTC))
	{
		if (offset >= 0x100000 && offset < 0x110000)
			value = spc7110_mmio_read(space, 0x4800);
	}
	else if ((state->m_cart[0].mode & 5) && !(state->m_has_addon_chip == HAS_SUPERFX))	/* Mode 20 & 22 */
	{
		if ((address < 0x8000) && (state->m_cart[0].mode == SNES_MODE_20))
			value = snes_open_bus_r(space, 0);							/* Reserved */
		else
			value = snes_ram[0x400000 + offset];
	}
	else											/* Mode 21 & 25 + SuperFX games */
		value = snes_ram[0x400000 + offset];

	#if USE_CYCLE_STEAL
	if(!space->debugger_access())
		device_adjust_icount(&space->device(), -8);
	#endif

	return value;
}

/* 0x600000 - 0x6fffff */
READ8_HANDLER( snes_r_bank4 )
{
	snes_state *state = space->machine().driver_data<snes_state>();
	UINT8 value = 0xff;
	UINT16 address = offset & 0xffff;

	if (state->m_has_addon_chip == HAS_SUPERFX && state->m_superfx != NULL)
	{
		if (superfx_access_ram(state->m_superfx))
			value = snes_ram[0xe00000 + offset];
		else
			value = snes_open_bus_r(space, 0);
	}
	else if (state->m_has_addon_chip == HAS_ST010 || state->m_has_addon_chip == HAS_ST011)
	{
		if (offset >= 0x80000 && address < 0x1000)
		{
			value = st010_read_ram(state, address);
		}
		else if (offset <= 1)
		{
			value = (address & 1) ? st010_get_sr() : st010_get_dr();
		}
	}
	else if (state->m_cart[0].mode & 5)							/* Mode 20 & 22 */
	{
		if (address >= 0x8000)
			value = snes_ram[0x600000 + offset];
		/* some other dsp1 games use these banks 0x60 to 0x6f at address 0x0000 */
		else if (state->m_has_addon_chip == HAS_DSP1)
			value = (address >= 0x4000) ? dsp_get_sr() : dsp_get_dr();
		else
		{
			logerror("(PC=%06x) snes_r_bank4: Unmapped external chip read: %04x\n",cpu_get_pc(&space->device()),address);
			value = snes_open_bus_r(space, 0);							/* Reserved */
		}
	}
	else if (state->m_cart[0].mode & 0x0a)					/* Mode 21 & 25 */
		value = snes_ram[0x600000 + offset];

	#if USE_CYCLE_STEAL
	if(!space->debugger_access())
		device_adjust_icount(&space->device(), -8);
	#endif

	return value;
}

/* 0x700000 - 0x7dffff */
READ8_HANDLER( snes_r_bank5 )
{
	snes_state *state = space->machine().driver_data<snes_state>();
	UINT8 value;
	UINT16 address = offset & 0xffff;

	if (state->m_has_addon_chip == HAS_SUPERFX && state->m_superfx != NULL)
	{
		if (superfx_access_ram(state->m_superfx))
			value = snes_ram[0xf00000 + offset];
		else
			value = snes_open_bus_r(space, 0);
	}
	else if ((state->m_cart[0].mode & 5) && (address < 0x8000))		/* Mode 20 & 22 */
	{
		if (state->m_cart[0].sram > 0)
		{
			int mask = state->m_cart[0].sram - 1;	/* Limit SRAM size to what's actually present */
			value = snes_ram[0x700000 + (offset & mask)];
		}
		else
		{
			logerror("(PC=%06x) snes_r_bank5: Unmapped external chip read: %04x\n",cpu_get_pc(&space->device()),address);
			value = snes_open_bus_r(space, 0);								/* Reserved */
		}
	}
	else
		value = snes_ram[0x700000 + offset];

	#if USE_CYCLE_STEAL
	if(!space->debugger_access())
		device_adjust_icount(&space->device(), -8);
	#endif

	return value;
}

/* 0x800000 - 0xbfffff */
READ8_HANDLER( snes_r_bank6 )
{
	snes_state *state = space->machine().driver_data<snes_state>();
	UINT8 value = 0;
	UINT16 address = offset & 0xffff;

	if (state->m_has_addon_chip == HAS_SUPERFX)
		value = space->read_byte(offset);
	else if (address < 0x8000)
	{
		if (state->m_cart[0].mode != SNES_MODE_25)
			value = space->read_byte(offset);
		else if ((state->m_has_addon_chip == HAS_CX4) && (address >= 0x6000))
			value = CX4_read(address - 0x6000);
		else							/* Mode 25 has SRAM not mirrored from lower banks */
		{
			if (address < 0x6000)
				value = space->read_byte(offset);
			else if ((offset >= 0x300000) && (state->m_cart[0].sram > 0))
			{
				int mask = (state->m_cart[0].sram - 1) | 0xff0000; /* Limit SRAM size to what's actually present */
				value = snes_ram[0x800000 + (offset & mask)];
			}
			else						/* Area 0x6000-0x8000 with offset < 0x300000 is reserved */
			{
				logerror("(PC=%06x) snes_r_bank6: Unmapped external chip read: %04x\n",cpu_get_pc(&space->device()),address);
				value = snes_open_bus_r(space, 0);
			}
		}
	}
	else if ((state->m_cart[0].mode == SNES_MODE_20) && (state->m_has_addon_chip == HAS_DSP1) && (offset >= 0x200000))
		value = (address < 0xc000) ? dsp_get_dr() : dsp_get_sr();
	else if ((state->m_cart[0].mode == SNES_MODE_20) && (state->m_has_addon_chip == HAS_DSP2) && (offset >= 0x200000))
		value = (address < 0xc000) ? dsp_get_dr() : dsp_get_sr();
	else if ((state->m_has_addon_chip == HAS_DSP3) && (offset >= 0x200000))
		value = (address < 0xc000) ? dsp_get_dr() : dsp_get_sr();
	else if ((state->m_has_addon_chip == HAS_DSP4) && (offset >= 0x300000))
		value = (address < 0xc000) ? dsp_get_dr() : dsp_get_sr();
	else
		value = snes_ram[0x800000 + offset];

	#if USE_CYCLE_STEAL
	if(!space->debugger_access())
		device_adjust_icount(&space->device(), -snes_bank_0x80_0xbf_cycles(space->machine(), offset));
	#endif

	return value;
}

/* 0xc00000 - 0xffffff */
READ8_HANDLER( snes_r_bank7 )
{
	snes_state *state = space->machine().driver_data<snes_state>();
	UINT8 value = 0;
	UINT16 address = offset & 0xffff;

	if (state->m_has_addon_chip == HAS_SUPERFX && state->m_superfx != NULL)
	{
		if (offset < 0x200000)	// ROM
		{
			if (superfx_access_rom(state->m_superfx))
				value = snes_ram[0xc00000 + offset];
			else
			{
				static const UINT8 sfx_data[16] = {
					0x00, 0x01, 0x00, 0x01, 0x04, 0x01, 0x00, 0x01,
					0x00, 0x01, 0x08, 0x01, 0x00, 0x01, 0x0c, 0x01,
				};
				return sfx_data[offset & 0x0f];
			}
		}
		else	// RAM
		{
			offset -= 0x200000;
			if (superfx_access_ram(state->m_superfx))
				value = snes_ram[0xe00000 + offset];
			else
				value = snes_open_bus_r(space, 0);
		}
	}
	else if ((state->m_has_addon_chip == HAS_SPC7110 || state->m_has_addon_chip == HAS_SPC7110_RTC) && offset >= 0x100000)
		value = spc7110_bank7_read(space, offset);
	else if (state->m_has_addon_chip == HAS_SDD1)
		value = sdd1_read(space->machine(), offset);
	else if (state->m_has_addon_chip == HAS_ST010 || state->m_has_addon_chip == HAS_ST011)
	{
		if (offset >= 0x280000 && offset < 0x300000 && address < 0x1000)
		{
			value = st010_read_ram(state, address);
		}
		else if (offset >= 0x200000 && offset <= 0x200001)
		{
			value = (address & 1) ? st010_get_sr() : st010_get_dr();
		}
	}
	else if ((state->m_cart[0].mode & 5) && !(state->m_has_addon_chip == HAS_SUPERFX))		/* Mode 20 & 22 */
	{
		if (address < 0x8000)
			value = space->read_byte(0x400000 + offset);
		else
			value = snes_ram[0xc00000 + offset];
	}
	else								/* Mode 21 & 25 + SuperFX Games */
		value = snes_ram[0xc00000 + offset];

	#if USE_CYCLE_STEAL
	if(!space->debugger_access())
		device_adjust_icount(&space->device(), -((snes_ram[MEMSEL] & 1) ? 6 : 8));
	#endif

	return value;
}


/* 0x000000 - 0x2fffff */
WRITE8_HANDLER( snes_w_bank1 )
{
	snes_state *state = space->machine().driver_data<snes_state>();
	UINT16 address = offset & 0xffff;

	if (address < 0x2000)							/* Mirror of Low RAM */
		space->write_byte(0x7e0000 + address, data);
	else if (address < 0x6000)						/* I/O */
	{
		if (state->m_cart[0].mode == SNES_MODE_BSX && address >= 0x5000)
			bsx_write(space, offset, data);
		else
			snes_w_io(space, address, data);
	}
	else if (address < 0x8000)
	{
		if (state->m_has_addon_chip == HAS_SUPERFX)
			snes_ram[0xf00000 + (offset & 0x1fff)] = data;	// here it should be 0xe00000 but there are mirroring issues
		else if (state->m_has_addon_chip == HAS_OBC1)
			obc1_write(space, offset, data);
		else if ((state->m_cart[0].mode == SNES_MODE_21) && (state->m_has_addon_chip == HAS_DSP1) && (offset < 0x100000))
			dsp_set_dr(data);
		else if (state->m_has_addon_chip == HAS_CX4)
			CX4_write(space->machine(), address - 0x6000, data);
		else if (state->m_has_addon_chip == HAS_SPC7110 || state->m_has_addon_chip == HAS_SPC7110_RTC)
		{
			if (offset < 0x10000)
				snes_ram[0x306000 + (offset & 0x1fff)] = data;
		}
		else
			logerror("snes_w_bank1: Attempt to write to reserved address: %x = %02x\n", offset, data);
	}
	else if ((state->m_cart[0].mode == SNES_MODE_20) && (state->m_has_addon_chip == HAS_DSP1) && (offset >= 0x200000))
		dsp_set_dr(data);
	else if ((state->m_cart[0].mode == SNES_MODE_20) && (state->m_has_addon_chip == HAS_DSP2) && (offset >= 0x200000))
	{
		if (address < 0xc000)
			dsp_set_dr(data);
		else
			dsp_set_sr(data);
	}
	else if ((state->m_has_addon_chip == HAS_DSP3) && (offset >= 0x200000))
		if (address < 0xc000)
			dsp_set_dr(data);
		else
			dsp_set_sr(data);
	else
		logerror( "(PC=%06x) Attempt to write to ROM address: %X\n",cpu_get_pc(&space->device()),offset );

	#if USE_CYCLE_STEAL
	if(!space->debugger_access())
		device_adjust_icount(&space->device(), -snes_bank_0x00_0x3f_cycles(space->machine(), offset));
	#endif
}

/* 0x300000 - 0x3fffff */
WRITE8_HANDLER( snes_w_bank2 )
{
	snes_state *state = space->machine().driver_data<snes_state>();
	UINT16 address = offset & 0xffff;

	if (address < 0x2000)							/* Mirror of Low RAM */
		space->write_byte(0x7e0000 + address, data);
	else if (address < 0x6000)						/* I/O */
	{
		if (state->m_cart[0].mode == SNES_MODE_BSX && address >= 0x5000)
			bsx_write(space, 0x300000 + offset, data);
		else
			snes_w_io(space, address, data);
	}
	else if (address < 0x8000)						/* SRAM for mode_21, Reserved othewise */
	{
		if (state->m_has_addon_chip == HAS_SUPERFX)
			snes_ram[0xf00000 + (offset & 0x1fff)] = data;	// here it should be 0xe00000 but there are mirroring issues
		else if (state->m_has_addon_chip == HAS_OBC1)
			obc1_write(space, offset, data);
		else if (state->m_has_addon_chip == HAS_CX4)
			CX4_write(space->machine(), address - 0x6000, data);
		else if (state->m_has_addon_chip == HAS_SPC7110 || state->m_has_addon_chip == HAS_SPC7110_RTC)
		{
			if (offset < 0x10000)
				snes_ram[0x306000 + (offset & 0x1fff)] = data;
		}
		else if ((state->m_cart[0].mode == SNES_MODE_21) && (state->m_cart[0].sram > 0))
		{
			/* Donkey Kong Country checks this and detects a copier if 0x800 is not masked out due to sram size */
			/* OTOH Secret of Mana does not work properly if sram is not mirrored on later banks */
			int mask = (state->m_cart[0].sram - 1) | 0xffe000; /* Limit SRAM size to what's actually present */
			snes_ram[0x300000 + (offset & mask)] = data;
		}
		else
			logerror("snes_w_bank2: Attempt to write to reserved address: %X = %02x\n", offset + 0x300000, data);
	}
	/* some dsp1 games use these banks 0x30 to 0x3f at address 0x8000 */
	else if ((state->m_cart[0].mode == SNES_MODE_20) && (state->m_has_addon_chip == HAS_DSP1))
		dsp_set_dr(data);
	else if ((state->m_cart[0].mode == SNES_MODE_20) && (state->m_has_addon_chip == HAS_DSP2))
	{
		if (address < 0xc000)
			dsp_set_dr(data);
		else
			dsp_set_sr(data);
	}
	else if ((state->m_has_addon_chip == HAS_DSP3) || (state->m_has_addon_chip == HAS_DSP4))
		if (address < 0xc000)
			dsp_set_dr(data);
		else
			dsp_set_sr(data);
	else
		logerror("(PC=%06x) Attempt to write to ROM address: %X\n",cpu_get_pc(&space->device()),offset + 0x300000);

	#if USE_CYCLE_STEAL
	if(!space->debugger_access())
		device_adjust_icount(&space->device(), -snes_bank_0x00_0x3f_cycles(space->machine(), offset));
	#endif
}

/* 0x600000 - 0x6fffff */
WRITE8_HANDLER( snes_w_bank4 )
{
	snes_state *state = space->machine().driver_data<snes_state>();
	UINT16 address = offset & 0xffff;

	if (state->m_has_addon_chip == HAS_SUPERFX)
		snes_ram[0xe00000 + offset] = data;
	else if (state->m_has_addon_chip == HAS_ST010 || state->m_has_addon_chip == HAS_ST011)
	{
		if (offset >= 0x80000 && address < 0x1000)
		{
			st010_write_ram(state, address, data);
		}
		else if (offset == 0)
		{
			st010_set_dr(data);
		}
		else if (offset == 1)
		{
			st010_set_sr(data);
		}
	}
	else if (state->m_cart[0].mode & 5)					/* Mode 20 & 22 */
	{
		if (address >= 0x8000)
			logerror("(PC=%06x) Attempt to write to ROM address: %X\n",cpu_get_pc(&space->device()),offset + 0x600000);
		else if (state->m_has_addon_chip == HAS_DSP1)
			dsp_set_dr(data);
		else
			logerror("snes_w_bank4: Attempt to write to reserved address: %X = %02x\n", offset + 0x600000, data);
	}
	else if (state->m_cart[0].mode & 0x0a)
		logerror("(PC=%06x) Attempt to write to ROM address: %X\n",cpu_get_pc(&space->device()),offset + 0x600000);

	#if USE_CYCLE_STEAL
	if(!space->debugger_access())
		device_adjust_icount(&space->device(), -8);
	#endif
}

/* 0x700000 - 0x7dffff */
WRITE8_HANDLER( snes_w_bank5 )
{
	snes_state *state = space->machine().driver_data<snes_state>();
	UINT16 address = offset & 0xffff;

	if (state->m_has_addon_chip == HAS_SUPERFX)
		snes_ram[0xf00000 + offset] = data;
	else if ((state->m_cart[0].mode & 5) && (address < 0x8000))			/* Mode 20 & 22 */
	{
		if (state->m_cart[0].sram > 0)
		{
			int mask = state->m_cart[0].sram - 1;	/* Limit SRAM size to what's actually present */
			snes_ram[0x700000 + (offset & mask)] = data;
		}
		else
			logerror("snes_w_bank5: Attempt to write to reserved address: %X = %02x\n", offset + 0x700000, data);
	}
	else if (state->m_cart[0].mode & 0x0a)
		logerror("(PC=%06x) Attempt to write to ROM address: %X\n",cpu_get_pc(&space->device()),offset + 0x700000);

	#if USE_CYCLE_STEAL
	if(!space->debugger_access())
		device_adjust_icount(&space->device(), -8);
	#endif
}


/* 0x800000 - 0xbfffff */
WRITE8_HANDLER( snes_w_bank6 )
{
	snes_state *state = space->machine().driver_data<snes_state>();
	UINT16 address = offset & 0xffff;

	if (state->m_has_addon_chip == HAS_SUPERFX)
		space->write_byte(offset, data);
	else if (address < 0x8000)
	{
		if ((state->m_has_addon_chip == HAS_CX4) && (address >= 0x6000))
			CX4_write(space->machine(), address - 0x6000, data);
		else if (state->m_cart[0].mode != SNES_MODE_25)
			space->write_byte(offset, data);
		else	/* Mode 25 has SRAM not mirrored from lower banks */
		{
			if (address < 0x6000)
				space->write_byte(offset, data);
			else if ((offset >= 0x300000) && (state->m_cart[0].sram > 0))
			{
				int mask = (state->m_cart[0].sram - 1) | 0xff0000; /* Limit SRAM size to what's actually present */
				snes_ram[0x800000 + (offset & mask)] = data;
			}
			else	/* Area in 0x6000-0x8000 && offset < 0x300000 is Reserved! */
				logerror("snes_w_bank6: Attempt to write to reserved address: %X = %02x\n", offset + 0x800000, data);
		}
	}
	else if ((state->m_cart[0].mode == SNES_MODE_20) && (state->m_has_addon_chip == HAS_DSP1) && (offset >= 0x200000))
		dsp_set_dr(data);
	else if ((state->m_cart[0].mode == SNES_MODE_20) && (state->m_has_addon_chip == HAS_DSP2) && (offset >= 0x200000))
	{
		if (address < 0xc000)
			dsp_set_dr(data);
		else
			dsp_set_sr(data);
	}
	else if ((state->m_has_addon_chip == HAS_DSP3) && (offset >= 0x200000))
		if (address < 0xc000)
			dsp_set_dr(data);
		else
			dsp_set_sr(data);
	else if ((state->m_has_addon_chip == HAS_DSP4) && (offset >= 0x300000))
		if (address < 0xc000)
			dsp_set_dr(data);
		else
			dsp_set_sr(data);
	else
		logerror("(PC=%06x) Attempt to write to ROM address: %X\n",cpu_get_pc(&space->device()),offset + 0x800000);

	#if USE_CYCLE_STEAL
	if(!space->debugger_access())
		device_adjust_icount(&space->device(), -snes_bank_0x80_0xbf_cycles(space->machine(), offset));
	#endif
}


/* 0xc00000 - 0xffffff */
WRITE8_HANDLER( snes_w_bank7 )
{
	snes_state *state = space->machine().driver_data<snes_state>();
	UINT16 address = offset & 0xffff;

	if (state->m_has_addon_chip == HAS_SUPERFX)
	{
		if (offset >= 0x200000)
		{
			offset -= 0x200000;
			snes_ram[0xe00000 + offset] = data;		// SFX RAM
		}
		else
			logerror("(PC=%06x) Attempt to write to ROM address: %X\n",cpu_get_pc(&space->device()),offset + 0xc00000);
	}
	else if (state->m_has_addon_chip == HAS_ST010 || state->m_has_addon_chip == HAS_ST011)
	{
		if (offset >= 0x280000 && offset < 0x300000 && address < 0x1000)
		{
			st010_write_ram(state, address, data);
		}
		else if (offset == 0x200000)
		{
			st010_set_dr(data);
		}
		else if (offset == 0x200001)
		{
			st010_set_sr(data);
		}
	}
	else if (state->m_cart[0].mode & 5)				/* Mode 20 & 22 */
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
			logerror("(PC=%06x) snes_w_bank7: Attempt to write to ROM address: %X = %02x\n",cpu_get_pc(&space->device()),offset + 0xc00000, data);
	}
	else if (state->m_cart[0].mode & 0x0a)
		logerror("(PC=%06x) Attempt to write to ROM address: %X\n",cpu_get_pc(&space->device()),offset + 0xc00000);

	#if USE_CYCLE_STEAL
	if(!space->debugger_access())
		device_adjust_icount(&space->device(), -((snes_ram[MEMSEL] & 1) ? 6 : 8));
	#endif
}


/*************************************

    Input Callbacks

*************************************/

static void nss_io_read( running_machine &machine )
{
	snes_state *state = machine.driver_data<snes_state>();
	static const char *const portnames[2][4] =
			{
				{ "SERIAL1_DATA1_L", "SERIAL1_DATA1_H", "SERIAL1_DATA2_L", "SERIAL1_DATA2_H" },
				{ "SERIAL2_DATA1_L", "SERIAL2_DATA1_H", "SERIAL2_DATA2_L", "SERIAL2_DATA2_H" },
			};
	int port;

	for (port = 0; port < 2; port++)
	{
		state->m_data1[port] = input_port_read(machine, portnames[port][0]) | (input_port_read(machine, portnames[port][1]) << 8);
		state->m_data2[port] = input_port_read(machine, portnames[port][2]) | (input_port_read(machine, portnames[port][3]) << 8);

		// avoid sending signals that could crash games
		// if left, no right
		if (state->m_data1[port] & 0x200)
			state->m_data1[port] &= ~0x100;
		// if up, no down
		if (state->m_data1[port] & 0x800)
			state->m_data1[port] &= ~0x400;

		state->m_joypad[port].buttons = state->m_data1[port];
	}

	// is automatic reading on? if so, copy port data1/data2 to joy1l->joy4h
	// this actually works like reading the first 16bits from oldjoy1/2 in reverse order
	if (snes_ram[NMITIMEN] & 1)
	{
		state->m_joy1l = (state->m_data1[0] & 0x00ff) >> 0;
		state->m_joy1h = (state->m_data1[0] & 0xff00) >> 8;
		state->m_joy2l = (state->m_data1[1] & 0x00ff) >> 0;
		state->m_joy2h = (state->m_data1[1] & 0xff00) >> 8;
		state->m_joy3l = (state->m_data2[0] & 0x00ff) >> 0;
		state->m_joy3h = (state->m_data2[0] & 0xff00) >> 8;
		state->m_joy4l = (state->m_data2[1] & 0x00ff) >> 0;
		state->m_joy4h = (state->m_data2[1] & 0xff00) >> 8;

		// make sure read_idx starts returning all 1s because the auto-read reads it :-)
		state->m_read_idx[0] = 16;
		state->m_read_idx[1] = 16;
	}

}

static UINT8 nss_oldjoy1_read( running_machine &machine )
{
	snes_state *state = machine.driver_data<snes_state>();
	UINT8 res;

	if (state->m_read_idx[0] >= 16)
		res = 0x01;
	else
		res = (state->m_joypad[0].buttons >> (15 - state->m_read_idx[0]++)) & 0x01;

	return res;
}

static UINT8 nss_oldjoy2_read( running_machine &machine )
{
	snes_state *state = machine.driver_data<snes_state>();
	UINT8 res;

	if (state->m_read_idx[1] >= 16)
		res = 0x01;
	else
		res = (state->m_joypad[1].buttons >> (15 - state->m_read_idx[1]++)) & 0x01;

	return res;
}

/*************************************

    Driver Init

*************************************/

static void snes_init_timers( running_machine &machine )
{
	snes_state *state = machine.driver_data<snes_state>();

	/* init timers and stop them */
	state->m_scanline_timer = machine.scheduler().timer_alloc(FUNC(snes_scanline_tick));
	state->m_scanline_timer->adjust(attotime::never);
	state->m_hblank_timer = machine.scheduler().timer_alloc(FUNC(snes_hblank_tick));
	state->m_hblank_timer->adjust(attotime::never);
	state->m_nmi_timer = machine.scheduler().timer_alloc(FUNC(snes_nmi_tick));
	state->m_nmi_timer->adjust(attotime::never);
	state->m_hirq_timer = machine.scheduler().timer_alloc(FUNC(snes_hirq_tick_callback));
	state->m_hirq_timer->adjust(attotime::never);
	state->m_div_timer = machine.scheduler().timer_alloc(FUNC(snes_div_callback));
	state->m_div_timer->adjust(attotime::never);
	state->m_mult_timer = machine.scheduler().timer_alloc(FUNC(snes_mult_callback));
	state->m_mult_timer->adjust(attotime::never);
	state->m_io_timer = machine.scheduler().timer_alloc(FUNC(snes_update_io));
	state->m_io_timer->adjust(attotime::never);

	// SNES hcounter has a 0-339 range.  hblank starts at counter 260.
	// clayfighter sets an HIRQ at 260, apparently it wants it to be before hdma kicks off, so we'll delay 2 pixels.
	state->m_hblank_offset = 274;
	state->m_hblank_timer->adjust(machine.primary_screen->time_until_pos(((snes_ram[STAT78] & 0x10) == SNES_NTSC) ? SNES_VTOTAL_NTSC - 1 : SNES_VTOTAL_PAL - 1, state->m_hblank_offset));
}

static void snes_init_ram( running_machine &machine )
{
	snes_state *state = machine.driver_data<snes_state>();
	address_space *cpu0space = machine.device("maincpu")->memory().space(AS_PROGRAM);
	int i;

	/* Init work RAM - 0x55 isn't exactly right but it's close */
	/* make sure it happens to the 65816 (CPU 0) */
	for (i = 0; i < (128*1024); i++)
	{
		cpu0space->write_byte(0x7e0000 + i, 0x55);
	}

	/* Inititialize registers/variables */
	state->m_cgram_address = 0;
	state->m_vram_read_offset = 2;
	state->m_read_ophct = 0;
	state->m_read_opvct = 0;

	state->m_joy1l = state->m_joy1h = state->m_joy2l = state->m_joy2h = state->m_joy3l = state->m_joy3h = 0;
	state->m_data1[0] = state->m_data2[0] = state->m_data1[1] = state->m_data2[1] = 0;

	state->m_io_read = nss_io_read;
	state->m_oldjoy1_read = nss_oldjoy1_read;
	state->m_oldjoy2_read = nss_oldjoy2_read;

	// set up some known register power-up defaults
	snes_ram[WRIO] = 0xff;
	snes_ram[VMAIN] = 0x80;

	// see if there's a uPD7725 DSP in the machine config
	state->m_upd7725 = machine.device<upd7725_device>("dsp");

	// if we have a DSP, halt it for the moment
	if (state->m_upd7725)
	{
		cputag_set_input_line(machine, "dsp", INPUT_LINE_RESET, ASSERT_LINE);
	}

	// ditto for a uPD96050 (Seta ST-010 or ST-011)
	state->m_upd96050 = machine.device<upd96050_device>("setadsp");
	if (state->m_upd96050)
	{
		cputag_set_input_line(machine, "setadsp", INPUT_LINE_RESET, ASSERT_LINE);
	}

	switch (state->m_has_addon_chip)
	{
		case HAS_DSP1:
		case HAS_DSP2:
		case HAS_DSP3:
		case HAS_DSP4:
			// cartridge uses the DSP, let 'er rip
			if (state->m_upd7725)
			{
				cputag_set_input_line(machine, "dsp", INPUT_LINE_RESET, CLEAR_LINE);
			}
			else
			{
				logerror("SNES: Game uses a DSP, but the machine driver is missing the uPD7725!\n");
				state->m_has_addon_chip = HAS_NONE;	// prevent crash trying to access NULL device
			}
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
		case HAS_ST011:
			// cartridge uses the DSP, let 'er rip
			if (state->m_upd96050)
			{
				cputag_set_input_line(machine, "setadsp", INPUT_LINE_RESET, CLEAR_LINE);
			}
			else
			{
				logerror("SNES: Game uses a Seta DSP, but the machine driver is missing the uPD96050!\n");
				state->m_has_addon_chip = HAS_NONE;	// prevent crash trying to access NULL device
			}
			break;

		default:
			break;
	}

	// init frame counter so first line is 0
	if (ATTOSECONDS_TO_HZ(machine.primary_screen->frame_period().attoseconds) >= 59)
		snes_ppu.beam.current_vert = SNES_VTOTAL_NTSC;
	else
		snes_ppu.beam.current_vert = SNES_VTOTAL_PAL;
}


DIRECT_UPDATE_HANDLER( snes_spc_direct )
{
	direct.explicit_configure(0x0000, 0xffff, 0xffff, spc_get_ram(machine.device("spc700")));
	return ~0;
}

DIRECT_UPDATE_HANDLER( snes_direct )
{
	direct.explicit_configure(0x0000, 0xffff, 0xffff, snes_ram);
	return ~0;
}

MACHINE_START( snes )
{
	snes_state *state = machine.driver_data<snes_state>();
	int i;

	state->m_maincpu = machine.device<_5a22_device>("maincpu");
	state->m_soundcpu = machine.device<spc700_device>("soundcpu");
	state->m_spc700 = machine.device<snes_sound_device>("spc700");
	state->m_superfx = machine.device<cpu_device>("superfx");

	state->m_maincpu->space(AS_PROGRAM)->set_direct_update_handler(direct_update_delegate(FUNC(snes_direct), &machine));
	state->m_soundcpu->space(AS_PROGRAM)->set_direct_update_handler(direct_update_delegate(FUNC(snes_spc_direct), &machine));

	// power-on sets these registers like this
	snes_ram[WRIO] = 0xff;
	snes_ram[WRMPYA] = 0xff;
	snes_ram[WRDIVL] = 0xff;
	snes_ram[WRDIVH] = 0xff;

	switch (state->m_has_addon_chip)
	{
		case HAS_SDD1:
			sdd1_init(machine);
			break;
		case HAS_SPC7110:
			spc7110_init(machine);
			break;
		case HAS_SPC7110_RTC:
			spc7110rtc_init(machine);
			break;
	}

	if (state->m_cart[0].mode == SNES_MODE_BSX)
		bsx_init(machine);

	snes_init_timers(machine);

	for (i = 0; i < 6; i++)
	{
		state_save_register_item(machine, "snes_dma", NULL, i, state->m_dma_channel[i].dmap);
		state_save_register_item(machine, "snes_dma", NULL, i, state->m_dma_channel[i].dest_addr);
		state_save_register_item(machine, "snes_dma", NULL, i, state->m_dma_channel[i].src_addr);
		state_save_register_item(machine, "snes_dma", NULL, i, state->m_dma_channel[i].bank);
		state_save_register_item(machine, "snes_dma", NULL, i, state->m_dma_channel[i].trans_size);
		state_save_register_item(machine, "snes_dma", NULL, i, state->m_dma_channel[i].ibank);
		state_save_register_item(machine, "snes_dma", NULL, i, state->m_dma_channel[i].hdma_addr);
		state_save_register_item(machine, "snes_dma", NULL, i, state->m_dma_channel[i].hdma_line_counter);
		state_save_register_item(machine, "snes_dma", NULL, i, state->m_dma_channel[i].unk);
		state_save_register_item(machine, "snes_dma", NULL, i, state->m_dma_channel[i].do_transfer);
		state_save_register_item(machine, "snes_dma", NULL, i, state->m_dma_channel[i].dma_disabled);
	}

	state->save_item(NAME(state->m_htmult));
	state->save_item(NAME(state->m_cgram_address));
	state->save_item(NAME(state->m_vram_read_offset));
	state->save_item(NAME(state->m_read_ophct));
	state->save_item(NAME(state->m_read_opvct));
	state->save_item(NAME(state->m_hblank_offset));
	state->save_item(NAME(state->m_vram_fgr_high));
	state->save_item(NAME(state->m_vram_fgr_increment));
	state->save_item(NAME(state->m_vram_fgr_count));
	state->save_item(NAME(state->m_vram_fgr_mask));
	state->save_item(NAME(state->m_vram_fgr_shift));
	state->save_item(NAME(state->m_vram_read_buffer));
	state->save_item(NAME(state->m_wram_address));
	state->save_item(NAME(state->m_htime));
	state->save_item(NAME(state->m_vtime));
	state->save_item(NAME(state->m_vmadd));
	state->save_item(NAME(state->m_hdmaen));
	state->save_item(NAME(state->m_joy1l));
	state->save_item(NAME(state->m_joy1h));
	state->save_item(NAME(state->m_joy2l));
	state->save_item(NAME(state->m_joy2h));
	state->save_item(NAME(state->m_joy3l));
	state->save_item(NAME(state->m_joy3h));
	state->save_item(NAME(state->m_joy4l));
	state->save_item(NAME(state->m_joy4h));
	state->save_item(NAME(state->m_data1));
	state->save_item(NAME(state->m_data2));
	state->save_item(NAME(state->m_read_idx));

	for (i = 0; i < 2; i++)
	{
		state_save_register_item(machine, "snes_dma", NULL, i, state->m_joypad[i].buttons);
		state_save_register_item(machine, "snes_dma", NULL, i, state->m_mouse[i].x);
		state_save_register_item(machine, "snes_dma", NULL, i, state->m_mouse[i].oldx);
		state_save_register_item(machine, "snes_dma", NULL, i, state->m_mouse[i].y);
		state_save_register_item(machine, "snes_dma", NULL, i, state->m_mouse[i].oldy);
		state_save_register_item(machine, "snes_dma", NULL, i, state->m_mouse[i].buttons);
		state_save_register_item(machine, "snes_dma", NULL, i, state->m_mouse[i].deltax);
		state_save_register_item(machine, "snes_dma", NULL, i, state->m_mouse[i].deltay);
		state_save_register_item(machine, "snes_dma", NULL, i, state->m_mouse[i].speed);
		state_save_register_item(machine, "snes_dma", NULL, i, state->m_scope[i].x);
		state_save_register_item(machine, "snes_dma", NULL, i, state->m_scope[i].y);
		state_save_register_item(machine, "snes_dma", NULL, i, state->m_scope[i].buttons);
		state_save_register_item(machine, "snes_dma", NULL, i, state->m_scope[i].turbo_lock);
		state_save_register_item(machine, "snes_dma", NULL, i, state->m_scope[i].pause_lock);
		state_save_register_item(machine, "snes_dma", NULL, i, state->m_scope[i].fire_lock);
		state_save_register_item(machine, "snes_dma", NULL, i, state->m_scope[i].offscreen);
	}
}

MACHINE_RESET( snes )
{
	snes_state *state = machine.driver_data<snes_state>();
	int i;

	snes_init_ram(machine);

	/* init DMA regs to be 0xff */
	for(i = 0; i < 8; i++)
	{
		state->m_dma_channel[i].dmap = 0xff;
		state->m_dma_channel[i].dest_addr = 0xff;
		state->m_dma_channel[i].src_addr = 0xffff;
		state->m_dma_channel[i].bank = 0xff;
		state->m_dma_channel[i].trans_size = 0xffff;
		state->m_dma_channel[i].ibank = 0xff;
		state->m_dma_channel[i].hdma_addr = 0xffff;
		state->m_dma_channel[i].hdma_line_counter = 0xff;
		state->m_dma_channel[i].unk = 0xff;
	}

	/* Set STAT78 to NTSC or PAL */
	if (ATTOSECONDS_TO_HZ(machine.primary_screen->frame_period().attoseconds) >= 59.0f)
		snes_ram[STAT78] = SNES_NTSC;
	else /* if (ATTOSECONDS_TO_HZ(machine.primary_screen->frame_period().attoseconds) == 50.0f) */
		snes_ram[STAT78] = SNES_PAL;

	// reset does this to these registers
	snes_ram[NMITIMEN] = 0;
	state->m_htime = 0x1ff;
	state->m_vtime = 0x1ff;

	state->m_htmult = 1;
	snes_ppu.interlace = 1;
	snes_ppu.obj_interlace = 1;
}


/* for mame we use an init, maybe we will need more for the different games */
DRIVER_INIT( snes )
{
	snes_state *state = machine.driver_data<snes_state>();
	address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);
	UINT16 total_blocks, read_blocks;
	UINT8 *rom;

	rom = machine.region("user3")->base();
	snes_ram = auto_alloc_array_clear(machine, UINT8, 0x1400000);

	/* all NSS games seem to use MODE 20 */
	state->m_cart[0].mode = SNES_MODE_20;
	state->m_cart[0].sram_max = 0x40000;
	state->m_has_addon_chip = HAS_NONE;

	/* Find the number of blocks in this ROM */
	total_blocks = (machine.region("user3")->bytes() / 0x8000);
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
	state->m_cart[0].sram = snes_r_bank1(space, 0x00ffd8);
	if (state->m_cart[0].sram > 0)
	{
		state->m_cart[0].sram = (1024 << state->m_cart[0].sram);
		if (state->m_cart[0].sram > state->m_cart[0].sram_max)
			state->m_cart[0].sram = state->m_cart[0].sram_max;
	}
}

DRIVER_INIT( snes_hirom )
{
	snes_state *state = machine.driver_data<snes_state>();
	address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);
	UINT16 total_blocks, read_blocks;
	UINT8  *rom;

	rom = machine.region("user3")->base();
	snes_ram = auto_alloc_array(machine, UINT8, 0x1400000);
	memset(snes_ram, 0, 0x1400000);

	state->m_cart[0].mode = SNES_MODE_21;
	state->m_cart[0].sram_max = 0x40000;
	state->m_has_addon_chip = HAS_NONE;

	/* Find the number of blocks in this ROM */
	total_blocks = (machine.region("user3")->bytes() / 0x10000);
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
	state->m_cart[0].sram = snes_r_bank1(space, 0x00ffd8);
	if (state->m_cart[0].sram > 0)
	{
		state->m_cart[0].sram = (1024 << state->m_cart[0].sram);
		if (state->m_cart[0].sram > state->m_cart[0].sram_max)
			state->m_cart[0].sram = state->m_cart[0].sram_max;
	}
}


/*************************************

    HDMA

*************************************/

INLINE int dma_abus_valid( UINT32 address )
{
	if((address & 0x40ff00) == 0x2100) return 0;  //$[00-3f|80-bf]:[2100-21ff]
	if((address & 0x40fe00) == 0x4000) return 0;  //$[00-3f|80-bf]:[4000-41ff]
	if((address & 0x40ffe0) == 0x4200) return 0;  //$[00-3f|80-bf]:[4200-421f]
	if((address & 0x40ff80) == 0x4300) return 0;  //$[00-3f|80-bf]:[4300-437f]

	return 1;
}

INLINE UINT8 snes_abus_read( address_space *space, UINT32 abus )
{
	if (!dma_abus_valid(abus))
		return 0;

	return space->read_byte(abus);
}

INLINE void snes_dma_transfer( address_space *space, UINT8 dma, UINT32 abus, UINT16 bbus )
{
	snes_state *state = space->machine().driver_data<snes_state>();

	#if USE_CYCLE_STEAL
	/* every byte transfer takes 8 master cycles */
//  FIXME: this cycle steal makes Final Fantasy VI (III in US) very glitchy!
//  device_adjust_icount(&space->device(),-8);
	#endif

	if (state->m_dma_channel[dma].dmap & 0x80)	/* PPU->CPU */
	{
		if (bbus == 0x2180 && ((abus & 0xfe0000) == 0x7e0000 || (abus & 0x40e000) == 0x0000))
		{
			//illegal WRAM->WRAM transfer (bus conflict)
			//no read occurs; write does occur
			space->write_byte(abus, 0x00);
			return;
		}
		else
		{
			if (!dma_abus_valid(abus))
				return;

			space->write_byte(abus, space->read_byte(bbus));
			return;
		}
	}
	else									/* CPU->PPU */
	{
		if (bbus == 0x2180 && ((abus & 0xfe0000) == 0x7e0000 || (abus & 0x40e000) == 0x0000))
		{
			//illegal WRAM->WRAM transfer (bus conflict)
			//read most likely occurs; no write occurs
			//read is irrelevant, as it cannot be observed by software
			return;
		}
		else
		{
			space->write_byte(bbus, snes_abus_read(space, abus));
			return;
		}
	}
}

/* WIP: These have the advantage to automatically update the address, but then we would need to
check again if the transfer is direct/indirect at each step... is it worth? */
INLINE UINT32 snes_get_hdma_addr( running_machine &machine, int dma )
{
	snes_state *state = machine.driver_data<snes_state>();
	return (state->m_dma_channel[dma].bank << 16) | (state->m_dma_channel[dma].hdma_addr++);
}

INLINE UINT32 snes_get_hdma_iaddr( running_machine &machine, int dma )
{
	snes_state *state = machine.driver_data<snes_state>();
	return (state->m_dma_channel[dma].ibank << 16) | (state->m_dma_channel[dma].trans_size++);
}

INLINE int is_last_active_channel( running_machine &machine, int dma )
{
	snes_state *state = machine.driver_data<snes_state>();
	int i;

	for (i = dma + 1; i < 8; i++)
	{
		if (BIT(state->m_hdmaen, i) && state->m_dma_channel[i].hdma_line_counter)
			return 0;	// there is still at least another channel with incomplete HDMA
	}

	// if we arrive here, all hdma transfers from (dma + 1) to 7 are completed or not active
	return 1;
}

static void snes_hdma_update( address_space *space, int dma )
{
	snes_state *state = space->machine().driver_data<snes_state>();
	UINT32 abus = snes_get_hdma_addr(space->machine(), dma);

	state->m_dma_channel[dma].hdma_line_counter = snes_abus_read(space, abus);

	if (state->m_dma_channel[dma].dmap & 0x40)
	{
		/* One oddity: if $43xA is 0 and this is the last active HDMA channel for this scanline, only load
        one byte for Address, and use the $00 for the low byte. So Address ends up incremented one less than
        otherwise expected */

		abus = snes_get_hdma_addr(space->machine(), dma);
		state->m_dma_channel[dma].trans_size = snes_abus_read(space, abus) << 8;

		if (state->m_dma_channel[dma].hdma_line_counter || !is_last_active_channel(space->machine(), dma))
		{
			// we enter here if we have more transfers to be done or if there are other active channels after this one
			abus = snes_get_hdma_addr(space->machine(), dma);
			state->m_dma_channel[dma].trans_size >>= 8;
			state->m_dma_channel[dma].trans_size |= snes_abus_read(space, abus) << 8;
		}
	}

	if (!state->m_dma_channel[dma].hdma_line_counter)
		state->m_hdmaen &= ~(1 << dma);

	state->m_dma_channel[dma].do_transfer = 1;
}

static void snes_hdma_init( address_space *space )
{
	snes_state *state = space->machine().driver_data<snes_state>();
	int i;

	state->m_hdmaen = snes_ram[HDMAEN];
	for (i = 0; i < 8; i++)
	{
		if (BIT(state->m_hdmaen, i))
		{
			state->m_dma_channel[i].hdma_addr = state->m_dma_channel[i].src_addr;
			snes_hdma_update(space, i);
		}
	}
}

static void snes_hdma( address_space *space )
{
	snes_state *state = space->machine().driver_data<snes_state>();
	UINT16 bbus;
	UINT32 abus;
	int i;

	for (i = 0; i < 8; i++)
	{
		if (BIT(state->m_hdmaen, i))
		{
			if (state->m_dma_channel[i].do_transfer)
			{
				/* Get transfer addresses */
				if (state->m_dma_channel[i].dmap & 0x40)	/* Indirect */
					abus = (state->m_dma_channel[i].ibank << 16) + state->m_dma_channel[i].trans_size;
				else									/* Absolute */
					abus = (state->m_dma_channel[i].bank << 16) + state->m_dma_channel[i].hdma_addr;

				bbus = state->m_dma_channel[i].dest_addr + 0x2100;



				switch (state->m_dma_channel[i].dmap & 0x07)
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
					mame_printf_debug( "  HDMA of unsupported type: %d\n", state->m_dma_channel[i].dmap & 0x07);
#endif
					break;
				}

				if (state->m_dma_channel[i].dmap & 0x40)	/* Indirect */
					state->m_dma_channel[i].trans_size = abus;
				else									/* Absolute */
					state->m_dma_channel[i].hdma_addr = abus;

			}
		}
	}

	for (i = 0; i < 8; i++)
	{
		if (BIT(state->m_hdmaen, i))
		{
			state->m_dma_channel[i].do_transfer = (--state->m_dma_channel[i].hdma_line_counter) & 0x80;

			if (!(state->m_dma_channel[i].hdma_line_counter & 0x7f))
				snes_hdma_update(space, i);
		}
	}
}

static void snes_dma( address_space *space, UINT8 channels )
{
	snes_state *state = space->machine().driver_data<snes_state>();
	int i;
	INT8 increment;
	UINT16 bbus;
	UINT32 abus, abus_bank;
	UINT16 length;

	/* FIXME: we also need to round to the nearest 8 master cycles */

	#if USE_CYCLE_STEAL
	/* overhead steals 8 master cycles, correct? */
	device_adjust_icount(&space->device(),-8);
	#endif

	/* Assume priority of the 8 DMA channels is 0-7 */
	for (i = 0; i < 8; i++)
	{
		if (BIT(channels, i))
		{
			/* FIXME: the following should be used to stop DMA if the same channel is used by HDMA (being set to 1 in snes_hdma)
             However, this cannot be implemented as is atm, because currently DMA transfers always happen as soon as they are enabled... */
			state->m_dma_channel[i].dma_disabled = 0;

			//printf( "Making a transfer on channel %d\n", i );
			/* Find transfer addresses */
			abus = state->m_dma_channel[i].src_addr;
			abus_bank = state->m_dma_channel[i].bank << 16;
			bbus = state->m_dma_channel[i].dest_addr + 0x2100;

			//printf("Address: %06x\n", abus | abus_bank);
			/* Auto increment */
			if (state->m_dma_channel[i].dmap & 0x8)
				increment = 0;
			else
			{
				if (state->m_dma_channel[i].dmap & 0x10)
					increment = -1;
				else
					increment = 1;
			}

			/* Number of bytes to transfer */
			length = state->m_dma_channel[i].trans_size;

//          printf( "DMA-Ch %d: len: %X, abus: %X, bbus: %X, incr: %d, dir: %s, type: %d\n", i, length, abus | abus_bank, bbus, increment, state->m_dma_channel[i].dmap & 0x80 ? "PPU->CPU" : "CPU->PPU", state->m_dma_channel[i].dmap & 0x07);

#ifdef SNES_DBG_DMA
			mame_printf_debug( "DMA-Ch %d: len: %X, abus: %X, bbus: %X, incr: %d, dir: %s, type: %d\n", i, length, abus | abus_bank, bbus, increment, state->m_dma_channel[i].dmap & 0x80 ? "PPU->CPU" : "CPU->PPU", state->m_dma_channel[i].dmap & 0x07);
#endif

			switch (state->m_dma_channel[i].dmap & 0x07)
			{
				case 0:		/* 1 register write once */
				case 2:		/* 1 register write twice */
				case 6:		/* 1 register write twice */
					do
					{
						snes_dma_transfer(space, i, (abus & 0xffff) | abus_bank, bbus);
						abus += increment;
					} while (--length && !state->m_dma_channel[i].dma_disabled);
					break;
				case 1:		/* 2 registers write once */
				case 5:		/* 2 registers write twice alternate */
					do
					{
						snes_dma_transfer(space, i, (abus & 0xffff) | abus_bank, bbus);
						abus += increment;
						if (!(--length) || state->m_dma_channel[i].dma_disabled)
							break;
						snes_dma_transfer(space, i, (abus & 0xffff) | abus_bank, bbus + 1);
						abus += increment;
					} while (--length && !state->m_dma_channel[i].dma_disabled);
					break;
				case 3:		/* 2 registers write twice each */
				case 7:		/* 2 registers write twice each */
					do
					{
						snes_dma_transfer(space, i, (abus & 0xffff) | abus_bank, bbus);
						abus += increment;
						if (!(--length) || state->m_dma_channel[i].dma_disabled)
							break;
						snes_dma_transfer(space, i, (abus & 0xffff) | abus_bank, bbus);
						abus += increment;
						if (!(--length) || state->m_dma_channel[i].dma_disabled)
							break;
						snes_dma_transfer(space, i, (abus & 0xffff) | abus_bank, bbus + 1);
						abus += increment;
						if (!(--length) || state->m_dma_channel[i].dma_disabled)
							break;
						snes_dma_transfer(space, i, (abus & 0xffff) | abus_bank, bbus + 1);
						abus += increment;
					} while (--length && !state->m_dma_channel[i].dma_disabled);
					break;
				case 4:		/* 4 registers write once */
					do
					{
						snes_dma_transfer(space, i, (abus & 0xffff) | abus_bank, bbus);
						abus += increment;
						if (!(--length) || state->m_dma_channel[i].dma_disabled)
							break;
						snes_dma_transfer(space, i, (abus & 0xffff) | abus_bank, bbus + 1);
						abus += increment;
						if (!(--length) || state->m_dma_channel[i].dma_disabled)
							break;
						snes_dma_transfer(space, i, (abus & 0xffff) | abus_bank, bbus + 2);
						abus += increment;
						if (!(--length) || state->m_dma_channel[i].dma_disabled)
							break;
						snes_dma_transfer(space, i, (abus & 0xffff) | abus_bank, bbus + 3);
						abus += increment;
					} while (--length && !state->m_dma_channel[i].dma_disabled);
					break;
				default:
#ifdef MAME_DEBUG
					mame_printf_debug("  DMA of unsupported type: %d\n", state->m_dma_channel[i].dmap & 0x07);
#endif
					break;
			}

			/* We're done, so write the new abus back to the registers */
			state->m_dma_channel[i].src_addr = abus;
			state->m_dma_channel[i].trans_size = 0;

			#if USE_CYCLE_STEAL
			/* active channel takes 8 master cycles */
			device_adjust_icount(&space->device(),-8);
			#endif
		}
	}

	/* finally, take yet another 8 master cycles for the aforementioned overhead */
	#if USE_CYCLE_STEAL
	device_adjust_icount(&space->device(),-8);
	#endif
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
	/* IMPORTANT: SFX RAM sits in 0x600000-0x7fffff, and it's mirrored in 0xe00000-0xffffff. However, SNES
    has only access to 0x600000-0x7dffff (because there is WRAM after that), hence we directly use the mirror
    as the place where to write & read SFX RAM. SNES handlers have been setup accordingly. */
	//printf("superfx_r_bank3: %08x = %02x\n", offset, snes_ram[0xe00000 + offset]);
	return snes_ram[0xe00000 + offset];
}

WRITE8_HANDLER( superfx_w_bank1 )
{
	printf("Attempting to write to cart ROM: %08x = %02x\n", offset, data);
	// Do nothing; can't write to cart ROM.
}

WRITE8_HANDLER( superfx_w_bank2 )
{
	printf("Attempting to write to cart ROM: %08x = %02x\n", 0x400000 + offset, data);
	// Do nothing; can't write to cart ROM.
}

WRITE8_HANDLER( superfx_w_bank3 )
{
	/* IMPORTANT: SFX RAM sits in 0x600000-0x7fffff, and it's mirrored in 0xe00000-0xffffff. However, SNES
    has only access to 0x600000-0x7dffff (because there is WRAM after that), hence we directly use the mirror
    as the place where to write & read SFX RAM. SNES handlers have been setup accordingly. */
	//printf("superfx_w_bank3: %08x = %02x\n", offset, data);
	snes_ram[0xe00000 + offset] = data;
}
