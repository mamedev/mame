// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle
/*******************************************************************************

Lexicon LK-3000, a pocket computer.
It was also licensed to Nixdorf.

Hardware notes:
- 3870 MCU (Motorola brand) @ 4MHz(2MHz internal)
- optional external ROM or RAM
- 4*Litronix DL1414 (16*17segs total)
- 33-button keypad

The CPU and memory is on the cartridge. In theory, any hardware configuration
is possible, but it looks like they always used a 3870.

cartridge types known:
- CPU + 4/8KB ROM (ROM may be unpopulated)
- CPU + 2*4KB ROM
- CPU + 1KB battery-backed RAM (2*NEC D444)

TODO:
- external ram should be battery-backed

*******************************************************************************/

#include "emu.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "cpu/f8/f8.h"
#include "machine/f3853.h"
#include "video/dl1416.h"

#include "softlist_dev.h"

// internal artwork
#include "lk3000.lh"


namespace {

class lk3000_state : public driver_device
{
public:
	lk3000_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_dl1414(*this, "dl1414_%u", 0),
		m_cart(*this, "cartslot"),
		m_inputs(*this, "IN.%u", 0),
		m_digits(*this, "digit%u", 0U)
	{ }

	void lk3000(machine_config &config);

	// CLR button is tied to MCU RESET pin
	DECLARE_INPUT_CHANGED_MEMBER(reset_button) { m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE); }

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device_array<dl1414_device, 4> m_dl1414;
	required_device<generic_slot_device> m_cart;
	required_ioport_array<8> m_inputs;
	output_finder<16> m_digits;

	u8 m_p0 = 0;
	u8 m_p1 = 0;
	u8 m_p4 = 0;
	u8 m_p5 = 0;

	bool m_has_ram = false;
	u8 m_ram[0x400];

	void main_map(address_map &map) ATTR_COLD;
	void main_io(address_map &map) ATTR_COLD;

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);
	template <int N> void update_display(offs_t offset, u16 data);

	u8 p0_r();
	void p0_w(u8 data);
	u8 p1_r();
	void p1_w(u8 data);
	u8 p4_r();
	void p4_w(u8 data);
	u8 p5_r();
	void p5_w(u8 data);
};

void lk3000_state::machine_start()
{
	m_digits.resolve();
	memset(m_ram, 0xff, 0x400);

	// register for savestates
	save_item(NAME(m_p0));
	save_item(NAME(m_p1));
	save_item(NAME(m_p4));
	save_item(NAME(m_p5));
	save_item(NAME(m_has_ram));
	save_item(NAME(m_ram));
}



/*******************************************************************************
    I/O
*******************************************************************************/

// misc

DEVICE_IMAGE_LOAD_MEMBER(lk3000_state::cart_load)
{
	u32 size = m_cart->common_get_size("rom");
	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	// extra ram (optional)
	m_has_ram = image.get_feature("ram") != nullptr;

	return std::make_pair(std::error_condition(), std::string());
}

template <int N>
void lk3000_state::update_display(offs_t offset, u16 data)
{
	m_digits[N * 4 + offset] = data;
}


// 3870 ports

u8 lk3000_state::p0_r()
{
	return m_p0;
}

void lk3000_state::p0_w(u8 data)
{
	m_p0 = data;
}

u8 lk3000_state::p1_r()
{
	u8 data = m_p1;

	// P10-P13: multiplexed inputs
	if (m_p4 & 0x10)
		data |= m_inputs[~m_p4 & 7]->read();

	// read rom data
	if (m_p0 & 0x80)
	{
		// P00-P06: A0-A6, P50-P54: A7-A11, P55 selects chip
		u16 offset = (~m_p0 & 0x7f) | (~m_p5 << 7 & 0x1f80);
		data |= ~m_cart->read_rom(offset + 0x800);
	}

	// read ram data
	if (m_has_ram && m_p4 & 0xc0)
	{
		u16 offset = (m_p0 & 0xff) | (m_p5 << 8 & 0x300);
		data |= m_ram[offset];
	}

	return data;
}

void lk3000_state::p1_w(u8 data)
{
	// P10-P15,_P15: DL1414 data
	for (int i = 0; i < 4; i++)
		m_dl1414[i]->data_w((~data & 0x3f) | (data << 1 & 0x40));

	m_p1 = data;
}

u8 lk3000_state::p4_r()
{
	return m_p4;
}

