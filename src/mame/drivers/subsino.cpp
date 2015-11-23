// license:BSD-3-Clause
// copyright-holders:Luca Elia, David Haywood, Angelo Salese, Roberto Fresca
/***************************************************************************

              -= Subsino's Gambling Games =-

            Driver by   Luca Elia (l.elia@tin.it)
   Additional work by   David Haywood, Angelo Salese and Roberto Fresca.


  CPU: Black epoxy box containing:
       1x CXK58257M (32768 x 8-Bit) High Speed CMOS SRAM.
       1x HD647180X0P6, HD643180X0P6 or HD641180X0P6 Z180-CPU.
       1x X-TAL (unknown frequency).
       1x Battery.

  GFX:    1 Tilemap (8x8 tiles, no scrolling)
  CUSTOM: 2 x SUBSINO SS9100, SUBSINO SS9101
  SOUND:  M6295, YM2413 or YM3812
  OTHER:  Battery

To Do:

- Remove ROM patches from smoto, stbsub and tesorone, emulate the protection instead.
- Hopper emulation currently hooked up in stbsub, tesorone and smoto. Add to others.

****************************************************************************

  Game Notes:
  ----------


  * Victor5, Victor 21, Shark Party.

  To enter the bookkeeping mode press STATS (key 9).
  Press START/DEAL (key 2) to exit.


  * Treasure Island.

  To enter test mode, keep STATS/TEST (key 9) pressed during boot.
  HOLD3 (key C) to increase the tune/sound number, START (key 2) to decrease it.

  To enter the bookkeeping mode press STATS/TEST (key 9).
  Press START/TAKE (key 2) to exit.


  * Poker Carnival (crsbingo)

  To adjust the game rate, press SETTINGS (key 0) and then use HOLD4 (key V)
  to choose the individual game to set. Press HOLD5 (key B) to change value.
  Press START/DEAL (key 2) to exit.

  To enter the bookkeeping mode press STATS (key 9).
  Press START/DEAL (key 2) to exit.


  * Super Rider, Super Moto

  To enter test mode, keep STATS/TEST (key 9) pressed during boot.
  HOLD5 (key B) to select between VOICE <-> MUSIC.
  HOLD4 (key V) to increase the tune/sound number, START (key 2) to decrease it.

  To enter the bookkeeping mode press STATS/TEST (key 9).
  Press START/DEAL (key 2) to exit.


****************************************************************************

  Updates:

  2010-01-01 / 2010-01-16
  -----------------------

  (general):

  - Lowered CPU clock to 1.5 MHz.
  - Reworked former inputs.
  - Added specific game and technical notes.
  - Added lamps support.
  - Cleaned-up a bit the driver.

  Victor 5:

  - Mapped the M6295 but commented out due to missing sample roms.
  - Added keyin, keyout, payout, stats and settings inputs.
  - Added coin/keyin/keyout counters.
  - Limited the bet and coin pulses to avoid repeats and coin jams.
  - Added complete coinage and keyin DIP switches.
  - Added main game and double-up rates DIP switches.
  - Added minimum bet DIP switches.
  - Added maximum bet DIP switches.
  - Added attract music DIP switch.
  - Added button-lamps layout.

  Victor 21:

  - Mapped the M6295 but commented out due to missing sample roms.
  - Added bet x10, keyin, keyout, stats and settings inputs.
  - Added coin/keyin/keyout/payout counters.
  - Limited the bet and coin pulses to avoid repeats and coin jams.
  - Added complete coinage and keyin DIP switches.
  - Added main game rate DIP switches.
  - Added minimum bet DIP switches.
  - Added attract music DIP switch.
  - Added button-lamps layout.

  Poker Carnival (crsbingo):

  - Added change card, keyin, keyout, stats and settings inputs.
  - Added coin/keyin/keyout/payout counters.
  - Limited the bet and coin pulses to avoid repeats and coin jams.
  - Added complete coinage and keyin DIP switches.
  - Added double-up rate DIP switches.
  - Added minimum bet DIP switches.
  - Added maximum bet DIP switches.
  - Added cards graphics DIP switches.
  - Added double-up type DIP switches.
  - Added button-lamps layout.

  Super Rider, Super Moto:

  - Added stats and settings inputs.
  - Added coin counters.
  - Added main game and double-up rates DIP switches.
  - Added double-up and control type DIP switches.
  - Added coinage and demo sounds DIP switches.
  - Added button-lamps layout.

  Shark Party:

  - Added stats and settings inputs.
  - Added coin counters.
  - Added main game and double-up rates DIP switches.
  - Added double-up, coinage and demo sounds DIP switches.
  - Added button-lamps layout.


  2010-02-03
  ----------

  Treasure Island:

  - Added proper inputs.
  - Added coin/keyin/keyout/payout counters.
  - Limited the bet and coin pulses to avoid repeats and coin jams.
  - Added complete coinage and keyin DIP switches.
  - Added main game and double-up rates DIP switches.
  - Added minimum bet DIP switches.
  - Added maximum bet DIP switches.
  - Added main game and double-up limit DIP switches.
  - Added payout mode and auto take DIP switches.
  - Added DIP locations as seen in the settings mode.
  - Added demo sounds DIP switch.
  - Created proper button-lamps layout.
  - Added technical notes.
  - Some clean-ups...


  2010-04-20
  ----------

  Shark Party (English, Alpha license):

  - Created complete inputs from the scratch.
  - Added coin/keyin/keyout counters.
  - Added main game and double-up rates DIP switches.
  - Added minimum bet DIP switches.
  - Added maximum bet DIP switches.
  - Added complete coinage and remote credits DIP switches.
  - Added jokers and demo sounds DIP switches.
  - Figured out and documented all the game outputs.
  - Created proper button-lamps layout.

  Now the game is in full-working state.


  2010-04-22
  ----------

  Treasure Bonus (Subsino)

  - Reworked and cleaned-up the inputs.
     Most buttons have more than one single function.
  - Added DIP locations.
  - Figured out the following DIP Switches:
     Complete Coinage (1/2/5/10/20/25/50/100).
     Remote Credits (1/2/5/10/20/25/50/100).
     Minimum Bet (1/8/16/32).
     Max Bet (16/32/64/80).
     Demo Sounds (Off/On).
     Game Limit (10000/20000/30000/60000).
     Double-Up (No/Yes).
     Win Rate (84/86/88/90/92/94/96%).
     Double-Up Level (0/1/2/3/4/5/6/7).
     Double-Up Game (Dancers / Panties Colors / Cards / Seven-Bingo).

  - Fixed inverted functions and buggy inputs.
  - Added lamps support.
  - Created button-lamps layout.
  - Remapped inputs to reflect the controls layout. This way is more
     user-friendly since controls are straight with button-lamps.
  - Added coin/keyin/keyout/payout counters.
  - Removed the pulse limitation in the BET input. This allow it to work
     as BET and STOP2 properly.
  - Added technical notes.


  2010-10-12
  ----------

  - Added Victor 6 (3 sets).
  - Created proper inputs for all sets.



***************************************************************************/

#include "emu.h"
#include "cpu/z180/z180.h"
#include "machine/i8255.h"
#include "machine/subsino.h"
#include "machine/ticket.h"
#include "sound/okim6295.h"
#include "sound/2413intf.h"
#include "sound/3812intf.h"

#include "victor5.lh"
#include "victor21.lh"
#include "crsbingo.lh"
#include "sharkpy.lh"
#include "sharkpye.lh"
#include "smoto.lh"
#include "tisub.lh"
#include "stisub.lh"

class subsino_state : public driver_device
{
public:
	subsino_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_reel3_scroll(*this, "reel3_scroll"),
		m_reel2_scroll(*this, "reel2_scroll"),
		m_reel1_scroll(*this, "reel1_scroll"),
		m_reel1_ram(*this, "reel1_ram"),
		m_reel2_ram(*this, "reel2_ram"),
		m_reel3_ram(*this, "reel3_ram"),
		m_stbsub_out_c(*this, "stbsub_out_c"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") {
	}

	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_videoram;
	optional_shared_ptr<UINT8> m_reel3_scroll;
	optional_shared_ptr<UINT8> m_reel2_scroll;
	optional_shared_ptr<UINT8> m_reel1_scroll;
	optional_shared_ptr<UINT8> m_reel1_ram;
	optional_shared_ptr<UINT8> m_reel2_ram;
	optional_shared_ptr<UINT8> m_reel3_ram;
	optional_shared_ptr<UINT8> m_stbsub_out_c;

	tilemap_t *m_tmap;
	tilemap_t *m_reel1_tilemap;
	tilemap_t *m_reel2_tilemap;
	tilemap_t *m_reel3_tilemap;
	int m_tiles_offset;
	UINT8 m_out_c;
	UINT8 *m_reel1_attr;
	UINT8 *m_reel2_attr;
	UINT8 *m_reel3_attr;
	UINT8 m_flash_val;
	UINT8 m_flash_packet;
	UINT8 m_flash_packet_start;
	int m_colordac_offs;
	UINT8 *m_stbsub_colorram;

	ticket_dispenser_device *m_hopper;

	DECLARE_WRITE8_MEMBER(subsino_tiles_offset_w);
	DECLARE_WRITE8_MEMBER(subsino_videoram_w);
	DECLARE_WRITE8_MEMBER(subsino_colorram_w);
	DECLARE_WRITE8_MEMBER(subsino_reel1_ram_w);
	DECLARE_WRITE8_MEMBER(subsino_reel2_ram_w);
	DECLARE_WRITE8_MEMBER(subsino_reel3_ram_w);
	DECLARE_WRITE8_MEMBER(subsino_out_a_w);
	DECLARE_WRITE8_MEMBER(subsino_out_b_w);
	DECLARE_READ8_MEMBER(flash_r);
	DECLARE_WRITE8_MEMBER(flash_w);
	DECLARE_READ8_MEMBER(hwcheck_r);
	DECLARE_WRITE8_MEMBER(subsino_out_c_w);
	DECLARE_WRITE8_MEMBER(colordac_w);
	DECLARE_WRITE8_MEMBER(reel_scrollattr_w);
	DECLARE_READ8_MEMBER(reel_scrollattr_r);
	DECLARE_DRIVER_INIT(stbsub);
	DECLARE_DRIVER_INIT(stisub);
	DECLARE_DRIVER_INIT(tesorone);
	DECLARE_DRIVER_INIT(tesorone230);
	DECLARE_DRIVER_INIT(smoto20);
	DECLARE_DRIVER_INIT(sharkpy);
	DECLARE_DRIVER_INIT(smoto16);
	DECLARE_DRIVER_INIT(crsbingo);
	DECLARE_DRIVER_INIT(victor21);
	DECLARE_DRIVER_INIT(victor5);
	DECLARE_DRIVER_INIT(tisuba);
	DECLARE_DRIVER_INIT(sharkpye);
	DECLARE_DRIVER_INIT(tisub);
	DECLARE_DRIVER_INIT(mtrainnv);
	TILE_GET_INFO_MEMBER(get_tile_info);
	TILE_GET_INFO_MEMBER(get_stbsub_tile_info);
	TILE_GET_INFO_MEMBER(get_subsino_reel1_tile_info);
	TILE_GET_INFO_MEMBER(get_stbsub_reel1_tile_info);
	TILE_GET_INFO_MEMBER(get_subsino_reel2_tile_info);
	TILE_GET_INFO_MEMBER(get_stbsub_reel2_tile_info);
	TILE_GET_INFO_MEMBER(get_subsino_reel3_tile_info);
	TILE_GET_INFO_MEMBER(get_stbsub_reel3_tile_info);
	DECLARE_VIDEO_START(subsino);
	DECLARE_PALETTE_INIT(subsino_2proms);
	DECLARE_PALETTE_INIT(subsino_3proms);
	DECLARE_VIDEO_START(subsino_reels);
	DECLARE_VIDEO_START(stbsub);
	UINT32 screen_update_subsino(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_subsino_reels(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_stbsub_reels(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	virtual void machine_start();
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};

void subsino_state::machine_start()
{
	m_hopper = machine().device<ticket_dispenser_device>("hopper");
}

/***************************************************************************
*                              Video Hardware                              *
***************************************************************************/


WRITE8_MEMBER(subsino_state::subsino_tiles_offset_w)
{
	m_tiles_offset = (data & 1) ? 0x1000: 0;
	m_tmap->mark_tile_dirty(offset);
//  popmessage("gfx %02x",data);
}

WRITE8_MEMBER(subsino_state::subsino_videoram_w)
{
	m_videoram[offset] = data;
	m_tmap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(subsino_state::subsino_colorram_w)
{
	m_colorram[offset] = data;
	m_tmap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(subsino_state::get_tile_info)
{
	UINT16 code = m_videoram[ tile_index ] + (m_colorram[ tile_index ] << 8);
	UINT16 color = (code >> 8) & 0x0f;
	code = ((code & 0xf000) >> 4) + ((code & 0xff) >> 0);
	code += m_tiles_offset;
	SET_TILE_INFO_MEMBER(0, code, color, 0);
}

TILE_GET_INFO_MEMBER(subsino_state::get_stbsub_tile_info)
{
	UINT16 code = m_videoram[ tile_index ] + (m_colorram[ tile_index ] << 8);
	code&= 0x3fff;
	SET_TILE_INFO_MEMBER(0, code, 0, 0);
}


VIDEO_START_MEMBER(subsino_state,subsino)
{
	m_tmap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(subsino_state::get_tile_info),this), TILEMAP_SCAN_ROWS, 8,8, 0x40,0x20 );
	m_tmap->set_transparent_pen(0 );
	m_tiles_offset = 0;
}



WRITE8_MEMBER(subsino_state::subsino_reel1_ram_w)
{
	m_reel1_ram[offset] = data;
	m_reel1_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(subsino_state::get_subsino_reel1_tile_info)
{
	int code = m_reel1_ram[tile_index];
	int colour = (m_out_c&0x7) + 8;

	SET_TILE_INFO_MEMBER(1,
			code,
			colour,
			0);
}

TILE_GET_INFO_MEMBER(subsino_state::get_stbsub_reel1_tile_info)
{
	int code = m_reel1_ram[tile_index];
	int attr = m_reel1_attr[tile_index];

	SET_TILE_INFO_MEMBER(1,
			code | (attr << 8),
			0,
			0);
}


WRITE8_MEMBER(subsino_state::subsino_reel2_ram_w)
{
	m_reel2_ram[offset] = data;
	m_reel2_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(subsino_state::get_subsino_reel2_tile_info)
{
	int code = m_reel2_ram[tile_index];
	int colour = (m_out_c&0x7) + 8;

	SET_TILE_INFO_MEMBER(1,
			code,
			colour,
			0);
}

TILE_GET_INFO_MEMBER(subsino_state::get_stbsub_reel2_tile_info)
{
	int code = m_reel2_ram[tile_index];
	int attr = m_reel2_attr[tile_index];

	SET_TILE_INFO_MEMBER(1,
			code | (attr << 8),
			0,
			0);
}

WRITE8_MEMBER(subsino_state::subsino_reel3_ram_w)
{
	m_reel3_ram[offset] = data;
	m_reel3_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(subsino_state::get_subsino_reel3_tile_info)
{
	int code = m_reel3_ram[tile_index];
	int colour = (m_out_c&0x7) + 8;

	SET_TILE_INFO_MEMBER(1,
			code,
			colour,
			0);
}

TILE_GET_INFO_MEMBER(subsino_state::get_stbsub_reel3_tile_info)
{
	int code = m_reel3_ram[tile_index];
	int attr = m_reel3_attr[tile_index];

	SET_TILE_INFO_MEMBER(1,
			code | (attr << 8),
			0,
			0);
}


VIDEO_START_MEMBER(subsino_state,subsino_reels)
{
	VIDEO_START_CALL_MEMBER( subsino );

	m_reel1_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(subsino_state::get_subsino_reel1_tile_info),this),TILEMAP_SCAN_ROWS, 8, 32, 64, 8);
	m_reel2_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(subsino_state::get_subsino_reel2_tile_info),this),TILEMAP_SCAN_ROWS, 8, 32, 64, 8);
	m_reel3_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(subsino_state::get_subsino_reel3_tile_info),this),TILEMAP_SCAN_ROWS, 8, 32, 64, 8);

	m_reel1_tilemap->set_scroll_cols(64);
	m_reel2_tilemap->set_scroll_cols(64);
	m_reel3_tilemap->set_scroll_cols(64);

}

VIDEO_START_MEMBER(subsino_state,stbsub)
{
	m_tmap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(subsino_state::get_stbsub_tile_info),this), TILEMAP_SCAN_ROWS, 8,8, 0x40,0x20 );
	m_tmap->set_transparent_pen(0 );

	m_reel1_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(subsino_state::get_stbsub_reel1_tile_info),this),TILEMAP_SCAN_ROWS, 8, 32, 64, 8);
	m_reel2_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(subsino_state::get_stbsub_reel2_tile_info),this),TILEMAP_SCAN_ROWS, 8, 32, 64, 8);
	m_reel3_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(subsino_state::get_stbsub_reel3_tile_info),this),TILEMAP_SCAN_ROWS, 8, 32, 64, 8);

	m_reel1_tilemap->set_scroll_cols(64);
	m_reel2_tilemap->set_scroll_cols(64);
	m_reel3_tilemap->set_scroll_cols(64);

	m_out_c = 0x08;
}


