/*
    Photon System

    Uses PK8000 emulation by Miodrag Milanovic
    Imported to MAME by Mariusz Wojcieszek

    Russian arcade system based on PK8000 home computer, created by unknown manufacturer
    in late 1980s or early 1990s.

    Following games were produced for this system:
    - Tetris
    - Python
    - Treasure/Labyrinth

    Dump was made using custom adaptor, hence it is marked as bad dump.
    The real machine has following roms:
    0000...07FFh - ROM1 (D41)
    0800...0FFFh - ROM2 (D42)
    1000...17FFh - ROM3 (D43)
    1800...1FFFh - not chip sealed (D44)
    2000...27FFh - ROM5 (D45)
    2800...2FFFh - ROM6 (D46)
    3000...37FFh - ROM7 (D47)
    3000...37FFh - ROM8 (D48)

*/

#include "driver.h"
#include "cpu/i8085/i8085.h"
#include "includes/pk8000.h"
#include "machine/i8255a.h"
#include "sound/dac.h"

static void pk8000_set_bank(running_machine *machine,UINT8 data)
{
	UINT8 *rom = memory_region(machine, "maincpu");
	UINT8 *ram = memory_region(machine, "maincpu");
	UINT8 block1 = data & 3;
	UINT8 block2 = (data >> 2) & 3;
	UINT8 block3 = (data >> 4) & 3;
	UINT8 block4 = (data >> 6) & 3;

	switch(block1) {
		case 0:
				memory_set_bankptr(machine, 1, rom + 0x10000);
				memory_set_bankptr(machine, 5, ram);
				break;
		case 1: break;
		case 2: break;
		case 3:
				memory_set_bankptr(machine, 1, ram);
				memory_set_bankptr(machine, 5, ram);
				break;
	}

	switch(block2) {
		case 0:
				memory_set_bankptr(machine, 2, rom + 0x14000);
				memory_set_bankptr(machine, 6, ram + 0x4000);
				break;
		case 1: break;
		case 2: break;
		case 3:
				memory_set_bankptr(machine, 2, ram + 0x4000);
				memory_set_bankptr(machine, 6, ram + 0x4000);
				break;
	}
	switch(block3) {
		case 0:
				memory_set_bankptr(machine, 3, rom + 0x18000);
				memory_set_bankptr(machine, 7, ram + 0x8000);
				break;
		case 1: break;
		case 2: break;
		case 3:
				memory_set_bankptr(machine, 3, ram + 0x8000);
				memory_set_bankptr(machine, 7, ram + 0x8000);
				break;
	}
	switch(block4) {
		case 0:
				memory_set_bankptr(machine, 4, rom + 0x1c000);
				memory_set_bankptr(machine, 8, ram + 0xc000);
				break;
		case 1: break;
		case 2: break;
		case 3:
				memory_set_bankptr(machine, 4, ram + 0xc000);
				memory_set_bankptr(machine, 8, ram + 0xc000);
				break;
	}
}
static WRITE8_DEVICE_HANDLER(pk8000_80_porta_w)
{
	pk8000_set_bank(device->machine,data);
}

static READ8_DEVICE_HANDLER(pk8000_80_portb_r)
{
	return 0xff;
}

static WRITE8_DEVICE_HANDLER(pk8000_80_portc_w)
{
	dac_signed_data_w (devtag_get_device(device->machine, "dac"), (BIT(data,7) ? 0 : 0x7f));
}

static I8255A_INTERFACE( pk8000_ppi8255_interface_1 )
{
	DEVCB_NULL,
	DEVCB_HANDLER(pk8000_80_portb_r),
	DEVCB_NULL,
	DEVCB_HANDLER(pk8000_80_porta_w),
	DEVCB_NULL,
	DEVCB_HANDLER(pk8000_80_portc_w)
};

static READ8_DEVICE_HANDLER(pk8000_84_porta_r)
{
	return pk8000_video_mode;
}

static WRITE8_DEVICE_HANDLER(pk8000_84_porta_w)
{
	pk8000_video_mode = data;
}

