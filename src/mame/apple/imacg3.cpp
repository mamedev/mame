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

    PCI addresses scanned: 14.0, 12.0, 0d.0, 0e.0, 0f.0

****************************************************************************/

#include "emu.h"
#include "bus/pci/opti82c861.h"
#include "cpu/powerpc/ppc.h"
#include "machine/dimm_spd.h"
#include "machine/i2cmem.h"
#include "machine/input_merger.h"
#include "machine/mpc106.h"
#include "machine/pci.h"
#include "machine/pci-ide.h"
#include "machine/ram.h"
#include "video/atirage.h"
#include "burgundy.h"
#include "cuda.h"
#include "heathrow.h"
#include "macadb.h"

class imac_state : public driver_device
{
public:
	void imac(machine_config &config);

	imac_state(const machine_config &mconfig, device_type type, const char *tag);

	required_device<ppc_device> m_maincpu;
	required_device<mpc106_host_device> m_mpc106;
	required_device<cuda_device> m_cuda;
	required_device<macadb_device> m_macadb;
	required_device<dimm_spd_device> m_dimm0, m_dimm1;
	required_device<i2c_24c01_device> m_edid;
	required_device<ram_device> m_ram;

private:
	u16 m_sense;

	void imac_map(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void cuda_reset_w(int state)
	{
		m_maincpu->set_input_line(INPUT_LINE_HALT, state);
		m_maincpu->set_input_line(INPUT_LINE_RESET, state);
	}

	void irq_w(int state)
	{
		m_maincpu->set_input_line(PPC_IRQ, state);
	}

	u16 read_sense();
	void write_sense(u16 data);
};

imac_state::imac_state(const machine_config &mconfig, device_type type, const char *tag) :
	driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_mpc106(*this, "pci:00.0"),
	m_cuda(*this, "cuda"),
	m_macadb(*this, "macadb"),
	m_dimm0(*this, "dimm0"),
	m_dimm1(*this, "dimm1"),
	m_edid(*this, "edid"),
	m_ram(*this, RAM_TAG)
{
}

void imac_state::machine_start()
{
	m_sense = 0;

	m_edid->set_address(0x50<<1);

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

	save_item(NAME(m_sense));
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

// EDID sense.  Rage sends a u32 where the top 16 bits are DDR, the bottom 16 bits are data.
// The return value only needs the data (the lower 16 bits) filled in.
u16 imac_state::read_sense()
{
	const u8 out = (m_sense >> 12) & 0xff;
	const u8 scl = ((out & 2) >> 1);
	const u8 sda = (out & 1);

	m_edid->write_sda(sda);
	m_edid->write_scl(scl);

	return (m_edid->read_sda() ? 0x1000 : 0) | (scl ? 0x2000 : 0);
}

void imac_state::write_sense(u16 data)
{
	m_sense = data;
}

void imac_state::imac(machine_config &config)
{
	PPC750(config, m_maincpu, 66000000);    // actually 233 MHz
	m_maincpu->ppcdrc_set_options(PPCDRC_COMPATIBLE_OPTIONS);
	m_maincpu->set_addrmap(AS_PROGRAM, &imac_state::imac_map);

	PCI_ROOT(config, "pci", 0);
	MPC106(config, m_mpc106, 0, mpc106_host_device::MAP_TYPE_B, "maincpu", "bootrom");

	paddington_device &paddington(PADDINGTON(config, "pci:10.0", 0));
	paddington.set_maincpu_tag("maincpu");
	paddington.irq_callback().set(FUNC(imac_state::irq_w));

	atirage_device &ati(ATI_RAGEIIC(config, "pci:12.0", 14.318181_MHz_XTAL));
	ati.gpio_get_cb().set(FUNC(imac_state::read_sense));
	ati.gpio_set_cb().set(FUNC(imac_state::write_sense));
	ati.set_gpio_pullups(0x3000);   // bits 8 & 9 are the I2C bus

	I2C_24C01(config, m_edid);

	OPTI_82C861(config, "pci:14.0", 0);

	MACADB(config, m_macadb, 15.6672_MHz_XTAL);

	CUDA_V2XX(config, m_cuda, XTAL(32'768));
	m_cuda->set_default_bios_tag("341s0060");
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

	burgundy_device &burgundy(BURGUNDY(config, "codec", 45.1584_MHz_XTAL / 2));
	burgundy.dma_output().set(paddington, FUNC(heathrow_device::codec_dma_read));

	paddington.codec_r_callback().set(burgundy, FUNC(burgundy_device::read_macrisc));
	paddington.codec_w_callback().set(burgundy, FUNC(burgundy_device::write_macrisc));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	burgundy.add_route(0, "lspeaker", 1.0);
	burgundy.add_route(1, "rspeaker", 1.0);
}

ROM_START(imac)
	ROM_REGION(0x100000, "bootrom", ROMREGION_64BIT | ROMREGION_BE)
	ROM_LOAD( "imacboot.u3",  0x000000, 0x100000, CRC(80d3174b) SHA1(e7a0c71822ec1e08435099af87b38bc82d315ed5) )

	ROM_REGION(0x80, "edid", 0)
	ROM_LOAD( "imac_16843009.edid", 0x000000, 0x000080, CRC(ea3051d1) SHA1(a0b0338cdd8f364135e1496b9a33f7509f0bb104) )
ROM_END

static INPUT_PORTS_START(imac)
INPUT_PORTS_END

COMP(1998, imac, 0, 0, imac, imac, imac_state, empty_init, "Apple Computer", "iMac (Bondi blue)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
