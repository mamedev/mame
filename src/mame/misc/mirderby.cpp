// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

Miracle Derby - Ascot

TODO:
- complete sprites;
- NVRAM;
- Pinpoint actual NMI sources;
- Game pukes if more than one key is pressed, and MAME input defaults for 2p side
  clashes with the remapped p1 keys.
  For now user has to workaround by mapping p2 keys manually,
  actual fix would be to define a horse betting layout in MAME input defs.
- Missing inputs, namely 10 and 100 Yen tokens. These comes from the Z80 x70coin subsystem.

Old Haze note:
- has the same GX61A01 custom (blitter?) as homedata.cpp and a 'similar' CPU setup
  (this has more CPUs) and similar board / rom numbering (X**-)

The drivers can probably be merged later, although the current per-game handling of the blitter in
homedata.cpp should be looked at.
Update: GX61A01 looks more of a CRTC, also note that "blitter" ROM spot at 12e on homedata PCBs is
actually a CPU. Is this a bootleg of an Home Data original?

===================================================================================================

Notes from Stefan Lindberg:

Eprom "x70_a04.5g" had wires attached to it, pin 2 and 16 was joined and pin 1,32,31,30 was joined,
i removed them and read the eprom as the type it was (D27c1000D).

Measured frequencies:
MBL68B09E = 2mhz
MBL68B09E = 2mhz
z80 = 4mhz
YM2203 = 2mhz

See included PCB pics.
- uPD8253C-2 + uPD8255AC-2 near Z80
- MB3730A audio power amplifier
- Partially surface scratched GX61A01, "HOME DATA" pattern still kinda readable anyway (lol)
- CXK5816PN-12L CMOS
- unpopulated 2.5MHz + various "option" slots
- two empty sockets, one near the Z80 (dev option?), another near A03 horse GFXs
- several jumpers across the board

Roms:

Name              Size     CRC32         Chip Type
---------------------------------------------------------------------------------
x70a07.8l         256      0x7d4c9712    82s129
x70a08.7l         256      0xc4e77174    82s129
x70a09.6l         256      0xd0187957    82s129
x70_a03.8g        32768    0x4e298b2d    27c256
x70_a04.5g        131072   0x14392fdb    D27c1000D
x70_a11.1g        32768    0xb394eef7    27c256
x70_b02.12e       32768    0x76c9bb6f    27c256
x70_c01.14e       65536    0xd79d072d    27c512


**************************************************************************************************/

#include "emu.h"
//#include "homedata.h"

#include "cpu/m6809/m6809.h"
#include "cpu/upd7810/upd7810.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/i8255.h"
#include "machine/nvram.h"
#include "machine/pit8253.h"
//#include "sound/dac.h"
//#include "sound/sn76496.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

class mirderby_state : public driver_device
{
public:
	mirderby_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_subcpu(*this, "subcpu")
		, m_x70coincpu(*this, "audiocpu")
		, m_ymsnd(*this, "ymsnd")
//      , m_vreg(*this, "vreg")
		, m_screen(*this, "screen")
		, m_videoram(*this, "videoram")
		, m_spriteram(*this, "spriteram")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_subbank(*this, "subbank")
		, m_coinlatch(*this, "coinlatch")
		, m_keys(*this, "KEY%u", 0U)
		, m_in(*this, "IN%u", 0U)
		, m_coin_ppi(*this, "coin_ppi")
		, m_coin_pit(*this, "pit")
	{
	}

	void mirderby(machine_config &config);

private:
//  optional_region_ptr<uint8_t> m_blit_rom;

	required_device<mc6809e_device> m_maincpu;
	required_device<mc6809e_device> m_subcpu;
	required_device<cpu_device> m_x70coincpu;
	optional_device<ym2203_device> m_ymsnd;
//  optional_shared_ptr<uint8_t> m_vreg;
	required_device<screen_device> m_screen;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_memory_bank m_subbank;
	optional_device<generic_latch_8_device> m_coinlatch;

