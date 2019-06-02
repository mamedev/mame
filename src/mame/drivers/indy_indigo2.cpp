// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*********************************************************************\
*
*   SGI IP22/IP24 Indigo2/Indy workstation
*
*  Known Issues:
*  - The proper hookup for the MAC address is unknown, requiring
*    a fake MAC to be set up before any IRIX installers will proceed.
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
*  1fb00000 - 1fb1a7ff      HPC3 CHIP1
*  1fb80000 - 1fb9a7ff      HPC3 CHIP0
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

#include "bus/gio64/gio64.h"

#include "cpu/mips/r4000.h"

#include "machine/ds1386.h"
#include "machine/eepromser.h"
#include "machine/hpc3.h"
#include "machine/nscsi_bus.h"
#include "machine/nscsi_cd.h"
#include "machine/nscsi_hd.h"
#include "machine/sgi.h"
#include "machine/vino.h"
#include "machine/wd33c9x.h"

#include "sound/cdda.h"

#include "emupal.h"
#include "screen.h"

class ip24_state : public driver_device
{
public:
	ip24_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_mainram(*this, "mainram")
		, m_mem_ctrl(*this, "memctrl")
		, m_scsi_ctrl(*this, "scsibus:0:wd33c93")
		, m_eeprom(*this, "eeprom")
		, m_hal2(*this, "hal2")
		, m_hpc3(*this, "hpc3")
		, m_ioc2(*this, "ioc2")
		, m_rtc(*this, "rtc")
		, m_vino(*this, "vino")
		, m_gio64(*this, "gio64")
		, m_gio64_gfx(*this, "gio64_gfx")
		, m_gio64_exp0(*this, "gio64_exp0")
		, m_gio64_exp1(*this, "gio64_exp1")
	{
	}

	void ip24_base(machine_config &config);
	void ip24(machine_config &config);
	void indy_5015(machine_config &config);
	void indy_4613(machine_config &config);
	void indy_4610(machine_config &config);

protected:
	virtual void machine_reset() override;

	DECLARE_WRITE64_MEMBER(write_ram);
	DECLARE_READ32_MEMBER(bus_error);

	void ip24_map(address_map &map);
	void ip24_base_map(address_map &map);

	void wd33c93(device_t *device);

	static void scsi_devices(device_slot_interface &device);

	required_device<r4000_base_device> m_maincpu;
	required_shared_ptr<uint64_t> m_mainram;
	required_device<sgi_mc_device> m_mem_ctrl;
	required_device<wd33c93b_device> m_scsi_ctrl;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<hal2_device> m_hal2;
	required_device<hpc3_device> m_hpc3;
	required_device<ioc2_device> m_ioc2;
	required_device<ds1386_device> m_rtc;
	optional_device<vino_device> m_vino;
	optional_device<gio64_device> m_gio64;
	optional_device<gio64_slot_device> m_gio64_gfx;
	optional_device<gio64_slot_device> m_gio64_exp0;
	optional_device<gio64_slot_device> m_gio64_exp1;
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
	DECLARE_READ32_MEMBER(eisa_io_r);

	void wd33c93_2(device_t *device);

	void ip22_map(address_map &map);

	required_device<wd33c93b_device> m_scsi_ctrl2;
};

READ32_MEMBER(ip24_state::bus_error)
{
	m_maincpu->bus_error();
	return 0;
}

READ32_MEMBER(ip22_state::eisa_io_r)
{
	return 0xffffffff;
}

// a bit hackish, but makes the memory detection work properly and allows a big cleanup of the mapping
WRITE64_MEMBER(ip24_state::write_ram)
{
	// if banks 2 or 3 are enabled, do nothing, we don't support that much memory
	if (m_mem_ctrl->get_mem_config(1) & 0x10001000)
	{
		// a random perturbation so the memory test fails
		data ^= 0xffffffffffffffffULL;
	}

	// if banks 0 or 1 have 2 membanks, also kill it, we only want 128 MB
	if (m_mem_ctrl->get_mem_config(0) & 0x40004000)
	{
		// a random perturbation so the memory test fails
		data ^= 0xffffffffffffffffULL;
	}
	COMBINE_DATA(&m_mainram[offset]);
}

