// license:BSD-3-Clause
// copyright-holders:Paul Hampson
/**********************************************************************************************************************
 Championship VBall
 Driver by Paul "TBBle" Hampson

 03/28/03 - Additions by Steve Ellenoff
 ---------------------------------------

 -Corrected background tiles (tiles are really 512x512 not 256x256 as previously setup)
 -Converted rendering to tilemap system
 -Implemented Scroll Y registers
 -Implemented X Line Scrolling (only seems to be used for displaying Hawaii and Airfield Map Screen)
 -Adjusted visible screen size to match more closely the real game
 -Added support for cocktail mode/flip screen
 -Confirmed the US version uses the oki6295 and does not display the story in attract mode like the JP version
 -Confirmed the Background graphics are contained in that unusual looking dip package on the US board


 Remaining Issues:
 -1) IRQ & NMI code is totally guessed, and needs to be solved properly

Measurements from Guru:
6502 /IRQ = 1.720kHz
6202 /NMI = 58 Hz
VBlank = 58Hz


 -2) X Line Scrolling doesn't work 100% when Flip Screen Dip is set
 -3) 2 Player Version - Dips for difficulty don't seem to work or just need more testing

 -4) 2 Player Version - sound ROM is different and the ADPCM chip is addressed differently
                        Changed it to use a ROM that was dumped from original PCB (readme below),
                        this makes the non-working ROM not used - I don't know where it came from.



  U.S. Championship V'Ball (Japan)
  Technos, 1988

  PCB Layout
  ----------


  TA-0025-P1-02 (M6100357A BEACH VOLLEY 880050B04)
  |---------------------------------------------------------------------|
  |          YM3014  M6295             25J1-0.47   YM2151   3.579545MHz |
  |                1.056MHz  25J0-0.78   Z80       6116                 |
  |                                                                     |
  |                                                                     |
  |                                                                     |
  |                                                                     |
  |    6502 25J2-2-5.124 6116                                           |
  |                                                                     |
  |                    2016                                     12MHz   |
  |J                                                                    |
  |A                                             2016  2016             |
  |M                                                                    |
  |M                                                                    |
  |A                                                                    |
  |  DSW1                              6264     25J4-0.35  25J3-0.5     |
  |  DSW2                                                               |
  |       25J6-0.144                                                    |
  |       25J5-0.143 2016                                               |
  |                       -------------------                           |
  |25J7-0.160             |                 |                           |
  |                       | TOSHIBA  0615   |                           |
  |                  2016 |                 |                           |
  |                       | T5324   TRJ-101 |                           |
  |                       |                 |                           |
  |-----------------------|-----------------|---------------------------|


  Notes:
        6502 clock: 2.000MHz
         Z80 clock: 3.579545MHz
      YM2151 clock: 3.579545MHz
       M6295 clock: 1.056MHz, sample rate = 8kHz (i.e. 1056000/132)


  *********************************************************************************************************************/

#include "emu.h"

#include "cpu/m6502/m6502.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "sound/okim6295.h"
#include "sound/ymopm.h"

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

class vball_state : public driver_device
{
public:
	vball_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_attribram(*this, "attribram"),
		m_videoram(*this, "videoram"),
		m_scrolly_lo(*this, "scrolly_lo"),
		m_spriteram(*this, "spriteram"),
		m_color_proms(*this, "color_proms"),
		m_mainbank(*this, "mainbank") { }

