// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Preliminary driver for DGP-80 Z80 trainer.

    Monitor command list:

        (B) INS   Insert Values into Memory
        (C) DSP   Display Memory
        (D) INC   Increment Address
        (E) DEC   Decrement Address
        (9) SUB   Substitute Memory Value
        (F) EXEC  Execute Program
        (A) XRG   Examine Register
        (8) XRG'  Examine Secondary Register
        (6) IN    Input from Port
        (7) OUT   Output to Port

    The Reset button is necessary to terminate most commands.

    All I/O lines of the 8255 & 8251 peripherals are tied to (currently
    unemulated) parallel and serial expansion connectors.

***************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/i8251.h"
#include "machine/i8255.h"

#include "dgp80.lh"

namespace {

class dgp80_state : public driver_device
{
public:
	dgp80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_i8251(*this, "i8251")
		, m_i8255(*this, "i8255")
		, m_keys(*this, "KEY%u", 1U)
		, m_display(*this, "dg%u", 1U)
	{
	}

	void dgp80(machine_config &config) ATTR_COLD;

	void reset_w(int state);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void display_w(u8 data);
	void scan_w(u8 data);
	u8 keyboard_r();

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	required_device<z80_device> m_maincpu;
	required_device<i8251_device> m_i8251;
	required_device<i8255_device> m_i8255;

	required_ioport_array<2> m_keys;
	output_finder<6> m_display;

	u8 m_scan = 0xff;
	u8 m_display_data = 0xff;
};

void dgp80_state::machine_start()
{
	save_item(NAME(m_scan));
	save_item(NAME(m_display_data));
}

void dgp80_state::display_w(u8 data)
{
	// Display data output via 7475 latches
	m_display_data = data;
}

void dgp80_state::scan_w(u8 data)
{
	// Keyboard and display scanning output via 74174 hex flip-flop
	m_scan = data | 0xc0;

	for (int i = 0; i < 6; i++)
		if (!BIT(data, 5 - i))
			m_display[i] = u8(~m_display_data);
}

u8 dgp80_state::keyboard_r()
{
	// Keyboard input via 74125 buffers
	u8 input = 0xff;

	for (int i = 0; i < 2; i++)
		if (!BIT(m_scan, 5 - i))
			input &= m_keys[i]->read();

	return input;
}

void dgp80_state::reset_w(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, state ? CLEAR_LINE : ASSERT_LINE);
	if (!state)
	{
		m_i8251->reset();
		m_i8255->reset();
	}
}

void dgp80_state::mem_map(address_map &map)
{
	map(0x0000, 0x0fff).rom().region("monitor", 0); // 2732 EPROM
	// 1000H-1FFFH: /MMS1
	map(0x2000, 0x27ff).mirror(0x800).ram(); // 6116 RAM
	// 3000H-3FFFH: /MMS3
	// 4000H-4FFFH: /MMS4
	// 5000H-5FFFH: /MMS5
	// 6000H-6FFFH: /MMS6
	// 7000H-7FFFH: /MMS7
}

void dgp80_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).mirror(0x0f).w(FUNC(dgp80_state::display_w));
	map(0x10, 0x10).mirror(0x0f).w(FUNC(dgp80_state::scan_w));
	map(0x20, 0x20).mirror(0x0f).r(FUNC(dgp80_state::keyboard_r));
	// 30H-3FH: /IOS3
	// 40H-4FH: /IOS4
	// 50H-5FH: /IOS5
	map(0x60, 0x61).mirror(0x0e).rw(m_i8251, FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x70, 0x73).mirror(0x0c).rw(m_i8255, FUNC(i8255_device::read), FUNC(i8255_device::write));
}

static INPUT_PORTS_START(dgp80)
	PORT_START("KEY1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("3 IR") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("2 IY") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("1 IX") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("0 HL") PORT_CODE(KEYCODE_0)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("7 OUT") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("6 IN") PORT_CODE(KEYCODE_6)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("5 PC") PORT_CODE(KEYCODE_5)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("4 SP") PORT_CODE(KEYCODE_4)

	PORT_START("KEY2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("B INS") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("A XRG") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("9 SUB") PORT_CODE(KEYCODE_9)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("8 XRG'") PORT_CODE(KEYCODE_8)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("F EXEC") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("E DEC") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("D INC") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("C DSP") PORT_CODE(KEYCODE_C)

	PORT_START("RESET")
	PORT_BIT(1, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Reset") PORT_CODE(KEYCODE_BACKSPACE) PORT_WRITE_LINE_MEMBER(FUNC(dgp80_state::reset_w))
INPUT_PORTS_END

void dgp80_state::dgp80(machine_config &config)
{
	Z80(config, m_maincpu, 3.58_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &dgp80_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &dgp80_state::io_map);

	I8251(config, m_i8251, 3.58_MHz_XTAL);

	I8255(config, m_i8255);

	// TODO: expansion connectors

	config.set_default_layout(layout_dgp80);
}

ROM_START(dgp80)
	ROM_REGION(0x1000, "monitor", 0)
	ROM_LOAD("dgp80.bin", 0x0000, 0x1000, CRC(68336b96) SHA1(6c656fdcc57708197cb8da25eb10ad3b4156623b))
ROM_END

} // anonymous namespace

SYST(1987, dgp80, 0, 0, dgp80, dgp80, dgp80_state, empty_init, "Digiplan Industria e Comercio", "DGP-80", MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING)
