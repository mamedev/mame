// license:BSD-3-Clause
// copyright-holders: David Haywood, Paul Priest

/* One Shot One Kill & Maddonna
   Driver by David Haywood and Paul Priest
   Dip Switches and Inputs by stephh

Hardware identified by Tuning manual as "VISYS-Board(R) (Video-System-Board)"

Notes :
  - The YM3812 is used only for timing. All sound is played with ADPCM samples.

  - It doesn't seem possible to make 2 consecutive shots at the same place !
    In-game bug or is there something missing in the emulation ?
    * several gun games are like this, changed the driver so it doesn't try

  - Gun X range is 0x0000-0x01ff and gun Y range is 0x0000-0x00ff, so you
    can shoot sometimes out of the "visible area" ... NOT A BUG !
  - Player 1 and 2 guns do NOT use the same routine to determine the
    coordinates of an impact on the screen : position both guns in the
    "upper left" corner in the "gun test" to see what I mean.
  - I've assumed that the shot was right at the place the shot was made,
    but I don't have any more information about that
    (what the hell is "Gun X Shift Left" REALLY used for ?)

TO DO :

  - fix some priorities for some tiles
  - verify the parameters for the guns (analog ports)
  - figure out year and manufacturer
    (NOTHING is displayed in "demo mode", nor when you complete ALL levels !)
  - sound too fast in Mad Donna?
  - layer order register?

NOTE: An eBay auction of the PCB shows "1996.9.16 PROMAT" on the JAMMA+ adapter for
      One Shot One Kill.  This information was used for the year & manufacturer.
      Also listed in an Approved Game list on a HK government site as "Promet"


Main board:
+---------------------------------------------+
|      5MHz                    +------------+ |
|VOL  YM3812  Z80A  M6295      |25x6 row pin| |
| YM3014 6116           PAL    +------------+ |
|        PAL                                  |
|                               +---------+   |
|                    KM681000LP | ACTEL   |   |
|                               | A1020A  |   |
|                    KM681000LP | PL84C   |   |
|J                              +---------+   |
|A                                            |
|M                   +---------++---------+   |
|M       MB.ROM      | Cypress || ACTEL   |   |
|A       KM6264      | CY7C384A|| A1020A  |   |
|        KM6264 PAL  |  -XJC   || PL84C   |   |
|                    +---------++---------+   |
|             PAL PAL   KM681000LP            |
| DSW2       6     6116 KM681000LP  PAL       |
|            8     6116             PAL       |
| DSW1       0                 +------------+ |
|     27MHz  0  CXK58256       |25x4 row pin| |
|LED  12MHz  0  CXK58256       +------------+ |
+---------------------------------------------+

  CPU: 68000, Z80A
Sound: YM3812/YM3014 + OKI M6295 (badged as K-666/K-664 + AD-65)
  OSC: 27MHz, 12MHz & 5MHz
  RAM: Samsung KM681000LP-10 128Kx8 Low power SRAM x 4
       Samsung KM6264AL-10 8Kx8 Low power SRAM x 2
       Sony CXK58256-10L 32Kx8 High speed SRAM x 2
       UMC UM6116K-2 2Kx8 SRAM x 3
Other: Cypress CY7C384A 2K Very high speed Gate CMOS FPGA
       Actel A1020A FPGA
       DSW 8 position dipswitch x 2
       LED - Power LED
       VOL - Volume adjust pot
Connectors to ROM board:
  25 pins by 6 rows & 25 pins by 4 rows

Rom board:
+-----------------------+
|        +------------+ |
|        |25x6 row pin| |
|        +------------+ |
|        U15*     UI8   |
|                       |
|        UA2      UI8A  |
|                       |
|        U14*     UI11  |
|                       |
|        UA22     UI11A |
|                       |
|        UA24     UI13  |
|                       |
|  +---------+    UI13A |
|  | ACTEL   |          |
|  | A1020A  |    UI16  |
|  | PL84C   |          |
|  +---------+    UI16A |
|        +------------+ |
|        |25x4 row pin| |
|        +------------+ |
+-----------------------+

Note: * denotes unpopulated

Other: Actel A1020A FPGA
Connectors to main board:
  25 pins by 6 rows & 25 pins by 4 rows

For One Shot One Kill:
 There is an additional board that connects to the JAMMA connector that has some logic chips,
 a JAMMA edge connector and two 4 pin headers to connect the light guns as well as a 2 wire jumper
 that connects to some ROM board logic chips (assumed) for light gun input. Also the YM3812 sound
 chip appears to be unpopulated for One Shot One Kill.
 The ROM board for One Shot One Kill has 8 additional logic chips along the front edge of the PCB


Clock measurements:
 SCN68000C8N64: 12MHz
     Z8400A PS: 3.375MHz (27MHz/8)
        YM3812: 3.375MHz (27MHz/8)
   OKI MSM6295: 1.25MHz, pin 7 is hard-wired to GROUND (5MHz/4)
         VSYNC: 56.05Hz
        H-SYNC: 15.06KHz

*/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


