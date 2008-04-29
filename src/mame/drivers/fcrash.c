/* Final Crash */

/*

Final Crash is a bootleg of Final Fight

Final Fight is by Capcom and runs on CPS1 hardware
The bootleg was manufactured by Playmark of Italy

this driver depends heavily on cps1.c, but has been
kept apart in an attempt to keep cps1.c clutter free

Sound is very different from CPS1.

---

Final Crash (bootleg of final fight)

1x 68k
1x z80
2x ym2203
2x oki5205
1x osc 10mhz
1x osc 24mhz

eproms:
1.bin sound eprom
from 2.bin to 9.bin program eproms
10.bin to 25.bin gfx eproms

---

*/

#include "driver.h"
#include "cpu/m68000/m68kmame.h"
#include "cps1.h"
#include "sound/2203intf.h"
#include "sound/msm5205.h"


static int sample_buffer1, sample_buffer2;
static int sample_select1, sample_select2;

static WRITE16_HANDLER( fcrash_soundlatch_w )
{
	if (ACCESSING_BITS_0_7)
	{
		soundlatch_w(machine,0,data & 0xff);
		cpunum_set_input_line(machine, 1, 0, HOLD_LINE);
	}
}

static WRITE8_HANDLER( fcrash_snd_bankswitch_w )
{
	UINT8 *RAM = memory_region(REGION_CPU2);
	int bankaddr;

	sndti_set_output_gain(SOUND_MSM5205, 0, 0, (data & 0x08) ? 0.0 : 1.0);
	sndti_set_output_gain(SOUND_MSM5205, 1, 0, (data & 0x10) ? 0.0 : 1.0);

	bankaddr = ((data & 7) * 0x4000);
	memory_set_bankptr(1,&RAM[0x10000 + bankaddr]);
}

static void m5205_int1(int data)
{
	MSM5205_data_w(0, sample_buffer1 & 0x0F);
	sample_buffer1 >>= 4;
	sample_select1 ^= 1;
	if (sample_select1 == 0)
		cpunum_set_input_line(Machine, 1, INPUT_LINE_NMI, PULSE_LINE);
}

static void m5205_int2(int data)
{
	MSM5205_data_w(1, sample_buffer2 & 0x0F);
	sample_buffer2 >>= 4;
	sample_select2 ^= 1;
}


static WRITE8_HANDLER( fcrash_msm5205_0_data_w )
{
    sample_buffer1 = data;
}

static WRITE8_HANDLER( fcrash_msm5205_1_data_w )
{
    sample_buffer2 = data;
}



/* not verified */
#define CPS1_ROWSCROLL_OFFS     (0x20/2)    /* base of row scroll offsets in other RAM */

static void fcrash_update_transmasks(void)
{
	int i;
	int priority[4];

	priority[0]=0x26;
	priority[1]=0x30;
	priority[2]=0x28;
	priority[3]=0x32;

	for (i = 0;i < 4;i++)
	{
		int mask;

		/* Get transparency registers */
		if (priority[i])
			mask = cps1_cps_b_regs[priority[i]/2] ^ 0xffff;
		else mask = 0xffff;	/* completely transparent if priority masks not defined (mercs, qad) */

		tilemap_set_transmask(cps1_bg_tilemap[0],i,mask,0x8000);
		tilemap_set_transmask(cps1_bg_tilemap[1],i,mask,0x8000);
		tilemap_set_transmask(cps1_bg_tilemap[2],i,mask,0x8000);
	}
}

static void fcrash_render_sprites(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect)
{
	int pos;
	int base=0x50c8/2; // and 10c8/2 for the buffer?

	for (pos=0x1ffc;pos>=0x0000;pos-=4)
	{
		int tileno;
		int xpos;
		int ypos;
		int flipx,flipy;
		int colour;

		tileno = cps1_gfxram[base+pos];
		xpos   = cps1_gfxram[base+pos+2];
		ypos   = cps1_gfxram[base+pos-1]&0xff;
		flipx  = cps1_gfxram[base+pos+1]&0x20;
		flipy  = cps1_gfxram[base+pos+1]&0x40;
		colour = cps1_gfxram[base+pos+1]&0x1f;
		ypos = 256-ypos;

		pdrawgfx(bitmap,machine->gfx[2],tileno,colour,flipx,flipy,xpos+49,ypos-16,cliprect,TRANSPARENCY_PEN,15,0x02);

	}

}

