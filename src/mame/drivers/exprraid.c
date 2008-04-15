/***************************************************************************

Express Raider - (c) 1986 Data East USA

Ernesto Corvi
ernesto@imagina.com

Last Modified 14th Jul 2003

Memory Map:
Main CPU: ( 6502 )
0000-05ff RAM
0600-07ff Sprites
0800-0bff Videoram
0c00-0fff Colorram
1800-1800 DSW 0
1801-1801 Controls
1802-1802 Coins
1803-1803 DSW 1
2100-2100 Sound latch write
2800-2801 Protection
3800-3800 VBblank ( bootleg 1 only )
4000-ffff SMH_ROM

Sound Cpu: ( 6809 )
0000-1fff RAM
2000-2001 YM2203
4000-4001 YM3526
6000-6000 Sound latch read
8000-ffff ROM

NOTES:
The main 6502 cpu is a custom one. The differences with a regular 6502 is as follows:
- Extra opcode ( $4b00 ), wich i think reads an external port. VBlank irq is on bit 1 ( 0x02 ).
- Reset, IRQ and NMI vectors are moved.

Also, there was some protection circuitry which is now emulated.

The way i dealt with the custom opcode was to change it to return memory
position $ff (wich i verified is not used by the game). And i hacked in
a read handler wich returns the vblank on bit 1. It's an ugly hack, but
works fine.

The bootleg version patched the rom to get rid of the extra opcode ( bootlegs
used a regular 6502 ), the vectors hardcoded in place, and also had the
protection cracked.

The background tiles had a very ugly encoding. It was so ugly that our
decode gfx routine will not be able to decode it without some little help.
So thats why exprraid_gfx_expand() is there. Many thanks to Phil
Stroffolino, who figured out the encoding.

NOTES ON THE BOOTLEGS:

1st bootleg set expects to read vblank status from 0x3800, country warning
sign has been defaced by the bootleggers

2nd bootleg set expects to read vblank status from 0xFFC0, country warning
sign is intact, however Credit is spelt incorrectly.

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "cpu/m6809/m6809.h"
#include "sound/2203intf.h"
#include "sound/3812intf.h"


extern WRITE8_HANDLER( exprraid_videoram_w );
extern WRITE8_HANDLER( exprraid_colorram_w );
extern WRITE8_HANDLER( exprraid_flipscreen_w );
extern WRITE8_HANDLER( exprraid_bgselect_w );
extern WRITE8_HANDLER( exprraid_scrollx_w );
extern WRITE8_HANDLER( exprraid_scrolly_w );

extern VIDEO_START( exprraid );
extern VIDEO_UPDATE( exprraid );


/*****************************************************************************************/
/* Emulate Protection ( only for original express raider, code is cracked on the bootleg */
/*****************************************************************************************/

static UINT8 *main_ram;

static READ8_HANDLER( exprraid_protection_r )
{
	switch (offset)
	{
	case 0:
		return main_ram[0x02a9];
	case 1:
		return 0x02;
	}

	return 0;
}

static WRITE8_HANDLER( sound_cpu_command_w )
{
    soundlatch_w(machine,0,data);
    cpunum_set_input_line(machine, 1,INPUT_LINE_NMI,PULSE_LINE);
}

static READ8_HANDLER( vblank_r )
{
	return input_port_read_indexed(machine,  0 );
}

