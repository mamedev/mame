// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Ungermann-Bass Personal Network Interface Controller (also known as IBM RT PC Baseband Adapter).
 *
 * Sources:
 *  - IBM RT PC Hardware Technical Reference, Volume II (84X0873), Second Edition (September 1986), International Business Machines Corporation 1986.
 *  - IBM Academic Operating System 4.3 source code (sys/caif/if_un.[ch])
 *
 * TODO:
 *  - overrun recovery
 *  - TPOK from EDLC
 */
#include "emu.h"

#include "ubpnic.h"

#include "machine/mb8795.h"

#define LOG_REGR (1U << 1)
#define LOG_REGW (1U << 2)
#define LOG_BUF  (1U << 3)

//#define VERBOSE (LOG_GENERAL|LOG_REGR|LOG_REGW)
#include "logmacro.h"

namespace {

enum intstat_mask : u8
{
	INTSTAT_TXDONE = 0x01, // transmission done
	INTSTAT_TPKTOK = 0x02, // packet transmission ok
	INTSTAT_DLRINT = 0x04, // edlc receive error detect interrupt (0=asserted)
	INTSTAT_DLTINT = 0x08, // edlc transmit error detect interrupt (0=asserted)
	INTSTAT_TIMINT = 0x10, // timer interrupt (0=asserted)
	INTSTAT_SFTINT = 0x20, // software interrupt (0=asserted)
	INTSTAT_PAVINT = 0x40, // packet available interrupt (0=asserted)
	INTSTAT_TRCINT = 0x80, // transmit complete interrupt (0=asserted)

	INTSTAT_ALLINT = 0xfc,
};
enum intctl_mask : u8
{
	INTCTL_TRCIE = 0x80, // transmit complete interrupt enable
	INTCTL_PAVIE = 0x40, // packet available interrupt enable
	INTCTL_SFTIE = 0x20, // software interrupt enable
	INTCTL_TIMIE = 0x10, // timer interrupt enable

	INTCTL_ALL   = 0xf0,
};
enum epppav_mask : u8
{
	EPP_EPP = 0x7f, // empty page pointer
	EPP_PAV = 0x80, // packet available

	EPP_MAX = 0x5f,
};
enum fppmie_mask : u8
{
	FPP_FPP   = 0x7f, // full page pointer
	FPP_INTEN = 0x80, // master interrupt enable
};
enum rpidx_mask : u8
{
	IDX_OFFSET   = 0x7f, // page byte count - 1
	IDX_LASTPAGE = 0x80, // end of packet