static void fcrash_render_layer(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect,int layer,int primask)
{
	switch (layer)
	{
		case 0:
			fcrash_render_sprites(machine,bitmap,cliprect);
			break;
		case 1:
		case 2:
		case 3:
			tilemap_draw(bitmap,cliprect,cps1_bg_tilemap[layer-1],TILEMAP_DRAW_LAYER1,primask);
			break;
	}
}

static void fcrash_render_high_layer(bitmap_t *bitmap, const rectangle *cliprect, int layer)
{
	switch (layer)
	{
		case 0:
			/* there are no high priority sprites */
			break;
		case 1:
		case 2:
		case 3:
			tilemap_draw(NULL,cliprect,cps1_bg_tilemap[layer-1],TILEMAP_DRAW_LAYER0,1);
			break;
	}
}

static void fcrash_build_palette(running_machine *machine)
{
	int offset;

	for (offset = 0; offset < 32*6*16; offset++)
	{
		int palette = cps1_gfxram[0x14000/2 + offset];
		int r, g, b, bright;

		// from my understanding of the schematics, when the 'brightness'
		// component is set to 0 it should reduce brightness to 1/3

		bright = 0x0f + ((palette>>12)<<1);

		r = ((palette>>8)&0x0f) * 0x11 * bright / 0x2d;
		g = ((palette>>4)&0x0f) * 0x11 * bright / 0x2d;
		b = ((palette>>0)&0x0f) * 0x11 * bright / 0x2d;

		palette_set_color (machine, offset, MAKE_RGB(r, g, b));
	}
}

static VIDEO_UPDATE( fcrash )
{
	int layercontrol,l0,l1,l2,l3;
	int videocontrol=cps1_cps_a_regs[0x22/2];


	flip_screen_set(videocontrol & 0x8000);

 	layercontrol = cps1_cps_b_regs[0x20/2];

	/* Get video memory base registers */
	cps1_get_video_base();

	/* Build palette */
	fcrash_build_palette(screen->machine);

	fcrash_update_transmasks();

	tilemap_set_scrollx(cps1_bg_tilemap[0],0,cps1_scroll1x-62);
	tilemap_set_scrolly(cps1_bg_tilemap[0],0,cps1_scroll1y);
	if (videocontrol & 0x01)	/* linescroll enable */
	{
		int scrly=-cps1_scroll2y;
		int i;
		int otheroffs;

		tilemap_set_scroll_rows(cps1_bg_tilemap[1],1024);

		otheroffs = cps1_cps_a_regs[CPS1_ROWSCROLL_OFFS];

		for (i = 0;i < 256;i++)
			tilemap_set_scrollx(cps1_bg_tilemap[1],(i - scrly) & 0x3ff,cps1_scroll2x + cps1_other[(i + otheroffs) & 0x3ff]);
	}
	else
	{
		tilemap_set_scroll_rows(cps1_bg_tilemap[1],1);
		tilemap_set_scrollx(cps1_bg_tilemap[1],0,cps1_scroll2x-60);
	}
	tilemap_set_scrolly(cps1_bg_tilemap[1],0,cps1_scroll2y);
	tilemap_set_scrollx(cps1_bg_tilemap[2],0,cps1_scroll3x-64);
	tilemap_set_scrolly(cps1_bg_tilemap[2],0,cps1_scroll3y);


	/* turn all tilemaps on regardless of settings in get_video_base() */
	/* write a custom get_video_base for this bootleg hardware? */
	tilemap_set_enable(cps1_bg_tilemap[0],1);
	tilemap_set_enable(cps1_bg_tilemap[1],1);
	tilemap_set_enable(cps1_bg_tilemap[2],1);

	/* Blank screen */
	fillbitmap(bitmap,0xbff,cliprect);

	fillbitmap(priority_bitmap,0,cliprect);
	l0 = (layercontrol >> 0x06) & 03;
	l1 = (layercontrol >> 0x08) & 03;
	l2 = (layercontrol >> 0x0a) & 03;
	l3 = (layercontrol >> 0x0c) & 03;

	fcrash_render_layer(screen->machine,bitmap,cliprect,l0,0);
	if (l1 == 0) fcrash_render_high_layer(bitmap,cliprect,l0);
	fcrash_render_layer(screen->machine,bitmap,cliprect,l1,0);
	if (l2 == 0) fcrash_render_high_layer(bitmap,cliprect,l1);
	fcrash_render_layer(screen->machine,bitmap,cliprect,l2,0);
	if (l3 == 0) fcrash_render_high_layer(bitmap,cliprect,l2);
	fcrash_render_layer(screen->machine,bitmap,cliprect,l3,0);


	return 0;
}



