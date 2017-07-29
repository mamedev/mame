// license:BSD-3-Clause
// copyright-holders:hap, Sean Riddle
// thanks-to:Igor, ChoccyHobNob, RColtrane
/***************************************************************************

  Sharp SM5xx family handhelds.

  TODO:
  - improve LCD segments in SVGs for: gnw_mc25, gnw_eg26, exospace
  - SVG background/foreground vector graphics where possible. Doesn't apply to eg. the
    Konami games where MAME's SVG renderer needs to add support for embedded images.
  - confirm gnw_mc25/gnw_eg26 rom (dumped from Soviet clone, but pretty confident that it's same)

***************************************************************************/

#include "emu.h"
#include "cpu/sm510/sm510.h"
#include "cpu/sm510/sm500.h"
#include "sound/spkrdev.h"
#include "rendlay.h"
#include "screen.h"
#include "speaker.h"

// internal artwork
#include "gnw_dualv.lh"
#include "gnw_dualh.lh"
//#include "hh_sm510_test.lh" // common test-layout - use external artwork
//#include "hh_sm500_test.lh" // "


class hh_sm510_state : public driver_device
{
public:
	hh_sm510_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_inp_matrix(*this, "IN.%u", 0),
		m_out_x(*this, "%u.%u.%u", 0U, 0U, 0U),
		m_speaker(*this, "speaker"),
		m_inp_lines(0),
		m_display_wait(33)
	{ }

	// devices
	required_device<cpu_device> m_maincpu;
	optional_ioport_array<7> m_inp_matrix; // max 7
	output_finder<16, 16, 4> m_out_x;
	optional_device<speaker_sound_device> m_speaker;

	// misc common
	u16 m_inp_mux;                  // multiplexed inputs mask
	int m_inp_lines;                // number of input mux columns
	u8 m_s;                         // MCU S output pins
	u8 m_r;                         // MCU R output pins

	u8 read_inputs(int columns);

	virtual void update_k_line();
	virtual DECLARE_INPUT_CHANGED_MEMBER(input_changed);
	virtual DECLARE_INPUT_CHANGED_MEMBER(acl_button);
	virtual DECLARE_WRITE16_MEMBER(sm510_lcd_segment_w);
	virtual DECLARE_WRITE8_MEMBER(sm500_lcd_segment_w);
	virtual DECLARE_READ8_MEMBER(input_r);
	virtual DECLARE_WRITE8_MEMBER(input_w);
	virtual DECLARE_WRITE8_MEMBER(piezo_r1_w);
	virtual DECLARE_WRITE8_MEMBER(piezo_r2_w);
	virtual DECLARE_WRITE8_MEMBER(piezo_input_w);

	// display common
	int m_display_wait;             // lcd segment on/off-delay in milliseconds (default 33ms)
	u8 m_display_x_len;             // lcd number of groups
	u8 m_display_y_len;             // lcd number of segments
	u8 m_display_z_len;             // lcd number of commons
	u32 m_display_state[0x20];      // lcd segment data (max. 5-bit offset)
	u8 m_display_decay[0x20][0x20]; // (internal use)

	void set_display_size(u8 x, u8 y, u8 z);
	TIMER_DEVICE_CALLBACK_MEMBER(display_decay_tick);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
};


// machine start/reset

void hh_sm510_state::machine_start()
{
	// resolve handlers
	m_out_x.resolve();

	// zerofill
	m_inp_mux = 0;
	/* m_inp_lines = 0; */ // not here
	m_s = 0;
	m_r = 0;
	m_display_x_len = 0;
	m_display_y_len = 0;
	m_display_z_len = 0;
	memset(m_display_state, 0, sizeof(m_display_state));
	memset(m_display_decay, 0, sizeof(m_display_decay));

	// register for savestates
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_inp_lines));
	save_item(NAME(m_s));
	save_item(NAME(m_r));
	save_item(NAME(m_display_x_len));
	save_item(NAME(m_display_y_len));
	save_item(NAME(m_display_z_len));
	save_item(NAME(m_display_state));
	save_item(NAME(m_display_decay));
}

void hh_sm510_state::machine_reset()
{
}



/***************************************************************************

  Helper Functions

***************************************************************************/

// lcd panel - on lcd handhelds, usually not a generic x/y screen device
// deflicker here, especially needed for SM500/SM5A with the active shift register

TIMER_DEVICE_CALLBACK_MEMBER(hh_sm510_state::display_decay_tick)
{
	u8 z_mask = (1 << m_display_z_len) - 1;
	u8 zx_len = 1 << (m_display_x_len + m_display_z_len);

	for (int zx = 0; zx < zx_len; zx++)
	{
		for (int y = 0; y < m_display_y_len; y++)
		{
			// delay lcd segment on/off state
			if (m_display_state[zx] >> y & 1)
			{
				if (m_display_decay[y][zx] < (2 * m_display_wait - 1))
					m_display_decay[y][zx]++;
			}
			else if (m_display_decay[y][zx] > 0)
				m_display_decay[y][zx]--;
			u8 active_state = (m_display_decay[y][zx] < m_display_wait) ? 0 : 1;

			// SM510 series: output to x.y.z, where:
			// x = group a/b/bs/c (0/1/2/3)
			// y = segment 1-16 (0-15)
			// z = common H1-H4 (0-3)

			// SM500 series: output to x.y.z, where:
			// x = O group (0-*)
			// y = O segment 1-4 (0-3)
			// z = common H1/H2 (0/1)
			m_out_x[zx >> m_display_z_len][y][zx & z_mask] = active_state;
		}
	}
}

void hh_sm510_state::set_display_size(u8 x, u8 y, u8 z)
{
	// x = groups(in bits)
	// y = number of segments per group
	// z = commons(in bits)
	m_display_x_len = x;
	m_display_y_len = y;
	m_display_z_len = z;
}

WRITE16_MEMBER(hh_sm510_state::sm510_lcd_segment_w)
{
	set_display_size(2, 16, 2);
	m_display_state[offset] = data;
}

WRITE8_MEMBER(hh_sm510_state::sm500_lcd_segment_w)
{
	set_display_size(4, 4, 1);
	m_display_state[offset] = data;
}


// generic input handlers - usually S output is input mux, and K input for buttons

u8 hh_sm510_state::read_inputs(int columns)
{
	u8 ret = 0;

	// read selected input rows
	for (int i = 0; i < columns; i++)
		if (m_inp_mux >> i & 1)
			ret |= m_inp_matrix[i]->read();

	return ret;
}

void hh_sm510_state::update_k_line()
{
	// this is necessary because the MCU can wake up on K input activity
	m_maincpu->set_input_line(SM510_INPUT_LINE_K, input_r(machine().dummy_space(), 0, 0xff) ? ASSERT_LINE : CLEAR_LINE);
}

INPUT_CHANGED_MEMBER(hh_sm510_state::input_changed)
{
	update_k_line();
}

WRITE8_MEMBER(hh_sm510_state::input_w)
{
	m_inp_mux = data;
	update_k_line();
}

READ8_MEMBER(hh_sm510_state::input_r)
{
	return read_inputs(m_inp_lines);
}

INPUT_CHANGED_MEMBER(hh_sm510_state::acl_button)
{
	// ACL button is directly tied to MCU ACL pin
	m_maincpu->set_input_line(SM510_INPUT_LINE_ACL, newval ? ASSERT_LINE : CLEAR_LINE);
}


// other generic output handlers

WRITE8_MEMBER(hh_sm510_state::piezo_r1_w)
{
	// R1 to piezo (SM511 R pin is melody output)
	m_speaker->level_w(data & 1);
}

WRITE8_MEMBER(hh_sm510_state::piezo_r2_w)
{
	// R2 to piezo
	m_speaker->level_w(data >> 1 & 1);
}

WRITE8_MEMBER(hh_sm510_state::piezo_input_w)
{
	// R1 to piezo, other to input mux
	piezo_r1_w(space, 0, data & 1);
	input_w(space, 0, data >> 1);
}





/***************************************************************************

  Minidrivers (subclass, I/O, Inputs, Machine Config)

***************************************************************************/

