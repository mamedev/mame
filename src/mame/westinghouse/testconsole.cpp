// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*

Westinghouse Test Console

This appears to be a wire-wrapped prototype of a device for field-
programming some kind of interlocking system (possibly railway
signalling or industrial automation).  Chips have date codes up to
8020, but there doesn't appear to be a copyright date in ROM, and the
nameplate has no date either.  There's a 110/220 switch on the side,
and the AC wiring uses the brown, blue, green/yellow standard.


Nameplate says:

Westinghouse
TEST CONSOLE
SERIAL #5
ELECTRONIC SYSTEMS DIVISION
WESTINGHOUSE CANADA LTD. ES-595C303


Key components:

* 6.144 MHz crystal
* Intel 8085A CPU
* Intel P8155 I/O, timer, RAM
* Intel 8255 PIA
* 2x D2732 EPROM (8 kB total)
* 8x 2114P45 RAM (4 kB total)
* 4x DL-1416 quad 16-segment displays
* 3x red LED
* 24-pin ZIF socket for burning 2716 or 2732
* 2x 4x4 keypad
* two insulated trimpots (programming voltage adjustment?)
* a sea of 7400 logic
* two DIP switches


Keypad layout (second pad is actually to the right of first):

MODE        TRAP/HEX    ARM/DEC     RESET/CLR
MNE/+       HARD/-      TRACE/×     REL/÷
REC/BCC     SOFT/LIST   DISP/VER    STEP/PROG
left/up     right/down  X/TRAN      ENTER

C/MW        D/IW        E/OF        F/IA
8/XX        9/HT        A/MR        B/IR
4           5           6           7
0           1           2           3

TODO:
* 8155/8255 peripherals
* Timers/interrupts
* EPROM programming
* Debugging target machine(s)
* DIP switches

*/

#include "emu.h"

#include "cpu/i8085/i8085.h"
#include "machine/i8155.h"
#include "machine/i8255.h"
#include "video/dl1416.h"

#include "whousetc.lh"

namespace {

class whouse_testcons_state : public driver_device
{
public:
	whouse_testcons_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_dsp(*this, "dsp%u", 0U)
		, m_digit(*this, "digit%u", 0U)
	{
	}

	void whousetc(machine_config &config);

private:
	template <unsigned Dsp> void update_dsp(offs_t offset, u16 data)
	{
		m_digit[(Dsp << 2) | offset] = data;
	}

	virtual void machine_start() override
	{
		m_digit.resolve();
	}

	virtual void machine_reset() override
	{
		for (required_device<dl1416_device> const &dsp : m_dsp)
			dsp->cu_w(1);
	}

	void io_map(address_map &map) ATTR_COLD;
	void program_map(address_map &map) ATTR_COLD;

	required_device_array<dl1416_device, 4> m_dsp;
	output_finder<16> m_digit;
};


void whouse_testcons_state::program_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x2003).w("dsp0", FUNC(dl1416_device::bus_w));
	map(0x2004, 0x2007).w("dsp1", FUNC(dl1416_device::bus_w));
	map(0x2008, 0x200b).w("dsp2", FUNC(dl1416_device::bus_w));
	map(0x200c, 0x200f).w("dsp3", FUNC(dl1416_device::bus_w));
	map(0x2800, 0x28ff).rw("i8155", FUNC(i8155_device::memory_r), FUNC(i8155_device::memory_w));
	map(0x3000, 0x3fff).ram();
	map(0x8800, 0x8800).portr("row0");
	map(0x8801, 0x8801).portr("row1");
	map(0x8802, 0x8802).portr("row2");
	map(0x8803, 0x8803).portr("row3");
}

void whouse_testcons_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0x80, 0x87).rw("i8155", FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
	map(0x88, 0x8b).rw("i8255", FUNC(i8255_device::read), FUNC(i8255_device::write));
}


INPUT_PORTS_START(whousetc)
	PORT_START("row0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_1)     PORT_NAME("MODE")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_2)     PORT_NAME("TRAP / HEX")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_3)     PORT_NAME("ARM / DEC")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_4)     PORT_NAME("RESET / CLR")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_5)     PORT_NAME("C / MW")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_6)     PORT_NAME("D / IW")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_7)     PORT_NAME("E / OF")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_8)     PORT_NAME("F / IA")

	PORT_START("row1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q)     PORT_NAME("MNE / +")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_W)     PORT_NAME("HARD / -")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_E)     PORT_NAME(u8"TRACE / ×")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_R)     PORT_NAME(u8"REL / ÷")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_T)     PORT_NAME("8 / XX")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y)     PORT_NAME("9 / HT")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_U)     PORT_NAME("A / MR")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_I)     PORT_NAME("B / IR")

	PORT_START("row2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_A)     PORT_NAME("REC / BCC")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_S)     PORT_NAME("SOFT / LIST")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_D)     PORT_NAME("DISP / VER")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_F)     PORT_NAME("STEP / PROG")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_G)     PORT_NAME("4")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_H)     PORT_NAME("5")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_J)     PORT_NAME("6")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_K)     PORT_NAME("7")

	PORT_START("row3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z)     PORT_NAME(u8"\u2190 / \u2191") // U+2190 = ←, U+2191 = ↑
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_X)     PORT_NAME(u8"\u2192 / \u2193") // U+2192 = →, U+2193 = ↓
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_C)     PORT_NAME("X / TRAN")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_V)     PORT_NAME("ENTER")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_B)     PORT_NAME("0")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_N)     PORT_NAME("1")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_M)     PORT_NAME("2")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("3")
INPUT_PORTS_END


void whouse_testcons_state::whousetc(machine_config &config)
{
	i8085a_cpu_device &maincpu(I8085A(config, "maincpu", 6.144_MHz_XTAL));
	maincpu.set_addrmap(AS_PROGRAM, &whouse_testcons_state::program_map);
	maincpu.set_addrmap(AS_IO, &whouse_testcons_state::io_map);

	I8155(config, "i8155", 6.144_MHz_XTAL);

	I8255(config, "i8255", 0);

	DL1416B(config, m_dsp[0], u32(0));
	m_dsp[0]->update().set(FUNC(whouse_testcons_state::update_dsp<0>));

	DL1416B(config, m_dsp[1], u32(0));
	m_dsp[1]->update().set(FUNC(whouse_testcons_state::update_dsp<1>));

	DL1416B(config, m_dsp[2], u32(0));
	m_dsp[2]->update().set(FUNC(whouse_testcons_state::update_dsp<2>));

	DL1416B(config, m_dsp[3], u32(0));
	m_dsp[3]->update().set(FUNC(whouse_testcons_state::update_dsp<3>));

	config.set_default_layout(layout_whousetc);
}


ROM_START(whousetc)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("ofm.g2", 0x0000, 0x1000, CRC(40bf8973) SHA1(7815d36532a40384f6ae2a38df3140c1e70a06aa))
	ROM_LOAD("ipc.a3", 0x1000, 0x1000, CRC(16f1e109) SHA1(0c6086f42c3cf6d79b1b54ab5c2dae8523c96d69))
ROM_END

} // anonymous namespace

//    YEAR   NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS                  INIT        COMPANY         FULLNAME                  FLAGS
COMP( 1980?, whousetc, 0,      0,      whousetc, whousetc, whouse_testcons_state, empty_init, "Westinghouse", "Test Console Serial #5", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
