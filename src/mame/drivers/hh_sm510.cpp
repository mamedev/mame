// license:BSD-3-Clause
// copyright-holders:hap, Sean Riddle
/***************************************************************************

  Sharp SM510 MCU x..

  TODO:
  - barnacles

***************************************************************************/

#include "emu.h"
#include "cpu/sm510/sm510.h"
#include "sound/speaker.h"

#include "hh_sm510_test.lh" // common test-layout - use external artwork


class hh_sm510_state : public driver_device
{
public:
	hh_sm510_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_inp_matrix(*this, "IN"),
		m_speaker(*this, "speaker"),
		m_inp_lines(0)
	{ }

	// devices
	required_device<cpu_device> m_maincpu;
	optional_ioport_array<5> m_inp_matrix; // max 5
	optional_device<speaker_sound_device> m_speaker;

	// misc common
	UINT16 m_inp_mux;                   // multiplexed inputs mask
	int m_inp_lines;                    // number of input mux columns
	UINT8 m_lcd_output_cache[0x100];

	UINT8 read_inputs(int columns);

	virtual void update_k_line();
	virtual DECLARE_INPUT_CHANGED_MEMBER(input_changed);
	virtual DECLARE_READ8_MEMBER(input_r);
	virtual DECLARE_WRITE8_MEMBER(input_w);
	virtual DECLARE_WRITE16_MEMBER(lcd_segment_w);
	virtual DECLARE_WRITE8_MEMBER(sm511_melody_w);

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

WRITE16_MEMBER(hh_sm510_state::lcd_segment_w)
{
	for (int seg = 0; seg < 0x10; seg++)
	{
		int index = offset << 4 | seg;
		UINT8 state = data >> seg & 1;

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


// generic input handlers - usually S output is input mux, and K input for buttons

UINT8 hh_sm510_state::read_inputs(int columns)
{
	UINT8 ret = 0;

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


// other generic output handlers

WRITE8_MEMBER(hh_sm510_state::sm511_melody_w)
{
	// SM511 R pin is melody output
	m_speaker->level_w(data & 1);
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

	DECLARE_WRITE8_MEMBER(speaker_w);
};

// handlers

WRITE8_MEMBER(ktopgun_state::speaker_w)
{
	m_speaker->level_w(data >> 0 & 1);
}


// config

static INPUT_PORTS_START( ktopgun )
	PORT_START("IN.0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, NULL)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, NULL)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, NULL)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, NULL)

	PORT_START("IN.1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, NULL)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_VOLUME_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, NULL) PORT_NAME("Sound")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, NULL)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, NULL)

	PORT_START("IN.2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POWER_ON ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, NULL)
	PORT_BIT( 0x0e, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static MACHINE_CONFIG_START( ktopgun, ktopgun_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SM510, XTAL_32_768kHz)
	MCFG_SM510_WRITE_SEGA_CB(WRITE16(hh_sm510_state, lcd_segment_w))
	MCFG_SM510_WRITE_SEGB_CB(WRITE16(hh_sm510_state, lcd_segment_w))
	MCFG_SM510_WRITE_SEGBS_CB(WRITE16(hh_sm510_state, lcd_segment_w))
	MCFG_SM510_READ_K_CB(READ8(hh_sm510_state, input_r))
	MCFG_SM510_WRITE_S_CB(WRITE8(hh_sm510_state, input_w))
	MCFG_SM510_WRITE_R_CB(WRITE8(ktopgun_state, speaker_w))

	MCFG_DEFAULT_LAYOUT(layout_hh_sm510_test)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Konami Teenage Mutant Ninja Turtles
  * Sharp SM511 under epoxy (die label KMS 73B, KMS 774)

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
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, NULL)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, NULL)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, NULL)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, NULL)

	PORT_START("IN.1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, NULL) PORT_NAME("Game Select")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_VOLUME_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, NULL) PORT_NAME("Sound")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, NULL)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_POWER_ON ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, NULL) PORT_NAME("On/Start")
	//Wouldn't be better if IPT_START for "On/Start" was used???

	PORT_START("IN.2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, NULL)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, NULL)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, NULL)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, NULL)
INPUT_PORTS_END

static MACHINE_CONFIG_START( ktmnt, ktmnt_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SM511, XTAL_32_768kHz)
	MCFG_SM510_WRITE_SEGA_CB(WRITE16(hh_sm510_state, lcd_segment_w))
	MCFG_SM510_WRITE_SEGB_CB(WRITE16(hh_sm510_state, lcd_segment_w))
	MCFG_SM510_WRITE_SEGBS_CB(WRITE16(hh_sm510_state, lcd_segment_w))
	MCFG_SM510_READ_K_CB(READ8(hh_sm510_state, input_r))
	MCFG_SM510_WRITE_S_CB(WRITE8(hh_sm510_state, input_w))
	MCFG_SM510_WRITE_R_CB(WRITE8(hh_sm510_state, sm511_melody_w))

	MCFG_DEFAULT_LAYOUT(layout_hh_sm510_test)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Konami Gradius
  * Sharp SM511 under epoxy (die label KMS 73B, KMS 774)

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
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POWER_ON ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, NULL)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_POWER_OFF ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, NULL)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, NULL)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_VOLUME_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, NULL) PORT_NAME("Sound")

	PORT_START("IN.1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, NULL)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, NULL)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, NULL)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static MACHINE_CONFIG_START( kgradius, kgradius_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SM511, XTAL_32_768kHz)
	MCFG_SM510_WRITE_SEGA_CB(WRITE16(hh_sm510_state, lcd_segment_w))
	MCFG_SM510_WRITE_SEGB_CB(WRITE16(hh_sm510_state, lcd_segment_w))
	MCFG_SM510_WRITE_SEGBS_CB(WRITE16(hh_sm510_state, lcd_segment_w))
	MCFG_SM510_READ_K_CB(READ8(hh_sm510_state, input_r))
	MCFG_SM510_WRITE_S_CB(WRITE8(hh_sm510_state, input_w))
	MCFG_SM510_WRITE_R_CB(WRITE8(hh_sm510_state, sm511_melody_w))

	MCFG_DEFAULT_LAYOUT(layout_hh_sm510_test)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END





/***************************************************************************

  Nintendo Game & Watch: Mickey & Donald (model DM-53)
  * PCB label DM-53
  * Sharp SM510 label DM-53 (die label CMS54C, CMS565)

***************************************************************************/

class gnwmndon_state : public hh_sm510_state
{
public:
	gnwmndon_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_sm510_state(mconfig, type, tag)
	{
		m_inp_lines = 2;
	}

	DECLARE_WRITE8_MEMBER(speaker_w);
};

// handlers

WRITE8_MEMBER(gnwmndon_state::speaker_w)
{
	m_speaker->level_w(data >> 1 & 1);
}


// config

static INPUT_PORTS_START( gnwmndon )
	PORT_START("IN.0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, NULL)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, NULL)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, NULL)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, NULL)

	PORT_START("IN.1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, NULL) // time
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, NULL) // b
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, NULL) // a
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_CHANGED_MEMBER(DEVICE_SELF, hh_sm510_state, input_changed, NULL) // alarm
INPUT_PORTS_END

static MACHINE_CONFIG_START( gnwmndon, gnwmndon_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SM510, XTAL_32_768kHz)
	MCFG_SM510_WRITE_SEGA_CB(WRITE16(hh_sm510_state, lcd_segment_w))
	MCFG_SM510_WRITE_SEGB_CB(WRITE16(hh_sm510_state, lcd_segment_w))
	MCFG_SM510_WRITE_SEGBS_CB(WRITE16(hh_sm510_state, lcd_segment_w))
	MCFG_SM510_READ_K_CB(READ8(hh_sm510_state, input_r))
	MCFG_SM510_WRITE_S_CB(WRITE8(hh_sm510_state, input_w))
	MCFG_SM510_WRITE_R_CB(WRITE8(gnwmndon_state, speaker_w))

	MCFG_DEFAULT_LAYOUT(layout_hh_sm510_test)

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
ROM_END


ROM_START( ktmnt ) // except for filler/unused bytes, ROM listing in patent US5150899 "BH005 TMNT" prog/music is same
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "kms73b_kms774.prog", 0x0000, 0x1000, CRC(a1064f87) SHA1(92156c35fbbb414007ee6804fe635128a741d5f1) )

	ROM_REGION( 0x100, "maincpu:music", 0 )
	ROM_LOAD( "kms73b_kms774.music", 0x000, 0x100, CRC(8270d626) SHA1(bd91ca1d5cd7e2a62eef05c0033b19dcdbe441ca) )
ROM_END


ROM_START( kcontra ) // except for filler/unused bytes, ROM listing in patent US5120057 "BH002 C (Contra)" prog/music is same
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "kms73b_kms773.prog", 0x0000, 0x1000, CRC(bf834877) SHA1(055dd56ec16d63afba61ab866481fd9c029fb54d) )

