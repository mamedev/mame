// license:GPL-2.0+
// copyright-holders:Jarek Burczynski
/***************************************************************************

Rastan

driver by Jarek Burczynski


custom ICs
----------
PC040DA x3  video DAC
PC050       coin I/O
PC060HA     main/sub CPU communication
PC080       tilemap generator
PC090       sprite generator


memory map
----------
68000:

The address decoding is done by two PALs (IC11 and IC12) which haven't been
read, so the memory map is inferred by program behaviour

Address                  Dir Data             Name      Description
------------------------ --- ---------------- --------- -----------------------
0000000xxxxxxxxxxxxxxxx- R   xxxxxxxxxxxxxxxx ROM0      program ROM
0000001xxxxxxxxxxxxxxxx- R   xxxxxxxxxxxxxxxx ROM1      program ROM
0000010xxxxxxxxxxxxxxxx- R   xxxxxxxxxxxxxxxx ROM2      program ROM
000100--11xxxxxxxxxxxxxx R/W xxxxxxxxxxxxxxxx WLRAM/WURAM work RAM
001000------xxxxxxxxxxx- R/W xxxxxxxxxxxxxxxx CLCS      palette RAM
0011100-------------000- R   --------xxxxxxxx IN PORT   player 1 inputs
0011100-------------001- R   --------xxxxxxxx IN PORT   player 2 inputs
0011100-------------010- R   --------xxxxxxxx IN PORT   extra inputs
0011100-------------011- R   --------xxxxxxxx IN PORT   coin inputs
0011100-------------100- R   --------xxxxxxxx IN PORT   dip SW1
0011100-------------101- R   --------xxxxxxxx IN PORT   dip SW2
0011100-------------110- R   ---------------- n.c.
0011100-------------111- R   ---------------- n.c.
0011100-----------------   W --------xxx----- OUT8-10   sprite palette bank
0011100-----------------   W -----------x---- n.c.
0011100-----------------   W ------------xxxx           PC050 (coin counters, coin lockout)
0011101----------------- R   ---------------- n.c.
0011101-----------------   W --------------x- M INT     [1]
0011101-----------------   W ---------------x SUB RESET [1]
0011110----------------- R   ---------------- n.c.
0011110-----------------   W ----------------           watchdog reset
0011111---------------x- R/W ------------xxxx SNRD/SNWR PC060HA
0011                                          EXT       [1]
11000xxxxxxxxxxxxxxxxxxx R/W xxxxxxxxxxxxxxxx SCN       PC080 PGA
11010000---xxxxxxxxxxxxx R/W xxxxxxxxxxxxxxxx OBJ       PC090 PGA

[1] Not used, goes to external connector, maybe provision for an MCU?


Z80:

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
00xxxxxxxxxxxxxx R   xxxxxxxx           program ROM
01xxxxxxxxxxxxxx R   xxxxxxxx           program ROM (banked)
1000xxxxxxxxxxxx R/W xxxxxxxx SRAM      work RAM
1001-----------x R/W xxxxxxxx YM2151    YM2151 [1]
1010-----------x R/W ----xxxx PC6       PC060HA
1011------------   W xxxxxxxx V-ST-ADRS MSM5205 start address (bits 15-8)
1100------------   W -------- START-VCE MSM5205 start
1101------------   W -------- STOP-VCE  MSM5205 stop
1110------------              n.c.
1111------------              n.c.

[1] Schematics also show a YM3526 that can replace the YM2151


TODO:
- Unknown writes to 0x350008.


Stephh's notes (based on the game M68000 code and some tests) :

1) 'rastan' and 'rastanu'

  - Region stored at 0x05fffe.w
  - Sets :
      * 'rastan'  : region = 0x0001
      * 'rastanu' : region = 0x0000
  - These 2 games are 100% the same, only region differs !
    The US version has the copyright message twice in ROM though
  - Coinage relies on the region (code at 0x05ffa2) :
      * 0x0001 (World) uses TAITO_COINAGE_WORLD
      * other uses TAITO_COINAGE_JAPAN_OLD
  - Game name : "RASTAN"
  - No notice screen
  - In "demo mode", you get a scrolling screen with what the various items do
  - No beginning screen when you start a new game
  - Same "YOU ARE A BRAVE FIGHTER ..." message between levels
  - No message after beating level 6 boss
  - No copyright message on scrolling credits screen
  - Game ends after round 6
  - There was sort of debug address at 0x05ff9e.w in ROM area :
      * bits 0 to 2 determine the level (0 to 5)
      * bits 3 and 5 determine where you start at the level !
          . OFF & OFF : part 1
          . OFF & ON  : part 2
          . ON  & OFF : part 3 (boss)
          . ON  & ON  : IMPOSSIBLE !
      * bit 4 doesn't seem to have any effect
      * bit 6 is the invulnerability flag (stored at 0x10c040.w = $40,A5)
        surprisingly, it doesn't work when you fall in the water
      * bit 7 is the infinite energy flag (stored at 0x10c044.w = $44,A5)
    Be aware that the bits are active low !


2) 'rastanua'

  - Region stored at 0x05fffe.w
  - Sets :
      * 'rastanua' : region = 0xffff
  - Game uses TAITO_COINAGE_JAPAN_OLD
  - Game name : "RASTAN"
  - There was sort of debug address at 0x05fffc.w in ROM area
    See 'rastan' comments to know what the different bits do
  - Same other notes as for 'rastan'


3) 'rastsaga' and 'rastsagaa'

  - Region stored at 0x05fffe.w
  - Sets :
      * 'rastsaga'  : region = 0xffff
      * 'rastsagaa' : region = 0xffff
  - Game uses TAITO_COINAGE_JAPAN_OLD
  - Game name : "RASTAN SAGA"
  - Notice screen
  - In "demo mode", you get no scrolling screen with what the various items do
  - Beginning screen when you start a new game
  - Different messages between levels
  - Message after beating level 6 boss
  - Copyright message on scrolling credits screen
  - There was sort of debug address at 0x05fffc.w in ROM area
    See 'rastan' comments to know what the different bits do
  - Different way to handle coins insertion ? See additional code at 0x039f00
  - Same other notes as for 'rastan'

Note: The 'rastsagaa' set's ROM numbers were named as RSxx_37 through RSxx_42
      skipping RSxx_41. It's doubtful that Taito would reuse those numbers in
      2 different sets with different data/code. The names have been corrected
      to known files names where the code matched.

***************************************************************************/

#include "emu.h"

