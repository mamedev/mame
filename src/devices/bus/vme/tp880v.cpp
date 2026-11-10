// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Tadpole Technology TP880V
 *
 * Sources:
 *  - TPROM/88K User Manual, Monitor/Debugger For 88000 Boards, Issue: 1.3, 26 June 1990, Part No. DS-1003
 *
 * TODO:
 *  - cio hookup is incomplete, inter-processor interrupts are not working
 *  - dmac
 *  - vme
 *  - leds and switches
 */

#include "emu.h"
#include "tp880v.h"

#include "bus/rs232/rs232.h"
#include "bus/nscsi/devices.h"
#include "bus/vme/vme.h"
#include "cpu/m68000/m68000.h"
#include "cpu/m88000/m88000.h"
#include "machine/hd63450.h"
#include "machine/mc68681.h"
#include "machine/mc88200.h"
#include "machine/ncr53c90.h"
#include "machine/timekpr.h"
#include "machine/z8536.h"

#include "endianness.h"

#define VERBOSE 0
#include "logmacro.h"

namespace {

class tp880v_card_device
	: public device_t
	, public device_vme_card_interface
{
public:
	tp880v_card_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: device_t(mconfig, VME_TP880V, tag, owner, clock)
		, device_vme_card_interface(mconfig, *this)
		, m_cpu(*this, "cpu")
		, m_mmu(*this, "mmu%u", 0U)
		, m_ios(*this, "ios")
		, m_cio(*this, "cio%u", 0U)
		, m_dma(*this, "dma")
		, m_rtc(*this, "rtc")
		, m_scsi(*this, "ncr53c90")
		, m_duart(*this, "duart")
		, m_serial(*this, "serial%u", 0U)
		, m_ram(*this, "ram")
		, m_ram_68k(nullptr)
	{
	}

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void cpu_mem(address_map &map) ATTR_COLD;
	void ios_mem(address_map &map) ATTR_COLD;

	required_device<mc88100_device> m_cpu;
	required_device_array<mc88200_device, 2> m_mmu;
	required_device<m68000_device> m_ios;

	required_device_array<z8036_device, 2> m_cio;
	required_device<hd63450_device> m_dma;
	required_device<m48t02_device> m_rtc;
	required_device<ncr53c90_device> m_scsi;
	required_device<scn2681_device> m_duart;

	required_device_array<rs232_port_device, 2> m_serial;

	required_shared_ptr<u32> m_ram;
	util::endian_cast<u32, u16, std::endian::big> m_ram_68k;
};

void tp880v_card_device::device_start()
{
	m_ram_68k = util::big_endian_cast<u16>(m_ram.target());
}

void tp880v_card_device::device_reset()
{
}

void tp880v_card_device::device_add_mconfig(machine_config &config)
{
	MC88100(config, m_cpu, 40_MHz_XTAL / 2);
	m_cpu->set_addrmap(AS_PROGRAM, &tp880v_card_device::cpu_mem);
	m_cpu->set_cmmu_code([this](u32 const address) -> mc88200_device & { return *m_mmu[0]; });
	m_cpu->set_cmmu_data([this](u32 const address) -> mc88200_device & { return *m_mmu[1]; });

	MC88200(config, m_mmu[0], 40_MHz_XTAL / 2, 0x7e).set_mbus(m_cpu, AS_PROGRAM);
	MC88200(config, m_mmu[1], 40_MHz_XTAL / 2, 0x7f).set_mbus(m_cpu, AS_PROGRAM);

	M68000(config, m_ios, 10'000'000);
	m_ios->set_addrmap(AS_PROGRAM, &tp880v_card_device::ios_mem);

	Z8036(config, m_cio[0], 4'000'000); // Z0803606VSC
	m_cio[0]->irq_wr_cb().set_inputline(m_ios, INPUT_LINE_IRQ4); // ?
	Z8036(config, m_cio[1], 4'000'000); // Z0803606VSC
	m_cio[0]->irq_wr_cb().set_inputline(m_ios, INPUT_LINE_IRQ1); // ?

	// TODO: MC68440 is function and pin compatible with MC68450/HD63450, but
	// has only two DMA channels instead of four.
	HD63450(config, m_dma, 10'000'000, m_ios, AS_PROGRAM); // MC68440FN10
	m_dma->irq_callback().set_inputline(m_ios, INPUT_LINE_IRQ3);
	m_dma->dma8_read<0>().set(m_scsi, FUNC(ncr53c90_device::dma_r));
	m_dma->dma8_write<0>().set(m_scsi, FUNC(ncr53c90_device::dma_w));

	M48T02(config, m_rtc); // MK40T02B-25

	auto &scsi(NSCSI_BUS(config, "scsi"));
	NSCSI_CONNECTOR(config, "scsi:0", default_scsi_devices, "harddisk", false);
	NSCSI_CONNECTOR(config, "scsi:1", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:2", default_scsi_devices, "tape", false);
	NSCSI_CONNECTOR(config, "scsi:3", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:4", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:5", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:6", default_scsi_devices, nullptr, false);

	NCR53C90(config, m_scsi, 10'000'000);
	scsi.set_external_device(7, m_scsi);
	m_scsi->irq_handler_cb().set_inputline(m_ios, INPUT_LINE_IRQ2);
	m_scsi->drq_handler_cb().set(m_dma, FUNC(hd63450_device::drq0_w));

	SCN2681(config, m_duart, 3.6864_MHz_XTAL);
	m_duart->irq_cb().set_inputline(m_ios, INPUT_LINE_IRQ6);
	m_duart->outport_cb().set([this](int state) { m_cpu->set_input_line(INPUT_LINE_RESET, state); }).bit(5);

	RS232_PORT(config, m_serial[0], default_rs232_devices, "terminal");
	RS232_PORT(config, m_serial[1], default_rs232_devices, nullptr);

	m_duart->a_tx_cb().set(m_serial[0], FUNC(rs232_port_device::write_txd));
	m_duart->b_tx_cb().set(m_serial[1], FUNC(rs232_port_device::write_txd));

	m_serial[0]->rxd_handler().set(m_duart, FUNC(scn2681_device::rx_a_w));
	m_serial[1]->rxd_handler().set(m_duart, FUNC(scn2681_device::rx_b_w));
}

void tp880v_card_device::cpu_mem(address_map &map)
{
	map(0x0000'0000, 0x003f'ffff).ram().share("ram");

	map(0xfff0'0002, 0xfff0'0002).select(0x3f00).lrw8(
		[this](offs_t offset) { return m_cio[0]->read(offset >> 8); }, "cio0_r",
		[this](offs_t offset, u8 data) { m_cio[0]->write(offset >> 8, data); }, "cio0_w");
	map(0xfff0'8002, 0xfff0'8002).select(0x3f00).lrw8(
		[this](offs_t offset) { return m_cio[1]->read(offset >> 8); }, "cio1_r",
		[this](offs_t offset, u8 data) { m_cio[1]->write(offset >> 8, data); }, "cio1_w");

	//map(0xfff1'0000, 0xfff1'0000); // vme iack vector?
}

void tp880v_card_device::ios_mem(address_map &map)
{
	map(0x00'0000, 0x01'ffff).rom().region("eprom", 0);

	map(0x10'0000, 0x10'3fff).ram(); // SRAM

	map(0x20'0000, 0x20'001f).rw(m_duart, FUNC(scn2681_device::read), FUNC(scn2681_device::write)).umask16(0x00ff);
	map(0x24'0000, 0x2f'001f).m(m_scsi, FUNC(ncr53c90_device::map)).umask16(0x00ff);
	map(0x28'0000, 0x28'00ff).rw(m_dma, FUNC(hd63450_device::read), FUNC(hd63450_device::write));
	map(0x2c'0000, 0x2c'0fff).rw(m_rtc, FUNC(m48t02_device::read), FUNC(m48t02_device::write)).umask16(0x00ff);

	map(0x80'0000, 0xbf'ffff).lrw16(
		[this](offs_t offset, u16 mem_mask) { return m_ram_68k[offset]; }, "ram_68k_r",
		[this](offs_t offset, u16 data, u16 mem_mask) { COMBINE_DATA(&m_ram_68k[offset]); }, "ram_68k_w");
}

ROM_START(tp880v)
	ROM_REGION16_BE(0x20000, "eprom", 0)
	ROM_SYSTEM_BIOS(0, "v421", "TP880V Monitor - Revision R4.21")
	ROMX_LOAD("880-v__v4.2.1.bin", 0x00000, 0x20000, CRC(c5e64026) SHA1(724f54573b1b0c79f20250fdb6434bb2248c5e04), ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(0)) // D27C1024D-15
ROM_END

const tiny_rom_entry *tp880v_card_device::device_rom_region() const
{
	return ROM_NAME(tp880v);
}

static INPUT_PORTS_START(tp880v)
INPUT_PORTS_END

ioport_constructor tp880v_card_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(tp880v);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(VME_TP880V, device_vme_card_interface, tp880v_card_device, "tp880v", "Tadpole Technology TP880V")
