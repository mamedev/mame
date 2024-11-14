// license: BSD-3-Clause
// copyright-holders: Aaron Giles, Bryan McPhail
/***************************************************************************

    Zwackery

    © 1984 Midway

    The hardware consists of the following boards:
    - Venus CPU (B084-91668-A385)
    - Venus Video (B084-91675-A385)
    - Venus Background (B084-91672-A385)
    - Artificial Artist (B084-91671-A385)

    TODO:
    - Accurate screen timings

***************************************************************************/

#include "emu.h"
#include "csd.h"

#include "cpu/m68000/m68000.h"
#include "machine/6821pia.h"
#include "machine/6840ptm.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class zwackery_state : public driver_device
{
public:
	zwackery_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pia0(*this, "pia0"),
		m_pia1(*this, "pia1"),
		m_pia2(*this, "pia2"),
		m_ptm(*this, "ptm"),
		m_watchdog(*this, "watchdog"),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_cheap_squeak_deluxe(*this, "csd"),
		m_bg_tilemap(nullptr),
		m_fg_tilemap(nullptr)
	{ }

	void zwackery(machine_config &config);

private:
	virtual void video_start() override ATTR_COLD;
	void scanline_cb(uint32_t data);
	void videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t spriteram_r(offs_t offset);
	void spriteram_w(offs_t offset, uint8_t data);
	void update_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	void pia0_porta_w(uint8_t data);
	void pia0_irq_w(int state);
	void pia1_porta_w(uint8_t data);
	uint8_t pia1_portb_r();

	uint8_t ptm_r(offs_t offset);

	void zwackery_map(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;

	required_device<m68000_device> m_maincpu;
	required_device<pia6821_device> m_pia0;
	required_device<pia6821_device> m_pia1;
	required_device<pia6821_device> m_pia2;
	required_device<ptm6840_device> m_ptm;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint16_t> m_videoram;
	required_device<midway_cheap_squeak_deluxe_device> m_cheap_squeak_deluxe;

	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;

	std::unique_ptr<uint8_t[]> m_spriteram;

	std::unique_ptr<uint8_t[]> m_srcdata0;
	std::unique_ptr<uint8_t[]> m_srcdata2;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void zwackery_state::zwackery_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x037fff).rom();
	map(0x080000, 0x080fff).ram();
	map(0x084000, 0x084fff).ram();
	map(0x100000, 0x10000f).r(FUNC(zwackery_state::ptm_r)).umask16(0xff00).w(m_ptm, FUNC(ptm6840_device::write)).umask16(0xff00);
	map(0x104000, 0x104007).rw(m_pia0, FUNC(pia6821_device::read), FUNC(pia6821_device::write)).umask16(0xff00);
	map(0x108000, 0x108007).rw(m_pia1, FUNC(pia6821_device::read), FUNC(pia6821_device::write)).umask16(0x00ff);
	map(0x10c000, 0x10c007).rw(m_pia2, FUNC(pia6821_device::read), FUNC(pia6821_device::write)).umask16(0x00ff);
	map(0x800000, 0x800fff).ram().w(FUNC(zwackery_state::videoram_w)).share("videoram");
	map(0x802000, 0x803fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xc00000, 0xc00fff).rw(FUNC(zwackery_state::spriteram_r), FUNC(zwackery_state::spriteram_w)).umask16(0x00ff);
}


//**************************************************************************
//  INPUTS
//**************************************************************************

static INPUT_PORTS_START( zwackery )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE( 0x0010, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Sword")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_CUSTOM )    // sound communications

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Spell Up")
	PORT_BIT( 0x3e, IP_ACTIVE_HIGH, IPT_UNUSED ) // encoder wheel
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Shield")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Spell Down")

	PORT_START("IN3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage ) )     PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x05, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x38, 0x00, "Buy-in" )               PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(    0x00, "1 coin" )
	PORT_DIPSETTING(    0x08, "2 coins" )
	PORT_DIPSETTING(    0x10, "3 coins" )
	PORT_DIPSETTING(    0x18, "4 coins" )
	PORT_DIPSETTING(    0x20, "5 coins" )
	PORT_DIPSETTING(    0x28, "6 coins" )
	PORT_DIPSETTING(    0x30, "7 coins" )
	PORT_DIPSETTING(    0x38, DEF_STR( None ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Difficulty ) )  PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0xc0, DEF_STR( Easier ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Harder ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Hardest ) )

	PORT_START("IN5")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_CODE_DEC(KEYCODE_Z) PORT_CODE_INC(KEYCODE_X) PORT_REVERSE
