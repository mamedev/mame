// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Roland JX-3P synthesizer and similar modules.

****************************************************************************/

#include "emu.h"
#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
//#include "bus/midi/midi.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/adc0804.h"
#include "machine/nvram.h"
#include "pg200.h"
#include "machine/pit8253.h"
#include "machine/rescap.h"


namespace {

class roland_jx3p_state : public driver_device
{
public:
	roland_jx3p_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ifcpu(*this, "ifcpu")
		, m_cartslot(*this, "cartslot")
		, m_ifpit(*this, "ifpit%u", 0U)
	{
	}

	void jx3p(machine_config &config);
	void mks30(machine_config &config);
	void gr700(machine_config &config);

private:
	void prescale_w(u8 data);
	void dac_w(offs_t offset, u8 data);
	void sw_interface_w(u8 data);
	void led_display_w(offs_t offset, u8 data);
	void analog_select_w(u8 data);

	void if_anlg_mux_w(u8 data);

	void prog_map(address_map &map) ATTR_COLD;
	void common_ext_map(address_map &map) ATTR_COLD;
	void jk3p_ext_map(address_map &map) ATTR_COLD;
	void mks30_ext_map(address_map &map) ATTR_COLD;
	void if_prog_map(address_map &map) ATTR_COLD;
	void if_ext_map(address_map &map) ATTR_COLD;

	required_device<mcs51_cpu_device> m_maincpu;
	optional_device<mcs51_cpu_device> m_ifcpu;
	optional_device<generic_cartslot_device> m_cartslot;
	optional_device_array<pit8253_device, 4> m_ifpit;
};


void roland_jx3p_state::prescale_w(u8 data)
{
}

void roland_jx3p_state::dac_w(offs_t offset, u8 data)
{
}

void roland_jx3p_state::sw_interface_w(u8 data)
{
}

void roland_jx3p_state::led_display_w(offs_t offset, u8 data)
{
}

void roland_jx3p_state::analog_select_w(u8 data)
{
}

void roland_jx3p_state::if_anlg_mux_w(u8 data)
{
}

void roland_jx3p_state::prog_map(address_map &map)
{
	map.global_mask(0x1fff);
	map(0x0000, 0x1fff).rom().region("program", 0);
}

void roland_jx3p_state::common_ext_map(address_map &map)
{
	map(0x0000, 0x0000).mirror(0x1ff).w(FUNC(roland_jx3p_state::prescale_w));
	map(0x0200, 0x0203).mirror(0x1fc).w("counter1", FUNC(pit8253_device::write));
	map(0x0400, 0x0403).mirror(0x1fc).w("counter2", FUNC(pit8253_device::write));
	map(0x0600, 0x0603).mirror(0x1fc).w("counter3", FUNC(pit8253_device::write));
	map(0x0800, 0x0803).mirror(0x1fc).w("counter4", FUNC(pit8253_device::write));
	map(0x0a00, 0x0a00).mirror(0x10f).select(0xf0).w(FUNC(roland_jx3p_state::dac_w));
	map(0x0c08, 0x0c08).mirror(0x1f0).portr("SWSCN0");
	map(0x0c09, 0x0c09).mirror(0x1f0).portr("SWSCN1");
	map(0x0c0a, 0x0c0a).mirror(0x1f0).portr("SWSCN2");
	map(0x0c0b, 0x0c0b).mirror(0x1f0).portr("SWSCN3");
	map(0x0e00, 0x0e00).mirror(0x10f).select(0xf0).w(FUNC(roland_jx3p_state::led_display_w));
}

void roland_jx3p_state::jk3p_ext_map(address_map &map)
{
	map.global_mask(0x1fff);
	common_ext_map(map);
	map(0x0c00, 0x0c00).mirror(0x1f0).portr("KYSCN0");
	map(0x0c01, 0x0c01).mirror(0x1f0).portr("KYSCN1");
	map(0x0c02, 0x0c02).mirror(0x1f0).portr("KYSCN2");
	map(0x0c03, 0x0c03).mirror(0x1f0).portr("KYSCN3");
	map(0x0c04, 0x0c04).mirror(0x1f0).portr("KYSCN4");
	map(0x0c05, 0x0c05).mirror(0x1f0).portr("KYSCN5");
	map(0x0c06, 0x0c06).mirror(0x1f0).portr("KYSCN6");
	map(0x0c07, 0x0c07).mirror(0x1f0).portr("KYSCN7");
	map(0x0c0c, 0x0c0c).mirror(0x1f0).portr("SWSCN4");
	map(0x1000, 0x1000).mirror(0x1ff).w(FUNC(roland_jx3p_state::sw_interface_w));
	map(0x1800, 0x1fff).ram().share("nvram");
}

void roland_jx3p_state::mks30_ext_map(address_map &map)
{
	map.global_mask(0x3fff);
	common_ext_map(map);
	map(0x0c00, 0x0c00).mirror(0x1ff).w(FUNC(roland_jx3p_state::sw_interface_w));
	map(0x1000, 0x17ff).mirror(0x800).ram().share("nvram");
	map(0x2000, 0x3fff).rw(m_cartslot, FUNC(generic_cartslot_device::read_ram), FUNC(generic_cartslot_device::write_ram));
}

void roland_jx3p_state::if_prog_map(address_map &map)
{
	map.global_mask(0x1fff);
	map(0x0000, 0x1fff).rom().region("interface", 0);
}

void roland_jx3p_state::if_ext_map(address_map &map)
{
	map.global_mask(0xfff);
	map(0x000, 0x7ff).ram();
	map(0x800, 0x803).mirror(0x788).rw(m_ifpit[0], FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x810, 0x813).mirror(0x788).rw(m_ifpit[1], FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x820, 0x823).mirror(0x788).rw(m_ifpit[2], FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x830, 0x833).mirror(0x788).rw(m_ifpit[3], FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x840, 0x840).mirror(0x78f).rw("adc", FUNC(adc0803_device::read), FUNC(adc0803_device::write));
}


static INPUT_PORTS_START(jx3p)
	PORT_START("KYSCN0")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("KYSCN1")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("KYSCN2")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("KYSCN3")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("KYSCN4")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("KYSCN5")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("KYSCN6")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("KYSCN7")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("SWSCN0")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("SWSCN1")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("SWSCN2")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("SWSCN3")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("SWSCN4")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)
