// license:BSD-3-Clause
// copyright-holders:Devin Acker

/*

    Driver for Yamaha YMW270-F (GEW7) and YMW282-F (GEW7S) keyboards

    A few different "families" are covered by this driver:

    - "Music Mode Selector" dial (1992-1993)
        - PSR-150: 61 keys, stereo, 4 drum pads + 4 sound effect pads
        - PSR-110: 49 keys, stereo, 4 drum/sound effect pads
        - PSR-75: 49 keys, mono, no pads
        - PSS-31: 49 mini keys, stereo, 4 drum pads
        - PSS-21: 37 mini keys, stereo, 4 drum pads
        - PSS-11: 32 mini keys, mono, no pads

    - Circular control panel w/ 7-segment display (1994)
        - PSR-180: 61 keys, stereo, 4 drum/sound effect pads
            later released as PSR-185 (1995)
        - PSR-76: 49 keys, mono, no pads
            later released as PSR-73/77 (1995), PSR-74 (1999), PSR-125 (2002)

    - Mini keyboards w/ tone variation button (1994)
        - PSS-12: 32 mini keys, mono, 2Mbit ROM
        - PSS-6: 32 "ultra mini" keys, mono, 1Mbit ROM, some tone differences

    - DD-9 Digital Percussion (1994), later released as DD-20 (2003)

    - LCD with large icons, metronome, volume display (1996)
        - PSR-190: 61 keys, stereo
        - PSR-78: 49 keys, mono

    Other known undumped models:
    - PSR-130 (1997, 61 keys, two dials for tone & rhythm selection)

*/

#include "emu.h"

#include "cpu/m6502/gew7.h"
#include "sound/flt_rc.h"
#include "video/hd44780.h"
#include "video/pwm.h"
#include "screen.h"
#include "speaker.h"

#include "dd9.lh"
#include "psr110.lh"
#include "psr150.lh"
#include "psr180.lh"
#include "psr75.lh"
#include "psr76.lh"
#include "pss11.lh"
#include "pss12.lh"
#include "pss21.lh"
#include "pss31.lh"
#include "pss6.lh"

namespace {

class psr150_state : public driver_device
{
public:
	psr150_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pwm(*this, "pwm")
		, m_lcdc(*this, "lcdc")
		, m_port(*this, "P%c", 'A')
		, m_keys(*this, "KEY%u", 0U)
		, m_dial(*this, "DIAL")
		, m_outputs(*this, "%02x.%d.%d", 0U, 0U, 0U)
		, m_switch(*this, "switch_pos")
		, m_led(*this, "led%u", 0U)
		, m_digit(*this, "digit%u", 0U)
	{ }

	void psr150(machine_config &config);
	void psr110(machine_config &config);
	void pss21(machine_config &config);
	void pss31(machine_config &config);
	void psr75(machine_config &config);
	void pss11(machine_config &config);
	void dd9(machine_config &config);
	void psr180_base(machine_config &config);
	void psr180(machine_config &config);
	void psr76(machine_config &config);
	void pss12(machine_config &config);
	void pss6(machine_config &config);
	void psr190_base(machine_config &config);
	void psr190(machine_config &config);
	void psr78(machine_config &config);

	template <offs_t Num, u8 PullUps = 0xff>
	void port_pullup_w(offs_t offset, u8 data, u8 mem_mask);

	// most of these keyboards have key matrix rows & columns split across multiple ports each,
	// so use templates to be able to generate multiple r/w methods as appropriate
	template <unsigned StartBit, unsigned Count>
	void keys_w(int state);
	template <unsigned StartBit>
	ioport_value keys_r();

	ioport_value dial_r();

	DECLARE_INPUT_CHANGED_MEMBER(switch_w);
	ioport_value switch_r() { return m_switch; }

	void apo_w(int state) { if (state) m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE); }
	DECLARE_INPUT_CHANGED_MEMBER(power_w) { if (!newval) m_maincpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE); }

	template <unsigned Num>
	void led_w(int state) { m_led[Num] = (state ? 1 : 0); }

	void pwm_row_w(int state);
	template <unsigned Bit>
	void pwm_col_w(int state);

	template <unsigned Num>
	void digit_w(u8 state) { m_digit[Num] = state ^ 0xff; }

	ioport_value lcd_r() { return m_lcdc->db_r() >> 4; }
	void lcd_w(int state) { m_lcdc->db_w(state << 4); }

private:
	virtual void driver_start() override;

	void render_w(int state);

	required_device<gew7_device> m_maincpu;
	optional_device<pwm_display_device> m_pwm;
	optional_device<ks0066_device> m_lcdc;

	optional_ioport_array<6> m_port;
	optional_ioport_array<19> m_keys;
	optional_ioport m_dial;
	output_finder<64, 8, 5> m_outputs;
	output_finder<> m_switch;
	output_finder<4> m_led;
	output_finder<2> m_digit;

	ioport_value m_key_sel{};
	ioport_value m_pwm_col{};
};

template <offs_t Num, u8 PullUps>
void psr150_state::port_pullup_w(offs_t offset, u8 data, u8 mem_mask)
{
	// these keyboards scan the buttons by setting one matrix row to output, the rest to input,
	// and relying on external pullup resistors to keep the 'input' rows deselected
	m_port[Num]->write((data & mem_mask) | (PullUps & ~mem_mask));
}

template <unsigned StartBit, unsigned Count>
void psr150_state::keys_w(int state)
{
	constexpr auto mask = make_bitmask<ioport_value>(Count);

	m_key_sel &= ~(mask << StartBit);
	m_key_sel |= (state & mask) << StartBit;
}

template <unsigned StartBit>
ioport_value psr150_state::keys_r()
{
	ioport_value result = 0;
	for (unsigned i = 0U; i < m_keys.size(); i++)
		if (BIT(m_key_sel, i))
			result |= m_keys[i].read_safe(0);

	return result >> StartBit;
}

ioport_value psr150_state::dial_r()
{
	// return the dial position as a 2-bit gray code
	const u8 val = m_dial->read();
	return (val >> 6) ^ (val >> 7);
}

INPUT_CHANGED_MEMBER(psr150_state::switch_w)
{
	if (!oldval && newval)
		m_switch = param;
}

void psr150_state::pwm_row_w(int state)
{
	m_pwm->matrix(state, m_pwm_col);
}

