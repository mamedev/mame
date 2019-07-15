// license:BSD-3-Clause
// copyright-holders:F. Ulivi
//
// ***************************************
// Driver for Intel Intellec MDS series-II
// ***************************************
//
// Documentation used for this driver:
// [1]  Intel, manual 9800554-04 rev. D - Intellec series II MDS - Schematic drawings
// [2]  Intel, manual 9800556-02 rev. B - Intellec series II MDS - Hardware reference manual
// [3]  Intel, manual 9800555-02 rev. B - Intellec series II MDS - Hardware interface manual
// [4]  Intel, manual 9800605-02 rev. B - Intellec series II MDS - Boot/monitor listing
//
// All these manuals are available on http://www.bitsavers.org
//
// An Intellec MDS series-II is composed of the following boards:
//
// **********
// Integrated Processor Card (IPC) or Integrated Processor Board (IPB)
//
// This is the board where the OS (ISIS-II) and the application software run.
// This driver emulates an IPC.
// IPC is composed as follows:
// A83 8085A-2  CPU @ 4 MHz
//              64k of DRAM
// A82 2732     EPROM with monitor & boot firmware (assembly source of this ROM is in [4])
// A66 8259A    System PIC
// A89 8259A    Local PIC
// A86 8253     PIT
// A91 8251A    Serial channel #0
// A90 8251A    Serial channel #1
//
// **********
// I/O Controller (IOC)
//
// This board acts as a controller for all I/O of the system.
// It is structured as if it were 2 boards in one:
// One part (around 8080) controls Keyboard, CRT & floppy, the other part (around PIO 8041A) controls all parallel I/Os
// (Line printer, Paper tape puncher, Paper tape reader, I/O to PROM programmer).
// Both parts are interfaced to IPC through a bidirectional 8-bit bus.
// IOC is composed of these parts:
// A69 8080A-2  CPU @ 2.448 MHz
//              8k of DRAM
// A50 2716
// A51 2716
// A52 2716
// A53 2716     EPROMs with IOC firmware
// A58 8257     DMA controller
// A35 8253     PIT
// A1  8271     Floppy controller
// A20 8275     CRT controller
// A19 2708     Character generator ROM
// LS1          3.3 kHz beeper
// A72 8041A    CPU @ 6 MHz (PIO: parallel I/O)
//
// **********
// Keyboard controller
//
// Keyboard is controlled by a 8741 CPU that scans the key matrix and sends key codes to IOC through a 8-bit bus.
//
// A3 8741      CPU @ 3.58 MHz
//
// Huge thanks to Dave Mabry for dumping IOC firmware, KB firmware and character generator. This driver would not
// exist without his dumps.
// (https://web.archive.org/web/20080509062332/http://www.s100-manuals.com/intel/IOC_iMDX_511_Upgrade.zip)
//
// Basic usage / test info:
// To use the system set DIP switches to:
// Floppy present
// IOC mode Diagnostic
// Keyboard present
// and reset the system. The built-in diagnostic mode should start.
//
// Another test is loading ISIS-II (the Intel OS for this system). Floppy image
// 9500007-07_ISIS-II_OPERATING_SYSTEM_DISKETTE_Ver_4.3.IMD
// To load it, the "IOC mode" should be set to "On line", the floppy image
// should be mounted and the system reset. After a few seconds the ISIS-II
// prompt should appear. A command that could be tried is "DIR" that lists
// the content of floppy disk.

#include "emu.h"
#include "machine/imds2ioc.h"

#include "cpu/i8085/i8085.h"
#include "machine/74259.h"
#include "machine/i8251.h"
#include "machine/pit8253.h"
#include "machine/pic8259.h"
#include "bus/rs232/rs232.h"

// CPU oscillator of IPC board: 8 MHz
#define IPC_XTAL_Y2     8_MHz_XTAL

// Y1 oscillator of IPC board: 19.6608 MHz
#define IPC_XTAL_Y1     19.6608_MHz_XTAL

