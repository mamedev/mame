// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle, David Viens, Kevin Horton
/*******************************************************************************

Tiger Electronics K28 (model 7-232) Sold in Hong Kong, distributed in US as:
- Coleco: Talking Teacher
- Sears: Talkatron: Learning Computer

1981 K28 models 7-230 and 7-231 are on different hardware, showing a different
keyboard, VFD, and use the SC-01 speech chip. --> driver k28.cpp

Hardware notes:
- MCU: TMS1400 MP7324 (die label: TMS1400, MP7324, 28L 01D D000 R100)
- TMS51xx: TMS5110A
- VSM: 16KB CM62084
- LCD: SMOS SMC1112 MCU to 8*14-seg display
- module slot (not compatible with the 1981 version(s))

TODO:
- emulate LCD
- dump/add module #5

*******************************************************************************/

#include "emu.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "cpu/tms1000/tms1400.h"
#include "machine/tms6100.h"
#include "sound/tms5110.h"

#include "softlist_dev.h"
#include "speaker.h"

// internal artwork
#include "k28m2.lh"


namespace {

class k28m2_state : public driver_device
{
public:
	k28m2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_tms5100(*this, "tms5100"),
		m_tms6100(*this, "tms6100"),
		m_cart(*this, "cartslot"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void k28m2(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(power_on);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	// devices/pointers
	required_device<tms1400_cpu_device> m_maincpu;
	required_device<tms5110_device> m_tms5100;
	required_device<tms6100_device> m_tms6100;
	optional_device<generic_slot_device> m_cart;
	required_ioport_array<9> m_inputs;

	u8 read_k();
	void write_o(u16 data);
	void write_r(u32 data);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);
	void power_off();

	bool m_power_on = false;
	u16 m_inp_mux = 0;
	u32 m_r = 0;
};

void k28m2_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_power_on));
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_r));
}



/*******************************************************************************
    Power
*******************************************************************************/

void k28m2_state::machine_reset()
{
	m_power_on = true;
	m_maincpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
}

INPUT_CHANGED_MEMBER(k28m2_state::power_on)
{
	if (newval && !m_power_on)
		machine_reset();
}

void k28m2_state::power_off()
{
	m_power_on = false;
	m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}



/*******************************************************************************
    Cartridge
*******************************************************************************/

DEVICE_IMAGE_LOAD_MEMBER(k28m2_state::cart_load)
{
	u32 size = m_cart->common_get_size("rom");

	if (size > 0x4000)
	{
		image.seterror(image_error::INVALIDIMAGE, "Invalid file size");
		return image_init_result::FAIL;
	}

	u8 *base = memregion("tms6100")->base() + 0x8000;
	m_cart->common_load_rom(base, size, "rom");

	return image_init_result::PASS;
}



/*******************************************************************************
    I/O
*******************************************************************************/

void k28m2_state::write_r(u32 data)
{
	// R1234: TMS5100 CTL8421
	u32 r = bitswap<5>(data,0,1,2,3,4) | (data & ~0x1f);
	m_tms5100->ctl_w(r & 0xf);

	// R0: TMS5100 PDC pin
	m_tms5100->pdc_w(data & 1);

	// R5: input mux high bit
	m_inp_mux = (m_inp_mux & 0xff) | (data << 3 & 0x100);

	// R7-R10: LCD data
	//TODO..

	// R6: power-off request, on falling edge
	if (~data & m_r & 0x40)
		power_off();

	m_r = r;
}

void k28m2_state::write_o(u16 data)
{
	// O0-O7: input mux low
	m_inp_mux = (m_inp_mux & ~0xff) | data;
}

