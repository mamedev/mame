// license:BSD-3-Clause
// copyright-holders:R. Belmont, Angelo Salese
/***************************************************************************

Centipede / Millipede / Missile Command / Let's Go Bowling
(c) 1980-2 / 2002 - Infogrames / Cosmodog

Earlier revisions of this cabinet did not include the bowling game.
 Known to exist "CMM Rev 1.03" (without Let's Go Bowling)
 Let's Go Bowling is actually a completely new game by Cosmodog, not
 a port or prototype of an old Atari game.

Hardware notes
--------------
U1  = WDC 65C02S8P-14
U2  = 256KB flash (rev-dependent type; see Flash / NVRAM below)
U4  = ISSI IS62LV256-45J (32KB)
U5  = CY39100V208B (Cypress CPLD)  -- per-game address decode / video
U9  = CY37128-P100 (Cypress CPLD)
U10 = CYC1399 (?)

OSC @ 72.576MHz

From Cosmodog's website (re: the CY39100):
 "Instead, we used a programmable chip that we could reconfigure very
 quickly while the game is running. So, during that 1/8th of a second
 or so when the screen goes black while it switches games, it's actually
 reloading the hardware with a whole new design to run the next game."

The 256KB flash is four 64KB slots.  Each slot starts with a CY39100
bitstream for that game's hardware, followed by the game program:

  flash 00000  CPLD #0 (Centipede)   + Centipede ROM at A000-BFFF
  flash 0C000  Cosmodog main kernel (always mapped at CPU $C000-$FFFF)
  flash 10000  CPLD #1 (Millipede)   + Millipede ROM at 1C000-1FFFF
  flash 20000  CPLD #2 (Missile)     + Missile ROM at 2D000-2FFFF
  flash 30000  CPLD #3 (Bowling)     + Bowling ROM at 3A000-3FFFF

Game ROM window (when that game is selected):
  flash_addr = (game << 16) | (0x8000 + cpu_addr)

  Centipede : CPU $2000-$3FFF <- flash $0A000-$0BFFF
  Millipede : CPU $4000-$7FFF <- flash $1C000-$1FFFF
  Missile Command  : CPU $5000-$7FFF <- flash $2D000-$2FFFF
  Bowling   : CPU $2000-$7FFF <- flash $3A000-$3FFFF

$C003 selects the game / CPLD configuration:
  bits 1-0 = game (0=Centipede, 1=Millipede, 2=Missile, 3=Bowling)
  bit 7    = 1 for multipede menu/service mode
  readback bits 5-2 must be 0100 ($04) or the kernel reports
  "INCORRECT IMAGE U9"

$C002:
  bit 7 = activity LED / watchdog heartbeat (toggled by kernel IRQ)
  bit 6 = flash overlay (JEDEC programming); $C0 enables access so
          the kernel can talk to flash at $2AAA/$5555/$8000

$C000-$C00F is the multipede I/O block (always present).  $A000-$AFFF
is multipede work RAM (shared with patched game code for credits etc).
$B000-$BFFF is character RAM used by the multipede UI.

Let's Go Bowling
----------------
  $1000-$13BF  playfield VRAM: tile code in bits 6-0, colour set in bit 7
  $13C0-$13FF  16 sprites: code / Y / X / colour at +$00/$10/$20/$30
  $1400-$140F  POKEY (AUDF/AUDC pairs, AUDCTL at $1408, SKCTL at $140F)
  $1500        write ADPCM FIFO (encoding TBD), read bit 7 for FIFO full
  $1501        volume?
  $1502        playback rate?
  $1503        Write 0 to flush the FIFO and stop playback
  $1600        trackball roll counter (free-running 8-bit, not Atari quadrature)
  $1700        trackball aim counter (ditto); the game differences both itself
  $1800        IRQ acknowledge
  $1900-$191F  palette: 16 playfield pens + 16 sprite-expansion entries,
               same layout as Millipede's $2480 but the RGB bitfields are
               NOT inverted (see cmmb_dac_color)
  $B000-$B7FF  128 8x8 playfield tiles
  $B800-$BFFF  32 16x16 sprites

TODO:
- Missile Command: write-PROM accuracy (currently approximated with bit masks)
- Bowling: $1500 sample playback is stubbed
- Is there a second player?  Doesn't seem to be, at least in the upright cabinet.

***************************************************************************/

#include "emu.h"

#include "cpu/m6502/w65c02s.h"
#include "machine/bankdev.h"
#include "machine/intelfsh.h"
#include "machine/timer.h"
#include "sound/pokey.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

namespace {

static constexpr XTAL MAIN_CLOCK = XTAL(72'576'000);

class cmmb_state : public driver_device
{
public:
	cmmb_state(const machine_config &mconfig, device_type type, const char *tag);

