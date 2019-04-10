// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*
    DataEast/Sega Version 1 and 2

    Main CPU: 6808 @ 4MHz (internally divided by 4)
    Audio CPU: 68B09E @ 2MHz
    Audio: YM2151 @ 3.58MHz, MSM5205 @ 384kHz
*/

#include "emu.h"

#include "machine/decopincpu.h"
#include "machine/genpin.h"

#include "cpu/m6800/m6800.h"
#include "cpu/m6809/m6809.h"
#include "machine/6821pia.h"
#include "sound/msm5205.h"
#include "sound/ym2151.h"
#include "speaker.h"

#include "de2.lh"
#include "de2a3.lh"


// To start Secret Service, hold I, O and Left ALT while pressing Start.
// To start Laser War, hold S, D, and F while pressing Start.
// To start Back to the Future, hold D and F while pressing Start.
// To start The Simpsons, hold D, F and G while pressing Start (can be tempremental)

// Data East CPU board is similar to Williams System 11, but without the generic audio board.
// For now, we'll presume the timings are the same.

class de_2_state : public genpin_class
{
public:
	de_2_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_ym2151(*this, "ym2151")
		, m_audiocpu(*this, "audiocpu")
		, m_msm5205(*this, "msm5205")
		, m_sample_bank(*this, "sample_bank")
		, m_digits(*this, "digit%u", 0U)
		, m_diag_digit(*this, "digit60")
	{ }

	void de_type1(machine_config &config);
	void de_type2(machine_config &config);
	void de_type2_alpha3(machine_config &config);
	void de_type3(machine_config &config);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	void de_bg_audio(machine_config &config);
	void de_2_map(address_map &map);
	void de_2_audio_map(address_map &map);

	DECLARE_WRITE8_MEMBER(sample_w);
	DECLARE_WRITE8_MEMBER(pia34_pa_w);
	DECLARE_WRITE8_MEMBER(type2alpha3_pia34_pa_w);
	DECLARE_WRITE8_MEMBER(alpha3_pia34_pa_w);
	DECLARE_READ8_MEMBER(switch_r);
	DECLARE_WRITE8_MEMBER(switch_w);
	DECLARE_WRITE8_MEMBER(pia2c_pa_w);
	DECLARE_WRITE8_MEMBER(pia2c_pb_w);
	DECLARE_WRITE_LINE_MEMBER(pia28_ca2_w) { } // comma3&4
	DECLARE_WRITE_LINE_MEMBER(pia28_cb2_w) { } // comma1&2
	DECLARE_READ8_MEMBER(pia28_w7_r);
	DECLARE_WRITE8_MEMBER(dig0_w);
	DECLARE_WRITE8_MEMBER(dig1_w);
	DECLARE_WRITE8_MEMBER(type2alpha3_dig1_w);
	DECLARE_WRITE8_MEMBER(alpha3_dig1_w);
	DECLARE_WRITE8_MEMBER(lamp0_w);
	DECLARE_WRITE8_MEMBER(lamp1_w) { }
	DECLARE_WRITE_LINE_MEMBER(ym2151_irq_w);
	DECLARE_WRITE_LINE_MEMBER(msm5205_irq_w);
	DECLARE_WRITE8_MEMBER(sol2_w) { } // solenoids 8-15
	DECLARE_WRITE8_MEMBER(sol3_w);
	DECLARE_WRITE8_MEMBER(sound_w);
	DECLARE_WRITE_LINE_MEMBER(pia21_ca2_w);

	DECLARE_READ8_MEMBER(sound_latch_r);
	DECLARE_WRITE8_MEMBER(sample_bank_w);

	// devcb callbacks
	DECLARE_READ8_MEMBER(display_r);
	DECLARE_WRITE8_MEMBER(display_w);
	DECLARE_WRITE8_MEMBER(type2alpha3_display_w);
	DECLARE_WRITE8_MEMBER(type3_display_w);
	DECLARE_WRITE8_MEMBER(lamps_w);

	// devices
	required_device<ym2151_device> m_ym2151;
	required_device<cpu_device> m_audiocpu;
	required_device<msm5205_device> m_msm5205;
	required_memory_bank m_sample_bank;
	output_finder<32> m_digits;
	output_finder<> m_diag_digit;

	uint8_t m_sample_data;
	bool m_more_data;
	bool m_nmi_enable;

	uint32_t m_segment1;
	uint32_t m_segment2;
	uint8_t m_strobe;
	uint8_t m_kbdrow;
	uint8_t m_diag;
	bool m_ca1;
	uint8_t m_sound_data;

	uint8_t m_sample_bank_num;
	uint8_t m_msm_prescaler;
};


void de_2_state::de_2_map(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("nvram");
	map(0x2100, 0x2103).rw("pia21", FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // sound+solenoids
	map(0x2200, 0x2200).w(FUNC(de_2_state::sol3_w)); // solenoids
	map(0x2400, 0x2403).rw("pia24", FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // lamps
	map(0x2800, 0x2803).rw("pia28", FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // display
	map(0x2c00, 0x2c03).rw("pia2c", FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // alphanumeric display
	map(0x3000, 0x3003).rw("pia30", FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // inputs
	map(0x3400, 0x3403).rw("pia34", FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // widget
	map(0x4000, 0xffff).rom();
}

void de_2_state::de_2_audio_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x2000, 0x2001).rw(m_ym2151, FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x2400, 0x2400).r(FUNC(de_2_state::sound_latch_r));
	map(0x2800, 0x2800).w(FUNC(de_2_state::sample_bank_w));
	// 0x2c00        - 4052(?)
	map(0x3000, 0x3000).w(FUNC(de_2_state::sample_w));
	// 0x3800        - Watchdog reset
	map(0x4000, 0x7fff).bankr("sample_bank");
	map(0x8000, 0xffff).rom();
}

static INPUT_PORTS_START( de_2 )
	PORT_START("INP0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("INP1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER )

	PORT_START("INP2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_K)

	PORT_START("INP4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_COMMA)

	PORT_START("INP8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_STOP)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_SLASH)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_COLON)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_MINUS)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSPACE)

	PORT_START("INP10")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_ENTER)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_LEFT)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_UP)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_DOWN)

	PORT_START("INP20")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_O)

	PORT_START("INP40")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_LALT)
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("INP80")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

INPUT_PORTS_END


void de_2_state::machine_reset()
{
	genpin_class::machine_reset();

	m_sample_bank->set_entry(0);
	m_more_data = false;
}

void de_2_state::machine_start()
{
	genpin_class::machine_start();

	m_digits.resolve();
	m_diag_digit.resolve();

	uint8_t *const ROM = memregion("sound1")->base();
	m_sample_bank->configure_entries(0, 16, &ROM[0x0000], 0x4000);
	m_sample_bank->set_entry(0);
}

WRITE_LINE_MEMBER(de_2_state::ym2151_irq_w)
{
	m_audiocpu->set_input_line(M6809_IRQ_LINE,state);
}

WRITE_LINE_MEMBER(de_2_state::msm5205_irq_w)
{
	m_msm5205->write_data(m_sample_data >> 4);
	if(m_more_data)
	{
		if(m_nmi_enable)
			m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);  // generate NMI when we need more data
		m_more_data = false;
	}
	else
	{
		m_more_data = true;
		m_sample_data <<= 4;
	}
}

