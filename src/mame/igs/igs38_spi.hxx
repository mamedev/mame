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
	auto select_callback() { return m_select.bind(); }
	void map(address_map &map) ATTR_COLD { map(0, 0x47).rw(FUNC(igs38_spi_device::read), FUNC(igs38_spi_device::write)); }

protected:
	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;

private:
	u32 read(offs_t offset);
	void write(offs_t offset, u32 data, u32 mem_mask = ~0U);
	u32 status() const;
	void update_irq();
	void schedule();
	TIMER_CALLBACK_MEMBER(transfer);
	devcb_write_line m_irq;
	devcb_write16 m_transmit;
	devcb_read16 m_receive;
	devcb_write8 m_select;
	emu_timer *m_timer = nullptr;
	u32 m_regs[18]{};
	u16 m_tx[8]{}, m_rx[8]{};
	u8 m_tx_head = 0, m_tx_count = 0, m_rx_head = 0, m_rx_count = 0;
	u32 m_tx_remaining = 0, m_rx_remaining = 0;
};
DECLARE_DEVICE_TYPE(IGS38_SPI, igs38_spi_device)
DEFINE_DEVICE_TYPE(IGS38_SPI, igs38_spi_device, "igs38_spi", "IGS38 SPI controller")

igs38_spi_device::igs38_spi_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: device_t(mconfig, IGS38_SPI, tag, owner, clock)
		, m_irq(*this)
		, m_transmit(*this)
		, m_receive(*this, 0xffff)
		, m_select(*this)
{
}

void igs38_spi_device::device_start()
{
	m_timer = timer_alloc(FUNC(igs38_spi_device::transfer), this);
	save_item(NAME(m_regs));
	save_item(NAME(m_tx));
	save_item(NAME(m_rx));
	save_item(NAME(m_tx_head));
	save_item(NAME(m_tx_count));
	save_item(NAME(m_rx_head));
	save_item(NAME(m_rx_count));
	save_item(NAME(m_tx_remaining));
	save_item(NAME(m_rx_remaining));
	machine().save().register_postload(save_prepost_delegate(FUNC(igs38_spi_device::update_irq), this));
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
	m_select(3);
	update_irq();
}

u32 igs38_spi_device::status() const
{
	unsigned const rx_level = std::min(8U, 2U * (1 + ((m_regs[2] >> 11) & 7)));
	unsigned const tx_level = std::min(8U, 2U * (1 + ((m_regs[2] >> 8) & 7)));
	return (m_regs[8] & 0xfa3) | (m_tx_count == 0 ? 0x40 : 0) | (m_rx_count ? 0x10 : 0) | (8 - m_tx_count >= tx_level ? 8 : 0) |
			(m_rx_count >= rx_level ? 4 : 0);
}

void igs38_spi_device::update_irq()
{
	m_irq(bool(status() & m_regs[1]));
}

u32 igs38_spi_device::read(offs_t offset)
{
	if (offset == 0)
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
	if (offset == 2)
		return (m_regs[2] & 0x3f00) | (m_tx_count == 8 ? 2 : 0) | (m_rx_count ? 1 : 0);
	if (offset == 8)
	{
		u32 const value = status();
		if (!machine().side_effects_disabled())
		{
			m_regs[8] &= ~1U;
			update_irq();
		}
		return value;
	}
	if (offset == 9)
		return (m_tx_count == 0 ? 1 : 0) | (m_tx_count >= 4 ? 2 : 0) | (m_tx_count == 8 ? 4 : 0) | (m_rx_count == 0 ? 0x10 : 0) |
				(m_rx_count >= 4 ? 0x20 : 0) | (m_rx_count == 8 ? 0x40 : 0);
	return m_regs[offset];
}

void igs38_spi_device::write(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_SPI, "write %02x=%08x mask=%08x\n", offset * 4, data, mem_mask);
	u32 const written = data & mem_mask;
	if (offset == 0)
	{
		if (m_tx_count < 8 && (mem_mask & 0xffff))
		{
			m_tx[(m_tx_head + m_tx_count) & 7] = written;
			++m_tx_count;
		}
	}
	else if (offset == 2)
	{
		if (written & 4)
			m_tx_head = m_tx_count = 0;
		if (written & 8)
		{
			m_rx_head = m_rx_count = 0;
			m_regs[8] &= ~2U;
		}
		mem_mask &= 0x3f00;
		COMBINE_DATA(&m_regs[offset]);
	}
	else if (offset == 3)
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
			LOGMASKED(LOG_SPI, "start tx=%u rx=%u select=%u bits=%u control=%04x\n", m_tx_remaining, m_rx_remaining, (m_regs[7] >> 8) & 7,
					((m_regs[7] >> 11) & 15) + 1, m_regs[3]);
		}
		unsigned const selected = (m_regs[7] >> 8) & 7;
		bool const active = BIT(m_regs[3], 14) ? !BIT(m_regs[3], 13) : BIT(m_regs[3], 9);
		m_select(active && selected < 2 ? (3U & ~(1U << selected)) : 3);
	}
	else if (offset == 8)
		m_regs[8] &= ~(written & 0xfa0);
	else if (offset == 9 || offset >= 14)
		return;
	else
	{
		mem_mask &= offset == 1 ? 0xfff : offset == 4 ? 0x73f : offset == 7 ? 0x7f3f : 0xffff;
		COMBINE_DATA(&m_regs[offset]);
	}
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
	unsigned const bits = ((m_regs[7] >> 11) & 15) + 1;
	u16 const mask = (u32(1) << bits) - 1;
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
		if (!BIT(m_regs[3], 14))
			m_select(3);
	}
	update_irq();
	schedule();
}
