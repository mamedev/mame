// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle
/*******************************************************************************

Applied Concepts Boris (electronic chess computer)

Hardware notes:
- MK3850N-3 CPU @ 2 MHz from XTAL, MK3853N memory interface
- 256 bytes RAM(2*2112), 2*AMI 2KB ROM (2nd ROM only half used)
- 8-digit 16seg led panel

When it was first released, it was in kit form. An extensive assembly manual with
schematics was included. It was later distributed by Chafitz in pre-assembled form.
There's also an updated revision, identifiable by the startup message "Boris awaits
your move"(same as Boris Master) instead of "Boris plays black".

Boris Master included a battery, RESET was renamed to MEMORY. 2 known versions:
one with C10617/C10618 ROMs(same as Boris rev. 01), and one with a single 4KB
ROM labeled 007-7027-00.

*******************************************************************************/

#include "emu.h"

#include "cpu/f8/f8.h"
#include "machine/f3853.h"
#include "video/pwm.h"

// internal artwork
#include "aci_boris.lh"


namespace {

class boris_state : public driver_device
{
public:
	boris_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_display(*this, "display"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void boris(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(reset_switch) { update_reset(newval); }

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<pwm_display_device> m_display;
	required_ioport_array<4> m_inputs;

	u8 m_io[2] = { };
	u8 m_4042 = 0;

	void main_map(address_map &map) ATTR_COLD;
	void main_io(address_map &map) ATTR_COLD;

	void update_reset(ioport_value state);

	void update_display();
	void mux_w(u8 data);
	void digit_w(u8 data);
	u8 input_r();
};

void boris_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_io));
	save_item(NAME(m_4042));
}

void boris_state::machine_reset()
{
	update_reset(ioport("RESET")->read());
}

void boris_state::update_reset(ioport_value state)
{
	// reset switch is tied to MK3850 RESET pin
	m_maincpu->set_input_line(INPUT_LINE_RESET, state ? ASSERT_LINE : CLEAR_LINE);

	// clear display
	if (state)
		m_display->clear();
}



/*******************************************************************************
    I/O
*******************************************************************************/

// MK3850 ports/TTL

void boris_state::update_display()
{
	// port 1 is latched as long as 4042 clock is low
	// (yes low, this is actually (~~m_io[0] & 8) since output ports are inverted)
	if (m_io[0] & 8)
		m_4042 = bitswap<8>(m_io[1],4,2,0,6,5,1,3,7);

	// 16 segments via port 1 and 4042 output (latched port 1)
	u16 seg_data = ~(m_4042 << 8 | m_io[1]);
	m_display->matrix(1 << (~m_io[0] & 7), seg_data);
}

void boris_state::mux_w(u8 data)
{
	// IO00-IO02: 4028 A-C to digit/input mux (4028 D to GND)
	// IO03: clock 4042
	m_io[0] = data;
	update_display();
}

u8 boris_state::input_r()
{
	// IO04-IO07: multiplexed inputs from 4028 4-7
	u8 data = m_io[0];
	u8 sel = ~data & 7;
	if (sel >= 4)
		data |= m_inputs[sel-4]->read() << 4;

	return data;
}

void boris_state::digit_w(u8 data)
{
	// IO10-IO17: digit segments
	m_io[1] = data;
	update_display();
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void boris_state::main_map(address_map &map)
{
	map.global_mask(0x0fff);
	map(0x0000, 0x0bff).rom();
	map(0x0c00, 0x0cff).mirror(0x300).ram();
}

void boris_state::main_io(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(boris_state::input_r), FUNC(boris_state::mux_w));
	map(0x01, 0x01).w(FUNC(boris_state::digit_w));
	map(0x0c, 0x0f).rw("smi", FUNC(f3853_device::read), FUNC(f3853_device::write));
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( boris )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("G.7")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("H.8")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("Set / 9") // labeled just "SET" on 1st version
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("CE") // clear entry

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("D.4 / Rook")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("E.5 / Queen")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("F.6 / King")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Time")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("A.1 / Pawn")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("B.2 / Knight")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("C.3 / Bishop")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_K) PORT_NAME("Rank")

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SPACE) PORT_CODE(KEYCODE_MINUS) PORT_NAME("-")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("B/W") // black/white
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Enter")

	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_F1) PORT_TOGGLE PORT_CHANGED_MEMBER(DEVICE_SELF, boris_state, reset_switch, 0) PORT_NAME("Reset Switch")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void boris_state::boris(machine_config &config)
{
	// basic machine hardware
	F8(config, m_maincpu, 2_MHz_XTAL); // MK3850
	m_maincpu->set_addrmap(AS_PROGRAM, &boris_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &boris_state::main_io);
	m_maincpu->set_irq_acknowledge_callback("smi", FUNC(f3853_device::int_acknowledge));

	f3853_device &smi(F3853(config, "smi", 2_MHz_XTAL));
	smi.int_req_callback().set_inputline("maincpu", F8_INPUT_LINE_INT_REQ);

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(8, 16);
	m_display->set_segmask(0xff, 0xffff);
	m_display->set_bri_levels(0.05);
	config.set_default_layout(layout_aci_boris);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( boris )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD("007-7020-01_c10617", 0x0000, 0x0800, CRC(dadf1693) SHA1(ffaef7a78f07dfcec9cc6e4034d665d188748225) )
	ROM_LOAD("007-7021-01_c10618", 0x0800, 0x0800, CRC(89b10faa) SHA1(b86cf42f93051b29f398691270e9a860b2978043) ) // identical halves
ROM_END

ROM_START( borisa )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD("007-7020-00_c10502", 0x0000, 0x0800, CRC(18182870) SHA1(cb717a4b5269b04b0d7ae61aaf4a8f6a019626a5) )
	ROM_LOAD("007-7021-00_c10503", 0x0800, 0x0800, CRC(4185d183) SHA1(43155493593d6f52a0f6906d4414f4eff3098c5f) ) // identical halves, less than 512 bytes used
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY, FULLNAME, FLAGS
SYST( 1978, boris,  0,      0,      boris,   boris, boris_state, empty_init, "Applied Concepts", "Boris (rev. 01)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW ) // "Boris awaits your move"
SYST( 1978, borisa, boris,  0,      boris,   boris, boris_state, empty_init, "Applied Concepts", "Boris (rev. 00)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW ) // "Boris plays black"
