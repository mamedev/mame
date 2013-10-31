// license: ?
// copyright-holders: Angelo Salese
/***************************************************************************

Device for Mazer Blazer/Great Guns custom Video Controller Unit

***************************************************************************/

#include "emu.h"
#include "video/mb_vcu.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type MB_VCU = &device_creator<mb_vcu_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mb_vcu_device - constructor
//-------------------------------------------------

mb_vcu_device::mb_vcu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MB_VCU, "Mazer Blazer custom VCU", tag, owner, clock, "mb_vcu", __FILE__),
	  device_video_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void mb_vcu_device::device_config_complete()
{
	// inherit a copy of the static data
	const mb_vcu_interface *intf = reinterpret_cast<const mb_vcu_interface *>(static_config());
	if (intf != NULL)
	{
		*static_cast<mb_vcu_interface *>(this) = *intf;
	}

	// or initialize to defaults if none provided
	else
	{
		m_cpu_tag = NULL;
	}
}

//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

void mb_vcu_device::device_validity_check(validity_checker &valid) const
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mb_vcu_device::device_start()
{
	if(m_cpu_tag)
	{
		m_cpu = machine().device(m_cpu_tag);
	}
	else
	{
		m_cpu = NULL;
	}

}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mb_vcu_device::device_reset()
{
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

READ8_MEMBER( mb_vcu_device::read )
{
	return 0;
}

WRITE8_MEMBER( mb_vcu_device::write )
{
}

//-------------------------------------------------
//  update_screen -
//-------------------------------------------------

UINT32 mb_vcu_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}
