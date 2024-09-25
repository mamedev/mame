// license:BSD-3-Clause
// copyright-holders:Paul Leaman
/***************************************************************************

  Legendary Wings
  Section Z
  Trojan
  Avengers

  Driver provided by Paul Leaman

TODO:
- sectionz does "false contacts" on the coin counters, causing them to
  increment twice per coin.
- accurate music tempo (audiocpu irq freq)
- accurate video timing, raw params
- verify avengers MCU comms, and redump internal ROM as well (see note
  in ROM defs under AVENGERS_MCU)


Notes:

  Avengers has a protection chip underneath the sound module.
  The protection is extensive: palette data, calculates player movement,
  even a hand in the sound.

  avengers061gre2: corrupted graphics in Avengers' ending not fixed.
  This bug is not in the Japanese set "Buraiken".
  It might just be a bug in the original: the tiles for the character
  image are just not present in the US version, replaced by more tiles
  for the title animation. The tile map ROM is the same between the two
  versions.

  trojan37b1gre: stage 2-1 boss x flip glitches not fixed.
  This could be a side effect of sprite RAM buffering. Suggest buffering
  on-screen content instead of sprite memory.

  Previous clock settings were too low. Sometimes Avengers and Trojan
  could not finish clearing VRAM before a new frame is drawn and left
  behind screen artifacts. Avengers' second CPU was forced to pre-empt
  during soundlatch operations, resulting in double or missing sound
  effects.

  Trojan (Romstar) Manual has some bonus live values as well as locations
  which do no jive with actual emulation.  One can only assume this means
  the manual is incorrect and software was adjusted later but the game could
  use some PCB comparisons of DIP selections to be certain.

***************************************************************************/

#include "emu.h"

#include "cpu/mcs51/mcs51.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/watchdog.h"
#include "sound/msm5205.h"
#include "sound/okim6295.h"
#include "sound/ymopn.h"
#include "video/bufsprite.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

namespace {

class lwings_state : public driver_device
{
public:
	lwings_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_adpcmcpu(*this, "adpcmcpu"),
		m_mcu(*this, "mcu"),
		m_mculatch(*this, "mculatch%u", 0U),
		m_msm(*this, "5205"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen"),
		m_soundlatch(*this, "soundlatch"),
		m_spriteram(*this, "spriteram"),
		m_fgvideoram(*this, "fgvideoram"),
		m_bg1videoram(*this, "bg1videoram"),
		m_bank1(*this, "bank1"),
		m_bank2(*this, "bank2"),
		m_samplebank(*this, "samplebank")
	{ }

	void lwings(machine_config &config);
	void sectionz(machine_config &config);
	void trojan(machine_config &config);
	void fball(machine_config &config);
	void avengers(machine_config &config);
	void buraikenb(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// devices
	required_device<z80_device> m_maincpu;
	required_device<z80_device> m_soundcpu;
	optional_device<z80_device> m_adpcmcpu;
	optional_device<i8751_device> m_mcu;
	optional_device_array<generic_latch_8_device, 3> m_mculatch;
	optional_device<msm5205_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_device<generic_latch_8_device> m_soundlatch;

	// memory pointers
	required_device<buffered_spriteram8_device> m_spriteram;
	required_shared_ptr<uint8_t> m_fgvideoram;
	required_shared_ptr<uint8_t> m_bg1videoram;
	required_memory_bank m_bank1;
	optional_memory_bank m_bank2;
	optional_memory_bank m_samplebank;
	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::specific m_maincpu_program;

	// video-related
	tilemap_t *m_fg_tilemap = nullptr;
	tilemap_t *m_bg1_tilemap = nullptr;
	tilemap_t *m_bg2_tilemap = nullptr;
	uint8_t   m_bg2_image = 0U;
	int       m_spr_avenger_hw = 0;
	uint8_t   m_scroll_x[2]{};
	uint8_t   m_scroll_y[2]{};

	// misc
	uint8_t   m_adpcm = 0U;
	uint8_t   m_nmi_mask = 0U;
	int       m_sprbank = 0;

	// MCU-related (avengers)
	uint8_t   m_mcu_data[2]{};
	uint8_t   m_mcu_control = 0xff;

	void avengers_adpcm_w(uint8_t data);
	uint8_t avengers_adpcm_r();
	void lwings_bankswitch_w(uint8_t data);
	uint8_t avengers_soundlatch_ack_r();
	void lwings_fgvideoram_w(offs_t offset, uint8_t data);
	void lwings_bg1videoram_w(offs_t offset, uint8_t data);
	void lwings_bg1_scrollx_w(offs_t offset, uint8_t data);
	void lwings_bg1_scrolly_w(offs_t offset, uint8_t data);
	void trojan_bg2_scrollx_w(uint8_t data);
	void trojan_bg2_image_w(uint8_t data);
	void msm5205_w(uint8_t data);
	void fball_oki_bank_w(uint8_t data);

	uint8_t mcu_p0_r();
	uint8_t mcu_p1_r();
	uint8_t mcu_p2_r();
	void mcu_p0_w(uint8_t data);
	void mcu_p2_w(uint8_t data);
	void mcu_control_w(uint8_t data);

	TILEMAP_MAPPER_MEMBER(get_bg2_memory_offset);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(lwings_get_bg1_tile_info);
	TILE_GET_INFO_MEMBER(trojan_get_bg1_tile_info);
	TILE_GET_INFO_MEMBER(get_bg2_tile_info);
	DECLARE_VIDEO_START(trojan);
	DECLARE_VIDEO_START(avengers);
	uint32_t screen_update_lwings(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_trojan(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void lwings_interrupt(int state);
	void avengers_interrupt(int state);
	bool is_sprite_on(uint8_t const *buffered_spriteram, int offs);
	void lwings_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void trojan_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );

	void avengers_adpcm_io_map(address_map &map) ATTR_COLD;
	void avengers_map(address_map &map) ATTR_COLD;
	void buraikenb_map(address_map &map) ATTR_COLD;
	void fball_map(address_map &map) ATTR_COLD;
	void fball_oki_map(address_map &map) ATTR_COLD;
	void fball_sound_map(address_map &map) ATTR_COLD;
	void lwings_map(address_map &map) ATTR_COLD;
	void lwings_sound_map(address_map &map) ATTR_COLD;
	void trojan_adpcm_io_map(address_map &map) ATTR_COLD;
	void trojan_adpcm_map(address_map &map) ATTR_COLD;
	void trojan_map(address_map &map) ATTR_COLD;
};

/* Avengers runs on hardware almost identical to Trojan, but with a protection
 * device and some small changes to the memory map and video hardware.
 *
 * Background colors are fetched 64 bytes at a time and copied to palette RAM.
 *
 * Another function takes as input 2 pairs of (x,y) coordinates, and returns
 * a code reflecting the direction (8 angles) from one point to the other.
 */

void lwings_state::avengers_adpcm_w(uint8_t data)
{
	m_adpcm = data;
}

uint8_t lwings_state::avengers_adpcm_r()
{
	return m_adpcm;
}

void lwings_state::lwings_bankswitch_w(uint8_t data)
{
	// bit 0 is flip screen
	flip_screen_set(BIT(~data, 0));

	// bits 1 and 2 select ROM bank
	m_bank1->set_entry(data >> 1 & 3);

	// bit 3 enables NMI
	m_nmi_mask = BIT(data, 3);

	// bit 4: sprite bank (fireball only)
	m_sprbank = BIT(data, 4);
	
	// bit 5 resets the sound CPU
	m_soundcpu->set_input_line(INPUT_LINE_RESET, BIT(data, 5) ? ASSERT_LINE : CLEAR_LINE);

	// bits 6 and 7 are coin counters
	machine().bookkeeping().coin_counter_w(1, BIT(data, 6));
	machine().bookkeeping().coin_counter_w(0, BIT(data, 7));
}

void lwings_state::lwings_interrupt(int state)
{
	if (state && m_nmi_mask)
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xd7); /* Z80 - RST 10h */
}

void lwings_state::avengers_interrupt(int state)
{
	if (state && m_nmi_mask)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}


uint8_t lwings_state::mcu_p0_r()
{
	if (!BIT(m_mcu_control, 7))
		return m_mculatch[0]->read();
	else
		return 0xff;
}

uint8_t lwings_state::mcu_p1_r()
{
	// this is used to decide if we're sending angle params or a sound write? compares against 0xf0, vpos like 1943?
	return m_screen->vpos();
}

uint8_t lwings_state::mcu_p2_r()
{
	if (!BIT(m_mcu_control, 7))
		return m_mculatch[1]->read();
	else
		return 0xff;
}

void lwings_state::mcu_p0_w(uint8_t data)
{
	m_mcu_data[0] = data;
}

void lwings_state::mcu_p2_w(uint8_t data)
{
	m_mcu_data[1] = data;
}

void lwings_state::mcu_control_w(uint8_t data)
{
	if (!BIT(m_mcu_control, 6) && BIT(data, 6))
	{
		//logerror("%s: MCU writes %02X back to main CPU\n", machine().time().to_string(), m_mcu_data[0]);
		m_mculatch[2]->write(m_mcu_data[0]);
		m_soundlatch->write(m_mcu_data[1]);
		machine().scheduler().perfect_quantum(attotime::from_usec(60));
	}

	if (BIT(m_mcu_control, 7) != BIT(data, 7))
		m_mculatch[0]->acknowledge_w();

	m_mcu_control = data;
}

uint8_t lwings_state::avengers_soundlatch_ack_r()
{
	uint8_t data = m_soundlatch->read() | (m_soundlatch->pending_r() ? 0x80 : 0);
	if (!machine().side_effects_disabled())
		m_soundlatch->acknowledge_w();

	return data;
}

void lwings_state::msm5205_w(uint8_t data)
{
	m_msm->reset_w(BIT(data, 7));
	m_msm->data_w(data);
	m_msm->vclk_w(1);
	m_msm->vclk_w(0);
}

void lwings_state::buraikenb_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr("bank1");
	map(0xc000, 0xddff).ram();
	map(0xde00, 0xdf7f).ram().share("spriteram");
	map(0xdf80, 0xdfff).ram();
	map(0xe000, 0xe7ff).ram().w(FUNC(lwings_state::lwings_fgvideoram_w)).share("fgvideoram");
	map(0xe800, 0xefff).ram().w(FUNC(lwings_state::lwings_bg1videoram_w)).share("bg1videoram");
	map(0xf000, 0xf3ff).ram().w(m_palette, FUNC(palette_device::write8_ext)).share("palette_ext");
	map(0xf400, 0xf7ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0xf800, 0xf801).w(FUNC(lwings_state::lwings_bg1_scrollx_w));
	map(0xf802, 0xf803).w(FUNC(lwings_state::lwings_bg1_scrolly_w));
	map(0xf804, 0xf804).w(FUNC(lwings_state::trojan_bg2_scrollx_w));
	map(0xf805, 0xf805).w(FUNC(lwings_state::trojan_bg2_image_w));

	map(0xf808, 0xf808).portr("SERVICE").nopw(); // ?
	map(0xf809, 0xf809).portr("P1");
	map(0xf80a, 0xf80a).portr("P2");
	map(0xf80b, 0xf80b).portr("DSWB");
	map(0xf80c, 0xf80c).portr("DSWA").w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0xf80d, 0xf80d).w(FUNC(lwings_state::avengers_adpcm_w));
	map(0xf80e, 0xf80e).w(FUNC(lwings_state::lwings_bankswitch_w));
}

void lwings_state::avengers_map(address_map &map)
{
	buraikenb_map(map);

	map(0xf809, 0xf809).w(m_mculatch[0], FUNC(generic_latch_8_device::write));
	map(0xf80c, 0xf80c).w(m_mculatch[1], FUNC(generic_latch_8_device::write));
	map(0xf80d, 0xf80d).r(m_mculatch[2], FUNC(generic_latch_8_device::read));
}

void lwings_state::lwings_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr("bank1");
	map(0xc000, 0xddff).ram();
	map(0xde00, 0xdfff).ram().share("spriteram");
	map(0xe000, 0xe7ff).ram().w(FUNC(lwings_state::lwings_fgvideoram_w)).share("fgvideoram");
	map(0xe800, 0xefff).ram().w(FUNC(lwings_state::lwings_bg1videoram_w)).share("bg1videoram");
	map(0xf000, 0xf3ff).ram().w(m_palette, FUNC(palette_device::write8_ext)).share("palette_ext");
	map(0xf400, 0xf7ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");

	map(0xf808, 0xf808).portr("SERVICE");
	map(0xf809, 0xf809).portr("P1");
	map(0xf808, 0xf809).w(FUNC(lwings_state::lwings_bg1_scrollx_w));
	map(0xf80a, 0xf80a).portr("P2");
	map(0xf80b, 0xf80b).portr("DSWA");
	map(0xf80a, 0xf80b).w(FUNC(lwings_state::lwings_bg1_scrolly_w));
	map(0xf80c, 0xf80c).portr("DSWB").w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0xf80d, 0xf80d).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0xf80e, 0xf80e).w(FUNC(lwings_state::lwings_bankswitch_w));
}

void lwings_state::trojan_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr("bank1");
	map(0xc000, 0xddff).ram();
	map(0xde00, 0xdf7f).ram().share("spriteram");
	map(0xdf80, 0xdfff).ram();
	map(0xe000, 0xe7ff).ram().w(FUNC(lwings_state::lwings_fgvideoram_w)).share("fgvideoram");
	map(0xe800, 0xefff).ram().w(FUNC(lwings_state::lwings_bg1videoram_w)).share("bg1videoram");
	map(0xf000, 0xf3ff).ram().w(m_palette, FUNC(palette_device::write8_ext)).share("palette_ext");
	map(0xf400, 0xf7ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");

	map(0xf800, 0xf801).w(FUNC(lwings_state::lwings_bg1_scrollx_w));
	map(0xf802, 0xf803).w(FUNC(lwings_state::lwings_bg1_scrolly_w));
	map(0xf804, 0xf804).w(FUNC(lwings_state::trojan_bg2_scrollx_w));
	map(0xf805, 0xf805).w(FUNC(lwings_state::trojan_bg2_image_w));
	map(0xf808, 0xf808).portr("SERVICE").nopw(); // watchdog
	map(0xf809, 0xf809).portr("P1");
	map(0xf80a, 0xf80a).portr("P2");
	map(0xf80b, 0xf80b).portr("DSWA");
	map(0xf80c, 0xf80c).portr("DSWB").w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0xf80d, 0xf80d).w("soundlatch2", FUNC(generic_latch_8_device::write));
	map(0xf80e, 0xf80e).w(FUNC(lwings_state::lwings_bankswitch_w));
}

void lwings_state::lwings_sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xc000, 0xc7ff).ram();
	map(0xc800, 0xc800).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0xe000, 0xe001).w("2203a", FUNC(ym2203_device::write));
	map(0xe002, 0xe003).w("2203b", FUNC(ym2203_device::write));
	map(0xe006, 0xe006).r(FUNC(lwings_state::avengers_soundlatch_ack_r)).nopw();
}


void lwings_state::fball_map(address_map &map)
{
	map(0x0000, 0x7fff).bankr("bank2");
	map(0x8000, 0xbfff).bankr("bank1");
	map(0xc000, 0xddff).ram();
	map(0xde00, 0xdfff).ram().share("spriteram");
	map(0xe000, 0xe7ff).ram().w(FUNC(lwings_state::lwings_fgvideoram_w)).share("fgvideoram");
	map(0xe800, 0xefff).ram().w(FUNC(lwings_state::lwings_bg1videoram_w)).share("bg1videoram");
	map(0xf000, 0xf3ff).ram().w(m_palette, FUNC(palette_device::write8_ext)).share("palette_ext");
	map(0xf400, 0xf7ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");

	map(0xf808, 0xf808).portr("SERVICE");
	map(0xf809, 0xf809).portr("P1");
	map(0xf808, 0xf809).w(FUNC(lwings_state::lwings_bg1_scrollx_w));
	map(0xf80a, 0xf80a).portr("P2");
	map(0xf80b, 0xf80b).portr("DSWA");
	map(0xf80a, 0xf80b).w(FUNC(lwings_state::lwings_bg1_scrolly_w));
	map(0xf80c, 0xf80c).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0xf80d, 0xf80d).portr("P3").w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0xf80e, 0xf80e).portr("P4");

	map(0xf80e, 0xf80e).w(FUNC(lwings_state::lwings_bankswitch_w));
}


void lwings_state::fball_oki_bank_w(uint8_t data)
{
	m_samplebank->set_entry((data >> 1) & 0x7);
}

void lwings_state::fball_oki_map(address_map &map)
{
	map(0x00000, 0x1ffff).rom();
	map(0x20000, 0x3ffff).bankr("samplebank");
}


void lwings_state::fball_sound_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
	map(0x8000, 0x8000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0xa000, 0xa000).w(FUNC(lwings_state::fball_oki_bank_w));
	map(0xc000, 0xc7ff).ram();
	map(0xe000, 0xe000).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}

/* Yes, _no_ ram */
void lwings_state::trojan_adpcm_map(address_map &map)
{
	map(0x0000, 0xffff).rom().nopw();
}

void lwings_state::avengers_adpcm_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).r(FUNC(lwings_state::avengers_adpcm_r));
	map(0x01, 0x01).w(FUNC(lwings_state::msm5205_w));
}

void lwings_state::trojan_adpcm_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).r("soundlatch2", FUNC(generic_latch_8_device::read));
	map(0x01, 0x01).w(FUNC(lwings_state::msm5205_w));
}


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILEMAP_MAPPER_MEMBER(lwings_state::get_bg2_memory_offset)
{
	return (row * 0x800) | (col * 2);
}

TILE_GET_INFO_MEMBER(lwings_state::get_fg_tile_info)
{
	int code = m_fgvideoram[tile_index];
	int color = m_fgvideoram[tile_index + 0x400];
	tileinfo.set(0,
			code + ((color & 0xc0) << 2),
			color & 0x0f,
			TILE_FLIPYX((color & 0x30) >> 4));
}

TILE_GET_INFO_MEMBER(lwings_state::lwings_get_bg1_tile_info)
{
	int code = m_bg1videoram[tile_index];
	int color = m_bg1videoram[tile_index + 0x400];
	tileinfo.set(1,
			code + ((color & 0xe0) << 3),
			color & 0x07,
			TILE_FLIPYX((color & 0x18) >> 3));
}

TILE_GET_INFO_MEMBER(lwings_state::trojan_get_bg1_tile_info)
{
	int code = m_bg1videoram[tile_index];
	int color = m_bg1videoram[tile_index + 0x400];
	code += (color & 0xe0)<<3;
	tileinfo.set(1,
			code,
			(color & 7),
			((color & 0x10) ? TILE_FLIPX : 0));

	tileinfo.group = (color & 0x08) >> 3;
}

