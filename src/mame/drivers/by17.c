// license:BSD-3-Clause
// copyright-holders:Robbbert, Quench
/********************************************************************************************

    PINBALL
    Bally MPU AS-2518-17

    These are some very early and well known Solid State machines, such as 'Eight Ball'.

    They have orange digital 6 digit displays, and a mechanical chime unit for sounds.


ToDo:
- Bow & Arrow fails the PIA test and doesn't boot
- Dips, Inputs, Solenoids vary per game
- Mechanical

*********************************************************************************************/


#include "machine/genpin.h"
#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "by17.lh"
#include "by17_pwerplay.lh"
#include "by17_matahari.lh"
#include "render.h"

class by17_state : public genpin_class
{
public:
	by17_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_nvram(*this, "nvram")
		, m_pia_u10(*this, "pia_u10")
		, m_pia_u11(*this, "pia_u11")
		, m_io_test(*this, "TEST")
		, m_io_dsw0(*this, "DSW0")
		, m_io_dsw1(*this, "DSW1")
		, m_io_dsw2(*this, "DSW2")
		, m_io_dsw3(*this, "DSW3")
		, m_io_x0(*this, "X0")
		, m_io_x1(*this, "X1")
		, m_io_x2(*this, "X2")
		, m_io_x3(*this, "X3")
		, m_io_x4(*this, "X4")
	{ }

	DECLARE_DRIVER_INIT(by17);
	DECLARE_DRIVER_INIT(matahari);
	DECLARE_DRIVER_INIT(pwerplay);
	DECLARE_READ8_MEMBER(u10_a_r);
	DECLARE_WRITE8_MEMBER(u10_a_w);
	DECLARE_READ8_MEMBER(u10_b_r);
	DECLARE_WRITE8_MEMBER(u10_b_w);
	DECLARE_READ8_MEMBER(u11_a_r);
	DECLARE_WRITE8_MEMBER(u11_a_w);
	DECLARE_WRITE8_MEMBER(u11_b_w);
	DECLARE_READ8_MEMBER(nibble_nvram_r);
	DECLARE_WRITE8_MEMBER(nibble_nvram_w);
	DECLARE_READ_LINE_MEMBER(u10_ca1_r);
	DECLARE_READ_LINE_MEMBER(u10_cb1_r);
	DECLARE_WRITE_LINE_MEMBER(u10_ca2_w);
	DECLARE_WRITE_LINE_MEMBER(u10_cb2_w);
	DECLARE_READ_LINE_MEMBER(u11_ca1_r);
	DECLARE_READ_LINE_MEMBER(u11_cb1_r);
	DECLARE_WRITE_LINE_MEMBER(u11_ca2_w);
	DECLARE_WRITE_LINE_MEMBER(u11_cb2_w);
	DECLARE_INPUT_CHANGED_MEMBER(activity_button);
	DECLARE_INPUT_CHANGED_MEMBER(self_test);
	DECLARE_CUSTOM_INPUT_MEMBER(outhole_x0);
	DECLARE_CUSTOM_INPUT_MEMBER(saucer_x3);
	DECLARE_CUSTOM_INPUT_MEMBER(drop_target_x2);
	DECLARE_MACHINE_RESET(by17);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_z_freq);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_z_pulse);
	TIMER_DEVICE_CALLBACK_MEMBER(u11_timer);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_d_pulse);
private:
	UINT8 m_u10a;
	UINT8 m_u10b;
	UINT8 m_u11a;
	UINT8 m_u11b;
	bool m_u10_ca2;
	bool m_u10_cb1;
	bool m_u10_cb2;
	bool m_u11_ca1;
	bool m_u11_cb2;
	UINT8 m_digit;
	UINT8 m_segment[6];
	UINT8 m_lamp_decode;
	UINT8 m_solenoid_features[20][4];
	UINT8 m_io_hold_x[5];       // Used to hold switches closed (drop targets, balls in outholes/saucers etc). Solenoid activity release them.
	required_device<m6800_cpu_device> m_maincpu;
	required_shared_ptr<UINT8> m_nvram;
	required_device<pia6821_device> m_pia_u10;
	required_device<pia6821_device> m_pia_u11;
	required_ioport m_io_test;
	required_ioport m_io_dsw0;
	required_ioport m_io_dsw1;
	required_ioport m_io_dsw2;
	required_ioport m_io_dsw3;
	required_ioport m_io_x0;
	required_ioport m_io_x1;
	required_ioport m_io_x2;
	required_ioport m_io_x3;
	required_ioport m_io_x4;
};


static ADDRESS_MAP_START( by17_map, AS_PROGRAM, 8, by17_state )
	ADDRESS_MAP_GLOBAL_MASK(0x1fff)
	AM_RANGE(0x0000, 0x007f) AM_RAM
	AM_RANGE(0x0088, 0x008b) AM_DEVREADWRITE("pia_u10", pia6821_device, read, write)
	AM_RANGE(0x0090, 0x0093) AM_DEVREADWRITE("pia_u11", pia6821_device, read, write)
	AM_RANGE(0x0200, 0x02ff) AM_RAM AM_READWRITE(nibble_nvram_r, nibble_nvram_w) AM_SHARE("nvram")
	AM_RANGE(0x1000, 0x1fff) AM_ROM AM_REGION("roms", 0 )
ADDRESS_MAP_END


