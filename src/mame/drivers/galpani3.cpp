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
#include "sound/ymz280b.h"
#include "video/sknsspr.h"
#include "machine/eepromser.h"
#include "machine/kaneko_toybox.h"
#include "video/kaneko_grap2.h"

class galpani3_state : public driver_device
{
public:
	galpani3_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_grap2_0(*this,"grap2_0"),
		m_grap2_1(*this,"grap2_1"),
		m_grap2_2(*this,"grap2_2"),
		m_palette(*this, "palette"),
		m_spritegen(*this, "spritegen"),
		m_paletteram(*this, "palette"),
		m_spriteram(*this, "spriteram"),
		m_priority_buffer(*this, "priority_buffer"),
		m_sprregs(*this, "sprregs"),
		m_sprite_bitmap_1(1024, 1024)

	{ }

	required_device<cpu_device> m_maincpu;
	required_device<kaneko_grap2_device> m_grap2_0;
	required_device<kaneko_grap2_device> m_grap2_1;
	required_device<kaneko_grap2_device> m_grap2_2;
	required_device<palette_device> m_palette;
	required_device<sknsspr_device> m_spritegen;

	required_shared_ptr<UINT16> m_paletteram;
	optional_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_priority_buffer;
	required_shared_ptr<UINT16> m_sprregs;

	bitmap_ind16 m_sprite_bitmap_1;
	UINT16 m_priority_buffer_scrollx;
	UINT16 m_priority_buffer_scrolly;
	UINT32 m_spriteram32[0x4000/4];
	UINT32 m_spc_regs[0x40/4];

	DECLARE_WRITE16_MEMBER(galpani3_suprnova_sprite32_w);
	DECLARE_WRITE16_MEMBER(galpani3_suprnova_sprite32regs_w);
	DECLARE_WRITE16_MEMBER(galpani3_priority_buffer_scrollx_w);
	DECLARE_WRITE16_MEMBER(galpani3_priority_buffer_scrolly_w);


	virtual void video_start() override;