	IDX_MAX      = 0x7f,
};

class isa8_ubpnic_device
	: public device_t
	, public device_isa8_card_interface
{
public:
	isa8_ubpnic_device(machine_config const &mconfig, char const *const tag, device_t *owner, u32 clock)
		: device_t(mconfig, ISA8_UBPNIC, tag, owner, clock)
		, device_isa8_card_interface(mconfig, *this)
		, m_edlc(*this, "edlc")
		, m_w9(*this, "W9")
		, m_w10_w13(*this, "W10-W13")
		, m_w14(*this, "W14")
		, m_buf(nullptr)
		, m_idx(nullptr)
		, m_int_state(false)
	{
	}

protected:
	virtual tiny_rom_entry const *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

	void mem_map(address_map &map);

	u8 txinit_r();
	u8 clrpav_r();
	u8 intstat_r();
	u8 epppav_r();

	void tsamsb_w(u8 data);
	void tsalsb_w(u8 data);
	void intctl_w(u8 data);
	void fppmie_w(u8 data);

	u8 rpidx_r(offs_t offset);
	void rpidx_w(offs_t offset, u8 data);

	u8 buf_r(offs_t offset);
	void buf_w(offs_t offset, u8 data);

private:
	bool rxb_full() const;
	void next_page();
	void rxdrq_w(int state);
	void txdrq_w(int state);

	template <u8 N> void irq(int state);
	void interrupt();

	required_device<mb8795_device> m_edlc;

	required_ioport m_w9;
	required_ioport m_w10_w13;
	required_ioport m_w14;

	emu_timer *m_timer;

	// registers
	u16 m_tsa;    // transmit start address
	u8 m_intstat; // interrupt status
	u8 m_intctl;  // interrupt control
	u8 m_epppav;  // empty page pointer
	u8 m_fppmie;  // full page pointer

	std::unique_ptr<u8[]> m_buf; // receive/transmit buffer
	std::unique_ptr<u8[]> m_idx; // receive page index array

	// internal state
	u8 m_irq;
	bool m_int_state;
	bool m_installed;
};

void isa8_ubpnic_device::device_add_mconfig(machine_config &config)
{
	MB8795(config, m_edlc, 100_MHz_XTAL / 10); // MB8795B
	m_edlc->rx_irq().set(FUNC(isa8_ubpnic_device::irq<INTSTAT_DLRINT>));
	m_edlc->rx_drq().set(FUNC(isa8_ubpnic_device::rxdrq_w));
	m_edlc->tx_irq().set(FUNC(isa8_ubpnic_device::irq<INTSTAT_DLTINT>));
	m_edlc->tx_drq().set(FUNC(isa8_ubpnic_device::txdrq_w));
}

void isa8_ubpnic_device::device_start()
{
	m_buf = std::make_unique<u8[]>(0x4000); // MB81416-12 x2
	m_idx = std::make_unique<u8[]>(96);     // P2114AL-3 x2

	save_item(NAME(m_tsa));
	save_item(NAME(m_intstat));
	save_item(NAME(m_intctl));
	save_item(NAME(m_epppav));
	save_item(NAME(m_fppmie));

	save_pointer(NAME(m_buf), 0x4000);
	save_pointer(NAME(m_idx), 96);

	set_isa_device();

	m_timer = timer_alloc(FUNC(isa8_ubpnic_device::irq<INTSTAT_TIMINT>), this);
	m_intstat = INTSTAT_ALLINT;
	m_installed = false;
}

void isa8_ubpnic_device::device_reset()
{
	m_tsa = 0;
	m_intstat |= INTSTAT_ALLINT;
	m_intctl = 0;
	m_epppav = 0;
	m_fppmie = 0;

	m_idx[0] = 0;

	if (!m_installed)
	{
		m_irq = m_w9->read();

		u32 const base = m_w10_w13->read() << 12;
		m_isa->install_memory(base, base | 0x7fff, *this, &isa8_ubpnic_device::mem_map);

		m_installed = true;
	}

	interrupt();
}

void isa8_ubpnic_device::mem_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("mac", 0);
	map(0x2080, 0x2080).rw(FUNC(isa8_ubpnic_device::txinit_r), FUNC(isa8_ubpnic_device::tsamsb_w));
	map(0x2081, 0x2081).rw(FUNC(isa8_ubpnic_device::clrpav_r), FUNC(isa8_ubpnic_device::tsalsb_w));
	map(0x2082, 0x2082).rw(FUNC(isa8_ubpnic_device::intstat_r), FUNC(isa8_ubpnic_device::intctl_w));
	map(0x2083, 0x2083).rw(FUNC(isa8_ubpnic_device::epppav_r), FUNC(isa8_ubpnic_device::fppmie_w));
	map(0x2100, 0x215f).rw(FUNC(isa8_ubpnic_device::rpidx_r), FUNC(isa8_ubpnic_device::rpidx_w)); // TODO: read only?
	map(0x2180, 0x218f).m(m_edlc, FUNC(mb8795_device::map));
	map(0x4000, 0x7fff).rw(FUNC(isa8_ubpnic_device::buf_r), FUNC(isa8_ubpnic_device::buf_w));
}

u8 isa8_ubpnic_device::txinit_r()
{
	if (!machine().side_effects_disabled())
	{
		LOGMASKED(LOG_REGR, "%s: txinit_r\n", machine().describe_context());

		m_intstat &= ~(INTSTAT_TPKTOK | INTSTAT_TXDONE);

		// trigger dma transfer
		txdrq_w(1);
	}

	return 0;
}

u8 isa8_ubpnic_device::clrpav_r()
{
	if (!machine().side_effects_disabled())
	{
		LOGMASKED(LOG_REGR, "%s: clrpav_r\n", machine().describe_context());

		// initialize next empty page if rx buffer was full
		if (rxb_full())
			m_idx[m_epppav & EPP_EPP] = 0;

		m_epppav &= ~EPP_PAV;

		if (!(m_intstat & INTSTAT_PAVINT))
			irq<INTSTAT_PAVINT>(0);
	}

	return 0;
}

u8 isa8_ubpnic_device::intstat_r()
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REGR, "%s: intstat_r 0x%02x\n", machine().describe_context(), m_intstat);

	return m_intstat;
}

u8 isa8_ubpnic_device::epppav_r()
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REGR, "%s: epppav_r 0x%02x\n", machine().describe_context(), m_epppav);

	return m_epppav;
}

