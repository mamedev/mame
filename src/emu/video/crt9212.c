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
	devcb_call_read_line(&m_in_ren_func)

#define WEN \
	devcb_call_read_line(&m_in_wen_func)

#define WEN2 \
	devcb_call_read_line(&m_in_wen2_func)

#define ROF(_state) \
	devcb_call_write_line(&m_out_rof_func, _state);

#define WOF(_state) \
	devcb_call_write_line(&m_out_wof_func, _state);



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// devices
const device_type CRT9212 = crt9212_device_config::static_alloc_device_config;



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  crt9212_device_config - constructor
//-------------------------------------------------

crt9212_device_config::crt9212_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
	: device_config(mconfig, static_alloc_device_config, "SMC CRT9212", tag, owner, clock)
{
}


//-------------------------------------------------
//  static_alloc_device_config - allocate a new
//  configuration object
//-------------------------------------------------

device_config *crt9212_device_config::static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
{
	return global_alloc(crt9212_device_config(mconfig, tag, owner, clock));
}


//-------------------------------------------------
//  alloc_device - allocate a new device object
//-------------------------------------------------

device_t *crt9212_device_config::alloc_device(running_machine &machine) const
{
	return auto_alloc(&machine, crt9212_device(machine, *this));
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void crt9212_device_config::device_config_complete()
{
	// inherit a copy of the static data
	const crt9212_interface *intf = reinterpret_cast<const crt9212_interface *>(static_config());
	if (intf != NULL)
		*static_cast<crt9212_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		memset(&out_rof_func, 0, sizeof(out_rof_func));
		memset(&out_wof_func, 0, sizeof(out_wof_func));
		memset(&in_ren_func, 0, sizeof(in_ren_func));
		memset(&in_wen_func, 0, sizeof(in_wen_func));
		memset(&in_wen2_func, 0, sizeof(in_wen2_func));
	}
}



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  crt9212_device - constructor
//-------------------------------------------------

crt9212_device::crt9212_device(running_machine &_machine, const crt9212_device_config &config)
    : device_t(_machine, config),
      m_config(config)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void crt9212_device::device_start()
{
	// resolve callbacks
	devcb_resolve_write_line(&m_out_rof_func, &m_config.out_rof_func, this);
	devcb_resolve_write_line(&m_out_wof_func, &m_config.out_wof_func, this);
	devcb_resolve_read_line(&m_in_ren_func, &m_config.in_ren_func, this);
	devcb_resolve_read_line(&m_in_wen_func, &m_config.in_wen_func, this);
	devcb_resolve_read_line(&m_in_wen2_func, &m_config.in_wen2_func, this);

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