TILE_GET_INFO_MEMBER(lwings_state::get_bg2_tile_info)
{
	int code, color;
	uint8_t *rom = memregion("gfx5")->base();
	int mask = memregion("gfx5")->bytes() - 1;

	tile_index = (tile_index + m_bg2_image * 0x20) & mask;
	code = rom[tile_index];
	color = rom[tile_index + 1];
	tileinfo.set(3,
			code + ((color & 0x80) << 1),
			color & 0x07,
			TILE_FLIPYX((color & 0x30) >> 4));
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void lwings_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(lwings_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg1_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(lwings_state::lwings_get_bg1_tile_info)), TILEMAP_SCAN_COLS, 16, 16, 32, 32);

	m_fg_tilemap->set_transparent_pen(3);
}

VIDEO_START_MEMBER(lwings_state,trojan)
{
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(lwings_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg1_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(lwings_state::trojan_get_bg1_tile_info)),TILEMAP_SCAN_COLS, 16, 16, 32, 32);
	m_bg2_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(lwings_state::get_bg2_tile_info)), tilemap_mapper_delegate(*this, FUNC(lwings_state::get_bg2_memory_offset)), 16, 16, 32, 16);

	m_fg_tilemap->set_transparent_pen(3);
	m_bg1_tilemap->set_transmask(0, 0xffff, 0x0001); // split type 0 is totally transparent in front half
	m_bg1_tilemap->set_transmask(1, 0xf07f, 0x0f81); // split type 1 has pens 7-11 opaque in front half

	m_spr_avenger_hw = 0;
}

VIDEO_START_MEMBER(lwings_state,avengers)
{
	VIDEO_START_CALL_MEMBER(trojan);
	m_spr_avenger_hw = 1;
}


/***************************************************************************

  Memory handlers

***************************************************************************/

void lwings_state::lwings_fgvideoram_w(offs_t offset, uint8_t data)
{
	m_fgvideoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset & 0x3ff);
}

void lwings_state::lwings_bg1videoram_w(offs_t offset, uint8_t data)
{
	m_bg1videoram[offset] = data;
	m_bg1_tilemap->mark_tile_dirty(offset & 0x3ff);
}


void lwings_state::lwings_bg1_scrollx_w(offs_t offset, uint8_t data)
{
	m_scroll_x[offset] = data;
	m_bg1_tilemap->set_scrollx(0, m_scroll_x[0] | (m_scroll_x[1] << 8));
}

void lwings_state::lwings_bg1_scrolly_w(offs_t offset, uint8_t data)
{
	m_scroll_y[offset] = data;
	m_bg1_tilemap->set_scrolly(0, m_scroll_y[0] | (m_scroll_y[1] << 8));
}

void lwings_state::trojan_bg2_scrollx_w(uint8_t data)
{
	m_bg2_tilemap->set_scrollx(0, data);
}

void lwings_state::trojan_bg2_image_w(uint8_t data)
{
	if (m_bg2_image != data)
	{
		m_bg2_image = data;
		m_bg2_tilemap->mark_all_dirty();
	}
}


/***************************************************************************

  Display refresh

***************************************************************************/

inline bool lwings_state::is_sprite_on(uint8_t const *buffered_spriteram, int offs)
{
	int const sx = buffered_spriteram[offs + 3] - 0x100 * (buffered_spriteram[offs + 1] & 0x01);
	int const sy = buffered_spriteram[offs + 2];

	return sx || sy;
}

void lwings_state::lwings_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	uint8_t const *const buffered_spriteram = m_spriteram->buffer();

	for (int offs = m_spriteram->bytes() - 4; offs >= 0; offs -= 4)
	{
		if (is_sprite_on(buffered_spriteram, offs))
		{
			int sx = buffered_spriteram[offs + 3] - 0x100 * (buffered_spriteram[offs + 1] & 0x01);
			int sy = buffered_spriteram[offs + 2];
			if (sy > 0xf8)
				sy -= 0x100;
			int code = buffered_spriteram[offs] | (buffered_spriteram[offs + 1] & 0xc0) << 2;
			int color = (buffered_spriteram[offs + 1] & 0x38) >> 3;
			int flipx = buffered_spriteram[offs + 1] & 0x02;
			int flipy = buffered_spriteram[offs + 1] & 0x04;

			if (flip_screen())
			{
				sx = 240 - sx;
				sy = 240 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			m_gfxdecode->gfx(2)->transpen(
					bitmap, cliprect,
					code + (m_sprbank * 0x400), color,
					flipx, flipy, sx, sy,
					15);
		}
	}
}

void lwings_state::trojan_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	uint8_t const *const buffered_spriteram = m_spriteram->buffer();

	for (int offs = m_spriteram->bytes() - 4; offs >= 0; offs -= 4)
	{
		if (is_sprite_on(buffered_spriteram, offs))
		{
			int sx = buffered_spriteram[offs + 3] - 0x100 * (buffered_spriteram[offs + 1] & 0x01);
			int sy = buffered_spriteram[offs + 2];
			if (sy > 0xf8)
				sy -= 0x100;
			int code = buffered_spriteram[offs] |
					((buffered_spriteram[offs + 1] & 0x20) << 4) |
					((buffered_spriteram[offs + 1] & 0x40) << 2) |
					((buffered_spriteram[offs + 1] & 0x80) << 3);
			int color = (buffered_spriteram[offs + 1] & 0x0e) >> 1;

			int flipx, flipy;
			if (m_spr_avenger_hw)
			{
				flipx = 0;                                      /* Avengers */
				flipy = ~buffered_spriteram[offs + 1] & 0x10;
			}
			else
			{
				flipx = buffered_spriteram[offs + 1] & 0x10;    /* Trojan */
				flipy = 1;
			}

			if (flip_screen())
			{
				sx = 240 - sx;
				sy = 240 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			m_gfxdecode->gfx(2)->transpen(
					bitmap, cliprect,
					code, color,
					flipx, flipy, sx, sy,
					15);
		}
	}
}

uint32_t lwings_state::screen_update_lwings(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg1_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	lwings_draw_sprites(bitmap, cliprect);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

uint32_t lwings_state::screen_update_trojan(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg2_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_bg1_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1, 0);
	trojan_draw_sprites(bitmap, cliprect);
	m_bg1_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


/*************************************
 *
 *  Generic port definitions
 *
 *************************************/

static INPUT_PORTS_START( lwings_generic )
	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
INPUT_PORTS_END


/*************************************
 *
 *  Game-specific port definitions
 *
 *************************************/

static INPUT_PORTS_START( sectionz )
	PORT_INCLUDE( lwings_generic )

	PORT_MODIFY("SERVICE")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_MODIFY("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL

	PORT_START("DSWA")
	PORT_SERVICE_DIPLOC( 0x01, IP_ACTIVE_LOW, "SWA:8" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SWA:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SWA:6,5")
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SWA:4,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SWA:2,1")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_3C ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SWB:7,6")
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x06, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SWB:5,4,3")
	PORT_DIPSETTING(    0x38, "20000 50000" )
	PORT_DIPSETTING(    0x18, "20000 60000" )
	PORT_DIPSETTING(    0x28, "20000 70000" )
	PORT_DIPSETTING(    0x08, "30000 60000" )
	PORT_DIPSETTING(    0x30, "30000 70000" )
	PORT_DIPSETTING(    0x10, "30000 80000" )
	PORT_DIPSETTING(    0x20, "40000 100000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SWB:2,1")
	PORT_DIPSETTING(    0x00, "Upright One Player" )
	PORT_DIPSETTING(    0x40, "Upright Two Players" )
/*      PORT_DIPSETTING(    0x80, "???" )       probably unused */
	PORT_DIPSETTING(    0xc0, DEF_STR( Cocktail ) )
INPUT_PORTS_END

static INPUT_PORTS_START( lwings )
	PORT_INCLUDE( lwings_generic )

	PORT_START("DSWA")
	PORT_DIPUNUSED_DIPLOC( 0x0001, 0x0001, "SWA:8" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SWA:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SWA:6,5")
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SWA:4,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SWA:2,1")
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_4C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_3C ) )

	PORT_START("DSWB")
	PORT_DIPUNUSED_DIPLOC( 0x0001, 0x0001, "SWB:8" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SWB:7,6")
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x06, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SWB:4")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SWB:3,2,1")
	PORT_DIPSETTING(    0xe0, "20000 and every 50000" )
	PORT_DIPSETTING(    0x60, "20000 and every 60000" )
	PORT_DIPSETTING(    0xa0, "20000 and every 70000" )
	PORT_DIPSETTING(    0x20, "30000 and every 60000" )
	PORT_DIPSETTING(    0xc0, "30000 and every 70000" )
	PORT_DIPSETTING(    0x40, "30000 and every 80000" )
	PORT_DIPSETTING(    0x80, "40000 and every 100000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
INPUT_PORTS_END

static INPUT_PORTS_START( lwingsb )
	PORT_INCLUDE( lwings )

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SWA:6,5")
	PORT_DIPSETTING(    0x0c, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x00, "5" )
INPUT_PORTS_END


static INPUT_PORTS_START( fball )
	PORT_INCLUDE( lwings_generic )

	PORT_MODIFY("SERVICE")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START4 )

	PORT_START("DSWA") // only one set of dipswitches
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SWA:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) ) // smaller starting explosion
	PORT_DIPNAME( 0x06, 0x04, DEF_STR( Lives ) ) PORT_DIPLOCATION("SWA:2,3")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x06, "4" )
	PORT_DIPNAME( 0x18, 0x08, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SWA:4,5")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SWA:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) ) PORT_DIPLOCATION("SWA:7") // 'X' in test mode, presumably unused
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_HIGH, "SWA:8" )

	PORT_START("P3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */

	PORT_START("P4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
INPUT_PORTS_END

/* Trojan with level selection - starting level dip switches not used */
static INPUT_PORTS_START( trojanls )
	PORT_INCLUDE( lwings_generic )

	PORT_MODIFY("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL

	/* DSW tags inverted to use lwings map */
	PORT_START("DSWA")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SWB:1,2")
	PORT_DIPSETTING(    0x00, "Upright 1 Player" )
	PORT_DIPSETTING(    0x02, "Upright 2 Players" )
	PORT_DIPSETTING(    0x03, DEF_STR( Cocktail ) )
/* 0x01 same as 0x02 or 0x03 */
	PORT_DIPNAME( 0x1c, 0x1c, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SWB:3,4,5")
	PORT_DIPSETTING(    0x10, "20000 60000" )
	PORT_DIPSETTING(    0x0c, "20000 70000" )
	PORT_DIPSETTING(    0x08, "20000 80000" )
	PORT_DIPSETTING(    0x1c, "30000 60000" )
	PORT_DIPSETTING(    0x18, "30000 70000" )
	PORT_DIPSETTING(    0x14, "30000 80000" )
	PORT_DIPSETTING(    0x04, "40000 80000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SWB:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SWB:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SWB:8" )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SWA:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SWA:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) ) PORT_DIPLOCATION("SWA:5,6")
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SWA:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SWA:8")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )
INPUT_PORTS_END

static INPUT_PORTS_START( trojan )
	PORT_INCLUDE( trojanls )

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0xe0, 0xe0, "Starting Level" ) PORT_DIPLOCATION("SWB:6,7,8")
	PORT_DIPSETTING(    0xe0, "1" )
	PORT_DIPSETTING(    0xc0, "2" )
	PORT_DIPSETTING(    0xa0, "3" )
	PORT_DIPSETTING(    0x80, "4" )
	PORT_DIPSETTING(    0x60, "5" )
	PORT_DIPSETTING(    0x40, "6" )
/* 0x00 and 0x20 start at level 6 */
INPUT_PORTS_END

static INPUT_PORTS_START( avengers )
	PORT_INCLUDE( lwings_generic )

	PORT_START("DSWA")
	PORT_SERVICE_DIPLOC( 0x01, IP_ACTIVE_LOW, "SWA:8")
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SWA:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x1c, 0x1c, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SWA:6,5,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SWA:3,2,1")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_6C ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SWB:6,5")
	PORT_DIPSETTING(    0x04, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SWB:4,3")
	PORT_DIPSETTING(    0x30, "20k 60k" )
	PORT_DIPSETTING(    0x10, "20k 70k" )
	PORT_DIPSETTING(    0x20, "20k 80k" )
	PORT_DIPSETTING(    0x00, "30k 80k" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) ) PORT_DIPLOCATION("SWB:2,1")
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x80, "5" )
	PORT_DIPSETTING(    0x00, "6" )
INPUT_PORTS_END


/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+4, RGN_FRAC(1,2)+0, 4, 0 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3,
			32*8+0, 32*8+1, 32*8+2, 32*8+3, 33*8+0, 33*8+1, 33*8+2, 33*8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	64*8
};

static const gfx_layout bg1_tilelayout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8
};

static const gfx_layout bg2_tilelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4, 0, 4 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3,
			32*8+0, 32*8+1, 32*8+2, 32*8+3, 33*8+0, 33*8+1, 33*8+2, 33*8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	64*8
};


static GFXDECODE_START( gfx_lwings )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     512, 16 ) /* colors 512-575 */
	GFXDECODE_ENTRY( "gfx2", 0, bg1_tilelayout,   0,  8 ) /* colors   0-127 */
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout,   384,  8 ) /* colors 384-511 */
GFXDECODE_END

static GFXDECODE_START( gfx_trojan )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     768, 16 ) /* colors 768-831 */
	GFXDECODE_ENTRY( "gfx2", 0, bg1_tilelayout, 256,  8 ) /* colors 256-383 */
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout,   640,  8 ) /* colors 640-767 */
	GFXDECODE_ENTRY( "gfx4", 0, bg2_tilelayout,   0,  8 ) /* colors   0-127 */
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void lwings_state::machine_start()
{
	uint8_t *ROM = memregion("maincpu")->base();

	m_bank1->configure_entries(0, 4, &ROM[0x10000], 0x4000);

	m_maincpu->space(AS_PROGRAM).specific(m_maincpu_program);

	save_item(NAME(m_bg2_image));
	save_item(NAME(m_scroll_x));
	save_item(NAME(m_scroll_y));
	save_item(NAME(m_adpcm));
	save_item(NAME(m_nmi_mask));
	save_item(NAME(m_sprbank));

	/*
	Fireball has 2 copies of the 'fixed' code in the main program ROM, with only slight changes.
	It might be possible the hardware can bank that whole area or alternatively only see one version of the program
	The only difference is 2 pieces of code have been swapped around.  It is unknown when this code is called.

	3822:   CD  73
	3823:   00  23
	3824:   3E  72
	3879:   73  CD
	387A:   23  00
	387B:   72  3E

	bank 0
	3822: 73            ld   (hl),e
	3823: 23            inc  hl
	3824: 72            ld   (hl),d
	...
	3879: CD 00 3E      call $3E00

	bank 1
	3822: CD 00 3E      call $3E00
	..
	3879: 73            ld   (hl),e
	387A: 23            inc  hl
	387B: 72            ld   (hl),d

	*/

	if (m_bank2.found())
	{
		m_bank2->configure_entries(0, 2, &ROM[0x0000], 0x8000);
		m_bank2->set_entry(0);
	}

	if (m_samplebank.found())
	{
		uint8_t *OKIROM = memregion("oki")->base();
		m_samplebank->configure_entries(0, 8, OKIROM, 0x20000);
	}

	if (m_mcu.found())
	{
		save_item(NAME(m_mcu_data));
		save_item(NAME(m_mcu_control));
	}
}

void lwings_state::machine_reset()
{
	m_bg2_image = 0;
	m_scroll_x[0] = 0;
	m_scroll_x[1] = 0;
	m_scroll_y[0] = 0;
	m_scroll_y[1] = 0;
	m_adpcm = 0;
}

void lwings_state::lwings(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 12_MHz_XTAL/2); // verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &lwings_state::lwings_map);

	Z80(config, m_soundcpu, 12_MHz_XTAL/4); // verified on PCB
	m_soundcpu->set_addrmap(AS_PROGRAM, &lwings_state::lwings_sound_map);
	m_soundcpu->set_periodic_int(FUNC(lwings_state::irq0_line_hold), attotime::from_hz(222));
	// above frequency is an approximation from PCB music recording - where is the frequency actually derived from?

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	BUFFERED_SPRITERAM8(config, m_spriteram);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 1*8, 31*8-1);
	m_screen->set_screen_update(FUNC(lwings_state::screen_update_lwings));
	m_screen->screen_vblank().set(m_spriteram, FUNC(buffered_spriteram8_device::vblank_copy_rising));
	m_screen->screen_vblank().append(FUNC(lwings_state::lwings_interrupt));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_lwings);
	PALETTE(config, m_palette).set_format(palette_device::RGBx_444, 1024);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	ym2203_device &ym2203a(YM2203(config, "2203a", 12_MHz_XTAL/8)); // verified on PCB
	ym2203a.add_route(0, "mono", 0.20);
	ym2203a.add_route(1, "mono", 0.20);
	ym2203a.add_route(2, "mono", 0.20);
	ym2203a.add_route(3, "mono", 0.10);

	ym2203_device &ym2203b(YM2203(config, "2203b", 12_MHz_XTAL/8)); // verified on PCB
	ym2203b.add_route(0, "mono", 0.20);
	ym2203b.add_route(1, "mono", 0.20);
	ym2203b.add_route(2, "mono", 0.20);
	ym2203b.add_route(3, "mono", 0.10);
}

void lwings_state::sectionz(machine_config &config)
{
	lwings(config);

	m_maincpu->set_clock(12_MHz_XTAL/4); // XTAL and clock verified on an original PCB and on a bootleg with ROMs matching those of sectionza

	m_screen->set_refresh_hz(55.37); // verified on an original PCB
}

void lwings_state::fball(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 12_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &lwings_state::fball_map);

	Z80(config, m_soundcpu, 12_MHz_XTAL/4); // ?
	m_soundcpu->set_addrmap(AS_PROGRAM, &lwings_state::fball_sound_map);
	//m_soundcpu->set_periodic_int(FUNC(lwings_state::irq0_line_hold), attotime::from_hz(222));

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	BUFFERED_SPRITERAM8(config, m_spriteram);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 1*8, 31*8-1); // the 16-pixel black border on left edge is correct, test mode actually uses that area
	m_screen->set_screen_update(FUNC(lwings_state::screen_update_lwings));
	m_screen->screen_vblank().set(m_spriteram, FUNC(buffered_spriteram8_device::vblank_copy_rising));
	m_screen->screen_vblank().append(FUNC(lwings_state::avengers_interrupt));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_lwings);
	PALETTE(config, m_palette).set_format(palette_device::RGBx_444, 1024);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->set_separate_acknowledge(true);

	okim6295_device &oki(OKIM6295(config, "oki", 12_MHz_XTAL/12, okim6295_device::PIN7_HIGH)); // clock frequency & pin 7 not verified
	oki.add_route(ALL_OUTPUTS, "mono", 1.0);
	oki.set_addrmap(0, &lwings_state::fball_oki_map);
}

