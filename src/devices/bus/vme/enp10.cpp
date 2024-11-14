// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Communication Machinery Corporation Ethernet Node Processor (ENP-10)
 *
 * Sources:
 *  - Ethernet Node Processor ENP-30 Reference Guide (6213000-05B), Communication Machinery Corporation, November 15, 1988
 *
 * TODO:
 *  - verify registers
 *  - uart?
 */

/*
 * WIP
 * ---
 *  - 0xef'8010-0xef'802f appears to be a uart, but doesn't match scn2681 per ENP-30 documentation?
 *  - following text is output to 0xef'8025 at startup:
 *      CMC ENP/10 CMOS - 112708 Bytes Free
 *      Ethernet Address: 02CF1F123456
 *      Allocating 30 receive buffers
 *      Allocating 30 transmit buffers
 */

#include "emu.h"
#include "enp10.h"

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(VME_ENP10, vme_enp10_card_device, "enp10", "CMC ENP-10")

vme_enp10_card_device::vme_enp10_card_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, VME_ENP10, tag, owner, clock)
	, device_vme_card_interface(mconfig, *this)
	, m_cpu(*this, "cpu")
	, m_net(*this, "net")
	, m_led(*this, "led%u", 0U)
	, m_base(*this, "BASE")
	, m_boot(*this, "boot")
{
}

