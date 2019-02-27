// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    IBM PC/AT and PS/2 keyboard controllers

    IBM used Intel UPI-41 microcontrollers to implement bidirectional
    keyboard communication from the PC/AT and PS/2.  On the PS/2, the
    microcontroller also handles mouse communication.  The PS/2 host
    interface to the keyboard/mouse controller is backwards-compatible
    for documented commands.  However a number of undocumented commands
    are no longer implemented, the pin assignments are different, and
    interrupt outputs are latched externally using a 74ALS74 at ZM87.

    PC/AT I/O pin assignments:
    P10:    NC              no connection
    P11:    NC              no connection
    P12:    NC              no connection
    P13:    NC              no connection
    P14:    -RAM SEL        enable external RAM         low disables
    P15:                    manufacturing setting       low disables
    P16:    SW1             CRT adapter switch          low for CGA
    P17:    +KBD INH        inhibit keyboard            active low
    P20:    RC              reset CPU                   active low
    P21:    A20 GATE        enable A20 line             active high
    P22:    NC              no connection
    P23:    NC              no connection
    P24:    +OPT BUF FULL   output buffer full          active high
    P25:    NC              no connection
    P26:                    open collector KBD CLK      inverted
    P27:                    open collector KBD DATA     not inverted
    TEST0:  KBD CLK                                     not inverted
    TEST1:  KBD DATA                                    not inverted

    PS/2 I/O pin assignments
    P10:    KYBD DATA                                   not inverted
    P11:    MOUSE DATA                                  not inverted
    P12:                    keyboard/mouse power fuse   low if blown
    P13:                    no connection
    P14:                    no connection
    P15:                    no connection
    P16:                    no connection
    P17:                    no connection
    P20:    +HOT RES        reset CPU                   inverted
    P21:    A20 PASS        enable A20 line             not inverted
    P22:                    open collector MOUSE DATA   inverted
    P23:                    open collector MOUSE CLK    inverted
    P24:                    latched as BIRQ1            rising trigger
    P25:                    latched as IRQ12            rising trigger
    P26:                    open collector KYBD CLK     inverted
    P27:                    open collector KYBD DATA    inverted
    TEST0:  KYBD CLK                                    not inverted
    TEST1:  MOUSE CLK                                   not inverted

    Notes:
    * PS/2 BIRQ1 and IRQ12 are cleard by reading data.
    * PS/2 BIRQ1 and IRQ12 are active low, but we use ASSERT_LINE here.
    * PS/2 IRQ12 is open collector for sharing with cards.

    TODO:
    * Move display type and key lock controls to drivers.
    * Expose power fuse failure.

***************************************************************************/

#include "emu.h"
#include "at_keybc.h"