UINT32 subsino_state::screen_update_subsino(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	m_tmap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

UINT32 subsino_state::screen_update_subsino_reels(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i;
	bitmap.fill(0, cliprect);

	for (i= 0;i < 64;i++)
	{
		m_reel1_tilemap->set_scrolly(i, m_reel1_scroll[i]);
		m_reel2_tilemap->set_scrolly(i, m_reel2_scroll[i]);
		m_reel3_tilemap->set_scrolly(i, m_reel3_scroll[i]);
	}

	if (m_out_c&0x08)
	{
		// are these hardcoded, or registers?
		const rectangle visible1(0*8, (14+48)*8-1,  4*8,  (4+7)*8-1);
		const rectangle visible2(0*8, (14+48)*8-1, 10*8, (10+7)*8-1);
		const rectangle visible3(0*8, (14+48)*8-1, 18*8, (18+7)*8-1);

		m_reel1_tilemap->draw(screen, bitmap, visible1, 0, 0);
		m_reel2_tilemap->draw(screen, bitmap, visible2, 0, 0);
		m_reel3_tilemap->draw(screen, bitmap, visible3, 0, 0);
	}

	m_tmap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


UINT32 subsino_state::screen_update_stbsub_reels(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i;
	bitmap.fill(0, cliprect);

	if (m_reel1_attr)
	{
		m_reel1_tilemap->mark_all_dirty();
		m_reel2_tilemap->mark_all_dirty();
		m_reel3_tilemap->mark_all_dirty();
	}

	for (i= 0;i < 64;i++)
	{
		m_reel1_tilemap->set_scrolly(i, m_reel1_scroll[i]);
		m_reel2_tilemap->set_scrolly(i, m_reel2_scroll[i]);
		m_reel3_tilemap->set_scrolly(i, m_reel3_scroll[i]);
	}

	if (m_out_c&0x08)
	{
		// areas based on d-up game in attract mode
		const rectangle visible1(0, 511,  0,  87);
		const rectangle visible2(0, 511,  88, 143);
		const rectangle visible3(0, 511,  144, 223);

		m_reel1_tilemap->draw(screen, bitmap, visible1, 0, 0);
		m_reel2_tilemap->draw(screen, bitmap, visible2, 0, 0);
		m_reel3_tilemap->draw(screen, bitmap, visible3, 0, 0);
	}

	m_tmap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}



PALETTE_INIT_MEMBER(subsino_state,subsino_2proms)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i,r,g,b,val;
	int bit0,bit1,bit2;

	for (i = 0; i < 256; i++)
	{
		val = (color_prom[i+0x100]) | (color_prom[i+0x000]<<4);

		bit0 = 0;
		bit1 = (val >> 6) & 0x01;
		bit2 = (val >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (val >> 3) & 0x01;
		bit1 = (val >> 4) & 0x01;
		bit2 = (val >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (val >> 0) & 0x01;
		bit1 = (val >> 1) & 0x01;
		bit2 = (val >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

PALETTE_INIT_MEMBER(subsino_state,subsino_3proms)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i,r,g,b,val;
	int bit0,bit1,bit2;

	for (i = 0; i < 256; i++)
	{
		val = (color_prom[i+0x000]) | (color_prom[i+0x100]<<3) | (color_prom[i+0x200]<<6);

		bit0 = 0;
		bit1 = (val >> 7) & 0x01;
		bit2 = (val >> 6) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (val >> 5) & 0x01;
		bit1 = (val >> 4) & 0x01;
		bit2 = (val >> 3) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (val >> 2) & 0x01;
		bit1 = (val >> 1) & 0x01;
		bit2 = (val >> 0) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}


/***************************************************************************
*                          Lamps & other outputs.                          *
***************************************************************************/

WRITE8_MEMBER(subsino_state::subsino_out_a_w)
{
/***** COIN PULSE: *****


  * Victor 5, Victor 21, Poker Carnival (crsbingo),
    Shark Party (English, Alpha license), Treasure Bonus (Subsino).

  7654 3210
  ---- ---x  Coin pulse.
  ---- --x-  Key In pulse.
  ---x ----  Key Out pulse.
  --x- ----  Payout pulse.


  * Treasure Island.

  7654 3210
  ---- ---x  Key In pulse.
  ---- --x-  Coin pulse.
  ---x ----  Key Out pulse.
  --x- ----  Payout pulse.


  * Super Rider, Super Moto, Shark Party.

  7654 3210
  ---- --x-  Coin pulse.

*/

	output_set_lamp_value(8, (data) & 1);       /* Lamp 8 */
	output_set_lamp_value(9, (data >> 1) & 1);  /* Lamp 9 */
	output_set_lamp_value(10, (data >> 2) & 1); /* Lamp 10 */
	output_set_lamp_value(11, (data >> 3) & 1); /* Lamp 11 */
	output_set_lamp_value(12, (data >> 4) & 1); /* Lamp 12 */
	output_set_lamp_value(13, (data >> 5) & 1); /* Lamp 13 */
	output_set_lamp_value(14, (data >> 6) & 1); /* Lamp 14 */
	output_set_lamp_value(15, (data >> 7) & 1); /* Lamp 15 */

	coin_counter_w( machine(), 0, data & 0x01 );    /* coin / keyin */
	coin_counter_w( machine(), 1, data & 0x02 );    /* keyin / coin */
	coin_counter_w( machine(), 2, data & 0x10 );    /* keyout */
	coin_counter_w( machine(), 3, data & 0x20 );    /* payout */

	m_hopper->write(space, 0, (data & 0x0020) ? 0x80 : 0);   // hopper motor

//  popmessage("Out A %02x",data);
}

WRITE8_MEMBER(subsino_state::subsino_out_b_w)
{
/***** LAMPS: *****

  * Victor 5.

  7654 3210
  ---- ---x  HOLD2.
  ---- --x-  BET.
  ---- -x--  ???.
  ---- x---  START / TAKE.
  ---x ----  HOLD1.
  --x- ----  HOLD5 / SMALL.
  -x-- ----  HOLD3 / HALF TAKE.
  x--- ----  HOLD4 / BIG.


  * Victor 21.

  7654 3210
  ---- ---x  DOUBLE (or SPLIT?).
  ---- --x-  BET.
  ---- -x--  SPLIT (or DOUBLE?).
  ---- x---  STAND.
  ---x ----  ???.
  --x- ----  DEAL / HIT.
  -x-- ----  ???.
  x--- ----  GAME OVER?.


  * Treasure Island.

  7654 3210
  ---- ---x  HOLD1.
  ---- --x-  HOLD2 / BIG.
  ---- -x--  ???.
  ---- x---  BET.
  ---x ----  DOUBLE / INFO.
  --x- ----  START / TAKE.
  -x-- ----  HOLD3 / SMALL.
  x--- ----  ???.


  * Poker Carnival (crsbingo).

  7654 3210
  ---- ---x  HOLD2 / DOUBLE.
  ---- --x-  BET.
  ---- -x--  ???.
  ---- x---  START / TAKE.
  ---x ----  HOLD1.
  --x- ----  HOLD5 / SMALL.
  -x-- ----  HOLD3 / CHANGE.
  x--- ----  HOLD4 / BIG.


  * Super Rider, Super Moto.

  7654 3210
  ---- ---x  HOLD1 / SELECT.
  ---- --x-  HOLD2.
  ---- -x--  ???.
  ---- x---  BET.
  ---x ----  HOLD3.
  --x- ----  START / TAKE.
  -x-- ----  HOLD4.
  x--- ----  HOLD5. Also TEST button (bookkeeping) lamp is routed here.


  * Shark Party.

  7654 3210
  ---- ---x  HOLD3.
  ---- --x-  HOLD4 / BIG.
  ---- -x--  ???.
  ---- x---  BET.
  ---x ----  HOLD1 / DOUBLE.
  --x- ----  START / TAKE.
  -x-- ----  HOLD2.
  x--- ----  HOLD5 / SMALL.


  * Shark Party (English, Alpha license).

  7654 3210
  ---- ---x  ???.
  ---- --x-  HOLD3 / D-UP.
  ---- -x--  START.
  ---- x---  HOLD1 / BET.
  ---x ----  ???.
  --x- ----  HOLD2 / TAKE.
  -x-- ----  ???.
  x--- ----  HOLD4 & HOLD5.


  * Treasure Bonus (Subsino).

  7654 3210
  ---- ---x  SMALL / RED / STOP3.
  ---- --x-  D-UP / INFO.
  ---- -x--  START / STOP ALL.
  ---- x---  BET / STOP2.
  ---x ----  ???.
  --x- ----  TAKE / STOP1.
  -x-- ----  ???.
  x--- ----  BIG / BLACK.


  After seeing the button/lamps functions, I think that controls layout should be
  arranged the following way:

   ________    ________    ________    ________    ________       __________
  |        |  |        |  |        |  |        |  |        |     |          |
  |  D-UP  |  |  TAKE  |  |  BET   |  | SMALL  |  |  BIG   |     |  START   |
  |  INFO  |  | STOP 1 |  | STOP 2 |  | STOP 3 |  |        |     | STOP ALL |
  |________|  |________|  |________|  |________|  |________|     |__________|

   Key 'Z'     Key 'X'     Key 'C'     Key 'V'     Key 'B'        Key 'N'

  Mapped to the above keys to be more user-friendly.

*/

	output_set_lamp_value(0, (data) & 1);       /* Lamp 0 */
	output_set_lamp_value(1, (data >> 1) & 1);  /* Lamp 1 */
	output_set_lamp_value(2, (data >> 2) & 1);  /* Lamp 2 */
	output_set_lamp_value(3, (data >> 3) & 1);  /* Lamp 3 */
	output_set_lamp_value(4, (data >> 4) & 1);  /* Lamp 4 */
	output_set_lamp_value(5, (data >> 5) & 1);  /* Lamp 5 */
	output_set_lamp_value(6, (data >> 6) & 1);  /* Lamp 6 */
	output_set_lamp_value(7, (data >> 7) & 1);  /* Lamp 7 */

//  popmessage("Out B %02x",data);
}


/***************************************************************************
*                              Memory Maps                                 *
***************************************************************************/

static ADDRESS_MAP_START( srider_map, AS_PROGRAM, 8, subsino_state )
	AM_RANGE( 0x00000, 0x0bfff ) AM_ROM

	AM_RANGE( 0x0c000, 0x0cfff ) AM_RAM

	AM_RANGE( 0x0d000, 0x0d000 ) AM_READ_PORT( "SW1" )
	AM_RANGE( 0x0d001, 0x0d001 ) AM_READ_PORT( "SW2" )
	AM_RANGE( 0x0d002, 0x0d002 ) AM_READ_PORT( "SW3" )

	AM_RANGE( 0x0d004, 0x0d004 ) AM_READ_PORT( "SW4" )
	AM_RANGE( 0x0d005, 0x0d005 ) AM_READ_PORT( "INA" )
	AM_RANGE( 0x0d006, 0x0d006 ) AM_READ_PORT( "INB" )

	AM_RANGE( 0x0d009, 0x0d009 ) AM_WRITE(subsino_out_b_w )
	AM_RANGE( 0x0d00a, 0x0d00a ) AM_WRITE(subsino_out_a_w )

	AM_RANGE( 0x0d00c, 0x0d00c ) AM_READ_PORT( "INC" )

	AM_RANGE( 0x0d016, 0x0d017 ) AM_DEVWRITE("ymsnd", ym3812_device, write)

	AM_RANGE( 0x0d018, 0x0d018 ) AM_DEVWRITE("oki", okim6295_device, write)

	AM_RANGE( 0x0d01b, 0x0d01b ) AM_WRITE(subsino_tiles_offset_w )

	AM_RANGE( 0x0e000, 0x0e7ff ) AM_RAM_WRITE(subsino_colorram_w ) AM_SHARE("colorram")
	AM_RANGE( 0x0e800, 0x0efff ) AM_RAM_WRITE(subsino_videoram_w ) AM_SHARE("videoram")
ADDRESS_MAP_END


static ADDRESS_MAP_START( sharkpy_map, AS_PROGRAM, 8, subsino_state )
	AM_RANGE( 0x09800, 0x09fff ) AM_RAM

	AM_RANGE( 0x09000, 0x09000 ) AM_READ_PORT( "SW1" )
	AM_RANGE( 0x09001, 0x09001 ) AM_READ_PORT( "SW2" )
	AM_RANGE( 0x09002, 0x09002 ) AM_READ_PORT( "SW3" )

	AM_RANGE( 0x09004, 0x09004 ) AM_READ_PORT( "SW4" )
	AM_RANGE( 0x09005, 0x09005 ) AM_READ_PORT( "INA" )
	AM_RANGE( 0x09006, 0x09006 ) AM_READ_PORT( "INB" )

	AM_RANGE( 0x09009, 0x09009 ) AM_WRITE(subsino_out_b_w )
	AM_RANGE( 0x0900a, 0x0900a ) AM_WRITE(subsino_out_a_w )

	AM_RANGE( 0x0900c, 0x0900c ) AM_READ_PORT( "INC" )

	AM_RANGE( 0x09016, 0x09017 ) AM_DEVWRITE("ymsnd", ym3812_device, write)

	AM_RANGE( 0x09018, 0x09018 ) AM_DEVWRITE("oki", okim6295_device, write)

	AM_RANGE( 0x0901b, 0x0901b ) AM_WRITE(subsino_tiles_offset_w )

	AM_RANGE( 0x07800, 0x07fff ) AM_RAM
	AM_RANGE( 0x08000, 0x087ff ) AM_RAM_WRITE(subsino_colorram_w ) AM_SHARE("colorram")
	AM_RANGE( 0x08800, 0x08fff ) AM_RAM_WRITE(subsino_videoram_w ) AM_SHARE("videoram")

	AM_RANGE( 0x00000, 0x13fff ) AM_ROM //overlap unmapped regions
ADDRESS_MAP_END

/*
Victor 21 protection is absolutely trivial. At every number of presetted hands, there's an animation
that announces to the player that the card deck changes. If the protection check fails (at start-up),
this event makes the game to reset without any money in the bank.
*/

static ADDRESS_MAP_START( victor21_map, AS_PROGRAM, 8, subsino_state )
	AM_RANGE( 0x09800, 0x09fff ) AM_RAM

	AM_RANGE( 0x09000, 0x09000 ) AM_WRITE(subsino_out_a_w )
	AM_RANGE( 0x09001, 0x09001 ) AM_WRITE(subsino_out_b_w )
	AM_RANGE( 0x09002, 0x09002 ) AM_READ_PORT( "INC" )
	AM_RANGE( 0x09004, 0x09004 ) AM_READ_PORT( "INA" )
	AM_RANGE( 0x09005, 0x09005 ) AM_READ_PORT( "INB" )

	AM_RANGE( 0x09006, 0x09006 ) AM_READ_PORT( "SW1" )
	AM_RANGE( 0x09007, 0x09007 ) AM_READ_PORT( "SW2" )
	AM_RANGE( 0x09008, 0x09008 ) AM_READ_PORT( "SW3" )

	AM_RANGE( 0x0900b, 0x0900b ) AM_RAM //protection

//  AM_RANGE( 0x0900c, 0x0900c ) AM_DEVWRITE("oki", okim6295_device, write)

	AM_RANGE( 0x0900e, 0x0900f ) AM_DEVWRITE("ymsnd", ym2413_device, write)

	AM_RANGE( 0x0900d, 0x0900d ) AM_WRITE(subsino_tiles_offset_w )

	AM_RANGE( 0x07800, 0x07fff ) AM_RAM
	AM_RANGE( 0x08000, 0x087ff ) AM_RAM_WRITE(subsino_videoram_w ) AM_SHARE("videoram")
	AM_RANGE( 0x08800, 0x08fff ) AM_RAM_WRITE(subsino_colorram_w ) AM_SHARE("colorram")

	AM_RANGE( 0x00000, 0x08fff ) AM_ROM //overlap unmapped regions
	AM_RANGE( 0x10000, 0x13fff ) AM_ROM
ADDRESS_MAP_END


/*
Victor 5 has a protection device that calculates the combinations of the screen.
0x00 = you lose
0x01 = royal flush
0x02 = 5 of a kind
 ...
0x0a = jacks of better

0xd9 is asked at the POST, game refuses to run otherwise.

The fun thing is that the main CPU actually calculates the combinations then it routes to this port
thru packet writes, 0xd* starts the packet, 0xe* ends the packet.
I'm fairly sure that this thing can do a lot more, the POST check makes me think that 0xd9 is actually a result of a RNG.
For now we'll emulate the bare minimum to let this game to work, obviously we need tests/tracing on the real board to understand
what it is exactly and what it can possibly do.
*/


READ8_MEMBER(subsino_state::flash_r)
{
//  printf("R %02x\n",m_flash_val);

	if(m_flash_val == 0xff)
		return 0xd9;
	else if(m_flash_val <= 0xa)
		return m_flash_val;
	else if(m_flash_val == 0x92)
		return 0xeb; //protected GFXs in Cross Bingo
	else
		return 0xd9;
}

WRITE8_MEMBER(subsino_state::flash_w)
{
	switch(m_flash_packet_start)
	{
		case 0:
			m_flash_packet = data;
			if((m_flash_packet & 0xf8) == 0xd0)
				m_flash_packet_start = 1; //start of packet
			break;
		case 1:
			m_flash_packet = data;
			if((m_flash_packet & 0xf8) == 0xe0)
				m_flash_packet_start = 0; //end of packet
			else
				m_flash_val = data;
			break;
	}
}

static ADDRESS_MAP_START( victor5_map, AS_PROGRAM, 8, subsino_state )
	AM_IMPORT_FROM( victor21_map )
	AM_RANGE( 0x0900a, 0x0900a ) AM_READWRITE(flash_r, flash_w )
	AM_RANGE( 0x0900b, 0x0900b ) AM_READNOP //"flash" status, bit 0
ADDRESS_MAP_END


READ8_MEMBER(subsino_state::hwcheck_r)
{
	/* Wants this at POST otherwise an "Hardware Error" occurs. */
	return 0x55;
}

static ADDRESS_MAP_START( crsbingo_map, AS_PROGRAM, 8, subsino_state )
	AM_RANGE( 0x09800, 0x09fff ) AM_RAM

	AM_RANGE( 0x09000, 0x09000 ) AM_READ_PORT( "SW1" )
	AM_RANGE( 0x09001, 0x09001 ) AM_READ_PORT( "SW2" )
	AM_RANGE( 0x09002, 0x09002 ) AM_READ_PORT( "INA" )
	AM_RANGE( 0x09003, 0x09003 ) AM_READ_PORT( "INB" )
	AM_RANGE( 0x09004, 0x09004 ) AM_READ_PORT( "INC" )
	AM_RANGE( 0x09005, 0x09005 ) AM_WRITE(subsino_out_a_w )

	AM_RANGE( 0x09008, 0x09008 ) AM_READ_PORT( "SW4" )
	AM_RANGE( 0x09009, 0x09009 ) AM_READ_PORT( "SW3" )  // AM_WRITE(subsino_out_a_w )
	AM_RANGE( 0x0900a, 0x0900a ) AM_READWRITE(hwcheck_r, subsino_out_b_w )

	AM_RANGE( 0x09010, 0x09010 ) AM_READWRITE(flash_r, flash_w )
//  AM_RANGE( 0x09011, 0x09011 ) //"flash" status, bit 0
//  AM_RANGE( 0x0900c, 0x0900c ) AM_READ_PORT( "INC" )
	AM_RANGE( 0x0900c, 0x0900d ) AM_DEVWRITE("ymsnd", ym2413_device, write)

//  AM_RANGE( 0x09018, 0x09018 ) AM_DEVWRITE("oki", okim6295_device, write)

//  AM_RANGE( 0x0900d, 0x0900d ) AM_WRITE(subsino_tiles_offset_w )

	AM_RANGE( 0x07800, 0x07fff ) AM_RAM
	AM_RANGE( 0x08000, 0x087ff ) AM_RAM_WRITE(subsino_videoram_w ) AM_SHARE("videoram")
	AM_RANGE( 0x08800, 0x08fff ) AM_RAM_WRITE(subsino_colorram_w ) AM_SHARE("colorram")

	AM_RANGE( 0x00000, 0x8fff ) AM_ROM //overlap unmapped regions

	AM_RANGE( 0x10000, 0x13fff ) AM_ROM //overlap unmapped regions

ADDRESS_MAP_END

WRITE8_MEMBER(subsino_state::subsino_out_c_w)
{
	// not 100% sure on this

	// ???? eccc
	// e = enable reels?
	// c = reel colour bank?
	m_out_c = data;

	m_reel1_tilemap->mark_all_dirty();
	m_reel2_tilemap->mark_all_dirty();
	m_reel3_tilemap->mark_all_dirty();
//  popmessage("data %02x\n",data);
}

static ADDRESS_MAP_START( tisub_map, AS_PROGRAM, 8, subsino_state )
	AM_RANGE( 0x09800, 0x09fff ) AM_RAM

	AM_RANGE( 0x09000, 0x09000 ) AM_READ_PORT( "SW1" )
	AM_RANGE( 0x09001, 0x09001 ) AM_READ_PORT( "SW2" )
	AM_RANGE( 0x09002, 0x09002 ) AM_READ_PORT( "SW3" )

	AM_RANGE( 0x09004, 0x09004 ) AM_READ_PORT( "SW4" )
	AM_RANGE( 0x09005, 0x09005 ) AM_READ_PORT( "INA" )
	AM_RANGE( 0x09006, 0x09006 ) AM_READ_PORT( "INB" )

	/* 0x09008: is marked as OUTPUT C in the test mode. */
	AM_RANGE( 0x09008, 0x09008 ) AM_WRITE(subsino_out_c_w )
	AM_RANGE( 0x09009, 0x09009 ) AM_WRITE(subsino_out_b_w )
	AM_RANGE( 0x0900a, 0x0900a ) AM_WRITE(subsino_out_a_w )

	AM_RANGE( 0x0900c, 0x0900c ) AM_READ_PORT( "INC" )

	AM_RANGE( 0x09016, 0x09017 ) AM_DEVWRITE("ymsnd", ym3812_device, write)

//  AM_RANGE( 0x0900c, 0x0900c ) AM_DEVWRITE("oki", okim6295_device, write)

	AM_RANGE( 0x0901b, 0x0901b ) AM_WRITE(subsino_tiles_offset_w )

	AM_RANGE( 0x07800, 0x07fff ) AM_RAM
	AM_RANGE( 0x08800, 0x08fff ) AM_RAM_WRITE(subsino_videoram_w ) AM_SHARE("videoram")
	AM_RANGE( 0x08000, 0x087ff ) AM_RAM_WRITE(subsino_colorram_w ) AM_SHARE("colorram")

	AM_RANGE( 0x00000, 0x0bfff ) AM_ROM // overlap unmapped regions
	AM_RANGE( 0x10000, 0x13fff ) AM_ROM
	AM_RANGE( 0x14000, 0x14fff ) AM_ROM // reads the card face data here (see rom copy in rom loading)

	AM_RANGE( 0x150c0, 0x150ff ) AM_RAM AM_SHARE("reel3_scroll")
	AM_RANGE( 0x15140, 0x1517f ) AM_RAM AM_SHARE("reel2_scroll")
	AM_RANGE( 0x15180, 0x151bf ) AM_RAM AM_SHARE("reel1_scroll")

	AM_RANGE( 0x15800, 0x159ff ) AM_RAM_WRITE(subsino_reel1_ram_w) AM_SHARE("reel1_ram")
	AM_RANGE( 0x15a00, 0x15bff ) AM_RAM_WRITE(subsino_reel2_ram_w) AM_SHARE("reel2_ram")
	AM_RANGE( 0x15c00, 0x15dff ) AM_RAM_WRITE(subsino_reel3_ram_w) AM_SHARE("reel3_ram")
ADDRESS_MAP_END


WRITE8_MEMBER(subsino_state::colordac_w)
{
	switch ( offset )
	{
		case 0:
			m_colordac_offs = data * 3;
			break;

		case 1:
			m_stbsub_colorram[m_colordac_offs] = data;
			m_palette->set_pen_color(m_colordac_offs/3,
				pal6bit(m_stbsub_colorram[(m_colordac_offs/3)*3+0]),
				pal6bit(m_stbsub_colorram[(m_colordac_offs/3)*3+1]),
				pal6bit(m_stbsub_colorram[(m_colordac_offs/3)*3+2])
			);
			m_colordac_offs = (m_colordac_offs+1) % (256*3);
			break;

		case 2:
			// ff?
			break;

		case 3:
			break;
	}
}


// this stuff is banked..
// not 100% sure on the bank bits.. other bits are also set
WRITE8_MEMBER(subsino_state::reel_scrollattr_w)
{
	if (*m_stbsub_out_c & 0x20)
	{
		if (offset<0x200)
		{
			m_reel1_attr[offset&0x1ff] = data;
		}
		else if (offset<0x400)
		{
			m_reel2_attr[offset&0x1ff] = data;
		}
		else if (offset<0x600)
		{
			m_reel3_attr[offset&0x1ff] = data;
		}
		else
		{
			// ??
		}
	}
	else
	{
		offset &=0xff;

		if (offset<0x40)
		{
			// ??
		}
		else if (offset<0x80)
		{
			m_reel2_scroll[offset&0x3f] = data;
		}
		else if (offset<0xc0)
		{
			m_reel1_scroll[offset&0x3f] = data;
		}
		else
		{
			m_reel3_scroll[offset&0x3f] = data;
		}
	}
}

READ8_MEMBER(subsino_state::reel_scrollattr_r)
{
	return m_reel1_attr[offset];
}

static ADDRESS_MAP_START( stbsub_map, AS_PROGRAM, 8, subsino_state )
	AM_RANGE( 0x00000, 0x0bfff ) AM_ROM

	AM_RANGE( 0x0c000, 0x0cfff ) AM_RAM

	AM_RANGE( 0x0d000, 0x0d000 ) AM_READ_PORT( "SW1" )
	AM_RANGE( 0x0d001, 0x0d001 ) AM_READ_PORT( "SW2" )
	AM_RANGE( 0x0d002, 0x0d002 ) AM_READ_PORT( "SW3" )

	AM_RANGE( 0x0d004, 0x0d004 ) AM_READ_PORT( "SW4" )
	AM_RANGE( 0x0d005, 0x0d005 ) AM_READ_PORT( "INB" )
	AM_RANGE( 0x0d006, 0x0d006 ) AM_READ_PORT( "INA" )

	AM_RANGE( 0x0d008, 0x0d008 ) AM_RAM AM_SHARE("stbsub_out_c")

	AM_RANGE( 0x0d009, 0x0d009 ) AM_WRITE(subsino_out_b_w )
	AM_RANGE( 0x0d00a, 0x0d00a ) AM_WRITE(subsino_out_a_w )

	AM_RANGE( 0x0d00c, 0x0d00c ) AM_READ_PORT( "INC" )

	AM_RANGE( 0x0d010, 0x0d013 ) AM_WRITE(colordac_w)

	AM_RANGE( 0x0d016, 0x0d017 ) AM_DEVWRITE("ymsnd", ym3812_device, write)

//  AM_RANGE( 0x0d01b, 0x0d01b ) AM_WRITE(subsino_tiles_offset_w )

	AM_RANGE( 0x0e000, 0x0e7ff ) AM_RAM_WRITE(subsino_colorram_w ) AM_SHARE("colorram")
	AM_RANGE( 0x0e800, 0x0efff ) AM_RAM_WRITE(subsino_videoram_w ) AM_SHARE("videoram")

	AM_RANGE( 0xf000, 0xf7ff ) AM_READWRITE(reel_scrollattr_r, reel_scrollattr_w)

	AM_RANGE( 0xf800, 0xf9ff ) AM_RAM_WRITE(subsino_reel1_ram_w) AM_SHARE("reel1_ram")
	AM_RANGE( 0xfa00, 0xfbff ) AM_RAM_WRITE(subsino_reel2_ram_w) AM_SHARE("reel2_ram")
	AM_RANGE( 0xfc00, 0xfdff ) AM_RAM_WRITE(subsino_reel3_ram_w) AM_SHARE("reel3_ram")
ADDRESS_MAP_END


/***************************************************************************
                        Magic Train (Clear NVRAM ROM?)
***************************************************************************/

static ADDRESS_MAP_START( mtrainnv_map, AS_PROGRAM, 8, subsino_state )
	AM_RANGE( 0x00000, 0x0bfff ) AM_ROM

	AM_RANGE( 0x0c000, 0x0cfff ) AM_RAM

	AM_RANGE( 0x0d000, 0x0d000 ) AM_READ_PORT( "SW1" )
	AM_RANGE( 0x0d001, 0x0d001 ) AM_READ_PORT( "SW2" )
	AM_RANGE( 0x0d002, 0x0d002 ) AM_READ_PORT( "SW3" )

	AM_RANGE( 0x0d004, 0x0d004 ) AM_READ_PORT( "SW4" )
	AM_RANGE( 0x0d005, 0x0d005 ) AM_READ_PORT( "INB" )
	AM_RANGE( 0x0d006, 0x0d006 ) AM_READ_PORT( "INA" )
	AM_RANGE( 0x0d008, 0x0d008 ) AM_RAM AM_SHARE("stbsub_out_c")
//  AM_RANGE( 0x0d009, 0x0d009 ) AM_WRITE
//  AM_RANGE( 0x0d00a, 0x0d00a ) AM_WRITE
//  AM_RANGE( 0x0d00b, 0x0d00b ) AM_WRITE
	AM_RANGE( 0x0d00c, 0x0d00c ) AM_READ_PORT( "INC" )

	AM_RANGE( 0x0d010, 0x0d013 ) AM_WRITE(colordac_w)

//  AM_RANGE( 0x0d012, 0x0d012 ) AM_WRITE

	AM_RANGE( 0x0d016, 0x0d017 ) AM_DEVWRITE("ymsnd", ym3812_device, write)

//  AM_RANGE( 0x0d018, 0x0d018 ) AM_DEVREADWRITE("oki", okim6295_device, read, write)

	AM_RANGE( 0x0e000, 0x0e7ff ) AM_RAM_WRITE(subsino_colorram_w ) AM_SHARE("colorram")
	AM_RANGE( 0x0e800, 0x0efff ) AM_RAM_WRITE(subsino_videoram_w ) AM_SHARE("videoram")

	AM_RANGE( 0xf000, 0xf7ff ) AM_READWRITE(reel_scrollattr_r, reel_scrollattr_w)

	AM_RANGE( 0xf800, 0xf9ff ) AM_RAM_WRITE(subsino_reel1_ram_w) AM_SHARE("reel1_ram")
	AM_RANGE( 0xfa00, 0xfbff ) AM_RAM_WRITE(subsino_reel2_ram_w) AM_SHARE("reel2_ram")
	AM_RANGE( 0xfc00, 0xfdff ) AM_RAM_WRITE(subsino_reel3_ram_w) AM_SHARE("reel3_ram")
ADDRESS_MAP_END


static ADDRESS_MAP_START( subsino_iomap, AS_IO, 8, subsino_state )
	AM_RANGE( 0x0000, 0x003f ) AM_RAM // internal regs
ADDRESS_MAP_END


/***************************************************************************
*                              Input Ports                                 *
***************************************************************************/

static INPUT_PORTS_START( victor21 )

	PORT_START( "SW1" )
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x07, "1 Coin / 10 Credits" )
	PORT_DIPSETTING(    0x03, "1 Coin / 20 Credits" )
	PORT_DIPSETTING(    0x02, "1 Coin / 25 Credits" )
	PORT_DIPSETTING(    0x01, "1 Coin / 50 Credits" )
	PORT_DIPSETTING(    0x00, "1 Coin / 100 Credits" )
	PORT_DIPNAME( 0x08, 0x08, "Key In" )
	PORT_DIPSETTING(    0x08, "100 Points/Pulse" )
	PORT_DIPSETTING(    0x00, "200 Points/Pulse" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START( "SW2" )
	PORT_DIPNAME( 0x03, 0x03, "Main Game Rate" )
	PORT_DIPSETTING(    0x00, "75%" )
	PORT_DIPSETTING(    0x01, "80%" )
	PORT_DIPSETTING(    0x03, "85%" )
	PORT_DIPSETTING(    0x02, "90%" )
	PORT_DIPNAME( 0x04, 0x04, "Max Rate" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, "Minimum Bet" )
	PORT_DIPSETTING(    0x10, "1" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x18, "10" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x20, 0x20, "Attract Music" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "SW2 - 40" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START( "SW3" )
	PORT_DIPNAME( 0x01, 0x01, "SW3" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START( "INA" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )        PORT_CODE(KEYCODE_V)    PORT_NAME("Split")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )    PORT_CODE(KEYCODE_Z)    PORT_NAME("Deal / Hit")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_BET )      PORT_IMPULSE(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_STAND )   PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_CODE(KEYCODE_A)    PORT_NAME("Bet x10")    // multibet
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )    PORT_CODE(KEYCODE_C)

	PORT_START( "INB" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )          PORT_IMPULSE(3) // coin 1
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )   // key in
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )          PORT_IMPULSE(3) // coin 2
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN3 )          PORT_IMPULSE(3) // coin 3
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_CODE(KEYCODE_9)    PORT_NAME("Stats")  // Bookkeeping.
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_CODE(KEYCODE_0)    PORT_NAME("Settings")   // Game Rate.
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )  // no payout?
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )  // key out

	PORT_START( "INC" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_CODE(KEYCODE_R)    PORT_NAME("Reset")  // hard reset
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( victor5 )

	PORT_START( "SW1" )
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x07, "1 Coin / 10 Credits" )
	PORT_DIPSETTING(    0x03, "1 Coin / 20 Credits" )
	PORT_DIPSETTING(    0x02, "1 Coin / 25 Credits" )
	PORT_DIPSETTING(    0x01, "1 Coin / 50 Credits" )
	PORT_DIPSETTING(    0x00, "1 Coin / 100 Credits" )
	PORT_DIPNAME( 0x08, 0x08, "Key In" )
	PORT_DIPSETTING(    0x08, "100 Points/Pulse" )
	PORT_DIPSETTING(    0x00, "500 Points/Pulse" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Max Bet" )
	PORT_DIPSETTING(    0x80, "20" )
	PORT_DIPSETTING(    0x40, "40" )
	PORT_DIPSETTING(    0xc0, "50" )
	PORT_DIPSETTING(    0x00, "80" )

	PORT_START( "SW2" )
	PORT_DIPNAME( 0x03, 0x03, "Main Game Rate" )
	PORT_DIPSETTING(    0x00, "68%" )
	PORT_DIPSETTING(    0x01, "76%" )
	PORT_DIPSETTING(    0x03, "84%" )
	PORT_DIPSETTING(    0x02, "92%" )
	PORT_DIPNAME( 0x0c, 0x0c, "Double-Up Rate" )
	PORT_DIPSETTING(    0x00, "74%" )
	PORT_DIPSETTING(    0x04, "80%" )
	PORT_DIPSETTING(    0x0c, "86%" )
	PORT_DIPSETTING(    0x08, "92%" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Minimum Bet" )
	PORT_DIPSETTING(    0x80, "1" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPSETTING(    0xc0, "10" )
	PORT_DIPSETTING(    0x00, "20" )

	PORT_START( "SW3" )
	PORT_DIPNAME( 0x01, 0x01, "SW3" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Attract Music" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START( "INA" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )    PORT_NAME("Hold 3 / Half Take")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )    PORT_NAME("Hold 4 / Big")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )    PORT_NAME("Hold 5 / Small")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )    PORT_NAME("Deal / Take")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_BET )      PORT_IMPULSE(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START( "INB" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )          PORT_IMPULSE(3) // coin
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )   // key in
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_CODE(KEYCODE_9)    PORT_NAME("Stats")  // Bookkeeping.
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_CODE(KEYCODE_0)    PORT_NAME("Settings")   // Game Rate & others.
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )  // payout
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )  // key out

	PORT_START( "INC" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_CODE(KEYCODE_R)    PORT_NAME("Reset")  // hard reset
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( tisub )

	PORT_START( "SW1" )
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:1,2,3")   // SW1-123
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x07, "1 Coin / 10 Credits" )
	PORT_DIPSETTING(    0x03, "1 Coin / 20 Credits" )
	PORT_DIPSETTING(    0x02, "1 Coin / 25 Credits" )
	PORT_DIPSETTING(    0x01, "1 Coin / 50 Credits" )
	PORT_DIPSETTING(    0x00, "1 Coin / 100 Credits" )
	PORT_DIPNAME( 0x38, 0x38, "Key In" )                PORT_DIPLOCATION("SW1:4,5,6")   // SW1-456
	PORT_DIPSETTING(    0x30, "4 Points/Pulse" )
	PORT_DIPSETTING(    0x28, "8 Points/Pulse" )
	PORT_DIPSETTING(    0x20, "20 Points/Pulse" )
	PORT_DIPSETTING(    0x38, "40 Points/Pulse" )
	PORT_DIPSETTING(    0x18, "80 Points/Pulse" )
	PORT_DIPSETTING(    0x10, "100 Points/Pulse" )
	PORT_DIPSETTING(    0x08, "200 Points/Pulse" )
	PORT_DIPSETTING(    0x00, "400 Points/Pulse" )
	PORT_DIPNAME( 0x40, 0x40, "Payout Mode" )           PORT_DIPLOCATION("SW1:7")   // SW1-7
	PORT_DIPSETTING(    0x40, "Coin Value" )
	PORT_DIPSETTING(    0x00, "Key In Value" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START( "SW2" )
	PORT_DIPNAME( 0x03, 0x03, "Minimum Bet" )           PORT_DIPLOCATION("SW2:1,2") // SW2-12
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "8" )
	PORT_DIPSETTING(    0x01, "16" )
	PORT_DIPSETTING(    0x00, "32" )
	PORT_DIPNAME( 0x0c, 0x08, "Max Bet" )               PORT_DIPLOCATION("SW2:3,4") // SW2-34
	PORT_DIPSETTING(    0x0c, "2" )
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x04, "16" )
	PORT_DIPSETTING(    0x08, "32" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:5")   // Not in test mode.
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Game Limit" )            PORT_DIPLOCATION("SW2:6")   // SW2-6
	PORT_DIPSETTING(    0x20, "20000" )
	PORT_DIPSETTING(    0x00, "30000" )
	PORT_DIPNAME( 0x40, 0x40, "Auto Take" )             PORT_DIPLOCATION("SW2:7")   // SW2-7
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START( "SW3" )
	PORT_DIPNAME( 0x07, 0x07, "Main Game Rate" )        PORT_DIPLOCATION("SW3:1,2,3")   // SW3-123
	PORT_DIPSETTING(    0x00, "77%" )
	PORT_DIPSETTING(    0x01, "80%" )
	PORT_DIPSETTING(    0x02, "83%" )
	PORT_DIPSETTING(    0x03, "86%" )
	PORT_DIPSETTING(    0x04, "89%" )
	PORT_DIPSETTING(    0x07, "92%" )
	PORT_DIPSETTING(    0x05, "95%" )
	PORT_DIPSETTING(    0x06, "98%" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START( "SW4" )
	PORT_DIPNAME( 0x07, 0x07, "Double-Up Rate" )        PORT_DIPLOCATION("SW4:1,2,3")   // SW4-123
	PORT_DIPSETTING(    0x00, "70%" )
	PORT_DIPSETTING(    0x01, "74%" )
	PORT_DIPSETTING(    0x02, "78%" )
	PORT_DIPSETTING(    0x03, "82%" )
	PORT_DIPSETTING(    0x04, "86%" )
	PORT_DIPSETTING(    0x07, "90%" )
	PORT_DIPSETTING(    0x05, "94%" )
	PORT_DIPSETTING(    0x06, "98%" )
	PORT_DIPNAME( 0x08, 0x08, "Double-Up Limit" )       PORT_DIPLOCATION("SW4:4")   // SW4-4
	PORT_DIPSETTING(    0x08, "5000" )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW4:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW4:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW4:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW4:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START( "INA" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )    PORT_NAME("Double / Info")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )    PORT_NAME("Hold 1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )    PORT_NAME("Hold 2 / Big")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )    PORT_NAME("Hold 3 / Small")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )    PORT_NAME("Start / Take")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_BET )      PORT_IMPULSE(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START( "INB" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )          PORT_IMPULSE(3) // coin
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )   // key in
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_CODE(KEYCODE_9)    PORT_NAME("Stats / Test")   // Bookkeeping.
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_CODE(KEYCODE_0)    PORT_NAME("Settings")       // Current settings.
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )  // payout
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )  // key out

	PORT_START( "INC" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_CODE(KEYCODE_R)    PORT_NAME("Reset")  // hard reset
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( stbsub )
	PORT_START("SW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x07, "1 Coin / 10 Credits" )
	PORT_DIPSETTING(    0x03, "1 Coin / 20 Credits" )
	PORT_DIPSETTING(    0x02, "1 Coin / 25 Credits" )
	PORT_DIPSETTING(    0x01, "1 Coin / 50 Credits" )
	PORT_DIPSETTING(    0x00, "1 Coin / 100 Credits" )
	PORT_DIPNAME( 0x38, 0x00, "Remote Credits" )        PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(    0x30, "1 Pulse / 1 Credits" )
	PORT_DIPSETTING(    0x28, "1 Pulse / 2 Credits" )
	PORT_DIPSETTING(    0x20, "1 Pulse / 5 Credits" )
	PORT_DIPSETTING(    0x38, "1 Pulse / 10 Credits" )
	PORT_DIPSETTING(    0x18, "1 Pulse / 20 Credits" )
	PORT_DIPSETTING(    0x10, "1 Pulse / 25 Credits" )
	PORT_DIPSETTING(    0x08, "1 Pulse / 50 Credits" )
	PORT_DIPSETTING(    0x00, "1 Pulse / 100 Credits" )
	PORT_DIPNAME( 0x40, 0x40, "Pay-out" )               PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, "Coin" )
	PORT_DIPSETTING(    0x00, "Key" )
	PORT_DIPNAME( 0x80, 0x80, "Hold Function" )         PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )

	PORT_START("SW2")
	PORT_DIPNAME( 0x03, 0x03, "Minimum Bet" )           PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x02, "1" )
	PORT_DIPSETTING(    0x03, "8" )
	PORT_DIPSETTING(    0x01, "16" )
	PORT_DIPSETTING(    0x00, "32" )
	PORT_DIPNAME( 0x0c, 0x0c, "Max Bet" )               PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x04, "16" )
	PORT_DIPSETTING(    0x08, "32" )
	PORT_DIPSETTING(    0x0c, "64" )
	PORT_DIPSETTING(    0x00, "80" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x60, "Game Limit" )            PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x40, "10000" )
	PORT_DIPSETTING(    0x60, "20000" )
	PORT_DIPSETTING(    0x20, "30000" )
	PORT_DIPSETTING(    0x00, "60000" )
	PORT_DIPNAME( 0x80, 0x80, "Double Up" )             PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )

	PORT_START( "SW3" )
	PORT_DIPNAME( 0x07, 0x07, "Win Rate" )              PORT_DIPLOCATION("SW3:1,2,3")
	PORT_DIPSETTING(    0x00, "84%" )
	PORT_DIPSETTING(    0x01, "84%" )   // yes, again!
	PORT_DIPSETTING(    0x02, "86%" )
	PORT_DIPSETTING(    0x03, "88%" )
	PORT_DIPSETTING(    0x04, "90%" )
	PORT_DIPSETTING(    0x07, "92%" )
	PORT_DIPSETTING(    0x05, "94%" )
	PORT_DIPSETTING(    0x06, "96%" )
	PORT_DIPNAME( 0x08, 0x08, "Control Panel" )         PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x08, "Type A (No Hold)" )
	PORT_DIPSETTING(    0x00, "Type B" )
	PORT_DIPNAME( 0x30, 0x30, "Max Bonus" )             PORT_DIPLOCATION("SW3:5,6")
	PORT_DIPSETTING(    0x30, "1000" )
	PORT_DIPSETTING(    0x20, "2000" )
	PORT_DIPSETTING(    0x10, "3000" )
	PORT_DIPSETTING(    0x00, "5000" )
	PORT_DIPNAME( 0x40, 0x40, "Gather Rate of Bonus" )  PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x40, "1.0%" )
	PORT_DIPSETTING(    0x00, "0.5%" )
	PORT_DIPNAME( 0x80, 0x80, "Reel Speed" )            PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x80, "Slow" )
	PORT_DIPSETTING(    0x00, "Fast" )

	PORT_START( "SW4" )
	PORT_DIPNAME( 0x07, 0x07, "Double-Up Level" )       PORT_DIPLOCATION("SW4:1,2,3")
	PORT_DIPSETTING(    0x07, "7 (Easy)" )
	PORT_DIPSETTING(    0x06, "6" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x00, "0 (Hard)" )
	PORT_DIPNAME( 0x08, 0x08, "Double-Up Limit" )       PORT_DIPLOCATION("SW4:4")
	PORT_DIPSETTING(    0x08, "5000" )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPNAME( 0x10, 0x10, "Double-Up Game" )        PORT_DIPLOCATION("SW4:5")
	PORT_DIPSETTING(    0x10, "Dancers / Panties Colors" )
	PORT_DIPSETTING(    0x00, "Cards / Seven-Bingo" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW4:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW4:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW4:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("INA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START )          PORT_CODE(KEYCODE_N)    PORT_NAME("Start / Stop All")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_BET )      PORT_CODE(KEYCODE_C)    PORT_NAME("Bet / Stop 2")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )    PORT_CODE(KEYCODE_Z)    PORT_NAME("Double / Info")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START( "INB" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )          PORT_IMPULSE(3) // coin
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )   // key in
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_CODE(KEYCODE_9)    PORT_NAME("Stats / Test")   // Bookkeeping.
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_CODE(KEYCODE_0)    PORT_NAME("Settings")       // Current settings.
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )  // payout
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )  // key out

	PORT_START("INC")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )     PORT_CODE(KEYCODE_V)    PORT_NAME("Small / Black / Stop 3")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("hopper", ticket_dispenser_device, line_r) // hopper sensor
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_CODE(KEYCODE_R)    PORT_NAME("Reset")  // hard reset
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )    PORT_CODE(KEYCODE_B)    PORT_NAME("Big / Red")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )    PORT_CODE(KEYCODE_X)    PORT_NAME("Take / Stop 1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( tesorone )
	PORT_START("SW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )        // 5
	PORT_DIPSETTING(    0x01, "1 Coin / 10 Credits" )   // 16
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:2")   // ?
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:3")   // ?
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:7")   // ?
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:8")   // ?
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:1")   // ?
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:2")   // ?
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:3")   // ?
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x60, "Game Limit" )            PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x40, "5000" )
	PORT_DIPSETTING(    0x60, "10000" )
	PORT_DIPSETTING(    0x20, "20000" )
	PORT_DIPSETTING(    0x00, "30000" )
	PORT_DIPNAME( 0x80, 0x80, "Double Up" )             PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )

	PORT_START( "SW3" )
	PORT_DIPNAME( 0x07, 0x07, "Win Rate" )              PORT_DIPLOCATION("SW3:1,2,3")
	PORT_DIPSETTING(    0x00, "59%" )
	PORT_DIPSETTING(    0x01, "64%" )
	PORT_DIPSETTING(    0x02, "69%" )
	PORT_DIPSETTING(    0x03, "74%" )
	PORT_DIPSETTING(    0x04, "79%" )
	PORT_DIPSETTING(    0x07, "84%" )
	PORT_DIPSETTING(    0x05, "89%" )
	PORT_DIPSETTING(    0x06, "94%" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW3:4")   // ?
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Reel Speed" )            PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x80, "Slow" )
	PORT_DIPSETTING(    0x00, "Fast" )

	PORT_START( "SW4" )
	PORT_DIPNAME( 0x07, 0x07, "Double-Up Level" )       PORT_DIPLOCATION("SW4:1,2,3")
	PORT_DIPSETTING(    0x07, "7 (Easy)" )
	PORT_DIPSETTING(    0x06, "6" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x00, "0 (Hard)" )
	PORT_DIPNAME( 0x08, 0x08, "Double-Up Limit" )       PORT_DIPLOCATION("SW4:4")
	PORT_DIPSETTING(    0x08, "5000" )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPNAME( 0x70, 0x70, "Remote Credits" )        PORT_DIPLOCATION("SW4:5,6,7")
//  PORT_DIPSETTING(    0x00, "50" )
//  PORT_DIPSETTING(    0x10, "50" )
//  PORT_DIPSETTING(    0x20, "50" )
	PORT_DIPSETTING(    0x30, "50" )
	PORT_DIPSETTING(    0x70, "100" )
	PORT_DIPSETTING(    0x60, "200" )
	PORT_DIPSETTING(    0x50, "400" )
	PORT_DIPSETTING(    0x40, "800" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW4:8")   // ?
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("INA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START )          PORT_CODE(KEYCODE_N)    PORT_NAME("Start / Stop All")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_BET )      PORT_CODE(KEYCODE_C)    PORT_NAME("Bet / Stop 2")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )    PORT_CODE(KEYCODE_Z)    PORT_NAME("Double / Info")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START( "INB" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )          PORT_IMPULSE(3) // coin
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )   // key in
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_CODE(KEYCODE_9)    PORT_NAME("Stats / Test")   // Bookkeeping.
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_CODE(KEYCODE_0)    PORT_NAME("Settings")       // Current settings.
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )  // payout
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )  // key out

	PORT_START("INC")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )     PORT_CODE(KEYCODE_V)    PORT_NAME("Small / Black / Stop 3")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("hopper", ticket_dispenser_device, line_r) // hopper sensor
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_CODE(KEYCODE_R)    PORT_NAME("Reset")  // hard reset
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )    PORT_CODE(KEYCODE_B)    PORT_NAME("Big / Red")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )    PORT_CODE(KEYCODE_X)    PORT_NAME("Take / Stop 1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( crsbingo )

	PORT_START( "SW1" )
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x07, "1 Coin / 10 Credits" )
	PORT_DIPSETTING(    0x03, "1 Coin / 20 Credits" )
	PORT_DIPSETTING(    0x02, "1 Coin / 25 Credits" )
	PORT_DIPSETTING(    0x01, "1 Coin / 50 Credits" )
	PORT_DIPSETTING(    0x00, "1 Coin / 100 Credits" )
	PORT_DIPNAME( 0x38, 0x38, "Key In" )
	PORT_DIPSETTING(    0x30, "4 Points/Pulse" )
	PORT_DIPSETTING(    0x28, "8 Points/Pulse" )
	PORT_DIPSETTING(    0x20, "20 Points/Pulse" )
	PORT_DIPSETTING(    0x10, "40 Points/Pulse" )
	PORT_DIPSETTING(    0x18, "80 Points/Pulse" )
	PORT_DIPSETTING(    0x38, "100 Points/Pulse" )
	PORT_DIPSETTING(    0x08, "200 Points/Pulse" )
	PORT_DIPSETTING(    0x00, "400 Points/Pulse" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START( "SW2" )
	PORT_DIPNAME( 0x03, 0x03, "Minimum Bet" )
	PORT_DIPSETTING(    0x02, "1" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x03, "10" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x0c, 0x0c, "Max Bet" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPSETTING(    0x08, "20" )
	PORT_DIPSETTING(    0x0c, "50" )
	PORT_DIPNAME( 0x30, 0x30, "Cards Graphics" )
	PORT_DIPSETTING(    0x00, "Classic Cards" )
	PORT_DIPSETTING(    0x10, "Alternate Set" )
	PORT_DIPSETTING(    0x20, "Fruits" )
	PORT_DIPSETTING(    0x30, "Classic Cards" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START( "SW3" )
	PORT_DIPNAME( 0x01, 0x01, "SW3" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START( "SW4" )
	PORT_DIPNAME( 0x07, 0x07, "Double-Up Rate" )
	PORT_DIPSETTING(    0x00, "70%" )
	PORT_DIPSETTING(    0x01, "74%" )
	PORT_DIPSETTING(    0x02, "78%" )
	PORT_DIPSETTING(    0x03, "82%" )
	PORT_DIPSETTING(    0x04, "86%" )
	PORT_DIPSETTING(    0x07, "90%" )
	PORT_DIPSETTING(    0x05, "94%" )
	PORT_DIPSETTING(    0x06, "98%" )
	PORT_DIPNAME( 0x08, 0x08, "Double-Up Type" )
	PORT_DIPSETTING(    0x08, "Type 1 (no change card)" )
	PORT_DIPSETTING(    0x00, "Type 2 (with change card)" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START( "INA" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )    PORT_NAME("Hold 2 / Double")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )    PORT_NAME("Hold 3 / Change")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )    PORT_NAME("Hold 4 / Big")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )    PORT_NAME("Hold 5 / Small")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )    PORT_NAME("Deal / Take")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_BET )      PORT_IMPULSE(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START( "INB" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )          PORT_IMPULSE(3) // coin
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )   // key in
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_CODE(KEYCODE_9)    PORT_NAME("Stats")      // Bookkeeping.
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_CODE(KEYCODE_0)    PORT_NAME("Settings")   // Game Rate & others.
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )  // payout
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )  // key out

	PORT_START( "INC" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_CODE(KEYCODE_R)    PORT_NAME("Reset")  // hard reset
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( sharkpy )

	PORT_START( "SW1" )
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x01, "1 Coin / 10 Credits" )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START( "SW2" )
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Double-Up" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )

	PORT_START( "SW3" )
	PORT_DIPNAME( 0x07, 0x07, "Main Game Rate" )
	PORT_DIPSETTING(    0x00, "55%" )
	PORT_DIPSETTING(    0x01, "60%" )
	PORT_DIPSETTING(    0x02, "65%" )
	PORT_DIPSETTING(    0x03, "70%" )
	PORT_DIPSETTING(    0x04, "75%" )
	PORT_DIPSETTING(    0x07, "80%" )
	PORT_DIPSETTING(    0x05, "85%" )
	PORT_DIPSETTING(    0x06, "90%" )
	PORT_DIPNAME( 0x38, 0x38, "Double-Up Rate" )
	PORT_DIPSETTING(    0x30, "0" )
	PORT_DIPSETTING(    0x28, "1" )
	PORT_DIPSETTING(    0x38, "2" )
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPSETTING(    0x18, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x08, "6" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START( "SW4" )
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START( "INA" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )    PORT_NAME("Hold 1 / Double (Select)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )    PORT_NAME("Hold 4 / Big")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )    PORT_NAME("Hold 5 / Small")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )    PORT_NAME("Deal / Take")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_BET )      PORT_NAME("Bet")    PORT_IMPULSE(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START( "INB" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )          PORT_IMPULSE(3) // coin
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_CODE(KEYCODE_9)    PORT_NAME("Stats / Test")   // Bookkeeping.
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_CODE(KEYCODE_0)    PORT_NAME("Settings")   // Game Rate & others.
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )  // payout?
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )  // key out?

	PORT_START( "INC" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_CODE(KEYCODE_R)    PORT_NAME("Reset")  // hard reset
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( sharkpye )

	PORT_START( "SW1" )
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x07, "1 Coin / 10 Credits" )
	PORT_DIPSETTING(    0x03, "1 Coin / 20 Credits" )
	PORT_DIPSETTING(    0x02, "1 Coin / 25 Credits" )
	PORT_DIPSETTING(    0x01, "1 Coin / 50 Credits" )
	PORT_DIPSETTING(    0x00, "1 Coin / 100 Credits" )
	PORT_DIPNAME( 0x38, 0x38, "Remote Credits" )
	PORT_DIPSETTING(    0x30, "1 Pulse / 1 Credits" )
	PORT_DIPSETTING(    0x28, "1 Pulse / 2 Credits" )
	PORT_DIPSETTING(    0x20, "1 Pulse / 5 Credits" )
	PORT_DIPSETTING(    0x18, "1 Pulse / 10 Credits" )
	PORT_DIPSETTING(    0x10, "1 Pulse / 50 Credits" )
	PORT_DIPSETTING(    0x38, "1 Pulse / 100 Credits" )
	PORT_DIPSETTING(    0x08, "1 Pulse / 200 Credits" )
	PORT_DIPSETTING(    0x00, "1 Pulse / 500 Credits" )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START( "SW2" )
	PORT_DIPNAME( 0x03, 0x03, "Minimum Bet" )
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x01, "10" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x0c, 0x0c, "Max Bet" )
	PORT_DIPSETTING(    0x08, "10" )
	PORT_DIPSETTING(    0x04, "40" )
	PORT_DIPSETTING(    0x0c, "50" )
	PORT_DIPSETTING(    0x00, "80" )
	PORT_DIPNAME( 0x10, 0x10, "Jokers" )
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START( "SW3" )
	PORT_DIPNAME( 0x07, 0x07, "Main Game Rate" )
	PORT_DIPSETTING(    0x00, "92%" )
	PORT_DIPSETTING(    0x01, "93%" )
	PORT_DIPSETTING(    0x02, "94%" )
	PORT_DIPSETTING(    0x03, "95%" )
	PORT_DIPSETTING(    0x04, "96%" )
	PORT_DIPSETTING(    0x07, "97%" )
	PORT_DIPSETTING(    0x05, "98%" )
	PORT_DIPSETTING(    0x06, "99%" )
	PORT_DIPNAME( 0x38, 0x38, "Double-Up Rate" )
	PORT_DIPSETTING(    0x00, "82" )
	PORT_DIPSETTING(    0x08, "84" )
	PORT_DIPSETTING(    0x10, "86" )
	PORT_DIPSETTING(    0x18, "88" )
	PORT_DIPSETTING(    0x20, "90" )
	PORT_DIPSETTING(    0x38, "92" )
	PORT_DIPSETTING(    0x28, "94" )
	PORT_DIPSETTING(    0x30, "96" )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START( "SW4" )
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START( "INA" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )    PORT_NAME("Start")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )    PORT_NAME("Hold 1 / Bet")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )    PORT_NAME("Hold 3 / Double-Up")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START( "INB" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )          PORT_IMPULSE(3) // coin
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_CODE(KEYCODE_9)    PORT_NAME("Stats / Test")   // Bookkeeping.
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_CODE(KEYCODE_0)    PORT_NAME("Settings")   // Game Rate & others.
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )

	PORT_START( "INC" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )    PORT_NAME("Hold 4 / Small")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_CODE(KEYCODE_R)    PORT_NAME("Reset Switch")   // hard reset
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )    PORT_NAME("Hold 5 / Big")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )    PORT_NAME("Hold 2 / Take")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( smoto16 )

	PORT_START( "SW1" )
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x01, "1 Coin / 10 Credits" )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPNAME( 0x04, 0x04, "Hopper" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START( "SW2" )
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Double-Up" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )

	PORT_START( "SW3" )
	PORT_DIPNAME( 0x07, 0x07, "Main Game Rate" )
	PORT_DIPSETTING(    0x00, "55%" )
	PORT_DIPSETTING(    0x01, "60%" )
	PORT_DIPSETTING(    0x02, "65%" )
	PORT_DIPSETTING(    0x03, "70%" )
	PORT_DIPSETTING(    0x04, "75%" )
	PORT_DIPSETTING(    0x07, "80%" )
	PORT_DIPSETTING(    0x05, "85%" )
	PORT_DIPSETTING(    0x06, "90%" )
	PORT_DIPNAME( 0x38, 0x38, "Double-Up Rate" )
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x18, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x28, "5" )
	PORT_DIPSETTING(    0x30, "6" )
	PORT_DIPSETTING(    0x38, "7" )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START( "SW4" )
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPNAME( 0x40, 0x40, "Game Control" )
	PORT_DIPSETTING(    0x40, "Normal Holds" )
	PORT_DIPSETTING(    0x00, "Left-Right Marker" )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START( "INA" ) // d005
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )    PORT_NAME("Hold 1 / Double (Select)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )    PORT_NAME("Hold 2 / Right")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )    PORT_NAME("Hold 3 / Left")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )    PORT_NAME("Hold 4 / Select")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )    PORT_NAME("Deal / Take")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_BET )      PORT_NAME("Bet / Speed")    PORT_IMPULSE(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START( "INB" ) // d006
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )          PORT_IMPULSE(3) // coin
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("hopper", ticket_dispenser_device, line_r) // hopper sensor
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_CODE(KEYCODE_9)    PORT_NAME("Stats / Test")   // Bookkeeping.
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_CODE(KEYCODE_0)    PORT_NAME("Settings")   // Game Rate & others.
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )  // payout
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )  // key out?

	PORT_START( "INC" ) // d00c
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("hopper", ticket_dispenser_device, line_r) // hopper sensor
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_CODE(KEYCODE_R)    PORT_NAME("Reset")  // hard reset
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( smoto20 )

	PORT_START( "SW1" )
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x01, "1 Coin / 10 Credits" )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPNAME( 0x04, 0x04, "Hopper" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START( "SW2" )
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Double-Up" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )

	PORT_START( "SW3" )
	PORT_DIPNAME( 0x07, 0x07, "Main Game Rate" )
	PORT_DIPSETTING(    0x00, "25%" )
	PORT_DIPSETTING(    0x01, "30%" )
	PORT_DIPSETTING(    0x02, "35%" )
	PORT_DIPSETTING(    0x03, "40%" )
	PORT_DIPSETTING(    0x04, "45%" )
	PORT_DIPSETTING(    0x07, "50%" )
	PORT_DIPSETTING(    0x05, "55%" )
	PORT_DIPSETTING(    0x06, "60%" )
	PORT_DIPNAME( 0x38, 0x38, "Double-Up Rate" )
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x18, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x28, "5" )
	PORT_DIPSETTING(    0x30, "6" )
	PORT_DIPSETTING(    0x38, "7" )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START( "SW4" )
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPNAME( 0x40, 0x40, "Game Control" )
	PORT_DIPSETTING(    0x40, "Normal Holds" )
	PORT_DIPSETTING(    0x00, "Left-Right Marker" )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START( "INA" ) // d005
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )    PORT_NAME("Hold 1 / Double (Select)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )    PORT_NAME("Hold 2 / Right")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )    PORT_NAME("Hold 3 / Left")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )    PORT_NAME("Hold 4 / Select")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )    PORT_NAME("Deal / Take")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_BET )      PORT_NAME("Bet / Speed")    PORT_IMPULSE(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START( "INB" ) // d006
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )          PORT_IMPULSE(3) // coin
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("hopper", ticket_dispenser_device, line_r) // hopper sensor
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_CODE(KEYCODE_9)    PORT_NAME("Stats / Test")   // Bookkeeping.
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_CODE(KEYCODE_0)    PORT_NAME("Settings")   // Game Rate & others.
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )  // payout
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )  // key out?

	PORT_START( "INC" ) // d00c
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("hopper", ticket_dispenser_device, line_r) // hopper sensor
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_CODE(KEYCODE_R)    PORT_NAME("Reset")  // hard reset
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( victor6 )

	PORT_START( "SW1" )
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x07, "1 Coin / 10 Credits" )
	PORT_DIPSETTING(    0x03, "1 Coin / 20 Credits" )
	PORT_DIPSETTING(    0x02, "1 Coin / 25 Credits" )
	PORT_DIPSETTING(    0x01, "1 Coin / 50 Credits" )
	PORT_DIPSETTING(    0x00, "1 Coin / 100 Credits" )
	PORT_DIPNAME( 0x38, 0x38, "Remote Credits" )
	PORT_DIPSETTING(    0x30, "1 Pulse / 1 Credits" )
	PORT_DIPSETTING(    0x28, "1 Pulse / 2 Credits" )
	PORT_DIPSETTING(    0x20, "1 Pulse / 5 Credits" )
	PORT_DIPSETTING(    0x18, "1 Pulse / 10 Credits" )
	PORT_DIPSETTING(    0x10, "1 Pulse / 50 Credits" )
	PORT_DIPSETTING(    0x38, "1 Pulse / 100 Credits" )
	PORT_DIPSETTING(    0x08, "1 Pulse / 200 Credits" )
	PORT_DIPSETTING(    0x00, "1 Pulse / 500 Credits" )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START( "SW2" )
	PORT_DIPNAME( 0x03, 0x03, "Minimum Bet" )
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x01, "10" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x0c, 0x0c, "Max Bet" )
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x08, "10" )
	PORT_DIPSETTING(    0x04, "40" )
	PORT_DIPSETTING(    0x0c, "50" )
	PORT_DIPNAME( 0x10, 0x10, "Jokers" )
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START( "SW3" )
	PORT_DIPNAME( 0x07, 0x07, "Main Game Rate" )
	PORT_DIPSETTING(    0x00, "87%" )
	PORT_DIPSETTING(    0x01, "89%" )
	PORT_DIPSETTING(    0x02, "91%" )
	PORT_DIPSETTING(    0x03, "93%" )
	PORT_DIPSETTING(    0x04, "95%" )
	PORT_DIPSETTING(    0x07, "97%" )
	PORT_DIPSETTING(    0x05, "99%" )
	PORT_DIPSETTING(    0x06, "101%" )
	PORT_DIPNAME( 0x38, 0x38, "Double-Up Rate" )
	PORT_DIPSETTING(    0x00, "82" )
	PORT_DIPSETTING(    0x08, "84" )
	PORT_DIPSETTING(    0x10, "86" )
	PORT_DIPSETTING(    0x18, "88" )
	PORT_DIPSETTING(    0x20, "90" )
	PORT_DIPSETTING(    0x38, "92" )
	PORT_DIPSETTING(    0x28, "94" )
	PORT_DIPSETTING(    0x30, "96" )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START( "SW4" )
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START( "INA" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )    PORT_NAME("Start")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )    PORT_NAME("Hold 1 / Bet")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )    PORT_NAME("Hold 3 / Double-Up")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START( "INB" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )          PORT_IMPULSE(3) // coin
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_CODE(KEYCODE_9)    PORT_NAME("Stats / Test")   // Bookkeeping.
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_CODE(KEYCODE_0)    PORT_NAME("Settings")   // Game Rate & others.
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )

	PORT_START( "INC" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )    PORT_NAME("Hold 4 / Small")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_CODE(KEYCODE_R)    PORT_NAME("Reset Switch")   // hard reset
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )    PORT_NAME("Hold 5 / Big")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )    PORT_NAME("Hold 2 / Take")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( victor6a )

	PORT_START( "SW1" )
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x07, "1 Coin / 10 Credits" )
	PORT_DIPSETTING(    0x03, "1 Coin / 20 Credits" )
	PORT_DIPSETTING(    0x02, "1 Coin / 25 Credits" )
	PORT_DIPSETTING(    0x01, "1 Coin / 50 Credits" )
	PORT_DIPSETTING(    0x00, "1 Coin / 100 Credits" )
	PORT_DIPNAME( 0x38, 0x38, "Remote Credits" )
	PORT_DIPSETTING(    0x30, "1 Pulse / 1 Credits" )
	PORT_DIPSETTING(    0x28, "1 Pulse / 2 Credits" )
	PORT_DIPSETTING(    0x20, "1 Pulse / 5 Credits" )
	PORT_DIPSETTING(    0x18, "1 Pulse / 10 Credits" )
	PORT_DIPSETTING(    0x10, "1 Pulse / 50 Credits" )
	PORT_DIPSETTING(    0x38, "1 Pulse / 100 Credits" )
	PORT_DIPSETTING(    0x08, "1 Pulse / 200 Credits" )
	PORT_DIPSETTING(    0x00, "1 Pulse / 500 Credits" )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START( "SW2" )
	PORT_DIPNAME( 0x03, 0x03, "Minimum Bet" )
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x01, "10" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x0c, 0x0c, "Max Bet" )
	PORT_DIPSETTING(    0x08, "10" )
	PORT_DIPSETTING(    0x04, "40" )
	PORT_DIPSETTING(    0x0c, "50" )
	PORT_DIPSETTING(    0x00, "80" )
	PORT_DIPNAME( 0x10, 0x10, "Jokers" )
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START( "SW3" )
	PORT_DIPNAME( 0x07, 0x07, "Main Game Rate" )
	PORT_DIPSETTING(    0x00, "94%" )
	PORT_DIPSETTING(    0x01, "95%" )
	PORT_DIPSETTING(    0x02, "96%" )
	PORT_DIPSETTING(    0x03, "97%" )
	PORT_DIPSETTING(    0x04, "98%" )
	PORT_DIPSETTING(    0x07, "99%" )
	PORT_DIPSETTING(    0x05, "100%" )
	PORT_DIPSETTING(    0x06, "101%" )
	PORT_DIPNAME( 0x38, 0x38, "Double-Up Rate" )
	PORT_DIPSETTING(    0x00, "82" )
	PORT_DIPSETTING(    0x08, "84" )
	PORT_DIPSETTING(    0x10, "86" )
	PORT_DIPSETTING(    0x18, "88" )
	PORT_DIPSETTING(    0x20, "90" )
	PORT_DIPSETTING(    0x38, "92" )
	PORT_DIPSETTING(    0x28, "94" )
	PORT_DIPSETTING(    0x30, "96" )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START( "SW4" )
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START( "INA" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )    PORT_NAME("Start")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )    PORT_NAME("Hold 1 / Bet")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )    PORT_NAME("Hold 3 / Double-Up")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START( "INB" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )          PORT_IMPULSE(3) // coin
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_CODE(KEYCODE_9)    PORT_NAME("Stats / Test")   // Bookkeeping.
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_CODE(KEYCODE_0)    PORT_NAME("Settings")   // Game Rate & others.
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )

	PORT_START( "INC" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )    PORT_NAME("Hold 4 / Small")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_CODE(KEYCODE_R)    PORT_NAME("Reset Switch")   // hard reset
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )    PORT_NAME("Hold 5 / Big")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )    PORT_NAME("Hold 2 / Take")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( victor6b )

	PORT_START( "SW1" )
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x07, "1 Coin / 10 Credits" )
	PORT_DIPSETTING(    0x03, "1 Coin / 20 Credits" )
	PORT_DIPSETTING(    0x02, "1 Coin / 25 Credits" )
	PORT_DIPSETTING(    0x01, "1 Coin / 50 Credits" )
	PORT_DIPSETTING(    0x00, "1 Coin / 100 Credits" )
	PORT_DIPNAME( 0x38, 0x38, "Remote Credits" )
	PORT_DIPSETTING(    0x30, "1 Pulse / 1 Credits" )
	PORT_DIPSETTING(    0x28, "1 Pulse / 2 Credits" )
	PORT_DIPSETTING(    0x20, "1 Pulse / 5 Credits" )
	PORT_DIPSETTING(    0x18, "1 Pulse / 10 Credits" )
	PORT_DIPSETTING(    0x10, "1 Pulse / 50 Credits" )
	PORT_DIPSETTING(    0x38, "1 Pulse / 100 Credits" )
	PORT_DIPSETTING(    0x08, "1 Pulse / 200 Credits" )
	PORT_DIPSETTING(    0x00, "1 Pulse / 500 Credits" )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START( "SW2" )
	PORT_DIPNAME( 0x03, 0x03, "Minimum Bet" )
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x01, "10" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x0c, 0x0c, "Max Bet" )
	PORT_DIPSETTING(    0x08, "Invalid" )
	PORT_DIPSETTING(    0x04, "40" )
	PORT_DIPSETTING(    0x0c, "50" )
	PORT_DIPSETTING(    0x00, "80" )
	PORT_DIPNAME( 0x10, 0x10, "Jokers" )
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START( "SW3" )
	PORT_DIPNAME( 0x07, 0x07, "Main Game Rate" )
	PORT_DIPSETTING(    0x00, "94%" )
	PORT_DIPSETTING(    0x01, "95%" )
	PORT_DIPSETTING(    0x02, "96%" )
	PORT_DIPSETTING(    0x03, "97%" )
	PORT_DIPSETTING(    0x04, "98%" )
	PORT_DIPSETTING(    0x07, "99%" )
	PORT_DIPSETTING(    0x05, "100%" )
	PORT_DIPSETTING(    0x06, "101%" )
	PORT_DIPNAME( 0x38, 0x38, "Double-Up Rate" )
	PORT_DIPSETTING(    0x00, "82" )
	PORT_DIPSETTING(    0x08, "84" )
	PORT_DIPSETTING(    0x10, "86" )
	PORT_DIPSETTING(    0x18, "88" )
	PORT_DIPSETTING(    0x20, "90" )
	PORT_DIPSETTING(    0x38, "92" )
	PORT_DIPSETTING(    0x28, "94" )
	PORT_DIPSETTING(    0x30, "96" )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START( "SW4" )
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START( "INA" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )    PORT_NAME("Start")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )    PORT_NAME("Hold 1 / Bet")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )    PORT_NAME("Hold 3 / Double-Up")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START( "INB" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )          PORT_IMPULSE(3) // coin
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_CODE(KEYCODE_9)    PORT_NAME("Stats / Test")   // Bookkeeping.
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_CODE(KEYCODE_0)    PORT_NAME("Settings")   // Game Rate & others.
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )

	PORT_START( "INC" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )    PORT_NAME("Hold 4 / Small")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_CODE(KEYCODE_R)    PORT_NAME("Reset Switch")   // hard reset
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )    PORT_NAME("Hold 5 / Big")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )    PORT_NAME("Hold 2 / Take")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************
*                             Graphics Layout                              *
***************************************************************************/

