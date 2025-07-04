// license:BSD-3-Clause
// copyright-holders: Manuel Abadia, David Haywood

/***************************************************************************

Glass (c) 1993 Gaelco (Developed by OMK. Produced by Gaelco)

Driver by Manuel Abadia <emumanu+mame@gmail.com>

Todo:
 - video priorities are incorrect, like most earlier Gaelco titles in MAME

***************************************************************************/

#include "emu.h"

#include "gaelco_ds5002fp.h"

#include "cpu/m68000/m68000.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/74259.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class glass_state : public driver_device
{
public:
	glass_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_outlatch(*this, "outlatch"),
		m_videoram(*this, "videoram"),
		m_vregs(*this, "vregs"),
		m_spriteram(*this, "spriteram"),
		m_shareram(*this, "shareram"),
		m_bmap(*this, "bmap"),
		m_okibank(*this, "okibank"),
		m_pant{ nullptr, nullptr },
		m_blitter_command(0)
	{ }

	void glass(machine_config &config);
	void glass_ds5002fp(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// devices
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<ls259_device> m_outlatch;

	// memory pointers
	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_vregs;
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_shareram;
	required_region_ptr<uint8_t>  m_bmap;

	required_memory_bank m_okibank;

	// video-related
	tilemap_t *m_pant[2]{};
	std::unique_ptr<bitmap_ind16> m_screen_bitmap;

	// misc
	uint8_t m_current_bit = 0;
	uint8_t m_blitter_command = 0;

	void shareram_w(offs_t offset, uint8_t data);
	uint8_t shareram_r(offs_t offset);
	void clr_int_w(uint8_t data);
	void oki_bankswitch_w(uint8_t data);
	void coin_w(offs_t offset, uint16_t data);
	void blitter_w(uint16_t data);
	void vram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	template <uint8_t Which> void coin_lockout_w(int state);
	template <uint8_t Which> void coin_counter_w(int state);

	template<int Layer> TILE_GET_INFO_MEMBER(get_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void main_map(address_map &map) ATTR_COLD;
	void mcu_hostmem_map(address_map &map) ATTR_COLD;
	void oki_map(address_map &map) ATTR_COLD;
};


/***************************************************************************

    Callbacks for the TileMap code

***************************************************************************/

/*
    Tile format
    -----------

    Screen 0 & 1: (32*32, 16x16 tiles)

    Word | Bit(s)            | Description
    -----+-FEDCBA98-76543210-+--------------------------
      0  | xxxxxxxx xxxxxxxx | code
      1  | -------- ---xxxxx | color (uses colors 0x200-0x3ff)
      1  | -------- --x----- | not used?
      1  | -------- -x------ | flip x
      1  | -------- x------- | flip y
      1  | xxxxxxxx -------- | not used
*/

template<int Layer>
TILE_GET_INFO_MEMBER(glass_state::get_tile_info)
{
	int const data = m_videoram[(Layer * 0x1000 / 2) + (tile_index << 1)];
	int const data2 = m_videoram[(Layer * 0x1000 / 2) + (tile_index << 1) + 1];
	int const code = ((data & 0x03) << 14) | ((data & 0x0fffc) >> 2);

	tileinfo.set(0, code, 0x20 + (data2 & 0x1f), TILE_FLIPYX((data2 & 0xc0) >> 6));
}

/***************************************************************************

    Blitter

***************************************************************************/

/*
    The blitter is accessed writing 5 consecutive bits. The stream is: P0 P1 B2 B1 B0

    if P0 is set, the hardware selects the first half of ROM H9 (girls)
    if P1 is set, the hardware selects the second half of ROM H9 (boys)

    B2B1B0 selects the picture (there are 8 pictures in each half of the ROM)
*/

void glass_state::blitter_w(uint16_t data)
{
	m_blitter_command = ((m_blitter_command << 1) | (data & 0x01)) & 0x1f;
	m_current_bit++;

	if (m_current_bit == 5)
	{
		m_current_bit = 0;

		// fill the screen bitmap with the current picture
		uint8_t const *gfx = m_bmap + (m_blitter_command & 0x07) * 0x10000 + (m_blitter_command & 0x08) * 0x10000 + 0x140;

		if (m_blitter_command & 0x18)
		{
			for (int j = 0; j < 200; j++)
			{
				for (int i = 0; i < 320; i++)
					m_screen_bitmap->pix(j, i) = *gfx++;
			}
		}
		else
		{
			m_screen_bitmap->fill(0);
		}
	}
}

/***************************************************************************

    Memory Handlers

***************************************************************************/

void glass_state::vram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_videoram[offset]);
	m_pant[offset >> 11]->mark_tile_dirty(((offset << 1) & 0x0fff) >> 2);
}


/***************************************************************************

    Start/Stop the video hardware emulation.

***************************************************************************/

