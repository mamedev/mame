/***************************************************************************

Asterix

TODO:
the konami logo: in the original the outline is drawn, then there's a slight
delay of 1 or 2 seconds, then it fills from the top to the bottom with the
colour, including the word "Konami"

***************************************************************************/

#include "driver.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/eepromdev.h"
#include "sound/2151intf.h"
#include "sound/k053260.h"
#include "video/konicdev.h"
#include "includes/konamipt.h"

VIDEO_UPDATE( asterix );
WRITE16_HANDLER( asterix_spritebank_w );

extern void asterix_tile_callback(running_machine *machine, int layer, int *code, int *color, int *flags);
extern void asterix_sprite_callback(running_machine *machine, int *code, int *color, int *priority_mask);

static UINT8 cur_control2;

static const eeprom_interface eeprom_intf =
{
	7,				/* address bits */
	8,				/* data bits */
	"111000",		/*  read command */
	"111100",		/* write command */
	"1100100000000",/* erase command */
	"1100000000000",/* lock command */
	"1100110000000" /* unlock command */
};

#if 0
static READ16_HANDLER( control2_r )
{
	return cur_control2;
}
#endif

static WRITE16_HANDLER( control2_w )
{
	const device_config *k056832 = devtag_get_device(space->machine, "k056832");

	if (ACCESSING_BITS_0_7)
	{
		cur_control2 = data;
		/* bit 0 is data */
		/* bit 1 is cs (active low) */
		/* bit 2 is clock (active high) */
		input_port_write(space->machine, "EEPROMOUT", data, 0xff);

		/* bit 5 is select tile bank */
		k056832_set_tile_bank(k056832, (data & 0x20) >> 5);
	}
}

static INTERRUPT_GEN( asterix_interrupt )
{
	const device_config *k056832 = devtag_get_device(device->machine, "k056832");

	// global interrupt masking
	if (!k056832_is_irq_enabled(k056832, 0)) return;

	cpu_set_input_line(device, 5, HOLD_LINE); /* ??? All irqs have the same vector, and the
                                              mask used is 0 or 7 */
}

static READ8_DEVICE_HANDLER( asterix_sound_r )
{
	return k053260_r(device, 2 + offset);
}

static TIMER_CALLBACK( nmi_callback )
{
	cputag_set_input_line(machine, "audiocpu", INPUT_LINE_NMI, ASSERT_LINE);
}

static WRITE8_HANDLER( sound_arm_nmi_w )
{
	cputag_set_input_line(space->machine, "audiocpu", INPUT_LINE_NMI, CLEAR_LINE);
	timer_set(space->machine, ATTOTIME_IN_USEC(5), NULL, 0, nmi_callback);
}

static WRITE16_HANDLER( sound_irq_w )
{
	cputag_set_input_line(space->machine, "audiocpu", 0, HOLD_LINE);
}

// Check the routine at 7f30 in the ead version.
// You're not supposed to laugh.
// This emulation is grossly overkill but hey, I'm having fun.

static UINT16 prot[2];

static WRITE16_HANDLER( protection_w )
{
	COMBINE_DATA(prot + offset);

	if (offset == 1)
	{
		UINT32 cmd = (prot[0] << 16) | prot[1];
		switch (cmd >> 24)
		{
		case 0x64:
		{
			UINT32 param1 = (memory_read_word(space, cmd & 0xffffff) << 16)
				| memory_read_word(space, (cmd & 0xffffff) + 2);
			UINT32 param2 = (memory_read_word(space, (cmd & 0xffffff) + 4) << 16)
				| memory_read_word(space, (cmd & 0xffffff) + 6);

			switch (param1 >> 24)
			{
			case 0x22:
			{
				int size = param2 >> 24;
				param1 &= 0xffffff;
				param2 &= 0xffffff;
				while(size >= 0)
				{
					memory_write_word(space, param2, memory_read_word(space, param1));
					param1 += 2;
					param2 += 2;
					size--;
				}
				break;
			}
			}
			break;
		}
		}
	}
}



