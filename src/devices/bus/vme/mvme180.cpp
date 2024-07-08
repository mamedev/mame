// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Motorola MVME180
 *
 * Sources:
 *  - OpenBSD 5.5 source code
 *
 * TODO:
 *  - skeleton only
 */

#include "emu.h"
#include "mvme180.h"

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(VME_MVME180, vme_mvme180_card_device, "mvme180", "Motorola MVME180")

vme_mvme180_card_device::vme_mvme180_card_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, VME_MVME180, tag, owner, clock)
	, device_vme_card_interface(mconfig, *this)
	, m_cpu(*this, "cpu")
	, m_mmu(*this, "mmu%u", 0U)
	, m_duart(*this, "duart")
	, m_serial(*this, "serial%u", 0U)
	, m_boot(*this, "boot")
{
}

ROM_START(mvme180)
	ROM_REGION32_BE(0x40000, "eprom", 0)
	ROM_SYSTEM_BIOS(0, "v25", "180bug version 2.5 copyright 1988 Motorola")
	ROMX_LOAD("51_w5110b19_u34__8846__180bug_2.5.u34", 0x0003, 0x10000, CRC(9b4e483a) SHA1(c9b1e9e1735a015c38eb223ab126c40c40a9f89d), ROM_SKIP(3) | ROM_BIOS(0))
	ROMX_LOAD("51_w5110b20_u35__8846__180bug_2.5.u35", 0x0002, 0x10000, CRC(1170b048) SHA1(ab8e2917c7150e4f74be600507690aa35444244c), ROM_SKIP(3) | ROM_BIOS(0))
	ROMX_LOAD("51_w5110b21_u36__8846__180bug_2.5.u36", 0x0001, 0x10000, CRC(270fbc7b) SHA1(a61b8701c5f1133f930320f313309008ff5d5504), ROM_SKIP(3) | ROM_BIOS(0))
	ROMX_LOAD("51_w5110b22_u37__8846__180bug_2.5.u37", 0x0000, 0x10000, CRC(67a60a6c) SHA1(0105317abf0105216078c94c02c223dd1cd61068), ROM_SKIP(3) | ROM_BIOS(0))
ROM_END

static INPUT_PORTS_START(mvme180)
INPUT_PORTS_END

const tiny_rom_entry *vme_mvme180_card_device::device_rom_region() const
{
	return ROM_NAME(mvme180);
}

ioport_constructor vme_mvme180_card_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(mvme180);
}

void vme_mvme180_card_device::device_start()
{
	m_isr = 0;
	m_imr = 0;
}

void vme_mvme180_card_device::device_reset()
{
	m_boot.select(0);
}

void vme_mvme180_card_device::device_add_mconfig(machine_config &config)
{
	MC88100(config, m_cpu, 40_MHz_XTAL / 2);
	m_cpu->set_addrmap(AS_PROGRAM, &vme_mvme180_card_device::cpu_mem);
	m_cpu->set_cmmu_code([this](u32 const address) -> mc88200_device & { return *m_mmu[0]; });
	m_cpu->set_cmmu_data([this](u32 const address) -> mc88200_device & { return *m_mmu[1]; });

	MC88200(config, m_mmu[0], 40_MHz_XTAL / 2, 0x7e).set_mbus(m_cpu, AS_PROGRAM);
	MC88200(config, m_mmu[1], 40_MHz_XTAL / 2, 0x7f).set_mbus(m_cpu, AS_PROGRAM);

	SCN2681(config, m_duart, 3.6864_MHz_XTAL);
	m_duart->irq_cb().set(FUNC(vme_mvme180_card_device::irq_w<6>));
	m_duart->outport_cb().set([this](u8 data) { LOG("port 0x%02x\n", data); });

	RS232_PORT(config, m_serial[0], default_rs232_devices, "terminal");
	RS232_PORT(config, m_serial[1], default_rs232_devices, nullptr);

	// TODO: dsr, dtr
	m_duart->a_tx_cb().set(m_serial[0], FUNC(rs232_port_device::write_txd));
	m_duart->b_tx_cb().set(m_serial[1], FUNC(rs232_port_device::write_txd));
	m_serial[0]->rxd_handler().set(m_duart, FUNC(scn2681_device::rx_a_w));
	m_serial[1]->rxd_handler().set(m_duart, FUNC(scn2681_device::rx_b_w));
}

void vme_mvme180_card_device::cpu_mem(address_map &map)
{
	map(0x0000'0000, 0x007f'ffff).view(m_boot);
	m_boot[0](0x0000'0000, 0x0003'ffff).rom().region("eprom", 0);
	m_boot[1](0x0000'0000, 0x007f'ffff).ram();

	map(0xff80'0000, 0xff83'ffff).rom().region("eprom", 0);
	// 0xff81'0000 // ds1287 rtc?
	map(0xffe1'0000, 0xffe1'0003).lr32([this]() { return m_isr; }, "isr");
	map(0xffe2'0000, 0xffe2'0003).lw32(
		[this](u32 data)
		{
			LOG("imr 0x%08x (%s)\n", data, machine().describe_context());
			m_imr = data;

			m_boot.select(1);

			interrupt();
		}, "imr");
	// 0xffe3'0000 // clear parity error interrupt?
	map(0xffe4'0000, 0xffe4'003f).rw(m_duart, FUNC(scn2681_device::read), FUNC(scn2681_device::write)).umask32(0x000000ff);
	// 0xffe8'0000 // vme vector register?
	// 0xffee'0000 // clear abort interrupt?
}

template <unsigned N> void vme_mvme180_card_device::irq_w(int state)
{
	LOG("irq_w<%d> %d\n", N, state);
	if (state)
		m_isr |= 1U << N;
	else
		m_isr &= ~(1U << N);

	interrupt();
}

void vme_mvme180_card_device::interrupt()
{
	m_cpu->set_input_line(INPUT_LINE_IRQ0, bool(m_isr & m_imr));
}