INPUT_PORTS_END


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

void zwackery_state::video_start()
{
	const uint8_t *colordatabase = (const uint8_t *)memregion("bg_color")->base();
	gfx_element *gfx0 = m_gfxdecode->gfx(0);
	gfx_element *gfx2 = m_gfxdecode->gfx(2);

	// initialize the background tilemap
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(zwackery_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS,  16,16, 32,32);

	// initialize the foreground tilemap
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(zwackery_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS,  16,16, 32,32);
	m_fg_tilemap->set_transparent_pen(0);

	// allocate memory for the assembled gfx data
	m_srcdata0 = std::make_unique<uint8_t[]>(gfx0->elements() * gfx0->width() * gfx0->height());
	m_srcdata2 = std::make_unique<uint8_t[]>(gfx2->elements() * gfx2->width() * gfx2->height());

	// "colorize" each code
	uint8_t *dest0 = m_srcdata0.get();
	uint8_t *dest2 = m_srcdata2.get();
	for (int code = 0; code < gfx0->elements(); code++)
	{
		const uint8_t *coldata = colordatabase + code * 32;
		const uint8_t *gfxdata0 = gfx0->get_data(code);
		const uint8_t *gfxdata2 = gfx2->get_data(code);

		// assume 16 rows
		for (int y = 0; y < 16; y++)
		{
			const uint8_t *gd0 = gfxdata0;
			const uint8_t *gd2 = gfxdata2;

			// 16 columns
			for (int x = 0; x < 16; x++, gd0++, gd2++)
			{
				int coloffs = (y & 0x0c) | ((x >> 2) & 0x03);
				int pen0 = coldata[coloffs * 2 + 0];
				int pen1 = coldata[coloffs * 2 + 1];
				int tp0, tp1;

				// every 4 pixels gets its own foreground/background colors
				*dest0++ = *gd0 ? pen1 : pen0;

				// for gfx 2, we convert all low-priority pens to 0
				tp0 = (pen0 & 0x80) ? pen0 : 0;
				tp1 = (pen1 & 0x80) ? pen1 : 0;
				*dest2++ = *gd2 ? tp1 : tp0;
			}

			// advance
			gfxdata0 += gfx0->rowbytes();
			gfxdata2 += gfx2->rowbytes();
		}
	}

	// make the assembled data our new source data
	gfx0->set_raw_layout(m_srcdata0.get(), gfx0->width(), gfx0->height(), gfx0->elements(), 8 * gfx0->width(), 8 * gfx0->width() * gfx0->height());
	gfx2->set_raw_layout(m_srcdata2.get(), gfx2->width(), gfx2->height(), gfx2->elements(), 8 * gfx2->width(), 8 * gfx2->width() * gfx2->height());
}

void zwackery_state::scanline_cb(uint32_t data)
{
	switch (data)
	{
	case 0:
		// VSYNC
		m_ptm->set_c1(0);
		m_ptm->set_c1(1);
		break;

	case 474:
		// source of this signal is pal.d3 (current value taken from original driver)
		m_pia0->ca1_w(1);
		break;

	case 475:
		// turn it off again after one scanline
		m_pia0->ca1_w(0);
		break;
	}

	// HSYNC
	m_ptm->set_c3(0);
	m_ptm->set_c3(1);
}

void zwackery_state::videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_videoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset);
	m_fg_tilemap->mark_tile_dirty(offset);
}

uint8_t zwackery_state::spriteram_r(offs_t offset)
{
	return m_spriteram[offset];
}

void zwackery_state::spriteram_w(offs_t offset, uint8_t data)
{
	m_spriteram[offset] = data;
}

