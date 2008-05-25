/***************************************************************************

Over Drive (GX789) (c) 1990 Konami

driver by Nicola Salmoria

Notes:
- Missing road (two unemulated K053250)
- Visible area and relative placement of sprites and tiles is most likely wrong.
- Test mode doesn't work well with 3 IRQ5 per frame, the ROM check doens't work
  and the coin A setting isn't shown. It's OK with 1 IRQ5 per frame.
- Some flickering sprites, this might be an interrupt/timing issue
- The screen is cluttered with sprites which aren't supposed to be visible,
  increasing the coordinate mask in K053247_sprites_draw() from 0x3ff to 0xfff
  fixes this but breaks other games (e.g. Vendetta).
- The "Continue?" sprites are not visible until you press start
- priorities

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "video/konamiic.h"
#include "machine/eeprom.h"
#include "cpu/m6809/m6809.h"
#include "sound/2151intf.h"
#include "sound/k053260.h"


VIDEO_START( overdriv );
VIDEO_UPDATE( overdriv );




static READ16_HANDLER( K051316_0_msb_r )
{
	return K051316_0_r(machine,offset) << 8;
}

static READ16_HANDLER( K051316_1_msb_r )
{
	return K051316_1_r(machine,offset) << 8;
}

static READ16_HANDLER( K051316_rom_0_msb_r )
{
	return K051316_rom_0_r(machine,offset) << 8;
}

static READ16_HANDLER( K051316_rom_1_msb_r )
{
	return K051316_rom_1_r(machine,offset) << 8;
}

static WRITE16_HANDLER( K051316_0_msb_w )
{
	if (ACCESSING_BITS_8_15)
		K051316_0_w(machine,offset,data >> 8);
}

static WRITE16_HANDLER( K051316_1_msb_w )
{
	if (ACCESSING_BITS_8_15)
		K051316_1_w(machine,offset,data >> 8);
}

static WRITE16_HANDLER( K051316_ctrl_0_msb_w )
{
	if (ACCESSING_BITS_8_15)
		K051316_ctrl_0_w(machine,offset,data >> 8);
}

static WRITE16_HANDLER( K051316_ctrl_1_msb_w )
{
	if (ACCESSING_BITS_8_15)
		K051316_ctrl_1_w(machine,offset,data >> 8);
}


/***************************************************************************

  EEPROM

***************************************************************************/

static const UINT8 default_eeprom[128] =
{
	0x77,0x58,0xFF,0xFF,0x00,0x78,0x90,0x00,0x00,0x78,0x70,0x00,0x00,0x78,0x50,0x00,
	0x54,0x41,0x4B,0x51,0x31,0x36,0x46,0x55,0x4A,0xFF,0x03,0x00,0x02,0x70,0x02,0x50,
	0x00,0xB4,0x03,0x00,0xB4,0x03,0x00,0xB4,0x03,0x00,0xB4,0x03,0x00,0xB4,0x03,0x00,
	0xB4,0x03,0x00,0xB4,0x03,0x00,0xB4,0x03,0x00,0xB4,0x03,0x00,0xB4,0x03,0x00,0xB4,
	0x03,0x00,0xB4,0x03,0x00,0xB4,0x03,0x00,0xB4,0x03,0x00,0xB4,0x03,0x00,0xB4,0x03,
	0x00,0xB4,0x03,0x00,0xB4,0x03,0x00,0xB4,0x03,0x00,0xB4,0x03,0x00,0xB4,0x03,0x00,
	0xB4,0x03,0x00,0xB4,0x03,0x00,0xB4,0x03,0x00,0xB4,0x03,0x00,0xB4,0x03,0x00,0xB4,
	0x03,0x00,0xB4,0x03,0x00,0xB4,0x03,0x00,0xB4,0x03,0x00,0xB4,0x03,0x00,0xB4,0x03
};


static const eeprom_interface eeprom_intf =
{
	6,				/* address bits */
	16,				/* data bits */
	"011000",		/*  read command */
	"010100",		/* write command */
	0,				/* erase command */
	"010000000000",	/* lock command */
	"010011000000"	/* unlock command */
};

static NVRAM_HANDLER( overdriv )
{
	if (read_or_write)
		eeprom_save(file);
	else
	{
		eeprom_init(&eeprom_intf);

		if (file)
			eeprom_load(file);
		else
			eeprom_set_data(default_eeprom,sizeof(default_eeprom));
	}
}

static READ16_HANDLER( eeprom_r )
{
	int res;

//logerror("%06x eeprom_r\n",activecpu_get_pc());
	/* bit 6 is EEPROM data */
	res = (eeprom_read_bit() << 6) | input_port_read_indexed(machine,0);

	return res;
}

