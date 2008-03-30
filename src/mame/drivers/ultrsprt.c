/*  Konami Ultra Sports hardware

    Driver by Ville Linde

*/

#include "driver.h"
#include "deprecat.h"
#include "cpu/powerpc/ppc.h"
#include "sound/k054539.h"
#include "machine/eeprom.h"
#include "machine/konppc.h"
#include "machine/konamiic.h"

static UINT32 *vram;

static VIDEO_UPDATE( ultrsprt )
{
	int i, j;

	UINT8 *ram = (UINT8 *)vram;

	for (j=0; j < 400; j++)
	{
		UINT16 *dest = BITMAP_ADDR16(bitmap, j, 0);
		int fb_index = j * 1024;

		for (i=0; i < 512; i++)
		{
			UINT8 p1 = ram[BYTE4_XOR_BE(fb_index + i + 512)];
			if (p1 == 0)
				dest[i] = ram[BYTE4_XOR_BE(fb_index + i)];
			else
				dest[i] = 0x100 + p1;
		}
	}

	return 0;
}

static WRITE32_HANDLER( palette_w )
{
	COMBINE_DATA(&paletteram32[offset]);
	data = paletteram32[offset];

	palette_set_color(machine, (offset*2)+0, MAKE_RGB(pal5bit(data >> 26), pal5bit(data >> 21), pal5bit(data >> 16)));
	palette_set_color(machine, (offset*2)+1, MAKE_RGB(pal5bit(data >> 10), pal5bit(data >>  5), pal5bit(data >>  0)));
}

static READ32_HANDLER( eeprom_r )
{
	UINT32 r = 0;

	if (ACCESSING_BYTE_3)
		r |= (((EEPROM_read_bit()) << 1) | (readinputport(6) << 3)) << 24;

	return r;
}

static WRITE32_HANDLER( eeprom_w )
{
	if (ACCESSING_BYTE_3)
	{
		EEPROM_write_bit((data & 0x01000000) ? 1 : 0);
		EEPROM_set_clock_line((data & 0x02000000) ? CLEAR_LINE : ASSERT_LINE);
		EEPROM_set_cs_line((data & 0x04000000) ? CLEAR_LINE : ASSERT_LINE);
	}
}

static READ32_HANDLER( control1_r )
{
	return (readinputport(0) << 28) | ((readinputport(1) & 0xfff) << 16) | (readinputport(2) & 0xfff);
}

static READ32_HANDLER( control2_r )
{
	return (readinputport(3) << 28) | ((readinputport(4) & 0xfff) << 16) | (readinputport(5) & 0xfff);
}

static WRITE32_HANDLER( int_ack_w )
{
	cpunum_set_input_line(machine, 0, INPUT_LINE_IRQ1, CLEAR_LINE);
}

static ADDRESS_MAP_START( ultrsprt_map, ADDRESS_SPACE_PROGRAM, 32 )
	ADDRESS_MAP_GLOBAL_MASK(0x7fffffff)
	AM_RANGE(0x00000000, 0x0007ffff) AM_RAM AM_BASE(&vram)
	AM_RANGE(0x70000000, 0x70000003) AM_READWRITE(eeprom_r, eeprom_w)
	AM_RANGE(0x70000020, 0x70000023) AM_READ(control1_r)
	AM_RANGE(0x70000040, 0x70000043) AM_READ(control2_r)
	AM_RANGE(0x70000080, 0x70000087) AM_WRITE(K056800_host_w)
	AM_RANGE(0x70000088, 0x7000008f) AM_READ(K056800_host_r)
	AM_RANGE(0x700000e0, 0x700000e3) AM_WRITE(int_ack_w)
	AM_RANGE(0x7f000000, 0x7f01ffff) AM_RAM
	AM_RANGE(0x7f700000, 0x7f703fff) AM_READWRITE(SMH_RAM, palette_w) AM_BASE(&paletteram32)
	AM_RANGE(0x7fa00000, 0x7fbfffff) AM_ROM AM_SHARE(1)
	AM_RANGE(0x7fc00000, 0x7fdfffff) AM_ROM AM_SHARE(1)
	AM_RANGE(0x7fe00000, 0x7fffffff) AM_ROM AM_REGION(REGION_USER1, 0) AM_SHARE(1)