void glass_state::video_start()
{
	m_pant[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(glass_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_pant[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(glass_state::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_screen_bitmap = std::make_unique<bitmap_ind16>(320, 200);

	save_item(NAME(*m_screen_bitmap));

	m_pant[0]->set_transparent_pen(0);
	m_pant[1]->set_transparent_pen(0);
}


/***************************************************************************

    Sprites

***************************************************************************/

/*
    Sprite Format
    -------------

    Word | Bit(s)            | Description
    -----+-FEDCBA98-76543210-+--------------------------
      0  | -------- xxxxxxxx | y position
      0  | --xxxxxx -------- | not used?
      0  | -x------ -------- | flipx
      0  | x------- -------- | flipy
      1  | xxxxxxxx xxxxxxxx | not used?
      2  | -------x xxxxxxxx | x position
      2  | ---xxxx- -------- | sprite color (uses colors 0x100-0x1ff)
      2  | xx------ -------- | not used?
      3  | xxxxxxxx xxxxxxxx | sprite code
*/

void glass_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	gfx_element *gfx = m_gfxdecode->gfx(0);

	for (int i = 3; i < (0x1000 - 6) / 2; i += 4)
	{
		int const sx = m_spriteram[i + 2] & 0x01ff;
		int const sy = (240 - (m_spriteram[i] & 0x00ff)) & 0x00ff;
		int number = m_spriteram[i + 3];
		int const color = (m_spriteram[i + 2] & 0x1e00) >> 9;
		int const attr = (m_spriteram[i] & 0xfe00) >> 9;

		int const xflip = attr & 0x20;
		int const yflip = attr & 0x40;

		number = ((number & 0x03) << 14) | ((number & 0x0fffc) >> 2);

		gfx->transpen(bitmap, cliprect, number,
				0x10 + (color & 0x0f), xflip, yflip,
				sx - 0x0f, sy, 0);
	}
}

/***************************************************************************

    Display Refresh

****************************************************************************/

uint32_t glass_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// set scroll registers
	m_pant[0]->set_scrolly(0, m_vregs[0]);
	m_pant[0]->set_scrollx(0, m_vregs[1] + 0x04);
	m_pant[1]->set_scrolly(0, m_vregs[2]);
	m_pant[1]->set_scrollx(0, m_vregs[3]);

	// draw layers + sprites
	bitmap.fill(m_palette->black_pen(), cliprect);
	copybitmap(bitmap, *m_screen_bitmap, 0, 0, 0x18, 0x24, cliprect);
	m_pant[1]->draw(screen, bitmap, cliprect, 0, 0);
	m_pant[0]->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}


void glass_state::shareram_w(offs_t offset, uint8_t data)
{
	// why isn't there address map functionality for this?
	reinterpret_cast<uint8_t *>(m_shareram.target())[BYTE_XOR_BE(offset)] = data;
}

uint8_t glass_state::shareram_r(offs_t offset)
{
	// why isn't there address map functionality for this?
	return reinterpret_cast<uint8_t const *>(m_shareram.target())[BYTE_XOR_BE(offset)];
}


void glass_state::clr_int_w(uint8_t data)
{
	m_maincpu->set_input_line(M68K_IRQ_6, CLEAR_LINE);
}


static const gfx_layout glass_tilelayout16 =
{
	16,16,                                  // 16x16 tiles
	RGN_FRAC(1,2),                          // number of tiles
	4,                                      // 4 bpp
	{ RGN_FRAC(1,2)+8, RGN_FRAC(1,2), 8, 0 },
	{ STEP8(0,1), STEP8(8*2*16,1), },
	{ STEP16(0,8*2) },
	8*2*16*2
};

static GFXDECODE_START( gfx_glass )
	GFXDECODE_ENTRY( "gfx", 0x000000, glass_tilelayout16, 0, 64 )
GFXDECODE_END


void glass_state::oki_bankswitch_w(uint8_t data)
{
	m_okibank->set_entry(data & 0x0f);
}

void glass_state::coin_w(offs_t offset, uint16_t data)
{
	m_outlatch->write_bit(offset >> 3, BIT(data, 0));
}

template <uint8_t Which>
void glass_state::coin_lockout_w(int state)
{
	machine().bookkeeping().coin_lockout_w(Which, !state);
}

template <uint8_t Which>
void glass_state::coin_counter_w(int state)
{
	machine().bookkeeping().coin_counter_w(Which, state);
}


void glass_state::mcu_hostmem_map(address_map &map)
{
	map(0x0000, 0xffff).mask(0x3fff).rw(FUNC(glass_state::shareram_r), FUNC(glass_state::shareram_w)); // shared RAM with the main CPU
}


void glass_state::main_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x101fff).ram().w(FUNC(glass_state::vram_w)).share(m_videoram);
	map(0x102000, 0x102fff).ram();                                                                  // Extra Video RAM
	map(0x108000, 0x108007).writeonly().share(m_vregs);
	map(0x108008, 0x108008).w(FUNC(glass_state::clr_int_w));
	map(0x108008, 0x108009).nopr();
	map(0x200000, 0x2007ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x440000, 0x440fff).ram().share(m_spriteram);
	map(0x700000, 0x700001).portr("DSW2");
	map(0x700002, 0x700003).portr("DSW1");
	map(0x700004, 0x700005).portr("P1");
	map(0x700006, 0x700007).portr("P2");
	map(0x700008, 0x700009).w(FUNC(glass_state::blitter_w));
	map(0x70000a, 0x70000b).select(0x000070).w(FUNC(glass_state::coin_w));
	map(0x70000d, 0x70000d).w(FUNC(glass_state::oki_bankswitch_w));
	map(0x70000f, 0x70000f).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xfec000, 0xfeffff).ram().share(m_shareram);                                                // Work RAM (partially shared with DS5002FP)
}


void glass_state::oki_map(address_map &map)
{
	map(0x00000, 0x2ffff).rom();
	map(0x30000, 0x3ffff).bankr(m_okibank);
}


