// license:GPLv2+
// copyright-holders:Felipe Sanches
#include "emu.h"
#include "cpu/i386/i386ex.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/ins8250.h"

/*
CPU Board:

U1  BRIGHT BM29F040-90NC                    | 4 MEGABIT (512K × 8) 5 VOLT SECTOR ERASE CMOS FLASH MEMORY
U5  NEW2001-16 1B9285LT M002646             | undumped maskROM
U4  HB993200-32                             | undumped maskROM
U2  EliteMT LP621024DM-70LL 0021S B3P14BA   | 128K X 8 BIT CMOS SRAM
U3  EliteMT LP621024DM-70LL 0021S B3P14BA   | 128K X 8 BIT CMOS SRAM
U10 Motorola AC139 XAA021 (16pin smd)       | ? (DUAL 1-OF-4 DECODER/DEMULTIPLEXER ?)
U7  Intel KU80386EX33                       | Intel 80386 EX Embedded Microprocessor
U11 XTAL 40.500MHz JTC                      | 40.5MHz crystal 
U6  XTAL 48.000MHz JTC                      | 48MHz crystal
U13 TMTECH UAP49MR 0025 83MHz T224162B-35J  | 256K x 16 DYNAMIC RAM
U15 Winbond W9925QF-K                       | VCD (Video CD) Decoder 
U16 Winbond W9952QP                         | TV Encoder
U12 ADC VISA99 0023E (160pin)               | (custom ?)
U17 TMTECH UAP49MR 0025 83MHz T224162B-35J  | 256K x 16 DYNAMIC RAM
U18 TMTECH UAP49MR 0025 83MHz T224162B-35J  | 256K x 16 DYNAMIC RAM
U9  LS04                                    | 6 x Boolean NOT gate
U14 Motorola MC1378P CTCG0029               | Motorola - Color Television Composite Video Overlay Synchronizer
U19  ?????                                  | ?

Audio Module Board:

?

Front Panel:

Atmega AT89C51 mcu (protected ?)
*/

class vmp3700_state : public driver_device
{
public:
	vmp3700_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_port1(0)
		, m_port2(0)
		, m_port3(0)
		, m_maskrom_bank(0)
		, m_maincpu(*this, "maincpu")
		, m_frontpanelcpu(*this, "frontpanel")
	{
	}

	void vmp3700(machine_config &config);

private:
	void mem_map(address_map &map);
	void io_map(address_map &map);
	void frontpanel_mem_map(address_map &map);

	uint16_t p1_data_r(offs_t offset);
	uint16_t p2_data_r(offs_t offset);
	uint16_t p3_data_r(offs_t offset);
	void p1_data_w(offs_t offset, uint16_t data);
	void p2_data_w(offs_t offset, uint16_t data);
	void p3_data_w(offs_t offset, uint16_t data);
	uint16_t p1_dir_r(offs_t offset);
	uint16_t p2_dir_r(offs_t offset);
	uint16_t p3_dir_r(offs_t offset);
	void p1_dir_w(offs_t offset, uint16_t data);
	void p2_dir_w(offs_t offset, uint16_t data);
	void p3_dir_w(offs_t offset, uint16_t data);
	uint16_t p1_config_r(offs_t offset);
	uint16_t p2_config_r(offs_t offset);
	uint16_t p3_config_r(offs_t offset);
	void p1_config_w(offs_t offset, uint16_t data);
	void p2_config_w(offs_t offset, uint16_t data);
	void p3_config_w(offs_t offset, uint16_t data);
	void vcd_decoder_w(offs_t offset, uint16_t data);
	uint16_t m_port1;
	uint16_t m_port2;
	uint16_t m_port3;
	uint16_t m_port1_config;
	uint16_t m_port2_config;
	uint16_t m_port3_config;
	uint16_t m_port1_direction;
	uint16_t m_port2_direction;
	uint16_t m_port3_direction;
	uint8_t m_maskrom_bank;
	required_device<i386ex_device> m_maincpu;
	required_device<at89c4051_device> m_frontpanelcpu;
};

void vmp3700_state::vcd_decoder_w(offs_t offset, uint16_t data)
{
	//TODO: Implement-me!
	//logerror("[VCD DECODER - Winbond W9925QF-K] write %02X '%c'\n", data & 0xFF, data & 0xFF);
}