static ADDRESS_MAP_START( fcrash_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM
	AM_RANGE(0x800030, 0x800031) AM_WRITE(cps1_coinctrl_w)
	AM_RANGE(0x800100, 0x80013f) AM_RAM AM_BASE(&cps1_cps_a_regs)	/* CPS-A custom */
	AM_RANGE(0x800140, 0x80017f) AM_RAM AM_BASE(&cps1_cps_b_regs)	/* CPS-B custom */
	AM_RANGE(0x880000, 0x880001) AM_READ(cps1_in1_r)          /* Player input ports */
	AM_RANGE(0x880006, 0x880007) AM_WRITE(fcrash_soundlatch_w) 	/* Sound command */
	AM_RANGE(0x880008, 0x88000f) AM_READ(cps1_dsw_r)          /* System input ports / Dip Switches */
	AM_RANGE(0x890000, 0x890001) AM_WRITENOP	// palette related?
	AM_RANGE(0x900000, 0x92ffff) AM_RAM_WRITE(cps1_gfxram_w) AM_BASE(&cps1_gfxram) AM_SIZE(&cps1_gfxram_size)
	AM_RANGE(0xff0000, 0xffffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK(1)
	AM_RANGE(0xd000, 0xd7ff) AM_RAM
	AM_RANGE(0xd800, 0xd800) AM_READWRITE(YM2203_status_port_0_r, YM2203_control_port_0_w)
	AM_RANGE(0xd801, 0xd801) AM_READWRITE(YM2203_read_port_0_r, YM2203_write_port_0_w)
	AM_RANGE(0xdc00, 0xdc00) AM_READWRITE(YM2203_status_port_1_r, YM2203_control_port_1_w)
	AM_RANGE(0xdc01, 0xdc01) AM_READWRITE(YM2203_read_port_1_r, YM2203_write_port_1_w)
	AM_RANGE(0xe000, 0xe000) AM_WRITE(fcrash_snd_bankswitch_w)
	AM_RANGE(0xe400, 0xe400) AM_READ(soundlatch_r)
	AM_RANGE(0xe800, 0xe800) AM_WRITE(fcrash_msm5205_0_data_w)
	AM_RANGE(0xec00, 0xec00) AM_WRITE(fcrash_msm5205_1_data_w)
ADDRESS_MAP_END


#define CPS1_COINAGE_1 \
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) ) \
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) ) \
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) ) \
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) ) \
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) ) \
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) ) \
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) ) \
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_6C ) ) \
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) ) \
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) ) \
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) ) \
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) ) \
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) ) \
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) ) \
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) ) \
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_6C ) )

static INPUT_PORTS_START( fcrash )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("DSWA")
	CPS1_COINAGE_1
	PORT_DIPNAME( 0x40, 0x40, "2 Coins to Start, 1 to Continue" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )

	PORT_START_TAG("DSWB")
	PORT_DIPNAME( 0x07, 0x04, "Difficulty Level 1" )
	PORT_DIPSETTING(    0x07, DEF_STR( Easiest ) )		// "01"
	PORT_DIPSETTING(    0x06, DEF_STR( Easier ) )		// "02"
	PORT_DIPSETTING(    0x05, DEF_STR( Easy ) )		// "03"
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )		// "04"
	PORT_DIPSETTING(    0x03, DEF_STR( Medium ) )		// "05"
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )		// "06"
	PORT_DIPSETTING(    0x01, DEF_STR( Harder ) )		// "07"
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )		// "08"
	PORT_DIPNAME( 0x18, 0x10, "Difficulty Level 2" )
	PORT_DIPSETTING(    0x18, DEF_STR( Easy ) )		// "01"
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )		// "02"
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )		// "03"
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )		// "04"
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x60, "100k" )
	PORT_DIPSETTING(    0x40, "200k" )
	PORT_DIPSETTING(    0x20, "100k and every 200k" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )

	PORT_START_TAG("DSWC")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Freeze" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Game Mode")
	PORT_DIPSETTING(    0x80, "Game" )
	PORT_DIPSETTING(    0x00, DEF_STR( Test ) )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME ("P1 Button 3 (Cheat)")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME ("P2 Button 3 (Cheat)")
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END



