// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Super Cross II (c) 1987 GM Shoji

    driver by Angelo Salese, based off "wiped off due of not anymore licenseable" driver by insideoutboy.

    TODO:
    - complete rewrite;
    - scanline renderer;
    - understand irq 0 source;
    - output bit 0 might be watchdog armed bit/sprite start DMA instead of irq enable;
    - weird visible area resolution, 224 or 240 x 224? Maybe it's really just 256 x 224 and then it's supposed
      to show garbage/nothing on the edges?

===================================

Super Cross II (JPN Ver.)
(c)1986 GM Shoji

C2-00172-D
CPU  :Z80B
Sound:SN76489 x3

SCS-24.4E
SCS-25.4C
SCS-26.4B
SCS-27.5K
SCS-28.5J
SCS-29.5H
SCS-30.5F

SC-62.3A
SC-63.3B
SC-64.6A

C2-00171-D
CPU  :Z80B
OSC  :10.000MHz

SCM-00.10L
SCM-01.10K
SCM-02.10J
SCM-03.10G
SCM-20.5K
SCM-21.5G
SCM-22.5E
SCM-23.5B

SC-60.4K
SC-61.5A

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/sn76496.h"

#define MAIN_CLOCK XTAL_10MHz

class sprcros2_state : public driver_device
{
public:
	sprcros2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_master_cpu(*this, "master_cpu"),
			m_slave_cpu(*this, "slave_cpu"),
			m_gfxdecode(*this, "gfxdecode"),
			m_fgvram(*this, "fgvram"),
			m_fgattr(*this, "fgattr"),
			m_bgvram(*this, "bgvram"),
			m_bgattr(*this, "bgattr"),
			m_sprram(*this, "sprram")
	{ }

	// devices
	required_device<cpu_device> m_master_cpu;
	required_device<cpu_device> m_slave_cpu;
	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr<UINT8> m_fgvram;
	required_shared_ptr<UINT8> m_fgattr;
	required_shared_ptr<UINT8> m_bgvram;
	required_shared_ptr<UINT8> m_bgattr;
	required_shared_ptr<UINT8> m_sprram;


	// screen updates
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_PALETTE_INIT(sprcros2);
	DECLARE_WRITE8_MEMBER(master_output_w);
	DECLARE_WRITE8_MEMBER(slave_output_w);
	DECLARE_WRITE8_MEMBER(bg_scrollx_w);
	DECLARE_WRITE8_MEMBER(bg_scrolly_w);
	INTERRUPT_GEN_MEMBER(master_vblank_irq);
	INTERRUPT_GEN_MEMBER(slave_vblank_irq);
	TIMER_DEVICE_CALLBACK_MEMBER(master_scanline);

	bool m_master_nmi_enable;
	bool m_master_irq_enable;
	bool m_slave_nmi_enable;
	bool m_screen_enable;
	UINT8 m_bg_scrollx, m_bg_scrolly;
protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;

private:
	void legacy_bg_draw(bitmap_ind16 &bitmap,const rectangle &cliprect);
	void legacy_fg_draw(bitmap_ind16 &bitmap,const rectangle &cliprect);
	void legacy_obj_draw(bitmap_ind16 &bitmap,const rectangle &cliprect);
};

void sprcros2_state::video_start()
{
}

void sprcros2_state::legacy_bg_draw(bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	gfx_element *gfx_0 = m_gfxdecode->gfx(0);
	int count = 0;

	for (int y=0;y<32;y++)
	{
		for (int x=0;x<32;x++)
		{
			UINT16 tile = m_bgvram[count];
			tile |= (m_bgattr[count] & 7) << 8;
			bool flipx = bool(m_bgattr[count] & 0x08);
			UINT8 color = (m_bgattr[count] & 0xf0) >> 4;

			gfx_0->opaque(bitmap,cliprect,tile,color,flipx,0,x*8-m_bg_scrollx,y*8-m_bg_scrolly);
			gfx_0->opaque(bitmap,cliprect,tile,color,flipx,0,x*8+256-m_bg_scrollx,y*8-m_bg_scrolly);
			gfx_0->opaque(bitmap,cliprect,tile,color,flipx,0,x*8-m_bg_scrollx,y*8+256-m_bg_scrolly);
			gfx_0->opaque(bitmap,cliprect,tile,color,flipx,0,x*8+256-m_bg_scrollx,y*8+256-m_bg_scrolly);

			count++;
		}
	}

}

