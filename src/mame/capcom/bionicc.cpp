// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino, Paul Leaman, Dirk Best
// thanks-to: Steven Frew (the author of Slutte)
/***************************************************************************

    Bionic Commando

    © 1987 Capcom

    Hardware:

        Main CPU board: 86612-A-2
    Graphics ROM board: 86612-B-2
     Program ROM board: 86612-C-2

      Main CPU: 68000CP10        @ 24MHz / 2 = 12MHz
     Sound CPU: Z80A             @ 14.31818 / 4 = 3.579545MHz
           MCU: Intel C8751H-88  @ 24MHz / 4 = 6MHz
    Sound Chip: YM2151 & YM3012
           OSC: 24.000 MHz (on the 86612-B-2 PCB)
        Custom: CAPCOM DL-010D-103 (on the 86612-B-2 PCB)

    Video timings:

    Horizontal scan rate: 15.606kHz
    Vertical scan rate: 60.024Hz

    pixel clock:         6.000MHz, 166ns per pixel

    htotal:             64.076us, 386 pixels
    hsync:               5.312us,  32 pixels
    back porch + sync:  15.106us,  91 pixels
    active video:       42.662us, 257 pixels (first pixel is repeated?)
    front porch:         6.308us,  38 pixels

    vtotal:             16.660ms, 260 lines
    vsync:             256.304us,   4 lines
    back porch + sync:   1.282ms,  20 lines
    active video:       14.353ms, 224 lines
    front porch:         1.025ms,  16 lines

    Clocks verified on 86612-A-2 and 86612-B-2 boards, serial no. 39646
    ("Bionic Commando", US region) by scope measurement at clock pins.
    Timings verified at SYNC pin and BLUE pin (jamma edge),
    using an Agilent DSO9404A scope and two N2873A 500MHz probes

    BTANB [MT00209] (verified on real PCB):
    - misplaced sprites (see beginning of level 1 or 2 for example)
    - sprite / sprite priority (see level 2 the reflectors)
    - sprite / background priority (see level 1: birds walk through
      branches of different trees)
    - see the beginning of level 3: background screwed
    - gray tiles around the title in Top Secret

    Notes:
    - Protection MCU is labelled "TS" without a number and without a coloured.
      Maybe its code is not region dependant.
    - The MCU rom contains the string
      "<for dealer-location test & USA show. 87/03/10 >"
      which indicates it could be from an earlier version, especially with it
      coming from a 'Top Secret' bootleg with identical program but unprotected
      MCU, however f1dream has a similar string, and is verified as being from
      a production board.
    - Euro rom labels (IE: "TSE") had a blue stripe, while those labeled
      as USA (TSU) had an red stripe on the sticker. The intermixing
      of TSE and TSU roms in the parent set is correct and verified.
    - Euro set simply states the game cannot be operated in Japan.
    - IRQ 4 is control related. On each interrupt, it reads 0xFE4000
      (coin/start), shift the bits around and move the resulting byte into a
      dword RAM location. The dword RAM location is rotated by 8 bits each time
      this happens. This is probably done to be pedantic about coin insertions
      (might be protection related).

    TODO:
    - Firing IRQ4 at line 16 causes the game to often miss coin inserts. Set
      to 128 currently to compensate.
    - The game doesn't set the coin lockout in service mode, so the coin inputs
      can't be tested there if you uncomment and enable it.

***************************************************************************/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "cpu/mcs51/mcs51.h"
#include "cpu/z80/z80.h"
#include "machine/timer.h"
#include "video/bufsprite.h"
#include "tigeroad_spr.h"
#include "sound/ymopm.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bionicc_state : public driver_device
{
public:
	bionicc_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_mcu(*this, "mcu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spritegen(*this, "spritegen"),
		m_spriteram(*this, "spriteram") ,
		m_txvideoram(*this, "txvideoram"),
		m_fgvideoram(*this, "fgvideoram"),
		m_bgvideoram(*this, "bgvideoram")
	{ }

	void bionicc(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<i8751_device> m_mcu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<tigeroad_spr_device> m_spritegen;
	required_device<buffered_spriteram16_device> m_spriteram;

	required_shared_ptr<uint16_t> m_txvideoram;
	required_shared_ptr<uint16_t> m_fgvideoram;
	required_shared_ptr<uint16_t> m_bgvideoram;

	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void mcu_io(address_map &map) ATTR_COLD;

	void output_w(u8 data);

	// video
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	static rgb_t RRRRGGGGBBBBIIII(uint32_t raw);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline);

	void bgvideoram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void fgvideoram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void txvideoram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void scroll_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	tilemap_t   *m_tx_tilemap = nullptr;
	tilemap_t   *m_bg_tilemap = nullptr;
	tilemap_t   *m_fg_tilemap = nullptr;
	uint16_t    m_scroll[4]{};

	// audio
	void audiocpu_nmi_w(u8 data);

	// protection mcu
	u8 m_audiocpu_to_mcu = 0; // ls374 at 4a
	u8 m_mcu_to_audiocpu = 0; // ls374 at 5a
	u8 m_mcu_p1 = 0;
	u8 m_mcu_p3 = 0xff;

	void dmaon_w(u16 data);
	u8 mcu_dma_r(offs_t offset);
	void mcu_dma_w(offs_t offset, u8 data);
	void mcu_p3_w(u8 data);
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void bionicc_state::main_map(address_map &map)
{
	map.global_mask(0xfffff);
	map(0x00000, 0x3ffff).rom();
	map(0xe0000, 0xe07ff).ram(); // RAM?
	map(0xe0800, 0xe0cff).ram().share("spriteram");
	map(0xe0d00, 0xe3fff).ram(); // RAM?
	map(0xe4000, 0xe4000).mirror(0x3ffc).w(FUNC(bionicc_state::output_w));
	map(0xe4000, 0xe4001).mirror(0x3ffc).portr("INPUTS");
	map(0xe4002, 0xe4002).mirror(0x3ffc).w(FUNC(bionicc_state::audiocpu_nmi_w));
	map(0xe4002, 0xe4003).mirror(0x3ffc).portr("DSW");
	map(0xe8010, 0xe8017).w(FUNC(bionicc_state::scroll_w));
	map(0xe8018, 0xe8019).nopw(); // vblank irq ack?
	map(0xe801a, 0xe801b).w(FUNC(bionicc_state::dmaon_w));
	map(0xec000, 0xecfff).mirror(0x3000).ram().w(FUNC(bionicc_state::txvideoram_w)).share("txvideoram");
	map(0xf0000, 0xf3fff).ram().w(FUNC(bionicc_state::fgvideoram_w)).share("fgvideoram");
	map(0xf4000, 0xf7fff).ram().w(FUNC(bionicc_state::bgvideoram_w)).share("bgvideoram");
	map(0xf8000, 0xf87ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xfc000, 0xfffff).ram(); // WRAM
}

void bionicc_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x8001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xa000, 0xa000).lrw8(NAME([this]() { return m_mcu_to_audiocpu; }), NAME([this](u8 data) { m_audiocpu_to_mcu = data; }));
	map(0xc000, 0xc7ff).ram();
}

void bionicc_state::mcu_io(address_map &map)
{
	map.global_mask(0x7ff);
	map(0x000, 0x7ff).rw(FUNC(bionicc_state::mcu_dma_r), FUNC(bionicc_state::mcu_dma_w));
}


