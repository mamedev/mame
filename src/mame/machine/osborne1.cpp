// license:BSD-3-Clause
// copyright-holders:Wilbert Pol,Vas Crabb
/***************************************************************************

There are three IRQ sources:
- IRQ0 = IRQ from the serial ACIA
- IRQ1 = IRQA from the video PIA
- IRQ2 = IRQA from the IEEE488 PIA

***************************************************************************/

#include "includes/osborne1.h"


WRITE8_MEMBER( osborne1_state::bank_0xxx_w )
{
	if (!m_rom_mode)
		m_ram->pointer()[offset] = data;
}

WRITE8_MEMBER( osborne1_state::bank_1xxx_w )
{
	if (!m_rom_mode)
		m_ram->pointer()[0x1000 + offset] = data;
}

READ8_MEMBER( osborne1_state::bank_2xxx_3xxx_r )
{
	if (!m_rom_mode)
		return m_ram->pointer()[0x2000 + offset];

	// Since each peripheral only checks two bits, many addresses will
	// result in multiple peripherals attempting to drive the bus.  This is
	// simulated by ANDing all the values together.
	UINT8 data = 0xFF;
	if ((offset & 0x900) == 0x100) // Floppy
		data &= m_fdc->read(space, offset & 0x03);
	if ((offset & 0x900) == 0x900) // IEEE488 PIA
		data &= m_pia0->read(space, offset & 0x03);
	if ((offset & 0xA00) == 0x200) // Keyboard
	{
		if (offset & 0x01) data &= m_keyb_row0->read();
		if (offset & 0x02) data &= m_keyb_row1->read();
		if (offset & 0x04) data &= m_keyb_row3->read();
		if (offset & 0x08) data &= m_keyb_row4->read();
		if (offset & 0x10) data &= m_keyb_row5->read();
		if (offset & 0x20) data &= m_keyb_row2->read();
		if (offset & 0x40) data &= m_keyb_row6->read();
		if (offset & 0x80) data &= m_keyb_row7->read();
	}
	if ((offset & 0xA00) == 0xA00) // Serial
	{
		if (offset & 0x01) data &= m_acia->data_r(space, 0);
		else data &= m_acia->status_r(space, 0);
	}
	if ((offset & 0xC00) == 0x400) // SCREEN-PAC
	{
		if (m_screen_pac) data &= 0xFB;
	}
	if ((offset & 0xC00) == 0xC00) // Video PIA
		data &= m_pia1->read(space, offset & 0x03);
	return data;
}

WRITE8_MEMBER( osborne1_state::bank_2xxx_3xxx_w )
{
	if (!m_rom_mode)
	{
		m_ram->pointer()[0x2000 + offset] = data;
	}
	else
	{
		// Handle writes to the I/O area
		if ((offset & 0x900) == 0x100) // Floppy
			m_fdc->write(space, offset & 0x03, data);
		if ((offset & 0x900) == 0x900) // IEEE488 PIA
			m_pia0->write(space, offset & 0x03, data);
		if ((offset & 0xA00) == 0xA00) // Serial
		{
			if (offset & 0x01) m_acia->data_w(space, 0, data);
			else m_acia->control_w(space, 0, data);
		}
		if ((offset & 0xC00) == 0x400) // SCREEN-PAC
		{
			m_resolution = data & 0x01;
			m_hc_left = (data & 0x02) ? 0 : 1;
		}
		if ((offset & 0xC00) == 0xC00) // Video PIA
			m_pia1->write(space, offset & 0x03, data);
	}
}

WRITE8_MEMBER( osborne1_state::videoram_w )
{
	// Attribute RAM is only one bit wide - low seven bits are discarded and read back high
	if (m_bit_9)
		data |= 0x7F;
	else
		m_tilemap->mark_tile_dirty(offset);
	reinterpret_cast<UINT8 *>(m_bank_fxxx->base())[offset] = data;
}

READ8_MEMBER( osborne1_state::opcode_r )
{
	// Update the flipflops that control bank selection and NMI
	UINT8 const new_ub6a_q = (m_btn_reset->read() & 0x80) ? 1 : 0;
	if (!m_rom_mode)
	{
		set_rom_mode(m_ub4a_q ? 0 : 1);
		m_ub4a_q = m_ub6a_q;
	}
	m_ub6a_q = new_ub6a_q;
	m_maincpu->set_input_line(INPUT_LINE_NMI, m_ub6a_q ? CLEAR_LINE : ASSERT_LINE);

	// Now that's sorted out we can call the normal read handler
	return m_maincpu->space(AS_PROGRAM).read_byte(offset);
}

