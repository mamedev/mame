// license:BSD-3-Clause
// copyright-holders:Angelo Salese, R. Belmont, Anthony Kruize, Fabio Priuli, Ryan Holtz
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

  TODO:
  - DMA takes some Master CPU clock cycles that aren't taken into account
    for now.

***************************************************************************/
#define __MACHINE_SNES_C

#include "emu.h"
#include "includes/snes.h"


#define DMA_REG(a) m_dma_regs[a - 0x4300]   // regs 0x4300-0x437f

UINT32 snes_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	/* NTSC SNES draw range is 1-225. */
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
		m_ppu->refresh_scanline(bitmap, y + 1);

	return 0;
}


/*************************************

    Timers

*************************************/
void snes_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_NMI_TICK:
		snes_nmi_tick(ptr, param);
		break;
	case TIMER_HIRQ_TICK:
		snes_hirq_tick_callback(ptr, param);
		break;
	case TIMER_RESET_OAM_ADDRESS:
		snes_reset_oam_address(ptr, param);
		break;
	case TIMER_RESET_HDMA:
		snes_reset_hdma(ptr, param);
		break;
	case TIMER_UPDATE_IO:
		snes_update_io(ptr, param);
		break;
	case TIMER_SCANLINE_TICK:
		snes_scanline_tick(ptr, param);
		break;
	case TIMER_HBLANK_TICK:
		snes_hblank_tick(ptr, param);
		break;
	default:
		assert_always(FALSE, "Unknown id in snes_state::device_timer");
	}
}


TIMER_CALLBACK_MEMBER(snes_state::snes_nmi_tick)
{
	// pull NMI
	m_maincpu->set_input_line(G65816_LINE_NMI, ASSERT_LINE);

	// don't happen again
	m_nmi_timer->adjust(attotime::never);
}

void snes_state::hirq_tick()
{
	// latch the counters and pull IRQ
	// (don't need to switch to the 65816 context, we don't do anything dependant on it)
	m_ppu->set_latch_hv(m_ppu->current_x(), m_ppu->current_y());
	SNES_CPU_REG(TIMEUP) = 0x80;    /* Indicate that irq occurred */
	m_maincpu->set_input_line(G65816_LINE_IRQ, ASSERT_LINE);

	// don't happen again
	m_hirq_timer->adjust(attotime::never);
}

TIMER_CALLBACK_MEMBER(snes_state::snes_hirq_tick_callback)
{
	hirq_tick();
}

TIMER_CALLBACK_MEMBER(snes_state::snes_reset_oam_address)
{
	// make sure we're in the 65816's context since we're messing with the OAM and stuff
	address_space &space = m_maincpu->space(AS_PROGRAM);

	if (!(m_ppu->m_screen_disabled)) //Reset OAM address, byuu says it happens at H=10
	{
		space.write_byte(OAMADDL, m_ppu->m_oam.saved_address_low); /* Reset oam address */
		space.write_byte(OAMADDH, m_ppu->m_oam.saved_address_high);
		m_ppu->m_oam.first_sprite = m_ppu->m_oam.priority_rotation ? (m_ppu->m_oam.address >> 1) & 127 : 0;
	}
}

TIMER_CALLBACK_MEMBER(snes_state::snes_reset_hdma)
{
	address_space &cpu0space = m_maincpu->space(AS_PROGRAM);
	hdma_init(cpu0space);
}

TIMER_CALLBACK_MEMBER(snes_state::snes_update_io)
{
	io_read(m_maincpu->space(AS_PROGRAM),0,0,0);
	SNES_CPU_REG(HVBJOY) &= 0xfe;       /* Clear busy bit */

	m_io_timer->adjust(attotime::never);
}

TIMER_CALLBACK_MEMBER(snes_state::snes_scanline_tick)
{
	/* Increase current line - we want to latch on this line during it, not after it */
	m_ppu->m_beam.current_vert = m_screen->vpos();

	// not in hblank
	SNES_CPU_REG(HVBJOY) &= ~0x40;

	/* Vertical IRQ timer - only if horizontal isn't also enabled! */
	if ((SNES_CPU_REG(NMITIMEN) & 0x20) && !(SNES_CPU_REG(NMITIMEN) & 0x10))
	{
		if (m_ppu->m_beam.current_vert == m_vtime)
		{
			SNES_CPU_REG(TIMEUP) = 0x80;    /* Indicate that irq occurred */
			// IRQ latches the counters, do it now
			m_ppu->set_latch_hv(m_ppu->current_x(), m_ppu->current_y());
			m_maincpu->set_input_line(G65816_LINE_IRQ, ASSERT_LINE );
		}
	}
	/* Horizontal IRQ timer */
	if (SNES_CPU_REG(NMITIMEN) & 0x10)
	{
		int setirq = 1;
		int pixel = m_htime;

		// is the HIRQ on a specific scanline?
		if (SNES_CPU_REG(NMITIMEN) & 0x20)
		{
			if (m_ppu->m_beam.current_vert != m_vtime)
			{
				setirq = 0;
			}
		}

		if (setirq)
		{
//          printf("HIRQ @ %d, %d\n", pixel * m_ppu->m_htmult, m_ppu->m_beam.current_vert);
			if (pixel == 0)
			{
				hirq_tick();
			}
			else
			{
				m_hirq_timer->adjust(m_screen->time_until_pos(m_ppu->m_beam.current_vert, pixel * m_ppu->m_htmult));
			}
		}
	}

	/* Start of VBlank */
	if (m_ppu->m_beam.current_vert == m_ppu->m_beam.last_visible_line)
	{
		timer_set(m_screen->time_until_pos(m_ppu->m_beam.current_vert, 10), TIMER_RESET_OAM_ADDRESS);

		SNES_CPU_REG(HVBJOY) |= 0x81;       /* Set vblank bit to on & indicate controllers being read */
		SNES_CPU_REG(RDNMI) |= 0x80;        /* Set NMI occurred bit */

		if (SNES_CPU_REG(NMITIMEN) & 0x80)  /* NMI only signaled if this bit set */
		{
			// NMI goes off about 12 cycles after this (otherwise Chrono Trigger, NFL QB Club, etc. lock up)
			m_nmi_timer->adjust(m_maincpu->cycles_to_attotime(12));
		}

		/* three lines after start of vblank we update the controllers (value from snes9x) */
		m_io_timer->adjust(m_screen->time_until_pos(m_ppu->m_beam.current_vert + 2, m_hblank_offset * m_ppu->m_htmult));
	}

	// hdma reset happens at scanline 0, H=~6
	if (m_ppu->m_beam.current_vert == 0)
	{
		address_space &cpu0space = m_maincpu->space(AS_PROGRAM);
		hdma_init(cpu0space);
	}

	if (m_ppu->m_beam.current_vert == 0)
	{   /* VBlank is over, time for a new frame */
		SNES_CPU_REG(HVBJOY) &= 0x7f;       /* Clear vblank bit */
		SNES_CPU_REG(RDNMI)  &= 0x7f;       /* Clear nmi occurred bit */
		m_ppu->m_stat78 ^= 0x80;       /* Toggle field flag */
		m_ppu->m_stat77 &= 0x3f;  /* Clear Time Over and Range Over bits */

		m_maincpu->set_input_line(G65816_LINE_NMI, CLEAR_LINE );
	}

	m_scanline_timer->adjust(attotime::never);
	m_hblank_timer->adjust(m_screen->time_until_pos(m_ppu->m_beam.current_vert, m_hblank_offset * m_ppu->m_htmult));

//  printf("%02x %d\n",SNES_CPU_REG(HVBJOY),m_ppu->m_beam.current_vert);
}