class imds2_state : public driver_device
{
public:
	imds2_state(const machine_config &mconfig, device_type type, const char *tag);

	void imds2(machine_config &config);

private:
	DECLARE_READ8_MEMBER(ipc_mem_read);
	DECLARE_WRITE8_MEMBER(ipc_mem_write);
	DECLARE_WRITE8_MEMBER(ipc_control_w);
	DECLARE_WRITE_LINE_MEMBER(ipc_intr_w);
	DECLARE_READ8_MEMBER(ipcsyspic_r);
	DECLARE_READ8_MEMBER(ipclocpic_r);
	DECLARE_WRITE8_MEMBER(ipcsyspic_w);
	DECLARE_WRITE8_MEMBER(ipclocpic_w);

	virtual void driver_start() override;

	void ipc_io_map(address_map &map);
	void ipc_mem_map(address_map &map);

	required_device<i8085a_cpu_device> m_ipccpu;
	required_device<pic8259_device> m_ipcsyspic;
	required_device<pic8259_device> m_ipclocpic;
	required_device<pit8253_device> m_ipctimer;
	required_device_array<i8251_device, 2> m_ipcusart;
	required_device<ls259_device> m_ipcctrl;
	required_device_array<rs232_port_device, 2> m_serial;
	required_device<imds2ioc_device> m_ioc;

	std::vector<uint8_t> m_ipc_ram;

	bool in_ipc_rom(offs_t offset) const;

	// IPC ROM content
	required_region_ptr<uint8_t> m_ipc_rom;
};

void imds2_state::ipc_mem_map(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(imds2_state::ipc_mem_read), FUNC(imds2_state::ipc_mem_write));
}

void imds2_state::ipc_io_map(address_map &map)
{
	map.unmap_value_low();
	map(0xc0, 0xc1).rw(m_ioc, FUNC(imds2ioc_device::dbb_master_r), FUNC(imds2ioc_device::dbb_master_w));
	map(0xf0, 0xf3).rw(m_ipctimer, FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0xf4, 0xf5).rw(m_ipcusart[0], FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0xf6, 0xf7).rw(m_ipcusart[1], FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0xf8, 0xf9).rw(m_ioc, FUNC(imds2ioc_device::pio_master_r), FUNC(imds2ioc_device::pio_master_w));
	map(0xfa, 0xfb).rw(FUNC(imds2_state::ipclocpic_r), FUNC(imds2_state::ipclocpic_w));
	map(0xfc, 0xfd).rw(FUNC(imds2_state::ipcsyspic_r), FUNC(imds2_state::ipcsyspic_w));
	map(0xff, 0xff).w(FUNC(imds2_state::ipc_control_w));
}

imds2_state::imds2_state(const machine_config &mconfig, device_type type, const char *tag) :
	driver_device(mconfig, type, tag),
	m_ipccpu(*this, "ipccpu"),
	m_ipcsyspic(*this, "ipcsyspic"),
	m_ipclocpic(*this, "ipclocpic"),
	m_ipctimer(*this, "ipctimer"),
	m_ipcusart(*this, "ipcusart%u", 0U),
	m_ipcctrl(*this, "ipcctrl"),
	m_serial(*this, "serial%u", 0U),
	m_ioc(*this, "ioc"),
	m_ipc_rom(*this, "ipcrom")
{
}

READ8_MEMBER(imds2_state::ipc_mem_read)
{
	if (in_ipc_rom(offset)) {
		return m_ipc_rom[ (offset & 0x07ff) | ((offset & 0x1000) >> 1) ];
	} else {
		return m_ipc_ram[ offset ];
	}
}

WRITE8_MEMBER(imds2_state::ipc_mem_write)
{
	if (!in_ipc_rom(offset)) {
		m_ipc_ram[ offset ] = data;
	}
}

WRITE8_MEMBER(imds2_state::ipc_control_w)
{
	// See A84, pg 28 of [1]
	// b3 is ~(bit to be written)
	// b2-b0 is ~(no. of bit to be written)
	m_ipcctrl->write_bit(~data & 7, BIT(~data, 3));
}

