// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_EMU_EMUMEM_HWS_H
#define MAME_EMU_EMUMEM_HWS_H

#pragma once

// handler_entry_read_before_time/handler_entry_write_before_time
// handler_entry_read_before_delay/handler_entry_write_before_delay
// handler_entry_read_after_delay/handler_entry_write_after_delay

// handlers which handle the different wait state cases

template<int Width, int AddrShift> class handler_entry_read_before_time : public handler_entry_read_passthrough<Width, AddrShift>
{
public:
	using uX = emu::detail::handler_entry_size_t<Width>;

	handler_entry_read_before_time(address_space *space, emu::detail::memory_passthrough_handler_impl &mph, const ws_time_delegate &ws) : handler_entry_read_passthrough<Width, AddrShift>(space, mph, 13), m_ws(ws), m_cpu(downcast<cpu_device &>(space->device())) {}
	~handler_entry_read_before_time() = default;

	uX read(offs_t offset, uX mem_mask) const override;
	uX read_interruptible(offs_t offset, uX mem_mask) const override;
	std::pair<uX, u16> read_flags(offs_t offset, uX mem_mask) const override;
	u16 lookup_flags(offs_t offset, uX mem_mask) const override;

	std::string name() const override;

	handler_entry_read_before_time<Width, AddrShift> *instantiate(handler_entry_read<Width, AddrShift> *next) const override;

protected:
	ws_time_delegate m_ws;
	cpu_device &m_cpu;

	handler_entry_read_before_time(address_space *space, emu::detail::memory_passthrough_handler_impl &mph, handler_entry_read<Width, AddrShift> *next, const ws_time_delegate &ws) : handler_entry_read_passthrough<Width, AddrShift>(space, mph, 13, next), m_ws(ws), m_cpu(downcast<cpu_device &>(space->device())) {}
};

template<int Width, int AddrShift> class handler_entry_write_before_time : public handler_entry_write_passthrough<Width, AddrShift>
{
public:
	using uX = emu::detail::handler_entry_size_t<Width>;

	handler_entry_write_before_time(address_space *space, emu::detail::memory_passthrough_handler_impl &mph, const ws_time_delegate &ws) : handler_entry_write_passthrough<Width, AddrShift>(space, mph, 13), m_ws(ws), m_cpu(downcast<cpu_device &>(space->device())) {}
	~handler_entry_write_before_time() = default;

	void write(offs_t offset, uX data, uX mem_mask) const override;
	void write_interruptible(offs_t offset, uX data, uX mem_mask) const override;
	u16 write_flags(offs_t offset, uX data, uX mem_mask) const override;
	u16 lookup_flags(offs_t offset, uX mem_mask) const override;

	std::string name() const override;

	handler_entry_write_before_time<Width, AddrShift> *instantiate(handler_entry_write<Width, AddrShift> *next) const override;

protected:
	ws_time_delegate m_ws;
	cpu_device &m_cpu;

	handler_entry_write_before_time(address_space *space, emu::detail::memory_passthrough_handler_impl &mph, handler_entry_write<Width, AddrShift> *next, const ws_time_delegate &ws) : handler_entry_write_passthrough<Width, AddrShift>(space, mph, 13, next), m_ws(ws), m_cpu(downcast<cpu_device &>(space->device())) {}
};



template<int Width, int AddrShift> class handler_entry_read_before_delay : public handler_entry_read_passthrough<Width, AddrShift>
{
public:
	using uX = emu::detail::handler_entry_size_t<Width>;

	handler_entry_read_before_delay(address_space *space, emu::detail::memory_passthrough_handler_impl &mph, const ws_delay_delegate &ws) : handler_entry_read_passthrough<Width, AddrShift>(space, mph, 13), m_ws(ws), m_cpu(downcast<cpu_device &>(space->device())) {}
	~handler_entry_read_before_delay() = default;

	uX read(offs_t offset, uX mem_mask) const override;
	uX read_interruptible(offs_t offset, uX mem_mask) const override;
	std::pair<uX, u16> read_flags(offs_t offset, uX mem_mask) const override;
	u16 lookup_flags(offs_t offset, uX mem_mask) const override;

	std::string name() const override;

	handler_entry_read_before_delay<Width, AddrShift> *instantiate(handler_entry_read<Width, AddrShift> *next) const override;

protected:
	ws_delay_delegate m_ws;
	cpu_device &m_cpu;

	handler_entry_read_before_delay(address_space *space, emu::detail::memory_passthrough_handler_impl &mph, handler_entry_read<Width, AddrShift> *next, const ws_delay_delegate &ws) : handler_entry_read_passthrough<Width, AddrShift>(space, mph, 13, next), m_ws(ws), m_cpu(downcast<cpu_device &>(space->device())) {}
};