// configurable logging
#define LOG_SCROLL     (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_SCROLL)

#include "logmacro.h"

#define LOGSCROLL(...)     LOGMASKED(LOG_SCROLL,     __VA_ARGS__)


namespace {

class maddonna_state : public driver_device
{
public:
	maddonna_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette"),
		m_scroll(*this, "scroll"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode"),
		m_spriteram(*this, "spriteram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_mid_videoram(*this, "mid_videoram"),
		m_fg_videoram(*this, "fg_videoram")
	{ }

	void maddonna(machine_config &config);
	void komocomo(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;

	required_shared_ptr<u16> m_scroll;

	// video-related
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_mid_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;

	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;

private:
	// devices
	required_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;

	// memory pointers
	required_shared_ptr<u16> m_spriteram;
	required_shared_ptr<u16> m_bg_videoram;
	required_shared_ptr<u16> m_mid_videoram;
	required_shared_ptr<u16> m_fg_videoram;

	void bg_videoram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void mid_videoram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void fg_videoram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void soundbank_w(u8 data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_mid_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	u32 screen_update_maddonna(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_komocomo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void sound_map(address_map &map) ATTR_COLD;
};

class oneshot_state : public maddonna_state
{
public:
	oneshot_state(const machine_config &mconfig, device_type type, const char *tag) :
		maddonna_state(mconfig, type, tag),
		m_io_dsw1(*this, "DSW1"),
		m_io_lightgun_x(*this, "LIGHT%u_X", 0U),
		m_io_lightgun_y(*this, "LIGHT%u_Y", 0U)
	{ }

	void oneshot(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_ioport m_io_dsw1;
	required_ioport_array<2> m_io_lightgun_x;
	required_ioport_array<2> m_io_lightgun_y;

	// gun
	u16 m_gun_x[2]{};
	u16 m_gun_y[2]{};
	u8 m_gun_x_shift = 0;
	u8 m_wobble[2]{};

	u16 in0_word_r();
	template <u8 Which> u16 gun_x_r();
	template <u8 Which> u16 gun_y_r();

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_crosshairs();

	void main_map(address_map &map) ATTR_COLD;
};


// bg tilemap
TILE_GET_INFO_MEMBER(maddonna_state::get_bg_tile_info)
{
	const u32 tileno = m_bg_videoram[tile_index * 2 + 1];

	tileinfo.set(0, tileno, 0, 0);
}

void maddonna_state::bg_videoram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_bg_videoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

// mid tilemap
TILE_GET_INFO_MEMBER(maddonna_state::get_mid_tile_info)
{
	const u32 tileno = m_mid_videoram[tile_index * 2 + 1];

	tileinfo.set(0, tileno, 2, 0);
}

void maddonna_state::mid_videoram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_mid_videoram[offset]);
	m_mid_tilemap->mark_tile_dirty(offset / 2);
}


// fg tilemap
TILE_GET_INFO_MEMBER(maddonna_state::get_fg_tile_info)
{
	const u32 tileno = m_fg_videoram[tile_index * 2 + 1];

	tileinfo.set(0, tileno, 3, 0);
}

void maddonna_state::fg_videoram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_fg_videoram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset / 2);
}

void maddonna_state::video_start()
{
	m_bg_tilemap =  &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(maddonna_state::get_bg_tile_info)),  TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_mid_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(maddonna_state::get_mid_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_fg_tilemap =  &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(maddonna_state::get_fg_tile_info)),  TILEMAP_SCAN_ROWS, 16, 16, 32, 32);