static INPUT_PORTS_START( glass )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:6,7,8")
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:3,4,5")
	PORT_DIPSETTING(    0x10, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x40, 0x40, "Credit configuration" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x40, "Start 1C" )
	PORT_DIPSETTING(    0x00, "Start 2C" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Version ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Light" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:2" ) // Listed as "Unused"
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW2:1" )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( ssplash10 )
	PORT_INCLUDE( glass )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x60, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, "1 Coin / 1 Credit (duplicate 1)" )
	PORT_DIPSETTING(    0xc0, "1 Coin / 1 Credit (duplicate 2)" )
	PORT_DIPSETTING(    0xa0, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(    0x06, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, "1 Coin / 1 Credit (duplicate 1)" )
	PORT_DIPSETTING(    0x0c, "1 Coin / 1 Credit (duplicate 2)" )
	PORT_DIPSETTING(    0x0a, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x10, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW2:6" ) // Listed as "Unused"
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Version ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Light" )
	PORT_SERVICE_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW2:8" )
INPUT_PORTS_END



void glass_state::machine_start()
{
	m_okibank->configure_entries(0, 16, memregion("oki")->base(), 0x10000);

	save_item(NAME(m_current_bit));
	save_item(NAME(m_blitter_command));
}

void glass_state::machine_reset()
{
	m_current_bit = 0;
	m_blitter_command = 0;
}

void glass_state::glass(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, XTAL(24'000'000) / 2);      // 12 MHz verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &glass_state::main_map);

	LS259(config, m_outlatch);
	m_outlatch->q_out_cb<0>().set(FUNC(glass_state::coin_lockout_w<0>));
	m_outlatch->q_out_cb<1>().set(FUNC(glass_state::coin_lockout_w<1>));
	m_outlatch->q_out_cb<2>().set(FUNC(glass_state::coin_counter_w<0>));
	m_outlatch->q_out_cb<3>().set(FUNC(glass_state::coin_counter_w<1>));
	m_outlatch->q_out_cb<4>().set_nop(); // Sound Muting (if bit 0 == 1, sound output stream = 0)

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(57.42); // see note in gaelco.cpp
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size(32*16, 32*16);
	screen.set_visarea(0, 368-1, 16, 256-1);
	screen.set_screen_update(FUNC(glass_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set_inputline(m_maincpu, M68K_IRQ_6, ASSERT_LINE);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_glass);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 1024);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	okim6295_device &oki(OKIM6295(config, "oki", XTAL(1'000'000), okim6295_device::PIN7_HIGH)); // 1MHz Resonator & pin 7 high verified on PCB
	oki.set_addrmap(0, &glass_state::oki_map);
	oki.add_route(ALL_OUTPUTS, "mono", 1.0);
}

void glass_state::glass_ds5002fp(machine_config &config)
{
	glass(config);
	gaelco_ds5002fp_device &ds5002fp(GAELCO_DS5002FP(config, "gaelco_ds5002fp", XTAL(24'000'000) / 2)); // verified on PCB
	ds5002fp.set_addrmap(0, &glass_state::mcu_hostmem_map);
	config.set_perfect_quantum("gaelco_ds5002fp:mcu");
}

ROM_START( glass )
	ROM_REGION( 0x100000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "europa_c23_3-2-94_3fb0 27c020.bin", 0x000000, 0x040000, CRC(f2932dc2) SHA1(7fc49da53f608599325670c8c92e0a1a7a8850a9) )
	ROM_LOAD16_BYTE( "europa_c22_3-2-94_a20c_27c020.bin", 0x000001, 0x040000, CRC(165e2e01) SHA1(180a2e2b5151f2321d85ac23eff7fbc9f52023a5) )

	ROM_REGION( 0x8000, "gaelco_ds5002fp:sram", 0 ) // DS5002FP code
	ROM_LOAD( "glass_ds5002fp_sram.bin", 0x00000, 0x8000, CRC(47c9df4c) SHA1(e0ac4f3d3086a4e8164d42aaae125037c222118a) )

	ROM_REGION( 0x100, "gaelco_ds5002fp:mcu:internal", ROMREGION_ERASE00 )
	// these are the default states stored in NVRAM
	DS5002FP_SET_MON( 0x29 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "h13.bin", 0x000000, 0x200000, CRC(13ab7f31) SHA1(468424f74d6cccd1b445a9f20e2d24bc46d61ed6) )
	ROM_LOAD( "h11.bin", 0x200000, 0x200000, CRC(c6ac41c8) SHA1(22408ef1e35c66d0fba0c72972c46fad891d1193) )

	ROM_REGION( 0x100000, "bmap", 0 )   // 16 bitmaps (320x200, indexed colors)
	ROM_LOAD( "h9.bin", 0x000000, 0x100000, CRC(b9492557) SHA1(3f5c0d696d65e1cd492763dfa749c813dd56a9bf) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "c1.bin", 0x000000, 0x100000, CRC(d9f075a2) SHA1(31a7a677861f39d512e9d1f51925c689e481159a) )
	// 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs
ROM_END

ROM_START( ssplash )
	ROM_REGION( 0x100000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "c_23_splash_titulo_11-4_27c2001.bin", 0x000000, 0x040000, CRC(563c4883) SHA1(98fff2df5eeeed58692c70ea995458b48355bb01) )
	ROM_LOAD16_BYTE( "europa_c22_3-2-94_a20c_27c020.bin", 0x000001, 0x040000, CRC(165e2e01) SHA1(180a2e2b5151f2321d85ac23eff7fbc9f52023a5) )

	ROM_REGION( 0x8000, "gaelco_ds5002fp:sram", 0 ) // DS5002FP code
	ROM_LOAD( "glass_ds5002fp_sram.bin", 0x00000, 0x8000, CRC(47c9df4c) SHA1(e0ac4f3d3086a4e8164d42aaae125037c222118a) )

	ROM_REGION( 0x100, "gaelco_ds5002fp:mcu:internal", ROMREGION_ERASE00 )
	// these are the default states stored in NVRAM
	DS5002FP_SET_MON( 0x29 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "h13.bin", 0x000000, 0x200000, CRC(13ab7f31) SHA1(468424f74d6cccd1b445a9f20e2d24bc46d61ed6) )
	ROM_LOAD( "h11.bin", 0x200000, 0x200000, CRC(c6ac41c8) SHA1(22408ef1e35c66d0fba0c72972c46fad891d1193) )

	ROM_REGION( 0x100000, "bmap", 0 )   // 16 bitmaps (320x200, indexed colors)
	ROM_LOAD( "h9.bin", 0x000000, 0x100000, CRC(b9492557) SHA1(3f5c0d696d65e1cd492763dfa749c813dd56a9bf) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "c1.bin", 0x000000, 0x100000, CRC(d9f075a2) SHA1(31a7a677861f39d512e9d1f51925c689e481159a) )
	// 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs
ROM_END

ROM_START( ssplash10 )
	ROM_REGION( 0x100000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "c-23_splash_0_27-10-93_27c020.bin", 0x000000, 0x040000, CRC(931b4cfd) SHA1(f323f489dfe5c35d0901fe906a973cc839bba248) )
	ROM_LOAD16_BYTE( "c-22_splash_1_27-10-93_27c020.bin", 0x000001, 0x040000, CRC(fa3049e9) SHA1(caa67837084b093b3ed1c0df4a43b3d0db179a6d) )

	ROM_REGION( 0x8000, "gaelco_ds5002fp:sram", 0 ) // DS5002FP code
	// needs to be different?
	ROM_LOAD( "ssplash10_ds5002fp_sram.bin", 0x00000, 0x8000, BAD_DUMP CRC(47c9df4c) SHA1(e0ac4f3d3086a4e8164d42aaae125037c222118a) )

	ROM_REGION( 0x100, "gaelco_ds5002fp:mcu:internal", ROMREGION_ERASE00 )
	// these are the default states stored in NVRAM
	DS5002FP_SET_MON( 0x29 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "h13.bin", 0x000000, 0x200000, CRC(13ab7f31) SHA1(468424f74d6cccd1b445a9f20e2d24bc46d61ed6) )
	ROM_LOAD( "h11.bin", 0x200000, 0x200000, CRC(c6ac41c8) SHA1(22408ef1e35c66d0fba0c72972c46fad891d1193) )

	ROM_REGION( 0x100000, "bmap", 0 )   // 16 bitmaps (320x200, indexed colors)
	ROM_LOAD( "h9.bin", 0x000000, 0x100000, CRC(b9492557) SHA1(3f5c0d696d65e1cd492763dfa749c813dd56a9bf) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "c1.bin", 0x000000, 0x100000, CRC(d9f075a2) SHA1(31a7a677861f39d512e9d1f51925c689e481159a) )
	// 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs
ROM_END

ROM_START( glassa ) // Version 1.1
	ROM_REGION( 0x100000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "1.c23", 0x000000, 0x040000, CRC(aeebd4ed) SHA1(04759dc146dff0fc74b78d70e79dfaebe68328f9) ) // old parent set, only differs from glass / ssplash by 2 bytes outside of the region byte, could be bad?
	ROM_LOAD16_BYTE( "2.c22", 0x000001, 0x040000, CRC(165e2e01) SHA1(180a2e2b5151f2321d85ac23eff7fbc9f52023a5) )

	ROM_REGION( 0x8000, "gaelco_ds5002fp:sram", 0 ) // DS5002FP code
	ROM_LOAD( "glass_ds5002fp_sram.bin", 0x00000, 0x8000, CRC(47c9df4c) SHA1(e0ac4f3d3086a4e8164d42aaae125037c222118a) )

	ROM_REGION( 0x100, "gaelco_ds5002fp:mcu:internal", ROMREGION_ERASE00 )
	// these are the default states stored in NVRAM
	DS5002FP_SET_MON( 0x29 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "h13.bin", 0x000000, 0x200000, CRC(13ab7f31) SHA1(468424f74d6cccd1b445a9f20e2d24bc46d61ed6) )
	ROM_LOAD( "h11.bin", 0x200000, 0x200000, CRC(c6ac41c8) SHA1(22408ef1e35c66d0fba0c72972c46fad891d1193) )

	ROM_REGION( 0x100000, "bmap", 0 )   // 16 bitmaps (320x200, indexed colors)
	ROM_LOAD( "h9.bin", 0x000000, 0x100000, CRC(b9492557) SHA1(3f5c0d696d65e1cd492763dfa749c813dd56a9bf) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "c1.bin", 0x000000, 0x100000, CRC(d9f075a2) SHA1(31a7a677861f39d512e9d1f51925c689e481159a) )
	// 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs
ROM_END

ROM_START( glass10 ) // Version 1.0
	ROM_REGION( 0x100000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "c23.bin", 0x000000, 0x040000, CRC(688cdf33) SHA1(b59dcc3fc15f72037692b745927b110e97d8282e) )
	ROM_LOAD16_BYTE( "c22.bin", 0x000001, 0x040000, CRC(ab17c992) SHA1(1509b5b4bbfb4e022e0ab6fbbc0ffc070adfa531) )

	ROM_REGION( 0x8000, "gaelco_ds5002fp:sram", 0 ) // DS5002FP code
	ROM_LOAD( "glass_ds5002fp_sram.bin", 0x00000, 0x8000, CRC(47c9df4c) SHA1(e0ac4f3d3086a4e8164d42aaae125037c222118a) )

	ROM_REGION( 0x100, "gaelco_ds5002fp:mcu:internal", ROMREGION_ERASE00 )
	// these are the default states stored in NVRAM
	DS5002FP_SET_MON( 0x29 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "h13.bin", 0x000000, 0x200000, CRC(13ab7f31) SHA1(468424f74d6cccd1b445a9f20e2d24bc46d61ed6) )
	ROM_LOAD( "h11.bin", 0x200000, 0x200000, CRC(c6ac41c8) SHA1(22408ef1e35c66d0fba0c72972c46fad891d1193) )

	ROM_REGION( 0x100000, "bmap", 0 )   // 16 bitmaps (320x200, indexed colors)
	ROM_LOAD( "h9.bin", 0x000000, 0x100000, CRC(b9492557) SHA1(3f5c0d696d65e1cd492763dfa749c813dd56a9bf) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "c1.bin", 0x000000, 0x100000, CRC(d9f075a2) SHA1(31a7a677861f39d512e9d1f51925c689e481159a) )
	// 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs
ROM_END

ROM_START( glass10a ) // Title screen shows "GLASS" and under that "Break Edition" on a real PCB
	ROM_REGION( 0x100000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "spl-c23.bin", 0x000000, 0x040000, CRC(c1393bea) SHA1(a5f877ba38305a7b49fa3c96b9344cbf71e8c9ef) )
	ROM_LOAD16_BYTE( "spl-c22.bin", 0x000001, 0x040000, CRC(0d6fa33e) SHA1(37e9258ef7e108d034c80abc8e5e5ab6dacf0a61) )

	ROM_REGION( 0x8000, "gaelco_ds5002fp:sram", 0 ) // DS5002FP code
	ROM_LOAD( "glass_ds5002fp_sram.bin", 0x00000, 0x8000, CRC(47c9df4c) SHA1(e0ac4f3d3086a4e8164d42aaae125037c222118a) )

	ROM_REGION( 0x100, "gaelco_ds5002fp:mcu:internal", ROMREGION_ERASE00 )
	// these are the default states stored in NVRAM
	DS5002FP_SET_MON( 0x29 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "h13.bin", 0x000000, 0x200000, CRC(13ab7f31) SHA1(468424f74d6cccd1b445a9f20e2d24bc46d61ed6) )
	ROM_LOAD( "h11.bin", 0x200000, 0x200000, CRC(c6ac41c8) SHA1(22408ef1e35c66d0fba0c72972c46fad891d1193) )

	ROM_REGION( 0x100000, "bmap", 0 )   // 16 bitmaps (320x200, indexed colors)
	ROM_LOAD( "h9.bin", 0x000000, 0x100000, CRC(b9492557) SHA1(3f5c0d696d65e1cd492763dfa749c813dd56a9bf) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "c1.bin", 0x000000, 0x100000, CRC(d9f075a2) SHA1(31a7a677861f39d512e9d1f51925c689e481159a) )
	// 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs
ROM_END

ROM_START( glassat )
	ROM_REGION( 0x100000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "atari_c-23_3-2-94_27c020.bin", 0x000000, 0x040000, CRC(ff26f608) SHA1(c72a3294e2878e5e93501d19555c62a7d5dd2f57) )
	ROM_LOAD16_BYTE( "atari_c-22_3-2-94_27c020.bin", 0x000001, 0x040000, CRC(ad805bc3) SHA1(7558a81d988bb4758a938ddf52823af11142889f) )

	ROM_REGION( 0x8000, "gaelco_ds5002fp:sram", 0 ) // DS5002FP code
	ROM_LOAD( "glass_ds5002fp_sram.bin", 0x00000, 0x8000, CRC(47c9df4c) SHA1(e0ac4f3d3086a4e8164d42aaae125037c222118a) )

	ROM_REGION( 0x100, "gaelco_ds5002fp:mcu:internal", ROMREGION_ERASE00 )
	// these are the default states stored in NVRAM
	DS5002FP_SET_MON( 0x29 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "h13.bin", 0x000000, 0x200000, CRC(13ab7f31) SHA1(468424f74d6cccd1b445a9f20e2d24bc46d61ed6) )
	ROM_LOAD( "h11.bin", 0x200000, 0x200000, CRC(c6ac41c8) SHA1(22408ef1e35c66d0fba0c72972c46fad891d1193) )

	ROM_REGION( 0x100000, "bmap", 0 )   // 16 bitmaps (320x200, indexed colors)
	ROM_LOAD( "atari_el_4269_27c4001.bin", 0x000000, 0x080000, CRC(514e50ea) SHA1(36b0209b1061103c99d26ff563fdbcdf0aa59433) )
	ROM_LOAD( "atari_eh", 0x080000, 0x080000, NO_DUMP ) // test mode shows this is used, but was missing

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "c1.bin", 0x000000, 0x100000, CRC(d9f075a2) SHA1(31a7a677861f39d512e9d1f51925c689e481159a) )
	// 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs
