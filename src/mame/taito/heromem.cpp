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

TODO:
- currently stuck at 'communication error'.
- everything is guesswork and should be verified.

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/timer.h"
#include "sound/ymopn.h"
#include "machine/gen_latch.h"
#include "machine/i8255.h"
#include "machine/nvram.h"
#include "machine/tc009xlvc.h"
#include "machine/te7750.h"
#include "screen.h"
#include "speaker.h"

#include "taitosnd.h"


namespace {

class heromem_state : public driver_device
{
public:
	heromem_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_tc0091lvc_l(*this, "tc0091lvc_l"),
		m_tc0091lvc_r(*this, "tc0091lvc_r")
	{ }

	void heromem(machine_config &config);

private:
	required_device<tc0091lvc_device> m_tc0091lvc_l;
	required_device<tc0091lvc_device> m_tc0091lvc_r;

	void maincpu_prg_map(address_map &map) ATTR_COLD;
	void audiocpu_l_prg_map(address_map &map) ATTR_COLD;
	void audiocpu_r_prg_map(address_map &map) ATTR_COLD;
	void tc0091lvc_l_prg_map(address_map &map) ATTR_COLD;
	void tc0091lvc_r_prg_map(address_map &map) ATTR_COLD;

	void screen_vblank_l(int state) { if (state) { m_tc0091lvc_l->screen_eof(); } }
	void screen_vblank_r(int state) { if (state) { m_tc0091lvc_l->screen_eof(); } }

