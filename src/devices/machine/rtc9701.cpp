// license:BSD-3-Clause
// copyright-holders:Angelo Salese, David Haywood
/***************************************************************************

    rtc9701.c

    Epson RTC-9701-JE

    Serial Real Time Clock + EEPROM


***************************************************************************/

#include "emu.h"
#include "machine/rtc9701.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type rtc9701 = &device_creator<rtc9701_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  rtc9701_device - constructor
//-------------------------------------------------

rtc9701_device::rtc9701_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, rtc9701, "RTC-9701", tag, owner, clock, "rtc9701", __FILE__),
		device_nvram_interface(mconfig, *this),
		m_latch(0),
		m_reset_line(CLEAR_LINE),
		m_clock_line(CLEAR_LINE), rtc_state(), cmd_stream_pos(0), current_cmd(0), rtc9701_address_pos(0), rtc9701_current_address(0), rtc9701_current_data(0), rtc9701_data_pos(0)
{
}

void rtc9701_device::timer_callback()
{
	static const UINT8 dpm[12] = { 0x31, 0x28, 0x31, 0x30, 0x31, 0x30, 0x31, 0x31, 0x30, 0x31, 0x30, 0x31 };
	int dpm_count;

	m_rtc.sec++;

	if((m_rtc.sec & 0x0f) >= 0x0a)              { m_rtc.sec+=0x10; m_rtc.sec&=0xf0; }
	if((m_rtc.sec & 0xf0) >= 0x60)              { m_rtc.min++; m_rtc.sec = 0; }
	if((m_rtc.min & 0x0f) >= 0x0a)              { m_rtc.min+=0x10; m_rtc.min&=0xf0; }
	if((m_rtc.min & 0xf0) >= 0x60)              { m_rtc.hour++; m_rtc.min = 0; }
	if((m_rtc.hour & 0x0f) >= 0x0a)             { m_rtc.hour+=0x10; m_rtc.hour&=0xf0; }
	if((m_rtc.hour & 0xff) >= 0x24)             { m_rtc.day++; m_rtc.wday<<=1; m_rtc.hour = 0; }
	if(m_rtc.wday & 0x80)                       { m_rtc.wday = 1; }
	if((m_rtc.day & 0x0f) >= 0x0a)              { m_rtc.day+=0x10; m_rtc.day&=0xf0; }

	/* TODO: crude leap year support */
	dpm_count = (m_rtc.month & 0xf) + (((m_rtc.month & 0x10) >> 4)*10)-1;

	if(((m_rtc.year % 4) == 0) && m_rtc.month == 2)
	{
		if((m_rtc.day & 0xff) >= dpm[dpm_count]+1+1)
			{ m_rtc.month++; m_rtc.day = 0x01; }
	}
	else if((m_rtc.day & 0xff) >= dpm[dpm_count]+1){ m_rtc.month++; m_rtc.day = 0x01; }
	if((m_rtc.month & 0x0f) >= 0x0a)            { m_rtc.month = 0x10; }
	if(m_rtc.month >= 0x13)                     { m_rtc.year++; m_rtc.month = 1; }
	if((m_rtc.year & 0x0f) >= 0x0a)             { m_rtc.year+=0x10; m_rtc.year&=0xf0; }
	if((m_rtc.year & 0xf0) >= 0xa0)             { m_rtc.year = 0; } //2000-2099 possible timeframe
}

TIMER_CALLBACK( rtc9701_device::rtc_inc_callback )
{
	reinterpret_cast<rtc9701_device *>(ptr)->timer_callback();
}

//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

