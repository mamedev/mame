// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
// IGS38 host/slot SDHC registers, native SDHC read subset.
// This functional model uses synthetic CID/CSD, SDMA, 32-bit ADMA2 and PIO.
// Signal timing, CPRM, ADMA1, SDIO, MMC and card writes are not modeled.
// The source CHD is read only.

class igs38_sdhc_device : public device_t
{
public:
	igs38_sdhc_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	template <typename T> void set_dma_space(T &&tag, int space) { m_dma.set_tag(std::forward<T>(tag), space); }
	void set_card_region(char const *region) { m_card_region = region; }
	auto irq_callback() { return m_irq.bind(); }
	auto dma_permissions_callback() { return m_dma_permissions.bind(); }

	void map(address_map &map);

protected:
	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;

private:
	u32 read(offs_t offset, u32 mem_mask = ~0U);
	void write(offs_t offset, u32 data, u32 mem_mask = ~0U);
	void reset_slot();
	void update_irq();
	void event(u32 bits);
	void fail(u32 bits);
	void long_response(std::array<u8, 16> const &value);

	u32 card_status() const { return (u32(m_card_state) << 9) | 0x100; }
	bool inserted() const { return m_card && m_card->logical_bytes() >= 512; }

	void start_data(u32 length, bool storage);
	bool fill_buffer();
	void consume(unsigned bytes);
	void complete_data();
	bool dma_range(u32 address, u32 length, bool write);
	void transfer_adma2();
	TIMER_CALLBACK_MEMBER(command_done);
	TIMER_CALLBACK_MEMBER(transfer_tick);
	required_address_space m_dma;
	devcb_read8 m_dma_permissions;
	devcb_write_line m_irq;
	char const *m_card_region = nullptr;
	chd_file *m_card = nullptr;
	emu_timer *m_command_timer = nullptr, *m_transfer_timer = nullptr;
	u32 m_regs[0x200 / 4]{};
	u8 m_table[512]{}, m_key[5]{}, m_tt_control = 0;
	u8 m_card_state = 0, m_speed = 0;
	bool m_app_command = false, m_busy_command = false;
	bool m_data_active = false, m_storage = false, m_paused = false;
	u32 m_command = 0, m_argument = 0, m_remaining = 0, m_done = 0;
	u64 m_card_offset = 0;
	u16 m_block_size = 0, m_buffer_pos = 0, m_buffer_size = 0;
	u8 m_buffer[512]{};
};

DEFINE_DEVICE_TYPE(IGS38_SDHC, igs38_sdhc_device, "igs38_sdhc", "IGS38 SDHC host")

igs38_sdhc_device::igs38_sdhc_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
		: device_t(mconfig, IGS38_SDHC, tag, owner, clock)
		, m_dma(*this, finder_base::DUMMY_TAG, -1)
		, m_dma_permissions(*this, 0)
		, m_irq(*this)
{
}

void igs38_sdhc_device::device_start()
{
	m_command_timer = timer_alloc(FUNC(igs38_sdhc_device::command_done), this);
	m_transfer_timer = timer_alloc(FUNC(igs38_sdhc_device::transfer_tick), this);
	if (m_card_region)
		m_card = machine().rom_load().get_disk_handle(machine().root_device().subtag(m_card_region));
	save_item(NAME(m_regs));
	save_item(NAME(m_table));
	save_item(NAME(m_key));
	save_item(NAME(m_tt_control));
	save_item(NAME(m_card_state));
	save_item(NAME(m_speed));
	save_item(NAME(m_app_command));
	save_item(NAME(m_busy_command));
	save_item(NAME(m_data_active));
	save_item(NAME(m_storage));
	save_item(NAME(m_paused));
	save_item(NAME(m_command));
	save_item(NAME(m_argument));
	save_item(NAME(m_remaining));
	save_item(NAME(m_done));
	save_item(NAME(m_card_offset));
	save_item(NAME(m_block_size));
	save_item(NAME(m_buffer));
	save_item(NAME(m_buffer_pos));
	save_item(NAME(m_buffer_size));
	machine().save().register_postload(save_prepost_delegate(FUNC(igs38_sdhc_device::update_irq), this));
}

