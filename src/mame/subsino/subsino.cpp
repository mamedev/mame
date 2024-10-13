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


  2019-07-31
  ----------

  - Added Victor 5 (original set, now parent).
  - Dumped the samples ROMs of Victor 5 and Victor 21, and hooked the OKI6295.


***************************************************************************/

#include "emu.h"
#include "subsino_crypt.h"

#include "cpu/z180/hd647180x.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"
#include "video/ramdac.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include "victor5.lh"
#include "victor21.lh"
#include "crsbingo.lh"
#include "sharkpy.lh"
#include "sharkpye.lh"
#include "smoto.lh"
#include "tisub.lh"
#include "tisubb.lh"
#include "stisub.lh"


namespace {

class subsino_state : public driver_device
{
public:
	subsino_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_reel_scroll(*this, "reel_scroll.%u", 0U, 0x40U, ENDIANNESS_LITTLE),
		m_reel_ram(*this, "reel_ram.%u", 0U),
		m_stbsub_out_c(*this, "stbsub_out_c"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_hopper(*this, "hopper"),
		m_lamps(*this, "lamp%u", 0U) {
	}

	void mtrainnv(machine_config &config);
	void stbsub(machine_config &config);
	void tisub(machine_config &config);
	void crsbingo(machine_config &config);
	void dinofmly(machine_config &config);
	void srider(machine_config &config);
	void victor21(machine_config &config);
	void sharkpy(machine_config &config);
	void victor5(machine_config &config);
	void newhunterb(machine_config &config);
	void newhunterd(machine_config &config);

	void init_stbsub();
	void init_stisub();
	void init_tesorone();
	void init_tesorone230();
	void init_smoto13();
	void init_smoto20();
	void init_sharkpy();
	void init_smoto16();
	void init_crsbingo();
	void init_victor21();
	void init_victor5();
	void init_tisubb();
	void init_newhunterb();
	void init_newhunterc();
	void init_sharkpye();
	void init_tisub();
	void init_grndprix();
	void init_mtrainnv();

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_videoram;
	memory_share_array_creator<uint8_t, 3> m_reel_scroll;
	optional_shared_ptr_array<uint8_t, 3> m_reel_ram;
	optional_shared_ptr<uint8_t> m_stbsub_out_c;
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<ticket_dispenser_device> m_hopper;
	output_finder<16> m_lamps;

	tilemap_t *m_tmap = nullptr;
	tilemap_t *m_reel_tilemap[3]{};
	int m_tiles_offset = 0;
	uint8_t m_out_c = 0;
	std::unique_ptr<uint8_t[]> m_reel_attr[3];
	uint8_t m_flash_val = 0;
	uint8_t m_flash_packet = 0;
	uint8_t m_flash_packet_start = 0;

	void tiles_offset_w(offs_t offset, uint8_t data);
	void videoram_w(offs_t offset, uint8_t data);
	void colorram_w(offs_t offset, uint8_t data);
	template<uint8_t Reel> void reel_ram_w(offs_t offset, uint8_t data);
	void out_a_w(uint8_t data);
	void out_b_w(uint8_t data);
	uint8_t flash_r();
	void flash_w(uint8_t data);
	uint8_t hwcheck_r();
	uint8_t out_c_r();
	void out_c_w(uint8_t data);
	void reel_scrollattr_w(offs_t offset, uint8_t data);
	uint8_t reel_scrollattr_r(offs_t offset);

	TILE_GET_INFO_MEMBER(get_tile_info);
	TILE_GET_INFO_MEMBER(get_stbsub_tile_info);
	template<uint8_t Reel> TILE_GET_INFO_MEMBER(get_reel_tile_info);
	template<uint8_t Reel> TILE_GET_INFO_MEMBER(get_stbsub_reel_tile_info);
	DECLARE_VIDEO_START(subsino);
	void _2proms_palette(palette_device &palette) const;
	void _3proms_palette(palette_device &palette) const;
	DECLARE_VIDEO_START(reels);
	DECLARE_VIDEO_START(stbsub);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_reels(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_stbsub_reels(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void crsbingo_map(address_map &map) ATTR_COLD;
	void dinofmly_map(address_map &map) ATTR_COLD;
	void mtrainnv_map(address_map &map) ATTR_COLD;
	void newhunterb_map(address_map &map) ATTR_COLD;
	void newhunterd_map(address_map &map) ATTR_COLD;
	void ramdac_map(address_map &map) ATTR_COLD;
	void sharkpy_map(address_map &map) ATTR_COLD;
	void srider_map(address_map &map) ATTR_COLD;
	void stbsub_map(address_map &map) ATTR_COLD;
	void subsino_iomap(address_map &map) ATTR_COLD;
	void tisub_base_map(address_map &map) ATTR_COLD;
	void tisub_map(address_map &map) ATTR_COLD;
	void victor21_map(address_map &map) ATTR_COLD;
	void victor5_map(address_map &map) ATTR_COLD;
};

void subsino_state::machine_start()
{
	m_lamps.resolve();

	save_item(NAME(m_tiles_offset));
	save_item(NAME(m_out_c));
}

/***************************************************************************
*                              Video Hardware                              *
***************************************************************************/


void subsino_state::tiles_offset_w(offs_t offset, uint8_t data)
{
	m_tiles_offset = (data & 1) ? 0x1000: 0;
	m_tmap->mark_tile_dirty(offset);
//  popmessage("gfx %02x",data);
}

void subsino_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_tmap->mark_tile_dirty(offset);
}

void subsino_state::colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_tmap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(subsino_state::get_tile_info)
{
	uint16_t code = m_videoram[ tile_index ] + (m_colorram[ tile_index ] << 8);
	uint16_t color = (code >> 8) & 0x0f;
	code = ((code & 0xf000) >> 4) + ((code & 0xff) >> 0);
	code += m_tiles_offset;
	tileinfo.set(0, code, color, 0);
}

TILE_GET_INFO_MEMBER(subsino_state::get_stbsub_tile_info)
{
	uint16_t code = m_videoram[ tile_index ] + (m_colorram[ tile_index ] << 8);
	code&= 0x3fff;
	tileinfo.set(0, code, 0, 0);
}


VIDEO_START_MEMBER(subsino_state,subsino)
{
	m_tmap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(subsino_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8,8, 0x40,0x20);
	m_tmap->set_transparent_pen(0 );
	m_tiles_offset = 0;
}


template<uint8_t Reel>
void subsino_state::reel_ram_w(offs_t offset, uint8_t data)
{
	m_reel_ram[Reel][offset] = data;
	m_reel_tilemap[Reel]->mark_tile_dirty(offset);
}

template<uint8_t Reel>
TILE_GET_INFO_MEMBER(subsino_state::get_reel_tile_info)
{
	int code = m_reel_ram[Reel][tile_index];
	int colour = (m_out_c&0x7) + 8;

	tileinfo.set(1,
			code,
			colour,
			0);
}

template<uint8_t Reel>
TILE_GET_INFO_MEMBER(subsino_state::get_stbsub_reel_tile_info)
{
	int code = m_reel_ram[Reel][tile_index];
	int attr = m_reel_attr[Reel][tile_index];

	tileinfo.set(1,
			code | (attr << 8),
			0,
			0);
}


VIDEO_START_MEMBER(subsino_state, reels)
{
	VIDEO_START_CALL_MEMBER( subsino );

	m_reel_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(subsino_state::get_reel_tile_info<0>)), TILEMAP_SCAN_ROWS, 8, 32, 64, 8);
	m_reel_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(subsino_state::get_reel_tile_info<1>)), TILEMAP_SCAN_ROWS, 8, 32, 64, 8);
	m_reel_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(subsino_state::get_reel_tile_info<2>)), TILEMAP_SCAN_ROWS, 8, 32, 64, 8);

	m_reel_tilemap[0]->set_scroll_cols(64);
	m_reel_tilemap[1]->set_scroll_cols(64);
	m_reel_tilemap[2]->set_scroll_cols(64);
}

VIDEO_START_MEMBER(subsino_state,stbsub)
{
	m_tmap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(subsino_state::get_stbsub_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 0x40, 0x20);
	m_tmap->set_transparent_pen(0 );

	m_reel_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(subsino_state::get_stbsub_reel_tile_info<0>)), TILEMAP_SCAN_ROWS, 8, 32, 64, 8);
	m_reel_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(subsino_state::get_stbsub_reel_tile_info<1>)), TILEMAP_SCAN_ROWS, 8, 32, 64, 8);
	m_reel_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(subsino_state::get_stbsub_reel_tile_info<2>)), TILEMAP_SCAN_ROWS, 8, 32, 64, 8);

	m_reel_tilemap[0]->set_scroll_cols(64);
	m_reel_tilemap[1]->set_scroll_cols(64);
	m_reel_tilemap[2]->set_scroll_cols(64);

	m_out_c = 0x08;
}


uint32_t subsino_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	m_tmap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

uint32_t subsino_state::screen_update_reels(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	for (uint8_t reel = 0; reel < 3; reel++)
	{
		for (int i = 0; i < 64; i++)
		{
			m_reel_tilemap[reel]->set_scrolly(i, m_reel_scroll[reel][i]);
		}
	}

	if (m_out_c&0x08)
	{
		// are these hardcoded, or registers?
		const rectangle visible1(0*8, (14+48)*8-1,  4*8,  (4+7)*8-1);
		const rectangle visible2(0*8, (14+48)*8-1, 10*8, (10+7)*8-1);
		const rectangle visible3(0*8, (14+48)*8-1, 18*8, (18+7)*8-1);

		m_reel_tilemap[0]->draw(screen, bitmap, visible1, 0, 0);
		m_reel_tilemap[1]->draw(screen, bitmap, visible2, 0, 0);
		m_reel_tilemap[2]->draw(screen, bitmap, visible3, 0, 0);
	}

	m_tmap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


uint32_t subsino_state::screen_update_stbsub_reels(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	if (m_reel_attr[0])
	{
		for (uint8_t reel = 0; reel < 3; reel++)
			m_reel_tilemap[reel]->mark_all_dirty();
	}

	for (uint8_t reel = 0; reel < 3; reel++)
	{
		for (int i = 0; i < 64; i++)
		{
			m_reel_tilemap[reel]->set_scrolly(i, m_reel_scroll[reel][i]);
		}
	}

	if (m_out_c & 0x08)
	{
		// areas based on d-up game in attract mode
		const rectangle visible1(0, 511,  0,  87);
		const rectangle visible2(0, 511,  88, 143);
		const rectangle visible3(0, 511,  144, 223);

		m_reel_tilemap[0]->draw(screen, bitmap, visible1, 0, 0);
		m_reel_tilemap[1]->draw(screen, bitmap, visible2, 0, 0);
		m_reel_tilemap[2]->draw(screen, bitmap, visible3, 0, 0);
	}

	m_tmap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


void subsino_state::_2proms_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();
	for (int i = 0; i < 256; i++)
	{
		int bit0, bit1, bit2;
		int const val = color_prom[i | 0x100] | (color_prom[i | 0x000] << 4);

		bit0 = 0;
		bit1 = BIT(val, 6);
		bit2 = BIT(val, 7);
		int const b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0 = BIT(val, 3);
		bit1 = BIT(val, 4);
		bit2 = BIT(val, 5);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0 = BIT(val, 0);
		bit1 = BIT(val, 1);
		bit2 = BIT(val, 2);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

void subsino_state::_3proms_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();
	for (int i = 0; i < 256; i++)
	{
		int bit0, bit1, bit2;
		int const val = color_prom[i | 0x000] | (color_prom[i | 0x100] << 3) | (color_prom[i | 0x200] << 6);

		bit0 = 0;
		bit1 = BIT(val, 7);
		bit2 = BIT(val, 6);
		int const b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0 = BIT(val, 5);
		bit1 = BIT(val, 4);
		bit2 = BIT(val, 3);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0 = BIT(val, 2);
		bit1 = BIT(val, 1);
		bit2 = BIT(val, 0);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}


/***************************************************************************
*                          Lamps & other outputs.                          *
***************************************************************************/

void subsino_state::out_a_w(uint8_t data)
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

	for (int i = 0; i < 8; i++) // Lamps 8 - 15
		m_lamps[i + 8] = BIT(data, i);

	machine().bookkeeping().coin_counter_w(0, data & 0x01 );    // coin / keyin
	machine().bookkeeping().coin_counter_w(1, data & 0x02 );    // keyin / coin
	machine().bookkeeping().coin_counter_w(2, data & 0x10 );    // keyout
	machine().bookkeeping().coin_counter_w(3, data & 0x20 );    // payout

	m_hopper->motor_w(BIT(data, 5));   // hopper motor

//  popmessage("Out A %02x",data);
}

void subsino_state::out_b_w(uint8_t data)
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


  * Treasure Island (American Alpha). // TODO: Where's the third hold lamp?

  7654 3210
  ---- ---x  ???.
  ---- --x-  DOUBLE / INFO.
  ---- -x--  START
  ---- x---  BET / STOP ALL.
  ---x ----  ???.
  --x- ----  HOLD1 / TAKE.
  -x-- ----  ???.
  x--- ----  HOLD3 / SMALL (or HOLD2 /BIG).


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

	// Lamps 0 - 7
	for (int i = 0; i < 8; i++)
		m_lamps[i] = BIT(data, i);

//  popmessage("Out B %02x",data);
}


/***************************************************************************
*                              Memory Maps                                 *
***************************************************************************/

void subsino_state::srider_map(address_map &map)
{
	map(0x00000, 0x0bfff).rom();
	map(0x0c000, 0x0cfff).ram();
	map(0x0d000, 0x0d002).r("ppi1", FUNC(i8255_device::read));
	map(0x0d004, 0x0d006).r("ppi2", FUNC(i8255_device::read));
	map(0x0d009, 0x0d009).w(FUNC(subsino_state::out_b_w));
	map(0x0d00a, 0x0d00a).w(FUNC(subsino_state::out_a_w));
	map(0x0d00c, 0x0d00c).portr("INC");
	map(0x0d016, 0x0d017).w("ymsnd", FUNC(ym3812_device::write));
	map(0x0d018, 0x0d018).w("oki", FUNC(okim6295_device::write));
	map(0x0d01b, 0x0d01b).w(FUNC(subsino_state::tiles_offset_w));
	map(0x0e000, 0x0e7ff).ram().w(FUNC(subsino_state::colorram_w)).share("colorram");
	map(0x0e800, 0x0efff).ram().w(FUNC(subsino_state::videoram_w)).share("videoram");
}

void subsino_state::dinofmly_map(address_map &map)
{
	srider_map(map);

	map(0x0d800, 0x0d800).w("ramdac", FUNC(ramdac_device::index_w));
	map(0x0d801, 0x0d801).w("ramdac", FUNC(ramdac_device::pal_w));
	map(0x0d802, 0x0d802).w("ramdac", FUNC(ramdac_device::mask_w));
}

void subsino_state::sharkpy_map(address_map &map)
{
	map(0x06000, 0x0d7ff).rom().region("program", 0x6000);
	map(0x0d800, 0x0dfff).ram();
	map(0x0e000, 0x0e7ff).ram().w(FUNC(subsino_state::colorram_w)).share("colorram");
	map(0x0e800, 0x0efff).ram().w(FUNC(subsino_state::videoram_w)).share("videoram");
	map(0x0f000, 0x0f002).r("ppi1", FUNC(i8255_device::read));
	map(0x0f004, 0x0f006).r("ppi2", FUNC(i8255_device::read));
	map(0x0f009, 0x0f009).w(FUNC(subsino_state::out_b_w));
	map(0x0f00a, 0x0f00a).w(FUNC(subsino_state::out_a_w));
	map(0x0f00c, 0x0f00c).portr("INC");
	map(0x0f016, 0x0f017).w("ymsnd", FUNC(ym3812_device::write));
	map(0x0f018, 0x0f018).w("oki", FUNC(okim6295_device::write));
	map(0x0f01b, 0x0f01b).w(FUNC(subsino_state::tiles_offset_w));
	map(0x10000, 0x15fff).rom().region("program", 0);
}

/*
Victor 21 protection is absolutely trivial. At every number of presetted hands, there's an animation
that announces to the player that the card deck changes. If the protection check fails (at start-up),
this event makes the game to reset without any money in the bank.
*/

void subsino_state::victor21_map(address_map &map)
{
	map(0x06000, 0x0d7ff).rom().region("program", 0x4000);
	map(0x0d800, 0x0dfff).ram();
	map(0x0e000, 0x0e7ff).ram().w(FUNC(subsino_state::videoram_w)).share("videoram");
	map(0x0e800, 0x0efff).ram().w(FUNC(subsino_state::colorram_w)).share("colorram");
	map(0x0f000, 0x0f003).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x0f004, 0x0f004).portr("INA");
	map(0x0f005, 0x0f005).portr("INB");
	map(0x0f006, 0x0f006).portr("SW1");
	map(0x0f007, 0x0f007).portr("SW2");
	map(0x0f008, 0x0f008).portr("SW3");
	map(0x0f00b, 0x0f00b).ram(); //protection
	map(0x0f00c, 0x0f00c).w("oki", FUNC(okim6295_device::write));
	map(0x0f00e, 0x0f00f).w("ymsnd", FUNC(ym2413_device::write));
	map(0x0f00d, 0x0f00d).w(FUNC(subsino_state::tiles_offset_w));
	map(0x10000, 0x13fff).rom().region("program", 0);
}


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


