// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
// CAT6613 HDMI control-plane subset. No TMDS/audio/HDCP engine.
class pgm3_cat6613_device : public device_t
{
public:
	pgm3_cat6613_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);
	u8 transmit(offs_t data);
	u8 receive(offs_t offset);

	void stop(int state) { m_selected = false; }

	void acknowledge(int state)
	{
		if (!state && m_read)
			m_selected = false;
	}

	void set_connected(int state);
	void set_video_signal(int state);

	auto hpd_callback() { return m_hpd.bind(); }

	// Normalized interrupt assertion; the PGM3 board uses active-low INT#.
	auto irq_callback() { return m_irq.bind(); }

protected:
	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;

private:
	u8 reg_read(u8 address);
	void reg_write(u8 address, u8 data);
	void reset_registers();
	bool interrupt_active() const;
	bool video_stable() const;
	void update_irq();
	void ddc_command(u8 command);
	TIMER_CALLBACK_MEMBER(ddc_complete);
	devcb_read_line m_hpd;
	devcb_write_line m_irq;
	emu_timer *m_ddc_timer = nullptr;
	u8 m_regs[2][256]{};
	u8 m_fifo[32]{};
	u8 m_fifo_head = 0, m_fifo_count = 0;
	u8 m_pointer = 0;
	u8 m_ddc_command = 0, m_ddc_header = 0, m_ddc_offset = 0, m_ddc_count = 0, m_ddc_segment = 0;
	bool m_selected = false, m_read = false, m_subaddress = false;
	bool m_connected = false, m_input_video = false;
	static const u8 EDID[256];
};

DEFINE_DEVICE_TYPE(PGM3_CAT6613, pgm3_cat6613_device, "pgm3_cat6613", "CAT6613 HDMI transmitter (PGM3 subset)")

pgm3_cat6613_device::pgm3_cat6613_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: device_t(mconfig, PGM3_CAT6613, tag, owner, clock)
		, m_hpd(*this, 1)
		, m_irq(*this)
{
}

void pgm3_cat6613_device::device_start()
{
	m_ddc_timer = timer_alloc(FUNC(pgm3_cat6613_device::ddc_complete), this);
	save_item(NAME(m_regs));
	save_item(NAME(m_fifo));
	save_item(NAME(m_fifo_head));
	save_item(NAME(m_fifo_count));
	save_item(NAME(m_pointer));
	save_item(NAME(m_ddc_command));
	save_item(NAME(m_ddc_header));
	save_item(NAME(m_ddc_offset));
	save_item(NAME(m_ddc_count));
	save_item(NAME(m_ddc_segment));
	save_item(NAME(m_selected));
	save_item(NAME(m_read));
	save_item(NAME(m_subaddress));
	save_item(NAME(m_connected));
	save_item(NAME(m_input_video));
	machine().save().register_postload(save_prepost_delegate(FUNC(pgm3_cat6613_device::update_irq), this));
}

void pgm3_cat6613_device::device_reset()
{
	m_selected = m_read = m_subaddress = false;
	m_pointer = 0;
	m_connected = m_hpd() != 0;
	m_input_video = false;
	reset_registers();
}

void pgm3_cat6613_device::reset_registers()
{
	for (auto &bank : m_regs)
		std::fill(std::begin(bank), std::end(bank), 0);
	m_regs[0][4] = 0x1d;
	m_regs[0][5] = 0x41;
	m_regs[0][9] = m_regs[0][0x0a] = m_regs[0][0x0b] = 0xff;
	m_regs[0][0x0e] = 0x0c;
	m_regs[0][0x61] = 0x10;
	// Sample an already attached receiver after reset, latching the connect event.
	m_regs[0][6] = m_connected ? 3 : 0;
	m_ddc_timer->adjust(attotime::never);
	m_fifo_head = m_fifo_count = 0;
	update_irq();
}

bool pgm3_cat6613_device::interrupt_active() const
{
	return (m_regs[0][6] & ~m_regs[0][9]) || (m_regs[0][7] & 0x3f & ~m_regs[0][0x0a]) ||
			((m_regs[0][7] & 0xc0) && !(m_regs[0][0x0a] & 0x40)) || ((m_regs[0][8] & 1) && !(m_regs[0][0x0a] & 0x80)) ||
			((m_regs[0][8] >> 1) & 0x3f & ~m_regs[0][0x0b]);
}

bool pgm3_cat6613_device::video_stable() const
{
	return m_input_video && !(m_regs[0][4] & 8);
}

