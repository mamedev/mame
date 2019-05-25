// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*********************************************************************\
*
*   SGI IP22 Indigo2/Indy workstation
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
*  1f000000 - 1f3fffff      GIO - GFX
*  1f400000 - 1f5fffff      GIO - EXP0
*  1f600000 - 1f9fffff      GIO - EXP1 - Unused
*  1fa00000 - 1fa1ffff      Memory Controller
*  1fb00000 - 1fb1a7ff      HPC3 CHIP1
*  1fb80000 - 1fb9a7ff      HPC3 CHIP0
*  1fc00000 - 1fffffff      BIOS
*  20000000 - 2fffffff      High System Memory
*  30000000 - 7fffffff      Reserved
*  80000000 - ffffffff      EISA Memory
*
*  IP22/24 has 2 pieces of PC-compatible hardware: the 8042 PS/2 keyboard/mouse
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

#include "bus/gio/gio.h"

#include "cpu/mips/r4000.h"

#include "machine/hpc3.h"
#include "machine/nscsi_bus.h"
#include "machine/nscsi_cd.h"
#include "machine/nscsi_hd.h"
#include "machine/sgi.h"
#include "machine/vino.h"

#include "sound/cdda.h"

#include "emupal.h"
#include "screen.h"

class ip22_state : public driver_device
{
public:
	ip22_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_mainram(*this, "mainram")
		, m_mem_ctrl(*this, "memctrl")
		, m_scsi_ctrl(*this, "scsibus:0:wd33c93")
		, m_hpc3(*this, "hpc3")
		, m_vino(*this, "vino")
		, m_gio(*this, "gio")
		, m_gio_gfx(*this, "gio_gfx")
		, m_gio_exp0(*this, "gio_exp0")
		, m_gio_exp1(*this, "gio_exp1")
	{
	}

	void ip22_base(machine_config &config);
	void ip225015(machine_config &config);
	void ip224613(machine_config &config);

protected:
	virtual void machine_reset() override;

	DECLARE_WRITE64_MEMBER(write_ram);
	DECLARE_READ32_MEMBER(bus_error);

	void ip22_map(address_map &map);
	void ip22_base_map(address_map &map);

	void wd33c93(device_t *device);

	static void scsi_devices(device_slot_interface &device);

	required_device<r4000_base_device> m_maincpu;
	required_shared_ptr<uint64_t> m_mainram;
	required_device<sgi_mc_device> m_mem_ctrl;
	required_device<wd33c93b_device> m_scsi_ctrl;
	required_device<hpc3_base_device> m_hpc3;
	optional_device<vino_device> m_vino;
	optional_device<gio_device> m_gio;
	optional_device<gio_slot_device> m_gio_gfx;
	optional_device<gio_slot_device> m_gio_exp0;
	optional_device<gio_slot_device> m_gio_exp1;
};

class ip24_state : public ip22_state
{
public:
	ip24_state(const machine_config &mconfig, device_type type, const char *tag)
		: ip22_state(mconfig, type, tag)
		, m_scsi_ctrl2(*this, "scsibus2:0:wd33c93")
	{
	}

	void ip244415(machine_config &config);

private:
	DECLARE_READ32_MEMBER(eisa_io_r);

	void wd33c93_2(device_t *device);

	void ip24_map(address_map &map);

	required_device<wd33c93b_device> m_scsi_ctrl2;
};

READ32_MEMBER(ip22_state::bus_error)
{
	m_maincpu->bus_error();
	return 0;
}

READ32_MEMBER(ip24_state::eisa_io_r)
{
	return 0xffffffff;
}

// a bit hackish, but makes the memory detection work properly and allows a big cleanup of the mapping
WRITE64_MEMBER(ip22_state::write_ram)
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

void ip22_state::ip22_base_map(address_map &map)
{
	map(0x00000000, 0x0007ffff).bankrw("bank1");    /* mirror of first 512k of main RAM */
	map(0x08000000, 0x0fffffff).share("mainram").ram().w(FUNC(ip22_state::write_ram));     /* 128 MB of main RAM */
	map(0x1f000000, 0x1f9fffff).rw(m_gio, FUNC(gio_device::read), FUNC(gio_device::write));
	map(0x1fa00000, 0x1fa1ffff).rw(m_mem_ctrl, FUNC(sgi_mc_device::read), FUNC(sgi_mc_device::write));
	map(0x1fb00000, 0x1fb7ffff).r(FUNC(ip22_state::bus_error));
	map(0x1fb80000, 0x1fbfffff).m(m_hpc3, FUNC(hpc3_base_device::map));
	map(0x1fc00000, 0x1fc7ffff).rom().region("user1", 0);
	map(0x20000000, 0x27ffffff).share("mainram").ram().w(FUNC(ip22_state::write_ram));
}

void ip22_state::ip22_map(address_map &map)
{
	ip22_base_map(map);
	map(0x00080000, 0x0009ffff).rw(m_vino, FUNC(vino_device::read), FUNC(vino_device::write));
}

void ip24_state::ip24_map(address_map &map)
{
	ip22_base_map(map);
	map(0x00080000, 0x0009ffff).r(FUNC(ip24_state::eisa_io_r));
}

void ip22_state::machine_reset()
{
	// set up low RAM mirror
	membank("bank1")->set_base(m_mainram);

	//m_maincpu->mips3drc_set_options(MIPS3DRC_COMPATIBLE_OPTIONS | MIPS3DRC_CHECK_OVERFLOWS);
}

