// license:BSD-3-Clause
// copyright-holders:Robbbert
/******************************************************************************

    PINBALL
    Midway A084-91313-G627
           A080-91313-G627
           A082-91320-C000

    Only one of its kind

    This is a cocktail pinball game, for up to 4 players. The board is round.
    When it is another player's turn, the playboard will turn around to face
    him. And so, the system has a motor and an infrared shaft-locating system.
    If this system does not return the expected data, the machine will refuse
    to start.

    Schematic and PinMAME used as references
    System made working in Sept 2012 [Robbbert]

Machine Operation:
1. Press .(period key) (this sets up nvram and the displays will flash 400000)
2. Insert a coin, credit will be registered
3. Hold X and start game. When 00 is flashing, release X
4. Press any of QWERYUIOASDFGHJKLZ-='; to simulate scoring shots (T will tilt)
5. Press and hold X to simulate losing the ball
6. When score starts flashing, release X and go to step 4 to play next ball

Note: You can start play at any of the 4 stations, and the table will rotate
      as needed. Multiplayer works as well.

The manual explains the tests available, and also how to set number of balls,
high score, etc., with the diagnostic keyboard.

ToDo:
- Lamp outputs
- Possibility of a rom missing (most likely it is optional)

*******************************************************************************/


#include "machine/genpin.h"
#include "cpu/z80/z80.h"
#include "machine/i8155.h"
#include "sound/astrocde.h"
#include "g627.lh"


class g627_state : public genpin_class
{
public:
	g627_state(const machine_config &mconfig, device_type type, std::string tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_switch(*this, "SWITCH")
		, m_testipt(*this, "TEST")
	{ }

	DECLARE_DRIVER_INIT(v115);
	DECLARE_DRIVER_INIT(v117);
	DECLARE_READ8_MEMBER(porta_r);
	DECLARE_READ8_MEMBER(portb_r);
	DECLARE_WRITE8_MEMBER(portc_w);
	DECLARE_WRITE8_MEMBER(disp_w);
	DECLARE_WRITE8_MEMBER(lamp_w);
private:
	UINT8 m_seg[6];
	UINT8 m_portc;
	UINT8 m_motor;
	bool m_type;
	required_device<cpu_device> m_maincpu;
	required_ioport_array<7> m_switch;
	required_ioport_array<6> m_testipt;
};


static ADDRESS_MAP_START( g627_map, AS_PROGRAM, 8, g627_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0xc000, 0xc0ff) AM_DEVREADWRITE("i8156", i8155_device, memory_r, memory_w)
	AM_RANGE(0xe000, 0xe0ff) AM_RAM AM_SHARE("nvram") // battery backed
ADDRESS_MAP_END

static ADDRESS_MAP_START( g627_io, AS_IO, 8, g627_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x02) AM_WRITE(disp_w)
	AM_RANGE(0x03, 0x07) AM_WRITE(lamp_w)
	AM_RANGE(0x10, 0x17) AM_DEVWRITE("astrocade", astrocade_device, astrocade_sound_w)
	AM_RANGE(0x20, 0x27) AM_DEVREADWRITE("i8156", i8155_device, io_r, io_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( g627 )
	PORT_START("SWITCH.0")
	//bits 0,1 : optical encoder for precise table alignment. Correct position = 3.
	//bit2-7   : position of table as it turns, using Gray code.
	PORT_START("SWITCH.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Centre TB") PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Bank Shot Outlane") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Spinner") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("8 Ball Target") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_START3) PORT_NAME("Call East")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_START2) PORT_NAME("Call South")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_START1) PORT_NAME("Call North")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_START4) PORT_NAME("Call West")
	PORT_START("SWITCH.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Bottom TB") PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left Flipper Return") PORT_CODE(KEYCODE_U)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("3/11 Target") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("1/9 Target") PORT_CODE(KEYCODE_O)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("North Slam")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("North Test") PORT_CODE(KEYCODE_9)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("North Coin")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_TILT) PORT_NAME("North Tilt")
	PORT_START("SWITCH.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Top Slingshot") PORT_CODE(KEYCODE_O)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Bank Shot Advance") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("4/12 Target") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("2/10 Target") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("South Slam")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("South Test") PORT_CODE(KEYCODE_0)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("South Coin")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("South Tilt")
	PORT_START("SWITCH.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right Slingshot") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("OutHole") PORT_CODE(KEYCODE_X)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("5/13 Target") PORT_CODE(KEYCODE_G)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("4/16 Target") PORT_CODE(KEYCODE_H)
	PORT_START("SWITCH.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left Slingshot") PORT_CODE(KEYCODE_J)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right Flipper Return") PORT_CODE(KEYCODE_K)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right Out Lane") PORT_CODE(KEYCODE_L)
	PORT_START("SWITCH.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Top TB") PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("7/15 Target") PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("10 Points") PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Break Shot Hole") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left Out Lane") PORT_CODE(KEYCODE_MINUS)

	// Diagnostic Keyboard: Press GAME then END then TEST#. Press GAME etc for more tests.
	// Pressing test 8 at any time will instantly reset the NVRAM.
	PORT_START("TEST.0")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("0") PORT_CODE(KEYCODE_0_PAD)
	PORT_START("TEST.1")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("7") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("4") PORT_CODE(KEYCODE_4_PAD)
	PORT_START("TEST.2")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("SET") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME(".") PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("9") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("8") PORT_CODE(KEYCODE_8_PAD)
	PORT_START("TEST.3")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Test 3") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Test 2") PORT_CODE(KEYCODE_V)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Test 1") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("GAME") PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_START("TEST.4")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Test 7")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Test 6") PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Test 5") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Test 4") PORT_CODE(KEYCODE_N)
	PORT_START("TEST.5")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("END") PORT_CODE(KEYCODE_END)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Test 10")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Test 9")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Test 8") PORT_CODE(KEYCODE_STOP)