// 6821 PIA at 0x2100
WRITE8_MEMBER( de_2_state::sol3_w )
{
}

WRITE8_MEMBER( de_2_state::sound_w )
{
	m_sound_data = data;
	m_audiocpu->set_input_line(M6809_FIRQ_LINE, ASSERT_LINE);
}

WRITE_LINE_MEMBER( de_2_state::pia21_ca2_w )
{
// sound ns
	m_ca1 = state;
}

// 6821 PIA at 0x2400
WRITE8_MEMBER( de_2_state::lamp0_w )
{
}

// 6821 PIA at 0x2800
WRITE8_MEMBER( de_2_state::dig0_w )
{
	static const uint8_t patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0x58, 0x4c, 0x62, 0x69, 0x78, 0 }; // 7447
	data &= 0x7f;
	m_strobe = data & 15;
	m_diag = (data & 0x70) >> 4;
	m_diag_digit = patterns[data>>4]; // diag digit
	m_segment1 = 0;
	m_segment2 = 0;
}

WRITE8_MEMBER( de_2_state::dig1_w )
{
	m_segment2 |= data;
	m_segment2 |= 0x30000;
	if ((m_segment2 & 0x70000) == 0x30000)
	{
		m_digits[m_strobe+16] = bitswap<16>(m_segment2, 11, 15, 12, 10, 8, 14, 13, 9, 7, 6, 5, 4, 3, 2, 1, 0);
		m_segment2 |= 0x40000;
	}
}

WRITE8_MEMBER( de_2_state::type2alpha3_dig1_w )
{
	m_segment2 |= data;
	m_segment2 |= 0x20000;
	if ((m_segment2 & 0x70000) == 0x30000)
	{
		m_digits[m_strobe+16] = bitswap<16>(m_segment2, 11, 15, 12, 10, 8, 14, 13, 9, 7, 6, 5, 4, 3, 2, 1, 0);
		m_segment2 |= 0x40000;
	}
}

WRITE8_MEMBER( de_2_state::alpha3_dig1_w )
{
	m_segment2 |= data;
	m_segment2 |= 0x20000;
	if ((m_segment2 & 0x70000) == 0x30000)
	{
		m_digits[m_strobe+16] = bitswap<16>(m_segment2, 7, 15, 12, 10, 8, 14, 13, 9, 11, 6, 5, 4, 3, 2, 1, 0);
		m_segment2 |= 0x40000;
	}
}

READ8_MEMBER( de_2_state::pia28_w7_r )
{
	uint8_t ret = 0x80;

	ret |= m_strobe;
	ret |= m_diag << 4;

	return ret;
}

// 6821 PIA at 0x2c00
WRITE8_MEMBER( de_2_state::pia2c_pa_w )
{
	m_segment1 |= (data<<8);
	m_segment1 |= 0x10000;
	if ((m_segment1 & 0x70000) == 0x30000)
	{
		m_digits[m_strobe] = bitswap<16>(m_segment1, 7, 15, 12, 10, 8, 14, 13, 9, 11, 6, 5, 4, 3, 2, 1, 0);
		m_segment1 |= 0x40000;
	}
}

WRITE8_MEMBER( de_2_state::pia2c_pb_w )
{
	m_segment1 |= data;
	m_segment1 |= 0x20000;
	if ((m_segment1 & 0x70000) == 0x30000)
	{
		m_digits[m_strobe] = bitswap<16>(m_segment1, 7, 15, 12, 10, 8, 14, 13, 9, 11, 6, 5, 4, 3, 2, 1, 0);
		m_segment1 |= 0x40000;
	}
}


// 6821 PIA at 0x3000
READ8_MEMBER( de_2_state::switch_r )
{
	char kbdrow[8];
	sprintf(kbdrow,"INP%X",m_kbdrow);
	return ~ioport(kbdrow)->read();
}

WRITE8_MEMBER( de_2_state::switch_w )
{
	int x;

	// about every second, 0xFF is written here, but it would be impossible to select more than one set of switches
	// at once, so just return the first bit set.  Maybe 0xFF has special meaning, or is just a disable?
	for(x=0;x<8;x++)
	{
		if(data & (1<<x))
			break;
	}
	m_kbdrow = data & (1<<x);
}

// 6821 PIA at 0x3400
WRITE8_MEMBER( de_2_state::pia34_pa_w )
{
	// Not connected on alphanumeric type 2 boards
}

WRITE8_MEMBER( de_2_state::type2alpha3_pia34_pa_w )
{
	m_segment2 |= (data<<8);
	m_segment2 |= 0x10000;
	if ((m_segment2 & 0x70000) == 0x30000)
	{
		m_digits[m_strobe+16] = bitswap<16>(m_segment2, 7, 15, 12, 10, 8, 14, 13, 9, 11, 6, 5, 4, 3, 2, 1, 0);
		m_segment2 |= 0x40000;
	}
}

WRITE8_MEMBER( de_2_state::alpha3_pia34_pa_w )
{
	m_segment2 |= (data<<8);
	m_segment2 |= 0x10000;
	if ((m_segment2 & 0x70000) == 0x30000)
	{
		m_digits[m_strobe+16] = bitswap<16>(m_segment2, 11, 15, 12, 10, 8, 14, 13, 9, 7, 6, 5, 4, 3, 2, 1, 0);
		m_segment2 |= 0x40000;
	}
}


