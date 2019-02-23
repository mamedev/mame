// license:BSD-3-Clause
// copyright-holders:Tim Schuerewegen
/*************************************************************************************

    Cybiko Wireless Inter-tainment System

    (c) 2001-2007 Tim Schuerewegen

    Cybiko Classic (V1)
    Cybiko Classic (V2)
    Cybiko Xtreme

ToDo:
- Remove the memory leak in the nvram handler.
- Provide facility to load games via a "cart".
- Need instructions! The black screen doesn't fill me with confidence..unable to test.

**************************************************************************************/


#include "emu.h"
#include "includes/cybiko.h"

#include "screen.h"
#include "speaker.h"

//  +------------------------------------------------------+
//  | Cybiko Classic (CY6411)                         | V2 |
//  +------------------------------------------------------+
//  | - CYBIKO | CY-OS 1.1.7 | 6432241M04FA | 0028R JAPAN  |
//  | - SST 39VF020 | 90-4C-WH | 0012175-D                 |
//  | - ATMEL 0027 | AT90S2313-4SC                         |
//  | - ATMEL | AT45DB041A | TC | 0027                     |
//  | - RF2915 | RFMD0028 | 0F540BT                        |
//  | - EliteMT | LP62S2048X-70LLT | 0026B H4A27HA         |
//  | - MP02AB | LMX2315 | TMD                             |
//  +------------------------------------------------------+

//  +------------------------------------------------------+
//  | Cybiko Xtreme (CY44802)                              |
//  +------------------------------------------------------+
//  | - CYBIKO | CYBOOT 1.5A | HD6432323G03F | 0131 JAPAN  |
//  | - SST 39VF400A | 70-4C-EK                            |
//  | - ATMEL 0033 | AT90S2313-4SC                         |
//  | - SAMSUNG 129 | K4F171612D-TL60                      |
//  | - 2E16AB | USBN9604-28M | NSC00A1                    |
//  +------------------------------------------------------+

//  +------------------------------------------------------+
//  | Cybiko MP3 Player (CY65P10)                          |
//  +------------------------------------------------------+
//  | - H8S/2246 | 0G1 | HD6472246FA20 | JAPAN             |
//  | - MICRONAS | DAC3550A C2 | 0394 22 HM U | 089472.000 |
//  | - 2E08AJ | USBN9603-28M | NSC99A1                    |
//  +------------------------------------------------------+

///////////////////////////
// ADDRESS MAP - PROGRAM //
///////////////////////////

// 512 kbyte ram + no memory mapped flash
void cybiko_state::cybikov1_mem(address_map &map)
{
	map(0x000000, 0x007fff).rom();
	map(0x600000, 0x600001).rw(FUNC(cybiko_state::cybiko_lcd_r), FUNC(cybiko_state::cybiko_lcd_w));
//  AM_RANGE( 0xe00000, 0xe07fff ) AM_READ( cybikov1_key_r )
}

/*

v1 shutdown:
242628: sleep of powerdown
-> 2425d2
242ad2 bsr 2425d2
check 242ac4
counter at 235fa0
242a2e?
24297a -> 294e32 -> thread start?
ptr: 243150=24297a
21cf3e writes the pointer

*/


//  +-------------------------------------+
//  | Cybiko Classic (V2) - Memory Map    |
//  +-------------------------------------+
//  | 000000 - 007FFF | rom               |
//  | 008000 - 00FFFF | 17 51 17 51 ..    |
//  | 010000 - 0FFFFF | flash mirror      |
//  | 100000 - 13FFFF | flash             |
//  | 140000 - 1FFFFF | flash mirror      |
//  | 200000 - 23FFFF | ram               |
//  | 240000 - 3FFFFF | ram mirror        |
//  | 400000 - 5FFFFF | FF FF FF FF ..    |
//  | 600000 - 600001 | lcd               |
//  | 600002 - DFFFFF | FF FF FF FF ..    |
//  | E00000 - FFDBFF | keyboard          |
//  | FFDC00 - FFFFFF | onchip ram & regs |
//  +-------------------------------------+

// 256 kbyte ram + 256 kbyte memory mapped flash
void cybiko_state::cybikov2_mem(address_map &map)
{
	map(0x000000, 0x007fff).rom();
	map(0x100000, 0x13ffff).r("flash2", FUNC(sst_39vf020_device::read)).mirror(0x0c0000);
	map(0x600000, 0x600001).rw(FUNC(cybiko_state::cybiko_lcd_r), FUNC(cybiko_state::cybiko_lcd_w)).mirror(0x1ffffe);
	map(0xe00000, 0xffdbff).r(FUNC(cybiko_state::cybikov2_key_r));
}

