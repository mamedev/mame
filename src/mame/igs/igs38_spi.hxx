// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
// IGS38 SPI master controller.
// Eight-entry transmit and receive FIFOs, polled/interrupt-driven transfers.
// TODO: slave mode, DMA handshakes, exact serial clock divider/delay timing.
class igs38_spi_device : public device_t
{
public:
	igs38_spi_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	auto irq_callback() { return m_irq.bind(); }
	auto transmit_callback() { return m_transmit.bind(); }
	auto receive_callback() { return m_receive.bind(); }
	template <u32 Line> auto select_callback() { return m_select[Line].bind(); }
	void map(address_map &map) ATTR_COLD;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	template <u32 Register> u32 register_r();
	template <u32 Register> void register_w(offs_t offset, u32 data, u32 mem_mask = ~0U);
	u32 status() const;
	void update_irq();
	void update_select();
	void schedule();
	TIMER_CALLBACK_MEMBER(transfer);

	devcb_write_line m_irq;
	devcb_write16 m_transmit;
	devcb_read16 m_receive;
	devcb_write_line::array<2> m_select;
	emu_timer *m_timer;
	u32 m_regs[18];
	u16 m_tx[8], m_rx[8];
	u8 m_tx_head, m_tx_count, m_rx_head, m_rx_count;
	u32 m_tx_remaining, m_rx_remaining;
	u8 m_select_state;
	bool m_irq_state;
};
DEFINE_DEVICE_TYPE_PRIVATE(IGS38_SPI, igs38_spi_device, igs38_spi_device, "igs38_spi", "IGS38 SPI controller")

igs38_spi_device::igs38_spi_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, IGS38_SPI, tag, owner, clock)
	, m_irq(*this)
	, m_transmit(*this)
	, m_receive(*this, 0xffff)
	, m_select(*this)
{
}

void igs38_spi_device::map(address_map &map)
{
	map(0x00, 0x03).r(FUNC(igs38_spi_device::register_r<0>)).w(FUNC(igs38_spi_device::register_w<0>));
	map(0x04, 0x07).r(FUNC(igs38_spi_device::register_r<1>)).w(FUNC(igs38_spi_device::register_w<1>));
	map(0x08, 0x0b).r(FUNC(igs38_spi_device::register_r<2>)).w(FUNC(igs38_spi_device::register_w<2>));
	map(0x0c, 0x0f).r(FUNC(igs38_spi_device::register_r<3>)).w(FUNC(igs38_spi_device::register_w<3>));
	map(0x10, 0x13).r(FUNC(igs38_spi_device::register_r<4>)).w(FUNC(igs38_spi_device::register_w<4>));
	map(0x14, 0x17).r(FUNC(igs38_spi_device::register_r<5>)).w(FUNC(igs38_spi_device::register_w<5>));
	map(0x18, 0x1b).r(FUNC(igs38_spi_device::register_r<6>)).w(FUNC(igs38_spi_device::register_w<6>));
	map(0x1c, 0x1f).r(FUNC(igs38_spi_device::register_r<7>)).w(FUNC(igs38_spi_device::register_w<7>));
	map(0x20, 0x23).r(FUNC(igs38_spi_device::register_r<8>)).w(FUNC(igs38_spi_device::register_w<8>));
	map(0x24, 0x27).r(FUNC(igs38_spi_device::register_r<9>)).nopw();
	map(0x28, 0x2b).r(FUNC(igs38_spi_device::register_r<10>)).w(FUNC(igs38_spi_device::register_w<10>));
	map(0x2c, 0x2f).r(FUNC(igs38_spi_device::register_r<11>)).w(FUNC(igs38_spi_device::register_w<11>));
	map(0x30, 0x33).r(FUNC(igs38_spi_device::register_r<12>)).w(FUNC(igs38_spi_device::register_w<12>));
	map(0x34, 0x37).r(FUNC(igs38_spi_device::register_r<13>)).w(FUNC(igs38_spi_device::register_w<13>));
	map(0x38, 0x3b).r(FUNC(igs38_spi_device::register_r<14>)).nopw();
	map(0x3c, 0x3f).r(FUNC(igs38_spi_device::register_r<15>)).nopw();
	map(0x40, 0x43).r(FUNC(igs38_spi_device::register_r<16>)).nopw();
	map(0x44, 0x47).r(FUNC(igs38_spi_device::register_r<17>)).nopw();
}