/***************************************************************************

  Konami Double Dribble
  * Sharp SM510 under epoxy (die label CMS54C, KMS584)
  * lcd screen with custom segments, 1-bit sound

  BTANB: At the basket, the ball goes missing sometimes for 1 frame, or
  may show 2 balls at the same time. It's the same on the real handheld.

***************************************************************************/

class kdribble_state : public hh_sm510_state
{
public:
	kdribble_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_sm510_state(mconfig, type, tag)
	{
		m_inp_lines = 3;
	}
};

// config

static INPUT_PORTS_START( kdribble )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Level Select")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_VOLUME_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Sound")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)

	PORT_START("IN.2") // S3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Power On/Start")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, acl_button, nullptr) PORT_NAME("All Clear")
INPUT_PORTS_END

static MACHINE_CONFIG_START( kdribble )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SM510, XTAL_32_768kHz)
	MCFG_SM510_WRITE_SEGS_CB(WRITE16(hh_sm510_state, sm510_lcd_segment_w))
	MCFG_SM510_READ_K_CB(READ8(hh_sm510_state, input_r))
	MCFG_SM510_WRITE_S_CB(WRITE8(hh_sm510_state, input_w))
	MCFG_SM510_WRITE_R_CB(WRITE8(hh_sm510_state, piezo_r1_w))

	/* video hardware */
	MCFG_SCREEN_SVG_ADD("screen", "svg")
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_SIZE(1490, 1080)
	MCFG_SCREEN_VISIBLE_AREA(0, 1490-1, 0, 1080-1)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_sm510_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_svg)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Konami Top Gun
  * PCB label BH003
  * Sharp SM510 under epoxy (die label CMS54C, KMS598)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class ktopgun_state : public hh_sm510_state
{
public:
	ktopgun_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_sm510_state(mconfig, type, tag)
	{
		m_inp_lines = 3;
	}
};

// config

static INPUT_PORTS_START( ktopgun )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_VOLUME_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Sound")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)

	PORT_START("IN.2") // S3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Power On/Start")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, acl_button, nullptr) PORT_NAME("All Clear")
INPUT_PORTS_END

static MACHINE_CONFIG_START( ktopgun )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SM510, XTAL_32_768kHz)
	MCFG_SM510_WRITE_SEGS_CB(WRITE16(hh_sm510_state, sm510_lcd_segment_w))
	MCFG_SM510_READ_K_CB(READ8(hh_sm510_state, input_r))
	MCFG_SM510_WRITE_S_CB(WRITE8(hh_sm510_state, input_w))
	MCFG_SM510_WRITE_R_CB(WRITE8(hh_sm510_state, piezo_r1_w))

	/* video hardware */
	MCFG_SCREEN_SVG_ADD("screen", "svg")
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_SIZE(1611, 1080)
	MCFG_SCREEN_VISIBLE_AREA(0, 1611-1, 0, 1080-1)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_sm510_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_svg)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Konami Contra
  * PCB label BH002
  * Sharp SM511 under epoxy (die label KMS73B, KMS773)
  * lcd screen with custom segments, 1-bit sound

  Contra handheld is titled simply "C" in the USA.

***************************************************************************/

class kcontra_state : public hh_sm510_state
{
public:
	kcontra_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_sm510_state(mconfig, type, tag)
	{
		m_inp_lines = 3;
	}
};

// config

static INPUT_PORTS_START( kcontra )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_VOLUME_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Sound")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)

	PORT_START("IN.2") // S3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Power On/Start")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, acl_button, nullptr) PORT_NAME("All Clear")
INPUT_PORTS_END

static MACHINE_CONFIG_START( kcontra )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SM511, XTAL_32_768kHz)
	MCFG_SM510_WRITE_SEGS_CB(WRITE16(hh_sm510_state, sm510_lcd_segment_w))
	MCFG_SM510_READ_K_CB(READ8(hh_sm510_state, input_r))
	MCFG_SM510_WRITE_S_CB(WRITE8(hh_sm510_state, input_w))
	MCFG_SM510_WRITE_R_CB(WRITE8(hh_sm510_state, piezo_r1_w))

	/* video hardware */
	MCFG_SCREEN_SVG_ADD("screen", "svg")
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_SIZE(1501, 1080)
	MCFG_SCREEN_VISIBLE_AREA(0, 1501-1, 0, 1080-1)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_sm510_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_svg)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Konami Teenage Mutant Ninja Turtles
  * PCB label BH005
  * Sharp SM511 under epoxy (die label KMS73B, KMS774)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class ktmnt_state : public hh_sm510_state
{
public:
	ktmnt_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_sm510_state(mconfig, type, tag)
	{
		m_inp_lines = 3;
	}
};

// config

static INPUT_PORTS_START( ktmnt )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_VOLUME_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Sound")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Power On/Start")

	PORT_START("IN.2") // S3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, acl_button, nullptr) PORT_NAME("All Clear")
INPUT_PORTS_END

static MACHINE_CONFIG_START( ktmnt )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SM511, XTAL_32_768kHz)
	MCFG_SM510_WRITE_SEGS_CB(WRITE16(hh_sm510_state, sm510_lcd_segment_w))
	MCFG_SM510_READ_K_CB(READ8(hh_sm510_state, input_r))
	MCFG_SM510_WRITE_S_CB(WRITE8(hh_sm510_state, input_w))
	MCFG_SM510_WRITE_R_CB(WRITE8(hh_sm510_state, piezo_r1_w))

	/* video hardware */
	MCFG_SCREEN_SVG_ADD("screen", "svg")
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_SIZE(1380, 1080)
	MCFG_SCREEN_VISIBLE_AREA(0, 1380-1, 0, 1080-1)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_sm510_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_svg)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Konami Gradius
  * PCB label BH004
  * Sharp SM511 under epoxy (die label KMS73B, KMS774)
  * lcd screen with custom segments, 1-bit sound

  Known in Japan as Nemesis.

***************************************************************************/

class kgradius_state : public hh_sm510_state
{
public:
	kgradius_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_sm510_state(mconfig, type, tag)
	{
		m_inp_lines = 2;
	}
};

// config

static INPUT_PORTS_START( kgradius )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Power On/Start")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_VOLUME_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Sound")

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, acl_button, nullptr) PORT_NAME("All Clear")
INPUT_PORTS_END

static MACHINE_CONFIG_START( kgradius )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SM511, XTAL_32_768kHz)
	MCFG_SM510_WRITE_SEGS_CB(WRITE16(hh_sm510_state, sm510_lcd_segment_w))
	MCFG_SM510_READ_K_CB(READ8(hh_sm510_state, input_r))
	MCFG_SM510_WRITE_S_CB(WRITE8(hh_sm510_state, input_w))
	MCFG_SM510_WRITE_R_CB(WRITE8(hh_sm510_state, piezo_r1_w))

	/* video hardware */
	MCFG_SCREEN_SVG_ADD("screen", "svg")
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_SIZE(1435, 1080)
	MCFG_SCREEN_VISIBLE_AREA(0, 1435-1, 0, 1080-1)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_sm510_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_svg)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Konami Lone Ranger
  * PCB label BH009
  * Sharp SM511 under epoxy (die label KMS73B, KMS781)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class kloneran_state : public hh_sm510_state
{
public:
	kloneran_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_sm510_state(mconfig, type, tag)
	{
		m_inp_lines = 2;
	}
};

// config

static INPUT_PORTS_START( kloneran )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_VOLUME_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Sound")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Pause")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Power On/Start")

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, acl_button, nullptr) PORT_NAME("All Clear")
INPUT_PORTS_END