static INPUT_PORTS_START( ip225015 )
	PORT_START("IN0")   // unused IN0
	PORT_START("DSW0")  // unused IN1
	PORT_START("DSW1")  // unused IN2
	PORT_START("DSW2")  // unused IN3
	PORT_INCLUDE( at_keyboard )     /* IN4 - IN11 */
INPUT_PORTS_END

//static void cdrom_config(device_t *device)
//{
//  cdda_device *cdda = device->subdevice<cdda_device>("cdda");
//  cdda->add_route(0, ":hpc3:lspeaker", 1.0);
//  cdda->add_route(1, ":hpc3:rspeaker", 1.0);
//}

void ip22_state::wd33c93(device_t *device)
{
	device->set_clock(10000000);
	downcast<wd33c93b_device *>(device)->irq_cb().set(m_hpc3, FUNC(hpc3_base_device::scsi0_irq));
	downcast<wd33c93b_device *>(device)->drq_cb().set(m_hpc3, FUNC(hpc3_base_device::scsi0_drq));
}

void ip22_state::scsi_devices(device_slot_interface &device)
{
	device.option_add("cdrom", NSCSI_CDROM_SGI);
	device.option_add("harddisk", NSCSI_HARDDISK);
	//device.set_option_machine_config("cdrom", cdrom_config);
}

void ip22_state::ip22_base(machine_config &config)
{
	SGI_MC(config, m_mem_ctrl, m_maincpu, ":hpc3:eeprom", m_hpc3);

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

	// GIO
	GIO(config, m_gio, m_maincpu, m_hpc3);
	GIO_SLOT(config, m_gio_gfx, m_gio, gio_cards, "xl24");
	GIO_SLOT(config, m_gio_exp0, m_gio, gio_cards, nullptr);
	GIO_SLOT(config, m_gio_exp1, m_gio, gio_cards, nullptr);
}

void ip22_state::ip225015(machine_config &config)
{
	ip22_base(config);

	R4000(config, m_maincpu, 50000000*3);
	//m_maincpu->set_icache_size(32768);
	//m_maincpu->set_dcache_size(32768);
	m_maincpu->set_addrmap(AS_PROGRAM, &ip22_state::ip22_map);

	SGI_HPC3_GUINNESS(config, m_hpc3, m_maincpu, m_scsi_ctrl);

	VINO(config, m_vino);
}

void ip22_state::ip224613(machine_config &config)
{
	ip22_base(config);

	R4600(config, m_maincpu, 33333333*4);
	//m_maincpu->set_icache_size(32768);
	//m_maincpu->set_dcache_size(32768);
	m_maincpu->set_addrmap(AS_PROGRAM, &ip22_state::ip22_map);

	SGI_HPC3_GUINNESS(config, m_hpc3, m_maincpu, m_scsi_ctrl);

	VINO(config, m_vino);
}

void ip24_state::wd33c93_2(device_t *device)
{
	device->set_clock(10000000);
	downcast<wd33c93b_device *>(device)->irq_cb().set(m_hpc3, FUNC(hpc3_base_device::scsi1_irq));
	downcast<wd33c93b_device *>(device)->drq_cb().set(m_hpc3, FUNC(hpc3_base_device::scsi1_drq));
}

void ip24_state::ip244415(machine_config &config)
{
	ip22_base(config);

	R4400(config, m_maincpu, 50000000*3);
	//m_maincpu->set_icache_size(32768);
	//m_maincpu->set_dcache_size(32768);
	m_maincpu->set_addrmap(AS_PROGRAM, &ip24_state::ip24_map);

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

	SGI_HPC3_FULL_HOUSE(config, m_hpc3, m_maincpu, m_scsi_ctrl, m_scsi_ctrl2);
}

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
ROM_START( ip225015 )
	ROM_REGION64_BE( 0x80000, "user1", 0 )
	ROMX_LOAD( "ip225015.bin", 0x000000, 0x080000, CRC(aee5502e) SHA1(9243fef0a3508790651e0d6d2705c887629b1280), ROM_GROUPDWORD | ROM_REVERSE )
ROM_END

ROM_START( ip224613 )
	ROM_REGION64_BE( 0x80000, "user1", 0 )
	ROMX_LOAD( "ip224613.bin", 0x000000, 0x080000, CRC(f1868b5b) SHA1(0dcbbd776e671785b9b65f3c6dbd609794a40157), ROM_GROUPDWORD | ROM_REVERSE )
ROM_END

ROM_START( ip244415 )
	ROM_REGION64_BE( 0x80000, "user1", 0 )
	ROMX_LOAD( "ip244415.bin", 0x000000, 0x080000, CRC(2f37825a) SHA1(0d48c573b53a307478820b85aacb57b868297ca3), ROM_GROUPDWORD | ROM_REVERSE )
ROM_END

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS       INIT        COMPANY                 FULLNAME                   FLAGS
COMP( 1993, ip225015, 0,      0,      ip225015, ip225015, ip22_state, empty_init, "Silicon Graphics Inc", "Indy (R5000, 150MHz)",    MACHINE_NOT_WORKING )
COMP( 1993, ip224613, 0,      0,      ip224613, ip225015, ip22_state, empty_init, "Silicon Graphics Inc", "Indy (R4600, 133MHz)",    MACHINE_NOT_WORKING )
COMP( 1994, ip244415, 0,      0,      ip244415, ip225015, ip24_state, empty_init, "Silicon Graphics Inc", "Indigo2 (R4400, 150MHz)", MACHINE_NOT_WORKING )
