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

ROM_START( coco_pak_rmulticart )
	ROM_REGION(0x100000, CARTSLOT_TAG, ROMREGION_ERASE00)
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
coco_pak_device::coco_pak_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_cococart_interface(mconfig, *this)
	, m_cart(nullptr), m_eprom(*this, CARTSLOT_TAG), m_autostart(*this, CART_AUTOSTART_TAG)
{
}

coco_pak_device::coco_pak_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
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

u32 coco_pak_device::get_cart_size()
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

u8 *coco_pak_device::get_cart_base()
{
	return m_eprom->base();
}

/*-------------------------------------------------
    get_cart_memregion
-------------------------------------------------*/

memory_region *coco_pak_device::get_cart_memregion()
{
	return m_eprom;
}

//-------------------------------------------------
//  cts_read
//-------------------------------------------------

u8 coco_pak_device::cts_read(offs_t offset)
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

coco_pak_banked_device::coco_pak_banked_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: coco_pak_device(mconfig, type, tag, owner, clock)
	, m_pos(0)
{
}
coco_pak_banked_device::coco_pak_banked_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
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

u8 *coco_pak_banked_device::get_cart_base()
{
	return m_eprom->base() + (m_pos * 0x4000) % m_eprom->bytes();
}

//-------------------------------------------------
//  get_cart_size
//-------------------------------------------------

u32 coco_pak_banked_device::get_cart_size()
{
	return 0x4000;
}

//-------------------------------------------------
//  cts_read
//-------------------------------------------------

u8 coco_pak_banked_device::cts_read(offs_t offset)
{
	return m_eprom->base()[(m_pos * 0x4000) % m_eprom->bytes() | offset];
}

//-------------------------------------------------
//  scs_write
//-------------------------------------------------

void coco_pak_banked_device::scs_write(offs_t offset, u8 data)
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

/***************************************************************************
    RAMOTH MULTICART
***************************************************************************/

/*
    The Ramoth Multicart is a banked ROM cartridge holding up to 1MB of ROM.

    The cartridge may contain one or two 32-pin flash ROMs.  These do not
    need to be the same size, but the larger must be the first.  All
    cartridges produced so far use 39SF040s, but any chip with a compatible
    pinout can be used.

    The ROM size is configured by two jumpers on the board:
    RS[1:0]     ROM chips
        00      2x 39SF010 (256K total)
        01      2x 39SF020 (512K total)
        10      2x 39SF040 (1M total)
        11      none (disable?) - not emulated

    The onboard CPLD implements two registers:
    $FF50       Page register (8 bits)
    $FF52       Control register (4 bits)

    The bits of the control register are:
    Bit         Read            Write
    0           Enable autostart if 1
    1           Page size 0=8K, 1=16K
    2           Cart reset, resets CPLD when written to 0 - not emulated
    3           Unused
    4           RS[0]           n/a
    5           RS[1]           n/a
    6           0               n/a
    7           0               n/a
*/

namespace {

constexpr u8 RMULTI_CTRL_AUTO = 0x01;
constexpr u8 RMULTI_CTRL_PAGE16K = 0x02;

} // anonymous namespace

static INPUT_PORTS_START( coco_pak_rmulticart )
	PORT_INCLUDE( coco_cart_autostart )

	PORT_START("jumpers")
	PORT_CONFNAME( 0x03, 0x02, "ROM Size (RS jumpers)" )
	PORT_CONFSETTING(    0x00, "2x 39SF010 (256K)" )
	PORT_CONFSETTING(    0x01, "2x 39SF020 (512K)" )
	PORT_CONFSETTING(    0x02, "2x 39SF040 (1M)" )
INPUT_PORTS_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(COCO_PAK_RMULTICART, coco_pak_rmulticart_device, "cocopak_rmulticart", "Ramoth Dragon/CoCo Multicart")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  coco_pak_rmulticart_device - constructor
//-------------------------------------------------

