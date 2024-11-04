// license:BSD-3-Clause
// copyright-holders: David Haywood

/* Mug Smashers (c)199? Electronic Devices (Italy) / 3D Games (England)
    driver by David Haywood - Dip Switches and Inputs by stephh

 a side scrolling beat-em-up, borrows ideas from Combatribes, including
 the music (apparently) and sound hardware!

 there is also a Spanish company logo in the graphic ROMs

 a 1990 copyright can be found in the sound program so it's 1990 at the
 earliest

*/

/* working notes:

68k interrupts
lev 1 : 0x64 : 0000 0100 - just rte
lev 2 : 0x68 : 0000 0100 - just rte
lev 3 : 0x6c : 0000 0100 - just rte
lev 4 : 0x70 : 0000 0100 - just rte
lev 5 : 0x74 : 0000 0100 - just rte
lev 6 : 0x78 : 0000 0102 - vblank?
lev 7 : 0x7c : 0000 0100 - just rte

to fix:

attribute bits on bg tiles - what are they for?

is scrolling / bg placement 100% correct?


dsw note:

the DSW ports are a bit odd, from reading the test mode it appears the
board has 2 physical dipswitches, however these are mapped over multiple
real addresses.
*/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/okim6295.h"
#include "sound/ymopm.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


// configurable logging
#define LOG_REGS1     (1U << 1)
#define LOG_REGS2     (1U << 2)

//#define VERBOSE (LOG_GENERAL | LOG_REGS1 | LOG_REGS2)

#include "logmacro.h"

#define LOGREGS1(...)     LOGMASKED(LOG_REGS1,     __VA_ARGS__)
#define LOGREGS2(...)     LOGMASKED(LOG_REGS2,     __VA_ARGS__)


namespace {

class mugsmash_state : public driver_device
{
public:
	mugsmash_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram%u", 1U),
		m_regs(*this, "regs%u", 1U),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch") { }