//          ADH  ADL |   MSKH MSKL |                address | SMM | bits | MEM | RDY | RES. | WAIT STATE
// CS0:  0x0000 0503 | 0x0001 FC01 |     000 << 11 = 000000 |   1 |    8 |   1 |   0 |   00 | 00011
// CS1:  0x0002 0503 | 0x0001 FC01 |     400 << 11 = 200000 |   1 |    8 |   1 |   0 |   00 | 00011
// CS2:  0x0006 8503 | 0x0000 7C01 |     C10 << 11 = 610000 |   1 |    8 |   1 |   0 |   00 | 00011
// CS3:  0x0004 0783 | 0x0000 FC01 |     800 << 11 = 400000 |   1 |   16 |   1 |   1 |   00 | 00011
// CS4:  0x0007 0503 | 0x0000 FC01 |     E00 << 11 = 700000 |   1 |    8 |   1 |   0 |   00 | 00011
// CS5:  0x0005 0503 | 0x0000 FC01 |     A00 << 11 = 500000 |   1 |    8 |   1 |   0 |   00 | 00011
// CS6:  0x0006 0503 | 0x0000 7C01 |     C00 << 11 = 600000 |   1 |    8 |   1 |   0 |   00 | 00011
// UCS:  0x0008 0503 | 0x0007 FC01 |    1000 << 11 = 800000 |   1 |    8 |   1 |   0 |   00 | 00011
//SS:SP => C9E0 + 0400 = 0xCDE0

void vmp3700_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0x0ffff).ram();	//CS0 128k SRAM at U2
	map(0x20000, 0x2ffff).ram();	//CS1 128k SRAM at U3
//	map(0x40000, 0x40fff).?		//CS3 U15-pin3 (Winbond W9925QF-K) ?
	map(0x50000, 0x50fff).rom().region("maskrom_u5", 0);	//CS5 U5-pin12 MX23C6410
//	map(0x60000, 0x607ff).?		//CS6 
	map(0x68000, 0x687ff).w(FUNC(vmp3700_state::vcd_decoder_w)); //CS2 U15-pin3 (Winbond W9925QF-K VCD Decoder) ?
	map(0x70000, 0x70fff).rom().region("maskrom_u4", 0);	//CS4 U4-pin12 MX23C6410

	map(0x080000, 0x0fffff).rom().region("bios", 0);	//UCS 512k
	map(0x800000, 0x87ffff).mirror(0x3780000).rom().region("bios", 0);	//UCS 512k
}

/*
[:maincpu] CS0# address: 000000 mask: 0007FF
[:maincpu] CS1# address: 020000 mask: 0007FF
[:maincpu] CS2# address: 068000 mask: 0007FF
[:maincpu] CS3# address: 040000 mask: 0007FF
[:maincpu] CS4# address: 070000 mask: 0007FF
[:maincpu] CS5# address: 050000 mask: 0007FF
[:maincpu] CS6# address: 060000 mask: 0007FF
[:maincpu] CS7# address: 080000 mask: 0007FF
*/

#define P1CFG 0xf820
#define P2CFG 0xf822
#define P3CFG 0xf824

#define P1DIR 0xf864
#define P2DIR 0xf86c
#define P3DIR 0xf874

#define P1LTC 0xf862
#define P2LTC 0xf86a
#define P3LTC 0xf872

#define P1PIN 0xf860
#define P2PIN 0xf868
#define P3PIN 0xf870

