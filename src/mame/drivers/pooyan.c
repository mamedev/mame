/***************************************************************************

Notes:
- Several people claim that colors are wrong, but the way the color PROMs
  are used seems correct.


Pooyan memory map (preliminary)

driver by Allard Van Der Bas

Thanks must go to Mike Cuddy for providing information on this one.

Sound processor memory map.
0x3000-0x33ff RAM.
AY-8910 #1 : reg 0x5000
         wr  0x4000
             rd  0x4000

AY-8910 #2 : reg 0x7000
         wr  0x6000
             rd  0x6000

Main processor memory map.
0000-7fff ROM
8000-83ff color RAM
8400-87ff video RAM
8800-8fff RAM
9000-97ff sprite RAM (only areas 0x9010 and 0x9410 are used).

memory mapped ports:

read:
0xA000  Dipswitch 2 adddbtll
        a = attract mode
        ddd = difficulty 0=easy, 7=hardest.
        b = bonus setting (easy/hard)
        t = table / upright
        ll = lives: 11=3, 10=4, 01=5, 00=255.

0xA0E0  llllrrrr
        l == left coin mech, r = right coinmech.

0xA080  IN0 Port
0xA0A0  IN1 Port
0xA0C0  IN2 Port

write:
0xA100  command for the audio CPU.
0xA180  NMI enable. (0xA180 == 1 = deliver NMI to CPU).

0xA181  interrupt trigger on audio CPU.

0xA183  maybe reset sound cpu?

0xA184  ????

0xA187  Flip screen

interrupts:
standard NMI at 0x66

***************************************************************************/

#include "driver.h"
#include "audio/timeplt.h"


extern WRITE8_HANDLER( pooyan_videoram_w );
extern WRITE8_HANDLER( pooyan_colorram_w );
extern WRITE8_HANDLER( pooyan_flipscreen_w );

extern PALETTE_INIT( pooyan );
extern VIDEO_START( pooyan );
extern VIDEO_UPDATE( pooyan );


static ADDRESS_MAP_START( pooyan_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x83ff) AM_RAM AM_WRITE(pooyan_colorram_w) AM_BASE(&colorram)
	AM_RANGE(0x8400, 0x87ff) AM_RAM AM_WRITE(pooyan_videoram_w) AM_BASE(&videoram)
	AM_RANGE(0x8800, 0x8fff) AM_RAM
	AM_RANGE(0x9010, 0x903f) AM_RAM AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
	AM_RANGE(0x9410, 0x943f) AM_RAM AM_BASE(&spriteram_2)
	AM_RANGE(0xa000, 0xa000) AM_READ(input_port_4_r) AM_WRITENOP	/* DSW2 */	/* watchdog reset? */
	AM_RANGE(0xa028, 0xa028) AM_WRITENOP // ???
	AM_RANGE(0xa080, 0xa080) AM_READ(input_port_0_r)	/* IN0 */
	AM_RANGE(0xa0a0, 0xa0a0) AM_READ(input_port_1_r)	/* IN1 */
	AM_RANGE(0xa0c0, 0xa0c0) AM_READ(input_port_2_r)	/* IN2 */
	AM_RANGE(0xa0e0, 0xa0e0) AM_READ(input_port_3_r)	/* DSW1 */
	AM_RANGE(0xa100, 0xa100) AM_WRITE(soundlatch_w)
	AM_RANGE(0xa180, 0xa180) AM_WRITE(interrupt_enable_w)
	AM_RANGE(0xa181, 0xa181) AM_WRITE(timeplt_sh_irqtrigger_w)
	AM_RANGE(0xa183, 0xa183) AM_WRITENOP // ???
	AM_RANGE(0xa187, 0xa187) AM_WRITE(pooyan_flipscreen_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( pooyan )
	PORT_START	/* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* IN1 */
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_2WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* IN2 */
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* DSW0 */
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_7C ) )

	PORT_START	/* DSW1 */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "256" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x08, "50K 80K+" )
	PORT_DIPSETTING(    0x00, "30K 70K+" )
	PORT_DIPNAME( 0x70, 0x70, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x70, "1 (Easy)" )
	PORT_DIPSETTING(    0x60, "2" )
	PORT_DIPSETTING(    0x50, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x30, "5" )
	PORT_DIPSETTING(    0x20, "6" )
	PORT_DIPSETTING(    0x10, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hard)" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,	/* 8*8 characters */
	256,	/* 256 characters */
	4,	/* 4 bits per pixel */
	{ 0x1000*8+4, 0x1000*8+0, 4, 0 },
	{ 0, 1, 2, 3, 8*8+0,8*8+1,8*8+2,8*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8	/* every char takes 16 consecutive bytes */
};
static const gfx_layout spritelayout =
{
	16,16,	/* 16*16 sprites */
	64,	/* 64 sprites */
	4,	/* 4 bits per pixel */
	{ 0x1000*8+4, 0x1000*8+0, 4, 0 },
	{ 0, 1, 2, 3,  8*8+0, 8*8+1, 8*8+2, 8*8+3,
			16*8+0, 16*8+1, 16*8+2, 16*8+3,  24*8+0, 24*8+1, 24*8+2, 24*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8 },
	64*8	/* every sprite takes 64 consecutive bytes */
};



