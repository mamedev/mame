// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Roland JX-8P and related synthesizers.

****************************************************************************/

#include "emu.h"
#include "jx8p_synth.h"
#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
//#include "bus/midi/midi.h"
#include "cpu/m6800/m6801.h"
//#include "cpu/upd7500/upd7500.h"
#include "mb63h149.h"
#include "machine/nvram.h"
//#include "machine/pg800.h"
#include "machine/rescap.h"
#include "machine/upd7001.h"


namespace {

class roland_jx8p_state : public driver_device
{
public:
	roland_jx8p_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_assignercpu(*this, "assignercpu")
		, m_sw_scan(*this, "SCAN%u", 0U)
	{
	}

	void jx8p(machine_config &config);
	void jx8po(machine_config &config);
	void jx10(machine_config &config);
	void mks70(machine_config &config);

private:
	u8 switches_r(offs_t offset);
	void leds_w(u8 data);

	void jx8p_assigner_map(address_map &map) ATTR_COLD;
	void superjx_assigner_map(address_map &map) ATTR_COLD;

	required_device<hd6303r_cpu_device> m_assignercpu;
	required_ioport_array<8> m_sw_scan;
};


u8 roland_jx8p_state::switches_r(offs_t offset)
{
	return m_sw_scan[offset]->read();
}

void roland_jx8p_state::leds_w(u8 data)
{
}

void roland_jx8p_state::jx8p_assigner_map(address_map &map)
{
	map(0x2000, 0x3fff).rw("cartslot", FUNC(generic_slot_device::read_ram), FUNC(generic_slot_device::write_ram));
	map(0x4000, 0x4007).mirror(0x1ff8).r(FUNC(roland_jx8p_state::switches_r));
	map(0x6000, 0x6000).mirror(0x1fff).w(FUNC(roland_jx8p_state::leds_w));
	map(0x8000, 0x87ff).mirror(0x1800).rw("keyscan", FUNC(mb63h149_device::read), FUNC(mb63h149_device::write));
	map(0xa000, 0xa7ff).mirror(0x1800).ram().share("nvram");
	map(0xc000, 0xffff).rom().region("assigner", 0);
}

void roland_jx8p_state::superjx_assigner_map(address_map &map)
{
	map(0x1000, 0x17ff).mirror(0x800).rw("keyscan", FUNC(mb63h149_device::read), FUNC(mb63h149_device::write));
	map(0x2000, 0x3fff).rw("cartslot", FUNC(generic_slot_device::read_ram), FUNC(generic_slot_device::write_ram));
	map(0x4000, 0x4007).mirror(0xff8).r(FUNC(roland_jx8p_state::switches_r));
	map(0x5000, 0x5000).mirror(0xfff).w(FUNC(roland_jx8p_state::leds_w));
	map(0x6000, 0x7fff).ram().share("nvram");
	map(0x8000, 0xffff).rom().region("assigner", 0);
}


static INPUT_PORTS_START(jx8p)
	PORT_START("SCAN0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("1")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("2")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("3")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("4")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("5")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("6")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("7")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("8")

	PORT_START("SCAN1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("9")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("10")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("11")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("12")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("13")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("14")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("15")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("16")

	PORT_START("SCAN2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("17")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("18")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("19")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("20")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("21")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("22")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("23")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("24")

	PORT_START("SCAN3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("25")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("26")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("27")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("28")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("29")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("30")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("31")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("32")

	PORT_START("SCAN4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Key Poly")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Key Unison")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Key Solo")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Bank Cartridge")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Bank Memory")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Bank Preset")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("SCAN5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("After Vibrato")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("After Brilliance")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("After Volume")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P.Chain Enter")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P.Chain Left")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P.Chain Right")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("SCAN6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Write")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Copy Cartridge")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Copy Memory")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Edit M.Tune")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Edit MIDI")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Edit Name")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Edit Param")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("SCAN7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("BRNG 0")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("BRNG 1")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("BRNG 2")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("BRNG 3")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("PORTA SW")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("BEND POL")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("LFO SW")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Hold Pedal")
INPUT_PORTS_END