	required_ioport_array<5 * 2> m_keys;
	required_ioport_array<2> m_in;
	required_device<i8255_device> m_coin_ppi;
	required_device<pit8253_device> m_coin_pit;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void screen_vblank(int state);
	virtual void video_start() override ATTR_COLD;

	uint8_t prot_r();
	void prot_w(uint8_t data);
	void palette_init(palette_device &palette) const;
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	void videoram_w(offs_t offset, u8 data);

	void main_map(address_map &map) ATTR_COLD;
	void sub_map(address_map &map) ATTR_COLD;

	void shared_map(address_map &map) ATTR_COLD;

	void x70coin_map(address_map &map) ATTR_COLD;
	void x70coin_io(address_map &map) ATTR_COLD;

	tilemap_t *m_bg_tilemap{};
//  int m_visible_page = 0;
//  int m_priority = 0;
//  [[maybe_unused]] int m_flipscreen = 0;
	u8 m_prot_data = 0;
	u8 m_latch = 0;
	u16 m_gfx_flip = 0;
	u16 m_gfx_bank = 0;
	bool m_main_irq_enable = false;
	bool m_main_nmi_enable = false;
	bool m_sub_irq_enable = false;
	bool m_sub_nmi_enable = false;

	u8 m_key_matrix = 0;
	u16 m_scrollx = 0;
	bool m_scrollx_enable = false;
};

void mirderby_state::palette_init(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();

	for (int i = 0; i < 0x100; i++)
	{
		int const r = color_prom[0x000 + i];
		int const g = color_prom[0x100 + i];
		int const b = color_prom[0x200 + i];

		palette.set_pen_color(i, pal4bit(r), pal4bit(g), pal4bit(b));
	}
}

void mirderby_state::videoram_w(offs_t offset, u8 data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty((offset & 0xffe) >> 1);
}

void mirderby_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(mirderby_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

//  m_bg_tilemap->set_transparent_pen(0);
}

TILE_GET_INFO_MEMBER(mirderby_state::get_bg_tile_info)
{
	int const addr  = tile_index * 2;
	int const attr  = m_videoram[addr];
	int const code  = m_videoram[addr + 1] + ((attr & 0x03) << 8) + m_gfx_bank + m_gfx_flip;
	int const color = (attr >> 4) & 0xf;

	tileinfo.category = BIT(attr, 3);

	tileinfo.set(1, code, color, 0 );
}

void mirderby_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const u8 *source = m_spriteram;
	const u8 * const finish = m_spriteram + m_spriteram.length();

	gfx_element * const gfx = m_gfxdecode->gfx(0);

	// TODO: gets corrupted during gameplay, moving entries with an offset of $-2
	for (; source < finish; source += 4 )
	{
		const u8 tile = source[ 2 ];
		// 0xc0 pinballs (0x300)
		// 0xdf opaque box on title (0x36e)
		// 0xd5 lower start gate (0x340 - 0x360)
		// 0x37 jockey
		u16 code    = (tile & 0xf8) << 2 | (tile & 0x07) << 1;
		u8 attr     = source[ 1 ];
		// TODO: bit 5 can't be flipx, it would cross top row horses on purple bet screen
		// more like an internal copy they disabled with this?
		int sx      = source[ 3 ] + ((attr & 0x30) << 4);
		int sy      = (0xf1 - source[ 0 ]) & 0xff;

		u8 color = attr & 0xf;

		// TODO: missing sprites (signed wraparound?)

		//if (attr == 0)
		//  continue;

		// draws in block strips of 16x16
		const u8 tile_offs[4] = { 0, 1, 0x10, 0x11 };
		const int draw_x[4] = { -4, 4, -4, 4 };
		const int draw_y[4] = { 0, 0, 8, 8 };

		for (int block_i = 0; block_i < 4; block_i ++)
		{
			gfx->transpen(bitmap, cliprect,
				code + tile_offs[block_i], color,
				0, 0,
				sx + draw_x[block_i],
				sy + draw_y[block_i],
				0);
		}
	}
}

