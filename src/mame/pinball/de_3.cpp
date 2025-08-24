// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/********************************************************************************************************************
PINBALL
Data East Systems 3 and 3B

Data East CPU board is similar to Williams System 11, but without the generic audio board.
For now, we'll presume the timings are the same.

Here are the key codes to enable play:
- Some games need you to hit H after the ball enters the shooter lane.
- If the game wants you to choose something at start of ball 1, hit a Shift key.

Game                                   NUM    Start game                          End ball
---------------------------------------------------------------------------------------------
Back to the Future                    5009    Hold CDE, hit 1                     CDE
Checkpoint                            5010    Hold CDE, hit 1                     CDE
Batman                                5011    Hold CDE, hit 1                     CDE
The Simpsons                          5012    Hold CDE, hit 1                     CDE
World's Fair (not emulated)           5013
Star Trek                             5014    Hold CDE, hit 1                     CDE
Total Recall (not emulated)           5015
Kill Shot (not emulated)              5016
Teenage Mutant Ninja Turtles          5017    Hold CDE, hit 1                     CDE
Tales from the Crypt                  5018    Hold ABCDE, hit 1                   Hold ABCD, hit F
Hook                                  5019    Hold CDE, hit 1                     CDE
Jurassic Park                         5020    Hold ABCDEF, hit 1                  ABCDEF
Operation Desert Storm (not emulated) 5021
Adv. of Rocky & Bullwinkle & Friends  5022    Hold CDE, hit 1                     CDE
WWF Royal Rumble                      5023    Hold ABC, hit 1                     Hold ABC, hit F
Star Wars                             5024    Hold CDE, hit 1                     CDE
Mad (not emulated)                    5025
Lethal Weapon 3                       5026    Hold CDE, hit 1                     CDE
Last Action Hero                      5027    Hold ABC, hit 1                     BCF
The Who's Tommy Pinball Wizard        5028    Hold ABC, hit 1                     Hold ABC, hit F
Guns n' Roses                         5029    Hold A, hit 1                       ABCDEF
Aaron Spelling                         --     Hold CDE, hit 1                     CDE
Michael Jordan                         --
Maverick the Movie                    5031    Hold CDE, hit 1                     Hold CDE, hit F
Baywatch                              5033    Hold BCD, hit 1                     Hold BCD, hit F
Mary Shelley's Frankenstein           5036    Hold A, hit 1 then H then Shift     Hold A, hit F
Batman Forever                        5038    Hold CDEF, hit 1                    CDEF
Derby Daze (not emulated)             5039
Cut the Cheese (redemption)           5048    6                                   INP17-40 (40=toilet)
Roach Racers (not emulated)           5054


Status:
- All pinball machines are playable

ToDo:
- Cut the Cheese: screen goes blank after a short while
- Test fixture: nothing to see

*********************************************************************************************************************/
#include "emu.h"

#include "decobsmt.h"
#include "decodmd1.h"
#include "decodmd2.h"
#include "decodmd3.h"
#include "decopincpu.h"
#include "genpin.h"

#include "cpu/m6809/m6809.h"
#include "sound/msm5205.h"
#include "sound/ymopm.h"

#include "speaker.h"


namespace {

class de_3_state : public genpin_class
{
public:
	de_3_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_ym2151(*this, "ym2151")
		, m_audiocpu(*this, "audiocpu")
		, m_msm5205(*this, "msm5205")
		, m_sample_bank(*this, "sample_bank")
		, m_dmdtype1(*this, "decodmd1")
		, m_dmdtype2(*this, "decodmd2")
		, m_dmdtype3(*this, "decodmd3")
		, m_decobsmt(*this, "decobsmt")
		, m_sound1(*this, "sound1")
		, m_io_keyboard(*this, "X%d", 0U)
	{ }

	void de_3_dmdo(machine_config &config);
	void de_3_dmd1(machine_config &config);
	void de_3_dmd2(machine_config &config);
	void detest(machine_config &config);
	void de_3b(machine_config &config);

protected:
	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void de_bg_audio(machine_config &config);
	void audio_map(address_map &map) ATTR_COLD;
	void ym2151_irq_w(int state);
	void msm5205_irq_w(int state);
	uint8_t sound_latch_r();
	void sample_bank_w(uint8_t data);
	void sample_w(uint8_t data);
	uint8_t m_sample_data = 0U;
	uint8_t m_sound_data = 0U;
	bool m_more_data = false;
	bool m_nmi_enable = false;

	uint8_t switch_r();
	void switch_w(uint8_t data);
	void pia2c_pa_w(uint8_t data);
	uint8_t pia2c_pb_r();
	void pia2c_pb_w(uint8_t data);
	void lamp0_w(uint8_t data) { }
	void lamp1_w(uint8_t data) { }
	void sound_w(uint8_t data);
	uint8_t dmd_status_r();

	// devcb callbacks
	uint8_t display_r(offs_t offset);
	void display_w(offs_t offset, uint8_t data);
	void lamps_w(offs_t offset, uint8_t data);
	void sol_w(offs_t, uint8_t);

	void de_3(machine_config &config);

	// devices
	optional_device<ym2151_device> m_ym2151;
	optional_device<cpu_device> m_audiocpu;
	optional_device<msm5205_device> m_msm5205;
	optional_memory_bank m_sample_bank;
	optional_device<decodmd_type1_device> m_dmdtype1;
	optional_device<decodmd_type2_device> m_dmdtype2;
	optional_device<decodmd_type3_device> m_dmdtype3;
	optional_device<decobsmt_device> m_decobsmt;
	optional_memory_region m_sound1;
	required_ioport_array<8> m_io_keyboard;

	uint8_t m_row = 0U;
	u16 m_sol = 0U;
};

void de_3_state::audio_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x2000, 0x2001).rw(m_ym2151, FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x2400, 0x2400).r(FUNC(de_3_state::sound_latch_r));
	map(0x2800, 0x2800).w(FUNC(de_3_state::sample_bank_w));
	// 0x2c00        - 4052(?)
	map(0x3000, 0x3000).w(FUNC(de_3_state::sample_w));
	// 0x3800        - Watchdog reset
	map(0x4000, 0x7fff).bankr("sample_bank");
	map(0x8000, 0xffff).rom();
}