static GFXDECODE_START( pooyan )
	GFXDECODE_ENTRY( REGION_GFX1, 0, charlayout,       0, 16 )
	GFXDECODE_ENTRY( REGION_GFX2, 0, spritelayout, 16*16, 16 )
GFXDECODE_END



static MACHINE_DRIVER_START( pooyan )

	/* basic machine hardware */
	MDRV_CPU_ADD(Z80, 3072000)	/* 3.072 MHz (?) */
	MDRV_CPU_PROGRAM_MAP(pooyan_map, 0)
	MDRV_CPU_VBLANK_INT(nmi_line_pulse,1)

	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_60HZ_VBLANK_DURATION)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MDRV_GFXDECODE(pooyan)
	MDRV_PALETTE_LENGTH(32)
	MDRV_COLORTABLE_LENGTH(16*16+16*16)

	MDRV_PALETTE_INIT(pooyan)
	MDRV_VIDEO_START(pooyan)
	MDRV_VIDEO_UPDATE(pooyan)

	/* sound hardware */
	MDRV_IMPORT_FROM(timeplt_sound)
MACHINE_DRIVER_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( pooyan )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "1.4a",         0x0000, 0x2000, CRC(bb319c63) SHA1(5401b8ef586127c8cf5a431e5c44e38be2254a98) )
	ROM_LOAD( "2.5a",         0x2000, 0x2000, CRC(a1463d98) SHA1(b23cc7e61276c61a78e80fe08c7f0c8adadf2ffe) )
	ROM_LOAD( "3.6a",         0x4000, 0x2000, CRC(fe1a9e08) SHA1(5206893760f188ac71a5e6bd42561cf25fcc3d49) )
	ROM_LOAD( "4.7a",         0x6000, 0x2000, CRC(9e0f9bcc) SHA1(4d9707423ad531ac535db432e329b3d52cbb4559) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "xx.7a",        0x0000, 0x1000, CRC(fbe2b368) SHA1(5689a84ef110bdc0039ad1a6c5778e0b8eccfce0) )
	ROM_LOAD( "xx.8a",        0x1000, 0x1000, CRC(e1795b3d) SHA1(9ab4e5362f9f7d9b46b750e14b1d9d71c57be40f) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "8.10g",        0x0000, 0x1000, CRC(931b29eb) SHA1(0325c1c1fdb44e0044b82b7c79b5eeabf5c11ce7) )
	ROM_LOAD( "7.9g",         0x1000, 0x1000, CRC(bbe6d6e4) SHA1(de5447d59a99c4c08c4f40c0b7dd3c3c609c11d4) )

	ROM_REGION( 0x2000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "6.9a",         0x0000, 0x1000, CRC(b2d8c121) SHA1(189ad488869f34d7a38b82ef70eb805acfe04312) )
	ROM_LOAD( "5.8a",         0x1000, 0x1000, CRC(1097c2b6) SHA1(c815f0d27593efd23923511bdd13835456ef7f76) )

	ROM_REGION( 0x0220, REGION_PROMS, 0 )
	ROM_LOAD( "pooyan.pr1",   0x0000, 0x0020, CRC(a06a6d0e) SHA1(ae131320b66d76d4bc9108da6708f6f874b2e123) ) /* palette */
	ROM_LOAD( "pooyan.pr2",   0x0020, 0x0100, CRC(82748c0b) SHA1(9ce8eb92e482eba5a9077e9db99841d65b011346) ) /* sprites */
	ROM_LOAD( "pooyan.pr3",   0x0120, 0x0100, CRC(8cd4cd60) SHA1(e0188ecd5b53a8e6e28c1de80def676740772334) ) /* characters */
ROM_END

