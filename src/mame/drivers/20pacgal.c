/***************************************************************************

    Ms.Pac-Man/Galaga - 20 Year Reunion hardware

    driver by Nicola Salmoria

    Notes:
        * There are four start buttons: the first two are for Ms. Pac-Man, the other two
          for Galaga.
        * To play Pac-Man instead of Ms. Pac-Man, insert coins then enter the following
          sequence: U U U D D D L R L R L. A sound will play and the ghost will change
          from red to pink.
        * To toggle the built-in speedup, insert coins then enter the following sequence:
          L R L R U U U Fire.  A sound will play if you did it correctly.  This will toggle
          the speed in both Ms Pacman & Pacman as well as provide a "Fast Shot" in Galaga
        * Writes to the Z180 ASCI port:
          MS PAC-MAN/GALAGA
          arcade video system
          version 1.01
          (c) 2000 Cosmodog, Ltd.
          >
          and it listens for incoming characters.
        * CPU is a Z8S18020VSC (20MHz part), OSC is 73.728MHz

    Known issues/to-do's:
        * ROM banking is not understood.  Shouldn't require copying and other
          trickey
        * Starfield missing
        * Check the ASCI interface, there probably is fully working debug code.
        * The timed interrupt is a kludge; it is supposed to be generated internally by
          the Z180, but the cpu core doesn't support that yet.
        * Is the clock divide 3 or 4?

    Versions known to exist but not dumped: v1.02 & v1.03

***************************************************************************/

#include "driver.h"
#include "cpu/z180/z180.h"
#include "machine/eepromdev.h"
#include "sound/namco.h"
#include "sound/dac.h"
#include "20pacgal.h"



/*************************************
 *
 *  Clocks
 *
 *************************************/

#define MASTER_CLOCK		(XTAL_73_728MHz)
#define MAIN_CPU_CLOCK		(MASTER_CLOCK / 4)  /* divider is either 3 or 4 */
#define NAMCO_AUDIO_CLOCK	(MASTER_CLOCK / 4 /  6 / 32)



/*************************************
 *
 *  Interrupt system
 *
 *************************************/

static WRITE8_HANDLER( irqack_w )
{
	int bit = data & 1;

	cpu_interrupt_enable(cputag_get_cpu(space->machine, "maincpu"), bit);

	if (!bit)
		cputag_set_input_line(space->machine, "maincpu", 0, CLEAR_LINE);
}



/*************************************
 *
 *  Audio
 *
 *************************************/

static const namco_interface namco_config =
{
	3,		/* number of voices */
	0		/* stereo */
};



/*************************************
 *
 *  Non-volatile memory
 *
 *************************************/

static const eeprom_interface _20pacgal_eeprom_intf =
{
	7,                /* address bits */
	8,                /* data bits */
	"*110",           /* read command */
	"*101",           /* write command */
	0,                /* erase command */
	"*10000xxxxx",    /* lock command */
	"*10011xxxxx",    /* unlock command */
};


static READ8_DEVICE_HANDLER( eeprom_r )
{
	_20pacgal_state *state = (_20pacgal_state *)device->machine->driver_data;

	/* bit 7 is EEPROM data */
	return eepromdev_read_bit(state->eeprom) << 7;
}


static WRITE8_DEVICE_HANDLER( eeprom_w )
{
	_20pacgal_state *state = (_20pacgal_state *)device->machine->driver_data;

	/* bit 7 is data */
	/* bit 6 is clock (active high) */
	/* bit 5 is cs (active low) */
	eepromdev_write_bit(state->eeprom, data & 0x80);
	eepromdev_set_cs_line(state->eeprom, (data & 0x20) ? CLEAR_LINE : ASSERT_LINE);
	eepromdev_set_clock_line(state->eeprom, (data & 0x40) ? ASSERT_LINE : CLEAR_LINE);
}



/*************************************
 *
 *  Coin counter
 *
 *************************************/

static WRITE8_HANDLER( _20pacgal_coin_counter_w )
{
	coin_counter_w(0, data & 1);
}



/*************************************
 *
 *  ROM banking - FIXME
 *
 *************************************/

static WRITE8_HANDLER( rom_bank_select_w )
{
	_20pacgal_state *state = (_20pacgal_state *)space->machine->driver_data;

	state->game_selected = data & 1;

	if (state->game_selected == 0)
	{
		UINT8 *rom = memory_region(space->machine, "maincpu");
		memcpy(rom + 0x48000, rom + 0x8000, 0x2000);
	}
}


static WRITE8_HANDLER( rom_48000_w )
{
	_20pacgal_state *state = (_20pacgal_state *)space->machine->driver_data;

	if (state->game_selected)
	{
		if (offset < 0x0800)
			state->video_ram[offset & 0x07ff] = data;

		memory_region(space->machine, "maincpu")[0x48000 + offset] = data;
	}
}



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( 20pacgal_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x00000, 0x03fff) AM_ROM
	AM_RANGE(0x08000, 0x09fff) AM_ROM
	AM_RANGE(0x0a000, 0x0ffff) AM_MIRROR(0x40000) AM_ROM
	AM_RANGE(0x10000, 0x3ffff) AM_ROM
	AM_RANGE(0x44000, 0x447ff) AM_RAM AM_BASE_MEMBER(_20pacgal_state, video_ram)
	AM_RANGE(0x45040, 0x4505f) AM_DEVWRITE("namco", pacman_sound_w) AM_BASE(&namco_soundregs)
	AM_RANGE(0x44800, 0x45eff) AM_RAM
	AM_RANGE(0x45f00, 0x45fff) AM_DEVWRITE("namco", _20pacgal_wavedata_w) AM_BASE(&namco_wavedata)
	AM_RANGE(0x46000, 0x46fff) AM_WRITEONLY AM_BASE_MEMBER(_20pacgal_state, char_gfx_ram)
	AM_RANGE(0x47100, 0x47100) AM_RAM	/* leftover from original Galaga code */
	AM_RANGE(0x48000, 0x49fff) AM_ROM AM_WRITE(rom_48000_w)	/* this should be a mirror of 08000-09ffff */
	AM_RANGE(0x4c000, 0x4dfff) AM_WRITE(SMH_RAM) AM_BASE_MEMBER(_20pacgal_state, sprite_gfx_ram)
	AM_RANGE(0x4e000, 0x4e17f) AM_WRITE(SMH_RAM) AM_BASE_MEMBER(_20pacgal_state, sprite_ram)
	AM_RANGE(0x4ff00, 0x4ffff) AM_WRITE(SMH_RAM) AM_BASE_MEMBER(_20pacgal_state, sprite_color_lookup)
