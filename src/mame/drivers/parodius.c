/***************************************************************************

Parodius (Konami GX955) (c) 1990 Konami

driver by Nicola Salmoria

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "cpu/konami/konami.h" /* for the callback and the firq irq definition */
#include "video/konamiic.h"
#include "sound/2151intf.h"
#include "sound/k053260.h"

/* prototypes */
static MACHINE_RESET( parodius );
static void parodius_banking( int lines );
VIDEO_START( parodius );
VIDEO_UPDATE( parodius );

static int videobank;
static UINT8 *ram;

static INTERRUPT_GEN( parodius_interrupt )
{
	if (K052109_is_IRQ_enabled()) cpunum_set_input_line(machine, 0, 0, HOLD_LINE);
}

static READ8_HANDLER( bankedram_r )
{
	if (videobank & 0x01)
	{
		if (videobank & 0x04)
			return paletteram[offset + 0x0800];
		else
			return paletteram[offset];
	}
	else
		return ram[offset];
}

static WRITE8_HANDLER( bankedram_w )
{
	if (videobank & 0x01)
	{
		if (videobank & 0x04)
			paletteram_xBBBBBGGGGGRRRRR_be_w(machine,offset + 0x0800,data);
		else
			paletteram_xBBBBBGGGGGRRRRR_be_w(machine,offset,data);
	}
	else
		ram[offset] = data;
}

static READ8_HANDLER( parodius_052109_053245_r )
{
	if (videobank & 0x02)
		return K053245_r(machine,offset);
	else
		return K052109_r(machine,offset);
}

static WRITE8_HANDLER( parodius_052109_053245_w )
{
	if (videobank & 0x02)
		K053245_w(machine,offset,data);
	else
		K052109_w(machine,offset,data);
}

static WRITE8_HANDLER( parodius_videobank_w )
{
	if (videobank & 0xf8) logerror("%04x: videobank = %02x\n",activecpu_get_pc(),data);

	/* bit 0 = select palette or work RAM at 0000-07ff */
	/* bit 1 = select 052109 or 053245 at 2000-27ff */
	/* bit 2 = select palette bank 0 or 1 */
	videobank = data;
}

static WRITE8_HANDLER( parodius_3fc0_w )
{
	if ((data & 0xf4) != 0x10) logerror("%04x: 3fc0 = %02x\n",activecpu_get_pc(),data);

	/* bit 0/1 = coin counters */
	coin_counter_w(0,data & 0x01);
	coin_counter_w(1,data & 0x02);

	/* bit 3 = enable char ROM reading through the video RAM */
	K052109_set_RMRD_line( ( data & 0x08 ) ? ASSERT_LINE : CLEAR_LINE );

	/* other bits unknown */
}

static READ8_HANDLER( parodius_sound_r )
{
	return k053260_0_r(machine,2 + offset);
}

static WRITE8_HANDLER( parodius_sh_irqtrigger_w )
{
	cpunum_set_input_line_and_vector(machine, 1,0,HOLD_LINE,0xff);
}

#if 0
static int nmi_enabled;

static void sound_nmi_callback( int param )
{
	cpunum_set_input_line(Machine, 1, INPUT_LINE_NMI, ( nmi_enabled ) ? CLEAR_LINE : ASSERT_LINE );

	nmi_enabled = 0;
}
#endif

static TIMER_CALLBACK( nmi_callback )
{
	cpunum_set_input_line(machine, 1, INPUT_LINE_NMI, ASSERT_LINE);
}

static WRITE8_HANDLER( sound_arm_nmi_w )
{
//  sound_nmi_enabled = 1;
	cpunum_set_input_line(machine, 1, INPUT_LINE_NMI, CLEAR_LINE);
	timer_set(ATTOTIME_IN_USEC(50), NULL,0,nmi_callback);	/* kludge until the K053260 is emulated correctly */
}

/********************************************/

