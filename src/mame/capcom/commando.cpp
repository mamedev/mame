// license:BSD-3-Clause
// copyright-holders: Nicola Salmoria

/***************************************************************************

Commando memory map (preliminary)

driver by Nicola Salmoria


MAIN CPU
0000-bfff ROM
d000-d3ff Video RAM
d400-d7ff Color RAM
d800-dbff background video RAM
dc00-dfff background color RAM
e000-ffff RAM
fe00-ff7f Sprites

read:
c000      IN0
c001      IN1
c002      IN2
c003      DSW1
c004      DSW2

write:
c808-c809 background scroll x position
c80a-c80b background scroll y position

SOUND CPU
0000-3fff ROM
4000-47ff RAM

write:
8000      YM2203 #1 control
8001      YM2203 #1 write
8002      YM2203 #2 control
8003      YM2203 #2 write

****************************************************************************

Note : there is an ingame typo bug that doesn't display the bonus life values
       correctly on the title screen in 'commando', 'commandoj' and 'spaceinv'.

***************************************************************************/

// Notes by Jose Tejada (jotego):
// The main CPU frequency is 3 MHz, after a two-stage FF clock divider.
// The CPU clock is gated by bus arbitration logic. The CPU clock is halted until
// video hardware has an opening in memory access, then the CPU is allowed to
// access common memory. This slows down the CPU but doesn't alter its basic 3MHz frequency.
//
// There is also a DMA circuit that copies object data from the CPU RAM to a buffer
// this also slows down the CPU as it is halted during that time.

/***************************************************************************

Commando (Capcom, 1985)
Hardware info by Guru

PCB Layout
----------

Top board (parts face up)

85605-C-1
   |----------|           |----------|
|--|----------|-----------|----------|--|
|                                       |
|                                       |
|            MB8416                     |
|                                       |
| VT11.5A                               |
|                                       |
| VT12.6A                               |
|                                       |
| VT13.7A                               |
|                                       |
| VT14.8A                               |
|                                       |
| VT15.9A                               |
|                                       |
| VT16.10A                              |
|---------------------------------------|
Notes:
      MB8416 - Fujitsu MB8416 2kBx8-bit SRAM (background tile RAM)
         VT* - 27C128 EPROM (background tiles)


Middle board (parts face up)

85605-A-2           |----------|           |----------|
|-------------------|----------|-----------|----------|--|
|DIP-A  DIP-B  24S10.1D             24S10.1H        12MHz|
|              24S10.2D                                  |
|              24S10.3D                                  |
|                                                        |
|                                                        |
|5             VT01.5D                                   |
|6                                                       |
|P                                                       |
|I                                          24S10.6L     |
|N                                                       |
|                        MB8128(1)                  6264 |
|      LM324  YM3014     MB8128(2)                CM03.8M|
|               YM3014   CM02.9F                  CM04.9M|
|               YM2203                            CUSTOM |
|HA1368 VOL VOL YM2203   Z80A(2)                  Z80A(1)|
|--------------------------------------------------------|
Notes:
        Z80A(1) - Z80A CPU. Clock 3.000MHz [12/4] (main CPU)
        Z80A(2) - Z80A CPU. Clock 3.000MHz [12/4] (sound CPU)
         YM2203 - Yamaha YM2203 FM Operator Type-N(OPN) sound chip. Clock 1.500MHz [12/8]
         YM3014 - Yamaha YM3014 Serial Input Floating D/A Converter. Clock 1.000MHz [12/12]
      MB8128(1) - Mitsubishi M58725 2kBx8-bit SRAM (character/text layer RAM)
      MB8128(2) - Mitsubishi M58725 2kBx8-bit SRAM (sound program RAM)
           6264 - Hitachi HM6264 8kBx8-bit SRAM (main program RAM)
         CUSTOM - DIP48 custom chip. On bootlegs this is replaced by a PAL, 74LS245 and 74LS08 on a plug-in sub-board
        CM02.9F - 27C128 EPROM (sound program)
        CM03.8M - 27C128 EPROM (main program)
        CM04.9M - 27C256 EPROM (main program)
        VT01.5D - 27C128 EPROM (characters/text layer)
           DIP* - 8-position DIP switch
          LM324 - Texas Instruments LM324 Quad Operational Amplifier
         HA1368 - Hitachi HA1368 5.3W Mono Audio Power Amplifier
          56PIN - 56-pin edge connector (not JAMMA)
       24S10.1D - Texas Instruments TBP24S10 bi-polar PROM (red color PROM)
       24S10.2D - Texas Instruments TBP24S10 bi-polar PROM (green color PROM)
       24S10.3D - Texas Instruments TBP24S10 bi-polar PROM (blue color PROM)
       24S10.1H - Texas Instruments TBP24S10 bi-polar PROM (palette selector PROM)
       24S10.6L - Texas Instruments TBP24S10 bi-polar PROM (interrupt timing PROM)
          HSync - 15.6196kHz
          VSync - 59.6170Hz


Bottom board (parts face up)

85605-B-1           |----------|           |----------|
|-------------------|----------|-----------|----------|--|
|2114                                                    |
|2114                                                    |
|                                                        |
|                                                        |
|                                2114 2114 2148          |
|                                2114 2114 2148          |
|                                                        |
|                                                        |
|                  24S10.6E                              |
|                                                        |
|                  VT05.7E  VT08.7H                      |
|                                                        |
|                  VT06.8E  VT09.8H                      |
|                                              2148 2148 |
|                  VT07.9E  VT10.9H                      |
|--------------------------------------------------------|
Notes:
      24S10.6E - Texas Instruments TBP24S10 bi-polar PROM (video timing PROM)
           VT* - 27C128 EPROM (sprites)
          2114 - Mitsubishi M5L2114 1kBx4-bit SRAM (sprite RAM)
          2148 - Intel P2148 1kBx4-bit SRAM (sprite RAM)

***************************************************************************/


#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/ymopn.h"
#include "video/bufsprite.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class commando_state : public driver_device
{
public:
	commando_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram%u", 1U),
		m_colorram(*this, "colorram%u", 1U),
		m_decrypted_opcodes(*this, "decrypted_opcodes")
	{ }

	void init_spaceinv();
	void init_commando();

	void commando(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<buffered_spriteram8_device> m_spriteram;

	// memory pointers
	required_shared_ptr_array<uint8_t, 2> m_videoram;
	required_shared_ptr_array<uint8_t, 2> m_colorram;
	required_shared_ptr<uint8_t> m_decrypted_opcodes;

	// video-related
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;
	uint8_t m_scroll_x[2]{};
	uint8_t m_scroll_y[2]{};

	template <uint8_t Which> void videoram_w(offs_t offset, uint8_t data);
	template <uint8_t Which> void colorram_w(offs_t offset, uint8_t data);
	void scrollx_w(offs_t offset, uint8_t data);
	void scrolly_w(offs_t offset, uint8_t data);
	void control_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank_irq(int state);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
	void decrypted_opcodes_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


template <uint8_t Which>
void commando_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[Which][offset] = data;
	Which ? m_fg_tilemap->mark_tile_dirty(offset) : m_bg_tilemap->mark_tile_dirty(offset);
}

template <uint8_t Which>
void commando_state::colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[Which][offset] = data;
	Which ? m_fg_tilemap->mark_tile_dirty(offset) : m_bg_tilemap->mark_tile_dirty(offset);
}

void commando_state::scrollx_w(offs_t offset, uint8_t data)
{
	m_scroll_x[offset] = data;
	m_bg_tilemap->set_scrollx(0, m_scroll_x[0] | (m_scroll_x[1] << 8));
}

void commando_state::scrolly_w(offs_t offset, uint8_t data)
{
	m_scroll_y[offset] = data;
	m_bg_tilemap->set_scrolly(0, m_scroll_y[0] | (m_scroll_y[1] << 8));
}

void commando_state::control_w(uint8_t data)
{
	// bits 0 and 1 are coin counters
	machine().bookkeeping().coin_counter_w(0, data & 0x01);
	machine().bookkeeping().coin_counter_w(1, data & 0x02);

	// bit 4 resets the sound CPU
	m_audiocpu->set_input_line(INPUT_LINE_RESET, (data & 0x10) ? ASSERT_LINE : CLEAR_LINE);

	// bit 7 flips screen
	flip_screen_set(data & 0x80);
}

