// license:BSD-3-Clause
// copyright-holders:Angelo Salese, David Haywood
/*********************************************************************************************************

    Double Dealer (c)NMK 1991

    driver by Angelo Salese & David Haywood, based on early work by Tomasz Slanina

    Appears to be a down-grade of the nmk16 HW

    TODO:
    - Understand better the video emulation and convert it to tilemaps;
    - Check audio IRQ frequency (controls music tempo, if too high 2 Player mode becomes slow)

==========================================================================================================

    pcb marked  GD91071

    68000P10
    YM2203C
    91071-3 (Mask ROM)
    NMK-110 8131
    NMK 901
    NMK 902
    NMK 903 x2
    82S135N ("5")
    82S129N ("6")
    xtals 16.000 MHz and 6.000 MHz
    DSW x2

*********************************************************************************************************/


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/tlcs90/tlcs90.h"
#include "machine/timer.h"
#include "sound/ymopn.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#define VERBOSE     0
#include "logmacro.h"

namespace {

class ddealer_state : public driver_device
{
public:
	ddealer_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_vregs(*this, "vregs"),
		m_back_vram(*this, "back_vram"),
		m_fg_vram(*this, "fg_vram"),
		m_work_ram(*this, "work_ram"),
		m_mcu_shared_ram(*this, "mcu_shared_ram"),
		m_in0_io(*this, "IN0"),
		m_maincpu(*this, "maincpu"),
		m_protcpu(*this, "protcpu"),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void ddealer(machine_config &config);

private:
	void flipscreen_w(u16 data);
	void back_vram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void fg_vram_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	// MCU related
	void mcu_port6_w(u8 data);
	u8 mcu_port5_r();
	u8 mcu_port6_r();
	u8 mcu_side_shared_r(offs_t offset);
	void mcu_side_shared_w(offs_t offset, u8 data);