	void cmmb(machine_config &config) ATTR_COLD;
	void cmmb103(machine_config &config) ATTR_COLD;
	void cmmb162(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	enum
	{
		VIEW_MENU = 0,

		VIEW_ATARI_BASE,
		VIEW_CENTIPEDE = VIEW_ATARI_BASE,
		VIEW_MILLIPEDE,
		VIEW_MISSILE,
		VIEW_BOWLING,

		VIEW_FLASH
	};

	required_device<w65c02s_device> m_maincpu;
	required_device<address_map_bank_device> m_mainmap;
	required_device<intelfsh8_device> m_flash;
	required_device<pokey_device> m_pokey, m_pokey2;
	required_device<screen_device> m_screen;
	memory_share_creator<u8> m_rambase;
	memory_share_creator<u8> m_missile_vram;
	required_shared_ptr<u8> m_videoram;
	required_shared_ptr<u8> m_charram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	memory_view m_lowview;
	required_ioport m_coins, m_buttons, m_system, m_dsw;
	required_ioport_array<2> m_trackball;

	u8 m_unk_c001;
	u8 m_sysctrl;
	u8 m_game_select;
	u8 m_sound_gate;
	u8 m_soft_dip[4];
	u8 m_outlatch;
	int m_irq_state;
	u8 m_flipscreen;
	u8 m_missile_ctrld;
	bool m_dsw_select;
	u8 m_coin_latched;
	u8 m_coin_prev;

	bool m_madsel_armed;
	u8 m_madsel_skips;
	u8 m_last_sync_op;
	u8 m_penmask[64];

	u8 m_track_oldpos[2];
	u8 m_track_sign[2];
	bool m_track_synced[2];
	u8 m_track_dir[2];
	u8 m_track_clk[2];

	tilemap_t *m_menu_tilemap;
	tilemap_t *m_milli_tilemap;
	tilemap_t *m_cent_tilemap;
	tilemap_t *m_bowl_tilemap;

	bool vblank_r();

	void update_lowview();
	int get_current_view() const;
	int selected_game() const;
	void set_irq(int state);
	u8 read_trackball(int axis, u8 switches);

	template <offs_t Base> u8 flash_r(offs_t offset);
	template <offs_t Base> void flash_w(offs_t offset, u8 data);

	u8 coin_latch_r();
	void coin_latch_clear_w(u8 data);
	u8 unk_c001_r();
	void unk_c001_w(u8 data);
	u8 sysctrl_r();
	void sysctrl_w(u8 data);
	u8 game_select_r();
	void game_select_w(u8 data);
	u8 sound_gate_r();
	void sound_gate_w(u8 data);
	u8 buttons_r();
	u8 system_r();

	u8 soft_dip_r(offs_t offset);
	void soft_dip_w(offs_t offset, u8 data);
	template <int N> u8 soft_dip_pot_r();

	template <int Axis> u8 milliped_track_r();
	u8 milliped_coin_bits();
	u8 milliped_coin_r();
	u8 milliped_IN3_r();
	template <int Axis> u8 centiped_track_r();
	u8 centiped_IN1_r();
	u8 missile_in1_r();
	u8 missile_r10_r();
	template <int Axis> u8 bowl_track_r();

	void irq_ack_w(u8 data);
	TIMER_DEVICE_CALLBACK_MEMBER(cb_irq);

	void vram_w(offs_t offset, u8 data);
	void cent_vram_w(offs_t offset, u8 data);
	void bowl_vram_w(offs_t offset, u8 data);
	void charram_w(offs_t offset, u8 data);
	void set_palette_entry(offs_t offset, rgb_t color);
	void milli_palette_w(offs_t offset, u8 data);
	void cent_palette_w(offs_t offset, u8 data);
	void bowl_palette_w(offs_t offset, u8 data);
	void missile_palette_w(offs_t offset, u8 data);
	u8 missile_4800_r();
	void missile_4800_w(u8 data);
	u8 missile_ram_r(offs_t offset);
	void missile_ram_w(offs_t offset, u8 data);
	void missile_load_madsel(u8 data, offs_t pc);
	void missile_disarm_madsel();
	bool missile_get_madsel_read();
	bool missile_get_madsel_write(offs_t offset);
	void missile_vram_mad_w(offs_t offset, u8 data);
	u8 missile_vram_mad_r(offs_t offset);
	static offs_t missile_bit3_addr(offs_t pixaddr);
	u8 trampoline_r(offs_t offset);
	void trampoline_w(offs_t offset, u8 data);
	void outlatch_w(offs_t offset, u8 data);

	void trampoline_map(address_map &map) ATTR_COLD;
	void cmmb_map(address_map &map) ATTR_COLD;
	void milliped_view_map(memory_view::memory_view_entry &view) ATTR_COLD;

	TILE_GET_INFO_MEMBER(menu_get_tile_info);
	TILE_GET_INFO_MEMBER(milli_get_tile_info);
	TILE_GET_INFO_MEMBER(cent_get_tile_info);
	TILE_GET_INFO_MEMBER(bowl_get_tile_info);
	void draw_atari_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, offs_t base, int gfxno, bool attr_flipx);
	void draw_bowl_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_missile_bitmap(bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};

cmmb_state::cmmb_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mainmap(*this, "mainmap"),
		m_flash(*this, "flash"),
		m_pokey(*this, "pokey"),
		m_pokey2(*this, "pokey2"),
		m_screen(*this, "screen"),
		m_rambase(*this, "rambase", 0x2000, ENDIANNESS_LITTLE),
		m_missile_vram(*this, "missile_vram", 0x10000, ENDIANNESS_LITTLE),
		m_videoram(*this, "videoram"),
		m_charram(*this, "charram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_lowview(*this, "lowview"),
		m_coins(*this, "COINS"),
		m_buttons(*this, "BUTTONS"),
		m_system(*this, "SYSTEM"),
		m_dsw(*this, "DSW"),
		m_trackball(*this, "TRACKBALL_%u", 0U),
		m_unk_c001(0),
		m_sysctrl(0),
		m_game_select(0),
		m_sound_gate(0),
		m_soft_dip{ 0, 0, 0, 0 },
		m_outlatch(0),
		m_irq_state(CLEAR_LINE),
		m_flipscreen(0),
		m_missile_ctrld(0),
		m_dsw_select(false),
		m_coin_latched(0),
		m_coin_prev(0),
		m_madsel_armed(false),
		m_madsel_skips(0),
		m_last_sync_op(0),
		m_penmask{ },
		m_track_oldpos{ 0, 0 },
		m_track_sign{ 0, 0 },
		m_track_synced{ false, false },
		m_track_dir{ 1, 1 },
		m_track_clk{ 1, 1 },
		m_menu_tilemap(nullptr),
		m_milli_tilemap(nullptr),
		m_cent_tilemap(nullptr),
		m_bowl_tilemap(nullptr)
{
}

// Cosmodog reorders the character ROM in groups of (1 << group_shift) tiles and
// the CPLD works it out: the two index bits above group_shift come back swapped
// with the new high bit inverted, so groups 0,1,2,3 become 2,0,3,1.
static inline u8 cmmb_scramble_index(u8 index, unsigned group_shift)
{
	const u8 group = bitswap<2>(index >> group_shift, 0, 1) ^ 2;
	return u8((group << group_shift) | (index & util::make_bitmask<u8>(group_shift)));
}

// Game picked by $C003 bits 1-0, ignoring the menu and flash overlays.
int cmmb_state::selected_game() const
{
	return VIEW_ATARI_BASE + (m_game_select & 0x03);
}

int cmmb_state::get_current_view() const
{
	if (BIT(m_sysctrl, 6))
	{
		return VIEW_FLASH;
	}
	if (BIT(m_game_select, 7))
	{
		return VIEW_MENU;
	}

	return selected_game();
}

TILE_GET_INFO_MEMBER(cmmb_state::menu_get_tile_info)
{
	if (tile_index >= 0x3c0)
	{
		tileinfo.set(0, 0, 0, 0);
		return;
	}
	const u8 data = m_videoram[tile_index];
	tileinfo.set(0, data & 0x3f, (data >> 6) & 3, 0);
}

TILE_GET_INFO_MEMBER(cmmb_state::milli_get_tile_info)
{
	if (tile_index >= 0x3c0)
	{
		tileinfo.set(0, 0, 0, 0);
		return;
	}
	const u8 data = m_videoram[tile_index];
	const int bank = BIT(data, 6);
	const int color = (data >> 6) & 3;
	const int flip = m_flipscreen ? 0x03 : 0;
	const u8 orig = u8((data & 0x3f) + 0x40 + (bank * 0x80));
	tileinfo.set(0, cmmb_scramble_index(orig, 6), color, TILE_FLIPYX(flip));
}

TILE_GET_INFO_MEMBER(cmmb_state::cent_get_tile_info)
{
	if (tile_index >= 0x3c0)
	{
		tileinfo.set(2, 0, 0, 0);
		return;
	}
	const u8 data = m_rambase[0x0400 + tile_index];
	const u8 orig = (data & 0x3f) + 0x40;
	tileinfo.set(2, cmmb_scramble_index(orig, 6), 0, TILE_FLIPYX(data >> 6));
}

TILE_GET_INFO_MEMBER(cmmb_state::bowl_get_tile_info)
{
	if (tile_index >= 0x3c0)
	{
		tileinfo.set(0, 0, 0, 0);
		return;
	}
	const u8 data = m_rambase[0x1000 + tile_index];
	tileinfo.set(0, data & 0x7f, BIT(data, 7), 0);
}

void cmmb_state::video_start()
{
	for (int i = 0; i < 64; i++)
	{
		u8 mask = 1;
		if (((i >> 0) & 3) == 0)
		{
			mask |= 2;
		}
		if (((i >> 2) & 3) == 0)
		{
			mask |= 4;
		}
		if (((i >> 4) & 3) == 0)
		{
			mask |= 8;
		}
		m_penmask[i] = mask;
	}

	m_menu_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cmmb_state::menu_get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_milli_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cmmb_state::milli_get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_cent_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cmmb_state::cent_get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bowl_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cmmb_state::bowl_get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

// Playfield writes.  The menu and Millipede bitstreams share one window at $1000
// and one tilemap source; Centipede's is at $0400 and Bowling's, despite being at
// $1000 too, is *not* the same storage.  Putting them in one buffer leaves menu
// leftovers all over the bowling screen.
void cmmb_state::vram_w(offs_t offset, u8 data)
{
	m_videoram[offset] = data;
	m_menu_tilemap->mark_tile_dirty(offset);
	m_milli_tilemap->mark_tile_dirty(offset);
}

void cmmb_state::cent_vram_w(offs_t offset, u8 data)
{
	m_rambase[0x0400 + offset] = data;
	m_cent_tilemap->mark_tile_dirty(offset);
}

void cmmb_state::bowl_vram_w(offs_t offset, u8 data)
{
	m_rambase[0x1000 + offset] = data;
	m_bowl_tilemap->mark_tile_dirty(offset);
}

void cmmb_state::charram_w(offs_t offset, u8 data)
{
	m_charram[offset] = data;
	m_gfxdecode->gfx(0)->mark_dirty(offset >> 4);
	m_gfxdecode->gfx(1)->mark_dirty(offset >> 5);
	m_gfxdecode->gfx(2)->mark_dirty(offset >> 4);
	m_gfxdecode->gfx(3)->mark_dirty(offset >> 5);
	if (offset >= 0x800)
	{
		m_gfxdecode->gfx(4)->mark_dirty((offset - 0x800) >> 6);
	}
}

static rgb_t cmmb_dac_color(u8 data)
{
	const int r = 0x21 * BIT(data, 5) + 0x47 * BIT(data, 6) + 0x97 * BIT(data, 7);
	const int g = 0x47 * BIT(data, 3) + 0x97 * BIT(data, 4);
	const int b = 0x21 * BIT(data, 0) + 0x47 * BIT(data, 1) + 0x97 * BIT(data, 2);
	return rgb_t(r, g, b);
}

// 32-entry palette RAM shared by every bitstream that has one: entries $00-$0F
// are the four playfield colour sets, $10-$1F the sprite colour expansion
// (sprite pen n comes from the group selected by colour bits 7-6, indexed by
// bits (2n-1):(2n-2) of the sprite's colour byte).
void cmmb_state::set_palette_entry(offs_t offset, rgb_t color)
{
	if (offset < 0x10)
	{
		m_palette->set_pen_color(offset, color);
	}
	else
	{
		const int base = offset & 0x0c;
		const int off = offset & 0x03;
		for (int i = (base << 6); i < (base << 6) + 0x100; i += 4)
		{
			if (off == ((i >> 2) & 0x03))
			{
				m_palette->set_pen_color(i + 0x10 + 1, color);
			}
			if (off == ((i >> 4) & 0x03))
			{
				m_palette->set_pen_color(i + 0x10 + 2, color);
			}
			if (off == ((i >> 6) & 0x03))
			{
				m_palette->set_pen_color(i + 0x10 + 3, color);
			}
		}
	}
}

void cmmb_state::milli_palette_w(offs_t offset, u8 data)
{
	set_palette_entry(offset, cmmb_dac_color(u8(~data)));
}

void cmmb_state::cent_palette_w(offs_t offset, u8 data)
{
	// Character pens at 0-3.  Sprite expansion at base 0x10 to match GFXDECODE
	// (original board used base 4; we share 0x10 with Millipede/menu).
	if (!BIT(offset, 2))
	{
		return;
	}

	const u8 inv = u8(~data);
	int r = 0xff * BIT(inv, 0);
	int g = 0xff * BIT(inv, 1);
	int b = 0xff * BIT(inv, 2);
	if (BIT(inv, 3))
	{
		if (b)
		{
			b = 0xc0;
		}
		else if (g)
		{
			g = 0xc0;
		}
	}
	const rgb_t color(r, g, b);

	if (!BIT(offset, 3))
	{
		const int pen = offset & 0x03;
		if (pen == 0)
		{
			m_palette->set_pen_color(0, rgb_t(0, 0, 0));
		}
		else
		{
			m_palette->set_pen_color(pen, color);
		}
	}
	else
	{
		// sprite expansion, colour set 0
		set_palette_entry(0x10 | (offset & 0x03), color);
	}
}

void cmmb_state::bowl_palette_w(offs_t offset, u8 data)
{
	// Cosmodog's bowling bitstream puts its 32-byte palette RAM at $1900 and
	// drives the video DAC directly rather than inverted like the Atari games.
	set_palette_entry(offset, cmmb_dac_color(data));
}

void cmmb_state::missile_palette_w(offs_t offset, u8 data)
{
	m_palette->set_pen_color(offset & 7, pal1bit(~data >> 3), pal1bit(~data >> 2), pal1bit(~data >> 1));
}

u8 cmmb_state::missile_4800_r()
{
	if (m_missile_ctrld)
	{
		const u8 x = u8(~m_trackball[0]->read());
		const u8 y = u8(~m_trackball[1]->read());
		return (y << 4) | (x & 0x0f);
	}
	return m_buttons->read() | 0xf0;
}

void cmmb_state::missile_4800_w(u8 data)
{
	m_missile_ctrld = BIT(data, 0);
	m_flipscreen = !BIT(data, 6);
}

void cmmb_state::missile_disarm_madsel()
{
	m_madsel_armed = false;
	m_madsel_skips = 0;
}

void cmmb_state::missile_load_madsel(u8 data, offs_t pc)
{
	// Arm MADSEL on Missile-ROM fetches of (zp,X) ops: low 5 bits == $01
	// (ORA/AND/EOR/ADC/STA/LDA/CMP/SBC).  STA/LDA are the pixel path.
	//
	// Only opcodes from $5000-$7FFF — multipede kernel at $C000+ must never
	// arm or a later stack/I/O cycle is stolen into VRAM and the kernel
	// aborts to the select menu.
	if (!m_maincpu->get_sync() || machine().side_effects_disabled())
	{
		return;
	}
	if (pc < 0x5000 || pc >= 0x8000)
	{
		missile_disarm_madsel();
		return;
	}
	m_last_sync_op = data;
	if ((data & 0x1f) != 0x01)
	{
		missile_disarm_madsel();
		return;
	}
	// 65C02 sta_s_idx / lda_s_idx after opcode prefetch:
	//   read_pc (zp), read_pc (dummy), read ptr.lo, read ptr.hi, then data.
	// Four intermediate memops, then the MADSEL data cycle.
	//
	// If IRQ is taken after this prefetch, the (zp,X) is abandoned (IR forced
	// to the interrupt sequence).  Stack pushes then appear while still armed —
	// trampoline_w cancels the arm on ZP/stack writes (see below).
	m_madsel_armed = true;
	m_madsel_skips = 4;
}

bool cmmb_state::missile_get_madsel_read()
{
	// Read-side data cycle (LDA (zp,X) etc.) after pointer walks.
	if (!m_madsel_armed)
	{
		return false;
	}
	if (m_madsel_skips > 0)
	{
		if (!machine().side_effects_disabled())
		{
			m_madsel_skips--;
		}
		return false;
	}
	if (!machine().side_effects_disabled())
	{
		missile_disarm_madsel();
	}
	return true;
}

// True when MADSEL may redirect this address as a pixel coordinate.
static bool missile_madsel_addr(offs_t offset)
{
	// ZP/stack share top blanking DRAM but stack pushes are ordinary writes.
	return offset >= 0x0200;
}

bool cmmb_state::missile_get_madsel_write(offs_t offset)
{
	// STA (zp,X) has no intermediate writes — the first bus write while armed
	// is the pixel store.  Do not rely on the skip counter alone (IRQ can
	// burn skips on stack traffic if we fail to cancel in time).
	if (!m_madsel_armed)
	{
		return false;
	}

	if (!machine().side_effects_disabled())
	{
		missile_disarm_madsel();
	}

	// Prefetch armed (zp,X) then IRQ: first traffic is stack at $0100-$01FF.
	// Cancel so we do not pack stack or leave a stale arm into the kernel.
	return missile_madsel_addr(offset);
}

// Reads of the multipede I/O block must never be MADSEL-packed: the kernel
// IRQ BIT/AND-tests $C00E and would treat pixel garbage as fire/start.
// Writes still pack — Missile blits through the whole 16-bit space, and
// addresses that land in $C000-$C00F would otherwise stomp $C003 game select.
static bool missile_madsel_read_addr(offs_t offset)
{
	return missile_madsel_addr(offset) && (offset < 0xc000 || offset > 0xc00f);
}

offs_t cmmb_state::missile_bit3_addr(offs_t pixaddr)
{
	return (( pixaddr & 0x0800) >> 1) |
			((~pixaddr & 0x0800) >> 2) |
			(( pixaddr & 0x07f8) >> 2) |
			(( pixaddr & 0x1000) >> 12);
}

void cmmb_state::missile_vram_mad_w(offs_t offset, u8 data)
{
	// Same packing as missile_state::vram_mad_w.  Write-enable masks match the
	// read-side layout (pixel n occupies bits n and n+4); the real board uses
	// PROM 035826-01 for this.
	constexpr u8 data_lookup[4] = { 0x00, 0x0f, 0xf0, 0xff };
	const offs_t vramaddr = (offset >> 2) & 0xffff;
	const u8 vramdata = data_lookup[data >> 6];
	const u8 writemask = u8(0x11 << (offset & 3)); // bits updated for this pixel
	m_missile_vram[vramaddr] = (m_missile_vram[vramaddr] & ~writemask) | (vramdata & writemask);

	// MUSHROOM: 3rd bit plane for the bottom band (cities / ground)
	if ((offset & 0xe000) == 0xe000)
	{
		const offs_t a3 = missile_bit3_addr(offset) & 0xffff;
		const u8 bit = u8(1 << (offset & 7));
		if (BIT(data, 5))
		{
			m_missile_vram[a3] |= bit;
		}
		else
		{
			m_missile_vram[a3] &= u8(~bit);
		}
		if (!machine().side_effects_disabled())
		{
			m_maincpu->adjust_icount(-1);
		}
	}
}

u8 cmmb_state::missile_vram_mad_r(offs_t offset)
{
	u8 result = 0xff;
	const offs_t vramaddr = (offset >> 2) & 0xffff;
	const u8 vrammask = 0x11 << (offset & 3);
	const u8 vramdata = m_missile_vram[vramaddr] & vrammask;
	if ((vramdata & 0xf0) == 0)
	{
		result &= ~0x80;
	}
	if ((vramdata & 0x0f) == 0)
	{
		result &= ~0x40;
	}

	if ((offset & 0xe000) == 0xe000)
	{
		const offs_t a3 = missile_bit3_addr(offset) & 0xffff;
		if (!BIT(m_missile_vram[a3], offset & 7))
		{
			result &= ~0x20;
		}
		if (!machine().side_effects_disabled())
		{
			m_maincpu->adjust_icount(-1);
		}
	}
	return result;
}

u8 cmmb_state::missile_ram_r(offs_t offset)
{
	// Direct byte access into the 64KB video buffer (work RAM in low pages,
	// packed 2bpp playfield elsewhere).  MADSEL packing is handled by the
	// full-space trampoline — never force-pack by address.
	return m_missile_vram[offset & 0xffff];
}

void cmmb_state::missile_ram_w(offs_t offset, u8 data)
{
	if (offset == 0x00ea)
	{
		data &= 0xbf;
	}

	m_missile_vram[offset & 0xffff] = data;
}

u8 cmmb_state::trampoline_r(offs_t offset)
{
	// Cosmodog's Missile CPLD implements MADSEL steering like the PCB: when
	// MADSEL is high, the full 16-bit address is a pixel coordinate — including
	// $E000+ (MUSHROOM / cities, native Y ≥ 224).  Only skip ZP/stack
	// ($0000-$01FF): those pages share DRAM with top blanking, but hardware
	// stack pushes are ordinary writes, not MADSEL cycles.
	//
	// MADSEL is armed only for opcodes fetched from Missile ROM ($5000-$7FFF),
	// so multipede kernel accesses at $A000/$C000 never trigger packing.
	if (get_current_view() == VIEW_MISSILE)
	{
		// Kernel / non-game PC: never leave a stale (zp,X) arm across IRQ.
		const u16 pc = m_maincpu->pc();
		if (pc < 0x5000 || pc >= 0x8000)
		{
			missile_disarm_madsel();
		}

		// Opcode/prefetch (SYNC): arm or disarm only — never treat as pixel.
		if (m_maincpu->get_sync())
		{
			const u8 data = m_mainmap->read8(offset);
			missile_load_madsel(data, offset);
			return data;
		}

		if (missile_get_madsel_read() && missile_madsel_read_addr(offset))
		{
			return missile_vram_mad_r(offset);
		}
	}

	return m_mainmap->read8(offset);
}

void cmmb_state::trampoline_w(offs_t offset, u8 data)
{
	// Cities plot at $E000+ under MADSEL; pixel ops may also land in $C000-$C00F.
	if (get_current_view() == VIEW_MISSILE)
	{
		const u16 pc = m_maincpu->pc();
		if (pc < 0x5000 || pc >= 0x8000)
		{
			missile_disarm_madsel();
		}

		if (missile_get_madsel_write(offset))
		{
			missile_vram_mad_w(offset, data);
			return;
		}

		// Safety net: game code must never poke multipede I/O as ordinary RAM
		// (MADSEL-miss on STA ($9E,X) with pointer in $C000 used to stomp
		// $C003).  Kernel writes have PC ≥ $C000.  Absolute multipede hooks
		// target $A0xx, not $C000-$C00F.
		if (pc >= 0x5000 && pc < 0x8000 && offset >= 0xc000 && offset <= 0xc00f)
		{
			missile_vram_mad_w(offset, data);
			return;
		}

		// Multipede workspace $A0xx: Cosmodog hooks use absolute ops —
		// STA ($A0A5 gate), INC/DEC ($A0AD insert-coin countdown, etc.).
		// A missed STA ($zz,X) pixel plot that lands here used to poison
		// $A0A5 (bit6 → kernel JMP $DF8C with credits).  Only redirect the
		// (zp,X) family (low 5 bits == $01); all absolute RMW/stores pass.
		if (pc >= 0x5000 && pc < 0x8000 && offset >= 0xa000 && offset <= 0xafff)
		{
			if ((m_last_sync_op & 0x1f) == 0x01 || m_madsel_armed)
			{
				missile_disarm_madsel();
				missile_vram_mad_w(offset, data);
				return;
			}
		}
	}

	// cmmb103 JEDEC unlock uses A15-high aliases ($D555/$AAAA).  The AT29x
	// core matches on address & $7FFF, so forward to the standard unlock cells
	// before the normal map (ROM ignores $D555; $AAAA is multipede work RAM).
	if (offset == 0xd555)
	{
		m_flash->write(0x5555, data);
	}
	else if (offset == 0xaaaa)
	{
		m_flash->write(0x2aaa, data);
	}

	m_mainmap->write8(offset, data);
}

void cmmb_state::outlatch_w(offs_t offset, u8 data)
{
	if (offset == 5)
	{
		m_dsw_select = !BIT(data, 7);
	}
	if (offset == 6)
	{
		m_flipscreen = BIT(data, 7);
	}
	if (offset == 7)
	{
		m_outlatch = data;
	}
}

void cmmb_state::draw_atari_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, offs_t base, int gfxno, bool attr_flipx)
{
	rectangle spriteclip = cliprect;
	if (m_flipscreen)
	{
		spriteclip.min_x += 8;
	}
	else
	{
		spriteclip.max_x -= 8;
	}

	u8 const *const spr = &m_rambase[base];
	for (int offs = 0; offs < 0x10; offs++)
	{
		const u8 attr = spr[offs];
		const int color = spr[offs + 0x30];
		const int tile = BIT(attr, 1, 5) | (BIT(attr, 0) << 6);
		const int code = cmmb_scramble_index(u8(tile), 5);
		const int flipx = attr_flipx ? BIT(attr, 6) : m_flipscreen;
		const int flipy = BIT(attr, 7) ^ 1;
		const int x = spr[offs + 0x20];
		const int y = 240 - spr[offs + 0x10];
		m_gfxdecode->gfx(gfxno)->transmask(bitmap, spriteclip, code, color, flipx, flipy, x, y, m_penmask[color & 0x3f]);
	}
}

void cmmb_state::draw_bowl_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u8 const *const spr = &m_rambase[0x13c0];
	for (int offs = 0; offs < 0x10; offs++)
	{
		const u8 attr = spr[offs];
		const int color = spr[offs + 0x30];
		const int y = spr[offs + 0x10];
		const int x = spr[offs + 0x20];
		m_gfxdecode->gfx(4)->transmask(bitmap, cliprect, attr & 0x1f, color, 0, BIT(attr, 7) ^ 1, x, y, m_penmask[color & 0x3f]);
	}
}

void cmmb_state::draw_missile_bitmap(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	constexpr int first_vis = 25; // matches missile.cpp VBEND
	for (int sy = first_vis; sy < 256; sy++)
	{
		const int effy = m_flipscreen ? ((256 + 24 - sy) & 0xff) : sy;
		u8 const *const src = &m_missile_vram[effy * 64];
		u8 const *src3 = nullptr;
		if (effy >= 224)
		{
			src3 = &m_missile_vram[missile_bit3_addr(u32(effy) << 8)];
		}

		for (int sx = 0; sx < 256; sx++)
		{
			u8 const *const col = &src[sx / 4];
			u8 pix = u8((BIT(*col, (sx & 3) + 4) << 2) | (BIT(*col, sx & 3) << 1));
			if (src3)
			{
				pix |= BIT(src3[(sx / 8) * 2], sx & 7);
			}

			const int dx = 255 - sy;
			const int dy = sx;
			if (cliprect.contains(dx, dy))
			{
				u16 *const dst = &bitmap.pix(dy);
				dst[dx] = pix & 7;
			}
		}
	}
}

u32 cmmb_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const int view = get_current_view();