	void mugsmash(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_shared_ptr_array<uint16_t, 2> m_videoram;
	required_shared_ptr_array<uint16_t, 2> m_regs;
	required_shared_ptr<uint16_t> m_spriteram;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	tilemap_t *m_tilemap[2]{};

	void reg_w(offs_t offset, uint16_t data);
	void reg2_w(offs_t offset, uint16_t data);
	template <uint8_t Which> void videoram_w(offs_t offset, uint16_t data);
	template <uint8_t Which> TILE_GET_INFO_MEMBER(get_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


void mugsmash_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// Each Sprite takes 16 bytes, 5 used?

	// ---- ----  xxxx xxxx  ---- ----  aaaa aaaa  ---- ----  NNNN NNNN  ---- ----  nnnn nnnn  ---- ----  yyyy yyyy (rest unused?)

	/* x = xpos LSB
	   y = ypos LSB
	   N = tile number MSB
	   n = tile number LSB
	   a = attribute / extra
	        f?XY cccc

	    f = x-flip
	    ? = unknown, probably y-flip
	    X = xpos MSB
	    y = ypos MSB
	    c = colour

	*/

	const uint16_t *source = m_spriteram;
	const uint16_t *finish = source + 0x2000;
	gfx_element *gfx = m_gfxdecode->gfx(0);

	while (source < finish)
	{
		int xpos = source[0] & 0x00ff;
		int ypos = source[4] & 0x00ff;
		int const num = (source[3] & 0x00ff) | ((source[2] & 0x00ff) << 8);
		int const attr = source[1];
		int const flipx = (attr & 0x0080) >> 7;
		int const colour = (attr & 0x000f);

		xpos += ((attr & 0x0020) >> 5) * 0x100;
		ypos += ((attr & 0x0010) >> 4) * 0x100;

		xpos -= 28;
		ypos -= 16;


				gfx->transpen(
				bitmap,
				cliprect,
				num,
				colour,
				flipx, 0,
				xpos, ypos, 0
				);

		source += 0x8;
	}
}

template <uint8_t Which>
TILE_GET_INFO_MEMBER(mugsmash_state::get_tile_info)
{
	// fF-- cccc  nnnn nnnn

	/* c = colour?
	   n = number?
	   F = flip-X
	   f = flip-Y
	*/

	int const tileno = m_videoram[Which][tile_index * 2 + 1];
	int const colour = m_videoram[Which][tile_index * 2] & 0x000f;
	int const fx = (m_videoram[Which][tile_index * 2] & 0xc0) >> 6;

	tileinfo.set(1, tileno, 16 * Which + colour, TILE_FLIPYX(fx));
}

template <uint8_t Which>
void mugsmash_state::videoram_w(offs_t offset, uint16_t data)
{
	m_videoram[Which][offset] = data;
	m_tilemap[Which]->mark_tile_dirty(offset / 2);
}

void mugsmash_state::reg_w(offs_t offset, uint16_t data)
{
	m_regs[0][offset] = data;
	LOGREGS1("Regs %04x, %04x, %04x, %04x", m_regs[0][0], m_regs[0][1], m_regs[0][2], m_regs[0][3]);

	switch (offset)
	{
	case 0:
		m_tilemap[1]->set_scrollx(0, m_regs[0][2] + 7);
		break;
	case 1:
		m_tilemap[1]->set_scrolly(0, m_regs[0][3] + 4);
		break;
	case 2:
		m_tilemap[0]->set_scrollx(0, m_regs[0][0] + 3);
		break;
	case 3:
		m_tilemap[0]->set_scrolly(0, m_regs[0][1] + 4);
		break;
	}
}

void mugsmash_state::video_start()
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(mugsmash_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_tilemap[0]->set_transparent_pen(0);

	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(mugsmash_state::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
}

uint32_t mugsmash_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}


void mugsmash_state::reg2_w(offs_t offset, uint16_t data)
{
	m_regs[1][offset] = data;
	LOGREGS2("Regs2 %04x, %04x, %04x, %04x", m_regs[1][0], m_regs[1][1], m_regs[1][2], m_regs[1][3]);

	switch (offset)
	{
	case 1:
		m_soundlatch->write(data & 0xff);
		break;

	default:
		break;
	}

}


/* Ports mapping :

    $180000.w : 0123456789ABCDEF
                x---------------    right     (player 1)
                -x--------------    left      (player 1)
                --x-------------    up        (player 1)
                ---x------------    down      (player 1)
                ----x-----------    button 1  (player 1)
                -----x----------    button 2  (player 1)
                ------x---------    button 3  (player 1)
                -------x--------    start     (player 1)
                --------x-------    coin 1
                ---------x------    coin 2
                ----------x-----    unused
                -----------x----    unused
                ------------x---    SW1-7     ("Color Test")     *
                -------------x--    SW1-8     ("Draw SF.")       *
                --------------x-    unused
                ---------------x    unused

    $180002.w : 0123456789ABCDEF
                x---------------    right     (player 2)
                -x--------------    left      (player 2)
                --x-------------    up        (player 2)
                ---x------------    down      (player 2)
                ----x-----------    button 1  (player 2)
                -----x----------    button 2  (player 2)
                ------x---------    button 3  (player 2)
                -------x--------    start     (player 2)
                --------x-------    SW1-1     ("Test Mode")
                ---------x------    SW1-2     ("Coin/Credit")
                ----------x-----    SW1-3     ("Coin/Credit")
                -----------x----    SW1-4     ("Coin/Credit")
                ------------x---    SW1-5     ("Continue")
                -------------x--    SW1-6     ("Sound Test")     *
                --------------x-    unused
                ---------------x    unused

    $180004.w : 0123456789ABCDEF
                x---------------    unused
                -x--------------    unused
                --x-------------    unused
                ---x------------    unused
                ----x-----------    unused
                -----x----------    unused
                ------x---------    unused
                -------x--------    unused
                --------x-------    SW2-1     ("Sound Demo")
                ---------x------    SW2-2     ("Lives Num")
                ----------x-----    SW2-3     ("Lives Num")
                -----------x----    SW2-4     ("Not Used")
                ------------x---    SW2-5     ("Diff Level")
                -------------x--    SW2-6     ("Diff Level")
                --------------x-    unused
                ---------------x    unused

    $180006.w : 0123456789ABCDEF
                x---------------    unused
                -x--------------    unused
                --x-------------    unused
                ---x------------    unused
                ----x-----------    unused
                -----x----------    unused
                ------x---------    unused
                -------x--------    unused
                --------x-------    SW2-7     ("Draw Obj")       *
                ---------x------    SW2-8     ("Screen Pause")
                ----------x-----    unused
                -----------x----    unused
                ------------x---    unused
                -------------x--    unused
                --------------x-    unused
                ---------------x    unused

    * only available when you are in "test mode"

*/

void mugsmash_state::main_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x080000, 0x080fff).ram().w(FUNC(mugsmash_state::videoram_w<0>)).share(m_videoram[0]);
	map(0x082000, 0x082fff).ram().w(FUNC(mugsmash_state::videoram_w<1>)).share(m_videoram[1]);
	map(0x0c0000, 0x0c0007).w(FUNC(mugsmash_state::reg_w)).share(m_regs[0]); // video registers
	map(0x100000, 0x1005ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x140000, 0x140007).w(FUNC(mugsmash_state::reg2_w)).share(m_regs[1]); // sound + ?
	map(0x1c0000, 0x1c3fff).ram(); // main RAM?
	map(0x1c4000, 0x1cffff).ram();
	map(0x200000, 0x203fff).ram().share(m_spriteram);
	map(0x180000, 0x180001).portr("IN0");
	map(0x180002, 0x180003).portr("IN1");
	map(0x180004, 0x180005).portr("IN2");
	map(0x180006, 0x180007).portr("IN3");
}

void mugsmash_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0x8800, 0x8801).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x9800, 0x9800).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xa000, 0xa000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
}


