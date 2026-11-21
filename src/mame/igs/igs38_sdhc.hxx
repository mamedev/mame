// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
// IGS38 host/slot SDHC registers, native SDHC read subset.
// This functional model uses synthetic CID/CSD, SDMA, 32-bit ADMA2 and PIO.
// Signal timing, CPRM, ADMA1, SDIO, MMC and card writes are not modeled.
// The source image is read only.

class igs38_sdhc_device : public device_t
{
public:
	igs38_sdhc_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	template <typename T> void set_dma_space(T &&tag, int space) { m_dma.set_tag(std::forward<T>(tag), space); }
	auto irq_callback() { return m_irq.bind(); }
	auto dma_permissions_callback() { return m_dma_permissions.bind(); }

	void map(address_map &map) ATTR_COLD;

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	u32 register_r(offs_t offset) { return m_regs[offset]; }
	template <u32 Reg> void register_w(offs_t offset, u32 data, u32 mem_mask)
	{
		constexpr u32 MASK = (Reg == 0x004) ? 0x00ffffff : (Reg == 0x008) ? 0x0000000f :
				((Reg == 0x134) || (Reg == 0x138)) ? 0x07ff01ff : 0xffffffff;
		mem_mask &= MASK;
		COMBINE_DATA(&m_regs[Reg / 4]);
	}
	template <u32 Reg> void interrupt_enable_w(offs_t offset, u32 data, u32 mem_mask)
	{
		register_w<Reg>(0, data, mem_mask);
		update_irq();
	}

	void control_w(offs_t offset, u32 data, u32 mem_mask);
	u32 slot_status_r();
	void dma_address_w(offs_t offset, u32 data, u32 mem_mask);
	void command_w(offs_t offset, u32 data, u32 mem_mask);
	u32 buffer_r(offs_t offset, u32 mem_mask);
	u32 present_state_r();
	void clock_reset_w(offs_t offset, u32 data, u32 mem_mask);
	void interrupt_status_w(offs_t offset, u32 data, u32 mem_mask);
	void force_event_w(offs_t offset, u32 data, u32 mem_mask);
	void tt_data_w(offs_t offset, u32 data, u32 mem_mask);
	void tt_enable_w(u8 data);
	void tt_disable_w(u8 data);
	u32 tt_status_r();

	void reset_slot();
	void update_irq();
	void event(u32 bits);
	void fail(u32 bits);
	void long_response(std::array<u8, 16> const &value);

	u32 card_status() const { return (u32(m_card_state) << 9) | 0x100; }
	bool inserted() const { return m_card->exists() && m_card->get_info().sectorbytes == 512; }
	u64 card_bytes() const;

	void start_data(u32 length, bool storage);
	bool fill_buffer();
	void consume(u32 bytes);
	void complete_data();
	bool dma_range(u32 address, u32 length, bool write);
	void transfer_adma2();
	TIMER_CALLBACK_MEMBER(command_done);
	TIMER_CALLBACK_MEMBER(transfer_tick);
	static u8 tt_decrypt(u32 address, u8 data, u32 table_mode, u32 key_mode, u8 table, u8 key);

	required_address_space m_dma;
	required_device<harddisk_image_device> m_card;
	devcb_read8 m_dma_permissions;
	devcb_write_line m_irq;
	emu_timer *m_command_timer;
	emu_timer *m_transfer_timer;
	u32 m_regs[0x200 / 4];
	u8 m_table[512], m_key[5], m_tt_control;
	u8 m_card_state, m_command_state, m_speed;
	bool m_app_command, m_busy_command;
	bool m_data_active, m_storage, m_paused;
	bool m_irq_state;
	u32 m_command, m_data_command, m_argument, m_remaining, m_done;
	u64 m_card_offset;
	u16 m_block_size, m_buffer_pos, m_buffer_size;
	u8 m_buffer[512];
};

DEFINE_DEVICE_TYPE_PRIVATE(IGS38_SDHC, igs38_sdhc_device, igs38_sdhc_device, "igs38_sdhc", "IGS38 SDHC host")

igs38_sdhc_device::igs38_sdhc_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, IGS38_SDHC, tag, owner, clock)
	, m_dma(*this, finder_base::DUMMY_TAG, -1)
	, m_card(*this, "card")
	, m_dma_permissions(*this, 0)
	, m_irq(*this)
{
}

