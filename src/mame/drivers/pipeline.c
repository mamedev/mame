/*
PCB Layout
----------

|----------------------------------------------------------|
|Z8430   ROM2.U84                          ROM3.U32        |
|                                                          |
|                                          ROM4.U31        |
|YM2203  Z80                                               |
|                                          ROM5.U30        |
|        6116                                              |
|                                          6116            |
| 8255                                                     |
|                                          6116   7.3728MHz|
|       Y3014                                              |
|                                                82S129.U10|
|J       82S123.U79                                        |
|A                                               82S129.U9 |
|M                                                         |
|M             6116                                        |
|A                                               ROM6.U8   |
|          ROM1.U77                                        |
| VOL                                            ROM7.U7   |
|               Z80                                        |
| UPC1241H                                       ROM8.U6   |
|              8255                                        |
|                                  6116          ROM9.U5   |
|                                                          |
|           68705R3                6116          ROM10.U4  |
|                                                          |
|DSW1                              6116          ROM11.U3  |
|                                                          |
|    8255                          6116          ROM12.U2  |
|                                                          |
|DSW2                              6116          ROM13.U1  |
|----------------------------------------------------------|
Notes:
      Z80 clocks (both) - 3.6864MHz [7.3728/2]
      Z8430             - zILOG Z8430 z80 CTC Counter Timer Circuit, clock 3.6864MHz [7.3728/2] (DIP28)
      68705R3           - Motorola MC68705R3 Microcontroller, clock 3.6864MHz [7.3728/2] (DIP40)
      YM2203 clock      - 1.8432MHz [7.3728/4]
      6116              - 2K x8 SRAM (DIP24)
      8255              - NEC D8255AC-2 Programmable Peripheral Interface (DIP40)
      VSync             - 60.0Hz
      HSync             - 15.40kHz


*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80ctc.h"
#include "sound/2203intf.h"
#include "machine/8255ppi.h"
#include "cpu/z80/z80daisy.h"
#include "cpu/m6805/m6805.h"

static tilemap_t *tilemap1;
static tilemap_t *tilemap2;

static UINT8 *vram1;
static UINT8 *vram2;

static UINT8 vidctrl;
static UINT8 *palram;
static UINT8 toMCU, fromMCU, ddrA;

static TILE_GET_INFO( get_tile_info )
{
	int code = vram2[tile_index]+vram2[tile_index+0x800]*256;
	SET_TILE_INFO(
		0,
		code,
		0,
		0);
}

static TILE_GET_INFO( get_tile_info2 )
{
	int code =vram1[tile_index]+((vram1[tile_index+0x800]>>4))*256;
	int color=((vram1[tile_index+0x800])&0xf);
	SET_TILE_INFO
	(
		1,
		code,
		color,
		0
	);
}

static VIDEO_START ( pipeline )
{
	palram=auto_alloc_array(machine, UINT8, 0x1000);
	tilemap1 = tilemap_create( machine, get_tile_info,tilemap_scan_rows,8,8,64,32 );
	tilemap2 = tilemap_create( machine, get_tile_info2,tilemap_scan_rows,8,8,64,32 );
	tilemap_set_transparent_pen(tilemap2,0);
}

static VIDEO_UPDATE ( pipeline)
{
	tilemap_draw(bitmap,cliprect,tilemap1, 0,0);
	tilemap_draw(bitmap,cliprect,tilemap2, 0,0);
	return 0;
}


static INPUT_PORTS_START( pipeline )
	PORT_START("P1")
	PORT_BIT(  0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT(  0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT(  0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT(  0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT(  0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT(  0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(  0x40, IP_ACTIVE_LOW, IPT_COIN1   )
	PORT_BIT(  0x80, IP_ACTIVE_LOW, IPT_START1  )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "DIPSW 1-1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "DIPSW 1-2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "DIPSW 1-3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "DIPSW 1-4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "DIPSW 1-5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "DIPSW 1-6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "DIPSW 1-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "DIPSW 1-8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "DIPSW 2-1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "DIPSW 2-2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "DIPSW 2-3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "DIPSW 2-4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "DIPSW 2-5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "DIPSW 2-6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "DIPSW 2-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "DIPSW 2-8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

INPUT_PORTS_END

static WRITE8_DEVICE_HANDLER(vidctrl_w)
{
	vidctrl=data;
}

static WRITE8_HANDLER(vram2_w)
{
	if(!(vidctrl&1))
	{
		tilemap_mark_tile_dirty(tilemap1,offset&0x7ff);
		vram2[offset]=data;
	}
	else
	{
		 palram[offset]=data;
		 if(offset<0x300)
		 {
			offset&=0xff;
			palette_set_color_rgb(space->machine, offset, pal6bit(palram[offset]), pal6bit(palram[offset+0x100]), pal6bit(palram[offset+0x200]));
		 }
	}
}

static WRITE8_HANDLER(vram1_w)
{
	tilemap_mark_tile_dirty(tilemap2,offset&0x7ff);
	vram1[offset]=data;
}

static READ8_DEVICE_HANDLER(protection_r)
{
	return fromMCU;
}

static TIMER_CALLBACK( protection_deferred_w )
{
	toMCU = param;
}

static WRITE8_DEVICE_HANDLER(protection_w)
{
	timer_call_after_resynch(device->machine, NULL, data, protection_deferred_w);
	cpuexec_boost_interleave(device->machine, attotime_zero, ATTOTIME_IN_USEC(100));
}

static ADDRESS_MAP_START( cpu0_mem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x8800, 0x97ff) AM_RAM_WRITE(vram1_w) AM_BASE(&vram1)
	AM_RANGE(0x9800, 0xa7ff) AM_RAM_WRITE(vram2_w) AM_BASE(&vram2)
	AM_RANGE(0xb800, 0xb803) AM_DEVREADWRITE("ppi8255_0", ppi8255_r, ppi8255_w)
	AM_RANGE(0xb810, 0xb813) AM_DEVREADWRITE("ppi8255_1", ppi8255_r, ppi8255_w)
	AM_RANGE(0xb830, 0xb830) AM_NOP
	AM_RANGE(0xb840, 0xb840) AM_NOP
ADDRESS_MAP_END

static ADDRESS_MAP_START( cpu1_mem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
	AM_RANGE(0xe000, 0xe003) AM_DEVREADWRITE("ppi8255_2", ppi8255_r, ppi8255_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_port, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("ctc", z80ctc_r, z80ctc_w)
	AM_RANGE(0x06, 0x07) AM_NOP
ADDRESS_MAP_END

static WRITE8_HANDLER(mcu_portA_w)
{
	fromMCU=data;
}

static READ8_HANDLER(mcu_portA_r)
{
	return (fromMCU&ddrA)|(toMCU& ~ddrA);
}

static WRITE8_HANDLER(mcu_ddrA_w)
{
	ddrA=data;
}

static ADDRESS_MAP_START( mcu_mem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0000) AM_READ(mcu_portA_r) AM_WRITE(mcu_portA_w)
	AM_RANGE(0x0004, 0x0004) AM_WRITE(mcu_ddrA_w)
	AM_RANGE(0x0010, 0x007f) AM_RAM
	AM_RANGE(0x0080, 0x0fff) AM_ROM
ADDRESS_MAP_END

static const gfx_layout layout_8x8x8 =
{
	8,8,
	RGN_FRAC(1,8),
	8,
	{RGN_FRAC(0,8),RGN_FRAC(1,8),RGN_FRAC(2,8),RGN_FRAC(3,8),RGN_FRAC(4,8),RGN_FRAC(5,8),RGN_FRAC(6,8),RGN_FRAC(7,8)},
	{ 0, 1, 2,  3,  4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout layout_8x8x3 =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{RGN_FRAC(0,3),RGN_FRAC(1,3),RGN_FRAC(2,3)},
	{ 0, 1, 2,  3,  4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( pipeline )
	GFXDECODE_ENTRY( "gfx1", 0, layout_8x8x8, 0x000, 1 ) // 8bpp tiles
	GFXDECODE_ENTRY( "gfx2", 0, layout_8x8x3, 0x100, 32 ) // 3bpp tiles
GFXDECODE_END

static Z80CTC_INTERFACE( ctc_intf )
{
	0,							// timer disables
	DEVCB_CPU_INPUT_LINE("audiocpu", INPUT_LINE_IRQ0),		// interrupt handler
	DEVCB_NULL,					// ZC/TO0 callback
	DEVCB_NULL,					// ZC/TO1 callback
	DEVCB_NULL					// ZC/TO2 callback
};

static const z80_daisy_config daisy_chain_sound[] =
{
	{ "ctc" },
	{ NULL }
};

static const ppi8255_interface ppi8255_intf[3] =
{
	{
		DEVCB_INPUT_PORT("P1"),			/* Port A read */
		DEVCB_NULL,						/* Port B read */
		DEVCB_NULL,						/* Port C read */
		DEVCB_NULL,						/* Port A write */
		DEVCB_NULL,						/* Port B write */
		DEVCB_HANDLER(vidctrl_w)		/* Port C write */
	},
	{
		DEVCB_INPUT_PORT("DSW1"),		/* Port A read */
		DEVCB_INPUT_PORT("DSW2"),		/* Port B read */
		DEVCB_HANDLER(protection_r),	/* Port C read */
		DEVCB_NULL,						/* Port A write */
		DEVCB_NULL,						/* Port B write */
		DEVCB_HANDLER(protection_w)		/* Port C write */
	},
	{
		DEVCB_NULL,						/* Port A read */
		DEVCB_NULL,						/* Port B read */
		DEVCB_NULL,						/* Port C read */
		DEVCB_NULL,						/* Port A write */
		DEVCB_NULL,						/* Port B write */
		DEVCB_NULL						/* Port C write */
	}
};

