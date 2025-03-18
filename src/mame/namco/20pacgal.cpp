// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
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

    Note: The "correct" size of the roms are 27C020 for the program rom and 27C256 for the
          graphics rom.  However genuine boards have been found with larger roms containing
          the same data with the extra rom space blanked out.

    Known issues/to-do's:
        * Starfield is not 100% accurate
        * Check the ASCI interface, there probably is fully working debug code.
        * The timed interrupt is a kludge; it is supposed to be generated internally by
          the Z180, but the cpu core doesn't support that yet.
        * Galaga attract mode isn't correct; reference : https://youtu.be/OQyWaN9fTgw?t=2m33s

+-------------------------------------------------------+
|                        +-------------+                |
|                        |     U13     |                |
|                        |ms pac/galaga|                |
|                        +-------------+                |
|                         +-----------+                 |
|        +---+            |           |                 |
|        |VOL|            |   ZiLOG   |                 |
|        +---+            |Z8S18020VSC|                 |
|                         | Z180 MPU  |                 |
|J                        |           |                 |
|A                        +-----------+                 |
|M              +-----------+      +-----------+        |
|M              |           |  OSC |           |        |
|A              |CY37256P160|      |CY37256P160|        |
|               |-83AC      |      |-83AC      |        |
|               |           |   :: |           |   +---+|
|               |           |   :: |           |   | C ||
|               +-----------+   J1 +-----------+   | 1 ||
|                 93LC46A                          | 9 ||
|         +-------------+              +-------+   | 9 ||
|         |     U14     |   +-------+  |CY7C199|   +---+|
|         |ms pac/galaga|   |CY7C199|  +-------+        |
|  D4     +-------------+   +-------+                   |
+-------------------------------------------------------+

     CPU: Z8S18020VSC ZiLOG Z180 (20MHz part) at 18.432MHz
Graphics: CY37256P160-83AC x 2 (Ultra37000 CPLD family - 160 pin TQFP, 256 Macrocells, 83MHz speed)
  MEMORY: CY7C199-15VC 32K x 8 Static RAM x 3 (or equivalent ISSI IS61C256AH-15J)
     OSC: 73.728MHz
  EEPROM: 93LC46A 128 x 8-bit 1K microwire compatible Serial EEPROM
     VOL: Volume adjust
      D4: Diode - Status light
      J1: 10 pin JTAG interface for programming the CY37256P160 CPLDs

***************************************************************************/

#include "emu.h"
#include "20pacgal.h"

#include "machine/eepromser.h"
#include "machine/watchdog.h"
#include "speaker.h"


/*************************************
 *
 *  Clocks
 *
 *************************************/

#define MASTER_CLOCK        (XTAL(73'728'000))
#define MAIN_CPU_CLOCK      (MASTER_CLOCK / 4)
#define NAMCO_AUDIO_CLOCK   (MASTER_CLOCK / 4 /  6 / 32)



/*************************************
 *
 *  Interrupt system
 *
 *************************************/

void _20pacgal_state::irqack_w(uint8_t data)
{
	m_irq_mask = data & 1;

	if (!m_irq_mask)
		m_maincpu->set_input_line(0, CLEAR_LINE);
}

void _20pacgal_state::timer_pulse_w(uint8_t data)
{
	//printf("timer pulse %02x\n", data);
}

/*************************************
 *
 *  Coin counter
 *
 *************************************/

void _20pacgal_state::_20pacgal_coin_counter_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0, data & 1);
}



/*************************************
 *
 *  ROM banking
 *
 *************************************/

void _20pacgal_state::ram_bank_select_w(uint8_t data)
{
	if (m_game_selected != (data & 1))
	{
		m_game_selected = data & 1;
		m_mainbank->set_entry(m_game_selected);
		get_pens();
	}
}

void _20pacgal_state::ram_48000_w(offs_t offset, uint8_t data)
{
	if (m_game_selected)
	{
		if (offset < 0x0800)
			m_video_ram[offset & 0x07ff] = data;

		m_ram_48000[offset] = data;
	}
}

/*************************************
 *
 *  Memory handlers
 *
 *************************************/

void _20pacgal_state::sprite_gfx_w(offs_t offset, uint8_t data)
{
	m_sprite_gfx_ram[offset] = data;
}

void _20pacgal_state::sprite_ram_w(offs_t offset, uint8_t data)
{
	m_sprite_ram[offset] = data;
}

