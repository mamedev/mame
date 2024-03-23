// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "msx_matsushita.h"


DEFINE_DEVICE_TYPE(MSX_MATSUSHITA, msx_matsushita_device, "msx_matsushita", "Matsushita switched device")

msx_matsushita_device::msx_matsushita_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MSX_MATSUSHITA, tag, owner, clock)
	, device_nvram_interface(mconfig, *this)
	, m_io_config(*this, "CONFIG")
	, m_turbo_out_cb(*this)
	, m_selected(false)
	, m_address(0)
	, m_nibble1(0)
	, m_nibble2(0)
	, m_pattern(0)
{
}

static INPUT_PORTS_START(matsushita)
	PORT_START("CONFIG")
	PORT_CONFNAME(0x80, 0x00, "Firmware switch")
	PORT_CONFSETTING(0x00, "On")
	PORT_CONFSETTING(0x80, "Off")
	PORT_BIT(0x7F, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

ioport_constructor msx_matsushita_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(matsushita);
}

void msx_matsushita_device::device_start()
{
	m_sram.resize(0x800);

	save_item(NAME(m_selected));
	save_item(NAME(m_address));
	save_item(NAME(m_sram));
	save_item(NAME(m_nibble1));
	save_item(NAME(m_nibble2));
	save_item(NAME(m_pattern));
}

void msx_matsushita_device::nvram_default()
{
	std::fill_n(m_sram.data(), m_sram.size(), 0);
}

bool msx_matsushita_device::nvram_read(util::read_stream &file)
{
	auto const [err, actual] = read(file, &m_sram[0], m_sram.size());
	return !err && (actual == m_sram.size());
}

bool msx_matsushita_device::nvram_write(util::write_stream &file)
{
	auto const [err, actual] = write(file, &m_sram[0], m_sram.size());
	return !err;
}

u8 msx_matsushita_device::switched_read(offs_t offset)
{
	if (m_selected)
	{
		switch (offset)
		{
		case 0x00:
			return MANUFACTURER_ID ^ 0xff;

		case 0x01:
			return m_io_config->read();

		case 0x03:
		{
			u8 result = ((BIT(m_pattern, 7) ? m_nibble1 : m_nibble2) << 4) | (BIT(m_pattern, 6) ? m_nibble1 : m_nibble2);

			if (!machine().side_effects_disabled())
				m_pattern = (m_pattern << 2) | (m_pattern >> 6);

			return result;
		}

		case 0x09:   // Data
			if (m_address < m_sram.size())
			{
				return m_sram[m_address];
			}
			break;

		default:
			logerror("msx_matsushita: unhandled read from offset %02x\n", offset);
			break;
		}
	}

	return 0xff;
}


void msx_matsushita_device::switched_write(offs_t offset, u8 data)
{
	if (offset == 0)
	{
		m_selected = (data == MANUFACTURER_ID);
	}
	else if (m_selected)
	{
		switch (offset)
		{
		case 0x01:
			// bit 0: CPU clock select
			//        0 - 5.369317 MHz
			//        1 - 3.579545 MHz
			m_turbo_out_cb((data & 1) ? ASSERT_LINE : CLEAR_LINE);
			break;

		case 0x03:
			m_nibble1 = data & 0x0f;
			m_nibble2 = data >> 4;
			break;

		case 0x04:
			m_pattern = data;
			break;

		case 0x07:   // Address low
			m_address = (m_address & 0xff00) | data;
			break;

		case 0x08:   // Address high
			m_address = (m_address & 0xff) | (data << 8);
			break;

		case 0x09:   // Data
			if (m_address < m_sram.size())
			{
				m_sram[m_address] = data;
			}
			break;

		default:
			logerror("msx_matsushita: unhandled write %02x to offset %02x\n", data, offset);
			break;
		}
	}
}
