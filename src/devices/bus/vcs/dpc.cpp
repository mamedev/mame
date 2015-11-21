// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***************************************************************************


 Atari 2600 cart with DPC chip (Pitfall II)

***************************************************************************/


#include "emu.h"
#include "dpc.h"


// DPC device

const device_type ATARI_DPC = &device_creator<dpc_device>;


dpc_device::dpc_device(const machine_config& mconfig, const char* tag, device_t* owner, UINT32 clock) :
	device_t(mconfig, ATARI_DPC, "Atari DCP", tag, owner, clock, "atari_dcp", __FILE__),
	m_movamt(0),
	m_latch_62(0),
	m_latch_64(0),
	m_dlc(0),
	m_shift_reg(0),
	m_displaydata(nullptr),
	m_oscillator(nullptr)
{
}


void dpc_device::device_start()
{
	m_oscillator = timer_alloc(TIMER_OSC);
	m_oscillator->reset();

	for (int i = 0; i < 8; i++)
	{
		save_item(NAME(m_df[i].top), i);
		save_item(NAME(m_df[i].bottom), i);
		save_item(NAME(m_df[i].low), i);
		save_item(NAME(m_df[i].high), i);
		save_item(NAME(m_df[i].flag), i);
		save_item(NAME(m_df[i].music_mode), i);
		save_item(NAME(m_df[i].osc_clk), i);
	}

	save_item(NAME(m_movamt));
	save_item(NAME(m_latch_62));
	save_item(NAME(m_latch_64));
	save_item(NAME(m_dlc));
	save_item(NAME(m_shift_reg));
}

void dpc_device::device_reset()
{
	for (int data_fetcher = 0; data_fetcher < 8; data_fetcher++)
	{
		m_df[data_fetcher].osc_clk = 0;
		m_df[data_fetcher].flag = 0;
		m_df[data_fetcher].music_mode = 0;
	}
	m_oscillator->adjust(attotime::from_hz(18400), 0, attotime::from_hz(18400));

}

void dpc_device::check_flag(UINT8 data_fetcher)
{
	/* Set flag when low counter equals top */
	if (m_df[data_fetcher].low == m_df[data_fetcher].top)
		m_df[data_fetcher].flag = 1;

	/* Reset flag when low counter equals bottom */
	if (m_df[data_fetcher].low == m_df[data_fetcher].bottom)
		m_df[data_fetcher].flag = 0;
}

void dpc_device::decrement_counter(UINT8 data_fetcher)
{
	m_df[data_fetcher].low -= 1;
	if (m_df[data_fetcher].low == 0xff)
	{
		m_df[data_fetcher].high -= 1;
		if (data_fetcher > 4 && m_df[data_fetcher].music_mode)
			m_df[data_fetcher].low = m_df[data_fetcher].top;
	}

	check_flag(data_fetcher);
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void dpc_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id == TIMER_OSC)
	{
		// callback
		for (int data_fetcher = 5; data_fetcher < 8; data_fetcher++)
		{
			if (m_df[data_fetcher].osc_clk)
			{
				decrement_counter(data_fetcher);
			}
		}
	}
}


//-------------------------------------------------
//  Read / Write accesses
//-------------------------------------------------

READ8_MEMBER(dpc_device::read)
{
	static const UINT8 dpc_amplitude[8] = { 0x00, 0x04, 0x05, 0x09, 0x06, 0x0a, 0x0b, 0x0f };
	UINT8   data_fetcher = offset & 0x07;
	UINT8   data = 0xff;

	//logerror("%04X: Read from DPC offset $%02X\n", machine().device<cpu_device>("maincpu")->pc(), offset);
	if (offset < 0x08)
	{
		switch(offset & 0x06)
		{
			case 0x00:      // Random number generator
			case 0x02:
				return m_shift_reg;
			case 0x04:      // Sound value, MOVAMT value AND'd with Draw Line Carry; with Draw Line Add
				m_latch_62 = m_latch_64;
			case 0x06:      // Sound value, MOVAMT value AND'd with Draw Line Carry; without Draw Line Add
				m_latch_64 = m_latch_62 + m_df[4].top;
				m_dlc = (m_latch_62 + m_df[4].top > 0xff) ? 1 : 0;
				data = 0;
				if (m_df[5].music_mode && m_df[5].flag)
					data |= 0x01;

				if (m_df[6].music_mode && m_df[6].flag)
					data |= 0x02;

				if (m_df[7].music_mode && m_df[7].flag)
					data |= 0x04;

				return (m_dlc ? m_movamt & 0xf0 : 0) | dpc_amplitude[data];
		}
	}
	else
	{
		UINT8 display_data = m_displaydata[(~((m_df[data_fetcher].low | (m_df[data_fetcher].high << 8))) & 0x7ff)];

		switch (offset & 0x38)
		{
			case 0x08:          // display data
				data = display_data;
				break;
			case 0x10:          // display data AND'd w/flag
				data = m_df[data_fetcher].flag ? display_data : 0x00;
				break;
			case 0x18:          // display data AND'd w/flag, nibbles swapped
				data = m_df[data_fetcher].flag ? BITSWAP8(display_data,3,2,1,0,7,6,5,4) : 0x00;
				break;
			case 0x20:          // display data AND'd w/flag, byte reversed
				data = m_df[data_fetcher].flag ? BITSWAP8(display_data,0,1,2,3,4,5,6,7) : 0x00;
				break;
			case 0x28:          // display data AND'd w/flag, rotated right
				data = m_df[data_fetcher].flag ? (display_data >> 1) : 0x00;
				break;
			case 0x30:          // display data AND'd w/flag, rotated left
				data = m_df[data_fetcher].flag ? (display_data << 1) : 0x00;
				break;
			case 0x38:          // flag
				data = m_df[data_fetcher].flag ? 0xff : 0x00;
				break;
		}

		if (data_fetcher < 5 || !m_df[data_fetcher].osc_clk)
		{
			decrement_counter(data_fetcher);
		}
	}
	return data;
}