/* This is called at the start of hblank *before* the scanline indicated in current_vert! */
TIMER_CALLBACK_MEMBER(snes_state::snes_hblank_tick)
{
	address_space &cpu0space = m_maincpu->space(AS_PROGRAM);
	int nextscan;

	m_ppu->m_beam.current_vert = m_screen->vpos();

	/* make sure we halt */
	m_hblank_timer->adjust(attotime::never);

	/* draw a scanline */
	if (m_ppu->m_beam.current_vert <= m_ppu->m_beam.last_visible_line)
	{
		if (m_screen->vpos() > 0)
		{
			/* Do HDMA */
			if (SNES_CPU_REG(HDMAEN))
				hdma(cpu0space);

			m_screen->update_partial((m_ppu->m_interlace == 2) ? (m_ppu->m_beam.current_vert * m_ppu->m_interlace) : m_ppu->m_beam.current_vert - 1);
		}
	}

	// signal hblank
	SNES_CPU_REG(HVBJOY) |= 0x40;

	/* kick off the start of scanline timer */
	nextscan = m_ppu->m_beam.current_vert + 1;
	if (nextscan >= (((m_ppu->m_stat78 & 0x10) == SNES_NTSC) ? SNES_VTOTAL_NTSC : SNES_VTOTAL_PAL))
	{
		nextscan = 0;
	}

	m_scanline_timer->adjust(m_screen->time_until_pos(nextscan));
}


/*************************************

    Input Handlers

*************************************/

READ8_MEMBER( snes_state::snes_open_bus_r )
{
	static UINT8 recurse = 0;
	UINT16 result;

	/* prevent recursion */
	if (recurse)
		return 0xff;

	recurse = 1;
	result = space.read_byte(space.device().safe_pc() - 1); //LAST opcode that's fetched on the bus
	recurse = 0;
	return result;
}

/* read & write to DMA addresses are defined separately, to be called by snessdd1 handlers */
READ8_MEMBER( snes_state::snes_io_dma_r )
{
	switch (offset)
	{
		case DMAP0: case DMAP1: case DMAP2: case DMAP3: /*0x43n0*/
		case DMAP4: case DMAP5: case DMAP6: case DMAP7:
			return m_dma_channel[(offset >> 4) & 0x07].dmap;
		case BBAD0: case BBAD1: case BBAD2: case BBAD3: /*0x43n1*/
		case BBAD4: case BBAD5: case BBAD6: case BBAD7:
			return m_dma_channel[(offset >> 4) & 0x07].dest_addr;
		case A1T0L: case A1T1L: case A1T2L: case A1T3L: /*0x43n2*/
		case A1T4L: case A1T5L: case A1T6L: case A1T7L:
			return m_dma_channel[(offset >> 4) & 0x07].src_addr & 0xff;
		case A1T0H: case A1T1H: case A1T2H: case A1T3H: /*0x43n3*/
		case A1T4H: case A1T5H: case A1T6H: case A1T7H:
			return (m_dma_channel[(offset >> 4) & 0x07].src_addr >> 8) & 0xff;
		case A1B0: case A1B1: case A1B2: case A1B3:     /*0x43n4*/
		case A1B4: case A1B5: case A1B6: case A1B7:
			return m_dma_channel[(offset >> 4) & 0x07].bank;
		case DAS0L: case DAS1L: case DAS2L: case DAS3L: /*0x43n5*/
		case DAS4L: case DAS5L: case DAS6L: case DAS7L:
			return m_dma_channel[(offset >> 4) & 0x07].trans_size & 0xff;
		case DAS0H: case DAS1H: case DAS2H: case DAS3H: /*0x43n6*/
		case DAS4H: case DAS5H: case DAS6H: case DAS7H:
			return (m_dma_channel[(offset >> 4) & 0x07].trans_size >> 8) & 0xff;
		case DSAB0: case DSAB1: case DSAB2: case DSAB3: /*0x43n7*/
		case DSAB4: case DSAB5: case DSAB6: case DSAB7:
			return m_dma_channel[(offset >> 4) & 0x07].ibank;
		case A2A0L: case A2A1L: case A2A2L: case A2A3L: /*0x43n8*/
		case A2A4L: case A2A5L: case A2A6L: case A2A7L:
			return m_dma_channel[(offset >> 4) & 0x07].hdma_addr & 0xff;
		case A2A0H: case A2A1H: case A2A2H: case A2A3H: /*0x43n9*/
		case A2A4H: case A2A5H: case A2A6H: case A2A7H:
			return (m_dma_channel[(offset >> 4) & 0x07].hdma_addr >> 8) & 0xff;
		case NTRL0: case NTRL1: case NTRL2: case NTRL3: /*0x43na*/
		case NTRL4: case NTRL5: case NTRL6: case NTRL7:
			return m_dma_channel[(offset >> 4) & 0x07].hdma_line_counter;
		case 0x430b: case 0x431b: case 0x432b: case 0x433b: /* according to bsnes, this does not return open_bus (even if its precise effect is unknown) */
		case 0x434b: case 0x435b: case 0x436b: case 0x437b:
			return m_dma_channel[(offset >> 4) & 0x07].unk;
	}

	/* we should never arrive here */
	return snes_open_bus_r(space, 0);
}

