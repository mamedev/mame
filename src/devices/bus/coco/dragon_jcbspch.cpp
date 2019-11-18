// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    JCB Speech Synthesis Module

    http://archive.worldofdragon.org/index.php?title=Dragon_Speech_Synthesis

    Speech synthesiser cartridge made by J.C.B. (Microsystems). It is based
    on the General Instruments SP0256-AL2 speech synthesiser. The cartridge
    provides extensions to BASIC for producing speech.

    TODO: verify everything

***************************************************************************/

#include "emu.h"
#include "dragon_jcbspch.h"
#include "speaker.h"


ROM_START(dragon_jcbspch)
	ROM_REGION(0x10000, "sp0256", 0)
	ROM_LOAD("sp0256-al2.bin", 0x1000, 0x0800, CRC(b504ac15) SHA1(e60fcb5fa16ff3f3b69d36c7a6e955744d3feafc))

	ROM_REGION(0x1000, "eprom", 0)
	ROM_LOAD("cb-speech.rom", 0x0000, 0x1000, CRC(e88dfe36) SHA1(df3f64a7a3beeb91469932035af5e4f8a7872aad))
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(DRAGON_JCBSPCH, dragon_jcbspch_device, "dragon_jcbspch", "Dragon Speech Synthesis Module")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  dragon_jcbspch_device - constructor
//-------------------------------------------------

dragon_jcbspch_device::dragon_jcbspch_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DRAGON_JCBSPCH, tag, owner, clock)
	, device_cococart_interface(mconfig, *this )
	, m_eprom(*this, "eprom")
	, m_pia(*this, "pia")
	, m_nsp(*this, "sp0256")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dragon_jcbspch_device::device_start()
{
	set_line_value(line::CART, line_value::Q);
}

//-------------------------------------------------
//  dragon_jcbspch_device::get_cart_base
//-------------------------------------------------

uint8_t* dragon_jcbspch_device::get_cart_base()
{
	return m_eprom->base();
}

//-------------------------------------------------
//  dragon_jcbspch_device::get_cart_memregion
//-------------------------------------------------

memory_region* dragon_jcbspch_device::get_cart_memregion()
{
	return m_eprom;
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void dragon_jcbspch_device::device_add_mconfig(machine_config &config)
{
	PIA6821(config, m_pia, 0);
	m_pia->writepb_handler().set(m_nsp, FUNC(sp0256_device::ald_w)).mask(0x3f);
	m_pia->cb2_handler().set(FUNC(dragon_jcbspch_device::pia_cb2_w));
	m_pia->irqb_handler().set(FUNC(dragon_jcbspch_device::nmi_w));

	SPEAKER(config, "mono").front_center();
	SP0256(config, m_nsp, 3.2768_MHz_XTAL);
	m_nsp->standby_callback().set(m_pia, FUNC(pia6821_device::cb1_w));
	m_nsp->add_route(ALL_OUTPUTS, "mono", 1.00);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *dragon_jcbspch_device::device_rom_region() const
{
	return ROM_NAME( dragon_jcbspch );
}

//-------------------------------------------------
//  cts_read
//-------------------------------------------------

READ8_MEMBER(dragon_jcbspch_device::cts_read)
{
	return m_eprom->base()[offset & 0x0fff];
}

//-------------------------------------------------
//  scs_read
//-------------------------------------------------

READ8_MEMBER(dragon_jcbspch_device::scs_read)
{
	uint8_t result = 0x00;

	switch (offset)
	{
	case 0: case 1: case 2: case 3:
		result = m_pia->read(offset);
		break;
	}
	return result;
}

//-------------------------------------------------
//  scs_write
//-------------------------------------------------

WRITE8_MEMBER(dragon_jcbspch_device::scs_write)
{
	switch (offset)
	{
	case 0: case 1: case 2: case 3:
		m_pia->write(offset, data);
		break;
	}
}

WRITE_LINE_MEMBER(dragon_jcbspch_device::pia_cb2_w)
{
	// TODO: what does this do?
}

WRITE_LINE_MEMBER(dragon_jcbspch_device::nmi_w)
{
	// set the NMI line
	set_line_value(line::NMI, state);
}
