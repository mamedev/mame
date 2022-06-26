// license:BSD-3-Clause
// copyright-holders:Katherine Rohl
/***************************************************************************

    IBM SVGA Adapter
    MCA ID @917B

    Basic GD5428-based video card with 1MB of DRAM.
    Fits in a 16-bit slot with AVE.
    Will conflict with any onboard VGA.

    No POS configuration - the VGA BIOS is always at C000-C7FFh.

***************************************************************************/

#include "emu.h"
#include "ibm_svga.h"

#define VERBOSE 1
#include "logmacro.h"

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

#define MCA_CARD_ID 0x917b

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(MCA16_IBM_SVGA, mca16_ibm_svga_device, "mca_ibm_svga", "IBM SVGA Adapter (@917B)")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void mca16_ibm_svga_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(25.1748_MHz_XTAL, 900, 0, 640, 526, 0, 480);
	screen.set_screen_update("vga", FUNC(cirrus_gd5428_device::screen_update));

	cirrus_gd5428_device &vga(CIRRUS_GD5428(config, "vga", 0));
	vga.set_screen("screen");
	vga.set_vram_size(0x100000);
}

//-------------------------------------------------
//  mca16_template_device - constructor
//-------------------------------------------------

mca16_ibm_svga_device::mca16_ibm_svga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	mca16_ibm_svga_device(mconfig, MCA16_IBM_SVGA, tag, owner, clock)
{
}

mca16_ibm_svga_device::mca16_ibm_svga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_mca16_card_interface(mconfig, *this, MCA_CARD_ID),
    m_svga(*this, "svga")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mca16_ibm_svga_device::device_start()
{
	set_mca_device();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mca16_ibm_svga_device::device_reset()
{
    if(m_is_mapped) unmap();
    m_svga->reset();
}

void mca16_ibm_svga_device::unmap()
{
    // All standard VGA.
    m_mca->unmap_readwrite(0xa0000, 0xbffff);   // VGA memory
    m_mca->unmap_rom(0xc0000, 0xc7fff);         // VGA BIOS
    m_mca->unmap_device(0x3b0, 0x3ba);          // I/O
    m_mca->unmap_device(0x3c0, 0x3cf);          // I/O
    m_mca->unmap_device(0x3d0, 0x3df);          // I/O
}

void mca16_ibm_svga_device::remap()
{
    m_mca->install_memory(0xa0000, 0xbffff, read8sm_delegate(*m_svga, FUNC(cirrus_gd5428_device::mem_r)), write8sm_delegate(*m_svga, FUNC(cirrus_gd5428_device::mem_w)));
    m_mca->install_rom(this, 0xc0000, 0xc7fff, "option");                   // VGA BIOS is always at C0000-C7FFF.
    // Need local trampolines to assert channel feedback
    m_mca->install_device(0x3b0, 0x3ba, 
        read8sm_delegate(*this, FUNC(mca16_ibm_svga_device::port_03b0_r)),
        write8sm_delegate(*this, FUNC(mca16_ibm_svga_device::port_03b0_w)));
    m_mca->install_device(0x3c0, 0x3cf, 
        read8sm_delegate(*this, FUNC(mca16_ibm_svga_device::port_03c0_r)),
        write8sm_delegate(*this, FUNC(mca16_ibm_svga_device::port_03c0_w)));
    m_mca->install_device(0x3d0, 0x3df, 
        read8sm_delegate(*this, FUNC(mca16_ibm_svga_device::port_03d0_r)),
        write8sm_delegate(*this, FUNC(mca16_ibm_svga_device::port_03d0_w)));
}

uint8_t mca16_ibm_svga_device::port_03b0_r(offs_t offset)
{
    assert_card_feedback();
    return m_svga->port_03b0_r(offset);
}

void mca16_ibm_svga_device::port_03b0_w(offs_t offset, uint8_t data)
{
    assert_card_feedback();
    m_svga->port_03b0_w(offset, data);
}

uint8_t mca16_ibm_svga_device::port_03c0_r(offs_t offset)
{
    assert_card_feedback();
    return m_svga->port_03c0_r(offset);
}

void mca16_ibm_svga_device::port_03c0_w(offs_t offset, uint8_t data)
{
    assert_card_feedback();
    m_svga->port_03c0_w(offset, data);
}

uint8_t mca16_ibm_svga_device::port_03d0_r(offs_t offset)
{
    assert_card_feedback();
    return m_svga->port_03d0_r(offset);
}

void mca16_ibm_svga_device::port_03d0_w(offs_t offset, uint8_t data)
{
    assert_card_feedback();
    m_svga->port_03d0_w(offset, data);
}

ROM_START( ibm_svga )
	ROM_REGION( 0xc0000, "option", 0 )
	ROM_LOAD( "06h6915.bin", 0x0000, 0x8000, CRC(39d4566d) SHA1(d275193a870250f212dc29768d4e68fb43770e95) )
ROM_END

const tiny_rom_entry *mca16_ibm_svga_device::device_rom_region() const
{
	return ROM_NAME( ibm_svga );
}

uint8_t mca16_ibm_svga_device::pos_r(offs_t offset)
{
	LOG("%s\n", FUNCNAME);

	switch(offset)
	{
		case 0:
			// Adapter Identification b0-b7
			return get_card_id() & 0xFF;
		case 1:
			// Adapter Identification b8-b15
			return (get_card_id() & 0xFF00) >> 8;
		case 2:
			// Option Select Data 1
			break;
		case 3:
			// Option Select Data 2
			break;
		case 4:
			// Option Select Data 3
			break;
		case 5:
			// Option Select Data 4
			break;
		case 6:
			// Subaddress Extension Low
			break;
		case 7:
			// Subaddress Extension High
			break;
		default:
			break;
	}

	return 0xFF;
}

void mca16_ibm_svga_device::pos_w(offs_t offset, uint8_t data)
{
	LOG("%s\n", FUNCNAME);

	switch(offset)
	{
		case 0:
			// Adapter Identification b0-b7
		case 1:
			// Adapter Identification b8-b15
			break;
		case 2:
			// Option Select Data 1
			break;
		case 3:
			// Option Select Data 2
			break;
		case 4:
			// Option Select Data 3
			break;
		case 5:
			// Option Select Data 4
			break;
		case 6:
			// Subaddress Extension Low
			break;
		case 7:
			// Subaddress Extension High
			break;
		default:
			break;
	}
}