uint32_t mirderby_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);

	if (m_scrollx_enable)
	{
		// category 1 is for scrolling elements, probably high priority wrt sprites too
		m_bg_tilemap->set_scrollx(0, m_scrollx & 0x1ff);
		m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_CATEGORY(0), 0);

		m_bg_tilemap->set_scrollx(0, 0);
		m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_CATEGORY(0), 0);

		m_bg_tilemap->set_scrollx(0, m_scrollx & 0x1ff);
		m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_CATEGORY(1), 0);

		draw_sprites(bitmap, cliprect);
	}
	else
	{
		// result screen doesn't explicitly reset the scroll reg but instead
		// just writes a 0x7c to $7ffa
		m_bg_tilemap->set_scrollx(0, 0);
		m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_CATEGORY(0), 0);
		draw_sprites(bitmap, cliprect);
		m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_CATEGORY(1), 0);
	}

	return 0;
}

void mirderby_state::screen_vblank(int state)
{
	if (state)
	{
		// TODO: each irq routine pings a bit of $8000 for masking/acknowledge
		// FIRQ seems valid for main CPU, but cannot be triggered
		if (m_main_irq_enable)
			m_maincpu->set_input_line(M6809_IRQ_LINE, HOLD_LINE);
		// FIRQ and IRQ same for sub CPU
		if (m_sub_irq_enable)
			m_subcpu->set_input_line(M6809_FIRQ_LINE, HOLD_LINE);
	}
}


// protection check? sound comms? video beam sync?
uint8_t mirderby_state::prot_r()
{
	m_prot_data&=0x7f;
	return m_prot_data++;
}

void mirderby_state::prot_w(uint8_t data)
{
	m_prot_data = data;
}


void mirderby_state::shared_map(address_map &map)
{
	map(0x0000, 0x0fff).ram().share(m_spriteram);
	map(0x1000, 0x1fff).ram().w(FUNC(mirderby_state::videoram_w)).share(m_videoram);
	map(0x2000, 0x6fff).ram().share("share");
	map(0x7000, 0x77ff).ram().share("nvram");
	map(0x7800, 0x7800).rw(FUNC(mirderby_state::prot_r), FUNC(mirderby_state::prot_w));
	//0x7ff0 onward seems CRTC
//  map(0x7ff0, 0x7ff?).writeonly().share("vreg");
	map(0x7ff2, 0x7ff2).portr("SYSTEM");
	map(0x7ff9, 0x7ffa).lr8(
		NAME([this] (offs_t offset) {
			u8 res = 0x3f | (m_in[offset]->read() & 0xc0);

			// tests KEY1-KEY4 then uses 0x1e
			// read by main in service mode, by sub in gameplay
			for (int i = 0; i < 5; i++)
			{
				if (BIT(m_key_matrix, i))
					res &= m_keys[i + offset * 5]->read() & 0xff;
			}

			return res;
		})
	);
	map(0x7ffa, 0x7ffa).lw8(
		NAME([this] (u8 data) {
			// TODO: other bits, hardly used so complete guess (0xb0 vs. 0x7c)
			m_scrollx_enable = bool(BIT(data, 7));
		})
	);
	map(0x7ffb, 0x7ffb).lr8(
		NAME([this] () {
			u8 res = m_coinlatch->read();
			//m_coinlatch->clear_w();
			// bit 4 will disable bet inputs if on, coin chute anti-tamper?
			return res & 0xef;
		})
	);
	map(0x7ffb, 0x7ffc).lw8(
		NAME([this] (offs_t offset, u8 data) {
			// offset 0 bits 4-6 seems fractional scroll,
			// but it first gets set up then quickly wiped by CLRA back to back
			// (intentionally disabled? flip flop?)
			// the working register gets written twice as well ...
			if (offset & 1)
				m_scrollx = data << 1;
		})
	);
	map(0x7ffe, 0x7ffe).nopr(); //watchdog?
	map(0x7ffe, 0x7ffe).lw8(
		NAME([this] (u8 data) {
			// bit 2 sound latch
			// bit 1-0 chip select?
			m_latch = data;
			if (data & 0xf8)
				logerror("latch write %02x\n", data);
		})
	);
	map(0x7fff, 0x7fff).lrw8(
		NAME([this] () {
			//  0x7fff $e / $f writes -> DSW reads
			return m_ymsnd->read(1);
		}),
		NAME([this] (u8 data) {
			//logerror("%s -> %02x\n", BIT(m_latch, 2) ? "address" : "data", data);
			m_ymsnd->write(BIT(m_latch, 2) ? 0 : 1, data);
		})
	);
}

