// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// handler_entry_read_tap/handler_entry_write_tap

// handler which tap on a bus access and possibly change the data value through a std::function

template<int Width, int AddrShift, endianness_t Endian> class handler_entry_read_tap : public handler_entry_read_passthrough<Width, AddrShift, Endian>
{
public:
	using uX = typename emu::detail::handler_entry_size<Width>::uX;
	using inh = handler_entry_read_passthrough<Width, AddrShift, Endian>;

	handler_entry_read_tap(address_space *space, memory_passthrough_handler &mph, std::string name, std::function<void (offs_t offset, uX &data, uX mem_mask)> tap) : handler_entry_read_passthrough<Width, AddrShift, Endian>(space, mph), m_name(name), m_tap(std::move(tap)) {}
	~handler_entry_read_tap() = default;

	uX read(offs_t offset, uX mem_mask) const override;

	std::string name() const override;

	handler_entry_read_tap<Width, AddrShift, Endian> *instantiate(handler_entry_read<Width, AddrShift, Endian> *next) const override;

protected:
	std::string m_name;
	std::function<void (offs_t offset, uX &data, uX mem_mask)> m_tap;

	handler_entry_read_tap(address_space *space, memory_passthrough_handler &mph, handler_entry_read<Width, AddrShift, Endian> *next, std::string name, std::function<void (offs_t offset, uX &data, uX mem_mask)> tap) : handler_entry_read_passthrough<Width, AddrShift, Endian>(space, mph, next), m_name(name), m_tap(tap) {}
};

template<int Width, int AddrShift, endianness_t Endian> class handler_entry_write_tap : public handler_entry_write_passthrough<Width, AddrShift, Endian>
{
public:
	using uX = typename emu::detail::handler_entry_size<Width>::uX;
	using inh = handler_entry_write_passthrough<Width, AddrShift, Endian>;

	handler_entry_write_tap(address_space *space, memory_passthrough_handler &mph, std::string name, std::function<void (offs_t offset, uX &data, uX mem_mask)> tap) : handler_entry_write_passthrough<Width, AddrShift, Endian>(space, mph), m_name(name), m_tap(std::move(tap)) {}
	~handler_entry_write_tap() = default;

	void write(offs_t offset, uX data, uX mem_mask) const override;

	std::string name() const override;

	handler_entry_write_tap<Width, AddrShift, Endian> *instantiate(handler_entry_write<Width, AddrShift, Endian> *next) const override;

protected:
	std::string m_name;
	std::function<void (offs_t offset, uX &data, uX mem_mask)> m_tap;

	handler_entry_write_tap(address_space *space, memory_passthrough_handler &mph, handler_entry_write<Width, AddrShift, Endian> *next, std::string name, std::function<void (offs_t offset, uX &data, uX mem_mask)> tap) : handler_entry_write_passthrough<Width, AddrShift, Endian>(space, mph, next), m_name(name), m_tap(tap) {}
};