static const gfx_layout layout_8x8x3 =
{
	8, 8,
	RGN_FRAC(1, 3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static const gfx_layout layout_8x8x4 =
{
	8, 8,
	RGN_FRAC(1, 4),
	4,
	{ RGN_FRAC(2,4), RGN_FRAC(3,4), RGN_FRAC(0,4), RGN_FRAC(1,4) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static const gfx_layout layout_8x32x4 =
{
	8, 32,
	RGN_FRAC(1, 4),
	4,
	{ RGN_FRAC(2,4), RGN_FRAC(3,4), RGN_FRAC(0,4), RGN_FRAC(1,4) },
	{ STEP8(0,1) },
	{ STEP32(0,8) },
	8*8*4
};

static const gfx_layout layout_8x8x8 =
{
	8, 8,
	RGN_FRAC(1,4),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ RGN_FRAC(0,4), RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4), RGN_FRAC(0,4)+8, RGN_FRAC(1,4)+8, RGN_FRAC(2,4)+8, RGN_FRAC(3,4)+8 },
	{ 0*16,1*16,2*16,3*16,4*16,5*16,6*16,7*16 },
	8*16
};

static const gfx_layout layout_8x32x8 =
{
	8, 32,
	RGN_FRAC(1,4),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ RGN_FRAC(0,4), RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4), RGN_FRAC(0,4)+8, RGN_FRAC(1,4)+8, RGN_FRAC(2,4)+8, RGN_FRAC(3,4)+8 },
	{ 0*16,1*16,2*16,3*16,4*16,5*16,6*16,7*16, 8*16,9*16,10*16,11*16,12*16,13*16,14*16,15*16,
		16*16,17*16,18*16,19*16,20*16,21*16,22*16,23*16,24*16,25*16,26*16,27*16,28*16,29*16,30*16,31*16},
	32*16
};