TILE_GET_INFO_MEMBER(commando_state::get_bg_tile_info)
{
	int const attr = m_colorram[0][tile_index];
	int const code = m_videoram[0][tile_index] + ((attr & 0xc0) << 2);
	int const color = attr & 0x0f;
	int const flags = TILE_FLIPYX((attr & 0x30) >> 4);

	tileinfo.set(1, code, color, flags);
}

TILE_GET_INFO_MEMBER(commando_state::get_fg_tile_info)
{
	int const attr = m_colorram[1][tile_index];
	int const code = m_videoram[1][tile_index] + ((attr & 0xc0) << 2);
	int const color = attr & 0x0f;
	int const flags = TILE_FLIPYX((attr & 0x30) >> 4);

	tileinfo.set(0, code, color, flags);
}

void commando_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(commando_state::get_bg_tile_info)), TILEMAP_SCAN_COLS, 16, 16, 32, 32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(commando_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_bg_tilemap->set_scrolldx(128, 128);
	m_bg_tilemap->set_scrolldy(6, 6);
	m_fg_tilemap->set_scrolldx(128, 128);
	m_fg_tilemap->set_scrolldy(6, 6);

	m_fg_tilemap->set_transparent_pen(3);
}

void commando_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t *buffered_spriteram = m_spriteram->buffer();

	for (int offs = m_spriteram->bytes() - 4; offs >= 0; offs -= 4)
	{
		// bit 1 of attr is not used
		int const attr = buffered_spriteram[offs + 1];
		int const bank = (attr & 0xc0) >> 6;
		int const code = buffered_spriteram[offs] + 256 * bank;
		int const color = (attr & 0x30) >> 4;
		int flipx = attr & 0x04;
		int flipy = attr & 0x08;
		int sx = buffered_spriteram[offs + 3] - ((attr & 0x01) << 8);
		int sy = buffered_spriteram[offs + 2];

		if (flip_screen())
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		if (bank < 3)
			m_gfxdecode->gfx(2)->transpen(bitmap, cliprect, code, color, flipx, flipy, sx + 128, sy + 6, 15);
	}
}

uint32_t commando_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


// Memory Maps

void commando_state::main_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xc000).portr("SYSTEM");
	map(0xc001, 0xc001).portr("P1");
	map(0xc002, 0xc002).portr("P2");
	map(0xc003, 0xc003).portr("DSW1");
	map(0xc004, 0xc004).portr("DSW2");
	map(0xc800, 0xc800).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0xc804, 0xc804).w(FUNC(commando_state::control_w));
	// 0xc806 triggers the DMA (not emulated)
	map(0xc808, 0xc809).w(FUNC(commando_state::scrollx_w));
	map(0xc80a, 0xc80b).w(FUNC(commando_state::scrolly_w));
	map(0xd000, 0xd3ff).ram().w(FUNC(commando_state::videoram_w<1>)).share(m_videoram[1]);
	map(0xd400, 0xd7ff).ram().w(FUNC(commando_state::colorram_w<1>)).share(m_colorram[1]);
	map(0xd800, 0xdbff).ram().w(FUNC(commando_state::videoram_w<0>)).share(m_videoram[0]);
	map(0xdc00, 0xdfff).ram().w(FUNC(commando_state::colorram_w<0>)).share(m_colorram[0]);
	map(0xe000, 0xfdff).ram();
	map(0xfe00, 0xff7f).ram().share("spriteram");
	map(0xff80, 0xffff).ram();
}

void commando_state::decrypted_opcodes_map(address_map &map)
{
	map(0x0000, 0xbfff).rom().share(m_decrypted_opcodes);
}

void commando_state::sound_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x47ff).ram();
	map(0x6000, 0x6000).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0x8000, 0x8001).w("ym1", FUNC(ym2203_device::write));
	map(0x8002, 0x8003).w("ym2", FUNC(ym2203_device::write));
}

// Input Ports