template <unsigned Bit>
void psr150_state::pwm_col_w(int state)
{
	keys_w<11 + Bit, 1>(state);
	m_pwm_col = m_key_sel >> 11;

	if (m_pwm->read_my() != 0)
		m_pwm->write_mx(m_pwm_col);
}

void psr150_state::render_w(int state)
{
	if (!state)
		return;

	const u8* render = m_lcdc->render();
	for (int x = 0; x != 64; x++) {
		for (int y = 0; y != 8; y++) {
			u8 v = *render++;
			for (int z = 0; z != 5; z++)
				m_outputs[x][y][z] = (v >> z) & 1;
		}
		render += 8;
	}
}


void psr150_state::driver_start()
{
	m_outputs.resolve();
	m_led.resolve();
	m_digit.resolve();
	m_switch.resolve();

	m_switch = 0x2; // "Voice Play" mode

	save_item(NAME(m_key_sel));
	save_item(NAME(m_pwm_col));
}


void psr150_state::psr150(machine_config &config)
{
	GEW7(config, m_maincpu, 8_MHz_XTAL);
	m_maincpu->port_in_cb<0>().set_ioport("PA");
	m_maincpu->port_out_cb<0>().set_ioport("PA");
	m_maincpu->port_out_cb<1>().set_ioport("PB");
	m_maincpu->port_in_cb<2>().set_ioport("PC");
	m_maincpu->port_in_cb<5>().set_ioport("PF");
	m_maincpu->port_out_cb<5>().set(FUNC(psr150_state::port_pullup_w<5>));
	m_maincpu->add_route(0, "lfilter", 1.0);
	m_maincpu->add_route(1, "rfilter", 1.0);

	// set up AC filters since the keyboard purposely outputs a DC offset when idle
	// TODO: there is also a RLC lowpass with R=120, L=3.3mH, C=0.33uF (or R=150 for psr110)
	FILTER_RC(config, "lfilter").set_ac().add_route(ALL_OUTPUTS, "speaker", 1.0, 0);
	FILTER_RC(config, "rfilter").set_ac().add_route(ALL_OUTPUTS, "speaker", 1.0, 1);

	SPEAKER(config, "speaker", 2).front();

	config.set_default_layout(layout_psr150);
}

void psr150_state::psr110(machine_config &config)
{
	psr150(config);
	config.set_default_layout(layout_psr110);
}

void psr150_state::pss21(machine_config &config)
{
	psr150(config);
	// bits 6-7 indicate model (see also pss11, pss31, psr75)
	m_maincpu->port_force_bits(5, 0x40, 0xc0);

	config.set_default_layout(layout_pss21);
}

void psr150_state::pss31(machine_config &config)
{
	psr150(config);
	// bits 6-7 indicate model (see also pss11, pss21, psr75)
	m_maincpu->port_force_bits(5, 0x80, 0xc0);

	config.set_default_layout(layout_pss31);
}

void psr150_state::psr75(machine_config &config)
{
	GEW7(config, m_maincpu, 8'000'000);
	m_maincpu->port_in_cb<0>().set_ioport("PA");
	m_maincpu->port_out_cb<0>().set_ioport("PA");
	m_maincpu->port_out_cb<1>().set_ioport("PB");
	m_maincpu->port_in_cb<2>().set_ioport("PC");
	m_maincpu->port_in_cb<5>().set_ioport("PF");
	m_maincpu->port_out_cb<5>().set(FUNC(psr150_state::port_pullup_w<5>));
	// bits 6-7 indicate model (see also pss11, pss21, pss31)
	m_maincpu->port_force_bits(5, 0xc0, 0xc0);
	m_maincpu->add_route(0, "filter", 1.0);

	// set up AC filter since the keyboard purposely outputs a DC offset when idle
	// TODO: there is probably also a RLC lowpass, check schematic if available (also for pss21 and pss31)
	FILTER_RC(config, "filter").set_ac().add_route(ALL_OUTPUTS, "speaker", 1.0);

	SPEAKER(config, "speaker").front_center();

	config.set_default_layout(layout_psr75);
}

void psr150_state::pss11(machine_config &config)
{
	GEW7(config, m_maincpu, 8'000'000);
	m_maincpu->port_out_cb<0>().set_ioport("PA");
	m_maincpu->port_in_cb<1>().set_ioport("PB");
	m_maincpu->port_in_cb<2>().set_ioport("PC");
	m_maincpu->port_out_cb<2>().set(FUNC(psr150_state::port_pullup_w<2>));
	m_maincpu->port_in_cb<5>().set_ioport("PF");
	// bits 6-7 indicate model (see also pss21, pss31, psr75)
	m_maincpu->port_force_bits(5, 0x00, 0xc0);
	m_maincpu->add_route(0, "filter", 1.0);

	// set up AC filter since the keyboard purposely outputs a DC offset when idle
	// TODO: there is also a RLC lowpass with R=120, L=3.3mH, C=0.1uF
	FILTER_RC(config, "filter").set_ac().add_route(ALL_OUTPUTS, "speaker", 1.0);

	SPEAKER(config, "speaker").front_center();

	config.set_default_layout(layout_pss11);
}

void psr150_state::dd9(machine_config &config)
{
	GEW7(config, m_maincpu, 8'000'000);
	m_maincpu->port_out_cb<0>().set_ioport("PA");
	m_maincpu->port_out_cb<1>().set(FUNC(psr150_state::digit_w<0>));
	m_maincpu->port_out_cb<2>().set(FUNC(psr150_state::digit_w<1>));
	m_maincpu->port_in_cb<5>().set_ioport("PF");
	m_maincpu->add_route(1, "speaker", 1.0);

	// TODO: there is also a RLC lowpass with R=150, L=3.3mH, C=0.33uF
	// (AC filter not really needed since this doesn't output a "dummy" DC offset sample, unlike the keyboards)

	SPEAKER(config, "speaker").front_center();

	config.set_default_layout(layout_dd9);
}

