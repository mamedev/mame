#define JOE_DEBUG 0
#define JOE_DMADELAY attotime_add(ATTOTIME_IN_NSEC(42700), ATTOTIME_IN_NSEC(341300))

/***************************************************************************

    GI Joe  (c) 1992 Konami


Change Log
----------

AT070403:

tilemap.h,tilemap.c
- added tilemap_get_transparency_data() for transparency cache manipulation

video\konamiic.c
- added preliminary K056832 tilemap<->linemap switching and tileline code

drivers\gijoe.c
- updated video settings, memory map and irq handler
- added object blitter

video\gijoe.c
- completed K054157 to K056832 migration
- added ground scroll emulation
- fixed sprite and BG priority
- improved shadows and layer alignment


Known Issues
------------

- sprite gaps (K053247 zoom fraction rounding)
- shadow masking (eg. the shadow of Baroness' aircraft should not project on the sky)

***************************************************************************/

#include "driver.h"
#include "cpu/m68000/m68000.h"
#include "video/konamiic.h"
#include "cpu/z80/z80.h"
#include "machine/eeprom.h"
#include "sound/k054539.h"
#include "konamipt.h"

VIDEO_START( gijoe );
VIDEO_UPDATE( gijoe );

static UINT16 *gijoe_workram;
static UINT16 cur_control2;
static int init_eeprom_count;
static emu_timer *dmadelay_timer;

static const eeprom_interface eeprom_intf =
{
	7,				/* address bits */
	8,				/* data bits */
	"011000",		/*  read command */
	"011100",		/* write command */
	"0100100000000",/* erase command */
	"0100000000000",/* lock command */
	"0100110000000" /* unlock command */
};

static NVRAM_HANDLER( gijoe )
{
	if (read_or_write)
		eeprom_save(file);
	else
	{
		eeprom_init(machine, &eeprom_intf);

		if (file)
		{
			init_eeprom_count = 0;
			eeprom_load(file);
		}
		else
			init_eeprom_count = 2720;
	}
}

static READ16_HANDLER( control1_r )
{
	int res;

	/* bit 8  is EEPROM data */
	/* bit 9  is EEPROM ready */
	/* bit 11 is service button */
	res = input_port_read(space->machine, "START");

	if (init_eeprom_count)
	{
		init_eeprom_count--;
		res &= 0xf7ff;
	}

	return res;
}

static READ16_HANDLER( control2_r )
{
	return cur_control2;
}

static WRITE16_HANDLER( control2_w )
{
	if(ACCESSING_BITS_0_7) {
		/* bit 0  is data */
		/* bit 1  is cs (active low) */
		/* bit 2  is clock (active high) */
		/* bit 3  (unknown: coin) */
		/* bit 5  is enable irq 6 */
		/* bit 7  (unknown: enable irq 5?) */

		eeprom_write_bit(data & 0x01);
		eeprom_set_cs_line((data & 0x02) ? CLEAR_LINE : ASSERT_LINE);
		eeprom_set_clock_line((data & 0x04) ? ASSERT_LINE : CLEAR_LINE);
		cur_control2 = data;

		/* bit 6 = enable sprite ROM reading */
		K053246_set_OBJCHA_line((data & 0x0040) ? ASSERT_LINE : CLEAR_LINE);
	}
}

static void gijoe_objdma(void)
{
	UINT16 *src_head, *src_tail, *dst_head, *dst_tail;

	src_head = spriteram16;
	src_tail = spriteram16 + 255*8;
	K053247_export_config(&dst_head, 0, 0, 0, 0);
	dst_tail = dst_head + 255*8;

	for (; src_head<=src_tail; src_head+=8)
	{
		if (*src_head & 0x8000)
		{
			memcpy(dst_head, src_head, 0x10);
			dst_head += 8;
		}
		else
		{
			*dst_tail = 0;
			dst_tail -= 8;
		}
	}
}

static TIMER_CALLBACK( dmaend_callback )
{
	if (cur_control2 & 0x0020)
		cputag_set_input_line(machine, "maincpu", 6, HOLD_LINE);
}

static INTERRUPT_GEN( gijoe_interrupt )
{
	// global interrupt masking (*this game only)
	if (!K056832_is_IRQ_enabled(0)) return;

	if (K053246_is_IRQ_enabled())
	{
		gijoe_objdma();

		// 42.7us(clr) + 341.3us(xfer) delay at 6Mhz dotclock
		timer_adjust_oneshot(dmadelay_timer, JOE_DMADELAY, 0);
	}

	// trigger V-blank interrupt
	if (cur_control2 & 0x0080)
		cpu_set_input_line(device, 5, HOLD_LINE);
}