void mirderby_state::main_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).m(*this, FUNC(mirderby_state::shared_map));
	map(0x7ffd, 0x7ffd).lw8(
		NAME([this] (u8 data) {
			// TODO: bit 7 (after POST)
			// TODO: cannot be NMI
			if (m_sub_nmi_enable)
				m_subcpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
			//logerror("%02x latch write\n", data);
			m_key_matrix = data;
		})
	);
	map(0x8000, 0xffff).rom().region("main_rom", 0);
	map(0x8000, 0x8000).lw8(
		NAME([this] (u8 data) {
			// TODO: bit 5 (coin counter), bit 4 (enabled after first coin in)
			m_main_nmi_enable = bool(BIT(data, 1));
			m_main_irq_enable = bool(BIT(data, 0));
		})
	);
}

void mirderby_state::sub_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).m(*this, FUNC(mirderby_state::shared_map));
	map(0x7ffd, 0x7ffd).lw8(
		NAME([this] (u8 data) {
			if (m_main_nmi_enable)
				m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
			m_key_matrix = data;
		})
	);
	map(0x7ffe, 0x7ffe).nopr(); //watchdog?
	map(0x8000, 0xffff).bankr(m_subbank);
	map(0x8000, 0x8000).lw8(
		NAME([this] (u8 data) {
			m_subbank->set_entry(BIT(data, 7) ? 1 : 0);
			const u16 new_gfx_flip = BIT(data, 5) ? 0x800 : 0;
			const u16 new_gfx_bank = BIT(data, 2) ? 0x400 : 0;
			if (new_gfx_flip != m_gfx_flip || new_gfx_bank != m_gfx_bank)
			{
				m_gfx_flip = new_gfx_flip;
				m_gfx_bank = new_gfx_bank;
				m_bg_tilemap->mark_all_dirty();
			}

			// TODO: bit 3 used
			m_sub_nmi_enable = bool(BIT(data, 1));
			m_sub_irq_enable = bool(BIT(data, 0));
		})
	);
}

void mirderby_state::x70coin_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("x70coin_rom", 0);
	map(0x8000, 0x87ff).ram();
}

void mirderby_state::x70coin_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw(m_coin_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x04, 0x07).rw(m_coin_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write));
}

static GFXDECODE_START( gfx_mirderby )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x4_packed_msb, 0x0000, 0x10 )
	GFXDECODE_ENTRY( "gfx2", 0, gfx_8x8x4_packed_msb, 0x0000, 0x10 )
GFXDECODE_END