static ADDRESS_MAP_START( parodius_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_READ(bankedram_r)
	AM_RANGE(0x0800, 0x1fff) AM_READ(SMH_RAM)
	AM_RANGE(0x3f8c, 0x3f8c) AM_READ(input_port_0_r)
	AM_RANGE(0x3f8d, 0x3f8d) AM_READ(input_port_1_r)
	AM_RANGE(0x3f8e, 0x3f8e) AM_READ(input_port_4_r)
	AM_RANGE(0x3f8f, 0x3f8f) AM_READ(input_port_2_r)
	AM_RANGE(0x3f90, 0x3f90) AM_READ(input_port_3_r)
	AM_RANGE(0x3fa0, 0x3faf) AM_READ(K053244_r)
	AM_RANGE(0x3fc0, 0x3fc0) AM_READ(watchdog_reset_r)
	AM_RANGE(0x3fcc, 0x3fcd) AM_READ(parodius_sound_r)	/* K053260 */
	AM_RANGE(0x2000, 0x27ff) AM_READ(parodius_052109_053245_r)
	AM_RANGE(0x2000, 0x5fff) AM_READ(K052109_r)
	AM_RANGE(0x6000, 0x9fff) AM_READ(SMH_BANK1)			/* banked ROM */
	AM_RANGE(0xa000, 0xffff) AM_READ(SMH_ROM)			/* ROM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( parodius_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_WRITE(bankedram_w) AM_BASE(&ram)
	AM_RANGE(0x0800, 0x1fff) AM_WRITE(SMH_RAM)
	AM_RANGE(0x3fa0, 0x3faf) AM_WRITE(K053244_w)
	AM_RANGE(0x3fb0, 0x3fbf) AM_WRITE(K053251_w)
	AM_RANGE(0x3fc0, 0x3fc0) AM_WRITE(parodius_3fc0_w)
	AM_RANGE(0x3fc4, 0x3fc4) AM_WRITE(parodius_videobank_w)
	AM_RANGE(0x3fc8, 0x3fc8) AM_WRITE(parodius_sh_irqtrigger_w)
	AM_RANGE(0x3fcc, 0x3fcd) AM_WRITE(k053260_0_w)
	AM_RANGE(0x2000, 0x27ff) AM_WRITE(parodius_052109_053245_w)
	AM_RANGE(0x2000, 0x5fff) AM_WRITE(K052109_w)
	AM_RANGE(0x6000, 0x9fff) AM_WRITE(SMH_ROM)					/* banked ROM */
	AM_RANGE(0xa000, 0xffff) AM_WRITE(SMH_ROM)					/* ROM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( parodius_readmem_sound, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xefff) AM_READ(SMH_ROM)
	AM_RANGE(0xf000, 0xf7ff) AM_READ(SMH_RAM)
	AM_RANGE(0xf801, 0xf801) AM_READ(ym2151_status_port_0_r)
	AM_RANGE(0xfc00, 0xfc2f) AM_READ(k053260_0_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( parodius_writemem_sound, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xefff) AM_WRITE(SMH_ROM)
	AM_RANGE(0xf000, 0xf7ff) AM_WRITE(SMH_RAM)
	AM_RANGE(0xf800, 0xf800) AM_WRITE(ym2151_register_port_0_w)
	AM_RANGE(0xf801, 0xf801) AM_WRITE(ym2151_data_port_0_w)
	AM_RANGE(0xfa00, 0xfa00) AM_WRITE(sound_arm_nmi_w)
	AM_RANGE(0xfc00, 0xfc2f) AM_WRITE(k053260_0_w)
ADDRESS_MAP_END

/***************************************************************************

    Input Ports

***************************************************************************/

static INPUT_PORTS_START( parodius )
	PORT_START("P1")	/* PLAYER 1 INPUTS */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)		// power-up
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)		// shoot
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)		// missile

	PORT_START("P2")	/* PLAYER 2 INPUTS */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_START("DSW1")	/* DSW #1 */
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

	PORT_START("DSW2")	/* DSW #2 */
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x18, "20000 80000" )
	PORT_DIPSETTING(    0x10, "30000 100000" )
	PORT_DIPSETTING(    0x08, "20000" )
	PORT_DIPSETTING(    0x00, "70000" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, "Difficult" )
	PORT_DIPSETTING(    0x00, "Very Difficult" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")	/* DSW #3 */
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



/***************************************************************************

    Machine Driver

***************************************************************************/

static MACHINE_DRIVER_START( parodius )

	/* basic machine hardware */
	MDRV_CPU_ADD("main", KONAMI, 3000000)		/* 053248 */
	MDRV_CPU_PROGRAM_MAP(parodius_readmem,parodius_writemem)
	MDRV_CPU_VBLANK_INT("main", parodius_interrupt)

	MDRV_CPU_ADD("audio", Z80, 3579545)
	MDRV_CPU_PROGRAM_MAP(parodius_readmem_sound,parodius_writemem_sound)
								/* NMIs are triggered by the 053260 */

	MDRV_MACHINE_RESET(parodius)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_HAS_SHADOWS)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(14*8, (64-14)*8-1, 2*8, 30*8-1 )

	MDRV_PALETTE_LENGTH(2048)

	MDRV_VIDEO_START(parodius)
	MDRV_VIDEO_UPDATE(parodius)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD("ym", YM2151, 3579545)
	MDRV_SOUND_ROUTE(0, "left", 1.0)
	MDRV_SOUND_ROUTE(1, "right", 1.0)

	MDRV_SOUND_ADD("konami", K053260, 3579545)
	MDRV_SOUND_ROUTE(0, "left", 0.70)
	MDRV_SOUND_ROUTE(1, "right", 0.70)
MACHINE_DRIVER_END

/***************************************************************************

  Game ROMs

***************************************************************************/

