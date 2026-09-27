// license:BSD-3-Clause
// copyright-holders:wurthless-elektroniks, R. Belmont
/****************************************************************************

    Power Macintosh x360/x400 "Alchemy" hardware

    The "Alchemy" board is the replacement for the much maligned Cordyceps (aka "Elixir").
    It ditches the copypasted Quadra 630 architecture for a new PCI-based one
    centered around the "PSX" PCI/memory controller. The backplane is still based
    on the Quadra 630, though, so these boards could be used as drop-in replacements
    in the "Bongo" all-in-one case.

    The cheapo Valkyrie framebuffer returns, but as the "Valkyrie-AR".
    Its only difference seems to be that it works on 64-bit data addressing.

    Basic architecture:
    - PowerPC 603e
    - PSX (DRAM/ROM controller + Bandit PCI)
    - Valkyrie-AR (Valkyrie from Quadra 630 and Cordyceps but mapped differently)
    - O'Hare PCI-to-Mac I/O chip, same as in Power Mac 7500 "TNT"

    Scans PCI slots 0d:, 0e:, 0f:, 11: at boot.

    Current status: Boots to Finder. Setting certain video resolutions results
    in a black screen, which persists through reboots (NVRAM/OS configuration switch).
    Mac OS 8 and later run better than 7.6 and earlier, which have sound problems.

 ****************************************************************************/

#include "emu.h"

#include "awacs_macrisc.h"
#include "bandit.h"
#include "cuda.h"
#include "heathrow.h"
#include "valkyrie.h"

#include "bus/adb/adb.h"
#include "bus/adb/cards.h"
#include "bus/nscsi/cd.h"
#include "bus/nscsi/devices.h"
#include "bus/pci/pci_slot.h"
#include "cpu/powerpc/ppc.h"
#include "machine/input_merger.h"
#include "machine/nvram.h"
#include "machine/ram.h"
#include "machine/timer.h"

#include "softlist_dev.h"

#define LOG_NVRAM   (1U << 1)

#define VERBOSE (0)

//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

static constexpr u32 MAIN_BUS_FREQUENCY = 40'000'000;

namespace { // anonymous namespace

class pmac6400_state : public driver_device
{
public:
	pmac6400_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pci_root(*this, "pci"),
		m_psx(*this, "pci:00.0"),
		m_ohare(*this, "pci:10.0"),
		m_video(*this, "valkyrie"),
		m_adbbus(*this, "adb"),
		m_cuda(*this, "cuda"),
		m_ram(*this, RAM_TAG),
		m_nvram(*this, "nvram")
	{
	}


	void pmac6400(machine_config &config);

	void pmac6400_map(address_map &map) ATTR_COLD;

	void init_pmac6400();
private:
	required_device<ppc603e_device> m_maincpu;
	required_device<pci_root_device> m_pci_root;
	required_device<applpsx_host_device> m_psx;
	required_device<ohare_device> m_ohare;
	required_device<valkyrie_device> m_video;
	required_device<adb_bus_device> m_adbbus;
	required_device<cuda_device> m_cuda;
	required_device<ram_device> m_ram;
	required_device<nvram_device> m_nvram;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void cuda_reset_w(int state)
	{
		m_maincpu->set_input_line(INPUT_LINE_HALT, state);
		m_maincpu->set_input_line(INPUT_LINE_RESET, state);
	}

	u32 machine_id_r(offs_t offset, u32 mem_mask);

	void slot_irq_handler(int line, int state);

	u16 m_machine_id;

	u8 m_nvram_data[0x2000];
};

void pmac6400_state::machine_start()
{
	m_nvram->set_base(&m_nvram_data[0], sizeof(m_nvram_data));
	m_pci_root->set_irq_handler(pci_irq_handler(*this, FUNC(pmac6400_state::slot_irq_handler)));

	address_space &space = m_maincpu->space(AS_PROGRAM);
	space.install_ram(0x00000000, m_ram->size() - 1, m_ram->pointer());
}

void pmac6400_state::machine_reset()
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
}

void pmac6400_state::init_pmac6400()
{
}

u32 pmac6400_state::machine_id_r(offs_t offset, u32 mem_mask)
{
	// same as catalyst machineregister
	// 0xE0xx = pmac6400, 0xF0xx = pmac5400
	return m_machine_id;
}

void pmac6400_state::slot_irq_handler(int line, int state)
{
	// TODO: determine how PCI IRQs fire on this board.
	// remember that valkyrie takes up IRQ 0x18 even though it's not a PCI device.
}

/***************************************************************************
    ADDRESS MAPS
***************************************************************************/


void pmac6400_state::pmac6400_map(address_map &map)
{
	map(0x00000000, 0xffffffff).m(m_video, FUNC(valkyrie_device::valkyriear_map));
	map(0xffc00000, 0xffffffff).rom().region("bootrom", 0);
}

static INPUT_PORTS_START( macadb )
INPUT_PORTS_END

