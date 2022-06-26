// license:BSD-3-Clause
// copyright-holders:Katherine Rohl
/***************************************************************************

    IBM PS/2 Planar VGA.

***************************************************************************/

#include "emu.h"
#include "planar_vga.h"

#define VERBOSE 1
#include "logmacro.h"

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

#define XTAL_U155 	28.322_MHz_XTAL
#define XTAL_U156 	25.175_MHz_XTAL

DEFINE_DEVICE_TYPE(MCA16_PLANAR_VGA, mca16_planar_vga_device, "mca16_planar_vga", "IBM PS/2 Planar VGA")

// static void mca_com(device_slot_interface &device)
// {

// }

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void mca16_planar_vga_device::device_add_mconfig(machine_config &config)
{
	// Onboard VGA
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL_U155, 800, 0, 640, 524, 0, 480);
	screen.set_screen_update(m_vga, FUNC(vga_device::screen_update));

	VGA(config, m_vga, 0);
	m_vga->set_screen("screen");
	m_vga->set_vram_size(0x40000); // 256K of VRAM
}

//-------------------------------------------------
//  mca16_planar_vga_device - constructor
//-------------------------------------------------

mca16_planar_vga_device::mca16_planar_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	mca16_planar_vga_device(mconfig, MCA16_PLANAR_VGA, tag, owner, clock)
{
}

mca16_planar_vga_device::mca16_planar_vga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_mca16_card_interface(mconfig, *this, 0xffff),
    m_vga(*this, "vga"),
    m_mcabus(*this, ":mb:mcabus")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mca16_planar_vga_device::device_start()
{
	set_mca_device();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mca16_planar_vga_device::device_reset()
{
    m_is_mapped = 0;
}

void mca16_planar_vga_device::enable()
{
    LOG("%s\n", FUNCNAME);

    m_mcabus->install_memory(0xa0000, 0xbffff, read8sm_delegate(*m_vga, FUNC(vga_device::mem_r)), write8sm_delegate(*m_vga, FUNC(vga_device::mem_w)));
    m_mcabus->install_device(0x3b0, 0x3ba, 
        read8sm_delegate(*this, FUNC(mca16_planar_vga_device::port_03b0_r)),
        write8sm_delegate(*this, FUNC(mca16_planar_vga_device::port_03b0_w)));
    m_mcabus->install_device(0x3c0, 0x3cf, 
        read8sm_delegate(*this, FUNC(mca16_planar_vga_device::port_03c0_r)),
        write8sm_delegate(*this, FUNC(mca16_planar_vga_device::port_03c0_w)));
    m_mcabus->install_device(0x3d0, 0x3df, 
        read8sm_delegate(*this, FUNC(mca16_planar_vga_device::port_03d0_r)),
        write8sm_delegate(*this, FUNC(mca16_planar_vga_device::port_03d0_w)));
    m_is_mapped = 1;
}

void mca16_planar_vga_device::disable()
{
    LOG("%s\n", FUNCNAME);
    m_mcabus->unmap_readwrite(0xA0000, 0xBFFFF);
    m_mcabus->unmap_device(0x3b0, 0x3ba);
    m_mcabus->unmap_device(0x3c0, 0x3cf);
    m_mcabus->unmap_device(0x3d0, 0x3df);
    m_is_mapped = 0;
}

void mca16_planar_vga_device::planar_remap(int space_id, offs_t start, offs_t end)
{  
    // Can't be remapped.
}

void mca16_planar_vga_device::planar_remap_irq(uint8_t new_irq)
{
    // Always IRQ 2/9.
}

uint8_t mca16_planar_vga_device::port_03b0_r(offs_t offset)
{
    assert_card_feedback();
    return m_vga->port_03b0_r(offset);
}

void mca16_planar_vga_device::port_03b0_w(offs_t offset, uint8_t data)
{
    assert_card_feedback();
    m_vga->port_03b0_w(offset, data);
}

uint8_t mca16_planar_vga_device::port_03c0_r(offs_t offset)
{
    assert_card_feedback();
    return m_vga->port_03c0_r(offset);
}

void mca16_planar_vga_device::port_03c0_w(offs_t offset, uint8_t data)
{
    assert_card_feedback();
    m_vga->port_03c0_w(offset, data);
}

uint8_t mca16_planar_vga_device::port_03d0_r(offs_t offset)
{
    assert_card_feedback();
    return m_vga->port_03d0_r(offset);
}

void mca16_planar_vga_device::port_03d0_w(offs_t offset, uint8_t data)
{
    assert_card_feedback();
    m_vga->port_03d0_w(offset, data);
}

void mca16_planar_vga_device::sleep_w(uint8_t data)
{
    m_sleep = data & 0x01;
    m_sleep ? enable() : disable();
}