namespace {

ROM_START(at_kbc)
	ROM_REGION(0x0800, "mcu", 0)
	ROM_SYSTEM_BIOS(0, "ibm", "IBM 1983") // 1983 IBM controller BIOS
	ROMX_LOAD("1503033.bin", 0x0000, 0x0800, CRC(5a81c0d2) SHA1(0100f8789fb4de74706ae7f9473a12ec2b9bd729), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "ptl", "PTL 1986") // unknown controller BIOS, (c) 1985, 1986 PTL
	ROMX_LOAD("yan25d05.bin", 0x0000, 0x0800, CRC(70c798f1) SHA1(ae9a79c7184a17331b70a50035ff63c757df094c), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "award15", "Award 1985 V1.5") // MIPS Rx2030 i8742 keyboard controller
	ROMX_LOAD("keyboard_1.5.bin", 0x0000, 0x0800, CRC(f86ba0f7) SHA1(1ad475451db35a76d929824c035d582279fbe3a3), ROM_BIOS(2))
ROM_END

ROM_START(ps2_kbc)
	ROM_REGION(0x0800, "mcu", 0)
	ROM_LOAD("72x8455.zm82", 0x0000, 0x0800, CRC(7da223d3) SHA1(54c52ff6c6a2310f79b2c7e6d1259be9de868f0e))
ROM_END

INPUT_PORTS_START(at_kbc)
	PORT_START("P1")
	PORT_BIT(0x3f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_CONFNAME(0x40, 0x00, "CRT Adapter Switch")
	PORT_CONFSETTING(   0x40, "Monochrome Display Adapter")
	PORT_CONFSETTING(   0x00, "Color Graphics Adapter")
	PORT_CONFNAME(0x80, 0x80, "Key Lock")
	PORT_CONFSETTING(   0x80, DEF_STR(Off))
	PORT_CONFSETTING(   0x00, DEF_STR(On))
INPUT_PORTS_END

} // anonymous namespace



//**************************************************************************
//  DEVICE TYPES
//**************************************************************************

DEFINE_DEVICE_TYPE(AT_KEYBOARD_CONTROLLER, at_keyboard_controller_device, "at_keybc", "PC/AT Keyboard Controller")
DEFINE_DEVICE_TYPE(PS2_KEYBOARD_CONTROLLER, ps2_keyboard_controller_device, "ps2_keybc", "PS/2 Keyboard/Mouse Controller")


//**************************************************************************
//  KEYBOARD CONTROLLER DEVICE BASE
//**************************************************************************

READ8_MEMBER(at_kbc_device_base::data_r)
{
	return m_mcu->upi41_master_r(space, 0U);
}

READ8_MEMBER(at_kbc_device_base::status_r)
{
	return m_mcu->upi41_master_r(space, 1U);
}

WRITE8_MEMBER(at_kbc_device_base::data_w)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(at_kbc_device_base::write_data), this), unsigned(data));
}

WRITE8_MEMBER(at_kbc_device_base::command_w)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(at_kbc_device_base::write_command), this), unsigned(data));
}

WRITE_LINE_MEMBER(at_kbc_device_base::kbd_clk_w)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(at_kbc_device_base::set_kbd_clk_in), this), state);
}

WRITE_LINE_MEMBER(at_kbc_device_base::kbd_data_w)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(at_kbc_device_base::set_kbd_data_in), this), state);
}

at_kbc_device_base::at_kbc_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_mcu(*this, "mcu")
	, m_hot_res_cb(*this), m_gate_a20_cb(*this), m_kbd_irq_cb(*this)
	, m_kbd_clk_cb(*this), m_kbd_data_cb(*this)
	, m_hot_res(0U), m_gate_a20(0U), m_kbd_irq(0U)
	, m_kbd_clk_in(1U), m_kbd_clk_out(1U), m_kbd_data_in(1U), m_kbd_data_out(1U)
{
}

void at_kbc_device_base::device_resolve_objects()
{
	m_hot_res_cb.resolve_safe();
	m_gate_a20_cb.resolve_safe();
	m_kbd_irq_cb.resolve_safe();
	m_kbd_clk_cb.resolve_safe();
	m_kbd_data_cb.resolve_safe();
}

void at_kbc_device_base::device_start()
{
	save_item(NAME(m_hot_res));
	save_item(NAME(m_gate_a20));
	save_item(NAME(m_kbd_irq));
	save_item(NAME(m_kbd_clk_in));
	save_item(NAME(m_kbd_clk_out));
	save_item(NAME(m_kbd_data_in));
	save_item(NAME(m_kbd_data_out));

	m_hot_res = m_gate_a20 = m_kbd_irq = 0U;
	m_kbd_clk_in = m_kbd_clk_out = 1U;
	m_kbd_data_in = m_kbd_data_out = 1U;
}

inline void at_kbc_device_base::set_hot_res(u8 state)
{
	if (state != m_hot_res)
		m_hot_res_cb((m_hot_res = state) ? ASSERT_LINE : CLEAR_LINE);
}

inline void at_kbc_device_base::set_gate_a20(u8 state)
{
	if (state != m_gate_a20)
		m_gate_a20_cb((m_gate_a20 = state) ? ASSERT_LINE : CLEAR_LINE);
}

