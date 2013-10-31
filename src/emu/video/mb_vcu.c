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

static ADDRESS_MAP_START( mb_vcu_vram, AS_0, 16, mb_vcu_device )
//	AM_RANGE() internal ROM space (shared with 0x4000 - 0x5fff)
//	AM_RANGE() RAM space (shared with 0x6000 - 0x67ff)
//	AM_RANGE() fb area
ADDRESS_MAP_END

//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *mb_vcu_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == AS_0) ? &m_space_config : NULL;
}

//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  read_byte - read a byte at the given address
//-------------------------------------------------

inline UINT16 mb_vcu_device::read_byte(offs_t address)
{
	return space().read_byte(address);
}

//-------------------------------------------------
//  write_word - write a word at the given address
//-------------------------------------------------

inline void mb_vcu_device::write_byte(offs_t address, UINT8 data)
{
	space().write_byte(address, data);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mb_vcu_device - constructor
//-------------------------------------------------

mb_vcu_device::mb_vcu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MB_VCU, "Mazer Blazer custom VCU", tag, owner, clock, "mb_vcu", __FILE__),
	  device_memory_interface(mconfig, *this),
	  device_video_interface(mconfig, *this),
	  m_space_config("videoram", ENDIANNESS_LITTLE, 8, 16, 0, NULL, *ADDRESS_MAP_NAME(mb_vcu_vram))
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
		//m_screen_tag = NULL;
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
	// TODO: m_screen_tag
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

READ8_MEMBER( mb_vcu_device::read_ram )
{
	return 0;
}

WRITE8_MEMBER( mb_vcu_device::write_ram )
{
}

WRITE8_MEMBER( mb_vcu_device::write_vregs )
{
}

READ8_MEMBER( mb_vcu_device::load_params )
{
	return 0;
}

READ8_MEMBER( mb_vcu_device::load_gfx )
{
	return 0;
}

READ8_MEMBER( mb_vcu_device::load_clr )
{
	return 0;
}

WRITE8_MEMBER( mb_vcu_device::background_color_w )
{
}

READ8_MEMBER( mb_vcu_device::status_r )
{
	return 0;
}

WRITE8_MEMBER( mb_vcu_device::vbank_w )
{
}

//-------------------------------------------------
//  update_screen -
//-------------------------------------------------

UINT32 mb_vcu_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}
