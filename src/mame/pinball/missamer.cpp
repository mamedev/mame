// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Miss Americana

    © 1984 Sirmo

    "Bingo Pinball" style game.

    Hardware:
    - P8085A
    - 6.144 MHz XTAL
    - 3x P8155

    TODO:
    - The "Magic Lines" feature isn't emulated
    - There is no ball physics simulation
    - Sound isn't emulated at all (you can see triggers on the artwork)
    - The optional hardware random number generator isn't emulated (see DSW D)
    - Output counters
    - Verify/measure clocks
    - "Telephone" input/output (probably just a remote control?)

    Notes:
    - It will display the error "cccc" on cold boot. Just push F3 once
      to reset and initialize
    - Blinking tilt led means "normal operation"
    - The service mode supports viewing and changing all memory values as
      well as an output (SV5) and input (SV6) test

***************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/clock.h"
#include "machine/i8155.h"
#include "machine/nvram.h"
#include "missamer.lh"


namespace {


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class missamer_state : public driver_device
{
public:
	missamer_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ramio(*this, "ramio%u", 0U),
		m_inputs(*this, "row%u", 0U),
		m_extra(*this, "extra"),
		m_lamps(*this, "lamp%u.%u", 0U, 0U),
		m_digits(*this, "digit%u", 0U),
		m_dy(0),
		m_scan(0)
	{ }

	void missamer(machine_config &config);

protected:
	void machine_start() override ATTR_COLD;

private:
	required_device<i8085a_cpu_device> m_maincpu;
	required_device_array<i8155_device, 3> m_ramio;
	required_ioport_array<16> m_inputs;
	required_ioport m_extra;
	output_finder<8, 18> m_lamps;
	output_finder<4> m_digits;

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	void ramio0_pa_w(uint8_t data);
	void ramio0_pb_w(uint8_t data);
	void ramio0_pc_w(uint8_t data);
	uint8_t ramio1_pa_r();
	uint8_t ramio1_pb_r();
	void ramio1_pc_w(uint8_t data);
	void ramio2_pb_w(uint8_t data);
	void ramio2_pc_w(uint8_t data);

	uint32_t m_dy;
	uint8_t m_scan;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void missamer_state::mem_map(address_map &map)
{
	map(0x0000, 0x27ff).rom().region("maincpu", 0);
	map(0x4000, 0x40ff).ram().share("nvram");
	map(0x4100, 0x41ff).mirror(0x2000).rw(m_ramio[0], FUNC(i8155_device::memory_r), FUNC(i8155_device::memory_w));
	map(0x4200, 0x42ff).mirror(0x2000).rw(m_ramio[1], FUNC(i8155_device::memory_r), FUNC(i8155_device::memory_w));
	map(0x4300, 0x43ff).mirror(0x2000).rw(m_ramio[2], FUNC(i8155_device::memory_r), FUNC(i8155_device::memory_w));
	map(0xc100, 0xc1ff).mirror(0x2000).rw(m_ramio[0], FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
	map(0xc200, 0xc2ff).mirror(0x2000).rw(m_ramio[1], FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
	map(0xc300, 0xc3ff).mirror(0x2000).rw(m_ramio[2], FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
}


//**************************************************************************
//  INPUT PORT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( missamer )
	PORT_START("row0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Hole 1") PORT_TOGGLE
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Hole 2") PORT_TOGGLE
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Hole 3") PORT_TOGGLE
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Hole 4") PORT_TOGGLE
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Hole 5") PORT_TOGGLE
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Hole 6") PORT_TOGGLE
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Hole 7") PORT_TOGGLE
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Shutter")

	PORT_START("row1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Hole 8")  PORT_TOGGLE
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Hole 9")  PORT_TOGGLE
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Hole 10") PORT_TOGGLE
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Hole 11") PORT_TOGGLE
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Hole 12") PORT_TOGGLE
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Hole 13") PORT_TOGGLE
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Gate")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Carry Over Shutter")

	PORT_START("row2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Hole 14") PORT_TOGGLE
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Hole 15") PORT_TOGGLE
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Hole 16") PORT_TOGGLE
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Hole 17") PORT_TOGGLE
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Hole 18") PORT_TOGGLE
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Hole 19") PORT_TOGGLE
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Hole 20") PORT_TOGGLE
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Hole 21") PORT_TOGGLE

	PORT_START("row3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Hole 22") PORT_TOGGLE
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Hole 23") PORT_TOGGLE
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Hole 24") PORT_TOGGLE
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Hole 25") PORT_TOGGLE
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Red Rollover")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Yellow Rollover")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Alley")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("row4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TRG 1")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TRG 2")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TRG 3")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TRG 4")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TRG 5")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Carry Over Balllifter")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Rest Contact Balllifter")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Manual Balllift")

	PORT_START("row5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER)  PORT_NAME("R Button")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER)  PORT_NAME("X Button")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER)  PORT_NAME("Door")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER)  PORT_NAME("TRG 0")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("row6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_TILT)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Red Doorbutton")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Yellow Doorbutton")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("A Button Magic Lines")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("B Button Magic Lines")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("C Button Magic Lines")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("D Button Magic Lines")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("E Button Magic Lines")

	PORT_START("row7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_COIN2) // right coinslot
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_COIN1) // left coinslot
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Service SV1 (Service)")     PORT_TOGGLE
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Service SV2 (+)")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Service SV3 (-)")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Service SV4 (See Inside)")  PORT_TOGGLE
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Service SV5 (Input Test)")  PORT_TOGGLE
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Service SV6 (Output Test)") PORT_TOGGLE

	// seems to control the hardware random number generator, set to 0xa7 to disable it
	PORT_START("row8")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x00, "D:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x00, "D:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x00, "D:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "D:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "D:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x00, "D:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "D:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x00, "D:8")

	PORT_START("row9")
	PORT_DIPNAME(0x0f, 0x0e, "Coins (Right)")   PORT_DIPLOCATION("C:1,2,3,4")
	PORT_DIPSETTING(   0x00, "16")
	PORT_DIPSETTING(   0x01, "15")
	PORT_DIPSETTING(   0x02, "14")
	PORT_DIPSETTING(   0x03, "13")
	PORT_DIPSETTING(   0x04, "12")
	PORT_DIPSETTING(   0x05, "11")
	PORT_DIPSETTING(   0x06, "10")
	PORT_DIPSETTING(   0x07, "9")
	PORT_DIPSETTING(   0x08, "8")
	PORT_DIPSETTING(   0x09, "7")
	PORT_DIPSETTING(   0x0a, "6")
	PORT_DIPSETTING(   0x0b, "5")
	PORT_DIPSETTING(   0x0c, "4")
	PORT_DIPSETTING(   0x0d, "3")
	PORT_DIPSETTING(   0x0e, "2")
	PORT_DIPSETTING(   0x0f, "1")
	PORT_DIPNAME(0xf0, 0xd0, "Credits (Right)") PORT_DIPLOCATION("C:5,6,7,8")
	PORT_DIPSETTING(   0x00, "15")
	PORT_DIPSETTING(   0x10, "14")
	PORT_DIPSETTING(   0x20, "13")
	PORT_DIPSETTING(   0x30, "12")
	PORT_DIPSETTING(   0x40, "11")
	PORT_DIPSETTING(   0x50, "10")
	PORT_DIPSETTING(   0x60, "9")
	PORT_DIPSETTING(   0x70, "8")
	PORT_DIPSETTING(   0x80, "7")
	PORT_DIPSETTING(   0x90, "6")
	PORT_DIPSETTING(   0xa0, "5")
	PORT_DIPSETTING(   0xb0, "4")
	PORT_DIPSETTING(   0xc0, "3")
	PORT_DIPSETTING(   0xd0, "2")
	PORT_DIPSETTING(   0xe0, "1")
	PORT_DIPSETTING(   0xf0, "0")

	PORT_START("row10")
	PORT_DIPNAME(0x0f, 0x0f, "Coins (Left)")   PORT_DIPLOCATION("B:1,2,3,4")
	PORT_DIPSETTING(   0x00, "16")
	PORT_DIPSETTING(   0x01, "15")
	PORT_DIPSETTING(   0x02, "14")
	PORT_DIPSETTING(   0x03, "13")
	PORT_DIPSETTING(   0x04, "12")
	PORT_DIPSETTING(   0x05, "11")
	PORT_DIPSETTING(   0x06, "10")
	PORT_DIPSETTING(   0x07, "9")
	PORT_DIPSETTING(   0x08, "8")
	PORT_DIPSETTING(   0x09, "7")
	PORT_DIPSETTING(   0x0a, "6")
	PORT_DIPSETTING(   0x0b, "5")
	PORT_DIPSETTING(   0x0c, "4")
	PORT_DIPSETTING(   0x0d, "3")
	PORT_DIPSETTING(   0x0e, "2")
	PORT_DIPSETTING(   0x0f, "1")
	PORT_DIPNAME(0xf0, 0xe0, "Credits (Left)") PORT_DIPLOCATION("B:5,6,7,8")
	PORT_DIPSETTING(   0x00, "15")
	PORT_DIPSETTING(   0x10, "14")
	PORT_DIPSETTING(   0x20, "13")
	PORT_DIPSETTING(   0x30, "12")
	PORT_DIPSETTING(   0x40, "11")
	PORT_DIPSETTING(   0x50, "10")
	PORT_DIPSETTING(   0x60, "9")
	PORT_DIPSETTING(   0x70, "8")
	PORT_DIPSETTING(   0x80, "7")
	PORT_DIPSETTING(   0x90, "6")
	PORT_DIPSETTING(   0xa0, "5")
	PORT_DIPSETTING(   0xb0, "4")
	PORT_DIPSETTING(   0xc0, "3")
	PORT_DIPSETTING(   0xd0, "2")
	PORT_DIPSETTING(   0xe0, "1")
	PORT_DIPSETTING(   0xf0, "0")

	PORT_START("row11")
	PORT_DIPNAME(0x07, 0x07, "Win Rate")             PORT_DIPLOCATION("A:1,2,3")
	PORT_DIPSETTING(   0x00, "8") // Most
	PORT_DIPSETTING(   0x01, "7")
	PORT_DIPSETTING(   0x02, "6")
	PORT_DIPSETTING(   0x03, "5")
	PORT_DIPSETTING(   0x04, "4")
	PORT_DIPSETTING(   0x05, "3")
	PORT_DIPSETTING(   0x06, "2")
	PORT_DIPSETTING(   0x07, "1") // Least
	PORT_DIPNAME(0x08, 0x08, "Reflex Chances")       PORT_DIPLOCATION("A:4")
	PORT_DIPSETTING(   0x00, DEF_STR( No ))
	PORT_DIPSETTING(   0x08, DEF_STR( Yes ))
	PORT_DIPNAME(0x10, 0x10, "Plots")                PORT_DIPLOCATION("A:5")
	PORT_DIPSETTING(   0x00, DEF_STR( No ))
	PORT_DIPSETTING(   0x10, DEF_STR( Yes ))
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "A:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "A:7")
	PORT_DIPNAME(0x80, 0x80, "Chance: Corner 65-10") PORT_DIPLOCATION("A:8")
	PORT_DIPSETTING(   0x00, DEF_STR( No ))
	PORT_DIPSETTING(   0x80, DEF_STR( Yes ))

	PORT_START("row12")
	PORT_DIPNAME(0x01, 0x01, "Chance: Corner 78-9")            PORT_DIPLOCATION("E:1")
	PORT_DIPSETTING(   0x00, DEF_STR( No ))
	PORT_DIPSETTING(   0x01, DEF_STR( Yes ))
	PORT_DIPNAME(0x02, 0x02, "Chance: 4 Stars As Green 53-12") PORT_DIPLOCATION("E:2")
	PORT_DIPSETTING(   0x00, DEF_STR( No ))
	PORT_DIPSETTING(   0x02, DEF_STR( Yes ))
	PORT_DIPNAME(0x04, 0x04, "Chance: 4 Stars As Green 61-12") PORT_DIPLOCATION("E:3")
	PORT_DIPSETTING(   0x00, DEF_STR( No ))
	PORT_DIPSETTING(   0x04, DEF_STR( Yes ))
	PORT_DIPNAME(0x08, 0x08, "Chance: Stripped Diagonal B")    PORT_DIPLOCATION("E:4")
	PORT_DIPSETTING(   0x00, DEF_STR( No ))
	PORT_DIPSETTING(   0x08, DEF_STR( Yes ))
	PORT_DIPNAME(0x10, 0x10, "Chance: Stripped Diagonal C")    PORT_DIPLOCATION("E:5")
	PORT_DIPSETTING(   0x00, DEF_STR( No ))
	PORT_DIPSETTING(   0x10, DEF_STR( Yes ))
	PORT_DIPNAME(0x20, 0x20, "Chance: Stripped Diagonal D")    PORT_DIPLOCATION("E:6")
	PORT_DIPSETTING(   0x00, DEF_STR( No ))
	PORT_DIPSETTING(   0x20, DEF_STR( Yes ))
	PORT_DIPNAME(0x40, 0x40, "Chance: Extra Card")             PORT_DIPLOCATION("E:7")
	PORT_DIPSETTING(   0x00, DEF_STR( No ))
	PORT_DIPSETTING(   0x40, DEF_STR( Yes ))
	PORT_DIPNAME(0x80, 0x80, "Chance: Any 2")                  PORT_DIPLOCATION("E:8")
	PORT_DIPSETTING(   0x00, DEF_STR( No ))
	PORT_DIPSETTING(   0x80, DEF_STR( Yes ))

	PORT_START("row13")
	PORT_DIPNAME(0x01, 0x01, "Chance: Any 3")         PORT_DIPLOCATION("F:1")
	PORT_DIPSETTING(   0x00, DEF_STR( No ))
	PORT_DIPSETTING(   0x01, DEF_STR( Yes ))
	PORT_DIPNAME(0x02, 0x02, "Chance: Extra Ball 17") PORT_DIPLOCATION("F:2")
	PORT_DIPSETTING(   0x00, DEF_STR( No ))
	PORT_DIPSETTING(   0x02, DEF_STR( Yes ))
	PORT_DIPNAME(0x04, 0x04, "Chance: Extra Ball 16") PORT_DIPLOCATION("F:3")
	PORT_DIPSETTING(   0x00, DEF_STR( No ))
	PORT_DIPSETTING(   0x04, DEF_STR( Yes ))
	PORT_DIPNAME(0x08, 0x08, "Chance: Extra Ball 15") PORT_DIPLOCATION("F:4")
	PORT_DIPSETTING(   0x00, DEF_STR( No ))
	PORT_DIPSETTING(   0x08, DEF_STR( Yes ))
	PORT_DIPNAME(0x10, 0x10, "Chance: Extra Ball 14") PORT_DIPLOCATION("F:5")
	PORT_DIPSETTING(   0x00, DEF_STR( No ))
	PORT_DIPSETTING(   0x10, DEF_STR( Yes ))
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "F:6")
	PORT_DIPNAME(0x40, 0x40, "Reflex")                PORT_DIPLOCATION("F:7")
	PORT_DIPSETTING(   0x00, DEF_STR( No ))
	PORT_DIPSETTING(   0x40, DEF_STR( Yes ))
	PORT_DIPNAME(0x80, 0x80, "Knock-Off")             PORT_DIPLOCATION("F:8")
	PORT_DIPSETTING(   0x00, DEF_STR( No ))
	PORT_DIPSETTING(   0x80, DEF_STR( Yes ))

	PORT_START("row14")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "G:1")
	PORT_DIPNAME(0x02, 0x02, "32 Points Extra Ball") PORT_DIPLOCATION("G:2")
	PORT_DIPSETTING(   0x00, DEF_STR( No ))
	PORT_DIPSETTING(   0x02, DEF_STR( Yes ))
	PORT_DIPNAME(0x04, 0x04, "Game Speed")           PORT_DIPLOCATION("G:3")
	PORT_DIPSETTING(   0x00, "Fast")
	PORT_DIPSETTING(   0x04, "Slow")
	PORT_DIPNAME(0x08, 0x08, "32 Points Selection")  PORT_DIPLOCATION("G:4")
	PORT_DIPSETTING(   0x00, DEF_STR( No ))
	PORT_DIPSETTING(   0x08, DEF_STR( Yes ))
	PORT_DIPNAME(0x10, 0x10, "Mixer 2")              PORT_DIPLOCATION("G:5")
	PORT_DIPSETTING(   0x00, DEF_STR( No ))
	PORT_DIPSETTING(   0x10, DEF_STR( Yes ))
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "G:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "G:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "G:8")

	PORT_START("row15")
	PORT_DIPNAME(0x01, 0x01, "Memory Test") PORT_DIPLOCATION("H:1")
	PORT_DIPSETTING(   0x00, DEF_STR( On ))
	PORT_DIPSETTING(   0x01, DEF_STR( Off ))
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "H:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "H:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "H:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "H:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "H:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "H:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "H:8")

	PORT_START("extra")
	PORT_DIPNAME(0x80, 0x80, "Bit 7")
	PORT_DIPSETTING(   0x00, DEF_STR( No ))
	PORT_DIPSETTING(   0x80, DEF_STR( Yes ))
	PORT_DIPNAME(0x40, 0x40, "Bit 6")
	PORT_DIPSETTING(   0x00, DEF_STR( No ))
	PORT_DIPSETTING(   0x40, DEF_STR( Yes ))
	PORT_DIPNAME(0x20, 0x20, "Switch Left")
	PORT_DIPSETTING(   0x00, DEF_STR( No ))
	PORT_DIPSETTING(   0x20, DEF_STR( Yes ))
	PORT_DIPNAME(0x10, 0x10, "Switch Right")
	PORT_DIPSETTING(   0x00, DEF_STR( No ))
	PORT_DIPSETTING(   0x10, DEF_STR( Yes ))
	PORT_DIPNAME(0x08, 0x08, "Tel 3")
	PORT_DIPSETTING(   0x00, DEF_STR( No ))
	PORT_DIPSETTING(   0x08, DEF_STR( Yes ))
	PORT_DIPNAME(0x04, 0x04, "Tel 2")
	PORT_DIPSETTING(   0x00, DEF_STR( No ))
	PORT_DIPSETTING(   0x04, DEF_STR( Yes ))
	PORT_DIPNAME(0x02, 0x02, "Tel 1")
	PORT_DIPSETTING(   0x00, DEF_STR( No ))
	PORT_DIPSETTING(   0x02, DEF_STR( Yes ))
	PORT_DIPNAME(0x01, 0x01, "Tel 0")
	PORT_DIPSETTING(   0x00, DEF_STR( No ))
	PORT_DIPSETTING(   0x01, DEF_STR( Yes ))
