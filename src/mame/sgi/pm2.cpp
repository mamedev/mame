// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * SGI IRIS PM2 processor board
 *
 * Sources:
 *  -
 *
 * TODO:
 *  - mouse
 *  - memory parity
 *  - serial flow control
 *  - parallel port
 *  - led
 */

#include "emu.h"

#include "pm2.h"
#include "pm2_mmu.h"
#include "iris_kbd.h"

#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "machine/input_merger.h"
#include "machine/mc68681.h"

#include "emupal.h"

//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

namespace {

enum status_mask : u16
{
	STATUS_LED     = 0x000f,
	STATUS_MBOX    = 0x0010, // mailbox interrupt enable
	STATUS_PARITY  = 0x0020, // parity enable
	STATUS_MBINIT  = 0x0040, // multibus init
	STATUS_NOTBOOT = 0x0080, // disable boot state
	STATUS_EN0     = 0x0100, // 0=enable external multibus memory access
	STATUS_EN1     = 0x0200, // 0=enable external multibus memory write access
	STATUS_GEUSE   = 0x0400, // allow user mode GE access
	STATUS_PPUSE   = 0x0800, // allow user mode parallel port access
};
enum exception_mask : u16
{
	EXCEPTION_PRESENT = 0x0001, // 0=page fault
	EXCEPTION_MAPERR  = 0x0002, // 0=error
	EXCEPTION_TIMEOUT = 0x0004, // 0=error
	EXCEPTION_PARERR  = 0x0008, // 0=parity error
	EXCEPTION_MBINT   = 0x0010, // mouse button interrupt
	EXCEPTION_MBOX    = 0x0020, // mailbox interrupt
	EXCEPTION_P0INT   = 0x0040, // parallel port receive interrupt
	EXCEPTION_P1INT   = 0x0080, // parallel port transmit interrupt