// 2048 kbyte ram + 512 kbyte memory mapped flash
void cybiko_state::cybikoxt_mem(address_map &map)
{
	map(0x000000, 0x007fff).rom().mirror(0x038000);
	map(0x100000, 0x100001).rw(FUNC(cybiko_state::cybiko_lcd_r), FUNC(cybiko_state::cybiko_lcd_w));
	map(0x200000, 0x200003).w(FUNC(cybiko_state::cybiko_usb_w));
	map(0x600000, 0x67ffff).r("flashxt", FUNC(sst_39vf400a_device::read)).mirror(0x180000);
	map(0xe00000, 0xefffff).r(FUNC(cybiko_state::cybikoxt_key_r));
}

WRITE16_MEMBER(cybiko_state::serflash_w)
{
	m_flash1->cs_w ((data & 0x10) ? 0 : 1);
}

READ16_MEMBER(cybiko_state::clock_r)
{
	if (m_rtc->sda_r())
	{
		return (0x01|0x04);
	}

	return 0x04;
}

WRITE16_MEMBER(cybiko_state::clock_w)
{
	m_rtc->scl_w((data & 0x02) ? 1 : 0);
	m_rtc->sda_w((data & 0x01) ? 0 : 1);
}

READ16_MEMBER(cybiko_state::xtclock_r)
{
	if (m_rtc->sda_r())
	{
		return 0x40;
	}

	return 0;
}

WRITE16_MEMBER(cybiko_state::xtclock_w)
{
	m_rtc->scl_w((data & 0x02) ? 1 : 0);
	m_rtc->sda_w((data & 0x40) ? 0 : 1);
}

READ16_MEMBER(cybiko_state::xtpower_r)
{
	// bit 7 = on/off button
	// bit 6 = battery charged if "1"
	return 0xc0;
}

READ16_MEMBER(cybiko_state::adc1_r)
{
	return 0x01;
}

READ16_MEMBER(cybiko_state::adc2_r)
{
	return 0x00;
}

READ16_MEMBER(cybiko_state::port0_r)
{
	// bit 3 = on/off button
	return 0x08;
}


//////////////////////
// ADDRESS MAP - IO //
//////////////////////

void cybiko_state::cybikov1_io(address_map &map)
{
	map(h8_device::PORT_3, h8_device::PORT_3).w(FUNC(cybiko_state::serflash_w));
	map(h8_device::PORT_F, h8_device::PORT_F).rw(FUNC(cybiko_state::clock_r), FUNC(cybiko_state::clock_w));
	map(h8_device::ADC_1, h8_device::ADC_1).r(FUNC(cybiko_state::adc1_r));
	map(h8_device::ADC_2, h8_device::ADC_2).r(FUNC(cybiko_state::adc2_r));
}

void cybiko_state::cybikov2_io(address_map &map)
{
	map(h8_device::PORT_1, h8_device::PORT_1).r(FUNC(cybiko_state::port0_r));
	map(h8_device::PORT_3, h8_device::PORT_3).w(FUNC(cybiko_state::serflash_w));
	map(h8_device::PORT_F, h8_device::PORT_F).rw(FUNC(cybiko_state::clock_r), FUNC(cybiko_state::clock_w));
	map(h8_device::ADC_1, h8_device::ADC_1).r(FUNC(cybiko_state::adc1_r));
	map(h8_device::ADC_2, h8_device::ADC_2).r(FUNC(cybiko_state::adc2_r));
}

void cybiko_state::cybikoxt_io(address_map &map)
{
	map(h8_device::PORT_A, h8_device::PORT_A).r(FUNC(cybiko_state::xtpower_r));
	map(h8_device::PORT_F, h8_device::PORT_F).rw(FUNC(cybiko_state::xtclock_r), FUNC(cybiko_state::xtclock_w));
}

/////////////////
// INPUT PORTS //
/////////////////