//**************************************************************************
//  INPUT PORT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( bionicc )
	PORT_START("INPUTS")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_BUTTON2)        PORT_COCKTAIL
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_BUTTON1)        PORT_COCKTAIL
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_COCKTAIL PORT_8WAY
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_COCKTAIL PORT_8WAY
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  PORT_COCKTAIL PORT_8WAY
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    PORT_COCKTAIL PORT_8WAY
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_BUTTON2)
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_BUTTON1)
	PORT_BIT(0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY
	PORT_BIT(0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_8WAY
	PORT_BIT(0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  PORT_8WAY
	PORT_BIT(0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    PORT_8WAY
	PORT_BIT(0x1000, IP_ACTIVE_LOW, IPT_START2)
	PORT_BIT(0x2000, IP_ACTIVE_LOW, IPT_START1)
	PORT_BIT(0x4000, IP_ACTIVE_LOW, IPT_COIN2)
	PORT_BIT(0x8000, IP_ACTIVE_LOW, IPT_COIN1)

	PORT_START("DSW")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SWB:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SWB:4,5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_6C ) )
	PORT_SERVICE_DIPLOC(  0x0040, IP_ACTIVE_LOW, "SWB:7" )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWA:1,2")
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0200, "4" )
	PORT_DIPSETTING(      0x0100, "5" )
	PORT_DIPSETTING(      0x0000, "7" )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SWA:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x1800, 0x1800, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SWA:4,5")   /* table at 0x00483a */
	PORT_DIPSETTING(      0x1800, "20k 40k 100k 60k+" )
	PORT_DIPSETTING(      0x1000, "30k 50k 120k 70k+" )
	PORT_DIPSETTING(      0x0800, "20k 60k")
	PORT_DIPSETTING(      0x0000, "30k 70k" )
	PORT_DIPNAME( 0x6000, 0x4000, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SWA:6,7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Freeze" )                PORT_DIPLOCATION("SWA:8")     /* Listed as "Unused" */
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


//**************************************************************************
//  PALETTE
//**************************************************************************

rgb_t bionicc_state::RRRRGGGGBBBBIIII(uint32_t raw)
{
	uint8_t bright = (raw & 0x0f);

	uint8_t r = ((raw >> 12) & 0x0f) * 0x11;
	uint8_t g = ((raw >>  8) & 0x0f) * 0x11;
	uint8_t b = ((raw >>  4) & 0x0f) * 0x11;

	if ((bright & 0x08) == 0)
	{
		r = r * (0x07 + bright) / 0x0e;
		g = g * (0x07 + bright) / 0x0e;
		b = b * (0x07 + bright) / 0x0e;
	}

	return rgb_t(r, g, b);
}


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

/*

    This board handles tile/tile and tile/sprite priority with a PROM. Its
    working is complicated and hardcoded in the driver.

    The PROM is a 256x4 chip, with address inputs wired as follows:

    A0 bg opaque
    A1 \
    A2 |  fg pen
    A3 |
    A4 /
    A5 fg has priority over sprites (bit 5 of tile attribute)
    A6 fg has not priority over bg (bits 6 & 7 of tile attribute both set)
    A7 sprite opaque

    The output selects the active layer, it can be:
    0  bg
    1  fg
    2  sprite

*/

void bionicc_state::bgvideoram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_bgvideoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

void bionicc_state::fgvideoram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_fgvideoram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset / 2);
}

void bionicc_state::txvideoram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_txvideoram[offset]);
	m_tx_tilemap->mark_tile_dirty(offset & 0x3ff);
}

void bionicc_state::scroll_w(offs_t offset, u16 data, u16 mem_mask)
{
	data = COMBINE_DATA(&m_scroll[offset]);

	switch (offset)
	{
		case 0:
			m_fg_tilemap->set_scrollx(0, data);
			break;
		case 1:
			m_fg_tilemap->set_scrolly(0, data);
			break;
		case 2:
			m_bg_tilemap->set_scrollx(0, data);
			break;
		case 3:
			m_bg_tilemap->set_scrolly(0, data);
			break;
	}
}

u32 bionicc_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);

	m_fg_tilemap->draw(screen, bitmap, cliprect, 1 | TILEMAP_DRAW_LAYER1, 0);   /* nothing in FRONT */
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0 | TILEMAP_DRAW_LAYER1, 0);
	m_spritegen->draw_sprites(bitmap, cliprect, m_spriteram->buffer(), m_spriteram->bytes(), flip_screen(), false);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0 | TILEMAP_DRAW_LAYER0, 0);

	// text layer last
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

void bionicc_state::video_start()
{
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(bionicc_state::get_tx_tile_info)), TILEMAP_SCAN_ROWS,  8,  8, 32, 32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(bionicc_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(bionicc_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS,  8,  8, 64, 64);

	m_tx_tilemap->set_transparent_pen(3);
	m_fg_tilemap->set_transmask(0, 0xffff, 0x8000); /* split type 0 is completely transparent in front half */
	m_fg_tilemap->set_transmask(1, 0xffc1, 0x803e); /* split type 1 has pens 1-5 opaque in front half */
	m_bg_tilemap->set_transparent_pen(15);

	m_tx_tilemap->set_scrolldx(128, 128);
	m_tx_tilemap->set_scrolldy(  6,   6);
	m_bg_tilemap->set_scrolldx(128, 128);
	m_bg_tilemap->set_scrolldy(  6,   6);
	m_fg_tilemap->set_scrolldx(128, 128);
	m_fg_tilemap->set_scrolldy(  6,   6);
}


//**************************************************************************
//  DRAWGFX LAYOUTS
//**************************************************************************

static const gfx_layout vramlayout=
{
	8,8,    /* 8*8 characters */
	RGN_FRAC(1,1),   /* 1024 character */
	2,      /* 2 bitplanes */
	{ 4,0 },
	{ STEP4(0,1), STEP4(4*2,1) },
	{ STEP8(0,4*2*2) },
	128   /* every character takes 128 consecutive bytes */
};

static const gfx_layout scroll2layout=
{
	8,8,    /* 8*8 tiles */
	RGN_FRAC(1,2),   /* 2048 tiles */
	4,      /* 4 bits per pixel */
	{ RGN_FRAC(1,2)+4,RGN_FRAC(1,2),4,0 },
	{ STEP4(0,1), STEP4(4*2,1) },
	{ STEP8(0,4*2*2) },
	128   /* every tile takes 128 consecutive bytes */
};

static const gfx_layout scroll1layout=
{
	16,16,  /* 16*16 tiles */
	RGN_FRAC(1,2),   /* 2048 tiles */
	4,      /* 4 bits per pixel */
	{ RGN_FRAC(1,2)+4,RGN_FRAC(1,2),4,0 },
	{ STEP4(0,1), STEP4(4*2,1), STEP4(4*2*2*16,1), STEP4(4*2*2*16+4*2,1) },
	{ STEP16(0,4*2*2) },
	512   /* each tile takes 512 consecutive bytes */
};

static GFXDECODE_START( gfx_bionicc )
	GFXDECODE_ENTRY( "gfx1", 0, vramlayout,    768, 64 )    /* colors 768-1023 */
	GFXDECODE_ENTRY( "gfx2", 0, scroll2layout,   0,  8 )    /* colors   0- 127 */
	GFXDECODE_ENTRY( "gfx3", 0, scroll1layout, 256,  4 )    /* colors 256- 319 */
GFXDECODE_END

TILE_GET_INFO_MEMBER(bionicc_state::get_tx_tile_info)
{
	// 76------  tile code high bits
	// --543210  color

	int attr = m_txvideoram[tile_index + 0x400];
	int code = m_txvideoram[tile_index] & 0xff;

	tileinfo.set(0, ((attr & 0xc0) << 2) | code, attr & 0x3f, 0);
}

TILE_GET_INFO_MEMBER(bionicc_state::get_fg_tile_info)
{
	// 7-------  tile flip x
	// -6------  tile flip y
	// --5-----  layer
	// ---43---  color
	// -----210  tile code high bits

	int attr = m_fgvideoram[2 * tile_index + 1];
	int code = m_fgvideoram[2 * tile_index] & 0xff;
	int flag = 0;

	if ((attr & 0xc0) == 0xc0)
	{
		// drawn to the background if both flip bits are set
		tileinfo.category = 1;
		tileinfo.group = 0;
	}
	else
	{
		tileinfo.category = 0;
		tileinfo.group = (attr & 0x20) >> 5;
		flag = TILE_FLIPXY((attr & 0xc0) >> 6);
	}

	tileinfo.set(2, ((attr & 0x07) << 8) | code, (attr & 0x18) >> 3, flag);
}

