// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Communication Machinery Corporation Ethernet Node Processor (ENP-10)
 *
 * Rebadged/resold by Motorola as the MVME330 Ethernet Controller, and also
 * by Silicon Graphics as part 013-0204-00[123].
 *
 * Firmware expects to find a UART at address 0xef'8010-0xef'802f, which was
 * apparently provided by a Mizar VME8300 card.
 *
 * Sources:
 *  - Ethernet Node Processor ENP-30 Reference Guide (6213000-05B), Communication Machinery Corporation, November 15, 1988
 *  - MVME330 Ethernet Controller User's Manual (MVME330/D2), Motorola, Second Edition, 1988
 *
 * TODO:
 *  - remaining control register flags
 *  - configurable interrupts
 *  - MVME330 -1 and -2 variants
 */

#include "emu.h"
#include "enp10.h"

//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(VME_ENP10, vme_enp10_card_device, "enp10", "CMC ENP-10")

enum csr_mask : u8
{
	CSR_SUSPEND = 0x08, // set by lance dma
	CSR_TIMER   = 0x10, // set by 2ms timer
	CSR_RESET   = 0x20, // assert vme reset
	CSR_FAIL    = 0x40,
	CSR_VIRQ    = 0x80,
};
enum obr_mask : u8
{
	OBR_IE = 0x01, // interrupt enable
	OBR_TI = 0x02, // transmit interrupt
	OBR_RI = 0x04, // receive interrupt
	OBR_UI = 0x08, // utility interrupt
	OBR_RE = 0x80, // RAM enable
};
enum exr_mask : u8
{
	EXR_RTO  = 0x01, // resource time-out
	EXR_PER  = 0x02, // parity error
	EXR_ABO  = 0x04, // abort
	EXR_ACLO = 0x08, // AC line voltage low
};

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
	ROM_REGION16_BE(0x10000, "eprom", 0)

	ROM_SYSTEM_BIOS(0, "enp10_0", "CMC ENP/10 CMOS")
	ROMX_LOAD("link_10_2.0_nh_rev.4.1h.u4",  0x0000, 0x2000, CRC(7532f2b1) SHA1(bdef6c525f451fbc67f3d4625c9db18975e7e1e4), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("link_10_2.0_nh_rev.k4.1l.u3", 0x0001, 0x2000, CRC(f2decb78) SHA1(795623274bfff6273790c30445e4dca4064859ed), ROM_SKIP(1) | ROM_BIOS(0))
	ROM_FILL(0x4000, 0xc000, 0xff)

	ROM_SYSTEM_BIOS(1, "enp10_1", "CMC ENP/10 CMOS (SGI?)")
	ROMX_LOAD("8845__070_0132_002s.u4", 0x0000, 0x2000, CRC(3ea05f63) SHA1(ee523928d27b854cd1be7e6aa2b8bb093d240022), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("8845__070_0131_002s.u3", 0x0001, 0x2000, CRC(d4439fb9) SHA1(51466000b613ab5c03b2bf933e1a485fe2e53d04), ROM_SKIP(1) | ROM_BIOS(1))
	ROM_FILL(0x4000, 0xc000, 0xff)

	ROM_SYSTEM_BIOS(2, "mvme330", "MVME330")
	ROMX_LOAD("knlrom10__v4.1-h.u4", 0x0000, 0x2000, CRC(b5f0a49b) SHA1(70e0d54c25a152503796fae8d7c5ffab6d625583), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("knlrom10__v4.1-l.u3", 0x0001, 0x2000, CRC(78d8ae1b) SHA1(c69a8fa2edec7d6faadb48591ce252ac45b55cad), ROM_SKIP(1) | ROM_BIOS(2))
	ROM_FILL(0x4000, 0xc000, 0xff)

	// this firmware requires 512KiB RAM
	ROM_SYSTEM_BIOS(3, "mvme330_1", "MVME330-1")
	ROMX_LOAD("u_rev-99592-616335-2.u4", 0x0000, 0x8000, CRC(88527c5e) SHA1(b953d99d5eb3462c41202f64f560fae592e269fa), ROM_SKIP(1) | ROM_BIOS(3))
	ROM_CONTINUE(0x0000, 0x8000) // first 32K is 0xff
	ROMX_LOAD("u_rev-99592-616335-1.u3", 0x0001, 0x8000, CRC(0d8e5aa4) SHA1(f4b165581c5840e0607bd979c297d3e5ee77cb0f), ROM_SKIP(1) | ROM_BIOS(3))
	ROM_CONTINUE(0x0001, 0x8000) // first 32K is 0xff

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
	save_item(NAME(m_obr));
	save_item(NAME(m_exr));

	save_item(NAME(m_bint));
	save_item(NAME(m_lint));
	save_item(NAME(m_int_state));

	m_bint = false;
	m_lint = false;
	m_int_state = 0;
}

void vme_enp10_card_device::device_reset()
{
	m_boot.select(0);

	m_ivr = 0;
	m_csr = CSR_FAIL | CSR_RESET;
	m_obr = 0;
	m_exr = 0;

	u32 const base = m_base->read() << 16;

	vme_space(vme::AM_39).install_device(base, base | 0x1'ffff, *this, &vme_enp10_card_device::vme_map);
	vme_space(vme::AM_3d).install_device(base, base | 0x1'ffff, *this, &vme_enp10_card_device::vme_map);

	vme_irq_w<4>(1);

	interrupt();
}

void vme_enp10_card_device::device_add_mconfig(machine_config &config)
{
	M68000(config, m_cpu, 20_MHz_XTAL / 2);
	m_cpu->set_addrmap(AS_PROGRAM, &vme_enp10_card_device::cpu_map);
	m_cpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &vme_enp10_card_device::cpu_ack);

	AM7990(config, m_net, 20_MHz_XTAL / 2);
	m_net->intr_out().set(
		[this](int state)
		{
			m_lint = !state;

			interrupt();
		});
	m_net->dma_in().set([this](offs_t offset) { return m_cpu->space(0).read_word(offset); });
	m_net->dma_out().set([this](offs_t offset, u16 data, u16 mem_mask) { m_cpu->space(0).write_word(offset, data, mem_mask); });

	TIMER(config, "timer").configure_periodic(FUNC(vme_enp10_card_device::timer), attotime::from_msec(2));

	vme_iack().set(FUNC(vme_enp10_card_device::iack_r));
	vme_berr().set(
		[this](int state)
		{
			m_exr |= EXR_RTO;

			m_cpu->trigger_bus_error();
		});
}

void vme_enp10_card_device::cpu_map(address_map &map)
{
	map(0x00'1000, 0xee'ffff).rw(FUNC(vme_enp10_card_device::vme_a24_r), FUNC(vme_enp10_card_device::vme_a24_w));
	map(0xef'0000, 0xef'ffff).rw(FUNC(vme_enp10_card_device::vme_a16_r), FUNC(vme_enp10_card_device::vme_a16_w));

	map(0xf0'0000, 0xf1'ffff).ram().share("ram");
	map(0xf8'0000, 0xf8'ffff).rom().region("eprom", 0).mirror(0x02'0000);

	map(0x00'0000, 0xf1'ffff).view(m_boot);
	// map first 4k of eprom at 0x00'0000
	m_boot[0](0x00'0000, 0x00'0fff).rom().region("eprom", 0);

	// map first 4k of ram at 0x00'0000, unmap at 0xf0'0000
	m_boot[1](0x00'0000, 0x01'ffff).ram().share("ram");
	m_boot[1](0x00'1000, 0x01'ffff).unmaprw();
	m_boot[1](0xf0'0000, 0xf0'0fff).unmaprw();

	map(0xfe'0080, 0xfe'0081).mirror(0x1e).umask16(0x00ff).rw(FUNC(vme_enp10_card_device::vect_r), FUNC(vme_enp10_card_device::vect_w));

	map(0xfe'00a0, 0xfe'00a1).mirror(0x1e).umask16(0x00ff).lrw8(
		[this]() { return m_csr; }, "csr_r",
		[this](u8 data)
		{
			LOG("csr_w 0x%02x\n", data);

			m_led[0] = BIT(data, 6); // fail
			m_led[1] = !BIT(data, 6); // run

			// TODO: CSR_RESET
			// TODO: CSR_SUSPEND
			m_csr = (m_csr & (CSR_VIRQ | CSR_TIMER)) | (data & (CSR_FAIL | CSR_RESET));
		}, "csr_w");

	map(0xfe'00c0, 0xfe'00cf).mirror(0x10).umask16(0x00ff).rw(FUNC(vme_enp10_card_device::obr_r), FUNC(vme_enp10_card_device::obr_w));

	map(0xfe'00e0, 0xfe'00ef).mirror(0x10).umask16(0x00ff).lrw8(
		NAME([this]() { return m_exr; }),
		NAME([this](u8 data)
		{
			if (m_exr)
				LOG("exr_w 0x%02x (%s)\n", data, machine().describe_context());

			m_exr = 0;
			m_csr &= ~CSR_TIMER;

			interrupt();
		}));

	map(0xfe'0200, 0xfe'0203).mirror(0x1fc).rw(m_net, FUNC(am7990_device::regs_r), FUNC(am7990_device::regs_w));
	map(0xfe'0400, 0xfe'041f).mirror(0x3e0).rom().region("mac", 0);
}

void vme_enp10_card_device::vme_map(address_map &map)
{
	map(0x0'0000, 0x1'fdff).lrw16(
		[this](offs_t offset, u16 mem_mask) { return m_cpu->space(0).read_word(0xf0'0000 | (offset << 1), mem_mask); }, "mem_r",
		[this](offs_t offset, u16 data, u16 mem_mask) { m_cpu->space(0).write_word(0xf0'0000 | (offset << 1), data, mem_mask); }, "mem_w");

	map(0x1'fe00, 0x1'feff).lw16(
		[this](u16 data)
		{
			LOG("host interrupt (%s)\n", machine().describe_context());

			m_bint = true;
			interrupt();
		}, "bint_w");

	map(0x1'ff00, 0x1'ffff).lw16(
		[this](u16 data)
		{
			LOG("host reset (%s)\n", machine().describe_context());
			reset();
		}, "reset_w");
}

u8 vme_enp10_card_device::vect_r()
{
	LOG("vect_r (%s)\n", machine().describe_context());

	return m_base->read();
}

void vme_enp10_card_device::vect_w(u8 data)
{
	LOG("vect_w 0x%02x (%s)\n", data, machine().describe_context());

	if (!(m_csr & CSR_VIRQ))
	{
		m_ivr = data;
		m_csr |= CSR_VIRQ;

		vme_irq_w<4>(0);
	}
}

u8 vme_enp10_card_device::iack_r()
{
	if (m_csr & CSR_VIRQ)
	{
		vme_irq_w<4>(1);
		m_csr &= ~CSR_VIRQ;
	}

	return m_ivr;
}

u8 vme_enp10_card_device::obr_r(offs_t offset)
{
	return (m_obr & (1U << offset)) ? 0x80 : 0;
}

void vme_enp10_card_device::obr_w(offs_t offset, u8 data)
{
	unsigned const state = BIT(data, 7);

	if (BIT(m_obr, offset) != state)
	{
		//static char const *const reg[] = { "ier", "tir", "rir", "uir", "bit4", "bit5", "bit6", "mapr" };
		//LOG("obr_w %s %u (%s)\n", reg[offset], state, machine().describe_context());

		if (state)
			m_obr |= 1U << offset;
		else
			m_obr &= ~(1U << offset);

		if (offset < 4)
			interrupt();
		else if (offset == 7)
			m_boot.select(state);
	}
}

void vme_enp10_card_device::timer(timer_device &timer, s32 param)
{
	m_csr |= CSR_TIMER;

	interrupt();
}

void vme_enp10_card_device::interrupt()
{
	u8 int_state = 0;

	if (m_obr & OBR_IE)
	{
		// find highest priority asserted interrupt source
		if ((m_csr & CSR_TIMER) || (m_exr & (EXR_ACLO | EXR_ABO | EXR_PER)))
			int_state = 7;
		else if (m_lint)
			int_state = 6;
		else if (m_obr & OBR_RI)
			int_state = 5;
		else if (m_obr & OBR_TI)
			int_state = 4;
		else if (m_bint)
			int_state = 3;
		else if (m_obr & OBR_UI)
			int_state = 2;
	}

	if (int_state != m_int_state)
	{
		// deassert old interrupt
		if (m_int_state)
			m_cpu->set_input_line(m_int_state, CLEAR_LINE);

		// assert new interrupt
		if (int_state)
			m_cpu->set_input_line(int_state, int_state == 3 ? HOLD_LINE : ASSERT_LINE);

		m_int_state = int_state;
	}
}

void vme_enp10_card_device::cpu_ack(address_map &map)
{
	map(0xff'fff3, 0xff'fff3).lr8(NAME([]() { return m68000_base_device::autovector(1); }));
	map(0xff'fff5, 0xff'fff5).lr8(NAME([]() { return m68000_base_device::autovector(2); }));
	map(0xff'fff7, 0xff'fff7).lr8(NAME([this]() { m_bint = false; return m68000_base_device::autovector(3); }));
	map(0xff'fff9, 0xff'fff9).lr8(NAME([]() { return m68000_base_device::autovector(4); }));
	map(0xff'fffb, 0xff'fffb).lr8(NAME([]() { return m68000_base_device::autovector(5); }));
	map(0xff'fffd, 0xff'fffd).lr8(NAME([]() { return m68000_base_device::autovector(6); }));
	map(0xff'ffff, 0xff'ffff).lr8(NAME([]() { return m68000_base_device::autovector(7); }));
}

u16 vme_enp10_card_device::vme_a16_r(offs_t offset, u16 mem_mask)
{
	if (!machine().side_effects_disabled())
	{
		if (m_cpu->supervisor_mode())
			return device_vme_card_interface::vme_read16<vme::AM_2d>(offset, mem_mask);
		else
			return device_vme_card_interface::vme_read16<vme::AM_29>(offset, mem_mask);
	}
	else
		return 0;
}

void vme_enp10_card_device::vme_a16_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (m_cpu->supervisor_mode())
		device_vme_card_interface::vme_write16<vme::AM_2d>(offset, data, mem_mask);
	else
		device_vme_card_interface::vme_write16<vme::AM_29>(offset, data, mem_mask);
}

u16 vme_enp10_card_device::vme_a24_r(offs_t offset, u16 mem_mask)
{
	if (!machine().side_effects_disabled())
	{
		switch (m_cpu->get_fc())
		{
		case 1: return device_vme_card_interface::vme_read16<vme::AM_39>(offset, mem_mask);
		case 2: return device_vme_card_interface::vme_read16<vme::AM_3a>(offset, mem_mask);
		case 5: return device_vme_card_interface::vme_read16<vme::AM_3d>(offset, mem_mask);
		case 6: return device_vme_card_interface::vme_read16<vme::AM_3e>(offset, mem_mask);
		default:
			fatalerror("enp10: unknown vme a24 space read (%s)\n", machine().describe_context());
		}
	}
	else
		return 0;
}

void vme_enp10_card_device::vme_a24_w(offs_t offset, u16 data, u16 mem_mask)
{
	switch (m_cpu->get_fc())
	{
	case 1: device_vme_card_interface::vme_write16<vme::AM_39>(offset, data, mem_mask); break;
	case 2: device_vme_card_interface::vme_write16<vme::AM_3a>(offset, data, mem_mask); break;
	case 5: device_vme_card_interface::vme_write16<vme::AM_3d>(offset, data, mem_mask); break;
	case 6: device_vme_card_interface::vme_write16<vme::AM_3e>(offset, data, mem_mask); break;
	default:
		fatalerror("enp10: unknown vme a24 space write (%s)\n", machine().describe_context());
	}
}