	switch (view)
	{
	case VIEW_MENU:
		bitmap.fill(0, cliprect);
		m_menu_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		draw_atari_sprites(bitmap, cliprect, 0x13c0, 1, false);
		break;

	case VIEW_MILLIPEDE:
		m_milli_tilemap->draw(screen, bitmap, cliprect, 0, 0);
		draw_atari_sprites(bitmap, cliprect, 0x13c0, 1, false);
		break;

	case VIEW_CENTIPEDE:
		bitmap.fill(0, cliprect);
		m_cent_tilemap->draw(screen, bitmap, cliprect, 0, 0);
		draw_atari_sprites(bitmap, cliprect, 0x07c0, 3, true);
		break;

	case VIEW_MISSILE:
		draw_missile_bitmap(bitmap, cliprect);
		break;

	case VIEW_BOWLING:
		m_bowl_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		draw_bowl_sprites(bitmap, cliprect);
		break;

	default:
		bitmap.fill(0, cliprect);
		break;
	}
	return 0;
}

void cmmb_state::update_lowview()
{
	m_lowview.select(get_current_view());
}

template <offs_t Base>
u8 cmmb_state::flash_r(offs_t offset)
{
	return m_flash->read(Base + offset);
}

template <offs_t Base>
void cmmb_state::flash_w(offs_t offset, u8 data)
{
	m_flash->write(Base + offset, data);
}

// $C001: apparently just a kernel scratch register?
u8 cmmb_state::unk_c001_r()
{
	return m_unk_c001;
}

void cmmb_state::unk_c001_w(u8 data)
{
	m_unk_c001 = data;
}

u8 cmmb_state::sysctrl_r()
{
	return m_sysctrl;
}

// kind of a Konami-style control register:
// bit 7: LED / watchdog heartbeat (kernel IRQ toggles it)
// bit 6: flash programming overlay
void cmmb_state::sysctrl_w(u8 data)
{
	const u8 changed = m_sysctrl ^ data;
	m_sysctrl = data;
	if (BIT(changed, 6))
	{
		update_lowview();
	}
}

u8 cmmb_state::sound_gate_r()
{
	return m_sound_gate;
}

void cmmb_state::sound_gate_w(u8 data)
{
	m_sound_gate = data;
}

// bits 0-1: game select / which CPLD bitstream is live
// bit 7: menu/service mode
void cmmb_state::game_select_w(u8 data)
{
	// If we're in Missile Command, ignore writes that aren't from the kernel
	if (get_current_view() == VIEW_MISSILE)
	{
		const u16 pc = m_maincpu->pc();
		if (pc < 0xc000)
		{
			return;
		}
	}

	const u8 changed = m_game_select ^ data;
	m_game_select = data;
	if (changed & 0x83)
	{
		update_lowview();

		for (tilemap_t *const tmap : { m_menu_tilemap, m_milli_tilemap, m_cent_tilemap, m_bowl_tilemap })
		{
			tmap->mark_all_dirty();
		}

		for (int i = 0; i < 5; i++)
		{
			m_gfxdecode->gfx(i)->mark_all_dirty();
		}
		if (!BIT(data, 7) && (data & 0x03) == 0)
		{
			m_palette->set_pen_color(0, rgb_t(0, 0, 0));
		}
	}
}

u8 cmmb_state::game_select_r()
{
	return (m_game_select & 0xc3) | 0x04;
}

u8 cmmb_state::coin_latch_r()
{
	const u8 raw = m_coins->read() & 0xc0;
	const u8 pressed = u8(~raw) & 0xc0;

	m_coin_latched |= pressed & ~m_coin_prev;
	m_coin_prev = pressed;
	return m_coin_latched;
}

void cmmb_state::coin_latch_clear_w(u8 data)
{
	m_coin_latched = 0;
}

u8 cmmb_state::system_r()
{
	return m_system->read();
}

u8 cmmb_state::buttons_r()
{
	// bits 7-6 report the live CPLD configuration back to the kernel
	const u8 game = BIT(m_game_select, 7) ? 0x01 : (m_game_select & 0x03);
	return (m_buttons->read() & 0x3f) | (game << 6);
}

u8 cmmb_state::read_trackball(int axis, u8 switches)
{
	const u8 newpos = m_trackball[axis]->read();
	if (!m_track_synced[axis])
	{
		m_track_oldpos[axis] = newpos;
		m_track_synced[axis] = true;
	}
	else if (newpos != m_track_oldpos[axis])
	{
		m_track_dir[axis] = BIT(newpos - m_track_oldpos[axis], 7);
		m_track_sign[axis] = u8(m_track_dir[axis] << 7);
		m_track_clk[axis] ^= 1;
		m_track_oldpos[axis] = newpos;
	}
	return (switches & 0x70) | (m_track_oldpos[axis] & 0x0f) | m_track_sign[axis];
}

// Bowling returns the direct trackball position (presumably that bitstream does the quadrature decode itself)
template <int Axis>
u8 cmmb_state::bowl_track_r()
{
	const u8 pos = m_trackball[Axis]->read();
	return Axis ? pos : u8(~pos);
}

// Millipede's $2000/$2001: one trackball axis each, plus the fire button and
// start switch of the matching player in the unused quadrature bits.  With TBEN
// low (see outlatch_w) the same addresses present the option DIPs instead.
template <int Axis>
u8 cmmb_state::milliped_track_r()
{
	u8 sw = 0x70;
	if (!BIT(m_buttons->read(), Axis))
	{
		sw &= ~0x10;
	}
	if (!BIT(m_system->read(), 3 - Axis))
	{
		sw &= ~0x20;
	}
	if (Axis == 0 && !vblank_r())
	{
		sw &= ~0x40;
	}

	if (m_dsw_select)
	{
		if (Axis == 0)
		{
			const u8 lang = m_maincpu->space(AS_PROGRAM).read_byte(0xaf0e) & 0x03;
			const u8 bonus = 0x04;
			return (lang | bonus) | (sw & 0x70);
		}
		return sw & 0x70;
	}
	return read_trackball(Axis, sw);
}

// Centipede's $0C00/$0C02.  The horizontal axis also carries VBLANK and the
// self-test switch in the bits the quadrature decode leaves free.
template <int Axis>
u8 cmmb_state::centiped_track_r()
{
	u8 sw = 0x00;
	if (Axis == 0)
	{
		if (vblank_r())
		{
			sw |= 0x40;
		}
		if (BIT(m_system->read(), 7))
		{
			sw |= 0x20;
		}
	}
	return read_trackball(Axis, sw);
}

u8 cmmb_state::centiped_IN1_r()
{
	const u8 sys = m_system->read();
	const u8 btn = m_buttons->read();
	u8 v = 0xff;
	if (!BIT(sys, 3))
	{
		v &= ~0x01;
	}
	if (!BIT(sys, 2))
	{
		v &= ~0x02;
	}
	if (!BIT(btn, 0))
	{
		v &= ~0x04;
	}
	if (!BIT(sys, 5))
	{
		v &= ~0x20;
	}
	if (!BIT(sys, 7))
	{
		v &= ~0x80;
	}
	return v;
}

u8 cmmb_state::milliped_coin_bits()
{
	const u8 coins = m_coins->read();
	return u8((BIT(coins, 6) << 6) | (BIT(coins, 7) << 4));
}

u8 cmmb_state::milliped_coin_r()
{
	return u8(0xaf | milliped_coin_bits());
}

u8 cmmb_state::milliped_IN3_r()
{
	read_trackball(0, 0x70);
	read_trackball(1, 0x70);

	u8 v = 0x20;        // cabinet type = upright
	v |= milliped_coin_bits();

	// test switch
	if (BIT(m_system->read(), 7))
	{
		v |= 0x80;
	}

	if (m_track_clk[1])
	{
		v |= 0x01;
	}
	if (m_track_dir[1])
	{
		v |= 0x02;
	}
	if (m_track_clk[0])
	{
		v |= 0x04;
	}
	if (m_track_dir[0])
	{
		v |= 0x08;
	}
	return v;
}

u8 cmmb_state::missile_in1_r()
{
	const u8 btn = m_buttons->read();
	u8 v = 0x7f;
	if (!BIT(btn, 2))
	{
		v &= ~0x01;
	}
	if (!BIT(btn, 1))
	{
		v &= ~0x02;
	}
	if (!BIT(btn, 0))
	{
		v &= ~0x04;
	}

	// test switch
	if (!BIT(m_system->read(), 7))
	{
		v &= ~0x40;
	}
	return v | (vblank_r() ? 0x80 : 0);
}

u8 cmmb_state::missile_r10_r()
{
	// $4A00 R10 — soft options the kernel wrote to $C005 or cabinet DSW
	return m_soft_dip[0] ? m_soft_dip[0] : m_dsw->read();
}

u8 cmmb_state::soft_dip_r(offs_t offset)
{
	return m_soft_dip[offset & 3];
}

void cmmb_state::soft_dip_w(offs_t offset, u8 data)
{
	m_soft_dip[offset & 3] = data;
}

template <int N>
u8 cmmb_state::soft_dip_pot_r()
{
	return m_soft_dip[N];
}

void cmmb_state::set_irq(int state)
{
	state = state ? ASSERT_LINE : CLEAR_LINE;
	if (state != m_irq_state)
	{
		m_irq_state = state;
		m_maincpu->set_input_line(m6502_device::IRQ_LINE, m_irq_state);
	}
}

void cmmb_state::irq_ack_w(u8 data)
{
	set_irq(CLEAR_LINE);
}

bool cmmb_state::vblank_r()
{
	if (!BIT(m_game_select, 7))
	{
		switch (selected_game())
		{
		case VIEW_CENTIPEDE:
		case VIEW_MILLIPEDE:
			return m_screen->vpos() >= 240;

		case VIEW_MISSILE:
			// Missile's /VBLANK is the top blanking band instead
			return m_screen->vpos() < 24;
		}
	}
	return m_screen->vblank();
}

// IRQ timing is part of each CY39100 bitstream (Cosmodog reloads the CPLD
// when switching games).  Model the three schemes we know:
//
//  * Multipede menu / service ($C003 bit7): one IRQ per frame
//
//  * Centipede / Millipede: same 16V/32V clock as the Atari PCBs (see
//    centiped_state::generate_interrupt)
//
//  * Missile Command: /32V edges at V=0,64,128,192
TIMER_DEVICE_CALLBACK_MEMBER(cmmb_state::cb_irq)
{
	const int scanline = param;

	// in menu / service mode, we need only 1 IRQ per frame
	if (BIT(m_game_select, 7))
	{
		if (scanline == 256)
		{
			set_irq(ASSERT_LINE);
		}
		return;
	}

	switch (selected_game())
	{
	case VIEW_CENTIPEDE:
	case VIEW_MILLIPEDE:
		if (scanline < 240)
		{
			if (BIT(scanline, 4))
			{
				set_irq(BIT(scanline - 1, 5) ? ASSERT_LINE : CLEAR_LINE);
			}
		}
		else if (scanline == 256)
		{
			set_irq(ASSERT_LINE);
		}
		break;

	case VIEW_MISSILE:
		if ((scanline & 31) == 0 && scanline < 256)
		{
			const u8 v = u8(scanline);
			set_irq(BIT(u8(~v), 5) ? ASSERT_LINE : CLEAR_LINE);
		}
		break;

	default:
		if (scanline == 256)
		{
			set_irq(ASSERT_LINE);
		}
		break;
	}
}

// Missile Command's MADSEL mechanism does weird things to the address space...
void cmmb_state::trampoline_map(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(cmmb_state::trampoline_r), FUNC(cmmb_state::trampoline_w));
}

