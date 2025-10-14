// license:BSD-3-Clause
// copyright-holders:

/*
Ganso Janken Man 2
Pretty Present
'S9405021 CPU-DM-NEW' PCB

Z0840004PSC
4.000 MHz XTAL
LH0082A Z80A-CTC-D
CXK5816PS-15L SRAM
2 banks of 8 DIP switches
*/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/z80ctc.h"
#include "machine/z80daisy.h"
#include "sound/dac.h"

#include "speaker.h"


namespace {

class jankmg2_state : public driver_device
{
public:
	jankmg2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void jankmg2(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<z80_device> m_maincpu;

	void program_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
};


void jankmg2_state::machine_start()
{
}


void jankmg2_state::program_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x47ff).ram();
}

void jankmg2_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	//map(0x??, 0x??).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
}


static INPUT_PORTS_START( jankmg2 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static const z80_daisy_config daisy_chain[] =
{
	{ "ctc" },
	{ nullptr }
};


void jankmg2_state::jankmg2(machine_config &config)
{
	Z80(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->set_daisy_config(daisy_chain);
	m_maincpu->set_addrmap(AS_PROGRAM, &jankmg2_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &jankmg2_state::io_map);

	z80ctc_device& ctc(Z80CTC(config, "ctc", 4_MHz_XTAL));
	ctc.intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	// sound hardware
	SPEAKER(config, "speaker").front_center();

	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.5); // TODO: wrong type, verify
}


ROM_START( jankmg2 )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "gjv1.1.p.u17", 0x0000, 0x4000, CRC(1aa201e7) SHA1(943dff75a46775dd12aed07659dff3d961a58211) )

	ROM_REGION( 0x40000, "dac_data", 0 )
	ROM_LOAD( "pcg2.1.u13", 0x00000, 0x40000, CRC(48a8f769) SHA1(656346ca0a83fd8ff5c8683152e4c5e1a1c797fa) ) // same as sunwise/jankenm.cpp
	// 3 more unpopulated ROM spaces at u14, u15 and u16
ROM_END

ROM_START( ppresent )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "ppr1.u17", 0x0000, 0x4000, CRC(c4a38d1c) SHA1(cfab8e494794ecdb7ac3418b5e71b7c028656456) )

	ROM_REGION( 0x40000, "dac_data", 0 )
	ROM_LOAD( "ppr2.u13", 0x00000, 0x40000, CRC(9757383d) SHA1(33204b82f6f3c14b42fad743f62764730c05d038) )
	// 3 more unpopulated ROM spaces at u14, u15 and u16
ROM_END

} // anonymous namespace


GAME( 199?, jankmg2,  0, jankmg2, jankmg2, jankmg2_state, empty_init, ROT0, "Sunwise", "Ganso Janken Man 2", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 199?, ppresent, 0, jankmg2, jankmg2, jankmg2_state, empty_init, ROT0, "Sunwise", "Pretty Present",     MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
