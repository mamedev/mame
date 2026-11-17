// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
// IGS38 I2S playback.
// Serial master, 8/16-bit mono/stereo, FIFO/interrupt/DMA handshakes.
// FIFO implementation depth is configurable in hardware and unverified here.
// Recording, slave clocks and serial pin timing are not yet modelled.
class igs38_i2s_device : public device_t, public device_sound_interface
{
public:
	igs38_i2s_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	auto irq_callback() { return m_irq.bind(); }
	template <u32 Line> auto dma_callback() { return m_request[Line].bind(); }
	void map(address_map &map) ATTR_COLD;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_sound_interface implementation
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	static constexpr u32 FIFO_SIZE = 32;

	template <u32 Register> u32 register_r() { return m_regs[Register]; }
	template <u32 Register> void register_w(offs_t offset, u32 data, u32 mem_mask = ~0U);
	void fifo_w(offs_t offset, u32 data, u32 mem_mask = ~0U);
	u32 status_r();
	void update_lines();
	void update_rate();
	void advance_word();
	TIMER_CALLBACK_MEMBER(sample);

	devcb_write_line m_irq;
	devcb_write_line::array<2> m_request;
	sound_stream *m_stream;
	emu_timer *m_timer;
	u32 m_regs[8], m_fifo[FIFO_SIZE];
	u8 m_head, m_count, m_byte;
	s16 m_output[2];
	bool m_irq_state, m_request_state[2];
};

DEFINE_DEVICE_TYPE_PRIVATE(IGS38_I2S, igs38_i2s_device, igs38_i2s_device, "igs38_i2s", "IGS38 I2S audio controller")

igs38_i2s_device::igs38_i2s_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, IGS38_I2S, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_irq(*this)
	, m_request(*this)
{
}

void igs38_i2s_device::map(address_map &map)
{
	map(0x00, 0x03).rw(FUNC(igs38_i2s_device::register_r<0>), FUNC(igs38_i2s_device::register_w<0>));
	map(0x04, 0x07).r(FUNC(igs38_i2s_device::register_r<1>)).w(FUNC(igs38_i2s_device::fifo_w));
	map(0x08, 0x0b).r(FUNC(igs38_i2s_device::register_r<2>)).nopw();
	map(0x0c, 0x0f).rw(FUNC(igs38_i2s_device::register_r<3>), FUNC(igs38_i2s_device::register_w<3>));
	map(0x10, 0x13).rw(FUNC(igs38_i2s_device::register_r<4>), FUNC(igs38_i2s_device::register_w<4>));
	map(0x14, 0x17).rw(FUNC(igs38_i2s_device::status_r), FUNC(igs38_i2s_device::register_w<5>));
	map(0x18, 0x1b).rw(FUNC(igs38_i2s_device::register_r<6>), FUNC(igs38_i2s_device::register_w<6>));
	map(0x1c, 0x1f).r(FUNC(igs38_i2s_device::register_r<7>)).nopw();
}

void igs38_i2s_device::device_start()
{
	m_stream = stream_alloc(0, 2, 22'050);
	m_timer = timer_alloc(FUNC(igs38_i2s_device::sample), this);
	m_irq_state = false;
	std::fill(std::begin(m_request_state), std::end(m_request_state), false);
	save_item(NAME(m_regs));
	save_item(NAME(m_fifo));
	save_item(NAME(m_head));
	save_item(NAME(m_count));
	save_item(NAME(m_byte));
	save_item(NAME(m_output));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_request_state));
}

void igs38_i2s_device::device_reset()
{
	std::fill(std::begin(m_regs), std::end(m_regs), 0);
	std::fill(std::begin(m_fifo), std::end(m_fifo), 0);
	m_regs[0] = 0x18;
	m_regs[3] = m_regs[4] = 0x10811;
	m_regs[5] = 0x10000;
	m_head = m_count = m_byte = 0;
	m_output[0] = m_output[1] = 0;
	update_rate();
	update_lines();
}