static MACHINE_CONFIG_START( kloneran )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SM511, XTAL_32_768kHz)
	MCFG_SM510_WRITE_SEGS_CB(WRITE16(hh_sm510_state, sm510_lcd_segment_w))
	MCFG_SM510_READ_K_CB(READ8(hh_sm510_state, input_r))
	MCFG_SM510_WRITE_S_CB(WRITE8(hh_sm510_state, input_w))
	MCFG_SM510_WRITE_R_CB(WRITE8(hh_sm510_state, piezo_r1_w))

	/* video hardware */
	MCFG_SCREEN_SVG_ADD("screen", "svg")
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_SIZE(1495, 1080)
	MCFG_SCREEN_VISIBLE_AREA(0, 1495-1, 0, 1080-1)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_sm510_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_svg)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Konami Blades of Steel
  * PCB label BH011
  * Sharp SM511 under epoxy (die label KMS73B, KMS782)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class kblades_state : public hh_sm510_state
{
public:
	kblades_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_sm510_state(mconfig, type, tag)
	{
		m_inp_lines = 3;
	}
};

// config

static INPUT_PORTS_START( kblades )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_VOLUME_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Sound")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Pause")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Power On/Start")

	PORT_START("IN.2") // S3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, acl_button, nullptr) PORT_NAME("All Clear")
INPUT_PORTS_END

static MACHINE_CONFIG_START( kblades )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SM511, XTAL_32_768kHz)
	MCFG_SM510_WRITE_SEGS_CB(WRITE16(hh_sm510_state, sm510_lcd_segment_w))
	MCFG_SM510_READ_K_CB(READ8(hh_sm510_state, input_r))
	MCFG_SM510_WRITE_S_CB(WRITE8(hh_sm510_state, input_w))
	MCFG_SM510_WRITE_R_CB(WRITE8(hh_sm510_state, piezo_r1_w))

	/* video hardware */
	MCFG_SCREEN_SVG_ADD("screen", "svg")
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_SIZE(1511, 1080)
	MCFG_SCREEN_VISIBLE_AREA(0, 1511-1, 0, 1080-1)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_sm510_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_svg)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Konami NFL Football
  * Sharp SM511 under epoxy (die label KMS73B, KMS786)
  * lcd screen with custom segments, 1-bit sound

  This is the 1989 version. It was rereleased in 1992, assumed to be the same
  game underneath.

***************************************************************************/

class knfl_state : public hh_sm510_state
{
public:
	knfl_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_sm510_state(mconfig, type, tag)
	{
		m_inp_lines = 3;
	}
};

// config

static INPUT_PORTS_START( knfl )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_VOLUME_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Sound")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Pause")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Power On/Start")

	PORT_START("IN.2") // S3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, acl_button, nullptr) PORT_NAME("All Clear")
INPUT_PORTS_END

static MACHINE_CONFIG_START( knfl )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SM511, XTAL_32_768kHz)
	MCFG_SM510_WRITE_SEGS_CB(WRITE16(hh_sm510_state, sm510_lcd_segment_w))
	MCFG_SM510_READ_K_CB(READ8(hh_sm510_state, input_r))
	MCFG_SM510_WRITE_S_CB(WRITE8(hh_sm510_state, input_w))
	MCFG_SM510_WRITE_R_CB(WRITE8(hh_sm510_state, piezo_r1_w))

	/* video hardware */
	MCFG_SCREEN_SVG_ADD("screen", "svg")
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_SIZE(1406, 1080)
	MCFG_SCREEN_VISIBLE_AREA(0, 1406-1, 0, 1080-1)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_sm510_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_svg)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Konami The Adventures of Bayou Billy
  * Sharp SM511 under epoxy (die label KMS73B, KMS788)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class kbilly_state : public hh_sm510_state
{
public:
	kbilly_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_sm510_state(mconfig, type, tag)
	{
		m_inp_lines = 3;
	}
};

// config

static INPUT_PORTS_START( kbilly )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_VOLUME_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Sound")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Pause")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Power On/Start")

	PORT_START("IN.2") // S3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, acl_button, nullptr) PORT_NAME("All Clear")
INPUT_PORTS_END

static MACHINE_CONFIG_START( kbilly )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SM511, XTAL_32_768kHz)
	MCFG_SM510_WRITE_SEGS_CB(WRITE16(hh_sm510_state, sm510_lcd_segment_w))
	MCFG_SM510_READ_K_CB(READ8(hh_sm510_state, input_r))
	MCFG_SM510_WRITE_S_CB(WRITE8(hh_sm510_state, input_w))
	MCFG_SM510_WRITE_R_CB(WRITE8(hh_sm510_state, piezo_r1_w))

	/* video hardware */
	MCFG_SCREEN_SVG_ADD("screen", "svg")
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_SIZE(1504, 1080)
	MCFG_SCREEN_VISIBLE_AREA(0, 1504-1, 0, 1080-1)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_sm510_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_svg)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Konami Bucky O'Hare
  * Sharp SM511 under epoxy (die label KMS73B, N58)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class kbucky_state : public hh_sm510_state
{
public:
	kbucky_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_sm510_state(mconfig, type, tag)
	{
		m_inp_lines = 2;
	}
};

// config

static INPUT_PORTS_START( kbucky )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Pause")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Power On/Start")

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_VOLUME_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Sound")

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, acl_button, nullptr) PORT_NAME("All Clear")
INPUT_PORTS_END

static MACHINE_CONFIG_START( kbucky )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SM511, XTAL_32_768kHz)
	MCFG_SM510_WRITE_SEGS_CB(WRITE16(hh_sm510_state, sm510_lcd_segment_w))
	MCFG_SM510_READ_K_CB(READ8(hh_sm510_state, input_r))
	MCFG_SM510_WRITE_S_CB(WRITE8(hh_sm510_state, input_w))
	MCFG_SM510_WRITE_R_CB(WRITE8(hh_sm510_state, piezo_r1_w))

	/* video hardware */
	MCFG_SCREEN_SVG_ADD("screen", "svg")
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_SIZE(1501, 1080)
	MCFG_SCREEN_VISIBLE_AREA(0, 1501-1, 0, 1080-1)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_sm510_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_svg)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Konami Garfield
  * Sharp SM511 under epoxy (die label KMS73B, N62)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class kgarfld_state : public hh_sm510_state
{
public:
	kgarfld_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_sm510_state(mconfig, type, tag)
	{
		m_inp_lines = 3;
	}
};

// config

static INPUT_PORTS_START( kgarfld )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_VOLUME_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Sound")

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Pause")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Power On/Start")

	PORT_START("IN.2") // S3
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, acl_button, nullptr) PORT_NAME("All Clear")
INPUT_PORTS_END

static MACHINE_CONFIG_START( kgarfld )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SM511, XTAL_32_768kHz)
	MCFG_SM510_WRITE_SEGS_CB(WRITE16(hh_sm510_state, sm510_lcd_segment_w))
	MCFG_SM510_READ_K_CB(READ8(hh_sm510_state, input_r))
	MCFG_SM510_WRITE_S_CB(WRITE8(hh_sm510_state, input_w))
	MCFG_SM510_WRITE_R_CB(WRITE8(hh_sm510_state, piezo_r1_w))

	/* video hardware */
	MCFG_SCREEN_SVG_ADD("screen", "svg")
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_SIZE(1501, 1080)
	MCFG_SCREEN_VISIBLE_AREA(0, 1501-1, 0, 1080-1)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_sm510_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_svg)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Nintendo Game & Watch: Mickey Mouse (model MC-25), Egg (model EG-26)
  * Sharp SM5A label ?
  * lcd screen with custom segments, 1-bit sound

  MC-25 and EG-26 are the same game, it's assumed that the latter was for
  regions where Nintendo wasn't able to license from Disney.

  In 1984, Elektronika(USSR) released a clone: Nu, pogodi! This was followed
  by several other titles that were the same under the hood, only differing
  in graphics. They also made a slightly modified version, adding a new game
  mode (by pressing A+B) where the player/CPU roles are reversed. This version
  is known as Razvedciki kosmosa (export version: Explorers of Space).

***************************************************************************/

class mc25_state : public hh_sm510_state
{
public:
	mc25_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_sm510_state(mconfig, type, tag)
	{
		m_inp_lines = 3;
	}
};

// config