void lwings_state::trojan(machine_config &config)
{
	lwings(config);

	// basic machine hardware
	m_maincpu->set_clock(12_MHz_XTAL/4); // verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &lwings_state::trojan_map);

	m_soundcpu->set_clock(12_MHz_XTAL/4); // verified on PCB

	Z80(config, m_adpcmcpu, 12_MHz_XTAL/4); // verified on PCB
	m_adpcmcpu->set_addrmap(AS_PROGRAM, &lwings_state::trojan_adpcm_map);
	m_adpcmcpu->set_addrmap(AS_IO, &lwings_state::trojan_adpcm_io_map);
	m_adpcmcpu->set_periodic_int(FUNC(lwings_state::irq0_line_hold), attotime::from_hz(4000));

	// video hardware
	m_gfxdecode->set_info(gfx_trojan);

	MCFG_VIDEO_START_OVERRIDE(lwings_state,trojan)
	m_screen->set_screen_update(FUNC(lwings_state::screen_update_trojan));

	// sound hardware
	GENERIC_LATCH_8(config, "soundlatch2");

	MSM5205(config, m_msm, 384_kHz_XTAL); // verified on PCB
	m_msm->set_prescaler_selector(msm5205_device::SEX_4B); // slave mode
	m_msm->add_route(ALL_OUTPUTS, "mono", 0.50);
}

void lwings_state::avengers(machine_config &config)
{
	trojan(config);

	// basic machine hardware
	m_maincpu->set_clock(12_MHz_XTAL/2);
	m_maincpu->z80_set_m1_cycles(4+2); // 2 WAIT states per M1? (needed to keep in sync with MCU)
	m_maincpu->set_addrmap(AS_PROGRAM, &lwings_state::avengers_map);

	I8751(config, m_mcu, 12_MHz_XTAL/2);
	m_mcu->port_in_cb<0>().set(FUNC(lwings_state::mcu_p0_r));
	m_mcu->port_out_cb<0>().set(FUNC(lwings_state::mcu_p0_w));
	m_mcu->port_in_cb<1>().set(FUNC(lwings_state::mcu_p1_r));
	m_mcu->port_in_cb<2>().set(FUNC(lwings_state::mcu_p2_r));
	m_mcu->port_out_cb<2>().set(FUNC(lwings_state::mcu_p2_w));
	m_mcu->port_out_cb<3>().set(FUNC(lwings_state::mcu_control_w));

	config.set_maximum_quantum(attotime::from_hz(10000));

	GENERIC_LATCH_8(config, m_mculatch[0]);
	m_mculatch[0]->data_pending_callback().set_inputline(m_mcu, MCS51_INT0_LINE);
	m_mculatch[0]->data_pending_callback().append([this] (int state) { if (state) machine().scheduler().perfect_quantum(attotime::from_usec(192)); });
	m_mculatch[0]->set_separate_acknowledge(true);

	GENERIC_LATCH_8(config, m_mculatch[1]);
	GENERIC_LATCH_8(config, m_mculatch[2]);

	m_screen->screen_vblank().set(m_spriteram, FUNC(buffered_spriteram8_device::vblank_copy_rising));
	m_screen->screen_vblank().append(FUNC(lwings_state::avengers_interrupt)); // RST 38h triggered by software

	m_adpcmcpu->set_addrmap(AS_IO, &lwings_state::avengers_adpcm_io_map);

	// video hardware
	MCFG_VIDEO_START_OVERRIDE(lwings_state,avengers)
}

void lwings_state::buraikenb(machine_config &config)
{
	avengers(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &lwings_state::buraikenb_map);
	config.device_remove("mcu");
	config.device_remove("mculatch0");
	config.device_remove("mculatch1");
	config.device_remove("mculatch2");
}


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

/*

For the Legendary Wings sets, the US sets were labeled as LW and had a red stripe across the labels.
  ROMs LW 01 through LW 05 would "usually" have a red U stamped on them as well.  Pictures of PCBs
  verify known revisions are A & C

*/
ROM_START( lwings )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code + 3*16k for the banked ROMs images */
	ROM_LOAD( "lwu_01c.6c", 0x00000, 0x8000, CRC(b55a7f60) SHA1(e28cc540892a9ad050693900356744f8f5d05237) )
	ROM_LOAD( "lwu_02c.7c", 0x10000, 0x8000, CRC(a5efbb1b) SHA1(9126efa78fd39a50032826d0b4bd3acffceba508) )
	ROM_LOAD( "lw_03.9c",   0x18000, 0x8000, CRC(ec5cc201) SHA1(1043c6a9678c18fef920be91b0796c93b83e0f73) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "lw_04.11e", 0x0000, 0x8000, CRC(a20337a2) SHA1(649e13a69ad9154657894fa7bf7c6e49b029a506) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "lw_05.9h",  0x00000, 0x4000, CRC(091d923c) SHA1(d686c860f147c4749ac1ee23cde5a7b570312622) ) /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "lw_14.3e",  0x00000, 0x8000, CRC(5436392c) SHA1(c33925c87e61aad278bef57fe9a8148ff2d4377f) ) /* tiles */
	ROM_LOAD( "lw_08.1e",  0x08000, 0x8000, CRC(b491bbbb) SHA1(474fc84667d978abfd5c9d94cf1e2ce55f70f865) )
	ROM_LOAD( "lw_13.3d",  0x10000, 0x8000, CRC(fdd1908a) SHA1(0b2de3d2f8e50f11c57822147bec6f2d9c9ff586) )
	ROM_LOAD( "lw_07.1d",  0x18000, 0x8000, CRC(5c73d406) SHA1(85386f6b387a85d8df7d800ffcecb2590613a42c) )
	ROM_LOAD( "lw_12.3b",  0x20000, 0x8000, CRC(32e17b3c) SHA1(db5488b7c48cd0df4571104169e42ff4094f1abd) )
	ROM_LOAD( "lw_06.1b",  0x28000, 0x8000, CRC(52e533c1) SHA1(9f333c9fb6e35db1264286be5b4f7e4dd18150de) )
	ROM_LOAD( "lw_15.3f",  0x30000, 0x8000, CRC(99e134ba) SHA1(9818a6ad3146ed95b29b9aeba2331a0e8e2a76b5) )
	ROM_LOAD( "lw_09.1f",  0x38000, 0x8000, CRC(c8f28777) SHA1(d08571d34f96e7d33506e374d047647f131dce71) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "lw_17.3j",  0x00000, 0x8000, CRC(5ed1bc9b) SHA1(717c80e180bc38cb66ac0135709e8df2cd7375aa) )  /* sprites */
	ROM_LOAD( "lw_11.1j",  0x08000, 0x8000, CRC(2a0790d6) SHA1(a0a8b5748b562e4c44cdb2e48cefbea0d4e9e6a8) )
	ROM_LOAD( "lw_16.3h",  0x10000, 0x8000, CRC(e8834006) SHA1(7d7ec16be325cbbaccf5dce101cb7bc719a5bef2) )
	ROM_LOAD( "lw_10.1h",  0x18000, 0x8000, CRC(b693f5a5) SHA1(134e255e670848f8aec82fcd848d1a4f1aefa636) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "szb01.15g",   0x0000, 0x0100, CRC(d96bcc98) SHA1(99e69a624d5586e5eedacd2083fa68b36e7b5e40) )    /* 63s141, timing (not used) */
ROM_END

ROM_START( lwingsa ) // Is this an original or a bootleg set???
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code + 3*16k for the banked ROMs images */
	ROM_LOAD( "u13-l",     0x00000, 0x8000, CRC(3069c01c) SHA1(84dfffeb58f7c5a75d2a59c2ce72c6db813af1be) ) // need to verify label & rev - lw_01a.6c??
	ROM_LOAD( "u14-k",     0x10000, 0x8000, CRC(5d91c828) SHA1(e0b9eab5b290203f71de27a78689adb2e7b07cea) ) // need to verify label & rev - lw_02.7c??
	ROM_LOAD( "lw_03.9c",  0x18000, 0x8000, CRC(ec5cc201) SHA1(1043c6a9678c18fef920be91b0796c93b83e0f73) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "lw_04.11e", 0x0000, 0x8000, CRC(a20337a2) SHA1(649e13a69ad9154657894fa7bf7c6e49b029a506) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "lw_05.9h",  0x00000, 0x4000, CRC(091d923c) SHA1(d686c860f147c4749ac1ee23cde5a7b570312622) ) /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "lw_14.3e",  0x00000, 0x8000, CRC(5436392c) SHA1(c33925c87e61aad278bef57fe9a8148ff2d4377f) ) /* tiles */
	ROM_LOAD( "lw_08.1e",  0x08000, 0x8000, CRC(b491bbbb) SHA1(474fc84667d978abfd5c9d94cf1e2ce55f70f865) )
	ROM_LOAD( "lw_13.3d",  0x10000, 0x8000, CRC(fdd1908a) SHA1(0b2de3d2f8e50f11c57822147bec6f2d9c9ff586) )
	ROM_LOAD( "lw_07.1d",  0x18000, 0x8000, CRC(5c73d406) SHA1(85386f6b387a85d8df7d800ffcecb2590613a42c) )
	ROM_LOAD( "lw_12.3b",  0x20000, 0x8000, CRC(32e17b3c) SHA1(db5488b7c48cd0df4571104169e42ff4094f1abd) )
	ROM_LOAD( "lw_06.1b",  0x28000, 0x8000, CRC(52e533c1) SHA1(9f333c9fb6e35db1264286be5b4f7e4dd18150de) )
	ROM_LOAD( "lw_15.3f",  0x30000, 0x8000, CRC(99e134ba) SHA1(9818a6ad3146ed95b29b9aeba2331a0e8e2a76b5) )
	ROM_LOAD( "lw_09.1f",  0x38000, 0x8000, CRC(c8f28777) SHA1(d08571d34f96e7d33506e374d047647f131dce71) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "lw_17.3j",  0x00000, 0x8000, CRC(5ed1bc9b) SHA1(717c80e180bc38cb66ac0135709e8df2cd7375aa) )  /* sprites */
	ROM_LOAD( "lw_11.1j",  0x08000, 0x8000, CRC(2a0790d6) SHA1(a0a8b5748b562e4c44cdb2e48cefbea0d4e9e6a8) )
	ROM_LOAD( "lw_16.3h",  0x10000, 0x8000, CRC(e8834006) SHA1(7d7ec16be325cbbaccf5dce101cb7bc719a5bef2) )
	ROM_LOAD( "lw_10.1h",  0x18000, 0x8000, CRC(b693f5a5) SHA1(134e255e670848f8aec82fcd848d1a4f1aefa636) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "szb01.15g",   0x0000, 0x0100, CRC(d96bcc98) SHA1(99e69a624d5586e5eedacd2083fa68b36e7b5e40) )    /* 63s141, timing (not used) */
ROM_END

ROM_START( lwingsj )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code + 3*16k for the banked ROMs images */
	ROM_LOAD( "at_01b.6c",   0x00000, 0x8000, CRC(2068a738) SHA1(1bbceee8138cdc3832a9330b967561b78b03933e) )
	ROM_LOAD( "at_02.7c",    0x10000, 0x8000, CRC(d6a2edc4) SHA1(ce7eef643b1570cab241355bfd7c2d7adb1e74b6) )
	ROM_LOAD( "at_03.9c",    0x18000, 0x8000, CRC(ec5cc201) SHA1(1043c6a9678c18fef920be91b0796c93b83e0f73) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "at_03.11e", 0x0000, 0x8000, CRC(a20337a2) SHA1(649e13a69ad9154657894fa7bf7c6e49b029a506) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "at_05.9h",  0x00000, 0x4000, CRC(091d923c) SHA1(d686c860f147c4749ac1ee23cde5a7b570312622) )  /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "at_14.3e",    0x00000, 0x8000, CRC(176e3027) SHA1(31947205c7a28d25b5982a9e6c079112c404d6b4) )  /* tiles */
	ROM_LOAD( "at_08.1e",    0x08000, 0x8000, CRC(f5d25623) SHA1(ff520df50011af5688be7e88712faa8f8436b462) )
	ROM_LOAD( "at_13.3d",    0x10000, 0x8000, CRC(001caa35) SHA1(2042136c592ce124a321fc6d05447b13a612b6b9) )
	ROM_LOAD( "at_07.1d",    0x18000, 0x8000, CRC(0ba008c3) SHA1(ed5c0d7191d021d6445f8f31a61eb99172fd2dc1) )
	ROM_LOAD( "at_12.3b",    0x20000, 0x8000, CRC(4f8182e9) SHA1(d0db174995be3937f5e5fe62ffe2112583dd78d7) )
	ROM_LOAD( "at_06.1b",    0x28000, 0x8000, CRC(f1617374) SHA1(01b77bc16c1e7d669f62adf759f820bc0241d959) )
	ROM_LOAD( "at_15.3f",    0x30000, 0x8000, CRC(9b374dcc) SHA1(3cb4243c304579536880ced86f0118c43413c1b4) )
	ROM_LOAD( "at_09.1f",    0x38000, 0x8000, CRC(23654e0a) SHA1(d97689b348ac4e1b380ad65133ede4bdd5ecaaee) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "at_17.3j",    0x00000, 0x8000, CRC(8f3c763a) SHA1(b34e62ab6652a2e9783351dde6a60af38a6ba084) )  /* sprites */
	ROM_LOAD( "at_11.1j",    0x08000, 0x8000, CRC(7cc90a1d) SHA1(ff194749397f06ad054917664bd4583b0e4e8d92) )
	ROM_LOAD( "at_16.3h",    0x10000, 0x8000, CRC(7d58f532) SHA1(debfb14cd1cefa1f61a8650cbc9f6e0fff3abe8b) )
	ROM_LOAD( "at_10.1h",    0x18000, 0x8000, CRC(3e396eda) SHA1(a736f108e0ed5fab6177f0d8a21feab8b686ee85) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "szb01.15g",   0x0000, 0x0100, CRC(d96bcc98) SHA1(99e69a624d5586e5eedacd2083fa68b36e7b5e40) )    /* 63s141, timing (not used) */
ROM_END

ROM_START( lwingsja ) // PCB Capcom 86607-A-2 + 86607-B-2, only different ROM from lwingsj is AT_01A.6c
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code + 3*16k for the banked ROMs images */
	ROM_LOAD( "at_01a.6c",   0x00000, 0x8000, CRC(568f1ea5) SHA1(b1e9a5f06793de7c9e0bf41eae2dd3a6ab5fc8be) )
	ROM_LOAD( "at_02.7c",    0x10000, 0x8000, CRC(d6a2edc4) SHA1(ce7eef643b1570cab241355bfd7c2d7adb1e74b6) )
	ROM_LOAD( "at_03.9c",    0x18000, 0x8000, CRC(ec5cc201) SHA1(1043c6a9678c18fef920be91b0796c93b83e0f73) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "at_03.11e", 0x0000, 0x8000, CRC(a20337a2) SHA1(649e13a69ad9154657894fa7bf7c6e49b029a506) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "at_05.9h",  0x00000, 0x4000, CRC(091d923c) SHA1(d686c860f147c4749ac1ee23cde5a7b570312622) )  /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "at_14.3e",    0x00000, 0x8000, CRC(176e3027) SHA1(31947205c7a28d25b5982a9e6c079112c404d6b4) )  /* tiles */
	ROM_LOAD( "at_08.1e",    0x08000, 0x8000, CRC(f5d25623) SHA1(ff520df50011af5688be7e88712faa8f8436b462) )
	ROM_LOAD( "at_13.3d",    0x10000, 0x8000, CRC(001caa35) SHA1(2042136c592ce124a321fc6d05447b13a612b6b9) )
	ROM_LOAD( "at_07.1d",    0x18000, 0x8000, CRC(0ba008c3) SHA1(ed5c0d7191d021d6445f8f31a61eb99172fd2dc1) )
	ROM_LOAD( "at_12.3b",    0x20000, 0x8000, CRC(4f8182e9) SHA1(d0db174995be3937f5e5fe62ffe2112583dd78d7) )
	ROM_LOAD( "at_06.1b",    0x28000, 0x8000, CRC(f1617374) SHA1(01b77bc16c1e7d669f62adf759f820bc0241d959) )
	ROM_LOAD( "at_15.3f",    0x30000, 0x8000, CRC(9b374dcc) SHA1(3cb4243c304579536880ced86f0118c43413c1b4) )
	ROM_LOAD( "at_09.1f",    0x38000, 0x8000, CRC(23654e0a) SHA1(d97689b348ac4e1b380ad65133ede4bdd5ecaaee) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "at_17.3j",    0x00000, 0x8000, CRC(8f3c763a) SHA1(b34e62ab6652a2e9783351dde6a60af38a6ba084) )  /* sprites */
	ROM_LOAD( "at_11.1j",    0x08000, 0x8000, CRC(7cc90a1d) SHA1(ff194749397f06ad054917664bd4583b0e4e8d92) )
	ROM_LOAD( "at_16.3h",    0x10000, 0x8000, CRC(7d58f532) SHA1(debfb14cd1cefa1f61a8650cbc9f6e0fff3abe8b) )
	ROM_LOAD( "at_10.1h",    0x18000, 0x8000, CRC(3e396eda) SHA1(a736f108e0ed5fab6177f0d8a21feab8b686ee85) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "szb01.15g",   0x0000, 0x0100, CRC(d96bcc98) SHA1(99e69a624d5586e5eedacd2083fa68b36e7b5e40) )    /* 63s141, timing (not used) */
ROM_END

