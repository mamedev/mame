// license:BSD-3-Clause
// copyright-holders: Nicola Salmoria

/*******************************************************************************

    Block Hole (GX973) (c) 1989 Konami

    Original driver by Nicola Salmoria

    Notes:
        - To advance to the next screen in service mode, press P1 and P2 start
          simultaneously

    Todo:
        - How is the sound irq cleared (currently using HOLD_LINE)?
        - Do bit 2 and 7 of the bankswitch port have any meaning?
        - Verify raw screen parameters

*******************************************************************************/

#include "emu.h"

#include "k051960.h"
#include "k052109.h"
#include "konamipt.h"

#include "cpu/m6809/konami.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/watchdog.h"
#include "sound/ymopm.h"

#include "emupal.h"
#include "speaker.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class blockhl_state : public driver_device
{
public:
	blockhl_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k052109(*this, "k052109"),
		m_k051960(*this, "k051960"),
		m_rombank(*this, "rombank"),
		m_view5800(*this, "view5800")
	{ }

	void blockhl(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<konami_cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k052109_device> m_k052109;
	required_device<k051960_device> m_k051960;
	required_memory_bank m_rombank;
	memory_view m_view5800;

	K052109_CB_MEMBER(tile_callback);
	K051960_CB_MEMBER(sprite_callback);
	uint32_t screen_update_blockhl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint8_t k052109_051960_r(offs_t offset);
	void k052109_051960_w(offs_t offset, uint8_t data);

	void sound_irq_w(uint8_t data);

	void banking_callback(uint8_t data);

	void audio_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void blockhl_state::main_map(address_map &map)
{
	map(0x0000, 0x3fff).rw(FUNC(blockhl_state::k052109_051960_r), FUNC(blockhl_state::k052109_051960_w));
	map(0x1f84, 0x1f84).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x1f88, 0x1f88).w(FUNC(blockhl_state::sound_irq_w));
	map(0x1f8c, 0x1f8c).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x1f94, 0x1f94).portr("DSW3");
	map(0x1f95, 0x1f95).portr("P1");
	map(0x1f96, 0x1f96).portr("P2");
	map(0x1f97, 0x1f97).portr("DSW1");
	map(0x1f98, 0x1f98).portr("DSW2");
	map(0x4000, 0x57ff).ram();
	map(0x5800, 0x5fff).view(m_view5800);
	m_view5800[0](0x5800, 0x5fff).ram().w("palette", FUNC(palette_device::write8)).share("palette");
	m_view5800[1](0x5800, 0x5fff).ram();
	map(0x6000, 0x7fff).bankr("rombank");
	map(0x8000, 0xffff).rom().region("maincpu", 0x8000);
}

void blockhl_state::audio_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0xa000, 0xa000).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0xc000, 0xc001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xe00c, 0xe00d).nopw(); // leftover from missing 007232?
}


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

K052109_CB_MEMBER(blockhl_state::tile_callback)
{
	static const int layer_colorbase[] = { 0 / 16, 256 / 16, 512 / 16 };

	*code |= ((*color & 0x0f) << 8);
	*color = layer_colorbase[layer] + ((*color & 0xe0) >> 5);
}

K051960_CB_MEMBER(blockhl_state::sprite_callback)
{
	enum { sprite_colorbase = 768 / 16 };

	*priority = (*color & 0x10) ? GFX_PMASK_1 : 0;
	*color = sprite_colorbase + (*color & 0x0f);
}

uint32_t blockhl_state::screen_update_blockhl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);

	m_k052109->tilemap_update();
	m_k052109->tilemap_draw(screen, bitmap, cliprect, 2, TILEMAP_DRAW_OPAQUE, 0);   // tile 2
	m_k052109->tilemap_draw(screen, bitmap, cliprect, 1, 0, 1); // tile 1
	m_k051960->k051960_sprites_draw(bitmap, cliprect, screen.priority(), -1, -1);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, 0, 0, 0); // tile 0

	return 0;
}

// special handlers to combine 052109 & 051960
uint8_t blockhl_state::k052109_051960_r(offs_t offset)
{
	if (m_k052109->get_rmrd_line() == CLEAR_LINE)
	{
		if (offset >= 0x3800 && offset < 0x3808)
			return m_k051960->k051937_r(offset - 0x3800);
		else if (offset < 0x3c00)
			return m_k052109->read(offset);
		else
			return m_k051960->k051960_r(offset - 0x3c00);
	}
	else
		return m_k052109->read(offset);
}

void blockhl_state::k052109_051960_w(offs_t offset, uint8_t data)
{
	if (offset >= 0x3800 && offset < 0x3808)
		m_k051960->k051937_w(offset - 0x3800, data);
	else if (offset < 0x3c00)
		m_k052109->write(offset, data);
	else
		m_k051960->k051960_w(offset - 0x3c00, data);
}