	void vball(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_attribram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_scrolly_lo;
	required_shared_ptr<uint8_t> m_spriteram;
	required_region_ptr<uint8_t> m_color_proms;
	required_memory_bank m_mainbank;

	uint16_t m_scrollx_hi = 0U;
	uint16_t m_scrolly_hi = 0U;
	uint8_t m_scrollx_lo = 0U;
	uint8_t m_gfxset = 0U;
	uint16_t m_scrollx[256]{};
	uint8_t m_bgprombank = 0U;
	uint8_t m_spprombank = 0U;
	tilemap_t *m_bg_tilemap = nullptr;

	void irq_ack_w(offs_t offset, uint8_t data);
	void bankswitch_w(uint8_t data);
	void scrollx_hi_w(uint8_t data);
	void scrollx_lo_w(uint8_t data);
	void videoram_w(offs_t offset, uint8_t data);
	void attrib_w(offs_t offset, uint8_t data);

	TILEMAP_MAPPER_MEMBER(background_scan);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(vball_scanline);
	void bgprombank_w(int bank);
	void spprombank_w(int bank);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	inline int scanline_to_vcount(int scanline);

	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


/***************************************************************************

  Video Hardware for Championship V'ball by Paul Hampson
  Generally copied from China Gate by Paul Hampson
  "Mainly copied from video of Double Dragon (bootleg) & Double Dragon II"

***************************************************************************/

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILEMAP_MAPPER_MEMBER(vball_state::background_scan)
{
	// logical (col, row) -> memory offset
	return (col & 0x1f) + ((row & 0x1f) << 5) + ((col & 0x20) << 5) + ((row & 0x20) << 6);
}

TILE_GET_INFO_MEMBER(vball_state::get_bg_tile_info)
{
	uint8_t code = m_videoram[tile_index];
	uint8_t attr = m_attribram[tile_index];
	tileinfo.set(0,
			code + ((attr & 0x1f) << 8) + (m_gfxset << 8),
			(attr >> 5) & 0x7,
			0);
}


void vball_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(vball_state::get_bg_tile_info)), tilemap_mapper_delegate(*this, FUNC(vball_state::background_scan)), 8, 8, 64, 64);

	m_bg_tilemap->set_scroll_rows(32);
	m_gfxset = 0;
	m_bgprombank = 0xff;
	m_spprombank = 0xff;

	save_item(NAME(m_scrollx_hi));
	save_item(NAME(m_scrolly_hi));
	save_item(NAME(m_scrollx_lo));
	save_item(NAME(m_gfxset));
	save_item(NAME(m_scrollx));
	save_item(NAME(m_bgprombank));
	save_item(NAME(m_spprombank));
}

void vball_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void vball_state::attrib_w(offs_t offset, uint8_t data)
{
	m_attribram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void vball_state::bgprombank_w(int bank)
{
	if (bank == m_bgprombank) return;

	uint8_t *color_prom = &m_color_proms[bank * 0x80];

	for (int i = 0; i < 128; i++, color_prom++)
		m_palette->set_pen_color(i, pal4bit(color_prom[0] >> 0), pal4bit(color_prom[0] >> 4), pal4bit(color_prom[0x800] >> 0));

	m_bgprombank = bank;
}

void vball_state::spprombank_w(int bank)
{
	if (bank == m_spprombank) return;

	uint8_t *color_prom = &m_color_proms[0x400 + bank * 0x80];

	for (int i = 128; i < 256; i++, color_prom++)
		m_palette->set_pen_color(i, pal4bit(color_prom[0] >> 0), pal4bit(color_prom[0] >> 4), pal4bit(color_prom[0x800] >> 0));

	m_spprombank = bank;
}


void vball_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	gfx_element *gfx = m_gfxdecode->gfx(1);

/*  240-Y    S|X|CLR|WCH WHICH    240-X
    xxxxxxxx x|x|xxx|xxx xxxxxxxx xxxxxxxx
*/
	for (int i = 0; i < m_spriteram.bytes(); i += 4)
	{
		int attr = m_spriteram[i + 1];
		int which = m_spriteram[i + 2]+ ((attr & 0x07) << 8);
		int sx = ((m_spriteram[i + 3] + 8) & 0xff) - 7;
		int sy = 240 - m_spriteram[i];
		int size = (attr & 0x80) >> 7;
		int color = (attr & 0x38) >> 3;
		int flipx = ~attr & 0x40;
		int flipy = 0;
		int dy = -16;

		if (flip_screen())
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
			dy = -dy;
		}

		switch (size)
		{
			case 0: // normal
			gfx->transpen(bitmap, cliprect, (which + 0), color, flipx, flipy, sx, sy, 0);
			break;

			case 1: // double y
			gfx->transpen(bitmap, cliprect, (which + 0), color, flipx, flipy, sx, sy + dy, 0);
			gfx->transpen(bitmap, cliprect, (which + 1), color, flipx, flipy, sx, sy, 0);
			break;
		}
	}
}

