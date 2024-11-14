// license:BSD-3-Clause
// copyright-holders:Curt Coder

/*

    Luxor X37 prototype

    (Luxor DS90-10 + ABC 1600 video)

*/

#include "emu.h"
#include "softlist_dev.h"
#include "bus/nscsi/devices.h"
#include "cpu/m68000/m68010.h"
#include "formats/abc1600_dsk.h"
#include "imagedev/floppy.h"
#include "machine/e0516.h"
#include "machine/hd63450.h"
#include "machine/nmc9306.h"
#include "machine/ns32081.h"
#include "machine/nscsi_bus.h"
#include "machine/nscsi_cb.h"
#include "machine/wd_fdc.h"
#include "machine/z80scc.h"
#include "machine/z8536.h"
#include "abc1600_v.h"


namespace {

#define MC68010_TAG  "14m"
#define NS32081_TAG  "06o"
#define MC68450_TAG  "11m"
#define Z8536A_TAG   "06l"
#define NMC9306_TAG  "05k"
#define E050_16_TAG  "03j"
#define Z8530A_0_TAG "16m"
#define Z8530A_1_TAG "16h"
#define Z8530A_2_TAG "16o"
#define FD1797_TAG   "15g"

class x37_state : public driver_device
{
public:
	x37_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_cpu(*this, MC68010_TAG),
		m_fpu(*this, NS32081_TAG),
		m_dmac(*this, MC68450_TAG),
		m_cio(*this, Z8536A_TAG),
		m_nvram(*this, NMC9306_TAG),
		m_scc(*this, {Z8530A_0_TAG, Z8530A_1_TAG, Z8530A_2_TAG}),
		m_fdc(*this, FD1797_TAG),
		m_floppy(*this, FD1797_TAG":%u", 0U),
		m_sasi(*this, "sasi:7:scsicb")
	{ }

	void x37(machine_config &config);

private:
	required_device<m68000_base_device> m_cpu;
	required_device<ns32081_device> m_fpu;
	required_device<hd63450_device> m_dmac;
	required_device<z8536_device> m_cio;
	required_device<nmc9306_device> m_nvram;
	required_device_array<scc8530_device, 3> m_scc;
	required_device<fd1797_device> m_fdc;
	required_device_array<floppy_connector, 3> m_floppy;
	required_device<nscsi_callback_device> m_sasi;

	virtual void machine_reset() override ATTR_COLD;

	static void floppy_formats(format_registration &fr);

	void program_map(address_map &map) ATTR_COLD;
	void cpu_space_map(address_map &map) ATTR_COLD;
};

void x37_state::program_map(address_map &map)
{
	map(0x000000, 0x007fff).rom().region(MC68010_TAG, 0);
}

void x37_state::cpu_space_map(address_map &map)
{
	map(0xffff0, 0xfffff).m(m_cpu, FUNC(m68010_device::autovectors_map));
}

static INPUT_PORTS_START( x37 )
INPUT_PORTS_END

void x37_state::machine_reset()
{
	m_fpu->reset();
}

static void x37_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}

void x37_state::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_ABC1600_FORMAT);
}

void x37_state::x37(machine_config &config)
{
	// basic machine hardware
	M68010(config, m_cpu, 20'000'000/2);
	m_cpu->set_addrmap(AS_PROGRAM, &x37_state::program_map);
	m_cpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &x37_state::cpu_space_map);

	NS32081(config, m_fpu, 20'000'000/2);

	HD63450(config, m_dmac, 20'000'000/2, m_cpu);

	Z8536(config, m_cio, 6000000);
	NMC9306(config, m_nvram, 0);
	E0516(config, E050_16_TAG, 32'768);

	SCC8530N(config, m_scc[0], 6000000);
	SCC8530N(config, m_scc[1], 6000000);
	SCC8530N(config, m_scc[2], 6000000);

	FD1797(config, m_fdc, 16'000'000/16);

	FLOPPY_CONNECTOR(config, m_floppy[0], x37_floppies, nullptr, x37_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[1], x37_floppies, nullptr, x37_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[2], x37_floppies, "525qd", x37_state::floppy_formats).enable_sound(true);

	NSCSI_BUS(config, "sasi");
	NSCSI_CONNECTOR(config, "sasi:0", default_scsi_devices, "s1410");
	NSCSI_CONNECTOR(config, "sasi:7", default_scsi_devices, "scsicb", true)
		.option_add_internal("scsicb", NSCSI_CB);

	// video hardware
	ABC1600_MOVER(config, ABC1600_MOVER_TAG, 0);

	// software list
	SOFTWARE_LIST(config, "flop_list").set_original("x37_flop");
}

ROM_START( x37 )
	ROM_REGION( 0x8000, MC68010_TAG, 0 )
	ROM_LOAD( "x37.07o", 0x0000, 0x8000, CRC(d505e7e7) SHA1(a3ad839e47b1f71c394e5ce28bce199e5e4810d2) )

	ROM_REGION( 0xa28, "plds", 0 )
	ROM_LOAD( "pat8003.12l", 0x000, 0x104, CRC(7c7b6dd1) SHA1(ab98fe70d589273b6a0437a818d9ae4bf9319ad5) ) // SCC decoder and clock multiplexor control
	ROM_LOAD( "pat8031.05h", 0x104, 0x104, CRC(2836e65b) SHA1(305feb8dff7d6762f2ab50d25316ad43140456eb) ) // DS60 MAPPER CONTROL
	ROM_LOAD( "pat8032.07h", 0x208, 0x104, CRC(356118d2) SHA1(e8e1dc6accdb8f0de481b91aa844f4b95f967826) ) // DS60 MAIN FUNCTION ENCODER
	ROM_LOAD( "pat8033.06h", 0x30c, 0x104, CRC(5f61f902) SHA1(b151621af0d9e851437ef4e3a02ecb78a6e102dd) ) // DS60 RAM REFRESH AND MAPPER WRITE CONTROL
	ROM_LOAD( "pat8034.12g", 0x410, 0x104, CRC(1105f161) SHA1(1923c0c954d3c812197d40f51bf3f53a158f87db) ) // DS60 CIO, SCC, FDC AND BOOTPROM CONTROLLER
	ROM_LOAD( "pat8035.05g", 0x514, 0x104, CRC(f25be5d9) SHA1(ed51b5cedea34c81b8cbdefd994e13aabd44a036) ) // DS60 RAM DATA PATH CONTROL
	ROM_LOAD( "pat8036.07g", 0x618, 0x104, CRC(350ff68e) SHA1(2d239bf324209adc7677eeb76b22c476ae0e6523) ) // DS60 RAM CONTROL SEQUENCER
	ROM_LOAD( "pat8037.10k", 0x71c, 0x104, CRC(a0e818b3) SHA1(a0d49ba0f09e235b28037539044e133f777fa4c7) ) // DS60 SASI INTERFACE CONTROL
	ROM_LOAD( "pat8038.04n", 0x820, 0x104, CRC(46ff5ce3) SHA1(c4a9025162b623bfcb74ac52f39de25bd53e448b) ) // DS60 PARITY GENERATION/DETECTION CONTROL
	ROM_LOAD( "pat8039.12h", 0x924, 0x104, CRC(d3f6974f) SHA1(98dc1bac1c822fe7af0edd683acfc2e5c51f0451) ) // DS60 NS32081 FLOATING POINT PROCESSOR INTERFACE
ROM_END

} // anonymous namespace


COMP( 1985, x37, 0,      0,      x37, x37, x37_state, empty_init, "Luxor", "X37 (prototype)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