void rtc9701_device::device_validity_check(validity_checker &valid) const
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void rtc9701_device::device_start()
{
	/* let's call the timer callback every second */
	machine().scheduler().timer_pulse(attotime::from_hz(clock() / XTAL_32_768kHz), FUNC(rtc_inc_callback), 0, (void *)this);

	system_time systime;
	machine().base_datetime(systime);

	m_rtc.day = ((systime.local_time.mday / 10)<<4) | ((systime.local_time.mday % 10) & 0xf);
	m_rtc.month = (((systime.local_time.month+1) / 10) << 4) | (((systime.local_time.month+1) % 10) & 0xf);
	m_rtc.wday = 1 << systime.local_time.weekday;
	m_rtc.year = (((systime.local_time.year % 100)/10)<<4) | ((systime.local_time.year % 10) & 0xf);
	m_rtc.hour = ((systime.local_time.hour / 10)<<4) | ((systime.local_time.hour % 10) & 0xf);
	m_rtc.min = ((systime.local_time.minute / 10)<<4) | ((systime.local_time.minute % 10) & 0xf);
	m_rtc.sec = ((systime.local_time.second / 10)<<4) | ((systime.local_time.second % 10) & 0xf);

	rtc_state = RTC9701_CMD_WAIT;
	cmd_stream_pos = 0;
	current_cmd = 0;
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void rtc9701_device::device_reset()
{
}



//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void rtc9701_device::nvram_default()
{
	for (auto & elem : rtc9701_data)
		elem = 0xffff;
}




//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

void rtc9701_device::nvram_read(emu_file &file)
{
	file.read(rtc9701_data, 0x200);
}


//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

void rtc9701_device::nvram_write(emu_file &file)
{
	file.write(rtc9701_data, 0x200);
}

//-------------------------------------------------
//  rtc_read - used to route RTC reading registers
//-------------------------------------------------

inline UINT8 rtc9701_device::rtc_read(UINT8 offset)
{
	UINT8 res;

	res = 0;

	switch(offset)
	{
		case 0: res = m_rtc.sec; break;
		case 1: res = m_rtc.min; break;
		case 2: res = m_rtc.hour; break;
		case 3: res = m_rtc.wday; break; /* untested */
		case 4: res = m_rtc.day; break;
		case 5: res = m_rtc.month; break;
		case 6: res = m_rtc.year & 0xff; break;
		case 7: res = 0x20; break;
	}

	return res;
}

inline void rtc9701_device::rtc_write(UINT8 offset,UINT8 data)
{
	switch(offset)
	{
		case 0: m_rtc.sec = data; break;
		case 1: m_rtc.min = data; break;
		case 2: m_rtc.hour = data; break;
		case 3: m_rtc.wday = data; break; /* untested */
		case 4: m_rtc.day = data; break;
		case 5: m_rtc.month = data; break;
		case 6: m_rtc.year = data; break;
		case 7: break; // NOP
	}
}

//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

WRITE_LINE_MEMBER( rtc9701_device::write_bit )
{
	m_latch = state;
}


READ_LINE_MEMBER( rtc9701_device::read_bit )
{
	if (rtc_state == RTC9701_RTC_READ)
	{
		//printf("RTC data bits left c9701_data_pos %02x\n", rtc9701_data_pos);
		return ((rtc9701_current_data) >> (rtc9701_data_pos-1))&1;

	}
	else if (rtc_state == RTC9701_EEPROM_READ)
	{
		//printf("EEPROM data bits left c9701_data_pos %02x\n", rtc9701_data_pos);
		return ((rtc9701_current_data) >> (rtc9701_data_pos-1))&1;

	}
	else
	{
		//printf("read something else (status?) %02x\n", rtc9701_data_pos);
	}



	return 0;
}


WRITE_LINE_MEMBER( rtc9701_device::set_cs_line )
{
	//logerror("set reset line %d\n",state);
	m_reset_line = state;

	if (m_reset_line != CLEAR_LINE)
	{
		rtc_state = RTC9701_CMD_WAIT;
		cmd_stream_pos = 0;
		current_cmd = 0;
		rtc9701_address_pos = 0;
		rtc9701_current_address = 0;
		rtc9701_current_data = 0;
		rtc9701_data_pos = 0;

	}
}



WRITE_LINE_MEMBER( rtc9701_device::set_clock_line )
{
	//logerror("set clock line %d\n",state);

	if (m_reset_line == CLEAR_LINE)
	{
		if (state==1)
		{
			//logerror("write latched bit %d\n",m_latch);

			switch (rtc_state)
			{
				case RTC9701_CMD_WAIT:

					//logerror("xx\n");
					current_cmd = (current_cmd << 1) | (m_latch&1);
					cmd_stream_pos++;

					if (cmd_stream_pos==4)
					{
						cmd_stream_pos = 0;
						//logerror("Comamnd is %02x\n", current_cmd);

						if (current_cmd==0x00) /* 0000 */
						{
							//logerror("WRITE RTC MODE\n");
							rtc_state = RTC9701_RTC_WRITE;
							cmd_stream_pos = 0;
							rtc9701_address_pos = 0;
							rtc9701_current_address = 0;
							rtc9701_data_pos = 0;
							rtc9701_current_data = 0;
						}
						else if (current_cmd==0x02) /* 0010 */
						{
							//logerror("WRITE EEPROM MODE\n");
							rtc_state = RTC9701_EEPROM_WRITE;
							cmd_stream_pos = 0;
							rtc9701_address_pos = 0;
							rtc9701_current_address = 0;
							rtc9701_data_pos = 0;
							rtc9701_current_data = 0;

						}
						else if (current_cmd==0x06) /* 0110 */
						{
							//logerror("WRITE ENABLE\n");
							rtc_state = RTC9701_AFTER_WRITE_ENABLE;
							cmd_stream_pos = 0;
						}
						else if (current_cmd==0x08) /* 1000 */
						{
							//logerror("READ RTC MODE\n");
							rtc_state = RTC9701_RTC_READ;
							cmd_stream_pos = 0;
							rtc9701_address_pos = 0;
							rtc9701_current_address = 0;
							rtc9701_data_pos = 0;
							rtc9701_current_data = 0;
						}
						else if (current_cmd==0x0a) /* 1010 */
						{
							//logerror("READ EEPROM MODE\n");
							rtc_state = RTC9701_EEPROM_READ;
							cmd_stream_pos = 0;
							rtc9701_address_pos = 0;
							rtc9701_current_address = 0;
							rtc9701_data_pos = 0;
							rtc9701_current_data = 0;


						}
						else
						{
							//logerror("RTC9701 UNKNOWN MODE\n");
						}

						current_cmd = 0;
					}
					break;

				case RTC9701_AFTER_WRITE_ENABLE:
					cmd_stream_pos++;
					if (cmd_stream_pos==12)
					{
						cmd_stream_pos = 0;
						//logerror("Written 12 bits, going back to WAIT mode\n");
						rtc_state = RTC9701_CMD_WAIT;
					}
					break;

				case RTC9701_RTC_WRITE:
					cmd_stream_pos++;
					if (cmd_stream_pos<=4)
					{
						rtc9701_address_pos++;
						rtc9701_current_address = (rtc9701_current_address << 1) | (m_latch&1);
						if (cmd_stream_pos==4)
						{
							//printf("Set RTC Write Address To %04x\n", rtc9701_current_address );
						}
					}

					if (cmd_stream_pos>4)
					{
						rtc9701_data_pos++;
						rtc9701_current_data = (rtc9701_current_data << 1) | (m_latch&1);;
					}

					if (cmd_stream_pos==12)
					{
						cmd_stream_pos = 0;
						rtc_write(rtc9701_current_address,rtc9701_current_data);
						//logerror("Written 12 bits, going back to WAIT mode\n");
						rtc_state = RTC9701_CMD_WAIT;
					}
					break;



				case RTC9701_EEPROM_READ:
					cmd_stream_pos++;
					if (cmd_stream_pos<=12)
					{
						rtc9701_address_pos++;
						rtc9701_current_address = (rtc9701_current_address << 1) | (m_latch&1);
						if (cmd_stream_pos==12)
						{
							//printf("Set EEPROM Read Address To %04x - ", (rtc9701_current_address>>1)&0xff );
							rtc9701_current_data = rtc9701_data[(rtc9701_current_address>>1)&0xff];
							//printf("Setting data latch for reading to %04x\n", rtc9701_current_data);
							rtc9701_data_pos = 16;
						}
					}

					if (cmd_stream_pos>12)
					{
						rtc9701_data_pos--;

					}

					if (cmd_stream_pos==28)
					{
						cmd_stream_pos = 0;
					//  //logerror("accesed 28 bits, going back to WAIT mode\n");
					//  rtc_state = RTC9701_CMD_WAIT;
					}
					break;



				case RTC9701_EEPROM_WRITE:
					cmd_stream_pos++;

					if (cmd_stream_pos<=12)
					{
						rtc9701_address_pos++;
						rtc9701_current_address = (rtc9701_current_address << 1) | (m_latch&1);
						if (cmd_stream_pos==12)
						{
							//printf("Set EEPROM Write Address To %04x\n", rtc9701_current_address );
						}
					}

					if (cmd_stream_pos>12)
					{
						rtc9701_data_pos++;
						rtc9701_current_data = (rtc9701_current_data << 1) | (m_latch&1);;
					}

					if (cmd_stream_pos==28)
					{
						cmd_stream_pos = 0;
						//printf("written 28 bits - writing data %04x to %04x and going back to WAIT mode\n", rtc9701_current_data, (rtc9701_current_address>>1)&0xff);
						rtc9701_data[(rtc9701_current_address>>1)&0xff] = rtc9701_current_data;
						rtc_state = RTC9701_CMD_WAIT;
					}
					break;

				case RTC9701_RTC_READ:
					cmd_stream_pos++;
					if (cmd_stream_pos<=4)
					{
						rtc9701_address_pos++;
						rtc9701_current_address = (rtc9701_current_address << 1) | (m_latch&1);
						if (cmd_stream_pos==4)
						{
							//printf("Set RTC Read Address To %04x\n", rtc9701_current_address );
							rtc9701_current_data = rtc_read(rtc9701_current_address);
							//printf("Setting data latch for reading to %04x\n", rtc9701_current_data);
							rtc9701_data_pos = 8;
						}
					}

					if (cmd_stream_pos>4)
					{
						rtc9701_data_pos--;
					}

					if (cmd_stream_pos==12)
					{
						cmd_stream_pos = 0;
					//  //logerror("accessed 12 bits, going back to WAIT mode\n");
					//  rtc_state = RTC9701_CMD_WAIT;
					}
					break;


				default:
					break;

			}
		}
	}
}
