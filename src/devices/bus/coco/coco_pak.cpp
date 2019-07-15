// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    coco_pak.cpp

    Code for emulating standard CoCo cartridges

***************************************************************************/

#include "emu.h"
#include "coco_pak.h"

#define CARTSLOT_TAG            "cart"
#define CART_AUTOSTART_TAG      "cart_autostart"


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

ROM_START( coco_pak )
	ROM_REGION(0x8000, CARTSLOT_TAG, ROMREGION_ERASE00)
	// this region is filled by cococart_slot_device::call_load()
ROM_END

ROM_START( coco_pak_banked )
	ROM_REGION(0x20000, CARTSLOT_TAG, ROMREGION_ERASE00)
	// this region is filled by cococart_slot_device::call_load()
ROM_END


//-------------------------------------------------
//  INPUT_PORTS( coco_cart_autostart )
//-------------------------------------------------

static INPUT_PORTS_START( coco_cart_autostart )
	PORT_START(CART_AUTOSTART_TAG)
	PORT_CONFNAME( 0x01, 0x01, "Cart Auto-Start" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ))
	PORT_CONFSETTING(    0x01, DEF_STR( On ))
INPUT_PORTS_END


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(COCO_PAK, coco_pak_device, "cocopak", "CoCo Program PAK")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  coco_pak_device - constructor
//-------------------------------------------------
coco_pak_device::coco_pak_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_cococart_interface(mconfig, *this)
	, m_cart(nullptr), m_eprom(*this, CARTSLOT_TAG), m_autostart(*this, CART_AUTOSTART_TAG)
{
}

coco_pak_device::coco_pak_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: coco_pak_device(mconfig, COCO_PAK, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void coco_pak_device::device_start()
{
	m_cart = dynamic_cast<device_image_interface *>(owner());
}


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor coco_pak_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( coco_cart_autostart );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *coco_pak_device::device_rom_region() const
{
	return ROM_NAME( coco_pak );
}

//-------------------------------------------------
//  get_cart_size
//-------------------------------------------------

uint32_t coco_pak_device::get_cart_size()
{
	return 0x8000;
}

/*-------------------------------------------------
    device_reset - device-specific startup
-------------------------------------------------*/

void coco_pak_device::device_reset()
{
	if (m_cart->exists())
	{
		auto cart_line = m_autostart.read_safe(0x01)
			? line_value::Q
			: line_value::CLEAR;

		// normal CoCo PAKs tie their CART line to Q - the system clock
		set_line_value(line::CART, cart_line);
	}
}

/*-------------------------------------------------
    get_cart_base
-------------------------------------------------*/

uint8_t* coco_pak_device::get_cart_base()
{
	return m_eprom->base();
}

/*-------------------------------------------------
    get_cart_memregion
-------------------------------------------------*/

memory_region* coco_pak_device::get_cart_memregion()
{
	return m_eprom;
}

//-------------------------------------------------
//  cts_read
//-------------------------------------------------

READ8_MEMBER(coco_pak_device::cts_read)
{
	if (offset < m_eprom->bytes())
		return m_eprom->base()[offset];
	else
		return 0x00;
}



/***************************************************************************
    BANKED CARTRIDGES
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(COCO_PAK_BANKED, coco_pak_banked_device, "cocopak_banked", "CoCo Program PAK (Banked)")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  coco_pak_device - constructor
//-------------------------------------------------

coco_pak_banked_device::coco_pak_banked_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: coco_pak_device(mconfig, type, tag, owner, clock)
	, m_pos(0)
{
}
coco_pak_banked_device::coco_pak_banked_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: coco_pak_banked_device(mconfig, COCO_PAK_BANKED, tag, owner, clock)
{
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *coco_pak_banked_device::device_rom_region() const
{
	return ROM_NAME( coco_pak_banked );
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void coco_pak_banked_device::device_start()
{
	coco_pak_device::device_start();

	save_item(NAME(m_pos));
}

//-------------------------------------------------
//  device_reset - device-specific startup
//-------------------------------------------------

void coco_pak_banked_device::device_reset()
{
	coco_pak_device::device_reset();

	m_pos = 0;
}

//-------------------------------------------------
//  get_cart_base
//-------------------------------------------------

uint8_t *coco_pak_banked_device::get_cart_base()
{
	return m_eprom->base() + (m_pos * 0x4000) % m_eprom->bytes();
}

//-------------------------------------------------
//  get_cart_size
//-------------------------------------------------

uint32_t coco_pak_banked_device::get_cart_size()
{
	return 0x4000;
}

//-------------------------------------------------
//  cts_read
//-------------------------------------------------

READ8_MEMBER(coco_pak_banked_device::cts_read)
{
	return m_eprom->base()[(m_pos * 0x4000) % m_eprom->bytes() | offset];
}

//-------------------------------------------------
//  scs_write
//-------------------------------------------------

WRITE8_MEMBER(coco_pak_banked_device::scs_write)
{
	switch(offset)
	{
		case 0:
			// set the bank
			if (m_pos != data)
			{
				m_pos = data;
				cart_base_changed();
			}
			break;
	}
}
