// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"

#include "vt_menu_protection.h"

DEFINE_DEVICE_TYPE(VT_MENU_PROTECTION, vt_menu_protection_device, "vtmenuprot", "VT Menu Protection")

vt_menu_protection_device::vt_menu_protection_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VT_MENU_PROTECTION, tag, owner, clock),
	m_extrarom(*this, DEVICE_SELF),
	m_in_data(false),
	m_in_enable(false),
	m_in_clock(false)
{
}

uint8_t vt_menu_protection_device::read()
{
	return m_protlatch;
}

u8 vt_menu_protection_device::compute_36pcase_late_response(u8 data)
{
	switch (data)
	{
	case 0x02:
		return 0x01;
	case 0x4a:
		return 0x00;
	case 0x6d:
		return 0x1e;
	case 0xb2:
		return 0x02;
	default:
		return 0x00;
	}
}

void vt_menu_protection_device::write_enable(int state)
{
	if (bool(state) != m_in_enable)
	{
		if (!state)
		{
			m_protectionstate = 1; // ready to get command
			m_commandbits = 8;
			logerror("protection state ready\n");
		}
		else
		{
			m_protectionstate = 0; // device not selected?
			logerror("protection state deselect\n");
		}
	}

	m_in_enable = bool(state);
}

void vt_menu_protection_device::write_clock(int state)
{
	if (bool(state) != m_in_clock)
	{
		if (state)
		{
			if (m_protectionstate == 1)
			{
				m_command = (m_command << 1) | (m_in_data ? 1:0);
				m_commandbits--;

				logerror("a %02x\n", m_commandbits);

				if (m_commandbits == 0)
				{
					logerror("got command %02x\n", m_command);

					// gtct885 uses command 0x30 to read, goretrop uses command 0x20
					if ((m_command == 0x30) || (m_command == 0x20))
					{
						logerror("(read bytes)\n");
						m_protectionstate = 2;
						m_protreadposition = m_read_start_byte * 8;
					}
					else if (m_36pcase_late_protocol && (m_command == 0x10))
					{
						m_protectionstate = 3;
						m_command = 0;
						m_commandbits = 8;
					}
					else
					{
						logerror("(unknown command)\n");
						m_protectionstate = 0;
					}
				}
			}
			else if (m_protectionstate == 2)
			{
				u8 readbyte = m_extrarom->base()[m_protreadposition >> 3];
				u8 readbit = BIT(readbyte, ~m_protreadposition & 7);

				logerror("reading bit at byte %02x bit %1x (byte is %02x bit read is %1x)\n", m_protreadposition >> 3, m_protreadposition & 7, readbyte, readbit);

				m_protlatch = readbit;

				m_protreadposition++;
			}
			else if (m_protectionstate == 3)
			{
				m_command = (m_command << 1) | (m_in_data ? 1 : 0);
				m_commandbits--;

				if (m_commandbits == 0)
				{
					m_36pcase_late_response = compute_36pcase_late_response(m_command);
					m_36pcase_late_response_pos = 0;
					m_protectionstate = 4;
				}
			}
			else if (m_protectionstate == 4)
			{
				const u8 readbit = BIT(m_36pcase_late_response, ~m_36pcase_late_response_pos & 7);

				m_protlatch = readbit;

				if (++m_36pcase_late_response_pos == 8)
					m_protectionstate = 0;
			}
		}
	}

	m_in_clock = bool(state);
}

void vt_menu_protection_device::write_data(int state)
{
	m_in_data = bool(state);
}

void vt_menu_protection_device::device_start()
{
	save_item(NAME(m_command));
	save_item(NAME(m_commandbits));
	save_item(NAME(m_protectionstate));
	save_item(NAME(m_protlatch));
	save_item(NAME(m_read_start_byte));
	save_item(NAME(m_protreadposition));
	save_item(NAME(m_36pcase_late_response));
	save_item(NAME(m_36pcase_late_response_pos));
	save_item(NAME(m_in_data));
	save_item(NAME(m_in_enable));
	save_item(NAME(m_in_clock));
}

void vt_menu_protection_device::device_reset()
{
	m_command = 0;
	m_commandbits = 0;
	m_protectionstate = 0;
	m_protlatch = 0;
	m_protreadposition = 0;
	m_36pcase_late_response = 0;
	m_36pcase_late_response_pos = 0;
}