static INPUT_PORTS_START( commando )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, "Starting Area" ) PORT_DIPLOCATION("SW1:8,7")
	PORT_DIPSETTING(    0x03, "0 (Forest 1)" )
	PORT_DIPSETTING(    0x01, "2 (Desert 1)" )
	PORT_DIPSETTING(    0x02, "4 (Forest 2)" )
	PORT_DIPSETTING(    0x00, "6 (Desert 2)" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:6,5")
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:4,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_3C ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:8,7,6")
	PORT_DIPSETTING(    0x07, "10K 50K+" )
	PORT_DIPSETTING(    0x03, "10K 60K+" )
	PORT_DIPSETTING(    0x05, "20K 60K+" )
	PORT_DIPSETTING(    0x01, "20K 70K+" )
	PORT_DIPSETTING(    0x06, "30K 70K+" )
	PORT_DIPSETTING(    0x02, "30K 80K+" )
	PORT_DIPSETTING(    0x04, "40K 100K+" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Difficult ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:2,1")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, "Upright Two Players" )
	PORT_DIPSETTING(    0xc0, DEF_STR( Cocktail ) )
INPUT_PORTS_END

static INPUT_PORTS_START( commandou )
	PORT_INCLUDE(commando)

	PORT_MODIFY("DSW2")
	PORT_SERVICE_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW2:5" )
INPUT_PORTS_END

/* Graphics Layouts */

static const gfx_layout charlayout =
{
	8, 8,
	RGN_FRAC(1,1),
	2,
	{ 4, 0 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static const gfx_layout tilelayout =
{
	16, 16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8
};

static const gfx_layout spritelayout =
{
	16, 16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+4, RGN_FRAC(1,2)+0, 4, 0 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3,
			32*8+0, 32*8+1, 32*8+2, 32*8+3, 33*8+0, 33*8+1, 33*8+2, 33*8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	64*8
};

// Graphics Decode Information

static GFXDECODE_START( gfx_commando )
	GFXDECODE_ENTRY( "chars",   0, charlayout,   192, 16 ) // colors 192-255
	GFXDECODE_ENTRY( "tiles",   0, tilelayout,     0, 16 ) // colors   0-127
	GFXDECODE_ENTRY( "sprites", 0, spritelayout, 128,  4 ) // colors 128-191
GFXDECODE_END

// Interrupt Generator

void commando_state::vblank_irq(int state)
{
	if (state)
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xd7); // Z80 - RST 10h - VBLANK
}

// Machine Driver

void commando_state::machine_start()
{
	save_item(NAME(m_scroll_x));
	save_item(NAME(m_scroll_y));
}

void commando_state::machine_reset()
{
	m_scroll_x[0] = 0;
	m_scroll_x[1] = 0;
	m_scroll_y[0] = 0;
	m_scroll_y[1] = 0;
}


void commando_state::commando(machine_config &config)
{
	static constexpr XTAL MAIN = 12_MHz_XTAL;
	static constexpr XTAL PHI_B = MAIN / 2 / 2;
	static constexpr XTAL PHI_MAIN = MAIN / 2 / 2; // As seen in the schematics:
	// the signal goes into a bus arbitration logic that doesn't affect its frequency
	// although the CPU gets slowed down when accessing char/background memories

	// basic machine hardware
	Z80(config, m_maincpu, PHI_MAIN); // 3 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &commando_state::main_map);
	m_maincpu->set_addrmap(AS_OPCODES, &commando_state::decrypted_opcodes_map);

	Z80(config, m_audiocpu, PHI_B); // 3 MHz
	m_audiocpu->set_addrmap(AS_PROGRAM, &commando_state::sound_map);
	m_audiocpu->set_periodic_int(FUNC(commando_state::irq0_line_hold), attotime::from_hz(4*60));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(MAIN / 2, 384, 128, 0, 262, 22, 246); // hsync is 50..77, vsync is 257..259
	screen.set_screen_update(FUNC(commando_state::screen_update));
	screen.screen_vblank().set(m_spriteram, FUNC(buffered_spriteram8_device::vblank_copy_rising));
	screen.screen_vblank().append(FUNC(commando_state::vblank_irq));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_commando);
	PALETTE(config, m_palette, palette_device::RGB_444_PROMS, "proms", 256);

	BUFFERED_SPRITERAM8(config, m_spriteram);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	YM2203(config, "ym1", PHI_B / 2).add_route(ALL_OUTPUTS, "mono", 0.15);
	YM2203(config, "ym2", PHI_B / 2).add_route(ALL_OUTPUTS, "mono", 0.15);
}


// ROMs

ROM_START( commando )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "cm04.9m",  0x0000, 0x8000, CRC(8438b694) SHA1(e154478d8f1b635355bd777370acabe49cb9d309) )
	ROM_LOAD( "cm03.8m",  0x8000, 0x4000, CRC(35486542) SHA1(531a85c9e03970ce037be84f2240c2df6f6e3ec1) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "cm02.9f",  0x0000, 0x4000, CRC(f9cc4a74) SHA1(ee8dd73919c6f47f62cc6d999de9510db9f79b8f) )

	ROM_REGION( 0x4000, "chars", 0 )
	ROM_LOAD( "vt01.5d",  0x0000, 0x4000, CRC(505726e0) SHA1(2435c87c9c9d78a6e703cf0e1f6a0288207fcd4c) )

	ROM_REGION( 0x18000, "tiles", 0 )
	ROM_LOAD( "vt11.5a",  0x00000, 0x4000, CRC(7b2e1b48) SHA1(5d49e1d8146e4ef744445b68f35677302e875a85) )   // SCR X
	ROM_LOAD( "vt12.6a",  0x04000, 0x4000, CRC(81b417d3) SHA1(5ec7e3f0c8069384a5f6eb39232c228b9d7b8c0c) )   // SCR X
	ROM_LOAD( "vt13.7a",  0x08000, 0x4000, CRC(5612dbd2) SHA1(9e4e1a22b6cbf60607b9a81dae34482ae55f7c47) )   // SCR Y
	ROM_LOAD( "vt14.8a",  0x0c000, 0x4000, CRC(2b2dee36) SHA1(8792278464fa3da47176582025f6673a15a581e2) )   // SCR Y
	ROM_LOAD( "vt15.9a",  0x10000, 0x4000, CRC(de70babf) SHA1(6717e23baf55f84d3143fb432140a7c3e102ac26) )   // SCR Z
	ROM_LOAD( "vt16.10a", 0x14000, 0x4000, CRC(14178237) SHA1(f896e71c7004349c9a46155edfd9f0aaa186065d) )   // SCR Z

	ROM_REGION( 0x18000, "sprites", 0 )
	ROM_LOAD( "vt05.7e",  0x00000, 0x4000, CRC(79f16e3d) SHA1(04e1f03a4d6b4cc2b81bce3a290bbb95de900d35) )
	ROM_LOAD( "vt06.8e",  0x04000, 0x4000, CRC(26fee521) SHA1(2fbfc73ee860f72a20229a01d4da9f5cc2e858d3) )
	ROM_LOAD( "vt07.9e",  0x08000, 0x4000, CRC(ca88bdfd) SHA1(548b05460bc7983cc81f15c70e87f47d10db2812) )
	ROM_LOAD( "vt08.7h",  0x0c000, 0x4000, CRC(2019c883) SHA1(883c0156ceab99f4849fe36972c4162b4ac8c216) )
	ROM_LOAD( "vt09.8h",  0x10000, 0x4000, CRC(98703982) SHA1(ba9a9b0dcadd4f52502828408c4a19b0bd518351) )
	ROM_LOAD( "vt10.9h",  0x14000, 0x4000, CRC(f069d2f8) SHA1(2c92300a9407470b34965021de882f1f7a84730c) )

	ROM_REGION( 0x600, "proms", 0 )
	ROM_LOAD( "vtb1.1d",  0x0000, 0x0100, CRC(3aba15a1) SHA1(8b057f6e26155dd9e48bde182e680fce4519f600) )    // red
	ROM_LOAD( "vtb2.2d",  0x0100, 0x0100, CRC(88865754) SHA1(ca6dddca98baf00a65b2fb70b69cf4704ef8c831) )    // green
	ROM_LOAD( "vtb3.3d",  0x0200, 0x0100, CRC(4c14c3f6) SHA1(644ac17c7413f094ec9a15cba87bbd421b26321f) )    // blue
	ROM_LOAD( "vtb4.1h",  0x0300, 0x0100, CRC(b388c246) SHA1(038f9851699331ad887b6281a9df053dca3db8fd) )    // palette selector (not used)
	ROM_LOAD( "vtb5.6l",  0x0400, 0x0100, CRC(712ac508) SHA1(5349d722ab6733afdda65f6e0a98322f0d515e86) )    // interrupt timing (not used)
	ROM_LOAD( "vtb6.6e",  0x0500, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )    // video timing (not used)
ROM_END

ROM_START( commandou )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "u4-f.9m",  0x0000, 0x8000, CRC(a6118935) SHA1(d5811968b23d61e344e151747bcc3c0ed2b9497b) )
	ROM_LOAD( "u3-f.8m",  0x8000, 0x4000, CRC(24f49684) SHA1(d38a7bd9f3b506747a03f6b94c3f8a2d9fc59166) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "cm02.9f",  0x0000, 0x4000, CRC(f9cc4a74) SHA1(ee8dd73919c6f47f62cc6d999de9510db9f79b8f) )

	ROM_REGION( 0x4000, "chars", 0 )
	ROM_LOAD( "vt01.5d",  0x0000, 0x4000, CRC(505726e0) SHA1(2435c87c9c9d78a6e703cf0e1f6a0288207fcd4c) )

	ROM_REGION( 0x18000, "tiles", 0 )
	ROM_LOAD( "vt11.5a",  0x00000, 0x4000, CRC(7b2e1b48) SHA1(5d49e1d8146e4ef744445b68f35677302e875a85) )   // SCR X
	ROM_LOAD( "vt12.6a",  0x04000, 0x4000, CRC(81b417d3) SHA1(5ec7e3f0c8069384a5f6eb39232c228b9d7b8c0c) )   // SCR X
	ROM_LOAD( "vt13.7a",  0x08000, 0x4000, CRC(5612dbd2) SHA1(9e4e1a22b6cbf60607b9a81dae34482ae55f7c47) )   // SCR Y
	ROM_LOAD( "vt14.8a",  0x0c000, 0x4000, CRC(2b2dee36) SHA1(8792278464fa3da47176582025f6673a15a581e2) )   // SCR Y
	ROM_LOAD( "vt15.9a",  0x10000, 0x4000, CRC(de70babf) SHA1(6717e23baf55f84d3143fb432140a7c3e102ac26) )   // SCR Z
	ROM_LOAD( "vt16.10a", 0x14000, 0x4000, CRC(14178237) SHA1(f896e71c7004349c9a46155edfd9f0aaa186065d) )   // SCR Z

	ROM_REGION( 0x18000, "sprites", 0 )
	ROM_LOAD( "vt05.7e",  0x00000, 0x4000, CRC(79f16e3d) SHA1(04e1f03a4d6b4cc2b81bce3a290bbb95de900d35) )
	ROM_LOAD( "vt06.8e",  0x04000, 0x4000, CRC(26fee521) SHA1(2fbfc73ee860f72a20229a01d4da9f5cc2e858d3) )
	ROM_LOAD( "vt07.9e",  0x08000, 0x4000, CRC(ca88bdfd) SHA1(548b05460bc7983cc81f15c70e87f47d10db2812) )
	ROM_LOAD( "vt08.7h",  0x0c000, 0x4000, CRC(2019c883) SHA1(883c0156ceab99f4849fe36972c4162b4ac8c216) )
	ROM_LOAD( "vt09.8h",  0x10000, 0x4000, CRC(98703982) SHA1(ba9a9b0dcadd4f52502828408c4a19b0bd518351) )
	ROM_LOAD( "vt10.9h",  0x14000, 0x4000, CRC(f069d2f8) SHA1(2c92300a9407470b34965021de882f1f7a84730c) )

	ROM_REGION( 0x600, "proms", 0 )
	ROM_LOAD( "vtb1.1d",  0x0000, 0x0100, CRC(3aba15a1) SHA1(8b057f6e26155dd9e48bde182e680fce4519f600) )    // red
	ROM_LOAD( "vtb2.2d",  0x0100, 0x0100, CRC(88865754) SHA1(ca6dddca98baf00a65b2fb70b69cf4704ef8c831) )    // green
	ROM_LOAD( "vtb3.3d",  0x0200, 0x0100, CRC(4c14c3f6) SHA1(644ac17c7413f094ec9a15cba87bbd421b26321f) )    // blue
	ROM_LOAD( "vtb4.1h",  0x0300, 0x0100, CRC(b388c246) SHA1(038f9851699331ad887b6281a9df053dca3db8fd) )    // palette selector (not used)
	ROM_LOAD( "vtb5.6l",  0x0400, 0x0100, CRC(712ac508) SHA1(5349d722ab6733afdda65f6e0a98322f0d515e86) )    // interrupt timing (not used)
	ROM_LOAD( "vtb6.6e",  0x0500, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )    // video timing (not used)