void igs38_sdhc_device::device_add_mconfig(machine_config &config)
{
	HARDDISK(config, m_card).set_interface("sdcard");
}

void igs38_sdhc_device::device_start()
{
	m_command_timer = timer_alloc(FUNC(igs38_sdhc_device::command_done), this);
	m_transfer_timer = timer_alloc(FUNC(igs38_sdhc_device::transfer_tick), this);
	m_irq_state = false;
	m_irq(0);

	save_item(NAME(m_regs));
	save_item(NAME(m_table));
	save_item(NAME(m_key));
	save_item(NAME(m_tt_control));
	save_item(NAME(m_card_state));
	save_item(NAME(m_command_state));
	save_item(NAME(m_speed));
	save_item(NAME(m_app_command));
	save_item(NAME(m_busy_command));
	save_item(NAME(m_data_active));
	save_item(NAME(m_storage));
	save_item(NAME(m_paused));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_command));
	save_item(NAME(m_data_command));
	save_item(NAME(m_argument));
	save_item(NAME(m_remaining));
	save_item(NAME(m_done));
	save_item(NAME(m_card_offset));
	save_item(NAME(m_block_size));
	save_item(NAME(m_buffer));
	save_item(NAME(m_buffer_pos));
	save_item(NAME(m_buffer_size));
}

void igs38_sdhc_device::reset_slot()
{
	std::fill(std::begin(m_regs) + 0x100 / 4, std::end(m_regs), 0);
	// Documented capability reset, restricted to SDMA/ADMA2 and default/high
	// speed. ADMA1, suspend-resume and 64-bit addressing are not implemented.
	m_regs[0x140 / 4] = 0x07fa32b2 & ~((3U << 16) | (1U << 20) | (1U << 23) | (1U << 28));
	m_regs[0x148 / 4] = 0x001f0f08;
	m_busy_command = m_data_active = m_paused = false;
	m_command = m_data_command = m_argument = 0;
	m_command_state = 0;
	m_storage = false;
	m_card_offset = 0;
	m_block_size = 0;
	m_remaining = m_done = 0;
	m_buffer_pos = m_buffer_size = 0;
	std::fill(std::begin(m_buffer), std::end(m_buffer), 0);
	m_command_timer->adjust(attotime::never);
	m_transfer_timer->adjust(attotime::never);
	update_irq();
}

void igs38_sdhc_device::device_reset()
{
	std::fill(std::begin(m_regs), std::end(m_regs), 0);
	m_regs[1] = 5;
	std::fill(std::begin(m_table), std::end(m_table), 0);
	std::fill(std::begin(m_key), std::end(m_key), 0);
	m_tt_control = m_card_state = m_speed = 0;
	m_app_command = false;
	reset_slot();
}

void igs38_sdhc_device::map(address_map &map)
{
	map(0x000, 0xb0b).lr32([] () -> u32 { return 0; }, "reserved_r").nopw();
	map(0x000, 0x1ff).r(FUNC(igs38_sdhc_device::register_r));
	map(0x000, 0x003).lr32([] () -> u32 { return 0x00010000; }, "control_r").w(FUNC(igs38_sdhc_device::control_w));
	map(0x004, 0x007).w(FUNC(igs38_sdhc_device::register_w<0x004>));
	map(0x008, 0x00b).w(FUNC(igs38_sdhc_device::register_w<0x008>));
	map(0x010, 0x013).w(FUNC(igs38_sdhc_device::register_w<0x140>));
	map(0x018, 0x01b).w(FUNC(igs38_sdhc_device::register_w<0x148>));
	map(0x0fc, 0x0ff).r(FUNC(igs38_sdhc_device::slot_status_r));
	map(0x100, 0x103).w(FUNC(igs38_sdhc_device::dma_address_w));
	map(0x104, 0x107).w(FUNC(igs38_sdhc_device::register_w<0x104>));
	map(0x108, 0x10b).w(FUNC(igs38_sdhc_device::register_w<0x108>));
	map(0x10c, 0x10f).w(FUNC(igs38_sdhc_device::command_w));
	map(0x120, 0x123).r(FUNC(igs38_sdhc_device::buffer_r));
	map(0x124, 0x127).r(FUNC(igs38_sdhc_device::present_state_r));
	map(0x128, 0x12b).w(FUNC(igs38_sdhc_device::register_w<0x128>));
	map(0x12c, 0x12f).w(FUNC(igs38_sdhc_device::clock_reset_w));
	map(0x130, 0x133).w(FUNC(igs38_sdhc_device::interrupt_status_w));
	map(0x134, 0x137).w(FUNC(igs38_sdhc_device::interrupt_enable_w<0x134>));
	map(0x138, 0x13b).w(FUNC(igs38_sdhc_device::interrupt_enable_w<0x138>));
	map(0x150, 0x153).w(FUNC(igs38_sdhc_device::force_event_w));
	map(0x158, 0x15b).w(FUNC(igs38_sdhc_device::register_w<0x158>));
	map(0x1fc, 0x1ff).r(FUNC(igs38_sdhc_device::slot_status_r));
	map(0x800, 0xa07).w(FUNC(igs38_sdhc_device::tt_data_w));
	map(0xb00, 0xb00).w(FUNC(igs38_sdhc_device::tt_enable_w));
	map(0xb04, 0xb04).w(FUNC(igs38_sdhc_device::tt_disable_w));
	map(0xb08, 0xb0b).r(FUNC(igs38_sdhc_device::tt_status_r));
}

