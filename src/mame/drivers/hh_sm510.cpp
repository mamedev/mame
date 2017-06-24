// license:BSD-3-Clause
// copyright-holders:hap, Sean Riddle
// thanks-to:Igor
/***************************************************************************

  Sharp SM5xx family handhelds.

  TODO:
  - gnw_mc25 emulation is preliminary
  - confirm gnw_mc25 romdump
  - improve svg layout for gnw_jr55, gnw_mw56
  - svg lcd screen background/foreground (not supported in core),
    or should it be for external artwork only?
  - flickering segments, lcd physics? or screen update timing?
    * ktmnt: "LIMIT" segment when going underwater on level 1
    * gnw_dj101: cage and clock before you start the game
    * gnw_mw56: truck boxes while playing

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
#include "hh_sm500_test.lh" // "


class hh_sm510_state : public driver_device
{
public:
	hh_sm510_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_inp_matrix(*this, "IN.%u", 0),
		m_speaker(*this, "speaker"),
		m_inp_lines(0)
	{ }

	// devices
	required_device<cpu_device> m_maincpu;
	optional_ioport_array<7> m_inp_matrix; // max 7
	optional_device<speaker_sound_device> m_speaker;

	// misc common
	u16 m_inp_mux;                 // multiplexed inputs mask
	int m_inp_lines;               // number of input mux columns
	u8 m_lcd_output_cache[0x100];

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

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
};


// machine start/reset

void hh_sm510_state::machine_start()
{
	// zerofill
	m_inp_mux = 0;
	/* m_inp_lines = 0; */ // not here
	memset(m_lcd_output_cache, ~0, sizeof(m_lcd_output_cache));

	// register for savestates
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_inp_lines));
	/* save_item(NAME(m_lcd_output_cache)); */ // don't save!
}

void hh_sm510_state::machine_reset()
{
}



/***************************************************************************

  Helper Functions

***************************************************************************/

// lcd panel - on lcd handhelds, usually not a generic x/y screen device

WRITE16_MEMBER(hh_sm510_state::sm510_lcd_segment_w)
{
	for (int seg = 0; seg < 0x10; seg++)
	{
		int index = offset << 4 | seg;
		u8 state = data >> seg & 1;

		if (state != m_lcd_output_cache[index])
		{
			// output to row.seg.H, where:
			// row = row a/b/bs/c (0/1/2/3)
			// seg = seg 1-16 (0-15)
			// H = H1-H4 (0-3)
			char buf[0x10];
			sprintf(buf, "%d.%d.%d", offset >> 2, seg, offset & 3);
			output().set_value(buf, state);

			m_lcd_output_cache[index] = state;
		}
	}
}