INPUT_PORTS_END

static INPUT_PORTS_START(mks30)
	PORT_START("SWSCN0")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("SWSCN1")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("SWSCN2")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("SWSCN3")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)
INPUT_PORTS_END


void roland_jx3p_state::jx3p(machine_config &config)
{
	I8031(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &roland_jx3p_state::prog_map);
	m_maincpu->set_addrmap(AS_IO, &roland_jx3p_state::jk3p_ext_map);
	m_maincpu->port_out_cb<1>().set(FUNC(roland_jx3p_state::analog_select_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // TC5517APL + battery

	PIT8253(config, "counter1");
	PIT8253(config, "counter2");
	PIT8253(config, "counter3");
	PIT8253(config, "counter4");

	PG200(config, "programmer");
}

void roland_jx3p_state::mks30(machine_config &config)
{
	jx3p(config);
	m_maincpu->set_addrmap(AS_IO, &roland_jx3p_state::mks30_ext_map);

	GENERIC_CARTSLOT(config, m_cartslot, generic_plain_slot, nullptr, "mks30_cart");
}

void roland_jx3p_state::gr700(machine_config &config)
{
	mks30(config);

	I8031(config, m_ifcpu, 12_MHz_XTAL);
	m_ifcpu->set_addrmap(AS_PROGRAM, &roland_jx3p_state::if_prog_map);
	m_ifcpu->set_addrmap(AS_IO, &roland_jx3p_state::if_ext_map);
	m_ifcpu->port_out_cb<1>().set(FUNC(roland_jx3p_state::if_anlg_mux_w));
	m_ifcpu->port_in_cb<3>().set("adc", FUNC(adc0803_device::intr_r)).lshift(4);

	for (auto &ifpit : m_ifpit)
	{
		PIT8253(config, ifpit);
		ifpit->set_clk<0>(12_MHz_XTAL / 6);
		ifpit->set_clk<1>(12_MHz_XTAL / 6);
		ifpit->set_clk<2>(12_MHz_XTAL / 6);
	}

	ADC0803(config, "adc", RES_K(10), CAP_P(150));
}

ROM_START(jx3p)
	ROM_REGION(0x2000, "program", 0)
	ROM_LOAD("v4_8c93.ic52", 0x0000, 0x2000, CRC(1c6790a2) SHA1(498c1f64d049ecba561149ae9e57f7a13972129f))
ROM_END

ROM_START(mks30)
	ROM_REGION(0x2000, "program", 0)
	ROM_LOAD("mks-30_2.0.ic46", 0x0000, 0x2000, CRC(09d6032f) SHA1(1bd8dac850e10ad53889ab2bf2ef913e0f932d96))
ROM_END

ROM_START(gr700)
	ROM_REGION(0x2000, "program", 0)
	ROM_LOAD("sh_v1.4.ic46", 0x0000, 0x2000, CRC(3c7eac76) SHA1(a8038a174d1a0dccb7197f9727b468ec9960f15a))

	ROM_REGION(0x2000, "interface", 0)
	ROM_LOAD("if_v1.4.ic17", 0x0000, 0x2000, CRC(56579d31) SHA1(b6d5b02e8952d52eff5a0bf77f7922e6d135454a))
ROM_END

} // anonymous namespace


SYST(1983, jx3p,  0, 0, jx3p,  jx3p,  roland_jx3p_state, empty_init, "Roland", "JX-3P Programmable Preset Polyphonic Synthesizer", MACHINE_IS_SKELETON)
SYST(1984, mks30, 0, 0, mks30, mks30, roland_jx3p_state, empty_init, "Roland", "MKS-30 Planet-S MIDI Sound Module",                MACHINE_IS_SKELETON)
SYST(1984, gr700, 0, 0, gr700, mks30, roland_jx3p_state, empty_init, "Roland", "GR-700 Guitar Synthesizer",                        MACHINE_IS_SKELETON)