// The menu map is based on Millipede's, so this base map works for both
void cmmb_state::milliped_view_map(memory_view::memory_view_entry &view)
{
	// The zero page and stack must stay consistent across bank/CPLD switches
	view(0x0000, 0x1fff).ram().share(m_rambase);
	view(0x0400, 0x040f).rw(m_pokey, FUNC(pokey_device::read), FUNC(pokey_device::write));
	view(0x0800, 0x080f).rw(m_pokey2, FUNC(pokey_device::read), FUNC(pokey_device::write));
	view(0x1000, 0x13bf).ram().w(FUNC(cmmb_state::vram_w)).share(m_videoram);
	view(0x2000, 0x2000).r(FUNC(cmmb_state::milliped_track_r<0>));
	view(0x2001, 0x2001).r(FUNC(cmmb_state::milliped_track_r<1>));
	view(0x2010, 0x2010).r(FUNC(cmmb_state::milliped_coin_r));
	view(0x2011, 0x2011).r(FUNC(cmmb_state::milliped_IN3_r));
	view(0x2480, 0x249f).w(FUNC(cmmb_state::milli_palette_w));
	view(0x2500, 0x2507).w(FUNC(cmmb_state::outlatch_w));
	view(0x2600, 0x2600).w(FUNC(cmmb_state::irq_ack_w));
	view(0x2680, 0x2680).nopw();
}

