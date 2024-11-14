// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/***************************************************************************

Side Pocket - (c) 1986 Data East

The original board has an 8751 protection MCU

Ernesto Corvi
ernesto@imagina.com

Thanks must go to Mirko Buffoni for testing the music.

i8751 protection simulation and other fixes by Bryan McPhail, 15/10/00.


ToDo:
- sidepcktj: Intermission screen's background for player 2 is completely screwed (Cause is currently unknown)


Stephh's notes (based on the games M6809 code and some tests) :

1) 'sidepckt'

  - World version.
  - Credits are BCD coded on 1 byte (range 0x00-0x99) at location 0x0007.
  - Bonus lives settings don't match the Dip Switches page : even if the table at 0x40af (4 * 2 words) is good,
    there's a ingame bug in code at 0x4062 :

      4062: CE 40 AF         LDU   #$40AF         U = 40AF
      4065: F6 30 03         LDB   $3003
      4068: 53               COMB
      4069: 54               LSRB
      406A: 54               LSRB
      406B: C4 0C            ANDB  #$0C
      406D: EC C5            LDD   B,U            U still = 40AF
      406F: 33 42            LEAU  $2,U           U ALWAYS = 40B1

    So 2nd and next extra lives are ALWAYS set to 50k+ regardless of the Dip Switches settings !
  - Player 2 controls are never used ingame for player 2 due to extra code at 0x5a35 :

      5A2E: 96 1A            LDA   $1A
      5A30: 84 01            ANDA  #$01           A = 00 for player 1 and 01 for player 2
      5A32: 8E 30 00         LDX   #$3000
      5A35: D6 CA            LDB   $CA            B ALWAYS = 01 due to initialisation of $CA at 0x43f4
      5A37: C5 01            BITB  #$01
      5A39: 26 02            BNE   $5A3D          always jumps to 0x53ad
      5A3B: 30 86            LEAX  A,X            this instruction is NEVER executed
      5A3D: A6 84            LDA   ,X

  - Player 2 controls are also never used for player 2 when entering initials due to extra code at 0x8baf :

      8BAF: 96 1A            LDA   $1A
      8BB1: 84 01            ANDA  #$01           A = 00 for player 1 and 01 for player 2
      8BB3: D6 CA            LDB   $CA            B ALWAYS = 01 due to initialisation of $CA at 0x43f4
      8BB5: 27 02            BEQ   $8BB9          never jumps to 0x8bb9
      8BB7: 86 00            LDA   #$00
      8BB9: 8E 30 00         LDX   #$3000
      8BBC: A6 86            LDA   A,X

  - Screen never flips ingame or in "continue" screen for player 2 due to code at 0x662f :

      662F: 0F 0A            CLR   $0A            $CA = 0
      6631: D6 1A            LDB   $1A
      6633: 27 00            BEQ   $6635          continue regardless of player
      6635: CC 00 00         LDD   #$0000

      75DF: D6 0A            LDB   $0A
      75E1: F7 30 0C         STB   $300C

    Surprisingly, the screen might flip for player 2 after GAME OVER due to original code at 0x4de8 :

      4DE8: D6 1A            LDB   $1A            A = 00 for player 1 and 01 for player 2
      4DEA: 27 03            BEQ   $4DEF
      4DEC: F7 30 0C         STB   $300C
      4DEF: C6 20            LDB   #$20