void sprcros2_state::legacy_obj_draw(bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	gfx_element *gfx_1 = m_gfxdecode->gfx(1);

	for(int count=0x40-4;count>-1;count-=4)
	{
		UINT8 x,y,tile,color;
		bool flipx;

		y = 224-m_sprram[count+2];
		x = m_sprram[count+3];
		tile = m_sprram[count+0];
		flipx = bool(m_sprram[count+1] & 2);
		color = (m_sprram[count+1] & 0x38) >> 3;
		gfx_1->transpen(bitmap,cliprect,tile,color,flipx,0,x,y & 0xff,0);
	}
}

void sprcros2_state::legacy_fg_draw(bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	gfx_element *gfx_2 = m_gfxdecode->gfx(2);
	int count = 0;

	for (int y=0;y<32;y++)
	{
		for (int x=0;x<32;x++)
		{
			UINT16 tile = m_fgvram[count];
			tile |= (m_fgattr[count] & 3) << 8;
			UINT8 color = (m_fgattr[count] & 0xfc) >> 2;

			// TODO: was using tileinfo.group, which I don't know what's for at all.
			//       This guess seems as good as the original one for all I know.
			if((color & 0x30) != 0x30)
				gfx_2->opaque(bitmap,cliprect,tile,color,0,0,x*8,y*8);
			else
				gfx_2->transpen(bitmap,cliprect,tile,color,0,0,x*8,y*8,0);

			count++;
		}
	}
}




UINT32 sprcros2_state::screen_update( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	if(m_screen_enable == false)
	{
		bitmap.fill(0,cliprect);
		return 0;
	}
	legacy_bg_draw(bitmap,cliprect);
	legacy_obj_draw(bitmap,cliprect);
	legacy_fg_draw(bitmap,cliprect);
	return 0;
}

WRITE8_MEMBER(sprcros2_state::master_output_w)
{
	//popmessage("%02x",data);
	//if(data & 0xbe)
	//  printf("master 07 -> %02x\n",data);

	membank("master_rombank")->set_entry((data&0x40)>>6);
	m_master_nmi_enable = bool(data & 1);
	m_screen_enable = bool(data & 4);
	m_master_irq_enable = bool(data & 8);
//  if(data & 0x80)
//      m_master_cpu->set_input_line(0,HOLD_LINE);
}

WRITE8_MEMBER(sprcros2_state::slave_output_w)
{
	//if(data & 0xf6)
	//  printf("slave 03 -> %02x\n",data);

	m_slave_nmi_enable = bool(data & 1);
	membank("slave_rombank")->set_entry((data&8)>>3);
}

WRITE8_MEMBER(sprcros2_state::bg_scrollx_w)
{
	m_bg_scrollx = data;
}

WRITE8_MEMBER(sprcros2_state::bg_scrolly_w)
{
	m_bg_scrolly = data;
}