//**************************************************************************
//  AUDIO EMULATION
//**************************************************************************

void blockhl_state::sound_irq_w(uint8_t data)
{
	m_audiocpu->set_input_line(INPUT_LINE_IRQ0, HOLD_LINE);
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void blockhl_state::machine_start()
{
	// the first 0x8000 are banked, the remaining 0x8000 are directly accessible
	m_rombank->configure_entries(0, 4, memregion("maincpu")->base(), 0x2000);

	m_view5800.select(0);
}

void blockhl_state::banking_callback(uint8_t data)
{
	// bits 0-1 = ROM bank
	m_rombank->set_entry(data & 0x03);

	// bit 2, unknown (always 0)

	// bits 3/4 = coin counters
	machine().bookkeeping().coin_counter_w(0, data & 0x08);
	machine().bookkeeping().coin_counter_w(1, data & 0x10);

	// bit 5 = select palette RAM or work RAM at 5800-5fff
	m_view5800.select(BIT(data, 5));

	// bit 6 = enable char ROM reading through the video RAM
	m_k052109->set_rmrd_line(BIT(data, 6) ? ASSERT_LINE : CLEAR_LINE);

	// bit 7, unknown (always 1)
}


//**************************************************************************
//  INPUTS
//**************************************************************************

static INPUT_PORTS_START( blockhl )
	PORT_START("P1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_4WAY PORT_PLAYER(1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    PORT_4WAY PORT_PLAYER(1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  PORT_4WAY PORT_PLAYER(1)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(1)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_PLAYER(1)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_START1)

	PORT_START("P2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_4WAY PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    PORT_4WAY PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(2)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_PLAYER(2)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_START2)

	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "Void", SW1)
	// "Void" = coins produce sound, but no effect on coin counter

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW2:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW2:5" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_COIN2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_SERVICE1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW3:2" )
	PORT_SERVICE_DIPLOC(   0x40, IP_ACTIVE_LOW, "SW3:3" )
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW3:4" )
INPUT_PORTS_END


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void blockhl_state::blockhl(machine_config &config)
{
	// basic machine hardware
	KONAMI(config, m_maincpu, XTAL(24'000'000) / 2); // Konami 052526
	m_maincpu->set_addrmap(AS_PROGRAM, &blockhl_state::main_map);
	m_maincpu->line().set(FUNC(blockhl_state::banking_callback));

	Z80(config, m_audiocpu, XTAL(3'579'545));
	m_audiocpu->set_addrmap(AS_PROGRAM, &blockhl_state::audio_map);

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(24'000'000) / 3, 528, 112, 400, 256, 16, 240);
//  6MHz dotclock is more realistic, however needs drawing updates. replace when ready
//  screen.set_raw(XTAL(24'000'000) / 4, 396, hbend, hbstart, 256, 16, 240);
	screen.set_screen_update(FUNC(blockhl_state::screen_update_blockhl));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::xBGR_555, 1024).enable_shadows();

	K052109(config, m_k052109, 0);
	m_k052109->set_palette("palette");
	m_k052109->set_screen("screen");
	m_k052109->set_tile_callback(FUNC(blockhl_state::tile_callback));
	m_k052109->irq_handler().set_inputline(m_maincpu, KONAMI_IRQ_LINE);

	K051960(config, m_k051960, 0);
	m_k051960->set_palette("palette");
	m_k051960->set_screen("screen");
	m_k051960->set_sprite_callback(FUNC(blockhl_state::sprite_callback));

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	YM2151(config, "ymsnd", XTAL(3'579'545)).add_route(0, "mono", 0.60).add_route(1, "mono", 0.60);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( blockhl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "973l02.e21", 0x00000, 0x10000, CRC(e14f849a) SHA1(d44cf178cc98998b72ed32c6e20b6ebdf1f97579) )

	ROM_REGION( 0x08000, "audiocpu", 0 )
	ROM_LOAD( "973d01.g6",  0x00000, 0x08000, CRC(eeee9d92) SHA1(6c6c324b1f6f4fba0aa12e0d1fc5dbab133ef669) )

	ROM_REGION( 0x20000, "k052109", 0 ) // tiles
	ROM_LOAD32_BYTE( "973f07.k15", 0x00000, 0x08000, CRC(1a8cd9b4) SHA1(7cb7944d24ac51fa6b610542d9dec68697cacf0f) )
	ROM_LOAD32_BYTE( "973f08.k18", 0x00001, 0x08000, CRC(952b51a6) SHA1(017575738d444b688b137cad5611638d53be84f2) )
	ROM_LOAD32_BYTE( "973f09.k20", 0x00002, 0x08000, CRC(77841594) SHA1(e1bfdc5bb598d865868d578ef7faba8078becd7a) )
	ROM_LOAD32_BYTE( "973f10.k23", 0x00003, 0x08000, CRC(09039fab) SHA1(a9dea17aacf4484d21ef3b16470263447b51b6b5) )

	ROM_REGION( 0x20000, "k051960", 0 ) // sprites
	ROM_LOAD32_BYTE( "973f06.k12", 0x00000, 0x08000, CRC(51acfdb6) SHA1(94d243f341b490684f5297d95d4835bd522ece35) )
	ROM_LOAD32_BYTE( "973f05.k9",  0x00001, 0x08000, CRC(4cfea298) SHA1(4772b5b99f5fd8174d8884bd84173512e1edabf4) )
	ROM_LOAD32_BYTE( "973f04.k7",  0x00002, 0x08000, CRC(69ca41bd) SHA1(9b0b1c888efd2f2d5525f14778e18fb4a7353eb6) )
	ROM_LOAD32_BYTE( "973f03.k4",  0x00003, 0x08000, CRC(21e98472) SHA1(8c697d369a1f57be0825c33b4e9107ce1b02a130) )

	ROM_REGION( 0x0100, "priority", 0 ) // not used
	ROM_LOAD( "973a11.h10", 0x0000, 0x0100, CRC(46d28fe9) SHA1(9d0811a928c8907785ef483bfbee5445506b3ec8) )