void vmp3700_state::io_map(address_map &map)
{
	map(P1LTC, P1LTC+1).rw(FUNC(vmp3700_state::p1_data_r), FUNC(vmp3700_state::p1_data_w));
	map(P2LTC, P2LTC+1).rw(FUNC(vmp3700_state::p2_data_r), FUNC(vmp3700_state::p2_data_w));
	map(P3LTC, P3LTC+1).rw(FUNC(vmp3700_state::p3_data_r), FUNC(vmp3700_state::p3_data_w));
	map(P1DIR, P1DIR+1).rw(FUNC(vmp3700_state::p1_dir_r), FUNC(vmp3700_state::p1_dir_w));
	map(P2DIR, P2DIR+1).rw(FUNC(vmp3700_state::p2_dir_r), FUNC(vmp3700_state::p2_dir_w));
	map(P3DIR, P3DIR+1).rw(FUNC(vmp3700_state::p3_dir_r), FUNC(vmp3700_state::p3_dir_w));
	map(P1CFG, P1CFG+1).rw(FUNC(vmp3700_state::p1_config_r), FUNC(vmp3700_state::p1_config_w));
	map(P2CFG, P2CFG+1).rw(FUNC(vmp3700_state::p2_config_r), FUNC(vmp3700_state::p2_config_w));
	map(P3CFG, P3CFG+1).rw(FUNC(vmp3700_state::p3_config_r), FUNC(vmp3700_state::p3_config_w));
}

void vmp3700_state::frontpanel_mem_map(address_map &map)
{
	map(0x0000, 0x0fff).rom().region("frontpanel", 0); // mcu (protected?)
}


/*********************************************************
  Parallel Port I/O Handling:

  Note : The ports are 8 bits, but the I/O bus is 16 bits
         Thus, only the low 8 bits are used.
**********************************************************/

static void bit_pattern(char* bits, int n, int data){
	int i;
	for (i=0; i<n; i++){
		bits[n-1-i] = (data & (1<<i)) ? '1' : '0';
	}
	bits[n] = '\0';
}

//---------------- PnLTC handlers: -------------------------

// JP2: (front panel connector)
// 1: GND 
// 2: RXD1
// 3: (TXD1, via 74139)
// 4: 5V (?)

// 74139
// ENABLE (pin15): CPU TXD1
// SELECTs A2/B2 => CPU pin 108 = P1.6
// 2Y0: JP2 - pin 3 (Front Panel)
// 2Y3: JP3 - pin 5 (Audio Module)

// KD16901A - U4-soundboard Alternative Part number S5A1901H02 "Audio Effect Processor"

uint16_t vmp3700_state::p1_data_r(offs_t offset)
{
	//TODO: Implement-me!
	return m_port1;
}

void vmp3700_state::p1_data_w(offs_t offset, uint16_t data)
{
	//TODO: Implement-me!
	char bits[9];
	bit_pattern(bits, 8, data);
	logerror("Port 1 data write:\t %02X [%s]\n", data, bits);
	logerror("Serial port #1 routed to: %s\n", BIT(data, 6) ? "Audio Module" : "Front Panel");
	m_port1 = data;
}

uint16_t vmp3700_state::p2_data_r(offs_t offset)
{
	//TODO: Implement-me!
	return m_port2;
}

void vmp3700_state::p2_data_w(offs_t offset, uint16_t data)
{
	//TODO: Implement-me!
	char bits[9];
	bit_pattern(bits, 8, data);
	logerror("Port 2 data write:\t %02X [%s]\n", data, bits);
	m_port2 = data;
}

/*
MaskROM pinout (for bankswitching):
   A17 = P3.2 pin 80
   A18 = P3.3 pin 82
   A19 = P3.4 pin 84
   A20 = P3.5 pin 85
U4 A21 = P3.6 pin 86
*/

uint16_t vmp3700_state::p3_data_r(offs_t offset)
{
	return m_port3;
}

void vmp3700_state::p3_data_w(offs_t offset, uint16_t data)
{
	char bits[9];
	bit_pattern(bits, 8, data);
	m_maskrom_bank = (data >> 2) & 0x1f;

	logerror("Port 3 data write:\t %02X [%s] -- BANK SELECTION: %d\n", data, bits, m_maskrom_bank);
	m_port3 = data;
}

//---------------- PnDIR handlers: -------------------------

uint16_t vmp3700_state::p1_dir_r(offs_t offset)
{
	//TODO: Implement-me!
	return m_port1_direction;
}

void vmp3700_state::p1_dir_w(offs_t offset, uint16_t data)
{
	//TODO: Implement-me!
	char bits[9];
	bit_pattern(bits, 8, data);
	logerror("Port 1 direction:\t %02X [%s]\n", data, bits);
	m_port1_direction = data;
}

uint16_t vmp3700_state::p2_dir_r(offs_t offset)
{
	//TODO: Implement-me!
	return m_port2_direction;
}