inline void at_kbc_device_base::set_kbd_irq(u8 state)
{
	if (state != m_kbd_irq)
		m_kbd_irq_cb((m_kbd_irq = state) ? ASSERT_LINE : CLEAR_LINE);
}

inline void at_kbc_device_base::set_kbd_clk_out(u8 state)
{
	if (state != m_kbd_clk_out)
		m_kbd_clk_cb(m_kbd_clk_out = state);
}

inline void at_kbc_device_base::set_kbd_data_out(u8 state)
{
	if (state != m_kbd_data_out)
		m_kbd_data_cb(m_kbd_data_out = state);
}

inline u8 at_kbc_device_base::kbd_clk_r() const
{
	return m_kbd_clk_in & m_kbd_clk_out;
}

inline u8 at_kbc_device_base::kbd_data_r() const
{
	return m_kbd_data_in & m_kbd_data_out;
}

TIMER_CALLBACK_MEMBER(at_kbc_device_base::write_data)
{
	m_mcu->upi41_master_w(machine().dummy_space(), 0U, u8(u32(param)));
}

TIMER_CALLBACK_MEMBER(at_kbc_device_base::write_command)
{
	m_mcu->upi41_master_w(machine().dummy_space(), 1U, u8(u32(param)));
}

TIMER_CALLBACK_MEMBER(at_kbc_device_base::set_kbd_clk_in)
{
	m_kbd_clk_in = param ? 1U : 0U;
}

TIMER_CALLBACK_MEMBER(at_kbc_device_base::set_kbd_data_in)
{
	m_kbd_data_in = param ? 1U : 0U;
}


//**************************************************************************
//  PC/AT KEYBOARD CONTROLLER DEVICE
//**************************************************************************

at_keyboard_controller_device::at_keyboard_controller_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: at_kbc_device_base(mconfig, AT_KEYBOARD_CONTROLLER, tag, owner, clock)
{
}

tiny_rom_entry const *at_keyboard_controller_device::device_rom_region() const
{
	return ROM_NAME(at_kbc);
}

void at_keyboard_controller_device::device_add_mconfig(machine_config &config)
{
	I8042(config, m_mcu, DERIVED_CLOCK(1, 1));
	m_mcu->p1_in_cb().set_ioport("P1");
	m_mcu->p1_out_cb().set_nop();
	m_mcu->p2_in_cb().set_constant(0xffU);
	m_mcu->p2_out_cb().set(FUNC(at_keyboard_controller_device::p2_w));
	m_mcu->t0_in_cb().set([this] () { return kbd_clk_r(); });
	m_mcu->t1_in_cb().set([this] () { return kbd_data_r(); });
}

ioport_constructor at_keyboard_controller_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(at_kbc);
}

WRITE8_MEMBER(at_keyboard_controller_device::p2_w)
{
	set_hot_res(BIT(~data, 0));
	set_gate_a20(BIT(data, 1));
	set_kbd_irq(BIT(data, 4));
	set_kbd_clk_out(BIT(~data, 6));
	set_kbd_data_out(BIT(data, 7));
}


//**************************************************************************
//  PS/2 KEYBOARD/MOUSE CONTROLLER DEVICE
//**************************************************************************

READ8_MEMBER(ps2_keyboard_controller_device::data_r)
{
	set_kbd_irq(0U);
	set_mouse_irq(0U);
	return m_mcu->upi41_master_r(space, 0U);
}

WRITE_LINE_MEMBER(ps2_keyboard_controller_device::mouse_clk_w)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(ps2_keyboard_controller_device::set_mouse_clk_in), this), state);
}

WRITE_LINE_MEMBER(ps2_keyboard_controller_device::mouse_data_w)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(ps2_keyboard_controller_device::set_mouse_data_in), this), state);
}

ps2_keyboard_controller_device::ps2_keyboard_controller_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: at_kbc_device_base(mconfig, PS2_KEYBOARD_CONTROLLER, tag, owner, clock)
	, m_mouse_irq_cb(*this)
	, m_mouse_clk_cb(*this), m_mouse_data_cb(*this)
	, m_mouse_irq(0U)
	, m_mouse_clk_in(1U), m_mouse_clk_out(1U), m_mouse_data_in(1U), m_mouse_data_out(1U)
	, m_p2_data(0xffU)
{
}