TILE_GET_INFO_MEMBER(bionicc_state::get_bg_tile_info)
{
	// 7-------  tile flip x
	// -6------  tile flip y
	// --543---  color
	// -----210  tile code high bits

	int attr = m_bgvideoram[2 * tile_index + 1];
	int code = m_bgvideoram[2 * tile_index] & 0xff;
	int flag = TILE_FLIPXY((attr & 0xc0) >> 6);

	tileinfo.set(1, ((attr & 0x07) << 8) | code, (attr & 0x38) >> 3, flag);
}


//**************************************************************************
//  PROTECTION MCU
//**************************************************************************

u8 bionicc_state::mcu_dma_r(offs_t offset)
{
	u8 data = 0xff;

	if (BIT(m_mcu_p3, 5) == 0)
	{
		// various address bits are pulled high because the mcu doesn't drive them
		// the 3 upper address bits (p2.0, p2.1, p2.2) are connected to a14 to a16
		offs_t address = 0xe3e01 | ((offset & 0x700) << 6) | ((offset & 0xff) << 1);
		data = m_maincpu->space(AS_PROGRAM).read_byte(address);
	}

	return data;
}

void bionicc_state::mcu_dma_w(offs_t offset, u8 data)
{
	if (BIT(m_mcu_p3, 5) == 0)
	{
		offs_t address = 0xe3e01 | ((offset & 0x700) << 6) | ((offset & 0xff) << 1);
		m_maincpu->space(AS_PROGRAM).write_byte(address, data);
	}
}

void bionicc_state::mcu_p3_w(u8 data)
{
	// 7-------  read strobe
	// -6------  write strobe
	// --5-----  dma
	// ---4----  int1 ack
	// ----3---  int1
	// -----2--  int0
	// ------1-  int0 flip-flop preset
	// -------0  int0 ack

	if (BIT(m_mcu_p3, 0) == 1 && BIT(data, 0) == 0)
	{
		m_mcu->set_input_line(MCS51_INT0_LINE, CLEAR_LINE);
		m_maincpu->resume(SUSPEND_REASON_HALT);
	}

	if (BIT(m_mcu_p3, 4) == 1 && BIT(data, 4) == 0)
		m_mcu->set_input_line(MCS51_INT1_LINE, CLEAR_LINE);

	if (BIT(m_mcu_p3, 6) == 1 && BIT(data, 6) == 0)
		m_mcu_to_audiocpu = m_mcu_p1;

	m_mcu_p3 = data;
}

void bionicc_state::dmaon_w(u16 data)
{
	m_mcu->set_input_line(MCS51_INT0_LINE, ASSERT_LINE);
	m_maincpu->suspend(SUSPEND_REASON_HALT, true);
}


//**************************************************************************
//  AUDIO
//**************************************************************************