ROM_END

ROM_START( glasskr )
	ROM_REGION( 0x100000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "c_23_korea_4f3e_27c020.bin", 0x000000, 0x040000, CRC(4d7749e4) SHA1(a9bbf2c51fef89321042e5b65b1fe27ced1c25f1) )
	ROM_LOAD16_BYTE( "c_22_korea_7522_27c020.bin", 0x000001, 0x040000, CRC(a23bba9e) SHA1(d4558e2d8c952296b930e09c265071d4fda7f731) )
	// also seen with these oversized ROMs - 1ST AND 2ND HALF IDENTICAL, matches above
	//ROM_LOAD16_BYTE( "glassk.c23", 0x000000, 0x080000, CRC(6ee19376) SHA1(8a8fdeebe094bd3e29c35cf59584e3cab708732d) )
	//ROM_LOAD16_BYTE( "glassk.c22", 0x000001, 0x080000, CRC(bd546568) SHA1(bcd5e7591f4e68c9470999b8a0ef1ee4392c907c) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "h13.bin", 0x000000, 0x200000, CRC(13ab7f31) SHA1(468424f74d6cccd1b445a9f20e2d24bc46d61ed6) )
	ROM_LOAD( "h11.bin", 0x200000, 0x200000, CRC(c6ac41c8) SHA1(22408ef1e35c66d0fba0c72972c46fad891d1193) )

	ROM_REGION( 0x100000, "bmap", 0 )   // 16 bitmaps (320x200, indexed colors)
	ROM_LOAD( "glassk.h9", 0x000000, 0x100000, CRC(d499be4c) SHA1(204f754813be687e8dc00bfe7b5dbc4857ac8738) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "c1.bin", 0x000000, 0x100000, CRC(d9f075a2) SHA1(31a7a677861f39d512e9d1f51925c689e481159a) )
	// 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs
