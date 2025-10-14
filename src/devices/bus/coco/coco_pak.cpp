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
    Ramoth Multicart 
***************************************************************************/

/*
	The Ramoth Multicart, is a banked ROM cartridge of up to 1MB of ROM.
	
	The cartridge may contain 1 or 2 32PIN Flash roms, these do not need to be the 
	same size but the Larger must be the first. All cartridges so far produced
	use 39SF040, but any compatible pinout chips can be used.
	
	The ROM size is configured by two jumpers on the board :
	RS[1:0]		Rom size
		00		2x39010, 128K
		01		2x39020, 256K
		10		2x39040, 512K
		11		None, disable?

	These are returned as bits 5 and 4 of the CTRL register.
	
	The onboard CPLD implements 2 registers :
	$FF50		Page register (8 bits).
	$FF52		CTRL register (4 bits).
	
	The bits of the CTRL register are :
	Bit			Read		Write
	0			Enable autostart if 1
	1			Page size 0=8K, 1=16K
	2			Cart reset, resets CPLD when written to 0
	3			Unused.
	4			RS[0]		n/a
	5			RS[1]		n/a
	6			0			n/a
	7			0			n/a
	
*/

#define RMULTI_AUTO		0x01
#define RMULTI_SIZE		0x02
#define RMULTI_RESET	0x04

#define RMULTI_PAGE_MASK8	0x00
#define RMULTI_PAGE_MASK16	0x01

#define RMULTI_RS		0b10


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(COCO_PAK_RMULTICART, coco_pak_rmulticart_device, "cocopak_rmulticart", "Ramoth Dragon/CoCo Multicart")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  coco_pak_device - constructor
//-------------------------------------------------

coco_pak_rmulticart_device::coco_pak_rmulticart_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: coco_pak_device(mconfig, type, tag, owner, clock)
	//, m_pos(0)
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
//  device_start - device-specific startup
//-------------------------------------------------

void coco_pak_rmulticart_device::device_start()
{
	coco_pak_device::device_start();

	save_item(NAME(Page));
	save_item(NAME(CTRL));
	save_item(NAME(PageSize));
	save_item(NAME(PageMask));
}

//-------------------------------------------------
//  device_reset - device-specific startup
//-------------------------------------------------

void coco_pak_rmulticart_device::device_reset()
{
	Page = 0;
	CTRL = RMULTI_AUTO;
	PageSize = 0x2000;
	PageMask = RMULTI_PAGE_MASK8;

	SetCART();
}

//-------------------------------------------------
//  get_cart_base
//-------------------------------------------------

u8 *coco_pak_rmulticart_device::get_cart_base()
{
	return m_eprom->base() + (Page * PageSize);
}

//-------------------------------------------------
//  get_cart_size
//-------------------------------------------------

u32 coco_pak_rmulticart_device::get_cart_size()
{
	return PageSize;
}

//-------------------------------------------------
//  cts_read
//-------------------------------------------------

u8 coco_pak_rmulticart_device::cts_read(offs_t offset)
{
	return m_eprom->base()[(Page * PageSize) + (offset % PageSize)];
}

// Set CART line dependent on CTRL reg & Autostart input.
void coco_pak_rmulticart_device::SetCART()
{
	if (m_cart->exists())
	{
		auto cart_line = (m_autostart.read_safe(0x01) && (CTRL & RMULTI_AUTO))
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
	switch(offset)
	{
		// $FF50
		case 0x10:
			logerror("Read Page: %02X\n",Page);
			return Page;
			break;
		
		// $FF52
		case 0x12:
			logerror("Read CTRL: %02X\n",CTRL);
			return (RMULTI_RS << 4) | (CTRL & 0x0F);
			break;
			
		default:
			return 0xFF;
			break;
	}
}

//-------------------------------------------------
//  scs_write
//-------------------------------------------------

void coco_pak_rmulticart_device::scs_write(offs_t offset, u8 data)
{
	switch(offset)
	{
		// $FF50
		case 0x10:
			logerror("Write Page: %02X\n",data);
			// set the bank
			if (Page != data)
			{
				Page = data;
				cart_base_changed();
			}
			break;
		
		// $FF52
		case 0x12:
			logerror("Write CTRL: %02X\n",data);
			if (CTRL != data)
			{
				CTRL = data;
				PageSize = (CTRL & RMULTI_SIZE) ? 0x4000 : 0x2000;
				PageMask = (CTRL & RMULTI_SIZE) ? RMULTI_PAGE_MASK16 : RMULTI_PAGE_MASK8;
				cart_base_changed();
				SetCART();
			}
			break;
	}
}
