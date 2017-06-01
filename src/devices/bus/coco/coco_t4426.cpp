// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/***************************************************************************
 *
 *  coco_t4426.cpp
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
#include "cococart.h"
#include "machine/6850acia.h"
#include "machine/6821pia.h"


#define LOG_GENERAL 0x01
#define LOG_SETUP   0x02
#define LOG_PRINTF  0x04
#define LOG_PIA     0x08

#define VERBOSE 0 //(LOG_PIA | LOG_PRINTF | LOG_SETUP  | LOG_GENERAL)

#define LOGMASK(mask, ...)   do { if (VERBOSE & mask) logerror(__VA_ARGS__); } while (0)
#define LOGLEVEL(mask, level, ...) do { if ((VERBOSE & mask) >= level) logerror(__VA_ARGS__); } while (0)

#define LOG(...)      LOGMASK(LOG_GENERAL, __VA_ARGS__)
#define LOGSETUP(...) LOGMASK(LOG_SETUP,   __VA_ARGS__)
#define LOGPIA(...)   LOGMASK(LOG_PIA,     __VA_ARGS__)

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
#define CARTSLOT_TAG    "t4426"
#define CART_AUTOSTART_TAG      "cart_autostart"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

namespace
{
	// ======================> coco_t4426_device

	class coco_t4426_device :
		public device_t,
		public device_cococart_interface
	{
	public:
		// construction/destruction
		coco_t4426_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

		// optional information overrides
		virtual void device_add_mconfig(machine_config &config) override;
		virtual const tiny_rom_entry *device_rom_region() const override;
		virtual ioport_constructor device_input_ports() const override;

		virtual uint8_t* get_cart_base() override;
		DECLARE_WRITE8_MEMBER(pia_A_w);

	protected:
		coco_t4426_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

		// device-level overrides
		virtual void device_start() override;
		virtual void device_reset() override;

		// internal state
		device_image_interface *m_cart;
		cococart_slot_device *m_owner;
		uint8_t m_select;

		optional_ioport m_autostart;

		virtual DECLARE_READ8_MEMBER(scs_read) override;
		virtual DECLARE_WRITE8_MEMBER(scs_write) override;
	private:
		// internal state
		required_device<acia6850_device> m_uart;
		required_device<pia6821_device> m_pia;
	};
};


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

MACHINE_CONFIG_MEMBER(coco_t4426_device::device_add_mconfig)
	MCFG_DEVICE_ADD(UART_TAG, ACIA6850, 0) // TODO: Figure out address mapping for ACIA
	MCFG_DEVICE_ADD(PIA_TAG, PIA6821, 0)
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(coco_t4426_device, pia_A_w))
MACHINE_CONFIG_END

ROM_START( coco_t4426 )
	ROM_REGION(0x1a000, CARTSLOT_TAG, ROMREGION_ERASE00)

	// 8 banked ROM:s TODO: Add the banking and the other ROM:s
	ROM_LOAD("tercoED4426-0-8549-5.3.bin",  0x00000, 0x2000, CRC(45665428) SHA1(ff49a79275772c4c4ab1ae29db662c9b10a744a7))
	ROM_LOAD("tercoED4426-1-8549-5.3.bin",  0x03000, 0x2000, CRC(44baba33) SHA1(01cee1b208c158e598e7ecd2189b5e0ffa7f3ab9))
	ROM_LOAD("tercoPD4426-2-8632-6.4.bin",  0x06000, 0x2000, CRC(258e443a) SHA1(9d8901f3e70ae4f8526dde1b5208b22f066f801f))
	ROM_LOAD("tercoPD4426-3-8638-6.4.bin",  0x09000, 0x2000, CRC(640d1de4) SHA1(5ae7427cb5729fd3920361855d954ea1f97f6ae5))
	ROM_LOAD("tercoCA4426-4-8549-3.4.bin",  0x0c000, 0x2000, CRC(df18397b) SHA1(2f9de210c039619c649be223c37a4eff873fa600))
	ROM_LOAD("tercoCA4426-5-8549-3.4.bin",  0x0f000, 0x2000, CRC(3fcdf92e) SHA1(ec1589f8d62701ca3faf2a85a57ab2e1f61d2137))
	ROM_LOAD("tercoCA4426-6-8549-3.4.bin",  0x13000, 0x2000, CRC(27652ccf) SHA1(529a6f736666ae6660483e547d076725606b1c1d))
	ROM_LOAD("tercoCA4426-7-8549-3.4.bin",  0x16000, 0x2000, CRC(f6640569) SHA1(03f70dcc5f7ab60cd908427d45ebd85b6f464b93))
	// Main cartridge ROM
	ROM_LOAD("tercoPMOS4426-8549-4.31.bin", 0x2000, 0x1000, CRC(bc65c45c) SHA1(e50cfd1d61e29fe05eb795d8bf6303e7b91ed8e5))
	// Interleaved copies for the fixed ROM for banking to work (nw yet though)
	ROM_RELOAD(0x05000,0x1000)
	ROM_RELOAD(0x08000,0x1000)
	ROM_RELOAD(0x0b000,0x1000)
	ROM_RELOAD(0x0e000,0x1000)
	ROM_RELOAD(0x12000,0x1000)
	ROM_RELOAD(0x15000,0x1000)
	ROM_RELOAD(0x18000,0x1000)
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

