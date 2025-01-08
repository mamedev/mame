// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * National Semiconductor SYS16 Multi-User Development System for the NS16000 Microprocessor Family
 *
 * Sources:
 *  - NS16000 Databook, National Semiconductor Corporation, 1983 (OEE1894, DA-RRD35M94)
 *
 * TODO:
 *  - gpib, printer ports
 *  - sio/disk/tape boards
 *
 * WIP:
 *  - multiple startup diagnostic tests failing
 *  - can exit and run monitor
 *
 *  p/f test                notes
 *   p  0 - timer
 *   p  1 - mmu
 *   p  2 - fpu             passes with firmware hack
 *   p  3 - cpu ram
 *   f  4 - emb ram
 *   f  5 - pio (printer)
 *   f  6 - gpib            requires 8291 talker/listener "loopback" mode
 *   f  7 - sio board
 *   f  8 - tape subsystem
 *   f  9 - disk subsystem
 *
 * Interrupts       (default is level triggered, active low)
 *  0
 *  1 timer         (edge triggered)
 *  2
 *  3 disk          (edge triggered)
 *  4 tape          (edge triggered)
 *  5
 *  6 gpib          (active high)
 *  7
 *  8
 *  9 rx
 * 10 tx            (edge triggered)
 * 11 graf
 * 12 bpio          (edge triggered)
 * 13 apio          (edge triggered, active high)
 * 14 sio
 * 15 softclock
 */

#include "emu.h"

// cpu cluster
#include "cpu/ns32000/ns32000.h"
#include "machine/ns32081.h"
#include "machine/ns32082.h"
#include "machine/ns32202.h"

// other devices
#include "machine/i8255.h"
#include "machine/i8291a.h"
#include "machine/pit8253.h"
#include "machine/scn_pci.h"

// busses and connectors
#include "bus/rs232/rs232.h"

// disk board
#include "cpu/mcs48/mcs48.h"
#include "machine/i8243.h"

#define VERBOSE 0
#include "logmacro.h"

namespace {

class sys16_state : public driver_device
{
public:
	sys16_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "cpu")
		, m_fpu(*this, "fpu")
		, m_mmu(*this, "mmu")
		, m_icu(*this, "icu")
		, m_pci(*this, "pci")
		, m_ppi(*this, "ppi")
		, m_pit(*this, "pit")
		, m_gtl(*this, "gtl")
		, m_console(*this, "console")
		, m_eprom(*this, "eprom")
		, m_hdc_cpu(*this, "hdc_cpu")
		, m_boot(*this, "boot")
	{
	}

	void sys16(machine_config &config);
	void init();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	template <unsigned ST> void cpu_map(address_map &map) ATTR_COLD;
	void hdc_mem(address_map &map) ATTR_COLD;

	required_device<ns32016_device> m_cpu;
	required_device<ns32081_device> m_fpu;
	required_device<ns32082_device> m_mmu;
	required_device<ns32202_device> m_icu;

	required_device<scn2651_device> m_pci;
	required_device<i8255_device> m_ppi;
	required_device<pit8253_device> m_pit;
	required_device<i8291a_device> m_gtl;

	required_device<rs232_port_device> m_console;

	required_region_ptr<u16> m_eprom;
	required_device<i8039_device> m_hdc_cpu;

	memory_view m_boot;

private:
	u8 nmi_r() { return m_nmi; }
	void dcr_w(u8 data);

	u16 opt_r();

	u8 m_dcr; // diagnostic control register
	u8 m_nmi; // nmi error register
};

void sys16_state::init()
{
	/*
	 * HACK: the FPU diagnostic test executes the following code after
	 * triggering an inexact result exception:
	 *
	 *   00e07db7  sfsr r0
	 *   00e07dba  movd 0x0,r1
	 *   00e07dc0  lfsr r1
	 *   00e07dc3  cmpw 0x66,r4
	 *
	 * Either the r0 at e07db7 should be r4, or the r4 at e07dc3 should be r0
	 * to correctly verify the FSR contains the expected trap type.
	 */
	m_eprom[0x7dc4 >> 1] = 0x00a0; // cmpw 0x66,r0
	m_eprom[0x930a >> 1] = 0x5e00; // checksum
}

void sys16_state::machine_start()
{
	save_item(NAME(m_dcr));
}

void sys16_state::machine_reset()
{
	m_dcr = 0;
	m_nmi = 6; // HACK: pass diagnostic

	m_boot.select(0);
}