void bionicc_state::audiocpu_nmi_w(u8 data)
{
	m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void bionicc_state::output_w(u8 data)
{
	// 7-------  coin counter 1
	// -6------  coin counter 2
	// --5-----  coin lockout 1
	// ---4----  coin lockout 2
	// ----321-  unused
	// -------0  flip screen

	flip_screen_set(BIT(data, 0));

	// commented out, else you can't test the coin inputs in service mode
//  machine().bookkeeping().coin_lockout_w(1, BIT(~data, 4));
//  machine().bookkeeping().coin_lockout_w(0, BIT(~data, 5));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 6));
	machine().bookkeeping().coin_counter_w(0, BIT(data, 7));
}

TIMER_DEVICE_CALLBACK_MEMBER(bionicc_state::scanline)
{
	// vblank-out irq - drives the game (V256)
	if (param == 256)
		m_maincpu->set_input_line(2, HOLD_LINE);

	// vblank-in irq - processes inputs (!LVBL)
	// should be 16? but then often loses coin inserts
	if (param == 128)
		m_maincpu->set_input_line(4, HOLD_LINE);
}

void bionicc_state::machine_start()
{
	save_item(NAME(m_scroll));
	save_item(NAME(m_audiocpu_to_mcu));
	save_item(NAME(m_mcu_to_audiocpu));
	save_item(NAME(m_mcu_p1));
	save_item(NAME(m_mcu_p3));
}

void bionicc_state::machine_reset()
{
	m_scroll[0] = 0;
	m_scroll[1] = 0;
	m_scroll[2] = 0;
	m_scroll[3] = 0;
}


//**************************************************************************
//  MACHINE DEFINITIONS
//**************************************************************************

void bionicc_state::bionicc(machine_config &config)
{
	// Main CPU 68000CP10 @ 12 MHz
	M68000(config, m_maincpu, 24_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &bionicc_state::main_map);

	TIMER(config, "scantimer").configure_scanline(FUNC(bionicc_state::scanline), "screen", 0, 1);

	// Audio CPU Z80 @ 3.579545 MHz (EXO3 C,B=GND, A=5V ==> Divisor 2^2)
	Z80(config, m_audiocpu, 14.318181_MHz_XTAL / 4);
	m_audiocpu->set_addrmap(AS_PROGRAM, &bionicc_state::sound_map);

	// Protection MCU Intel C8751H-88 @ 6 MHz
	I8751(config, m_mcu, 24_MHz_XTAL / 4);
	m_mcu->set_addrmap(AS_IO, &bionicc_state::mcu_io);
	m_mcu->port_in_cb<1>().set([this](){ return m_audiocpu_to_mcu; });
	m_mcu->port_out_cb<1>().set([this](u8 data){ m_mcu_p1 = data; });
	m_mcu->port_out_cb<3>().set(FUNC(bionicc_state::mcu_p3_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(24_MHz_XTAL / 4, 384, 128, 0, 262, 22, 246); // hsync is 50..77, vsync is 257..259
	screen.set_screen_update(FUNC(bionicc_state::screen_update));
	screen.screen_vblank().set(m_spriteram, FUNC(buffered_spriteram16_device::vblank_copy_rising));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_bionicc);

	TIGEROAD_SPRITE(config, m_spritegen, 0);
	m_spritegen->set_palette(m_palette);
	m_spritegen->set_color_base(512);    /* colors 512- 767 */

	PALETTE(config, m_palette).set_format(2, &bionicc_state::RRRRGGGGBBBBIIII, 1024);

	BUFFERED_SPRITERAM16(config, m_spriteram);

	SPEAKER(config, "mono").front_center();

	YM2151(config, "ymsnd", 14.318181_MHz_XTAL / 4).add_route(0, "mono", 0.60).add_route(1, "mono", 0.60);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( bionicc ) /* "Not for use in Japan" */
	ROM_REGION( 0x40000, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "tse_02.1a",  0x00000, 0x10000, CRC(e4aeefaa) SHA1(77b6a2d4337bf350239abb50013d030d7c5c8640) ) /* 68000 code */
	ROM_LOAD16_BYTE( "tse_04.1b",  0x00001, 0x10000, CRC(d0c8ec75) SHA1(04138c75ca3939604100b7e9fb451f7fceee67ca) ) /* 68000 code */
	ROM_LOAD16_BYTE( "tse_03.2a",  0x20000, 0x10000, CRC(b2ac0a45) SHA1(d0933e74870efa9ea703251b30a56ef706ac24fe) ) /* 68000 code */
	ROM_LOAD16_BYTE( "tse_05.2b",  0x20001, 0x10000, CRC(a79cb406) SHA1(50eada2f3e80c28dcb5529890d9b279c73f0115a) ) /* 68000 code */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ts_01b.4e",  0x00000, 0x8000, CRC(a9a6cafa) SHA1(55e0a0e6ca11e8e73339d5b4604e130031211291) )

	ROM_REGION( 0x1000, "mcu", 0 )  /* i8751 microcontroller */
	ROM_LOAD( "ts.2f",     0x0000, 0x1000, CRC(3ed7f0be) SHA1(db9e972065c8e60b5d74762dc3424271ea9524cb) )  /* from 'topsecrt' bootleg, but appears to be original */

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "tsu_08.8l",   0x00000, 0x8000, CRC(9bf0b7a2) SHA1(1361335c3c2c8a9c6a7d99566048d8aac99e7c8f) )    /* VIDEORAM (text layer) tiles */

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "tsu_07.5l",   0x00000, 0x8000, CRC(9469efa4) SHA1(53c70361e8d9e54825f61b87a10df42438aaf5b0) )    /* SCROLL2 Layer Tiles */
	ROM_LOAD( "tsu_06.4l",   0x08000, 0x8000, CRC(40bf0eb4) SHA1(fcb186c31747e2c9872de01e34b3e713dc74df82) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "ts_12.17f",    0x00000, 0x8000, CRC(e4b4619e) SHA1(3bec8399ffb28fd50ce6ae88d90b091eadf8bda1) )   /* SCROLL1 Layer Tiles */
	ROM_LOAD( "ts_11.15f",    0x08000, 0x8000, CRC(ab30237a) SHA1(ea6c07df992ba48f9eca7daa4ea775faa94358d2) )
	ROM_LOAD( "ts_17.17g",    0x10000, 0x8000, CRC(deb657e4) SHA1(b36b468f9bbb7a4937286230d3f6caa14c61d4dd) )
	ROM_LOAD( "ts_16.15g",    0x18000, 0x8000, CRC(d363b5f9) SHA1(1dd3991d99db2d6bcbdb12879ba50a01fef95004) )
	ROM_LOAD( "ts_13.18f",    0x20000, 0x8000, CRC(a8f5a004) SHA1(36ab0cb8ec9ce0519876f7461ccc5020c9c5b597) )
	ROM_LOAD( "ts_18.18g",    0x28000, 0x8000, CRC(3b36948c) SHA1(d85fcc0265ba1729c587b046cc5a7ba6f25363dd) )
	ROM_LOAD( "ts_23.18j",    0x30000, 0x8000, CRC(bbfbe58a) SHA1(9b1d5672b6f3c5c0952f8dcd0da71acc68a97a5e) )
	ROM_LOAD( "ts_24.18k",    0x38000, 0x8000, CRC(f156e564) SHA1(a6cad05bcc6d9ded6294f9b5aa856d05641aed02) )

	ROM_REGION( 0x40000, "spritegen", 0 )
	ROM_LOAD32_BYTE( "tse_10.13f",   0x00003, 0x8000, CRC(d28eeacc) SHA1(8b4a655a48da276b07f3464c65743b13cec52bcb) )   /* Sprites */
	ROM_LOAD32_BYTE( "tsu_09.11f",   0x20003, 0x8000, CRC(6a049292) SHA1(525c862061f426d679b539b6926af4c9f14b47b5) )
	ROM_LOAD32_BYTE( "tse_15.13g",   0x00002, 0x8000, CRC(9b5593c0) SHA1(73c0acbb01fe69c2bd29dea11b6a223c8efb54a0) )
	ROM_LOAD32_BYTE( "tsu_14.11g",   0x20002, 0x8000, CRC(46b2ad83) SHA1(21ebd5691a544323fdfcf330b9a37bbe0428e3e3) )
	ROM_LOAD32_BYTE( "tse_20.13j",   0x00001, 0x8000, CRC(b03db778) SHA1(f72a93e73196c800c1893fd3b523394d702547dd) )
	ROM_LOAD32_BYTE( "tsu_19.11j",   0x20001, 0x8000, CRC(b5c82722) SHA1(969f9159f7d59e4e4c9ef9ddbdc27cbfa531eabf) )
	ROM_LOAD32_BYTE( "tse_22.17j",   0x00000, 0x8000, CRC(d4dedeb3) SHA1(e121057bb541f3f5c755963ca22832c3fe2637c0) )
	ROM_LOAD32_BYTE( "tsu_21.15j",   0x20000, 0x8000, CRC(98777006) SHA1(bcc2058b639e9b71d16af05f63df298bcce91fdc) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "63s141.18f",   0x0000, 0x0100, CRC(b58d0023) SHA1(e8a4a2e2951bf73b3d9eed6957e9ee1e61c9c58a) )    /* priority (not used), Labeled "TSB" */
ROM_END

ROM_START( bionicc1 ) /* "Not for use outside of USA or Canada" revision B */
	ROM_REGION( 0x40000, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "tsu_02b.1a",  0x00000, 0x10000, CRC(cf965a0a) SHA1(ab88742a3225a0b82ee2dfef6ed0058d3e11c38c) ) /* 68000 code */
	ROM_LOAD16_BYTE( "tsu_04b.1b",  0x00001, 0x10000, CRC(c9884bfb) SHA1(7d10cedff0a62847f8deb61a9611cc6661efb037) ) /* 68000 code */
	ROM_LOAD16_BYTE( "tsu_03b.2a",  0x20000, 0x10000, CRC(4e157ae2) SHA1(cc02931376d22a7fcfc320e6fd4129e03a461a49) ) /* 68000 code */
	ROM_LOAD16_BYTE( "tsu_05b.2b",  0x20001, 0x10000, CRC(e66ca0f9) SHA1(a503badf2fed38786d38c313d1dc315f3175d6de) ) /* 68000 code */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ts_01b.4e",  0x00000, 0x8000, CRC(a9a6cafa) SHA1(55e0a0e6ca11e8e73339d5b4604e130031211291) )

	ROM_REGION( 0x1000, "mcu", 0 )  /* i8751 microcontroller */
	ROM_LOAD( "ts.2f",     0x0000, 0x1000, CRC(3ed7f0be) SHA1(db9e972065c8e60b5d74762dc3424271ea9524cb) )  /* from 'topsecrt' bootleg, but appears to be original */

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "tsu_08.8l",   0x00000, 0x8000, CRC(9bf0b7a2) SHA1(1361335c3c2c8a9c6a7d99566048d8aac99e7c8f) )    /* VIDEORAM (text layer) tiles */

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "tsu_07.5l",   0x00000, 0x8000, CRC(9469efa4) SHA1(53c70361e8d9e54825f61b87a10df42438aaf5b0) )    /* SCROLL2 Layer Tiles */
	ROM_LOAD( "tsu_06.4l",   0x08000, 0x8000, CRC(40bf0eb4) SHA1(fcb186c31747e2c9872de01e34b3e713dc74df82) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "ts_12.17f",    0x00000, 0x8000, CRC(e4b4619e) SHA1(3bec8399ffb28fd50ce6ae88d90b091eadf8bda1) )   /* SCROLL1 Layer Tiles */
	ROM_LOAD( "ts_11.15f",    0x08000, 0x8000, CRC(ab30237a) SHA1(ea6c07df992ba48f9eca7daa4ea775faa94358d2) )
	ROM_LOAD( "ts_17.17g",    0x10000, 0x8000, CRC(deb657e4) SHA1(b36b468f9bbb7a4937286230d3f6caa14c61d4dd) )
	ROM_LOAD( "ts_16.15g",    0x18000, 0x8000, CRC(d363b5f9) SHA1(1dd3991d99db2d6bcbdb12879ba50a01fef95004) )
	ROM_LOAD( "ts_13.18f",    0x20000, 0x8000, CRC(a8f5a004) SHA1(36ab0cb8ec9ce0519876f7461ccc5020c9c5b597) )
	ROM_LOAD( "ts_18.18g",    0x28000, 0x8000, CRC(3b36948c) SHA1(d85fcc0265ba1729c587b046cc5a7ba6f25363dd) )
	ROM_LOAD( "ts_23.18j",    0x30000, 0x8000, CRC(bbfbe58a) SHA1(9b1d5672b6f3c5c0952f8dcd0da71acc68a97a5e) )
	ROM_LOAD( "ts_24.18k",    0x38000, 0x8000, CRC(f156e564) SHA1(a6cad05bcc6d9ded6294f9b5aa856d05641aed02) )

	ROM_REGION( 0x40000, "spritegen", 0 )
	ROM_LOAD32_BYTE( "tsu_10.13f",   0x00003, 0x8000, CRC(f1180d02) SHA1(312626af48235a1f726ab596f296ef4739785ca0) )   /* Sprites */
	ROM_LOAD32_BYTE( "tsu_09.11f",   0x20003, 0x8000, CRC(6a049292) SHA1(525c862061f426d679b539b6926af4c9f14b47b5) )
	ROM_LOAD32_BYTE( "tsu_15.13g",   0x00002, 0x8000, CRC(ea912701) SHA1(106336c63a1c8a0b13236268bc533a8263285cad) )
	ROM_LOAD32_BYTE( "tsu_14.11g",   0x20002, 0x8000, CRC(46b2ad83) SHA1(21ebd5691a544323fdfcf330b9a37bbe0428e3e3) )
	ROM_LOAD32_BYTE( "tsu_20.13j",   0x00001, 0x8000, CRC(17857ad2) SHA1(9f45cea6e9ce82bfc9ee6896a30257d20fb38bca) )
	ROM_LOAD32_BYTE( "tsu_19.11j",   0x20001, 0x8000, CRC(b5c82722) SHA1(969f9159f7d59e4e4c9ef9ddbdc27cbfa531eabf) )
	ROM_LOAD32_BYTE( "tsu_22.17j",   0x00000, 0x8000, CRC(5ee1ae6a) SHA1(76ca53d847c940c4176d79ba49b0c10efd6342e8) )
	ROM_LOAD32_BYTE( "tsu_21.15j",   0x20000, 0x8000, CRC(98777006) SHA1(bcc2058b639e9b71d16af05f63df298bcce91fdc) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "63s141.18f",   0x0000, 0x0100, CRC(b58d0023) SHA1(e8a4a2e2951bf73b3d9eed6957e9ee1e61c9c58a) )    /* priority (not used), Labeled "TSB" */