	m_bg_tilemap->set_transparent_pen(0);
	m_mid_tilemap->set_transparent_pen(0);
	m_fg_tilemap->set_transparent_pen(0);
}

void oneshot_state::draw_crosshairs()
{
	//int xpos,ypos;

	// get gun raw coordinates (player 1)
	m_gun_x[0] = (m_io_lightgun_x[0]->read() & 0xff) * 320 / 256;
	m_gun_y[0] = (m_io_lightgun_y[0]->read() & 0xff) * 240 / 256;

	// compute the coordinates for drawing (from routine at 0x009ab0)
	//xpos = m_gun_x[0];
	//ypos = m_gun_y[0];

	m_gun_x[0] += m_gun_x_shift;

	if (m_gun_y[0] >= 0x0a)
		m_gun_y[0] -= 0x0a;
	else
		m_gun_y[0] = 0;

	// get gun raw coordinates (player 2)
	m_gun_x[1] = (m_io_lightgun_x[1]->read() & 0xff) * 320 / 256;
	m_gun_y[1] = (m_io_lightgun_y[1]->read() & 0xff) * 240 / 256;

	// compute the coordinates for drawing (from routine at 0x009b6e)
	//xpos = m_gun_x[1];
	//ypos = m_gun_y[1];

	m_gun_x[1] += m_gun_x_shift;
	if (m_gun_x[1] >= 0x0a)
		m_gun_x[1] -= 0x0a;
	else
		m_gun_x[1] = 0;
}

void maddonna_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const u16 *source = m_spriteram;
	const u16 *finish = source + (0x1000 / 2);
	gfx_element *gfx = m_gfxdecode->gfx(1);

	while (source < finish)
	{
		const u16 attr = source[0];

		if (attr & 0x0001) // end of sprites
			break;

		if (!(attr & 0x8000)) // visible bit
		{
			source += 0x4;
			continue;
		}

		const u32 num = source[1] & 0xffff;
		const int xsize = (source[2] & 0x000f) + 1;
		const int ysize = (source[3] & 0x000f) + 1;

		int ypos = source[3] & 0xff80;
		int xpos = source[2] & 0xff80;

		ypos = ypos >> 7;
		xpos = xpos >> 7;

		xpos -= 8;
		ypos -= 6;

		for (int blockx = 0; blockx < xsize; blockx++)
		{
			for (int blocky = 0; blocky < ysize; blocky++)
			{
						gfx->transpen(
						bitmap,
						cliprect,
						num + (blocky * xsize) + blockx,
						1,
						0, 0,
						xpos + blockx * 8, ypos + blocky * 8, 0);

						gfx->transpen(
						bitmap,
						cliprect,
						num + (blocky * xsize) + blockx,
						1,
						0, 0,
						xpos + blockx * 8 - 0x200, ypos + blocky * 8, 0);
			}
		}
		source += 0x4;
	}
}

u32 oneshot_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);

	m_mid_tilemap->set_scrollx(0, m_scroll[0] - 0x1f5);
	m_mid_tilemap->set_scrolly(0, m_scroll[1]);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_mid_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_crosshairs();
	return 0;
}

u32 maddonna_state::screen_update_maddonna(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);

	m_mid_tilemap->set_scrolly(0, m_scroll[1]); // other registers aren't used so we don't know which layers they relate to

	m_mid_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);

	LOGSCROLL("%04x %04x %04x %04x %04x %04x %04x %04x", m_scroll[0], m_scroll[1], m_scroll[2], m_scroll[3], m_scroll[4], m_scroll[5], m_scroll[6], m_scroll[7]);
	return 0;
}

// why are the layers in a different order?
u32 maddonna_state::screen_update_komocomo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);

	m_mid_tilemap->set_scrolly(0, m_scroll[1]); // other registers aren't used so we don't know which layers they relate to

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_mid_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);

	LOGSCROLL("%04x %04x %04x %04x %04x %04x %04x %04x", m_scroll[0], m_scroll[1], m_scroll[2], m_scroll[3], m_scroll[4], m_scroll[5], m_scroll[6], m_scroll[7]);
	return 0;
}


u16 oneshot_state::in0_word_r()
{
	const u16 data = m_io_dsw1->read();

	switch (data & 0x0c)
	{
		case 0x00 :
			m_gun_x_shift = 35;
			break;
		case 0x04 :
			m_gun_x_shift = 30;
			break;
		case 0x08 :
			m_gun_x_shift = 40;
			break;
		case 0x0c :
			m_gun_x_shift = 50;
			break;
	}

	return data;
}

