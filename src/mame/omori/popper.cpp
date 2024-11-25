// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Popper

    © 1983 Omori Electric Co. (OEC)

    PCB connector pinout

          GND   1   2  GND
          GND   3   4  GND
           5V   5   6  5V
           5V   7   8  5V
      Speaker   9  10  Speaker
          12V  11  12  12V
     1-P Push  13  14  1-P Right
     2-P Push  15  16  2-P Right
    1-P Start  17  18  1-P Left
    2-P Start  19  20  2-P Left
       Coin A  21  22  1-P Down
       Coin B  23  24  2-P Down
      Service  25  26  1-P Up
               27  28  2-P Up
               29  30
               31  32
        Synic  33  34  Red
        Green  35  36  Blue
           5V  37  38  5V
           5V  39  40  5V
          GND  41  42  GND

    TODO:
    - According to the schematics the sub CPU ROM should be 0x2000
    - Verify screen raw parameters
    - Verify graphics with real hardware
    - Watchdog

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "sound/ay8910.h"
#include "video/resnet.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class popper_state : public driver_device
{
public:
	popper_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "subcpu"),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_ay{ {*this, "ay1"}, {*this, "ay2"} },
		m_video_ram(*this, "video_ram"),
		m_attribute_ram(*this, "attribute_ram"),
		m_sprite_ram(*this, "sprite_ram"),
		m_inputs{ {*this, "in1"}, {*this, "in0"}, {*this, "dsw2"}, {*this, "dsw1"} },
		m_scanline_timer(nullptr),
		m_layer0_tilemap(nullptr), m_layer1_tilemap(nullptr),
		m_nmi_enable(0), m_back_color(0), m_vram_page(0)
	{ }

	void popper(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(scanline_tick);

private:
	required_device<z80_device> m_maincpu;
	required_device<z80_device> m_subcpu;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<ay8910_device> m_ay[2];
	required_shared_ptr<uint8_t> m_video_ram;
	required_shared_ptr<uint8_t> m_attribute_ram;
	required_shared_ptr<uint8_t> m_sprite_ram;
	required_ioport m_inputs[4];

	emu_timer *m_scanline_timer;
	tilemap_t *m_layer0_tilemap;
	tilemap_t *m_layer1_tilemap;

	int m_nmi_enable;
	int m_back_color;
	int m_vram_page;

	void main_map(address_map &map) ATTR_COLD;
	void sub_map(address_map &map) ATTR_COLD;

	void popper_palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TILE_GET_INFO_MEMBER(layer0_tile_info);
	TILE_GET_INFO_MEMBER(layer1_tile_info);

	void nmi_control_w(int state);
	void back_color_select_w(int state);
	void vram_page_select_w(int state);
	template <int N> void intcycle_w(int state);
	uint8_t subcpu_nmi_r();
	uint8_t subcpu_reset_r();
	void ay1_w(offs_t offset, uint8_t data);
	uint8_t watchdog_clear_r();
	uint8_t inputs_r(offs_t offset);
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void popper_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).noprw();
	map(0xc000, 0xc0ff).ram();
	map(0xc100, 0xc6ff).ram().share("video_ram");
	map(0xc700, 0xc8ff).ram();
	map(0xc900, 0xceff).ram().share("attribute_ram");
	map(0xcf00, 0xcfff).ram();
	map(0xd000, 0xd7ff).ram().share("sprite_ram");
	map(0xd800, 0xdfff).ram().share("shared");
	map(0xe000, 0xe003).mirror(0x03fc).r(FUNC(popper_state::inputs_r));
	map(0xe000, 0xe007).mirror(0x1ff8).w("outlatch", FUNC(addressable_latch_device::write_d0));
	map(0xe400, 0xe400).mirror(0x03ff).r(FUNC(popper_state::subcpu_nmi_r));
	map(0xe800, 0xf7ff).noprw();
	map(0xf800, 0xf800).mirror(0x03ff).r(FUNC(popper_state::subcpu_reset_r));
	map(0xfc00, 0xfc00).mirror(0x03ff).r(FUNC(popper_state::watchdog_clear_r));
}

