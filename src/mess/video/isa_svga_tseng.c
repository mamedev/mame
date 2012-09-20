/***************************************************************************

  ISA SVGA Tseng wrapper

***************************************************************************/

#include "emu.h"
#include "isa_svga_tseng.h"
#include "video/pc_vga.h"

ROM_START( et4000 )
	ROM_REGION(0x8000,"et4000", 0)
	ROM_LOAD("et4000.bin", 0x00000, 0x8000, CRC(f1e817a8) SHA1(945d405b0fb4b8f26830d495881f8587d90e5ef9) )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type ISA8_SVGA_ET4K = &device_creator<isa8_svga_et4k_device>;


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor isa8_svga_et4k_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( pcvideo_vga_isa );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *isa8_svga_et4k_device::device_rom_region() const
{
	return ROM_NAME( et4000 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa8_vga_device - constructor
//-------------------------------------------------

isa8_svga_et4k_device::isa8_svga_et4k_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
        device_t(mconfig, ISA8_SVGA_ET4K, "SVGA Tseng ET4000 Graphics Card", tag, owner, clock),
		device_isa8_card_interface(mconfig, *this)
{
	m_shortname = "et4000";
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
READ8_MEMBER(isa8_svga_et4k_device::input_port_0_r ) { return 0xff; } //return space.machine().root_device().ioport("IN0")->read(); }

void isa8_svga_et4k_device::device_start()
{
	set_isa_device();

	video_start_vga( machine() );

	pc_vga_init(machine(), read8_delegate(FUNC(isa8_svga_et4k_device::input_port_0_r),this), NULL);

	int i;
	for (i = 0; i < 0x100; i++)
		palette_set_color_rgb(machine(), i, 0, 0, 0);
	pc_video_start(machine());

	m_isa->install_rom(this, 0xc0000, 0xc7fff, 0, 0, "et4000", "et4000");

	m_isa->install_device(0x3b0, 0x3bf, 0, 0, FUNC(tseng_et4k_03b0_r), FUNC(tseng_et4k_03b0_w));
	m_isa->install_device(0x3c0, 0x3cf, 0, 0, FUNC(tseng_et4k_03c0_r), FUNC(tseng_et4k_03c0_w));
	m_isa->install_device(0x3d0, 0x3df, 0, 0, FUNC(tseng_et4k_03d0_r), FUNC(tseng_et4k_03d0_w));

	m_isa->install_memory(0xa0000, 0xbffff, 0, 0, FUNC(tseng_mem_r), FUNC(tseng_mem_w));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa8_svga_et4k_device::device_reset()
{
	pc_vga_reset(machine());
}
