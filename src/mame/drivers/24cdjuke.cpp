// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/*

 Midcoin 24 CD Coin-operated Jukebox

CPUs
QTY     Type            clock   position        function
1x      D780C-1                 ic1             8-bit Microprocessor - main
1x      MAB8441T-T042           ic10            8-bit Single Chip Microcontroller - main (internal ROM not dumped)
3x      P8255A-5                ic11,ic25,ic31  Programmable Peripheral Interface
1x      MM5450N                 ic29            LED Display Driver - main
2x      LM358                   ic64,ic65       Dual Operational Amplifier - sound
1x      oscillator      6.0MHz  Q1

ROMs
QTY     Type                    position        status
1x      D27128D                 ic5             dumped
2x      M1-7611A-5              ic27,ic28       dumped
2x      DM74LS471N              ic20,ic21       dumped

RAMs
QTY     Type                    position
1x      HY6116ALP-10            ic22
1x      D43256C-12L             ic6

Others

QTY     Type
1x      6 legs connector (ALIM - power)
1x      10 pins flat cable connector purple (ALIM DISP - power display)
1x      10 pins flat cable connector yellow (AMPLI - amplificator)
1x      10 pins flat cable connector green (MOTORE - engine)
1x      10 pins flat cable connector black (CD)
1x      10 pins flat cable connector blu (SERVICE)
1x      10 pins flat cable connector red (CONTROL MOT - engine control)
2x      jumper (J1 set, RIF unset) 5x trimmer (RP1,RP2,RP4,RP50,RP51)
2x      8x2 switches DIP (MD1,MD3)
1x      battery 3.6V
2x      red LED (5V,12V)
4x      red LED (CONTR. RAMPA - ramp control - L1,L2,L3,L4)
8x      red LED (HO, BR.D, P.D, D.I, BR.F, P.UP, ENC, MAB)
1x      red LED (5V)
1x      red LED (5V)
1x      red LED (5V)
1x      16 digits LED display (on solder side), each digit is made by 18 segments
Notes

This is the PCB for one of the first ever CD based Juke Box, made in 1988 by Midcoin, some info here:
http://www.tilt.it/deb/i-midcoin.html


*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "24cdjuke.lh"

class midcoin24cdjuke_state : public driver_device
{
public:
	midcoin24cdjuke_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_io_row0(*this, "ROW0"),
		m_io_row1(*this, "ROW1"),
		m_io_row2(*this, "ROW2"),
		m_io_row3(*this, "ROW3"),
		m_charset(*this, "charset")
		{ }

	required_device<cpu_device> m_maincpu;
	required_ioport m_io_row0;
	required_ioport m_io_row1;
	required_ioport m_io_row2;
	required_ioport m_io_row3;
	required_region_ptr<UINT16> m_charset;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	DECLARE_READ8_MEMBER(kb_row_r);
	DECLARE_WRITE8_MEMBER(kb_col_w);
	DECLARE_WRITE8_MEMBER(digit_w);

	DECLARE_READ8_MEMBER(unknown_r) { return machine().rand(); }

private:
	UINT8 m_kb_col;
};


READ8_MEMBER(midcoin24cdjuke_state::kb_row_r)
{
	UINT8 data = 0xff;

	if (!(m_kb_col & 0x10))
		data &= m_io_row0->read();
	if (!(m_kb_col & 0x20))
		data &= m_io_row1->read();
	if (!(m_kb_col & 0x40))
		data &= m_io_row2->read();
	if (!(m_kb_col & 0x80))
		data &= m_io_row3->read();

	return data;
}

WRITE8_MEMBER(midcoin24cdjuke_state::kb_col_w)
{
	m_kb_col = data & 0xf0;
}

WRITE8_MEMBER(midcoin24cdjuke_state::digit_w)
{
	UINT16 char_data = m_charset[((data & 0x60) << 1) | (data & 0x1f)];

	char_data = BITSWAP16(char_data, 13,11,9,15,14,10,12,8,7,6,5,4,3,2,1,0);

	output().set_digit_value(offset, char_data ^ 0xffff);
}


