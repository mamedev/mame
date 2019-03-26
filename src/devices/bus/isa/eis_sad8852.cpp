// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/***********************************************************************************************************
 *
 *   Ericsson Information Systems/Nokia Data/ICL, SAD 8852 IBM 3270/5250 terminal emulation adapter
 *
 * This board is a terminal adapter for AT class PC machines to be connected as a terminal to
 * IBM mainframes. There are two on board connectors, a BNC connector to act as a 3270 terminal
 * and a twinax connector for the older 5250 terminal.
 *
 * The original design was probably done by the Alfaskop division at Ericsson Information Systems (EIS)
 * The Alfaskop division was aquired by Ericsson from Datasaab AB in 1980 who was very successful selling
 * IBM compatible terminals at strict 10% below IBM list price, whatever that was, usually quite high.
 * The Ericsson PC AT WS286, their first 16 bit PC, was introduced 1986 and as the SAD8852 card is a 16 bit
 * ISA card it is likelly to have been designed for the launch as Ericsson targeted the office market only.
 * EIS was aquired by the Nokia Data division in 1988 which in turn was aquired by ICL 1991 and the terminal
 * division by Wyse 1996. Firmware confirms 1987 as copyrigt date which fits the WS286 time line well.
 *
 * TODO:
 *    - Hook up 8274 and the rest of the local memory map
 *    - Hook up ISA IRQ:s
 *    - Find suitable software package to analyze control protocol between main cpu and the board cpu
 *    - Add bitbanger device and hook it up to an (emulated?!) IBM mainframe
 *
 ************************************************************************************************************/
/*
 Links:
 ------
   https://www.ibm.com/support/knowledgecenter/en/SSEQ5Y_5.9.0/com.ibm.pcomm.doc/books/html/admin_guide11.htm
   http://www.ebay.fr/itm/iCL-94V0-SAD-8852-ROA-1195296-R4B-Card-Expansion-Card/222825926815?hash=item33e175d09f:g:zAUAAOSwe7laPCr8
   https://www.sweclockers.com/forum/trad/1392179-intel-80188-16-bit-isa-expansionskort-vad-ar-detta (in swedish)
   http://www.veteranklubbenalfa.se/veteran/litt/jub97v3_4.pdf (PDF about the computer industry of Sweden, in swedish)

 PCB layouts and assembly years from online pictures and physical unit.
 Ericsson   -  marked TVK 119 5211 R3, rev date xxxx, assembled in 1987 indicated by chip dates
 Nokia Data -  marked TVK 119 5211 R3, rev date 9127, assembled in 1991 indicated by chip dates
 ICL        -  marked TVK 119 5211 R3, rev date 9343, assembled in 1993 indicated by chip dates
 +--------------------------------------------------------------------------------------+ ___
 |       IC18  IC24  +----------+                                                   ||
 | IC1  IC9              |IC31 EPROM|  +--------------+   IC44                          ||
 |           IC19  IC25  |27128     |  | IC35 i8274   |         IC51                    ||
 | IC2  IC10             +----------+  | MPSC serial  |  IC45                           ||
 |                                     +--------------+        +--------+               ||
 | IC3  IC11     IC17    IC32                                   SW1 1-10  IC57          ||
 |                       +----------+  IC23   IC40       IC46  +--------+               ||
 | IC4  IC12      IC36   |          |                           IC52      IC58  IC63   o-|
 |           IC20        | IC33     |  IC37   IC41       IC47                           ||
 | IC5  IC13       IC26  | i80188   |                         IC53 XTAL   IC59          ||
 |           IC21        | CPU      |  IC38   IC42       IC48  19.17MHz                 ||
 | IC6  IC14       IC27  |          |                               IC54     IC62       ||
 |                       +----------+                          J3                  J2   ||---
 | IC7  IC15                XTAL       IC39   IC43        IC49 I   IC55   IC60          || |= Twinax (5250)
 |                 IC28     12Mhz                              R                   J1   ||---
 | IC8  IC16                                             IC50  Q   IC56   IC61         o-|
 |           IC22  IC29 IC30 IC34          +--------------------------------------------+|---
 +-----------------------------------------+    |||||||||  |||||||||||||||||||||||||     | |- BNC (3270)
                                                                                         |---
                                                                                         |
 IC's
 ------------------------------------------------------------------------------
 IC1-                                     IC40-
 IC16 41256-12 256Kbits DRAM => 512KB     IC42 74ALS273
 IC17 PAL20L8ACNS '1020045 R2A'           IC43 74F138
 IC18 74F109                              IC44 ROP101506 kS09240CB012Y- custom Philips chip?
 IC19-                                    IC45 74ALS540
 IC20 74F00                               IC46-
 IC21-                                    IC47 74ALS684
 IC22 74ALS153                            IC48 74ALS521
 IC23 74LS139                             IC49 74F257
 IC24 74F74                               IC50 74ALS541
 IC25 74LS258                             IC51 LM339 - quad voltage comparator
 IC26 74F32                               IC52 74ALS74
 IC27-                                    IC53 XTAL 19.170MHz SCC oscillator
 IC28 74ALS153                            IC54 74ALS38
 IC29 74F175                              IC55 74F138
 IC30 74F08                               IC56 74ALS541
 IC31  EPROM 'RON 1020044/R2A D500 IC31'  IC57 SN75452BP - dual very-fast NAND
 IC32 74ALS573                            IC58 74F109
 IC33 i80188 CPU                          IC59 74F10
 IC34 74F00                               IC60 74ALS573
 IC35 i8274 MPSC serial controller        IC61 74ALS541
 IC36 74ALS573                            IC62 74F32
 IC37 74F10                               IC63 74F175
 IC38 74ALS245
 IC39 74F00

 General description
 -------------------
 The PCB has a 10 bit DIP switch and three jumper fields

 SW1 has 10 positions labeled '1' to '10', left to right, default bit value is off (down) and only bit '10' is on (up)
 J1 has 2 positions labeled 'AT' and 'PC' top to bottom, 'AT' is default
 J2 has 2 positions labeled 'AT' and 'PC' top to bottom, 'AT' is default
 J3 has 7 positions labeled 'IRQ2', IRQ3', 'IRQ4', 'IRQ10', 'IRQ11, 'IRQ12' and 'IRQ15' top to bottom, 'IRQ11' is default

 */

