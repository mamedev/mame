// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
// IGS38 I2S playback.
// Serial master, 8/16-bit mono/stereo, FIFO/interrupt/DMA handshakes.
// FIFO implementation depth is configurable in hardware and unverified here.
// Recording, slave clocks and serial pin timing are not yet modelled.
class igs38_i2s_device : public device_t, public device_sound_interface
{
public:
	igs38_i2s_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 11289600);

	auto irq_callback() { return m_irq.bind(); }
	auto dma_callback() { return m_request.bind(); }
	auto gain_callback() { return m_gain.bind(); }
	void map(address_map &map) ATTR_COLD { map(0, 0x1f).rw(FUNC(igs38_i2s_device::read), FUNC(igs38_i2s_device::write)); }

protected:
	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;
	void sound_stream_update(sound_stream &stream) override;

private:
	u32 read(offs_t offset);
	void write(offs_t offset, u32 data, u32 mem_mask = ~0U);
	u32 status() const;
	void update_lines();
	void update_rate();
	TIMER_CALLBACK_MEMBER(sample);
	devcb_write_line m_irq;
	devcb_write8 m_request;
	devcb_read32 m_gain;
	sound_stream *m_stream = nullptr;
	emu_timer *m_timer = nullptr;
	static constexpr unsigned FIFO_SIZE = 32;
	u32 m_regs[8]{}, m_fifo[FIFO_SIZE]{};
	u8 m_head = 0, m_count = 0, m_byte = 0;
	s16 m_output[2]{};
};
DECLARE_DEVICE_TYPE(IGS38_I2S, igs38_i2s_device)
DEFINE_DEVICE_TYPE(IGS38_I2S, igs38_i2s_device, "igs38_i2s", "IGS38 I2S audio controller")

igs38_i2s_device::igs38_i2s_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: device_t(mconfig, IGS38_I2S, tag, owner, clock)
		, device_sound_interface(mconfig, *this)
		, m_irq(*this)
		, m_request(*this)
		, m_gain(*this, 0x10001000)
{
}

void igs38_i2s_device::device_start()
{
	m_stream = stream_alloc(0, 2, 22050);
	m_timer = timer_alloc(FUNC(igs38_i2s_device::sample), this);
	save_item(NAME(m_regs));
	save_item(NAME(m_fifo));
	save_item(NAME(m_head));
	save_item(NAME(m_count));
	save_item(NAME(m_byte));
	save_item(NAME(m_output));
	machine().save().register_postload(save_prepost_delegate(FUNC(igs38_i2s_device::update_lines), this));
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

u32 igs38_i2s_device::status() const
{
	return (m_regs[5] & 0xf0000) | 0x11 | (m_count == 0 ? 4 : 0) | (m_count == FIFO_SIZE ? 8 : 0) | (m_count <= 1 ? 0x40 : 0) |
			(m_count >= FIFO_SIZE - 1 ? 0x80 : 0) | (m_count >= FIFO_SIZE / 2 ? 0x200 : 0);
}

void igs38_i2s_device::update_lines()
{
	unsigned const level = (m_regs[5] >> 18) & 3;
	unsigned const threshold = level == 0 ? 1 : level == 1 ? FIFO_SIZE / 2 : FIFO_SIZE - 1;
	m_regs[7] = m_count <= threshold ? 4 : 0;
	m_irq(bool(m_regs[7] & m_regs[6]));
	
	for (unsigned i = 0; i < 2; ++i)
		m_request(i, !BIT(m_regs[0], 6 - i) && !BIT(m_regs[0], 4 - i) && m_count < FIFO_SIZE);
}

void igs38_i2s_device::update_rate()
{
	unsigned const ratio = (m_regs[3] >> 8) & 0xff, oversample = (m_regs[3] >> 16) & 3;
	if (!ratio || oversample == 3 || !BIT(m_regs[0], 1) || !BIT(m_regs[3], 0))
	{
		m_timer->adjust(attotime::never);
		m_output[0] = m_output[1] = 0;
		return;
	}
	unsigned const divider = ratio * (32U << oversample);
	m_stream->set_sample_rate(std::max(1U, clock() / divider));
	attotime const period = attotime::from_ticks(divider, clock());
	m_timer->adjust(period, 0, period);
}

u32 igs38_i2s_device::read(offs_t offset)
{
	if (offset == 1 || offset == 2)
		return 0;
	if (offset == 5)
		return status();
	return m_regs[offset];
}

void igs38_i2s_device::write(offs_t offset, u32 data, u32 mem_mask)
{
	if (offset == 1)
	{
		if (m_count < FIFO_SIZE)
		{
			m_fifo[(m_head + m_count) % FIFO_SIZE] = data & mem_mask;
			++m_count;
		}
		update_lines();
		return;
	}
	if (offset == 2 || offset == 7)
		return;
	m_stream->update();
	if (offset == 0)
	{
		if (data & mem_mask & 0x20000)
		{
			m_head = m_count = m_byte = 0;
			m_output[0] = m_output[1] = 0;
		}
		mem_mask &= 0x7f;
	}
	if (offset == 3 || offset == 4)
		mem_mask &= 0x3ff3f;
	if (offset == 5)
		mem_mask &= 0xf0000;
	if (offset == 6)
		mem_mask &= 7;
	COMBINE_DATA(&m_regs[offset]);
	if (offset == 0 || offset == 3)
		update_rate();
	update_lines();
	LOGMASKED(LOG_AUDIO, "write %02x=%08x\n", offset * 4, m_regs[offset]);
}

TIMER_CALLBACK_MEMBER(igs38_i2s_device::sample)
{
	m_stream->update();
	if (m_count)
	{
		unsigned const bytes = BIT(m_regs[3], 4) ? 2 : 1;
		unsigned const channels = BIT(m_regs[3], 3) ? 1 : 2;
		for (unsigned i = 0; i < channels; ++i)
		{
			u32 const v = m_fifo[m_head] >> (8 * m_byte);
			m_output[i] = bytes == 2 ? s16(v) : s16(s8(v) * 256);
			m_byte += bytes;
		}
		if (channels == 1)
			m_output[1] = 0;
		if (m_byte == 4)
		{
			m_byte = 0;
			m_head = (m_head + 1) % FIFO_SIZE;
			--m_count;
		}
	}
	else
		m_output[0] = m_output[1] = 0;
	update_lines();
}

void igs38_i2s_device::sound_stream_update(sound_stream &stream)
{
	u32 const gains = m_gain();
	for (unsigned ch = 0; ch < 2; ++ch)
		stream.fill(ch, double(m_output[ch]) * ((gains >> (16 * ch)) & 0xffff) / (32768.0 * 4096.0));
}