ROM_END

ROM_START( glasskra )
	ROM_REGION( 0x100000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "c_23_alt_korea_7ce8_27c020.bin", 0x000000, 0x040000, CRC(6534fd44) SHA1(5e59c283a595b38f6e01930334bb9861e6efccdd) )
	ROM_LOAD16_BYTE( "c_22_alt_korea_8ce7_27c020.bin", 0x000001, 0x040000, CRC(ccde51eb) SHA1(0adb8f4a69cdc3f9470cd007f68c2435efcfacb0) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "h13.bin", 0x000000, 0x200000, CRC(13ab7f31) SHA1(468424f74d6cccd1b445a9f20e2d24bc46d61ed6) )
	ROM_LOAD( "h11.bin", 0x200000, 0x200000, CRC(c6ac41c8) SHA1(22408ef1e35c66d0fba0c72972c46fad891d1193) )

	ROM_REGION( 0x100000, "bmap", 0 )   // 16 bitmaps (320x200, indexed colors)
	// same as glasskr but split in 2
	ROM_LOAD( "el_korea_d4f1_27c4001.bin", 0x000000, 0x080000, CRC(b7b78b93) SHA1(dd0ae387a95ee4912b1c2b80732b3e6e0a496c8e) )
	ROM_LOAD( "eh_glass.bin",              0x080000, 0x080000, CRC(ea568c62) SHA1(ccf155b32cfb6fc52b2ea13ce72444eb68c3417d) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "c1.bin", 0x000000, 0x100000, CRC(d9f075a2) SHA1(31a7a677861f39d512e9d1f51925c689e481159a) )
	// 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs
ROM_END

