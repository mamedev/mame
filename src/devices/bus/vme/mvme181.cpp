// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Motorola MVME181
 *
 * Sources:
 *  - OpenBSD 5.5 source code
 *
 * TODO:
 *  - skeleton only
 */
/*
 * dip  function
 *  1   system controller enable
 *  2   VMEbus request level 0
 *  3   VMEbus request level 1
 *  4   VMEbus request level 2
 *  5   VMEbus request level 3
 *  6   system mode/board select
 *  7   onboard memory base address (0x00000000, 0x00800000, 0x01000000, 0x01800000)
 *  8   onboard memory base address
 */

#include "emu.h"
#include "mvme181.h"

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(VME_MVME181, vme_mvme181_card_device, "mvme181", "Motorola MVME181")

vme_mvme181_card_device::vme_mvme181_card_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, VME_MVME181, tag, owner, clock)
	, device_vme_card_interface(mconfig, *this)
	, m_cpu(*this, "cpu")
	, m_mmu(*this, "mmu%u", 0U)
	, m_rtc(*this, "rtc")
	, m_duart(*this, "duart")
	, m_serial(*this, "serial%u", 0U)
	, m_boot(*this, "boot")
{
}

ROM_START(mvme181)
	ROM_REGION32_BE(0x80000, "eprom", 0)
	ROM_SYSTEM_BIOS(0, "v301", "181bug Release Version 3.01 - 08/17/89")
	ROMX_LOAD("181bug_a0.read.bin", 0x0000, 0x20000, CRC(79018bdb) SHA1(c52bfea145a56d2ad8765a508292557f096c4208), ROM_SKIP(3) | ROM_BIOS(0))
	ROMX_LOAD("181bug_a1.read.bin", 0x0001, 0x20000, CRC(a5ac2d78) SHA1(feb16d3767cd382094de78b1043c702e29cde2ee), ROM_SKIP(3) | ROM_BIOS(0))
	ROMX_LOAD("181bug_a2.read.bin", 0x0002, 0x20000, CRC(ddb90610) SHA1(30bd0cba86436689f8994c448515dd409e66d895), ROM_SKIP(3) | ROM_BIOS(0))
	ROMX_LOAD("181bug_a3.read.bin", 0x0003, 0x20000, CRC(646d301f) SHA1(5871b5194e7fc623923f5df005b0463002e85626), ROM_SKIP(3) | ROM_BIOS(0))
ROM_END

static INPUT_PORTS_START(mvme181)
INPUT_PORTS_END

const tiny_rom_entry *vme_mvme181_card_device::device_rom_region() const
{
	return ROM_NAME(mvme181);
}

ioport_constructor vme_mvme181_card_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(mvme181);
}

void vme_mvme181_card_device::device_start()
{
	m_isr = 0;
	m_imr = 0;

	// memory tap offers a tidy solution for the "phantom" rtc
	m_cpu->space(AS_PROGRAM).install_read_tap(0xff820000, 0xff82001f, "rtc",
		[this](offs_t offset, u32 &data, u32 mem_mask)
		{
			if (!m_rtc->chip_enable())
			{
				if (BIT(offset, 2))
					m_rtc->read_1();
				else
					m_rtc->read_0();
			}
			else if (BIT(offset, 4))
				data = u32(m_rtc->read_data()) << 24;
			else
				m_rtc->write_data(BIT(offset, 2));
		});
}

void vme_mvme181_card_device::device_reset()
{
	m_boot.select(0);
}

void vme_mvme181_card_device::device_add_mconfig(machine_config &config)
{
	MC88100(config, m_cpu, 40_MHz_XTAL / 2);
	m_cpu->set_addrmap(AS_PROGRAM, &vme_mvme181_card_device::cpu_mem);
	m_cpu->set_cmmu_code([this](u32 const address) -> mc88200_device & { return *m_mmu[0]; });
	m_cpu->set_cmmu_data([this](u32 const address) -> mc88200_device & { return *m_mmu[1]; });

	MC88200(config, m_mmu[0], 40_MHz_XTAL / 2, 0x7e).set_mbus(m_cpu, AS_PROGRAM);
	MC88200(config, m_mmu[1], 40_MHz_XTAL / 2, 0x7f).set_mbus(m_cpu, AS_PROGRAM);

	DS1315(config, m_rtc, 0); // DS1216

	SCN2681(config, m_duart, 3.6864_MHz_XTAL); // SCC68692C1A44
	m_duart->irq_cb().set(FUNC(vme_mvme181_card_device::irq_w<6>));
	m_duart->outport_cb().set([this](u8 data) { LOG("port 0x%02x\n", data); });

	RS232_PORT(config, m_serial[0], default_rs232_devices, "terminal");
	RS232_PORT(config, m_serial[1], default_rs232_devices, nullptr);

	// TODO: dsr, dtr
	m_duart->a_tx_cb().set(m_serial[0], FUNC(rs232_port_device::write_txd));
	m_duart->b_tx_cb().set(m_serial[1], FUNC(rs232_port_device::write_txd));
	m_serial[0]->rxd_handler().set(m_duart, FUNC(scn2681_device::rx_a_w));
	m_serial[1]->rxd_handler().set(m_duart, FUNC(scn2681_device::rx_b_w));
}

void vme_mvme181_card_device::cpu_mem(address_map &map)
{
	map(0x0000'0000, 0x007f'ffff).view(m_boot);
	m_boot[0](0x0000'0000, 0x0007'ffff).rom().region("eprom", 0);
	m_boot[1](0x0000'0000, 0x007f'ffff).ram();

	map(0xff80'0000, 0xff87'ffff).rom().region("eprom", 0);
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

template <unsigned N> void vme_mvme181_card_device::irq_w(int state)
{
	LOG("irq_w<%d> %d\n", N, state);
	if (state)
		m_isr |= 1U << N;
	else
		m_isr &= ~(1U << N);

	interrupt();
}

void vme_mvme181_card_device::interrupt()
{
	m_cpu->set_input_line(INPUT_LINE_IRQ0, bool(m_isr & m_imr));
}