ADDRESS_MAP_END



/*************************************
 *
 *  I/O port handlers
 *
 *************************************/

static ADDRESS_MAP_START( 20pacgal_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x3f) AM_NOP /* Z180 internal registers */
	AM_RANGE(0x40, 0x7f) AM_NOP	/* Z180 internal registers */
	AM_RANGE(0x80, 0x80) AM_READ_PORT("P1")
	AM_RANGE(0x81, 0x81) AM_READ_PORT("P2")
	AM_RANGE(0x82, 0x82) AM_READ_PORT("SERVICE")
	AM_RANGE(0x80, 0x80) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x81, 0x81) AM_WRITENOP				/* ??? pulsed by the timer irq */
	AM_RANGE(0x82, 0x82) AM_WRITE(irqack_w)
	AM_RANGE(0x85, 0x86) AM_WRITENOP				/* stars: rng seed (lo/hi) */
	AM_RANGE(0x87, 0x87) AM_DEVREADWRITE("eeprom", eeprom_r, eeprom_w)
	AM_RANGE(0x88, 0x88) AM_WRITE(rom_bank_select_w)
	AM_RANGE(0x89, 0x89) AM_DEVWRITE("dac", dac_signed_w)
	AM_RANGE(0x8a, 0x8a) AM_WRITENOP				/* stars: bits 3-4 = active set; bit 5 = enable */
	AM_RANGE(0x8b, 0x8b) AM_WRITE(SMH_RAM) AM_BASE_MEMBER(_20pacgal_state, flip)
	AM_RANGE(0x8f, 0x8f) AM_WRITE(_20pacgal_coin_counter_w)
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( 20pacgal )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START3 ) PORT_NAME( "Right 1 Player Start" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME( "Left 1 Player Start" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 ) PORT_NAME( "Left 2 Players Start" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START4 ) PORT_NAME( "Right 2 Players Start" )

	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x80, IP_ACTIVE_LOW )
INPUT_PORTS_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_START( 20pacgal )
{
	_20pacgal_state *state = (_20pacgal_state *)machine->driver_data;

	state->eeprom = devtag_get_device(machine, "eeprom");

	state_save_register_global(machine, state->game_selected);
}

static MACHINE_RESET( 20pacgal )
{
	_20pacgal_state *state = (_20pacgal_state *)machine->driver_data;

	state->game_selected = 0;
}

static MACHINE_DRIVER_START( 20pacgal )

	MDRV_DRIVER_DATA(_20pacgal_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z180, MAIN_CPU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(20pacgal_map)
	MDRV_CPU_IO_MAP(20pacgal_io_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_assert)

	MDRV_MACHINE_START(20pacgal)
	MDRV_MACHINE_RESET(20pacgal)

	MDRV_EEPROM_NODEFAULT_ADD("eeprom", _20pacgal_eeprom_intf)

	/* video hardware */
	MDRV_IMPORT_FROM(20pacgal_video)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("namco", NAMCO, NAMCO_AUDIO_CLOCK)
	MDRV_SOUND_CONFIG(namco_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MDRV_SOUND_ADD("dac", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definition
 *
 *************************************/

ROM_START( 20pacgal ) /* Version 1.04 */
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "20th_104.u13", 0x00000, 0x40000, CRC(6c474d2d) SHA1(5a150fc9d2ed0e908385b9f9d532aa33cf80dba4) )

	ROM_REGION( 0x8000, "proms", 0 )	/* palette */
	ROM_LOAD( "20th_101.u14", 0x0000, 0x8000, CRC(c19d9ad0) SHA1(002581fbc2c32cdf7cfb0b0f64061591a462ec14) )
ROM_END

ROM_START( 20pacgala ) /* Version 1.01 */
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "20th_101.u13", 0x00000, 0x40000, CRC(77159582) SHA1(c05e005a941cbdc806dcd76b315069362c792a72) )

	ROM_REGION( 0x8000, "proms", 0 )	/* palette */
	ROM_LOAD( "20th_101.u14", 0x0000, 0x8000, CRC(c19d9ad0) SHA1(002581fbc2c32cdf7cfb0b0f64061591a462ec14) )
ROM_END



/*************************************
 *
 *  Game driver
 *
 *************************************/

GAME( 2000, 20pacgal,         0, 20pacgal, 20pacgal, 0, ROT90, "Namco", "Ms. Pac-Man/Galaga - 20 Year Reunion (V1.04)", GAME_IMPERFECT_GRAPHICS )
GAME( 2000, 20pacgala, 20pacgal, 20pacgal, 20pacgal, 0, ROT90, "Namco", "Ms. Pac-Man/Galaga - 20 Year Reunion (V1.01)", GAME_IMPERFECT_GRAPHICS )