2) 'sidepcktj'

  - Japan version.
  - Credits are coded on 1 byte (range 0x00-0xff) at location 0x0007, but their display is limited to 9.
  - Same bonus lives ingame bug as in 'sidepckt'.
  - Player 2 controls are always used ingame for player 2 due to code at 0x58ab :

      58AB: 96 1A            LDA   $1A
      58AD: 84 01            ANDA  #$01           A = 00 for player 1 and 01 for player 2
      58AF: 8E 30 00         LDX   #$3000
      58B2: A6 86            LDA   A,X

  - Player 2 controls are also always used for player 2 when entering initials due to extra code at 0x8b9f :

      8B9F: 96 1A            LDA   $1A
      8BA1: 84 01            ANDA  #$01           A = 00 for player 1 and 01 for player 2
      8BA3: 8E 30 00         LDX   #$3000
      8BA6: A6 86            LDA   A,X

  - Screen always flips ingame or in "continue" screen for player 2 due to code at 0x662f :

      6473: 0F 0A            CLR   $0A            $CA = 0
      6475: D6 1A            LDB   $1A            A = 00 for player 1 and 01 for player 2
      6477: 27 04            BEQ   $647D          jumps if player 1
      6479: 86 20            LDA   #$20
      647B: 97 0A            STA   $0A            $CA = 0x20
      647D: CC 00 00         LDD   #$0000

      75A0: D6 0A            LDB   $0A
      75A2: F7 30 0C         STB   $300C

    After GAME OVER, code at 0x4d16 is slightly different than in 'sidepckt' :

      4D16: D6 1A            LDB   $1A            A = 00 for player 1 and 01 for player 2
      4D18: 27 07            BEQ   $4D21          jumps if player 1
      4D1A: C6 20            LDB   #$20
      4D1C: D7 0A            STB   $0A            $CA = 0x20
      4D1E: F7 30 0C         STB   $300C          flip screen
      4D21: C6 20            LDB   #$20

3) 'sidepcktb'

  - Bootleg heavily based on the World version, so ingame bugs about bonus lives, player 2 inputs and screen flipping are still there.
  - 2 little differences :
      * Lives settings (table at 0x4696) : 06 03 02 instead of 06 03 09
      * Timer settings (table at 0x9d99) : 30 20 18 instead of 40 30 20, so the timer is faster

Additional notes:
----------------
- sidepckt and sidepcktb don't have cocktail mode at all, while sidepcktj has a 'cooperative' cocktail mode; when it's the p2 turn,
  the screen scrolls and a 'flipped score area' is shown on the other side, so the 2nd player just continues the same game.
  Note that the screen never flips in any case.

***************************************************************************/

#include "emu.h"

#include "cpu/m6502/m6502.h"
#include "cpu/m6809/m6809.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/gen_latch.h"
#include "sound/ymopl.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class sidepckt_state : public driver_device
{
public:
	sidepckt_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram")
	{ }

	void sidepcktb(machine_config &config);

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	void bootleg_main_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;

private:
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_spriteram;

	tilemap_t *m_bg_tilemap = nullptr;

	uint8_t m_scroll_y = 0;

	void videoram_w(offs_t offset, uint8_t data);
	void colorram_w(offs_t offset, uint8_t data);
	uint8_t scroll_y_r();
	void scroll_y_w(uint8_t data);

	TILE_GET_INFO_MEMBER(get_tile_info);

	void palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);

	void sound_map(address_map &map) ATTR_COLD;
};

class sidepckt_mcu_state : public sidepckt_state
{
public:
	sidepckt_mcu_state(const machine_config &mconfig, device_type type, const char *tag) :
		sidepckt_state(mconfig, type, tag),
		m_mcu(*this, "mcu")
	{ }

	void sidepckt(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<i8751_device> m_mcu;

	uint8_t m_mcu_p1 = 0;
	uint8_t m_mcu_p2 = 0;
	uint8_t m_mcu_p3 = 0;

	uint8_t mcu_r();
	void mcu_w(uint8_t data);

	void mcu_p1_w(uint8_t data);
	uint8_t mcu_p2_r();
	void mcu_p3_w(uint8_t data);

	void original_main_map(address_map &map) ATTR_COLD;
};


void sidepckt_state::palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();

