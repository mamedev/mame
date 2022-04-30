// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Siemens S26361-D279 SERAD serial interface board.
 *
 * This Multibus card was used with several Multibus-based computers from
 * Siemens, including the PC-MX2, MX300 and MX500. It has 3 SCN2681 DUARTs,
 * giving a total of two V24 and four SS97 serial ports. These are controlled
 * by an on-board 8085A microcontroller, with a dual-port RAM mailbox used to
 * communicate with the host.
 *
 * Sources:
 *  - https://oldcomputers-ddns.org/public/pub/rechner/siemens/mx-rm/pc-mx2/manuals/pc-mx2_pc2000_9780_logik.pdf
 *  - https://mx300i.narten.de/view_board.cfm?5EF287A1ABC3F4DCAFEA2BC2FAB8C504041A
 *
 * TODO:
 *  - multibus lock/unlock/interrupt
 *  - SS97 ports with power on/reset signals
 */

#include "emu.h"
#include "serad.h"

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(SERAD, serad_device, "serad", "Siemens S26361-D279 SERAD")

serad_device::serad_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SERAD, tag, owner, clock)
	, device_multibus_interface(mconfig, *this)
	, m_cpu(*this, "cpu")
	, m_mbx(*this, "mbx")
	, m_rst65(*this, "rst65")
	, m_duart(*this, "duart%u", 0U)
	, m_port(*this, "port%u", 0U)
	, m_installed(false)
{
}

ROM_START(serad)
	ROM_REGION(0x2000, "cpu", 0)
	ROM_LOAD("361d0279d031__e00422_tex.d31", 0x0000, 0x2000, CRC(369f5fd1) SHA1(6a1c2d351d5552d54d835b7726b3f9b921605d0e))
ROM_END

static INPUT_PORTS_START(serad)
	PORT_START("MEM")
	PORT_DIPNAME(0xfff, 0xef7, "Base Address")
	PORT_DIPSETTING(0xef7, "EF7000")
	PORT_DIPSETTING(0xef6, "EF6000")
	PORT_DIPSETTING(0xef5, "EF5000")
	PORT_DIPSETTING(0xef4, "EF4000")
	PORT_DIPSETTING(0xef3, "EF3000")

	PORT_START("PIO")
	PORT_DIPNAME(0xff, 0x10, "I/O Address")
	PORT_DIPSETTING(0x0f, "0F00")
	PORT_DIPSETTING(0x10, "1000")
	PORT_DIPSETTING(0x11, "1100")
	PORT_DIPSETTING(0x12, "1200")
	PORT_DIPSETTING(0x13, "1300")

	PORT_START("IRQ")
	PORT_DIPNAME(0xff, 0x00, "Interrupt")
	PORT_DIPSETTING(0x00, "None")
	PORT_DIPSETTING(0x01, "0")
	PORT_DIPSETTING(0x02, "1")
	PORT_DIPSETTING(0x04, "2")
	PORT_DIPSETTING(0x08, "3")
	PORT_DIPSETTING(0x10, "4")
	PORT_DIPSETTING(0x20, "5")
	PORT_DIPSETTING(0x40, "6")
	PORT_DIPSETTING(0x80, "7")
INPUT_PORTS_END

const tiny_rom_entry *serad_device::device_rom_region() const
{
	return ROM_NAME(serad);
}

ioport_constructor serad_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(serad);
}

void serad_device::device_start()
{
}

void serad_device::device_reset()
{
	if (!m_installed)
	{
		u32 const mem = ioport("MEM")->read() << 12;
		u32 const pio = ioport("PIO")->read() << 8;

		m_bus->space(AS_PROGRAM).install_ram(mem, mem | 0xfff, m_mbx.target());
		m_bus->space(AS_IO).install_write_handler(pio, pio | 0xff, write8smo_delegate(*this, &serad_device::rst55_w<1>, "serad_device::rst55_w"));

		m_installed = true;
	}
}