WRITE8_MEMBER(hh_sm510_state::sm500_lcd_segment_w)
{
	for (int seg = 0; seg < 4; seg++)
	{
		int index = offset << 2 | seg;
		u8 state = data >> seg & 1;

		if (state != m_lcd_output_cache[index])
		{
			// output to row.seg.H, where:
			// row = O group (0-*)
			// seg = O data (0-3)
			// H = H1/H2 (0/1)
			char buf[0x10];
			sprintf(buf, "%d.%d.%d", offset & 0xf, seg, offset >> 4);
			output().set_value(buf, state);

			m_lcd_output_cache[index] = state;
		}
	}
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
	m_maincpu->set_input_line(SM510_INPUT_LINE_K, read_inputs(m_inp_lines) ? ASSERT_LINE : CLEAR_LINE);
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

  Konami Top Gun
  * PCB label BH003
  * Sharp SM510 under epoxy (die label CMS54C, KMS598)

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
	PORT_START("IN.0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)

	PORT_START("IN.1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_VOLUME_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Sound")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)

	PORT_START("IN.2")
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
	PORT_START("IN.0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)

	PORT_START("IN.1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_VOLUME_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Sound")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)

	PORT_START("IN.2")
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
	PORT_START("IN.0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)

	PORT_START("IN.1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_VOLUME_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Sound")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Power On/Start")

	PORT_START("IN.2")
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
	PORT_START("IN.0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Power On/Start")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_VOLUME_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Sound")

	PORT_START("IN.1")
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
	PORT_START("IN.0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)

	PORT_START("IN.1")
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
	MCFG_DEFAULT_LAYOUT(layout_svg)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Nintendo Game & Watch: Mickey Mouse (model MC-25), Egg (model EG-26)
  * Sharp SM5A label ?
  
  MC-25 and EG-26 are the same game, it's assumed that the latter was for
  regions where Nintendo wasn't able to license from Disney.
  
  In 1984, Elektronika(USSR) released a clone, Nu, Pogodi! This was followed
  by several other games that were the same under the hood, only differing
  in graphics.
  
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
	PORT_START("IN.0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)

	PORT_START("IN.1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)

	PORT_START("IN.2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) // game b
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) // game a
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, acl_button, nullptr) PORT_NAME("ACL")
INPUT_PORTS_END

static MACHINE_CONFIG_START( mc25 )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SM5A, XTAL_32_768kHz)
	MCFG_SM500_WRITE_O_CB(WRITE8(hh_sm510_state, sm500_lcd_segment_w))
	MCFG_SM510_READ_K_CB(READ8(hh_sm510_state, input_r))
	MCFG_SM510_WRITE_R_CB(WRITE8(hh_sm510_state, piezo_input_w))

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_hh_sm500_test)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Nintendo Game & Watch: Mickey & Donald (model DM-53)
  * PCB label DM-53
  * Sharp SM510 label DM-53 52ZC (die label CMS54C, CMS565)

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
	PORT_START("IN.0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)

	PORT_START("IN.1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Game B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Game A")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Alarm")

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, acl_button, nullptr) PORT_NAME("ACL")
INPUT_PORTS_END

static MACHINE_CONFIG_START( dm53 )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SM510, XTAL_32_768kHz)
	MCFG_SM510_WRITE_SEGS_CB(WRITE16(hh_sm510_state, sm510_lcd_segment_w))
	MCFG_SM510_READ_K_CB(READ8(hh_sm510_state, input_r))
	MCFG_SM510_WRITE_S_CB(WRITE8(hh_sm510_state, input_w))
	MCFG_SM510_WRITE_R_CB(WRITE8(hh_sm510_state, piezo_r2_w))

	/* video hardware */
	MCFG_SCREEN_SVG_ADD("screen_top", "svg_top")
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_SIZE(1920/2, 1238/2)
	MCFG_SCREEN_VISIBLE_AREA(0, 1920/2-1, 0, 1238/2-1)

	MCFG_SCREEN_SVG_ADD("screen_bottom", "svg_bottom")
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_SIZE(1920/2, 1219/2)
	MCFG_SCREEN_VISIBLE_AREA(0, 1920/2-1, 0, 1219/2-1)

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
	PORT_START("IN.0")
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)

	PORT_START("IN.1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)

	PORT_START("IN.2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Game B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Game A")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Alarm")

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, acl_button, nullptr) PORT_NAME("ACL")
INPUT_PORTS_END

static MACHINE_CONFIG_START( jr55 )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SM510, XTAL_32_768kHz)
	MCFG_SM510_WRITE_SEGS_CB(WRITE16(hh_sm510_state, sm510_lcd_segment_w))
	MCFG_SM510_READ_K_CB(READ8(hh_sm510_state, input_r))
	MCFG_SM510_WRITE_S_CB(WRITE8(hh_sm510_state, input_w))
	MCFG_SM510_WRITE_R_CB(WRITE8(hh_sm510_state, piezo_r1_w))

	/* video hardware */
	MCFG_SCREEN_SVG_ADD("screen_top", "svg_top")
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_SIZE(1920/2, 1225/2)
	MCFG_SCREEN_VISIBLE_AREA(0, 1920/2-1, 0, 1225/2-1)

	MCFG_SCREEN_SVG_ADD("screen_bottom", "svg_bottom")
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_SIZE(1920/2, 1261/2)
	MCFG_SCREEN_VISIBLE_AREA(0, 1920/2-1, 0, 1261/2-1)

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
	PORT_START("IN.0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)

	PORT_START("IN.1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Game B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Game A")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Alarm")

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, acl_button, nullptr) PORT_NAME("ACL")
INPUT_PORTS_END

