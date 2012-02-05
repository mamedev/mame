/***************************************************************************

    Pacman - 25th Anniversary Edition
    Ms.Pac-Man/Galaga - 20th Anniversary Class of 1981 Reunion

    driver by Nicola Salmoria

    Notes for Ms.Pac-Man/Galaga - 20th Anniversary:
        * There are four start buttons: the first two are for Ms. Pac-Man, the other two
          are for Galaga.
        * To play Pac-Man instead of Ms. Pac-Man, insert coins then enter the following
          sequence: U U U D D D L R L R L. A sound will play and the ghost will change
          from red to pink. Then press the start button for Ms. Pac-Man.
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

    Notes for Pacman - 25th Anniversary:
        * Pacman 25th Anniversary is a program update which allows the player to choose the
          game they want to play. You highlight the wanted game and then press the 1 or 2
          player start button to start the game.
        * There is a minor board difference that the program code can detect through the Z80
          ports to prevent ROM swaps to upgrade Ms. Pac-Man/Galaga - 20th Anniversary Class
          of 1981 Reunion boards.
        * The above listed joystick maneuver for the built-in speed up still works.
        * The above listed joystick maneuver to enable Pac-Man will still play a tone, but
          the effect (if any) is unknown.

        * CPU is a Z8S18020VSC (20MHz part), OSC is 73.728MHz

    Note: The "correct" size of the roms are 27C020 for the program rom and 27C256 for the
          graphics rom.  However genuine boards have been found with larger roms containing
          the same data with the extra rom space blanked out.

    Known issues/to-do's:
        * Starfield is not 100% accurate
        * Check the ASCI interface, there probably is fully working debug code.
        * The timed interrupt is a kludge; it is supposed to be generated internally by
          the Z180, but the cpu core doesn't support that yet.
        * Is the clock divide 3 or 4?

***************************************************************************/

#include "emu.h"
#include "cpu/z180/z180.h"
#include "machine/eeprom.h"
#include "sound/namco.h"
#include "sound/dac.h"
#include "includes/20pacgal.h"


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
	_20pacgal_state *state = space->machine().driver_data<_20pacgal_state>();

	state->m_irq_mask = data & 1;

	if (!state->m_irq_mask)
		device_set_input_line(state->m_maincpu, 0, CLEAR_LINE);
}

