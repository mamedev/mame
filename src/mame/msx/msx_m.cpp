// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*
 * msx.c: MSX emulation
 *
 * Copyright (C) 2004 Sean Young
 *
 * Todo:
 *
 * - fix mouse support
 * - cassette support doesn't work
 * - Ensure changing cartridge after boot works
 * - wd2793, nms8255
 */

#include "emu.h"
#include "msx.h"

#define VERBOSE 0


void msx_state::machine_reset()
{
	msx_memory_reset ();
	msx_memory_map_all ();
}


void msx_state::machine_start()
{
	m_leds.resolve();
	m_port_c_old = 0xff;
}


void msx2_state::machine_start()
{
	msx_state::machine_start();

	for (msx_switched_interface &switched : device_interface_enumerator<msx_switched_interface>(*this))
		m_switched.push_back(&switched);

	save_item(NAME(m_rtc_latch));
}

/* A hack to add 1 wait cycle in each opcode fetch.
   Possibly worth not to use custom table at all but adjust desired icount
   directly in m_opcodes.read_byte handler. */
static const uint8_t cc_op[0x100] = {
	4+1,10+1, 7+1, 6+1, 4+1, 4+1, 7+1, 4+1, 4+1,11+1, 7+1, 6+1, 4+1, 4+1, 7+1, 4+1,
	8+1,10+1, 7+1, 6+1, 4+1, 4+1, 7+1, 4+1,12+1,11+1, 7+1, 6+1, 4+1, 4+1, 7+1, 4+1,
	7+1,10+1,16+1, 6+1, 4+1, 4+1, 7+1, 4+1, 7+1,11+1,16+1, 6+1, 4+1, 4+1, 7+1, 4+1,
	7+1,10+1,13+1, 6+1,11+1,11+1,10+1, 4+1, 7+1,11+1,13+1, 6+1, 4+1, 4+1, 7+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1,
	7+1, 7+1, 7+1, 7+1, 7+1, 7+1, 4+1, 7+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 7+1, 4+1,
	5+1,10+1,10+1,10+1,10+1,11+1, 7+1,11+1, 5+1,10+1,10+1, 4+1,10+1,17+1, 7+1,11+1,
	5+1,10+1,10+1,11+1,10+1,11+1, 7+1,11+1, 5+1, 4+1,10+1,11+1,10+1, 4+1, 7+1,11+1,
	5+1,10+1,10+1,19+1,10+1,11+1, 7+1,11+1, 5+1, 4+1,10+1, 4+1,10+1, 4+1, 7+1,11+1,
	5+1,10+1,10+1, 4+1,10+1,11+1, 7+1,11+1, 5+1, 6+1,10+1, 4+1,10+1, 4+1, 7+1,11+1
};

static const uint8_t cc_cb[0x100] = {
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 8+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 8+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 8+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 8+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 8+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 8+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 8+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 8+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1,
	4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,11+1, 4+1
};

static const uint8_t cc_ed[0x100] = {
	 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,
	 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,
	 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,
	 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,
	 8+1, 8+1,15+2,16+1, 4+1,14+2, 4+1, 5+1, 8+1, 8+1,15+2,16+1, 4+1,14+2, 4+1, 5+1,
	 8+1, 8+1,15+2,16+1, 4+1,14+2, 4+1, 5+1, 8+1, 8+1,15+2,16+1, 4+1,14+2, 4+1, 5+1,
	 8+1, 8+1,15+2,16+1, 4+1,14+2, 4+1,14+1, 8+1, 8+1,15+2,16+1, 4+1,14+2, 4+1,14+1,
	 8+1, 8+1,15+2,16+1, 4+1,14+2, 4+1, 4+1, 8+1, 8+1,15+2,16+1, 4+1,14+2, 4+1, 4+1,
	 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,
	 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,
	12+1,12+1,12+1,12+1, 4+1, 4+1, 4+1, 4+1,12+1,12+1,12+1,12+1, 4+1, 4+1, 4+1, 4+1,
	12+1,12+1,12+1,12+1, 4+1, 4+1, 4+1, 4+1,12+1,12+1,12+1,12+1, 4+1, 4+1, 4+1, 4+1,
	 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,
	 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,
	 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,
	 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1
};