static const struct MSM5205interface msm5205_interface1 =
{
	m5205_int1,	/* interrupt function */
	MSM5205_S96_4B		/* 4KHz 4-bit */
};

static const struct MSM5205interface msm5205_interface2 =
{
	m5205_int2,	/* interrupt function */
	MSM5205_S96_4B		/* 4KHz 4-bit */
};


static MACHINE_DRIVER_START( fcrash )

	/* basic machine hardware */
	MDRV_CPU_ADD_TAG("main", M68000, 10000000)
	MDRV_CPU_PROGRAM_MAP(fcrash_map,0)
	MDRV_CPU_VBLANK_INT("main", cps1_interrupt)

	MDRV_CPU_ADD_TAG("sound", Z80, 24000000/6) /* ? */
	MDRV_CPU_PROGRAM_MAP(sound_map,0)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(8*8, (64-8)*8-1, 2*8, 30*8-1 )

	MDRV_GFXDECODE(cps1)
	MDRV_PALETTE_LENGTH(4096)

	MDRV_VIDEO_START(cps1)
	MDRV_VIDEO_EOF(cps1)
	MDRV_VIDEO_UPDATE(fcrash)

	// sound hardware
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(YM2203, 24000000/6)	/* ? */
	MDRV_SOUND_ROUTE(0, "mono", 0.10)
	MDRV_SOUND_ROUTE(1, "mono", 0.10)
	MDRV_SOUND_ROUTE(2, "mono", 0.10)
	MDRV_SOUND_ROUTE(3, "mono", 1.0)

	MDRV_SOUND_ADD(YM2203, 24000000/6)	/* ? */
	MDRV_SOUND_ROUTE(0, "mono", 0.10)
	MDRV_SOUND_ROUTE(1, "mono", 0.10)
	MDRV_SOUND_ROUTE(2, "mono", 0.10)
	MDRV_SOUND_ROUTE(3, "mono", 1.0)

	MDRV_SOUND_ADD(MSM5205, 24000000/64)	/* ? */
	MDRV_SOUND_CONFIG(msm5205_interface1)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MDRV_SOUND_ADD(MSM5205, 24000000/64)	/* ? */
	MDRV_SOUND_CONFIG(msm5205_interface2)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_DRIVER_END


