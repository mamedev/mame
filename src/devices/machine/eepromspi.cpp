// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    SPI EEPROM devices with byte-oriented commands and addresses, page writes,
    upper-quarter/half/all block protection and nonvolatile status bits.
    Serial input is sampled on rising edges and output changes on falling edges.

    The shared protocol accepts one to three address bytes independently of
    array capacity. Devices that encode address bits in the command need a
    separate decoder. Deselected and held outputs read as pulled high.
    The 25320 model has 4 KiB, two address bytes and 32-byte pages. Its default
    write time is the 25LC320A's 5 ms maximum; board configuration can override it.

***************************************************************************/

#include "emu.h"
#include "eepromspi.h"

#include <algorithm>
#include <bit>


DEFINE_DEVICE_TYPE(EEPROM_25320, eeprom_spi_25320_device, "eeprom_25320", "25320 SPI EEPROM")


eeprom_spi_base_device::eeprom_spi_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 page_bytes, u8 address_bytes) :
	eeprom_base_device(mconfig, type, tag, owner),
	m_page_bytes(page_bytes),
	m_address_bytes(address_bytes),
	m_write_timer(nullptr),
	m_cs(true),
	m_sck(false),
	m_si(false),
	m_wp(true),
	m_hold(true),
	m_status(0)
{
}


eeprom_spi_25320_device::eeprom_spi_25320_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	eeprom_spi_base_device(mconfig, EEPROM_25320, tag, owner, 32, 2)
{
	size(0x1000, 8);
	write_time(attotime::from_msec(5));
}


bool eeprom_spi_base_device::valid_geometry() const
{
	// Page boundaries must also align with the block-protection boundaries.
	return (m_data_bits == 8) && std::has_single_bit(m_cells)
			&& std::has_single_bit(m_page_bytes) && (m_page_bytes <= m_cells / 4)
			&& (m_address_bytes >= 1) && (m_address_bytes <= 3)
			&& (m_cells <= (u32(1) << (8 * m_address_bytes)));
}


void eeprom_spi_base_device::device_validity_check(validity_checker &valid) const
{
	eeprom_base_device::device_validity_check(valid);
	if (!valid_geometry())
		osd_printf_error("%s: Invalid SPI EEPROM geometry\n", tag());
}


void eeprom_spi_base_device::device_start()
{
	if (!valid_geometry())
		throw emu_fatalerror("%s: Invalid SPI EEPROM geometry", tag());

	eeprom_base_device::device_start();
	m_page = std::make_unique<u8[]>(m_page_bytes);
	m_write_timer = timer_alloc(FUNC(eeprom_spi_base_device::write_complete), this);

	save_item(NAME(m_cs));
	save_item(NAME(m_sck));
	save_item(NAME(m_si));
	save_item(NAME(m_so));
	save_item(NAME(m_wp));
	save_item(NAME(m_hold));
	save_item(NAME(m_held));
	save_item(NAME(m_write_enable));
	save_item(NAME(m_state));
	save_item(NAME(m_command));
	save_item(NAME(m_shift));
	save_item(NAME(m_bits));
	save_item(NAME(m_address_count));
	save_item(NAME(m_address));
	save_item(NAME(m_status));
	save_item(NAME(m_pending_status));
	save_item(NAME(m_pending_command));
	save_item(NAME(m_page_base));
	save_item(NAME(m_page_start));
	save_item(NAME(m_page_count));
	save_pointer(NAME(m_page), m_page_bytes);
}


void eeprom_spi_base_device::device_reset()
{
	eeprom_base_device::device_reset();
	m_write_timer->adjust(attotime::never);
	m_so = true;
	m_held = !m_hold;
	m_write_enable = false;
	m_state = STATE_COMMAND;
	m_command = 0;
	m_shift = 0;
	m_bits = 0;
	m_address_count = 0;
	m_address = 0;
	m_pending_status = 0;
	m_pending_command = 0;
	m_page_base = 0;
	m_page_start = 0;
	m_page_count = 0;
	std::fill_n(m_page.get(), m_page_bytes, 0);
}


void eeprom_spi_base_device::nvram_default()
{
	eeprom_base_device::nvram_default();
	m_status = 0;
}


bool eeprom_spi_base_device::nvram_read(util::read_stream &file)
{
	if (!eeprom_base_device::nvram_read(file))
		return false;

	// The final NVRAM byte holds protection bits, not an extra array location.
	auto const [err, actual] = util::read(file, &m_status, 1);
	m_status &= STATUS_NONVOLATILE;
	return !err && (actual == 1);
}


bool eeprom_spi_base_device::nvram_write(util::write_stream &file)
{
	if (!eeprom_base_device::nvram_write(file))
		return false;

	auto const [err, actual] = util::write(file, &m_status, 1);
	return !err && (actual == 1);
}


void eeprom_spi_base_device::cs_w(int state)
{
	bool const cs = bool(state);
	if (m_cs == cs)
		return;

	m_cs = cs;
	if (cs)
	{
		finish_command();
		m_so = true;
	}
	else
	{
		m_state = STATE_COMMAND;
		m_command = 0;
		m_shift = 0;
		m_bits = 0;
		m_held = !m_hold;
	}
}