void igs38_sdhc_device::update_irq()
{
	u32 &status = m_regs[0x130 / 4];
	status = (status & ~0x8000U) | ((status & 0x07ff0000) ? 0x8000 : 0);
	bool const state = bool(status & m_regs[0x138 / 4] & 0x07ff01ff);
	if (state != m_irq_state)
	{
		m_irq_state = state;
		m_irq(state);
	}
}

void igs38_sdhc_device::event(u32 bits)
{
	m_regs[0x130 / 4] |= bits & m_regs[0x134 / 4];
	update_irq();
}

void igs38_sdhc_device::fail(u32 bits)
{
	m_busy_command = m_data_active = m_paused = false;
	m_transfer_timer->adjust(attotime::never);
	event(bits);
}

void igs38_sdhc_device::control_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (data & mem_mask & 1)
	{
		std::fill(std::begin(m_regs), std::begin(m_regs) + 0x100 / 4, 0);
		m_regs[1] = 5;
		reset_slot();
	}
}

u32 igs38_sdhc_device::slot_status_r()
{
	return 0x00010000 | (m_irq_state ? 1 : 0);
}

void igs38_sdhc_device::dma_address_w(offs_t offset, u32 data, u32 mem_mask)
{
	register_w<0x100>(0, data, mem_mask);
	if (m_paused && (mem_mask & 0xff000000))
	{
		m_paused = false;
		m_transfer_timer->adjust(attotime::from_usec(1));
	}
}

void igs38_sdhc_device::command_w(offs_t offset, u32 data, u32 mem_mask)
{
	register_w<0x10c>(0, data, mem_mask);
	if ((mem_mask & 0xff000000) && !m_busy_command)
	{
		m_command = m_regs[0x10c / 4];
		m_command_state = m_card_state;
		m_argument = m_regs[0x108 / 4];
		m_busy_command = true;
		m_command_timer->adjust(attotime::from_usec(10));
	}
}

u32 igs38_sdhc_device::buffer_r(offs_t offset, u32 mem_mask)
{
	u32 value = 0;
	u32 count = 0;
	for (u32 lane = 0; lane < 4; ++lane)
	{
		if (mem_mask & (0xffU << (8 * lane)))
		{
			if (m_data_active && !BIT(m_data_command, 0) && m_buffer_pos + count < m_buffer_size)
				value |= u32(m_buffer[m_buffer_pos + count]) << (8 * lane);
			++count;
		}
	}
	if (!machine().side_effects_disabled() && m_data_active && !BIT(m_data_command, 0))
		consume(std::min<u32>(count, m_buffer_size - m_buffer_pos));
	return value;
}

u32 igs38_sdhc_device::present_state_r()
{
	return (inserted() ? 0x01ff0000 : 0x01020000) | (m_busy_command ? 1 : 0) | (m_data_active ? 0x206 : 0) |
			((m_data_active && !BIT(m_data_command, 0) && m_buffer_pos < m_buffer_size) ? 0x800 : 0);
}