ROM_START( lwingsb )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code + 3*16k for the banked ROMs images */
	ROM_LOAD( "ic17.bin",  0x00000, 0x8000, CRC(fe8a8823) SHA1(aa968fda368cc904b22ea68d7b5d4fcfba2227b1) )
	ROM_LOAD( "ic18.bin",  0x10000, 0x8000, CRC(2a00cde8) SHA1(5b2ef3bb08aed1b99eee0c6d7f5b9d3af807c13e) )
	ROM_LOAD( "ic19.bin",  0x18000, 0x8000, CRC(ec5cc201) SHA1(1043c6a9678c18fef920be91b0796c93b83e0f73) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "ic37.bin", 0x0000, 0x8000, CRC(a20337a2) SHA1(649e13a69ad9154657894fa7bf7c6e49b029a506) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "ic60.bin",  0x00000, 0x4000, CRC(091d923c) SHA1(d686c860f147c4749ac1ee23cde5a7b570312622) ) /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "ic50.bin",  0x00000, 0x8000, CRC(5436392c) SHA1(c33925c87e61aad278bef57fe9a8148ff2d4377f) ) /* tiles */
	ROM_LOAD( "ic49.bin",  0x08000, 0x8000, CRC(ffdbdd69) SHA1(746eb51ae2b70349bc51099092442fb05b02d64c) )
	ROM_LOAD( "ic26.bin",  0x10000, 0x8000, CRC(fdd1908a) SHA1(0b2de3d2f8e50f11c57822147bec6f2d9c9ff586) )
	ROM_LOAD( "ic25.bin",  0x18000, 0x8000, CRC(5c73d406) SHA1(85386f6b387a85d8df7d800ffcecb2590613a42c) )
	ROM_LOAD( "ic2.bin",   0x20000, 0x8000, CRC(32e17b3c) SHA1(db5488b7c48cd0df4571104169e42ff4094f1abd) )
	ROM_LOAD( "ic1.bin",   0x28000, 0x8000, CRC(52e533c1) SHA1(9f333c9fb6e35db1264286be5b4f7e4dd18150de) )
	ROM_LOAD( "ic63.bin",  0x30000, 0x8000, CRC(99e134ba) SHA1(9818a6ad3146ed95b29b9aeba2331a0e8e2a76b5) )
	ROM_LOAD( "ic62.bin",  0x38000, 0x8000, CRC(c8f28777) SHA1(d08571d34f96e7d33506e374d047647f131dce71) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "ic99.bin",  0x00000, 0x8000, CRC(163946da) SHA1(719735ccb965c91b152ef07d64393506808c8f55) )  /* sprites */
	ROM_LOAD( "ic98.bin",  0x08000, 0x8000, CRC(7cc90a1d) SHA1(ff194749397f06ad054917664bd4583b0e4e8d92) )
	ROM_LOAD( "ic87.bin",  0x10000, 0x8000, CRC(bca275ac) SHA1(c1cdf9f7f5e99ff85521f27565047455f0c2b78b) )
	ROM_LOAD( "ic86.bin",  0x18000, 0x8000, CRC(3e396eda) SHA1(a736f108e0ed5fab6177f0d8a21feab8b686ee85) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "63s141.15g",   0x0000, 0x0100, CRC(d96bcc98) SHA1(99e69a624d5586e5eedacd2083fa68b36e7b5e40) )    /* timing (not used) */
ROM_END


ROM_START( fball )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "d4.bin", 0x00000, 0x20000, CRC(6122b3dc) SHA1(25aad9a7a26a10985a4af2de34d48ac917cfff04) )

	ROM_REGION( 0x01000, "soundcpu", ROMREGION_ERASEFF )
	ROM_LOAD( "a05.bin", 0x00000, 0x01000, CRC(474dd19e) SHA1(962837716f54d0de2afb7f9df29f96b2e023bbcb) ) // BADADDR        ----xxxxxxxxxxxx (16x data repeat)
	ROM_IGNORE(0x01000) ROM_IGNORE(0x01000) ROM_IGNORE(0x01000) ROM_IGNORE(0x01000)
	ROM_IGNORE(0x01000) ROM_IGNORE(0x01000) ROM_IGNORE(0x01000) ROM_IGNORE(0x01000)
	ROM_IGNORE(0x01000) ROM_IGNORE(0x01000) ROM_IGNORE(0x01000) ROM_IGNORE(0x01000)
	ROM_IGNORE(0x01000) ROM_IGNORE(0x01000) ROM_IGNORE(0x01000)

	ROM_REGION( 0x04000, "gfx1", ROMREGION_ERASEFF )
	ROM_LOAD( "j03.bin", 0x00000, 0x04000, CRC(be11627f) SHA1(de6b25e1b951d786d28a1c26716587754cfdc0df) ) // BADADDR        --xxxxxxxxxxxxxx (4x data repeat)
	ROM_IGNORE(0x04000)
	ROM_IGNORE(0x04000)
	ROM_IGNORE(0x04000)

	ROM_REGION( 0x40000, "gfx2", ROMREGION_ERASEFF )
	ROM_LOAD( "b15.bin", 0x20000, 0x10000, CRC(2169ad3e) SHA1(5628b97e6f4ad4291eb98b02ea8f9b2282b44c60) ) ROM_IGNORE(0x10000) // 1ST AND 2ND HALF IDENTICAL
	ROM_LOAD( "c15.bin", 0x10000, 0x10000, CRC(0f77b03e) SHA1(23e4e7268346abcbadd9e42184853e2884a27430) ) ROM_IGNORE(0x10000) // ^
	ROM_LOAD( "e15.bin", 0x00000, 0x10000, CRC(89a761d2) SHA1(71305ede65a2fa13f4331008f851509a0e1d92f9) ) ROM_IGNORE(0x10000) // ^
	ROM_LOAD( "f15.bin", 0x30000, 0x10000, CRC(34b3f9a2) SHA1(29aeb22f0ee6b68a7a6d2a63bb99d5466d9ea798) ) ROM_IGNORE(0x10000) // ^

	ROM_REGION( 0x40000, "gfx3", ROMREGION_ERASEFF )
	ROM_LOAD( "j15.bin", 0x00000, 0x20000, CRC(ed7be8e7) SHA1(27f0e10161e0243b18326d4b23b2aaaaf4753960) )
	ROM_LOAD( "h15.bin", 0x20000, 0x20000, CRC(6ffb5433) SHA1(8001b16f51909cf3f29f06650b60d99558759194) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASEFF )
	ROM_LOAD( "a03.bin", 0x00000, 0x40000, CRC(22b0d089) SHA1(a82d04c389694e1ed0b9b24555ddd6f9d9f6ca38) )
	ROM_RELOAD(0x40000,0x40000)
	ROM_LOAD( "a02.bin", 0x80000, 0x40000, CRC(951d6579) SHA1(8976a836538eb510888f49af94dbf66dacb8f067) )
	ROM_LOAD( "a01.bin", 0xc0000, 0x40000, CRC(020b5261) SHA1(698dbd7e125e4edd988791ecdae7db9ddc0705b3) )
ROM_END


ROM_START( sectionz ) // this is likely rev C (rev B for SZ 03)
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code + 3*16k for the banked ROMs images */
	ROM_LOAD( "sz_01.6c",  0x00000, 0x8000, CRC(69585125) SHA1(a341e3a5507e961d5763be6acf420695bb32709e) ) // need to verify, should this really be szu_01c.6c??
	ROM_LOAD( "sz_02.7c",  0x10000, 0x8000, CRC(22f161b8) SHA1(094ee6b6c8750de682c1ba4e387b31d58f734604) )
	ROM_LOAD( "sz_03.9c",  0x18000, 0x8000, CRC(4c7111ed) SHA1(57c6ad6a86c64ffb17ec8f584c5e003440390344) ) // need to verify, should this really be szu_03b.9c??

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "sz_04.11e", 0x0000, 0x8000, CRC(a6073566) SHA1(d7dc382ba780cc4f25f7d7e7630cff1090488843) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "sz_05.9h",  0x00000, 0x4000, CRC(3173ba2e) SHA1(4e0b4fc1efd7b5eb598fe5d5d7f1de01ba52dbdc) )  /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "sz_14.3e",  0x00000, 0x8000, CRC(63782e30) SHA1(9a23b4849ff210bd4482e4e8c57e578387d19c46) )  /* tiles */
	ROM_LOAD( "sz_08.1e",  0x08000, 0x8000, CRC(d57d9f13) SHA1(1d07b9eca588985a5e0cec27394ad5b3191c8dc4) )
	ROM_LOAD( "sz_13.3d",  0x10000, 0x8000, CRC(1b3d4d7f) SHA1(66eed80865b2a480762cc8d9fda9e82c9c463e71) )
	ROM_LOAD( "sz_07.1d",  0x18000, 0x8000, CRC(f5b3a29f) SHA1(0dbf8caf09e319fb2303e7e865f55effa59c761c) )
	ROM_LOAD( "sz_12.3b",  0x20000, 0x8000, CRC(11d47dfd) SHA1(bc8a7369ed671ef714472ead2d17228de2567865) )
	ROM_LOAD( "sz_06.1b",  0x28000, 0x8000, CRC(df703b68) SHA1(ae98a718dab96f3c0e4827e78938c3984a6641d6) )
	ROM_LOAD( "sz_15.3f",  0x30000, 0x8000, CRC(36bb9bf7) SHA1(53f6d375947f9fb28f295935a0fe27f826234765) )
	ROM_LOAD( "sz_09.1f",  0x38000, 0x8000, CRC(da8f06c9) SHA1(c0eb4406cdf0d5f25bab28de8222b28da9a97943) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "sz_17.3j",  0x00000, 0x8000, CRC(8df7b24a) SHA1(078789d0912010fa96b6f267de3ebec9beca6681) )  /* sprites */
	ROM_LOAD( "sz_11.1j",  0x08000, 0x8000, CRC(685d4c54) SHA1(ef580e04b6dcb0b65f12c519a4085c98ac0bc261) )
	ROM_LOAD( "sz_16.3h",  0x10000, 0x8000, CRC(500ff2bb) SHA1(eb20148388e5271b1fed23a536035e8490474489) )
	ROM_LOAD( "sz_10.1h",  0x18000, 0x8000, CRC(00b3d244) SHA1(ed923bd5371f4665744344b94df3547c5db5058c) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "szb01.15g", 0x0000, 0x0100, CRC(d96bcc98) SHA1(99e69a624d5586e5eedacd2083fa68b36e7b5e40) )    /* timing (not used) */
ROM_END

ROM_START( sectionza )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code + 3*16k for the banked ROMs images */
	ROM_LOAD( "sz_01a.6c", 0x00000, 0x8000, CRC(98df49fd) SHA1(80d7d9f83ea2f606e48606dbfe69cf347aadf079) )
	ROM_LOAD( "sz_02.7c",  0x10000, 0x8000, CRC(22f161b8) SHA1(094ee6b6c8750de682c1ba4e387b31d58f734604) )
	ROM_LOAD( "szj_03.9c", 0x18000, 0x8000, CRC(94547abf) SHA1(9af9e76e6657d7fd742630cfe2f2eb76d231dec4) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "sz_04.11e", 0x0000, 0x8000, CRC(a6073566) SHA1(d7dc382ba780cc4f25f7d7e7630cff1090488843) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "sz_05.9h",  0x00000, 0x4000, CRC(3173ba2e) SHA1(4e0b4fc1efd7b5eb598fe5d5d7f1de01ba52dbdc) )  /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "sz_14.3e",  0x00000, 0x8000, CRC(63782e30) SHA1(9a23b4849ff210bd4482e4e8c57e578387d19c46) )  /* tiles */
	ROM_LOAD( "sz_08.1e",  0x08000, 0x8000, CRC(d57d9f13) SHA1(1d07b9eca588985a5e0cec27394ad5b3191c8dc4) )
	ROM_LOAD( "sz_13.3d",  0x10000, 0x8000, CRC(1b3d4d7f) SHA1(66eed80865b2a480762cc8d9fda9e82c9c463e71) )
	ROM_LOAD( "sz_07.1d",  0x18000, 0x8000, CRC(f5b3a29f) SHA1(0dbf8caf09e319fb2303e7e865f55effa59c761c) )
	ROM_LOAD( "sz_12.3b",  0x20000, 0x8000, CRC(11d47dfd) SHA1(bc8a7369ed671ef714472ead2d17228de2567865) )
	ROM_LOAD( "sz_06.1b",  0x28000, 0x8000, CRC(df703b68) SHA1(ae98a718dab96f3c0e4827e78938c3984a6641d6) )
	ROM_LOAD( "sz_15.3f",  0x30000, 0x8000, CRC(36bb9bf7) SHA1(53f6d375947f9fb28f295935a0fe27f826234765) )
	ROM_LOAD( "sz_09.1f",  0x38000, 0x8000, CRC(da8f06c9) SHA1(c0eb4406cdf0d5f25bab28de8222b28da9a97943) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "sz_17.3j",  0x00000, 0x8000, CRC(8df7b24a) SHA1(078789d0912010fa96b6f267de3ebec9beca6681) )  /* sprites */
	ROM_LOAD( "sz_11.1j",  0x08000, 0x8000, CRC(685d4c54) SHA1(ef580e04b6dcb0b65f12c519a4085c98ac0bc261) )
	ROM_LOAD( "sz_16.3h",  0x10000, 0x8000, CRC(500ff2bb) SHA1(eb20148388e5271b1fed23a536035e8490474489) )
	ROM_LOAD( "sz_10.1h",  0x18000, 0x8000, CRC(00b3d244) SHA1(ed923bd5371f4665744344b94df3547c5db5058c) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "szb01.15g", 0x0000, 0x0100, CRC(d96bcc98) SHA1(99e69a624d5586e5eedacd2083fa68b36e7b5e40) )    /* timing (not used) */
ROM_END


/*

For the Trojan sets, Capcom labeled all program ROMs as TB 04, TB 05 & TB 06.  Some sets would get
  an additional stamp for rev A and Romstar sets would "usually" have a red U stamped on the label.
  However, different Romstar PCBs have been shown with and without the "U" being stamped.

*/

ROM_START( trojan ) // This set is likely rev A - need to verify
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code + 3*16k for the banked ROMs images */
	ROM_LOAD( "tb_04.10n", 0x00000, 0x8000, CRC(c1bbeb4e) SHA1(248ae4184d25b642b282ef44ac729c0f7952834d) )
	ROM_LOAD( "tb_06.13n", 0x10000, 0x8000, CRC(d49592ef) SHA1(b538bac3c73f35474cc6745a4e4dc3ab6217eaac) )
	ROM_LOAD( "tb_05.12n", 0x18000, 0x8000, CRC(9273b264) SHA1(ab23b16bf53b5baf106ea0cac50754aa967300cf) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "tb_02.15h", 0x0000, 0x8000, CRC(21154797) SHA1(e1a3006746cc2d692ecd4369cc0a77c596abd60b) )

	ROM_REGION( 0x10000, "adpcmcpu", 0 ) /* 64k for ADPCM CPU */
	ROM_LOAD( "tb_01.6d",  0x0000, 0x4000, CRC(1c0f91b2) SHA1(163bf6aa1936994659661653eabdc368199b0070) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "tb_03.8k",  0x00000, 0x4000, CRC(581a2b4c) SHA1(705b499da5d01a946f06234a4bab72a291c79034) )     /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "tb_13.6b",  0x00000, 0x8000, CRC(285a052b) SHA1(8ce055c7ac9ce1560552fc7f857f60e7a5af0779) )     /* tiles */
	ROM_LOAD( "tb_09.6a",  0x08000, 0x8000, CRC(aeb693f7) SHA1(a811ea67abdd4adfc68224257973802e2a36fc36) )
	ROM_LOAD( "tb_12.4b",  0x10000, 0x8000, CRC(dfb0fe5c) SHA1(82542692ab71b9126e6c301ed0803db58734273c) )
	ROM_LOAD( "tb_08.4a",  0x18000, 0x8000, CRC(d3a4c9d1) SHA1(3d787f6a4583b80f2d254947890f676cda17b242) )
	ROM_LOAD( "tb_11.3b",  0x20000, 0x8000, CRC(00f0f4fd) SHA1(3a862360a26ae1c3a945949d6d47f88aa4b728a4) )
	ROM_LOAD( "tb_07.3a",  0x28000, 0x8000, CRC(dff2ee02) SHA1(4877c52f2a0e24a95bcda1d8636ea993c2c3c240) )
	ROM_LOAD( "tb_14.8b",  0x30000, 0x8000, CRC(14bfac18) SHA1(84266140e9679912dbbb185fd3b9b497297dcb16) )
	ROM_LOAD( "tb_10.8a",  0x38000, 0x8000, CRC(71ba8a6d) SHA1(53ff6850f9f8a19c57c19ef56fd45975f0ec133e) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "tb_18.7l",  0x00000, 0x8000, CRC(862c4713) SHA1(a3707d950f4f5de5208e64207016ef2256eb8c5b) )     /* sprites */
	ROM_LOAD( "tb_16.3l",  0x08000, 0x8000, CRC(d86f8cbd) SHA1(8a16130632e20ad3cae8e817da7b661c3ac60f30) )
	ROM_LOAD( "tb_17.5l",  0x10000, 0x8000, CRC(12a73b3f) SHA1(6bb54d4fdf01fd2cdd76a0b47be4d8cae8a1e19b) )
	ROM_LOAD( "tb_15.2l",  0x18000, 0x8000, CRC(bb1a2769) SHA1(9884dceb00e6d88908a1c107b83cc1711b0cf1f7) )
	ROM_LOAD( "tb_22.7n",  0x20000, 0x8000, CRC(39daafd4) SHA1(1e49a273f51cccec3141d540032fd9a3041a3cbd) )
	ROM_LOAD( "tb_20.3n",  0x28000, 0x8000, CRC(94615d2a) SHA1(112a299ff1bb878cf7e24c2ad337440c3df0a6d5) )
	ROM_LOAD( "tb_21.5n",  0x30000, 0x8000, CRC(66c642bd) SHA1(b57f0f8d8e21c9f94ffc0e9f9304b5ab5d4ed3fc) )
	ROM_LOAD( "tb_19.2n",  0x38000, 0x8000, CRC(81d5ab36) SHA1(31103759676a8d1badaf7bde79e7f28d69486106) )

	ROM_REGION( 0x10000, "gfx4", 0 )
	ROM_LOAD( "tb_25.15n", 0x00000, 0x8000, CRC(6e38c6fa) SHA1(c51228d5d063dcf4361c76fa49dbe18db80c50a0) )     /* Bk Tiles */
	ROM_LOAD( "tb_24.13n", 0x08000, 0x8000, CRC(14fc6cf2) SHA1(080a2d845cb36c637f76d8e062725bd13dd1aed0) )

	ROM_REGION( 0x08000, "gfx5", 0 )
	ROM_LOAD( "tb_23.9n",  0x00000, 0x08000, CRC(eda13c0e) SHA1(806f0819af8b25c2b46de3d1fd95bc9c0e883bd9) )   /* Tile Map */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "tbb-2.7j",  0x0000, 0x0100, CRC(d96bcc98) SHA1(99e69a624d5586e5eedacd2083fa68b36e7b5e40) ) /* timing (not used) */
	ROM_LOAD( "tbb-1.1e",  0x0100, 0x0100, CRC(5052fa9d) SHA1(8cd240f4795a7ae76499573c09069dba37182be2) ) /* priority (not used) */
ROM_END