#include "taitosnd.h"
#include "taitoipt.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/74157.h"
#include "machine/watchdog.h"
#include "sound/msm5205.h"
#include "sound/ymopm.h"
#include "pc080sn.h"
#include "pc090oj.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class rastan_state : public driver_device
{
public:
	rastan_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_audiobank(*this, "audiobank"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_msm(*this, "msm"),
		m_adpcm_sel(*this, "adpcm_sel"),
		m_adpcm_data(*this, "adpcm"),
		m_pc080sn(*this, "pc080sn"),
		m_pc090oj(*this, "pc090oj")
	{ }

	void rastan(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_memory_bank m_audiobank;

	// video-related
	u16         m_sprite_ctrl;
	u16         m_sprites_flipscreen;

	// misc
	u16         m_adpcm_pos;
	bool        m_adpcm_ff;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<msm5205_device> m_msm;
	required_device<ls157_device> m_adpcm_sel;
	required_region_ptr<u8> m_adpcm_data;
	required_device<pc080sn_device> m_pc080sn;
	required_device<pc090oj_device> m_pc090oj;

	void msm5205_address_w(u8 data);
	void spritectrl_w(u16 data);
	void sound_bankswitch_w(u8 data);
	void msm5205_start_w(u8 data);
	void msm5205_stop_w(u8 data);
	void colpri_cb(u32 &sprite_colbank, u32 &pri_mask, u16 sprite_ctrl);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void msm5205_vck(int state);
	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


void rastan_state::colpri_cb(u32 &sprite_colbank, u32 &pri_mask, u16 sprite_ctrl)
{
	// bits 5-7 are the sprite palette bank
	sprite_colbank = (sprite_ctrl & 0xe0) >> 1;
	pri_mask = 0; // sprites over everything
}

void rastan_state::spritectrl_w(u16 data)
{
	m_pc090oj->sprite_ctrl_w(data);

	// bit 4 unused

	// bits 0 and 1 are coin lockout
	machine().bookkeeping().coin_lockout_w(1, ~data & 0x01);
	machine().bookkeeping().coin_lockout_w(0, ~data & 0x02);

	// bits 2 and 3 are the coin counters
	machine().bookkeeping().coin_counter_w(1, data & 0x04);
	machine().bookkeeping().coin_counter_w(0, data & 0x08);
}

u32 rastan_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_pc080sn->tilemap_update();

	screen.priority().fill(0, cliprect);

	m_pc080sn->tilemap_draw(screen, bitmap, cliprect, 0, TILEMAP_DRAW_OPAQUE, 1);
	m_pc080sn->tilemap_draw(screen, bitmap, cliprect, 1, 0, 2);

	m_pc090oj->draw_sprites(screen, bitmap, cliprect);
	return 0;
}


void rastan_state::sound_bankswitch_w(u8 data)
{
	m_audiobank->set_entry(data & 3);
}

void rastan_state::msm5205_vck(int state)
{
	if (!state)
		return;

	m_adpcm_ff = !m_adpcm_ff;
	m_adpcm_sel->select_w(m_adpcm_ff);

	if (m_adpcm_ff)
	{
		m_adpcm_sel->ba_w(m_adpcm_data[m_adpcm_pos]);
		m_adpcm_pos = (m_adpcm_pos + 1) & 0xffff;
	}
}

void rastan_state::msm5205_address_w(u8 data)
{
	m_adpcm_pos = (m_adpcm_pos & 0x00ff) | (data << 8);
}

void rastan_state::msm5205_start_w(u8 data)
{
	m_msm->reset_w(0);
	m_adpcm_ff = false;
}

void rastan_state::msm5205_stop_w(u8 data)
{
	m_msm->reset_w(1);
	m_adpcm_pos &= 0xff00;
}


void rastan_state::main_map(address_map &map)
{
	map(0x000000, 0x05ffff).rom();
	map(0x10c000, 0x10ffff).ram();
	map(0x200000, 0x200fff).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0x350008, 0x350009).nopw();    // 0 only (often) ?
	map(0x380000, 0x380001).w(FUNC(rastan_state::spritectrl_w));  // sprite palette bank, coin counters & lockout
	map(0x390000, 0x390001).portr("P1");
	map(0x390002, 0x390003).portr("P2");
	map(0x390004, 0x390005).portr("SPECIAL");
	map(0x390006, 0x390007).portr("SYSTEM");
	map(0x390008, 0x390009).portr("DSWA");
	map(0x39000a, 0x39000b).portr("DSWB");
	map(0x3c0000, 0x3c0001).w("watchdog", FUNC(watchdog_timer_device::reset16_w));
	map(0x3e0000, 0x3e0001).nopr();
	map(0x3e0001, 0x3e0001).w("ciu", FUNC(pc060ha_device::master_port_w));
	map(0x3e0003, 0x3e0003).rw("ciu", FUNC(pc060ha_device::master_comm_r), FUNC(pc060ha_device::master_comm_w));
	map(0xc00000, 0xc0ffff).rw(m_pc080sn, FUNC(pc080sn_device::word_r), FUNC(pc080sn_device::word_w));
	map(0xc20000, 0xc20003).w(m_pc080sn, FUNC(pc080sn_device::yscroll_word_w));
	map(0xc40000, 0xc40003).w(m_pc080sn, FUNC(pc080sn_device::xscroll_word_w));
	map(0xc50000, 0xc50003).w(m_pc080sn, FUNC(pc080sn_device::ctrl_word_w));
	map(0xd00000, 0xd03fff).rw(m_pc090oj, FUNC(pc090oj_device::word_r), FUNC(pc090oj_device::word_w));  // sprite ram
}


void rastan_state::sound_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x7fff).bankr(m_audiobank);
	map(0x8000, 0x8fff).ram();
	map(0x9000, 0x9001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xa000, 0xa000).w("ciu", FUNC(pc060ha_device::slave_port_w));
	map(0xa001, 0xa001).rw("ciu", FUNC(pc060ha_device::slave_comm_r), FUNC(pc060ha_device::slave_comm_w));
	map(0xb000, 0xb000).w(FUNC(rastan_state::msm5205_address_w));
	map(0xc000, 0xc000).w(FUNC(rastan_state::msm5205_start_w));
	map(0xd000, 0xd000).w(FUNC(rastan_state::msm5205_stop_w));
}