WRITE8_MEMBER( osborne1_state::bankswitch_w )
{
	switch (offset & 0x03)
	{
	case 0x00:
		if (set_rom_mode(1))
			m_ub4a_q = m_ub6a_q;
		break;
	case 0x01:
		m_ub4a_q = 1;
		m_ub6a_q = 1;
		set_rom_mode(0);
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
		break;
	case 0x02:
		set_bit_9(1);
		break;
	case 0x03:
		set_bit_9(0);
		break;
	}
}

WRITE_LINE_MEMBER( osborne1_state::irqack_w )
{
	// Update the flipflops that control bank selection and NMI
	if (!m_rom_mode) set_rom_mode(m_ub4a_q ? 0 : 1);
	m_ub4a_q = 0;
	m_ub6a_q = (m_btn_reset->read() & 0x80) ? 1 : 0;
	m_maincpu->set_input_line(INPUT_LINE_NMI, m_ub6a_q ? CLEAR_LINE : ASSERT_LINE);
}


READ8_MEMBER( osborne1_state::ieee_pia_pb_r )
{
	/*
	    bit     description

	    0
	    1
	    2
	    3       EOI
	    4
	    5       DAV
	    6       NDAC
	    7       NRFD
	*/
	UINT8 data = 0;

	data |= m_ieee->eoi_r() << 3;
	data |= m_ieee->dav_r() << 5;
	data |= m_ieee->ndac_r() << 6;
	data |= m_ieee->nrfd_r() << 7;

	return data;
}

WRITE8_MEMBER( osborne1_state::ieee_pia_pb_w )
{
	/*
	    bit     description

	    0       0 = DATAn as output, 1 = DATAn as input
	    1       0 = NDAC/NRFD as output, 1 = NDAC/NRFD as input; also gates SRQ
	    2       0 = EOI/DAV as output, 1 = EOI/DAV as input
	    3       EOI
	    4       ATN
	    5       DAV
	    6       NDAC
	    7       NRFD
	*/
	m_ieee->eoi_w(BIT(data, 3));
	m_ieee->atn_w(BIT(data, 4));
	m_ieee->dav_w(BIT(data, 5));
	m_ieee->ndac_w(BIT(data, 6));
	m_ieee->nrfd_w(BIT(data, 7));
}

WRITE_LINE_MEMBER( osborne1_state::ieee_pia_irq_a_func )
{
	update_irq();
}


WRITE8_MEMBER( osborne1_state::video_pia_port_a_w )
{
	m_scroll_x = data >> 1;

	m_fdc->dden_w(BIT(data, 0));
}

WRITE8_MEMBER( osborne1_state::video_pia_port_b_w )
{
	m_speaker->level_w((BIT(data, 5) && m_beep_state) ? 1 : 0);

	if (BIT(data, 6))
	{
		m_fdc->set_floppy(m_floppy0);
		m_floppy0->mon_w(0);
	}
	else if (BIT(data, 7))
	{
		m_fdc->set_floppy(m_floppy1);
		m_floppy1->mon_w(0);
	}
	else
	{
		m_fdc->set_floppy(nullptr);
	}
}

WRITE_LINE_MEMBER( osborne1_state::video_pia_out_cb2_dummy )
{
}

WRITE_LINE_MEMBER( osborne1_state::video_pia_irq_a_func )
{
	update_irq();
}


WRITE_LINE_MEMBER( osborne1_state::serial_acia_irq_func )
{
	m_acia_irq_state = state;
	update_irq();
}


DRIVER_INIT_MEMBER( osborne1_state, osborne1 )
{
	m_bank_0xxx->configure_entries(0, 1, m_ram->pointer(), 0);
	m_bank_0xxx->configure_entries(1, 1, m_region_maincpu->base(), 0);
	m_bank_1xxx->configure_entries(0, 1, m_ram->pointer() + 0x1000, 0);
	m_bank_1xxx->configure_entries(1, 1, m_region_maincpu->base(), 0);
	m_bank_fxxx->configure_entries(0, 1, m_ram->pointer() + 0xF000, 0);
	m_bank_fxxx->configure_entries(1, 1, m_ram->pointer() + 0x10000, 0);

	m_p_chargen = memregion("chargen")->base();
	m_video_timer = timer_alloc(TIMER_VIDEO);
	m_tilemap = &machine().tilemap().create(
			m_gfxdecode,
			tilemap_get_info_delegate(FUNC(osborne1_state::get_tile_info), this), TILEMAP_SCAN_ROWS,
			8, 10, 128, 32);

	m_acia_rxc_txc_timer = timer_alloc(TIMER_ACIA_RXC_TXC);

	save_item(NAME(m_screen_pac));
	save_item(NAME(m_acia_rxc_txc_div));
	save_item(NAME(m_acia_rxc_txc_p_low));
	save_item(NAME(m_acia_rxc_txc_p_high));

	save_item(NAME(m_ub4a_q));
	save_item(NAME(m_ub6a_q));
	save_item(NAME(m_rom_mode));
	save_item(NAME(m_bit_9));

	save_item(NAME(m_scroll_x));
	save_item(NAME(m_scroll_y));
	save_item(NAME(m_beep_state));

	save_item(NAME(m_resolution));
	save_item(NAME(m_hc_left));

	save_item(NAME(m_acia_irq_state));
	save_item(NAME(m_acia_rxc_txc_state));
}