static ADDRESS_MAP_START( master_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x00ff, 0x00ff) AM_READ(vblank_r) /* HACK!!!! see init_exprraid below */
    AM_RANGE(0x0000, 0x05ff) AM_RAM AM_BASE(&main_ram)
    AM_RANGE(0x0600, 0x07ff) AM_RAM AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
    AM_RANGE(0x0800, 0x0bff) AM_RAM AM_WRITE(exprraid_videoram_w) AM_BASE(&videoram)
    AM_RANGE(0x0c00, 0x0fff) AM_RAM AM_WRITE(exprraid_colorram_w) AM_BASE(&colorram)
	AM_RANGE(0x1317, 0x1317) AM_READNOP // ???
	AM_RANGE(0x1700, 0x1700) AM_READNOP // ???
    AM_RANGE(0x1800, 0x1800) AM_READ(input_port_1_r) /* DSW 0 */
    AM_RANGE(0x1801, 0x1801) AM_READ(input_port_2_r) /* Controls */
    AM_RANGE(0x1802, 0x1802) AM_READ(input_port_3_r) /* Coins */
    AM_RANGE(0x1803, 0x1803) AM_READ(input_port_4_r) /* DSW 1 */
	AM_RANGE(0x2000, 0x2000) AM_WRITENOP // ???
    AM_RANGE(0x2001, 0x2001) AM_WRITE(sound_cpu_command_w)
	AM_RANGE(0x2002, 0x2002) AM_WRITE(exprraid_flipscreen_w)
	AM_RANGE(0x2003, 0x2003) AM_WRITENOP // ???
	AM_RANGE(0x2800, 0x2801) AM_READ(exprraid_protection_r)
    AM_RANGE(0x2800, 0x2803) AM_WRITE(exprraid_bgselect_w)
    AM_RANGE(0x2804, 0x2804) AM_WRITE(exprraid_scrolly_w)
    AM_RANGE(0x2805, 0x2806) AM_WRITE(exprraid_scrollx_w)
    AM_RANGE(0x2807, 0x2807) AM_WRITENOP	// Scroll related ?
    AM_RANGE(0x4000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( slave_map, ADDRESS_SPACE_PROGRAM, 8 )
    AM_RANGE(0x0000, 0x1fff) AM_RAM
    AM_RANGE(0x2000, 0x2000) AM_READWRITE(YM2203_status_port_0_r, YM2203_control_port_0_w)
	AM_RANGE(0x2001, 0x2001) AM_READWRITE(YM2203_read_port_0_r, YM2203_write_port_0_w)
    AM_RANGE(0x4000, 0x4000) AM_READWRITE(YM3526_status_port_0_r, YM3526_control_port_0_w)
    AM_RANGE(0x4001, 0x4001) AM_WRITE(YM3526_write_port_0_w)
	AM_RANGE(0x6000, 0x6000) AM_READ(soundlatch_r)
    AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END


static INPUT_PORTS_START( exprraid )
	PORT_START /* IN 0 - 0x3800 */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_VBLANK )

	PORT_START /* DSW 0 - 0x1800 */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START /* IN 1 - 0x1801 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START /* IN 2 - 0x1802 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START /* IN 3 - 0x1803 */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(	0x00, DEF_STR( Infinite ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x04, "50K+" )
	PORT_DIPSETTING(    0x00, "50K 80K" )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,	/* 8*8 characters */
	1024,	/* 1024 characters */
	2,	/* 2 bits per pixel */
	{ 0, 4 },	/* the bitplanes are packed in the same byte */
	{ (0x2000*8)+0, (0x2000*8)+1, (0x2000*8)+2, (0x2000*8)+3, 0, 1, 2, 3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8	/* every char takes 8 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	16,16,	/* 16*16 sprites */
	2048,	/* 2048 sprites */
	3,	/* 3 bits per pixel */
	{ 2*2048*32*8, 2048*32*8, 0 },	/* the bitplanes are separated */
	{ 128+0, 128+1, 128+2, 128+3, 128+4, 128+5, 128+6, 128+7, 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8	/* every char takes 32 consecutive bytes */
};

static const gfx_layout tile1 =
{
	16,16,	/* 16*16 tiles */
	128,	/* 128 tiles */
	3,	/* 3 bits per pixel */
	{ 4, 0x10000*8+0, 0x10000*8+4 },
	{ 0, 1, 2, 3, 1024*32*2,1024*32*2+1,1024*32*2+2,1024*32*2+3,
		128+0,128+1,128+2,128+3,128+1024*32*2,128+1024*32*2+1,128+1024*32*2+2,128+1024*32*2+3 }, /* BOGUS */
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8,
		64+0*8,64+1*8,64+2*8,64+3*8,64+4*8,64+5*8,64+6*8,64+7*8 },
	32*8
};

static const gfx_layout tile2 =
{
	16,16,	/* 16*16 tiles */
	128,	/* 128 tiles */
	3,	/* 3 bits per pixel */
	{ 0, 0x11000*8+0, 0x11000*8+4  },
	{ 0, 1, 2, 3, 1024*32*2,1024*32*2+1,1024*32*2+2,1024*32*2+3,
		128+0,128+1,128+2,128+3,128+1024*32*2,128+1024*32*2+1,128+1024*32*2+2,128+1024*32*2+3 }, /* BOGUS */
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8,
		64+0*8,64+1*8,64+2*8,64+3*8,64+4*8,64+5*8,64+6*8,64+7*8 },
	32*8
};