ROM_START( fcrash )
	ROM_REGION( 0x200000, REGION_CPU1, 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "9.bin",  0x00000, 0x20000, CRC(c6854c91) SHA1(29f01cc65be5eaa3f86e99eebdd284104623abb0) )
	ROM_LOAD16_BYTE( "5.bin",  0x00001, 0x20000, CRC(77f7c2b3) SHA1(feea48d9555824a2e5bf5e99ce159edc015f0792) )
	ROM_LOAD16_BYTE( "8.bin",  0x40000, 0x20000, CRC(1895b3df) SHA1(415a26050c50ed79a7ee5ddd1b8d61593b1ce876) )
	ROM_LOAD16_BYTE( "4.bin",  0x40001, 0x20000, CRC(bbd411ee) SHA1(85d50ca72ec46d627f9c88ff0809aa30e164821a) )
	ROM_LOAD16_BYTE( "7.bin",  0x80000, 0x20000, CRC(5b23ebf2) SHA1(8c28c21a72a28ad249170026891c6bb865943f84) )
	ROM_LOAD16_BYTE( "3.bin",  0x80001, 0x20000, CRC(aba2aebe) SHA1(294109b5929ed63859a55bef16643e3ade7da16f) )
	ROM_LOAD16_BYTE( "6.bin",  0xc0000, 0x20000, CRC(d4bf37f6) SHA1(f47e1cc9aa3b3019ee57f59715e3a611acf9fe3e) )
	ROM_LOAD16_BYTE( "2.bin",  0xc0001, 0x20000, CRC(07ac8f43) SHA1(7a41b003c76adaabd3f94929cc163461b70e0ed9) )

	ROM_REGION( 0x200000, REGION_GFX1, 0 )
	ROMX_LOAD( "18.bin",     0x000000, 0x20000, CRC(f1eee6d9) SHA1(bee95efbff49c582cff1cc6d9bb5ef4ea5c4a074) , ROM_SKIP(3) )
	ROMX_LOAD( "20.bin",     0x000001, 0x20000, CRC(675f4537) SHA1(acc68822da3aafbb62f76cbffa5f3389fcc91447) , ROM_SKIP(3) )
	ROMX_LOAD( "22.bin",     0x000002, 0x20000, CRC(db8a32ac) SHA1(b95f73dff291acee239e22e5fd7efe15d0de23be) , ROM_SKIP(3) )
	ROMX_LOAD( "24.bin",     0x000003, 0x20000, CRC(f4113e57) SHA1(ff1f443c13494a169b9be24abc361d27a6d01c09) , ROM_SKIP(3) )
	ROMX_LOAD( "10.bin",     0x080000, 0x20000, CRC(d478853e) SHA1(91fcf8eb022ccea66d291bec84ace557181cf861) , ROM_SKIP(3) )
	ROMX_LOAD( "12.bin",     0x080001, 0x20000, CRC(25055642) SHA1(578cf6a436489cc1f2d1acdb0cba6c1cbee2e21f) , ROM_SKIP(3) )
	ROMX_LOAD( "14.bin",     0x080002, 0x20000, CRC(b77d0328) SHA1(42eb1ebfda301f2b09f3add5932e8331f4790706) , ROM_SKIP(3) )
	ROMX_LOAD( "16.bin",     0x080003, 0x20000, CRC(ea111a79) SHA1(1b86aa984d2d6c527e96b61274a82263f34d0d89) , ROM_SKIP(3) )
	ROMX_LOAD( "19.bin",     0x100000, 0x20000, CRC(b3aa1f48) SHA1(411f3855739992f5967e915f2a5255afcedeac2e) , ROM_SKIP(3) )
	ROMX_LOAD( "21.bin",     0x100001, 0x20000, CRC(04d175c9) SHA1(33e6e3fefae4e3977c8c954fbd7feff36e92d723) , ROM_SKIP(3) )
	ROMX_LOAD( "23.bin",     0x100002, 0x20000, CRC(e592ba4f) SHA1(62559481e0da3954a90da0ab0fb51f87f1b3dd9d) , ROM_SKIP(3) )
	ROMX_LOAD( "25.bin",     0x100003, 0x20000, CRC(b89a740f) SHA1(516d73c772e0a904dfb0bd84874919d78bbbd200) , ROM_SKIP(3) )
	ROMX_LOAD( "11.bin",     0x180000, 0x20000, CRC(d4457a60) SHA1(9e956efafa81a81aca92837df03968f5670ffc15) , ROM_SKIP(3) )
	ROMX_LOAD( "13.bin",     0x180001, 0x20000, CRC(3b26a37d) SHA1(58d8d0cdef81c938fb1a5595f2d02b228865893b) , ROM_SKIP(3) )
	ROMX_LOAD( "15.bin",     0x180002, 0x20000, CRC(6d837e09) SHA1(b4a133ab96c35b689ee692bfcc04981791099b6f) , ROM_SKIP(3) )
	ROMX_LOAD( "17.bin",     0x180003, 0x20000, CRC(c59a4d6c) SHA1(59e49c7d24dd333007de4bb621050011a5392bcc) , ROM_SKIP(3) )

	ROM_REGION( 0x8000, REGION_GFX2, 0 )
	ROM_COPY( REGION_GFX1, 0x000000, 0x000000, 0x8000 )	/* stars */

	ROM_REGION( 0x30000, REGION_CPU2, 0 ) /* Audio CPU + Sample Data */
	ROM_LOAD( "1.bin",   0x00000, 0x20000, CRC(5b276c14) SHA1(73e53c077d4e3c1b919eee28b29e34176ee204f8) )
	ROM_RELOAD(          0x10000, 0x20000 )
ROM_END

GAME( 1990, fcrash,   ffight,  fcrash,     fcrash,   cps1,     ROT0,   "Playmark, bootleg [Capcom]", "Final Crash (World, bootleg)", 0 )
