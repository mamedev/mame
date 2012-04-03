/* Midway's 18 Wheeler hardware, game number 653

driver todo:
- discrete sound
- hook up lamps (and sensors)
- lots of external artwork

To diagnose game, turn on service mode and:
- test RAM/ROM, leds, lamps:    reset with shifter in neutral
- test sound and misc input:    turn on DSW 7 and reset with shifter in neutral
- test accelerator:             reset with shifter in 1st gear
- test steering wheel:          reset with shifter in 2nd gear

*/

#include "emu.h"
#include "cpu/z80/z80.h"

#include "18w.lh"


class mw18w_state : public driver_device
{
public:
	mw18w_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	DECLARE_WRITE8_MEMBER(mw18w_sound0_w);
	DECLARE_WRITE8_MEMBER(mw18w_sound1_w);
	DECLARE_WRITE8_MEMBER(mw18w_lamps_w);
	DECLARE_WRITE8_MEMBER(mw18w_led_display_w);
	DECLARE_WRITE8_MEMBER(mw18w_irq0_clear_w);
};


WRITE8_MEMBER(mw18w_state::mw18w_sound0_w)
{
	// sound write (airhorn, brake, crash) plus motor speed for backdrop, and coin counter
	coin_counter_w(machine(), 0, data&1);
}

WRITE8_MEMBER(mw18w_state::mw18w_sound1_w)
{
	// sound write (bell, engine) plus lamp dim control for backdrop lamp
	;
}

WRITE8_MEMBER(mw18w_state::mw18w_lamps_w)
{
	;
}

WRITE8_MEMBER(mw18w_state::mw18w_led_display_w)
{
	// d0-3: 7448 (BCD to LED segment)
	const UINT8 ls48_map[16] =
		{ 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7c,0x07,0x7f,0x67,0x58,0x4c,0x62,0x69,0x78,0x00 };

	// d4-7: 7442 (BCD to decimal) -> pick digit panel
	if ((data&0xf0)>0x90) return;
	output_set_digit_value(data>>4, ls48_map[data&0xf]);
}

WRITE8_MEMBER(mw18w_state::mw18w_irq0_clear_w)
{
	cputag_set_input_line(machine(), "maincpu", 0, CLEAR_LINE);
}

static CUSTOM_INPUT( mw18w_sensors_r )
{
	// d7: off road
	// d6: in dock area
	return 0xff;
}




static ADDRESS_MAP_START( mw18w_map, AS_PROGRAM, 8, mw18w_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x23ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( mw18w_portmap, AS_IO, 8, mw18w_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("IN0") AM_WRITE(mw18w_sound0_w)
	AM_RANGE(0x01, 0x01) AM_READ_PORT("IN1") AM_WRITE(mw18w_sound1_w)
	AM_RANGE(0x02, 0x02) AM_READ_PORT("IN2") AM_WRITE(mw18w_lamps_w)
	AM_RANGE(0x03, 0x03) AM_READ_PORT("DSW") AM_WRITE(mw18w_led_display_w)
	AM_RANGE(0x04, 0x04) AM_READ_PORT("IN4")
	AM_RANGE(0x06, 0x06) AM_WRITE_LEGACY(watchdog_reset_w)
	AM_RANGE(0x07, 0x07) AM_WRITE(mw18w_irq0_clear_w)
ADDRESS_MAP_END

static const UINT32 mw18w_controller_table[] =
{
	// same encoder as sspeedr
	0x3f, 0x3e, 0x3c, 0x3d, 0x39, 0x38, 0x3a, 0x3b,
	0x33, 0x32, 0x30, 0x31, 0x35, 0x34, 0x36, 0x37,
	0x27, 0x26, 0x24, 0x25, 0x21, 0x20, 0x22, 0x23,
	0x2b, 0x2a, 0x28, 0x29, 0x2d, 0x2c, 0x2e, 0x2f,
	0x0f, 0x0e, 0x0c, 0x0d, 0x09, 0x08, 0x0a, 0x0b,
	0x03, 0x02, 0x00, 0x01, 0x05, 0x04, 0x06, 0x07,
	0x17, 0x16, 0x14, 0x15, 0x11, 0x10, 0x12, 0x13,
	0x1b, 0x1a, 0x18, 0x19, 0x1d, 0x1c, 0x1e, 0x1f
};

static INPUT_PORTS_START( mw18w )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )	// left/right sw.
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(mw18w_sensors_r, NULL)

	PORT_START("IN1")
	PORT_BIT( 0x1f, 0x00, IPT_PEDAL ) PORT_REMAP_TABLE(mw18w_controller_table + 0x20) PORT_SENSITIVITY(100) PORT_KEYDELTA(1)	// accelerate
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Shifter 1st Gear")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Shifter 3rd Gear")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Shifter 2nd Gear")

	PORT_START("IN2")
	PORT_BIT( 0x3f, 0x1f, IPT_PADDLE ) PORT_REMAP_TABLE(mw18w_controller_table) PORT_SENSITIVITY(100) PORT_KEYDELTA(1)			// steering wheel
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )	// brake
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Shifter Reverse")

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW:1,2")
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Game_Time ) ) PORT_DIPLOCATION("SW:5,6")
	PORT_DIPSETTING(    0x00, "60 seconds" )
	PORT_DIPSETTING(    0x10, "70 seconds" )
	PORT_DIPSETTING(    0x20, "80 seconds" )
	PORT_DIPSETTING(    0x30, "90 seconds" )

	PORT_DIPNAME( 0x0c, 0x00, "Extended Time" ) PORT_DIPLOCATION("SW:3,4")
	PORT_DIPSETTING(    0x00, "20 seconds at 4000" )  PORT_CONDITION("DSW", 0x30, PORTCOND_EQUALS, 0x00) // @ 60 seconds
	PORT_DIPSETTING(    0x04, "30 seconds at 8000" )  PORT_CONDITION("DSW", 0x30, PORTCOND_EQUALS, 0x00)
	PORT_DIPSETTING(    0x08, "15 seconds at 8000" )  PORT_CONDITION("DSW", 0x30, PORTCOND_EQUALS, 0x00)
	PORT_DIPSETTING(    0x0c, "30 seconds at 10000" ) PORT_CONDITION("DSW", 0x30, PORTCOND_EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, "20 seconds at 6000" )  PORT_CONDITION("DSW", 0x30, PORTCOND_EQUALS, 0x10) // @ 70 seconds
	PORT_DIPSETTING(    0x04, "30 seconds at 9000" )  PORT_CONDITION("DSW", 0x30, PORTCOND_EQUALS, 0x10)
	PORT_DIPSETTING(    0x08, "20 seconds at 9000" )  PORT_CONDITION("DSW", 0x30, PORTCOND_EQUALS, 0x10)
	PORT_DIPSETTING(    0x0c, "30 seconds at 12000" ) PORT_CONDITION("DSW", 0x30, PORTCOND_EQUALS, 0x10)
	PORT_DIPSETTING(    0x00, "20 seconds at 8000" )  PORT_CONDITION("DSW", 0x30, PORTCOND_EQUALS, 0x20) // @ 80 seconds
	PORT_DIPSETTING(    0x04, "30 seconds at 12000" ) PORT_CONDITION("DSW", 0x30, PORTCOND_EQUALS, 0x20)
	PORT_DIPSETTING(    0x08, "20 seconds at 12000" ) PORT_CONDITION("DSW", 0x30, PORTCOND_EQUALS, 0x20)
	PORT_DIPSETTING(    0x0c, "30 seconds at 16000" ) PORT_CONDITION("DSW", 0x30, PORTCOND_EQUALS, 0x20)
	PORT_DIPSETTING(    0x00, "20 seconds at 10000" ) PORT_CONDITION("DSW", 0x30, PORTCOND_EQUALS, 0x30) // @ 90 seconds
	PORT_DIPSETTING(    0x04, "30 seconds at 15000" ) PORT_CONDITION("DSW", 0x30, PORTCOND_EQUALS, 0x30)
	PORT_DIPSETTING(    0x08, "20 seconds at 15000" ) PORT_CONDITION("DSW", 0x30, PORTCOND_EQUALS, 0x30)
	PORT_DIPSETTING(    0x0c, "30 seconds at 20000" ) PORT_CONDITION("DSW", 0x30, PORTCOND_EQUALS, 0x30)

	PORT_DIPNAME( 0x40, 0x40, "I/O Test" ) PORT_DIPLOCATION("SW:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(0x80, IP_ACTIVE_LOW, "SW:8" )

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )		// n/c
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )		// n/c
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )		// for both coin chutes
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END




