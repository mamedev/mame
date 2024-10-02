// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*********************************************************************\
*
*   SGI IP22/IP24 Indigo2/Indy workstation
*
*  Known Issues:
*  - The MAC address is supplied by the NVRAM, requiring the user to
*    use "setenv -f eaddr 08:00:69:xx:yy:zz" from the Indy boot PROM
*    before any IRIX installers will proceed.
*  - The Gentoo Linux live CD hangs on starting the kernel.
*
*  Memory map:
*
*  00000000 - 0007ffff      Alias for first 512kbyte of RAM
*  00080000 - 0008ffff      EISA I/O space (VINO on Indy)
*  00090000 - 0009ffff      EISA I/O space Alias (pullups on Indy)
*  000a0000 - 07ffffff      EISA Memory
*  08000000 - 17ffffff      Low System Memory
*  18000000 - 1effffff      RESERVED - Unused
*  1f000000 - 1f3fffff      GIO64 - GFX
*  1f400000 - 1f5fffff      GIO64 - EXP0
*  1f600000 - 1f9fffff      GIO64 - EXP1 - Unused
*  1fa00000 - 1fa1ffff      Memory Controller
*  1fb00000 - 1fb7ffff      HPC3 CHIP1
*  1fb80000 - 1fbfffff      HPC3 CHIP0
*  1fc00000 - 1fffffff      BIOS
*  20000000 - 2fffffff      High System Memory
*  30000000 - 7fffffff      Reserved
*  80000000 - ffffffff      EISA Memory
*
*  IP22/IP24 has 2 pieces of PC-compatible hardware: the 8042 PS/2 keyboard/mouse
*  interface and the 8254 PIT.  Both are licensed cores embedded in the IOC custom chip.
*
*  References used:
*    MipsLinux: http://www.mips-linux.org/
*      linux-2.6.6/include/newport.h
*      linux-2.6.6/include/asm-mips/sgi/gio.h
*      linux-2.6.6/include/asm-mips/sgi/mc.h
*      linux-2.6.6/include/asm-mips/sgi/hpc3.h
*    NetBSD: http://www.netbsd.org/
*    gxemul: http://gavare.se/gxemul/
*
*  Gentoo LiveCD r5 boot instructions:
*  - Specify an appropriate LiveCD image at the command line.
*  - Enter the command interpreter and type "sashARCS". Press enter and
*    it will autoboot.
*
*  IRIX boot instructions:
*  - Specify an appropriate IRIX CD image at the command line.
*  - At the menu, choose either "run diagnostics" or "install
*    system software".
*
\*********************************************************************/

#include "emu.h"

#include "hpc3.h"
#include "ioc2.h"
#include "mc.h"
#include "vino.h"

#include "bus/gio64/gio64.h"
#include "bus/nscsi/cd.h"
#include "bus/nscsi/hd.h"
#include "cpu/mips/mips3.h"
#include "machine/ds1386.h"
#include "machine/edlc.h"
#include "machine/eepromser.h"
#include "machine/nscsi_bus.h"
#include "machine/saa7191.h"
#include "machine/wd33c9x.h"
#include "sound/cdda.h"

#include "emupal.h"
#include "screen.h"
#include "softlist.h"

#include "logmacro.h"


namespace {

class ip24_state : public driver_device
{
public:
	ip24_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_mem_ctrl(*this, "memctrl")
		, m_scsi_ctrl(*this, "scsibus:0:wd33c93")
		, m_edlc(*this, "edlc")
		, m_eeprom(*this, "eeprom")
		, m_hal2(*this, "hal2")
		, m_hpc3(*this, "hpc3")
		, m_ioc2(*this, "ioc2")
		, m_rtc(*this, "rtc")
		, m_vino(*this, "vino")
		, m_dmsd(*this, "dmsd")
		, m_gio64(*this, "gio64")
		, m_gio64_gfx(*this, "gio64_gfx")
		, m_gio64_exp0(*this, "gio64_exp0")
		, m_gio64_exp1(*this, "gio64_exp1")
	{
	}

