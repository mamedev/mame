/******************************************************************************

    Pinball
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

Machine Operation:
1. Press num-enter then END then .(period key)   (the displays will flash 400000)
2. Press num-enter
3. Insert a coin, credit will be registered
4. Hold X and start game. When 00 is flashing, release X
5. Press any of QWERYUIOASDFGHJKLZ-='; to simulate scoring shots (T will tilt)
6. Press and hold X to simulate losing the ball
7. When score starts flashing, release X and go to step 5 to play next ball

Notes: Do not play more than one player because the machine will try to
rotate the table, and the motor circuits are not emulated due to lack of info.
This means that the score labels of East, West and South are not verified.

The manual explains the tests available, and also how to set number of balls,
high score, etc., with the diagnostic keyboard.

ToDo:
- Battery backup
- Outputs
- Simulate motor circuitry and sensor feedback
- Verify labels of East, West and South on the display panel
- Possibility of a rom missing (most likely it is optional)

*******************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/i8155.h"
#include "sound/astrocde.h"
#include "g627.lh"


class g627_state : public driver_device
{
public:
	g627_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu")
	{ }

	DECLARE_READ8_MEMBER(porta_r);
	DECLARE_READ8_MEMBER(portb_r);
	DECLARE_WRITE8_MEMBER(portc_w);
	DECLARE_WRITE8_MEMBER(disp_w);
	DECLARE_WRITE8_MEMBER(lamp_w);

protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset();
private:
	UINT8 m_seg[6];
	UINT8 m_portc;
};


static ADDRESS_MAP_START( g627_map, AS_PROGRAM, 8, g627_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0xc000, 0xc0ff) AM_DEVREADWRITE("i8156", i8155_device, memory_r, memory_w)
	AM_RANGE(0xe000, 0xe0ff) AM_RAM // battery backed
ADDRESS_MAP_END

static ADDRESS_MAP_START( g627_io, AS_IO, 8, g627_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x02) AM_WRITE(disp_w)
	AM_RANGE(0x03, 0x07) AM_WRITE(lamp_w)
	AM_RANGE(0x10, 0x17) AM_DEVWRITE_LEGACY("astrocade", astrocade_sound_w)
	AM_RANGE(0x20, 0x27) AM_DEVREADWRITE("i8156", i8155_device, io_r, io_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( g627 )
	PORT_START("X0")
	PORT_BIT(0x03, IP_ACTIVE_LOW, IPT_UNUSED) // force 3 here so game can start
	//bits 0,1 : optical encoder for precise table alignment. Correct position = 3.
	//bit2-7   : position of table as it turns, using Gray code.
	// code to convert a number to a Gray Code: { return (input >> 1)^input; }
	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Centre TB") PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Bank Shot Outlane") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Spinner") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("8 Ball Target") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_START3) PORT_NAME("Call East")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_START2) PORT_NAME("Call South")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_START1) PORT_NAME("Call North")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_START4) PORT_NAME("Call West")
	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Bottom TB") PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left Flipper Return") PORT_CODE(KEYCODE_U)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("3/11 Target") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("1/9 Target") PORT_CODE(KEYCODE_O)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("North Slam")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("North Test") PORT_CODE(KEYCODE_9)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("North Coin")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_TILT) PORT_NAME("North Tilt")
	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Top Slingshot") PORT_CODE(KEYCODE_O)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Bank Shot Advance") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("4/12 Target") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("2/10 Target") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("South Slam")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("South Test") PORT_CODE(KEYCODE_0)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("South Coin")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("South Tilt")
	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right Slingshot") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("OutHole") PORT_CODE(KEYCODE_X)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("5/13 Target") PORT_CODE(KEYCODE_G)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("4/16 Target") PORT_CODE(KEYCODE_H)
	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left Slingshot") PORT_CODE(KEYCODE_J)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right Flipper Return") PORT_CODE(KEYCODE_K)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right Out Lane") PORT_CODE(KEYCODE_L)
	PORT_START("X6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Top TB") PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("7/15 Target") PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("10 Points") PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Break Shot Hole") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left Out Lane") PORT_CODE(KEYCODE_MINUS)

	// Diagnostic Keyboard: Press GAME then END then TEST#. Press GAME etc for more tests.
	// Pressing test 8 at any time will instantly reset the NVRAM.
	PORT_START("Y0")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("0") PORT_CODE(KEYCODE_0_PAD)
	PORT_START("Y1")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("7") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("4") PORT_CODE(KEYCODE_4_PAD)
	PORT_START("Y2")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("SET") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME(".") PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("9") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("8") PORT_CODE(KEYCODE_8_PAD)
	PORT_START("Y3")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Test 3") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Test 2") PORT_CODE(KEYCODE_V)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Test 1") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("GAME") PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_START("Y4")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Test 7")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Test 6") PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Test 5") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Test 4") PORT_CODE(KEYCODE_N)
	PORT_START("Y5")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("END") PORT_CODE(KEYCODE_END)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Test 10") 
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Test 9") 
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Test 8") PORT_CODE(KEYCODE_STOP)
INPUT_PORTS_END

void g627_state::machine_reset()
{

}

// inputs
READ8_MEMBER( g627_state::porta_r )
{
	if (m_portc < 7)
	{
		char kbdrow[6];
		sprintf(kbdrow,"X%X",m_portc);
		return ioport(kbdrow)->read();
	}
	return 0;
}

// diagnostic keyboard
READ8_MEMBER( g627_state::portb_r )
{
	if (m_portc < 6)
	{
		char kbdrow[6];
		sprintf(kbdrow,"Y%X",m_portc);
		return ioport(kbdrow)->read();
	}
	return 0;
}

// write 6 digits
WRITE8_MEMBER( g627_state::portc_w )
{
	m_portc = data;
	if (data < 6)
	{
		output_set_digit_value(data, m_seg[0]);
		output_set_digit_value(10 + data, m_seg[1]);
		output_set_digit_value(20 + data, m_seg[2]);
		output_set_digit_value(30 + data, m_seg[3]);
		output_set_digit_value(50 + data, m_seg[5]);
	}
}

// save segments until we can write the digits
WRITE8_MEMBER( g627_state::disp_w )
{
	static const UINT8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0, 0, 0, 0, 0, 0 };
	offset <<= 1;
	m_seg[offset] = patterns[data>>4];
	m_seg[++offset] = patterns[data&15];
}

// lamps and solenoids
WRITE8_MEMBER( g627_state::lamp_w )
{
}

static I8156_INTERFACE(i8156_intf)
{
	DEVCB_DRIVER_MEMBER(g627_state,porta_r), // Port A in
	DEVCB_NULL, // Port A out
	DEVCB_DRIVER_MEMBER(g627_state,portb_r), // Port B in
	DEVCB_NULL, // Port B out
	DEVCB_NULL, // Port C in
	DEVCB_DRIVER_MEMBER(g627_state,portc_w), // Port C out
	DEVCB_CPU_INPUT_LINE("maincpu", INPUT_LINE_NMI) // timer out
};

static MACHINE_CONFIG_START( g627, g627_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 14138000/8)
	MCFG_CPU_PROGRAM_MAP(g627_map)
	MCFG_CPU_IO_MAP(g627_io)
	MCFG_I8156_ADD("i8156", 14138000/8, i8156_intf)
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
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF) // schematic shows 4x 2716 roms
	ROM_LOAD("rot-a117.dat", 0x0000, 0x0800, CRC(7bb6beb3) SHA1(5ee62246032158c68d426c11a4a9a889ee7655d7))
	ROM_LOAD("rot-b117.dat", 0x0800, 0x0800, CRC(538e37b2) SHA1(d283ac4d0024388b92b6494fcde63957b705bf48))
	ROM_LOAD("rot-c117.dat", 0x1000, 0x0800, CRC(3321ff08) SHA1(d6d94fea27ef58ca648b2829b32d62fcec108c9b))
ROM_END


GAME(1978,  rotation,  0,  g627,  g627, driver_device,  0,  ROT0,  "Midway", "Rotation VIII", GAME_MECHANICAL )
