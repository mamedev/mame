// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
// IGS38 internal timer.
// Three instances share PCLK and drive VIC0 inputs 4, 5 and 6.
class igs38_timer_device : public device_t
{
public:
	igs38_timer_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	auto irq_callback() { return m_irq.bind(); }

	void map(address_map &map);
	void set_pclk(u32 hz);

protected:
	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;

private:
	u32 read(offs_t offset);
	void write(offs_t offset, u32 data, u32 mem_mask = ~0U);
	u32 rate() const;
	u64 remaining() const;
	void schedule();
	void update_irq();
	TIMER_CALLBACK_MEMBER(expired);
	TIMER_CALLBACK_MEMBER(pulse_end);
	devcb_write_line m_irq;
	emu_timer *m_expiry = nullptr, *m_pulse = nullptr;
	u32 m_load = 0, m_control = 2, m_pclk = 0;
	u64 m_ticks_left = 0;
	attotime m_started = attotime::zero;
	bool m_pending = false, m_edge_active = false;
};

DEFINE_DEVICE_TYPE(IGS38_TIMER, igs38_timer_device, "igs38_timer", "IGS38 internal timer")

igs38_timer_device::igs38_timer_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: device_t(mconfig, IGS38_TIMER, tag, owner, clock)
		, m_irq(*this)
{
}

void igs38_timer_device::map(address_map &map)
{
	map(0x00, 0x0f).rw(FUNC(igs38_timer_device::read), FUNC(igs38_timer_device::write));
}

void igs38_timer_device::device_start()
{
	m_expiry = timer_alloc(FUNC(igs38_timer_device::expired), this);
	m_pulse = timer_alloc(FUNC(igs38_timer_device::pulse_end), this);
	save_item(NAME(m_load));
	save_item(NAME(m_control));
	save_item(NAME(m_pclk));
	save_item(NAME(m_ticks_left));
	save_item(NAME(m_started));
	save_item(NAME(m_pending));
	save_item(NAME(m_edge_active));
	machine().save().register_postload(save_prepost_delegate(FUNC(igs38_timer_device::update_irq), this));
}

void igs38_timer_device::device_reset()
{
	m_load = 0;
	m_control = 2;
	m_ticks_left = 0;
	m_started = machine().time();
	m_pending = m_edge_active = false;
	m_expiry->adjust(attotime::never);
	m_pulse->adjust(attotime::never);
	update_irq();
}

u32 igs38_timer_device::rate() const
{
	if (!BIT(m_control, 8) || BIT(m_control, 10))
		return 0;
	unsigned const prescale = (m_control >> 4) & 7;
	return m_pclk / (prescale ? (2U << prescale) : 1);
}

u64 igs38_timer_device::remaining() const
{
	if (!rate())
		return m_ticks_left;
	u64 const elapsed = (machine().time() - m_started).as_ticks(rate());
	return m_ticks_left - std::min(m_ticks_left, elapsed);
}

void igs38_timer_device::schedule()
{
	m_started = machine().time();
	m_expiry->adjust(rate() ? attotime::from_ticks(std::max<u64>(m_ticks_left, 1), rate()) : attotime::never);
}

void igs38_timer_device::set_pclk(u32 hz)
{
	if (hz == m_pclk)
		return;
	m_ticks_left = remaining();
	m_pclk = hz;
	schedule();
}

void igs38_timer_device::update_irq()
{
	m_irq(!BIT(m_control, 3) && (BIT(m_control, 1) ? m_pending : m_edge_active));
}

TIMER_CALLBACK_MEMBER(igs38_timer_device::expired)
{
	m_pending = true;
	if (!BIT(m_control, 1) && !BIT(m_control, 3))
	{
		m_edge_active = true;
		m_pulse->adjust(attotime::from_ticks(1, m_pclk));
	}

	m_ticks_left = BIT(m_control, 7) ? m_load : (u64(1) << 32);
	schedule();
	update_irq();
}

TIMER_CALLBACK_MEMBER(igs38_timer_device::pulse_end)
{
	m_edge_active = false;
	update_irq();
}

u32 igs38_timer_device::read(offs_t offset)
{
	switch (offset)
	{
	case 1:
		return u32(remaining());
	case 2:
		return m_control | (m_pending ? 4 : 0);
	default:
		return 0; // load is write-only; +0c is reserved
	}
}

void igs38_timer_device::write(offs_t offset, u32 data, u32 mem_mask)
{
	if (offset == 0)
	{
		COMBINE_DATA(&m_load);
		m_ticks_left = m_load;
		schedule();
	}
	else if (offset == 2)
	{
		u32 const old_control = m_control;
		u32 const new_control = (m_control & ~(mem_mask & 0x5fa)) | (data & mem_mask & 0x5fa);
		
		if ((old_control ^ new_control) & 0x5f0)
			m_ticks_left = remaining();
		m_control = new_control;
		if ((old_control ^ new_control) & 0x5f0)
			schedule();
		
		if (mem_mask & ~data & 4)
		{
			m_pending = m_edge_active = false;
			m_pulse->adjust(attotime::never);
		}
		update_irq();
	}
}
