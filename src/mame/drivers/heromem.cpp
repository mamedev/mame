// license:BSD-3-Clause
// copyright-holders:
/*

    Heroine's Memory (c) 1997 Taito

    Coin pusher with dual screens
    Video: https://www.youtube.com/watch?v=TVSH54wozQ4

    A (possibly) one-off PCB design with the following main components:
    3 x Z80 (2 x Z0840004PSC, 1 x Z84C0008PEC, all marked Z80A on PCB)
    2 x TC0091LVC (marked as TC0090LVC on PCB)
    2 x Taito B17Z2016B (marked as YM2610B on PCB)
    2 x Taito PC060HA - near the TC0091LVCs
    2 x Taito customs with unreadable print (marked as TC0140SYT on PCB) - near the audio Z80s
    1 x MB89255B
    1 x TE7751
    1 x MACH210-15JC (undumped)
    1 x 8-dip switch
    1 x 16.0000MHz Osc

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/2610intf.h"
#include "machine/gen_latch.h"
#include "machine/i8255.h"
#include "machine/nvram.h"
#include "machine/tc009xlvc.h"
#include "machine/te7750.h"
#include "screen.h"
#include "speaker.h"

#include "audio/taitosnd.h"


namespace {

class heromem_state : public driver_device
{
public:
	heromem_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{ }

	void heromem(machine_config &config);

private:
	void maincpu_prg_map(address_map &map);
	void audiocpu_l_prg_map(address_map &map);
	void audiocpu_r_prg_map(address_map &map);
	void tc0091lvc_l_prg_map(address_map &map);
	void tc0091lvc_r_prg_map(address_map &map);
};


void heromem_state::maincpu_prg_map(address_map &map)
{
}

void heromem_state::audiocpu_l_prg_map(address_map &map)
{
}

void heromem_state::audiocpu_r_prg_map(address_map &map)
{
}

void heromem_state::tc0091lvc_l_prg_map(address_map &map)
{
}

void heromem_state::tc0091lvc_r_prg_map(address_map &map)
{
}


static INPUT_PORTS_START( heromem )
	PORT_START("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


void heromem_state::heromem(machine_config &config)
{
	// basic machine hardware
	z80_device &maincpu(Z80(config, "maincpu", 16000000 / 2)); // divider unverified
	maincpu.set_addrmap(AS_PROGRAM, &heromem_state::maincpu_prg_map);

	z80_device &audiocpu_l(Z80(config, "audiocpu_l", 16000000 / 4)); // divider unverified
	audiocpu_l.set_addrmap(AS_PROGRAM, &heromem_state::audiocpu_l_prg_map);

	z80_device &audiocpu_r(Z80(config, "audiocpu_r", 16000000 / 4)); // divider unverified
	audiocpu_r.set_addrmap(AS_PROGRAM, &heromem_state::audiocpu_r_prg_map);

	I8255(config, "ppi"); // MB89255B

	TE7750(config, "io"); // TE7751

	// video hardware
	screen_device &lscreen(SCREEN(config, "lscreen", SCREEN_TYPE_RASTER)); // all wrong
	lscreen.set_refresh_hz(60);
	lscreen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	lscreen.set_size(64*8, 32*8);
	lscreen.set_visarea(0*8, 40*8-1, 2*8, 30*8-1);
	lscreen.set_screen_update("tc0091lvc_vdp_l", FUNC(tc0091lvc_device::screen_update));
	lscreen.set_palette("tc0091lvc_vdp_l:palette");

	screen_device &rscreen(SCREEN(config, "rscreen", SCREEN_TYPE_RASTER)); // all wrong
	rscreen.set_refresh_hz(60);
	rscreen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	rscreen.set_size(64*8, 32*8);
	rscreen.set_visarea(0*8, 40*8-1, 2*8, 30*8-1);
	rscreen.set_screen_update("tc0091lvc_vdp_r", FUNC(tc0091lvc_device::screen_update));
	rscreen.set_palette("tc0091lvc_vdp_r:palette");

	pc060ha_device &ciu_l(PC060HA(config, "ciu_l", 0));
	ciu_l.set_master_tag("maincpu");
	ciu_l.set_slave_tag("tc0091lvc_l");

	pc060ha_device &ciu_r(PC060HA(config, "ciu_r", 0));
	ciu_r.set_master_tag("maincpu");
	ciu_r.set_slave_tag("tc0091lvc_r");

	z80_device &vdp_l(Z80(config, "tc0091lvc_l", 16000000 / 4)); // MAME'S TC0091LVC emulation isn't currently derived from the Z80, so needs both
	vdp_l.set_addrmap(AS_PROGRAM, &heromem_state::tc0091lvc_l_prg_map);
	TC0091LVC(config, "tc0091lvc_vdp_l", 0);

	z80_device &vdp_r(Z80(config, "tc0091lvc_r", 16000000 / 4)); // MAME'S TC0091LVC emulation isn't currently derived from the Z80, so needs both
	vdp_r.set_addrmap(AS_PROGRAM, &heromem_state::tc0091lvc_r_prg_map);
	TC0091LVC(config, "tc0091lvc_vdp_r", 0);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	tc0140syt_device &syt_l(TC0140SYT(config, "tc0140syt_l", 0));
	syt_l.set_master_tag("maincpu");
	syt_l.set_slave_tag("audiocpu_l");

	tc0140syt_device &syt_r(TC0140SYT(config, "tc0140syt_r", 0));
	syt_r.set_master_tag("maincpu");
	syt_r.set_slave_tag("audiocpu_r");

	YM2610B(config, "ym_l", 16000000 / 2);

	YM2610B(config, "ym_r", 16000000 / 2);
}



ROM_START( heromem )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "e34-06.ic66", 0x00000, 0x10000, CRC(334c0e4c) SHA1(346c8fddc6a47123c034678295f816b96d934b27) ) // H MemoryTAITO CORP string

	ROM_REGION( 0x10000, "audiocpu_l", 0 ) // both audio CPU have the same ROM content
	ROM_LOAD( "e34-04.ic32", 0x00000, 0x10000, CRC(f9b66d64) SHA1(c998b0e2ec6659e3addbcc1602ae62871e010c7e) )

	ROM_REGION( 0x10000, "audiocpu_r", 0 )
	ROM_LOAD( "e34-04.ic18", 0x00000, 0x10000, CRC(f9b66d64) SHA1(c998b0e2ec6659e3addbcc1602ae62871e010c7e) )

	ROM_REGION( 0x80000, "tc0091lvc_vdp_l", 0 ) // both TC0091LVC have the same ROM content
	ROM_LOAD( "e34-07.ic40", 0x00000, 0x80000, CRC(7f4d2664) SHA1(6d249f1e5f341da5923b45c2863ee418bb057586) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x200000, "tc0091lvc_vdp_l:gfx", 0 ) // marked LV-CHR0-3 on PCB (LV probably stands for left video)
	ROM_LOAD( "e34-08.ic41", 0x000000, 0x80000, CRC(a8c572f8) SHA1(f98de6a9eaa49e037f02f9e56da9edbebc535cd7) )
	ROM_LOAD( "e34-09.ic43", 0x080000, 0x80000, CRC(2c8849b7) SHA1(090ab881b0a98b8b1522282f46b70edeb83681d9) )
	ROM_LOAD( "e34-10.ic42", 0x100000, 0x80000, CRC(e7986216) SHA1(43fea3f1c80f9e7e051e9321d8d28e9ce5ae22f3) )
	ROM_LOAD( "e34-11.ic44", 0x180000, 0x80000, CRC(4da5904d) SHA1(280a63444af25d143c7543607cd942d0ffc33a56) )

	ROM_REGION( 0x80000, "tc0091lvc_vdp_r", 0 )
	ROM_LOAD( "e34-07.ic54", 0x00000, 0x80000, CRC(7f4d2664) SHA1(6d249f1e5f341da5923b45c2863ee418bb057586) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x200000, "tc0091lvc_vdp_r:gfx", 0 ) // marked RV-CHR0-3 on PCB (RV probably stands for right video)
	ROM_LOAD( "e34-08.ic55", 0x000000, 0x80000, CRC(a8c572f8) SHA1(f98de6a9eaa49e037f02f9e56da9edbebc535cd7) )
	ROM_LOAD( "e34-09.ic57", 0x080000, 0x80000, CRC(2c8849b7) SHA1(090ab881b0a98b8b1522282f46b70edeb83681d9) )
	ROM_LOAD( "e34-10.ic56", 0x100000, 0x80000, CRC(e7986216) SHA1(43fea3f1c80f9e7e051e9321d8d28e9ce5ae22f3) )
	ROM_LOAD( "e34-11.ic58", 0x180000, 0x80000, CRC(4da5904d) SHA1(280a63444af25d143c7543607cd942d0ffc33a56) )

	ROM_REGION( 0x200000, "ym_l", 0 ) // marked LS-PCM0 to LS-PCM3 on PCB (LS probably stands for left sound), same ROM content for the two YMs
	ROM_LOAD( "e34-12.ic29", 0x000000, 0x80000, CRC(7bb1f476) SHA1(c06c27a2c59953f9ff1eb7679257970fd9c346a3) )
	ROM_LOAD( "e34-13.ic27", 0x080000, 0x80000, CRC(a43e6cc0) SHA1(090f8f3977c99687dd8461382d0b552c4c3deb9f) )
	ROM_LOAD( "e34-14.ic30", 0x100000, 0x80000, CRC(0fce5b29) SHA1(aeb626ecead85c5ca926763d928df9eca73acca3) )
	ROM_LOAD( "e34-15.ic28", 0x180000, 0x80000, CRC(d2403bdd) SHA1(61be189a92c7c5143aa4a06d9bbfc667dd737fd8) )

	ROM_REGION( 0x200000, "ym_r", 0 ) // marked RS-PCM0 to RS-PCM3 on PCB (RS probably stands for right sound)
	ROM_LOAD( "e34-12.ic15", 0x000000, 0x80000, CRC(7bb1f476) SHA1(c06c27a2c59953f9ff1eb7679257970fd9c346a3) )
	ROM_LOAD( "e34-13.ic13", 0x080000, 0x80000, CRC(a43e6cc0) SHA1(090f8f3977c99687dd8461382d0b552c4c3deb9f) )
	ROM_LOAD( "e34-14.ic16", 0x100000, 0x80000, CRC(0fce5b29) SHA1(aeb626ecead85c5ca926763d928df9eca73acca3) )
	ROM_LOAD( "e34-15.ic14", 0x180000, 0x80000, CRC(d2403bdd) SHA1(61be189a92c7c5143aa4a06d9bbfc667dd737fd8) )

	ROM_REGION( 0x400, "plds", 0 ) // both PAL16L8A, same label, why different content?
	ROM_LOAD( "e34-01.ic46", 0x000, 0x104, CRC(96f2e73e) SHA1(42d82bc4fb30aecb2aa70959d1df92847ca02913) )
	ROM_LOAD( "e34-01.ic60", 0x200, 0x104, CRC(fb47c21a) SHA1(3fca687545cb8fbc620c63e9558af48231c8cbfd) )
ROM_END

} // Anonymous namespace


GAME( 1997, heromem, 0, heromem, heromem, heromem_state, empty_init, ROT0, "Taito", "Heroine's Memory", MACHINE_IS_SKELETON_MECHANICAL ) // video is emulatable, coin pushing mechanics less so