void igs38_sdhc_device::clock_reset_w(offs_t offset, u32 data, u32 mem_mask)
{
	u32 value = (m_regs[0x12c / 4] & ~mem_mask) | (data & mem_mask);
	if (value & 0x01000000)
	{
		reset_slot();
		value = 0;
	}
	else
	{
		if (value & 0x02000000)
		{
			m_busy_command = false;
			m_command_timer->adjust(attotime::never);
			m_regs[0x130 / 4] &= ~0x000f0001U;
		}
		if (value & 0x04000000)
		{
			m_data_active = m_paused = false;
			m_remaining = 0;
			m_buffer_size = m_buffer_pos = 0;
			m_transfer_timer->adjust(attotime::never);
			m_regs[0x130 / 4] &= ~0x07f0003eU;
			m_regs[0x128 / 4] &= ~0x00030000U;
		}
	}
	m_regs[0x12c / 4] = (value & 0x000ffffd) | (BIT(value, 0) ? 2 : 0);
	update_irq();
}

void igs38_sdhc_device::interrupt_status_w(offs_t offset, u32 data, u32 mem_mask)
{
	m_regs[0x130 / 4] &= ~(data & mem_mask & 0x07ff00ff);
	update_irq();
}

void igs38_sdhc_device::force_event_w(offs_t offset, u32 data, u32 mem_mask)
{
	event(data & mem_mask & 0x07ff0000);
}

void igs38_sdhc_device::tt_data_w(offs_t offset, u32 data, u32 mem_mask)
{
	for (u32 lane = 0; lane < 4; ++lane)
	{
		if (mem_mask & (0xffU << (lane * 8)))
		{
			u32 const byte = offset * 4 + lane;
			if (byte < std::size(m_table))
				m_table[byte] = data >> (lane * 8);
			else if (byte < std::size(m_table) + std::size(m_key))
				m_key[byte - std::size(m_table)] = data >> (lane * 8);
		}
	}
}

void igs38_sdhc_device::tt_enable_w(u8 data)
{
	if (BIT(data, 0))
	{
		// ER=(key_mode<<6)|(table_mode<<4)|1 bypasses the transformation;
		// setting bits 0-3 enables it. Other lane encodings are unverified.
		m_tt_control = data;
		LOGMASKED(LOG_SDHC, "%s: TT control=%02x status=%03x\n", machine().describe_context(), m_tt_control, tt_status_r());
	}
}

void igs38_sdhc_device::tt_disable_w(u8 data)
{
	if (BIT(data, 0))
		m_tt_control &= 0xf0;
}

u32 igs38_sdhc_device::tt_status_r()
{
	return ((m_tt_control >> 6) << 8) | (BIT(m_tt_control, 4, 2) << 10) | ((BIT(m_tt_control, 0) && BIT(m_tt_control, 1)) ? 0xff : 0);
}

u64 igs38_sdhc_device::card_bytes() const
{
	auto const &info = m_card->get_info();
	return u64(info.cylinders) * info.heads * info.sectors * info.sectorbytes;
}

void igs38_sdhc_device::long_response(std::array<u8, 16> const &value)
{
	for (u32 i = 0; i < 4; ++i)
		m_regs[0x110 / 4 + i] = 0;
	// R[127:8] -> RESP[119:0]. The host strips the CRC/end byte.
	for (u32 i = 0; i < 15; ++i)
	{
		u32 const shift = (14 - i) * 8;
		m_regs[0x110 / 4 + shift / 32] |= u32(value[i]) << (shift % 32);
	}
}