static INPUT_PORTS_START( mugsmash )
	PORT_START("IN0")   /* IN0 - $180000.w */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x1000, 0x1000, "Color Test" )            // SW1-7 (in "test mode" only)
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, "Draw SF." )              // SW1-8 (in "test mode" only)
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")   /* IN1 - $180002.w */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 )
	PORT_SERVICE( 0x0100, IP_ACTIVE_LOW )                   // SW1-1
	PORT_DIPNAME( 0x0e00, 0x0000, DEF_STR( Coinage ) )      // SW1-2 to SW1-4
	PORT_DIPSETTING(      0x0c00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0a00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0e00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR( Allow_Continue ) )   // SW1-5
	PORT_DIPSETTING(      0x1000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x2000, 0x2000, "Sound Test" )            // SW1-6 (in "test mode" only)
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")   /* IN2 - $180004.w */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Demo_Sounds ) )  // SW2-1
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0600, 0x0200, DEF_STR( Lives ) )        // SW2-2 and SW2-3
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0200, "2" )
	PORT_DIPSETTING(      0x0400, "3" )
	PORT_DIPSETTING(      0x0600, "4" )
	PORT_DIPNAME( 0x0800, 0x0800, "Unused SW 2-4" )         // SW2-4
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x3000, 0x1000, DEF_STR( Difficulty ) )   // SW2-5 and SW2-6
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Easy) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( Very_Hard ) )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")   /* IN3 - $180006.w */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0100, 0x0100, "Draw Objects" )          // SW2-7 (in "test mode" only)
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, "Freeze" )                // SW2-8 (= "Screen Pause" in "test mode")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static const gfx_layout mugsmash_layout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 16,20,24,28,0,4,8,12,48,52,56,60,32,36,40,44 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
		8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64},
	16*64
};

static const gfx_layout mugsmash2_layout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{   0x080000*3*8,   0x080000*2*8,   0x080000*1*8,   0x080000*0*8 },
	{ 0, 1, 2, 3, 4, 5, 6, 7, 16*8+0,16*8+1,16*8+2,16*8+3,16*8+4,16*8+5,16*8+6,16*8+7  },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8
};

static GFXDECODE_START( gfx_mugsmash )
	GFXDECODE_ENTRY( "sprites", 0, mugsmash_layout,   0x00,   16  )
	GFXDECODE_ENTRY( "bgtiles", 0, mugsmash2_layout,  0x100, 256  )
GFXDECODE_END

