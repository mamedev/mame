// license:BSD-3-Clause
// copyright-holders:Robbbert
/**************************************************************

PINBALL
Bally MPU AS-2518-133
A blend of arcade video game and pinball. It has the shape of
 an upright arcade game. The vertical part contains the video
 screen, while the flat part is a small pinball machine. The
 ball is injected from between the flippers; there's no shooter.
 There's flipper buttons on the sides, and a joystick on the
 control panel.
Granny has an additional "Power" button, it's a gun to shoot
 the alligators, if you have collected ammo. You need money to
 pass the toll gates.

List of BY133 games:

Game                              NUM         Sound Board
-----------------------------------------------------------------------------------------------
Baby Pacman (1982)               1299         The vidiot board contains the cheap squeak circuit
Grand Slam (1983)                1311         AS-2518-51 (in by35.cpp)
Gold Ball (1983)                 1314 (0371)  A084-91495-A371/A360,AS-2518-51 (in by35.cpp)
Granny & the Gators (1984)       1369 (0369)  Cheap Squeak
Big Bat (1984)                   ----         AS-2518-61 (in by35.cpp)

Babypac uses a MPU4 board containing the main cpu, and a Vidiot
board containing the video and sound cpus, and the video controller
and the sound DAC and amp.

Granny uses the MPU4 board, but it has a Vidiot Deluxe for the
video, and a Cheap Squeak sound board. The manual incorrectly
describes the babypac vidiot board. Unable to locate a schematic.

Status:
- Both machines are playable.
- To exit the pinball back to the video:
    Babypac: press O or P;
    Granny: press X.

ToDo: (all)
- Beeper needs to be replaced by a red LED.

ToDo (granny):
- Video CPU clock needs verification.
- Screen blending needs improvement
- No schematic found.


***************************************************************/


#include "emu.h"
#include "machine/genpin.h"
#include "cpu/m6800/m6801.h"
#include "cpu/m6809/m6809.h"
#include "machine/6821pia.h"
#include "machine/timer.h"
#include "audio/bally.h"
#include "sound/beep.h"
#include "video/tms9928a.h"
#include "speaker.h"


namespace {

class by133_state : public genpin_class
{
public:
	by133_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_videocpu(*this, "videocpu")
		, m_cheap_squeak(*this, "cheap_squeak")
		, m_sound_select_handler(*this)
		, m_sound_int_handler(*this)
		, m_pia_u7(*this, "pia_u7")
		, m_pia_u10(*this, "pia_u10")
		, m_pia_u11(*this, "pia_u11")
		, m_crtc(*this, "crtc")
		, m_crtc2(*this, "crtc2")
		, m_beep(*this, "beeper")
		, m_io_test(*this, "TEST")
		, m_io_dsw0(*this, "DSW0")
		, m_io_dsw1(*this, "DSW1")
		, m_io_dsw2(*this, "DSW2")
		, m_io_dsw3(*this, "DSW3")
		, m_io_joy(*this, "JOY")
		, m_io_x0(*this, "X0")
		, m_io_x1(*this, "X1")
		, m_io_x2(*this, "X2")
		, m_io_x3(*this, "X3")
		, m_io_x4(*this, "X4")
		, m_io_outputs(*this, "out%d", 0U)
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(video_test);
	DECLARE_INPUT_CHANGED_MEMBER(activity_test);
	DECLARE_INPUT_CHANGED_MEMBER(self_test);

	void by133(machine_config &config);
	void granny(machine_config &config);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	u8 m_mpu_to_vid = 0U;
	u8 m_vid_to_mpu = 0U;
	u8 m_u7a = 0U;
	u8 m_u7b = 0U;
	u8 m_u10a = 0U;
	u8 m_u10b = 0U;
	bool m_u10_cb2 = 0;
	bool m_u11_cb2 = 0;
	u8 m_u11a = 0U;
	u8 m_u11b = 0U;
	bool m_u10_timer = 0;
	bool m_u11_timer = 0;
	required_device<m6800_cpu_device> m_maincpu;
	required_device<mc6809_device> m_videocpu;
	required_device<bally_cheap_squeak_device> m_cheap_squeak;
	devcb_write8 m_sound_select_handler;
	devcb_write_line m_sound_int_handler;
	required_device<pia6821_device> m_pia_u7;
	required_device<pia6821_device> m_pia_u10;
	required_device<pia6821_device> m_pia_u11;
	required_device<tms9928a_device> m_crtc;
	optional_device<tms9928a_device> m_crtc2; // for Granny only
	required_device<beep_device> m_beep; // temp
	required_ioport m_io_test;
	required_ioport m_io_dsw0;
	required_ioport m_io_dsw1;
	required_ioport m_io_dsw2;
	required_ioport m_io_dsw3;
	required_ioport m_io_joy;
	required_ioport m_io_x0;
	required_ioport m_io_x1;
	required_ioport m_io_x2;
	required_ioport m_io_x3;
	required_ioport m_io_x4; // Granny
	output_finder<80> m_io_outputs; // 16 solenoids + 64 lamps