static INPUT_PORTS_START( de3 )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN4 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_NAME("Slam Tilt")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START2 )  // Buy extra ball on some machines

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("INP09")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("INP10")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("INP11")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("INP12")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("INP13")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("INP14")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("INP15")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("INP16")

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("INP17")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("INP18")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("INP19")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("INP20")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("INP21")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("INP22")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_O) PORT_NAME("INP23")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_P) PORT_NAME("INP24")

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("INP25")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("INP26")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("INP27")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("INP28")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("INP29")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("INP30")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("INP31")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("INP32")

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("INP33")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("INP34")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("INP35")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_NAME("INP36")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("INP37")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_COLON) PORT_NAME("INP38")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("INP39")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("INP40")

	PORT_START("X5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("INP41")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("INP42")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("INP43")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("INP44")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("INP45")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("INP46")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_LEFT) PORT_NAME("INP47")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_UP) PORT_NAME("INP48")

	PORT_START("X6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("INP49")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DOWN) PORT_NAME("INP50")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_HOME) PORT_NAME("INP51")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_END) PORT_NAME("INP52")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DEL) PORT_NAME("INP53")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PGDN) PORT_NAME("INP54")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PGUP) PORT_NAME("INP55")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SPACE) PORT_NAME("INP56")

	PORT_START("X7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("INP57")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("INP58")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("INP59")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("INP60")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("INP61")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME("INP62")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("INP63")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("INP64")
INPUT_PORTS_END

// 6821 PIA at 0x2000
void de_3_state::sound_w(uint8_t data)
{
	if (m_decobsmt)
	{
		if(data != 0xfe)
			m_decobsmt->bsmt_comms_w(data);
	}
	if (m_sound1)
	{
		m_sound_data = data;
		m_audiocpu->set_input_line(M6809_FIRQ_LINE, ASSERT_LINE);
	}
}

// 6821 PIA at 0x2400

// 6821 PIA at 0x2800

// 6821 PIA at 0x2c00
void de_3_state::pia2c_pa_w(uint8_t data)
{
	// DMD data
	if(m_dmdtype3)
	{
		m_dmdtype3->data_w(data);
		//logerror("DMD: Data write %02x\n", data);
	}
	else
	if(m_dmdtype2)
	{
		m_dmdtype2->data_w(data);
		//logerror("DMD: Data write %02x\n", data);
	}
	else
	if(m_dmdtype1)
	{
		m_dmdtype1->data_w(data);
		//logerror("DMD: Data write %02x\n", data);
	}
}

uint8_t de_3_state::pia2c_pb_r()
{
	if(m_dmdtype1)
		return m_dmdtype1->busy_r();
	else
	if(m_dmdtype2)
		return m_dmdtype2->busy_r();
	else
	if(m_dmdtype3)
		return m_dmdtype3->busy_r();
	return 0;
}

void de_3_state::pia2c_pb_w(uint8_t data)
{
	// DMD ctrl
	if(m_dmdtype3)
	{
		m_dmdtype3->ctrl_w(data);
		//logerror("DMD: Control write %02x\n", data);
	}
	else
	if(m_dmdtype2)
	{
		m_dmdtype2->ctrl_w(data);
		//logerror("DMD: Control write %02x\n", data);
	}
	else
	if(m_dmdtype1)
	{
		m_dmdtype1->ctrl_w(data);
		//logerror("DMD: Control write %02x\n", data);
	}
}


// 6821 PIA at 0x3000
uint8_t de_3_state::switch_r()
{
	u8 data = 0;
	if (m_row < 0x81)
		for (u8 i = 0; i < 8; i++)
			if (BIT(m_row, i))
				data |= m_io_keyboard[i]->read();

	return data;
}

void de_3_state::switch_w(uint8_t data)
{
	m_row = data;
}

void de_3_state::sol_w(offs_t offset, u8 data)
{
	if (!offset)
		m_sol = (m_sol & 0xff00) | data;
	else
		m_sol = (m_sol & 0xff) | (BIT(data, 1) ? 0x100 : 0);

	// these vary per game, this is an example
	switch (m_sol)
	{
		case 0x0002:
			m_samples->start(5, 5); // outhole
			break;
		case 0x0080:
			m_samples->start(0, 6); // knocker
			break;
	}
}

// 6821 PIA at 0x3400
uint8_t de_3_state::dmd_status_r()
{
	if(m_dmdtype1)
		return m_dmdtype1->status_r();
	else
	if(m_dmdtype2)
		return m_dmdtype2->status_r();
	else
	if(m_dmdtype3)
		return m_dmdtype3->status_r();

	return 0;
}

uint8_t de_3_state::display_r(offs_t offset)
{
	uint8_t ret = 0x00;

	switch(offset)
	{
	case 0:
		break;
	case 3:
		ret = pia2c_pb_r();
		break;
	}

	return ret;
}

void de_3_state::display_w(offs_t offset, uint8_t data)
{
	switch(offset)
	{
	case 0:
		break;
	case 1:
		break;
	case 2:
		pia2c_pa_w(data);
		break;
	case 3:
		pia2c_pb_w(data);
		break;
	case 4:
		break;
	}
}

void de_3_state::lamps_w(offs_t offset, uint8_t data)
{
	switch(offset)
	{
	case 0:
		lamp0_w(data);
		break;
	case 1:
		lamp1_w(data);
		break;
	}
}

void de_3_state::ym2151_irq_w(int state)
{
	m_audiocpu->set_input_line(M6809_IRQ_LINE,state);
}

void de_3_state::msm5205_irq_w(int state)
{
	m_msm5205->data_w(m_sample_data >> 4);
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

// Sound board
void de_3_state::sample_w(uint8_t data)
{
	m_sample_data = data;
}

uint8_t de_3_state::sound_latch_r()
{
	if (!machine().side_effects_disabled())
		m_audiocpu->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE);
	return m_sound_data;
}

void de_3_state::sample_bank_w(uint8_t data)
{
	static constexpr uint8_t prescale[4] = { msm5205_device::S96_4B, msm5205_device::S48_4B, msm5205_device::S64_4B, 0 };

	m_sample_bank->set_entry(data & 15);
	m_nmi_enable = !BIT(data, 7);
	m_msm5205->playmode_w(prescale[BIT(data, 4, 2)]);
	m_msm5205->reset_w(BIT(data, 6));
}

void de_3_state::machine_start()
{
	genpin_class::machine_start();

	save_item(NAME(m_row));
	save_item(NAME(m_sol));
	save_item(NAME(m_nmi_enable));
	save_item(NAME(m_sample_data));
	save_item(NAME(m_more_data));

	if (m_sound1)
	{
		uint8_t *const ROM = m_sound1->base();
		m_sample_bank->configure_entries(0, 16, &ROM[0x0000], 0x4000);
		m_sample_bank->set_entry(0);
	}
}

void de_3_state::machine_reset()
{
	genpin_class::machine_reset();

	if (m_sound1)
		m_sample_bank->set_entry(0);
}

void de_3_state::de_bg_audio(machine_config &config)
{
	// sound CPU
	MC6809E(config, m_audiocpu, XTAL(8'000'000) / 4); // MC68B09E
	m_audiocpu->set_addrmap(AS_PROGRAM, &de_3_state::audio_map);

	SPEAKER(config, "bg").front_center();

	YM2151(config, m_ym2151, XTAL(3'579'545));
	m_ym2151->irq_handler().set(FUNC(de_3_state::ym2151_irq_w));
	m_ym2151->add_route(ALL_OUTPUTS, "bg", 0.40);

	MSM5205(config, m_msm5205, XTAL(384'000));
	m_msm5205->vck_legacy_callback().set(FUNC(de_3_state::msm5205_irq_w));
	m_msm5205->set_prescaler_selector(msm5205_device::S96_4B);
	m_msm5205->add_route(ALL_OUTPUTS, "bg", 0.90);
}

void de_3_state::de_3(machine_config &config)
{
	// basic machine hardware
	decocpu_type3_device &decocpu(DECOCPU3(config, "decocpu", XTAL(8'000'000) / 2, "maincpu"));
	decocpu.display_read_callback().set(FUNC(de_3_state::display_r));
	decocpu.display_write_callback().set(FUNC(de_3_state::display_w));
	decocpu.soundlatch_write_callback().set(FUNC(de_3_state::sound_w));
	decocpu.switch_read_callback().set(FUNC(de_3_state::switch_r));
	decocpu.switch_write_callback().set(FUNC(de_3_state::switch_w));
	decocpu.lamp_write_callback().set(FUNC(de_3_state::lamps_w));
	decocpu.solenoid_write_callback().set(FUNC(de_3_state::sol_w));
	decocpu.dmdstatus_read_callback().set(FUNC(de_3_state::dmd_status_r));

	genpin_audio(config);
}

void de_3_state::de_3_dmd2(machine_config &config)
{
	de_3(config);
	DECODMD2(config, m_dmdtype2, 0);

	SPEAKER(config, "speaker", 2).front();

	DECOBSMT(config, m_decobsmt, 0);
	m_decobsmt->add_route(0, "speaker", 1.0, 0);
	m_decobsmt->add_route(1, "speaker", 1.0, 1);
}

void de_3_state::de_3_dmd1(machine_config &config)
{
	de_3(config);
	DECODMD1(config, m_dmdtype1, 0);

	SPEAKER(config, "speaker", 2).front();

	DECOBSMT(config, m_decobsmt, 0);
	m_decobsmt->add_route(0, "speaker", 1.0, 0);
	m_decobsmt->add_route(1, "speaker", 1.0, 1);
}

void de_3_state::de_3_dmdo(machine_config &config)
{
	de_3(config);
	DECODMD1(config, m_dmdtype1, 0);
	de_bg_audio(config);
}

void de_3_state::de_3b(machine_config &config)
{
	// basic machine hardware
	decocpu_type3b_device &decocpu(DECOCPU3B(config, "decocpu", XTAL(8'000'000) / 2, "maincpu"));
	decocpu.display_read_callback().set(FUNC(de_3_state::display_r));
	decocpu.display_write_callback().set(FUNC(de_3_state::display_w));
	decocpu.soundlatch_write_callback().set(FUNC(de_3_state::sound_w));
	decocpu.switch_read_callback().set(FUNC(de_3_state::switch_r));
	decocpu.switch_write_callback().set(FUNC(de_3_state::switch_w));
	decocpu.lamp_write_callback().set(FUNC(de_3_state::lamps_w));
	decocpu.solenoid_write_callback().set(FUNC(de_3_state::sol_w));
	decocpu.dmdstatus_read_callback().set(FUNC(de_3_state::dmd_status_r));

	genpin_audio(config);

	DECODMD3(config, m_dmdtype3, 0);

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	DECOBSMT(config, m_decobsmt, 0);
	m_decobsmt->add_route(0, "speaker", 1.0, 0);
	m_decobsmt->add_route(1, "speaker", 1.0, 1);
}

void de_3_state::detest(machine_config &config)
{
	// basic machine hardware
	DECOCPU3B(config, "decocpu", XTAL(8'000'000) / 2, "maincpu");

	genpin_audio(config);
}


/*-------------------------------------------------------------
/ Adventures of Rocky and Bullwinkle and Friends - CPU Rev 3b /DMD  Type 2 512K Rom - 64K CPU Rom
/------------------------------------------------------------*/
ROM_START(rab_320)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("rabcpua.320", 0x0000, 0x10000, CRC(21a2d518) SHA1(42123dca519034ecb740e5cb493b1b0b6b44e3be))
	ROM_REGION(0x80000, "decodmd2", 0)
	ROM_LOAD("rbdspa.300", 0x00000, 0x80000, CRC(a5dc2f72) SHA1(60bbb4914ff56ad48c86c3550e094a3d9d70c700))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("rab.u7", 0x0000, 0x10000, CRC(b232e630) SHA1(880fffc395d7c24bdea4e7e8000afba7ea71c094))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("rab.u17", 0x000000, 0x80000, CRC(7f2b53b8) SHA1(fd4f4ed1ed343069ffc534fe4b20026fe7403220))
	ROM_LOAD("rab.u21", 0x080000, 0x40000, CRC(3de1b375) SHA1(a48bb80483ca03cd7c3bf0b5f2930a6ee9cc448d))
ROM_END

ROM_START(rab_130)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("rabcpua.130", 0x0000, 0x10000, CRC(f59b1a53) SHA1(046cd0eaee6e646286f3dfa73eeacfd93c2be273))
	ROM_REGION(0x80000, "decodmd2", 0)
	ROM_LOAD("rbdspa.130", 0x00000, 0x80000, CRC(b6e2176e) SHA1(9ccbb30dc0f386fcf5e5255c9f80c720e601565f))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("rab.u7", 0x0000, 0x10000, CRC(b232e630) SHA1(880fffc395d7c24bdea4e7e8000afba7ea71c094))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("rab.u17", 0x000000, 0x80000, CRC(7f2b53b8) SHA1(fd4f4ed1ed343069ffc534fe4b20026fe7403220))
	ROM_LOAD("rab.u21", 0x080000, 0x40000, CRC(3de1b375) SHA1(a48bb80483ca03cd7c3bf0b5f2930a6ee9cc448d))
ROM_END

ROM_START(rab_103s)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("rabcpu.103", 0x0000, 0x10000, CRC(d5fe3184) SHA1(dc1ca938f15240d1c15ee5724d29a3538418f8de))
	ROM_REGION(0x80000, "decodmd2", 0)
	ROM_LOAD("rabdspsp.103", 0x00000, 0x80000, CRC(02624948) SHA1(069ef69d6ce193d73954935b378230c05b83b8fc))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("rab.u7", 0x0000, 0x10000, CRC(b232e630) SHA1(880fffc395d7c24bdea4e7e8000afba7ea71c094))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("rab.u17", 0x000000, 0x80000, CRC(7f2b53b8) SHA1(fd4f4ed1ed343069ffc534fe4b20026fe7403220))
	ROM_LOAD("rab.u21", 0x080000, 0x40000, CRC(3de1b375) SHA1(a48bb80483ca03cd7c3bf0b5f2930a6ee9cc448d))
ROM_END

/*-------------------------------------------------------------
/ Aaron Spelling - CPU Rev 3 /DMD  Type 2 512K Rom - 64K CPU Rom
/------------------------------------------------------------*/
ROM_START(aar_101)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("as512cpu.bin", 0x0000, 0x10000, CRC(03c70e67) SHA1(3093e217943ae80c842a1d893cff5330ac90bc30))
	ROM_REGION(0x80000, "decodmd2", 0)
	ROM_LOAD("asdspu12.bin", 0x00000, 0x40000, CRC(5dd81be6) SHA1(20e5ec19550e3795670c5ee4e8e92fae0499fdb8))
	ROM_LOAD("asdspu14.bin", 0x40000, 0x40000, CRC(3f2204ca) SHA1(69523d6c5555d391ab24912f4c4c78aa09a400c1))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("assndu7.bin", 0x0000, 0x10000, CRC(f0414a0d) SHA1(b1f940be05426a39f4e5ea0802fd03a7ce055ebc))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("assndu17.bin", 0x000000, 0x80000, CRC(e151b1fe) SHA1(d7d97499d93885a4f7ebd7bb302731bc5bc456ff))
	ROM_LOAD("assndu21.bin", 0x080000, 0x80000, CRC(7d69e917) SHA1(73e21e65bc194c063933288cb617127b41593466))
ROM_END

/*-------------------------------------------------------------
/ Batman - CPU Rev 3 /DMD Type 1 128K Rom 16/32K CPU Roms
/------------------------------------------------------------*/
ROM_START(btmn_106)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("b5_a106.128", 0x4000, 0x4000, CRC(5aa7fbe3) SHA1(587be4fd18ad730e675e720923e00d1775a4560e))
	ROM_LOAD("c5_a106.256", 0x8000, 0x8000, CRC(79e86ccd) SHA1(430ac436bd1c8841950986af80747285a7d25942))
	ROM_REGION(0x20000, "decodmd1", 0)
	ROM_LOAD("batdsp.106", 0x00000, 0x20000, CRC(4c4120e7) SHA1(ba7d78c933f6709b3db4efcca5e7bb9099074550))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("batman.u7", 0x8000, 0x8000, CRC(b2e88bf5) SHA1(28f814ea73f8eefd1bb5499a599e67a6850c92c0))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("batman.u17", 0x000000, 0x40000, CRC(b84914dd) SHA1(333d88033428705cbd0a40d70d938c0021bb0015))
	ROM_LOAD("batman.u21", 0x080000, 0x20000, CRC(42dab6ac) SHA1(facf993db2ce240c9e825ca9a21ac65a0fbba188))
ROM_END

ROM_START(btmn_103)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("batcpub5.103", 0x4000, 0x4000, CRC(6f160581) SHA1(0f2d6c396324fbf116309a872cf95d9a05446cea))
	ROM_LOAD("batcpuc5.103", 0x8000, 0x8000, CRC(8588c5a8) SHA1(41b159c9e4ca523b37f0b893e57f166c85e812e9))
	ROM_REGION(0x20000, "decodmd1", 0)
	ROM_LOAD("batdsp.102", 0x00000, 0x20000, CRC(4c4120e7) SHA1(ba7d78c933f6709b3db4efcca5e7bb9099074550))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("batman.u7", 0x8000, 0x8000, CRC(b2e88bf5) SHA1(28f814ea73f8eefd1bb5499a599e67a6850c92c0))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("batman.u17", 0x000000, 0x40000, CRC(b84914dd) SHA1(333d88033428705cbd0a40d70d938c0021bb0015))
	ROM_LOAD("batman.u21", 0x080000, 0x20000, CRC(42dab6ac) SHA1(facf993db2ce240c9e825ca9a21ac65a0fbba188))
ROM_END

ROM_START(btmn_103f)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("batcpub5.103", 0x4000, 0x4000, CRC(6f160581) SHA1(0f2d6c396324fbf116309a872cf95d9a05446cea))
	ROM_LOAD("batccpuf.103", 0x8000, 0x8000, CRC(6f654fb4) SHA1(4901326f92aab1f5a2cdf9032511bef8b197f7e4))
	ROM_REGION(0x20000, "decodmd1", 0)
	ROM_LOAD("bat_dspf.103", 0x00000, 0x20000, CRC(747be2e6) SHA1(47ac64b91eabc24be57e376035ef8da95259587d))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("batman.u7", 0x8000, 0x8000, CRC(b2e88bf5) SHA1(28f814ea73f8eefd1bb5499a599e67a6850c92c0))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("batman.u17", 0x000000, 0x40000, CRC(b84914dd) SHA1(333d88033428705cbd0a40d70d938c0021bb0015))
	ROM_LOAD("batman.u21", 0x080000, 0x20000, CRC(42dab6ac) SHA1(facf993db2ce240c9e825ca9a21ac65a0fbba188))
ROM_END

ROM_START(btmn_103g)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("batbcpug.103", 0x4000, 0x4000, CRC(6f160581) SHA1(0f2d6c396324fbf116309a872cf95d9a05446cea))
	ROM_LOAD("batccpug.103", 0x8000, 0x8000, CRC(a199ab0f) SHA1(729dab10fee708a18b7be5a2b9b904aa211b233a))
	ROM_REGION(0x20000, "decodmd1", 0)
	ROM_LOAD("bat_dspg.104", 0x00000, 0x20000, CRC(1581819f) SHA1(88facfad2e74dd44b71fd19df685a4c2378d26de))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("batman.u7", 0x8000, 0x8000, CRC(b2e88bf5) SHA1(28f814ea73f8eefd1bb5499a599e67a6850c92c0))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("batman.u17", 0x000000, 0x40000, CRC(b84914dd) SHA1(333d88033428705cbd0a40d70d938c0021bb0015))
	ROM_LOAD("batman.u21", 0x080000, 0x20000, CRC(42dab6ac) SHA1(facf993db2ce240c9e825ca9a21ac65a0fbba188))
ROM_END

ROM_START(btmn_101)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("batcpub5.101", 0x0000, 0x8000, CRC(a7f5754e) SHA1(2c24cab4cc5f1e05539d2843a49b4b1a8d507630))
	ROM_LOAD("batcpuc5.101", 0x8000, 0x8000, CRC(1fcb85ca) SHA1(daf1e1297975b9b577c796d50b973885f925508e))
	ROM_REGION(0x20000, "decodmd1", 0)
	ROM_LOAD("batdsp.102", 0x00000, 0x20000, CRC(4c4120e7) SHA1(ba7d78c933f6709b3db4efcca5e7bb9099074550))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("batman.u7", 0x8000, 0x8000, CRC(b2e88bf5) SHA1(28f814ea73f8eefd1bb5499a599e67a6850c92c0))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("batman.u17", 0x000000, 0x40000, CRC(b84914dd) SHA1(333d88033428705cbd0a40d70d938c0021bb0015))
	ROM_LOAD("batman.u21", 0x080000, 0x20000, CRC(42dab6ac) SHA1(facf993db2ce240c9e825ca9a21ac65a0fbba188))
ROM_END

/*------------------------------------------------------------
/ Checkpoint - CPU Rev 3 /DMD Type 1 64K Rom 16/32K CPU Roms
/------------------------------------------------------------*/
ROM_START(ckpt_a17)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("chkpntb5.107", 0x4000, 0x4000, CRC(9fbae8e3) SHA1(a25b9dcba2a3f84394972bf36930c0f0344eccbd))
	ROM_LOAD("chkpntc5.107", 0x8000, 0x8000, CRC(082dc283) SHA1(cc3038e0999d2c403fe1863e649b8029376b0387))
	ROM_REGION(0x20000, "decodmd1", 0)
	ROM_LOAD("chkpntds.512", 0x00000, 0x10000, CRC(14d9c6d6) SHA1(5470a4ebe7bc4a056f75aa1fffe3a4e3e24457c6))
	ROM_RELOAD(0x10000, 0x10000)
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("chkpntf7.rom", 0x8000, 0x8000, CRC(e6f6d716) SHA1(a034eb94acb174f7dbe192a55cfd00715ca85a75))
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("chkpntf6.rom", 0x00000, 0x20000, CRC(2d08043e) SHA1(476c9945354e733bfc9a854760ca8cfa3bc62294))
	ROM_LOAD("chkpntf5.rom", 0x20000, 0x20000, CRC(167daa2c) SHA1(458781726c73a09da2b8e8313e1d359cb795a744))
ROM_END

/*-------------------------------------------------------------
/ Guns N Roses - CPU Rev 3b /DMD  Type 2 512K Rom - 64K CPU Rom
/------------------------------------------------------------*/
ROM_START(gnr_300)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gnrcpua.300", 0x0000, 0x10000, CRC(faf0cc8c) SHA1(0e889ad6eed832d4ccdc6e379f9e4e58ae0e0b83))
	ROM_REGION(0x80000, "decodmd2", 0)
	ROM_LOAD("gnrdispa.300", 0x00000, 0x80000, CRC(4abf29e3) SHA1(595328e0f92a6e1972d71c56505a5dd07a373ef5))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("gnru7.snd", 0x0000, 0x10000, CRC(3b9de915) SHA1(a901a1f37bf5433c819393c4355f9d13164b32ce))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("gnru17.snd", 0x000000, 0x80000, CRC(3d3219d6) SHA1(ac4a6d3eff0cdd02b8c79dddcb8fec2e22faa9b9))
	ROM_LOAD("gnru21.snd", 0x080000, 0x80000, CRC(d2ca17ab) SHA1(db7c4f74a2e2c099fe14f38de922fdc851bd4a6b))
	ROM_LOAD("gnru36.snd", 0x100000, 0x80000, CRC(5b32396e) SHA1(66462a6a929c869d668968e057fac199d05df267))
	ROM_LOAD("gnru37.snd", 0x180000, 0x80000, CRC(4930e1f2) SHA1(1569d0c7fea1af008acbdc492c3677ace7d1897a))
ROM_END

ROM_START(gnr_300f)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gnrcpuf.300", 0x0000, 0x10000, CRC(7f9006b2) SHA1(429d90fa27ea39176b94d1293a313ec3d1033dbc))
	ROM_REGION(0x80000, "decodmd2", 0)
	ROM_LOAD("gnrdispf.300", 0x00000, 0x80000, CRC(63e9761a) SHA1(05e5a61b66148da7728779d8e5fa14a489e09441))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("gnru7.snd", 0x0000, 0x10000, CRC(3b9de915) SHA1(a901a1f37bf5433c819393c4355f9d13164b32ce))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("gnru17.snd", 0x000000, 0x80000, CRC(3d3219d6) SHA1(ac4a6d3eff0cdd02b8c79dddcb8fec2e22faa9b9))
	ROM_LOAD("gnru21.snd", 0x080000, 0x80000, CRC(d2ca17ab) SHA1(db7c4f74a2e2c099fe14f38de922fdc851bd4a6b))
	ROM_LOAD("gnru36.snd", 0x100000, 0x80000, CRC(5b32396e) SHA1(66462a6a929c869d668968e057fac199d05df267))
	ROM_LOAD("gnru37.snd", 0x180000, 0x80000, CRC(4930e1f2) SHA1(1569d0c7fea1af008acbdc492c3677ace7d1897a))
ROM_END

ROM_START(gnr_300d)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gnrcpud.300", 0x0000, 0x10000, CRC(ae35f830) SHA1(adf853f50ed01c3261d7ce4064c45f834934b5e2))
	ROM_REGION(0x80000, "decodmd2", 0)
	ROM_LOAD("gnrdispd.300", 0x00000, 0x80000, CRC(4abf29e3) SHA1(595328e0f92a6e1972d71c56505a5dd07a373ef5))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("gnru7.snd", 0x0000, 0x10000, CRC(3b9de915) SHA1(a901a1f37bf5433c819393c4355f9d13164b32ce))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("gnru17.snd", 0x000000, 0x80000, CRC(3d3219d6) SHA1(ac4a6d3eff0cdd02b8c79dddcb8fec2e22faa9b9))
	ROM_LOAD("gnru21.snd", 0x080000, 0x80000, CRC(d2ca17ab) SHA1(db7c4f74a2e2c099fe14f38de922fdc851bd4a6b))
	ROM_LOAD("gnru36.snd", 0x100000, 0x80000, CRC(5b32396e) SHA1(66462a6a929c869d668968e057fac199d05df267))
	ROM_LOAD("gnru37.snd", 0x180000, 0x80000, CRC(4930e1f2) SHA1(1569d0c7fea1af008acbdc492c3677ace7d1897a))
ROM_END

ROM_START(gnr_200)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gnrcpua.200", 0x0000, 0x10000, CRC(365ddd22) SHA1(e316ddca4b68145e6f4efc2cd1d3f6d13fefad1d))
	ROM_REGION(0x80000, "decodmd2", 0)
	ROM_LOAD("gnrdispa.300", 0x00000, 0x80000, CRC(4abf29e3) SHA1(595328e0f92a6e1972d71c56505a5dd07a373ef5))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("gnru7.snd", 0x0000, 0x10000, CRC(3b9de915) SHA1(a901a1f37bf5433c819393c4355f9d13164b32ce))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("gnru17.snd", 0x000000, 0x80000, CRC(3d3219d6) SHA1(ac4a6d3eff0cdd02b8c79dddcb8fec2e22faa9b9))
	ROM_LOAD("gnru21.snd", 0x080000, 0x80000, CRC(d2ca17ab) SHA1(db7c4f74a2e2c099fe14f38de922fdc851bd4a6b))
	ROM_LOAD("gnru36.snd", 0x100000, 0x80000, CRC(5b32396e) SHA1(66462a6a929c869d668968e057fac199d05df267))
	ROM_LOAD("gnru37.snd", 0x180000, 0x80000, CRC(4930e1f2) SHA1(1569d0c7fea1af008acbdc492c3677ace7d1897a))
ROM_END

/*-------------------------------------------------------------
/ Hook - CPU Rev 3 /DMD  Type 1 128K Rom - CPU Rom
/------------------------------------------------------------*/
ROM_START(hook_408)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("hokcpua.408", 0x0000, 0x10000, CRC(46477fc7) SHA1(ce6228fd9ab4b6c774e128d291f50695746da358))
	ROM_REGION(0x20000, "decodmd1", 0)
	ROM_LOAD("hokdspa.401", 0x00000, 0x20000, CRC(59a07eb5) SHA1(d1ca41ce417f1772fe4da1eb37077f924b66ad36))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("hooksnd.u7", 0x8000, 0x8000, CRC(642f45b3) SHA1(a4b2084f32e52a596547384906281d04424332fc))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("hook-voi.u17", 0x000000, 0x40000, CRC(6ea9fcd2) SHA1(bffc66df542e06dedddaa403b5513446d9d6fc8c))
	ROM_LOAD("hook-voi.u21", 0x040000, 0x40000, CRC(b5c275e2) SHA1(ff51c2007132a1310ac53b5ab2a4af7d0ab15948))
ROM_END

ROM_START(hook_404)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("hokcpua.404", 0x0000, 0x10000, CRC(53357d8b) SHA1(4e8f5f4376418fbac782065c602da82acab06ef3))
	ROM_REGION(0x20000, "decodmd1", 0)
	ROM_LOAD("hokdspa.401", 0x00000, 0x20000, CRC(59a07eb5) SHA1(d1ca41ce417f1772fe4da1eb37077f924b66ad36))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("hooksnd.u7", 0x8000, 0x8000, CRC(642f45b3) SHA1(a4b2084f32e52a596547384906281d04424332fc))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("hook-voi.u17", 0x000000, 0x40000, CRC(6ea9fcd2) SHA1(bffc66df542e06dedddaa403b5513446d9d6fc8c))
	ROM_LOAD("hook-voi.u21", 0x040000, 0x40000, CRC(b5c275e2) SHA1(ff51c2007132a1310ac53b5ab2a4af7d0ab15948))
ROM_END

ROM_START(hook_401)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("hokcpua.401", 0x0000, 0x10000, CRC(20223298) SHA1(a8063765db947b059eadaad6654ed0c5cad9198d))
	ROM_REGION(0x20000, "decodmd1", 0)
	ROM_LOAD("hokdspa.401", 0x00000, 0x20000, CRC(59a07eb5) SHA1(d1ca41ce417f1772fe4da1eb37077f924b66ad36))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("hooksnd.u7", 0x8000, 0x8000, CRC(642f45b3) SHA1(a4b2084f32e52a596547384906281d04424332fc))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("hook-voi.u17", 0x000000, 0x40000, CRC(6ea9fcd2) SHA1(bffc66df542e06dedddaa403b5513446d9d6fc8c))
	ROM_LOAD("hook-voi.u21", 0x040000, 0x40000, CRC(b5c275e2) SHA1(ff51c2007132a1310ac53b5ab2a4af7d0ab15948))
ROM_END

ROM_START(hook_f401)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("hokcpua.401", 0x0000, 0x10000, CRC(20223298) SHA1(a8063765db947b059eadaad6654ed0c5cad9198d)) // unknown if there is a special french CPU 4.01, as the dump only included the display ROM
	ROM_REGION(0x20000, "decodmd1", 0)
	ROM_LOAD("hook_display_f401.bin", 0x00000, 0x20000, CRC(b501edbd) SHA1(df369f569243d633aa24edd4289ace645e4a9358))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("hooksnd.u7", 0x8000, 0x8000, CRC(642f45b3) SHA1(a4b2084f32e52a596547384906281d04424332fc))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("hook-voi.u17", 0x000000, 0x40000, CRC(6ea9fcd2) SHA1(bffc66df542e06dedddaa403b5513446d9d6fc8c))
	ROM_LOAD("hook-voi.u21", 0x040000, 0x40000, CRC(b5c275e2) SHA1(ff51c2007132a1310ac53b5ab2a4af7d0ab15948))
ROM_END

ROM_START(hook_401_p)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("hokcpua.401", 0x0000, 0x10000, CRC(20223298) SHA1(a8063765db947b059eadaad6654ed0c5cad9198d))
	ROM_REGION(0x20000, "decodmd1", 0)
	ROM_LOAD("hokdspa.401", 0x00000, 0x20000, CRC(59a07eb5) SHA1(d1ca41ce417f1772fe4da1eb37077f924b66ad36))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("hooksnd_p.u7", 0x8000, 0x8000, CRC(20091293) SHA1(fdfc4eadef0bf1915c7c72c1fd8dafaa429b3c44))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("hook-voi_p.u17", 0x000000, 0x40000, CRC(667cf0fb) SHA1(dd12a7fa280384381ebc5c3d8add652eddb294fb))
	ROM_LOAD("hook-voi_p.u21", 0x040000, 0x40000, CRC(04775416) SHA1(5675aea39b76178ff476b0f627223a1c75a3d6b7))
ROM_END

ROM_START(hook_400)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("hokcpua.401", 0x0000, 0x10000, CRC(20223298) SHA1(a8063765db947b059eadaad6654ed0c5cad9198d))// no CPU-Version < 4.01 yet
	ROM_REGION(0x20000, "decodmd1", 0)
	ROM_LOAD("hook_dspl_4.00.bin", 0x00000, 0x20000, CRC(14d2387c) SHA1(b3e78ffa7e9bdd4bc7fe08e3a0a8631178a5fc09))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("hooksnd.u7", 0x8000, 0x8000, CRC(642f45b3) SHA1(a4b2084f32e52a596547384906281d04424332fc))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("hook-voi.u17", 0x000000, 0x40000, CRC(6ea9fcd2) SHA1(bffc66df542e06dedddaa403b5513446d9d6fc8c))
	ROM_LOAD("hook-voi.u21", 0x040000, 0x40000, CRC(b5c275e2) SHA1(ff51c2007132a1310ac53b5ab2a4af7d0ab15948))
ROM_END

ROM_START(hook_e406)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("hokcpue.406", 0x0000, 0x10000, CRC(0e2893e2) SHA1(fb13f34a45ec75d9cc1439c90b10c0b1ad38d1f6))
	ROM_REGION(0x20000, "decodmd1", 0)
	ROM_LOAD("hokdspa.401", 0x00000, 0x20000, CRC(59a07eb5) SHA1(d1ca41ce417f1772fe4da1eb37077f924b66ad36))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("hooksnd.u7", 0x8000, 0x8000, CRC(642f45b3) SHA1(a4b2084f32e52a596547384906281d04424332fc))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("hook-voi.u17", 0x000000, 0x40000, CRC(6ea9fcd2) SHA1(bffc66df542e06dedddaa403b5513446d9d6fc8c))
	ROM_LOAD("hook-voi.u21", 0x040000, 0x40000, CRC(b5c275e2) SHA1(ff51c2007132a1310ac53b5ab2a4af7d0ab15948))
ROM_END

/*-------------------------------------------------------------
/ Jurassic Park - CPU Rev 3b /DMD  Type 2 512K Rom - 64K CPU Rom
/------------------------------------------------------------*/
ROM_START(jupk_513)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("jpcpua.513", 0x0000, 0x10000, CRC(9f70a937) SHA1(cdea6c6e852982eb5e800db138f7660d51b6fdc8))
	ROM_REGION(0x80000, "decodmd2", 0)
	ROM_LOAD("jpdspa.510", 0x00000, 0x80000, CRC(9ca61e3c) SHA1(38ae472f38e6fc33671e9a276313208e5ccd8640))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("jpu7.dat", 0x0000, 0x10000, CRC(f3afcf13) SHA1(64e12f9d42c00ae08a4584b2ebea475566b90c13))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("jpu17.dat", 0x000000, 0x80000, CRC(38135a23) SHA1(7c284c17783269824a3d3e83c4cd8ead27133309))
	ROM_LOAD("jpu21.dat", 0x080000, 0x40000, CRC(6ac1554c) SHA1(9a91ce836c089f96ad9c809bb66fcddda1f3e456))
ROM_END

ROM_START(jupk_501)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("jpcpua.501", 0x0000, 0x10000, CRC(d25f09c4) SHA1(a12ace496352002685b0415515f5f5ce4fc95bdb))
	ROM_REGION(0x80000, "decodmd2", 0)
	ROM_LOAD("jpdspa.501", 0x00000, 0x80000, CRC(04a87d42) SHA1(e13df9a63ec77ec6f97b681ed99216ef3f3af691))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("jpu7.dat", 0x0000, 0x10000, CRC(f3afcf13) SHA1(64e12f9d42c00ae08a4584b2ebea475566b90c13))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("jpu17.dat", 0x000000, 0x80000, CRC(38135a23) SHA1(7c284c17783269824a3d3e83c4cd8ead27133309))
	ROM_LOAD("jpu21.dat", 0x080000, 0x40000, CRC(6ac1554c) SHA1(9a91ce836c089f96ad9c809bb66fcddda1f3e456))
ROM_END

ROM_START(jupk_501g)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("jpcpua.501", 0x0000, 0x10000, CRC(d25f09c4) SHA1(a12ace496352002685b0415515f5f5ce4fc95bdb))
	ROM_REGION(0x80000, "decodmd2", 0)
	ROM_LOAD("jpdspg.501", 0x00000, 0x80000, CRC(3b524bfe) SHA1(ea6ae6f8fc8379f311fd7ef456f0d6711c4e35c5))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("jpu7.dat", 0x0000, 0x10000, CRC(f3afcf13) SHA1(64e12f9d42c00ae08a4584b2ebea475566b90c13))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("jpu17.dat", 0x000000, 0x80000, CRC(38135a23) SHA1(7c284c17783269824a3d3e83c4cd8ead27133309))
	ROM_LOAD("jpu21.dat", 0x080000, 0x40000, CRC(6ac1554c) SHA1(9a91ce836c089f96ad9c809bb66fcddda1f3e456))
ROM_END

ROM_START(jupk_307)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("jpcpua.307", 0x0000, 0x10000, CRC(b60c3bca) SHA1(0f5619319d2affefa993f396f7a4b1875eea81ab))
	ROM_REGION(0x80000, "decodmd2", 0)
	ROM_LOAD("jpdspa.400", 0x00000, 0x80000, CRC(4c044f05) SHA1(573a188a255ad3b6aa18427fd6b45aeca6f83e04))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("jpu7.dat", 0x0000, 0x10000, CRC(f3afcf13) SHA1(64e12f9d42c00ae08a4584b2ebea475566b90c13))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("jpu17.dat", 0x000000, 0x80000, CRC(38135a23) SHA1(7c284c17783269824a3d3e83c4cd8ead27133309))
	ROM_LOAD("jpu21.dat", 0x080000, 0x40000, CRC(6ac1554c) SHA1(9a91ce836c089f96ad9c809bb66fcddda1f3e456))
ROM_END

ROM_START(jupk_305)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("jpcpua3.05.bin", 0x0000, 0x10000, CRC(0a9bd439) SHA1(486df3e268c81518ff4d0638517e93b57a8d9d2e))
	ROM_REGION(0x80000, "decodmd2", 0)
	ROM_LOAD("jpdspa.400", 0x00000, 0x80000, CRC(4c044f05) SHA1(573a188a255ad3b6aa18427fd6b45aeca6f83e04)) // Not dumped on this set, using 4.00 from the 3.07 revision
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("jpu7.dat", 0x0000, 0x10000, CRC(f3afcf13) SHA1(64e12f9d42c00ae08a4584b2ebea475566b90c13))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("jpu17.dat", 0x000000, 0x80000, CRC(38135a23) SHA1(7c284c17783269824a3d3e83c4cd8ead27133309))
	ROM_LOAD("jpu21.dat", 0x080000, 0x40000, CRC(6ac1554c) SHA1(9a91ce836c089f96ad9c809bb66fcddda1f3e456))
ROM_END

/*-------------------------------------------------------------
/ Last Action Hero - CPU Rev 3b /DMD  Type 2 512K Rom - 64K CPU Rom
/------------------------------------------------------------*/
ROM_START(lah_112)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("lahcpua.112", 0x0000, 0x10000, CRC(e7422236) SHA1(c0422fa6d29fe615cb718056bea00eb9a80ce803))
	ROM_REGION(0x80000, "decodmd2", 0)
	ROM_LOAD("lahdispa.106", 0x00000, 0x80000, CRC(ca6cfec5) SHA1(5e2081387d76bed17c14120cd347d6aaf435276b))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("lahsnd.u7", 0x0000, 0x10000, CRC(0279c45b) SHA1(14daf6b711d1936352209e90240f51812ebe76e0))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("lahsnd.u17", 0x000000, 0x80000, CRC(d0c15fa6) SHA1(5dcd13b578fa53c82353cda5aa774ca216c5ddfe))
	ROM_LOAD("lahsnd.u21", 0x080000, 0x40000, CRC(4571dc2e) SHA1(a1068cb080c30dbc07d164eddfc5dfd0afd52d3b))
ROM_END

ROM_START(lah_110)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("lahcpua.110", 0x0000, 0x10000, CRC(d1861dc2) SHA1(288bd06b6ae346d1f6a17a642d5533f1a9a3bf5e))
	ROM_REGION(0x80000, "decodmd2", 0)
	ROM_LOAD("lahdispa.106", 0x00000, 0x80000, CRC(ca6cfec5) SHA1(5e2081387d76bed17c14120cd347d6aaf435276b))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("lahsnd.u7", 0x0000, 0x10000, CRC(0279c45b) SHA1(14daf6b711d1936352209e90240f51812ebe76e0))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("lahsnd.u17", 0x000000, 0x80000, CRC(d0c15fa6) SHA1(5dcd13b578fa53c82353cda5aa774ca216c5ddfe))
	ROM_LOAD("lahsnd.u21", 0x080000, 0x40000, CRC(4571dc2e) SHA1(a1068cb080c30dbc07d164eddfc5dfd0afd52d3b))
ROM_END

// Loose display ROM
ROM_START(lah_xxx_s105)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("lahcpux.xxx", 0x0000, 0x10000, NO_DUMP)
	ROM_REGION(0x80000, "decodmd2", 0)
	ROM_LOAD("lahdispl.105", 0x00000, 0x80000, CRC(eb861132) SHA1(46786c55256bd6da491bacbf53c4fac444d9d3d4))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("lahsnd.u7", 0x0000, 0x10000, BAD_DUMP CRC(0279c45b) SHA1(14daf6b711d1936352209e90240f51812ebe76e0)) // Not dumped on this set
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("lahsnd.u17", 0x000000, 0x80000, BAD_DUMP CRC(d0c15fa6) SHA1(5dcd13b578fa53c82353cda5aa774ca216c5ddfe)) // Not dumped on this set
	ROM_LOAD("lahsnd.u21", 0x080000, 0x40000, BAD_DUMP CRC(4571dc2e) SHA1(a1068cb080c30dbc07d164eddfc5dfd0afd52d3b)) // Not dumped on this set
ROM_END

ROM_START(lah_108s)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("lahcpua.108", 0x0000, 0x10000, CRC(8942794b) SHA1(f023ca040d6d4c6da80b58a162f1d217e571ed81))
	ROM_REGION(0x80000, "decodmd2", 0)
	ROM_LOAD("lahdispl.104", 0x00000, 0x80000, CRC(6b1e51a7) SHA1(ad17507b63f2da8aa0651401ccb8d449c15aa46c))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("lahsnd.u7", 0x0000, 0x10000, CRC(0279c45b) SHA1(14daf6b711d1936352209e90240f51812ebe76e0))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("lahsnd.u17", 0x000000, 0x80000, CRC(d0c15fa6) SHA1(5dcd13b578fa53c82353cda5aa774ca216c5ddfe))
	ROM_LOAD("lahsnd.u21", 0x080000, 0x40000, CRC(4571dc2e) SHA1(a1068cb080c30dbc07d164eddfc5dfd0afd52d3b))
ROM_END

ROM_START(lah_107)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("lah_c5_a107.rom", 0x0000, 0x10000, CRC(f777fc1b) SHA1(a1a645df907e1e88123113823d3edf12c4e1e5df))
	ROM_REGION(0x80000, "decodmd2", 0)
	ROM_LOAD("lahdispa.106", 0x00000, 0x80000, CRC(ca6cfec5) SHA1(5e2081387d76bed17c14120cd347d6aaf435276b))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("lahsnd.u7", 0x0000, 0x10000, CRC(0279c45b) SHA1(14daf6b711d1936352209e90240f51812ebe76e0))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("lahsnd.u17", 0x000000, 0x80000, CRC(d0c15fa6) SHA1(5dcd13b578fa53c82353cda5aa774ca216c5ddfe))
	ROM_LOAD("lahsnd.u21", 0x080000, 0x40000, CRC(4571dc2e) SHA1(a1068cb080c30dbc07d164eddfc5dfd0afd52d3b))
ROM_END

ROM_START(lah_106c)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("lahcpuc.106", 0x0000, 0x10000, CRC(d4be4178) SHA1(ea2d9c780f6636a8768164d3a1bb33b050c3a2a7))
	ROM_REGION(0x80000, "decodmd2", 0)
	ROM_LOAD("lahdispa.104", 0x00000, 0x80000, CRC(baf4e7b3) SHA1(78924d992c0e206bfbf4a6fcc62ea7f91e995260))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("lahsnd.u7", 0x0000, 0x10000, CRC(0279c45b) SHA1(14daf6b711d1936352209e90240f51812ebe76e0))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("lahsnd.u17", 0x000000, 0x80000, CRC(d0c15fa6) SHA1(5dcd13b578fa53c82353cda5aa774ca216c5ddfe))
	ROM_LOAD("lahsnd.u21", 0x080000, 0x40000, CRC(4571dc2e) SHA1(a1068cb080c30dbc07d164eddfc5dfd0afd52d3b))
ROM_END

ROM_START(lah_104f)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("lahcpua.104", 0x0000, 0x10000, CRC(49b9e5e9) SHA1(cf6198e4c93ce839dc6e5231090d4ca56e9bdea2))
	ROM_REGION(0x80000, "decodmd2", 0)
	ROM_LOAD("lahdispf.101", 0x00000, 0x80000, CRC(826a0a8b) SHA1(daad062edd8b6f468991d941e40d86711f8505df))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("lahsnd.u7", 0x0000, 0x10000, CRC(0279c45b) SHA1(14daf6b711d1936352209e90240f51812ebe76e0))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("lahsnd.u17", 0x000000, 0x80000, CRC(d0c15fa6) SHA1(5dcd13b578fa53c82353cda5aa774ca216c5ddfe))
	ROM_LOAD("lahsnd.u21", 0x080000, 0x40000, CRC(4571dc2e) SHA1(a1068cb080c30dbc07d164eddfc5dfd0afd52d3b))
ROM_END

ROM_START(lah_104s)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("lahcpua.104", 0x0000, 0x10000, CRC(49b9e5e9) SHA1(cf6198e4c93ce839dc6e5231090d4ca56e9bdea2))
	ROM_REGION(0x80000, "decodmd2", 0)
	ROM_LOAD("lahdispl.102", 0x00000, 0x80000, CRC(3482c349) SHA1(8f03ba28132ea5159d3193b3adb7b4a6a43046c6))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("lahsnd.u7", 0x0000, 0x10000, CRC(0279c45b) SHA1(14daf6b711d1936352209e90240f51812ebe76e0))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("lahsnd.u17", 0x000000, 0x80000, CRC(d0c15fa6) SHA1(5dcd13b578fa53c82353cda5aa774ca216c5ddfe))
	ROM_LOAD("lahsnd.u21", 0x080000, 0x40000, CRC(4571dc2e) SHA1(a1068cb080c30dbc07d164eddfc5dfd0afd52d3b))
ROM_END

/*----------------------------------------------------------------
/ Lethal Weapon 3 - CPU Rev 3 /DMD  Type 2 512K Rom - 64K CPU Rom
/---------------------------------------------------------------*/
ROM_START(lw3_208)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("lw3cpuu.208", 0x0000, 0x10000, CRC(a3041f8a) SHA1(3c5b8525b8e9b924590648429c56aaf97adee460))
	ROM_REGION(0x80000, "decodmd2", 0)
	ROM_LOAD("lw3drom1.a26", 0x00000, 0x40000, CRC(44a4cf81) SHA1(c7f3e3d5fbe930650e48423c8ba0ac484ce0640c))
	ROM_LOAD("lw3drom0.a26", 0x40000, 0x40000, CRC(22932ed5) SHA1(395aa376cd8562de7956a6e34b8747e7cf81f935))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("lw3u7.dat", 0x8000, 0x8000, CRC(ba845ac3) SHA1(bb50413ace1885870cb3817edae478904b0eefb8))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("lw3u17.dat", 0x000000, 0x40000, CRC(e34cf2fc) SHA1(417c83ded6637f891c8bb42b32d6898c90a0e5cf))
	ROM_LOAD("lw3u21.dat", 0x040000, 0x40000, CRC(82bed051) SHA1(49ddc4190762d9b473fda270e0d6d88a4422d5d7))
ROM_END

ROM_START(lw3_207)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("lw3ugc5.207", 0x0000, 0x10000, CRC(edca3e08) SHA1(6c9714a2021acc8c0965f96a1af8b33c87a1708d))
	ROM_REGION(0x80000, "decodmd2", 0)
	ROM_LOAD("lw3drom1.a26", 0x00000, 0x40000, CRC(44a4cf81) SHA1(c7f3e3d5fbe930650e48423c8ba0ac484ce0640c))
	ROM_LOAD("lw3drom0.a26", 0x40000, 0x40000, CRC(22932ed5) SHA1(395aa376cd8562de7956a6e34b8747e7cf81f935))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("lw3u7.dat", 0x8000, 0x8000, CRC(ba845ac3) SHA1(bb50413ace1885870cb3817edae478904b0eefb8))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("lw3u17.dat", 0x000000, 0x40000, CRC(e34cf2fc) SHA1(417c83ded6637f891c8bb42b32d6898c90a0e5cf))
	ROM_LOAD("lw3u21.dat", 0x040000, 0x40000, CRC(82bed051) SHA1(49ddc4190762d9b473fda270e0d6d88a4422d5d7))
ROM_END

ROM_START(lw3_207c)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("lw3gc5.207", 0x0000, 0x10000, CRC(27aeaea9) SHA1(f8c40cbc37edac20187ac880be281dd45d8ad614))
	ROM_REGION(0x80000, "decodmd2", 0)
	ROM_LOAD("lw3drom1.a26", 0x00000, 0x40000, CRC(44a4cf81) SHA1(c7f3e3d5fbe930650e48423c8ba0ac484ce0640c))
	ROM_LOAD("lw3drom0.a26", 0x40000, 0x40000, CRC(22932ed5) SHA1(395aa376cd8562de7956a6e34b8747e7cf81f935))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("lw3u7.dat", 0x8000, 0x8000, CRC(ba845ac3) SHA1(bb50413ace1885870cb3817edae478904b0eefb8))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("lw3u17.dat", 0x000000, 0x40000, CRC(e34cf2fc) SHA1(417c83ded6637f891c8bb42b32d6898c90a0e5cf))
	ROM_LOAD("lw3u21.dat", 0x040000, 0x40000, CRC(82bed051) SHA1(49ddc4190762d9b473fda270e0d6d88a4422d5d7))
ROM_END

ROM_START(lw3_205)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("lw3gc5.205", 0x0000, 0x10000, CRC(5ad8ff4a) SHA1(6a01a2195543c0c57ce4ce78703c91500835a2da))
	ROM_REGION(0x80000, "decodmd2", 0)
	ROM_LOAD("lw3dsp1.205", 0x00000, 0x40000, CRC(9dfeffb4) SHA1(f62f2a884da68b4dbfe7da071058dc8cd1766c36))
	ROM_LOAD("lw3dsp0.205", 0x40000, 0x40000, CRC(bd8156f1) SHA1(b18214af1b79cca79bdc634c175c3bf7d0052843))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("lw3u7.dat", 0x8000, 0x8000, CRC(ba845ac3) SHA1(bb50413ace1885870cb3817edae478904b0eefb8))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("lw3u17.dat", 0x000000, 0x40000, CRC(e34cf2fc) SHA1(417c83ded6637f891c8bb42b32d6898c90a0e5cf))
	ROM_LOAD("lw3u21.dat", 0x040000, 0x40000, CRC(82bed051) SHA1(49ddc4190762d9b473fda270e0d6d88a4422d5d7))
ROM_END

ROM_START(lw3_204e)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("lw3_cpu_c5_v2.04.bin", 0x00000, 0x10000, CRC(33cb9197) SHA1(c6b25dfd93bb5c425a606ae21f757a87a07dc320))
	ROM_REGION(0x80000, "decodmd2", 0)
	ROM_LOAD("lw3_display_a2.02_rom1.u14", 0x00000, 0x40000, CRC(4920f84f) SHA1(928e4aefdcf9462201001f4ac03d56a0cda25ec1))
	ROM_LOAD("lw3_display_a2.02_rom0.u12", 0x40000, 0x40000, CRC(f0ac3da3) SHA1(2e65f31e65302a5d343915488b229769f9784657))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("lethal_0601.u7", 0x08000, 0x08000, CRC(ba845ac3) SHA1(bb50413ace1885870cb3817edae478904b0eefb8))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("lethal_0601.u17", 0x00000, 0x40000, CRC(e34cf2fc) SHA1(417c83ded6637f891c8bb42b32d6898c90a0e5cf))
	ROM_LOAD("lethal_0601.u21", 0x40000, 0x40000, CRC(82bed051) SHA1(49ddc4190762d9b473fda270e0d6d88a4422d5d7))
ROM_END

ROM_START(lw3_203)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("lw3cpuu.203", 0x0000, 0x10000, CRC(0cfa38d4) SHA1(11d2e101a574c2dfec49ec701f480173b84c842e))
	ROM_REGION(0x80000, "decodmd2", 0)
	ROM_LOAD("lw3dsp1.204", 0x00000, 0x40000, CRC(1ba79363) SHA1(46d489a1190533c73370acd8a48cef60d12f87ce))
	ROM_LOAD("lw3dsp0.204", 0x40000, 0x40000, CRC(c74d3cf2) SHA1(076ee9b2e3cad0b8058ac0c70f5ffe7e29f3eff5))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("lw3u7.dat", 0x8000, 0x8000, CRC(ba845ac3) SHA1(bb50413ace1885870cb3817edae478904b0eefb8))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("lw3u17.dat", 0x000000, 0x40000, CRC(e34cf2fc) SHA1(417c83ded6637f891c8bb42b32d6898c90a0e5cf))
	ROM_LOAD("lw3u21.dat", 0x040000, 0x40000, CRC(82bed051) SHA1(49ddc4190762d9b473fda270e0d6d88a4422d5d7))
ROM_END

ROM_START(lw3_200)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("lw3cpu.200", 0x0000, 0x10000, CRC(ddb6e7a7) SHA1(d48309e1984ef9a7682dfde190cf457632044657))
	ROM_REGION(0x80000, "decodmd2", 0)
	ROM_LOAD("lw3dsp1.204", 0x00000, 0x40000, CRC(1ba79363) SHA1(46d489a1190533c73370acd8a48cef60d12f87ce))
	ROM_LOAD("lw3dsp0.204", 0x40000, 0x40000, CRC(c74d3cf2) SHA1(076ee9b2e3cad0b8058ac0c70f5ffe7e29f3eff5))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("lw3u7.dat", 0x8000, 0x8000, CRC(ba845ac3) SHA1(bb50413ace1885870cb3817edae478904b0eefb8))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("lw3u17.dat", 0x000000, 0x40000, CRC(e34cf2fc) SHA1(417c83ded6637f891c8bb42b32d6898c90a0e5cf))
	ROM_LOAD("lw3u21.dat", 0x040000, 0x40000, CRC(82bed051) SHA1(49ddc4190762d9b473fda270e0d6d88a4422d5d7))
ROM_END

/*-------------------------------------------------------------
/ Michael Jordan - CPU Rev 3 / DMD Type 2 512K Rom - 64K CPU Rom
/------------------------------------------------------------*/

ROM_START(mj_130)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("mjcpuc5.bin", 0x0000, 0x10000, CRC(311ab1d1) SHA1(062b02aab851f9f2ca64c24b8faa7dd293cacd22))
	ROM_REGION(0x80000, "decodmd2", 0)
	ROM_LOAD("mjdsp0.bin", 0x00000, 0x80000, CRC(1e2f27e8) SHA1(bfc567d6d3a7cecf7623ceb383350c78c14baef3))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("mjsndu7.bin", 0x0000, 0x10000, CRC(a32237f5) SHA1(0fc106429af320c4a30a99c67b45f44cb9a45644))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("mjsndu17.bin", 0x000000, 0x80000, CRC(8b11d7b9) SHA1(bb84b1650b253a433e947137256e4bc34a6ceac4))
	ROM_LOAD("mjsndu21.bin", 0x080000, 0x80000, CRC(addfe20e) SHA1(3a6862640f81493da1beddca11011090d8b7cab0))
ROM_END

/*-------------------------------------------------------------
/ Star Trek - CPU Rev 3 /DMD Type 1 128K Rom - 64K CPU Rom
/------------------------------------------------------------*/
ROM_START(trek_201)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("trekcpuu.201", 0x0000, 0x10000, CRC(ea0681fe) SHA1(282c8181e60da6358ef320358575a538aa4abe8c))
	ROM_REGION(0x20000, "decodmd1", 0)
	ROM_LOAD("trekdspa.109", 0x00000, 0x20000, CRC(a7e7d44d) SHA1(d26126310b8b316ca161d4202645de8fb6359822))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("trek.u7", 0x8000, 0x8000, CRC(f137abbb) SHA1(11731170ed4f04dd8af05d8f79ad727b0e0104d7))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("trek.u17", 0x000000, 0x40000, CRC(531545da) SHA1(905f34173db0e04eaf5236191186ea209b8a0a34))
	ROM_LOAD("trek.u21", 0x040000, 0x40000, CRC(6107b004) SHA1(1f9bed9b06d5b19fbc0cc0bef2e493eb1a3f1aa4))
ROM_END

ROM_START(trek_200)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("trekcpuu.200", 0x0000, 0x10000, CRC(4528e803) SHA1(0ebb16ab8b95f04a19fa4510e58c01493393d48c))
	ROM_REGION(0x20000, "decodmd1", 0)
	ROM_LOAD("trekdspa.109", 0x00000, 0x20000, CRC(a7e7d44d) SHA1(d26126310b8b316ca161d4202645de8fb6359822))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("trek.u7", 0x8000, 0x8000, CRC(f137abbb) SHA1(11731170ed4f04dd8af05d8f79ad727b0e0104d7))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("trek.u17", 0x000000, 0x40000, CRC(531545da) SHA1(905f34173db0e04eaf5236191186ea209b8a0a34))
	ROM_LOAD("trek.u21", 0x040000, 0x40000, CRC(6107b004) SHA1(1f9bed9b06d5b19fbc0cc0bef2e493eb1a3f1aa4))
ROM_END

ROM_START(trek_120)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("trekcpu.120", 0x0000, 0x10000, CRC(2cac0731) SHA1(abf68c358c50bdeb36714cca0a9848e398a6f9fc))
	ROM_REGION(0x20000, "decodmd1", 0)
	ROM_LOAD("trekdsp.106", 0x00000, 0x20000, CRC(dc3bf312) SHA1(3262d6604d1dcd1dc738bc3f919a3319b783fd73))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("trek.u7", 0x8000, 0x8000, CRC(f137abbb) SHA1(11731170ed4f04dd8af05d8f79ad727b0e0104d7))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("trek.u17", 0x000000, 0x40000, CRC(531545da) SHA1(905f34173db0e04eaf5236191186ea209b8a0a34))
	ROM_LOAD("trek.u21", 0x040000, 0x40000, CRC(6107b004) SHA1(1f9bed9b06d5b19fbc0cc0bef2e493eb1a3f1aa4))
ROM_END

ROM_START(trek_117)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("trekcpu.117", 0x0000, 0x10000, CRC(534ebb09) SHA1(96f343fcc7b0f39e0a8ec7df47cea433ad2c9119))
	ROM_REGION(0x20000, "decodmd1", 0)
	ROM_LOAD("trekdspa.109", 0x00000, 0x20000, CRC(a7e7d44d) SHA1(d26126310b8b316ca161d4202645de8fb6359822))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("trek.u7", 0x8000, 0x8000, CRC(f137abbb) SHA1(11731170ed4f04dd8af05d8f79ad727b0e0104d7))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("trek.u17", 0x000000, 0x40000, CRC(531545da) SHA1(905f34173db0e04eaf5236191186ea209b8a0a34))
	ROM_LOAD("trek.u21", 0x040000, 0x40000, CRC(6107b004) SHA1(1f9bed9b06d5b19fbc0cc0bef2e493eb1a3f1aa4))
ROM_END

ROM_START(trek_110)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("trekcpu.110", 0x0000, 0x10000, CRC(06e0f87b) SHA1(989d70e067cd322351768550549a4e2c8923132c))
	ROM_REGION(0x20000, "decodmd1", 0)
	ROM_LOAD("trekdsp.106", 0x00000, 0x20000, CRC(dc3bf312) SHA1(3262d6604d1dcd1dc738bc3f919a3319b783fd73))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("trek.u7", 0x8000, 0x8000, CRC(f137abbb) SHA1(11731170ed4f04dd8af05d8f79ad727b0e0104d7))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("trek.u17", 0x000000, 0x40000, CRC(531545da) SHA1(905f34173db0e04eaf5236191186ea209b8a0a34))
	ROM_LOAD("trek.u21", 0x040000, 0x40000, CRC(6107b004) SHA1(1f9bed9b06d5b19fbc0cc0bef2e493eb1a3f1aa4))
ROM_END

ROM_START(trek_110_a027)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("trekcpu.110", 0x0000, 0x10000, CRC(06e0f87b) SHA1(989d70e067cd322351768550549a4e2c8923132c))
	ROM_REGION(0x20000, "decodmd1", 0)
	ROM_LOAD("trekadsp.bin", 0x00000, 0x20000, CRC(54681627) SHA1(4251fa0568d2e869b44358471a3d4a4e88443954))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("trek.u7", 0x8000, 0x8000, CRC(f137abbb) SHA1(11731170ed4f04dd8af05d8f79ad727b0e0104d7))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("trek.u17", 0x000000, 0x40000, CRC(531545da) SHA1(905f34173db0e04eaf5236191186ea209b8a0a34))
	ROM_LOAD("trek.u21", 0x040000, 0x40000, CRC(6107b004) SHA1(1f9bed9b06d5b19fbc0cc0bef2e493eb1a3f1aa4))
ROM_END

/*-------------------------------------------------------------
/ Star Wars - CPU Rev 3 /DMD  Type 2 512K Rom - 64K CPU Rom
/------------------------------------------------------------*/
ROM_START(stwr_106)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("starcpua.106", 0x0000, 0x10000, CRC(35d3cfd9) SHA1(14d8960f3657d7cd977b0a749e995aadb3fd4c7c))
	ROM_REGION(0x80000, "decodmd2", 0)
	ROM_LOAD("sw4mrom.a15", 0x00000, 0x80000, CRC(00c87952) SHA1(cd2f491f03fcb3e3ceff7ee7f678aa1957a5d14b))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("s-wars.u7", 0x8000, 0x8000, CRC(cefa19d5) SHA1(7ddf9cc85ab601514305bc46083a07a3d087b286))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("s-wars.u17", 0x000000, 0x80000, CRC(7950a147) SHA1(f5bcd5cf6b35f9e4f14d62b084495c3a743d92a1))
	ROM_LOAD("s-wars.u21", 0x080000, 0x40000, CRC(7b08fdf1) SHA1(489d21a10e97e886f948d81dedd7f8de3acecd2b))
ROM_END

ROM_START(stwr_106_s105)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("starcpua.106", 0x0000, 0x10000, CRC(35d3cfd9) SHA1(14d8960f3657d7cd977b0a749e995aadb3fd4c7c))
	ROM_REGION(0x80000, "decodmd2", 0)
	ROM_LOAD("sw4mrom.s15", 0x00000, 0x80000, CRC(158867b9) SHA1(45a0f4d26c21e2259aeb2a726a1eac23744213a2))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("s-wars.u7", 0x8000, 0x8000, CRC(cefa19d5) SHA1(7ddf9cc85ab601514305bc46083a07a3d087b286))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("s-wars.u17", 0x000000, 0x80000, CRC(7950a147) SHA1(f5bcd5cf6b35f9e4f14d62b084495c3a743d92a1))
	ROM_LOAD("s-wars.u21", 0x080000, 0x40000, CRC(7b08fdf1) SHA1(489d21a10e97e886f948d81dedd7f8de3acecd2b))
ROM_END

ROM_START(stwr_106_a046) // DISPLAY VERSION- STAR WARS A0.46 10/9/1992
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("starcpua.106", 0x0000, 0x10000, CRC(35d3cfd9) SHA1(14d8960f3657d7cd977b0a749e995aadb3fd4c7c))
	ROM_REGION(0x80000, "decodmd2", 0)
	ROM_LOAD("sw4mrom1.a046", 0x00000, 0x40000, CRC(5ceac219) SHA1(76b7acf378f83bacf6c4adb020d6e544eacbac7a))
	ROM_LOAD("sw4mrom0.a046", 0x40000, 0x40000, CRC(305e45be) SHA1(fbdc90175467a9ee59dc11c5ccbe83130b3644c8))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("s-wars.u7", 0x8000, 0x8000, CRC(cefa19d5) SHA1(7ddf9cc85ab601514305bc46083a07a3d087b286))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("s-wars.u17", 0x000000, 0x80000, CRC(7950a147) SHA1(f5bcd5cf6b35f9e4f14d62b084495c3a743d92a1))
	ROM_LOAD("s-wars.u21", 0x080000, 0x40000, CRC(7b08fdf1) SHA1(489d21a10e97e886f948d81dedd7f8de3acecd2b))
ROM_END

ROM_START(stwr_104)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("starcpua.104", 0x0000, 0x10000, CRC(12b87cfa) SHA1(12e0ab52f6784beefce8291d29b8aff01b2f2818))
	ROM_REGION(0x80000, "decodmd2", 0)
	ROM_LOAD("sw4mrom.a15", 0x00000, 0x80000, CRC(00c87952) SHA1(cd2f491f03fcb3e3ceff7ee7f678aa1957a5d14b))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("s-wars.u7", 0x8000, 0x8000, CRC(cefa19d5) SHA1(7ddf9cc85ab601514305bc46083a07a3d087b286))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("s-wars.u17", 0x000000, 0x80000, CRC(7950a147) SHA1(f5bcd5cf6b35f9e4f14d62b084495c3a743d92a1))
	ROM_LOAD("s-wars.u21", 0x080000, 0x40000, CRC(7b08fdf1) SHA1(489d21a10e97e886f948d81dedd7f8de3acecd2b))
ROM_END

ROM_START(stwr_103)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("starcpua.103", 0x0000, 0x10000, CRC(318085ca) SHA1(7c35bdee52e8093fe05f0624615baabe559a1917))
	ROM_REGION(0x80000, "decodmd2", 0)
	ROM_LOAD("sw4mrom.a15", 0x00000, 0x80000, CRC(00c87952) SHA1(cd2f491f03fcb3e3ceff7ee7f678aa1957a5d14b))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("s-wars.u7", 0x8000, 0x8000, CRC(cefa19d5) SHA1(7ddf9cc85ab601514305bc46083a07a3d087b286))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("s-wars.u17", 0x000000, 0x80000, CRC(7950a147) SHA1(f5bcd5cf6b35f9e4f14d62b084495c3a743d92a1))
	ROM_LOAD("s-wars.u21", 0x080000, 0x40000, CRC(7b08fdf1) SHA1(489d21a10e97e886f948d81dedd7f8de3acecd2b))
ROM_END

ROM_START(stwr_103_a104) // STAR WARS USA CPU 1.03. DISPLAY VERSION- STAR WARS A1.04 11/20/1992
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("starcpua.103", 0x0000, 0x10000, CRC(318085ca) SHA1(7c35bdee52e8093fe05f0624615baabe559a1917))
	ROM_REGION(0x80000, "decodmd2", 0)
	ROM_LOAD("swrom1.a14", 0x00000, 0x40000, CRC(4d577828) SHA1(8b1f302621fe2ee13a067b9c97e3dc33f4519cea))
	ROM_LOAD("swrom0.a14", 0x40000, 0x40000, CRC(104e5a6b) SHA1(b6a9e32f8aec078665faf2ba9ba4f9f51f68cea8))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("s-wars.u7", 0x8000, 0x8000, CRC(cefa19d5) SHA1(7ddf9cc85ab601514305bc46083a07a3d087b286))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("s-wars.u17", 0x000000, 0x80000, CRC(7950a147) SHA1(f5bcd5cf6b35f9e4f14d62b084495c3a743d92a1))
	ROM_LOAD("s-wars.u21", 0x080000, 0x40000, CRC(7b08fdf1) SHA1(489d21a10e97e886f948d81dedd7f8de3acecd2b))
ROM_END

ROM_START(stwr_102)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("starcpua.102", 0x0000, 0x10000, CRC(8b9d90d6) SHA1(2fb7594e6f4aae1dc3a07192546fabd2901acbed))
	ROM_REGION(0x80000, "decodmd2", 0)
	ROM_LOAD("sw4mrom.a15", 0x00000, 0x80000, CRC(00c87952) SHA1(cd2f491f03fcb3e3ceff7ee7f678aa1957a5d14b))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("s-wars.u7", 0x8000, 0x8000, CRC(cefa19d5) SHA1(7ddf9cc85ab601514305bc46083a07a3d087b286))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("s-wars.u17", 0x000000, 0x80000, CRC(7950a147) SHA1(f5bcd5cf6b35f9e4f14d62b084495c3a743d92a1))
	ROM_LOAD("s-wars.u21", 0x080000, 0x40000, CRC(7b08fdf1) SHA1(489d21a10e97e886f948d81dedd7f8de3acecd2b))
ROM_END

ROM_START(stwr_102e)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("starcpue.102", 0x0000, 0x10000, CRC(b441abd3) SHA1(42cab6e16be8e25a68b2db30f53ba516bbb8741d))
	ROM_REGION(0x80000, "decodmd2", 0)
	ROM_LOAD("sw4mrom.a15", 0x00000, 0x80000, CRC(00c87952) SHA1(cd2f491f03fcb3e3ceff7ee7f678aa1957a5d14b))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("s-wars.u7", 0x8000, 0x8000, CRC(cefa19d5) SHA1(7ddf9cc85ab601514305bc46083a07a3d087b286))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("s-wars.u17", 0x000000, 0x80000, CRC(7950a147) SHA1(f5bcd5cf6b35f9e4f14d62b084495c3a743d92a1))
	ROM_LOAD("s-wars.u21", 0x080000, 0x40000, CRC(7b08fdf1) SHA1(489d21a10e97e886f948d81dedd7f8de3acecd2b))
ROM_END

ROM_START(stwr_101)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("starcpu.101", 0x0000, 0x10000, CRC(6efc7b14) SHA1(f669669fbd8733d06b386ea352fdb2041bf98362))
	ROM_REGION(0x80000, "decodmd2", 0)
	ROM_LOAD("stardisp_u14.102", 0x00000, 0x40000, CRC(f8087364) SHA1(4cd66b72cf430018cfb7ac8306b96a8499d41896))
	ROM_LOAD("stardisp_u12.102", 0x40000, 0x40000, CRC(fde126c6) SHA1(0a3eacfd4589ee0f26c4212ba9948dff061f3338))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("s-wars.u7", 0x8000, 0x8000, CRC(cefa19d5) SHA1(7ddf9cc85ab601514305bc46083a07a3d087b286))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("s-wars.u17", 0x000000, 0x80000, CRC(7950a147) SHA1(f5bcd5cf6b35f9e4f14d62b084495c3a743d92a1))
	ROM_LOAD("s-wars.u21", 0x080000, 0x40000, CRC(7b08fdf1) SHA1(489d21a10e97e886f948d81dedd7f8de3acecd2b))
ROM_END

ROM_START(stwr_101g)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("starcpug.101", 0x0000, 0x10000, CRC(c74b4576) SHA1(67db9294cd802be8d62102fe756648f750821960))
	ROM_REGION(0x80000, "decodmd2", 0)
	ROM_LOAD("swdsp_g.102", 0x00000, 0x80000, CRC(afdfbfc4) SHA1(1c3cd90b9cd4f88ee2b556abef863a0ae9a10056))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("s-wars.u7", 0x8000, 0x8000, CRC(cefa19d5) SHA1(7ddf9cc85ab601514305bc46083a07a3d087b286))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("s-wars.u17", 0x000000, 0x80000, CRC(7950a147) SHA1(f5bcd5cf6b35f9e4f14d62b084495c3a743d92a1))
	ROM_LOAD("s-wars.u21", 0x080000, 0x40000, CRC(7b08fdf1) SHA1(489d21a10e97e886f948d81dedd7f8de3acecd2b))
ROM_END

/*-------------------------------------------------------------
/ Tales From the Crypt - CPU Rev 3b /DMD  Type 2 512K Rom - 64K CPU Rom
/------------------------------------------------------------*/
ROM_START(tftc_303)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("tftccpua.303", 0x0000, 0x10000, CRC(e9bec98e) SHA1(02643805d596017c88d9a534b94b2075bb2ab101))
	ROM_REGION(0x80000, "decodmd2", 0)
	ROM_LOAD("tftcdspa.301", 0x00000, 0x80000, CRC(3888d06f) SHA1(3d276df436a76c6e9bed6629114204dacd88245b))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("sndu7.dat", 0x0000, 0x10000, CRC(7963740e) SHA1(fc1f150dcbab8af865a8ea624dfdcc03301f05e6))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("sndu17.dat", 0x000000, 0x80000, CRC(5c5d009a) SHA1(57d0307ea682eca5a57957e4f61fd92bb7f40e17))
	ROM_LOAD("sndu21.dat", 0x080000, 0x80000, CRC(a0ae61f7) SHA1(c7b5766fda64642f77bdc03b2025cd84f29f4495))
ROM_END
ROM_START(tftc_302)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("tftccpua.302", 0x0000, 0x10000, CRC(a194fe0f) SHA1(b83e048300f7e072f76672d72cdf43e43fab2e9e))
	ROM_REGION(0x80000, "decodmd2", 0)
	ROM_LOAD("tftcdspa.301", 0x00000, 0x80000, CRC(3888d06f) SHA1(3d276df436a76c6e9bed6629114204dacd88245b))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("sndu7.dat", 0x0000, 0x10000, CRC(7963740e) SHA1(fc1f150dcbab8af865a8ea624dfdcc03301f05e6))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("sndu17.dat", 0x000000, 0x80000, CRC(5c5d009a) SHA1(57d0307ea682eca5a57957e4f61fd92bb7f40e17))
	ROM_LOAD("sndu21.dat", 0x080000, 0x80000, CRC(a0ae61f7) SHA1(c7b5766fda64642f77bdc03b2025cd84f29f4495))
ROM_END
ROM_START(tftc_300)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("tftccpua.300", 0x0000, 0x10000, CRC(3d275152) SHA1(0aa6df629c27d9265cf35ca0724e241d9820e56b))
	ROM_REGION(0x80000, "decodmd2", 0)
	ROM_LOAD("tftcdspa.300", 0x00000, 0x80000, CRC(bf5c812b) SHA1(c10390b6cad0ad457fb83241c7ee1d6b109cf5be))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("sndu7.dat", 0x0000, 0x10000, CRC(7963740e) SHA1(fc1f150dcbab8af865a8ea624dfdcc03301f05e6))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("sndu17.dat", 0x000000, 0x80000, CRC(5c5d009a) SHA1(57d0307ea682eca5a57957e4f61fd92bb7f40e17))
	ROM_LOAD("sndu21.dat", 0x080000, 0x80000, CRC(a0ae61f7) SHA1(c7b5766fda64642f77bdc03b2025cd84f29f4495))
ROM_END

ROM_START(tftc_200)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("tftcgc5.a20", 0x0000, 0x10000, CRC(94b61f83) SHA1(9f36353a06cacb8ad67f70cd8d9d8ac698905ba3))
	ROM_REGION(0x80000, "decodmd2", 0)
	ROM_LOAD("tftcdot.a20", 0x00000, 0x80000, CRC(16b3968a) SHA1(6ce91774fc60187e4b0d8874a14ef64e2805eb3f))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("sndu7.dat", 0x0000, 0x10000, CRC(7963740e) SHA1(fc1f150dcbab8af865a8ea624dfdcc03301f05e6))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("sndu17.dat", 0x000000, 0x80000, CRC(5c5d009a) SHA1(57d0307ea682eca5a57957e4f61fd92bb7f40e17))
	ROM_LOAD("sndu21.dat", 0x080000, 0x80000, CRC(a0ae61f7) SHA1(c7b5766fda64642f77bdc03b2025cd84f29f4495))
ROM_END

ROM_START(tftc_104s)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("tftccpua.104", 0x0000, 0x10000, CRC(efb3c0d0) SHA1(df1505947732704171e31dbace4c263723c8342b))
	ROM_REGION(0x80000, "decodmd2", 0)
	ROM_LOAD("tftcdspl.103", 0x00000, 0x80000, CRC(98f3b13e) SHA1(909c373b1a27b5aeebad2535ae4fb9bba71e9b5c))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("sndu7.dat", 0x0000, 0x10000, CRC(7963740e) SHA1(fc1f150dcbab8af865a8ea624dfdcc03301f05e6))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("sndu17.dat", 0x000000, 0x80000, CRC(5c5d009a) SHA1(57d0307ea682eca5a57957e4f61fd92bb7f40e17))
	ROM_LOAD("sndu21.dat", 0x080000, 0x80000, CRC(a0ae61f7) SHA1(c7b5766fda64642f77bdc03b2025cd84f29f4495))
ROM_END

/*-----------------------------------------------------------------------------
/ Teenage Mutant Ninja Turtles - CPU Rev 3 /DMD Type 1 64K Rom 16/32K CPU Roms
/-----------------------------------------------------------------------------*/
ROM_START(tmnt_104)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("tmntb5a.104", 0x4000, 0x4000, CRC(f508eeee) SHA1(5e67fde49f6e7d5d563645df9036d5691be076cf))
	ROM_LOAD("tmntc5a.104", 0x8000, 0x8000, CRC(a33d18d4) SHA1(41cf815c1f3d117efe0ddd14ad84076dcb80318a))
	ROM_REGION(0x20000, "decodmd1", 0)
	ROM_LOAD("tmntdsp.104", 0x00000, 0x10000, CRC(545686b7) SHA1(713df7820d024db3406f5e171f62a53e34474f70))
	ROM_RELOAD(0x10000, 0x10000)
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("tmntf7.rom", 0x8000, 0x8000, CRC(59ba0153) SHA1(e7b02a656c67a0d866020a60ee90e30bef77f67f))
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("tmntf6.rom", 0x00000, 0x20000, CRC(5668d45a) SHA1(65766cb47791ec0a2243015d487f1156a2819fe6))
	ROM_LOAD("tmntf4.rom", 0x20000, 0x20000, CRC(6c38cd84) SHA1(bbe8797fe1622cb8f0842c4d7159760fed080880))
ROM_END

ROM_START(tmnt_104g)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("tmntb5a.104", 0x4000, 0x4000, CRC(f508eeee) SHA1(5e67fde49f6e7d5d563645df9036d5691be076cf))
	ROM_LOAD("tmntc5g.104", 0x8000, 0x8000, CRC(d7f2fd8b) SHA1(b80f6201ca2981ec4a3869688963884948a6bd72))
	ROM_REGION(0x20000, "decodmd1", 0)
	ROM_LOAD("tmntdsp.104", 0x00000, 0x10000, CRC(545686b7) SHA1(713df7820d024db3406f5e171f62a53e34474f70))
	ROM_RELOAD(0x10000, 0x10000)
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("tmntf7.rom", 0x8000, 0x8000, CRC(59ba0153) SHA1(e7b02a656c67a0d866020a60ee90e30bef77f67f))
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("tmntf6.rom", 0x00000, 0x20000, CRC(5668d45a) SHA1(65766cb47791ec0a2243015d487f1156a2819fe6))
	ROM_LOAD("tmntf4.rom", 0x20000, 0x20000, CRC(6c38cd84) SHA1(bbe8797fe1622cb8f0842c4d7159760fed080880))
ROM_END

ROM_START(tmnt_103)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("tmntb5.103", 0x4000, 0x4000, CRC(fcc6c5b0) SHA1(062bbc93de0f8bb1921da4d756a13923f23cf5d9))
	ROM_LOAD("tmntc5.103", 0x8000, 0x8000, CRC(46b68ecc) SHA1(cb94041017c0856f1e15de05c70369cb4f8756cd))
	ROM_REGION(0x20000, "decodmd1", 0)
	ROM_LOAD("tmntdsp.104", 0x00000, 0x10000, CRC(545686b7) SHA1(713df7820d024db3406f5e171f62a53e34474f70))
	ROM_RELOAD(0x10000, 0x10000)
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("tmntf7.rom", 0x8000, 0x8000, CRC(59ba0153) SHA1(e7b02a656c67a0d866020a60ee90e30bef77f67f))
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("tmntf6.rom", 0x00000, 0x20000, CRC(5668d45a) SHA1(65766cb47791ec0a2243015d487f1156a2819fe6))
	ROM_LOAD("tmntf4.rom", 0x20000, 0x20000, CRC(6c38cd84) SHA1(bbe8797fe1622cb8f0842c4d7159760fed080880))
ROM_END

ROM_START(tmnt_101)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("tmntb5a.101", 0x4000, 0x4000, CRC(42ae083d) SHA1(0dd62dd7e3cd5db1729cec30e1831093bc63ce6e))
	ROM_LOAD("tmntc5a.101", 0x8000, 0x8000, CRC(24ba0267) SHA1(8e157faa1ade6a1d9f9b6d395d708cfa521597b3))
	ROM_REGION(0x20000, "decodmd1", 0)
	ROM_LOAD("tmntdspa.103", 0x00000, 0x10000, CRC(d52a7d49) SHA1(9249aafe272a052d19f1dd461708e8152516f79f))
	ROM_RELOAD(0x10000, 0x10000)
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("tmntf7.rom", 0x8000, 0x8000, CRC(59ba0153) SHA1(e7b02a656c67a0d866020a60ee90e30bef77f67f))
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("tmntf6.rom", 0x00000, 0x20000, CRC(5668d45a) SHA1(65766cb47791ec0a2243015d487f1156a2819fe6))
	ROM_LOAD("tmntf4.rom", 0x20000, 0x20000, CRC(6c38cd84) SHA1(bbe8797fe1622cb8f0842c4d7159760fed080880))
ROM_END

ROM_START(tmnt_a07)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("tmntb5a.007", 0x4000, 0x4000, CRC(7314f444) SHA1(e18c46009b90d8671a1f15542eb882f391bd57db))
	ROM_LOAD("tmntc5a.007", 0x8000, 0x8000, CRC(f0ec9ac0) SHA1(c7c70c1185dffa725fbba09aa1c5ea52cabbc4a5))
	ROM_REGION(0x20000, "decodmd1", 0)
	ROM_LOAD("tmntdsp16.491", 0x00000, 0x10000, CRC(6ed5744e) SHA1(7d02ed19d3bc479386c58282826a3dc06999307f))
	ROM_RELOAD(0x10000, 0x10000)
	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("tmntf7.rom", 0x8000, 0x8000, CRC(59ba0153) SHA1(e7b02a656c67a0d866020a60ee90e30bef77f67f))
	ROM_REGION(0x40000, "sound1", 0)
	ROM_LOAD("tmntf6.rom", 0x00000, 0x20000, CRC(5668d45a) SHA1(65766cb47791ec0a2243015d487f1156a2819fe6))
	ROM_LOAD("tmntf4.rom", 0x20000, 0x20000, CRC(6c38cd84) SHA1(bbe8797fe1622cb8f0842c4d7159760fed080880))
ROM_END

/*-------------------------------------------------------------
/ The Who's Tommy Pinball Wizard - CPU Rev 3b /DMD  Type 2 512K Rom - 64K CPU Rom
/------------------------------------------------------------*/
ROM_START(tomy_400)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("tomcpua.400", 0x0000, 0x10000, CRC(d0310a1a) SHA1(5b14f5d6e271676b4ec93b64f1cde9607844b677))
	ROM_REGION(0x80000, "decodmd2", 0)
	ROM_LOAD("tommydva.400", 0x00000, 0x80000, CRC(9e640d09) SHA1(d921fadeb728cf929c6bae2e79bd4d140192a4d2))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("tommysnd.u7", 0x0000, 0x10000, CRC(ab0b4626) SHA1(31237b4f5e866710506f1336e3ca2dbd6a89385a))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("tommysnd.u17", 0x000000, 0x80000, CRC(11bb2aa7) SHA1(57b4867c109996861f45ead1ceedb7153aff852e))
	ROM_LOAD("tommysnd.u21", 0x080000, 0x80000, CRC(bb4aeec3) SHA1(2ac6cd25b79584fa6ad2c8a36c3cc58ab8ec0206))
	ROM_LOAD("tommysnd.u36", 0x100000, 0x80000, CRC(208d7aeb) SHA1(af8af2094d1a91c7b4ef8ac6d4f594728e97450f))
	ROM_LOAD("tommysnd.u37", 0x180000, 0x80000, CRC(46180085) SHA1(f761c27532180de313f23b41f02341783be8938b))
ROM_END

ROM_START(tomy_301g)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("tom_3.00_german_cpu_c5.bin", 0x0000, 0x10000, CRC(a24ba9c0) SHA1(fab504372df9231a8078af23acfdef185b0d7b05))
	ROM_REGION(0x80000, "decodmd2", 0)
	ROM_LOAD("tom_3.00_german_display_rom0.bin", 0x00000, 0x80000, CRC(a8a47c4d) SHA1(62a05ede57ab5d4be4c53155788bb7f899198846))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("tommysnd.u7", 0x0000, 0x10000, CRC(ab0b4626) SHA1(31237b4f5e866710506f1336e3ca2dbd6a89385a))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("tommysnd.u17", 0x000000, 0x80000, CRC(11bb2aa7) SHA1(57b4867c109996861f45ead1ceedb7153aff852e))
	ROM_LOAD("tommysnd.u21", 0x080000, 0x80000, CRC(bb4aeec3) SHA1(2ac6cd25b79584fa6ad2c8a36c3cc58ab8ec0206))
	ROM_LOAD("tommysnd.u36", 0x100000, 0x80000, CRC(208d7aeb) SHA1(af8af2094d1a91c7b4ef8ac6d4f594728e97450f))
	ROM_LOAD("tommysnd.u37", 0x180000, 0x80000, CRC(46180085) SHA1(f761c27532180de313f23b41f02341783be8938b))
ROM_END

ROM_START(tomy_300h)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("tomcpuh.300", 0x0000, 0x10000, CRC(121b5932) SHA1(e7d7bf8a78baf1c00c8bac908d4646586b8cf1f5))
	ROM_REGION(0x80000, "decodmd2", 0)
	ROM_LOAD("tommydva.300", 0x00000, 0x80000, CRC(1f2d0896) SHA1(50c617e30bb843c69a6ca8afeeb751c886f5e6bd))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("tommysnd.u7", 0x0000, 0x10000, CRC(ab0b4626) SHA1(31237b4f5e866710506f1336e3ca2dbd6a89385a))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("tommysnd.u17", 0x000000, 0x80000, CRC(11bb2aa7) SHA1(57b4867c109996861f45ead1ceedb7153aff852e))
	ROM_LOAD("tommysnd.u21", 0x080000, 0x80000, CRC(bb4aeec3) SHA1(2ac6cd25b79584fa6ad2c8a36c3cc58ab8ec0206))
	ROM_LOAD("tommysnd.u36", 0x100000, 0x80000, CRC(208d7aeb) SHA1(af8af2094d1a91c7b4ef8ac6d4f594728e97450f))
	ROM_LOAD("tommysnd.u37", 0x180000, 0x80000, CRC(46180085) SHA1(f761c27532180de313f23b41f02341783be8938b))
ROM_END

ROM_START(tomy_201h)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("tommy_2.01_dutch_cpu.bin", 0x0000, 0x10000, CRC(9705af61) SHA1(8a302d2f217d0f10bf100606643fc1780564da67))
	ROM_REGION(0x80000, "decodmd2", 0)
	ROM_LOAD("tommy_2.00_display.bin", 0x00000, 0x80000, CRC(e554e0dc) SHA1(2b3baf20280134e3a40c41e0e0c39578dd905abe))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("tommysnd.u7", 0x0000, 0x10000, CRC(ab0b4626) SHA1(31237b4f5e866710506f1336e3ca2dbd6a89385a))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("tommysnd.u17", 0x000000, 0x80000, CRC(11bb2aa7) SHA1(57b4867c109996861f45ead1ceedb7153aff852e))
	ROM_LOAD("tommysnd.u21", 0x080000, 0x80000, CRC(bb4aeec3) SHA1(2ac6cd25b79584fa6ad2c8a36c3cc58ab8ec0206))
	ROM_LOAD("tommysnd.u36", 0x100000, 0x80000, CRC(208d7aeb) SHA1(af8af2094d1a91c7b4ef8ac6d4f594728e97450f))
	ROM_LOAD("tommysnd.u37", 0x180000, 0x80000, CRC(46180085) SHA1(f761c27532180de313f23b41f02341783be8938b))
ROM_END

ROM_START(tomy_102)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("tomcpua.102", 0x0000, 0x10000, CRC(e470b78e) SHA1(9d358e9d87469cdefb5c373f16c51774bbd390ea))
	ROM_REGION(0x80000, "decodmd2", 0)
	ROM_LOAD("tommydva.300", 0x00000, 0x80000, CRC(1f2d0896) SHA1(50c617e30bb843c69a6ca8afeeb751c886f5e6bd))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("tommysnd.u7", 0x0000, 0x10000, CRC(ab0b4626) SHA1(31237b4f5e866710506f1336e3ca2dbd6a89385a))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("tommysnd.u17", 0x000000, 0x80000, CRC(11bb2aa7) SHA1(57b4867c109996861f45ead1ceedb7153aff852e))
	ROM_LOAD("tommysnd.u21", 0x080000, 0x80000, CRC(bb4aeec3) SHA1(2ac6cd25b79584fa6ad2c8a36c3cc58ab8ec0206))
	ROM_LOAD("tommysnd.u36", 0x100000, 0x80000, CRC(208d7aeb) SHA1(af8af2094d1a91c7b4ef8ac6d4f594728e97450f))
	ROM_LOAD("tommysnd.u37", 0x180000, 0x80000, CRC(46180085) SHA1(f761c27532180de313f23b41f02341783be8938b))
ROM_END

ROM_START(tomy_102be)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("tomcpub.102", 0x0000, 0x10000, CRC(59e4e029) SHA1(55da9eade7a3e08e556799bd27ab4b8347fe87cb))
	ROM_REGION(0x80000, "decodmd2", 0)
	ROM_LOAD("tommydvb.102", 0x00000, 0x80000, CRC(f20b9890) SHA1(66a120ba4f3dfd2195a3c430e678ed47f826a73c))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("tommysnd.u7", 0x0000, 0x10000, CRC(ab0b4626) SHA1(31237b4f5e866710506f1336e3ca2dbd6a89385a))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("tommysnd.u17", 0x000000, 0x80000, CRC(11bb2aa7) SHA1(57b4867c109996861f45ead1ceedb7153aff852e))
	ROM_LOAD("tommysnd.u21", 0x080000, 0x80000, CRC(bb4aeec3) SHA1(2ac6cd25b79584fa6ad2c8a36c3cc58ab8ec0206))
	ROM_LOAD("tommysnd.u36", 0x100000, 0x80000, CRC(208d7aeb) SHA1(af8af2094d1a91c7b4ef8ac6d4f594728e97450f))
	ROM_LOAD("tommysnd.u37", 0x180000, 0x80000, CRC(46180085) SHA1(f761c27532180de313f23b41f02341783be8938b))
ROM_END

/*-------------------------------------------------------------
/ WWF Royal Rumble - CPU Rev 3b /DMD  Type 2 512K Rom - 64K CPU Rom
/------------------------------------------------------------*/
ROM_START(wwfr_106)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("wwfcpua.106", 0x0000, 0x10000, CRC(5f1c7da2) SHA1(9188e0b9c26e4b6c92c63a58b52ee42bd3b77ca0))
	ROM_REGION(0x80000, "decodmd2", 0)
	ROM_LOAD("wwfdispa.102", 0x00000, 0x80000, CRC(4b629a4f) SHA1(c301d0c785f7bc4d3c23cbda76ff955c742eaeef))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("wfsndu7.512", 0x0000, 0x10000, CRC(eb01745c) SHA1(7222e39c52ed298b737aadaa5b57d2068d39287e))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("wfsndu17.400", 0x000000, 0x80000, CRC(7d9c2ca8) SHA1(5d84559455fe7e27634b28bcab81d54f2676390e))
	ROM_LOAD("wfsndu21.400", 0x080000, 0x80000, CRC(242dcdcb) SHA1(af7220e14b0956ef40f75b2749eb1b9d715a1af0))
	ROM_LOAD("wfsndu36.400", 0x100000, 0x80000, CRC(39db8d85) SHA1(a55dd88fd4d9154b523dca9160bf96119af1f94d))
ROM_END

ROM_START(wwfr_103)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("wfcpuc5.512", 0x0000, 0x10000, CRC(7e9ead89) SHA1(6cfd64899128b5f9b4ccc37b7bfdbb0a2a75a3a5))
	ROM_REGION(0x80000, "decodmd2", 0)
	ROM_LOAD("wfdisp0.400", 0x00000, 0x80000, CRC(e190b90f) SHA1(a0e73ce0b241a81e935e6790e04ea5e1fccf3742))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("wfsndu7.512", 0x0000, 0x10000, CRC(eb01745c) SHA1(7222e39c52ed298b737aadaa5b57d2068d39287e))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("wfsndu17.400", 0x000000, 0x80000, CRC(7d9c2ca8) SHA1(5d84559455fe7e27634b28bcab81d54f2676390e))
	ROM_LOAD("wfsndu21.400", 0x080000, 0x80000, CRC(242dcdcb) SHA1(af7220e14b0956ef40f75b2749eb1b9d715a1af0))
	ROM_LOAD("wfsndu36.400", 0x100000, 0x80000, CRC(39db8d85) SHA1(a55dd88fd4d9154b523dca9160bf96119af1f94d))
ROM_END

ROM_START(wwfr_103f)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("wfcpucf.103", 0x0000, 0x10000, CRC(0e211494) SHA1(c601a075636f84ad12ec0693772a8759049077d5))
	ROM_REGION(0x80000, "decodmd2", 0)
	ROM_LOAD("wfdspf.101", 0x00000, 0x80000, CRC(4c39bda9) SHA1(2ea61a2020a4a4e3f23853ab8780d6999053e8ae))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("wfsndu7.512", 0x0000, 0x10000, CRC(eb01745c) SHA1(7222e39c52ed298b737aadaa5b57d2068d39287e))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("wfsndu17.400", 0x000000, 0x80000, CRC(7d9c2ca8) SHA1(5d84559455fe7e27634b28bcab81d54f2676390e))
	ROM_LOAD("wfsndu21.400", 0x080000, 0x80000, CRC(242dcdcb) SHA1(af7220e14b0956ef40f75b2749eb1b9d715a1af0))
	ROM_LOAD("wfsndu36.400", 0x100000, 0x80000, CRC(39db8d85) SHA1(a55dd88fd4d9154b523dca9160bf96119af1f94d))
ROM_END

/*-------------------------------------------------------------
/ Batman Forever 4.0
/------------------------------------------------------------*/
ROM_START(batmanf)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("batnova.401", 0x0000, 0x10000, CRC(4e62df4e) SHA1(6c3be65fc8825f47cd08755b58fdcf3652ede702))
	ROM_REGION16_BE(0x100000, "decodmd3", 0)
	ROM_LOAD16_BYTE("bfdrom0a.401", 0x000001, 0x80000, CRC(8a3c20ad) SHA1(37415ac7ba178981dffce3a17502f39ab29d90ea))
	ROM_LOAD16_BYTE("bfdrom3a.401", 0x000000, 0x80000, CRC(5ef46847) SHA1(a80f241db3d309f0bcb455051e33fc2b74e2ddcd))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("bmfu7.bin", 0x0000, 0x10000, CRC(58c0d144) SHA1(88a404d3625c7c154892282598b4949ac97de12b))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("bmfu17.bin", 0x000000, 0x80000, CRC(edcd5c10) SHA1(561f22fb7817f64e09ef6adda646f58f31b80bf4))
	ROM_LOAD("bmfu21.bin", 0x080000, 0x80000, CRC(e41a516d) SHA1(9c41803a01046e57f8bd8759fe5e62ad6abaa80c))
ROM_END

ROM_START(batmanf3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("batcpua.302", 0x0000, 0x10000, CRC(5ae7ce69) SHA1(13409c7c993bd9940f3a72f3bac8c8c57a665b3f))
	ROM_REGION16_BE(0x100000, "decodmd3", 0)
	ROM_LOAD16_BYTE("bmfrom0a.300", 0x000001, 0x80000, CRC(764bb217) SHA1(2923d2d2924faa4bdc6e67087fb8ce694d27809a))
	ROM_LOAD16_BYTE("bmfrom3a.300", 0x000000, 0x80000, CRC(b4e3b515) SHA1(0f8bf08bc480eed575da54bfc0135f38a86302d4))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("bmfu7.bin", 0x0000, 0x10000, CRC(58c0d144) SHA1(88a404d3625c7c154892282598b4949ac97de12b))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("bmfu17.bin", 0x000000, 0x80000, CRC(edcd5c10) SHA1(561f22fb7817f64e09ef6adda646f58f31b80bf4))
	ROM_LOAD("bmfu21.bin", 0x080000, 0x80000, CRC(e41a516d) SHA1(9c41803a01046e57f8bd8759fe5e62ad6abaa80c))
ROM_END

ROM_START(batmanf2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("batcpua.202", 0x0000, 0x10000, CRC(3e2fe40b) SHA1(afacbbc8af319110149b25c35ef03dcf019ca8da))
	ROM_REGION16_BE(0x100000, "decodmd3", 0)
	ROM_LOAD16_BYTE("bmfrom0.200", 0x000001, 0x80000, CRC(17086824) SHA1(37f2d463d7cc15739fb18000c81dbc1e79c1549a))
	ROM_LOAD16_BYTE("bmfrom3.200", 0x000000, 0x80000, CRC(9c8a9a8f) SHA1(8dce048cac657da66478ae0b6bd000a2648a118a))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("bmfu7.bin", 0x0000, 0x10000, CRC(58c0d144) SHA1(88a404d3625c7c154892282598b4949ac97de12b))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("bmfu17.bin", 0x000000, 0x80000, CRC(edcd5c10) SHA1(561f22fb7817f64e09ef6adda646f58f31b80bf4))
	ROM_LOAD("bmfu21.bin", 0x080000, 0x80000, CRC(e41a516d) SHA1(9c41803a01046e57f8bd8759fe5e62ad6abaa80c))
ROM_END

ROM_START(batmanf1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("batcpua.102", 0x0000, 0x10000, CRC(aafba427) SHA1(485fa3b76569a8c9ed640e9fa8fd714fdd2268b8))
	ROM_REGION16_BE(0x100000, "decodmd3", 0)
	ROM_LOAD16_BYTE("bmfrom0.100", 0x000001, 0x80000, CRC(4d65a45c) SHA1(b4a112f8a70ad887e1a23291bcec1d55bd7277c1))
	ROM_LOAD16_BYTE("bmfrom3.100", 0x000000, 0x80000, CRC(b4b774d1) SHA1(5dacfb5cedc597dbb2d72e83de4979eb19b19d72))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("bmfu7.bin", 0x0000, 0x10000, CRC(58c0d144) SHA1(88a404d3625c7c154892282598b4949ac97de12b))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("bmfu17.bin", 0x000000, 0x80000, CRC(edcd5c10) SHA1(561f22fb7817f64e09ef6adda646f58f31b80bf4))
	ROM_LOAD("bmfu21.bin", 0x080000, 0x80000, CRC(e41a516d) SHA1(9c41803a01046e57f8bd8759fe5e62ad6abaa80c))
ROM_END

ROM_START(bmf_uk)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("batnove.401", 0x0000, 0x10000, CRC(80f6e4af) SHA1(dd233d2150dcb50b74a70e6ff89c74a3f0d8fae1))
	ROM_REGION16_BE(0x100000, "decodmd3", 0)
	ROM_LOAD16_BYTE("bfdrom0a.401", 0x000001, 0x80000, CRC(8a3c20ad) SHA1(37415ac7ba178981dffce3a17502f39ab29d90ea))
	ROM_LOAD16_BYTE("bfdrom3a.401", 0x000000, 0x80000, CRC(5ef46847) SHA1(a80f241db3d309f0bcb455051e33fc2b74e2ddcd))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("bmfu7.bin", 0x0000, 0x10000, CRC(58c0d144) SHA1(88a404d3625c7c154892282598b4949ac97de12b))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("bmfu17.bin", 0x000000, 0x80000, CRC(edcd5c10) SHA1(561f22fb7817f64e09ef6adda646f58f31b80bf4))
	ROM_LOAD("bmfu21.bin", 0x080000, 0x80000, CRC(e41a516d) SHA1(9c41803a01046e57f8bd8759fe5e62ad6abaa80c))
ROM_END

ROM_START(bmf_cn)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("batnovc.401", 0x0000, 0x10000, CRC(99936537) SHA1(08ff9c6a1fcb3f198190d24bbc75ea1178427fda))
	ROM_REGION16_BE(0x100000, "decodmd3", 0)
	ROM_LOAD16_BYTE("bfdrom0a.401", 0x000001, 0x80000, CRC(8a3c20ad) SHA1(37415ac7ba178981dffce3a17502f39ab29d90ea))
	ROM_LOAD16_BYTE("bfdrom3a.401", 0x000000, 0x80000, CRC(5ef46847) SHA1(a80f241db3d309f0bcb455051e33fc2b74e2ddcd))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("bmfu7.bin", 0x0000, 0x10000, CRC(58c0d144) SHA1(88a404d3625c7c154892282598b4949ac97de12b))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("bmfu17.bin", 0x000000, 0x80000, CRC(edcd5c10) SHA1(561f22fb7817f64e09ef6adda646f58f31b80bf4))
	ROM_LOAD("bmfu21.bin", 0x080000, 0x80000, CRC(e41a516d) SHA1(9c41803a01046e57f8bd8759fe5e62ad6abaa80c))
ROM_END

ROM_START(bmf_no)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("batnovn.401", 0x0000, 0x10000, CRC(79dd48b4) SHA1(eefdf423f9638e293e51bd31413de898ec4eb83a))
	ROM_REGION16_BE(0x100000, "decodmd3", 0)
	ROM_LOAD16_BYTE("bfdrom0a.401", 0x000001, 0x80000, CRC(8a3c20ad) SHA1(37415ac7ba178981dffce3a17502f39ab29d90ea))
	ROM_LOAD16_BYTE("bfdrom3a.401", 0x000000, 0x80000, CRC(5ef46847) SHA1(a80f241db3d309f0bcb455051e33fc2b74e2ddcd))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("bmfu7.bin", 0x0000, 0x10000, CRC(58c0d144) SHA1(88a404d3625c7c154892282598b4949ac97de12b))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("bmfu17.bin", 0x000000, 0x80000, CRC(edcd5c10) SHA1(561f22fb7817f64e09ef6adda646f58f31b80bf4))
	ROM_LOAD("bmfu21.bin", 0x080000, 0x80000, CRC(e41a516d) SHA1(9c41803a01046e57f8bd8759fe5e62ad6abaa80c))
ROM_END

ROM_START(bmf_sv)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("batnovt.401", 0x0000, 0x10000, CRC(854029ab) SHA1(044c2fff6f3e8995c48344f727c1cd9079f7e232))
	ROM_REGION16_BE(0x100000, "decodmd3", 0)
	ROM_LOAD16_BYTE("bfdrom0a.401", 0x000001, 0x80000, CRC(8a3c20ad) SHA1(37415ac7ba178981dffce3a17502f39ab29d90ea))
	ROM_LOAD16_BYTE("bfdrom3a.401", 0x000000, 0x80000, CRC(5ef46847) SHA1(a80f241db3d309f0bcb455051e33fc2b74e2ddcd))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("bmfu7.bin", 0x0000, 0x10000, CRC(58c0d144) SHA1(88a404d3625c7c154892282598b4949ac97de12b))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("bmfu17.bin", 0x000000, 0x80000, CRC(edcd5c10) SHA1(561f22fb7817f64e09ef6adda646f58f31b80bf4))
	ROM_LOAD("bmfu21.bin", 0x080000, 0x80000, CRC(e41a516d) SHA1(9c41803a01046e57f8bd8759fe5e62ad6abaa80c))
ROM_END

ROM_START(bmf_at)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("batnovh.401", 0x0000, 0x10000, CRC(acba13d7) SHA1(b5e5dc5ffc926612ea3d592b6d4e8e02f6290bc7))
	ROM_REGION16_BE(0x100000, "decodmd3", 0)
	ROM_LOAD16_BYTE("bfdrom0g.401", 0x000001, 0x80000, CRC(3a2d7d53) SHA1(340107290d58bfb8b9a6613215eb556626fe2461))
	ROM_LOAD16_BYTE("bfdrom3g.401", 0x000000, 0x80000, CRC(94e424f1) SHA1(3a6daf9cbd38e21e2c6447ff1fb0e86b4c03f971))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("bmfu7.bin", 0x0000, 0x10000, CRC(58c0d144) SHA1(88a404d3625c7c154892282598b4949ac97de12b))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("bmfu17.bin", 0x000000, 0x80000, CRC(edcd5c10) SHA1(561f22fb7817f64e09ef6adda646f58f31b80bf4))
	ROM_LOAD("bmfu21.bin", 0x080000, 0x80000, CRC(e41a516d) SHA1(9c41803a01046e57f8bd8759fe5e62ad6abaa80c))
ROM_END

ROM_START(bmf_ch)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("batnovs.401", 0x0000, 0x10000, CRC(4999d5f9) SHA1(61a9220da38e05360a9496504fa7b11aff14515d))
	ROM_REGION16_BE(0x100000, "decodmd3", 0)
	ROM_LOAD16_BYTE("bfdrom0g.401", 0x000001, 0x80000, CRC(3a2d7d53) SHA1(340107290d58bfb8b9a6613215eb556626fe2461))
	ROM_LOAD16_BYTE("bfdrom3g.401", 0x000000, 0x80000, CRC(94e424f1) SHA1(3a6daf9cbd38e21e2c6447ff1fb0e86b4c03f971))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("bmfu7.bin", 0x0000, 0x10000, CRC(58c0d144) SHA1(88a404d3625c7c154892282598b4949ac97de12b))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("bmfu17.bin", 0x000000, 0x80000, CRC(edcd5c10) SHA1(561f22fb7817f64e09ef6adda646f58f31b80bf4))
	ROM_LOAD("bmfu21.bin", 0x080000, 0x80000, CRC(e41a516d) SHA1(9c41803a01046e57f8bd8759fe5e62ad6abaa80c))
ROM_END

ROM_START(bmf_de)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("batnovg.401", 0x0000, 0x10000, CRC(dd37e99a) SHA1(7949ed43df38849d927f6ed0afa8c3f77cd74b6a))
	ROM_REGION16_BE(0x100000, "decodmd3", 0)
	ROM_LOAD16_BYTE("bfdrom0g.401", 0x000001, 0x80000, CRC(3a2d7d53) SHA1(340107290d58bfb8b9a6613215eb556626fe2461))
	ROM_LOAD16_BYTE("bfdrom3g.401", 0x000000, 0x80000, CRC(94e424f1) SHA1(3a6daf9cbd38e21e2c6447ff1fb0e86b4c03f971))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("bmfu7.bin", 0x0000, 0x10000, CRC(58c0d144) SHA1(88a404d3625c7c154892282598b4949ac97de12b))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("bmfu17.bin", 0x000000, 0x80000, CRC(edcd5c10) SHA1(561f22fb7817f64e09ef6adda646f58f31b80bf4))
	ROM_LOAD("bmfu21.bin", 0x080000, 0x80000, CRC(e41a516d) SHA1(9c41803a01046e57f8bd8759fe5e62ad6abaa80c))
ROM_END

ROM_START(bmf_be)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("batnovb.401", 0x0000, 0x10000, CRC(21309873) SHA1(cebd0c5c05dc5c0a2eb8563ad5c4759f78d6a4b9))
	ROM_REGION16_BE(0x100000, "decodmd3", 0)
	ROM_LOAD16_BYTE("bfdrom0f.401", 0x000001, 0x80000, CRC(e7473f6f) SHA1(f5951a9b6a8776073adf10e38b9d68d6d444240a))
	ROM_LOAD16_BYTE("bfdrom3f.401", 0x000000, 0x80000, CRC(f7951709) SHA1(ace5b374d1e382d6f612b2bafc0e9fdde9e21014))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("bmfu7.bin", 0x0000, 0x10000, CRC(58c0d144) SHA1(88a404d3625c7c154892282598b4949ac97de12b))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("bmfu17.bin", 0x000000, 0x80000, CRC(edcd5c10) SHA1(561f22fb7817f64e09ef6adda646f58f31b80bf4))
	ROM_LOAD("bmfu21.bin", 0x080000, 0x80000, CRC(e41a516d) SHA1(9c41803a01046e57f8bd8759fe5e62ad6abaa80c))
ROM_END

ROM_START(bmf_fr)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("batnovf.401", 0x0000, 0x10000, CRC(4baa793d) SHA1(4ba258d11f1bd7a2078ae6cd823a11e10ca96627))
	ROM_REGION16_BE(0x100000, "decodmd3", 0)
	ROM_LOAD16_BYTE("bfdrom0f.401", 0x000001, 0x80000, CRC(e7473f6f) SHA1(f5951a9b6a8776073adf10e38b9d68d6d444240a))
	ROM_LOAD16_BYTE("bfdrom3f.401", 0x000000, 0x80000, CRC(f7951709) SHA1(ace5b374d1e382d6f612b2bafc0e9fdde9e21014))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("bmfu7.bin", 0x0000, 0x10000, CRC(58c0d144) SHA1(88a404d3625c7c154892282598b4949ac97de12b))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("bmfu17.bin", 0x000000, 0x80000, CRC(edcd5c10) SHA1(561f22fb7817f64e09ef6adda646f58f31b80bf4))
	ROM_LOAD("bmfu21.bin", 0x080000, 0x80000, CRC(e41a516d) SHA1(9c41803a01046e57f8bd8759fe5e62ad6abaa80c))
ROM_END

ROM_START(bmf_nl)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("batnovd.401", 0x0000, 0x10000, CRC(6ae4570c) SHA1(e863d6d0963910a993f2a0b8ddeefba48d304ca6))
	ROM_REGION16_BE(0x100000, "decodmd3", 0)
	ROM_LOAD16_BYTE("bfdrom0f.401", 0x000001, 0x80000, CRC(e7473f6f) SHA1(f5951a9b6a8776073adf10e38b9d68d6d444240a))
	ROM_LOAD16_BYTE("bfdrom3f.401", 0x000000, 0x80000, CRC(f7951709) SHA1(ace5b374d1e382d6f612b2bafc0e9fdde9e21014))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("bmfu7.bin", 0x0000, 0x10000, CRC(58c0d144) SHA1(88a404d3625c7c154892282598b4949ac97de12b))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("bmfu17.bin", 0x000000, 0x80000, CRC(edcd5c10) SHA1(561f22fb7817f64e09ef6adda646f58f31b80bf4))
	ROM_LOAD("bmfu21.bin", 0x080000, 0x80000, CRC(e41a516d) SHA1(9c41803a01046e57f8bd8759fe5e62ad6abaa80c))
ROM_END

ROM_START(bmf_nl302)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("batnovd.302", 0x0000, 0x10000, CRC(844c7f6a) SHA1(8c035848644329d121780081a12d16721454cb8c))
	ROM_REGION16_BE(0x100000, "decodmd3", 0)
	ROM_LOAD16_BYTE("bfdrom0f.401", 0x000001, 0x80000, CRC(e7473f6f) SHA1(f5951a9b6a8776073adf10e38b9d68d6d444240a))
	ROM_LOAD16_BYTE("bfdrom3f.401", 0x000000, 0x80000, CRC(f7951709) SHA1(ace5b374d1e382d6f612b2bafc0e9fdde9e21014))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("bmfu7.bin", 0x0000, 0x10000, CRC(58c0d144) SHA1(88a404d3625c7c154892282598b4949ac97de12b))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("bmfu17.bin", 0x000000, 0x80000, CRC(edcd5c10) SHA1(561f22fb7817f64e09ef6adda646f58f31b80bf4))
	ROM_LOAD("bmfu21.bin", 0x080000, 0x80000, CRC(e41a516d) SHA1(9c41803a01046e57f8bd8759fe5e62ad6abaa80c))
ROM_END

ROM_START(bmf_it)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("batnovi.401", 0x0000, 0x10000, CRC(7053ef9e) SHA1(918ab3e250b5965998ca0a38e1b8ba3cc012083f))
	ROM_REGION16_BE(0x100000, "decodmd3", 0)
	ROM_LOAD16_BYTE("bfdrom0i.401", 0x000001, 0x80000, CRC(23051253) SHA1(155669a3fecd6e67838b10e71a57a6b871c8762a))
	ROM_LOAD16_BYTE("bfdrom3i.401", 0x000000, 0x80000, CRC(82b61a41) SHA1(818c8fdbf44e29fe0ec5362a34ac948e98002efa))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("bmfu7.bin", 0x0000, 0x10000, CRC(58c0d144) SHA1(88a404d3625c7c154892282598b4949ac97de12b))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("bmfu17.bin", 0x000000, 0x80000, CRC(edcd5c10) SHA1(561f22fb7817f64e09ef6adda646f58f31b80bf4))
	ROM_LOAD("bmfu21.bin", 0x080000, 0x80000, CRC(e41a516d) SHA1(9c41803a01046e57f8bd8759fe5e62ad6abaa80c))
ROM_END

ROM_START(bmf_ita)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("bmcpu_italy.c5", 0x0000, 0x10000, CRC(adab3152) SHA1(d99124f4199988e50933f8fb9de2677e5327e139))
	ROM_REGION16_BE(0x100000, "decodmd3", 0)
	ROM_LOAD16_BYTE("bmf_dmd0i.bin", 0x000001, 0x80000, CRC(78e15b14) SHA1(67fba0efcd94e2210ea3532e6a3a8f4a6d5891b0))
	ROM_LOAD16_BYTE("bmf_dmd3i.bin", 0x000000, 0x80000, CRC(759f676b) SHA1(622c105fa30d44b244ef9eeecfdd6f753031fc4d))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("bmfu7.bin", 0x0000, 0x10000, CRC(58c0d144) SHA1(88a404d3625c7c154892282598b4949ac97de12b))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("bmfu17.bin", 0x000000, 0x80000, CRC(edcd5c10) SHA1(561f22fb7817f64e09ef6adda646f58f31b80bf4))
	ROM_LOAD("bmfu21.bin", 0x080000, 0x80000, CRC(e41a516d) SHA1(9c41803a01046e57f8bd8759fe5e62ad6abaa80c))
ROM_END

ROM_START(bmf_itb)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("bmcpu_italy.c5", 0x0000, 0x10000, CRC(adab3152) SHA1(d99124f4199988e50933f8fb9de2677e5327e139))
	ROM_REGION16_BE(0x100000, "decodmd3", 0)
	ROM_LOAD16_BYTE("bmf_dmd0i.bin", 0x000001, 0x80000, CRC(78e15b14) SHA1(67fba0efcd94e2210ea3532e6a3a8f4a6d5891b0))
	ROM_LOAD16_BYTE("bmf_dmd3i.bin", 0x000000, 0x80000, CRC(759f676b) SHA1(622c105fa30d44b244ef9eeecfdd6f753031fc4d))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("batforesndu7_92f0_7-16-95.bin", 0x0000, 0x10000, CRC(9d8b2477) SHA1(9e5087e3c84b440e26bb7a6952e6f0c9db734de3))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("batforesndu17_1587_6-14-95.u17", 0x000000, 0x80000, CRC(edcd5c10) SHA1(561f22fb7817f64e09ef6adda646f58f31b80bf4))
	ROM_LOAD("batforesndu21_6fa1_6-14-95.u21", 0x080000, 0x80000, CRC(e41a516d) SHA1(9c41803a01046e57f8bd8759fe5e62ad6abaa80c))
ROM_END

ROM_START(bmf_sp)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("batnova.401", 0x0000, 0x10000, CRC(4e62df4e) SHA1(6c3be65fc8825f47cd08755b58fdcf3652ede702))
	ROM_REGION16_BE(0x100000, "decodmd3", 0)
	ROM_LOAD16_BYTE("bfdrom0l.401", 0x000001, 0x80000, CRC(b22b10d9) SHA1(c8f5637b00b0701d47a3b6bc0fdae08ae1a8df64))
	ROM_LOAD16_BYTE("bfdrom3l.401", 0x000000, 0x80000, CRC(016b8666) SHA1(c10b7fc2c1e5b8382ff5b021a6b70f3a550b190e))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("bmfu7.bin", 0x0000, 0x10000, CRC(58c0d144) SHA1(88a404d3625c7c154892282598b4949ac97de12b))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("bmfu17.bin", 0x000000, 0x80000, CRC(edcd5c10) SHA1(561f22fb7817f64e09ef6adda646f58f31b80bf4))
	ROM_LOAD("bmfu21.bin", 0x080000, 0x80000, CRC(e41a516d) SHA1(9c41803a01046e57f8bd8759fe5e62ad6abaa80c))
ROM_END

ROM_START(bmf_jp)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("batnovj.401", 0x0000, 0x10000, CRC(eef9bef0) SHA1(ac37ae12673351be939a969ecbc5b68c3995dca0))
	ROM_REGION16_BE(0x100000, "decodmd3", 0)
	ROM_LOAD16_BYTE("bfdrom0a.401", 0x000001, 0x80000, CRC(8a3c20ad) SHA1(37415ac7ba178981dffce3a17502f39ab29d90ea))
	ROM_LOAD16_BYTE("bfdrom3a.401", 0x000000, 0x80000, CRC(5ef46847) SHA1(a80f241db3d309f0bcb455051e33fc2b74e2ddcd))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("bmfu7.bin", 0x0000, 0x10000, CRC(58c0d144) SHA1(88a404d3625c7c154892282598b4949ac97de12b))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("bmfu17.bin", 0x000000, 0x80000, CRC(edcd5c10) SHA1(561f22fb7817f64e09ef6adda646f58f31b80bf4))
	ROM_LOAD("bmfu21.bin", 0x080000, 0x80000, CRC(e41a516d) SHA1(9c41803a01046e57f8bd8759fe5e62ad6abaa80c))
ROM_END

ROM_START(bmf_time)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("batnova.401", 0x0000, 0x10000, CRC(4e62df4e) SHA1(6c3be65fc8825f47cd08755b58fdcf3652ede702))
	ROM_REGION16_BE(0x100000, "decodmd3", 0)
	ROM_LOAD16_BYTE("bfdrom0t.401", 0x000001, 0x80000, CRC(b83b8d28) SHA1(b90e6a6fa55dadbf0e752745b87d1e8e9d7ccfa7))
	ROM_LOAD16_BYTE("bfdrom3t.401", 0x000000, 0x80000, CRC(a024b1a5) SHA1(2fc8697fa98b7de7a844ca4d6a162b96cc751447))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("bmfu7.bin", 0x0000, 0x10000, CRC(58c0d144) SHA1(88a404d3625c7c154892282598b4949ac97de12b))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("bmfu17.bin", 0x000000, 0x80000, CRC(edcd5c10) SHA1(561f22fb7817f64e09ef6adda646f58f31b80bf4))
	ROM_LOAD("bmfu21.bin", 0x080000, 0x80000, CRC(e41a516d) SHA1(9c41803a01046e57f8bd8759fe5e62ad6abaa80c))
ROM_END

/*-------------------------------------------------------------
/ Baywatch - CPU Rev 3b /DMD  Type 3 2x512K Rom - 64K CPU Rom
/------------------------------------------------------------*/
ROM_START(baywatch)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("baycpua.400", 0x0000, 0x10000, CRC(89facfda) SHA1(71720b1da227752b0e276390abd08c742bca9090))
	ROM_REGION16_BE(0x100000, "decodmd3", 0)
	ROM_LOAD16_BYTE("bayrom0a.400", 0x000001, 0x80000, CRC(43d615c6) SHA1(7c843b6d5215305b02a55c9fa1d62375ef0766ea))
	ROM_LOAD16_BYTE("bayrom3a.400", 0x000000, 0x80000, CRC(41bcb66b) SHA1(e6f0a9236e14c2e919881ca1ffe3356aaa121730))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("bayw.u7", 0x0000, 0x10000, CRC(90d6d8a8) SHA1(482c5643453f21a078257aa13398845ef19cab3c))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("bayw.u17", 0x000000, 0x80000, CRC(b20fde56) SHA1(2f2db49245e4a6a8251cbe896b2437fcec88d42d))
	ROM_LOAD("bayw.u21", 0x080000, 0x80000, CRC(b7598881) SHA1(19d1dde1cb6634a7c7b5cdb4fa01cd09cc7d7777))
ROM_END

ROM_START(bay_d400)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("baycpud.400", 0x0000, 0x10000, CRC(45019616) SHA1(5a1e04cdfa00f179f010c09fae52d090553cd82e))
	ROM_REGION16_BE(0x100000, "decodmd3", 0)
	ROM_LOAD16_BYTE("bayrom0a.400", 0x000001, 0x80000, CRC(43d615c6) SHA1(7c843b6d5215305b02a55c9fa1d62375ef0766ea))
	ROM_LOAD16_BYTE("bayrom3a.400", 0x000000, 0x80000, CRC(41bcb66b) SHA1(e6f0a9236e14c2e919881ca1ffe3356aaa121730))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("bayw.u7", 0x0000, 0x10000, CRC(90d6d8a8) SHA1(482c5643453f21a078257aa13398845ef19cab3c))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("bayw.u17", 0x000000, 0x80000, CRC(b20fde56) SHA1(2f2db49245e4a6a8251cbe896b2437fcec88d42d))
	ROM_LOAD("bayw.u21", 0x080000, 0x80000, CRC(b7598881) SHA1(19d1dde1cb6634a7c7b5cdb4fa01cd09cc7d7777))
ROM_END

ROM_START(bay_e400)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("baycpue.400", 0x0000, 0x10000, CRC(07b77fe2) SHA1(4f81a5b3d821907e06d6b547117ad39c238a900c))
	ROM_REGION16_BE(0x100000, "decodmd3", 0)
	ROM_LOAD16_BYTE("bayrom0a.400", 0x000001, 0x80000, CRC(43d615c6) SHA1(7c843b6d5215305b02a55c9fa1d62375ef0766ea))
	ROM_LOAD16_BYTE("bayrom3a.400", 0x000000, 0x80000, CRC(41bcb66b) SHA1(e6f0a9236e14c2e919881ca1ffe3356aaa121730))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("bw-u7.u7", 0x0000, 0x10000, CRC(a5e57557) SHA1(a884c1118331b8724507b0a916127ce5df309fe4))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("bw-u17.bin", 0x000000, 0x80000, CRC(660e7f5d) SHA1(6dde294e728e596a6c455326793b65254139620e))
	ROM_LOAD("bw-u21.bin", 0x080000, 0x80000, CRC(5ec3a889) SHA1(f355f742de137344e6e4b5d3a4b2380a876c8cc3))
	ROM_LOAD("bw-u36.bin", 0x100000, 0x80000, CRC(1877abc5) SHA1(13ca231a486495a83cc1d9c6dde558a57eb4abe1))
ROM_END

ROM_START(bay_d300)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("baycpud.300", 0x0000, 0x10000, CRC(c160f045) SHA1(d1f75d5ba292b25278539b01e0f4908276d34e34))
	ROM_REGION16_BE(0x100000, "decodmd3", 0)
	ROM_LOAD16_BYTE("bayrom0a.300", 0x000001, 0x80000, CRC(3f195829) SHA1(a10a1b7f125f239b0eff87ee6667c8250b7ffc87))
	ROM_LOAD16_BYTE("bayrom3a.300", 0x000000, 0x80000, CRC(ae3d8585) SHA1(28b38ebc2755ffb3859f8091a9bf50d868794a3e))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("bayw.u7", 0x0000, 0x10000, CRC(90d6d8a8) SHA1(482c5643453f21a078257aa13398845ef19cab3c))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("bayw.u17", 0x000000, 0x80000, CRC(b20fde56) SHA1(2f2db49245e4a6a8251cbe896b2437fcec88d42d))
	ROM_LOAD("bayw.u21", 0x080000, 0x80000, CRC(b7598881) SHA1(19d1dde1cb6634a7c7b5cdb4fa01cd09cc7d7777))
ROM_END

ROM_START(bay_g300)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("baywatch_cpug_c5_3.00.bin", 0x0000, 0x10000, CRC(8f85fd10) SHA1(1fd561bf6145caf15846a776b0b92e7953fdf3ff))
	ROM_REGION16_BE(0x100000, "decodmd3", 0)
	ROM_LOAD16_BYTE("baywatch_dispg_rom0_3.00.bin", 0x000001, 0x80000, CRC(30b0a96a) SHA1(ff0e4d4a1726ff2a553ebe4f21c0534e0a06b960))
	ROM_LOAD16_BYTE("baywatch_dispg_rom3_3.00.bin", 0x000000, 0x80000, CRC(c7e50fad) SHA1(95b60855cb6718eb9c8a7231e9cd2e1c326cd1e3))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("bayw.u7", 0x0000, 0x10000, CRC(90d6d8a8) SHA1(482c5643453f21a078257aa13398845ef19cab3c))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("bayw.u17", 0x000000, 0x80000, CRC(b20fde56) SHA1(2f2db49245e4a6a8251cbe896b2437fcec88d42d))
	ROM_LOAD("bayw.u21", 0x080000, 0x80000, CRC(b7598881) SHA1(19d1dde1cb6634a7c7b5cdb4fa01cd09cc7d7777))
ROM_END

ROM_START(bay_f201)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("baycpuf.201", 0x0000, 0x10000, CRC(d2fddeaa) SHA1(839baca46823dc72a7ef1421764815f69f0e7084))
	ROM_REGION16_BE(0x100000, "decodmd3", 0)
	ROM_LOAD16_BYTE("bayrom0f.200", 0x000001, 0x80000, CRC(6dc898b6) SHA1(087b043acf64b2a16c8e4c879b90dbea1d79c614))
	ROM_LOAD16_BYTE("bayrom3f.200", 0x000000, 0x80000, CRC(9db1b94e) SHA1(056c1a0fd1c99c1c9426f2e2cdd68f4bbaa89d81))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("bayw.u7", 0x0000, 0x10000, CRC(90d6d8a8) SHA1(482c5643453f21a078257aa13398845ef19cab3c))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("bayw.u17", 0x000000, 0x80000, CRC(b20fde56) SHA1(2f2db49245e4a6a8251cbe896b2437fcec88d42d))
	ROM_LOAD("bayw.u21", 0x080000, 0x80000, CRC(b7598881) SHA1(19d1dde1cb6634a7c7b5cdb4fa01cd09cc7d7777))
ROM_END

/*-------------------------------------------------------------
/ Mary Shelley's Frankenstein - CPU Rev 3b /DMD  Type 3 2x512K Rom - 64K CPU Rom
/------------------------------------------------------------*/
ROM_START(frankst)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("franka.103", 0x0000, 0x10000, CRC(a9aba9be) SHA1(1cc22fcbc0f51a17037637c04e606579956c9cba))
	ROM_REGION16_BE(0x100000, "decodmd3", 0)
	ROM_LOAD16_BYTE("frdspr0a.103", 0x000001, 0x80000, CRC(9dd09c7d) SHA1(c5668e53d6c914667a59538f82222ec2efc6f187))
	ROM_LOAD16_BYTE("frdspr3a.103", 0x000000, 0x80000, CRC(73b538bb) SHA1(07d7ae21f062d15711d72af03bfcd52608f75a5f))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("frsnd.u7", 0x0000, 0x10000, CRC(084f856c) SHA1(c91331a32b565c2ed3f96156f44143dc22009e8e))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("frsnd.u17", 0x000000, 0x80000, CRC(0da904d6) SHA1(e190f1a35147b2f39224832969ca7b1d4a30f6cc))
	ROM_LOAD("frsnd.u21", 0x080000, 0x80000, CRC(14d4bc12) SHA1(9e7005c5bd0afe7f9c9215b39878496640cdea77))
	ROM_LOAD("frsnd.u36", 0x100000, 0x80000, CRC(9964d721) SHA1(5ea0bc051d1909bee80d3feb6b7350b6307b6dcb))
ROM_END

ROM_START(frankstg)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("franka.103", 0x0000, 0x10000, CRC(a9aba9be) SHA1(1cc22fcbc0f51a17037637c04e606579956c9cba))
	ROM_REGION16_BE(0x100000, "decodmd3", 0)
	ROM_LOAD16_BYTE("frdspr0g.101", 0x000001, 0x80000, CRC(5e27ec02) SHA1(351d6f1b7d72e415f2bf5780b6533dbd67579261))
	ROM_LOAD16_BYTE("frdspr3g.101", 0x000000, 0x80000, CRC(d6c607b5) SHA1(876d4bd2a5b89f1a28ff7cd45494c7245f147d27))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("frsnd.u7", 0x0000, 0x10000, CRC(084f856c) SHA1(c91331a32b565c2ed3f96156f44143dc22009e8e))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("frsnd.u17", 0x000000, 0x80000, CRC(0da904d6) SHA1(e190f1a35147b2f39224832969ca7b1d4a30f6cc))
	ROM_LOAD("frsnd.u21", 0x080000, 0x80000, CRC(14d4bc12) SHA1(9e7005c5bd0afe7f9c9215b39878496640cdea77))
	ROM_LOAD("frsnd.u36", 0x100000, 0x80000, CRC(9964d721) SHA1(5ea0bc051d1909bee80d3feb6b7350b6307b6dcb))
ROM_END

ROM_START(franksti)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("franka.103", 0x0000, 0x10000, CRC(a9aba9be) SHA1(1cc22fcbc0f51a17037637c04e606579956c9cba))
	ROM_REGION16_BE(0x100000, "decodmd3", 0)
	ROM_LOAD16_BYTE("frankdisprom0_i1.03", 0x000001, 0x80000, CRC(6be7cc1c) SHA1(418ea4843c1380478a88aa08a63ab8ec98a55ac3))
	ROM_LOAD16_BYTE("frankdisprom3_i1.03", 0x000000, 0x80000, CRC(9e08281e) SHA1(dcebeccf86b20cbd89449f1f9f1879bcb7abb836))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("frsnd.u7", 0x0000, 0x10000, CRC(084f856c) SHA1(c91331a32b565c2ed3f96156f44143dc22009e8e))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("frsnd.u17", 0x000000, 0x80000, CRC(0da904d6) SHA1(e190f1a35147b2f39224832969ca7b1d4a30f6cc))
	ROM_LOAD("frsnd.u21", 0x080000, 0x80000, CRC(14d4bc12) SHA1(9e7005c5bd0afe7f9c9215b39878496640cdea77))
	ROM_LOAD("frsnd.u36", 0x100000, 0x80000, CRC(9964d721) SHA1(5ea0bc051d1909bee80d3feb6b7350b6307b6dcb))
ROM_END

/*-------------------------------------------------------------
/ Maverick - CPU Rev 3b /DMD  Type 3 2x512K Rom - 64K CPU Rom
/------------------------------------------------------------*/
ROM_START(mav_402)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("mavcpua.404", 0x0000, 0x10000, CRC(9f06bd8d) SHA1(3b931af5455ed9c40f2b6c884427a326bba8f75a))
	ROM_REGION16_BE(0x100000, "decodmd3", 0)
	ROM_LOAD16_BYTE("mavdisp0.402", 0x000001, 0x80000, CRC(4e643525) SHA1(30b91c91c2f1295cdd018023c5ac783570a0aeea))
	ROM_LOAD16_BYTE("mavdisp3.402", 0x000000, 0x80000, CRC(8c5f9460) SHA1(6369b4c98ec6fd5e769275b44631b2b6dd5c411b))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("mavu7.dat", 0x0000, 0x10000, CRC(427e6ab9) SHA1(6ad9295097f3d498383c91adf4ca667f797f29b1))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("mavu17.dat", 0x000000, 0x80000, CRC(cba377b8) SHA1(b7551b6cb64357cdacf1a96cedfccbabf4bd070a))
	ROM_LOAD("mavu21.dat", 0x080000, 0x80000, CRC(be0c6a6f) SHA1(4fee912d9f0d4b196dbfacf06a4202b2fa3037b1))
ROM_END

ROM_START(mav_401)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("mavcpua.404", 0x0000, 0x10000, CRC(9f06bd8d) SHA1(3b931af5455ed9c40f2b6c884427a326bba8f75a))
	ROM_REGION16_BE(0x100000, "decodmd3", 0)
	ROM_LOAD16_BYTE("mavdsar0.401", 0x000001, 0x80000, CRC(35b811af) SHA1(1e235a0f16ef0eecca5b6ec7a2234ed1dc4e4440))
	ROM_LOAD16_BYTE("mavdsar3.401", 0x000000, 0x80000, CRC(c4c126ae) SHA1(b4841e83ec075bddc919217b65afaac97709e69b))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("mavu7.dat", 0x0000, 0x10000, CRC(427e6ab9) SHA1(6ad9295097f3d498383c91adf4ca667f797f29b1))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("mavu17.dat", 0x000000, 0x80000, CRC(cba377b8) SHA1(b7551b6cb64357cdacf1a96cedfccbabf4bd070a))
	ROM_LOAD("mavu21.dat", 0x080000, 0x80000, CRC(be0c6a6f) SHA1(4fee912d9f0d4b196dbfacf06a4202b2fa3037b1))
ROM_END

ROM_START(mav_400)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("mavgc5.400", 0x0000, 0x10000, CRC(e2d0a88b) SHA1(d1571edba47aecc871ac0cfdaabca31774f70fa1))
	ROM_REGION16_BE(0x100000, "decodmd3", 0)
	ROM_LOAD16_BYTE("mavdisp0.400", 0x000001, 0x80000, CRC(b6069484) SHA1(2878d9a0151194bd4a0e12e2f75b02a5d7316b68))
	ROM_LOAD16_BYTE("mavdisp3.400", 0x000000, 0x80000, CRC(149f871f) SHA1(e29a8bf149b77bccaeed202786cf76d9a4fd51df))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("mavu7.dat", 0x0000, 0x10000, CRC(427e6ab9) SHA1(6ad9295097f3d498383c91adf4ca667f797f29b1))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("mavu17.dat", 0x000000, 0x80000, CRC(cba377b8) SHA1(b7551b6cb64357cdacf1a96cedfccbabf4bd070a))
	ROM_LOAD("mavu21.dat", 0x080000, 0x80000, CRC(be0c6a6f) SHA1(4fee912d9f0d4b196dbfacf06a4202b2fa3037b1))
ROM_END

ROM_START(mav_200)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("mavcpua.200", 0x0000, 0x10000, CRC(0d9e9bc3) SHA1(23342950c2343591bdf168e3bc92ca669a702ffa))
	ROM_REGION16_BE(0x100000, "decodmd3", 0)
	ROM_LOAD16_BYTE("mavdisp0.200", 0x000001, 0x80000, CRC(8a85785f) SHA1(af1b8b972359cce4bfd82743e90be8066fa57c31))
	ROM_LOAD16_BYTE("mavdisp3.200", 0x000000, 0x80000, CRC(2098ad22) SHA1(2ccf9c455b89a6b439957199bca75e299e18ac62))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("mavu7.dat", 0x0000, 0x10000, CRC(427e6ab9) SHA1(6ad9295097f3d498383c91adf4ca667f797f29b1))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("mavu17.dat", 0x000000, 0x80000, CRC(cba377b8) SHA1(b7551b6cb64357cdacf1a96cedfccbabf4bd070a))
	ROM_LOAD("mavu21.dat", 0x080000, 0x80000, CRC(be0c6a6f) SHA1(4fee912d9f0d4b196dbfacf06a4202b2fa3037b1))
ROM_END

ROM_START(mav_100)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("mavcpu.100", 0x0000, 0x10000, CRC(13fdc959) SHA1(f8155f0fe5d4c3fe55000ab3b57f298fd9229fef))
	ROM_REGION16_BE(0x100000, "decodmd3", 0)
	ROM_LOAD16_BYTE("mavdsp0.100", 0x000001, 0x80000, CRC(3e01f5c8) SHA1(8e40f399c77aa17bebbefe04742ff2ff95508323))
	ROM_LOAD16_BYTE("mavdsp3.100", 0x000000, 0x80000, CRC(e2b623f2) SHA1(7b5a6d0db30f3deedb8fe0e1731c81ec836a66f5))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("mavu7.dat", 0x0000, 0x10000, CRC(427e6ab9) SHA1(6ad9295097f3d498383c91adf4ca667f797f29b1))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("mavu17.dat", 0x000000, 0x80000, CRC(cba377b8) SHA1(b7551b6cb64357cdacf1a96cedfccbabf4bd070a))
	ROM_LOAD("mavu21.dat", 0x080000, 0x80000, CRC(be0c6a6f) SHA1(4fee912d9f0d4b196dbfacf06a4202b2fa3037b1))
ROM_END

/*-------------------------------------------------------------
/ Cut The Cheese (Redemption)
/------------------------------------------------------------*/
ROM_START(ctcheese)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ctcc5.bin", 0x0000, 0x10000, CRC(465d41de) SHA1(0e30b527d5b47f8823cbe6f196052b090e69e907))
	ROM_REGION16_BE(0x100000, "decodmd3", 0)
	ROM_LOAD16_BYTE("ctcdsp0.bin", 0x000001, 0x80000, CRC(6885734d) SHA1(9ac82c9c8bf4e66d2999fbfd08617ef6c266dfe8))
	ROM_LOAD16_BYTE("ctcdsp3.bin", 0x000000, 0x80000, CRC(0c2b3f3c) SHA1(cb730cc6fdd2a2786d25b46b1c45466ee56132d1))
	ROM_REGION(0x010000, "decobsmt:soundcpu", 0)
	ROM_LOAD("ctcu7.bin", 0x0000, 0x10000, CRC(406b9b9e) SHA1(f3f86c368c92ee0cb47323e6e0ca0fa05b6122bd))
	ROM_REGION(0x1000000, "decobsmt:bsmt", 0)
	ROM_LOAD("ctcu17.bin", 0x000000, 0x80000, CRC(ea125fb3) SHA1(2bc1d2a6138ff77ad19b7bcff784dba73f545883))
	ROM_LOAD("ctcu21.bin", 0x080000, 0x80000, CRC(1b3af383) SHA1(c6b57f3f0781954f75d164d909093e4ed8da440e))
ROM_END

/*-------------------------------------------------------------
/ Roach Racers / Derby Daze
/------------------------------------------------------------*/


/*-------------------------------------------------------------
/ Data East Test Chip 64K ROM
/------------------------------------------------------------*/
ROM_START(detest)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("de_test.512", 0x0000, 0x10000, CRC(bade8ca8) SHA1(e7e9d6622b9c9b9381ba2793297f87f102214972))

	ROM_REGION16_BE(0x01000000, "decodmd3", ROMREGION_ERASE00)
	ROM_REGION(0x010000, "decobsmt:soundcpu", ROMREGION_ERASE00)
	ROM_REGION(0x1000000, "decobsmt:bsmt", ROMREGION_ERASE00)
ROM_END

} // anonymous namespace


GAME(1993,  rab_320,       0,        de_3_dmd2, de3, de_3_state, empty_init, ROT0, "Data East", "Adventures of Rocky and Bullwinkle and Friends (USA 3.20, display A3.00)",           MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // ROCKY+BULLWINKLE AUGUST 12, 1993 USA CPU 3.20. DISPLAY VERSION- BULLWINKLE A3.00 5/24/1993
GAME(1993,  rab_130,       rab_320,  de_3_dmd2, de3, de_3_state, empty_init, ROT0, "Data East", "Adventures of Rocky and Bullwinkle and Friends (USA 1.30, display A1.30)",           MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // ROCKY+BULLWINKLE APRIL 1, 1993 USA CPU 1.30. DISPLAY VERSION- BULLWINKLE A1.30 4/1/1993
GAME(1993,  rab_103s,      rab_320,  de_3_dmd2, de3, de_3_state, empty_init, ROT0, "Data East", "Adventures of Rocky and Bullwinkle and Friends (USA 1.03, display S1.03)",           MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // ROCKY+BULLWINKLE FEBRUARY 3, 1993 USA CPU 1.03. DISPLAY VERSION- BULLWINKLE S1.03 2/2/1993
GAME(1992,  aar_101,       0,        de_3_dmd2, de3, de_3_state, empty_init, ROT0, "Data East", "Aaron Spelling (1.01)",                                                              MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // AARON SPELLING V1.01 12/23/92
GAME(1991,  btmn_106,      0,        de_3_dmd1, de3, de_3_state, empty_init, ROT0, "Data East", "Batman (USA 1.06, display A1.02)",                                                   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // BATMAN USA 1.06. DISP VER: BATMAN A1.02
GAME(1991,  btmn_103,      btmn_106, de_3_dmd1, de3, de_3_state, empty_init, ROT0, "Data East", "Batman (USA 1.03, display A1.02)",                                                   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // BATMAN USA 1.03. DISP VER: BATMAN A1.02
GAME(1991,  btmn_103f,     btmn_106, de_3_dmd1, de3, de_3_state, empty_init, ROT0, "Data East", "Batman (France 1.03, display F1.03)",                                                MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // BATMAN FRANCE 1.03. DISP VER: BATMAN F1.03
GAME(1991,  btmn_103g,     btmn_106, de_3_dmd1, de3, de_3_state, empty_init, ROT0, "Data East", "Batman (Germany 1.03, display G1.04)",                                               MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // BATMAN GERMANY 1.03. DISP VER: BATMAN G1.04
GAME(1991,  btmn_101,      btmn_106, de_3_dmd1, de3, de_3_state, empty_init, ROT0, "Data East", "Batman (USA 1.01, display A1.02)",                                                   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // BATMAN USA 1.01
GAME(1991,  ckpt_a17,      0,        de_3_dmdo, de3, de_3_state, empty_init, ROT0, "Data East", "Checkpoint (1.7)",                                                                   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // CP80 3/6/91
GAME(1994,  gnr_300,       0,        de_3_dmd2, de3, de_3_state, empty_init, ROT0, "Data East", "Guns N Roses (USA 3.00, display A3.00)",                                             MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // GUNS-N-ROSES AUGUST 21, 1994 USA CPU 3.00. DISPLAY VERSION- GNR A3.00 AUGUST 16, 1994
GAME(1994,  gnr_300f,      gnr_300,  de_3_dmd2, de3, de_3_state, empty_init, ROT0, "Data East", "Guns N Roses (French 3.00, display F3.00)",                                          MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // GUNS-N-ROSES AUGUST 21, 1994 FRENCH CPU 3.00. DISPLAY VERSION- GNR F3.00 AUGUST 16, 1994
GAME(1994,  gnr_300d,      gnr_300,  de_3_dmd2, de3, de_3_state, empty_init, ROT0, "Data East", "Guns N Roses (Dutch 3.00, display A3.00)",                                           MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // GUNS-N-ROSES AUGUST 21, 1994 DUTCH CPU 3.00. DISPLAY VERSION- GNR A3.00 AUGUST 16, 1994
GAME(1994,  gnr_200,       gnr_300,  de_3_dmd2, de3, de_3_state, empty_init, ROT0, "Data East", "Guns N Roses (USA 2.00, display A3.00)",                                             MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // GUNS-N-ROSES JULY 5, 1994 USA CPU 2.00
GAME(1992,  hook_408,      0,        de_3_dmd1, de3, de_3_state, empty_init, ROT0, "Data East", "Hook (USA 4.08, display A4.01)",                                                     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // HOOK USA 4.08. DISPLAY: HOOK A4.01
GAME(1992,  hook_404,      hook_408, de_3_dmd1, de3, de_3_state, empty_init, ROT0, "Data East", "Hook (USA 4.04, display A4.01)",                                                     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // HOOK USA 4.04
GAME(1992,  hook_401,      hook_408, de_3_dmd1, de3, de_3_state, empty_init, ROT0, "Data East", "Hook (USA 4.01, display A4.01)",                                                     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // HOOK USA 4.01
GAME(1992,  hook_401_p,    hook_408, de_3_dmd1, de3, de_3_state, empty_init, ROT0, "Data East", "Hook (USA 4.01 with prototype sound, display A4.01)",                                MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // HOOK USA 4.01
GAME(1992,  hook_f401,     hook_408, de_3_dmd1, de3, de_3_state, empty_init, ROT0, "Data East", "Hook (USA 4.01, display F4.01)",                                                     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // HOOK USA 4.01 DISPLAY: HOOK F4.01
GAME(1992,  hook_400,      hook_408, de_3_dmd1, de3, de_3_state, empty_init, ROT0, "Data East", "Hook (USA 4.01, display A4.00)",                                                     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // HOOK USA 4.01 DISPLAY: HOOK A4.00
GAME(1992,  hook_e406,     hook_408, de_3_dmd1, de3, de_3_state, empty_init, ROT0, "Data East", "Hook (UK 4.06, display A4.01)",                                                      MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // HOOK U.K. 4.06
GAME(1993,  jupk_513,      0,        de_3_dmd2, de3, de_3_state, empty_init, ROT0, "Data East", "Jurassic Park (USA 5.13, display A5.10)",                                            MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // JURASSIC PARK SEP. 28, 1993 USA CPU 5.13. DISPLAY VERSION- JURASSIC A5.10 8/24/1993
GAME(1993,  jupk_501,      jupk_513, de_3_dmd2, de3, de_3_state, empty_init, ROT0, "Data East", "Jurassic Park (USA 5.01, display A5.01)",                                            MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // JURASSIC PARK JUNE 28, 1993 USA CPU 5.01. DISPLAY VERSION- JURASSIC A5.01 6/24/1993
GAME(1993,  jupk_501g,     jupk_513, de_3_dmd2, de3, de_3_state, empty_init, ROT0, "Data East", "Jurassic Park (USA 5.01, display G5.01)",                                            MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // JURASSIC PARK JUNE 28, 1993 USA CPU 5.01. DISPLAY VERSION- JURASSIC G5.01 6/24/1993
GAME(1993,  jupk_307,      jupk_513, de_3_dmd2, de3, de_3_state, empty_init, ROT0, "Data East", "Jurassic Park (USA 3.07, display A4.00)",                                            MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // JURASSIC PARK. MAY 25, 1993. USA CPU 3.05
GAME(1993,  jupk_305,      jupk_513, de_3_dmd2, de3, de_3_state, empty_init, ROT0, "Data East", "Jurassic Park (USA 3.05, display A4.00)",                                            MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // JURASSIC PARK. MAY 25, 1993. USA CPU 3.05
GAME(1993,  lah_112,       0,        de_3_dmd2, de3, de_3_state, empty_init, ROT0, "Data East", "Last Action Hero (USA 1.12, display A1.06)",                                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // LAST ACTION HERO NOV. 10, 1993 USA CPU 1.12. DISPLAY VERSION- ACTION HERO A1.06 11/11/1993
GAME(1993,  lah_110,       lah_112,  de_3_dmd2, de3, de_3_state, empty_init, ROT0, "Data East", "Last Action Hero (USA 1.10, display A1.06)",                                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // LAST ACTION HERO OCT. 18, 1993 USA CPU 1.10
GAME(1993,  lah_xxx_s105,  lah_112,  de_3_dmd2, de3, de_3_state, empty_init, ROT0, "Data East", "Last Action Hero (unknown CPU, display L1.05)",                                      MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // DISPLAY VERSION- ACTION HERO L1.05 11/11/1993
GAME(1993,  lah_108s,      lah_112,  de_3_dmd2, de3, de_3_state, empty_init, ROT0, "Data East", "Last Action Hero (USA 1.08, display L1.04)",                                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // LAST ACTION HERO SEPT. 28, 1993 USA CPU 1.08. DISPLAY VERSION- ACTION HERO L1.04 9/5/1993
GAME(1993,  lah_107,       lah_112,  de_3_dmd2, de3, de_3_state, empty_init, ROT0, "Data East", "Last Action Hero (USA 1.07, display A1.06)",                                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // LAST ACTION HERO SEPT. 22, 1993 USA CPU 1.07
GAME(1993,  lah_106c,      lah_112,  de_3_dmd2, de3, de_3_state, empty_init, ROT0, "Data East", "Last Action Hero (Canada 1.06, display A1.04)",                                      MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // LAST ACTION HERO SEPT. 20, 1993 CANADA CPU 1.06. DISPLAY VERSION- ACTION HERO A1.04 9/5/1993
GAME(1993,  lah_104f,      lah_112,  de_3_dmd2, de3, de_3_state, empty_init, ROT0, "Data East", "Last Action Hero (USA 1.04, display F1.01)",                                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // LAST ACTION HERO SEPT. 1, 1993 USA CPU 1.04. DISPLAY VERSION- ACTION HERO F1.01 8/18/1993
GAME(1993,  lah_104s,      lah_112,  de_3_dmd2, de3, de_3_state, empty_init, ROT0, "Data East", "Last Action Hero (USA 1.04, display L1.02)",                                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // LAST ACTION HERO SEPT. 1, 1993 USA CPU 1.04. DISPLAY VERSION- ACTION HERO L1.02 8/30/1993
GAME(1992,  lw3_208,       0,        de_3_dmd2, de3, de_3_state, empty_init, ROT0, "Data East", "Lethal Weapon 3 (USA 2.08, display A2.06)",                                          MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // LW3 11/17/92 USA CPU 2.08. DISPLAY VERSION- LETHAL WEAPON A2.06 9/29/1992
GAME(1992,  lw3_207,       lw3_208,  de_3_dmd2, de3, de_3_state, empty_init, ROT0, "Data East", "Lethal Weapon 3 (USA 2.07, display A2.06)",                                          MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // LW3 AUG 31, 1992 USA CPU 2.07
GAME(1992,  lw3_207c,      lw3_208,  de_3_dmd2, de3, de_3_state, empty_init, ROT0, "Data East", "Lethal Weapon 3 (Canada 2.07, display A2.06)",                                       MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // LW3 AUG 31, 1992 CANADA CPU 2.07
GAME(1992,  lw3_205,       lw3_208,  de_3_dmd2, de3, de_3_state, empty_init, ROT0, "Data East", "Lethal Weapon 3 (USA 2.05, display A2.05)",                                          MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // LW3 JULY 30, 1992 USA CPU 2.05. DISPLAY VERSION- LETHAL WEAPON A2.05 8/14/1992
GAME(1992,  lw3_204e,      lw3_208,  de_3_dmd2, de3, de_3_state, empty_init, ROT0, "Data East", "Lethal Weapon 3 (England 2.04, display A2.02)",                                      MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // LW3 JULY 30. 1992 ENGLAND CPU 2.04. DISPLAY VERSION LETHAL WEAPON A2.02 7/17/1992
GAME(1992,  lw3_203,       lw3_208,  de_3_dmd2, de3, de_3_state, empty_init, ROT0, "Data East", "Lethal Weapon 3 (USA 2.03, display A2.04)",                                          MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // LW3 JULY 17, 1992 USA CPU 2.03. DISPLAY VERSION- LETHAL WEAPON  A2.04 7/29/1992
GAME(1992,  lw3_200,       lw3_208,  de_3_dmd2, de3, de_3_state, empty_init, ROT0, "Data East", "Lethal Weapon 3 (USA 2.00, display A2.04)",                                          MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // LW3 JUNE 16, 1992 USA CPU 2.00. DISPLAY VERSION- LETHAL WEAPON A2.04 7/29/1992
GAME(1992,  mj_130,        0,        de_3_dmd2, de3, de_3_state, empty_init, ROT0, "Data East", "Michael Jordan (1.30, display A1.03)",                                               MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // MICHAEL JORDAN V 1.30 11/4/92. DISPLAY VERSION- JORDAN A1.03 8/13/1993
GAME(1992,  trek_201,      0,        de_3_dmd1, de3, de_3_state, empty_init, ROT0, "Data East", "Star Trek 25th Anniversary (USA 2.01, display A1.09)",                               MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // STARTREK 4/30/92 USA VER. 2.01. DISPLAY: STARTREK A1.09
GAME(1992,  trek_200,      trek_201, de_3_dmd1, de3, de_3_state, empty_init, ROT0, "Data East", "Star Trek 25th Anniversary (USA 2.00, display A1.09)",                               MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // STARTREK 4/16/92 USA VER. 2.00
GAME(1992,  trek_120,      trek_201, de_3_dmd1, de3, de_3_state, empty_init, ROT0, "Data East", "Star Trek 25th Anniversary (USA 1.20, display A1.06)",                               MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // STAR TREK 1/10 USA VER. 1.20. DISPLAY: STARTREK A1.06
GAME(1992,  trek_117,      trek_201, de_3_dmd1, de3, de_3_state, empty_init, ROT0, "Data East", "Star Trek 25th Anniversary (USA 1.17, display A1.09)",                               MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // STAR TREK 12/9 USA VER. 1.17
GAME(1992,  trek_110,      trek_201, de_3_dmd1, de3, de_3_state, empty_init, ROT0, "Data East", "Star Trek 25th Anniversary (USA 1.10, display A1.06)",                               MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // STAR TREK 11/14 USA VER. 1.10. DISPLAY: STARTREK A1.06
GAME(1992,  trek_110_a027, trek_201, de_3_dmd1, de3, de_3_state, empty_init, ROT0, "Data East", "Star Trek 25th Anniversary (USA 1.10, display A0.27)",                               MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // STAR TREK 11/14 USA VER. 1.10. DISPLAY: STARTREK A0.27
GAME(1992,  stwr_106,      0,        de_3_dmd2, de3, de_3_state, empty_init, ROT0, "Data East", "Star Wars (Unofficial 1.06, display A1.05)",                                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // STAR WARS 2016 UNOFFICIAL 1.06. DISPLAY VERSION- STAR WARS A1.05 12/4/1992
GAME(1992,  stwr_106_s105, stwr_106, de_3_dmd2, de3, de_3_state, empty_init, ROT0, "Data East", "Star Wars (Unofficial 1.06, display S1.05)",                                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // DISPLAY VERSION- STAR WARS S1.05 12/4/1992
GAME(1992,  stwr_106_a046, stwr_106, de_3_dmd2, de3, de_3_state, empty_init, ROT0, "Data East", "Star Wars (Unofficial 1.06, display A0.46)",                                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // DISPLAY VERSION- STAR WARS A0.46 10/9/1992
GAME(1992,  stwr_104,      stwr_106, de_3_dmd2, de3, de_3_state, empty_init, ROT0, "Data East", "Star Wars (USA 1.04, display A1.05)",                                                MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // STAR WARS USA CPU 1.04
GAME(1992,  stwr_103,      stwr_106, de_3_dmd2, de3, de_3_state, empty_init, ROT0, "Data East", "Star Wars (USA 1.03, display A1.05)",                                                MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // STAR WARS USA CPU 1.03
GAME(1992,  stwr_103_a104, stwr_106, de_3_dmd2, de3, de_3_state, empty_init, ROT0, "Data East", "Star Wars (USA 1.03, display A1.04)",                                                MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // STAR WARS USA CPU 1.03. DISPLAY VERSION- STAR WARS A1.04 11/20/1992
GAME(1992,  stwr_102,      stwr_106, de_3_dmd2, de3, de_3_state, empty_init, ROT0, "Data East", "Star Wars (USA 1.02, display A1.05)",                                                MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // STAR WARS USA CPU 1.02
GAME(1992,  stwr_102e,     stwr_106, de_3_dmd2, de3, de_3_state, empty_init, ROT0, "Data East", "Star Wars (England 1.02, display A1.05)",                                            MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // STAR WARS ENGLAND CPU 1.02
GAME(1992,  stwr_101,      stwr_106, de_3_dmd2, de3, de_3_state, empty_init, ROT0, "Data East", "Star Wars (USA 1.01, display A1.02)",                                                MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // STAR WARS USA CPU 1.01. DISPLAY VERSION- STAR WARS A1.02 10/29/1992
GAME(1992,  stwr_101g,     stwr_106, de_3_dmd2, de3, de_3_state, empty_init, ROT0, "Data East", "Star Wars (German 1.01, display G1.02)",                                             MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // STAR WARS GERMAN CPU 1.01. DISPLAY VERSION- STAR WARS G1.02 29/10/1992
GAME(1993,  tftc_303,      0,        de_3_dmd2, de3, de_3_state, empty_init, ROT0, "Data East", "Tales From the Crypt (USA 3.03, display A3.01)",                                     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // TFTC FEBRUARY 22,1994 USA CPU 3.03. DISPLAY VERSION- CRYPT A3.01 12/28/1993
GAME(1993,  tftc_302,      tftc_303, de_3_dmd2, de3, de_3_state, empty_init, ROT0, "Data East", "Tales From the Crypt (Dutch 3.02, display A3.01)",                                   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // TFTC JANUARY 06, 1994 DUTCH CPU 3.02
GAME(1993,  tftc_300,      tftc_303, de_3_dmd2, de3, de_3_state, empty_init, ROT0, "Data East", "Tales From the Crypt (USA 3.00, display A3.00)",                                     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // TFTC DECEMBER 15, 1993 USA CPU 3.00. DISPLAY VERSION- CRYPT A3.00 12/16/1993
GAME(1993,  tftc_200,      tftc_303, de_3_dmd2, de3, de_3_state, empty_init, ROT0, "Data East", "Tales From the Crypt (USA 2.00, display A2.00)",                                     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // TFTC DECEMBER 03, 1993 USA CPU 2.00. DISPLAY VERSION- CRYPT A2.00 12/3/1993
GAME(1993,  tftc_104s,     tftc_303, de_3_dmd2, de3, de_3_state, empty_init, ROT0, "Data East", "Tales From the Crypt (USA 1.04, display L1.03)",                                     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // TFTC NOVEMBER 19, 1993 USA CPU 1.04. DISPLAY VERSION- CRYPT L1.03 11/11/1993
GAME(1991,  tmnt_104,      0,        de_3_dmdo, de3, de_3_state, empty_init, ROT0, "Data East", "Teenage Mutant Ninja Turtles (USA 1.04, display A1.04)",                             MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // T.M.N.T. USA 1.04. DISPLAY VER: TMNT A1.04
GAME(1991,  tmnt_104g,     tmnt_104, de_3_dmdo, de3, de_3_state, empty_init, ROT0, "Data East", "Teenage Mutant Ninja Turtles (Germany 1.04, display A1.04)",                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // T.M.N.T. GERMANY 1.04.
GAME(1991,  tmnt_103,      tmnt_104, de_3_dmdo, de3, de_3_state, empty_init, ROT0, "Data East", "Teenage Mutant Ninja Turtles (1.03)",                                                MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // T.M.N.T. A 1.03
GAME(1991,  tmnt_101,      tmnt_104, de_3_dmdo, de3, de_3_state, empty_init, ROT0, "Data East", "Teenage Mutant Ninja Turtles (1.01)",                                                MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // T.M.N.T. A 1.01
GAME(1991,  tmnt_a07,      tmnt_104, de_3_dmdo, de3, de_3_state, empty_init, ROT0, "Data East", "Teenage Mutant Ninja Turtles (A 0.7 VUK prototype)",                                 MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // T.M.N.T. A 0.7 VUK
GAME(1994,  tomy_400,      0,        de_3_dmd2, de3, de_3_state, empty_init, ROT0, "Data East", "The Who's Tommy Pinball Wizard (USA 4.00, display A4.00)",                           MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // TOMMY APRIL 6, 1994 USA CPU 4.00. DISPLAY VERSION- TOMMY A4.00 MAY 5, 1994
GAME(1994,  tomy_301g,     tomy_400, de_3_dmd2, de3, de_3_state, empty_init, ROT0, "Data East", "The Who's Tommy Pinball Wizard (German 3.01, display G3.00)",                        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // TOMMY MARCH 22, 1994 GERMAN CPU 3.01. DISPLAY VERSION- TOMMY G3.00 FEBRUARY 16, 1994
GAME(1994,  tomy_300h,     tomy_400, de_3_dmd2, de3, de_3_state, empty_init, ROT0, "Data East", "The Who's Tommy Pinball Wizard (Dutch 3.00, display A3.00)",                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // TOMMY FEBRUARY 16, 1994 DUTCH CPU 3.00. DISPLAY VERSION- TOMMY A3.00 FEBRUARY 15, 1994
GAME(1994,  tomy_201h,     tomy_400, de_3_dmd2, de3, de_3_state, empty_init, ROT0, "Data East", "The Who's Tommy Pinball Wizard (Dutch 2.01, display A2.00)",                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // TOMMY FEBRUARY 16, 1994 DUTCH CPU 3.00. DISPLAY VERSION- TOMMY A3.00 FEBRUARY 15, 1994
GAME(1994,  tomy_102,      tomy_400, de_3_dmd2, de3, de_3_state, empty_init, ROT0, "Data East", "The Who's Tommy Pinball Wizard (USA 1.02, display A3.00)",                           MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // TOMMY FEBRUARY 11, 1994 DUTCH CPU 2.01. DISPLAY VERSION- TOMMY A2.00 FEBRUARY 1, 1994
GAME(1994,  tomy_102be,    tomy_400, de_3_dmd2, de3, de_3_state, empty_init, ROT0, "Data East", "The Who's Tommy Pinball Wizard (Belgium 1.02, display A1.02)",                       MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // TOMMY JANUARY 26, 1994 BELGIUM CPU 1.02. DISPLAY VERSION- TOMMY A1.02 JANUARY 25, 1994
GAME(1994,  wwfr_106,      0,        de_3_dmd2, de3, de_3_state, empty_init, ROT0, "Data East", "WWF Royal Rumble (USA 1.06, display A1.02)",                                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // RUMBLIN' AN' A TUMBLIN' WWF WRESTLING AUG. 01, 1994 USA CPU 1.06. DISPLAY VERSION- WWF A1.02 JUNE 29, 1994
GAME(1994,  wwfr_103,      wwfr_106, de_3_dmd2, de3, de_3_state, empty_init, ROT0, "Data East", "WWF Royal Rumble (USA 1.03, display A1.01)",                                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // RUMBLIN' AN' A TUMBLIN' WWF WRESTLING APR. 28, 1994 USA CPU 1.03. DISPLAY VERSION- WWF A1.01 APRIL 14, 1994
GAME(1994,  wwfr_103f,     wwfr_106, de_3_dmd2, de3, de_3_state, empty_init, ROT0, "Data East", "WWF Royal Rumble (French 1.03, display F1.01)",                                      MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE ) // RUMBLIN' AN' A TUMBLIN' WWF WRESTLING APR. 28, 1994 FRENCH CPU 1.03. DISPLAY VERSION- WWF F1.01 APRIL 14, 1994
GAME(1995,  batmanf,       0,        de_3b,     de3, de_3_state, empty_init, ROT0, "Sega",      "Batman Forever (4.0)",                                                               MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  batmanf3,      batmanf,  de_3b,     de3, de_3_state, empty_init, ROT0, "Sega",      "Batman Forever (3.0)",                                                               MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  batmanf2,      batmanf,  de_3b,     de3, de_3_state, empty_init, ROT0, "Sega",      "Batman Forever (2.02)",                                                              MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  batmanf1,      batmanf,  de_3b,     de3, de_3_state, empty_init, ROT0, "Sega",      "Batman Forever (1.02)",                                                              MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  bmf_uk,        batmanf,  de_3b,     de3, de_3_state, empty_init, ROT0, "Sega",      "Batman Forever (English)",                                                           MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  bmf_cn,        batmanf,  de_3b,     de3, de_3_state, empty_init, ROT0, "Sega",      "Batman Forever (Canadian)",                                                          MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  bmf_no,        batmanf,  de_3b,     de3, de_3_state, empty_init, ROT0, "Sega",      "Batman Forever (Norwegian)",                                                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  bmf_sv,        batmanf,  de_3b,     de3, de_3_state, empty_init, ROT0, "Sega",      "Batman Forever (Swedish)",                                                           MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  bmf_at,        batmanf,  de_3b,     de3, de_3_state, empty_init, ROT0, "Sega",      "Batman Forever (Austrian)",                                                          MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  bmf_ch,        batmanf,  de_3b,     de3, de_3_state, empty_init, ROT0, "Sega",      "Batman Forever (Swiss)",                                                             MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  bmf_de,        batmanf,  de_3b,     de3, de_3_state, empty_init, ROT0, "Sega",      "Batman Forever (German)",                                                            MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  bmf_be,        batmanf,  de_3b,     de3, de_3_state, empty_init, ROT0, "Sega",      "Batman Forever (Belgian)",                                                           MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  bmf_fr,        batmanf,  de_3b,     de3, de_3_state, empty_init, ROT0, "Sega",      "Batman Forever (French)",                                                            MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  bmf_nl,        batmanf,  de_3b,     de3, de_3_state, empty_init, ROT0, "Sega",      "Batman Forever (Dutch, 4.0)",                                                        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  bmf_nl302,     batmanf,  de_3b,     de3, de_3_state, empty_init, ROT0, "Sega",      "Batman Forever (Dutch, 3.02)",                                                       MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  bmf_it,        batmanf,  de_3b,     de3, de_3_state, empty_init, ROT0, "Sega",      "Batman Forever (Italian, 4.0, Nov. 1 1995, Display Rev. 4.01)",                      MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  bmf_ita,       batmanf,  de_3b,     de3, de_3_state, empty_init, ROT0, "Sega",      "Batman Forever (Italian, 4.0, Sept. 26 1995, Display Rev. 4.00)",                    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  bmf_itb,       batmanf,  de_3b,     de3, de_3_state, empty_init, ROT0, "Sega",      "Batman Forever (Italian, 4.0, Sept. 26 1995, Display Rev. 4.00, earlier sound ROM)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  bmf_sp,        batmanf,  de_3b,     de3, de_3_state, empty_init, ROT0, "Sega",      "Batman Forever (Spanish)",                                                           MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  bmf_jp,        batmanf,  de_3b,     de3, de_3_state, empty_init, ROT0, "Sega",      "Batman Forever (Japanese)",                                                          MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  bmf_time,      batmanf,  de_3b,     de3, de_3_state, empty_init, ROT0, "Sega",      "Batman Forever (Timed Play)",                                                        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  baywatch,      0,        de_3b,     de3, de_3_state, empty_init, ROT0, "Sega",      "Baywatch",                                                                           MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  bay_d300,      baywatch, de_3b,     de3, de_3_state, empty_init, ROT0, "Sega",      "Baywatch (3.00 Dutch)",                                                              MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  bay_g300,      baywatch, de_3b,     de3, de_3_state, empty_init, ROT0, "Sega",      "Baywatch (3.00 German)",                                                             MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  bay_d400,      baywatch, de_3b,     de3, de_3_state, empty_init, ROT0, "Sega",      "Baywatch (4.00 English)",                                                            MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  bay_e400,      baywatch, de_3b,     de3, de_3_state, empty_init, ROT0, "Sega",      "Baywatch (4.00 Dutch)",                                                              MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  bay_f201,      baywatch, de_3b,     de3, de_3_state, empty_init, ROT0, "Sega",      "Baywatch (2.01 French)",                                                             MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1994,  frankst,       0,        de_3b,     de3, de_3_state, empty_init, ROT0, "Sega",      "Mary Shelley's Frankenstein",                                                        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  frankstg,      frankst,  de_3b,     de3, de_3_state, empty_init, ROT0, "Sega",      "Mary Shelley's Frankenstein (Germany)",                                              MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  franksti,      frankst,  de_3b,     de3, de_3_state, empty_init, ROT0, "Sega",      "Mary Shelley's Frankenstein (Italy)",                                                MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1994,  mav_402,       0,        de_3b,     de3, de_3_state, empty_init, ROT0, "Sega",      "Maverick, The Movie (4.04, Display Rev. 4.02)",                                      MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1994,  mav_401,       mav_402,  de_3b,     de3, de_3_state, empty_init, ROT0, "Sega",      "Maverick, The Movie (4.02, Display Rev. 4.01)",                                      MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1994,  mav_400,       mav_402,  de_3b,     de3, de_3_state, empty_init, ROT0, "Sega",      "Maverick, The Movie (Display Rev. 4.00)",                                            MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1994,  mav_200,       mav_402,  de_3b,     de3, de_3_state, empty_init, ROT0, "Data East", "Maverick, The Movie (2.00)",                                                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1994,  mav_100,       mav_402,  de_3b,     de3, de_3_state, empty_init, ROT0, "Data East", "Maverick, The Movie (1.00)",                                                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1998,  detest,        0,        detest,    de3, de_3_state, empty_init, ROT0, "Data East", "Data East Test Chip",                                                                MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1996,  ctcheese,      0,        de_3b,     de3, de_3_state, empty_init, ROT0, "Sega",      "Cut The Cheese (Redemption)",                                                        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
