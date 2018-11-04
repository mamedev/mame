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
		, m_dsp(*this, "dsp%u", 0)
	{
	}

	DECLARE_DRIVER_INIT(whousetc)
	{
	}

	template <unsigned Dsp> DECLARE_WRITE16_MEMBER(update_dsp)
	{
		output().set_digit_value((Dsp << 2) | offset, data);
	}

	void whousetc(machine_config &config);
	void io_map(address_map &map);
	void program_map(address_map &map);
protected:
	virtual void machine_reset() override
	{
		for (required_device<dl1416_device> const &dsp : m_dsp)
			dsp->cu_w(1);
	}

	required_device_array<dl1416_device, 4> m_dsp;
};


ADDRESS_MAP_START(whouse_testcons_state::program_map)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x2003) AM_DEVWRITE("dsp0", dl1416_device, bus_w)
	AM_RANGE(0x2004, 0x2007) AM_DEVWRITE("dsp1", dl1416_device, bus_w)
	AM_RANGE(0x2008, 0x200b) AM_DEVWRITE("dsp2", dl1416_device, bus_w)
	AM_RANGE(0x200c, 0x200f) AM_DEVWRITE("dsp3", dl1416_device, bus_w)
	AM_RANGE(0x2800, 0x28ff) AM_DEVREADWRITE("i8155", i8155_device, memory_r, memory_w)
	AM_RANGE(0x3000, 0x3fff) AM_RAM
	AM_RANGE(0x8800, 0x8800) AM_READ_PORT("row0")
	AM_RANGE(0x8801, 0x8801) AM_READ_PORT("row1")
	AM_RANGE(0x8802, 0x8802) AM_READ_PORT("row2")
	AM_RANGE(0x8803, 0x8803) AM_READ_PORT("row3")
ADDRESS_MAP_END

ADDRESS_MAP_START(whouse_testcons_state::io_map)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x80, 0x87) AM_DEVREADWRITE("i8155", i8155_device, io_r, io_w)
	AM_RANGE(0x88, 0x8b) AM_DEVREADWRITE("i8255", i8255_device, read, write)
ADDRESS_MAP_END


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
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_W)     PORT_NAME("HARD / \xe2\x88\x92")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_E)     PORT_NAME("TRACE / \xc3\x97")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_R)     PORT_NAME("REL / \xc3\xb7")
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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z)     PORT_NAME("\xe2\x86\x90 / \xe2\x86\x91")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_X)     PORT_NAME("\xe2\x86\x92 / \xe2\x86\x93")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_C)     PORT_NAME("X / TRAN")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_V)     PORT_NAME("ENTER")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_B)     PORT_NAME("0")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_N)     PORT_NAME("1")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_M)     PORT_NAME("2")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("3")
INPUT_PORTS_END


MACHINE_CONFIG_START(whouse_testcons_state::whousetc)
	MCFG_CPU_ADD("maincpu", I8085A, 6.144_MHz_XTAL)
	MCFG_CPU_PROGRAM_MAP(program_map)
	MCFG_CPU_IO_MAP(io_map)

	MCFG_DEVICE_ADD("i8155", I8155, 6.144_MHz_XTAL)

	MCFG_DEVICE_ADD("i8255", I8255, 0)

	MCFG_DEVICE_ADD("dsp0", DL1416B, 0)
	MCFG_DL1416_UPDATE_HANDLER(WRITE16(whouse_testcons_state, update_dsp<0>))

	MCFG_DEVICE_ADD("dsp1", DL1416B, 0)
	MCFG_DL1416_UPDATE_HANDLER(WRITE16(whouse_testcons_state, update_dsp<1>))

	MCFG_DEVICE_ADD("dsp2", DL1416B, 0)
	MCFG_DL1416_UPDATE_HANDLER(WRITE16(whouse_testcons_state, update_dsp<2>))

	MCFG_DEVICE_ADD("dsp3", DL1416B, 0)
	MCFG_DL1416_UPDATE_HANDLER(WRITE16(whouse_testcons_state, update_dsp<3>))

	MCFG_DEFAULT_LAYOUT(layout_whousetc)
MACHINE_CONFIG_END


ROM_START(whousetc)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("ofm.g2", 0x0000, 0x1000, CRC(40bf8973) SHA1(7815d36532a40384f6ae2a38df3140c1e70a06aa))
	ROM_LOAD("ipc.a3", 0x1000, 0x1000, CRC(16f1e109) SHA1(0c6086f42c3cf6d79b1b54ab5c2dae8523c96d69))
ROM_END

} // anonymous namespace

//    YEAR   NAME      PARENT  COMPAT  MACHINE   INPUT     STATE                  INIT      COMPANY         FULLNAME                  FLAGS
COMP( 1980?, whousetc, 0,      0,      whousetc, whousetc, whouse_testcons_state, whousetc, "Westinghouse", "Test Console Serial #5", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW | MACHINE_CLICKABLE_ARTWORK )
