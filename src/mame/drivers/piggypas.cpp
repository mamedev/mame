// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

Piggy Pass

hw platform unknown
game details unknown

*/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/ticket.h"
#include "sound/okim6295.h"
#include "video/hd44780.h"

#include "piggypas.lh"

class piggypas_state : public driver_device
{
public:
	piggypas_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ticket(*this, "ticket")
	{ }

	virtual void machine_start() override;
	virtual void machine_reset() override;
	DECLARE_WRITE8_MEMBER(ctrl_w);
	DECLARE_WRITE8_MEMBER(mcs51_tx_callback);
	DECLARE_INPUT_CHANGED_MEMBER(ball_sensor);
	DECLARE_CUSTOM_INPUT_MEMBER(ticket_r);

	required_device<mcs51_cpu_device> m_maincpu;
	required_device<ticket_dispenser_device> m_ticket;
	UINT8   m_ctrl;
	UINT8   m_digit_idx;
};



WRITE8_MEMBER(piggypas_state::ctrl_w)
{
	if ((m_ctrl ^ data) & m_ctrl & 0x04)
		m_digit_idx = 0;

	m_ticket->write(space, 0, (data & 0x40) ? 0x80 : 0);

	m_ctrl = data;
}

WRITE8_MEMBER(piggypas_state::mcs51_tx_callback)
{
	output_set_digit_value(m_digit_idx++, BITSWAP8(data,7,6,4,3,2,1,0,5) & 0x7f);
}

static ADDRESS_MAP_START( piggypas_map, AS_PROGRAM, 8, piggypas_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( piggypas_io, AS_IO, 8, piggypas_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM

	AM_RANGE(0x1000, 0x1000) AM_DEVREADWRITE("oki", okim6295_device, read, write)

	AM_RANGE(0x0800, 0x0800) AM_READ_PORT("IN1")
	AM_RANGE(0x0801, 0x0801) AM_WRITE(ctrl_w)
	AM_RANGE(0x0802, 0x0802) AM_READ_PORT("IN0")

	AM_RANGE(0x1800, 0x1801) AM_DEVWRITE("hd44780", hd44780_device, write)
	AM_RANGE(0x1802, 0x1803) AM_DEVREAD("hd44780", hd44780_device, read)

	AM_RANGE(MCS51_PORT_P3, MCS51_PORT_P3) AM_READ_PORT("IN2")
ADDRESS_MAP_END


INPUT_CHANGED_MEMBER(piggypas_state::ball_sensor)
{
	m_maincpu->set_input_line(1, newval ? CLEAR_LINE : ASSERT_LINE);
}

CUSTOM_INPUT_MEMBER(piggypas_state::ticket_r)
{
	return m_ticket->line_r();
}

static INPUT_PORTS_START( piggypas )
	PORT_START("IN0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_COIN3)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_START1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_COIN4)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_SPECIAL)  PORT_CUSTOM_MEMBER(DEVICE_SELF, piggypas_state, ticket_r, NULL)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_COIN2)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("Gate sensor")   PORT_CODE(KEYCODE_G)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_COIN1)

	PORT_START("IN1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_NAME("Decrease")   PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_NAME("Exit")       PORT_CODE(KEYCODE_E)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_NAME("Last")       PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_NAME("Run")        PORT_CODE(KEYCODE_R)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_NAME("Increase")   PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_NAME("Enter")      PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_NAME("Next")       PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_SERVICE)  PORT_NAME("Program")

	PORT_START("IN2")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_CHANGED_MEMBER(DEVICE_SELF, piggypas_state, ball_sensor, 0) // ball sensor
INPUT_PORTS_END



void piggypas_state::machine_start()
{
	m_maincpu->i8051_set_serial_tx_callback(WRITE8_DELEGATE(piggypas_state, mcs51_tx_callback));
}

void piggypas_state::machine_reset()
{
	m_digit_idx = 0;
}

static HD44780_PIXEL_UPDATE(piggypas_pixel_update)
{
	if (pos < 8)
		bitmap.pix16(y, (line * 8 + pos) * 6 + x) = state;
}

static MACHINE_CONFIG_START( piggypas, piggypas_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8031, 8000000) // unknown variant
	MCFG_CPU_PROGRAM_MAP(piggypas_map)
	MCFG_CPU_IO_MAP(piggypas_io)
//  MCFG_CPU_VBLANK_INT_DRIVER("screen", piggypas_state,  irq0_line_hold)

	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_UPDATE_DEVICE("hd44780", hd44780_device, screen_update)
	MCFG_SCREEN_SIZE(16*6, 8)
	MCFG_SCREEN_VISIBLE_AREA(0, 16*6-1, 0, 8-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 2)
	MCFG_DEFAULT_LAYOUT(layout_piggypas)

	MCFG_HD44780_ADD("hd44780")
	MCFG_HD44780_LCD_SIZE(1, 16)
	MCFG_HD44780_PIXEL_UPDATE_CB(piggypas_pixel_update)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", 1000000, OKIM6295_PIN7_HIGH) // not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_TICKET_DISPENSER_ADD("ticket", attotime::from_msec(100), TICKET_MOTOR_ACTIVE_HIGH, TICKET_STATUS_ACTIVE_HIGH)
MACHINE_CONFIG_END



ROM_START( piggypas )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pigypass.u6", 0x00000, 0x10000, CRC(c8dc4e26) SHA1(f9643945f84fe2679742922abf5a92b77bf59e4c) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "pigypass.u14", 0x00000, 0x40000, CRC(855504c1) SHA1(dfe91943057fa66798c8395348cf703cb11468d2) )
ROM_END


ROM_START( hoopshot )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hoopshot.u6", 0x00000, 0x08000, CRC(fa8ae8aa) SHA1(2266a775fba7c8f8e3e24441aca6c4b89a6d1ec7) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "hoopshot.u14", 0x00000, 0x40000, CRC(748462b5) SHA1(ccb8f1dbb6471b134c1e97699383c3ef139c42c3) )
ROM_END



ROM_START( rndrndqs )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "round.u6", 0x00000, 0x08000, CRC(3eb64b10) SHA1(66051cdd6be33f4f7249be1c8d56e5e43c838163) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "round.u14", 0x00000, 0x40000, CRC(36d1c07a) SHA1(3c978d4d03d8dbf79e1afe7dc46209d9ac4d3cc3) )
ROM_END

ROM_START( fidlstix )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fiddle.u6", 0x00000, 0x08000, CRC(48125bf1) SHA1(5772fe3c0987fc6b2508da5efe3c4c3c179b76a1) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "fiddle.u14", 0x00000, 0x40000, CRC(baf4e1cd) SHA1(ae153f832cbd188e9f3f357a1a1f68cc8264d346) )
ROM_END

// bad dump of program rom
ROM_START( jackbean )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "beanstlk.u6", 0x00000, 0x10000, BAD_DUMP CRC(127c4d6c) SHA1(c864293f42e81a1b8e5dcb12abc1c0019853792e) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "beanstlk.u14", 0x00000, 0x40000, CRC(e33ef0a3) SHA1(337ce3d0c901b0b3241d76601eaad6e3e2724e1a) )
ROM_END

// bad dump of program rom
ROM_START( dumpump )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dump-ump.u6", 0x00000, 0x08000, BAD_DUMP CRC(410fc27e) SHA1(d9505c11f4844b9b58c12b3ff6b860357a4be75e))

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "dump-ump.u14", 0x00000, 0x20000, CRC(08bc7bb5) SHA1(2355783ec614d8f4e1dca3cb415a97a28411157b))
ROM_END

// bad dump of program rom
ROM_START( 3lilpigs )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "3-pigs.u6", 0x00000, 0x10000, BAD_DUMP CRC(1db9d754) SHA1(9b1db9c9bb155ebb5509970476b20b9dda6d3021) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "3-pigs.u14", 0x00000, 0x40000, CRC(62eb76e2) SHA1(c4cad241dedf2c290f9bf80038415fe39b3ce17d) )
ROM_END





