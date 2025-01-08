// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle
/*******************************************************************************

Atari Europe Hit Parade series jukeboxes

Atari Europe is unrelated to the modern Atari Europe, or Infogrames.
It was a French jukebox company that was partially owned by Atari for a while
in the mid-1970s. It gets a bit confusing, for more information, Google for
Socodimex, Electro-Kicker, Europe Electronique.

There are probably more jukeboxes on similar hardware.

Hardware notes:
- TMS1300NL MP2012 (die label: 1100B, MP2012)
- 512 nibbles RAM (2*MCM5101 or equivalent), kept alive by a big capacitor
- keypad interface, 8 7segs
- electromechanical stuff for the actual jukebox

TODO:
- why does the display blink? because no songs are in memory?
- PA,PB sensors at K1,K2 + R8
- P1,P2 sensors at K1,K2 (not muxed)
- motor triggers (which R pins?)
- CPU speed is guessed

*******************************************************************************/

#include "emu.h"

#include "cpu/tms1000/tms1100.h"
#include "machine/nvram.h"
#include "video/pwm.h"

#include "hitparade.lh"


namespace {

class hitpar_state : public driver_device
{
public:
	hitpar_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_nvram(*this, "nvram", 0x200, ENDIANNESS_BIG),
		m_display(*this, "display"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void hitpar(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<tms1k_base_device> m_maincpu;
	memory_share_creator<u8> m_nvram;
	required_device<pwm_display_device> m_display;
	required_ioport_array<6> m_inputs;

	u32 m_r = 0;
	u16 m_o = 0;

	void write_r(u32 data);
	void write_o(u16 data);
	u8 read_k();
};

void hitpar_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_r));
	save_item(NAME(m_o));
}



/*******************************************************************************
    I/O
*******************************************************************************/

void hitpar_state::write_r(u32 data)
{
	// R14,R15: RAM CS
	// R13: RAM R/W (write on falling edge)
	// R0-R7: RAM address
	if (BIT(m_r, 13) && !BIT(data, 13))
	{
		if (BIT(data, 14))
			m_nvram[data & 0xff] = m_o & 0xf;
		if (BIT(data, 15))
			m_nvram[(data & 0xff) | 0x100] = m_o & 0xf;
	}

	// R0-R7: digit select
	// R0-R5: input mux
	// R8: sensors mux
	m_display->write_my(data);
	m_r = data;
}

void hitpar_state::write_o(u16 data)
{
	// O0-O6: digit segment data
	// O7: mask digit segments A-D
	u8 mask = (data & 0x80) ? 0x7f : 0x70;
	m_display->write_mx(data & mask);

	// O0-O3: RAM data
	m_o = data;
}

u8 hitpar_state::read_k()
{
	u8 data = 0;

	// read RAM
	if (BIT(m_r, 14))
		data |= m_nvram[m_r & 0xff];
	if (BIT(m_r, 15))
		data |= m_nvram[(m_r & 0xff) | 0x100];

	// read inputs
	for (int i = 0; i < 6; i++)
		if (BIT(m_r, i))
			data |= m_inputs[i]->read();

	return data;
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( hitpar )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Hit")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_SERVICE3) PORT_NAME("Service Tot")

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME("Reset")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_SERVICE2) PORT_NAME("Service Sel")
	PORT_SERVICE( 0x04, IP_ACTIVE_HIGH )
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_SERVICE1) PORT_NAME("Service Credit")

	PORT_START("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN) // income-total
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN)

	PORT_START("IN.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_COIN2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_COIN3)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_COIN4)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void hitpar_state::hitpar(machine_config &config)
{
	// basic machine hardware
	TMS1300(config, m_maincpu, 350000); // guessed
	m_maincpu->read_k().set(FUNC(hitpar_state::read_k));
	m_maincpu->write_r().set(FUNC(hitpar_state::write_r));
	m_maincpu->write_o().set(FUNC(hitpar_state::write_o));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(8, 7);
	m_display->set_segmask(0xff, 0x7f);
	config.set_default_layout(layout_hitparade);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( hitpar )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp2012", 0x0000, 0x0800, CRC(6c5d84a2) SHA1(0f028d36eaf4cb07d0b28b94e0bfa551d78c5db8) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common2_micro.pla", 0, 867, CRC(7cc90264) SHA1(c6e1cf1ffb178061da9e31858514f7cd94e86990) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_hitpar_output.pla", 0, 365, CRC(19feb313) SHA1(7a124e313c0339c1c4983656ab07eb7128a396d4) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME      PARENT  MACHINE  INPUT   CLASS         INIT        SCREEN  COMPANY, FULLNAME, FLAGS
GAME( 1977, hitpar,   0,      hitpar,  hitpar, hitpar_state, empty_init, ROT0,   "Atari Europe", "Hit Parade 108 / 144 / 160", MACHINE_SUPPORTS_SAVE | MACHINE_MECHANICAL | MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
