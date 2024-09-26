// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle
/*******************************************************************************

Suwa Seikosha (S-MOS) SMC11xx handhelds
Some LCD games with this MCU, and perhaps early-80s Seiko wristwatches too.

In Europe, Tiger didn't release their pre-78-xxx LCD games (hh_sm510.cpp)
themselves, but through different publishers, eg. Orlitronic in France,
Grandstand in the UK, Virca in Italy.

TODO:
- add common mcfg (like hh_sm510) when more games are added?
- add svg for: tkkongq, tlluke2

*******************************************************************************/

#include "emu.h"

#include "cpu/tms1000/smc1102.h"
#include "sound/spkrdev.h"

#include "screen.h"
#include "speaker.h"

#include "hh_smc1k_test.lh" // common test-layout - no svg artwork(yet), use external artwork


namespace {

class hh_smc1k_state : public driver_device
{
public:
	hh_smc1k_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_speaker(*this, "speaker"),
		m_inputs(*this, "IN.%u", 0),
		m_out_x(*this, "%u.%u", 0U, 0U)
	{ }

	virtual DECLARE_INPUT_CHANGED_MEMBER(acl_button);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	// devices
	required_device<smc1102_cpu_device> m_maincpu;
	optional_device<speaker_sound_device> m_speaker;
	optional_ioport_array<4> m_inputs; // max 4
	output_finder<4, 32> m_out_x;

	u8 m_inp_mux = 0;

	virtual void write_segs(offs_t offset, u32 data);
	u8 read_inputs(int columns);
};


// machine start/reset

void hh_smc1k_state::machine_start()
{
	// resolve handlers
	m_out_x.resolve();

	// register for savestates
	save_item(NAME(m_inp_mux));
}

void hh_smc1k_state::machine_reset()
{
}



/*******************************************************************************

  Helper Functions

*******************************************************************************/

// LCD screen

void hh_smc1k_state::write_segs(offs_t offset, u32 data)
{
	for (int i = 0; i < 32; i++)
	{
		// 4*32 segments
		m_out_x[offset & 3][i] = data & 1;
		data >>= 1;
	}
}


// generic input handlers

u8 hh_smc1k_state::read_inputs(int columns)
{
	u8 ret = 0;

	// read selected input rows
	for (int i = 0; i < columns; i++)
		if (BIT(m_inp_mux, i))
			ret |= m_inputs[i]->read();

	return ret;
}

INPUT_CHANGED_MEMBER(hh_smc1k_state::acl_button)
{
	// ACL button is directly tied to MCU INIT pin
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE);
}



/*******************************************************************************

  Minidrivers (subclass, I/O, Inputs, Machine Config, ROM Defs)

*******************************************************************************/

/*******************************************************************************

  Tiger King Kong (model 7-721)
  * PCB label: TG-LC003
  * SMC1112 under epoxy (die label: SMC1112 D0W0)
  * lcd screen with custom segments, 1-bit sound

  Pyramid is suspected to have the same ROM as this.

  This is the "Quartz Game Clock" version. Tiger also released King Kong on
  several older LCD series.

*******************************************************************************/

class tkkongq_state : public hh_smc1k_state
{
public:
	tkkongq_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_smc1k_state(mconfig, type, tag)
	{ }

	void tkkongq(machine_config &config);

private:
	void write_r(u32 data);
	void write_o(u16 data);
	u8 read_k();
};

// handlers

void tkkongq_state::write_r(u32 data)
{
	// R0,R1: input mux
	m_inp_mux = data & 3;

	// R7: speaker out
	m_speaker->level_w(BIT(data, 7));
}

u8 tkkongq_state::read_k()
{
	// K: multiplexed inputs
	return read_inputs(2);
}

// inputs

static INPUT_PORTS_START( tkkongq )
	PORT_START("IN.0") // R0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN ) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN ) PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP ) PORT_16WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP ) PORT_16WAY

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_NAME("Set")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_NAME("Mode")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START2 ) PORT_NAME("Game B")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Game A")

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, tkkongq_state, acl_button, 0) PORT_NAME("ACL")
INPUT_PORTS_END

// config

void tkkongq_state::tkkongq(machine_config &config)
{
	// basic machine hardware
	SMC1112(config, m_maincpu, 32.768_kHz_XTAL);
	m_maincpu->write_r().set(FUNC(tkkongq_state::write_r));
	m_maincpu->read_k().set(FUNC(tkkongq_state::read_k));
	m_maincpu->write_segs().set(FUNC(tkkongq_state::write_segs));

	// video hardware
	config.set_default_layout(layout_hh_smc1k_test);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( tkkongq )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "smc1112_d0w0", 0x0000, 0x0800, CRC(77ff7a01) SHA1(ed5dc860e400dc11518b0a32f13879e143c17953) )
ROM_END





/*******************************************************************************

  Tiger Lucky Luke (model 7-???)
  * PCB label: REV 0, ET820, MM
  * SMC1112 under epoxy (die label: SMC1112 D2A0)
  * lcd screen with custom segments, 1-bit sound

  Mickey Mouse is suspected to have the same ROM as this.

  This is the "Double Wide Screen" version, there's also a "Large Screen" one.

*******************************************************************************/

class tlluke2_state : public hh_smc1k_state
{
public:
	tlluke2_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_smc1k_state(mconfig, type, tag)
	{ }

	void tlluke2(machine_config &config);

private:
	void write_r(u32 data);
	void write_o(u16 data);
	u8 read_k();
};

// handlers

void tlluke2_state::write_r(u32 data)
{
	// R0-R2: input mux
	m_inp_mux = data & 7;

	// R7: speaker out
	m_speaker->level_w(BIT(data, 7));
}

u8 tlluke2_state::read_k()
{
	// K: multiplexed inputs
	return read_inputs(3);
}

// inputs

static INPUT_PORTS_START( tlluke2 )
	PORT_START("IN.0") // R0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_NAME("Mode")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Game A")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 ) PORT_NAME("Game B")

	PORT_START("IN.1") // R1
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) // Jump/Open

	PORT_START("IN.2") // R2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )

	PORT_START("ACL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, tlluke2_state, acl_button, 0) PORT_NAME("ACL")
INPUT_PORTS_END

// config

void tlluke2_state::tlluke2(machine_config &config)
{
	// basic machine hardware
	SMC1112(config, m_maincpu, 32.768_kHz_XTAL);
	m_maincpu->write_r().set(FUNC(tlluke2_state::write_r));
	m_maincpu->read_k().set(FUNC(tlluke2_state::read_k));
	m_maincpu->write_segs().set(FUNC(tlluke2_state::write_segs));

	// video hardware
	config.set_default_layout(layout_hh_smc1k_test);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( tlluke2 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "smc1112_d2a0", 0x0000, 0x0800, CRC(73df08ff) SHA1(8ddf058e74e07364ab702e01f1cecb5fdb22133f) )
ROM_END



} // anonymous namespace

/*******************************************************************************

  Game driver(s)

*******************************************************************************/

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY, FULLNAME, FLAGS

// Tiger Quartz Game Clock
SYST( 1982, tkkongq, 0,      0,      tkkongq, tkkongq, tkkongq_state, empty_init, "Tiger Electronics", "King Kong (Tiger, Quartz Game Clock)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK | MACHINE_NOT_WORKING )

// Tiger Double Wide Screen
SYST( 1984, tlluke2, 0,      0,      tlluke2, tlluke2, tlluke2_state, empty_init, "Tiger Electronics", "Lucky Luke (Tiger, Double Wide Screen)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK | MACHINE_NOT_WORKING )
