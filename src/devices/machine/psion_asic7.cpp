// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/******************************************************************************

    Psion ASIC7

******************************************************************************/

#include "emu.h"
#include "psion_asic7.h"


#define VERBOSE 0
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


DEFINE_DEVICE_TYPE(PSION_ASIC7, psion_asic7_device, "psion_asic7", "Psion ASIC7")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

psion_asic7_device::psion_asic7_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PSION_ASIC7, tag, owner, clock)
	, m_as2rd_callback(*this, 0)
	, m_as2wr_callback(*this)
	, m_pgsel_callback(*this)
	, m_lcdcom_callback(*this)
	, m_caps_callback(*this)
	, m_numl_callback(*this)
	, m_scrl_callback(*this)
	, m_batt_callback(*this)
	, m_stby_callback(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void psion_asic7_device::device_start()
{
	save_item(NAME(m_a7_control));
	save_item(NAME(m_a7_key_command));
	save_item(NAME(m_a7_access_key));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void psion_asic7_device::device_reset()
{
	m_a7_control     = 0x08; // STBY
	m_a7_key_command = 0x00;
	m_a7_access_key  = 0x00;

	m_pgsel_callback(0);
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

uint8_t psion_asic7_device::io_r(offs_t offset, uint8_t mem_mask)
{
	uint8_t data = 0x00;

	if (m_a7_access_key == 0x5aa5)
	{
		switch (offset & 8)
		{
		case 0x00:
			data = m_as2rd_callback(offset);
			break;

		case 0x08:
			switch (offset)
			{
			case 0x08: // System Control
				//   b0    RamEnable
				//   b1    PageSel
				//   b2    MaximEnable
				//   b3    StandEnable
				//   b4    CapsLockLcd
				//   b5    NumLockLcd
				//   b6    ScrlLockLcd
				//   b7    BatLowLcd
				data = m_a7_control;
				LOG("%s io_r: A7Control => %02x\n", machine().describe_context(), data);
				break;

			case 0x09: // Keyboard Command Busy Flag
				//   b0    KeyBusy
				data = 0;
				LOG("%s io_r: A7KeyStatus => %02x\n", machine().describe_context(), data);
				break;
			}
		}
	}
	return data;
}

void psion_asic7_device::io_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	if (m_a7_access_key == 0x5aa5 || (offset & 0x0e) == 0x0a)
	{
		switch (offset & 0x08)
		{
		case 0x00:
			m_as2wr_callback(offset, data);
			break;

		case 0x08:
			switch (offset)
			{
			case 0x08: // System Control
				//   b0    RamEnable
				//   b1    PageSel
				//   b2    MaximEnable
				//   b3    StandEnable
				//   b4    CapsLockLcd
				//   b5    NumLockLcd
				//   b6    ScrlLockLcd
				//   b7    BatLowLcd
				LOG("%s io_w: A7Control <= %02x\n", machine().describe_context(), data);
				m_pgsel_callback(BIT(data, 1));
				m_lcdcom_callback(BIT(data, 2));
				if (BIT(data, 3) != BIT(m_a7_control, 3))
					m_stby_callback(BIT(data, 3));
				if (BIT(data, 4) != BIT(m_a7_control, 4))
					m_caps_callback(BIT(data, 4));
				if (BIT(data, 5) != BIT(m_a7_control, 5))
					m_numl_callback(BIT(data, 5));
				if (BIT(data, 6) != BIT(m_a7_control, 6))
					m_scrl_callback(BIT(data, 6));
				if (BIT(data, 7) != BIT(m_a7_control, 7))
					m_batt_callback(BIT(data, 7));
				m_a7_control = data;
				break;

			case 0x09: // Keyboard Command
				LOG("%s io_w: A7KeyCommand <= %02x\n", machine().describe_context(), data);
				m_a7_key_command = data;
				break;

			case 0x0a: // Access Key 0
			case 0x0b: // Access Key 1
				LOG("%s io_w: A7KeyAccess%d <= %02x\n", machine().describe_context(), (offset & 1) + 1, data);
				switch (offset & 1)
				{
				case 0:
					m_a7_access_key = (m_a7_access_key & 0xff00) | (data << 0);
					break;
				case 1:
					m_a7_access_key = (m_a7_access_key & 0x00ff) | (data << 8);
					break;
				}
				break;
			}
		}
	}
}

void psion_asic7_device::update_ramen()
{
	if (m_intr_state || m_nmi_state)
		m_a7_control |= 0x01;
	else
		m_a7_control &= ~0x01;
}