static const uint8_t cc_xy[0x100] = {
	 4+1,10+1, 7+1, 6+1, 4+1, 4+1, 7+1, 4+1, 4+1,11+1, 7+1, 6+1, 4+1, 4+1, 7+1, 4+1,
	 8+1,10+1, 7+1, 6+1, 4+1, 4+1, 7+1, 4+1,12+1,11+1, 7+1, 6+1, 4+1, 4+1, 7+1, 4+1,
	 7+1,10+1,16+1, 6+1, 4+1, 4+1, 7+1, 4+1, 7+1,11+1,16+1, 6+1, 4+1, 4+1, 7+1, 4+1,
	 7+1,10+1,13+1, 6+1,19+1,19+1,15+1, 4+1, 7+1,11+1,13+1, 6+1, 4+1, 4+1, 7+1, 4+1,
	 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,15+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,15+1, 4+1,
	 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,15+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,15+1, 4+1,
	 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,15+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,15+1, 4+1,
	15+1,15+1,15+1,15+1,15+1,15+1, 4+1,15+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,15+1, 4+1,
	 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,15+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,15+1, 4+1,
	 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,15+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,15+1, 4+1,
	 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,15+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,15+1, 4+1,
	 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,15+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1, 4+1,15+1, 4+1,
	 5+1,10+1,10+1,10+1,10+1,11+1, 7+1,11+1, 5+1,10+1,10+1, 7+1,10+1,17+1, 7+1,11+1,
	 5+1,10+1,10+1,11+1,10+1,11+1, 7+1,11+1, 5+1, 4+1,10+1,11+1,10+1, 4+1, 7+1,11+1,
	 5+1,10+1,10+1,19+1,10+1,11+1, 7+1,11+1, 5+1, 4+1,10+1, 4+1,10+1, 4+1, 7+1,11+1,
	 5+1,10+1,10+1, 4+1,10+1,11+1, 7+1,11+1, 5+1, 6+1,10+1, 4+1,10+1, 4+1, 7+1,11+1
};

/* extra cycles if jr/jp/call taken and 'interrupt latency' on rst 0-7 */
static const uint8_t cc_ex[0x100] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* DJNZ */
	5, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, /* JR NZ/JR Z */
	5, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, /* JR NC/JR C */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	5, 5, 5, 5, 0, 0, 0, 0, 5, 5, 5, 5, 0, 0, 0, 0, /* LDIR/CPIR/INIR/OTIR LDDR/CPDR/INDR/OTDR */
	6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2,
	6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2,
	6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2,
	6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2+1
};

void msx_state::driver_start()
{
	m_maincpu->set_input_line_vector(0, 0xff); // Z80

	msx_memory_init();

	m_maincpu->z80_set_cycle_tables(cc_op, cc_cb, cc_ed, cc_xy, nullptr, cc_ex);

	save_item(NAME(m_psg_b));
	save_item(NAME(m_mouse));
	save_item(NAME(m_mouse_stat));
	save_item(NAME(m_kanji_latch));
	save_item(NAME(m_slot_expanded));
	save_item(NAME(m_primary_slot));
	save_item(NAME(m_secondary_slot));
	save_item(NAME(m_port_c_old));
	save_item(NAME(m_keylatch));
}

void msx_state::device_post_load()
{
	for (int page = 0; page < 4; page++)
	{
		int slot_primary = (m_primary_slot >> (page * 2)) & 3;
		int slot_secondary = (m_secondary_slot[slot_primary] >> (page * 2)) & 3;

		m_current_page[page] = m_all_slots[slot_primary][slot_secondary][page];
	}
}