WRITE8_MEMBER( snes_state::snes_io_dma_w )
{
	switch (offset)
	{
			/* Below is all DMA related */
		case DMAP0: case DMAP1: case DMAP2: case DMAP3: /*0x43n0*/
		case DMAP4: case DMAP5: case DMAP6: case DMAP7:
			m_dma_channel[(offset >> 4) & 0x07].dmap = data;
			break;
		case BBAD0: case BBAD1: case BBAD2: case BBAD3: /*0x43n1*/
		case BBAD4: case BBAD5: case BBAD6: case BBAD7:
			m_dma_channel[(offset >> 4) & 0x07].dest_addr = data;
			break;
		case A1T0L: case A1T1L: case A1T2L: case A1T3L: /*0x43n2*/
		case A1T4L: case A1T5L: case A1T6L: case A1T7L:
			m_dma_channel[(offset >> 4) & 0x07].src_addr = (m_dma_channel[(offset >> 4) & 0x07].src_addr & 0xff00) | (data << 0);
			break;
		case A1T0H: case A1T1H: case A1T2H: case A1T3H: /*0x43n3*/
		case A1T4H: case A1T5H: case A1T6H: case A1T7H:
			m_dma_channel[(offset >> 4) & 0x07].src_addr = (m_dma_channel[(offset >> 4) & 0x07].src_addr & 0x00ff) | (data << 8);
			break;
		case A1B0: case A1B1: case A1B2: case A1B3:     /*0x43n4*/
		case A1B4: case A1B5: case A1B6: case A1B7:
			m_dma_channel[(offset >> 4) & 0x07].bank = data;
			break;
		case DAS0L: case DAS1L: case DAS2L: case DAS3L: /*0x43n5*/
		case DAS4L: case DAS5L: case DAS6L: case DAS7L:
			m_dma_channel[(offset >> 4) & 0x07].trans_size = (m_dma_channel[(offset >> 4) & 0x07].trans_size & 0xff00) | (data << 0);
			break;
		case DAS0H: case DAS1H: case DAS2H: case DAS3H: /*0x43n6*/
		case DAS4H: case DAS5H: case DAS6H: case DAS7H:
			m_dma_channel[(offset >> 4) & 0x07].trans_size = (m_dma_channel[(offset >> 4) & 0x07].trans_size & 0x00ff) | (data << 8);
			break;
		case DSAB0: case DSAB1: case DSAB2: case DSAB3: /*0x43n7*/
		case DSAB4: case DSAB5: case DSAB6: case DSAB7:
			m_dma_channel[(offset >> 4) & 0x07].ibank = data;
			break;
		case A2A0L: case A2A1L: case A2A2L: case A2A3L: /*0x43n8*/
		case A2A4L: case A2A5L: case A2A6L: case A2A7L:
			m_dma_channel[(offset >> 4) & 0x07].hdma_addr = (m_dma_channel[(offset >> 4) & 0x07].hdma_addr & 0xff00) | (data << 0);
			break;
		case A2A0H: case A2A1H: case A2A2H: case A2A3H: /*0x43n9*/
		case A2A4H: case A2A5H: case A2A6H: case A2A7H:
			m_dma_channel[(offset >> 4) & 0x07].hdma_addr = (m_dma_channel[(offset >> 4) & 0x07].hdma_addr & 0x00ff) | (data << 8);
			break;
		case NTRL0: case NTRL1: case NTRL2: case NTRL3: /*0x43na*/
		case NTRL4: case NTRL5: case NTRL6: case NTRL7:
			m_dma_channel[(offset >> 4) & 0x07].hdma_line_counter = data;
			break;
		case 0x430b: case 0x431b: case 0x432b: case 0x433b:
		case 0x434b: case 0x435b: case 0x436b: case 0x437b:
			m_dma_channel[(offset >> 4) & 0x07].unk = data;
			break;
	}

	DMA_REG(offset) = data;
}

/*
 * DR   - Double read : address is read twice to return a 16bit value.
 * low  - This is the low byte of a 16 or 24 bit value
 * mid  - This is the middle byte of a 24 bit value
 * high - This is the high byte of a 16 or 24 bit value
 */
READ8_MEMBER( snes_state::snes_r_io )
{
	UINT8 value = 0;

	// PPU accesses are from 2100 to 213f
	if (offset >= INIDISP && offset < APU00)
	{
		return m_ppu->read(space, offset, SNES_CPU_REG(WRIO) & 0x80);
	}

	// APU is mirrored from 2140 to 217f
	if (offset >= APU00 && offset < WMDATA)
	{
		return m_spc700->spc_port_out(space, offset & 0x3);
	}

	// DMA accesses are from 4300 to 437f
	if (offset >= DMAP0 && offset < 0x4380)
	{
		return snes_io_dma_r(space, offset);
	}


	// other accesses (notice that WRMPYA, WRMPYB, WRDIVL, WRDIVH, WRDVDD, MEMSEL, RDDIVL, RDDIVH, RDMPYL, RDMPYH are handled by the CPU directly)
	switch (offset) // offset is from 0x000000
	{
		case WMDATA:    /* Data to read from WRAM */
			value = space.read_byte(0x7e0000 + m_wram_address++);
			m_wram_address &= 0x1ffff;
			return value;

		case OLDJOY1:   /* Data for old NES controllers (JOYSER1) */
			return (oldjoy1_read(m_oldjoy1_latch & 0x1) & 0x03) | (snes_open_bus_r(space, 0) & 0xfc);

		case OLDJOY2:   /* Data for old NES controllers (JOYSER2) */
			return (oldjoy2_read(m_oldjoy1_latch & 0x1) & 0x03) | 0x1c | (snes_open_bus_r(space, 0) & 0xe0);

		case RDNMI:         /* NMI flag by v-blank and version number */
			value = (SNES_CPU_REG(RDNMI) & 0x80) | (snes_open_bus_r(space, 0) & 0x70);
			SNES_CPU_REG(RDNMI) &= 0x70;   /* NMI flag is reset on read */
			return value | 2; //CPU version number
		case TIMEUP:        /* IRQ flag by H/V count timer */
			value = (snes_open_bus_r(space, 0) & 0x7f) | (SNES_CPU_REG(TIMEUP) & 0x80);
			m_maincpu->set_input_line(G65816_LINE_IRQ, CLEAR_LINE );
			SNES_CPU_REG(TIMEUP) = 0;   // flag is cleared on both read and write
			return value;
		case HVBJOY:        /* H/V blank and joypad controller enable */
			// electronics test says hcounter 272 is start of hblank, which is beampos 363
//          if (m_screen->hpos() >= 363) SNES_CPU_REG(HVBJOY) |= 0x40;
//              else SNES_CPU_REG(HVBJOY) &= ~0x40;
			return (SNES_CPU_REG(HVBJOY) & 0xc1) | (snes_open_bus_r(space, 0) & 0x3e);
		case RDIO:          /* Programmable I/O port - echos back what's written to WRIO */
			return SNES_CPU_REG(WRIO);
		case JOY1L:         /* Joypad 1 status register (low) */
		case JOY1H:         /* Joypad 1 status register (high) */
		case JOY2L:         /* Joypad 2 status register (low) */
		case JOY2H:         /* Joypad 2 status register (high) */
		case JOY3L:         /* Joypad 3 status register (low) */
		case JOY3H:         /* Joypad 3 status register (high) */
		case JOY4L:         /* Joypad 4 status register (low) */
		case JOY4H:         /* Joypad 4 status register (high) */
			if (m_is_nss && m_input_disabled)
				return 0;
			return SNES_CPU_REG(offset);

		case 0x4100:        /* NSS Dip-Switches */
			if (m_is_nss)
				return ioport("DSW")->read();
			break;
//      case 0x4101: //PC: a104 - a10e - a12a   //only nss_actr (DSW actually reads in word units ...)

		default:
//          osd_printf_debug("snes_r: offset = %x pc = %x\n",offset,space.device().safe_pc());
// Added break; after commenting above line.  If uncommenting, drop the break;
			break;
	}

//  printf("unsupported read: offset == %08x\n", offset);

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
WRITE8_MEMBER( snes_state::snes_w_io )
{
	// PPU accesses are from 2100 to 213f
	if (offset >= INIDISP && offset < APU00)
	{
		m_ppu->write(space, offset, data);
		return;
	}

	// APU is mirrored from 2140 to 217f
	if (offset >= APU00 && offset < WMDATA)
	{
//      printf("816: %02x to APU @ %d (PC=%06x)\n", data, offset & 3,space.device().safe_pc());
		m_spc700->spc_port_in(space, offset & 0x3, data);
		space.machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(20));
		return;
	}

	// DMA accesses are from 4300 to 437f
	if (offset >= DMAP0 && offset < 0x4380)
	{
		snes_io_dma_w(space, offset, data);
		return;
	}

	/* offset is from 0x000000 */
	switch (offset)
	{
		case WMDATA:    /* Data to write to WRAM */
			space.write_byte(0x7e0000 + m_wram_address++, data );
			m_wram_address &= 0x1ffff;
			return;
		case WMADDL:    /* Address to read/write to wram (low) */
			m_wram_address = (m_wram_address & 0xffff00) | (data <<  0);
			m_wram_address &= 0x1ffff;
			return;
		case WMADDM:    /* Address to read/write to wram (mid) */
			m_wram_address = (m_wram_address & 0xff00ff) | (data <<  8);
			m_wram_address &= 0x1ffff;
			return;
		case WMADDH:    /* Address to read/write to wram (high) */
			m_wram_address = (m_wram_address & 0x00ffff) | (data << 16);
			m_wram_address &= 0x1ffff;
			return;
		case OLDJOY1:   /* Old NES joystick support */
			write_joy_latch(data);
			if (m_is_nss)
				m_game_over_flag = (data & 4) >> 2;
			return;
		case OLDJOY2:   /* Old NES joystick support */
			return;
		case NMITIMEN:  /* Flag for v-blank, timer int. and joy read */
			if ((data & 0x30) == 0x00)
			{
				m_maincpu->set_input_line(G65816_LINE_IRQ, CLEAR_LINE );
				SNES_CPU_REG(TIMEUP) = 0;   // clear pending IRQ if irq is disabled here, 3x3 Eyes - Seima Korin Den behaves on this
			}
			SNES_CPU_REG(NMITIMEN) = data;
			return;
		case WRIO:      /* Programmable I/O port - latches H/V counters on a 0->1 transition */
			wrio_write(data);
			SNES_CPU_REG(WRIO) = data;
			return;
		case HTIMEL:    /* H-Count timer settings (low)  */
			m_htime = (m_htime & 0xff00) | (data <<  0);
			m_htime &= 0x1ff;
			return;
		case HTIMEH:    /* H-Count timer settings (high) */
			m_htime = (m_htime & 0x00ff) | (data <<  8);
			m_htime &= 0x1ff;
			return;
		case VTIMEL:    /* V-Count timer settings (low)  */
			m_vtime = (m_vtime & 0xff00) | (data <<  0);
			m_vtime &= 0x1ff;
			return;
		case VTIMEH:    /* V-Count timer settings (high) */
			m_vtime = (m_vtime & 0x00ff) | (data <<  8);
			m_vtime &= 0x1ff;
			return;
		case MDMAEN:    /* DMA channel designation and trigger */
			dma(space, data);
			SNES_CPU_REG(MDMAEN) = 0;   /* Once DMA is done we need to reset all bits to 0 */
			return;
		case HDMAEN:    /* HDMA channel designation */
			if (data) //if a HDMA is enabled, data is inited at the next scanline
				timer_set(m_screen->time_until_pos(m_ppu->m_beam.current_vert + 1), TIMER_RESET_HDMA);
			SNES_CPU_REG(HDMAEN) = data;
			return;
		case TIMEUP:    // IRQ Flag is cleared on both read and write
			m_maincpu->set_input_line(G65816_LINE_IRQ, CLEAR_LINE );
			SNES_CPU_REG(TIMEUP) = 0;
			return;
		/* Following are read-only */
		case HVBJOY:    /* H/V blank and joypad enable */
		case MPYL:      /* Multiplication result (low) */
		case MPYM:      /* Multiplication result (mid) */
		case MPYH:      /* Multiplication result (high) */
		case RDIO:
//      case RDDIVL:
//      case RDDIVH:
//      case RDMPYL:
//      case RDMPYH:
		case JOY1L:
		case JOY1H:
		case JOY2L:
		case JOY2H:
		case JOY3L:
		case JOY3H:
		case JOY4L:
		case JOY4H:
//#ifdef MAME_DEBUG
			logerror( "Write to read-only register: %X value: %X", offset, data );
//#endif /* MAME_DEBUG */
			return;
	}

}

