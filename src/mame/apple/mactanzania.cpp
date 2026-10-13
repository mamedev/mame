// license:BSD-3-Clause
// copyright-holders:R. Belmont
/****************************************************************************

    Power Macintosh 4400 "Tanzania"hardware
    Emulation by R. Belmont

    Moving on from Gazelle we have this board, which was the basis for many
    of the Mac clones in addition to the official Apple model.

    Basic architecture:
    - PowerPC 603e
    - PSX+ (DRAM/ROM controller + Bandit PCI)
    - ATI 264VT 3D RAGE
    - O'Hare PCI-to-Mac I/O chip

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

static constexpr u32 MAIN_BUS_FREQUENCY = 40'000'000;

namespace { // anonymous namespace

class tanzania_state : public driver_device
{
public:
	tanzania_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pci_root(*this, "pci"),
		m_psx(*this, "pci:00.0"),
		m_ohare(*this, "pci:10.0"),
		m_video(*this, "pci:11.0"),
		m_adbbus(*this, "adb"),
		m_cuda(*this, "cuda"),
		m_ram(*this, RAM_TAG),
		m_nvram(*this, "nvram"),
		m_monitor_config(*this, "monitor")
	{
	}

	void pmac4400(machine_config &config);
	void pmac4400_200(machine_config &config);

	void pmac4400_map(address_map &map) ATTR_COLD;

	void init_pmac4400();
	void init_pmac4400_200();

private:
	required_device<ppc603e_device> m_maincpu;
	required_device<pci_root_device> m_pci_root;
	required_device<applpsx_host_device> m_psx;
	required_device<ohare_device> m_ohare;
	required_device<atimach64vt_device> m_video;
	required_device<adb_bus_device> m_adbbus;
	required_device<cuda_device> m_cuda;
	required_device<ram_device> m_ram;
	required_device<nvram_device> m_nvram;
	required_ioport m_monitor_config;

	u8 m_sense;
	u16 m_board_register;
	u8 m_nvram_data[0x2000];

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	u8 sense_lines();
	u16 read_gpio();
	void write_gpio(u16 data);
	u8 read_dac_gio();
	void write_dac_gio(u8 data);

	u32 board_register_r(offs_t offset, u32 mem_mask);

	void cuda_reset_w(int state)
	{
		m_maincpu->set_input_line(INPUT_LINE_HALT, state);
		m_maincpu->set_input_line(INPUT_LINE_RESET, state);
	}

	void slot_irq_handler(int line, int state);
};

void tanzania_state::machine_start()
{
	m_nvram->set_base(&m_nvram_data[0], sizeof(m_nvram_data));
	m_pci_root->set_irq_handler(pci_irq_handler(*this, FUNC(tanzania_state::slot_irq_handler)));

	address_space &space = m_maincpu->space(AS_PROGRAM);
	space.install_ram(0x00000000, m_ram->size() - 1, m_ram->pointer());

	m_sense = 7;
	save_item(NAME(m_sense));
}

void tanzania_state::machine_reset()
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
}

// The ROM rejects boards with an O'Hare box ID nibble of 0 or F.
// The model itself comes from the board register.
void tanzania_state::init_pmac4400()
{
	m_ohare->set_system_id(0x30e0);
	m_board_register = 0x443f; // Gestalt 514: 4400/160
}

void tanzania_state::init_pmac4400_200()
{
	m_ohare->set_system_id(0x30e0);
	m_board_register = 0x403f; // Gestalt 515: 4400/200 and 7220
}

// The monitor sense lines are spread over two of the VT's registers: sense 0 is GP_IO 9, and
// senses 1 and 2 are the DAC's GIO0 and GIO4 pins (DAC_CNTL).  m_sense has what the VT drives
// onto the three lines, which are open collector with pullups.
u8 tanzania_state::sense_lines()
{
	const u8 mon = m_monitor_config->read();

	u8 res;
	if (BIT(mon, 6))
	{
		res = BIT(mon, 7) ? 6 : 7;

		switch (m_sense)
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

	// a line that the VT is pulling low reads back low
	return res & m_sense;
}

// GP_IO 12 is looped back from the VT's own VSYNC output.  Open Firmware's ATY,264VT driver flips
// the sync polarity with the CRTC stopped and only reads the sense lines (and sets the ATY,Flags
// bit the Mac OS driver needs before it will) if this pin follows.  Presumably that tells
// Apple's DA-15 board apart from the VGA/DDC clones sharing this ROM.
u16 tanzania_state::read_gpio()
{
	return (BIT(sense_lines(), 0) << 9) | (m_video->vsync_r() << 12);
}

void tanzania_state::write_gpio(u16 data)
{
	m_sense = (m_sense & 6) | BIT(data, 9);
}

u8 tanzania_state::read_dac_gio()
{
	const u8 lines = sense_lines();
	return BIT(lines, 1) | (BIT(lines, 2) << 4);
}

void tanzania_state::write_dac_gio(u8 data)
{
	m_sense = (m_sense & 1) | (BIT(data, 0) << 1) | (BIT(data, 4) << 2);
}

// Board register on the O'Hare I/O bus, read as a little-endian 16-bit value
// bit 15: set = enter the ROM's serial Test Manager after POST
// bit 14: set = SWIM3 and floppy drive fitted (clear makes POST power down the floppy cell
//         and Open Firmware leave the swim3 node out of the device tree)
// bits 12-10: model.  Bit 10 clear is the 4400/200; with it set, bits 14, 12, and 11 pick the
//         4400/160 (14 set, 11 clear) or one of the clone boards sharing this ROM.
// bits 5-0: PRSNT1#/PRSNT2# from PCI slots C1, B1, and A1 (both high = slot empty)
u32 tanzania_state::board_register_r(offs_t offset, u32 mem_mask)
{
	return m_board_register;
}

void tanzania_state::slot_irq_handler(int line, int state)
{
}

/***************************************************************************
    ADDRESS MAPS
***************************************************************************/