ROM_END

ROM_START( bionicc2 ) /* "Not for use outside of USA or Canada" 1st release */
	ROM_REGION( 0x40000, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "tsu_02.1a",  0x00000, 0x10000, CRC(f2528f08) SHA1(04c793837c86d83312fd44b46a6a94378c90113b) ) /* 68000 code */
	ROM_LOAD16_BYTE( "tsu_04.1b",  0x00001, 0x10000, CRC(38b1c7e4) SHA1(14bf743726c214bd00177e7b410c272dd7ab3d3f) ) /* 68000 code */
	ROM_LOAD16_BYTE( "tsu_03.2a",  0x20000, 0x10000, CRC(72c3b76f) SHA1(f7f71eae7617e3348b727775088b496e86d51e38) ) /* 68000 code */
	ROM_LOAD16_BYTE( "tsu_05.2b",  0x20001, 0x10000, CRC(70621f83) SHA1(0a77c2827a5c50457d90ccc62e463508d83d2f20) ) /* 68000 code */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ts_01b.4e",  0x00000, 0x8000, CRC(a9a6cafa) SHA1(55e0a0e6ca11e8e73339d5b4604e130031211291) )

	ROM_REGION( 0x1000, "mcu", 0 )  /* i8751 microcontroller */
	ROM_LOAD( "ts.2f",     0x0000, 0x1000, CRC(3ed7f0be) SHA1(db9e972065c8e60b5d74762dc3424271ea9524cb) )  /* from 'topsecrt' bootleg, but appears to be original */

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "tsu_08.8l",   0x00000, 0x8000, CRC(9bf0b7a2) SHA1(1361335c3c2c8a9c6a7d99566048d8aac99e7c8f) )    /* VIDEORAM (text layer) tiles */

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "tsu_07.5l",   0x00000, 0x8000, CRC(9469efa4) SHA1(53c70361e8d9e54825f61b87a10df42438aaf5b0) )    /* SCROLL2 Layer Tiles */
	ROM_LOAD( "tsu_06.4l",   0x08000, 0x8000, CRC(40bf0eb4) SHA1(fcb186c31747e2c9872de01e34b3e713dc74df82) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "ts_12.17f",    0x00000, 0x8000, CRC(e4b4619e) SHA1(3bec8399ffb28fd50ce6ae88d90b091eadf8bda1) )   /* SCROLL1 Layer Tiles */
	ROM_LOAD( "ts_11.15f",    0x08000, 0x8000, CRC(ab30237a) SHA1(ea6c07df992ba48f9eca7daa4ea775faa94358d2) )
	ROM_LOAD( "ts_17.17g",    0x10000, 0x8000, CRC(deb657e4) SHA1(b36b468f9bbb7a4937286230d3f6caa14c61d4dd) )
	ROM_LOAD( "ts_16.15g",    0x18000, 0x8000, CRC(d363b5f9) SHA1(1dd3991d99db2d6bcbdb12879ba50a01fef95004) )
	ROM_LOAD( "ts_13.18f",    0x20000, 0x8000, CRC(a8f5a004) SHA1(36ab0cb8ec9ce0519876f7461ccc5020c9c5b597) )
	ROM_LOAD( "ts_18.18g",    0x28000, 0x8000, CRC(3b36948c) SHA1(d85fcc0265ba1729c587b046cc5a7ba6f25363dd) )
	ROM_LOAD( "ts_23.18j",    0x30000, 0x8000, CRC(bbfbe58a) SHA1(9b1d5672b6f3c5c0952f8dcd0da71acc68a97a5e) )
	ROM_LOAD( "ts_24.18k",    0x38000, 0x8000, CRC(f156e564) SHA1(a6cad05bcc6d9ded6294f9b5aa856d05641aed02) )

	ROM_REGION( 0x40000, "spritegen", 0 )
	ROM_LOAD32_BYTE( "tsu_10.13f",   0x00003, 0x8000, CRC(f1180d02) SHA1(312626af48235a1f726ab596f296ef4739785ca0) )   /* Sprites */
	ROM_LOAD32_BYTE( "tsu_09.11f",   0x20003, 0x8000, CRC(6a049292) SHA1(525c862061f426d679b539b6926af4c9f14b47b5) )
	ROM_LOAD32_BYTE( "tsu_15.13g",   0x00002, 0x8000, CRC(ea912701) SHA1(106336c63a1c8a0b13236268bc533a8263285cad) )
	ROM_LOAD32_BYTE( "tsu_14.11g",   0x20002, 0x8000, CRC(46b2ad83) SHA1(21ebd5691a544323fdfcf330b9a37bbe0428e3e3) )
	ROM_LOAD32_BYTE( "tsu_20.13j",   0x00001, 0x8000, CRC(17857ad2) SHA1(9f45cea6e9ce82bfc9ee6896a30257d20fb38bca) )
	ROM_LOAD32_BYTE( "tsu_19.11j",   0x20001, 0x8000, CRC(b5c82722) SHA1(969f9159f7d59e4e4c9ef9ddbdc27cbfa531eabf) )
	ROM_LOAD32_BYTE( "tsu_22.17j",   0x00000, 0x8000, CRC(5ee1ae6a) SHA1(76ca53d847c940c4176d79ba49b0c10efd6342e8) )
	ROM_LOAD32_BYTE( "tsu_21.15j",   0x20000, 0x8000, CRC(98777006) SHA1(bcc2058b639e9b71d16af05f63df298bcce91fdc) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "63s141.18f",   0x0000, 0x0100, CRC(b58d0023) SHA1(e8a4a2e2951bf73b3d9eed6957e9ee1e61c9c58a) )    /* priority (not used), Labeled "TSB" */