void zwackery_state::update_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority)
{
	screen.priority().fill(1, cliprect);

	// loop over sprite RAM
	for (int offs = 0x800 - 4; offs >= 0; offs -= 4)
	{
		int code, color, flipx, flipy, x, y, flags;

		// get the code and skip if zero
		code = m_spriteram[offs + 2];
		if (code == 0)
			continue;

		// extract the flag bits and determine the color
		flags = m_spriteram[offs + 1];
		color = ((~flags >> 2) & 0x0f) | ((flags & 0x02) << 3);

		// for low priority, draw everything but color 7
		if (!priority)
		{
			if (color == 7)
				continue;
		}

		// for high priority, only draw color 7
		else
		{
			if (color != 7)
				continue;
		}

		// determine flipping and coordinates
		flipx = ~flags & 0x40;
		flipy = flags & 0x80;
		x = (231 - m_spriteram[offs + 3]) * 2;
		y = (241 - m_spriteram[offs]) * 2;

		if (x <= -32) x += 512;

		// sprites use color 0 for background pen and 8 for the 'under tile' pen.
		// The color 8 is used to cover over other sprites.

		// first draw the sprite, visible
		m_gfxdecode->gfx(1)->prio_transmask(bitmap,cliprect, code, color, flipx, flipy, x, y,
				screen.priority(), 0x00, 0x0101);

		// then draw the mask, behind the background but obscuring following sprites
		m_gfxdecode->gfx(1)->prio_transmask(bitmap,cliprect, code, color, flipx, flipy, x, y,
				screen.priority(), 0x02, 0xfeff);
	}
}

uint32_t zwackery_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// draw the background
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	// draw the low-priority sprites
	update_sprites(screen, bitmap, cliprect, 0);

	// redraw tiles with priority over sprites
	m_fg_tilemap->draw(screen, bitmap, cliprect, 1, 0);

	// draw the high-priority sprites
	update_sprites(screen, bitmap, cliprect, 1);

	return 0;
}

TILE_GET_INFO_MEMBER( zwackery_state::get_bg_tile_info )
{
	uint16_t data = m_videoram[tile_index];
	int color = (data >> 13) & 7;
	tileinfo.set(0, data & 0x3ff, color, TILE_FLIPYX(data >> 11));
}

TILE_GET_INFO_MEMBER( zwackery_state::get_fg_tile_info )
{
	uint16_t data = m_videoram[tile_index];
	int color = (data >> 13) & 7;
	tileinfo.set(2, data & 0x3ff, color, TILE_FLIPYX(data >> 11));
	tileinfo.category = (color != 0);
}

static const gfx_layout zwackery_layout =
{
	16,16,
	RGN_FRAC(1,2),
	8,
	{ 0, 0, 0, 0, 0, 0, 0, 0 },
	{ STEP4(3,-1), STEP4(11,-1), STEP4(19,-1), STEP4(27,-1) },
	{ 4, RGN_FRAC(1,2)+4, 0, RGN_FRAC(1,2)+0, 36, RGN_FRAC(1,2)+36, 32, RGN_FRAC(1,2)+32,
		68, RGN_FRAC(1,2)+68, 64, RGN_FRAC(1,2)+64, 100, RGN_FRAC(1,2)+100, 96, RGN_FRAC(1,2)+96 },
	128
};

static const gfx_layout mcr68_sprite_layout =
{
	32,32,
	RGN_FRAC(1,4),
	4,
	{ STEP4(0,1) },
	{ STEP2(RGN_FRAC(0,4)+0,4), STEP2(RGN_FRAC(1,4)+0,4), STEP2(RGN_FRAC(2,4)+0,4), STEP2(RGN_FRAC(3,4)+0,4),
		STEP2(RGN_FRAC(0,4)+8,4), STEP2(RGN_FRAC(1,4)+8,4), STEP2(RGN_FRAC(2,4)+8,4), STEP2(RGN_FRAC(3,4)+8,4),
		STEP2(RGN_FRAC(0,4)+16,4), STEP2(RGN_FRAC(1,4)+16,4), STEP2(RGN_FRAC(2,4)+16,4), STEP2(RGN_FRAC(3,4)+16,4),
		STEP2(RGN_FRAC(0,4)+24,4), STEP2(RGN_FRAC(1,4)+24,4), STEP2(RGN_FRAC(2,4)+24,4), STEP2(RGN_FRAC(3,4)+24,4) },
	{ STEP32(0,32) },
	32*32
};

static GFXDECODE_START( gfx_zwackery )
	GFXDECODE_ENTRY( "gfx1",    0, zwackery_layout,     0,     16 )
	GFXDECODE_ENTRY( "sprites", 0, mcr68_sprite_layout, 0x800, 32 )
	GFXDECODE_ENTRY( "gfx1",    0, zwackery_layout,     0,     16 )  // yes, an extra copy