static INPUT_PORTS_START( by17 )
	PORT_START("TEST")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Self Test") PORT_CHANGED_MEMBER(DEVICE_SELF, by17_state, self_test, NULL)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Activity")  PORT_CHANGED_MEMBER(DEVICE_SELF, by17_state, activity_button, NULL)

	PORT_START("DSW0")
	PORT_DIPNAME( 0x1f, 0x02, "Coin Slot 1")                PORT_DIPLOCATION("SW0:!1,!2,!3,!4,!5") // same as 03
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ))
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_2C ))
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ))
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_4C ))
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(    0x0b, DEF_STR( 2C_5C ))
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_6C ))
	PORT_DIPSETTING(    0x0d, DEF_STR( 2C_6C ))
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_7C ))
	PORT_DIPSETTING(    0x0f, DEF_STR( 2C_7C ))
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_8C ))
	PORT_DIPSETTING(    0x11, DEF_STR( 2C_8C ))
	PORT_DIPSETTING(    0x12, DEF_STR( 1C_9C ))
	PORT_DIPSETTING(    0x13, "2 Coins/9 Credits")
	PORT_DIPSETTING(    0x14, "1 Coin/10 Credits")
	PORT_DIPSETTING(    0x15, "2 Coins/10 Credits")
	PORT_DIPSETTING(    0x16, "1 Coin/11 Credits")
	PORT_DIPSETTING(    0x17, "2 Coins/11 Credits")
	PORT_DIPSETTING(    0x18, "1 Coin/12 Credits")
	PORT_DIPSETTING(    0x19, "2 Coins/12 Credits")
	PORT_DIPSETTING(    0x1a, "1 Coin/13 Credits")
	PORT_DIPSETTING(    0x1b, "2 Coins/13 Credits")
	PORT_DIPSETTING(    0x1c, "1 Coin/14 Credits")
	PORT_DIPSETTING(    0x1d, "2 Coins/14 Credits")
	PORT_DIPSETTING(    0x1e, "1 Coin/15 Credits")
	PORT_DIPSETTING(    0x1f, "2 Coins/15 Credits")
	PORT_DIPNAME( 0x20, 0x20, "Score Level Award")          PORT_DIPLOCATION("SW0:!6")
	PORT_DIPSETTING(    0x00, "Extra Ball")
	PORT_DIPSETTING(    0x20, "Replay")
	PORT_DIPNAME( 0x40, 0x00, "S07")                        PORT_DIPLOCATION("SW0:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x40, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x80, "Play Melodies")              PORT_DIPLOCATION("SW0:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))

	PORT_START("DSW1")
	PORT_DIPNAME( 0x1f, 0x02, "Coin Slot 3")                PORT_DIPLOCATION("SW1:!1,!2,!3,!4,!5") // same as 01
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ))
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_2C ))
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ))
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_4C ))
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(    0x0b, DEF_STR( 2C_5C ))
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_6C ))
	PORT_DIPSETTING(    0x0d, DEF_STR( 2C_6C ))
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_7C ))
	PORT_DIPSETTING(    0x0f, DEF_STR( 2C_7C ))
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_8C ))
	PORT_DIPSETTING(    0x11, DEF_STR( 2C_8C ))
	PORT_DIPSETTING(    0x12, DEF_STR( 1C_9C ))
	PORT_DIPSETTING(    0x13, "2 Coins/9 Credits")
	PORT_DIPSETTING(    0x14, "1 Coin/10 Credits")
	PORT_DIPSETTING(    0x15, "2 Coins/10 Credits")
	PORT_DIPSETTING(    0x16, "1 Coin/11 Credits")
	PORT_DIPSETTING(    0x17, "2 Coins/11 Credits")
	PORT_DIPSETTING(    0x18, "1 Coin/12 Credits")
	PORT_DIPSETTING(    0x19, "2 Coins/12 Credits")
	PORT_DIPSETTING(    0x1a, "1 Coin/13 Credits")
	PORT_DIPSETTING(    0x1b, "2 Coins/13 Credits")
	PORT_DIPSETTING(    0x1c, "1 Coin/14 Credits")
	PORT_DIPSETTING(    0x1d, "2 Coins/14 Credits")
	PORT_DIPSETTING(    0x1e, "1 Coin/15 Credits")
	PORT_DIPSETTING(    0x1f, "2 Coins/15 Credits")
	PORT_DIPNAME( 0x20, 0x00, "S14")                                PORT_DIPLOCATION("SW1:!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x40, "Award for Beating Highest Score")    PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(    0x00, "Novelty")
	PORT_DIPSETTING(    0x40, "3 Credits")
	PORT_DIPNAME( 0x80, 0x80, "Balls per Game")                     PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(    0x00, "3")
	PORT_DIPSETTING(    0x80, "5")

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x01, "Maximum Credits")        PORT_DIPLOCATION("SW2:!1,!2,!3")
	PORT_DIPSETTING(    0x00, "5")
	PORT_DIPSETTING(    0x01, "10")
	PORT_DIPSETTING(    0x02, "15")
	PORT_DIPSETTING(    0x03, "20")
	PORT_DIPSETTING(    0x04, "25")
	PORT_DIPSETTING(    0x05, "30")
	PORT_DIPSETTING(    0x06, "35")
	PORT_DIPSETTING(    0x07, "40")
	PORT_DIPNAME( 0x08, 0x08, "Credits Displayed")      PORT_DIPLOCATION("SW2:!4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x08, DEF_STR( On ))
	PORT_DIPNAME( 0x10, 0x10, "Match")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))           PORT_DIPLOCATION("SW2:!5")
	PORT_DIPSETTING(    0x10, DEF_STR( On ))
	// from here, game-specific options
	PORT_DIPNAME( 0x20, 0x00, "S22 (game specific)")    PORT_DIPLOCATION("SW2:!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, "S23 (game specific)")    PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x40, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x00, "No Free Balls or Games") PORT_DIPLOCATION("SW2:!8")   // night rider
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))

	PORT_START("DSW3")
	PORT_DIPNAME( 0x0f, 0x00, "Coin Slot 2")            PORT_DIPLOCATION("SW3:!1,!2,!3,!4")
	PORT_DIPSETTING(    0x00, "Same as Slot 1")
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_6C ))
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_7C ))
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_8C ))
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_9C ))
	PORT_DIPSETTING(    0x0a, "1 Coin/10 Credits")
	PORT_DIPSETTING(    0x0b, "1 Coin/11 Credits")
	PORT_DIPSETTING(    0x0c, "1 Coin/12 Credits")
	PORT_DIPSETTING(    0x0d, "1 Coin/13 Credits")
	PORT_DIPSETTING(    0x0e, "1 Coin/14 Credits")
	PORT_DIPSETTING(    0x0f, "1 Coin/15 Credits")
	PORT_DIPNAME( 0x10, 0x00, "S29 (game specific)")    PORT_DIPLOCATION("SW3:!5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x10, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x00, "S30 (game specific)")    PORT_DIPLOCATION("SW3:!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, "Awards")                 PORT_DIPLOCATION("SW3:!7") // night rider
	PORT_DIPSETTING(    0x00, "Conservative")
	PORT_DIPSETTING(    0x40, "Liberal")
	PORT_DIPNAME( 0x80, 0x00, "Lane Adjustment")        PORT_DIPLOCATION("SW3:!8") // night rider
	PORT_DIPSETTING(    0x00, "Conservative")
	PORT_DIPSETTING(    0x80, "Liberal")

	PORT_START("X0")
	// custom
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_STOP)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_SLASH)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSLASH)
	// standard
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_TILT )
//  PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Outhole") PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, outhole_x0, (void *)0x07)  //  PORT_CODE(KEYCODE_BACKSPACE)

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_ENTER)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_COLON)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_TILT2 ) PORT_NAME("Slam Tilt") PORT_CODE(KEYCODE_EQUALS)

	// custom
	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_A)

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_Q)

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_COMMA)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_Z)
INPUT_PORTS_END