static WRITE16_HANDLER( eeprom_w )
{
//logerror("%06x: write %04x to eeprom_w\n",activecpu_get_pc(),data);
	if (ACCESSING_BITS_0_7)
	{
		/* bit 0 is data */
		/* bit 1 is clock (active high) */
		/* bit 2 is cs (active low) */
		eeprom_write_bit(data & 0x01);
		eeprom_set_cs_line((data & 0x04) ? CLEAR_LINE : ASSERT_LINE);
		eeprom_set_clock_line((data & 0x02) ? ASSERT_LINE : CLEAR_LINE);
	}
}





static INTERRUPT_GEN( cpuA_interrupt )
{
	if (cpu_getiloops()) cpunum_set_input_line(machine, 0, 5, HOLD_LINE);
	else cpunum_set_input_line(machine, 0, 4, HOLD_LINE);
}

static INTERRUPT_GEN( cpuB_interrupt )
{
	if (K053246_is_IRQ_enabled()) cpunum_set_input_line(machine, 1, 4, HOLD_LINE);
}


static MACHINE_RESET( overdriv )
{
	/* start with cpu B halted */
	cpunum_set_input_line(machine, 1, INPUT_LINE_RESET, ASSERT_LINE);
}

static WRITE16_HANDLER( cpuA_ctrl_w )
{
	if (ACCESSING_BITS_0_7)
	{
		/* bit 0 probably enables the second 68000 */
		cpunum_set_input_line(machine, 1, INPUT_LINE_RESET, (data & 0x01) ? CLEAR_LINE : ASSERT_LINE);

		/* bit 1 is clear during service mode - function unknown */

		set_led_status(0,data & 0x08);
		coin_counter_w(0,data & 0x10);
		coin_counter_w(1,data & 0x20);

//logerror("%06x: write %04x to cpuA_ctrl_w\n",activecpu_get_pc(),data);
	}
}


static UINT16 cpuB_ctrl;

static READ16_HANDLER( cpuB_ctrl_r )
{
	return cpuB_ctrl;
}

static WRITE16_HANDLER( cpuB_ctrl_w )
{
	COMBINE_DATA(&cpuB_ctrl);

	if (ACCESSING_BITS_0_7)
	{
		/* bit 0 = enable sprite ROM reading */
		K053246_set_OBJCHA_line((data & 0x01) ? ASSERT_LINE : CLEAR_LINE);

		/* bit 1 used but unknown (irq enable?) */

		/* other bits unused? */
	}
}


static UINT16 *sharedram;

static READ16_HANDLER( sharedram_r )
{
	return sharedram[offset];
}

static WRITE16_HANDLER( sharedram_w )
{
	COMBINE_DATA(&sharedram[offset]);
}



static READ16_HANDLER( overdriv_sound_0_r )
{
	return K053260_0_r(machine,2 + offset);
}

static READ16_HANDLER( overdriv_sound_1_r )
{
	return K053260_1_r(machine,2 + offset);
}

static WRITE16_HANDLER( overdriv_soundirq_w )
{
	cpunum_set_input_line(machine, 2,M6809_IRQ_LINE,HOLD_LINE);
}

static WRITE16_HANDLER( overdriv_cpuB_irq5_w )
{
	cpunum_set_input_line(machine, 1,5,HOLD_LINE);
}

static WRITE16_HANDLER( overdriv_cpuB_irq6_w )
{
	cpunum_set_input_line(machine, 1,6,HOLD_LINE);
}




