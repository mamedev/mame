// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/***************************************************************************
 *
 *  coco_t4426.cpp
 *
 *  Terco T4426 CNC Programming Station multi cart
 *
 *  The T4426 is based on a Color Computer II PCB revision 26-3134A with a modified
 *  Extended Basic fitted through an adapter and a soldered wire as one address line
 *  was missing. Modifications involved obfuscation changing names of some common
 *  commands such as LIST renamed to LARS (which happens to be the name of the CEO).
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
 *  |    | 2764|   | 2764|    | 2732|      | ACIA        |     |89|                 |
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

#include "machine/6850acia.h"
#include "bus/rs232/rs232.h"
#include "machine/mc14411.h"
#include "machine/6821pia.h"

#define LOG_SETUP   (1U << 1)
#define LOG_PIA     (1U << 2)
#define LOG_ACIA    (1U << 3)

//#define VERBOSE (LOG_PIA) // (LOG_PIA | LOG_GENERAL | LOG_SETUP)
//#define LOG_OUTPUT_STREAM std::cout
#include "logmacro.h"

#define LOGSETUP(...)   LOGMASKED(LOG_SETUP,   __VA_ARGS__)
#define LOGPIA(...) LOGMASKED(LOG_PIA,     __VA_ARGS__)
#define LOGACIA(...)    LOGMASKED(LOG_ACIA,    __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

/***************************************************************************
    CONSTANTS
***************************************************************************/

#define UART_TAG            "acia"
#define SERIAL_TAG      "ser2" // Labled "Ser.I/O2 RC 232C" on the back of the case
#define BRG_TAG         "brg"
#define SERIAL_BRF      "serial_brf"
#define SERIAL_BAUD     "serial_baud"
#define PIA_TAG             "pia"
#define CARTSLOT_TAG        "t4426"
#define CARTBANK_TAG        "t4426_banks"
#define CART_AUTOSTART_TAG  "cart_autostart"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

namespace
{
	class coco_t4426_device :
		public device_t,
		public device_cococart_interface
	{
	public:
		// construction/destruction
		coco_t4426_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

		// optional information overrides
		virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
		virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
		virtual ioport_constructor device_input_ports() const override ATTR_COLD;

		virtual u8 *get_cart_base() override;
		void pia_A_w(u8 data);

		// Clocks
		void write_acia_clocks(int id, int state);
		void write_f1_clock(int state) { write_acia_clocks(mc14411_device::TIMER_F1, state); }
		void write_f3_clock(int state) { write_acia_clocks(mc14411_device::TIMER_F3, state); }
		void write_f5_clock(int state) { write_acia_clocks(mc14411_device::TIMER_F5, state); }
		void write_f7_clock(int state) { write_acia_clocks(mc14411_device::TIMER_F7, state); }
		void write_f8_clock(int state) { write_acia_clocks(mc14411_device::TIMER_F8, state); }
		void write_f9_clock(int state) { write_acia_clocks(mc14411_device::TIMER_F9, state); }
		void write_f11_clock(int state) { write_acia_clocks(mc14411_device::TIMER_F11, state); }
		void write_f13_clock(int state) { write_acia_clocks(mc14411_device::TIMER_F13, state); }
		void write_f15_clock(int state) { write_acia_clocks(mc14411_device::TIMER_F15, state); }
	protected:
		coco_t4426_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

		// device-level overrides
		virtual void device_start() override ATTR_COLD;
		virtual void device_reset() override ATTR_COLD;

		// internal state
		device_image_interface *m_cart;
		u8 m_select;

		optional_ioport m_autostart;

		virtual u8 cts_read(offs_t offset) override;
		virtual u8 scs_read(offs_t offset) override;
		virtual void scs_write(offs_t offset, u8 data) override;
	private:
		// internal state
		required_memory_region m_eprom;
		required_memory_region m_eprom_banked;
		required_device<acia6850_device> m_uart;
		required_device<pia6821_device> m_pia;
		required_device<mc14411_device> m_brg;

		required_ioport             m_serial_baud;
	};
};


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

