// license:BSD-3-Clause
// copyright-holders:David Haywood

// some kind of 2-wire protocol (but not SPI?)
// might be doing a calculation rather than returning data from a table

#include "emu.h"

#include "vt_menu_protection_lxcap.h"

DEFINE_DEVICE_TYPE(VT_MENU_PROTECTION_LXCAP, vt_menu_protection_lxcap_device, "vtmenuprot_lxcap", "VT Menu Protection (lxcap)")

vt_menu_protection_lxcap_device::vt_menu_protection_lxcap_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VT_MENU_PROTECTION_LXCAP, tag, owner, clock)
{
}

uint8_t vt_menu_protection_lxcap_device::read()
{
	logerror("reading data bit\n");
	return m_outlatch;
}

void vt_menu_protection_lxcap_device::write_clock(bool state)
{
	if (state != m_clock)
	{
		if (state)
		{
			logerror("%s vt_menu_protection_lxcap_device::write_clock HIGH\n", machine().describe_context());
			logerror("reading/writing data bit %d\n", m_data ? 1 : 0);

			if (m_phase == 0)
			{
				m_command = (m_command << 1) | (m_data ? 1 : 0);
				m_bitcount++;

				if (m_bitcount == 20)
				{

					// c0328 - 1 10000000 0 11001010 00 (from 80 ca)
					//
					u8 type = (m_command >> 11) & 0xff;
					u8 param = (m_command >> 2) & 0xff;
					//m_retdat = m_extrarom->base()[param]; // use the old table
					m_retdat = (0x100 - param) & 0xff;
					m_retdat = bitswap<8>(m_retdat, 3, 2, 1, 0, 7, 6, 5, 4);

					logerror("got command %04x (%02x %02x) ready to return %02x\n", m_command, type, param, m_retdat);

					m_phase = 1;
					m_bitcount = 0;
				}
			}
			else if (m_phase == 1)
			{
				m_bitcount++;

				m_outlatch = (m_retdat & 0x80) ? 1 : 0;
				m_retdat <<= 1;

				if (m_bitcount == 9)
				{
					m_phase = 0;
					m_bitcount = 0;
					logerror("setting back to phase 0\n");
				}
			}

		}
		else
		{
			logerror("%s vt_menu_protection_lxcap_device::write_clock LOW\n", machine().describe_context());
		}
	}

	m_clock = state;
}

void vt_menu_protection_lxcap_device::write_data(bool state)
{
	if (state != m_data)
	{
		if (state)
			logerror("%s vt_menu_protection_lxcap_device::write_data HIGH\n", machine().describe_context());
		else
			logerror("%s vt_menu_protection_lxcap_device::write_data LOW\n", machine().describe_context());
	}

	m_data = state;
}

void vt_menu_protection_lxcap_device::device_start()
{
	save_item(NAME(m_data));
	save_item(NAME(m_clock));
	save_item(NAME(m_bitcount));
	save_item(NAME(m_command));
	save_item(NAME(m_phase));
	save_item(NAME(m_retdat));
	save_item(NAME(m_outlatch));
}

void vt_menu_protection_lxcap_device::device_reset()
{
	m_data = false;
	m_clock = false;
	m_bitcount = 0;
	m_command = 0;
	m_phase = 0;
	m_retdat = 0;
	m_outlatch = 0;
}