ROM_START( trojana )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code + 3*16k for the banked ROMs images */
	ROM_LOAD( "tb_04.10n", 0x00000, 0x8000, CRC(0113a551) SHA1(933ebaf73fb70772fc2cf2b9143bf00757505772) ) // SLDH
	ROM_LOAD( "tb_06.13n", 0x10000, 0x8000, CRC(aa127a5b) SHA1(0b7115c2ffe8456ef463e22d68e03a2e396abf92) ) // SLDH
	ROM_LOAD( "tb_05.12n", 0x18000, 0x8000, CRC(9273b264) SHA1(ab23b16bf53b5baf106ea0cac50754aa967300cf) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "tb_02.15h", 0x0000, 0x8000, CRC(21154797) SHA1(e1a3006746cc2d692ecd4369cc0a77c596abd60b) )

	ROM_REGION( 0x10000, "adpcmcpu", 0 ) /* 64k for ADPCM CPU */
	ROM_LOAD( "tb_01.6d",  0x0000, 0x4000, CRC(1c0f91b2) SHA1(163bf6aa1936994659661653eabdc368199b0070) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "tb_03.8k",  0x00000, 0x4000, CRC(581a2b4c) SHA1(705b499da5d01a946f06234a4bab72a291c79034) )     /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "tb_13.6b",  0x00000, 0x8000, CRC(285a052b) SHA1(8ce055c7ac9ce1560552fc7f857f60e7a5af0779) )     /* tiles */
	ROM_LOAD( "tb_09.6a",  0x08000, 0x8000, CRC(aeb693f7) SHA1(a811ea67abdd4adfc68224257973802e2a36fc36) )
	ROM_LOAD( "tb_12.4b",  0x10000, 0x8000, CRC(dfb0fe5c) SHA1(82542692ab71b9126e6c301ed0803db58734273c) )
	ROM_LOAD( "tb_08.4a",  0x18000, 0x8000, CRC(d3a4c9d1) SHA1(3d787f6a4583b80f2d254947890f676cda17b242) )
	ROM_LOAD( "tb_11.3b",  0x20000, 0x8000, CRC(00f0f4fd) SHA1(3a862360a26ae1c3a945949d6d47f88aa4b728a4) )
	ROM_LOAD( "tb_07.3a",  0x28000, 0x8000, CRC(dff2ee02) SHA1(4877c52f2a0e24a95bcda1d8636ea993c2c3c240) )
	ROM_LOAD( "tb_14.8b",  0x30000, 0x8000, CRC(14bfac18) SHA1(84266140e9679912dbbb185fd3b9b497297dcb16) )
	ROM_LOAD( "tb_10.8a",  0x38000, 0x8000, CRC(71ba8a6d) SHA1(53ff6850f9f8a19c57c19ef56fd45975f0ec133e) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "tb_18.7l",  0x00000, 0x8000, CRC(862c4713) SHA1(a3707d950f4f5de5208e64207016ef2256eb8c5b) )     /* sprites */
	ROM_LOAD( "tb_16.3l",  0x08000, 0x8000, CRC(d86f8cbd) SHA1(8a16130632e20ad3cae8e817da7b661c3ac60f30) )
	ROM_LOAD( "tb_17.5l",  0x10000, 0x8000, CRC(12a73b3f) SHA1(6bb54d4fdf01fd2cdd76a0b47be4d8cae8a1e19b) )
	ROM_LOAD( "tb_15.2l",  0x18000, 0x8000, CRC(bb1a2769) SHA1(9884dceb00e6d88908a1c107b83cc1711b0cf1f7) )
	ROM_LOAD( "tb_22.7n",  0x20000, 0x8000, CRC(39daafd4) SHA1(1e49a273f51cccec3141d540032fd9a3041a3cbd) )
	ROM_LOAD( "tb_20.3n",  0x28000, 0x8000, CRC(94615d2a) SHA1(112a299ff1bb878cf7e24c2ad337440c3df0a6d5) )
	ROM_LOAD( "tb_21.5n",  0x30000, 0x8000, CRC(66c642bd) SHA1(b57f0f8d8e21c9f94ffc0e9f9304b5ab5d4ed3fc) )
	ROM_LOAD( "tb_19.2n",  0x38000, 0x8000, CRC(81d5ab36) SHA1(31103759676a8d1badaf7bde79e7f28d69486106) )

	ROM_REGION( 0x10000, "gfx4", 0 )
	ROM_LOAD( "tb_25.15n", 0x00000, 0x8000, CRC(6e38c6fa) SHA1(c51228d5d063dcf4361c76fa49dbe18db80c50a0) )     /* Bk Tiles */
	ROM_LOAD( "tb_24.13n", 0x08000, 0x8000, CRC(14fc6cf2) SHA1(080a2d845cb36c637f76d8e062725bd13dd1aed0) )

	ROM_REGION( 0x08000, "gfx5", 0 )
	ROM_LOAD( "tb_23.9n",  0x00000, 0x08000, CRC(eda13c0e) SHA1(806f0819af8b25c2b46de3d1fd95bc9c0e883bd9) )   /* Tile Map */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "tbb-2.7j",  0x0000, 0x0100, CRC(d96bcc98) SHA1(99e69a624d5586e5eedacd2083fa68b36e7b5e40) ) /* timing (not used) */
	ROM_LOAD( "tbb-1.1e",  0x0100, 0x0100, CRC(5052fa9d) SHA1(8cd240f4795a7ae76499573c09069dba37182be2) ) /* priority (not used) */
ROM_END

ROM_START( trojanr ) // This set is likely rev A - need to verify
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code + 3*16k for the banked ROMs images */
	ROM_LOAD( "tbu_04.10n", 0x00000, 0x8000, CRC(92670f27) SHA1(d2cb35a9fade971770db1a58e961bc03cc3de6ff) )
	ROM_LOAD( "tbu_06.13n", 0x10000, 0x8000, CRC(a4951173) SHA1(2d3db0ee3a1680f2cce21cf15f8bd434325d8648) )
	ROM_LOAD( "tbu_05.12n", 0x18000, 0x8000, CRC(9273b264) SHA1(ab23b16bf53b5baf106ea0cac50754aa967300cf) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "tb_02.15h", 0x0000, 0x8000, CRC(21154797) SHA1(e1a3006746cc2d692ecd4369cc0a77c596abd60b) )

	ROM_REGION( 0x10000, "adpcmcpu", 0 ) /* 64k for ADPCM CPU */
	ROM_LOAD( "tb_01.6d",  0x0000, 0x4000, CRC(1c0f91b2) SHA1(163bf6aa1936994659661653eabdc368199b0070) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "tb_03.8k",  0x00000, 0x4000, CRC(581a2b4c) SHA1(705b499da5d01a946f06234a4bab72a291c79034) )     /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "tb_13.6b",  0x00000, 0x8000, CRC(285a052b) SHA1(8ce055c7ac9ce1560552fc7f857f60e7a5af0779) )     /* tiles */
	ROM_LOAD( "tb_09.6a",  0x08000, 0x8000, CRC(aeb693f7) SHA1(a811ea67abdd4adfc68224257973802e2a36fc36) )
	ROM_LOAD( "tb_12.4b",  0x10000, 0x8000, CRC(dfb0fe5c) SHA1(82542692ab71b9126e6c301ed0803db58734273c) )
	ROM_LOAD( "tb_08.4a",  0x18000, 0x8000, CRC(d3a4c9d1) SHA1(3d787f6a4583b80f2d254947890f676cda17b242) )
	ROM_LOAD( "tb_11.3b",  0x20000, 0x8000, CRC(00f0f4fd) SHA1(3a862360a26ae1c3a945949d6d47f88aa4b728a4) )
	ROM_LOAD( "tb_07.3a",  0x28000, 0x8000, CRC(dff2ee02) SHA1(4877c52f2a0e24a95bcda1d8636ea993c2c3c240) )
	ROM_LOAD( "tb_14.8b",  0x30000, 0x8000, CRC(14bfac18) SHA1(84266140e9679912dbbb185fd3b9b497297dcb16) )
	ROM_LOAD( "tb_10.8a",  0x38000, 0x8000, CRC(71ba8a6d) SHA1(53ff6850f9f8a19c57c19ef56fd45975f0ec133e) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "tb_18.7l",  0x00000, 0x8000, CRC(862c4713) SHA1(a3707d950f4f5de5208e64207016ef2256eb8c5b) )     /* sprites */
	ROM_LOAD( "tb_16.3l",  0x08000, 0x8000, CRC(d86f8cbd) SHA1(8a16130632e20ad3cae8e817da7b661c3ac60f30) )
	ROM_LOAD( "tb_17.5l",  0x10000, 0x8000, CRC(12a73b3f) SHA1(6bb54d4fdf01fd2cdd76a0b47be4d8cae8a1e19b) )
	ROM_LOAD( "tb_15.2l",  0x18000, 0x8000, CRC(bb1a2769) SHA1(9884dceb00e6d88908a1c107b83cc1711b0cf1f7) )
	ROM_LOAD( "tb_22.7n",  0x20000, 0x8000, CRC(39daafd4) SHA1(1e49a273f51cccec3141d540032fd9a3041a3cbd) )
	ROM_LOAD( "tb_20.3n",  0x28000, 0x8000, CRC(94615d2a) SHA1(112a299ff1bb878cf7e24c2ad337440c3df0a6d5) )
	ROM_LOAD( "tb_21.5n",  0x30000, 0x8000, CRC(66c642bd) SHA1(b57f0f8d8e21c9f94ffc0e9f9304b5ab5d4ed3fc) )
	ROM_LOAD( "tb_19.2n",  0x38000, 0x8000, CRC(81d5ab36) SHA1(31103759676a8d1badaf7bde79e7f28d69486106) )

	ROM_REGION( 0x10000, "gfx4", 0 )
	ROM_LOAD( "tb_25.15n", 0x00000, 0x8000, CRC(6e38c6fa) SHA1(c51228d5d063dcf4361c76fa49dbe18db80c50a0) )     /* Bk Tiles */
	ROM_LOAD( "tb_24.13n", 0x08000, 0x8000, CRC(14fc6cf2) SHA1(080a2d845cb36c637f76d8e062725bd13dd1aed0) )

	ROM_REGION( 0x08000, "gfx5", 0 )
	ROM_LOAD( "tb_23.9n",  0x00000, 0x08000, CRC(eda13c0e) SHA1(806f0819af8b25c2b46de3d1fd95bc9c0e883bd9) )   /* Tile Map */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "tbb-2.7j",  0x0000, 0x0100, CRC(d96bcc98) SHA1(99e69a624d5586e5eedacd2083fa68b36e7b5e40) ) /* timing (not used) */
	ROM_LOAD( "tbb-1.1e",  0x0100, 0x0100, CRC(5052fa9d) SHA1(8cd240f4795a7ae76499573c09069dba37182be2) ) /* priority (not used) */
ROM_END

ROM_START( trojanra )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code + 3*16k for the banked ROMs images */
	ROM_LOAD( "tbu_04.10n", 0x00000, 0x8000, CRC(0e003b0f) SHA1(1f5559943cfa3dce5b9022771e991371a1dc6ec4) ) // SLDH
	ROM_LOAD( "tbu_06.13n", 0x10000, 0x8000, CRC(a5a3d848) SHA1(5ca1026f3edcb50ebea4362d10244ebe459b3fa5) ) // SLDH
	ROM_LOAD( "tbu_05.12n", 0x18000, 0x8000, CRC(9273b264) SHA1(ab23b16bf53b5baf106ea0cac50754aa967300cf) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "tb_02.15h", 0x0000, 0x8000, CRC(21154797) SHA1(e1a3006746cc2d692ecd4369cc0a77c596abd60b) )

	ROM_REGION( 0x10000, "adpcmcpu", 0 ) /* 64k for ADPCM CPU */
	ROM_LOAD( "tb_01.6d",  0x0000, 0x4000, CRC(1c0f91b2) SHA1(163bf6aa1936994659661653eabdc368199b0070) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "tb_03.8k",  0x00000, 0x4000, CRC(581a2b4c) SHA1(705b499da5d01a946f06234a4bab72a291c79034) )     /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "tb_13.6b",  0x00000, 0x8000, CRC(285a052b) SHA1(8ce055c7ac9ce1560552fc7f857f60e7a5af0779) )     /* tiles */
	ROM_LOAD( "tb_09.6a",  0x08000, 0x8000, CRC(aeb693f7) SHA1(a811ea67abdd4adfc68224257973802e2a36fc36) )
	ROM_LOAD( "tb_12.4b",  0x10000, 0x8000, CRC(dfb0fe5c) SHA1(82542692ab71b9126e6c301ed0803db58734273c) )
	ROM_LOAD( "tb_08.4a",  0x18000, 0x8000, CRC(d3a4c9d1) SHA1(3d787f6a4583b80f2d254947890f676cda17b242) )
	ROM_LOAD( "tb_11.3b",  0x20000, 0x8000, CRC(00f0f4fd) SHA1(3a862360a26ae1c3a945949d6d47f88aa4b728a4) )
	ROM_LOAD( "tb_07.3a",  0x28000, 0x8000, CRC(dff2ee02) SHA1(4877c52f2a0e24a95bcda1d8636ea993c2c3c240) )
	ROM_LOAD( "tb_14.8b",  0x30000, 0x8000, CRC(14bfac18) SHA1(84266140e9679912dbbb185fd3b9b497297dcb16) )
	ROM_LOAD( "tb_10.8a",  0x38000, 0x8000, CRC(71ba8a6d) SHA1(53ff6850f9f8a19c57c19ef56fd45975f0ec133e) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "tb_18.7l",  0x00000, 0x8000, CRC(862c4713) SHA1(a3707d950f4f5de5208e64207016ef2256eb8c5b) )     /* sprites */
	ROM_LOAD( "tb_16.3l",  0x08000, 0x8000, CRC(d86f8cbd) SHA1(8a16130632e20ad3cae8e817da7b661c3ac60f30) )
	ROM_LOAD( "tb_17.5l",  0x10000, 0x8000, CRC(12a73b3f) SHA1(6bb54d4fdf01fd2cdd76a0b47be4d8cae8a1e19b) )
	ROM_LOAD( "tb_15.2l",  0x18000, 0x8000, CRC(bb1a2769) SHA1(9884dceb00e6d88908a1c107b83cc1711b0cf1f7) )
	ROM_LOAD( "tb_22.7n",  0x20000, 0x8000, CRC(39daafd4) SHA1(1e49a273f51cccec3141d540032fd9a3041a3cbd) )
	ROM_LOAD( "tb_20.3n",  0x28000, 0x8000, CRC(94615d2a) SHA1(112a299ff1bb878cf7e24c2ad337440c3df0a6d5) )
	ROM_LOAD( "tb_21.5n",  0x30000, 0x8000, CRC(66c642bd) SHA1(b57f0f8d8e21c9f94ffc0e9f9304b5ab5d4ed3fc) )
	ROM_LOAD( "tb_19.2n",  0x38000, 0x8000, CRC(81d5ab36) SHA1(31103759676a8d1badaf7bde79e7f28d69486106) )

	ROM_REGION( 0x10000, "gfx4", 0 )
	ROM_LOAD( "tb_25.15n", 0x00000, 0x8000, CRC(6e38c6fa) SHA1(c51228d5d063dcf4361c76fa49dbe18db80c50a0) )     /* Bk Tiles */
	ROM_LOAD( "tb_24.13n", 0x08000, 0x8000, CRC(14fc6cf2) SHA1(080a2d845cb36c637f76d8e062725bd13dd1aed0) )

	ROM_REGION( 0x08000, "gfx5", 0 )
	ROM_LOAD( "tb_23.9n",  0x00000, 0x08000, CRC(eda13c0e) SHA1(806f0819af8b25c2b46de3d1fd95bc9c0e883bd9) )   /* Tile Map */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "tbb-2.7j",  0x0000, 0x0100, CRC(d96bcc98) SHA1(99e69a624d5586e5eedacd2083fa68b36e7b5e40) ) /* timing (not used) */
	ROM_LOAD( "tbb-1.1e",  0x0100, 0x0100, CRC(5052fa9d) SHA1(8cd240f4795a7ae76499573c09069dba37182be2) ) /* priority (not used) */
ROM_END

ROM_START( trojanj ) // This set is likely rev A - need to verify
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code + 3*16k for the banked ROMs images */
	ROM_LOAD( "tb_04.10n", 0x00000, 0x8000, CRC(0b5a7f49) SHA1(eebdfaf905a2b7ac8a0f0f9a7ae4a0daf130a5ea) ) // SLDH
	ROM_LOAD( "tb_06.13n", 0x10000, 0x8000, CRC(dee6ed92) SHA1(80aa16f2ae23581d00f4d58a2075993e7171ed0c) ) // SLDH
	ROM_LOAD( "tb_05.12n", 0x18000, 0x8000, CRC(9273b264) SHA1(ab23b16bf53b5baf106ea0cac50754aa967300cf) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "tb_02.15h", 0x0000, 0x8000, CRC(21154797) SHA1(e1a3006746cc2d692ecd4369cc0a77c596abd60b) )

	ROM_REGION( 0x10000, "adpcmcpu", 0 ) /* 64k for ADPCM CPU */
	ROM_LOAD( "tb_01.6d",  0x0000, 0x8000, CRC(83c715b2) SHA1(0c69c086657f91828a639ff7c72c703a27ade710) ) // Real board is use 27c256

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "tb_03.8k",  0x00000, 0x4000, CRC(581a2b4c) SHA1(705b499da5d01a946f06234a4bab72a291c79034) )     /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "tb_13.6b",  0x00000, 0x8000, CRC(285a052b) SHA1(8ce055c7ac9ce1560552fc7f857f60e7a5af0779) )     /* tiles */
	ROM_LOAD( "tb_09.6a",  0x08000, 0x8000, CRC(aeb693f7) SHA1(a811ea67abdd4adfc68224257973802e2a36fc36) )
	ROM_LOAD( "tb_12.4b",  0x10000, 0x8000, CRC(dfb0fe5c) SHA1(82542692ab71b9126e6c301ed0803db58734273c) )
	ROM_LOAD( "tb_08.4a",  0x18000, 0x8000, CRC(d3a4c9d1) SHA1(3d787f6a4583b80f2d254947890f676cda17b242) )
	ROM_LOAD( "tb_11.3b",  0x20000, 0x8000, CRC(00f0f4fd) SHA1(3a862360a26ae1c3a945949d6d47f88aa4b728a4) )
	ROM_LOAD( "tb_07.3a",  0x28000, 0x8000, CRC(dff2ee02) SHA1(4877c52f2a0e24a95bcda1d8636ea993c2c3c240) )
	ROM_LOAD( "tb_14.8b",  0x30000, 0x8000, CRC(14bfac18) SHA1(84266140e9679912dbbb185fd3b9b497297dcb16) )
	ROM_LOAD( "tb_10.8a",  0x38000, 0x8000, CRC(71ba8a6d) SHA1(53ff6850f9f8a19c57c19ef56fd45975f0ec133e) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "tb_18.7l",  0x00000, 0x8000, CRC(862c4713) SHA1(a3707d950f4f5de5208e64207016ef2256eb8c5b) )     /* sprites */
	ROM_LOAD( "tb_16.3l",  0x08000, 0x8000, CRC(d86f8cbd) SHA1(8a16130632e20ad3cae8e817da7b661c3ac60f30) )
	ROM_LOAD( "tb_17.5l",  0x10000, 0x8000, CRC(12a73b3f) SHA1(6bb54d4fdf01fd2cdd76a0b47be4d8cae8a1e19b) )
	ROM_LOAD( "tb_15.2l",  0x18000, 0x8000, CRC(bb1a2769) SHA1(9884dceb00e6d88908a1c107b83cc1711b0cf1f7) )
	ROM_LOAD( "tb_22.7n",  0x20000, 0x8000, CRC(39daafd4) SHA1(1e49a273f51cccec3141d540032fd9a3041a3cbd) )
	ROM_LOAD( "tb_20.3n",  0x28000, 0x8000, CRC(94615d2a) SHA1(112a299ff1bb878cf7e24c2ad337440c3df0a6d5) )
	ROM_LOAD( "tb_21.5n",  0x30000, 0x8000, CRC(66c642bd) SHA1(b57f0f8d8e21c9f94ffc0e9f9304b5ab5d4ed3fc) )
	ROM_LOAD( "tb_19.2n",  0x38000, 0x8000, CRC(81d5ab36) SHA1(31103759676a8d1badaf7bde79e7f28d69486106) )

	ROM_REGION( 0x10000, "gfx4", 0 )
	ROM_LOAD( "tb_25.15n", 0x00000, 0x8000, CRC(6e38c6fa) SHA1(c51228d5d063dcf4361c76fa49dbe18db80c50a0) )     /* Bk Tiles */
	ROM_LOAD( "tb_24.13n", 0x08000, 0x8000, CRC(14fc6cf2) SHA1(080a2d845cb36c637f76d8e062725bd13dd1aed0) )

	ROM_REGION( 0x08000, "gfx5", 0 )
	ROM_LOAD( "tb_23.9n",  0x00000, 0x08000, CRC(eda13c0e) SHA1(806f0819af8b25c2b46de3d1fd95bc9c0e883bd9) )   /* Tile Map */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "tbb-2.7j",  0x0000, 0x0100, CRC(d96bcc98) SHA1(99e69a624d5586e5eedacd2083fa68b36e7b5e40) ) /* timing (not used) */
	ROM_LOAD( "tbb-1.1e",  0x0100, 0x0100, CRC(5052fa9d) SHA1(8cd240f4795a7ae76499573c09069dba37182be2) ) /* priority (not used) */
