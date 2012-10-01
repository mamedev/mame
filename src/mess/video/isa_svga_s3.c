/***************************************************************************

  ISA SVGA S3 wrapper

***************************************************************************/

#include "emu.h"
#include "isa_svga_s3.h"
#include "video/pc_vga.h"

ROM_START( s3_764 )
	ROM_REGION(0x8000,"s3_764", 0)
	ROM_LOAD("s3_764.bin", 0x00000, 0x8000, CRC(4f10aac7) SHA1(c77b3f11cc15679121314823588887dd547cd715) )
	ROM_IGNORE( 0x8000 )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type ISA16_SVGA_S3 = &device_creator<isa16_svga_s3_device>;


static MACHINE_CONFIG_FRAGMENT( vga_s3 )
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_25_1748MHz,900,0,640,526,0,480)
	MCFG_SCREEN_UPDATE_DEVICE("vga", s3_vga_device, screen_update)

	MCFG_PALETTE_LENGTH(0x100)
	
	MCFG_DEVICE_ADD("vga", S3_VGA, 0)
MACHINE_CONFIG_END

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor isa16_svga_s3_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( vga_s3 );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *isa16_svga_s3_device::device_rom_region() const
{
	return ROM_NAME( s3_764 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa8_vga_device - constructor
//-------------------------------------------------

isa16_svga_s3_device::isa16_svga_s3_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
        device_t(mconfig, ISA16_SVGA_S3, "SVGA S3 Graphics Card", tag, owner, clock),
		device_isa16_card_interface(mconfig, *this)
{
	m_shortname = "s3_764";
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
READ8_MEMBER(isa16_svga_s3_device::input_port_0_r ) { return 0xff; } //return space.machine().root_device().ioport("IN0")->read(); }

void isa16_svga_s3_device::device_start()
{
	set_isa_device();

	m_vga = subdevice<s3_vga_device>("vga");
	
	m_isa->install_rom(this, 0xc0000, 0xc7fff, 0, 0, "svga", "s3_764");

	m_isa->install_device(0x03b0, 0x03bf, 0, 0, read8_delegate(FUNC(s3_vga_device::port_03b0_r),m_vga), write8_delegate(FUNC(s3_vga_device::port_03b0_w),m_vga));
	m_isa->install_device(0x03c0, 0x03cf, 0, 0, read8_delegate(FUNC(s3_vga_device::port_03c0_r),m_vga), write8_delegate(FUNC(s3_vga_device::port_03c0_w),m_vga));
	m_isa->install_device(0x03d0, 0x03df, 0, 0, read8_delegate(FUNC(s3_vga_device::port_03d0_r),m_vga), write8_delegate(FUNC(s3_vga_device::port_03d0_w),m_vga));
	m_isa->install16_device(0x82e8, 0x82eb, 0, 0, read16_delegate(FUNC(s3_vga_device::ibm8514_currenty_r),m_vga), write16_delegate(FUNC(s3_vga_device::ibm8514_currenty_w),m_vga));
	m_isa->install16_device(0x86e8, 0x86eb, 0, 0, read16_delegate(FUNC(s3_vga_device::ibm8514_currentx_r),m_vga), write16_delegate(FUNC(s3_vga_device::ibm8514_currentx_w),m_vga));
	m_isa->install16_device(0x8ae8, 0x8aeb, 0, 0, read16_delegate(FUNC(s3_vga_device::ibm8514_desty_r),m_vga), write16_delegate(FUNC(s3_vga_device::ibm8514_desty_w),m_vga));
	m_isa->install16_device(0x8ee8, 0x8eeb, 0, 0, read16_delegate(FUNC(s3_vga_device::ibm8514_destx_r),m_vga), write16_delegate(FUNC(s3_vga_device::ibm8514_destx_w),m_vga));
	m_isa->install16_device(0x92e8, 0x92eb, 0, 0, read16_delegate(FUNC(s3_vga_device::s3_line_error_r),m_vga), write16_delegate(FUNC(s3_vga_device::s3_line_error_w),m_vga));
	m_isa->install16_device(0x96e8, 0x96eb, 0, 0, read16_delegate(FUNC(s3_vga_device::s3_width_r),m_vga), write16_delegate(FUNC(s3_vga_device::s3_width_w),m_vga));
	m_isa->install16_device(0x9ae8, 0x9aeb, 0, 0, read16_delegate(FUNC(s3_vga_device::s3_gpstatus_r),m_vga), write16_delegate(FUNC(s3_vga_device::s3_cmd_w),m_vga));
	m_isa->install16_device(0x9ee8, 0x9eeb, 0, 0, read16_delegate(FUNC(s3_vga_device::ibm8514_ssv_r),m_vga), write16_delegate(FUNC(s3_vga_device::ibm8514_ssv_w),m_vga));
	m_isa->install16_device(0xa2e8, 0xa2eb, 0, 0, read16_delegate(FUNC(s3_vga_device::s3_bgcolour_r),m_vga), write16_delegate(FUNC(s3_vga_device::s3_bgcolour_w),m_vga));
	m_isa->install16_device(0xa6e8, 0xa6eb, 0, 0, read16_delegate(FUNC(s3_vga_device::s3_fgcolour_r),m_vga), write16_delegate(FUNC(s3_vga_device::s3_fgcolour_w),m_vga));
	m_isa->install16_device(0xb6e8, 0xb6eb, 0, 0, read16_delegate(FUNC(s3_vga_device::s3_backmix_r),m_vga), write16_delegate(FUNC(s3_vga_device::s3_backmix_w),m_vga));
	m_isa->install16_device(0xbae8, 0xbaeb, 0, 0, read16_delegate(FUNC(s3_vga_device::s3_foremix_r),m_vga), write16_delegate(FUNC(s3_vga_device::s3_foremix_w),m_vga));
	m_isa->install16_device(0xbee8, 0xbeeb, 0, 0, read16_delegate(FUNC(s3_vga_device::s3_multifunc_r),m_vga), write16_delegate(FUNC(s3_vga_device::s3_multifunc_w),m_vga));
	m_isa->install16_device(0xe2e8, 0xe2eb, 0, 0, read16_delegate(FUNC(s3_vga_device::s3_pixel_xfer_r),m_vga), write16_delegate(FUNC(s3_vga_device::s3_pixel_xfer_w),m_vga));

	m_isa->install_memory(0xa0000, 0xbffff, 0, 0, read8_delegate(FUNC(s3_vga_device::mem_r),m_vga), write8_delegate(FUNC(s3_vga_device::mem_w),m_vga));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa16_svga_s3_device::device_reset()
{
}