static GFXDECODE_START( subsino_depth3 )
	GFXDECODE_ENTRY( "tilemap", 0, layout_8x8x3, 0, 16 )
GFXDECODE_END

static GFXDECODE_START( subsino_depth4 )
	GFXDECODE_ENTRY( "tilemap", 0, layout_8x8x4, 0, 16 )
GFXDECODE_END

static GFXDECODE_START( subsino_depth4_reels )
	GFXDECODE_ENTRY( "tilemap", 0, layout_8x8x4, 0, 16 )
	GFXDECODE_ENTRY( "reels", 0, layout_8x32x4, 0, 16 )
GFXDECODE_END

static GFXDECODE_START( subsino_stbsub )
	GFXDECODE_ENTRY( "tilemap", 0, layout_8x8x8, 0, 1 )
	GFXDECODE_ENTRY( "reels", 0, layout_8x32x8, 0, 1 )
GFXDECODE_END

/***************************************************************************
*                             Machine Drivers                              *
***************************************************************************/

static MACHINE_CONFIG_START( victor21, subsino_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z180, XTAL_12MHz / 8)   /* Unknown clock */
	MCFG_CPU_PROGRAM_MAP(victor21_map)
	MCFG_CPU_IO_MAP(subsino_iomap)

	MCFG_TICKET_DISPENSER_ADD("hopper", attotime::from_msec(200), TICKET_MOTOR_ACTIVE_HIGH, TICKET_STATUS_ACTIVE_LOW)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0+16, 256-16-1)
	MCFG_SCREEN_UPDATE_DRIVER(subsino_state, screen_update_subsino)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", subsino_depth3)

	MCFG_PALETTE_ADD("palette", 0x100)
	MCFG_PALETTE_INIT_OWNER(subsino_state,subsino_2proms)

	MCFG_VIDEO_START_OVERRIDE(subsino_state,subsino)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2413, XTAL_3_579545MHz)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_OKIM6295_ADD("oki", XTAL_4_433619MHz / 4, OKIM6295_PIN7_HIGH)  /* Clock frequency & pin 7 not verified */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