INPUT_PORTS_END

DRIVER_INIT_MEMBER( g627_state, v115 )
{
	m_type = 0;
}

DRIVER_INIT_MEMBER( g627_state, v117 )
{
	m_type = 1;
}

// inputs
READ8_MEMBER( g627_state::porta_r )
{
	if (!m_portc)
		return ((m_motor >> 1)^m_motor) | 3; // convert to Gray Code
	else if (m_portc < 7)
		return m_switch[m_portc]->read();

	return 0;
}

// diagnostic keyboard
READ8_MEMBER( g627_state::portb_r )
{
	if (m_portc < 6)
		return m_testipt[m_portc]->read();

	return 0;
}

// display digits
WRITE8_MEMBER( g627_state::portc_w )
{
	m_portc = data;
	if ((m_type) && (data < 6))
	{
		output().set_digit_value(data, m_seg[0]);
		output().set_digit_value(10 + data, m_seg[1]);
		output().set_digit_value(20 + data, m_seg[2]);
		output().set_digit_value(30 + data, m_seg[3]);
		output().set_digit_value(50 + data, m_seg[5]);
	}
	else
	if ((!m_type) && (data))
	{
		data--;

		output().set_digit_value(data, m_seg[0]);
		output().set_digit_value(10 + data, m_seg[1]);
		output().set_digit_value(20 + data, m_seg[2]);
		output().set_digit_value(30 + data, m_seg[3]);
		output().set_digit_value(50 + data, m_seg[5]);
	}
}

// save segments until we can write the digits
WRITE8_MEMBER( g627_state::disp_w )
{
	static const UINT8 patterns[16] = { 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7c,0x07,0x7f,0x67,0x58,0x4c,0x62,0x69,0x78,0 }; // 7448
	offset <<= 1;
	m_seg[offset] = patterns[data>>4];
	m_seg[++offset] = patterns[data&15];
}