void tanzania_state::pmac4400_map(address_map &map)
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

static INPUT_PORTS_START( pmac4400 )
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

void tanzania_state::pmac4400(machine_config &config)
{
	PPC603E(config, m_maincpu, 160'000'000);
	m_maincpu->ppcdrc_set_options(PPCDRC_COMPATIBLE_OPTIONS | PPCDRC_MACOS_CACHE_HACK);
	m_maincpu->set_addrmap(AS_PROGRAM, &tanzania_state::pmac4400_map);
	m_maincpu->set_bus_frequency(MAIN_BUS_FREQUENCY);
	m_maincpu->set_tb_divisor(4);
	config.set_perfect_quantum(m_maincpu);

	PCI_ROOT(config, m_pci_root, 0);
	APPLPSX(config, m_psx, MAIN_BUS_FREQUENCY, "maincpu");
	m_psx->set_dev_offset(1);
	m_psx->set_system_id(0x10020000);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	RAM(config, m_ram);
	m_ram->set_default_size("16M");
	m_ram->set_extra_options("24M,32M,40M,64M,72M,96M,128M");

	OHARE(config, m_ohare);
	m_ohare->set_maincpu_tag("maincpu");
	m_ohare->irq_callback().set_inputline(m_maincpu, PPC_IRQ);
	m_ohare->iobus_a_r_callback().set(FUNC(tanzania_state::board_register_r));

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

	ATI_MACH64VT(config, m_video, 14.318181_MHz_XTAL);
	m_video->gpio_get_cb().set(FUNC(tanzania_state::read_gpio));
	m_video->gpio_set_cb().set(FUNC(tanzania_state::write_gpio));
	m_video->set_gpio_pullups(0x0200); // the 3 monitor sense lines are open collector
	m_video->dac_gio_get_cb().set(FUNC(tanzania_state::read_dac_gio));
	m_video->dac_gio_set_cb().set(FUNC(tanzania_state::write_dac_gio));
	m_video->set_dac_gio_pullups(0x11);
	m_video->irq_cb().set(m_ohare, FUNC(ohare_device::set_irq_line<0x16>));

	SPEAKER(config, "speaker", 2).front();
	awacs.add_route(0, "speaker", 1.0, 0);
	awacs.add_route(1, "speaker", 1.0, 1);

	ADB_BUS(config, m_adbbus);
	ADB_CONNECTOR(config, "adb:0", adb_devices, "hle_keyboard");
	ADB_CONNECTOR(config, "adb:1", adb_devices, "hle_mouse");

	CUDA_V2XX(config, m_cuda, XTAL(32'768));
	m_cuda->set_default_bios_tag("341s0060");
	m_cuda->reset_callback().set(FUNC(tanzania_state::cuda_reset_w));
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

void tanzania_state::pmac4400_200(machine_config &config)
{
	pmac4400(config);
	m_maincpu->set_clock(200'000'000);
}

ROM_START( pmac4400 )
	ROM_REGION64_BE(0x400000, "bootrom", 0)
	ROM_LOAD( "58f03416.rom", 0x000000, 0x400000, CRC(46476ff4) SHA1(ae1ff7f7c8247ed686024184705b7e831c6194c0) )
ROM_END

#define rom_pmac4400_200 rom_pmac4400

} // anonymous namespace

//    YEAR  NAME          PARENT    COMPAT  MACHINE       INPUT     CLASS           INIT               COMPANY           FULLNAME                    FLAGS
COMP( 1996, pmac4400,     0,        0,      pmac4400,     pmac4400, tanzania_state, init_pmac4400,     "Apple Computer", "Power Macintosh 4400/160", MACHINE_SUPPORTS_SAVE)
COMP( 1997, pmac4400_200, pmac4400, 0,      pmac4400_200, pmac4400, tanzania_state, init_pmac4400_200, "Apple Computer", "Power Macintosh 4400/200", MACHINE_SUPPORTS_SAVE)
