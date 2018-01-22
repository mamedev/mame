// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    ZX Interface 2

    The Sinclair ZX Interface 2 allows you to connect two joysticks
    and a ROM software cartridge to your ZX Spectrum.

**********************************************************************/


#include "emu.h"
#include "intf2.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SPECTRUM_INTF2, spectrum_intf2_device, "spectrum_intf2", "ZX Interface 2")


//-------------------------------------------------
//  INPUT_PORTS( intf2 )
//-------------------------------------------------

static INPUT_PORTS_START( intf2 )
	PORT_START("LINE3") /* 0xF7FE */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_8WAY PORT_PLAYER(2) PORT_CODE(JOYCODE_X_LEFT_SWITCH)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY PORT_PLAYER(2) PORT_CODE(JOYCODE_X_RIGHT_SWITCH)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  PORT_8WAY PORT_PLAYER(2) PORT_CODE(JOYCODE_Y_DOWN_SWITCH)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    PORT_8WAY PORT_PLAYER(2) PORT_CODE(JOYCODE_Y_UP_SWITCH)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1)        PORT_PLAYER(2) PORT_CODE(JOYCODE_BUTTON1)

	PORT_START("LINE4") /* 0xEFFE */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1)        PORT_PLAYER(1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_8WAY PORT_PLAYER(1)
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor spectrum_intf2_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( intf2 );
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(spectrum_intf2_device::device_add_mconfig)
	/* cartridge */
	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "spectrum_cart")
	MCFG_GENERIC_EXTENSIONS("bin,rom")
	MCFG_GENERIC_LOAD(spectrum_intf2_device, spectrum_cart)

	MCFG_SOFTWARE_LIST_ADD("cart_list", "spectrum_cart")
MACHINE_CONFIG_END

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  spectrum_intf2_device - constructor
//-------------------------------------------------

spectrum_intf2_device::spectrum_intf2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SPECTRUM_INTF2, tag, owner, clock),
		device_spectrum_expansion_interface(mconfig, *this),
	m_cart(*this, "cartslot"),
	m_exp_line3(*this, "LINE3"),
	m_exp_line4(*this, "LINE4")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void spectrum_intf2_device::device_start()
{
	m_slot = dynamic_cast<spectrum_expansion_slot_device *>(owner());
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void spectrum_intf2_device::device_reset()
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

DEVICE_IMAGE_LOAD_MEMBER(spectrum_intf2_device, spectrum_cart)
{
	uint32_t size = m_cart->common_get_size("rom");

	if (size != 0x4000)
	{
		image.seterror(IMAGE_ERROR_UNSPECIFIED, "Unsupported cartridge size");
		return image_init_result::FAIL;
	}

	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return image_init_result::PASS;
}

READ_LINE_MEMBER(spectrum_intf2_device::romcs)
{
	if (m_cart && m_cart->exists())
		return 1;
	else
		return 0;
}

READ8_MEMBER(spectrum_intf2_device::mreq_r)
{
	if (m_cart && m_cart->exists())
		return m_cart->get_rom_base()[offset & 0x3fff];
	else
		return 0xff;
}

READ8_MEMBER(spectrum_intf2_device::port_fe_r)
{
	uint8_t data = 0xff;

	uint8_t lines = offset >> 8;

	if ((lines & 8) == 0)
		data = m_exp_line3->read() | (0xff ^ 0x1f);

	if ((lines & 16) == 0)
		data = m_exp_line4->read() | (0xff ^ 0x1f);

	return data;
}