	UINT32 screen_update_galpani3(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(galpani3_vblank);
	int gp3_is_alpha_pen(int pen);
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

	save_item(NAME(m_priority_buffer_scrollx));
	save_item(NAME(m_priority_buffer_scrolly));
	save_item(NAME(m_spriteram32));
	save_item(NAME(m_spc_regs));
}



int galpani3_state::gp3_is_alpha_pen(int pen)
{
	UINT16 dat = 0;

	if (pen<0x4000)
	{
		dat = m_paletteram[pen];
	}
	else if (pen<0x4100)
	{
		dat = m_grap2_0->m_framebuffer_palette[pen&0xff];
	}
	else if (pen<0x4200)
	{
		dat = m_grap2_1->m_framebuffer_palette[pen&0xff];
	}
	else if (pen<0x4300)
	{
		dat = m_grap2_2->m_framebuffer_palette[pen&0xff];
	}
	else if (pen<0x4301)
	{
		dat = m_grap2_0->m_framebuffer_bgcol;
	}
	else if (pen<0x4302)
	{
		dat = m_grap2_1->m_framebuffer_bgcol;
	}
	else if (pen<0x4303)
	{
		dat = m_grap2_2->m_framebuffer_bgcol;
	}

	if (dat&0x8000) return 1;
	else return 0;
}


UINT32 galpani3_state::screen_update_galpani3(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int x,y;
	UINT16* src1;
	UINT32* dst;
	UINT16 pixdata1;
	const pen_t *paldata = m_palette->pens();

	bitmap.fill(0x0000, cliprect);

//  popmessage("%02x %02x", m_grap2_0->m_framebuffer_bright2, m_grap2_1->m_framebuffer_bright2);



	{
		int drawy, drawx;
		for (drawy=0;drawy<512;drawy++)
		{
			UINT16* srcline1 = m_grap2_0->m_framebuffer.get() + ((drawy+m_grap2_0->m_framebuffer_scrolly+11)&0x1ff) * 0x200;
			UINT16* srcline2 = m_grap2_1->m_framebuffer.get() + ((drawy+m_grap2_1->m_framebuffer_scrolly+11)&0x1ff) * 0x200;
			UINT16* srcline3 = m_grap2_2->m_framebuffer.get() + ((drawy+m_grap2_2->m_framebuffer_scrolly+11)&0x1ff) * 0x200;

			UINT16*  priline  = m_priority_buffer + ((drawy+m_priority_buffer_scrolly+11)&0x1ff) * 0x200;

			for (drawx=0;drawx<512;drawx++)
			{
				int srcoffs1 = (drawx+m_grap2_0->m_framebuffer_scrollx+67)&0x1ff;
				int srcoffs2 = (drawx+m_grap2_1->m_framebuffer_scrollx+67)&0x1ff;
				int srcoffs3 = (drawx+m_grap2_2->m_framebuffer_scrollx+67)&0x1ff;

				int prioffs  = (drawx+m_priority_buffer_scrollx+66)&0x1ff;

				UINT8 dat1 = srcline1[srcoffs1];
				UINT8 dat2 = srcline2[srcoffs2];
				UINT8 dat3 = srcline3[srcoffs3];

				UINT8 pridat = priline[prioffs];

				UINT32* dst = &bitmap.pix32(drawy, drawx);



				// this is all wrong
				if (pridat==0x0f) // relates to the area you've drawn over
				{
					if (dat1 && m_grap2_0->m_framebuffer_enable)
					{
						dst[0] = paldata[dat1+0x4000];
					}

					if (dat2 && m_grap2_1->m_framebuffer_enable)
					{
						dst[0] = paldata[dat2+0x4100];
					}

				}
				else if (pridat==0xcf) // the girl
				{
					dst[0] = paldata[0x4300];
				}
				else
				{
					/* this isn't right, but the registers have something to do with
					   alpha / mixing, and bit 0x8000 of the palette is DEFINITELY alpha
					   enable -- see fading in intro */
					if (dat1 && m_grap2_0->m_framebuffer_enable)
					{
						UINT16 pen = dat1+0x4000;
						UINT32 pal = paldata[pen];

						if (gp3_is_alpha_pen(pen))
						{
							int r,g,b;
							r = (pal & 0x00ff0000)>>16;
							g = (pal & 0x0000ff00)>>8;
							b = (pal & 0x000000ff)>>0;

							r = (r * m_grap2_0->m_framebuffer_bright2) / 0xff;
							g = (g * m_grap2_0->m_framebuffer_bright2) / 0xff;
							b = (b * m_grap2_0->m_framebuffer_bright2) / 0xff;

							pal = (r & 0x000000ff)<<16;
							pal |=(g & 0x000000ff)<<8;
							pal |=(b & 0x000000ff)<<0;

							dst[0] = pal;
						}
						else
						{
							dst[0] = pal;
						}
					}

					if (dat2 && m_grap2_1->m_framebuffer_enable)
					{
						UINT16 pen = dat2+0x4100;
						UINT32 pal = paldata[pen];

						if (gp3_is_alpha_pen(pen))
						{
							int r,g,b;
							r = (pal & 0x00ff0000)>>16;
							g = (pal & 0x0000ff00)>>8;
							b = (pal & 0x000000ff)>>0;

							r = (r * m_grap2_1->m_framebuffer_bright2) / 0xff;
							g = (g * m_grap2_1->m_framebuffer_bright2) / 0xff;
							b = (b * m_grap2_1->m_framebuffer_bright2) / 0xff;

							pal = (r & 0x000000ff)<<16;
							pal |=(g & 0x000000ff)<<8;
							pal |=(b & 0x000000ff)<<0;

							dst[0] |= pal;
						}
						else
						{
							dst[0] = pal;
						}
					}

					if (dat3 && m_grap2_2->m_framebuffer_enable)
					{
						dst[0] = paldata[dat3+0x4200];
					}
				}

				/*
				else if (pridat==0x2f) // area outside of the girl
				{
				    //dst[0] = machine().rand()&0x3fff;
				}

				else if (pridat==0x00) // the initial line / box that gets drawn
				{
				    //dst[0] = machine().rand()&0x3fff;
				}
				else if (pridat==0x30) // during the 'gals boxes' on the intro
				{
				    //dst[0] = machine().rand()&0x3fff;
				}
				else if (pridat==0x0c) // 'nice' at end of level
				{
				    //dst[0] = machine().rand()&0x3fff;
				}
				else
				{
				    //printf("%02x, ",pridat);
				}
				*/
			}
		}
	}


	m_sprite_bitmap_1.fill(0x0000, cliprect);

	m_spritegen->skns_draw_sprites(m_sprite_bitmap_1, cliprect, &m_spriteram32[0], 0x4000, memregion("gfx1")->base(), memregion ("gfx1")->bytes(), m_spc_regs );

	// ignoring priority bits for now..
	for (y=0;y<240;y++)
	{
		src1 = &m_sprite_bitmap_1.pix16(y);
		dst =  &bitmap.pix32(y);

		for (x=0;x<320;x++)
		{
			pixdata1 = src1[x];

			if (pixdata1 & 0x3fff)
			{
				dst[x] = paldata[(pixdata1 & 0x3fff)];
			}
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


WRITE16_MEMBER(galpani3_state::galpani3_suprnova_sprite32_w)
{
	COMBINE_DATA(&m_spriteram[offset]);
	offset>>=1;
	m_spriteram32[offset]=(m_spriteram[offset*2+1]<<16) | (m_spriteram[offset*2]);
}

WRITE16_MEMBER(galpani3_state::galpani3_suprnova_sprite32regs_w)
{
	COMBINE_DATA(&m_sprregs[offset]);
	offset>>=1;
	m_spc_regs[offset]=(m_sprregs[offset*2+1]<<16) | (m_sprregs[offset*2]);
}

WRITE16_MEMBER(galpani3_state::galpani3_priority_buffer_scrollx_w)
{
	m_priority_buffer_scrollx = data;
}

WRITE16_MEMBER(galpani3_state::galpani3_priority_buffer_scrolly_w)
{
	m_priority_buffer_scrolly = data;
}







static ADDRESS_MAP_START( galpani3_map, AS_PROGRAM, 16, galpani3_state )
	AM_RANGE(0x000000, 0x17ffff) AM_ROM

	AM_RANGE(0x200000, 0x20ffff) AM_RAM // area [B] - Work RAM
	AM_RANGE(0x280000, 0x287fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette") // area [A] - palette for sprites

	AM_RANGE(0x300000, 0x303fff) AM_RAM_WRITE(galpani3_suprnova_sprite32_w) AM_SHARE("spriteram")
	AM_RANGE(0x380000, 0x38003f) AM_RAM_WRITE(galpani3_suprnova_sprite32regs_w) AM_SHARE("sprregs")

	AM_RANGE(0x400000, 0x40ffff) AM_RAM AM_SHARE("mcuram") // area [C]

	AM_RANGE(0x580000, 0x580001) AM_DEVWRITE( "toybox", kaneko_toybox_device, mcu_com0_w)
	AM_RANGE(0x600000, 0x600001) AM_DEVWRITE( "toybox", kaneko_toybox_device, mcu_com1_w)
	AM_RANGE(0x680000, 0x680001) AM_DEVWRITE( "toybox", kaneko_toybox_device, mcu_com2_w)
	AM_RANGE(0x700000, 0x700001) AM_DEVWRITE( "toybox", kaneko_toybox_device, mcu_com3_w)
	AM_RANGE(0x780000, 0x780001) AM_DEVREAD( "toybox", kaneko_toybox_device, mcu_status_r)

	GRAP2_AREA( 0x800000, "grap2_0" )
	GRAP2_AREA( 0xa00000, "grap2_1" )
	GRAP2_AREA( 0xc00000, "grap2_2" )

	// ?? priority / alpha buffer?
	AM_RANGE(0xe00000, 0xe7ffff) AM_RAM AM_SHARE("priority_buffer") // area [J] - A area ? odd bytes only, initialized 00..ff,00..ff,..., then cleared
	AM_RANGE(0xe80000, 0xe80001) AM_WRITE(galpani3_priority_buffer_scrollx_w) // scroll?
	AM_RANGE(0xe80002, 0xe80003) AM_WRITE(galpani3_priority_buffer_scrolly_w) // scroll?


	AM_RANGE(0xf00000, 0xf00001) AM_NOP // ? written once (2nd opcode, $1.b)
	AM_RANGE(0xf00010, 0xf00011) AM_READ_PORT("P1")
	AM_RANGE(0xf00012, 0xf00013) AM_READ_PORT("P2")
	AM_RANGE(0xf00014, 0xf00015) AM_READ_PORT("COIN")
	AM_RANGE(0xf00016, 0xf00017) AM_NOP // ? read, but overwritten
	AM_RANGE(0xf00020, 0xf00023) AM_DEVWRITE8("ymz", ymz280b_device, write, 0x00ff)     // sound
	AM_RANGE(0xf00040, 0xf00041) AM_READWRITE(watchdog_reset16_r, watchdog_reset16_w)   // watchdog
	AM_RANGE(0xf00050, 0xf00051) AM_NOP // ? written once (3rd opcode, $30.b)
ADDRESS_MAP_END


static MACHINE_CONFIG_START( galpani3, galpani3_state )
	MCFG_CPU_ADD("maincpu", M68000, XTAL_28_63636MHz/2) // Confirmed from PCB
	MCFG_CPU_PROGRAM_MAP(galpani3_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", galpani3_state, galpani3_vblank, "screen", 0, 1)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 30*8-1)
	//MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 64*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(galpani3_state, screen_update_galpani3)

	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")

	MCFG_DEVICE_ADD("toybox", KANEKO_TOYBOX, 0)

	MCFG_PALETTE_ADD("palette", 0x4303)
	MCFG_PALETTE_FORMAT(xGGGGGRRRRRBBBBB)

	MCFG_DEVICE_ADD("spritegen", SKNS_SPRITE, 0)

	MCFG_DEVICE_ADD("grap2_0", KANEKO_GRAP2, 0)
	kaneko_grap2_device::set_chipnum(*device, 0);
	MCFG_KANEKO_GRAP2_PALETTE("palette")

	MCFG_DEVICE_ADD("grap2_1", KANEKO_GRAP2, 0)
	kaneko_grap2_device::set_chipnum(*device, 1);
	MCFG_KANEKO_GRAP2_PALETTE("palette")

	MCFG_DEVICE_ADD("grap2_2", KANEKO_GRAP2, 0)
	kaneko_grap2_device::set_chipnum(*device, 2);
	MCFG_KANEKO_GRAP2_PALETTE("palette")


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymz", YMZ280B, XTAL_33_333MHz / 2)  // Confirmed from PCB
	MCFG_SOUND_ROUTE(0, "mono", 1.0)
	MCFG_SOUND_ROUTE(1, "mono", 1.0)
MACHINE_CONFIG_END


ROM_START( galpani3 ) /* All game text in English */
	ROM_REGION( 0x180000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "g3p0e0.u71",  0x000000, 0x080000, CRC(fa681118) SHA1(982b568a77ed620ba5708fec4c186d329d48cb48) )
	ROM_LOAD16_BYTE( "g3p1e0.u102", 0x000001, 0x080000, CRC(f1150f1b) SHA1(a6fb719937927a9a39c7a4888017c63c47c2dd6c) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* Sprites - RLE encoded */
	ROM_LOAD( "gp320000.1", 0x000000, 0x200000, CRC(a0112827) SHA1(0a6c78d71b75a1d78215aab3104176aa1769b14f) )

	ROM_REGION( 0x1000000, "gfx2", 0 ) /* Backgrounds - RLE encoded */
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

	ROM_REGION( 0x200000, "gfx1", 0 ) /* Sprites - RLE encoded */
	ROM_LOAD( "gp320000.1", 0x000000, 0x200000, CRC(a0112827) SHA1(0a6c78d71b75a1d78215aab3104176aa1769b14f) )

	ROM_REGION( 0x1000000, "gfx2", 0 ) /* Backgrounds - RLE encoded */
	ROM_LOAD( "gp340000.123", 0x000000, 0x200000, CRC(a58a26b1) SHA1(832d70cce1b4f04fa50fc221962ff6cc4287cb92) )        // 19950414GROMACap
	ROM_LOAD( "gp340100.122", 0x200000, 0x200000, CRC(746fe4a8) SHA1(a5126ae9e83d556277d31b166296a708c311a902) )        // 19950414GROMBCap
	ROM_LOAD( "gp340200.121", 0x400000, 0x200000, CRC(e9bc15c8) SHA1(2c6a10e768709d1937d9206970553f4101ce9016) )        // 19950414GROMCCap
	ROM_LOAD( "gp340300.120", 0x600000, 0x200000, CRC(59062eef) SHA1(936977c20d83540c1e0f65d429c7ebea201ef991) )        // 19950414GROMDCap
	// I'm guessing these are the same as the Korea set, because the regular ones don't give correct gfx, but it should be checked
	ROM_LOAD16_BYTE( "g3g0k0.101", 0xe00000, 0x080000, CRC(23d895b0) SHA1(621cc1500e26c3fe4410eefadd325891e7806f85) )   // 19950523GROMECap
	ROM_LOAD16_BYTE( "g3g1k0.100", 0xe00001, 0x080000, CRC(9b1eac6d) SHA1(1393d42a7ad70af90fa0f48fb8da7e2f9085f98f) )   //

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

	ROM_REGION( 0x200000, "gfx1", 0 ) /* Sprites - RLE encoded */
	ROM_LOAD( "gp320000.1", 0x000000, 0x200000, CRC(a0112827) SHA1(0a6c78d71b75a1d78215aab3104176aa1769b14f) )

	ROM_REGION( 0x1000000, "gfx2", 0 ) /* Backgrounds - RLE encoded */
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

	ROM_REGION( 0x200000, "gfx1", 0 ) /* Sprites - RLE encoded */
	ROM_LOAD( "gp320000.1", 0x000000, 0x200000, CRC(a0112827) SHA1(0a6c78d71b75a1d78215aab3104176aa1769b14f) )

	ROM_REGION( 0x1000000, "gfx2", 0 ) /* Backgrounds - RLE encoded */
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




GAME( 1995, galpani3,  0,        galpani3, galpani3, driver_device, 0, ROT90, "Kaneko", "Gals Panic 3 (Euro)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1995, galpani3j, galpani3, galpani3, galpani3, driver_device, 0, ROT90, "Kaneko", "Gals Panic 3 (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1995, galpani3k, galpani3, galpani3, galpani3, driver_device, 0, ROT90, "Kaneko", "Gals Panic 3 (Korea)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1995, galpani3hk,galpani3, galpani3, galpani3, driver_device, 0, ROT90, "Kaneko", "Gals Panic 3 (Hong Kong)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