tiny_rom_entry const *ps2_keyboard_controller_device::device_rom_region() const
{
	return ROM_NAME(ps2_kbc);
}

void ps2_keyboard_controller_device::device_add_mconfig(machine_config &config)
{
	I8042(config, m_mcu, DERIVED_CLOCK(1, 1));
	m_mcu->p1_in_cb().set(FUNC(ps2_keyboard_controller_device::p1_r));
	m_mcu->p1_out_cb().set_nop();
	m_mcu->p2_in_cb().set_constant(0xffU);
	m_mcu->p2_out_cb().set(FUNC(ps2_keyboard_controller_device::p2_w));
	m_mcu->t0_in_cb().set([this] () { return kbd_clk_r(); });
	m_mcu->t1_in_cb().set([this] () { return mouse_clk_r(); });
}

void ps2_keyboard_controller_device::device_resolve_objects()
{
	at_kbc_device_base::device_resolve_objects();

	m_mouse_clk_cb.resolve_safe();
	m_mouse_data_cb.resolve_safe();
}

void ps2_keyboard_controller_device::device_start()
{
	at_kbc_device_base::device_start();

	save_item(NAME(m_mouse_irq));
	save_item(NAME(m_mouse_clk_in));
	save_item(NAME(m_mouse_clk_out));
	save_item(NAME(m_mouse_data_in));
	save_item(NAME(m_mouse_data_out));
	save_item(NAME(m_p2_data));

	m_mouse_irq = 0U;
	m_mouse_clk_in = m_mouse_clk_out = 1U;
	m_mouse_data_in = m_mouse_data_out = 1U;
	m_p2_data = 0xffU;
}

inline void ps2_keyboard_controller_device::set_mouse_irq(u8 state)
{
	if (state != m_mouse_irq)
		m_mouse_irq_cb((m_mouse_irq = state) ? ASSERT_LINE : CLEAR_LINE);
}

inline void ps2_keyboard_controller_device::set_mouse_clk_out(u8 state)
{
	if (state != m_mouse_clk_out)
		m_mouse_clk_cb(m_mouse_clk_out = state);
}

inline void ps2_keyboard_controller_device::set_mouse_data_out(u8 state)
{
	if (state != m_mouse_data_out)
		m_mouse_data_cb(m_mouse_data_out = state);
}

inline u8 ps2_keyboard_controller_device::mouse_clk_r() const
{
	return m_mouse_clk_in & m_mouse_clk_out;
}

inline u8 ps2_keyboard_controller_device::mouse_data_r() const
{
	return m_mouse_data_in & m_mouse_data_out;
}

TIMER_CALLBACK_MEMBER(ps2_keyboard_controller_device::set_mouse_clk_in)
{
	m_mouse_clk_in = param ? 1U : 0U;
}

TIMER_CALLBACK_MEMBER(ps2_keyboard_controller_device::set_mouse_data_in)
{
	m_mouse_data_in = param ? 1U : 0U;
}

READ8_MEMBER(ps2_keyboard_controller_device::p1_r)
{
	return kbd_data_r() | (mouse_data_r() << 1) | 0xfcU;
}

WRITE8_MEMBER(ps2_keyboard_controller_device::p2_w)
{
	set_hot_res(BIT(~data, 0));
	set_gate_a20(BIT(data, 1));
	set_mouse_data_out(BIT(~data, 2));
	set_mouse_clk_out(BIT(~data, 3));
	set_kbd_clk_out(BIT(~data, 6));
	set_kbd_data_out(BIT(~data, 7));

	if (BIT(data & ~m_p2_data, 4))
		set_kbd_irq(1U);
	if (BIT(data & ~m_p2_data, 5))
		set_mouse_irq(1U);
	m_p2_data = data;
}