void igs38_spi_device::device_start()
{
	m_timer = timer_alloc(FUNC(igs38_spi_device::transfer), this);
	m_irq_state = false;
	m_select_state = 3;
	save_item(NAME(m_regs));
	save_item(NAME(m_tx));
	save_item(NAME(m_rx));
	save_item(NAME(m_tx_head));
	save_item(NAME(m_tx_count));
	save_item(NAME(m_rx_head));
	save_item(NAME(m_rx_count));
	save_item(NAME(m_tx_remaining));
	save_item(NAME(m_rx_remaining));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_select_state));
}

void igs38_spi_device::device_reset()
{
	m_timer->adjust(attotime::never);
	std::fill(std::begin(m_regs), std::end(m_regs), 0);
	std::fill(std::begin(m_tx), std::end(m_tx), 0);
	std::fill(std::begin(m_rx), std::end(m_rx), 0);
	m_regs[3] = 0x100;
	m_tx_head = m_tx_count = m_rx_head = m_rx_count = 0;
	m_tx_remaining = m_rx_remaining = 0;
	update_select();
	update_irq();
}

u32 igs38_spi_device::status() const
{
	u32 const rx_level = std::min(8U, 2U * (1 + ((m_regs[2] >> 11) & 7)));
	u32 const tx_level = std::min(8U, 2U * (1 + ((m_regs[2] >> 8) & 7)));
	return (m_regs[8] & 0xfa3) | ((m_tx_count == 0) ? 0x40 : 0) | (m_rx_count ? 0x10 : 0) | ((8 - m_tx_count >= tx_level) ? 8 : 0) |
			((m_rx_count >= rx_level) ? 4 : 0);
}

void igs38_spi_device::update_irq()
{
	bool const irq = bool(status() & m_regs[1]);
	if (m_irq_state != irq)
	{
		m_irq_state = irq;
		m_irq(irq);
	}
}

void igs38_spi_device::update_select()
{
	u32 const selected = BIT(m_regs[7], 8, 3);
	bool const active = BIT(m_regs[3], 14) ? !BIT(m_regs[3], 13) : BIT(m_regs[3], 9);
	u8 const lines = (active && (selected < 2)) ? (3U & ~(1U << selected)) : 3;
	for (u32 i = 0; i < 2; ++i)
	{
		if (BIT(m_select_state ^ lines, i))
			m_select[i](BIT(lines, i));
	}
	m_select_state = lines;
}

template <u32 Register>
u32 igs38_spi_device::register_r()
{
	if constexpr (Register == 0)
	{
		u32 const value = m_rx_count ? m_rx[m_rx_head] : 0;
		if (m_rx_count && !machine().side_effects_disabled())
		{
			m_rx_head = (m_rx_head + 1) & 7;
			--m_rx_count;
			update_irq();
			schedule();
		}
		return value;
	}
	if constexpr (Register == 2)
		return (m_regs[2] & 0x3f00) | ((m_tx_count == 8) ? 2 : 0) | (m_rx_count ? 1 : 0);
	if constexpr (Register == 8)
	{
		u32 const value = status();
		if (!machine().side_effects_disabled())
		{
			m_regs[8] &= ~1U;
			update_irq();
		}
		return value;
	}
	if constexpr (Register == 9)
		return ((m_tx_count == 0) ? 1 : 0) | ((m_tx_count >= 4) ? 2 : 0) | ((m_tx_count == 8) ? 4 : 0) | ((m_rx_count == 0) ? 0x10 : 0) |
				((m_rx_count >= 4) ? 0x20 : 0) | ((m_rx_count == 8) ? 0x40 : 0);
	return m_regs[Register];
}