void snes_state::write_joy_latch(UINT8 data)
{
	if (m_oldjoy1_latch == (data & 0x01))
		return;

	m_oldjoy1_latch = data & 0x01;
	m_read_idx[0] = 0;
	m_read_idx[1] = 0;
	m_read_idx[2] = 0;
	m_read_idx[3] = 0;
}


void snes_state::wrio_write(UINT8 data)
{
	if (!(SNES_CPU_REG(WRIO) & 0x80) && (data & 0x80))
	{
		// external latch
		m_ppu->set_latch_hv(m_ppu->current_x(), m_ppu->current_y());
	}
}

WRITE_LINE_MEMBER(snes_state::snes_extern_irq_w)
{
	m_maincpu->set_input_line(G65816_LINE_IRQ, state);
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

inline UINT8 snes_state::snes_rom_access(UINT32 offset)
{
	UINT32 addr;
	UINT8 value = 0xff;
	UINT8 base_bank = (offset < 0x800000) ? 0x80 : 0x00;

	switch (m_cart.mode)
	{
		case SNES_MODE_20:
		case SNES_MODE_22:
			addr = (m_cart.rom_bank_map[offset/0x10000] * 0x8000) + (offset & 0x7fff);
			value = m_cart.m_rom[addr];
			break;
		case SNES_MODE_21:
		case SNES_MODE_25:
			offset &= 0x3fffff;
			addr = (m_cart.rom_bank_map[base_bank + (offset/0x8000)] * 0x8000) + (offset & 0x7fff);
			value = m_cart.m_rom[addr];
			break;
	}

	return value;
}

/* 0x000000 - 0x7dffff */
READ8_MEMBER(snes_state::snes_r_bank1)
{
	UINT8 value = 0xff;
	UINT16 address = offset & 0xffff;

	if (offset < 0x400000)
	{
		if (address < 0x2000)                                           /* Mirror of Low RAM */
			value = space.read_byte(0x7e0000 + address);
		else if (address < 0x6000)                                      /* I/O */
			value = snes_r_io(space, address);
		else if (address < 0x8000)
		{
			if (offset >= 0x300000 && m_cart.mode == SNES_MODE_21 && m_cart.m_nvram_size > 0)
			{
				/* Donkey Kong Country checks this and detects a copier if 0x800 is not masked out due to sram size */
				/* OTOH Secret of Mana does not work properly if sram is not mirrored on later banks */
				int mask = (m_cart.m_nvram_size - 1) & 0x7fff; /* Limit SRAM size to what's actually present */
				value = m_cart.m_nvram[(offset - 0x6000) & mask];
			}
			else
				value = snes_open_bus_r(space, 0);                              /* Reserved */
		}
		else
			value = snes_rom_access(offset);   //ROM
	}
	else if (offset < 0x700000)
	{
		if (m_cart.mode & 5 && address < 0x8000)  /* Mode 20 & 22 in 0x0000-0x7fff */
			value = snes_open_bus_r(space, 0);
		else
			value = snes_rom_access(offset);    //ROM
	}
	else
	{
		if (m_cart.mode & 5 && address < 0x8000)     /* Mode 20 & 22 */
		{
			if (m_cart.m_nvram_size > 0x8000)
			{
				// In this case, SRAM is mapped in 0x8000 chunks at diff offsets: 0x700000-0x707fff, 0x710000-0x717fff, etc.
				int mask = m_cart.m_nvram_size - 1;
				offset = (offset / 0x10000) * 0x8000 + (offset & 0x7fff);
				value = m_cart.m_nvram[offset & mask];
			}
			else if (m_cart.m_nvram_size > 0)
			{
				int mask = m_cart.m_nvram_size - 1;   /* Limit SRAM size to what's actually present */
				value = m_cart.m_nvram[offset & mask];
			}
			else
			{
				logerror("(PC=%06x) snes_r_bank1: Unmapped external chip read: %X\n", space.device().safe_pc(), offset);
				value = snes_open_bus_r(space, 0);                              /* Reserved */
			}
		}
		else
			value = snes_rom_access(offset);    //ROM
	}

	return value;
}


/* 0x800000 - 0xffffff */
READ8_MEMBER(snes_state::snes_r_bank2)
{
	UINT8 value = 0;
	UINT16 address = offset & 0xffff;

	if (offset < 0x400000)
	{
		if (address < 0x8000)
			value = space.read_byte(offset);
		else
			value = snes_rom_access(0x800000 + offset);    //ROM
	}
	else
	{
		if (m_cart.mode & 5 && address < 0x8000)      /* Mode 20 & 22 in 0x0000-0x7fff */
		{
			if (offset < 0x700000)
				value = space.read_byte(offset);
			else
			{
				if (m_cart.m_nvram_size > 0x8000)
				{
					// In this case, SRAM is mapped in 0x8000 chunks at diff offsets: 0x700000-0x707fff, 0x710000-0x717fff, etc.
					int mask = m_cart.m_nvram_size - 1;
					offset = (offset / 0x10000) * 0x8000 + (offset & 0x7fff);
					value = m_cart.m_nvram[offset & mask];
				}
				else if (m_cart.m_nvram_size > 0)
				{
					int mask = m_cart.m_nvram_size - 1;   /* Limit SRAM size to what's actually present */
					value = m_cart.m_nvram[offset & mask];
				}
				else
				{
					logerror("(PC=%06x) snes_r_bank2: Unmapped external chip read: %X\n", space.device().safe_pc(), offset);
					value = snes_open_bus_r(space, 0);                              /* Reserved */
				}
			}
		}
		else
			value = snes_rom_access(0x800000 + offset);    //ROM
	}

	return value;
}


/* 0x000000 - 0x7dffff */
WRITE8_MEMBER(snes_state::snes_w_bank1)
{
	UINT16 address = offset & 0xffff;

	if (offset < 0x400000)
	{
		if (address < 0x2000)                           /* Mirror of Low RAM */
			space.write_byte(0x7e0000 + address, data);
		else if (address < 0x6000)                      /* I/O */
			snes_w_io(space, address, data);
		else if (address < 0x8000)
		{
			if (offset >= 0x300000 && m_cart.mode == SNES_MODE_21 && m_cart.m_nvram_size > 0)
			{
				/* Donkey Kong Country checks this and detects a copier if 0x800 is not masked out due to sram size */
				/* OTOH Secret of Mana does not work properly if sram is not mirrored on later banks */
				int mask = (m_cart.m_nvram_size - 1) & 0x7fff; /* Limit SRAM size to what's actually present */
				m_cart.m_nvram[(offset - 0x6000) & mask] = data;
			}
			else
				logerror("(PC=%06x) snes_w_bank1: Attempt to write to reserved address: %X = %02X\n", space.device().safe_pc(), offset, data);
		}
		else
			logerror("(PC=%06x) Attempt to write to ROM address: %X\n", space.device().safe_pc(), offset);
	}
	else if (offset >= 0x600000 && offset < 0x700000)
	{
		if (m_cart.mode & 5 && address < 0x8000)        /* Mode 20 & 22 */
			logerror("(PC=%06x) snes_w_bank1: Attempt to write to reserved address: %X = %02X\n", space.device().safe_pc(), offset, data);
		else
			logerror("(PC=%06x) Attempt to write to ROM address: %X\n", space.device().safe_pc(), offset);
	}
	else if (offset >= 0x700000)
	{
		if (m_cart.mode & 5 && address < 0x8000)         /* Mode 20 & 22 */
		{
			if (m_cart.m_nvram_size > 0x8000)
			{
				// In this case, SRAM is mapped in 0x8000 chunks at diff offsets: 0x700000-0x707fff, 0x710000-0x717fff, etc.
				int mask = m_cart.m_nvram_size - 1;
				offset = (offset / 0x10000) * 0x8000 + (offset & 0x7fff);
				m_cart.m_nvram[offset & mask] = data;
			}
			else if (m_cart.m_nvram_size > 0)
			{
				int mask = m_cart.m_nvram_size - 1;   /* Limit SRAM size to what's actually present */
				m_cart.m_nvram[offset & mask] = data;
			}
			else
				logerror("(PC=%06x) snes_w_bank1: Attempt to write to reserved address: %X = %02X\n", space.device().safe_pc(), offset, data);
		}
		else
			logerror("(PC=%06x) Attempt to write to ROM address: %X\n", space.device().safe_pc(), offset);
	}
}

/* 0x800000 - 0xffffff */
WRITE8_MEMBER(snes_state::snes_w_bank2)
{
	UINT16 address = offset & 0xffff;

	if (offset < 0x400000)
	{
		if (address < 0x8000)
			space.write_byte(offset, data);
		else
			logerror("(PC=%06x) Attempt to write to ROM address: %X\n", space.device().safe_pc(), offset + 0x800000);
	}
	else
	{
		if (m_cart.mode & 5 && address < 0x8000)      /* Mode 20 & 22 in 0x0000-0x7fff */
		{
			if (offset < 0x700000)
				space.write_byte(offset, data);
			else
			{
				if (m_cart.m_nvram_size > 0x8000)
				{
					// In this case, SRAM is mapped in 0x8000 chunks at diff offsets: 0x700000-0x707fff, 0x710000-0x717fff, etc.
					int mask = m_cart.m_nvram_size - 1;
					offset = (offset / 0x10000) * 0x8000 + (offset & 0x7fff);
					m_cart.m_nvram[offset & mask] = data;
				}
				else if (m_cart.m_nvram_size > 0)
				{
					int mask = m_cart.m_nvram_size - 1;   /* Limit SRAM size to what's actually present */
					m_cart.m_nvram[offset & mask] = data;
				}
				else
					logerror("(PC=%06x) snes_w_bank2: Attempt to write to reserved address: %X = %02X\n", space.device().safe_pc(), offset, data);
			}
		}
		else
			logerror("(PC=%06x) Attempt to write to ROM address: %X\n", space.device().safe_pc(), offset);
	}
}


/*************************************

    Input Callbacks

*************************************/

WRITE8_MEMBER(snes_state::io_read)
{
	static const char *const portnames[2][2] =
	{
		{ "SERIAL1_DATA1", "SERIAL1_DATA2" },
		{ "SERIAL2_DATA1", "SERIAL2_DATA2" }
	};

	for (int port = 0; port < 2; port++)
	{
		m_data1[port] = ioport(portnames[port][0])->read();
		m_data2[port] = ioport(portnames[port][1])->read();

		// avoid sending signals that could crash games
		// if left, no right
		if (m_data1[port] & 0x200)
			m_data1[port] &= ~0x100;
		// if up, no down
		if (m_data1[port] & 0x800)
			m_data1[port] &= ~0x400;
		// if left, no right
		if (m_data2[port] & 0x200)
			m_data2[port] &= ~0x100;
		// if up, no down
		if (m_data2[port] & 0x800)
			m_data2[port] &= ~0x400;
	}

	// is automatic reading on? if so, copy port data1/data2 to joy1l->joy4h
	// this actually works like reading the first 16bits from oldjoy1/2 in reverse order
	if (SNES_CPU_REG(NMITIMEN) & 1)
	{
		SNES_CPU_REG(JOY1L) = (m_data1[0] & 0x00ff) >> 0;
		SNES_CPU_REG(JOY1H) = (m_data1[0] & 0xff00) >> 8;
		SNES_CPU_REG(JOY2L) = (m_data1[1] & 0x00ff) >> 0;
		SNES_CPU_REG(JOY2H) = (m_data1[1] & 0xff00) >> 8;
		SNES_CPU_REG(JOY3L) = (m_data2[0] & 0x00ff) >> 0;
		SNES_CPU_REG(JOY3H) = (m_data2[0] & 0xff00) >> 8;
		SNES_CPU_REG(JOY4L) = (m_data2[1] & 0x00ff) >> 0;
		SNES_CPU_REG(JOY4H) = (m_data2[1] & 0xff00) >> 8;

		// make sure read_idx starts returning all 1s because the auto-read reads it :-)
		m_read_idx[0] = 16;
		m_read_idx[1] = 16;
	}

	if(m_is_nss)
		m_joy_flag = 0;
}


UINT8 snes_state::oldjoy1_read(int latched)
{
	if (latched)
		return 0;

	if (m_read_idx[0] >= 16)
		return 1;
	else
		return (m_data1[0] >> (15 - m_read_idx[0]++)) & 0x01;
}

UINT8 snes_state::oldjoy2_read(int latched)
{
	if (latched)
		return 0;

	if (m_read_idx[1] >= 16)
		return 1;
	else
		return (m_data1[1] >> (15 - m_read_idx[1]++)) & 0x01;
}


/*************************************

    Driver Init

*************************************/

void snes_state::snes_init_timers()
{
	/* init timers and stop them */
	m_scanline_timer = timer_alloc(TIMER_SCANLINE_TICK);
	m_scanline_timer->adjust(attotime::never);
	m_hblank_timer = timer_alloc(TIMER_HBLANK_TICK);
	m_hblank_timer->adjust(attotime::never);
	m_nmi_timer = timer_alloc(TIMER_NMI_TICK);
	m_nmi_timer->adjust(attotime::never);
	m_hirq_timer = timer_alloc(TIMER_HIRQ_TICK);
	m_hirq_timer->adjust(attotime::never);
	//m_div_timer = timer_alloc(TIMER_DIV);
	//m_div_timer->adjust(attotime::never);
	//m_mult_timer = timer_alloc(TIMER_MULT);
	//m_mult_timer->adjust(attotime::never);
	m_io_timer = timer_alloc(TIMER_UPDATE_IO);
	m_io_timer->adjust(attotime::never);

	// SNES hcounter has a 0-339 range.  hblank starts at counter 260.
	// clayfighter sets an HIRQ at 260, apparently it wants it to be before hdma kicks off, so we'll delay 2 pixels.
	m_hblank_offset = 274;
	m_hblank_timer->adjust(m_screen->time_until_pos(((m_ppu->m_stat78 & 0x10) == SNES_NTSC) ? SNES_VTOTAL_NTSC - 1 : SNES_VTOTAL_PAL - 1, m_hblank_offset));
}

void snes_state::snes_init_ram()
{
	address_space &cpu0space = m_maincpu->space(AS_PROGRAM);
	int i;

	/* Init work RAM - 0x55 isn't exactly right but it's close */
	/* make sure it happens to the 65816 (CPU 0) */
	for (i = 0; i < (128*1024); i++)
	{
		cpu0space.write_byte(0x7e0000 + i, 0x55);
	}

	/* Inititialize registers/variables */
	SNES_CPU_REG(JOY1L) = SNES_CPU_REG(JOY1H) = 0;
	SNES_CPU_REG(JOY2L) = SNES_CPU_REG(JOY2H) = 0;
	SNES_CPU_REG(JOY3L) = SNES_CPU_REG(JOY3H) = 0;
	SNES_CPU_REG(JOY4L) = SNES_CPU_REG(JOY4H) = 0;
	m_data1[0] = m_data2[0] = m_data1[1] = m_data2[1] = 0;

	// set up some known register power-up defaults
	SNES_CPU_REG(WRIO) = 0xff;

	// init frame counter so first line is 0
	if (ATTOSECONDS_TO_HZ(m_screen->frame_period().attoseconds()) >= 59)
		m_ppu->m_beam.current_vert = SNES_VTOTAL_NTSC;
	else
		m_ppu->m_beam.current_vert = SNES_VTOTAL_PAL;
}

void snes_state::machine_start()
{
	// power-on sets these registers like this
	SNES_CPU_REG(WRIO) = 0xff;
//  SNES_CPU_REG(WRMPYA) = 0xff;
//  SNES_CPU_REG(WRDIVL) = 0xff;
//  SNES_CPU_REG(WRDIVH) = 0xff;

	snes_init_timers();

	for (int i = 0; i < 8; i++)
	{
		save_item(NAME(m_dma_channel[i].dmap), i);
		save_item(NAME(m_dma_channel[i].dest_addr), i);
		save_item(NAME(m_dma_channel[i].src_addr), i);
		save_item(NAME(m_dma_channel[i].bank), i);
		save_item(NAME(m_dma_channel[i].trans_size), i);
		save_item(NAME(m_dma_channel[i].ibank), i);
		save_item(NAME(m_dma_channel[i].hdma_addr), i);
		save_item(NAME(m_dma_channel[i].hdma_iaddr), i);
		save_item(NAME(m_dma_channel[i].hdma_line_counter), i);
		save_item(NAME(m_dma_channel[i].unk), i);
		save_item(NAME(m_dma_channel[i].do_transfer), i);
		save_item(NAME(m_dma_channel[i].dma_disabled), i);
	}

	save_item(NAME(m_hblank_offset));
	save_item(NAME(m_wram_address));
	save_item(NAME(m_htime));
	save_item(NAME(m_vtime));
	save_item(NAME(m_hdmaen));
	save_item(NAME(m_data1));
	save_item(NAME(m_data2));
	save_item(NAME(m_read_idx));
	save_item(NAME(m_dma_regs));
	save_item(NAME(m_cpu_regs));
	save_item(NAME(m_oldjoy1_latch));
	save_item(NAME(m_input_disabled));
	save_item(NAME(m_game_over_flag));
	save_item(NAME(m_joy_flag));

	m_is_nss = 0;
	m_is_sfcbox = 0;
	m_input_disabled = 0;
	m_game_over_flag = 0;
	m_joy_flag = 1;
}

void snes_state::machine_reset()
{
	snes_init_ram();

	/* init DMA regs to be 0xff */
	for (int i = 0; i < 8; i++)
	{
		m_dma_channel[i].dmap = 0xff;
		m_dma_channel[i].dest_addr = 0xff;
		m_dma_channel[i].src_addr = 0xffff;
		m_dma_channel[i].bank = 0xff;
		m_dma_channel[i].trans_size = 0xffff;
		m_dma_channel[i].ibank = 0xff;
		m_dma_channel[i].hdma_addr = 0xffff;
		m_dma_channel[i].hdma_line_counter = 0xff;
		m_dma_channel[i].unk = 0xff;
	}

	/* Set STAT78 to NTSC or PAL */
	if (ATTOSECONDS_TO_HZ(m_screen->frame_period().attoseconds()) >= 59.0)
		m_ppu->m_stat78 = SNES_NTSC;
	else /* if (ATTOSECONDS_TO_HZ(m_screen->frame_period().attoseconds) == 50.0f) */
		m_ppu->m_stat78 = SNES_PAL;

	// reset does this to these registers
	SNES_CPU_REG(NMITIMEN) = 0;
	m_htime = 0x1ff;
	m_vtime = 0x1ff;

	m_ppu->m_htmult = 1;
	m_ppu->m_interlace = 1;
	m_ppu->m_obj_interlace = 1;
}


void snes_state::rom_map_setup(UINT32 size)
{
	int i;
	// setup the rom_bank_map array to faster ROM read
	for (i = 0; i < size / 0x8000; i++)
		m_cart.rom_bank_map[i] = i;

	// fill up remaining blocks with mirrors
	while (i % 256)
	{
		int j = 0, repeat_banks;
		while ((i % (256 >> j)) && j < 8)
			j++;
		repeat_banks = i % (256 >> (j - 1));
		for (int k = 0; k < repeat_banks; k++)
			m_cart.rom_bank_map[i + k] = m_cart.rom_bank_map[i + k - repeat_banks];
		i += repeat_banks;
	}
}

/* for mame we use an init, maybe we will need more for the different games */
DRIVER_INIT_MEMBER(snes_state,snes)
{
	m_cart.m_rom_size = memregion("user3")->bytes();
	m_cart.m_rom = memregion("user3")->base();
	rom_map_setup(m_cart.m_rom_size);

	m_cart.m_nvram_size = 0;
	if (m_cart.m_rom[0x7fd8] > 0)
	{
		UINT32 nvram_size = (1024 << m_cart.m_rom[0x7fd8]);
		if (nvram_size > 0x40000)
			nvram_size = 0x40000;

		m_cart.m_nvram = auto_alloc_array_clear(machine(), UINT8, nvram_size);
		m_cart.m_nvram_size = nvram_size;
	}

	/* all NSS games seem to use MODE 20 */
	m_cart.mode = SNES_MODE_20;
}

DRIVER_INIT_MEMBER(snes_state,snes_hirom)
{
	m_cart.m_rom_size = memregion("user3")->bytes();
	m_cart.m_rom = memregion("user3")->base();
	rom_map_setup(m_cart.m_rom_size);

	m_cart.m_nvram_size = 0;
	if (m_cart.m_rom[0xffd8] > 0)
	{
		UINT32 nvram_size = (1024 << m_cart.m_rom[0xffd8]);
		if (nvram_size > 0x40000)
			nvram_size = 0x40000;

		m_cart.m_nvram = auto_alloc_array_clear(machine(), UINT8, nvram_size);
		m_cart.m_nvram_size = nvram_size;
	}

	m_cart.mode = SNES_MODE_21;
}


/*************************************

    HDMA

*************************************/

inline int snes_state::dma_abus_valid( UINT32 address )
{
	if((address & 0x40ff00) == 0x2100) return 0;  //$[00-3f|80-bf]:[2100-21ff]
	if((address & 0x40fe00) == 0x4000) return 0;  //$[00-3f|80-bf]:[4000-41ff]
	if((address & 0x40ffe0) == 0x4200) return 0;  //$[00-3f|80-bf]:[4200-421f]
	if((address & 0x40ff80) == 0x4300) return 0;  //$[00-3f|80-bf]:[4300-437f]

	return 1;
}

inline UINT8 snes_state::abus_read( address_space &space, UINT32 abus )
{
	if (!dma_abus_valid(abus))
		return 0;

	return space.read_byte(abus);
}

inline void snes_state::dma_transfer( address_space &space, UINT8 dma, UINT32 abus, UINT16 bbus )
{
	if (m_dma_channel[dma].dmap & 0x80)  /* PPU->CPU */
	{
		if (bbus == 0x2180 && ((abus & 0xfe0000) == 0x7e0000 || (abus & 0x40e000) == 0x0000))
		{
			//illegal WRAM->WRAM transfer (bus conflict)
			//no read occurs; write does occur
			space.write_byte(abus, 0x00);
			return;
		}
		else
		{
			if (!dma_abus_valid(abus))
				return;

			space.write_byte(abus, space.read_byte(bbus));
			return;
		}
	}
	else                                    /* CPU->PPU */
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
			space.write_byte(bbus, abus_read(space, abus));
			return;
		}
	}
}

