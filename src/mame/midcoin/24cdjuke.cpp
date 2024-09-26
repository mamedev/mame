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
1x      oscillator      6.0MHz  Q1              Near MCU
1x      oscillator      ?MHz    Q2              Near Z80 (ceramic resonator)

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
1x      16 digits VFD (on solder side), each digit is made by 18 segments
Notes

This is the PCB for one of the first ever CD based Juke Box, made in 1988 by Midcoin, some info here:
http://www.tilt.it/deb/i-midcoin.html


*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "24cdjuke.lh"


namespace {

class midcoin24cdjuke_state : public driver_device
{
public:
	midcoin24cdjuke_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_io_row0(*this, "ROW0")
		, m_io_row1(*this, "ROW1")
		, m_io_row2(*this, "ROW2")
		, m_io_row3(*this, "ROW3")
		, m_charset(*this, "charset")
		, m_digits(*this, "digit%u", 0U)
	{ }

	void midcoin24cdjuke(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_ioport m_io_row0;
	required_ioport m_io_row1;
	required_ioport m_io_row2;
	required_ioport m_io_row3;
	required_region_ptr<uint16_t> m_charset;
	output_finder<16> m_digits;

	uint8_t kb_row_r();
	void kb_col_w(uint8_t data);
	void digit_w(offs_t offset, uint8_t data);

	uint8_t unknown_r() { return machine().rand(); }

	void midcoin24cdjuke_io(address_map &map) ATTR_COLD;
	void midcoin24cdjuke_map(address_map &map) ATTR_COLD;

	uint8_t m_kb_col;
};


uint8_t midcoin24cdjuke_state::kb_row_r()
{
	uint8_t data = 0xff;

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

void midcoin24cdjuke_state::kb_col_w(uint8_t data)
{
	m_kb_col = data & 0xf0;
}

void midcoin24cdjuke_state::digit_w(offs_t offset, uint8_t data)
{
	uint16_t char_data = m_charset[((data & 0x60) << 1) | (data & 0x1f)];

	char_data = bitswap<16>(char_data, 13,11,9,15,14,10,12,8,7,6,5,4,3,2,1,0);

	m_digits[offset] = char_data ^ 0xffff;
}


void midcoin24cdjuke_state::midcoin24cdjuke_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x7800, 0x780f).w(FUNC(midcoin24cdjuke_state::digit_w));
	map(0x8000, 0xffff).ram();
}

void midcoin24cdjuke_state::midcoin24cdjuke_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw("ic31", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x04, 0x07).rw("ic11", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x08, 0x0b).rw("ic25", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x0c, 0x0c).nopw();
	map(0x10, 0x1f).r(FUNC(midcoin24cdjuke_state::unknown_r));
}

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
	m_digits.resolve();
}

void midcoin24cdjuke_state::machine_reset()
{
}


void midcoin24cdjuke_state::midcoin24cdjuke(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 6000000);         /* ? MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &midcoin24cdjuke_state::midcoin24cdjuke_map);
	m_maincpu->set_addrmap(AS_IO, &midcoin24cdjuke_state::midcoin24cdjuke_io);
	m_maincpu->set_periodic_int(FUNC(midcoin24cdjuke_state::irq0_line_hold), attotime::from_hz(500));

	config.set_default_layout(layout_24cdjuke);

	i8255_device &ic11(I8255A(config, "ic11"));
	ic11.in_pa_callback().set_ioport("MD1");
	ic11.in_pb_callback().set_ioport("MD2");
	ic11.in_pc_callback().set_ioport("MD3");

	i8255_device &ic25(I8255A(config, "ic25"));
	ic25.in_pb_callback().set_ioport("PB");
	ic25.in_pc_callback().set(FUNC(midcoin24cdjuke_state::kb_row_r));
	ic25.out_pc_callback().set(FUNC(midcoin24cdjuke_state::kb_col_w));

	i8255_device &ic31(I8255A(config, "ic31", 0));
	ic31.out_pb_callback().set([this](uint8_t data) { logerror("%s ic31 write port B: %02X\n", machine().describe_context(), data); });
	ic31.in_pc_callback().set_ioport("MD4");
}


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

} // anonymous namespace


GAME( 1988, 24cdjuke, 0, midcoin24cdjuke, midcoin24cdjuke, midcoin24cdjuke_state, empty_init, ROT0, "Midcoin", "Midcoin Juke Box 24CD", MACHINE_NO_SOUND | MACHINE_NOT_WORKING ) // what name was it sold under? name is from the PCB text
