// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Tadpole Technology TP881V
 *
 * Sources:
 *  - TPROM/88K User Manual, Monitor/Debugger For 88000 Boards, Issue: 1.3, 26 June 1990, Part No. DS-1003
 *  - TPIX/88K version 1.06 source code
 *
 * TODO:
 *  - scsi dma controllers
 *  - serial dma
 *  - vme interface
 *  - interrupt enables/masking
 *  - multiprocessor configurations
 */

#include "emu.h"
#include "tp881v.h"

#include "machine/input_merger.h"

#include "bus/nscsi/hd.h"
#include "bus/nscsi/cd.h"

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(VME_TP881V, vme_tp881v_card_device, "tp881v", "Tadpole Technology TP881V")

vme_tp881v_card_device::vme_tp881v_card_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, VME_TP881V, tag, owner, clock)
	, device_vme_card_interface(mconfig, *this)
	, m_cpu(*this, "cpu")
	, m_mmu(*this, "mmu%u", 0U)
	, m_cio(*this, "cio%u", 0U)
	, m_rtc(*this, "rtc")
	, m_scsi(*this, "scsi%u:7:ncr53c90a", 0U)
	, m_net(*this, "net")
	, m_scc(*this, "scc%u", 0U)
	, m_scc_dma(*this, "scc_dma")
	, m_vcs(*this, "vcs")
	, m_gcs(*this, "gcs%u", 0U)
	, m_eeprom(*this, "eeprom")
	, m_serial(*this, "serial%u", 0U)
{
}

ROM_START(tp881v)
	ROM_REGION32_BE(0x40000, "eprom", 0)
	ROM_SYSTEM_BIOS(0, "v504", "TP881 Multiprocessor Monitor V5.04")
	ROMX_LOAD("881v__5.04l.bin", 0x0002, 0x20000, CRC(d9027c64) SHA1(88cca011dee005d273a1cd7048480ff3fac9c06a), ROM_REVERSE | ROM_GROUPWORD | ROM_SKIP(2) | ROM_BIOS(0))
	ROMX_LOAD("881v__5.04h.bin", 0x0000, 0x20000, CRC(74de0772) SHA1(8e5bfd427c0bdea4ab350333fd5a814712014302), ROM_REVERSE | ROM_GROUPWORD | ROM_SKIP(2) | ROM_BIOS(0))
ROM_END

static INPUT_PORTS_START(tp881v)
INPUT_PORTS_END

const tiny_rom_entry *vme_tp881v_card_device::device_rom_region() const
{
	return ROM_NAME(tp881v);
}

ioport_constructor vme_tp881v_card_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(tp881v);
}

void vme_tp881v_card_device::device_start()
{
}

void vme_tp881v_card_device::device_reset()
{
}

static void scsi_devices(device_slot_interface &device)
{
	device.option_add("cdrom", NSCSI_CDROM);
	device.option_add("harddisk", NSCSI_HARDDISK);
	device.option_add("ncr53c90a", NCR53C90A);
}

