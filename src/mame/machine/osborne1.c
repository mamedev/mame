// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
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

	// This isn't really accurate - bus fighting will occur for many values
	// since each peripheral only checks two bits.  We just return 0xFF for
	// any undocumented address.
	UINT8   data = 0xFF;
	switch (offset & 0x0F00)
	{
	case 0x100: /* Floppy */
		data = m_fdc->read(space, offset & 0x03);
		break;
	case 0x200: /* Keyboard */
		if (offset & 0x01)  data &= m_keyb_row0->read();
		if (offset & 0x02)  data &= m_keyb_row1->read();
		if (offset & 0x04)  data &= m_keyb_row3->read();
		if (offset & 0x08)  data &= m_keyb_row4->read();
		if (offset & 0x10)  data &= m_keyb_row5->read();
		if (offset & 0x20)  data &= m_keyb_row2->read();
		if (offset & 0x40)  data &= m_keyb_row6->read();
		if (offset & 0x80)  data &= m_keyb_row7->read();
		break;
	case 0x400: /* SCREEN-PAC */
		if (m_screen_pac) data &= 0xFB;
		break;
	case 0x900: /* IEEE488 PIA */
		data = m_pia0->read(space, offset & 0x03);
		break;
	case 0xA00: /* Serial */
		if (offset & 0x01) data = m_acia->data_r(space, 0);
		else data = m_acia->status_r(space, 0);
		break;
	case 0xC00: /* Video PIA */
		data = m_pia1->read(space, offset & 0x03);
		break;
	}
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
			m_hc_left = (data >> 1) & 0x01;
		}
		if ((offset & 0xC00) == 0xC00) // Video PIA
			m_pia1->write(space, offset & 0x03, data);
	}
}

WRITE8_MEMBER( osborne1_state::videoram_w )
{
	// Attribute RAM is only one bit wide - low seven bits are discarded and read back high
	if (m_bit_9) data |= 0x7F;
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

	    0
	    1
	    2
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
	m_fdc->dden_w(BIT(data, 0));

	data -= 0xea; // remove bias

	m_new_start_x = (data >> 1);
	if (m_new_start_x)
		m_new_start_x--;

	//logerror("Video pia port a write: %02X, density set to %s\n", data, data & 1 ? "FM" : "MFM" );
}

WRITE8_MEMBER( osborne1_state::video_pia_port_b_w )
{
	m_new_start_y = data & 0x1F;
	m_beep_state = BIT(data, 5);

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
		m_fdc->set_floppy(NULL);
	}

	//logerror("Video pia port b write: %02X\n", data );
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

	m_video_timer = timer_alloc(TIMER_VIDEO);
	m_video_timer->adjust(machine().first_screen()->time_until_pos(1, 0));

	m_acia_rxc_txc_timer = timer_alloc(TIMER_ACIA_RXC_TXC);

	timer_set(attotime::zero, TIMER_SETUP);
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

	m_resolution = 0;
	m_hc_left = 0;
	m_p_chargen = memregion( "chargen" )->base();

	for (unsigned i = 0; i < 0x1000; i++)
		m_ram->pointer()[0x10000 + i] |= 0x7F;
}

void osborne1_state::video_start()
{
	machine().first_screen()->register_screen_bitmap(m_bitmap);
}

UINT32 osborne1_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}


TIMER_CALLBACK_MEMBER(osborne1_state::video_callback)
{
	int const y = machine().first_screen()->vpos();
	UINT8 ra = 0;

	// Check for start/end of visible area and clear/set CA1 on video PIA
	if (y == 0)
		m_pia1->ca1_w(0);
	else if (y == 240)
		m_pia1->ca1_w(1);

	if (y < 240)
	{
		ra = y % 10;
		// Draw a line of the display
		bool const hires = m_screen_pac & m_resolution;
		UINT16 const row = (m_new_start_y + (y/10)) * 128 & 0xF80;
		UINT16 const col = (m_new_start_x & (hires ? 0x60 : 0x7F)) - ((hires && m_hc_left) ? 8 : 0);
		UINT16 *p = &m_bitmap.pix16(y);

		for ( UINT16 x = 0; x < (hires ? 104 : 52); x++ )
		{
			UINT16 offs = row | ((col + x) & 0x7F);
			UINT8 const chr = m_ram->pointer()[0xF000 + offs];
			UINT8 const dim = m_ram->pointer()[0x10000 + offs] & 0x80;

			UINT8 const gfx = ((chr & 0x80) && (ra == 9)) ? 0xFF : m_p_chargen[(ra << 7) | (chr & 0x7F)];

			// Display a scanline of a character
			*p++ = BIT(gfx, 7) ? ( dim ? 2 : 1 ) : 0;
			if (!hires) { p[0] = p[-1]; p++; }
			*p++ = BIT(gfx, 6) ? ( dim ? 2 : 1 ) : 0;
			if (!hires) { p[0] = p[-1]; p++; }
			*p++ = BIT(gfx, 5) ? ( dim ? 2 : 1 ) : 0;
			if (!hires) { p[0] = p[-1]; p++; }
			*p++ = BIT(gfx, 4) ? ( dim ? 2 : 1 ) : 0;
			if (!hires) { p[0] = p[-1]; p++; }
			*p++ = BIT(gfx, 3) ? ( dim ? 2 : 1 ) : 0;
			if (!hires) { p[0] = p[-1]; p++; }
			*p++ = BIT(gfx, 2) ? ( dim ? 2 : 1 ) : 0;
			if (!hires) { p[0] = p[-1]; p++; }
			*p++ = BIT(gfx, 1) ? ( dim ? 2 : 1 ) : 0;
			if (!hires) { p[0] = p[-1]; p++; }
			*p++ = BIT(gfx, 0) ? ( dim ? 2 : 1 ) : 0;
			if (!hires) { p[0] = p[-1]; p++; }
		}
	}

	if ((ra == 2) || (ra == 6))
		m_beep->set_state(m_beep_state);
	else
		m_beep->set_state(0);

	// Check reset key if necessary - it affects NMI
	if (!m_ub6a_q)
		m_maincpu->set_input_line(INPUT_LINE_NMI, (m_btn_reset->read() && 0x80) ? CLEAR_LINE : ASSERT_LINE);

	m_video_timer->adjust(machine().first_screen()->time_until_pos(y + 1, 0));
}

TIMER_CALLBACK_MEMBER(osborne1_state::setup_callback)
{
	m_beep->set_state( 0 );
	m_beep->set_frequency( 300 /* 60 * 240 / 2 */ );
	m_pia1->ca1_w(0);
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
	case TIMER_SETUP:
		setup_callback(ptr, param);
		break;
	default:
		assert_always(FALSE, "Unknown id in osborne1_state::device_timer");
	}
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
