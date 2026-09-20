// license:BSD-3-Clause
// copyright-holders:R. Belmont
/****************************************************************************

	Power Macintosh x500 and Twentieth Anniversary Macintosh "Gazelle" hardware
	Emulation by R. Belmont

	Gazelle is a minor evolution of the Alchemy board, but it's a big upgrade.
	Gone is Valkyrie-AR, and in is the ATI 264GT 3D RAGE.

	Basic architecture:
	- PowerPC 603e
	- PSX+ (DRAM/ROM controller + Bandit PCI)
	- ATI 264GT 3D RAGE
	- O'Hare PCI-to-Mac I/O chip, same as in Power Mac 7500 "TNT"

	Machine IDs:
	0x30F0 - PM5500
	0x30E0 - PM6500
	0x70F0 - TAM

	Slot IRQs: (TODO)
	0x17 for slot 0xd, 0x19 for slot 0xe, 0x1c for slot 0xf,
	and 0x16 for the Comm Slot II.

 ****************************************************************************/

#include "emu.h"

#include "awacs_macrisc.h"
#include "bandit.h"
#include "cuda.h"
#include "heathrow.h"

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
#include "video/atirage.h"

#include "softlist_dev.h"

#define LOG_NVRAM   (1U << 1)

#define VERBOSE (0)

//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

static constexpr u32 MAIN_BUS_FREQUENCY = 50'000'000;

namespace { // anonymous namespace

class gazelle_state : public driver_device
{
public:
	gazelle_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pci_root(*this, "pci"),
		m_psx(*this, "pci:00.0"),
		m_ohare(*this, "pci:10.0"),
		m_video(*this, "pci:12.0"),
		m_adbbus(*this, "adb"),
		m_cuda(*this, "cuda"),
		m_ram(*this, RAM_TAG),
		m_nvram(*this, "nvram"),
		m_monitor_config(*this, "monitor")
	{
	}

	void gazelle(machine_config &config);
	void gaz250(machine_config &config);
	void gaz275(machine_config &config);
	void gaz300(machine_config &config);

	void pmac6500_map(address_map &map) ATTR_COLD;

	void init_pmac5500();
	void init_pmac6500();
	void init_tam();

private:
	required_device<ppc603e_device> m_maincpu;
	required_device<pci_root_device> m_pci_root;
	required_device<applpsx_host_device> m_psx;
	required_device<ohare_device> m_ohare;
	required_device<atirage_device> m_video;
	required_device<adb_bus_device> m_adbbus;
	required_device<cuda_device> m_cuda;
	required_device<ram_device> m_ram;
	required_device<nvram_device> m_nvram;
	required_ioport m_monitor_config;

	u16 m_sense;
	u8 m_nvram_data[0x2000];

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	u16 read_sense();
	void write_sense(u16 data);

	void cuda_reset_w(int state)
	{
		m_maincpu->set_input_line(INPUT_LINE_HALT, state);
		m_maincpu->set_input_line(INPUT_LINE_RESET, state);
	}

	void slot_irq_handler(int line, int state);
};

void gazelle_state::machine_start()
{
	m_nvram->set_base(&m_nvram_data[0], sizeof(m_nvram_data));
	m_pci_root->set_irq_handler(pci_irq_handler(*this, FUNC(gazelle_state::slot_irq_handler)));

	address_space &space = m_maincpu->space(AS_PROGRAM);
	space.install_ram(0x00000000, m_ram->size() - 1, m_ram->pointer());
}

void gazelle_state::machine_reset()
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
}

void gazelle_state::init_pmac5500()
{
	m_ohare->set_system_id(0x30f0);
}

void gazelle_state::init_pmac6500()
{
	m_ohare->set_system_id(0x30e0);
}

void gazelle_state::init_tam()
{
	m_ohare->set_system_id(0x70f0);
}

static constexpr u16 sense_to_gpio(u8 sense)
{
	return (BIT(sense, 0) << 8) | (BIT(sense, 1) << 12) | (BIT(sense, 2) << 13);
}

static constexpr u8 gpio_to_sense(u16 gpio)
{
	return BIT(gpio, 8) | (BIT(gpio, 12) << 1) | (BIT(gpio, 13) << 2);
}