void vme_tp881v_card_device::device_add_mconfig(machine_config &config)
{
	MC88100(config, m_cpu, 40_MHz_XTAL / 2);
	m_cpu->set_addrmap(AS_PROGRAM, &vme_tp881v_card_device::cpu_mem);
	m_cpu->set_cmmu_code([this](u32 const address) -> mc88200_device & { return *m_mmu[0]; });
	m_cpu->set_cmmu_data([this](u32 const address) -> mc88200_device & { return *m_mmu[1]; });

	MC88200(config, m_mmu[0], 40_MHz_XTAL / 2, 0x00).set_mbus(m_cpu, AS_PROGRAM);
	MC88200(config, m_mmu[1], 40_MHz_XTAL / 2, 0x01).set_mbus(m_cpu, AS_PROGRAM);

	// per-jp interrupt controllers
	// 4MHz input clock, ct3 gives 100Hz clock, ct2 counts at 10kHz
	Z8036(config, m_cio[0], 4'000'000); // Z0803606VSC
	/*
	 * port a bit mode ddr 0xff
	 * bit  i/o  function
	 *  0    i   porta int?
	 *  1    i   ethernet
	 *  2    i   scc dma
	 *  3    i   scsi0
	 *  4    i   scsi1
	 *  5    i   scc
	 *  6    i   acfail
	 *  7    i   parity
	 *
	 * port b bit mode ddr 0xbf
	 * bit  i/o  function
	 *  0    i   port b
	 *  1    i   mbus err
	 *  2    i   port b err
	 *  3    i   vme
	 *  4    i   vme mailbox
	 *  5    i   port b err
	 *  6    o   software interrupt
	 *  7    i   vsb
	 *
	 * port c ? ddr 0xf
	 * bit  i/o  function
	 *  0    i
	 *  1    i
	 *  2    i
	 *  3    i
	 */
	m_cio[0]->irq_wr_cb().set_inputline(m_cpu, INPUT_LINE_IRQ0);

	Z8036(config, m_cio[1], 4'000'000); // Z0803606VSC
	Z8036(config, m_cio[2], 4'000'000); // Z0803606VSC
	Z8036(config, m_cio[3], 4'000'000); // Z0803606VSC
	Z8036(config, m_cio[4], 4'000'000); // Z0803606VSC
	Z8036(config, m_cio[5], 4'000'000); // Z0803606VSC
	Z8036(config, m_cio[6], 4'000'000); // Z0803606VSC
	Z8036(config, m_cio[7], 4'000'000); // Z0803606VSC

	M48T02(config, m_rtc, 0);

	NSCSI_BUS(config, "scsi0", 0);
	NSCSI_CONNECTOR(config, "scsi0:0", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi0:1", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi0:2", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi0:3", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi0:4", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi0:5", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi0:6", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi0:7").option_set("ncr53c90a", NCR53C90A).machine_config(
		[this](device_t *device)
		{
			ncr53c90a_device &ncr53c90a(downcast<ncr53c90a_device &>(*device));

			ncr53c90a.set_clock(20'000'000);
			ncr53c90a.irq_handler_cb().set(m_cio[0], FUNC(z8036_device::pa3_w));
			//ncr53c90a.drq_cb().set(*this, ...);
		});

	NSCSI_BUS(config, "scsi1", 0);
	NSCSI_CONNECTOR(config, "scsi1:0", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi1:1", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi1:2", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi1:3", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi1:4", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi1:5", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi1:6", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi1:7").option_set("ncr53c90a", NCR53C90A).machine_config(
		[this](device_t *device)
		{
			ncr53c90a_device &ncr53c90a(downcast<ncr53c90a_device &>(*device));

			ncr53c90a.set_clock(20'000'000);
			ncr53c90a.irq_handler_cb().set(m_cio[0], FUNC(z8036_device::pa4_w));
			//ncr53c90a.drq_cb().set(*this, ...);
		});

	I82596_BE32(config, m_net, 20'000'000); // A82596DX-25
	m_net->out_irq_cb().set(m_cio[0], FUNC(z8036_device::pa1_w));
	m_net->set_addrmap(0, &vme_tp881v_card_device::net_mem);

	input_merger_any_high_device &scc_irq(INPUT_MERGER_ANY_HIGH(config, "scc_irq"));
	scc_irq.output_handler().set(m_cio[0], FUNC(z8036_device::pa5_w));

	SCC8030(config, m_scc[0], 3.6864_MHz_XTAL); // Z0803006VSC
	m_scc[0]->configure_channels(m_scc[0]->clock(), m_scc[0]->clock(), m_scc[0]->clock(), m_scc[0]->clock());
	m_scc[0]->out_int_callback().set(scc_irq, FUNC(input_merger_any_high_device::in_w<0>));

	SCC8030(config, m_scc[1], 3.6864_MHz_XTAL); // Z0803006VSC
	m_scc[1]->configure_channels(m_scc[1]->clock(), m_scc[1]->clock(), m_scc[1]->clock(), m_scc[1]->clock());
	m_scc[1]->out_int_callback().set(scc_irq, FUNC(input_merger_any_high_device::in_w<1>));

	RS232_PORT(config, m_serial[0], default_rs232_devices, "terminal");
	RS232_PORT(config, m_serial[1], default_rs232_devices, nullptr);

	m_scc[0]->out_txda_callback().set(m_serial[0], FUNC(rs232_port_device::write_txd));
	m_scc[0]->out_dtra_callback().set(m_serial[0], FUNC(rs232_port_device::write_dtr));
	m_scc[0]->out_rtsa_callback().set(m_serial[0], FUNC(rs232_port_device::write_rts));
	m_scc[0]->out_txdb_callback().set(m_serial[1], FUNC(rs232_port_device::write_txd));
	m_scc[0]->out_dtrb_callback().set(m_serial[1], FUNC(rs232_port_device::write_dtr));
	m_scc[0]->out_rtsb_callback().set(m_serial[1], FUNC(rs232_port_device::write_rts));

	m_serial[0]->rxd_handler().set(m_scc[0], FUNC(z80scc_device::rxa_w));
	m_serial[0]->dcd_handler().set(m_scc[0], FUNC(z80scc_device::dcda_w));
	m_serial[0]->cts_handler().set(m_scc[0], FUNC(z80scc_device::ctsa_w));
	m_serial[1]->rxd_handler().set(m_scc[0], FUNC(z80scc_device::rxb_w));
	m_serial[1]->dcd_handler().set(m_scc[0], FUNC(z80scc_device::dcdb_w));
	m_serial[1]->cts_handler().set(m_scc[0], FUNC(z80scc_device::ctsb_w));

	RS232_PORT(config, m_serial[2], default_rs232_devices, nullptr);
	RS232_PORT(config, m_serial[3], default_rs232_devices, nullptr);

	m_scc[1]->out_txda_callback().set(m_serial[2], FUNC(rs232_port_device::write_txd));
	m_scc[1]->out_dtra_callback().set(m_serial[2], FUNC(rs232_port_device::write_dtr));
	m_scc[1]->out_rtsa_callback().set(m_serial[2], FUNC(rs232_port_device::write_rts));
	m_scc[1]->out_txdb_callback().set(m_serial[3], FUNC(rs232_port_device::write_txd));
	m_scc[1]->out_dtrb_callback().set(m_serial[3], FUNC(rs232_port_device::write_dtr));
	m_scc[1]->out_rtsb_callback().set(m_serial[3], FUNC(rs232_port_device::write_rts));

	m_serial[2]->rxd_handler().set(m_scc[1], FUNC(z80scc_device::rxa_w));
	m_serial[2]->dcd_handler().set(m_scc[1], FUNC(z80scc_device::dcda_w));
	m_serial[2]->cts_handler().set(m_scc[1], FUNC(z80scc_device::ctsa_w));
	m_serial[3]->rxd_handler().set(m_scc[1], FUNC(z80scc_device::rxb_w));
	m_serial[3]->dcd_handler().set(m_scc[1], FUNC(z80scc_device::dcdb_w));
	m_serial[3]->cts_handler().set(m_scc[1], FUNC(z80scc_device::ctsb_w));

	// TODO: MC68440 is function and pin compatible with MC68450/HD63450, but
	// has only two DMA channels instead of four.
	HD63450(config, m_scc_dma, 10'000'000); // MC68440FN10
	m_scc_dma->set_cpu_tag(m_cpu);
	m_scc_dma->irq_callback().set(m_cio[0], FUNC(z8036_device::pa2_w));

	// VME control and status CIO
	Z8036(config, m_vcs, 4'000'000); // Z0803606VSC
	/*
	 * port a
	 * bit  i/o  function
	 *  0    i   vme dram address bit 25
	 *  1    i   vme dram address bit 26
	 *  2    i   vme dram address bit 27
	 *  3    i   vme dram address bit 28
	 *  4    i   vme dram address bit 29
	 *  5    i   vme dram address bit 30
	 *  6    i   vme dram address bit 31
	 *  7    ?
	 */

	// general control and status CIO 0
	Z8036(config, m_gcs[0], 4'000'000); // Z0803606VSC
	/*
	 * port a ddr 0x01
	 * bit  i/o  function
	 *  0    i
	 *  1    o
	 *  2    o
	 *  3    o   fail led
	 *  4    o   user led
	 *  5    o
	 *  6    o
	 *  7    o
	 *
	 * port b ddr 0x00
	 * bit  i/o  function
	 *  0    o
	 *  1    o
	 *  2    o
	 *  3    o
	 *  4    o
	 *  5    o
	 *  6    o   scsi1 dma memory->device
	 *  7    o   scsi0 dma memory->device
	 *
	 * port c 0xe
	 * bit  i/o  function
	 *  0    o
	 *  1    i   vme irq level bit 0
	 *  2    i   vme irq level bit 1
	 *  3    i   vme irq level bit 2
	 */
	// back panel has scsi0, scsi1, vme, net, sysfail, user,

	 // general control and status CIO 1
	Z8036(config, m_gcs[1], 4'000'000); // Z0803606VSC
	/*
	 * port c ddr 0x1
	 * bit  i/o  function
	 *  0    i   eeprom data in
	 *  1    o   eeprom data out
	 *  2    o   eeprom chip select
	 *  3    o   eeprom clock
	 */
	m_gcs[1]->pc_wr_cb().set(
		[this](u8 data)
		{
			// chip select
			if (!BIT(data, 6))
				m_eeprom->cs_w(BIT(data, 2));

			// data out
			if (!BIT(data, 5))
				m_eeprom->di_w(BIT(data, 1));

			// clock
			if (!BIT(data, 4))
				m_eeprom->sk_w(BIT(data, 3));
		});
	m_gcs[1]->pc_rd_cb().set(m_eeprom, FUNC(nmc9306_device::do_r));

	NMC9306(config, m_eeprom, 0);
}

