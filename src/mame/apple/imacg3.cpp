// license:BSD-3-Clause
// copyright-holders:R. Belmont
/****************************************************************************

    imacg3.cpp
    iMac G3 trayloader Gossamer-based hardware
    Preliminary driver by R. Belmont

    The original iMac, codenamed Columbus, was the first of the New World Macs,
    and was built to be a consumer product in the spirit of the original 128k Macintosh.
    It inherits its basic hardware design from the Gossamer (G3 Beige); however,
    with the New World ROM paradigm, its firmware, read off a 1 MiB flash chip,
    leaves out the bloated Macintosh Toolbox and forces the Classic Mac OS to load
    the Toolbox itself. This was in preparation for Mac OS X, which did not need
    a Toolbox ROM.

    With the New World paradigm came USB and the end of legacy devices, as the
    iMac does not have an ADB port, and, more famously, lacked a floppy drive
    at a time when floppies were still very popular. However, the design still
    keeps these around vestigially. The Cuda can still handle ADB commands
    and the Paddington retains an integrated SWIM3 floppy controller, though
    on the Columbus design, the SWIM3 connections just go to an unpopulated
    20-pin connector that, if populated, would only support Apple-style
    GCR drives. The unpopulated connector is J12, labeled in the schematics
    as "GCR FLOPPY CONNECTOR".

    The mezzanine connector:
    ======================================================================

    The Bondi Blue (rev A and rev B) machines have a mezzanine expansion slot
    that was officially only used by Apple for internal testing.

    The mezzanine slot, had Apple embraced it, would have allowed the trayloading iMacs
    to be greatly expanded. It would have allowed for up to three PCI devices
    (on addresses 0d, 0e, 0f), Rage video expansions like TV tuners, and external sound cards.

    The mezzanine slot was ultimately left unpopulated on the multicolored Lifesavers (rev C) revision,
    but that wasn't before third parties reverse engineered the slot's pinout and released
    expansion cards like Voodoo graphic accelerators.

    Basic hardware:
    ======================================================================

    CPU: PowerPC 750 "G3" @ 233 MHz (rev A/B), 266 MHz (rev C), 333 MHz (rev D)
    Memory controller/PCI bridge: Motorola MPC106 "Grackle"
    Video: ATI Rage IIc w/2MB VRAM with memory expansion slot (rev A),
           ATI Rage Pro Turbo w/6MB VRAM (rev B onward)
    USB: OPTi 82C861 PCI/USB controller
    I/O: Paddington PCI I/O ASIC (see heathrow.cpp for details)
    RAM: 2 PC66 DIMM slots, max 128 MB

    PCI addresses scanned: 14.0, 12.0, 0d.0, 0e.0, 0f.0

****************************************************************************/

#include "emu.h"

#include "bus/adb/adb.h"
#include "bus/adb/cards.h"
#include "bus/pci/opti82c861.h"
#include "cpu/powerpc/ppc.h"
#include "machine/dimm_spd.h"
#include "machine/i2cmem.h"
#include "machine/input_merger.h"
#include "machine/mpc106.h"
#include "machine/pci.h"
#include "machine/ram.h"
#include "sound/cdda.h"
#include "video/atirage.h"

#include "burgundy.h"
#include "cuda.h"
#include "heathrow.h"
#include "softlist.h"
#include "softlist_dev.h"

class imac_state : public driver_device
{
public:
	void imac(machine_config &config);

	imac_state(const machine_config &mconfig, device_type type, const char *tag);

	required_device<ppc_device> m_maincpu;
	required_device<pci_root_device> m_pci_root;
	required_device<mpc106_host_device> m_mpc106;
	required_device<cuda_device> m_cuda;
	required_device<burgundy_device> m_codec;
	required_device<adb_bus_device> m_adbbus;
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
		// The Cuda, on the trayloading iMacs and G3 Yosemite, controls reset for *all* devices
		if (state == ASSERT_LINE)
		{
			// On real hardware, the Paddington (PADD_RST3V_L), the onboard
			// PCI devices like the Rage, Ethernet PHY and USB (MLB_RST5V_L),
			// and the mezzanine connector (PCI_RST3V_L, PCI_RST5V_L) all have
			// separate reset signals. In MAME land we can just reset all devices
			// on the PCI bus.
			m_pci_root->reset();

			m_codec->reset(); // BURG_RST5VL
		}

		m_maincpu->set_input_line(INPUT_LINE_HALT, state);
		m_maincpu->set_input_line(INPUT_LINE_RESET, state); // MODRST_L signal
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
	m_pci_root(*this, "pci"),
	m_mpc106(*this, "pci:00.0"),
	m_cuda(*this, "cuda"),
	m_codec(*this, "codec"),
	m_adbbus(*this, "adb"),
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