static INPUT_PORTS_START( matahari )
	PORT_INCLUDE( by17 )

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x60, 0x60, "Award for Beating Highest Score")        PORT_DIPLOCATION("SW0:!6,!7")
	PORT_DIPSETTING(    0x00, "Nothing")
	PORT_DIPSETTING(    0x20, "1 Credit")
	PORT_DIPSETTING(    0x40, "2 Credits")
	PORT_DIPSETTING(    0x60, "3 Credits")

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x40, 0x00, "S15")                                    PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x40, DEF_STR( On ))

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x20, 0x00, "S22")                                    PORT_DIPLOCATION("SW2:!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, "Saucer Award Feature")                   PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(    0x00, "Start at 3000 Points")
	PORT_DIPSETTING(    0x40, "Start at 2X Bonus")
	PORT_DIPNAME( 0x80, 0x00, "A & B Special Award Feature Per Ball")   PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(    0x00, "Award Special Once")
	PORT_DIPSETTING(    0x80, "Award Special Alternates")

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x10, 0x00, "S29")                                    PORT_DIPLOCATION("SW3:!5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x10, DEF_STR( On ))
	PORT_DIPNAME( 0x60, 0x60, "Extra Ball / Specials Award Mode")       PORT_DIPLOCATION("SW3:!6,!7")
	PORT_DIPSETTING(    0x00, "Novelty / 50,000")
//  PORT_DIPSETTING(    0x20, "")
	PORT_DIPSETTING(    0x40, "Extra Ball / 50,000")
	PORT_DIPSETTING(    0x60, "Extra Ball / Replay")
	PORT_DIPNAME( 0x80, 0x80, "Score Level Award")                      PORT_DIPLOCATION("SW3:!8")
	PORT_DIPSETTING(    0x00, "Extra Ball")
	PORT_DIPSETTING(    0x80, "Replay")

	PORT_MODIFY("X2")   /* Drop Target switches */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, drop_target_x2, (void *)0x20)  // PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, drop_target_x2, (void *)0x21)  // PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, drop_target_x2, (void *)0x22)  // PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, drop_target_x2, (void *)0x23)  // PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, drop_target_x2, (void *)0x24)  // PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, drop_target_x2, (void *)0x25)  // PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, drop_target_x2, (void *)0x26)  // PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, drop_target_x2, (void *)0x27)  // PORT_CODE(KEYCODE_A)

	PORT_MODIFY("X3")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, saucer_x3, (void *)0x37)   // PORT_CODE(KEYCODE_Q)
INPUT_PORTS_END

static INPUT_PORTS_START( pwerplay )
	PORT_INCLUDE( by17 )

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x60, 0x60, "Award for Beating Highest Score")    PORT_DIPLOCATION("SW0:!6,!7")
	PORT_DIPSETTING(    0x00, "Nothing")
	PORT_DIPSETTING(    0x20, "1 Credit")
	PORT_DIPSETTING(    0x40, "2 Credits")
	PORT_DIPSETTING(    0x60, "3 Credits")

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x20, 0x00, "Drop Target Award Sequence")         PORT_DIPLOCATION("SW1:!6")
	PORT_DIPSETTING(    0x00, "5X then Extra Ball")
	PORT_DIPSETTING(    0x20, "5X and Extra Ball")
	PORT_DIPNAME( 0x40, 0x00, "Rollover Button Score")              PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(    0x00, "Conservative - 100 Points")
	PORT_DIPSETTING(    0x40, "Liberal - 1,000 Points")

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x20, 0x00, "Drop Target Bank Reset")             PORT_DIPLOCATION("SW2:!6")
	PORT_DIPSETTING(    0x00, "Reset Both Banks")
	PORT_DIPSETTING(    0x20, "Reset Completed Bank Only")
	PORT_DIPNAME( 0x40, 0x00, "Pop Bumper Scores")                  PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(    0x00, "Alternate 1,000 Points Top and Bottom")
	PORT_DIPSETTING(    0x40, "All score 1,000 Points When Lit")

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x30, 0x20, "Top Saucer Specials Feature")        PORT_DIPLOCATION("SW3:!5,!6")
	PORT_DIPSETTING(    0x00, "Outlane Specials do Not Light")
//  PORT_DIPSETTING(    0x10, "")
	PORT_DIPSETTING(    0x20, "Outlane Specials Alternate")
	PORT_DIPSETTING(    0x30, "Outlane Specials Both Light")
	PORT_DIPNAME( 0xc0, 0xc0, "Award Mode")                         PORT_DIPLOCATION("SW3:!7,!8")
	PORT_DIPSETTING(    0x00, "Novelty / 50,000")
//  PORT_DIPSETTING(    0x40, "")
	PORT_DIPSETTING(    0x80, "Extra Ball / 50,000")
	PORT_DIPSETTING(    0xc0, "Extra Ball / Replay")

	PORT_MODIFY("X2")   /* Drop Target switches */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, drop_target_x2, (void *)0x20)  // PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, drop_target_x2, (void *)0x21)  // PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, drop_target_x2, (void *)0x22)  // PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, drop_target_x2, (void *)0x23)  // PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, drop_target_x2, (void *)0x24)  // PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, drop_target_x2, (void *)0x25)  // PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, drop_target_x2, (void *)0x26)  // PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, drop_target_x2, (void *)0x27)  // PORT_CODE(KEYCODE_A)

	PORT_MODIFY("X3")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, by17_state, saucer_x3, (void *)0x37)   // PORT_CODE(KEYCODE_Q)
INPUT_PORTS_END