/* WIP: These have the advantage to automatically update the address, but then we would need to
check again if the transfer is direct/indirect at each step... is it worth? */
inline UINT32 snes_state::get_hdma_addr( int dma )
{
	return (m_dma_channel[dma].bank << 16) | (m_dma_channel[dma].hdma_addr++);
}

inline UINT32 snes_state::get_hdma_iaddr( int dma )
{
	return (m_dma_channel[dma].ibank << 16) | (m_dma_channel[dma].trans_size++);
}

inline int snes_state::is_last_active_channel( int dma )
{
	for (int i = dma + 1; i < 8; i++)
	{
		if (BIT(m_hdmaen, i) && m_dma_channel[i].hdma_line_counter)
			return 0;   // there is still at least another channel with incomplete HDMA
	}

	// if we arrive here, all hdma transfers from (dma + 1) to 7 are completed or not active
	return 1;
}

void snes_state::hdma_update( address_space &space, int dma )
{
	UINT32 abus = get_hdma_addr(dma);

	m_dma_channel[dma].hdma_line_counter = abus_read(space, abus);

	if (m_dma_channel[dma].dmap & 0x40)
	{
		/* One oddity: if $43xA is 0 and this is the last active HDMA channel for this scanline, only load
		one byte for Address, and use the $00 for the low byte. So Address ends up incremented one less than
		otherwise expected */

		abus = get_hdma_addr(dma);
		m_dma_channel[dma].trans_size = abus_read(space, abus) << 8;

		if (m_dma_channel[dma].hdma_line_counter || !is_last_active_channel(dma))
		{
			// we enter here if we have more transfers to be done or if there are other active channels after this one
			abus = get_hdma_addr(dma);
			m_dma_channel[dma].trans_size >>= 8;
			m_dma_channel[dma].trans_size |= abus_read(space, abus) << 8;
		}
	}

	if (!m_dma_channel[dma].hdma_line_counter)
		m_hdmaen &= ~(1 << dma);

	m_dma_channel[dma].do_transfer = 1;
}

