// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle
/*******************************************************************************

Diamond Bridge Computer (model M1011)
Also sold by Nu Vations as Nu Va Bridge Computer (model NV211)

Hardware notes:
- PCB label: MCL, M1011
- Hitachi HD44860 @ ~800kHz (33K resistor)
- LCD with custom segments, no sound
- comms jack for playing against another Bridge Computer

TODO:
- add comms port
- is Diamond Bridge Computer II (model M1021) on similar hardware?

*******************************************************************************/

#include "emu.h"

#include "cpu/hmcs40/hmcs40.h"
#include "video/pwm.h"

#include "screen.h"

// internal artwork
#include "dbridgec.lh"


namespace {

class dbridgec_state : public driver_device
{
public:
	dbridgec_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_display(*this, "display"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void dbridgec(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(in0_changed) { refresh_irq(); }

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD { refresh_irq(); }

private:
	// devices/pointers
	required_device<hmcs40_cpu_device> m_maincpu;
	required_device<pwm_display_device> m_display;
	required_ioport_array<4> m_inputs;

	u8 m_inp_mux = 0;
	u8 m_lcd_com = 0;
	u64 m_lcd_segs = 0;

	// I/O handlers
	void update_lcd();
	template<int N> void lcd1_w(u8 data);
	void lcd2_w(u16 data);

	u8 read_buttons();
	void refresh_irq();
	u16 input_r();
	template<int N> void input_w(u8 data);
};

void dbridgec_state::machine_start()
{
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_lcd_com));
	save_item(NAME(m_lcd_segs));
}



/*******************************************************************************
    I/O
*******************************************************************************/

void dbridgec_state::update_lcd()
{
	const u8 com1 = BIT(m_lcd_com, 4);
	const u8 com2 = m_lcd_com & 0xf;
	const u64 data = com1 ? m_lcd_segs : ~m_lcd_segs;

	for (int i = 0; i < 4; i++)
		m_display->write_row(i, (BIT(com2, i) == com1) ? data : 0);
}

template<int N>
void dbridgec_state::lcd1_w(u8 data)
{
	// R0x-R6x: LCD segments
	const u8 shift = N * 4;
	m_lcd_segs = (m_lcd_segs & ~(u64(0xf << shift))) | (u64(data) << shift);
	update_lcd();
}

void dbridgec_state::lcd2_w(u16 data)
{
	// D0-D4: LCD common
	m_lcd_com = data & 0x1f;

	// D9,D10,D12-D15: more LCD segments
	const u8 segs = (data >> 9 & 3) | (data >> 10 & 0x3c);
	m_lcd_segs = (m_lcd_segs & 0x0fff'ffffULL) | u64(segs) << 28;
	update_lcd();

	// D5: comms port
}

u8 dbridgec_state::read_buttons()
{
	u8 data = 0;

	for (int i = 0; i < 4; i++)
		if (m_inp_mux & m_inputs[i]->read())
			data |= 1 << i;

	return data;
}

void dbridgec_state::refresh_irq()
{
	// right button column goes to MCU INT0
	m_maincpu->set_input_line(0, (read_buttons() & 1) ? CLEAR_LINE : ASSERT_LINE);
}

u16 dbridgec_state::input_r()
{
	// D6-D8: read buttons
	u16 data = read_buttons() << 5 & 0x1c0;
	return ~data;

	// D5: comms port
}

template<int N>
void dbridgec_state::input_w(u8 data)
{
	// R4x,R5x: input mux
	const u8 shift = N * 4;
	m_inp_mux = (m_inp_mux & ~(0xf << shift)) | ((data ^ 0xf) << shift);
	refresh_irq();
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

#define IN0_CHANGED(x) \
	PORT_CHANGED_MEMBER(DEVICE_SELF, dbridgec_state, in0_changed, 0)

static INPUT_PORTS_START( dbridgec )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) IN0_CHANGED() PORT_CODE(KEYCODE_U) PORT_NAME("Dummy")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) IN0_CHANGED() PORT_CODE(KEYCODE_T) PORT_NAME("Partner")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) IN0_CHANGED() PORT_CODE(KEYCODE_L) PORT_NAME("Level")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) IN0_CHANGED() PORT_CODE(KEYCODE_H) PORT_NAME("Change Side")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) IN0_CHANGED() PORT_CODE(KEYCODE_S) PORT_NAME("Score")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) IN0_CHANGED() PORT_CODE(KEYCODE_R) PORT_NAME("New Rubber")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) IN0_CHANGED() PORT_CODE(KEYCODE_D) PORT_NAME("Deal")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("Pass")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Double")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("No Trump")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("Clubs")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Diamonds")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("Hearts")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("Spades")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("A")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_K) PORT_NAME("K")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Q")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME("J")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("10")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Enter")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void dbridgec_state::dbridgec(machine_config &config)
{
	// basic machine hardware
	HD44860(config, m_maincpu, 800'000); // approximation
	m_maincpu->write_r<0>().set(FUNC(dbridgec_state::lcd1_w<0>));
	m_maincpu->write_r<1>().set(FUNC(dbridgec_state::lcd1_w<1>));
	m_maincpu->write_r<2>().set(FUNC(dbridgec_state::lcd1_w<2>));
	m_maincpu->write_r<3>().set(FUNC(dbridgec_state::lcd1_w<3>));
	m_maincpu->write_r<4>().set(FUNC(dbridgec_state::lcd1_w<4>));
	m_maincpu->write_r<4>().append(FUNC(dbridgec_state::input_w<0>));
	m_maincpu->write_r<5>().set(FUNC(dbridgec_state::lcd1_w<5>));
	m_maincpu->write_r<5>().append(FUNC(dbridgec_state::input_w<1>));
	m_maincpu->write_r<6>().set(FUNC(dbridgec_state::lcd1_w<6>));
	m_maincpu->write_d().set(FUNC(dbridgec_state::lcd2_w));
	m_maincpu->read_d().set(FUNC(dbridgec_state::input_r));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(4, 34);
	m_display->set_bri_levels(0.1);
	config.set_default_layout(layout_dbridgec);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1920/1.5, 1068/1.5);
	screen.set_visarea_full();
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( dbridgec )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD("hd44860_b29.u1", 0x0000, 0x2000, CRC(9ebb51e0) SHA1(0e99f4247ba516cd93bec889faebf1b6ba22f361) )
	ROM_IGNORE( 0x2000 ) // ignore factory test banks

	ROM_REGION( 234761, "screen", 0 )
	ROM_LOAD("dbridgec.svg", 0, 234761, CRC(24834e57) SHA1(3e10afefa6ef112cf1bb339bf9dd25ed0ca4c150) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY, FULLNAME, FLAGS
SYST( 1987, dbridgec, 0,      0,      dbridgec, dbridgec, dbridgec_state, empty_init, "Diamond", "Bridge Computer (Diamond)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW | MACHINE_NODEVICE_LAN )