static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x107fff) AM_RAM
	AM_RANGE(0x180000, 0x1807ff) AM_DEVREADWRITE("k053244", k053245_word_r, k053245_word_w)
	AM_RANGE(0x180800, 0x180fff) AM_RAM								// extra RAM, or mirror for the above?
	AM_RANGE(0x200000, 0x20000f) AM_DEVREADWRITE("k053244", k053244_word_r, k053244_word_w)
	AM_RANGE(0x280000, 0x280fff) AM_RAM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x300000, 0x30001f) AM_DEVREADWRITE("k053244", k053244_lsb_r, k053244_lsb_w)
	AM_RANGE(0x380000, 0x380001) AM_READ_PORT("IN0")
	AM_RANGE(0x380002, 0x380003) AM_READ_PORT("IN1")
	AM_RANGE(0x380100, 0x380101) AM_WRITE(control2_w)
	AM_RANGE(0x380200, 0x380203) AM_DEVREADWRITE8("konami", asterix_sound_r, k053260_w, 0x00ff)
	AM_RANGE(0x380300, 0x380301) AM_WRITE(sound_irq_w)
	AM_RANGE(0x380400, 0x380401) AM_WRITE(asterix_spritebank_w)
	AM_RANGE(0x380500, 0x38051f) AM_DEVWRITE("k053251", k053251_lsb_w)
	AM_RANGE(0x380600, 0x380601) AM_NOP								// Watchdog
	AM_RANGE(0x380700, 0x380707) AM_DEVWRITE("k056832", k056832_b_word_w)
	AM_RANGE(0x380800, 0x380803) AM_WRITE(protection_w)
	AM_RANGE(0x400000, 0x400fff) AM_DEVREADWRITE("k056832", k056832_ram_half_word_r, k056832_ram_half_word_w)
	AM_RANGE(0x420000, 0x421fff) AM_DEVREAD("k056832", k056832_old_rom_word_r)	// Passthrough to tile roms
	AM_RANGE(0x440000, 0x44003f) AM_DEVWRITE("k056832", k056832_word_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xefff) AM_ROM
	AM_RANGE(0xf000, 0xf7ff) AM_RAM
	AM_RANGE(0xf801, 0xf801) AM_DEVREADWRITE("ymsnd", ym2151_status_port_r, ym2151_data_port_w)
	AM_RANGE(0xfa00, 0xfa2f) AM_DEVREADWRITE("konami", k053260_r, k053260_w)
	AM_RANGE(0xfc00, 0xfc00) AM_WRITE(sound_arm_nmi_w)
	AM_RANGE(0xfe00, 0xfe00) AM_DEVWRITE("ymsnd", ym2151_register_port_w)
ADDRESS_MAP_END



static INPUT_PORTS_START( asterix )
	PORT_START("IN0")
	KONAMI16_LSB(1, IPT_UNKNOWN, IPT_START1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0xf800, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	KONAMI16_LSB(2, IPT_UNKNOWN, IPT_START2)
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE("eeprom", eepromdev_read_bit)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_UNUSED )	// EEPROM ready (always 1)
	PORT_SERVICE_NO_TOGGLE(0x0400, IP_ACTIVE_LOW )
	PORT_BIT( 0xf800, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eepromdev_write_bit)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eepromdev_set_cs_line)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eepromdev_set_clock_line)
INPUT_PORTS_END


static const k056832_interface asterix_k056832_intf =
{
	"gfx1", 0,
	K056832_BPP_4,
	1, 1,
	KONAMI_ROM_DEINTERLEAVE_2,
	asterix_tile_callback, "none"
};

static const k05324x_interface asterix_k05324x_intf =
{
	"gfx2", 1,
	NORMAL_PLANE_ORDER,
	-3, -1,
	KONAMI_ROM_DEINTERLEAVE_2,
	asterix_sprite_callback
};

static MACHINE_DRIVER_START( asterix )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 12000000)
	MDRV_CPU_PROGRAM_MAP(main_map)
	MDRV_CPU_VBLANK_INT("screen", asterix_interrupt)

	MDRV_CPU_ADD("audiocpu", Z80, 8000000)
	MDRV_CPU_PROGRAM_MAP(sound_map)

	MDRV_EEPROM_NODEFAULT_ADD("eeprom", eeprom_intf)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_HAS_SHADOWS)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(14*8, (64-14)*8-1, 2*8, 30*8-1 )
	MDRV_PALETTE_LENGTH(2048)

	MDRV_VIDEO_UPDATE(asterix)

	MDRV_K056832_ADD("k056832", asterix_k056832_intf)
	MDRV_K053244_ADD("k053244", asterix_k05324x_intf)
	MDRV_K053251_ADD("k053251")

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2151, 4000000)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)

	MDRV_SOUND_ADD("konami", K053260, 4000000)
	MDRV_SOUND_ROUTE(0, "lspeaker", 0.75)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.75)
MACHINE_DRIVER_END


