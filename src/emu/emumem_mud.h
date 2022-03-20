// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Descriptors for subunit support

template<int Width, int AddrShift> class memory_units_descriptor {
public:
	using uX = typename emu::detail::handler_entry_size<Width>::uX;

	struct entry {
		uX m_amask;
		uX m_dmask;
		s8 m_ashift;
		u8 m_dshift;
		u8 m_offset;
	};

	memory_units_descriptor(u8 access_width, endianness_t access_endian, handler_entry *handler, offs_t addrstart, offs_t addrend, offs_t mask, uX unitmask, int cswidth);

	offs_t get_handler_start() const { return m_handler_start; }
	offs_t get_handler_mask() const { return m_handler_mask; }

	u8 rkey_to_ukey(u8 rkey) const { return m_keymap[rkey]; }
	const std::vector<entry> &get_entries_for_key(u8 key) const { return m_entries_for_key.find(key)->second; }

	u8 get_subunit_width() const { return m_access_width; }
	endianness_t get_subunit_endian() const { return m_access_endian; }

	void set_subunit_handler(handler_entry *handler) { m_handler = handler; }
	handler_entry *get_subunit_handler() const { return m_handler; }

private:
	std::map<u8, std::vector<entry>> m_entries_for_key;
	offs_t m_addrstart, m_addrend, m_handler_start, m_handler_mask;
	handler_entry *m_handler;
	std::array<u8, 4> m_keymap;
	u8 m_access_width;
	endianness_t m_access_endian;

	void generate(u8 ukey, uX gumask, uX umask, u32 cswidth, u32 bits_per_access, u8 base_shift, s8 shift, u32 active_count);
};