void osborne1_state::machine_reset()
{
	// Refresh configuration
	m_screen_pac = 0 != (m_cnf->read() & 0x01);
	switch (m_cnf->read() & 0x06)
	{
	case 0x00:
		m_acia_rxc_txc_div      = 16;
		m_acia_rxc_txc_p_low    = 23;
		m_acia_rxc_txc_p_high   = 29;
		break;
	case 0x02:
		m_acia_rxc_txc_div      = 16;
		m_acia_rxc_txc_p_low    = 9;
		m_acia_rxc_txc_p_high   = 15;
		break;
	case 0x04:
		m_acia_rxc_txc_div      = 16;
		m_acia_rxc_txc_p_low    = 5;
		m_acia_rxc_txc_p_high   = 8;
		break;
	case 0x06:
		m_acia_rxc_txc_div      = 8;
		m_acia_rxc_txc_p_low    = 5;
		m_acia_rxc_txc_p_high   = 8;
		break;
	}

	// Initialise memory configuration
	m_rom_mode = 0;
	m_bit_9 = 1;
	set_rom_mode(1);
	set_bit_9(0);

	// Reset serial state
	m_acia_irq_state = 0;
	m_acia_rxc_txc_state = 0;
	update_acia_rxc_txc();

	// Reset video hardware
	m_resolution = 0;
	m_hc_left = 1;

	// The low bits of attribute RAM are not physically present and hence always read high
	for (unsigned i = 0; i < 0x1000; i++)
		m_ram->pointer()[0x10000 + i] |= 0x7F;
}

void osborne1_state::video_start()
{
	machine().first_screen()->register_screen_bitmap(m_bitmap);
	m_video_timer->adjust(machine().first_screen()->time_until_pos(1, 0));
}

UINT32 osborne1_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}


void osborne1_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_VIDEO:
		video_callback(ptr, param);
		break;
	case TIMER_ACIA_RXC_TXC:
		m_acia_rxc_txc_state = m_acia_rxc_txc_state ? 0 : 1;
		update_acia_rxc_txc();
		break;
	default:
		assert_always(FALSE, "Unknown id in osborne1_state::device_timer");
	}
}