ROM_START(enp10)
	ROM_REGION16_BE(0x4000, "eprom", 0)

	ROM_SYSTEM_BIOS(0, "enp10_0", "CMC ENP/10 CMOS")
	ROMX_LOAD("link_10_2.0_nh_rev.4.1h.u4",  0x0000, 0x2000, CRC(7532f2b1) SHA1(bdef6c525f451fbc67f3d4625c9db18975e7e1e4), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("link_10_2.0_nh_rev.k4.1l.u3", 0x0001, 0x2000, CRC(f2decb78) SHA1(795623274bfff6273790c30445e4dca4064859ed), ROM_SKIP(1) | ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "enp10_1", "CMC ENP/10 CMOS (SGI?)")
	ROMX_LOAD("8845__070_0132_002s.u4", 0x0000, 0x2000, CRC(3ea05f63) SHA1(ee523928d27b854cd1be7e6aa2b8bb093d240022), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("8845__070_0131_002s.u3", 0x0001, 0x2000, CRC(d4439fb9) SHA1(51466000b613ab5c03b2bf933e1a485fe2e53d04), ROM_SKIP(1) | ROM_BIOS(1))

	// hand-crafted prom containing address 02:cf:1f:12:34:56
	ROM_REGION16_BE(0x20, "mac", 0)
	ROM_LOAD("mac.bin", 0x00, 0x20, CRC(99ac9577) SHA1(b4d6bba88dd376cc492738d57742628f42e9265e))
ROM_END

static INPUT_PORTS_START(enp10)
	PORT_START("BASE")
	PORT_CONFNAME(0xff, 0xde, "Base Address")
	PORT_CONFSETTING(0xd8, "0xd80000")
	PORT_CONFSETTING(0xda, "0xda0000")
	PORT_CONFSETTING(0xdc, "0xdc0000")
	PORT_CONFSETTING(0xde, "0xde0000")
INPUT_PORTS_END

const tiny_rom_entry *vme_enp10_card_device::device_rom_region() const
{
	return ROM_NAME(enp10);
}

ioport_constructor vme_enp10_card_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(enp10);
}

void vme_enp10_card_device::device_start()
{
	m_led.resolve();

	save_item(NAME(m_ivr));
	save_item(NAME(m_csr));
	save_item(NAME(m_ier));
	save_item(NAME(m_tir));
	save_item(NAME(m_rir));
	save_item(NAME(m_uir));
	save_item(NAME(m_rer));
	save_item(NAME(m_exr));
	save_item(NAME(m_hir));
}

void vme_enp10_card_device::device_reset()
{
	m_boot.select(0);

	m_ivr = 0;
	m_csr = 0;
	m_ier = 0;
	m_tir = 0;
	m_rir = 0;
	m_uir = 0;
	m_rer = 0;
	m_exr = 0;
	m_hir = 0;

	u32 const base = m_base->read() << 16;

	vme_space(vme::AM_39).install_device(base, base | 0x1'ffff, *this, &vme_enp10_card_device::vme_map);
	vme_space(vme::AM_3d).install_device(base, base | 0x1'ffff, *this, &vme_enp10_card_device::vme_map);

	vme_irq_w<4>(1);
}

void vme_enp10_card_device::device_add_mconfig(machine_config &config)
{
	M68000(config, m_cpu, 20_MHz_XTAL / 2);
	m_cpu->set_addrmap(AS_PROGRAM, &vme_enp10_card_device::cpu_map);

	AM7990(config, m_net, 20_MHz_XTAL / 2);
	m_net->intr_out().set_inputline(m_cpu, INPUT_LINE_IRQ6).invert();
	m_net->dma_in().set([this](offs_t offset) { return m_cpu->space(0).read_word(offset); });
	m_net->dma_out().set([this](offs_t offset, u16 data, u16 mem_mask) { m_cpu->space(0).write_word(offset, data, mem_mask); });

	vme_iack().set(FUNC(vme_enp10_card_device::iack_r));
}

void vme_enp10_card_device::cpu_map(address_map &map)
{
	map(0xf0'0000, 0xf1'ffff).ram().share("ram");
	map(0xf8'0000, 0xf8'3fff).rom().region("eprom", 0).mirror(0x02'0000);

	map(0x00'0000, 0xf1'ffff).view(m_boot);
	// map first 1k of eprom at 0x00'0000
	m_boot[0](0x00'0000, 0x00'0fff).rom().region("eprom", 0);

	// map first 1k of ram at 0x00'0000, unmap at 0xf0'0000
	m_boot[1](0x00'0000, 0x01'ffff).ram().share("ram");
	m_boot[1](0x00'1000, 0x01'ffff).unmaprw();
	m_boot[1](0xf0'0000, 0xf0'0fff).unmaprw();

	// uart: 16 byte registers 10-2f?
	map(0xef'8010, 0xef'802f).noprw();

	map(0xfe'0080, 0xfe'0081).umask16(0x00ff).rw(FUNC(vme_enp10_card_device::addr_r), FUNC(vme_enp10_card_device::irq_w));

	map(0xfe'00a0, 0xfe'00a1).umask16(0x00ff).lrw8(
		[this]() { return m_csr; }, "csr_r",
		[this](u8 data)
		{
			LOG("csr_w 0x%02x\n", data);

			m_led[0] = BIT(data, 6); // fail?
			m_led[1] = BIT(data, 5); // run?

			m_csr = data;
		}, "csr_w");

	map(0xfe'00c0, 0xfe'00c1).umask16(0x00ff).lrw8(NAME([this]() { return m_ier; }), NAME([this](u8 data) { m_ier = data; interrupt(); }));
	map(0xfe'00c2, 0xfe'00c3).umask16(0x00ff).lrw8(NAME([this]() { return m_tir; }), NAME([this](u8 data) { m_tir = data; interrupt(); }));
	map(0xfe'00c4, 0xfe'00c5).umask16(0x00ff).lrw8(NAME([this]() { return m_rir; }), NAME([this](u8 data) { m_rir = data; interrupt(); }));
	map(0xfe'00c6, 0xfe'00c7).umask16(0x00ff).lrw8(NAME([this]() { return m_uir; }), NAME([this](u8 data) { m_uir = data; interrupt(); }));
	map(0xfe'00ce, 0xfe'00cf).umask16(0x00ff).lrw8(NAME([this]() { return m_rer; }), NAME([this](u8 data) { m_rer = data; m_boot.select(BIT(data, 7)); }));

	map(0xfe'00e0, 0xfe'00e1).umask16(0x00ff).lrw8(NAME([this]() { return m_exr; }), NAME([this](u8 data) { m_exr = data; })); // TODO: bit 1 parity error?
	map(0xfe'00ee, 0xfe'00ef).umask16(0x00ff).lr8(NAME([]() { return 0; })); // TODO: bit 1 enables additional ram test?

	map(0xfe'0200, 0xfe'0203).rw(m_net, FUNC(am7990_device::regs_r), FUNC(am7990_device::regs_w));
	map(0xfe'0400, 0xfe'041f).rom().region("mac", 0);

	// TODO: verify the next two registers
	map(0xfe'0e00, 0xfe'0e01).umask16(0x00ff).lrw8(NAME([this]() { return m_hir; }), NAME([this](u8 data) { m_hir = data; interrupt(); }));
	map(0xfe'0f00, 0xfe'0fff).umask16(0x00ff).lrw8(NAME([this]() { reset(); return 0; }), NAME([this](u8 data) { reset(); }));
}

void vme_enp10_card_device::vme_map(address_map &map)
{
	map(0x0'0000, 0x1'efff).lrw16(
		[this](offs_t offset, u16 mem_mask) { return m_cpu->space(0).read_word(0xf0'0000 | (offset << 1), mem_mask); }, "mem_r",
		[this](offs_t offset, u16 data, u16 mem_mask) { m_cpu->space(0).write_word(0xf0'0000 | (offset << 1), data, mem_mask); }, "mem_w");

	map(0x1'f000, 0x1'ffff).lrw16(
		[this](offs_t offset, u16 mem_mask) { return m_cpu->space(0).read_word(0xfe'0000 | (offset << 1), mem_mask); }, "reg_r",
		[this](offs_t offset, u16 data, u16 mem_mask) { m_cpu->space(0).write_word(0xfe'0000 | (offset << 1), mem_mask); }, "reg_w");
}

u8 vme_enp10_card_device::addr_r()
{
	LOG("addr_r (%s)\n", machine().describe_context());

	// TODO: what is returned here?
	return m_base->read();
}

void vme_enp10_card_device::irq_w(u8 data)
{
	LOG("irq 0x%02x (%s)\n", data, machine().describe_context());

	m_ivr = data;

	vme_irq_w<4>(0);
}

u8 vme_enp10_card_device::iack_r()
{
	vme_irq_w<4>(1);

	return m_ivr;
}

void vme_enp10_card_device::interrupt()
{
	bool const enable = BIT(m_ier, 7);

	m_cpu->set_input_line(INPUT_LINE_IRQ2, enable && BIT(m_uir, 7));
	m_cpu->set_input_line(INPUT_LINE_IRQ3, enable && BIT(m_hir, 7));
	m_cpu->set_input_line(INPUT_LINE_IRQ4, enable && BIT(m_tir, 7));
	m_cpu->set_input_line(INPUT_LINE_IRQ5, enable && BIT(m_rir, 7));
}