template <u8 Which>
u16 oneshot_state::gun_x_r()
{
	// shots must be in a different location to register
	m_wobble[Which] ^= 1;

	return m_gun_x[Which] ^ m_wobble[Which];
}

template <u8 Which>
u16 oneshot_state::gun_y_r()
{
	return m_gun_y[Which];
}

void maddonna_state::soundbank_w(u8 data)
{
	m_oki->set_rom_bank((data & 0x03) ^ 0x03);
}



void maddonna_state::main_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x080000, 0x087fff).ram();
	map(0x0c0000, 0x0c07ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x120000, 0x120fff).ram().share(m_spriteram);
	//map(0x13f000, 0x13f000).noprw(); // Unknown read / writes
	map(0x180000, 0x180fff).ram().w(FUNC(maddonna_state::mid_videoram_w)).share(m_mid_videoram); // some people , girl etc.
	map(0x181000, 0x181fff).ram().w(FUNC(maddonna_state::fg_videoram_w)).share(m_fg_videoram); // credits etc.
	map(0x182000, 0x182fff).ram().w(FUNC(maddonna_state::bg_videoram_w)).share(m_bg_videoram); // credits etc.
	map(0x188000, 0x18800f).writeonly().share(m_scroll);
	map(0x190003, 0x190003).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0x190011, 0x190011).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x190019, 0x190019).w(FUNC(maddonna_state::soundbank_w));
	map(0x19c020, 0x19c021).portr("DSW1");
	map(0x19c024, 0x19c025).portr("DSW2");
	map(0x19c02c, 0x19c02d).portr("CREDITS");
	map(0x19c030, 0x19c031).portr("P1");
	map(0x19c034, 0x19c035).portr("P2");
}

void oneshot_state::main_map(address_map &map)
{
	maddonna_state::main_map(map);

	map(0x190026, 0x190027).r(FUNC(oneshot_state::gun_x_r<0>));
	map(0x19002e, 0x19002f).r(FUNC(oneshot_state::gun_x_r<1>));
	map(0x190036, 0x190037).r(FUNC(oneshot_state::gun_y_r<0>));
	map(0x19003e, 0x19003f).r(FUNC(oneshot_state::gun_y_r<1>));
	map(0x19c020, 0x19c021).r(FUNC(oneshot_state::in0_word_r));
}

void maddonna_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x8000).rw("soundlatch", FUNC(generic_latch_8_device::read), FUNC(generic_latch_8_device::write));
	map(0x8001, 0x87ff).ram();
	map(0xe000, 0xe001).rw("ymsnd", FUNC(ym3812_device::read), FUNC(ym3812_device::write));
	map(0xe010, 0xe010).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}


static INPUT_PORTS_START( oneshot )
	PORT_START("DSW1")  // 0x19c020.l -> 0x08006c.l
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) )      // 0x080084.l : credits (00-09)
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x00, "Gun X Shift Left" )      // 0x0824ec.l (not in "test mode")
	PORT_DIPSETTING(    0x04, "30" )
	PORT_DIPSETTING(    0x00, "35" )
	PORT_DIPSETTING(    0x08, "40" )
	PORT_DIPSETTING(    0x0c, "50" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) )  // 0x082706.l - to be confirmed
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_SERVICE( 0x20, IP_ACTIVE_HIGH )            // 0x0824fe.l
	PORT_DIPNAME( 0x40, 0x00, "Start Round" )           // 0x08224e.l
	PORT_DIPSETTING(    0x00, "Gun Trigger" )
	PORT_DIPSETTING(    0x40, "Start Button" )
	PORT_DIPNAME( 0x80, 0x00, "Gun Test" )          // 0x082286.l
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")  // 0x19c024.l -> 0x08006e.l
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )        // 0x082500.l
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPUNUSED( 0x04, IP_ACTIVE_HIGH )
	PORT_DIPUNUSED( 0x08, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Difficulty ) )   // 0x082506.l
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )             // 0
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )           // 1
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )             // 2
	PORT_DIPSETTING(    0x30, DEF_STR( Hardest ) )          // 3
	PORT_DIPNAME( 0x40, 0x00, "Round Select" )      // 0x082f16.l - only after 1st stage
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )      // "On"  in the "test mode"
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )           // "Off" in the "test mode"
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Free_Play ) )    // 0x0800ca.l
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("CREDITS")   // 0x19c02c.l -> 0x08007a.l
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P1")    // Player 1 Gun Trigger (0x19c030.l)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P2")    // Player 2 Gun Trigger (0x19c034.l)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("LIGHT0_X")  // Player 1 Gun X       ($190026.l)
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(35) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("LIGHT0_Y")  // Player 1 Gun Y       ($190036.l)
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(35) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("LIGHT1_X")  // Player 2 Gun X       ($19002e.l)
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(35) PORT_KEYDELTA(15) PORT_PLAYER(2)

	PORT_START("LIGHT1_Y")  // Player 2 Gun Y       ($19003e.l)
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(35) PORT_KEYDELTA(15) PORT_PLAYER(2)

