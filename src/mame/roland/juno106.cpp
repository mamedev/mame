// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Roland Juno-106 synthesizer.

****************************************************************************/

#include "emu.h"
#include "mb63h114.h"
//#include "bus/midi/midi.h"
#include "cpu/upd7810/upd7810.h"
#include "machine/nvram.h"
#include "machine/pit8253.h"


namespace {

class juno106_state : public driver_device
{
public:
	juno106_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_modulecpu(*this, "modulecpu")
		, m_mac(*this, "mac")
		, m_pit(*this, "pit%u", 1U)
	{
	}

	void juno106(machine_config &config);
	void mks7(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void cpu_latch_w(u8 data);
	void module_pit_w(offs_t offset, u8 data);
	void module_latch_w(u8 data);

	void bass_w(u8 data);
	void dcom_w(u8 data);
	void rhythm_w(offs_t offset, u8 data);

	void main_map(address_map &map) ATTR_COLD;
	void mks7_main_map(address_map &map) ATTR_COLD;
	void module_map(address_map &map) ATTR_COLD;

	required_device<upd7810_device> m_maincpu;
	required_device<upd7810_device> m_modulecpu;
	optional_device<mb63h114_device> m_mac;
	required_device_array<pit8253_device, 2> m_pit;
};

void juno106_state::machine_start()
{
}


void juno106_state::cpu_latch_w(u8 data)
{
}

void juno106_state::bass_w(u8 data)
{
}

void juno106_state::dcom_w(u8 data)
{
}

void juno106_state::rhythm_w(offs_t offset, u8 data)
{
	m_mac->xst_w(offset & 0xff);
}

void juno106_state::module_pit_w(offs_t offset, u8 data)
{
	if (!BIT(offset, 12))
		m_pit[0]->write((offset & 0x300) >> 8, data);
	if (!BIT(offset, 13))
		m_pit[1]->write((offset & 0x300) >> 8, data);
}

void juno106_state::module_latch_w(u8 data)
{
}

void juno106_state::main_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("assigner", 0);
	map(0x0000, 0x0000).mirror(0x1fff).w(FUNC(juno106_state::cpu_latch_w));
	map(0x2000, 0x27ff).mirror(0x1800).ram().share("nvram");
}

void juno106_state::mks7_main_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("mainboard", 0);
	map(0x2800, 0x2800).mirror(0x07ff).w(FUNC(juno106_state::bass_w));
	map(0x3000, 0x3000).mirror(0x07ff).w(FUNC(juno106_state::dcom_w));
	map(0x3800, 0x3fff).w(FUNC(juno106_state::rhythm_w));
}

void juno106_state::module_map(address_map &map)
{
	map(0x0000, 0x0fff).mirror(0x3000).rom().region("module", 0);
	map(0x0000, 0x2fff).w(FUNC(juno106_state::module_pit_w));
	map(0x3000, 0x3000).mirror(0x0fff).w(FUNC(juno106_state::module_latch_w));
}


static INPUT_PORTS_START(juno106)
INPUT_PORTS_END

static INPUT_PORTS_START(mks7)
INPUT_PORTS_END

void juno106_state::juno106(machine_config &config)
{
	UPD7810(config, m_maincpu, 12_MHz_XTAL); // µPD7810G or µPD7811G-101-36
	m_maincpu->set_addrmap(AS_PROGRAM, &juno106_state::main_map);

	UPD7810(config, m_modulecpu, 12_MHz_XTAL); // µPD7810G or µPD7811G-102-36
	m_modulecpu->set_addrmap(AS_PROGRAM, &juno106_state::module_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // TC5517APL or MB8416-20L + battery

	PIT8253(config, m_pit[0]); // M82C53
	PIT8253(config, m_pit[1]); // M82C53
}

void juno106_state::mks7(machine_config &config)
{
	juno106(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &juno106_state::mks7_main_map);

	config.device_remove("nvram"); // no battery or external RAM

	MB63H114(config, "mac", 1.6_MHz_XTAL);
}

ROM_START(juno106)
	ROM_REGION(0x2000, "assigner", 0)
	ROM_LOAD("v5 a_5.ic2", 0x0000, 0x2000, CRC(4818686b) SHA1(4ab414a70281d382cb45478a62a6467e8dbb0a5d))

	ROM_REGION(0x2000, "module", 0)
	ROM_LOAD("v2 b_2.ic37", 0x0000, 0x2000, CRC(e0dc721e) SHA1(892b919a2476b269d916ba01dd5a81a25e044171))
ROM_END

ROM_START(mks7)
	ROM_REGION(0x2000, "mainboard", 0)
	ROM_LOAD("mks7-a-main.ic43", 0x0000, 0x2000, CRC(27d72dfb) SHA1(f5ed299e87d42ffccff70d8f0a406385050b4f46))

	ROM_REGION(0x10000, "mac", 0)
	ROM_LOAD("hn61256p_c71.ic14", 0x0000, 0x8000, NO_DUMP)
	ROM_LOAD("hn61256p_c72.ic13", 0x8000, 0x8000, NO_DUMP)

	ROM_REGION(0x8000, "ride", 0)
	ROM_LOAD("hn61256p_c44.ic22", 0x0000, 0x8000, NO_DUMP)

	ROM_REGION(0x8000, "crash", 0)
	ROM_LOAD("hn61256p_c42.ic21", 0x0000, 0x8000, NO_DUMP)

	ROM_REGION(0x2000, "module", 0)
	ROM_LOAD("mks7-b-module.ic38", 0x0000, 0x2000, CRC(306c6c54) SHA1(725e24199056c2788380b70acbc1a3842780e225))
ROM_END

} // anonymous namespace


SYST(1984, juno106, 0, 0, juno106, juno106, juno106_state, empty_init, "Roland", "Juno-106 Programmable Polyphonic Synthesizer", MACHINE_IS_SKELETON)
SYST(1985, mks7,    0, 0, mks7,    mks7,    juno106_state, empty_init, "Roland", "MKS-7 Super Quartet",                          MACHINE_IS_SKELETON)