static WRITE8_HANDLER( timer_pulse_w )
{
	//_20pacgal_state *state = space->machine().driver_data<_20pacgal_state>();
	//printf("timer pulse %02x\n", data);
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


/*************************************
 *
 *  Coin counter
 *
 *************************************/

static WRITE8_HANDLER( _20pacgal_coin_counter_w )
{
	coin_counter_w(space->machine(), 0, data & 1);
}



/*************************************
 *
 *  ROM banking
 *
 *************************************/

static void set_bankptr(running_machine &machine)
{
	_20pacgal_state *state =  machine.driver_data<_20pacgal_state>();
	if (state->m_game_selected == 0)
	{
		UINT8 *rom = machine.region("maincpu")->base();
		memory_set_bankptr(machine, "bank1", rom + 0x08000);
	}
	else
		memory_set_bankptr(machine, "bank1", state->m_ram_48000);
}

static WRITE8_HANDLER( ram_bank_select_w )
{
	_20pacgal_state *state = space->machine().driver_data<_20pacgal_state>();

	state->m_game_selected = data & 1;
	set_bankptr(space->machine());
}

static WRITE8_HANDLER( ram_48000_w )
{
	_20pacgal_state *state = space->machine().driver_data<_20pacgal_state>();

	if (state->m_game_selected)
	{
		if (offset < 0x0800)
			state->m_video_ram[offset & 0x07ff] = data;

		state->m_ram_48000[offset] = data;
	}
}

/*************************************
 *
 *  Memory handlers
 *
 *************************************/

static WRITE8_HANDLER( sprite_gfx_w )
{
	_20pacgal_state *state = space->machine().driver_data<_20pacgal_state>();
	state->m_sprite_gfx_ram[offset] = data;
}

static WRITE8_HANDLER( sprite_ram_w )
{
	_20pacgal_state *state = space->machine().driver_data<_20pacgal_state>();
	state->m_sprite_ram[offset] = data;
}

static WRITE8_HANDLER( sprite_lookup_w )
{
	_20pacgal_state *state = space->machine().driver_data<_20pacgal_state>();
	state->m_sprite_color_lookup[offset] = data;
}

static ADDRESS_MAP_START( 20pacgal_map, AS_PROGRAM, 8 )
	AM_RANGE(0x00000, 0x03fff) AM_ROM
	AM_RANGE(0x04000, 0x07fff) AM_ROM
	AM_RANGE(0x08000, 0x09fff) AM_ROM
	AM_RANGE(0x0a000, 0x0ffff) AM_MIRROR(0x40000) AM_ROM
	AM_RANGE(0x10000, 0x3ffff) AM_ROM
	AM_RANGE(0x44000, 0x447ff) AM_RAM AM_BASE_MEMBER(_20pacgal_state, m_video_ram)
	AM_RANGE(0x45040, 0x4505f) AM_DEVWRITE("namco", pacman_sound_w)
	AM_RANGE(0x44800, 0x45eff) AM_RAM
	AM_RANGE(0x45f00, 0x45fff) AM_DEVWRITE("namco", namcos1_cus30_w)
	AM_RANGE(0x46000, 0x46fff) AM_WRITEONLY AM_BASE_MEMBER(_20pacgal_state, m_char_gfx_ram)
	AM_RANGE(0x47100, 0x47100) AM_RAM	/* leftover from original Galaga code */
	AM_RANGE(0x48000, 0x49fff) AM_READ_BANK("bank1") AM_WRITE(ram_48000_w)	/* this should be a mirror of 08000-09ffff */
	AM_RANGE(0x4c000, 0x4dfff) AM_WRITE(sprite_gfx_w)
	AM_RANGE(0x4e000, 0x4e17f) AM_WRITE(sprite_ram_w)
	AM_RANGE(0x4e180, 0x4feff) AM_WRITENOP
	AM_RANGE(0x4ff00, 0x4ffff) AM_WRITE(sprite_lookup_w)
ADDRESS_MAP_END



/*************************************
 *
 *  I/O port handlers
 *
 *************************************/

static ADDRESS_MAP_START( 20pacgal_io_map, AS_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x3f) AM_NOP	/* Z180 internal registers */
	AM_RANGE(0x40, 0x7f) AM_NOP	/* Z180 internal registers */
	AM_RANGE(0x80, 0x80) AM_READ_PORT("P1")
	AM_RANGE(0x81, 0x81) AM_READ_PORT("P2")
	AM_RANGE(0x82, 0x82) AM_READ_PORT("SERVICE")
	AM_RANGE(0x80, 0x80) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x81, 0x81) AM_WRITE(timer_pulse_w)		/* ??? pulsed by the timer irq */
	AM_RANGE(0x82, 0x82) AM_WRITE(irqack_w)
	AM_RANGE(0x84, 0x84) AM_NOP	/* ?? */
	AM_RANGE(0x85, 0x86) AM_WRITEONLY AM_BASE_MEMBER(_20pacgal_state, m_stars_seed)	/* stars: rng seed (lo/hi) */
	AM_RANGE(0x87, 0x87) AM_READ_PORT("EEPROMIN") AM_WRITE_PORT("EEPROMOUT")
	AM_RANGE(0x88, 0x88) AM_WRITE(ram_bank_select_w)
	AM_RANGE(0x89, 0x89) AM_DEVWRITE("dac", dac_signed_w)
	AM_RANGE(0x8a, 0x8a) AM_WRITEONLY AM_BASE_MEMBER(_20pacgal_state, m_stars_ctrl)	/* stars: bits 3-4 = active set; bit 5 = enable */
	AM_RANGE(0x8b, 0x8b) AM_WRITEONLY AM_BASE_MEMBER(_20pacgal_state, m_flip)
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

	PORT_START( "EEPROMIN" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_device, read_bit)	/* bit 7 is EEPROM data */

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_device, set_cs_line)		/* bit 5 is cs (active low) */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_device, set_clock_line)	/* bit 6 is clock (active high) */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_device, write_bit)		/* bit 7 is data */
INPUT_PORTS_END