ADDRESS_MAP_END


/*****************************************************************************/


static READ16_HANDLER( sound_r )
{
	UINT16 r = 0;
	int reg = offset * 2;

	if (ACCESSING_BYTE_1)
		r |= K054539_0_r(machine, reg+0) << 8;

	if (ACCESSING_BYTE_0)
		r |= K054539_0_r(machine, reg+1) << 0;

	return r;
}

static WRITE16_HANDLER( sound_w )
{
	int reg = offset * 2;

	if (ACCESSING_BYTE_1)
		K054539_0_w(machine, reg+0, (data >> 8) & 0xff);

	if (ACCESSING_BYTE_0)
		K054539_0_w(machine, reg+1, (data >> 0) & 0xff);
}

static READ16_HANDLER( K056800_68k_r )
{
	UINT16 r = 0;

	if (ACCESSING_BYTE_1)
		r |= K056800_sound_r(machine, (offset*2)+0, 0xffff) << 8;

	if (ACCESSING_BYTE_0)
		r |= K056800_sound_r(machine, (offset*2)+1, 0xffff) << 0;

	return r;
}

static WRITE16_HANDLER( K056800_68k_w )
{
	if (ACCESSING_BYTE_1)
		K056800_sound_w(machine, (offset*2)+0, (data >> 8) & 0xff, 0xffff);

	if (ACCESSING_BYTE_0)
		K056800_sound_w(machine, (offset*2)+1, (data >> 0) & 0xff, 0xffff);
}

static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00000000, 0x0001ffff) AM_READ(SMH_ROM)
	AM_RANGE(0x00100000, 0x00101fff) AM_RAM
	AM_RANGE(0x00200000, 0x00200007) AM_WRITE(K056800_68k_w)
	AM_RANGE(0x00200008, 0x0020000f) AM_READ(K056800_68k_r)
	AM_RANGE(0x00400000, 0x004002ff) AM_READWRITE(sound_r, sound_w)
ADDRESS_MAP_END

/*****************************************************************************/

static INPUT_PORTS_START( ultrsprt )
	PORT_START
	PORT_BIT( 0x4, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x2, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x1, IP_ACTIVE_HIGH, IPT_START1 )

	PORT_START
	PORT_BIT( 0xfff, 0x800, IPT_AD_STICK_X ) PORT_MINMAX(0x000,0xfff) PORT_SENSITIVITY(70) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START
	PORT_BIT( 0xfff, 0x800, IPT_AD_STICK_Y ) PORT_MINMAX(0x000,0xfff) PORT_SENSITIVITY(70) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(1)

	PORT_START
	PORT_BIT( 0x4, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x2, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x1, IP_ACTIVE_HIGH, IPT_START2 )

	PORT_START
	PORT_BIT( 0xfff, 0x800, IPT_AD_STICK_X ) PORT_MINMAX(0x000,0xfff) PORT_SENSITIVITY(70) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START
	PORT_BIT( 0xfff, 0x800, IPT_AD_STICK_Y ) PORT_MINMAX(0x000,0xfff) PORT_SENSITIVITY(70) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(2)

	PORT_START
	PORT_SERVICE_NO_TOGGLE( 0x1, IP_ACTIVE_LOW )

INPUT_PORTS_END

static void eeprom_handler(mame_file *file, int read_or_write)
{
	if (read_or_write)
	{
		EEPROM_save(file);
	}
	else
	{
		EEPROM_init(&eeprom_interface_93C46);
		if (file)
		{
			EEPROM_load(file);
		}
	}
}

static NVRAM_HANDLER(ultrsprt)
{
	eeprom_handler(file, read_or_write);
}

static const ppc_config ultrsprt_ppc_cfg =
{
	PPC_MODEL_403GA
};