INPUT_PORTS_END

static INPUT_PORTS_START( maddonna )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x04, 0x04, "Girl Pictures" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )      // Not defined in the manual
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )      // Not defined in the manual
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Invulnerability" )       // This one was not defined in the manual
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_HIGH )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )             // 2 Monsters at start, but "dumber"??
	PORT_DIPSETTING(    0x01, DEF_STR( Normal ) )           // 2 Monsters at start
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )             // 3 Monsters at start
	PORT_DIPSETTING(    0x03, DEF_STR( Hardest ) )          // 4 Monsters at start
	PORT_DIPNAME( 0x0c, 0x08, "Time Per Round" )
	PORT_DIPSETTING(    0x08, "80 Seconds" )
	PORT_DIPSETTING(    0x04, "90 Seconds" )
	PORT_DIPSETTING(    0x00, "100 Seconds" )
	PORT_DIPSETTING(    0x0c, "?? Seconds" )        // Not Defined for On+On
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )      // Not defined in the manual
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, "Hurry Up!" )         // Controls "Hurry Up!" banner & Vampire - Not defined the in manual
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )      // No Hurry up
	PORT_DIPSETTING(    0x40, "On - 10" )           // The rest show the banner but is there a difference in how the Vampire shows up???
	PORT_DIPSETTING(    0x80, "On - 01" )
	PORT_DIPSETTING(    0xc0, "On - 11" )

	PORT_START("CREDITS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	// "Very important ! (Sehr WICHTIG) 4-Way Joystick !!!" (Tuning manual)
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


static const gfx_layout oneshot16x16_layout =
{
	16,16,
	RGN_FRAC(1,8),
	8,
	{ RGN_FRAC(0,8),RGN_FRAC(1,8),RGN_FRAC(2,8),RGN_FRAC(3,8),RGN_FRAC(4,8),RGN_FRAC(5,8),RGN_FRAC(6,8),RGN_FRAC(7,8) },
	{ STEP8(0,1), STEP8(8*8,1) },
	{ STEP8(0,8), STEP8(8*8*2,8) },
	16*16
};

static const gfx_layout oneshot8x8_layout =
{
	8,8,
	RGN_FRAC(1,8),
	8,
	{ RGN_FRAC(0,8),RGN_FRAC(1,8),RGN_FRAC(2,8),RGN_FRAC(3,8),RGN_FRAC(4,8),RGN_FRAC(5,8),RGN_FRAC(6,8),RGN_FRAC(7,8) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};


static GFXDECODE_START( gfx_oneshot )
	GFXDECODE_ENTRY( "sprites", 0, oneshot16x16_layout, 0, 4  )
	GFXDECODE_ENTRY( "sprites", 0, oneshot8x8_layout,   0, 4  )
GFXDECODE_END

void oneshot_state::machine_start()
{
	save_item(NAME(m_gun_x));
	save_item(NAME(m_gun_y));
	save_item(NAME(m_gun_x_shift));
	save_item(NAME(m_wobble));
}

void oneshot_state::machine_reset()
{
	m_gun_x[0] = 0;
	m_gun_y[0] = 0;
	m_gun_x[1] = 0;
	m_gun_y[1] = 0;
	m_gun_x_shift = 0;
	m_wobble[0] = 0;
	m_wobble[1] = 0;
}

void maddonna_state::maddonna(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 12_MHz_XTAL); // 12MHz - verified
	m_maincpu->set_addrmap(AS_PROGRAM, &maddonna_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(maddonna_state::irq4_line_hold));

	Z80(config, "audiocpu", 27_MHz_XTAL / 8).set_addrmap(AS_PROGRAM, &maddonna_state::sound_map); // 3.375MHz - verified

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(56);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*16, 32*16);
	screen.set_visarea(0*16, 20*16-1, 0*16, 15*16-1);
	screen.set_screen_update(FUNC(maddonna_state::screen_update_maddonna));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_oneshot);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x400);

	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	ym3812_device &ymsnd(YM3812(config, "ymsnd", 27_MHz_XTAL / 8)); // 3.375MHz - verified
	ymsnd.irq_handler().set_inputline("audiocpu", 0);
	ymsnd.add_route(ALL_OUTPUTS, "mono", 1.0);

	OKIM6295(config, m_oki, 5_MHz_XTAL / 4, okim6295_device::PIN7_LOW); // 1.25MHz - verified, pin 7 is hard-wired to GROUND
	m_oki->add_route(ALL_OUTPUTS, "mono", 1.0);
}