static INPUT_PORTS_START( 25pacman )
	PORT_INCLUDE(20pacgal)

	PORT_MODIFY("P2")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x80, IP_ACTIVE_LOW )
INPUT_PORTS_END




/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_START( 20pacgal )
{
	_20pacgal_state *state = machine.driver_data<_20pacgal_state>();

	state->m_maincpu = machine.device("maincpu");
	state->m_eeprom = machine.device("eeprom");

	state->save_item(NAME(state->m_game_selected));
	state->save_item(NAME(state->m_ram_48000));
	state->save_item(NAME(state->m_irq_mask));

	machine.save().register_postload(save_prepost_delegate(FUNC(set_bankptr), &machine));
}

static MACHINE_RESET( 20pacgal )
{
	_20pacgal_state *state = machine.driver_data<_20pacgal_state>();

	state->m_game_selected = 0;
}

static INTERRUPT_GEN( vblank_irq )
{
	_20pacgal_state *state = device->machine().driver_data<_20pacgal_state>();

	if(state->m_irq_mask)
		device_set_input_line(device, 0, HOLD_LINE); // TODO: assert breaks the inputs in 25pacman test mode
}

static MACHINE_CONFIG_START( 20pacgal, _20pacgal_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z180, MAIN_CPU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(20pacgal_map)
	MCFG_CPU_IO_MAP(20pacgal_io_map)
	MCFG_CPU_VBLANK_INT("screen", vblank_irq)

	MCFG_MACHINE_START(20pacgal)
	MCFG_MACHINE_RESET(20pacgal)

	MCFG_EEPROM_ADD("eeprom", _20pacgal_eeprom_intf)

	/* video hardware */
	MCFG_FRAGMENT_ADD(20pacgal_video)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("namco", NAMCO, NAMCO_AUDIO_CLOCK)
	MCFG_SOUND_CONFIG(namco_config)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definition
 *
 *************************************/


/*
     Pacman - 25th Anniversary Edition
*/

ROM_START( 25pacman ) /* Revision 2.00 */
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "pacman_25th_rev2.0.u13", 0x00000, 0x40000, CRC(99a52784) SHA1(6222c2eb686e65ba23ca376ff4392be1bc826a03) ) /* Label printed Rev 2.0, program says Rev 2.00 */

	ROM_REGION( 0x8000, "proms", 0 )	/* palette */
	ROM_LOAD( "pacman_25th.u14", 0x0000, 0x8000, CRC(c19d9ad0) SHA1(002581fbc2c32cdf7cfb0b0f64061591a462ec14) ) /* Same as the MS. Pacman / Galaga graphics rom */
ROM_END

/*
     Ms. Pac-Man/Galaga - 20th Anniversary Class of 1981 Reunion
*/

ROM_START( 20pacgal ) /* Version 1.08 */
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "ms_pac-galaga_v1.08.u13", 0x00000, 0x40000, CRC(2ea16809) SHA1(27f041bdbb590917e9dcb70c21aa6b6d6c9f04fb) ) /* Also found labeled as "V1.08 HO" */

	ROM_REGION( 0x8000, "proms", 0 )	/* palette */
	ROM_LOAD( "ms_pac-galaga.u14", 0x0000, 0x8000, CRC(c19d9ad0) SHA1(002581fbc2c32cdf7cfb0b0f64061591a462ec14) )
ROM_END

ROM_START( 20pacgalr4 ) /* Version 1.04 */
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "ms_pac-galaga_v1.04.u13", 0x00000, 0x40000, CRC(6c474d2d) SHA1(5a150fc9d2ed0e908385b9f9d532aa33cf80dba4) )

	ROM_REGION( 0x8000, "proms", 0 )	/* palette */
	ROM_LOAD( "ms_pac-galaga.u14", 0x0000, 0x8000, CRC(c19d9ad0) SHA1(002581fbc2c32cdf7cfb0b0f64061591a462ec14) )
ROM_END

ROM_START( 20pacgalr3 ) /* Version 1.03 */
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "ms_pac-galaga_v1.03.u13", 0x00000, 0x40000, CRC(e13dce63) SHA1(c8943f082883c423210fc3c97323222afb00f0a2) )

	ROM_REGION( 0x8000, "proms", 0 )	/* palette */
	ROM_LOAD( "ms_pac-galaga.u14", 0x0000, 0x8000, CRC(c19d9ad0) SHA1(002581fbc2c32cdf7cfb0b0f64061591a462ec14) )