ROM_END

ROM_START( commandoj )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "so04.9m", 0x0000, 0x8000, CRC(d3f2bfb3) SHA1(738a5673ac6a907cb04cfb125e8aab3f7437b9d2) )
	ROM_LOAD( "so03.8m", 0x8000, 0x4000, CRC(ed01f472) SHA1(fa181293ae8f0fee78d412259eb81f6de1e1307a) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "so02.9f", 0x0000, 0x4000, CRC(ca20aca5) SHA1(206a8fd4a8985e7ceed7de8349ba02627e881503) )

	ROM_REGION( 0x4000, "chars", 0 )
	ROM_LOAD( "vt01.5d",  0x0000, 0x4000, CRC(505726e0) SHA1(2435c87c9c9d78a6e703cf0e1f6a0288207fcd4c) )

	ROM_REGION( 0x18000, "tiles", 0 )
	ROM_LOAD( "vt11.5a",  0x00000, 0x4000, CRC(7b2e1b48) SHA1(5d49e1d8146e4ef744445b68f35677302e875a85) )   // SCR X
	ROM_LOAD( "vt12.6a",  0x04000, 0x4000, CRC(81b417d3) SHA1(5ec7e3f0c8069384a5f6eb39232c228b9d7b8c0c) )   // SCR X
	ROM_LOAD( "vt13.7a",  0x08000, 0x4000, CRC(5612dbd2) SHA1(9e4e1a22b6cbf60607b9a81dae34482ae55f7c47) )   // SCR Y
	ROM_LOAD( "vt14.8a",  0x0c000, 0x4000, CRC(2b2dee36) SHA1(8792278464fa3da47176582025f6673a15a581e2) )   // SCR Y
	ROM_LOAD( "vt15.9a",  0x10000, 0x4000, CRC(de70babf) SHA1(6717e23baf55f84d3143fb432140a7c3e102ac26) )   // SCR Z
	ROM_LOAD( "vt16.10a", 0x14000, 0x4000, CRC(14178237) SHA1(f896e71c7004349c9a46155edfd9f0aaa186065d) )   // SCR Z

	ROM_REGION( 0x18000, "sprites", 0 )
	ROM_LOAD( "vt05.7e",  0x00000, 0x4000, CRC(79f16e3d) SHA1(04e1f03a4d6b4cc2b81bce3a290bbb95de900d35) )
	ROM_LOAD( "vt06.8e",  0x04000, 0x4000, CRC(26fee521) SHA1(2fbfc73ee860f72a20229a01d4da9f5cc2e858d3) )
	ROM_LOAD( "vt07.9e",  0x08000, 0x4000, CRC(ca88bdfd) SHA1(548b05460bc7983cc81f15c70e87f47d10db2812) )
	ROM_LOAD( "vt08.7h",  0x0c000, 0x4000, CRC(2019c883) SHA1(883c0156ceab99f4849fe36972c4162b4ac8c216) )
	ROM_LOAD( "vt09.8h",  0x10000, 0x4000, CRC(98703982) SHA1(ba9a9b0dcadd4f52502828408c4a19b0bd518351) )
	ROM_LOAD( "vt10.9h",  0x14000, 0x4000, CRC(f069d2f8) SHA1(2c92300a9407470b34965021de882f1f7a84730c) )

	ROM_REGION( 0x600, "proms", 0 )
	ROM_LOAD( "vtb1.1d",  0x0000, 0x0100, CRC(3aba15a1) SHA1(8b057f6e26155dd9e48bde182e680fce4519f600) )    // red
	ROM_LOAD( "vtb2.2d",  0x0100, 0x0100, CRC(88865754) SHA1(ca6dddca98baf00a65b2fb70b69cf4704ef8c831) )    // green
	ROM_LOAD( "vtb3.3d",  0x0200, 0x0100, CRC(4c14c3f6) SHA1(644ac17c7413f094ec9a15cba87bbd421b26321f) )    // blue
	ROM_LOAD( "vtb4.1h",  0x0300, 0x0100, CRC(b388c246) SHA1(038f9851699331ad887b6281a9df053dca3db8fd) )    // palette selector (not used)
	ROM_LOAD( "vtb5.6l",  0x0400, 0x0100, CRC(712ac508) SHA1(5349d722ab6733afdda65f6e0a98322f0d515e86) )    // interrupt timing (not used)
	ROM_LOAD( "vtb6.6e",  0x0500, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )    // video timing (not used)
ROM_END

ROM_START( commandob )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "commandob_04_9m_27256.bin",  0x0000, 0x8000, CRC(348a7654) SHA1(f3668c47c154a9c7d7afeabb0259c9bc56e847ac) )
	ROM_LOAD( "cm03.8m",  0x8000, 0x4000, CRC(35486542) SHA1(531a85c9e03970ce037be84f2240c2df6f6e3ec1) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "cm02.9f",  0x0000, 0x4000, CRC(f9cc4a74) SHA1(ee8dd73919c6f47f62cc6d999de9510db9f79b8f) )

	ROM_REGION( 0x4000, "chars", 0 )
	ROM_LOAD( "vt01.5d",  0x0000, 0x4000, CRC(505726e0) SHA1(2435c87c9c9d78a6e703cf0e1f6a0288207fcd4c) )

	ROM_REGION( 0x18000, "tiles", 0 )
	ROM_LOAD( "vt11.5a",  0x00000, 0x4000, CRC(7b2e1b48) SHA1(5d49e1d8146e4ef744445b68f35677302e875a85) )   // SCR X
	ROM_LOAD( "vt12.6a",  0x04000, 0x4000, CRC(81b417d3) SHA1(5ec7e3f0c8069384a5f6eb39232c228b9d7b8c0c) )   // SCR X
	ROM_LOAD( "vt13.7a",  0x08000, 0x4000, CRC(5612dbd2) SHA1(9e4e1a22b6cbf60607b9a81dae34482ae55f7c47) )   // SCR Y
	ROM_LOAD( "vt14.8a",  0x0c000, 0x4000, CRC(2b2dee36) SHA1(8792278464fa3da47176582025f6673a15a581e2) )   // SCR Y
	ROM_LOAD( "vt15.9a",  0x10000, 0x4000, CRC(de70babf) SHA1(6717e23baf55f84d3143fb432140a7c3e102ac26) )   // SCR Z
	ROM_LOAD( "vt16.10a", 0x14000, 0x4000, CRC(14178237) SHA1(f896e71c7004349c9a46155edfd9f0aaa186065d) )   // SCR Z

	ROM_REGION( 0x18000, "sprites", 0 )
	ROM_LOAD( "vt05.7e",  0x00000, 0x4000, CRC(79f16e3d) SHA1(04e1f03a4d6b4cc2b81bce3a290bbb95de900d35) )
	ROM_LOAD( "vt06.8e",  0x04000, 0x4000, CRC(26fee521) SHA1(2fbfc73ee860f72a20229a01d4da9f5cc2e858d3) )
	ROM_LOAD( "vt07.9e",  0x08000, 0x4000, CRC(ca88bdfd) SHA1(548b05460bc7983cc81f15c70e87f47d10db2812) )
	ROM_LOAD( "vt08.7h",  0x0c000, 0x4000, CRC(2019c883) SHA1(883c0156ceab99f4849fe36972c4162b4ac8c216) )
	ROM_LOAD( "vt09.8h",  0x10000, 0x4000, CRC(98703982) SHA1(ba9a9b0dcadd4f52502828408c4a19b0bd518351) )
	ROM_LOAD( "vt10.9h",  0x14000, 0x4000, CRC(f069d2f8) SHA1(2c92300a9407470b34965021de882f1f7a84730c) )

		/* I did not dump the PROMs of the bootleg board, I'm just adding the parent ones, it has the same
		   number of PROMs on the same board locations as the original board. */
	ROM_REGION( 0x600, "proms", 0 )
	ROM_LOAD( "vtb1.1d",  0x0000, 0x0100, CRC(3aba15a1) SHA1(8b057f6e26155dd9e48bde182e680fce4519f600) )    // red
	ROM_LOAD( "vtb2.2d",  0x0100, 0x0100, CRC(88865754) SHA1(ca6dddca98baf00a65b2fb70b69cf4704ef8c831) )    // green
	ROM_LOAD( "vtb3.3d",  0x0200, 0x0100, CRC(4c14c3f6) SHA1(644ac17c7413f094ec9a15cba87bbd421b26321f) )    // blue
	ROM_LOAD( "vtb4.1h",  0x0300, 0x0100, CRC(b388c246) SHA1(038f9851699331ad887b6281a9df053dca3db8fd) )    // palette selector (not used)
	ROM_LOAD( "vtb5.6l",  0x0400, 0x0100, CRC(712ac508) SHA1(5349d722ab6733afdda65f6e0a98322f0d515e86) )    // interrupt timing (not used)
	ROM_LOAD( "vtb6.6e",  0x0500, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )    // video timing (not used)

		/* There's a 16L8A PAL (with a 74LS244P and a 74LS367AP) on a tiny sub-board between the CPU1 ROMs
		   and the CPU1 (a Z80 compatible NEC D780C-1). This sub-board is plugged on what seems to be
		   a ROM socket. */
	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "commandob_pal16l8a.bin", 0x0000, 0x0104, NO_DUMP ) // I Didn't try to dump it...