	TIMER_DEVICE_CALLBACK_MEMBER(irq_scanline_l);
	TIMER_DEVICE_CALLBACK_MEMBER(irq_scanline_r);
};


TIMER_DEVICE_CALLBACK_MEMBER(heromem_state::irq_scanline_l)
{
	int scanline = param;

	if (scanline == 240 && (m_tc0091lvc_l->irq_enable() & 4))
	{
		m_tc0091lvc_l->set_input_line_and_vector(0, HOLD_LINE, m_tc0091lvc_l->irq_vector(2));
	}

	if (scanline == 0 && (m_tc0091lvc_l->irq_enable() & 2))
	{
		m_tc0091lvc_l->set_input_line_and_vector(0, HOLD_LINE, m_tc0091lvc_l->irq_vector(1));
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(heromem_state::irq_scanline_r)
{
	int scanline = param;

	if (scanline == 240 && (m_tc0091lvc_r->irq_enable() & 4))
	{
		m_tc0091lvc_r->set_input_line_and_vector(0, HOLD_LINE, m_tc0091lvc_r->irq_vector(2));
	}

	if (scanline == 0 && (m_tc0091lvc_r->irq_enable() & 2))
	{
		m_tc0091lvc_r->set_input_line_and_vector(0, HOLD_LINE, m_tc0091lvc_r->irq_vector(1));
	}
}


void heromem_state::maincpu_prg_map(address_map &map)
{
	map(0x0000, 0xafff).rom().region("maincpu", 0);
	map(0xd000, 0xefff).ram();
	// TODO: from here on it needs verification
	map(0xf000, 0xf000).nopr().w("ciu_l", FUNC(pc060ha_device::master_port_w));
	map(0xf001, 0xf001).rw("ciu_l", FUNC(pc060ha_device::master_comm_r), FUNC(pc060ha_device::master_comm_w));
	map(0xf100, 0xf100).nopr().w("ciu_r", FUNC(pc060ha_device::master_port_w));
	map(0xf101, 0xf101).rw("ciu_r", FUNC(pc060ha_device::master_comm_r), FUNC(pc060ha_device::master_comm_w));
	map(0xf200, 0xf200).w("tc0140syt_l", FUNC(tc0140syt_device::master_port_w));
	map(0xf201, 0xf201).rw("tc0140syt_l", FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
	map(0xf300, 0xf300).w("tc0140syt_r", FUNC(tc0140syt_device::master_port_w));
	map(0xf301, 0xf301).rw("tc0140syt_r", FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
	map(0xf400, 0xf403).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xf500, 0xf50f).rw("io", FUNC(te7751_device::read), FUNC(te7751_device::write));
	// TODO: lots more writes
}

void heromem_state::audiocpu_l_prg_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("audiocpu_l", 0);
	// TODO: banking?
	map(0xc000, 0xdfff).ram();
	// TODO: from here on verify when it works
	map(0xe000, 0xe003).rw("ym_l", FUNC(ym2610b_device::read), FUNC(ym2610b_device::write));
	map(0xe200, 0xe200).w("tc0140syt_l", FUNC(tc0140syt_device::slave_port_w));
	map(0xe201, 0xe201).rw("tc0140syt_l", FUNC(tc0140syt_device::slave_comm_r), FUNC(tc0140syt_device::slave_comm_w));
	// TODO: lots more writes
}

void heromem_state::audiocpu_r_prg_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("audiocpu_r", 0);
	// TODO: banking?
	map(0xc000, 0xdfff).ram();
	// TODO: from here on verify when it works
	map(0xe000, 0xe003).rw("ym_r", FUNC(ym2610b_device::read), FUNC(ym2610b_device::write));
	map(0xe200, 0xe200).w("tc0140syt_r", FUNC(tc0140syt_device::slave_port_w));
	map(0xe201, 0xe201).rw("tc0140syt_r", FUNC(tc0140syt_device::slave_comm_r), FUNC(tc0140syt_device::slave_comm_w));
	// TODO: lots more writes
}

void heromem_state::tc0091lvc_l_prg_map(address_map &map)
{
	map(0x8000, 0x9fff).ram();
	// TODO: where is the PC060HA hooked up?
	//map(0xa004, 0xa004).nopr().w("ciu_l", FUNC(pc060ha_device::slave_port_w));
	//map(0xa005, 0xa005).rw("ciu_l", FUNC(pc060ha_device::slave_comm_r), FUNC(pc060ha_device::slave_comm_w));
	map(0xfe00, 0xfeff).rw(m_tc0091lvc_l, FUNC(tc0091lvc_device::vregs_r), FUNC(tc0091lvc_device::vregs_w));
	map(0xff00, 0xff02).rw(m_tc0091lvc_l, FUNC(tc0091lvc_device::irq_vector_r), FUNC(tc0091lvc_device::irq_vector_w));
	map(0xff03, 0xff03).rw(m_tc0091lvc_l, FUNC(tc0091lvc_device::irq_enable_r), FUNC(tc0091lvc_device::irq_enable_w));
	map(0xff04, 0xff07).rw(m_tc0091lvc_l, FUNC(tc0091lvc_device::ram_bank_r), FUNC(tc0091lvc_device::ram_bank_w));
	map(0xff08, 0xff08).rw(m_tc0091lvc_l, FUNC(tc0091lvc_device::rom_bank_r), FUNC(tc0091lvc_device::rom_bank_w));
}

void heromem_state::tc0091lvc_r_prg_map(address_map &map)
{
	map(0x8000, 0x9fff).ram();
	// TODO: where is the PC060HA hooked up?
	//map(0xa004, 0xa004).nopr().w("ciu_r", FUNC(pc060ha_device::slave_port_w));
	//map(0xa005, 0xa005).rw("ciu_r", FUNC(pc060ha_device::slave_comm_r), FUNC(pc060ha_device::slave_comm_w));
	map(0xfe00, 0xfeff).rw(m_tc0091lvc_r, FUNC(tc0091lvc_device::vregs_r), FUNC(tc0091lvc_device::vregs_w));
	map(0xff00, 0xff02).rw(m_tc0091lvc_r, FUNC(tc0091lvc_device::irq_vector_r), FUNC(tc0091lvc_device::irq_vector_w));
	map(0xff03, 0xff03).rw(m_tc0091lvc_r, FUNC(tc0091lvc_device::irq_enable_r), FUNC(tc0091lvc_device::irq_enable_w));
	map(0xff04, 0xff07).rw(m_tc0091lvc_r, FUNC(tc0091lvc_device::ram_bank_r), FUNC(tc0091lvc_device::ram_bank_w));
	map(0xff08, 0xff08).rw(m_tc0091lvc_r, FUNC(tc0091lvc_device::rom_bank_r), FUNC(tc0091lvc_device::rom_bank_w));
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

	TIMER(config, "scantimer_l").configure_scanline(FUNC(heromem_state::irq_scanline_l), "lscreen", 0, 1);
	TIMER(config, "scantimer_r").configure_scanline(FUNC(heromem_state::irq_scanline_r), "rscreen", 0, 1);

	I8255(config, "ppi"); // MB89255B

	TE7751(config, "io");

	// video hardware
	screen_device &lscreen(SCREEN(config, "lscreen", SCREEN_TYPE_RASTER)); // all wrong
	lscreen.set_refresh_hz(60);
	lscreen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	lscreen.set_size(64*8, 32*8);
	lscreen.set_visarea(0*8, 40*8-1, 2*8, 30*8-1);
	lscreen.set_screen_update(m_tc0091lvc_l, FUNC(tc0091lvc_device::screen_update));
	lscreen.screen_vblank().set(FUNC(heromem_state::screen_vblank_l));
	lscreen.set_palette("tc0091lvc_l:palette");

	screen_device &rscreen(SCREEN(config, "rscreen", SCREEN_TYPE_RASTER)); // all wrong
	rscreen.set_refresh_hz(60);
	rscreen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	rscreen.set_size(64*8, 32*8);
	rscreen.set_visarea(0*8, 40*8-1, 2*8, 30*8-1);
	rscreen.set_screen_update(m_tc0091lvc_r, FUNC(tc0091lvc_device::screen_update));
	rscreen.screen_vblank().set(FUNC(heromem_state::screen_vblank_r));
	rscreen.set_palette("tc0091lvc_r:palette");

	pc060ha_device &ciu_l(PC060HA(config, "ciu_l", 0));
	ciu_l.nmi_callback().set_inputline(m_tc0091lvc_l, INPUT_LINE_NMI);
	ciu_l.reset_callback().set_inputline(m_tc0091lvc_l, INPUT_LINE_RESET);

	pc060ha_device &ciu_r(PC060HA(config, "ciu_r", 0));
	ciu_r.nmi_callback().set_inputline(m_tc0091lvc_r, INPUT_LINE_NMI);
	ciu_r.reset_callback().set_inputline(m_tc0091lvc_r, INPUT_LINE_RESET);

	tc0091lvc_device &vdp_l(TC0091LVC(config, m_tc0091lvc_l, 16000000 / 4));
	vdp_l.set_addrmap(AS_PROGRAM, &heromem_state::tc0091lvc_l_prg_map);

	tc0091lvc_device &vdp_r(TC0091LVC(config, m_tc0091lvc_r, 16000000 / 4));
	vdp_r.set_addrmap(AS_PROGRAM, &heromem_state::tc0091lvc_r_prg_map);

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();

	SPEAKER(config, "rspeaker").front_right();

	tc0140syt_device &syt_l(TC0140SYT(config, "tc0140syt_l", 0));
	syt_l.nmi_callback().set_inputline("audiocpu_l", INPUT_LINE_NMI);
	syt_l.reset_callback().set_inputline("audiocpu_l", INPUT_LINE_RESET);

	tc0140syt_device &syt_r(TC0140SYT(config, "tc0140syt_r", 0));
	syt_r.nmi_callback().set_inputline("audiocpu_r", INPUT_LINE_NMI);
	syt_r.reset_callback().set_inputline("audiocpu_r", INPUT_LINE_RESET);

	ym2610b_device &ym_l(YM2610B(config, "ym_l", 16000000 / 2));
	ym_l.irq_handler().set_inputline("audiocpu_l", 0);
	ym_l.add_route(0, "lspeaker", 0.25);
	ym_l.add_route(0, "lspeaker", 0.25);
	ym_l.add_route(1, "lspeaker", 1.0);
	ym_l.add_route(2, "lspeaker", 1.0);

	ym2610b_device &ym_r(YM2610B(config, "ym_r", 16000000 / 2));
	ym_r.irq_handler().set_inputline("audiocpu_r", 0);
	ym_r.add_route(0, "rspeaker", 0.25);
	ym_r.add_route(0, "rspeaker", 0.25);
	ym_r.add_route(1, "rspeaker", 1.0);
	ym_r.add_route(2, "rspeaker", 1.0);
}



ROM_START( heromem )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "e34-06.ic66", 0x00000, 0x10000, CRC(334c0e4c) SHA1(346c8fddc6a47123c034678295f816b96d934b27) ) // H MemoryTAITO CORP string

