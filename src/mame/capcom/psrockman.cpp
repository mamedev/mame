// license:BSD-3-Clause
// copyright-holders:

/*
PANIC SHOT ROCKMAN - カプコン パニックショットロックマン
MECHANICAL GAME (pinball / medal game hybrid)
1992
D-CPC-ROCKMAN-02

===

EPROMS

D27C010A

D27C512

===

PLD
PALACE16V8H-25PC/4

===

OKI M6295

YAMAHA YM2203C

ZILOG Z80 CPU
*/


#include "emu.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "cpu/z80/z80.h"
#include "sound/okim6295.h"
#include "sound/ymopn.h"


namespace {

class psrockman_state : public driver_device
{
public:
	psrockman_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag)
	{}


	void psrockman(machine_config &config);

private:
	void io_map(address_map &map) ATTR_COLD;
	void prg_map(address_map &map) ATTR_COLD;
};


void psrockman_state::prg_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xd000, 0xd000).portr("IN0"); // TODO: guess, 0x40 seems to make the call attendant speech stutter/restart, also .w()?
	//map(0xd008, 0xd008).rw();
	//map(0xd010, 0xd010).rw();
	//map(0xd018, 0xd018).w();
	//map(0xd020, 0xd020).w();
	//map(0xd038, 0xd038).w();
	map(0xe000, 0xffff).ram();
}

void psrockman_state::io_map(address_map &map) // TODO: only guesses
{
	map.global_mask(0xff);

	map(0x00, 0x01).rw("ym", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0x40, 0x40).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}


static INPUT_PORTS_START( psrockman ) // 2 8-dip blocks
	PORT_START("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

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


void psrockman_state::psrockman(machine_config &config)
{
	z80_device &maincpu(Z80(config, "maincpu", 3.579545_MHz_XTAL));
	maincpu.set_addrmap(AS_PROGRAM, &psrockman_state::prg_map);
	maincpu.set_addrmap(AS_IO, &psrockman_state::io_map);

// TODO: flyer picture shows at least two LED displays

	SPEAKER(config, "mono").front_center();

	ym2203_device &ym(YM2203(config, "ym", 3.579545_MHz_XTAL));
	ym.port_a_read_callback().set_ioport("DSW1"); // guess
	ym.port_b_read_callback().set_ioport("DSW2"); // guess
	ym.irq_handler().set_inputline("maincpu", 0); // TODO: with this the game says 係員をお呼びください (Please call staff) over and over
	ym.add_route(ALL_OUTPUTS, "mono", 0.5);

	OKIM6295(config, "oki", 1056000, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.5); // clock frequency & pin 7 not verified
}


ROM_START( psrockmn )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "prm0-1b-rom1.bin", 0x00000, 0x10000, CRC(9d840391) SHA1(84d90a5fb7901e18f74cb5acb6dd933023f2836f) )

	ROM_REGION(0x40000, "oki", 0)
	ROM_LOAD( "rockman-f188-rom2.bin", 0x00000, 0x20000, CRC(2ade4f1d) SHA1(e664fbe9d0c4013af3f765a8c52757e8e5ad2449) )
	ROM_LOAD( "rockman-9d48-rom3.bin", 0x20000, 0x20000, CRC(d47373ef) SHA1(a595b09ae75b7dae6ad2332e063fedc3b450e74e) )

	ROM_REGION(0x117, "plds", 0)
	ROM_LOAD( "rock-pal1.bin", 0x000, 0x117, CRC(50688629) SHA1(c7ced85561ff86c6c5a348765d8b3b6eef21554b) )
ROM_END

} // Anonymous namespace


GAME( 1992, psrockmn, 0, psrockman, psrockman, psrockman_state, empty_init, ROT0, "Capcom", "Panic Shot Rockman", MACHINE_IS_SKELETON_MECHANICAL )