void psr150_state::psr180_base(machine_config &config)
{
	GEW7(config, m_maincpu, 8'000'000);
	m_maincpu->port_out_cb<0>().set_ioport("PA");
	m_maincpu->port_in_cb<1>().set_ioport("PB");
	m_maincpu->port_out_cb<1>().set_ioport("PB");
	m_maincpu->port_in_cb<2>().set_ioport("PC");
	m_maincpu->port_out_cb<2>().set_ioport("PC");
	m_maincpu->port_in_cb<5>().set_ioport("PF");
	m_maincpu->port_out_cb<5>().set_ioport("PF");

	PWM_DISPLAY(config, m_pwm);
	m_pwm->set_size(3, 8);
	m_pwm->set_segmask(0x6, 0xff);
}

void psr150_state::psr180(machine_config &config)
{
	psr180_base(config);
	m_maincpu->add_route(0, "lfilter", 1.0);
	m_maincpu->add_route(1, "rfilter", 1.0);

	// set up AC filters since the keyboard purposely outputs a DC offset when idle
	// TODO: there is also a RLC lowpass with R=120, L=3.3mH, C=0.39uF
	FILTER_RC(config, "lfilter").set_ac().add_route(ALL_OUTPUTS, "speaker", 1.0, 0);
	FILTER_RC(config, "rfilter").set_ac().add_route(ALL_OUTPUTS, "speaker", 1.0, 1);

	SPEAKER(config, "speaker", 2).front();

	config.set_default_layout(layout_psr180);
}

void psr150_state::psr76(machine_config &config)
{
	psr180_base(config);
	m_maincpu->add_route(0, "filter", 1.0);

	// set up AC filter since the keyboard purposely outputs a DC offset when idle
	// TODO: there is also a RLC lowpass with R=120, L=1mH, C=0.33uF
	FILTER_RC(config, "filter").set_ac().add_route(ALL_OUTPUTS, "speaker", 1.0);

	SPEAKER(config, "speaker").front_center();

	config.set_default_layout(layout_psr76);
}

void psr150_state::pss12(machine_config &config)
{
	GEW7(config, m_maincpu, 8'000'000);
	m_maincpu->port_in_cb<0>().set_ioport("PA");
	m_maincpu->port_out_cb<1>().set_ioport("PB");
	m_maincpu->port_in_cb<2>().set_ioport("PC");
	m_maincpu->port_out_cb<2>().set(FUNC(psr150_state::port_pullup_w<2>));
	m_maincpu->port_out_cb<5>().set_ioport("PF");
	m_maincpu->add_route(1, "filter", 1.0);

	// set up AC filter since the keyboard purposely outputs a DC offset when idle
	// TODO: there is also a RLC lowpass with R=120, L=3.3mH, C=0.33uF
	FILTER_RC(config, "filter").set_ac().add_route(ALL_OUTPUTS, "speaker", 1.0);

	SPEAKER(config, "speaker").front_center();

	config.set_default_layout(layout_pss12);
}

void psr150_state::pss6(machine_config &config)
{
	GEW7(config, m_maincpu, 8'000'000);
	m_maincpu->port_out_cb<1>().set_ioport("PB");
	m_maincpu->port_out_cb<2>().set(FUNC(psr150_state::port_pullup_w<2>));
	m_maincpu->port_in_cb<5>().set_ioport("PF");
	m_maincpu->add_route(0, "filter", 1.0);

	// set up AC filter since the keyboard purposely outputs a DC offset when idle
	FILTER_RC(config, "filter").set_ac().add_route(ALL_OUTPUTS, "speaker", 1.0);

	SPEAKER(config, "speaker").front_center();

	config.set_default_layout(layout_pss6);
}

void psr150_state::psr190_base(machine_config &config)
{
	GEW7(config, m_maincpu, 8'000'000);
	m_maincpu->port_out_cb<0>().set_ioport("PA");
	m_maincpu->port_in_cb<1>().set_ioport("PB");
	m_maincpu->port_out_cb<1>().set_ioport("PB");
	m_maincpu->port_in_cb<2>().set_ioport("PC_R");
	m_maincpu->port_out_cb<2>().set_ioport("PC_W");

	KS0066(config, m_lcdc, 270'000); // OSC = 91K resistor, TODO: actually KS0076B-00
	m_lcdc->set_lcd_size(2, 8);

	screen_device& screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1000, 775);
	screen.set_visarea_full();
	screen.screen_vblank().set(FUNC(psr150_state::render_w));
}

void psr150_state::psr190(machine_config &config)
{
	psr190_base(config);
	m_maincpu->port_out_cb<5>().set(FUNC(psr150_state::port_pullup_w<5>));
	m_maincpu->add_route(0, "lfilter", 1.0);
	m_maincpu->add_route(1, "rfilter", 1.0);

	// set up AC filters since the keyboard purposely outputs a DC offset when idle
	// TODO: there is also a RLC lowpass with R=120, L=3.3mH, C=0.33uF
	FILTER_RC(config, "lfilter").set_ac().add_route(ALL_OUTPUTS, "speaker", 1.0, 0);
	FILTER_RC(config, "rfilter").set_ac().add_route(ALL_OUTPUTS, "speaker", 1.0, 1);

	SPEAKER(config, "speaker", 2).front();
}

void psr150_state::psr78(machine_config &config)
{
	psr190_base(config);
	// pull up the button select bits, but not the LCD enable bit
	m_maincpu->port_out_cb<5>().set(&psr150_state::port_pullup_w<5, 0x1f>, "port_pullup_w");
	m_maincpu->add_route(0, "filter", 1.0);

	// set up AC filter since the keyboard purposely outputs a DC offset when idle
	// TODO: there is also a RLC lowpass with R=120, L=1mH, C=0.33uF
	FILTER_RC(config, "filter").set_ac().add_route(ALL_OUTPUTS, "speaker", 1.0);

	SPEAKER(config, "speaker").front_center();
}

// helper to use keys_w with PORT_WRITE_LINE_MEMBER
#define KEY_OUT_BITS(start, count) NAME((&psr150_state::keys_w<start, count>))

