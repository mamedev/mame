// license:BSD-3-Clause
// copyright-holders:
/*
Namco System 1 bootleg hardware

While obviously based on the original, the hardware differs quite a bit. Thus the separate driver instead of a derived class.

2 x M6809
3 x OKI M6295
2 x ACTEL FPGA
Xtal below Amp 16.00 MHz
Xtal by the 2x M6809 48.00 MHz
1 x 8 dsw bank
No Namco custom chips
*/

#include "emu.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "cpu/m6809/m6809.h"
#include "sound/okim6295.h"


namespace {

class namcos1b_state : public driver_device
{
public:
	namcos1b_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{
	}

	void namcos1b(machine_config &config);

private:
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_prg_map(address_map &map) ATTR_COLD;
	void sub_prg_map(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
};

uint32_t namcos1b_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void namcos1b_state::main_prg_map(address_map &map)
{
	map(0x0000, 0xffff).rom().region("cpus", 0);
}

void namcos1b_state::sub_prg_map(address_map &map)
{
	map(0x0000, 0xffff).rom().region("cpus", 0);
}


static INPUT_PORTS_START( namcos1b )
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


void namcos1b_state::machine_start()
{
}



void namcos1b_state::namcos1b(machine_config &config)
{
	/* basic machine hardware */
	m6809_device &maincpu(M6809(config, "maincpu", XTAL(48'000'000) / 32)); // divider guessed
	maincpu.set_addrmap(AS_PROGRAM, &namcos1b_state::main_prg_map);

	m6809_device &subcpu(M6809(config, "subcpu", XTAL(48'000'000) / 32)); // divider guessed
	subcpu.set_addrmap(AS_PROGRAM, &namcos1b_state::sub_prg_map);

	// all wrong
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 64*8-1, 0*8, 32*8-1);
	screen.set_screen_update(FUNC(namcos1b_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_entries(0x1000);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, "oki1", XTAL(16'000'000) / 16, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.50); // divider & pin 7 not verified

	OKIM6295(config, "oki2", XTAL(16'000'000) / 16, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.50); // divider & pin 7 not verified

	OKIM6295(config, "oki3", XTAL(16'000'000) / 16, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.50); // divider & pin 7 not verified
}


/* Tank Force (bootleg) */
ROM_START( tankfrceb ) // ROMs dumped from 2 different PCBs, marked KMW-2801/03/13
	ROM_REGION( 0x80000, "cpus", 0 ) // shared by both CPUs
	ROM_LOAD( "tk_r4-ug25.ug25",  0x00000, 0x80000, CRC(132cbf9a) SHA1(dda8f2e157616f21898a6cc08f19b223088abc28) )

	ROM_REGION( 0x80000, "oki1", 0 )
	ROM_LOAD( "tk_r1-su4.su4",    0x00000, 0x80000, CRC(76ec5342) SHA1(d633e9987955c3b927455a115460379c45c4777c) )

	ROM_REGION( 0x80000, "oki2", 0 )
	ROM_LOAD( "tk_r2-su5.su5",    0x00000, 0x80000, CRC(9ce0de3d) SHA1(9f98eb15801a6f57ce83bbceac95e0bc05ed07dc) ) // 1ST AND 2ND HALF IDENTICAL

	ROM_REGION( 0x80000, "oki3", 0 )
	ROM_LOAD( "tk_r3-su6.su6",    0x00000, 0x80000, CRC(d4977dce) SHA1(9eea5be150d9ff016638af727b0f52ea94cb353c) )

	ROM_REGION( 0x20000, "mask", 0 )
	ROM_LOAD( "tk_r6-rom4.010",   0x00000, 0x20000, CRC(7d53b31e) SHA1(7e4b5fc92f7956477392f1e14c6edfc0cada2be0) ) // identical to chr8

	ROM_REGION( 0x100000, "tmap", 0 )
	ROM_LOAD( "tk_r7-rom5.040",   0x00000, 0x80000, CRC(993291bf) SHA1(11ef01f613a64644bd69385464d5ed8f6d69f3a8) ) // chr0 (99.999237% match) + chr1 (identical) + chr2 (identical)  + chr3 (identical)
	ROM_LOAD( "tk_r8-rom6.020",   0x80000, 0x40000, CRC(8574efa6) SHA1(f37c6e94fe9673303f7f969ff4d579f44f90d8a3) ) // identical to chr4 + chr5

	ROM_REGION( 0x100000, "sprite", 0 )
	ROM_LOAD( "tk_r5-uj24.uj24",  0x00000, 0x40000, CRC(7e8e7852) SHA1(7a85d8b0bb3cb0366455b7f105b8f97bca045094) ) // identical to obj0 + obj1
ROM_END

} // anonymous namespace


GAME( 199?, tankfrceb, tankfrce, namcos1b, namcos1b, namcos1b_state, empty_init, ROT0, "bootleg", "Tank Force (bootleg)", MACHINE_IS_SKELETON )
