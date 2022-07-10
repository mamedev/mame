// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_EMU_EMUMEM_HET_H
#define MAME_EMU_EMUMEM_HET_H

#pragma once

// handler_entry_read_tap/handler_entry_write_tap

// handler which tap on a bus access and possibly change the data value through a std::function

template<int Width, int AddrShift> class handler_entry_read_tap : public handler_entry_read_passthrough<Width, AddrShift>
{
public:
	using uX = typename emu::detail::handler_entry_size<Width>::uX;

	handler_entry_read_tap(address_space *space, emu::detail::memory_passthrough_handler_impl &mph, std::string name, std::function<void (offs_t offset, uX &data, uX mem_mask)> tap) : handler_entry_read_passthrough<Width, AddrShift>(space, mph), m_name(name), m_tap(std::move(tap)) {}
	~handler_entry_read_tap() = default;

	uX read(offs_t offset, uX mem_mask) const override;
	std::pair<uX, u16> read_flags(offs_t offset, uX mem_mask) const override;

	std::string name() const override;

	handler_entry_read_tap<Width, AddrShift> *instantiate(handler_entry_read<Width, AddrShift> *next) const override;

protected:
	std::string m_name;
	std::function<void (offs_t offset, uX &data, uX mem_mask)> m_tap;

	handler_entry_read_tap(address_space *space, emu::detail::memory_passthrough_handler_impl &mph, handler_entry_read<Width, AddrShift> *next, std::string name, std::function<void (offs_t offset, uX &data, uX mem_mask)> tap) : handler_entry_read_passthrough<Width, AddrShift>(space, mph, next), m_name(name), m_tap(tap) {}
};

template<int Width, int AddrShift> class handler_entry_write_tap : public handler_entry_write_passthrough<Width, AddrShift>
{
public:
	using uX = typename emu::detail::handler_entry_size<Width>::uX;

	handler_entry_write_tap(address_space *space, emu::detail::memory_passthrough_handler_impl &mph, std::string name, std::function<void (offs_t offset, uX &data, uX mem_mask)> tap) : handler_entry_write_passthrough<Width, AddrShift>(space, mph), m_name(name), m_tap(std::move(tap)) {}
	~handler_entry_write_tap() = default;

	void write(offs_t offset, uX data, uX mem_mask) const override;
	u16 write_flags(offs_t offset, uX data, uX mem_mask) const override;

	std::string name() const override;

	handler_entry_write_tap<Width, AddrShift> *instantiate(handler_entry_write<Width, AddrShift> *next) const override;

protected:
	std::string m_name;
	std::function<void (offs_t offset, uX &data, uX mem_mask)> m_tap;

	handler_entry_write_tap(address_space *space, emu::detail::memory_passthrough_handler_impl &mph, handler_entry_write<Width, AddrShift> *next, std::string name, std::function<void (offs_t offset, uX &data, uX mem_mask)> tap) : handler_entry_write_passthrough<Width, AddrShift>(space, mph, next), m_name(name), m_tap(tap) {}
};

#endif // MAME_EMU_EMUMEM_HET_H