	ROM_REGION( 0x10000, "audiocpu_l", 0 ) // both audio CPU have the same ROM content
	ROM_LOAD( "e34-04.ic32", 0x00000, 0x10000, CRC(f9b66d64) SHA1(c998b0e2ec6659e3addbcc1602ae62871e010c7e) )

	ROM_REGION( 0x10000, "audiocpu_r", 0 )
	ROM_LOAD( "e34-04.ic18", 0x00000, 0x10000, CRC(f9b66d64) SHA1(c998b0e2ec6659e3addbcc1602ae62871e010c7e) )

	ROM_REGION( 0x80000, "tc0091lvc_l", 0 ) // both TC0091LVC have the same ROM content
	ROM_LOAD( "e34-07.ic40", 0x00000, 0x80000, CRC(7f4d2664) SHA1(6d249f1e5f341da5923b45c2863ee418bb057586) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x200000, "tc0091lvc_l:gfx", 0 ) // marked LV-CHR0-3 on PCB (LV probably stands for left video)
	ROM_LOAD16_BYTE( "e34-08.ic41", 0x000000, 0x80000, CRC(a8c572f8) SHA1(f98de6a9eaa49e037f02f9e56da9edbebc535cd7) )
	ROM_LOAD16_BYTE( "e34-09.ic43", 0x000001, 0x80000, CRC(2c8849b7) SHA1(090ab881b0a98b8b1522282f46b70edeb83681d9) )
	ROM_LOAD16_BYTE( "e34-10.ic42", 0x100000, 0x80000, CRC(e7986216) SHA1(43fea3f1c80f9e7e051e9321d8d28e9ce5ae22f3) )
	ROM_LOAD16_BYTE( "e34-11.ic44", 0x100001, 0x80000, CRC(4da5904d) SHA1(280a63444af25d143c7543607cd942d0ffc33a56) )

