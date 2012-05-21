/**********************************************************************

    SMC CRT9212 Double Row Buffer (DRB) emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "emu.h"
#include "crt9212.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 1


#define REN \
	m_in_ren_func()

#define WEN \
	m_in_wen_func()

#define WEN2 \
	m_in_wen2_func()

#define ROF(_state) \
	m_out_rof_func(_state);

#define WOF(_state) \
	m_out_wof_func(_state);



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
const device_type CRT9212 = &device_creator<crt9212_device>;

//-------------------------------------------------
//  crt9212_device - constructor
//-------------------------------------------------

crt9212_device::crt9212_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, CRT9212, "SMC CRT9212", tag, owner, clock)
{
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void crt9212_device::device_config_complete()
{
	// inherit a copy of the static data
	const crt9212_interface *intf = reinterpret_cast<const crt9212_interface *>(static_config());
	if (intf != NULL)
		*static_cast<crt9212_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		memset(&m_out_rof_cb, 0, sizeof(m_out_rof_cb));
		memset(&m_out_wof_cb, 0, sizeof(m_out_wof_cb));
		memset(&m_in_ren_cb, 0, sizeof(m_in_ren_cb));
		memset(&m_in_wen_cb, 0, sizeof(m_in_wen_cb));
		memset(&m_in_wen2_cb, 0, sizeof(m_in_wen2_cb));
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void crt9212_device::device_start()
{
	// resolve callbacks
	m_out_rof_func.resolve(m_out_rof_cb, *this);
	m_out_wof_func.resolve(m_out_wof_cb, *this);
	m_in_ren_func.resolve(m_in_ren_cb, *this);
	m_in_wen_func.resolve(m_in_wen_cb, *this);
	m_in_wen2_func.resolve(m_in_wen2_cb, *this);

	// register for state saving
	save_item(NAME(m_input));
	save_item(NAME(m_output));
	save_item(NAME(m_buffer));
	save_item(NAME(m_rac));
	save_item(NAME(m_wac));
	save_item(NAME(m_tog));
	save_item(NAME(m_clrcnt));
	save_item(NAME(m_rclk));
	save_item(NAME(m_wclk));
}


//-------------------------------------------------
//  read - buffer read
//-------------------------------------------------

READ8_MEMBER( crt9212_device::read )
{
	return m_output;
}


//-------------------------------------------------
//  write - buffer write
//-------------------------------------------------

WRITE8_MEMBER( crt9212_device::write )
{
	m_input = data;
}


//-------------------------------------------------
//  clrcnt_w - clear address counters
//-------------------------------------------------

WRITE_LINE_MEMBER( crt9212_device::clrcnt_w )
{
	m_clrcnt = state;
}


//-------------------------------------------------
//  tog_w - toggle buffer
//-------------------------------------------------

WRITE_LINE_MEMBER( crt9212_device::tog_w )
{
	m_tog = state;
}


//-------------------------------------------------
//  rclk_w - read clock
//-------------------------------------------------

WRITE_LINE_MEMBER( crt9212_device::rclk_w )
{
	if (m_rclk && !state)
	{
		if (!m_clrcnt)
		{
			if (!m_tog)
			{
				// switch buffer
				m_buffer = !m_buffer;

				// clear write address counter
				m_wac = 0;
				WOF(0);
			}
			else
			{
				// clear read address counter
				m_rac = 0;
				ROF(0);
			}
		}
		else
		{
			if (REN && (m_rac < CRT9212_RAM_SIZE))
			{
				//
				m_output = m_ram[m_rac][!m_buffer];

				// increment read address counter
				m_rac++;

				if (m_rac == CRT9212_RAM_SIZE)
				{
					// set read overflow
					ROF(1);
				}
			}
		}
	}

	m_rclk = state;
}


//-------------------------------------------------
//  wclk_w - write clock
//-------------------------------------------------

WRITE_LINE_MEMBER( crt9212_device::wclk_w )
{
	if (!m_rclk && state)
	{
		if (WEN && WEN2 && (m_wac < CRT9212_RAM_SIZE))
		{
			//
			m_ram[m_rac][m_buffer] = m_input;

			// increment read address counter
			m_wac++;

			if (m_wac == CRT9212_RAM_SIZE)
			{
				// set write overflow
				WOF(1);
			}
		}
	}

	m_wclk = state;
}
