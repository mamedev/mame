// license:BSD-3-Clause
// copyright-holders:AJR
/************************************************************************************************************

    Skeleton driver for Ericsson/Datasaab 9161 Display Processor Unit (DPU).

    This is a high-end video terminal that can connect to a Datasaab E2500 mainframe system over a coaxial
    cable. It also has an unusual differential connector for its display (which operates at 50 Hz).

************************************************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/hd63450.h"


class e9161_state : public driver_device
{
public:
	e9161_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_dmac(*this, "dmac")
	{
	}

	void e9161(machine_config &config);

private:
	void mem_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<hd63450_device> m_dmac;
};


void e9161_state::mem_map(address_map &map)
{
	map(0x000000, 0x000007).rom().region("program", 0);
	map(0x018000, 0x0187ff).ram();
	map(0xc00000, 0xc03fff).rom().region("program", 0);
}


static INPUT_PORTS_START(e9161) // TODO: serial keyboard
INPUT_PORTS_END


void e9161_state::e9161(machine_config &config)
{
	M68000(config, m_maincpu, 8'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &e9161_state::mem_map);

	HD63450(config, m_dmac, 8'000'000, m_maincpu);
}


// CPU: MC68000G8
// DMAC: HD68450-8
// DRAM: 16 x M5K4164ANP-12
// DRAM controller: TMS4500A-15NL
// SIO: MK68564N-4A
// CRTC: Am8052-6LC (under heatsink)
// XTALs: 19.1700 MHz (middle of board), ??.???? MHz (near video connector)
ROM_START(e9161)
	ROM_REGION16_BE(0x4000, "program", 0)
	ROM_LOAD16_BYTE("e3405_87080_7403.bin", 0x0000, 0x2000, CRC(97f72404) SHA1(ced003ce294cd7370051e1f774d5120062390647))
	ROM_LOAD16_BYTE("e3405_87080_7303.bin", 0x0001, 0x2000, CRC(ec94aec4) SHA1(f41ae1b7f04ca3a2d0def6ff9aad3ff41782589a))
ROM_END


COMP(198?, e9161, 0, 0, e9161, e9161, e9161_state, empty_init, "Ericsson", "9161 Display Processor Unit", MACHINE_IS_SKELETON)
