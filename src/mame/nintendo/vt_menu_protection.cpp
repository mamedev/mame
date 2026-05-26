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
						m_protreadposition = 0;
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
	save_item(NAME(m_protlatch));;
	save_item(NAME(m_protreadposition));
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
}