uint32_t vball_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrolly(0, m_scrolly_hi + *m_scrolly_lo);

	// To get linescrolling to work properly, we must ignore the 1st two scroll values, no idea why! -SJE
	for (int i = 2; i < 256; i++)
	{
		m_bg_tilemap->set_scrollx(i, m_scrollx[i - 2]);
		LOGSCROLL("scrollx[%d] = %d\n", i, m_scrollx[i]);
	}
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}


// Based on ddragon driver
inline int vball_state::scanline_to_vcount(int scanline)
{
	int vcount = scanline + 8;
	if (vcount < 0x100)
		return vcount;
	else
		return (vcount - 0x18) | 0x100;
}

TIMER_DEVICE_CALLBACK_MEMBER(vball_state::vball_scanline)
{
	int scanline = param;
	int screen_height = m_screen->height();
	int vcount_old = scanline_to_vcount((scanline == 0) ? screen_height - 1 : scanline - 1);
	int vcount = scanline_to_vcount(scanline);

	// Update to the current point
	if (scanline > 0)
	{
		m_screen->update_partial(scanline - 1);
	}

	// IRQ fires every on every 8th scanline
	if (!(vcount_old & 8) && (vcount & 8))
	{
		m_maincpu->set_input_line(M6502_IRQ_LINE, ASSERT_LINE);
	}

	// NMI fires on scanline 248 (VBL) and is latched
	if (vcount == 0xf8)
	{
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	}

	// Save the scroll x register value
	if (scanline < 256)
	{
		m_scrollx[255 - scanline] = (m_scrollx_hi + m_scrollx_lo + 4);
	}
}

void vball_state::irq_ack_w(offs_t offset, uint8_t data)
{
	if (offset == 0)
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);

	else
		m_maincpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
}


/* bit 0 = bank switch
   bit 1 = ?
   bit 2 = ?
   bit 3 = ?
   bit 4 = ?
   bit 5 = graphics tile offset
   bit 6 = scroll y hi
   bit 7 = ?
*/
void vball_state::bankswitch_w(uint8_t data)
{
	m_mainbank->set_entry(data & 1);

	if (m_gfxset != ((data & 0x20) ^ 0x20))
	{
		m_gfxset = (data & 0x20) ^ 0x20;
			m_bg_tilemap->mark_all_dirty();
	}
	m_scrolly_hi = (data & 0x40) << 2;
}


/* bit 0 = flip screen
   bit 1 = scrollx hi
   bit 2 = bg prom bank
   bit 3 = bg prom bank
   bit 4 = bg prom bank
   bit 5 = sp prom bank
   bit 6 = sp prom bank
   bit 7 = sp prom bank
*/
void vball_state::scrollx_hi_w(uint8_t data)
{
	flip_screen_set(~data & 1);
	m_scrollx_hi = (data & 0x02) << 7;
	bgprombank_w((data >> 2) & 0x07);
	spprombank_w((data >> 5) & 0x07);
	LOGSCROLL("%04x: scrollx_hi = %d\n", m_maincpu->pcbase(), m_scrollx_hi);
}

void vball_state::scrollx_lo_w(uint8_t data)
{
	m_scrollx_lo = data;
	LOGSCROLL("%04x: scrollx_lo = %d\n", m_maincpu->pcbase(), m_scrollx_lo);
}