ROM_START( parodius )
	ROM_REGION( 0x51000, "main", 0 ) /* code + banked roms + palette RAM */
	ROM_LOAD( "955l01.bin", 0x10000, 0x20000, CRC(49a658eb) SHA1(dd53060c4da99b8e1f896ebfec572296ef2b5665) )
	ROM_LOAD( "955l02.bin", 0x30000, 0x18000, CRC(161d7322) SHA1(a752f28c19c58263680221ad1119f2fd57df4723) )
	ROM_CONTINUE(           0x08000, 0x08000 )

	ROM_REGION( 0x10000, "audio", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "955e03.bin", 0x0000, 0x10000, CRC(940aa356) SHA1(e7466f049be48861fd2d929eed786bd48782b5bb) )

	ROM_REGION( 0x100000, "gfx1", 0 ) /* graphics ( don't dispose as the program can read them, 0 ) */
	ROM_LOAD( "955d07.bin", 0x000000, 0x080000, CRC(89473fec) SHA1(0da18c4b078c3a30233a6f5c2b90032168136f58) ) /* characters */
	ROM_LOAD( "955d08.bin", 0x080000, 0x080000, CRC(43d5cda1) SHA1(2c51bad4857d1d31456c6dc1e7d41326ea35468b) ) /* characters */

	ROM_REGION( 0x100000, "gfx2", 0 ) /* graphics ( don't dispose as the program can read them, 0 ) */
	ROM_LOAD( "955d05.bin", 0x000000, 0x080000, CRC(7a1e55e0) SHA1(7a0e04ebde28d1e7b60aef3de926dc0e78662b1e) )	/* sprites */
	ROM_LOAD( "955d06.bin", 0x080000, 0x080000, CRC(f4252875) SHA1(490f2e19b30cf8724e4b03b8d9f089c470ec13bd) )	/* sprites */

	ROM_REGION( 0x80000, "konami", 0 ) /* 053260 samples */
	ROM_LOAD( "955d04.bin", 0x00000, 0x80000, CRC(e671491a) SHA1(79e71cb5212eb7d14d3479b0734ea0270473a66d) )
ROM_END

ROM_START( parodisj )
	ROM_REGION( 0x51000, "main", 0 ) /* code + banked roms + palette RAM */
	ROM_LOAD( "955e01.bin", 0x10000, 0x20000, CRC(49baa334) SHA1(8902fbb2228111b15de6537bd168241933df134d) )
	ROM_LOAD( "955e02.bin", 0x30000, 0x18000, CRC(14010d6f) SHA1(69fe162ea08c3bd4b3e78e9d10d278bd15444af4) )
	ROM_CONTINUE(           0x08000, 0x08000 )

	ROM_REGION( 0x10000, "audio", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "955e03.bin", 0x0000, 0x10000, CRC(940aa356) SHA1(e7466f049be48861fd2d929eed786bd48782b5bb) )

	ROM_REGION( 0x100000, "gfx1", 0 ) /* graphics ( don't dispose as the program can read them, 0 ) */
	ROM_LOAD( "955d07.bin", 0x000000, 0x080000, CRC(89473fec) SHA1(0da18c4b078c3a30233a6f5c2b90032168136f58) ) /* characters */
	ROM_LOAD( "955d08.bin", 0x080000, 0x080000, CRC(43d5cda1) SHA1(2c51bad4857d1d31456c6dc1e7d41326ea35468b) ) /* characters */

	ROM_REGION( 0x100000, "gfx2", 0 ) /* graphics ( don't dispose as the program can read them, 0 ) */
	ROM_LOAD( "955d05.bin", 0x000000, 0x080000, CRC(7a1e55e0) SHA1(7a0e04ebde28d1e7b60aef3de926dc0e78662b1e) )	/* sprites */
	ROM_LOAD( "955d06.bin", 0x080000, 0x080000, CRC(f4252875) SHA1(490f2e19b30cf8724e4b03b8d9f089c470ec13bd) )	/* sprites */

	ROM_REGION( 0x80000, "konami", 0 ) /* 053260 samples */
	ROM_LOAD( "955d04.bin", 0x00000, 0x80000, CRC(e671491a) SHA1(79e71cb5212eb7d14d3479b0734ea0270473a66d) )
ROM_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

static void parodius_banking(int lines)
{
	UINT8 *RAM = memory_region(Machine, "main");
	int offs = 0;

	if (lines & 0xf0) logerror("%04x: setlines %02x\n",activecpu_get_pc(),lines);

	offs = 0x10000 + (((lines & 0x0f)^0x0f) * 0x4000);
	if (offs >= 0x48000) offs -= 0x40000;
	memory_set_bankptr( 1, &RAM[offs] );
}

static MACHINE_RESET( parodius )
{
	UINT8 *RAM = memory_region(machine, "main");

	cpunum_set_info_fct(0, CPUINFO_PTR_KONAMI_SETLINES_CALLBACK, (genf *)parodius_banking);

	paletteram = &memory_region(machine, "main")[0x48000];

	videobank = 0;

	/* init the default bank */
	memory_set_bankptr(1,&RAM[0x10000]);
}


static DRIVER_INIT( parodius )
{
	konami_rom_deinterleave_2("gfx1");
	konami_rom_deinterleave_2("gfx2");
}



GAME( 1990, parodius, 0,        parodius, parodius, parodius, ROT0, "Konami", "Parodius DA! (World)", 0 )
GAME( 1990, parodisj, parodius, parodius, parodius, parodius, ROT0, "Konami", "Parodius DA! (Japan)", 0 )
