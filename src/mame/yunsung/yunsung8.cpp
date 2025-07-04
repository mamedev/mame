// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

                          -= Yun Sung 8 Bit Games =-

                    driver by   Luca Elia (l.elia@tin.it)


Main  CPU    :  Z80B
Sound CPU    :  Z80A
Video Chips  :  ?
Sound Chips  :  OKI M5205 + YM3812

---------------------------------------------------------------------------
Year + Game         Board#
---------------------------------------------------------------------------
95  Cannon Ball      YS-ROCK-970712 or 940712?
95  Magix / Rock     YS-ROCK-940712
94? Rock Tris        YS-ROCK-940712
---------------------------------------------------------------------------

Notes:

- "Magix" can change title to "Rock" through a DSW
- In service mode press Service Coin (e.g. '9')

***************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/74157.h"
#include "machine/gen_latch.h"
#include "sound/msm5205.h"
#include "sound/ymopl.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class yunsung8_state : public driver_device
{
public:
	yunsung8_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_tileram(*this, "tileram.%u", 0U, 0x1000U, ENDIANNESS_LITTLE),
		m_colorram(*this, "colorram.%u", 0U, 0x800U, ENDIANNESS_LITTLE),
		m_paletteram(*this, "paletteram.%u", 0U, 0x800U, ENDIANNESS_LITTLE),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_msm(*this, "msm"),
		m_adpcm_select(*this, "adpcm_select"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_mainbank(*this, "mainbank"),
		m_soundbank(*this, "soundbank"),
		m_tile_view(*this, "tile_view"),
		m_palette_view(*this, "palette_view")
	{
	}

	void yunsung8(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// video-related
	tilemap_t *m_tilemap[2];
	memory_share_array_creator<uint8_t, 2> m_tileram;
	memory_share_array_creator<uint8_t, 2> m_colorram;
	memory_share_array_creator<uint8_t, 2> m_paletteram;
	uint8_t m_layers_ctrl;

	// misc
	bool m_toggle;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<msm5205_device> m_msm;
	required_device<ls157_device> m_adpcm_select;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	// memory
	required_memory_bank m_mainbank;
	required_memory_bank m_soundbank;
	memory_view m_tile_view;
	memory_view m_palette_view;

	void bankswitch_w(uint8_t data);
	void main_irq_ack_w(uint8_t data);
	void videobank_w(uint8_t data);
	template <uint8_t Which> void tileram_w(offs_t offset, uint8_t data);
	template <uint8_t Which> void colorram_w(offs_t offset, uint8_t data);
	template <uint8_t Which> void paletteram_w(offs_t offset, uint8_t data);
	void flipscreen_w(uint8_t data);
	void sound_bankswitch_w(uint8_t data);
	void adpcm_int(int state);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
	void port_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


/***************************************************************************

Note:   if MAME_DEBUG is defined, pressing Z with:

        Q       shows the background layer
        W       shows the foreground layer

        [ 2 Fixed Layers ]

            [ Background ]

            Layer Size:             512 x 256
            Tiles:                  8 x 8 x 8

            [ Foreground ]

            Layer Size:             512 x 256
            Tiles:                  8 x 8 x 4


        There are no sprites.

***************************************************************************/

/***************************************************************************

                                Memory Handlers

***************************************************************************/

void yunsung8_state::videobank_w(uint8_t data)
{
	//  Bit 1 of the bankswitching register controls the c000-c7ff area (Palette). Bit 0 controls the c800-dfff area (Tiles)

	m_palette_view.select(BIT(data, 1));
	m_tile_view.select(BIT(data, 0));
}

template <uint8_t Which>
void yunsung8_state::paletteram_w(offs_t offset, uint8_t data)
{

	m_paletteram[Which ^ 1][offset] = data;
	int color = m_paletteram[Which ^ 1][offset & ~1] | (m_paletteram[Which ^ 1][offset | 1] << 8);

	// BBBBBGGGGGRRRRRx
	m_palette->set_pen_color(offset / 2 + (Which ? 0x400 : 0), pal5bit(color >> 0), pal5bit(color >> 5), pal5bit(color >> 10));
}

template <uint8_t Which>
void yunsung8_state::colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[Which ^ 1][offset] = data;
	m_tilemap[Which ^ 1]->mark_tile_dirty(offset);
}

template <uint8_t Which>
void yunsung8_state::tileram_w(offs_t offset, uint8_t data)
{
	m_tileram[Which ^ 1][offset] = data;
	m_tilemap[Which ^ 1]->mark_tile_dirty(offset / 2);
}

void yunsung8_state::flipscreen_w(uint8_t data)
{
	machine().tilemap().set_flip_all((data & 1) ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0);
}


/***************************************************************************

                              [ Tiles Format ]

    Offset:

    Video RAM + 0000.b      Code (Low  Bits)
    Video RAM + 0001.b      Code (High Bits)

    Color RAM + 0000.b      Color


***************************************************************************/

// Background

#define DIM_NX_0            (0x40)
#define DIM_NY_0            (0x20)

TILE_GET_INFO_MEMBER(yunsung8_state::get_bg_tile_info)
{
	int code  =  m_tileram[0][tile_index * 2 + 0] + m_tileram[0][tile_index * 2 + 1] * 256;
	int color =  m_colorram[0][tile_index] & 0x07;

	tileinfo.set(0,
			code,
			color,
			0);
}

// Text Plane

#define DIM_NX_1            (0x40)
#define DIM_NY_1            (0x20)

TILE_GET_INFO_MEMBER(yunsung8_state::get_fg_tile_info)
{
	int code  =  m_tileram[1][tile_index * 2 + 0] + m_tileram[1][tile_index * 2 + 1] * 256;
	int color =  m_colorram[1][tile_index] & 0x3f;

	tileinfo.set(1,
			code,
			color,
			0);
}


/***************************************************************************


                            Video Hardware Init


***************************************************************************/

void yunsung8_state::video_start()
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(yunsung8_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, DIM_NX_0, DIM_NY_0 ); // BG
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(yunsung8_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, DIM_NX_1, DIM_NY_1 ); // FG

	m_tilemap[1]->set_transparent_pen(0);
}



/***************************************************************************


                                Screen Drawing


***************************************************************************/

uint32_t yunsung8_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int layers_ctrl = (~m_layers_ctrl) >> 4;

#ifdef MAME_DEBUG
if (machine().input().code_pressed(KEYCODE_Z))
{
	int msk = 0;
	if (machine().input().code_pressed(KEYCODE_Q))  msk |= 1;
	if (machine().input().code_pressed(KEYCODE_W))  msk |= 2;
	if (msk != 0) layers_ctrl &= msk;
}
#endif

	if (layers_ctrl & 1)
		m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
	else
		bitmap.fill(0, cliprect);

	if (layers_ctrl & 2)
		m_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}


