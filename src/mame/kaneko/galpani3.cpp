// license:BSD-3-Clause
// copyright-holders:David Haywood, Uki
/*
    Gals Panic 3
    (c) Kaneko 1995

    Driver by David Haywood

    Original Skeleton driver by David Haywood
    Early Progress by Sebastien Volpe

Check done by main code, as part of EEPROM data:
'Gals Panic 3 v0.96 95/08/29(Tue)'

 Sprites are from Supernova
 Backgrounds are 3x bitmap layers + some kind of priority / mask layer
 The bitmaps have blitter devices to decompress RLE rom data into them

*/



/*

Gals Panic 3 (JPN Ver.)
(c)1995 Kaneko

CPU:    68000-16
Sound:  YMZ280B-F
OSC:    28.6363MHz
        33.3333MHz
EEPROM: 93C46
Chips.: GRAP2 x3                <- R/G/B Chips?
        APRIO-GL
        BABY004
        GCNT2
        TBSOP01                 <- ToyBox NEC uPD78324 series MCU with 32K internal rom
        CG24173 6186            <- Sprites, see suprnova.c
        CG24143 4181            <- ^


G3P0J1.71     prg.
G3P1J1.102

GP340000.123  chr.
GP340100.122
GP340200.121
GP340300.120
G3G0J0.101
G3G1J0.100

G3D0X0.134

GP320000.1    OBJ chr.

GP310000.41   sound data
GP310100.40


--- Team Japump!!! ---
Dumped by Uki
10/22/2000

*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/eepromser.h"
#include "kaneko_toybox.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "sound/ymz280b.h"
#include "kaneko_grap2.h"
#include "sknsspr.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class galpani3_state : public driver_device
{
public:
	galpani3_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_grap2(*this,"grap2_%u", 0),
		m_palette(*this, "palette"),
		m_spritegen(*this, "spritegen"),
		m_paletteram(*this, "palette"),
		m_spriteram(*this, "spriteram"),
		m_priority_buffer(*this, "priority_buffer"),
		m_sprregs(*this, "sprregs"),
		m_sprite_bitmap(1024, 1024)
	{ }

	void galpani3(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device_array<kaneko_grap2_device, 3> m_grap2;
	required_device<palette_device> m_palette;
	required_device<sknsspr_device> m_spritegen;

	required_shared_ptr<uint16_t> m_paletteram;
	optional_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_priority_buffer;
	required_shared_ptr<uint16_t> m_sprregs;

	bitmap_ind16 m_sprite_bitmap;
	uint16_t m_priority_buffer_scrollx = 0;
	uint16_t m_priority_buffer_scrolly = 0;
	std::unique_ptr<uint32_t[]> m_spriteram32;
	std::unique_ptr<uint32_t[]> m_spc_regs;

	void galpani3_suprnova_sprite32_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void galpani3_suprnova_sprite32regs_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void galpani3_priority_buffer_scrollx_w(uint16_t data);
	void galpani3_priority_buffer_scrolly_w(uint16_t data);

	virtual void video_start() override ATTR_COLD;

	uint32_t screen_update_galpani3(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(galpani3_vblank);
	void galpani3_map(address_map &map) ATTR_COLD;
};


/***************************************************************************

 video

***************************************************************************/



TIMER_DEVICE_CALLBACK_MEMBER(galpani3_state::galpani3_vblank)// 2, 3, 5 ?
{
	int scanline = param;

	if(scanline == 240)
		m_maincpu->set_input_line(2, HOLD_LINE);

	if(scanline == 0)
		m_maincpu->set_input_line(3, HOLD_LINE);

	if(scanline == 128)
		m_maincpu->set_input_line(5, HOLD_LINE); // timer, related to sound chip?
}


void galpani3_state::video_start()
{
	/* so we can use video/sknsspr.c */
	m_spritegen->skns_sprite_kludge(0,0);

	m_spriteram32 = make_unique_clear<uint32_t[]>(0x4000/4);
	m_spc_regs = make_unique_clear<uint32_t[]>(0x40/4);

	save_item(NAME(m_priority_buffer_scrollx));
	save_item(NAME(m_priority_buffer_scrolly));
	save_pointer(NAME(m_spriteram32), 0x4000/4);
	save_pointer(NAME(m_spc_regs), 0x40/4);
}