static GFXDECODE_START( exprraid )
	GFXDECODE_ENTRY( REGION_GFX1, 0x00000, charlayout,   128, 2 ) /* characters */
	GFXDECODE_ENTRY( REGION_GFX2, 0x00000, spritelayout,  64, 8 ) /* sprites */
	GFXDECODE_ENTRY( REGION_GFX3, 0x00000, tile1,          0, 4 ) /* background tiles */
	GFXDECODE_ENTRY( REGION_GFX3, 0x00000, tile2,          0, 4 )
	GFXDECODE_ENTRY( REGION_GFX3, 0x04000, tile1,          0, 4 )
	GFXDECODE_ENTRY( REGION_GFX3, 0x04000, tile2,          0, 4 )
	GFXDECODE_ENTRY( REGION_GFX3, 0x08000, tile1,          0, 4 )
	GFXDECODE_ENTRY( REGION_GFX3, 0x08000, tile2,          0, 4 )
	GFXDECODE_ENTRY( REGION_GFX3, 0x0c000, tile1,          0, 4 )
	GFXDECODE_ENTRY( REGION_GFX3, 0x0c000, tile2,          0, 4 )
GFXDECODE_END



/* handler called by the 3812 emulator when the internal timers cause an IRQ */
static void irqhandler(int linestate)
{
	cpunum_set_input_line_and_vector(Machine, 1,0,linestate,0xff);
}

static const struct YM3526interface ym3526_interface =
{
	irqhandler
};

static INTERRUPT_GEN( exprraid_interrupt )
{
	static int coin = 0;

	if ( ( ~input_port_read_indexed(machine,  3 ) ) & 0xc0 ) {
		if ( coin == 0 ) {
			coin = 1;
			cpunum_set_input_line(machine, 0, INPUT_LINE_NMI, PULSE_LINE);
		}
	} else
		coin = 0;
}