ROM_START( pooyans )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "ic22_a4.cpu",  0x0000, 0x2000, CRC(916ae7d7) SHA1(e96eba381e6ad228acf4b74240d618f9d0bae39d) )
	ROM_LOAD( "ic23_a5.cpu",  0x2000, 0x2000, CRC(8fe38c61) SHA1(4588f9f80a5884e056a1d429785c7331e92d5654) )
	ROM_LOAD( "ic24_a6.cpu",  0x4000, 0x2000, CRC(2660218a) SHA1(606b10a4bab2432e20471440105e04d15d384570) )
	ROM_LOAD( "ic25_a7.cpu",  0x6000, 0x2000, CRC(3d2a10ad) SHA1(962c621a19e9797b8f3d12c150aa0b90958c9498) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "xx.7a",        0x0000, 0x1000, CRC(fbe2b368) SHA1(5689a84ef110bdc0039ad1a6c5778e0b8eccfce0) )
	ROM_LOAD( "xx.8a",        0x1000, 0x1000, CRC(e1795b3d) SHA1(9ab4e5362f9f7d9b46b750e14b1d9d71c57be40f) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "ic13_g10.cpu", 0x0000, 0x1000, CRC(7433aea9) SHA1(a5ad6311f097fefb6e7b747ebe9d01d72d7755d0) )
	ROM_LOAD( "ic14_g9.cpu",  0x1000, 0x1000, CRC(87c1789e) SHA1(7637a9604a3ad4f9a27105d87252de3d923672aa) )

	ROM_REGION( 0x2000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "6.9a",         0x0000, 0x1000, CRC(b2d8c121) SHA1(189ad488869f34d7a38b82ef70eb805acfe04312) )
	ROM_LOAD( "5.8a",         0x1000, 0x1000, CRC(1097c2b6) SHA1(c815f0d27593efd23923511bdd13835456ef7f76) )

	ROM_REGION( 0x0220, REGION_PROMS, 0 )
	ROM_LOAD( "pooyan.pr1",   0x0000, 0x0020, CRC(a06a6d0e) SHA1(ae131320b66d76d4bc9108da6708f6f874b2e123) ) /* palette */
	ROM_LOAD( "pooyan.pr2",   0x0020, 0x0100, CRC(82748c0b) SHA1(9ce8eb92e482eba5a9077e9db99841d65b011346) ) /* sprites */
	ROM_LOAD( "pooyan.pr3",   0x0120, 0x0100, CRC(8cd4cd60) SHA1(e0188ecd5b53a8e6e28c1de80def676740772334) ) /* characters */
ROM_END

ROM_START( pootan )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "poo_ic22.bin", 0x0000, 0x2000, CRC(41b23a24) SHA1(366efcc45613391c1ab1514654ecac1ae3d39d0e) )
	ROM_LOAD( "poo_ic23.bin", 0x2000, 0x2000, CRC(c9d94661) SHA1(af1e818335adb4398ea0dc41be0d6399999f3946) )
	ROM_LOAD( "3.6a",         0x4000, 0x2000, CRC(fe1a9e08) SHA1(5206893760f188ac71a5e6bd42561cf25fcc3d49) )
	ROM_LOAD( "poo_ic25.bin", 0x6000, 0x2000, CRC(8ae459ef) SHA1(995eba204bbb82da20063b965bf79a64441a907a) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "xx.7a",        0x0000, 0x1000, CRC(fbe2b368) SHA1(5689a84ef110bdc0039ad1a6c5778e0b8eccfce0) )
	ROM_LOAD( "xx.8a",        0x1000, 0x1000, CRC(e1795b3d) SHA1(9ab4e5362f9f7d9b46b750e14b1d9d71c57be40f) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "poo_ic13.bin", 0x0000, 0x1000, CRC(0be802e4) SHA1(07adc17bcb7332ddc00b7c71bf4919eda80b0bdb) )
	ROM_LOAD( "poo_ic14.bin", 0x1000, 0x1000, CRC(cba29096) SHA1(b5a4cf75089cf04f7361e00074816facd57452b2) )

	ROM_REGION( 0x2000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "6.9a",         0x0000, 0x1000, CRC(b2d8c121) SHA1(189ad488869f34d7a38b82ef70eb805acfe04312) )
	ROM_LOAD( "5.8a",         0x1000, 0x1000, CRC(1097c2b6) SHA1(c815f0d27593efd23923511bdd13835456ef7f76) )

	ROM_REGION( 0x0220, REGION_PROMS, 0 )
	ROM_LOAD( "pooyan.pr1",   0x0000, 0x0020, CRC(a06a6d0e) SHA1(ae131320b66d76d4bc9108da6708f6f874b2e123) ) /* palette */
	ROM_LOAD( "pooyan.pr2",   0x0020, 0x0100, CRC(82748c0b) SHA1(9ce8eb92e482eba5a9077e9db99841d65b011346) ) /* sprites */
	ROM_LOAD( "pooyan.pr3",   0x0120, 0x0100, CRC(8cd4cd60) SHA1(e0188ecd5b53a8e6e28c1de80def676740772334) ) /* characters */
ROM_END



GAME( 1982, pooyan,  0,      pooyan, pooyan, 0, ROT270, "Konami", "Pooyan", 0 )
GAME( 1982, pooyans, pooyan, pooyan, pooyan, 0, ROT270, "[Konami] (Stern license)", "Pooyan (Stern)", 0 )
GAME( 1982, pootan,  pooyan, pooyan, pooyan, 0, ROT270, "bootleg", "Pootan", 0 )