ROM_END

ROM_START( quarth )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "973j02.e21", 0x00000, 0x10000, CRC(27a90118) SHA1(51309385b93db29b9277d14252166c4ea1746303) )

	ROM_REGION( 0x08000, "audiocpu", 0 )
	ROM_LOAD( "973d01.g6",  0x00000, 0x08000, CRC(eeee9d92) SHA1(6c6c324b1f6f4fba0aa12e0d1fc5dbab133ef669) )

	ROM_REGION( 0x20000, "k052109", 0 ) // tiles
	ROM_LOAD32_BYTE( "973e07.k15", 0x00000, 0x08000, CRC(0bd6b0f8) SHA1(6c59cf637354fe2df424eaa89feb9c1bc1f66a92) )
	ROM_LOAD32_BYTE( "973e08.k18", 0x00001, 0x08000, CRC(104d0d5f) SHA1(595698911513113d01e5b565f5b073d1bd033d3f) )
	ROM_LOAD32_BYTE( "973e09.k20", 0x00002, 0x08000, CRC(bd3a6f24) SHA1(eb45db3a6a52bb2b25df8c2dace877e59b4130a6) )
	ROM_LOAD32_BYTE( "973e10.k23", 0x00003, 0x08000, CRC(cf5e4b86) SHA1(43348753894c1763b26dbfc70245dac92048db8f) )

	ROM_REGION( 0x20000, "k051960", 0 ) // sprites
	ROM_LOAD32_BYTE( "973e06.k12", 0x00000, 0x08000, CRC(0d58af85) SHA1(2efd661d614fb305a14cfe1aa4fb17714f215d4f) )
	ROM_LOAD32_BYTE( "973e05.k9",  0x00001, 0x08000, CRC(15d822cb) SHA1(70ecad5e0a461df0da6e6eb23f43a7b643297f0d) )
	ROM_LOAD32_BYTE( "973e04.k7",  0x00002, 0x08000, CRC(d70f4a2c) SHA1(25f835a17bacf2b8debb2eb8a3cff90cab3f402a) )
	ROM_LOAD32_BYTE( "973e03.k4",  0x00003, 0x08000, CRC(2c5a4b4b) SHA1(e2991dd78b9cd96cf93ebd6de0d4e060d346ab9c) )

	ROM_REGION( 0x0100, "priority", 0 ) // not used
	ROM_LOAD( "973a11.h10", 0x0000, 0x0100, CRC(46d28fe9) SHA1(9d0811a928c8907785ef483bfbee5445506b3ec8) )
ROM_END

} // anonymous namespace


//**************************************************************************
//  GAME DRIVERS
//**************************************************************************

//    YEAR  NAME     PARENT   MACHINE  INPUT    CLASS          INIT        ROT   COMPANY   FULLNAME          FLAGS
GAME( 1989, blockhl, 0,       blockhl, blockhl, blockhl_state, empty_init, ROT0, "Konami", "Block Hole",     MACHINE_SUPPORTS_SAVE )
GAME( 1989, quarth,  blockhl, blockhl, blockhl, blockhl_state, empty_init, ROT0, "Konami", "Quarth (Japan)", MACHINE_SUPPORTS_SAVE )