uint8_t subsino_state::flash_r()
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

void subsino_state::flash_w(uint8_t data)
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

void subsino_state::victor5_map(address_map &map)
{
	victor21_map(map);
	map(0x0f00a, 0x0f00a).rw(FUNC(subsino_state::flash_r), FUNC(subsino_state::flash_w));
	map(0x0f00b, 0x0f00b).nopr(); //"flash" status, bit 0
	map(0x0f00c, 0x0f00c).w("oki", FUNC(okim6295_device::write));
}


uint8_t subsino_state::hwcheck_r()
{
	// Wants this at POST otherwise an "Hardware Error" occurs.
	return 0x55;
}

void subsino_state::crsbingo_map(address_map &map)
{
	map(0x06000, 0x0d7ff).rom().region("program", 0x4000);
	map(0x0d800, 0x0dfff).ram();
	map(0x0e000, 0x0e7ff).ram().w(FUNC(subsino_state::videoram_w)).share("videoram");
	map(0x0e800, 0x0efff).ram().w(FUNC(subsino_state::colorram_w)).share("colorram");

	map(0x0f000, 0x0f000).portr("SW1");
	map(0x0f001, 0x0f001).portr("SW2");
	map(0x0f002, 0x0f002).portr("INA");
	map(0x0f003, 0x0f003).portr("INB");
	map(0x0f004, 0x0f004).portr("INC");
	map(0x0f005, 0x0f005).w(FUNC(subsino_state::out_a_w));

	map(0x0f008, 0x0f008).portr("SW4");
	map(0x0f009, 0x0f009).portr("SW3");  // .w(FUNC(subsino_state::out_a_w));
	map(0x0f00a, 0x0f00a).rw(FUNC(subsino_state::hwcheck_r), FUNC(subsino_state::out_b_w));

	map(0x0f010, 0x0f010).rw(FUNC(subsino_state::flash_r), FUNC(subsino_state::flash_w));
//  map(0x0f011, 0x0f011) //"flash" status, bit 0
//  map(0x0f00c, 0x0f00c).portr("INC");
	map(0x0f00c, 0x0f00d).w("ymsnd", FUNC(ym2413_device::write));

//  map(0x0f018, 0x0f018).w("oki", FUNC(okim6295_device::write));

//  map(0x0f00d, 0x0f00d).w(FUNC(subsino_state::tiles_offset_w));

	map(0x10000, 0x13fff).rom().region("program", 0); //overlap unmapped regions
}

uint8_t subsino_state::out_c_r()
{
	return m_out_c;
}

void subsino_state::out_c_w(uint8_t data)
{
	// not 100% sure on this

	// ???? eccc
	// e = enable reels?
	// c = reel colour bank?
	m_out_c = data;

	for (uint8_t reel = 0; reel < 3; reel++)
		m_reel_tilemap[reel]->mark_all_dirty();
//  popmessage("data %02x\n",data);
}

void subsino_state::tisub_base_map(address_map &map)
{
	map(0x0d800, 0x0dfff).ram();
	map(0x0e000, 0x0e7ff).ram().w(FUNC(subsino_state::colorram_w)).share("colorram");
	map(0x0e800, 0x0efff).ram().w(FUNC(subsino_state::videoram_w)).share("videoram");
	map(0x0f000, 0x0f002).r("ppi1", FUNC(i8255_device::read));
	map(0x0f004, 0x0f006).r("ppi2", FUNC(i8255_device::read));
	map(0x0f008, 0x0f008).rw(FUNC(subsino_state::out_c_r), FUNC(subsino_state::out_c_w));
	map(0x0f009, 0x0f009).w(FUNC(subsino_state::out_b_w));
	map(0x0f00a, 0x0f00a).w(FUNC(subsino_state::out_a_w));
	map(0x10000, 0x13fff).rom().region("program", 0);
	map(0x1e0c0, 0x1e0ff).ram().share("reel_scroll.2");
	map(0x1e140, 0x1e17f).ram().share("reel_scroll.1");
	map(0x1e180, 0x1e1bf).ram().share("reel_scroll.0");
	map(0x1e800, 0x1e9ff).ram().w(FUNC(subsino_state::reel_ram_w<0>)).share("reel_ram.0");
	map(0x1ea00, 0x1ebff).ram().w(FUNC(subsino_state::reel_ram_w<1>)).share("reel_ram.1");
	map(0x1ec00, 0x1edff).ram().w(FUNC(subsino_state::reel_ram_w<2>)).share("reel_ram.2");
}

void subsino_state::tisub_map(address_map &map)
{
	tisub_base_map(map);

	map(0x06000, 0x0d7ff).rom().region("program", 0x4000);
	map(0x0f00c, 0x0f00c).portr("INC");
	map(0x0f016, 0x0f017).w("ymsnd", FUNC(ym3812_device::write));
	map(0x0f01b, 0x0f01b).w(FUNC(subsino_state::tiles_offset_w));
	map(0x1d000, 0x1dfff).rom().region("program", 0xd000); // reads the card face data here
}

void subsino_state::newhunterb_map(address_map &map)
{
	tisub_base_map(map);

	map(0x06000, 0x0d7ff).rom().region("program", 0x6000);
	map(0x0f00c, 0x0f00d).w("ymsnd", FUNC(ym3812_device::write));
	map(0x0f00e, 0x0f00e).portr("INC");
	map(0x0f00f, 0x0f00f).w(FUNC(subsino_state::tiles_offset_w));
	map(0x14000, 0x15fff).rom().region("program", 0x4000);
}

void subsino_state::newhunterd_map(address_map &map)
{
	map(0x00000, 0x06fff).rom().region("maincpu", 0x0000); // 0x7000 - 0x9fff are 0xff filled
	map(0x07000, 0x07fff).ram();
	map(0x08000, 0x087ff).ram().w(FUNC(subsino_state::colorram_w)).share("colorram");
	map(0x08800, 0x08fff).ram().w(FUNC(subsino_state::videoram_w)).share("videoram");
	map(0x09000, 0x09002).r("ppi1", FUNC(i8255_device::read));
	map(0x09004, 0x09006).r("ppi2", FUNC(i8255_device::read));
	map(0x09008, 0x09008).rw(FUNC(subsino_state::out_c_r), FUNC(subsino_state::out_c_w));
	map(0x09009, 0x09009).w(FUNC(subsino_state::out_b_w));
	map(0x0900a, 0x0900a).w(FUNC(subsino_state::out_a_w));
	map(0x0900c, 0x0900d).w("ymsnd", FUNC(ym3812_device::write));
	map(0x0900e, 0x0900e).portr("INC");
	map(0x0900f, 0x0900f).w(FUNC(subsino_state::tiles_offset_w));
	map(0x09800, 0x09fff).ram();
	map(0x0a000, 0x0dfff).rom().region("maincpu", 0xa000);
	map(0x0f0c0, 0x0f0ff).ram().share("reel_scroll.2");
	map(0x0f140, 0x0f17f).ram().share("reel_scroll.1");
	map(0x0f180, 0x0f1bf).ram().share("reel_scroll.0");
	map(0x0f800, 0x0f9ff).ram().w(FUNC(subsino_state::reel_ram_w<0>)).share("reel_ram.0");
	map(0x0fa00, 0x0fbff).ram().w(FUNC(subsino_state::reel_ram_w<1>)).share("reel_ram.1");
	map(0x0fc00, 0x0fdff).ram().w(FUNC(subsino_state::reel_ram_w<2>)).share("reel_ram.2");
}

void subsino_state::ramdac_map(address_map &map)
{
	map(0x000, 0x3ff).rw("ramdac", FUNC(ramdac_device::ramdac_pal_r), FUNC(ramdac_device::ramdac_rgb666_w));
}

// this stuff is banked..
// not 100% sure on the bank bits.. other bits are also set
void subsino_state::reel_scrollattr_w(offs_t offset, uint8_t data)
{
	if (*m_stbsub_out_c & 0x20)
	{
		if (offset<0x200)
		{
			m_reel_attr[0][offset&0x1ff] = data;
		}
		else if (offset<0x400)
		{
			m_reel_attr[1][offset&0x1ff] = data;
		}
		else if (offset<0x600)
		{
			m_reel_attr[2][offset&0x1ff] = data;
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
			m_reel_scroll[1][offset&0x3f] = data;
		}
		else if (offset<0xc0)
		{
			m_reel_scroll[0][offset&0x3f] = data;
		}
		else
		{
			m_reel_scroll[2][offset&0x3f] = data;
		}
	}
}

uint8_t subsino_state::reel_scrollattr_r(offs_t offset)
{
	return m_reel_attr[0][offset];
}

void subsino_state::stbsub_map(address_map &map)
{
	map(0x00000, 0x0bfff).rom();

	map(0x0c000, 0x0cfff).ram().share("nvram");

	map(0x0d000, 0x0d002).r("ppi1", FUNC(i8255_device::read));
	map(0x0d004, 0x0d006).r("ppi2", FUNC(i8255_device::read));

	map(0x0d008, 0x0d008).ram().share("stbsub_out_c");

	map(0x0d009, 0x0d009).w(FUNC(subsino_state::out_b_w));
	map(0x0d00a, 0x0d00a).w(FUNC(subsino_state::out_a_w));

	map(0x0d00c, 0x0d00c).portr("INC");

	map(0x0d010, 0x0d010).w("ramdac", FUNC(ramdac_device::index_w));
	map(0x0d011, 0x0d011).w("ramdac", FUNC(ramdac_device::pal_w));
	map(0x0d012, 0x0d012).w("ramdac", FUNC(ramdac_device::mask_w));

	map(0x0d016, 0x0d017).w("ymsnd", FUNC(ym3812_device::write));

//  map(0x0d01b, 0x0d01b).w(FUNC(subsino_state::tiles_offset_w));

	map(0x0e000, 0x0e7ff).ram().w(FUNC(subsino_state::colorram_w)).share("colorram");
	map(0x0e800, 0x0efff).ram().w(FUNC(subsino_state::videoram_w)).share("videoram");

	map(0xf000, 0xf7ff).rw(FUNC(subsino_state::reel_scrollattr_r), FUNC(subsino_state::reel_scrollattr_w));

	map(0xf800, 0xf9ff).ram().w(FUNC(subsino_state::reel_ram_w<0>)).share("reel_ram.0");
	map(0xfa00, 0xfbff).ram().w(FUNC(subsino_state::reel_ram_w<1>)).share("reel_ram.1");
	map(0xfc00, 0xfdff).ram().w(FUNC(subsino_state::reel_ram_w<2>)).share("reel_ram.2");
}


