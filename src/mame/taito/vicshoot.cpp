// license:BSD-3-Clause
// copyright-holders:

/*
    Victory Shoot (c) 1995 Taito
    MAIN PCB-B (stickered K11J0817A VICTORY SHOOT)

    Main components:

    2 x Z80A
    1 x PC060HA CIU
    1 x YM2203
    2 x OKIM6295
    2 x TC5563 CMOS static RAM
    2 x M66500 programmable buffered I/O expander
    1 x 8 MHz XTAL
    2 x 8-dip banks
*/

#include "emu.h"

#include "taitosnd.h"

#include "cpu/z80/z80.h"
#include "sound/okim6295.h"
#include "sound/ymopn.h"

#include "speaker.h"


namespace {

class vicshoot_state : public driver_device
{
public:
	vicshoot_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void vicshoot(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;

	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


void vicshoot_state::main_map(address_map &map) // TODO: verify everything
{
	map(0x0000, 0x3fff).rom().region("maincpu", 0);
	map(0xc000, 0xdfff).ram();
	// map(0xe000, 0xe00f) // ?
	map(0xf000, 0xf000).w("ciu", FUNC(pc060ha_device::master_port_w));
	map(0xf001, 0xf001).rw("ciu", FUNC(pc060ha_device::master_comm_r), FUNC(pc060ha_device::master_comm_w));
}

void vicshoot_state::sound_map(address_map &map) // TODO: verify everything
{
	map(0x0000, 0x7fff).rom().region("audiocpu", 0);
	map(0xc000, 0xdfff).ram();
	map(0xe000, 0xe000).w("ciu", FUNC(pc060ha_device::slave_port_w));
	map(0xe001, 0xe001).rw("ciu", FUNC(pc060ha_device::slave_comm_r), FUNC(pc060ha_device::slave_comm_w));
	map(0xe800, 0xe801).rw("opn", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0xf000, 0xf000).rw("oki0", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xf800, 0xf800).rw("oki1", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}


static INPUT_PORTS_START( vicshoot )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW2:8")
INPUT_PORTS_END


void vicshoot_state::vicshoot(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 8_MHz_XTAL / 2); // divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &vicshoot_state::main_map);

	z80_device &audiocpu(Z80(config, "audiocpu", 8_MHz_XTAL / 2)); // divider not verified
	audiocpu.set_addrmap(AS_PROGRAM, &vicshoot_state::sound_map);

	// M66500(config, "io0");
	// M66500(config, "io1");

	// sound hardware
	SPEAKER(config, "mono").front_center();

	pc060ha_device &ciu(PC060HA(config, "ciu", 0));
	ciu.nmi_callback().set_inputline("audiocpu", INPUT_LINE_NMI);
	ciu.reset_callback().set_inputline("audiocpu", INPUT_LINE_RESET);

	ym2203_device &opn(YM2203(config, "opn", 8_MHz_XTAL / 2)); // divider not verified
	opn.irq_handler().set_inputline("audiocpu", 0);
	opn.add_route(ALL_OUTPUTS, "mono", 0.30);

	OKIM6295(config, "oki0", 1.056_MHz_XTAL, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.5);

	OKIM6295(config, "oki1", 1.056_MHz_XTAL, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.5);
}


ROM_START( vicshoot )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "d97_03.mainprg.ic27", 0x00000, 0x10000, CRC(df845776) SHA1(f5fe4430218de2a3f92f8b0a299152cf264b3c89) ) // 11xxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "d97_04.sndprg.ic46", 0x00000, 0x10000, CRC(d95ebcc4) SHA1(41e1239495092af4acc91b36a7d9beb7206a00bc) ) // 1xxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x40000, "oki0", 0 ) // two ROMs with identical content
	ROM_LOAD( "d97_05.1padpcm.ic41", 0x00000, 0x40000, CRC(1491186c) SHA1(e51618a47526f2e3c78e74652a9bb848fc692210) )

	ROM_REGION( 0x40000, "oki1", 0 )
	ROM_LOAD( "d97_05.2padpcm.ic40", 0x00000, 0x40000, CRC(1491186c) SHA1(e51618a47526f2e3c78e74652a9bb848fc692210) )

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "d97_02.ic43", 0x000, 0x104, NO_DUMP ) // PAL16L8A
	ROM_LOAD( "d97_0x.ic13", 0x200, 0x144, NO_DUMP ) // unreadable marking, PAL16L8A
ROM_END

} // anonymous namespace


GAME( 1995, vicshoot, 0, vicshoot, vicshoot, vicshoot_state, empty_init, ROT0, "Taito Corporation", "Victory Shoot", MACHINE_NOT_WORKING | MACHINE_REQUIRES_ARTWORK ) // a website lists it as 1994, but it was publicized on 1995 magazines. Going with the latter for now