void snes_state::hdma_init( address_space &space )
{
	m_hdmaen = SNES_CPU_REG(HDMAEN);
	for (int i = 0; i < 8; i++)
	{
		if (BIT(m_hdmaen, i))
		{
			m_dma_channel[i].hdma_addr = m_dma_channel[i].src_addr;
			hdma_update(space, i);
		}
	}
}

void snes_state::hdma( address_space &space )
{
	UINT16 bbus;
	UINT32 abus;

	for (int i = 0; i < 8; i++)
	{
		if (BIT(m_hdmaen, i))
		{
			if (m_dma_channel[i].do_transfer)
			{
				/* Get transfer addresses */
				if (m_dma_channel[i].dmap & 0x40)    /* Indirect */
					abus = (m_dma_channel[i].ibank << 16) + m_dma_channel[i].trans_size;
				else                                    /* Absolute */
					abus = (m_dma_channel[i].bank << 16) + m_dma_channel[i].hdma_addr;

				bbus = m_dma_channel[i].dest_addr + 0x2100;



				switch (m_dma_channel[i].dmap & 0x07)
				{
				case 0:     /* 1 register write once             (1 byte:  p               ) */
					dma_transfer(space, i, abus++, bbus);
					break;
				case 5:     /* 2 registers write twice alternate (4 bytes: p, p+1, p,   p+1) */
					dma_transfer(space, i, abus++, bbus);
					dma_transfer(space, i, abus++, bbus + 1);
					dma_transfer(space, i, abus++, bbus);
					dma_transfer(space, i, abus++, bbus + 1);
					break;
				case 1:     /* 2 registers write once            (2 bytes: p, p+1          ) */
					dma_transfer(space, i, abus++, bbus);
					dma_transfer(space, i, abus++, bbus + 1);
					break;
				case 2:     /* 1 register write twice            (2 bytes: p, p            ) */
				case 6:
					dma_transfer(space, i, abus++, bbus);
					dma_transfer(space, i, abus++, bbus);
					break;
				case 3:     /* 2 registers write twice each      (4 bytes: p, p,   p+1, p+1) */
				case 7:
					dma_transfer(space, i, abus++, bbus);
					dma_transfer(space, i, abus++, bbus);
					dma_transfer(space, i, abus++, bbus + 1);
					dma_transfer(space, i, abus++, bbus + 1);
					break;
				case 4:     /* 4 registers write once            (4 bytes: p, p+1, p+2, p+3) */
					dma_transfer(space, i, abus++, bbus);
					dma_transfer(space, i, abus++, bbus + 1);
					dma_transfer(space, i, abus++, bbus + 2);
					dma_transfer(space, i, abus++, bbus + 3);
					break;
				default:
#ifdef MAME_DEBUG
					osd_printf_debug( "  HDMA of unsupported type: %d\n", m_dma_channel[i].dmap & 0x07);
#endif
					break;
				}

				if (m_dma_channel[i].dmap & 0x40)    /* Indirect */
					m_dma_channel[i].trans_size = abus;
				else                                    /* Absolute */
					m_dma_channel[i].hdma_addr = abus;

			}
		}
	}

	for (int i = 0; i < 8; i++)
	{
		if (BIT(m_hdmaen, i))
		{
			m_dma_channel[i].do_transfer = (--m_dma_channel[i].hdma_line_counter) & 0x80;

			if (!(m_dma_channel[i].hdma_line_counter & 0x7f))
				hdma_update(space, i);
		}
	}
}

