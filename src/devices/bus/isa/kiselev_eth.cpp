// license:BSD-3-Clause
// copyright-holders:Dmitry Mikhalchenkov

/*
 * Sergey Kiselev ISA 8-Bit Ethernet Controller.
 *
 * An open hardware 8-bit ISA card built around the Realtek RTL8019AS, an
 * NE2000-compatible controller with an integrated 10BASE-T transceiver and
 * 16 KiB of on-chip packet buffer.  The controller is strapped for the 8-bit
 * bus, so the remote DMA data port is a single byte wide.  This emulates the
 * card in its Switches configuration mode, with the IRQ and I/O base address
 * set by the SW2 and SW3 DIP switches.
 *
 * The card occupies a 32 byte I/O window:
 *
 *   base+0x00..base+0x0f  NE2000 register window
 *   base+0x10..base+0x17  remote DMA data port
 *   base+0x18..base+0x1f  reset port (reading it resets the controller)
 *
 * The station address PROM appears to remote DMA at 0x0000 and the 16 KiB
 * packet buffer at 0x4000..0x7fff.  All of the on-chip SRAM is modelled, but
 * drivers strapped for the 8-bit bus normally keep the receive ring below page
 * 0x60 because the data sheet limits PSTOP in byte mode, so the upper half of
 * the buffer usually goes unused.
 *
 * Sources:
 *  - https://github.com/skiselev/isa8_eth
 *  - RTL8019AS Realtek Full-Duplex Ethernet Controller with Plug and Play
 *    Function (RealPNP) data sheet, Rev 1.3, Realtek Semiconductor, 1998.
 *
 * TODO:
 *  - 93C46 EEPROM, jumperless and Plug and Play configuration modes (SW1)
 *  - CONFIG0 and CONFIG1 do not reflect the switch configuration
 *  - boot ROM socket, its SW4 address and size selection, and the BPAGE
 *    page register
 *  - full duplex mode
 *  - ignore DCR WTS, which the 8-bit strapping overrides on real hardware;
 *    word wide remote DMA is currently only warned about
 */

#include "emu.h"
#include "kiselev_eth.h"

#include "machine/dp8390.h"

#include "multibyte.h"

#define LOG_IO  (1U << 1)
#define LOG_DMA (1U << 2)
#define LOG_IRQ (1U << 3)

//#define VERBOSE (LOG_GENERAL|LOG_IRQ)
#include "logmacro.h"

#define LOGIO(...)  LOGMASKED(LOG_IO, __VA_ARGS__)
#define LOGDMA(...) LOGMASKED(LOG_DMA, __VA_ARGS__)
#define LOGIRQ(...) LOGMASKED(LOG_IRQ, __VA_ARGS__)

namespace {

constexpr unsigned RAM_BASE = 0x4000;

class kiselev_eth_device : public device_t, public device_isa8_card_interface
{
public:
	kiselev_eth_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	u8 port_r(offs_t offset);
	void port_w(offs_t offset, u8 data);

	u8 mem_r(offs_t offset);
	void mem_w(offs_t offset, u8 data);

	void irq_w(int state);

	required_device<rtl8019a_device> m_nic;
	required_ioport m_config;

	u8 m_prom[32];
	u8 m_ram[16 * 1024];
	u8 m_irq;
	bool m_installed;
	bool m_wts_warned;
};

kiselev_eth_device::kiselev_eth_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ISA8_KISELEV_ETH, tag, owner, clock)
	, device_isa8_card_interface(mconfig, *this)
	, m_nic(*this, "rtl8019a")
	, m_config(*this, "CONFIG")
	, m_prom{}
	, m_ram{}
	, m_irq(0)
	, m_installed(false)
	, m_wts_warned(false)
{
}

void kiselev_eth_device::device_add_mconfig(machine_config &config)
{
	RTL8019A(config, m_nic, 0);
	m_nic->irq_callback().set(FUNC(kiselev_eth_device::irq_w));
	m_nic->mem_read_callback().set(FUNC(kiselev_eth_device::mem_r));
	m_nic->mem_write_callback().set(FUNC(kiselev_eth_device::mem_w));
}

void kiselev_eth_device::device_start()
{
	set_isa_device();

	// create a MAC address by hashing the device tag
	u8 mac[6] = { 0x02, 0xe0, 0x4c, 0x00, 0x00, 0x00 };
	u32 hash = 0;
	for (char const *p = tag(); *p; p++)
		hash = (hash * 33) ^ u8(*p);
	put_u24be(&mac[3], hash);
	m_nic->set_mac(mac);

	// Strapped for the 8-bit bus, the station address PROM is fetched a byte
	// at a time and the bytes are not duplicated as they are on a 16-bit NE2000
	// card.  The unused locations read as 'W', which NE drivers check for.
	std::fill(std::begin(m_prom), std::end(m_prom), 0x57);
	std::copy(std::begin(mac), std::end(mac), std::begin(m_prom));

	m_installed = false;
	m_wts_warned = false;

	save_item(NAME(m_ram));
	save_item(NAME(m_irq));
}

