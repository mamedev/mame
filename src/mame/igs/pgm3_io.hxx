// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
// PGM3 SPI I/O MCU, host-protocol emulation. Firmware is not available.
// TODO: actual MCU revision, board-level pin mapping, JVS and watchdog.
class pgm3_io_device : public device_t
{
public:
	pgm3_io_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	template <typename T> void set_key_region(T &&tag) { m_iokey.set_tag(std::forward<T>(tag)); }

	auto dips_callback() { return m_dips.bind(); }
	auto buttons_callback() { return m_buttons.bind(); }

	void select_w(int state);
	void data_w(u16 data);
	u16 data_r();
	DECLARE_INPUT_CHANGED_MEMBER(coin_changed);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	static constexpr u32 PACKET_SIZE = 256;

	void process();
	void reply(u8 command, u8 const *data, u32 length);
	void crypt(u8 *block, bool encrypt);

	required_region_ptr<u8> m_iokey;
	devcb_read8 m_dips;
	devcb_read32 m_buttons;
	u8 m_input[PACKET_SIZE], m_output[1 + 2 * PACKET_SIZE];
	u16 m_input_count, m_output_count, m_output_pos;
	bool m_selected, m_packet, m_escape;
	u8 m_mode, m_period, m_counters[4];
};

DEFINE_DEVICE_TYPE_PRIVATE(PGM3_IO, pgm3_io_device, pgm3_io_device, "pgm3_io", "PGM3 SPI I/O MCU (host protocol)")

pgm3_io_device::pgm3_io_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, PGM3_IO, tag, owner, clock)
	, m_iokey(*this, finder_base::DUMMY_TAG)
	, m_dips(*this, 0)
	, m_buttons(*this, 0)
{
}

void pgm3_io_device::device_start()
{
	m_selected = false;
	save_item(NAME(m_input));
	save_item(NAME(m_output));
	save_item(NAME(m_input_count));
	save_item(NAME(m_output_count));
	save_item(NAME(m_output_pos));
	save_item(NAME(m_selected));
	save_item(NAME(m_packet));
	save_item(NAME(m_escape));
	save_item(NAME(m_mode));
	save_item(NAME(m_period));
	save_item(NAME(m_counters));
}

void pgm3_io_device::device_reset()
{
	std::fill(std::begin(m_input), std::end(m_input), 0);
	std::fill(std::begin(m_output), std::end(m_output), 0);
	m_input_count = m_output_count = m_output_pos = 0;
	m_packet = m_escape = false;
	m_mode = 1;
	m_period = 50;
	std::fill(std::begin(m_counters), std::end(m_counters), 0);
}

INPUT_CHANGED_MEMBER(pgm3_io_device::coin_changed)
{
	// Coin events are queued separately from the button bitmap. The host's
	// bSPI_CoinRequest consumes one event through the next command 4 request.
	if (newval && !oldval && m_counters[param & 1] != 0xff)
		++m_counters[param & 1];
}

void pgm3_io_device::select_w(int state)
{
	bool const active = !state;
	if (m_selected && !active && m_packet)
	{
		process();
		m_packet = m_escape = false;
		m_input_count = 0;
	}
	m_selected = active;
}

void pgm3_io_device::data_w(u16 data)
{
	if (!m_selected)
		return;
	u8 value = data;
	if (!m_escape && value == 0xe0)
	{
		if (m_packet)
			process();
		m_input_count = 0;
		m_packet = true;
		m_output_count = m_output_pos = 0;
		return;
	}
	if (!m_packet)
		return; // receive clocks send idle 0xff on MOSI
	if (!m_escape && value == 0xd0)
	{
		m_escape = true;
		return;
	}
	if (m_escape)
	{
		m_escape = false;
		if (value != 0xcf && value != 0xdf)
		{
			m_packet = false;
			return;
		}
		++value;
	}
	if (m_input_count < std::size(m_input))
		m_input[m_input_count++] = value;
	else
	{
		m_packet = false;
	}
}

u16 pgm3_io_device::data_r()
{
	if (!m_selected || m_packet || m_output_pos >= m_output_count)
		return 0xff;
	u16 const value = m_output[m_output_pos];
	if (!machine().side_effects_disabled())
		++m_output_pos;
	return value;
}

