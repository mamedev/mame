// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    Aladdin/Fast Hardlock parallel-port dongle (Luna ASIC)
    Emulation by R. Belmont

    This parallel port dongle has 4 16-bit keys and a small EEPROM.

***************************************************************************/

#include "emu.h"
#include "hardlock.h"

DEFINE_DEVICE_TYPE(HARDLOCK, hardlock_device, "hardlock", "Aladdin Hardlock (Luna ASIC)")

hardlock_device::hardlock_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, HARDLOCK, tag, owner, clock)
	, device_centronics_peripheral_interface(mconfig, *this)
	, m_eeprom(*this, "eeprom")
	, m_module(0)
	, m_seed1(0)
	, m_seed2(0)
	, m_seed3(0)
	, m_data(0xff)
	, m_previous(0xff)
	, m_pending(false)
	, m_token(0)
	, m_sync_clocks(-1)
	, m_address_count(0)
	, m_address{}
	, m_selected(false)
	, m_eeprom_mode(false)
	, m_eeprom_do(true)
	, m_index(15)
	, m_history(0)
	, m_shift(0)
{
	m_default_eeprom.fill(0xffff);
}

void hardlock_device::set_keys(u16 module, u16 seed1, u16 seed2, u16 seed3)
{
	m_module = module;
	m_seed1 = seed1;
	m_seed2 = seed2;
	m_seed3 = seed3;
}

void hardlock_device::device_add_mconfig(machine_config &config)
{
	EEPROM_93C46_16BIT(config, m_eeprom);
	m_eeprom->default_data(m_default_eeprom.data(), sizeof(m_default_eeprom));
	m_eeprom->do_callback().set(FUNC(hardlock_device::eeprom_do_w));
}

void hardlock_device::device_start()
{
	save_item(NAME(m_data));
	save_item(NAME(m_previous));
	save_item(NAME(m_pending));
	save_item(NAME(m_token));
	save_item(NAME(m_sync_clocks));
	save_item(NAME(m_address_count));
	save_item(NAME(m_address));
	save_item(NAME(m_selected));
	save_item(NAME(m_eeprom_mode));
	save_item(NAME(m_eeprom_do));
	save_item(NAME(m_index));
	save_item(NAME(m_history));
	save_item(NAME(m_shift));
}

void hardlock_device::device_reset()
{
	m_pending = false;
	++m_token;
	m_previous = m_data;
	m_sync_clocks = -1;
	m_address_count = 0;
	m_selected = false;
	m_eeprom_mode = false;
	m_eeprom->cs_write(0);
	reset_cipher();
	output_fault(1);
	output_select(1);
	output_perror(0);
	update_output();
}

void hardlock_device::input_data0(int state) { data_w(0, state); }
void hardlock_device::input_data1(int state) { data_w(1, state); }
void hardlock_device::input_data2(int state) { data_w(2, state); }
void hardlock_device::input_data3(int state) { data_w(3, state); }
void hardlock_device::input_data4(int state) { data_w(4, state); }
void hardlock_device::input_data5(int state) { data_w(5, state); }
void hardlock_device::input_data6(int state) { data_w(6, state); }
void hardlock_device::input_data7(int state) { data_w(7, state); }

void hardlock_device::data_w(u8 bit, int state)
{
	m_data = (m_data & ~(1U << bit)) | (state ? (1U << bit) : 0);

	// A data latch write changes several pins. Observe the settled byte once.
	if (!m_pending)
	{
		m_pending = true;
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(hardlock_device::update_data), this), int(++m_token));
	}
}

TIMER_CALLBACK_MEMBER(hardlock_device::update_data)
{
	if (!m_pending || param != int(m_token))
	{
		return;
	}
	m_pending = false;
	process_data(m_data);
}

bool hardlock_device::select_module(u8 data)
{
	if (data == 0x7e)
	{
		m_sync_clocks = 0;
		m_address_count = 0;
		m_selected = false;
		m_eeprom_mode = false;
		return true;
	}
	if ((m_sync_clocks >= 0) && (m_sync_clocks < 32))
	{
		if (data == ((m_sync_clocks & 1) ? 0xff : 0x7f))
		{
			++m_sync_clocks;
		}
		else
		{
			m_sync_clocks = -1;
		}
		return true;
	}
	if (m_sync_clocks != 32)
	{
		return false;
	}
	m_address[m_address_count++] = data;
	if (m_address_count == m_address.size())
	{
		const u16 module = BIT(m_address[0], 1, 5) | (BIT(m_address[2], 1, 5) << 5) | (BIT(m_address[5], 1, 5) << 10);
		m_selected = (module == m_module) && (m_address[7] == 0xc1) && (m_address[8] == 0x41);
		m_sync_clocks = -1;
		m_address_count = 0;
		reset_cipher();
	}
	return true;
}

void hardlock_device::reset_cipher()
{
	m_index = 15;
	m_history = 0;
	m_shift = 0;
}

void hardlock_device::clock_cipher(u8 data)
{
	const u8 value = (BIT(m_seed1, m_index) << 1) | BIT(m_seed2, m_index);
	m_shift = (m_shift << 4) | value;
	m_index ^= BIT(data, 2, 4) ^ m_history ^ BIT(m_seed3, 12 - 4 * value, 4);
	m_history = ((m_history << 1) | BIT(value, 1)) & 15;
}

void hardlock_device::process_data(u8 data)
{
	const u8 previous = m_previous;
	if (data == previous)
	{
		return;
	}
	m_previous = data;

	if (select_module(data))
	{
		m_eeprom->cs_write(0);
		update_output();
		return;
	}

	if (!BIT(data, 6))
	{
		reset_cipher();
		m_eeprom_mode = false;
	}
	else if (m_selected && BIT(data, 0) && BIT(data, 1))
	{
		m_eeprom_mode = true;
	}

	m_eeprom->di_write(BIT(data, 4));
	m_eeprom->cs_write(m_selected && m_eeprom_mode && BIT(data, 6));
	m_eeprom->clk_write(BIT(data, 7));
	if (m_selected && !m_eeprom_mode && BIT(data, 6) && BIT(data, 0) && !BIT(previous, 7) && BIT(data, 7))
	{
		clock_cipher(data);
	}
	update_output();
}

void hardlock_device::eeprom_do_w(int state)
{
	m_eeprom_do = bool(state);
	update_output();
}

void hardlock_device::update_output()
{
	const int bit = m_selected ? (m_eeprom_mode ? m_eeprom_do : BIT(m_shift, 12)) : 0;
	output_ack(!bit);
	output_busy(bit);
}