static INPUT_PORTS_START(jx10)
	PORT_START("SCAN0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("A")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("B")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("C")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("D")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("E")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("F")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("G")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("H")

	PORT_START("SCAN1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P1")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P2")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P3")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P4")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P5")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P6")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P7")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P8")

	PORT_START("SCAN2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("T0")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("T1")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("T2")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("T3")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("T4")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("T5")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("T6")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("T7")

	PORT_START("SCAN3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("CH FC")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("CH TM")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("CH SW")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SQ FC")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SQ RE")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SW SW")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("T8")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("T9")

	PORT_START("SCAN4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("SCAN5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("SCAN6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("T ENT")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("SCAN7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)
INPUT_PORTS_END

static INPUT_PORTS_START(mks70)
	PORT_START("SCAN0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Write")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Param")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Value")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Name")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Chase")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Cart Sel")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("SCAN1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("M. Tune")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Patch")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Tone")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("MIDI")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Back")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Forward")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("SCAN2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("8")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("7")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("G9")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("H0")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Enter")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Tenkey")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("SCAN3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("A")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("B")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("C")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("D")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("E")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("F")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("SCAN4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("1")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("2")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("3")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("4")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("5")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("6")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("SCAN5")
	PORT_BIT(0x3f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("SCAN6")
	PORT_BIT(0x3f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("SCAN7")
	PORT_BIT(0x3f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)
INPUT_PORTS_END

void roland_jx8p_state::jx8p(machine_config &config)
{
	HD6303R(config, m_assignercpu, 16_MHz_XTAL / 2); // HD63B03RP
	m_assignercpu->set_addrmap(AS_PROGRAM, &roland_jx8p_state::jx8p_assigner_map);
	m_assignercpu->in_p1_cb().set("adc", FUNC(upd7001_device::eoc_so_r)).bit(0);
	m_assignercpu->out_p1_cb().set("adc", FUNC(upd7001_device::sck_w)).bit(1);
	m_assignercpu->out_p1_cb().append("adc", FUNC(upd7001_device::si_w)).bit(2);
	m_assignercpu->out_p1_cb().append("adc", FUNC(upd7001_device::cs_w)).bit(3);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // TC5517APL + battery

	mb63h149_device &keyscan(MB63H149(config, "keyscan", 16_MHz_XTAL));
	keyscan.int_callback().set_inputline(m_assignercpu, HD6301_IRQ1_LINE);

	GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, nullptr, "jx8p_cart");

	upd7001_device &adc(UPD7001(config, "adc", RES_K(27), CAP_P(47)));
	adc.dl_w(1);

	//UPD7537(config, "displaycpu", 400_kHz_XTAL);

	JX8P_SYNTH(config, "synth", 16_MHz_XTAL / 2);
}

void roland_jx8p_state::jx8po(machine_config &config)
{
	jx8p(config);

	MB63H130(config.replace(), "keyscan", 16_MHz_XTAL); // no INT
}

void roland_jx8p_state::jx10(machine_config &config)
{
	HD6303R(config, m_assignercpu, 16_MHz_XTAL / 2); // HD63B03RP
	m_assignercpu->set_addrmap(AS_PROGRAM, &roland_jx8p_state::superjx_assigner_map);
	m_assignercpu->in_p1_cb().set("adc", FUNC(upd7001_device::eoc_so_r)).bit(0);
	m_assignercpu->out_p1_cb().set("adc", FUNC(upd7001_device::sck_w)).bit(1);
	m_assignercpu->out_p1_cb().append("adc", FUNC(upd7001_device::si_w)).bit(2);
	m_assignercpu->out_p1_cb().append("adc", FUNC(upd7001_device::cs_w)).bit(3);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // TC5564PL-20 + battery

	mb63h149_device &keyscan(MB63H149(config, "keyscan", 16_MHz_XTAL));
	keyscan.int_callback().set_inputline(m_assignercpu, HD6301_IRQ1_LINE);

	GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, nullptr, "jx8p_cart");

	upd7001_device &adc(UPD7001(config, "adc", RES_K(27), CAP_P(47)));
	adc.dl_w(1);

	//UPD7538A(config, "displaycpu", 600_kHz_XTAL);

	SUPERJX_SYNTH(config, "lower", 16_MHz_XTAL / 2);
	SUPERJX_SYNTH(config, "upper", 16_MHz_XTAL / 2);
}

void roland_jx8p_state::mks70(machine_config &config)
{
	jx10(config);

	m_assignercpu->in_p1_cb().set_constant(0x01);
	m_assignercpu->out_p1_cb().set_nop();

	config.device_remove("adc");
}

ROM_START(jx8p)
	ROM_REGION(0x4000, "assigner", 0)
	ROM_LOAD("jx8p_a3-1.ic6", 0x0000, 0x4000, CRC(fc566635) SHA1(aa7ce16107553f337eb87bbe6171062950389ea1))

	ROM_REGION(0x800, "displaycpu", 0)
	ROM_LOAD("upd7537-014_15179201.ic1", 0x000, 0x800, NO_DUMP)

	ROM_REGION(0x4000, "synth:program", 0)
	ROM_LOAD("jx8p-b-v21.ic22", 0x0000, 0x2000, CRC(4f51c873) SHA1(c3217596000329ac2ff03f1b8b2a5e0f2f3d7783))
	ROM_RELOAD(0x2000, 0x2000)
ROM_END

// Older assigner boards with MB63H130 instead of MB63H149 are incompatible with Ver. 3.0 and later ROMs
ROM_START(jx8po)
	ROM_REGION(0x4000, "assigner", 0)
	ROM_SYSTEM_BIOS(0, "v21", "Ver. 2.1 (English)")
	ROMX_LOAD("jx8p-a-v21.ic6", 0x0000, 0x4000, CRC(4c357353) SHA1(287aad14dc22b1f9d814d0fa9ef3088dd8a33dc3), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v21d", "Ver. 2.1 (Danish?)") // only differences are "KLAVER" replaces "PIANO" and "SIUUUU" replaces "JX-JET"
	ROMX_LOAD("jx8p21a.ic6", 0x0000, 0x4000, CRC(f2d1ac36) SHA1(8df4d0647abbf70e4eea2971ac9c629c6f6ef4a3), ROM_BIOS(1))

	ROM_REGION(0x800, "displaycpu", 0)
	ROM_LOAD("upd7537-014_15179201.ic1", 0x000, 0x800, NO_DUMP)

	ROM_REGION(0x4000, "synth:program", 0)
	ROM_LOAD("jx8p-b-v21.ic22", 0x0000, 0x2000, CRC(4f51c873) SHA1(c3217596000329ac2ff03f1b8b2a5e0f2f3d7783))
	ROM_RELOAD(0x2000, 0x2000)
ROM_END

ROM_START(jx10)
	ROM_REGION(0x8000, "assigner", 0)
	ROM_SYSTEM_BIOS(0, "v230", "Ver. 2.30")
	ROMX_LOAD("jx-10_a2_3.ic6", 0x0000, 0x8000, CRC(3cfc8b94) SHA1(add36ff741f25ddb5324ec921fe06e7f19a61efe), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v230se", "Ver. 2.30 (tone edit hack)") // http://www.colinfraser.com/jx10/jx.htm
	ROMX_LOAD("jx10-2.3-se-00.ic6", 0x0000, 0x8000, CRC(60773d32) SHA1(74980fb5864b9ffeca2b1efce7512c0dbef96a67), ROM_BIOS(1))

	ROM_REGION(0x1000, "displaycpu", 0)
	ROM_LOAD("upd7538a-013_15179240.ic3", 0x0000, 0x1000, NO_DUMP)

	ROM_REGION(0x4000, "lower:program", 0)
	ROM_LOAD("jx-10_b2_1.ic1", 0x0000, 0x2000, CRC(db2c8a7a) SHA1(c99253149a9f23e6944e319f12594e8a02c3976f)) // M5L27128K-2
	ROM_RELOAD(0x2000, 0x2000)

	ROM_REGION(0x4000, "upper:program", 0)
	ROM_LOAD("jx-10_c2_1.ic1", 0x0000, 0x2000, CRC(4c701d9b) SHA1(76bb3e89d6dd348eda5c353fd822e95b3927e679)) // M5L27128K-2
	ROM_RELOAD(0x2000, 0x2000)
ROM_END

ROM_START(mks70)
	ROM_REGION(0x8000, "assigner", 0)
	ROM_SYSTEM_BIOS(0, "v108", "Ver. 1.08")
	ROMX_LOAD("mks70_v108-a.ic6", 0x0000, 0x8000, CRC(d0f2ed4e) SHA1(3f052684fbccf30cef391dc0420673002279d8e0), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v103", "Ver. 1.03")
	ROMX_LOAD("a-v103.ic6", 0x0000, 0x8000, CRC(9d43f917) SHA1(fa9a1c8d5dd2c19bee2650aa3f7080773d90f8ef), ROM_BIOS(1))

	ROM_REGION(0x1000, "displaycpu", 0)
	ROM_LOAD("upd7538a-013_15179240.ic3", 0x0000, 0x1000, NO_DUMP)

	ROM_REGION(0x4000, "lower:program", 0) // no change between 1.03 and 1.06
	ROM_LOAD("b-v103.ic1", 0x0000, 0x4000, CRC(d568f6d3) SHA1(78b723ad1e606f0fefc0ee17297172c1842e6bb6))

	ROM_REGION(0x4000, "upper:program", 0) // no change between 1.03 and 1.06
	ROM_LOAD("c-v103.ic1", 0x0000, 0x4000, CRC(4808729c) SHA1(0adcfa405d6f5be7c4c32ffa5b2e224c66e72f74))
ROM_END

} // anonymous namespace


SYST(1985, jx8p,  0,    0, jx8p,  jx8p,  roland_jx8p_state, empty_init, "Roland", "JX-8P Polyphonic Synthesizer (Ver. 3.x)", MACHINE_IS_SKELETON)
SYST(1985, jx8po, jx8p, 0, jx8po, jx8p,  roland_jx8p_state, empty_init, "Roland", "JX-8P Polyphonic Synthesizer (Ver. 2.x)", MACHINE_IS_SKELETON)
SYST(1986, jx10,  0,    0, jx10,  jx10,  roland_jx8p_state, empty_init, "Roland", "JX-10 Super JX Polyphonic Synthesizer", MACHINE_IS_SKELETON)
SYST(1987, mks70, jx10, 0, mks70, mks70, roland_jx8p_state, empty_init, "Roland", "MKS-70 Super JX Polyphonic Synthesizer", MACHINE_IS_SKELETON)