TIMER_CALLBACK_MEMBER(igs38_sdhc_device::command_done)
{
	u32 const cmd = BIT(m_command, 24, 6);
	bool const app = m_app_command;
	m_app_command = false;
	m_busy_command = false;
	for (u32 i = 0; i < 4; ++i)
		m_regs[0x110 / 4 + i] = 0;
	LOGMASKED(LOG_SDHC, "%s: CMD%s%u arg=%08x mode=%08x block=%08x dma=%08x\n", machine().describe_context(), app ? "A" : "", cmd, m_argument, m_command, m_regs[0x104 / 4],
			m_regs[0x100 / 4]);
	// Ending the command cycle drops CICMD and signals CC,
	// including when response-error status is set. Data errors do not do this.
	if (!inserted())
	{
		fail((1U << 16) | 1);
		return;
	}
	m_regs[0x110 / 4] = (u32(m_command_state) << 9) | 0x100;
	if (app)
	{
		switch (cmd)
		{
		case 41:
			m_card_state = 1;
			m_regs[0x110 / 4] = 0xc0ff8000;
			break;
		case 6:
			break; // SD bus width, selected by ACMD6.
		case 51:
			std::fill(std::begin(m_buffer), std::end(m_buffer), 0);
			m_buffer[0] = 2;
			m_buffer[1] = 5; // SCR: SD 2.0, 1-/4-bit bus.
			start_data(8, false);
			break;
		case 13:
			std::fill(std::begin(m_buffer), std::end(m_buffer), 0);
			start_data(64, false);
			break;
		default:
			m_regs[0x110 / 4] |= 1U << 22;
			break; // R1 illegal command.
		}
	}
	else
	{
		switch (cmd)
		{
		case 0:
			m_card_state = m_speed = 0;
			m_regs[0x110 / 4] = 0;
			break;
		case 8:
			if ((m_argument & 0xf00) != 0x100)
			{
				fail((1U << 16) | 1);
				return;
			}
			m_regs[0x110 / 4] = m_argument & 0xfff;
			break;
		case 55:
			m_app_command = true;
			m_regs[0x110 / 4] |= 0x20;
			break;
		case 2:
		case 10:
			long_response({ 0x03, 'M', 'E', 'P', 'G', 'M', '3', ' ', 0x10, 0, 0, 0, 1, 0x01, 0x61, 1 });
			if (cmd == 2)
				m_card_state = 2;
			break;
		case 3:
			m_card_state = 3;
			m_regs[0x110 / 4] |= 0x00010000;
			break;
		case 9:
		{
			// CSD v2 capacity has 512 KiB granularity. Round down rather than
			// advertise sectors beyond a raw dump that ends partway through a unit.
			u32 const size = std::max<u64>(1, card_bytes() / 0x80000) - 1;
			long_response({ 0x40, 0x0e, 0, 0x32, 0x5b, 0x59, 0, u8(size >> 16), u8(size >> 8), u8(size), 0x7f, 0x80, 0x0a, 0x40, 0, 1 });
			break;
		}
		case 7:
			m_card_state = m_argument ? 4 : 3;
			break;
		case 13:
			break;
		case 16:
			if (m_argument != 512)
				m_regs[0x110 / 4] |= 1U << 29;
			break;
		case 6:
		{
			std::fill(std::begin(m_buffer), std::end(m_buffer), 0);
			m_buffer[1] = 100; // Synthetic card maximum current, in mA.
			m_buffer[13] = 3; // Function group 1 supports default and high speed.
			u32 const requested = m_argument & 15;
			u32 const selected = (requested == 15) ? m_speed : requested;
			m_buffer[16] = (selected <= 1) ? selected : 15;
			if (BIT(m_argument, 31) && selected <= 1)
				m_speed = selected;
			start_data(64, false);
			break;
		}
		case 17:
		case 18:
			if (m_card_state != 4 || !BIT(m_command, 4))
			{
				fail(1U << 20);
				return;
			}
			m_card_offset = u64(m_argument) * 512;
			start_data((m_regs[0x104 / 4] & 0xfff) * ((BIT(m_command, 5) && BIT(m_command, 1)) ? (m_regs[0x104 / 4] >> 16) : 1), true);
			break;
		case 12:
			m_data_active = m_paused = false;
			m_card_state = 4;
			m_transfer_timer->adjust(attotime::never);
			break;
		default:
			fail((1U << 16) | 1);
			return; // No response for unimplemented card families.
		}
	}
	event(1);
	if (!m_data_active && BIT(m_command, 16, 2) == 3)
		event(2); // R1b busy released.
}

void igs38_sdhc_device::start_data(u32 length, bool storage)
{
	m_block_size = m_regs[0x104 / 4] & 0xfff;
	if (!length || !m_block_size || m_block_size > std::size(m_buffer) ||
			(storage && (m_block_size != 512 || m_card_offset + length > card_bytes())))
	{
		fail(1U << 20);
		return;
	}
	m_remaining = length;
	m_done = 0;
	m_storage = storage;
	m_data_command = m_command;
	m_card_state = 5;
	m_data_active = true;
	m_paused = false;
	m_buffer_pos = m_buffer_size = 0;
	m_transfer_timer->adjust(attotime::from_usec(10));
}