template <u32 Register>
void igs38_spi_device::register_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_SPI, "%s: write %02x=%08x mask=%08x\n", machine().describe_context(), Register * 4, data, mem_mask);
	u32 const written = data & mem_mask;
	if constexpr (Register == 0)
	{
		if (m_tx_count < 8 && (mem_mask & 0xffff))
		{
			m_tx[(m_tx_head + m_tx_count) & 7] = written;
			++m_tx_count;
		}
	}
	else if constexpr (Register == 2)
	{
		if (written & 4)
			m_tx_head = m_tx_count = 0;
		if (written & 8)
		{
			m_rx_head = m_rx_count = 0;
			m_regs[8] &= ~2U;
		}
		mem_mask &= 0x3f00;
		COMBINE_DATA(&m_regs[Register]);
	}
	else if constexpr (Register == 3)
	{
		if (written & 0x800)
		{
			device_reset();
			return;
		}
		u32 const old = m_regs[3];
		mem_mask &= 0x777d;
		COMBINE_DATA(&m_regs[3]);
		if ((m_regs[3] & 0x1600) == 0x1600 && !(old & 0x200))
		{
			m_tx_remaining = m_regs[5];
			m_rx_remaining = m_regs[6];
			m_regs[8] &= ~1U;
			LOGMASKED(LOG_SPI, "%s: start tx=%u rx=%u select=%u bits=%u control=%04x\n", machine().describe_context(), m_tx_remaining, m_rx_remaining, (m_regs[7] >> 8) & 7,
					((m_regs[7] >> 11) & 15) + 1, m_regs[3]);
		}
	}
	else if constexpr (Register == 8)
	{
		m_regs[8] &= ~(written & 0xfa0);
	}
	else
	{
		mem_mask &= (Register == 1) ? 0xfff : (Register == 4) ? 0x73f : (Register == 7) ? 0x7f3f : 0xffff;
		COMBINE_DATA(&m_regs[Register]);
	}
	update_select();
	update_irq();
	schedule();
}

void igs38_spi_device::schedule()
{
	if ((m_regs[3] & 0x1600) != 0x1600)
	{
		m_timer->adjust(attotime::never);
		return;
	}
	bool const simultaneous = BIT(m_regs[3], 5);
	if (m_tx_remaining && !m_tx_count)
		return;
	if (m_rx_remaining && (simultaneous || !m_tx_remaining) && m_rx_count == 8)
		return;
	if (!m_timer->enabled() || m_timer->expire().is_never())
		m_timer->adjust(attotime::from_usec(1));
}

TIMER_CALLBACK_MEMBER(igs38_spi_device::transfer)
{
	bool const tx = m_tx_remaining != 0;
	bool const rx = m_rx_remaining && (BIT(m_regs[3], 5) || !tx);
	u32 const bits = ((m_regs[7] >> 11) & 15) + 1;
	u16 const mask = util::make_bitmask<u16>(bits);
	u16 outgoing = mask;
	if (tx)
	{
		if (!m_tx_count)
			return;
		outgoing = m_tx[m_tx_head] & mask;
		m_tx_head = (m_tx_head + 1) & 7;
		--m_tx_count;
		--m_tx_remaining;
	}
	if (tx || rx)
	{
		m_transmit(outgoing);
		u16 const incoming = BIT(m_regs[3], 0) ? outgoing : m_receive() & mask;
		if (rx)
		{
			if (m_rx_count == 8)
			{
				m_regs[8] |= 2;
				return;
			}
			m_rx[(m_rx_head + m_rx_count) & 7] = incoming;
			++m_rx_count;
			if (!--m_rx_remaining)
				m_regs[8] |= 1;
		}
	}
	if (!m_tx_remaining && !m_rx_remaining)
	{
		m_regs[3] &= ~0x200U;
		update_select();
	}
	update_irq();
	schedule();
}