#define SPRITE_DRAW_PIXEL(_pri)                                    \
	if (((sprdat & 0xc000) == _pri) && ((sprdat & 0xff) != 0))     \
	{                                                              \
		dst[drawx] = paldata[sprdat & 0x3fff];                     \
	}

// Switchable brightness value in highest bit of palette
// TODO : m_framebuffer_bright1 is alpha-blended?
#define FB_DRAW_PIXEL(_chip, _pixel)                                                              \
	int alpha = 0xff;                                                                             \
	const pen_t &pal = m_grap2[_chip]->pen(_pixel);                                               \
	if (m_grap2[_chip]->m_framebuffer_palette[_pixel] & 0x8000)                                   \
	{                                                                                             \
		alpha = (m_grap2[_chip]->m_framebuffer_bright2 & 0xff);                                   \
	}                                                                                             \
	else                                                                                          \
	{                                                                                             \
		alpha = (m_grap2[_chip]->m_framebuffer_bright1 & 0xff);                                   \
	}                                                                                             \
	if (alpha)                                                                                    \
	{                                                                                             \
		if (alpha == 0xff)                                                                        \
			dst[drawx] = pal;                                                                     \
		else                                                                                      \
			dst[drawx] = alpha_blend_r32(dst[drawx], pal, alpha);                                 \
	}

