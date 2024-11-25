// license:BSD-3-Clause
// copyright-holders:Robbbert
/******************************************************************************

PINBALL
Midway A084-91313-G627
       A080-91313-G627
       A082-91320-C000

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
4. Press any of A thru U to simulate scoring shots
5. Press and hold X to simulate losing the ball
6. When score starts flashing, release X and go to step 4 to play next ball

Note: You can start play at any of the 4 stations, and the table will rotate
      as needed. Multiplayer works as well.

The manual explains the tests available, and also how to set number of balls,
high score, etc., with the diagnostic keyboard.

Status:
- Game is working

ToDo:
- Nothing

*******************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "genpin.h"
#include "machine/i8155.h"
#include "sound/astrocde.h"
#include "screen.h"
#include "speaker.h"

#include "g627.lh"

namespace {

class g627_state : public genpin_class
{
public:
	g627_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_io_keyboard(*this, "X%d", 0U)
		, m_testipt(*this, "T%d", 0U)
		, m_digits(*this, "digit%d", 0U)
		, m_io_outputs(*this, "out%d", 0U)
	{ }

	void g627(machine_config &config);

	void init_v115();
	void init_v117();

private:
	u8 porta_r();
	u8 portb_r();
	void portc_w(u8 data);
	void disp_w(offs_t offset, u8 data);
	void lamp_w(offs_t offset, u8 data);
	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;
	u8 m_seg[6]{};
	u8 m_portc = 0U;
	u8 m_motor = 0U;
	u8 m_last_solenoid = 0U;
	bool m_type = false;
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	required_device<cpu_device> m_maincpu;
	required_ioport_array<7> m_io_keyboard;
	required_ioport_array<6> m_testipt;
	output_finder<56> m_digits;
	output_finder<72> m_io_outputs;  // 16 solenoids + 56 lamps
};


void g627_state::mem_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0xc000, 0xc0ff).rw("i8156", FUNC(i8155_device::memory_r), FUNC(i8155_device::memory_w));
	map(0xe000, 0xe0ff).ram().share("nvram"); // battery backed
}

void g627_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x02).w(FUNC(g627_state::disp_w));
	map(0x03, 0x07).w(FUNC(g627_state::lamp_w));
	map(0x10, 0x17).w("astrocade", FUNC(astrocade_io_device::write));
	map(0x20, 0x27).rw("i8156", FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
}

static INPUT_PORTS_START( g627 )
	PORT_START("X0")
	//bits 0,1 : optical encoder for precise table alignment. Correct position = 3.
	//bit2-7   : position of table as it turns, using Gray code.
	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Centre TB")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("Bank Shot Outlane")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Spinner")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("8 Ball Target")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_START3) PORT_NAME("Call East")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_START2) PORT_NAME("Call South")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_START1) PORT_NAME("Call North")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_START4) PORT_NAME("Call West")
	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Bottom TB")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Left Flipper Return") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("3/11 Target")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("1/9 Target")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_NAME("North Slam")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_NAME("North Test")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("North Coin")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_NAME("North Tilt")
	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Top Slingshot")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Bank Shot Advance")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME("4/12 Target")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_K) PORT_NAME("2/10 Target")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("South Slam")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_NAME("South Test")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("South Coin")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("South Tilt")
	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Right Slingshot")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("OutHole")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("5/13 Target")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("4/16 Target")
	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Left Slingshot")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Right Flipper Return") PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("Right Out Lane")
	PORT_START("X6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Top TB")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("7/15 Target")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("10 Points")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Break Shot Hole")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Left Out Lane")

	// Diagnostic Keyboard: Press GAME then END then TEST#. Press GAME etc for more tests.
	// Pressing test 8 at any time will instantly reset the NVRAM.
	PORT_START("T0")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("0") PORT_CODE(KEYCODE_0_PAD)
	PORT_START("T1")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("7") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("4") PORT_CODE(KEYCODE_4_PAD)
	PORT_START("T2")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("SET") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME(".") PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("9") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("8") PORT_CODE(KEYCODE_8_PAD)
	PORT_START("T3")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Test 3") PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Test 2") PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Test 1") PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("GAME") PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_START("T4")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Test 7") PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Test 6") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Test 5") PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Test 4") PORT_CODE(KEYCODE_COLON)
	PORT_START("T5")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("END") PORT_CODE(KEYCODE_END)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Test 10") PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Test 9") PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Test 8") PORT_CODE(KEYCODE_STOP)
INPUT_PORTS_END

void g627_state::init_v117()
{
	m_type = 1;
}

// inputs
u8 g627_state::porta_r()
{
	if (!m_portc)
		return ((m_motor >> 1)^m_motor) | 3; // convert to Gray Code
	else if (m_portc < 7)
		return m_io_keyboard[m_portc]->read();

	return 0;
}

// diagnostic keyboard
u8 g627_state::portb_r()
{
	if (m_portc < 6)
		return m_testipt[m_portc]->read();

	return 0;
}

// display digits
void g627_state::portc_w(u8 data)
{
	m_portc = data;
	if ((m_type) && (data < 6))
	{
		m_digits[data] = m_seg[0];
		m_digits[10 + data] = m_seg[1];
		m_digits[20 + data] = m_seg[2];
		m_digits[30 + data] = m_seg[3];
		m_digits[50 + data] = m_seg[5];
	}
	else
	if ((!m_type) && (data) && (data < 7))
	{
		data--;

		m_digits[data] = m_seg[0];
		m_digits[10 + data] = m_seg[1];
		m_digits[20 + data] = m_seg[2];
		m_digits[30 + data] = m_seg[3];
		m_digits[50 + data] = m_seg[5];
	}
}

// save segments until we can write the digits
void g627_state::disp_w(offs_t offset, u8 data)
{
	static const u8 patterns[16] = { 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7c,0x07,0x7f,0x67,0x58,0x4c,0x62,0x69,0x78,0 }; // 7448
	offset <<= 1;
	m_seg[offset] = patterns[data>>4];
	m_seg[++offset] = patterns[data&15];
}

// lamps and solenoids
void g627_state::lamp_w(offs_t offset, u8 data)
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

	u8 i;
	// Lamps
	if (!offset && (m_portc < 7))
	{
		for (i = 0; i < 8; i++)
			m_io_outputs[16+m_portc*8+i] = BIT(data , i);
	}
	else
	if (offset < 3)
	// Solenoids
	{
		bool is_sol = false;
		m_io_outputs[m_last_solenoid] = 0;
		u16 solenoid = (offset << 8) | data;
		switch (solenoid)
		{
			case 0x0101:
			case 0x0120:
			case 0x0140:
			case 0x0180:
			case 0x0201:
				is_sol = true;
				m_samples->start(0, 5);
				break;
			case 0x0104:
			case 0x0108:
			case 0x0110:
				is_sol = true;
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
		if (is_sol)
		{
			u8 j = 9;
			for (i = 0; i < 8; i++)
				if (BIT(data, i))
					j = i;
			if (j < 8)
			{
				m_last_solenoid = (offset-1)* 8 + j;
				m_io_outputs[m_last_solenoid] = 1;
			}
		}
	}
}

void g627_state::machine_start()
{
	genpin_class::machine_start();
	m_digits.resolve();
	m_io_outputs.resolve();

	save_item(NAME(m_seg));
	save_item(NAME(m_portc));
	save_item(NAME(m_motor));
	save_item(NAME(m_type));
	save_item(NAME(m_last_solenoid));
}

void g627_state::machine_reset()
{
	genpin_class::machine_reset();
	for (u8 i = 0; i < m_io_outputs.size(); i++)
		m_io_outputs[i] = 0;
}

void g627_state::g627(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 14138000/8);
	m_maincpu->set_addrmap(AS_PROGRAM, &g627_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &g627_state::io_map);

	i8156_device &i8156(I8156(config, "i8156", 14138000/8));
	i8156.in_pa_callback().set(FUNC(g627_state::porta_r));
	i8156.in_pb_callback().set(FUNC(g627_state::portb_r));
	i8156.out_pc_callback().set(FUNC(g627_state::portc_w));
	i8156.out_to_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* Sound */
	genpin_audio(config);
	SPEAKER(config, "mono").front_center();
	ASTROCADE_IO(config, "astrocade", 14138000/8).add_route(ALL_OUTPUTS, "mono", 1.0); // 0066-117XX audio chip

	/* Video */
	config.set_default_layout(layout_g627);
}

