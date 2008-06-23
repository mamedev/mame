/***************************************************************************

Surprise Attack (Konami GX911) (c) 1990 Konami

Very similar to Parodius

driver by Nicola Salmoria

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "cpu/konami/konami.h" /* for the callback and the firq irq definition */
#include "video/konamiic.h"
#include "sound/2151intf.h"

/* prototypes */
static MACHINE_RESET( surpratk );
static void surpratk_banking( int lines );
VIDEO_START( surpratk );
VIDEO_UPDATE( surpratk );


static int videobank;
static UINT8 *ram;

static INTERRUPT_GEN( surpratk_interrupt )
{
	if (K052109_is_IRQ_enabled()) cpunum_set_input_line(machine, 0,0,HOLD_LINE);
}

static READ8_HANDLER( bankedram_r )
{
	if (videobank & 0x02)
	{
		if (videobank & 0x04)
			return paletteram[offset + 0x0800];
		else
			return paletteram[offset];
	}
	else if (videobank & 0x01)
		return K053245_r(machine,offset);
	else
		return ram[offset];
}

static WRITE8_HANDLER( bankedram_w )
{
	if (videobank & 0x02)
	{
		if (videobank & 0x04)
			paletteram_xBBBBBGGGGGRRRRR_be_w(machine,offset + 0x0800,data);
		else
			paletteram_xBBBBBGGGGGRRRRR_be_w(machine,offset,data);
	}
	else if (videobank & 0x01)
		K053245_w(machine,offset,data);
	else
		ram[offset] = data;
}

static WRITE8_HANDLER( surpratk_videobank_w )
{
logerror("%04x: videobank = %02x\n",activecpu_get_pc(),data);
	/* bit 0 = select 053245 at 0000-07ff */
	/* bit 1 = select palette at 0000-07ff */
	/* bit 2 = select palette bank 0 or 1 */
	videobank = data;
}

static WRITE8_HANDLER( surpratk_5fc0_w )
{
	if ((data & 0xf4) != 0x10) logerror("%04x: 3fc0 = %02x\n",activecpu_get_pc(),data);

	/* bit 0/1 = coin counters */
	coin_counter_w(0,data & 0x01);
	coin_counter_w(1,data & 0x02);

	/* bit 3 = enable char ROM reading through the video RAM */
	K052109_set_RMRD_line( ( data & 0x08 ) ? ASSERT_LINE : CLEAR_LINE );

	/* other bits unknown */
}


/********************************************/