static MACHINE_CONFIG_START( mw18w, mw18w_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_19_968MHz/8)
	MCFG_CPU_PERIODIC_INT(irq0_line_assert,960.516)	// 555 IC
	MCFG_CPU_PROGRAM_MAP(mw18w_map)
	MCFG_CPU_IO_MAP(mw18w_portmap)

	/* no video! */

	/* sound hardware */
	// MCFG_SPEAKER_STANDARD_MONO("mono")
MACHINE_CONFIG_END


ROM_START(18w)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "18w_b1.rom1", 0x0000, 0x0800, CRC(200c5beb) SHA1(994d67a89f18df9716c5dd4dd60f6e7eeb880f1b))
	ROM_LOAD( "18w_a2.rom2", 0x0800, 0x0800, CRC(efbadee8) SHA1(834eaf8ca50544123de7529b90b828cf46b1c001))
	ROM_LOAD( "18w_b3.rom3", 0x1000, 0x0800, CRC(214606f6) SHA1(9a9dc20259b4462661c6be410d98d2be54657a0e))
	ROM_LOAD( "18w_a4.rom4", 0x1800, 0x0800, CRC(e88ad6a9) SHA1(ac010aa7e0288197ff9342801522623b64dd2a47))
ROM_END

ROM_START(18w2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "18w_b1(__18w2).rom1",0x0000, 0x0800, CRC(cbc0fb2c) SHA1(66b14f0d76baebbd64e8ed107e536ad811d55273))
	ROM_LOAD( "18w_b2.rom2", 0x0800, 0x0800, CRC(efbadee8) SHA1(834eaf8ca50544123de7529b90b828cf46b1c001))
	ROM_LOAD( "18w_b3.rom3", 0x1000, 0x0800, CRC(214606f6) SHA1(9a9dc20259b4462661c6be410d98d2be54657a0e))
	ROM_LOAD( "18w_b4.rom4", 0x1800, 0x0800, CRC(e88ad6a9) SHA1(ac010aa7e0288197ff9342801522623b64dd2a47))
ROM_END


GAMEL( 1979, 18w,  0,   mw18w, mw18w, 0, ROT0, "Midway", "18 Wheeler (set 1)", GAME_NO_SOUND | GAME_NOT_WORKING | GAME_MECHANICAL, layout_18w )
GAMEL( 1979, 18w2, 18w, mw18w, mw18w, 0, ROT0, "Midway", "18 Wheeler (set 2)", GAME_NO_SOUND | GAME_NOT_WORKING | GAME_MECHANICAL, layout_18w )
