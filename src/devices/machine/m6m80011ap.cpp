// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

Template for skeleton device

***************************************************************************/

#include "emu.h"
#include "machine/m6m80011ap.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type M6M80011AP = &device_creator<m6m80011ap_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  m6m80011ap_device - constructor
//-------------------------------------------------

m6m80011ap_device::m6m80011ap_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, M6M80011AP, "M6M80011AP EEPROM", tag, owner, clock, "m6m80011ap", __FILE__),
		device_nvram_interface(mconfig, *this), m_latch(0), m_reset_line(0), m_cmd_stream_pos(0), m_current_cmd(0), m_read_latch(0), m_current_addr(0), m_eeprom_we(0), m_eeprom_state()
{
}


//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

void m6m80011ap_device::device_validity_check(validity_checker &valid) const
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void m6m80011ap_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void m6m80011ap_device::device_reset()
{
}

//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void m6m80011ap_device::nvram_default()
{
	for (auto & elem : m_eeprom_data)
		elem = 0xffff;
}




//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

void m6m80011ap_device::nvram_read(emu_file &file)
{
	file.read(m_eeprom_data, 0x100);
}


//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

void m6m80011ap_device::nvram_write(emu_file &file)
{
	file.write(m_eeprom_data, 0x100);
}

//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************


READ_LINE_MEMBER( m6m80011ap_device::read_bit )
{
	return m_read_latch;
}

READ_LINE_MEMBER( m6m80011ap_device::ready_line )
{
	return 1; // TODO
}

WRITE_LINE_MEMBER( m6m80011ap_device::set_cs_line )
{
	m_reset_line = state;

	if (m_reset_line != CLEAR_LINE)
	{
		m_eeprom_state = EEPROM_GET_CMD;
		m_cmd_stream_pos = 0;
		m_current_cmd = 0;
	}
}


WRITE_LINE_MEMBER( m6m80011ap_device::write_bit )
{
	m_latch = state;
}

WRITE_LINE_MEMBER( m6m80011ap_device::set_clock_line )
{
	if (m_reset_line == CLEAR_LINE)
	{
		if(state == 1)
		{
			switch(m_eeprom_state)
			{
				case EEPROM_GET_CMD:
					m_current_cmd = (m_current_cmd >> 1) | ((m_latch & 1)<< 7);
					m_cmd_stream_pos++;

					if (m_cmd_stream_pos==8)
					{
						m_cmd_stream_pos = 0;
						switch(m_current_cmd)
						{
							case 0xc5: m_eeprom_state = EEPROM_WRITE_ENABLE; break;
							case 0x05: m_eeprom_state = EEPROM_WRITE_DISABLE; break;
							case 0x25: m_eeprom_state = EEPROM_WRITE; break;
							case 0x15: m_eeprom_state = EEPROM_READ; break;
							case 0x95: m_eeprom_state = EEPROM_STATUS_OUTPUT; break;
							default:
								printf("Write M6M80011 unknown %02x cmd\n",m_current_cmd );
								break;
						}
					}
					break;

				case EEPROM_READ:
					m_current_cmd = (m_current_cmd >> 1) | ((m_latch & 1)<< 23);
					m_cmd_stream_pos++;

					if (m_cmd_stream_pos==8)
					{
						m_current_addr = m_current_cmd >> 16;
					}

					if(m_cmd_stream_pos>=8)
					{
						m_read_latch = (m_eeprom_data[m_current_addr] >> (23-m_cmd_stream_pos)) & 1;
						//printf("%d %04x <- %04x %d\n",m_read_latch,m_eeprom_data[m_current_addr],m_current_addr,m_cmd_stream_pos-8);
					}

					if(m_cmd_stream_pos==24)
					{
						m_eeprom_state = EEPROM_GET_CMD;
						m_cmd_stream_pos = 0;
					}
					break;

				case EEPROM_WRITE:
					m_current_cmd = (m_current_cmd >> 1) | ((m_latch & 1)<< 23);
					m_cmd_stream_pos++;

					if (m_cmd_stream_pos==8)
					{
						m_current_addr = m_current_cmd >> 16;
					}

					if(m_cmd_stream_pos==24)
					{
						if(m_eeprom_we)
							m_eeprom_data[m_current_addr] = (m_current_cmd >> 8) & 0xffff;

						//printf("%04x %04x -> %04x\n",m_eeprom_data[m_current_addr],m_current_addr,m_current_cmd >> 8);

						m_eeprom_state = EEPROM_GET_CMD;
						m_cmd_stream_pos = 0;
					}
					break;

				case EEPROM_WRITE_ENABLE:
				case EEPROM_WRITE_DISABLE:
					m_current_cmd = (m_current_cmd >> 1) | ((m_latch & 1)<< 7);
					m_cmd_stream_pos++;

					if (m_cmd_stream_pos==8)
					{
						m_eeprom_we = (m_eeprom_state == EEPROM_WRITE_ENABLE) ? 1 : 0;
						m_eeprom_state = EEPROM_GET_CMD;
						m_cmd_stream_pos = 0;
					}

					break;

				case EEPROM_STATUS_OUTPUT:
					m_current_cmd = (m_current_cmd >> 1) | ((m_latch & 1)<< 7);
					m_cmd_stream_pos++;

					if (m_cmd_stream_pos==8)
					{
						printf("Status output\n");
						m_eeprom_state = EEPROM_GET_CMD;
						m_cmd_stream_pos = 0;
					}
					break;
			}
		}
	}
}