void cmmb_state::cmmb_map(address_map &map)
{
	// $0000-$7fff is mode-dependent on CPLD swaps.  Most of $8000-$ffff is fixed
	map(0x0000, 0x7fff).view(m_lowview);

	map(0x8000, 0x9fff).rw(FUNC(cmmb_state::flash_r<0x8000>), FUNC(cmmb_state::flash_w<0x8000>));

	milliped_view_map(m_lowview[VIEW_MENU]);

	m_lowview[VIEW_CENTIPEDE](0x0000, 0x1fff).ram().share(m_rambase);
	m_lowview[VIEW_CENTIPEDE](0x0400, 0x07bf).w(FUNC(cmmb_state::cent_vram_w));
	m_lowview[VIEW_CENTIPEDE](0x0800, 0x0801).r(FUNC(cmmb_state::soft_dip_r));
	m_lowview[VIEW_CENTIPEDE](0x0c00, 0x0c00).r(FUNC(cmmb_state::centiped_track_r<0>));
	m_lowview[VIEW_CENTIPEDE](0x0c01, 0x0c01).r(FUNC(cmmb_state::centiped_IN1_r));
	m_lowview[VIEW_CENTIPEDE](0x0c02, 0x0c02).r(FUNC(cmmb_state::centiped_track_r<1>));
	m_lowview[VIEW_CENTIPEDE](0x0c03, 0x0c03).lr8(NAME([] () { return 0xff; }));
	m_lowview[VIEW_CENTIPEDE](0x1000, 0x100f).rw(m_pokey, FUNC(pokey_device::read), FUNC(pokey_device::write));
	m_lowview[VIEW_CENTIPEDE](0x1400, 0x140f).w(FUNC(cmmb_state::cent_palette_w));
	m_lowview[VIEW_CENTIPEDE](0x1600, 0x163f).nopw();
	m_lowview[VIEW_CENTIPEDE](0x1680, 0x1680).nopw();
	m_lowview[VIEW_CENTIPEDE](0x1700, 0x173f).lr8(NAME([] () { return 0x00; }));
	m_lowview[VIEW_CENTIPEDE](0x1800, 0x1800).w(FUNC(cmmb_state::irq_ack_w));
	m_lowview[VIEW_CENTIPEDE](0x1c00, 0x1c07).nopw();
	m_lowview[VIEW_CENTIPEDE](0x2000, 0x3fff).rom().region("flash", 0x0a000);
	m_lowview[VIEW_CENTIPEDE](0x2000, 0x2000).nopw(); // watchdog

	milliped_view_map(m_lowview[VIEW_MILLIPEDE]);
	m_lowview[VIEW_MILLIPEDE](0x2030, 0x2030).lr8(NAME([] () { return 0x00; }));
	m_lowview[VIEW_MILLIPEDE](0x2700, 0x2700).nopw();
	m_lowview[VIEW_MILLIPEDE](0x2780, 0x27bf).nopw();
	m_lowview[VIEW_MILLIPEDE](0x4000, 0x7fff).rom().region("flash", 0x1c000);

	m_lowview[VIEW_MISSILE](0x0000, 0x3fff).rw(FUNC(cmmb_state::missile_ram_r), FUNC(cmmb_state::missile_ram_w));
	m_lowview[VIEW_MISSILE](0x4000, 0x400f).rw(m_pokey, FUNC(pokey_device::read), FUNC(pokey_device::write));
	m_lowview[VIEW_MISSILE](0x4800, 0x4800).rw(FUNC(cmmb_state::missile_4800_r), FUNC(cmmb_state::missile_4800_w));
	m_lowview[VIEW_MISSILE](0x4900, 0x4900).r(FUNC(cmmb_state::missile_in1_r));
	m_lowview[VIEW_MISSILE](0x4a00, 0x4a00).r(FUNC(cmmb_state::missile_r10_r));
	m_lowview[VIEW_MISSILE](0x4b00, 0x4b07).w(FUNC(cmmb_state::missile_palette_w));
	m_lowview[VIEW_MISSILE](0x4c00, 0x4c00).nopw();
	m_lowview[VIEW_MISSILE](0x4d00, 0x4d00).w(FUNC(cmmb_state::irq_ack_w));
	m_lowview[VIEW_MISSILE](0x5000, 0x7fff).rom().region("flash", 0x2d000);

	m_lowview[VIEW_BOWLING](0x0000, 0x1fff).ram().share(m_rambase);
	m_lowview[VIEW_BOWLING](0x1000, 0x13bf).w(FUNC(cmmb_state::bowl_vram_w));
	m_lowview[VIEW_BOWLING](0x1400, 0x140f).rw(m_pokey, FUNC(pokey_device::read), FUNC(pokey_device::write));
	m_lowview[VIEW_BOWLING](0x1500, 0x1503).nopw();
	m_lowview[VIEW_BOWLING](0x1600, 0x1600).r(FUNC(cmmb_state::bowl_track_r<1>));
	m_lowview[VIEW_BOWLING](0x1700, 0x1700).r(FUNC(cmmb_state::bowl_track_r<0>));
	m_lowview[VIEW_BOWLING](0x1800, 0x1800).w(FUNC(cmmb_state::irq_ack_w));
	m_lowview[VIEW_BOWLING](0x1900, 0x191f).w(FUNC(cmmb_state::bowl_palette_w));
	m_lowview[VIEW_BOWLING](0x2000, 0x7fff).rom().region("flash", 0x3a000);

	m_lowview[VIEW_FLASH](0x0000, 0x1fff).ram().share(m_rambase);
	m_lowview[VIEW_FLASH](0x2000, 0x7fff).rw(FUNC(cmmb_state::flash_r<0x2000>), FUNC(cmmb_state::flash_w<0x2000>));

	map(0xa000, 0xafff).ram();
	map(0xb000, 0xbfff).ram().w(FUNC(cmmb_state::charram_w)).share(m_charram);

	map(0xc000, 0xc000).rw(FUNC(cmmb_state::coin_latch_r), FUNC(cmmb_state::coin_latch_clear_w));
	map(0xc001, 0xc001).rw(FUNC(cmmb_state::unk_c001_r), FUNC(cmmb_state::unk_c001_w));
	map(0xc002, 0xc002).rw(FUNC(cmmb_state::sysctrl_r), FUNC(cmmb_state::sysctrl_w));
	map(0xc003, 0xc003).rw(FUNC(cmmb_state::game_select_r), FUNC(cmmb_state::game_select_w));
	map(0xc005, 0xc008).rw(FUNC(cmmb_state::soft_dip_r), FUNC(cmmb_state::soft_dip_w));
	map(0xc009, 0xc009).rw(FUNC(cmmb_state::sound_gate_r), FUNC(cmmb_state::sound_gate_w));
	map(0xc00e, 0xc00e).r(FUNC(cmmb_state::buttons_r));
	map(0xc00f, 0xc00f).r(FUNC(cmmb_state::system_r));

	map(0xc010, 0xffff).rom().region("flash", 0x0c010);
}

