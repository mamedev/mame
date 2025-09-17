// license:BSD-3-Clause
// copyright-holders:stonedDiscord
/****************************************************************************

    Skeleton driver for Elaut Megacrane

****************************************************************************/

#include "emu.h"
#include "speaker.h"

#include "cpu/mc68hc11/mc68hc11.h"
#include "sound/ay8910.h"

namespace {

class megacrane_state : public driver_device
{
public:
	megacrane_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void megacrane(machine_config &config);

private:
	void mem_map(address_map &map) ATTR_COLD;

	required_device<mc68hc11_cpu_device> m_maincpu;
};


void megacrane_state::mem_map(address_map &map)
{
	map(0x8000, 0xffff).rom().region("program", 0x8000);

	map(0x8000, 0x8000).w("ymz", FUNC(ay8910_device::address_w));
	map(0x8001, 0x8001).w("ymz", FUNC(ay8910_device::data_w));
}


static INPUT_PORTS_START(megacrane)
	PORT_START("SW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")   // (Port A)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("IN2")   // (Port D)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN) //MISO
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN) //MOSI
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN) //SCK
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN) //CS
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)
INPUT_PORTS_END


void megacrane_state::megacrane(machine_config &config)
{
	MC68HC11E1(config, m_maincpu, 8_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &megacrane_state::mem_map);
	m_maincpu->in_pa_callback().set_ioport("IN1");
	m_maincpu->in_pe_callback().set_ioport("SW1");

	SPEAKER(config, "mono").front_center();

	YMZ284(config, "ymz", 4000000).add_route(ALL_OUTPUTS, "mono", 1.0);
}


ROM_START(megacrane)
    ROM_REGION(0x10000, "program", 0)
	ROM_LOAD("elaut_2001_eu_mg_i_02.39.07.u5", 0x0000, 0x10000, CRC(feb5cfa1) SHA1(3c091543c0419ea15a5d66d2b9602668e7c35b10))
	ROM_REGION(0x2000, "voice", 0)
	ROM_LOAD("elaut_2001_sound_megacrane.u5", 0x0000, 0x2000, NO_DUMP)
ROM_END

} // anonymous namespace


GAME( 1997, megacrane, 0,     megacrane, megacrane, megacrane_state, empty_init, ROT0, "Elaut", "Megacrane", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL)