bool igs38_sdhc_device::fill_buffer()
{
	m_buffer_pos = 0;
	m_buffer_size = std::min<u32>(m_block_size, m_remaining);
	if (m_storage)
	{
		u64 const offset = m_card_offset + m_done;
		if (!m_card->read(u32(offset / 512), m_buffer))
		{
			fail(1U << 20);
			return false;
		}
		if (BIT(m_tt_control, 0) && BIT(m_tt_control, 1))
		{
			for (u32 i = 0; i < m_buffer_size; ++i)
			{
				u64 const address = offset + i;
				m_buffer[i] = tt_decrypt(
						u32(address), m_buffer[i], BIT(m_tt_control, 4, 2), m_tt_control >> 6, m_table[address % 512], m_key[address % 5]);
			}
		}
	}
	return true;
}

void igs38_sdhc_device::complete_data()
{
	m_data_active = m_paused = false;
	if (BIT(m_data_command, 2, 2) == 1)
	{
		m_regs[0x11c / 4] = card_status(); // Auto-CMD12 R1, before leaving data state.
		m_card_state = 4;
	}
	else if (!m_storage || BIT(m_data_command, 24, 6) == 17)
	{
		m_card_state = 4;
	}
	LOGMASKED(LOG_SDHC, "%s: DATA complete bytes=%08x end_dma=%08x\n", machine().describe_context(), m_done, m_regs[0x100 / 4]);
	event(2);
}

void igs38_sdhc_device::consume(u32 bytes)
{
	if (!bytes)
		return; // An empty FIFO read cannot complete a block.
	m_buffer_pos += bytes;
	m_done += bytes;
	m_remaining -= bytes;
	if (m_buffer_pos == m_buffer_size)
	{
		if (BIT(m_data_command, 5) && BIT(m_data_command, 1) && (m_regs[0x104 / 4] >> 16))
			m_regs[0x104 / 4] -= 0x10000;
		if (!m_remaining)
			complete_data();
		else if (!BIT(m_data_command, 0))
			m_transfer_timer->adjust(attotime::from_usec(1));
	}
}

bool igs38_sdhc_device::dma_range(u32 address, u32 length, bool write)
{
	if (u64(address) + length > 0x1'0000'0000ULL)
		return false;
	// The driver's DMA permission regions are aligned to 4 KiB pages.
	for (u64 current = address, end = u64(address) + length; current < end; current = (current | 0xfff) + 1)
	{
		if (!(m_dma_permissions(u32(current)) & (write ? 2 : 1)))
			return false;
	}
	return true;
}

void igs38_sdhc_device::transfer_adma2()
{
	// Eight-byte, little-endian
	// descriptors with VALID/END/INT, action, length and physical address.
	struct descriptor
	{
		u32 next, address, length;
		bool interrupt;
	};

	std::vector<descriptor> descriptors;
	u32 next = m_regs[0x158 / 4], remaining = m_remaining;
	bool end = false;
	auto const error = [this] (u32 state)
	{
		m_regs[0x154 / 4] = state;
		fail(1U << 25);
	};
	// Validate a bounded list before writing destination memory. Bad pointers
	// must not access MMIO, and a cyclic LINK list must not hang the emulator.
	for (u32 count = 0; count < 4096 && !end; ++count)
	{
		m_regs[0x158 / 4] = next;
		if ((next & 3) || !dma_range(next, 8, false))
		{
			error(1);
			return;
		}
		u32 const attributes = m_dma->read_dword(next);
		u32 const address = m_dma->read_dword(next + 4);
		if (!BIT(attributes, 0))
		{
			error(1);
			return;
		}
		u32 length = 0;
		u32 const action = BIT(attributes, 4, 2);
		next = (action == 3) ? address : next + 8;
		if (action == 2)
		{
			length = attributes >> 16;
			if (!length)
				length = 0x10000;
			if (length > remaining)
			{
				m_regs[0x158 / 4] = next;
				error(7);
				return;
			}
			if ((address & 3) || !dma_range(address, length, true))
			{
				m_regs[0x158 / 4] = next;
				error(3);
				return;
			}
			remaining -= length;
		}
		descriptors.push_back({ next, address, length, bool(BIT(attributes, 2)) });
		end = BIT(attributes, 1);
	}
	if (!end)
	{
		error(1);
		return;
	}
	if (remaining)
	{
		m_regs[0x158 / 4] = next;
		error(4);
		return;
	}
	m_regs[0x154 / 4] = 0;
	for (auto const &desc : descriptors)
	{
		m_regs[0x158 / 4] = desc.next;
		u32 address = desc.address, length = desc.length;
		while (length)
		{
			if (m_buffer_pos == m_buffer_size && !fill_buffer())
				return;
			u32 const count = std::min<u32>(length, m_buffer_size - m_buffer_pos);
			for (u32 i = 0; i < count; ++i)
				m_dma->write_byte(address++, m_buffer[m_buffer_pos + i]);
			consume(count);
			length -= count;
		}
		if (desc.interrupt)
			event(8);
	}
}