	void ip24_base(machine_config &config, uint32_t system_clock);
	void ip24(machine_config &config, uint32_t system_clock);
	void indy_5015(machine_config &config);
	void indy_4613(machine_config &config);
	void indy_4610(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	template <uint32_t addr_base> uint64_t bus_error_r(offs_t offset, uint64_t mem_mask = ~0);
	template <uint32_t addr_base> void bus_error_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);

	uint8_t volume_r(offs_t offset);
	void volume_w(offs_t offset, uint8_t data);

	void ip24_map(address_map &map) ATTR_COLD;
	void ip24_base_map(address_map &map) ATTR_COLD;
	void pio0_map(address_map &map) ATTR_COLD;
	void pio1_map(address_map &map) ATTR_COLD;
	void pio2_map(address_map &map) ATTR_COLD;
	void pio3_map(address_map &map) ATTR_COLD;
	void pio5_map(address_map &map) ATTR_COLD;
	void pio6_map(address_map &map) ATTR_COLD;

	void wd33c93(device_t *device);

	static void scsi_devices(device_slot_interface &device);

	required_device<mips3_device> m_maincpu;
	required_device<sgi_mc_device> m_mem_ctrl;
	required_device<wd33c93b_device> m_scsi_ctrl;
	required_device<seeq80c03_device> m_edlc;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<hal2_device> m_hal2;
	required_device<hpc3_device> m_hpc3;
	required_device<ioc2_device> m_ioc2;
	required_device<ds1386_device> m_rtc;
	optional_device<vino_device> m_vino;
	optional_device<saa7191_device> m_dmsd;
	optional_device<gio64_device> m_gio64;
	optional_device<gio64_slot_device> m_gio64_gfx;
	optional_device<gio64_slot_device> m_gio64_exp0;
	optional_device<gio64_slot_device> m_gio64_exp1;

	uint8_t m_volume_l = 0;
	uint8_t m_volume_r = 0;
};

class ip22_state : public ip24_state
{
public:
	ip22_state(const machine_config &mconfig, device_type type, const char *tag)
		: ip24_state(mconfig, type, tag)
		, m_scsi_ctrl2(*this, "scsibus2:0:wd33c93")
	{
	}

	void indigo2_4415(machine_config &config);

private:
	uint32_t eisa_io_r();

	void wd33c93_2(device_t *device);

	void ip22_map(address_map &map) ATTR_COLD;
	void pio4_map(address_map &map) ATTR_COLD;
	void pio6_map(address_map &map) ATTR_COLD;

