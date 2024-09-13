// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Kevin Horton
/*******************************************************************************

Tiger Electronics K-2-8: Talking Learning Computer (model 7-230/7-231)

3 models exist:
- 7-230: darkblue case, toy-ish looks
- 7-231: gray case, hardware is the same
- 7-232: this one is on completely different hardware*

*: See k28.cpp for the newer version. Considering the different look and feel
and the 4 year difference, k28o is not a MAME clone set of k28. It's more like
a predecessor instead of an older revision.

Hardware notes:
- PCB label: PB-123 WIZARD, TIGER
- Intel P8021 MCU with 1KB internal ROM
- MM5445N VFD driver, 9-digit alphanumeric display same as snmath
- 2*TMS6100 (32KB VSM)
- SC-01-A speech chip
- module slot

6 modules were announced (see back of the box), but it's not known if they were
actually released.

TODO:
- plosive consonants are very difficult to hear, it's an issue in votrax.cpp
- add module slot

*******************************************************************************/

#include "emu.h"

#include "cpu/mcs48/mcs48.h"
#include "machine/tms6100.h"
#include "video/mm5445.h"
#include "video/pwm.h"
#include "sound/votrax.h"

#include "speaker.h"

// internal artwork
#include "k28o.lh"


namespace {

class k28o_state : public driver_device
{
public:
	k28o_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_vfd(*this, "vfd"),
		m_display(*this, "display"),
		m_tms6100(*this, "tms6100"),
		m_speech(*this, "speech"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void k28o(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(power_on);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	// devices/pointers
	required_device<i8021_device> m_maincpu;
	required_device<mm5445_device> m_vfd;
	required_device<pwm_display_device> m_display;
	required_device<tms6100_device> m_tms6100;
	required_device<votrax_sc01_device> m_speech;
	required_ioport_array<7> m_inputs;

	bool m_power_on = false;
	attotime m_onbutton_time;
	u8 m_inp_mux = 0;
	u8 m_phoneme = 0x3f;
	int m_speech_strobe = 0;
	u64 m_vfd_data = 0;

	void vfd_output_w(u64 data);
	void mcu_p0_w(u8 data);
	u8 mcu_p1_r();
	u8 mcu_p2_r();
	void mcu_p2_w(u8 data);

	void power_off();
};

void k28o_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_power_on));
	save_item(NAME(m_onbutton_time));
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_phoneme));
	save_item(NAME(m_speech_strobe));
	save_item(NAME(m_vfd_data));
}



/*******************************************************************************
    Power
*******************************************************************************/

void k28o_state::machine_reset()
{
	m_vfd_data = 0;
	m_power_on = true;
	m_maincpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);

	// it relies on reading the on-button as pressed when it's turned on
	m_onbutton_time = machine().time() + attotime::from_msec(250);
}

INPUT_CHANGED_MEMBER(k28o_state::power_on)
{
	if (newval && !m_power_on)
		machine_reset();
}

void k28o_state::power_off()
{
	m_power_on = false;
	m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_display->clear();
}



/*******************************************************************************
    I/O
*******************************************************************************/

// MM5445 VFD

void k28o_state::vfd_output_w(u64 data)
{
	// O1-O16: digit segment data
	// O17-O25: digit select
	u16 seg_data = bitswap<16>(data,15,14,2,6,5,3,1,7,12,11,10,13,0,4,9,8);
	m_display->matrix(data >> 16, seg_data);

	// O26: power-off request on falling edge
	if (~data & m_vfd_data & 1 << 25)
		power_off();
	m_vfd_data = data;
}


// I8021 ports

void k28o_state::mcu_p0_w(u8 data)
{
	// d0,d1: phoneme high bits
	// d0-d2: input mux high bits
	m_inp_mux = (m_inp_mux & 0xf) | (~data << 4 & 0x70);
	m_phoneme = (m_phoneme & 0xf) | (data << 4 & 0x30);

	// d3: SC-01 strobe, latch phoneme on rising edge
	int strobe = data >> 3 & 1;
	if (strobe && !m_speech_strobe)
		m_speech->write(m_phoneme);
	m_speech_strobe = strobe;

	// d5: VFD driver data enable
	m_vfd->enable_w(data >> 5 & 1);

	// d4: VSM chip enable
	// d6: VSM M0
	// d7: VSM M1
	m_tms6100->cs_w(~data >> 4 & 1);
	m_tms6100->m0_w(data >> 6 & 1);
	m_tms6100->m1_w(data >> 7 & 1);
	m_tms6100->clk_w(1);
	m_tms6100->clk_w(0);
}