	EXCEPTION_L4      = 0x00f0,
};

class sgi_pm2_device
	: public device_t
	, public device_multibus_interface
{
public:
	sgi_pm2_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: device_t(mconfig, SGI_PM2, tag, owner, clock)
		, device_multibus_interface(mconfig, *this)
		, m_cpu(*this, "cpu")
		, m_mmu(*this, "mmu")
		, m_duart(*this, "duart%u", 0U)
		, m_port(*this, "port%u", 1U)
		, m_ram(*this, "ram")
		, m_config(*this, "CONFIG")
		, m_led(*this, "led")
		, m_map{}
		, m_installed(false)
	{
	}

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_config_complete() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void mem_map(address_map &map) ATTR_COLD;

	u16 status_r();
	void status_w(u16 data);

	template <u16 E> void irq4_w(int state);

	// Multibus access helpers
	offs_t map(offs_t offset) const;
	u16 mem_r(offs_t offset, u16 mem_mask);
	void mem_w(offs_t offset, u16 data, u16 mem_mask);
	u16 map_r(offs_t offset, u16 mem_mask);
	void map_w(offs_t offset, u16 data, u16 mem_mask);

private:
	required_device<m68000_device> m_cpu;
	required_device<pm2_mmu_device> m_mmu;
	required_device_array<mc68681_device, 2> m_duart;
	required_device_array<rs232_port_device, 4> m_port;
	required_shared_ptr<u16> m_ram;

	required_ioport m_config;
	output_finder<> m_led;

	u16 m_status;
	u16 m_exception;
	u16 m_map[240]; // AM2148-55DC 1Kx4 SRAM (x3)?

	bool m_installed;
};

void sgi_pm2_device::device_start()
{
	save_item(NAME(m_status));
	save_item(NAME(m_exception));
	save_item(NAME(m_map));

	m_led.resolve();
}

void sgi_pm2_device::device_reset()
{
	if (!m_installed)
	{
		// TODO: configuration switches, slave mode
		m_bus->space(AS_PROGRAM).install_readwrite_handler(0x00'0000, 0x0e'ffff,
			emu::rw_delegate(*this, FUNC(sgi_pm2_device::mem_r)),
			emu::rw_delegate(*this, FUNC(sgi_pm2_device::mem_w)));

		m_bus->space(AS_PROGRAM).install_readwrite_handler(0x10'0000, 0x1e'ffff,
			emu::rw_delegate(*this, FUNC(sgi_pm2_device::map_r)),
			emu::rw_delegate(*this, FUNC(sgi_pm2_device::map_w)));

		m_installed = true;
	}

	m_status = STATUS_EN0 | STATUS_EN1;
	m_exception = 0x0f;
}

void keyboard_devices(device_slot_interface &device)
{
	device.option_add("kbd", IRIS_KBD);
}

void sgi_pm2_device::device_add_mconfig(machine_config &config)
{
	// TODO: replace 68000 with 68010 when MMU-capable
	M68000(config, m_cpu, 20_MHz_XTAL / 2); // MC68010L10
	m_cpu->set_addrmap(AS_PROGRAM, &sgi_pm2_device::mem_map);

	SGI_PM2_MMU(config, m_mmu, 0);
	m_mmu->error().set([this](int state) { m_cpu->trigger_bus_error(); });
	m_cpu->set_current_mmu(m_mmu);

	// Multibus interrupts
	int_callback<1>().set_inputline(m_cpu, INPUT_LINE_IRQ1).invert();
	int_callback<2>().set_inputline(m_cpu, INPUT_LINE_IRQ2).invert();
	int_callback<5>().set_inputline(m_cpu, INPUT_LINE_IRQ5).invert();

	input_merger_any_high_device &irq6(INPUT_MERGER_ANY_HIGH(config, "irq6"));
	irq6.output_handler().set_inputline(m_cpu, INPUT_LINE_IRQ6);

	/*
	 * UART0: refresh timer (15us)
	 * UART1: system clock
	 *
	 * IP
	 *  0x04 DTRA
	 *  0x08 DTRB
	 *
	 * OP
	 *  0x10 DSRA
	 *  0x20 DSRB
	 *  0x01 DTRA
	 *  0x02 DTRB
	 */
	MC68681(config, m_duart[0], 3.6864_MHz_XTAL).irq_cb().set(irq6, FUNC(input_merger_any_high_device::in_w<0>));
	MC68681(config, m_duart[1], 3.6864_MHz_XTAL).irq_cb().set(irq6, FUNC(input_merger_any_high_device::in_w<1>));

	RS232_PORT(config, m_port[0], keyboard_devices, "kbd");
	RS232_PORT(config, m_port[1], default_rs232_devices, nullptr);
	RS232_PORT(config, m_port[2], default_rs232_devices, nullptr);
	RS232_PORT(config, m_port[3], default_rs232_devices, nullptr);

	m_duart[0]->a_tx_cb().set(m_port[0], FUNC(rs232_port_device::write_txd));
	m_duart[0]->b_tx_cb().set(m_port[1], FUNC(rs232_port_device::write_txd));
	m_duart[1]->a_tx_cb().set(m_port[2], FUNC(rs232_port_device::write_txd));
	m_duart[1]->b_tx_cb().set(m_port[3], FUNC(rs232_port_device::write_txd));

	m_port[0]->rxd_handler().set(m_duart[0], FUNC(scn2681_device::rx_a_w));
	m_port[1]->rxd_handler().set(m_duart[0], FUNC(scn2681_device::rx_b_w));
	m_port[2]->rxd_handler().set(m_duart[1], FUNC(scn2681_device::rx_a_w));
	m_port[3]->rxd_handler().set(m_duart[1], FUNC(scn2681_device::rx_b_w));
}

void sgi_pm2_device::device_config_complete()
{
	m_mmu.lookup()->set_space<0>(m_bus, AS_PROGRAM);
	m_mmu.lookup()->set_space<1>(m_bus, AS_IO);
	m_mmu.lookup()->set_space<2>(m_cpu, AS_PROGRAM);
	m_mmu.lookup()->set_space<3>(m_cpu, m68000_device::AS_CPU_SPACE);
}

void sgi_pm2_device::mem_map(address_map &map)
{
	// TODO: was there a 2M PM2 variant?
	map(0x00'0000, 0x17'ffff).ram().share("ram"); // mt4264-15 64kx1 DRAM (8x9) + 1M on PM2M board

	map(0x18'0000, 0xf7'ffff).noprw(); // HACK: silence ram sizing

	map(0xf8'0000, 0xf8'7fff).rom().region("prom0", 0);
	map(0xf9'0000, 0xf9'7fff).rom().region("prom1", 0);
	// 0xfa'0000 prom2
	// 0xfb'0000 prom3 - DC4?

	map(0xfc'0000, 0xfc'1fff).rw(m_mmu, FUNC(pm2_mmu_device::page_r), FUNC(pm2_mmu_device::page_w));
	map(0xfc'2000, 0xfc'3fff).rw(m_mmu, FUNC(pm2_mmu_device::prot_r), FUNC(pm2_mmu_device::prot_w));
	map(0xfc'4000, 0xfc'401f).rw(m_duart[0], FUNC(scn2681_device::read), FUNC(scn2681_device::write)).umask16(0xff00);
	map(0xfc'6000, 0xfc'601f).rw(m_duart[1], FUNC(scn2681_device::read), FUNC(scn2681_device::write)).umask16(0xff00);
	map(0xfc'8000, 0xfc'8001).rw(m_mmu, FUNC(pm2_mmu_device::context_r), FUNC(pm2_mmu_device::context_w));

	map(0xfc'9000, 0xfc'9001).rw(FUNC(sgi_pm2_device::status_r), FUNC(sgi_pm2_device::status_w));
	map(0xfc'a000, 0xfc'a001).lr16([this]() { return m_exception; }, "exception_r");
	//0xfc'c000 // mouse
	map(0xfd'0000, 0xfd'0000).lr8([this]() { return m_config->read(); }, "config_r");
	//0xfd'4000 // ge token
	//0xfd'5000 // ge port
}

u16 sgi_pm2_device::status_r()
{
	return m_status;
}

void sgi_pm2_device::status_w(u16 data)
{
	m_mmu->boot_w(BIT(data, 7));

	if ((m_status & STATUS_MBOX) && !(data & STATUS_MBOX) && (m_exception & EXCEPTION_MBOX))
		irq4_w<EXCEPTION_MBOX>(0);

	m_led = BIT(data, 0, 4);

	m_status = data;
}

template <u16 E> void sgi_pm2_device::irq4_w(int state)
{
	// mouse buttons, mailbox and parallel ports share interrupt request level 4
	if (state)
		m_exception |= E;
	else
		m_exception &= ~E;

	m_cpu->set_input_line(INPUT_LINE_IRQ4, (m_exception & EXCEPTION_L4) ? ASSERT_LINE : CLEAR_LINE);
}

offs_t sgi_pm2_device::map(offs_t offset) const
{
	return u32(m_map[offset >> 11]) << 11 | (offset & 0x7ff);
}

u16 sgi_pm2_device::mem_r(offs_t offset, u16 mem_mask)
{
	u16 data = 0;

	if (!(m_status & STATUS_EN0))
	{
		offs_t const physical = map(offset);
		data = m_ram[physical];

		if (!machine().side_effects_disabled())
			LOG("%s: mem_r 0x%06x physical 0x%06x data 0x%04x\n", machine().describe_context(), offset << 1, physical << 1, data);

		if ((offset < 0x8000) && (m_status & STATUS_MBOX))
			irq4_w<EXCEPTION_MBOX>(1);
	}
	else
		LOG("%s: mem_r access disabled\n", machine().describe_context());

	return data;
}

void sgi_pm2_device::mem_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (!(m_status & (STATUS_EN0 | STATUS_EN1)))
	{
		offs_t const physical = map(offset);

		if (!machine().side_effects_disabled())
			LOG("%s: mem_w 0x%06x physical 0x%06x data 0x%04x\n", machine().describe_context(), offset << 1, physical << 1, data);

		COMBINE_DATA(&m_ram[physical]);

		if ((offset < 0x8000) && (m_status & STATUS_MBOX))
			irq4_w<EXCEPTION_MBOX>(1);
	}
	else
		LOG("%s: mem_w access disabled\n", machine().describe_context());
}