// Cheaters note: Scores are stored in RAM @ 0x57-0x58 (though the space is used for other things between matches)
void vball_state::main_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x0800, 0x08ff).ram().share(m_spriteram);
	map(0x1000, 0x1000).portr("P1");
	map(0x1001, 0x1001).portr("P2");
	map(0x1002, 0x1002).portr("SYSTEM");
	map(0x1003, 0x1003).portr("DSW1");
	map(0x1004, 0x1004).portr("DSW2");
	map(0x1005, 0x1005).portr("P3");
	map(0x1006, 0x1006).portr("P4");
	map(0x1008, 0x1008).w(FUNC(vball_state::scrollx_hi_w));
	map(0x1009, 0x1009).w(FUNC(vball_state::bankswitch_w));
	map(0x100a, 0x100b).w(FUNC(vball_state::irq_ack_w));  // is there a scanline counter here?
	map(0x100c, 0x100c).w(FUNC(vball_state::scrollx_lo_w));
	map(0x100d, 0x100d).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x100e, 0x100e).writeonly().share(m_scrolly_lo);
	map(0x2000, 0x2fff).w(FUNC(vball_state::videoram_w)).share(m_videoram);
	map(0x3000, 0x3fff).w(FUNC(vball_state::attrib_w)).share(m_attribram);
	map(0x4000, 0x7fff).bankr(m_mainbank);
	map(0x8000, 0xffff).rom();
}

void vball_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0x8800, 0x8801).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x9800, 0x9803).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xa000, 0xa000).r("soundlatch", FUNC(generic_latch_8_device::read));
}