WRITE_LINE_MEMBER(imds2_state::ipc_intr_w)
{
	m_ipccpu->set_input_line(I8085_INTR_LINE, (state != 0) && m_ipcctrl->q2_r());
}

READ8_MEMBER(imds2_state::ipcsyspic_r)
{
	return m_ipcsyspic->read(!BIT(offset, 0));
}

READ8_MEMBER(imds2_state::ipclocpic_r)
{
	return m_ipclocpic->read(!BIT(offset, 0));
}

WRITE8_MEMBER(imds2_state::ipcsyspic_w)
{
	m_ipcsyspic->write(!BIT(offset, 0), data);
}

WRITE8_MEMBER(imds2_state::ipclocpic_w)
{
	m_ipclocpic->write(!BIT(offset, 0), data);
}

void imds2_state::driver_start()
{
	// Allocate 64k for IPC RAM
	m_ipc_ram.resize(0x10000);
}

bool imds2_state::in_ipc_rom(offs_t offset) const
{
	offs_t masked_offset = offset & 0xf800;
	bool start_up = m_ipcctrl->q5_r();
	bool sel_boot = m_ipcctrl->q3_r();

	// Region 0000-07ff is in ROM when START_UP/ == 0 && SEL_BOOT/ == 0
	if (masked_offset == 0x0000 && !start_up && !sel_boot) {
		return true;
	}

	// Region e800-efff is in ROM when SEL_BOOT/ == 0
	if (masked_offset == 0xe800 && !sel_boot) {
		return true;
	}

	// Region f800-ffff is always in ROM
	if (masked_offset == 0xf800) {
		return true;
	}

	return false;
}

static INPUT_PORTS_START(imds2)
INPUT_PORTS_END

void imds2_state::imds2(machine_config &config)
{
	I8085A(config, m_ipccpu, IPC_XTAL_Y2);  // CLK OUT = 4 MHz
	m_ipccpu->set_addrmap(AS_PROGRAM, &imds2_state::ipc_mem_map);
	m_ipccpu->set_addrmap(AS_IO, &imds2_state::ipc_io_map);
	m_ipccpu->set_irq_acknowledge_callback("ipcsyspic", FUNC(pic8259_device::inta_cb));
	//config.m_minimum_quantum = attotime::from_hz(100);

	PIC8259(config, m_ipcsyspic, 0);
	m_ipcsyspic->out_int_callback().set(FUNC(imds2_state::ipc_intr_w));
	m_ipcsyspic->in_sp_callback().set_constant(1);

	PIC8259(config, m_ipclocpic, 0);
	m_ipclocpic->out_int_callback().set(m_ipcsyspic, FUNC(pic8259_device::ir7_w));
	m_ipclocpic->in_sp_callback().set_constant(1);

	PIT8253(config, m_ipctimer, 0);
	m_ipctimer->set_clk<0>(IPC_XTAL_Y1 / 16);
	m_ipctimer->set_clk<1>(IPC_XTAL_Y1 / 16);
	m_ipctimer->set_clk<2>(IPC_XTAL_Y1 / 16);
	m_ipctimer->out_handler<0>().set(m_ipcusart[0], FUNC(i8251_device::write_txc));
	m_ipctimer->out_handler<0>().append(m_ipcusart[0], FUNC(i8251_device::write_rxc));
	m_ipctimer->out_handler<1>().set(m_ipcusart[1], FUNC(i8251_device::write_txc));
	m_ipctimer->out_handler<1>().append(m_ipcusart[1], FUNC(i8251_device::write_rxc));
	m_ipctimer->out_handler<2>().set(m_ipclocpic, FUNC(pic8259_device::ir4_w));

	I8251(config, m_ipcusart[0], IPC_XTAL_Y1 / 9);
	m_ipcusart[0]->rts_handler().set(m_ipcusart[0], FUNC(i8251_device::write_cts));
	m_ipcusart[0]->rxrdy_handler().set(m_ipclocpic, FUNC(pic8259_device::ir0_w));
	m_ipcusart[0]->txrdy_handler().set(m_ipclocpic, FUNC(pic8259_device::ir1_w));
	m_ipcusart[0]->txd_handler().set(m_serial[0], FUNC(rs232_port_device::write_txd));

	I8251(config, m_ipcusart[1], IPC_XTAL_Y1 / 9);
	m_ipcusart[1]->rxrdy_handler().set(m_ipclocpic, FUNC(pic8259_device::ir2_w));
	m_ipcusart[1]->txrdy_handler().set(m_ipclocpic, FUNC(pic8259_device::ir3_w));
	m_ipcusart[1]->txd_handler().set(m_serial[1], FUNC(rs232_port_device::write_txd));
	m_ipcusart[1]->rts_handler().set(m_serial[1], FUNC(rs232_port_device::write_rts));
	m_ipcusart[1]->dtr_handler().set(m_serial[1], FUNC(rs232_port_device::write_dtr));

	LS259(config, m_ipcctrl); // A84

	RS232_PORT(config, m_serial[0], default_rs232_devices, nullptr);
	m_serial[0]->rxd_handler().set(m_ipcusart[0], FUNC(i8251_device::write_rxd));
	m_serial[0]->dsr_handler().set(m_ipcusart[0], FUNC(i8251_device::write_dsr));

	RS232_PORT(config, m_serial[1], default_rs232_devices, nullptr);
	m_serial[1]->rxd_handler().set(m_ipcusart[1], FUNC(i8251_device::write_rxd));
	m_serial[1]->cts_handler().set(m_ipcusart[1], FUNC(i8251_device::write_cts));
	m_serial[1]->dsr_handler().set(m_ipcusart[1], FUNC(i8251_device::write_dsr));

	IMDS2IOC(config, m_ioc);
	m_ioc->master_intr_cb().set(m_ipclocpic, FUNC(pic8259_device::ir6_w));
	m_ioc->parallel_int_cb().set(m_ipclocpic, FUNC(pic8259_device::ir5_w));
}

