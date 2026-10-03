// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
// IGS38 APB I2C master.
// Byte-level bus: timing includes SCL clocks, but not individual wire edges,
// multi-master arbitration, external clock stretching or slave mode.
class igs38_i2c_device : public device_t
{
public:
	igs38_i2c_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);
	void map(address_map &map);
	void set_pclk(u32 hz);

	auto irq_callback() { return m_irq.bind(); }

	// TX offset: bit 8 marks an address phase, bits 7:0 are the byte.
	// Return 0 for ACK, 1 for NACK. An unconnected bus NACKs all writes.
	auto transmit_callback() { return m_transmit.bind(); }
	auto receive_callback() { return m_receive.bind(); }
	auto stop_callback() { return m_stop.bind(); }
	auto acknowledge_callback() { return m_acknowledge.bind(); }

protected:
	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;

private:
	u32 read(offs_t offset);
	void write(offs_t offset, u32 data, u32 mem_mask = ~0U);
	void reset_bus();
	void command(u8 data);
	void schedule();
	void update_irq();
	u64 remaining() const;
	TIMER_CALLBACK_MEMBER(complete);
	devcb_write_line m_irq, m_stop, m_acknowledge;
	devcb_read8 m_transmit, m_receive;
	emu_timer *m_timer = nullptr;
	u32 m_regs[11]{};
	u32 m_pclk = 0;
	u64 m_clocks_left = 0;
	attotime m_started = attotime::zero;
	u8 m_command = 0, m_data = 0;
	bool m_active = false, m_tx = false, m_ack_pending = false;
};

DEFINE_DEVICE_TYPE(IGS38_I2C, igs38_i2c_device, "igs38_i2c", "IGS38 I2C master")

igs38_i2c_device::igs38_i2c_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: device_t(mconfig, IGS38_I2C, tag, owner, clock)
		, m_irq(*this)
		, m_stop(*this)
		, m_acknowledge(*this)
		, m_transmit(*this, 1)
		, m_receive(*this, 0xff)
{
}

void igs38_i2c_device::map(address_map &map)
{
	map(0x00, 0x2b).rw(FUNC(igs38_i2c_device::read), FUNC(igs38_i2c_device::write));
}

void igs38_i2c_device::device_start()
{
	m_timer = timer_alloc(FUNC(igs38_i2c_device::complete), this);
	save_item(NAME(m_regs));
	save_item(NAME(m_pclk));
	save_item(NAME(m_clocks_left));
	save_item(NAME(m_started));
	save_item(NAME(m_command));
	save_item(NAME(m_data));
	save_item(NAME(m_active));
	save_item(NAME(m_tx));
	save_item(NAME(m_ack_pending));
	machine().save().register_postload(save_prepost_delegate(FUNC(igs38_i2c_device::update_irq), this));
}

void igs38_i2c_device::device_reset()
{
	std::fill(std::begin(m_regs), std::end(m_regs), 0);
	m_regs[4] = 0x3ff;
	reset_bus();
}

void igs38_i2c_device::reset_bus()
{
	m_timer->adjust(attotime::never);
	m_active = m_ack_pending = false;
	m_command = 0;
	m_clocks_left = 0;
	m_started = machine().time();
	m_regs[6] = m_regs[7] = m_regs[8] = 0;
	m_stop(1);
	update_irq();
}

void igs38_i2c_device::update_irq()
{
	m_irq(bool(m_regs[5] & m_regs[6] & 0xff));
}

u64 igs38_i2c_device::remaining() const
{
	if (!m_active || !m_pclk)
		return m_clocks_left;
	u64 const elapsed = (machine().time() - m_started).as_ticks(m_pclk);
	return m_clocks_left - std::min(m_clocks_left, elapsed);
}

void igs38_i2c_device::schedule()
{
	m_started = machine().time();
	m_timer->adjust(m_active && m_pclk ? attotime::from_ticks(std::max<u64>(m_clocks_left, 1), m_pclk) : attotime::never);
}

void igs38_i2c_device::set_pclk(u32 hz)
{
	if (hz != m_pclk)
	{
		m_clocks_left = remaining();
		m_pclk = hz;
		schedule();
	}
}

void igs38_i2c_device::command(u8 data)
{
	if (m_active || !(m_regs[10] & 0x40) || (m_regs[10] & 0x80) || !(m_regs[9] & 4))
		return;
	// START includes the address byte. Subsequent actions require RESUME;
	// clearing ISR alone never clocks another byte.
	if (!(data & 1) && (!(data & 4) || !(m_regs[8] & 1)))
		return;
	if ((m_regs[8] & 1) && !(data & 4))
		return;
	m_command = (data & 7) | (m_ack_pending ? 0x40 : 0) | (BIT(m_regs[9], 4) ? 0x80 : 0);
	m_data = m_regs[0];
	m_tx = BIT(m_regs[9], 3);
	m_regs[7] = m_command & 7;
	if (data & 1)
		m_regs[8] |= 1;
	unsigned const divisor = (((m_regs[10] >> 3) & 7) + 1) << ((m_regs[10] & 7) + 1);
	unsigned const clocks = ((data & 2) ? 1 : ((data & 1) || m_tx ? 9 : 8)) + (m_ack_pending ? 1 : 0);
	m_clocks_left = u64(clocks) * 5 * divisor;
	m_active = true;
	schedule();
}

TIMER_CALLBACK_MEMBER(igs38_i2c_device::complete)
{
	m_active = false;
	m_regs[7] = 0;
	if (m_command & 0x40)
		m_acknowledge(!(m_command & 0x80));
	m_ack_pending = false;
	if (m_command & 2)
	{
		m_stop(1);
		m_regs[8] &= ~1U;
	}
	else if ((m_command & 1) || m_tx)
	{
		bool const nack = m_transmit(m_data | ((m_command & 1) ? 0x100 : 0)) != 0;
		m_regs[8] = (m_regs[8] & ~2U) | (nack ? 2 : 0);
		m_regs[6] |= 1;
	}
	else
	{
		m_regs[1] = m_receive(0);
		m_ack_pending = true; // software chooses ACK/NAK before RESUME
		m_regs[6] |= 2;
	}
	update_irq();
}

u32 igs38_i2c_device::read(offs_t offset)
{
	return m_regs[offset];
}

void igs38_i2c_device::write(offs_t offset, u32 data, u32 mem_mask)
{
	static constexpr u32 MASKS[11] = { 0xff, 0, 0xff, 0, 0x3ff, 0xff, 0xff, 7, 0, 0x1f, 0x1ff };
	mem_mask &= MASKS[offset];
	if (offset == 6)
		m_regs[6] &= data | ~mem_mask; // write zero to clear
	else if (offset == 7)
	{
		command(data & mem_mask);
		return;
	}
	else
	{
		COMBINE_DATA(&m_regs[offset]);
		if ((offset == 10 && ((m_regs[10] & 0xc0) != 0x40)) || (offset == 9 && !(m_regs[9] & 4)))
			reset_bus();
	}
	update_irq();
}