template <unsigned ST> void sys16_state::cpu_map(address_map &map)
{
	if (ST == 0)
	{
		map(0x00'0000, 0x03'ffff).view(m_boot);
		m_boot[0](0x00'0000, 0x00'9fff).rom().region("eprom", 0);
		m_boot[1](0x00'0000, 0x03'ffff).ram();

		map(0x04'0000, 0x3f'ffff).noprw();

		map(0xa0'0c00, 0xa0'1bff).ram(); // serial board ram?

		map(0xe0'0000, 0xe0'9fff).rom().region("eprom", 0);

		// extended memory board control regs
		// ffe802 embcra
		// ffe602 embcra1
		// ffe402 embcra2
		// ffea00 ieee488 system controller
		map(0xff'ec00, 0xff'ec0f).umask16(0x00ff).m(m_gtl, FUNC(i8291a_device::map));
		map(0xff'ee00, 0xff'ee01).umask16(0x00ff).w(FUNC(sys16_state::dcr_w));

		// fff000 led control
		map(0xff'f200, 0xff'f200).umask16(0x00ff).r(FUNC(sys16_state::nmi_r));
		map(0xff'f400, 0xff'f401).r(FUNC(sys16_state::opt_r));
		map(0xff'f600, 0xff'f607).umask16(0x00ff).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write));
		map(0xff'f800, 0xff'f807).umask16(0x00ff).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write));
		map(0xff'fa00, 0xff'fa07).umask16(0x00ff).rw(m_pci, FUNC(scn2651_device::read), FUNC(scn2651_device::write));
	}

	map(0xff'fe00, 0xff'feff).umask16(0x00ff).m(m_icu, FUNC(ns32202_device::map<BIT(ST, 1)>));
}

void sys16_state::hdc_mem(address_map &map)
{
	map(0x0000, 0x0fff).rom().region("hdc_rom", 0);
}

static INPUT_PORTS_START(sys16)
INPUT_PORTS_END

void sys16_state::sys16(machine_config &config)
{
	NS32016(config, m_cpu, 6'000'000); // NS32016D-10
	m_cpu->set_addrmap(0, &sys16_state::cpu_map<0>);
	m_cpu->set_addrmap(4, &sys16_state::cpu_map<4>);
	m_cpu->set_addrmap(6, &sys16_state::cpu_map<6>);

	NS32081(config, m_fpu, 6'000'000); // NS32081D-10
	m_cpu->set_fpu(m_fpu);

	NS32082(config, m_mmu, 6'000'000); // NS32082D-10
	m_cpu->set_mmu(m_mmu);

	NS32202(config, m_icu, 18.432_MHz_XTAL / 10); // NS32202-10
	m_icu->out_int().set_inputline(m_cpu, INPUT_LINE_IRQ0).invert();

	I8255(config, m_ppi); // QP8255A

	PIT8253(config, m_pit); // P8253-5
	m_pit->set_clk<0>(1.8432_MHz_XTAL);
	m_pit->set_clk<1>(1.8432_MHz_XTAL);
	m_pit->set_clk<2>(1.8432_MHz_XTAL);
	m_pit->out_handler<0>().set(m_pci, FUNC(scn2651_device::rxc_w));
	m_pit->out_handler<1>().set(m_pci, FUNC(scn2651_device::txc_w));
	m_pit->out_handler<2>().set(m_icu, FUNC(ns32202_device::ir_w<1>));

	SCN2651(config, m_pci, 1.8432_MHz_XTAL); // INS2651N

	RS232_PORT(config, m_console, default_rs232_devices, "terminal");
	m_console->rxd_handler().set(m_pci, FUNC(scn2651_device::rxd_w));
	m_pci->txd_handler().set(m_console, FUNC(rs232_port_device::write_txd));

	I8291A(config, m_gtl, 6'000'000);
	m_gtl->int_write().set(m_icu, FUNC(ns32202_device::ir_w<6>));

	// TODO: serial controller board
	// MK3880BN-6 Z80-CPU
	// INS2651N (x8)
	// 5.0688MHz
	// MM2114N-15 1024x4 Static RAM (x8) -> 4096 bytes

	// TODO: hard disk controller board
	// INS8039J-11
	// 11.0MHz
	// INS8243N input/output expander
	// 25.0MHz
	// AM2901BPC (x2)
	// IDM2911ANC (x2) microprogram sequencer
	// AM93L422PCB (x8) 256 x 4-Bit Low Power TTL Bipolar IMOX RAM
	// AM25LS2569PC (x2) binary counter
	// N9403N (x2) 64-bit fifo buffer memory

	I8039(config, m_hdc_cpu, 11_MHz_XTAL);
	m_hdc_cpu->set_addrmap(AS_PROGRAM, &sys16_state::hdc_mem);
}

