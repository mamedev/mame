// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***************************************************************************

 Atari 2600 cart with DPC chip (Pitfall II)

***************************************************************************/


#include "emu.h"
#include "dpc.h"


// DPC device

DEFINE_DEVICE_TYPE(ATARI_DPC, dpc_device, "atari_dpc", "Atari DPC")


dpc_device::dpc_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	device_t(mconfig, ATARI_DPC, tag, owner, clock),
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
	m_oscillator = timer_alloc(FUNC(dpc_device::oscillator_tick), this);
	m_oscillator->reset();

	save_item(STRUCT_MEMBER(m_df, top));
	save_item(STRUCT_MEMBER(m_df, bottom));
	save_item(STRUCT_MEMBER(m_df, low));
	save_item(STRUCT_MEMBER(m_df, high));
	save_item(STRUCT_MEMBER(m_df, flag));
	save_item(STRUCT_MEMBER(m_df, music_mode));
	save_item(STRUCT_MEMBER(m_df, osc_clk));

	save_item(NAME(m_movamt));
	save_item(NAME(m_latch_62));
	save_item(NAME(m_latch_64));
	save_item(NAME(m_dlc));
	save_item(NAME(m_shift_reg));
}

void dpc_device::device_reset()
{
	for (df_t & elem : m_df)
	{
		elem.osc_clk = 0;
		elem.flag = 0;
		elem.music_mode = 0;
	}
	m_oscillator->adjust(attotime::from_hz(18400), 0, attotime::from_hz(18400));

}

void dpc_device::check_flag(uint8_t data_fetcher)
{
	/* Set flag when low counter equals top */
	if (m_df[data_fetcher].low == m_df[data_fetcher].top)
		m_df[data_fetcher].flag = 1;

	/* Reset flag when low counter equals bottom */
	if (m_df[data_fetcher].low == m_df[data_fetcher].bottom)
		m_df[data_fetcher].flag = 0;
}

void dpc_device::decrement_counter(uint8_t data_fetcher)
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
//  oscillator_tick
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(dpc_device::oscillator_tick)
{
	for (int data_fetcher = 5; data_fetcher < 8; data_fetcher++)
	{
		if (m_df[data_fetcher].osc_clk)
		{
			decrement_counter(data_fetcher);
		}
	}
}


//-------------------------------------------------
//  Read / Write accesses
//-------------------------------------------------

uint8_t dpc_device::read(offs_t offset)
{
	static const uint8_t dpc_amplitude[8] = { 0x00, 0x04, 0x05, 0x09, 0x06, 0x0a, 0x0b, 0x0f };
	uint8_t   data_fetcher = offset & 0x07;
	uint8_t   data = 0xff;

	//logerror("%s: Read from DPC offset $%02X\n", machine().describe_context(), offset);
	if (offset < 0x08)
	{
		switch(offset & 0x06)
		{
			case 0x00:      // Random number generator
			case 0x02:
				m_shift_reg = (m_shift_reg << 1) | (~(((m_shift_reg >> 7) ^ (m_shift_reg >> 5)) ^ ((m_shift_reg >> 4) ^ (m_shift_reg >> 3))) & 1);
				return m_shift_reg;
			case 0x04:      // Sound value, MOVAMT value AND'd with Draw Line Carry; with Draw Line Add
				m_latch_62 = m_latch_64;
				[[fallthrough]];
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
		uint8_t display_data = m_displaydata[(~((m_df[data_fetcher].low | (m_df[data_fetcher].high << 8))) & 0x7ff)];

		switch (offset & 0x38)
		{
			case 0x08:          // display data
				data = display_data;
				break;
			case 0x10:          // display data AND'd w/flag
				data = m_df[data_fetcher].flag ? display_data : 0x00;
				break;
			case 0x18:          // display data AND'd w/flag, nibbles swapped
				data = m_df[data_fetcher].flag ? bitswap<8>(display_data,3,2,1,0,7,6,5,4) : 0x00;
				break;
			case 0x20:          // display data AND'd w/flag, byte reversed
				data = m_df[data_fetcher].flag ? bitswap<8>(display_data,0,1,2,3,4,5,6,7) : 0x00;
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

void dpc_device::write(offs_t offset, uint8_t data)
{
	uint8_t data_fetcher = offset & 0x07;

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
			logerror("%s: Write to unused DPC register $%02X, data $%02X\n", machine().describe_context(), offset, data);
			break;
		case 0x30:          // Random number generator reset
			m_shift_reg = 0;
			break;
		case 0x38:          // Not used
			logerror("%s: Write to unused DPC register $%02X, data $%02X\n", machine().describe_context(), offset, data);
			break;
	}
}



// cart device

DEFINE_DEVICE_TYPE(A26_ROM_DPC, a26_rom_dpc_device, "a2600_dpc", "Atari 2600 ROM Cart Pitfall II")


a26_rom_dpc_device::a26_rom_dpc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a26_rom_f8_device(mconfig, A26_ROM_DPC, tag, owner, clock), m_dpc(*this, "dpc")
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

void a26_rom_dpc_device::setup_addon_ptr(uint8_t *ptr)
{
	m_dpc->set_display_data(ptr);
}


void a26_rom_dpc_device::device_add_mconfig(machine_config &config)
{
	ATARI_DPC(config, m_dpc, 0);
}

uint8_t a26_rom_dpc_device::read_rom(offs_t offset)
{
	if (offset < 0x40)
		return m_dpc->read(offset);
	else
		return a26_rom_f8_device::read_rom(offset);
}

void a26_rom_dpc_device::write_bank(address_space &space, offs_t offset, uint8_t data)
{
	if (offset >= 0x40 && offset < 0x80)
		m_dpc->write(offset, data);
	else
		a26_rom_f8_device::write_bank(space, offset, data);
}