CUSTOM_INPUT_MEMBER( by17_state::outhole_x0 )
{
	int bit_shift = ((FPTR)param & 0x07);
	int port = (((FPTR)param >> 4) & 0x07);

	/* Here we simulate the ball sitting in the Outhole so the Outhole Solenoid can release it */

	if (machine().input().code_pressed_once(KEYCODE_BACKSPACE))
		m_io_hold_x[port] |= (1 << bit_shift);

	return ((m_io_hold_x[port] >> bit_shift) & 1);
}

CUSTOM_INPUT_MEMBER( by17_state::saucer_x3 )
{
	int bit_shift = ((FPTR)param & 0x07);
	int port = (((FPTR)param >> 4) & 0x07);

	/* Here we simulate the ball sitting in a Saucer so the Saucer Solenoid can release it */

	if (machine().input().code_pressed_once(KEYCODE_Q))
		m_io_hold_x[port] |= (1 << bit_shift);

	return ((m_io_hold_x[port] >> bit_shift) & 1);
}


CUSTOM_INPUT_MEMBER( by17_state::drop_target_x2 )
{
	/* Here we simulate fallen Drop Targets so the Drop Target Reset Solenoids can release the switches */

	int bit_shift = ((FPTR)param & 0x07);
	int port = (((FPTR)param >> 4) & 0x07);

	switch (bit_shift)
	{
		case 0: if (machine().input().code_pressed_once(KEYCODE_K))
						m_io_hold_x[port] |= (1 << bit_shift);
					break;
		case 1: if (machine().input().code_pressed_once(KEYCODE_J))
						m_io_hold_x[port] |= (1 << bit_shift);
					break;
		case 2: if (machine().input().code_pressed_once(KEYCODE_H))
						m_io_hold_x[port] |= (1 << bit_shift);
					break;
		case 3: if (machine().input().code_pressed_once(KEYCODE_G))
						m_io_hold_x[port] |= (1 << bit_shift);
					break;
		case 4: if (machine().input().code_pressed_once(KEYCODE_F))
						m_io_hold_x[port] |= (1 << bit_shift);
					break;
		case 5: if (machine().input().code_pressed_once(KEYCODE_D))
						m_io_hold_x[port] |= (1 << bit_shift);
					break;
		case 6: if (machine().input().code_pressed_once(KEYCODE_S))
						m_io_hold_x[port] |= (1 << bit_shift);
					break;
		case 7: if (machine().input().code_pressed_once(KEYCODE_A))
						m_io_hold_x[port] |= (1 << bit_shift);
					break;
	}
	return ((m_io_hold_x[port] >> bit_shift) & 1);
}


READ8_MEMBER(by17_state::nibble_nvram_r)
{
	return (m_nvram[offset] | 0x0f);
}

WRITE8_MEMBER(by17_state::nibble_nvram_w)
{
	m_nvram[offset] = (data | 0x0f);
}

INPUT_CHANGED_MEMBER( by17_state::activity_button )
{
	if (newval != oldval)
		m_maincpu->set_input_line(INPUT_LINE_NMI, (newval ? ASSERT_LINE : CLEAR_LINE));
}

INPUT_CHANGED_MEMBER( by17_state::self_test )
{
	m_pia_u10->ca1_w(newval);
}

READ_LINE_MEMBER( by17_state::u10_ca1_r )
{
	return m_io_test->read() & 0x01;
}
READ_LINE_MEMBER( by17_state::u10_cb1_r )
{
	return m_u10_cb1;
}

WRITE_LINE_MEMBER( by17_state::u10_ca2_w )
{
#if 0                   // Display Blanking - Out of sync with video redraw rate and causes flicker so it's disabled
	if (state == 0)
	{
		int digit;

		for (digit=1; digit<=8; digit++)
		{
			output_set_digit_value(10+digit, 0);
			output_set_digit_value(20+digit, 0);
			output_set_digit_value(30+digit, 0);
			output_set_digit_value(40+digit, 0);
			output_set_digit_value(50+digit, 0);
		}
	}
#endif

	m_u10_ca2 = state;
}

WRITE_LINE_MEMBER( by17_state::u10_cb2_w )
{
//  logerror("New U10 CB2 state %01x, was %01x.   PIA=%02x\n", state, m_u10_cb2, m_u10a);

	if (state == TRUE)
		m_lamp_decode = m_u10a & 0x0f;

	m_u10_cb2 = state;
}

WRITE_LINE_MEMBER( by17_state::u11_ca2_w )
{
	output_set_value("led0", state);
}

READ_LINE_MEMBER( by17_state::u11_ca1_r )
{
	return m_u11_ca1;
}

READ_LINE_MEMBER( by17_state::u11_cb1_r )
{
	/* Pin 32 on MPU J5 AID connector tied low */
	return 0;
}

WRITE_LINE_MEMBER( by17_state::u11_cb2_w )
{
	m_u11_cb2 = state;
}

READ8_MEMBER( by17_state::u10_a_r )
{
	return m_u10a;
}

WRITE8_MEMBER( by17_state::u10_a_w )
{
//  logerror("Writing %02x to U10 PIA, CB2 state is %01x,  CA2 state is %01x, Lamp_Dec is %02x\n",data, m_u10_cb2, m_u10_ca2, (m_lamp_decode & 0x0f));

	if (!m_u10_ca2)
	{
		if (BIT(data, 0)==0)            // Display 1
			m_segment[1] = data>>4;
		else
		if (BIT(data, 1)==0)            // Display 2
			m_segment[2] = data>>4;
		else
		if (BIT(data, 2)==0)            // Display 3
			m_segment[3] = data>>4;
		else
		if (BIT(data, 3)==0)            // Display 4
			m_segment[4] = data>>4;
	}

	/*** Update the Lamp latched outputs ***/
	if ((data & 0x0f) == 0x0f)
	{
		if ((m_lamp_decode & 0x0f) < 0x0f)
		{
			if (output_get_indexed_value("lamp", ((m_lamp_decode & 0x0f)+00) ) ==0 ) output_set_indexed_value("lamp", ((m_lamp_decode & 0x0f)+00), ((data & 0x10) ? FALSE : TRUE));
			if (output_get_indexed_value("lamp", ((m_lamp_decode & 0x0f)+15) ) ==0 ) output_set_indexed_value("lamp", ((m_lamp_decode & 0x0f)+15), ((data & 0x20) ? FALSE : TRUE));
			if (output_get_indexed_value("lamp", ((m_lamp_decode & 0x0f)+30) ) ==0 ) output_set_indexed_value("lamp", ((m_lamp_decode & 0x0f)+30), ((data & 0x40) ? FALSE : TRUE));
			if (output_get_indexed_value("lamp", ((m_lamp_decode & 0x0f)+45) ) ==0 ) output_set_indexed_value("lamp", ((m_lamp_decode & 0x0f)+45), ((data & 0x80) ? FALSE : TRUE));
		}
		else
		{
			// Rest output - all lamps are off
		}
	}

	m_u10a = data;
}