/***************************************************************************
                        Magic Train (Clear NVRAM ROM?)
***************************************************************************/

void subsino_state::mtrainnv_map(address_map &map)
{
	map(0x00000, 0x0bfff).rom();

	map(0x0c000, 0x0cfff).ram().share("nvram");

	map(0x0d000, 0x0d002).r("ppi1", FUNC(i8255_device::read));
	map(0x0d004, 0x0d006).r("ppi2", FUNC(i8255_device::read));

	map(0x0d008, 0x0d008).ram().share("stbsub_out_c");
//  map(0x0d009, 0x0d009).w(FUNC(subsino_state::));
//  map(0x0d00a, 0x0d00a).w(FUNC(subsino_state::));
//  map(0x0d00b, 0x0d00b).w(FUNC(subsino_state::));
	map(0x0d00c, 0x0d00c).portr("INC");

	map(0x0d010, 0x0d010).w("ramdac", FUNC(ramdac_device::index_w));
	map(0x0d011, 0x0d011).w("ramdac", FUNC(ramdac_device::pal_w));
	map(0x0d012, 0x0d012).w("ramdac", FUNC(ramdac_device::mask_w));

//  map(0x0d012, 0x0d012).w(FUNC(subsino_state::));

	map(0x0d016, 0x0d017).w("ymsnd", FUNC(ym3812_device::write));

//  map(0x0d018, 0x0d018).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));

	map(0x0e000, 0x0e7ff).ram().w(FUNC(subsino_state::colorram_w)).share("colorram");
	map(0x0e800, 0x0efff).ram().w(FUNC(subsino_state::videoram_w)).share("videoram");

	map(0xf000, 0xf7ff).rw(FUNC(subsino_state::reel_scrollattr_r), FUNC(subsino_state::reel_scrollattr_w));

	map(0xf800, 0xf9ff).ram().w(FUNC(subsino_state::reel_ram_w<0>)).share("reel_ram.0");
	map(0xfa00, 0xfbff).ram().w(FUNC(subsino_state::reel_ram_w<1>)).share("reel_ram.1");
	map(0xfc00, 0xfdff).ram().w(FUNC(subsino_state::reel_ram_w<2>)).share("reel_ram.2");
}


void subsino_state::subsino_iomap(address_map &map)
{
	map(0x0000, 0x003f).ram(); // internal regs
}


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
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_BET )     PORT_IMPULSE(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_STAND )   PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_CODE(KEYCODE_A)    PORT_NAME("Bet x10")    // multibet
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )    PORT_CODE(KEYCODE_C)

	PORT_START( "INB" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )          PORT_IMPULSE(3) // coin 1
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )   // key in
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )          PORT_IMPULSE(3) // coin 2
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN3 )          PORT_IMPULSE(3) // coin 3
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )                            PORT_NAME("Stats")  // Bookkeeping.
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE )                         PORT_NAME("Settings")   // Game Rate.
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )  // no payout?
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )  // key out

	PORT_START( "INC" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )                           PORT_NAME("Reset")  // hard reset
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
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_BET )     PORT_IMPULSE(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START( "INB" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )          PORT_IMPULSE(3) // coin
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )   // key in
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )                            PORT_NAME("Stats")  // Bookkeeping.
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE )                         PORT_NAME("Settings")   // Game Rate & others.
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )  // payout
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )  // key out

	PORT_START( "INC" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )                           PORT_NAME("Reset")  // hard reset
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
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SLOT_STOP1 )     PORT_NAME("Stop 1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SLOT_STOP2 )     PORT_NAME("Stop 2 / Big")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SLOT_STOP3 )     PORT_NAME("Stop 3 / Small")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )         PORT_NAME("Start / Take")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_BET )     PORT_IMPULSE(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START( "INB" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )          PORT_IMPULSE(3) // coin
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )   // key in
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )                            PORT_NAME("Stats / Test")   // Bookkeeping.
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE )                         PORT_NAME("Settings")       // Current settings.
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )  // payout
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )  // key out

	PORT_START( "INC" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )                           PORT_NAME("Reset")  // hard reset
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( tisubb )
	PORT_INCLUDE( tisub )

	PORT_MODIFY( "SW2" )
	PORT_DIPNAME( 0x0c, 0x08, "Max Bet" )
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x04, "16" )
	PORT_DIPSETTING(    0x08, "32" )
	PORT_DIPSETTING(    0x0c, "64" )
	PORT_DIPNAME( 0x20, 0x20, "Game Limit" )            PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, "5000" )
	PORT_DIPSETTING(    0x00, "10000" )

	PORT_MODIFY( "SW3" )
	PORT_DIPNAME( 0x07, 0x07, "Main Game Rate" )        PORT_DIPLOCATION("SW3:1,2,3")
	PORT_DIPSETTING(    0x00, "84%" )
	PORT_DIPSETTING(    0x01, "86%" )
	PORT_DIPSETTING(    0x02, "88%" )
	PORT_DIPSETTING(    0x03, "89%" )
	PORT_DIPSETTING(    0x04, "90%" )
	PORT_DIPSETTING(    0x07, "91%" )
	PORT_DIPSETTING(    0x05, "92%" )
	PORT_DIPSETTING(    0x06, "94%" )

	PORT_MODIFY( "INA" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_BET )     PORT_IMPULSE(3) PORT_NAME("Bet / Stop All")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )    PORT_NAME("Double / Info")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY( "INB" )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SLOT_STOP1 )     PORT_NAME("Stop 1 / Take")

	PORT_MODIFY( "INC" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SLOT_STOP3 )     PORT_NAME("Stop 3 / Small")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SLOT_STOP2 )     PORT_NAME("Stop 2 / Big")
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
	PORT_DIPNAME( 0xe0, 0xe0, "Clear Ticket Unit" )     PORT_DIPLOCATION("SW4:6,7,8")
	PORT_DIPSETTING(    0x00, "500" )
	PORT_DIPSETTING(    0x20, "100" )
	PORT_DIPSETTING(    0x40, "50" )
	PORT_DIPSETTING(    0x60, "25" )
	PORT_DIPSETTING(    0x80, "20" )
	PORT_DIPSETTING(    0xa0, "10" )
	PORT_DIPSETTING(    0xc0, "5" )
	PORT_DIPSETTING(    0xe0, "1" )

	PORT_START("INA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )                                 PORT_NAME("Start / Stop All")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SLOT_STOP2 )                             PORT_NAME("Bet / Stop 2")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )    PORT_CODE(KEYCODE_Z)    PORT_NAME("Double / Info")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START( "INB" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )          PORT_IMPULSE(3) // coin
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )   // key in
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )                            PORT_NAME("Stats / Test")   // Bookkeeping.
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE )                         PORT_NAME("Settings")       // Current settings.
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )  // payout
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )  // key out

	PORT_START("INC")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SLOT_STOP3 )                             PORT_NAME("Small / Black / Stop 3")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", ticket_dispenser_device, line_r) // hopper sensor
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )                           PORT_NAME("Reset")  // hard reset
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )    PORT_CODE(KEYCODE_B)    PORT_NAME("Big / Red")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SLOT_STOP1 )                             PORT_NAME("Take / Stop 1")
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
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )                                 PORT_NAME("Start / Stop All")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SLOT_STOP2 )                             PORT_NAME("Bet / Stop 2")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )    PORT_CODE(KEYCODE_Z)    PORT_NAME("Double / Info")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START( "INB" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )          PORT_IMPULSE(3) // coin
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )   // key in
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )                            PORT_NAME("Stats / Test")   // Bookkeeping.
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE )                         PORT_NAME("Settings")       // Current settings.
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )  // payout
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )  // key out

	PORT_START("INC")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SLOT_STOP3 )                             PORT_NAME("Small / Black / Stop 3")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", ticket_dispenser_device, line_r) // hopper sensor
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )                           PORT_NAME("Reset")  // hard reset
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )    PORT_CODE(KEYCODE_B)    PORT_NAME("Big / Red")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SLOT_STOP1 )                             PORT_NAME("Take / Stop 1")
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
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_BET )     PORT_IMPULSE(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START( "INB" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )          PORT_IMPULSE(3) // coin
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )   // key in
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )                            PORT_NAME("Stats")      // Bookkeeping.
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE )                         PORT_NAME("Settings")   // Game Rate & others.
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )  // payout
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )  // key out

	PORT_START( "INC" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )                           PORT_NAME("Reset")  // hard reset
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
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_BET )     PORT_NAME("Bet")    PORT_IMPULSE(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START( "INB" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )          PORT_IMPULSE(3) // coin
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )                            PORT_NAME("Stats / Test")   // Bookkeeping.
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE )                         PORT_NAME("Settings")   // Game Rate & others.
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )  // payout?
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )  // key out?

	PORT_START( "INC" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )                           PORT_NAME("Reset")  // hard reset
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
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )                            PORT_NAME("Stats / Test")   // Bookkeeping.
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE )                         PORT_NAME("Settings")   // Game Rate & others.
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )

	PORT_START( "INC" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )    PORT_NAME("Hold 4 / Small")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )                           PORT_NAME("Reset Switch")   // hard reset
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
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_BET )     PORT_NAME("Bet / Speed")    PORT_IMPULSE(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START( "INB" ) // d006
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )          PORT_IMPULSE(3) // coin
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", ticket_dispenser_device, line_r) // hopper sensor
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )                            PORT_NAME("Stats / Test")   // Bookkeeping.
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE )                         PORT_NAME("Settings")   // Game Rate & others.
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )  // payout
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )  // key out?

	PORT_START( "INC" ) // d00c
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", ticket_dispenser_device, line_r) // hopper sensor
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )                           PORT_NAME("Reset")  // hard reset
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
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_BET )     PORT_NAME("Bet / Speed")    PORT_IMPULSE(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START( "INB" ) // d006
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )          PORT_IMPULSE(3) // coin
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", ticket_dispenser_device, line_r) // hopper sensor
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )                            PORT_NAME("Stats / Test")   // Bookkeeping.
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE )                         PORT_NAME("Settings")   // Game Rate & others.
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )  // payout
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )  // key out?

	PORT_START( "INC" ) // d00c
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", ticket_dispenser_device, line_r) // hopper sensor
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )                           PORT_NAME("Reset")  // hard reset
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
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )                            PORT_NAME("Stats / Test")   // Bookkeeping.
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE )                         PORT_NAME("Settings")   // Game Rate & others.
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )

	PORT_START( "INC" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )    PORT_NAME("Hold 4 / Small")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )                           PORT_NAME("Reset Switch")   // hard reset
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
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )                            PORT_NAME("Stats / Test")   // Bookkeeping.
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE )                         PORT_NAME("Settings")   // Game Rate & others.
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )

	PORT_START( "INC" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )    PORT_NAME("Hold 4 / Small")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )                           PORT_NAME("Reset Switch")   // hard reset
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
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )                            PORT_NAME("Stats / Test")   // Bookkeeping.
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE )                         PORT_NAME("Settings")   // Game Rate & others.
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )

	PORT_START( "INC" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )    PORT_NAME("Hold 4 / Small")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )                           PORT_NAME("Reset Switch")   // hard reset
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