TIMER_CALLBACK_MEMBER(igs38_sdhc_device::transfer_tick)
{
	if (!m_data_active || m_paused)
		return;
	if (!BIT(m_data_command, 0))
	{
		if (fill_buffer())
			event(0x20);
		return;
	}
	u32 const mode = BIT(m_regs[0x128 / 4], 3, 2);
	if (mode == 2)
	{
		transfer_adma2();
		return;
	}
	if (mode)
	{
		m_regs[0x154 / 4] = 1;
		fail(1U << 25);
		return;
	}
	u32 address = m_regs[0x100 / 4];
	u32 const boundary = 4096U << BIT(m_regs[0x104 / 4], 12, 3);
	u32 amount = std::min(m_remaining, boundary - (address & (boundary - 1)));
	if (!dma_range(address, amount, true))
	{
		fail(1U << 20);
		return;
	}
	while (amount && m_data_active)
	{
		if (m_buffer_pos == m_buffer_size && !fill_buffer())
			return;
		u32 const count = std::min<u32>(amount, m_buffer_size - m_buffer_pos);
		for (u32 i = 0; i < count; ++i)
			m_dma->write_byte(address++, m_buffer[m_buffer_pos + i]);
		m_regs[0x100 / 4] = address;
		consume(count);
		amount -= count;
	}
	if (m_data_active)
	{
		m_paused = true;
		LOGMASKED(LOG_SDHC, "%s: DMA boundary address=%08x remaining=%08x\n", machine().describe_context(), address, m_remaining);
		event(8);
	}
}