READ8_MEMBER( by17_state::u10_b_r )
{
	UINT8 data = 0;

	if (BIT(m_u10a, 0))
		data |= m_io_x0->read();

	if (BIT(m_u10a, 1))
		data |= m_io_x1->read();

	if (BIT(m_u10a, 2))
		data |= m_io_x2->read();

	if (BIT(m_u10a, 3))
		data |= m_io_x3->read();

	if (BIT(m_u10a, 4))
		data |= m_io_x4->read();

	if (BIT(m_u10a, 5))
		data |= m_io_dsw0->read();

	if (BIT(m_u10a, 6))
		data |= m_io_dsw1->read();

	if (BIT(m_u10a, 7))
		data |= m_io_dsw2->read();

	if (m_u10_cb2)
		data |= m_io_dsw3->read();

	return data;
}

WRITE8_MEMBER( by17_state::u10_b_w )
{
	m_u10b = data;
}

READ8_MEMBER( by17_state::u11_a_r )
{
	return m_u11a;
}

WRITE8_MEMBER( by17_state::u11_a_w )
{
	if (BIT(data, 0)==0)            // Display Credit/Ball
	{
		m_segment[5] = m_u10a>>4;
	}


	m_digit = 0;

	if BIT(data, 7)
		m_digit = 1;
	else
	if BIT(data, 6)
		m_digit = 2;
	else
	if BIT(data, 5)
		m_digit = 3;
	else
	if BIT(data, 4)
		m_digit = 4;
	else
	if BIT(data, 3)
		m_digit = 5;
	else
	if BIT(data, 2)
		m_digit = 6;
	else
	if (BIT(data, 2) && BIT(data, 3))   // Aftermarket 7th digit strobe for 6 digit games
		m_digit = 7;

	if ((m_u10_ca2==0) && m_digit)
	{
		static const UINT8 patterns[16] = { 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0,0,0,0,0,0 }; // MC14543 - BCD to 7 Segment Display Decoder

		output_set_digit_value(10+m_digit, patterns[m_segment[1]]);
		output_set_digit_value(20+m_digit, patterns[m_segment[2]]);
		output_set_digit_value(30+m_digit, patterns[m_segment[3]]);
		output_set_digit_value(40+m_digit, patterns[m_segment[4]]);
		output_set_digit_value(50+m_digit, patterns[m_segment[5]]);

	}

	m_u11a = data;
}

WRITE8_MEMBER( by17_state::u11_b_w )
{
	if (!m_u11_cb2)
	{
		if ((data & 0x0f) < 0x0f)   // Momentary Solenoids
		{
			output_set_indexed_value( "solenoid", (data & 0x0f), TRUE);

			if (m_solenoid_features[(data & 0x0f)][3])  // Reset/release relevant switch after firing Solenoid
				m_io_hold_x[(m_solenoid_features[(data & 0x0f)][2])] &= (m_solenoid_features[(data & 0x0f)][3]);

			if (m_solenoid_features[(data & 0x0f)][0] != 0xff)  // Play solenoid audio sample
				m_samples->start(m_solenoid_features[(data & 0x0f)][0], m_solenoid_features[(data & 0x0f)][1]);
		}
		else                        // Rest output - all momentary solenoids are off
		{
			for (int i=0; i<15; i++)
			{
				output_set_indexed_value( "solenoid", i, FALSE);
			}
		}
	}


	if ((m_u11b & 0x10) && ((data & 0x10)==0))
	{
		output_set_value("solenoid16", TRUE);
		if (m_solenoid_features[16][0] != 0xff)
			m_samples->start(m_solenoid_features[16][0], m_solenoid_features[16][1]);
	}
	else if ((data & 0x10) && ((m_u11b & 0x10)==0))
	{
		output_set_value("solenoid16", FALSE);
		if (m_solenoid_features[16][0] != 0xff)
			m_samples->start(m_solenoid_features[16][0], m_solenoid_features[16][2]);
	}
	if ((m_u11b & 0x20) && ((data & 0x20)==0))
	{
		output_set_value("solenoid17", TRUE);                   // Coin Lockout Coil engage
		if (m_solenoid_features[17][0] != 0xff)
			m_samples->start(m_solenoid_features[17][0], m_solenoid_features[17][1]);
	}
	else if ((data & 0x20) && ((m_u11b & 0x20)==0))
	{
		output_set_value("solenoid17", FALSE);                  // Coin Lockout Coil release
		if (m_solenoid_features[17][0] != 0xff)
			m_samples->start(m_solenoid_features[17][0], m_solenoid_features[17][2]);
	}
	if ((m_u11b & 0x40) && ((data & 0x40)==0))
	{
		output_set_value("solenoid18", TRUE);                   // Flipper Enable Relay engage
		if (m_solenoid_features[18][0] != 0xff)
			m_samples->start(m_solenoid_features[18][0], m_solenoid_features[18][1]);
	}
	else if ((data & 0x40) && ((m_u11b & 0x40)==0))
	{
		output_set_value("solenoid18", FALSE);                  // Flipper Enable Relay release
		if (m_solenoid_features[18][0] != 0xff)
			m_samples->start(m_solenoid_features[18][0], m_solenoid_features[18][2]);
	}
	if ((m_u11b & 0x80) && ((data & 0x80)==0))
	{
		output_set_value("solenoid19", TRUE);
		if (m_solenoid_features[19][0] != 0xff)
			m_samples->start(m_solenoid_features[19][0], m_solenoid_features[19][1]);
	}
	else if ((data & 0x80) && ((m_u11b & 0x80)==0))
	{
		output_set_value("solenoid19", FALSE);
		if (m_solenoid_features[19][0] != 0xff)
			m_samples->start(m_solenoid_features[19][0], m_solenoid_features[19][2]);
	}

	m_u11b = data;
}