	required_device<wd33c93b_device> m_scsi_ctrl2;
};

template <uint32_t addr_base>
uint64_t ip24_state::bus_error_r(offs_t offset, uint64_t mem_mask)
{
	logerror("Bus error (read)\n");
	// FIXME: m_maincpu->bus_error();
	m_mem_ctrl->set_cpu_buserr(addr_base + (offset << 3), mem_mask);
	return 0;
}

template <uint32_t addr_base>
void ip24_state::bus_error_w(offs_t offset, uint64_t data, uint64_t mem_mask)
{
	logerror("Bus error (write)\n");
	// FIXME: m_maincpu->bus_error();
	m_mem_ctrl->set_cpu_buserr(addr_base + (offset << 3), mem_mask);
}

uint32_t ip22_state::eisa_io_r()
{
	return 0xffffffff;
}

uint8_t ip24_state::volume_r(offs_t offset)
{
	if (offset == 0)
		return m_volume_r;
	else
		return m_volume_l;
}

void ip24_state::volume_w(offs_t offset, uint8_t data)
{
	if (offset == 0)
	{
		m_volume_r = data;
		m_hal2->set_right_volume(data);
	}
	else
	{
		m_volume_l = data;
		m_hal2->set_left_volume(data);
	}
}

void ip24_state::ip24_base_map(address_map &map)
{
	map(0x1f000000, 0x1f9fffff).rw(m_gio64, FUNC(gio64_device::read), FUNC(gio64_device::write));
	map(0x1fa00000, 0x1fa1ffff).rw(m_mem_ctrl, FUNC(sgi_mc_device::read), FUNC(sgi_mc_device::write));
	map(0x1fb00000, 0x1fb7ffff).rw(FUNC(ip24_state::bus_error_r<0x1fb00000>), FUNC(ip24_state::bus_error_w<0x1fb00000>));
	map(0x1fb80000, 0x1fbfffff).m(m_hpc3, FUNC(hpc3_device::map));
	map(0x1fc00000, 0x1fc7ffff).rom().region("user1", 0);
}

void ip24_state::ip24_map(address_map &map)
{
	ip24_base_map(map);
	map(0x00080000, 0x0009ffff).rw(m_vino, FUNC(vino_device::read), FUNC(vino_device::write));
}

void ip24_state::pio0_map(address_map &map)
{
	map(0x00, 0xff).rw(m_hal2, FUNC(hal2_device::read), FUNC(hal2_device::write));
}

void ip24_state::pio1_map(address_map &map)
{
	map(0x00, 0xff).ram(); // hack
}

void ip24_state::pio2_map(address_map &map)
{
	map(0x00, 0x01).rw(FUNC(ip24_state::volume_r), FUNC(ip24_state::volume_w)).umask16(0x00ff);
}

void ip24_state::pio6_map(address_map &map)
{
	map(0x00, 0x2f).m("ioc2", FUNC(ioc2_guinness_device::map)).umask16(0x00ff);
}

void ip22_state::ip22_map(address_map &map)
{
	ip22_state::ip24_base_map(map);
	map(0x00080000, 0x0009ffff).r(FUNC(ip22_state::eisa_io_r));
}

void ip22_state::pio4_map(address_map &map)
{
	map(0x00, 0x0f).m("ioc2", FUNC(ioc2_full_house_device::int2_map)).umask16(0x00ff);
}

void ip22_state::pio6_map(address_map &map)
{
	map(0x00, 0x1f).m("ioc2", FUNC(ioc2_full_house_device::map)).umask16(0x00ff);
}

void ip24_state::machine_start()
{
	save_item(NAME(m_volume_l));
	save_item(NAME(m_volume_r));
}

void ip24_state::machine_reset()
{
	//m_maincpu->mips3drc_set_options(MIPS3DRC_COMPATIBLE_OPTIONS | MIPS3DRC_CHECK_OVERFLOWS);
}

static INPUT_PORTS_START( ip24 )
INPUT_PORTS_END

void ip24_state::wd33c93(device_t *device)
{
	device->set_clock(10000000);
	downcast<wd33c93b_device *>(device)->irq_cb().set(m_ioc2, FUNC(ioc2_device::scsi0_int_w));
	downcast<wd33c93b_device *>(device)->drq_cb().set(m_hpc3, FUNC(hpc3_device::scsi0_drq));
}

void ip24_state::scsi_devices(device_slot_interface &device)
{
	device.option_add("cdrom", NSCSI_CDROM_SGI);
	device.option_add("harddisk", NSCSI_HARDDISK);
	//device.set_option_machine_config("cdrom", cdrom_config);
}

static DEVICE_INPUT_DEFAULTS_START(ip22_mc)
	DEVICE_INPUT_DEFAULTS("VALID", 0x0f, 0x07)
DEVICE_INPUT_DEFAULTS_END

static DEVICE_INPUT_DEFAULTS_START(ip24_mc)
	DEVICE_INPUT_DEFAULTS("VALID", 0x0f, 0x03)
DEVICE_INPUT_DEFAULTS_END

void ip24_state::ip24_base(machine_config &config, uint32_t system_clock)
{
	SGI_MC(config, m_mem_ctrl, m_maincpu, m_eeprom, system_clock);
	m_mem_ctrl->int_dma_done_cb().set(m_ioc2, FUNC(ioc2_device::mc_dma_done_w));
	m_mem_ctrl->eisa_present().set_constant(1);

	NSCSI_BUS(config, "scsibus", 0);
	NSCSI_CONNECTOR(config, "scsibus:0").option_set("wd33c93", WD33C93B)
		.machine_config([this](device_t *device) { wd33c93(device); });
	NSCSI_CONNECTOR(config, "scsibus:1", scsi_devices, "harddisk", false);
	NSCSI_CONNECTOR(config, "scsibus:2", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsibus:3", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsibus:4", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsibus:5", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsibus:6", scsi_devices, "cdrom", false);
	NSCSI_CONNECTOR(config, "scsibus:7", scsi_devices, nullptr, false);

	// GIO64
	GIO64(config, m_gio64, m_maincpu);
	m_gio64->interrupt_cb<0>().set(m_ioc2, FUNC(ioc2_device::gio_int0_w));
	m_gio64->interrupt_cb<1>().set(m_ioc2, FUNC(ioc2_device::gio_int1_w));
	m_gio64->interrupt_cb<2>().set(m_ioc2, FUNC(ioc2_device::gio_int2_w));
	GIO64_SLOT(config, m_gio64_gfx, m_gio64, gio64_slot_device::GIO64_SLOT_GFX, gio64_cards, "xl24");
	GIO64_SLOT(config, m_gio64_exp0, m_gio64, gio64_slot_device::GIO64_SLOT_EXP0, gio64_cards, nullptr);
	GIO64_SLOT(config, m_gio64_exp1, m_gio64, gio64_slot_device::GIO64_SLOT_EXP1, gio64_cards, nullptr);

	SGI_HPC3(config, m_hpc3, m_hal2);
	m_hpc3->set_gio64_space(m_maincpu, AS_PROGRAM);
	m_hpc3->set_addrmap(hpc3_device::AS_PIO0, &ip24_state::pio0_map);
	m_hpc3->set_addrmap(hpc3_device::AS_PIO1, &ip24_state::pio1_map);
	m_hpc3->set_addrmap(hpc3_device::AS_PIO2, &ip24_state::pio2_map);
	m_hpc3->enet_intr_out_cb().set(m_ioc2, FUNC(ioc2_device::enet_int_w));
	m_hpc3->hd_rd_cb<0>().set(m_scsi_ctrl, FUNC(wd33c93b_device::indir_r));
	m_hpc3->hd_wr_cb<0>().set(m_scsi_ctrl, FUNC(wd33c93b_device::indir_w));
	m_hpc3->hd_dma_rd_cb<0>().set(m_scsi_ctrl, FUNC(wd33c93b_device::dma_r));
	m_hpc3->hd_dma_wr_cb<0>().set(m_scsi_ctrl, FUNC(wd33c93b_device::dma_w));
	m_hpc3->hd_reset_cb<0>().set(m_scsi_ctrl, FUNC(wd33c93b_device::reset_w));
	m_hpc3->bbram_rd_cb().set(m_rtc, FUNC(ds1386_device::data_r));
	m_hpc3->bbram_wr_cb().set(m_rtc, FUNC(ds1386_device::data_w));
	m_hpc3->eeprom_dati_cb().set(m_eeprom, FUNC(eeprom_serial_93cxx_device::do_read));
	m_hpc3->eeprom_dato_cb().set(m_eeprom, FUNC(eeprom_serial_93cxx_device::di_write));
	m_hpc3->eeprom_clk_cb().set(m_eeprom, FUNC(eeprom_serial_93cxx_device::clk_write));
	m_hpc3->eeprom_cs_cb().set(m_eeprom, FUNC(eeprom_serial_93cxx_device::cs_write));
	//m_hpc3->eeprom_pre_cb().set(m_eeprom, FUNC(eeprom_serial_93cxx_device::pre_write));
	m_hpc3->dma_complete_int_cb().set(m_ioc2, FUNC(ioc2_device::hpc_dma_done_w));

	SEEQ80C03(config, m_edlc);
	m_edlc->out_int_cb().set(m_hpc3, FUNC(hpc3_device::enet_intr_in_w));
	m_edlc->out_rxrdy_cb().set(m_hpc3, FUNC(hpc3_device::enet_rxrdy_w));
	m_hpc3->set_enet(m_edlc);

	SGI_HAL2(config, m_hal2);
	EEPROM_93C56_16BIT(config, m_eeprom);

	SOFTWARE_LIST(config, "sgi_mips").set_original("sgi_mips");
	SOFTWARE_LIST(config, "sgi_mips_hdd").set_original("sgi_mips_hdd");
}

void ip24_state::ip24(machine_config &config, uint32_t system_clock)
{
	ip24_base(config, system_clock);
	m_mem_ctrl->set_input_default(DEVICE_INPUT_DEFAULTS_NAME(ip24_mc));

	m_hpc3->set_addrmap(hpc3_device::AS_PIO6, &ip24_state::pio6_map);

	SGI_IOC2_GUINNESS(config, m_ioc2, m_maincpu);

	SAA7191(config, m_dmsd);
	VINO(config, m_vino);
	m_vino->set_gio64_space(m_maincpu, AS_PROGRAM);
	m_vino->i2c_data_out().set(m_dmsd, FUNC(saa7191_device::i2c_data_w));
	m_vino->i2c_data_in().set(m_dmsd, FUNC(saa7191_device::i2c_data_r));
	m_vino->i2c_stop().set(m_dmsd, FUNC(saa7191_device::i2c_stop_w));
	m_vino->interrupt_cb().set(m_ioc2, FUNC(ioc2_device::video_int_w));

	DS1386_8K(config, m_rtc, 32768);
}

void ip24_state::indy_5015(machine_config &config)
{
	constexpr uint32_t system_clock = 50'000'000;
	ip24(config, system_clock);
	R5000BE(config, m_maincpu, 3 * system_clock);
	m_maincpu->set_system_clock(system_clock);
	m_maincpu->set_icache_size(0x8000);
	m_maincpu->set_dcache_size(0x8000);
	m_maincpu->set_addrmap(AS_PROGRAM, &ip24_state::ip24_map);
}

void ip24_state::indy_4613(machine_config &config)
{
	constexpr uint32_t system_clock = 66'666'666;
	ip24(config, system_clock);
	R4600BE(config, m_maincpu, 2 * system_clock);
	m_maincpu->set_system_clock(system_clock);
	m_maincpu->set_icache_size(0x4000);
	m_maincpu->set_dcache_size(0x4000);
	m_maincpu->set_addrmap(AS_PROGRAM, &ip24_state::ip24_map);
}

void ip24_state::indy_4610(machine_config &config)
{
	constexpr uint32_t system_clock = 50'000'000;
	ip24(config, system_clock);
	R4600BE(config, m_maincpu, 2 * system_clock);
	m_maincpu->set_system_clock(system_clock);
	m_maincpu->set_icache_size(0x4000);
	m_maincpu->set_dcache_size(0x4000);
	m_maincpu->set_addrmap(AS_PROGRAM, &ip24_state::ip24_map);
}

void ip22_state::wd33c93_2(device_t *device)
{
	device->set_clock(10000000);
	downcast<wd33c93b_device *>(device)->irq_cb().set(m_ioc2, FUNC(ioc2_device::scsi1_int_w));
	downcast<wd33c93b_device *>(device)->drq_cb().set(m_hpc3, FUNC(hpc3_device::scsi1_drq));
}

void ip22_state::indigo2_4415(machine_config &config)
{
	constexpr uint32_t system_clock = 50'000'000;
	R4400BE(config, m_maincpu, 3 * system_clock);
	m_maincpu->set_system_clock(system_clock);
	m_maincpu->set_icache_size(0x4000);
	m_maincpu->set_dcache_size(0x4000);
	m_maincpu->set_addrmap(AS_PROGRAM, &ip22_state::ip22_map);

	ip24_base(config, system_clock);
	m_mem_ctrl->set_input_default(DEVICE_INPUT_DEFAULTS_NAME(ip22_mc));

	NSCSI_BUS(config, "scsibus2", 0);
	NSCSI_CONNECTOR(config, "scsibus2:0").option_set("wd33c93", WD33C93B)
		.machine_config([this](device_t *device) { wd33c93_2(device); });
	NSCSI_CONNECTOR(config, "scsibus2:1", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsibus2:2", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsibus2:3", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsibus2:4", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsibus2:5", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsibus2:6", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsibus2:7", scsi_devices, nullptr, false);

	m_hpc3->set_addrmap(hpc3_device::AS_PIO4, &ip22_state::pio4_map);
	m_hpc3->set_addrmap(hpc3_device::AS_PIO6, &ip22_state::pio6_map);
	m_hpc3->hd_rd_cb<1>().set(m_scsi_ctrl2, FUNC(wd33c93b_device::indir_r));
	m_hpc3->hd_wr_cb<1>().set(m_scsi_ctrl2, FUNC(wd33c93b_device::indir_w));
	m_hpc3->hd_dma_rd_cb<1>().set(m_scsi_ctrl2, FUNC(wd33c93b_device::dma_r));
	m_hpc3->hd_dma_wr_cb<1>().set(m_scsi_ctrl2, FUNC(wd33c93b_device::dma_w));
	m_hpc3->hd_reset_cb<1>().set(m_scsi_ctrl2, FUNC(wd33c93b_device::reset_w));

	SGI_IOC2_FULL_HOUSE(config, m_ioc2, m_maincpu);
	DS1286(config, m_rtc, 32768);
}

#define INDY_BIOS_FLAGS(bios) ROM_GROUPDWORD | ROM_BIOS(bios)

#define INDY_BIOS_R5K \
	ROM_SYSTEM_BIOS( 0, "b10", "Version 5.3 Rev B10 R4X00/R5000 Feb 12, 1996" ) \
	ROMX_LOAD( "ip24prom.070-9101-011.bin", 0x000000, 0x080000, CRC(5e7f7e3a) SHA1(ac663a4db68528e400487e98cbf901f620fb30ce), INDY_BIOS_FLAGS(0) )

#define INDY_BIOS_R4K \
	INDY_BIOS_R5K \
	ROM_SYSTEM_BIOS( 1, "b7", "Version 5.3 Rev B7 R4X00 IP24 Feb 16, 1995" ) \
	ROMX_LOAD( "ip24prom.070-9101-008.bin", 0x000000, 0x080000, CRC(ee0b55c4) SHA1(a752a4aef7e2c6086b8b0244e9f064861a11870f), INDY_BIOS_FLAGS(1) ) \
	ROM_SYSTEM_BIOS( 2, "b6", "Version 5.0 Rev B6 Sep 28, 1994" ) \
	ROMX_LOAD( "ip24prom.070-9101-007.bin", 0x000000, 0x080000, CRC(70d8d1b1) SHA1(ade54cd2ecb7064957f8602894f05685e2f4e8fb), INDY_BIOS_FLAGS(2) ) \
	ROM_SYSTEM_BIOS( 3, "b4", "Version 5.1.2 Rev B4 R4X00 Dec 9, 1993" ) \
	ROMX_LOAD( "ip24prom.070-9101-005.bin", 0x000000, 0x080000, CRC(f5e41008) SHA1(28b769b28218a1fcd0400dceef9a284dcfbdda5b), INDY_BIOS_FLAGS(3) )

/* SCC init ip225015
 * Channel A
 * 09 <- c0 Master Interrup Control: Force HW reset + enable SWI INTACK
 * 04 <- 44 Clocks: x16 mode, 1 stop bits, no parity
 * 03 <- c0 Receiver: 8 bit data, auto enables, Rx disabled
 * 05 <- e2 Transmitter: DTR set, 8 bit data, RTS set, Tx disabled
 * 0b <- 50 Clock Mode: TRxC: XTAL output, TRxC: Output, TxC from BRG, RxC from BRG
 * 0c <- 0a Low const BRG  3.6864Mhz CLK => 9600 baud
 * 0d <- 00 High Const BRG = (CLK / (2 x Desired Rate x BR Clock period)) - 2
 * 0e <- 01 Mics: BRG enable
 * 03 <- c1 Receiver: as above + Receiver enable
 * 05 <- ea Transmitter: as above + Transmitter enable
 *
 * Channel A and B init - only BRG low const differs
 * 09 <- 80 channel A reset
 * 04 <- 44 Clocks: x16 mode, 1 stop bits, no parity
 * 0f <- 81 External/Status Control: Break/Abort enabled, WR7 prime enabled
 * 07p<- 40 External read enable (RR9=WR3, RR4=WR4, RR5=WR5, RR14=WR7 and RR11=WR10)
 * 03 <- c0 Receiver: 8 bit data, auto enables, Rx disabled
 * 05 <- e2 Transmitter: DTR set, 8 bit data, RTS set, Tx disabled
 * 0b <- 50 Clock Mode: TRxC: XTAL output, TRxC: Output, TxC from BRG, RxC from BRG
 * 0e <- 00 Mics: BRG disable
 * 0c <- 0a/04 Low const BRG, 3.6864Mhz CLK => Chan A:9600 Chan B:38400
 * 0d <- 00 High Const BRG = (CLK / (2 x Desired Rate x BR Clock period)) - 2
 * 0e <- 01 Mics: BRG enable
 * 03 <- c1 Receiver: as above + Receiver enable
 * 05 <- ea Transmitter: as above + Transmitter enable
 * 00 <- 10 Reset External/status IE
*/

ROM_START( indy_4610 )
	ROM_REGION64_BE( 0x80000, "user1", 0 )
	INDY_BIOS_R4K
ROM_END

ROM_START( indy_4613 )
	ROM_REGION64_BE( 0x80000, "user1", 0 )
	INDY_BIOS_R4K
ROM_END

ROM_START( indy_5015 )
	ROM_REGION64_BE( 0x80000, "user1", 0 )
	INDY_BIOS_R5K
ROM_END

ROM_START( indigo2_4415 )
	ROM_REGION64_BE( 0x80000, "user1", 0 )
	ROM_SYSTEM_BIOS( 0, "e", "Version 5.3 Rev E R4X00 Jan 29, 1996" )
	ROMX_LOAD( "ip22prom.070-1367-012.bin", 0x000000, 0x080000, CRC(54460c16) SHA1(330d87b3a02a05fb49c85a569f6f84904587cb35), ROM_GROUPDWORD | ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "b4", "Version 5.1.2 Rev B4 R4X00 Dec 9, 1993" )
	ROMX_LOAD( "ip22prom.070-1367-002.bin", 0x000000, 0x080000, CRC(ae5ecd08) SHA1(422568ae95282ee23b2fe123267f9b915a1dc3dc), ROM_GROUPDWORD | ROM_BIOS(1) )
ROM_END

} // anonymous namespace


//    YEAR  NAME          PARENT     COMPAT  MACHINE       INPUT CLASS       INIT        COMPANY                 FULLNAME                   FLAGS
COMP( 1993, indy_4610,    0,         0,      indy_4610,    ip24, ip24_state, empty_init, "Silicon Graphics Inc", "Indy (R4600, 100MHz)",    MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NODEVICE_MICROPHONE )
COMP( 1993, indy_4613,    indy_4610, 0,      indy_4613,    ip24, ip24_state, empty_init, "Silicon Graphics Inc", "Indy (R4600, 133MHz)",    MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NODEVICE_MICROPHONE )
COMP( 1996, indy_5015,    indy_4610, 0,      indy_5015,    ip24, ip24_state, empty_init, "Silicon Graphics Inc", "Indy (R5000, 150MHz)",    MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NODEVICE_MICROPHONE )
COMP( 1993, indigo2_4415, 0,         0,      indigo2_4415, ip24, ip22_state, empty_init, "Silicon Graphics Inc", "Indigo2 (R4400, 150MHz)", MACHINE_NOT_WORKING )