INPUT_PORTS_START(psr150)
	PORT_START("PA")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(psr150_state::keys_r<0>))
	PORT_BIT( 0xc0, IP_ACTIVE_LOW,  IPT_OUTPUT)  PORT_WRITE_LINE_MEMBER(KEY_OUT_BITS(8, 2))

	PORT_START("PB")
	PORT_BIT( 0xff, IP_ACTIVE_LOW,  IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(KEY_OUT_BITS(0, 8))

	PORT_START("PC")
	PORT_BIT( 0x03, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(psr150_state::keys_r<6>))
	PORT_BIT( 0x1c, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(psr150_state::switch_r))
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("PF")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW,  IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(KEY_OUT_BITS(11, 4))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(KEY_OUT_BITS(10, 1))
	PORT_BIT( 0xc0, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_START("SWITCH")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Mode (Song)")  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(psr150_state::switch_w), 0x1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Mode (Voice)") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(psr150_state::switch_w), 0x2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Mode (Style)") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(psr150_state::switch_w), 0x4)

	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_C6
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_G5
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_B5
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_AS5
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_A5
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_GS5
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_FS4
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_CS4
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_F4
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_E4
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_DS4
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_D4
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_C4
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_G3
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_B3
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_AS3
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_A3
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_GS3
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_C5
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_G4
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_B4
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_AS4
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_A4
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_GS4
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_FS1
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_CS1
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_F1
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_E1
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_DS1
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_D1
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_C2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_G1
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_B1
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_AS1
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_A1
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_GS1
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_FS2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_CS2
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_F2
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_E2
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_DS2
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_D2
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_FS3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_CS3
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_F3
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_E3
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_DS3
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_D3
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_FS5
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_CS5
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_F5
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_E5
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_DS5
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_D5
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY9")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_C1
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY10")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_C3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_G2
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_B2
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_AS2
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_A2
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_GS2
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY11")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_NAME("SE Pad 1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_NAME("SE Pad -")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Demo")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Keypad 6")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Keypad 5")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_NAME("SE Pad 2")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("Keypad 8")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("Keypad 9")

	PORT_START("KEY12")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_NAME("SE Pad 3")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_NAME("SE Pad +")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SPACE) PORT_NAME("Start / Stop")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Keypad 0")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Keypad 1")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_NAME("SE Pad 4")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Keypad 2")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Keypad 3")

	PORT_START("KEY13")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_NAME("Drum Pad 1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("Drum Pad -")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("Keypad -")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("Keypad +")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_NAME("Drum Pad 2")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("Keypad 7")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Keypad 4")

	PORT_START("KEY14")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Drum Pad 3")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("Drum Pad +")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_UP) PORT_NAME("Volume Up")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Volume Down")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_NAME("Drum Pad 4")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Tempo Up")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Tempo Down")
INPUT_PORTS_END