void popper_state::sub_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x7fff).noprw();
	map(0x8000, 0x8003).mirror(0x1ffc).w(FUNC(popper_state::ay1_w));
	map(0xa000, 0xa003).mirror(0x1ffc).w("ay2", FUNC(ay8910_device::write_bc1_bc2));
	map(0xc000, 0xc7ff).mirror(0x1800).ram().share("shared");
	map(0xe000, 0xffff).noprw();
}


//**************************************************************************
//  INPUT PORT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( popper )
	PORT_START("in0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN)                  PORT_8WAY
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_COIN2)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN)   PORT_COCKTAIL  PORT_8WAY
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_SERVICE1)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP)                    PORT_8WAY
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP)     PORT_COCKTAIL  PORT_8WAY

	PORT_START("in1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT)                 PORT_8WAY
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON1)         PORT_COCKTAIL
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT)  PORT_COCKTAIL  PORT_8WAY
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_START1)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT)                  PORT_8WAY
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_START2)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT)   PORT_COCKTAIL  PORT_8WAY

	PORT_START("dsw1")
	PORT_DIPNAME(0x03, 0x00, DEF_STR( Coin_A ))      PORT_DIPLOCATION("DSW1:1,2")
	PORT_DIPSETTING(   0x00, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(   0x02, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(   0x01, DEF_STR( 3C_1C ))
	PORT_DIPSETTING(   0x03, DEF_STR( 1C_2C ))
	PORT_DIPNAME(0x0c, 0x04, DEF_STR( Coin_B ))      PORT_DIPLOCATION("DSW1:3,4")
	PORT_DIPSETTING(   0x00, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(   0x08, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(   0x04, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(   0x0c, DEF_STR( 1C_6C ))
	PORT_DIPNAME(0x30, 0x20, DEF_STR( Lives ))       PORT_DIPLOCATION("DSW1:5,6")
	PORT_DIPSETTING(   0x00, "2")
	PORT_DIPSETTING(   0x20, "3")
	PORT_DIPSETTING(   0x10, "4")
	PORT_DIPSETTING(   0x30, "5")
	PORT_DIPNAME(0xc0, 0x00, DEF_STR( Bonus_Life ))  PORT_DIPLOCATION("DSW1:7,8")
	PORT_DIPSETTING(   0x00, "20,000 Points")
	PORT_DIPSETTING(   0x80, "30,000 Points")
	PORT_DIPSETTING(   0x40, "40,000 Points")
	PORT_DIPSETTING(   0xc0, "50,000 Points")

	PORT_START("dsw2")
	PORT_DIPNAME(0x01, 0x01, DEF_STR( Demo_Sounds )) PORT_DIPLOCATION("DSW2:1")
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x01, DEF_STR( On ))
	PORT_DIPUNUSED_DIPLOC(0x02, IP_ACTIVE_LOW, "DSW2:2")
	PORT_DIPNAME(0x04, 0x00, DEF_STR( Free_Play ))   PORT_DIPLOCATION("DSW2:3")
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x04, DEF_STR( On ))
	PORT_DIPNAME(0x08, 0x00, "Invulnerability")      PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x08, DEF_STR( On ))
	PORT_DIPNAME(0x10, 0x00, DEF_STR( Flip_Screen )) PORT_DIPLOCATION("DSW2:5")
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x10, DEF_STR( On ))
	PORT_DIPNAME(0x20, 0x20, DEF_STR( Cabinet ))     PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(   0x00, DEF_STR( Cocktail ))
	PORT_DIPSETTING(   0x20, DEF_STR( Upright ))
	PORT_DIPNAME(0x40, 0x00, "Game Repeating")       PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x40, DEF_STR( On ))
	PORT_DIPNAME(0x80, 0x00, DEF_STR( Pause ))       PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x80, DEF_STR( On ))
INPUT_PORTS_END


//**************************************************************************
//  INPUT PORT HANDLING
//**************************************************************************

