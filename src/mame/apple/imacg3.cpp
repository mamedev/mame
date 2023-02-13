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

****************************************************************************/

#include "emu.h"
#include "cpu/powerpc/ppc.h"
#include "machine/pci.h"
#include "machine/pci-ide.h"
#include "machine/mpc106.h"
#include "cuda.h"
#include "heathrow.h"

class imac_state : public driver_device
{
public:
	void imac(machine_config &config);

	imac_state(const machine_config &mconfig, device_type type, const char *tag);

	required_device<cpu_device> m_maincpu;
	required_device<cuda_device> m_cuda;

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
	m_cuda(*this, "cuda")
{
}

void imac_state::machine_start()
{
}

void imac_state::machine_reset()
{
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
	MPC106(config, "pci:00.0", 0, mpc106_host_device::MAP_TYPE_B, "maincpu", "bootrom", 32 * 1024 * 1024);

	paddington_device &paddington(PADDINGTON(config, "pci:10.0", 0));
	paddington.set_maincpu_tag("maincpu");

	CUDA(config, m_cuda, CUDA_341S0060);
	m_cuda->reset_callback().set(FUNC(imac_state::cuda_reset_w));
	m_cuda->via_clock_callback().set(paddington, FUNC(paddington_device::cb1_w));
	m_cuda->via_data_callback().set(paddington, FUNC(paddington_device::cb2_w));
	config.set_perfect_quantum(m_maincpu);

	paddington.pb3_callback().set(m_cuda, FUNC(cuda_device::get_treq));
	paddington.pb4_callback().set(m_cuda, FUNC(cuda_device::set_byteack));
	paddington.pb5_callback().set(m_cuda, FUNC(cuda_device::set_tip));
	paddington.cb2_callback().set(m_cuda, FUNC(cuda_device::set_via_data));
}

ROM_START(imac)
	ROM_REGION(0x100000, "bootrom", ROMREGION_64BIT | ROMREGION_BE)
	ROM_LOAD( "imacboot.u3",  0x000000, 0x100000, CRC(80d3174b) SHA1(e7a0c71822ec1e08435099af87b38bc82d315ed5) )
ROM_END

static INPUT_PORTS_START(imac)
INPUT_PORTS_END

COMP(1998, imac, 0, 0, imac, imac, imac_state, empty_init, "Apple Computer", "iMac (Bondi blue)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