ROM_END

ROM_START( topsecrt2 ) /* "Not for use in any other country but Japan" */
	ROM_REGION( 0x40000, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "ts_02b.1a",  0x00000, 0x10000, CRC(0b84497f) SHA1(f7e9412d37a1e4b7a437d3f7bc5dc448c8a22079) ) /* 68000 code */
	ROM_LOAD16_BYTE( "ts_04b.1b",  0x00001, 0x10000, CRC(9ab6de8d) SHA1(04445bc183364ebb6a8833a147b694234b509634) ) /* 68000 code */
	ROM_LOAD16_BYTE( "ts_03b.2a",  0x20000, 0x10000, CRC(1b3f8a82) SHA1(2f9cad83b7bd10617bbd5172a1d31f41b194d3ac) ) /* 68000 code */
	ROM_LOAD16_BYTE( "ts_05b.2b",  0x20001, 0x10000, CRC(962a89d8) SHA1(4aa5b0fd68a59ff74716253b0ba4c1933e92855b) ) /* 68000 code */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ts_01b.4e",  0x00000, 0x8000, CRC(a9a6cafa) SHA1(55e0a0e6ca11e8e73339d5b4604e130031211291) )

	ROM_REGION( 0x1000, "mcu", 0 )  /* i8751 microcontroller */
	ROM_LOAD( "ts.2f",     0x0000, 0x1000, CRC(3ed7f0be) SHA1(db9e972065c8e60b5d74762dc3424271ea9524cb) )  /* from 'topsecrt' bootleg, but appears to be original */

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "ts_08.8l",    0x00000, 0x8000, CRC(96ad379e) SHA1(accd3a560b259c186bc28cdc004ed8de0b12f9d5) )    /* VIDEORAM (text layer) tiles */

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "ts_07.5l",    0x00000, 0x8000, CRC(25cdf8b2) SHA1(316f6acc46878682dabeab12722e6a64504d23bd) )    /* SCROLL2 Layer Tiles */
	ROM_LOAD( "ts_06.4l",    0x08000, 0x8000, CRC(314fb12d) SHA1(dab0519a49b64fe7a837b3c6383f6147e1ab6ffd) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "ts_12.17f",    0x00000, 0x8000, CRC(e4b4619e) SHA1(3bec8399ffb28fd50ce6ae88d90b091eadf8bda1) )   /* SCROLL1 Layer Tiles */
	ROM_LOAD( "ts_11.15f",    0x08000, 0x8000, CRC(ab30237a) SHA1(ea6c07df992ba48f9eca7daa4ea775faa94358d2) )
	ROM_LOAD( "ts_17.17g",    0x10000, 0x8000, CRC(deb657e4) SHA1(b36b468f9bbb7a4937286230d3f6caa14c61d4dd) )
	ROM_LOAD( "ts_16.15g",    0x18000, 0x8000, CRC(d363b5f9) SHA1(1dd3991d99db2d6bcbdb12879ba50a01fef95004) )
	ROM_LOAD( "ts_13.18f",    0x20000, 0x8000, CRC(a8f5a004) SHA1(36ab0cb8ec9ce0519876f7461ccc5020c9c5b597) )
	ROM_LOAD( "ts_18.18g",    0x28000, 0x8000, CRC(3b36948c) SHA1(d85fcc0265ba1729c587b046cc5a7ba6f25363dd) )
	ROM_LOAD( "ts_23.18j",    0x30000, 0x8000, CRC(bbfbe58a) SHA1(9b1d5672b6f3c5c0952f8dcd0da71acc68a97a5e) )
	ROM_LOAD( "ts_24.18k",    0x38000, 0x8000, CRC(f156e564) SHA1(a6cad05bcc6d9ded6294f9b5aa856d05641aed02) )

	ROM_REGION( 0x40000, "spritegen", 0 )
	ROM_LOAD32_BYTE( "ts_10.13f",    0x00003, 0x8000, CRC(c3587d05) SHA1(ad0898a5d4cf110783ef092bf8e65b6ef31a8ae0) )   /* Sprites */
	ROM_LOAD32_BYTE( "ts_09.11f",    0x20003, 0x8000, CRC(6b63eef2) SHA1(5d1580db7f49c5994c2a08a36c2d05f3e246930d) )
	ROM_LOAD32_BYTE( "ts_15.13g",    0x00002, 0x8000, CRC(db8cebb0) SHA1(1cc9eac14851cde95fb2d69d6f5ffb08bc9c0d93) )
	ROM_LOAD32_BYTE( "ts_14.11g",    0x20002, 0x8000, CRC(e2e41abf) SHA1(d002d0d8fdbb9ec3e2eac218f6338f733953ca82) )
	ROM_LOAD32_BYTE( "ts_20.13j",    0x00001, 0x8000, CRC(bfd1a695) SHA1(bf93486b96bfa1a1d5015189043b07e6130e6df1) )
	ROM_LOAD32_BYTE( "ts_19.11j",    0x20001, 0x8000, CRC(928b669e) SHA1(98ea9d23a46b0700490fd2fa7ab4fb0988dd5ca6) )
	ROM_LOAD32_BYTE( "ts_22.17j",    0x00000, 0x8000, CRC(3fe05d9a) SHA1(32e28ef03fb82785019d1ae8b3859215b5368c2b) )
	ROM_LOAD32_BYTE( "ts_21.15j",    0x20000, 0x8000, CRC(27a9bb7c) SHA1(bb60332c0ecde4d7797960dec39c1079498175c3) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "63s141.18f",   0x0000, 0x0100, CRC(b58d0023) SHA1(e8a4a2e2951bf73b3d9eed6957e9ee1e61c9c58a) )    /* priority (not used), Labeled "TSB" */
ROM_END