u32 igs38_i2s_device::status_r()
{
	return (m_regs[5] & 0xf0000) | 0x11 | ((m_count == 0) ? 4 : 0) | ((m_count == FIFO_SIZE) ? 8 : 0) | ((m_count <= 1) ? 0x40 : 0) |
			((m_count >= FIFO_SIZE - 1) ? 0x80 : 0) | ((m_count >= FIFO_SIZE / 2) ? 0x200 : 0);
}

void igs38_i2s_device::update_lines()
{
	u32 const level = BIT(m_regs[5], 18, 2);
	u32 const threshold = (level == 0) ? 1 : (level == 1) ? FIFO_SIZE / 2 : FIFO_SIZE - 1;
	m_regs[7] = (m_count <= threshold) ? 4 : 0;
	bool const irq = bool(m_regs[7] & m_regs[6]);
	if (m_irq_state != irq)
	{
		m_irq_state = irq;
		m_irq(irq);
	}

	for (u32 i = 0; i < 2; ++i)
	{
		bool const request = !BIT(m_regs[0], 6 - i) && !BIT(m_regs[0], 4 - i) && (m_count < FIFO_SIZE);
		if (m_request_state[i] != request)
		{
			m_request_state[i] = request;
			m_request[i](request);
		}
	}
}

void igs38_i2s_device::update_rate()
{
	u32 const ratio = BIT(m_regs[3], 8, 8), oversample = BIT(m_regs[3], 16, 2);
	if (!ratio || (oversample == 3) || !BIT(m_regs[0], 1) || !BIT(m_regs[3], 0))
	{
		m_timer->adjust(attotime::never);
		m_output[0] = m_output[1] = 0;
		return;
	}
	u32 const divider = ratio * (32U << oversample);
	m_stream->set_sample_rate(std::max(1U, clock() / divider));
	attotime const period = attotime::from_ticks(divider, clock());
	m_timer->adjust(period, 0, period);
}

void igs38_i2s_device::fifo_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (m_count < FIFO_SIZE)
	{
		m_fifo[(m_head + m_count) % FIFO_SIZE] = data & mem_mask;
		++m_count;
	}
	update_lines();
}

template <u32 Register>
void igs38_i2s_device::register_w(offs_t offset, u32 data, u32 mem_mask)
{
	m_stream->update();
	if constexpr (Register == 0)
	{
		if (data & mem_mask & 0x20000)
		{
			m_head = m_count = m_byte = 0;
			m_output[0] = m_output[1] = 0;
		}
		mem_mask &= 0x7f;
	}
	else if constexpr ((Register == 3) || (Register == 4))
	{
		mem_mask &= 0x3ff3f;
	}
	else if constexpr (Register == 5)
	{
		mem_mask &= 0xf0000;
	}
	else if constexpr (Register == 6)
	{
		mem_mask &= 7;
	}
	COMBINE_DATA(&m_regs[Register]);
	if constexpr ((Register == 0) || (Register == 3))
		update_rate();
	update_lines();
	LOGMASKED(LOG_AUDIO, "%s: write %02x=%08x\n", machine().describe_context(), Register * 4, m_regs[Register]);
}

void igs38_i2s_device::advance_word()
{
	m_byte = 0;
	m_head = (m_head + 1) % FIFO_SIZE;
	--m_count;
}

TIMER_CALLBACK_MEMBER(igs38_i2s_device::sample)
{
	m_stream->update();
	u32 const bytes = BIT(m_regs[3], 4) ? 2 : 1;
	u32 const channels = BIT(m_regs[3], 3) ? 1 : 2;
	m_output[0] = m_output[1] = 0;
	for (u32 i = 0; i < channels; ++i)
	{
		// A format change can leave fewer than one sample in the current word.
		if (m_count && (m_byte + bytes > 4))
			advance_word();
		if (!m_count)
			break;
		u32 const value = m_fifo[m_head] >> (8 * m_byte);
		m_output[i] = (bytes == 2) ? s16(value) : s16(s8(value) * 256);
		m_byte += bytes;
		if (m_byte >= 4)
			advance_word();
	}
	update_lines();
}

void igs38_i2s_device::sound_stream_update(sound_stream &stream)
{
	for (u32 ch = 0; ch < 2; ++ch)
		stream.fill(ch, double(m_output[ch]) / 32768.0);
}