INPUT_PORTS_START(psr110)
	PORT_START("PA")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(psr150_state::keys_r<0>))
	PORT_BIT( 0xc0, IP_ACTIVE_LOW,  IPT_OUTPUT)  PORT_WRITE_LINE_MEMBER(KEY_OUT_BITS(7, 2))

	PORT_START("PB")
	PORT_BIT( 0x7f, IP_ACTIVE_LOW,  IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(KEY_OUT_BITS(0, 7))
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_START("PC")
	PORT_BIT( 0x1f, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(psr150_state::keys_r<6>))
	PORT_BIT( 0xe0, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(psr150_state::switch_r))

	PORT_START("PF")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW,  IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(KEY_OUT_BITS(9, 6))
	PORT_BIT( 0xc0, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_START("SWITCH")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Mode (Style)")  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(psr150_state::switch_w), 0x1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Mode (Voice)") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(psr150_state::switch_w), 0x2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Mode (Song)") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(psr150_state::switch_w), 0x4)

	PORT_START("KEY0")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_G3
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_GS3
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_A3
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_AS3
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_B3
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_C4
	PORT_BIT( 0x7c0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY1")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_CS3
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_D3
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_DS3
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_E3
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_F3
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_FS3
	PORT_BIT( 0x7c0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY2")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_G2
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_GS2
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_A2
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_AS2
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_B2
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_C3
	PORT_BIT( 0x7c0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY3")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_CS2
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_D2
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_DS2
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_E2
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_F2
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_FS2
	PORT_BIT( 0x7c0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY4")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_G1
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_GS1
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_A1
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_AS1
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_B1
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_C2
	PORT_BIT( 0x7c0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY5")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_CS1
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_D1
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_DS1
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_E1
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_F1
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_FS1
	PORT_BIT( 0x7c0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY6")
	PORT_BIT( 0x01f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_C1
	PORT_BIT( 0x7c0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY7")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_G4
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_GS4
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_A4
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_AS4
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_B4
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_C5
	PORT_BIT( 0x7c0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY8")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_CS4
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_D4
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_DS4
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_E4
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_F4
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_FS4
	PORT_BIT( 0x7c0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY9")
	PORT_BIT( 0x07f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Tempo Down")
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Tempo Up")
	PORT_BIT( 0x200, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Volume Down")
	PORT_BIT( 0x400, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("Keypad -")

	PORT_START("KEY10")
	PORT_BIT( 0x03f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("Keypad 8")
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Keypad 5")
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("Keypad 7")
	PORT_BIT( 0x200, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Keypad 4")
	PORT_BIT( 0x400, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("Keypad +")

	PORT_START("KEY11")
	PORT_BIT( 0x07f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Keypad 3")
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Keypad 2")
	PORT_BIT( 0x200, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Keypad 1")
	PORT_BIT( 0x400, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Keypad 0")

	PORT_START("KEY12")
	PORT_BIT( 0x03f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Demo")
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SPACE) PORT_NAME("Start / Stop")
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_UP) PORT_NAME("Volume Up")
	PORT_BIT( 0x200, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("Keypad 9")
	PORT_BIT( 0x400, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Keypad 6")

	PORT_START("KEY13")
	PORT_BIT( 0x03f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("Drum/SE Pad +")
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("Drum/SE Pad -")
	PORT_BIT( 0x700, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY14")
	PORT_BIT( 0x03f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Drum/SE Pad 3")
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_NAME("Drum/SE Pad 2")
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_NAME("Drum/SE Pad 1")
	PORT_BIT( 0x200, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_NAME("Drum/SE Pad 4")
	PORT_BIT( 0x400, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

INPUT_PORTS_START(psr75)
	PORT_START("PA")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(psr150_state::keys_r<0>))
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(KEY_OUT_BITS(8, 1))
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_START("PB")
	PORT_BIT( 0xff, IP_ACTIVE_LOW,  IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(KEY_OUT_BITS(0, 8))

	PORT_START("PC")
	PORT_BIT( 0x03, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(psr150_state::keys_r<6>))
	PORT_BIT( 0x1c, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(psr150_state::switch_r))
	PORT_BIT( 0xe0, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_START("PF")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW,  IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(KEY_OUT_BITS(9, 4))
	PORT_BIT( 0xf0, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_START("SWITCH")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Mode (Song)")  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(psr150_state::switch_w), 0x1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Mode (Voice)") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(psr150_state::switch_w), 0x2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Mode (Style)") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(psr150_state::switch_w), 0x4)

	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_C1
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_FS1
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_F1
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_E1
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_DS1
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_D1
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_CS1
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_C2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_B1
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_AS1
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_A1
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_GS1
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_G1
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_FS2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_F2
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_E2
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_DS2
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_D2
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_CS2
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_C3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_B2
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_AS2
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_A2
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_GS2
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_G2
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_FS3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_F3
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_E3
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_DS3
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_D3
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_CS3
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_C4
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_B3
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_AS3
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_A3
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_GS3
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_G3
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_FS4
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_F4
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_E4
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_DS4
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_D4
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_CS4
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_C5
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_B4
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_AS4
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_A4
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_GS4
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_G4
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY9")
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Demo")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Keypad 6")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Keypad 5")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("Keypad 8")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("Keypad 9")

	PORT_START("KEY10")
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SPACE) PORT_NAME("Start/Stop")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Keypad 0")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Keypad 1")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Keypad 2")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Keypad 3")

	PORT_START("KEY11")
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("Keypad -")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PLUS_PAD)  PORT_NAME("Keypad +")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("Keypad 7")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Keypad 4")

	PORT_START("KEY12")
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_UP)   PORT_NAME("Volume Up")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Volume Down")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Tempo Up")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_LEFT)  PORT_NAME("Tempo Down")
INPUT_PORTS_END

INPUT_PORTS_START(pss31)
	PORT_INCLUDE(psr110)

	PORT_MODIFY("KEY13")
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("Drum Pad +")
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("Drum Pad -")
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_NAME("Drum Pad 4")

	PORT_MODIFY("KEY14")
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Drum Pad 3")
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_NAME("Drum Pad 2")
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_NAME("Drum Pad 1")
	PORT_BIT( 0x200, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

INPUT_PORTS_START(pss21)
	PORT_INCLUDE(pss31)

	PORT_MODIFY("PA") // in & out bits are swapped
	PORT_BIT( 0x03, IP_ACTIVE_LOW,  IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(KEY_OUT_BITS(7, 2))
	PORT_BIT( 0xfc, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(psr150_state::keys_r<0>))

	PORT_MODIFY("KEY7")
	PORT_BIT( 0x7ff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("KEY8")
	PORT_BIT( 0x7ff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

INPUT_PORTS_START(pss11)
	PORT_START("PA")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW,  IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(KEY_OUT_BITS(0, 6))
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("PB")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(psr150_state::keys_r<0>))
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("PC")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW,  IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(KEY_OUT_BITS(6, 4))
	PORT_BIT( 0x70, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(psr150_state::switch_r))
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(psr150_state::keys_r<6>))

	PORT_START("PF")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(psr150_state::keys_r<7>))
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED ) // bits 6-7 indicate model

	PORT_START("SWITCH")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Mode (Style)")  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(psr150_state::switch_w), 0x1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Mode (Voice)") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(psr150_state::switch_w), 0x2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_NAME("Mode (Song)") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(psr150_state::switch_w), 0x4)

	PORT_START("KEY0")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_C5
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_B4
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_AS4
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_A4
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_GS4
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_G4
	PORT_BIT( 0x7c0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY1")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_FS4
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_F4
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_E4
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_DS4
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_D4
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_CS4
	PORT_BIT( 0x7c0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY2")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_C4
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_B3
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_AS3
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_A3
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_GS3
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_G3
	PORT_BIT( 0x7c0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY3")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_FS3
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_F3
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_E3
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_DS3
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_D3
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_CS3
	PORT_BIT( 0x7c0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY4")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_C3
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_B2
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_AS2
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_A2
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_GS2
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_G2
	PORT_BIT( 0x7c0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY5")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_FS2
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_F2
	PORT_BIT( 0x7fc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY6")
	PORT_BIT( 0x03f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Keypad 6")
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("Keypad 9")
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_UP) PORT_NAME("Volume Up")
	PORT_BIT( 0x200, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SPACE) PORT_NAME("Start / Stop")
	PORT_BIT( 0x400, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Demo")

	PORT_START("KEY7")
	PORT_BIT( 0x03f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Keypad 0")
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Keypad 1")
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Keypad 2")
	PORT_BIT( 0x200, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Keypad 3")
	PORT_BIT( 0x400, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY8")
	PORT_BIT( 0x03f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("Keypad +")
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Keypad 4")
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("Keypad 7")
	PORT_BIT( 0x200, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Keypad 5")
	PORT_BIT( 0x400, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("Keypad 8")

	PORT_START("KEY9")
	PORT_BIT( 0x03f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("Keypad -")
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Volume Down")
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Tempo Up")
	PORT_BIT( 0x200, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Tempo Down")
	PORT_BIT( 0x400, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

INPUT_PORTS_START( dd9 )
	PORT_START("PA")
	PORT_BIT( 0x1f, IP_ACTIVE_LOW,  IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(KEY_OUT_BITS(0, 5))
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("PF")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_NAME("Drum Pad 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_NAME("Drum Pad 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_BUTTON3 ) PORT_NAME("Drum Pad 3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_BUTTON4 ) PORT_NAME("Drum Pad 4")
	PORT_BIT( 0xf0, IP_ACTIVE_LOW,  IPT_CUSTOM  ) PORT_CUSTOM_MEMBER(FUNC(psr150_state::keys_r<0>))

	PORT_START("KEY0")
	PORT_BIT( 0x1, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_NAME("Style")
	PORT_BIT( 0x2, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_NAME("Perc. Set")
	PORT_BIT( 0x4, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_NAME("Tempo")
	PORT_BIT( 0x8, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Start/Stop")

	PORT_START("KEY1")
	PORT_BIT( 0x1, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_NAME("Demo")
	PORT_BIT( 0x2, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_NAME("Tap Start")
	PORT_BIT( 0x4, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_NAME("SE Select")
	PORT_BIT( 0x8, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_NAME("Auto Roll")

	PORT_START("KEY2")
	PORT_BIT( 0x1, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_UP) PORT_NAME("Volume Up")
	PORT_BIT( 0x2, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Volume Down")
	PORT_BIT( 0x4, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_NAME("Pad Assign +")
	PORT_BIT( 0x8, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_NAME("Pad Assign -")

	PORT_START("KEY3")
	PORT_BIT( 0x1, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("SE Pad 1")
	PORT_BIT( 0x2, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("SE Pad 2")
	PORT_BIT( 0x4, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("SE Pad 3")
	PORT_BIT( 0x8, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("SE Pad 4")

	PORT_START("KEY4")
	PORT_BIT( 0x3, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(psr150_state::dial_r))
	PORT_BIT( 0xc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DIAL")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(75)
INPUT_PORTS_END

INPUT_PORTS_START( pss12 )
	PORT_START("PA")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(psr150_state::keys_r<0>))
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("PB")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW,  IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(KEY_OUT_BITS(0, 6))
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("PC")
	PORT_BIT( 0x1f, IP_ACTIVE_LOW,  IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(KEY_OUT_BITS(6, 5))
	PORT_BIT( 0xe0, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(psr150_state::keys_r<6>))

	PORT_START("PF") // LEDs use two bits each and are numbered highest to lowest
	PORT_BIT( 0x03, IP_ACTIVE_LOW,  IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(FUNC(psr150_state::led_w<3>))
	PORT_BIT( 0x0c, IP_ACTIVE_LOW,  IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(FUNC(psr150_state::led_w<2>))
	PORT_BIT( 0x30, IP_ACTIVE_LOW,  IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(FUNC(psr150_state::led_w<1>))
	PORT_BIT( 0xc0, IP_ACTIVE_LOW,  IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(FUNC(psr150_state::led_w<0>))

	PORT_START("KEY0")
	PORT_BIT( 0x00f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_F2
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_FS2
	PORT_BIT( 0x1c0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY1")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_G2
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_GS2
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_A2
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_AS2
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_B2
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_C3
	PORT_BIT( 0x1c0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY2")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_CS3
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_D3
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_DS3
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_E3
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_F3
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_FS3
	PORT_BIT( 0x1c0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY3")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_G3
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_GS3
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_A3
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_AS3
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_B3
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_C4
	PORT_BIT( 0x1c0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY4")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_CS4
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_D4
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_DS4
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_E4
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_F4
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_FS4
	PORT_BIT( 0x1c0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY5")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_G4
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_GS3
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_A4
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_AS4
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_B4
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_OTHER )  PORT_GM_C5
	PORT_BIT( 0x1c0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY6")
	PORT_BIT( 0x03f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1)     PORT_NAME("Voice/Song 1")
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8)     PORT_NAME("One Touch Setting")
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Tempo Up")

	PORT_START("KEY7")
	PORT_BIT( 0x03f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2)  PORT_NAME("Voice/Song 2")
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9)  PORT_NAME("Minus One")
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_UP) PORT_NAME("Volume Up")

	PORT_START("KEY8")
	PORT_BIT( 0x03f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3)    PORT_NAME("Voice/Song 3")
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7)    PORT_NAME("Mode Select")
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Tempo Down")

	PORT_START("KEY9")
	PORT_BIT( 0x03f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4)    PORT_NAME("Voice/Song 4")
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0)    PORT_NAME("Start / Stop")
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Volume Down")

	PORT_START("KEY10")
	PORT_BIT( 0x03f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_NAME("Voice/Song 5")
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_NAME("Voice/Song 6 / Variation")
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

INPUT_PORTS_START( pss6 )
	PORT_START("PB") // LEDs use two bits each
	PORT_BIT( 0x03, IP_ACTIVE_LOW,  IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(FUNC(psr150_state::led_w<0>))
	PORT_BIT( 0x0c, IP_ACTIVE_LOW,  IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(FUNC(psr150_state::led_w<1>))
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("PC")
	PORT_BIT( 0xff, IP_ACTIVE_LOW,  IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(KEY_OUT_BITS(0, 8))

	PORT_START("PF")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(psr150_state::keys_r<0>))
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_C5
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_E4
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_GS3
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_DS3
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_AS2
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_F2
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_B4
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_DS4
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_G3
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_D3
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_A2
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_AS4
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_D4
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_FS3
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_CS3
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_GS2
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_A4
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_CS4
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_F3
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_C3
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_G2
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_GS4
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_C4
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_E3
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_B2
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_FS2
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_G4
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_B3
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_NAME("Voice/Song 6 / Variation")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_NAME("Voice/Song 2")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Start / Stop")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_NAME("Tempo")
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_FS4
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_AS3
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_NAME("Voice/Song 5")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_NAME("Voice/Song 3")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_NAME("Mode Select")
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_F4
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_A3
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4)     PORT_NAME("Voice/Song 4")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1)     PORT_NAME("Voice/Song 1")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8)     PORT_NAME("Minus One")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("Volume")
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

INPUT_PORTS_START(psr180_keys) // also psr190
	PORT_START("PA")
	PORT_BIT( 0xff, IP_ACTIVE_LOW,  IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(KEY_OUT_BITS(0, 8))

	PORT_START("PB")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(psr150_state::keys_r<0>))
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("PF")
	PORT_BIT( 0x1f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW,  IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(KEY_OUT_BITS(8, 3))

	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_B2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_G2
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_C3
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_AS2
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_A2
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_GS2
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY1")
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_C1
	PORT_BIT( 0xf8, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_F2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_CS2
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_FS2
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_E2
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_DS2
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_D2
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_B1
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_G1
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_C2
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_AS1
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_A1
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_GS1
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_F1
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_CS1
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_FS1
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_E1
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_DS1
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_D1
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_F4
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_CS4
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_FS4
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_E4
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_DS4
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_D4
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_F3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_CS3
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_FS3
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_E3
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_DS3
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_D3
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_B3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_G3
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_C4
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_AS3
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_A3
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_GS3
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_B4
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_G4
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_C5
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_AS4
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_A4
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_GS4
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY9")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_F5
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_CS5
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_FS5
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_E5
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_DS5
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_D5
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY10")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_B5
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_G5
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_C6
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_AS5
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_A5
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_GS5
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

INPUT_PORTS_START(psr76_keys) // also psr78
	PORT_START("PA")
	PORT_BIT( 0xff, IP_ACTIVE_LOW,  IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(KEY_OUT_BITS(0, 8))

	PORT_START("PB")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(KEY_OUT_BITS(8, 1))
	PORT_BIT( 0x7e, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(psr150_state::keys_r<0>))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_CS3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_D3
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_DS3
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_E3
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_F3
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_FS3
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_G4
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_GS4
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_A4
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_AS4
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_B4
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_C5
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_G2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_GS2
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_A2
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_AS2
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_B2
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_C3
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_CS2
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_D2
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_DS2
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_E2
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_F2
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_FS2
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY4")
	PORT_BIT( 0x1f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_C1
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_G3
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_GS3
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_A3
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_AS3
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_B3
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_C4
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_CS1
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_D1
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_DS1
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_E1
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_F1
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_FS1
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_CS4
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_D4
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_DS4
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_E4
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_F4
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_FS4
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_G1
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_GS1
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_A1
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_AS1
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_B1
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_GM_C2
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

INPUT_PORTS_START(psr180)
	PORT_INCLUDE(psr180_keys)

	PORT_START("PC")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(FUNC(psr150_state::apo_w))
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_POWER_ON ) PORT_NAME("Power") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(psr150_state::power_w), 0)
	PORT_BIT( 0x1c, IP_ACTIVE_LOW,  IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(FUNC(psr150_state::pwm_row_w))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(FUNC(psr150_state::pwm_col_w<7>)) // H
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(FUNC(psr150_state::pwm_col_w<3>)) // D
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(FUNC(psr150_state::pwm_col_w<4>)) // E

	PORT_MODIFY("PF")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(FUNC(psr150_state::pwm_col_w<1>)) // B
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(FUNC(psr150_state::pwm_col_w<0>)) // A
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(FUNC(psr150_state::pwm_col_w<5>)) // F
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(FUNC(psr150_state::pwm_col_w<6>)) // G
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(FUNC(psr150_state::pwm_col_w<2>)) // C

	PORT_START("KEY11")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Drum")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_NAME("Drum/SE Pad 3")
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY12")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("Sound Effect")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("Drum/SE Pad 4")
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY13")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("Keypad -")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_NAME("Voice")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("Keypad 7")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Keypad 4")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Keypad 1")
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY14")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("Keypad +")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Tempo Down")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("Keypad 9")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Keypad 6")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Keypad 3")
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY15")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_NAME("One Touch Setting")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Tempo Up")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_NAME("Start / Stop")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_NAME("Minus One")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_NAME("Song")
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY16")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_NAME("Sync Start / Fill In")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Drum/SE Pad 2")
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY17")
	PORT_BIT( 0x1f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_NAME("Drum/SE Pad 1")
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY18")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Keypad 0")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_NAME("Style")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("Keypad 8")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Keypad 5")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Keypad 2")
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

INPUT_PORTS_START(psr76)
	PORT_INCLUDE(psr76_keys)

	PORT_START("PC")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(FUNC(psr150_state::apo_w))
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_POWER_ON ) PORT_NAME("Power") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(psr150_state::power_w), 0)
	PORT_BIT( 0x1c, IP_ACTIVE_LOW,  IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(FUNC(psr150_state::pwm_row_w))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(FUNC(psr150_state::pwm_col_w<6>)) // G
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(FUNC(psr150_state::pwm_col_w<5>)) // F
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(FUNC(psr150_state::pwm_col_w<0>)) // A

	PORT_START("PF")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(FUNC(psr150_state::pwm_col_w<1>)) // B
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(FUNC(psr150_state::pwm_col_w<4>)) // E
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(FUNC(psr150_state::pwm_col_w<3>)) // D
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(FUNC(psr150_state::pwm_col_w<7>)) // H
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(FUNC(psr150_state::pwm_col_w<2>)) // C
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY12")
	PORT_BIT( 0x1f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_NAME("Sync Start / Fill In")
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY13")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Keypad 1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Keypad 4")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("Keypad 7")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("Keypad -")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_NAME("Voice")
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY14")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Keypad 3")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Keypad 6")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("Keypad 9")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("Keypad +")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Tempo Down")
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY15")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_NAME("Song / Demo")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_NAME("Minus One")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_NAME("Start / Stop")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_NAME("One Touch Setting")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Tempo Up")
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY18")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Keypad 2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Keypad 5")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("Keypad 8")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Keypad 0")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_NAME("Style")
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

INPUT_PORTS_START(psr190)
	PORT_INCLUDE(psr180_keys)

	PORT_MODIFY("PB")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("lcdc", FUNC(hd44780_device::e_w))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("PC_R")
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x78, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(psr150_state::lcd_r))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("PC_W")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("lcdc", FUNC(hd44780_device::rs_w))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("lcdc", FUNC(hd44780_device::rw_w))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x78, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(FUNC(psr150_state::lcd_w))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("PF")
	PORT_BIT( 0x1f, IP_ACTIVE_LOW,  IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(KEY_OUT_BITS(11, 5))

	PORT_START("KEY11")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_NAME("Sync Start")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_NAME("Main / Auto Fill B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Tempo Up")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_NAME("Demo")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Keypad 6")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("Keypad +")
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY12")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_NAME("Start / Stop")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_NAME("Ending")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_NAME("One Touch Setting")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("Voice")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("Keypad -")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Keypad 0")
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY13")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_NAME("Intro")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_NAME("Jam Track")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("Style")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Acc. Volume Down")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Keypad 5")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("Keypad 9")
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY14")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_UP) PORT_NAME("Acc. Volume Up")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Large/Small / Minus One")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Song")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Keypad 1")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Keypad 4")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("Keypad 8")
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY15")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_NAME("Main / Auto Fill A")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Tempo Down")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Keypad 2")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Keypad 3")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("Keypad 7")
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

INPUT_PORTS_START(psr78)
	PORT_INCLUDE(psr76_keys)

	PORT_START("PC_R")
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x78, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(psr150_state::lcd_r))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("PC_W")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("lcdc", FUNC(hd44780_device::rs_w))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("lcdc", FUNC(hd44780_device::rw_w))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x78, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(FUNC(psr150_state::lcd_w))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("PF")
	PORT_BIT( 0x1f, IP_ACTIVE_LOW,  IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(KEY_OUT_BITS(9, 5))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("lcdc", FUNC(hd44780_device::e_w))
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY9")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("Keypad +")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Keypad 6")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_NAME("Demo")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_NAME("Sync Start")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_NAME("Main / Auto Fill B")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Tempo Up")
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY10")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Keypad 0")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("Keypad -")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("Voice")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_NAME("Start / Stop")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_NAME("Ending")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_NAME("One Touch Setting")
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY11")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("Keypad 9")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Keypad 5")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Acc. Volume Down")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_NAME("Intro")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_NAME("Jam Track")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("Style")
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY12")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("Keypad 8")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Keypad 4")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Keypad 1")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_UP) PORT_NAME("Acc. Volume Up")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Large/Small / Minus One")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Song")
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY13")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("Keypad 7")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Keypad 3")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Keypad 2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_NAME("Main / Auto Fill A")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Tempo Down")
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

#undef KEY_OUT_BITS


ROM_START( psr150 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "xl561a00.ic4", 0x00000, 0x80000, CRC(ccb11ccf) SHA1(d4c00e4ee07e4bbe919e3cbeaf397ae1601533e3))
ROM_END

ROM_START( psr110 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "xm041a00.ic2", 0x00000, 0x80000, CRC(b2db9672) SHA1(dee46ca980f88a4f06fd88437e39c9a6b8cec7bf))
ROM_END

ROM_START( psr75 )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "xl437a00.ic4", 0x00000, 0x40000, CRC(75fba08f) SHA1(ad79cdc7026ed621004e8f600d61ee62c598fa1a))
ROM_END

#define rom_pss11 rom_psr75
#define rom_pss21 rom_psr75
#define rom_pss31 rom_psr75

ROM_START( dd9 )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "xp170b00.ic2", 0x00000, 0x40000, CRC(66f09612) SHA1(7b106dc3717992c5b1a96bd5b27417bd98b38f7f))
ROM_END

ROM_START( psr180 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "xp687a00.ic6", 0x00000, 0x80000, CRC(df3a568d) SHA1(ddc260d55d874987950817817117df141668f1f2))
ROM_END

ROM_START( psr76 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "xp686a00.ic6", 0x00000, 0x80000, CRC(0d5299fc) SHA1(744e63edc1089cb2ece2105fd53993a66c6dc2ab))
ROM_END

ROM_START( pss12 )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "xp598a00.ic2", 0x00000, 0x40000, CRC(7e05f1cb) SHA1(1a05996002bb7bfdde215349d235269795c88693))
ROM_END

ROM_START( pss6 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "xp733a00.ic2", 0x00000, 0x20000, CRC(5a7ad160) SHA1(01e7b988db37a2553c71d51f585736df286a245c))
ROM_END

ROM_START( psr190 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "xr814100.ic6", 0x00000, 0x80000, CRC(91743a1d) SHA1(7124d4a53667e41d03c3bc4ac08f991886bc5e42))

	ROM_REGION( 175935, "screen", 0 )
	ROM_LOAD( "psr190.svg", 0, 175935, CRC(1382c3df) SHA1(504ca22300f220785d1d7476e39cf2077684bb70))
ROM_END

ROM_START( psr78 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "xr813100.ic6", 0x00000, 0x80000, CRC(cf1b959c) SHA1(6459a322789285ab203bbbd29ddfe0d877514b41))

	ROM_REGION( 175935, "screen", 0 )
	ROM_LOAD( "psr190.svg", 0, 175935, CRC(1382c3df) SHA1(504ca22300f220785d1d7476e39cf2077684bb70))
ROM_END

} // anonymous namespace

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT   CLASS          INIT         COMPANY   FULLNAME   FLAGS
SYST( 1992, psr150,  0,      0,      psr150,  psr150, psr150_state,  empty_init,  "Yamaha", "PSR-150", MACHINE_SUPPORTS_SAVE )
SYST( 1993, psr110,  psr150, 0,      psr110,  psr110, psr150_state,  empty_init,  "Yamaha", "PSR-110", MACHINE_SUPPORTS_SAVE )
SYST( 1992, psr75,   psr150, 0,      psr75,   psr75,  psr150_state,  empty_init,  "Yamaha", "PSR-75",  MACHINE_SUPPORTS_SAVE )
SYST( 1992, pss11,   psr150, 0,      pss11,   pss11,  psr150_state,  empty_init,  "Yamaha", "PSS-11",  MACHINE_SUPPORTS_SAVE )
SYST( 1992, pss21,   psr150, 0,      pss21,   pss21,  psr150_state,  empty_init,  "Yamaha", "PSS-21",  MACHINE_SUPPORTS_SAVE )
SYST( 1992, pss31,   psr150, 0,      pss31,   pss31,  psr150_state,  empty_init,  "Yamaha", "PSS-31",  MACHINE_SUPPORTS_SAVE )
SYST( 1994, dd9,     0,      0,      dd9,     dd9,    psr150_state,  empty_init,  "Yamaha", "DD-9 Digital Percussion", MACHINE_SUPPORTS_SAVE )
SYST( 1994, psr180,  0,      0,      psr180,  psr180, psr150_state,  empty_init,  "Yamaha", "PSR-180", MACHINE_SUPPORTS_SAVE )
SYST( 1994, psr76,   psr180, 0,      psr76,   psr76,  psr150_state,  empty_init,  "Yamaha", "PSR-76",  MACHINE_SUPPORTS_SAVE )
SYST( 1994, pss12,   0,      0,      pss12,   pss12,  psr150_state,  empty_init,  "Yamaha", "PSS-12",  MACHINE_SUPPORTS_SAVE )
SYST( 1994, pss6,    pss12,  0,      pss6,    pss6,   psr150_state,  empty_init,  "Yamaha", "PSS-6",   MACHINE_SUPPORTS_SAVE )
SYST( 1996, psr190,  0,      0,      psr190,  psr190, psr150_state,  empty_init,  "Yamaha", "PSR-190", MACHINE_SUPPORTS_SAVE )
SYST( 1996, psr78,   psr190, 0,      psr78,   psr78,  psr150_state,  empty_init,  "Yamaha", "PSR-78",  MACHINE_SUPPORTS_SAVE )