static WRITE16_HANDLER( sound_cmd_w )
{
	if(ACCESSING_BITS_0_7) {
		data &= 0xff;
		soundlatch_w(space, 0, data);
	}
}

static WRITE16_HANDLER( sound_irq_w )
{
	cputag_set_input_line(space->machine, "audiocpu", 0, HOLD_LINE);
}

static READ16_HANDLER( sound_status_r )
{
	return soundlatch2_r(space,0);
}

static void sound_nmi(const device_config *device)
{
	cputag_set_input_line(device->machine, "audiocpu", INPUT_LINE_NMI, PULSE_LINE);
}

static MACHINE_START( gijoe )
{
	state_save_register_global(machine, cur_control2);

	dmadelay_timer = timer_alloc(machine, dmaend_callback, NULL);
}


static ADDRESS_MAP_START( gijoe_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x100fff) AM_RAM AM_BASE(&spriteram16)								// Sprites
	AM_RANGE(0x110000, 0x110007) AM_WRITE(K053246_word_w)
	AM_RANGE(0x120000, 0x121fff) AM_READWRITE(K056832_ram_word_r, K056832_ram_word_w)		// Graphic planes
	AM_RANGE(0x122000, 0x123fff) AM_READWRITE(K056832_ram_word_r, K056832_ram_word_w)		// Graphic planes mirror read
	AM_RANGE(0x130000, 0x131fff) AM_READ(K056832_rom_word_r) 								// Passthrough to tile roms
	AM_RANGE(0x160000, 0x160007) AM_WRITE(K056832_b_word_w)									// VSCCS (board dependent)
	AM_RANGE(0x170000, 0x170001) AM_WRITENOP												// Watchdog
	AM_RANGE(0x180000, 0x18ffff) AM_RAM AM_BASE(&gijoe_workram)					// Main RAM.  Spec. 180000-1803ff, 180400-187fff
	AM_RANGE(0x190000, 0x190fff) AM_RAM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE(&paletteram16)
	AM_RANGE(0x1a0000, 0x1a001f) AM_WRITE(K053251_lsb_w)
	AM_RANGE(0x1b0000, 0x1b003f) AM_WRITE(K056832_word_w)
	AM_RANGE(0x1c000c, 0x1c000d) AM_WRITE(sound_cmd_w)
	AM_RANGE(0x1c0014, 0x1c0015) AM_READ(sound_status_r)
	AM_RANGE(0x1c0000, 0x1c001f) AM_RAM
	AM_RANGE(0x1d0000, 0x1d0001) AM_WRITE(sound_irq_w)
	AM_RANGE(0x1e0000, 0x1e0001) AM_READ_PORT("P1_P2")
	AM_RANGE(0x1e0002, 0x1e0003) AM_READ_PORT("P3_P4")
	AM_RANGE(0x1e4000, 0x1e4001) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x1e4002, 0x1e4003) AM_READ(control1_r)
	AM_RANGE(0x1e8000, 0x1e8001) AM_READWRITE(control2_r, control2_w)
	AM_RANGE(0x1f0000, 0x1f0001) AM_READ(K053246_word_r)
#if JOE_DEBUG
	AM_RANGE(0x110000, 0x110007) AM_READ(K053246_reg_word_r)
	AM_RANGE(0x160000, 0x160007) AM_READ(K056832_b_word_r)
	AM_RANGE(0x1a0000, 0x1a001f) AM_READ(K053251_lsb_r)
	AM_RANGE(0x1b0000, 0x1b003f) AM_READ(K056832_word_r)
#endif
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xebff) AM_ROM
	AM_RANGE(0xf000, 0xf7ff) AM_RAM
	AM_RANGE(0xf800, 0xfa2f) AM_DEVREADWRITE("konami", k054539_r, k054539_w)
	AM_RANGE(0xfc00, 0xfc00) AM_WRITE(soundlatch2_w)
	AM_RANGE(0xfc02, 0xfc02) AM_READ(soundlatch_r)
ADDRESS_MAP_END