static ADDRESS_MAP_START( master_map, AS_PROGRAM, 8, sprcros2_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM AM_REGION("master", 0)
	AM_RANGE(0xc000, 0xdfff) AM_ROMBANK("master_rombank")
	AM_RANGE(0xe000, 0xe3ff) AM_RAM AM_SHARE("fgvram")
	AM_RANGE(0xe400, 0xe7ff) AM_RAM AM_SHARE("fgattr")
	AM_RANGE(0xe800, 0xe83f) AM_RAM AM_SHARE("sprram")
	AM_RANGE(0xe840, 0xf7ff) AM_RAM
	AM_RANGE(0xf800, 0xffff) AM_RAM AM_SHARE("shared_ram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( master_io, AS_IO, 8, sprcros2_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("P1") AM_DEVWRITE("sn1", sn76489_device, write)
	AM_RANGE(0x01, 0x01) AM_READ_PORT("P2") AM_DEVWRITE("sn2", sn76489_device, write)
	AM_RANGE(0x02, 0x02) AM_READ_PORT("EXTRA") AM_DEVWRITE("sn3", sn76489_device, write)
	AM_RANGE(0x04, 0x04) AM_READ_PORT("DSW1")
	AM_RANGE(0x05, 0x05) AM_READ_PORT("DSW2")
	AM_RANGE(0x07, 0x07) AM_WRITE(master_output_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( slave_map, AS_PROGRAM, 8, sprcros2_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM AM_REGION("slave", 0)
	AM_RANGE(0xc000, 0xdfff) AM_ROMBANK("slave_rombank")
	AM_RANGE(0xe000, 0xe3ff) AM_RAM AM_SHARE("bgvram")
	AM_RANGE(0xe400, 0xe7ff) AM_RAM AM_SHARE("bgattr")
	AM_RANGE(0xe800, 0xf7ff) AM_RAM
	AM_RANGE(0xf800, 0xffff) AM_RAM AM_SHARE("shared_ram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( slave_io, AS_IO, 8, sprcros2_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(bg_scrollx_w)
	AM_RANGE(0x01, 0x01) AM_WRITE(bg_scrolly_w)
	AM_RANGE(0x03, 0x03) AM_WRITE(slave_output_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( sprcros2 )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("EXTRA")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )
	PORT_BIT( 0xf8, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x70, IP_ACTIVE_HIGH, IPT_UNUSED )            //unused coinage bits
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout sprite_layout =
{
	32,32,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ STEP8(0,1), STEP8(256,1), STEP8(512,1), STEP8(768,1) },
	{ STEP16(0,8), STEP16(128,8) },
	32*32
};

static const gfx_layout fg_layout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ STEP4(64,1), STEP4(0,1) },
	{ STEP8(0,8) },
	8*8*2
};

static GFXDECODE_START( sprcros2 )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x3_planar, 0,   16 )
	GFXDECODE_ENTRY( "gfx2", 0, sprite_layout, 256, 32 )
	GFXDECODE_ENTRY( "gfx3", 0, fg_layout,     512, 64 )
GFXDECODE_END



void sprcros2_state::machine_start()
{
	membank("master_rombank")->configure_entries(0, 2, memregion("master_bank")->base(), 0x2000);
	membank("slave_rombank")->configure_entries(0, 2, memregion("slave_bank")->base(), 0x2000);

}

void sprcros2_state::machine_reset()
{
	m_master_nmi_enable = false;
	m_slave_nmi_enable = false;
}


PALETTE_INIT_MEMBER(sprcros2_state, sprcros2)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	/* create a lookup table for the palette */
	for (i = 0; i < 0x20; i++)
	{
		int bit0, bit1, bit2;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* blue component */
		bit0 = (color_prom[i] >> 6) & 0x01;
		bit1 = (color_prom[i] >> 7) & 0x01;
		b = 0x47 * bit0 + 0xb8 * bit1;
		palette.set_pen_color(i,rgb_t(r,g,b));

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x20;

	/* bg */
	for (i = 0; i < 0x100; i++)
	{
		UINT8 ctabentry = (color_prom[i] & 0x0f) | ((color_prom[i + 0x100] & 0x0f) << 4);
		palette.set_pen_indirect(i, ctabentry);
	}

	/* sprites & fg */
	for (i = 0x100; i < 0x300; i++)
	{
		UINT8 ctabentry = color_prom[i + 0x100];
		palette.set_pen_indirect(i, ctabentry);
	}
}

INTERRUPT_GEN_MEMBER(sprcros2_state::master_vblank_irq)
{
	if(m_master_nmi_enable == true)
		device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

INTERRUPT_GEN_MEMBER(sprcros2_state::slave_vblank_irq)
{
	if(m_slave_nmi_enable == true)
		device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

TIMER_DEVICE_CALLBACK_MEMBER(sprcros2_state::master_scanline)
{
	int scanline = param;


	if(scanline == 0 && m_master_irq_enable == true)
		m_master_cpu->set_input_line(0, HOLD_LINE);
}

static MACHINE_CONFIG_START( sprcros2, sprcros2_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("master_cpu",Z80,MAIN_CLOCK/4)
	MCFG_CPU_PROGRAM_MAP(master_map)
	MCFG_CPU_IO_MAP(master_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", sprcros2_state,  master_vblank_irq)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", sprcros2_state, master_scanline, "screen", 0, 1)

	MCFG_CPU_ADD("slave_cpu",Z80,MAIN_CLOCK/4)
	MCFG_CPU_PROGRAM_MAP(slave_map)
	MCFG_CPU_IO_MAP(slave_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", sprcros2_state,  slave_vblank_irq)

	MCFG_QUANTUM_PERFECT_CPU("master_cpu")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_UPDATE_DRIVER(sprcros2_state, screen_update)
	MCFG_SCREEN_RAW_PARAMS(MAIN_CLOCK/2, 343, 8, 256-8, 262, 16, 240) // TODO: Wrong screen parameters
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", sprcros2)

	MCFG_PALETTE_ADD("palette", 768)
	MCFG_PALETTE_INDIRECT_ENTRIES(32)
	MCFG_PALETTE_INIT_OWNER(sprcros2_state, sprcros2)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("sn1", SN76489, 10000000/4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_SOUND_ADD("sn2", SN76489, 10000000/4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_SOUND_ADD("sn3", SN76489, 10000000/4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


/***************************************************************************

  Machine driver(s)

***************************************************************************/



ROM_START( sprcros2 )
	ROM_REGION( 0x0c000, "master", 0 )
	ROM_LOAD( "scm-03.10g", 0x00000, 0x4000, CRC(b9757908) SHA1(d59cb2aac1b6268fc766306850f5711d4a12d897) )
	ROM_LOAD( "scm-02.10j", 0x04000, 0x4000, CRC(849c5c87) SHA1(0e02c4990e371d6a290efa53301818e769648945) )
	ROM_LOAD( "scm-01.10k", 0x08000, 0x4000, CRC(385a62de) SHA1(847bf9d97ab3fa8949d9198e4e509948a940d6aa) )

	ROM_REGION( 0x4000, "master_bank", 0)
	ROM_LOAD( "scm-00.10l", 0x00000, 0x4000, CRC(13fa3684) SHA1(611b7a237e394f285dcc5beb027dacdbdd58a7a0) )

	ROM_REGION( 0xc000, "slave", 0 )
	ROM_LOAD( "scs-30.5f",  0x00000, 0x4000, CRC(c0a40e41) SHA1(e74131b353855749258dffa45091c825ccdbf05a) )
	ROM_LOAD( "scs-29.5h",  0x04000, 0x4000, CRC(83d49fa5) SHA1(7112110df2f382bbc0e651adcec975054a485b9b) )
	ROM_LOAD( "scs-28.5j",  0x08000, 0x4000, CRC(480d351f) SHA1(d1b86f441ae0e58b30e0f089ab25de219d5f30e3) )

	ROM_REGION( 0x4000, "slave_bank", 0)
	ROM_LOAD( "scs-27.5k",  0x00000, 0x4000, CRC(2cf720cb) SHA1(a95c5b8c88371cf597bb7d80afeca6a48c7b74e6) )

	ROM_REGION( 0xc000, "gfx1", 0 ) //bg
	ROM_LOAD( "scs-26.4b",   0x8000, 0x4000, CRC(f958b56d) SHA1(a1973179d336d2ba57294155550515f2b8a33a09) )
	ROM_LOAD( "scs-25.4c",   0x4000, 0x4000, CRC(d6fd7ba5) SHA1(1c26c4c1655b2be9cb6103e75386cc2f0cf27fc5) )
	ROM_LOAD( "scs-24.4e",   0x0000, 0x4000, CRC(87783c36) SHA1(7102be795afcddd76b4d41823e95c65fe1ffbca0) )

	ROM_REGION( 0xc000, "gfx2", 0 )
	ROM_LOAD( "scm-23.5b",   0x0000, 0x4000, CRC(ab42f8e3) SHA1(8c2213b7c47a48e223fc3f7d323d16c0e4cd0457) )
	ROM_LOAD( "scm-22.5e",   0x4000, 0x4000, CRC(0cad254c) SHA1(36e30e30b652b3a388a3c4a82251196f79368f59) )
	ROM_LOAD( "scm-21.5g",   0x8000, 0x4000, CRC(b6b68998) SHA1(cc3c6d996beeedcc7e5199f10d65c5b1d3c6e666) )

	ROM_REGION( 0x4000, "gfx3", 0 ) //fg
	ROM_LOAD( "scm-20.5k",   0x0000, 0x4000, CRC(67a099a6) SHA1(43981abdcaa0ff36183027a3c691ce2df7f06ec7) )

	ROM_REGION( 0x0420, "proms", 0 )
	ROM_LOAD( "sc-64.6a",    0x0000, 0x0020, CRC(336dd1c0) SHA1(f0a0d2c13617fd84ee55c0cb96643761a8735147) ) //palette
	ROM_LOAD( "sc-63.3b",    0x0020, 0x0100, CRC(9034a059) SHA1(1801965b4f0f3e04ca4b3faf0ba3a27dbb008474) ) //bg clut lo nibble
	ROM_LOAD( "sc-62.3a",    0x0120, 0x0100, CRC(3c78a14f) SHA1(8f9c196a3e18bdce2d4855bc285bd5bde534bf09) ) //bg clut hi nibble
	ROM_LOAD( "sc-61.5a",    0x0220, 0x0100, CRC(2f71185d) SHA1(974fbb52285f01f4353e9acb1992dcd6fdefedcb) ) //sprite clut
	ROM_LOAD( "sc-60.4k",    0x0320, 0x0100, CRC(d7a4e57d) SHA1(6db02ec6aa55b05422cb505e63c71e36b4b11b4a) ) //fg clut
ROM_END

ROM_START( sprcros2a )
	ROM_REGION( 0xc000, "master", 0 )
	ROM_LOAD( "15.bin",     0x00000, 0x4000, CRC(b9d02558) SHA1(775404c6c7648d9dab02b496541739ea700cd481) )
	ROM_LOAD( "scm-02.10j", 0x04000, 0x4000, CRC(849c5c87) SHA1(0e02c4990e371d6a290efa53301818e769648945) )
	ROM_LOAD( "scm-01.10k", 0x08000, 0x4000, CRC(385a62de) SHA1(847bf9d97ab3fa8949d9198e4e509948a940d6aa) )

	ROM_REGION( 0x4000, "master_bank", 0)
	ROM_LOAD( "scm-00.10l", 0x00000, 0x4000, CRC(13fa3684) SHA1(611b7a237e394f285dcc5beb027dacdbdd58a7a0) )

	ROM_REGION( 0xc000, "slave", 0 )
	ROM_LOAD( "scs-30.5f",  0x00000, 0x4000, CRC(c0a40e41) SHA1(e74131b353855749258dffa45091c825ccdbf05a) )
	ROM_LOAD( "scs-29.5h",  0x04000, 0x4000, CRC(83d49fa5) SHA1(7112110df2f382bbc0e651adcec975054a485b9b) )
	ROM_LOAD( "scs-28.5j",  0x08000, 0x4000, CRC(480d351f) SHA1(d1b86f441ae0e58b30e0f089ab25de219d5f30e3) )

	ROM_REGION( 0x4000, "slave_bank", 0)
	ROM_LOAD( "scs-27.5k",  0x00000, 0x4000, CRC(2cf720cb) SHA1(a95c5b8c88371cf597bb7d80afeca6a48c7b74e6) )

	ROM_REGION( 0xc000, "gfx1", 0 ) //bg
	ROM_LOAD( "scs-26.4b",   0x8000, 0x4000, CRC(f958b56d) SHA1(a1973179d336d2ba57294155550515f2b8a33a09) )
	ROM_LOAD( "scs-25.4c",   0x4000, 0x4000, CRC(d6fd7ba5) SHA1(1c26c4c1655b2be9cb6103e75386cc2f0cf27fc5) )
	ROM_LOAD( "scs-24.4e",   0x0000, 0x4000, CRC(87783c36) SHA1(7102be795afcddd76b4d41823e95c65fe1ffbca0) )

	ROM_REGION( 0xc000, "gfx2", 0 )
	ROM_LOAD( "scm-23.5b",   0x0000, 0x4000, CRC(ab42f8e3) SHA1(8c2213b7c47a48e223fc3f7d323d16c0e4cd0457) )
	ROM_LOAD( "scm-22.5e",   0x4000, 0x4000, CRC(0cad254c) SHA1(36e30e30b652b3a388a3c4a82251196f79368f59) )
	ROM_LOAD( "scm-21.5g",   0x8000, 0x4000, CRC(b6b68998) SHA1(cc3c6d996beeedcc7e5199f10d65c5b1d3c6e666) )

	ROM_REGION( 0x4000, "gfx3", 0 ) //fg
	ROM_LOAD( "scm-20.5k",   0x0000, 0x4000, CRC(67a099a6) SHA1(43981abdcaa0ff36183027a3c691ce2df7f06ec7) )

	ROM_REGION( 0x0420, "proms", 0 )
	ROM_LOAD( "sc-64.6a",    0x0000, 0x0020, CRC(336dd1c0) SHA1(f0a0d2c13617fd84ee55c0cb96643761a8735147) ) //palette
	ROM_LOAD( "sc-63.3b",    0x0020, 0x0100, CRC(9034a059) SHA1(1801965b4f0f3e04ca4b3faf0ba3a27dbb008474) ) //bg clut lo nibble
	ROM_LOAD( "sc-62.3a",    0x0120, 0x0100, CRC(3c78a14f) SHA1(8f9c196a3e18bdce2d4855bc285bd5bde534bf09) ) //bg clut hi nibble
	ROM_LOAD( "sc-61.5a",    0x0220, 0x0100, CRC(2f71185d) SHA1(974fbb52285f01f4353e9acb1992dcd6fdefedcb) ) //sprite clut
	ROM_LOAD( "sc-60.4k",    0x0320, 0x0100, CRC(d7a4e57d) SHA1(6db02ec6aa55b05422cb505e63c71e36b4b11b4a) ) //fg clut
ROM_END

GAME( 1986, sprcros2, 0,        sprcros2, sprcros2, driver_device, 0, ROT0, "GM Shoji", "Super Cross II (Japan, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, sprcros2a,sprcros2, sprcros2, sprcros2, driver_device, 0, ROT0, "GM Shoji", "Super Cross II (Japan, set 2)", MACHINE_SUPPORTS_SAVE )
