/***************************************************************************

Mitsubishi M50458 OSD chip

***************************************************************************/

#include "emu.h"
#include "video/m50458.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type M50458 = &device_creator<m50458_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  m50458_device - constructor
//-------------------------------------------------

m50458_device::m50458_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, M50458, "m50458", tag, owner, clock)
{

}


//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

void m50458_device::device_validity_check(validity_checker &valid) const
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void m50458_device::device_start()
{

}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void m50458_device::device_reset()
{
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

WRITE_LINE_MEMBER( m50458_device::write_bit )
{
	m_latch = state;
}

WRITE_LINE_MEMBER( m50458_device::set_cs_line )
{
	m_reset_line = state;

	if(m_reset_line != CLEAR_LINE)
	{
		printf("Reset asserted\n");
		m_cmd_stream_pos = 0;
		m_current_cmd = 0;
		m_osd_state = OSD_SET_ADDRESS;
	}
}

/*
0x7e: "JUST A MOMENT"
0x81: "NON SLOT"
*/

WRITE_LINE_MEMBER( m50458_device::set_clock_line )
{
	if (m_reset_line == CLEAR_LINE)
	{
		if(state == 1)
		{
			//printf("%d\n",m_latch);

			m_current_cmd = (m_current_cmd >> 1) | ((m_latch<<15)&0x8000);
			m_cmd_stream_pos++;

			if(m_cmd_stream_pos == 16)
			{
				switch(m_osd_state)
				{
					case OSD_SET_ADDRESS:
						m_osd_addr = m_current_cmd;
						m_osd_state = OSD_SET_DATA;
						break;
					case OSD_SET_DATA:
						printf("%04x %04x\n",m_osd_addr,m_current_cmd);

						m_osd_addr++;
						/* Presumably wraps at 0x127? */
						if(m_osd_addr > 0x127) { m_osd_addr = 0; }
						break;
				}

				m_cmd_stream_pos = 0;
				m_current_cmd = 0;
			}
		}
	}
}