static INPUT_PORTS_START( mc25 )
	PORT_START("IN.0") // R2
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_16WAY

	PORT_START("IN.2") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Game B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Game A")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Alarm")

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, acl_button, nullptr) PORT_NAME("ACL")

	PORT_START("BA") // MCU BA(alpha) pin pulled to GND, only works after power-on
	PORT_CONFNAME( 0x01, 0x01, "Infinite Lives (Cheat)")
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( exospace )
	PORT_INCLUDE( mc25 )

	PORT_MODIFY("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static MACHINE_CONFIG_START( mc25 )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SM5A, XTAL_32_768kHz)
	MCFG_SM500_WRITE_O_CB(WRITE8(hh_sm510_state, sm500_lcd_segment_w))
	MCFG_SM510_READ_K_CB(READ8(hh_sm510_state, input_r))
	MCFG_SM510_WRITE_R_CB(WRITE8(hh_sm510_state, piezo_input_w))
	MCFG_SM510_READ_BA_CB(IOPORT("BA"))

	/* video hardware */
	MCFG_SCREEN_SVG_ADD("screen", "svg")
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_SIZE(1711, 1080)
	MCFG_SCREEN_VISIBLE_AREA(0, 1711-1, 0, 1080-1)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_sm510_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_svg)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( eg26, mc25 )

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_SIZE(1694, 1080)
	MCFG_SCREEN_VISIBLE_AREA(0, 1694-1, 0, 1080-1)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( nupogodi, mc25 )

	/* basic machine hardware */
	MCFG_CPU_REPLACE("maincpu", KB1013VK12, XTAL_32_768kHz)
	MCFG_SM500_WRITE_O_CB(WRITE8(hh_sm510_state, sm500_lcd_segment_w))
	MCFG_SM510_READ_K_CB(READ8(hh_sm510_state, input_r))
	MCFG_SM510_WRITE_R_CB(WRITE8(hh_sm510_state, piezo_input_w))
	MCFG_SM510_READ_BA_CB(IOPORT("BA"))

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_SIZE(1715, 1080)
	MCFG_SCREEN_VISIBLE_AREA(0, 1715-1, 0, 1080-1)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( exospace, nupogodi )

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_SIZE(1756, 1080)
	MCFG_SCREEN_VISIBLE_AREA(0, 1756-1, 0, 1080-1)
MACHINE_CONFIG_END





/***************************************************************************

  Nintendo Game & Watch: Mickey & Donald (model DM-53)
  * PCB label DM-53
  * Sharp SM510 label DM-53 52ZC (die label CMS54C, CMS565)
  * vertical dual lcd screens with custom segments, 1-bit sound

***************************************************************************/

class dm53_state : public hh_sm510_state
{
public:
	dm53_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_sm510_state(mconfig, type, tag)
	{
		m_inp_lines = 2;
	}
};

// config

static INPUT_PORTS_START( dm53 )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Game B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Game A")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Alarm")

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, acl_button, nullptr) PORT_NAME("ACL")

	PORT_START("B") // MCU B(beta) pin pulled to GND
	PORT_CONFNAME( 0x01, 0x01, "Infinite Lives (Cheat)")
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static MACHINE_CONFIG_START( dm53 )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SM510, XTAL_32_768kHz)
	MCFG_SM510_WRITE_SEGS_CB(WRITE16(hh_sm510_state, sm510_lcd_segment_w))
	MCFG_SM510_READ_K_CB(READ8(hh_sm510_state, input_r))
	MCFG_SM510_WRITE_S_CB(WRITE8(hh_sm510_state, input_w))
	MCFG_SM510_WRITE_R_CB(WRITE8(hh_sm510_state, piezo_r2_w))
	MCFG_SM510_READ_B_CB(IOPORT("B"))

	/* video hardware */
	MCFG_SCREEN_SVG_ADD("screen_top", "svg_top")
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_SIZE(1920/2, 1281/2)
	MCFG_SCREEN_VISIBLE_AREA(0, 1920/2-1, 0, 1281/2-1)

	MCFG_SCREEN_SVG_ADD("screen_bottom", "svg_bottom")
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_SIZE(1920/2, 1236/2)
	MCFG_SCREEN_VISIBLE_AREA(0, 1920/2-1, 0, 1236/2-1)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_sm510_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_gnw_dualv)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Nintendo Game & Watch: Donkey Kong II (model JR-55)
  * PCB label JR-55
  * Sharp SM510 label JR-55 53YC (die label CMS54C, KMS560)
  * vertical dual lcd screens with custom segments, 1-bit sound

***************************************************************************/

class jr55_state : public hh_sm510_state
{
public:
	jr55_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_sm510_state(mconfig, type, tag)
	{
		m_inp_lines = 3;
	}
};

// config

static INPUT_PORTS_START( jr55 )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)

	PORT_START("IN.2") // S3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Game B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Game A")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Alarm")

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, acl_button, nullptr) PORT_NAME("ACL")

	PORT_START("B") // MCU B(beta) pin pulled to GND
	PORT_CONFNAME( 0x01, 0x01, "Invincibility (Cheat)")
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static MACHINE_CONFIG_START( jr55 )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SM510, XTAL_32_768kHz)
	MCFG_SM510_WRITE_SEGS_CB(WRITE16(hh_sm510_state, sm510_lcd_segment_w))
	MCFG_SM510_READ_K_CB(READ8(hh_sm510_state, input_r))
	MCFG_SM510_WRITE_S_CB(WRITE8(hh_sm510_state, input_w))
	MCFG_SM510_WRITE_R_CB(WRITE8(hh_sm510_state, piezo_r1_w))
	MCFG_SM510_READ_B_CB(IOPORT("B"))

	/* video hardware */
	MCFG_SCREEN_SVG_ADD("screen_top", "svg_top")
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_SIZE(1920/2, 1229/2)
	MCFG_SCREEN_VISIBLE_AREA(0, 1920/2-1, 0, 1229/2-1)

	MCFG_SCREEN_SVG_ADD("screen_bottom", "svg_bottom")
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_SIZE(1920/2, 1238/2)
	MCFG_SCREEN_VISIBLE_AREA(0, 1920/2-1, 0, 1238/2-1)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_sm510_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_gnw_dualv)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Nintendo Game & Watch: Mario Bros. (model MW-56)
  * PCB label MW-56-M
  * Sharp SM510 label MW-56 533C (no decap)
  * horizontal dual lcd screens with custom segments, 1-bit sound

***************************************************************************/

class mw56_state : public hh_sm510_state
{
public:
	mw56_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_sm510_state(mconfig, type, tag)
	{
		m_inp_lines = 2;
	}
};

// config

static INPUT_PORTS_START( mw56 )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Game B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Game A")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Alarm")

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, acl_button, nullptr) PORT_NAME("ACL")

	PORT_START("BA") // MCU BA(alpha) pin pulled to GND
	PORT_CONFNAME( 0x01, 0x01, "Increase Score (Cheat)")
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("B") // MCU B(beta) pin pulled to GND
	PORT_CONFNAME( 0x01, 0x01, "Infinite Lives (Cheat)")
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static MACHINE_CONFIG_START( mw56 )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SM510, XTAL_32_768kHz)
	MCFG_SM510_WRITE_SEGS_CB(WRITE16(hh_sm510_state, sm510_lcd_segment_w))
	MCFG_SM510_READ_K_CB(READ8(hh_sm510_state, input_r))
	MCFG_SM510_WRITE_S_CB(WRITE8(hh_sm510_state, input_w))
	MCFG_SM510_WRITE_R_CB(WRITE8(hh_sm510_state, piezo_r1_w))
	MCFG_SM510_READ_BA_CB(IOPORT("BA"))
	MCFG_SM510_READ_B_CB(IOPORT("B"))

	/* video hardware */
	MCFG_SCREEN_SVG_ADD("screen_left", "svg_left")
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_SIZE(2258/2, 1440/2)
	MCFG_SCREEN_VISIBLE_AREA(0, 2258/2-1, 0, 1440/2-1)

	MCFG_SCREEN_SVG_ADD("screen_right", "svg_right")
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_SIZE(2261/2, 1440/2)
	MCFG_SCREEN_VISIBLE_AREA(0, 2261/2-1, 0, 1440/2-1)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_sm510_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_gnw_dualh)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Nintendo Game & Watch: Donkey Kong Jr. (model DJ-101)
  * Sharp SM510 label DJ-101 52ZA (no decap)
  * lcd screen with custom segments, 1-bit sound

  This is the new wide screen version, there's also a tabletop version that
  plays more like the arcade game.