uint8_t popper_state::inputs_r(offs_t offset)
{
	uint8_t data = 0;

	data |= BIT(m_inputs[3]->read(), offset + 4) << 7;
	data |= BIT(m_inputs[3]->read(), offset + 0) << 6;
	data |= BIT(m_inputs[2]->read(), offset + 4) << 5;
	data |= BIT(m_inputs[2]->read(), offset + 0) << 4;
	data |= BIT(m_inputs[1]->read(), offset + 4) << 3;
	data |= BIT(m_inputs[1]->read(), offset + 0) << 2;
	data |= BIT(m_inputs[0]->read(), offset + 4) << 1;
	data |= BIT(m_inputs[0]->read(), offset + 0) << 0;

	return data;
}


//**************************************************************************
//  PALETTE
//**************************************************************************

static const res_net_decode_info popper_decode_info =
{
	1,
	0,
	63,
	//   R     G     B
	{    0,    0,    0 }, // offsets
	{    0,    3,    6 }, // shifts
	{ 0x07, 0x07, 0x03 }  // masks
};

static const res_net_info popper_net_info =
{
	RES_NET_VCC_5V | RES_NET_VBIAS_5V | RES_NET_VIN_TTL_OUT,
	{
			{ RES_NET_AMP_NONE, 0, 0, 3, { 1000, 470, 220 } },
			{ RES_NET_AMP_NONE, 0, 0, 3, { 1000, 470, 220 } },
			{ RES_NET_AMP_NONE, 0, 0, 2, {  470, 220,   0 } }
	}
};

void popper_state::popper_palette(palette_device &palette) const
{
	const uint8_t *prom = memregion("colors")->base();
	std::vector<rgb_t> rgb;

	compute_res_net_all(rgb, prom, popper_decode_info, popper_net_info);

	palette.set_pen_colors(0, rgb);
	palette.palette()->normalize_range(0, 63);
}


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

TIMER_CALLBACK_MEMBER(popper_state::scanline_tick)
{
	int y = m_screen->vpos();

	// the maincpu gets an nmi when we enter vblank (when enabled)
	m_maincpu->set_input_line(INPUT_LINE_NMI, (m_nmi_enable && y == 240) ? ASSERT_LINE : CLEAR_LINE);

	// the subcpu gets an interrupt each 32 lines
	m_subcpu->set_input_line(INPUT_LINE_IRQ0, ((y & 31) == 0) ? ASSERT_LINE : CLEAR_LINE);

	m_scanline_timer->adjust(m_screen->time_until_pos(y + 1, 0));
}

void popper_state::back_color_select_w(int state)
{
	m_back_color = state;
}

void popper_state::vram_page_select_w(int state)
{
	m_vram_page = state;
}

uint32_t popper_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	// always draw all tiles
	m_layer0_tilemap->mark_all_dirty();
	m_layer1_tilemap->mark_all_dirty();

	// draw the layers with lower priority
	m_layer0_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_layer1_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	// draw the sprites
	for (int offs = 0; offs < 0x800; offs += 4)
	{
		// 0  76543210  Y coordinate
		// 1  76543210  Code
		// 2  7-------  Flip Y
		// 2  -6------  Flip X
		// 2  --54----  Not used
		// 2  ----3210  Color
		// 3  76543210  X coordinate

		int sx = m_sprite_ram[offs + 3];
		int sy = m_sprite_ram[offs + 0];

		// only render sprite if it's in the current offset range
		if (((sy + (flip_screen() ? 2 : 0)) >> 4) != ((~(offs >> 7)) & 0x0f))
			continue;

		sx += 64;
		sy = 240 - sy;

		int flipx = BIT(m_sprite_ram[offs + 2], 6);
		int flipy = BIT(m_sprite_ram[offs + 2], 7);

		if (flip_screen())
		{
			flipx = !flipx;
			flipy = !flipy;

			sx = 232 - sx;
			sx += 128;
			sy = (240 + 2) - sy;
		}

		int code = m_sprite_ram[offs + 1];
		int color = m_sprite_ram[offs + 2] & 0x0f;

		m_gfxdecode->gfx(2)->transpen(bitmap, cliprect, code, color, flipx, flipy, sx, sy, 0);
	}

	// draw the tiles with priority over the sprites
	m_layer0_tilemap->draw(screen, bitmap, cliprect, 1, 0);
	m_layer1_tilemap->draw(screen, bitmap, cliprect, 1, 0);

	return 0;
}


