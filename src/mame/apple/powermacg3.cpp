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

#include "bus/adb/adb.h"
#include "bus/adb/cards.h"
#include "bus/nscsi/cd.h"
#include "bus/nscsi/devices.h"
#include "cpu/powerpc/ppc.h"
#include "machine/dimm_spd.h"
#include "machine/input_merger.h"
#include "machine/mpc106.h"
#include "machine/pci.h"
#include "machine/ram.h"
#include "sound/cdda.h"
#include "sound/tda7433.h"
#include "video/atirage.h"

#include "awacs_macrisc.h"
#include "cuda.h"
#include "heathrow.h"

#include "softlist.h"

class pwrmacg3_state : public driver_device
{
public:
	void pwrmacg3(machine_config &config);

	pwrmacg3_state(const machine_config &mconfig, device_type type, const char *tag);

	required_device<ppc_device> m_maincpu;
	required_device<mpc106_host_device> m_mpc106;
	required_device<cuda_device> m_cuda;
	required_device<adb_bus_device> m_adbbus;
	required_device<dimm_spd_device> m_dimm0, m_dimm1, m_dimm2;
	required_device<ram_device> m_ram;
	required_device<atirage_device> m_atirage;
	required_device<tda7433_device> m_tda7433;

private:
	required_ioport m_monitor_config;

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
	m_adbbus(*this, "adb"),
	m_dimm0(*this, "dimm0"),
	m_dimm1(*this, "dimm1"),
	m_dimm2(*this, "dimm2"),
	m_ram(*this, RAM_TAG),
	m_atirage(*this, "pci:12.0"),
	m_tda7433(*this, "tda7433"),
	m_monitor_config(*this, "monitor")
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

// The Apple monitor sense lines hang off the Rage's GPIO port: sense 0 on bit 8, sense 1
// on bit 12, and sense 2 on bit 13.  (Sense 1 and 2 double as the DDC clock and data.)
static constexpr u16 sense_to_gpio(u8 sense)
{
	return (BIT(sense, 0) << 8) | (BIT(sense, 1) << 12) | (BIT(sense, 2) << 13);
}

static constexpr u8 gpio_to_sense(u16 gpio)
{
	return BIT(gpio, 8) | (BIT(gpio, 12) << 1) | (BIT(gpio, 13) << 2);
}

u16 pwrmacg3_state::read_sense()
{
	const u8 mon = m_monitor_config->read();

	u8 res;
	if (BIT(mon, 6))
	{
		res = BIT(mon, 7) ? 6 : 7;

		switch (gpio_to_sense(m_sense))
		{
			case 0b011:     // sense 2 pulled low: sense 1 and 0 return extended bits 5 and 4
				res &= 4 | BIT(mon, 4, 2);
				break;

			case 0b101:     // sense 1 pulled low: sense 2 and 0 return extended bits 3 and 2
				res &= (BIT(mon, 3) << 2) | 2 | BIT(mon, 2);
				break;

			case 0b110:     // sense 0 pulled low: sense 2 and 1 return extended bits 1 and 0
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

void pwrmacg3_state::write_sense(u16 data)
{
	m_sense = data;
}

void pwrmacg3_state::pwrmacg3(machine_config &config)
{
	PPC750(config, m_maincpu, 233'000'000);
	m_maincpu->set_bus_frequency(66'820'000);
	m_maincpu->set_tb_divisor(14);
	m_maincpu->ppcdrc_set_options(PPCDRC_COMPATIBLE_OPTIONS | PPCDRC_MACOS_CACHE_HACK);
	m_maincpu->set_addrmap(AS_PROGRAM, &pwrmacg3_state::pwrmacg3_map);

	PCI_ROOT(config, "pci");
	MPC106(config, m_mpc106, mpc106_host_device::MAP_TYPE_B, "maincpu", "bootrom");

	heathrow_device &heathrow(HEATHROW(config, "pci:10.0"));
	heathrow.set_maincpu_tag("maincpu");

	// Apple's documentation says systems with the 4.0f2 ROM use a Rage II+, but
	// the 4.0f2 ROM won't init the Rage if the PCI ID is 4755 (II+), only 4754 (Rage II).
	ATI_RAGEII(config, m_atirage, 14.318181_MHz_XTAL);
	m_atirage->gpio_get_cb().set(FUNC(pwrmacg3_state::read_sense));
	m_atirage->gpio_set_cb().set(FUNC(pwrmacg3_state::write_sense));
	m_atirage->set_gpio_pullups(0x3100);    // the 3 monitor sense lines are open collector
	m_atirage->irq_cb().set(heathrow, FUNC(heathrow_device::set_irq_line<22>));

	ADB_BUS(config, m_adbbus);
	ADB_CONNECTOR(config, "adb:0", adb_devices, "hle_keyboard");
	ADB_CONNECTOR(config, "adb:1", adb_devices, "hle_mouse");

	CUDA_V2XX(config, m_cuda, XTAL(32'768));
	m_cuda->set_default_bios_tag("341s0060");
	m_cuda->reset_callback().set(FUNC(pwrmacg3_state::cuda_reset_w));
	m_cuda->linechange_callback().set(m_adbbus, FUNC(adb_bus_device::adb_host_line_w));
	m_cuda->via_clock_callback().set(heathrow, FUNC(heathrow_device::cb1_w));
	m_cuda->via_data_callback().set(heathrow, FUNC(heathrow_device::cb2_w));
	m_adbbus->out_adb_callback().set(m_cuda, FUNC(cuda_device::set_adb_line));
	m_adbbus->out_poweron_callback().set(m_cuda, FUNC(cuda_device::set_adb_power));
	config.set_perfect_quantum(m_maincpu);

	heathrow.irq_callback().set(FUNC(pwrmacg3_state::irq_w));
	heathrow.pb3_callback().set(m_cuda, FUNC(cuda_device::get_treq));
	heathrow.pb4_callback().set(m_cuda, FUNC(cuda_device::set_byteack));
	heathrow.pb5_callback().set(m_cuda, FUNC(cuda_device::set_tip));
	heathrow.cb2_callback().set(m_cuda, FUNC(cuda_device::set_via_data));

	heathrow.ata(0).slot(0).set_default_option("hdd");
	heathrow.ata(1).slot(0).set_default_option("cdrom");
	heathrow.ata(1).slot(0).set_option_machine_config("cdrom", [](device_t *device)
	{
		device->subdevice<cdda_device>("cdda")->add_route(0, "^^^^speaker", 1.0, 0);
		device->subdevice<cdda_device>("cdda")->add_route(1, "^^^^speaker", 1.0, 1);
	});

	// the SCSI bus and its MESH controller live inside the Heathrow
	NSCSI_CONNECTOR(config, "pci:10.0:scsi:0", default_scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "pci:10.0:scsi:1", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "pci:10.0:scsi:2", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "pci:10.0:scsi:3").option_set("cdrom", NSCSI_CDROM_APPLE).machine_config([](device_t *device)
																							{
		device->subdevice<cdda_device>("cdda")->add_route(0, "^^^^speaker", 1.0, 0);
		device->subdevice<cdda_device>("cdda")->add_route(1, "^^^^speaker", 1.0, 1); });
	NSCSI_CONNECTOR(config, "pci:10.0:scsi:4", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "pci:10.0:scsi:5", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "pci:10.0:scsi:6", default_scsi_devices, nullptr);

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

	SOFTWARE_LIST(config, "hdd_list").set_original("mac_hdd");
	SOFTWARE_LIST(config, "cd_list").set_original("mac_cdrom");

	screamer_device &screamer(SCREAMER(config, "codec", 45.1584_MHz_XTAL / 2));
	screamer.dma_output().set(heathrow, FUNC(heathrow_device::codec_dma_read));
	screamer.dma_input().set(heathrow, FUNC(heathrow_device::codec_dma_write));

	heathrow.codec_r_callback().set(screamer, FUNC(screamer_device::read_macrisc));
	heathrow.codec_w_callback().set(screamer, FUNC(screamer_device::write_macrisc));

	// The CODEC's line output goes through the TDA7433, which is what the Sound control
	// panel's "Main Volume" drives.  It hangs off the same Cuda I2C bus as the DIMMs.
	TDA7433(config, m_tda7433);
	m_cuda->iic_scl_callback().append(m_tda7433, FUNC(tda7433_device::scl_write));
	m_cuda->iic_sda_callback().append(m_tda7433, FUNC(tda7433_device::sda_write));
	m_tda7433->sda_callback().set(sda_merger, FUNC(input_merger_device::in_w<4>));

	SPEAKER(config, "speaker", 2).front();
	screamer.add_route(0, m_tda7433, 1.0, 0);
	screamer.add_route(1, m_tda7433, 1.0, 1);
	m_tda7433->add_route(0, "speaker", 1.0, 0);
	m_tda7433->add_route(1, "speaker", 1.0, 1);
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

static constexpr u8 ext(u8 bc, u8 ac, u8 ab)
{
	return 0x40 | (bc << 4) | (ac << 2) | ab;
}

static constexpr u8 ext6(u8 bc, u8 ac, u8 ab)
{
	return 0xc0 | (bc << 4) | (ac << 2) | ab;
}

static INPUT_PORTS_START(pwrmacg3)
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

COMP(1997, pwrmacg3, 0, 0, pwrmacg3, pwrmacg3, pwrmacg3_state, empty_init, "Apple Computer", "Power Macintosh G3", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
