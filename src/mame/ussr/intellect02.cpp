// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Alex_LG, Berger
/*******************************************************************************

Интеллект-02 (Intellect-02) driver

This is a Soviet electronic board game console, a dozen or so cartridge games were
announced(can't say for certain how many released). PCB labels have prefix ДМП,
it's assumed to have been designed by НИИ БРЭА (SRI BREA). First shown in 1983,
produced during around 1985-1992.

Hardware notes:
- КР580ВМ80А CPU (i8080A clone) @ 1.5MHz
- КР580ИК55 (i8255 clone)
- 1 KB RAM (8*КР565РУ2), cartridge port
- 4-digit 7seg panel, 2 leds, 16 buttons, game board above it

The chess/checkers board is detachable, with a board for Kalah underneath it.

The hardware is very similar to Fidelity Chess Challenger 3. Actually, one of the
first cartridges for this system, a Chess program, is a modified Chess Challenger 3 ROM.
The chess engine was kept identical. (note: the "lose" LED is used for "check" in this game)
The 2nd(4-level) chess cartridge is completely different, not a CC3 clone.

Intellect-01 looks like it didn't get further than a prototype. It was a dedicated
chess computer, probably a clone of CC3.

Keypad legend:

СБ - сброс (reset)
ВВ - ввод (input)
ВИ - выбор игры (game select)
СТ - стирание (erase)
ПП - просмотр позиции (view position)
УИ - уровень игры (game level)

*******************************************************************************/

#include "emu.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8255.h"
#include "sound/beep.h"
#include "video/pwm.h"

#include "softlist_dev.h"
#include "speaker.h"

// internal artwork
#include "intellect02.lh"


namespace {

class intel02_state : public driver_device
{
public:
	intel02_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ppi8255(*this, "ppi8255"),
		m_display(*this, "display"),
		m_beeper(*this, "beeper"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	// machine configs
	void intel02(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(reset_button);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<i8255_device> m_ppi8255;
	required_device<pwm_display_device> m_display;
	required_device<beep_device> m_beeper;
	required_ioport_array<2> m_inputs;

	u8 m_digit_data = 0;
	u8 m_led_select = 0;

	// address maps
	void main_map(address_map &map) ATTR_COLD;
	void main_io(address_map &map) ATTR_COLD;

	// I/O handlers
	void update_display();
	u8 input_r();
	void digit_w(u8 data);
	void control_w(u8 data);
};

void intel02_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_digit_data));
	save_item(NAME(m_led_select));
}

INPUT_CHANGED_MEMBER(intel02_state::reset_button)
{
	// reset button goes to 8080/8255 RESET pins simultaneously (via 7474, and also a maze of NAND gates to who knows where)
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE);
	if (newval)
		m_ppi8255->reset();
}



/*******************************************************************************
    I/O
*******************************************************************************/

// I8255 PPI

void intel02_state::update_display()
{
	m_display->matrix(m_led_select, m_digit_data);
}

u8 intel02_state::input_r()
{
	// d0-d3: buttons through a maze of logic gates
	// basically giving each button its own 4-bit scancode
	u8 data = count_leading_zeros_32(m_inputs[0]->read()) - 17;

	// d4: Vcc, d5-d7: buttons (direct)
	return data | (~m_inputs[1]->read() << 4 & 0xf0);
}

void intel02_state::digit_w(u8 data)
{
	// d0-d6: digit segment data, d7: N/C
	m_digit_data = bitswap<7>(data,0,1,2,3,4,5,6);
	update_display();
}

void intel02_state::control_w(u8 data)
{
	// d0-d5: select digit/leds
	m_led_select = data;
	update_display();

	// d6: N/C (it's a delay timer on CC3)

	// d7: enable beeper
	m_beeper->set_state(BIT(data, 7));
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void intel02_state::main_map(address_map &map)
{
	map(0x0000, 0x1fff).r("cartslot", FUNC(generic_slot_device::read_rom));
	map(0xf000, 0xf3ff).ram();
}

void intel02_state::main_io(address_map &map)
{
	map.global_mask(0xff);
	map(0xf4, 0xf7).rw(m_ppi8255, FUNC(i8255_device::read), FUNC(i8255_device::write));
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( intel02 )
	PORT_START("IN.0")
	PORT_BIT(0x0007, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME(u8"ПП (View Position)")
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME(u8"УИ (Game Level)")
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_CODE(KEYCODE_H) PORT_NAME("H8")
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_CODE(KEYCODE_G) PORT_NAME("G7")
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_CODE(KEYCODE_F) PORT_NAME("F6")
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_CODE(KEYCODE_E) PORT_NAME("E5")
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_CODE(KEYCODE_D) PORT_NAME("D4")
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_CODE(KEYCODE_C) PORT_NAME("C3")
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_CODE(KEYCODE_B) PORT_NAME("B2")
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_CODE(KEYCODE_A) PORT_NAME("A1")
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME(u8"ВВ (Input)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME(u8"ВИ (Game Select)")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME(u8"СТ (Erase)")

	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_CHANGED_MEMBER(DEVICE_SELF, intel02_state, reset_button, 0) PORT_NAME(u8"СБ (Reset)")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void intel02_state::intel02(machine_config &config)
{
	// basic machine hardware
	I8080A(config, m_maincpu, 1'500'000); // measured (no XTAL)
	m_maincpu->set_addrmap(AS_PROGRAM, &intel02_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &intel02_state::main_io);

	I8255(config, m_ppi8255);
	m_ppi8255->in_pa_callback().set(FUNC(intel02_state::input_r));
	m_ppi8255->out_pb_callback().set(FUNC(intel02_state::digit_w));
	m_ppi8255->tri_pb_callback().set_constant(0);
	m_ppi8255->out_pc_callback().set(FUNC(intel02_state::control_w));
	m_ppi8255->tri_pc_callback().set_constant(0x80);

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(6, 7);
	m_display->set_segmask(0xf, 0x7f);
	config.set_default_layout(layout_intellect02);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	BEEP(config, m_beeper, 3640); // measured, from RC circuit
	m_beeper->add_route(ALL_OUTPUTS, "speaker", 0.25);

	// cartridge
	GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "intellect02").set_must_be_loaded(true);
	SOFTWARE_LIST(config, "cart_list").set_original("intellect02");
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( intel02 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	// nothing here, it's on a cartridge
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY, FULLNAME, FLAGS
SYST( 1985, intel02, 0,      0,      intel02, intel02, intel02_state, empty_init, "BREA Research Institute", "Intellect-02", MACHINE_SUPPORTS_SAVE )
