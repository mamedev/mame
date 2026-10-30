// license:BSD-3-Clause
// copyright-holders:Robbbert, Mark Garlanger
/***************************************************************************

Heathkit ET-3400 Microprocessor Trainer

2009-05-12 Skeleton driver.
2016-04-29 Added Accessory.

ET-3400 (595-2021 manual)
- 6800 CPU clocked at about 500 kHz from an RC oscillator in the MC6875.
- 1K monitor ROM at FC00-FFFF, 256 bytes of RAM at 0000-00FF with sockets
  for another 256 bytes at 0100-01FF.
- Hex keypad read at C0xx, six 7-segment LEDs written one segment at a time
  at C1xx.

ET-3400A (595-2610 schematic)
- Redesign with an MC6808 (or MC6802) and a 4 MHz crystal on its own
  oscillator, so the CPU runs at 1 MHz without the accessory modification.
- 512 bytes of RAM in two 2114s with A9 grounded, at 0000-01FF.
- The monitor is a 2316 (444-76) with A10 used as a chip select, so only
  its upper half appears, at FC00-FFFF. That half matches the ET-3400 ROM
  and the lower half is blank.
- Reset is buffered by a 74LS241 instead of coming from the MC6875.

ETA-3400 Memory I/O Accessory
- Connects to the ET-3400A's 40-pin accessory connector, see
  bus/heathzenith/et3400/eta3400.cpp. An original ET-3400 needs the
  modification kit (597-1954) for it, which adds the 4 MHz crystal and the
  connector, so it is only offered on the ET-3400A.

Things to test
- The keypad keys are mapped to 0-9 and A-F, and RESET is mapped to '.'.
- After RESET the display shows "CPU UP.".
- Enter bytes: AUTO (A), a 4-digit address, then the bytes; the address
  advances on its own. Press RESET when done.
- Look at memory: EXAM (E) and an address, then FWD (F) or BACK (B) to
  move, and CHAN (C) to change the byte shown.
- Run a program: DO (D) and its address. This one prints 123456 using the
  monitor's REDIS (FCBC) and OUTBYT (FE20) routines and then halts; press
  RESET to return to the monitor:
    A 0000  BD FC BC 86 12 BD FE 20 86 34 BD FE 20 86 56 BD FE 20 3E
    D 0000
  End programs with WAI (3E). SWI (3F) only returns to the monitor while
  single-stepping; otherwise its vector at 00F4 is not set up.
- Single step: enter 86 42 C6 37 at 0000, then PC (3), CHAN (C), 0000 to
  set the PC, and SS (8) twice. ACCA (1) shows 42 and ACCB (2) shows 37.
- RAM size: load the program at 0100 instead. It runs on the ET-3400A or
  with "-ram 512"; with 256 bytes EXAM 0100 shows FF.


****************************************************************************/

#include "emu.h"

#include "bus/heathzenith/et3400/exp.h"
#include "cpu/m6800/m6800.h"
#include "machine/74259.h"
#include "machine/ram.h"

#include "et3400.lh"


namespace {

class et3400_state : public driver_device
{
public:
	et3400_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, RAM_TAG)
		, m_exp(*this, "exp")
		, m_displatch(*this, "displatch%u", 1)
		, m_x(*this, "X%u", 0U)
		, m_digit(*this, "digit%u", 1U)
	{ }

	void et3400(machine_config &config) ATTR_COLD;
	void et3400a(machine_config &config) ATTR_COLD;

	void reset_key_w(int state);
	void segment_test_w(int state);

protected:
	virtual void machine_start() override ATTR_COLD;

	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	optional_device<et3400_expansion_slot_device> m_exp;

private:
	u8 keypad_r(offs_t offset);
	void display_w(offs_t offset, u8 data);
	template <int Digit> void led_w(u8 data);

	required_device_array<ls259_device, 6> m_displatch;
	required_ioport_array<3> m_x;
	output_finder<6> m_digit;
};

void et3400_state::machine_start()
{
	m_maincpu->space(AS_PROGRAM).install_ram(0x0000, m_ram->size() - 1, m_ram->pointer());
}

u8 et3400_state::keypad_r(offs_t offset)
{
	u8 data = 0xff;

	if (~offset & 4) data &= m_x[2]->read();
	if (~offset & 2) data &= m_x[1]->read();
	if (~offset & 1) data &= m_x[0]->read();

	return data;
}

void et3400_state::display_w(offs_t offset, u8 data)
{
	// A6-A4 decoded by IC22 (74LS42); D0 inverted by one gate of IC21 (74S00)
	u8 digit = (offset >> 4) & 7;
	if (digit >= 1 && digit <= 6)
		m_displatch[digit - 1]->write_bit(offset & 7, !BIT(data, 0));
}

template <int Digit>
void et3400_state::led_w(u8 data)
{
	// This computer sets each segment, one at a time.
	m_digit[Digit - 1] = bitswap<8>(~data, 7, 0, 1, 2, 3, 4, 5, 6);
}