ROM_END

ROM_START( 20pacgalr2 ) /* Version 1.02 */
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "ms_pac-galaga_v1.02.u13", 0x00000, 0x40000, CRC(b939f805) SHA1(5fe9470601156dfc2d339c94fd8f0aa4db197760) )

	ROM_REGION( 0x8000, "proms", 0 )	/* palette */
	ROM_LOAD( "ms_pac-galaga.u14", 0x0000, 0x8000, CRC(c19d9ad0) SHA1(002581fbc2c32cdf7cfb0b0f64061591a462ec14) )
ROM_END

ROM_START( 20pacgalr1 ) /* Version 1.01 */
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "ms_pac-galaga_v1.01.u13", 0x00000, 0x40000, CRC(77159582) SHA1(c05e005a941cbdc806dcd76b315069362c792a72) )

	ROM_REGION( 0x8000, "proms", 0 )	/* palette */
	ROM_LOAD( "ms_pac-galaga.u14", 0x0000, 0x8000, CRC(c19d9ad0) SHA1(002581fbc2c32cdf7cfb0b0f64061591a462ec14) )
ROM_END

ROM_START( 20pacgalr0 ) /* Version 1.00 */
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "ms_pac-galaga_v1.0.u13", 0x00000, 0x40000, CRC(3c92a269) SHA1(a616d912393f4e49b95231d72eec48567f46fc00) ) /* Label printed V1.0, program says v1.00 */

	ROM_REGION( 0x8000, "proms", 0 )	/* palette */
	ROM_LOAD( "ms_pac-galaga.u14", 0x0000, 0x8000, CRC(c19d9ad0) SHA1(002581fbc2c32cdf7cfb0b0f64061591a462ec14) )
ROM_END


static DRIVER_INIT(20pacgal)
{
	_20pacgal_state *state = machine.driver_data<_20pacgal_state>();
	state->m_sprite_pal_base = 0x00<<2;
}

static DRIVER_INIT(25pacman)

{
	_20pacgal_state *state = machine.driver_data<_20pacgal_state>();
	state->m_sprite_pal_base = 0x20<<2;
}


/*************************************
 *
 *  Game driver
 *
 *************************************/

GAME( 2005, 25pacman,          0, 20pacgal, 25pacman, 25pacman, ROT90, "Namco", "Pac-Man - 25th Anniversary Edition (Rev 2.00)", GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE)

GAME( 2000, 20pacgal,          0, 20pacgal, 20pacgal, 20pacgal, ROT90, "Namco / Cosmodog", "Ms. Pac-Man/Galaga - 20th Anniversary Class of 1981 Reunion (V1.08)", GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE)
GAME( 2000, 20pacgalr4, 20pacgal, 20pacgal, 20pacgal, 20pacgal, ROT90, "Namco / Cosmodog", "Ms. Pac-Man/Galaga - 20th Anniversary Class of 1981 Reunion (V1.04)", GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE)
GAME( 2000, 20pacgalr3, 20pacgal, 20pacgal, 20pacgal, 20pacgal, ROT90, "Namco / Cosmodog", "Ms. Pac-Man/Galaga - 20th Anniversary Class of 1981 Reunion (V1.03)", GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE)
GAME( 2000, 20pacgalr2, 20pacgal, 20pacgal, 20pacgal, 20pacgal, ROT90, "Namco / Cosmodog", "Ms. Pac-Man/Galaga - 20th Anniversary Class of 1981 Reunion (V1.02)", GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE)
GAME( 2000, 20pacgalr1, 20pacgal, 20pacgal, 20pacgal, 20pacgal, ROT90, "Namco / Cosmodog", "Ms. Pac-Man/Galaga - 20th Anniversary Class of 1981 Reunion (V1.01)", GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE)
GAME( 2000, 20pacgalr0, 20pacgal, 20pacgal, 20pacgal, 20pacgal, ROT90, "Namco / Cosmodog", "Ms. Pac-Man/Galaga - 20th Anniversary Class of 1981 Reunion (V1.00)", GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE)
