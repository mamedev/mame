// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_EMU_EMUMEM_HEDP_H
#define MAME_EMU_EMUMEM_HEDP_H

#pragma once

// handler_entry_read_delegate/handler_entry_write_delegate

// Executes an access through a delegate, usually containing a handler or a lambda

template<int Width, int AddrShift, typename READ> class handler_entry_read_delegate : public handler_entry_read_address<Width, AddrShift>
{
public:
	using uX = emu::detail::handler_entry_size_t<Width>;

	handler_entry_read_delegate(address_space *space, u16 flags, const READ &delegate) : handler_entry_read_address<Width, AddrShift>(space, flags), m_delegate(delegate) {}
	~handler_entry_read_delegate() = default;

	uX read(offs_t offset, uX mem_mask) const override;
	uX read_interruptible(offs_t offset, uX mem_mask) const override;
	std::pair<uX, u16> read_flags(offs_t offset, uX mem_mask) const override;
	u16 lookup_flags(offs_t offset, uX mem_mask) const override;

	std::string name() const override;

private:
	READ m_delegate;

	uX read_impl(offs_t offset, uX mem_mask) const requires (
			std::is_same_v<READ, read8_delegate> ||
			std::is_same_v<READ, read16_delegate> ||
			std::is_same_v<READ, read32_delegate> ||
			std::is_same_v<READ, read64_delegate>);

	uX read_impl(offs_t offset, uX mem_mask) const requires (
			std::is_same_v<READ, read8m_delegate> ||
			std::is_same_v<READ, read16m_delegate> ||
			std::is_same_v<READ, read32m_delegate> ||
			std::is_same_v<READ, read64m_delegate>);

	uX read_impl(offs_t offset, uX mem_mask) const requires (
			std::is_same_v<READ, read8s_delegate> ||
			std::is_same_v<READ, read16s_delegate> ||
			std::is_same_v<READ, read32s_delegate> ||
			std::is_same_v<READ, read64s_delegate>);

	uX read_impl(offs_t offset, uX mem_mask) const requires (
			std::is_same_v<READ, read8sm_delegate> ||
			std::is_same_v<READ, read16sm_delegate> ||
			std::is_same_v<READ, read32sm_delegate> ||
			std::is_same_v<READ, read64sm_delegate>);

	uX read_impl(offs_t offset, uX mem_mask) const requires (
			std::is_same_v<READ, read8mo_delegate> ||
			std::is_same_v<READ, read16mo_delegate> ||
			std::is_same_v<READ, read32mo_delegate> ||
			std::is_same_v<READ, read64mo_delegate>);

	uX read_impl(offs_t offset, uX mem_mask) const requires (
			std::is_same_v<READ, read8smo_delegate> ||
			std::is_same_v<READ, read16smo_delegate> ||
			std::is_same_v<READ, read32smo_delegate> ||
			std::is_same_v<READ, read64smo_delegate>);
};

template<int Width, int AddrShift, typename WRITE> class handler_entry_write_delegate : public handler_entry_write_address<Width, AddrShift>
{
public:
	using uX = emu::detail::handler_entry_size_t<Width>;

	handler_entry_write_delegate(address_space *space, u16 flags, const WRITE &delegate) : handler_entry_write_address<Width, AddrShift>(space, flags), m_delegate(delegate) {}
	~handler_entry_write_delegate() = default;

	void write(offs_t offset, uX data, uX mem_mask) const override;
	void write_interruptible(offs_t offset, uX data, uX mem_mask) const override;
	u16 write_flags(offs_t offset, uX data, uX mem_mask) const override;
	u16 lookup_flags(offs_t offset, uX mem_mask) const override;

	std::string name() const override;

private:
	WRITE m_delegate;

	void write_impl(offs_t offset, uX data, uX mem_mask) const requires (
			std::is_same_v<WRITE, write8_delegate> ||
			std::is_same_v<WRITE, write16_delegate> ||
			std::is_same_v<WRITE, write32_delegate> ||
			std::is_same_v<WRITE, write64_delegate>);

	void write_impl(offs_t offset, uX data, uX mem_mask) const requires (
			std::is_same_v<WRITE, write8m_delegate> ||
			std::is_same_v<WRITE, write16m_delegate> ||
			std::is_same_v<WRITE, write32m_delegate> ||
			std::is_same_v<WRITE, write64m_delegate>);

	void write_impl(offs_t offset, uX data, uX mem_mask) const requires (
			std::is_same_v<WRITE, write8s_delegate> ||
			std::is_same_v<WRITE, write16s_delegate> ||
			std::is_same_v<WRITE, write32s_delegate> ||
			std::is_same_v<WRITE, write64s_delegate>);

	void write_impl(offs_t offset, uX data, uX mem_mask) const requires (
			std::is_same_v<WRITE, write8sm_delegate> ||
			std::is_same_v<WRITE, write16sm_delegate> ||
			std::is_same_v<WRITE, write32sm_delegate> ||
			std::is_same_v<WRITE, write64sm_delegate>);

	void write_impl(offs_t offset, uX data, uX mem_mask) const requires (
			std::is_same_v<WRITE, write8mo_delegate> ||
			std::is_same_v<WRITE, write16mo_delegate> ||
			std::is_same_v<WRITE, write32mo_delegate> ||
			std::is_same_v<WRITE, write64mo_delegate>);

	void write_impl(offs_t offset, uX data, uX mem_mask) const requires (
			std::is_same_v<WRITE, write8smo_delegate> ||
			std::is_same_v<WRITE, write16smo_delegate> ||
			std::is_same_v<WRITE, write32smo_delegate> ||
			std::is_same_v<WRITE, write64smo_delegate>);
};


// handler_entry_read_ioport/handler_entry_write_ioport

// Accesses an ioport

template<int Width, int AddrShift> class handler_entry_read_ioport : public handler_entry_read<Width, AddrShift>
{
public:
	using uX = emu::detail::handler_entry_size_t<Width>;

	handler_entry_read_ioport(address_space *space, u16 flags, ioport_port *port) : handler_entry_read<Width, AddrShift>(space, flags), m_port(port) {}
	~handler_entry_read_ioport() = default;

	uX read(offs_t offset, uX mem_mask) const override;
	uX read_interruptible(offs_t offset, uX mem_mask) const override;
	std::pair<uX, u16> read_flags(offs_t offset, uX mem_mask) const override;
	u16 lookup_flags(offs_t offset, uX mem_mask) const override;

	std::string name() const override;

private:
	ioport_port *m_port;
};

template<int Width, int AddrShift> class handler_entry_write_ioport : public handler_entry_write<Width, AddrShift>
{
public:
	using uX = emu::detail::handler_entry_size_t<Width>;

	handler_entry_write_ioport(address_space *space, u16 flags, ioport_port *port) : handler_entry_write<Width, AddrShift>(space, flags), m_port(port) {}
	~handler_entry_write_ioport() = default;

	void write(offs_t offset, uX data, uX mem_mask) const override;
	void write_interruptible(offs_t offset, uX data, uX mem_mask) const override;
	u16 write_flags(offs_t offset, uX data, uX mem_mask) const override;
	u16 lookup_flags(offs_t offset, uX mem_mask) const override;

	std::string name() const override;

private:
	ioport_port *m_port;
};

#endif // MAME_EMU_EMUMEM_HEDP_H