static MACHINE_CONFIG_START( mw56 )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SM510, XTAL_32_768kHz)
	MCFG_SM510_WRITE_SEGS_CB(WRITE16(hh_sm510_state, sm510_lcd_segment_w))
	MCFG_SM510_READ_K_CB(READ8(hh_sm510_state, input_r))
	MCFG_SM510_WRITE_S_CB(WRITE8(hh_sm510_state, input_w))
	MCFG_SM510_WRITE_R_CB(WRITE8(hh_sm510_state, piezo_r1_w))

	/* video hardware */
	MCFG_SCREEN_SVG_ADD("screen_left", "svg_left")
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_SIZE(2087/2, 1440/2)
	MCFG_SCREEN_VISIBLE_AREA(0, 2087/2-1, 0, 1440/2-1)

	MCFG_SCREEN_SVG_ADD("screen_right", "svg_right")
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_SIZE(2079/2, 1440/2)
	MCFG_SCREEN_VISIBLE_AREA(0, 2079/2-1, 0, 1440/2-1)

	MCFG_DEFAULT_LAYOUT(layout_gnw_dualh)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Nintendo Game & Watch: Donkey Kong Jr. (model DJ-101)
  * Sharp SM510 label DJ-101 52ZA (no decap)
  
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
	PORT_START("IN.0")
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)

	PORT_START("IN.1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)

	PORT_START("IN.2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Game B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Game A")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Alarm")

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, acl_button, nullptr) PORT_NAME("ACL")
INPUT_PORTS_END

static MACHINE_CONFIG_START( dj101 )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SM510, XTAL_32_768kHz)
	MCFG_SM510_WRITE_SEGS_CB(WRITE16(hh_sm510_state, sm510_lcd_segment_w))
	MCFG_SM510_READ_K_CB(READ8(hh_sm510_state, input_r))
	MCFG_SM510_WRITE_S_CB(WRITE8(hh_sm510_state, input_w))
	MCFG_SM510_WRITE_R_CB(WRITE8(hh_sm510_state, piezo_r1_w))

	/* video hardware */
	MCFG_SCREEN_SVG_ADD("screen", "svg")
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_SIZE(1665, 1080)
	MCFG_SCREEN_VISIBLE_AREA(0, 1665-1, 0, 1080-1)
	MCFG_DEFAULT_LAYOUT(layout_svg)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Nintendo Game & Watch: Mario's Cement Factory (model ML-102)
  * Sharp SM510 label ML-102 298D (die label CMS54C, KMS577)
  
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
	PORT_START("IN.0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Game B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Game A")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Alarm")

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, acl_button, nullptr) PORT_NAME("ACL")
INPUT_PORTS_END