INPUT_PORTS_END


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void missamer_state::ramio0_pa_w(uint8_t data)
{
	// outputs row 1-8
	m_dy = (m_dy & 0x3ff00) | (data << 0);
}

void missamer_state::ramio0_pb_w(uint8_t data)
{
	// outputs row 9-16
	m_dy = (m_dy & 0x300ff) | (data << 8);
}

void missamer_state::ramio0_pc_w(uint8_t data)
{
	// 54----  outputs row 18, 17
	// --3210  scan column

	m_dy = (m_dy & 0x0ffff) | ((data >> 4) << 16);
	m_scan = ~data & 0x0f;

	// circuit is actually more complicated with a 555 timer
	if (BIT(m_scan, 3))
		for (int i = 0; i < 18; i++)
			m_lamps[m_scan & 0x07][i] = BIT(m_dy, i) ? 0 : 1;
}

uint8_t missamer_state::ramio1_pa_r()
{
	// 76------  bit 7 and 6
	// --5-----  switch right
	// ---4----  switch left
	// ----3210  telephone

	return m_extra->read();
}

uint8_t missamer_state::ramio1_pb_r()
{
	return m_inputs[m_scan]->read();
}

void missamer_state::ramio1_pc_w(uint8_t data)
{
	// 5-----  cash counter
	// -43210  magic motor lines e to a

	//logerror("ramio1_pc_w: %02x\n", data);
}