/***************************************************************************


                            Memory Maps - Main CPU


***************************************************************************/


void yunsung8_state::bankswitch_w(uint8_t data)
{
	m_layers_ctrl = data & 0x30;    // Layers enable

	m_mainbank->set_entry(data & 0x07);

	if (data & ~0x37)
		logerror("CPU #0 - PC %04X: Bank %02X\n", m_maincpu->pc(), data);
}

void yunsung8_state::main_irq_ack_w(uint8_t data)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
}


void yunsung8_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x0001, 0x0001).w(FUNC(yunsung8_state::bankswitch_w));    // ROM Bank (again?)
	map(0x8000, 0xbfff).bankr(m_mainbank);
	map(0xc000, 0xc7ff).view(m_palette_view);
	m_palette_view[0](0xc000, 0xc7ff).ram().share(m_paletteram[0]).w(FUNC(yunsung8_state::paletteram_w<0>));
	m_palette_view[1](0xc000, 0xc7ff).ram().share(m_paletteram[1]).w(FUNC(yunsung8_state::paletteram_w<1>));
	map(0xc800, 0xdfff).view(m_tile_view);
	m_tile_view[0](0xc800, 0xcfff).ram().share(m_colorram[0]).w(FUNC(yunsung8_state::colorram_w<0>));
	m_tile_view[1](0xc800, 0xcfff).ram().share(m_colorram[1]).w(FUNC(yunsung8_state::colorram_w<1>));
	m_tile_view[0](0xd000, 0xdfff).ram().share(m_tileram[0]).w(FUNC(yunsung8_state::tileram_w<0>));
	m_tile_view[1](0xd000, 0xdfff).ram().share(m_tileram[1]).w(FUNC(yunsung8_state::tileram_w<1>));
	map(0xe000, 0xffff).ram();
}


