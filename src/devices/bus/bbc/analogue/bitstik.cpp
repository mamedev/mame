// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn ANF04 Bitstik

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Acorn_BitStik.html

    Robocom Bitstik 2

**********************************************************************/

#include "emu.h"
#include "bitstik.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_BITSTIK1, bbc_bitstik1_device, "bbc_bitstik1", "Acorn Bitstik")
DEFINE_DEVICE_TYPE(BBC_BITSTIK2, bbc_bitstik2_device, "bbc_bitstik2", "Robo Bitstik 2")

//-------------------------------------------------
//  ROM( bitstik )
//-------------------------------------------------

ROM_START(bitstik1)
	ROM_REGION(0x4000, "exp_rom", 0)
	ROM_LOAD("bitstik1.rom", 0x0000, 0x2000, CRC(a3c539f8) SHA1(c6f2cf2f6d1e48819a8381c6a1c97ca8fa5ab117))
	ROM_RELOAD(0x2000, 0x2000)
ROM_END

ROM_START(bitstik2)
	ROM_REGION(0x4000, "exp_rom", 0)
	ROM_LOAD("bitstik2.rom", 0x0000, 0x2000, CRC(a2b5a743) SHA1(4d0040af6bdc6a587e42d4aa36d528b768cf9549))
	ROM_RELOAD(0x2000, 0x2000)
ROM_END

//-------------------------------------------------
//  INPUT_PORTS( bitstik )
//-------------------------------------------------

static INPUT_PORTS_START(bitstik)
	PORT_START("CHANNEL0")
	PORT_BIT(0xff, 0x00, IPT_AD_STICK_X) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(200) PORT_NAME("AD Stick X") PORT_REVERSE

	PORT_START("CHANNEL1")
	PORT_BIT(0xff, 0x00, IPT_AD_STICK_Y) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(200) PORT_NAME("AD Stick Y") PORT_REVERSE

	PORT_START("CHANNEL2")
	PORT_BIT(0xff, 0x00, IPT_AD_STICK_Z) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(200) PORT_NAME("AD Stick Z") PORT_REVERSE

	PORT_START("CHANNEL3")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Right Button - Release")

	PORT_START("BUTTONS")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Top Button - Execute")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("Left Button - Confirm")
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor bbc_bitstik_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(bitstik);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *bbc_bitstik1_device::device_rom_region() const
{
	return ROM_NAME(bitstik1);
}

const tiny_rom_entry *bbc_bitstik2_device::device_rom_region() const
{
	return ROM_NAME(bitstik2);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_bitstik_device - constructor
//-------------------------------------------------

bbc_bitstik_device::bbc_bitstik_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_bbc_analogue_interface(mconfig, *this),
	m_channel(*this, "CHANNEL%u", 0),
	m_buttons(*this, "BUTTONS")
{
}

bbc_bitstik1_device::bbc_bitstik1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	bbc_bitstik_device(mconfig, BBC_BITSTIK1, tag, owner, clock)
{
}

bbc_bitstik2_device::bbc_bitstik2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	bbc_bitstik_device(mconfig, BBC_BITSTIK2, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_bitstik_device::device_start()
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t bbc_bitstik_device::ch_r(int channel)
{
	return m_channel[channel]->read();
}

uint8_t bbc_bitstik_device::pb_r()
{
	return m_buttons->read() & 0x30;
}