static INPUT_PORTS_START( vball )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:1,2") // Verified against Taito's US Vball manual
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x0c, 0x00, "Single Player Game Time")    PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, "1:15")
	PORT_DIPSETTING(    0x04, "1:30")
	PORT_DIPSETTING(    0x0c, "1:45")
	PORT_DIPSETTING(    0x08, "2:00")
	PORT_DIPNAME( 0x30, 0x00, "Start Buttons (4-player)")   PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, "Button A")
	PORT_DIPSETTING(    0x10, "Button B")
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPNAME( 0x40, 0x40, "PL 1&4 (4-player)")      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Rotate 90")
	PORT_DIPNAME( 0x80, 0x00, "Player Mode")        PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, "2 Players")
	PORT_DIPSETTING(    0x00, "4 Players")

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("P3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START3 )

	PORT_START("P4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START4 )
INPUT_PORTS_END

static INPUT_PORTS_START (vball2pj)
	PORT_INCLUDE( vball )

	// The 2-player ROMs have the game-time in the difficulty spot, and I've assumed vice-versa. (VS the instructions scanned in Naz's dump)

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x03, 0x00, "Single Player Game Time")    PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "1:30")
	PORT_DIPSETTING(    0x01, "1:45")
	PORT_DIPSETTING(    0x03, "2:00")
	PORT_DIPSETTING(    0x02, "2:15")
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:3,4") // Difficulty order needs to be verified
	PORT_DIPSETTING(    0x04, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Very_Hard ) )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW1:5" ) // Dips 5 through 8 are used for 4 player mode, not supported in 2 player set
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW1:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW1:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW1:8" )

	PORT_MODIFY("P3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED ) // Used in 4 player mode, not supported in 2 player set

	PORT_MODIFY("P4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED ) // Used in 4 player mode, not supported in 2 player set
INPUT_PORTS_END

void vball_state::machine_start()
{
	m_mainbank->configure_entries(0, 2, memregion("maincpu")->base(), 0x4000);
}


static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 2, 4, 6 },
	{ 0*8*8+1, 0*8*8+0, 1*8*8+1, 1*8*8+0, 2*8*8+1, 2*8*8+0, 3*8*8+1, 3*8*8+0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	32*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4, 0, 4 },
	{ 3, 2, 1, 0, 16*8+3, 16*8+2, 16*8+1, 16*8+0,
			32*8+3, 32*8+2, 32*8+1, 32*8+0, 48*8+3, 48*8+2, 48*8+1, 48*8+0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	64*8
};


static GFXDECODE_START( gfx_vb )
	GFXDECODE_ENTRY( "fg_tiles", 0, charlayout,    0, 8 )  // 8x8 chars
	GFXDECODE_ENTRY( "sprites", 0, spritelayout, 128, 8 )  // 16x16 sprites
GFXDECODE_END


void vball_state::vball(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, 12_MHz_XTAL / 6);   // 2 MHz - measured by Guru but it makes the game far far too slow ?!
	m_maincpu->set_addrmap(AS_PROGRAM, &vball_state::main_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(vball_state::vball_scanline), "screen", 0, 1);

	Z80(config, m_audiocpu, 3.579545_MHz_XTAL);
	m_audiocpu->set_addrmap(AS_PROGRAM, &vball_state::sound_map);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_size(32*8, 32*8);
	m_screen->set_raw(12_MHz_XTAL / 2, 384, 0, 256, 272, 8, 248);   // based on ddragon driver
	m_screen->set_screen_update(FUNC(vball_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_vb);
	PALETTE(config, m_palette).set_entries(256);

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	// The sound system comes all but verbatim from Double Dragon
	GENERIC_LATCH_8(config, "soundlatch").data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	ym2151_device &ymsnd(YM2151(config, "ymsnd", 3.579545_MHz_XTAL));
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "lspeaker", 0.60);
	ymsnd.add_route(1, "rspeaker", 0.60);

	okim6295_device &oki(OKIM6295(config, "oki", 1.056_MHz_XTAL, okim6295_device::PIN7_HIGH));
	oki.add_route(ALL_OUTPUTS, "lspeaker", 1.0);
	oki.add_route(ALL_OUTPUTS, "rspeaker", 1.0);
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( vball ) // US version
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "25a2-4.124",   0x00000, 0x10000, CRC(be04c2b5) SHA1(40fed4ae272719e940f1796ef35420ab451ab7b6) ) // First 0x8000 banked, second 0x8000 fixed

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "25j1-0.47",    0x00000, 0x8000,  CRC(10ca79ad) SHA1(aad4a09d6745ca0b5665cb00ff7a4e08ea434068) )

	/* the original has the image data stored in a special ceramic embedded package made by Toshiba
	with part number 'TOSHIBA TRJ-101' (which has been dumped using a custom made adapter)
	there are a few bytes different between the bootleg and the original (the original is correct though!) */
	ROM_REGION(0x80000, "fg_tiles", 0 )
	ROM_LOAD( "trj-101.96",   0x00000, 0x80000, CRC(f343eee4) SHA1(1ce95285631f7ec91fe3f6c3d62b13f565d3816a) )

	ROM_REGION(0x40000, "sprites", 0 )
	ROM_LOAD( "25j4-0.35",    0x00000, 0x20000, CRC(877826d8) SHA1(fd77298f9343051f66259dad9127f40afb95f385) ) // 0,1,2,3
	ROM_LOAD( "25j3-0.5",     0x20000, 0x20000, CRC(c6afb4fa) SHA1(6d7c966300ce5fb2094476b393434486965d62b4) ) // 0,1,2,3

	ROM_REGION(0x40000, "oki", 0 )
	ROM_LOAD( "25j0-0.78",    0x00000, 0x20000, CRC(8e04bdbf) SHA1(baafc5033c9442b83cb332c2c453c13117b31a3b) )

	ROM_REGION(0x1000, "color_proms", 0 )
	ROM_LOAD_NIB_LOW ( "25j5-0.144",   0x0000,  0x00800, CRC(a317240f) SHA1(bd57ad516f7a8ff774276fd26b02dd34659d41ad) )
	ROM_LOAD_NIB_HIGH( "25j6-0.143",   0x0000,  0x00800, CRC(1ff70b4f) SHA1(a469baa0dda844ba307c09ddefb23f239cfe7b5f) )
	ROM_LOAD(          "25j7-0.160",   0x0800,  0x00800, CRC(2ffb68b3) SHA1(d560fdcd5e5c79d37e5b5bde22fbaf662fe89252) )