ROM_END

ROM_START( trojanjo )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code + 3*16k for the banked ROMs images */
	ROM_LOAD( "tb_04.10n", 0x00000, 0x8000, CRC(134dc35b) SHA1(6360c1efa7c4e1d6d817a97ca43dd4af8ed6afe5) ) // SLDH
	ROM_LOAD( "tb_06.13n", 0x10000, 0x8000, CRC(fdda9d55) SHA1(5c2b84418ada1b9bfd548ca23939a6b20613a63a) ) // SLDH
	ROM_LOAD( "tb_05.12n", 0x18000, 0x8000, CRC(9273b264) SHA1(ab23b16bf53b5baf106ea0cac50754aa967300cf) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "tb_02.15h", 0x0000, 0x8000, CRC(21154797) SHA1(e1a3006746cc2d692ecd4369cc0a77c596abd60b) )

	ROM_REGION( 0x10000, "adpcmcpu", 0 ) /* 64k for ADPCM CPU */
	ROM_LOAD( "tb_01.6d",  0x0000, 0x8000, CRC(83c715b2) SHA1(0c69c086657f91828a639ff7c72c703a27ade710) ) // Real board is use 27c256

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "tb_03.8k",  0x00000, 0x4000, CRC(581a2b4c) SHA1(705b499da5d01a946f06234a4bab72a291c79034) )     /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "tb_13.6b",  0x00000, 0x8000, CRC(285a052b) SHA1(8ce055c7ac9ce1560552fc7f857f60e7a5af0779) )     /* tiles */
	ROM_LOAD( "tb_09.6a",  0x08000, 0x8000, CRC(aeb693f7) SHA1(a811ea67abdd4adfc68224257973802e2a36fc36) )
	ROM_LOAD( "tb_12.4b",  0x10000, 0x8000, CRC(dfb0fe5c) SHA1(82542692ab71b9126e6c301ed0803db58734273c) )
	ROM_LOAD( "tb_08.4a",  0x18000, 0x8000, CRC(d3a4c9d1) SHA1(3d787f6a4583b80f2d254947890f676cda17b242) )
	ROM_LOAD( "tb_11.3b",  0x20000, 0x8000, CRC(00f0f4fd) SHA1(3a862360a26ae1c3a945949d6d47f88aa4b728a4) )
	ROM_LOAD( "tb_07.3a",  0x28000, 0x8000, CRC(dff2ee02) SHA1(4877c52f2a0e24a95bcda1d8636ea993c2c3c240) )
	ROM_LOAD( "tb_14.8b",  0x30000, 0x8000, CRC(14bfac18) SHA1(84266140e9679912dbbb185fd3b9b497297dcb16) )
	ROM_LOAD( "tb_10.8a",  0x38000, 0x8000, CRC(71ba8a6d) SHA1(53ff6850f9f8a19c57c19ef56fd45975f0ec133e) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "tb_18.7l",  0x00000, 0x8000, CRC(862c4713) SHA1(a3707d950f4f5de5208e64207016ef2256eb8c5b) )     /* sprites */
	ROM_LOAD( "tb_16.3l",  0x08000, 0x8000, CRC(d86f8cbd) SHA1(8a16130632e20ad3cae8e817da7b661c3ac60f30) )
	ROM_LOAD( "tb_17.5l",  0x10000, 0x8000, CRC(12a73b3f) SHA1(6bb54d4fdf01fd2cdd76a0b47be4d8cae8a1e19b) )
	ROM_LOAD( "tb_15.2l",  0x18000, 0x8000, CRC(bb1a2769) SHA1(9884dceb00e6d88908a1c107b83cc1711b0cf1f7) )
	ROM_LOAD( "tb_22.7n",  0x20000, 0x8000, CRC(39daafd4) SHA1(1e49a273f51cccec3141d540032fd9a3041a3cbd) )
	ROM_LOAD( "tb_20.3n",  0x28000, 0x8000, CRC(94615d2a) SHA1(112a299ff1bb878cf7e24c2ad337440c3df0a6d5) )
	ROM_LOAD( "tb_21.5n",  0x30000, 0x8000, CRC(66c642bd) SHA1(b57f0f8d8e21c9f94ffc0e9f9304b5ab5d4ed3fc) )
	ROM_LOAD( "tb_19.2n",  0x38000, 0x8000, CRC(81d5ab36) SHA1(31103759676a8d1badaf7bde79e7f28d69486106) )

	ROM_REGION( 0x10000, "gfx4", 0 )
	ROM_LOAD( "tb_25.15n", 0x00000, 0x8000, CRC(6e38c6fa) SHA1(c51228d5d063dcf4361c76fa49dbe18db80c50a0) )     /* Bk Tiles */
	ROM_LOAD( "tb_24.13n", 0x08000, 0x8000, CRC(14fc6cf2) SHA1(080a2d845cb36c637f76d8e062725bd13dd1aed0) )

	ROM_REGION( 0x08000, "gfx5", 0 )
	ROM_LOAD( "tb_23.9n",  0x00000, 0x08000, CRC(eda13c0e) SHA1(806f0819af8b25c2b46de3d1fd95bc9c0e883bd9) )   /* Tile Map */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "tbb-2.7j",  0x0000, 0x0100, CRC(d96bcc98) SHA1(99e69a624d5586e5eedacd2083fa68b36e7b5e40) ) /* timing (not used) */
	ROM_LOAD( "tbb-1.1e",  0x0100, 0x0100, CRC(5052fa9d) SHA1(8cd240f4795a7ae76499573c09069dba37182be2) ) /* priority (not used) */
ROM_END

ROM_START( trojanlt ) // titled Trojan but only shows Capcom like the Japanese version, instead of Capcom USA like the other US sets
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code + 3*16k for the banked ROMs images */
	ROM_LOAD( "tb_04.10n", 0x00000, 0x8000, CRC(52a4f8a1) SHA1(169097096189bc9ee54e1504cd16256adbd447f9) ) // SLDH
	ROM_LOAD( "tb_06.13n", 0x10000, 0x8000, CRC(ef182e53) SHA1(20e65763ca18b1431e903dd41716a157ef1be28a) ) // SLDH
	ROM_LOAD( "tb_05.12n", 0x18000, 0x8000, CRC(9273b264) SHA1(ab23b16bf53b5baf106ea0cac50754aa967300cf) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "tb_02.15h", 0x0000, 0x8000, CRC(21154797) SHA1(e1a3006746cc2d692ecd4369cc0a77c596abd60b) )

	ROM_REGION( 0x10000, "adpcmcpu", 0 ) /* 64k for ADPCM CPU */
	ROM_LOAD( "tb01.3f", 0x0000, 0x8000, CRC(83c715b2) SHA1(0c69c086657f91828a639ff7c72c703a27ade710) ) // 1xxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "tb_03.8k",  0x00000, 0x4000, CRC(581a2b4c) SHA1(705b499da5d01a946f06234a4bab72a291c79034) )     /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "tb_13.6b",  0x00000, 0x8000, CRC(285a052b) SHA1(8ce055c7ac9ce1560552fc7f857f60e7a5af0779) )     /* tiles */
	ROM_LOAD( "tb_09.6a",  0x08000, 0x8000, CRC(aeb693f7) SHA1(a811ea67abdd4adfc68224257973802e2a36fc36) )
	ROM_LOAD( "tb_12.4b",  0x10000, 0x8000, CRC(dfb0fe5c) SHA1(82542692ab71b9126e6c301ed0803db58734273c) )
	ROM_LOAD( "tb_08.4a",  0x18000, 0x8000, CRC(d3a4c9d1) SHA1(3d787f6a4583b80f2d254947890f676cda17b242) )
	ROM_LOAD( "tb_11.3b",  0x20000, 0x8000, CRC(00f0f4fd) SHA1(3a862360a26ae1c3a945949d6d47f88aa4b728a4) )
	ROM_LOAD( "tb_07.3a",  0x28000, 0x8000, CRC(dff2ee02) SHA1(4877c52f2a0e24a95bcda1d8636ea993c2c3c240) )
	ROM_LOAD( "tb_14.8b",  0x30000, 0x8000, CRC(14bfac18) SHA1(84266140e9679912dbbb185fd3b9b497297dcb16) )
	ROM_LOAD( "tb_10.8a",  0x38000, 0x8000, CRC(71ba8a6d) SHA1(53ff6850f9f8a19c57c19ef56fd45975f0ec133e) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "tb_18.7l",  0x00000, 0x8000, CRC(862c4713) SHA1(a3707d950f4f5de5208e64207016ef2256eb8c5b) )     /* sprites */
	ROM_LOAD( "tb_16.3l",  0x08000, 0x8000, CRC(d86f8cbd) SHA1(8a16130632e20ad3cae8e817da7b661c3ac60f30) )
	ROM_LOAD( "tb_17.5l",  0x10000, 0x8000, CRC(12a73b3f) SHA1(6bb54d4fdf01fd2cdd76a0b47be4d8cae8a1e19b) )
	ROM_LOAD( "tb_15.2l",  0x18000, 0x8000, CRC(bb1a2769) SHA1(9884dceb00e6d88908a1c107b83cc1711b0cf1f7) )
	ROM_LOAD( "tb_22.7n",  0x20000, 0x8000, CRC(39daafd4) SHA1(1e49a273f51cccec3141d540032fd9a3041a3cbd) )
	ROM_LOAD( "tb_20.3n",  0x28000, 0x8000, CRC(94615d2a) SHA1(112a299ff1bb878cf7e24c2ad337440c3df0a6d5) )
	ROM_LOAD( "tb_21.5n",  0x30000, 0x8000, CRC(66c642bd) SHA1(b57f0f8d8e21c9f94ffc0e9f9304b5ab5d4ed3fc) )
	ROM_LOAD( "tb_19.2n",  0x38000, 0x8000, CRC(81d5ab36) SHA1(31103759676a8d1badaf7bde79e7f28d69486106) )

	ROM_REGION( 0x10000, "gfx4", 0 )
	ROM_LOAD( "tb_25.15n", 0x00000, 0x8000, CRC(6e38c6fa) SHA1(c51228d5d063dcf4361c76fa49dbe18db80c50a0) )     /* Bk Tiles */
	ROM_LOAD( "tb_24.13n", 0x08000, 0x8000, CRC(14fc6cf2) SHA1(080a2d845cb36c637f76d8e062725bd13dd1aed0) )

	ROM_REGION( 0x08000, "gfx5", 0 )
	ROM_LOAD( "tb_23.9n",  0x00000, 0x08000, CRC(eda13c0e) SHA1(806f0819af8b25c2b46de3d1fd95bc9c0e883bd9) )   /* Tile Map */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "tbb-2.7j",  0x0000, 0x0100, CRC(d96bcc98) SHA1(99e69a624d5586e5eedacd2083fa68b36e7b5e40) ) /* timing (not used) */
	ROM_LOAD( "tbb-1.1e",  0x0100, 0x0100, CRC(5052fa9d) SHA1(8cd240f4795a7ae76499573c09069dba37182be2) ) /* priority (not used) */
ROM_END

ROM_START( trojanb )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code + 3*16k for the banked ROMs images */
	ROM_LOAD( "4.11l", 0x00000, 0x8000, CRC(aad03bc7) SHA1(d889f0db3cf2c77d502442d27ff5d48bfbb854e2) ) // different
	ROM_LOAD( "6.11p", 0x10000, 0x8000, CRC(8ad19c83) SHA1(eff6f0052c891b6b0ff4af53067bc695c773f510) ) // different
	ROM_LOAD( "5.11m", 0x18000, 0x8000, CRC(9273b264) SHA1(ab23b16bf53b5baf106ea0cac50754aa967300cf) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "2.6q", 0x0000, 0x8000, CRC(21154797) SHA1(e1a3006746cc2d692ecd4369cc0a77c596abd60b) )

	ROM_REGION( 0x10000, "adpcmcpu", 0 ) /* 64k for ADPCM CPU */
	ROM_LOAD( "1.3f", 0x0000, 0x8000, CRC(83c715b2) SHA1(0c69c086657f91828a639ff7c72c703a27ade710) ) // different

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "3.8h", 0x00000, 0x4000, CRC(581a2b4c) SHA1(705b499da5d01a946f06234a4bab72a291c79034) )     /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "13.3e", 0x00000, 0x8000, CRC(285a052b) SHA1(8ce055c7ac9ce1560552fc7f857f60e7a5af0779) )     /* tiles */
	ROM_LOAD( "9.1e", 0x08000, 0x8000, CRC(aeb693f7) SHA1(a811ea67abdd4adfc68224257973802e2a36fc36) )
	ROM_LOAD( "12.3d", 0x10000, 0x8000, CRC(dfb0fe5c) SHA1(82542692ab71b9126e6c301ed0803db58734273c) )
	ROM_LOAD( "8.1d", 0x18000, 0x8000, CRC(d3a4c9d1) SHA1(3d787f6a4583b80f2d254947890f676cda17b242) )
	ROM_LOAD( "11.3b", 0x20000, 0x8000, CRC(00f0f4fd) SHA1(3a862360a26ae1c3a945949d6d47f88aa4b728a4) )
	ROM_LOAD( "7.1b", 0x28000, 0x8000, CRC(dff2ee02) SHA1(4877c52f2a0e24a95bcda1d8636ea993c2c3c240) )
	ROM_LOAD( "14.3g", 0x30000, 0x8000, CRC(14bfac18) SHA1(84266140e9679912dbbb185fd3b9b497297dcb16) )
	ROM_LOAD( "10.1g", 0x38000, 0x8000, CRC(71ba8a6d) SHA1(53ff6850f9f8a19c57c19ef56fd45975f0ec133e) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "18.10f", 0x00000, 0x8000, CRC(862c4713) SHA1(a3707d950f4f5de5208e64207016ef2256eb8c5b) )     /* sprites */
	ROM_LOAD( "16.10c", 0x08000, 0x8000, CRC(d86f8cbd) SHA1(8a16130632e20ad3cae8e817da7b661c3ac60f30) )
	ROM_LOAD( "17.10e", 0x10000, 0x8000, CRC(12a73b3f) SHA1(6bb54d4fdf01fd2cdd76a0b47be4d8cae8a1e19b) )
	ROM_LOAD( "15.10b", 0x18000, 0x8000, CRC(bb1a2769) SHA1(9884dceb00e6d88908a1c107b83cc1711b0cf1f7) )
	ROM_LOAD( "22.12f", 0x20000, 0x8000, CRC(39daafd4) SHA1(1e49a273f51cccec3141d540032fd9a3041a3cbd) )
	ROM_LOAD( "20.12c", 0x28000, 0x8000, CRC(94615d2a) SHA1(112a299ff1bb878cf7e24c2ad337440c3df0a6d5) )
	ROM_LOAD( "21.12e", 0x30000, 0x8000, CRC(66c642bd) SHA1(b57f0f8d8e21c9f94ffc0e9f9304b5ab5d4ed3fc) )
	ROM_LOAD( "19.12b", 0x38000, 0x8000, CRC(81d5ab36) SHA1(31103759676a8d1badaf7bde79e7f28d69486106) )

	ROM_REGION( 0x10000, "gfx4", 0 )
	ROM_LOAD( "25.12q", 0x00000, 0x8000, CRC(6e38c6fa) SHA1(c51228d5d063dcf4361c76fa49dbe18db80c50a0) )     /* Bk Tiles */
	ROM_LOAD( "24.12o", 0x08000, 0x8000, CRC(14fc6cf2) SHA1(080a2d845cb36c637f76d8e062725bd13dd1aed0) )

	ROM_REGION( 0x08000, "gfx5", 0 )
	ROM_LOAD( "23.12h", 0x00000, 0x08000, CRC(eda13c0e) SHA1(806f0819af8b25c2b46de3d1fd95bc9c0e883bd9) )   /* Tile Map (had a RED strip across label) */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s129.8g", 0x0000, 0x0100, CRC(d96bcc98) SHA1(99e69a624d5586e5eedacd2083fa68b36e7b5e40) ) /* timing (not used) */
	ROM_LOAD( "82s129.4a",  0x0100, 0x0100, CRC(5052fa9d) SHA1(8cd240f4795a7ae76499573c09069dba37182be2) ) /* priority (not used) */
ROM_END

/*

It was common for Capcom to use the same ROM label across regional sets but add a RED stripe for the US
  region, BLUE stripe for Europe and no stripe for the Japanese region. Capcom was not always consistent
  including the region letter stamped on labels. Different US PCBs show the red stripe across the label both
  with and without the "U" being stamped.

*/

// there is definitely at least one bad opcode in this dump, there could be others affecting enemy movement
#define AVENGERS_MCU \
	ROM_REGION( 0x1000, "mcu", 0 ) /* Intel C8751H - 88 */ \
	ROM_LOAD( "av.13k", 0x0000, 0x1000, BAD_DUMP CRC(505a0987) SHA1(ea1d855a9870d79d0e00eaa88a23038355a1203a) ) \
	ROM_FILL(0x0b84, 0x01, 0x02) /* bad code! bit 0x80 was flipped */