ROM_START( glass10b )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "glass 0 23 22-11 b041 27c020.bin", 0x000000, 0x040000, CRC(d2e81d61) SHA1(01dab0a670ac314576f24886701accb9be732fb8) )
	ROM_LOAD16_BYTE( "glass 1 22 22-11 90d9 27c020.bin", 0x000001, 0x040000, CRC(8b631fb8) SHA1(1101d671611cbae29b96e75601e6856f2931b00c) )

	ROM_REGION( 0x8000, "gaelco_ds5002fp:sram", 0 ) // DS5002FP code
	ROM_LOAD( "glass_ds5002fp_sram.bin", 0x00000, 0x8000, CRC(47c9df4c) SHA1(e0ac4f3d3086a4e8164d42aaae125037c222118a) )

	ROM_REGION( 0x100, "gaelco_ds5002fp:mcu:internal", ROMREGION_ERASE00 )
	// these are the default states stored in NVRAM
	DS5002FP_SET_MON( 0x29 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x400000, "gfx", 0 ) // same split format as the development board
	ROM_LOAD16_BYTE( "al_d0-d7_27c4001.bin",  0x000000, 0x080000, CRC(c668caad) SHA1(742030d27ed5f7f7966d00181e13c7ba9b885df5) )
	ROM_LOAD16_BYTE( "bl_d8-d15_27c4001.bin", 0x000001, 0x080000, CRC(7a6cb91f) SHA1(2c8592d4f92ea380a85f3a43acef3f3898973963) )
	ROM_LOAD16_BYTE( "ah_d0-d7_27c4001.bin",  0x100000, 0x080000, CRC(de27b785) SHA1(aa14e33f42039f67fb418ff45b26bef511c9c7c8) )
	ROM_LOAD16_BYTE( "bh_d8-d15_27c4001.bin", 0x100001, 0x080000, CRC(e9aa45d3) SHA1(a9e09b2e338d75a414eea90672a1a7142e858d84) )
	ROM_LOAD16_BYTE( "cl_d0-d7_27c4001.bin",  0x200000, 0x080000, CRC(e32639be) SHA1(9c18c871d042fe7c62a344f35968457ae8302fa9) )
	ROM_LOAD16_BYTE( "dl_d8-d15_27c4001.bin", 0x200001, 0x080000, CRC(b56eb8a4) SHA1(5d647a3ae175051a11a3c496a32b34a368096487) )
	ROM_LOAD16_BYTE( "ch_d0-d7_27c4001.bin",  0x300000, 0x080000, CRC(1b3f148d) SHA1(b62c5c6fb3f4ad543538764305e2e27b35c37334) )
	ROM_LOAD16_BYTE( "dh_d8-d15_27c4001.bin", 0x300001, 0x080000, CRC(f3638123) SHA1(60d58608de193c1a3176c95419aa38a66e149e2a) )

	ROM_REGION( 0x100000, "bmap", 0 )   // 16 bitmaps (320x200, indexed colors)
	ROM_LOAD( "h9.bin", 0x000000, 0x100000, CRC(b9492557) SHA1(3f5c0d696d65e1cd492763dfa749c813dd56a9bf) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "c1.bin", 0x000000, 0x100000, CRC(d9f075a2) SHA1(31a7a677861f39d512e9d1f51925c689e481159a) )
	// 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs
ROM_END

ROM_START( glass10c )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "glass break 0 16-11 e419 27c020.bin", 0x000000, 0x040000, CRC(dab83534) SHA1(daf7c5b82d4912b78264e9a2d17b19fafc2d96a9) )
	ROM_LOAD16_BYTE( "glass break 1 16-11 d948 27c020.bin", 0x000001, 0x040000, CRC(35348cae) SHA1(6657a2564a8f9af1673183ea997c0f3c4a65d639) )

	ROM_REGION( 0x8000, "gaelco_ds5002fp:sram", 0 ) // DS5002FP code
	ROM_LOAD( "glass_ds5002fp_sram.bin", 0x00000, 0x8000, CRC(47c9df4c) SHA1(e0ac4f3d3086a4e8164d42aaae125037c222118a) )

	ROM_REGION( 0x100, "gaelco_ds5002fp:mcu:internal", ROMREGION_ERASE00 )
	// these are the default states stored in NVRAM
	DS5002FP_SET_MON( 0x29 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x400000, "gfx", 0 ) // same split format as the development board
	ROM_LOAD16_BYTE( "al_d0-d7_27c4001.bin",  0x000000, 0x080000, CRC(c668caad) SHA1(742030d27ed5f7f7966d00181e13c7ba9b885df5) )
	ROM_LOAD16_BYTE( "bl_d8-d15_27c4001.bin", 0x000001, 0x080000, CRC(7a6cb91f) SHA1(2c8592d4f92ea380a85f3a43acef3f3898973963) )
	ROM_LOAD16_BYTE( "ah_d0-d7_27c4001.bin",  0x100000, 0x080000, CRC(de27b785) SHA1(aa14e33f42039f67fb418ff45b26bef511c9c7c8) )
	ROM_LOAD16_BYTE( "bh_d8-d15_27c4001.bin", 0x100001, 0x080000, CRC(e9aa45d3) SHA1(a9e09b2e338d75a414eea90672a1a7142e858d84) )
	ROM_LOAD16_BYTE( "cl_d0-d7_27c4001.bin",  0x200000, 0x080000, CRC(e32639be) SHA1(9c18c871d042fe7c62a344f35968457ae8302fa9) )
	ROM_LOAD16_BYTE( "dl_d8-d15_27c4001.bin", 0x200001, 0x080000, CRC(b56eb8a4) SHA1(5d647a3ae175051a11a3c496a32b34a368096487) )
	ROM_LOAD16_BYTE( "ch_d0-d7_27c4001.bin",  0x300000, 0x080000, CRC(1b3f148d) SHA1(b62c5c6fb3f4ad543538764305e2e27b35c37334) )
	ROM_LOAD16_BYTE( "dh_d8-d15_27c4001.bin", 0x300001, 0x080000, CRC(f3638123) SHA1(60d58608de193c1a3176c95419aa38a66e149e2a) )

	ROM_REGION( 0x100000, "bmap", 0 )   // 16 bitmaps (320x200, indexed colors)
	ROM_LOAD( "h9.bin", 0x000000, 0x100000, CRC(b9492557) SHA1(3f5c0d696d65e1cd492763dfa749c813dd56a9bf) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "c1.bin", 0x000000, 0x100000, CRC(d9f075a2) SHA1(31a7a677861f39d512e9d1f51925c689e481159a) )
	// 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs
ROM_END

