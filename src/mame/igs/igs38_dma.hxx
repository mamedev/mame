// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
// IGS38 PL080 DMA.
// Memory transfers and DMA-controlled peripheral requests, with linked lists.
// Bus arbitration/FIFO timing is approximate. Peripheral-controlled lengths,
//
// TODO: unequal source/destination widths, and big-endian AHB masters
class igs38_dma_device : public device_t
{
public:
	igs38_dma_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 100000000);

	template <typename T> void set_dma_space(T &&tag, int space) { m_dma.set_tag(std::forward<T>(tag), space); }
	auto irq_callback() { return m_irq.bind(); }

	void request_w(offs_t request, u8 state);

	void map(address_map &map) ATTR_COLD { map(0, 0xfff).rw(FUNC(igs38_dma_device::read), FUNC(igs38_dma_device::write)); }

protected:
	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;

private:
	u32 read(offs_t offset);
	void write(offs_t offset, u32 data, u32 mem_mask = ~0U);
	u32 masked_status(bool error) const;
	void update_irq();
	bool ready(unsigned ch) const;
	void schedule();
	void complete(unsigned ch);
	TIMER_CALLBACK_MEMBER(transfer);
	required_address_space m_dma;
	devcb_write_line m_irq;
	emu_timer *m_timer = nullptr;
	u32 m_channels[8][5]{};
	u32 m_config = 0, m_sync = 0;
	u16 m_requests = 0, m_soft[4]{};
	u8 m_terminal = 0, m_errors = 0;
};
DECLARE_DEVICE_TYPE(IGS38_DMA, igs38_dma_device)
DEFINE_DEVICE_TYPE(IGS38_DMA, igs38_dma_device, "igs38_dma", "IGS38 PL080 DMA")

igs38_dma_device::igs38_dma_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: device_t(mconfig, IGS38_DMA, tag, owner, clock)
		, m_dma(*this, finder_base::DUMMY_TAG, -1)
		, m_irq(*this)
{
}

void igs38_dma_device::device_start()
{
	m_timer = timer_alloc(FUNC(igs38_dma_device::transfer), this);
	save_item(NAME(m_channels));
	save_item(NAME(m_config));
	save_item(NAME(m_sync));
	save_item(NAME(m_requests));
	save_item(NAME(m_soft));
	save_item(NAME(m_terminal));
	save_item(NAME(m_errors));
	machine().save().register_postload(save_prepost_delegate(FUNC(igs38_dma_device::update_irq), this));
}

void igs38_dma_device::device_reset()
{
	for (auto &ch : m_channels)
		std::fill(std::begin(ch), std::end(ch), 0);
	std::fill(std::begin(m_soft), std::end(m_soft), 0);
	m_config = m_sync = m_terminal = m_errors = m_requests = 0;
	m_timer->adjust(attotime::never);
	update_irq();
}

u32 igs38_dma_device::masked_status(bool error) const
{
	u32 mask = 0;
	for (unsigned i = 0; i < 8; ++i)
		if (BIT(m_channels[i][4], error ? 14 : 15))
			mask |= 1U << i;
	return (error ? m_errors : m_terminal) & mask;
}

void igs38_dma_device::update_irq()
{
	m_irq(bool(masked_status(false) | masked_status(true)));
}

u32 igs38_dma_device::read(offs_t offset)
{
	if (offset >= 0x40 && offset < 0x80)
		return (offset & 7) < 5 ? m_channels[(offset - 0x40) / 8][offset & 7] : 0;
	if (offset >= 0x3f8)
	{
		static constexpr u8 ID[] = { 0x80, 0x10, 0x04, 0x0a, 0x0d, 0xf0, 0x05, 0xb1 };
		return ID[offset - 0x3f8];
	}
	switch (offset)
	{
	case 0:
		return masked_status(false) | masked_status(true);
	case 1:
		return masked_status(false);
	case 3:
		return masked_status(true);
	case 5:
		return m_terminal;
	case 6:
		return m_errors;
	case 7:
	{
		u32 bits = 0;
		for (unsigned i = 0; i < 8; ++i)
			bits |= (m_channels[i][4] & 1) << i;
		return bits;
	}
	case 8:
	case 9:
	case 10:
	case 11:
		return m_soft[offset - 8];
	case 12:
		return m_config;
	case 13:
		return m_sync;
	default:
		return 0;
	}
}