	ROM_REGION( 0x100, "maincpu:music", 0 )
	ROM_LOAD( "kms73b_kms773.music", 0x000, 0x100, CRC(23d02b99) SHA1(703938e496db0eeacd14fe7605d4b5c39e0a5bc8) )
ROM_END


ROM_START( kgradius )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "kms73b_kms771.prog", 0x0000, 0x1000, CRC(830c2afc) SHA1(bb9ebd4e52831cc02cd92dd4b37675f34cf37b8c) )

	ROM_REGION( 0x100, "maincpu:music", 0 )
	ROM_LOAD( "kms73b_kms771.music", 0x000, 0x100, CRC(4c586b73) SHA1(14c5ab2898013a577f678970a648c374749cc66d) )
ROM_END


ROM_START( gnwmndon )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "dm53_cms54c_565", 0x0000, 0x1000, CRC(e21fc0f5) SHA1(3b65ccf9f98813319410414e11a3231b787cdee6) )
ROM_END



/*    YEAR  NAME       PARENT COMPAT MACHINE   INPUT      INIT              COMPANY, FULLNAME, FLAGS */
CONS( 1989, kcontra,   0,        0, ktmnt,     ktopgun,   driver_device, 0, "Konami", "Contra (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK | MACHINE_NOT_WORKING )
CONS( 1989, ktopgun,   0,        0, ktopgun,   ktopgun,   driver_device, 0, "Konami", "Top Gun (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK | MACHINE_NOT_WORKING )
CONS( 1989, ktmnt,     0,        0, ktmnt,     ktmnt,     driver_device, 0, "Konami", "Teenage Mutant Ninja Turtles (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK | MACHINE_NOT_WORKING )
CONS( 1989, kgradius,  0,        0, kgradius,  kgradius,  driver_device, 0, "Konami", "Gradius (handheld)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK | MACHINE_NOT_WORKING )

CONS( 1982, gnwmndon,  0,        0, gnwmndon,  gnwmndon,  driver_device, 0, "Nintendo", "Game & Watch: Mickey & Donald", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK | MACHINE_NOT_WORKING )
