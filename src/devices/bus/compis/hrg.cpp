// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    TeleNova Compis (Ultra) High Resolution Graphics adapter emulation

**********************************************************************/

#include "hrg.h"


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define UPD7220_TAG		"upd7220"
#define SCREEN_TAG      "screen"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type COMPIS_HRG = &device_creator<compis_hrg_t>;
const device_type COMPIS_UHRG = &device_creator<compis_uhrg_t>;


//-------------------------------------------------
//  ADDRESS_MAP( upd7220_map )
//-------------------------------------------------

static ADDRESS_MAP_START( hrg_map, AS_0, 16, compis_hrg_t )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x00000, 0x7fff) AM_RAM AM_SHARE("video_ram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( uhrg_map, AS_0, 16, compis_uhrg_t )
	ADDRESS_MAP_GLOBAL_MASK(0x1ffff)
	AM_RANGE(0x00000, 0x1ffff) AM_RAM AM_SHARE("video_ram")
ADDRESS_MAP_END


//-------------------------------------------------
//  UPD7220_INTERFACE( hgdc_intf )
//-------------------------------------------------

UPD7220_DISPLAY_PIXELS_MEMBER( compis_hrg_t::display_pixels )
{
	UINT16 i,gfx = m_video_ram[(address & 0x7fff) >> 1];
	const pen_t *pen = m_palette->pens();

	for(i=0; i<16; i++)
		bitmap.pix32(y, x + i) = pen[BIT(gfx, i)];
}


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( hrg )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( hrg )
	MCFG_SCREEN_ADD_MONOCHROME(SCREEN_TAG, RASTER, rgb_t::green)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) // not accurate
	MCFG_SCREEN_SIZE(640, 400)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 400-1)
	MCFG_SCREEN_UPDATE_DEVICE(UPD7220_TAG, upd7220_device, screen_update)

	MCFG_DEVICE_ADD(UPD7220_TAG, UPD7220, XTAL_4_433619MHz/2) // unknown clock
	MCFG_DEVICE_ADDRESS_MAP(AS_0, hrg_map)
	MCFG_UPD7220_DISPLAY_PIXELS_CALLBACK_OWNER(compis_hrg_t, display_pixels)
	MCFG_VIDEO_SET_SCREEN(SCREEN_TAG)

	MCFG_PALETTE_ADD_MONOCHROME("palette")
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor compis_hrg_t::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( hrg );
}


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( uhrg )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( uhrg )
	MCFG_SCREEN_ADD_MONOCHROME(SCREEN_TAG, RASTER, rgb_t::green)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) // not accurate
	MCFG_SCREEN_SIZE(1280, 800)
	MCFG_SCREEN_VISIBLE_AREA(0, 1280-1, 0, 800-1)
	MCFG_SCREEN_UPDATE_DEVICE(UPD7220_TAG, upd7220_device, screen_update)

	MCFG_DEVICE_ADD(UPD7220_TAG, UPD7220, XTAL_4_433619MHz/2) // unknown clock
	MCFG_DEVICE_ADDRESS_MAP(AS_0, uhrg_map)
	MCFG_UPD7220_DISPLAY_PIXELS_CALLBACK_OWNER(compis_hrg_t, display_pixels)
	MCFG_VIDEO_SET_SCREEN(SCREEN_TAG)

	MCFG_PALETTE_ADD_MONOCHROME("palette")
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor compis_uhrg_t::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( uhrg );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  compis_hrg_t - constructor
//-------------------------------------------------

compis_hrg_t::compis_hrg_t(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	device_compis_graphics_card_interface(mconfig, *this),
	m_crtc(*this, UPD7220_TAG),
	m_palette(*this, "palette"),
	m_video_ram(*this, "video_ram")
{
}

compis_hrg_t::compis_hrg_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	compis_hrg_t(mconfig, COMPIS_HRG, "Compis HRG", tag, owner, clock, "compis_hrg", __FILE__)
{
}

compis_uhrg_t::compis_uhrg_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	compis_hrg_t(mconfig, COMPIS_UHRG, "Compis UHRG", tag, owner, clock, "compis_uhrg", __FILE__)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void compis_hrg_t::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void compis_hrg_t::device_reset()
{
}


//-------------------------------------------------
//  pcs6_6_r -
//-------------------------------------------------

UINT8 compis_hrg_t::pcs6_6_r(address_space &space, offs_t offset)
{
	UINT8 data = 0xff;

	if (offset < 2)
		data = m_crtc->read(space, offset & 0x01);
	else
		// monochrome only, hblank? vblank?
		if(offset == 2)
		{
			switch(m_unk_video)
			{
				case 0x04:
					m_unk_video = 0x44;
					break;
				case 0x44:
					m_unk_video = 0x64;
					break;
				default:
					m_unk_video = 0x04;
					break;
			}
			data = m_unk_video;
		}
	else
		data = 0;

	logerror("%s PCS 6:6 read %04x : %02x\n", machine().describe_context(), offset, data);

	return data;
}


//-------------------------------------------------
//  pcs6_6_w -
//-------------------------------------------------

void compis_hrg_t::pcs6_6_w(address_space &space, offs_t offset, UINT8 data)
{
	logerror("%s PCS 6:6 write %04x : %02x\n", machine().describe_context(), offset, data);

	// 0x336 is likely the color plane register
	if (offset < 2) m_crtc->write(space, offset & 0x01, data);
}