	TILE_GET_INFO_MEMBER(get_back_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	template<unsigned Offset> TILE_GET_INFO_MEMBER(get_fg_splitted_tile_info);
	TILEMAP_MAPPER_MEMBER(scan_fg);
	void draw_video_layer(u16* vreg_base, tilemap_t *tmap, screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void ddealer_map(address_map &map);
	void prot_map(address_map &map);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	// memory pointers
	required_shared_ptr<u16> m_vregs;
	required_shared_ptr<u16> m_back_vram;
	required_shared_ptr<u16> m_fg_vram;
	required_shared_ptr<u16> m_work_ram;
	required_shared_ptr<u16> m_mcu_shared_ram;
	required_ioport m_in0_io;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<tlcs90_device> m_protcpu;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	// video-related
	tilemap_t  *m_back_tilemap;
	tilemap_t  *m_fg_tilemap; // overall foreground
	// splitted foreground area
	tilemap_t  *m_fg_tilemap_left;
	tilemap_t  *m_fg_tilemap_right;

	u8 m_bus_status;
};

void ddealer_state::machine_start()
{
	save_item(NAME(m_bus_status));
}

void ddealer_state::machine_reset()
{
	m_bus_status = 0x04;
}

void ddealer_state::mcu_port6_w(u8 data)
{
	// the actual mechanism is a little more complex, but these are written at the
	// start and end of the take / release bus function
	if (data == 0x08)
	{
		LOG("%s: mcu_port6_w 68k bus taken, 68k stopped (data %02x)\n", machine().describe_context(), data);
		m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	}
	else if (data == 0xb)
	{
		LOG("%s: mcu_port6_w 68k bus returned, 68k started (data %02x)\n", machine().describe_context(), data);
		m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
	}
	else
	{
		LOG("%s: mcu_port6_w 68k bus access (data %02x)\n", machine().describe_context(), data);
	}
}

u8 ddealer_state::mcu_port5_r()
{
	return m_screen->vpos() >> 2;
}

u8 ddealer_state::mcu_port6_r()
{
	// again this is simplified for now
	if (!machine().side_effects_disabled())
		m_bus_status ^= 0x04;
	return m_bus_status;
}

void ddealer_state::mcu_side_shared_w(offs_t offset, u8 data)
{
	m_maincpu->space(AS_PROGRAM).write_byte(offset, data);
}

u8 ddealer_state::mcu_side_shared_r(offs_t offset)
{
	u8 retval = m_maincpu->space(AS_PROGRAM).read_byte((offset));
	return retval;
}

void ddealer_state::prot_map(address_map &map)
{
	//  0x000000- 0x003fff is hidden by internal ROM, as are some 0x00fxxx addresses by RAM
	map(0x000000, 0x0fffff).rw(FUNC(ddealer_state::mcu_side_shared_r), FUNC(ddealer_state::mcu_side_shared_w));
}


void ddealer_state::flipscreen_w(u16 data)
{
	flip_screen_set(data & 0x01);
}

static inline void get_tile_info(u16 src, u32 &code, u32 &color)
{
	code = src & 0xfff;
	color = (src >> 12) & 0xf;
}

TILE_GET_INFO_MEMBER(ddealer_state::get_back_tile_info)
{
	u32 code, color;
	get_tile_info(m_back_vram[tile_index], code, color);
	tileinfo.set(0,
			code,
			color,
			0);
}

TILE_GET_INFO_MEMBER(ddealer_state::get_fg_tile_info)
{
	u32 code, color;
	get_tile_info(m_fg_vram[tile_index], code, color);
	tileinfo.set(1,
			code,
			color,
			0);
}

template<unsigned Offset>
TILE_GET_INFO_MEMBER(ddealer_state::get_fg_splitted_tile_info)
{
	u32 code, color;
	get_tile_info(m_fg_vram[Offset + (tile_index & 0x17ff)], code, color);
	tileinfo.set(1,
			code,
			color,
			0);
}

TILEMAP_MAPPER_MEMBER(ddealer_state::scan_fg)
{
	return (row & 0x0f) | ((col & 0xff) << 4) | ((row & 0x10) << 8);
}

void ddealer_state::video_start()
{
	m_back_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ddealer_state::get_back_tile_info)), TILEMAP_SCAN_COLS, 8, 8, 64, 32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode,
					tilemap_get_info_delegate(*this, FUNC(ddealer_state::get_fg_tile_info)),
					tilemap_mapper_delegate(*this, FUNC(ddealer_state::scan_fg)),
					16, 16, 256, 32);
	m_fg_tilemap_left = &machine().tilemap().create(*m_gfxdecode,
					tilemap_get_info_delegate(*this, FUNC(ddealer_state::get_fg_splitted_tile_info<0>)),
					tilemap_mapper_delegate(*this, FUNC(ddealer_state::scan_fg)),
					16, 16, 128, 32);
	m_fg_tilemap_right = &machine().tilemap().create(*m_gfxdecode,
					tilemap_get_info_delegate(*this, FUNC(ddealer_state::get_fg_splitted_tile_info<0x800>)),
					tilemap_mapper_delegate(*this, FUNC(ddealer_state::scan_fg)),
					16, 16, 128, 32);

	m_fg_tilemap->set_transparent_pen(15);
	m_fg_tilemap_left->set_transparent_pen(15);
	m_fg_tilemap_right->set_transparent_pen(15);

	m_back_tilemap->set_scrolldx(64,64);
	m_fg_tilemap->set_scrolldx(64,64);
	m_fg_tilemap_left->set_scrolldx(64,64);
	m_fg_tilemap_right->set_scrolldx(64,64);
}

void ddealer_state::draw_video_layer(u16* vreg_base, tilemap_t *tmap, screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int sx, sy;

	sx =  ((vreg_base[0x4 / 2] & 0xff));
	sx |= ((vreg_base[0x2 / 2] & 0xff) << 8);

	sy =  ((vreg_base[0x8 / 2] & 0xff));
	sy |= ((vreg_base[0x6 / 2] & 0xff) << 8);

	tmap->set_scrollx(sx);
	tmap->set_scrolly(sy);
	tmap->draw(screen, bitmap, cliprect, 0, 0);
}