	u8 sound_data_r();
	void sound_data_w(u8 data);
	u8 u7_a_r();
	void u7_a_w(u8 data);
	u8 u7_b_r();
	void u7_b_w(u8 data);
	u8 u10_a_r();
	void u10_a_w(u8 data);
	u8 u10_b_r();
	void u10_b_w(u8 data);
	u8 u11_a_r();
	void u11_a_w(u8 data);
	u8 u11_b_r();
	void u11_b_w(u8 data);
	DECLARE_WRITE_LINE_MEMBER(u7_ca2_w);
	DECLARE_WRITE_LINE_MEMBER(u10_ca2_w);
	DECLARE_WRITE_LINE_MEMBER(u11_ca2_w);
	DECLARE_WRITE_LINE_MEMBER(u7_cb2_w);
	DECLARE_WRITE_LINE_MEMBER(u10_cb2_w);
	DECLARE_WRITE_LINE_MEMBER(u11_cb2_w);
	TIMER_DEVICE_CALLBACK_MEMBER(u10_timer);
	TIMER_DEVICE_CALLBACK_MEMBER(u11_timer);
	void granny_crtc_w(offs_t offset, u8 data);
	uint32_t screen_update_granny(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void granny_map(address_map &map);
	void main_map(address_map &map);
	void video_map(address_map &map);
};


void by133_state::main_map(address_map &map)
{ // U9 MPU
	map.global_mask(0x7fff);
	map(0x0000, 0x007f).ram(); // 128x8 in MC6810 U7 MPU
	map(0x0088, 0x008b).rw(m_pia_u10, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // PIA U10 MPU
	map(0x0090, 0x0093).rw(m_pia_u11, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // PIA U11 MPU
	map(0x0200, 0x03ff).ram().share("nvram"); // 256x4 in 5101L U8 MPU, battery backed (D4-7 are data, A4-8 are address)
	map(0x1000, 0x17ff).rom().region("maincpu", 0x0000);
	map(0x1800, 0x1fff).rom().region("maincpu", 0x1000);
	map(0x5000, 0x57ff).rom().region("maincpu", 0x0800);
	map(0x5800, 0x5fff).rom().region("maincpu", 0x1800);
	map(0x7000, 0x7fff).rom().region("maincpu", 0x1000);
}

void by133_state::video_map(address_map &map)
{ // U8 Vidiot
	map(0x0000, 0x1fff).rw(FUNC(by133_state::sound_data_r), FUNC(by133_state::sound_data_w));
	map(0x2000, 0x2003).mirror(0x0ffc).rw(m_pia_u7, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // PIA U7 Vidiot
	map(0x4000, 0x4001).mirror(0x0ffe).rw(m_crtc, FUNC(tms9928a_device::read), FUNC(tms9928a_device::write));
	map(0x6000, 0x63ff).mirror(0x1c00).ram();
	map(0x8000, 0xffff).rom();
}

void by133_state::granny_map(address_map &map)
{
	map(0x0000, 0x0001).rw(FUNC(by133_state::sound_data_r), FUNC(by133_state::sound_data_w));
	map(0x0002, 0x0003).rw(m_crtc, FUNC(tms9928a_device::read), FUNC(tms9928a_device::write));
	map(0x0004, 0x0005).rw(m_crtc2, FUNC(tms9928a_device::read), FUNC(tms9928a_device::write));
	map(0x0006, 0x0007).w(FUNC(by133_state::granny_crtc_w)); // can write to both at once
	map(0x0008, 0x000b).rw(m_pia_u7, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x2000, 0x27ff).ram();
	map(0x2801, 0x2801).nopr();
	map(0x4000, 0xffff).rom();
}


INPUT_CHANGED_MEMBER( by133_state::video_test )
{
	if(newval)
		m_videocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

INPUT_CHANGED_MEMBER( by133_state::activity_test )
{
	if(newval)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

INPUT_CHANGED_MEMBER( by133_state::self_test )
{
	m_pia_u10->ca1_w(newval);
}

static INPUT_PORTS_START( babypac )
	PORT_START("TEST")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Video Test") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, by133_state, video_test, 0)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Activity") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, by133_state, activity_test, 0)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Self Test") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, by133_state, self_test, 0)

	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x00, "S01") // S1-5: 32 combinations of coins/credits of a coin slot. S9-13 other slot.
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x01, DEF_STR( On ))
	PORT_DIPNAME( 0x02, 0x00, "S02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x02, DEF_STR( On ))
	PORT_DIPNAME( 0x04, 0x00, "S03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x04, DEF_STR( On ))
	PORT_DIPNAME( 0x08, 0x00, "S04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x08, DEF_STR( On ))
	PORT_DIPNAME( 0x10, 0x00, "S05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x10, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x20, "Remember centre arrows")
	PORT_DIPSETTING(    0x00, DEF_STR( No ))
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ))
	PORT_DIPNAME( 0x40, 0x40, "Cherry at start of game")
	PORT_DIPSETTING(    0x00, DEF_STR( No ))
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ))
	PORT_DIPNAME( 0x80, 0x80, "Side tunnel open at start of game")
	PORT_DIPSETTING(    0x00, DEF_STR( No ))
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ))

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "S09")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x01, DEF_STR( On ))
	PORT_DIPNAME( 0x02, 0x00, "S10")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x02, DEF_STR( On ))
	PORT_DIPNAME( 0x04, 0x00, "S11")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x04, DEF_STR( On ))
	PORT_DIPNAME( 0x08, 0x00, "S12")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x08, DEF_STR( On ))
	PORT_DIPNAME( 0x10, 0x00, "S13")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x10, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x00, "After 3 balls without score the ball is lost")
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ))
	PORT_DIPSETTING(    0x20, DEF_STR( No ))
	PORT_DIPNAME( 0x40, 0x40, "Remember energisers")
	PORT_DIPSETTING(    0x00, DEF_STR( No ))
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ))
	PORT_DIPNAME( 0x80, 0x00, "Disable video")
	PORT_DIPSETTING(    0x00, DEF_STR( No ))
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ))

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "S17")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x01, DEF_STR( On ))
	PORT_DIPNAME( 0x02, 0x00, "S18")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x02, DEF_STR( On ))
	PORT_DIPNAME( 0x04, 0x00, "S19")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x04, DEF_STR( On ))
	PORT_DIPNAME( 0x08, 0x00, "S20")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x08, DEF_STR( On ))
	PORT_DIPNAME( 0x10, 0x00, "S21")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x10, DEF_STR( On ))
	PORT_DIPNAME( 0x60, 0x00, "Special after x mazes")
	PORT_DIPSETTING(    0x00, "3") // also 0x40
	PORT_DIPSETTING(    0x20, "4")
	PORT_DIPSETTING(    0x60, "5")
	PORT_DIPNAME( 0x80, 0x80, "Remember centre arrows")
	PORT_DIPSETTING(    0x00, DEF_STR( No ))
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ))

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x00, "S25")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x01, DEF_STR( On ))
	PORT_DIPNAME( 0x02, 0x00, "S26")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x02, DEF_STR( On ))
	PORT_DIPNAME( 0x04, 0x04, "Credits displayed")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x04, DEF_STR( On ))
	PORT_DIPNAME( 0x08, 0x00, "S28")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x08, DEF_STR( On ))
	PORT_DIPNAME( 0x10, 0x00, "S29")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x10, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0xC0, 0x40, DEF_STR( Lives ))
	PORT_DIPSETTING(    0xC0, "2")
	PORT_DIPSETTING(    0x00, "3")
	PORT_DIPSETTING(    0x80, "4")
	PORT_DIPSETTING(    0x40, "5")

	PORT_START("JOY")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY

	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Right Flipper EOS") PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT( 0x0a, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Rebounds") PORT_CODE(KEYCODE_A) // press to start pinball game
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Right Spinner") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Left Spinner") PORT_CODE(KEYCODE_C)

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x3c, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_NAME("Slam Tilt")

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("R. Top Loop Lane") PORT_CODE(KEYCODE_D) // energiser 1
	PORT_BIT( 0x06, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("L. Top Loop Lane") PORT_CODE(KEYCODE_E) // energiser 4
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Tunnel Outlane") PORT_CODE(KEYCODE_F)   // tunnel speed
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Fruits Outlane") PORT_CODE(KEYCODE_G)   // increase fruit bonus
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("R. Inside Outlane") PORT_CODE(KEYCODE_H) // energiser 3
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("L. Inside Outlane") PORT_CODE(KEYCODE_I) // energiser 2

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("#5 Drop Target (R.)") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("#4 Drop Target") PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("#3 Drop Target") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("#2 Drop Target") PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("#1 Drop Target (L.)") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Outhole") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("R. Maze Saucer") PORT_CODE(KEYCODE_O) // return to video game
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("L. Maze Saucer") PORT_CODE(KEYCODE_P) // return to video game

	PORT_START("X4")
INPUT_PORTS_END

static INPUT_PORTS_START( granny )
	PORT_START("TEST")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Video Test") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, by133_state, video_test, 0)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Activity") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, by133_state, activity_test, 0)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Self Test") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, by133_state, self_test, 0)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Power")  // Also 2P start

	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x00, "S01") // S1-5: 32 combinations of coins/credits of a coin slot. S9-13 other slot.
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x01, DEF_STR( On ))
	PORT_DIPNAME( 0x02, 0x00, "S02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x02, DEF_STR( On ))
	PORT_DIPNAME( 0x04, 0x00, "S03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x04, DEF_STR( On ))
	PORT_DIPNAME( 0x08, 0x00, "S04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x08, DEF_STR( On ))
	PORT_DIPNAME( 0x10, 0x00, "S05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x10, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x20, "Hoop flashes centre rollover")
	PORT_DIPSETTING(    0x00, "Long")
	PORT_DIPSETTING(    0x20, "Short")
	PORT_DIPNAME( 0x40, 0x40, "S07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x40, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x80, "Centre rollover lights come on for next canoe")
	PORT_DIPSETTING(    0x00, DEF_STR( No ))
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ))

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "S09")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x01, DEF_STR( On ))
	PORT_DIPNAME( 0x02, 0x00, "S10")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x02, DEF_STR( On ))
	PORT_DIPNAME( 0x04, 0x00, "S11")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x04, DEF_STR( On ))
	PORT_DIPNAME( 0x08, 0x00, "S12")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x08, DEF_STR( On ))
	PORT_DIPNAME( 0x10, 0x00, "S13")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x10, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x00, "S14")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x40, "Extra canoe light will come on for next canoe")
	PORT_DIPSETTING(    0x00, DEF_STR( No ))
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ))
	PORT_DIPNAME( 0x80, 0x00, "S16")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "S17")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x01, DEF_STR( On ))
	PORT_DIPNAME( 0x02, 0x00, "S18")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x02, DEF_STR( On ))
	PORT_DIPNAME( 0x04, 0x00, "S19")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x04, DEF_STR( On ))
	PORT_DIPNAME( 0x08, 0x00, "S20")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x08, DEF_STR( On ))
	PORT_DIPNAME( 0x10, 0x00, "S21")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x10, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x00, "S22")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, "S23")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x40, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x80, "Exit to video light will come on for")
	PORT_DIPSETTING(    0x00, "paddle power")
	PORT_DIPSETTING(    0x80, "next canoe")

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x00, "S25")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x01, DEF_STR( On ))
	PORT_DIPNAME( 0x02, 0x00, "S26")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x02, DEF_STR( On ))
	PORT_DIPNAME( 0x04, 0x04, "Credits displayed")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x04, DEF_STR( On ))
	PORT_DIPNAME( 0x08, 0x00, "S28")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x08, DEF_STR( On ))
	PORT_DIPNAME( 0x10, 0x00, "S29")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x10, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0xC0, 0x40, "Canoes")
	PORT_DIPSETTING(    0xC0, "2")
	PORT_DIPSETTING(    0x00, "3")
	PORT_DIPSETTING(    0x80, "4")
	PORT_DIPSETTING(    0x40, "5")

	PORT_START("JOY")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START2 )

	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Canoe Rollover Button 1") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Canoe Rollover Button 2") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Canoe Rollover Button 3") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Canoe Rollover Button 4") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Canoe Rollover Button 5") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Canoe Rollover Button 6") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Canoe Rollover Button 7") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Canoe Rollover Button 8") PORT_CODE(KEYCODE_H)

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x3c, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_NAME("Slam Tilt")

	PORT_START("X2")
	PORT_BIT( 0x09, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Outhole") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Top R. Gate") PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("L. Rollover Buttons") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("R. Return Lane") PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("L. Return Lane") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Drop Target R") PORT_CODE(KEYCODE_M)

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Top Saucer") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("R. Lane Kickback") PORT_CODE(KEYCODE_O) // Gold
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Ammo Target O") PORT_CODE(KEYCODE_P)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("2nd Ammo Target M") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("1st Ammo Target M") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Ammo Target A") PORT_CODE(KEYCODE_S)

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Back Target T") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Back Target I") PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Back Target X") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Back Target E") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Drop Target P") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Drop Target O") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Drop Target W") PORT_CODE(KEYCODE_COMMA)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Drop Target E") PORT_CODE(KEYCODE_STOP)
INPUT_PORTS_END