// zero-cross detection
TIMER_DEVICE_CALLBACK_MEMBER( by17_state::timer_z_freq )
{
/*  Zero Crossing Detector - this timing is based on 50Hz AC line power input converted to unregulated DC

    -+                          +---+
     |                          |   |
     |<-------- 9.30ms -------->|<->|700us
     |                          |   |
     +--------------------------+   +-----
*/

	timer_device *zero_crossing_active_timer = machine().device<timer_device>("timer_z_pulse");

	zero_crossing_active_timer->adjust(attotime::from_usec(700));

	m_u10_cb1 = true;
	m_pia_u10->cb1_w(m_u10_cb1);

	/*** Zero Crossing - power to all Lamp SCRs is cut off and reset ***/

	for (int i=0; i<60; i++)
	{
		output_set_indexed_value( "lamp", i, 0 );
	}

}
TIMER_DEVICE_CALLBACK_MEMBER( by17_state::timer_z_pulse )
{
	/*** Line Power to DC Zero Crossing has ended ***/

	m_u10_cb1 = false;
	m_pia_u10->cb1_w(m_u10_cb1);
}

// 555 timer for display refresh
TIMER_DEVICE_CALLBACK_MEMBER( by17_state::u11_timer )
{
/*   +--------------------------+   +-----
     |                          |   |
     |<-------- 2.85ms -------->|<->|300us
     |                          |   |
    -+                          +---+
*/

	timer_device *display_refresh_timer = machine().device<timer_device>("timer_d_pulse");

	display_refresh_timer->adjust(attotime::from_msec(2.85));

	m_u11_ca1 = true;
	m_pia_u11->ca1_w(m_u11_ca1);
}

TIMER_DEVICE_CALLBACK_MEMBER( by17_state::timer_d_pulse )
{
	m_u11_ca1 = false;
	m_pia_u11->ca1_w(m_u11_ca1);
}



DRIVER_INIT_MEMBER( by17_state, by17 )
{
	static const UINT8 solenoid_features_default[20][4] =
	{
	// This table serves two functions and is configured on a per game basis:
	// Assign a particular sound sample corresponding to a solenoid function, and
	// release any switches being held closed eg. drop targets, ball in saucer/outhole, etc

	//  { Sound Channel, Sound Sample, Switch Strobe, Switch Return Mask }
	/*00*/  { 0x00, 0x00,  0x00, 0x00 },
	/*01*/  { 0x05, 0x01,  0x00, 0x00 },        // Chime 10
	/*02*/  { 0x05, 0x02,  0x00, 0x00 },        // Chime 100
	/*03*/  { 0x05, 0x03,  0x00, 0x00 },        // Chime 1000
	/*04*/  { 0x05, 0x04,  0x00, 0x00 },        // Chime 10000
	/*05*/  { 0x04, 0x00,  0x00, 0x00 },        // Knocker
	/*06*/  { 0x01, 0x09,  0x00, 0x7f },        // Outhole
	/*07*/  { 0x00, 0x00,  0x00, 0x00 },
	/*08*/  { 0x02, 0x00,  0x00, 0x00 },
	/*09*/  { 0x02, 0x00,  0x00, 0x00 },
	/*10*/  { 0x02, 0x00,  0x00, 0x00 },
	/*11*/  { 0x02, 0x00,  0x00, 0x00 },
	/*12*/  { 0x00, 0x00,  0x00, 0x00 },
	/*13*/  { 0x02, 0x00,  0x00, 0x00 },
	/*14*/  { 0x00, 0x00,  0x00, 0x00 },
	/*15*/  { 0xff, 0xff,  0x00, 0x00 },        // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0xff, 0xff, 0xff,  0x00 },
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },        // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },        // Flipper Enable relay
	/*19*/  { 0xff, 0xff, 0xff,  0x00 }
	};

	for (int i=0; i<20; i++)
	{
		for (int j=0; j<4; j++)
			m_solenoid_features[i][j] = solenoid_features_default[i][j];
	}
}


DRIVER_INIT_MEMBER( by17_state, matahari )
{
	static const UINT8 solenoid_features_matahari[20][4] =
	{
	//  { Sound Channel, Sound Sample, Switch Strobe, Switch Return Mask }
	/*00*/  { 0x02, 0x05,  0x03, 0x7f },        // Saucer
	/*01*/  { 0x05, 0x01,  0x00, 0x00 },        // Chime 10
	/*02*/  { 0x05, 0x02,  0x00, 0x00 },        // Chime 100
	/*03*/  { 0x05, 0x03,  0x00, 0x00 },        // Chime 1000
	/*04*/  { 0x05, 0x04,  0x00, 0x00 },        // Chime 10000
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },        // Knocker
	/*06*/  { 0x01, 0x09,  0x00, 0x7f },        // Outhole
	/*07*/  { 0x02, 0x00,  0x00, 0x00 },        // Pop Bumper Left Bottom
	/*08*/  { 0x02, 0x00,  0x00, 0x00 },        // Pop Bumper Left Top
	/*09*/  { 0x02, 0x00,  0x00, 0x00 },        // Pop Bumper Right Top
	/*10*/  { 0x02, 0x00,  0x00, 0x00 },        // Pop Bumper Right Bottom
	/*11*/  { 0x02, 0x07,  0x00, 0x00 },        // Slingshot Left
	/*12*/  { 0x00, 0x0b,  0x02, 0x0f },        // Drop Target Reset Left
	/*13*/  { 0x02, 0x07,  0x00, 0x00 },        // Slingshot Right
	/*14*/  { 0x03, 0x0b,  0x02, 0xf0 },        // Drop Target Reset Right
	/*15*/  { 0xff, 0xff,  0x00, 0x00 },        // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0xff, 0xff, 0xff,  0x00 },
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },        // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },        // Flipper Enable relay
	/*19*/  { 0xff, 0xff, 0xff,  0x00 }
	};

	for (int i=0; i<20; i++)
	{
		for (int j=0; j<4; j++)
			m_solenoid_features[i][j] = solenoid_features_matahari[i][j];
	}
}


