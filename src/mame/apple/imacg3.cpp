// license:BSD-3-Clause
// copyright-holders:R. Belmont
/****************************************************************************

    imacg3.cpp
    iMac G3 (original hardware)
    Preliminary driver by R. Belmont

    This is a "New World" PCI-based machine.

    CPU: PowerPC 750 "G3" @ 233 MHz
    Memory controller/PCI bridge: Motorola MPC106 "Grackle"
    Video: ATI Rage IIc w/2MB VRAM, ATI Rage Pro Turbo w/6MB VRAM on revised model
    USB: OPTi 82C861 PCI/USB controller
    I/O: Paddington PCI I/O ASIC (see heathrow.cpp for details)
    RAM: 2 PC66 DIMM slots, max 128 MB

****************************************************************************/

#include "emu.h"
#include "cpu/powerpc/ppc.h"
#include "machine/dimm_spd.h"
#include "machine/input_merger.h"
#include "machine/mpc106.h"
#include "machine/pci.h"
#include "machine/pci-ide.h"
#include "machine/ram.h"
#include "cuda.h"
#include "heathrow.h"
#include "macadb.h"

class imac_state : public driver_device
{
public:
	void imac(machine_config &config);

	imac_state(const machine_config &mconfig, device_type type, const char *tag);

	required_device<cpu_device> m_maincpu;
	required_device<mpc106_host_device> m_mpc106;
	required_device<cuda_device> m_cuda;
	required_device<macadb_device> m_macadb;
	required_device<dimm_spd_device> m_dimm0, m_dimm1;
	required_device<ram_device> m_ram;

private:
	void imac_map(address_map &map);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	WRITE_LINE_MEMBER(cuda_reset_w)
	{
		m_maincpu->set_input_line(INPUT_LINE_HALT, state);
		m_maincpu->set_input_line(INPUT_LINE_RESET, state);
	}
};

imac_state::imac_state(const machine_config &mconfig, device_type type, const char *tag) :
	driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_mpc106(*this, "pci:00.0"),
	m_cuda(*this, "cuda"),
	m_macadb(*this, "macadb"),
	m_dimm0(*this, "dimm0"),
	m_dimm1(*this, "dimm1"),
	m_ram(*this, RAM_TAG)
{
}

void imac_state::machine_start()
{
	m_mpc106->set_ram_info((u8 *)m_ram->pointer(), m_ram->size());

	// start off disabling all of the DIMMs
	m_dimm0->set_dimm_size(dimm_spd_device::SIZE_SLOT_EMPTY);
	m_dimm1->set_dimm_size(dimm_spd_device::SIZE_SLOT_EMPTY);

// TODO: iMac queries DIMM0 twice, so we have to
// halve the DIMM size here to get the requested amount.
// Cuda 3.0 may fix this, since that's what it's expecting to talk to
	switch (m_ram->size())
	{
		case 32 * 1024 * 1024:
			m_dimm0->set_dimm_size(dimm_spd_device::SIZE_16_MIB);
			break;

		case 64 * 1024 * 1024:
			m_dimm0->set_dimm_size(dimm_spd_device::SIZE_32_MIB);
			break;

		case 128 * 1024 * 1024:
			m_dimm0->set_dimm_size(dimm_spd_device::SIZE_64_MIB);
			break;
	}
}

void imac_state::machine_reset()
{
	// the PPC can't be allowed to run until Cuda's ready
	m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
}

void imac_state::imac_map(address_map &map)
{
	map.unmap_value_high();
}

void imac_state::imac(machine_config &config)
{
	PPC604(config, m_maincpu, 66000000);    // actually PPC750
	m_maincpu->set_addrmap(AS_PROGRAM, &imac_state::imac_map);

	PCI_ROOT(config, "pci", 0);
	MPC106(config, m_mpc106, 0, mpc106_host_device::MAP_TYPE_B, "maincpu", "bootrom");

	paddington_device &paddington(PADDINGTON(config, "pci:10.0", 0));
	paddington.set_maincpu_tag("maincpu");

	MACADB(config, m_macadb, 15.6672_MHz_XTAL);

	CUDA(config, m_cuda, CUDA_341S0060);
	m_cuda->reset_callback().set(FUNC(imac_state::cuda_reset_w));
	m_cuda->linechange_callback().set(m_macadb, FUNC(macadb_device::adb_linechange_w));
	m_cuda->via_clock_callback().set(paddington, FUNC(heathrow_device::cb1_w));
	m_cuda->via_data_callback().set(paddington, FUNC(heathrow_device::cb2_w));
	m_macadb->adb_data_callback().set(m_cuda, FUNC(cuda_device::set_adb_line));
	config.set_perfect_quantum(m_maincpu);

	paddington.pb3_callback().set(m_cuda, FUNC(cuda_device::get_treq));
	paddington.pb4_callback().set(m_cuda, FUNC(cuda_device::set_byteack));
	paddington.pb5_callback().set(m_cuda, FUNC(cuda_device::set_tip));
	paddington.cb2_callback().set(m_cuda, FUNC(cuda_device::set_via_data));

	// ALL_HIGH logically ANDs all sources, which is what we want for I2C/SMBus
	input_merger_device &sda_merger(INPUT_MERGER_ALL_HIGH(config, "sda"));
	sda_merger.output_handler().append(m_cuda, FUNC(cuda_device::set_iic_sda));

	m_cuda->iic_sda_callback().set(sda_merger, FUNC(input_merger_device::in_w<0>));
	m_cuda->iic_sda_callback().append(m_dimm0, FUNC(dimm_spd_device::sda_write));
	m_cuda->iic_sda_callback().append(m_dimm1, FUNC(dimm_spd_device::sda_write));

	DIMM_SPD(config, m_dimm0).set_address(0x50);
	m_cuda->iic_scl_callback().set(m_dimm0, FUNC(dimm_spd_device::scl_write));
	m_dimm0->sda_callback().set(sda_merger, FUNC(input_merger_device::in_w<1>));

	DIMM_SPD(config, m_dimm1).set_address(0x51);
	m_cuda->iic_scl_callback().append(m_dimm1, FUNC(dimm_spd_device::scl_write));
	m_dimm1->sda_callback().set(sda_merger, FUNC(input_merger_device::in_w<2>));

	RAM(config, m_ram);
	m_ram->set_default_size("32M");
	m_ram->set_extra_options("32M,64M,128M");
}

ROM_START(imac)
	ROM_REGION(0x100000, "bootrom", ROMREGION_64BIT | ROMREGION_BE)
	ROM_LOAD( "imacboot.u3",  0x000000, 0x100000, CRC(80d3174b) SHA1(e7a0c71822ec1e08435099af87b38bc82d315ed5) )
ROM_END

static INPUT_PORTS_START(imac)
INPUT_PORTS_END

COMP(1998, imac, 0, 0, imac, imac, imac_state, empty_init, "Apple Computer", "iMac (Bondi blue)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