void _20pacgal_state::sprite_lookup_w(offs_t offset, uint8_t data)
{
	m_sprite_color_lookup[offset] = data;
}

// this is wrong
// where does the clut (sprite_lookup_w) get uploaded? even if I set a WP on that data in ROM it isn't hit?
// likewise the sound table.. is it being uploaded in a different format at 0x0c000?
// we also need the palette data because there is only a single rom on this pcb?
void _25pacman_state::_25pacman_map(address_map &map)
{
	map(0x00000, 0x3ffff).rw("flash", FUNC(amd_29lv200t_device::read), FUNC(amd_29lv200t_device::write));  // (always fall through if nothing else is mapped?)

	map(0x04000, 0x047ff).ram().share("video_ram");
	map(0x04800, 0x05fff).ram();
	map(0x06000, 0x06fff).writeonly().share("char_gfx_ram");
	map(0x07000, 0x0717f).w(FUNC(_25pacman_state::sprite_ram_w));
//  map(0x08000, 0x09fff).bankr("mainbank").w(FUNC(_20pacgal_state::ram_48000_w));
	map(0x08000, 0x09fff).nopw();
	map(0x0a000, 0x0bfff).w(FUNC(_25pacman_state::sprite_gfx_w));
	map(0x0c000, 0x0dfff).nopw(); // is this the sound waveforms in a different format?
	map(0x0e000, 0x0ffff).nopw();
	map(0x1c000, 0x1ffff).nopw();
}

void _20pacgal_state::_20pacgal_map(address_map &map)
{
	map(0x00000, 0x03fff).rom();
	map(0x04000, 0x07fff).rom();
	map(0x08000, 0x09fff).rom();
	map(0x0a000, 0x0ffff).mirror(0x40000).rom();
	map(0x10000, 0x3ffff).rom();
	map(0x44000, 0x447ff).ram().share("video_ram");
	map(0x44800, 0x45eff).ram();
	map(0x45040, 0x4505f).w("namco", FUNC(namco_cus30_device::pacman_sound_w));
	map(0x45f00, 0x45fff).w("namco", FUNC(namco_cus30_device::namcos1_cus30_w));
	map(0x46000, 0x46fff).writeonly().share("char_gfx_ram");
	map(0x47100, 0x47100).ram();   /* leftover from original Galaga code */
	map(0x48000, 0x49fff).bankr("mainbank").w(FUNC(_20pacgal_state::ram_48000_w));  /* this should be a mirror of 08000-09fff */
	map(0x4c000, 0x4dfff).w(FUNC(_20pacgal_state::sprite_gfx_w));
	map(0x4e000, 0x4e17f).w(FUNC(_20pacgal_state::sprite_ram_w));
	map(0x4e180, 0x4feff).nopw();
	map(0x4ff00, 0x4ffff).w(FUNC(_20pacgal_state::sprite_lookup_w));
}


/*************************************
 *
 *  I/O port handlers
 *
 *************************************/

uint8_t _25pacman_state::_25pacman_io_87_r()
{
	return 0xff;
}

void _25pacman_state::_25pacman_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x3f).noprw(); /* Z180 internal registers */
	map(0x40, 0x7f).noprw(); /* Z180 internal registers */
	map(0x80, 0x80).portr("P1");
	map(0x81, 0x81).portr("P2");
	map(0x82, 0x82).portr("SERVICE");
	map(0x80, 0x80).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x81, 0x81).w(FUNC(_25pacman_state::timer_pulse_w));        /* ??? pulsed by the timer irq */
	map(0x82, 0x82).w(FUNC(_25pacman_state::irqack_w));
//  map(0x84, 0x84).noprw(); /* ?? */
	map(0x85, 0x86).writeonly().share("stars_seed");    /* stars: rng seed (lo/hi) */
	map(0x87, 0x87).r(FUNC(_25pacman_state::_25pacman_io_87_r)); // not eeprom on this
	map(0x87, 0x87).nopw();
//  map(0x88, 0x88).w(FUNC(_25pacman_state::ram_bank_select_w));
	map(0x89, 0x89).w("dac", FUNC(dac_byte_interface::data_w));
	map(0x8a, 0x8a).writeonly().share("stars_ctrl");    /* stars: bits 3-4 = active set; bit 5 = enable */
	map(0x8b, 0x8b).writeonly().share("flip");
	map(0x8c, 0x8c).nopw();
	map(0x8f, 0x8f).w(FUNC(_25pacman_state::_20pacgal_coin_counter_w));
}