void missamer_state::ramio2_pb_w(uint8_t data)
{
	// 7-------  total in counter
	// -6------  total out counter
	// --5-----  telephone in counter
	// ---4----  telephone out counter
	// ----3210  led digit value

	//logerror("ramio2_pb_w: %02x\n", data);

	static const uint8_t ls7447[16] = {
		0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07,
		0x7f, 0x67, 0x58, 0x4c, 0x62, 0x69, 0x78, 0x00
	};

	// 74145
	if (m_scan < 4)
		m_digits[m_scan] = ls7447[~data & 0x0f];
}

void missamer_state::ramio2_pc_w(uint8_t data)
{
	// 5-----  unused
	// -4----  motor playfield
	// --3---  motor lifter
	// ---2--  strobe port a
	// ----1-  abf
	// -----0  int port a

	//logerror("ramio2_pc_w: %02x\n", data);
}

void missamer_state::machine_start()
{
	// resolve artwork outputs
	m_lamps.resolve();
	m_digits.resolve();

	// register for save states
	save_item(NAME(m_dy));
	save_item(NAME(m_scan));
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void missamer_state::missamer(machine_config &config)
{
	I8085A(config, m_maincpu, 6.144_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &missamer_state::mem_map);
	m_maincpu->set_clk_out(m_ramio[1], FUNC(i8155_device::set_unscaled_clock_int));

	clock_device &vco_clock(CLOCK(config, "vco_clock", 500)); // 74LS124
	vco_clock.signal_handler().set_inputline(m_maincpu, I8085_RST75_LINE);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	I8155(config, m_ramio[0], 500); // VCO (470nf)
	m_ramio[0]->out_to_callback().set_inputline(m_maincpu, I8085_RST65_LINE);
	m_ramio[0]->out_pa_callback().set(FUNC(missamer_state::ramio0_pa_w));
	m_ramio[0]->out_pb_callback().set(FUNC(missamer_state::ramio0_pb_w));
	m_ramio[0]->out_pc_callback().set(FUNC(missamer_state::ramio0_pc_w));

	I8155(config, m_ramio[1], 0); // CLK from 8085
	// timer output: UART
	m_ramio[1]->in_pa_callback().set(FUNC(missamer_state::ramio1_pa_r));
	m_ramio[1]->in_pb_callback().set(FUNC(missamer_state::ramio1_pb_r));
	m_ramio[1]->out_pc_callback().set(FUNC(missamer_state::ramio1_pc_w));

	I8155(config, m_ramio[2], 235000); // VCO (100pf)
	m_ramio[2]->out_pb_callback().set(FUNC(missamer_state::ramio2_pb_w));
	m_ramio[2]->out_pc_callback().set(FUNC(missamer_state::ramio2_pc_w));

	config.set_default_layout(layout_missamer);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( missamer )
	ROM_REGION(0x2800, "maincpu", 0)
	ROM_LOAD("c48327.6p", 0x0000, 0x0800, CRC(45fd79f7) SHA1(67b115b13ef7fb94b63176e8e17d97e9480ab4d3))
	ROM_LOAD("c48328.6q", 0x0800, 0x0800, CRC(0cd444ec) SHA1(a797053dbfeb67428f08e914220c34cd440181f1))
	ROM_LOAD("c48329.6s", 0x1000, 0x0800, CRC(82b87e2d) SHA1(0727ddb5792e1dc105ed7478474cc97839b87106))
	ROM_LOAD("c48330.6u", 0x1800, 0x0800, CRC(f76142c6) SHA1(727d657f19d5341323d1ccb6eb96e48a402e089d))
	ROM_LOAD("c48331.6w", 0x2000, 0x0800, CRC(5dd0b97a) SHA1(38d8183865e7c182aef0a0a7d514acf6f961f72c))
ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME      PARENT  MACHINE   INPUT     CLASS           INIT        ROTATION  COMPANY  FULLNAME          FLAGS
GAME(1984,  missamer, 0,      missamer, missamer, missamer_state, empty_init, ROT0,     "Sirmo", "Miss Americana", MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