***************************************************************************/

class dj101_state : public hh_sm510_state
{
public:
	dj101_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_sm510_state(mconfig, type, tag)
	{
		m_inp_lines = 3;
	}
};

// config

static INPUT_PORTS_START( dj101 )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)

	PORT_START("IN.2") // S3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Game B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Game A")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Alarm")

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, acl_button, nullptr) PORT_NAME("ACL")

	PORT_START("BA") // MCU BA(alpha) pin pulled to GND
	PORT_CONFNAME( 0x01, 0x01, "Increase Score (Cheat)")
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("B") // MCU B(beta) pin pulled to GND
	PORT_CONFNAME( 0x01, 0x01, "Invincibility (Cheat)")
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static MACHINE_CONFIG_START( dj101 )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SM510, XTAL_32_768kHz)
	MCFG_SM510_WRITE_SEGS_CB(WRITE16(hh_sm510_state, sm510_lcd_segment_w))
	MCFG_SM510_READ_K_CB(READ8(hh_sm510_state, input_r))
	MCFG_SM510_WRITE_S_CB(WRITE8(hh_sm510_state, input_w))
	MCFG_SM510_WRITE_R_CB(WRITE8(hh_sm510_state, piezo_r1_w))
	MCFG_SM510_READ_BA_CB(IOPORT("BA"))
	MCFG_SM510_READ_B_CB(IOPORT("B"))

	/* video hardware */
	MCFG_SCREEN_SVG_ADD("screen", "svg")
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_SIZE(1665, 1080)
	MCFG_SCREEN_VISIBLE_AREA(0, 1665-1, 0, 1080-1)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_sm510_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_svg)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Nintendo Game & Watch: Mario's Cement Factory (model ML-102)
  * Sharp SM510 label ML-102 298D (die label CMS54C, KMS577)
  * lcd screen with custom segments, 1-bit sound

  This is the new wide screen version, there's also a tabletop version.

***************************************************************************/

class ml102_state : public hh_sm510_state
{
public:
	ml102_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_sm510_state(mconfig, type, tag)
	{
		m_inp_lines = 2;
	}
};

// config

static INPUT_PORTS_START( ml102 )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Game B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Game A")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Alarm")

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, acl_button, nullptr) PORT_NAME("ACL")

	PORT_START("BA") // MCU BA(alpha) pin pulled to GND
	PORT_CONFNAME( 0x01, 0x01, "Increase Score (Cheat)")
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("B") // MCU B(beta) pin pulled to GND
	PORT_CONFNAME( 0x01, 0x01, "Infinite Lives (Cheat)")
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static MACHINE_CONFIG_START( ml102 )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SM510, XTAL_32_768kHz)
	MCFG_SM510_WRITE_SEGS_CB(WRITE16(hh_sm510_state, sm510_lcd_segment_w))
	MCFG_SM510_READ_K_CB(READ8(hh_sm510_state, input_r))
	MCFG_SM510_WRITE_S_CB(WRITE8(hh_sm510_state, input_w))
	MCFG_SM510_WRITE_R_CB(WRITE8(hh_sm510_state, piezo_r1_w))
	MCFG_SM510_READ_BA_CB(IOPORT("BA"))
	MCFG_SM510_READ_B_CB(IOPORT("B"))

	/* video hardware */
	MCFG_SCREEN_SVG_ADD("screen", "svg")
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_SIZE(1647, 1080)
	MCFG_SCREEN_VISIBLE_AREA(0, 1647-1, 0, 1080-1)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_sm510_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_svg)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Nintendo Game & Watch: Boxing (model BX-301)
  * Sharp SM511 label BX-301 287C (die label KMS73B, KMS744)
  * wide lcd screen with custom segments, 1-bit sound

  Also known as Punch-Out!! in the USA.

***************************************************************************/

class bx301_state : public hh_sm510_state
{
public:
	bx301_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_sm510_state(mconfig, type, tag)
	{
		m_inp_lines = 7;
	}
};

// config

static INPUT_PORTS_START( bx301 )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x0b, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_PLAYER(2)

	PORT_START("IN.1") // S2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S3
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_PLAYER(2)

	PORT_START("IN.3") // S4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S5
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_PLAYER(2)

	PORT_START("IN.5") // S6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.6") // S7
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Game B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Game A")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Alarm")

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, acl_button, nullptr) PORT_NAME("ACL")

	PORT_START("BA") // MCU BA(alpha) pin pulled to GND
	PORT_CONFNAME( 0x01, 0x01, "P2 Decrease Health (Cheat)")
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("B") // MCU B(beta) pin pulled to GND
	PORT_CONFNAME( 0x01, 0x01, "P1 Infinite Health (Cheat)")
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static MACHINE_CONFIG_START( bx301 )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SM511, XTAL_32_768kHz)
	MCFG_SM510_WRITE_SEGS_CB(WRITE16(hh_sm510_state, sm510_lcd_segment_w))
	MCFG_SM510_READ_K_CB(READ8(hh_sm510_state, input_r))
	MCFG_SM510_WRITE_S_CB(WRITE8(hh_sm510_state, input_w))
	MCFG_SM510_WRITE_R_CB(WRITE8(hh_sm510_state, piezo_r1_w))
	MCFG_SM510_READ_BA_CB(IOPORT("BA"))
	MCFG_SM510_READ_B_CB(IOPORT("B"))

	/* video hardware */
	MCFG_SCREEN_SVG_ADD("screen", "svg")
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_SIZE(1920, 529)
	MCFG_SCREEN_VISIBLE_AREA(0, 1920-1, 0, 529-1)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_sm510_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_svg)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Tiger Gauntlet
  * Sharp SM510 under epoxy (die label CMS54C, KMS583)
  * lcd screen with custom segments, 1-bit sound

  known releases:
  - World: Gauntlet
  - Japan: Gauntlet (published by Sega)

***************************************************************************/

class tgaunt_state : public hh_sm510_state
{
public:
	tgaunt_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_sm510_state(mconfig, type, tag)
	{ }

	virtual DECLARE_READ8_MEMBER(input_r) override;
};

// handlers

READ8_MEMBER(tgaunt_state::input_r)
{
	// K: multiplexed inputs (note: Vdd row is always on)
	return m_inp_matrix[6]->read() | read_inputs(6);
}


// config

static INPUT_PORTS_START( tgaunt )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Pick")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x0a, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x09, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // S4
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Bomb")
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Attack")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Key")
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // S6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.6") // Vdd!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Power On/Start")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Sound")

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POWER_OFF )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, acl_button, nullptr) PORT_NAME("ACL")
INPUT_PORTS_END

static MACHINE_CONFIG_START( tgaunt )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SM510, XTAL_32_768kHz)
	MCFG_SM510_WRITE_SEGS_CB(WRITE16(hh_sm510_state, sm510_lcd_segment_w))
	MCFG_SM510_READ_K_CB(READ8(tgaunt_state, input_r))
	MCFG_SM510_WRITE_S_CB(WRITE8(hh_sm510_state, input_w))
	MCFG_SM510_WRITE_R_CB(WRITE8(hh_sm510_state, piezo_r1_w))
	MCFG_SM510_READ_BA_CB(IOPORT("BA"))
	MCFG_SM510_READ_B_CB(IOPORT("B"))

	/* video hardware */
	MCFG_SCREEN_SVG_ADD("screen", "svg")
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_SIZE(1445, 1080)
	MCFG_SCREEN_VISIBLE_AREA(0, 1445-1, 0, 1080-1)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_sm510_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_svg)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Tiger Double Dragon
  * Sharp SM510 under epoxy (die label CMS54C, KMS570, KMS593)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class tddragon_state : public hh_sm510_state
{
public:
	tddragon_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_sm510_state(mconfig, type, tag)
	{ }

	virtual DECLARE_READ8_MEMBER(input_r) override;
};

// handlers