//**************************************************************************
//  DRAWGFX LAYOUTS
//**************************************************************************

static const gfx_layout layer0_charlayout =
{
	8,8,
	RGN_FRAC(2,2),
	1,
	{ 0, 4 },
	{ STEP4(8,1), STEP4(0,1) },
	{ STEP8(0,16) },
	16*8
};

static const gfx_layout layer1_charlayout =
{
	8,8,
	RGN_FRAC(2,2),
	2,
	{ 0, 4 },
	{ STEP4(8,1), STEP4(0,1) },
	{ STEP8(0,16) },
	16*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(0,2), RGN_FRAC(1,2) },
	{ STEP8(8,1), STEP8(0,1) },
	{ STEP16(0,16) },
	16*2*8
};

static GFXDECODE_START( gfx_popper )
	GFXDECODE_ENTRY("tiles",   0, layer0_charlayout, 0, 32)
	GFXDECODE_ENTRY("tiles",   0, layer1_charlayout, 0, 16)
	GFXDECODE_ENTRY("sprites", 0, spritelayout,      0, 16)
GFXDECODE_END

// attribute ram layout
// 7------- high priority
// -654---- layer0 color
// ----3210 layer1 color

TILE_GET_INFO_MEMBER( popper_state::layer0_tile_info )
{
	int code = (m_vram_page << 8) | m_video_ram[tile_index];
	int attr = m_attribute_ram[tile_index];
	int color = (~m_back_color & 1) << 3 | ((attr >> 4) & 7);
	color <<= 1;

	// high priority only applies if a color is set
	tileinfo.category = BIT(attr, 7) && (attr & 0x70);

	tileinfo.set(0, code, color, 0);
}

TILE_GET_INFO_MEMBER( popper_state::layer1_tile_info )
{
	int code = (m_vram_page << 8) | m_video_ram[tile_index];
	int attr = m_attribute_ram[tile_index];
	int color = attr & 0x0f;

	tileinfo.category = BIT(attr, 7);

	tileinfo.set(1, code, color, 0);
}


//**************************************************************************
//  SUBCPU
//**************************************************************************

uint8_t popper_state::subcpu_nmi_r()
{
	m_subcpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	m_subcpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);

	return 0;
}

uint8_t popper_state::subcpu_reset_r()
{
	m_subcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_subcpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);

	return 0;
}

void popper_state::ay1_w(offs_t offset, uint8_t data)
{
	if (offset == 3)
	{
		m_ay[0]->reset();
		m_ay[1]->reset();
	}

	m_ay[0]->write_bc1_bc2(offset, data);
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void popper_state::nmi_control_w(int state)
{
	m_nmi_enable = state;
}

template <int N>
void popper_state::intcycle_w(int state)
{
	// set to 0 and apparently not used by the game
	logerror("intcycle_w<%d> = %d\n", N, state);
}

uint8_t popper_state::watchdog_clear_r()
{
	return 0;
}

void popper_state::machine_start()
{
	// create tilemaps
	m_layer0_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(popper_state::layer0_tile_info)), TILEMAP_SCAN_COLS, 8, 8, 48, 32);
	m_layer0_tilemap->set_transparent_pen(1);

	m_layer1_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(popper_state::layer1_tile_info)), TILEMAP_SCAN_COLS, 8, 8, 48, 32);
	m_layer1_tilemap->set_transparent_pen(0);

	// allocate and start scanline timer
	m_scanline_timer = timer_alloc(FUNC(popper_state::scanline_tick), this);
	m_scanline_timer->adjust(m_screen->time_until_pos(0, 0));

	// register for save states
	save_item(NAME(m_nmi_enable));
	save_item(NAME(m_back_color));
	save_item(NAME(m_vram_page));
}