u16 sgi_pm2_device::map_r(offs_t offset, u16 mem_mask)
{
	if (m_status & STATUS_EN0)
	{
		LOG("%s: map_r access disabled\n", machine().describe_context());

		return 0;
	}
	else
		return m_map[offset >> 11];
}

void sgi_pm2_device::map_w(offs_t offset, u16 data, u16 mem_mask)
{
	LOG("%s: map_w page 0x%03x data 0x%04x\n", machine().describe_context(), offset >> 11, data);

	if (m_status & (STATUS_EN0 | STATUS_EN1))
		LOG("map_w access disabled\n");
	else
		m_map[offset >> 11] = data;
}

ROM_START(sgi_pm2)
	ROM_DEFAULT_BIOS("v36")
	ROM_SYSTEM_BIOS(0, "v36", "V3.6 February 28, 1985")

	ROM_REGION16_BE(0x8000, "prom0", 0)
	ROMX_LOAD("u2g_5000_361_04.0", 0x0000, 0x4000, CRC(110e0a9d) SHA1(6a6b03e7f0cb2aacbf11db39b13e94a8a54b2df5), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("u2g_5000_361_04.1", 0x0001, 0x4000, CRC(38fa42f3) SHA1(6bfc3c6872415c86df8a3ece651e4290f5a772c2), ROM_BIOS(0) | ROM_SKIP(1))

	ROM_REGION16_BE(0x8000, "prom1", 0)
	ROMX_LOAD("u9g_5000_363_04.0", 0x0000, 0x4000, CRC(709b9f5c) SHA1(389a043d27cb26e29bc55a72cb7f1cbaecaacd0c), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("u9g_5000_363_04.1", 0x0001, 0x4000, CRC(c380715b) SHA1(1cab5ce1c7df43ebeffab04b388d2e07c9853f8f), ROM_BIOS(0) | ROM_SKIP(1))
ROM_END

const tiny_rom_entry *sgi_pm2_device::device_rom_region() const
{
	return ROM_NAME(sgi_pm2);
}

static INPUT_PORTS_START(sgi_pm2)
	PORT_START("CONFIG")
	PORT_DIPNAME(0x03, 0x03, "Host Speed")
	PORT_DIPSETTING(0x00, "300")
	PORT_DIPSETTING(0x01, "1200")
	PORT_DIPSETTING(0x02, "19200")
	PORT_DIPSETTING(0x03, "9600")

	PORT_DIPNAME(0x04, 0x04, "Verbose")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x04, DEF_STR(On))

	PORT_DIPNAME(0xf8, 0x10, "Boot Environment")
	PORT_DIPSETTING(0x00, "Floppy")
	PORT_DIPSETTING(0x10, "Monitor")
	PORT_DIPSETTING(0x20, "SGI XNS")
	PORT_DIPSETTING(0x30, "Terminal Emulation")
	PORT_DIPSETTING(0x40, "Netboot 0")
	PORT_DIPSETTING(0x50, "Tape")
	PORT_DIPSETTING(0x60, "SGI 488")
	PORT_DIPSETTING(0x80, "SMD")
	PORT_DIPSETTING(0xb8, "Page Test")
	PORT_DIPSETTING(0xc0, "Protection Test")
	PORT_DIPSETTING(0xc8, "Context Test")
	PORT_DIPSETTING(0xd0, "DUART Test")
	PORT_DIPSETTING(0xd8, "Timer Test")
	PORT_DIPSETTING(0xe0, "RAM Boot Test")
	PORT_DIPSETTING(0xe8, "Slave")
	PORT_DIPSETTING(0xf0, "Diagnostics")
	PORT_DIPSETTING(0xf8, "Don't Touch")
INPUT_PORTS_END

ioport_constructor sgi_pm2_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(sgi_pm2);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(SGI_PM2, device_multibus_interface, sgi_pm2_device, "sgi_pm2", "Silicon Graphics PM2")