ROM_END

ROM_START( commandob2 )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "10",  0x0000, 0x8000, CRC(ab5d1469) SHA1(05935155365bef3c40823101303a3857af1c71e6) )
	ROM_LOAD( "11",  0x8000, 0x4000, CRC(d1a43ba1) SHA1(dbaedcd3a1b489a01b7f783fda6fcad203758717) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "8,so02.9f",  0x0000, 0x4000, CRC(ca20aca5) SHA1(206a8fd4a8985e7ceed7de8349ba02627e881503) )

	ROM_REGION( 0x4000, "chars", 0 )
	ROM_LOAD( "7,vt01.5d",  0x0000, 0x4000, CRC(505726e0) SHA1(2435c87c9c9d78a6e703cf0e1f6a0288207fcd4c) )

	ROM_REGION( 0x18000, "tiles", 0 )
	ROM_LOAD( "17,vt11.5a",  0x00000, 0x4000, CRC(7b2e1b48) SHA1(5d49e1d8146e4ef744445b68f35677302e875a85) )   // SCR X
	ROM_LOAD( "16,vt12.6a",  0x04000, 0x4000, CRC(81b417d3) SHA1(5ec7e3f0c8069384a5f6eb39232c228b9d7b8c0c) )   // SCR X
	ROM_LOAD( "15,vt13.7a",  0x08000, 0x4000, CRC(5612dbd2) SHA1(9e4e1a22b6cbf60607b9a81dae34482ae55f7c47) )   // SCR Y
	ROM_LOAD( "14,vt14.8a",  0x0c000, 0x4000, CRC(2b2dee36) SHA1(8792278464fa3da47176582025f6673a15a581e2) )   // SCR Y
	ROM_LOAD( "13,vt15.9a",  0x10000, 0x4000, CRC(de70babf) SHA1(6717e23baf55f84d3143fb432140a7c3e102ac26) )   // SCR Z
	ROM_LOAD( "12,vt16.10a", 0x14000, 0x4000, CRC(14178237) SHA1(f896e71c7004349c9a46155edfd9f0aaa186065d) )   // SCR Z

	ROM_REGION( 0x18000, "sprites", 0 )
	ROM_LOAD( "3,vt05.7e",  0x00000, 0x4000, CRC(79f16e3d) SHA1(04e1f03a4d6b4cc2b81bce3a290bbb95de900d35) )
	ROM_LOAD( "2,vt06.8e",  0x04000, 0x4000, CRC(26fee521) SHA1(2fbfc73ee860f72a20229a01d4da9f5cc2e858d3) )
	ROM_LOAD( "1,vt07.9e",  0x08000, 0x4000, CRC(ca88bdfd) SHA1(548b05460bc7983cc81f15c70e87f47d10db2812) )
	ROM_LOAD( "6,vt08.7h",  0x0c000, 0x4000, CRC(2019c883) SHA1(883c0156ceab99f4849fe36972c4162b4ac8c216) )
	ROM_LOAD( "5,vt09.8h",  0x10000, 0x4000, CRC(98703982) SHA1(ba9a9b0dcadd4f52502828408c4a19b0bd518351) )
	ROM_LOAD( "4,vt10.9h",  0x14000, 0x4000, CRC(f069d2f8) SHA1(2c92300a9407470b34965021de882f1f7a84730c) )

		/* I did not dump the PROMs of the bootleg board, I'm just adding the parent ones, it has the same
		   number of PROMs on the same board locations as the original board. */
	ROM_REGION( 0x600, "proms", 0 )
	ROM_LOAD( "vtb1.1d",  0x0000, 0x0100, CRC(3aba15a1) SHA1(8b057f6e26155dd9e48bde182e680fce4519f600) )    // red
	ROM_LOAD( "vtb2.2d",  0x0100, 0x0100, CRC(88865754) SHA1(ca6dddca98baf00a65b2fb70b69cf4704ef8c831) )    // green
	ROM_LOAD( "vtb3.3d",  0x0200, 0x0100, CRC(4c14c3f6) SHA1(644ac17c7413f094ec9a15cba87bbd421b26321f) )    // blue
	ROM_LOAD( "vtb4.1h",  0x0300, 0x0100, CRC(b388c246) SHA1(038f9851699331ad887b6281a9df053dca3db8fd) )    // palette selector (not used)
	ROM_LOAD( "vtb5.6l",  0x0400, 0x0100, CRC(712ac508) SHA1(5349d722ab6733afdda65f6e0a98322f0d515e86) )    // interrupt timing (not used)
	ROM_LOAD( "vtb6.6e",  0x0500, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )    // video timing (not used)

		/* There's a 16L8 PAL (with a 74LS245 and a 74LS08) on a tiny sub-board between the CPU1 ROMs
		   and the CPU1 (a Z80 compatible NEC D780C-1). This sub-board is plugged on what seems to be
		   a ROM socket.

		PAL16L8 (SWELT-019A)

		Pin 1  - Pin 12 of 27256 (D1) and Pin 3 of 74LS245 (A2) (Input)
		Pin 2  - Pin 13 of 27256 (D2) and Pin 4 of 74LS245 (A3) (Input)
		Pin 3  - Pin 15 of 27256 (D3) and Pin 5 of 74LS245 (A4) (Input)
		Pin 4  - Pin 8 of 74LS08 (Input)
		Pin 5  - Pin 17 of 27256 (D5) and Pin 7 of 74LS245 (A6) (Input)
		Pin 6  - Pin 18 of 27256 (D6) and Pin 8 of 74LS245 (A7) (Input)
		Pin 7  - Pin 19 of 27256 (D7) and Pin 9 of 74LS245 (A8) (Input)
		Pin 8  - Pin 6 of 74LS14 (Input)
		Pin 9  - Pin 11 of 74LS08 (Input)
		Pin 10 - Ground
		Pin 11 - Pin 22 of Z80 (~WD) and Pin 1 of 74LS245 (DIR) (Input)
		Pin 12 - Pin 19 of 74LS245 (~OE) (Output)
		Pin 13 - Pin 13 of Z80 (D7) (Input?, Output)
		Pin 14 - Pin 10 of Z80 (D6) (Input?, Output)
		Pin 15 - Pin 9 of Z80 (D5) (Input?, Output)
		Pin 16 - Not Used (Input?, Output)
		Pin 17 - Pin 8 of Z80 (D3) (Input?, Output)
		Pin 18 - Pin 12 of Z80 (D2) (Input?, Output)
		Pin 19 - Pin 15 of Z80 (D1) (Output)
		Pin 20 - VCC

		------------------------------------------------------------------------------

		Pin 14 of Z80 (D0) to Pin 11 of 27256 (D0)
		Pin 7 of Z80 (D4) to Pin 16 of 27256 (D4)
		*/

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "commandob2_pal16l8.bin", 0x000000, 0x000104, CRC(bdbcaf02) SHA1(148591f95a343c8ffa2eaa02764c91557aa523d3) )
ROM_END