static MACHINE_CONFIG_START( ml102 )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SM510, XTAL_32_768kHz)
	MCFG_SM510_WRITE_SEGS_CB(WRITE16(hh_sm510_state, sm510_lcd_segment_w))
	MCFG_SM510_READ_K_CB(READ8(hh_sm510_state, input_r))
	MCFG_SM510_WRITE_S_CB(WRITE8(hh_sm510_state, input_w))
	MCFG_SM510_WRITE_R_CB(WRITE8(hh_sm510_state, piezo_r1_w))

	/* video hardware */
	MCFG_SCREEN_SVG_ADD("screen", "svg")
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_SIZE(1759, 1080)
	MCFG_SCREEN_VISIBLE_AREA(0, 1759-1, 0, 1080-1)
	MCFG_DEFAULT_LAYOUT(layout_svg)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Nintendo Game & Watch: Boxing (model BX-301)
  * Sharp SM511 label BX-301 287C (die label KMS73B, KMS744)

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
	PORT_START("IN.0")
	PORT_BIT( 0x0b, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_PLAYER(2)

	PORT_START("IN.1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.2")
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_PLAYER(2)

	PORT_START("IN.3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.4")
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_PLAYER(2)

	PORT_START("IN.5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr)
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Time")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Game B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Game A")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, nullptr) PORT_NAME("Alarm")

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, acl_button, nullptr) PORT_NAME("ACL")
INPUT_PORTS_END

static MACHINE_CONFIG_START( bx301 )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SM511, XTAL_32_768kHz)
	MCFG_SM510_WRITE_SEGS_CB(WRITE16(hh_sm510_state, sm510_lcd_segment_w))
	MCFG_SM510_READ_K_CB(READ8(hh_sm510_state, input_r))
	MCFG_SM510_WRITE_S_CB(WRITE8(hh_sm510_state, input_w))
	MCFG_SM510_WRITE_R_CB(WRITE8(hh_sm510_state, piezo_r1_w))

	/* video hardware */
	MCFG_SCREEN_SVG_ADD("screen", "svg")
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_SIZE(1920, 529)
	MCFG_SCREEN_VISIBLE_AREA(0, 1920-1, 0, 529-1)
	MCFG_DEFAULT_LAYOUT(layout_svg)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( ktopgun ) // except for filler/unused bytes, ROM listing in patent US5137277 "BH003 Top Gun" is same
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "cms54c_kms598", 0x0000, 0x1000, CRC(50870b35) SHA1(cda1260c2e1c180995eced04b7d7ff51616dcef5) )

	ROM_REGION( 423317, "svg", 0)
	ROM_LOAD( "ktopgun.svg", 0, 423317, CRC(1e341717) SHA1(74f4ae3fa0e4aacfda76d46753a5a06f115d221f) )
ROM_END


ROM_START( kcontra ) // except for filler/unused bytes, ROM listing in patent US5120057 "BH002 C (Contra)" program/melody is same
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "kms73b_kms773.program", 0x0000, 0x1000, CRC(bf834877) SHA1(055dd56ec16d63afba61ab866481fd9c029fb54d) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "kms73b_kms773.melody", 0x000, 0x100, CRC(23d02b99) SHA1(703938e496db0eeacd14fe7605d4b5c39e0a5bc8) )

	ROM_REGION( 710430, "svg", 0)
	ROM_LOAD( "kcontra.svg", 0, 710430, CRC(66cfc3a2) SHA1(bd38d62bb14321dfec2f99c1cd9346fb5f1fd856) )
ROM_END


ROM_START( ktmnt ) // except for filler/unused bytes, ROM listing in patent US5150899 "BH005 TMNT" program/melody is same
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "kms73b_kms774.program", 0x0000, 0x1000, CRC(a1064f87) SHA1(92156c35fbbb414007ee6804fe635128a741d5f1) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "kms73b_kms774.melody", 0x000, 0x100, CRC(8270d626) SHA1(bd91ca1d5cd7e2a62eef05c0033b19dcdbe441ca) )

	ROM_REGION( 607424, "svg", 0)
	ROM_LOAD( "ktmnt.svg", 0, 607424, CRC(54ce0f2e) SHA1(1cd2d4c3026e8693f234ddfbbbe5f24311e5981d) )
ROM_END


