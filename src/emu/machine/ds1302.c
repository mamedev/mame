/************************************************************

    DALLAS DS1302

    RTC + BACKUP RAM



    Emulation by ElSemi


    Missing Features:
      - Burst Mode
      - Clock programming (useless)



    2009-05 Converted to be a device

************************************************************/


#include "emu.h"
#include "ds1302.h"
#include "devhelpr.h"


/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE UINT8 convert_to_bcd(int val)
{
	return ((val / 10) << 4) | (val % 10);
}


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  ds1302_device_config - constructor
//-------------------------------------------------

ds1302_device_config::ds1302_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
    : device_config(mconfig, static_alloc_device_config, "Dallas DS1302 RTC", tag, owner, clock)
{
}


//-------------------------------------------------
//  static_alloc_device_config - allocate a new
//  configuration object
//-------------------------------------------------

device_config *ds1302_device_config::static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
{
    return global_alloc(ds1302_device_config(mconfig, tag, owner, clock));
}


//-------------------------------------------------
//  alloc_device - allocate a new device object
//-------------------------------------------------

device_t *ds1302_device_config::alloc_device(running_machine &machine) const
{
    return auto_alloc(&machine, ds1302_device(machine, *this));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

const device_type DS1302 = ds1302_device_config::static_alloc_device_config;

//-------------------------------------------------
//  ds1302_device - constructor
//-------------------------------------------------

ds1302_device::ds1302_device(running_machine &_machine, const ds1302_device_config &config)
    : device_t(_machine, config),
      m_config(config)
{

}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ds1302_device::device_start()
{
	state_save_register_device_item(this, 0, m_shift_in);
	state_save_register_device_item(this, 0, m_shift_out);
	state_save_register_device_item(this, 0, m_icount);
	state_save_register_device_item(this, 0, m_last_clk);
	state_save_register_device_item(this, 0, m_last_cmd);
	state_save_register_device_item_array(this, 0, m_sram);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ds1302_device::device_reset()
{
	m_shift_in  = 0;
	m_shift_out = 0;
	m_icount    = 0;
	m_last_clk  = 0;
	m_last_cmd  = 0;
}


/*-------------------------------------------------
    ds1302_dat_w
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER_TRAMPOLINE(ds1302, ds1302_dat_w)
{
	if (data)
	{
		m_shift_in |= (1 << m_icount);
	}
	else
	{
		m_shift_in &= ~(1 << m_icount);
	}
}


/*-------------------------------------------------
    ds1302_clk_w
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER_TRAMPOLINE(ds1302, ds1302_clk_w)
{
	if (data != m_last_clk)
	{
		if (data)	//Rising, shift in command
		{
			m_icount++;
			if(m_icount == 8)	//Command start
			{
				system_time systime;
				m_machine.base_datetime(systime);

				switch(m_shift_in)
				{
					case 0x81:	//Sec
						m_shift_out = convert_to_bcd(systime.local_time.second);
						break;

					case 0x83:	//Min
						m_shift_out = convert_to_bcd(systime.local_time.minute);
						break;

					case 0x85:	//Hour
						m_shift_out = convert_to_bcd(systime.local_time.hour);
						break;

					case 0x87:	//Day
						m_shift_out = convert_to_bcd(systime.local_time.mday);
						break;

					case 0x89:	//Month
						m_shift_out = convert_to_bcd(systime.local_time.month + 1);
						break;

					case 0x8b:	//weekday
						m_shift_out = convert_to_bcd(systime.local_time.weekday);
						break;

					case 0x8d:	//Year
						m_shift_out = convert_to_bcd(systime.local_time.year % 100);
						break;

					default:
						m_shift_out = 0x0;
				}

				if(m_shift_in > 0xc0)
				{
					m_shift_out = m_sram[(m_shift_in >> 1) & 0x1f];
				}
				m_last_cmd = m_shift_in & 0xff;
				m_icount++;
			}

			if(m_icount == 17 && !(m_last_cmd & 1))
			{
				UINT8 val = (m_shift_in >> 9) & 0xff;

				switch(m_last_cmd)
				{
					case 0x80:	//Sec
						break;

					case 0x82:	//Min
						break;

					case 0x84:	//Hour
						break;

					case 0x86:	//Day
						break;

					case 0x88:	//Month
						break;

					case 0x8a:	//weekday
						break;

					case 0x8c:	//Year
						break;

					default:
						m_shift_out = 0x0;
				}

				if(m_last_cmd > 0xc0)
				{
					m_sram[(m_last_cmd >> 1) & 0x1f] = val;
				}



			}
		}
	}
	m_last_clk = data;
}


/*-------------------------------------------------
    ds1302_read
-------------------------------------------------*/

READ8_DEVICE_HANDLER_TRAMPOLINE(ds1302, ds1302_read)
{
	return (m_shift_out & (1 << (m_icount - 9))) ? 1 : 0;
}