ROM_END

ROM_START( vball2pj ) // Japan version
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "25j2-2-5.124", 0x00000, 0x10000,  CRC(432509c4) SHA1(6de50e21d279f4ac9674bc91990ba9535e80908c) ) // First 0x8000 banked, second 0x8000 fixed

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "25j1-0.47",    0x00000, 0x8000,  CRC(10ca79ad) SHA1(aad4a09d6745ca0b5665cb00ff7a4e08ea434068) )

	/* the original has the image data stored in a special ceramic embedded package made by Toshiba
	with part number 'TOSHIBA TRJ-101' (which has been dumped using a custom made adapter)
	there are a few bytes different between the bootleg and the original (the original is correct though!) */
	ROM_REGION(0x80000, "fg_tiles", 0 )
	ROM_LOAD( "trj-101.96",   0x00000, 0x80000, CRC(f343eee4) SHA1(1ce95285631f7ec91fe3f6c3d62b13f565d3816a) )

	ROM_REGION(0x40000, "sprites", 0 )
	ROM_LOAD( "25j4-0.35",    0x00000, 0x20000, CRC(877826d8) SHA1(fd77298f9343051f66259dad9127f40afb95f385) ) // 0,1,2,3
	ROM_LOAD( "25j3-0.5",     0x20000, 0x20000, CRC(c6afb4fa) SHA1(6d7c966300ce5fb2094476b393434486965d62b4) ) // 0,1,2,3

	ROM_REGION(0x40000, "oki", 0 )
	ROM_LOAD( "25j0-0.78",    0x00000, 0x20000, CRC(8e04bdbf) SHA1(baafc5033c9442b83cb332c2c453c13117b31a3b) )

	ROM_REGION(0x1000, "color_proms", 0 )
	ROM_LOAD_NIB_LOW ( "25j5-0.144",   0x0000,  0x00800, CRC(a317240f) SHA1(bd57ad516f7a8ff774276fd26b02dd34659d41ad) )
	ROM_LOAD_NIB_HIGH( "25j6-0.143",   0x0000,  0x00800, CRC(1ff70b4f) SHA1(a469baa0dda844ba307c09ddefb23f239cfe7b5f) )
	ROM_LOAD(          "25j7-0.160",   0x0800,  0x00800, CRC(2ffb68b3) SHA1(d560fdcd5e5c79d37e5b5bde22fbaf662fe89252) )
ROM_END