WRITE8_MEMBER(dpc_device::write)
{
	UINT8 data_fetcher = offset & 0x07;

	switch (offset & 0x38)
	{
		case 0x00:          // Top count
			m_df[data_fetcher].top = data;
			m_df[data_fetcher].flag = 0;
			check_flag(data_fetcher);
			break;
		case 0x08:          // Bottom count
			m_df[data_fetcher].bottom = data;
			check_flag(data_fetcher);
			break;
		case 0x10:          // Counter low
			m_df[data_fetcher].low = data;
			if (data_fetcher == 4)
				m_latch_64 = data;

			if (data_fetcher > 4 && m_df[data_fetcher].music_mode)
				m_df[data_fetcher].low = m_df[data_fetcher].top;

			check_flag(data_fetcher);
			break;
		case 0x18:          // Counter high
			m_df[data_fetcher].high = data;
			m_df[data_fetcher].music_mode = data & 0x10;
			m_df[data_fetcher].osc_clk = data & 0x20;
			if (data_fetcher > 4 && m_df[data_fetcher].music_mode && m_df[data_fetcher].low == 0xff)
			{
				m_df[data_fetcher].low = m_df[data_fetcher].top;
				check_flag(data_fetcher);
			}
			break;
		case 0x20:          // Draw line movement value / MOVAMT
			m_movamt = data;
			break;
		case 0x28:          // Not used
			logerror("%04X: Write to unused DPC register $%02X, data $%02X\n", machine().device<cpu_device>("maincpu")->pc(), offset, data);
			break;
		case 0x30:          // Random number generator reset
			m_shift_reg = 0;
			break;
		case 0x38:          // Not used
			logerror("%04X: Write to unused DPC register $%02X, data $%02X\n", machine().device<cpu_device>("maincpu")->pc(), offset, data);
			break;
	}
}



// cart device

const device_type A26_ROM_DPC = &device_creator<a26_rom_dpc_device>;


a26_rom_dpc_device::a26_rom_dpc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
						: a26_rom_f8_device(mconfig, A26_ROM_DPC, "Atari 2600 ROM Cart Pitfall II", tag, owner, clock, "a2600_dcp", __FILE__),
						m_dpc(*this, "dpc")
{
}

//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------

void a26_rom_dpc_device::device_start()
{
	save_item(NAME(m_base_bank));
}

void a26_rom_dpc_device::device_reset()
{
	m_base_bank = 0;
}

void a26_rom_dpc_device::setup_addon_ptr(UINT8 *ptr)
{
	m_dpc->set_display_data(ptr);
}


static MACHINE_CONFIG_FRAGMENT( a26_dpc )
	MCFG_DEVICE_ADD("dpc", ATARI_DPC, 0)
MACHINE_CONFIG_END

machine_config_constructor a26_rom_dpc_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( a26_dpc );
}


READ8_MEMBER(a26_rom_dpc_device::read_rom)
{
	if (offset < 0x40)
		return m_dpc->read(space, offset);
	else
		return a26_rom_f8_device::read_rom(space, offset);
}

WRITE8_MEMBER(a26_rom_dpc_device::write_bank)
{
	if (offset >= 0x40 && offset < 0x80)
		m_dpc->write(space, offset, data);
	else
		a26_rom_f8_device::write_bank(space, offset, data);
}

DIRECT_UPDATE_MEMBER(a26_rom_dpc_device::cart_opbase)
{
	if (!direct.space().debugger_access())
	{
		UINT8 new_bit;
		new_bit = (m_dpc->m_shift_reg & 0x80) ^ ((m_dpc->m_shift_reg & 0x20) << 2);
		new_bit = new_bit ^ (((m_dpc->m_shift_reg & 0x10) << 3) ^ ((m_dpc->m_shift_reg & 0x08) << 4));
		new_bit = new_bit ^ 0x80;
		m_dpc->m_shift_reg = new_bit | (m_dpc->m_shift_reg >> 1);
	}
	return address;
}