DRIVER_INIT_MEMBER( by17_state, pwerplay )
{
	static const UINT8 solenoid_features_pwerplay[20][4] =
	{
	//  { Sound Channel, Sound Sample, Switch Strobe, Switch Return Mask }
	/*00*/  { 0x00, 0x10,  0x00, 0x00 },        // Post Down
	/*01*/  { 0x05, 0x01,  0x00, 0x00 },        // Chime 10
	/*02*/  { 0x05, 0x02,  0x00, 0x00 },        // Chime 100
	/*03*/  { 0x05, 0x03,  0x00, 0x00 },        // Chime 1000
	/*04*/  { 0x05, 0x04,  0x00, 0x00 },        // Chime 10000
	/*05*/  { 0x04, 0x06,  0x00, 0x00 },        // Knocker
	/*06*/  { 0x01, 0x09,  0x00, 0x7f },        // Outhole
	/*07*/  { 0x02, 0x05,  0x03, 0x7f },        // Saucer
	/*08*/  { 0x02, 0x00,  0x00, 0x00 },        // Pop Bumper Left
	/*09*/  { 0x02, 0x00,  0x00, 0x00 },        // Pop Bumper Right
	/*10*/  { 0x02, 0x00,  0x00, 0x00 },        // Pop Bumper Bottom
	/*11*/  { 0x02, 0x07,  0x00, 0x00 },        // Slingshot Left
	/*12*/  { 0x00, 0x0b,  0x02, 0x0f },        // Drop Target Reset Left
	/*13*/  { 0x02, 0x07,  0x00, 0x00 },        // Slingshot Right
	/*14*/  { 0x03, 0x0b,  0x02, 0xf0 },        // Drop Target Reset Right
	/*15*/  { 0xff, 0xff,  0x00, 0x00 },        // None - all momentary solenoids off
	//  { Sound Channel, Sound engage, Sound release, Not Used }
	/*16*/  { 0x00, 0x11, 0x0f,  0x00 },        // Post Up
	/*17*/  { 0x00, 0x0c, 0x0d,  0x00 },        // Coin Lockout coil
	/*18*/  { 0x00, 0x0e, 0x0f,  0x00 },        // Flipper Enable relay
	/*19*/  { 0xff, 0xff, 0xff,  0x00 }
	};


	for (int i=0; i<20; i++)
	{
		for (int j=0; j<4; j++)
			m_solenoid_features[i][j] = solenoid_features_pwerplay[i][j];
	}
}



MACHINE_RESET_MEMBER( by17_state, by17 )
{
	render_target *target = machine().render().first_target();

	target->set_view(0);

	m_u10a = 0;
	m_u10b = 0;
	m_u11a = 0;
	m_u11b = 0;
	m_lamp_decode = 0x0f;
	m_io_hold_x[0] = 0x80;  // Put ball in Outhole on startup
	m_io_hold_x[1] = m_io_hold_x[2] = m_io_hold_x[3] = m_io_hold_x[4] = 0;
}