void pgm3_cat6613_device::update_irq()
{
	m_irq(interrupt_active());
}

void pgm3_cat6613_device::set_connected(int state)
{
	if (m_connected != bool(state))
	{
		m_connected = bool(state);
		m_regs[0][6] |= 3; // HPD and receiver-sense changes
		update_irq();
	}
}

void pgm3_cat6613_device::set_video_signal(int state)
{
	bool const was_stable = video_stable();
	m_input_video = bool(state);
	if (was_stable != video_stable())
		m_regs[0][8] |= 0x10;
	update_irq();
}

u8 pgm3_cat6613_device::transmit(offs_t data)
{
	if (BIT(data, 8))
	{
		m_selected = ((data & 0xfe) == 0x98);
		m_read = BIT(data, 0);
		m_subaddress = !m_read;
	}
	else if (m_selected && !m_read)
	{
		if (m_subaddress)
		{
			m_pointer = data;
			m_subaddress = false;
		}
		else
			reg_write(m_pointer++, data);
	}
	else
		return 1;
	return m_selected ? 0 : 1;
}

u8 pgm3_cat6613_device::receive(offs_t offset)
{
	return m_selected && m_read ? reg_read(m_pointer++) : 0xff;
}

u8 pgm3_cat6613_device::reg_read(u8 address)
{
	unsigned const bank = address == 0x0f ? 0 : BIT(m_regs[0][0x0f], 0);
	u8 result = m_regs[bank][address];
	if (!bank)
	{
		switch (address)
		{
		case 1:
			result = 0xca;
			break;
		case 2:
			result = 0x13;
			break;
		case 3:
			result = 0x06;
			break;
		case 0x0e:
			result = (result & 0x0f) | (m_connected ? 0x60 : 0) | (video_stable() ? 0x10 : 0) | (interrupt_active() ? 0x80 : 0);
			break;
		case 0x15:
			result |= 0xf0;
			break; // idle pulled-up DDC/ROM bus lines
		case 0x16:
			result = (result & 0xf8) | (!m_fifo_count ? 2 : 0) | (m_fifo_count == 32 ? 4 : 0);
			break;
		case 0x17:
			result = m_fifo_count ? m_fifo[m_fifo_head] : 0xff;
			if (m_fifo_count && !machine().side_effects_disabled())
			{
				m_fifo_head = (m_fifo_head + 1) & 31;
				--m_fifo_count;
			}
			break;
		}
	}
	LOGMASKED(LOG_HDMI, "read bank=%u reg=%02x data=%02x\n", bank, address, result);
	return result;
}

void pgm3_cat6613_device::reg_write(u8 address, u8 data)
{
	unsigned const bank = address == 0x0f ? 0 : BIT(m_regs[0][0x0f], 0);
	LOGMASKED(LOG_HDMI, "write bank=%u reg=%02x data=%02x\n", bank, address, data);
	if (bank)
	{
		m_regs[1][address] = data;
		return;
	}
	if (address <= 3 || (address >= 6 && address <= 8) || address == 0x16 || address == 0x17)
		return;
	if (address == 4)
	{
		bool const was_stable = video_stable();
		if (data & 0x20)
			reset_registers();
		m_regs[0][4] = data & 0x3d;
		if (data & 4)
			m_regs[0][6] &= ~0x80;
		if (was_stable != video_stable())
			m_regs[0][8] |= 0x10;
	}
	else if (address == 0x0e)
	{
		m_regs[0][address] = data & 0x0f;
		if (data & 1)
		{
			u8 const c = m_regs[0][0x0c], d = m_regs[0][0x0d];
			m_regs[0][6] &= ~(c & 3);
			m_regs[0][7] &= ~(((c >> 2) & 0x3f) | ((d & 1) << 6) | ((d & 2) << 6));
			m_regs[0][8] &= ~(((d >> 2) & 0x3f) | ((data & 2) << 5));
		}
	}
	else if (address == 0x15)
		ddc_command(data & 0x0f);
	else
		m_regs[0][address] = data;
	update_irq();
}

void pgm3_cat6613_device::ddc_command(u8 command)
{
	m_regs[0][0x15] = command;
	m_ddc_command = command;
	m_ddc_header = m_regs[0][0x11];
	m_ddc_offset = m_regs[0][0x12];
	m_ddc_count = m_regs[0][0x13];
	m_ddc_segment = m_regs[0][0x14];
	m_regs[0][0x16] = 0x40;
	// Functional DDC timing at 100 kHz; FIFO clear/abort need no data clocks.
	m_ddc_timer->adjust(attotime::from_usec((command == 0 || command == 3) ? 90 * (unsigned(m_ddc_count) + 3) : 10));
}