u32 ddealer_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_back_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	/* the fg tilemap handling is a little hacky right now,
	   I'm not sure if it should be a single tilemap with
	   rowscroll / linescroll, or two tilemaps which can be
	   combined, the flipscreen case makes things more
	   difficult to understand */
	bool const flip = flip_screen();

	if (!flip)
	{
		if (m_vregs[0xcc / 2] & 0x80)
		{
			draw_video_layer(&m_vregs[0x1e0 / 2], m_fg_tilemap_left, screen, bitmap, cliprect);
			draw_video_layer(&m_vregs[0xcc / 2], m_fg_tilemap_right, screen, bitmap, cliprect);
		}
		else
		{
			draw_video_layer(&m_vregs[0x1e0 / 2], m_fg_tilemap, screen, bitmap, cliprect);
		}
	}
	else
	{
		if (m_vregs[0xcc / 2] & 0x80)
		{
			draw_video_layer(&m_vregs[0xcc / 2], m_fg_tilemap_left, screen, bitmap, cliprect);
			draw_video_layer(&m_vregs[0x1e0 / 2], m_fg_tilemap_right, screen, bitmap, cliprect);
		}
		else
		{
			draw_video_layer(&m_vregs[0x1e0 / 2], m_fg_tilemap, screen, bitmap, cliprect);
		}
	}

	return 0;
}

void ddealer_state::back_vram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_back_vram[offset]);
	m_back_tilemap->mark_tile_dirty(offset);
}

void ddealer_state::fg_vram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_fg_vram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset);
	if (offset & 0x800)
		m_fg_tilemap_right->mark_tile_dirty(offset & 0x17ff);
	else
		m_fg_tilemap_left->mark_tile_dirty(offset & 0x17ff);
}


void ddealer_state::ddealer_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x080000, 0x080001).portr("IN0");
	map(0x080002, 0x080003).portr("IN1");
	map(0x080006, 0x080007).portr("DSW2");
	map(0x080008, 0x080009).portr("DSW1");
	map(0x084000, 0x084003).w("ymsnd", FUNC(ym2203_device::write)).umask16(0x00ff); // ym ?
	map(0x088000, 0x0883ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x08c000, 0x08c1ff).ram().share("vregs"); // video registers

	/* this might actually be 1 tilemap with some funky rowscroll / columnscroll enabled, I'm not sure */
	// certainly seems derivative of the design used in Urashima Mahjong (jalmah.cpp), not identical tho
	map(0x090000, 0x093fff).ram().w(FUNC(ddealer_state::fg_vram_w)).share("fg_vram"); // fg tilemap
	//  map(0x094000, 0x094001).noprw(); // Set at POST via clr.w, unused afterwards
	map(0x098000, 0x098001).w(FUNC(ddealer_state::flipscreen_w));
	map(0x09c000, 0x09cfff).ram().w(FUNC(ddealer_state::back_vram_w)).share("back_vram"); // bg tilemap
	map(0x0f0000, 0x0fdfff).ram().share("work_ram");
	map(0x0fe000, 0x0fefff).ram().share("mcu_shared_ram");
	map(0x0ff000, 0x0fffff).ram();
}