void cmmb_state::machine_start()
{
	save_item(NAME(m_unk_c001));
	save_item(NAME(m_sysctrl));
	save_item(NAME(m_game_select));
	save_item(NAME(m_sound_gate));
	save_item(NAME(m_soft_dip));
	save_item(NAME(m_outlatch));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_track_oldpos));
	save_item(NAME(m_track_sign));
	save_item(NAME(m_track_synced));
	save_item(NAME(m_track_dir));
	save_item(NAME(m_track_clk));
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_missile_ctrld));
	save_item(NAME(m_dsw_select));
	save_item(NAME(m_coin_latched));
	save_item(NAME(m_coin_prev));
	save_item(NAME(m_madsel_armed));
	save_item(NAME(m_madsel_skips));
	save_item(NAME(m_last_sync_op));
}

void cmmb_state::machine_reset()
{
	m_unk_c001 = 0;
	m_sysctrl = 0;
	m_game_select = 0x81;
	m_sound_gate = 0;
	m_outlatch = 0;
	m_flipscreen = 0;
	m_missile_ctrld = 0;
	m_dsw_select = false;
	m_coin_latched = 0;
	m_coin_prev = 0;
	m_madsel_armed = false;
	m_madsel_skips = 0;
	m_last_sync_op = 0;
	m_irq_state = CLEAR_LINE;

	std::fill(std::begin(m_track_oldpos), std::end(m_track_oldpos), 0);
	std::fill(std::begin(m_track_sign), std::end(m_track_sign), 0);
	std::fill(std::begin(m_track_synced), std::end(m_track_synced), false);
	std::fill(std::begin(m_track_dir), std::end(m_track_dir), 1);
	std::fill(std::begin(m_track_clk), std::end(m_track_clk), 1);

	m_maincpu->set_input_line(m6502_device::IRQ_LINE, CLEAR_LINE);

	update_lowview();
}

