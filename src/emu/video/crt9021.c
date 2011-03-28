/**********************************************************************

    SMC CRT9021 Video Attributes Controller (VAC) emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

/*

    TODO:

    - attributes
        - reverse video
        - character blank
        - character blink
        - underline
        - full/half intensity
    - operation modes
        - wide graphics
        - thin graphics
        - character mode w/o underline
        - character mode w/underline
    - double height characters
    - double width characters
    - parallel scan line
    - serial scan line
    - cursor
        - underline
        - blinking underline
        - reverse video
        - blinking reverse video
    - programmable character blink rate (75/25 duty)
    - programmable cursor blink rate (50/50 duty)
    - data/attribute latches

*/

#include "emu.h"
#include "crt9021.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 1


// attributes
enum
{
	ATTRIBUTE_REVID = 0x80,
	ATTRIBUTE_INT   = 0x40,
	ATTRIBUTE_BLINK = 0x20,
	ATTRIBUTE_MS1   = 0x10,
	ATTRIBUTE_MS0   = 0x08,
	ATTRIBUTE_CHABL = 0x04,
	ATTRIBUTE_BKC   = 0x02,
	ATTRIBUTE_BLC   = 0x01
};


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// devices
const device_type CRT9021 = crt9021_device_config::static_alloc_device_config;



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  crt9021_device_config - constructor
//-------------------------------------------------

crt9021_device_config::crt9021_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
	: device_config(mconfig, static_alloc_device_config, "SMC CRT9021", tag, owner, clock)
{
}


//-------------------------------------------------
//  static_alloc_device_config - allocate a new
//  configuration object
//-------------------------------------------------

device_config *crt9021_device_config::static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
{
	return global_alloc(crt9021_device_config(mconfig, tag, owner, clock));
}


//-------------------------------------------------
//  alloc_device - allocate a new device object
//-------------------------------------------------

device_t *crt9021_device_config::alloc_device(running_machine &machine) const
{
	return auto_alloc(&machine, crt9021_device(machine, *this));
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void crt9021_device_config::device_config_complete()
{
	// inherit a copy of the static data
	const crt9021_interface *intf = reinterpret_cast<const crt9021_interface *>(static_config());
	if (intf != NULL)
		*static_cast<crt9021_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		memset(&in_data_func, 0, sizeof(in_data_func));
		memset(&in_attr_func, 0, sizeof(in_attr_func));
		memset(&in_atten_func, 0, sizeof(in_atten_func));
	}
}



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  crt9021_device - constructor
//-------------------------------------------------

crt9021_device::crt9021_device(running_machine &_machine, const crt9021_device_config &config)
    : device_t(_machine, config),
      m_config(config)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void crt9021_device::device_start()
{
	// allocate timers

	// resolve callbacks
	devcb_resolve_read8(&m_in_data_func, &m_config.in_data_func, this);
	devcb_resolve_read8(&m_in_attr_func, &m_config.in_attr_func, this);
	devcb_resolve_read_line(&m_in_atten_func, &m_config.in_atten_func, this);

	// get the screen device
	m_screen = machine->device<screen_device>(m_config.screen_tag);
	assert(m_screen != NULL);

	// register for state saving
	save_item(NAME(m_slg));
	save_item(NAME(m_sld));
	save_item(NAME(m_cursor));
	save_item(NAME(m_retbl));
	save_item(NAME(m_vsync));
}


//-------------------------------------------------
//  device_clock_changed - handle clock change
//-------------------------------------------------

void crt9021_device::device_clock_changed()
{
}


//-------------------------------------------------
//  device_timer - handle timer events
//-------------------------------------------------

void crt9021_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
}


//-------------------------------------------------
//  slg_w - scan line gate
//-------------------------------------------------

WRITE_LINE_MEMBER( crt9021_device::slg_w )
{
	if (LOG) logerror("CRT9021 '%s' SLG: %u\n", tag(), state);

	m_slg = state;
}


//-------------------------------------------------
//  sld_w - scan line data
//-------------------------------------------------

WRITE_LINE_MEMBER( crt9021_device::sld_w )
{
	if (LOG) logerror("CRT9021 '%s' SLG: %u\n", tag(), state);

	if (!m_slg)
	{
		m_sld <<= 1;
		m_sld |= state;
	}
}


//-------------------------------------------------
//  cursor_w - cursor
//-------------------------------------------------

WRITE_LINE_MEMBER( crt9021_device::cursor_w )
{
	if (LOG) logerror("CRT9021 '%s' CURSOR: %u\n", tag(), state);

	m_cursor = state;
}


//-------------------------------------------------
//  retbl_w - retrace blank
//-------------------------------------------------

WRITE_LINE_MEMBER( crt9021_device::retbl_w )
{
	if (LOG) logerror("CRT9021 '%s' RETBL: %u\n", tag(), state);

	m_retbl = state;
}


//-------------------------------------------------
//  vsync_w - vertical sync
//-------------------------------------------------

WRITE_LINE_MEMBER( crt9021_device::vsync_w )
{
	if (LOG) logerror("CRT9021 '%s' VSYNC: %u\n", tag(), state);

	m_vsync = state;
}


//-------------------------------------------------
//  update_screen - update screen
//-------------------------------------------------

void crt9021_device::update_screen(bitmap_t *bitmap, const rectangle *cliprect)
{
}