ROM_START( topsecrt ) /* "Not for use in any other country but Japan" */
	ROM_REGION( 0x40000, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "ts_02.1a",  0x00000, 0x10000, CRC(b2fe1ddb) SHA1(892f19124993add96edabdba3aafeecc6668c5d9) ) /* 68000 code */
	ROM_LOAD16_BYTE( "ts_04.1b",  0x00001, 0x10000, CRC(427a003d) SHA1(5a379fe2942e5565810939d5eb843003226222cc) ) /* 68000 code */
	ROM_LOAD16_BYTE( "ts_03.2a",  0x20000, 0x10000, CRC(27f04bb6) SHA1(41d17b84b34dc8b2e5dfa67794a8df3e898b740b) ) /* 68000 code */
	ROM_LOAD16_BYTE( "ts_05.2b",  0x20001, 0x10000, CRC(c01547b1) SHA1(563bf6be4f10f5e6eb5b562266accf168f62bf30) ) /* 68000 code */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ts_01.4e",    0x00000, 0x8000, CRC(8ea07917) SHA1(e9ace70d89482fc3669860450a41aacacbee9083) )

	ROM_REGION( 0x1000, "mcu", 0 )  /* i8751 microcontroller */
	ROM_LOAD( "ts.2f",     0x0000, 0x1000, CRC(3ed7f0be) SHA1(db9e972065c8e60b5d74762dc3424271ea9524cb) )  /* from 'topsecrt' bootleg, but appears to be original */

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "ts_08.8l",    0x00000, 0x8000, CRC(96ad379e) SHA1(accd3a560b259c186bc28cdc004ed8de0b12f9d5) )    /* VIDEORAM (text layer) tiles */

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "ts_07.5l",    0x00000, 0x8000, CRC(25cdf8b2) SHA1(316f6acc46878682dabeab12722e6a64504d23bd) )    /* SCROLL2 Layer Tiles */
	ROM_LOAD( "ts_06.4l",    0x08000, 0x8000, CRC(314fb12d) SHA1(dab0519a49b64fe7a837b3c6383f6147e1ab6ffd) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "ts_12.17f",    0x00000, 0x8000, CRC(e4b4619e) SHA1(3bec8399ffb28fd50ce6ae88d90b091eadf8bda1) )   /* SCROLL1 Layer Tiles */
	ROM_LOAD( "ts_11.15f",    0x08000, 0x8000, CRC(ab30237a) SHA1(ea6c07df992ba48f9eca7daa4ea775faa94358d2) )
	ROM_LOAD( "ts_17.17g",    0x10000, 0x8000, CRC(deb657e4) SHA1(b36b468f9bbb7a4937286230d3f6caa14c61d4dd) )
	ROM_LOAD( "ts_16.15g",    0x18000, 0x8000, CRC(d363b5f9) SHA1(1dd3991d99db2d6bcbdb12879ba50a01fef95004) )
	ROM_LOAD( "ts_13.18f",    0x20000, 0x8000, CRC(a8f5a004) SHA1(36ab0cb8ec9ce0519876f7461ccc5020c9c5b597) )
	ROM_LOAD( "ts_18.18g",    0x28000, 0x8000, CRC(3b36948c) SHA1(d85fcc0265ba1729c587b046cc5a7ba6f25363dd) )
	ROM_LOAD( "ts_23.18j",    0x30000, 0x8000, CRC(bbfbe58a) SHA1(9b1d5672b6f3c5c0952f8dcd0da71acc68a97a5e) )
	ROM_LOAD( "ts_24.18k",    0x38000, 0x8000, CRC(f156e564) SHA1(a6cad05bcc6d9ded6294f9b5aa856d05641aed02) )

	ROM_REGION( 0x40000, "spritegen", 0 )
	ROM_LOAD32_BYTE( "ts_10.13f",    0x00003, 0x8000, CRC(c3587d05) SHA1(ad0898a5d4cf110783ef092bf8e65b6ef31a8ae0) )   /* Sprites */
	ROM_LOAD32_BYTE( "ts_09.11f",    0x20003, 0x8000, CRC(6b63eef2) SHA1(5d1580db7f49c5994c2a08a36c2d05f3e246930d) )
	ROM_LOAD32_BYTE( "ts_15.13g",    0x00002, 0x8000, CRC(db8cebb0) SHA1(1cc9eac14851cde95fb2d69d6f5ffb08bc9c0d93) )
	ROM_LOAD32_BYTE( "ts_14.11g",    0x20002, 0x8000, CRC(e2e41abf) SHA1(d002d0d8fdbb9ec3e2eac218f6338f733953ca82) )
	ROM_LOAD32_BYTE( "ts_20.13j",    0x00001, 0x8000, CRC(bfd1a695) SHA1(bf93486b96bfa1a1d5015189043b07e6130e6df1) )
	ROM_LOAD32_BYTE( "ts_19.11j",    0x20001, 0x8000, CRC(928b669e) SHA1(98ea9d23a46b0700490fd2fa7ab4fb0988dd5ca6) )
	ROM_LOAD32_BYTE( "ts_22.17j",    0x00000, 0x8000, CRC(3fe05d9a) SHA1(32e28ef03fb82785019d1ae8b3859215b5368c2b) )
	ROM_LOAD32_BYTE( "ts_21.15j",    0x20000, 0x8000, CRC(27a9bb7c) SHA1(bb60332c0ecde4d7797960dec39c1079498175c3) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "63s141.18f",   0x0000, 0x0100, CRC(b58d0023) SHA1(e8a4a2e2951bf73b3d9eed6957e9ee1e61c9c58a) )    /* priority (not used), Labeled "TSB" */
ROM_END

ROM_START( bioniccbl )
	ROM_REGION( 0x40000, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "02l.bin",   0x00000, 0x10000, CRC(b2fe1ddb) SHA1(892f19124993add96edabdba3aafeecc6668c5d9) )
	ROM_LOAD16_BYTE( "03l.bin",   0x00001, 0x10000, CRC(427a003d) SHA1(5a379fe2942e5565810939d5eb843003226222cc) )
	ROM_LOAD16_BYTE( "02u.bin",   0x20000, 0x10000, CRC(27f04bb6) SHA1(41d17b84b34dc8b2e5dfa67794a8df3e898b740b) )
	ROM_LOAD16_BYTE( "03u.bin",   0x20001, 0x10000, CRC(c01547b1) SHA1(563bf6be4f10f5e6eb5b562266accf168f62bf30) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "01.bin",       0x00000, 0x8000, CRC(8ea07917) SHA1(e9ace70d89482fc3669860450a41aacacbee9083) )

	ROM_REGION( 0x1000, "mcu", 0 )  /* i8751 microcontroller */
	ROM_LOAD( "d8751h.bin",     0x0000, 0x1000, CRC(3ed7f0be) SHA1(db9e972065c8e60b5d74762dc3424271ea9524cb) )  /* from 'topsecrt' bootleg, but appears to be original */

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "06.bin",       0x00000, 0x4000, CRC(4e6b81d9) SHA1(052784d789b0c9193edf218fa1883b6e3b7df988) )

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "05.bin",       0x00000, 0x8000, CRC(9bf8dc7f) SHA1(539a52087ed1f58839b6aa8c0b5ed249f4f4041e) )
	ROM_LOAD( "04.bin",       0x08000, 0x8000, CRC(1b43bf63) SHA1(b80d675a07cebd83daf202d1c3d3f7c2dedf5c30) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "ts_12.17f",    0x00000, 0x8000, CRC(e4b4619e) SHA1(3bec8399ffb28fd50ce6ae88d90b091eadf8bda1) ) // 09.bin
	ROM_LOAD( "ts_11.15f",    0x08000, 0x8000, CRC(ab30237a) SHA1(ea6c07df992ba48f9eca7daa4ea775faa94358d2) ) // 08.bin
	ROM_LOAD( "ts_17.17g",    0x10000, 0x8000, CRC(deb657e4) SHA1(b36b468f9bbb7a4937286230d3f6caa14c61d4dd) ) // 13.bin
	ROM_LOAD( "ts_16.15g",    0x18000, 0x8000, CRC(d363b5f9) SHA1(1dd3991d99db2d6bcbdb12879ba50a01fef95004) ) // 12.bin
	ROM_LOAD( "ts_13.18f",    0x20000, 0x8000, CRC(a8f5a004) SHA1(36ab0cb8ec9ce0519876f7461ccc5020c9c5b597) ) // 10.bin
	ROM_LOAD( "ts_18.18g",    0x28000, 0x8000, CRC(3b36948c) SHA1(d85fcc0265ba1729c587b046cc5a7ba6f25363dd) ) // 14.bin
	ROM_LOAD( "ts_23.18j",    0x30000, 0x8000, CRC(bbfbe58a) SHA1(9b1d5672b6f3c5c0952f8dcd0da71acc68a97a5e) ) // 17.bin
	ROM_LOAD( "ts_24.18k",    0x38000, 0x8000, CRC(f156e564) SHA1(a6cad05bcc6d9ded6294f9b5aa856d05641aed02) ) // 18.bin

	ROM_REGION( 0x40000, "spritegen", 0 )
	ROM_LOAD32_BYTE( "07.bin",       0x00003, 0x10000, CRC(a0e78996) SHA1(810a54e657c0faaff6a8494acaf803d1d2151893) )
	ROM_LOAD32_BYTE( "11.bin",       0x00002, 0x10000, CRC(37cb11c2) SHA1(af8c2ae4bb6e6c13ea3e8b7c96e5b18f1eb1d5a5) )
	ROM_LOAD32_BYTE( "15.bin",       0x00001, 0x10000, CRC(4e0354ce) SHA1(d3256c891b44c6593b0b44c0d0a3e754ce78c1cb) )
	ROM_LOAD32_BYTE( "16.bin",       0x00000, 0x10000, CRC(ac89e5cc) SHA1(aa7e065ece6d25b7e83fadcd22c09e1f7dc0b86f) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "63s141.18f",   0x0000, 0x0100, CRC(b58d0023) SHA1(e8a4a2e2951bf73b3d9eed6957e9ee1e61c9c58a) )    /* priority (not used), Labeled "TSB" */
