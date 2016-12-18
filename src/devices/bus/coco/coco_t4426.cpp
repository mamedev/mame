// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/***************************************************************************
 *
 *  coco_t4426.c
 *
 *  Terco T4426 CNC Programming Station multi cart
 *
 *  The code here is heavily inspired by coco_pak and coco_232
 *
 *  +-------------------------------------------------------------------------------+
 *  ||__|+-----+    oo   75                               |O ||||||||||||||| O|     |
 *  |XTL||MC   |    oo  110                                                         |
 *  |1.8||14411|    oo  ..        +--+ +--+                                         |
 *  |432||BRG  |    ==  600       |74| |74| +------+                                |
 *  |MHz||     |    oo  ..        |LS| |LS| |MC1488|                                |
 *  +---+|     |    oo 7200       |139 |00| +------+                                |
 *  |    +-----+    oo 9600       +--+ +--+                    +--+                 |
 *  |                                      +-------------+     |MC|                 |
 *  |    +-----+   +-----+    +-----+      | EF68B50P    |     |14|                 |
 *  |    | 2764|   | 2764|    |     |      | ACIA        |     |89|                 |
 *  |    |     |   |     |    |     |      +-------------+     +--+                 |
 *  |    |CA   |   |CA   |    |PMOS |                                               |
 *  |    | 4426|   | 4426|    | 4426|   +-------------------+                       |
 *  |    |  -6 |   |  -7 |    |     |   |  EP68B21P         |                       |
 *  |    |     |   |     |    |     |   |  PIA              |                       |
 *  |    |     |   |     |    +-----+   +-------------------+                       |
 *  |    +-----+   +-----+                                                          |
 *  |    +-----+   +-----+    +-----+   +-----+    +-----+   +-----+                |
 *  |    | 2764|   | 2764|    | 2764|   | 2764|    | 2764|   | 2764|                |
 *  |    |     |   |     |    |     |   |     |    |     |   |     |                |
 *  |    |CA   |   |CA   |    |PD   |   |PD   |    |ED   |   |ED   |                |
 *  |    | 4426|   | 4426|    | 4426|   | 4426|    | 4426|   | 4426|                |
 *  |    |  -5 |   |  -4 |    |  -3 |   |  -2 |    |  -1 |   |  -0 |                |
 *  |    |     |   |     |    |     |   |     |    |     |   |     |       OO       |
 *  |    |     |   |     |    |     |   |     |    |     |   |     |                |
 *  |    +-----+   +-----+    +-----+   +-----+    +-----+   +-----+                |
 *  +-------------------------------------------------------------------------------+
 *
 ***************************************************************************/

#include "emu.h"
#include "coco_t4426.h"
#include "includes/coco.h"

#define LOG_GENERAL 0x01
#define LOG_SETUP   0x02
#define LOG_PRINTF  0x04

#define VERBOSE 0 // (LOG_PRINTF | LOG_SETUP  | LOG_GENERAL)

#define LOGMASK(mask, ...)   do { if (VERBOSE & mask) logerror(__VA_ARGS__); } while (0)
#define LOGLEVEL(mask, level, ...) do { if ((VERBOSE & mask) >= level) logerror(__VA_ARGS__); } while (0)

#define LOG(...)      LOGMASK(LOG_GENERAL, __VA_ARGS__)
#define LOGSETUP(...) LOGMASK(LOG_SETUP,   __VA_ARGS__)

#if VERBOSE & LOG_PRINTF
#define logerror printf
#endif

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

/***************************************************************************
    CONSTANTS
***************************************************************************/

#define UART_TAG        "acia"
#define PIA_TAG         "pia"
#define CARTSLOT_TAG	"t4426"

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

// TODO: Figure out address mapping for these devices
static MACHINE_CONFIG_FRAGMENT(coco_t4426)
	MCFG_DEVICE_ADD(UART_TAG, ACIA6850, 0)
	MCFG_DEVICE_ADD(PIA_TAG, PIA6821, 0)
MACHINE_CONFIG_END

ROM_START( coco_t4426 )
	ROM_REGION(0x8000, CARTSLOT_TAG, ROMREGION_ERASE00)
	// First of 8 banked ROM:s TODO: Add the banking and the other ROM:s 
	ROM_LOAD("tercoED4426-0-8549-5.3.bin",  0x0000, 0x2000, CRC(45665428) SHA1(ff49a79275772c4c4ab1ae29db662c9b10a744a7)) 

	// Main cartridge ROM
	ROM_LOAD("tercoPMOS4426-8549-4.31.bin", 0x2000, 0x1000, CRC(bc65c45c) SHA1(e50cfd1d61e29fe05eb795d8bf6303e7b91ed8e5))
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type COCO_T4426 = &device_creator<coco_t4426_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  coco_t4426_device - constructor
//-------------------------------------------------

coco_t4426_device::coco_t4426_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source)
	,device_cococart_interface( mconfig, *this )
	,m_cart(nullptr)
	,m_owner(nullptr)
	,m_autostart(*this, ":" CART_AUTOSTART_TAG)
	,m_uart(*this, UART_TAG)
	,m_pia(*this, PIA_TAG)
{
}

coco_t4426_device::coco_t4426_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, COCO_T4426, "Terco CNC Programming Station 4426 multi cart", tag, owner, clock, "coco_t4426", __FILE__)
	,device_cococart_interface( mconfig, *this )
	,m_cart(nullptr)
	,m_owner(nullptr)
	,m_autostart(*this, ":" CART_AUTOSTART_TAG)
	,m_uart(*this, UART_TAG)
	,m_pia(*this, PIA_TAG)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void coco_t4426_device::device_start()
{
	m_cart = dynamic_cast<device_image_interface *>(owner());
	m_owner = dynamic_cast<cococart_slot_device *>(owner());
}

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor coco_t4426_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( coco_t4426 );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *coco_t4426_device::device_rom_region() const
{
	return ROM_NAME( coco_t4426 );
}

/*-------------------------------------------------
    device_reset - device-specific startup
-------------------------------------------------*/

void coco_t4426_device::device_reset()
{
	auto cart_line = cococart_slot_device::line_value::Q;
	m_owner->cart_set_line(cococart_slot_device::line::CART, cart_line);
}

/*-------------------------------------------------
    get_cart_base
-------------------------------------------------*/

uint8_t* coco_t4426_device::get_cart_base()
{
	return memregion(CARTSLOT_TAG)->base();
}

/*-------------------------------------------------
    read
-------------------------------------------------*/

READ8_MEMBER(coco_t4426_device::read)
{
	uint8_t result = 0x00;

	LOG("%s()\n", FUNCNAME);
	LOGSETUP(" * Offs:%02x -> %02x\n", offset, result);

	return result;
}

/*-------------------------------------------------
    write
-------------------------------------------------*/

WRITE8_MEMBER(coco_t4426_device::write)
{
	LOG("%s(%02x)\n", FUNCNAME, data);
	LOGSETUP(" * Offs:%02x <- %02x\n", offset, data);
}