static const ym2203_interface ym2203_config =
{
	{
		AY8910_LEGACY_OUTPUT,
		AY8910_DEFAULT_LOADS,
		DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL
	},
	NULL
};

static PALETTE_INIT(pipeline)
{
	int r,g,b,i,c;
	UINT8 *prom1 = &memory_region(machine, "proms")[0x000];
	UINT8 *prom2 = &memory_region(machine, "proms")[0x100];

	for(i=0;i<0x100;i++)
	{
		c=prom1[i]|(prom2[i]<<4);
		r=c&7;
		g=(c>>3)&7;
		b=(c>>6)&3;
		r*=36;
		g*=36;
		b*=85;
		palette_set_color(machine, 0x100+i, MAKE_RGB(r, g, b));
	}
}

static MACHINE_DRIVER_START( pipeline )
	/* basic machine hardware */

	MDRV_CPU_ADD("maincpu", Z80, 7372800/2)
	MDRV_CPU_PROGRAM_MAP(cpu0_mem)
	MDRV_CPU_VBLANK_INT("screen", nmi_line_pulse)

	MDRV_CPU_ADD("audiocpu", Z80, 7372800/2)
	MDRV_CPU_CONFIG(daisy_chain_sound)
	MDRV_CPU_PROGRAM_MAP(cpu1_mem)
	MDRV_CPU_IO_MAP(sound_port)

	MDRV_CPU_ADD("mcu", M68705, 7372800/2)
	MDRV_CPU_PROGRAM_MAP(mcu_mem)

	MDRV_Z80CTC_ADD( "ctc", 7372800/2 /* same as "audiocpu" */, ctc_intf )

	MDRV_PPI8255_ADD( "ppi8255_0", ppi8255_intf[0] )
	MDRV_PPI8255_ADD( "ppi8255_1", ppi8255_intf[1] )
	MDRV_PPI8255_ADD( "ppi8255_2", ppi8255_intf[2] )

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(512, 512)
	MDRV_SCREEN_VISIBLE_AREA(0, 319, 16, 239)

	MDRV_GFXDECODE(pipeline)

	MDRV_PALETTE_INIT(pipeline)
	MDRV_PALETTE_LENGTH(0x100+0x100)

	MDRV_VIDEO_START(pipeline)
	MDRV_VIDEO_UPDATE(pipeline)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ymsnd", YM2203, 7372800/4)
	MDRV_SOUND_CONFIG(ym2203_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.60)