ROM_START( kgradius )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "kms73b_kms771.program", 0x0000, 0x1000, CRC(830c2afc) SHA1(bb9ebd4e52831cc02cd92dd4b37675f34cf37b8c) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "kms73b_kms771.melody", 0x000, 0x100, CRC(4c586b73) SHA1(14c5ab2898013a577f678970a648c374749cc66d) )

	ROM_REGION( 628695, "svg", 0)
	ROM_LOAD( "kgradius.svg", 0, 628695, CRC(56ac8ee8) SHA1(c47190e7aaebbe84ed1ad55a8e88f5ebb18f939b) )
ROM_END


ROM_START( kloneran )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "kms73b_kms781.program", 0x0000, 0x1000, CRC(52b9735f) SHA1(06c5ef6e7e781b1176d4c1f2445f765ccf18b3f7) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "kms73b_kms781.melody", 0x000, 0x100, CRC(a393de36) SHA1(55089f04833ccb318524ab2b584c4817505f4019) )

	ROM_REGION( 630184, "svg", 0)
	ROM_LOAD( "kloneran.svg", 0, 630184, CRC(9b254520) SHA1(c9c85df44cc16f59f25df418b2e1aeba9f2f470c) )
ROM_END


ROM_START( gnw_mc25 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mc-25", 0x0000, 0x0740, BAD_DUMP CRC(cb820c32) SHA1(7e94fc255f32db725d5aa9e196088e490c1a1443) ) // dumped from Soviet clone

	ROM_REGION( 100000, "svg", 0)
	ROM_LOAD( "gnw_mc25.svg", 0, 100000, NO_DUMP )
ROM_END


ROM_START( gnw_dm53 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "dm-53_cms54c_cms565", 0x0000, 0x1000, CRC(e21fc0f5) SHA1(3b65ccf9f98813319410414e11a3231b787cdee6) )

	ROM_REGION( 124249, "svg_top", 0)
	ROM_LOAD( "gnw_dm53_top.svg", 0, 124249, CRC(d85ec1cc) SHA1(19828945c09defa3efc47eea8fa6128a0e5e9a2a) )

	ROM_REGION( 108485, "svg_bottom", 0)
	ROM_LOAD( "gnw_dm53_bottom.svg", 0, 108485, CRC(c5649701) SHA1(b5bd436015927503a8e647a5d02bd9c87acd1e55) )
ROM_END


ROM_START( gnw_jr55 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "jr-55_cms54c_kms560", 0x0000, 0x1000, CRC(46aed0ae) SHA1(72f75ccbd84aea094148c872fc7cc1683619a18a) )

	ROM_REGION( 207510, "svg_top", 0)
	ROM_LOAD( "gnw_jr55_top.svg", 0, 207510, CRC(598cc4b1) SHA1(ec485b73b8dcd521c236803d50c80203e0f29330) )

	ROM_REGION( 200145, "svg_bottom", 0)
	ROM_LOAD( "gnw_jr55_bottom.svg", 0, 200145, CRC(07285db4) SHA1(49e1fc394750457cab0fb3297b12957af685ac2d) )
ROM_END


ROM_START( gnw_mw56 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mw-56", 0x0000, 0x1000, CRC(385e59da) SHA1(2f79281bdf2f2afca2fb5bd7b9a3beeffc9c4eb7) )

	ROM_REGION( 100251, "svg_left", 0)
	ROM_LOAD( "gnw_mw56_left.svg", 0, 100251, CRC(316b2516) SHA1(41b5a1e9ceb919bc8b0b1dfb0f8f21cda6fee665) )

	ROM_REGION( 85767, "svg_right", 0)
	ROM_LOAD( "gnw_mw56_right.svg", 0, 85767, CRC(8210e6e0) SHA1(d42d47cea5a5a4d6c979ba7b06b646320a458290) )
ROM_END


ROM_START( gnw_dj101 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "dj-101", 0x0000, 0x1000, CRC(8dcfb5d1) SHA1(e0ef578e9362eb9a3cab631376df3cf55978f2de) )

	ROM_REGION( 277252, "svg", 0)
	ROM_LOAD( "gnw_dj101.svg", 0, 277252, CRC(d7a3832d) SHA1(59e603d9a3ba503ebbf94574d6544cb5095a0df4) )
ROM_END


ROM_START( gnw_ml102 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "ml-102_cms54c_kms577", 0x0000, 0x1000, CRC(c1128dea) SHA1(8647e36f43a0e37756a3c7b6a3f08d4c8243f1cc) )

	ROM_REGION( 300571, "svg", 0)
	ROM_LOAD( "gnw_ml102.svg", 0, 300571, CRC(8d8be5a9) SHA1(ee3a761ee956b86c064c83a15e4b833f83c23520) )
ROM_END


ROM_START( gnw_bx301 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "bx-301_kms73b_kms744.program", 0x0000, 0x1000, CRC(0fdf0303) SHA1(0b791c9d4874e9534d0a9b7a8968ce02fe4bee96) )

	ROM_REGION( 0x100, "maincpu:melody", 0 )
	ROM_LOAD( "bx-301_kms73b_kms744.melody", 0x000, 0x100, CRC(439d943d) SHA1(52880df15ec7513f96482f455ef3d9778aa24750) )

	ROM_REGION( 258514, "svg", 0)
	ROM_LOAD( "gnw_bx301.svg", 0, 258514, CRC(7b251101) SHA1(002e41374517f1fd8cecd66a2b2338aac736f319) )
ROM_END



//    YEAR  NAME       PARENT CMP MACHINE    INPUT      STATE        INIT  COMPANY, FULLNAME, FLAGS
CONS( 1989, ktopgun,   0,      0, ktopgun,   ktopgun,   ktopgun_state,  0, "Konami", "Top Gun (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1989, kcontra,   0,      0, kcontra,   kcontra,   kcontra_state,  0, "Konami", "Contra (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1989, ktmnt,     0,      0, ktmnt,     ktmnt,     ktmnt_state,    0, "Konami", "Teenage Mutant Ninja Turtles (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1989, kgradius,  0,      0, kgradius,  kgradius,  kgradius_state, 0, "Konami", "Gradius (handheld)", MACHINE_SUPPORTS_SAVE )
CONS( 1989, kloneran,  0,      0, kloneran,  kloneran,  kloneran_state, 0, "Konami", "Lone Ranger (handheld)", MACHINE_SUPPORTS_SAVE )

CONS( 1981, gnw_mc25,  0,      0, mc25,      mc25,      mc25_state,     0, "Nintendo", "Game & Watch: Mickey Mouse", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
CONS( 1982, gnw_dm53,  0,      0, dm53,      dm53,      dm53_state,     0, "Nintendo", "Game & Watch: Mickey & Donald", MACHINE_SUPPORTS_SAVE )
CONS( 1983, gnw_jr55,  0,      0, jr55,      jr55,      jr55_state,     0, "Nintendo", "Game & Watch: Donkey Kong II", MACHINE_SUPPORTS_SAVE )
CONS( 1983, gnw_mw56,  0,      0, mw56,      mw56,      mw56_state,     0, "Nintendo", "Game & Watch: Mario Bros.", MACHINE_SUPPORTS_SAVE )
CONS( 1982, gnw_dj101, 0,      0, dj101,     dj101,     dj101_state,    0, "Nintendo", "Game & Watch: Donkey Kong Jr. (new wide screen)", MACHINE_SUPPORTS_SAVE )
CONS( 1983, gnw_ml102, 0,      0, ml102,     ml102,     ml102_state,    0, "Nintendo", "Game & Watch: Mario's Cement Factory (new wide screen)", MACHINE_SUPPORTS_SAVE )
CONS( 1984, gnw_bx301, 0,      0, bx301,     bx301,     bx301_state,    0, "Nintendo", "Game & Watch: Boxing", MACHINE_SUPPORTS_SAVE )