// RAM is installed in machine_start, sized by the RAM device
void et3400_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0xc000, 0xc0ff).r(FUNC(et3400_state::keypad_r));
	map(0xc100, 0xc1ff).w(FUNC(et3400_state::display_w));
	map(0xfc00, 0xffff).rom().region("monitor", 0);
}

void et3400_state::reset_key_w(int state)
{
	// delivered through MC6875 (or 74LS241 on ET-3400A)
	m_maincpu->set_input_line(INPUT_LINE_RESET, state ? CLEAR_LINE : ASSERT_LINE);

	if (m_exp)
		m_exp->reset_w(state);
}

void et3400_state::segment_test_w(int state)
{
	for (int d = 0; d < 6; d++)
		m_displatch[d]->clear_w(state);
}

/* Input ports */
static INPUT_PORTS_START( et3400 )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("D DO")    PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("A AUTO")  PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("7 RTI")   PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("4 INDEX") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("1 ACCA")  PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("0")       PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0xc0, 0xc0, IPT_UNUSED )

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("E EXAM")  PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("B BACK")  PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("8 SS")    PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("5 CC")    PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("2 ACCB")  PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0xe0, 0xe0, IPT_UNUSED )

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("F FWD")   PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("C CHAN")  PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("9 BR")    PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("6 SP")    PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("3 PC")    PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0xe0, 0xe0, IPT_UNUSED )

	PORT_START("RESET") // RESET is directly next to 0, but electrically separate from key matrix
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("RESET") PORT_CODE(KEYCODE_STOP) PORT_WRITE_LINE_MEMBER(FUNC(et3400_state::reset_key_w))

	PORT_START("TEST") // No input mechanism for "Segment Test" defined other than shorting pins together
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Segment Test") PORT_WRITE_LINE_MEMBER(FUNC(et3400_state::segment_test_w))
INPUT_PORTS_END

void et3400_state::et3400(machine_config &config)
{
	/* basic machine hardware */
	M6800(config, m_maincpu, 500'000); // approximate, set by an RC oscillator in the MC6875
	m_maincpu->set_addrmap(AS_PROGRAM, &et3400_state::mem_map);

	// 256 bytes at 0000, plus sockets for another 256
	RAM(config, m_ram).set_default_size("256").set_extra_options("512");

	/* video hardware */
	config.set_default_layout(layout_et3400);

	for (std::size_t i = 0; i < 6; i++)
		LS259(config, m_displatch[i]);

	m_displatch[0]->parallel_out_cb().set(FUNC(et3400_state::led_w<1>));
	m_displatch[1]->parallel_out_cb().set(FUNC(et3400_state::led_w<2>));
	m_displatch[2]->parallel_out_cb().set(FUNC(et3400_state::led_w<3>));
	m_displatch[3]->parallel_out_cb().set(FUNC(et3400_state::led_w<4>));
	m_displatch[4]->parallel_out_cb().set(FUNC(et3400_state::led_w<5>));
	m_displatch[5]->parallel_out_cb().set(FUNC(et3400_state::led_w<6>));
}

void et3400_state::et3400a(machine_config &config)
{
	et3400(config);

	M6808(config.replace(), m_maincpu, XTAL(4'000'000)); // MC6808P or MC6802P, divides by 4 internally
	m_maincpu->set_addrmap(AS_PROGRAM, &et3400_state::mem_map);

	// two 2114s with A9 grounded
	m_ram->set_default_size("512").set_extra_options("");

	ET3400_EXPANSION_SLOT(config, m_exp, et3400_expansion_cards, nullptr);
	m_exp->set_space(m_maincpu, AS_PROGRAM);
}

/* ROM definition */
ROM_START( et3400 )
	ROM_REGION( 0x0400, "monitor", 0 )
	ROM_LOAD( "et3400.ic12",  0x0000, 0x0400, CRC(2eff1f58) SHA1(38b655de7393d7a92b08276f7c14a99eaa2a4a9f) )
ROM_END

ROM_START( et3400a )
	// only the upper half (A10 high) is selected; it matches et3400.ic12
	ROM_REGION( 0x0400, "monitor", 0 )
	ROM_LOAD( "444-76.u12",   0x0000, 0x0400, CRC(a99031d3) SHA1(e34cf4a92e3732ea7e6292762d96e880bbc86f22) )
	ROM_CONTINUE(             0x0000, 0x0400 )
ROM_END

} // Anonymous namespace

/*    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY          FULLNAME                                          FLAGS */
COMP( 1976, et3400,  0,      0,      et3400,  et3400,  et3400_state,  empty_init, "Heath Company", "Heathkit Model ET-3400 Microprocessor Trainer",  MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 1981, et3400a, et3400, 0,      et3400a, et3400,  et3400_state,  empty_init, "Heath Company", "Heathkit Model ET-3400A Microprocessor Trainer", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