static MACHINE_DRIVER_START( exprraid )

	/* basic machine hardware */
	MDRV_CPU_ADD(M6502, 4000000)        /* 4 MHz ??? */
	MDRV_CPU_PROGRAM_MAP(master_map, 0)
	MDRV_CPU_VBLANK_INT("main", exprraid_interrupt)

	MDRV_CPU_ADD(M6809, 2000000)        /* 2 MHz ??? */
	MDRV_CPU_PROGRAM_MAP(slave_map, 0)
								/* IRQs are caused by the YM3526 */

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(1*8, 31*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(exprraid)
	MDRV_PALETTE_LENGTH(256)

	MDRV_PALETTE_INIT(RRRR_GGGG_BBBB)
	MDRV_VIDEO_START(exprraid)
	MDRV_VIDEO_UPDATE(exprraid)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(YM2203, 1500000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	MDRV_SOUND_ADD(YM3526, 3600000)
	MDRV_SOUND_CONFIG(ym3526_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.60)
MACHINE_DRIVER_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( exprraid )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "cz01",    0x4000, 0x4000, CRC(dc8f9fba) SHA1(cae6af54fc0081d606b6884e8873aed356a37ba9) )
	ROM_LOAD( "cz00",    0x8000, 0x8000, CRC(a81290bc) SHA1(ddb0acda6124427bee691f9926c41fda27ed816e) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )	/* 64k for the sub cpu */
	ROM_LOAD( "cz02",    0x8000, 0x8000, CRC(552e6112) SHA1(f8412a63cab0aa47321d602f69bf534426c6aa5d) )

	ROM_REGION( 0x04000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "cz07",    0x00000, 0x4000, CRC(686bac23) SHA1(b6c96ed40e90a8ba32c2e78a65f9589d387b0254) )	/* characters */

	ROM_REGION( 0x30000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "cz09",    0x00000, 0x8000, CRC(1ed250d1) SHA1(c98b0440e4319308e683e857bbfeb6a150c76ff3) )	/* sprites */
	ROM_LOAD( "cz08",    0x08000, 0x8000, CRC(2293fc61) SHA1(bf81db375f5424396559dcf0e04d34a52f6a020a) )
	ROM_LOAD( "cz13",    0x10000, 0x8000, CRC(7c3bfd00) SHA1(87b48e09aaeacf78f3260df893b0922e25d10a5d) )
	ROM_LOAD( "cz12",    0x18000, 0x8000, CRC(ea2294c8) SHA1(bc996351921e68e6237cee2d29fee882931ce0ea) )
	ROM_LOAD( "cz11",    0x20000, 0x8000, CRC(b7418335) SHA1(e9d08ee651b9221c371e2629a757bceca7b6192b) )
	ROM_LOAD( "cz10",    0x28000, 0x8000, CRC(2f611978) SHA1(fb60be573184d2af1dfdd543e68eeec53f2788f2) )

	ROM_REGION( 0x20000, REGION_GFX3, ROMREGION_DISPOSE )
	ROM_LOAD( "cz04",    0x00000, 0x8000, CRC(643a1bd3) SHA1(b23631d96cb413808f65f3ebe8fe6539b6140606) )	/* tiles */
	/* Save 0x08000-0x0ffff to expand the previous so we can decode the thing */
	ROM_LOAD( "cz05",    0x10000, 0x8000, CRC(c44570bf) SHA1(3e9b8b6b36c7f5ae016dba3987ea19a29bd5ee5b) )	/* tiles */
	ROM_LOAD( "cz06",    0x18000, 0x8000, CRC(b9bb448b) SHA1(84974b1f3a5b58cd427d874f805a6dd9244c1101) )	/* tiles */

	ROM_REGION( 0x8000, REGION_GFX4, 0 )     /* background tilemaps */
	ROM_LOAD( "cz03",    0x0000, 0x8000, CRC(6ce11971) SHA1(16bfa69b3ad02253e81c8110c9b840be03952790) )

	ROM_REGION( 0x0400, REGION_PROMS, 0 )
	ROM_LOAD( "cz17.prm", 0x0000, 0x0100, CRC(da31dfbc) SHA1(ac476440864f538918f7bef2e1db82fd19195f89) ) /* red */
	ROM_LOAD( "cz16.prm", 0x0100, 0x0100, CRC(51f25b4c) SHA1(bfcca57613fbb22919e00db1f6a8c7ca50faa60b) ) /* green */
	ROM_LOAD( "cz15.prm", 0x0200, 0x0100, CRC(a6168d7f) SHA1(0c7b31adcd764ce2631c3fb5c1a968b01f65e741) ) /* blue */
	ROM_LOAD( "cz14.prm", 0x0300, 0x0100, CRC(52aad300) SHA1(ff09772b930afa87e28d0628ef85a589a3d149c9) ) /* ??? */

	ROM_REGION( 0x0400, REGION_PLDS, ROMREGION_DISPOSE )
	ROM_LOAD( "pal16r4a.5c",   0x0000, 0x0104, CRC(d66aaa87) SHA1(dc29b473238ed6a9de2076c79644b613a9ba6924) )
	ROM_LOAD( "pal16r4a.5e",   0x0200, 0x0104, CRC(9a8766a7) SHA1(5f84ad9e633daeb14531ef527827ef3d9b269437) )
ROM_END

ROM_START( exprrada )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "cz01-2e.15a", 0x4000, 0x4000, CRC(a0ae6756) SHA1(7f7ec1efddbb62e9d201c6013bca8ab72c3f75f6) )
	ROM_LOAD( "cz00-4e.16b", 0x8000, 0x8000, CRC(910f6ccc) SHA1(1dbf164a7add9335d90ee07b6db9a162a28e407b) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )	/* 64k for the sub cpu */
	ROM_LOAD( "cz02-1.2a",    0x8000, 0x8000, CRC(552e6112) SHA1(f8412a63cab0aa47321d602f69bf534426c6aa5d) )

	ROM_REGION( 0x04000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "cz07.5b",    0x00000, 0x4000, CRC(686bac23) SHA1(b6c96ed40e90a8ba32c2e78a65f9589d387b0254) )	/* characters */

	ROM_REGION( 0x30000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "cz09.16h",    0x00000, 0x8000, CRC(1ed250d1) SHA1(c98b0440e4319308e683e857bbfeb6a150c76ff3) )	/* sprites */
	ROM_LOAD( "cz08.14h",    0x08000, 0x8000, CRC(2293fc61) SHA1(bf81db375f5424396559dcf0e04d34a52f6a020a) )
	ROM_LOAD( "cz13.16k",    0x10000, 0x8000, CRC(7c3bfd00) SHA1(87b48e09aaeacf78f3260df893b0922e25d10a5d) )
	ROM_LOAD( "cz12.14k",    0x18000, 0x8000, CRC(ea2294c8) SHA1(bc996351921e68e6237cee2d29fee882931ce0ea) )
	ROM_LOAD( "cz11.13k",    0x20000, 0x8000, CRC(b7418335) SHA1(e9d08ee651b9221c371e2629a757bceca7b6192b) )
	ROM_LOAD( "cz10.11k",    0x28000, 0x8000, CRC(2f611978) SHA1(fb60be573184d2af1dfdd543e68eeec53f2788f2) )

	ROM_REGION( 0x20000, REGION_GFX3, ROMREGION_DISPOSE )
	ROM_LOAD( "cz04.8e",    0x00000, 0x8000, CRC(643a1bd3) SHA1(b23631d96cb413808f65f3ebe8fe6539b6140606) )	/* tiles */
	/* Save 0x08000-0x0ffff to expand the previous so we can decode the thing */
	ROM_LOAD( "cz05.8f",    0x10000, 0x8000, CRC(c44570bf) SHA1(3e9b8b6b36c7f5ae016dba3987ea19a29bd5ee5b) )	/* tiles */
	ROM_LOAD( "cz06.8h",    0x18000, 0x8000, CRC(b9bb448b) SHA1(84974b1f3a5b58cd427d874f805a6dd9244c1101) )	/* tiles */

	ROM_REGION( 0x8000, REGION_GFX4, 0 )     /* background tilemaps */
	ROM_LOAD( "cz03.12d",    0x0000, 0x8000, CRC(6ce11971) SHA1(16bfa69b3ad02253e81c8110c9b840be03952790) )

	ROM_REGION( 0x0400, REGION_PROMS, 0 )
	ROM_LOAD( "cz17.prm", 0x0000, 0x0100, CRC(da31dfbc) SHA1(ac476440864f538918f7bef2e1db82fd19195f89) ) /* red */
	ROM_LOAD( "cz16.prm", 0x0100, 0x0100, CRC(51f25b4c) SHA1(bfcca57613fbb22919e00db1f6a8c7ca50faa60b) ) /* green */
	ROM_LOAD( "cz15.prm", 0x0200, 0x0100, CRC(a6168d7f) SHA1(0c7b31adcd764ce2631c3fb5c1a968b01f65e741) ) /* blue */
	ROM_LOAD( "cz14.prm", 0x0300, 0x0100, CRC(52aad300) SHA1(ff09772b930afa87e28d0628ef85a589a3d149c9) ) /* ??? */

	ROM_REGION( 0x0400, REGION_PLDS, ROMREGION_DISPOSE )
	ROM_LOAD( "pal16r4a.5c",   0x0000, 0x0104, CRC(d66aaa87) SHA1(dc29b473238ed6a9de2076c79644b613a9ba6924) )
	ROM_LOAD( "pal16r4a.5e",   0x0200, 0x0104, CRC(9a8766a7) SHA1(5f84ad9e633daeb14531ef527827ef3d9b269437) )