static INPUT_PORTS_START( mirderby )
	PORT_START("SYSTEM")
	PORT_DIPNAME( 0x01, 0x01, "SYSTEM" )
	PORT_DIPSETTING(      0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )

	// bit 6-7 of key matrix, common for all the ports
	PORT_START("IN0")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 ) // Medal
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE2 ) // Credit Clear

	PORT_START("IN1")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 ) // analyzer
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) // hangs game if triggered, break from external debugger?

	// p1 side
	// TODO: sketchy layout, derived from kingdrby.cpp
	PORT_START("KEY0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON15 ) PORT_NAME("1P 5-6") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("1P 1-3") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("1P 2-3") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("1P 3-4") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE ) PORT_NAME("1P Take Score")
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON14 ) PORT_NAME("1P 4-6") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("1P 1-2") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("1P 1-6") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("1P 2-6") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON13 ) PORT_NAME("1P 4-5") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("1P 1-4") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("1P 2-4") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_NAME("1P 3-5") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP ) PORT_NAME("1P Double Up")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON16 ) PORT_NAME("1P Screen Change")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("1P 1-5") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("1P 2-5") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON12 ) PORT_NAME("1P 3-6") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) PORT_NAME("1P Payout")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_NAME("1P Flip Flop")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	// p2 side, with a limited subset of keys (!?)
	PORT_START("KEY5")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY6")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY7")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY8")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("2P Screen Change") PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("2P 1-5") PORT_CODE(KEYCODE_R) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("2P 2-5") PORT_CODE(KEYCODE_D) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("2P 3-6") PORT_CODE(KEYCODE_Z) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) PORT_NAME("2P Payout") PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_NAME("2P Flip Flop") PORT_PLAYER(2)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	// "henkyaku ritsu" in analyzer
	PORT_DIPNAME( 0x07, 0x07, "Payout Rate" ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x00, "95%" )
	PORT_DIPSETTING(      0x01, "90%" )
	PORT_DIPSETTING(      0x02, "85%" )
	PORT_DIPSETTING(      0x03, "80%" )
	PORT_DIPSETTING(      0x04, "75%" )
	PORT_DIPSETTING(      0x05, "70%" )
	PORT_DIPSETTING(      0x06, "65%" )
	PORT_DIPSETTING(      0x07, "60%" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	// Applies to Medal
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x00, "1 Coin / 100 Credits" )
	PORT_DIPSETTING(      0x01, "1 Coin / 50 Credits" )
	PORT_DIPSETTING(      0x02, "1 Coin / 20 Credits" )
	PORT_DIPSETTING(      0x03, "1 Coin / 10 Credits" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )

	PORT_START("SUB_COIN0")
	// detected coin weight, currently hardwired at 100 Yen (cfr. analyzer),
	// with a bit of input hold
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SUB_COIN1")
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

void mirderby_state::machine_start()
{
	m_subbank->configure_entries(0, 2, memregion("sub_rom")->base(), 0x8000);
}

void mirderby_state::machine_reset()
{
	m_subbank->set_entry(0);
	m_gfx_flip = m_gfx_bank = 0;
	m_main_irq_enable = m_main_nmi_enable = false;
	m_sub_irq_enable = m_sub_nmi_enable = false;
}