u16 gazelle_state::read_sense()
{
	const u8 mon = m_monitor_config->read();

	u8 res;
	if (BIT(mon, 6))
	{
		res = BIT(mon, 7) ? 6 : 7;

		switch (gpio_to_sense(m_sense))
		{
		case 0b011: // sense 2 pulled low: sense 1 and 0 return extended bits 5 and 4
			res &= 4 | BIT(mon, 4, 2);
			break;

		case 0b101: // sense 1 pulled low: sense 2 and 0 return extended bits 3 and 2
			res &= (BIT(mon, 3) << 2) | 2 | BIT(mon, 2);
			break;

		case 0b110: // sense 0 pulled low: sense 2 and 1 return extended bits 1 and 0
			res &= (BIT(mon, 0, 2) << 1) | 1;
			break;
		}
	}
	else
	{
		res = mon;
	}

	return sense_to_gpio(res);
}

void gazelle_state::write_sense(u16 data)
{
	m_sense = data;
}

void gazelle_state::slot_irq_handler(int line, int state)
{
}

/***************************************************************************
    ADDRESS MAPS
***************************************************************************/


void gazelle_state::pmac6500_map(address_map &map)
{
	map(0xffc00000, 0xffffffff).rom().region("bootrom", 0);
}

static constexpr u8 ext(u8 bc, u8 ac, u8 ab)
{
	return 0x40 | (bc << 4) | (ac << 2) | ab;
}

static constexpr u8 ext6(u8 bc, u8 ac, u8 ab)
{
	return 0xc0 | (bc << 4) | (ac << 2) | ab;
}

static INPUT_PORTS_START( gazelle )
	PORT_START("monitor")
	PORT_CONFNAME(0xff, 0x06, "Monitor type")
	PORT_CONFSETTING(0x00, u8"Mac 21\" Color Display (1152\u00d7870)")          // "RGB 2 Page" or "Kong"
	PORT_CONFSETTING(0x01, u8"Mac Portrait Display (B&W 15\" 640\u00d7870)")    // "Full Page" or "Portrait"
	PORT_CONFSETTING(0x02, u8"Mac RGB Display (12\" 512\u00d7384)")             // "Rubik" (modified IIgs AppleColor RGB)
	PORT_CONFSETTING(0x03, u8"Mac Two-Page Display (B&W 21\" 1152\u00d7870)")   // "2 Page"
	PORT_CONFSETTING(0x06, u8"Mac Hi-Res Display (12-14\" 640\u00d7480)")       // "High Res"
	PORT_CONFSETTING(ext(1, 1, 3), u8"640\u00d7480 VGA")
	PORT_CONFSETTING(ext(2, 3, 1), u8"832\u00d7624 16\" RGB")                   // "Goldfish" or "16 inch RGB"
	PORT_CONFSETTING(ext(3, 2, 2), u8"1024\u00d7768 19\" RGB")
	PORT_CONFSETTING(ext6(0, 0, 3), u8"Multiple Scan 14\"")
	PORT_CONFSETTING(ext6(0, 2, 3), u8"Multiple Scan 16\"")
	PORT_CONFSETTING(ext6(2, 0, 3), u8"Multiple Scan 21\"")
INPUT_PORTS_END

