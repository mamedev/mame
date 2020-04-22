// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// handler_entry_read_address/handler_entry_write_address

// parent class for final handlers which want an address base and a mask

template<int Width, int AddrShift, int Endian> class handler_entry_read_address : public handler_entry_read<Width, AddrShift, Endian>
{
public:
	using uX = typename emu::detail::handler_entry_size<Width>::uX;

	handler_entry_read_address(address_space *space, u32 flags) : handler_entry_read<Width, AddrShift, Endian>(space, flags) {}
	~handler_entry_read_address() = default;

	inline void set_address_info(offs_t base, offs_t mask) {
		m_address_base = base & ~handler_entry_read<Width, AddrShift, Endian>::NATIVE_MASK;
		m_address_mask = mask;
	}

protected:
	offs_t m_address_base, m_address_mask;
};

template<int Width, int AddrShift, int Endian> class handler_entry_write_address : public handler_entry_write<Width, AddrShift, Endian>
{
public:
	using uX = typename emu::detail::handler_entry_size<Width>::uX;

	static constexpr u32 NATIVE_MASK = Width + AddrShift >= 0 ? (1 << (Width + AddrShift)) - 1 : 0;

	handler_entry_write_address(address_space *space, u32 flags) : handler_entry_write<Width, AddrShift, Endian>(space, flags) {}
	~handler_entry_write_address() = default;

	inline void set_address_info(offs_t base, offs_t mask) {
		m_address_base = base & ~handler_entry_write<Width, AddrShift, Endian>::NATIVE_MASK;
		m_address_mask = mask;
	}

protected:
	offs_t m_address_base, m_address_mask;
};