	ROM_REGION( 0x80000, "tc0091lvc_r", 0 )
	ROM_LOAD( "e34-07.ic54", 0x00000, 0x80000, CRC(7f4d2664) SHA1(6d249f1e5f341da5923b45c2863ee418bb057586) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x200000, "tc0091lvc_r:gfx", 0 ) // marked RV-CHR0-3 on PCB (RV probably stands for right video)
	ROM_LOAD16_BYTE( "e34-08.ic55", 0x000000, 0x80000, CRC(a8c572f8) SHA1(f98de6a9eaa49e037f02f9e56da9edbebc535cd7) )
	ROM_LOAD16_BYTE( "e34-09.ic57", 0x000001, 0x80000, CRC(2c8849b7) SHA1(090ab881b0a98b8b1522282f46b70edeb83681d9) )
	ROM_LOAD16_BYTE( "e34-10.ic56", 0x100000, 0x80000, CRC(e7986216) SHA1(43fea3f1c80f9e7e051e9321d8d28e9ce5ae22f3) )
	ROM_LOAD16_BYTE( "e34-11.ic58", 0x100001, 0x80000, CRC(4da5904d) SHA1(280a63444af25d143c7543607cd942d0ffc33a56) )

	ROM_REGION( 0x200000, "ym_l:adpcma", 0 ) // marked LS-PCM0 to LS-PCM3 on PCB (LS probably stands for left sound), same ROM content for the two YMs
	ROM_LOAD( "e34-12.ic29", 0x000000, 0x80000, CRC(7bb1f476) SHA1(c06c27a2c59953f9ff1eb7679257970fd9c346a3) )
	ROM_LOAD( "e34-13.ic27", 0x080000, 0x80000, CRC(a43e6cc0) SHA1(090f8f3977c99687dd8461382d0b552c4c3deb9f) )
	ROM_LOAD( "e34-14.ic30", 0x100000, 0x80000, CRC(0fce5b29) SHA1(aeb626ecead85c5ca926763d928df9eca73acca3) )
	ROM_LOAD( "e34-15.ic28", 0x180000, 0x80000, CRC(d2403bdd) SHA1(61be189a92c7c5143aa4a06d9bbfc667dd737fd8) )

	ROM_REGION( 0x200000, "ym_r:adpcma", 0 ) // marked RS-PCM0 to RS-PCM3 on PCB (RS probably stands for right sound)
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
