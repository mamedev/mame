// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/******************************************************************************

Chess Champion: Mark V / Mark VI (aka MK V / MK VI)

Released in late 1981, the chess engine was initially written by David Broughton
for a Z80 CPU and used in a prototype. It was ported to 6502 by Mark Taylor,
I/O by Mike Johnson. Support from David Levy and Kevin o'Connell, hardware
by Nick Toop. These credits are in the ROM data.

Mark VI/Philidor was released a year later, it was a plug-in module for the Mark V.
It's not much stronger than Mark V(retroactively called Mark V/Travemunde).

Hardware notes:
- x

TODO:
- WIP

******************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "machine/nvram.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "video/hlcd0538.h"
#include "video/pwm.h"

#include "screen.h"
#include "speaker.h"

#include <algorithm>

// internal artwork
//#include "saitek_mark5.lh" // clickable


namespace {

class mark5_state : public driver_device
{
public:
	mark5_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_display(*this, "display%u", 0),
		m_lcd(*this, "lcd%u", 0),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0),
		m_out_x(*this, "%u.%u.%u", 0U, 0U, 0U)
	{ }

	// machine drivers
	void mark5(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device_array<pwm_display_device, 4> m_display;
	required_device_array<hlcd0538_device, 3> m_lcd;
	required_device<dac_bit_interface> m_dac;
	required_ioport_array<2> m_inputs;
	output_finder<3, 8, 34> m_out_x;

	// address maps
	void main_map(address_map &map);

	// I/O handlers
	DECLARE_WRITE8_MEMBER(lcd_data_w);
	DECLARE_READ8_MEMBER(sound_r);
	DECLARE_READ8_MEMBER(reset_irq_r);
	DECLARE_READ8_MEMBER(input_r);

	template<int N> DECLARE_WRITE8_MEMBER(pwm_output_w);
	template<int N> DECLARE_WRITE64_MEMBER(lcd_output_w);

	u8 m_dac_data;
	u8 m_lcd_lcd;
	u64 m_lcd_data[3];

	emu_timer *m_irqtimer;
	TIMER_CALLBACK_MEMBER(interrupt);
	void write_lcd(int state);
};

void mark5_state::machine_start()
{
	m_out_x.resolve();
	m_irqtimer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(mark5_state::interrupt),this));

	// zerofill
	m_dac_data = 0;
	m_lcd_lcd = 0;
	std::fill_n(m_lcd_data, ARRAY_LENGTH(m_lcd_data), 0);

	// register for savestates
	save_item(NAME(m_dac_data));
	save_item(NAME(m_lcd_lcd));
	save_item(NAME(m_lcd_data));
}

void mark5_state::machine_reset()
{
	reset_irq_r(machine().dummy_space(), 0);
}



/******************************************************************************
    I/O
******************************************************************************/

template<int N>
WRITE8_MEMBER(mark5_state::pwm_output_w)
{
	m_out_x[N][offset & 0x3f][offset >> 6] = data;
}

template<int N>
WRITE64_MEMBER(mark5_state::lcd_output_w)
{
	m_lcd_data[N] = data;

	u8 sel = m_lcd_data[0] & 0xff;
	if (N == 0)
		data >>= 8;

	m_display[N]->matrix(sel, data);
}

void mark5_state::write_lcd(int state)
{
	for (int i = 0; i < 3; i++)
		m_lcd[i]->lcd_w(state);

	m_lcd_lcd = state;
}

TIMER_CALLBACK_MEMBER(mark5_state::interrupt)
{
	// master clock to MC14020, Q12 to IRQ @ 480Hz
	// irq active ~34.61us (470pF, 100K to GND)
	m_irqtimer->adjust(attotime::from_hz(19.6608_MHz_XTAL / 10 / 0x1000));
	m_maincpu->pulse_input_line(0, attotime::from_nsec(34610));

	// MC14020 Q13(1 stage further than IRQ) goes to LCD "LCD" pins
	write_lcd(m_lcd_lcd ^ 1);
}

READ8_MEMBER(mark5_state::reset_irq_r)
{
	if (!machine().side_effects_disabled())
	{
		// MC14020 R
		m_irqtimer->adjust(attotime::from_hz((19.6608_MHz_XTAL / 10 / 0x1000) * 2));
		write_lcd(0);
	}

	return 0xff;
}

READ8_MEMBER(mark5_state::sound_r)
{
	if (!machine().side_effects_disabled())
	{
		// 7474 to speaker out
		m_dac->write(m_dac_data);
		m_dac_data ^= 1;
	}

	return 0xff;
}

