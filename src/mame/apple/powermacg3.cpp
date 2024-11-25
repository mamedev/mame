// license:BSD-3-Clause
// copyright-holders:R. Belmont
/****************************************************************************

    powermacg3.cpp
    PowerMac G3 (original beige hardware)
    Preliminary driver by R. Belmont

    The last desktop Old World Mac, with hardware very similar to the first
    New World machines.

    CPU: PowerPC 750 "G3" @ 233 MHz
    Memory controller/PCI bridge: Motorola MPC106 "Grackle"
    Video: ATI Rage II, ATI Rage Pro on rev. B, ATI Rage Pro Turbo on rev. C
    I/O: Heathrow PCI I/O ASIC (see heathrow.cpp for details)

****************************************************************************/

#include "emu.h"
#include "cpu/powerpc/ppc.h"
#include "machine/dimm_spd.h"
#include "machine/input_merger.h"
#include "machine/mpc106.h"
#include "machine/pci.h"
#include "machine/pci-ide.h"
#include "machine/ram.h"
#include "video/atirage.h"
#include "awacs_macrisc.h"
#include "cuda.h"
#include "heathrow.h"
#include "macadb.h"

class pwrmacg3_state : public driver_device
{
public:
	void pwrmacg3(machine_config &config);

	pwrmacg3_state(const machine_config &mconfig, device_type type, const char *tag);

	required_device<ppc_device> m_maincpu;
	required_device<mpc106_host_device> m_mpc106;
	required_device<cuda_device> m_cuda;
	required_device<macadb_device> m_macadb;
	required_device<dimm_spd_device> m_dimm0, m_dimm1, m_dimm2;
	required_device<ram_device> m_ram;
	required_device<atirage_device> m_atirage;

private:
	u16 m_sense;