void ip24_state::ip24_base_map(address_map &map)
{
	map(0x00000000, 0x0007ffff).bankrw("bank1");    /* mirror of first 512k of main RAM */
	map(0x08000000, 0x0fffffff).share("mainram").ram().w(FUNC(ip24_state::write_ram));     /* 128 MB of main RAM */
	map(0x1f000000, 0x1f9fffff).rw(m_gio64, FUNC(gio64_device::read), FUNC(gio64_device::write));
	map(0x1fa00000, 0x1fa1ffff).rw(m_mem_ctrl, FUNC(sgi_mc_device::read), FUNC(sgi_mc_device::write));
	map(0x1fb00000, 0x1fb7ffff).r(FUNC(ip24_state::bus_error));
	map(0x1fb80000, 0x1fbfffff).m(m_hpc3, FUNC(hpc3_device::map));
	map(0x1fc00000, 0x1fc7ffff).rom().region("user1", 0);
	map(0x20000000, 0x27ffffff).share("mainram").ram().w(FUNC(ip24_state::write_ram));
}

void ip24_state::ip24_map(address_map &map)
{
	ip24_base_map(map);
	map(0x00080000, 0x0009ffff).rw(m_vino, FUNC(vino_device::read), FUNC(vino_device::write));
}

void ip22_state::ip22_map(address_map &map)
{
	ip22_state::ip24_base_map(map);
	map(0x00080000, 0x0009ffff).r(FUNC(ip22_state::eisa_io_r));
}

void ip24_state::machine_reset()
{
	// set up low RAM mirror
	membank("bank1")->set_base(m_mainram);

	//m_maincpu->mips3drc_set_options(MIPS3DRC_COMPATIBLE_OPTIONS | MIPS3DRC_CHECK_OVERFLOWS);
}

static INPUT_PORTS_START( ip24 )
	PORT_INCLUDE( at_keyboard )
INPUT_PORTS_END

void ip24_state::wd33c93(device_t *device)
{
	device->set_clock(10000000);
	downcast<wd33c93b_device *>(device)->irq_cb().set(m_hpc3, FUNC(hpc3_device::scsi0_irq));
	downcast<wd33c93b_device *>(device)->drq_cb().set(m_hpc3, FUNC(hpc3_device::scsi0_drq));
}

void ip24_state::scsi_devices(device_slot_interface &device)
{
	device.option_add("cdrom", NSCSI_CDROM_SGI);
	device.option_add("harddisk", NSCSI_HARDDISK);
	//device.set_option_machine_config("cdrom", cdrom_config);
}

void ip24_state::ip24_base(machine_config &config)
{
	SGI_MC(config, m_mem_ctrl, m_maincpu, m_eeprom);
	m_mem_ctrl->int_dma_done_cb().set(m_ioc2, FUNC(ioc2_device::mc_dma_done_w));

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

	SGI_HPC3(config, m_hpc3, m_ioc2, m_hal2);
	m_hpc3->set_gio64_space(m_maincpu, AS_PROGRAM);
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

	SGI_HAL2(config, m_hal2);
	EEPROM_93C56_16BIT(config, m_eeprom);
	DS1386_8K(config, m_rtc, 32768);
}

void ip24_state::ip24(machine_config &config)
{
	ip24_base(config);

	SGI_IOC2_GUINNESS(config, m_ioc2, m_maincpu);
	VINO(config, m_vino);
}

void ip24_state::indy_5015(machine_config &config)
{
	ip24(config);

	R4000(config, m_maincpu, 50000000*3);
	//m_maincpu->set_icache_size(32768);
	//m_maincpu->set_dcache_size(32768);
	m_maincpu->set_addrmap(AS_PROGRAM, &ip24_state::ip24_map);
}

void ip24_state::indy_4613(machine_config &config)
{
	ip24(config);

	R4600(config, m_maincpu, 33333333*4);
	//m_maincpu->set_icache_size(16384);
	//m_maincpu->set_dcache_size(16384);
	m_maincpu->set_addrmap(AS_PROGRAM, &ip24_state::ip24_map);
}

void ip24_state::indy_4610(machine_config &config)
{
	ip24(config);

	R4600(config, m_maincpu, 33333333*3);
	//m_maincpu->set_icache_size(16384);
	//m_maincpu->set_dcache_size(16384);
	m_maincpu->set_addrmap(AS_PROGRAM, &ip24_state::ip24_map);
}

void ip22_state::wd33c93_2(device_t *device)
{
	device->set_clock(10000000);
	downcast<wd33c93b_device *>(device)->irq_cb().set(m_hpc3, FUNC(hpc3_device::scsi1_irq));
	downcast<wd33c93b_device *>(device)->drq_cb().set(m_hpc3, FUNC(hpc3_device::scsi1_drq));
}

