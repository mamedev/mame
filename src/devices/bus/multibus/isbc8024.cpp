// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Intel iSBC 80/24 Single Board Computer
 *
 * Sources:
 *  - http://www.bitsavers.org/pdf/intel/iSBC/148437-001_iSBC_80_24A_Hardware_Reference_Manual_Nov85.pdf
 *
 * TODO:
 *  - configurable eprom quantity/size
 *  - configurable irq, clock and I/O
 *  - reverse engineer unknown firmware
 *  - led output
 */

#include "emu.h"
#include "isbc8024.h"

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(ISBC8024, isbc8024_device, "isbc8024", "Intel iSBC 80/24 Single Board Computer")

isbc8024_device::isbc8024_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ISBC8024, tag, owner, clock)
	, device_multibus_interface(mconfig, *this)
	, m_cpu(*this, "cpu")
	, m_pit(*this, "pit")
	, m_pci(*this, "pci")
	, m_ppi(*this, "ppi%u", 0U)
	, m_pic(*this, "pic")
	, m_j3(*this, "j3")
	, m_conf(*this, "CONF")
	, m_installed(false)
{
}

ROM_START(isbc8024)
	ROM_REGION(0x1000, "eprom", 0)
	ROM_SYSTEM_BIOS(0, "unknown", "unknown")
	ROMX_LOAD("unknown.u50", 0x000, 0x800, CRC(28b18721) SHA1(37526ed0a4f1e910fc0a8f6af0708c77a7358f69), ROM_BIOS(0))
	ROMX_LOAD("unknown.u51", 0x800, 0x800, CRC(9e49f6ad) SHA1(565ab4740d3151eb4d8ef9ff3e02570cf7804be1), ROM_BIOS(0))
ROM_END

static INPUT_PORTS_START(isbc8024)
	PORT_START("CONF")
	PORT_CONFNAME(0x0003, 0x0003, "Size")
	PORT_CONFSETTING(     0x0000, "None")
	PORT_CONFSETTING(     0x0001, "2048")
	PORT_CONFSETTING(     0x0002, "4096")
	PORT_CONFSETTING(     0x0003, "8192")

	PORT_CONFNAME(0xf000, 0xf000, "End")
	PORT_CONFSETTING(     0x3000, "3FFF")
	PORT_CONFSETTING(     0x7000, "7FFF")
	PORT_CONFSETTING(     0xb000, "BFFF")
	PORT_CONFSETTING(     0xf000, "FFFF")
INPUT_PORTS_END

const tiny_rom_entry *isbc8024_device::device_rom_region() const
{
	return ROM_NAME(isbc8024);
}

ioport_constructor isbc8024_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(isbc8024);
}

void isbc8024_device::device_start()
{
}

void isbc8024_device::device_reset()
{
	if (!m_installed)
	{
		ioport_value const conf = m_conf->read();
		if (conf & 3)
		{
			size_t const ram_size = 1024 << (conf & 3);
			offs_t ram_end = (conf & 0xf000) | 0x0fff;

			std::unique_ptr<u8[]> const ram = std::make_unique<u8[]>(ram_size);
			m_cpu->space(AS_PROGRAM).install_ram(ram_end - ram_size + 1, ram_end, ram.get());
		}

		// TODO: what's exposed to the Multibus?

		m_installed = true;
	}
}

void isbc8024_device::device_add_mconfig(machine_config &config)
{
	I8085A(config, m_cpu, 19'354'000 / 2);
	m_cpu->set_addrmap(AS_PROGRAM, &isbc8024_device::cpu_mem_map);
	m_cpu->set_addrmap(AS_IO, &isbc8024_device::cpu_pio_map);

	PIT8254(config, m_pit);
	m_pit->set_clk<0>(19'354'000 / 18);
	m_pit->set_clk<1>(19'354'000 / 18);
	m_pit->set_clk<2>(19'354'000 / 18);
	m_pit->out_handler<0>().set(m_pic, FUNC(pic8259_device::ir2_w));
	//m_pit->out_handler<1>().set(m_pic, FUNC(pic8259_device::ir?));
	m_pit->out_handler<2>().append(m_pci, FUNC(i8251_device::write_rxc));
	m_pit->out_handler<2>().append(m_pci, FUNC(i8251_device::write_txc));

	I8251(config, m_pci, 0);

	I8255(config, m_ppi[0], 0); // j1
	I8255(config, m_ppi[1], 0); // j2

	PIC8259(config, m_pic);
	m_pic->out_int_callback().set_inputline(m_cpu, I8085_INTR_LINE);
	int_callback<2>().set(m_pic, FUNC(pic8259_device::ir1_w));

	RS232_PORT(config, m_j3, default_rs232_devices, nullptr);

	m_pci->txd_handler().set(m_j3, FUNC(rs232_port_device::write_txd));
	m_pci->dtr_handler().set(m_j3, FUNC(rs232_port_device::write_dtr));
	m_pci->rts_handler().set(m_j3, FUNC(rs232_port_device::write_rts));
	m_j3->rxd_handler().set(m_pci, FUNC(i8251_device::write_rxd));
	m_j3->dsr_handler().set(m_pci, FUNC(i8251_device::write_dsr));
	m_j3->cts_handler().set(m_pci, FUNC(i8251_device::write_cts));
}

void isbc8024_device::cpu_mem_map(address_map &map)
{
	// default off-board mem access to multibus
	map(0x0000, 0xffff).rw(FUNC(isbc8024_device::bus_mem_r), FUNC(isbc8024_device::bus_mem_w));

	// TODO: variable number and size of EPROMs
	map(0x0000, 0x0fff).rom().region("eprom", 0);
}

void isbc8024_device::cpu_pio_map(address_map &map)
{
	// default off-board pio access to multibus
	map(0x00, 0xff).rw(FUNC(isbc8024_device::bus_pio_r), FUNC(isbc8024_device::bus_pio_w));

	map(0xd4, 0xd4).noprw(); // TODO: power fail
	//map(0xd5, 0xd5); // system bus override
	map(0xd6, 0xd6).lw8([this](u8 data) { popmessage("LED"); }, "led_w"); // led diagnostic indicator
	map(0xd7, 0xd7).noprw(); // reserved
	map(0xd8, 0xd9).rw(m_pic, FUNC(pic8259_device::read), FUNC(pic8259_device::write)).mirror(0x02);
	map(0xdc, 0xdf).rw(m_pit, FUNC(pit8254_device::read), FUNC(pit8254_device::write));
	map(0xe4, 0xe7).rw(m_ppi[0], FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xe8, 0xeb).rw(m_ppi[1], FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xec, 0xed).rw(m_pci, FUNC(i8251_device::read), FUNC(i8251_device::write)).mirror(0x02);
	//map(0xc0, 0xc7); // mutimodule 1 mcs0/ j5
	//map(0xc8, 0xcf); // mutimodule 1 mcs1/ j5
	//map(0xf0, 0xf7); // mutimodule 2 mcs0/ j6
	//map(0xf8, 0xff); // mutimodule 2 mcs1/ j6
}