	void pwrmacg3_map(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	u16 read_sense();
	void write_sense(u16 data);

	void cuda_reset_w(int state)
	{
		m_maincpu->set_input_line(INPUT_LINE_HALT, state);
		m_maincpu->set_input_line(INPUT_LINE_RESET, state);
	}

	void irq_w(int state)
	{
		m_maincpu->set_input_line(PPC_IRQ, state);
	}
};

pwrmacg3_state::pwrmacg3_state(const machine_config &mconfig, device_type type, const char *tag) :
	driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_mpc106(*this, "pci:00.0"),
	m_cuda(*this, "cuda"),
	m_macadb(*this, "macadb"),
	m_dimm0(*this, "dimm0"),
	m_dimm1(*this, "dimm1"),
	m_dimm2(*this, "dimm2"),
	m_ram(*this, RAM_TAG),
	m_atirage(*this, "pci:12.0")
{
}

void pwrmacg3_state::machine_start()
{
	m_sense = 0;

	m_mpc106->set_ram_info((u8 *)m_ram->pointer(), m_ram->size());

	// start off disabling all of the DIMMs
	m_dimm0->set_dimm_size(dimm_spd_device::SIZE_SLOT_EMPTY);
	m_dimm1->set_dimm_size(dimm_spd_device::SIZE_SLOT_EMPTY);
	m_dimm2->set_dimm_size(dimm_spd_device::SIZE_SLOT_EMPTY);

	switch (m_ram->size())
	{
		case 64*1024*1024:
			m_dimm0->set_dimm_size(dimm_spd_device::SIZE_64_MIB);
			break;

		case 96*1024*1024:
			m_dimm0->set_dimm_size(dimm_spd_device::SIZE_64_MIB);
			m_dimm1->set_dimm_size(dimm_spd_device::SIZE_32_MIB);
			break;

		case 128*1024*1024:
			m_dimm0->set_dimm_size(dimm_spd_device::SIZE_128_MIB);
			break;

		case 256*1024*1024:
			m_dimm0->set_dimm_size(dimm_spd_device::SIZE_256_MIB);
			break;
	}

	save_item(NAME(m_sense));
}

void pwrmacg3_state::machine_reset()
{
	// the PPC can't be allowed to run until Cuda's ready
	m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
}

void pwrmacg3_state::pwrmacg3_map(address_map &map)
{
	map.unmap_value_high();
}

u16 pwrmacg3_state::read_sense()
{
	// ID as a 640x480 13" for now
	return (6 << 8);
}

void pwrmacg3_state::write_sense(u16 data)
{
	m_sense = data;
}

void pwrmacg3_state::pwrmacg3(machine_config &config)
{
	PPC750(config, m_maincpu, 66000000);    // actually 233 MHz
	m_maincpu->ppcdrc_set_options(PPCDRC_COMPATIBLE_OPTIONS);
	m_maincpu->set_addrmap(AS_PROGRAM, &pwrmacg3_state::pwrmacg3_map);

	PCI_ROOT(config, "pci", 0);
	MPC106(config, m_mpc106, 0, mpc106_host_device::MAP_TYPE_B, "maincpu", "bootrom");

	heathrow_device &heathrow(HEATHROW(config, "pci:10.0", 0));
	heathrow.set_maincpu_tag("maincpu");

	// Apple's documentation says systems with the 4.0f2 ROM use a Rage II+, but
	// the 4.0f2 ROM won't init the Rage if the PCI ID is 4755 (II+), only 4754 (Rage II).
	atirage_device &ati(ATI_RAGEII(config, m_atirage, 14.318181_MHz_XTAL));
	ati.gpio_get_cb().set(FUNC(pwrmacg3_state::read_sense));
	ati.gpio_set_cb().set(FUNC(pwrmacg3_state::write_sense));

	MACADB(config, m_macadb, 15.6672_MHz_XTAL);

	CUDA_V2XX(config, m_cuda, XTAL(32'768));
	m_cuda->set_default_bios_tag("341s0060");
	m_cuda->reset_callback().set(FUNC(pwrmacg3_state::cuda_reset_w));
	m_cuda->linechange_callback().set(m_macadb, FUNC(macadb_device::adb_linechange_w));
	m_cuda->via_clock_callback().set(heathrow, FUNC(heathrow_device::cb1_w));
	m_cuda->via_data_callback().set(heathrow, FUNC(heathrow_device::cb2_w));
	m_macadb->adb_data_callback().set(m_cuda, FUNC(cuda_device::set_adb_line));
	config.set_perfect_quantum(m_maincpu);

	heathrow.irq_callback().set(FUNC(pwrmacg3_state::irq_w));
	heathrow.pb3_callback().set(m_cuda, FUNC(cuda_device::get_treq));
	heathrow.pb4_callback().set(m_cuda, FUNC(cuda_device::set_byteack));
	heathrow.pb5_callback().set(m_cuda, FUNC(cuda_device::set_tip));
	heathrow.cb2_callback().set(m_cuda, FUNC(cuda_device::set_via_data));

	// ALL_HIGH logically ANDs all sources, which is what we want for I2C/SMBus
	input_merger_device &sda_merger(INPUT_MERGER_ALL_HIGH(config, "sda"));
	sda_merger.output_handler().append(m_cuda, FUNC(cuda_device::set_iic_sda));

	m_cuda->iic_sda_callback().set(sda_merger, FUNC(input_merger_device::in_w<0>));
	m_cuda->iic_sda_callback().append(m_dimm0, FUNC(dimm_spd_device::sda_write));
	m_cuda->iic_sda_callback().append(m_dimm1, FUNC(dimm_spd_device::sda_write));
	m_cuda->iic_sda_callback().append(m_dimm2, FUNC(dimm_spd_device::sda_write));

	DIMM_SPD(config, m_dimm0).set_address(0x50);
	m_cuda->iic_scl_callback().set(m_dimm0, FUNC(dimm_spd_device::scl_write));
	m_dimm0->sda_callback().set(sda_merger, FUNC(input_merger_device::in_w<1>));

	DIMM_SPD(config, m_dimm1).set_address(0x51);
	m_cuda->iic_scl_callback().append(m_dimm1, FUNC(dimm_spd_device::scl_write));
	m_dimm1->sda_callback().set(sda_merger, FUNC(input_merger_device::in_w<2>));

	DIMM_SPD(config, m_dimm2).set_address(0x52);
	m_cuda->iic_scl_callback().append(m_dimm2, FUNC(dimm_spd_device::scl_write));
	m_dimm2->sda_callback().set(sda_merger, FUNC(input_merger_device::in_w<3>));

	RAM(config, m_ram);
	m_ram->set_default_size("64M");
	m_ram->set_extra_options("64M,96M,128M,256M");

	screamer_device &screamer(SCREAMER(config, "codec", 45.1584_MHz_XTAL / 2));
	screamer.dma_output().set(heathrow, FUNC(heathrow_device::codec_dma_read));

	heathrow.codec_r_callback().set(screamer, FUNC(screamer_device::read_macrisc));
	heathrow.codec_w_callback().set(screamer, FUNC(screamer_device::write_macrisc));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	screamer.add_route(0, "lspeaker", 1.0);
	screamer.add_route(1, "rspeaker", 1.0);
}

/*
    Config register for Gossamer beige G3 and all-in-one
    bit 15: 1 = SWIM3, 0 = PC style FDC
    bit 14: 1 = slow ROM, 0 = burstable ROM
    bit 12 = PCI slot C card present
    bit 10 = PCI slot B card present
    bit 8  = PCI slot A card present
    bits 7-5 = bus to CPU clock ratio (1 for 2:1)
    bit 4:  0 = all-in-one "Molar Mac", 1 = Desktop beige G3
    bits 3-1: bus speed (0=75 MHz, 1=70, 2=78.75, 3=invalid, 4=75, 5=60, 6=66.82, 7=83)
    bit 0:  must be 1 (burn-in diagnostics?)

    desktop = 0b1011111100111101;
    AIO     = 0b1001010100101101;
*/
ROM_START(pwrmacg3)
	ROM_REGION(0x1000000, "bootrom", ROMREGION_64BIT | ROMREGION_BE | ROMREGION_ERASEFF)
	ROM_LOAD( "pmacg3_79d68d63.bin", 0xc00000, 0x400000, CRC(74a3badf) SHA1(e7fc183f62addc6499350c727252d3348184955e) )

	// The Gossamer machine config register is at 0xFF000000, which is in the MPC106's ROM space.
	// So we're hacking it like this.  Hardware is assumed to operate similarly.
	ROM_FILL(0, 1, 0b10111111)  // bf
	ROM_FILL(1, 1, 0b00111101)  // 3d
	ROM_FILL(2, 1, 0b10111111)
	ROM_FILL(3, 1, 0b00111101)
	ROM_FILL(4, 1, 0b10111111)
	ROM_FILL(5, 1, 0b00111101)
	ROM_FILL(6, 1, 0b10111111)
	ROM_FILL(7, 1, 0b00111101)
ROM_END

static INPUT_PORTS_START(pwrmacg3)
INPUT_PORTS_END

COMP(1997, pwrmacg3, 0, 0, pwrmacg3, pwrmacg3, pwrmacg3_state, empty_init, "Apple Computer", "Power Macintosh G3", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
