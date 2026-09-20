// license:BSD-3-Clause
// copyright-holders:Dmitry Mikhalchenkov

/*
 * Realtek RTL8019AS Ethernet adapter.
 *
 * The RTL8019AS is an NE2000-compatible controller with an integrated
 * 10BASE-T/10BASE2 transceiver and 16 KiB of on-chip packet buffer.  This
 * emulates a jumper configured card strapped for the 8-bit ISA bus, as used by
 * the Sprinter, so the remote DMA data port is a single byte wide.
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
 *  - RTL8019AS Realtek Full-Duplex Ethernet Controller with Plug and Play
 *    Function (RealPNP) data sheet, Rev 1.3, Realtek Semiconductor, 1998.
 *
 * TODO:
 *  - 93C46 EEPROM, jumperless and Plug and Play configuration modes
 *  - boot ROM window and the BPAGE page register
 *  - full duplex mode
 *  - ignore DCR WTS, which the 8-bit strapping overrides on real hardware;
 *    word wide remote DMA is currently only warned about
 */

#include "emu.h"
#include "rtl8019as.h"

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

class isa8_rtl8019as_device : public device_t, public device_isa8_card_interface
{
public:
	isa8_rtl8019as_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

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

	required_device<rtl8019a_device> m_dp8390;
	required_ioport m_config;

	u8 m_prom[32];
	u8 m_ram[16 * 1024];
	u8 m_irq;
	u8 m_page;
	bool m_installed;
	bool m_wts_warned;
};

isa8_rtl8019as_device::isa8_rtl8019as_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ISA8_RTL8019AS, tag, owner, clock)
	, device_isa8_card_interface(mconfig, *this)
	, m_dp8390(*this, "rtl8019a")
	, m_config(*this, "CONFIG")
	, m_prom{}
	, m_ram{}
	, m_irq(0)
	, m_page(0)
	, m_installed(false)
	, m_wts_warned(false)
{
}

void isa8_rtl8019as_device::device_add_mconfig(machine_config &config)
{
	RTL8019A(config, m_dp8390, 0);
	m_dp8390->irq_callback().set(FUNC(isa8_rtl8019as_device::irq_w));
	m_dp8390->mem_read_callback().set(FUNC(isa8_rtl8019as_device::mem_r));
	m_dp8390->mem_write_callback().set(FUNC(isa8_rtl8019as_device::mem_w));
}

void isa8_rtl8019as_device::device_start()
{
	set_isa_device();

	// The 93C46 EEPROM that supplies the station address is not emulated, so
	// derive a locally administered address from the device tag.  This keeps
	// the address stable from run to run and distinct between cards in one
	// machine, but two instances of the same machine bridged onto a single
	// network will need one of them reprogrammed through PAR0-PAR5.
	u8 mac[6] = { 0x02, 0xe0, 0x4c, 0x00, 0x00, 0x00 };
	u32 hash = 0;
	for (char const *p = tag(); *p; p++)
		hash = (hash * 33) ^ u8(*p);
	put_u24be(&mac[3], hash);
	m_dp8390->set_mac(mac);

	// Strapped for the 8-bit bus, the station address PROM is fetched a byte
	// at a time and the bytes are not duplicated as they are on a 16-bit NE2000
	// card.  The unused locations read as 'W', which NE drivers check for.
	std::fill(std::begin(m_prom), std::end(m_prom), 0x57);
	std::copy(std::begin(mac), std::end(mac), std::begin(m_prom));

	m_installed = false;
	m_wts_warned = false;

	save_item(NAME(m_ram));
	save_item(NAME(m_irq));
	save_item(NAME(m_page));
}

void isa8_rtl8019as_device::device_reset()
{
	m_page = 0;

	// the jumpers are only sampled at power-on
	if (!m_installed)
	{
		u8 const config = m_config->read();

		m_irq = BIT(config, 0, 2);

		// 0x300, 0x320, 0x340 or 0x360
		offs_t const base = 0x300 | (BIT(config, 4, 2) << 5);
		m_isa->install_device(base, base | 0x1f,
				read8sm_delegate(*this, FUNC(isa8_rtl8019as_device::port_r)),
				write8sm_delegate(*this, FUNC(isa8_rtl8019as_device::port_w)));

		m_installed = true;
	}
}

