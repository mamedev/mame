// license:BSD-3-Clause
// copyright-holders:Robbbert
/**************************************************************

    PINBALL
    Bally MPU AS-2518-133
    Baby Pacman
    Granny & the Gators
    A blend of arcade video game, and pinball.

Babypac uses a MPU4 board containing the main cpu, and a Vidiot
board containing the video and sound cpus, and the video controller
and the sound DAC and amp.

Granny uses the MPU4 board, but it has a Vidiot Deluxe for the
video, and a Cheep Squeek sound board. The manual incorrectly
describes the babypac vidiot board, which is of little use.


ToDo (babypac):
- No sound
- Mechanical
- Artwork
- Beeper needs to be replaced by a red LED when artwork is done.

ToDo (granny):
- No sound
- Playfield inputs
- Mechanical
- Artwork
- Beeper needs to be replaced by a red LED when artwork is done.
- Video CPU type and clock needs verification.
- Screen blending needs improvement
- No schematic found.


***************************************************************/


#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "cpu/m6809/m6809.h"
#include "video/tms9928a.h"
#include "machine/6821pia.h"
#include "sound/dac.h"
#include "machine/nvram.h"
#include "sound/beep.h"

class by133_state : public driver_device
{
public:
	by133_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_videocpu(*this, "videocpu")
		, m_audiocpu(*this, "audiocpu")
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
	{ }

	DECLARE_READ8_MEMBER(sound_data_r);
	DECLARE_WRITE8_MEMBER(sound_data_w);
	DECLARE_READ8_MEMBER(m6803_port2_r);
	DECLARE_WRITE8_MEMBER(m6803_port2_w);
	DECLARE_INPUT_CHANGED_MEMBER(video_test);
	DECLARE_INPUT_CHANGED_MEMBER(sound_test);
	DECLARE_INPUT_CHANGED_MEMBER(activity_test);
	DECLARE_INPUT_CHANGED_MEMBER(self_test);
	DECLARE_READ8_MEMBER(u7_a_r);
	DECLARE_WRITE8_MEMBER(u7_a_w);
	DECLARE_READ8_MEMBER(u7_b_r);
	DECLARE_WRITE8_MEMBER(u7_b_w);
	DECLARE_READ8_MEMBER(u10_a_r);
	DECLARE_WRITE8_MEMBER(u10_a_w);
	DECLARE_READ8_MEMBER(u10_b_r);
	DECLARE_WRITE8_MEMBER(u10_b_w);
	DECLARE_READ8_MEMBER(u11_a_r);
	DECLARE_WRITE8_MEMBER(u11_a_w);
	DECLARE_READ8_MEMBER(u11_b_r);
	DECLARE_WRITE8_MEMBER(u11_b_w);
	DECLARE_WRITE_LINE_MEMBER(u7_ca2_w);
	DECLARE_WRITE_LINE_MEMBER(u10_ca2_w);
	DECLARE_WRITE_LINE_MEMBER(u11_ca2_w);
	DECLARE_WRITE_LINE_MEMBER(u7_cb2_w);
	DECLARE_WRITE_LINE_MEMBER(u10_cb2_w);
	DECLARE_WRITE_LINE_MEMBER(u11_cb2_w);
	TIMER_DEVICE_CALLBACK_MEMBER(u10_timer);
	TIMER_DEVICE_CALLBACK_MEMBER(u11_timer);
	DECLARE_WRITE8_MEMBER(granny_crtc_w);
	UINT32 screen_update_granny(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
private:
	UINT8 m_mpu_to_vid;
	UINT8 m_vid_to_mpu;
	UINT8 m_u7_a;
	UINT8 m_u7_b;
	UINT8 m_u10_a;
	UINT8 m_u10_b;
	bool m_u10_cb2;
	UINT8 m_u11_a;
	UINT8 m_u11_b;
	bool m_u10_timer;
	bool m_u11_timer;
	virtual void machine_reset() override;
	required_device<m6800_cpu_device> m_maincpu;
	required_device<m6809e_device> m_videocpu;
	required_device<m6803_cpu_device> m_audiocpu;
	required_device<pia6821_device> m_pia_u7;
	required_device<pia6821_device> m_pia_u10;
	required_device<pia6821_device> m_pia_u11;
	required_device<tms9928a_device> m_crtc;
	optional_device<tms9928a_device> m_crtc2; // for Granny only
	optional_device<beep_device> m_beep; // temp
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
};


static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, by133_state ) // U9 MPU
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x007f) AM_RAM // 128x8 in MC6810 U7 MPU
	AM_RANGE(0x0088, 0x008b) AM_DEVREADWRITE("pia_u10", pia6821_device, read, write) // PIA U10 MPU
	AM_RANGE(0x0090, 0x0093) AM_DEVREADWRITE("pia_u11", pia6821_device, read, write) // PIA U11 MPU
	AM_RANGE(0x0200, 0x03ff) AM_RAM AM_SHARE("nvram") // 256x4 in 5101L U8 MPU, battery backed (D4-7 are data, A4-8 are address)
	AM_RANGE(0x1000, 0x17ff) AM_ROM AM_REGION("roms", 0x0000)
	AM_RANGE(0x1800, 0x1fff) AM_ROM AM_REGION("roms", 0x1000)
	AM_RANGE(0x5000, 0x57ff) AM_ROM AM_REGION("roms", 0x0800)
	AM_RANGE(0x5800, 0x5fff) AM_ROM AM_REGION("roms", 0x1800)
	AM_RANGE(0x7000, 0x7fff) AM_ROM AM_REGION("roms", 0x1000)