static INPUT_PORTS_START( cmmb )
	PORT_START("COINS")
	PORT_BIT( 0x3f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Action 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Action 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Action 3")
	PORT_BIT( 0x38, IP_ACTIVE_LOW, IPT_UNUSED ) // P2 fire 1-3 (cocktail)
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED ) // CPLD game-ID (c00e_r)

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Service Credit")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x80, IP_ACTIVE_LOW )

	// Physical DIP functions unknown.  The game DIPs were replaced by menu settings
	// via the CPLD.
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, "DIP 1" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "DIP 2" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "DIP 3" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "DIP 4" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "DIP 5" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "DIP 6" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "DIP 7" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "DIP 8" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("TRACKBALL_0")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_REVERSE

	PORT_START("TRACKBALL_1")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10)
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8, 8,
	RGN_FRAC(1, 1),
	2,
	{ 1, 0 },
	{ STEP4(6, -2), STEP4(14, -2) },
	{ STEP8(0, 16) },
	8 * 16
};

static const gfx_layout spritelayout =
{
	8, 16,
	RGN_FRAC(1, 1),
	2,
	{ 1, 0 },
	{ STEP4(6, -2), STEP4(14, -2) },
	{ STEP16(0, 16) },
	8 * 32
};

static const gfx_layout cent_charlayout =
{
	8, 8,
	RGN_FRAC(1, 1),
	2,
	{ 0, 1 },
	{ STEP4(6, -2), STEP4(14, -2) },
	{ STEP8(0, 16) },
	8 * 16
};