void subsino_state::victor21(machine_config &config)
{
	// basic machine hardware
	HD647180X(config, m_maincpu, XTAL(12'000'000));   // Unknown clock
	m_maincpu->set_addrmap(AS_PROGRAM, &subsino_state::victor21_map);
	m_maincpu->set_addrmap(AS_IO, &subsino_state::subsino_iomap);

	i8255_device &ppi(I8255A(config, "ppi"));
	ppi.out_pa_callback().set(FUNC(subsino_state::out_a_w));
	ppi.tri_pa_callback().set_constant(0);
	ppi.out_pb_callback().set(FUNC(subsino_state::out_b_w));
	ppi.tri_pb_callback().set_constant(0);
	ppi.in_pc_callback().set_ioport("INC");

	TICKET_DISPENSER(config, m_hopper, attotime::from_msec(200));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(512, 256);
	screen.set_visarea(0, 512-1, 0+16, 256-16-1);
	screen.set_screen_update(FUNC(subsino_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, subsino_depth3);

	PALETTE(config, m_palette, FUNC(subsino_state::_2proms_palette), 0x100);

	MCFG_VIDEO_START_OVERRIDE(subsino_state,subsino)

	// sound hardware
	SPEAKER(config, "mono").front_center();

	YM2413(config, "ymsnd", XTAL(3'579'545)).add_route(ALL_OUTPUTS, "mono", 1.0);

	OKIM6295(config, "oki", XTAL(4'433'619) / 4, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0);  // Clock frequency & pin 7 not verified
}

// same but with an additional protection.
void subsino_state::victor5(machine_config &config)
{
	victor21(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &subsino_state::victor5_map);
}


void subsino_state::crsbingo(machine_config &config)
{
	// basic machine hardware
	HD647180X(config, m_maincpu, XTAL(12'000'000));   // Unknown CPU and clock
	m_maincpu->set_addrmap(AS_PROGRAM, &subsino_state::crsbingo_map);
	m_maincpu->set_addrmap(AS_IO, &subsino_state::subsino_iomap);

	TICKET_DISPENSER(config, m_hopper, attotime::from_msec(200));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(512, 256);
	screen.set_visarea(0, 512-1, 0+16, 256-16-1);
	screen.set_screen_update(FUNC(subsino_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, subsino_depth4);

	PALETTE(config, m_palette, FUNC(subsino_state::_2proms_palette), 0x100);

	MCFG_VIDEO_START_OVERRIDE(subsino_state,subsino)

	// sound hardware
	SPEAKER(config, "mono").front_center();

	YM2413(config, "ymsnd", XTAL(3'579'545)).add_route(ALL_OUTPUTS, "mono", 1.0);   // Unknown clock
}


void subsino_state::srider(machine_config &config)
{
	// basic machine hardware
	HD647180X(config, m_maincpu, XTAL(12'000'000));   // Unknown clock
	m_maincpu->set_addrmap(AS_PROGRAM, &subsino_state::srider_map);
	m_maincpu->set_addrmap(AS_IO, &subsino_state::subsino_iomap);

	i8255_device &ppi1(I8255A(config, "ppi1"));
	ppi1.in_pa_callback().set_ioport("SW1");
	ppi1.in_pb_callback().set_ioport("SW2");
	ppi1.in_pc_callback().set_ioport("SW3");

	i8255_device &ppi2(I8255A(config, "ppi2"));
	ppi2.in_pa_callback().set_ioport("SW4");
	ppi2.in_pb_callback().set_ioport("INA");
	ppi2.in_pc_callback().set_ioport("INB");

	TICKET_DISPENSER(config, m_hopper, attotime::from_msec(200));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(512, 256);
	screen.set_visarea(0, 512-1, 0+16, 256-16-1);
	screen.set_screen_update(FUNC(subsino_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, subsino_depth4);

	PALETTE(config, m_palette, FUNC(subsino_state::_3proms_palette), 0x100);

	MCFG_VIDEO_START_OVERRIDE(subsino_state,subsino)

	// sound hardware
	SPEAKER(config, "mono").front_center();

	YM3812(config, "ymsnd", XTAL(3'579'545)).add_route(ALL_OUTPUTS, "mono", 1.0);

	OKIM6295(config, "oki", XTAL(4'433'619) / 4, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0);  // Clock frequency & pin 7 not verified
}

void subsino_state::sharkpy(machine_config &config)
{
	srider(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &subsino_state::sharkpy_map);
}

void subsino_state::dinofmly(machine_config &config)
{
	srider(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &subsino_state::dinofmly_map);

	PALETTE(config.replace(), m_palette).set_entries(0x100);

	ramdac_device &ramdac(RAMDAC(config, "ramdac", 0, m_palette)); // HMC HM86171 VGA 256 colour RAMDAC
	ramdac.set_addrmap(0, &subsino_state::ramdac_map);
}

void subsino_state::tisub(machine_config &config)
{
	// basic machine hardware
	HD647180X(config, m_maincpu, XTAL(12'000'000));   // Unknown CPU and clock
	m_maincpu->set_addrmap(AS_PROGRAM, &subsino_state::tisub_map);
	m_maincpu->set_addrmap(AS_IO, &subsino_state::subsino_iomap);

	i8255_device &ppi1(I8255A(config, "ppi1"));
	ppi1.in_pa_callback().set_ioport("SW1");
	ppi1.in_pb_callback().set_ioport("SW2");
	ppi1.in_pc_callback().set_ioport("SW3");

	i8255_device &ppi2(I8255A(config, "ppi2"));
	ppi2.in_pa_callback().set_ioport("SW4");
	ppi2.in_pb_callback().set_ioport("INA");
	ppi2.in_pc_callback().set_ioport("INB");

	TICKET_DISPENSER(config, m_hopper, attotime::from_msec(200));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(512, 256);
	screen.set_visarea(0, 512-1, 0+16, 256-16-1);
	screen.set_screen_update(FUNC(subsino_state::screen_update_reels));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, subsino_depth4_reels);

	PALETTE(config, m_palette, FUNC(subsino_state::_3proms_palette), 0x100);

	MCFG_VIDEO_START_OVERRIDE(subsino_state, reels)

	// sound hardware
	SPEAKER(config, "mono").front_center();

	YM3812(config, "ymsnd", XTAL(3'579'545)).add_route(ALL_OUTPUTS, "mono", 1.0);   // Unknown clock
}

void subsino_state::newhunterb(machine_config &config)
{
	tisub(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &subsino_state::newhunterb_map);
}

void subsino_state::newhunterd(machine_config &config)
{
	tisub(config);

	Z80(config.replace(), m_maincpu, 6'000'000); // unknown clock
	m_maincpu->set_addrmap(AS_PROGRAM, &subsino_state::newhunterd_map);

	subdevice<screen_device>("screen")->screen_vblank().set_inputline(m_maincpu, 0, HOLD_LINE); // ? (needs some kind of source for IM 1 interrupt)
}

void subsino_state::stbsub(machine_config &config)
{
	// basic machine hardware
	HD647180X(config, m_maincpu, XTAL(12'000'000));   // Unknown clock
	m_maincpu->set_addrmap(AS_PROGRAM, &subsino_state::stbsub_map);
	m_maincpu->set_addrmap(AS_IO, &subsino_state::subsino_iomap);

	i8255_device &ppi1(I8255A(config, "ppi1"));
	ppi1.in_pa_callback().set_ioport("SW1");
	ppi1.in_pb_callback().set_ioport("SW2");
	ppi1.in_pc_callback().set_ioport("SW3");

	i8255_device &ppi2(I8255A(config, "ppi2"));
	ppi2.in_pa_callback().set_ioport("SW4");
	ppi2.in_pb_callback().set_ioport("INB");
	ppi2.in_pc_callback().set_ioport("INA");

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
	TICKET_DISPENSER(config, m_hopper, attotime::from_msec(200));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(512, 256);
	screen.set_visarea(0, 512-1, 0+16, 256-16-1);
	screen.set_screen_update(FUNC(subsino_state::screen_update_stbsub_reels));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, subsino_stbsub);

	PALETTE(config, m_palette).set_entries(0x100);
	//PALETTE(config, m_palette, FUNC(subsino_state::_3proms_palette), 0x100);

	ramdac_device &ramdac(RAMDAC(config, "ramdac", 0, m_palette)); // HMC HM86171 VGA 256 colour RAMDAC
	ramdac.set_addrmap(0, &subsino_state::ramdac_map);

	MCFG_VIDEO_START_OVERRIDE(subsino_state,stbsub)

	// sound hardware
	SPEAKER(config, "mono").front_center();

	YM3812(config, "ymsnd", XTAL(3'579'545)).add_route(ALL_OUTPUTS, "mono", 1.0);
}

void subsino_state::mtrainnv(machine_config &config)
{
	stbsub(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &subsino_state::mtrainnv_map);
}


/***************************************************************************
*                               ROMs Loading                               *
***************************************************************************/

#define HD647180X_FAKE_INTERNAL_ROM \
	ROM_FILL( 0x0000, 1, 0x3e ) \
	ROM_FILL( 0x0001, 1, 0x01 ) \
	ROM_FILL( 0x0002, 1, 0xed ) \
	ROM_FILL( 0x0003, 1, 0x39 ) \
	ROM_FILL( 0x0004, 1, 0x38 ) \
	ROM_FILL( 0x0005, 1, 0xc3 ) \
	ROM_FILL( 0x0006, 1, 0x16 ) \
	ROM_FILL( 0x0007, 1, 0xf0 )

/***************************************************************************

  Victor 5
  (C)1991 Subsino / Buffy

  Original Subsino PCB
  with CPU brick.

  Dumped by Team Europe.

***************************************************************************/

ROM_START( victor5 )
	ROM_REGION( 0x4000, "maincpu", 0 )
	HD647180X_FAKE_INTERNAL_ROM

	ROM_REGION( 0x10000, "program", 0 )
	ROM_LOAD( "1.u1", 0x00000, 0x10000, CRC(bc4d6ed6) SHA1(6b2087360ea0ae9e48a623934cb2fb973a80f1ec) )

	ROM_REGION( 0x18000, "tilemap", 0 )
	ROM_LOAD( "2.u24", 0x10000, 0x8000, CRC(f7026c74) SHA1(75a72839ad6b349563110ed10ad235958d5c0170) )
	ROM_LOAD( "3.u25", 0x08000, 0x8000, CRC(24ebe112) SHA1(61c32bb76c7600837880f468829dba176f8330f3) )
	ROM_LOAD( "4.u26", 0x00000, 0x8000, CRC(889baf02) SHA1(a2d01f3c09a69bd5b38531b41c53c550a03de229) )

	ROM_REGION( 0x20000, "oki", ROMREGION_ERASE )
	ROM_LOAD( "6.u49", 0x00000, 0x10000, CRC(73fb4f7b) SHA1(23db0ff42503847c6c7ebb364985430a48de4d8a) )
	ROM_LOAD( "5.u48", 0x10000, 0x10000, CRC(403d5632) SHA1(844e1a4bdf7cc9c1196f79e75a83f03a964feb16) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s129.u35", 0x000, 0x100, CRC(40094bed) SHA1(b25d96126b3f7bd06bf76dc9958f8669f83abdb7) )
	ROM_LOAD( "82s129.u36", 0x100, 0x100, CRC(9ca021c5) SHA1(6a1d8d4f958d36e4a676dc4f4aee83d637933bc3) )
ROM_END

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

ROM_START( victor5a )
	ROM_REGION( 0x4000, "maincpu", 0 )
	HD647180X_FAKE_INTERNAL_ROM

	ROM_REGION( 0x10000, "program", 0 )
	ROM_LOAD( "1.u1", 0x00000, 0x10000, CRC(e3ada2fc) SHA1(eddb460dcb80a29fbbe3ed6c4733c75b892baf52) )

	ROM_REGION( 0x18000, "tilemap", 0 )
	ROM_LOAD( "2.u24", 0x10000, 0x8000, CRC(1229e951) SHA1(1e548625bb60e2d6f52a376a0dea9e5709f94135) )
	ROM_LOAD( "3.u23", 0x08000, 0x8000, CRC(2d89bbf1) SHA1(d7fda0174a835e88b330dfd09bdb604bfe4c2e44) )
	ROM_LOAD( "4.u22", 0x00000, 0x8000, CRC(ecf840a1) SHA1(9ecf522afb23e3557d37effc3c8568e8a14dad1a) )

	ROM_REGION( 0x20000, "oki", ROMREGION_ERASE )
	ROM_LOAD( "6.u49", 0x00000, 0x10000, CRC(73fb4f7b) SHA1(23db0ff42503847c6c7ebb364985430a48de4d8a) )
	ROM_LOAD( "5.u48", 0x10000, 0x10000, CRC(403d5632) SHA1(844e1a4bdf7cc9c1196f79e75a83f03a964feb16) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s129.u35", 0x000, 0x100, CRC(40094bed) SHA1(b25d96126b3f7bd06bf76dc9958f8669f83abdb7) )
	ROM_LOAD( "82s129.u36", 0x100, 0x100, CRC(9ca021c5) SHA1(6a1d8d4f958d36e4a676dc4f4aee83d637933bc3) )
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
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_FILL( 0x0000, 1, 0x3e )
	ROM_FILL( 0x0001, 1, 0x01 )
	ROM_FILL( 0x0002, 1, 0xed )
	ROM_FILL( 0x0003, 1, 0x39 )
	ROM_FILL( 0x0004, 1, 0x38 )
	ROM_FILL( 0x0005, 1, 0xc3 )
	ROM_FILL( 0x0006, 1, 0x15 ) // entrypoint is slightly different
	ROM_FILL( 0x0007, 1, 0xf0 )

	ROM_REGION( 0x10000, "program", 0 )
	ROM_LOAD( "1.u1", 0x00000, 0x10000, CRC(43999b2d) SHA1(7ce26fd332ffe35fd826a1a6166b228d4bc370b8) )

	ROM_REGION( 0x18000, "tilemap", 0 )
	ROM_LOAD( "2.u24", 0x10000, 0x8000, CRC(f1181b93) SHA1(53cd4d2ce13973495b51d911a4745a69a9784983) )
	ROM_LOAD( "3.u25", 0x08000, 0x8000, CRC(437abb27) SHA1(bd3790807d60a41d58e07f60fb990553076d6e96) )
	ROM_LOAD( "4.u26", 0x00000, 0x8000, CRC(e2f66eee) SHA1(ece924fe626f21fd7d31faabf19225d80e2bcfd3) )

	ROM_REGION( 0x20000, "oki", ROMREGION_ERASE )
	ROM_LOAD( "6.u49", 0x00000, 0x10000, CRC(4153711c) SHA1(11b4f5f8ec3c93194d1d5b78ae35ca79d8f66a16) )
	ROM_LOAD( "5.u48", 0x10000, 0x10000, CRC(3d451de6) SHA1(cbb22679fc9ce27e2ca90aa8035bf1b1c353c69e) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s129.u35", 0x000, 0x100, CRC(40094bed) SHA1(b25d96126b3f7bd06bf76dc9958f8669f83abdb7) )
	ROM_LOAD( "82s129.u36", 0x100, 0x100, CRC(9ca021c5) SHA1(6a1d8d4f958d36e4a676dc4f4aee83d637933bc3) )
ROM_END


/***************************************************************************

  Treasure Island
  -- this has an extra layer for the reels, exactly the same as goldstar.cpp

***************************************************************************/

ROM_START( tisub )
	ROM_REGION( 0x4000, "maincpu", 0 )
	HD647180X_FAKE_INTERNAL_ROM

	ROM_REGION( 0x10000, "program", 0 )
	ROM_LOAD( "rom_1.bin", 0x00000, 0x10000, CRC(ed3b4a69) SHA1(c57985e8d19b2b495fc768e52b83cbbd75f027ad) )

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
	ROM_REGION( 0x4000, "maincpu", 0 )
	HD647180X_FAKE_INTERNAL_ROM

	ROM_REGION( 0x10000, "program", 0 )
	ROM_LOAD( "01.bin", 0x00000, 0x10000, CRC(9967dd38) SHA1(63b74bc0c0952114b7321e8f399bd64dc293aade) )

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

ROM_START( tisubb )
	ROM_REGION( 0x4000, "maincpu", 0 )
	HD647180X_FAKE_INTERNAL_ROM

	ROM_REGION( 0x10000, "program", 0 )
	ROM_LOAD( "ti alpha rom_1 ver3.0n.u6", 0x00000, 0x10000, CRC(7f0756cc) SHA1(e383b6abea136b79acabbffd9d247cc51c9aaa85) )

	ROM_REGION( 0x40000, "tilemap", 0 )
	ROM_LOAD( "ti alpha rom_6 ver1.1.u43", 0x00000, 0x10000, CRC(e7abd3b6) SHA1(bb22161b6c082d3ea0c80e0ff413bc43edd6e678) )
	ROM_LOAD( "ti alpha rom_4 ver1.1.u33", 0x10000, 0x10000, CRC(a4d13ea2) SHA1(e6366411c0f4b672e788a36a72a529449a5dd305) )
	ROM_LOAD( "ti alpha rom_5 ver1.1.u44", 0x20000, 0x10000, CRC(23f9eaca) SHA1(b130b4c2cadc63224d4d3dd2d957398c1338ae18) )
	ROM_LOAD( "ti alpha rom_7 ver1.1.u34", 0x30000, 0x10000, CRC(6f388cee) SHA1(bafdf44ae6e57d4114b6e008744628d2175ae023) )

	ROM_REGION( 0x8000, "reels", 0 )
	ROM_LOAD( "ti alpha rom_2 ver1.1.u19", 0x0000, 0x4000, CRC(2b009e98) SHA1(f91e304006f37fbd25d9752297d59f62e5786251) )
	ROM_IGNORE(0x4000)
	ROM_LOAD( "ti alpha rom_3 ver1.1.u20", 0x4000, 0x4000, CRC(52335a53) SHA1(d932fa755a8a98ee26ecf25594aca1d3d8af4f99) )
	ROM_IGNORE(0x4000)

	ROM_REGION( 0x300, "proms", 0 ) // not dumped for this set, but should be correct
	ROM_LOAD( "n82s129n.u39", 0x000, 0x100, BAD_DUMP CRC(971843e5) SHA1(4cb5fc1085503dae2f2f02eb49cca051ac84b890) )
	ROM_LOAD( "n82s129n.u40", 0x100, 0x100, BAD_DUMP CRC(b4bd872c) SHA1(c0f9fe68186636d6d6bc6f81415459631cf38edd) )
	ROM_LOAD( "n82s129n.u41", 0x200, 0x100, BAD_DUMP CRC(db99f6da) SHA1(d281a2fa06f1890ef0b1c4d099e6828827db14fd) )
ROM_END

/* This bootleg shows year 1989 on title screen, but it's from 1992 (there are 1992 strings on the ROMs). Probably Karam
   modified the New HUNTer set from Mecca, which also shows 1989, and was legally registered on Korea on 1989-8-25.
   Hardware:
    - Sound: SM64 + SM65 + 12.000 MHz xtal (currently, the pitch is wrong, you have to overclock ymsnd to 112% to get the same as the PCB).
    - Five positions for banks of eight DIP switches, but four of them unpopulated (only one present on the PCB).
    - MCU with its surface scratched out, probably an HD647180X0CP8L + 12.000 MHz xtal.
    - TI TPC1020AFN-084C.
    - Unpopulated locations on the PCB for a battery and a reset switch. */
ROM_START( newhunter )
	ROM_REGION( 0x4000, "maincpu", 0 )  // The MCU had its surface scratched out, but almost sure it's an HD647180X0CP8L
	ROM_LOAD( "hd647180.bin", 0x00000, 0x04000, NO_DUMP )
	HD647180X_FAKE_INTERNAL_ROM

	ROM_REGION( 0x10000, "program", 0 )
	ROM_LOAD( "27c512.u18",   0x00000, 0x10000, CRC(d0d863a7) SHA1(0ee5ee04c3da83320bea8130be30f7fe6446b32f) )

	ROM_REGION( 0x40000, "tilemap", 0 )
	ROM_LOAD( "d27c010a.u16", 0x00000, 0x08000, CRC(c1daa20f) SHA1(c46278a00b6cbbd79c4184db910bf2770d77d92f) )
	ROM_CONTINUE(             0x10000, 0x08000 )
	ROM_CONTINUE(             0x08000, 0x08000 )
	ROM_CONTINUE(             0x18000, 0x08000 )
	ROM_LOAD( "27c010.u17",   0x20000, 0x08000, CRC(bfd2bfb5) SHA1(4ee11e2a86f36744eff347df4e3801c731406147) )
	ROM_CONTINUE(             0x30000, 0x08000 )
	ROM_CONTINUE(             0x28000, 0x08000 )
	ROM_CONTINUE(             0x38000, 0x08000 )

	ROM_REGION( 0x08000, "reels", 0 )
	ROM_LOAD( "27c512.u14",   0x00000, 0x04000, CRC(e7632aaa) SHA1(e502554f84130f5a8cd86007f20f958cd303c0eb) )
	ROM_IGNORE(0xc000)
	ROM_LOAD( "27c512.u15",   0x04000, 0x04000, CRC(44933beb) SHA1(a19ed785cc3b38c2a2a6a08e9d639361ee118343) )
	ROM_IGNORE(0xc000)

	ROM_REGION( 0x00300, "proms", 0 )
	ROM_LOAD( "hu-1.bin",     0x00000, 0x00100, CRC(971843e5) SHA1(4cb5fc1085503dae2f2f02eb49cca051ac84b890) )
	ROM_LOAD( "hu-2.bin",     0x00100, 0x00100, CRC(b4bd872c) SHA1(c0f9fe68186636d6d6bc6f81415459631cf38edd) )
	ROM_LOAD( "hu-3.bin",     0x00200, 0x00100, CRC(db99f6da) SHA1(d281a2fa06f1890ef0b1c4d099e6828827db14fd) )

	ROM_REGION( 0x00100, "xtraprom", 0 )
	ROM_LOAD( "82s129.u34",   0x00000, 0x00100, CRC(72d30ca3) SHA1(1054da8e0c18fac8d9fd93cfa441a17df430686b) ) // There's an additional PROM near the MCU

	ROM_REGION( 0x00045c, "plds", 0 )
	ROM_LOAD( "ht-1.bin",     0x00000, 0x00117, CRC(fdb4cd61) SHA1(f510077b707864b2536942db6157118ca15922de) )
	ROM_LOAD( "ht-2.bin",     0x00117, 0x00117, CRC(0cf55cec) SHA1(845395ca0587627331b9ac48777f7cb6b54b9401) )
	ROM_LOAD( "ht-3.bin",     0x0022e, 0x00117, CRC(8272668f) SHA1(9037f0d9c7625d05d2087e6f2d159dece934a945) )
	ROM_LOAD( "ht-4.bin",     0x00345, 0x00117, CRC(f4f78925) SHA1(738281c05c0f51c0edcf65bacc18ebc87c2408c1) )
ROM_END

/*  The PCB had five positions for banks of eight DIP switches, but four of them were unpopulated (only one was present on the PCB).
    The position for the reset switch was also unpopulated, and there wasn't battery-backed RAM on the PCB
    (present on most games on this hardware). */
ROM_START( newhunterb )
	ROM_REGION( 0x04000, "maincpu", 0 )
	ROM_LOAD( "hd647180.bin", 0x00000, 0x04000, NO_DUMP ) // HD647180X0CP8L
	HD647180X_FAKE_INTERNAL_ROM

	ROM_REGION( 0x10000, "program", 0 )
	ROM_LOAD( "tms27c512.u26", 0x00000, 0x10000, CRC(a155197c) SHA1(01c53b436de14717260c88352b09f1a01b6fd8b2) )

	ROM_REGION( 0x40000, "tilemap", 0 )
	ROM_LOAD( "am27c010.u24", 0x00000, 0x08000, CRC(e712ab8a) SHA1(ddeb92213124c6114951b9b5739eed25efe3c7a6) )
	ROM_CONTINUE(             0x10000, 0x08000 )
	ROM_CONTINUE(             0x08000, 0x08000 )
	ROM_CONTINUE(             0x18000, 0x08000 )
	ROM_LOAD( "i27c010.u25",  0x20000, 0x08000, CRC(f13c08e3) SHA1(f25ce6e73be9ee23f8a75fc6771d8224c4dd07b8) )
	ROM_CONTINUE(             0x30000, 0x08000 )
	ROM_CONTINUE(             0x28000, 0x08000 )
	ROM_CONTINUE(             0x38000, 0x08000 )

	ROM_REGION( 0x08000, "reels", 0 )
	ROM_LOAD( "27c512.u29",   0x00000, 0x04000, CRC(dbe8bacd) SHA1(a0bdcb62b3a3b0d649c3cc63123882c33bcf17bd) )
	ROM_IGNORE(0xc000)
	ROM_LOAD( "m27512.u30",   0x04000, 0x04000, CRC(74d9b34f) SHA1(355157cf019a4fb3177ecf129b5237502bc8026b) )
	ROM_IGNORE(0xc000)

	ROM_REGION( 0x00300, "proms", 0 )
	ROM_LOAD( "hu-1.bin",     0x00000, 0x00100, CRC(971843e5) SHA1(4cb5fc1085503dae2f2f02eb49cca051ac84b890) )
	ROM_LOAD( "hu-2.bin",     0x00100, 0x00100, CRC(b4bd872c) SHA1(c0f9fe68186636d6d6bc6f81415459631cf38edd) )
	ROM_LOAD( "hu-3.bin",     0x00200, 0x00100, CRC(db99f6da) SHA1(d281a2fa06f1890ef0b1c4d099e6828827db14fd) )

	ROM_REGION( 0x00045c, "plds", 0 )
	ROM_LOAD( "ht-1.bin",     0x00000, 0x00117, CRC(fdb4cd61) SHA1(f510077b707864b2536942db6157118ca15922de) )
	ROM_LOAD( "ht-2.bin",     0x00117, 0x00117, CRC(0cf55cec) SHA1(845395ca0587627331b9ac48777f7cb6b54b9401) )
	ROM_LOAD( "ht-3.bin",     0x0022e, 0x00117, CRC(8272668f) SHA1(9037f0d9c7625d05d2087e6f2d159dece934a945) )
	ROM_LOAD( "ht-4.bin",     0x00345, 0x00117, CRC(f4f78925) SHA1(738281c05c0f51c0edcf65bacc18ebc87c2408c1) )
ROM_END

/*  Several custom modifications on this PCB:
     -3V battery directly soldered to the main HM62256ALP-10 RAM.
     -One bank of 8 DIP switches soldered to a 74LS245.
     -One bank of 5 DIP switches soldered to another 74LS245.
     -A single switch button soldered to another 74LS245.
   And the usual hardware configuration:
     -One 12.000 MHz xtal.
     -HD647180X0CP6.
     -12 MHz xtal.
     -One bank of 8 DIP switches, plus the other two banks described before.
     -Sound: K-666 + K-664.
     -TI TPC1020AFN-084C. */
ROM_START( newhunterc )
	ROM_REGION( 0x04000, "maincpu", 0 )
	ROM_LOAD( "hd647180.bin", 0x00000, 0x04000, NO_DUMP ) // HD647180X0CP8L
	HD647180X_FAKE_INTERNAL_ROM

	ROM_REGION( 0x10000, "program", 0 )
	ROM_LOAD( "nm27c512q.u26", 0x00000, 0x10000, CRC(fe79a7da) SHA1(f326070f8a15aa518e1c928890549e63a99e6961) )

	ROM_REGION( 0x40000, "tilemap", 0 )
	ROM_LOAD( "e27c010u.u24",  0x00000, 0x08000, CRC(893d8e6d) SHA1(e18fa4db54230eef92b78ccd5fe3ea509b83a1ec) )
	ROM_CONTINUE(              0x10000, 0x08000 )
	ROM_CONTINUE(              0x08000, 0x08000 )
	ROM_CONTINUE(              0x18000, 0x08000 )
	ROM_LOAD( "ds40986.u25",   0x20000, 0x08000, CRC(3b46a3a1) SHA1(db98ed804def24d069e09623588f37a60a2cce46) )
	ROM_CONTINUE(              0x30000, 0x08000 )
	ROM_CONTINUE(              0x28000, 0x08000 )
	ROM_CONTINUE(              0x38000, 0x08000 )

	ROM_REGION( 0x08000, "reels", 0 )
	ROM_LOAD( "tms27c512.u29", 0x00000, 0x04000, CRC(ad203af2) SHA1(58314dc795fba157bd9c3da64e7e158e13376860) )
	ROM_IGNORE(0xc000)
	ROM_LOAD( "tms27c512.u30", 0x04000, 0x04000, CRC(27a3910e) SHA1(6c9f5061f8a278d5a570f0f10f4f01b86558fe46) )
	ROM_IGNORE(0xc000)

	ROM_REGION( 0x00300, "proms", 0 )
	ROM_LOAD( "n82s129an.u1",  0x00000, 0x00100, CRC(69af17fc) SHA1(38546f5665cf731195ac384aca182c56884333f0) )
	ROM_LOAD( "n82s129an.u2",  0x00100, 0x00100, CRC(4b5f288a) SHA1(b6b9f9067afe93bd13ea17311484e2a2af01a0ed) )
	ROM_LOAD( "n82s129an.u3",  0x00200, 0x00100, CRC(a1c0d069) SHA1(794df68451525901ebd5895feb26fcda2c517c3f) )

	ROM_REGION( 0x00045c, "plds", 0 )
	ROM_LOAD( "gal16v8b.u41",  0x00000, 0x00117, CRC(fdb4cd61) SHA1(f510077b707864b2536942db6157118ca15922de) )
	ROM_LOAD( "gal16v8b.s63",  0x00117, 0x00117, CRC(f4f78925) SHA1(738281c05c0f51c0edcf65bacc18ebc87c2408c1) )
	ROM_LOAD( "gal16v8b.u42",  0x0022e, 0x00117, CRC(0cf55cec) SHA1(845395ca0587627331b9ac48777f7cb6b54b9401) )
	ROM_LOAD( "gal16v8b.u9",   0x00345, 0x00117, CRC(8272668f) SHA1(9037f0d9c7625d05d2087e6f2d159dece934a945) )
ROM_END

/* Hardware:
    - Sound: SM64 + SM65.
    - 12.000 MHz xtal (only one xtal on the PCB).
    - Five positions for banks of eight DIP switches, but four of them unpopulated (only one present on the PCB).
    - MCU with its surface scratched out. Appears to run in external ROM mode.
   Direct recording from PCB for reference: https://youtu.be/0cFNFCqEEQo */
ROM_START( newhunterd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1_am27c512.u44", 0x00000, 0x10000, CRC(456bdb88) SHA1(7135584576f7761b4a0b4c66318cd0cb649eeb93) )

	ROM_REGION( 0x40000, "tilemap", 0 )
	ROM_LOAD( "4_d27c010a.u42", 0x00000, 0x08000, CRC(c1daa20f) SHA1(c46278a00b6cbbd79c4184db910bf2770d77d92f) )
	ROM_CONTINUE(               0x10000, 0x08000 )
	ROM_CONTINUE(               0x08000, 0x08000 )
	ROM_CONTINUE(               0x18000, 0x08000 )
	ROM_LOAD( "5_am27c010.u41", 0x20000, 0x08000, CRC(b1f7bdde) SHA1(1b258450e8e73fd35feb2048a19659546cdceabd) )
	ROM_CONTINUE(               0x30000, 0x08000 )
	ROM_CONTINUE(               0x28000, 0x08000 )
	ROM_CONTINUE(               0x38000, 0x08000 )

	ROM_REGION( 0x08000, "reels", 0 )
	ROM_LOAD( "2_27c512.u3",    0x00000, 0x04000, CRC(bafd35ef) SHA1(5ef297e03d11faa97a79ed87eb73debf81af2b91) )
	ROM_IGNORE(0xc000)
	ROM_LOAD( "3_m27512.u43",   0x04000, 0x04000, CRC(44933beb) SHA1(a19ed785cc3b38c2a2a6a08e9d639361ee118343) )
	ROM_IGNORE(0xc000)

	ROM_REGION( 0x00300, "proms", 0 ) // Not dumped for this set, the ones from newhunterb seem to fit
	ROM_LOAD( "n82s129an.u67",  0x00000, 0x00100, BAD_DUMP CRC(971843e5) SHA1(4cb5fc1085503dae2f2f02eb49cca051ac84b890) )
	ROM_LOAD( "n82s129an.u68",  0x00100, 0x00100, BAD_DUMP CRC(b4bd872c) SHA1(c0f9fe68186636d6d6bc6f81415459631cf38edd) )
	ROM_LOAD( "n82s129an.u69",  0x00200, 0x00100, BAD_DUMP CRC(db99f6da) SHA1(d281a2fa06f1890ef0b1c4d099e6828827db14fd) )

	ROM_REGION( 0x00045c, "plds", 0 )
	ROM_LOAD( "palce16v8h.u61", 0x00000, 0x00117, NO_DUMP )
	ROM_LOAD( "palce16v8h.u6",  0x00117, 0x00117, NO_DUMP )
	ROM_LOAD( "palce16v8h.u30", 0x0022e, 0x00117, NO_DUMP )
	ROM_LOAD( "palce16v8h.u59", 0x00345, 0x00117, NO_DUMP )
	ROM_LOAD( "palce16v8h.u29", 0x00000, 0x00117, NO_DUMP )
	ROM_LOAD( "palce16v8h.u35", 0x00000, 0x00117, NO_DUMP )
ROM_END

/*  Korean title: 뉴 동물 동물
    PCB silkscreened "V 98.03.18". Four banks of eight DIP switches, unpopulated socket (SW5) for reset switch.
    Hardware: K-664 + K-666, HD647180X0CP8L, QL8X12B-XPL68C, 12.000 MHz xtal.
    Video from the real hardware: https://youtu.be/VtPV3DonIsY */
ROM_START( ndongmul )
	ROM_REGION( 0x04000, "maincpu", 0 )
	ROM_LOAD( "hd647180.bin", 0x00000, 0x04000, NO_DUMP ) // HD647180X0CP8L
	HD647180X_FAKE_INTERNAL_ROM

	ROM_REGION( 0x20000, "program", 0 )
	ROM_LOAD( "tms27c010a.u26", 0x10000, 0x10000, CRC(cf30ed7e) SHA1(138bbfa252769d19b976906a057e54e41135ebe7) )
	ROM_CONTINUE(               0x00000, 0x10000 )

	ROM_REGION( 0x40000, "tilemap", 0 )
	ROM_LOAD( "m27512.u24",     0x00000, 0x08000, CRC(c3a4460c) SHA1(a8c663354faded0012e8fc18d37fdbe76e301fc1) )
	ROM_CONTINUE(               0x10000, 0x08000 )
	ROM_LOAD( "nm27c512q.u24a", 0x08000, 0x08000, CRC(e86f4432) SHA1(d545087561c93690438affea8994da1e6e3ce8af) )
	ROM_CONTINUE(               0x18000, 0x08000 )
	ROM_LOAD( "d27c512.u25",    0x20000, 0x08000, CRC(9ca948c1) SHA1(bf407820b10bea1726e00786892188e3dc07018f) )
	ROM_CONTINUE(               0x30000, 0x08000 )
	ROM_LOAD( "m27c512.u25a",   0x28000, 0x08000, CRC(1b943b64) SHA1(518c0e18bb942c8756364d60d9390e02e79e94f4) )
	ROM_CONTINUE(               0x38000, 0x08000 )

	ROM_REGION( 0x20000, "reels", 0 )
	ROM_LOAD( "tms27c512.u29",  0x00000, 0x10000, BAD_DUMP CRC(c271fb5f) SHA1(980ab3f14b84dcb1802519047b6afdf0671259d2) ) // Bitrotten, address 0x00410 sometimes reads as 0002 and others as 0003
	ROM_LOAD( "d27512.u30",     0x10000, 0x10000, CRC(55ddc00a) SHA1(8d6287f22a73792a438214098451809604186cb0) )

	ROM_REGION( 0x00300, "proms", 0 )
	ROM_LOAD( "n82s129an.u1",   0x00000, 0x00100, CRC(69af17fc) SHA1(38546f5665cf731195ac384aca182c56884333f0) )
	ROM_LOAD( "n82s129an.u2",   0x00100, 0x00100, CRC(4b5f288a) SHA1(b6b9f9067afe93bd13ea17311484e2a2af01a0ed) )
	ROM_LOAD( "n82s129an.u3",   0x00200, 0x00100, CRC(a1c0d069) SHA1(794df68451525901ebd5895feb26fcda2c517c3f) )

	ROM_REGION( 0x00045c, "plds", 0 )
	ROM_LOAD( "palce16v8h.u27", 0x00000, 0x00117, CRC(bf7e1812) SHA1(f299fda423fce9f8ffd4df07e315bea3662479b4) )
	ROM_LOAD( "palce16v8h.u42", 0x00117, 0x00117, CRC(0cf55cec) SHA1(845395ca0587627331b9ac48777f7cb6b54b9401) )
	ROM_LOAD( "palce16v8h.u9",  0x0022e, 0x00117, CRC(8272668f) SHA1(9037f0d9c7625d05d2087e6f2d159dece934a945) )
	ROM_LOAD( "palce16v8h.u31", 0x00345, 0x00117, CRC(c40859b3) SHA1(e25e1c4311deaa174c8a7e015e55a1aecd83005e) )
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
	ROM_REGION( 0x4000, "maincpu", 0 )
	HD647180X_FAKE_INTERNAL_ROM

	ROM_REGION( 0x10000, "program", 0 )
	ROM_LOAD( "1.u36", 0x00000, 0x10000, CRC(c5aff4eb) SHA1(74f06d7735975657fca9be5fff9e7d53f38fcd02) )

	ROM_REGION( 0x20000, "tilemap", 0 )
	ROM_LOAD( "2.u4",  0x00000, 0x10000, CRC(ce527722) SHA1(f3759cefab902259eb25f8d4be2fcafc1afd90b9) )
	ROM_LOAD( "3.u15", 0x10000, 0x10000, CRC(23785451) SHA1(8574e59aa816a644ff4b102bd5754ef1284deea0) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s129.u13", 0x000, 0x100, CRC(89c06859) SHA1(b98d5335f36ea3842086677aca47b605030d442f) )
	ROM_LOAD( "82s129.u14", 0x100, 0x100, CRC(eaddb54f) SHA1(51fbf31e910a93315204a892d10bcf982a6ed099) )

	ROM_REGION( 0x40000, "oki", ROMREGION_ERASE00 )

	ROM_REGION( 0x155 * 2, "plds", 0 )
	ROM_LOAD( "18cv8.u22", 0x000, 0x155, NO_DUMP )
	ROM_LOAD( "18cv8.u29", 0x155, 0x155, NO_DUMP )
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
	ROM_REGION( 0x4000, "maincpu", 0 )
	HD647180X_FAKE_INTERNAL_ROM

	ROM_REGION( 0x10000, "program", 0 )
	ROM_LOAD( "shark_n.1.u18", 0x00000, 0x10000, CRC(25aeac2f) SHA1(d94e3e5cfffd150ac48e1463493a8323f42e7a89) )

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
	ROM_LOAD( "shark=ii=-italy_4_ver1.0.u54", 0x00000, 0x20000, CRC(9f384c59) SHA1(d2b087b8370b40b6f0944de661ea6aebaebea06f) )

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
	ROM_REGION( 0x4000, "maincpu", 0 )
	HD647180X_FAKE_INTERNAL_ROM

	ROM_REGION( 0x10000, "program", 0 )
	ROM_LOAD( "shark1.6.u18", 0x00000, 0x10000, CRC(365312a0) SHA1(de8370b1f35e8d071185d2e5f2fbd2fdf74c55ac) )

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
	ROM_LOAD( "shark=ii=-italy_4_ver1.0.u54", 0x00000, 0x20000, CRC(9f384c59) SHA1(d2b087b8370b40b6f0944de661ea6aebaebea06f) )

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
	ROM_REGION( 0x4000, "maincpu", 0 )
	HD647180X_FAKE_INTERNAL_ROM

	ROM_REGION( 0x10000, "program", 0 )
	ROM_LOAD( "sharkpye.u18", 0x00000, 0x10000, CRC(12473814) SHA1(9c24ed41781aefee0161add912e730ba0d4f4d3e) )

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
	ROM_REGION( 0x4000, "maincpu", 0 )
	HD647180X_FAKE_INTERNAL_ROM

	ROM_REGION( 0x10000, "program", 0 )
	ROM_LOAD( "victor_6ii-rom_1.ver2.3n.u18", 0x00000, 0x10000, CRC(d496ecbd) SHA1(1f982b42bc46c09298916a6cb2db0b38c6451ec3) )

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
	ROM_REGION( 0x4000, "maincpu", 0 )
	HD647180X_FAKE_INTERNAL_ROM

	ROM_REGION( 0x10000, "program", 0 )
	ROM_LOAD( "victor_6ii_alpha_1_ver2.3.u18", 0x00000, 0x10000, CRC(2a3eaecd) SHA1(18bf2dfec8cd5690d6465f750093942afda66475) )

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
	ROM_REGION( 0x4000, "maincpu", 0 )
	HD647180X_FAKE_INTERNAL_ROM

	ROM_REGION( 0x10000, "program", 0 )
	ROM_LOAD( "victor_6ii_rom_1_ver1.2.u18", 0x00000, 0x10000, CRC(309876fc) SHA1(305c4cf347b512607e2c58a580075a34b48bedd5) )

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

void subsino_state::init_smoto16()
{
	uint8_t *rom = memregion( "maincpu" )->base();
	rom[0x12d0] = 0x20; // "ERROR 951010"
}

/***************************************************************************

Super Rider (Italy Ver 2.0)
(C)1997 Subsino

Chips:

2x custom QFP44 label SUBSINOSS9100
1x custom DIP42 label SUBSINOSS9101
2x D8255AC-2 (equivalent to 8255)
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

ROM_START( smoto13 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rider out_1 ver1.3.u18", 0x00000, 0x10000, CRC(45a9ebb2) SHA1(216be0d93a9787593578343277fa82f2d8a2e75c) )

	ROM_REGION( 0x40000, "tilemap", 0 )
	ROM_LOAD( "rider rom_3 ver1.0.u16", 0x00000, 0x08000, CRC(511cccaa) SHA1(6d47f3d90049c141202c864a8ef6ed7d5a9077a4) )
	ROM_CONTINUE(      0x10000, 0x08000 )
	ROM_CONTINUE(      0x08000, 0x08000 )
	ROM_CONTINUE(      0x18000, 0x08000 )
	ROM_LOAD( "rider rom_2 ver1.0.u17", 0x20000, 0x08000, CRC(b0d3ec58) SHA1(c10008993c0b9368164e537386d14cb5e9aaf761) )
	ROM_CONTINUE(      0x30000, 0x08000 )
	ROM_CONTINUE(      0x28000, 0x08000 )
	ROM_CONTINUE(      0x38000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "rider rom_4 ver1.0.u54", 0x00000, 0x20000, CRC(df828563) SHA1(f39324c5c37486ed9512e0ff934394556dd182ae) )

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

  A version of Magic Train running on subsino.cpp (unlike mtrain, which is
  subsino2.cpp) is needed to match this program ROM.

***************************************************************************/

ROM_START( mtrainnv )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mtrain_settings.bin", 0x00000, 0x10000, CRC(584af1b5) SHA1(91d966d282823dddfdc455bb03728fcdf3713dd7) )

	ROM_REGION( 0x10000, "tilemap", 0 )
	ROM_LOAD( "mtrain_tilemap.bin", 0x00000, 0x10000, NO_DUMP )
	ROM_COPY( "maincpu", 0x000000, 0x00000, 0x10000 ) // just to show something

	ROM_REGION( 0x10000, "reels", 0 )
	ROM_LOAD( "mtrain_reels.bin", 0x00000, 0x10000, NO_DUMP )
	ROM_COPY( "maincpu", 0x000000, 0x00000, 0x10000 ) // just to show something
ROM_END

/*
 Seven Land (Korea, probably a bootleg of Super Treasure Island). String "KAM 1.2" on program ROM.
   MCU Hitachi HD64180RP6.
   Actel A40MX04-F.
   Sound: U6612 (YM3812) + U6614 (YM3014) + 12.000 MHz xtal.
   HM86171-80 RAMDAC.
   Four banks of 8 DIP switches labeled SW1 to SW4.
   Reset switch near the DIP switches banks.
   One unpopulated location on the PCB (near the sound chips) for another bank of 8 DIP switches, labeled SW0.
   3.6V battery near the HD64180RP6
   No bipolar PROMs on this PCB.
 Known issues:
   When setting "Clear Ticket Unit" DIP switch to 100, the game resets after betting 120.
   Lamps aren't working properly.
   There's a gray stripe on the title screen that isn't present on the real hardware.
*/

ROM_START( sevenlnd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a_am27c512.u58", 0x00000, 0x10000, CRC(7abaca14) SHA1(48e4eb4ef7df09f29a382167291ee6385279d1f5) )

	ROM_REGION( 0x100000, "tilemap", 0 )
	ROM_LOAD( "sevenlnd_tilemap_1.u30", 0x00000, 0x40000, CRC(17ebde1c) SHA1(8588830628ce09de57acf287b983bfcfcb0e35f2) ) // A278308
	ROM_LOAD( "sevenlnd_tilemap_3.u28", 0x40000, 0x40000, CRC(615f6f8f) SHA1(e75e935fd7e5d1a68c0e22d8dddcb706278833db) ) // A278308
	ROM_LOAD( "sevenlnd_tilemap_4.u27", 0x80000, 0x40000, CRC(f78b273e) SHA1(20bb58042db14af6c65804dfd24e5fb38ce68bd2) ) // A278308
	ROM_LOAD( "sevenlnd_tilemap_2.u29", 0xc0000, 0x40000, CRC(be95e805) SHA1(8d920e4a04a1f53f3354b031d1cc3bda5f13df44) ) // A278308

	ROM_REGION( 0x80000, "reels", 0 )
	ROM_LOAD( "sevenlnd_reels_1.u25", 0x00000, 0x20000, CRC(49ce7fea) SHA1(eed75c012da5cbcee0762a5692f81b824e9715c4) ) // 27C010
	ROM_LOAD( "sevenlnd_reels_2.u24", 0x20000, 0x20000, CRC(4d5c4550) SHA1(c78adf8ee38d82251bcb26d7b693a5155997c2d1) ) // 27C010
	ROM_LOAD( "sevenlnd_reels_3.u23", 0x40000, 0x20000, CRC(31b7e157) SHA1(5e07725a91d5087e97b2299e5e81356b6d0fb5e9) ) // 27C010
	ROM_LOAD( "sevenlnd_reels_4.u22", 0x60000, 0x20000, CRC(27d575bc) SHA1(e2a0164cf57f39554cace53b3d894c0e32ad2f56) ) // 27C010

	ROM_REGION( 0x157, "plds", 0 )
	ROM_LOAD( "gal16v8d_1.bin", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8d_2.bin", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8d_3.bin", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "gal16v8d_4.bin", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "palce20v8h.bin", 0x000, 0x157, NO_DUMP )
ROM_END

/* Lucky Seven. String "KAM 1.2" on program ROM.
   Same PCB as "sevenlnd", but with a PAL soldered to SW0 dip switches bank (this socket is empty on "sevenlnd").
   Bad graphics on title screen, but happens also on real hardware. Mismatched program and tile ROMs,
   a poor effort at a hack, or a buggy development version?
   There's a gray stripe on the title screen that isn't present on the real hardware.
   Recording from real hardware: https://youtu.be/By7Xi5jf2Qc
*/
ROM_START( luckyseven )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "m27c512.u12",         0x00000, 0x10000, CRC(7abaca14) SHA1(48e4eb4ef7df09f29a382167291ee6385279d1f5) )

	ROM_REGION( 0x100000, "tilemap", 0 )
	ROM_LOAD( "m27c2001.u30",        0x00000, 0x40000, CRC(7fe8562c) SHA1(dec6d1bf4026a0cc5afb34838a5a5d7e480d2e9d) )
	ROM_LOAD( "tms27c020.u29",       0x40000, 0x40000, CRC(61208b5b) SHA1(148c1e639245169b7ebfd0f2b75de4f179cdfc54) )
	ROM_LOAD( "m27c2001.u28",        0x80000, 0x40000, CRC(0eef9939) SHA1(50268863df7602fe5264867eb9b65f00e78f424a) )
	ROM_LOAD( "am27c020.u27",        0xc0000, 0x40000, CRC(1502d4c7) SHA1(a5b7897004d70aa1b69a547e8c6a1756bd83ccb6) )

	ROM_REGION( 0x80000, "reels", 0 )
	ROM_LOAD( "tms27c010a.u25",      0x00000, 0x20000, CRC(4f28aeb0) SHA1(686d003d674186d95eeda8139d89f2a39a703b41) )
	ROM_LOAD( "am27c010.u24",        0x20000, 0x20000, CRC(4f28aeb0) SHA1(686d003d674186d95eeda8139d89f2a39a703b41) ) // Same content as U25
	ROM_LOAD( "m27c1001.u23",        0x40000, 0x20000, CRC(fdea6687) SHA1(7d352b1675380a8bb61af2a73ac3b85ce5ac433c) )
	ROM_LOAD( "am27c010.u22",        0x60000, 0x20000, CRC(c8b90af5) SHA1(ff1bf2c6d2b8d0f2926127bc74bc4488c7771cdd) )

	ROM_REGION( 0x157, "plds", 0 )
	ROM_LOAD( "palce16v8h-25.u1",    0x00000, 0x00117, CRC(57e44e7e) SHA1(ca92a40f2781ac11ffcfd9a7ef1e852b719fe35c) )
	ROM_LOAD( "palce16v8h-25.u2",    0x00000, 0x00117, CRC(8272668f) SHA1(9037f0d9c7625d05d2087e6f2d159dece934a945) )
	ROM_LOAD( "palce16v8h-25.u18",   0x00000, 0x00117, CRC(d88c5718) SHA1(80914932b3fd5b200ffb5fb8ac30b8636cfa72de) )
	ROM_LOAD( "palce16v8h-25.u43",   0x00000, 0x00117, CRC(cca094fd) SHA1(9c6b10e0c831b7ab5fd0c91bd357be38bc3df020) )
	ROM_LOAD( "palce20v8h-25pc.u55", 0x00000, 0x00157, CRC(c6c6fa81) SHA1(05fbd86db3624f67f766817d18f8b7d386d67b74) )
	ROM_LOAD( "palce16v8h-25.sw0",   0x00000, 0x00117, CRC(92bb58d6) SHA1(73e0626354738f74d1624d5a430a00b3e6e227d7) ) // Soldered to dip switches bank SW0
ROM_END

ROM_START( grndprix )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "m27c512.u12", 0x00000, 0x10000, CRC(d624ef87) SHA1(77c03cf4715906d64531abf1e3e85212107f1cde) )

	ROM_REGION( 0x100000, "tilemap", ROMREGION_ERASE00 )
	ROM_LOAD( "tms27c020.u30", 0x00000, 0x40000, NO_DUMP )
	ROM_LOAD( "am27c020.u29",  0x40000, 0x40000, CRC(a2ecf811) SHA1(10c1339040f39f63b5d3b706b35b417f244981a6) )
	ROM_LOAD( "m27c2001.u28",  0x80000, 0x40000, CRC(a923eb32) SHA1(b5da49cab5be7832592633bd4d8a7e37dc545e2a) )
	ROM_LOAD( "m27c2001.u27",  0xc0000, 0x40000, CRC(822f9138) SHA1(5673796684aa157288fbe02eab75598c61735fa9) )

	ROM_REGION( 0x80000, "reels", 0 )
	ROM_LOAD( "m27c1001.u25",   0x00000, 0x20000, CRC(42f3bac5) SHA1(33bff2c9904c3188d5d1076363fab4cf1b1f3946) )
	ROM_LOAD( "tms27c010a.u24", 0x20000, 0x20000, CRC(675a4e94) SHA1(c501e5f53b5c2610212d776341a14d692fc1a175) )
	ROM_LOAD( "27c010a.u23",    0x40000, 0x20000, CRC(7de68191) SHA1(0ca2869fbe146393bb567e28d937702892e0b2fd) )
	ROM_LOAD( "27c010a.u22",    0x60000, 0x20000, CRC(ff2c3e13) SHA1(cdaeb16a55487afdc53b4105d3dc653f20b21df4) )

	ROM_REGION( 0xa00, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "palce16v8h-25.u1",    0x000, 0x117, CRC(57e44e7e) SHA1(ca92a40f2781ac11ffcfd9a7ef1e852b719fe35c) )
	ROM_LOAD( "palce16v8h-25.u2",    0x200, 0x117, CRC(8272668f) SHA1(9037f0d9c7625d05d2087e6f2d159dece934a945) )
	ROM_LOAD( "palce16v8h-25.u18",   0x400, 0x117, CRC(d88c5718) SHA1(80914932b3fd5b200ffb5fb8ac30b8636cfa72de) )
	ROM_LOAD( "palce16v8h-25.u43",   0x600, 0x117, CRC(cca094fd) SHA1(9c6b10e0c831b7ab5fd0c91bd357be38bc3df020) )
	ROM_LOAD( "palce20v8h-25pc.u55", 0x800, 0x157, CRC(c6c6fa81) SHA1(05fbd86db3624f67f766817d18f8b7d386d67b74) )
ROM_END

ROM_START( dinofmly ) // very similar PCB to the smoto set, but instead of 3 PROMs it has a RAMDAC.
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dino iii tetris_1 ver1.3.u18", 0x00000, 0x10000, CRC(ddf09230) SHA1(1e83b17cfc64b5eba484abfc922a67c9c3e0d1bf) )

	ROM_REGION( 0x40000, "tilemap", 0 )
	ROM_LOAD( "dino iii tetris_3 ver1.0.u16", 0x00000, 0x08000, CRC(88319fdf) SHA1(f0e97476d9664a5bdf16c27568a2c044d0818fad) )
	ROM_CONTINUE(             0x10000, 0x08000 )
	ROM_CONTINUE(             0x08000, 0x08000 )
	ROM_CONTINUE(             0x18000, 0x08000 )
	ROM_LOAD( "dino iii tetris_2 ver1.0.u17", 0x20000, 0x08000, CRC(fa355811) SHA1(d2f40e648d0c9f72c38e39021897cba23f09a56f) )
	ROM_CONTINUE(             0x30000, 0x08000 )
	ROM_CONTINUE(             0x28000, 0x08000 )
	ROM_CONTINUE(             0x38000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "u54", 0x00000, 0x20000, CRC(4e2ef62a) SHA1(77dbc2a03619ad3608a27ed70e74f3e76431498d) ) // missing label
ROM_END

ROM_START( dinofmlya )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tangasofii rom1.u18", 0x00000, 0x10000, CRC(0039174c) SHA1(452d0704620600b8c376674a300b2481598f31a8) ) // hand-written label

	ROM_REGION( 0x40000, "tilemap", 0 )
	ROM_LOAD( "dino iii tetris_3 ver1.0.u16", 0x00000, 0x08000, CRC(88319fdf) SHA1(f0e97476d9664a5bdf16c27568a2c044d0818fad) )
	ROM_CONTINUE(             0x10000, 0x08000 )
	ROM_CONTINUE(             0x08000, 0x08000 )
	ROM_CONTINUE(             0x18000, 0x08000 )
	ROM_LOAD( "dino iii tetris_2 ver1.0.u17", 0x20000, 0x08000, CRC(fa355811) SHA1(d2f40e648d0c9f72c38e39021897cba23f09a56f) )
	ROM_CONTINUE(             0x30000, 0x08000 )
	ROM_CONTINUE(             0x28000, 0x08000 )
	ROM_CONTINUE(             0x38000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "u54", 0x00000, 0x20000, CRC(4e2ef62a) SHA1(77dbc2a03619ad3608a27ed70e74f3e76431498d) ) // missing label
ROM_END


/***************************************************************************
*                        Driver Init / Decryption                          *
***************************************************************************/

void subsino_state::init_victor5()
{
	uint8_t *rom = memregion( "program" )->base() + 0x4000;
	subsino_decrypt(rom, victor5_bitswaps, victor5_xors, 0x8000);

	m_flash_packet = 0;
	m_flash_packet_start = 0;
	m_flash_val = 0;

	save_item(NAME(m_flash_packet));
	save_item(NAME(m_flash_packet_start));
	save_item(NAME(m_flash_val));
}

void subsino_state::init_victor21()
{
	uint8_t *rom = memregion( "program" )->base() + 0x4000;
	subsino_decrypt(rom, victor21_bitswaps, victor21_xors, 0x8000);
}

void subsino_state::init_crsbingo()
{
	uint8_t *rom = memregion( "program" )->base() + 0x4000;
	subsino_decrypt(rom, crsbingo_bitswaps, crsbingo_xors, 0x8000);

	m_flash_packet = 0;
	m_flash_packet_start = 0;
	m_flash_val = 0;

	save_item(NAME(m_flash_packet));
	save_item(NAME(m_flash_packet_start));
	save_item(NAME(m_flash_val));
}

void subsino_state::init_sharkpy()
{
	uint8_t *rom = memregion( "program" )->base() + 0x6000;
	subsino_decrypt(rom, sharkpy_bitswaps, sharkpy_xors, 0x8000);
}

void subsino_state::init_sharkpye()
{
	uint8_t *rom = memregion( "program" )->base() + 0x6000;
	subsino_decrypt(rom, victor5_bitswaps, victor5_xors, 0x8000);
}

void subsino_state::init_smoto20()
{
	uint8_t *rom = memregion( "maincpu" )->base();
	rom[0x12e1] = 0x20; // "ERROR 951010"
}

void subsino_state::init_smoto13()
{
	uint8_t *rom = memregion( "maincpu" )->base();
	rom[0x1308] = 0x20; // "ERROR 951010"
}

void subsino_state::init_tisub()
{
	uint8_t *rom = memregion( "program" )->base() + 0x4000;
	subsino_decrypt(rom, victor5_bitswaps, victor5_xors, 0x8000);
}

void subsino_state::init_tisubb()
{
	uint8_t *rom = memregion( "program" )->base() + 0x4000;
	subsino_decrypt(rom, tisubb_bitswaps, tisubb_xors, 0x8000);
}

void subsino_state::init_newhunterb()
{
	uint8_t *rom = memregion( "program" )->base() + 0x6000;
	subsino_decrypt(rom, tisubb_bitswaps, tisubb_xors, 0x8000);
}

void subsino_state::init_newhunterc()
{
	uint8_t *rom = memregion( "program" )->base() + 0x6000;
	subsino_decrypt(rom, newhunterc_bitswaps, newhunterc_xors, 0x8000);
}

void subsino_state::init_stbsub()
{
#if 1
	uint8_t *rom = memregion( "maincpu" )->base();
	rom[0x1005] = 0x1d; //patch protection check
	rom[0x7ab] = 0x18; //patch "winning protection" check
	rom[0x957] = 0x18; //patch "losing protection" check
#endif

	for (uint8_t reel = 0; reel < 3; reel++)
	{
		m_reel_attr[reel] = std::make_unique<uint8_t[]>(0x200);

		save_pointer(NAME(m_reel_attr[reel]), 0x200, reel);
	}
}

void subsino_state::init_stisub()
{
	uint8_t *rom = memregion( "maincpu" )->base();
	rom[0xfa0] = 0x28;
	rom[0xfa1] = 0x1d; //patch protection check
	rom[0x7ed] = 0x18; //patch "winning protection" check
	rom[0x9bb] = 0x18; //patch "losing protection" check

	for (uint8_t reel = 0; reel < 3; reel++)
	{
		m_reel_attr[reel] = std::make_unique<uint8_t[]>(0x200);

		save_pointer(NAME(m_reel_attr[reel]), 0x200, reel);
	}
}

void subsino_state::init_tesorone()
{
#if 1
	uint8_t *rom = memregion( "maincpu" )->base();
	rom[0x10a4] = 0x18; //patch protection check ("ERROR 08073"):
	rom[0x10a5] = 0x11;
	rom[0x8b6] = 0x18; //patch "winning protection" check
	rom[0xa84] = 0x18; //patch "losing protection" check
#endif

	for (uint8_t reel = 0; reel < 3; reel++)
	{
		m_reel_attr[reel] = std::make_unique<uint8_t[]>(0x200);

		save_pointer(NAME(m_reel_attr[reel]), 0x200, reel);
	}
}

void subsino_state::init_tesorone230()
{
#if 1
	uint8_t *rom = memregion( "maincpu" )->base();            //check this patch!!!!
	rom[0x10a8] = 0x18; //patch protection check ("ERROR 08073"):
	rom[0x10a9] = 0x11;
	rom[0x8ba] = 0x18; //patch "winning protection" check
	rom[0xa88] = 0x18; //patch "losing protection" check
#endif

	for (uint8_t reel = 0; reel < 3; reel++)
	{
		m_reel_attr[reel] = std::make_unique<uint8_t[]>(0x200);

		save_pointer(NAME(m_reel_attr[reel]), 0x200, reel);
	}
}

void subsino_state::init_grndprix()
{
#if 1
	uint8_t *rom = memregion( "maincpu" )->base();
	rom[0x1464] = 0x18; //patch protection check
	rom[0x1465] = 0x11;
	rom[0x7ab] = 0x18; //patch "winning protection" check
	rom[0x96f] = 0x18; //patch "losing protection" check
#endif

	for (uint8_t reel = 0; reel < 3; reel++)
	{
		m_reel_attr[reel] = std::make_unique<uint8_t[]>(0x200);

		save_pointer(NAME(m_reel_attr[reel]), 0x200, reel);
	}
}

void subsino_state::init_mtrainnv()
{
	for (uint8_t reel = 0; reel < 3; reel++)
	{
		m_reel_attr[reel] = std::make_unique<uint8_t[]>(0x200);

		save_pointer(NAME(m_reel_attr[reel]), 0x200, reel);
	}
}

} // Anonymous namespace


/***************************************************************************
*                               Game Drivers                               *
***************************************************************************/

//     YEAR  NAME         PARENT   MACHINE     INPUT     CLASS          INIT              ROT   COMPANY            FULLNAME                                       FLAGS                LAYOUT
GAMEL( 1990, victor21,    0,       victor21,   victor21, subsino_state, init_victor21,    ROT0, "Subsino / Buffy", "Victor 21",                                   0,                   layout_victor21 )

GAMEL( 1991, victor5,     0,       victor5,    victor5,  subsino_state, init_victor5,     ROT0, "Subsino / Buffy", "Victor 5",                                    0,                   layout_victor5  ) // Original PCB and game from Subsino.
GAMEL( 1991, victor5a,    victor5, victor5,    victor5,  subsino_state, init_victor5,     ROT0, "Subsino",         "G.E.A.",                                      0,                   layout_victor5  ) // PCB black-box was marked 'victor 5' - in-game says G.E.A with no manufacturer info?

GAMEL( 1992, tisub,       0,       tisub,      tisub,    subsino_state, init_tisub,       ROT0, "Subsino",         "Treasure Island (Subsino, set 1)",            0,                   layout_tisub    )
GAMEL( 1992, tisuba,      tisub,   tisub,      tisub,    subsino_state, init_tisub,       ROT0, "Subsino",         "Treasure Island (Subsino, set 2)",            0,                   layout_tisub    )
GAMEL( 1992, tisubb,      tisub,   tisub,      tisubb,   subsino_state, init_tisubb,      ROT0, "American Alpha",  "Treasure Island (American Alpha, v3.0N)",     0,                   layout_tisubb   )

// tilemaps, lamps and inputs troubles.
GAMEL( 1992, newhunter,   tisub,   tisub,      tisub,    subsino_state, init_tisubb,      ROT0, "Karam",           "New HUNTer",                                  MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING, layout_tisubb   ) // 1989 on screen, but this bootleg is from 1992
GAMEL( 1993, newhunterb,  tisub,   newhunterb, tisub,    subsino_state, init_newhunterb,  ROT0, "bootleg",         "New HUNTer (bootleg, set 1)",                 MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING, layout_tisubb   )

GAMEL( 1993, newhunterc,  tisub,   newhunterb, tisub,    subsino_state, init_newhunterc,  ROT0, "bootleg",         "New HUNTer (bootleg, set 2)",                 MACHINE_NOT_WORKING, layout_tisubb   ) // 1989 on screen, but "Copyright 1993 SubSino Corp. Taipei, Taiwan." on program ROM
GAMEL( 1993, newhunterd,  tisub,   newhunterd, tisub,    subsino_state, empty_init,       ROT0, "bootleg",         "New HUNTer (bootleg, set 3)",                 MACHINE_NOT_WORKING, layout_tisubb   ) // reel GFX don't appear, inputs
GAMEL( 1998, ndongmul,    0,       newhunterb, tisub,    subsino_state, init_newhunterc,  ROT0, "Hyoja Game",      "New DongmulDongmul",                          MACHINE_NOT_WORKING, layout_tisubb   ) // hangs after a while, bad reels GFX loading / decode


GAMEL( 1991, crsbingo,    0,       crsbingo,   crsbingo, subsino_state, init_crsbingo,    ROT0, "Subsino",         "Poker Carnival",                              0,                   layout_crsbingo )

GAMEL( 1994, dinofmly,    0,       dinofmly,   sharkpy,  subsino_state, empty_init,       ROT0, "Subsino",         "Dino Family",                                 MACHINE_NOT_WORKING, layout_sharkpy  ) // stops with 'error password' message during boot
GAMEL( 1995, dinofmlya,   dinofmly,dinofmly,   sharkpy,  subsino_state, empty_init,       ROT0, "Tangasoft",       "Dino Family (Portuguese, Tangasoft license)", MACHINE_NOT_WORKING, layout_sharkpy  ) // stops with 'error password' message during boot

GAMEL( 1995, stbsub,      0,       stbsub,     stbsub,   subsino_state, init_stbsub,      ROT0, "American Alpha",  "Treasure Bonus (Subsino, v1.6)",              0,                   layout_stisub   ) // board CPU module marked 'Super Treasure Island' (alt title?)
GAMEL( 1995, stisub,      stbsub,  stbsub,     stbsub,   subsino_state, init_stisub,      ROT0, "Subsino",         "Super Treasure Island (Italy, v1.6)",         0,                   layout_stisub   )
GAMEL( 1995, tesorone,    stbsub,  stbsub,     tesorone, subsino_state, init_tesorone,    ROT0, "Subsino",         "Tesorone Dell'Isola (Italy, v2.41)",          0,                   layout_stisub   )
GAMEL( 1995, tesorone240, stbsub,  stbsub,     tesorone, subsino_state, init_tesorone,    ROT0, "Subsino",         "Tesorone Dell'Isola (Italy, v2.40)",          0,                   layout_stisub   )
GAMEL( 1995, tesorone230, stbsub,  stbsub,     tesorone, subsino_state, init_tesorone230, ROT0, "Subsino",         "Tesorone Dell'Isola (Italy, v2.30)",          0,                   layout_stisub   )
GAMEL( 1995, sevenlnd,    stbsub,  mtrainnv,   stbsub,   subsino_state, init_mtrainnv,    ROT0, "bootleg",         "Seven Land",                                  MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING, layout_stisub   )
GAMEL( 1995, luckyseven,  stbsub,  mtrainnv,   stbsub,   subsino_state, init_mtrainnv,    ROT0, "bootleg",         "Lucky Seven",                                 MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING, layout_stisub   )
GAMEL( 1995, grndprix,    stbsub,  mtrainnv,   stbsub,   subsino_state, init_grndprix,    ROT0, "bootleg",         "Grand Prix (Treasure Bonus bootleg)",         MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING, layout_stisub   ) // bad dump of a GFX ROM

GAMEL( 1996, sharkpy,     0,       sharkpy,    sharkpy,  subsino_state, init_sharkpy,     ROT0, "Subsino",         "Shark Party (Italy, v1.3)",                   0,                   layout_sharkpy  ) // missing POST messages?
GAMEL( 1996, sharkpya,    sharkpy, sharkpy,    sharkpy,  subsino_state, init_sharkpy,     ROT0, "Subsino",         "Shark Party (Italy, v1.6)",                   0,                   layout_sharkpy  ) // missing POST messages?
GAMEL( 1995, sharkpye,    sharkpy, sharkpy,    sharkpye, subsino_state, init_sharkpye,    ROT0, "American Alpha",  "Shark Party (English, Alpha license)",        0,                   layout_sharkpye ) // PCB black-box was marked 'victor 6'

GAMEL( 1995, victor6,     0,       sharkpy,    victor6,  subsino_state, init_sharkpye,    ROT0, "American Alpha",  "Victor 6 (v2.3N)",                            0,                   layout_sharkpye ) // ^^
GAMEL( 1995, victor6a,    victor6, sharkpy,    victor6a, subsino_state, init_sharkpye,    ROT0, "American Alpha",  "Victor 6 (v2.3)",                             0,                   layout_sharkpye ) // ^^
GAMEL( 1995, victor6b,    victor6, sharkpy,    victor6b, subsino_state, init_sharkpye,    ROT0, "American Alpha",  "Victor 6 (v1.2)",                             0,                   layout_sharkpye ) // ^^ Version # according to label, not displayed

GAMEL( 1996, smoto20,     0,       srider,     smoto20,  subsino_state, init_smoto20,     ROT0, "Subsino",         "Super Rider (Italy, v2.0)",                   0,                   layout_smoto    )
GAMEL( 1996, smoto16,     smoto20, srider,     smoto16,  subsino_state, init_smoto16,     ROT0, "Subsino",         "Super Moto (Italy, v1.6)",                    0,                   layout_smoto    )
GAMEL( 1996, smoto13,     smoto20, srider,     smoto16,  subsino_state, init_smoto13,     ROT0, "Subsino",         "Super Rider (v1.3)",                          0,                   layout_smoto    )

GAME(  1996, mtrainnv,    mtrain,  mtrainnv,   stbsub,   subsino_state, init_mtrainnv,    ROT0, "Subsino",         "Magic Train (Clear NVRAM ROM?)",              MACHINE_NOT_WORKING )