ADDRESS_MAP_END

static ADDRESS_MAP_START( video_map, AS_PROGRAM, 8, by133_state ) // U8 Vidiot
	AM_RANGE(0x0000, 0x1fff) AM_READWRITE(sound_data_r,sound_data_w)
	AM_RANGE(0x2000, 0x2003) AM_MIRROR(0x0ffc) AM_DEVREADWRITE("pia_u7", pia6821_device, read, write) // PIA U7 Vidiot
	AM_RANGE(0x4000, 0x4000) AM_MIRROR(0x0ffe) AM_DEVREADWRITE("crtc", tms9928a_device, vram_read, vram_write)
	AM_RANGE(0x4001, 0x4001) AM_MIRROR(0x0ffe) AM_DEVREADWRITE("crtc", tms9928a_device, register_read, register_write)
	AM_RANGE(0x6000, 0x63ff) AM_MIRROR(0x1c00) AM_RAM
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( granny_map, AS_PROGRAM, 8, by133_state )
	AM_RANGE(0x0000, 0x0001) AM_READWRITE(sound_data_r,sound_data_w)
	AM_RANGE(0x0002, 0x0002) AM_DEVREADWRITE("crtc", tms9928a_device, vram_read, vram_write)
	AM_RANGE(0x0003, 0x0003) AM_DEVREADWRITE("crtc", tms9928a_device, register_read, register_write)
	AM_RANGE(0x0004, 0x0004) AM_DEVREADWRITE("crtc2", tms9928a_device, vram_read, vram_write)
	AM_RANGE(0x0005, 0x0005) AM_DEVREADWRITE("crtc2", tms9928a_device, register_read, register_write)
	AM_RANGE(0x0006, 0x0007) AM_WRITE(granny_crtc_w) // can write to both at once
	AM_RANGE(0x0008, 0x000b) AM_DEVREADWRITE("pia_u7", pia6821_device, read, write)
	AM_RANGE(0x2000, 0x27ff) AM_RAM
	AM_RANGE(0x2801, 0x2801) AM_READNOP // The '9' test reads this location constantly and throws away the result
	AM_RANGE(0x4000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, by133_state ) // U27 Vidiot
	AM_RANGE(0xc000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_portmap, AS_IO, 8, by133_state )
	AM_RANGE(M6801_PORT1, M6801_PORT1) AM_DEVWRITE("dac", dac_device, write_unsigned8) // P10-P17
	AM_RANGE(M6801_PORT2, M6801_PORT2) AM_READWRITE(m6803_port2_r, m6803_port2_w) // P20-P24 sound command in
ADDRESS_MAP_END