void lk3000_state::p4_w(u8 data)
{
	// P40,P41: DL1414 address
	for (int i = 0; i < 4; i++)
		m_dl1414[i]->addr_w(~data & 3);

	// P42,P43,GND,P45: 7442 to DL1414 _WR
	for (int i = 0; i < 4; i++)
		m_dl1414[i]->wr_w(BIT(1 << (~data >> 2 & 0xb), i));

	// P46: write ram data
	if (m_has_ram && ~data & m_p4 & 0x40)
	{
		u16 offset = (m_p0 & 0xff) | (m_p5 << 8 & 0x300);
		m_ram[offset] = m_p1;
	}

	// P40,P41,P42,P44: 7442 to keypad mux
	m_p4 = data;
}

u8 lk3000_state::p5_r()
{
	return m_p5;
}

void lk3000_state::p5_w(u8 data)
{
	m_p5 = data;
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void lk3000_state::main_map(address_map &map)
{
	map.global_mask(0x7ff);
	map(0x0000, 0x07ff).r("cartslot", FUNC(generic_slot_device::read_rom));
}

void lk3000_state::main_io(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(lk3000_state::p0_r), FUNC(lk3000_state::p0_w));
	map(0x01, 0x01).rw(FUNC(lk3000_state::p1_r), FUNC(lk3000_state::p1_w));
	map(0x04, 0x07).rw("psu", FUNC(f38t56_device::read), FUNC(f38t56_device::write));
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( lk3000 )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_NAME("A / MET")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_NAME("B / US")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_NAME("C / X>M")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_NAME("D / K")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_NAME("J / C1")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_NAME("K / C2")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_NAME("L / RM")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_NAME("M / %")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_NAME("S / EXC")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_NAME("T / +/-")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_NAME("U / M+")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("V / 0")

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_CHAR('e') PORT_NAME("E / 7")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_CHAR('f') PORT_NAME("F / 8")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_CHAR('g') PORT_NAME("G / 9")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_SLASH_PAD) PORT_CHAR('h') PORT_NAME(u8"H / ÷")

	PORT_START("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_CHAR('n') PORT_NAME("N / 4")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_CHAR('o') PORT_NAME("O / 5")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_CHAR('p') PORT_NAME("P / 6")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR('q') PORT_NAME("Q / +")

	PORT_START("IN.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_CHAR('w') PORT_NAME("W / 1")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_CHAR('x') PORT_NAME("X / 2")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_CHAR('y') PORT_NAME("Y / 3")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CODE(KEYCODE_STOP) PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR('z') PORT_NAME("Z / .")

	PORT_START("IN.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('?') PORT_NAME("? / P2")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8) PORT_NAME("bs / P1")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR('r') PORT_NAME("R / -")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR('i') PORT_NAME(u8"I / ×")

	PORT_START("IN.7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13) PORT_NAME("def / P4")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t') PORT_NAME("stp / P3")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(' ') PORT_NAME("sp / =")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_NAME("f")

	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL) PORT_CHANGED_MEMBER(DEVICE_SELF, lk3000_state, reset_button, 0) PORT_CHAR(127) PORT_NAME("clr")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void lk3000_state::lk3000(machine_config &config)
{
	// basic machine hardware
	F8(config, m_maincpu, 4_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &lk3000_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &lk3000_state::main_io);
	m_maincpu->set_irq_acknowledge_callback("psu", FUNC(f38t56_device::int_acknowledge));

	auto &psu(F38T56(config, "psu", 4_MHz_XTAL/2));
	psu.set_int_vector(0x20);
	psu.int_req_callback().set_inputline("maincpu", F8_INPUT_LINE_INT_REQ);
	psu.read_a().set(FUNC(lk3000_state::p4_r));
	psu.write_a().set(FUNC(lk3000_state::p4_w));
	psu.read_b().set(FUNC(lk3000_state::p5_r));
	psu.write_b().set(FUNC(lk3000_state::p5_w));

	// video hardware
	DL1414T(config, m_dl1414[0], 0U).update().set(FUNC(lk3000_state::update_display<0>));
	DL1414T(config, m_dl1414[1], 0U).update().set(FUNC(lk3000_state::update_display<1>));
	DL1414T(config, m_dl1414[2], 0U).update().set(FUNC(lk3000_state::update_display<2>));
	DL1414T(config, m_dl1414[3], 0U).update().set(FUNC(lk3000_state::update_display<3>));
	config.set_default_layout(layout_lk3000);

	// cartridge
	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "lk3000");
	m_cart->set_device_load(FUNC(lk3000_state::cart_load));
	m_cart->set_must_be_loaded(true);

	SOFTWARE_LIST(config, "cart_list").set_original("lk3000");
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( lk3000 )
	ROM_REGION( 0x0800, "maincpu", ROMREGION_ERASE00 )
	// none here, it's in the module slot
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY, FULLNAME, FLAGS
SYST( 1979, lk3000, 0,      0,      lk3000,  lk3000, lk3000_state, empty_init, "Lexicon", "LK-3000", MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