void yunsung8_state::port_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("SYSTEM").w(FUNC(yunsung8_state::videobank_w));
	map(0x01, 0x01).portr("P1").w(FUNC(yunsung8_state::bankswitch_w)); // ROM Bank + Layers Enable
	map(0x02, 0x02).portr("P2").w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x03, 0x03).portr("DSW1");
	map(0x04, 0x04).portr("DSW2");
	map(0x06, 0x06).w(FUNC(yunsung8_state::flipscreen_w));
	map(0x07, 0x07).w(FUNC(yunsung8_state::main_irq_ack_w));
}



/***************************************************************************


                            Memory Maps - Sound CPU


***************************************************************************/

void yunsung8_state::sound_bankswitch_w(uint8_t data)
{
	m_msm->reset_w(data & 0x20);

	m_soundbank->set_entry(data & 0x07);

	if (data != (data & (~0x27)))
		logerror("%s: Bank %02X\n", machine().describe_context(), data);
}



void yunsung8_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_soundbank);
	map(0xe000, 0xe000).w(FUNC(yunsung8_state::sound_bankswitch_w));
	map(0xe400, 0xe400).w(m_adpcm_select, FUNC(ls157_device::ba_w));
	map(0xec00, 0xec01).w("ymsnd", FUNC(ym3812_device::write));
	map(0xf000, 0xf7ff).ram();
	map(0xf800, 0xf800).r("soundlatch", FUNC(generic_latch_8_device::read));
}




/***************************************************************************


                                Input Ports


***************************************************************************/