static ADDRESS_MAP_START( midcoin24cdjuke_map, AS_PROGRAM, 8, midcoin24cdjuke_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x7800, 0x780f) AM_WRITE(digit_w)
	AM_RANGE(0x8000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( midcoin24cdjuke_io, AS_IO, 8, midcoin24cdjuke_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("ic31", i8255_device, read, write)
	AM_RANGE(0x04, 0x07) AM_DEVREADWRITE("ic11", i8255_device, read, write)
	AM_RANGE(0x08, 0x0b) AM_DEVREADWRITE("ic25", i8255_device, read, write)
	AM_RANGE(0x0c, 0x0c) AM_WRITENOP
	AM_RANGE(0x10, 0x1f) AM_READ(unknown_r)
ADDRESS_MAP_END

static INPUT_PORTS_START( midcoin24cdjuke )
	PORT_START("ROW0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("DEL") PORT_CODE(KEYCODE_DEL)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("9") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("3") PORT_CODE(KEYCODE_3_PAD)
	PORT_START("ROW1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("0") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("8") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("2") PORT_CODE(KEYCODE_2_PAD)
	PORT_START("ROW2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("7") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("1") PORT_CODE(KEYCODE_1_PAD)
	PORT_START("ROW3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_R)
	PORT_START("PB")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("MD1")
	PORT_DIPNAME( 0x01, 0x01, "MD1 1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_START("MD2")
	PORT_DIPNAME( 0x01, 0x01, "MD2 1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_START("MD3")
	PORT_DIPNAME( 0x01, 0x01, "MD3 1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_START("MD4")
	PORT_DIPNAME( 0x01, 0x01, "MD4 1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


void midcoin24cdjuke_state::machine_start()
{
}

void midcoin24cdjuke_state::machine_reset()
{
}


static MACHINE_CONFIG_START( midcoin24cdjuke, midcoin24cdjuke_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,6000000)         /* ? MHz */
	MCFG_CPU_PROGRAM_MAP(midcoin24cdjuke_map)
	MCFG_CPU_IO_MAP(midcoin24cdjuke_io)
	MCFG_CPU_PERIODIC_INT_DRIVER(midcoin24cdjuke_state, irq0_line_hold, 500)

	MCFG_DEFAULT_LAYOUT(layout_24cdjuke)

	MCFG_DEVICE_ADD("ic11", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(IOPORT("MD1"))
	MCFG_I8255_IN_PORTB_CB(IOPORT("MD2"))
	MCFG_I8255_IN_PORTC_CB(IOPORT("MD3"))

	MCFG_DEVICE_ADD("ic25", I8255A, 0)
	MCFG_I8255_IN_PORTB_CB(IOPORT("PB"))
	MCFG_I8255_IN_PORTC_CB(READ8(midcoin24cdjuke_state, kb_row_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(midcoin24cdjuke_state, kb_col_w))

	MCFG_DEVICE_ADD("ic31", I8255A, 0)
	MCFG_I8255_OUT_PORTB_CB(LOGGER("PPI8255 - unmapped write port B", 0))
	MCFG_I8255_IN_PORTC_CB(IOPORT("MD4"))
MACHINE_CONFIG_END


ROM_START( 24cdjuke )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "1.ic5", 0x0000, 0x4000,  CRC(df2419ad) SHA1(dd9dd85011d46581dccabcfdb5959a8b018df937)  )

	ROM_REGION16_LE( 0x200, "charset", 0 )
	ROM_LOAD16_BYTE( "dm74ls471n.ic20", 0x000, 0x100, CRC(d05765e6) SHA1(119ec6ca1a4afa0ea6ab1020ba2a8b02fd434e3f) )
	ROM_LOAD16_BYTE( "dm74ls471n.ic21", 0x001, 0x100, CRC(e12d5a04) SHA1(be52ee4e4a5ea225fce39c759645a7cf49cea370) )

	// MAB8441T-T042 internal ROM?
	ROM_REGION( 0x80000, "misc", 0 )
	ROM_LOAD( "m1-7611a-5.ic27", 0x000, 0x100, CRC(29b068e8) SHA1(477e2445c58b7d14c56a3ad4050eb22474d56005) )
	ROM_LOAD( "m1-7611a-5.ic28", 0x000, 0x100, CRC(29b068e8) SHA1(477e2445c58b7d14c56a3ad4050eb22474d56005) )
ROM_END


GAME( 1988, 24cdjuke,  0,    midcoin24cdjuke, midcoin24cdjuke, driver_device,  0, ROT0, "Midcoin", "Midcoin Juke Box 24CD", MACHINE_NO_SOUND | MACHINE_NOT_WORKING ) // what name was it sold under? name is from the PCB text