ROM_START( glass10d )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "glass spain 0 10-11-93 27c020.bin", 0x000000, 0x040000, CRC(9e359418) SHA1(5f735af493cf1565ae7d8304e042f4809f3bc64c) )
	ROM_LOAD16_BYTE( "glass spain 1 10-11-93 27c020.bin", 0x000001, 0x040000, CRC(1f5ed48d) SHA1(e900574791b4d2fbe7d0d706dac93b3086aed7eb) )

	ROM_REGION( 0x8000, "gaelco_ds5002fp:sram", 0 ) // DS5002FP code
	ROM_LOAD( "glass_ds5002fp_sram.bin", 0x00000, 0x8000, CRC(47c9df4c) SHA1(e0ac4f3d3086a4e8164d42aaae125037c222118a) )

	ROM_REGION( 0x100, "gaelco_ds5002fp:mcu:internal", ROMREGION_ERASE00 )
	// these are the default states stored in NVRAM
	DS5002FP_SET_MON( 0x29 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x400000, "gfx", 0 ) // same split format as the development board
	ROM_LOAD16_BYTE( "al_d0-d7_27c4001.bin",  0x000000, 0x080000, CRC(c668caad) SHA1(742030d27ed5f7f7966d00181e13c7ba9b885df5) )
	ROM_LOAD16_BYTE( "bl_d8-d15_27c4001.bin", 0x000001, 0x080000, CRC(7a6cb91f) SHA1(2c8592d4f92ea380a85f3a43acef3f3898973963) )
	ROM_LOAD16_BYTE( "ah_d0-d7_27c4001.bin",  0x100000, 0x080000, CRC(de27b785) SHA1(aa14e33f42039f67fb418ff45b26bef511c9c7c8) )
	ROM_LOAD16_BYTE( "bh_d8-d15_27c4001.bin", 0x100001, 0x080000, CRC(e9aa45d3) SHA1(a9e09b2e338d75a414eea90672a1a7142e858d84) )
	ROM_LOAD16_BYTE( "cl_d0-d7_27c4001.bin",  0x200000, 0x080000, CRC(e32639be) SHA1(9c18c871d042fe7c62a344f35968457ae8302fa9) )
	ROM_LOAD16_BYTE( "dl_d8-d15_27c4001.bin", 0x200001, 0x080000, CRC(b56eb8a4) SHA1(5d647a3ae175051a11a3c496a32b34a368096487) )
	ROM_LOAD16_BYTE( "ch_d0-d7_27c4001.bin",  0x300000, 0x080000, CRC(1b3f148d) SHA1(b62c5c6fb3f4ad543538764305e2e27b35c37334) )
	ROM_LOAD16_BYTE( "dh_d8-d15_27c4001.bin", 0x300001, 0x080000, CRC(f3638123) SHA1(60d58608de193c1a3176c95419aa38a66e149e2a) )

	ROM_REGION( 0x100000, "bmap", 0 )   // 16 bitmaps (320x200, indexed colors)
	ROM_LOAD( "h9.bin", 0x000000, 0x100000, CRC(b9492557) SHA1(3f5c0d696d65e1cd492763dfa749c813dd56a9bf) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "c1.bin", 0x000000, 0x100000, CRC(d9f075a2) SHA1(31a7a677861f39d512e9d1f51925c689e481159a) )
	// 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs
ROM_END

ROM_START( glassp )
	ROM_REGION( 0x100000, "maincpu", 0 )
	// There were 8 x SRM20100LC chips near the CPU and the 2 ROMs below; presumably the game code would have
	// been uploaded to those for testing as these program ROMs do not contain the game, only a stub program.
	ROM_LOAD16_BYTE( "omk_pds_glass_0_27c512_e.bin", 0x000000, 0x010000, CRC(3b106c6e) SHA1(403b5402e36c9c484cdb7a709deb3cc2b55374a1) )
	ROM_LOAD16_BYTE( "omk_pds_glass_1_27c512_o.bin", 0x000001, 0x010000, CRC(593581e1) SHA1(927b06857f17552c5315e2e5e1bb3feca4255010) )

	// there was space for a DS5002, but not populated

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD16_BYTE( "al_d0-d7_27c4001.bin",  0x000000, 0x080000, CRC(c668caad) SHA1(742030d27ed5f7f7966d00181e13c7ba9b885df5) )
	ROM_LOAD16_BYTE( "bl_d8-d15_27c4001.bin", 0x000001, 0x080000, CRC(7a6cb91f) SHA1(2c8592d4f92ea380a85f3a43acef3f3898973963) )
	ROM_LOAD16_BYTE( "ah_d0-d7_27c4001.bin",  0x100000, 0x080000, CRC(de27b785) SHA1(aa14e33f42039f67fb418ff45b26bef511c9c7c8) )
	ROM_LOAD16_BYTE( "bh_d8-d15_27c4001.bin", 0x100001, 0x080000, CRC(e9aa45d3) SHA1(a9e09b2e338d75a414eea90672a1a7142e858d84) )
	ROM_LOAD16_BYTE( "cl_d0-d7_27c4001.bin",  0x200000, 0x080000, CRC(e32639be) SHA1(9c18c871d042fe7c62a344f35968457ae8302fa9) )
	ROM_LOAD16_BYTE( "dl_d8-d15_27c4001.bin", 0x200001, 0x080000, CRC(b56eb8a4) SHA1(5d647a3ae175051a11a3c496a32b34a368096487) )
	ROM_LOAD16_BYTE( "ch_d0-d7_27c4001.bin",  0x300000, 0x080000, CRC(1b3f148d) SHA1(b62c5c6fb3f4ad543538764305e2e27b35c37334) )
	ROM_LOAD16_BYTE( "dh_d8-d15_27c4001.bin", 0x300001, 0x080000, CRC(f3638123) SHA1(60d58608de193c1a3176c95419aa38a66e149e2a) )

	ROM_REGION( 0x100000, "bmap", 0 )   // 16 bitmaps (320x200, indexed colors)
	ROM_LOAD( "el_spain_27c4001.bin", 0x000000, 0x080000, CRC(e8c10b29) SHA1(ee289ff8fb8d4dad6e83f8d837dd10942e554695) )
	ROM_LOAD( "eh_spain_27c4001.bin", 0x080000, 0x080000, CRC(ea568c62) SHA1(ccf155b32cfb6fc52b2ea13ce72444eb68c3417d) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "c1_fl_27c4001.bin", 0x000000, 0x080000, CRC(8d9aff0c) SHA1(c704988f752126ee59241aa79bd648986242cf56) )
	ROM_LOAD( "c3_fh_27c4001.bin", 0x080000, 0x080000, CRC(124e1bb2) SHA1(a715fd294b93c80adfc91a7d69eea716ff040232) )
	// 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs

	ROM_REGION( 0x144, "plds", 0 )
	ROM_LOAD( "dec_pds_new_gal16v8.bin", 0x000, 0x117, CRC(3e165a3e) SHA1(bf40d26cb004b4a652848b2ef335a9a8272b7400) )
	ROM_LOAD( "palce16v8h.e2",           0x000, 0x117, CRC(d5ed5985) SHA1(a4e9c8e3a7774e2a02fbca3ddf8175cf251825ba) )
	ROM_LOAD( "palce16v8h.h21",          0x000, 0x117, CRC(f4f1e1e8) SHA1(0802a84b988a06ad276f3d593d8f826b765185c9) )
	ROM_LOAD( "tibpal16r8.b15",          0x000, 0x104, CRC(27b1ca8b) SHA1(038d1352baff18f619ac4149e5825ef9664c983b) )
	ROM_LOAD( "tibpal20l8.b23",          0x000, 0x144, CRC(87e5e6ab) SHA1(f42b952128bd26fe565b06403c7b1c95061e5034) )
	ROM_LOAD( "tibpal20l8.h15",          0x000, 0x144, CRC(55a214bd) SHA1(956738e0bbe8cc91e220a158893b61867bae5cb2) )
