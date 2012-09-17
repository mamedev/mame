/***************************************************************************

  ISA SVGA Cirrus Logic wrapper

***************************************************************************/

#include "emu.h"
#include "isa_svga_cirrus.h"
#include "video/pc_vga.h"

ROM_START( dm_clgd5430 )
	ROM_REGION(0x8000,"dm_clgd5430", 0)
	ROM_LOAD("speedstar_pro_se_v1.00.u2", 0x00000, 0x8000, CRC(ed79572c) SHA1(15131e2b2db7a34971083a250e4a21ab7bd64a9d) )
	ROM_IGNORE( 0x8000 )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type ISA8_SVGA_CIRRUS = &device_creator<isa8_svga_cirrus_device>;


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor isa8_svga_cirrus_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( pcvideo_vga_isa );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *isa8_svga_cirrus_device::device_rom_region() const
{
	return ROM_NAME( dm_clgd5430 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa8_vga_device - constructor
//-------------------------------------------------

isa8_svga_cirrus_device::isa8_svga_cirrus_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
        device_t(mconfig, ISA8_SVGA_CIRRUS, "Diamond Multimedia Graphics Card", tag, owner, clock),
		device_isa8_card_interface(mconfig, *this)
{
	m_shortname = "dm_clgd5430";
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
static READ8_HANDLER( input_port_0_r ) { return 0xff; } //return space.machine().root_device().ioport("IN0")->read(); }

void isa8_svga_cirrus_device::device_start()
{
	set_isa_device();

	video_start_vga( machine() );

	pc_vga_init(machine(), input_port_0_r, NULL);

	int i;
	for (i = 0; i < 0x100; i++)
		palette_set_color_rgb(machine(), i, 0, 0, 0);
	pc_video_start(machine());

	m_isa->install_rom(this, 0xc0000, 0xc7fff, 0, 0, "svga", "dm_clgd5430");

	m_isa->install_device(0x03b0, 0x03bf, 0, 0, FUNC(vga_port_03b0_r), FUNC(vga_port_03b0_w));
	m_isa->install_device(0x03c0, 0x03cf, 0, 0, FUNC(vga_port_03c0_r), FUNC(vga_port_03c0_w));
	m_isa->install_device(0x03d0, 0x03df, 0, 0, FUNC(vga_port_03d0_r), FUNC(vga_port_03d0_w));
//  m_isa->install_device(0x9ae8, 0x9aeb, 0, 0, FUNC(s3_port_9ae8_r), FUNC(s3_port_9ae8_w));

	m_isa->install_memory(0xa0000, 0xbffff, 0, 0, FUNC(vga_mem_r), FUNC(vga_mem_w));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa8_svga_cirrus_device::device_reset()
{
	pc_vga_reset(machine());
}