/* same but with an additional protection. */
static MACHINE_CONFIG_DERIVED( victor5, victor21 )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(victor5_map)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( crsbingo, subsino_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z180, XTAL_12MHz / 8)   /* Unknown CPU and clock */
	MCFG_CPU_PROGRAM_MAP(crsbingo_map)
	MCFG_CPU_IO_MAP(subsino_iomap)

	MCFG_TICKET_DISPENSER_ADD("hopper", attotime::from_msec(200), TICKET_MOTOR_ACTIVE_HIGH, TICKET_STATUS_ACTIVE_LOW)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0+16, 256-16-1)
	MCFG_SCREEN_UPDATE_DRIVER(subsino_state, screen_update_subsino)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", subsino_depth4)

	MCFG_PALETTE_ADD("palette", 0x100)
	MCFG_PALETTE_INIT_OWNER(subsino_state,subsino_2proms)

	MCFG_VIDEO_START_OVERRIDE(subsino_state,subsino)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2413, XTAL_3_579545MHz)   /* Unknown clock */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( srider, subsino_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z180, XTAL_12MHz / 8)   /* Unknown clock */
	MCFG_CPU_PROGRAM_MAP(srider_map)
	MCFG_CPU_IO_MAP(subsino_iomap)

	MCFG_TICKET_DISPENSER_ADD("hopper", attotime::from_msec(200), TICKET_MOTOR_ACTIVE_HIGH, TICKET_STATUS_ACTIVE_LOW)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0+16, 256-16-1)
	MCFG_SCREEN_UPDATE_DRIVER(subsino_state, screen_update_subsino)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", subsino_depth4)

	MCFG_PALETTE_ADD("palette", 0x100)
	MCFG_PALETTE_INIT_OWNER(subsino_state,subsino_3proms)

	MCFG_VIDEO_START_OVERRIDE(subsino_state,subsino)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, XTAL_3_579545MHz)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_OKIM6295_ADD("oki", XTAL_4_433619MHz / 4, OKIM6295_PIN7_HIGH)  /* Clock frequency & pin 7 not verified */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( sharkpy, srider )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(sharkpy_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( tisub, subsino_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z180, XTAL_12MHz / 8)   /* Unknown CPU and clock */
	MCFG_CPU_PROGRAM_MAP(tisub_map)
	MCFG_CPU_IO_MAP(subsino_iomap)

	MCFG_TICKET_DISPENSER_ADD("hopper", attotime::from_msec(200), TICKET_MOTOR_ACTIVE_HIGH, TICKET_STATUS_ACTIVE_LOW)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0+16, 256-16-1)
	MCFG_SCREEN_UPDATE_DRIVER(subsino_state, screen_update_subsino_reels)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", subsino_depth4_reels)

	MCFG_PALETTE_ADD("palette", 0x100)
	MCFG_PALETTE_INIT_OWNER(subsino_state,subsino_3proms)

	MCFG_VIDEO_START_OVERRIDE(subsino_state,subsino_reels)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, XTAL_3_579545MHz)   /* Unknown clock */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( stbsub, subsino_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z180, XTAL_12MHz / 8)   /* Unknown clock */
	MCFG_CPU_PROGRAM_MAP(stbsub_map)
	MCFG_CPU_IO_MAP(subsino_iomap)

	MCFG_TICKET_DISPENSER_ADD("hopper", attotime::from_msec(200), TICKET_MOTOR_ACTIVE_HIGH, TICKET_STATUS_ACTIVE_LOW)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0+16, 256-16-1)
	MCFG_SCREEN_UPDATE_DRIVER(subsino_state, screen_update_stbsub_reels)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", subsino_stbsub)

	MCFG_PALETTE_ADD("palette", 0x100)
	//MCFG_PALETTE_INIT_OWNER(subsino_state,subsino_3proms)

	MCFG_VIDEO_START_OVERRIDE(subsino_state,stbsub)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, XTAL_3_579545MHz)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( mtrainnv, stbsub )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(mtrainnv_map)