static INPUT_PORTS_START( gijoe )
	PORT_START("START")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_START3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_START4 )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(eeprom_bit_r, NULL)	// EEPROM data
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_SPECIAL )	// EEPROM ready (always 1)
	PORT_SERVICE_NO_TOGGLE( 0x0800, IP_ACTIVE_LOW )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_COIN3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_COIN4 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_SERVICE2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_SERVICE3 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_SERVICE4 )

	PORT_START("P1_P2")
	KONAMI16_LSB_40(1, IPT_BUTTON3 )
	PORT_DIPNAME( 0x0080, 0x0000, "Sound" )			PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0080, DEF_STR( Mono ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Stereo ) )
	KONAMI16_MSB_40(2, IPT_BUTTON3 )
	PORT_DIPNAME( 0x8000, 0x8000, "Coin mechanism" )	PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x8000, "Common" )
	PORT_DIPSETTING(      0x0000, "Independant" )

	PORT_START("P3_P4")
	KONAMI16_LSB_40(3, IPT_BUTTON3 )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Players ) )	PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0080, "2" )
	PORT_DIPSETTING(      0x0000, "4" )
	KONAMI16_MSB_40(4, IPT_BUTTON3 )
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x8000, "SW1:4" )	/* Listed as "Unused" */
INPUT_PORTS_END

static const k054539_interface k054539_config =
{
	NULL,
	NULL,
	sound_nmi
};

static MACHINE_DRIVER_START( gijoe )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 16000000)	/* Confirmed */
	MDRV_CPU_PROGRAM_MAP(gijoe_map)
	MDRV_CPU_VBLANK_INT("screen", gijoe_interrupt)

	MDRV_CPU_ADD("audiocpu", Z80, 8000000)	/* Amuse & confirmed. z80e */
	MDRV_CPU_PROGRAM_MAP(sound_map)

	MDRV_MACHINE_START(gijoe)
	MDRV_NVRAM_HANDLER(gijoe)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_HAS_SHADOWS | VIDEO_UPDATE_BEFORE_VBLANK)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(24, 24+288-1, 16, 16+224-1)

	MDRV_PALETTE_LENGTH(2048)

	MDRV_VIDEO_START(gijoe)
	MDRV_VIDEO_UPDATE(gijoe)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("konami", K054539, 48000)
	MDRV_SOUND_CONFIG(k054539_config)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END