INTERRUPT_GEN_MEMBER(msx_state::msx_interrupt)
{
	m_mouse[0] = m_io_mouse[0]->read();
	m_mouse_stat[0] = -1;
	m_mouse[1] = m_io_mouse[1]->read();
	m_mouse_stat[1] = -1;
}

/*
** The I/O functions
*/


uint8_t msx_state::msx_psg_port_a_r()
{
	uint8_t data = (m_cassette->input() > 0.0038 ? 0x80 : 0);

	if ( (m_psg_b ^ m_io_dsw->read() ) & 0x40)
	{
		/* game port 2 */
		uint8_t inp = m_io_joy[1]->read();
		if ( !(inp & 0x80) )
		{
			/* joystick */
			data |= ( inp & 0x7f );
		}
		else
		{
			/* mouse */
			data |= ( inp & 0x70 );
			if (m_mouse_stat[1] < 0)
				data |= 0xf;
			else
				data |= ~(m_mouse[1] >> (4*m_mouse_stat[1]) ) & 15;
		}
	}
	else
	{
		/* game port 1 */
		uint8_t inp = m_io_joy[0]->read();
		if ( !(inp & 0x80) )
		{
			/* joystick */
			data |= ( inp & 0x7f );
		}
		else
		{
			/* mouse */
			data |= ( inp & 0x70 );
			if (m_mouse_stat[0] < 0)
				data |= 0xf;
			else
				data |= ~(m_mouse[0] >> (4*m_mouse_stat[0]) ) & 15;
		}
	}

	return data;
}

uint8_t msx_state::msx_psg_port_b_r()
{
	return m_psg_b;
}

void msx_state::msx_psg_port_a_w(uint8_t data)
{
}

void msx_state::msx_psg_port_b_w(uint8_t data)
{
	/* Arabic or kana mode led */
	if ( (data ^ m_psg_b) & 0x80)
		m_leds[1] = BIT(~data, 7);

	if ( (m_psg_b ^ data) & 0x10)
	{
		if (++m_mouse_stat[0] > 3) m_mouse_stat[0] = -1;
	}
	if ( (m_psg_b ^ data) & 0x20)
	{
		if (++m_mouse_stat[1] > 3) m_mouse_stat[1] = -1;
	}

	m_psg_b = data;
}


/*
** RTC functions
*/

void msx2_state::msx_rtc_latch_w(uint8_t data)
{
	m_rtc_latch = data & 15;
}

void msx2_state::msx_rtc_reg_w(uint8_t data)
{
	m_rtc->write(m_rtc_latch, data);
}

uint8_t msx2_state::msx_rtc_reg_r()
{
	return m_rtc->read(m_rtc_latch);
}


/*
** The PPI functions
*/

void msx_state::msx_ppi_port_a_w(uint8_t data)
{
	m_primary_slot = data;

	if (VERBOSE)
		logerror ("write to primary slot select: %02x\n", m_primary_slot);
	msx_memory_map_all ();
}

void msx_state::msx_ppi_port_c_w(uint8_t data)
{
	m_keylatch = data & 0x0f;

	/* caps lock */
	if ( BIT(m_port_c_old ^ data, 6) )
		m_leds[0] = BIT(~data, 6);

	/* key click */
	if ( BIT(m_port_c_old ^ data, 7) )
		m_dac->write(BIT(data, 7));

	/* cassette motor on/off */
	if ( BIT(m_port_c_old ^ data, 4) )
		m_cassette->change_state(BIT(data, 4) ? CASSETTE_MOTOR_DISABLED : CASSETTE_MOTOR_ENABLED, CASSETTE_MASK_MOTOR);

	/* cassette signal write */
	if ( BIT(m_port_c_old ^ data, 5) )
		m_cassette->output(BIT(data, 5) ? -1.0 : 1.0);

	m_port_c_old = data;
}

uint8_t msx_state::msx_ppi_port_b_r()
{
	uint8_t result = 0xff;
	int row, data;

	row = m_keylatch;
	if (row <= 10)
	{
		data = m_io_key[row / 2]->read();

		if (BIT(row, 0))
			data >>= 8;
		result = data & 0xff;
	}
	return result;
}

