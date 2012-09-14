/*
 * isa_vga_ati.c
 *
 *	Implementation of the ATi Graphics Ultra ISA Video card
 *	 - Uses ATi 28800-6 (VGA Wonder) and ATi 38800-1 (Mach8, 8514/A clone)
 *
 *  Created on: 9/09/2012
 */

#include "emu.h"
#include "isa_vga_ati.h"
#include "video/pc_vga.h"

ROM_START( gfxultra )
	ROM_REGION(0x8000,"gfxultra", 0)
	ROM_LOAD("113-11504-002.bin", 0x00000, 0x8000, CRC(f498b36a) SHA1(117cfc972ce4645538ba7262222d8ff38bc2c58c) )
	ROM_IGNORE( 0x8000 )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type ISA16_VGA_GFXULTRA = &device_creator<isa16_vga_gfxultra_device>;


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor isa16_vga_gfxultra_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( pcvideo_ati_isa );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *isa16_vga_gfxultra_device::device_rom_region() const
{
	return ROM_NAME( gfxultra );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa8_vga_device - constructor
//-------------------------------------------------

isa16_vga_gfxultra_device::isa16_vga_gfxultra_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
        device_t(mconfig, ISA16_VGA_GFXULTRA, "ATi Graphics Ultra Graphics Card", tag, owner, clock),
		device_isa16_card_interface(mconfig, *this)
{
	m_shortname = "gfxultra";
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
static READ8_HANDLER( input_port_0_r ) { return 0xff; } //return space->machine().root_device().ioport("IN0")->read(); }

void isa16_vga_gfxultra_device::device_start()
{
	set_isa_device();

	video_start_vga( machine() );

	pc_vga_init(machine(), input_port_0_r, NULL);

	int i;
	for (i = 0; i < 0x100; i++)
		palette_set_color_rgb(machine(), i, 0, 0, 0);
	pc_video_start(machine());

	m_isa->install_rom(this, 0xc0000, 0xc7fff, 0, 0, "vga", "gfxultra");

	m_isa->install_device(this, 0x1ce, 0x1cf, 0, 0, FUNC(ati_port_ext_r), FUNC(ati_port_ext_w));
	m_isa->install16_device(0x2e8, 0x2eb, 0, 0, FUNC(mach8_status_r), FUNC(mach8_htotal_w));
	m_isa->install_device(0x3b0, 0x3bf, 0, 0, FUNC(vga_port_03b0_r), FUNC(vga_port_03b0_w));
	m_isa->install_device(0x3c0, 0x3cf, 0, 0, FUNC(ati_port_03c0_r), FUNC(vga_port_03c0_w));
	m_isa->install_device(0x3d0, 0x3df, 0, 0, FUNC(vga_port_03d0_r), FUNC(vga_port_03d0_w));
	m_isa->install16_device(0x12e8, 0x12eb, 0, 0, FUNC(mach8_vtotal_r),FUNC(mach8_vtotal_w));
	m_isa->install16_device(0x12ec, 0x12ef, 0, 0, FUNC(mach8_config1_r),NULL,NULL);
	m_isa->install16_device(0x16e8, 0x16eb, 0, 0, FUNC(mach8_vdisp_r),FUNC(mach8_vdisp_w));
	m_isa->install16_device(0x16ec, 0x16ef, 0, 0, FUNC(mach8_config2_r),NULL,NULL);
	m_isa->install16_device(0x1ae8, 0x1aeb, 0, 0, FUNC(mach8_vsync_r),FUNC(mach8_vsync_w));
	m_isa->install16_device(0x26e8, 0x26eb, 0, 0, FUNC(mach8_htotal_r),NULL,NULL);
	m_isa->install16_device(0x2ee8, 0x2eeb, 0, 0, FUNC(mach8_subcontrol_r),NULL,NULL);
	m_isa->install16_device(0x42e8, 0x42eb, 0, 0, FUNC(mach8_substatus_r), FUNC(mach8_subcontrol_w));
	m_isa->install16_device(0x52e8, 0x52eb, 0, 0, FUNC(mach8_ec0_r), FUNC(mach8_ec0_w));
	m_isa->install16_device(0x52ec, 0x52ef, 0, 0, FUNC(mach8_scratch0_r), FUNC(mach8_scratch0_w));
	m_isa->install16_device(0x56e8, 0x56eb, 0, 0, FUNC(mach8_ec1_r), FUNC(mach8_ec1_w));
	m_isa->install16_device(0x56ec, 0x56ef, 0, 0, FUNC(mach8_scratch0_r), FUNC(mach8_scratch0_w));
	m_isa->install16_device(0x5ae8, 0x5aeb, 0, 0, FUNC(mach8_ec2_r), FUNC(mach8_ec2_w));
	m_isa->install16_device(0x5ee8, 0x5eeb, 0, 0, FUNC(mach8_ec3_r), FUNC(mach8_ec3_w));
	m_isa->install16_device(0x82e8, 0x82eb, 0, 0, FUNC(s3_currenty_r), FUNC(s3_currenty_w));
	m_isa->install16_device(0x86e8, 0x86eb, 0, 0, FUNC(s3_currentx_r), FUNC(s3_currentx_w));
	m_isa->install16_device(0x8ae8, 0x8aeb, 0, 0, FUNC(s3_8ae8_r), FUNC(s3_8ae8_w));
	m_isa->install16_device(0x8ee8, 0x8eeb, 0, 0, FUNC(s3_8ee8_r), FUNC(s3_8ee8_w));
	m_isa->install16_device(0x92e8, 0x92eb, 0, 0, FUNC(s3_line_error_r), FUNC(s3_line_error_w));
	m_isa->install16_device(0x96e8, 0x96eb, 0, 0, FUNC(s3_width_r), FUNC(s3_width_w));
	m_isa->install16_device(0x96ec, 0x96ef, 0, 0, FUNC(mach8_bresenham_count_r), FUNC(mach8_bresenham_count_w));
	m_isa->install16_device(0x9ae8, 0x9aeb, 0, 0, FUNC(ibm8514_gpstatus_r), FUNC(ibm8514_cmd_w));
	m_isa->install16_device(0x9aec, 0x9aef, 0, 0, FUNC(mach8_ext_fifo_r), FUNC(mach8_linedraw_index_w));
	m_isa->install16_device(0x9ee8, 0x9eeb, 0, 0, FUNC(ibm8514_ssv_r), FUNC(ibm8514_ssv_w));
	m_isa->install16_device(0xa2e8, 0xa2eb, 0, 0, FUNC(s3_bgcolour_r), FUNC(s3_bgcolour_w));
	m_isa->install16_device(0xa6e8, 0xa6eb, 0, 0, FUNC(s3_fgcolour_r), FUNC(s3_fgcolour_w));
	m_isa->install16_device(0xb6e8, 0xb6eb, 0, 0, FUNC(s3_backmix_r), FUNC(s3_backmix_w));
	m_isa->install16_device(0xbae8, 0xbaeb, 0, 0, FUNC(s3_foremix_r), FUNC(s3_foremix_w));
	m_isa->install16_device(0xbee8, 0xbeeb, 0, 0, FUNC(s3_multifunc_r), FUNC(s3_multifunc_w));
	m_isa->install16_device(0xe2e8, 0xe2eb, 0, 0, FUNC(s3_pixel_xfer_r), FUNC(s3_pixel_xfer_w));
	m_isa->install16_device(0xfeec, 0xfeef, 0, 0, NULL, NULL, FUNC(mach8_linedraw_w));

	m_isa->install_memory(0xa0000, 0xbffff, 0, 0, FUNC(ati_mem_r), FUNC(ati_mem_w));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa16_vga_gfxultra_device::device_reset()
{
	pc_vga_reset(machine());
}