void coco_t4426_device::device_add_mconfig(machine_config &config)
{
	PIA6821(config, m_pia);
	m_pia->writepa_handler().set(FUNC(coco_t4426_device::pia_A_w));

	ACIA6850(config, m_uart, 0);
	m_uart->txd_handler().set(SERIAL_TAG, FUNC(rs232_port_device::write_txd));
	m_uart->rts_handler().set(SERIAL_TAG, FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232(RS232_PORT(config, SERIAL_TAG, default_rs232_devices, nullptr));
	rs232.rxd_handler().set(UART_TAG, FUNC(acia6850_device::write_rxd));
	rs232.cts_handler().set(UART_TAG, FUNC(acia6850_device::write_cts));

	/* Bit Rate Generator */
	MC14411(config, m_brg, 1.8432_MHz_XTAL);
	m_brg->out_f<1>().set(FUNC(coco_t4426_device::write_f1_clock));
	m_brg->out_f<3>().set(FUNC(coco_t4426_device::write_f3_clock));
	m_brg->out_f<5>().set(FUNC(coco_t4426_device::write_f5_clock));
	m_brg->out_f<7>().set(FUNC(coco_t4426_device::write_f7_clock));
	m_brg->out_f<8>().set(FUNC(coco_t4426_device::write_f8_clock));
	m_brg->out_f<9>().set(FUNC(coco_t4426_device::write_f9_clock));
	m_brg->out_f<11>().set(FUNC(coco_t4426_device::write_f11_clock));
	m_brg->out_f<13>().set(FUNC(coco_t4426_device::write_f13_clock));
	m_brg->out_f<15>().set(FUNC(coco_t4426_device::write_f15_clock));
}

ROM_START( coco_t4426 )
	// Part of this region is filled by set_bank
	ROM_REGION(0x4000, CARTSLOT_TAG, ROMREGION_ERASE00)
	// Main cartridge ROM
	ROM_LOAD("tercopmos4426-8549-4.31.bin", 0x2000, 0x1000, CRC(bc65c45c) SHA1(e50cfd1d61e29fe05eb795d8bf6303e7b91ed8e5))
	ROM_RELOAD(0x03000,0x1000) // Mirrored

	// this region is loaded from separate ROM images
	ROM_REGION(0x1a000, CARTBANK_TAG, ROMREGION_ERASE00)

	// 8 banked ROM:s
	ROM_LOAD("tercoed4426-0-8549-5.3.bin",  0x0000, 0x2000, CRC(45665428) SHA1(ff49a79275772c4c4ab1ae29db662c9b10a744a7))
	ROM_LOAD("tercoed4426-1-8549-5.3.bin",  0x2000, 0x2000, CRC(854cd50d) SHA1(0786391b4e7a78af0a984b6313eec7f71fb4ad9e))
	ROM_LOAD("tercopd4426-2-8632-6.4.bin",  0x4000, 0x2000, CRC(258e443a) SHA1(9d8901f3e70ae4f8526dde1b5208b22f066f801f))
	ROM_LOAD("tercopd4426-3-8638-6.4.bin",  0x6000, 0x2000, CRC(640d1de4) SHA1(5ae7427cb5729fd3920361855d954ea1f97f6ae5))
	ROM_LOAD("tercoca4426-4-8549-3.4.bin",  0x8000, 0x2000, CRC(df18397b) SHA1(2f9de210c039619c649be223c37a4eff873fa600))
	ROM_LOAD("tercoca4426-5-8549-3.4.bin",  0xa000, 0x2000, CRC(3fcdf92e) SHA1(ec1589f8d62701ca3faf2a85a57ab2e1f61d2137))
	ROM_LOAD("tercoca4426-6-8549-3.4.bin",  0xc000, 0x2000, CRC(27652ccf) SHA1(529a6f736666ae6660483e547d076725606b1c1d))
	ROM_LOAD("tercoca4426-7-8549-3.4.bin",  0xe000, 0x2000, CRC(f6640569) SHA1(03f70dcc5f7ab60cd908427d45ebd85b6f464b93))
ROM_END

//-------------------------------------------------
//  INPUT_PORTS( coco_cart_autostart )
//-------------------------------------------------

static INPUT_PORTS_START( coco_cart_autostart )
	PORT_START(CART_AUTOSTART_TAG)
	PORT_CONFNAME( 0x01, 0x01, "Cart Auto-Start" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ))
	PORT_CONFSETTING(    0x01, DEF_STR( On ))

	PORT_START(SERIAL_BAUD)
	PORT_CONFNAME(0x0F , 0x00 , "Ser. I/O2 RC 232C - Baud Rate") // Typo on real hardware!
	PORT_CONFSETTING(mc14411_device::TIMER_F1,  "9600") // RSA=RSB=1: X1
	PORT_CONFSETTING(mc14411_device::TIMER_F2,  "7200")
	PORT_CONFSETTING(mc14411_device::TIMER_F3,  "4800")
	PORT_CONFSETTING(mc14411_device::TIMER_F4,  "3600")
	PORT_CONFSETTING(mc14411_device::TIMER_F5,  "2400")
	PORT_CONFSETTING(mc14411_device::TIMER_F6,  "1800")
	PORT_CONFSETTING(mc14411_device::TIMER_F7,  "1200")
	PORT_CONFSETTING(mc14411_device::TIMER_F8,  "600")
	PORT_CONFSETTING(mc14411_device::TIMER_F9,  "300")
	PORT_CONFSETTING(mc14411_device::TIMER_F10, "200")
	PORT_CONFSETTING(mc14411_device::TIMER_F11, "150")
	PORT_CONFSETTING(mc14411_device::TIMER_F13, "110")
INPUT_PORTS_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(COCO_T4426, device_cococart_interface, coco_t4426_device, "coco_t4426", "Terco CNC Programming Station 4426 multi cart")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  coco_t4426_device - constructor
//-------------------------------------------------