ROM_START( avengers )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code + 3*16k for the banked ROMs images */
	ROM_LOAD( "avu_04d.10n",  0x00000, 0x8000, CRC(a94aadcc) SHA1(796545ab5c69c093aaac58f7cff36109dea8df80) ) /* Red stripe across label for US region */
	ROM_LOAD( "avu_06d.13n",  0x10000, 0x8000, CRC(39cd80bd) SHA1(3f8df0096f393efae2d76982640ccc4d33bde8ca) ) /* Red stripe across label for US region */
	ROM_LOAD( "avu_05d.12n",  0x18000, 0x8000, CRC(06b1cec9) SHA1(db5370f3ff1b4456461698af64962cad028561cd) ) /* Red stripe across label for US region */

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "av_02.15h",    0x0000, 0x8000, CRC(107a2e17) SHA1(5aae2f4ac9f15ccb4122f3ba9fba588438d62f4f) ) /* ?? */

	ROM_REGION( 0x10000, "adpcmcpu", 0 )     /* ADPCM CPU */
	ROM_LOAD( "av_01.6d",     0x0000, 0x8000, CRC(c1e5d258) SHA1(88ed978e6df72ce22f9371930360aa9cde73abe9) ) /* adpcm player - "Talker" ROM */

	AVENGERS_MCU

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "av_03.8k",     0x00000, 0x8000, CRC(efb5883e) SHA1(08aebf579f2c5ff472db66597cde1c6871d7d757) )  /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "av_13.6b",     0x00000, 0x8000, CRC(9b5ff305) SHA1(8843c757e040b58efd36299eb3c56d9c51362b20) ) /* plane 1 */
	ROM_LOAD( "av_09.6a",     0x08000, 0x8000, CRC(08323355) SHA1(c5778c6835f2801fba0250cea21796ea201642f7) )
	ROM_LOAD( "av_12.4b",     0x10000, 0x8000, CRC(6d5261ba) SHA1(667e3b8df871c3052bde7a3c79daa7f70eaa0b8b) ) /* plane 2 */
	ROM_LOAD( "av_08.4a",     0x18000, 0x8000, CRC(a13d9f54) SHA1(e1bcb6d12cdfc9ad780f131272d12d9af751f429) )
	ROM_LOAD( "av_11.3b",     0x20000, 0x8000, CRC(a2911d8b) SHA1(f51ef7bb8a275fdd92a9a9ad516218d2f8c3e1fb) ) /* plane 3 */
	ROM_LOAD( "av_07.3a",     0x28000, 0x8000, CRC(cde78d32) SHA1(8cb69b7a25e935073887628565cb4f9787186ea9) )
	ROM_LOAD( "av_14.8b",     0x30000, 0x8000, CRC(44ac2671) SHA1(60baa541debd8aa7d32a512906d0d6c6e9955968) ) /* plane 4 */
	ROM_LOAD( "av_10.8a",     0x38000, 0x8000, CRC(b1a717cb) SHA1(2730764ece0e9231955b9c07de537f1f97729599) )

	ROM_REGION( 0x40000, "gfx3", 0 ) /* sprites */
	ROM_LOAD( "av_18.7l",     0x00000, 0x8000, CRC(3c876a17) SHA1(1f06b695b78a2e1db151f3c5baa1bb17ccef951e) ) /* planes 0,1 */
	ROM_LOAD( "av_16.3l",     0x08000, 0x8000, CRC(4b1ff3ac) SHA1(5166f2a2c9ba2483a4e340d756303cba46b7de88) )
	ROM_LOAD( "av_17.5l",     0x10000, 0x8000, CRC(4eb543ef) SHA1(5dfdd2568a50b179e724643880d79f79d831be19) )
	ROM_LOAD( "av_15.2l",     0x18000, 0x8000, CRC(8041de7f) SHA1(c301b20edad1981dd20cd6d4f7de703d9dc80b83) )
	ROM_LOAD( "av_22.7n",     0x20000, 0x8000, CRC(bdaa8b22) SHA1(9a03d20cc7010f9b7c602db86808d54fdd7e228d) ) /* planes 2,3 */
	ROM_LOAD( "av_20.3n",     0x28000, 0x8000, CRC(566e3059) SHA1(cf3e5cfcb5ebbff3f9a8e1da9f7242a7a00fee83) )
	ROM_LOAD( "av_21.5n",     0x30000, 0x8000, CRC(301059aa) SHA1(c529ad83d4e4139ce4d4d912c00aef9ece297706) )
	ROM_LOAD( "av_19.2n",     0x38000, 0x8000, CRC(a00485ec) SHA1(cc24e7243f55bdfaedeabb7dddf7e1ef32811c45) )

	ROM_REGION( 0x10000, "gfx4", 0 ) /* bg tiles */
	ROM_LOAD( "avu_25.15n",    0x00000, 0x8000, CRC(230d9e30) SHA1(05a20bb32ce1299d7645312624de8a1d074bacee) ) /* planes 0,1 - Red stripe across label for US region */
	ROM_LOAD( "avu_24.13n",    0x08000, 0x8000, CRC(a6354024) SHA1(ce2aaec8349c08f58cc469514100bcd3a97d24d7) ) /* planes 2,3 - Red stripe across label for US region */

	ROM_REGION( 0x08000, "gfx5", 0 )
	ROM_LOAD( "av_23.9n",     0x0000,  0x8000, CRC(c0a93ef6) SHA1(2dc9cd4eb142d74aea8d151904cb60a0767c6393) )  /* Tile Map */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "tbb_2bpr.7j",  0x0000,  0x0100, CRC(d96bcc98) SHA1(99e69a624d5586e5eedacd2083fa68b36e7b5e40) )   /* timing (not used) */
	ROM_LOAD( "tbb_1bpr.1e",  0x0100,  0x0100, CRC(5052fa9d) SHA1(8cd240f4795a7ae76499573c09069dba37182be2) )   /* priority (not used) */
ROM_END

ROM_START( avengersa )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code + 3*16k for the banked ROMs images */
	ROM_LOAD( "avu_04c.10n",  0x00000, 0x8000, CRC(4555b925) SHA1(49829272b23a39798bcaeb6d847a4091031b3dec) ) /* Red stripe across label for US region */
	ROM_LOAD( "avu_06c.13n",  0x10000, 0x8000, CRC(ea202879) SHA1(915eeea9b728613de59d53f2de313df88de23cbd) ) /* Red stripe across label for US region */
	ROM_LOAD( "av_05.12n",    0x18000, 0x8000, CRC(9a214b42) SHA1(e13d47dcf9fa055fef467a10751badffcc3b8734) ) /* No red stripe on this ROM */

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "av_02.15h",    0x0000, 0x8000, CRC(107a2e17) SHA1(5aae2f4ac9f15ccb4122f3ba9fba588438d62f4f) ) /* ?? */

	ROM_REGION( 0x10000, "adpcmcpu", 0 )     /* ADPCM CPU */
	ROM_LOAD( "av_01.6d",     0x0000, 0x8000, CRC(c1e5d258) SHA1(88ed978e6df72ce22f9371930360aa9cde73abe9) ) /* adpcm player - "Talker" ROM */

	AVENGERS_MCU

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "av_03.8k",     0x00000, 0x8000, CRC(efb5883e) SHA1(08aebf579f2c5ff472db66597cde1c6871d7d757) )  /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "av_13.6b",     0x00000, 0x8000, CRC(9b5ff305) SHA1(8843c757e040b58efd36299eb3c56d9c51362b20) ) /* plane 1 */
	ROM_LOAD( "av_09.6a",     0x08000, 0x8000, CRC(08323355) SHA1(c5778c6835f2801fba0250cea21796ea201642f7) )
	ROM_LOAD( "av_12.4b",     0x10000, 0x8000, CRC(6d5261ba) SHA1(667e3b8df871c3052bde7a3c79daa7f70eaa0b8b) ) /* plane 2 */
	ROM_LOAD( "av_08.4a",     0x18000, 0x8000, CRC(a13d9f54) SHA1(e1bcb6d12cdfc9ad780f131272d12d9af751f429) )
	ROM_LOAD( "av_11.3b",     0x20000, 0x8000, CRC(a2911d8b) SHA1(f51ef7bb8a275fdd92a9a9ad516218d2f8c3e1fb) ) /* plane 3 */
	ROM_LOAD( "av_07.3a",     0x28000, 0x8000, CRC(cde78d32) SHA1(8cb69b7a25e935073887628565cb4f9787186ea9) )
	ROM_LOAD( "av_14.8b",     0x30000, 0x8000, CRC(44ac2671) SHA1(60baa541debd8aa7d32a512906d0d6c6e9955968) ) /* plane 4 */
	ROM_LOAD( "av_10.8a",     0x38000, 0x8000, CRC(b1a717cb) SHA1(2730764ece0e9231955b9c07de537f1f97729599) )

	ROM_REGION( 0x40000, "gfx3", 0 ) /* sprites */
	ROM_LOAD( "av_18.7l",     0x00000, 0x8000, CRC(3c876a17) SHA1(1f06b695b78a2e1db151f3c5baa1bb17ccef951e) ) /* planes 0,1 */
	ROM_LOAD( "av_16.3l",     0x08000, 0x8000, CRC(4b1ff3ac) SHA1(5166f2a2c9ba2483a4e340d756303cba46b7de88) )
	ROM_LOAD( "av_17.5l",     0x10000, 0x8000, CRC(4eb543ef) SHA1(5dfdd2568a50b179e724643880d79f79d831be19) )
	ROM_LOAD( "av_15.2l",     0x18000, 0x8000, CRC(8041de7f) SHA1(c301b20edad1981dd20cd6d4f7de703d9dc80b83) )
	ROM_LOAD( "av_22.7n",     0x20000, 0x8000, CRC(bdaa8b22) SHA1(9a03d20cc7010f9b7c602db86808d54fdd7e228d) ) /* planes 2,3 */
	ROM_LOAD( "av_20.3n",     0x28000, 0x8000, CRC(566e3059) SHA1(cf3e5cfcb5ebbff3f9a8e1da9f7242a7a00fee83) )
	ROM_LOAD( "av_21.5n",     0x30000, 0x8000, CRC(301059aa) SHA1(c529ad83d4e4139ce4d4d912c00aef9ece297706) )
	ROM_LOAD( "av_19.2n",     0x38000, 0x8000, CRC(a00485ec) SHA1(cc24e7243f55bdfaedeabb7dddf7e1ef32811c45) )

	ROM_REGION( 0x10000, "gfx4", 0 ) /* bg tiles */
	ROM_LOAD( "avu_25.15n",    0x00000, 0x8000, CRC(230d9e30) SHA1(05a20bb32ce1299d7645312624de8a1d074bacee) ) /* planes 0,1 - Red stripe across label for US region */
	ROM_LOAD( "avu_24.13n",    0x08000, 0x8000, CRC(a6354024) SHA1(ce2aaec8349c08f58cc469514100bcd3a97d24d7) ) /* planes 2,3 - Red stripe across label for US region */

	ROM_REGION( 0x08000, "gfx5", 0 )
	ROM_LOAD( "av_23.9n",     0x0000,  0x8000, CRC(c0a93ef6) SHA1(2dc9cd4eb142d74aea8d151904cb60a0767c6393) )  /* Tile Map */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "tbb_2bpr.7j",  0x0000,  0x0100, CRC(d96bcc98) SHA1(99e69a624d5586e5eedacd2083fa68b36e7b5e40) )   /* timing (not used) */
	ROM_LOAD( "tbb_1bpr.1e",  0x0100,  0x0100, CRC(5052fa9d) SHA1(8cd240f4795a7ae76499573c09069dba37182be2) )   /* priority (not used) */
ROM_END

ROM_START( avengersb )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code + 3*16k for the banked ROMs images */
	ROM_LOAD( "av_04a.10n",   0x00000, 0x8000, CRC(0fea7ac5) SHA1(b978adf5fc90e1e51a995dbec2246d2776264afd) ) /* Red stripe across label for US region */
	ROM_LOAD( "av_06a.13n",   0x10000, 0x8000, CRC(491a712c) SHA1(67a335b57117ba498d3ae412ac0025477bc79b16) ) /* Red stripe across label for US region */
	ROM_LOAD( "av_05.12n",    0x18000, 0x8000, CRC(9a214b42) SHA1(e13d47dcf9fa055fef467a10751badffcc3b8734) ) /* No red stripe on this ROM */

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "av_02.15h",    0x0000, 0x8000, CRC(107a2e17) SHA1(5aae2f4ac9f15ccb4122f3ba9fba588438d62f4f) ) /* ?? */

	ROM_REGION( 0x10000, "adpcmcpu", 0 )     /* ADPCM CPU */
	ROM_LOAD( "av_01.6d",     0x0000, 0x8000, CRC(c1e5d258) SHA1(88ed978e6df72ce22f9371930360aa9cde73abe9) ) /* adpcm player - "Talker" ROM */

	AVENGERS_MCU

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "av_03.8k",     0x00000, 0x8000, CRC(efb5883e) SHA1(08aebf579f2c5ff472db66597cde1c6871d7d757) )  /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "av_13.6b",     0x00000, 0x8000, CRC(9b5ff305) SHA1(8843c757e040b58efd36299eb3c56d9c51362b20) ) /* plane 1 */
	ROM_LOAD( "av_09.6a",     0x08000, 0x8000, CRC(08323355) SHA1(c5778c6835f2801fba0250cea21796ea201642f7) )
	ROM_LOAD( "av_12.4b",     0x10000, 0x8000, CRC(6d5261ba) SHA1(667e3b8df871c3052bde7a3c79daa7f70eaa0b8b) ) /* plane 2 */
	ROM_LOAD( "av_08.4a",     0x18000, 0x8000, CRC(a13d9f54) SHA1(e1bcb6d12cdfc9ad780f131272d12d9af751f429) )
	ROM_LOAD( "av_11.3b",     0x20000, 0x8000, CRC(a2911d8b) SHA1(f51ef7bb8a275fdd92a9a9ad516218d2f8c3e1fb) ) /* plane 3 */
	ROM_LOAD( "av_07.3a",     0x28000, 0x8000, CRC(cde78d32) SHA1(8cb69b7a25e935073887628565cb4f9787186ea9) )
	ROM_LOAD( "av_14.8b",     0x30000, 0x8000, CRC(44ac2671) SHA1(60baa541debd8aa7d32a512906d0d6c6e9955968) ) /* plane 4 */
	ROM_LOAD( "av_10.8a",     0x38000, 0x8000, CRC(b1a717cb) SHA1(2730764ece0e9231955b9c07de537f1f97729599) )

	ROM_REGION( 0x40000, "gfx3", 0 ) /* sprites */
	ROM_LOAD( "av_18.7l",     0x00000, 0x8000, CRC(3c876a17) SHA1(1f06b695b78a2e1db151f3c5baa1bb17ccef951e) ) /* planes 0,1 */
	ROM_LOAD( "av_16.3l",     0x08000, 0x8000, CRC(4b1ff3ac) SHA1(5166f2a2c9ba2483a4e340d756303cba46b7de88) )
	ROM_LOAD( "av_17.5l",     0x10000, 0x8000, CRC(4eb543ef) SHA1(5dfdd2568a50b179e724643880d79f79d831be19) )
	ROM_LOAD( "av_15.2l",     0x18000, 0x8000, CRC(8041de7f) SHA1(c301b20edad1981dd20cd6d4f7de703d9dc80b83) )
	ROM_LOAD( "av_22.7n",     0x20000, 0x8000, CRC(bdaa8b22) SHA1(9a03d20cc7010f9b7c602db86808d54fdd7e228d) ) /* planes 2,3 */
	ROM_LOAD( "av_20.3n",     0x28000, 0x8000, CRC(566e3059) SHA1(cf3e5cfcb5ebbff3f9a8e1da9f7242a7a00fee83) )
	ROM_LOAD( "av_21.5n",     0x30000, 0x8000, CRC(301059aa) SHA1(c529ad83d4e4139ce4d4d912c00aef9ece297706) )
	ROM_LOAD( "av_19.2n",     0x38000, 0x8000, CRC(a00485ec) SHA1(cc24e7243f55bdfaedeabb7dddf7e1ef32811c45) )

	ROM_REGION( 0x10000, "gfx4", 0 ) /* bg tiles */
	ROM_LOAD( "avu_25.15n",    0x00000, 0x8000, CRC(230d9e30) SHA1(05a20bb32ce1299d7645312624de8a1d074bacee) ) /* planes 0,1 - Red stripe across label for US region */
	ROM_LOAD( "avu_24.13n",    0x08000, 0x8000, CRC(a6354024) SHA1(ce2aaec8349c08f58cc469514100bcd3a97d24d7) ) /* planes 2,3 - Red stripe across label for US region */

	ROM_REGION( 0x08000, "gfx5", 0 )
	ROM_LOAD( "av_23.9n",     0x0000,  0x8000, CRC(c0a93ef6) SHA1(2dc9cd4eb142d74aea8d151904cb60a0767c6393) )  /* Tile Map */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "tbb_2bpr.7j",  0x0000,  0x0100, CRC(d96bcc98) SHA1(99e69a624d5586e5eedacd2083fa68b36e7b5e40) )   /* timing (not used) */
	ROM_LOAD( "tbb_1bpr.1e",  0x0100,  0x0100, CRC(5052fa9d) SHA1(8cd240f4795a7ae76499573c09069dba37182be2) )   /* priority (not used) */
ROM_END