	// The iMac probes DIMM0 through two Grackle bank numbers.
	// In the Columbus schematic the I2C bus goes through a 74FST3125 switch
	// which the Paddington controls from its MBINUSE pin. Until that's implemented,
	// we hack around it by reporting half the actual RAM size so that the bootrom
	// probes read the same DIMM values twice.
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
	PPC750(config, m_maincpu, 233'000'000);
	m_maincpu->ppcdrc_set_options(PPCDRC_COMPATIBLE_OPTIONS | PPCDRC_MACOS_CACHE_HACK);
	m_maincpu->set_bus_frequency(66'820'000);
	m_maincpu->set_tb_divisor(14);
	m_maincpu->set_addrmap(AS_PROGRAM, &imac_state::imac_map);

	PCI_ROOT(config, m_pci_root);
	MPC106(config, m_mpc106, mpc106_host_device::MAP_TYPE_B, "maincpu", "bootrom");

	// TODO: mezzanine slot.
	// PCI devices 0d,0e,0f, audio signal routing to the Burgundy, etc.

	paddington_device &paddington(PADDINGTON(config, "pci:10.0"));
	paddington.set_maincpu_tag("maincpu");
	paddington.irq_callback().set(FUNC(imac_state::irq_w));

	// ATA HDD on bus 0, ATAPI CD-ROM on bus 1
	paddington.ata(0).slot(0).set_default_option("hdd");
	paddington.ata(1).slot(0).set_default_option("cdrom");
	paddington.ata(1).slot(0).set_option_machine_config("cdrom", [](device_t *device)
	{
		device->subdevice<cdda_device>("cdda")->add_route(0, "^^^^speaker", 1.0, 0);
		device->subdevice<cdda_device>("cdda")->add_route(1, "^^^^speaker", 1.0, 1);
	});

	atirage_device &ati(ATI_RAGEIIC(config, "pci:12.0", 14.318181_MHz_XTAL));
	ati.gpio_get_cb().set(FUNC(imac_state::read_sense));
	ati.gpio_set_cb().set(FUNC(imac_state::write_sense));
	ati.irq_cb().set(paddington, FUNC(heathrow_device::set_irq_line<22>));
	ati.set_gpio_pullups(0x3000);   // bits 8 & 9 are the I2C bus

	I2C_24C01(config, m_edid);

	OPTI_82C861(config, "pci:14.0", 0);
	// TODO: USB controller IRQ goes to Paddington IRQ 8 (= PCI IRQ 28)

	// HACK: until USB is implemented we just attach ADB devices to the system.
	// Real hardware has no ADB port, but the Cuda still has ADB support internally.
	ADB_BUS(config, m_adbbus);
	ADB_CONNECTOR(config, "adb:0", adb_devices, "hle_keyboard");
	ADB_CONNECTOR(config, "adb:1", adb_devices, "hle_mouse");

	// TODO: Cuda is marked "CUDA 3.01" in the schematic.
	// The machine still boots with a Cuda 2 in its place.
	CUDA_V2XX(config, m_cuda, XTAL(32'768));
	m_cuda->set_default_bios_tag("341s0060");
	m_cuda->reset_callback().set(FUNC(imac_state::cuda_reset_w));
	m_cuda->linechange_callback().set(m_adbbus, FUNC(adb_bus_device::adb_host_line_w));
	m_cuda->via_clock_callback().set(paddington, FUNC(heathrow_device::cb1_w));
	m_cuda->via_data_callback().set(paddington, FUNC(heathrow_device::cb2_w));
	m_adbbus->out_adb_callback().set(m_cuda, FUNC(cuda_device::set_adb_line));
	m_adbbus->out_poweron_callback().set(m_cuda, FUNC(cuda_device::set_adb_power));
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

	BURGUNDY(config, m_codec, 45.1584_MHz_XTAL);
	m_codec->dma_output().set(paddington, FUNC(heathrow_device::codec_dma_read));
	m_codec->dma_input().set(paddington, FUNC(heathrow_device::codec_dma_write));

	paddington.codec_r_callback().set(m_codec, FUNC(burgundy_device::read_macrisc));
	paddington.codec_w_callback().set(m_codec, FUNC(burgundy_device::write_macrisc));

	SPEAKER(config, "speaker", 2).front();
	m_codec->add_route(0, "speaker", 1.0, 0);
	m_codec->add_route(1, "speaker", 1.0, 1);

	SOFTWARE_LIST(config, "hdd_list").set_original("mac_hdd");
	SOFTWARE_LIST(config, "cd_list").set_original("mac_cdrom");
}

ROM_START(imac)
	ROM_REGION(0x100000, "bootrom", ROMREGION_64BIT | ROMREGION_BE)
	ROM_LOAD( "imacboot.u3",  0x000000, 0x100000, CRC(80d3174b) SHA1(e7a0c71822ec1e08435099af87b38bc82d315ed5) )

	ROM_REGION(0x80, "edid", 0)
	ROM_LOAD( "imac_16843009.edid", 0x000000, 0x000080, CRC(ea3051d1) SHA1(a0b0338cdd8f364135e1496b9a33f7509f0bb104) )
ROM_END

static INPUT_PORTS_START(imac)
INPUT_PORTS_END

COMP(1998, imac, 0, 0, imac, imac, imac_state, empty_init, "Apple Computer", "iMac (Bondi blue)", MACHINE_NOT_WORKING )