u8 isa8_rtl8019as_device::port_r(offs_t offset)
{
	if (offset < 0x10)
	{
		u8 const data = m_dp8390->cs_read(offset);

		LOGIO("register read 0x%02x data 0x%02x\n", offset, data);

		return data;
	}
	else if (offset < 0x18)
	{
		// The remote DMA data port is mirrored across eight locations and is a
		// single byte wide on an 8-bit card.  Reading it advances the remote
		// DMA address, so give the debugger nothing rather than let it disturb
		// a transfer in progress.
		if (machine().side_effects_disabled())
			return 0xff;

		u8 const data = u8(m_dp8390->remote_read());

		LOGDMA("remote read data 0x%02x\n", data);

		return data;
	}
	else
	{
		// The reset port is mirrored across eight locations.  Drivers read it
		// and write the value back; the dp8390 device models only the release,
		// so the read is what actually resets the controller.
		if (!machine().side_effects_disabled())
		{
			LOG("reset cleared\n");

			m_dp8390->dp8390_reset(CLEAR_LINE);
			m_page = 0;
		}

		return 0;
	}
}

void isa8_rtl8019as_device::port_w(offs_t offset, u8 data)
{
	if (offset < 0x10)
	{
		LOGIO("register write 0x%02x data 0x%02x\n", offset, data);

		// The page select lives in the command register, and the register at
		// offset 0x0e is DCR only while page 0 is selected, so the page has to
		// be followed to tell a DCR write from a multicast filter write.
		if (!offset)
			m_page = BIT(data, 6, 2);
		else if ((offset == 0x0e) && !m_page && BIT(data, 0) && !m_wts_warned)
		{
			// The controller honours DCR WTS, but the 8-bit strapping overrides
			// it on real hardware and the data port here is a byte wide, so a
			// word wide transfer would quietly drop every second byte.
			m_wts_warned = true;
			logerror("word wide remote DMA selected on an 8-bit card, transfers will lose data\n");
		}

		m_dp8390->cs_write(offset, data);
	}
	else if (offset < 0x18)
	{
		LOGDMA("remote write data 0x%02x\n", data);

		m_dp8390->remote_write(data);
	}
	else
	{
		LOG("reset asserted\n");

		m_dp8390->dp8390_reset(ASSERT_LINE);
	}
}

u8 isa8_rtl8019as_device::mem_r(offs_t offset)
{
	if (offset < std::size(m_prom))
		return m_prom[offset];

	if ((offset >= RAM_BASE) && (offset < (RAM_BASE + std::size(m_ram))))
		return m_ram[offset - RAM_BASE];

	LOGDMA("invalid buffer read offset 0x%04x\n", offset);

	return 0xff;
}

void isa8_rtl8019as_device::mem_w(offs_t offset, u8 data)
{
	if ((offset >= RAM_BASE) && (offset < (RAM_BASE + std::size(m_ram))))
	{
		m_ram[offset - RAM_BASE] = data;
		return;
	}

	LOGDMA("invalid buffer write offset 0x%04x data 0x%02x\n", offset, data);
}

void isa8_rtl8019as_device::irq_w(int state)
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

INPUT_PORTS_START(rtl8019as)
	PORT_START("CONFIG")
	PORT_CONFNAME(0x03, 0x01, "Interrupt")
	PORT_CONFSETTING(0x00, "IRQ2/9")
	PORT_CONFSETTING(0x01, "IRQ3")
	PORT_CONFSETTING(0x02, "IRQ4")
	PORT_CONFSETTING(0x03, "IRQ5")
	PORT_CONFNAME(0x30, 0x00, "I/O Base Address")
	PORT_CONFSETTING(0x00, "0x300")
	PORT_CONFSETTING(0x10, "0x320")
	PORT_CONFSETTING(0x20, "0x340")
	PORT_CONFSETTING(0x30, "0x360")
INPUT_PORTS_END

ioport_constructor isa8_rtl8019as_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(rtl8019as);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(ISA8_RTL8019AS, device_isa8_card_interface, isa8_rtl8019as_device, "rtl8019as", "Realtek RTL8019AS Ethernet Adapter")