void serad_device::device_add_mconfig(machine_config &config)
{
	I8085A(config, m_cpu, 20_MHz_XTAL / 2); // P8085AH-2
	m_cpu->set_addrmap(AS_PROGRAM, &serad_device::mem_map);
	m_cpu->set_addrmap(AS_IO, &serad_device::pio_map);

	INPUT_MERGER_ANY_HIGH(config, m_rst65);
	m_rst65->output_handler().set_inputline(m_cpu, I8085_RST65_LINE);

	SCN2681(config, m_duart[0], 7.3728_MHz_XTAL / 2);
	SCN2681(config, m_duart[1], 7.3728_MHz_XTAL / 2);
	SCN2681(config, m_duart[2], 7.3728_MHz_XTAL / 2);

	m_duart[0]->irq_cb().set(m_rst65, FUNC(input_merger_any_high_device::in_w<0>));
	m_duart[1]->irq_cb().set(m_rst65, FUNC(input_merger_any_high_device::in_w<1>));
	m_duart[2]->irq_cb().set(m_rst65, FUNC(input_merger_any_high_device::in_w<2>));

	RS232_PORT(config, m_port[0], default_rs232_devices, nullptr); // SS97
	RS232_PORT(config, m_port[1], default_rs232_devices, nullptr); // SS97
	RS232_PORT(config, m_port[2], default_rs232_devices, nullptr); // V24
	RS232_PORT(config, m_port[3], default_rs232_devices, nullptr); // SS97
	RS232_PORT(config, m_port[4], default_rs232_devices, nullptr); // SS97
	RS232_PORT(config, m_port[5], default_rs232_devices, nullptr); // V24

	m_duart[0]->a_tx_cb().set(m_port[0], FUNC(rs232_port_device::write_txd));
	m_duart[0]->b_tx_cb().set(m_port[1], FUNC(rs232_port_device::write_txd));
	m_duart[0]->outport_cb().set(
		[this](u8 data)
		{
			// clear 0
			// clear 1
			// -
			// -
			m_duart[1]->ip0_w(BIT(data, 4)); // rxrdy0
			m_duart[1]->ip1_w(BIT(data, 5)); // rxrdy1
			m_duart[0]->ip0_w(BIT(data, 6)); // txrdy0
			m_duart[0]->ip1_w(BIT(data, 7)); // txrdy1
		}
	);

	m_duart[1]->a_tx_cb().set(m_port[2], FUNC(rs232_port_device::write_txd));
	m_duart[1]->b_tx_cb().set(m_port[3], FUNC(rs232_port_device::write_txd));
	m_duart[1]->outport_cb().set(
		[this](u8 data)
		{
			m_port[2]->write_rts(BIT(data, 0)); // s2.k2 (rts)
			// clear 3
			// -
			m_port[2]->write_dtr(BIT(data, 3)); // s1.k2 (dtr)
			m_duart[1]->ip2_w(BIT(data, 4)); // rxrdy2
			m_duart[1]->ip3_w(BIT(data, 5)); // rxrdy3
			m_duart[0]->ip2_w(BIT(data, 6)); // txrdy2
			m_duart[0]->ip3_w(BIT(data, 7)); // txrdy3
		}
	);

	m_duart[2]->a_tx_cb().set(m_port[4], FUNC(rs232_port_device::write_txd));
	m_duart[2]->b_tx_cb().set(m_port[5], FUNC(rs232_port_device::write_txd));
	m_duart[2]->outport_cb().set(
		[this](u8 data)
		{
			// clear 4
			m_port[5]->write_rts(BIT(data, 1)); // s2.k5 (rts)
			// resled
			m_port[5]->write_dtr(BIT(data, 3)); // s1.k5 (dtr)
			m_duart[1]->ip4_w(BIT(data, 4)); // rxrdy4
			m_duart[1]->ip5_w(BIT(data, 5)); // rxrdy5
			m_duart[0]->ip4_w(BIT(data, 6)); // txrdy4
			m_duart[0]->ip5_w(BIT(data, 7)); // txrdy5
		}
	);

	m_port[0]->rxd_handler().set(m_duart[0], FUNC(scn2681_device::rx_a_w));
	m_port[1]->rxd_handler().set(m_duart[0], FUNC(scn2681_device::rx_b_w));
	m_port[2]->rxd_handler().set(m_duart[1], FUNC(scn2681_device::rx_a_w));
	m_port[2]->cts_handler().set(m_duart[2], FUNC(scn2681_device::ip0_w));
	m_port[3]->rxd_handler().set(m_duart[1], FUNC(scn2681_device::rx_b_w));
	m_port[4]->rxd_handler().set(m_duart[2], FUNC(scn2681_device::rx_a_w));
	m_port[5]->rxd_handler().set(m_duart[2], FUNC(scn2681_device::rx_b_w));
	m_port[5]->cts_handler().set(m_duart[2], FUNC(scn2681_device::ip1_w));
}

void serad_device::mem_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("cpu", 0).mirror(0x2000);
	map(0x4000, 0x4fff).ram().share("mbx").mirror(0x3000);
	map(0x8000, 0x87ff).ram();
}

void serad_device::pio_map(address_map &map)
{
	map(0x00, 0x0f).rw(m_duart[0], FUNC(scn2681_device::read), FUNC(scn2681_device::write)); // duart 0
	map(0x10, 0x1f).rw(m_duart[1], FUNC(scn2681_device::read), FUNC(scn2681_device::write)); // duart 2
	map(0x20, 0x2f).rw(m_duart[2], FUNC(scn2681_device::read), FUNC(scn2681_device::write)); // duart 4
	//map(0x30, 0x3f); // TODO: multibus interrupt
	map(0x40, 0x4f).nopw(); // TODO: multibus lock
	map(0x50, 0x5f).nopw(); // TODO: multibus unlock
	map(0x60, 0x6f).w(FUNC(serad_device::rst55_w<0>));
	//map(0x70, 0x7f); // TODO: power
}