INPUT_CHANGED_MEMBER( by133_state::video_test )
{
	if(newval)
		m_videocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

INPUT_CHANGED_MEMBER( by133_state::sound_test )
{
	if(newval)
		m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

INPUT_CHANGED_MEMBER( by133_state::activity_test )
{
	if(newval)
		m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

INPUT_CHANGED_MEMBER( by133_state::self_test )
{
	m_pia_u10->ca1_w(newval);
}

static INPUT_PORTS_START( babypac )
	PORT_START("TEST")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Video Test") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, by133_state, video_test, 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Sound Test") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, by133_state, sound_test, 0)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME("Activity") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, by133_state, activity_test, 0)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE4 ) PORT_NAME("Self Test") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, by133_state, self_test, 0)

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
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Right Flipper EOS") PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT( 0x0a, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Rebounds") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Right Spinner") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Left Spinner") PORT_CODE(KEYCODE_F)

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x3c, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_TILT1 ) PORT_NAME("Slam Tilt")

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("R. Top Loop Lane") PORT_CODE(KEYCODE_COLON)
	PORT_BIT( 0x06, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("L. Top Loop Lane") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Tunnel Outlane") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Fruits Outlane") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("R. Inside Outlane") PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("L. Inside Outlane") PORT_CODE(KEYCODE_D)

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("#5 Drop Target (R.)") PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("#4 Drop Target") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("#3 Drop Target") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("#2 Drop Target") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("#1 Drop Target (L.)") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Outhole") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("R. Maze Saucer") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("L. Maze Saucer") PORT_CODE(KEYCODE_G)

	PORT_START("X4")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( granny )
	PORT_START("TEST")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Video Test") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, by133_state, video_test, 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Sound Test") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, by133_state, sound_test, 0)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME("Activity") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, by133_state, activity_test, 0)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE4 ) PORT_NAME("Self Test") PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, by133_state, self_test, 0)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_NAME("Power")

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
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Canoe Rollover Button 1") PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Canoe Rollover Button 2") PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Canoe Rollover Button 3") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Canoe Rollover Button 4") PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Canoe Rollover Button 5") PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Canoe Rollover Button 6") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Canoe Rollover Button 7") PORT_CODE(KEYCODE_COLON)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Canoe Rollover Button 8") PORT_CODE(KEYCODE_QUOTE)

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x3c, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_TILT1 ) PORT_NAME("Slam Tilt")

	PORT_START("X2")
	PORT_BIT( 0x09, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Outhole") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Top R. Gate") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("L. Rollover Buttons") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("R. Return Lane") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("L. Return Lane") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Drop Target R") PORT_CODE(KEYCODE_M)

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Top Saucer") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("R. Lane Kickback") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Ammo Target O") PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("2nd Ammo Target M") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("1st Ammo Target M") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Ammo Target A") PORT_CODE(KEYCODE_G)

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Back Target T") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Back Target I") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Back Target X") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Back Target E") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Drop Target P") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Drop Target O") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Drop Target W") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Drop Target E") PORT_CODE(KEYCODE_N)
INPUT_PORTS_END


WRITE8_MEMBER( by133_state::granny_crtc_w )
{
	if (offset)
	{
		m_crtc->register_write(space, 0, data);
		m_crtc2->register_write(space, 0, data);
	}
	else
	{
		m_crtc->vram_write(space, 0, data);
		m_crtc2->vram_write(space, 0, data);
	}
}

READ8_MEMBER( by133_state::sound_data_r )
{
	return m_mpu_to_vid;
}

WRITE8_MEMBER( by133_state::sound_data_w )
{
	m_vid_to_mpu = data;
}

READ8_MEMBER( by133_state::m6803_port2_r )
{
	//machine().scheduler().synchronize();
	return (m_u7_b << 1) | 0;
}

WRITE8_MEMBER( by133_state::m6803_port2_w )
{
	//m_u7_b = data >> 1;
	m_beep->set_frequency(600);
	m_beep->set_state(BIT(data, 0));
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
	m_beep->set_frequency(950);
	m_beep->set_state(state);
}

WRITE_LINE_MEMBER( by133_state::u10_cb2_w )
{
	// lamp strobe #1
	m_u10_cb2 = state;
}

WRITE_LINE_MEMBER( by133_state::u11_cb2_w )
{
	// solenoid-sound selector
}

READ8_MEMBER( by133_state::u7_a_r )
{
	return m_u7_a;
}

WRITE8_MEMBER( by133_state::u7_a_w )
{
	m_u7_a = data;
}

