// license:BSD-3-Clause
// copyright-holders:Wilbert Pol,Vas Crabb
/***************************************************************************

There are three IRQ sources:
- IRQ0 = IRQ from the serial ACIA
- IRQ1 = IRQA from the video PIA
- IRQ2 = IRQA from the IEEE488 PIA

***************************************************************************/

#include "emu.h"
#include "osborne1.h"


void osborne1_state::bank_0xxx_w(offs_t offset, u8 data)
{
	if (!rom_mode())
		m_ram->pointer()[offset] = data;
}

void osborne1_state::bank_1xxx_w(offs_t offset, u8 data)
{
	if (!rom_mode())
		m_ram->pointer()[0x1000 + offset] = data;
}

u8 osborne1_state::bank_2xxx_3xxx_r(offs_t offset)
{
	if (!rom_mode())
		return m_ram->pointer()[0x2000 + offset];

	// Since each peripheral only checks two bits, many addresses will
	// result in multiple peripherals attempting to drive the bus.  This is
	// simulated by ANDing all the values together.
	u8 data = 0xFF;
	if ((offset & 0x900) == 0x100) // Floppy
		data &= m_fdc->read(offset & 0x03);
	if ((offset & 0x900) == 0x900) // IEEE488 PIA
		data &= m_pia0->read(offset & 0x03);
	if ((offset & 0xA00) == 0x200) // Keyboard
	{
		for (unsigned b = 0; 8 > b; ++b)
		{
			if (BIT(offset, b))
				data &= m_keyb_row[b]->read();
		}
	}
	if ((offset & 0xA00) == 0xA00) // Serial
	{
		data &= m_acia->read(offset & 0x01);
	}
	if ((offset & 0xC00) == 0xC00) // Video PIA
		data &= m_pia1->read(offset & 0x03);
	return data;
}

void osborne1_state::bank_2xxx_3xxx_w(offs_t offset, u8 data)
{
	if (!rom_mode())
	{
		m_ram->pointer()[0x2000 + offset] = data;
	}
	else
	{
		// Handle writes to the I/O area
		if ((offset & 0x900) == 0x100) // Floppy
			m_fdc->write(offset & 0x03, data);
		if ((offset & 0x900) == 0x900) // IEEE488 PIA
			m_pia0->write(offset & 0x03, data);
		if ((offset & 0xA00) == 0xA00) // Serial
		{
			m_acia->write(offset & 0x01, data);
		}
		if ((offset & 0xC00) == 0xC00) // Video PIA
			m_pia1->write(offset & 0x03, data);
	}
}

u8 osborne1sp_state::bank_2xxx_3xxx_r(offs_t offset)
{
	u8 data = osborne1_state::bank_2xxx_3xxx_r(offset);
	if (!rom_mode())
		return data;

	if ((offset & 0xC00) == 0x400) // SCREEN-PAC
	{
		data &= 0xFB;
	}
	return data;
}

void osborne1sp_state::bank_2xxx_3xxx_w(offs_t offset, u8 data)
{
	osborne1_state::bank_2xxx_3xxx_w(offset, data);

	if (rom_mode())
	{
		if ((offset & 0xC00) == 0x400) // SCREEN-PAC
		{
			m_resolution = BIT(data, 0);
			m_hc_left = BIT(~data, 1);
		}
	}
}

void osborne1_state::videoram_w(offs_t offset, u8 data)
{
	// Attribute RAM is only one bit wide - low seven bits are discarded and read back high
	if (m_bit_9)
		data |= 0x7F;
	else
		m_tilemap->mark_tile_dirty(offset);
	reinterpret_cast<u8 *>(m_bank_fxxx->base())[offset] = data;
}

u8 osborne1_state::opcode_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		// Update the flipflops that control bank selection and NMI
		u8 const new_ub6a_q = (m_btn_reset->read() & 0x80) ? 1 : 0;
		if (!rom_mode())
		{
			set_rom_mode(m_ub4a_q ? 0 : 1);
			m_ub4a_q = m_ub6a_q;
		}
		m_ub6a_q = new_ub6a_q;
		m_maincpu->set_input_line(INPUT_LINE_NMI, m_ub6a_q ? CLEAR_LINE : ASSERT_LINE);
	}

	// Now that's sorted out we can call the normal read handler
	return m_mem_cache.read_byte(offset);
}

void osborne1_state::bankswitch_w(offs_t offset, u8 data)
{
	switch (offset)
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
	if (!rom_mode())
		set_rom_mode(m_ub4a_q ? 0 : 1);
	m_ub4a_q = 0;
	m_ub6a_q = (m_btn_reset->read() & 0x80) ? 1 : 0;
	m_maincpu->set_input_line(INPUT_LINE_NMI, m_ub6a_q ? CLEAR_LINE : ASSERT_LINE);
}