u8 k28m2_state::read_k()
{
	u8 data = 0;

	// K: multiplexed inputs
	for (int i = 0; i < 9; i++)
		if (BIT(m_inp_mux, i))
			data |= m_inputs[i]->read();

	// and TMS5100 CTL (also tied to R1234)
	return data | m_tms5100->ctl_r() | (m_r & 0xf);
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( k28m2 )
	PORT_START("IN.0") // O0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2) PORT_NAME("Off") // -> auto_power_off
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_CHAR('A') PORT_NAME("A/1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_CHAR('J') PORT_NAME("J/0")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S')

	PORT_START("IN.1") // O1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F1) PORT_NAME("On") PORT_CHANGED_MEMBER(DEVICE_SELF, k28m2_state, power_on, 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_CHAR('B') PORT_NAME("B/2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR('K') PORT_NAME("K/+")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T')

	PORT_START("IN.2") // O2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("Repeat")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_CHAR('C') PORT_NAME("C/3")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR('L') PORT_NAME("L/-")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U')

	PORT_START("IN.3") // O3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("Prompt")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_CHAR('D') PORT_NAME("D/4")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR('M') PORT_NAME(u8"M/×")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V')

	PORT_START("IN.4") // O4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_HOME) PORT_NAME("Menu")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_CHAR('E') PORT_NAME("E/5")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CODE(KEYCODE_SLASH_PAD) PORT_CHAR('N') PORT_NAME(u8"N/÷")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W')

	PORT_START("IN.5") // O5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_END) PORT_NAME("Module")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_CHAR('F') PORT_NAME("F/6")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X')

	PORT_START("IN.6") // O6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE) PORT_NAME("Select")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_CHAR('G') PORT_NAME("G/7")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')

	PORT_START("IN.7") // O7
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Clear")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_CHAR('H') PORT_NAME("H/8")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')

	PORT_START("IN.8") // R5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_CHAR('I') PORT_NAME("I/9")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Enter")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void k28m2_state::k28m2(machine_config &config)
{
	static constexpr u32 MASTER_CLOCK = 640'000; // approximation

	// basic machine hardware
	TMS1400(config, m_maincpu, MASTER_CLOCK/2);
	m_maincpu->read_k().set(FUNC(k28m2_state::read_k));
	m_maincpu->write_o().set(FUNC(k28m2_state::write_o));
	m_maincpu->write_r().set(FUNC(k28m2_state::write_r));

	config.set_default_layout(layout_k28m2);

	// sound hardware
	TMS6100(config, m_tms6100, MASTER_CLOCK/4);

	SPEAKER(config, "mono").front_center();
	TMS5110A(config, m_tms5100, MASTER_CLOCK);
	m_tms5100->m0().set(m_tms6100, FUNC(tms6100_device::m0_w));
	m_tms5100->m1().set(m_tms6100, FUNC(tms6100_device::m1_w));
	m_tms5100->addr().set(m_tms6100, FUNC(tms6100_device::add_w));
	m_tms5100->data().set(m_tms6100, FUNC(tms6100_device::data_line_r));
	m_tms5100->romclk().set(m_tms6100, FUNC(tms6100_device::clk_w));
	m_tms5100->add_route(ALL_OUTPUTS, "mono", 0.5);

	// cartridge
	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "k28m2", "vsm,bin");
	m_cart->set_device_load(FUNC(k28m2_state::cart_load));

	SOFTWARE_LIST(config, "cart_list").set_original("k28m2");
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( k28m2 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mp7324", 0x0000, 0x1000, CRC(08d15ab6) SHA1(5b0f6c53e6732a362c4bb25d966d4072fdd33db8) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common1_micro.pla", 0, 867, CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) )
	ROM_REGION( 557, "maincpu:opla", 0 )
	ROM_LOAD( "tms1400_k28m2_output.pla", 0, 557, CRC(3a5c7005) SHA1(3fe5819c138a90e7fc12817415f2622ca81b40b2) )

	ROM_REGION( 0x10000, "tms6100", ROMREGION_ERASEFF ) // 8000-bfff? = space reserved for cartridge
	ROM_LOAD( "cm62084.vsm", 0x0000, 0x4000, CRC(cd1376f7) SHA1(96fa484c392c451599bc083b8376cad9c998df7d) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME    PARENT  CMP MACHINE  INPUT  CLASS        INIT        COMPANY, FULLNAME, FLAGS
COMP( 1985, k28m2,  0,       0, k28m2,   k28m2, k28m2_state, empty_init, "Tiger Electronics", "K28: Talking Learning Computer (model 7-232)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