void maddonna_state::komocomo(machine_config &config)
{
	maddonna(config);
	subdevice<screen_device>("screen")->set_screen_update(FUNC(maddonna_state::screen_update_komocomo));
}

void oneshot_state::oneshot(machine_config &config)
{
	maddonna(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &oneshot_state::main_map);

	subdevice<screen_device>("screen")->set_screen_update(FUNC(oneshot_state::screen_update));
}


ROM_START( oneshot )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "1shot-u.a24", 0x00000, 0x20000, CRC(0ecd33da) SHA1(d050e9a1900cd9f629818034b1445e034b6cf81c) )
	ROM_LOAD16_BYTE( "1shot-u.a22", 0x00001, 0x20000, CRC(26c3ae2d) SHA1(47e479abe06d508a9d9fe677d34d6a485bde5533) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Z80 Code
	ROM_LOAD( "1shot.ua2", 0x00000, 0x010000, CRC(f655b80e) SHA1(2574a812c35801755c187a47f46ccdb0983c5feb) )

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD( "1shot-ui.16a", 0x000000, 0x080000, CRC(f765f9a2) SHA1(f6c386e0421fcb0e420585dd27d9dad951bb2556) )
	ROM_LOAD( "1shot-ui.13a", 0x080000, 0x080000, CRC(3361b5d8) SHA1(f7db674d479765d4e58fb663aa5e13dde2abcce7) )
	ROM_LOAD( "1shot-ui.11a", 0x100000, 0x080000, CRC(8f8bd027) SHA1(fbec952ab5604c8e20c5e7cfd2844f4fe5441186) )
	ROM_LOAD( "1shot-ui.08a", 0x180000, 0x080000, CRC(254b1701) SHA1(163bfa70508fca20be70dd0af8b768ab6bf211b9) )
	ROM_LOAD( "1shot-ui.16",  0x200000, 0x080000, CRC(ff246b27) SHA1(fef6029030268174ef9648b8f437aeda68475346) )
	ROM_LOAD( "1shot-ui.13",  0x280000, 0x080000, CRC(80342e83) SHA1(2ac2b300382a607a539d2b0982ab596f05be3ad3) )
	ROM_LOAD( "1shot-ui.11",  0x300000, 0x080000, CRC(b8938345) SHA1(318cf0d070db786680a45811bbd765fa37caaf62) )
	ROM_LOAD( "1shot-ui.08",  0x380000, 0x080000, CRC(c9953bef) SHA1(21917a9dcc0afaeec20672ad863d0c9d583369e3) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "1shot.u15", 0x000000, 0x080000, CRC(e3759a47) SHA1(1159335924a6d68a0a24bfbe0c9182107f3f05f8) )
	ROM_LOAD( "1shot.u14", 0x080000, 0x080000, CRC(222e33f8) SHA1(2665afdf4cb1a29325df62efc1843a4b2cf34a4e) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "1shot.mb", 0x00000, 0x10000, CRC(6b213183) SHA1(599c59d155d11edb151bfaed1d24ef964462a447) ) // motherboard ROM, unknown purpose
ROM_END

/*
For this set of Mad Donna, original ROM labels follow this format:
  MADDONNA
  TMAD_B15  (B15 being the PCB location of ROM)
  160595    (Date, IE: May 16th, 1995)
*/
ROM_START( maddonna )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "maddonna_tmad_b16_160595.ua24", 0x00000, 0x20000, CRC(643f9054) SHA1(77907ecdb02a525f9beed7fee203431eda16c831) )
	ROM_LOAD16_BYTE( "maddonna_tmad_b15_160595.ua22", 0x00001, 0x20000, CRC(e36c0e26) SHA1(f261b2c74eeca05df302aa4956f5d02121d42054) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Z80 Code
	ROM_LOAD( "maddonna_tmad_b13_160595.ua2", 0x00000, 0x010000, CRC(f2080071) SHA1(68cbae9559879b2dc19c41a7efbd13ab4a569d3f) )

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD( "maddonna_tmad_b5_160595.ui16a", 0x000000, 0x080000, CRC(838d3244) SHA1(7339143481ec043219825f282450ff53bb718f8c) )
	ROM_LOAD( "maddonna_tmad_b7_160595.ui13a", 0x080000, 0x080000, CRC(4920d2ec) SHA1(e72a374bca81ffa4f925326455e007df7227ae08) )
	ROM_LOAD( "maddonna_tmad_b9_160595.ui11a", 0x100000, 0x080000, CRC(3a8a3feb) SHA1(832654902963c163644134431fd1221e1895cfec) )
	ROM_LOAD( "maddonna_tmad_b11_160595.ui8a", 0x180000, 0x080000, CRC(6f9b7fdf) SHA1(14ced1d43eae3b6db4a0a4c12fb26cbd13eb7428) )
	ROM_LOAD( "maddonna_tmad_b6_160595.ui16",  0x200000, 0x080000, CRC(b02e9e0e) SHA1(6e527a2bfda0f4f420c10139c75dac2704e08d08) )
	ROM_LOAD( "maddonna_tmad_b8_160595.ui13",  0x280000, 0x080000, CRC(03f1de40) SHA1(bb0c0525155404c0740ac5f048f71ae7651a5941) )
	ROM_LOAD( "maddonna_tmad_b10_160595.ui11", 0x300000, 0x080000, CRC(87936423) SHA1(dda42f3685427edad7686d9712ff07d2fd9bf57e) )
	ROM_LOAD( "maddonna_tmad_b12_160595.ui8",  0x380000, 0x080000, CRC(879ab23c) SHA1(5288016542a10e60ccb28a930d8dfe4db41c6fc6) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 )
	// no samples for this game

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x1", 0x00000, 0x10000, CRC(6b213183) SHA1(599c59d155d11edb151bfaed1d24ef964462a447) ) // motherboard ROM, unknown purpose
ROM_END

ROM_START( komocomo ) // ROM PCB marked : GAME B/D TOPnew1 002
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "11.ua24", 0x00000, 0x10000, CRC(31c18579) SHA1(da97207afced0cf844b111752e9f634a49bc7115) )
	ROM_LOAD16_BYTE( "10.ua22", 0x00001, 0x10000, CRC(fa839c0f) SHA1(53aee489e694e5777bd5ac20aa2b51c2c9e5493a) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Z80 Code
	ROM_LOAD( "9.ua2", 0x00000, 0x010000, CRC(f2080071) SHA1(68cbae9559879b2dc19c41a7efbd13ab4a569d3f) ) // 1ST AND 2ND HALF IDENTICAL

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD( "8.ui16a",  0x000000, 0x080000, CRC(11ac04ab) SHA1(321a7af3bdf47fa0ec1f7bbd758dd1023b409a06) )
	ROM_LOAD( "6.ui13a",  0x080000, 0x080000, CRC(bb498592) SHA1(f7946a40d35ae0726860dd65f1aaf1c145d51afd) )
	ROM_LOAD( "4.ui11a",  0x100000, 0x080000, CRC(d22dd6d8) SHA1(f466167d088275cdc973535401895272d48c8079) )
	ROM_LOAD( "2.ui8a",   0x180000, 0x080000, CRC(cc5360f8) SHA1(61670b970a71af8f008052a98c10127f8b412a85) )
	ROM_LOAD( "7.ui16",   0x200000, 0x080000, CRC(dc71de75) SHA1(adc8f51acf34e41b94a740dc2b7a66b23a7ffcf5) )
	ROM_LOAD( "5.ui13",   0x280000, 0x080000, CRC(56bc3719) SHA1(947d7fe45273052fbd666ad8e9bef9818c6e3104) )
	ROM_LOAD( "3.ui11",   0x300000, 0x080000, CRC(646073d1) SHA1(87593ccbe9419da2ea70763784acc12221b5bed8) )
	ROM_LOAD( "1.ui8",    0x380000, 0x080000, CRC(58ba6622) SHA1(ee5a759ab22a663da5bba1db5fec23958bfee771) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 )
	// no samples for this game

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "12.bin", 0x00000, 0x10000, CRC(6b213183) SHA1(599c59d155d11edb151bfaed1d24ef964462a447) ) // motherboard ROM, unknown purpose
ROM_END