u8 igs38_sdhc_device::tt_decrypt(
		u32 address, u8 data, u32 table_mode, u32 key_mode, u8 table, u8 key)
{
	auto const a = [address] (u32 n)
	{
		return BIT(address, n);
	};
	auto const t = [table] (u32 n)
	{
		return BIT(table, n);
	};
	auto const k = [key] (u32 n)
	{
		return BIT(key, n);
	};
	u32 gt = 0, gk = 0;
	switch (table_mode & 3)
	{
	case 0:
		gt |= (((t(0) & (a(8))) | (~a(0))) & 1U) << 0;
		gt |= (((t(2) | (~a(9))) & (~a(1))) & 1U) << 1;
		gt |= (((t(4) & (~a(10))) | (~a(2))) & 1U) << 2;
		gt |= (((t(6) & (a(11))) & (~a(3))) & 1U) << 3;
		gt |= (((t(7) | (a(12))) & (a(4))) & 1U) << 4;
		gt |= (((t(5) & (~a(13))) | (a(5))) & 1U) << 5;
		gt |= (((t(3) | (~a(14))) & (~a(6))) & 1U) << 6;
		gt |= (((t(1) | (~a(15))) | (~a(7))) & 1U) << 7;
		break;
	case 1:
		gt |= (((t(0) | (~a(0))) | (a(15))) & 1U) << 0;
		gt |= (((t(2) & (a(1))) & (~a(14))) & 1U) << 1;
		gt |= (((t(4) & (a(2))) | (a(13))) & 1U) << 2;
		gt |= (((t(6) | (a(3))) | (~a(12))) & 1U) << 3;
		gt |= (((t(1) | (~a(4))) & (a(11))) & 1U) << 4;
		gt |= (((t(3) & (~a(5))) | (~a(10))) & 1U) << 5;
		gt |= (((t(5) | (~a(6))) | (a(9))) & 1U) << 6;
		gt |= (((t(7) | (~a(7))) & (~a(8))) & 1U) << 7;
		break;
	case 2:
		gt |= (((t(0) & (~a(0))) & (a(15))) & 1U) << 0;
		gt |= (((t(1) | (~a(2))) | (~a(13))) & 1U) << 1;
		gt |= (((t(2) & (a(4))) & (a(11))) & 1U) << 2;
		gt |= (((t(3) | (~a(6))) | (~a(9))) & 1U) << 3;
		gt |= (((t(4) & (a(8))) & (a(7))) & 1U) << 4;
		gt |= (((t(5) | (a(10))) | (~a(5))) & 1U) << 5;
		gt |= (((t(6) & (~a(12))) & (a(3))) & 1U) << 6;
		gt |= (((t(7) | (a(14))) | (~a(1))) & 1U) << 7;
		break;
	case 3:
		gt |= (((t(0) & (a(14))) | (a(1))) & 1U) << 0;
		gt |= (((t(1) | (~a(12))) | (~a(3))) & 1U) << 1;
		gt |= (((t(2) & (a(10))) & (~a(5))) & 1U) << 2;
		gt |= (((t(3) | (a(8))) & (a(7))) & 1U) << 3;
		gt |= (((t(4) & (~a(6))) | (~a(9))) & 1U) << 4;
		gt |= (((t(5) | (a(4))) | (a(11))) & 1U) << 5;
		gt |= (((t(6) & (~a(2))) & (~a(13))) & 1U) << 6;
		gt |= (((t(7) | (a(0))) | (a(15))) & 1U) << 7;
		break;
	}
	switch (key_mode & 3)
	{
	case 0:
		gk |= (((k(7) & (a(0))) & (~a(21))) & 1U) << 0;
		gk |= (((k(5) & (~a(17))) | (a(4))) & 1U) << 1;
		gk |= (((k(3) | (~a(2))) & (~a(23))) & 1U) << 2;
		gk |= (((k(1) & (a(19))) | (~a(6))) & 1U) << 3;
		gk |= (((k(0) | (~a(20))) & (a(7))) & 1U) << 4;
		gk |= (((k(2) & (~a(3))) | (~a(16))) & 1U) << 5;
		gk |= (((k(4) | (a(18))) & (a(5))) & 1U) << 6;
		gk |= (((k(6) & (~a(1))) | (~a(22))) & 1U) << 7;
		break;
	case 1:
		gk |= (((k(0) | (a(6))) & (a(3))) & 1U) << 0;
		gk |= (((k(2) & (~a(16))) | (~a(7))) & 1U) << 1;
		gk |= (((k(4) | (~a(2))) | (a(4))) & 1U) << 2;
		gk |= (((k(6) & (a(17))) | (a(1))) & 1U) << 3;
		gk |= (((k(1) | (~a(5))) & (a(0))) & 1U) << 4;
		gk |= (((k(3) & (~a(21))) | (~a(20))) & 1U) << 5;
		gk |= (((k(5) | (~a(19))) & (~a(22))) & 1U) << 6;
		gk |= (((k(7) | (~a(23))) | (~a(18))) & 1U) << 7;
		break;
	case 2:
		gk |= (((k(0) | (a(21))) & (a(17))) & 1U) << 0;
		gk |= (((k(1) & (~a(16))) | (~a(22))) & 1U) << 1;
		gk |= (((k(2) | (~a(3))) & (a(7))) & 1U) << 2;
		gk |= (((k(3) | (a(6))) | (~a(4))) & 1U) << 3;
		gk |= (((k(4) & (~a(5))) & (~a(23))) & 1U) << 4;
		gk |= (((k(5) & (a(18))) | (a(2))) & 1U) << 5;
		gk |= (((k(6) | (~a(1))) & (~a(19))) & 1U) << 6;
		gk |= (((k(7) | (~a(20))) | (a(0))) & 1U) << 7;
		break;
	case 3:
		gk |= (((k(0) | (a(16))) & (a(0))) & 1U) << 0;
		gk |= (((k(1) & (~a(20))) & (~a(22))) & 1U) << 1;
		gk |= (((k(2) | (a(23))) & (~a(17))) & 1U) << 2;
		gk |= (((k(3) & (a(3))) | (a(21))) & 1U) << 3;
		gk |= (((k(4) | (~a(1))) & (a(18))) & 1U) << 4;
		gk |= (((k(5) | (a(4))) & (~a(7))) & 1U) << 5;
		gk |= (((k(6) & (~a(19))) | (~a(6))) & 1U) << 6;
		gk |= (((k(7) | (a(2))) | (a(5))) & 1U) << 7;
		break;
	}
	u32 const rot = bitswap<3>(gt, 4, 2, 0);
	u32 value = data ^ gk;
	value = (gt & 0x40) ? ((value >> rot) | (value << (8 - rot))) : ((value << rot) | (value >> (8 - rot)));
	u8 const permuted = bitswap<8>(value, 0, 7, 1, 6, 2, 5, 3, 4);
	return u8(permuted ^ gt);
}