MACHINE_DRIVER_END

ROM_START( pipeline )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rom1.u77", 0x00000, 0x08000, CRC(6e928290) SHA1(e2c8c35c04fd8ce3ddd6ecec04b0193a248e4362) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "rom2.u84", 0x00000, 0x08000, CRC(e77c43b7) SHA1(8b04005bc448083a429ace3319fc7e168a61f2f9) )

	ROM_REGION( 0x1000, "mcu", 0 )
	ROM_LOAD( "68705r3.u74", 0x00000, 0x01000, CRC(9bef427e) SHA1(d8e9b144190ac1c837e379e4be69d1e258a6c666) )

	ROM_REGION( 0x18000, "gfx2",0  )
	ROM_LOAD( "rom3.u32", 0x00000, 0x08000, CRC(d065ca46) SHA1(9fd8bb66735195d1cd20420096438abb5cb3fd54) )
	ROM_LOAD( "rom4.u31", 0x08000, 0x08000, CRC(6dc86355) SHA1(4b73e95726f7f244977634a5a152c90acb4ba89f) )
	ROM_LOAD( "rom5.u30", 0x10000, 0x08000, CRC(93f3f82a) SHA1(bc018370efc67a614ef6efa82526225f0db008ac) )

	ROM_REGION( 0x100000, "gfx1",0 )
	ROM_LOAD( "rom13.u1", 0x00000, 0x20000,CRC(611d7e01) SHA1(77e83c51b059f6d64009740d2e7e7ac1c8a6c7ec) )
	ROM_LOAD( "rom12.u2", 0x20000, 0x20000,CRC(8933c908) SHA1(9f04e454aacda479b6d6dd84dedbf56855f07fca) )
	ROM_LOAD( "rom11.u3", 0x40000, 0x20000,CRC(4e20e82d) SHA1(507b996ab5c8b8fb88f826d808f4b74dfc770db3) )
	ROM_LOAD( "rom10.u4", 0x60000, 0x20000,CRC(9892e465) SHA1(8789d169128bfc8de449bd617601a0f7fe1a19fb) )
	ROM_LOAD( "rom9.u5",  0x80000, 0x20000,CRC(07d16ca1) SHA1(caecf98284236af0e1f77566a9c6950491d0902a) )
	ROM_LOAD( "rom8.u6",  0xa0000, 0x20000,CRC(4e244c8a) SHA1(6808b2b195601e8041a12fff4b77e487efba015e) )
	ROM_LOAD( "rom7.u7",  0xc0000, 0x20000,CRC(23eb84dd) SHA1(ad1d359ba59087a5e786d262194cfe7db9bb0000) )
	ROM_LOAD( "rom6.u8",  0xe0000, 0x20000,CRC(c34bee64) SHA1(2da2dc6e6615ccc2e3e7ca0ceb735a347923a728) )

	ROM_REGION( 0x00220, "proms", 0 )
	ROM_LOAD( "82s129.u10", 0x00000, 0x00100,CRC(e91a1f9e) SHA1(a293023ebe96a5438e89457a98d94beb6dad5418) )
	ROM_LOAD( "82s129.u9",  0x00100, 0x00100,CRC(1cc09f6f) SHA1(35c857e0f3df0dcceec963459978e779e94f76f6) )
	ROM_LOAD( "82s123.u79", 0x00200, 0x00020,CRC(6df3f972) SHA1(0096a7f7452b70cac6c0752cb62e24b643015b5c) )
ROM_END

GAME( 1990, pipeline, 0, pipeline, pipeline, 0, ROT0, "Daehyun Electronics", "Pipeline",GAME_NO_SOUND )