void igs38_dma_device::write(offs_t offset, u32 data, u32 mem_mask)
{
	if (offset >= 0x40 && offset < 0x80)
	{
		unsigned const ch = (offset - 0x40) / 8, reg = offset & 7;
		if (reg >= 5)
			return;
		if (reg == 4)
			mem_mask &= 0x5ffff; // Active is read-only; no outstanding FIFO beats.
		COMBINE_DATA(&m_channels[ch][reg]);
		if (reg == 4)
			LOGMASKED(LOG_DMA, "channel %u src=%08x dst=%08x lli=%08x control=%08x config=%08x\n", ch, m_channels[ch][0], m_channels[ch][1],
					m_channels[ch][2], m_channels[ch][3], m_channels[ch][4]);
	}
	else
		switch (offset)
		{
		case 2:
			m_terminal &= ~(data & mem_mask);
			break;
		case 4:
			m_errors &= ~(data & mem_mask);
			break;
		case 8:
		case 9:
		case 10:
		case 11:
			m_soft[offset - 8] |= data & mem_mask;
			break;
		case 12:
			mem_mask &= 7;
			COMBINE_DATA(&m_config);
			break;
		case 13:
			mem_mask &= 0xffff;
			COMBINE_DATA(&m_sync);
			break;
		default:
			return;
		}
	update_irq();
	schedule();
}

void igs38_dma_device::request_w(offs_t request, u8 state)
{
	if (request >= 16)
		return;
	if (state)
		m_requests |= 1U << request;
	else
		m_requests &= ~(1U << request);
	schedule();
}

bool igs38_dma_device::ready(unsigned ch) const
{
	u32 const cfg = m_channels[ch][4];
	if (!(m_config & 1) || !(cfg & 1) || BIT(cfg, 18))
		return false;
	unsigned const flow = (cfg >> 11) & 7;
	u16 const requests = m_requests | m_soft[0] | m_soft[1] | m_soft[2] | m_soft[3];
	return !flow || flow > 3 || (flow == 1 && BIT(requests, (cfg >> 6) & 15)) || (flow == 2 && BIT(requests, (cfg >> 1) & 15)) ||
			(flow == 3 && BIT(requests, (cfg >> 6) & 15) && BIT(requests, (cfg >> 1) & 15));
}

void igs38_dma_device::schedule()
{
	for (unsigned i = 0; i < 8; ++i)
		if (ready(i))
		{
			if (!m_timer->enabled() || m_timer->expire().is_never())
				m_timer->adjust(attotime::from_ticks(1, clock()));
			return;
		}
	m_timer->adjust(attotime::never);
}

void igs38_dma_device::complete(unsigned ch)
{
	auto &r = m_channels[ch];
	if (BIT(r[3], 31))
		m_terminal |= 1U << ch;
	u32 const next = r[2] & ~3U;
	if (next)
		for (unsigned i = 0; i < 4; ++i)
			r[i] = m_dma->read_dword(next + i * 4);
	else
		r[4] &= ~1U;
	update_irq();
}

TIMER_CALLBACK_MEMBER(igs38_dma_device::transfer)
{
	for (unsigned ch = 0; ch < 8; ++ch)
	{
		if (!ready(ch))
			continue;
		auto &r = m_channels[ch];
		unsigned const sw = (r[3] >> 18) & 7, dw = (r[3] >> 21) & 7, flow = (r[4] >> 11) & 7;
		if (sw > 2 || dw != sw || flow > 3 || (m_config & 6))
		{
			logerror("unsupported channel %u control=%08x config=%08x global=%08x\n", ch, r[3], r[4], m_config);
			m_errors |= 1U << ch;
			r[4] &= ~1U;
			update_irq();
			break;
		}
		if (!(r[3] & 0xfff))
		{
			complete(ch);
			break;
		}
		if (!sw)
			m_dma->write_byte(r[1], m_dma->read_byte(r[0]));
		else if (sw == 1)
			m_dma->write_word(r[1], m_dma->read_word(r[0]));
		else
			m_dma->write_dword(r[1], m_dma->read_dword(r[0]));
		if (BIT(r[3], 26))
			r[0] += 1U << sw;
		if (BIT(r[3], 27))
			r[1] += 1U << dw;
		--r[3];
		for (auto &soft : m_soft)
			soft &= ~((1U << ((r[4] >> 1) & 15)) | (1U << ((r[4] >> 6) & 15)));
		if (!(r[3] & 0xfff))
			complete(ch);
		break; // Fixed channel priority, one AHB beat per timer event.
	}
	schedule();
}