void pgm3_io_device::crypt(u8 *block, bool encrypt)
{
	for (u32 i = 0; i < 16; i += 4)
	{
		std::swap(block[i], block[i + 3]);
		std::swap(block[i + 1], block[i + 2]);
	}
	aes128ecb::AES_CTX ctx;
	(encrypt ? aes128ecb::AES_EncryptInit : aes128ecb::AES_DecryptInit)(&ctx, &m_iokey[0]);
	(encrypt ? aes128ecb::AES_Encrypt : aes128ecb::AES_Decrypt)(&ctx, block, 16, block);
	aes128ecb::AES_CTX_Free(&ctx);
	for (u32 i = 0; i < 16; i += 4)
	{
		std::swap(block[i], block[i + 3]);
		std::swap(block[i + 1], block[i + 2]);
	}
}

void pgm3_io_device::reply(u8 command, u8 const *data, u32 length)
{
	u8 plain[PACKET_SIZE]{};
	if (length > PACKET_SIZE - 3)
		return;
	plain[0] = command;
	plain[1] = length + 1;
	std::copy_n(data, length, plain + 2);
	for (u32 i = 0; i < length + 2; ++i)
		plain[length + 2] += plain[i];
	u32 const count = (length + 3 + 15) & ~15U;
	m_output_count = m_output_pos = 0;
	m_output[m_output_count++] = 0xe0;
	for (u32 i = 0; i < count; i += 16)
		crypt(plain + i, true);
	for (u32 i = 0; i < count; ++i)
	{
		if (plain[i] == 0xe0 || plain[i] == 0xd0)
		{
			m_output[m_output_count++] = 0xd0;
			m_output[m_output_count++] = plain[i] - 1;
		}
		else
		{
			m_output[m_output_count++] = plain[i];
		}
	}
}

void pgm3_io_device::process()
{
	if (!m_input_count || (m_input_count & 15))
	{
		logerror("incomplete packet %u bytes\n", m_input_count);
		return;
	}
	for (u32 i = 0; i < m_input_count; i += 16)
		crypt(m_input + i, false);
	u32 const length = m_input[1];
	if (!length || length + 2 > m_input_count)
	{
		logerror("invalid packet length %u\n", length);
		return;
	}
	u8 sum = 0;
	for (u32 i = 0; i < length + 1; ++i)
		sum += m_input[i];
	if (sum != m_input[length + 1])
	{
		logerror("packet checksum error\n");
		return;
	}
	LOGMASKED(LOG_IO, "%s: command %02x length=%u data=%02x %02x %02x %02x\n", machine().describe_context(), m_input[0], length - 1, m_input[2], m_input[3], m_input[4],
			m_input[5]);
	switch (m_input[0])
	{
	case 3: // configure/report input interface and polling period
		if (length != 3)
			break;
		// The first request byte is a control field, not a mode to echo.
		// vSPI_ResetMCU sends 80 00; dwIO_GetInputMode2P sends 00 00.
		// A zero field queries the existing setting. The clients configure
		// modes 1/3 (normal/expanded inputs), and report the interval in 100 us.
		// MCU firmware is unavailable; reset defaults remain provisional.
		if (BIT(m_input[2], 7))
		{
			m_mode = 1;
			m_period = 50;
			std::fill(std::begin(m_counters), std::end(m_counters), 0);
		}
		else
		{
			if (BIT(m_input[2], 0))
				m_mode = m_input[2] & 3;
			if (m_input[3])
				m_period = m_input[3];
		}
		{
			u8 const data[3] = { m_dips(), m_mode, m_period };
			reply(3, data, 3);
		}
		break;
	case 4: // read switches/buttons, acknowledge coins and pulse output counters
		if (length != 5)
			break;
		for (u32 i = 0; i < 2; ++i)
		{
			// vSPI_InitMCU sends ff to discard old events. Later requests
			// subtract the number of coins consumed, never underflowing.
			m_counters[i] -= std::min<u32>(m_counters[i], m_input[2 + i]);
			m_counters[2 + i] += m_input[4 + i];
			for (u32 pulse = 0; pulse < m_input[4 + i]; ++pulse)
			{
				machine().bookkeeping().coin_counter_w(i, 1);
				machine().bookkeeping().coin_counter_w(i, 0);
			}
		}
		{
			u32 const buttons = m_buttons();
			u8 const data[10] = { m_dips(), 0, m_counters[0], m_counters[1], m_counters[2], m_counters[3], u8(buttons >> 8), u8(buttons),
				u8(buttons >> 24), u8(buttons >> 16) };
			reply(4, data, 10);
		}
		break;
	default:
		logerror("unsupported I/O command %02x\n", m_input[0]);
		break;
	}
}
