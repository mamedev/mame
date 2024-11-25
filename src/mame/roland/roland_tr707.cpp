// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Roland TR-707/727 drum machines.

    From the Service Notes: “The differences between two models [TR-707 and
    TR-727] are sound data, component values in several audio stages and a
    couple of pin connections at IC30 of Voice board. Both models derive all
    rhythm sounds from PCM-encoded samples of real sounds stored in ROM.”

****************************************************************************/

#include "emu.h"
#include "mb63h114.h"
#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
//#include "bus/midi/midi.h"
#include "cpu/m6800/m6801.h"
#include "machine/nvram.h"
//#include "video/hd61603.h"


namespace {

class roland_tr707_state : public driver_device
{
public:
	roland_tr707_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_mac(*this, "mac")
		, m_key_switches(*this, "KEY%u", 0U)
		, m_misc_select(0xff)
	{
	}

	void tr707(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void misc_select_w(u8 data);
	u8 key_scan_r();
	void leds_w(u8 data);
	void accent_level_w(u8 data);
	void ga_trigger_w(offs_t offset, u8 data);
	void voice_select_w(u8 data);

	void mem_map(address_map &map) ATTR_COLD;

	required_device<hd6303x_cpu_device> m_maincpu;
	required_device<mb63h114_device> m_mac;
	required_ioport_array<4> m_key_switches;

	u8 m_misc_select;
};

void roland_tr707_state::machine_start()
{
	save_item(NAME(m_misc_select));
}


void roland_tr707_state::misc_select_w(u8 data)
{
	m_misc_select = data;
}

u8 roland_tr707_state::key_scan_r()
{
	u8 data = 0xff;

	for (int n = 0; n < 4; n++)
		if (!BIT(m_misc_select, n))
			data &= m_key_switches[n]->read();

	return data;
}

void roland_tr707_state::leds_w(u8 data)
{
}

void roland_tr707_state::accent_level_w(u8 data)
{
}

void roland_tr707_state::ga_trigger_w(offs_t offset, u8 data)
{
	m_mac->xst_w(offset & 0xff);
}

void roland_tr707_state::voice_select_w(u8 data)
{
}

void roland_tr707_state::mem_map(address_map &map)
{
	map(0x0800, 0x0800).mirror(0x7ff).r(FUNC(roland_tr707_state::key_scan_r));
	//map(0x1000, 0x1000).mirror(0xfff).rw("lcdd", FUNC(hd61602_device::ready_r), FUNC(hd61602_device::write));
	map(0x2000, 0x27ff).ram().share("nvram1");
	map(0x2800, 0x2fff).ram().share("nvram2");
	map(0x3000, 0x3fff).rw("cartslot", FUNC(generic_slot_device::read_ram), FUNC(generic_slot_device::write_ram));
	map(0x4000, 0x4000).mirror(0xfff).w(FUNC(roland_tr707_state::leds_w));
	map(0x5000, 0x5000).mirror(0xfff).w(FUNC(roland_tr707_state::accent_level_w));
	map(0x6000, 0x6fff).w(FUNC(roland_tr707_state::ga_trigger_w));
	map(0x7000, 0x7000).mirror(0xfff).w(FUNC(roland_tr707_state::voice_select_w));
	map(0x8000, 0xbfff).mirror(0x4000).rom().region("program", 0);
}


static INPUT_PORTS_START(tr707)
	PORT_START("KEY0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("1")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("2")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("3")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("4")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("5")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("6")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("7")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("8")

	PORT_START("KEY1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("9")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("10")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("11")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("12")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("13")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("14")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("15")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("16")

	PORT_START("KEY2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Start")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Stop/Cont")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Enter")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Shift")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Pattern Clear")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Scale")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Last Step")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Inst Select")

	PORT_START("KEY3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Tempo")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Track")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Pattern")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Shuffle")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Group A")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Group B")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Group C")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Group D")
INPUT_PORTS_END

void roland_tr707_state::tr707(machine_config &config)
{
	HD6303X(config, m_maincpu, 4_MHz_XTAL); // HD6303XF
	m_maincpu->set_addrmap(AS_PROGRAM, &roland_tr707_state::mem_map);
	m_maincpu->out_p6_cb().set(FUNC(roland_tr707_state::misc_select_w));

	NVRAM(config, "nvram1", nvram_device::DEFAULT_ALL_0); // HM6116LP-4 + battery
	NVRAM(config, "nvram2", nvram_device::DEFAULT_ALL_0); // HM6116LP-4 + battery

	//HD61602(config, "lcdd");

	GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, nullptr, "tr707_cart");

	MB63H114(config, m_mac, 1.6_MHz_XTAL);
}

ROM_START(tr707)
	ROM_REGION(0x4000, "program", 0)
	ROM_LOAD("os_rom_firmware.ic13", 0x0000, 0x4000, CRC(3517ea00) SHA1(f5d57a79abf49131bd9832ae4e2dbced914ea523)) // 27128

	ROM_REGION(0x10000, "voices", 0) // "BD-MT" (IC34) and "HT-TAMB" (IC35)
	ROM_LOAD("hn61256p_c71_15179694.ic34", 0x0000, 0x8000, CRC(a196489b) SHA1(fd2bfe67d4d03d2b2134aa7feebe9167c44b1f8d))
	ROM_LOAD("hn61256p_c72_15179695.ic35", 0x8000, 0x8000, CRC(b05302e5) SHA1(5cc866f345906d817147ae2a61bc36d7be926511))

	ROM_REGION(0x8000, "cymbal1", 0) // "Crash Cymbal"
	ROM_LOAD("hn61256p_c73_15179696.ic19", 0x0000, 0x8000, CRC(b0bea07f) SHA1(965e23ad71e1f95d56307fa67272725dff46ba67))

	ROM_REGION(0x8000, "cymbal2", 0) // "Ride Cymbal"
	ROM_LOAD("hn61256p_c74_15179697.ic22", 0x0000, 0x8000, CRC(9411943a) SHA1(6c7c0f002ed66e4ccf182a4538d9bb239623ac43))
ROM_END

ROM_START(tr727)
	ROM_REGION(0x4000, "program", 0)
	ROM_LOAD("osv_1.0_hd4827128.ic13", 0x0000, 0x4000, CRC(49954161) SHA1(8eb033d9729aa84cc3c33b8ce30925ff3c35e70a))

	ROM_REGION(0x10000, "voices", 0) // "BNG-HTB" (IC34) and "LTB-MC" (IC35)
	ROM_LOAD("hn61256p_15179694.ic34", 0x0000, 0x8000, NO_DUMP)
	ROM_LOAD("hn61256p_15179695.ic35", 0x8000, 0x8000, NO_DUMP)

	ROM_REGION(0x8000, "cymbal1", 0) // "Quijada"
	ROM_LOAD("hn61256p_15179696.ic19", 0x0000, 0x8000, NO_DUMP)

	ROM_REGION(0x8000, "cymbal2", 0) // "Star Chime"
	ROM_LOAD("hn61256p_15179697.ic22", 0x0000, 0x8000, NO_DUMP)
ROM_END

} // anonymous namespace


SYST(1985, tr707, 0, 0, tr707, tr707, roland_tr707_state, empty_init, "Roland", "TR-707 Rhythm Composer", MACHINE_IS_SKELETON)
SYST(1985, tr727, 0, 0, tr707, tr707, roland_tr707_state, empty_init, "Roland", "TR-727 Rhythm Composer", MACHINE_IS_SKELETON)