static MACHINE_CONFIG_START( by17, by17_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800, 530000)  // No xtal, just 2 chips forming a multivibrator oscillator around 530KHz
	MCFG_CPU_PROGRAM_MAP(by17_map)

	MCFG_MACHINE_RESET_OVERRIDE( by17_state, by17 )

	MCFG_NVRAM_ADD_0FILL("nvram")   // 'F' filled causes Credit Display to be blank on first startup

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_by17)

	/* Sound */
	MCFG_FRAGMENT_ADD( genpin_audio )

	/* Devices */
	MCFG_DEVICE_ADD("pia_u10", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(by17_state, u10_a_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(by17_state, u10_a_w))
	MCFG_PIA_READPB_HANDLER(READ8(by17_state, u10_b_r))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(by17_state, u10_b_w))
	MCFG_PIA_READCA1_HANDLER(READLINE(by17_state, u10_ca1_r))
	MCFG_PIA_READCB1_HANDLER(READLINE(by17_state, u10_cb1_r))
	MCFG_PIA_CA2_HANDLER(WRITELINE(by17_state, u10_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(by17_state, u10_cb2_w))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("maincpu", m6800_cpu_device, irq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("maincpu", m6800_cpu_device, irq_line))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("timer_z_freq", by17_state, timer_z_freq, attotime::from_hz(100)) // Mains Line Frequency * 2
	MCFG_TIMER_DRIVER_ADD("timer_z_pulse", by17_state, timer_z_pulse)                                // Active pulse length from Zero Crossing detector

	MCFG_DEVICE_ADD("pia_u11", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(by17_state, u11_a_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(by17_state, u11_a_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(by17_state, u11_b_w))
	MCFG_PIA_READCA1_HANDLER(READLINE(by17_state, u11_ca1_r))
	MCFG_PIA_READCB1_HANDLER(READLINE(by17_state, u11_cb1_r))
	MCFG_PIA_CA2_HANDLER(WRITELINE(by17_state, u11_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(by17_state, u11_cb2_w))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("maincpu", m6800_cpu_device, irq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("maincpu", m6800_cpu_device, irq_line))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("timer_d_freq", by17_state, u11_timer, attotime::from_hz(317)) // 555 timer
	MCFG_TIMER_DRIVER_ADD("timer_d_pulse", by17_state, timer_d_pulse)                             // 555 Active pulse length
MACHINE_CONFIG_END




/*------------------------------------------------------------------
/ Bow and Arrow #1033 (prototype only, slightly different hardware)
/ not sure yet if it belongs in this driver
/-------------------------------------------------------------------*/
ROM_START(bowarrow)
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD("b14.bin", 0x0400, 0x0200, CRC(d4d0f92a) SHA1(b996cbe9762fafd64115dc78e24626cf08f8abf7))
	ROM_LOAD("b16.bin", 0x0600, 0x0200, CRC(ad2102e7) SHA1(86887beea5e03e80f60c947d6d71431e5eab3d1b))
	ROM_LOAD("b18.bin", 0x0800, 0x0200, CRC(5d84656b) SHA1(d17350f5a0cc0cd00b60df4903034489dce7ade5))
	ROM_LOAD("b1a.bin", 0x0a00, 0x0200, CRC(6f083ce6) SHA1(624b00e72e223c6b9fbf38b831200c9a7aa0d8f7))
	ROM_LOAD("b1c.bin", 0x0c00, 0x0200, CRC(6ed4d39e) SHA1(1f6c57c7274c76246dd2f0b70ec459857a5cf1eb))
	ROM_LOAD("b1e.bin", 0x0e00, 0x0200, CRC(ff2f97de) SHA1(28a8fdeccb1382d3a1153c97466426459c9fa075))
ROM_END

/*--------------------------------
/ Freedom #1066
/-------------------------------*/
ROM_START(freedom)
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD( "720-08_1.474", 0x0400, 0x0200, CRC(b78bceeb) SHA1(acf6f1a497ada344211f12dbf4be619bee559950))
	ROM_LOAD( "720-10_2.474", 0x0600, 0x0200, CRC(ca90c8a7) SHA1(d9b5e95247e846e50a2a43c85ad5eb1fc761ab67))
	ROM_LOAD( "720-07_6.716", 0x0800, 0x0800, CRC(0f4e8b83) SHA1(faa05dde24eb60be0cdc4456ae2e660a15ed85ac))
ROM_END

/*--------------------------------
/ Night Rider #1074
/-------------------------------*/
ROM_START(nightrdr)
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD( "721-21_1.716", 0x0000, 0x0800, CRC(237c4060) SHA1(4ce3dba9189fe7666fc76a2c8ee7fff9b12d4c00))
	ROM_LOAD( "720-20_6.716", 0x0800, 0x0800, CRC(f394e357) SHA1(73444f848825a398515153d18de027792b57bcc7))
ROM_END

ROM_START(nightr20)
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD( "721-21_1.716", 0x0000, 0x0800, CRC(237c4060) SHA1(4ce3dba9189fe7666fc76a2c8ee7fff9b12d4c00))
	ROM_LOAD( "720-20_6.716", 0x0800, 0x0800, CRC(0c17aa4d) SHA1(729e61a29691857112579efcdb96a35e8e5b1279)) // sldh
ROM_END

/*--------------------------------
/ Black Jack #1092
/-------------------------------*/
ROM_START(blackjck)
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD( "728-32_2.716", 0x0000, 0x0800, CRC(1333c9d1) SHA1(1fbb60d84db47ffaf7f291575b2705783a110678))
	ROM_LOAD( "720-20_6.716", 0x0800, 0x0800, CRC(0c17aa4d) SHA1(729e61a29691857112579efcdb96a35e8e5b1279))
ROM_END

/*--------------------------------
/ Evel Knievel #1094
/-------------------------------*/
ROM_START(evelknie)
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD( "722-17_2.716", 0x0000, 0x0800, CRC(b6d9a3fa) SHA1(1939e13f73a324e3d2fd269a54446f48cf530f50))
	ROM_LOAD( "720-20_6.716", 0x0800, 0x0800, CRC(0c17aa4d) SHA1(729e61a29691857112579efcdb96a35e8e5b1279))
ROM_END

/*--------------------------------
/ Mata Hari #1104
/-------------------------------*/
ROM_START(matahari)
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD( "725-21_2.716", 0x0000, 0x0800, CRC(63acd9b0) SHA1(2347342f1281c097ea39c79236d85b00a1dfc7b2))
	ROM_LOAD( "720-20_6.716", 0x0800, 0x0800, CRC(0c17aa4d) SHA1(729e61a29691857112579efcdb96a35e8e5b1279))
ROM_END

/*--------------------------------
/ Eight Ball #1118
/-------------------------------*/
ROM_START(eightbll)
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD( "723-20_2.716", 0x0000, 0x0800, CRC(33559e7b) SHA1(49008db95c8f012e7e3b613e6eee811512207fa9))
	ROM_LOAD( "720-20_6.716", 0x0800, 0x0800, CRC(0c17aa4d) SHA1(729e61a29691857112579efcdb96a35e8e5b1279))
ROM_END

/*--------------------------------
/ Power Play #1120
/-------------------------------*/
ROM_START(pwerplay)
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD( "724-25_2.716", 0x0000, 0x0800, CRC(43012f35) SHA1(f90d582e3394d949a637a09882ffdad7664c44c0))
	ROM_LOAD( "720-20_6.716", 0x0800, 0x0800, CRC(0c17aa4d) SHA1(729e61a29691857112579efcdb96a35e8e5b1279))
ROM_END

/*--------------------------------
/ Strikes and Spares #1135
/-------------------------------*/
ROM_START(stk_sprs)
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD( "740-16_2.716", 0x0000, 0x0800, CRC(2be27024) SHA1(266dee3a5c4c115acc20543df2eb172f1e85dacb))
	ROM_LOAD( "720-20_6.716", 0x0800, 0x0800, CRC(0c17aa4d) SHA1(729e61a29691857112579efcdb96a35e8e5b1279))
ROM_END

/*--------------------------------------------------------------
/ Stellar Airship / Geiger-Automatenbau GMBH, of Germany (1981)
/---------------------------------------------------------------*/


GAME(  1976, bowarrow, 0,        by17, by17,     by17_state, by17,     ROT0, "Bally", "Bow & Arrow (Prototype)", MACHINE_IS_SKELETON_MECHANICAL)
GAME(  1977, freedom,  0,        by17, by17,     by17_state, by17,     ROT0, "Bally", "Freedom", MACHINE_IS_SKELETON_MECHANICAL)
GAME(  1977, nightrdr, 0,        by17, by17,     by17_state, by17,     ROT0, "Bally", "Night Rider (rev. 21)", MACHINE_IS_SKELETON_MECHANICAL)
GAME(  1977, nightr20, nightrdr, by17, by17,     by17_state, by17,     ROT0, "Bally", "Night Rider (rev. 20)", MACHINE_IS_SKELETON_MECHANICAL)
GAME(  1978, blackjck, 0,        by17, by17,     by17_state, by17,     ROT0, "Bally", "Black Jack (Pinball)", MACHINE_IS_SKELETON_MECHANICAL)
GAME(  1977, evelknie, 0,        by17, by17,     by17_state, by17,     ROT0, "Bally", "Evel Knievel", MACHINE_IS_SKELETON_MECHANICAL)
GAMEL( 1978, matahari, 0,        by17, matahari, by17_state, matahari, ROT0, "Bally", "Mata Hari", MACHINE_MECHANICAL, layout_by17_matahari)
GAME(  1977, eightbll, 0,        by17, by17,     by17_state, by17,     ROT0, "Bally", "Eight Ball", MACHINE_IS_SKELETON_MECHANICAL)
GAMEL( 1978, pwerplay, 0,        by17, pwerplay, by17_state, pwerplay, ROT0, "Bally", "Power Play (Pinball)", MACHINE_MECHANICAL, layout_by17_pwerplay)
GAME(  1978, stk_sprs, 0,        by17, by17,     by17_state, by17,     ROT0, "Bally", "Strikes and Spares", MACHINE_IS_SKELETON_MECHANICAL)
