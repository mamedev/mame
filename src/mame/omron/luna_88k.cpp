// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Omron Luna 88K and 88K² systems.
 *
 * Sources:
 *  -
 *
 * TODO:
 *  - everything
 */

/*
 * Omron 3W4SX-9100/DT9581 UNIX WORKSTATION
 *
 * PWB7187 (MAIN)
 * --
 * MB89352A                 SCSI
 * D7201AC                  serial (keyboard/mouse + port A)
 * HD647180X0FS6            I/O processor (serial port -00 and -01, printer)
 * TF116NF
 * TF175NF
 * HM62256ALFP-10T * 5      32,768x8 static RAM
 * MB89352FP                SCSI
 * MDS74ACE36X646
 * TF177HF
 * MC88915FN70              clock driver
 * DS1000M-25               delay line
 * HN27C1024HCC-85 * 2      PROM
 * DS1397 RTC/NVRAM         MC146818 RTC + 4Kx8 NVRAM
 * M5M82C55A * 2            PPI
 * OSC1 12.288MHz           IOP clock
 * OSC2 33.333MHz           CPU clock
 * OSC3 50MHz
 * OSC4 200Hz
 * OSC5 8MHz                UPD7201 clock?
 * KSS EXO3 3C 19.660M      PC-98 bus?
 * Ports: keyboard, SCSI, RS232C-A, -01, -00, printer
 *
 * PWB7188 (CPU+MMU)
 * --
 * MC88100RC33
 * MC88200RC33 * 2
 *
 * PWB87132 (W-LAN)
 * --
 * AM7990JC/80
 * DS1220Y-200 NVRAM
 * AM7992BDC
 * OSC1 40MHz
 * Another AM7990 and AM7992B are unpopulated
 *
 * PWB7131 BM08HR (video)
 * --
 * Bt458LPJ124 RAMDAC
 * HD6445CP4
 * Bt438KPJ
 * OSC1 108.9920MHz
 * HM53462ZP-10 * 8*8
 */

#include "emu.h"

#include "cpu/m88000/m88000.h"
//#include "machine/mc88200.h"
#include "cpu/z180/hd647180x.h"

// memory
#include "machine/ram.h"
#include "machine/nvram.h"

// various hardware
#include "machine/clock.h"
#include "machine/i8255.h"
#include "machine/mc146818.h"
#include "machine/z80sio.h"

// busses and connectors
#include "bus/rs232/rs232.h"

#include "debugger.h"

#define VERBOSE 0
#include "logmacro.h"

namespace {

class luna_88k_state : public driver_device
{
public:
	luna_88k_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "cpu")
		//, m_cmmu(*this, "cmmu%u", 0U)
		, m_ram(*this, "ram")
		, m_iop(*this, "iop")
		, m_rtc(*this, "rtc")
		, m_sio(*this, "sio")
		, m_ppi(*this, "ppi%u", 0U)
		, m_serial(*this, "serial%u", 0U)
		, m_boot(*this, "boot")
	{
	}

	void luna88k2(machine_config &config);
	void init();

protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	// address maps
	void cpu_map(address_map &map);

	// machine config
	void common(machine_config &config);

private:
	// devices
	required_device<mc88100_device> m_cpu;
	//required_device_array<mc88200_device, 2> m_cmmu;
	required_device<ram_device> m_ram;
	required_device<hd647180x_device> m_iop;
	required_device<mc146818_device> m_rtc;
	required_device<upd7201_device> m_sio;
	required_device_array<i8255_device, 2> m_ppi;
	required_device_array<rs232_port_device, 2> m_serial;

	memory_view m_boot;
};

void luna_88k_state::init()
{
}

void luna_88k_state::machine_start()
{
}

void luna_88k_state::machine_reset()
{
	m_boot.select(0);

	// TODO: disabled until firmware is dumped
	m_iop->suspend(SUSPEND_REASON_DISABLE, false);
}

void luna_88k_state::cpu_map(address_map &map)
{
	map(0x00000000, 0x00ffffff).view(m_boot);
	m_boot[0](0x00000000, 0x0003ffff).rom().region("eprom", 0);
	m_boot[1](0x00000000, 0x00ffffff).ram();

	// obio 1: 0x41000000 0x1f000000
	// obio 2: 0x61000000 0x1f000000
	// obio 3: 0x80000000 0x80000000

	map(0x41000000, 0x4103ffff).rom().region("eprom", 0);
	map(0x41000000, 0x4103ffff).lw32([this](offs_t offset, u32 data) { m_boot.select(1); }, "boot");

	// map(0x43000000, 0x430003ff); // fuse rom
	// 0x45000000 timekeeper? 0x1fdc? data reg 0x45000001
	// 0x47000000 nvram 88k2 0x40 nvram page 0x47000020
	map(0x49000000, 0x4900000f).rw(m_ppi[0], FUNC(i8255_device::read), FUNC(i8255_device::write)).umask32(0xff000000); // port a(dipsw1), port b(dipsw2)
	map(0x4d000000, 0x4d00000f).rw(m_ppi[1], FUNC(i8255_device::read), FUNC(i8255_device::write)).umask32(0xff000000);
	// 0x4d000000 lm16x212 lcd module?
	// 0x51000000 upd7201a irq 5?
	map(0x51000000, 0x5100000f).rw(m_sio, FUNC(upd7201_device::ba_cd_r), FUNC(upd7201_device::ba_cd_w)).umask32(0xff000000);
	// 0x61000000 tas register
	// 0x63000000 system clock cpu0-3, also power off switch?
	// 0x65000000 int status cpu0-3
	// 0x69000000 soft int cpu 0-3
	// 0x6b000000 soft int flag cpu0-3
	// 0x6d000000 reset cpu 0-3, all
	// 0x71000000 hd647180xp/3 port ram (0x20000)
	// 0x81000000 ext board A
	// 0x83000000 ext board B
	// 0x90000000 pc-98 ext board
	// 0x91000000 pc-9801 irq 4
	// 0xa1000000 mask rom 0x400000
	// 0xb1000000 bitmap
	//
	// 0xc1100000 bt458
	// 0xd0000000 board check register?
	// 0xd1000000 ctrc-ii
	// 0xd1800000 bitmap board identify rom
	// 0xe1000000 mb89352 irq 3
	// 0xe1000040 mb89352 #2 irq 3
	// 0xf1000000 am7990 irq 4?

	// clock irq 0
	// software irq 1
}