static ADDRESS_MAP_START( surpratk_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_READ(bankedram_r)
	AM_RANGE(0x0800, 0x1fff) AM_READ(SMH_RAM)
	AM_RANGE(0x2000, 0x3fff) AM_READ(SMH_BANK1)			/* banked ROM */
	AM_RANGE(0x5f8c, 0x5f8c) AM_READ(input_port_0_r)
	AM_RANGE(0x5f8d, 0x5f8d) AM_READ(input_port_1_r)
	AM_RANGE(0x5f8e, 0x5f8e) AM_READ(input_port_4_r)
	AM_RANGE(0x5f8f, 0x5f8f) AM_READ(input_port_2_r)
	AM_RANGE(0x5f90, 0x5f90) AM_READ(input_port_3_r)
//  AM_RANGE(0x5f91, 0x5f91) AM_READ(YM2151_status_port_0_r)    /* ? */
	AM_RANGE(0x5fa0, 0x5faf) AM_READ(K053244_r)
	AM_RANGE(0x5fc0, 0x5fc0) AM_READ(watchdog_reset_r)
	AM_RANGE(0x4000, 0x7fff) AM_READ(K052109_r)
	AM_RANGE(0x8000, 0xffff) AM_READ(SMH_ROM)			/* ROM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( surpratk_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_WRITE(bankedram_w) AM_BASE(&ram)
	AM_RANGE(0x0800, 0x1fff) AM_WRITE(SMH_RAM)
	AM_RANGE(0x2000, 0x3fff) AM_WRITE(SMH_ROM)					/* banked ROM */
	AM_RANGE(0x5fa0, 0x5faf) AM_WRITE(K053244_w)
	AM_RANGE(0x5fb0, 0x5fbf) AM_WRITE(K053251_w)
	AM_RANGE(0x5fc0, 0x5fc0) AM_WRITE(surpratk_5fc0_w)
	AM_RANGE(0x5fd0, 0x5fd0) AM_WRITE(YM2151_register_port_0_w)
	AM_RANGE(0x5fd1, 0x5fd1) AM_WRITE(YM2151_data_port_0_w)
	AM_RANGE(0x5fc4, 0x5fc4) AM_WRITE(surpratk_videobank_w)
	AM_RANGE(0x4000, 0x7fff) AM_WRITE(K052109_w)
	AM_RANGE(0x8000, 0xffff) AM_WRITE(SMH_ROM)					/* ROM */
ADDRESS_MAP_END


/***************************************************************************

    Input Ports

***************************************************************************/

static INPUT_PORTS_START( surpratk )
	PORT_START	/* PLAYER 1 INPUTS */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START	/* PLAYER 2 INPUTS */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START	/* DSW #1 */
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
//  PORT_DIPSETTING(    0x00, "No Use" )

	PORT_START	/* DSW #2 */
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, "Difficult" )
	PORT_DIPSETTING(    0x00, "Very Difficult" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START	/* DSW #3 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Upright Controls" )
	PORT_DIPSETTING(    0x20, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Dual ) )
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END



static void irqhandler(running_machine *machine, int linestate)
{
	cpunum_set_input_line(machine, 0,KONAMI_FIRQ_LINE,linestate);
}

static const struct YM2151interface ym2151_interface =
{
	irqhandler
};



static MACHINE_DRIVER_START( surpratk )

	/* basic machine hardware */
	MDRV_CPU_ADD(KONAMI, 3000000)	/* 053248 */
	MDRV_CPU_PROGRAM_MAP(surpratk_readmem,surpratk_writemem)
	MDRV_CPU_VBLANK_INT("main", surpratk_interrupt)

	MDRV_MACHINE_RESET(surpratk)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_HAS_SHADOWS)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(14*8, (64-14)*8-1, 2*8, 30*8-1 )

	MDRV_PALETTE_LENGTH(2048)

	MDRV_VIDEO_START(surpratk)
	MDRV_VIDEO_UPDATE(surpratk)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD(YM2151, 3579545)
	MDRV_SOUND_CONFIG(ym2151_interface)
	MDRV_SOUND_ROUTE(0, "left", 1.0)
	MDRV_SOUND_ROUTE(1, "right", 1.0)
MACHINE_DRIVER_END

/***************************************************************************

  Game ROMs

***************************************************************************/


ROM_START( suratk )
	ROM_REGION( 0x51000, REGION_CPU1, 0 ) /* code + banked roms + palette RAM */
	ROM_LOAD( "911j01.f5", 0x10000, 0x20000, CRC(1e647881) SHA1(241e421d5599ebd9fcfb8be9c48dfd3b4c671958) )
	ROM_LOAD( "911k02.h5", 0x30000, 0x18000, CRC(ef10e7b6) SHA1(0b41a929c0c579d688653a8d90dd6b40db12cfb3) )
	ROM_CONTINUE(           0x08000, 0x08000 )

	ROM_REGION( 0x080000, REGION_GFX1, 0 ) /* graphics ( don't dispose as the program can read them, 0 ) */
	ROM_LOAD( "911d05.bin", 0x000000, 0x040000, CRC(308d2319) SHA1(521d2a72fecb094e2c2f23b535f0b527886b4d3a) ) /* characters */
	ROM_LOAD( "911d06.bin", 0x040000, 0x040000, CRC(91cc9b32) SHA1(e05b7bbff30f24fe6f009560410f5e90bb118692) ) /* characters */

	ROM_REGION( 0x080000, REGION_GFX2, 0 ) /* graphics ( don't dispose as the program can read them, 0 ) */
	ROM_LOAD( "911d03.bin", 0x000000, 0x040000, CRC(e34ff182) SHA1(075ca7a91c843bdac7da21ddfcd43f7a043a09b6) )	/* sprites */
	ROM_LOAD( "911d04.bin", 0x040000, 0x040000, CRC(20700bd2) SHA1(a2fa4a3ee28c1542cdd798907a9ece249aadff0a) )	/* sprites */
ROM_END