READ8_MEMBER( by133_state::u7_b_r )
{
	if (BIT(m_u7_a, 7)) // bits 6 and 7 work; pinmame uses 7
		m_u7_b |= m_io_joy->read();

	if (BIT(m_u7_a, 6)) // Granny has a "power" button - used to shoot your gun. Is also 2-player start.
		m_u7_b = m_io_test->read() & 0x80;

	return m_u7_b;
}

WRITE8_MEMBER( by133_state::u7_b_w )
{
	//machine().scheduler().synchronize();
	m_u7_b = data;
}

READ8_MEMBER( by133_state::u10_a_r )
{
	return m_u10_a;
}

WRITE8_MEMBER( by133_state::u10_a_w )
{
	m_u10_a = data;
	if (BIT(m_u11_a, 2) == 0)
		m_mpu_to_vid = data ^ 0x0f;
}

READ8_MEMBER( by133_state::u10_b_r )
{
	if (BIT(m_u11_a, 3) == 0)
		return ~m_u7_a & 0x03;

	if (BIT(m_u11_a, 1) == 0)
		return m_vid_to_mpu;

	UINT8 data = 0;

	if (BIT(m_u10_a, 0))
		data |= m_io_x0->read();

	if (BIT(m_u10_a, 1))
		data |= m_io_x1->read();

	if (BIT(m_u10_a, 2))
		data |= m_io_x2->read();

	if (BIT(m_u10_a, 3))
		data |= m_io_x3->read();

	if (BIT(m_u10_a, 4))
		data |= m_io_x4->read(); // granny only

	if (BIT(m_u10_a, 5))
		data |= m_io_dsw0->read();

	if (BIT(m_u10_a, 6))
		data |= m_io_dsw1->read();

	if (BIT(m_u10_a, 7))
		data |= m_io_dsw2->read();

	if (m_u10_cb2)
		data |= m_io_dsw3->read();

	return data;
}

WRITE8_MEMBER( by133_state::u10_b_w )
{
	m_u10_b = data;
}

READ8_MEMBER( by133_state::u11_a_r )
{
	return m_u11_a;
}

WRITE8_MEMBER( by133_state::u11_a_w )
{
	m_u11_a = data;
	m_pia_u7->ca1_w(BIT(data, 1));
	m_pia_u7->ca2_w(BIT(data, 2));
}

READ8_MEMBER( by133_state::u11_b_r )
{
	return m_u11_b;
}