void isa8_ubpnic_device::tsamsb_w(u8 data)
{
	LOGMASKED(LOG_REGW, "%s: tsamsb_w 0x%02x\n", machine().describe_context(), data);

	m_tsa = u16(data) << 8 | (m_tsa & 0x00ff);
}

void isa8_ubpnic_device::tsalsb_w(u8 data)
{
	LOGMASKED(LOG_REGW, "%s: tsalsb_w 0x%02x\n", machine().describe_context(), data);

	m_tsa = (m_tsa & 0xff00) | data;
}

void isa8_ubpnic_device::intctl_w(u8 data)
{
	LOGMASKED(LOG_REGW, "%s: intctl_w 0x%02x\n", machine().describe_context(), data);

	// clear disabled interrupts
	m_intstat |= (~data) & INTCTL_ALL;

	// assert enabled software interrupt
	if (data & INTCTL_SFTIE)
		m_intstat &= ~INTSTAT_SFTINT;

	// enable/disable timer
	if ((data ^ m_intctl) & INTCTL_TIMIE)
	{
		if (data & INTCTL_TIMIE)
		{
			// timer is multiple of 9.15ms
			attotime const period = attotime::from_double(0.00915 * m_w14->read());

			m_timer->adjust(period, 1, period);
		}
		else
			m_timer->reset();
	}

	m_intctl = data;

	interrupt();
}

void isa8_ubpnic_device::fppmie_w(u8 data)
{
	LOGMASKED(LOG_REGW, "%s: fppmie_w 0x%02x\n", machine().describe_context(), data);

	m_fppmie = data;

	interrupt();
}

u8 isa8_ubpnic_device::rpidx_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REGR, "%s: rpidx_r 0x%02x data 0x%02x\n", machine().describe_context(), offset, m_idx[offset]);

	return m_idx[offset];
}

void isa8_ubpnic_device::rpidx_w(offs_t offset, u8 data)
{
	LOGMASKED(LOG_REGW, "%s: rpidx_w 0x%02x data 0x%02x\n", machine().describe_context(), offset, data);

	m_idx[offset] = data;
}

u8 isa8_ubpnic_device::buf_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_BUF, "%s: buf_r 0x%02x data 0x%02x\n", machine().describe_context(), offset, m_buf[offset]);

	return m_buf[offset];
}

void isa8_ubpnic_device::buf_w(offs_t offset, u8 data)
{
	LOGMASKED(LOG_BUF, "%s: buf_w 0x%02x data 0x%02x\n", machine().describe_context(), offset, data);

	m_buf[offset] = data;
}

bool isa8_ubpnic_device::rxb_full() const
{
	return (m_epppav & EPP_PAV) && (m_epppav & EPP_EPP) == (m_fppmie & FPP_FPP);
}

void isa8_ubpnic_device::next_page()
{
	// advance empty page pointer
	if ((m_epppav & EPP_EPP) == EPP_MAX)
		m_epppav &= ~EPP_EPP;
	else
		m_epppav++;

	// initialize next receive page index
	if (!rxb_full())
		m_idx[m_epppav & EPP_EPP] = 0;
}

void isa8_ubpnic_device::rxdrq_w(int state)
{
	if (!state)
		return;

	// check for overrun
	if (rxb_full())
	{
		LOG("receive buffer overrun\n");
		return;
	}

	u8 const epp = m_epppav & EPP_EPP;
	u8 &idx = m_idx[epp];
	bool eof;

	// store received byte at current page and offset
	m_edlc->rx_dma_r(m_buf[(epp * 128) + idx], eof);

	if (!eof)
	{
		if (idx == IDX_MAX)
			next_page();
		else
			idx++;
	}
	else
	{
		LOG("receive complete\n");

		// flag end of packet
		idx |= IDX_LASTPAGE;

		next_page();

		m_epppav |= EPP_PAV;
		irq<INTSTAT_PAVINT>(1);
	}
}

void isa8_ubpnic_device::txdrq_w(int state)
{
	if (!state)
		return;

	if (m_intstat & INTSTAT_TXDONE)
		return;

	bool const eof = (m_tsa & 0x7ff) == 0x7ff;
	u8 const data = m_buf[0x3000 | (m_tsa++ & 0xfff)];

	// transmit byte at current page and offset
	m_edlc->tx_dma_w(data, eof);

	if (eof)
	{
		LOG("transmit complete\n");

		m_intstat |= INTSTAT_TPKTOK | INTSTAT_TXDONE;
		irq<INTSTAT_TRCINT>(1);
	}
}