void gazelle_state::gazelle(machine_config &config)
{
	PPC603E(config, m_maincpu, 225'000'000);
	m_maincpu->ppcdrc_set_options(PPCDRC_COMPATIBLE_OPTIONS | PPCDRC_MACOS_CACHE_HACK);
	m_maincpu->set_addrmap(AS_PROGRAM, &gazelle_state::pmac6500_map);
	m_maincpu->set_bus_frequency(MAIN_BUS_FREQUENCY);
	m_maincpu->set_tb_divisor(4);
	config.set_perfect_quantum(m_maincpu);

	PCI_ROOT(config, m_pci_root, 0);
	APPLPSX(config, m_psx, MAIN_BUS_FREQUENCY, "maincpu");
	m_psx->set_dev_offset(1);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// 8 MB built-in, 2x DIMM slots; modules can be 8, 16, 32 or 64 MB.
	// note however that the PSX can support 5 memory banks.
	RAM(config, m_ram);
	m_ram->set_default_size("16M");
	m_ram->set_extra_options("24M,32M,40M,64M,72M,96M,128M");

	OHARE(config, m_ohare);
	m_ohare->set_maincpu_tag("maincpu");
	m_ohare->irq_callback().set_inputline(m_maincpu, PPC_IRQ);

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

	ATI_RAGEII(config, m_video, 14.318181_MHz_XTAL);
	m_video->gpio_get_cb().set(FUNC(gazelle_state::read_sense));
	m_video->gpio_set_cb().set(FUNC(gazelle_state::write_sense));
	m_video->set_gpio_pullups(0x3100); // the 3 monitor sense lines are open collector
	m_video->irq_cb().set(m_ohare, FUNC(ohare_device::set_irq_line<0x18>));

	SPEAKER(config, "speaker", 2).front();
	awacs.add_route(0, "speaker", 1.0, 0);
	awacs.add_route(1, "speaker", 1.0, 1);

	ADB_BUS(config, m_adbbus);
	ADB_CONNECTOR(config, "adb:0", adb_devices, "hle_keyboard");
	ADB_CONNECTOR(config, "adb:1", adb_devices, "hle_mouse");

	CUDA_V2XX(config, m_cuda, XTAL(32'768));
	m_cuda->set_default_bios_tag("341s0060");
	m_cuda->reset_callback().set(FUNC(gazelle_state::cuda_reset_w));
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

void gazelle_state::gaz250(machine_config &config)
{
	gazelle(config);
	m_maincpu->set_clock(250'000'000);
}
void gazelle_state::gaz275(machine_config &config)
{
	gazelle(config);
	m_maincpu->set_clock(275'000'000);
}

void gazelle_state::gaz300(machine_config &config)
{
	gazelle(config);
	m_maincpu->set_clock(300'000'000);
}

ROM_START( pmac6500 )
	ROM_REGION64_BE(0x400000, "bootrom", 0)
	ROM_LOAD("6e92fe08.rom", 0x000000, 0x400000, CRC(084646f4) SHA1(9c5fe05473650be61e582e42bc03ba4be6cf1072))
ROM_END

#define rom_pmac5500 rom_pmac6500
#define rom_pmac5500_250 rom_pmac6500
#define rom_pmac5500_275 rom_pmac6500
#define rom_pmac6500_250 rom_pmac6500
#define rom_pmac6500_275 rom_pmac6500
#define rom_pmac6500_300 rom_pmac6500
#define rom_pmac20th rom_pmac6500

} // anonymous namespace

// 5500: 225, 250, 275
// 6500: 225, 250, 275, 300
// TAM: 250

//    YEAR  NAME      PARENT    COMPAT  MACHINE   INPUT    CLASS           INIT            COMPANY           FULLNAME                   FLAGS
COMP( 1997, pmac6500, 0,        0,      gazelle,  gazelle, gazelle_state, init_pmac6500,  "Apple Computer", "Performa 6500/225", MACHINE_SUPPORTS_SAVE)
COMP( 1997, pmac6500_250, pmac6500, 0,  gaz250,   gazelle, gazelle_state, init_pmac6500,  "Apple Computer", "Performa 6500/250", MACHINE_SUPPORTS_SAVE)
COMP( 1997, pmac6500_275, pmac6500, 0,  gaz275,   gazelle, gazelle_state, init_pmac6500,  "Apple Computer", "Performa 6500/275", MACHINE_SUPPORTS_SAVE)
COMP( 1997, pmac6500_300, pmac6500, 0,  gaz300,   gazelle, gazelle_state, init_pmac6500,  "Apple Computer", "Performa 6500/300", MACHINE_SUPPORTS_SAVE)
COMP( 1997, pmac5500, pmac6500, 0,      gazelle,  gazelle, gazelle_state, init_pmac5500,  "Apple Computer", "Performa 5500/225", MACHINE_SUPPORTS_SAVE)
COMP( 1997, pmac5500_250, pmac6500, 0,  gaz250,   gazelle, gazelle_state, init_pmac5500,  "Apple Computer", "Performa 5500/250", MACHINE_SUPPORTS_SAVE)
COMP( 1997, pmac5500_275, pmac6500, 0,  gaz275,   gazelle, gazelle_state, init_pmac5500,  "Apple Computer", "Performa 5500/275", MACHINE_SUPPORTS_SAVE)
COMP( 1997, pmac20th, pmac6500, 0,      gaz250,   gazelle, gazelle_state, init_tam,       "Apple Computer", "Twentieth Anniversary Macintosh", MACHINE_SUPPORTS_SAVE)