ROM_START( vballb ) // bootleg
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "vball.124",    0x00000, 0x10000, CRC(be04c2b5) SHA1(40fed4ae272719e940f1796ef35420ab451ab7b6) ) // First 0x8000 banked, second 0x8000 fixed

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "25j1-0.47",    0x00000, 0x8000,  CRC(10ca79ad) SHA1(aad4a09d6745ca0b5665cb00ff7a4e08ea434068) )

	// The bootlegs used standard ROMs on a daughter card that plugs into the socket for the TOSHIBA TRJ-101 DIP ROM
	ROM_REGION(0x80000, "fg_tiles", 0 )
	ROM_LOAD( "13", 0x00000, 0x10000, CRC(f26df8e1) SHA1(72186c1430d07c7fd9211245b539f05a0660bebe) ) // 0,1,2,3
	ROM_LOAD( "14", 0x10000, 0x10000, CRC(c9798d0e) SHA1(ec156f6c7ecccaa216ce8076f75ad7627ee90945) ) // 0,1,2,3
	ROM_LOAD( "15", 0x20000, 0x10000, CRC(68e69c4b) SHA1(9870674c91cab7215ad8ed40eb82facdee478fde) ) // 0,1,2,3
	ROM_LOAD( "16", 0x30000, 0x10000, CRC(936457ba) SHA1(1662bbd777fcd33a298d192a3f06681809b9d049) ) // 0,1,2,3
	ROM_LOAD( "9",  0x40000, 0x10000, CRC(42874924) SHA1(a75eed7934e089f035000b7f35f6ba8dd96f1e98) ) // 0,1,2,3
	ROM_LOAD( "10", 0x50000, 0x10000, CRC(6cc676ee) SHA1(6e8c590946211baa9266b19b871f252829057696) ) // 0,1,2,3
	ROM_LOAD( "11", 0x60000, 0x10000, CRC(4754b303) SHA1(8630f077b542590ef1340a2f0a6b94086ff91c40) ) // 0,1,2,3
	ROM_LOAD( "12", 0x70000, 0x10000, CRC(21294a84) SHA1(b36ea9ddf6879443d3104241997fa0f916856528) ) // 0,1,2,3

	ROM_REGION(0x40000, "sprites", 0 )
	ROM_LOAD( "vball.35",     0x00000, 0x20000, CRC(877826d8) SHA1(fd77298f9343051f66259dad9127f40afb95f385) ) // 0,1,2,3 == 25j4-0.35
	ROM_LOAD( "vball.5",      0x20000, 0x20000, CRC(c6afb4fa) SHA1(6d7c966300ce5fb2094476b393434486965d62b4) ) // 0,1,2,3 == 25j3-0.5

	ROM_REGION(0x40000, "oki", 0 )
	ROM_LOAD( "vball.78a",    0x00000, 0x10000, CRC(f3e63b76) SHA1(da54d1d7d7d55b73e49991e4363bc6f46e0f70eb) ) // == 1st half of 25j0-0.78
	ROM_LOAD( "vball.78b",    0x10000, 0x10000, CRC(7ad9d338) SHA1(3e3c270fa69bda93b03f07a54145eb5e211ec8ba) ) // == 2nd half of 25j0-0.78

	ROM_REGION(0x1000, "color_proms", 0 )
	ROM_LOAD_NIB_LOW ( "25j5-0.144",   0x0000,  0x00800, CRC(a317240f) SHA1(bd57ad516f7a8ff774276fd26b02dd34659d41ad) )
	ROM_LOAD_NIB_HIGH( "25j6-0.143",   0x0000,  0x00800, CRC(1ff70b4f) SHA1(a469baa0dda844ba307c09ddefb23f239cfe7b5f) )
	ROM_LOAD(          "25j7-0.160",   0x0800,  0x00800, CRC(2ffb68b3) SHA1(d560fdcd5e5c79d37e5b5bde22fbaf662fe89252) )
ROM_END