/************************************************************************
 *
 * New memory emulation !!
 *
 ***********************************************************************/

void msx_state::install_slot_pages(uint8_t prim, uint8_t sec, uint8_t page, uint8_t numpages, msx_internal_slot_interface &device)
{
	for ( int i = page; i < std::min(page + numpages, 4); i++ )
	{
		m_all_slots[prim][sec][i] = &device;
	}
	if ( sec )
	{
		m_slot_expanded[prim] = true;
	}
}

void msx_state::msx_memory_init()
{
	int count_populated_pages = 0;

	// Populate all unpopulated slots with the dummy interface
	for (auto & elem : m_all_slots)
	{
		for ( int sec = 0; sec < 4; sec++ )
		{
			for ( int page = 0; page < 4; page++ )
			{
				if ( elem[sec][page] == nullptr )
				{
					elem[sec][page] = &m_empty_slot;
				}
				else
				{
					count_populated_pages++;
				}
			}
		}
	}

	if ( count_populated_pages == 0 ) {
		fatalerror("No msx slot layout defined for this system!\n");
	}
}

void msx_state::msx_memory_reset ()
{
	m_primary_slot = 0;

	for (auto & elem : m_secondary_slot)
	{
		elem = 0;
	}
}

void msx_state::msx_memory_map_page (uint8_t page)
{
	int slot_primary = (m_primary_slot >> (page * 2)) & 3;
	int slot_secondary = (m_secondary_slot[slot_primary] >> (page * 2)) & 3;

	m_current_page[page] = m_all_slots[slot_primary][slot_secondary][page];
}

void msx_state::msx_memory_map_all ()
{
	for (uint8_t i=0; i<4; i++)
		msx_memory_map_page (i);
}

uint8_t msx_state::msx_mem_read(offs_t offset)
{
	return m_current_page[offset >> 14]->read(offset);
}

void msx_state::msx_mem_write(offs_t offset, uint8_t data)
{
	m_current_page[offset >> 14]->write(offset, data);
}

void msx_state::msx_sec_slot_w(uint8_t data)
{
	int slot = m_primary_slot >> 6;
	if (m_slot_expanded[slot])
	{
		if (VERBOSE)
			logerror ("write to secondary slot %d select: %02x\n", slot, data);

		m_secondary_slot[slot] = data;
		msx_memory_map_all ();
	}
	else
		m_current_page[3]->write(0xffff, data);
}

uint8_t msx_state::msx_sec_slot_r()
{
	int slot = m_primary_slot >> 6;

	if (m_slot_expanded[slot])
	{
		return ~m_secondary_slot[slot];
	}
	else
	{
		return m_current_page[3]->read(0xffff);
	}
}

uint8_t msx_state::msx_kanji_r(offs_t offset)
{
	uint8_t result = 0xff;

	if (offset && m_region_kanji)
	{
		int latch = m_kanji_latch;
		result = m_region_kanji->as_u8(latch++);

		m_kanji_latch &= ~0x1f;
		m_kanji_latch |= latch & 0x1f;
	}
	return result;
}

void msx_state::msx_kanji_w(offs_t offset, uint8_t data)
{
	if (offset)
		m_kanji_latch = (m_kanji_latch & 0x007E0) | ((data & 0x3f) << 11);
	else
		m_kanji_latch = (m_kanji_latch & 0x1f800) | ((data & 0x3f) << 5);
}

uint8_t msx2_state::msx_switched_r(offs_t offset)
{
	uint8_t data = 0xff;

	for (int i = 0; i < m_switched.size(); i++)
	{
		data &= m_switched[i]->switched_read(offset);
	}

	return data;
}

void msx2_state::msx_switched_w(offs_t offset, uint8_t data)
{
	for (int i = 0; i < m_switched.size(); i++)
	{
		m_switched[i]->switched_write(offset, data);
	}
}