template <u8 IRQ> void isa8_ubpnic_device::irq(int state)
{
	if (state)
		m_intstat &= ~IRQ;
	else
		m_intstat |= IRQ;

	interrupt();
}

void isa8_ubpnic_device::interrupt()
{
	bool const int_state = ((~m_intstat & (m_intctl | INTSTAT_DLTINT | INTSTAT_DLRINT)) & INTSTAT_ALLINT) && (m_fppmie & FPP_INTEN);

	if (int_state != m_int_state)
	{
		LOG("interrupt %u\n", int_state);

		m_int_state = int_state;
		switch (m_irq)
		{
		case 2: m_isa->irq2_w(int_state ? ASSERT_LINE : CLEAR_LINE); break;
		case 3: m_isa->irq3_w(int_state ? ASSERT_LINE : CLEAR_LINE); break;
		case 4: m_isa->irq4_w(int_state ? ASSERT_LINE : CLEAR_LINE); break;
		case 5: m_isa->irq5_w(int_state ? ASSERT_LINE : CLEAR_LINE); break;
		case 6: m_isa->irq6_w(int_state ? ASSERT_LINE : CLEAR_LINE); break;
		case 7: m_isa->irq7_w(int_state ? ASSERT_LINE : CLEAR_LINE); break;
		}
	}
}

ROM_START(ubpnic)
	/*
	 * The card supports a 32Kbit or 64Kbit EPROM, which is filled with 0xff
	 * except for 6 bytes from offset 0x10 which contain the device medium
	 * access control address. It is assumed that when a 32Kbit EPROM is fitted
	 * that it is mirrored into the upper 4KiB of the address range.
	 *
	 * This dump is hand-made to assign address 00:dd:01:12:34:56 (the prefix
	 * is assigned to Ungermann-Bass Inc.), but otherwise matches a real dump
	 * from a board with a 32Kbit EPROM.
	 */
	ROM_REGION(0x2000, "mac", 0)
	ROM_LOAD("mac.u62", 0x0000, 0x2000, CRC(b651fec4) SHA1(e0effcf33a25954d990d0d01d5c0ac009a1f282d))
ROM_END

tiny_rom_entry const *isa8_ubpnic_device::device_rom_region() const
{
	return ROM_NAME(ubpnic);
}

INPUT_PORTS_START(ubpnic)
	PORT_START("W9")
	PORT_CONFNAME(0x07, 0x03, "Interrupt Level Select")
	PORT_CONFSETTING(0x03, "IR3")
	PORT_CONFSETTING(0x04, "IR4")
	PORT_CONFSETTING(0x05, "IR5")
	PORT_CONFSETTING(0x06, "IR6")
	PORT_CONFSETTING(0x07, "IR7")
	PORT_CONFSETTING(0x02, "IR9 (IR2)")

	PORT_START("W14")
	PORT_CONFNAME(0x0f, 0x02, "Interrupt Request Rate")
	PORT_CONFSETTING(0x01, "9.15 ms")
	PORT_CONFSETTING(0x02, "18.3 ms")
	PORT_CONFSETTING(0x04, "36.6 ms")
	PORT_CONFSETTING(0x08, "73.2 ms")

	PORT_START("W10-W13")
	PORT_CONFNAME(0xf8, 0x80, "I/O Memory Address")
	PORT_CONFSETTING(0x80, "0x080000")
	PORT_CONFSETTING(0x88, "0x088000")
	PORT_CONFSETTING(0x90, "0x090000")
	PORT_CONFSETTING(0x98, "0x098000")
	PORT_CONFSETTING(0xa0, "0x0a0000")
	PORT_CONFSETTING(0xa8, "0x0a8000")
	PORT_CONFSETTING(0xb0, "0x0b0000")
	PORT_CONFSETTING(0xb8, "0x0b8000")
	PORT_CONFSETTING(0xc0, "0x0c0000")
	PORT_CONFSETTING(0xc8, "0x0c8000")
	PORT_CONFSETTING(0xd0, "0x0d0000")
	PORT_CONFSETTING(0xd8, "0x0d8000")
	PORT_CONFSETTING(0xe0, "0x0e0000")
	PORT_CONFSETTING(0xe8, "0x0e8000")
	PORT_CONFSETTING(0xf0, "0x0f0000")
	PORT_CONFSETTING(0xf8, "0x0f8000")
INPUT_PORTS_END

ioport_constructor isa8_ubpnic_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(ubpnic);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(ISA8_UBPNIC, device_isa8_card_interface, isa8_ubpnic_device, "ubpnic", "Ungermann-Bass Personal NIC")
