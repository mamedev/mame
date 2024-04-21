// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Colour Card 500 - CTS Recognition

    The CTS Palette is a 1 MHz bus device and it can be used to convert the 8
    colours available on the BBC into any 8 colours from a range of 4096.

    Programming the hardware directly
    ---------------------------------
    The hardware consists of 3 banks of very fast RAM (for red, green and blue
    levels) and a 6522 versatile interface adaptor.

      The memory map is as follows:

      6522 VIA        &FC90 to &FC9F

      RED BANK        &FCA0 to &FCA7
      GREEN BANK      &FCA8 to &FCAF
      BLUE BANK       &FCB0 to &FCB8

    There are 8 bytes in each bank, one for each actual colour. The physical
    colour for logical colour L is stored at &FCA0+L, &FCA8+L and &FCB0+L.

    For example, to reprogram actual colour 1 to brown, 10 would be stored in
    &FCA1, 6 in &FCA9 and 0 in &FCB1.

    Port A of the 6522 VIA is available for use as an extra 'user port', and
    Port B is used internally.

      Note : Line (Horizontal) sync pulses are available on PB6
             Field (Vertical) sync pulses are available on CB1

**********************************************************************/


#include "emu.h"
#include "cc500.h"

#include <algorithm>


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_CC500, bbc_cc500_device, "bbc_cc500", "CTS Colour Card 500")


//-------------------------------------------------
//  ROM( cc500 )
//-------------------------------------------------

ROM_START(cc500)
	ROM_REGION(0x4000, "exp_rom", 0)
	ROM_LOAD("cts_palette_1.10.rom", 0x0000, 0x2000, CRC(93533144) SHA1(e645afdc5c7574a487ff484b79ea21a3b476f847))
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_cc500_device::device_add_mconfig(machine_config &config)
{
	MOS6522(config, m_via, DERIVED_CLOCK(1, 1));
	m_via->readpa_handler().set(m_userport, FUNC(bbc_userport_slot_device::pb_r));
	m_via->writepa_handler().set(m_userport, FUNC(bbc_userport_slot_device::pb_w));
	m_via->irq_handler().set(DEVICE_SELF_OWNER, FUNC(bbc_1mhzbus_slot_device::irq_w));

	BBC_USERPORT_SLOT(config, m_userport, bbc_userport_devices, nullptr);
	m_userport->cb1_handler().set(m_via, FUNC(via6522_device::write_ca1));
	m_userport->cb2_handler().set(m_via, FUNC(via6522_device::write_ca2));

	BBC_1MHZBUS_SLOT(config, m_1mhzbus, DERIVED_CLOCK(1, 1), bbc_1mhzbus_devices, nullptr);
	m_1mhzbus->irq_handler().set(DEVICE_SELF_OWNER, FUNC(bbc_1mhzbus_slot_device::irq_w));
	m_1mhzbus->nmi_handler().set(DEVICE_SELF_OWNER, FUNC(bbc_1mhzbus_slot_device::nmi_w));
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry* bbc_cc500_device::device_rom_region() const
{
	return ROM_NAME(cc500);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_cc500_device - constructor
//-------------------------------------------------

bbc_cc500_device::bbc_cc500_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_CC500, tag, owner, clock)
	, device_bbc_1mhzbus_interface(mconfig, *this)
	, m_palette(*this, ":palette")
	, m_1mhzbus(*this, "1mhzbus")
	, m_via(*this, "via")
	, m_userport(*this, "userport")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_cc500_device::device_start()
{
	std::fill(std::begin(m_palette_ram), std::end(m_palette_ram), rgb_t(0));

	// register for save states
	save_item(NAME(m_palette_ram));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t bbc_cc500_device::fred_r(offs_t offset)
{
	uint8_t data = 0xff;

	switch (offset & 0xf0)
	{
	case 0x90:
		data = m_via->read(offset & 0x0f);
		break;
	}

	data &= m_1mhzbus->fred_r(offset);

	return data;
}

void bbc_cc500_device::fred_w(offs_t offset, uint8_t data)
{
	switch (offset & 0xf0)
	{
	case 0x90:
		m_via->write(offset & 0x0f, data);
		break;

	case 0xa0: case 0xb0:
		switch (offset & 0x18)
		{
		case 0x00:
			m_palette_ram[~offset & 0x07].set_r(pal4bit(data));
			break;
		case 0x08:
			m_palette_ram[~offset & 0x07].set_g(pal4bit(data));
			break;
		case 0x10:
			m_palette_ram[~offset & 0x07].set_b(pal4bit(data));
			break;
		}
		m_palette->set_pen_colors(0, &m_palette_ram[0], 8);
		break;
	}

	m_1mhzbus->fred_w(offset, data);
}

uint8_t bbc_cc500_device::jim_r(offs_t offset)
{
	return m_1mhzbus->jim_r(offset);
}

void bbc_cc500_device::jim_w(offs_t offset, uint8_t data)
{
	m_1mhzbus->jim_w(offset, data);
}