void vme_tp881v_card_device::cpu_mem(address_map &map)
{
	map(0x0000'0000, 0x0003'ffff).rom().region("eprom", 0);
	//map(0x2000'0000, 0x20ff'ffff); // vme short space (a24 d32)

	map(0x4000'0000, 0x41ff'ffff).ram().share("ram");
	//map(0x8000'0000, 0xbfff'ffff); // vme extended space (a32 d32)
	//map(0xc000'0000, 0xc0ff'ffff); // vsb space

	map(0xfff3'0000, 0xfff3'003f).m(m_scsi[0], FUNC(ncr53c90a_device::map)).umask32(0x000000ff);
	//0xfff30080; // dmac scsi0
	map(0xfff3'8000, 0xfff3'803f).m(m_scsi[1], FUNC(ncr53c90a_device::map)).umask32(0x000000ff);
	//0xfff38080; // dmac scsi1
	//0xfff40000; // vme a16 d16 base
	map(0xfff5'2000, 0xfff5'207f).rw(m_scc[1], FUNC(z80scc_device::zbus_r), FUNC(z80scc_device::zbus_w)).umask32(0x000000ff);
	map(0xfff5'4000, 0xfff5'407f).rw(m_scc[0], FUNC(z80scc_device::zbus_r), FUNC(z80scc_device::zbus_w)).umask32(0x000000ff);
	map(0xfff5'6000, 0xfff5'60bf).rw(m_gcs[0], FUNC(z8036_device::read), FUNC(z8036_device::write)).umask32(0x000000ff);
	map(0xfff5'8000, 0xfff5'80bf).rw(m_gcs[1], FUNC(z8036_device::read), FUNC(z8036_device::write)).umask32(0x000000ff);
	map(0xfff5'a000, 0xfff5'a0bf).rw(m_vcs, FUNC(z8036_device::read), FUNC(z8036_device::write)).umask32(0x000000ff);
	map(0xfff5'e000, 0xfff5'ffff).rw(m_rtc, FUNC(m48t02_device::read), FUNC(m48t02_device::write)).umask32(0x000000ff);
	//0xfff60000; // vme iack
	//map(0xfff7'8002, 0xfff7'8003).w(m_net, FUNC(i82596_device::port));
	//map(0xfff7'c000, 0xfffc'0000).lw8([this](u8 data) { m_net->ca(1); }, "net_ca");
	map(0xfff8'0000, 0xfff8'00bf).rw(m_cio[0], FUNC(z8036_device::read), FUNC(z8036_device::write)).umask32(0x000000ff);
	map(0xfff8'0400, 0xfff8'04bf).rw(m_cio[1], FUNC(z8036_device::read), FUNC(z8036_device::write)).umask32(0x000000ff);
	map(0xfff8'0800, 0xfff8'08bf).rw(m_cio[2], FUNC(z8036_device::read), FUNC(z8036_device::write)).umask32(0x000000ff);
	map(0xfff8'0c00, 0xfff8'0cbf).rw(m_cio[3], FUNC(z8036_device::read), FUNC(z8036_device::write)).umask32(0x000000ff);
	map(0xfff9'0000, 0xfff9'00bf).rw(m_cio[4], FUNC(z8036_device::read), FUNC(z8036_device::write)).umask32(0x000000ff);
	map(0xfff9'0400, 0xfff9'04bf).rw(m_cio[5], FUNC(z8036_device::read), FUNC(z8036_device::write)).umask32(0x000000ff);
	map(0xfff9'0800, 0xfff9'08bf).rw(m_cio[6], FUNC(z8036_device::read), FUNC(z8036_device::write)).umask32(0x000000ff);
	map(0xfff9'0c00, 0xfff9'0cbf).rw(m_cio[7], FUNC(z8036_device::read), FUNC(z8036_device::write)).umask32(0x000000ff);
}

void vme_tp881v_card_device::net_mem(address_map &map)
{
	map(0x4000'0000, 0x41ff'ffff).ram().share("ram");
}