ROM_START( vball2pjb ) // bootleg of the Japan set with unmodified program ROM
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.124", 0x00000, 0x10000, CRC(432509c4) SHA1(6de50e21d279f4ac9674bc91990ba9535e80908c) )// First 0x8000 banked, second 0x8000 fixed  == 25j2-2-5.124 from vball2pj

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "4.ic47", 0x00000, 0x8000,  CRC(534dfbd9) SHA1(d0cb37caf94fa85da4ebdfe15e7a78109084bf91) )

	// The bootlegs used standard ROMs on a daughter card that plugs into the socket for the TOSHIBA TRJ-101 DIP ROM
	ROM_REGION(0x80000, "fg_tiles", 0 )
	ROM_LOAD( "13", 0x00000, 0x10000, CRC(f26df8e1) SHA1(72186c1430d07c7fd9211245b539f05a0660bebe) ) // 0,1,2,3
	ROM_LOAD( "14", 0x10000, 0x10000, CRC(c9798d0e) SHA1(ec156f6c7ecccaa216ce8076f75ad7627ee90945) ) // 0,1,2,3
	ROM_LOAD( "15", 0x20000, 0x10000, CRC(68e69c4b) SHA1(9870674c91cab7215ad8ed40eb82facdee478fde) ) // 0,1,2,3
	ROM_LOAD( "16", 0x30000, 0x10000, CRC(936457ba) SHA1(1662bbd777fcd33a298d192a3f06681809b9d049) ) // 0,1,2,3
	ROM_LOAD( "9",  0x40000, 0x10000, CRC(42874924) SHA1(a75eed7934e089f035000b7f35f6ba8dd96f1e98) ) // 0,1,2,3
	ROM_LOAD( "10", 0x50000, 0x10000, CRC(6cc676ee) SHA1(6e8c590946211baa9266b19b871f252829057696) ) // 0,1,2,3
	ROM_LOAD( "11", 0x60000, 0x10000, CRC(4754b303) SHA1(8630f077b542590ef1340a2f0a6b94086ff91c40) ) // 0,1,2,3
	ROM_LOAD( "12", 0x70000, 0x10000, CRC(21294a84) SHA1(b36ea9ddf6879443d3104241997fa0f916856528) ) // 0,1,2,3

	ROM_REGION(0x40000, "sprites", 0 )
	ROM_LOAD( "8", 0x00000, 0x10000, CRC(b18d083c) SHA1(8c7a39b8a9c79a13682a4f283470801c3cbb748c) ) // == 1st half of 25j4-0.35
	ROM_LOAD( "7", 0x10000, 0x10000, CRC(79a35321) SHA1(0953730b1baa9bda4b2eb703258476423e5448f5) ) // == 2nd half of 25j4-0.35
	ROM_LOAD( "6", 0x20000, 0x10000, CRC(49c6aad7) SHA1(6c026ddd97a5dfd138fb65781504f192c11ee6aa) ) // == 1st half of 25j3-0.5
	ROM_LOAD( "5", 0x30000, 0x10000, CRC(9bb95651) SHA1(ec8a481cc7f0d6e469489db7c51103446910ae80) ) // == 2nd half of 25j3-0.5

	ROM_REGION(0x40000, "oki", 0 )
	ROM_LOAD( "vball.78a", 0x00000, 0x10000, CRC(f3e63b76) SHA1(da54d1d7d7d55b73e49991e4363bc6f46e0f70eb) ) // == 1st half of 25j0-0.78    (ROM type 27512)
	ROM_LOAD( "3.ic79",    0x10000, 0x08000, CRC(d77349ba) SHA1(5ef25636056607fae7a5463957487b53da0dd310) ) // == 3rd quarter of 25j0-0.78 (ROM type 27256)

	ROM_REGION(0x1000, "color_proms", 0 )
	ROM_LOAD_NIB_LOW ( "25j5-0.144",   0x0000,  0x00800, CRC(a317240f) SHA1(bd57ad516f7a8ff774276fd26b02dd34659d41ad) )
	ROM_LOAD_NIB_HIGH( "25j6-0.143",   0x0000,  0x00800, CRC(1ff70b4f) SHA1(a469baa0dda844ba307c09ddefb23f239cfe7b5f) )
	ROM_LOAD(          "25j7-0.160",   0x0800,  0x00800, CRC(2ffb68b3) SHA1(d560fdcd5e5c79d37e5b5bde22fbaf662fe89252) )
ROM_END

} // anonymous namespace


GAME( 1988, vball,    0,     vball,    vball,    vball_state, empty_init, ROT0, "Technos Japan", "U.S. Championship V'ball (US)",                   MACHINE_SUPPORTS_SAVE )
GAME( 1988, vball2pj, vball, vball,    vball2pj, vball_state, empty_init, ROT0, "Technos Japan", "U.S. Championship V'ball (Japan)",                MACHINE_SUPPORTS_SAVE )
GAME( 1988, vballb,   vball, vball,    vball,    vball_state, empty_init, ROT0, "bootleg",       "U.S. Championship V'ball (bootleg of US set)",    MACHINE_SUPPORTS_SAVE )
GAME( 1988, vball2pjb,vball, vball,    vball,    vball_state, empty_init, ROT0, "bootleg",       "U.S. Championship V'ball (bootleg of Japan set)", MACHINE_SUPPORTS_SAVE )