// COPYRIGHT (c) 1990, 1991, 1992, DOYLE & ASSOC., INC.   VERSION 04.40
GAME( 1992, piggypas,  0,    piggypas, piggypas, driver_device,  0, ROT0, "Doyle & Assoc.", "Piggy Pass (version 04.40)", MACHINE_IS_SKELETON_MECHANICAL )
// COPYRIGHT (c) 1990, 1991, 1992, DOYLE & ASSOC., INC.   VERSION 05.22
GAME( 1992, hoopshot,  0,    piggypas, piggypas, driver_device,  0, ROT0, "Doyle & Assoc.", "Hoop Shot (version 05.22)", MACHINE_IS_SKELETON_MECHANICAL )
// Quick $ilver   Development Co.    10/08/96      ROUND  REV 6
GAME( 1996, rndrndqs,  0,    piggypas, piggypas, driver_device,  0, ROT0, "Quick $ilver Development Co.", "Round and Round (Rev 6) (Quick $ilver)", MACHINE_IS_SKELETON_MECHANICAL )
//  Quick$ilver   Development Co.    10/02/95      -FIDDLESTIX-       REV 15T
GAME( 1995, fidlstix,  0,    piggypas, piggypas, driver_device,  0, ROT0, "Quick $ilver Development Co.", "Fiddle Stix (1st Rev)", MACHINE_IS_SKELETON_MECHANICAL )
// bad dump, so version unknown
GAME( 199?, jackbean,  0,    piggypas, piggypas, driver_device,  0, ROT0, "Doyle & Assoc.", "Jack & The Beanstalk (Doyle & Assoc.?)", MACHINE_IS_SKELETON_MECHANICAL )
// bad dump, so version unknown
GAME( 199?, dumpump,   0,    piggypas, piggypas, driver_device,  0, ROT0, "Doyle & Assoc.", "Dump The Ump", MACHINE_IS_SKELETON_MECHANICAL )
// bad dump, so version unknown
GAME( 199?, 3lilpigs,  0,    piggypas, piggypas, driver_device,  0, ROT0, "Doyle & Assoc.", "3 Lil' Pigs", MACHINE_IS_SKELETON_MECHANICAL )