enum nmi_mask : u8
{
	NMI_POWER  = 0x01, // power fail
	NMI_ECC    = 0x02, // extended memory board error
	NMI_PARITY = 0x04, // ram parity error
};

enum dcr_bits : u8
{
	DCR_RAM    = 0, // ram switch
	DCR_RESET  = 1, // reset flop
	DCR_ION    = 2, // interrupt enable
	DCR_POWER  = 3, // power fail enable
	DCR_PARITY = 4, // parity error enable
	DCR_NMI    = 5, // nmi error enable
	DCR_PERROR = 6, // force parity errors
	DCR_XRESET = 7, // external reset low
};

void sys16_state::dcr_w(u8 data)
{
	LOG("dcr_w 0x%02x (%s)\n", data, machine().describe_context());

	if (BIT(data, DCR_RAM))
		m_boot.select(1);

	m_dcr = data;
}

enum opt_mask : u16
{
	OPT_EMB   = 0x0001, // extended memory board (0=256K, 1=1.2M)
	OPT_ECHO  = 0x0002, // echo serial to monitor
	OPT_DIAG  = 0x0004, // run diagnostics
	OPT_FPU   = 0x0100, // FPU installed
	OPT_MOVSU = 0x0200, // MOVSU instruction works
	OPT_WORK  = 0x0400, // SYS16 or workstation
	OPT_BOOT  = 0x0800, // auto boot unix (0=boot /vmunix, 1=monitor)
	OPT_DUMP  = 0x1000, // breakpoint (0=bpt, 1=dump)
	OPT_GPIB  = 0xe000, // GPIB address
};

// TODO: change to dip switches?
u16 sys16_state::opt_r()
{
	return OPT_WORK | OPT_MOVSU | OPT_FPU | OPT_DIAG | OPT_ECHO;
}

ROM_START(sys16)
	ROM_REGION16_LE(0xa000, "eprom", 0)

	ROM_SYSTEM_BIOS(0, "v3.4", "National 16032 ROM Monitor (Rev. 3.4) (kernel) Fri Jul 29 12:51:11 PDT 1983")
	ROMX_LOAD("7000__001d.u163", 0x0000, 0x1000, CRC(62190bba) SHA1(43dfa1f05ffe18540fc9642b00b1e9211d73e947), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("7000__006d.u184", 0x0001, 0x1000, CRC(a6ecae9e) SHA1(e98d038edac8083b487bebe176daf788f4713a88), ROM_BIOS(0) | ROM_SKIP(1))

	ROMX_LOAD("7000__002d.u164", 0x2000, 0x1000, CRC(5df7b3d1) SHA1(71f8961f2cb6d592034f3929d6abcda9d6a35212), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("7000__007d.u185", 0x2001, 0x1000, CRC(72117cb2) SHA1(cbac772399375a99075b85f18619fb671a99f3bd), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("7000__003d.u165", 0x4000, 0x1000, CRC(27ae56d2) SHA1(53b097eb8c56d49bc85d2954c2fa799ccd503484), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("7000__008d.u186", 0x4001, 0x1000, CRC(3909d280) SHA1(d0db32a804dfde3cd2fcd0b136726868d7592498), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("7000__004d.u166", 0x6000, 0x1000, CRC(34553048) SHA1(03c2bd7de8192412e4efea3b2d7ba14db18d65e8), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("7000__009d.u187", 0x6001, 0x1000, CRC(910119fd) SHA1(2c103cf5175585d9748877f71bcfaf0ca591e6f7), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("7000__005d.u167", 0x8000, 0x1000, CRC(f5243c77) SHA1(243780f652867bb70634b2df8c7a02d634675029), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("7000__010d.u188", 0x8001, 0x1000, CRC(508bbaf1) SHA1(ed8a7f5f65f1fb19c34cc7f5d2e2b65207c9a1c8), ROM_BIOS(0) | ROM_SKIP(1))

	ROM_REGION(0x1000, "hdc_rom", 0)
	ROM_LOAD("hdc_rom.u157", 0x0000, 0x800, CRC(cc17d74a) SHA1(b6604043527ad24c3472d84c5747ef378433629f))
	ROM_LOAD("hdc_rom.u158", 0x0800, 0x800, CRC(512bad81) SHA1(6bc262028a397bdf33951ebd775e42189b03b7bd))
ROM_END

} // anonymous namespace

/*   YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT  COMPANY                   FULLNAME  FLAGS */
COMP(1983, sys16, 0,      0,      sys16,   sys16, sys16_state, init, "National Semiconductor", "SYS16",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