READ8_MEMBER(tddragon_state::input_r)
{
	// K: multiplexed inputs (note: Vdd row is always on)
	return m_inp_matrix[5]->read() | read_inputs(5);
}


// config

static INPUT_PORTS_START( tddragon )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) // Jump
	PORT_BIT( 0x0b, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x09, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) // Sway
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // S4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) // Punch/Pick
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) // Kick
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Status")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // Vdd!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Power On/Start")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Sound")

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POWER_OFF )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, acl_button, nullptr) PORT_NAME("ACL")
INPUT_PORTS_END

static MACHINE_CONFIG_START( tddragon )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SM510, XTAL_32_768kHz)
	MCFG_SM510_WRITE_SEGS_CB(WRITE16(hh_sm510_state, sm510_lcd_segment_w))
	MCFG_SM510_READ_K_CB(READ8(tddragon_state, input_r))
	MCFG_SM510_WRITE_S_CB(WRITE8(hh_sm510_state, input_w))
	MCFG_SM510_WRITE_R_CB(WRITE8(hh_sm510_state, piezo_r1_w))
	MCFG_SM510_READ_BA_CB(IOPORT("BA"))
	MCFG_SM510_READ_B_CB(IOPORT("B"))

	/* video hardware */
	MCFG_SCREEN_SVG_ADD("screen", "svg")
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_SIZE(1477, 1080)
	MCFG_SCREEN_VISIBLE_AREA(0, 1477-1, 0, 1080-1)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_sm510_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_svg)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Tiger Space Jam
  * Sharp SM510 under epoxy (die label KMS10, 23)
  * lcd screen with custom segments, 1-bit sound

***************************************************************************/

class tsjam_state : public hh_sm510_state
{
public:
	tsjam_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_sm510_state(mconfig, type, tag)
	{ }

	virtual DECLARE_READ8_MEMBER(input_r) override;
};

// handlers

READ8_MEMBER(tsjam_state::input_r)
{
	// K: multiplexed inputs (note: Vdd row is always on)
	return m_inp_matrix[5]->read() | read_inputs(5);
}


// config

static INPUT_PORTS_START( tsjam )
	PORT_START("IN.0") // S1
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x0b, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x09, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // S4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) // Shoot/Block
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) // Tune/Steal
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Pause")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // Vdd!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Power On/Start")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Sound")

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POWER_OFF )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, acl_button, nullptr) PORT_NAME("ACL")
INPUT_PORTS_END

static MACHINE_CONFIG_START( tsjam )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SM510, XTAL_32_768kHz) // no external XTAL
	MCFG_SM510_WRITE_SEGS_CB(WRITE16(hh_sm510_state, sm510_lcd_segment_w))
	MCFG_SM510_READ_K_CB(READ8(tsjam_state, input_r))
	MCFG_SM510_WRITE_S_CB(WRITE8(hh_sm510_state, input_w))
	MCFG_SM510_WRITE_R_CB(WRITE8(hh_sm510_state, piezo_r1_w))
	MCFG_SM510_READ_BA_CB(IOPORT("BA"))
	MCFG_SM510_READ_B_CB(IOPORT("B"))

	/* video hardware */
	MCFG_SCREEN_SVG_ADD("screen", "svg")
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_SIZE(1421, 1080)
	MCFG_SCREEN_VISIBLE_AREA(0, 1421-1, 0, 1080-1)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_sm510_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_svg)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Tiger Sonic The Hedgehog
  * Sharp SM511 under epoxy (die label KMS73B, N71)
  * lcd screen with custom segments, 2-bit sound

***************************************************************************/

class tsonic_state : public hh_sm510_state
{
public:
	tsonic_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_sm510_state(mconfig, type, tag)
	{ }

	void update_speaker();
	DECLARE_WRITE8_MEMBER(write_r);
	DECLARE_WRITE8_MEMBER(write_s);
	virtual DECLARE_READ8_MEMBER(input_r) override;
};

// handlers

void tsonic_state::update_speaker()
{
	m_speaker->level_w((m_s << 1 & 2) | (m_r & 1));
}

WRITE8_MEMBER(tsonic_state::write_r)
{
	// R1: to speaker, via 120K resistor
	m_r = data;
	update_speaker();
}

WRITE8_MEMBER(tsonic_state::write_s)
{
	// S1: to speaker, via 39K resistor
	m_s = data;
	update_speaker();

	// other: input mux
	hh_sm510_state::input_w(space, 0, data >> 1);
}

READ8_MEMBER(tsonic_state::input_r)
{
	// K: multiplexed inputs (note: Vdd row is always on)
	return m_inp_matrix[5]->read() | read_inputs(5);
}


// config

static INPUT_PORTS_START( tsonic )
	PORT_START("IN.0") // S2
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) // Jump
	PORT_BIT( 0x0b, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // S3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x09, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2") // S4
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x0d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // S5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) // Super Sonic Spin
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4") // S6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Pause")
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.5") // Vdd!
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Power On/Start")

	PORT_START("BA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_NAME("Sound")

	PORT_START("B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POWER_OFF )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, acl_button, nullptr) PORT_NAME("ACL")
INPUT_PORTS_END

static const s16 tsonic_speaker_levels[] = { 0, 0x7fff/3*1, 0x7fff/3*2, 0x7fff };

static MACHINE_CONFIG_START( tsonic )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SM511, XTAL_32_768kHz)
	MCFG_SM510_WRITE_SEGS_CB(WRITE16(hh_sm510_state, sm510_lcd_segment_w))
	MCFG_SM510_READ_K_CB(READ8(tsonic_state, input_r))
	MCFG_SM510_WRITE_S_CB(WRITE8(tsonic_state, write_s))
	MCFG_SM510_WRITE_R_CB(WRITE8(tsonic_state, write_r))
	MCFG_SM510_READ_BA_CB(IOPORT("BA"))
	MCFG_SM510_READ_B_CB(IOPORT("B"))

	/* video hardware */
	MCFG_SCREEN_SVG_ADD("screen", "svg")
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_SIZE(1523, 1080)
	MCFG_SCREEN_VISIBLE_AREA(0, 1523-1, 0, 1080-1)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_sm510_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_svg)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SPEAKER_LEVELS(4, tsonic_speaker_levels)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( kdribble )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "584", 0x0000, 0x1000, CRC(1d9022c8) SHA1(64567f9f161e830a0634d5c89917ab866c26c0f8) )

	ROM_REGION( 450127, "svg", 0)
	ROM_LOAD( "kdribble.svg", 0, 450127, CRC(e1d99a0f) SHA1(ec4c4b9ce613df06837f48872dbc65fbaa00a959) )
ROM_END


ROM_START( ktopgun ) // except for filler/unused bytes, ROM listing in patent US5137277 "BH003 Top Gun" is same
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "598", 0x0000, 0x1000, CRC(50870b35) SHA1(cda1260c2e1c180995eced04b7d7ff51616dcef5) )

	ROM_REGION( 425775, "svg", 0)
	ROM_LOAD( "ktopgun.svg", 0, 425775, CRC(414061e3) SHA1(15493f0688e80677c6d92e2b8966926e68119870) )
ROM_END


ROM_START( kcontra ) // except for filler/unused bytes, ROM listing in patent US5120057 "BH002 C (Contra)" program/melody is same
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "773.program", 0x0000, 0x1000, CRC(bf834877) SHA1(055dd56ec16d63afba61ab866481fd9c029fb54d) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "773.melody", 0x000, 0x100, CRC(23d02b99) SHA1(703938e496db0eeacd14fe7605d4b5c39e0a5bc8) )

	ROM_REGION( 719746, "svg", 0)
	ROM_LOAD( "kcontra.svg", 0, 719746, CRC(48beb6ec) SHA1(abbc95c31b5f0bfec237ff518790a70f8102595c) )
ROM_END


ROM_START( ktmnt ) // except for filler/unused bytes, ROM listing in patent US5150899 "BH005 TMNT" program/melody is same
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "774.program", 0x0000, 0x1000, CRC(a1064f87) SHA1(92156c35fbbb414007ee6804fe635128a741d5f1) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "774.melody", 0x000, 0x100, CRC(8270d626) SHA1(bd91ca1d5cd7e2a62eef05c0033b19dcdbe441ca) )

	ROM_REGION( 610088, "svg", 0)
	ROM_LOAD( "ktmnt.svg", 0, 610088, CRC(97e168ad) SHA1(ec5a3820404f291386861deb4238b69bb0f57fd6) )