coco_pak_rmulticart_device::coco_pak_rmulticart_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: coco_pak_device(mconfig, type, tag, owner, clock)
	, m_jumpers(*this, "jumpers")
	, m_page(0)
	, m_ctrl(0)
	, m_rom_mask(0)
{
}

coco_pak_rmulticart_device::coco_pak_rmulticart_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: coco_pak_rmulticart_device(mconfig, COCO_PAK_RMULTICART, tag, owner, clock)
{
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *coco_pak_rmulticart_device::device_rom_region() const
{
	return ROM_NAME( coco_pak_rmulticart );
}

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor coco_pak_rmulticart_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( coco_pak_rmulticart );
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void coco_pak_rmulticart_device::device_start()
{
	coco_pak_device::device_start();

	save_item(NAME(m_page));
	save_item(NAME(m_ctrl));
	save_item(NAME(m_rom_mask));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void coco_pak_rmulticart_device::device_reset()
{
	// ROM size jumpers, from 256K (2x 39SF010) up to 1M (2x 39SF040)
	m_rom_mask = (0x40000 << (m_jumpers->read() & 0x03)) - 1;
	m_page = 0;
	m_ctrl = RMULTI_CTRL_AUTO;

	cart_base_changed();
	update_cart_line();
}

//-------------------------------------------------
//  device_post_load - device-specific post load
//-------------------------------------------------

void coco_pak_rmulticart_device::device_post_load()
{
	cart_base_changed();
}

//-------------------------------------------------
//  page_size
//-------------------------------------------------

u32 coco_pak_rmulticart_device::page_size() const
{
	return (m_ctrl & RMULTI_CTRL_PAGE16K) ? 0x4000 : 0x2000;
}

//-------------------------------------------------
//  page_base - ROM offset of the selected page
//-------------------------------------------------

u32 coco_pak_rmulticart_device::page_base() const
{
	return (m_page * page_size()) & m_rom_mask & (m_eprom->bytes() - 1);
}

//-------------------------------------------------
//  get_cart_base
//-------------------------------------------------

u8 *coco_pak_rmulticart_device::get_cart_base()
{
	return m_eprom->base() + page_base();
}

//-------------------------------------------------
//  get_cart_size
//-------------------------------------------------

u32 coco_pak_rmulticart_device::get_cart_size()
{
	return page_size();
}

//-------------------------------------------------
//  cts_read
//-------------------------------------------------

u8 coco_pak_rmulticart_device::cts_read(offs_t offset)
{
	return m_eprom->base()[page_base() | (offset & (page_size() - 1))];
}

//-------------------------------------------------
//  update_cart_line - set CART line from the
//  control register and auto-start setting
//-------------------------------------------------

void coco_pak_rmulticart_device::update_cart_line()
{
	if (m_cart->exists())
	{
		auto cart_line = (m_autostart.read_safe(0x01) && (m_ctrl & RMULTI_CTRL_AUTO))
			? line_value::Q
			: line_value::CLEAR;

		// normal CoCo PAKs tie their CART line to Q - the system clock
		set_line_value(line::CART, cart_line);
	}
}

//-------------------------------------------------
//  scs_read
//-------------------------------------------------

u8 coco_pak_rmulticart_device::scs_read(offs_t offset)
{
	switch (offset)
	{
		case 0x10: // $FF50
			return m_page;

		case 0x12: // $FF52
			return ((m_jumpers->read() & 0x03) << 4) | (m_ctrl & 0x0f);
	}

	return 0xff;
}

//-------------------------------------------------
//  scs_write
//-------------------------------------------------

void coco_pak_rmulticart_device::scs_write(offs_t offset, u8 data)
{
	switch (offset)
	{
		case 0x10: // $FF50
			if (m_page != data)
			{
				m_page = data;
				cart_base_changed();
			}
			break;

		case 0x12: // $FF52
			data &= 0x0f;
			if (m_ctrl != data)
			{
				m_ctrl = data;
				cart_base_changed();
				update_cart_line();
			}
			break;
	}
}
