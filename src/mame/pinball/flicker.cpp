// license:BSD-3-Clause
// copyright-holders:Robbbert, Vas Crabb
/*************************************************************************

PINBALL
Flicker was originally an electromechanical machine, and Bally asked
 Nutting Associates to create a solid-state prototype.

Seems to be the first ever microprocessor-controlled pinball machine.

Inputs from US Patent 4093232
Some clues from PinMAME

Note: If F3 pressed, it will remember any credits from last time.
      However, you still need to insert a coin before the start button
      will work.

The input/output multiplexing on this machine is quite clever.  RAM0
 output connected to two 1-of-16 decoders.  These are strobed using
 CM-RAM1 and CM-RAM2.  There is no RAM or I/O mapped there - the
 additional chip select lines are just used to strobe the decoders.

The programming seems to be incomplete with some bugs and omissions.
- If you score 10 then 1000 at start, the hundreds digit will be blank.
   It will fix itself during the natural course of play.
- If you enable the Match digit, and it doesn't match, the knocker
   will continually bang away. Works correctly if the match succeeds.
- The 15k to 110k switches are presumed to be the score at which a free
   game is granted, but none of that works.
   - Setting 45K and 50K can award credits
   - Setting 65K and 70K does something
- The "Add-a-ball, Replay, Straight" switches have unknown function,
   and don't seem to do anything.
- If you press some keys before starting a game, you'll be awarded the
   points when the game starts.
- So, the only settings that are known to work are the games per coin,
   and the 3/5 balls per game.
- Test button not working.

*************************************************************************/

#include "emu.h"
#include "genpin.h"
#include "cpu/mcs40/mcs40.h"

#include "flicker.lh"