WRITE8_MEMBER( by133_state::u11_b_w )
{
	m_u11_b = data;
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

void by133_state::machine_reset()
{
	m_u7_a = 0;
	m_u7_b = 1; // select mode 2 of mc6803 on /reset
	m_u10_a = 0;
	m_u10_b = 0;
	m_u10_cb2 = 0;
	m_u11_a = 0;
	m_u11_b = 0;
	m_mpu_to_vid = 0;
	m_vid_to_mpu = 0;
	m_beep->set_state(0);
}

UINT32 by133_state::screen_update_granny(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	//bitmap.fill(0xff000000, cliprect);
	copybitmap(bitmap, m_crtc->get_bitmap(), 0, 0, 0, 0, cliprect);
	copybitmap_trans(bitmap, m_crtc2->get_bitmap(), 0, 0, 0, 0, cliprect, 0xff000000);
	return 0;
}

static MACHINE_CONFIG_START( babypac, by133_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800, XTAL_3_579545MHz/4) // no xtal, just 2 chips
	MCFG_CPU_PROGRAM_MAP(main_map)

	MCFG_CPU_ADD("videocpu", M6809E, XTAL_3_579545MHz)
	MCFG_CPU_PROGRAM_MAP(video_map)

	MCFG_CPU_ADD("audiocpu", M6803, XTAL_3_579545MHz)
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_IO_MAP(sound_portmap)

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_DEVICE_ADD("pia_u7", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(by133_state, u7_a_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(by133_state, u7_a_w))
	MCFG_PIA_READPB_HANDLER(READ8(by133_state, u7_b_r))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(by133_state, u7_b_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(by133_state, u7_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(by133_state, u7_cb2_w))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("videocpu", m6809e_device, firq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("videocpu", m6809e_device, firq_line))

	MCFG_DEVICE_ADD("pia_u10", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(by133_state, u10_a_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(by133_state, u10_a_w))
	MCFG_PIA_READPB_HANDLER(READ8(by133_state, u10_b_r))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(by133_state, u10_b_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(by133_state, u10_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(by133_state, u10_cb2_w))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("maincpu", m6800_cpu_device, irq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("maincpu", m6800_cpu_device, irq_line))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("babypac1", by133_state, u10_timer, attotime::from_hz(120)) // mains freq*2

	MCFG_DEVICE_ADD("pia_u11", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(by133_state, u11_a_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(by133_state, u11_a_w))
	MCFG_PIA_READPB_HANDLER(READ8(by133_state, u11_b_r))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(by133_state, u11_b_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(by133_state, u11_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(by133_state, u11_cb2_w))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("maincpu", m6800_cpu_device, irq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("maincpu", m6800_cpu_device, irq_line))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("babypac2", by133_state, u11_timer, attotime::from_hz(634)) // 555 timer*2

	/* video hardware */
	MCFG_DEVICE_ADD( "crtc", TMS9928A, XTAL_10_738635MHz / 2 )
	MCFG_TMS9928A_VRAM_SIZE(0x4000)
	MCFG_TMS9928A_OUT_INT_LINE_CB(DEVWRITELINE("videocpu", m6809e_device, irq_line))
	MCFG_TMS9928A_SCREEN_ADD_NTSC( "screen" )
	MCFG_SCREEN_UPDATE_DEVICE( "crtc", tms9928a_device, screen_update )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
	MCFG_SPEAKER_STANDARD_MONO("beee")
	MCFG_SOUND_ADD("beeper", BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "beee", 0.10)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( granny, babypac )
	MCFG_DEVICE_REMOVE("videocpu")
	MCFG_CPU_ADD("videocpu", M6809E, XTAL_8MHz) //??
	MCFG_CPU_PROGRAM_MAP(granny_map)

	MCFG_DEVICE_REMOVE("screen")

	MCFG_DEVICE_ADD( "crtc2", TMS9928A, XTAL_10_738635MHz / 2 )
	MCFG_TMS9928A_VRAM_SIZE(0x4000)
	MCFG_TMS9928A_OUT_INT_LINE_CB(DEVWRITELINE("videocpu", m6809e_device, irq_line))
	MCFG_VIDEO_SET_SCREEN("screen")

	MCFG_SCREEN_ADD( "screen", RASTER )
	MCFG_SCREEN_RAW_PARAMS( XTAL_10_738635MHz / 2, TMS9928A_TOTAL_HORZ, TMS9928A_HORZ_DISPLAY_START-12, TMS9928A_HORZ_DISPLAY_START + 256 + 12, \
			TMS9928A_TOTAL_VERT_NTSC, TMS9928A_VERT_DISPLAY_START_NTSC - 12, TMS9928A_VERT_DISPLAY_START_NTSC + 192 + 12 )
	MCFG_SCREEN_UPDATE_DRIVER(by133_state, screen_update_granny)
MACHINE_CONFIG_END


/*-----------------------------------------------------
/ Baby Pacman (Video/Pinball Combo) (BY133-891:  10/82)
/-----------------------------------------------------*/
ROM_START(babypac)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD( "891-u2.732", 0x0000, 0x1000, CRC(7f7242d1) SHA1(213a697bb7fc69f93ea04621f0fcfdd796f35196))
	ROM_LOAD( "891-u6.732", 0x1000, 0x1000, CRC(6136d636) SHA1(c01a0a2fcad3bdabd649128e012ab558b1c90cd3) )

	ROM_REGION(0x10000, "videocpu", 0)
	ROM_LOAD( "891-16-u09.764", 0x8000, 0x2000, CRC(781e90e9) SHA1(940047cc875ae531a825af069bb650d59c9495a6))
	ROM_LOAD( "891-11-u10.764", 0xa000, 0x2000, CRC(28f4df8b) SHA1(bd6a3598c2c90b5a3a59327616d2f5b9940d98bc))
	ROM_LOAD( "891-05-u11.764", 0xc000, 0x2000, CRC(0a5967a4) SHA1(26d56ddea3f39d41e382449007bf7ba113c0285f))
	ROM_LOAD( "891-06-u12.764", 0xe000, 0x2000, CRC(58cfe542) SHA1(e024d14019866bd460d1da6b901f9b786a76a181))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD( "891-12-u29.764", 0xe000, 0x2000, CRC(0b57fd5d) SHA1(43a03e6d16c87c3305adb04722484f992f23a1bd))
ROM_END

ROM_START(babypac2)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD( "891-u2.732", 0x0000, 0x1000, CRC(7f7242d1) SHA1(213a697bb7fc69f93ea04621f0fcfdd796f35196))
	ROM_LOAD( "891-u6.732", 0x1000, 0x1000, CRC(6136d636) SHA1(c01a0a2fcad3bdabd649128e012ab558b1c90cd3) )

	ROM_REGION(0x10000, "videocpu", 0)
	ROM_LOAD( "891-13-u09.764", 0x8000, 0x2000, CRC(7fa570f3) SHA1(423ad9266b1ded00fa52ce4180d518874142a203))
	ROM_LOAD( "891-11-u10.764", 0xa000, 0x2000, CRC(28f4df8b) SHA1(bd6a3598c2c90b5a3a59327616d2f5b9940d98bc))
	ROM_LOAD( "891-05-u11.764", 0xc000, 0x2000, CRC(0a5967a4) SHA1(26d56ddea3f39d41e382449007bf7ba113c0285f))
	ROM_LOAD( "891-06-u12.764", 0xe000, 0x2000, CRC(58cfe542) SHA1(e024d14019866bd460d1da6b901f9b786a76a181))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD( "891-12-u29.764", 0xe000, 0x2000, CRC(0b57fd5d) SHA1(43a03e6d16c87c3305adb04722484f992f23a1bd))
ROM_END

/*-----------------------------------------------------------------
/ Granny and the Gators (Video/Pinball Combo) - (BY35-???: 01/84)
/----------------------------------------------------------------*/
ROM_START(granny)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD( "cpu_u2.532", 0x0000, 0x1000, CRC(d45bb956) SHA1(86a6942ff9fe38fa109ecde40dc2dd19adf938a9))
	ROM_LOAD( "cpu_u6.532", 0x1000, 0x1000, CRC(306aa673) SHA1(422c3d9decf9214a18edb536c2077bf52b272e7d) )

	ROM_REGION(0x10000, "videocpu", 0)
	ROM_LOAD( "vid_u4.764", 0x4000, 0x2000, CRC(3a3d4c6b) SHA1(a6c27eee178a4bde67004e11f6ddf3b6414571dd))
	ROM_LOAD( "vid_u5.764", 0x6000, 0x2000, CRC(78bcb0fb) SHA1(d9dc1cc1bef063d5fbdbf2d1daf793234a9c55a0))
	ROM_LOAD( "vid_u6.764", 0x8000, 0x2000, CRC(8d8220a6) SHA1(64aa7d6ef2702c1b9afc61528434caf56cb91396))
	ROM_LOAD( "vid_u7.764", 0xa000, 0x2000, CRC(aa71cf29) SHA1(b69cd4060f5d4d2a7f85d901552cdc987013fde2))
	ROM_LOAD( "vid_u8.764", 0xc000, 0x2000, CRC(a442bc01) SHA1(2c01123dc5799561ae9e7c5d6db588b82b5ae59c))
	ROM_LOAD( "vid_u9.764", 0xe000, 0x2000, CRC(6b67a1f7) SHA1(251c2b941898363bbd6ee1a94710e2b2938ec851))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD( "cs_u3.764", 0xe000, 0x2000, CRC(0a39a51d) SHA1(98342ba38e48578ce9870f2ee85b553d46c0e35f))
ROM_END


GAME( 1982, babypac,  0,        babypac, babypac, driver_device,  0,  ROT90, "Dave Nutting Associates / Bally", "Baby Pac-Man (set 1)", MACHINE_MECHANICAL | MACHINE_NO_SOUND )
GAME( 1982, babypac2, babypac,  babypac, babypac, driver_device,  0,  ROT90, "Dave Nutting Associates / Bally", "Baby Pac-Man (set 2)", MACHINE_MECHANICAL | MACHINE_NO_SOUND )
GAME( 1984, granny,   0,        granny,  granny,  driver_device,  0,  ROT0,  "Bally", "Granny and the Gators", MACHINE_MECHANICAL | MACHINE_NO_SOUND )
