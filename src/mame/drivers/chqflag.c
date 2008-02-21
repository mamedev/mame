/***************************************************************************

Chequered Flag / Checkered Flag (GX717) (c) Konami 1988

Notes:
- Position counter doesn't behave correctly because of the K051733 protection.
- 007232 volume & panning control is almost certainly wrong.

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "cpu/z80/z80.h"
#include "cpu/konami/konami.h"
#include "video/konamiic.h"
#include "sound/2151intf.h"
#include "sound/k007232.h"

static int K051316_readroms;

static WRITE8_HANDLER( k007232_extvolume_w );

/* from video/chqflag.c */
VIDEO_START( chqflag );
VIDEO_UPDATE( chqflag );


static INTERRUPT_GEN( chqflag_interrupt )
{
	if (cpu_getiloops() == 0)
	{
		if (K051960_is_IRQ_enabled())
			cpunum_set_input_line(machine, 0, KONAMI_IRQ_LINE, HOLD_LINE);
	}
	else if (cpu_getiloops() % 2)
	{
		if (K051960_is_NMI_enabled())
			cpunum_set_input_line(machine, 0, INPUT_LINE_NMI, PULSE_LINE);
	}
}

static WRITE8_HANDLER( chqflag_bankswitch_w )
{
	int bankaddress;
	UINT8 *RAM = memory_region(REGION_CPU1);

	/* bits 0-4 = ROM bank # (0x00-0x11) */
	bankaddress = 0x10000 + (data & 0x1f)*0x4000;
	memory_set_bankptr(4,&RAM[bankaddress]);

	/* bit 5 = memory bank select */
	if (data & 0x20)
	{
		memory_install_readwrite8_handler(0, ADDRESS_SPACE_PROGRAM, 0x1800, 0x1fff, 0, 0, MRA8_BANK5, paletteram_xBBBBBGGGGGRRRRR_be_w);
		memory_set_bankptr(5, paletteram);

		if (K051316_readroms)
			memory_install_readwrite8_handler(0, ADDRESS_SPACE_PROGRAM, 0x1000, 0x17ff, 0, 0, K051316_rom_0_r, K051316_0_w);	/* 051316 #1 (ROM test) */
		else
			memory_install_readwrite8_handler(0, ADDRESS_SPACE_PROGRAM, 0x1000, 0x17ff, 0, 0, K051316_0_r, K051316_0_w);		/* 051316 #1 */
	}
	else
	{
		memory_install_readwrite8_handler(0, ADDRESS_SPACE_PROGRAM, 0x1000, 0x17ff, 0, 0, MRA8_BANK1, MWA8_BANK1);				/* RAM */
		memory_install_readwrite8_handler(0, ADDRESS_SPACE_PROGRAM, 0x1800, 0x1fff, 0, 0, MRA8_BANK2, MWA8_BANK2);				/* RAM */
	}

	/* other bits unknown/unused */
}

static WRITE8_HANDLER( chqflag_vreg_w )
{
	static int last;

	/* bits 0 & 1 = coin counters */
	coin_counter_w(1,data & 0x01);
	coin_counter_w(0,data & 0x02);

	/* bit 4 = enable rom reading thru K051316 #1 & #2 */
	if ((K051316_readroms = (data & 0x10))){
		memory_install_read8_handler(0, ADDRESS_SPACE_PROGRAM, 0x2800, 0x2fff, 0, 0, K051316_rom_1_r);	/* 051316 (ROM test) */
	}
	else{
		memory_install_read8_handler(0, ADDRESS_SPACE_PROGRAM, 0x2800, 0x2fff, 0, 0, K051316_1_r);		/* 051316 */
	}

	/* Bits 3-7 probably control palette dimming in a similar way to TMNT2/Saunset Riders, */
	/* however I don't have enough evidence to determine the exact behaviour. */
	/* Bits 3 and 7 are set in night stages, where the background should get darker and */
	/* the headlight (which have the shadow bit set) become highlights */
	/* Maybe one of the bits inverts the SHAD line while the other darkens the background. */
	if (data & 0x08)
		palette_set_shadow_factor(Machine,1/PALETTE_DEFAULT_SHADOW_FACTOR);
	else
		palette_set_shadow_factor(Machine,PALETTE_DEFAULT_SHADOW_FACTOR);

	if ((data & 0x80) != last)
	{
		double brt = (data & 0x80) ? PALETTE_DEFAULT_SHADOW_FACTOR : 1.0;
		int i;

		last = data & 0x80;

		/* only affect the background */
		for (i = 512;i < 1024;i++)
			palette_set_brightness(Machine,i,brt);
	}

//if ((data & 0xf8) && (data & 0xf8) != 0x88)
//  popmessage("chqflag_vreg_w %02x",data);


	/* other bits unknown. bit 5 is used. */
}