ROM_END

ROM_START( wexpress )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "2",       0x4000, 0x4000, CRC(ea5e5a8f) SHA1(fa92bcb6b97c2966cd330b309eba73f9c059f14e) )
	ROM_LOAD( "1",       0x8000, 0x8000, CRC(a7daae12) SHA1(a97f4bc05a3ec096d8c717bdf096f4b0e59dc2c2) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )	/* 64k for the sub cpu */
	ROM_LOAD( "cz02",    0x8000, 0x8000, CRC(552e6112) SHA1(f8412a63cab0aa47321d602f69bf534426c6aa5d) )

	ROM_REGION( 0x04000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "cz07",    0x00000, 0x4000, CRC(686bac23) SHA1(b6c96ed40e90a8ba32c2e78a65f9589d387b0254) )	/* characters */

	ROM_REGION( 0x30000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "cz09",    0x00000, 0x8000, CRC(1ed250d1) SHA1(c98b0440e4319308e683e857bbfeb6a150c76ff3) )	/* sprites */
	ROM_LOAD( "cz08",    0x08000, 0x8000, CRC(2293fc61) SHA1(bf81db375f5424396559dcf0e04d34a52f6a020a) )
	ROM_LOAD( "cz13",    0x10000, 0x8000, CRC(7c3bfd00) SHA1(87b48e09aaeacf78f3260df893b0922e25d10a5d) )
	ROM_LOAD( "cz12",    0x18000, 0x8000, CRC(ea2294c8) SHA1(bc996351921e68e6237cee2d29fee882931ce0ea) )
	ROM_LOAD( "cz11",    0x20000, 0x8000, CRC(b7418335) SHA1(e9d08ee651b9221c371e2629a757bceca7b6192b) )
	ROM_LOAD( "cz10",    0x28000, 0x8000, CRC(2f611978) SHA1(fb60be573184d2af1dfdd543e68eeec53f2788f2) )

	ROM_REGION( 0x20000, REGION_GFX3, ROMREGION_DISPOSE )
	ROM_LOAD( "4",       0x00000, 0x8000, CRC(f2e93ff0) SHA1(2e631966e1fa0b2699aa782b589d36801072ba03) )	/* tiles */
	/* Save 0x08000-0x0ffff to expand the previous so we can decode the thing */
	ROM_LOAD( "cz05",    0x10000, 0x8000, CRC(c44570bf) SHA1(3e9b8b6b36c7f5ae016dba3987ea19a29bd5ee5b) )	/* tiles */
	ROM_LOAD( "6",       0x18000, 0x8000, CRC(c3a56de5) SHA1(aefc516c6c69b12291c0bda03729910181a91a17) )	/* tiles */

	ROM_REGION( 0x8000, REGION_GFX4, 0 )     /* background tilemaps */
	ROM_LOAD( "3",        0x0000, 0x8000, CRC(242e3e64) SHA1(4fa8e93ef055bfdbe3bd619c53bf2448e1b832f0) )

	ROM_REGION( 0x0400, REGION_PROMS, 0 )
	ROM_LOAD( "cz17.prm", 0x0000, 0x0100, CRC(da31dfbc) SHA1(ac476440864f538918f7bef2e1db82fd19195f89) ) /* red */
	ROM_LOAD( "cz16.prm", 0x0100, 0x0100, CRC(51f25b4c) SHA1(bfcca57613fbb22919e00db1f6a8c7ca50faa60b) ) /* green */
	ROM_LOAD( "cz15.prm", 0x0200, 0x0100, CRC(a6168d7f) SHA1(0c7b31adcd764ce2631c3fb5c1a968b01f65e741) ) /* blue */
	ROM_LOAD( "cz14.prm", 0x0300, 0x0100, CRC(52aad300) SHA1(ff09772b930afa87e28d0628ef85a589a3d149c9) ) /* ??? */

	ROM_REGION( 0x0400, REGION_PLDS, ROMREGION_DISPOSE )
	ROM_LOAD( "pal16r4a.5c",   0x0000, 0x0104, CRC(d66aaa87) SHA1(dc29b473238ed6a9de2076c79644b613a9ba6924) )
	ROM_LOAD( "pal16r4a.5e",   0x0200, 0x0104, CRC(9a8766a7) SHA1(5f84ad9e633daeb14531ef527827ef3d9b269437) )