/***************************************************************************
                                    Rock Tris
***************************************************************************/
static INPUT_PORTS_START( rocktris )
	PORT_START("SYSTEM")
	PORT_BIT(  0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT(  0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(  0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x20, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_IMPULSE(1)
	PORT_BIT(  0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)

	PORT_START("P1")
	PORT_BIT(  0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) // Bomb
	PORT_BIT(  0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) // Rotate
	PORT_BIT(  0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT(  0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT(  0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT(  0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)

	PORT_START("P2")
	PORT_BIT(  0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) // Bomb
	PORT_BIT(  0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) // Rotate
	PORT_BIT(  0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT(  0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT(  0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT(  0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)

	PORT_START("DSW1")
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x06, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_4C ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Unknown 2-0" )   // the rest seems unused
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 2-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 2-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 2-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 2-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 2-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 2-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 2-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

/***************************************************************************
                                    Magix
***************************************************************************/
static INPUT_PORTS_START( magix )
	PORT_INCLUDE(rocktris) // BTN1 = Rotate, BTN2 = Rotate (Again! ...same dir as BTN1)

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Title" )
	PORT_DIPSETTING(    0x01, "Magix" )
	PORT_DIPSETTING(    0x00, "Rock" )
INPUT_PORTS_END

/***************************************************************************
                                Cannon Ball
***************************************************************************/
static INPUT_PORTS_START( cannball )
	PORT_INCLUDE(rocktris)

	PORT_MODIFY("P1")
	PORT_BIT(  0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) // BTN1 = Launch, BTN2 = Rotate, BTN3 = Bomb

	PORT_MODIFY("P2")
	PORT_BIT(  0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) // BTN1 = Launch, BTN2 = Rotate, BTN3 = Bomb

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x0c, 0x0c, "Bombs" )
	PORT_DIPSETTING(    0x04, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x00, "4" )
INPUT_PORTS_END

static INPUT_PORTS_START( cannbalv )
	PORT_INCLUDE(cannball)

	PORT_MODIFY("SYSTEM")
	PORT_BIT(  0x40, IP_ACTIVE_HIGH, IPT_CUSTOM  ) // always activated, otherwise the game resets. a simple check for horizontal / vertical version of the game?
INPUT_PORTS_END

/***************************************************************************


                                Graphics Layouts


***************************************************************************/

// 8x8x4 tiles in 2 ROMs
static const gfx_layout layout_8x8x4 =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ STEP4(0,1) },
	{ RGN_FRAC(1,2)+1*4,RGN_FRAC(1,2)+0*4,1*4,0*4, RGN_FRAC(1,2)+3*4,RGN_FRAC(1,2)+2*4,3*4,2*4},
	{ STEP8(0,16) },
	8*8*4/2
};

// 8x8x8 tiles in 4 ROMs
static const gfx_layout layout_8x8x8 =
{
	8,8,
	RGN_FRAC(1,4),
	8,
	{ STEP8(0,1) },
	{ RGN_FRAC(0,4) + 0*8, RGN_FRAC(1,4) + 0*8, RGN_FRAC(2,4) + 0*8, RGN_FRAC(3,4) + 0*8,
		RGN_FRAC(0,4) + 1*8, RGN_FRAC(1,4) + 1*8, RGN_FRAC(2,4) + 1*8, RGN_FRAC(3,4) + 1*8 },
	{ STEP8(0,16) },
	8*8*8/4
};

static GFXDECODE_START( gfx_yunsung8 )
	GFXDECODE_ENTRY( "bgfx", 0, layout_8x8x8, 0, 0x08 )
	GFXDECODE_ENTRY( "text", 0, layout_8x8x4, 0, 0x40 )
GFXDECODE_END



/***************************************************************************


                                Machine Drivers


***************************************************************************/


void yunsung8_state::adpcm_int(int state)
{
	if (!state)
		return;

	m_toggle = !m_toggle;
	m_adpcm_select->select_w(m_toggle);
	m_audiocpu->set_input_line(INPUT_LINE_NMI, m_toggle);
}

void yunsung8_state::machine_start()
{
	m_mainbank->configure_entries(0, 8, memregion("maincpu")->base(), 0x4000);
	m_soundbank->configure_entries(0, 8, memregion("audiocpu")->base(), 0x4000);

	save_item(NAME(m_layers_ctrl));
	save_item(NAME(m_toggle));
}

void yunsung8_state::machine_reset()
{
	m_layers_ctrl = 0;
	m_toggle = false;
}


void yunsung8_state::yunsung8(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, XTAL(16'000'000) / 2);     // Z80B @ 8MHz?
	m_maincpu->set_addrmap(AS_PROGRAM, &yunsung8_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &yunsung8_state::port_map);
	m_maincpu->set_vblank_int("screen", FUNC(yunsung8_state::irq0_line_assert));    // No NMI routine

	Z80(config, m_audiocpu, XTAL(16'000'000) / 4);    // ?
	m_audiocpu->set_addrmap(AS_PROGRAM, &yunsung8_state::sound_map);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(16'000'000) / 2, 512, 64, 512-64, 262, 8, 256-8); // TODO: completely inaccurate
	screen.set_screen_update(FUNC(yunsung8_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_yunsung8);
	PALETTE(config, m_palette).set_entries(2048);

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	GENERIC_LATCH_8(config, "soundlatch").data_pending_callback().set_inputline(m_audiocpu, 0);

	ym3812_device &ymsnd(YM3812(config, "ymsnd", XTAL(16'000'000) / 4));
	ymsnd.add_route(ALL_OUTPUTS, "speaker", 1.0, 0);
	ymsnd.add_route(ALL_OUTPUTS, "speaker", 1.0, 1);

	LS157(config, m_adpcm_select, 0);
	m_adpcm_select->out_callback().set("msm", FUNC(msm5205_device::data_w));

	MSM5205(config, m_msm, XTAL(400'000)); // verified on PCB
	m_msm->vck_legacy_callback().set(FUNC(yunsung8_state::adpcm_int)); // interrupt function
	m_msm->set_prescaler_selector(msm5205_device::S96_4B);  // 4KHz, 4 Bits
	m_msm->add_route(ALL_OUTPUTS, "speaker", 0.80, 0);
	m_msm->add_route(ALL_OUTPUTS, "speaker", 0.80, 1);
}


/***************************************************************************


                                ROMs Loading


***************************************************************************/

/***************************************************************************

Magix
Yun Sung, 1995

YS-ROCK-940712
+-------------------------------------+
|VOL YM3014      6116               04|
|     M5205 400KHz                  03|
|                09                   |
|     Z80A      CXK5118PN-15L       02|
|      08       GM76C28-10          01|
|    MCM2018AN45                      |
|J   MCM2018AN45                      |
|A DSW1       +--------+              |
|M            | Quick  |              |
|M DSW2*      | Logic  |              |
|A            |QL12x16B|              |
|             | OPL84C |              |
| U66         +--------+            06|
|    HM6264                         05|
|     07     HM6264                   |
|    Z80B    YM3812   10      16MHz   |
+-------------------------------------+

 Main CPU: Z80B
Sound CPU: Z80A
    Sound: Yamaha YM3812 + Oki M5205 + YM3014 DAC
    Video: QuickLogic QL12x16B-OPL84C FPGA
      OSC: 16MHz + 400Khz resonator
   Memory: 2 x MCM2018AN45, 2 x HM6264, CXK5118PN-15L, GM76C28-10 & 6116
     Misc: DSW1 is a 8 position dipswitch
           DSW2 is not populated
           VOL Volume pot

***************************************************************************/

ROM_START( magix )
	ROM_REGION( 0x20000, "maincpu", 0 )     // Z80 code
	ROM_LOAD( "yunsung8.07", 0x00000, 0x20000, CRC(d4d0b68b) SHA1(d7e1fb57a14f8b822791b98cecc6d5a053a89e0f) )

	ROM_REGION( 0x20000, "audiocpu", 0 )        // Z80 code
	ROM_LOAD( "yunsung8.08", 0x00000, 0x20000, CRC(6fd60be9) SHA1(87622dc2967842629e90a02b415bec86cc26cbc7) )

	ROM_REGION( 0x200000, "bgfx", 0 )
	ROM_LOAD( "yunsung8.04", 0x000000, 0x80000, CRC(0a100d2b) SHA1(c36a2489748c8ac7b6d7457ad09d8153707c85be) )
	ROM_LOAD( "yunsung8.03", 0x080000, 0x80000, CRC(c8cb0373) SHA1(339c4e0fef44da3cab615e07dc8739bd925ebf28) )
	ROM_LOAD( "yunsung8.02", 0x100000, 0x80000, CRC(09efb8e5) SHA1(684bb5c4b579f8c77e79aab4decbefea495d9474) )
	ROM_LOAD( "yunsung8.01", 0x180000, 0x80000, CRC(4590d782) SHA1(af875166207793572b9ecf01bb6a24feba562a96) )

	ROM_REGION( 0x40000, "text", 0 )
	ROM_LOAD( "yunsung8.05", 0x00000, 0x20000, CRC(862d378c) SHA1(a4e2cf14b5b25c6b8725dd285ddea65ce9ee257a) )   // only first $8000 bytes != 0
	ROM_LOAD( "yunsung8.06", 0x20000, 0x20000, CRC(8b2ab901) SHA1(1a5c05dd0cf830b645357a62d8e6e876b44c6b7f) )   // only first $8000 bytes != 0

	ROM_REGION( 0x0002, "plds", 0 )
	ROM_LOAD( "palce20v8h.09", 0x0000, 0x0001, NO_DUMP ) // PALCE20V8H-25PC/4 - security fuse blown
	ROM_LOAD( "palce20v8h.10", 0x0000, 0x0001, NO_DUMP ) // PALCE20V8H-25PC/4 - security fuse blown
ROM_END

/***************************************************************************

Magix / Rock

Original Yun Sung board, but has EPROMs with open windows and handwritten
numbers on them. "Yun Sung 1995" logo has been removed from the text tiles.
Code is different, shifted around not patched.

***************************************************************************/

ROM_START( magixb )
	ROM_REGION( 0x20000, "maincpu", 0 )     // Z80 code
	ROM_LOAD( "8.bin", 0x00000, 0x20000, CRC(3b92020f) SHA1(edc15c5b712774dad1685ce9a94e4290aab9934a) )

	ROM_REGION( 0x20000, "audiocpu", 0 )        // Z80 code
	ROM_LOAD( "9.bin", 0x00000, 0x20000, CRC(6fd60be9) SHA1(87622dc2967842629e90a02b415bec86cc26cbc7) ) // yunsung8.08

	ROM_REGION( 0x200000, "bgfx", 0 )
	ROM_LOAD( "1.bin", 0x000000, 0x80000, CRC(0a100d2b) SHA1(c36a2489748c8ac7b6d7457ad09d8153707c85be) ) // yunsung8.04
	ROM_LOAD( "2.bin", 0x080000, 0x80000, CRC(c8cb0373) SHA1(339c4e0fef44da3cab615e07dc8739bd925ebf28) ) // yunsung8.03
	ROM_LOAD( "3.bin", 0x100000, 0x80000, CRC(09efb8e5) SHA1(684bb5c4b579f8c77e79aab4decbefea495d9474) ) // yunsung8.02
	ROM_LOAD( "4.bin", 0x180000, 0x80000, CRC(4590d782) SHA1(af875166207793572b9ecf01bb6a24feba562a96) ) // yunsung8.01

	ROM_REGION( 0x40000, "text", 0 )
	ROM_LOAD( "5.bin", 0x00000, 0x20000, CRC(11b99819) SHA1(4b20feea227cefd2e905601d934538a13ba6685b) ) // only first $8000 bytes != 0
	ROM_LOAD( "6.bin", 0x20000, 0x20000, CRC(361a864c) SHA1(e0bb78b49fc3d461d6ac46ad97a9d04112783132) ) // only first $8000 bytes != 0

	ROM_REGION( 0x0002, "plds", 0 )
	ROM_LOAD( "palce20v8h.09", 0x0000, 0x0001, NO_DUMP ) // PALCE20V8H-25PC/4 - security fuse blown
	ROM_LOAD( "palce20v8h.10", 0x0000, 0x0001, NO_DUMP ) // PALCE20V8H-25PC/4 - security fuse blown
ROM_END


/***************************************************************************

Cannon Ball
Yun Sung, 1995

Cannon Ball (vertical)
+-------------------------------------+
|VOL YM3014      6116         YunSung7|
|     M5205 400KHz            YunSung6|
|     Z80A      CXK5118PN-15L YunSung5|
|    YunSung8   GM76C28-10    YunSung4|
|    MCM2018AN45                      |
|J   MCM2018AN45                      |
|A DSW1       +--------+              |
|M            |Cy7C384A|              |
|M DSW2*      |XJC 9506|              |
|A            | CYP    |              |
|             | 001002 |              |
| U66         +--------+      YunSung3|
|    HM6264                   YunSung2|
|   YunSung1 HM6264                   |
|    Z80B    YM3812           16MHz   |
+-------------------------------------+

 Main CPU: Z80B
Sound CPU: Z80A
    Sound: Yamaha YM3812 + Oki M5205 + YM3014 DAC
    Video: Cypress CY7C384A - Very high speed 6K gate CMOS FPGA
      OSC: 16MHz + 400Khz resonator
   Memory: 2 x MCM2018AN45, 2 x HM6264, CXK5118PN-15L, GM76C28-10 & 6116
     Misc: DSW1 is a 8 position dipswitch
           DSW2 is not populated
           VOL Volume pot

01, 02, 03, 04  are 27c020
05, 06, 07, 08  are 27c010
2 pals used

***************************************************************************/

ROM_START( cannball )
	ROM_REGION( 0x20000, "maincpu", 0 )     // Z80 code
	ROM_LOAD( "cannball.07", 0x00000, 0x20000, CRC(17db56b4) SHA1(032e3dbde0b0e315dcb5f2b31f57e75e78818f2d) )

	ROM_REGION( 0x20000, "audiocpu", 0 )        // Z80 code
	ROM_LOAD( "cannball.08", 0x00000, 0x20000, CRC(11403875) SHA1(9f583bc4f08e7aef3fd0f3fe3f31cce1d226641a) )

	ROM_REGION( 0x100000, "bgfx", 0 )
	ROM_LOAD( "cannball.01", 0x000000, 0x40000, CRC(2d7785e4) SHA1(9911354c0be192506f8bfca3e85ede0bbc4828d5) )
	ROM_LOAD( "cannball.02", 0x040000, 0x40000, CRC(24df387e) SHA1(5f4afe11feb367ca3b3c4f5eb37a6b6c4edb83bb) )
	ROM_LOAD( "cannball.03", 0x080000, 0x40000, CRC(4d62f192) SHA1(8c60b9b4b36c13c2d145c49413580a10e71eb283) )
	ROM_LOAD( "cannball.04", 0x0c0000, 0x40000, CRC(37cf8b12) SHA1(f93df8e0babe2c4ec996aa3c2a48bf40a5a02e62) )

	ROM_REGION( 0x40000, "text", 0 )
	ROM_LOAD( "cannball.05", 0x00000, 0x20000, CRC(87c1f1fa) SHA1(dbc568d2133734e41b69fd8d18b76531648b32ef) )
	ROM_LOAD( "cannball.06", 0x20000, 0x20000, CRC(e722bee8) SHA1(3aed7df9df81a6776b6bf2f5b167965b0d689216) )
ROM_END


ROM_START( cannballv )
	ROM_REGION( 0x20000, "maincpu", 0 )     // Z80 code
	ROM_LOAD( "yunsung1", 0x00000, 0x20000, CRC(f7398b0d) SHA1(f2cdb9c4662cd325376d25ae9611f689605042db) )

	ROM_REGION( 0x20000, "audiocpu", 0 )        // Z80 code
	ROM_LOAD( "yunsung8", 0x00000, 0x20000, CRC(11403875) SHA1(9f583bc4f08e7aef3fd0f3fe3f31cce1d226641a) )

	ROM_REGION( 0x200000, "bgfx", 0 )
	ROM_LOAD( "yunsung7", 0x000000, 0x80000, CRC(a5f1a648) SHA1(7a5bf5bc0ad257ccb12104512e98dfb3525babfc) )
	ROM_LOAD( "yunsung6", 0x080000, 0x80000, CRC(8baa686e) SHA1(831c3e2864d262bf5429dca6653c83dc976e610e) )
	ROM_LOAD( "yunsung5", 0x100000, 0x80000, CRC(a7f2ce51) SHA1(81632aca067f2c8c45488266c4489d9af24fb552) )
	ROM_LOAD( "yunsung4", 0x180000, 0x80000, CRC(74bef793) SHA1(6208580ce747cec3d410ce3c71e07aa570b9121d) )

	ROM_REGION( 0x40000, "text", 0 )
	ROM_LOAD( "yunsung3", 0x00000, 0x20000, CRC(8217abbe) SHA1(1a459a816a1aa5b68858e39c4a21bd78ee78dcab) )
	ROM_LOAD( "yunsung2", 0x20000, 0x20000, CRC(76de1045) SHA1(a3845ee1874e6ec0ce26e6e73e4643243779e70d) )
ROM_END


/***************************************************************************

Rock Tris by Yunsung

YS-ROCK-970712
+-------------------------------------+
|VOL YM3014      6116               04|
|     M5205 400KHz                  03|
|     Z80A      CXK5118PN-15L       02|
|      08       GM76C28-10          01|
|    MCM2018AN45                      |
|J   MCM2018AN45                      |
|A DSW1       +--------+              |
|M            | Quick  |              |
|M DSW2*      | Logic  |              |
|A            |QL12x16B|              |
|             | OPL84C |              |
| U66         +--------+            06|
|    HM6264                         05|
|     07     HM6264                   |
|    Z80B    YM3812           16MHz   |
+-------------------------------------+

 Main CPU: Z80B
Sound CPU: Z80A
    Sound: Yamaha YM3812 (marked as UA010) + Oki M5205 + YM3014 DAC
    Video: QuickLogic QL12x16B-OPL84C FPGA
      OSC: 16MHz + 400Khz resonator
   Memory: 2 x MCM2018AN45, 2 x HM6264, CXK5118PN-15L, GM76C28-10 & 6116
     Misc: DSW1 is a 8 position dipswitch
           DSW2 is not populated
           VOL Volume pot

***************************************************************************/

ROM_START( rocktris )
	ROM_REGION( 0x20000, "maincpu", 0 )     // Z80 code
	ROM_LOAD( "cpu07.bin", 0x00000, 0x20000, CRC(46e3b79c) SHA1(81a587b9f986c4e39b1888ec6ed6b86d1469b9a0) )

	ROM_REGION( 0x20000, "audiocpu", 0 )        // Z80 code
	ROM_LOAD( "cpu08.bin", 0x00000, 0x20000, CRC(3a78a4cf) SHA1(f643c7a217cbb71f3a03f1f4a16545c546332819) )

	ROM_REGION( 0x200000, "bgfx", 0 )
	ROM_LOAD( "gfx4.bin", 0x000000, 0x80000, CRC(abb49cac) SHA1(e2d766e950df398a8ec8b6888e128ffc3bdf1ce9) )
	ROM_LOAD( "gfx3.bin", 0x080000, 0x80000, CRC(70a6ad52) SHA1(04cd58d3f885dd7c2fb1061f93d3ae3a418ad762) )
	ROM_LOAD( "gfx2.bin", 0x100000, 0x80000, CRC(fcc9ec97) SHA1(1f09452988e3fa976b233e3b458c7a60977b76aa) )
	ROM_LOAD( "gfx1.bin", 0x180000, 0x80000, CRC(4295034d) SHA1(9bdbbcdb46eb659a13b77c5bb26c9d8ad43731a7) )


	ROM_REGION( 0x40000, "text", 0 )
	ROM_LOAD( "gfx5.bin", 0x00000, 0x20000, CRC(058ee379) SHA1(57088bb02c56212979b9119b773eedc31af17e50) )
	ROM_LOAD( "gfx6.bin", 0x20000, 0x20000, CRC(593cbd39) SHA1(4d60b5811118f3f22f6f3b300a4daec158456b72) )
ROM_END

} // anonymous namespace


/***************************************************************************


                                Game Drivers


***************************************************************************/

GAME( 1995,  cannball,  0,        yunsung8, cannball, yunsung8_state, empty_init, ROT0,   "Yun Sung / Soft Vision",    "Cannon Ball (Yun Sung, horizontal)",  MACHINE_SUPPORTS_SAVE )
GAME( 1995,  cannballv, cannball, yunsung8, cannbalv, yunsung8_state, empty_init, ROT270, "Yun Sung / J&K Production", "Cannon Ball (Yun Sung, vertical)",    MACHINE_SUPPORTS_SAVE )
GAME( 1995,  magix,     0,        yunsung8, magix,    yunsung8_state, empty_init, ROT0,   "Yun Sung",                  "Magix / Rock",                        MACHINE_SUPPORTS_SAVE )
GAME( 1995,  magixb,    magix,    yunsung8, magix,    yunsung8_state, empty_init, ROT0,   "Yun Sung",                  "Magix / Rock (no copyright message)", MACHINE_SUPPORTS_SAVE ) // was marked as bootleg, but has been seen on original PCBs
GAME( 1994?, rocktris,  0,        yunsung8, rocktris, yunsung8_state, empty_init, ROT0,   "Yun Sung",                  "Rock Tris",                           MACHINE_SUPPORTS_SAVE )