#include "emu.h"
#include "eis_sad8852.h"
#include "cpu/i86/i186.h"
#include "machine/z80sio.h"

#define LOG_READ    (1U << 1)

//#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

#define LOGR(...)   LOGMASKED(LOG_READ, __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

DEFINE_DEVICE_TYPE(ISA16_SAD8852, isa16_sad8852_device, "sad8852", "SAD8852 IBM mainframe terminal adapter")

#define I80188_TAG "ic33"

//-------------------------------------------------
//  Access methods from ISA bus
//-------------------------------------------------
READ8_MEMBER( isa16_sad8852_device::sad8852_r )
{
	LOG("%s sad8852_r(): offset=%d\n", FUNCNAME, offset);
	return 0xff;
}

WRITE8_MEMBER( isa16_sad8852_device::sad8852_w )
{
	LOG("%s : offset=%d data=0x%02x\n", FUNCNAME, offset, data);
}

//-------------------------------------------------
//  ROM( sad8852 )
//-------------------------------------------------
ROM_START( sad8852 )
	ROM_REGION( 0x4000, I80188_TAG, 0 )
	ROM_LOAD( "ron_1020044_r2a_d500_ic31.bin", 0x0000, 0x4000, CRC(cbdd042a) SHA1(7a3d43e6b1f6fcd4402c54aaacf169ac74141cd2) )
ROM_END

const tiny_rom_entry *isa16_sad8852_device::device_rom_region() const
{
	return ROM_NAME( sad8852 );
}

//-------------------------------------------------
//  ADDRESS maps
//-------------------------------------------------
void isa16_sad8852_device::sad8852_mem(address_map &map)
{
	map(0x00000, 0x80000).ram();
	map(0xfc000, 0xfffff).rom().region(I80188_TAG, 0);
}

void isa16_sad8852_device::sad8852_io(address_map &map)
{
}

//----------------------------------------------------------
//  UI I/O
//  - TODO: Figure out function of SW1, J1 and J2
//          SW1 bits 1-8 are gated to data bus through IC45
//----------------------------------------------------------
static INPUT_PORTS_START( sad8852_ports )
	PORT_START("SW1")
	PORT_DIPNAME( 0x3ff, 0x001, "Unknown" )
	PORT_DIPSETTING( 0x00, "None" )
	PORT_DIPSETTING( 0x01, "Default" )

	PORT_START("J1")
	PORT_CONFNAME( 0x01, 0x01, "PC - AT")
	PORT_CONFSETTING(    0x00, "PC")
	PORT_CONFSETTING(    0x01, "AT")

	PORT_START("J2")
	PORT_CONFNAME( 0x01, 0x01, "PC - AT")
	PORT_CONFSETTING(    0x00, "PC")
	PORT_CONFSETTING(    0x01, "AT")

	PORT_START("J3")
	PORT_CONFNAME( 0x07, 0x04, "ISA IRQ")
	PORT_CONFSETTING(    0x00, "IRQ2")
	PORT_CONFSETTING(    0x01, "IRQ3")
	PORT_CONFSETTING(    0x02, "IRQ4")
	PORT_CONFSETTING(    0x03, "IRQ10")
	PORT_CONFSETTING(    0x04, "IRQ11")
	PORT_CONFSETTING(    0x05, "IRQ12")
	PORT_CONFSETTING(    0x06, "IRQ15")
INPUT_PORTS_END

ioport_constructor isa16_sad8852_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( sad8852_ports );
}

//-------------------------------------------------
//  Board configuration
//-------------------------------------------------
void isa16_sad8852_device::device_add_mconfig(machine_config &config)
{
	i80188_cpu_device &cpu(I80188(config, I80188_TAG, XTAL(12'000'000) / 2)); // Chip revision is 6 MHz
	cpu.set_addrmap(AS_PROGRAM, &isa16_sad8852_device::sad8852_mem);
	cpu.set_addrmap(AS_IO, &isa16_sad8852_device::sad8852_io);

	I8274_NEW(config, "terminal", XTAL(12'000'000) / 3); // Needs verification
}

isa16_sad8852_device::isa16_sad8852_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ISA16_SAD8852, tag, owner, clock)
	, device_isa16_card_interface(mconfig, *this)
	, m_sw1(*this, "SW1")
	, m_j1(*this, "J1")
	, m_j2(*this, "J2")
	, m_isairq(*this, "J3")
	, m_installed(false)
	, m_irq(4)
{
}

//-------------------------------------------------
//  Overloading methods
//-------------------------------------------------
void isa16_sad8852_device::device_start()
{
	set_isa_device();
	m_installed = false;
}


void isa16_sad8852_device::device_reset()
{
	if (!m_installed)
	{
		m_isa->install_device(
				0x378, 0x378, // Wrong, need to find real i/o addresses
				read8_delegate(FUNC( isa16_sad8852_device::sad8852_r ), this),
				write8_delegate(FUNC( isa16_sad8852_device::sad8852_w ), this));
		m_irq = m_isairq->read();
		m_installed = true;
	}
}
