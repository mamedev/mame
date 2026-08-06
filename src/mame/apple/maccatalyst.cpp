// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  maccatalyst.cpp - Apple Power Macintosh 7200
  Driver by R. Belmont

  The 7200 had 4 speed grades: 75, 90, 100, and 120 MHz.
  The 100 and 120 MHz models were also available as the Power Macintosh 8200,
  in the 8100's case but with no other changes (still 3 PCI slots).

  Outside of the Pippin these are basically the simplest possible PCI Power
  Mac, featuring a DAFB follow-on with some QuickDraw acceleration.

****************************************************************************/

#include "emu.h"

#include "awacs_macrisc.h"
#include "bandit.h"
#include "cuda.h"
#include "heathrow.h"
#include "macadb.h"
#include "platinum.h"

#include "bus/nscsi/cd.h"
#include "bus/nscsi/devices.h"
#include "bus/pci/pci_slot.h"
#include "cpu/powerpc/ppc.h"
#include "machine/input_merger.h"
#include "machine/ncr53c90.h"
#include "machine/nvram.h"
#include "machine/ram.h"
#include "sound/cdda.h"

#include "endianness.h"
#include "softlist.h"
#include "speaker.h"

#define LOG_NVRAM   (1U << 1)

#define VERBOSE (0)

//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

class catalyst_state : public driver_device
{
public:
	void pm7200(machine_config &config);
	void pm7200_90(machine_config &config);
	void pm7200_100(machine_config &config);
	void pm7200_120(machine_config &config);

	catalyst_state(const machine_config &mconfig, device_type type, const char *tag);

private:
	void pm7200_map(address_map &map) ATTR_COLD;

	void cuda_reset_w(int state);

	u32 nvram_addr_r(offs_t offset, u32 mem_mask);
	void nvram_addr_w(offs_t offset, u32 data, u32 mem_mask);
	u32 nvram_data_r(offs_t offset, u32 mem_mask);
	void nvram_data_w(offs_t offset, u32 data, u32 mem_mask);

	u32 machine_id_r(offs_t offset, u32 mem_mask);

	void slot_irq_handler(int line, int state);

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	required_device<ppc_device> m_maincpu;
	required_device<pci_root_device> m_pci_root;
	required_device<bandit_host_device> m_bandit;
	required_device<grandcentral_device> m_grandcentral;
	required_device<cuda_device> m_cuda;
	required_device<platinum_device> m_platinum;
	required_device<nscsi_bus_device> m_scsibus;
	required_device<ncr53cf94_device> m_ncr53c94;
	required_device<macadb_device> m_macadb;
	required_device<nvram_device> m_nvram;
	required_device<ram_device> m_ram;

	u32 m_nvram_addr;
	u8 m_nvram_data[0x2000];
};

catalyst_state::catalyst_state(const machine_config &mconfig, device_type type, const char *tag) :
	driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_pci_root(*this, "pci"),
	m_bandit(*this, "pci:00.0"),
	m_grandcentral(*this, "pci:10.0"),
	m_cuda(*this, "cuda"),
	m_platinum(*this, "platinum"),
	m_scsibus(*this, "scsi"),
	m_ncr53c94(*this, "ncr53c94"),
	m_macadb(*this, "macadb"),
	m_nvram(*this, "nvram"),
	m_ram(*this, RAM_TAG)
{
}

void catalyst_state::machine_start()
{
	m_maincpu->space().install_ram(0, m_ram->mask(), 0, m_ram->pointer());

	m_nvram->set_base(&m_nvram_data[0], sizeof(m_nvram_data));
	m_platinum->set_maincpu_space(&m_maincpu->space(AS_PROGRAM));

	m_pci_root->set_irq_handler(pci_irq_handler(*this, FUNC(catalyst_state::slot_irq_handler)));
}

void catalyst_state::machine_reset()
{
	// the PPC can't run until Cuda's ready
	m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
}

void catalyst_state::pm7200_map(address_map &map)
{
	map(0xf100'0000, 0xf17f'ffff).rw(m_platinum, FUNC(platinum_device::vram_r), FUNC(platinum_device::vram_w));
	map(0xf800'0000, 0xf800'07ff).m(m_platinum, FUNC(platinum_device::map));

	map(0xffc0'0000, 0xffff'ffff).rom().region("bootrom", 0);
}

void catalyst_state::cuda_reset_w(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state);
	m_maincpu->set_input_line(INPUT_LINE_RESET, state);
}

u32 catalyst_state::nvram_addr_r(offs_t offset, u32 mem_mask)
{
	return m_nvram_addr;
}

void catalyst_state::nvram_addr_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_nvram_addr);
	LOGMASKED(LOG_NVRAM, "NVRAM addr: %08x (mask %08x)\n", m_nvram_addr, mem_mask);
}

u32 catalyst_state::nvram_data_r(offs_t offset, u32 mem_mask)
{
	offset >>= 2;
	// high address (page) plus the 5-bit index within the data window; mask to
	// the backing store so a stray address register can't run off the end
	const u32 addr = (offset + (m_nvram_addr << 5)) & (sizeof(m_nvram_data) - 1);
	LOGMASKED(LOG_NVRAM, "NVRAM read @ %x (nvram_addr %x offset %x)\n", addr, m_nvram_addr, offset);
	return m_nvram_data[addr];
}