void pmac6400_state::pmac6400(machine_config &config)
{
	m_machine_id = 0xE03F;

	PPC603E(config, m_maincpu, 180'000'000);
	m_maincpu->ppcdrc_set_options(PPCDRC_COMPATIBLE_OPTIONS | PPCDRC_MACOS_CACHE_HACK);
	m_maincpu->set_addrmap(AS_PROGRAM, &pmac6400_state::pmac6400_map);
	m_maincpu->set_bus_frequency(MAIN_BUS_FREQUENCY);
	m_maincpu->set_tb_divisor(4);
	config.set_perfect_quantum(m_maincpu);

	PCI_ROOT(config, m_pci_root, 0);
	APPLPSX(config, m_psx, MAIN_BUS_FREQUENCY, "maincpu");
	m_psx->set_dev_offset(1);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// 8 MB built-in, 2x DIMM slots; modules can be 8, 16, 32 or 64 MB.
	// note however that the PSX can support 5 memory banks.
	// the minimum these machines shipped with was 16mb, so use that.
	RAM(config, m_ram);
	m_ram->set_default_size("16M");
	m_ram->set_extra_options("8M,24M,40M,72M,104M,136M");

	OHARE(config, m_ohare);
	m_ohare->set_maincpu_tag("maincpu");
	m_ohare->irq_callback().set_inputline(m_maincpu, PPC_IRQ);
	m_ohare->iobus_a_r_callback().set(FUNC(pmac6400_state::machine_id_r));

	m_ohare->ata(0).slot(0).set_default_option("hdd");

	awacs_macrisc_device &awacs(AWACS_MACRISC(config, "codec", 45.1584_MHz_XTAL / 2));
	awacs.dma_output().set(m_ohare, FUNC(ohare_device::codec_dma_read));
	awacs.dma_input().set(m_ohare, FUNC(ohare_device::codec_dma_write));
	m_ohare->codec_r_callback().set(awacs, FUNC(awacs_macrisc_device::read_macrisc));
	m_ohare->codec_w_callback().set(awacs, FUNC(awacs_macrisc_device::write_macrisc));

	// the SCSI bus and its MESH controller live inside the O'Hare
	NSCSI_CONNECTOR(config, "pci:10.0:scsi:0", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "pci:10.0:scsi:1", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "pci:10.0:scsi:2", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "pci:10.0:scsi:3").option_set("cdrom", NSCSI_CDROM_APPLE).machine_config(
			[] (device_t *device)
			{
				device->subdevice<cdda_device>("cdda")->add_route(0, "^^^^speaker", 1.0, 0);
				device->subdevice<cdda_device>("cdda")->add_route(1, "^^^^speaker", 1.0, 1);
			});
	NSCSI_CONNECTOR(config, "pci:10.0:scsi:4", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "pci:10.0:scsi:5", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "pci:10.0:scsi:6", default_scsi_devices, nullptr);

	SOFTWARE_LIST(config, "hdd_list").set_original("mac_hdd");
	SOFTWARE_LIST(config, "cd_list").set_original("mac_cdrom");

	VALKYRIE(config, m_video, 31.3344_MHz_XTAL);
	m_video->write_irq().set(m_ohare, FUNC(ohare_device::set_irq_line<0x18>));


	SPEAKER(config, "speaker", 2).front();
	awacs.add_route(0, "speaker", 1.0, 0);
	awacs.add_route(1, "speaker", 1.0, 1);

	ADB_BUS(config, m_adbbus);
	ADB_CONNECTOR(config, "adb:0", adb_devices, "hle_keyboard");
	ADB_CONNECTOR(config, "adb:1", adb_devices, "hle_mouse");

	CUDA_V2XX(config, m_cuda, XTAL(32'768));
	m_cuda->set_default_bios_tag("341s0060");
	m_cuda->reset_callback().set(FUNC(pmac6400_state::cuda_reset_w));
	m_cuda->linechange_callback().set(m_adbbus, FUNC(adb_bus_device::adb_host_line_w));
	m_cuda->via_clock_callback().set(m_ohare, FUNC(ohare_device::cb1_w));
	m_cuda->via_data_callback().set(m_ohare, FUNC(ohare_device::cb2_w));

	m_adbbus->out_adb_callback().set(m_cuda, FUNC(cuda_device::set_adb_line));
	m_adbbus->out_poweron_callback().set(m_cuda, FUNC(cuda_device::set_adb_power));

	m_ohare->pb3_callback().set(m_cuda, FUNC(cuda_device::get_treq));
	m_ohare->pb4_callback().set(m_cuda, FUNC(cuda_device::set_byteack));
	m_ohare->pb5_callback().set(m_cuda, FUNC(cuda_device::set_tip));
	m_ohare->cb2_callback().set(m_cuda, FUNC(cuda_device::set_via_data));
}

ROM_START( pmac6400 )
	ROM_REGION64_BE(0x400000, "bootrom", 0)
	ROM_LOAD( "6f5724c0.bin", 0x000000, 0x400000, CRC(ec9914be) SHA1(822ab19b360b8fa25238e531dcb85a4459f7c8be) )
ROM_END

} // anonymous namespace

//    YEAR  NAME      PARENT    COMPAT  MACHINE   INPUT   CLASS           INIT            COMPANY           FULLNAME                   FLAGS
COMP( 1996, pmac6400, 0,        0,      pmac6400, macadb, pmac6400_state, init_pmac6400,  "Apple Computer", "Performa 6400/180", MACHINE_NOT_WORKING)