void _20pacgal_state::_20pacgal_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x3f).noprw(); /* Z180 internal registers */
	map(0x40, 0x7f).noprw(); /* Z180 internal registers */
	map(0x80, 0x80).portr("P1");
	map(0x81, 0x81).portr("P2");
	map(0x82, 0x82).portr("SERVICE");
	map(0x80, 0x80).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x81, 0x81).w(FUNC(_20pacgal_state::timer_pulse_w));        /* ??? pulsed by the timer irq */
	map(0x82, 0x82).w(FUNC(_20pacgal_state::irqack_w));
	map(0x84, 0x84).noprw(); /* ?? */
	map(0x85, 0x86).writeonly().share("stars_seed");    /* stars: rng seed (lo/hi) */
	map(0x87, 0x87).portr("EEPROMIN").portw("EEPROMOUT");
	map(0x88, 0x88).w(FUNC(_20pacgal_state::ram_bank_select_w));
	map(0x89, 0x89).w("dac", FUNC(dac_byte_interface::data_w));
	map(0x8a, 0x8a).writeonly().share("stars_ctrl");    /* stars: bits 3-4 = active set; bit 5 = enable */
	map(0x8b, 0x8b).writeonly().share("flip");
	map(0x8f, 0x8f).w(FUNC(_20pacgal_state::_20pacgal_coin_counter_w));
}



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
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x80, IP_ACTIVE_LOW )

	PORT_START( "EEPROMIN" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))    // bit 7 is EEPROM data

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::cs_write))  // bit 5 is cs (active high)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::clk_write)) // bit 6 is clock (active high)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::di_write))  // bit 7 is data
INPUT_PORTS_END


/* Note: 25pacman Control Panel functions the same as 20pacgal, even if Right P1/P2 start are not shown during Switch Test */
static INPUT_PORTS_START( 25pacman )
	PORT_INCLUDE(20pacgal)

	PORT_MODIFY("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME ( "Service Volume Up" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME ( "Service Volume Down" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( 25pacmano )
	PORT_INCLUDE(20pacgal)

	PORT_MODIFY("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("EEPROMIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

/*************************************
 *
 *  Machine driver
 *
 *************************************/

void _20pacgal_state::common_save_state()
{
	m_ram_48000 = make_unique_clear<uint8_t[]>(0x2000);

	save_item(NAME(m_game_selected));
	save_pointer(NAME(m_ram_48000), 0x2000);
	save_item(NAME(m_irq_mask));
}

void _20pacgal_state::machine_start()
{
	common_save_state();

	m_game_selected = 0;

	// center the DAC; not all revisions do this
	m_dac->data_w(0x80);

	// membank currently used only by 20pacgal
	m_mainbank->configure_entry(0, memregion("maincpu")->base() + 0x08000);
	m_mainbank->configure_entry(1, m_ram_48000.get());
}

void _25pacman_state::machine_start()
{
	common_save_state();

	m_game_selected = 0;
}

void _20pacgal_state::vblank_irq(int state)
{
	if (state && m_irq_mask)
		m_maincpu->set_input_line(0, HOLD_LINE); // TODO: assert breaks the inputs in 25pacman test mode
}

static DEVICE_INPUT_DEFAULTS_START( null_modem )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_115200 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_115200 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

void _20pacgal_state::_20pacgal(machine_config &config)
{
	/* basic machine hardware */
	Z8S180(config, m_maincpu, MAIN_CPU_CLOCK); // 18.432MHz verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &_20pacgal_state::_20pacgal_map);
	m_maincpu->set_addrmap(AS_IO, &_20pacgal_state::_20pacgal_io_map);
	m_maincpu->txa0_wr_callback().set("rs232", FUNC(rs232_port_device::write_txd));

	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->set_option_device_input_defaults("null_modem", DEVICE_INPUT_DEFAULTS_NAME(null_modem));
	m_rs232->set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(null_modem));
	m_rs232->rxd_handler().set(m_maincpu, FUNC(z180_device::rxa0_w));
	m_rs232->cts_handler().set(m_maincpu, FUNC(z180_device::cts0_w));

	EEPROM_93C46_8BIT(config, m_eeprom);

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	_20pacgal_video(config);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();

	namco_cus30_device &namco(NAMCO_CUS30(config, "namco", NAMCO_AUDIO_CLOCK));
	namco.set_voices(3);
	namco.add_route(ALL_OUTPUTS, "speaker", 1.0);

	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 1.0); // unknown DAC
}