ROM_START( avengersc )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code + 3*16k for the banked ROMs images */
	ROM_LOAD( "av_04.10n",    0x00000, 0x8000, CRC(c785e1f2) SHA1(a84610e82d7afefafdb457e4c361d657c313d378) ) /* Red stripe across label for US region */
	ROM_LOAD( "av_06.13n",    0x10000, 0x8000, CRC(c6f84a5f) SHA1(33e97c11eb3520fb91e6523f608a3b9f0f338a7c) ) /* Red stripe across label for US region */
	ROM_LOAD( "av_05.12n",    0x18000, 0x8000, CRC(f9a9a92f) SHA1(3f5b3bee702d3324b2a2274c1727e1a30196ae68) ) /* sldh - Red stripe across label for US region */

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "av_02.15h",    0x0000, 0x8000, CRC(107a2e17) SHA1(5aae2f4ac9f15ccb4122f3ba9fba588438d62f4f) ) /* ?? */

	ROM_REGION( 0x10000, "adpcmcpu", 0 )     /* ADPCM CPU */
	ROM_LOAD( "av_01.6d",     0x0000, 0x8000, CRC(c1e5d258) SHA1(88ed978e6df72ce22f9371930360aa9cde73abe9) ) /* adpcm player - "Talker" ROM */

	AVENGERS_MCU

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "av_03.8k",     0x00000, 0x8000, CRC(efb5883e) SHA1(08aebf579f2c5ff472db66597cde1c6871d7d757) )  /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "av_13.6b",     0x00000, 0x8000, CRC(9b5ff305) SHA1(8843c757e040b58efd36299eb3c56d9c51362b20) ) /* plane 1 */
	ROM_LOAD( "av_09.6a",     0x08000, 0x8000, CRC(08323355) SHA1(c5778c6835f2801fba0250cea21796ea201642f7) )
	ROM_LOAD( "av_12.4b",     0x10000, 0x8000, CRC(6d5261ba) SHA1(667e3b8df871c3052bde7a3c79daa7f70eaa0b8b) ) /* plane 2 */
	ROM_LOAD( "av_08.4a",     0x18000, 0x8000, CRC(a13d9f54) SHA1(e1bcb6d12cdfc9ad780f131272d12d9af751f429) )
	ROM_LOAD( "av_11.3b",     0x20000, 0x8000, CRC(a2911d8b) SHA1(f51ef7bb8a275fdd92a9a9ad516218d2f8c3e1fb) ) /* plane 3 */
	ROM_LOAD( "av_07.3a",     0x28000, 0x8000, CRC(cde78d32) SHA1(8cb69b7a25e935073887628565cb4f9787186ea9) )
	ROM_LOAD( "av_14.8b",     0x30000, 0x8000, CRC(44ac2671) SHA1(60baa541debd8aa7d32a512906d0d6c6e9955968) ) /* plane 4 */
	ROM_LOAD( "av_10.8a",     0x38000, 0x8000, CRC(b1a717cb) SHA1(2730764ece0e9231955b9c07de537f1f97729599) )

	ROM_REGION( 0x40000, "gfx3", 0 ) /* sprites */
	ROM_LOAD( "av_18.7l",     0x00000, 0x8000, CRC(3c876a17) SHA1(1f06b695b78a2e1db151f3c5baa1bb17ccef951e) ) /* planes 0,1 */
	ROM_LOAD( "av_16.3l",     0x08000, 0x8000, CRC(4b1ff3ac) SHA1(5166f2a2c9ba2483a4e340d756303cba46b7de88) )
	ROM_LOAD( "av_17.5l",     0x10000, 0x8000, CRC(4eb543ef) SHA1(5dfdd2568a50b179e724643880d79f79d831be19) )
	ROM_LOAD( "av_15.2l",     0x18000, 0x8000, CRC(8041de7f) SHA1(c301b20edad1981dd20cd6d4f7de703d9dc80b83) )
	ROM_LOAD( "av_22.7n",     0x20000, 0x8000, CRC(bdaa8b22) SHA1(9a03d20cc7010f9b7c602db86808d54fdd7e228d) ) /* planes 2,3 */
	ROM_LOAD( "av_20.3n",     0x28000, 0x8000, CRC(566e3059) SHA1(cf3e5cfcb5ebbff3f9a8e1da9f7242a7a00fee83) )
	ROM_LOAD( "av_21.5n",     0x30000, 0x8000, CRC(301059aa) SHA1(c529ad83d4e4139ce4d4d912c00aef9ece297706) )
	ROM_LOAD( "av_19.2n",     0x38000, 0x8000, CRC(a00485ec) SHA1(cc24e7243f55bdfaedeabb7dddf7e1ef32811c45) )

	ROM_REGION( 0x10000, "gfx4", 0 ) /* bg tiles */
	ROM_LOAD( "avu_25.15n",    0x00000, 0x8000, CRC(230d9e30) SHA1(05a20bb32ce1299d7645312624de8a1d074bacee) ) /* planes 0,1 - Red stripe across label for US region */
	ROM_LOAD( "avu_24.13n",    0x08000, 0x8000, CRC(a6354024) SHA1(ce2aaec8349c08f58cc469514100bcd3a97d24d7) ) /* planes 2,3 - Red stripe across label for US region */

	ROM_REGION( 0x08000, "gfx5", 0 )
	ROM_LOAD( "av_23.9n",     0x0000,  0x8000, CRC(c0a93ef6) SHA1(2dc9cd4eb142d74aea8d151904cb60a0767c6393) )  /* Tile Map */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "tbb_2bpr.7j",  0x0000,  0x0100, CRC(d96bcc98) SHA1(99e69a624d5586e5eedacd2083fa68b36e7b5e40) )   /* timing (not used) */
	ROM_LOAD( "tbb_1bpr.1e",  0x0100,  0x0100, CRC(5052fa9d) SHA1(8cd240f4795a7ae76499573c09069dba37182be2) )   /* priority (not used) */
ROM_END

ROM_START( buraiken )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code + 3*16k for the banked ROMs images */
	ROM_LOAD( "av_04a.10n",   0x00000, 0x8000, CRC(361fc614) SHA1(0ecd9400dfcb03fc94685b33b060a524a5d3c575) ) /* sldh (no red stripe) */
	ROM_LOAD( "av_06a.13n",   0x10000, 0x8000, CRC(491a712c) SHA1(67a335b57117ba498d3ae412ac0025477bc79b16) ) /* sldh (no red stripe) */
	ROM_LOAD( "av_05.12n",    0x18000, 0x8000, CRC(9a214b42) SHA1(e13d47dcf9fa055fef467a10751badffcc3b8734) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "av_02.15h",    0x0000, 0x8000, CRC(107a2e17) SHA1(5aae2f4ac9f15ccb4122f3ba9fba588438d62f4f) ) /* ?? */

	ROM_REGION( 0x10000, "adpcmcpu", 0 )     /* ADPCM CPU */
	ROM_LOAD( "av_01.6d",     0x0000, 0x8000, CRC(c1e5d258) SHA1(88ed978e6df72ce22f9371930360aa9cde73abe9) ) /* adpcm player - "Talker" ROM */

	AVENGERS_MCU

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "av_03.8k",     0x00000, 0x8000, CRC(efb5883e) SHA1(08aebf579f2c5ff472db66597cde1c6871d7d757) )  /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "av_13.6b",     0x00000, 0x8000, CRC(9b5ff305) SHA1(8843c757e040b58efd36299eb3c56d9c51362b20) ) /* plane 1 */
	ROM_LOAD( "av_09.6a",     0x08000, 0x8000, CRC(08323355) SHA1(c5778c6835f2801fba0250cea21796ea201642f7) )
	ROM_LOAD( "av_12.4b",     0x10000, 0x8000, CRC(6d5261ba) SHA1(667e3b8df871c3052bde7a3c79daa7f70eaa0b8b) ) /* plane 2 */
	ROM_LOAD( "av_08.4a",     0x18000, 0x8000, CRC(a13d9f54) SHA1(e1bcb6d12cdfc9ad780f131272d12d9af751f429) )
	ROM_LOAD( "av_11.3b",     0x20000, 0x8000, CRC(a2911d8b) SHA1(f51ef7bb8a275fdd92a9a9ad516218d2f8c3e1fb) ) /* plane 3 */
	ROM_LOAD( "av_07.3a",     0x28000, 0x8000, CRC(cde78d32) SHA1(8cb69b7a25e935073887628565cb4f9787186ea9) )
	ROM_LOAD( "av_14.8b",     0x30000, 0x8000, CRC(44ac2671) SHA1(60baa541debd8aa7d32a512906d0d6c6e9955968) ) /* plane 4 */
	ROM_LOAD( "av_10.8a",     0x38000, 0x8000, CRC(b1a717cb) SHA1(2730764ece0e9231955b9c07de537f1f97729599) )

	ROM_REGION( 0x40000, "gfx3", 0 ) /* sprites */
	ROM_LOAD( "av_18.7l",     0x00000, 0x8000, CRC(3c876a17) SHA1(1f06b695b78a2e1db151f3c5baa1bb17ccef951e) ) /* planes 0,1 */
	ROM_LOAD( "av_16.3l",     0x08000, 0x8000, CRC(4b1ff3ac) SHA1(5166f2a2c9ba2483a4e340d756303cba46b7de88) )
	ROM_LOAD( "av_17.5l",     0x10000, 0x8000, CRC(4eb543ef) SHA1(5dfdd2568a50b179e724643880d79f79d831be19) )
	ROM_LOAD( "av_15.2l",     0x18000, 0x8000, CRC(8041de7f) SHA1(c301b20edad1981dd20cd6d4f7de703d9dc80b83) )
	ROM_LOAD( "av_22.7n",     0x20000, 0x8000, CRC(bdaa8b22) SHA1(9a03d20cc7010f9b7c602db86808d54fdd7e228d) ) /* planes 2,3 */
	ROM_LOAD( "av_20.3n",     0x28000, 0x8000, CRC(566e3059) SHA1(cf3e5cfcb5ebbff3f9a8e1da9f7242a7a00fee83) )
	ROM_LOAD( "av_21.5n",     0x30000, 0x8000, CRC(301059aa) SHA1(c529ad83d4e4139ce4d4d912c00aef9ece297706) )
	ROM_LOAD( "av_19.2n",     0x38000, 0x8000, CRC(a00485ec) SHA1(cc24e7243f55bdfaedeabb7dddf7e1ef32811c45) )

	ROM_REGION( 0x10000, "gfx4", 0 )
	ROM_LOAD( "av_25.15n",    0x00000, 0x8000, CRC(88a505a7) SHA1(ef4371e082b2370fcbfc96bfef5a94910acd9eff) ) /* planes 0,1 - no stripe across the label */
	ROM_LOAD( "av_24.13n",    0x08000, 0x8000, CRC(1f4463c8) SHA1(04cdb0187dcbdd4f5f53e60c856d4925ade8d7df) ) /* planes 2,3 - no stripe across the label */

	ROM_REGION( 0x08000, "gfx5", 0 )
	ROM_LOAD( "av_23.9n",     0x0000,  0x8000, CRC(c0a93ef6) SHA1(2dc9cd4eb142d74aea8d151904cb60a0767c6393) )  /* Tile Map */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "tbb_2bpr.7j",  0x0000,  0x0100, CRC(d96bcc98) SHA1(99e69a624d5586e5eedacd2083fa68b36e7b5e40) )   /* timing (not used) */
	ROM_LOAD( "tbb_1bpr.1e",  0x0100,  0x0100, CRC(5052fa9d) SHA1(8cd240f4795a7ae76499573c09069dba37182be2) )   /* priority (not used) */
ROM_END

ROM_START( buraikenb )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* 64k for code + 3*16k for the banked ROMs images */
	ROM_LOAD( "a4",        0x00000, 0x8000, CRC(b4ac7928) SHA1(4a525532f634dd9e800dc3dbd1230a5c431f869a) )
	ROM_LOAD( "a6",        0x10000, 0x8000, CRC(b1c6d40d) SHA1(d150adace829130ebf99b8beeedde0e673124984) )
	ROM_LOAD( "av_05.12n", 0x18000, 0x8000, CRC(9a214b42) SHA1(e13d47dcf9fa055fef467a10751badffcc3b8734) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "a2",       0x0000, 0x8000, CRC(5e991c96) SHA1(1866f38043f61244b65213544fa5ec5d6d82f96f) )

	ROM_REGION( 0x10000, "adpcmcpu", 0 )     /* ADPCM CPU */
	ROM_LOAD( "av_01.6d",     0x0000, 0x8000, CRC(c1e5d258) SHA1(88ed978e6df72ce22f9371930360aa9cde73abe9) ) /* adpcm player - "Talker" ROM */

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "av_03.8k",     0x00000, 0x8000, CRC(efb5883e) SHA1(08aebf579f2c5ff472db66597cde1c6871d7d757) )  /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "av_13.6b",     0x00000, 0x8000, CRC(9b5ff305) SHA1(8843c757e040b58efd36299eb3c56d9c51362b20) ) /* plane 1 */
	ROM_LOAD( "av_09.6a",     0x08000, 0x8000, CRC(08323355) SHA1(c5778c6835f2801fba0250cea21796ea201642f7) )
	ROM_LOAD( "av_12.4b",     0x10000, 0x8000, CRC(6d5261ba) SHA1(667e3b8df871c3052bde7a3c79daa7f70eaa0b8b) ) /* plane 2 */
	ROM_LOAD( "av_08.4a",     0x18000, 0x8000, CRC(a13d9f54) SHA1(e1bcb6d12cdfc9ad780f131272d12d9af751f429) )
	ROM_LOAD( "av_11.3b",     0x20000, 0x8000, CRC(a2911d8b) SHA1(f51ef7bb8a275fdd92a9a9ad516218d2f8c3e1fb) ) /* plane 3 */
	ROM_LOAD( "av_07.3a",     0x28000, 0x8000, CRC(cde78d32) SHA1(8cb69b7a25e935073887628565cb4f9787186ea9) )
	ROM_LOAD( "av_14.8b",     0x30000, 0x8000, CRC(44ac2671) SHA1(60baa541debd8aa7d32a512906d0d6c6e9955968) ) /* plane 4 */
	ROM_LOAD( "av_10.8a",     0x38000, 0x8000, CRC(b1a717cb) SHA1(2730764ece0e9231955b9c07de537f1f97729599) )

	ROM_REGION( 0x40000, "gfx3", 0 ) /* sprites */
	ROM_LOAD( "av_18.7l",     0x00000, 0x8000, CRC(3c876a17) SHA1(1f06b695b78a2e1db151f3c5baa1bb17ccef951e) ) /* planes 0,1 */
	ROM_LOAD( "av_16.3l",     0x08000, 0x8000, CRC(4b1ff3ac) SHA1(5166f2a2c9ba2483a4e340d756303cba46b7de88) )
	ROM_LOAD( "av_17.5l",     0x10000, 0x8000, CRC(4eb543ef) SHA1(5dfdd2568a50b179e724643880d79f79d831be19) )
	ROM_LOAD( "av_15.2l",     0x18000, 0x8000, CRC(8041de7f) SHA1(c301b20edad1981dd20cd6d4f7de703d9dc80b83) )
	ROM_LOAD( "av_22.7n",     0x20000, 0x8000, CRC(bdaa8b22) SHA1(9a03d20cc7010f9b7c602db86808d54fdd7e228d) ) /* planes 2,3 */
	ROM_LOAD( "av_20.3n",     0x28000, 0x8000, CRC(566e3059) SHA1(cf3e5cfcb5ebbff3f9a8e1da9f7242a7a00fee83) )
	ROM_LOAD( "av_21.5n",     0x30000, 0x8000, CRC(301059aa) SHA1(c529ad83d4e4139ce4d4d912c00aef9ece297706) )
	ROM_LOAD( "av_19.2n",     0x38000, 0x8000, CRC(a00485ec) SHA1(cc24e7243f55bdfaedeabb7dddf7e1ef32811c45) )

	ROM_REGION( 0x10000, "gfx4", 0 )
	ROM_LOAD( "av_25.15n",    0x00000, 0x8000, CRC(88a505a7) SHA1(ef4371e082b2370fcbfc96bfef5a94910acd9eff) ) /* planes 0,1 - no stripe across the label */
	ROM_LOAD( "av_24.13n",    0x08000, 0x8000, CRC(1f4463c8) SHA1(04cdb0187dcbdd4f5f53e60c856d4925ade8d7df) ) /* planes 2,3 - no stripe across the label */

	ROM_REGION( 0x08000, "gfx5", 0 )
	ROM_LOAD( "av_23.9n",     0x0000,  0x8000, CRC(c0a93ef6) SHA1(2dc9cd4eb142d74aea8d151904cb60a0767c6393) )  /* Tile Map */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "tbb_2bpr.7j",  0x0000,  0x0100, CRC(d96bcc98) SHA1(99e69a624d5586e5eedacd2083fa68b36e7b5e40) )   /* timing (not used) */
	ROM_LOAD( "tbb_1bpr.1e",  0x0100,  0x0100, CRC(5052fa9d) SHA1(8cd240f4795a7ae76499573c09069dba37182be2) )   /* priority (not used) */
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1985, sectionz,  0,        sectionz,  sectionz, lwings_state, empty_init, ROT0,  "Capcom", "Section Z (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, sectionza, sectionz, sectionz,  sectionz, lwings_state, empty_init, ROT0,  "Capcom", "Section Z (Japan, rev. A)", MACHINE_SUPPORTS_SAVE )

GAME( 1986, lwings,    0,        lwings,    lwings,   lwings_state, empty_init, ROT90, "Capcom", "Legendary Wings (US, rev. C)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, lwingsa,   lwings,   lwings,    lwings,   lwings_state, empty_init, ROT90, "Capcom", "Legendary Wings (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, lwingsj,   lwings,   lwings,    lwings,   lwings_state, empty_init, ROT90, "Capcom", "Ares no Tsubasa (Japan, rev. B)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, lwingsja,  lwings,   lwings,    lwings,   lwings_state, empty_init, ROT90, "Capcom", "Ares no Tsubasa (Japan, rev. A)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, lwingsb,   lwings,   lwings,    lwingsb,  lwings_state, empty_init, ROT90, "bootleg", "Legendary Wings (bootleg)", MACHINE_SUPPORTS_SAVE )

GAME( 1986, trojan,    0,        trojan,    trojanls, lwings_state, empty_init, ROT0,  "Capcom", "Trojan (US set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, trojana,   trojan,   trojan,    trojan,   lwings_state, empty_init, ROT0,  "Capcom", "Trojan (US set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, trojanr,   trojan,   trojan,    trojan,   lwings_state, empty_init, ROT0,  "Capcom (Romstar license)", "Trojan (Romstar, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, trojanra,  trojan,   trojan,    trojan,   lwings_state, empty_init, ROT0,  "Capcom (Romstar license)", "Trojan (Romstar, set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, trojanj,   trojan,   trojan,    trojan,   lwings_state, empty_init, ROT0,  "Capcom", "Tatakai no Banka (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, trojanjo,  trojan,   trojan,    trojan,   lwings_state, empty_init, ROT0,  "Capcom", "Tatakai no Banka (Japan, old ver.)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, trojanb,   trojan,   trojan,    trojan,   lwings_state, empty_init, ROT0,  "bootleg", "Trojan (bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, trojanlt,  trojan,   trojan,    trojan,   lwings_state, empty_init, ROT0,  "Capcom", "Trojan (location test)", MACHINE_SUPPORTS_SAVE )

GAME( 1987, avengers,  0,        avengers,  avengers, lwings_state, empty_init, ROT90, "Capcom", "Avengers (US, rev. D)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, avengersa, avengers, avengers,  avengers, lwings_state, empty_init, ROT90, "Capcom", "Avengers (US, rev. C)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, avengersb, avengers, avengers,  avengers, lwings_state, empty_init, ROT90, "Capcom", "Avengers (US, rev. A)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, avengersc, avengers, avengers,  avengers, lwings_state, empty_init, ROT90, "Capcom", "Avengers (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, buraiken,  avengers, avengers,  avengers, lwings_state, empty_init, ROT90, "Capcom", "Hissatsu Buraiken (Japan, rev. A)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, buraikenb, avengers, buraikenb, avengers, lwings_state, empty_init, ROT90, "bootleg", "Hissatsu Buraiken (Japan, bootleg)", MACHINE_SUPPORTS_SAVE )

// cloned lwings hardware
GAME( 1992, fball,     0,        fball,     fball,    lwings_state, empty_init, ROT0,  "FM Work", "Fire Ball (FM Work)", MACHINE_SUPPORTS_SAVE )