void snes_state::dma( address_space &space, UINT8 channels )
{
	INT8 increment;
	UINT16 bbus;
	UINT32 abus, abus_bank;
	UINT16 length;

	/* FIXME: we also need to round to the nearest 8 master cycles */

	/* Assume priority of the 8 DMA channels is 0-7 */
	for (int i = 0; i < 8; i++)
	{
		if (BIT(channels, i))
		{
			/* FIXME: the following should be used to stop DMA if the same channel is used by HDMA (being set to 1 in snes_hdma)
			 However, this cannot be implemented as is atm, because currently DMA transfers always happen as soon as they are enabled... */
			m_dma_channel[i].dma_disabled = 0;

			//printf( "Making a transfer on channel %d\n", i );
			/* Find transfer addresses */
			abus = m_dma_channel[i].src_addr;
			abus_bank = m_dma_channel[i].bank << 16;
			bbus = m_dma_channel[i].dest_addr + 0x2100;

			//printf("Address: %06x\n", abus | abus_bank);
			/* Auto increment */
			if (m_dma_channel[i].dmap & 0x8)
				increment = 0;
			else
			{
				if (m_dma_channel[i].dmap & 0x10)
					increment = -1;
				else
					increment = 1;
			}

			/* Number of bytes to transfer */
			length = m_dma_channel[i].trans_size;

//          printf( "DMA-Ch %d: len: %X, abus: %X, bbus: %X, incr: %d, dir: %s, type: %d\n", i, length, abus | abus_bank, bbus, increment, m_dma_channel[i].dmap & 0x80 ? "PPU->CPU" : "CPU->PPU", m_dma_channel[i].dmap & 0x07);

#ifdef SNES_DBG_DMA
			osd_printf_debug( "DMA-Ch %d: len: %X, abus: %X, bbus: %X, incr: %d, dir: %s, type: %d\n", i, length, abus | abus_bank, bbus, increment, m_dma_channel[i].dmap & 0x80 ? "PPU->CPU" : "CPU->PPU", m_dma_channel[i].dmap & 0x07);
#endif

			switch (m_dma_channel[i].dmap & 0x07)
			{
				case 0:     /* 1 register write once */
				case 2:     /* 1 register write twice */
				case 6:     /* 1 register write twice */
					do
					{
						dma_transfer(space, i, (abus & 0xffff) | abus_bank, bbus);
						abus += increment;
					} while (--length && !m_dma_channel[i].dma_disabled);
					break;
				case 1:     /* 2 registers write once */
				case 5:     /* 2 registers write twice alternate */
					do
					{
						dma_transfer(space, i, (abus & 0xffff) | abus_bank, bbus);
						abus += increment;
						if (!(--length) || m_dma_channel[i].dma_disabled)
							break;
						dma_transfer(space, i, (abus & 0xffff) | abus_bank, bbus + 1);
						abus += increment;
					} while (--length && !m_dma_channel[i].dma_disabled);
					break;
				case 3:     /* 2 registers write twice each */
				case 7:     /* 2 registers write twice each */
					do
					{
						dma_transfer(space, i, (abus & 0xffff) | abus_bank, bbus);
						abus += increment;
						if (!(--length) || m_dma_channel[i].dma_disabled)
							break;
						dma_transfer(space, i, (abus & 0xffff) | abus_bank, bbus);
						abus += increment;
						if (!(--length) || m_dma_channel[i].dma_disabled)
							break;
						dma_transfer(space, i, (abus & 0xffff) | abus_bank, bbus + 1);
						abus += increment;
						if (!(--length) || m_dma_channel[i].dma_disabled)
							break;
						dma_transfer(space, i, (abus & 0xffff) | abus_bank, bbus + 1);
						abus += increment;
					} while (--length && !m_dma_channel[i].dma_disabled);
					break;
				case 4:     /* 4 registers write once */
					do
					{
						dma_transfer(space, i, (abus & 0xffff) | abus_bank, bbus);
						abus += increment;
						if (!(--length) || m_dma_channel[i].dma_disabled)
							break;
						dma_transfer(space, i, (abus & 0xffff) | abus_bank, bbus + 1);
						abus += increment;
						if (!(--length) || m_dma_channel[i].dma_disabled)
							break;
						dma_transfer(space, i, (abus & 0xffff) | abus_bank, bbus + 2);
						abus += increment;
						if (!(--length) || m_dma_channel[i].dma_disabled)
							break;
						dma_transfer(space, i, (abus & 0xffff) | abus_bank, bbus + 3);
						abus += increment;
					} while (--length && !m_dma_channel[i].dma_disabled);
					break;
				default:
#ifdef MAME_DEBUG
					osd_printf_debug("  DMA of unsupported type: %d\n", m_dma_channel[i].dmap & 0x07);
#endif
					break;
			}

			/* We're done, so write the new abus back to the registers */
			m_dma_channel[i].src_addr = abus;
			m_dma_channel[i].trans_size = 0;
		}
	}

}