coco_t4426_device::coco_t4426_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_cococart_interface(mconfig, *this)
	, m_cart(nullptr)
	, m_select(0)
	, m_autostart(*this, CART_AUTOSTART_TAG)
	, m_eprom(*this, CARTSLOT_TAG)
	, m_eprom_banked(*this, CARTBANK_TAG)
	, m_uart(*this, UART_TAG)
	, m_pia(*this, PIA_TAG)
	, m_brg(*this, BRG_TAG)
	, m_serial_baud(*this, SERIAL_BAUD)
{
}

coco_t4426_device::coco_t4426_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
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

	// save state support
	save_item(NAME(m_select));
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
	auto cart_line = line_value::Q;
	set_line_value(line::CART, cart_line);
	m_select = 0x00;

	// Set up the BRG divider statically to X1
	m_brg->rsa_w( ASSERT_LINE );
	m_brg->rsb_w( ASSERT_LINE );

	// Disable all configured timers, only enabling the used ones
	m_brg->timer_disable_all();
	m_brg->timer_enable((mc14411_device::timer_id) m_serial_baud->read(), true);
}

/*----------------------------------------------------
 * Serial port clock sources driven by the selected
 * output of the MC14411
 ----------------------------------------------------*/
void coco_t4426_device::write_acia_clocks(int id, int state)
{
	if (id == m_serial_baud->read())
	{
		m_uart->write_txc(state);
		m_uart->write_rxc(state);
	}
}

/*-------------------------------------------------
    scs_read custom devices
  PIA  is located at ff40-ff47
  ACIA is located at ff48-ff4F with A1 = 1
-------------------------------------------------*/

u8 coco_t4426_device::scs_read(offs_t offset)
{
	u8 result = 0x00;

	LOG("%s Offs:%d\n", FUNCNAME, offset);

	if ((offset >= 0x00) && (offset <= 0x07))
	{
		LOGPIA("- PIA\n");
		result = m_pia->read(offset & 3);
		LOGPIA("- Offs:%04x Data:%02x\n", offset - 0x04, result);
	}
	else if ((offset >= 0x08) && (offset <= 0x0f) && (offset & 2))
	{
		LOGACIA("- ACIA\n");
		result = m_uart->read(offset & 1);
		LOGACIA("- Offs:%04x Data:%02x\n", offset - 0x04, result);
	}
	else
	{
	  LOG(" Unknown Device! Offs:%04x\n", offset);
	}

	return result;
}

/*-------------------------------------------------
    scs_write - custom devices
  PIA  is located at ff40-ff47
  ACIA is located at ff48-ff4F with A1 = 1
-------------------------------------------------*/

void coco_t4426_device::scs_write(offs_t offset, u8 data)
{
	LOG("%s Offs:%d Data:%02x\n", FUNCNAME, offset, data);
	LOGSETUP(" * Offs:%02x <- %02x\n", offset, data);

	if ((offset >= 0x00) && (offset <= 0x07))
	{
		LOG("- PIA\n");
		m_pia->write(offset & 3, data);
	}
	else if ((offset >= 0x08) && (offset <= 0x0f) && (offset & 2))
	{
		LOGACIA("- ACIA");
		m_uart->write(offset & 1, data);
		LOGACIA(" - Offs:%04x Data:%02x\n", offset & 1, data);
	}
	else
	{
		LOG(" Unknown device\n");
	}
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

void coco_t4426_device::pia_A_w(u8 data)
{
	LOGPIA("%s(%02x)\n", FUNCNAME, data);
	m_select = data;
}

/*-------------------------------------------------
    cts_read
-------------------------------------------------*/

u8 coco_t4426_device::cts_read(offs_t offset)
{
	u8 result = 0x00;

	switch (offset & 0x2000)
	{
	case 0x0000:
		switch (m_select)
		{
		case 0:
		case ROM0:result = m_eprom_banked->base()[0x0000 | offset]; break;
		case ROM1:result = m_eprom_banked->base()[0x2000 | offset]; break;
		case ROM2:result = m_eprom_banked->base()[0x4000 | offset]; break;
		case ROM3:result = m_eprom_banked->base()[0x6000 | offset]; break;
		case ROM4:result = m_eprom_banked->base()[0x8000 | offset]; break;
		case ROM5:result = m_eprom_banked->base()[0xa000 | offset]; break;
		case ROM6:result = m_eprom_banked->base()[0xc000 | offset]; break;
		case ROM7:result = m_eprom_banked->base()[0xe000 | offset]; break;
		}
		break;

	case 0x2000:
		result = m_eprom->base()[offset];
		break;
	}

	return result;
}

/*-------------------------------------------------
    get_cart_base
-------------------------------------------------*/

u8 *coco_t4426_device::get_cart_base()
{
	LOG("%s - m_select %02x -> %02x\n", FUNCNAME, m_select, ~m_select & 0xff );
	return memregion(CARTSLOT_TAG)->base();
}