static int analog_ctrl;

static WRITE8_HANDLER( select_analog_ctrl_w )
{
	analog_ctrl = data;
}

static READ8_HANDLER( analog_read_r )
{
	static int accel, wheel;

	switch (analog_ctrl & 0x03){
		case 0x00: return (accel = readinputport(5));	/* accelerator */
		case 0x01: return (wheel = readinputport(6));	/* steering */
		case 0x02: return accel;						/* accelerator (previous?) */
		case 0x03: return wheel;						/* steering (previous?) */
	}

	return 0xff;
}

static WRITE8_HANDLER( chqflag_sh_irqtrigger_w )
{
	cpunum_set_input_line(Machine, 1,0,HOLD_LINE);
}


/****************************************************************************/

static ADDRESS_MAP_START( chqflag_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_READ(MRA8_RAM)					/* RAM */
	AM_RANGE(0x1000, 0x17ff) AM_READ(MRA8_BANK1)					/* banked RAM (RAM/051316 (chip 1)) */
	AM_RANGE(0x1800, 0x1fff) AM_READ(MRA8_BANK2)					/* palette + RAM */
	AM_RANGE(0x2000, 0x2007) AM_READ(K051937_r)					/* Sprite control registers */
	AM_RANGE(0x2400, 0x27ff) AM_READ(K051960_r)					/* Sprite RAM */
	AM_RANGE(0x2800, 0x2fff) AM_READ(MRA8_BANK3)					/* 051316 zoom/rotation (chip 2) */
	AM_RANGE(0x3100, 0x3100) AM_READ(input_port_0_r)				/* DIPSW #1  */
	AM_RANGE(0x3200, 0x3200) AM_READ(input_port_3_r)				/* COINSW, STARTSW, test mode */
	AM_RANGE(0x3201, 0x3201) AM_READ(input_port_2_r)				/* DIPSW #3, SW 4 */
	AM_RANGE(0x3203, 0x3203) AM_READ(input_port_1_r)				/* DIPSW #2 */
	AM_RANGE(0x3400, 0x341f) AM_READ(K051733_r)					/* 051733 (protection) */
	AM_RANGE(0x3701, 0x3701) AM_READ(input_port_4_r)				/* Brake + Shift + ? */
	AM_RANGE(0x3702, 0x3702) AM_READ(analog_read_r)				/* accelerator/wheel */
	AM_RANGE(0x4000, 0x7fff) AM_READ(MRA8_BANK4)					/* banked ROM */
	AM_RANGE(0x8000, 0xffff) AM_READ(MRA8_ROM)					/* ROM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( chqflag_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_WRITE(MWA8_RAM)					/* RAM */
	AM_RANGE(0x1000, 0x17ff) AM_WRITE(MWA8_BANK1)				/* banked RAM (RAM/051316 (chip 1)) */
	AM_RANGE(0x1800, 0x1fff) AM_WRITE(MWA8_BANK2)					/* palette + RAM */
	AM_RANGE(0x2000, 0x2007) AM_WRITE(K051937_w)					/* Sprite control registers */
	AM_RANGE(0x2400, 0x27ff) AM_WRITE(K051960_w)					/* Sprite RAM */
	AM_RANGE(0x2800, 0x2fff) AM_WRITE(K051316_1_w)				/* 051316 zoom/rotation (chip 2) */
	AM_RANGE(0x3000, 0x3000) AM_WRITE(soundlatch_w)				/* sound code # */
	AM_RANGE(0x3001, 0x3001) AM_WRITE(chqflag_sh_irqtrigger_w)	/* cause interrupt on audio CPU */
	AM_RANGE(0x3002, 0x3002) AM_WRITE(chqflag_bankswitch_w)		/* bankswitch control */
	AM_RANGE(0x3003, 0x3003) AM_WRITE(chqflag_vreg_w)				/* enable K051316 ROM reading */
	AM_RANGE(0x3300, 0x3300) AM_WRITE(watchdog_reset_w)			/* watchdog timer */
	AM_RANGE(0x3400, 0x341f) AM_WRITE(K051733_w)					/* 051733 (protection) */
	AM_RANGE(0x3500, 0x350f) AM_WRITE(K051316_ctrl_0_w)			/* 051316 control registers (chip 1) */
	AM_RANGE(0x3600, 0x360f) AM_WRITE(K051316_ctrl_1_w)			/* 051316 control registers (chip 2) */
	AM_RANGE(0x3700, 0x3700) AM_WRITE(select_analog_ctrl_w)		/* select accelerator/wheel */
	AM_RANGE(0x3702, 0x3702) AM_WRITE(select_analog_ctrl_w)		/* select accelerator/wheel (mirror?) */
	AM_RANGE(0x4000, 0x7fff) AM_WRITE(MWA8_ROM)					/* banked ROM */
	AM_RANGE(0x8000, 0xffff) AM_WRITE(MWA8_ROM)					/* ROM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( chqflag_readmem_sound, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_READ(MRA8_ROM)				/* ROM */
	AM_RANGE(0x8000, 0x87ff) AM_READ(MRA8_RAM)				/* RAM */
	AM_RANGE(0xa000, 0xa00d) AM_READ(K007232_read_port_0_r)	/* 007232 (chip 1) */
	AM_RANGE(0xb000, 0xb00d) AM_READ(K007232_read_port_1_r)	/* 007232 (chip 2) */
	AM_RANGE(0xc001, 0xc001) AM_READ(YM2151_status_port_0_r)	/* YM2151 */
	AM_RANGE(0xd000, 0xd000) AM_READ(soundlatch_r)			/* soundlatch_r */
	//AM_RANGE(0xe000, 0xe000) AM_READ(MRA8_NOP)                /* ??? */
ADDRESS_MAP_END

static WRITE8_HANDLER( k007232_bankswitch_w )
{
	int bank_A, bank_B;

	/* banks # for the 007232 (chip 1) */
	bank_A = ((data >> 4) & 0x03);
	bank_B = ((data >> 6) & 0x03);
	K007232_set_bank( 0, bank_A, bank_B );

	/* banks # for the 007232 (chip 2) */
	bank_A = ((data >> 0) & 0x03);
	bank_B = ((data >> 2) & 0x03);
	K007232_set_bank( 1, bank_A, bank_B );
}

static ADDRESS_MAP_START( chqflag_writemem_sound, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_WRITE(MWA8_ROM)					/* ROM */
	AM_RANGE(0x8000, 0x87ff) AM_WRITE(MWA8_RAM)					/* RAM */
	AM_RANGE(0x9000, 0x9000) AM_WRITE(k007232_bankswitch_w)		/* 007232 bankswitch */
	AM_RANGE(0xa000, 0xa00d) AM_WRITE(K007232_write_port_0_w)		/* 007232 (chip 1) */
	AM_RANGE(0xa01c, 0xa01c) AM_WRITE(k007232_extvolume_w)/* extra volume, goes to the 007232 w/ A11 */
											/* selecting a different latch for the external port */
	AM_RANGE(0xb000, 0xb00d) AM_WRITE(K007232_write_port_1_w)		/* 007232 (chip 2) */
	AM_RANGE(0xc000, 0xc000) AM_WRITE(YM2151_register_port_0_w)	/* YM2151 */
	AM_RANGE(0xc001, 0xc001) AM_WRITE(YM2151_data_port_0_w)		/* YM2151 */
	AM_RANGE(0xf000, 0xf000) AM_WRITE(MWA8_NOP)					/* ??? */
ADDRESS_MAP_END


static INPUT_PORTS_START( chqflag )
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
//  PORT_DIPSETTING(    0x00, "Coin Slot 2 Invalidity" )

	PORT_START	/* DSW #2 (according to the manual SW1 thru SW5 are not used) */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unused ) )
	PORT_DIPSETTING(	0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unused ) )
	PORT_DIPSETTING(	0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) )
	PORT_DIPSETTING(	0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) )
	PORT_DIPSETTING(	0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) )
	PORT_DIPSETTING(	0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(	0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(	0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(	0x20, "Difficult" )
	PORT_DIPSETTING(	0x00, "Very difficult" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(	0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )

	PORT_START
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )	/* DIPSW #3 - SW4 */
	PORT_DIPSETTING(	0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )

	PORT_START
	/* COINSW + STARTSW */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	/* DIPSW #3 */
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) )
	PORT_DIPSETTING(	0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Title" )
	PORT_DIPSETTING(	0x40, "Chequered Flag" )
	PORT_DIPSETTING(	0x00, "Checkered Flag" )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START	/* Brake, Shift + ??? */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* if this is set, it goes directly to test mode */
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* if bit 7 == 0, the game resets */

	PORT_START	/* Accelerator */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(5)

	PORT_START	/* Driving wheel */
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x10,0xef) PORT_SENSITIVITY(80) PORT_KEYDELTA(8)
INPUT_PORTS_END



static void chqflag_ym2151_irq_w(int data)
{
	cpunum_set_input_line(Machine, 1,INPUT_LINE_NMI,PULSE_LINE);
}


static const struct YM2151interface ym2151_interface =
{
	chqflag_ym2151_irq_w
};

static void volume_callback0(int v)
{
	K007232_set_volume(0,0,(v & 0x0f)*0x11,0);
	K007232_set_volume(0,1,0,(v >> 4)*0x11);
}

static WRITE8_HANDLER( k007232_extvolume_w )
{
	K007232_set_volume(1,1,(data & 0x0f)*0x11/2,(data >> 4)*0x11/2);
}

static void volume_callback1(int v)
{
	K007232_set_volume(1,0,(v & 0x0f)*0x11/2,(v >> 4)*0x11/2);
}

static const struct K007232_interface k007232_interface_1 =
{
	REGION_SOUND1,
	volume_callback0
};

static const struct K007232_interface k007232_interface_2 =
{
	REGION_SOUND2,
	volume_callback1
};

static MACHINE_DRIVER_START( chqflag )

	/* basic machine hardware */
	MDRV_CPU_ADD(KONAMI,XTAL_24MHz/8)	/* 052001 (verified on pcb) */
	MDRV_CPU_PROGRAM_MAP(chqflag_readmem,chqflag_writemem)
	MDRV_CPU_VBLANK_INT(chqflag_interrupt,16)	/* ? */

	MDRV_CPU_ADD(Z80, XTAL_3_579545MHz) /* verified on pcb */
	/* audio CPU */	/* ? */
	MDRV_CPU_PROGRAM_MAP(chqflag_readmem_sound,chqflag_writemem_sound)

	MDRV_INTERLEAVE(10)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_HAS_SHADOWS)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(12*8, (64-14)*8-1, 2*8, 30*8-1 )

	MDRV_PALETTE_LENGTH(1024)

	MDRV_VIDEO_START(chqflag)
	MDRV_VIDEO_UPDATE(chqflag)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD(YM2151, XTAL_3_579545MHz) /* verified on pcb */
	MDRV_SOUND_CONFIG(ym2151_interface)
	MDRV_SOUND_ROUTE(0, "left", 0.80)
	MDRV_SOUND_ROUTE(1, "right", 0.80)

	MDRV_SOUND_ADD(K007232, XTAL_3_579545MHz) /* verified on pcb */
	MDRV_SOUND_CONFIG(k007232_interface_1)
	MDRV_SOUND_ROUTE(0, "left", 0.20)
	MDRV_SOUND_ROUTE(0, "right", 0.20)
	MDRV_SOUND_ROUTE(1, "left", 0.20)
	MDRV_SOUND_ROUTE(1, "right", 0.20)

	MDRV_SOUND_ADD(K007232, XTAL_3_579545MHz) /* verified on pcb */
	MDRV_SOUND_CONFIG(k007232_interface_2)
	MDRV_SOUND_ROUTE(0, "left", 0.20)
	MDRV_SOUND_ROUTE(1, "right", 0.20)