void igs38_sdhc_device::reset_slot()
{
	std::fill(std::begin(m_regs) + 0x100 / 4, std::end(m_regs), 0);
	// Documented capability reset, restricted to SDMA/ADMA2 and default/high
	// speed. ADMA1, suspend-resume and 64-bit addressing are not implemented.
	m_regs[0x140 / 4] = 0x07fa32b2 & ~((3U << 16) | (1U << 20) | (1U << 23) | (1U << 28));
	m_regs[0x148 / 4] = 0x001f0f08;
	m_busy_command = m_data_active = m_paused = false;
	m_remaining = m_done = 0;
	m_buffer_pos = m_buffer_size = 0;
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
	map(0x000, 0xb0b).rw(FUNC(igs38_sdhc_device::read), FUNC(igs38_sdhc_device::write));
}

void igs38_sdhc_device::update_irq()
{
	u32 &status = m_regs[0x130 / 4];
	status = (status & ~0x8000U) | ((status & 0x07ff0000) ? 0x8000 : 0);
	m_irq(bool(status & m_regs[0x138 / 4] & 0x07ff01ff));
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

u32 igs38_sdhc_device::read(offs_t offset, u32 mem_mask)
{
	unsigned const reg = offset * 4;
	if (reg == 0xb08)
		return ((m_tt_control >> 6) << 8) | (BIT(m_tt_control, 4, 2) << 10) | (BIT(m_tt_control, 0) && BIT(m_tt_control, 1) ? 0xff : 0);
	if (reg >= 0x800)
		return 0; // TT data/key/control ports are write only.
	if (reg == 0)
		return 0x00010000;
	if (reg == 0x010 || reg == 0x018)
		return 0; // Write-only capability overrides.
	if (reg == 0xfc || reg == 0x1fc)
		return 0x00010000 | bool(m_regs[0x130 / 4] & m_regs[0x138 / 4] & 0x07ff01ff);
	if (reg == 0x124)
		return (inserted() ? 0x01ff0000 : 0x01000000) | (m_busy_command ? 1 : 0) | (m_data_active ? 0x206 : 0) |
				(m_data_active && !BIT(m_command, 0) && m_buffer_pos < m_buffer_size ? 0x800 : 0);
	if (reg == 0x120)
	{
		u32 value = 0;
		unsigned count = 0;
		for (unsigned lane = 0; lane < 4; ++lane)
			if (mem_mask & (0xffU << (8 * lane)))
			{
				if (m_data_active && !BIT(m_command, 0) && m_buffer_pos + count < m_buffer_size)
					value |= u32(m_buffer[m_buffer_pos + count]) << (8 * lane);
				++count;
			}
		if (!machine().side_effects_disabled() && m_data_active && !BIT(m_command, 0))
			consume(std::min<unsigned>(count, m_buffer_size - m_buffer_pos));
		return value;
	}
	if (reg == 0x150)
		return 0;
	if (reg < 0x200)
		return m_regs[offset];
	return 0;
}

void igs38_sdhc_device::write(offs_t offset, u32 data, u32 mem_mask)
{
	unsigned const reg = offset * 4;
	if (reg >= 0x800 && reg < 0xa08)
	{
		for (unsigned lane = 0; lane < 4; ++lane)
			if (mem_mask & (0xffU << (lane * 8)))
			{
				unsigned const byte = reg - 0x800 + lane;
				if (byte < 512)
					m_table[byte] = data >> (lane * 8);
				else if (byte < 517)
					m_key[byte - 512] = data >> (lane * 8);
			}
		return;
	}
	if (reg == 0xb00 && (mem_mask & 0xff) && BIT(data, 0))
	{
		// Observed loader: ER=(key_mode<<6)|(table_mode<<4)|1 bypasses,
		// |15 enables; DR=1 disables. These mode names follow the supplied
		// working decryptor; the register document names them in reverse.
		// Other per-lane control encodings are unverified.
		m_tt_control = data;
		LOGMASKED(LOG_SDHC, "TT control=%02x status=%03x\n", m_tt_control, read(0xb08 / 4, ~0U));
		return;
	}
	if (reg == 0xb04 && (mem_mask & 1) && BIT(data, 0))
	{
		m_tt_control &= 0xf0;
		return;
	}
	if (reg >= 0x200)
		return;
	if (reg == 0)
	{
		if (data & mem_mask & 1)
		{
			std::fill(std::begin(m_regs), std::begin(m_regs) + 0x100 / 4, 0);
			m_regs[1] = 5;
			reset_slot();
		}
		return;
	}
	if (reg == 0x130)
	{
		m_regs[offset] &= ~(data & mem_mask & 0x07ff00ff);
		update_irq();
		return;
	}
	if (reg == 0x12c)
	{
		u32 value = (m_regs[offset] & ~mem_mask) | (data & mem_mask);
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
		m_regs[offset] = (value & 0x000ffffd) | (BIT(value, 0) ? 2 : 0);
		update_irq();
		return;
	}
	if (reg == 0x10c)
	{
		COMBINE_DATA(&m_regs[offset]);
		if ((mem_mask & 0xff000000) && !m_busy_command)
		{
			m_command = m_regs[offset];
			m_argument = m_regs[0x108 / 4];
			m_busy_command = true;
			m_command_timer->adjust(attotime::from_usec(10));
		}
		return;
	}
	if (reg == 0x100)
	{
		COMBINE_DATA(&m_regs[offset]);
		if (m_paused && (mem_mask & 0xff000000))
		{
			m_paused = false;
			m_transfer_timer->adjust(attotime::from_usec(1));
		}
		return;
	}
	switch (reg)
	{
	case 0x004:
		mem_mask &= 0x00ffffff;
		break;
	case 0x008:
		mem_mask &= 0x0000000f;
		break;
	case 0x010:
		// The firmware can override host capability bits.
		m_regs[0x140 / 4] = (m_regs[0x140 / 4] & ~mem_mask) | (data & mem_mask);
		return;
	case 0x018:
		m_regs[0x148 / 4] = (m_regs[0x148 / 4] & ~mem_mask) | (data & mem_mask);
		return;
	case 0x104:
	case 0x108:
	case 0x128:
	case 0x158:
		break;
	case 0x134:
	case 0x138:
		mem_mask &= 0x07ff01ff;
		break;
	case 0x150:
		event(data & mem_mask & 0x07ff0000);
		return;
	default:
		return;
	}
	COMBINE_DATA(&m_regs[offset]);
	if (reg == 0x134 || reg == 0x138)
		update_irq();
}

void igs38_sdhc_device::long_response(std::array<u8, 16> const &value)
{
	for (unsigned i = 0; i < 4; ++i)
		m_regs[0x110 / 4 + i] = 0;
	// R[127:8] -> RESP[119:0]. The host strips the CRC/end byte.
	for (unsigned i = 0; i < 15; ++i)
	{
		unsigned const shift = (14 - i) * 8;
		m_regs[0x110 / 4 + shift / 32] |= u32(value[i]) << (shift % 32);
	}
}

TIMER_CALLBACK_MEMBER(igs38_sdhc_device::command_done)
{
	unsigned const cmd = BIT(m_command, 24, 6);
	bool const app = m_app_command;
	m_app_command = false;
	m_busy_command = false;
	for (unsigned i = 0; i < 4; ++i)
		m_regs[0x110 / 4 + i] = 0;
	LOGMASKED(LOG_SDHC, "CMD%s%u arg=%08x mode=%08x block=%08x dma=%08x\n", app ? "A" : "", cmd, m_argument, m_command, m_regs[0x104 / 4],
			m_regs[0x100 / 4]);
	// Ending the command cycle drops CICMD and signals CC,
	// including when response-error status is set. Data errors do not do this.
	if (!inserted())
	{
		fail((1U << 16) | 1);
		return;
	}
	m_regs[0x110 / 4] = card_status();
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
			m_regs[0x110 / 4] = 0x00010000 | card_status();
			break;
		case 9:
		{
			// CSD v2 capacity has 512 KiB granularity. Round down rather than
			// advertise sectors beyond a raw dump that ends partway through a unit.
			u32 const size = std::max<u64>(1, m_card->logical_bytes() / 0x80000) - 1;
			long_response({ 0x40, 0x0e, 0, 0x32, 0x5b, 0x59, 0, u8(size >> 16), u8(size >> 8), u8(size), 0x7f, 0x80, 0x0a, 0x40, 0, 1 });
			break;
		}
		case 7:
			m_card_state = m_argument ? 4 : 3;
			m_regs[0x110 / 4] = card_status();
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
			unsigned const requested = m_argument & 15;
			unsigned const selected = requested == 15 ? m_speed : requested;
			m_buffer[16] = selected <= 1 ? selected : 15;
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
			start_data((m_regs[0x104 / 4] & 0xfff) * (BIT(m_command, 1) ? (m_regs[0x104 / 4] >> 16) : 1), true);
			break;
		case 12:
			m_data_active = m_paused = false;
			m_card_state = 4;
			m_transfer_timer->adjust(attotime::never);
			m_regs[0x110 / 4] = card_status();
			break;
		default:
			fail((1U << 16) | 1);
			return; // No response for unimplemented card families.
		}
	event(1);
	if (!m_data_active && BIT(m_command, 16, 2) == 3)
		event(2); // R1b busy released.
}