TIMER_CALLBACK_MEMBER(pgm3_cat6613_device::ddc_complete)
{
	m_regs[0][0x16] = 0x80;
	if (!(m_regs[0][0x10] & 1))
		m_regs[0][0x16] = 0x20;
	else if (m_ddc_command == 9)
	{
		m_fifo_head = m_fifo_count = 0;
		m_regs[0][6] &= ~0x10;
	}
	else if (m_ddc_command == 0x0f || m_ddc_command == 0x0a)
		m_regs[0][6] &= ~4;
	else if ((m_ddc_command == 0 || m_ddc_command == 3) && m_connected && m_ddc_header == 0xa0 && m_ddc_count <= 32 &&
			unsigned(m_ddc_segment) * 256 + m_ddc_offset + m_ddc_count <= sizeof(EDID))
	{
		m_fifo_head = 0;
		m_fifo_count = m_ddc_count;
		std::copy_n(EDID + unsigned(m_ddc_segment) * 256 + m_ddc_offset, m_fifo_count, m_fifo);
		LOGMASKED(LOG_HDMI, "DDC EDID offset=%02x count=%u complete\n", m_ddc_offset, m_fifo_count);
	}
	else
		m_regs[0][0x16] = 0x20; // absent/unsupported sink address or command: NACK
	if (m_regs[0][0x16] & 0x20)
		m_regs[0][6] |= 0x20;
	update_irq();
}

// Synthetic connected monitor: EDID 1.3, 720p60 preferred, 800x600p60,
// 640x480p60, CTA HDMI vendor block and two-channel LPCM capability.
const u8 pgm3_cat6613_device::EDID[256] = {
	0x00,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0x00,
	0x34,
	0x2d,
	0x38,
	0x03,
	0x00,
	0x00,
	0x00,
	0x00,
	0x01,
	0x20,
	0x01,
	0x03,
	0x80,
	0x20,
	0x12,
	0x78,
	0x0e,
	0xee,
	0x91,
	0xa3,
	0x54,
	0x4c,
	0x99,
	0x26,
	0x0f,
	0x50,
	0x54,
	0x21,
	0x00,
	0x00,
	0x45,
	0x40,
	0x81,
	0xc0,
	0x01,
	0x01,
	0x01,
	0x01,
	0x01,
	0x01,
	0x01,
	0x01,
	0x01,
	0x01,
	0x01,
	0x01,
	0x01,
	0x1d,
	0x00,
	0x72,
	0x51,
	0xd0,
	0x1e,
	0x20,
	0x6e,
	0x28,
	0x55,
	0x00,
	0x40,
	0xb4,
	0x10,
	0x00,
	0x00,
	0x1e,
	0xa0,
	0x0f,
	0x20,
	0x00,
	0x31,
	0x58,
	0x1c,
	0x20,
	0x28,
	0x80,
	0x14,
	0x00,
	0x40,
	0xb4,
	0x10,
	0x00,
	0x00,
	0x1e,
	0x00,
	0x00,
	0x00,
	0xfc,
	0x00,
	0x4d,
	0x41,
	0x4d,
	0x45,
	0x20,
	0x50,
	0x47,
	0x4d,
	0x33,
	0x0a,
	0x20,
	0x20,
	0x20,
	0x00,
	0x00,
	0x00,
	0xfd,
	0x00,
	0x32,
	0x4b,
	0x1e,
	0x50,
	0x08,
	0x00,
	0x0a,
	0x20,
	0x20,
	0x20,
	0x20,
	0x20,
	0x20,
	0x01,
	0x00,
	0x02,
	0x03,
	0x18,
	0xc1,
	0x42,
	0x84,
	0x01,
	0x23,
	0x09,
	0x07,
	0x07,
	0x83,
	0x01,
	0x00,
	0x00,
	0x65,
	0x03,
	0x0c,
	0x00,
	0x10,
	0x00,
	0xe2,
	0x00,
	0x6a,
	0x01,
	0x1d,
	0x00,
	0x72,
	0x51,
	0xd0,
	0x1e,
	0x20,
	0x6e,
	0x28,
	0x55,
	0x00,
	0x40,
	0xb4,
	0x10,
	0x00,
	0x00,
	0x1e,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0xd1,
};