void ip22_state::indigo2_4415(machine_config &config)
{
	R4400(config, m_maincpu, 50000000*3);
	//m_maincpu->set_icache_size(32768);
	//m_maincpu->set_dcache_size(32768);
	m_maincpu->set_addrmap(AS_PROGRAM, &ip22_state::ip22_map);

	ip24_base(config);

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

	m_hpc3->hd_rd_cb<1>().set(m_scsi_ctrl2, FUNC(wd33c93b_device::indir_r));
	m_hpc3->hd_wr_cb<1>().set(m_scsi_ctrl2, FUNC(wd33c93b_device::indir_w));
	m_hpc3->hd_dma_rd_cb<1>().set(m_scsi_ctrl2, FUNC(wd33c93b_device::dma_r));
	m_hpc3->hd_dma_wr_cb<1>().set(m_scsi_ctrl2, FUNC(wd33c93b_device::dma_w));
	m_hpc3->hd_reset_cb<1>().set(m_scsi_ctrl2, FUNC(wd33c93b_device::reset_w));

	SGI_IOC2_FULL_HOUSE(config, m_ioc2, m_maincpu);
}

#define INDY_BIOS_FLAGS(bios) ROM_GROUPDWORD | ROM_BIOS(bios)

#define INDY_BIOS_R5K \
	ROM_SYSTEM_BIOS( 0, "b10", "Version 5.3 Rev B10 R4X00/R5000 Feb 12, 1996" ) \
	ROMX_LOAD( "ip24prom.070-9101-011.bin", 0x000000, 0x080000, CRC(5e7f7e3a) SHA1(ac663a4db68528e400487e98cbf901f620fb30ce), INDY_BIOS_FLAGS(0) )

#define INDY_BIOS_R4K \
	INDY_BIOS_R5K \
	ROM_SYSTEM_BIOS( 1, "b4", "Version 5.1.2 Rev B4 R4X00 Dec 9, 1993" ) \
	ROMX_LOAD( "ip24prom.070-9101-005.bin", 0x000000, 0x080000, CRC(f5e41008) SHA1(28b769b28218a1fcd0400dceef9a284dcfbdda5b), INDY_BIOS_FLAGS(1) ) \
	ROM_SYSTEM_BIOS( 2, "b6", "Version 5.0 Rev B6 Sep 28, 1994" ) \
	ROMX_LOAD( "ip24prom.070-9101-007.bin", 0x000000, 0x080000, CRC(70d8d1b1) SHA1(ade54cd2ecb7064957f8602894f05685e2f4e8fb), INDY_BIOS_FLAGS(2) )

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
 * 05 <- ea Transmitter: as above + Transmitetr enable
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
	ROM_SYSTEM_BIOS( 0, "e", "Version 5.3 Rev E R4X00 Jan 29, 1996" ) \
	ROMX_LOAD( "ip22prom.070-1367-012.bin", 0x000000, 0x080000, CRC(54460c16) SHA1(330d87b3a02a05fb49c85a569f6f84904587cb35), ROM_GROUPDWORD | ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "b4", "Version 5.1.2 Rev B4 R4X00 Dec 9, 1993" ) \
	ROMX_LOAD( "ip22prom.070-1367-002.bin", 0x000000, 0x080000, CRC(ae5ecd08) SHA1(422568ae95282ee23b2fe123267f9b915a1dc3dc), ROM_GROUPDWORD | ROM_BIOS(1) )
ROM_END

//    YEAR  NAME          PARENT     COMPAT  MACHINE       INPUT CLASS       INIT        COMPANY                 FULLNAME                   FLAGS
COMP( 1993, indy_4610,    0,         0,      indy_4610,    ip24, ip24_state, empty_init, "Silicon Graphics Inc", "Indy (R4600, 100MHz)",    MACHINE_NOT_WORKING )
COMP( 1993, indy_4613,    indy_4610, 0,      indy_4613,    ip24, ip24_state, empty_init, "Silicon Graphics Inc", "Indy (R4600, 133MHz)",    MACHINE_NOT_WORKING )
COMP( 1996, indy_5015,    indy_4610, 0,      indy_5015,    ip24, ip24_state, empty_init, "Silicon Graphics Inc", "Indy (R5000, 150MHz)",    MACHINE_NOT_WORKING )
COMP( 1993, indigo2_4415, 0,         0,      indigo2_4415, ip24, ip22_state, empty_init, "Silicon Graphics Inc", "Indigo2 (R4400, 150MHz)", MACHINE_NOT_WORKING )