/*-------------------------------------------------------------------
/ Rotation VIII (09/1978)
/-------------------------------------------------------------------*/
ROM_START(rotation)
	ROM_REGION(0x2000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("rot-a117.dat", 0x0000, 0x0800, CRC(7bb6beb3) SHA1(5ee62246032158c68d426c11a4a9a889ee7655d7))
	ROM_LOAD("rot-b117.dat", 0x0800, 0x0800, CRC(538e37b2) SHA1(d283ac4d0024388b92b6494fcde63957b705bf48))
	ROM_LOAD("rot-c117.dat", 0x1000, 0x0800, CRC(3321ff08) SHA1(d6d94fea27ef58ca648b2829b32d62fcec108c9b))
ROM_END

ROM_START(rota_115)
	ROM_REGION(0x2000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("v115-a.bin", 0x0000, 0x0800, CRC(474884b3) SHA1(b7919bf2e3a3897c1180373cccf88240c57b5645))
	ROM_LOAD("v115-b.bin", 0x0800, 0x0800, CRC(8779fc6c) SHA1(df00f58d38b4eca68603247ae69009e13cfa31fb))
	ROM_LOAD("v115-c.bin", 0x1000, 0x0800, CRC(54b420f9) SHA1(597bb9f8ad0b20babc696175e9fbcecf2d5d799d))
ROM_END

ROM_START(rota_101)
	ROM_REGION(0x2000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("v101-a.bin", 0x0000, 0x0800, CRC(e89f3de6) SHA1(0b62220a24e176f2d7838da080b447a3df9ce05d))
	ROM_LOAD("v101-b.bin", 0x0800, 0x0800, CRC(0690670b) SHA1(6399a7df707d644d0b7fe7b4fea6fb5091a9883d))
	ROM_LOAD("v101-c.bin", 0x1000, 0x0800, CRC(c7e85638) SHA1(b59805d8b558ab8f5ea5b4b9261e862afca4b9d3))
ROM_END

} // anonymous namespace

GAME( 1978, rotation, 0,        g627, g627, g627_state, init_v117,  ROT0, "Midway", "Rotation VIII (v. 1.17)", MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1978, rota_115, rotation, g627, g627, g627_state, empty_init, ROT0, "Midway", "Rotation VIII (v. 1.15)", MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1978, rota_101, rotation, g627, g627, g627_state, empty_init, ROT0, "Midway", "Rotation VIII (v. 1.01)", MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