ROM_START(imds2)
	// ROM definition of IPC cpu (8085A)
	ROM_REGION(0x1000, "ipcrom", 0)
	ROM_DEFAULT_BIOS("mon13")
	// 1x2732 Copyright 1979
	ROM_SYSTEM_BIOS(0, "mon13", "Series II Monitor v1.3")
	ROMX_LOAD("ipc13_a82.bin", 0x0000, 0x1000, CRC(0889394f) SHA1(b7525baf1884a7d67402dea4b5566016a9861ef2), ROM_BIOS(0))
	// 2x2716 Copyright 1978
	ROM_SYSTEM_BIOS(1, "mon12", "Series II Monitor v1.2")
	ROMX_LOAD("ipc12_a57.bin", 0x0000, 0x0800, CRC(6496efaf) SHA1(1a9c0f1b19c1807803db3f1543f51349d7fd693a), ROM_BIOS(1))
	ROMX_LOAD("ipc12_a48.bin", 0x0800, 0x0800, CRC(258dc9a6) SHA1(3fde993aee06d9af5093d7a2d9a8cbd71fed0951), ROM_BIOS(1))
	// 2x2716 Copyright 1977
	ROM_SYSTEM_BIOS(2, "mon11", "Series II Monitor v1.1")
	ROMX_LOAD("ipc11_a57.bin", 0x0000, 0x0800, CRC(ffb7c036) SHA1(6f60cdfe20621c4b633c972adcb644a1c02eaa39), ROM_BIOS(2))
	ROMX_LOAD("ipc11_a48.bin", 0x0800, 0x0800, CRC(3696ff28) SHA1(38b435e10a81629430275aec051fb0a55ec1f6fd), ROM_BIOS(2))
ROM_END

/*    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY  FULLNAME */
COMP( 1979, imds2, 0,      0,      imds2,   imds2, imds2_state, empty_init, "Intel", "Intellec MDS-II", 0)