void igs38_sdhc_device::start_data(u32 length, bool storage)
{
	m_block_size = m_regs[0x104 / 4] & 0xfff;
	if (!length || !m_block_size || m_block_size > sizeof(m_buffer) ||
			(storage && (m_block_size != 512 || m_card_offset + length > m_card->logical_bytes())))
	{
		fail(1U << 20);
		return;
	}
	m_remaining = length;
	m_done = 0;
	m_storage = storage;
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
		if (m_card->read_bytes(offset, m_buffer, m_buffer_size))
		{
			fail(1U << 20);
			return false;
		}
		if (BIT(m_tt_control, 0) && BIT(m_tt_control, 1))
			for (unsigned i = 0; i < m_buffer_size; ++i)
			{
				u64 const address = offset + i;
				m_buffer[i] = igs38_sd_tt_decrypt(
						u32(address), m_buffer[i], BIT(m_tt_control, 4, 2), m_tt_control >> 6, m_table[address % 512], m_key[address % 5]);
			}
	}
	return true;
}

void igs38_sdhc_device::complete_data()
{
	m_data_active = m_paused = false;
	if (BIT(m_command, 2))
		m_regs[0x11c / 4] = card_status(); // Auto-CMD12 R1.
	LOGMASKED(LOG_SDHC, "DATA complete bytes=%08x end_dma=%08x\n", m_done, m_regs[0x100 / 4]);
	event(2);
}