// mix of ROMs from Mercenario and other commando bootlegs, only the first maincpu ROM is unique
ROM_START( commandob3 )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "b5.10n",  0x0000, 0x4000, CRC(df8f4e9a) SHA1(72f08dc70a72c183c3f071ed47b1d060516e545a) ) // like mercenario but for byte 0x00 changed to the encrypted one
	ROM_LOAD( "b4.9n",   0x4000, 0x4000, CRC(aca99905) SHA1(609cf3d180ceb0c67e2deff9db16fa56c8948a97) ) // like mercenario
	ROM_LOAD( "b3.8n",   0x8000, 0x4000, CRC(35486542) SHA1(531a85c9e03970ce037be84f2240c2df6f6e3ec1) ) // like commandob

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "b2.9f",   0x0000, 0x4000, CRC(f9cc4a74) SHA1(ee8dd73919c6f47f62cc6d999de9510db9f79b8f) )    // 2c.9f

	ROM_REGION( 0x4000, "chars", 0 )
	ROM_LOAD( "b1.5d",   0x0000, 0x4000, CRC(505726e0) SHA1(2435c87c9c9d78a6e703cf0e1f6a0288207fcd4c) )

	ROM_REGION( 0x18000, "tiles", 0 )
	ROM_LOAD( "b12.5a",  0x00000, 0x4000, CRC(7b2e1b48) SHA1(5d49e1d8146e4ef744445b68f35677302e875a85) )   // 11c.5a
	ROM_LOAD( "b13.6a",  0x04000, 0x4000, CRC(81b417d3) SHA1(5ec7e3f0c8069384a5f6eb39232c228b9d7b8c0c) )   // 12c.6a
	ROM_LOAD( "b14.7a",  0x08000, 0x4000, CRC(5612dbd2) SHA1(9e4e1a22b6cbf60607b9a81dae34482ae55f7c47) )   // 13c.7a
	ROM_LOAD( "b15.8a",  0x0c000, 0x4000, CRC(2b2dee36) SHA1(8792278464fa3da47176582025f6673a15a581e2) )   // 14c.8a
	ROM_LOAD( "b16.9a",  0x10000, 0x4000, CRC(de70babf) SHA1(6717e23baf55f84d3143fb432140a7c3e102ac26) )   // 15c.9a
	ROM_LOAD( "b17.10a", 0x14000, 0x4000, CRC(14178237) SHA1(f896e71c7004349c9a46155edfd9f0aaa186065d) )   // 16c.10a

	ROM_REGION( 0x18000, "sprites", 0 )
	ROM_LOAD( "b6.7e",   0x00000, 0x4000, CRC(79f16e3d) SHA1(04e1f03a4d6b4cc2b81bce3a290bbb95de900d35) )   // 5c.7e
	ROM_LOAD( "b7.8e",   0x04000, 0x4000, CRC(26fee521) SHA1(2fbfc73ee860f72a20229a01d4da9f5cc2e858d3) )   // 6c.8e
	ROM_LOAD( "b8.9e",   0x08000, 0x4000, CRC(ca88bdfd) SHA1(548b05460bc7983cc81f15c70e87f47d10db2812) )   // 7c.9e
	ROM_LOAD( "b9.7h",   0x0c000, 0x4000, CRC(2019c883) SHA1(883c0156ceab99f4849fe36972c4162b4ac8c216) )   // 8c.7h
	ROM_LOAD( "b10.8h",  0x10000, 0x4000, CRC(98703982) SHA1(ba9a9b0dcadd4f52502828408c4a19b0bd518351) )   // 9c.8h
	ROM_LOAD( "b11.9h",  0x14000, 0x4000, CRC(f069d2f8) SHA1(2c92300a9407470b34965021de882f1f7a84730c) )   // 10c.9h

	ROM_REGION( 0x600, "proms", 0 )
	ROM_LOAD( "vtb1.1d", 0x0000, 0x0100, CRC(3aba15a1) SHA1(8b057f6e26155dd9e48bde182e680fce4519f600) )    // red
	ROM_LOAD( "vtb2.2d", 0x0100, 0x0100, CRC(88865754) SHA1(ca6dddca98baf00a65b2fb70b69cf4704ef8c831) )    // green
	ROM_LOAD( "vtb3.3d", 0x0200, 0x0100, CRC(4c14c3f6) SHA1(644ac17c7413f094ec9a15cba87bbd421b26321f) )    // blue
	ROM_LOAD( "vtb4.1h", 0x0300, 0x0100, CRC(b388c246) SHA1(038f9851699331ad887b6281a9df053dca3db8fd) )    // palette selector (not used)
	ROM_LOAD( "vtb5.6l", 0x0400, 0x0100, CRC(712ac508) SHA1(5349d722ab6733afdda65f6e0a98322f0d515e86) )    // interrupt timing (not used)
	ROM_LOAD( "vtb6.6e", 0x0500, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )    // video timing (not used)
ROM_END

ROM_START( commandou2 )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "uc4.9m",   0x0000, 0x8000, CRC(89ee8e17) SHA1(68db271af8b0f400ca95df5672983bfb87f3f84a) )
	ROM_LOAD( "uc3.8m",   0x8000, 0x4000, CRC(72a1a529) SHA1(fe7797206e38bd78e817b6c351d5cb943720fe6c) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "cd02.9f",  0x0000, 0x4000, CRC(f9cc4a74) SHA1(ee8dd73919c6f47f62cc6d999de9510db9f79b8f) )

	ROM_REGION( 0x4000, "chars", 0 )
	ROM_LOAD( "vt01.5d",  0x0000, 0x4000, CRC(505726e0) SHA1(2435c87c9c9d78a6e703cf0e1f6a0288207fcd4c) )

	ROM_REGION( 0x18000, "tiles", 0 )
	ROM_LOAD( "vt11.5a",  0x00000, 0x4000, CRC(7b2e1b48) SHA1(5d49e1d8146e4ef744445b68f35677302e875a85) )   // SCR X
	ROM_LOAD( "vt12.6a",  0x04000, 0x4000, CRC(81b417d3) SHA1(5ec7e3f0c8069384a5f6eb39232c228b9d7b8c0c) )   // SCR X
	ROM_LOAD( "vt13.7a",  0x08000, 0x4000, CRC(5612dbd2) SHA1(9e4e1a22b6cbf60607b9a81dae34482ae55f7c47) )   // SCR Y
	ROM_LOAD( "vt14.8a",  0x0c000, 0x4000, CRC(2b2dee36) SHA1(8792278464fa3da47176582025f6673a15a581e2) )   // SCR Y
	ROM_LOAD( "vt15.9a",  0x10000, 0x4000, CRC(de70babf) SHA1(6717e23baf55f84d3143fb432140a7c3e102ac26) )   // SCR Z
	ROM_LOAD( "vt16.10a", 0x14000, 0x4000, CRC(14178237) SHA1(f896e71c7004349c9a46155edfd9f0aaa186065d) )   // SCR Z

	ROM_REGION( 0x18000, "sprites", 0 )
	ROM_LOAD( "vt05.7e",  0x00000, 0x4000, CRC(79f16e3d) SHA1(04e1f03a4d6b4cc2b81bce3a290bbb95de900d35) )
	ROM_LOAD( "vt06.8e",  0x04000, 0x4000, CRC(26fee521) SHA1(2fbfc73ee860f72a20229a01d4da9f5cc2e858d3) )
	ROM_LOAD( "vt07.9e",  0x08000, 0x4000, CRC(ca88bdfd) SHA1(548b05460bc7983cc81f15c70e87f47d10db2812) )
	ROM_LOAD( "vt08.7h",  0x0c000, 0x4000, CRC(2019c883) SHA1(883c0156ceab99f4849fe36972c4162b4ac8c216) )
	ROM_LOAD( "vt09.8h",  0x10000, 0x4000, CRC(98703982) SHA1(ba9a9b0dcadd4f52502828408c4a19b0bd518351) )
	ROM_LOAD( "vt10.9h",  0x14000, 0x4000, CRC(f069d2f8) SHA1(2c92300a9407470b34965021de882f1f7a84730c) )

	ROM_REGION( 0x600, "proms", 0 )
	ROM_LOAD( "vtb-1.1d",  0x0000, 0x0100, CRC(3aba15a1) SHA1(8b057f6e26155dd9e48bde182e680fce4519f600) )    // red
	ROM_LOAD( "vtb-2.2d",  0x0100, 0x0100, CRC(88865754) SHA1(ca6dddca98baf00a65b2fb70b69cf4704ef8c831) )    // green
	ROM_LOAD( "vtb-3.3d",  0x0200, 0x0100, CRC(4c14c3f6) SHA1(644ac17c7413f094ec9a15cba87bbd421b26321f) )    // blue
	ROM_LOAD( "vtb-4.1h",  0x0300, 0x0100, CRC(b388c246) SHA1(038f9851699331ad887b6281a9df053dca3db8fd) )    // palette selector (not used)
	ROM_LOAD( "vtb-5.6l",  0x0400, 0x0100, CRC(712ac508) SHA1(5349d722ab6733afdda65f6e0a98322f0d515e86) )    // interrupt timing (not used)
	ROM_LOAD( "vtb-6.6e",  0x0500, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )    // video timing (not used)