WRITE8_MEMBER(mark5_state::lcd_data_w)
{
	// d0,d2,d4: LCD data
	for (int i = 0; i < 3; i++)
	{
		m_lcd[i]->data_w(BIT(data, i*2));

		m_lcd[i]->clk_w(1);
		m_lcd[i]->clk_w(0);
	}
}

READ8_MEMBER(mark5_state::input_r)
{
	// _a6: configuration diodes
	// a0-a5: multiplexed inputs
	return 0xff;
}



/******************************************************************************
    Address Maps
******************************************************************************/

void mark5_state::main_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x3fff).ram();
	map(0x4000, 0x407f).r(FUNC(mark5_state::input_r));
	map(0x4400, 0x4400).w(FUNC(mark5_state::lcd_data_w));
	map(0x4800, 0x4800).r(FUNC(mark5_state::sound_r));
	map(0x4c00, 0x4c00).r(FUNC(mark5_state::reset_irq_r));
	map(0x5000, 0x50ff).ram();
	map(0x8000, 0xffff).rom();
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( mark5 )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4)

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8)
INPUT_PORTS_END



/******************************************************************************
    Machine Drivers
******************************************************************************/

void mark5_state::mark5(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, 19.6608_MHz_XTAL / 10);
	m_maincpu->set_addrmap(AS_PROGRAM, &mark5_state::main_map);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::MAGNETS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));

	/* video hardware */
	HLCD0538(config, m_lcd[0]).write_cols().set(FUNC(mark5_state::lcd_output_w<0>));
	PWM_DISPLAY(config, m_display[0]).set_size(8, 26);
	m_display[0]->output_x().set(FUNC(mark5_state::pwm_output_w<0>));

	HLCD0539(config, m_lcd[1]).write_cols().set(FUNC(mark5_state::lcd_output_w<1>));
	PWM_DISPLAY(config, m_display[1]).set_size(8, 34);
	m_display[1]->output_x().set(FUNC(mark5_state::pwm_output_w<1>));

	HLCD0539(config, m_lcd[2]).write_cols().set(FUNC(mark5_state::lcd_output_w<2>));
	PWM_DISPLAY(config, m_display[2]).set_size(8, 34);
	m_display[2]->output_x().set(FUNC(mark5_state::pwm_output_w<2>));

	PWM_DISPLAY(config, m_display[3]).set_size(8, 8);

	//config.set_default_layout(layout_saitek_mark5);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
	VOLTAGE_REGULATOR(config, "vref").add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( ccmk5 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("c47024_syp_2364-3-y51", 0x8000, 0x2000, CRC(c210f530) SHA1(60ba3809ed3054024508344f654a6846061fafd5) ) // 2364
	ROM_LOAD("c47025_syp_2364-3-y5a", 0xa000, 0x2000, CRC(3239c96b) SHA1(6a23713b30c48546d993a0de8998c8de9044e48c) ) // "
	ROM_LOAD("c47026_syp_2364-3-y5c", 0xc000, 0x2000, CRC(1754ccab) SHA1(d246b6aa2e2a1858dd6608a4dbf496778f79b22e) ) // "
	ROM_LOAD("c47027_syp_2364-3-y5d", 0xe000, 0x2000, CRC(7c0f7bd8) SHA1(68b4566f0501005f6b1739bb24a4bec990421a6f) ) // "
ROM_END

ROM_START( ccmk6 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("y6_80", 0x8000, 0x2000, CRC(8144dd71) SHA1(8d6fbb4aa9757149c81d2bf533085dc5203f0751) ) // 2764
	ROM_LOAD("y6_a0", 0xa000, 0x2000, CRC(dd77dd90) SHA1(844aee56e1941f05bdf046d95c5ae687707a2c95) ) // "
	ROM_LOAD("y6_c0", 0xc000, 0x2000, CRC(705e5718) SHA1(513bba3e7344194efaaf022a7934d32d8cba3cb5) ) // "
	ROM_LOAD("y6_e0", 0xe000, 0x2000, CRC(b92c3eb3) SHA1(99a20f5e971b8c4228e0eda0a4c05750d46b95f6) ) // "
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME   PARENT CMP MACHINE INPUT  STATE        INIT        COMPANY, FULLNAME, FLAGS
CONS( 1981, ccmk5, 0,      0, mark5,  mark5, mark5_state, empty_init, "SciSys / Philidor Software", "Chess Champion: Mark V", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
CONS( 1982, ccmk6, 0,      0, mark5,  mark5, mark5_state, empty_init, "SciSys / Philidor Software", "Chess Champion: Mark VI/Philidor", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