static WRITE8_DEVICE_HANDLER(pk8000_84_portc_w)
{
	pk8000_video_enable = BIT(data,4);
}
static I8255A_INTERFACE( pk8000_ppi8255_interface_2 )
{
	DEVCB_HANDLER(pk8000_84_porta_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_HANDLER(pk8000_84_porta_w),
	DEVCB_NULL,
	DEVCB_HANDLER(pk8000_84_portc_w)
};

static ADDRESS_MAP_START(pk8000_mem, ADDRESS_SPACE_PROGRAM, 8)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x3fff ) AM_READWRITE(SMH_BANK(1), SMH_BANK(5))
	AM_RANGE( 0x4000, 0x7fff ) AM_READWRITE(SMH_BANK(2), SMH_BANK(6))
	AM_RANGE( 0x8000, 0xbfff ) AM_READWRITE(SMH_BANK(3), SMH_BANK(7))
	AM_RANGE( 0xc000, 0xffff ) AM_READWRITE(SMH_BANK(4), SMH_BANK(8))
ADDRESS_MAP_END

static ADDRESS_MAP_START( pk8000_io , ADDRESS_SPACE_IO, 8)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x80, 0x83) AM_DEVREADWRITE("ppi8255_1", i8255a_r, i8255a_w)
	AM_RANGE(0x84, 0x87) AM_DEVREADWRITE("ppi8255_2", i8255a_r, i8255a_w)
	AM_RANGE(0x88, 0x88) AM_READWRITE(pk8000_video_color_r,pk8000_video_color_w)
	AM_RANGE(0x8c, 0x8c) AM_READ_PORT("JOY1")
	AM_RANGE(0x8d, 0x8d) AM_READ_PORT("JOY2")
	AM_RANGE(0x90, 0x90) AM_READWRITE(pk8000_text_start_r,pk8000_text_start_w)
	AM_RANGE(0x91, 0x91) AM_READWRITE(pk8000_chargen_start_r,pk8000_chargen_start_w)
	AM_RANGE(0x92, 0x92) AM_READWRITE(pk8000_video_start_r,pk8000_video_start_w)
	AM_RANGE(0x93, 0x93) AM_READWRITE(pk8000_color_start_r,pk8000_color_start_w)
	AM_RANGE(0xa0, 0xbf) AM_READWRITE(pk8000_color_r,pk8000_color_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( photon )
	PORT_START("JOY1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP) PORT_PLAYER(1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN) PORT_PLAYER(1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT) PORT_PLAYER(1)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_PLAYER(1)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_COIN1)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("JOY2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP) PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN) PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT) PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_COIN2)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END


INTERRUPT_GEN( pk8000_interrupt )
{
	cpu_set_input_line(device, 0, HOLD_LINE);
}

static IRQ_CALLBACK(pk8000_irq_callback)
{
	return 0xff;
}


static MACHINE_RESET(pk8000)
{
	pk8000_set_bank(machine,0);
	cpu_set_irq_callback(cputag_get_cpu(machine, "maincpu"), pk8000_irq_callback);
}

static VIDEO_START( photon )
{
}

static VIDEO_UPDATE( photon )
{
	return pk8000_video_update(screen, bitmap, cliprect, memory_region(screen->machine, "maincpu"));
}

static MACHINE_DRIVER_START( photon )

    /* basic machine hardware */
    MDRV_CPU_ADD("maincpu",8080, 1780000)
    MDRV_CPU_PROGRAM_MAP(pk8000_mem)
    MDRV_CPU_IO_MAP(pk8000_io)
    MDRV_CPU_VBLANK_INT("screen", pk8000_interrupt)

    MDRV_MACHINE_RESET(pk8000)

    /* video hardware */
    MDRV_SCREEN_ADD("screen", RASTER)
    MDRV_SCREEN_REFRESH_RATE(50)
    MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
    MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
    MDRV_SCREEN_SIZE(256+32, 192+32)
    MDRV_SCREEN_VISIBLE_AREA(0, 256+32-1, 0, 192+32-1)
    MDRV_PALETTE_LENGTH(16)
    MDRV_PALETTE_INIT(pk8000)

    MDRV_VIDEO_START(photon)
    MDRV_VIDEO_UPDATE(photon)

    MDRV_I8255A_ADD( "ppi8255_1", pk8000_ppi8255_interface_1 )
    MDRV_I8255A_ADD( "ppi8255_2", pk8000_ppi8255_interface_2 )

    /* audio hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("dac", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END


ROM_START( phtetris )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "foton_tetris.bin", 0x10000, 0x4000, BAD_DUMP CRC(a8af10bb) SHA1(5e2ea9a5d38399cbe156638eea73a3d25c442f77) )
ROM_END

GAME( 19??, phtetris, 0,      photon, photon, 0, ROT0, "<unknown>", "Tetris (Photon System)", 0 )