ROM_END

ROM_START( sinvasn )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "sp04.9m",  0x0000, 0x8000, CRC(33f9601e) SHA1(71182227b77fccbbc1d89b5828aa86dcc64ca05e) )
	ROM_LOAD( "sp03.8m",  0x8000, 0x4000, CRC(c7fb43b3) SHA1(36d0dffdacc36a6b6a77101d942c0821846f3275) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "u2.9f",    0x0000, 0x4000, CRC(cbf8c40e) SHA1(0c8dce034d96d075e012cbb8f68c2817b860d969) )

	ROM_REGION( 0x04000, "chars", 0 )
	ROM_LOAD( "u1.5d",    0x0000, 0x4000, CRC(f477e13a) SHA1(ec5b80f5d508501e72cba028dc45b2c307ac452b) )

	ROM_REGION( 0x18000, "tiles", 0 )
	ROM_LOAD( "vt11.5a",  0x00000, 0x4000, CRC(7b2e1b48) SHA1(5d49e1d8146e4ef744445b68f35677302e875a85) )   // SCR X
	ROM_LOAD( "vt12.6a",  0x04000, 0x4000, CRC(81b417d3) SHA1(5ec7e3f0c8069384a5f6eb39232c228b9d7b8c0c) )   // SCR X
	ROM_LOAD( "vt13.7a",  0x08000, 0x4000, CRC(5612dbd2) SHA1(9e4e1a22b6cbf60607b9a81dae34482ae55f7c47) )   // SCR Y
	ROM_LOAD( "vt14.8a",  0x0c000, 0x4000, CRC(2b2dee36) SHA1(8792278464fa3da47176582025f6673a15a581e2) )   // SCR Y
	ROM_LOAD( "vt15.9a",  0x10000, 0x4000, CRC(de70babf) SHA1(6717e23baf55f84d3143fb432140a7c3e102ac26) )   // SCR Z
	ROM_LOAD( "vt16.10a", 0x14000, 0x4000, CRC(14178237) SHA1(f896e71c7004349c9a46155edfd9f0aaa186065d) )   // SCR Z

	ROM_REGION( 0x18000, "sprites", 0 )
	ROM_LOAD( "u5.e7",    0x00000, 0x4000, CRC(2a97c933) SHA1(bfddb0c0f930a7caffad7e52d394d72c09ffb45f) )
	ROM_LOAD( "sp06.e8",  0x04000, 0x4000, CRC(d7887212) SHA1(43ad98263d6314d40abf33087127c23a3ad72335) )
	ROM_LOAD( "sp07.e9",  0x08000, 0x4000, CRC(9abe7a20) SHA1(5f1b851bd66a3ab818b893286d3ebf2194f425c4) )
	ROM_LOAD( "u8.h7",    0x0c000, 0x4000, CRC(d6b4aa2e) SHA1(5bbf536f73010182b9150dd4fb1e2a42b5b380b0) )
	ROM_LOAD( "sp09.h8",  0x10000, 0x4000, CRC(3985b318) SHA1(ac4c67c3af42121869c1b9470377404bc88793c2) )
	ROM_LOAD( "sp10.h9",  0x14000, 0x4000, CRC(3c131b0f) SHA1(dd3e63199120502c03eedd024a2eed3b5d3e2a1c) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "vtb1.1d",  0x0000, 0x0100, CRC(3aba15a1) SHA1(8b057f6e26155dd9e48bde182e680fce4519f600) )    // red
	ROM_LOAD( "vtb2.2d",  0x0100, 0x0100, CRC(88865754) SHA1(ca6dddca98baf00a65b2fb70b69cf4704ef8c831) )    // green
	ROM_LOAD( "vtb3.3d",  0x0200, 0x0100, CRC(4c14c3f6) SHA1(644ac17c7413f094ec9a15cba87bbd421b26321f) )    // blue
	ROM_LOAD( "vtb4.1h",  0x0300, 0x0100, CRC(b388c246) SHA1(038f9851699331ad887b6281a9df053dca3db8fd) )    // palette selector (not used)
	ROM_LOAD( "vtb5.6l",  0x0400, 0x0100, CRC(712ac508) SHA1(5349d722ab6733afdda65f6e0a98322f0d515e86) )    // interrupt timing (not used)
	ROM_LOAD( "vtb6.6e",  0x0500, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )    // video timing (not used)
ROM_END

ROM_START( sinvasnb )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "u4",       0x0000, 0x8000, CRC(834ba0de) SHA1(85f40559e6a436f3f752b6e862a419a5b9481fa8) )
	ROM_LOAD( "u3",       0x8000, 0x4000, CRC(07e4ee3a) SHA1(6d7665b3072f075893ef37e55147b10271d069ef) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "u2",       0x0000, 0x4000, CRC(cbf8c40e) SHA1(0c8dce034d96d075e012cbb8f68c2817b860d969) )

	ROM_REGION( 0x04000, "chars", 0 )
	ROM_LOAD( "u1",       0x0000, 0x4000, CRC(f477e13a) SHA1(ec5b80f5d508501e72cba028dc45b2c307ac452b) )

	ROM_REGION( 0x18000, "tiles", 0 )
	ROM_LOAD( "vt11.5a",  0x00000, 0x4000, CRC(7b2e1b48) SHA1(5d49e1d8146e4ef744445b68f35677302e875a85) )   // SCR X
	ROM_LOAD( "vt12.6a",  0x04000, 0x4000, CRC(81b417d3) SHA1(5ec7e3f0c8069384a5f6eb39232c228b9d7b8c0c) )   // SCR X
	ROM_LOAD( "vt13.7a",  0x08000, 0x4000, CRC(5612dbd2) SHA1(9e4e1a22b6cbf60607b9a81dae34482ae55f7c47) )   // SCR Y
	ROM_LOAD( "vt14.8a",  0x0c000, 0x4000, CRC(2b2dee36) SHA1(8792278464fa3da47176582025f6673a15a581e2) )   // SCR Y
	ROM_LOAD( "vt15.9a",  0x10000, 0x4000, CRC(de70babf) SHA1(6717e23baf55f84d3143fb432140a7c3e102ac26) )   // SCR Z
	ROM_LOAD( "vt16.10a", 0x14000, 0x4000, CRC(14178237) SHA1(f896e71c7004349c9a46155edfd9f0aaa186065d) )   // SCR Z

	ROM_REGION( 0x18000, "sprites", 0 )
	ROM_LOAD( "u5",       0x00000, 0x4000, CRC(2a97c933) SHA1(bfddb0c0f930a7caffad7e52d394d72c09ffb45f) )
	ROM_LOAD( "vt06.e8",  0x04000, 0x4000, CRC(26fee521) SHA1(2fbfc73ee860f72a20229a01d4da9f5cc2e858d3) )
	ROM_LOAD( "vt07.e9",  0x08000, 0x4000, CRC(ca88bdfd) SHA1(548b05460bc7983cc81f15c70e87f47d10db2812) )
	ROM_LOAD( "u8",       0x0c000, 0x4000, CRC(d6b4aa2e) SHA1(5bbf536f73010182b9150dd4fb1e2a42b5b380b0) )
	ROM_LOAD( "vt09.h8",  0x10000, 0x4000, CRC(98703982) SHA1(ba9a9b0dcadd4f52502828408c4a19b0bd518351) )
	ROM_LOAD( "vt10.h9",  0x14000, 0x4000, CRC(f069d2f8) SHA1(2c92300a9407470b34965021de882f1f7a84730c) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "vtb1.1d",  0x0000, 0x0100, CRC(3aba15a1) SHA1(8b057f6e26155dd9e48bde182e680fce4519f600) )    // red
	ROM_LOAD( "vtb2.2d",  0x0100, 0x0100, CRC(88865754) SHA1(ca6dddca98baf00a65b2fb70b69cf4704ef8c831) )    // green
	ROM_LOAD( "vtb3.3d",  0x0200, 0x0100, CRC(4c14c3f6) SHA1(644ac17c7413f094ec9a15cba87bbd421b26321f) )    // blue
	ROM_LOAD( "vtb4.1h",  0x0300, 0x0100, CRC(b388c246) SHA1(038f9851699331ad887b6281a9df053dca3db8fd) )    // palette selector (not used)
	ROM_LOAD( "vtb5.6l",  0x0400, 0x0100, CRC(712ac508) SHA1(5349d722ab6733afdda65f6e0a98322f0d515e86) )    // interrupt timing (not used)
	ROM_LOAD( "vtb6.6e",  0x0500, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )    // video timing (not used)