	for (int i = 0; i < palette.entries(); i++)
	{
		int bit0, bit1, bit2, bit3;

		// red component
		bit0 = BIT(color_prom[i], 4);
		bit1 = BIT(color_prom[i], 5);
		bit2 = BIT(color_prom[i], 6);
		bit3 = BIT(color_prom[i], 7);
		int const r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		// green component
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		bit3 = BIT(color_prom[i], 3);
		int const g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		// blue component
		bit0 = BIT(color_prom[i + palette.entries()], 0);
		bit1 = BIT(color_prom[i + palette.entries()], 1);
		bit2 = BIT(color_prom[i + palette.entries()], 2);
		bit3 = BIT(color_prom[i + palette.entries()], 3);
		int const b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(sidepckt_state::get_tile_info)
{
	uint8_t attr = m_colorram[tile_index];
	tileinfo.set(0,
			m_videoram[tile_index] + ((attr & 0x07) << 8),
			((attr & 0x10) >> 3) | ((attr & 0x20) >> 5),
			TILE_FLIPX);
	tileinfo.group = (attr & 0x80) >> 7;
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void sidepckt_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(sidepckt_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8,8, 32,32);

	m_bg_tilemap->set_transmask(0, 0xff, 0x00); // split type 0 is totally transparent in front half
	m_bg_tilemap->set_transmask(1, 0x01, 0xfe); // split type 1 has pen 0 transparent in front half

	machine().tilemap().set_flip_all(TILEMAP_FLIPX);

	save_item(NAME(m_scroll_y));
}



/***************************************************************************

  Memory handlers

***************************************************************************/

void sidepckt_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void sidepckt_state::colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

uint8_t sidepckt_state::scroll_y_r()
{
	return m_scroll_y;
}

void sidepckt_state::scroll_y_w(uint8_t data)
{
	// Bits 0-5: Scroll y
	m_scroll_y = data & 0x3f;

	// Other bits: Unknown, but they seem never written
	if (data > 0x3f)
		logerror ("scroll_y_w: Unknown write -> data = 0x%02X\n", data);
}


/***************************************************************************

  Display refresh

***************************************************************************/

void sidepckt_state::draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	for (int offs = 0; offs < m_spriteram.bytes(); offs += 4)
	{
		int const attr  = m_spriteram[offs | 1];
		int const code  = ((attr & 0x03) << 8) | m_spriteram[offs | 3];
		int const color = (attr & 0xf0) >> 4;

		int const sx = m_spriteram[offs | 2] - 2;
		int const sy = m_spriteram[offs];

		int const flipx = attr & 0x08;
		int const flipy = attr & 0x04;

		m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
				code,
				color,
				flipx, flipy,
				sx, sy, 0);

		// wraparound
		m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
				code,
				color,
				flipx, flipy,
				sx - 256, sy, 0);
	}
}


uint32_t sidepckt_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrolly(0, m_scroll_y);

	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1, 0);
	draw_sprites(bitmap, cliprect);
	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0, 0);
	return 0;
}


//**************************************************************************
//  PROTECTION MCU
//**************************************************************************

uint8_t sidepckt_mcu_state::mcu_r()
{
	return m_mcu_p1;
}

void sidepckt_mcu_state::mcu_w(uint8_t data)
{
	m_mcu_p2 = data;
	m_mcu->set_input_line(MCS51_INT0_LINE, ASSERT_LINE);
}

void sidepckt_mcu_state::mcu_p1_w(uint8_t data)
{
	m_mcu_p1 = data;
}

uint8_t sidepckt_mcu_state::mcu_p2_r()
{
	return m_mcu_p2;
}

void sidepckt_mcu_state::mcu_p3_w(uint8_t data)
{
	// 765432--  unused
	// ------1-  MCU int ack
	// -------0  CPU firq

	if (BIT(data, 0) == 0 && BIT(m_mcu_p3, 0) == 1)
		m_maincpu->set_input_line(M6809_FIRQ_LINE, ASSERT_LINE);

	if (BIT(data, 0) == 1 && BIT(m_mcu_p3, 0) == 0)
		m_maincpu->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE);

	if (BIT(data, 1) == 0 && BIT(m_mcu_p3, 1) == 1)
		m_mcu->set_input_line(MCS51_INT0_LINE, CLEAR_LINE);

	m_mcu_p3 = data;
}