MACHINE_CONFIG_END


/***************************************************************************
*                               ROMs Loading                               *
***************************************************************************/

/***************************************************************************

Victor 5
(C)1991 Subsino

Chips:

1x unknown big black box
1x M5L8255AP
1x UM3567
1x M6295
1x oscillator 12.000
1x oscillator 4.433619
1x oscillator 3.579545

ROMs:

1x M27C512 (1)
3x 27C256 (2,3,4)

Notes:

1x 36x2 edge connector (con3)
1x 10x2 edge connector (con4)
1x RS232 9pins connector (con5)
2x batteries
3x 8x2 switches dip
1x pushbutton

Sticker on PCB reads V552520

Info by f205v (26/03/2008)

***************************************************************************/

ROM_START( victor5 )
	ROM_REGION( 0x14000, "maincpu", 0 )
	ROM_LOAD( "1.u1", 0x10000, 0x4000, CRC(e3ada2fc) SHA1(eddb460dcb80a29fbbe3ed6c4733c75b892baf52) )
	ROM_CONTINUE(0x0000,0xc000)

	ROM_REGION( 0x18000, "tilemap", 0 )
	ROM_LOAD( "2.u24", 0x10000, 0x8000, CRC(1229e951) SHA1(1e548625bb60e2d6f52a376a0dea9e5709f94135) )
	ROM_LOAD( "3.u23", 0x08000, 0x8000, CRC(2d89bbf1) SHA1(d7fda0174a835e88b330dfd09bdb604bfe4c2e44) )
	ROM_LOAD( "4.u22", 0x00000, 0x8000, CRC(ecf840a1) SHA1(9ecf522afb23e3557d37effc3c8568e8a14dad1a) )

	ROM_REGION( 0x40000, "oki", ROMREGION_ERASE )
	// rom socket is empty

	ROM_REGION( 0x200, "proms", 0 ) //missing proms in this set, we'll use the same as Victor 21 for now.
	ROM_LOAD( "74s287.u35", 0x000, 0x100, BAD_DUMP CRC(40094bed) SHA1(b25d96126b3f7bd06bf76dc9958f8669f83abdb7) )
	ROM_LOAD( "74s287.u36", 0x100, 0x100, BAD_DUMP CRC(9ca021c5) SHA1(6a1d8d4f958d36e4a676dc4f4aee83d637933bc3) )
ROM_END

/***************************************************************************

Victor 21
(C)1990 Subsino

Chips:

1x unknown big black box
1x M5L8255AP
1x UM3567
1x M6295
1x oscillator 12.000
1x oscillator 4.433619
1x oscillator 3.579545

ROMs:

1x M27C512 (1)
3x 27C256 (2,3,4)

Other:

1x 36x2 edge connector (con3)
1x 10x2 edge connector (con4)
1x RS232 9pins connector (con5)
2x batteries
3x 8 switches dips
1x pushbutton

PCB layout is identical to "Victor 5"
Sticker on PCB reads V12040

Info by f205v, Corrado Tomaselli (20/04/2008)

***************************************************************************/

ROM_START( victor21 )
	ROM_REGION( 0x14000, "maincpu", 0 )
	ROM_LOAD( "1.u1", 0x10000, 0x4000, CRC(43999b2d) SHA1(7ce26fd332ffe35fd826a1a6166b228d4bc370b8) )
	ROM_CONTINUE(0x0000,0xc000)

	ROM_REGION( 0x18000, "tilemap", 0 )
	ROM_LOAD( "2.u24", 0x10000, 0x8000, CRC(f1181b93) SHA1(53cd4d2ce13973495b51d911a4745a69a9784983) )
	ROM_LOAD( "3.u25", 0x08000, 0x8000, CRC(437abb27) SHA1(bd3790807d60a41d58e07f60fb990553076d6e96) )
	ROM_LOAD( "4.u26", 0x00000, 0x8000, CRC(e2f66eee) SHA1(ece924fe626f21fd7d31faabf19225d80e2bcfd3) )

	ROM_REGION( 0x40000, "oki", ROMREGION_ERASE )
	// rom socket is empty

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "74s287.u35", 0x000, 0x100, CRC(40094bed) SHA1(b25d96126b3f7bd06bf76dc9958f8669f83abdb7) )
	ROM_LOAD( "74s287.u36", 0x100, 0x100, CRC(9ca021c5) SHA1(6a1d8d4f958d36e4a676dc4f4aee83d637933bc3) )
ROM_END

/***************************************************************************

  Treasure Island
  -- this has an extra layer for the reels, exactly the same as goldstar.c

***************************************************************************/

ROM_START( tisub )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "rom_1.bin", 0x10000, 0x4000,  CRC(ed3b4a69) SHA1(c57985e8d19b2b495fc768e52b83cbbd75f027ad) )
	ROM_CONTINUE(0x0000,0xc000)
	ROM_COPY( "maincpu", 0x09000, 0x14000, 0x1000)

	ROM_REGION( 0x40000, "tilemap", 0 )
	ROM_LOAD( "rom_6.bin", 0x00000, 0x10000, CRC(c2c226df) SHA1(39762b390d6b271c3252342e843a181dd152a0cc) )
	ROM_LOAD( "rom_4.bin", 0x10000, 0x10000, CRC(37724fda) SHA1(084653662c9f77afef2a77c607e1fb093aaf3adf) )
	ROM_LOAD( "rom_5.bin", 0x20000, 0x10000, CRC(3d18acd8) SHA1(179545c18ad880097366c07c8e2fa821701a2758) )
	ROM_LOAD( "rom_7.bin", 0x30000, 0x10000, CRC(9d7d99d8) SHA1(a3df5e023c2102028a5186101dc0b19d91e8965e) )

	ROM_REGION( 0x8000, "reels", 0 )
	ROM_LOAD( "rom_2.bin", 0x0000, 0x4000, CRC(836c756d) SHA1(fca1d5b600861eea30ed73ee13be735e7d167097) )
	ROM_IGNORE(0x4000)
	ROM_LOAD( "rom_3.bin", 0x4000, 0x4000, CRC(2ad82222) SHA1(68780b9528393b28eaa2f90501efb5a8c39bed63) )
	ROM_IGNORE(0x4000)

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "n82s129n.u39", 0x000, 0x100, CRC(971843e5) SHA1(4cb5fc1085503dae2f2f02eb49cca051ac84b890) )
	ROM_LOAD( "n82s129n.u40", 0x100, 0x100, CRC(b4bd872c) SHA1(c0f9fe68186636d6d6bc6f81415459631cf38edd) )
	ROM_LOAD( "n82s129n.u41", 0x200, 0x100, CRC(db99f6da) SHA1(d281a2fa06f1890ef0b1c4d099e6828827db14fd) )
ROM_END

/*

  Treasure Island (Alt version)...

  ROMs 4 & 5 are missing. ROMs 6 & 7 are identical to parent set.
  So... Assuming that 4 & 5 should have the same bitplanes.

  ROM 3 is bad, but ROM 2 only has the first byte different,
  getting different values in each dump. The rest remains identical.

  Program ROM is different.

  Color PROMs are from this set.

*/

ROM_START( tisuba )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "01.bin", 0x10000, 0x4000,  CRC(9967dd38) SHA1(63b74bc0c0952114b7321e8f399bd64dc293aade) )
	ROM_CONTINUE(0x0000,0xc000)
	ROM_COPY( "maincpu", 0x09000, 0x14000, 0x1000)

	ROM_REGION( 0x40000, "tilemap", 0 )
	ROM_LOAD( "rom_6.bin", 0x00000, 0x10000, CRC(c2c226df) SHA1(39762b390d6b271c3252342e843a181dd152a0cc) )
	ROM_LOAD( "rom_4.bin", 0x10000, 0x10000, CRC(37724fda) SHA1(084653662c9f77afef2a77c607e1fb093aaf3adf) )
	ROM_LOAD( "rom_5.bin", 0x20000, 0x10000, CRC(3d18acd8) SHA1(179545c18ad880097366c07c8e2fa821701a2758) )
	ROM_LOAD( "rom_7.bin", 0x30000, 0x10000, CRC(9d7d99d8) SHA1(a3df5e023c2102028a5186101dc0b19d91e8965e) )

	ROM_REGION( 0x8000, "reels", 0 )
	ROM_LOAD( "rom_2.bin", 0x0000, 0x4000, CRC(836c756d) SHA1(fca1d5b600861eea30ed73ee13be735e7d167097) )
	ROM_IGNORE(0x4000)
	ROM_LOAD( "rom_3.bin", 0x4000, 0x4000, CRC(2ad82222) SHA1(68780b9528393b28eaa2f90501efb5a8c39bed63) )
	ROM_IGNORE(0x4000)

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "n82s129n.u39", 0x000, 0x100, CRC(971843e5) SHA1(4cb5fc1085503dae2f2f02eb49cca051ac84b890) )
	ROM_LOAD( "n82s129n.u40", 0x100, 0x100, CRC(b4bd872c) SHA1(c0f9fe68186636d6d6bc6f81415459631cf38edd) )
	ROM_LOAD( "n82s129n.u41", 0x200, 0x100, CRC(db99f6da) SHA1(d281a2fa06f1890ef0b1c4d099e6828827db14fd) )
ROM_END

/***************************************************************************

Cross Bingo
(C)1991 Subsino

Chips:

1x big unknown epoxy block (main)
1x unknown Subsino SS9101-173001 (DIP42)
1x unknown blank quad chip (QFP68)
1x UM3567 (sound)
3x ULN2003AN (sound)
1x LM324N (sound)
1x oscillator 12.000
1x oscillator 3.579545

ROMs:

3x M27512
2x PROM N82S129N
2x PLD 18CV8PC (read protected)

Other:

1x 22x2 edge connector
1x 11x2 edge connector
1x 10x2 edge connector (payout system)
1x RS232 connector
1x trimmer (volume)
1x pushbutton
4x 8x2 switches dip
1x battery

Info by f205v (14/12/2008)

***************************************************************************/