// Sound board
WRITE8_MEMBER(de_2_state::sample_w)
{
	m_sample_data = data;
}

READ8_MEMBER( de_2_state::sound_latch_r )
{
	m_audiocpu->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE);
	return m_sound_data;
}

WRITE8_MEMBER( de_2_state::sample_bank_w )
{
	static constexpr uint8_t prescale[4] = { msm5205_device::S96_4B, msm5205_device::S48_4B, msm5205_device::S64_4B, 0 };

	m_sample_bank_num = (data & 0x07);
	m_sample_bank->set_entry(m_sample_bank_num);
	m_msm_prescaler = (data & 0x30) >> 4;
	m_nmi_enable = (~data & 0x80);
	m_msm5205->playmode_w(prescale[m_msm_prescaler]);
	m_msm5205->reset_w(data & 0x40);
}

READ8_MEMBER(de_2_state::display_r)
{
	uint8_t ret = 0x00;

	switch(offset)
	{
	case 0:
		ret = pia28_w7_r(space,0);
		break;
	}

	return ret;
}

WRITE8_MEMBER(de_2_state::display_w)
{
	switch(offset)
	{
	case 0:
		dig0_w(space,0,data);
		break;
	case 1:
		dig1_w(space,0,data);
		break;
	case 2:
		pia2c_pa_w(space,0,data);
		break;
	case 3:
		pia2c_pb_w(space,0,data);
		break;
	case 4:
		pia34_pa_w(space,0,data);
		break;
	}
}

WRITE8_MEMBER(de_2_state::type2alpha3_display_w)
{
	switch(offset)
	{
	case 0:
		dig0_w(space,0,data);
		break;
	case 1:
		type2alpha3_dig1_w(space,0,data);
		break;
	case 2:
		pia2c_pa_w(space,0,data);
		break;
	case 3:
		pia2c_pb_w(space,0,data);
		break;
	case 4:
		type2alpha3_pia34_pa_w(space,0,data);
		break;
	}
}

WRITE8_MEMBER(de_2_state::type3_display_w)
{
	switch(offset)
	{
	case 0:
		dig0_w(space,0,data);
		break;
	case 1:
		alpha3_dig1_w(space,0,data);
		break;
	case 2:
		pia2c_pa_w(space,0,data);
		break;
	case 3:
		pia2c_pb_w(space,0,data);
		break;
	case 4:
		alpha3_pia34_pa_w(space,0,data);
		break;
	}
}

WRITE8_MEMBER(de_2_state::lamps_w)
{
	switch(offset)
	{
	case 0:
		lamp0_w(space,0,data);
		break;
	case 1:
		lamp1_w(space,0,data);
		break;
	}
}


