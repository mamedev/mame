// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
// PGM3 S-35390A RTC, byte-level I2C interface at 0x30-0x37 on bus 1.
// Time/status/free-register subset.
// 
// TODO: Alarm outputs, frequency output and oscillator correction

class pgm3_s35390a_device : public device_t, public device_rtc_interface, public device_nvram_interface
{
public:
	pgm3_s35390a_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 32768);
	u8 transmit(offs_t data);
	u8 receive(offs_t offset);

	void stop(int state) { m_selected = false; }

	void acknowledge(int state)
	{
		if (!state && m_read)
			m_selected = false;
	}

protected:
	void device_start() override ATTR_COLD;

	void device_reset() override ATTR_COLD { m_selected = false; }
	bool rtc_feature_leap_year() const override { return true; }

	void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;
	void nvram_default() override;
	bool nvram_read(util::read_stream &file) override;
	bool nvram_write(util::write_stream &file) override;

private:
	void apply_time();
	void reset_registers();
	TIMER_CALLBACK_MEMBER(tick);

	static u8 reverse(u8 b) { return bitswap<8>(b, 0, 1, 2, 3, 4, 5, 6, 7); }
	u8 m_regs[8][7]{};
	u8 m_time[7]{}, m_transfer[7]{};
	u8 m_command = 0, m_pos = 0, m_length = 0;
	bool m_selected = false, m_read = false;
	emu_timer *m_timer = nullptr;
};
DECLARE_DEVICE_TYPE(PGM3_S35390A, pgm3_s35390a_device)
DEFINE_DEVICE_TYPE(PGM3_S35390A, pgm3_s35390a_device, "pgm3_s35390a", "PGM3 S-35390A RTC")

pgm3_s35390a_device::pgm3_s35390a_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: device_t(mconfig, PGM3_S35390A, tag, owner, clock)
		, device_rtc_interface(mconfig, *this)
		, device_nvram_interface(mconfig, *this)
{
}

void pgm3_s35390a_device::device_start()
{
	m_timer = timer_alloc(FUNC(pgm3_s35390a_device::tick), this);
	m_timer->adjust(attotime::from_ticks(32768, clock()), 0, attotime::from_ticks(32768, clock()));
	save_item(NAME(m_regs));
	save_item(NAME(m_time));
	save_item(NAME(m_transfer));
	save_item(NAME(m_register));
	save_item(NAME(m_command));
	save_item(NAME(m_pos));
	save_item(NAME(m_length));
	save_item(NAME(m_selected));
	save_item(NAME(m_read));
}

void pgm3_s35390a_device::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
	m_time[0] = convert_to_bcd(year % 100);
	m_time[1] = convert_to_bcd(month);
	m_time[2] = convert_to_bcd(day);
	m_time[3] = (day_of_week - 1) % 7;
	m_time[4] = convert_to_bcd(hour);
	m_time[5] = convert_to_bcd(minute);
	m_time[6] = convert_to_bcd(second);
}