ROM_END


ROM_START( kgradius )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "771.program", 0x0000, 0x1000, CRC(830c2afc) SHA1(bb9ebd4e52831cc02cd92dd4b37675f34cf37b8c) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "771.melody", 0x000, 0x100, CRC(4c586b73) SHA1(14c5ab2898013a577f678970a648c374749cc66d) )

	ROM_REGION( 637904, "svg", 0)
	ROM_LOAD( "kgradius.svg", 0, 637904, CRC(793791f5) SHA1(ac37c5accc79fd6275024a519ff92d47a017c752) )
ROM_END


ROM_START( kloneran )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "781.program", 0x0000, 0x1000, CRC(52b9735f) SHA1(06c5ef6e7e781b1176d4c1f2445f765ccf18b3f7) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "781.melody", 0x000, 0x100, CRC(a393de36) SHA1(55089f04833ccb318524ab2b584c4817505f4019) )

	ROM_REGION( 632924, "svg", 0)
	ROM_LOAD( "kloneran.svg", 0, 632924, CRC(4e7312ab) SHA1(c42db017e5ef3629bb7edbb420f8b43bd2251f39) )
ROM_END


ROM_START( kblades )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "782.program", 0x0000, 0x1000, CRC(3351a35d) SHA1(84c64b65d3cabfa20c18f4649c9ede2578b82523) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "782.melody", 0x000, 0x100, CRC(e8bf48ba) SHA1(3852c014dc9136566322b4f9e2aab0e3ec3a7387) )

	ROM_REGION( 454923, "svg", 0)
	ROM_LOAD( "kblades.svg", 0, 454923, CRC(933c6627) SHA1(d463d34141a40a7003b0e252c08e27a6a6a1d14e) )
ROM_END


ROM_START( knfl )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "786.program", 0x0000, 0x1000, CRC(0535c565) SHA1(44cdcd284713ff0b194b24beff9f1b94c8bc63b2) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "786.melody", 0x000, 0x100, CRC(6c80263b) SHA1(d3c21e2f8491fef101907b8e0871b1e1c1ed58f5) )

	ROM_REGION( 570905, "svg", 0)
	ROM_LOAD( "knfl.svg", 0, 570905, CRC(c1079dc0) SHA1(0d5e0daff734dd46a24735dd3d3899286a13144b) )
ROM_END


ROM_START( kbilly )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "788.program", 0x0000, 0x1000, CRC(b8b1f734) SHA1(619dd527187b43276d081cdb1b13e0a9a81f2c6a) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "788.melody", 0x000, 0x100, CRC(cd488bea) SHA1(8fc60081f46e392978d6950c74711fb7ebd154de) )

	ROM_REGION( 598098, "svg", 0)
	ROM_LOAD( "kbilly.svg", 0, 598098, CRC(370cb107) SHA1(97abd0cce3ca7e6722dced256aed5afe766411d1) )
ROM_END


ROM_START( kbucky )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "n58.program", 0x0000, 0x1000, CRC(7c36a0c4) SHA1(1b55ac64a71af746fd0a0f44266fcc92cca77482) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "n58.melody", 0x000, 0x100, CRC(7e99e469) SHA1(3e9a3843c6ab392f5989f3366df87a2d26cb8620) )

	ROM_REGION( 727660, "svg", 0)
	ROM_LOAD( "kbucky.svg", 0, 727660, CRC(e2b819c6) SHA1(f20df397f869d4dfebf219196f71774a5dd9a7a0) )
ROM_END


ROM_START( kgarfld )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "n62.program", 0x0000, 0x1000, CRC(5a762049) SHA1(26d4d891160d254dfd752734e1047126243f88dd) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "n62.melody", 0x000, 0x100, CRC(232b7d55) SHA1(76f6a19e8182ee3f00c9f4ef007b5dde75a9c00d) )

	ROM_REGION( 580911, "svg", 0)
	ROM_LOAD( "kgarfld.svg", 0, 580911, CRC(dff81825) SHA1(01a1596b832752ff86ac2dbad09119cbb114737b) )
ROM_END


ROM_START( gnw_mc25 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mc-25", 0x0000, 0x0740, BAD_DUMP CRC(cb820c32) SHA1(7e94fc255f32db725d5aa9e196088e490c1a1443) ) // dumped from Soviet clone

	ROM_REGION( 102453, "svg", 0)
	ROM_LOAD( "gnw_mc25.svg", 0, 102453, CRC(88cc7c49) SHA1(c000d51d1b99750116b97f9bafc0314ea506366d) )
ROM_END

ROM_START( gnw_eg26 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "eg-26", 0x0000, 0x0740, BAD_DUMP CRC(cb820c32) SHA1(7e94fc255f32db725d5aa9e196088e490c1a1443) ) // dumped from Soviet clone

	ROM_REGION( 102848, "svg", 0)
	ROM_LOAD( "gnw_eg26.svg", 0, 102848, CRC(742c2605) SHA1(984d430ad2ff47ad7a3f9b25b7d3f3d51b10cca5) )
ROM_END

ROM_START( nupogodi )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "nupogodi.bin", 0x0000, 0x0740, CRC(cb820c32) SHA1(7e94fc255f32db725d5aa9e196088e490c1a1443) )

	ROM_REGION( 202839, "svg", 0)
	ROM_LOAD( "nupogodi.svg", 0, 202839, CRC(4c8a38ce) SHA1(cdb5cbbef0f71584d89a5acfea73dd21a72d2318) )
ROM_END

ROM_START( exospace )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "exospace.bin", 0x0000, 0x0740, CRC(553e2b09) SHA1(2b74f8437b881fbb62b61f25435a5bfc66872a9a) )

	ROM_REGION( 66790, "svg", 0)
	ROM_LOAD( "exospace.svg", 0, 66790, CRC(df31043a) SHA1(2d8caf42894df699e469652e5f448beaebbcc1ae) )
ROM_END


ROM_START( gnw_dm53 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "dm-53_565", 0x0000, 0x1000, CRC(e21fc0f5) SHA1(3b65ccf9f98813319410414e11a3231b787cdee6) )

	ROM_REGION( 207524, "svg_top", 0)
	ROM_LOAD( "gnw_dm53_top.svg", 0, 207524, CRC(07a19adb) SHA1(605b73d79639bbe6a2e88e3186d677ad0e0a5a86) )

	ROM_REGION( 227954, "svg_bottom", 0)
	ROM_LOAD( "gnw_dm53_bottom.svg", 0, 227954, CRC(906121e9) SHA1(1319226f9259cc179e2336308e1ab279d6b4097e) )
ROM_END


ROM_START( gnw_jr55 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "jr-55_560", 0x0000, 0x1000, CRC(46aed0ae) SHA1(72f75ccbd84aea094148c872fc7cc1683619a18a) )

	ROM_REGION( 264099, "svg_top", 0)
	ROM_LOAD( "gnw_jr55_top.svg", 0, 264099, CRC(0d2f6d9e) SHA1(9a6b0b453dd66e31c48799f1712fe3ea435331af) )

	ROM_REGION( 391063, "svg_bottom", 0)
	ROM_LOAD( "gnw_jr55_bottom.svg", 0, 391063, CRC(8214db21) SHA1(e7bb53193bdebd4bba69e399b9633c7dd5884d04) )
ROM_END