void de_2_state::de_bg_audio(machine_config &config)
{
	/* sound CPU */
	MC6809E(config, m_audiocpu, XTAL(8'000'000) / 4); // MC68B09E
	m_audiocpu->set_addrmap(AS_PROGRAM, &de_2_state::de_2_audio_map);

	SPEAKER(config, "bg").front_center();

	YM2151(config, m_ym2151, XTAL(3'579'545));
	m_ym2151->irq_handler().set(FUNC(de_2_state::ym2151_irq_w));
	m_ym2151->add_route(ALL_OUTPUTS, "bg", 0.50);

	MSM5205(config, m_msm5205, XTAL(384'000));
	m_msm5205->vck_legacy_callback().set(FUNC(de_2_state::msm5205_irq_w));
	m_msm5205->set_prescaler_selector(msm5205_device::S96_4B);
	m_msm5205->add_route(ALL_OUTPUTS, "bg", 0.50);
}

void de_2_state::de_type1(machine_config &config)
{
	/* basic machine hardware */
	decocpu_type1_device &decocpu(DECOCPU1(config, "decocpu", XTAL(8'000'000) / 2, "maincpu"));
	decocpu.display_read_callback().set(FUNC(de_2_state::display_r));
	decocpu.display_write_callback().set(FUNC(de_2_state::display_w));
	decocpu.soundlatch_write_callback().set(FUNC(de_2_state::sound_w));
	decocpu.switch_read_callback().set(FUNC(de_2_state::switch_r));
	decocpu.switch_write_callback().set(FUNC(de_2_state::switch_w));
	decocpu.lamp_write_callback().set(FUNC(de_2_state::lamps_w));

	/* Video */
	config.set_default_layout(layout_de2);

	genpin_audio(config);
	de_bg_audio(config);
}

void de_2_state::de_type2(machine_config &config)
{
	/* basic machine hardware */
	decocpu_type2_device &decocpu(DECOCPU2(config, "decocpu", XTAL(8'000'000) / 2, "maincpu"));
	decocpu.display_read_callback().set(FUNC(de_2_state::display_r));
	decocpu.display_write_callback().set(FUNC(de_2_state::display_w));
	decocpu.soundlatch_write_callback().set(FUNC(de_2_state::sound_w));
	decocpu.switch_read_callback().set(FUNC(de_2_state::switch_r));
	decocpu.switch_write_callback().set(FUNC(de_2_state::switch_w));
	decocpu.lamp_write_callback().set(FUNC(de_2_state::lamps_w));

	/* Video */
	config.set_default_layout(layout_de2);

	genpin_audio(config);
	de_bg_audio(config);
}

void de_2_state::de_type2_alpha3(machine_config &config)
{
	/* basic machine hardware */
	de_type2(config);
	subdevice<decocpu_type2_device>("decocpu")->display_write_callback().set(FUNC(de_2_state::type2alpha3_display_w));

	/* Video */
	config.set_default_layout(layout_de2a3);
}

void de_2_state::de_type3(machine_config &config)
{
	/* basic machine hardware */
	decocpu_type3_device &decocpu(DECOCPU3(config, "decocpu", XTAL(8'000'000) / 2, "maincpu"));
	decocpu.display_read_callback().set(FUNC(de_2_state::display_r));
	decocpu.display_write_callback().set(FUNC(de_2_state::type3_display_w));
	decocpu.soundlatch_write_callback().set(FUNC(de_2_state::sound_w));
	decocpu.switch_read_callback().set(FUNC(de_2_state::switch_r));
	decocpu.switch_write_callback().set(FUNC(de_2_state::switch_w));
	decocpu.lamp_write_callback().set(FUNC(de_2_state::lamps_w));

	/* Video */
	config.set_default_layout(layout_de2a3);

	genpin_audio(config);
	de_bg_audio(config);
}


/*--------------------------------------------------------------------------------
/ Back To the Future - CPU Rev 3 /Alpha Type 3 - 32K Roms - 32/64K Sound Roms
/--------------------------------------------------------------------------------*/
ROM_START(bttf_a28)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("bttfb5.2-8", 0x4000, 0x4000, CRC(a7dafa3c) SHA1(a29b8986d1886aa7bb7dea2521c3d7143ab75320))
	ROM_LOAD("bttfc5.2-8", 0x8000, 0x8000, CRC(5dc9928f) SHA1(03de05ed7b04ba86d695f03b1a3d65788faf2d4f))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("bttfsf7.rom", 0x8000, 0x8000, CRC(7673146e) SHA1(d6bd7cf39c78c8aff0b1a0b6cfd46a2a8ce9e086))
	ROM_REGION(0x1000000, "sound1", 0)
	ROM_LOAD("bttfsf6.rom", 0x00000, 0x10000, CRC(468a8d9c) SHA1(713cf84cc5f0531e2e9f7aaa58ebeb53c28ba395))
	ROM_LOAD("bttfsf5.rom", 0x10000, 0x10000, CRC(37a6f6b8) SHA1(ebd603d36527a2af25dcda1fde5cdf9a34d1f9cd))
ROM_END

ROM_START(bttf_a27)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("bttfb5.2-7", 0x4000, 0x4000, CRC(24b53174) SHA1(00a5e47e70ce4244873980c946479f0bbc414f2e))
	ROM_LOAD("bttfc5.2-7", 0x8000, 0x8000, CRC(c4d85d7e) SHA1(88bb91f9ed50335fc402b68983b49319c7dd4e99))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("bttfsf7.rom", 0x8000, 0x8000, CRC(7673146e) SHA1(d6bd7cf39c78c8aff0b1a0b6cfd46a2a8ce9e086))
	ROM_REGION(0x1000000, "sound1", 0)
	ROM_LOAD("bttfsf6.rom", 0x00000, 0x10000, CRC(468a8d9c) SHA1(713cf84cc5f0531e2e9f7aaa58ebeb53c28ba395))
	ROM_LOAD("bttfsf5.rom", 0x10000, 0x10000, CRC(37a6f6b8) SHA1(ebd603d36527a2af25dcda1fde5cdf9a34d1f9cd))
ROM_END

ROM_START(bttf_a20)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("bttfb5.2-0", 0x0000, 0x8000, CRC(c0d4df6b) SHA1(647d0d0a5af04f4255a588da41a6cdb2cf522875))
	ROM_LOAD("bttfc5.2-0", 0x8000, 0x8000, CRC(a189a189) SHA1(9669653280c78c811931ea3944817c717f3b5b77))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("bttfsf7.rom", 0x8000, 0x8000, CRC(7673146e) SHA1(d6bd7cf39c78c8aff0b1a0b6cfd46a2a8ce9e086))
	ROM_REGION(0x1000000, "sound1", 0)
	ROM_LOAD("bttfsf6.rom", 0x00000, 0x10000, CRC(468a8d9c) SHA1(713cf84cc5f0531e2e9f7aaa58ebeb53c28ba395))
	ROM_LOAD("bttfsf5.rom", 0x10000, 0x10000, CRC(37a6f6b8) SHA1(ebd603d36527a2af25dcda1fde5cdf9a34d1f9cd))
ROM_END

ROM_START(bttf_a21)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("bktofutr.b5", 0x4000, 0x4000, CRC(a651f867) SHA1(99cff09a06a99abac505c7732bb4ed985f0946e4))
	ROM_LOAD("bktofutr.c5", 0x8000, 0x8000, CRC(118ae58e) SHA1(a17e4cc3c12ca770e6e0674cfbeb55482739f735))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("bttfsf7.rom", 0x8000, 0x8000, CRC(7673146e) SHA1(d6bd7cf39c78c8aff0b1a0b6cfd46a2a8ce9e086))
	ROM_REGION(0x1000000, "sound1", 0)
	ROM_LOAD("bttfsf6.rom", 0x00000, 0x10000, CRC(468a8d9c) SHA1(713cf84cc5f0531e2e9f7aaa58ebeb53c28ba395))
	ROM_LOAD("bttfsf5.rom", 0x10000, 0x10000, CRC(37a6f6b8) SHA1(ebd603d36527a2af25dcda1fde5cdf9a34d1f9cd))
ROM_END

ROM_START(bttf_g27)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("bttfb5g.2-7", 0x4000, 0x4000, CRC(5e3e3cfa) SHA1(2d489c48463c7d28614d56aa566ffbc745bf6a8b))
	ROM_LOAD("bttfc5g.2-7", 0x8000, 0x8000, CRC(31dec6d0) SHA1(b0f9323ace3f6d96790be7fe2df67b974c291a29))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("bttfsf7.rom", 0x8000, 0x8000, CRC(7673146e) SHA1(d6bd7cf39c78c8aff0b1a0b6cfd46a2a8ce9e086))
	ROM_REGION(0x1000000, "sound1", 0)
	ROM_LOAD("bttfsf6.rom", 0x00000, 0x10000, CRC(468a8d9c) SHA1(713cf84cc5f0531e2e9f7aaa58ebeb53c28ba395))
	ROM_LOAD("bttfsf5.rom", 0x10000, 0x10000, CRC(37a6f6b8) SHA1(ebd603d36527a2af25dcda1fde5cdf9a34d1f9cd))
ROM_END

/*-------------------------------------------------------------------------------
/ King Kong - CPU Rev 3 /Alpha Type 3 16/32K Roms - 32/64K Sound Roms
/-------------------------------------------------------------------------------*/
ROM_START(kiko_a10)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("kkcpu_b5.bin", 0x4000, 0x4000, CRC(97b80fd2) SHA1(a704bda771bd44676a0de2f698a713d10feb01f3))
	ROM_LOAD("kkcpu_c5.bin", 0x8000, 0x8000, CRC(d42cab64) SHA1(ca4ceac34384804395b3e3035a430560f194846b))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("kksnd_f7.bin", 0x8000, 0x8000, CRC(fb1b3e11) SHA1(3c9a6958749d7e4dc5a1a57d6683e3cb3dc34890))
	ROM_REGION(0x1000000, "sound1", 0)
	ROM_LOAD("kkvoi_f5.bin", 0x00000, 0x10000, CRC(415f814c) SHA1(27e5b6b7f7ce2e5548ee9bf30966fa4f276bdc4d))
	ROM_LOAD("kkvoi_f4.bin", 0x10000, 0x10000, CRC(bbdc836c) SHA1(825a02b4f058d9dbc387035eb6533547d1766396))
ROM_END

/*-------------------------------------------------------------------
/ Laser War - CPU Rev 1 /Alpha Type 1 - 32K ROM - 32/64K Sound Roms
/-------------------------------------------------------------------*/
ROM_START(lwar_a83)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("lwar8-3.c5", 0x8000, 0x8000, CRC(eee158ee) SHA1(54db2342bdd15b16fee906dc65f183a957fd0012))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("lwar_e9.snd", 0x8000, 0x8000, CRC(9a6c834d) SHA1(c6e2c4658db4bd8dfcbb0351793837cdff30ba28))
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("lwar_e6.snd", 0x00000, 0x10000, CRC(7307d795) SHA1(5d88b8d883a2f17ca9fa30c7e7ac29c9f236ac4d))
	ROM_LOAD("lwar_e7.snd", 0x10000, 0x10000, CRC(0285cff9) SHA1(2c5e3de649e419ec7944059f2a226aaf58fe2af5))
ROM_END

ROM_START(lwar_a81)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("c100_g8.256", 0x8000, 0x8000, CRC(fe63ef04) SHA1(edab4b7fab4a016e653a546110a4bc8c563e7cb7))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("lwar_e9.snd", 0x8000, 0x8000, CRC(9a6c834d) SHA1(c6e2c4658db4bd8dfcbb0351793837cdff30ba28))
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("lwar_e6.snd", 0x00000, 0x10000, CRC(7307d795) SHA1(5d88b8d883a2f17ca9fa30c7e7ac29c9f236ac4d))
	ROM_LOAD("lwar_e7.snd", 0x10000, 0x10000, CRC(0285cff9) SHA1(2c5e3de649e419ec7944059f2a226aaf58fe2af5))
ROM_END

ROM_START(lwar_e90)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("lwar9-0.e5", 0x8000, 0x8000, CRC(b596151f) SHA1(10dade79ded71625770ec7e21ea50b7aa64023d0))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("lwar_e9.snd", 0x8000, 0x8000, CRC(9a6c834d) SHA1(c6e2c4658db4bd8dfcbb0351793837cdff30ba28))
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("lwar_e6.snd", 0x00000, 0x10000, CRC(7307d795) SHA1(5d88b8d883a2f17ca9fa30c7e7ac29c9f236ac4d))
	ROM_LOAD("lwar_e7.snd", 0x10000, 0x10000, CRC(0285cff9) SHA1(2c5e3de649e419ec7944059f2a226aaf58fe2af5))
ROM_END


/*-----------------------------------------------------------------------------------
/ Monday Night Football - CPU Rev 2 /Alpha Type 3 16/32K Roms - 32/64K Sound Roms
/----------------------------------------------------------------------------------*/
ROM_START(mnfb_c29)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("mnfb2-9.b5", 0x4000, 0x4000, BAD_DUMP CRC(2d6805d1) SHA1(f222cbf30d07975279eea210738f7d4f73b3fcf4)) // patched by PINMAME dev
	ROM_LOAD("mnfb2-9.c5", 0x8000, 0x8000, CRC(98d50cf5) SHA1(59d3b16f8195ab95cece71a12dab3349dfeb2c2b))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("mnf-f7.256", 0x8000, 0x8000, CRC(fbc2d6f6) SHA1(33173c081de776d32e926481e94b265ec48d770b))
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("mnf-f5-6.512", 0x00000, 0x10000, CRC(0c6ea963) SHA1(8c88fa588222ef8a6c872b8c5b49639b108384d4))
	ROM_LOAD("mnf-f4-5.512", 0x10000, 0x10000, CRC(efca5d80) SHA1(9655c885dd64aa170205170b6a0c052bd9367379))
ROM_END

ROM_START(mnfb_c27)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("mnfb2-7.b5", 0x4000, 0x4000, CRC(995eb9b8) SHA1(d05d74393fda59ffd8d7b5546313779cdb10d23e))
	ROM_LOAD("mnfb2-7.c5", 0x8000, 0x8000, CRC(579d81df) SHA1(9c96da34d37d3369513003e208222bd6e8698638))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("mnf-f7.256", 0x8000, 0x8000, CRC(fbc2d6f6) SHA1(33173c081de776d32e926481e94b265ec48d770b))
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("mnf-f5-6.512", 0x00000, 0x10000, CRC(0c6ea963) SHA1(8c88fa588222ef8a6c872b8c5b49639b108384d4))
	ROM_LOAD("mnf-f4-5.512", 0x10000, 0x10000, CRC(efca5d80) SHA1(9655c885dd64aa170205170b6a0c052bd9367379))
ROM_END

/*-------------------------------------------------------------------------------
/ Phantom of the Opera - CPU Rev 3 /Alpha Type 3 16/32K Roms - 32/64K Sound Roms
/-------------------------------------------------------------------------------*/
// Display LED inputs are in the same order as for Monday Night Football, is this actually CPU type 2?
ROM_START(poto_a32)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("potob5.3-2", 0x4000, 0x4000, CRC(bdc39205) SHA1(67b3f56655ef2cc056912ab6e351cf83352abaa9))
	ROM_LOAD("potoc5.3-2", 0x8000, 0x8000, CRC(e6026455) SHA1(c1441fda6181e9014a8a6f93b7405998a952f508))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("potof7.rom", 0x8000, 0x8000, CRC(2e60b2e3) SHA1(0be89fc9b2c6548392febb35c1ace0eb912fc73f))
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("potof6.rom", 0x00000, 0x10000, CRC(62b8f74b) SHA1(f82c706b88f49341bab9014bd83371259eb53b47))
	ROM_LOAD("potof5.rom", 0x10000, 0x10000, CRC(5a0537a8) SHA1(26724441d7e2edd7725337b262d95448499151ad))
ROM_END

ROM_START(poto_a29)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("potob5.2-9", 0x4000, 0x4000, CRC(f01b5510) SHA1(90c632ee74a2dbf877cfe013a69067b1771f1d67))
	ROM_LOAD("potoc5.2-9", 0x8000, 0x8000, CRC(c34975b3) SHA1(c9c57126a5da6d78b4066b1d316ffc840660689d))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("potof7.rom", 0x8000, 0x8000, CRC(2e60b2e3) SHA1(0be89fc9b2c6548392febb35c1ace0eb912fc73f))
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("potof6.rom", 0x00000, 0x10000, CRC(62b8f74b) SHA1(f82c706b88f49341bab9014bd83371259eb53b47))
	ROM_LOAD("potof5.rom", 0x10000, 0x10000, CRC(5a0537a8) SHA1(26724441d7e2edd7725337b262d95448499151ad))
ROM_END

/*-----------------------------------------------------------------------------------
/ Playboy 35th Anniversary - CPU Rev 2 /Alpha Type 2 - 32K Roms - 32/64K Sound Roms
/-----------------------------------------------------------------------------------*/
ROM_START(play_a24)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("play2-4.b5", 0x0000, 0x8000, CRC(bc8d7b32) SHA1(3b57dea2feb12315586283548e0bffdc8173b8fb))
	ROM_LOAD("play2-4.c5", 0x8000, 0x8000, CRC(47c30bc2) SHA1(c62e192ec01f4884226e9628baa2cad10cc57bd9))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("pbsnd7.dat", 0x8000, 0x8000, CRC(c2cf2cc5) SHA1(1277704b1b38558c341b52da5e06ffa9f07942ad))
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("pbsnd6.dat", 0x00000, 0x10000, CRC(c2570631) SHA1(135db5b923689884c73aa5ce48f566db7f1cf831))
	ROM_LOAD("pbsnd5.dat", 0x10000, 0x10000, CRC(0fd30569) SHA1(0bf53fe4b5dffb5e15212c3371f51e98ad14e258))
ROM_END

/*------------------------------------------------------------------
/ Robocop - CPU Rev 3 /Alpha Type 3 - 32K Roms - 32/64K Sound Roms
/-----------------------------------------------------------------*/
ROM_START(robo_a34)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("robob5.a34", 0x0000, 0x8000, CRC(5a611004) SHA1(08722f8f4386bbc467cfbe8854f0d45c4537bdc6))
	ROM_LOAD("roboc5.a34", 0x8000, 0x8000, CRC(c8705f47) SHA1(a29ad9e4e0269ab19dae77b1e70ff84c8c8d9e85))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("robof7.rom", 0x8000, 0x8000, CRC(fa0891bd) SHA1(332d03c7802989abf717564230993b54819ebc0d))
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("robof6.rom", 0x00000, 0x10000, CRC(9246e107) SHA1(e8e72c0d099b17ea9e59ea7794011bad4c072c5e))
	ROM_LOAD("robof4.rom", 0x10000, 0x10000, CRC(27d31df3) SHA1(1611a508ce74eb62a07296d69782ea4fa14503fc))
ROM_END

ROM_START(robo_a30)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("b5.256", 0x0000, 0x8000, CRC(6870f3ae) SHA1(f02cace5f1d1922aed52c84efe60a46e5297865c))
	ROM_LOAD("c5.256", 0x8000, 0x8000, CRC(f2de58cf) SHA1(0b5dd14761b4c64c1b01faad923ab671573499c5))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("robof7.rom", 0x8000, 0x8000, CRC(fa0891bd) SHA1(332d03c7802989abf717564230993b54819ebc0d))
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("robof6.rom", 0x00000, 0x10000, CRC(9246e107) SHA1(e8e72c0d099b17ea9e59ea7794011bad4c072c5e))
	ROM_LOAD("robof4.rom", 0x10000, 0x10000, CRC(27d31df3) SHA1(1611a508ce74eb62a07296d69782ea4fa14503fc))
ROM_END

/*-------------------------------------------------------------------------
/ Secret Service - CPU Rev 2 /Alpha Type 2 - 32K Roms - 32K/64K Sound Roms
/-------------------------------------------------------------------------*/
ROM_START(ssvc_a26)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ssvc2-6.b5", 0x0000, 0x8000, CRC(e5eab8cd) SHA1(63cb678084d4fb2131ba64ed9de1294830057960))
	ROM_LOAD("ssvc2-6.c5", 0x8000, 0x8000, CRC(171b97ae) SHA1(9d678b7b91a5d50ea3cf4f2352094c2355f917b2))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("sssndf7.rom", 0x8000, 0x8000, CRC(980778d0) SHA1(7c1f14d327b6d0e6d0fef058f96bb1cb440c9330))
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("ssv1f6.rom", 0x00000, 0x10000, CRC(ccbc72f8) SHA1(c5c13fb8d05d7fb4005636655073d88b4d12d65e))
	ROM_LOAD("ssv2f4.rom", 0x10000, 0x10000, CRC(53832d16) SHA1(2227eb784e0221f1bf2bdf7ea48ecd122433f1ea))
ROM_END

ROM_START(ssvc_b26)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ssvc2-6.b5", 0x0000, 0x8000, CRC(e5eab8cd) SHA1(63cb678084d4fb2131ba64ed9de1294830057960))
	ROM_LOAD("ssvc2-6.c5", 0x8000, 0x8000, CRC(171b97ae) SHA1(9d678b7b91a5d50ea3cf4f2352094c2355f917b2))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("sssndf7b.rom", 0x8000, 0x8000, CRC(4bd6b16a) SHA1(b9438a16cd35820628fe6eb82287b2c39fe4b1c6))
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("ssv1f6.rom", 0x00000, 0x10000, CRC(ccbc72f8) SHA1(c5c13fb8d05d7fb4005636655073d88b4d12d65e))
	ROM_LOAD("ssv2f4.rom", 0x10000, 0x10000, CRC(53832d16) SHA1(2227eb784e0221f1bf2bdf7ea48ecd122433f1ea))
ROM_END

ROM_START(ssvc_a42)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ss-b5.256", 0x0000, 0x8000, CRC(e7d27ea1) SHA1(997412f62c95cffc0cf9eba065fbc020574c7ad5))
	ROM_LOAD("ss-c5.256", 0x8000, 0x8000, CRC(eceab834) SHA1(d946adac7ec8688709fd75108674a82f2f5c7b53))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("sssndf7b.rom", 0x8000, 0x8000, CRC(4bd6b16a) SHA1(b9438a16cd35820628fe6eb82287b2c39fe4b1c6))
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("ssv1f6.rom", 0x00000, 0x10000, CRC(ccbc72f8) SHA1(c5c13fb8d05d7fb4005636655073d88b4d12d65e))
	ROM_LOAD("ssv2f4.rom", 0x10000, 0x10000, CRC(53832d16) SHA1(2227eb784e0221f1bf2bdf7ea48ecd122433f1ea))
ROM_END

/*------------------------------------------------------------------------
/ The Simpsons - CPU Rev 3 /Alpha Type 3 16/32K Roms - 32/128K Sound Roms
/------------------------------------------------------------------------*/
ROM_START(simp_a27)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("simpb5.2-7", 0x4000, 0x4000, CRC(701c4a4b) SHA1(2a19e2340d119e8813df27a9455aefb599c20a61))
	ROM_LOAD("simpc5.2-7", 0x8000, 0x8000, CRC(400a98b2) SHA1(8d11063712dd718ff8badc29586c700208e7442c))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("simpf7.rom", 0x8000, 0x8000, CRC(a36febbc) SHA1(3b96e05f797dd0dc0d4d52544ed995d477991a9f))
	ROM_REGION(0x1000000, "sound1", 0)
	ROM_LOAD("simpf6.rom", 0x00000, 0x20000, CRC(2eb32ed0) SHA1(e7bc3291cb88bf70010865f64496a3ca393257e7))
	ROM_LOAD("simpf5.rom", 0x20000, 0x20000, CRC(bd0671ae) SHA1(b116a23db956a3dd9fc138ec25af250885ba4ef5))
ROM_END

ROM_START(simp_a20)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("simpa2-0.b5", 0x4000, 0x4000, CRC(e67038d1) SHA1(f3eae2ed45caca97a1eb53d847366c52ea68bbee))
	ROM_LOAD("simpa2-0.c5", 0x8000, 0x8000, CRC(43662bc3) SHA1(d8171a5c083eb8bffa61353b74db6b3ebab96923))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("simpf7.rom", 0x8000, 0x8000, CRC(a36febbc) SHA1(3b96e05f797dd0dc0d4d52544ed995d477991a9f))
	ROM_REGION(0x1000000, "sound1", 0)
	ROM_LOAD("simpf6.rom", 0x00000, 0x20000, CRC(2eb32ed0) SHA1(e7bc3291cb88bf70010865f64496a3ca393257e7))
	ROM_LOAD("simpf5.rom", 0x20000, 0x20000, CRC(bd0671ae) SHA1(b116a23db956a3dd9fc138ec25af250885ba4ef5))
ROM_END

/*--------------------------------------------------------------------------
/ Time Machine - CPU Rev 2 /Alpha Type 2 16/32K Roms - 32/64K Sound Roms
/--------------------------------------------------------------------------*/
ROM_START(tmac_a24)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("tmach2-4.b5", 0x4000, 0x4000, CRC(6ef3cf07) SHA1(3fabfbb2166273bf5bfab06d92fff094d3331d1a))
	ROM_LOAD("tmach2-4.c5", 0x8000, 0x8000, CRC(b61035f5) SHA1(08436b68f37323f50c1fec86aba303a1690af653))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("tmachf7.rom", 0x8000, 0x8000, CRC(0f518bd4) SHA1(05e24ca0e76d576c65d9d2a01417f1ad2aa984bb))
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("tmachf6.rom", 0x00000, 0x10000, CRC(47e61641) SHA1(93cd946ebc9f69d82512429a9ae5f2754499b00a))
	ROM_LOAD("tmachf4.rom", 0x10000, 0x10000, CRC(51e3aade) SHA1(38fc0f3a9c727bfd07fbcb16c3ca6d0560dc65c3))
ROM_END

ROM_START(tmac_a18)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("tmach1-8.b5", 0x4000, 0x4000, CRC(5dabdc4c) SHA1(67fe261888ddaa088abe2f8a331eaa5ac34be92e))
	ROM_LOAD("tmach1-8.c5", 0x8000, 0x8000, CRC(5a348def) SHA1(bf2b9a69d516d38e6f87c5886e0ba768c2dc28ab))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("tmachf7.rom", 0x8000, 0x8000, CRC(0f518bd4) SHA1(05e24ca0e76d576c65d9d2a01417f1ad2aa984bb))
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("tmachf6.rom", 0x00000, 0x10000, CRC(47e61641) SHA1(93cd946ebc9f69d82512429a9ae5f2754499b00a))
	ROM_LOAD("tmachf4.rom", 0x10000, 0x10000, CRC(51e3aade) SHA1(38fc0f3a9c727bfd07fbcb16c3ca6d0560dc65c3))
ROM_END

ROM_START(tmac_g18)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("tmachg18.b5", 0x4000, 0x4000, CRC(513d70ad) SHA1(dacdfc77956b1b5fb9bebca59fdb705aefa1b5b2))
	ROM_LOAD("tmachg18.c5", 0x8000, 0x8000, CRC(5a348def) SHA1(bf2b9a69d516d38e6f87c5886e0ba768c2dc28ab))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("tmachf7.rom", 0x8000, 0x8000, CRC(0f518bd4) SHA1(05e24ca0e76d576c65d9d2a01417f1ad2aa984bb))
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("tmachf6.rom", 0x00000, 0x10000, CRC(47e61641) SHA1(93cd946ebc9f69d82512429a9ae5f2754499b00a))
	ROM_LOAD("tmachf4.rom", 0x10000, 0x10000, CRC(51e3aade) SHA1(38fc0f3a9c727bfd07fbcb16c3ca6d0560dc65c3))
ROM_END

/*-----------------------------------------------------------------------
/ Torpedo Alley - CPU Rev 2 /Alpha Type 2 - 32K Roms - 32/64K Sound Roms
/------------------------------------------------------------------------*/
ROM_START(torp_e21)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("torpe2-1.b5", 0x0000, 0x8000, CRC(ac0b03e3) SHA1(0ac57b2fec29cdc90ab35cba49844f0cf545d959))
	ROM_LOAD("torpe2-1.c5", 0x8000, 0x8000, CRC(9ad33882) SHA1(c4504d8e136f667652f79b54d4e8d775169c6ac3))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("torpef7.rom", 0x8000, 0x8000, CRC(26f4c33e) SHA1(114f85e93e7b699c4cd6ce1298f95228d439deba))
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("torpef6.rom", 0x00000, 0x10000, CRC(b214a7ea) SHA1(d972148395581844e3eaed08f755f3e2217dbbc0))
	ROM_LOAD("torpef4.rom", 0x10000, 0x10000, CRC(83a4e7f3) SHA1(96deac9251fe68cc0319ac009becd424c4e444c5))
ROM_END

ROM_START(torp_a16)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("b5.256", 0x0000, 0x8000, CRC(89711a7c) SHA1(b976b32b287d6cbaf4c448697f8aa12452db1f0b))
	ROM_LOAD("c5.256", 0x8000, 0x8000, CRC(3b3d754f) SHA1(c5d4a09f4daf92af78d778148377fa0d2a550761))
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("torpef7.rom", 0x8000, 0x8000, CRC(26f4c33e) SHA1(114f85e93e7b699c4cd6ce1298f95228d439deba))
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("torpef6.rom", 0x00000, 0x10000, CRC(b214a7ea) SHA1(d972148395581844e3eaed08f755f3e2217dbbc0))
	ROM_LOAD("torpef4.rom", 0x10000, 0x10000, CRC(83a4e7f3) SHA1(96deac9251fe68cc0319ac009becd424c4e444c5))
ROM_END


GAME( 1990, bttf_a28, 0,        de_type3,        de_2, de_2_state, empty_init, ROT0, "Data East", "Back to the Future - The Pinball (2.8)",          MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1990, bttf_a27, bttf_a28, de_type3,        de_2, de_2_state, empty_init, ROT0, "Data East", "Back to the Future - The Pinball (2.7)",          MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1990, bttf_a20, bttf_a28, de_type3,        de_2, de_2_state, empty_init, ROT0, "Data East", "Back to the Future - The Pinball (2.0)",          MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1990, bttf_a21, bttf_a28, de_type3,        de_2, de_2_state, empty_init, ROT0, "Data East", "Back to the Future - The Pinball (2.1)",          MACHINE_IS_SKELETON_MECHANICAL)
GAME( 199?, bttf_g27, bttf_a28, de_type3,        de_2, de_2_state, empty_init, ROT0, "Data East", "Back to the Future - The Pinball (2.7, Germany)", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1990, kiko_a10, 0,        de_type3,        de_2, de_2_state, empty_init, ROT0, "Data East", "King Kong (1.0)",                      MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1987, lwar_a83, 0,        de_type1,        de_2, de_2_state, empty_init, ROT0, "Data East", "Laser War (8.3)",                      MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1987, lwar_a81, lwar_a83, de_type1,        de_2, de_2_state, empty_init, ROT0, "Data East", "Laser War (8.1)",                      MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1987, lwar_e90, lwar_a83, de_type1,        de_2, de_2_state, empty_init, ROT0, "Data East", "Laser War (9.0 Europe)",               MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1989, mnfb_c29, 0,        de_type2_alpha3, de_2, de_2_state, empty_init, ROT0, "Data East", "Monday Night Football (2.9, 50cts)",   MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1989, mnfb_c27, mnfb_c29, de_type2_alpha3, de_2, de_2_state, empty_init, ROT0, "Data East", "Monday Night Football (2.7, 50cts)",   MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1990, poto_a32, 0,        de_type2_alpha3, de_2, de_2_state, empty_init, ROT0, "Data East", "The Phantom of the Opera (3.2)",       MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1990, poto_a29, poto_a32, de_type2_alpha3, de_2, de_2_state, empty_init, ROT0, "Data East", "The Phantom of the Opera (2.9)",       MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1989, play_a24, 0,        de_type2,        de_2, de_2_state, empty_init, ROT0, "Data East", "Playboy 35th Anniversary (2.4)",       MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1989, robo_a34, 0,        de_type3,        de_2, de_2_state, empty_init, ROT0, "Data East", "Robocop (3.4)",                        MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1989, robo_a30, robo_a34, de_type3,        de_2, de_2_state, empty_init, ROT0, "Data East", "Robocop (3.0)",                        MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1988, ssvc_a26, 0,        de_type2,        de_2, de_2_state, empty_init, ROT0, "Data East", "Secret Service (2.6)",                 MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1988, ssvc_b26, ssvc_a26, de_type2,        de_2, de_2_state, empty_init, ROT0, "Data East", "Secret Service (2.6 alternate sound)", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1988, ssvc_a42, ssvc_a26, de_type2,        de_2, de_2_state, empty_init, ROT0, "Data East", "Secret Service (4.2 alternate sound)", MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1990, simp_a27, 0,        de_type3,        de_2, de_2_state, empty_init, ROT0, "Data East", "The Simpsons (2.7)",                   MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1990, simp_a20, simp_a27, de_type3,        de_2, de_2_state, empty_init, ROT0, "Data East", "The Simpsons (2.0)",                   MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1988, tmac_a24, 0,        de_type2,        de_2, de_2_state, empty_init, ROT0, "Data East", "Time Machine (2.4)",                   MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1988, tmac_a18, tmac_a24, de_type2,        de_2, de_2_state, empty_init, ROT0, "Data East", "Time Machine (1.8)",                   MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1988, tmac_g18, tmac_a24, de_type2,        de_2, de_2_state, empty_init, ROT0, "Data East", "Time Machine (1.8, Germany)",          MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1988, torp_e21, 0,        de_type2,        de_2, de_2_state, empty_init, ROT0, "Data East", "Torpedo Alley (2.1, Europe)",          MACHINE_IS_SKELETON_MECHANICAL)
GAME( 1988, torp_a16, torp_e21, de_type2,        de_2, de_2_state, empty_init, ROT0, "Data East", "Torpedo Alley (1.6)",                  MACHINE_IS_SKELETON_MECHANICAL)