/* clocks are 16mhz and 9mhz */
void mirderby_state::mirderby(machine_config &config)
{
	/* basic machine hardware */
	MC6809E(config, m_maincpu, 16000000/8);  /* MBL68B09E 2 Mhz */
	m_maincpu->set_addrmap(AS_PROGRAM, &mirderby_state::main_map);
//  m_maincpu->set_vblank_int("screen", FUNC(mirderby_state::homedata_irq));

	MC6809E(config, m_subcpu, 16000000/8); /* MBL68B09E 2 Mhz */
	m_subcpu->set_addrmap(AS_PROGRAM, &mirderby_state::sub_map);
//  m_subcpu->set_vblank_int("screen", FUNC(mirderby_state::homedata_irq));

	// im 0, doesn't bother in setting a vector table,
	// should just require a NMI from somewhere ...
	Z80(config, m_x70coincpu, XTAL(16'000'000)/4);   /* 4 Mhz */
	m_x70coincpu->set_addrmap(AS_PROGRAM, &mirderby_state::x70coin_map);
	m_x70coincpu->set_addrmap(AS_IO, &mirderby_state::x70coin_io);

	GENERIC_LATCH_8(config, m_coinlatch);

	I8255A(config, m_coin_ppi);
	m_coin_ppi->in_pa_callback().set_ioport("SUB_COIN0");
	m_coin_ppi->out_pb_callback().set(m_coinlatch, FUNC(generic_latch_8_device::write));
	m_coin_ppi->in_pc_callback().set_ioport("SUB_COIN1");

	PIT8253(config, m_coin_pit, 0);
	m_coin_pit->set_clk<0>(XTAL(16'000'000) / 8);
	m_coin_pit->out_handler<0>().set_inputline(m_x70coincpu, INPUT_LINE_NMI);
//  m_coin_pit->set_clk<1>(XTAL(16'000'000) / 8);
//  m_coin_pit->set_clk<2>(XTAL(16'000'000) / 8);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);
//  config.set_maximum_quantum(attotime::from_hz(6000));
	config.set_perfect_quantum("maincpu");

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(59);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(1*8, 50*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(mirderby_state::screen_update));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set(FUNC(mirderby_state::screen_vblank));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_mirderby);
	PALETTE(config, m_palette, FUNC(mirderby_state::palette_init), 0x100);

	SPEAKER(config, "speaker").front_center();

	YM2203(config, m_ymsnd, 2'000'000);
	m_ymsnd->port_a_read_callback().set_ioport("DSW1");
	m_ymsnd->port_b_read_callback().set_ioport("DSW2");
	m_ymsnd->add_route(0, "speaker", 0.25);
	m_ymsnd->add_route(1, "speaker", 0.25);
	m_ymsnd->add_route(2, "speaker", 0.25);
	m_ymsnd->add_route(3, "speaker", 1.0);
}

ROM_START( mirderby )
	ROM_REGION( 0x8000, "main_rom", 0 ) // M6809 code
	ROM_LOAD( "x70_b02.12e", 0x0000, 0x8000, CRC(76c9bb6f) SHA1(dd8893f3082d33d366247295e9531f8879c219c5) )

	ROM_REGION( 0x10000, "sub_rom", 0 ) // M6809 code
	ROM_LOAD( "x70_c01.14e", 0x00000, 0x10000, CRC(d79d072d) SHA1(8e189931de9c4eb520c1ec2d0898d8eaba0f01b5) )

	ROM_REGION( 0x2000, "x70coin_rom", 0 ) // Z80 Code
	ROM_LOAD( "x70_a11.1g", 0x0000, 0x2000, CRC(b394eef7) SHA1(a646596d09b90eda44aaf8ccbf8f3fccfd3d5dad) ) // first 0x6000 bytes are blank!
	ROM_CONTINUE(0x0000, 0x2000)
	ROM_CONTINUE(0x0000, 0x2000)
	ROM_CONTINUE(0x0000, 0x2000) // main z80 code is here

	ROM_REGION( 0x8000, "gfx1", 0 ) // horse gfx
	ROM_LOAD( "x70_a03.8g", 0x0000, 0x8000, CRC(4e298b2d) SHA1(ae78327d1f30c8d19ef772b82803dab4d6b7b919))

	ROM_REGION( 0x20000, "gfx2", 0 ) // fonts etc.
	ROM_LOAD( "x70_a04.5g", 0x0000, 0x20000, CRC(14392fdb) SHA1(dafdce473b2d2ebbdbf49fbd12f85c1ad69b2877) )

	ROM_REGION( 0x300, "proms", 0 ) // colours
	ROM_LOAD( "x70a07.8l", 0x000, 0x100, CRC(7d4c9712) SHA1(fe2a89841fdf5e4fd6cd41478ad2f29d28bed54d) )
	ROM_LOAD( "x70a08.7l", 0x100, 0x100, CRC(c4e77174) SHA1(ada238ded69f01b4daeb0159a2c5c422977bb95e) )
	ROM_LOAD( "x70a09.6l", 0x200, 0x100, CRC(d0187957) SHA1(6b36c1bccad24708cfa2fc78da08313f9bcfdbc0) )
ROM_END

GAME( 1988, mirderby,  0, mirderby, mirderby, mirderby_state, empty_init, ROT0, "Home Data / Ascot", "Miracle Derby (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
