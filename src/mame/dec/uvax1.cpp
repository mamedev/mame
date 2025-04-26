// license:BSD-3-Clause
// copyright-holders:AJR
/*******************************************************************************

    Skeleton driver for DEC MicroVAX I ("Seahorse").

    The MicroVAX I processor occupies two interconnected PCBs: the M7135 data
    path module (DAP) and the M7136 memory controller (MCT). The LSI data path
    chip, developed by DEC in conjunction with Silicon Compilers, Inc., is not
    really a single-chip VAX CPU, since it relies on a PROM microcode store and
    numerous PLDs and support logic ICs. Though the memory controller module
    contains TLB/cache SRAMs and a few register files, main memory is provided
    separately on the 16-bit Q22 bus in the form of MSV11-P modules.

    There were two versions of the processor: one supporting only F_floating
    and D_floating types, the other supporting only F_floating and G_floating.

    Q-bus I/O options used in the MicroVAX I include DLVJ1, DZV11, DEQNA and
    RQDX1.

*******************************************************************************/

#include "emu.h"

//#include "bus/qbus/qbus.h"
#include "cpu/vax/vax.h"
#include "machine/scn_pci.h"

namespace {

class uvax1_state : public driver_device
{
public:
	uvax1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void uvax1(machine_config &config) ATTR_COLD;

private:
	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
};

void uvax1_state::mem_map(address_map &map)
{
	map(0x00000000, 0x00001fff).rom().region("bootrom", 0);
}

static INPUT_PORTS_START(uvax1)
INPUT_PORTS_END

void uvax1_state::uvax1(machine_config &config)
{
	KD32A(config, m_maincpu, 64_MHz_XTAL / 16); // cycle time is 250 ns
	m_maincpu->set_addrmap(AS_PROGRAM, &uvax1_state::mem_map);

	SCN2661C(config, "conuart", 5.0688_MHz_XTAL);
}

ROM_START(uvax1)
	ROM_REGION32_LE(0x2000, "bootrom", 0)
	ROM_LOAD("m7135_microvax_i_e78_2764.bin", 0x0000, 0x2000, CRC(8c6f01f0) SHA1(c3effa180faa7767b267b047d724871592466788))

	ROM_REGION64_LE(0x10000, "dap_proms", ROMREGION_ERASE00) // MB7144H or equivalent
	ROM_LOAD64_BYTE("dap0.bin", 0x0000, 0x2000, NO_DUMP)
	ROM_LOAD64_BYTE("dap1.bin", 0x0001, 0x2000, NO_DUMP)
	ROM_LOAD64_BYTE("dap2.bin", 0x0002, 0x2000, NO_DUMP)
	ROM_LOAD64_BYTE("dap3.bin", 0x0003, 0x2000, NO_DUMP)
	ROM_LOAD64_BYTE("dap4.bin", 0x0004, 0x2000, NO_DUMP)

	ROM_REGION64_LE(0x2000, "mct_proms", 0) // Am27S35DC
	ROM_LOAD64_BYTE("mct0.bin", 0x0000, 0x0400, NO_DUMP)
	ROM_LOAD64_BYTE("mct1.bin", 0x0001, 0x0400, NO_DUMP)
	ROM_LOAD64_BYTE("mct2.bin", 0x0002, 0x0400, NO_DUMP)
	ROM_LOAD64_BYTE("mct3.bin", 0x0003, 0x0400, NO_DUMP)
	ROM_LOAD64_BYTE("mct4.bin", 0x0004, 0x0400, NO_DUMP)
	ROM_LOAD64_BYTE("mct5.bin", 0x0005, 0x0400, NO_DUMP)
	ROM_LOAD64_BYTE("mct6.bin", 0x0006, 0x0400, NO_DUMP)
	ROM_LOAD64_BYTE("mct7.bin", 0x0007, 0x0400, NO_DUMP)
ROM_END

} // anonymous namespace

COMP(1984, uvax1, 0, 0, uvax1, uvax1, uvax1_state, empty_init, "Digital Equipment Corporation", "MicroVAX I", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