static DEVICE_INPUT_DEFAULTS_START( null_modem_57600 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_57600 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_57600 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

void _25pacman_state::_25pacman(machine_config &config)
{
	_20pacgal(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &_25pacman_state::_25pacman_map);
	m_maincpu->set_addrmap(AS_IO, &_25pacman_state::_25pacman_io_map);

	m_rs232->set_option_device_input_defaults("null_modem", DEVICE_INPUT_DEFAULTS_NAME(null_modem_57600));
	m_rs232->set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(null_modem_57600));

	AMD_29LV200T(config, "flash");
}



/*************************************
 *
 *  ROM definition
 *
 *************************************/


/*
     Pacman - 25th Anniversary Edition
*/

ROM_START( 25pacmano ) /* Revision 2.00 */
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "pacman_25th_rev2.0.u13", 0x00000, 0x40000, CRC(99a52784) SHA1(6222c2eb686e65ba23ca376ff4392be1bc826a03) ) /* Label printed Rev 2.0, program says Rev 2.00 */

	ROM_REGION( 0x8000, "proms", 0 )    /* palette */
	ROM_LOAD( "pacman_25th.u14", 0x0000, 0x8000, CRC(c19d9ad0) SHA1(002581fbc2c32cdf7cfb0b0f64061591a462ec14) ) /* Same as the MS. Pacman / Galaga graphics rom */
ROM_END

// guzuta v2.2 PCB
// this uses the main FLASH rom to save things instead of eeprom (type is AM29LV200)
// different memory map.. no palette rom
ROM_START( 25pacman ) /* Revision 3.00 */
	ROM_REGION( 0x40000, "flash", 0 )
	ROM_LOAD( "pacman25ver3.u1", 0x00000, 0x40000, CRC(55b0076e) SHA1(4544cc193bdd22bfc88d096083ccc4069cac4607) ) /* program says Rev 3.00 */
	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x8000, "proms", 0 )    /* palette */
	// shouldn't be loading this! must be uploaded somewhere
	ROM_LOAD( "pacman_25th.u14", 0x0000, 0x8000, CRC(c19d9ad0) SHA1(002581fbc2c32cdf7cfb0b0f64061591a462ec14) )
ROM_END

/*
     Ms. Pac-Man/Galaga - 20th Anniversary Class of 1981 Reunion
*/

ROM_START( 20pacgal ) /* Version 1.08 */
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "ms_pac-galaga_v1.08.u13", 0x00000, 0x40000, CRC(2ea16809) SHA1(27f041bdbb590917e9dcb70c21aa6b6d6c9f04fb) ) /* Also found labeled as "V1.08 HO" */

	ROM_REGION( 0x8000, "proms", 0 )    /* palette */
	ROM_LOAD( "ms_pac-galaga.u14", 0x0000, 0x8000, CRC(c19d9ad0) SHA1(002581fbc2c32cdf7cfb0b0f64061591a462ec14) )
ROM_END

ROM_START( 20pacgalr4 ) /* Version 1.04 */
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "ms_pac-galaga_v1.04.u13", 0x00000, 0x40000, CRC(6c474d2d) SHA1(5a150fc9d2ed0e908385b9f9d532aa33cf80dba4) )

	ROM_REGION( 0x8000, "proms", 0 )    /* palette */
	ROM_LOAD( "ms_pac-galaga.u14", 0x0000, 0x8000, CRC(c19d9ad0) SHA1(002581fbc2c32cdf7cfb0b0f64061591a462ec14) )
ROM_END

ROM_START( 20pacgalr3 ) /* Version 1.03 */
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "ms_pac-galaga_v1.03.u13", 0x00000, 0x40000, CRC(e13dce63) SHA1(c8943f082883c423210fc3c97323222afb00f0a2) )

	ROM_REGION( 0x8000, "proms", 0 )    /* palette */
	ROM_LOAD( "ms_pac-galaga.u14", 0x0000, 0x8000, CRC(c19d9ad0) SHA1(002581fbc2c32cdf7cfb0b0f64061591a462ec14) )
