// license:BSD-3-Clause
// copyright-holders:

/*****************************************************************************

  Sega Exciting Speed Hockey @ 1993

  Hardware is very similar to sega/segaufo.cpp (see 3rd gen hardware).
  Kept separate because the mechanical parts are very different
  (crane vs. air hockey), so they will need different simulations.

  Board has two stickers: '834-9442 EXCITING SPEED HOCKEY' and '931105 1336 H'
  Main components are:
  1x Z80 (sticker obscures exact type)
  16.000 MHz XTAL
  2x 8-dip banks
  2x Sega 315-5296(I/O)
  1x NEC uPD71054C
  1x YM3438

*****************************************************************************/

#include "emu.h"

#include "315_5296.h"

#include "cpu/z80/z80.h"
#include "machine/pit8253.h"
#include "sound/ymopn.h"

#include "speaker.h"


namespace {

class eshockey_state : public driver_device
{
public:
	eshockey_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void eshockey(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;

	void pit_out0(int state);
	void pit_out1(int state);
	void pit_out2(int state);

	void prg_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
};


/***************************************************************************

  I/O

***************************************************************************/

void eshockey_state::pit_out0(int state)
{
	// ?
}

void eshockey_state::pit_out1(int state)
{
	// NMI?
	if (state)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void eshockey_state::pit_out2(int state)
{
	// ?
}

void eshockey_state::prg_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xe000, 0xffff).ram();
}

void eshockey_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x03).rw("pit", FUNC(pit8254_device::read), FUNC(pit8254_device::write));
	map(0x40, 0x43).rw("ym", FUNC(ym3438_device::read), FUNC(ym3438_device::write));
	map(0x80, 0xbf).rw("io1", FUNC(sega_315_5296_device::read), FUNC(sega_315_5296_device::write));
	map(0xc0, 0xff).rw("io2", FUNC(sega_315_5296_device::read), FUNC(sega_315_5296_device::write));
}


/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( eshockey )
	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "UNK1-01" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "UNK1-02" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "UNK1-04" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "UNK1-08" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "UNK1-10" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "UNK1-20" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "UNK1-40" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "UNK1-80" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "UNK2-01" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "UNK2-02" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "UNK2-04" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "UNK2-08" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "UNK2-10" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "UNK2-20" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "UNK2-40" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "UNK2-80" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


/***************************************************************************

  Machine Config

***************************************************************************/

void eshockey_state::eshockey(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 16_MHz_XTAL / 2); // divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &eshockey_state::prg_map);
	m_maincpu->set_addrmap(AS_IO, &eshockey_state::io_map);

	SEGA_315_5296(config, "io1", 16_MHz_XTAL);

	SEGA_315_5296(config, "io2", 16_MHz_XTAL);

	pit8254_device &pit(PIT8254(config, "pit", 16_MHz_XTAL / 2)); // uPD71054C, configuration is unknown, clocks not verified
	pit.set_clk<0>(16_MHz_XTAL / 2 / 256);
	pit.out_handler<0>().set(FUNC(eshockey_state::pit_out0));
	pit.set_clk<1>(16_MHz_XTAL / 2 / 256);
	pit.out_handler<1>().set(FUNC(eshockey_state::pit_out1));
	pit.set_clk<2>(16_MHz_XTAL / 2/ 256);
	pit.out_handler<2>().set(FUNC(eshockey_state::pit_out2));

	// no video

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ym3438_device &ym(YM3438(config, "ym", 16_MHz_XTAL / 2)); // divider not verified
	ym.irq_handler().set_inputline("maincpu", 0);
	ym.add_route(0, "mono", 0.40);
	ym.add_route(1, "mono", 0.40);
}


/***************************************************************************

  Game drivers

***************************************************************************/

ROM_START( eshockey )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-15424.ic21",   0x000000, 0x010000, CRC(e1ec8e6b) SHA1(90e106b23839cbd6ad2fe4d8a74a4fd5252a45cd) )

	ROM_REGION( 0x200, "plds", 0 )
	ROM_LOAD( "gal16v8a.ic11", 0x000, 0x117, NO_DUMP )
ROM_END

} // anonymous namespace


GAME( 1993, eshockey, 0, eshockey, eshockey, eshockey_state, empty_init, ROT0, "Sega", "Exciting Speed Hockey (V19930325)", MACHINE_IS_SKELETON_MECHANICAL )