void kiselev_eth_device::device_reset()
{
	// the switches are only sampled at power-on
	if (!m_installed)
	{
		u8 const config = m_config->read();

		m_irq = BIT(config, 0, 2);

		// IOS3 selects 0x200 or 0x300, IOS2-IOS0 select one of eight 0x20 steps
		offs_t const base = (BIT(config, 7) ? 0x200 : 0x300) | (BIT(config, 4, 3) << 5);
		m_isa->install_device(base, base | 0x1f,
				read8sm_delegate(*this, FUNC(kiselev_eth_device::port_r)),
				write8sm_delegate(*this, FUNC(kiselev_eth_device::port_w)));

		m_installed = true;
	}
}

u8 kiselev_eth_device::port_r(offs_t offset)
{
	if (offset < 0x10)
	{
		u8 const data = m_nic->cs_read(offset);

		if (!machine().side_effects_disabled())
			LOGIO("register read 0x%02x data 0x%02x\n", offset, data);

		return data;
	}
	else if (offset < 0x18)
	{
		if (machine().side_effects_disabled())
			return 0xff;

		u8 const data = u8(m_nic->remote_read());

		LOGDMA("remote read data 0x%02x\n", data);

		return data;
	}
	else
	{
		// the dp8390 device models only the release, so the read is what
		// actually resets the controller
		if (!machine().side_effects_disabled())
		{
			LOG("reset cleared\n");

			m_nic->dp8390_reset(CLEAR_LINE);
		}

		return 0;
	}
}

void kiselev_eth_device::port_w(offs_t offset, u8 data)
{
	if (offset < 0x10)
	{
		LOGIO("register write 0x%02x data 0x%02x\n", offset, data);

		if ((offset == 0x0e) && BIT(data, 0) && !m_wts_warned && !BIT(m_nic->cs_read(0), 6, 2))
		{
			m_wts_warned = true;
			logerror("word wide remote DMA selected on an 8-bit card, transfers will lose data\n");
		}

		m_nic->cs_write(offset, data);
	}
	else if (offset < 0x18)
	{
		LOGDMA("remote write data 0x%02x\n", data);

		m_nic->remote_write(data);
	}
	else
	{
		LOG("reset asserted\n");

		m_nic->dp8390_reset(ASSERT_LINE);
	}
}

u8 kiselev_eth_device::mem_r(offs_t offset)
{
	if (offset < std::size(m_prom))
		return m_prom[offset];

	if ((offset >= RAM_BASE) && (offset < (RAM_BASE + std::size(m_ram))))
		return m_ram[offset - RAM_BASE];

	LOGDMA("invalid buffer read offset 0x%04x\n", offset);

	return 0xff;
}

void kiselev_eth_device::mem_w(offs_t offset, u8 data)
{
	if ((offset >= RAM_BASE) && (offset < (RAM_BASE + std::size(m_ram))))
	{
		m_ram[offset - RAM_BASE] = data;
		return;
	}

	LOGDMA("invalid buffer write offset 0x%04x data 0x%02x\n", offset, data);
}

void kiselev_eth_device::irq_w(int state)
{
	LOGIRQ("irq %d\n", state);

	switch (m_irq)
	{
	case 0: m_isa->irq2_w(state); break;
	case 1: m_isa->irq3_w(state); break;
	case 2: m_isa->irq4_w(state); break;
	case 3: m_isa->irq5_w(state); break;
	}
}

INPUT_PORTS_START(kiselev_eth)
	PORT_START("CONFIG")
	PORT_CONFNAME(0x03, 0x01, "Interrupt (SW2)")
	PORT_CONFSETTING(0x00, "IRQ2/9")
	PORT_CONFSETTING(0x01, "IRQ3")
	PORT_CONFSETTING(0x02, "IRQ4")
	PORT_CONFSETTING(0x03, "IRQ5")
	PORT_CONFNAME(0xf0, 0x00, "I/O Base Address (SW3)")
	PORT_CONFSETTING(0x80, "0x200")
	PORT_CONFSETTING(0x90, "0x220")
	PORT_CONFSETTING(0xa0, "0x240")
	PORT_CONFSETTING(0xb0, "0x260")
	PORT_CONFSETTING(0xc0, "0x280")
	PORT_CONFSETTING(0xd0, "0x2A0")
	PORT_CONFSETTING(0xe0, "0x2C0")
	PORT_CONFSETTING(0xf0, "0x2E0")
	PORT_CONFSETTING(0x00, "0x300")
	PORT_CONFSETTING(0x10, "0x320")
	PORT_CONFSETTING(0x20, "0x340")
	PORT_CONFSETTING(0x30, "0x360")
	PORT_CONFSETTING(0x40, "0x380")
	PORT_CONFSETTING(0x50, "0x3A0")
	PORT_CONFSETTING(0x60, "0x3C0")
	PORT_CONFSETTING(0x70, "0x3E0")
INPUT_PORTS_END

ioport_constructor kiselev_eth_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(kiselev_eth);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(ISA8_KISELEV_ETH, device_isa8_card_interface, kiselev_eth_device, "kiselev_eth", "Sergey Kiselev ISA 8-Bit Ethernet Controller")