ROM_START( suratka )
	ROM_REGION( 0x51000, REGION_CPU1, 0 ) /* code + banked roms + palette RAM */
	ROM_LOAD( "911j01.f5", 0x10000, 0x20000, CRC(1e647881) SHA1(241e421d5599ebd9fcfb8be9c48dfd3b4c671958) )
	ROM_LOAD( "911l02.h5", 0x30000, 0x18000, CRC(11db8288) SHA1(09fe187855172ebf0c57f561cce7f41e47f53114) )
	ROM_CONTINUE(           0x08000, 0x08000 )

	ROM_REGION( 0x080000, REGION_GFX1, 0 ) /* graphics ( don't dispose as the program can read them, 0 ) */
	ROM_LOAD( "911d05.bin", 0x000000, 0x040000, CRC(308d2319) SHA1(521d2a72fecb094e2c2f23b535f0b527886b4d3a) ) /* characters */
	ROM_LOAD( "911d06.bin", 0x040000, 0x040000, CRC(91cc9b32) SHA1(e05b7bbff30f24fe6f009560410f5e90bb118692) ) /* characters */

	ROM_REGION( 0x080000, REGION_GFX2, 0 ) /* graphics ( don't dispose as the program can read them, 0 ) */
	ROM_LOAD( "911d03.bin", 0x000000, 0x040000, CRC(e34ff182) SHA1(075ca7a91c843bdac7da21ddfcd43f7a043a09b6) )	/* sprites */
	ROM_LOAD( "911d04.bin", 0x040000, 0x040000, CRC(20700bd2) SHA1(a2fa4a3ee28c1542cdd798907a9ece249aadff0a) )	/* sprites */
ROM_END

ROM_START( suratkj )
	ROM_REGION( 0x51000, REGION_CPU1, 0 ) /* code + banked roms + palette RAM */
	ROM_LOAD( "911m01.f5", 0x10000, 0x20000, CRC(ee5b2cc8) SHA1(4b05f7ba4e804a3bccb41fe9d3258cbcfe5324aa) )
	ROM_LOAD( "911m02.h5", 0x30000, 0x18000, CRC(5d4148a8) SHA1(4fa5947db777b4c742775d588dea38758812a916) )
	ROM_CONTINUE(           0x08000, 0x08000 )

	ROM_REGION( 0x080000, REGION_GFX1, 0 ) /* graphics ( don't dispose as the program can read them, 0 ) */
	ROM_LOAD( "911d05.bin", 0x000000, 0x040000, CRC(308d2319) SHA1(521d2a72fecb094e2c2f23b535f0b527886b4d3a) ) /* characters */
	ROM_LOAD( "911d06.bin", 0x040000, 0x040000, CRC(91cc9b32) SHA1(e05b7bbff30f24fe6f009560410f5e90bb118692) ) /* characters */

	ROM_REGION( 0x080000, REGION_GFX2, 0 ) /* graphics ( don't dispose as the program can read them, 0 ) */
	ROM_LOAD( "911d03.bin", 0x000000, 0x040000, CRC(e34ff182) SHA1(075ca7a91c843bdac7da21ddfcd43f7a043a09b6) )	/* sprites */
	ROM_LOAD( "911d04.bin", 0x040000, 0x040000, CRC(20700bd2) SHA1(a2fa4a3ee28c1542cdd798907a9ece249aadff0a) )	/* sprites */
ROM_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

static void surpratk_banking(int lines)
{
	UINT8 *RAM = memory_region(Machine, REGION_CPU1);
	int offs = 0;

logerror("%04x: setlines %02x\n",activecpu_get_pc(),lines);

	offs = 0x10000 + ((lines & 0x1f) * 0x2000);
	if (offs >= 0x48000) offs -= 0x40000;
	memory_set_bankptr(1,&RAM[offs]);
}

static MACHINE_RESET( surpratk )
{
	cpunum_set_info_fct(0, CPUINFO_PTR_KONAMI_SETLINES_CALLBACK, (genf *)surpratk_banking);

	paletteram = &memory_region(machine, REGION_CPU1)[0x48000];
}

static DRIVER_INIT( surpratk )
{
	konami_rom_deinterleave_2(REGION_GFX1);
	konami_rom_deinterleave_2(REGION_GFX2);
}


GAME( 1990, suratk,  0,      surpratk, surpratk, surpratk, ROT0, "Konami", "Surprise Attack (World ver. K)", 0 )
GAME( 1990, suratka, suratk, surpratk, surpratk, surpratk, ROT0, "Konami", "Surprise Attack (Asia ver. L)", 0 )
GAME( 1990, suratkj, suratk, surpratk, surpratk, surpratk, ROT0, "Konami", "Surprise Attack (Japan ver. M)", 0 )