void popper_state::machine_reset()
{
	m_nmi_enable = 0;
	m_back_color = 0;
	m_vram_page = 0;
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void popper_state::popper(machine_config &config)
{
	Z80(config, m_maincpu, XTAL(18'432'000)/3/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &popper_state::main_map);

	Z80(config, m_subcpu, XTAL(18'432'000)/3/2);
	m_subcpu->set_addrmap(AS_PROGRAM, &popper_state::sub_map);

	config.set_perfect_quantum(m_maincpu);

	ls259_device &outlatch(LS259(config, "outlatch"));
	outlatch.q_out_cb<0>().set(FUNC(popper_state::nmi_control_w));
	outlatch.q_out_cb<1>().set(FUNC(popper_state::flip_screen_set));
	outlatch.q_out_cb<2>().set(FUNC(popper_state::back_color_select_w));
	outlatch.q_out_cb<3>().set(FUNC(popper_state::vram_page_select_w));
	outlatch.q_out_cb<4>().set(FUNC(popper_state::intcycle_w<0>));
	outlatch.q_out_cb<5>().set(FUNC(popper_state::intcycle_w<1>));
	outlatch.q_out_cb<6>().set(FUNC(popper_state::intcycle_w<2>));
	outlatch.q_out_cb<7>().set(FUNC(popper_state::intcycle_w<3>));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(XTAL(18'432'000)/3, 384, 48, 328, 264, 16, 240);
	m_screen->set_screen_update(FUNC(popper_state::screen_update));
	m_screen->set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_popper);

	PALETTE(config, "palette", FUNC(popper_state::popper_palette), 64);

	// audio hardware
	SPEAKER(config, "mono").front_center();

	AY8910(config, m_ay[0], XTAL(18'432'000)/3/2/2).add_route(ALL_OUTPUTS, "mono", 0.25);

	AY8910(config, m_ay[1], XTAL(18'432'000)/3/2/2).add_route(ALL_OUTPUTS, "mono", 0.25);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( popper )
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("p1", 0x0000, 0x2000, CRC(56881b70) SHA1(d3ade7a54a6cb8a0babf0d667a6b27f492a739dc))
	ROM_LOAD("p2", 0x2000, 0x2000, CRC(a054d9d2) SHA1(fcd86e7247b40cf07ea595a64c104b99b0e93ced))
	ROM_LOAD("p3", 0x4000, 0x2000, CRC(6201928a) SHA1(53b571b9f2c0568f10cd974641863c2e00777b46))

	ROM_REGION(0x2000, "subcpu", 0)
	ROM_LOAD("p0", 0x0000, 0x1000, CRC(ef5f7c5b) SHA1(c63a3d9ef2868ad7eaacddec810d62d2e124dc15))

	ROM_REGION(0x2000, "tiles", 0)
	ROM_LOAD("p4", 0x0000, 0x2000, CRC(86203349) SHA1(cce2dd3fa786c2fb3ca80e7b93adf94db3b46b01))

	ROM_REGION(0x4000, "sprites", 0)
	ROM_LOAD("p5", 0x0000, 0x2000, CRC(a21ac194) SHA1(2c0e3df8981a12d383b1c4619a0b95a7c2d176a7))
	ROM_LOAD("p6", 0x2000, 0x2000, CRC(d99fa790) SHA1(201271ee4fb812236a38cb5f9070ac29e8186097))

	ROM_REGION(0x40, "colors", 0)
	ROM_LOAD("p.m3", 0x00, 0x20, CRC(713217aa) SHA1(6083c3432bf94c9e983fcc79171529f519c86105))
	ROM_LOAD("p.m4", 0x20, 0x20, CRC(384de5c1) SHA1(892c89a01c11671c5708113b4e4c27b84be37ea6))
ROM_END

} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME    PARENT  MACHINE  INPUT   CLASS         INIT        ROTATION  COMPANY                     FULLNAME  FLAGS
GAME( 1983, popper, 0,      popper,  popper, popper_state, empty_init, ROT90,    "Omori Electric Co., Ltd.", "Popper", MACHINE_SUPPORTS_SAVE )