GFXDECODE_END


//**************************************************************************
//  AUDIO
//**************************************************************************

void zwackery_state::pia1_porta_w(uint8_t data)
{
	m_cheap_squeak_deluxe->sr_w(data >> 4);
}


//**************************************************************************
//  INPUTS/OUTPUTS
//**************************************************************************

void zwackery_state::pia0_porta_w(uint8_t data)
{
	// bits 0, 1 and 2 control meters?
	// bits 3 and 4 control coin counters?
	// bits 5 and 6 control hflip/vflip

	// bit 7, watchdog
	m_watchdog->reset_line_w(BIT(data, 7));
}

void zwackery_state::pia0_irq_w(int state)
{
	int irq_state = m_pia0->irq_a_state() | m_pia0->irq_b_state();
	m_maincpu->set_input_line(5, irq_state ? ASSERT_LINE : CLEAR_LINE);
}

uint8_t zwackery_state::pia1_portb_r()
{
	uint8_t result = ioport("IN2")->read();
	uint8_t wheel = ioport("IN5")->read();

	return result | ((wheel >> 2) & 0x3e);
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

// Zwackery does a timer test:
// It loads $1388 into one of the timers clocked by E
// Then it sits in a tight loop counting down from $4E4
//       BTST #$1,($2,A0)
//       DBNE D1,*-6
// It expects D1 to end up between 0 and 5; in order to
// make this happen, we must assume that reads from the
// 6840 take 14 additional cycles
uint8_t zwackery_state::ptm_r(offs_t offset)
{
	m_maincpu->adjust_icount(-14);
	return m_ptm->read(offset);
}

void zwackery_state::machine_start()
{
	// allocate 8-bit spriteram
	m_spriteram = std::make_unique<uint8_t[]>(0x800);

	// register for save states
	save_pointer(NAME(m_spriteram), 0x800);
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void zwackery_state::zwackery(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 7652400);    // based on counter usage, should be XTAL(16'000'000)/2
	m_maincpu->set_addrmap(AS_PROGRAM, &zwackery_state::zwackery_map);

	WATCHDOG_TIMER(config, m_watchdog);

	PTM6840(config, m_ptm, 7652400 / 10);
	m_ptm->irq_callback().set_inputline("maincpu", 6);

	PIA6821(config, m_pia0);
	m_pia0->readpb_handler().set_ioport("IN0");
	m_pia0->writepa_handler().set(FUNC(zwackery_state::pia0_porta_w));
	m_pia0->irqa_handler().set(FUNC(zwackery_state::pia0_irq_w));
	m_pia0->irqb_handler().set(FUNC(zwackery_state::pia0_irq_w));

	PIA6821(config, m_pia1);
	m_pia1->readpa_handler().set_ioport("IN1");
	m_pia1->writepa_handler().set(FUNC(zwackery_state::pia1_porta_w));
	m_pia1->readpb_handler().set(FUNC(zwackery_state::pia1_portb_r));
	m_pia1->ca2_handler().set(m_cheap_squeak_deluxe, FUNC(midway_cheap_squeak_deluxe_device::sirq_w));

	PIA6821(config, m_pia2);
	m_pia2->readpa_handler().set_ioport("IN3");
	m_pia2->readpb_handler().set_ioport("DSW");

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(30);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	m_screen->set_size(32*16, 30*16);
	m_screen->set_visarea(0, 32*16-1, 0, 30*16-1);
	m_screen->set_screen_update(FUNC(zwackery_state::screen_update));
	m_screen->set_palette(m_palette);
	m_screen->scanline().set(FUNC(zwackery_state::scanline_cb));

	GFXDECODE(config, m_gfxdecode, "palette", gfx_zwackery);
	PALETTE(config, "palette").set_format(palette_device::xRBG_555_inverted, 4096);

	// sound hardware
	SPEAKER(config, "speaker").front_center();

	MIDWAY_CHEAP_SQUEAK_DELUXE(config, m_cheap_squeak_deluxe);
	m_cheap_squeak_deluxe->add_route(ALL_OUTPUTS, "speaker", 1.0);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( zwackery )
	ROM_REGION(0x40000, "maincpu", 0)
	ROM_LOAD16_BYTE("385a-42aae-jxrd.a6",  0x00000, 0x4000, CRC(6fb9731c) SHA1(ee5b297ef2b4cf20df5e776f1c585b51f174bfa7))
	ROM_LOAD16_BYTE("385a-42aae-kxrd.b6",  0x00001, 0x4000, CRC(84b92555) SHA1(9b4af81374828c1742c1e13fc425eea2973b0867))
	ROM_LOAD16_BYTE("385a-42aae-lxrd.a7",  0x08000, 0x4000, CRC(e6977a2a) SHA1(602bf3f7e0f4080cb5b72d8fd3ee9fd11f27c558))
	ROM_LOAD16_BYTE("385a-42aae-mxrd.b7",  0x08001, 0x4000, CRC(f5d0a60e) SHA1(7e0e4936cb37ac16d6db5533ae4aecdfb07ead93))
	ROM_LOAD16_BYTE("385a-42aae-nxrd.a8",  0x10000, 0x4000, CRC(ec5841d9) SHA1(4bafe614e8993994b0ea9aedc8dc2474361e4594))
	ROM_LOAD16_BYTE("385a-42aae-pxrd.b8",  0x10001, 0x4000, CRC(d7d99ce0) SHA1(fdf428ab9c96dae555d49bac47495613ba265452))
	ROM_LOAD16_BYTE("385a-42aae-rxrd.a9",  0x18000, 0x4000, CRC(b9fe7bf5) SHA1(a94f80f49b4520a2c1098eee8983560b4ecdf3d5))
	ROM_LOAD16_BYTE("385a-42aae-txrd.b9",  0x18001, 0x4000, CRC(5e261b3b) SHA1(dcf99f528c9e3b4f8b52d413c088559bfb37d733))
	ROM_LOAD16_BYTE("385a-42aae-uxrd.a10", 0x20000, 0x4000, CRC(55e380a5) SHA1(e3fef8486858cd714086449327a93b4a70ae73ff))
	ROM_LOAD16_BYTE("385a-42aae-vxrd.b10", 0x20001, 0x4000, CRC(12249dca) SHA1(154170286047ea78645d45dfdd895a597dad17da))
	ROM_LOAD16_BYTE("385a-42aae-wxrd.a11", 0x28000, 0x4000, CRC(6a39a8ca) SHA1(8ac9c3e60dc6f1918bfb95acf3ee170cedfb20ea))
	ROM_LOAD16_BYTE("385a-42aae-yxrd.b11", 0x28001, 0x4000, CRC(ad6b45bc) SHA1(118496e898654b028f008a3d493e693ba000ef38))
	ROM_LOAD16_BYTE("385a-42aae-zxrd.a12", 0x30000, 0x4000, CRC(e2d25e1f) SHA1(5d8ff303441eccf431422b453a173983a4513630))
	ROM_LOAD16_BYTE("385a-42aae-1xrd.b12", 0x30001, 0x4000, CRC(e131f9b8) SHA1(08b131f2acc84d4c2c931bfd24e7de3d92a8a817))

	ROM_REGION(0x8000, "csd:cpu", 0)
	ROM_LOAD16_BYTE("385a-22aae-aamd.u7",  0x0000, 0x2000, CRC(5501f54b) SHA1(84c0851fb868e81400cfe3ebfd7b91fe98a47bac))
	ROM_LOAD16_BYTE("385a-22aae-bamd.u17", 0x0001, 0x2000, CRC(2e482580) SHA1(92bd3e64ff580800ee16579d97bcb8b3bd9f755c))
	ROM_LOAD16_BYTE("385a-22aae-camd.u8",  0x4000, 0x2000, CRC(13366575) SHA1(bcf25a7d4c6b2ccd7cd9978edafc66ef0cadfe72))
	ROM_LOAD16_BYTE("385a-22aae-damd.u18", 0x4001, 0x2000, CRC(bcfe5820) SHA1(ca32daa645851a2373b3cdb8a5e63ebda84aa762))

	ROM_REGION(0x8000, "gfx1", ROMREGION_INVERT)
	ROM_LOAD("385a-42aae-2xrd.1h", 0x0000, 0x4000, CRC(a7237eb1) SHA1(197e5838ac2bc732ae9eb33a9257b9391d50abf8))
	ROM_LOAD("385a-42aae-3xrd.1g", 0x4000, 0x4000, CRC(626cc69b) SHA1(86142bafa78f45d1a0bed0b83f3558b21384fa1a))

	ROM_REGION(0x20000, "sprites", 0)
	ROM_LOAD("385a-42aae-axrd.6h",  0x00000, 0x4000, CRC(a51158dc) SHA1(8d3b0054950443fdf57f83dcb973d05f6c7ad9c8))
	ROM_LOAD("385a-42aae-exrd.7h",  0x04000, 0x4000, CRC(941feecf) SHA1(8e88c956332e78dc7e55139879f2272116415714))
	ROM_LOAD("385a-42aae-bxrd.6j",  0x08000, 0x4000, CRC(f3eef316) SHA1(026e18bdfdda8cc9d0774e6d9d758686bf16992c))
	ROM_LOAD("385a-42aae-fxrd.7j",  0x0c000, 0x4000, CRC(a8a34033) SHA1(abd9fde84bb079c84126ad04d584ec03b44b60cd))
	ROM_LOAD("385a-42aae-cxrd.10h", 0x10000, 0x4000, CRC(a99daea6) SHA1(c323e05f398b7e9e04b75fd8ac5e8ab675236d66))
	ROM_LOAD("385a-42aae-gxrd.11h", 0x14000, 0x4000, CRC(c1a767fb) SHA1(c16e09b39b09d409b534ce4c53366e43237a3759))
	ROM_LOAD("385a-42aae-dxrd.10j", 0x18000, 0x4000, CRC(4dd04376) SHA1(069b64397e7a961c1fc246671472f759bd9f6c03))
	ROM_LOAD("385a-42aae-hxrd.11j", 0x1c000, 0x4000, CRC(e8c6a880) SHA1(dd3d52ddbc36e244b96cfb87e6a80adb94626407))

	ROM_REGION( 0x8000, "bg_color", 0 )
	ROM_LOAD16_BYTE("385a-42aae-5xrd.1f", 0x0000, 0x4000, CRC(a0dfcd7e) SHA1(0fc6723eddef2a96de9bf1f48006dd067c148540))
	ROM_LOAD16_BYTE("385a-42aae-4xrd.1e", 0x0001, 0x4000, CRC(ab504dc8) SHA1(4ebdcd42624e94c29ccdb8247bfff2d8e936ddd7))

	ROM_REGION( 0x000c, "plds", 0 )
	// located on the "Venus CPU" board
	ROM_LOAD( "pal.d5",    0x0000, 0x00001, NO_DUMP ) // H-T
	ROM_LOAD( "pal.d2",    0x0001, 0x00001, NO_DUMP ) // V-T
	ROM_LOAD( "pal.d4",    0x0002, 0x00001, NO_DUMP ) // MISC V&H PAL
	ROM_LOAD( "pal.d3",    0x0003, 0x00001, NO_DUMP ) // MISC CUSTOM PAL
	ROM_LOAD( "pal.e6",    0x0004, 0x00001, NO_DUMP ) // CPU WTS PAL
	ROM_LOAD( "pal.f8",    0x0005, 0x00001, NO_DUMP ) // CPU IOC PAL
	ROM_LOAD( "pal.a5",    0x0006, 0x00001, NO_DUMP ) // CPU RMD PAL
	// located on the "Venus VIDEO" board
	ROM_LOAD( "pal.1f",    0x0007, 0x00001, NO_DUMP ) // PAL FGBDCD
	ROM_LOAD( "pal.1d",    0x0008, 0x00001, NO_DUMP ) // PAL HCT
	// located on the "Venus BACKGROUND" board
	ROM_LOAD( "pal.1c",    0x0009, 0x00001, NO_DUMP ) // BGBPE PAL
	ROM_LOAD( "pal.5c",    0x000a, 0x00001, NO_DUMP ) // HCT PAL
	ROM_LOAD( "pal.5j",    0x000b, 0x00001, NO_DUMP ) // BGBDCD PAL
ROM_END

} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME      PARENT  MACHINE   INPUT     CLASS           INIT        ROTATION  COMPANY         FULLNAME    FLAGS
GAME( 1984, zwackery, 0,      zwackery, zwackery, zwackery_state, empty_init, ROT0,     "Bally Midway", "Zwackery", MACHINE_SUPPORTS_SAVE )