static INPUT_PORTS_START( rastan )
	PORT_START("P1")
	TAITO_JOY_UDLR_2_BUTTONS( 1 )

	PORT_START("P2")
	TAITO_JOY_UDLR_2_BUTTONS( 2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SPECIAL")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM )                // from PC050 (coin A gets locked if 0)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM )                // from PC050 (coin B gets locked if 0)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM )                // from PC050 (above 2 bits not checked when 0)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	// 0x390008 -> 0x10c018 ($18,A5)
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x04, IP_ACTIVE_LOW, "SW1:3" )    // Normally Demo Sound, but not used
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW1:4" )
	TAITO_COINAGE_WORLD_LOC(SW1)

	// 0x39000a -> 0x10c01c ($1c,A5)
	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:3,4")           // table at 0x059f2e
	PORT_DIPSETTING(    0x0c, "100k 200k 400k 600k 800k" )
	PORT_DIPSETTING(    0x08, "150k 300k 600k 900k 1200k" )
	PORT_DIPSETTING(    0x04, "200k 400k 800k 1200k 1600k" )
	PORT_DIPSETTING(    0x00, "250k 500k 1000k 1500k 2000k" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( rastsaga )
	PORT_INCLUDE( rastan )

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_OLD_LOC(SW1)
INPUT_PORTS_END


static GFXDECODE_START( gfx_rastan )
	GFXDECODE_ENTRY( "pc080sn", 0, gfx_8x8x4_packed_msb, 0, 0x80 )
GFXDECODE_END


void rastan_state::machine_start()
{
	m_audiobank->configure_entries(0, 4, memregion("audiocpu")->base(), 0x4000);

	save_item(NAME(m_sprite_ctrl));
	save_item(NAME(m_sprites_flipscreen));

	save_item(NAME(m_adpcm_pos));
	save_item(NAME(m_adpcm_ff));
}

void rastan_state::machine_reset()
{
	m_sprite_ctrl = 0;
	m_sprites_flipscreen = 0;
	m_adpcm_pos = 0;
	m_adpcm_ff = false;
}


void rastan_state::rastan(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, XTAL(16'000'000)/2);  // verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &rastan_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(rastan_state::irq5_line_hold));

	Z80(config, m_audiocpu, XTAL(16'000'000)/4);    // verified on PCB
	m_audiocpu->set_addrmap(AS_PROGRAM, &rastan_state::sound_map);

	config.set_maximum_quantum(attotime::from_hz(600));  // 10 CPU slices per frame - enough for the sound CPU to read all commands

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(40*8, 32*8);
	screen.set_visarea(0*8, 40*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(rastan_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::xBGR_555, 2048);

	PC080SN(config, m_pc080sn, 0, "palette", gfx_rastan);

	PC090OJ(config, m_pc090oj, 0);
	m_pc090oj->set_palette("palette");
	m_pc090oj->set_colpri_callback(FUNC(rastan_state::colpri_cb));

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ym2151_device &ymsnd(YM2151(config, "ymsnd", XTAL(16'000'000)/4));  // verified on PCB
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.port_write_handler().set(FUNC(rastan_state::sound_bankswitch_w));
	ymsnd.add_route(0, "mono", 0.50);
	ymsnd.add_route(1, "mono", 0.50);

	MSM5205(config, m_msm, XTAL(384'000)); // verified on PCB
	m_msm->vck_legacy_callback().set(FUNC(rastan_state::msm5205_vck)); // VCK function
	m_msm->set_prescaler_selector(msm5205_device::S48_4B);  // 8 kHz
	m_msm->add_route(ALL_OUTPUTS, "mono", 0.60);

	LS157(config, m_adpcm_sel, 0);
	m_adpcm_sel->out_callback().set(m_msm, FUNC(msm5205_device::data_w));

	pc060ha_device &ciu(PC060HA(config, "ciu", 0));
	ciu.nmi_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);
	ciu.reset_callback().set_inputline(m_audiocpu, INPUT_LINE_RESET);
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

/* Byte changes in Rev 1 world sets:

 ROM       0x5203  0x520B
-------------------------
B04-43-1    0x00    0x00
B04-43      0x01    0x03
*/
ROM_START( rastan )
	ROM_REGION( 0x60000, "maincpu", 0 ) // 6*64k for 68000 code
	ROM_LOAD16_BYTE( "b04-38.19",  0x00000, 0x10000, CRC(1c91dbb1) SHA1(17fc55e8546cc0b847aebd67fb4570a1e9f128f3) )
	ROM_LOAD16_BYTE( "b04-37.7",   0x00001, 0x10000, CRC(ecf20bdd) SHA1(92e46b1edef40a19be17091c09daba598d77bca8) )
	ROM_LOAD16_BYTE( "b04-40.20",  0x20000, 0x10000, CRC(0930d4b3) SHA1(c269b3856040ed9409de99cca48f22a2f355fc4c) )
	ROM_LOAD16_BYTE( "b04-39.8",   0x20001, 0x10000, CRC(d95ade5e) SHA1(f47557dcfa9d3137e2a3838e45858fc21471cc91) )
	ROM_LOAD16_BYTE( "b04-42.21",  0x40000, 0x10000, CRC(1857a7cb) SHA1(7d967d04ade648c6ddb19aad9e184b6e272856da) )
	ROM_LOAD16_BYTE( "b04-43-1.9", 0x40001, 0x10000, CRC(ca4702ff) SHA1(0f8c2d7d332c4e35884c48d87ba9fd26924d1692) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "b04-19.49", 0x00000, 0x10000, CRC(ee81fdd8) SHA1(fa59dac2583a7d2979550dffc6f9c6c2bd67bfd5) )

	ROM_REGION( 0x080000, "pc080sn", 0 )
	ROM_LOAD16_BYTE( "b04-01.40", 0x00000, 0x20000, CRC(cd30de19) SHA1(f8d158d38cd07a24cb5ddefd4ce90beec706924d) )
	ROM_LOAD16_BYTE( "b04-02.67", 0x00001, 0x20000, CRC(54040fec) SHA1(a2bea2ce1cebd25b33be41723299ca0512d95f9e) )
	ROM_LOAD16_BYTE( "b04-03.39", 0x40000, 0x20000, CRC(ab67e064) SHA1(5c49f0ff9221cba9f2bb8da86eb4448c73012410) )
	ROM_LOAD16_BYTE( "b04-04.66", 0x40001, 0x20000, CRC(94737e93) SHA1(3df7f085fe6468bda11fab2e86252df6f74f7a99) )

	ROM_REGION( 0x080000, "pc090oj", 0 )
	ROM_LOAD16_BYTE( "b04-05.15", 0x00000, 0x20000, CRC(c22d94ac) SHA1(04f69f9af7ac4242e95dba32988afa3616d75a92) )
	ROM_LOAD16_BYTE( "b04-06.28", 0x00001, 0x20000, CRC(002ccf39) SHA1(fdc29f39198f9b488e298ee89b0eeb3417527733) )
	ROM_LOAD16_BYTE( "b04-07.14", 0x40000, 0x20000, CRC(b5632a51) SHA1(da6ebe6afe245443a76b33714213549356c0c5c3) )
	ROM_LOAD16_BYTE( "b04-08.27", 0x40001, 0x20000, CRC(feafca05) SHA1(9de9ff1fcf037e5ab25c181b678245041238d6ae) )

	ROM_REGION( 0x10000, "adpcm", 0 )   // MSM5205 samples
	ROM_LOAD( "b04-20.76", 0x0000, 0x10000, CRC(fd1a34cc) SHA1(b1682959521fa295769207b75cf7d839e9ec95fd) )
ROM_END

ROM_START( rastana ) // also found on PCB with SEGA S.A. labels, for distribution in Spain
	ROM_REGION( 0x60000, "maincpu", 0 ) // 6*64k for 68000 code
	ROM_LOAD16_BYTE( "b04-38.19", 0x00000, 0x10000, CRC(1c91dbb1) SHA1(17fc55e8546cc0b847aebd67fb4570a1e9f128f3) )
	ROM_LOAD16_BYTE( "b04-37.7",  0x00001, 0x10000, CRC(ecf20bdd) SHA1(92e46b1edef40a19be17091c09daba598d77bca8) )
	ROM_LOAD16_BYTE( "b04-40.20", 0x20000, 0x10000, CRC(0930d4b3) SHA1(c269b3856040ed9409de99cca48f22a2f355fc4c) )
	ROM_LOAD16_BYTE( "b04-39.8",  0x20001, 0x10000, CRC(d95ade5e) SHA1(f47557dcfa9d3137e2a3838e45858fc21471cc91) )
	ROM_LOAD16_BYTE( "b04-42.21", 0x40000, 0x10000, CRC(1857a7cb) SHA1(7d967d04ade648c6ddb19aad9e184b6e272856da) )
	ROM_LOAD16_BYTE( "b04-43.9",  0x40001, 0x10000, CRC(c34b9152) SHA1(6ed9247ad455bc3b71d78b541591b269969830cb) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "b04-19.49", 0x00000, 0x10000, CRC(ee81fdd8) SHA1(fa59dac2583a7d2979550dffc6f9c6c2bd67bfd5) )

	ROM_REGION( 0x080000, "pc080sn", 0 )
	ROM_LOAD16_BYTE( "b04-01.40", 0x00000, 0x20000, CRC(cd30de19) SHA1(f8d158d38cd07a24cb5ddefd4ce90beec706924d) )
	ROM_LOAD16_BYTE( "b04-02.67", 0x00001, 0x20000, CRC(54040fec) SHA1(a2bea2ce1cebd25b33be41723299ca0512d95f9e) )
	ROM_LOAD16_BYTE( "b04-03.39", 0x40000, 0x20000, CRC(ab67e064) SHA1(5c49f0ff9221cba9f2bb8da86eb4448c73012410) )
	ROM_LOAD16_BYTE( "b04-04.66", 0x40001, 0x20000, CRC(94737e93) SHA1(3df7f085fe6468bda11fab2e86252df6f74f7a99) )

	ROM_REGION( 0x080000, "pc090oj", 0 )
	ROM_LOAD16_BYTE( "b04-05.15", 0x00000, 0x20000, CRC(c22d94ac) SHA1(04f69f9af7ac4242e95dba32988afa3616d75a92) )
	ROM_LOAD16_BYTE( "b04-06.28", 0x00001, 0x20000, CRC(002ccf39) SHA1(fdc29f39198f9b488e298ee89b0eeb3417527733) )
	ROM_LOAD16_BYTE( "b04-07.14", 0x40000, 0x20000, CRC(b5632a51) SHA1(da6ebe6afe245443a76b33714213549356c0c5c3) )
	ROM_LOAD16_BYTE( "b04-08.27", 0x40001, 0x20000, CRC(feafca05) SHA1(9de9ff1fcf037e5ab25c181b678245041238d6ae) )

	ROM_REGION( 0x10000, "adpcm", 0 )   // 64k for the samples
	ROM_LOAD( "b04-20.76", 0x0000, 0x10000, CRC(fd1a34cc) SHA1(b1682959521fa295769207b75cf7d839e9ec95fd) ) // samples are 4bit ADPCM
ROM_END

ROM_START( rastanb ) // Earlier code base, but uses 2 US region roms and shows Taito Corporation Japan instead of Taito America Corporation
	ROM_REGION( 0x60000, "maincpu", 0 ) // 6*64k for 68000 code
	ROM_LOAD16_BYTE( "b04-14.19", 0x00000, 0x10000, CRC(a38ac909) SHA1(66d792fee03c6bd87d15060b9d5cae74137c5ebd) ) // These two are from the US rastanub set below
	ROM_LOAD16_BYTE( "b04-21.7",  0x00001, 0x10000, CRC(7c8dde9a) SHA1(0cfc3b4f3bc7b940a6c07267ac95e4aae25801ea) ) // These two are from the US rastanub set below
	ROM_LOAD16_BYTE( "b04-27.20", 0x20000, 0x10000, CRC(ce37694b) SHA1(343c35d93f59c0696104e6934063057d117f4d9f) )
	ROM_LOAD16_BYTE( "b04-26.8",  0x20001, 0x10000, CRC(fbdb98c7) SHA1(717aabf6ce0c7c6107c4bcaae646f7c67b644a54) )
	ROM_LOAD16_BYTE( "b04-29.21", 0x40000, 0x10000, CRC(90d7c6e8) SHA1(6a36cd6db04fcefbf0487f574fccb0eab94e058f) )
	ROM_LOAD16_BYTE( "b04-28.9",  0x40001, 0x10000, CRC(d6440242) SHA1(ba73ca01cc58a8e7bb4d6b7b927658445e97168f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "b04-19.49", 0x00000, 0x10000, CRC(ee81fdd8) SHA1(fa59dac2583a7d2979550dffc6f9c6c2bd67bfd5) )

	ROM_REGION( 0x080000, "pc080sn", 0 )
	ROM_LOAD16_BYTE( "b04-01.40", 0x00000, 0x20000, CRC(cd30de19) SHA1(f8d158d38cd07a24cb5ddefd4ce90beec706924d) )
	ROM_LOAD16_BYTE( "b04-02.67", 0x00001, 0x20000, CRC(54040fec) SHA1(a2bea2ce1cebd25b33be41723299ca0512d95f9e) )
	ROM_LOAD16_BYTE( "b04-03.39", 0x40000, 0x20000, CRC(ab67e064) SHA1(5c49f0ff9221cba9f2bb8da86eb4448c73012410) )
	ROM_LOAD16_BYTE( "b04-04.66", 0x40001, 0x20000, CRC(94737e93) SHA1(3df7f085fe6468bda11fab2e86252df6f74f7a99) )

	ROM_REGION( 0x080000, "pc090oj", 0 )
	ROM_LOAD16_BYTE( "b04-05.15", 0x00000, 0x20000, CRC(c22d94ac) SHA1(04f69f9af7ac4242e95dba32988afa3616d75a92) )
	ROM_LOAD16_BYTE( "b04-06.28", 0x00001, 0x20000, CRC(002ccf39) SHA1(fdc29f39198f9b488e298ee89b0eeb3417527733) )
	ROM_LOAD16_BYTE( "b04-07.14", 0x40000, 0x20000, CRC(b5632a51) SHA1(da6ebe6afe245443a76b33714213549356c0c5c3) )
	ROM_LOAD16_BYTE( "b04-08.27", 0x40001, 0x20000, CRC(feafca05) SHA1(9de9ff1fcf037e5ab25c181b678245041238d6ae) )

	ROM_REGION( 0x10000, "adpcm", 0 )   // MSM5205 samples
	ROM_LOAD( "b04-20.76", 0x0000, 0x10000, CRC(fd1a34cc) SHA1(b1682959521fa295769207b75cf7d839e9ec95fd) )
ROM_END

/* Byte changes in Rev 1 US sets:

 ROM       0x5203  0x520B
-------------------------
B04-41-1    0x00    0x00 <-- Same changes as seen with the World set's B04-43-1.9 & B04-43.9
B04-41      0x01    0x03
*/
ROM_START( rastanu ) // This US set is based on newer code
	ROM_REGION( 0x60000, "maincpu", 0 ) // 6*64k for 68000 code
	ROM_LOAD16_BYTE( "b04-38.19",  0x00000, 0x10000, CRC(1c91dbb1) SHA1(17fc55e8546cc0b847aebd67fb4570a1e9f128f3) )
	ROM_LOAD16_BYTE( "b04-37.7",   0x00001, 0x10000, CRC(ecf20bdd) SHA1(92e46b1edef40a19be17091c09daba598d77bca8) )
	ROM_LOAD16_BYTE( "b04-45.20",  0x20000, 0x10000, CRC(362812dd) SHA1(f7df037ef423d931ca780b34813d4e9e4db67054) )
	ROM_LOAD16_BYTE( "b04-44.8",   0x20001, 0x10000, CRC(51cc5508) SHA1(2bd266351a4d1b94c8c3a489e4d267695d93db5e) )
	ROM_LOAD16_BYTE( "b04-42.21",  0x40000, 0x10000, CRC(1857a7cb) SHA1(7d967d04ade648c6ddb19aad9e184b6e272856da) )
	ROM_LOAD16_BYTE( "b04-41-1.9", 0x40001, 0x10000, CRC(bd403269) SHA1(14aee828d5efb65370a5e453c8fd1c7b3e718074) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "b04-19.49", 0x00000, 0x10000, CRC(ee81fdd8) SHA1(fa59dac2583a7d2979550dffc6f9c6c2bd67bfd5) )

	ROM_REGION( 0x080000, "pc080sn", 0 )
	ROM_LOAD16_BYTE( "b04-01.40", 0x00000, 0x20000, CRC(cd30de19) SHA1(f8d158d38cd07a24cb5ddefd4ce90beec706924d) )
	ROM_LOAD16_BYTE( "b04-02.67", 0x00001, 0x20000, CRC(54040fec) SHA1(a2bea2ce1cebd25b33be41723299ca0512d95f9e) )
	ROM_LOAD16_BYTE( "b04-03.39", 0x40000, 0x20000, CRC(ab67e064) SHA1(5c49f0ff9221cba9f2bb8da86eb4448c73012410) )
	ROM_LOAD16_BYTE( "b04-04.66", 0x40001, 0x20000, CRC(94737e93) SHA1(3df7f085fe6468bda11fab2e86252df6f74f7a99) )

	ROM_REGION( 0x080000, "pc090oj", 0 )
	ROM_LOAD16_BYTE( "b04-05.15", 0x00000, 0x20000, CRC(c22d94ac) SHA1(04f69f9af7ac4242e95dba32988afa3616d75a92) )
	ROM_LOAD16_BYTE( "b04-06.28", 0x00001, 0x20000, CRC(002ccf39) SHA1(fdc29f39198f9b488e298ee89b0eeb3417527733) )
	ROM_LOAD16_BYTE( "b04-07.14", 0x40000, 0x20000, CRC(b5632a51) SHA1(da6ebe6afe245443a76b33714213549356c0c5c3) )
	ROM_LOAD16_BYTE( "b04-08.27", 0x40001, 0x20000, CRC(feafca05) SHA1(9de9ff1fcf037e5ab25c181b678245041238d6ae) )

	ROM_REGION( 0x10000, "adpcm", 0 )   // MSM5205 samples
	ROM_LOAD( "b04-20.76", 0x0000, 0x10000, CRC(fd1a34cc) SHA1(b1682959521fa295769207b75cf7d839e9ec95fd) )
ROM_END

ROM_START( rastanua ) // This US set is based on newer code
	ROM_REGION( 0x60000, "maincpu", 0 ) // 6*64k for 68000 code
	ROM_LOAD16_BYTE( "b04-38.19", 0x00000, 0x10000, CRC(1c91dbb1) SHA1(17fc55e8546cc0b847aebd67fb4570a1e9f128f3) )
	ROM_LOAD16_BYTE( "b04-37.7",  0x00001, 0x10000, CRC(ecf20bdd) SHA1(92e46b1edef40a19be17091c09daba598d77bca8) )
	ROM_LOAD16_BYTE( "b04-45.20", 0x20000, 0x10000, CRC(362812dd) SHA1(f7df037ef423d931ca780b34813d4e9e4db67054) )
	ROM_LOAD16_BYTE( "b04-44.8",  0x20001, 0x10000, CRC(51cc5508) SHA1(2bd266351a4d1b94c8c3a489e4d267695d93db5e) )
	ROM_LOAD16_BYTE( "b04-42.21", 0x40000, 0x10000, CRC(1857a7cb) SHA1(7d967d04ade648c6ddb19aad9e184b6e272856da) )
	ROM_LOAD16_BYTE( "b04-41.9",  0x40001, 0x10000, CRC(b44ca1c4) SHA1(11f1ccc35b6b24aaf253c7994014f08007aba76b) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "b04-19.49", 0x00000, 0x10000, CRC(ee81fdd8) SHA1(fa59dac2583a7d2979550dffc6f9c6c2bd67bfd5) )

	ROM_REGION( 0x080000, "pc080sn", 0 )
	ROM_LOAD16_BYTE( "b04-01.40", 0x00000, 0x20000, CRC(cd30de19) SHA1(f8d158d38cd07a24cb5ddefd4ce90beec706924d) )
	ROM_LOAD16_BYTE( "b04-02.67", 0x00001, 0x20000, CRC(54040fec) SHA1(a2bea2ce1cebd25b33be41723299ca0512d95f9e) )
	ROM_LOAD16_BYTE( "b04-03.39", 0x40000, 0x20000, CRC(ab67e064) SHA1(5c49f0ff9221cba9f2bb8da86eb4448c73012410) )
	ROM_LOAD16_BYTE( "b04-04.66", 0x40001, 0x20000, CRC(94737e93) SHA1(3df7f085fe6468bda11fab2e86252df6f74f7a99) )

	ROM_REGION( 0x080000, "pc090oj", 0 )
	ROM_LOAD16_BYTE( "b04-05.15", 0x00000, 0x20000, CRC(c22d94ac) SHA1(04f69f9af7ac4242e95dba32988afa3616d75a92) )
	ROM_LOAD16_BYTE( "b04-06.28", 0x00001, 0x20000, CRC(002ccf39) SHA1(fdc29f39198f9b488e298ee89b0eeb3417527733) )
	ROM_LOAD16_BYTE( "b04-07.14", 0x40000, 0x20000, CRC(b5632a51) SHA1(da6ebe6afe245443a76b33714213549356c0c5c3) )
	ROM_LOAD16_BYTE( "b04-08.27", 0x40001, 0x20000, CRC(feafca05) SHA1(9de9ff1fcf037e5ab25c181b678245041238d6ae) )

	ROM_REGION( 0x10000, "adpcm", 0 )   // MSM5205 samples
	ROM_LOAD( "b04-20.76", 0x0000, 0x10000, CRC(fd1a34cc) SHA1(b1682959521fa295769207b75cf7d839e9ec95fd) )
ROM_END

/* Byte changes between US & Japan sets for the B04-14/B04-21 & B04-14/B04-13 combo:

 ROM     0x01CB  0x02E2
-----------------------
B04-21    0x1C    0x1C
B04-13    0x44    0x44

These US & Japan sets didn't use a region code byte (see above) so the changes are likely
pointers to the copyright string.
*/
ROM_START( rastanub ) // This US set is based on the earlier code
	ROM_REGION( 0x60000, "maincpu", 0 ) // 6*64k for 68000 code
	ROM_LOAD16_BYTE( "b04-14.19", 0x00000, 0x10000, CRC(a38ac909) SHA1(66d792fee03c6bd87d15060b9d5cae74137c5ebd) ) // This is the correct verified ROM number
	ROM_LOAD16_BYTE( "b04-21.7",  0x00001, 0x10000, CRC(7c8dde9a) SHA1(0cfc3b4f3bc7b940a6c07267ac95e4aae25801ea) ) // 2 bytes differ from the Japan set
	ROM_LOAD16_BYTE( "b04-23.20", 0x20000, 0x10000, CRC(254b3dce) SHA1(5126cd5268abaa78dfdcd2ca70621c093c79be67) )
	ROM_LOAD16_BYTE( "b04-22.8",  0x20001, 0x10000, CRC(98e8edcf) SHA1(cc89ef36da6d21192efc19c3bbb215b1635b7ef3) )
	ROM_LOAD16_BYTE( "b04-25.21", 0x40000, 0x10000, CRC(d1e5adee) SHA1(eafc275a0023aecb2efaff14cd890915fa162624) )
	ROM_LOAD16_BYTE( "b04-24.9",  0x40001, 0x10000, CRC(a3dcc106) SHA1(3a8854530b08864a1f7f46c427e49ceec8297806) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "b04-19.49", 0x00000, 0x10000, CRC(ee81fdd8) SHA1(fa59dac2583a7d2979550dffc6f9c6c2bd67bfd5) )

	ROM_REGION( 0x080000, "pc080sn", 0 )
	ROM_LOAD16_BYTE( "b04-01.40", 0x00000, 0x20000, CRC(cd30de19) SHA1(f8d158d38cd07a24cb5ddefd4ce90beec706924d) )
	ROM_LOAD16_BYTE( "b04-02.67", 0x00001, 0x20000, CRC(54040fec) SHA1(a2bea2ce1cebd25b33be41723299ca0512d95f9e) )
	ROM_LOAD16_BYTE( "b04-03.39", 0x40000, 0x20000, CRC(ab67e064) SHA1(5c49f0ff9221cba9f2bb8da86eb4448c73012410) )
	ROM_LOAD16_BYTE( "b04-04.66", 0x40001, 0x20000, CRC(94737e93) SHA1(3df7f085fe6468bda11fab2e86252df6f74f7a99) )

	ROM_REGION( 0x080000, "pc090oj", 0 )
	ROM_LOAD16_BYTE( "b04-05.15", 0x00000, 0x20000, CRC(c22d94ac) SHA1(04f69f9af7ac4242e95dba32988afa3616d75a92) )
	ROM_LOAD16_BYTE( "b04-06.28", 0x00001, 0x20000, CRC(002ccf39) SHA1(fdc29f39198f9b488e298ee89b0eeb3417527733) )
	ROM_LOAD16_BYTE( "b04-07.14", 0x40000, 0x20000, CRC(b5632a51) SHA1(da6ebe6afe245443a76b33714213549356c0c5c3) )
	ROM_LOAD16_BYTE( "b04-08.27", 0x40001, 0x20000, CRC(feafca05) SHA1(9de9ff1fcf037e5ab25c181b678245041238d6ae) )

	ROM_REGION( 0x10000, "adpcm", 0 )   // MSM5205 samples
	ROM_LOAD( "b04-20.76", 0x0000, 0x10000, CRC(fd1a34cc) SHA1(b1682959521fa295769207b75cf7d839e9ec95fd) )
ROM_END

ROM_START( rastsaga )
	ROM_REGION( 0x60000, "maincpu", 0 ) // 6*64k for 68000 code
	ROM_LOAD16_BYTE( "b04-32.19",   0x00000, 0x10000, CRC(1c91dbb1) SHA1(17fc55e8546cc0b847aebd67fb4570a1e9f128f3) )
	ROM_LOAD16_BYTE( "b04-31.7",    0x00001, 0x10000, CRC(4c62e89e) SHA1(41673a1375a4ff3e59edbb6194915bb2348b70dd) )
	ROM_LOAD16_BYTE( "b04-34-1.20", 0x20000, 0x10000, CRC(8f54dd19) SHA1(ed9c221ebce3ba76c39dc83fed58acff3028e6c9) )
	ROM_LOAD16_BYTE( "b04-33-1.8",  0x20001, 0x10000, CRC(810a02a3) SHA1(813a8d4d15d02c131dcae831d1b449c754d91f44) )
	ROM_LOAD16_BYTE( "b04-36.21",   0x40000, 0x10000, CRC(32e286c0) SHA1(7f80aff6d9f5700932d970f7ebdc9efe10db7f76) )
	ROM_LOAD16_BYTE( "b04-35.9",    0x40001, 0x10000, CRC(ee5ec5bc) SHA1(8bdd2cdcb4b664131ef9a394b3ab30b0fde71b02) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "b04-19.49", 0x00000, 0x10000, CRC(ee81fdd8) SHA1(fa59dac2583a7d2979550dffc6f9c6c2bd67bfd5) )

	ROM_REGION( 0x080000, "pc080sn", 0 )
	ROM_LOAD16_BYTE( "b04-01.40", 0x00000, 0x20000, CRC(cd30de19) SHA1(f8d158d38cd07a24cb5ddefd4ce90beec706924d) )
	ROM_LOAD16_BYTE( "b04-02.67", 0x00001, 0x20000, CRC(54040fec) SHA1(a2bea2ce1cebd25b33be41723299ca0512d95f9e) )
	ROM_LOAD16_BYTE( "b04-03.39", 0x40000, 0x20000, CRC(ab67e064) SHA1(5c49f0ff9221cba9f2bb8da86eb4448c73012410) )
	ROM_LOAD16_BYTE( "b04-04.66", 0x40001, 0x20000, CRC(94737e93) SHA1(3df7f085fe6468bda11fab2e86252df6f74f7a99) )

	ROM_REGION( 0x080000, "pc090oj", 0 )
	ROM_LOAD16_BYTE( "b04-05.15", 0x00000, 0x20000, CRC(c22d94ac) SHA1(04f69f9af7ac4242e95dba32988afa3616d75a92) )
	ROM_LOAD16_BYTE( "b04-06.28", 0x00001, 0x20000, CRC(002ccf39) SHA1(fdc29f39198f9b488e298ee89b0eeb3417527733) )
	ROM_LOAD16_BYTE( "b04-07.14", 0x40000, 0x20000, CRC(b5632a51) SHA1(da6ebe6afe245443a76b33714213549356c0c5c3) )
	ROM_LOAD16_BYTE( "b04-08.27", 0x40001, 0x20000, CRC(feafca05) SHA1(9de9ff1fcf037e5ab25c181b678245041238d6ae) )

	ROM_REGION( 0x10000, "adpcm", 0 )   // MSM5205 samples
	ROM_LOAD( "b04-20.76", 0x0000, 0x10000, CRC(fd1a34cc) SHA1(b1682959521fa295769207b75cf7d839e9ec95fd) )
ROM_END

ROM_START( rastsagaa )
	ROM_REGION( 0x60000, "maincpu", 0 ) // 6*64k for 68000 code
	ROM_LOAD16_BYTE( "b04-14.19",   0x00000, 0x10000, CRC(a38ac909) SHA1(66d792fee03c6bd87d15060b9d5cae74137c5ebd) )
	ROM_LOAD16_BYTE( "b04-13.7",    0x00001, 0x10000, CRC(bad60872) SHA1(e020f79b3ac3d2abccfcd5d135d2dc49e1335c7d) )
	ROM_LOAD16_BYTE( "b04-16-1.20", 0x20000, 0x10000, CRC(00b59e60) SHA1(545ab3eb9ef25c532dda5a9eec087665ba0cecc1) )
	ROM_LOAD16_BYTE( "b04-15-1.8",  0x20001, 0x10000, CRC(ff9e018a) SHA1(37048eecec799f29564517fae9525ef0e3d9d9e5) )
	ROM_LOAD16_BYTE( "b04-18-1.21", 0x40000, 0x10000, CRC(b626c439) SHA1(976e820edc4ba107c5b579edaaee1e354e85fb67) )
	ROM_LOAD16_BYTE( "b04-17-1.9",  0x40001, 0x10000, CRC(c928a516) SHA1(fe87fdf2d1b7ba93e1986460eb6af648b58f42e4) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "b04-19.49", 0x00000, 0x10000, CRC(ee81fdd8) SHA1(fa59dac2583a7d2979550dffc6f9c6c2bd67bfd5) )

	ROM_REGION( 0x080000, "pc080sn", 0 )
	ROM_LOAD16_BYTE( "b04-01.40", 0x00000, 0x20000, CRC(cd30de19) SHA1(f8d158d38cd07a24cb5ddefd4ce90beec706924d) )
	ROM_LOAD16_BYTE( "b04-02.67", 0x00001, 0x20000, CRC(54040fec) SHA1(a2bea2ce1cebd25b33be41723299ca0512d95f9e) )
	ROM_LOAD16_BYTE( "b04-03.39", 0x40000, 0x20000, CRC(ab67e064) SHA1(5c49f0ff9221cba9f2bb8da86eb4448c73012410) )
	ROM_LOAD16_BYTE( "b04-04.66", 0x40001, 0x20000, CRC(94737e93) SHA1(3df7f085fe6468bda11fab2e86252df6f74f7a99) )

	ROM_REGION( 0x080000, "pc090oj", 0 )
	ROM_LOAD16_BYTE( "b04-05.15", 0x00000, 0x20000, CRC(c22d94ac) SHA1(04f69f9af7ac4242e95dba32988afa3616d75a92) )
	ROM_LOAD16_BYTE( "b04-06.28", 0x00001, 0x20000, CRC(002ccf39) SHA1(fdc29f39198f9b488e298ee89b0eeb3417527733) )
	ROM_LOAD16_BYTE( "b04-07.14", 0x40000, 0x20000, CRC(b5632a51) SHA1(da6ebe6afe245443a76b33714213549356c0c5c3) )
	ROM_LOAD16_BYTE( "b04-08.27", 0x40001, 0x20000, CRC(feafca05) SHA1(9de9ff1fcf037e5ab25c181b678245041238d6ae) )

	ROM_REGION( 0x10000, "adpcm", 0 )   // MSM5205 samples
	ROM_LOAD( "b04-20.76", 0x0000, 0x10000, CRC(fd1a34cc) SHA1(b1682959521fa295769207b75cf7d839e9ec95fd) )
ROM_END

ROM_START( rastsagaabl ) // 2-PCB set, data is identical to rastsagaa but the GFX ROMs are smaller
	ROM_REGION( 0x60000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "9",  0x00000, 0x10000, CRC(a38ac909) SHA1(66d792fee03c6bd87d15060b9d5cae74137c5ebd) )
	ROM_LOAD16_BYTE( "12", 0x00001, 0x10000, CRC(bad60872) SHA1(e020f79b3ac3d2abccfcd5d135d2dc49e1335c7d) )
	ROM_LOAD16_BYTE( "10", 0x20000, 0x10000, CRC(00b59e60) SHA1(545ab3eb9ef25c532dda5a9eec087665ba0cecc1) )
	ROM_LOAD16_BYTE( "13", 0x20001, 0x10000, CRC(ff9e018a) SHA1(37048eecec799f29564517fae9525ef0e3d9d9e5) )
	ROM_LOAD16_BYTE( "11", 0x40000, 0x10000, CRC(b626c439) SHA1(976e820edc4ba107c5b579edaaee1e354e85fb67) )
	ROM_LOAD16_BYTE( "14", 0x40001, 0x10000, CRC(c928a516) SHA1(fe87fdf2d1b7ba93e1986460eb6af648b58f42e4) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "2", 0x00000, 0x10000, CRC(ee81fdd8) SHA1(fa59dac2583a7d2979550dffc6f9c6c2bd67bfd5) )

	ROM_REGION( 0x080000, "pc080sn", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "8", 0x00000, 0x10000, CRC(6aac8f67) SHA1(6042dc936eaa22b0afb200c8306be4bbddaad4d7) )
	ROM_LOAD16_BYTE( "5", 0x00001, 0x10000, CRC(8c184637) SHA1(fcabe034aa331d3c0b05854b9f504edb4f04f5b8) )
	ROM_LOAD16_BYTE( "6", 0x20000, 0x10000, CRC(e8b64ced) SHA1(17c09a26ce0ee0f10f3cbaa166f38f9716e7e4ea) )
	ROM_LOAD16_BYTE( "3", 0x20001, 0x10000, CRC(27f6c59b) SHA1(84697bd33ccd64914955f3cdb41684f0be428f54) )
	ROM_LOAD16_BYTE( "7", 0x40000, 0x10000, CRC(225e19fa) SHA1(14dd2f8b7d81e5f39fcf07f9c4421815f1fefe22) )
	ROM_LOAD16_BYTE( "4", 0x40001, 0x10000, CRC(aa3a2d5e) SHA1(1c6017e2c4604c315517e5acd7eced4e08515728) )

	ROM_REGION( 0x080000, "pc090oj", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "22", 0x00000, 0x08000, CRC(d9d09beb) SHA1(cddbda5a1af7564a63d00926891f5d8c75715a2f) )
	ROM_LOAD16_BYTE( "18", 0x00001, 0x08000, CRC(d6fb44fe) SHA1(ac0ef2f8771bfd9b5bbbecfb81ba8b6de1bd290a) )
	ROM_LOAD16_BYTE( "28", 0x10000, 0x08000, CRC(f19630f7) SHA1(c6bd015185e883be8a6df97fdcfcd229d01496f6) )
	ROM_LOAD16_BYTE( "25", 0x10001, 0x08000, CRC(d778ceea) SHA1(398135f3dab4ff27d206e204fd49c06315788a3f) )
	ROM_LOAD16_BYTE( "21", 0x20000, 0x08000, CRC(21453a7a) SHA1(d7b18ad96ce9fc85138521404359a95d00fe01ff) )
	ROM_LOAD16_BYTE( "17", 0x20001, 0x08000, CRC(5cd5c7e9) SHA1(f50d595f5fa8eb59a8a50ce1968dd4e362d7ab04) )
	ROM_LOAD16_BYTE( "27", 0x30000, 0x08000, CRC(2e66aa0b) SHA1(6ab10ddba5cd94f995f02425090fff1b7c08b08b) )
	ROM_LOAD16_BYTE( "24", 0x30001, 0x08000, CRC(88217cbc) SHA1(149f54ae4ef00dacbc95a29f45b854efd53fc8a9) )
	ROM_LOAD16_BYTE( "20", 0x40000, 0x08000, CRC(9cd627e1) SHA1(31481048e4fcec8c31c247e16953774e063189ec) )
	ROM_LOAD16_BYTE( "16", 0x40001, 0x08000, CRC(93d64eef) SHA1(6faa76cb5bb8ffa77c7dce85678a0e6f1e1b9ba8) )
	ROM_LOAD16_BYTE( "26", 0x50000, 0x08000, CRC(9bca4abc) SHA1(ae88e5c7b75bb9a9d80582f10c56cdd70bc86412) )
	ROM_LOAD16_BYTE( "23", 0x50001, 0x08000, CRC(cde8891b) SHA1(8f52df3cfe585982bd1bb03c34b2de02b424490d) )
	ROM_LOAD16_BYTE( "19", 0x60000, 0x08000, CRC(02ed3b16) SHA1(2f92ba8181a6391afa9ccd290ba13b69cf735e51) )
	ROM_LOAD16_BYTE( "15", 0x60001, 0x08000, CRC(3e4c41d4) SHA1(4868ba2239d77659e63894898c81a0355214ea3b) )

	ROM_REGION( 0x10000, "adpcm", 0 )
	ROM_LOAD( "1", 0x0000, 0x10000, CRC(fd1a34cc) SHA1(b1682959521fa295769207b75cf7d839e9ec95fd) )
ROM_END

ROM_START( rastsagab )
	ROM_REGION( 0x60000, "maincpu", 0 ) // 6*64k for 68000 code
	ROM_LOAD16_BYTE( "b04-14.19",   0x00000, 0x10000, CRC(a38ac909) SHA1(66d792fee03c6bd87d15060b9d5cae74137c5ebd) ) // Dumped as "RS19_38.BIN", corrected to B04-14
	ROM_LOAD16_BYTE( "b04-13.7",    0x00001, 0x10000, CRC(bad60872) SHA1(e020f79b3ac3d2abccfcd5d135d2dc49e1335c7d) ) // Dumped as "RS07_37.BIN", corrected to B04-13
	ROM_LOAD16_BYTE( "b04-16.20",   0x20000, 0x10000, CRC(6bcf70dc) SHA1(3e369548ac01981c503150b44c2747e6c2cec12a) ) // Dumped as "RS20_40.BIN", but is likely B04-16 - Need to verify
	ROM_LOAD16_BYTE( "b04-15.8",    0x20001, 0x10000, CRC(8838ecc5) SHA1(42b43ab77969bbacdf178fbe73a0a27652ccb297) ) // Dumped as "RS08_39.BIN", but is likely B04-15 - Need to verify
	ROM_LOAD16_BYTE( "b04-18-1.21", 0x40000, 0x10000, CRC(b626c439) SHA1(976e820edc4ba107c5b579edaaee1e354e85fb67) ) // Dumped as "RS21_42.BIN", corrected to B04-18-1
	ROM_LOAD16_BYTE( "b04-17-1.9",  0x40001, 0x10000, CRC(c928a516) SHA1(fe87fdf2d1b7ba93e1986460eb6af648b58f42e4) ) // Dumped as "RS09_43.BIN", corrected to B04-17-1

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "b04-19.49", 0x00000, 0x10000, CRC(ee81fdd8) SHA1(fa59dac2583a7d2979550dffc6f9c6c2bd67bfd5) )

	ROM_REGION( 0x080000, "pc080sn", 0 )
	ROM_LOAD16_BYTE( "b04-01.40", 0x00000, 0x20000, CRC(cd30de19) SHA1(f8d158d38cd07a24cb5ddefd4ce90beec706924d) )
	ROM_LOAD16_BYTE( "b04-02.67", 0x00001, 0x20000, CRC(54040fec) SHA1(a2bea2ce1cebd25b33be41723299ca0512d95f9e) )
	ROM_LOAD16_BYTE( "b04-03.39", 0x40000, 0x20000, CRC(ab67e064) SHA1(5c49f0ff9221cba9f2bb8da86eb4448c73012410) )
	ROM_LOAD16_BYTE( "b04-04.66", 0x40001, 0x20000, CRC(94737e93) SHA1(3df7f085fe6468bda11fab2e86252df6f74f7a99) )

	ROM_REGION( 0x080000, "pc090oj", 0 )
	ROM_LOAD16_BYTE( "b04-05.15", 0x00000, 0x20000, CRC(c22d94ac) SHA1(04f69f9af7ac4242e95dba32988afa3616d75a92) )
	ROM_LOAD16_BYTE( "b04-06.28", 0x00001, 0x20000, CRC(002ccf39) SHA1(fdc29f39198f9b488e298ee89b0eeb3417527733) )
	ROM_LOAD16_BYTE( "b04-07.14", 0x40000, 0x20000, CRC(b5632a51) SHA1(da6ebe6afe245443a76b33714213549356c0c5c3) )
	ROM_LOAD16_BYTE( "b04-08.27", 0x40001, 0x20000, CRC(feafca05) SHA1(9de9ff1fcf037e5ab25c181b678245041238d6ae) )

	ROM_REGION( 0x10000, "adpcm", 0 )   // MSM5205 samples
	ROM_LOAD( "b04-20.76", 0x0000, 0x10000, CRC(fd1a34cc) SHA1(b1682959521fa295769207b75cf7d839e9ec95fd) )
ROM_END

} // Anonymous namespace


// Newer revised code base
GAME( 1987, rastan,      0,      rastan, rastan,   rastan_state, empty_init, ROT0, "Taito Corporation Japan",   "Rastan (World Rev 1)",      MACHINE_SUPPORTS_SAVE )
GAME( 1987, rastana,     rastan, rastan, rastsaga, rastan_state, empty_init, ROT0, "Taito Corporation Japan",   "Rastan (World)",            MACHINE_SUPPORTS_SAVE )
GAME( 1987, rastanu,     rastan, rastan, rastsaga, rastan_state, empty_init, ROT0, "Taito America Corporation", "Rastan (US Rev 1)",         MACHINE_SUPPORTS_SAVE )
GAME( 1987, rastanua,    rastan, rastan, rastsaga, rastan_state, empty_init, ROT0, "Taito America Corporation", "Rastan (US)",               MACHINE_SUPPORTS_SAVE )
GAME( 1987, rastsaga,    rastan, rastan, rastsaga, rastan_state, empty_init, ROT0, "Taito Corporation",         "Rastan Saga (Japan Rev 1)", MACHINE_SUPPORTS_SAVE )

// Based on earliest code base
GAME( 1987, rastanb,     rastan, rastan, rastsaga, rastan_state, empty_init, ROT0, "Taito Corporation Japan",   "Rastan (World, earlier code base)",                     MACHINE_SUPPORTS_SAVE )
GAME( 1987, rastanub,    rastan, rastan, rastsaga, rastan_state, empty_init, ROT0, "Taito America Corporation", "Rastan (US, earlier code base)",                        MACHINE_SUPPORTS_SAVE )
GAME( 1987, rastsagaa,   rastan, rastan, rastsaga, rastan_state, empty_init, ROT0, "Taito Corporation",         "Rastan Saga (Japan Rev 1, earlier code base)",          MACHINE_SUPPORTS_SAVE )
GAME( 1987, rastsagaabl, rastan, rastan, rastsaga, rastan_state, empty_init, ROT0, "bootleg",                   "Rastan Saga (bootleg, Japan Rev 1, earlier code base)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, rastsagab,   rastan, rastan, rastsaga, rastan_state, empty_init, ROT0, "Taito Corporation",         "Rastan Saga (Japan, earlier code base)",                MACHINE_SUPPORTS_SAVE )