void by133_state::granny_crtc_w(offs_t offset, u8 data)
{
	m_crtc->write(offset, data);
	m_crtc2->write(offset, data);
}

u8 by133_state::sound_data_r()
{
	return m_mpu_to_vid;
}

void by133_state::sound_data_w(u8 data)
{
	m_vid_to_mpu = data;
}

WRITE_LINE_MEMBER( by133_state::u7_ca2_w )
{
	// comms out
}

WRITE_LINE_MEMBER( by133_state::u10_ca2_w )
{
	// enable digital display
}

WRITE_LINE_MEMBER( by133_state::u11_ca2_w )
{
	// green led
}

WRITE_LINE_MEMBER( by133_state::u7_cb2_w )
{
	// red led
	m_beep->set_clock(950);
	m_beep->set_state(state);
	m_sound_int_handler(state);
}

WRITE_LINE_MEMBER( by133_state::u10_cb2_w )
{
	// lamp strobe #1
	m_u10_cb2 = state;
}

WRITE_LINE_MEMBER( by133_state::u11_cb2_w )
{
	// solenoid-sound selector

	m_u11_cb2 = state;
}

u8 by133_state::u7_a_r()
{
	return m_u7a;
}

void by133_state::u7_a_w(u8 data)
{
	m_u7a = data;
}