ROM_START( gnw_mw56 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mw-56", 0x0000, 0x1000, CRC(385e59da) SHA1(2f79281bdf2f2afca2fb5bd7b9a3beeffc9c4eb7) )

	ROM_REGION( 172381, "svg_left", 0)
	ROM_LOAD( "gnw_mw56_left.svg", 0, 172381, CRC(6000f1c2) SHA1(7395d89d62cc77f59b9ce12c200cd6748f287f9d) )

	ROM_REGION( 229866, "svg_right", 0)
	ROM_LOAD( "gnw_mw56_right.svg", 0, 229866, CRC(7c58f0c2) SHA1(6dd975ecd52ab6fc436b671b6a31007f94628e3d) )
ROM_END


ROM_START( gnw_dj101 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "dj-101", 0x0000, 0x1000, CRC(8dcfb5d1) SHA1(e0ef578e9362eb9a3cab631376df3cf55978f2de) )

	ROM_REGION( 280871, "svg", 0)
	ROM_LOAD( "gnw_dj101.svg", 0, 280871, CRC(e75adb36) SHA1(2a9854fabb8efab9e11f1f661a09c2cc30de28af) )
ROM_END


ROM_START( gnw_ml102 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "ml-102_577", 0x0000, 0x1000, CRC(c1128dea) SHA1(8647e36f43a0e37756a3c7b6a3f08d4c8243f1cc) )

	ROM_REGION( 361742, "svg", 0)
	ROM_LOAD( "gnw_ml102.svg", 0, 361742, CRC(a9fe2c05) SHA1(ec16081a7444cccd7fa90fda9a94dbcb037c5c67) )
ROM_END


ROM_START( gnw_bx301 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "bx-301_744.program", 0x0000, 0x1000, CRC(0fdf0303) SHA1(0b791c9d4874e9534d0a9b7a8968ce02fe4bee96) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "bx-301_744.melody", 0x000, 0x100, CRC(439d943d) SHA1(52880df15ec7513f96482f455ef3d9778aa24750) )

	ROM_REGION( 265213, "svg", 0)
	ROM_LOAD( "gnw_bx301.svg", 0, 265213, CRC(90514ce3) SHA1(544b6ffbcf04e847fef8723b603f46769a6a09e4) )
ROM_END


ROM_START( tgaunt )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "583", 0x0000, 0x1000, CRC(598d8156) SHA1(9f776e8b9b4321e8118481e6b1304f8a38f9932e) )

	ROM_REGION( 704880, "svg", 0)
	ROM_LOAD( "tgaunt.svg", 0, 704880, CRC(4d744da9) SHA1(487704f815ce2f7b091a35bd7222b48e1426c0f2) )
ROM_END


ROM_START( tddragon )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "593", 0x0000, 0x1000, CRC(2642f778) SHA1(fee77acf93e057a8b4627389dfd481c6d9cbd02b) )

	ROM_REGION( 511413, "svg", 0)
	ROM_LOAD( "tddragon.svg", 0, 511413, CRC(a20e0cba) SHA1(dca222a0c78a0f05c7f1269f102a371802888a5e) )
ROM_END


ROM_START( tsonic )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "n71.program", 0x0000, 0x1000, CRC(44cafd68) SHA1(bf8d0ab88d153fabc688ffec19959209ca79c3db) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "n71.melody", 0x000, 0x100, CRC(bae258c8) SHA1(81cb75d73fab4479cd92fcb13d9cb03cec2afdd5) )

	ROM_REGION( 541278, "svg", 0)
	ROM_LOAD( "tsonic.svg", 0, 541278, CRC(afdfd041) SHA1(57ad458fbaf9e66f0f394a7ddf63dc3bcc4a932c) )
ROM_END


ROM_START( tsjam )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "10_23", 0x0000, 0x1000, CRC(6eaabfbd) SHA1(f0ecbd6f65fe72ce2d8a452685be2e77a63fc9f0) )

	ROM_REGION( 1044640, "svg", 0)
	ROM_LOAD( "tsjam.svg", 0, 1044640, CRC(4e33adee) SHA1(220bab1e2921032e254f3f8bdf6916a84abb6de4) )
ROM_END



//    YEAR  NAME       PARENT  COMP MACHINE    INPUT      STATE        INIT  COMPANY, FULLNAME, FLAGS
CONS( 1989, kdribble,  0,        0, kdribble,  kdribble,  kdribble_state, 0, "Konami", "Double Dribble (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1989, ktopgun,   0,        0, ktopgun,   ktopgun,   ktopgun_state,  0, "Konami", "Top Gun (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1989, kcontra,   0,        0, kcontra,   kcontra,   kcontra_state,  0, "Konami", "Contra (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1989, ktmnt,     0,        0, ktmnt,     ktmnt,     ktmnt_state,    0, "Konami", "Teenage Mutant Ninja Turtles (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1989, kgradius,  0,        0, kgradius,  kgradius,  kgradius_state, 0, "Konami", "Gradius (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1989, kloneran,  0,        0, kloneran,  kloneran,  kloneran_state, 0, "Konami", "Lone Ranger (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1989, kblades,   0,        0, kblades,   kblades,   kblades_state,  0, "Konami", "Blades of Steel (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1989, knfl,      0,        0, knfl,      knfl,      knfl_state,     0, "Konami", "NFL Football (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1989, kbilly,    0,        0, kbilly,    kbilly,    kbilly_state,   0, "Konami", "The Adventures of Bayou Billy (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1991, kbucky,    0,        0, kbucky,    kbucky,    kbucky_state,   0, "Konami", "Bucky O'Hare (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1991, kgarfld,   0,        0, kgarfld,   kgarfld,   kgarfld_state,  0, "Konami", "Garfield (handheld)", MACHINE_SUPPORTS_SAVE )

CONS( 1981, gnw_mc25,  0,        0, mc25,      mc25,      mc25_state,     0, "Nintendo", "Game & Watch: Mickey Mouse", MACHINE_SUPPORTS_SAVE )
CONS( 1981, gnw_eg26,  gnw_mc25, 0, eg26,      mc25,      mc25_state,     0, "Nintendo", "Game & Watch: Egg", MACHINE_SUPPORTS_SAVE )
CONS( 1984, nupogodi,  gnw_mc25, 0, nupogodi,  mc25,      mc25_state,     0, "Elektronika", "Nu, pogodi!", MACHINE_SUPPORTS_SAVE )
CONS( 1989, exospace,  gnw_mc25, 0, exospace,  exospace,  mc25_state,     0, "Elektronika", "Explorers of Space", MACHINE_SUPPORTS_SAVE )

CONS( 1982, gnw_dm53,  0,        0, dm53,      dm53,      dm53_state,     0, "Nintendo", "Game & Watch: Mickey & Donald", MACHINE_SUPPORTS_SAVE )
CONS( 1983, gnw_jr55,  0,        0, jr55,      jr55,      jr55_state,     0, "Nintendo", "Game & Watch: Donkey Kong II", MACHINE_SUPPORTS_SAVE )
CONS( 1983, gnw_mw56,  0,        0, mw56,      mw56,      mw56_state,     0, "Nintendo", "Game & Watch: Mario Bros.", MACHINE_SUPPORTS_SAVE )

CONS( 1982, gnw_dj101, 0,        0, dj101,     dj101,     dj101_state,    0, "Nintendo", "Game & Watch: Donkey Kong Jr. (new wide screen)", MACHINE_SUPPORTS_SAVE )
CONS( 1983, gnw_ml102, 0,        0, ml102,     ml102,     ml102_state,    0, "Nintendo", "Game & Watch: Mario's Cement Factory (new wide screen)", MACHINE_SUPPORTS_SAVE )

CONS( 1984, gnw_bx301, 0,        0, bx301,     bx301,     bx301_state,    0, "Nintendo", "Game & Watch: Boxing", MACHINE_SUPPORTS_SAVE )

CONS( 1988, tgaunt,    0,        0, tgaunt,    tgaunt,    tgaunt_state,   0, "Tiger Electronics (licensed from Tengen)", "Gauntlet (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1988, tddragon,  0,        0, tddragon,  tddragon,  tddragon_state, 0, "Tiger Electronics (licensed from Tradewest/Technos)", "Double Dragon (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1991, tsonic,    0,        0, tsonic,    tsonic,    tsonic_state,   0, "Tiger Electronics (licensed from Sega)", "Sonic The Hedgehog (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1996, tsjam,     0,        0, tsjam,     tsjam,     tsjam_state,    0, "Tiger Electronics", "Space Jam (handheld)", MACHINE_SUPPORTS_SAVE )