ROM_START( asterix )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "068_ea_d01.8c", 0x000000,  0x20000, CRC(61d6621d) SHA1(908a344e9bbce0c7544bd049494258d1d3ad073b) )
	ROM_LOAD16_BYTE( "068_ea_d02.8d", 0x000001,  0x20000, CRC(53aac057) SHA1(7401ca5b70f384688c3353fc1ac9ef0b27814c66) )
	ROM_LOAD16_BYTE( "068a03.7c", 0x080000,  0x20000, CRC(8223ebdc) SHA1(e4aa39e4bc1d210bdda5b0cb41d6c8006c48dd24) )
	ROM_LOAD16_BYTE( "068a04.7d", 0x080001,  0x20000, CRC(9f351828) SHA1(e03842418f08e6267eeea03362450da249af73be) )

	ROM_REGION( 0x010000, "audiocpu", 0 )
	ROM_LOAD( "068_a05.5f", 0x000000, 0x010000,  CRC(d3d0d77b) SHA1(bfa77a8bf651dc27f481e96a2d63242084cc214c) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "068a12.16k", 0x000000, 0x080000, CRC(b9da8e9c) SHA1(a46878916833923e421da0667e37620ae0b77744) )
	ROM_LOAD( "068a11.12k", 0x080000, 0x080000, CRC(7eb07a81) SHA1(672c0c60834df7816d33d88643e4575b8ca9bcc1) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD( "068a08.7k", 0x000000, 0x200000, CRC(c41278fe) SHA1(58e5f67a67ae97e0b264489828cd7e74662c5ed5) )
	ROM_LOAD( "068a07.3k", 0x200000, 0x200000, CRC(32efdbc4) SHA1(b7e8610aa22249176d82b750e2549d1eea6abe4f) )

	ROM_REGION( 0x200000, "konami", 0 )
	ROM_LOAD( "068a06.1e", 0x000000, 0x200000, CRC(6df9ec0e) SHA1(cee60312e9813bd6579f3ac7c3c2521a8e633eca) )
ROM_END

ROM_START( asterixeac )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "068_ea_c01.8c", 0x000000,  0x20000, CRC(0ccd1feb) SHA1(016d642e3a745f0564aa93f0f66d5c0f37962990) )
	ROM_LOAD16_BYTE( "068_ea_c02.8d", 0x000001,  0x20000, CRC(b0805f47) SHA1(b58306164e8fec69002656993ae80abbc8f136cd) )
	ROM_LOAD16_BYTE( "068a03.7c", 0x080000,  0x20000, CRC(8223ebdc) SHA1(e4aa39e4bc1d210bdda5b0cb41d6c8006c48dd24) )
	ROM_LOAD16_BYTE( "068a04.7d", 0x080001,  0x20000, CRC(9f351828) SHA1(e03842418f08e6267eeea03362450da249af73be) )

	ROM_REGION( 0x010000, "audiocpu", 0 )
	ROM_LOAD( "068_a05.5f", 0x000000, 0x010000,  CRC(d3d0d77b) SHA1(bfa77a8bf651dc27f481e96a2d63242084cc214c) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "068a12.16k", 0x000000, 0x080000, CRC(b9da8e9c) SHA1(a46878916833923e421da0667e37620ae0b77744) )
	ROM_LOAD( "068a11.12k", 0x080000, 0x080000, CRC(7eb07a81) SHA1(672c0c60834df7816d33d88643e4575b8ca9bcc1) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD( "068a08.7k", 0x000000, 0x200000, CRC(c41278fe) SHA1(58e5f67a67ae97e0b264489828cd7e74662c5ed5) )
	ROM_LOAD( "068a07.3k", 0x200000, 0x200000, CRC(32efdbc4) SHA1(b7e8610aa22249176d82b750e2549d1eea6abe4f) )

	ROM_REGION( 0x200000, "konami", 0 )
	ROM_LOAD( "068a06.1e", 0x000000, 0x200000, CRC(6df9ec0e) SHA1(cee60312e9813bd6579f3ac7c3c2521a8e633eca) )
ROM_END

ROM_START( asterixeaa )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "068_ea_a01.8c", 0x000000,  0x20000, CRC(85b41d8e) SHA1(e1326f6d61b8097f5201d5bd37e4d2a357d17b47) )
	ROM_LOAD16_BYTE( "068_ea_a02.8d", 0x000001,  0x20000, CRC(8e886305) SHA1(41a9de2cdad8c1185b4d13ea5b4a9309716947c5) )
	ROM_LOAD16_BYTE( "068a03.7c", 0x080000,  0x20000, CRC(8223ebdc) SHA1(e4aa39e4bc1d210bdda5b0cb41d6c8006c48dd24) )
	ROM_LOAD16_BYTE( "068a04.7d", 0x080001,  0x20000, CRC(9f351828) SHA1(e03842418f08e6267eeea03362450da249af73be) )

	ROM_REGION( 0x010000, "audiocpu", 0 )
	ROM_LOAD( "068_a05.5f", 0x000000, 0x010000,  CRC(d3d0d77b) SHA1(bfa77a8bf651dc27f481e96a2d63242084cc214c) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "068a12.16k", 0x000000, 0x080000, CRC(b9da8e9c) SHA1(a46878916833923e421da0667e37620ae0b77744) )
	ROM_LOAD( "068a11.12k", 0x080000, 0x080000, CRC(7eb07a81) SHA1(672c0c60834df7816d33d88643e4575b8ca9bcc1) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD( "068a08.7k", 0x000000, 0x200000, CRC(c41278fe) SHA1(58e5f67a67ae97e0b264489828cd7e74662c5ed5) )
	ROM_LOAD( "068a07.3k", 0x200000, 0x200000, CRC(32efdbc4) SHA1(b7e8610aa22249176d82b750e2549d1eea6abe4f) )

	ROM_REGION( 0x200000, "konami", 0 )
	ROM_LOAD( "068a06.1e", 0x000000, 0x200000, CRC(6df9ec0e) SHA1(cee60312e9813bd6579f3ac7c3c2521a8e633eca) )
ROM_END

ROM_START( asterixj )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "068_ja_d01.8c", 0x000000,  0x20000, CRC(2bc10940) SHA1(e25cc97435f157bed9c28d9e9277c9f47d4fb5fb) )
	ROM_LOAD16_BYTE( "068_ja_d02.8d", 0x000001,  0x20000, CRC(de438300) SHA1(8d72988409e6c28a06fb2325087d27ebd2d02c92) )
	ROM_LOAD16_BYTE( "068a03.7c", 0x080000,  0x20000, CRC(8223ebdc) SHA1(e4aa39e4bc1d210bdda5b0cb41d6c8006c48dd24) )
	ROM_LOAD16_BYTE( "068a04.7d", 0x080001,  0x20000, CRC(9f351828) SHA1(e03842418f08e6267eeea03362450da249af73be) )

	ROM_REGION( 0x010000, "audiocpu", 0 )
	ROM_LOAD( "068_a05.5f", 0x000000, 0x010000,  CRC(d3d0d77b) SHA1(bfa77a8bf651dc27f481e96a2d63242084cc214c) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "068a12.16k", 0x000000, 0x080000, CRC(b9da8e9c) SHA1(a46878916833923e421da0667e37620ae0b77744) )
	ROM_LOAD( "068a11.12k", 0x080000, 0x080000, CRC(7eb07a81) SHA1(672c0c60834df7816d33d88643e4575b8ca9bcc1) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD( "068a08.7k", 0x000000, 0x200000, CRC(c41278fe) SHA1(58e5f67a67ae97e0b264489828cd7e74662c5ed5) )
	ROM_LOAD( "068a07.3k", 0x200000, 0x200000, CRC(32efdbc4) SHA1(b7e8610aa22249176d82b750e2549d1eea6abe4f) )

	ROM_REGION( 0x200000, "konami", 0 )
	ROM_LOAD( "068a06.1e", 0x000000, 0x200000, CRC(6df9ec0e) SHA1(cee60312e9813bd6579f3ac7c3c2521a8e633eca) )
ROM_END


static DRIVER_INIT( asterix )
{
#if 0
	*(UINT16 *)(memory_region(machine, "maincpu") + 0x07f34) = 0x602a;
	*(UINT16 *)(memory_region(machine, "maincpu") + 0x00008) = 0x0400;
#endif
}


GAME( 1992, asterix,    0,       asterix, asterix, asterix, ROT0, "Konami", "Asterix (ver EAD)", GAME_IMPERFECT_GRAPHICS )
GAME( 1992, asterixeac, asterix, asterix, asterix, asterix, ROT0, "Konami", "Asterix (ver EAC)", GAME_IMPERFECT_GRAPHICS )
GAME( 1992, asterixeaa, asterix, asterix, asterix, asterix, ROT0, "Konami", "Asterix (ver EAA)", GAME_IMPERFECT_GRAPHICS )
GAME( 1992, asterixj,   asterix, asterix, asterix, asterix, ROT0, "Konami", "Asterix (ver JAD)", GAME_IMPERFECT_GRAPHICS )