static INPUT_PORTS_START( cybiko )
	PORT_START("A.0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Esc") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Del") PORT_CODE(KEYCODE_DEL) PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CHAR('`')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START("A.1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Up") PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("As") PORT_CODE(KEYCODE_INSERT) PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Fn") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)

	PORT_START("A.2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Help") PORT_CODE(KEYCODE_END) PORT_CHAR(UCHAR_MAMEKEY(END))

	PORT_START("A.3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')

	PORT_START("A.4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Select") PORT_CODE(KEYCODE_HOME) PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')

	PORT_START("A.5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_CHAR( 13 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|')

	PORT_START("A.6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("BkSp") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N')

	PORT_START("A.7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M')

	PORT_START("A.8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')

INPUT_PORTS_END

static INPUT_PORTS_START( cybikoxt )
	PORT_START("A.0")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L')

	PORT_START("A.1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J')

	PORT_START("A.2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T')

	PORT_START("A.3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')

	PORT_START("A.4")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_CHAR( 13 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Select") PORT_CODE(KEYCODE_HOME) PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')

	PORT_START("A.5")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t')
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Del") PORT_CODE(KEYCODE_DEL) PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("As") PORT_CODE(KEYCODE_INSERT) PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Esc") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))

	PORT_START("A.6")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Up") PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))

	PORT_START("A.7")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Fn") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)

	PORT_START("A.8")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START("A.13")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Help") PORT_CODE(KEYCODE_END) PORT_CHAR(UCHAR_MAMEKEY(END))
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P')

	PORT_START("A.14")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("On/Off") PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F8))

INPUT_PORTS_END

////////////////////
// MACHINE DRIVER //
////////////////////

static DEVICE_INPUT_DEFAULTS_START( debug_serial ) // set up debug port to default to 57600
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_57600 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_57600 )
	DEVICE_INPUT_DEFAULTS( "RS232_STARTBITS", 0xff, RS232_STARTBITS_1 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

void cybiko_state::cybikov1_debug_serial(machine_config &config)
{
	m_debug_serial->rxd_handler().set("maincpu:sci2", FUNC(h8_sci_device::rx_w));
	subdevice<h8_sci_device>("maincpu:sci2")->tx_handler().set(m_debug_serial, FUNC(rs232_port_device::write_txd));
}

void cybiko_state::cybikov1_base(machine_config &config)
{
	// screen
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_size(hd66421_device::WIDTH, hd66421_device::HEIGHT);
	screen.set_visarea(0, hd66421_device::WIDTH - 1, 0, hd66421_device::HEIGHT - 1);
	screen.set_screen_update("hd66421", FUNC(hd66421_device::update_screen));
	screen.set_palette("hd66421:palette");

	// video
	HD66421(config, m_crtc);

	// sound
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 1.00);

	// machine
	PCF8593(config, m_rtc);

	NVRAM(config, m_nvram, nvram_device::DEFAULT_ALL_0);

	// internal ram
	RAM(config, m_ram).set_default_size("512K").set_extra_options("1M");

	// serial debug port
	RS232_PORT(config, m_debug_serial, default_rs232_devices, nullptr);
	m_debug_serial->set_option_device_input_defaults("null_modem", DEVICE_INPUT_DEFAULTS_NAME(debug_serial));
	m_debug_serial->set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(debug_serial));
	m_debug_serial->set_option_device_input_defaults("pty", DEVICE_INPUT_DEFAULTS_NAME(debug_serial));

	// quickload
	quickload_image_device &quickload(QUICKLOAD(config, "quickload"));
	quickload.set_handler(snapquick_load_delegate(&QUICKLOAD_LOAD_NAME(cybiko_state, cybiko), this), "bin,nv");
}

void cybiko_state::cybikov1_flash(machine_config &config)
{
	AT45DB041(config, m_flash1, 0);
	m_flash1->so_callback().set("maincpu:sci1", FUNC(h8_sci_device::rx_w));
}

void cybiko_state::cybikov1(machine_config &config)
{
	cybikov1_base(config);

	// cpu
	H8S2241(config, m_maincpu, XTAL(11'059'200));
	m_maincpu->set_addrmap(AS_PROGRAM, &cybiko_state::cybikov1_mem);
	m_maincpu->set_addrmap(AS_IO, &cybiko_state::cybikov1_io);

	subdevice<h8_sci_device>("maincpu:sci1")->tx_handler().set("flash1", FUNC(at45db041_device::si_w));
	subdevice<h8_sci_device>("maincpu:sci1")->clk_handler().set("flash1", FUNC(at45db041_device::sck_w));

	// machine
	cybikov1_flash(config);
	cybikov1_debug_serial(config);
}

void cybiko_state::cybikov2(machine_config &config)
{
	cybikov1_base(config);
	cybikov1_flash(config);

	// cpu
	H8S2246(config, m_maincpu, XTAL(11'059'200));
	m_maincpu->set_addrmap(AS_PROGRAM, &cybiko_state::cybikov2_mem);
	m_maincpu->set_addrmap(AS_IO, &cybiko_state::cybikov2_io);

	subdevice<h8_sci_device>("maincpu:sci1")->tx_handler().set("flash1", FUNC(at45db041_device::si_w));
	subdevice<h8_sci_device>("maincpu:sci1")->clk_handler().set("flash1", FUNC(at45db041_device::sck_w));

	// machine
	SST_39VF020(config, "flash2");
	cybikov1_debug_serial(config);

	// internal ram
	m_ram->set_default_size("256K").set_extra_options("512K,1M");

	// serial debug port
	m_debug_serial->rxd_handler().set("maincpu:sci2", FUNC(h8_sci_device::rx_w));
	subdevice<h8_sci_device>("maincpu:sci2")->tx_handler().set(m_debug_serial, FUNC(rs232_port_device::write_txd));
}

void cybiko_state::cybikoxt(machine_config &config)
{
	cybikov1_base(config);

	// cpu
	H8S2323(config, m_maincpu, XTAL(18'432'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &cybiko_state::cybikoxt_mem);
	m_maincpu->set_addrmap(AS_IO, &cybiko_state::cybikoxt_io);

	// machine
	SST_39VF400A(config, "flashxt");

	// internal ram
	m_ram->set_default_size("2M");

	// serial debug port
	m_debug_serial->rxd_handler().set("maincpu:sci2", FUNC(h8_sci_device::rx_w));
	subdevice<h8_sci_device>("maincpu:sci2")->tx_handler().set("debug_serial", FUNC(rs232_port_device::write_txd));

	// quickload
	quickload_image_device &quickload(QUICKLOAD(config.replace(), "quickload"));
	quickload.set_handler(snapquick_load_delegate(&QUICKLOAD_LOAD_NAME(cybiko_state, cybikoxt), this), "bin,nv");
}

/////////
// ROM //
/////////

ROM_START( cybikov1 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "cyrom112.bin", 0, 0x8000, CRC(9e1f1a0f) SHA1(6fc08de6b2c67d884ec78f748e4a4bad27ee8045) )

	ROM_REGION( 0x84000, "flash1", 0 )
	ROM_LOAD( "flash_v1246.bin", 0, 0x84000, CRC(3816d0ab) SHA1(19be4fed8d95112568adf93219afe9406d7baecf) )
ROM_END

ROM_START( cybikov2 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "cyrom117.bin", 0, 0x8000, CRC(268da7bf) SHA1(135eaf9e3905e69582aabd9b06bc4de0a66780d5) )

	ROM_SYSTEM_BIOS( 0, "v1358", "v1.3.58" )
	ROM_SYSTEM_BIOS( 1, "v1357", "v1.3.57" )
	ROM_SYSTEM_BIOS( 2, "v1355", "v1.3.55" )

	ROM_REGION( 0x84000, "flash1", 0 )
	ROMX_LOAD( "flash_v1358.bin", 0, 0x84000, CRC(e485880f) SHA1(e414d6d2f876c7c811946bcdfcb6212999412381), ROM_BIOS(0) )
	ROMX_LOAD( "flash_v1357.bin", 0, 0x84000, CRC(9fd3c058) SHA1(dad0c3db0f11c91747db6ccc1900004432afb881), ROM_BIOS(1) )
	ROMX_LOAD( "flash_v1355.bin", 0, 0x84000, CRC(497a5bbe) SHA1(0af611424cbf287b26c668a3109fb0861a27f603), ROM_BIOS(2) )

	ROM_REGION( 0x40000, "flash2", 0 )
	ROMX_LOAD( "cyos_v1358.bin", 0, 0x40000, CRC(05ca4ece) SHA1(eee329e8541e1e36c22acb1317378ce23ccd1e12), ROM_BIOS(0) )
	ROMX_LOAD( "cyos_v1357.bin", 0, 0x40000, CRC(54ba7d43) SHA1(c6e0f7982e0f7a5fa65f2cecc8b27cb21909a407), ROM_BIOS(1) )
	ROMX_LOAD( "cyos_v1355.bin", 0, 0x40000, CRC(02d3dba5) SHA1(4ed728940bbcb3d2464fc7fba14d17924ece94aa), ROM_BIOS(2) )
ROM_END

ROM_START( cybikoxt )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "cyrom150.bin", 0, 0x8000, CRC(18b9b21f) SHA1(28868d6174eb198a6cec6c3c70b6e494517229b9) )

	ROM_SYSTEM_BIOS( 0, "v1508", "v1.5.08" )

	ROM_REGION16_BE( 0x80000, "flashxt", 0 )
	ROMX_LOAD( "cyos_v1508.bin", 0, 0x80000, CRC(f79400ba) SHA1(537a88e238746b3944b0cdfd4b0a9396460b2977), ROM_BIOS(0) )
ROM_END

//////////////
// DRIVERS  //
//////////////

/*    YEAR  NAME      PARENT    COMPAT  MACHINE   INPUT     CLASS         INIT           COMPANY       FULLNAME               FLAGS */
COMP( 2000, cybikov1, 0,        0,      cybikov1, cybiko,   cybiko_state, init_cybiko,   "Cybiko Inc", "Cybiko Classic (V1)", MACHINE_IMPERFECT_SOUND )
COMP( 2000, cybikov2, cybikov1, 0,      cybikov2, cybiko,   cybiko_state, init_cybiko,   "Cybiko Inc", "Cybiko Classic (V2)", MACHINE_IMPERFECT_SOUND )
COMP( 2001, cybikoxt, cybikov1, 0,      cybikoxt, cybikoxt, cybiko_state, init_cybikoxt, "Cybiko Inc", "Cybiko Xtreme",       MACHINE_IMPERFECT_SOUND )