static const gfx_layout cent_spritelayout =
{
	8, 16,
	RGN_FRAC(1, 1),
	2,
	{ 0, 1 },
	{ STEP4(6, -2), STEP4(14, -2) },
	{ STEP16(0, 16) },
	8 * 32
};

static const gfx_layout bowl_spritelayout =
{
	16, 16,
	RGN_FRAC(1, 2),
	2,
	{ 1, 0 },
	{ STEP16(30, -2) },
	{ STEP16(0, 32) },
	16 * 32
};

static GFXDECODE_START( gfx_cmmb )
	GFXDECODE_RAM( "charram", 0x000, charlayout,        0x00, 4 )                 // menu / Millipede / bowling tiles
	GFXDECODE_RAM( "charram", 0x000, spritelayout,      0x10, 4 * 4 * 4 * 4 )     // menu / Millipede sprites
	GFXDECODE_RAM( "charram", 0x000, cent_charlayout,   0x00, 4 )                 // Centipede tiles
	GFXDECODE_RAM( "charram", 0x000, cent_spritelayout, 0x10, 4 * 4 * 4 * 4 )     // Centipede sprites
	GFXDECODE_RAM( "charram", 0x800, bowl_spritelayout, 0x10, 4 * 4 * 4 * 4 )     // bowling sprites
GFXDECODE_END

void cmmb_state::cmmb(machine_config &config)
{
	W65C02S(config, m_maincpu, MAIN_CLOCK / 5); // rated 14 MHz; exact divisor unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &cmmb_state::trampoline_map);

	ADDRESS_MAP_BANK(config, m_mainmap);
	m_mainmap->set_options(ENDIANNESS_LITTLE, 8, 16);
	m_mainmap->set_map(&cmmb_state::cmmb_map);

	screen_device &screen(SCREEN(config, "screen"));
	screen.set_raw(MAIN_CLOCK/12, 384, 0, 256, 272, 0, 240); // TBD, not real measurements
	screen.set_screen_update(FUNC(cmmb_state::screen_update));
	screen.set_palette(m_palette);

	TIMER(config, "irq").configure_scanline(FUNC(cmmb_state::cb_irq), "screen", 0, 16);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_cmmb);

	PALETTE(config, m_palette).set_entries(1040);

	SPEAKER(config, "mono").front_center();
	POKEY(config, m_pokey, MAIN_CLOCK / 6 / 8);
	m_pokey->allpot_r().set(FUNC(cmmb_state::soft_dip_pot_r<1>));
	m_pokey->add_route(ALL_OUTPUTS, "mono", 0.5);
	POKEY(config, m_pokey2, MAIN_CLOCK / 6 / 8);
	m_pokey2->allpot_r().set(FUNC(cmmb_state::soft_dip_pot_r<2>));
	m_pokey2->add_route(ALL_OUTPUTS, "mono", 0.5);
}

void cmmb_state::cmmb103(machine_config &config)
{
	cmmb(config);
	SST_39SF020(config, m_flash);
}

void cmmb_state::cmmb162(machine_config &config)
{
	cmmb(config);
	ATMEL_29C020(config, m_flash);
}

ROM_START( cmmb162 )
	ROM_REGION( 0x40000, "flash", 0 )
	ROM_LOAD( "27c020.u2", 0x00000, 0x40000, CRC(a3ddc739) SHA1(17a72fb60fbc190fc459e8b55e8e38320f8bceeb) )
ROM_END

ROM_START( cmmb103 )
	ROM_REGION( 0x40000, "flash", 0 )
	ROM_LOAD( "cmmb103.u2", 0x00000, 0x40000, CRC(58b873d1) SHA1(aa4596006861596452ac224b65de4a1ac2e52a8d) )
ROM_END

} // anonymous namespace


GAME( 2001, cmmb103, 0, cmmb103, cmmb, cmmb_state, empty_init, ROT270, "Cosmodog / Team Play (licensed from Infogrames via Midway Games West)", "Centipede / Millipede / Missile Command (rev 1.03)", MACHINE_SUPPORTS_SAVE )
GAME( 2002, cmmb162, 0, cmmb162, cmmb, cmmb_state, empty_init, ROT270, "Cosmodog / Team Play (licensed from Infogrames via Midway Games West)", "Centipede / Millipede / Missile Command / Let's Go Bowling (rev 1.62)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