void mugsmash_state::mugsmash(machine_config &config)
{
	M68000(config, m_maincpu, 12'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &mugsmash_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(mugsmash_state::irq6_line_hold));

	Z80(config, m_audiocpu, 4'000'000);  // guess
	m_audiocpu->set_addrmap(AS_PROGRAM, &mugsmash_state::sound_map);


	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(40*8, 32*8);
	screen.set_visarea(0*8, 40*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(mugsmash_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_mugsmash);

	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 0x300);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	ym2151_device &ymsnd(YM2151(config, "ymsnd", 3'579'545));
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "lspeaker", 1.00);   // music
	ymsnd.add_route(1, "rspeaker", 1.00);

	okim6295_device &oki(OKIM6295(config, "oki", 1'122'000, okim6295_device::PIN7_HIGH)); // clock frequency & pin 7 not verified
	oki.add_route(ALL_OUTPUTS, "lspeaker", 0.50); // sound fx
	oki.add_route(ALL_OUTPUTS, "rspeaker", 0.50);
}

ROM_START( mugsmash )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "mugs_04.bin", 0x00000, 0x40000, CRC(2498fd27) SHA1(7b746efe8aaf346e4489118ac2a3fc9929a55b83) )
	ROM_LOAD16_BYTE( "mugs_05.bin", 0x00001, 0x40000, CRC(95efb40b) SHA1(878c0a3754aa728f58044c6a7f243724b718fe1b) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Z80 code
	ROM_LOAD( "mugs_03.bin", 0x00000, 0x10000 , CRC(0101df2d) SHA1(35e1efa4a11c0f9d9db5ee057926e5de29c3a4c1) )

	ROM_REGION( 0x040000, "oki", 0 ) // samples
	ROM_LOAD( "mugs_02.bin", 0x00000, 0x20000, CRC(f92a7f4a) SHA1(3717ef64876be9ada378b449749918ce9072073a) )
	ROM_LOAD( "mugs_01.bin", 0x20000, 0x20000, CRC(1a3a0b39) SHA1(8847530027cf4be03ffbc6d78dee97b459d03a04) )

	ROM_REGION( 0x300000, "sprites", 0 )
	ROM_LOAD16_BYTE( "mugs_11.bin", 0x000000, 0x080000, CRC(1c9f5acf) SHA1(dd8329ed05a3467844c26d3f89ffb6213aba2034) )
	ROM_LOAD16_BYTE( "mugs_10.bin", 0x000001, 0x080000, CRC(6b3c22d9) SHA1(7ba6d754c08ed5b2be282ffd6a674c3a4aa0e9b2) )
	ROM_LOAD16_BYTE( "mugs_09.bin", 0x100000, 0x080000, CRC(4e9490f3) SHA1(e5f195c9bee3b92c559d1100c1019473a30ba28e) )
	ROM_LOAD16_BYTE( "mugs_08.bin", 0x100001, 0x080000, CRC(716328d5) SHA1(d1b84a25985acfb7ccb1582ef9ac8267250888c0) )
	ROM_LOAD16_BYTE( "mugs_07.bin", 0x200000, 0x080000, CRC(9e3167fd) SHA1(8c73c26e8e50e8f2ee3307f5aef23caba90c22eb) )
	ROM_LOAD16_BYTE( "mugs_06.bin", 0x200001, 0x080000, CRC(8df75d29) SHA1(d0add24ac974da4636d2631f5590516de0f8df4a) )

	ROM_REGION( 0x200000, "bgtiles", 0 )
	ROM_LOAD( "mugs_12.bin", 0x000000, 0x080000, CRC(c0a6ed98) SHA1(13850c6bcca65bdc782040c470c4966aee19551d) )
	ROM_LOAD( "mugs_13.bin", 0x080000, 0x080000, CRC(e2be8595) SHA1(077b1a262c54acf74e6ec37702bcfed41bc31000) )
	ROM_LOAD( "mugs_14.bin", 0x100000, 0x080000, CRC(24e81068) SHA1(1e33aa7d2b873dd13d5823880c46d3d3e867d6b6) )
	ROM_LOAD( "mugs_15.bin", 0x180000, 0x080000, CRC(82e8187c) SHA1(c7a0e1b3d90dbbe2588886a27a07a9c336447ae3) )
ROM_END

} // anonymous namespace


GAME( 1990?, mugsmash, 0, mugsmash, mugsmash, mugsmash_state, empty_init, ROT0, "Electronic Devices Italy / 3D Games England", "Mug Smashers", MACHINE_SUPPORTS_SAVE )