ROM_START( crsbingo )
	ROM_REGION( 0x14000, "maincpu", 0 )
	ROM_LOAD( "1.u36", 0x10000, 0x4000, CRC(c5aff4eb) SHA1(74f06d7735975657fca9be5fff9e7d53f38fcd02) )
	ROM_CONTINUE(0x0000,0xc000)

	ROM_REGION( 0x20000, "tilemap", 0 )
	ROM_LOAD( "2.u4",  0x00000, 0x10000, CRC(ce527722) SHA1(f3759cefab902259eb25f8d4be2fcafc1afd90b9) )
	ROM_LOAD( "3.u15", 0x10000, 0x10000, CRC(23785451) SHA1(8574e59aa816a644ff4b102bd5754ef1284deea0) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s129.u13", 0x000, 0x100, CRC(89c06859) SHA1(b98d5335f36ea3842086677aca47b605030d442f) )
	ROM_LOAD( "82s129.u14", 0x100, 0x100, CRC(eaddb54f) SHA1(51fbf31e910a93315204a892d10bcf982a6ed099) )

	ROM_REGION( 0x40000, "oki", ROMREGION_ERASE00 )

	ROM_REGION( 0x155 * 2, "plds", 0 )
	ROM_LOAD( "18cv8.u22", 0x000, 0x155, BAD_DUMP CRC(996e8f59) SHA1(630d9b91f6e8eda781061e2a8ff6fb0fecaf034c) )
	ROM_LOAD( "18cv8.u29", 0x155, 0x155, BAD_DUMP CRC(996e8f59) SHA1(630d9b91f6e8eda781061e2a8ff6fb0fecaf034c) )
ROM_END

/***************************************************************************

Shark Party
(C)1993 Subsino

Chips:

1x unknown big black box
1x custom SUBSINO_SS9101_409235I (DIL42)(u48)
2x KD89C55 (u49,u50)
1x K-665 (u55)(equivalent to M6295)
1x K-664 (u57)(equivalent to YM3014)
1x K-666 (u52)(equivalent to YM3812)
3x 45580D (amplifier)(u58,u59,u60)
2x custom SUBSINO_SS9100_3512204V (SMT 44pins)(u10,u19)
1x oscillator 4.433619MHz
1x oscillator 12.000MHz

ROMs:

1x 27C1001 (u54)
1x 27C512 (u18)
2x 27C010 (u16,u17)
3x N82S129AN (u11,u12,u13)
4x GAL16V8B (u2,u37,u45,u46)(not dumped)
2x TIBPAL16L8 (u43,u44)(not dumped)

Other:

1x 36x2 edge connector (con5)
1x 10x2 edge connector (con4)
1x battery
1x trimmer (volume)
1x pushbutton (sw5)
4x 8x2 switches dip (sw1,sw2,sw3,sw4)

Info by f205v (25/03/2008)

***************************************************************************/

ROM_START( sharkpy )
	ROM_REGION( 0x14000, "maincpu", 0 )
	ROM_LOAD( "shark_n.1.u18", 0x0a000, 0x6000, CRC(25aeac2f) SHA1(d94e3e5cfffd150ac48e1463493a8323f42e7a89) ) // is this mapped correctly? - used during gameplay?
	ROM_CONTINUE(0x0000, 0xa000)

	ROM_REGION( 0x40000, "tilemap", 0 )
	ROM_LOAD( "shark_n.3.u16", 0x00000, 0x08000, CRC(a7a715ce) SHA1(38b93e05377d9cb816688f5070e847480f195c6b) )
	ROM_CONTINUE(              0x10000, 0x08000 )
	ROM_CONTINUE(              0x08000, 0x08000 )
	ROM_CONTINUE(              0x18000, 0x08000 )
	ROM_LOAD( "shark_n.2.u17", 0x20000, 0x08000, CRC(c27f3d0a) SHA1(77c8eb0322c5b9c89777cb080d26ecf9abe01ae7) )
	ROM_CONTINUE(              0x30000, 0x08000 )
	ROM_CONTINUE(              0x28000, 0x08000 )
	ROM_CONTINUE(              0x38000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "shark(ii)-italy_4_ver1.0.u54", 0x00000, 0x20000, CRC(9f384c59) SHA1(d2b087b8370b40b6f0944de661ea6aebaebea06f) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "n82s129an.u11", 0x000, 0x100, CRC(daf3657a) SHA1(93005938e2d60d54e7bbf1e234bba3802ee1af21) )
	ROM_LOAD( "n82s129an.u12", 0x100, 0x100, CRC(5a7a25ed) SHA1(eebd679195e6ea50f64f3c46cd06ee21a1550491) )
	ROM_LOAD( "n82s129an.u13", 0x200, 0x100, CRC(0ef5f218) SHA1(a02cf266661385aa078563bd83240d36549c1cf0) )
ROM_END

/***************************************************************************

Shark Party (alt)
(C)1993 Subsino

Chips:

1x unknown big black box
1x custom SUBSINO_SS9101_409235I (DIL42)(u48)
2x KD89C55 (u49,u50)
1x K-665 (u55)(equivalent to M6295)
1x K-664 (u57)(equivalent to YM3014)
1x SM64JBCK (u52)(equivalent to YM3812)
3x 45580D (amplifier)(u58,u59,u60)
2x custom SUBSINO_SS9100_3512201V (SMT 44pins)(u10,u19)
1x oscillator 4.433619MHz
1x oscillator 12.000MHz

ROMs:

2x 27C1001 (u54,u17)
1x 27C512 (u18)
1x 27C010 (u16)
3x N82S129AN (u11,u12,u13)
4x GAL16V8B (u2,u37,u45,u46)(not dumped)
2x TIBPAL16L8 (u43,u44)(not dumped)

Other:

1x 36x2 edge connector (con5)
1x 10x2 edge connector (con4)
1x battery
1x trimmer (volume)
1x pushbutton (sw5)
4x 8x2 switches dip (sw1,sw2,sw3,sw4)

Info by f205v (25/03/2008)

***************************************************************************/

ROM_START( sharkpya )
	ROM_REGION( 0x14000, "maincpu", 0 )
	ROM_LOAD( "shark1.6.u18", 0x0a000, 0x6000, CRC(365312a0) SHA1(de8370b1f35e8d071185d2e5f2fbd2fdf74c55ac) )
	ROM_CONTINUE(0x0000, 0xa000)

	ROM_REGION( 0x40000, "tilemap", 0 )
	ROM_LOAD( "shark_n.3.u16", 0x00000, 0x08000, CRC(a7a715ce) SHA1(38b93e05377d9cb816688f5070e847480f195c6b) )
	ROM_CONTINUE(              0x10000, 0x08000 )
	ROM_CONTINUE(              0x08000, 0x08000 )
	ROM_CONTINUE(              0x18000, 0x08000 )
	ROM_LOAD( "shark_n.2.u17", 0x20000, 0x08000, CRC(c27f3d0a) SHA1(77c8eb0322c5b9c89777cb080d26ecf9abe01ae7) )
	ROM_CONTINUE(              0x30000, 0x08000 )
	ROM_CONTINUE(              0x28000, 0x08000 )
	ROM_CONTINUE(              0x38000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "shark(ii)-italy_4_ver1.0.u54", 0x00000, 0x20000, CRC(9f384c59) SHA1(d2b087b8370b40b6f0944de661ea6aebaebea06f) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "sn82s129an.u11", 0x000, 0x100, CRC(daf3657a) SHA1(93005938e2d60d54e7bbf1e234bba3802ee1af21) )
	ROM_LOAD( "sn82s129an.u12", 0x100, 0x100, CRC(5a7a25ed) SHA1(eebd679195e6ea50f64f3c46cd06ee21a1550491) )
	ROM_LOAD( "sn82s129an.u13", 0x200, 0x100, CRC(0ef5f218) SHA1(a02cf266661385aa078563bd83240d36549c1cf0) )
ROM_END


/***************************************************************************

  Shark Party (English, Alpha license)

  - Different inputs system.
  - Different DIP Switches.
  - Different Button-Lamps outputs.

***************************************************************************/

ROM_START( sharkpye )
	ROM_REGION( 0x14000, "maincpu", 0 )
	ROM_LOAD( "sharkpye.u18", 0x0a000, 0x6000, CRC(12473814) SHA1(9c24ed41781aefee0161add912e730ba0d4f4d3e) )
	ROM_CONTINUE(0x0000, 0xa000)

	ROM_REGION( 0x40000, "tilemap", 0 )
	ROM_LOAD( "sharkpye.u16", 0x00000, 0x08000, CRC(90862185) SHA1(9d632bfa707d3449a87d7f370eb2b5c36e61aadd) )
	ROM_CONTINUE(             0x10000, 0x08000 )
	ROM_CONTINUE(             0x08000, 0x08000 )
	ROM_CONTINUE(             0x18000, 0x08000 )
	ROM_LOAD( "sharkpye.u17", 0x20000, 0x08000, CRC(b7b6119a) SHA1(b61c77d2170d96fcb39ea31c4136387441b9037f) )
	ROM_CONTINUE(             0x30000, 0x08000 )
	ROM_CONTINUE(             0x28000, 0x08000 )
	ROM_CONTINUE(             0x38000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "sharkpye.u54", 0x00000, 0x20000, CRC(9f384c59) SHA1(d2b087b8370b40b6f0944de661ea6aebaebea06f) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "n82s129an.u11", 0x000, 0x100, CRC(daf3657a) SHA1(93005938e2d60d54e7bbf1e234bba3802ee1af21) )
	ROM_LOAD( "n82s129an.u12", 0x100, 0x100, CRC(5a7a25ed) SHA1(eebd679195e6ea50f64f3c46cd06ee21a1550491) )
	ROM_LOAD( "n82s129an.u13", 0x200, 0x100, CRC(0ef5f218) SHA1(a02cf266661385aa078563bd83240d36549c1cf0) )
ROM_END


/****************************************************************************

  Victor 6 (Subsino/Alpha)

  SET        MAINRATE  MAXBET
  ----------------------------
  victor6    87-101%    50
  victor6a   94-101%    80
  victor6b   94-101%    80 (no 10 option)


****************************************************************************/

ROM_START( victor6 )
	ROM_REGION( 0x14000, "maincpu", 0 )
	ROM_LOAD( "victor_6ii-rom_1.ver2.3n.u18", 0x0a000, 0x6000, CRC(d496ecbd) SHA1(1f982b42bc46c09298916a6cb2db0b38c6451ec3) )
	ROM_CONTINUE(0x0000, 0xa000)

	ROM_REGION( 0x40000, "tilemap", 0 )
	ROM_LOAD( "victor_6ii-rom_3_ver1.0.u16", 0x00000, 0x08000, CRC(4e96c30a) SHA1(4989b10a52ba61459864aa44be9ebafe68b4d231) )
	ROM_CONTINUE(             0x10000, 0x08000 )
	ROM_CONTINUE(             0x08000, 0x08000 )
	ROM_CONTINUE(             0x18000, 0x08000 )
	ROM_LOAD( "victor_6ii-rom_2_ver1.0.u17", 0x20000, 0x08000, CRC(4630a1da) SHA1(a14df7d7047350a7b1ae485570869d9fa50a2f6d) )
	ROM_CONTINUE(             0x30000, 0x08000 )
	ROM_CONTINUE(             0x28000, 0x08000 )
	ROM_CONTINUE(             0x38000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "victor_6ii-rom_4_ver1.0.u54", 0x00000, 0x20000, CRC(ed2a6ff8) SHA1(b776b85a350cd0176ffa04248084475d07ac5bfa) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "n82s129.u11", 0x000, 0x100, CRC(e8d7c8c3) SHA1(26ea907e45e70269956f842817b2d827cbc647ec) )
	ROM_LOAD( "n82s129.u12", 0x100, 0x100, CRC(4cee9225) SHA1(bb784ff636f90de3965272021f610abb41e0d40d) )
	ROM_LOAD( "n82s129.u13", 0x200, 0x100, CRC(b135c3eb) SHA1(54b04c5c4eb3a769123f2630740f0575e2ea6ff2) )
ROM_END

ROM_START( victor6a )
	ROM_REGION( 0x14000, "maincpu", 0 )
	ROM_LOAD( "victor_6ii_alpha_1_ver2.3.u18", 0x0a000, 0x6000, CRC(2a3eaecd) SHA1(18bf2dfec8cd5690d6465f750093942afda66475) )
	ROM_CONTINUE(0x0000, 0xa000)

	ROM_REGION( 0x40000, "tilemap", 0 )
	ROM_LOAD( "victor_6ii-rom_3_ver1.0.u16", 0x00000, 0x08000, CRC(4e96c30a) SHA1(4989b10a52ba61459864aa44be9ebafe68b4d231) )
	ROM_CONTINUE(             0x10000, 0x08000 )
	ROM_CONTINUE(             0x08000, 0x08000 )
	ROM_CONTINUE(             0x18000, 0x08000 )
	ROM_LOAD( "victor_6ii-rom_2_ver1.0.u17", 0x20000, 0x08000, CRC(4630a1da) SHA1(a14df7d7047350a7b1ae485570869d9fa50a2f6d) )
	ROM_CONTINUE(             0x30000, 0x08000 )
	ROM_CONTINUE(             0x28000, 0x08000 )
	ROM_CONTINUE(             0x38000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "victor_6ii-rom_4_ver1.0.u54", 0x00000, 0x20000, CRC(ed2a6ff8) SHA1(b776b85a350cd0176ffa04248084475d07ac5bfa) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "n82s129.u11", 0x000, 0x100, CRC(e8d7c8c3) SHA1(26ea907e45e70269956f842817b2d827cbc647ec) )
	ROM_LOAD( "n82s129.u12", 0x100, 0x100, CRC(4cee9225) SHA1(bb784ff636f90de3965272021f610abb41e0d40d) )
	ROM_LOAD( "n82s129.u13", 0x200, 0x100, CRC(b135c3eb) SHA1(54b04c5c4eb3a769123f2630740f0575e2ea6ff2) )
ROM_END

ROM_START( victor6b )
	ROM_REGION( 0x14000, "maincpu", 0 )
	ROM_LOAD( "victor_6ii_rom_1_ver1.2.u18", 0x0a000, 0x6000, CRC(309876fc) SHA1(305c4cf347b512607e2c58a580075a34b48bedd5) )
	ROM_CONTINUE(0x0000, 0xa000)

	ROM_REGION( 0x40000, "tilemap", 0 )
	ROM_LOAD( "victor_6ii-rom_3_ver1.0.u16", 0x00000, 0x08000, CRC(4e96c30a) SHA1(4989b10a52ba61459864aa44be9ebafe68b4d231) )
	ROM_CONTINUE(             0x10000, 0x08000 )
	ROM_CONTINUE(             0x08000, 0x08000 )
	ROM_CONTINUE(             0x18000, 0x08000 )
	ROM_LOAD( "victor_6ii-rom_2_ver1.0.u17", 0x20000, 0x08000, CRC(4630a1da) SHA1(a14df7d7047350a7b1ae485570869d9fa50a2f6d) )
	ROM_CONTINUE(             0x30000, 0x08000 )
	ROM_CONTINUE(             0x28000, 0x08000 )
	ROM_CONTINUE(             0x38000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "victor_6ii-rom_4_ver1.0.u54", 0x00000, 0x20000, CRC(ed2a6ff8) SHA1(b776b85a350cd0176ffa04248084475d07ac5bfa) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "n82s129.u11", 0x000, 0x100, CRC(e8d7c8c3) SHA1(26ea907e45e70269956f842817b2d827cbc647ec) )
	ROM_LOAD( "n82s129.u12", 0x100, 0x100, CRC(4cee9225) SHA1(bb784ff636f90de3965272021f610abb41e0d40d) )
	ROM_LOAD( "n82s129.u13", 0x200, 0x100, CRC(b135c3eb) SHA1(54b04c5c4eb3a769123f2630740f0575e2ea6ff2) )
ROM_END



/***************************************************************************

Super Rider (Italy Ver 1.6)
(C)1996 Subsino

Chips:

2x custom QFP44 label SUBSINOSS9100
1x custom DIP42 label SUBSINOSS9101
2x FILE KD89C55A (equivalent to 8255)
1x custom QFP44 label M28 (sound)(equivalent to M6295)
1x custom DIP24 label K-666 (sound)(equivalent to YM3812)
1x custom DIP8 label K-664 (sound)(equivalent to YM3014)
1x oscillator 12.000MHz (main)
1x oscillator 4.43361MHz (sound)

ROMs:

1x TMS27C512 (1)
2x TMS27C010A (2,3)(main)
1x TMS27C010A (4) (sound)
3x PROM N82S129AN

Other:

1x 10x2 edge connector (looks like a coin payout)
1x 36x2 edge connector
1x battery 3.6V NiCd
1x pushbutton (sw5)
4x 8 switches dips (sw1-4)
1x trimmer (volume)
1x BIG BLACK BOX (on top of the box there is a small door closing a button-battery; for sure there is more in it, but I do not know how to open it / tore it apart)

This game is the official Italian version of "Super Rider" by Subsino

Info by f205v (29/12/2005)

***************************************************************************/

ROM_START( smoto16 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rideritaly_1ver1.6.u18", 0x0000, 0x10000, CRC(c7c0c3e8) SHA1(5dc80bc775f370653135a7b3ea9c8d3c92263804) )

	ROM_REGION( 0x40000, "tilemap", 0 )
	ROM_LOAD( "rideritaly_3ver1.6.u16", 0x00000, 0x08000, CRC(998a8feb) SHA1(27f08b23f2dd3736f4f12f489d9a3aa096c99e8a) )
	ROM_CONTINUE(                       0x10000, 0x08000 )
	ROM_CONTINUE(                       0x08000, 0x08000 )
	ROM_CONTINUE(                       0x18000, 0x08000 )
	ROM_LOAD( "rideritaly_2ver1.6.u17", 0x20000, 0x08000, CRC(bdf9bf26) SHA1(49e7c0b99fec06dca5816eb7e38aed025efcaaa7) )
	ROM_CONTINUE(                       0x30000, 0x08000 )
	ROM_CONTINUE(                       0x28000, 0x08000 )
	ROM_CONTINUE(                       0x38000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "rideritaly_4ver1.6.u54", 0x00000, 0x20000, CRC(df828563) SHA1(f39324c5c37486ed9512e0ff934394556dd182ae) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "prom-n82s129an.u11", 0x000, 0x100, CRC(e17730a6) SHA1(50c730b24e1d3d205c70f9381e4136e2ba6e499a) )
	ROM_LOAD( "prom-n82s129an.u12", 0x100, 0x100, CRC(df848861) SHA1(f7e382f8b56d6b9f2af6c7a48a19e3631a64bb6d) )
	ROM_LOAD( "prom-n82s129an.u13", 0x200, 0x100, CRC(9cb4a5c0) SHA1(0e0a368329c6d1cb685ed655d699a4894988fdb1) )
ROM_END

DRIVER_INIT_MEMBER(subsino_state,smoto16)
{
	UINT8 *rom = memregion( "maincpu" )->base();
	rom[0x12d0] = 0x20; // "ERROR 951010"
}

/***************************************************************************

Super Rider (Italy Ver 2.0)
(C)1997 Subsino

Chips:

2x custom QFP44 label SUBSINOSS9100
1x custom DIP42 label SUBSINOSS9101
2x D8255AC-2 (are they 8255 equivalent?)
1x custom QFP44 label K-665 (sound)(equivalent to OKI M6295)
1x custom DIP24 label SM64 (sound)(equivalent to YM3812)
1x custom DIP8 label K-664 (sound)(equivalent to YM3014)
1x oscillator 12.000MHz (main)
1x oscillator 4.433619MHz (sound)

ROMs:

1x 27C512 (1)
2x M27C1001 (2,3)(main)
1x M27C1001 (4) (sound)
3x PROM N82S129AN
3x PALCE16V8H (not dumped)
2x TIBPAL16L8B (not dumped)
1x GAL16V8B (not dumped)

Other:

1x 10x2 edge connector (looks like a coin payout)
1x 36x2 edge connector
1x battery 3.6V NiCd
4x 8 switches dips (sw1-4)
1x trimmer (volume)
1x BIG BLACK BOX (on top of the box there is a small door closing a button-battery; for sure there is more in it, but I do not know how to open it / tore it apart)

This game is the official Italian version of "Super Rider" by Subsino

Info by f205v, Corrado Tomaselli (20/04/2008)

***************************************************************************/

ROM_START( smoto20 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "italyv2.0-25.u18", 0x00000, 0x10000, CRC(91abc76e) SHA1(b0eb3afda1d94111056559017802b16b2e72a9a5) )

	ROM_REGION( 0x40000, "tilemap", 0 )
	ROM_LOAD( "3.u16", 0x00000, 0x08000, CRC(44b44385) SHA1(27c2865e52ab67aa8e077e8e1202cbf2addc0dfc) )
	ROM_CONTINUE(      0x10000, 0x08000 )
	ROM_CONTINUE(      0x08000, 0x08000 )
	ROM_CONTINUE(      0x18000, 0x08000 )
	ROM_LOAD( "2.u17", 0x20000, 0x08000, CRC(380fc964) SHA1(4a5076d90cb94e2ffeec7534ce64d4cdb320f374) )
	ROM_CONTINUE(      0x30000, 0x08000 )
	ROM_CONTINUE(      0x28000, 0x08000 )
	ROM_CONTINUE(      0x38000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "rom4ver1.0.u54", 0x00000, 0x20000, CRC(df828563) SHA1(f39324c5c37486ed9512e0ff934394556dd182ae) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "82s129.u11", 0x000, 0x100, CRC(e17730a6) SHA1(50c730b24e1d3d205c70f9381e4136e2ba6e499a) )
	ROM_LOAD( "82s129.u12", 0x100, 0x100, CRC(df848861) SHA1(f7e382f8b56d6b9f2af6c7a48a19e3631a64bb6d) )
	ROM_LOAD( "82s129.u13", 0x200, 0x100, CRC(9cb4a5c0) SHA1(0e0a368329c6d1cb685ed655d699a4894988fdb1) )
ROM_END

/***************************************************************************

   Treasure Bonus
   (C) American Alpha

   CPU module marked 'Super Treasure Island'

***************************************************************************/

ROM_START( stbsub )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "trbon-rlu16.u12", 0x00000, 0x10000, CRC(07771290) SHA1(c485943045396d8580271504a1fec7c88579f4a2) )

	ROM_REGION( 0x100000, "tilemap", 0 )
	ROM_LOAD( "sti-alpha_2-ver1.1.u30", 0x00000, 0x40000, CRC(3bc4c8c5) SHA1(12e868f4b4d4df6b59befcd785ab1fe5c1def58d) )
	ROM_LOAD( "sti-alpha_3-ver1.1.u29", 0x40000, 0x40000, CRC(5473c41a) SHA1(94294887af8ffc4f2edbcbde1c51797f20c44efe) )
	ROM_LOAD( "sti-alpha_4-ver1.1.u28", 0x80000, 0x40000, CRC(ccf895e1) SHA1(c12ecf0577b5b856d8202474f084003cc95da51c) )
	ROM_LOAD( "sti-alpha_5-ver1.1.u27", 0xc0000, 0x40000, CRC(98eed855) SHA1(89291b1b143924caa79a6d694f10c14d93c57eac) )

	ROM_REGION( 0x80000, "reels", 0 )
	ROM_LOAD( "sti-alpha_6-ver1.1.u25", 0x00000, 0x20000, CRC(83471a70) SHA1(c63e4c1a8cfb6e7feae4fd97f7d77feaf63c949b) )
	ROM_LOAD( "sti-alpha_7-ver1.1.u24", 0x20000, 0x20000, CRC(05bc7ed2) SHA1(23ae716cd149ee940ac4bdc114fbfeb290e91b11) )
	ROM_LOAD( "sti-alpha_8-ver1.1.u23", 0x40000, 0x20000, CRC(d3c11545) SHA1(0383358d223c9bfe67c3b5de7a9cc3e43a9769b2) )
	ROM_LOAD( "sti-alpha_9-ver1.1.u22", 0x60000, 0x20000, CRC(9710a223) SHA1(76ef6bd77ae33d91a9b6a9a615d07caee3356dfb) )
ROM_END



ROM_START( stisub )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b1", 0x00000, 0x10000, CRC(3f7adf66) SHA1(6ff37d070c7866133853c7cb3e2fbcb5610d87e8) )

	ROM_REGION( 0x100000, "tilemap", 0 )
	ROM_LOAD( "-2.u30",  0x00000, 0x40000, CRC(60596c9c) SHA1(6cea104539957bacb857bb14f967633e8cd729c0) )
	ROM_LOAD( "b-3.u29", 0x40000, 0x40000, CRC(eb0968d3) SHA1(5313150725d9b7019ddaddc0b1cdb92330ab0b49) )
	ROM_LOAD( "b-4.u28", 0x80000, 0x40000, CRC(ee5024ba) SHA1(cf65bbee12f6aaf8bb22c2a03e7b360fa58f3b80) )
	ROM_LOAD( "a-5.u27", 0xc0000, 0x40000, CRC(6748c76d) SHA1(1013f5924c584df4bd6a1a3dbd0fff96c1313ed3) )

	ROM_REGION( 0x80000, "reels", 0 )
	ROM_LOAD( "a-6.u25", 0x00000, 0x20000, CRC(69a19c43) SHA1(d90a59bfee500ea9b1a21f60bc2fd7c3ddadb6a6) )
	ROM_LOAD( "b-7.u24", 0x20000, 0x20000, CRC(09173bec) SHA1(c9bf491a9d4009d1debf7a19657129a209f02768) )
	ROM_LOAD( "b-8.u23", 0x40000, 0x20000, CRC(10ff8fdf) SHA1(1f07ce5517c816852e5b739e3170d104c080ea18) )
	ROM_LOAD( "a-9.u22", 0x60000, 0x20000, CRC(ce1e9a3d) SHA1(263e396058e74ae55834dc028b477eb21ceab9b9) )
ROM_END



/***************************************************************************

Tesorone Dell'Isola (2 sets)
(C) Subsino

Italian version of "Treasure Bonus"

PCB: SN01256-3 CS186P006-1 (same as "Treasure Bonus")

Chips:

1x pLSI 1032-60
2x FILE KD89C55A (equivalent to 8255)
1x K-664 (equivalent to YM3014)
1x K-665 (equivalent to M6295)
1x K-666 (equivalent to YM3812)
1x custom DIP42 SUBSINO SS9101
1x HMC HM86171-80 (RAMDAC)

2x oscillator 12.000MHz ?
1x oscillator 4.43361MHz ?

Other:

1x empty ROM socket for upgrades
1x battery (unpopulated)
1x 6x2 edge connector (con2)
1x 36x2 edge connector
1x pushbutton (sw5)
4x 8 switches dips (sw1-4)
1x trimmer (volume)
1x BIG BLACK BOX

***************************************************************************/

ROM_START( tesorone )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tesorone.d.isol.italy_1ver2.41.u12", 0x00000, 0x10000, CRC(b019b689) SHA1(ba7acd15842b29e6ac37795a4d6e0f93d99393a4) )

	ROM_REGION( 0x100000, "tilemap", 0 )
	ROM_LOAD( "tesorone.d.isol.italy_2ver1.7.u30", 0x00000, 0x40000, CRC(295887c5) SHA1(b36914977b276ac5e5e31902dff28796f3a28ea1) )
	ROM_LOAD( "tesorone.d.isol.italy_3ver1.7.u29", 0x40000, 0x40000, CRC(89469522) SHA1(ba373900e0310aad3d04ff58909f6144d9b689a7) )
	ROM_LOAD( "tesorone.d.isol.italy_4ver1.7.u28", 0x80000, 0x40000, CRC(2092a368) SHA1(05e1af15761e0186ea7ddb8b82c177e35fcdd382) )
	ROM_LOAD( "tesorone.d.isol.italy_5ver1.7.u27", 0xc0000, 0x40000, CRC(57870bad) SHA1(7a3342c5cc3ed5f48d2dda224913eb357aeb401b) )

	ROM_REGION( 0x80000, "reels", 0 )
	ROM_LOAD( "tesorone.d.isol.italy_6ver1.7.u25", 0x00000, 0x20000, CRC(e5578d00) SHA1(28882131d13f052bc31c3fc1b6dc5d9e45d30e82) )
	ROM_LOAD( "tesorone.d.isol.italy_7ver1.7.u24", 0x20000, 0x20000, CRC(c29a7841) SHA1(7bec4a4db0b545b9b9d9a4c14efa9442e7738d8a) )
	ROM_LOAD( "tesorone.d.isol.italy_8ver1.7.u23", 0x40000, 0x20000, CRC(2b4b195a) SHA1(cb165f6737231ae52dbf9775fff13b778835fcac) )
	ROM_LOAD( "tesorone.d.isol.italy_9ver1.7.u22", 0x60000, 0x20000, CRC(1c9f754e) SHA1(7b2feeeaaa4845d2fcfebb2c1bc4d6b69d937400) )
ROM_END

ROM_START( tesorone230 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tesorone.d.isol.italy_1ver2.3.u12", 0x00000, 0x10000, CRC(46cd019b) SHA1(40412ac1234ae0f31b13c4d3b48681da34f1ded9) )

	ROM_REGION( 0x100000, "tilemap", 0 )
	ROM_LOAD( "tesorone.d.isol.italy_2ver1.7.u30", 0x00000, 0x40000, CRC(295887c5) SHA1(b36914977b276ac5e5e31902dff28796f3a28ea1) )
	ROM_LOAD( "tesorone.d.isol.italy_3ver1.7.u29", 0x40000, 0x40000, CRC(89469522) SHA1(ba373900e0310aad3d04ff58909f6144d9b689a7) )
	ROM_LOAD( "tesorone.d.isol.italy_4ver1.7.u28", 0x80000, 0x40000, CRC(2092a368) SHA1(05e1af15761e0186ea7ddb8b82c177e35fcdd382) )
	ROM_LOAD( "tesorone.d.isol.italy_5ver1.7.u27", 0xc0000, 0x40000, CRC(57870bad) SHA1(7a3342c5cc3ed5f48d2dda224913eb357aeb401b) )

	ROM_REGION( 0x80000, "reels", 0 )
	ROM_LOAD( "tesorone.d.isol.italy_6ver1.7.u25", 0x00000, 0x20000, CRC(e5578d00) SHA1(28882131d13f052bc31c3fc1b6dc5d9e45d30e82) )
	ROM_LOAD( "tesorone.d.isol.italy_7ver1.7.u24", 0x20000, 0x20000, CRC(c29a7841) SHA1(7bec4a4db0b545b9b9d9a4c14efa9442e7738d8a) )
	ROM_LOAD( "tesorone.d.isol.italy_8ver1.7.u23", 0x40000, 0x20000, CRC(2b4b195a) SHA1(cb165f6737231ae52dbf9775fff13b778835fcac) )
	ROM_LOAD( "tesorone.d.isol.italy_9ver1.7.u22", 0x60000, 0x20000, CRC(1c9f754e) SHA1(7b2feeeaaa4845d2fcfebb2c1bc4d6b69d937400) )
ROM_END

ROM_START( tesorone240 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tesorone.d.isol.italy_1ver2.4.u12", 0x00000, 0x10000, CRC(6a7d5395) SHA1(448184b78b6a3e28f891731c83a4e2d1e283c205) )

	ROM_REGION( 0x100000, "tilemap", 0 )
	ROM_LOAD( "tesorone.d.isol.italy_2ver1.7.u30", 0x00000, 0x40000, CRC(295887c5) SHA1(b36914977b276ac5e5e31902dff28796f3a28ea1) )
	ROM_LOAD( "tesorone.d.isol.italy_3ver1.7.u29", 0x40000, 0x40000, CRC(89469522) SHA1(ba373900e0310aad3d04ff58909f6144d9b689a7) )
	ROM_LOAD( "tesorone.d.isol.italy_4ver1.7.u28", 0x80000, 0x40000, CRC(2092a368) SHA1(05e1af15761e0186ea7ddb8b82c177e35fcdd382) )
	ROM_LOAD( "tesorone.d.isol.italy_5ver1.7.u27", 0xc0000, 0x40000, CRC(57870bad) SHA1(7a3342c5cc3ed5f48d2dda224913eb357aeb401b) )

	ROM_REGION( 0x80000, "reels", 0 )
	ROM_LOAD( "tesorone.d.isol.italy_6ver1.7.u25", 0x00000, 0x20000, CRC(e5578d00) SHA1(28882131d13f052bc31c3fc1b6dc5d9e45d30e82) )
	ROM_LOAD( "tesorone.d.isol.italy_7ver1.7.u24", 0x20000, 0x20000, CRC(c29a7841) SHA1(7bec4a4db0b545b9b9d9a4c14efa9442e7738d8a) )
	ROM_LOAD( "tesorone.d.isol.italy_8ver1.7.u23", 0x40000, 0x20000, CRC(2b4b195a) SHA1(cb165f6737231ae52dbf9775fff13b778835fcac) )
	ROM_LOAD( "tesorone.d.isol.italy_9ver1.7.u22", 0x60000, 0x20000, CRC(1c9f754e) SHA1(7b2feeeaaa4845d2fcfebb2c1bc4d6b69d937400) )
ROM_END


/***************************************************************************

  This is allegedly Magic Train - Clear NVRAM ROM:

  Subsino sold a "Settings/Clear ROM" for some released titles.
  These devices are *extremely* expensive (and ultra rare, only sold
  to big casino corporations), and should be placed in the empty socket
  to fix a dead board due to NVRAM corruption.

  A version of Magic Train running on subsino.c (unlike mtrain, which is
  subsino2.c) is needed to match this program ROM.

***************************************************************************/

ROM_START( mtrainnv )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mtrain_settings.bin", 0x00000, 0x10000, CRC(584af1b5) SHA1(91d966d282823dddfdc455bb03728fcdf3713dd7) )

	ROM_REGION( 0x10000, "tilemap", 0 )
	ROM_LOAD( "mtrain_tilemap.bin", 0x00000, 0x10000, NO_DUMP )
	ROM_COPY( "maincpu", 0x0000, 0x00000, 0x10000 ) // just to show something

	ROM_REGION( 0x10000, "reels", 0 )
	ROM_LOAD( "mtrain_reels.bin", 0x00000, 0x10000, NO_DUMP )
	ROM_COPY( "maincpu", 0x0000, 0x00000, 0x10000 ) // just to show something
ROM_END


/***************************************************************************
*                        Driver Init / Decryption                          *
***************************************************************************/

DRIVER_INIT_MEMBER(subsino_state,victor5)
{
	subsino_decrypt(machine(), victor5_bitswaps, victor5_xors, 0xc000);
}

DRIVER_INIT_MEMBER(subsino_state,victor21)
{
	subsino_decrypt(machine(), victor21_bitswaps, victor21_xors, 0xc000);
}

DRIVER_INIT_MEMBER(subsino_state,crsbingo)
{
	subsino_decrypt(machine(), crsbingo_bitswaps, crsbingo_xors, 0xc000);
}

DRIVER_INIT_MEMBER(subsino_state,sharkpy)
{
	subsino_decrypt(machine(), sharkpy_bitswaps, sharkpy_xors, 0xa000);
}

DRIVER_INIT_MEMBER(subsino_state,sharkpye)
{
	subsino_decrypt(machine(), victor5_bitswaps, victor5_xors, 0xa000);
}

DRIVER_INIT_MEMBER(subsino_state,smoto20)
{
	UINT8 *rom = memregion( "maincpu" )->base();
	rom[0x12e1] = 0x20; // "ERROR 951010"
}

DRIVER_INIT_MEMBER(subsino_state,tisub)
{
	UINT8 *rom = memregion( "maincpu" )->base();

	DRIVER_INIT_CALL(victor5);

	/* this trips a z180 MMU core bug? It unmaps a region then the program code jumps to that region... */
	rom[0x64c8] = 0x00;
	rom[0x64c9] = 0x00;
	rom[0x64ca] = 0x00;
	rom[0x64cd] = 0x00;
	rom[0x64ce] = 0x00;
	rom[0x64cf] = 0x00;
}

DRIVER_INIT_MEMBER(subsino_state,tisuba)
{
	UINT8 *rom = memregion( "maincpu" )->base();

	DRIVER_INIT_CALL(victor5);

	/* this trips a z180 MMU core bug? It unmaps a region then the program code jumps to that region... */
	rom[0x6491] = 0x00;
	rom[0x6492] = 0x00;
	rom[0x6493] = 0x00;
	rom[0x6496] = 0x00;
	rom[0x6497] = 0x00;
	rom[0x6498] = 0x00;
}

DRIVER_INIT_MEMBER(subsino_state,stbsub)
{
#if 1
	UINT8 *rom = memregion( "maincpu" )->base();
	rom[0x1005] = 0x1d; //patch protection check
	rom[0x7ab] = 0x18; //patch "winning protection" check
	rom[0x957] = 0x18; //patch "losing protection" check
#endif

	m_stbsub_colorram = auto_alloc_array(machine(), UINT8, 256*3);

	m_reel1_scroll.allocate(0x40);
	m_reel2_scroll.allocate(0x40);
	m_reel3_scroll.allocate(0x40);

	m_reel1_attr = auto_alloc_array(machine(), UINT8, 0x200);
	m_reel2_attr = auto_alloc_array(machine(), UINT8, 0x200);
	m_reel3_attr = auto_alloc_array(machine(), UINT8, 0x200);
}

DRIVER_INIT_MEMBER(subsino_state, stisub)
{
	UINT8 *rom = memregion( "maincpu" )->base();
	rom[0x0FA0] = 0x28;
	rom[0x0FA1] = 0x1d; //patch protection check

	m_stbsub_colorram = auto_alloc_array(machine(), UINT8, 256*3);

	m_reel1_scroll.allocate(0x40);
	m_reel2_scroll.allocate(0x40);
	m_reel3_scroll.allocate(0x40);

	m_reel1_attr = auto_alloc_array(machine(), UINT8, 0x200);
	m_reel2_attr = auto_alloc_array(machine(), UINT8, 0x200);
	m_reel3_attr = auto_alloc_array(machine(), UINT8, 0x200);
}

DRIVER_INIT_MEMBER(subsino_state,tesorone)
{
#if 1
	UINT8 *rom = memregion( "maincpu" )->base();
	rom[0x10a4] = 0x18; //patch protection check ("ERROR 08073"):
	rom[0x10a5] = 0x11;
	rom[0x8b6] = 0x18; //patch "winning protection" check
	rom[0xa84] = 0x18; //patch "losing protection" check
#endif

	m_stbsub_colorram = auto_alloc_array(machine(), UINT8, 256*3);

	m_reel1_scroll.allocate(0x40);
	m_reel2_scroll.allocate(0x40);
	m_reel3_scroll.allocate(0x40);

	m_reel1_attr = auto_alloc_array(machine(), UINT8, 0x200);
	m_reel2_attr = auto_alloc_array(machine(), UINT8, 0x200);
	m_reel3_attr = auto_alloc_array(machine(), UINT8, 0x200);
}

DRIVER_INIT_MEMBER(subsino_state,tesorone230)
{
#if 1
	UINT8 *rom = memregion( "maincpu" )->base();            //check this patch!!!!
	rom[0x10a8] = 0x18; //patch protection check ("ERROR 08073"):
	rom[0x10a9] = 0x11;
	rom[0x8ba] = 0x18; //patch "winning protection" check
	rom[0xa88] = 0x18; //patch "losing protection" check
#endif

	m_stbsub_colorram = auto_alloc_array(machine(), UINT8, 256*3);

	m_reel1_scroll.allocate(0x40);
	m_reel2_scroll.allocate(0x40);
	m_reel3_scroll.allocate(0x40);

	m_reel1_attr = auto_alloc_array(machine(), UINT8, 0x200);
	m_reel2_attr = auto_alloc_array(machine(), UINT8, 0x200);
	m_reel3_attr = auto_alloc_array(machine(), UINT8, 0x200);
}


DRIVER_INIT_MEMBER(subsino_state,mtrainnv)
{
	m_stbsub_colorram = auto_alloc_array(machine(), UINT8, 256*3);

	m_reel1_scroll.allocate(0x40);
	m_reel2_scroll.allocate(0x40);
	m_reel3_scroll.allocate(0x40);

	m_reel1_attr = auto_alloc_array(machine(), UINT8, 0x200);
	m_reel2_attr = auto_alloc_array(machine(), UINT8, 0x200);
	m_reel3_attr = auto_alloc_array(machine(), UINT8, 0x200);
}

/***************************************************************************
*                               Game Drivers                               *
***************************************************************************/

//     YEAR  NAME         PARENT    MACHINE   INPUT     INIT                     ROT   COMPANY            FULLNAME                                FLAGS            LAYOUT
GAMEL( 1990, victor21,    0,        victor21, victor21, subsino_state, victor21, ROT0, "Subsino / Buffy", "Victor 21",                            0,               layout_victor21 )

GAMEL( 1991, victor5,     0,        victor5,  victor5,  subsino_state, victor5,  ROT0, "Subsino",         "G.E.A.",                               0,               layout_victor5  ) // PCB black-box was marked 'victor 5' - in-game says G.E.A with no manufacturer info?

GAMEL( 1992, tisub,       0,        tisub,    tisub,    subsino_state, tisub,    ROT0, "Subsino",         "Treasure Island (Subsino, set 1)",     0,               layout_tisub    )
GAMEL( 1992, tisuba,      tisub,    tisub,    tisub,    subsino_state, tisuba,   ROT0, "Subsino",         "Treasure Island (Subsino, set 2)",     0,               layout_tisub    )

GAMEL( 1991, crsbingo,    0,        crsbingo, crsbingo, subsino_state, crsbingo, ROT0, "Subsino",         "Poker Carnival",                       0,               layout_crsbingo )

GAMEL( 1995, stbsub,      0,        stbsub,   stbsub,   subsino_state, stbsub,   ROT0, "American Alpha",  "Treasure Bonus (Subsino, v1.6)",       0,               layout_stisub   ) // board CPU module marked 'Super Treasure Island' (alt title?)
GAMEL( 1995, stisub,      stbsub,   stbsub,   stbsub,   subsino_state, stisub,   ROT0, "Subsino",         "Super Treasure Island (Italy, v1.6)",  MACHINE_NOT_WORKING, layout_stisub   ) // need proper patches
GAMEL( 1995, tesorone,    stbsub,   stbsub,   tesorone, subsino_state, tesorone, ROT0, "Subsino",         "Tesorone Dell'Isola (Italy, v2.41)",   0,               layout_stisub   )
GAMEL( 1995, tesorone240, stbsub,   stbsub,   tesorone, subsino_state, tesorone, ROT0, "Subsino",         "Tesorone Dell'Isola (Italy, v2.40)",   0,               layout_stisub   )
GAMEL( 1995, tesorone230, stbsub,   stbsub,   tesorone, subsino_state, tesorone230,ROT0,"Subsino",        "Tesorone Dell'Isola (Italy, v2.30)",   0,               layout_stisub   )

GAMEL( 1996, sharkpy,     0,        sharkpy,  sharkpy,  subsino_state, sharkpy,  ROT0, "Subsino",         "Shark Party (Italy, v1.3)",            0,               layout_sharkpy  ) // missing POST messages?
GAMEL( 1996, sharkpya,    sharkpy,  sharkpy,  sharkpy,  subsino_state, sharkpy,  ROT0, "Subsino",         "Shark Party (Italy, v1.6)",            0,               layout_sharkpy  ) // missing POST messages?
GAMEL( 1995, sharkpye,    sharkpy,  sharkpy,  sharkpye, subsino_state, sharkpye, ROT0, "American Alpha",  "Shark Party (English, Alpha license)", 0,               layout_sharkpye ) // PCB black-box was marked 'victor 6'

GAMEL( 1995, victor6,     0,        sharkpy,  victor6,  subsino_state, sharkpye, ROT0, "American Alpha",  "Victor 6 (v2.3N)",                     0,               layout_sharkpye ) // ^^
GAMEL( 1995, victor6a,    victor6,  sharkpy,  victor6a, subsino_state, sharkpye, ROT0, "American Alpha",  "Victor 6 (v2.3)",                      0,               layout_sharkpye ) // ^^
GAMEL( 1995, victor6b,    victor6,  sharkpy,  victor6b, subsino_state, sharkpye, ROT0, "American Alpha",  "Victor 6 (v1.2)",                      0,               layout_sharkpye ) // ^^ Version # according to label, not displayed

GAMEL( 1996, smoto20,     0,        srider,   smoto20,  subsino_state, smoto20,  ROT0, "Subsino",         "Super Rider (Italy, v2.0)",            0,               layout_smoto    )
GAMEL( 1996, smoto16,     smoto20,  srider,   smoto16,  subsino_state, smoto16,  ROT0, "Subsino",         "Super Moto (Italy, v1.6)",             0,               layout_smoto    )

GAME ( 1996, mtrainnv,    mtrain,   mtrainnv, stbsub,   subsino_state, mtrainnv, ROT0, "Subsino",         "Magic Train (Clear NVRAM ROM?)",       MACHINE_NOT_WORKING )
