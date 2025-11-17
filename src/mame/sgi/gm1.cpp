// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Silicon Graphics GT Graphics (aka Clover 2) Graphics Manager 1 (GM1) board, part 030-0076-00[34].
 *
 * TODO:
 *  - everything (skeleton only)
 *
 * WIP:
 *  - launch with -vme:slot9 gm1 [-vme:slot9:gm1:serial0 terminal]
 *  - skeleton is sufficient to pass gm1 diagnostics, but nothing else
 */

#include "emu.h"
#include "gm1.h"

//#define VERBOSE (LOG_GENERAL)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(SGI_GM1, sgi_gm1_device, "sgi_gm1", "SGI GM1")

sgi_gm1_device::sgi_gm1_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SGI_GM1, tag, owner, clock)
	, device_vme_card_interface(mconfig, *this)
	, m_cpu(*this, "cpu")
	, m_duart(*this, "duart")
	, m_serial(*this, "serial%u", 0U)
{
}

void sgi_gm1_device::cpu_map(address_map &map)
{
	map(0x0000'0000, 0x0000'ffff).rom().region("eprom", 0);

	map(0x0200'0000, 0x0200'000f).rw(m_duart, FUNC(scn2681_device::read), FUNC(scn2681_device::write));

	map(0x9800'0000, 0x980f'ffff).ram().share("gm_dram"); // 1M?

	// polygon processor (pp) interface?
	map(0xc800'2000, 0xc800'3fff).ram().share("pp_sram"); // 8K
	map(0xc800'4000, 0xc800'4003).umask32(0xffff'0000).lr16([this]() { return m_pp_wc; }, "pp_wc_r");

	map(0xcc00'0000, 0xcc00'0000).umask32(0xff00'0000).lrw8(
		[]() { return 0x02; }, "pp_sts_r",
		[this](u8 data) { LOG("pp_cmd_w 0x%02x (%s)\n", data, machine().describe_context()); }, "pp_cmd_w");
	map(0xcc00'0010, 0xcc00'0013).umask32(0xffff'0000).lw16([this](u16 data) { m_pp_wc = data; }, "pp_wc_w");

	map(0xce00'0000, 0xce00'ffff).ram().share("pp_ucode"); // 64K?
}

void sgi_gm1_device::vme_a16_map(address_map &map)
{
	map(0x0, 0x3).umask32(0x0000'ffff).rw(FUNC(sgi_gm1_device::status_r), FUNC(sgi_gm1_device::reset_w));
	map(0x4, 0x7).umask32(0x0000'ffff).w(FUNC(sgi_gm1_device::interrupt_w));
	map(0x8, 0xb).umask32(0x0000'ffff).w(FUNC(sgi_gm1_device::interrupt_disable_w));
	map(0xc, 0xf).umask32(0x0000'ffff).w(FUNC(sgi_gm1_device::interrupt_vector_w));
}
void sgi_gm1_device::vme_a32_map(address_map &map)
{
	map(0x0000'0000, 0x000f'ffff).ram().share("gm_dram");
	// TODO: pipe
}

u16 sgi_gm1_device::status_r() { return 0x0700; }
void sgi_gm1_device::reset_w(u16 data) { LOG("reset_w 0x%04x (%s)\n", data, machine().describe_context()); }
void sgi_gm1_device::interrupt_w(u16 data) { LOG("interrupt_w 0x%04x (%s)\n", data, machine().describe_context()); }
void sgi_gm1_device::interrupt_disable_w(u16 data) { LOG("interrupt_disable_w 0x%04x (%s)\n", data, machine().describe_context()); }
void sgi_gm1_device::interrupt_vector_w(u16 data) { LOG("interrupt_vector_w 0x%04x (%s)\n", data, machine().describe_context()); }

void sgi_gm1_device::device_add_mconfig(machine_config &config)
{
	/*
	 * irq  source
	 *  1   unused
	 *  2   pp
	 *  3   unused
	 *  4   fifo
	 *  5   retrace
	 *  6   host
	 *  7   uart
	*/
	M68020(config, m_cpu, 16_MHz_XTAL);
	m_cpu->set_addrmap(AS_PROGRAM, &sgi_gm1_device::cpu_map);

	SCN2681(config, m_duart, 3.6864_MHz_XTAL); // SCN2681AC1N24

	RS232_PORT(config, m_serial[0], default_rs232_devices, nullptr);
	RS232_PORT(config, m_serial[1], default_rs232_devices, nullptr);

	m_duart->irq_cb().set_inputline(m_cpu, INPUT_LINE_IRQ7);
	m_duart->a_tx_cb().set(m_serial[0], FUNC(rs232_port_device::write_txd));
	m_duart->b_tx_cb().set(m_serial[1], FUNC(rs232_port_device::write_txd));

	m_serial[0]->rxd_handler().set(m_duart, FUNC(scn2681_device::rx_a_w));
	m_serial[1]->rxd_handler().set(m_duart, FUNC(scn2681_device::rx_b_w));
}

void sgi_gm1_device::device_start()
{
	m_pp_wc = 0;
}

void sgi_gm1_device::device_reset()
{
	/*
	 * #define GM_VME_BASE_ADDR     0xbd002000  // in VME A16 space
	 * #define GM_VME_DRAM_ADDR     0xb8800000  // in VME A32 space
	 * #define GM_VME_PIPE_ADDR     0xb8900000  // in VME A32 space
	 */
	vme_space(vme::AM_2d).install_device(0x2000, 0x200f, *this, &sgi_gm1_device::vme_a16_map);
	vme_space(vme::AM_09).install_device(0x1880'0000, 0x189f'ffff, *this, &sgi_gm1_device::vme_a32_map);

	// FIXME: interrupts asserted at power-on?
	m_cpu->set_input_line(INPUT_LINE_IRQ2, HOLD_LINE);
	m_cpu->set_input_line(INPUT_LINE_IRQ5, HOLD_LINE);
}

ROM_START(gm1)
	ROM_REGION32_BE(0x10000, "eprom", 0)

	// "GM-1 Prom revision Thu Mar 16 18:52:42 PST 1989"
	ROM_LOAD("070-0253-005.bin", 0x00000, 0x10000, CRC(a5b883b8) SHA1(ad22a07d0a57b012e708b38216b7f3e600e12597))
ROM_END

static INPUT_PORTS_START(gm1)
INPUT_PORTS_END

tiny_rom_entry const *sgi_gm1_device::device_rom_region() const
{
	return ROM_NAME(gm1);
}

ioport_constructor sgi_gm1_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(gm1);
}