void vmp3700_state::p2_dir_w(offs_t offset, uint16_t data)
{
	//TODO: Implement-me!
	char bits[9];
	bit_pattern(bits, 8, data);
	logerror("Port 2 direction:\t %02X [%s]\n", data, bits);
	m_port2_direction = data;
}

uint16_t vmp3700_state::p3_dir_r(offs_t offset)
{
	//TODO: Implement-me!
	return m_port3_direction;
}

void vmp3700_state::p3_dir_w (offs_t offset, uint16_t data)
{
	//TODO: Implement-me!
	char bits[9];
	bit_pattern(bits, 8, data);
	logerror("Port 3 direction:\t %02X [%s]\n", data, bits);
	m_port3_direction = data;
}

//---------------- PnCFG handlers: -------------------------

uint16_t vmp3700_state::p1_config_r(offs_t offset)
{
	//TODO: Implement-me!
	return m_port1_config;
}

void vmp3700_state::p1_config_w (offs_t offset, uint16_t data)
{
	//TODO: Implement-me!
	char bits[9];
	bit_pattern(bits, 8, data);
	logerror("Port 1 config:\t %02X [%s]\n", data, bits);
	m_port1_config = data;
}

uint16_t vmp3700_state::p2_config_r(offs_t offset)
{
	//TODO: Implement-me!
	return m_port2_config;
}

void vmp3700_state::p2_config_w(offs_t offset, uint16_t data)
{
	//TODO: Implement-me!
	char bits[9];
	bit_pattern(bits, 8, data);
	logerror("Port 2 config:\t %02X [%s]\n", data, bits);
	m_port2_config = data;
}

uint16_t vmp3700_state::p3_config_r(offs_t offset)
{
	//TODO: Implement-me!
	return m_port3_config;
}

void vmp3700_state::p3_config_w(offs_t offset, uint16_t data)
{
	//TODO: Implement-me!
	char bits[9];
	bit_pattern(bits, 8, data);
	logerror("Port 3 config:\t %02X [%s]\n", data, bits);
	m_port3_config = data;
}


void vmp3700_state::vmp3700(machine_config &config)
{
	I386EX(config, m_maincpu, 48_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &vmp3700_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &vmp3700_state::io_map);

	AT89C4051(config, m_frontpanelcpu, 12_MHz_XTAL); /* Atmel "AT89C51 20PC 9927" with xtal labeled "SB12.000" */
	m_frontpanelcpu->set_addrmap(AS_PROGRAM, &vmp3700_state::frontpanel_mem_map);

//	MCFG_MCS51_SERIAL_RX_CB(DEVWRITELINE("maincpu:uart1", ins8250_uart_device, rx_w));
//	MCFG_MCS51_SERIAL_TX_CB(DEVREADLINE("maincpu:uart1", ins8250_uart_device, tx_w));

//	MCFG_DEVICE_MODIFY("maincpu:uart1");
//	MCFG_INS8250_OUT_TX_CB(DEVWRITELINE("frontpanel", _device, write_txd));

//	TODO:	- video chips
//		- sound board
}

ROM_START(vmp3700)
	ROM_REGION16_LE(0x80000,"bios", 0)
	ROM_LOAD("vmp3700.u1", 0x00000, 0x80000, CRC(a98ee764) SHA1(10352257f6be9e5053cc05e39932efd8de4f89a8))

	ROM_REGION16_LE(0x800000,"maskrom_u4", 0)
	ROM_LOAD("hb993200-32.u4", 0x00000, 0x800000, NO_DUMP)

	ROM_REGION16_LE(0x800000,"maskrom_u5", 0)
	ROM_LOAD("new2001-16.u5", 0x00000, 0x800000, NO_DUMP)

	ROM_REGION(0x1000,"frontpanel", 0)
	ROM_LOAD("at89c51.u5", 0x0000, 0x1000, NO_DUMP)
ROM_END

/*    YEAR  NAME     PARENT  COMPAT   MACHINE  INPUT                       INIT     COMPANY            FULLNAME */
COMP( 1998, vmp3700,      0,      0,  vmp3700,     0, vmp3700_state, empty_init,    "Raf Electronics", "Vmp 3700 Videoke", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