template<int Width, int AddrShift> class handler_entry_write_before_delay : public handler_entry_write_passthrough<Width, AddrShift>
{
public:
	using uX = emu::detail::handler_entry_size_t<Width>;

	handler_entry_write_before_delay(address_space *space, emu::detail::memory_passthrough_handler_impl &mph, const ws_delay_delegate &ws) : handler_entry_write_passthrough<Width, AddrShift>(space, mph, 11), m_ws(ws), m_cpu(downcast<cpu_device &>(space->device())) {}
	~handler_entry_write_before_delay() = default;

	void write(offs_t offset, uX data, uX mem_mask) const override;
	void write_interruptible(offs_t offset, uX data, uX mem_mask) const override;
	u16 write_flags(offs_t offset, uX data, uX mem_mask) const override;
	u16 lookup_flags(offs_t offset, uX mem_mask) const override;

	std::string name() const override;

	handler_entry_write_before_delay<Width, AddrShift> *instantiate(handler_entry_write<Width, AddrShift> *next) const override;

protected:
	ws_delay_delegate m_ws;
	cpu_device &m_cpu;

	handler_entry_write_before_delay(address_space *space, emu::detail::memory_passthrough_handler_impl &mph, handler_entry_write<Width, AddrShift> *next, const ws_delay_delegate &ws) : handler_entry_write_passthrough<Width, AddrShift>(space, mph, 11, next), m_ws(ws), m_cpu(downcast<cpu_device &>(space->device())) {}
};



template<int Width, int AddrShift> class handler_entry_read_after_delay : public handler_entry_read_passthrough<Width, AddrShift>
{
public:
	using uX = emu::detail::handler_entry_size_t<Width>;

	handler_entry_read_after_delay(address_space *space, emu::detail::memory_passthrough_handler_impl &mph, const ws_delay_delegate &ws) : handler_entry_read_passthrough<Width, AddrShift>(space, mph, 9), m_ws(ws), m_cpu(downcast<cpu_device &>(space->device())) {}
	~handler_entry_read_after_delay() = default;

	uX read(offs_t offset, uX mem_mask) const override;
	uX read_interruptible(offs_t offset, uX mem_mask) const override;
	std::pair<uX, u16> read_flags(offs_t offset, uX mem_mask) const override;
	u16 lookup_flags(offs_t offset, uX mem_mask) const override;

	std::string name() const override;

	handler_entry_read_after_delay<Width, AddrShift> *instantiate(handler_entry_read<Width, AddrShift> *next) const override;

protected:
	ws_delay_delegate m_ws;
	cpu_device &m_cpu;

	handler_entry_read_after_delay(address_space *space, emu::detail::memory_passthrough_handler_impl &mph, handler_entry_read<Width, AddrShift> *next, const ws_delay_delegate &ws) : handler_entry_read_passthrough<Width, AddrShift>(space, mph, 9, next), m_ws(ws), m_cpu(downcast<cpu_device &>(space->device())) {}
};

template<int Width, int AddrShift> class handler_entry_write_after_delay : public handler_entry_write_passthrough<Width, AddrShift>
{
public:
	using uX = emu::detail::handler_entry_size_t<Width>;

	handler_entry_write_after_delay(address_space *space, emu::detail::memory_passthrough_handler_impl &mph, const ws_delay_delegate &ws) : handler_entry_write_passthrough<Width, AddrShift>(space, mph, 9), m_ws(ws), m_cpu(downcast<cpu_device &>(space->device())) {}
	~handler_entry_write_after_delay() = default;

	void write(offs_t offset, uX data, uX mem_mask) const override;
	void write_interruptible(offs_t offset, uX data, uX mem_mask) const override;
	u16 write_flags(offs_t offset, uX data, uX mem_mask) const override;
	u16 lookup_flags(offs_t offset, uX mem_mask) const override;

	std::string name() const override;

	handler_entry_write_after_delay<Width, AddrShift> *instantiate(handler_entry_write<Width, AddrShift> *next) const override;

protected:
	ws_delay_delegate m_ws;
	cpu_device &m_cpu;

	handler_entry_write_after_delay(address_space *space, emu::detail::memory_passthrough_handler_impl &mph, handler_entry_write<Width, AddrShift> *next, const ws_delay_delegate &ws) : handler_entry_write_passthrough<Width, AddrShift>(space, mph, 9, next), m_ws(ws), m_cpu(downcast<cpu_device &>(space->device())) {}
};

#endif // MAME_EMU_EMUMEM_HWS_H