ROM_START( gijoe )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "069eab03.rom", 0x000000,  0x40000, CRC(dd2d533f) SHA1(6fc9f7a8fc89155ef2b9ee43fe5e456d9b574f8c) )
	ROM_LOAD16_BYTE( "069eab02.rom", 0x000001,  0x40000, CRC(6bb11c87) SHA1(86581d24f73f2e837f1d4fc5f1f2188f610c50b6) )
	ROM_LOAD16_BYTE( "069a12",       0x080000,  0x40000, CRC(75a7585c) SHA1(443d6dee99edbe81ab1b7289e6cad403fe01cc0d) )
	ROM_LOAD16_BYTE( "069a11",       0x080001,  0x40000, CRC(3153e788) SHA1(fde4543eac707ef24b431e64011cf0f923d4d3ac) )

	ROM_REGION( 0x010000, "audiocpu", 0 )
	ROM_LOAD( "069a01", 0x000000, 0x010000, CRC(74172b99) SHA1(f5e0e0d43317454fdacd3df7cd3035fcae4aef68) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "069a10", 0x000000, 0x100000, CRC(4c6743ee) SHA1(fa94fbfb55955fdb40705e79b49103676961d919) )
	ROM_LOAD( "069a09", 0x100000, 0x100000, CRC(e6e36b05) SHA1(fecad503f2c285b2b0312e888c06dd6e87f95a07) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD( "069a08", 0x000000, 0x100000, CRC(325477d4) SHA1(140c57b0ac9e5cf702d788f416408a5eeb5d6d3c) )
	ROM_LOAD( "069a05", 0x100000, 0x100000, CRC(c4ab07ed) SHA1(dc806eff00937d9465b1726fae8fdc3022464a28) )
	ROM_LOAD( "069a07", 0x200000, 0x100000, CRC(ccaa3971) SHA1(16989cbbd65fe1b41c4a85fea02ba1e9880818a9) )
	ROM_LOAD( "069a06", 0x300000, 0x100000, CRC(63eba8e1) SHA1(aa318d356c2580765452106ea0d2228273a90523) )

	ROM_REGION( 0x200000, "konami", 0 )
	ROM_LOAD( "069a04", 0x000000, 0x200000, CRC(11d6dcd6) SHA1(04cbff9f61cd8641db538db809ddf20da29fd5ac) )
ROM_END

ROM_START( gijoeu )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("069uab03", 0x000000,  0x40000, CRC(25ff77d2) SHA1(bea2ae975718806698fd35ef1217bd842b2b69ec) )
	ROM_LOAD16_BYTE("069uab02", 0x000001,  0x40000, CRC(31cced1c) SHA1(3df1def671966b3c3d8117ac1b68adeeef9d98c0) )
	ROM_LOAD16_BYTE("069a12",   0x080000,  0x40000, CRC(75a7585c) SHA1(443d6dee99edbe81ab1b7289e6cad403fe01cc0d) )
	ROM_LOAD16_BYTE("069a11",   0x080001,  0x40000, CRC(3153e788) SHA1(fde4543eac707ef24b431e64011cf0f923d4d3ac) )

	ROM_REGION( 0x010000, "audiocpu", 0 )
	ROM_LOAD( "069a01", 0x000000, 0x010000, CRC(74172b99) SHA1(f5e0e0d43317454fdacd3df7cd3035fcae4aef68) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "069a10", 0x000000, 0x100000, CRC(4c6743ee) SHA1(fa94fbfb55955fdb40705e79b49103676961d919) )
	ROM_LOAD( "069a09", 0x100000, 0x100000, CRC(e6e36b05) SHA1(fecad503f2c285b2b0312e888c06dd6e87f95a07) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD( "069a08", 0x000000, 0x100000, CRC(325477d4) SHA1(140c57b0ac9e5cf702d788f416408a5eeb5d6d3c) )
	ROM_LOAD( "069a05", 0x100000, 0x100000, CRC(c4ab07ed) SHA1(dc806eff00937d9465b1726fae8fdc3022464a28) )
	ROM_LOAD( "069a07", 0x200000, 0x100000, CRC(ccaa3971) SHA1(16989cbbd65fe1b41c4a85fea02ba1e9880818a9) )
	ROM_LOAD( "069a06", 0x300000, 0x100000, CRC(63eba8e1) SHA1(aa318d356c2580765452106ea0d2228273a90523) )

	ROM_REGION( 0x200000, "konami", 0 )
	ROM_LOAD( "069a04", 0x000000, 0x200000, CRC(11d6dcd6) SHA1(04cbff9f61cd8641db538db809ddf20da29fd5ac) )
ROM_END

ROM_START( gijoej )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("069jaa03", 0x000000,  0x40000, CRC(4b398901) SHA1(98fcc6ae9cc69c67d82eb1a7ab0bb71e61aee623) )
	ROM_LOAD16_BYTE("069jaa02", 0x000001,  0x40000, CRC(8bb22392) SHA1(9f066ce2b529f7dad6f80a91fff266c478d56414) )
	ROM_LOAD16_BYTE("069a12", 0x080000,  0x40000, CRC(75a7585c) SHA1(443d6dee99edbe81ab1b7289e6cad403fe01cc0d) )
	ROM_LOAD16_BYTE("069a11", 0x080001,  0x40000, CRC(3153e788) SHA1(fde4543eac707ef24b431e64011cf0f923d4d3ac) )

	ROM_REGION( 0x010000, "audiocpu", 0 )
	ROM_LOAD( "069a01", 0x000000, 0x010000, CRC(74172b99) SHA1(f5e0e0d43317454fdacd3df7cd3035fcae4aef68) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "069a10", 0x000000, 0x100000, CRC(4c6743ee) SHA1(fa94fbfb55955fdb40705e79b49103676961d919) )
	ROM_LOAD( "069a09", 0x100000, 0x100000, CRC(e6e36b05) SHA1(fecad503f2c285b2b0312e888c06dd6e87f95a07) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD( "069a08", 0x000000, 0x100000, CRC(325477d4) SHA1(140c57b0ac9e5cf702d788f416408a5eeb5d6d3c) )
	ROM_LOAD( "069a05", 0x100000, 0x100000, CRC(c4ab07ed) SHA1(dc806eff00937d9465b1726fae8fdc3022464a28) )
	ROM_LOAD( "069a07", 0x200000, 0x100000, CRC(ccaa3971) SHA1(16989cbbd65fe1b41c4a85fea02ba1e9880818a9) )
	ROM_LOAD( "069a06", 0x300000, 0x100000, CRC(63eba8e1) SHA1(aa318d356c2580765452106ea0d2228273a90523) )

	ROM_REGION( 0x200000, "konami", 0 )
	ROM_LOAD( "069a04", 0x000000, 0x200000, CRC(11d6dcd6) SHA1(04cbff9f61cd8641db538db809ddf20da29fd5ac) )
ROM_END

static DRIVER_INIT( gijoe )
{
	konami_rom_deinterleave_2(machine, "gfx1");
	konami_rom_deinterleave_4(machine, "gfx2");
}

GAME( 1992, gijoe,  0,     gijoe, gijoe, gijoe, ROT0, "Konami", "GI Joe (World)", 0)
GAME( 1992, gijoeu, gijoe, gijoe, gijoe, gijoe, ROT0, "Konami", "GI Joe (US)", 0)
GAME( 1992, gijoej, gijoe, gijoe, gijoe, gijoe, ROT0, "Konami", "GI Joe (Japan)", 0)
