// license:BSD-3-Clause
// copyright-holders:Steve Ellenoff, Brad Oliver
/***************************************************************************

Tag Team Wrestling hardware description:

This hardware is very similar to the BurgerTime/Lock N Chase family of games
but there are just enough differences to make it a pain to share the
codebase. It looks like this hardware is a bridge between the BurgerTime
family and the later Technos games, like Mat Mania and Mysterious Stones.

The video hardware supports 3 sprite banks instead of 1
The sound hardware appears nearly identical to Mat Mania


Stephh's notes :

  - When the "Cabinet" Dip Switch is set to "Cocktail", the screen status
    depends on which player is the main wrestler on the ring :

      * player 1 : normal screen
      * player 2 : inverted screen

TODO:
        * fix hi-score (reset) bug

***************************************************************************/

#include "emu.h"

#include "cpu/m6502/m6502.h"
#include "machine/gen_latch.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "video/resnet.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class tagteam_state : public driver_device
{
public:
	tagteam_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram")
	{ }

	void tagteam(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;

	uint8_t m_palettebank = 0U;
	tilemap_t *m_bg_tilemap = nullptr;
	uint8_t m_sound_nmi_mask = 0U;

	void irq_clear_w(uint8_t data);
	void sound_nmi_mask_w(uint8_t data);
	void videoram_w(offs_t offset, uint8_t data);
	void colorram_w(offs_t offset, uint8_t data);
	uint8_t mirrorvideoram_r(offs_t offset);
	uint8_t mirrorcolorram_r(offs_t offset);
	void mirrorvideoram_w(offs_t offset, uint8_t data);
	void mirrorcolorram_w(offs_t offset, uint8_t data);
	void control_w(uint8_t data);
	void flipscreen_w(uint8_t data);

	INTERRUPT_GEN_MEMBER(sound_timer_irq);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	void palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


// TODO: fix or confirm resnet implementation
// schematics say emitter circuit with 47 ohm pullup and 470 ohm pulldown but this results in a bad palette
static const res_net_info tagteam_net_info =
{
	RES_NET_VCC_5V | RES_NET_VBIAS_5V | RES_NET_VIN_TTL_OUT,
	{
		{ RES_NET_AMP_EMITTER, 4700, 0, 3, { 4700, 3300, 1500 } },
		{ RES_NET_AMP_EMITTER, 4700, 0, 3, { 4700, 3300, 1500 } },
		{ RES_NET_AMP_EMITTER, 4700, 0, 2, {       3300, 1500 } }
	}
};

static const res_net_decode_info tagteam_decode_info =
{
	1,              // single PROM per color
	0x000, 0x01f,   // start/end
	/* R     G     B */
	{  0x00, 0x00, 0x00 }, // offsets
	{  0x00, 0x03, 0x06 }, // shifts
	{  0x07, 0x07, 0x03 }  // masks
};

void tagteam_state::palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();

	std::vector<rgb_t> rgb;
	compute_res_net_all(rgb, color_prom, tagteam_decode_info, tagteam_net_info);
	palette.set_pen_colors(0x00, rgb);
}


void tagteam_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void tagteam_state::colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

uint8_t tagteam_state::mirrorvideoram_r(offs_t offset)
{
	// swap x and y coordinates
	int const x = offset / 32;
	int const y = offset % 32;
	offset = 32 * y + x;

	return m_videoram[offset];
}

uint8_t tagteam_state::mirrorcolorram_r(offs_t offset)
{
	// swap x and y coordinates
	int const x = offset / 32;
	int const y = offset % 32;
	offset = 32 * y + x;

	return m_colorram[offset];
}

void tagteam_state::mirrorvideoram_w(offs_t offset, uint8_t data)
{
	// swap x and y coordinates
	int const x = offset / 32;
	int const y = offset % 32;
	offset = 32 * y + x;

	videoram_w(offset, data);
}

void tagteam_state::mirrorcolorram_w(offs_t offset, uint8_t data)
{
	// swap x and y coordinates
	int const x = offset / 32;
	int const y = offset % 32;
	offset = 32 * y + x;

	colorram_w(offset, data);
}

void tagteam_state::control_w(uint8_t data)
{
	// d0-3: color for blank screen, applies to h/v borders too
	// (not implemented yet, and tagteam doesn't have a global screen on/off bit)

	// d7: palette bank
	m_palettebank = (data & 0x80) >> 7;
}

void tagteam_state::flipscreen_w(uint8_t data)
{
	// d0: flip screen
	if (flip_screen() != (data & 0x01))
	{
		flip_screen_set(data & 0x01);
		machine().tilemap().mark_all_dirty();
	}

	// d6/7: coin counters
	machine().bookkeeping().coin_counter_w(0, data & 0x80);
	machine().bookkeeping().coin_counter_w(1, data & 0x40);
}

TILE_GET_INFO_MEMBER(tagteam_state::get_bg_tile_info)
{
	int const code = m_videoram[tile_index] + 256 * m_colorram[tile_index];
	int const color = m_palettebank << 1;

	tileinfo.set(0, code, color, 0);
}

void tagteam_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tagteam_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS_FLIP_X,
			8, 8, 32, 32);

	save_item(NAME(m_palettebank));
}