ROM_END

ROM_START( bioniccbl2 ) // only the 4 maincpu ROMs differ, they came from an original, not working, Capcom board, but the title screen is Bionic Commandos like a bootleg? The other ROMs match topsecrt
	ROM_REGION( 0x40000, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "tsu_02.1a",   0x00000, 0x10000, CRC(c03d3424) SHA1(1d47185e10813c2792bda31c7dfbb40e88fd46ee) ) //sldc
	ROM_LOAD16_BYTE( "tsu_04.1b",   0x00001, 0x10000, CRC(9f13eb9d) SHA1(b9c8cfc22a1d2adcc6a93e3e382b2126446c05aa) ) //sldc
	ROM_LOAD16_BYTE( "3",   0x20000, 0x10000, CRC(a909ec2c) SHA1(7979480b5d82704f7d15546bda6af78316505f85) ) // the only ROM without an original sticker
	ROM_LOAD16_BYTE( "tsu_05.1d",   0x20001, 0x10000, CRC(4e6b75ce) SHA1(44c3900b8dc9f375a7d6ed57c4025bc35771e29c) ) //sldc

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ts_01.4e",    0x00000, 0x8000, CRC(8ea07917) SHA1(e9ace70d89482fc3669860450a41aacacbee9083) )

	ROM_REGION( 0x1000, "mcu", 0 )  /* i8751 microcontroller */
	ROM_LOAD( "d8751h.bin",     0x0000, 0x1000, CRC(3ed7f0be) SHA1(db9e972065c8e60b5d74762dc3424271ea9524cb) )  /* from 'topsecrt' bootleg, but appears to be original */

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "ts_08.8l",    0x00000, 0x8000, CRC(96ad379e) SHA1(accd3a560b259c186bc28cdc004ed8de0b12f9d5) )    /* VIDEORAM (text layer) tiles */

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "ts_07.5l",    0x00000, 0x8000, CRC(25cdf8b2) SHA1(316f6acc46878682dabeab12722e6a64504d23bd) )    /* SCROLL2 Layer Tiles */
	ROM_LOAD( "ts_06.4l",    0x08000, 0x8000, CRC(314fb12d) SHA1(dab0519a49b64fe7a837b3c6383f6147e1ab6ffd) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "ts_12.17f",    0x00000, 0x8000, CRC(e4b4619e) SHA1(3bec8399ffb28fd50ce6ae88d90b091eadf8bda1) )   /* SCROLL1 Layer Tiles */
	ROM_LOAD( "ts_11.15f",    0x08000, 0x8000, CRC(ab30237a) SHA1(ea6c07df992ba48f9eca7daa4ea775faa94358d2) )
	ROM_LOAD( "ts_17.17g",    0x10000, 0x8000, CRC(deb657e4) SHA1(b36b468f9bbb7a4937286230d3f6caa14c61d4dd) )
	ROM_LOAD( "ts_16.15g",    0x18000, 0x8000, CRC(d363b5f9) SHA1(1dd3991d99db2d6bcbdb12879ba50a01fef95004) )
	ROM_LOAD( "ts_13.18f",    0x20000, 0x8000, CRC(a8f5a004) SHA1(36ab0cb8ec9ce0519876f7461ccc5020c9c5b597) )
	ROM_LOAD( "ts_18.18g",    0x28000, 0x8000, CRC(3b36948c) SHA1(d85fcc0265ba1729c587b046cc5a7ba6f25363dd) )
	ROM_LOAD( "ts_23.18j",    0x30000, 0x8000, CRC(bbfbe58a) SHA1(9b1d5672b6f3c5c0952f8dcd0da71acc68a97a5e) )
	ROM_LOAD( "ts_24.18k",    0x38000, 0x8000, CRC(f156e564) SHA1(a6cad05bcc6d9ded6294f9b5aa856d05641aed02) )

	ROM_REGION( 0x40000, "spritegen", 0 )
	ROM_LOAD32_BYTE( "tse_10.13f",   0x00003, 0x8000, CRC(d28eeacc) SHA1(8b4a655a48da276b07f3464c65743b13cec52bcb) )   /* Sprites */
	ROM_LOAD32_BYTE( "tsu_09.11f",   0x20003, 0x8000, CRC(6a049292) SHA1(525c862061f426d679b539b6926af4c9f14b47b5) )
	ROM_LOAD32_BYTE( "tse_15.13g",   0x00002, 0x8000, CRC(9b5593c0) SHA1(73c0acbb01fe69c2bd29dea11b6a223c8efb54a0) )
	ROM_LOAD32_BYTE( "tsu_14.11g",   0x20002, 0x8000, CRC(46b2ad83) SHA1(21ebd5691a544323fdfcf330b9a37bbe0428e3e3) )
	ROM_LOAD32_BYTE( "tse_20.13j",   0x00001, 0x8000, CRC(b03db778) SHA1(f72a93e73196c800c1893fd3b523394d702547dd) )
	ROM_LOAD32_BYTE( "tsu_19.11j",   0x20001, 0x8000, CRC(b5c82722) SHA1(969f9159f7d59e4e4c9ef9ddbdc27cbfa531eabf) )
	ROM_LOAD32_BYTE( "tse_22.17j",   0x00000, 0x8000, CRC(d4dedeb3) SHA1(e121057bb541f3f5c755963ca22832c3fe2637c0) )
	ROM_LOAD32_BYTE( "tsu_21.15j",   0x20000, 0x8000, CRC(98777006) SHA1(bcc2058b639e9b71d16af05f63df298bcce91fdc) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "63s141.18f",   0x0000, 0x0100, CRC(b58d0023) SHA1(e8a4a2e2951bf73b3d9eed6957e9ee1e61c9c58a) )    /* priority (not used), Labeled "TSB" */
ROM_END

} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME        PARENT   MACHINE  INPUT    CLASS          INIT        ROT   COMPANY    FULLNAME                             FLAGS
GAME( 1987, bionicc,    0,       bionicc, bionicc, bionicc_state, empty_init, ROT0, "Capcom",  "Bionic Commando (Euro)",            MACHINE_SUPPORTS_SAVE )
GAME( 1987, bionicc1,   bionicc, bionicc, bionicc, bionicc_state, empty_init, ROT0, "Capcom",  "Bionic Commando (US set 1)",        MACHINE_SUPPORTS_SAVE )
GAME( 1987, bionicc2,   bionicc, bionicc, bionicc, bionicc_state, empty_init, ROT0, "Capcom",  "Bionic Commando (US set 2)",        MACHINE_SUPPORTS_SAVE )
GAME( 1987, topsecrt,   bionicc, bionicc, bionicc, bionicc_state, empty_init, ROT0, "Capcom",  "Top Secret (Japan, old revision)",  MACHINE_SUPPORTS_SAVE )
GAME( 1987, topsecrt2,  bionicc, bionicc, bionicc, bionicc_state, empty_init, ROT0, "Capcom",  "Top Secret (Japan, revision B)",    MACHINE_SUPPORTS_SAVE )
GAME( 1987, bioniccbl,  bionicc, bionicc, bionicc, bionicc_state, empty_init, ROT0, "bootleg", "Bionic Commandos (bootleg, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, bioniccbl2, bionicc, bionicc, bionicc, bionicc_state, empty_init, ROT0, "bootleg", "Bionic Commandos (bootleg, set 2)", MACHINE_SUPPORTS_SAVE )

// there's also an undumped JP new revision on which there are no extra lives after 1 million points, plus other bug-fixes / changes (possibly topsecrt2 set?)