u8 osborne1_state::ieee_pia_pb_r()
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
	u8 data = 0;

	data |= m_ieee->eoi_r() << 3;
	data |= m_ieee->dav_r() << 5;
	data |= m_ieee->ndac_r() << 6;
	data |= m_ieee->nrfd_r() << 7;

	return data;
}

void osborne1_state::ieee_pia_pb_w(u8 data)
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
	m_ieee->host_eoi_w(BIT(data, 3));
	m_ieee->host_atn_w(BIT(data, 4));
	m_ieee->host_dav_w(BIT(data, 5));
	m_ieee->host_ndac_w(BIT(data, 6));
	m_ieee->host_nrfd_w(BIT(data, 7));
}

WRITE_LINE_MEMBER( osborne1_state::ieee_pia_irq_a_func )
{
	update_irq();
}


void osborne1_state::video_pia_port_a_w(u8 data)
{
	m_scroll_x = data >> 1;

	m_fdc->dden_w(BIT(data, 0));
}

void osborne1_state::video_pia_port_b_w(u8 data)
{
	m_speaker->level_w((BIT(data, 5) && m_beep_state) ? 1 : 0);

	if (BIT(data, 6))
	{
		m_fdc->set_floppy(m_floppy0->get_device());
		m_floppy0->get_device()->mon_w(0);
	}
	else if (BIT(data, 7))
	{
		m_fdc->set_floppy(m_floppy1->get_device());
		m_floppy1->get_device()->mon_w(0);
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


INPUT_CHANGED_MEMBER( osborne1_state::reset_key )
{
	// This key affects NMI
	if (!m_ub6a_q)
		m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? CLEAR_LINE : ASSERT_LINE);
}


void osborne1_state::machine_start()
{
	m_bank_0xxx->configure_entries(0, 1, m_ram->pointer(), 0);
	m_bank_0xxx->configure_entries(1, 1, m_region_maincpu->base(), 0);
	m_bank_1xxx->configure_entries(0, 1, m_ram->pointer() + 0x1000, 0);
	m_bank_1xxx->configure_entries(1, 1, m_region_maincpu->base(), 0);
	m_bank_fxxx->configure_entries(0, 1, m_ram->pointer() + 0xF000, 0);
	m_bank_fxxx->configure_entries(1, 1, m_ram->pointer() + 0x10000, 0);

	m_video_timer = timer_alloc(FUNC(osborne1_state::video_callback), this);
	m_tilemap = &machine().tilemap().create(
			*m_gfxdecode,
			tilemap_get_info_delegate(*this, FUNC(osborne1_state::get_tile_info)), TILEMAP_SCAN_ROWS,
			8, 10, 128, 32);

	m_acia_rxc_txc_timer = timer_alloc(FUNC(osborne1_state::acia_rxc_txc_callback), this);

	m_maincpu->space(AS_PROGRAM).cache(m_mem_cache);

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

	save_item(NAME(m_acia_irq_state));
	save_item(NAME(m_acia_rxc_txc_state));
}

void osborne1_state::machine_reset()
{
	// Refresh configuration
	switch (m_cnf->read() & 0x03)
	{
	case 0x00:
		m_acia_rxc_txc_div      = 16;
		m_acia_rxc_txc_p_low    = 23;
		m_acia_rxc_txc_p_high   = 29;
		break;
	case 0x01:
		m_acia_rxc_txc_div      = 16;
		m_acia_rxc_txc_p_low    = 9;
		m_acia_rxc_txc_p_high   = 15;
		break;
	case 0x02:
		m_acia_rxc_txc_div      = 16;
		m_acia_rxc_txc_p_low    = 5;
		m_acia_rxc_txc_p_high   = 8;
		break;
	case 0x03:
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

	// The low bits of attribute RAM are not physically present and hence always read high
	for (unsigned i = 0; i < 0x1000; i++)
		m_ram->pointer()[0x10000 + i] |= 0x7F;
}

void osborne1_state::video_start()
{
	m_video_timer->adjust(m_screen->time_until_pos(1, 0));
}

void osborne1sp_state::machine_start()
{
	osborne1_state::machine_start();

	save_item(NAME(m_resolution));
	save_item(NAME(m_hc_left));
}

void osborne1sp_state::machine_reset()
{
	osborne1_state::machine_reset();

	// Reset video hardware
	m_resolution = 0;
	m_hc_left = 1;
}

template <int Width, unsigned Scale>
inline void osborne1_state::draw_rows(uint16_t col, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int y = cliprect.min_y; cliprect.max_y >= y; ++y)
	{
		// Vertical scroll is latched at the start of the visible area
		if (0 == y)
			m_scroll_y = m_pia1->b_output() & 0x1F;

		// Draw a line of the display
		u8 const ra(y % 10);
		uint16_t *p(&bitmap.pix(y));
		uint16_t const row(((m_scroll_y + (y / 10)) << 7) & 0x0F80);

		for (uint16_t x = 0; Width > x; ++x)
		{
			uint16_t const offs(row | ((col + x) & 0x7F));
			u8 const chr(m_ram->pointer()[0xF000 + offs]);
			u8 const clr((m_ram->pointer()[0x10000 + offs] & 0x80) ? 2 : 1);

			u8 const gfx(((chr & 0x80) && (ra == 9)) ? 0xFF : m_p_chargen[(ra << 7) | (chr & 0x7F)]);

			// Display a scanline of a character
			for (unsigned b = 0; 8 > b; ++b)
			{
				uint16_t const pixel(BIT(gfx, 7 - b) ? clr : 0);
				for (unsigned i = 0; Scale > i; ++i)
					*p++ = pixel;
			}
		}
	}
}

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
// past 0x7F.

uint32_t osborne1_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	draw_rows<52, 1>(scroll_x() + 0x0B, bitmap, cliprect);

	return 0;
}