ROM_END

} // anonymous namespace


/*
 All versions of Glass (but 'ssplash10') contain the 'Break Edition' string (it just seems to be part of the title?).

 Version 1.1 releases also show Version 1994 on the title screen. These versions do not have skulls in the playfield (at least not on early stages).
 The protected version 1.1 also doesn't show any kind of attract gameplay, looks like it was patched out? (should be verified on an untouched original 1.1 using its original SRAM tho).
 The unprotected versions are Korean sets, they're censored, and have different girl pictures.
*/

// Newer 1.1 versions from 1994, all seem to be dated 3 Feb 1994 (apart from Korea?)

// These 2 are the same codebase, just a config byte change
GAME( 1994, glass,     0,     glass_ds5002fp, glass, glass_state, empty_init, ROT0, "OMK / Gaelco",                  "Glass (ver 1.1, Break Edition, checksum 49D5E66B, Version 1994, set 1)",              MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS ) // 3 Feb 1994 on stickers
GAME( 1994, ssplash,   glass, glass_ds5002fp, glass, glass_state, empty_init, ROT0, "OMK / Gaelco",                  "Super Splash (ver 1.1, Break Edition, checksum 59D5E66B, Version 1994)",              MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS ) // 3 Feb 1994 on stickers

// This is 2 bytes different to the above 'glass' set, could be bad? shows same checksum(!)
GAME( 1994, glassa,    glass, glass_ds5002fp, glass, glass_state, empty_init, ROT0, "OMK / Gaelco",                  "Glass (ver 1.1, Break Edition, checksum 49D5E66B, Version 1994, set 2)",              MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )

// Censored images, playable but missing a GFX ROM used for some images, Atari marked on ROMs, not shown in game
GAME( 1994, glassat,   glass, glass_ds5002fp, glass, glass_state, empty_init, ROT0, "OMK / Gaelco (Atari license)",  "Glass (ver 1.1, Break Edition, checksum D7AF5496, Version 1994, US)",                 MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS ) // 3 Feb 1994 on stickers

// Censored images, unprotected
GAME( 1994, glasskr,   glass, glass,          glass, glass_state, empty_init, ROT0, "OMK / Gaelco (Promat license)", "Glass (ver 1.1, Break Edition, checksum D419AB69, Version 1994, unprotected, Korea)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS ) // promat stickers on program ROMs
GAME( 1994, glasskra,  glass, glass,          glass, glass_state, empty_init, ROT0, "OMK / Gaelco (Promat license)", "Glass (ver 1.1, Break Edition, checksum 3D8A724F, Version 1994, unprotected, Korea)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS ) // promat stickers on program ROMs

// Older 1.0 versions from 1993
GAME( 1993, glass10,   glass, glass_ds5002fp, glass, glass_state, empty_init, ROT0, "OMK / Gaelco",                  "Glass (ver 1.0, Break Edition, checksum C5513F3C)",                                   MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1993, glass10a,  glass, glass_ds5002fp, glass, glass_state, empty_init, ROT0, "OMK / Gaelco",                  "Glass (ver 1.0, Break Edition, checksum D3864FDB)",                                   MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1993, glass10b,  glass, glass_ds5002fp, glass, glass_state, empty_init, ROT0, "OMK / Gaelco",                  "Glass (ver 1.0, Break Edition, checksum EBCB0BFE, 22 Nov 1993)",                      MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS ) // date from stickers
GAME( 1993, glass10c,  glass, glass_ds5002fp, glass, glass_state, empty_init, ROT0, "OMK / Gaelco",                  "Glass (ver 1.0, Break Edition, checksum 6241CD67, 16 Nov 1993)",                      MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS ) // date from stickers
GAME( 1993, glass10d,  glass, glass_ds5002fp, glass, glass_state, empty_init, ROT0, "OMK / Gaelco",                  "Glass (ver 1.0, Break Edition, checksum 2B43D337, 10 Nov 1993)",                      MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS ) // date from stickers

// Even older 1.0 version, without the Break Edition subtitle, maybe prototype?
// Enemies don't move correctly and can't be shot, probably requires a different DS5002FP even if it 'passes' the startup check
GAME( 1993, ssplash10, glass, glass_ds5002fp, ssplash10, glass_state, empty_init, ROT0, "OMK / Gaelco", "Super Splash (Ver 1.0, checksum 2104394E, 27 Oct 1993)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS ) // date from stickers

// This development board can't work without the code/data that would have been remotely uploaded to the SRAMs, hence 'incomplete' flag.
GAME( 1993, glassp,    glass, glass,          glass, glass_state, empty_init, ROT0, "OMK / Gaelco", "Glass (development PCB)", MACHINE_NOT_WORKING | MACHINE_IS_INCOMPLETE )