void sidepckt_mcu_state::machine_start()
{
	sidepckt_state::machine_start();

	save_item(NAME(m_mcu_p1));
	save_item(NAME(m_mcu_p2));
	save_item(NAME(m_mcu_p3));
}

/******************************************************************************/

void sidepckt_state::bootleg_main_map(address_map &map)
{
	map(0x0000, 0x0fff).ram();
	map(0x1000, 0x13ff).mirror(0x400).ram().w(FUNC(sidepckt_state::videoram_w)).share(m_videoram);
	map(0x1800, 0x1bff).mirror(0x400).ram().w(FUNC(sidepckt_state::colorram_w)).share(m_colorram);
	map(0x2000, 0x20ff).ram().share(m_spriteram);
	map(0x2100, 0x24ff).nopw(); // ??? (Unused spriteram? The game writes some values at boot, but never read)
	map(0x3000, 0x3000).portr("P1");
	map(0x3001, 0x3001).portr("P2");
	map(0x3002, 0x3002).portr("DSW1");
	map(0x3003, 0x3003).portr("DSW2");
	map(0x3004, 0x3004).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x300c, 0x300c).rw(FUNC(sidepckt_state::scroll_y_r), FUNC(sidepckt_state::scroll_y_w));
	map(0x4000, 0xffff).rom();
}

void sidepckt_mcu_state::original_main_map(address_map &map)
{
	bootleg_main_map(map);

	map(0x3014, 0x3014).r(FUNC(sidepckt_mcu_state::mcu_r));
	map(0x3018, 0x3018).w(FUNC(sidepckt_mcu_state::mcu_w));
}


void sidepckt_state::sound_map(address_map &map)
{
	map(0x0000, 0x0fff).ram();
	map(0x1000, 0x1001).w("ym1", FUNC(ym2203_device::write));
	map(0x2000, 0x2001).w("ym2", FUNC(ym3526_device::write));
	map(0x3000, 0x3000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x8000, 0xffff).rom();
}


/******************************************************************************/

// verified from M6809 code
static INPUT_PORTS_START( sidepckt )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("P2")     // see notes
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, "Timer Speed" )               // table at 0x9d99
	PORT_DIPSETTING(    0x00, "Stopped (Cheat)")
	PORT_DIPSETTING(    0x03, "Slow" )                      // 0x40 - "Normal" in the Dip Switches page
	PORT_DIPSETTING(    0x02, DEF_STR( Medium ) )           // 0x30 - "Bit fast" in the Dip Switches page
	PORT_DIPSETTING(    0x01, "Fast" )                      // 0x20 - "Fast" in the Dip Switches page
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x0c, "6" )
	PORT_DIPSETTING(    0x04, "9" )
	PORT_DIPSETTING(    0x00, "Infinite (Cheat)")           // always gives 6 balls
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )       // table at 0x40af (4 * 2 words) - see notes
	PORT_DIPSETTING(    0x30, "10k 60k 50k+" )              // "10000, after each 50000" in the Dip Switches page
	PORT_DIPSETTING(    0x20, "20k 70k 50k+" )              // "20000, after each 70000" in the Dip Switches page
	PORT_DIPSETTING(    0x00, "20k 70k 50k+" )              // "20000" in the Dip Switches page
	PORT_DIPSETTING(    0x10, "30k 80k 50k+" )              // "30000, after each 100000" in the Dip Switches page
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )
INPUT_PORTS_END

// verified from M6809 code
static INPUT_PORTS_START( sidepcktj )
	PORT_INCLUDE(sidepckt)

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
INPUT_PORTS_END

