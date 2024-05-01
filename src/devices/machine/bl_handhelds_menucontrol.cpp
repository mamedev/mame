// license:BSD-3-Clause
// copyright-holders:David Haywood

// Menu controller, stores / increments menu position etc. used on handhelds that typically have BL on the boot screen (BaoBaoLong?)

#include "emu.h"
#include "bl_handhelds_menucontrol.h"

DEFINE_DEVICE_TYPE(BL_HANDHELDS_MENUCONTROL, bl_handhelds_menucontrol_device, "blhandheldmenu", "BaoBaoLong Handhelds Menu Controller")

bl_handhelds_menucontrol_device::bl_handhelds_menucontrol_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, BL_HANDHELDS_MENUCONTROL, tag, owner, clock),
	m_is_unsp_type_hack(false)
{
}

int bl_handhelds_menucontrol_device::status_r()
{
	return m_clockstate;
}

int bl_handhelds_menucontrol_device::data_r()
{
	return m_responsebit;
}

// is there some 'carry' related behavior on the add / remove, because sometimes calculations are off by 1 eg command 0x05 (subtract) subcommand 0xf0
// edge cases to test - moving up and left off entry 0, moving next / previous page when on any entry on the first/last pages, moving down/right off last entry, crossing 255/256 game boundary
void bl_handhelds_menucontrol_device::handle_command()
{
	uint8_t command = m_command;

	// additions and subtractions here are likely also meant to be done as high byte and low byte
	if (m_menustate == MENU_READY_FOR_COMMAND)
	{
		if (command == 0x00)
		{
			m_menustate = MENU_COMMAND_00_IN;
			m_response = m_menupos & 0xff;
		}
		else if (command == 0x01)
		{
			m_menustate = MENU_COMMAND_01_IN;
			m_response = (m_menupos >> 8) & 0xff;
		}
		else if (command == 0x02)
		{
			m_menustate = MENU_COMMAND_02_IN;
		}
		else if (command == 0x03)
		{
			m_menustate = MENU_COMMAND_03_IN;
		}
		else if (command == 0x04)
		{
			m_menustate = MENU_COMMAND_04_IN;
		}
		else if (command == 0x05)
		{
			m_menustate = MENU_COMMAND_05_IN;
		}
		else if (command == 0x09)
		{
			// ...
		}
		else if ((command) == 0x10)
		{
			// this is followed by 0x1b, written if you try to move right off last entry
			m_menupos = 0x00;
		}
		else if (command == 0x30)
		{
			m_menupos++;
		}
		else if (command == 0x37)
		{
			m_menupos--;
		}
		else if (command == 0x39)
		{
			m_menupos -= 4;
		}
		else if (command == 0x2c)
		{
			m_menupos = 0x01;
		}
		else
		{
			logerror("handle_command %02x (unknown)\n", command);
		}
	}
	else if (m_menustate == MENU_COMMAND_00_IN)
	{
		m_menustate = MENU_READY_FOR_COMMAND;
	}
	else if (m_menustate == MENU_COMMAND_01_IN)
	{
		m_menustate = MENU_READY_FOR_COMMAND;
	}
	else if (m_menustate == MENU_COMMAND_02_IN)
	{
		m_menupos = (m_menupos & 0xff00) | ((command - 0x8) & 0xff);
		m_menustate = MENU_READY_FOR_COMMAND;
	}
	else if (m_menustate == MENU_COMMAND_03_IN)
	{
		m_menupos = (m_menupos & 0x00ff) | (((command - 0x9) & 0xff) << 8);
		m_menustate = MENU_READY_FOR_COMMAND;
	}
	else if (m_menustate == MENU_COMMAND_04_IN)
	{
		// used if you try to scroll up or left past 0 and the value becomes too large (a negative number)
		if (m_is_unsp_type_hack)
		{
			if (command == 0x0d)
				m_menupos += 4;
			else if (command == 0x0a)
				m_menupos += 0;
			// used if you try to scroll up or left past 0 and the value becomes too large (a negative number)
			// actually writes 0x314 split into 2 commands, so the 2nd write to 0x04 with param then instead 0b/16 sequence of writes instead of 26/0c adds to the high byte?
			else if (command == 0x1e)
				m_menupos += 0x310;
		}
		else
		{
			m_menupos += (command - 0x09);
			m_menustate = MENU_READY_FOR_COMMAND;
		}

		m_menustate = MENU_READY_FOR_COMMAND;
	}
	else if (m_menustate == MENU_COMMAND_05_IN)
	{
		// used if you try to scroll down past the and the value becomes too large
		if (m_is_unsp_type_hack)
		{
			// actually writes 0x313 split into 2 commands, so the 2nd write to 0x05 with param then instead 0b/16 sequence of writes instead of 26/0c subtracts from the high byte?
			if (command == 0x0b)
			{
				m_menupos -= 0xdc;
			}
			else if (command == 0x0e)
			{
				m_menupos -= 0x314;
			}
		}
		else
		{
			if (command != 0xf0)
				m_menupos -= (command - 0x0b);
			else
				m_menupos -= (command - 0x0a); // dphh8630 when pressing 'right' on final menu page
		}

		m_menustate = MENU_READY_FOR_COMMAND;
	}
}

void bl_handhelds_menucontrol_device::clock_w(int state)
{
	if (state)
	{
		m_clockstate = 1;
	}
	else
	{
		m_clockstate = 0;
		m_command <<= 1;
		m_command |= ((m_commandbit));
		m_responsebit = (m_response >> (7 - m_datashifterpos)) & 1;
		m_datashifterpos++;

		if (m_datashifterpos == 8)
		{
			m_datashifterpos = 0;
			handle_command();
		}
	}
}

void bl_handhelds_menucontrol_device::data_w(int state)
{
	m_commandbit = state;
}

void bl_handhelds_menucontrol_device::reset_w(int state)
{
	m_datashifterpos = 0;
}

void bl_handhelds_menucontrol_device::device_start()
{
	m_responsebit = 0;
	m_clockstate = 0;
	m_command = 0;
	m_datashifterpos = 0;
	m_menupos = 0;
	m_response = 0;
	m_commandbit = 0;
	m_menustate = MENU_READY_FOR_COMMAND;

	save_item(NAME(m_menupos));
	save_item(NAME(m_clockstate));
	save_item(NAME(m_datashifterpos));
	save_item(NAME(m_responsebit));
	save_item(NAME(m_response));
	save_item(NAME(m_commandbit));
	save_item(NAME(m_command));
	save_item(NAME(m_menustate));
}

void bl_handhelds_menucontrol_device::device_reset()
{
}