u8 by133_state::u7_b_r()
{
	if (BIT(m_u7a, 7)) // bits 6 and 7 work; pinmame uses 7
		m_u7b |= m_io_joy->read();

	if (BIT(m_u7a, 6)) // Granny has a "power" button - used to shoot your gun. Is also 2-player start.
		m_u7b = m_io_test->read() & 0x80;

	return m_u7b;
}

void by133_state::u7_b_w(u8 data)
{
	//machine().scheduler().synchronize();
	m_u7b = data;
	// Handle sound
	m_sound_select_handler(data & 0x1f);
}

u8 by133_state::u10_a_r()
{
	return m_u10a;
}

void by133_state::u10_a_w(u8 data)
{
	m_u10a = data;
	if (BIT(m_u11a, 2) == 0)
		m_mpu_to_vid = data ^ 0x0f;
	// Lamps
	if (!m_u10_cb2)
	{
		if (!BIT(m_u10a, 6))
			for (u8 i = 0; i < 16; i++)
				m_io_outputs[16+u8(m_u10_timer)*16+i] = ((m_u10a & 15) == i);
		if (!BIT(m_u10a, 7))
			for (u8 i = 0; i < 16; i++)
				m_io_outputs[48+u8(m_u10_timer)*16+i] = ((m_u10a & 15) == i);
	}
}