void catalyst_state::nvram_data_w(offs_t offset, u32 data, u32 mem_mask)
{
	offset >>= 2;
	const u32 addr = (offset + (m_nvram_addr << 5)) & (sizeof(m_nvram_data) - 1);
	COMBINE_DATA(&m_nvram_data[addr]);
	LOGMASKED(LOG_NVRAM, "NVRAM write: %02x @ %x (nvram_addr %x offset %x)\n", data & 0xff, addr, m_nvram_addr, offset);
}

u32 catalyst_state::machine_id_r(offs_t offset, u32 mem_mask)
{
	return 0xe13f;  // bits 11-12 are box ID, 0x100 means no factory burn-in
}

void catalyst_state::slot_irq_handler(int line, int state)
{
	switch (line)
	{
		case 0:
			m_grandcentral->set_irq_line<0x17>(state);
			break;

		case 1:
			m_grandcentral->set_irq_line<0x18>(state);
			break;

		case 2:
			m_grandcentral->set_irq_line<0x19>(state);
			break;
	}
}

/* Input ports */
static INPUT_PORTS_START( pm7200 )
INPUT_PORTS_END

void catalyst_state::pm7200(machine_config &config)
{
	PPC601(config, m_maincpu, 75_MHz_XTAL);
	m_maincpu->ppcdrc_set_options(PPCDRC_COMPATIBLE_OPTIONS | PPCDRC_MACOS_CACHE_HACK);
	m_maincpu->set_addrmap(AS_PROGRAM, &catalyst_state::pm7200_map);

	PCI_ROOT(config, m_pci_root, 0);
	BANDIT(config, m_bandit, 50_MHz_XTAL, "maincpu");
	m_bandit->set_dev_offset(1);

	// TODO: some PCI cards in MAME hardcode x86 system details which would make this driver fail validation
#if 0
	PCI_SLOT(config, "pci:0d", pci_cards, 1, 0, 1, 2, 3, nullptr);
	PCI_SLOT(config, "pci:0e", pci_cards, 1, 1, 2, 3, 0, nullptr);
	PCI_SLOT(config, "pci:0f", pci_cards, 1, 2, 3, 0, 1, nullptr);
#endif

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	RAM(config, m_ram);
	m_ram->set_default_size("8M");
	m_ram->set_extra_options("16M, 32M, 64M");

	GRAND_CENTRAL(config, m_grandcentral);
	m_grandcentral->set_maincpu_tag("maincpu");
	m_grandcentral->irq_callback().set_inputline(m_maincpu, PPC_IRQ);
	m_grandcentral->iobus_a_r_callback().set(FUNC(catalyst_state::machine_id_r));
	m_grandcentral->iobus_d_r_callback().set(FUNC(catalyst_state::nvram_addr_r));
	m_grandcentral->iobus_d_w_callback().set(FUNC(catalyst_state::nvram_addr_w));
	m_grandcentral->iobus_f_r_callback().set(FUNC(catalyst_state::nvram_data_r));
	m_grandcentral->iobus_f_w_callback().set(FUNC(catalyst_state::nvram_data_w));

	awacs_macrisc_device &awacs(AWACS_MACRISC(config, "codec", 45.1584_MHz_XTAL / 2));
	awacs.dma_output().set(m_grandcentral, FUNC(heathrow_device::codec_dma_read));
	awacs.dma_input().set(m_grandcentral, FUNC(heathrow_device::codec_dma_write));

	m_grandcentral->codec_r_callback().set(awacs, FUNC(awacs_macrisc_device::read_macrisc));
	m_grandcentral->codec_w_callback().set(awacs, FUNC(awacs_macrisc_device::write_macrisc));

	PLATINUM(config, m_platinum, 50_MHz_XTAL);
	m_platinum->set_ram_tag(m_ram);
	m_platinum->irq_cb().set(m_grandcentral, FUNC(grandcentral_device::set_irq_line<30>));
	m_grandcentral->iobus_b_r_callback().set(m_platinum, FUNC(platinum_device::dacula_r));
	m_grandcentral->iobus_b_w_callback().set(m_platinum, FUNC(platinum_device::dacula_w));

	NSCSI_BUS(config, m_scsibus);
	NSCSI_CONNECTOR(config, "scsi:0", default_scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi:1", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3").option_set("cdrom", NSCSI_CDROM_APPLE).machine_config([](device_t *device)
																							{
			device->subdevice<cdda_device>("cdda")->add_route(0, "^^speaker", 1.0, 0);
			device->subdevice<cdda_device>("cdda")->add_route(1, "^^speaker", 1.0, 1); });
	NSCSI_CONNECTOR(config, "scsi:4", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", default_scsi_devices, nullptr);

	NCR53CF94(config, m_ncr53c94, 25_MHz_XTAL);
	m_scsibus->set_external_device(7, m_ncr53c94);
	m_ncr53c94->set_busmd(ncr53cf94_device::BUSMD_3);
	m_ncr53c94->drq_handler_cb().set(m_grandcentral, FUNC(grandcentral_device::scsi0_drq));
	m_ncr53c94->irq_handler_cb().set(m_grandcentral, FUNC(grandcentral_device::scsi0_irq));
	m_grandcentral->scsi0_r_callback().set(m_ncr53c94, FUNC(ncr53c94_device::read));
	m_grandcentral->scsi0_w_callback().set(m_ncr53c94, FUNC(ncr53c94_device::write));
	m_grandcentral->scsi0_dma_r_callback().set(m_ncr53c94, FUNC(ncr53c94_device::dma16_r));
	m_grandcentral->scsi0_dma_w_callback().set(m_ncr53c94, FUNC(ncr53c94_device::dma16_w));

	SOFTWARE_LIST(config, "flop_mac35_orig").set_original("mac_flop_orig");
	SOFTWARE_LIST(config, "flop_mac35_clean").set_original("mac_flop_clcracked");
	SOFTWARE_LIST(config, "flop35_list").set_original("mac_flop");
	SOFTWARE_LIST(config, "flop35hd_list").set_original("mac_hdflop");
	SOFTWARE_LIST(config, "hdd_list").set_original("mac_hdd");
	SOFTWARE_LIST(config, "cd_list").set_original("mac_cdrom").set_filter("PPC601");

	SPEAKER(config, "speaker", 2).front();
	awacs.add_route(0, "speaker", 1.0, 0);
	awacs.add_route(1, "speaker", 1.0, 1);

	MACADB(config, m_macadb, 15.6672_MHz_XTAL);

	CUDA_V2XX(config, m_cuda, XTAL(32'768));
	m_cuda->set_default_bios_tag("341s0060");
	m_cuda->reset_callback().set(FUNC(catalyst_state::cuda_reset_w));
	m_cuda->linechange_callback().set(m_macadb, FUNC(macadb_device::adb_linechange_w));
	m_cuda->via_clock_callback().set(m_grandcentral, FUNC(heathrow_device::cb1_w));
	m_cuda->via_data_callback().set(m_grandcentral, FUNC(heathrow_device::cb2_w));
	m_macadb->adb_data_callback().set(m_cuda, FUNC(cuda_device::set_adb_line));

	config.set_perfect_quantum(m_maincpu);

	m_grandcentral->pb3_callback().set(m_cuda, FUNC(cuda_device::get_treq));
	m_grandcentral->pb4_callback().set(m_cuda, FUNC(cuda_device::set_byteack));
	m_grandcentral->pb5_callback().set(m_cuda, FUNC(cuda_device::set_tip));
	m_grandcentral->cb2_callback().set(m_cuda, FUNC(cuda_device::set_via_data));
}

void catalyst_state::pm7200_90(machine_config &config)
{
	pm7200(config);

	m_maincpu->set_clock(90'000'000);
}

void catalyst_state::pm7200_100(machine_config &config)
{
	pm7200(config);

	m_maincpu->set_clock(100'000'000);
}

void catalyst_state::pm7200_120(machine_config &config)
{
	pm7200(config);

	m_maincpu->set_clock(120'000'000);
}

ROM_START( pmac7200 )
	ROM_REGION( 0x400000, "bootrom",  ROMREGION_64BIT | ROMREGION_BE )
	ROM_SYSTEM_BIOS(0, "default", "Version 28F2")
	ROMX_LOAD( "9630c688.bin", 0x000000, 0x400000, CRC(fdad904d) SHA1(3a436af3f06680d270d1325ed7787a7bb91d978f), ROM_BIOS(0) )

	ROM_SYSTEM_BIOS(1, "older", "Version 28F1")
	ROMX_LOAD( "96cd923d.bin", 0x000000, 0x400000, CRC(a39fb2b9) SHA1(6ddeabaa9faca21901d6448a3b8d23004219df35), ROM_BIOS(1) )
ROM_END

#define rom_pmac7200_90 rom_pmac7200
#define rom_pmac7200_100 rom_pmac7200
#define rom_pmac7200_120 rom_pmac7200

/*    YEAR  NAME          PARENT    COMPAT  MACHINE  INPUT   CLASS           INIT        COMPANY           FULLNAME        FLAGS */
COMP( 1995, pmac7200,     0,        0,       pm7200,  pm7200, catalyst_state, empty_init, "Apple Computer", "Power Macintosh 7200/75", MACHINE_SUPPORTS_SAVE)
COMP( 1995, pmac7200_90,  pmac7200, 0,    pm7200_90,  pm7200, catalyst_state, empty_init, "Apple Computer", "Power Macintosh 7200/90", MACHINE_SUPPORTS_SAVE)
COMP( 1996, pmac7200_100, pmac7200, 0,   pm7200_100,  pm7200, catalyst_state, empty_init, "Apple Computer", "Power Macintosh 7200/100", MACHINE_SUPPORTS_SAVE)
COMP( 1996, pmac7200_120, pmac7200, 0,   pm7200_120,  pm7200, catalyst_state, empty_init, "Apple Computer", "Power Macintosh 7200/120", MACHINE_SUPPORTS_SAVE)