TIMER_CALLBACK_MEMBER(osborne1_state::video_callback)
{
	int const y = machine().first_screen()->vpos();
	UINT8 const ra = y % 10;
	UINT8 const port_b = m_pia1->b_output();

	// Check for start/end of visible area and clear/set CA1 on video PIA
	if (y == 0)
	{
		m_scroll_y = port_b & 0x1F;
		m_pia1->ca1_w(0);
	}
	else if (y == 240)
	{
		m_pia1->ca1_w(1);
	}

	if (y < 240)
	{
		// Draw a line of the display
		UINT16 *p = &m_bitmap.pix16(y);
		bool const hires = m_screen_pac & m_resolution;
		UINT16 const row = ((m_scroll_y + (y / 10)) << 7) & 0xF80;

		// The derivation of the initial column is not obvious.  The 7-bit
		// column counter is preloaded near the beginning of the horizontal
		// blank period.  The initial column is offset by the number of
		// character clock periods in the horizontal blank period minus one
		// because it latches the value before it's displayed.  Using the
		// standard video display, there are 12 character clock periods in
		// the horizontal blank period, so subtracting 1 gives 0x0B.  Using
		// the SCREEN-PAC's high-resolution mode, the character clock is
		// twice the frequency giving 24 character clock periods in the
		// horizontal blanking period, so subtracting 1 gives 0x17.  Using
		// the standard video display, the column counter is preloaded with
		// the high 7 bits of the value from PIA1 PORTB.  The SCREEN-PAC
		// takes the two high bits of this value, but sets the low five bits
		// to a fixed value of 1 or 9 depending on the value of the HC-LEFT
		// signal (set by bit 1 of the value written to 0x2400).  Of course
		// it depends on the value wrapping around to zero when it counts
		// past 0x7F
		UINT16 const col = hires ? ((m_scroll_x & 0x60) + (m_hc_left ? 0x09 : 0x01) + 0x17) : (m_scroll_x + 0x0B);

		for (UINT16 x = 0; x < (hires ? 104 : 52); x++)
		{
			UINT16 const offs = row | ((col + x) & 0x7F);
			UINT8 const chr = m_ram->pointer()[0xF000 + offs];
			UINT8 const clr = (m_ram->pointer()[0x10000 + offs] & 0x80) ? 2 : 1;

			UINT8 const gfx = ((chr & 0x80) && (ra == 9)) ? 0xFF : m_p_chargen[(ra << 7) | (chr & 0x7F)];

			// Display a scanline of a character
			*p++ = BIT(gfx, 7) ? clr : 0;
			if (!hires) { p[0] = p[-1]; p++; }
			*p++ = BIT(gfx, 6) ? clr : 0;
			if (!hires) { p[0] = p[-1]; p++; }
			*p++ = BIT(gfx, 5) ? clr : 0;
			if (!hires) { p[0] = p[-1]; p++; }
			*p++ = BIT(gfx, 4) ? clr : 0;
			if (!hires) { p[0] = p[-1]; p++; }
			*p++ = BIT(gfx, 3) ? clr : 0;
			if (!hires) { p[0] = p[-1]; p++; }
			*p++ = BIT(gfx, 2) ? clr : 0;
			if (!hires) { p[0] = p[-1]; p++; }
			*p++ = BIT(gfx, 1) ? clr : 0;
			if (!hires) { p[0] = p[-1]; p++; }
			*p++ = BIT(gfx, 0) ? clr : 0;
			if (!hires) { p[0] = p[-1]; p++; }
		}
	}

	// The beeper is gated so it's active four out of every ten scanlines
	m_beep_state = (ra & 0x04) ? 1 : 0;
	m_speaker->level_w((BIT(port_b, 5) && m_beep_state) ? 1 : 0);

	// Check reset key if necessary - it affects NMI
	if (!m_ub6a_q)
		m_maincpu->set_input_line(INPUT_LINE_NMI, (m_btn_reset->read() & 0x80) ? CLEAR_LINE : ASSERT_LINE);

	m_video_timer->adjust(machine().first_screen()->time_until_pos(y + 1, 0));
}


TILE_GET_INFO_MEMBER(osborne1_state::get_tile_info)
{
	// The gfxdecode and tilemap aren't actually used for drawing, they just look nice in the F4 GFX viewer
	tileinfo.set(0, m_ram->pointer()[0xF000 | tile_index] & 0x7F, 0, 0);
}


bool osborne1_state::set_rom_mode(UINT8 value)
{
	if (value != m_rom_mode)
	{
		m_rom_mode = value;
		m_bank_0xxx->set_entry(m_rom_mode);
		m_bank_1xxx->set_entry(m_rom_mode);
		return true;
	}
	else
	{
		return false;
	}
}

bool osborne1_state::set_bit_9(UINT8 value)
{
	if (value != m_bit_9)
	{
		m_bit_9 = value;
		m_bank_fxxx->set_entry(m_bit_9);
		return true;
	}
	else
	{
		return false;
	}
}

void osborne1_state::update_irq()
{
	if (m_pia0->irq_a_state())
		m_maincpu->set_input_line_and_vector(INPUT_LINE_IRQ0, ASSERT_LINE, 0xF0);
	else if (m_pia1->irq_a_state())
		m_maincpu->set_input_line_and_vector(INPUT_LINE_IRQ0, ASSERT_LINE, 0xF8);
	else if (m_acia_irq_state)
		m_maincpu->set_input_line_and_vector(INPUT_LINE_IRQ0, ASSERT_LINE, 0xFC);
	else
		m_maincpu->set_input_line_and_vector(INPUT_LINE_IRQ0, CLEAR_LINE, 0xFE);
}

void osborne1_state::update_acia_rxc_txc()
{
	m_acia->write_rxc(m_acia_rxc_txc_state);
	m_acia->write_txc(m_acia_rxc_txc_state);
	attoseconds_t const dividend = (ATTOSECONDS_PER_SECOND / 100) * (m_acia_rxc_txc_state ? m_acia_rxc_txc_p_high : m_acia_rxc_txc_p_low);
	attoseconds_t const divisor = (15974400 / 100) / m_acia_rxc_txc_div;
	m_acia_rxc_txc_timer->adjust(attotime(0, dividend / divisor));
}