u8 by133_state::u10_b_r()
{
	if (BIT(m_u11a, 3) == 0)
		return ~m_u7a & 0x03;

	if (BIT(m_u11a, 1) == 0)
		return m_vid_to_mpu;

	u8 data = 0;

	if (BIT(m_u10a, 0))
		data |= m_io_x0->read();

	if (BIT(m_u10a, 1))
		data |= m_io_x1->read();

	if (BIT(m_u10a, 2))
		data |= m_io_x2->read();

	if (BIT(m_u10a, 3))
		data |= m_io_x3->read();

	if (BIT(m_u10a, 4))
		data |= m_io_x4->read(); // granny only

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

void by133_state::u10_b_w(u8 data)
{
	m_u10b = data;
}

u8 by133_state::u11_a_r()
{
	return m_u11a;
}

void by133_state::u11_a_w(u8 data)
{
	m_u11a = data;
	m_pia_u7->ca1_w(BIT(data, 1));
	m_pia_u7->ca2_w(BIT(data, 2));
}

u8 by133_state::u11_b_r()
{
	return m_u11b;
}

void by133_state::u11_b_w(u8 data)
{
	m_u11b = data;
	if (data == 0xB1)
		m_samples->start(5, 5); // outhole
	// Bits 0,1,2 go to 74LS138 to select one solenoid
	for (u8 i = 0; i < 8; i++)
		m_io_outputs[i] = ((data & 7) == i);
	// Bits 4,5,6 do a solenoid each
	for (u8 i = 4; i < 7; i++)
		m_io_outputs[i+8] = BIT(data, i);
	// Bits 3,7 not shown
}

// zero-cross detection
TIMER_DEVICE_CALLBACK_MEMBER( by133_state::u10_timer )
{
	m_u10_timer ^= 1;
	m_pia_u10->cb1_w(m_u10_timer);
}

// 555 timer for display refresh
TIMER_DEVICE_CALLBACK_MEMBER( by133_state::u11_timer )
{
	m_u11_timer ^= 1;
	m_pia_u11->ca1_w(m_u11_timer);
}

void by133_state::machine_start()
{
	genpin_class::machine_start();
	m_sound_select_handler.resolve();
	m_sound_int_handler.resolve();
	m_io_outputs.resolve();

	save_item(NAME(m_mpu_to_vid));
	save_item(NAME(m_vid_to_mpu));
	save_item(NAME(m_u7a));
	save_item(NAME(m_u7b));
	save_item(NAME(m_u10a));
	save_item(NAME(m_u10b));
	save_item(NAME(m_u10_cb2));
	save_item(NAME(m_u11_cb2));
	save_item(NAME(m_u11a));
	save_item(NAME(m_u11b));
	save_item(NAME(m_u10_timer));
	save_item(NAME(m_u11_timer));
}

void by133_state::machine_reset()
{
	genpin_class::machine_reset();
	for (u8 i = 0; i < m_io_outputs.size(); i++)
		m_io_outputs[i] = 0;

	m_u7a = 0;
	m_u7b = 1; // select mode 2 of mc6803 on /reset (not emulated yet by the cpu)
	m_u10a = 0;
	m_u10b = 0;
	m_u10_cb2 = 0;
	m_u11a = 0;
	m_u11b = 0;
	m_mpu_to_vid = 0;
	m_vid_to_mpu = 0;
	m_beep->set_state(0);
	m_u10_timer = false;
	m_u11_timer = false;
}

uint32_t by133_state::screen_update_granny(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	//bitmap.fill(0xff000000, cliprect);
	copybitmap(bitmap, m_crtc->get_bitmap(), 0, 0, 0, 0, cliprect);
	copybitmap_trans(bitmap, m_crtc2->get_bitmap(), 0, 0, 0, 0, cliprect, 0xff000000);
	return 0;
}

void by133_state::by133(machine_config &config)
{
	/* basic machine hardware */
	M6800(config, m_maincpu, XTAL(3'579'545)/4); // no xtal, just 2 chips
	m_maincpu->set_addrmap(AS_PROGRAM, &by133_state::main_map);

	MC6809(config, m_videocpu, XTAL(3'579'545));
	m_videocpu->set_addrmap(AS_PROGRAM, &by133_state::video_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	PIA6821(config, m_pia_u7, 0);
	m_pia_u7->readpa_handler().set(FUNC(by133_state::u7_a_r));
	m_pia_u7->writepa_handler().set(FUNC(by133_state::u7_a_w));
	m_pia_u7->readpb_handler().set(FUNC(by133_state::u7_b_r));
	m_pia_u7->writepb_handler().set(FUNC(by133_state::u7_b_w));
	m_pia_u7->ca2_handler().set(FUNC(by133_state::u7_ca2_w));
	m_pia_u7->cb2_handler().set(FUNC(by133_state::u7_cb2_w));
	m_pia_u7->irqa_handler().set_inputline("videocpu", M6809_FIRQ_LINE);
	m_pia_u7->irqa_handler().set_inputline("videocpu", M6809_FIRQ_LINE);

	PIA6821(config, m_pia_u10, 0);
	m_pia_u10->readpa_handler().set(FUNC(by133_state::u10_a_r));
	m_pia_u10->writepa_handler().set(FUNC(by133_state::u10_a_w));
	m_pia_u10->readpb_handler().set(FUNC(by133_state::u10_b_r));
	m_pia_u10->writepb_handler().set(FUNC(by133_state::u10_b_w));
	m_pia_u10->ca2_handler().set(FUNC(by133_state::u10_ca2_w));
	m_pia_u10->cb2_handler().set(FUNC(by133_state::u10_cb2_w));
	m_pia_u10->irqa_handler().set_inputline("maincpu", M6800_IRQ_LINE);
	m_pia_u10->irqb_handler().set_inputline("maincpu", M6800_IRQ_LINE);
	TIMER(config, "babypac1").configure_periodic(FUNC(by133_state::u10_timer), attotime::from_hz(120)); // mains freq*2

	PIA6821(config, m_pia_u11, 0);
	m_pia_u11->readpa_handler().set(FUNC(by133_state::u11_a_r));
	m_pia_u11->writepa_handler().set(FUNC(by133_state::u11_a_w));
	m_pia_u11->readpb_handler().set(FUNC(by133_state::u11_b_r));
	m_pia_u11->writepb_handler().set(FUNC(by133_state::u11_b_w));
	m_pia_u11->ca2_handler().set(FUNC(by133_state::u11_ca2_w));
	m_pia_u11->cb2_handler().set(FUNC(by133_state::u11_cb2_w));
	m_pia_u11->irqa_handler().set_inputline("maincpu", M6800_IRQ_LINE);
	m_pia_u11->irqb_handler().set_inputline("maincpu", M6800_IRQ_LINE);
	TIMER(config, "babypac2").configure_periodic(FUNC(by133_state::u11_timer), attotime::from_hz(634)); // 555 timer*2

	/* video hardware */
	TMS9928A(config, m_crtc, XTAL(10'738'635)).set_screen("screen");
	m_crtc->set_vram_size(0x4000);
	m_crtc->int_callback().set_inputline(m_videocpu, M6809_IRQ_LINE);
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	/* sound hardware */
	genpin_audio(config);
	SPEAKER(config, "mono").front_center();

	BALLY_CHEAP_SQUEAK(config, m_cheap_squeak);
	m_cheap_squeak->add_route(ALL_OUTPUTS, "mono", 1.00);
	m_sound_select_handler.bind().set(m_cheap_squeak, FUNC(bally_cheap_squeak_device::sound_select));
	m_sound_int_handler.bind().set(m_cheap_squeak, FUNC(bally_cheap_squeak_device::sound_int));
	m_cheap_squeak->sound_ack_w_handler().set(m_pia_u11, FUNC(pia6821_device::cb2_w));

	SPEAKER(config, "beee").front_center();
	BEEP(config, m_beep, 600).add_route(ALL_OUTPUTS, "beee", 0.10);
}

void by133_state::granny(machine_config &config)
{
	by133(config);

	MC6809(config.replace(), m_videocpu, XTAL(8'000'000)); // MC68B09P (XTAL value hard to read)
	m_videocpu->set_addrmap(AS_PROGRAM, &by133_state::granny_map);

	TMS9928A(config, m_crtc2, XTAL(10'738'635)).set_screen("screen");
	m_crtc2->set_vram_size(0x4000);
	m_crtc2->int_callback().set_inputline(m_videocpu, M6809_IRQ_LINE);

	subdevice<screen_device>("screen")->set_screen_update(FUNC(by133_state::screen_update_granny));
}


/*-----------------------------------------------------
/ Baby Pacman (Video/Pinball Combo) (BY133-891:  10/82) (Game #1299)
/-----------------------------------------------------*/
ROM_START(babypac)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "891-u2.732", 0x0000, 0x1000, CRC(7f7242d1) SHA1(213a697bb7fc69f93ea04621f0fcfdd796f35196))
	ROM_LOAD( "891-u6.732", 0x1000, 0x1000, CRC(6136d636) SHA1(c01a0a2fcad3bdabd649128e012ab558b1c90cd3) )

	ROM_REGION(0x10000, "videocpu", 0)
	ROM_LOAD( "891-16-u09.764", 0x8000, 0x2000, CRC(781e90e9) SHA1(940047cc875ae531a825af069bb650d59c9495a6))
	ROM_LOAD( "891-11-u10.764", 0xa000, 0x2000, CRC(28f4df8b) SHA1(bd6a3598c2c90b5a3a59327616d2f5b9940d98bc))
	ROM_LOAD( "891-05-u11.764", 0xc000, 0x2000, CRC(0a5967a4) SHA1(26d56ddea3f39d41e382449007bf7ba113c0285f))
	ROM_LOAD( "891-06-u12.764", 0xe000, 0x2000, CRC(58cfe542) SHA1(e024d14019866bd460d1da6b901f9b786a76a181))

	ROM_REGION(0x10000, "cheap_squeak:cpu", 0)
	ROM_LOAD( "891-12-u29.764", 0xc000, 0x2000, CRC(0b57fd5d) SHA1(43a03e6d16c87c3305adb04722484f992f23a1bd))
ROM_END

ROM_START(babypac2)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "891-u2.732", 0x0000, 0x1000, CRC(7f7242d1) SHA1(213a697bb7fc69f93ea04621f0fcfdd796f35196))
	ROM_LOAD( "891-u6.732", 0x1000, 0x1000, CRC(6136d636) SHA1(c01a0a2fcad3bdabd649128e012ab558b1c90cd3) )

	ROM_REGION(0x10000, "videocpu", 0)
	ROM_LOAD( "891-13-u09.764", 0x8000, 0x2000, CRC(7fa570f3) SHA1(423ad9266b1ded00fa52ce4180d518874142a203))
	ROM_LOAD( "891-11-u10.764", 0xa000, 0x2000, CRC(28f4df8b) SHA1(bd6a3598c2c90b5a3a59327616d2f5b9940d98bc))
	ROM_LOAD( "891-05-u11.764", 0xc000, 0x2000, CRC(0a5967a4) SHA1(26d56ddea3f39d41e382449007bf7ba113c0285f))
	ROM_LOAD( "891-06-u12.764", 0xe000, 0x2000, CRC(58cfe542) SHA1(e024d14019866bd460d1da6b901f9b786a76a181))

	ROM_REGION(0x10000, "cheap_squeak:cpu", 0)
	ROM_LOAD( "891-12-u29.764", 0xc000, 0x2000, CRC(0b57fd5d) SHA1(43a03e6d16c87c3305adb04722484f992f23a1bd))