u8 k28o_state::mcu_p1_r()
{
	u8 data = 0;

	// multiplexed inputs (active low)
	for (int i = 0; i < 7; i++)
		if (BIT(m_inp_mux, i))
		{
			data |= m_inputs[i]->read();

			// force press on-button at boot
			if (i == 5 && machine().time() < m_onbutton_time)
				data |= 1;
		}

	return data ^ 0xff;
}

u8 k28o_state::mcu_p2_r()
{
	// d3: VSM data
	return (m_tms6100->data_line_r()) ? 8 : 0;
}

void k28o_state::mcu_p2_w(u8 data)
{
	// d0: VFD driver serial data
	m_vfd->data_w(data & 1);

	// d0-d3: VSM data, input mux and SC-01 phoneme lower nibble
	m_tms6100->add_w(data);
	m_inp_mux = (m_inp_mux & ~0xf) | (~data & 0xf);
	m_phoneme = (m_phoneme & ~0xf) | (data & 0xf);
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( k28o )
	PORT_START("IN.0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("Yes/True")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("No/False")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE) PORT_NAME("Select")

	PORT_START("IN.1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_END) PORT_NAME("Scroll")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("<")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8) PORT_NAME("Erase/Clear")

	PORT_START("IN.2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_HOME) PORT_NAME("Menu")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_NAME(">")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13) PORT_NAME("Enter/Start")

	PORT_START("IN.3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("Prompt")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")

	PORT_START("IN.4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("Say It Again(Repeat)")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME(".")

	PORT_START("IN.5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POWER_ON ) PORT_CHANGED_MEMBER(DEVICE_SELF, k28o_state, power_on, 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("=")

	PORT_START("IN.6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POWER_OFF ) // -> auto_power_off
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME(u8"÷")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ASTERISK) PORT_NAME(u8"×")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("-")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("+")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void k28o_state::k28o(machine_config &config)
{
	// basic machine hardware
	I8021(config, m_maincpu, 3.579545_MHz_XTAL);
	m_maincpu->bus_out_cb().set(FUNC(k28o_state::mcu_p0_w));
	m_maincpu->p1_in_cb().set(FUNC(k28o_state::mcu_p1_r));
	m_maincpu->p2_in_cb().set(FUNC(k28o_state::mcu_p2_r));
	m_maincpu->p2_out_cb().set(FUNC(k28o_state::mcu_p2_w));
	m_maincpu->prog_out_cb().set("vfd", FUNC(mm5445_device::clock_w));
	m_maincpu->t1_in_cb().set("speech", FUNC(votrax_sc01_device::request));

	TMS6100(config, m_tms6100, 3.579545_MHz_XTAL / 15); // CLK tied to 8021 ALE pin

	// video hardware
	MM5445(config, m_vfd).output_cb().set(FUNC(k28o_state::vfd_output_w));
	PWM_DISPLAY(config, m_display).set_size(9, 16);
	m_display->set_segmask(0x1ff, 0x3fff);

	config.set_default_layout(layout_k28o);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	VOTRAX_SC01A(config, "speech", 760000).add_route(ALL_OUTPUTS, "mono", 0.5); // measured 760kHz on its RC pin
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( k28o )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "p8021_7-230-itl", 0x0000, 0x0400, CRC(15536d20) SHA1(fac98ce652340ffb2d00952697c3a9ce75393fa4) )

	ROM_REGION( 0x10000, "tms6100", ROMREGION_ERASEFF ) // 8000-bfff? = space reserved for cartridge
	ROM_LOAD( "cm62050u", 0x0000, 0x4000, CRC(6afb8645) SHA1(e22435568ed11c6516a3b4008131f99cd4e47aa9) )
	ROM_LOAD( "cm62051u", 0x4000, 0x4000, CRC(0fa61baa) SHA1(831be669423ba60c7f85a896b4b09a1295478bd9) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY, FULLNAME, FLAGS
SYST( 1981, k28o,  0,      0,      k28o,    k28o,  k28o_state, empty_init, "Tiger Electronics", "K-2-8: Talking Learning Computer (model 7-230)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