ROM_END

ROM_START( 20pacgalr2 ) /* Version 1.02 */
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "ms_pac-galaga_v1.02.u13", 0x00000, 0x40000, CRC(b939f805) SHA1(5fe9470601156dfc2d339c94fd8f0aa4db197760) )

	ROM_REGION( 0x8000, "proms", 0 )    /* palette */
	ROM_LOAD( "ms_pac-galaga.u14", 0x0000, 0x8000, CRC(c19d9ad0) SHA1(002581fbc2c32cdf7cfb0b0f64061591a462ec14) )
ROM_END

ROM_START( 20pacgalr1 ) /* Version 1.01 */
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "ms_pac-galaga_v1.01.u13", 0x00000, 0x40000, CRC(77159582) SHA1(c05e005a941cbdc806dcd76b315069362c792a72) )

	ROM_REGION( 0x8000, "proms", 0 )    /* palette */
	ROM_LOAD( "ms_pac-galaga.u14", 0x0000, 0x8000, CRC(c19d9ad0) SHA1(002581fbc2c32cdf7cfb0b0f64061591a462ec14) )
ROM_END

ROM_START( 20pacgalr0 ) /* Version 1.00 */
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "ms_pac-galaga_v1.0.u13", 0x00000, 0x40000, CRC(3c92a269) SHA1(a616d912393f4e49b95231d72eec48567f46fc00) ) /* Label printed V1.0, program says v1.00 */

	ROM_REGION( 0x8000, "proms", 0 )    /* palette */
	ROM_LOAD( "ms_pac-galaga.u14", 0x0000, 0x8000, CRC(c19d9ad0) SHA1(002581fbc2c32cdf7cfb0b0f64061591a462ec14) )
ROM_END




void _20pacgal_state::init_20pacgal()
{
	m_sprite_pal_base = 0x00<<2;
}

void _20pacgal_state::init_25pacman()
{
	m_sprite_pal_base = 0x20<<2;
}


/*************************************
 *
 *  Game driver
 *
 *************************************/

GAME( 2006, 25pacman,   0,        _25pacman, 25pacman,  _25pacman_state, init_25pacman, ROT90, "Namco / Cosmodog", "Pac-Man - 25th Anniversary Edition (Rev 3.00)",                       MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 2005, 25pacmano,  25pacman, _20pacgal, 25pacmano, _20pacgal_state, init_25pacman, ROT90, "Namco / Cosmodog", "Pac-Man - 25th Anniversary Edition (Rev 2.00)",                       MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE)

GAME( 2000, 20pacgal,   0,        _20pacgal, 20pacgal,  _20pacgal_state, init_20pacgal, ROT90, "Namco / Cosmodog", "Ms. Pac-Man/Galaga - 20th Anniversary Class of 1981 Reunion (V1.08)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE)
GAME( 2000, 20pacgalr4, 20pacgal, _20pacgal, 20pacgal,  _20pacgal_state, init_20pacgal, ROT90, "Namco / Cosmodog", "Ms. Pac-Man/Galaga - 20th Anniversary Class of 1981 Reunion (V1.04)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE)
GAME( 2000, 20pacgalr3, 20pacgal, _20pacgal, 20pacgal,  _20pacgal_state, init_20pacgal, ROT90, "Namco / Cosmodog", "Ms. Pac-Man/Galaga - 20th Anniversary Class of 1981 Reunion (V1.03)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE)
GAME( 2000, 20pacgalr2, 20pacgal, _20pacgal, 20pacgal,  _20pacgal_state, init_20pacgal, ROT90, "Namco / Cosmodog", "Ms. Pac-Man/Galaga - 20th Anniversary Class of 1981 Reunion (V1.02)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE)
GAME( 2000, 20pacgalr1, 20pacgal, _20pacgal, 20pacgal,  _20pacgal_state, init_20pacgal, ROT90, "Namco / Cosmodog", "Ms. Pac-Man/Galaga - 20th Anniversary Class of 1981 Reunion (V1.01)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE)
GAME( 2000, 20pacgalr0, 20pacgal, _20pacgal, 20pacgal,  _20pacgal_state, init_20pacgal, ROT90, "Namco / Cosmodog", "Ms. Pac-Man/Galaga - 20th Anniversary Class of 1981 Reunion (V1.00)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE)