void eeprom_spi_base_device::sck_w(int state)
{
	bool const sck = bool(state);
	if (m_sck == sck)
		return;

	m_sck = sck;
	if (m_cs)
		return;

	if (!sck)
		m_held = !m_hold;
	if (m_held)
		return;

	if (sck)
	{
		if ((m_state == STATE_READ) || (m_state == STATE_READ_STATUS) || (m_state == STATE_IGNORE))
			return;

		if (m_state == STATE_DONE)
		{
			m_state = STATE_IGNORE;
			return;
		}

		m_shift = (m_shift << 1) | u8(m_si);
		if (++m_bits == 8)
		{
			m_bits = 0;
			receive_byte(m_shift);
		}
	}
	else if ((m_state == STATE_READ) || (m_state == STATE_READ_STATUS))
	{
		if (!m_bits)
			m_shift = (m_state == STATE_READ) ? internal_read(m_address) : status_r();
		m_so = BIT(m_shift, 7);
		m_shift <<= 1;
		if (++m_bits == 8)
		{
			m_bits = 0;
			if (m_state == STATE_READ)
				m_address = (m_address + 1) & (m_cells - 1);
		}
	}
}


void eeprom_spi_base_device::hold_w(int state)
{
	m_hold = bool(state);
	if (!m_sck)
		m_held = !m_hold;
}


void eeprom_spi_base_device::receive_byte(u8 data)
{
	switch (m_state)
	{
	case STATE_COMMAND:
		m_command = data;
		if ((m_pending_command || !ready()) && (data != COMMAND_RDSR))
		{
			m_state = STATE_IGNORE;
			break;
		}
		switch (data)
		{
		case COMMAND_READ:
		case COMMAND_WRITE:
			m_address = 0;
			m_address_count = 0;
			m_state = STATE_ADDRESS;
			break;
		case COMMAND_RDSR:
			m_state = STATE_READ_STATUS;
			break;
		case COMMAND_WRSR:
			m_state = STATE_WRITE_STATUS;
			break;
		case COMMAND_WREN:
		case COMMAND_WRDI:
			m_state = STATE_DONE;
			break;
		default:
			m_state = STATE_IGNORE;
			break;
		}
		break;

	case STATE_ADDRESS:
		m_address = (m_address << 8) | data;
		if (++m_address_count != m_address_bytes)
			break;
		m_address &= m_cells - 1;
		if (m_command == COMMAND_READ)
		{
			m_state = STATE_READ;
		}
		else
		{
			m_page_base = m_address & ~(m_page_bytes - 1);
			m_page_start = m_address & (m_page_bytes - 1);
			m_page_count = 0;
			m_state = STATE_WRITE;
		}
		break;

	case STATE_WRITE:
		m_page[m_address & (m_page_bytes - 1)] = data;
		m_page_count = std::min(m_page_count + 1, m_page_bytes);
		m_address = m_page_base | ((m_address + 1) & (m_page_bytes - 1));
		break;

	case STATE_WRITE_STATUS:
		m_pending_status = data & STATUS_NONVOLATILE;
		m_state = STATE_DONE;
		break;
	}
}


void eeprom_spi_base_device::finish_command()
{
	if (m_pending_command || !ready() || m_bits)
		return;

	switch (m_command)
	{
	case COMMAND_WREN:
		if (m_state == STATE_DONE)
			m_write_enable = true;
		break;
	case COMMAND_WRDI:
		if (m_state == STATE_DONE)
			m_write_enable = false;
		break;
	case COMMAND_WRITE:
		if ((m_state == STATE_WRITE) && m_page_count && m_write_enable && !write_protected())
			start_write(COMMAND_WRITE);
		break;
	case COMMAND_WRSR:
		if ((m_state == STATE_DONE) && m_write_enable && (m_wp || !BIT(m_status, 7)))
			start_write(COMMAND_WRSR);
		break;
	}
}


void eeprom_spi_base_device::start_write(u8 command)
{
	m_pending_command = command;
	m_completion_time = machine().time() + m_operation_time[WRITE_TIME];
	m_write_timer->adjust(m_operation_time[WRITE_TIME]);
}


u8 eeprom_spi_base_device::status_r() const
{
	return m_status | (u8(m_write_enable) << 1) | u8(m_pending_command || !ready());
}


bool eeprom_spi_base_device::write_protected() const
{
	switch (BIT(m_status, 2, 2))
	{
	case 1: return m_page_base >= (m_cells - m_cells / 4);
	case 2: return m_page_base >= (m_cells / 2);
	case 3: return true;
	default: return false;
	}
}


TIMER_CALLBACK_MEMBER(eeprom_spi_base_device::write_complete)
{
	if (m_pending_command == COMMAND_WRITE)
	{
		for (u32 byte = 0; byte < m_page_count; ++byte)
		{
			u32 const offset = (m_page_start + byte) & (m_page_bytes - 1);
			internal_write(m_page_base + offset, m_page[offset]);
		}
	}
	else if (m_pending_command == COMMAND_WRSR)
	{
		m_status = m_pending_status;
	}

	m_pending_command = 0;
	m_write_enable = false;
}