MACHINE_DRIVER_END

ROM_START( chqflag )
	ROM_REGION( 0x58800, REGION_CPU1, 0 )	/* 052001 code */
	ROM_LOAD( "717h02",		0x050000, 0x008000, CRC(f5bd4e78) SHA1(7bab02152d055a6c3a322c88e7ee0b85a39d8ef2) )	/* banked ROM */
	ROM_CONTINUE(			0x008000, 0x008000 )				/* fixed ROM */
	ROM_LOAD( "717e10",		0x010000, 0x040000, CRC(72fc56f6) SHA1(433ea9a33f0230e046c731c70060f6a38db14ac7) )	/* banked ROM */
	/* extra memory for banked RAM */

	ROM_REGION( 0x10000, REGION_CPU2, 0 )	/* 64k for the SOUND CPU */
	ROM_LOAD( "717e01",		0x000000, 0x008000, CRC(966b8ba8) SHA1(ab7448cb61fa5922b1d8ae5f0d0f42d734ed4f93) )

    ROM_REGION( 0x100000, REGION_GFX1, 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD( "717e04",		0x000000, 0x080000, CRC(1a50a1cc) SHA1(bc16fab84c637ed124e37b115ddc0149560b727d) )	/* sprites */
	ROM_LOAD( "717e05",		0x080000, 0x080000, CRC(46ccb506) SHA1(3ed1f54744fc5cdc0f48e42f250c366267a8199a) )	/* sprites */

	ROM_REGION( 0x020000, REGION_GFX2, 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD( "717e06",		0x000000, 0x020000, CRC(1ec26c7a) SHA1(05b5b522c5ebf5d0a71a7fc39ec9382008ef33c8) )	/* zoom/rotate (N16) */

	ROM_REGION( 0x100000, REGION_GFX3, 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD( "717e07",		0x000000, 0x040000, CRC(b9a565a8) SHA1(a11782f7336e5ad58a4c6ea81f2eeac35d5e7d0a) )	/* zoom/rotate (L20) */
	ROM_LOAD( "717e08",		0x040000, 0x040000, CRC(b68a212e) SHA1(b2bd121a43552c3ade528ac763a0df40c3e648e0) )	/* zoom/rotate (L22) */
	ROM_LOAD( "717e11",		0x080000, 0x040000, CRC(ebb171ec) SHA1(d65d4a6b169ce03e4427b2a397484634f938236b) )	/* zoom/rotate (N20) */
	ROM_LOAD( "717e12",		0x0c0000, 0x040000, CRC(9269335d) SHA1(af298c8cff50d707d6abc806065f8e931f975dc0) )	/* zoom/rotate (N22) */

	ROM_REGION( 0x080000, REGION_SOUND1, 0 )	/* 007232 data (chip 1) */
	ROM_LOAD( "717e03",		0x000000, 0x080000, CRC(ebe73c22) SHA1(fad3334e5e91bf8d11b74ffdbbfd57567e6f6f8c) )

	ROM_REGION( 0x080000, REGION_SOUND2, 0 )	/* 007232 data (chip 2) */
	ROM_LOAD( "717e09",		0x000000, 0x080000, CRC(d74e857d) SHA1(00c851c857650d67fc4caccea4461d99be4acb3c) )
ROM_END

ROM_START( chqflagj )
	ROM_REGION( 0x58800, REGION_CPU1, 0 )	/* 052001 code */
	ROM_LOAD( "717j02.bin",	0x050000, 0x008000, CRC(05355daa) SHA1(130ddbc289c077565e44f33c63a63963e6417e19) )	/* banked ROM */
	ROM_CONTINUE(			0x008000, 0x008000 )				/* fixed ROM */
	ROM_LOAD( "717e10",		0x010000, 0x040000, CRC(72fc56f6) SHA1(433ea9a33f0230e046c731c70060f6a38db14ac7) )	/* banked ROM */
	/* extra memory for banked RAM */

	ROM_REGION( 0x10000, REGION_CPU2, 0 )	/* 64k for the SOUND CPU */
	ROM_LOAD( "717e01",		0x000000, 0x008000, CRC(966b8ba8) SHA1(ab7448cb61fa5922b1d8ae5f0d0f42d734ed4f93) )

    ROM_REGION( 0x100000, REGION_GFX1, 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD( "717e04",		0x000000, 0x080000, CRC(1a50a1cc) SHA1(bc16fab84c637ed124e37b115ddc0149560b727d) )	/* sprites */
	ROM_LOAD( "717e05",		0x080000, 0x080000, CRC(46ccb506) SHA1(3ed1f54744fc5cdc0f48e42f250c366267a8199a) )	/* sprites */

	ROM_REGION( 0x020000, REGION_GFX2, 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD( "717e06",		0x000000, 0x020000, CRC(1ec26c7a) SHA1(05b5b522c5ebf5d0a71a7fc39ec9382008ef33c8) )	/* zoom/rotate (N16) */

	ROM_REGION( 0x100000, REGION_GFX3, 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD( "717e07",		0x000000, 0x040000, CRC(b9a565a8) SHA1(a11782f7336e5ad58a4c6ea81f2eeac35d5e7d0a) )	/* zoom/rotate (L20) */
	ROM_LOAD( "717e08",		0x040000, 0x040000, CRC(b68a212e) SHA1(b2bd121a43552c3ade528ac763a0df40c3e648e0) )	/* zoom/rotate (L22) */
	ROM_LOAD( "717e11",		0x080000, 0x040000, CRC(ebb171ec) SHA1(d65d4a6b169ce03e4427b2a397484634f938236b) )	/* zoom/rotate (N20) */
	ROM_LOAD( "717e12",		0x0c0000, 0x040000, CRC(9269335d) SHA1(af298c8cff50d707d6abc806065f8e931f975dc0) )	/* zoom/rotate (N22) */

	ROM_REGION( 0x080000, REGION_SOUND1, 0 )	/* 007232 data (chip 1) */
	ROM_LOAD( "717e03",		0x000000, 0x080000, CRC(ebe73c22) SHA1(fad3334e5e91bf8d11b74ffdbbfd57567e6f6f8c) )

	ROM_REGION( 0x080000, REGION_SOUND2, 0 )	/* 007232 data (chip 2) */
	ROM_LOAD( "717e09",		0x000000, 0x080000, CRC(d74e857d) SHA1(00c851c857650d67fc4caccea4461d99be4acb3c) )
ROM_END



static DRIVER_INIT( chqflag )
{
	UINT8 *RAM = memory_region(REGION_CPU1);

	konami_rom_deinterleave_2(REGION_GFX1);
	paletteram = &RAM[0x58000];
}

GAME( 1988, chqflag,        0, chqflag, chqflag, chqflag, ROT90, "Konami", "Chequered Flag", GAME_UNEMULATED_PROTECTION | GAME_IMPERFECT_SOUND )
GAME( 1988, chqflagj, chqflag, chqflag, chqflag, chqflag, ROT90, "Konami", "Chequered Flag (Japan)", GAME_UNEMULATED_PROTECTION | GAME_IMPERFECT_SOUND )