uint32_t osborne1sp_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
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
	// past 0x7F.
	if (m_resolution)
		draw_rows<104, 1>((scroll_x() & 0x60) + (m_hc_left ? 0x09 : 0x01) + 0x17, bitmap, cliprect);
	else
		draw_rows<52, 2>(scroll_x() + 0x0B, bitmap, cliprect);

	return 0;
}


TIMER_CALLBACK_MEMBER(osborne1_state::video_callback)
{
	int const y(m_screen->vpos());
	u8 const ra(y % 10);

	// The beeper is gated so it's active four out of every ten scanlines
	m_beep_state = (ra & 0x04) ? 1 : 0;
	m_speaker->level_w((m_beep_state && BIT(m_pia1->b_output(), 5)) ? 1 : 0);

	int const next((y - ra) + ((ra < 4) ? 4 : (ra < 8) ? 8 : 14));
	m_video_timer->adjust(m_screen->time_until_pos((m_screen->height() > next) ? next : 0, 0));
}

TIMER_CALLBACK_MEMBER(osborne1_state::acia_rxc_txc_callback)
{
	m_acia_rxc_txc_state = m_acia_rxc_txc_state ? 0 : 1;
	update_acia_rxc_txc();
}


TILE_GET_INFO_MEMBER(osborne1_state::get_tile_info)
{
	// The gfxdecode and tilemap aren't actually used for drawing, they just look nice in the F4 GFX viewer
	tileinfo.set(0, m_ram->pointer()[0xF000 | tile_index] & 0x7F, 0, 0);
}


bool osborne1_state::set_rom_mode(u8 value)
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

bool osborne1_state::set_bit_9(u8 value)
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
		m_maincpu->set_input_line_and_vector(INPUT_LINE_IRQ0, ASSERT_LINE, 0xF0); // Z80
	else if (m_pia1->irq_a_state())
		m_maincpu->set_input_line_and_vector(INPUT_LINE_IRQ0, ASSERT_LINE, 0xF8); // Z80
	else if (m_acia_irq_state)
		m_maincpu->set_input_line_and_vector(INPUT_LINE_IRQ0, ASSERT_LINE, 0xFC); // Z80
	else
		m_maincpu->set_input_line_and_vector(INPUT_LINE_IRQ0, CLEAR_LINE, 0xFE); // Z80
}

void osborne1_state::update_acia_rxc_txc()
{
	m_acia->write_rxc(m_acia_rxc_txc_state);
	m_acia->write_txc(m_acia_rxc_txc_state);
	attoseconds_t const dividend = (ATTOSECONDS_PER_SECOND / 100) * (m_acia_rxc_txc_state ? m_acia_rxc_txc_p_high : m_acia_rxc_txc_p_low);
	attoseconds_t const divisor = (15974400 / 100) / m_acia_rxc_txc_div;
	m_acia_rxc_txc_timer->adjust(attotime(0, dividend / divisor));
}


MC6845_UPDATE_ROW(osborne1nv_state::crtc_update_row)
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	uint16_t const base = (ma >> 1) & 0xF80;
	uint32_t *p = &bitmap.pix(y);
	for (u8 x = 0; x < x_count; ++x)
	{
		uint16_t const offset = base | ((ma + x) & 0x7F);
		u8 const chr = m_ram->pointer()[0xF000 | offset];
		u8 const clr = BIT(m_ram->pointer()[0x10000 | offset], 7) ? 2 : 1;

		u8 const gfx = ((chr & 0x80) && (ra == 9)) ? 0xFF : m_p_nuevo[(ra << 7) | (chr & 0x7F)];

		for (unsigned bit = 0; 8 > bit; ++bit)
			*p++ = palette[BIT(gfx, 7 - bit) ? clr : 0];
	}
}

MC6845_ON_UPDATE_ADDR_CHANGED(osborne1nv_state::crtc_update_addr_changed)
{
}