ROM_END

ROM_START( wexpresb )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "wexpress.3", 0x4000, 0x4000, CRC(b4dd0fa4) SHA1(8d17eb28ae92486c67859871ea2bef8f50f39dbd) )
	ROM_LOAD( "wexpress.1", 0x8000, 0x8000, CRC(e8466596) SHA1(dbbd3b84d0f017292595fc19f7412b984851221a) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )	/* 64k for the sub cpu */
	ROM_LOAD( "cz02",    0x8000, 0x8000, CRC(552e6112) SHA1(f8412a63cab0aa47321d602f69bf534426c6aa5d) )

	ROM_REGION( 0x04000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "cz07",    0x00000, 0x4000, CRC(686bac23) SHA1(b6c96ed40e90a8ba32c2e78a65f9589d387b0254) )	/* characters */

	ROM_REGION( 0x30000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "cz09",    0x00000, 0x8000, CRC(1ed250d1) SHA1(c98b0440e4319308e683e857bbfeb6a150c76ff3) )	/* sprites */
	ROM_LOAD( "cz08",    0x08000, 0x8000, CRC(2293fc61) SHA1(bf81db375f5424396559dcf0e04d34a52f6a020a) )
	ROM_LOAD( "cz13",    0x10000, 0x8000, CRC(7c3bfd00) SHA1(87b48e09aaeacf78f3260df893b0922e25d10a5d) )
	ROM_LOAD( "cz12",    0x18000, 0x8000, CRC(ea2294c8) SHA1(bc996351921e68e6237cee2d29fee882931ce0ea) )
	ROM_LOAD( "cz11",    0x20000, 0x8000, CRC(b7418335) SHA1(e9d08ee651b9221c371e2629a757bceca7b6192b) )
	ROM_LOAD( "cz10",    0x28000, 0x8000, CRC(2f611978) SHA1(fb60be573184d2af1dfdd543e68eeec53f2788f2) )

	ROM_REGION( 0x20000, REGION_GFX3, ROMREGION_DISPOSE )
	ROM_LOAD( "4",       0x00000, 0x8000, CRC(f2e93ff0) SHA1(2e631966e1fa0b2699aa782b589d36801072ba03) )	/* tiles */
	/* Save 0x08000-0x0ffff to expand the previous so we can decode the thing */
	ROM_LOAD( "cz05",    0x10000, 0x8000, CRC(c44570bf) SHA1(3e9b8b6b36c7f5ae016dba3987ea19a29bd5ee5b) )	/* tiles */
	ROM_LOAD( "6",       0x18000, 0x8000, CRC(c3a56de5) SHA1(aefc516c6c69b12291c0bda03729910181a91a17) )	/* tiles */

	ROM_REGION( 0x8000, REGION_GFX4, 0 )     /* background tilemaps */
	ROM_LOAD( "3",        0x0000, 0x8000, CRC(242e3e64) SHA1(4fa8e93ef055bfdbe3bd619c53bf2448e1b832f0) )

	ROM_REGION( 0x0400, REGION_PROMS, 0 )
	ROM_LOAD( "cz17.prm", 0x0000, 0x0100, CRC(da31dfbc) SHA1(ac476440864f538918f7bef2e1db82fd19195f89) ) /* red */
	ROM_LOAD( "cz16.prm", 0x0100, 0x0100, CRC(51f25b4c) SHA1(bfcca57613fbb22919e00db1f6a8c7ca50faa60b) ) /* green */
	ROM_LOAD( "cz15.prm", 0x0200, 0x0100, CRC(a6168d7f) SHA1(0c7b31adcd764ce2631c3fb5c1a968b01f65e741) ) /* blue */
	ROM_LOAD( "cz14.prm", 0x0300, 0x0100, CRC(52aad300) SHA1(ff09772b930afa87e28d0628ef85a589a3d149c9) ) /* ??? */