void luna_88k_state::luna88k2(machine_config &config)
{
	MC88100(config, m_cpu, 33.333_MHz_XTAL);
	m_cpu->set_addrmap(AS_PROGRAM, &luna_88k_state::cpu_map);

#if 0
	MC88200(config, m_cmmu[0], 33.333_MHz_XTAL, 0x06); // cpu0 cmmu d0
	m_cmmu[0]->set_mbus(m_cpu, AS_PROGRAM);
	m_cpu->set_cmmu_d(m_cmmu[0]);
	MC88200(config, m_cmmu[1], 33.333_MHz_XTAL, 0x07); // cpu0 cmmu i0
	m_cmmu[1]->set_mbus(m_cpu, AS_PROGRAM);
	m_cpu->set_cmmu_i(m_cmmu[1]);
#endif

	// 6 SIMMs for RAM arranged as three groups of 2?
	RAM(config, m_ram);
	m_ram->set_default_size("16M");

	MC146818(config, m_rtc, 32'768); // DS1397

	HD647180X(config, m_iop, 12'288'000);

	UPD7201(config, m_sio, 8'000'000); // ?

	// RS-232C-A
	RS232_PORT(config, m_serial[0], default_rs232_devices, "terminal");
	m_sio->out_txda_callback().set(m_serial[0], FUNC(rs232_port_device::write_txd));
	m_sio->out_dtra_callback().set(m_serial[0], FUNC(rs232_port_device::write_dtr));
	m_sio->out_rtsa_callback().set(m_serial[0], FUNC(rs232_port_device::write_rts));
	m_serial[0]->rxd_handler().set(m_sio, FUNC(upd7201_device::rxa_w));
	m_serial[0]->cts_handler().set(m_sio, FUNC(upd7201_device::ctsa_w));

	// keyboard/mouse
	RS232_PORT(config, m_serial[1], default_rs232_devices, nullptr);
	m_sio->out_txdb_callback().set(m_serial[1], FUNC(rs232_port_device::write_txd));
	m_sio->out_dtrb_callback().set(m_serial[1], FUNC(rs232_port_device::write_dtr));
	m_sio->out_rtsb_callback().set(m_serial[1], FUNC(rs232_port_device::write_rts));
	m_serial[1]->rxd_handler().set(m_sio, FUNC(upd7201_device::rxb_w));
	m_serial[1]->cts_handler().set(m_sio, FUNC(upd7201_device::ctsb_w));

	clock_device &sio_clk(CLOCK(config, "sio_clk", 12'288'000 / 80)); // clock output from iop?
	sio_clk.signal_handler().append(m_sio, FUNC(upd7201_device::rxca_w));
	sio_clk.signal_handler().append(m_sio, FUNC(upd7201_device::txca_w));
	sio_clk.signal_handler().append(m_sio, FUNC(upd7201_device::rxcb_w));
	sio_clk.signal_handler().append(m_sio, FUNC(upd7201_device::txcb_w));

	I8255A(config, m_ppi[0], 8'000'000); // M5M82C55AFP-2
	I8255A(config, m_ppi[1], 8'000'000); // M5M82C55AFP-2
}

ROM_START(luna88k2)
	ROM_REGION32_BE(0x40000, "eprom", 0)
	ROM_LOAD32_WORD_SWAP("7187__high__1.37.bin", 0x00000, 0x20000, CRC(a7515231) SHA1(86b3e42a8df6fa33cf68f372f4053b240c8cc4e2)) // HN27C1024HCC-85
	ROM_LOAD32_WORD_SWAP("7187__low__1.37.bin",  0x00002, 0x20000, CRC(8e65ea4a) SHA1(288300c71c0e92f114cb84fa293a4839d2e181a6)) // HN27C1024HCC-85

	ROM_REGION(0x4000, "iop", 0)
	ROM_LOAD("hd647180x.ic13", 0x0000, 0x4000, NO_DUMP) // HD647180X0FS6
ROM_END

} // anonymous namespace

/*   YEAR   NAME      PARENT  COMPAT  MACHINE   INPUT  CLASS           INIT  COMPANY  FULLNAME     FLAGS */
COMP(1992?, luna88k2, 0,      0,      luna88k2, 0,     luna_88k_state, init, "Omron", "Luna 88K²", MACHINE_IS_SKELETON)