void tagteam_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = 0; offs < 0x20; offs += 4)
	{
		int const spritebank = (m_videoram[offs] & 0x30) << 4;
		int code = m_videoram[offs + 1] + 256 * spritebank;
		int color = m_palettebank << 1 | 1;
		int flipx = m_videoram[offs] & 0x04;
		int flipy = m_videoram[offs] & 0x02;
		int sx = 240 - m_videoram[offs + 3];
		int sy = 240 - m_videoram[offs + 2];

		if (!(m_videoram[offs] & 0x01)) continue;

		if (flip_screen())
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}


		m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
		code, color,
		flipx, flipy,
		sx, sy, 0);

		// Wrap around

		code = m_videoram[offs + 0x20] + 256 * spritebank;
		color = m_palettebank;
		sy += (flip_screen() ? -256 : 256);


			m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
			code, color,
			flipx, flipy,
			sx, sy, 0);
	}
}

uint32_t tagteam_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}


void tagteam_state::machine_start()
{
	save_item(NAME(m_sound_nmi_mask));
}

void tagteam_state::irq_clear_w(uint8_t data)
{
	m_maincpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
}

void tagteam_state::main_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x2000, 0x2000).portr("P2").w(FUNC(tagteam_state::flipscreen_w));
	map(0x2001, 0x2001).portr("P1").w(FUNC(tagteam_state::control_w));
	map(0x2002, 0x2002).portr("DSW1").w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x2003, 0x2003).portr("DSW2").w(FUNC(tagteam_state::irq_clear_w));
	map(0x4000, 0x43ff).rw(FUNC(tagteam_state::mirrorvideoram_r), FUNC(tagteam_state::mirrorvideoram_w));
	map(0x4400, 0x47ff).rw(FUNC(tagteam_state::mirrorcolorram_r), FUNC(tagteam_state::mirrorcolorram_w));
	map(0x4800, 0x4bff).ram().w(FUNC(tagteam_state::videoram_w)).share(m_videoram);
	map(0x4c00, 0x4fff).ram().w(FUNC(tagteam_state::colorram_w)).share(m_colorram);
	map(0x8000, 0xffff).rom();
}

void tagteam_state::sound_nmi_mask_w(uint8_t data)
{
	m_sound_nmi_mask = data & 1;
}

// Same as Syusse Oozumou
void tagteam_state::sound_map(address_map &map)
{
	map(0x0000, 0x03ff).ram();
	map(0x2000, 0x2001).w("ay1", FUNC(ay8910_device::data_address_w));
	map(0x2002, 0x2003).w("ay2", FUNC(ay8910_device::data_address_w));
	map(0x2004, 0x2004).w("dac", FUNC(dac_byte_interface::data_w));
	map(0x2005, 0x2005).w(FUNC(tagteam_state::sound_nmi_mask_w));
	map(0x2007, 0x2007).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x4000, 0xffff).rom();
}


INPUT_CHANGED_MEMBER(tagteam_state::coin_inserted)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? CLEAR_LINE : ASSERT_LINE);
}

static INPUT_PORTS_START( bigprowr )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, tagteam_state, coin_inserted, 0)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, tagteam_state, coin_inserted, 0)

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_COCKTAIL PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_COCKTAIL PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_COCKTAIL PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )        PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )      // "Upright, Single Controls"
	PORT_DIPSETTING(    0x40, "Upright, Dual Controls" )
	PORT_DIPSETTING(    0x20, "Cocktail, Single Controls" ) // IMPOSSIBLE !
	PORT_DIPSETTING(    0x60, DEF_STR( Cocktail ) )     // "Cocktail, Dual Controls"
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM  ) PORT_VBLANK("screen")

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x00, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x00, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x00, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x00, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x00, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x00, "SW2:8" )
INPUT_PORTS_END