namespace {

class flicker_state : public genpin_class
{
public:
	flicker_state(machine_config const &mconfig, device_type type, char const *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_testport(*this, "TEST")
		, m_coinport(*this, "COIN")
		, m_switch(*this, "X%d", 0U)
		, m_digits(*this, "digit%d", 0U)
		, m_io_outputs(*this, "out%d", 0U)
	{
	}

	ioport_value coins_in();

	DECLARE_INPUT_CHANGED_MEMBER(test_changed);

	void flicker(machine_config &config);

private:
	virtual void driver_start() override;

	void ram0_out(u8 data) { m_ram0_output = data; }
	void rom0_out(u8 data) { m_rom0_output = data; }
	void rom1_out(u8 data) { m_rom1_output = data; }
	u8 rom2_in();

	void cm_ram1_w(int state);
	void cm_ram2_w(int state);

	void flicker_memory(address_map &map) ATTR_COLD;
	void flicker_ram_ports(address_map &map) ATTR_COLD;
	void flicker_rom(address_map &map) ATTR_COLD;
	void flicker_rom_ports(address_map &map) ATTR_COLD;
	void flicker_status(address_map &map) ATTR_COLD;

	required_device<i4004_cpu_device>   m_maincpu;
	required_ioport                     m_testport;
	required_ioport                     m_coinport;
	required_ioport_array<16>           m_switch;
	output_finder<16>                   m_digits;
	output_finder<80>                   m_io_outputs;     // 16 solenoids + 64 lamps

	bool    m_cm_ram1 = false, m_cm_ram2 = false;
	u8      m_ram0_output = 0U, m_rom0_output = 0U, m_rom1_output = 0U;
	u8      m_mux_col = 0U, m_relay_drive = 0U;
};


void flicker_state::flicker_rom(address_map &map)
{
	map(0x0000, 0x03ff).rom().region("maincpu", 0);
}

void flicker_state::flicker_memory(address_map &map)
{
	map(0x0000, 0x003f).ram().share("memory");
}

void flicker_state::flicker_status(address_map &map)
{
	map(0x0000, 0x000f).ram().share("status");
}

void flicker_state::flicker_rom_ports(address_map &map)
{
	map(0x0000, 0x000f).mirror(0x0700).w(FUNC(flicker_state::rom0_out));
	map(0x0010, 0x001f).mirror(0x0700).w(FUNC(flicker_state::rom1_out));
	map(0x0020, 0x002f).mirror(0x0700).r(FUNC(flicker_state::rom2_in));
}

void flicker_state::flicker_ram_ports(address_map &map)
{
	map(0x00, 0x00).w(FUNC(flicker_state::ram0_out));
}

static INPUT_PORTS_START( flicker )
	PORT_START("TEST")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("Door Slam")     PORT_CODE(KEYCODE_0) PORT_CHANGED_MEMBER(DEVICE_SELF, flicker_state, test_changed, 0)
	PORT_BIT(0x001c, IP_ACTIVE_HIGH, IPT_UNKNOWN)  // called "two coins", "three coins", "four coins" in patent, purpose unknown
	PORT_BIT(0x07e0, IP_ACTIVE_HIGH, IPT_CUSTOM)  PORT_CUSTOM_MEMBER(flicker_state, coins_in)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("Tilt") PORT_CODE(KEYCODE_9)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_START)    PORT_NAME("Credit Button")                         PORT_CHANGED_MEMBER(DEVICE_SELF, flicker_state, test_changed, 0)
	PORT_BIT(0x6000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Test")  PORT_CODE(KEYCODE_0_PAD)          PORT_CHANGED_MEMBER(DEVICE_SELF, flicker_state, test_changed, 0)

	// The coin slot would be connected to one of the lines via a wire jumper on a terminal strip
	PORT_START("COIN")
	PORT_CONFNAME(0x3f, 0x01, DEF_STR(Coinage)) PORT_CHANGED_MEMBER(DEVICE_SELF, flicker_state, test_changed, 0)
	PORT_CONFSETTING(   0x01, DEF_STR(1C_1C))
	PORT_CONFSETTING(   0x02, DEF_STR(1C_2C))
	PORT_CONFSETTING(   0x04, DEF_STR(1C_3C))
	PORT_CONFSETTING(   0x08, DEF_STR(1C_4C))
	PORT_CONFSETTING(   0x10, DEF_STR(1C_5C))
	PORT_CONFSETTING(   0x20, DEF_STR(1C_6C))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_COIN1)   PORT_CHANGED_MEMBER(DEVICE_SELF, flicker_state, test_changed, 0)

	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("Left Lane Target")  PORT_CODE(KEYCODE_A)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("\"/B\" Target")     PORT_CODE(KEYCODE_B)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("Left Lane 1000")    PORT_CODE(KEYCODE_C)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("\"/A\" Target")     PORT_CODE(KEYCODE_D)

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("Right Lane Target") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("\"/C\" Target")     PORT_CODE(KEYCODE_F)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("Right Lane 1000")   PORT_CODE(KEYCODE_G)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("\"/D\" Target")     PORT_CODE(KEYCODE_H)

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("Spinner")           PORT_CODE(KEYCODE_I)
	PORT_BIT(0x0e, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("10's Target")       PORT_CODE(KEYCODE_J)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("100's Target")      PORT_CODE(KEYCODE_K)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("Pot Bumper")        PORT_CODE(KEYCODE_L)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("3000 Hole")         PORT_CODE(KEYCODE_M)

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("1000 Bonus")        PORT_CODE(KEYCODE_N)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("500 Targets")       PORT_CODE(KEYCODE_O)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("Out Hole")          PORT_CODE(KEYCODE_X)

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("Left 500 Out")      PORT_CODE(KEYCODE_P)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("Left Sling")        PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("Right 500 Out")     PORT_CODE(KEYCODE_R)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("Right Sling")       PORT_CODE(KEYCODE_S)

	PORT_START("X6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("\"A\" Target")      PORT_CODE(KEYCODE_T)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("\"B\" Target")      PORT_CODE(KEYCODE_U)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("\"C\" Target")      PORT_CODE(KEYCODE_V)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("\"D\" Target")      PORT_CODE(KEYCODE_W)

	PORT_START("X7")

	PORT_START("X8")

	PORT_START("X9")
	PORT_CONFNAME(0x01, 0x00, "Balls")
	PORT_CONFSETTING(   0x00, "3")
	PORT_CONFSETTING(   0x01, "5")
	PORT_BIT(0x0e, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X10")
	PORT_CONFNAME(0x01, 0x00, "Straight")
	PORT_CONFSETTING(   0x00, DEF_STR(Off))
	PORT_CONFSETTING(   0x01, DEF_STR(On))
	PORT_CONFNAME(0x02, 0x00, "Add-A-Ball")
	PORT_CONFSETTING(   0x00, DEF_STR(Off))
	PORT_CONFSETTING(   0x02, DEF_STR(On))
	PORT_CONFNAME(0x04, 0x00, "Replay")
	PORT_CONFSETTING(   0x00, DEF_STR(Off))
	PORT_CONFSETTING(   0x04, DEF_STR(On))
	PORT_CONFNAME(0x08, 0x00, "Match")
	PORT_CONFSETTING(   0x00, DEF_STR(Off))
	PORT_CONFSETTING(   0x08, DEF_STR(On))

	PORT_START("X11")
	PORT_CONFNAME(0x01, 0x00, "15K")
	PORT_CONFSETTING(   0x00, DEF_STR(Off))
	PORT_CONFSETTING(   0x01, DEF_STR(On))
	PORT_CONFNAME(0x02, 0x00, "20K")
	PORT_CONFSETTING(   0x00, DEF_STR(Off))
	PORT_CONFSETTING(   0x02, DEF_STR(On))
	PORT_CONFNAME(0x04, 0x00, "25K")
	PORT_CONFSETTING(   0x00, DEF_STR(Off))
	PORT_CONFSETTING(   0x04, DEF_STR(On))
	PORT_CONFNAME(0x08, 0x00, "30K")
	PORT_CONFSETTING(   0x00, DEF_STR(Off))
	PORT_CONFSETTING(   0x08, DEF_STR(On))

	PORT_START("X12")
	PORT_CONFNAME(0x01, 0x00, "35K")
	PORT_CONFSETTING(   0x00, DEF_STR(Off))
	PORT_CONFSETTING(   0x01, DEF_STR(On))
	PORT_CONFNAME(0x02, 0x00, "40K")
	PORT_CONFSETTING(   0x00, DEF_STR(Off))
	PORT_CONFSETTING(   0x02, DEF_STR(On))
	PORT_CONFNAME(0x04, 0x00, "45K")
	PORT_CONFSETTING(   0x00, DEF_STR(Off))
	PORT_CONFSETTING(   0x04, DEF_STR(On))
	PORT_CONFNAME(0x08, 0x00, "50K")
	PORT_CONFSETTING(   0x00, DEF_STR(Off))
	PORT_CONFSETTING(   0x08, DEF_STR(On))

	PORT_START("X13")
	PORT_CONFNAME(0x01, 0x00, "55K")
	PORT_CONFSETTING(   0x00, DEF_STR(Off))
	PORT_CONFSETTING(   0x01, DEF_STR(On))
	PORT_CONFNAME(0x02, 0x00, "60K")
	PORT_CONFSETTING(   0x00, DEF_STR(Off))
	PORT_CONFSETTING(   0x02, DEF_STR(On))
	PORT_CONFNAME(0x04, 0x00, "65K")
	PORT_CONFSETTING(   0x00, DEF_STR(Off))
	PORT_CONFSETTING(   0x04, DEF_STR(On))
	PORT_CONFNAME(0x08, 0x00, "70K")
	PORT_CONFSETTING(   0x00, DEF_STR(Off))
	PORT_CONFSETTING(   0x08, DEF_STR(On))

	PORT_START("X14")
	PORT_CONFNAME(0x01, 0x00, "75K")
	PORT_CONFSETTING(   0x00, DEF_STR(Off))
	PORT_CONFSETTING(   0x01, DEF_STR(On))
	PORT_CONFNAME(0x02, 0x00, "80K")
	PORT_CONFSETTING(   0x00, DEF_STR(Off))
	PORT_CONFSETTING(   0x02, DEF_STR(On))
	PORT_CONFNAME(0x04, 0x00, "85K")
	PORT_CONFSETTING(   0x00, DEF_STR(Off))
	PORT_CONFSETTING(   0x04, DEF_STR(On))
	PORT_CONFNAME(0x08, 0x00, "90K")
	PORT_CONFSETTING(   0x00, DEF_STR(Off))
	PORT_CONFSETTING(   0x08, DEF_STR(On))

	PORT_START("X15")
	PORT_CONFNAME(0x01, 0x00, "95K")
	PORT_CONFSETTING(   0x00, DEF_STR(Off))
	PORT_CONFSETTING(   0x01, DEF_STR(On))
	PORT_CONFNAME(0x02, 0x00, "100K")
	PORT_CONFSETTING(   0x00, DEF_STR(Off))
	PORT_CONFSETTING(   0x02, DEF_STR(On))
	PORT_CONFNAME(0x04, 0x00, "105K")
	PORT_CONFSETTING(   0x00, DEF_STR(Off))
	PORT_CONFSETTING(   0x04, DEF_STR(On))
	PORT_CONFNAME(0x08, 0x00, "110K")
	PORT_CONFSETTING(   0x00, DEF_STR(Off))
	PORT_CONFSETTING(   0x08, DEF_STR(On))
INPUT_PORTS_END


u8 flicker_state::rom2_in()
{
	return (m_switch.size() > m_mux_col) ? m_switch[m_mux_col]->read() : 0;
}


void flicker_state::cm_ram1_w(int state)
{
	static constexpr u8 led_digits[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0, 0, 0, 0, 0, 0 };
	static constexpr char const *const lamp_matrix[][4] = {
			{ nullptr,                "lamp_credit_lamp",  "lamp_flippers",    "lamp_special"            },
			{ "lamp_a_lamp",          "lamp_b_lamp",       "lamp_c_lamp",      "lamp_d_lamp"             },
			{ "lamp_not_a_lamp",      "lamp_not_b_lamp",   "lamp_not_c_lamp",  "lamp_not_d_lamp"         },
			{ "lamp_left_extra_ball", "lamp_double_bonus", "lamp_shoot_again", "lamp_right_extra_ball"   },
			{ "lamp_00_100s",         "lamp_100",          "lamp_200",         "lamp_300"                },
			{ "lamp_400",             "lamp_500",          "lamp_600",         "lamp_700"                },
			{ "lamp_800",             "lamp_900",          nullptr,            nullptr                   },
			{ "lamp_point_00",        "lamp_1000",         "lamp_2000",        "lamp_3000"               },
			{ "lamp_4000",            "lamp_5000",         "lamp_6000",        "lamp_7000"               },
			{ "lamp_8000",            "lamp_9000",         "lamp_10000",       nullptr                   },
			{ "lamp_dummy_zero",      "lamp_game_over",    "lamp_tilt",        "lamp_same_player_shoots" },
			{ "lamp_1_up",            "lamp_2_up",         "lamp_one_player",  "lamp_two_player"         } };

	if (!m_cm_ram1 && !state)
	{
		m_mux_col = m_ram0_output;
		m_digits[m_mux_col] = led_digits[m_rom0_output];
		if (std::size(lamp_matrix) > m_mux_col)
		{
			if (lamp_matrix[m_mux_col][0])
				output().set_value(lamp_matrix[m_mux_col][0], BIT(m_rom1_output, 0));
			if (lamp_matrix[m_mux_col][1])
				output().set_value(lamp_matrix[m_mux_col][1], BIT(m_rom1_output, 1));
			if (lamp_matrix[m_mux_col][2])
				output().set_value(lamp_matrix[m_mux_col][2], BIT(m_rom1_output, 2));
			if (lamp_matrix[m_mux_col][3])
				output().set_value(lamp_matrix[m_mux_col][3], BIT(m_rom1_output, 3));
			for (u8 i = 0; i < 4; i++)
				m_io_outputs[16U+m_mux_col*4U+i] = BIT(m_rom1_output, i);
		}
		if (0x0c == m_mux_col)
		{
			// TODO: BIT(m_rom1_output, 0) -> COIN ACC.
		}
		m_maincpu->set_input_line(I4004_TEST_LINE, BIT(m_testport->read(), m_mux_col));
	}
	m_cm_ram1 = !state;
}


void flicker_state::cm_ram2_w(int state)
{
	if (!m_cm_ram2 && !state && (m_relay_drive != m_ram0_output))
	{
		for (u8 i = 0; i < 16; i++)
			m_io_outputs[i] = (i == m_ram0_output);

		// The coin outputs (A and B) aren't used
		switch (m_relay_drive = m_ram0_output)
		{
		case 0x01: // 10 chime
			m_samples->start(3, 3);
			break;
		case 0x02: // 100 chime
			m_samples->start(2, 2);
			break;
		case 0x03: // 1000 chime
			m_samples->start(1, 1);
			break;
		case 0x04: // left sling
		case 0x05: // right sling
			m_samples->start(0, 7);
			break;
		case 0x06: // pot bumper
			m_samples->start(0, 0);
			break;
		case 0x07: // out hole
		case 0x08: // 3000 hole
			m_samples->start(5, 5);
			break;
		case 0x09: // door knocker
			m_samples->start(0, 6);
			break;
		case 0x0a: // coin counter
			logerror("coin counter\n");
			break;
		case 0x0b: // coin acceptor
			logerror("coin acceptor\n");
			break;
		default: // 0/C/D/E/F not connected
			break;
		}
	}
	m_cm_ram2 = !state;
}


ioport_value flicker_state::coins_in()
{
	u8 const coins(m_coinport->read());
	return BIT(coins, 7) ? (coins & 0x3f) : 0;
}


INPUT_CHANGED_MEMBER(flicker_state::test_changed)
{
	m_maincpu->set_input_line(I4004_TEST_LINE, BIT(m_testport->read(), m_mux_col));
}


void flicker_state::driver_start()
{
	m_digits.resolve();
	m_io_outputs.resolve();

	save_item(NAME(m_cm_ram1));
	save_item(NAME(m_cm_ram2));
	save_item(NAME(m_ram0_output));
	save_item(NAME(m_rom0_output));
	save_item(NAME(m_rom1_output));
	save_item(NAME(m_relay_drive));
}


void flicker_state::flicker(machine_config &config)
{
	// basic machine hardware
	I4004(config, m_maincpu, 5_MHz_XTAL / 8);
	m_maincpu->set_rom_map(&flicker_state::flicker_rom);
	m_maincpu->set_ram_memory_map(&flicker_state::flicker_memory);
	m_maincpu->set_rom_ports_map(&flicker_state::flicker_rom_ports);
	m_maincpu->set_ram_status_map(&flicker_state::flicker_status);
	m_maincpu->set_ram_ports_map(&flicker_state::flicker_ram_ports);
	m_maincpu->cm_ram_cb<1>().set(FUNC(flicker_state::cm_ram1_w));
	m_maincpu->cm_ram_cb<2>().set(FUNC(flicker_state::cm_ram2_w));

	// video
	config.set_default_layout(layout_flicker);

	// sound
	genpin_audio(config);
}


ROM_START(flicker)
	ROM_REGION(0x0400, "maincpu", 0)
	ROM_LOAD("flicker.rom", 0x0000, 0x0400, CRC(c692e586) SHA1(5cabb28a074d18b589b5b8f700c57e1610071c68))
ROM_END

} // Anonymous namespace

//    YEAR   GAME      PARENT  MACHINE   INPUT    CLASS           INIT        ORIENTATION  COMPANY                            DESCRIPTION            FLAGS
GAME( 1974,  flicker,  0,      flicker,  flicker, flicker_state,  empty_init, ROT0,        "Dave Nutting Associates / Bally", "Flicker (prototype)", MACHINE_IS_INCOMPLETE | MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