/* The tiles containing the copyright string (tiles 0x3979 onwards) differ in this set.
   Both versions have tiles containing the 'Tuning - Germany' copyright messages, but
   the parent set has additional tiles containing the '(c)Copyright 1995' which is shown
   on the title screen.

   The lack of these tiles in this set causes all subsequent tiles to be shifted.  It is
   likely that the correct program ROMs for this set either don't show '(c)Copyright 1995'
   or display it using the regular font instead. */

ROM_START( maddonnab )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 Code
	// program ROMs missing in this dump, gfx don't seem 100% correct for other ones
	ROM_LOAD16_BYTE( "maddonnb.b16", 0x00000, 0x20000, NO_DUMP )
	ROM_LOAD16_BYTE( "maddonnb.b15", 0x00001, 0x20000, NO_DUMP )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Z80 Code
	ROM_LOAD( "x13.ua2", 0x00000, 0x010000, CRC(f2080071) SHA1(68cbae9559879b2dc19c41a7efbd13ab4a569d3f) )

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD( "x5.16a",  0x000000, 0x080000, CRC(1aae0ad3) SHA1(a5afe699c66dcc5e7928807ae1c8be7ffdda798c) )
	ROM_LOAD( "x7.13a",  0x080000, 0x080000, CRC(39d13e25) SHA1(bfe8b187c7fc9dc1ac2cc3f840a686a25ec55340) )
	ROM_LOAD( "x9.11a",  0x100000, 0x080000, CRC(2027faeb) SHA1(cb8c697705ac70ec3cf74901a2becf6abd8be63d) )
	ROM_LOAD( "x11.08a", 0x180000, 0x080000, CRC(4afcfba6) SHA1(b3fff9217db2770e703bf8317a718aeee1e5c44d) )
	ROM_LOAD( "x6.16",   0x200000, 0x080000, CRC(7b893e78) SHA1(d38c5b159031976e7864021e59cc4fff61ffb53f) )
	ROM_LOAD( "x8.13",   0x280000, 0x080000, CRC(fed90a1f) SHA1(e2cff7ce24697308c50dadb0c042d87f3e46abdb) )
	ROM_LOAD( "x10.11",  0x300000, 0x080000, CRC(479d718c) SHA1(4fbc2568744cf78b15c6e0f3caba4d7109743cdd) )
	ROM_LOAD( "x12.08",  0x380000, 0x080000, CRC(d56ca9f8) SHA1(49bca5dbc048e7b7efa34e1c08ee1b76767ffe38) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00 )
	// no samples for this game

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "x1", 0x00000, 0x10000, CRC(6b213183) SHA1(599c59d155d11edb151bfaed1d24ef964462a447) ) // motherboard ROM, unknown purpose
ROM_END

} // anonymous namespace


// all sets have a large (unused) PROMAT logo in them
GAME( 1995, maddonna,  0,        maddonna, maddonna, maddonna_state, empty_init, ROT0, "Promat (Tuning license)",  "Mad Donna (Tuning, set 1)",    MACHINE_SUPPORTS_SAVE )
GAME( 1995, maddonnab, maddonna, maddonna, maddonna, maddonna_state, empty_init, ROT0, "Promat (Tuning license)",  "Mad Donna (Tuning, set 2)",    MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 1995, komocomo,  maddonna, komocomo, maddonna, maddonna_state, empty_init, ROT0, "bootleg",                  "Komo Como (Topmar, bootleg?)", MACHINE_SUPPORTS_SAVE ) // shows TOPMAR (an anagram of PROMAT) on the side, possibly a hack of the original PROMAT version

GAME( 1996, oneshot,   0,        oneshot,  oneshot , oneshot_state,  empty_init, ROT0, "Promat",                   "One Shot One Kill",            MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