// lamps and solenoids
WRITE8_MEMBER( g627_state::lamp_w )
{
/* offset 0 together with m_portc activates the lamps.
   offset 1 and 2 are solenoids.
   offset 1:
     d0   = Outhole
     d1   = Bank shot
     d2-4 = Bumper : centre, Bottom, Top
     d5-7 = Sling Shot : Top, Right, Left
   offset 2:
     d0   = Break Shot
     d1   = Motor clockwise*
     d2   = Motor anti-clockwise*
     d3   = 3 flippers
     d4   = unknown*

   * = undocumented

 */

	UINT16 solenoid = (offset << 8) | data;
	switch (solenoid)
	{
		case 0x0101:
		case 0x0120:
		case 0x0140:
		case 0x0180:
		case 0x0201:
			m_samples->start(0, 5);
			break;
		case 0x0104:
		case 0x0108:
		case 0x0110:
			m_samples->start(1, 0);
			break;
		case 0x0202:
		case 0x0212:
			m_motor++;
			break;
		case 0x0204:
		case 0x0214:
			m_motor--;
			break;
		default:
			break;
	}
}

static MACHINE_CONFIG_START( g627, g627_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 14138000/8)
	MCFG_CPU_PROGRAM_MAP(g627_map)
	MCFG_CPU_IO_MAP(g627_io)

	MCFG_DEVICE_ADD("i8156", I8156, 14138000/8)
	MCFG_I8155_IN_PORTA_CB(READ8(g627_state, porta_r))
	MCFG_I8155_IN_PORTB_CB(READ8(g627_state, portb_r))
	MCFG_I8155_OUT_PORTC_CB(WRITE8(g627_state, portc_w))
	MCFG_I8155_OUT_TIMEROUT_CB(INPUTLINE("maincpu", INPUT_LINE_NMI))

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* Sound */
	MCFG_FRAGMENT_ADD( genpin_audio )
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("astrocade",  ASTROCADE, 14138000/8) // 0066-117XX audio chip
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_g627)
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ Rotation VIII (09/1978)
/-------------------------------------------------------------------*/
ROM_START(rotation)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("rot-a117.dat", 0x0000, 0x0800, CRC(7bb6beb3) SHA1(5ee62246032158c68d426c11a4a9a889ee7655d7))
	ROM_LOAD("rot-b117.dat", 0x0800, 0x0800, CRC(538e37b2) SHA1(d283ac4d0024388b92b6494fcde63957b705bf48))
	ROM_LOAD("rot-c117.dat", 0x1000, 0x0800, CRC(3321ff08) SHA1(d6d94fea27ef58ca648b2829b32d62fcec108c9b))
ROM_END

ROM_START(rota_115)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("v115-a.bin", 0x0000, 0x0800, CRC(474884b3) SHA1(b7919bf2e3a3897c1180373cccf88240c57b5645))
	ROM_LOAD("v115-b.bin", 0x0800, 0x0800, CRC(8779fc6c) SHA1(df00f58d38b4eca68603247ae69009e13cfa31fb))
	ROM_LOAD("v115-c.bin", 0x1000, 0x0800, CRC(54b420f9) SHA1(597bb9f8ad0b20babc696175e9fbcecf2d5d799d))
ROM_END

ROM_START(rota_101)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("v101-a.bin", 0x0000, 0x0800, CRC(e89f3de6) SHA1(0b62220a24e176f2d7838da080b447a3df9ce05d))
	ROM_LOAD("v101-b.bin", 0x0800, 0x0800, CRC(0690670b) SHA1(6399a7df707d644d0b7fe7b4fea6fb5091a9883d))
	ROM_LOAD("v101-c.bin", 0x1000, 0x0800, CRC(c7e85638) SHA1(b59805d8b558ab8f5ea5b4b9261e862afca4b9d3))
ROM_END

GAME(1978,  rotation,  0,         g627,  g627, g627_state, v117,  ROT0,  "Midway", "Rotation VIII (v. 1.17)", MACHINE_MECHANICAL )
GAME(1978,  rota_115,  rotation,  g627,  g627, g627_state, v115,  ROT0,  "Midway", "Rotation VIII (v. 1.15)", MACHINE_MECHANICAL )
GAME(1978,  rota_101,  rotation,  g627,  g627, g627_state, v115,  ROT0,  "Midway", "Rotation VIII (v. 1.01)", MACHINE_MECHANICAL )