static INPUT_PORTS_START( ddealer )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_SERVICE( 0x20, IP_ACTIVE_LOW ) //used, "test" in service mode, unknown purpose
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(1)
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(1)
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(0x1000, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(2)
	PORT_BIT(0x2000, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(2)
	PORT_BIT(0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Language ) )     PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Japanese ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:5,4")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x20, 0x20, "Lady Stripping" )        PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW2:2" ) /* Listed as "Always Off" */
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW2:1" ) /* Listed as "Always Off" */

	PORT_START("DSW2")
	PORT_DIPUNUSED_DIPLOC( 0x0001, IP_ACTIVE_LOW, "SW1:8" ) /* Listed as "Always Off" */
	PORT_DIPUNUSED_DIPLOC( 0x0002, IP_ACTIVE_LOW, "SW1:7" ) /* Listed as "Always Off" */
	PORT_DIPNAME( 0x001c, 0x001c, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:6,5,4")
	PORT_DIPSETTING(      0x0010, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x001c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0014, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x00e0, 0x00e0, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:3,2,1")
	PORT_DIPSETTING(      0x0080, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x00e0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x00a0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
INPUT_PORTS_END

static GFXDECODE_START( gfx_ddealer )
	GFXDECODE_ENTRY( "bgrom", 0, gfx_8x8x4_packed_msb,               0x000, 16 )
	GFXDECODE_ENTRY( "fgrom", 0, gfx_8x8x4_col_2x2_group_packed_msb, 0x100, 16 )
GFXDECODE_END

void ddealer_state::ddealer(machine_config &config)
{
	M68000(config, m_maincpu, XTAL(16'000'000)/2); /* 8MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &ddealer_state::ddealer_map);
	m_maincpu->set_vblank_int("screen", FUNC(ddealer_state::irq4_line_hold));
	m_maincpu->set_periodic_int(FUNC(ddealer_state::irq1_line_hold), attotime::from_hz(60)); //guess, controls music tempo, 112 is way too fast, 90 causes 2 player mode to be too slow

	TMP91640(config, m_protcpu, XTAL(16'000'000)/4); // Toshiba TMP91640 marked as NMK-110, with 16Kbyte internal ROM, 512bytes internal RAM
	m_protcpu->set_addrmap(AS_PROGRAM, &ddealer_state::prot_map);
	m_protcpu->port_write<6>().set(FUNC(ddealer_state::mcu_port6_w));
	m_protcpu->port_read<5>().set(FUNC(ddealer_state::mcu_port5_r));
	m_protcpu->port_read<6>().set(FUNC(ddealer_state::mcu_port6_r));

	config.set_maximum_quantum(attotime::from_hz(6000));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_ddealer);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(512, 256);
	screen.set_visarea(0*8, 48*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(ddealer_state::screen_update));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::RRRRGGGGBBBBRGBx, 0x200);

	SPEAKER(config, "mono").front_center();
	YM2203(config, "ymsnd", XTAL(6'000'000) / 8).add_route(ALL_OUTPUTS, "mono", 0.40); /* 7.5KHz */
}

ROM_START( ddealer )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "1.ic6",  0x00001, 0x20000, CRC(ce0dff50) SHA1(2d7a03f6b9609aea7511a4dc49560a901b0b9f19) )
	ROM_LOAD16_BYTE( "2.ic28", 0x00000, 0x20000, CRC(f00c346f) SHA1(bd73efb19d5f9efc88210d92a82a3f4595b41097) )

	ROM_REGION( 0x04000, "protcpu", 0 )
	ROM_LOAD( "nmk-110_ddealer.bin", 0x0000, 0x4000, CRC(088db9b4) SHA1(71946399e37ffa9293eceac637b76c9169ac16e6) ) // chip markings are identical to nmk-110 on thunder dragon, code is confirmed to be different

	ROM_REGION( 0x20000, "bgrom", 0 ) /* BG */
	ROM_LOAD( "4.ic65", 0x00000, 0x20000, CRC(4939ff1b) SHA1(af2f2feeef5520d775731a58cbfc8fcc913b7348) )

	ROM_REGION( 0x80000, "fgrom", 0 ) /* FG */
	ROM_LOAD( "3.ic64", 0x00000, 0x80000, CRC(660e367c) SHA1(54827a8998c58c578c594126d5efc18a92363eaa))

	ROM_REGION( 0x200, "user1", 0 ) /* Proms */
	ROM_LOAD( "5.ic67", 0x000, 0x100, CRC(1d3d7e17) SHA1(b5aa0d024f0c0b5f72a2d0a23d1576775a7b3826) ) // 82S135
	ROM_LOAD( "6.ic86", 0x100, 0x100, CRC(435653a2) SHA1(575b4a46ea65179de3042614da438d2f6d8b572e) ) // 82S129
ROM_END

} // anonymous namespace


GAME( 1991, ddealer, 0, ddealer, ddealer, ddealer_state, empty_init, ROT0, "NMK", "Double Dealer", MACHINE_SUPPORTS_SAVE )