static const struct K054539interface k054539_interface =
{
	REGION_SOUND1
};

static INTERRUPT_GEN( ultrsprt_vblank )
{
	cpunum_set_input_line(machine, 0, INPUT_LINE_IRQ1, ASSERT_LINE);
}


static MACHINE_DRIVER_START( ultrsprt )
	/* basic machine hardware */
	MDRV_CPU_ADD(PPC403, 25000000)		/* PowerPC 403GA 25MHz */
	MDRV_CPU_CONFIG(ultrsprt_ppc_cfg)
	MDRV_CPU_PROGRAM_MAP(ultrsprt_map, 0)
	MDRV_CPU_VBLANK_INT("main", ultrsprt_vblank)

	MDRV_CPU_ADD(M68000, 8000000)		/* Not sure about the frequency */
	MDRV_CPU_PROGRAM_MAP(sound_map, 0)
	MDRV_CPU_PERIODIC_INT(irq5_line_hold, 1)	// ???

	MDRV_INTERLEAVE(200)

	MDRV_NVRAM_HANDLER(ultrsprt)

 	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(512, 400)
	MDRV_SCREEN_VISIBLE_AREA(0, 511, 0, 399)

	MDRV_PALETTE_LENGTH(8192)

	MDRV_VIDEO_UPDATE(ultrsprt)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD(K054539, 48000)
	MDRV_SOUND_CONFIG(k054539_interface)
	MDRV_SOUND_ROUTE(0, "left", 1.0)
	MDRV_SOUND_ROUTE(1, "right", 1.0)
MACHINE_DRIVER_END

static void sound_irq_callback(int irq)
{
	if (irq == 0)
		/*cpunum_set_input_line(Machine, 1, INPUT_LINE_IRQ5, PULSE_LINE)*/;
	else
		cpunum_set_input_line(Machine, 1, INPUT_LINE_IRQ6, PULSE_LINE);
}

static DRIVER_INIT( ultrsprt )
{
	cpunum_set_input_line(machine, 1, INPUT_LINE_HALT, ASSERT_LINE);

	K056800_init(sound_irq_callback);
}

/*****************************************************************************/

ROM_START( fiveside )
	ROM_REGION(0x200000, REGION_USER1, 0)	/* PowerPC program roms */
	ROM_LOAD32_BYTE("479uaa01.bin", 0x000003, 0x80000, CRC(1bc4893d) SHA1(2c9df38ecb7efa7b686221ee98fa3aad9a63e152))
	ROM_LOAD32_BYTE("479uaa02.bin", 0x000002, 0x80000, CRC(ae74a6d0) SHA1(6113c2eea1628b22737c7b87af0e673d94984e88))
	ROM_LOAD32_BYTE("479uaa03.bin", 0x000001, 0x80000, CRC(5c0b176f) SHA1(9560259bc081d4cfd72eb485c3fdcecf484ba7a8))
	ROM_LOAD32_BYTE("479uaa04.bin", 0x000000, 0x80000, CRC(01a3e4cb) SHA1(819df79909d57fa12481698ffdb32b00586131d8))

	ROM_REGION(0x20000, REGION_CPU2, 0)		/* M68K program */
	ROM_LOAD("479_a05.bin", 0x000000, 0x20000, CRC(251ae299) SHA1(5ffd74357e3c6ddb3a208c39a3b32b53fea90282))

	ROM_REGION(0x100000, REGION_SOUND1, 0)	/* Sound roms */
	ROM_LOAD("479_a06.bin", 0x000000, 0x80000, CRC(8d6ac8a2) SHA1(7c4b8bd47cddc766cbdb6a486acc9221be55b579))
	ROM_LOAD("479_a07.bin", 0x080000, 0x80000, CRC(75835df8) SHA1(105b95c16f2ce6902c2e4c9c2fd9f2f7a848c546))
ROM_END

GAME(1995, fiveside, 0, ultrsprt, ultrsprt, ultrsprt, ROT90, "Konami", "Five a Side Soccer (ver UAA)", GAME_IMPERFECT_SOUND)