// Same as 'bigprowr', but additional "Coin Mode" Dip Switch
static INPUT_PORTS_START( tagteam )
	PORT_INCLUDE( bigprowr )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )    PORT_CONDITION("DSW2", 0xe0, NOTEQUALS, 0x80) // Mode 1
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_6C ) )    PORT_CONDITION("DSW2", 0xe0, EQUALS, 0x80)    // Mode 2
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )    PORT_CONDITION("DSW2", 0xe0, NOTEQUALS, 0x80) // Mode 1
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_6C ) )    PORT_CONDITION("DSW2", 0xe0, EQUALS, 0x80)    // Mode 2

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0xe0, 0x00, "Coin Mode" )         PORT_DIPLOCATION("SW2:6,7,8")
	PORT_DIPSETTING(    0x00, "Mode 1" )
	PORT_DIPSETTING(    0x80, "Mode 2" )
	// Check code at 0xff5c
	// Other values (0x20, 0x40, 0x60, 0xa0, 0xc0, 0xe0) : "Mode 1"
	// Therefore the logic for DIPCONDITION is '=0x80' not '<>0x00'
INPUT_PORTS_END



static const gfx_layout spritelayout =
{
	16,16,  // 16*16 sprites
	768,    // 768 sprites
	3,      // 3 bits per pixel
	{ 2*768*16*16, 768*16*16, 0 },  // the bitplanes are separated
	{ 16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7,
			0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8    // every sprite takes 32 consecutive bytes
};

static GFXDECODE_START( gfx_tagteam )
	GFXDECODE_ENTRY( "gfx", 0, gfx_8x8x3_planar, 0, 4 ) // chars
	GFXDECODE_ENTRY( "gfx", 0, spritelayout,     0, 4 ) // sprites
GFXDECODE_END


INTERRUPT_GEN_MEMBER(tagteam_state::sound_timer_irq)
{
	if (m_sound_nmi_mask)
		device.execute().pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}


void tagteam_state::tagteam(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, XTAL(12'000'000) / 8);
	m_maincpu->set_addrmap(AS_PROGRAM, &tagteam_state::main_map);
	m_maincpu->set_periodic_int(FUNC(tagteam_state::irq0_line_assert), attotime::from_hz(272 / 16 * 57)); // connected to bit 4 of vcount (basically once every 16 scanlines)

	M6502(config, m_audiocpu, XTAL(12'000'000) / 2 / 6); // daughterboard gets 12mhz / 2 from mainboard, but how it's divided further is a guess
	m_audiocpu->set_addrmap(AS_PROGRAM, &tagteam_state::sound_map);
	m_audiocpu->set_periodic_int(FUNC(tagteam_state::sound_timer_irq), attotime::from_hz(272 / 16 * 57)); // same source as maincpu IRQ

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(57); // measured?
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(3072));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(tagteam_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_tagteam);
	PALETTE(config, m_palette, FUNC(tagteam_state::palette), 32);

	// sound hardware
	SPEAKER(config, "speaker").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, M6502_IRQ_LINE);

	AY8910(config, "ay1", XTAL(12'000'000) / 8).add_route(ALL_OUTPUTS, "speaker", 0.25);
	AY8910(config, "ay2", XTAL(12'000'000) / 8).add_route(ALL_OUTPUTS, "speaker", 0.25);

	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.25); // unknown DAC
}