TIMER_CALLBACK_MEMBER(pgm3_s35390a_device::tick)
{
	// The chip has a two-digit calendar and treats year 00 as a leap year.
	// Keep its calendar here: the generic RTC advance_days currently has an
	// inverted leap-year test, which would skip February 29 on this device.
	if (++m_register[RTC_SECOND] == 60)
	{
		m_register[RTC_SECOND] = 0;
		if (++m_register[RTC_MINUTE] == 60)
		{
			m_register[RTC_MINUTE] = 0;
			if (++m_register[RTC_HOUR] == 24)
			{
				static constexpr int DAYS[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
				m_register[RTC_HOUR] = 0;
				m_register[RTC_DAY_OF_WEEK] = m_register[RTC_DAY_OF_WEEK] % 7 + 1;
				int const month = m_register[RTC_MONTH];
				int const limit = DAYS[month - 1] + (month == 2 && !(m_register[RTC_YEAR] % 4));
				if (++m_register[RTC_DAY] > limit)
				{
					m_register[RTC_DAY] = 1;
					if (++m_register[RTC_MONTH] == 13)
					{
						m_register[RTC_MONTH] = 1;
						m_register[RTC_YEAR] = (m_register[RTC_YEAR] + 1) % 100;
					}
				}
			}
		}
	}
	clock_updated();
}

void pgm3_s35390a_device::nvram_default()
{
	for (auto &bank : m_regs)
		std::fill(std::begin(bank), std::end(bank), 0);
	m_regs[0][0] = 0x40; // Valid, battery-backed MAME base time in 24-hour mode.
}

bool pgm3_s35390a_device::nvram_read(util::read_stream &file)
{
	auto const [err, actual] = read(file, &m_regs[0][0], sizeof(m_regs));
	return !err && actual == sizeof(m_regs);
}

bool pgm3_s35390a_device::nvram_write(util::write_stream &file)
{
	auto const [err, actual] = write(file, &m_regs[0][0], sizeof(m_regs));
	return !err;
}

void pgm3_s35390a_device::reset_registers()
{
	u8 const status = m_regs[0][0] & 0x70;
	nvram_default();
	m_regs[0][0] = status;
	set_time(true, 0, 1, 1, 1, 0, 0, 0);
	m_timer->adjust(attotime::from_ticks(32768, clock()), 0, attotime::from_ticks(32768, clock()));
}

void pgm3_s35390a_device::apply_time()
{
	unsigned const base = m_command == 3 ? 4 : 0;
	for (unsigned i = 0; i < m_length; ++i)
		m_time[base + i] = reverse(m_transfer[i]);
	int hour = bcd_to_integer(m_time[4] & 0x3f);
	if (!BIT(m_regs[0][0], 6))
		hour = hour % 12 + (BIT(m_time[4], 6) ? 12 : 0);
	int const month = bcd_to_integer(m_time[1] & 0x1f), day = bcd_to_integer(m_time[2] & 0x3f);
	int const minute = bcd_to_integer(m_time[5] & 0x7f), second = bcd_to_integer(m_time[6] & 0x7f);
	// Undefined out-of-range BCD writes must not reach the generic calendar.
	if (month < 1 || month > 12 || day < 1 || day > 31 || hour > 23 || minute > 59 || second > 59)
		return;
	set_time(true, bcd_to_integer(m_time[0]), month, day, (m_time[3] & 7) % 7 + 1, hour, minute, second);
	m_timer->adjust(attotime::from_ticks(32768, clock()), 0, attotime::from_ticks(32768, clock()));
	LOGMASKED(LOG_RTC, "time set %02x-%02x-%02x %02x:%02x:%02x\n", m_time[0], m_time[1], m_time[2], m_time[4], m_time[5], m_time[6]);
}

u8 pgm3_s35390a_device::transmit(offs_t data)
{
	if (BIT(data, 8))
	{
		m_selected = (data & 0xf0) == 0x60;
		m_read = BIT(data, 0);
		m_command = (data >> 1) & 7;
		m_pos = 0;
		m_length = m_command == 2 ? 7 : (m_command == 3 || m_command == 4 || m_command == 5) ? 3 : 1;
		if (!m_selected)
			return 1;
		if (m_command == 2 || m_command == 3)
		{
			unsigned const base = m_command == 3 ? 4 : 0;
			for (unsigned i = 0; i < m_length; ++i)
			{
				u8 value = m_time[base + i];
				if (base + i == 4)
				{
					int const hour = bcd_to_integer(value);
					if (!BIT(m_regs[0][0], 6))
						value = convert_to_bcd(hour % 12);
					if (hour >= 12)
						value |= 0x40;
				}
				m_transfer[i] = reverse(value);
			}
		}
		else
			std::copy_n(m_regs[m_command], m_length, m_transfer);
		return 0;
	}
	if (!m_selected || m_read || m_pos >= m_length)
		return 1;
	m_transfer[m_pos++] = data;
	if (m_pos == m_length)
	{
		if (m_command == 2 || m_command == 3)
			apply_time();
		else if (m_command == 0)
		{
			m_regs[0][0] = (m_regs[0][0] & 0x0f) | (data & 0x70);
			if (BIT(data, 7))
				reset_registers();
		}
		else
			std::copy_n(m_transfer, m_length, m_regs[m_command]);
	}
	return 0;
}

u8 pgm3_s35390a_device::receive(offs_t offset)
{
	if (!m_selected || !m_read || m_pos >= m_length)
		return 0xff;
	u8 const result = m_transfer[m_pos++];
	if (!m_command)
		m_regs[0][0] &= ~0x0d; // POC and alarm flags clear on read.
	return result;
}