uint32_t galpani3_state::screen_update_galpani3(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	pen_t const *const paldata = m_palette->pens();

	bitmap.fill(0, cliprect);

	m_spritegen->skns_draw_sprites(m_sprite_bitmap, cliprect, m_spriteram32.get(), 0x4000, m_spc_regs.get() );

//  popmessage("%02x %02x", m_grap2[0]->m_framebuffer_bright2, m_grap2[1]->m_framebuffer_bright2);

	for (int drawy=cliprect.min_y;drawy<=cliprect.max_y;drawy++)
	{
		uint16_t const *const sprline  = &m_sprite_bitmap.pix(drawy);
		uint16_t const *const srcline1 = m_grap2[0]->m_framebuffer.get() + ((drawy+m_grap2[0]->m_framebuffer_scrolly+11)&0x1ff) * 0x200;
		uint16_t const *const srcline2 = m_grap2[1]->m_framebuffer.get() + ((drawy+m_grap2[1]->m_framebuffer_scrolly+11)&0x1ff) * 0x200;
		uint16_t const *const srcline3 = m_grap2[2]->m_framebuffer.get() + ((drawy+m_grap2[2]->m_framebuffer_scrolly+11)&0x1ff) * 0x200;

		uint16_t const *const priline  = m_priority_buffer + ((drawy+m_priority_buffer_scrolly+11)&0x1ff) * 0x200;

		uint32_t *const dst = &bitmap.pix(drawy & 0x3ff);

		for (int drawx=cliprect.min_x;drawx<=cliprect.max_x;drawx++)
		{
			int sproffs  = drawx & 0x3ff;
			int srcoffs1 = (drawx+m_grap2[0]->m_framebuffer_scrollx+67)&0x1ff;
			int srcoffs2 = (drawx+m_grap2[1]->m_framebuffer_scrollx+67)&0x1ff;
			int srcoffs3 = (drawx+m_grap2[2]->m_framebuffer_scrollx+67)&0x1ff;

			int prioffs  = (drawx+m_priority_buffer_scrollx+66)&0x1ff;

			uint16_t sprdat = sprline[sproffs];
			uint8_t  dat1 = srcline1[srcoffs1];
			uint8_t  dat2 = srcline2[srcoffs2];
			uint8_t  dat3 = srcline3[srcoffs3];

			uint8_t  pridat = priline[prioffs];

			// TODO : Verify priorities, blendings from real PCB.
			if (pridat==0x0f) // relates to the area you've drawn over
			{
				SPRITE_DRAW_PIXEL(0x0000);
				if (m_grap2[2]->m_framebuffer_enable)
				{
					FB_DRAW_PIXEL(2, dat3);
				}
				SPRITE_DRAW_PIXEL(0x4000);
				if (dat1 && m_grap2[0]->m_framebuffer_enable)
				{
					FB_DRAW_PIXEL(0, dat1);
				}
				SPRITE_DRAW_PIXEL(0x8000);
				if (dat2 && m_grap2[1]->m_framebuffer_enable)
				{
					FB_DRAW_PIXEL(1, dat2);
				}
				SPRITE_DRAW_PIXEL(0xc000);
			}
			else if (pridat==0xcf) // the girl
			{
				SPRITE_DRAW_PIXEL(0x0000);
				if (m_grap2[0]->m_framebuffer_enable)
				{
					FB_DRAW_PIXEL(0, 0x100);
				}
				SPRITE_DRAW_PIXEL(0x4000);
				if (m_grap2[1]->m_framebuffer_enable)
				{
					FB_DRAW_PIXEL(1, 0x100);
				}
				SPRITE_DRAW_PIXEL(0x8000);
				if (dat3 && m_grap2[2]->m_framebuffer_enable)
				{
					FB_DRAW_PIXEL(2, dat3);
				}
				SPRITE_DRAW_PIXEL(0xc000);
			}
			else if (pridat==0x30) // during the 'gals boxes' on the intro
			{
				SPRITE_DRAW_PIXEL(0x0000);
				if (m_grap2[1]->m_framebuffer_enable) // TODO : Opaqued and Swapped order?
				{
					FB_DRAW_PIXEL(1, dat2);
				}
				SPRITE_DRAW_PIXEL(0x4000);
				if (dat1 && m_grap2[0]->m_framebuffer_enable)
				{
					FB_DRAW_PIXEL(0, dat1);
				}
				SPRITE_DRAW_PIXEL(0x8000);
				if (dat3 && m_grap2[2]->m_framebuffer_enable)
				{
					FB_DRAW_PIXEL(2, dat3);
				}
				SPRITE_DRAW_PIXEL(0xc000);
			}
			else
			{
				SPRITE_DRAW_PIXEL(0x0000);
				if (m_grap2[0]->m_framebuffer_enable) // TODO : Opaque drawing 1st framebuffer in real PCB?
				{
					FB_DRAW_PIXEL(0, dat1);
				}
				SPRITE_DRAW_PIXEL(0x4000);
				if (dat2 && m_grap2[1]->m_framebuffer_enable)
				{
					FB_DRAW_PIXEL(1, dat2);
				}
				SPRITE_DRAW_PIXEL(0x8000);
				if (dat3 && m_grap2[2]->m_framebuffer_enable)
				{
					FB_DRAW_PIXEL(2, dat3);
				}
				SPRITE_DRAW_PIXEL(0xc000);
			}

			/*
			else if (pridat==0x2f) // area outside of the girl
			{
			    //dst[drawx] = machine().rand()&0x3fff;
			}

			else if (pridat==0x00) // the initial line / box that gets drawn
			{
			    //dst[drawx] = machine().rand()&0x3fff;
			}
			else if (pridat==0x30) // during the 'gals boxes' on the intro
			{
			    //dst[drawx] = machine().rand()&0x3fff;
			}
			else if (pridat==0x0c) // 'nice' at end of level
			{
			    //dst[drawx] = machine().rand()&0x3fff;
			}
			else
			{
			    //printf("%02x, ",pridat);
			}
			*/
		}
	}
	return 0;
}


static INPUT_PORTS_START( galpani3 )
	PORT_START("P1")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // ?
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // ?
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COIN")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN1  ) PORT_IMPULSE(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN2  ) PORT_IMPULSE(2)
	PORT_SERVICE_NO_TOGGLE( 0x1000, IP_ACTIVE_LOW )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_TILT     )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")  /* provided by the MCU - $200386.b <- $400200 */
	PORT_DIPNAME( 0x0100, 0x0100, "Test Mode" )     PORT_DIPLOCATION("DSW:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("DSW:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("DSW:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0800, IP_ACTIVE_LOW, "DSW:4" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x1000, IP_ACTIVE_LOW, "DSW:5" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x2000, IP_ACTIVE_LOW, "DSW:6" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x4000, IP_ACTIVE_LOW, "DSW:7" ) /* Listed as "Unused" */
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW:8")  // unused ?
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


void galpani3_state::galpani3_suprnova_sprite32_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_spriteram[offset]);
	offset>>=1;
	m_spriteram32[offset]=(m_spriteram[offset*2+1]<<16) | (m_spriteram[offset*2]);
}

void galpani3_state::galpani3_suprnova_sprite32regs_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_sprregs[offset]);
	offset>>=1;
	m_spc_regs[offset]=(m_sprregs[offset*2+1]<<16) | (m_sprregs[offset*2]);
}