ROM_START( bigprowr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bf00-1.20",    0x08000, 0x2000, CRC(8aba32c9) SHA1(9228082a8251feaf25849311c3de63ca42cf659e) )
	ROM_LOAD( "bf01.33",      0x0a000, 0x2000, CRC(0a41f3ae) SHA1(1b82cd864f0bd7f16f961fec0b88307996abb166) )
	ROM_LOAD( "bf02.34",      0x0c000, 0x2000, CRC(a28b0a0e) SHA1(50b40048a3e2efb2afb7acfb4efde6dbc25fc009) )
	ROM_LOAD( "bf03.46",      0x0e000, 0x2000, CRC(d4cf7ec7) SHA1(cfabe40adb05f6239c3e2f002a78efb50150d27d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "bf4.8",        0x04000, 0x2000, CRC(0558e1d8) SHA1(317011c0e3a9d5f73c67d044c1fab315ff8049fb) )
	ROM_LOAD( "bf5.7",        0x06000, 0x2000, CRC(c1073f24) SHA1(0337c259c10fae3067e5e0e0acf54e6d0891b29f) )
	ROM_LOAD( "bf6.6",        0x08000, 0x2000, CRC(208cd081) SHA1(e5f6379e7f7bc80cdea12de7e0a2bb232bb16b5a) )
	ROM_LOAD( "bf7.3",        0x0a000, 0x2000, CRC(34a033dc) SHA1(01e4c331233a2337c7c53edd221bb87859278b04) )
	ROM_LOAD( "bf8.2",        0x0c000, 0x2000, CRC(eafe8056) SHA1(4a2d1c903e4acee962aeb0f0f18333252790f686) )
	ROM_LOAD( "bf9.1",        0x0e000, 0x2000, CRC(d589ce1b) SHA1(c2aca1cc6867d4d6d6e02ac29a4c53c667bf6d89) )

	ROM_REGION( 0x12000, "gfx", 0 )
	ROM_LOAD( "bf10.89",      0x00000, 0x2000, CRC(b1868746) SHA1(a3eff1b00f9ac2512d6ca81f9223723642d4f06a) )
	ROM_LOAD( "bf11.94",      0x02000, 0x2000, CRC(c3fe99c1) SHA1(beb3056b37f26a52f3c0907868054b3cc3c4e3ea) )
	ROM_LOAD( "bf12.103",     0x04000, 0x2000, CRC(c8717a46) SHA1(6de0238071aacb443234d9a7ef250ddfaa9dd1a8) )
	ROM_LOAD( "bf13.91",      0x06000, 0x2000, CRC(23ee34d3) SHA1(2af79845c2d4f06eb85db14d67ec4a499fc272b1) )
	ROM_LOAD( "bf14.95",      0x08000, 0x2000, CRC(a6721142) SHA1(200058d7b688dccda0ab0f568ab6c6c215a23e0a) )
	ROM_LOAD( "bf15.105",     0x0a000, 0x2000, CRC(60ae1078) SHA1(f9c162ff0830aff26d121f04c107dadb060d4bd5) )
	ROM_LOAD( "bf16.93",      0x0c000, 0x2000, CRC(d33dc245) SHA1(3a40ca7f7e17eaaea6e3f90fcf4cb9a72fc4ba8f) )
	ROM_LOAD( "bf17.96",      0x0e000, 0x2000, CRC(ccf42380) SHA1(6a8958201125c1b13b1354c98adc573dbea64d56) )
	ROM_LOAD( "bf18.107",     0x10000, 0x2000, CRC(fd6f006d) SHA1(ad100ac8c0fed24f922a2cc908c88b4fced07eb0) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "fko.8",        0x0000, 0x0020, CRC(b6ee1483) SHA1(b2ea7be533e29da6cd7302532da2eb0410490e6a) ) // palette
	ROM_LOAD( "fjo.25",       0x0020, 0x0020, CRC(24da2b63) SHA1(4db7e1ff1b9fd5ae4098cd7ca66cf1fa2574501a) ) // hcount related, not implemented yet

	ROM_REGION( 0x0104, "plds", 0 )
	ROM_LOAD( "pal16r4.ic57", 0x0000, 0x0104, NO_DUMP )
ROM_END

ROM_START( bigprowra ) // TA-007-P1-1 + TA-007-P1-2 PCBs. Dumper thinks it may be a field test PCB but it has a high serial number
	ROM_REGION( 0x10000, "maincpu", 0 ) // handwritten labels
	ROM_LOAD( "xa-0.ic20", 0x08000, 0x2000, CRC(4f711b0a) SHA1(890655279311925a054181453f90050b7084d623) )
	ROM_LOAD( "xa-1.ic33", 0x0a000, 0x2000, CRC(5471efc3) SHA1(9b5246f60ba73521b1e98aeb4503665579315153) )
	ROM_LOAD( "xa-2.ic34", 0x0c000, 0x2000, CRC(353ed84f) SHA1(73c09f35aa5ec1a881a4f47239c032927f042fb4) )
	ROM_LOAD( "xa-3.ic46", 0x0e000, 0x2000, CRC(4f1b7203) SHA1(154886bc65b6820555cd3d072135514254d0eaa0) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "bf04-.ic8", 0x04000, 0x2000, CRC(0558e1d8) SHA1(317011c0e3a9d5f73c67d044c1fab315ff8049fb) )
	ROM_LOAD( "bf05-.ic7", 0x06000, 0x2000, CRC(c1073f24) SHA1(0337c259c10fae3067e5e0e0acf54e6d0891b29f) )
	ROM_LOAD( "bf06-.ic6", 0x08000, 0x2000, CRC(208cd081) SHA1(e5f6379e7f7bc80cdea12de7e0a2bb232bb16b5a) )
	ROM_LOAD( "bf07-.ic3", 0x0a000, 0x2000, CRC(34a033dc) SHA1(01e4c331233a2337c7c53edd221bb87859278b04) )
	ROM_LOAD( "bf08-.ic2", 0x0c000, 0x2000, CRC(eafe8056) SHA1(4a2d1c903e4acee962aeb0f0f18333252790f686) ) // this ROM wouldn't give consistent reads, however since all other audio CPU ROMs match the other sets..
	ROM_LOAD( "bf09-.ic1", 0x0e000, 0x2000, CRC(d589ce1b) SHA1(c2aca1cc6867d4d6d6e02ac29a4c53c667bf6d89) )

	ROM_REGION( 0x12000, "gfx", 0 ) // labels with x are handwritten
	ROM_LOAD( "x-1.ic89",    0x00000, 0x2000, CRC(48165902) SHA1(3145fc83f17712b460a08b882677cfcac08fc272) )
	ROM_LOAD( "bf11-.ic94",  0x02000, 0x2000, CRC(c3fe99c1) SHA1(beb3056b37f26a52f3c0907868054b3cc3c4e3ea) )
	ROM_LOAD( "bf12-.ic103", 0x04000, 0x2000, CRC(c8717a46) SHA1(6de0238071aacb443234d9a7ef250ddfaa9dd1a8) )
	ROM_LOAD( "x-2.ic91",    0x06000, 0x2000, CRC(ecfa581d) SHA1(1352c6a5f8e6f2d3fbe9f9e74c542ea2467f1438) )
	ROM_LOAD( "bf14-.ic95",  0x08000, 0x2000, CRC(a6721142) SHA1(200058d7b688dccda0ab0f568ab6c6c215a23e0a) )
	ROM_LOAD( "bf15-.ic105", 0x0a000, 0x2000, CRC(60ae1078) SHA1(f9c162ff0830aff26d121f04c107dadb060d4bd5) )
	ROM_LOAD( "x-3.ic93",    0x0c000, 0x2000, CRC(75ee5705) SHA1(95fdea3768f2d5b81ba5dafd3b13a061cd96689a) )
	ROM_LOAD( "bf17-.ic96",  0x0e000, 0x2000, CRC(ccf42380) SHA1(6a8958201125c1b13b1354c98adc573dbea64d56) )
	ROM_LOAD( "bf18-.ic107", 0x10000, 0x2000, CRC(fd6f006d) SHA1(ad100ac8c0fed24f922a2cc908c88b4fced07eb0) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "fko.ic8",  0x0000, 0x0020, CRC(b6ee1483) SHA1(b2ea7be533e29da6cd7302532da2eb0410490e6a) ) // n82s123n, palette
	ROM_LOAD( "fjo.ic25", 0x0020, 0x0020, CRC(24da2b63) SHA1(4db7e1ff1b9fd5ae4098cd7ca66cf1fa2574501a) ) // n82s123n, hcount related, not implemented yet

	ROM_REGION( 0x0104, "plds", 0 )
	ROM_LOAD( "pal16r4.ic57", 0x0000, 0x0104, NO_DUMP )
ROM_END

ROM_START( tagteam )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "prowbf0.bin",  0x08000, 0x2000, CRC(6ec3afae) SHA1(8ae11cb41a72bda053ce8b79c383503da5324cd1) )
	ROM_LOAD( "prowbf1.bin",  0x0a000, 0x2000, CRC(b8fdd176) SHA1(afa8e890ac54101eef0274c8aabe25d188085a18) )
	ROM_LOAD( "prowbf2.bin",  0x0c000, 0x2000, CRC(3d33a923) SHA1(e6402290fca72f4fa3a76e37957b9d4f5b4aeddb) )
	ROM_LOAD( "prowbf3.bin",  0x0e000, 0x2000, CRC(518475d2) SHA1(b26bb0bb658bfd5ac24ee8ebb7fc11a79917aeda) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "bf4.8",        0x04000, 0x2000, CRC(0558e1d8) SHA1(317011c0e3a9d5f73c67d044c1fab315ff8049fb) )
	ROM_LOAD( "bf5.7",        0x06000, 0x2000, CRC(c1073f24) SHA1(0337c259c10fae3067e5e0e0acf54e6d0891b29f) )
	ROM_LOAD( "bf6.6",        0x08000, 0x2000, CRC(208cd081) SHA1(e5f6379e7f7bc80cdea12de7e0a2bb232bb16b5a) )
	ROM_LOAD( "bf7.3",        0x0a000, 0x2000, CRC(34a033dc) SHA1(01e4c331233a2337c7c53edd221bb87859278b04) )
	ROM_LOAD( "bf8.2",        0x0c000, 0x2000, CRC(eafe8056) SHA1(4a2d1c903e4acee962aeb0f0f18333252790f686) )
	ROM_LOAD( "bf9.1",        0x0e000, 0x2000, CRC(d589ce1b) SHA1(c2aca1cc6867d4d6d6e02ac29a4c53c667bf6d89) )

	ROM_REGION( 0x12000, "gfx", 0 )
	ROM_LOAD( "prowbf10.bin", 0x00000, 0x2000, CRC(48165902) SHA1(3145fc83f17712b460a08b882677cfcac08fc272) )
	ROM_LOAD( "bf11.94",      0x02000, 0x2000, CRC(c3fe99c1) SHA1(beb3056b37f26a52f3c0907868054b3cc3c4e3ea) )
	ROM_LOAD( "prowbf12.bin", 0x04000, 0x2000, CRC(69de1ea2) SHA1(7b696c74e29c0bae33b386da463365e7e796c6a0) )
	ROM_LOAD( "prowbf13.bin", 0x06000, 0x2000, CRC(ecfa581d) SHA1(1352c6a5f8e6f2d3fbe9f9e74c542ea2467f1438) )
	ROM_LOAD( "bf14.95",      0x08000, 0x2000, CRC(a6721142) SHA1(200058d7b688dccda0ab0f568ab6c6c215a23e0a) )
	ROM_LOAD( "prowbf15.bin", 0x0a000, 0x2000, CRC(d0de7e03) SHA1(79267c09621f8f5255c82a69790d5b52b1c5dd6e) )
	ROM_LOAD( "prowbf16.bin", 0x0c000, 0x2000, CRC(75ee5705) SHA1(95fdea3768f2d5b81ba5dafd3b13a061cd96689a) )
	ROM_LOAD( "bf17.96",      0x0e000, 0x2000, CRC(ccf42380) SHA1(6a8958201125c1b13b1354c98adc573dbea64d56) )
	ROM_LOAD( "prowbf18.bin", 0x10000, 0x2000, CRC(e73a4bba) SHA1(6dbc2d741ebf8fcce9144cfe6fe6f35acd25ceef) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "fko.8",        0x0000, 0x0020, CRC(b6ee1483) SHA1(b2ea7be533e29da6cd7302532da2eb0410490e6a) ) // palette
	ROM_LOAD( "fjo.25",       0x0020, 0x0020, CRC(24da2b63) SHA1(4db7e1ff1b9fd5ae4098cd7ca66cf1fa2574501a) ) // hcount related, not implemented yet

	ROM_REGION( 0x0104, "plds", 0 )
	ROM_LOAD( "pal16r4.ic57", 0x0000, 0x0104, NO_DUMP )
ROM_END

} // anonymous namespace


GAME( 1983, bigprowr,  0,        tagteam, bigprowr, tagteam_state, empty_init, ROT270, "Technos Japan",                     "The Big Pro Wrestling! (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, bigprowra, bigprowr, tagteam, bigprowr, tagteam_state, empty_init, ROT270, "Technos Japan",                     "The Big Pro Wrestling! (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, tagteam,   bigprowr, tagteam, tagteam,  tagteam_state, empty_init, ROT270, "Technos Japan (Data East license)", "Tag Team Wrestling",             MACHINE_SUPPORTS_SAVE )