void igs38_sdhc_device::consume(unsigned bytes)
{
	if (!bytes)
		return; // An empty FIFO read cannot complete a block.
	m_buffer_pos += bytes;
	m_done += bytes;
	m_remaining -= bytes;
	if (m_buffer_pos == m_buffer_size)
	{
		if (BIT(m_command, 1) && (m_regs[0x104 / 4] >> 16))
			m_regs[0x104 / 4] -= 0x10000;
		if (!m_remaining)
			complete_data();
		else if (!BIT(m_command, 0))
			m_transfer_timer->adjust(attotime::from_usec(1));
	}
}

bool igs38_sdhc_device::dma_range(u32 address, u32 length, bool write)
{
	if (u64(address) + length > 0x100000000ULL)
		return false;
	for (u32 i = 0; i < length; ++i)
		if (!(m_dma_permissions(address + i) & (write ? 2 : 1)))
			return false;
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
	auto error = [this](u32 state)
	{
		m_regs[0x154 / 4] = state;
		fail(1U << 25);
	};
	// Validate a bounded list before writing destination memory. Bad pointers
	// must not access MMIO, and a cyclic LINK list must not hang the emulator.
	for (unsigned count = 0; count < 4096 && !end; ++count)
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
		if (action == 1)
		{
			error(1);
			return;
		}
		next = action == 3 ? address : next + 8;
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
			unsigned const count = std::min<unsigned>(length, m_buffer_size - m_buffer_pos);
			for (unsigned i = 0; i < count; ++i)
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
	if (!BIT(m_command, 0))
	{
		if (fill_buffer())
			event(0x20);
		return;
	}
	unsigned const mode = BIT(m_regs[0x128 / 4], 3, 2);
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
	if (u64(address) + amount > 0x100000000ULL)
	{
		fail(1U << 20);
		return;
	}
	// Validate the whole burst before touching memory, avoiding MMIO recursion.
	for (u32 i = 0; i < amount; ++i)
		if (!(m_dma_permissions(address + i) & 2))
		{
			fail(1U << 20);
			return;
		}
	while (amount && m_data_active)
	{
		if (m_buffer_pos == m_buffer_size && !fill_buffer())
			return;
		unsigned const count = std::min<unsigned>(amount, m_buffer_size - m_buffer_pos);
		for (unsigned i = 0; i < count; ++i)
			m_dma->write_byte(address++, m_buffer[m_buffer_pos + i]);
		m_regs[0x100 / 4] = address;
		consume(count);
		amount -= count;
	}
	if (m_data_active)
	{
		m_paused = true;
		LOGMASKED(LOG_SDHC, "DMA boundary address=%08x remaining=%08x\n", address, m_remaining);
		event(8);
	}
}