ROM_END

/*-----------------------------------------------------------------
/ Granny and the Gators (Video/Pinball Combo) - (BY35-???: 01/84) (Game #1369)
/----------------------------------------------------------------*/
ROM_START(granny)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "cpu_u2.532", 0x0000, 0x1000, CRC(d45bb956) SHA1(86a6942ff9fe38fa109ecde40dc2dd19adf938a9))
	ROM_LOAD( "cpu_u6.532", 0x1000, 0x1000, CRC(306aa673) SHA1(422c3d9decf9214a18edb536c2077bf52b272e7d) )

	ROM_REGION(0x10000, "videocpu", 0)
	ROM_LOAD( "vid_u4.764", 0x4000, 0x2000, CRC(3a3d4c6b) SHA1(a6c27eee178a4bde67004e11f6ddf3b6414571dd))
	ROM_LOAD( "vid_u5.764", 0x6000, 0x2000, CRC(78bcb0fb) SHA1(d9dc1cc1bef063d5fbdbf2d1daf793234a9c55a0))
	ROM_LOAD( "vid_u6.764", 0x8000, 0x2000, CRC(8d8220a6) SHA1(64aa7d6ef2702c1b9afc61528434caf56cb91396))
	ROM_LOAD( "vid_u7.764", 0xa000, 0x2000, CRC(aa71cf29) SHA1(b69cd4060f5d4d2a7f85d901552cdc987013fde2))
	ROM_LOAD( "vid_u8.764", 0xc000, 0x2000, CRC(a442bc01) SHA1(2c01123dc5799561ae9e7c5d6db588b82b5ae59c))
	ROM_LOAD( "vid_u9.764", 0xe000, 0x2000, CRC(6b67a1f7) SHA1(251c2b941898363bbd6ee1a94710e2b2938ec851))

	ROM_REGION(0x10000, "cheap_squeak:cpu", 0)
	ROM_LOAD( "cs_u3.764", 0xc000, 0x2000, CRC(0a39a51d) SHA1(98342ba38e48578ce9870f2ee85b553d46c0e35f))
ROM_END

} // Anonymous namespace


GAME( 1982, babypac,  0,       by133,   babypac, by133_state, empty_init, ROT90, "Dave Nutting Associates / Bally", "Baby Pac-Man (set 1)",  MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1982, babypac2, babypac, by133,   babypac, by133_state, empty_init, ROT90, "Dave Nutting Associates / Bally", "Baby Pac-Man (set 2)",  MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1984, granny,   0,       granny,  granny,  by133_state, empty_init, ROT0,  "Bally",                           "Granny and the Gators", MACHINE_IS_SKELETON_MECHANICAL )