ROM_END

ROM_START( wexpresc )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "s2",      0x4000, 0x4000, CRC(40d70fcb) SHA1(1327d39f872a39e020972952e5756ca59c55f9d0) )
	ROM_LOAD( "s1",      0x8000, 0x8000, CRC(7c573824) SHA1(f5e4d4f0866c08c88d012a77e8aa2e74a779f986) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )	/* 64k for the sub cpu */
	ROM_LOAD( "cz02",    0x8000, 0x8000, CRC(552e6112) SHA1(f8412a63cab0aa47321d602f69bf534426c6aa5d) )

	ROM_REGION( 0x04000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "cz07",    0x00000, 0x4000, CRC(686bac23) SHA1(b6c96ed40e90a8ba32c2e78a65f9589d387b0254) )	/* characters */

	ROM_REGION( 0x30000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "cz09",    0x00000, 0x8000, CRC(1ed250d1) SHA1(c98b0440e4319308e683e857bbfeb6a150c76ff3) )	/* sprites */
	ROM_LOAD( "cz08",    0x08000, 0x8000, CRC(2293fc61) SHA1(bf81db375f5424396559dcf0e04d34a52f6a020a) )
	ROM_LOAD( "cz13",    0x10000, 0x8000, CRC(7c3bfd00) SHA1(87b48e09aaeacf78f3260df893b0922e25d10a5d) )
	ROM_LOAD( "cz12",    0x18000, 0x8000, CRC(ea2294c8) SHA1(bc996351921e68e6237cee2d29fee882931ce0ea) )
	ROM_LOAD( "cz11",    0x20000, 0x8000, CRC(b7418335) SHA1(e9d08ee651b9221c371e2629a757bceca7b6192b) )
	ROM_LOAD( "cz10",    0x28000, 0x8000, CRC(2f611978) SHA1(fb60be573184d2af1dfdd543e68eeec53f2788f2) )

	ROM_REGION( 0x20000, REGION_GFX3, ROMREGION_DISPOSE )
	ROM_LOAD( "4",       0x00000, 0x8000, CRC(f2e93ff0) SHA1(2e631966e1fa0b2699aa782b589d36801072ba03) )	/* tiles */
	/* Save 0x08000-0x0ffff to expand the previous so we can decode the thing */
	ROM_LOAD( "cz05",    0x10000, 0x8000, CRC(c44570bf) SHA1(3e9b8b6b36c7f5ae016dba3987ea19a29bd5ee5b) )	/* tiles */
	ROM_LOAD( "6",       0x18000, 0x8000, CRC(c3a56de5) SHA1(aefc516c6c69b12291c0bda03729910181a91a17) )	/* tiles */

	ROM_REGION( 0x8000, REGION_GFX4, 0 )     /* background tilemaps */
	ROM_LOAD( "3",        0x0000, 0x8000, CRC(242e3e64) SHA1(4fa8e93ef055bfdbe3bd619c53bf2448e1b832f0) )
	/* Proms Weren't Present In This Set, Using the One from the Other */
	ROM_REGION( 0x0400, REGION_PROMS, 0 )
	ROM_LOAD( "cz17.prm", 0x0000, 0x0100, CRC(da31dfbc) SHA1(ac476440864f538918f7bef2e1db82fd19195f89) ) /* red */
	ROM_LOAD( "cz16.prm", 0x0100, 0x0100, CRC(51f25b4c) SHA1(bfcca57613fbb22919e00db1f6a8c7ca50faa60b) ) /* green */
	ROM_LOAD( "cz15.prm", 0x0200, 0x0100, CRC(a6168d7f) SHA1(0c7b31adcd764ce2631c3fb5c1a968b01f65e741) ) /* blue */
	ROM_LOAD( "cz14.prm", 0x0300, 0x0100, CRC(52aad300) SHA1(ff09772b930afa87e28d0628ef85a589a3d149c9) ) /* ??? */
