/***************************************************************************

Seiko/Epson S-3520CF

preliminary device by Angelo Salese

TODO:
- kludge on address?

***************************************************************************/

#include "emu.h"
#include "machine/s3520cf.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type S3520CF = &device_creator<s3520cf_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  s3520cf_device - constructor
//-------------------------------------------------

s3520cf_device::s3520cf_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, S3520CF, "s3520cf", tag, owner, clock)
{

}


//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

void s3520cf_device::device_validity_check(validity_checker &valid) const
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void s3520cf_device::device_start()
{

}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void s3520cf_device::device_reset()
{
}

//-------------------------------------------------
//  rtc_read - used to route RTC reading registers
//-------------------------------------------------

inline UINT8 s3520cf_device::rtc_read(UINT8 offset)
{
	UINT8 res;

	res = 0;

	switch(offset)
	{
//		case 0: // 1 sec
//		case 1: // 10 sec
//		case 2: // 1 min
//		case 3: // 10 min
//		case 6: // week
//		case 7: // 1 day
		case 4: // 1 hour
			res = 1;
			break;
		case 5: // 10 hour
			res = 2;
			break;
	}



	return res;
}

inline void s3520cf_device::rtc_write(UINT8 offset,UINT8 data)
{

}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

READ_LINE_MEMBER( s3520cf_device::read_bit )
{
	return m_read_latch;
}

WRITE_LINE_MEMBER( s3520cf_device::set_dir_line )
{
	//printf("%d DIR LINE\n",state);

	m_dir = state;
}

WRITE_LINE_MEMBER( s3520cf_device::set_cs_line )
{
	m_reset_line = state;

	//printf("%d CS LINE\n",state);

	if(m_reset_line != CLEAR_LINE)
	{
		//printf("Reset asserted\n");
		m_current_cmd = 0;
		m_cmd_stream_pos = 0;
		m_rtc_state = RTC_SET_ADDRESS;
		//m_latch = 0;
	}
}

WRITE_LINE_MEMBER( s3520cf_device::write_bit )
{
	m_latch = state;
//	printf("%d LATCH LINE\n",state);
}

WRITE_LINE_MEMBER( s3520cf_device::set_clock_line )
{
	if(state == 1 && m_reset_line == CLEAR_LINE)
	{
		//printf("%d %d\n",m_latch, m_dir);

		switch(m_rtc_state)
		{
			case RTC_SET_ADDRESS:
				m_current_cmd = (m_current_cmd >> 1) | ((m_latch<<3)&8);
				m_cmd_stream_pos++;

				if(m_cmd_stream_pos == 4)
				{
					m_rtc_addr = (m_current_cmd + 1) & 0xf; /* TODO: +1??? */
					m_rtc_state = RTC_SET_DATA;
					m_cmd_stream_pos = 0;
					m_current_cmd = 0;
				}
				break;
			case RTC_SET_DATA:
				if(m_dir == 1) // READ
				{
					//if(m_cmd_stream_pos == 0)
					{
						//printf("%02x %d\n",m_rtc_addr,m_cmd_stream_pos);
					}
					m_read_latch = (rtc_read(m_rtc_addr) >> (m_cmd_stream_pos)) & 1;
				}

				m_current_cmd = (m_current_cmd >> 1) | ((m_latch<<3)&8);
				m_cmd_stream_pos++;
				if(m_cmd_stream_pos == 4)
				{
					if(m_dir == 0) // WRITE
					{
						printf("%02x %02x\n",m_rtc_addr,m_current_cmd);
						rtc_write(m_rtc_addr,m_current_cmd);
					}

					m_rtc_addr = m_current_cmd;
					m_rtc_state = RTC_SET_ADDRESS;
					m_cmd_stream_pos = 0;
					m_current_cmd = 0;
				}
				break;
		}
	}
}