ROM_END

ROM_START( mercenario )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "4ac.bin",  0x0000, 0x4000, CRC(59ebf408) SHA1(ce2c06580a9fc902b1f6409249f25eba8216af8a) )
	ROM_LOAD( "4bc.bin",  0x4000, 0x4000, CRC(aca99905) SHA1(609cf3d180ceb0c67e2deff9db16fa56c8948a97) )
	ROM_LOAD( "b3.8n",    0x8000, 0x4000, CRC(f998d08a) SHA1(05d86daeaaffbf5f67c77b630b91064c9ae7b6d4) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "cm02.9f",  0x0000, 0x4000, CRC(f9cc4a74) SHA1(ee8dd73919c6f47f62cc6d999de9510db9f79b8f) )    // 2c.9f

	ROM_REGION( 0x4000, "chars", 0 )
	ROM_LOAD( "1c.5d",    0x0000, 0x4000, CRC(fe3ebe35) SHA1(298f1620e83929af01bc9c999262c022e9421c41) )

	ROM_REGION( 0x18000, "tiles", 0 )
	ROM_LOAD( "vt11.5a",  0x00000, 0x4000, CRC(7b2e1b48) SHA1(5d49e1d8146e4ef744445b68f35677302e875a85) )   // 11c.5a
	ROM_LOAD( "vt12.6a",  0x04000, 0x4000, CRC(81b417d3) SHA1(5ec7e3f0c8069384a5f6eb39232c228b9d7b8c0c) )   // 12c.6a
	ROM_LOAD( "vt13.7a",  0x08000, 0x4000, CRC(5612dbd2) SHA1(9e4e1a22b6cbf60607b9a81dae34482ae55f7c47) )   // 13c.7a
	ROM_LOAD( "vt14.8a",  0x0c000, 0x4000, CRC(2b2dee36) SHA1(8792278464fa3da47176582025f6673a15a581e2) )   // 14c.8a
	ROM_LOAD( "vt15.9a",  0x10000, 0x4000, CRC(de70babf) SHA1(6717e23baf55f84d3143fb432140a7c3e102ac26) )   // 15c.9a
	ROM_LOAD( "vt16.10a", 0x14000, 0x4000, CRC(14178237) SHA1(f896e71c7004349c9a46155edfd9f0aaa186065d) )   // 16c.10a

	ROM_REGION( 0x18000, "sprites", 0 )
	ROM_LOAD( "vt05.7e",  0x00000, 0x4000, CRC(79f16e3d) SHA1(04e1f03a4d6b4cc2b81bce3a290bbb95de900d35) )   // 5c.7e
	ROM_LOAD( "vt06.8e",  0x04000, 0x4000, CRC(26fee521) SHA1(2fbfc73ee860f72a20229a01d4da9f5cc2e858d3) )   // 6c.8e
	ROM_LOAD( "vt07.9e",  0x08000, 0x4000, CRC(ca88bdfd) SHA1(548b05460bc7983cc81f15c70e87f47d10db2812) )   // 7c.9e
	ROM_LOAD( "vt08.7h",  0x0c000, 0x4000, CRC(2019c883) SHA1(883c0156ceab99f4849fe36972c4162b4ac8c216) )   // 8c.7h
	ROM_LOAD( "vt09.8h",  0x10000, 0x4000, CRC(98703982) SHA1(ba9a9b0dcadd4f52502828408c4a19b0bd518351) )   // 9c.8h
	ROM_LOAD( "vt10.9h",  0x14000, 0x4000, CRC(f069d2f8) SHA1(2c92300a9407470b34965021de882f1f7a84730c) )   // 10c.9h

	ROM_REGION( 0x600, "proms", 0 )
	ROM_LOAD( "vtb1.1d",  0x0000, 0x0100, CRC(3aba15a1) SHA1(8b057f6e26155dd9e48bde182e680fce4519f600) )    // red
	ROM_LOAD( "vtb2.2d",  0x0100, 0x0100, CRC(88865754) SHA1(ca6dddca98baf00a65b2fb70b69cf4704ef8c831) )    // green
	ROM_LOAD( "vtb3.3d",  0x0200, 0x0100, CRC(4c14c3f6) SHA1(644ac17c7413f094ec9a15cba87bbd421b26321f) )    // blue
	ROM_LOAD( "vtb4.1h",  0x0300, 0x0100, CRC(b388c246) SHA1(038f9851699331ad887b6281a9df053dca3db8fd) )    // palette selector (not used)
	ROM_LOAD( "vtb5.6l",  0x0400, 0x0100, CRC(712ac508) SHA1(5349d722ab6733afdda65f6e0a98322f0d515e86) )    // interrupt timing (not used)
	ROM_LOAD( "vtb6.6e",  0x0500, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )    // video timing (not used)
ROM_END


// Driver Initialization

void commando_state::init_commando()
{
	uint8_t *rom = memregion("maincpu")->base();

	// the first opcode is *not* encrypted
	m_decrypted_opcodes[0] = rom[0];
	for (int A = 1; A < 0xc000; A++)
	{
		uint8_t src = rom[A];
		m_decrypted_opcodes[A] = (src & 0x11) | ((src & 0xe0) >> 4) | ((src & 0x0e) << 4);
	}
}

void commando_state::init_spaceinv()
{
	uint8_t *rom = memregion("maincpu")->base();

	// the first opcode *is* encrypted
	for (int A = 0; A < 0xc000; A++)
	{
		uint8_t src = rom[A];
		m_decrypted_opcodes[A] = (src & 0x11) | ((src & 0xe0) >> 4) | ((src & 0x0e) << 4);
	}
}

} // anonymous namespace


// Game Drivers

GAME( 1985, commando,   0,        commando, commando, commando_state, init_commando, ROT270, "Capcom",                         "Commando (World)",                 MACHINE_SUPPORTS_SAVE )
GAME( 1985, commandou,  commando, commando, commandou,commando_state, init_commando, ROT270, "Capcom (Data East USA license)", "Commando (US set 1)",              MACHINE_SUPPORTS_SAVE )
GAME( 1985, commandou2, commando, commando, commando, commando_state, init_commando, ROT270, "Capcom (Data East USA license)", "Commando (US set 2)",              MACHINE_SUPPORTS_SAVE )
GAME( 1985, commandoj,  commando, commando, commando, commando_state, init_commando, ROT270, "Capcom",                         "Senjou no Ookami",                 MACHINE_SUPPORTS_SAVE )
GAME( 1985, commandob,  commando, commando, commando, commando_state, init_spaceinv, ROT270, "bootleg",                        "Commando (bootleg set 1)",         MACHINE_SUPPORTS_SAVE )
GAME( 1985, commandob2, commando, commando, commando, commando_state, init_commando, ROT270, "bootleg",                        "Commando (bootleg set 2)",         MACHINE_SUPPORTS_SAVE )
GAME( 1985, commandob3, commando, commando, commando, commando_state, init_commando, ROT270, "bootleg",                        "Commando (bootleg set 3)",         MACHINE_SUPPORTS_SAVE )
GAME( 1985, sinvasn,    commando, commando, commando, commando_state, init_commando, ROT270, "Capcom",                         "Space Invasion (Europe)",          MACHINE_SUPPORTS_SAVE )
GAME( 1985, sinvasnb,   commando, commando, commando, commando_state, init_spaceinv, ROT270, "bootleg",                        "Space Invasion (bootleg)",         MACHINE_SUPPORTS_SAVE )
GAME( 1985, mercenario, commando, commando, commando, commando_state, init_spaceinv, ROT270, "bootleg",                        "Mercenario (bootleg of Commando)", MACHINE_SUPPORTS_SAVE )