ROM_END


static void exprraid_gfx_expand(void)
{
	/* Expand the background rom so we can use regular decode routines */

	UINT8	*gfx = memory_region(REGION_GFX3);
	int				offs = 0x10000-0x1000;
	int				i;


	for ( i = 0x8000-0x1000; i >= 0; i-= 0x1000 )
	{
		memcpy( &(gfx[offs]), &(gfx[i]), 0x1000 );

		offs -= 0x1000;

		memcpy( &(gfx[offs]), &(gfx[i]), 0x1000 );

		offs -= 0x1000;
	}
}


static void patch_rom1(void)
{
	UINT8 *rom = memory_region(REGION_CPU1);
	int i;

	/* HACK!: Implement custom opcode as regular with a mapped io read */
	for ( i = 0; i < 0x10000; i++ )
	{
		/* make sure is what we want to patch */
		if ( rom[i] == 0x4b && rom[i+1] == 0x00 && rom[i+2] == 0x29 && rom[i+3] == 0x02 )
		{
			/* replace custom opcode with: LDA  $FF */
			rom[i] = 0xa5;
			i++;
			rom[i] = 0xff;
		}
	}
}

static DRIVER_INIT( wexpress )
{
	patch_rom1();
	exprraid_gfx_expand();
}

static DRIVER_INIT( exprraid )
{
	UINT8 *rom = memory_region(REGION_CPU1);


	/* decode vectors */
	rom[0xfffa] = rom[0xfff7];
	rom[0xfffb] = rom[0xfff6];

	rom[0xfffc] = rom[0xfff1];
	rom[0xfffd] = rom[0xfff0];

	rom[0xfffe] = rom[0xfff3];
	rom[0xffff] = rom[0xfff2];

	patch_rom1();
	exprraid_gfx_expand();
}

static DRIVER_INIT( wexpresb )
{
	memory_install_read8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x3800, 0x3800, 0, 0, vblank_r);
	exprraid_gfx_expand();
}

static DRIVER_INIT( wexpresc )
{
	memory_install_read8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xFFC0, 0xFFC0, 0, 0, vblank_r);
	exprraid_gfx_expand();
}


GAME( 1986, exprraid, 0,        exprraid, exprraid, exprraid, ROT0, "Data East USA", "Express Raider (US set 1)", 0 )
GAME( 1986, exprrada, exprraid, exprraid, exprraid, exprraid, ROT0, "Data East USA", "Express Raider (US set 2)", 0 )
GAME( 1986, wexpress, exprraid, exprraid, exprraid, wexpress, ROT0, "Data East Corporation", "Western Express (World?)", 0 )
GAME( 1986, wexpresb, exprraid, exprraid, exprraid, wexpresb, ROT0, "bootleg", "Western Express (bootleg set 1)", 0 )
GAME( 1986, wexpresc, exprraid, exprraid, exprraid, wexpresc, ROT0, "bootleg", "Western Express (bootleg set 2)", 0 )