void galpani3_state::galpani3_priority_buffer_scrollx_w(uint16_t data)
{
	m_priority_buffer_scrollx = data;
}

void galpani3_state::galpani3_priority_buffer_scrolly_w(uint16_t data)
{
	m_priority_buffer_scrolly = data;
}


void galpani3_state::galpani3_map(address_map &map)
{
	map(0x000000, 0x17ffff).rom();

	map(0x200000, 0x20ffff).ram(); // area [B] - Work RAM
	map(0x280000, 0x287fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette"); // area [A] - palette for sprites

	map(0x300000, 0x303fff).ram().w(FUNC(galpani3_state::galpani3_suprnova_sprite32_w)).share("spriteram");
	map(0x380000, 0x38003f).ram().w(FUNC(galpani3_state::galpani3_suprnova_sprite32regs_w)).share("sprregs");

	map(0x400000, 0x40ffff).ram().share("mcuram"); // area [C]

	map(0x580000, 0x580001).w("toybox", FUNC(kaneko_toybox_device::mcu_com0_w));
	map(0x600000, 0x600001).w("toybox", FUNC(kaneko_toybox_device::mcu_com1_w));
	map(0x680000, 0x680001).w("toybox", FUNC(kaneko_toybox_device::mcu_com2_w));
	map(0x700000, 0x700001).w("toybox", FUNC(kaneko_toybox_device::mcu_com3_w));
	map(0x780000, 0x780001).r("toybox", FUNC(kaneko_toybox_device::mcu_status_r));

	map(0x800000, 0x9fffff).m("grap2_0", FUNC(kaneko_grap2_device::grap2_map));
	map(0xa00000, 0xbfffff).m("grap2_1", FUNC(kaneko_grap2_device::grap2_map));
	map(0xc00000, 0xdfffff).m("grap2_2", FUNC(kaneko_grap2_device::grap2_map));

	// ?? priority / alpha buffer?
	map(0xe00000, 0xe7ffff).ram().share("priority_buffer"); // area [J] - A area ? odd bytes only, initialized 00..ff,00..ff,..., then cleared
	map(0xe80000, 0xe80001).w(FUNC(galpani3_state::galpani3_priority_buffer_scrollx_w)); // scroll?
	map(0xe80002, 0xe80003).w(FUNC(galpani3_state::galpani3_priority_buffer_scrolly_w)); // scroll?


	map(0xf00000, 0xf00001).noprw(); // ? written once (2nd opcode, $1.b)
	map(0xf00010, 0xf00011).portr("P1");
	map(0xf00012, 0xf00013).portr("P2");
	map(0xf00014, 0xf00015).portr("COIN");
	map(0xf00016, 0xf00017).noprw(); // ? read, but overwritten
	map(0xf00020, 0xf00023).w("ymz", FUNC(ymz280b_device::write)).umask16(0x00ff);     // sound
	map(0xf00040, 0xf00041).rw("watchdog", FUNC(watchdog_timer_device::reset16_r), FUNC(watchdog_timer_device::reset16_w));
	map(0xf00050, 0xf00051).noprw(); // ? written once (3rd opcode, $30.b)
}


void galpani3_state::galpani3(machine_config &config)
{
	M68000(config, m_maincpu, XTAL(28'636'363)/2); // Confirmed from PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &galpani3_state::galpani3_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(galpani3_state::galpani3_vblank), "screen", 0, 1);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 64*8);
	screen.set_visarea(0*8, 40*8-1, 0*8, 30*8-1);
	//screen.set_visarea(0*8, 64*8-1, 0*8, 64*8-1);
	screen.set_screen_update(FUNC(galpani3_state::screen_update_galpani3));

	EEPROM_93C46_16BIT(config, "eeprom");

	WATCHDOG_TIMER(config, "watchdog");

	KANEKO_TOYBOX(config, "toybox", "eeprom", "DSW1", "mcuram", "mcudata");

	PALETTE(config, m_palette).set_format(palette_device::xGRB_555, 0x4000);

	SKNS_SPRITE(config, m_spritegen, 0);

	KANEKO_GRAP2(config, m_grap2[0], 0).set_device_rom_tag("rlebg");

	KANEKO_GRAP2(config, m_grap2[1], 0).set_device_rom_tag("rlebg");

	KANEKO_GRAP2(config, m_grap2[2], 0).set_device_rom_tag("rlebg");

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ymz280b_device &ymz(YMZ280B(config, "ymz", XTAL(33'333'000) / 2));  // Confirmed from PCB
	ymz.add_route(0, "mono", 1.0);
	ymz.add_route(1, "mono", 1.0);
}


ROM_START( galpani3 ) /* All game text in English */
	ROM_REGION( 0x180000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "g3p0e0.u71",  0x000000, 0x080000, CRC(fa681118) SHA1(982b568a77ed620ba5708fec4c186d329d48cb48) )
	ROM_LOAD16_BYTE( "g3p1e0.u102", 0x000001, 0x080000, CRC(f1150f1b) SHA1(a6fb719937927a9a39c7a4888017c63c47c2dd6c) )

	ROM_REGION( 0x200000, "spritegen", 0 ) /* Sprites - RLE encoded */
	ROM_LOAD( "gp320000.1", 0x000000, 0x200000, CRC(a0112827) SHA1(0a6c78d71b75a1d78215aab3104176aa1769b14f) )

	ROM_REGION( 0x1000000, "rlebg", 0 ) /* Backgrounds - RLE encoded */
	ROM_LOAD( "gp340000.123", 0x000000, 0x200000, CRC(a58a26b1) SHA1(832d70cce1b4f04fa50fc221962ff6cc4287cb92) )        // 19950414GROMACap
	ROM_LOAD( "gp340100.122", 0x200000, 0x200000, CRC(746fe4a8) SHA1(a5126ae9e83d556277d31b166296a708c311a902) )        // 19950414GROMBCap
	ROM_LOAD( "gp340200.121", 0x400000, 0x200000, CRC(e9bc15c8) SHA1(2c6a10e768709d1937d9206970553f4101ce9016) )        // 19950414GROMCCap
	ROM_LOAD( "gp340300.120", 0x600000, 0x200000, CRC(59062eef) SHA1(936977c20d83540c1e0f65d429c7ebea201ef991) )        // 19950414GROMDCap
	ROM_LOAD16_BYTE( "g3g0j0.101", 0xe00000, 0x040000, CRC(fbb1e0dc) SHA1(14f6377afd93054aa5dc38af235ae12b932e847f) )   // 19950523GROMECap
	ROM_LOAD16_BYTE( "g3g1j0.100", 0xe00001, 0x040000, CRC(18edb5f0) SHA1(5e2ed0105b3e6037f6116494d3b186a368824171) )   //

	ROM_REGION( 0x300000, "ymz", 0 ) /* Samples */
	ROM_LOAD( "gp310100.40", 0x000000, 0x200000, CRC(6a0b1d12) SHA1(11fed80b96d07fddb27599743991c58c12c048e0) )
	ROM_LOAD( "gp310000.41", 0x200000, 0x100000, CRC(641062ef) SHA1(c8902fc46319eac94b3f95d18afa24bd895078d6) )

	ROM_REGION( 0x20000, "mcudata", 0 ) /* MCU Code? */
	ROM_LOAD16_WORD_SWAP( "g3d0x0.134", 0x000000, 0x020000, CRC(4ace10f9) SHA1(d19e4540d535ce10d23cb0844be03a3239b3402e) )
ROM_END

ROM_START( galpani3hk )
	ROM_REGION( 0x180000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "gp3_hk.u71",  0x000000, 0x080000, CRC(b8fc7826) SHA1(9ce97f2bb6af6a3aa19d2a7d4c159e3c33f43f63) )
	ROM_LOAD16_BYTE( "gp3_hk.u102", 0x000001, 0x080000, CRC(658f5fe8) SHA1(09c52d7676ccf31a7696596279cb07564ae018b3) )

	ROM_REGION( 0x200000, "spritegen", 0 ) /* Sprites - RLE encoded */
	ROM_LOAD( "gp320000.1", 0x000000, 0x200000, CRC(a0112827) SHA1(0a6c78d71b75a1d78215aab3104176aa1769b14f) )

	ROM_REGION( 0x1000000, "rlebg", 0 ) /* Backgrounds - RLE encoded */
	ROM_LOAD( "gp340000.123", 0x000000, 0x200000, CRC(a58a26b1) SHA1(832d70cce1b4f04fa50fc221962ff6cc4287cb92) )        // 19950414GROMACap
	ROM_LOAD( "gp340100.122", 0x200000, 0x200000, CRC(746fe4a8) SHA1(a5126ae9e83d556277d31b166296a708c311a902) )        // 19950414GROMBCap
	ROM_LOAD( "gp340200.121", 0x400000, 0x200000, CRC(e9bc15c8) SHA1(2c6a10e768709d1937d9206970553f4101ce9016) )        // 19950414GROMCCap
	ROM_LOAD( "gp340300.120", 0x600000, 0x200000, CRC(59062eef) SHA1(936977c20d83540c1e0f65d429c7ebea201ef991) )        // 19950414GROMDCap
	ROM_LOAD16_BYTE( "g3g0h0.101", 0xe00000, 0x040000, CRC(dca3109a) SHA1(d7741e992ffc9f8f57ce6770bf4bcb8d0858d72b) )   // 19950523GROMECap
	ROM_LOAD16_BYTE( "g3g1h0.100", 0xe00001, 0x040000, CRC(2ebe6ed0) SHA1(72d487c7f6339d7a39b04e95e76d0c4f3e432240) )   //

	ROM_REGION( 0x300000, "ymz", 0 ) /* Samples */
	ROM_LOAD( "gp310100.40", 0x000000, 0x200000, CRC(6a0b1d12) SHA1(11fed80b96d07fddb27599743991c58c12c048e0) )
	ROM_LOAD( "gp310000.41", 0x200000, 0x100000, CRC(641062ef) SHA1(c8902fc46319eac94b3f95d18afa24bd895078d6) )

	ROM_REGION( 0x20000, "mcudata", 0 ) /* MCU Code? */
	ROM_LOAD16_WORD_SWAP( "g3d0x0.134", 0x000000, 0x020000, CRC(4ace10f9) SHA1(d19e4540d535ce10d23cb0844be03a3239b3402e) )
ROM_END

ROM_START( galpani3j ) /* Some game text in Japanese, but no "For use in Japan" type region notice */
	ROM_REGION( 0x180000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "g3p0j1.71",  0x000000, 0x080000, CRC(52893326) SHA1(78fdbf3436a4ba754d7608fedbbede5c719a4505) )
	ROM_LOAD16_BYTE( "g3p1j1.102", 0x000001, 0x080000, CRC(05f935b4) SHA1(81e78875585bcdadad1c302614b2708e60563662) )

	ROM_REGION( 0x200000, "spritegen", 0 ) /* Sprites - RLE encoded */
	ROM_LOAD( "gp320000.1", 0x000000, 0x200000, CRC(a0112827) SHA1(0a6c78d71b75a1d78215aab3104176aa1769b14f) )

	ROM_REGION( 0x1000000, "rlebg", 0 ) /* Backgrounds - RLE encoded */
	ROM_LOAD( "gp340000.123", 0x000000, 0x200000, CRC(a58a26b1) SHA1(832d70cce1b4f04fa50fc221962ff6cc4287cb92) )        // 19950414GROMACap
	ROM_LOAD( "gp340100.122", 0x200000, 0x200000, CRC(746fe4a8) SHA1(a5126ae9e83d556277d31b166296a708c311a902) )        // 19950414GROMBCap
	ROM_LOAD( "gp340200.121", 0x400000, 0x200000, CRC(e9bc15c8) SHA1(2c6a10e768709d1937d9206970553f4101ce9016) )        // 19950414GROMCCap
	ROM_LOAD( "gp340300.120", 0x600000, 0x200000, CRC(59062eef) SHA1(936977c20d83540c1e0f65d429c7ebea201ef991) )        // 19950414GROMDCap
	ROM_LOAD16_BYTE( "g3g0j0.101", 0xe00000, 0x040000, CRC(fbb1e0dc) SHA1(14f6377afd93054aa5dc38af235ae12b932e847f) )   // 19950523GROMECap
	ROM_LOAD16_BYTE( "g3g1j0.100", 0xe00001, 0x040000, CRC(18edb5f0) SHA1(5e2ed0105b3e6037f6116494d3b186a368824171) )   //

	ROM_REGION( 0x300000, "ymz", 0 ) /* Samples */
	ROM_LOAD( "gp310100.40", 0x000000, 0x200000, CRC(6a0b1d12) SHA1(11fed80b96d07fddb27599743991c58c12c048e0) )
	ROM_LOAD( "gp310000.41", 0x200000, 0x100000, CRC(641062ef) SHA1(c8902fc46319eac94b3f95d18afa24bd895078d6) )

	ROM_REGION( 0x20000, "mcudata", 0 ) /* MCU Code? */
	ROM_LOAD16_WORD_SWAP( "g3d0x0.134", 0x000000, 0x020000, CRC(4ace10f9) SHA1(d19e4540d535ce10d23cb0844be03a3239b3402e) )
ROM_END

ROM_START( galpani3k ) /* Some game text in Korean, but no "For use in Korea" type region notice */
	ROM_REGION( 0x180000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "g3p0k0.71",  0x000000, 0x080000, CRC(98147760) SHA1(6db614e1af4e163488ab9675d96db829f45cec22) )
	ROM_LOAD16_BYTE( "g3p1k0.102", 0x000001, 0x080000, CRC(27416b22) SHA1(dbb3ec78cf70fd9a56e4f51c1c2b65feabc14190) )

	ROM_REGION( 0x200000, "spritegen", 0 ) /* Sprites - RLE encoded */
	ROM_LOAD( "gp320000.1", 0x000000, 0x200000, CRC(a0112827) SHA1(0a6c78d71b75a1d78215aab3104176aa1769b14f) )

	ROM_REGION( 0x1000000, "rlebg", 0 ) /* Backgrounds - RLE encoded */
	ROM_LOAD( "gp340000.123", 0x000000, 0x200000, CRC(a58a26b1) SHA1(832d70cce1b4f04fa50fc221962ff6cc4287cb92) )        // 19950414GROMACap
	ROM_LOAD( "gp340100.122", 0x200000, 0x200000, CRC(746fe4a8) SHA1(a5126ae9e83d556277d31b166296a708c311a902) )        // 19950414GROMBCap
	ROM_LOAD( "gp340200.121", 0x400000, 0x200000, CRC(e9bc15c8) SHA1(2c6a10e768709d1937d9206970553f4101ce9016) )        // 19950414GROMCCap
	ROM_LOAD( "gp340300.120", 0x600000, 0x200000, CRC(59062eef) SHA1(936977c20d83540c1e0f65d429c7ebea201ef991) )        // 19950414GROMDCap
	ROM_LOAD16_BYTE( "g3g0k0.101", 0xe00000, 0x080000, CRC(23d895b0) SHA1(621cc1500e26c3fe4410eefadd325891e7806f85) )   // 19950523GROMECap
	ROM_LOAD16_BYTE( "g3g1k0.100", 0xe00001, 0x080000, CRC(9b1eac6d) SHA1(1393d42a7ad70af90fa0f48fb8da7e2f9085f98f) )   //

	ROM_REGION( 0x300000, "ymz", 0 ) /* Samples */
	ROM_LOAD( "gp310100.40", 0x000000, 0x200000, CRC(6a0b1d12) SHA1(11fed80b96d07fddb27599743991c58c12c048e0) )
	ROM_LOAD( "gp310000.41", 0x200000, 0x100000, CRC(641062ef) SHA1(c8902fc46319eac94b3f95d18afa24bd895078d6) )

	ROM_REGION( 0x20000, "mcudata", 0 ) /* MCU Code? */
	ROM_LOAD16_WORD_SWAP( "g3d0x0.134", 0x000000, 0x020000, CRC(4ace10f9) SHA1(d19e4540d535ce10d23cb0844be03a3239b3402e) )
ROM_END

} // anonymous namespace


GAME( 1995, galpani3,  0,        galpani3, galpani3, galpani3_state, empty_init, ROT90, "Kaneko", "Gals Panic 3 (Euro)",      MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1995, galpani3j, galpani3, galpani3, galpani3, galpani3_state, empty_init, ROT90, "Kaneko", "Gals Panic 3 (Japan)",     MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1995, galpani3k, galpani3, galpani3, galpani3, galpani3_state, empty_init, ROT90, "Kaneko", "Gals Panic 3 (Korea)",     MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1995, galpani3hk,galpani3, galpani3, galpani3, galpani3_state, empty_init, ROT90, "Kaneko", "Gals Panic 3 (Hong Kong)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