static ADDRESS_MAP_START( overdriv_readmem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_READ(SMH_ROM)
	AM_RANGE(0x040000, 0x043fff) AM_READ(SMH_RAM)
	AM_RANGE(0x080000, 0x080fff) AM_READ(SMH_RAM)
	AM_RANGE(0x0c0000, 0x0c0001) AM_READ(eeprom_r)
	AM_RANGE(0x0c0002, 0x0c0003) AM_READ(input_port_1_word_r)
	AM_RANGE(0x180000, 0x180001) AM_READ(input_port_2_word_r)
	AM_RANGE(0x1d8000, 0x1d8003) AM_READ(overdriv_sound_0_r)	/* K053260 */
	AM_RANGE(0x1e0000, 0x1e0003) AM_READ(overdriv_sound_1_r)	/* K053260 */
	AM_RANGE(0x200000, 0x203fff) AM_READ(sharedram_r)
	AM_RANGE(0x210000, 0x210fff) AM_READ(K051316_0_msb_r)
	AM_RANGE(0x218000, 0x218fff) AM_READ(K051316_1_msb_r)
	AM_RANGE(0x220000, 0x220fff) AM_READ(K051316_rom_0_msb_r)
	AM_RANGE(0x228000, 0x228fff) AM_READ(K051316_rom_1_msb_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( overdriv_writemem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_WRITE(SMH_ROM)
	AM_RANGE(0x040000, 0x043fff) AM_WRITE(SMH_RAM)	/* work RAM */
	AM_RANGE(0x080000, 0x080fff) AM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE(&paletteram16)
	AM_RANGE(0x0e0000, 0x0e0001) AM_WRITE(SMH_NOP)	/* unknown (always 0x30) */
	AM_RANGE(0x100000, 0x10001f) AM_WRITE(SMH_NOP)	/* 053252? (LSB) */
	AM_RANGE(0x140000, 0x140001) AM_WRITE(watchdog_reset16_w)
	AM_RANGE(0x1c0000, 0x1c001f) AM_WRITE(K051316_ctrl_0_msb_w)
	AM_RANGE(0x1c8000, 0x1c801f) AM_WRITE(K051316_ctrl_1_msb_w)
	AM_RANGE(0x1d0000, 0x1d001f) AM_WRITE(K053251_msb_w)
	AM_RANGE(0x1d8000, 0x1d8003) AM_WRITE(K053260_0_lsb_w)
	AM_RANGE(0x1e0000, 0x1e0003) AM_WRITE(K053260_1_lsb_w)
	AM_RANGE(0x1e8000, 0x1e8001) AM_WRITE(overdriv_soundirq_w)
	AM_RANGE(0x1f0000, 0x1f0001) AM_WRITE(cpuA_ctrl_w)	/* halt cpu B, coin counter, start lamp, other? */
	AM_RANGE(0x1f8000, 0x1f8001) AM_WRITE(eeprom_w)
	AM_RANGE(0x200000, 0x203fff) AM_WRITE(sharedram_w) AM_BASE(&sharedram)
	AM_RANGE(0x210000, 0x210fff) AM_WRITE(K051316_0_msb_w)
	AM_RANGE(0x218000, 0x218fff) AM_WRITE(K051316_1_msb_w)
	AM_RANGE(0x230000, 0x230001) AM_WRITE(overdriv_cpuB_irq6_w)
	AM_RANGE(0x238000, 0x238001) AM_WRITE(overdriv_cpuB_irq5_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( overdriv_readmem2, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_READ(SMH_ROM)
	AM_RANGE(0x080000, 0x083fff) AM_READ(SMH_RAM)
AM_RANGE(0x0c0000, 0x0c1fff) AM_READ(SMH_RAM)
AM_RANGE(0x100000, 0x10000f) AM_READ(SMH_NOP)	// K053250 #0
AM_RANGE(0x108000, 0x10800f) AM_READ(SMH_NOP)	// K053250 #1
	AM_RANGE(0x118000, 0x118fff) AM_READ(K053247_word_r)
	AM_RANGE(0x120000, 0x120001) AM_READ(K053246_word_r)
	AM_RANGE(0x128000, 0x128001) AM_READ(cpuB_ctrl_r)
	AM_RANGE(0x200000, 0x203fff) AM_READ(sharedram_r)
AM_RANGE(0x208000, 0x20bfff) AM_READ(SMH_RAM)

AM_RANGE(0x218000, 0x219fff) AM_READ(SMH_NOP)	// K053250 #0 gfx ROM read (LSB)
AM_RANGE(0x220000, 0x221fff) AM_READ(SMH_NOP)	// K053250 #1 gfx ROM read (LSB)
ADDRESS_MAP_END

static ADDRESS_MAP_START( overdriv_writemem2, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_WRITE(SMH_ROM)
	AM_RANGE(0x080000, 0x083fff) AM_WRITE(SMH_RAM)	/* work RAM */
AM_RANGE(0x0c0000, 0x0c1fff) AM_WRITE(SMH_RAM)
AM_RANGE(0x100000, 0x10000f) AM_WRITE(SMH_NOP)	// K053250 #0
AM_RANGE(0x108000, 0x10800f) AM_WRITE(SMH_NOP)	// K053250 #1
	AM_RANGE(0x118000, 0x118fff) AM_WRITE(K053247_word_w)
	AM_RANGE(0x128000, 0x128001) AM_WRITE(cpuB_ctrl_w)	/* enable K053247 ROM reading, plus something else */
	AM_RANGE(0x130000, 0x130007) AM_WRITE(K053246_word_w)
	AM_RANGE(0x200000, 0x203fff) AM_WRITE(sharedram_w)
AM_RANGE(0x208000, 0x20bfff) AM_WRITE(SMH_RAM)
ADDRESS_MAP_END


static ADDRESS_MAP_START( overdriv_s_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0201, 0x0201) AM_READ(YM2151_status_port_0_r)
	AM_RANGE(0x0400, 0x042f) AM_READ(K053260_0_r)
	AM_RANGE(0x0600, 0x062f) AM_READ(K053260_1_r)
	AM_RANGE(0x0800, 0x0fff) AM_READ(SMH_RAM)
	AM_RANGE(0x1000, 0xffff) AM_READ(SMH_ROM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( overdriv_s_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0200, 0x0200) AM_WRITE(YM2151_register_port_0_w)
	AM_RANGE(0x0201, 0x0201) AM_WRITE(YM2151_data_port_0_w)
	AM_RANGE(0x0400, 0x042f) AM_WRITE(K053260_0_w)
	AM_RANGE(0x0600, 0x062f) AM_WRITE(K053260_1_w)
	AM_RANGE(0x0800, 0x0fff) AM_WRITE(SMH_RAM)
	AM_RANGE(0x1000, 0xffff) AM_WRITE(SMH_ROM)
ADDRESS_MAP_END



static INPUT_PORTS_START( overdriv )
	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL )	/* EEPROM data */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL )	// ?

	PORT_START
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(100) PORT_KEYDELTA(50)
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ STEP8(0,4) },
	{ STEP8(7*8*4,-8*4) },
	8*8*4
};

static GFXDECODE_START( overdriv )
	GFXDECODE_ENTRY( REGION_GFX4, 0, charlayout, 0, 0x80 )
	GFXDECODE_ENTRY( REGION_GFX5, 0, charlayout, 0, 0x80 )
GFXDECODE_END



static const struct K053260_interface k053260_interface =
{
	REGION_SOUND1
};



static MACHINE_DRIVER_START( overdriv )

	/* basic machine hardware */
	MDRV_CPU_ADD(M68000,24000000/2)	/* 12 MHz */
	MDRV_CPU_PROGRAM_MAP(overdriv_readmem,overdriv_writemem)
	MDRV_CPU_VBLANK_INT_HACK(cpuA_interrupt,4)	/* ??? IRQ 4 is vblank, IRQ 5 of unknown origin */

	MDRV_CPU_ADD(M68000,24000000/2)	/* 12 MHz */
	MDRV_CPU_PROGRAM_MAP(overdriv_readmem2,overdriv_writemem2)
	MDRV_CPU_VBLANK_INT("main", cpuB_interrupt)	/* IRQ 5 and 6 are generated by the main CPU. */
								/* IRQ 5 is used only in test mode, to request the checksums of the gfx ROMs. */
	MDRV_CPU_ADD(M6809,3579545/2)	/* 1.789 MHz?? This might be the right speed, but ROM testing */
						/* takes a little too much (the counter wraps from 0000 to 9999). */
						/* This might just mean that the video refresh rate is less than */
						/* 60 fps, that's how I fixed it for now. */
	MDRV_CPU_PROGRAM_MAP(overdriv_s_readmem,overdriv_s_writemem)

	MDRV_INTERLEAVE(200)

	MDRV_MACHINE_RESET(overdriv)
	MDRV_NVRAM_HANDLER(overdriv)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_HAS_SHADOWS)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(59)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(13*8, (64-13)*8-1, 0*8, 32*8-1 )

	MDRV_GFXDECODE(overdriv)
	MDRV_PALETTE_LENGTH(2048)

	MDRV_VIDEO_START(overdriv)
	MDRV_VIDEO_UPDATE(overdriv)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD(YM2151, 3579545)
	MDRV_SOUND_ROUTE(0, "left", 1.0)
	MDRV_SOUND_ROUTE(1, "right", 1.0)

	MDRV_SOUND_ADD(K053260, 3579545)
	MDRV_SOUND_CONFIG(k053260_interface)
	MDRV_SOUND_ROUTE(0, "left", 0.70)
	MDRV_SOUND_ROUTE(1, "right", 0.70)

	MDRV_SOUND_ADD(K053260, 3579545)
	MDRV_SOUND_CONFIG(k053260_interface)
	MDRV_SOUND_ROUTE(0, "left", 0.70)
	MDRV_SOUND_ROUTE(1, "right", 0.70)
MACHINE_DRIVER_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( overdriv )
	ROM_REGION( 0x40000, REGION_CPU1, 0 )
	ROM_LOAD16_BYTE( "789.2",        0x00000, 0x20000, CRC(77f18f3f) SHA1(a8c91435573c7851a7864d07eeacfb2f142abbe2) )
	ROM_LOAD16_BYTE( "789.1",        0x00001, 0x20000, CRC(4f44e6ad) SHA1(9fa871f55e6b2ec353dd979ded568cd9da83f5d6) )

	ROM_REGION( 0x40000, REGION_CPU2, 0 )
	ROM_LOAD16_BYTE( "789.4",        0x00000, 0x20000, CRC(46fb7e88) SHA1(f706a76aff9bec64abe6da325cba0715d6e6ed0a) )
	ROM_LOAD16_BYTE( "789.3",        0x00001, 0x20000, CRC(24427195) SHA1(48f4f81729acc0e497b40fddbde11242c5c4c573) )

	ROM_REGION( 0x10000, REGION_CPU3, 0 )
	ROM_LOAD( "789.5",        0x00000, 0x10000, CRC(1085f069) SHA1(27228cedb357ff2e130a4bd6d8aa01cf537e034f) )

	ROM_REGION( 0x400000, REGION_GFX1, 0 )	/* graphics (addressable by the CPU) */
	ROM_LOAD( "e12.r1",       0x000000, 0x100000, CRC(14a10fb2) SHA1(03fb9c15514c5ecc2d9ae4a53961c4bbb49cec73) )	/* sprites */
	ROM_LOAD( "e13.r4",       0x100000, 0x100000, CRC(6314a628) SHA1(f8a8918998c266109348c77427a7696b503daeb3) )
	ROM_LOAD( "e14.r10",      0x200000, 0x100000, CRC(b5eca14b) SHA1(a1c5f5e9cd8bbcfc875e2acb33be024724da63aa) )
	ROM_LOAD( "e15.r15",      0x300000, 0x100000, CRC(5d93e0c3) SHA1(d5cb7666c0c28fd465c860c7f9dbb18a7f739a93) )

	ROM_REGION( 0x020000, REGION_GFX2, 0 )	/* graphics (addressable by the CPU) */
	ROM_LOAD( "e06.a21",      0x000000, 0x020000, CRC(14a085e6) SHA1(86dad6f223e13ff8af7075c3d99bb0a83784c384) )	/* zoom/rotate */

	ROM_REGION( 0x020000, REGION_GFX3, 0 )	/* graphics (addressable by the CPU) */
	ROM_LOAD( "e07.c23",      0x000000, 0x020000, CRC(8a6ceab9) SHA1(1a52b7361f71a6126cd648a76af00223d5b25c7a) )	/* zoom/rotate */

	ROM_REGION( 0x0c0000, REGION_GFX4, 0 )	/* graphics (addressable by the CPU) */
	ROM_LOAD( "e18.p22",      0x000000, 0x040000, CRC(985a4a75) SHA1(b726166c295be6fbec38a9d11098cc4a4a5de456) )	/* 053250 #0 */
	ROM_LOAD( "e19.r22",      0x040000, 0x040000, CRC(15c54ea2) SHA1(5b10bd28e48e51613359820ba8c75d4a91c2d322) )
	ROM_LOAD( "e20.s22",      0x080000, 0x040000, CRC(ea204acd) SHA1(52b8c30234eaefcba1074496028a4ac2bca48e95) )

	ROM_REGION( 0x080000, REGION_GFX5, 0 )	/* unknown (053250?) */
	ROM_LOAD( "e16.p12",      0x000000, 0x040000, CRC(9348dee1) SHA1(367193373e28962b5b0e54cc15d68ed88ab83f12) )	/* 053250 #1 */
	ROM_LOAD( "e17.p17",      0x040000, 0x040000, CRC(04c07248) SHA1(873445002cbf90c9fc5a35bf4a8f6c43193ee342) )

	ROM_REGION( 0x200000, REGION_SOUND1, 0 )	/* 053260 samples */
	ROM_LOAD( "e03.j1",       0x000000, 0x100000, CRC(51ebfebe) SHA1(17f0c23189258e801f48d5833fe934e7a48d071b) )
	ROM_LOAD( "e02.f1",       0x100000, 0x100000, CRC(bdd3b5c6) SHA1(412332d64052c0a3714f4002c944b0e7d32980a4) )
ROM_END



static DRIVER_INIT( overdriv )
{
	konami_rom_deinterleave_4(REGION_GFX1);
}



GAME( 1990, overdriv, 0, overdriv, overdriv, overdriv, ROT90, "Konami", "Over Drive", GAME_IMPERFECT_GRAPHICS )
