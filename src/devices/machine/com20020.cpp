// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    SMSC COM20020 Universal Local Area Network Controller (ARCNET)

    Only the host interface is emulated: the node acts as if no network
    cable is attached.

***************************************************************************/

#include "emu.h"
#include "com20020.h"

#define VERBOSE (0)
#include "logmacro.h"

namespace {

constexpr u8 STATUS_RI      = 0x80;
constexpr u8 STATUS_POR     = 0x10;
constexpr u8 STATUS_RECON   = 0x04;
constexpr u8 STATUS_TMA     = 0x02;
constexpr u8 STATUS_TA      = 0x01;

constexpr u8 CONFIG_RESET   = 0x80;

constexpr u8 PTR_AUTOINC    = 0x40;

} // anonymous namespace

DEFINE_DEVICE_TYPE(COM20020, com20020_device, "com20020", "SMSC COM20020 ULANC")

com20020_device::com20020_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, COM20020, tag, owner, clock)
	, m_int_cb(*this)
	, m_ram(*this, "ram", RAM_SIZE, ENDIANNESS_LITTLE)
	, m_status(0)
	, m_imr(0)
	, m_config(0)
	, m_subadr(0)
	, m_tentid(0)
	, m_nodeid(0)
	, m_setup1(0)
	, m_setup2(0)
	, m_ptr_high(0)
	, m_address(0)
	, m_int(false)
{
}

void com20020_device::device_start()
{
	save_item(NAME(m_status));
	save_item(NAME(m_imr));
	save_item(NAME(m_config));
	save_item(NAME(m_subadr));
	save_item(NAME(m_tentid));
	save_item(NAME(m_nodeid));
	save_item(NAME(m_setup1));
	save_item(NAME(m_setup2));
	save_item(NAME(m_ptr_high));
	save_item(NAME(m_address));
	save_item(NAME(m_int));
}

void com20020_device::device_reset()
{
	m_imr = 0;
	m_config = 0x18;
	m_subadr = 0;
	m_tentid = 0;
	m_nodeid = 0;
	m_setup1 = 0;
	m_setup2 = 0;

	software_reset();
}

void com20020_device::software_reset()
{
	m_status = STATUS_RI | STATUS_POR | STATUS_TA;
	update_int();
}

// the core writes a signature and its node ID to the start of RAM when it wakes up
void com20020_device::wake_core()
{
	m_ram[0] = 0xd1;
	m_ram[1] = m_nodeid;
}

void com20020_device::update_int()
{
	const bool state = (m_imr & m_status & (STATUS_RI | STATUS_RECON | STATUS_TA)) != 0;
	if (state != m_int)
	{
		m_int = state;
		m_int_cb(state ? 1 : 0);
	}
}

void com20020_device::command_w(u8 data)
{
	LOG("%s: command %02X\n", machine().describe_context(), data);

	if ((data & 0x47) == 0x04)
	{
		// enable receive to page
		m_status &= ~STATUS_RI;
	}
	else if ((data & 0xc7) == 0x03)
	{
		// enable transmit from page
		m_status &= ~(STATUS_TA | STATUS_TMA);
	}
	else if ((data & 0xe7) == 0x06)
	{
		// clear flags
		if (BIT(data, 3))
		{
			m_status &= ~STATUS_POR;
		}
		if (BIT(data, 4))
		{
			m_status &= ~STATUS_RECON;
		}
	}

	// disabling the transmitter or receiver only takes effect when the token arrives
	update_int();
}

u8 com20020_device::read(offs_t offset)
{
	switch (offset & 7)
	{
		case 0:
			return m_status;

		case 1:
			return 0;

		case 2:
			return (m_ptr_high & 0xc0) | (m_address >> 8);

		case 3:
			return m_address & 0xff;

		case 4:
		{
			const u8 data = m_ram[m_address];
			if (!machine().side_effects_disabled() && (m_ptr_high & PTR_AUTOINC))
			{
				m_address = (m_address + 1) & (RAM_SIZE - 1);
			}
			return data;
		}

		case 5:
			return m_subadr;

		case 6:
			return m_config;

		default:
			switch (m_subadr)
			{
				case 0:
					return m_tentid;

				case 1:
					return m_nodeid;

				case 2:
					return m_setup1;

				case 3:
					return 0; // next ID: no token is ever passed on

				case 4:
					return m_setup2;

				default:
					return 0xff;
			}
	}
}

void com20020_device::write(offs_t offset, u8 data)
{
	switch (offset & 7)
	{
		case 0:
			m_imr = data;
			update_int();
			break;

		case 1:
			command_w(data);
			break;

		case 2:
			m_ptr_high = data;
			break;

		case 3:
			m_address = ((m_ptr_high & 0x07) << 8) | data;
			break;

		case 4:
			m_ram[m_address] = data;
			if (m_ptr_high & PTR_AUTOINC)
			{
				m_address = (m_address + 1) & (RAM_SIZE - 1);
			}
			break;

		case 5:
			m_subadr = data & 0x07;
			m_config = (m_config & ~0x03) | (data & 0x03);
			break;

		case 6:
			if (data & CONFIG_RESET)
			{
				software_reset();
			}
			else if ((m_config & CONFIG_RESET) && m_nodeid)
			{
				wake_core();
			}
			m_config = data;
			m_subadr = data & 0x03;
			break;

		default:
			switch (m_subadr)
			{
				case 0:
					m_tentid = data;
					break;

				case 1:
					// datasheet table 4: writing 0 sets POR
					m_nodeid = data;
					if (!data)
					{
						m_status |= STATUS_POR;
					}
					else if (!(m_config & CONFIG_RESET))
					{
						wake_core();
					}
					break;

				case 2:
					m_setup1 = data;
					break;

				case 4:
					m_setup2 = data;
					break;

				default:
					break;
			}
			break;
	}
}