// verified from M6809 code
static INPUT_PORTS_START( sidepcktb )
	PORT_INCLUDE(sidepckt)

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x03, 0x03, "Timer Speed" )
	PORT_DIPSETTING(    0x00, "Stopped (Cheat)")
	PORT_DIPSETTING(    0x03, DEF_STR( Medium ) )           // 0x30
	PORT_DIPSETTING(    0x02, "Fast" )                      // 0x20
	PORT_DIPSETTING(    0x01, "Fastest" )                   // 0x18
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x0c, "6" )
	PORT_DIPSETTING(    0x00, "Infinite (Cheat)")           // always gives 6 balls
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,    // 8*8 characters
	2048,   // 2048 characters
	3,      // 3 bits per pixel
	{ 0, 0x8000*8, 0x10000*8 },     // the bitplanes are separated
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8     // every char takes 8 consecutive bytes
};

static const gfx_layout spritelayout =
{
	16,16,  // 16*16 sprites
	1024,   // 1024 sprites
	3,      // 3 bits per pixel
	{ 0, 0x8000*8, 0x10000*8 },     // the bitplanes are separated
	{ 128+0, 128+1, 128+2, 128+3, 128+4, 128+5, 128+6, 128+7, 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8    // every char takes 8 consecutive bytes
};

static GFXDECODE_START( gfx_sidepckt )
	GFXDECODE_ENTRY( "chars",   0, charlayout,   128,  4 ) // colors 128-159
	GFXDECODE_ENTRY( "sprites", 0, spritelayout,   0, 16 ) // colors   0-127
GFXDECODE_END


void sidepckt_state::machine_reset()
{
	m_scroll_y = 0;
}

void sidepckt_state::sidepcktb(machine_config &config)
{
	// basic machine hardware
	MC6809E(config, m_maincpu, 12_MHz_XTAL / 6); // MC68B09EP, 2 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &sidepckt_state::bootleg_main_map);

	M6502(config, m_audiocpu, 12_MHz_XTAL / 8); // 1.5 MHz
	m_audiocpu->set_addrmap(AS_PROGRAM, &sidepckt_state::sound_map);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(58); // VERIFY: May be 55 or 56
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(sidepckt_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set_inputline(m_maincpu, INPUT_LINE_NMI);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_sidepckt);
	PALETTE(config, m_palette, FUNC(sidepckt_state::palette), 256);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	ym2203_device &ym1(YM2203(config, "ym1", 12_MHz_XTAL / 8)); // 1.5 MHz
	ym1.add_route(ALL_OUTPUTS, "mono", 0.25);

	ym3526_device &ym2(YM3526(config, "ym2", 12_MHz_XTAL / 4)); // 3 MHz
	ym2.irq_handler().set_inputline(m_audiocpu, M6502_IRQ_LINE);
	ym2.add_route(ALL_OUTPUTS, "mono", 1.0);
}

void sidepckt_mcu_state::sidepckt(machine_config &config)
{
	sidepcktb(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &sidepckt_mcu_state::original_main_map);

	I8751(config, m_mcu, 8_MHz_XTAL); // 8.0MHz OSC on PCB
	m_mcu->port_out_cb<1>().set(FUNC(sidepckt_mcu_state::mcu_p1_w));
	m_mcu->port_in_cb<2>().set(FUNC(sidepckt_mcu_state::mcu_p2_r));
	m_mcu->port_out_cb<3>().set(FUNC(sidepckt_mcu_state::mcu_p3_w));

	// needs a tight sync with the mcu
	config.set_perfect_quantum(m_maincpu);
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( sidepckt ) // DE-0245-2
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dh00-e.3c", 0x00000, 0x10000, CRC(251b316e) SHA1(c777d87621b8fefe0e33156be03da8aed733db9a) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "dh04.3h", 0x08000, 0x8000, CRC(d076e62e) SHA1(720ff1a6a58697b4a9c7c4f31c24a2cf8a04900a) ) // is this really DH-04-E??

	ROM_REGION( 0x1000, "mcu", 0 ) // i8751 MCU (BAD_DUMP because it was created from the Japanese version)
	ROM_LOAD( "dh-e.6d", 0x0000, 0x1000, BAD_DUMP CRC(00654574) SHA1(7d775e7b7cbb548c50b9b838a525a12bf7a32f8e) )

	ROM_REGION( 0x18000, "chars", 0 )
	ROM_LOAD( "dh07-e.13k", 0x00000, 0x8000, CRC(9d6f7969) SHA1(583852be0861a89c63ce09eb39146ec379b9e12d) )
	ROM_LOAD( "dh06-e.13j", 0x08000, 0x8000, CRC(580e4e43) SHA1(de152a5d4fbc52d80e3eb9af17835ecb6258d45e) )
	ROM_LOAD( "dh05-e.13h", 0x10000, 0x8000, CRC(05ab71d2) SHA1(6f06d1d1440a5fb05c01f712457d0bb167e93099) )

	ROM_REGION( 0x18000, "sprites", 0 )
	ROM_LOAD( "dh01.14a", 0x00000, 0x8000, CRC(a2cdfbea) SHA1(0721e538e3306d616f11008f784cf21e679f330d) )
	ROM_LOAD( "dh02.15a", 0x08000, 0x8000, CRC(eeb5c3e7) SHA1(57eda1cc29124e04fe5025a904634d8ca52c0f12) )
	ROM_LOAD( "dh03.17a", 0x10000, 0x8000, CRC(8e18d21d) SHA1(74f0ddf1fcbed386332eba882b4136295b4f096d) )

	ROM_REGION( 0x0200, "proms", 0 ) // color PROMs
	ROM_LOAD( "dh-09.16l", 0x0000, 0x0100, CRC(ce049b4f) SHA1(e4918cef7b319dd40cf1722eb8bf5e79be04fd6c) ) // MMI 6309-1N BPROM
	ROM_LOAD( "dh-08.15l", 0x0100, 0x0100, CRC(cdf2180f) SHA1(123215d096f88b66396d40d7a579380d0b5b2b89) ) // MMI 6303-1N BPROM
ROM_END

ROM_START( sidepcktj ) // DE-0245-1
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dh00-1.3c", 0x00000, 0x10000, CRC(a66bc28d) SHA1(cd62ce1dce6fe42d9745eec50d11e86b076d28e1) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "dh04_6-19.3h", 0x08000, 0x8000, CRC(053ff83a) SHA1(e6e3ce15a86172bdc6094b4999e52d1aafc0ae10) ) // handwritten 6-19 on label

	ROM_REGION( 0x1000, "mcu", 0 ) // i8751 MCU
	ROM_LOAD( "dh.6d", 0x0000, 0x1000, CRC(f7e099b6) SHA1(8e718384489a589acebc19ca361e0aa8a4c6b63b) )

	ROM_REGION( 0x18000, "chars", 0 )
	ROM_LOAD( "dh07.13k", 0x00000, 0x8000, CRC(7d0ce858) SHA1(3a158f218a762e6841d2611f41ace67a1afefb35) )
	ROM_LOAD( "dh06.13j", 0x08000, 0x8000, CRC(b86ddf72) SHA1(7596dd1b646971d8df1bc4fd157ccf161a712d59) )
	ROM_LOAD( "dh05.13h", 0x10000, 0x8000, CRC(df6f94f2) SHA1(605796191f37cb76d496aa459243655070bb90c0) )

	ROM_REGION( 0x18000, "sprites", 0 )
	ROM_LOAD( "dh01.14a", 0x00000, 0x8000, CRC(a2cdfbea) SHA1(0721e538e3306d616f11008f784cf21e679f330d) )
	ROM_LOAD( "dh02.15a", 0x08000, 0x8000, CRC(eeb5c3e7) SHA1(57eda1cc29124e04fe5025a904634d8ca52c0f12) )
	ROM_LOAD( "dh03.17a", 0x10000, 0x8000, CRC(8e18d21d) SHA1(74f0ddf1fcbed386332eba882b4136295b4f096d) )

	ROM_REGION( 0x0200, "proms", 0 ) // color PROMs
	ROM_LOAD( "dh-09.16l", 0x0000, 0x0100, CRC(ce049b4f) SHA1(e4918cef7b319dd40cf1722eb8bf5e79be04fd6c) ) // MMI 6309-1N BPROM
	ROM_LOAD( "dh-08.15l", 0x0100, 0x0100, CRC(cdf2180f) SHA1(123215d096f88b66396d40d7a579380d0b5b2b89) ) // MMI 6303-1N BPROM
ROM_END

ROM_START( sidepcktb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sp_09.bin", 0x04000, 0x4000, CRC(3c6fe54b) SHA1(4025ac48d75f171f4c979d3fcd6a2f8da18cef4f) )
	ROM_LOAD( "sp_08.bin", 0x08000, 0x8000, CRC(347f81cd) SHA1(5ab06130f35788e51a881cc0f387649532145bd6) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "dh04.3h", 0x08000, 0x8000, CRC(d076e62e) SHA1(720ff1a6a58697b4a9c7c4f31c24a2cf8a04900a) )

	ROM_REGION( 0x18000, "chars", 0 )
	ROM_LOAD( "dh07-e.13k", 0x00000, 0x8000, CRC(9d6f7969) SHA1(583852be0861a89c63ce09eb39146ec379b9e12d) )
	ROM_LOAD( "dh06-e.13j", 0x08000, 0x8000, CRC(580e4e43) SHA1(de152a5d4fbc52d80e3eb9af17835ecb6258d45e) )
	ROM_LOAD( "dh05-e.13h", 0x10000, 0x8000, CRC(05ab71d2) SHA1(6f06d1d1440a5fb05c01f712457d0bb167e93099) )

	ROM_REGION( 0x18000, "sprites", 0 )
	ROM_LOAD( "dh01.14a", 0x00000, 0x8000, CRC(a2cdfbea) SHA1(0721e538e3306d616f11008f784cf21e679f330d) )
	ROM_LOAD( "dh02.15a", 0x08000, 0x8000, CRC(eeb5c3e7) SHA1(57eda1cc29124e04fe5025a904634d8ca52c0f12) )
	ROM_LOAD( "dh03.17a", 0x10000, 0x8000, CRC(8e18d21d) SHA1(74f0ddf1fcbed386332eba882b4136295b4f096d) )

	ROM_REGION( 0x0200, "proms", 0 ) // color PROMs
	ROM_LOAD( "dh-09.16l", 0x0000, 0x0100, CRC(ce049b4f) SHA1(e4918cef7b319dd40cf1722eb8bf5e79be04fd6c) )
	ROM_LOAD( "dh-08.15l", 0x0100, 0x0100, CRC(cdf2180f) SHA1(123215d096f88b66396d40d7a579380d0b5b2b89) )
ROM_END

} // anonymous namespace


GAME( 1986, sidepckt,  0,        sidepckt,  sidepckt,  sidepckt_mcu_state, empty_init, ROT0, "Data East Corporation", "Side Pocket (World)",           MACHINE_SUPPORTS_SAVE )
GAME( 1986, sidepcktj, sidepckt, sidepckt,  sidepcktj, sidepckt_mcu_state, empty_init, ROT0, "Data East Corporation", "Side Pocket (Japan, Cocktail)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, sidepcktb, sidepckt, sidepcktb, sidepcktb, sidepckt_state,     empty_init, ROT0, "bootleg",               "Side Pocket (bootleg)",         MACHINE_SUPPORTS_SAVE )