DEFINE_DEVICE_TYPE(COCO_T4426, coco_t4426_device, "coco_t4426", "Terco CNC Programming Station 4426 multi cart")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  coco_t4426_device - constructor
//-------------------------------------------------

coco_t4426_device::coco_t4426_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_cococart_interface(mconfig, *this)
	, m_cart(nullptr)
	, m_owner(nullptr)
	, m_select(0)
	, m_autostart(*this, CART_AUTOSTART_TAG)
	, m_uart(*this, UART_TAG)
	, m_pia(*this, PIA_TAG)
{
}

coco_t4426_device::coco_t4426_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: coco_t4426_device(mconfig, COCO_T4426, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void coco_t4426_device::device_start()
{
	LOG("%s()\n", FUNCNAME );
	m_cart = dynamic_cast<device_image_interface *>(owner());
	m_owner = dynamic_cast<cococart_slot_device *>(owner());
}


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *coco_t4426_device::device_rom_region() const
{
	LOG("%s()\n", FUNCNAME );
	return ROM_NAME( coco_t4426 );
}

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor coco_t4426_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( coco_cart_autostart );
}


/*-------------------------------------------------
    device_reset - device-specific startup
-------------------------------------------------*/

void coco_t4426_device::device_reset()
{
	LOG("%s()\n", FUNCNAME );
	auto cart_line = cococart_slot_device::line_value::Q;
	m_owner->set_line_value(cococart_slot_device::line::CART, cart_line);
}

/*-------------------------------------------------
    scs_read
 The 4426 cartridge PIA is located at ff44-ff47
-------------------------------------------------*/

READ8_MEMBER(coco_t4426_device::scs_read)
{
	uint8_t result = 0x00;

	LOG("%s", FUNCNAME);

	if ((offset >= 0x04) && (offset <= 0x07))
		result = m_pia->read(space, offset - 0x04);

	LOG(" - Offs:%04x Data:%02x\n", offset, result);
	return result;
}

/*-------------------------------------------------
    scs_write
 The 4426 cartridge PIA is located at ff44-ff47
-------------------------------------------------*/

WRITE8_MEMBER(coco_t4426_device::scs_write)
{
	LOG("%s(%02x)\n", FUNCNAME, data);
	LOGSETUP(" * Offs:%02x <- %02x\n", offset, data);

	if ((offset >= 0x04) && (offset <= 0x07))
		m_pia->write(space, offset - 0x04, data);
}

/*----------------------------------------------------
    pia_A_w - PIA port A write

 The T4426 cartridge PIA Port A is connected to
 the CE* input of each 2764 ROM and used for banking
 in the correct BASIC module at C000-DFFF
 The main cartridge ROM at E000-FF00 is fixed however
-----------------------------------------------------*/
#define ROM0 (~0x01 & 0xff)
#define ROM1 (~0x02 & 0xff)
#define ROM2 (~0x04 & 0xff)
#define ROM3 (~0x08 & 0xff)
#define ROM4 (~0x10 & 0xff)
#define ROM5 (~0x20 & 0xff)
#define ROM6 (~0x40 & 0xff)
#define ROM7 (~0x80 & 0xff)

WRITE8_MEMBER( coco_t4426_device::pia_A_w )
{
	LOGPIA("%s(%02x)\n", FUNCNAME, data);
	m_select = data;
	cart_base_changed();
}

/*-------------------------------------------------
    get_cart_base
-------------------------------------------------*/

uint8_t* coco_t4426_device::get_cart_base()
{
	LOG("%s - m_select %02x -> %02x\n", FUNCNAME, m_select, ~m_select & 0xff );
	uint8_t *base = memregion(CARTSLOT_TAG)->base();

	switch (m_select)
	{
	case 0:
	case ROM0: base = (uint8_t *) (base + 0x00000); break;
	case ROM1: base = (uint8_t *) (base + 0x03000); break;
	case ROM2: base = (uint8_t *) (base + 0x06000); break;
	case ROM3: base = (uint8_t *) (base + 0x09000); break;
	case ROM4: base = (uint8_t *) (base + 0x0c000); break;
	case ROM5: base = (uint8_t *) (base + 0x0f000); break;
	case ROM6: base = (uint8_t *) (base + 0x13000); break;
	case ROM7: base = (uint8_t *) (base + 0x16000); break;
	}

#if 0 // Print the beginning of the selected ROM bank, seems to be correct but is not mapped in correctly via coco12.cpp m_sam->read yet
	printf("\n");
	for (int i = 0; i < 48; i++) printf("%02x ", *((uint8_t *)(base + i)));
	printf("\n");
	for (int i = 0; i < 48; i++) printf("'%c'",  *((uint8_t *)(base + i)));
	printf("\n");
#endif

	return base;
}
