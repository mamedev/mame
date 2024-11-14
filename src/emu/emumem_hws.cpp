// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "emumem_hep.h"
#include "emumem_hws.h"

template<int Width, int AddrShift> emu::detail::handler_entry_size_t<Width> handler_entry_read_before_time<Width, AddrShift>::read(offs_t offset, uX mem_mask) const
{
	return this->m_next->read(offset, mem_mask);
}

template<int Width, int AddrShift> emu::detail::handler_entry_size_t<Width> handler_entry_read_before_time<Width, AddrShift>::read_interruptible(offs_t offset, uX mem_mask) const
{
	u64 tc = m_cpu.total_cycles();
	u64 ac = m_ws(offset, tc);

	if(ac < tc)
		ac = tc;

	if(m_cpu.access_before_time(ac, tc))
		return 0;

	return this->m_next->read_interruptible(offset, mem_mask);
}

template<int Width, int AddrShift> std::pair<emu::detail::handler_entry_size_t<Width>, u16> handler_entry_read_before_time<Width, AddrShift>::read_flags(offs_t offset, uX mem_mask) const
{
	return this->m_next->read_flags(offset, mem_mask);
}

template<int Width, int AddrShift> u16 handler_entry_read_before_time<Width, AddrShift>::lookup_flags(offs_t offset, uX mem_mask) const
{
	return this->m_next->lookup_flags(offset, mem_mask);
}

template<int Width, int AddrShift> std::string handler_entry_read_before_time<Width, AddrShift>::name() const
{
	return util::string_format("(bt %s) %s", m_ws.name(), this->m_next->name());
}

template<int Width, int AddrShift> handler_entry_read_before_time<Width, AddrShift> *handler_entry_read_before_time<Width, AddrShift>::instantiate(handler_entry_read<Width, AddrShift> *next) const
{
	return new handler_entry_read_before_time<Width, AddrShift>(this->m_space, this->m_mph, next, m_ws);
}


template<int Width, int AddrShift> void handler_entry_write_before_time<Width, AddrShift>::write(offs_t offset, uX data, uX mem_mask) const
{
	this->m_next->write(offset, data, mem_mask);
}

template<int Width, int AddrShift> void handler_entry_write_before_time<Width, AddrShift>::write_interruptible(offs_t offset, uX data, uX mem_mask) const
{
	u64 tc = m_cpu.total_cycles();
	u64 ac = m_ws(offset, tc);

	if(ac < tc)
		ac = tc;

	if(m_cpu.access_before_time(ac, tc))
		return;
	this->m_next->write_interruptible(offset, data, mem_mask);
}

template<int Width, int AddrShift> u16 handler_entry_write_before_time<Width, AddrShift>::write_flags(offs_t offset, uX data, uX mem_mask) const
{
	return this->m_next->write_flags(offset, data, mem_mask);
}

template<int Width, int AddrShift> u16 handler_entry_write_before_time<Width, AddrShift>::lookup_flags(offs_t offset, uX mem_mask) const
{
	return this->m_next->lookup_flags(offset, mem_mask);
}

template<int Width, int AddrShift> std::string handler_entry_write_before_time<Width, AddrShift>::name() const
{
	return util::string_format("(bt %s) %s", m_ws.name(), this->m_next->name());
}


template<int Width, int AddrShift> handler_entry_write_before_time<Width, AddrShift> *handler_entry_write_before_time<Width, AddrShift>::instantiate(handler_entry_write<Width, AddrShift> *next) const
{
	return new handler_entry_write_before_time<Width, AddrShift>(this->m_space, this->m_mph, next, m_ws);
}



template class handler_entry_read_before_time<0,  1>;
template class handler_entry_read_before_time<0,  0>;
template class handler_entry_read_before_time<1,  3>;
template class handler_entry_read_before_time<1,  0>;
template class handler_entry_read_before_time<1, -1>;
template class handler_entry_read_before_time<2,  3>;
template class handler_entry_read_before_time<2,  0>;
template class handler_entry_read_before_time<2, -1>;
template class handler_entry_read_before_time<2, -2>;
template class handler_entry_read_before_time<3,  0>;
template class handler_entry_read_before_time<3, -1>;
template class handler_entry_read_before_time<3, -2>;
template class handler_entry_read_before_time<3, -3>;

template class handler_entry_write_before_time<0,  1>;
template class handler_entry_write_before_time<0,  0>;
template class handler_entry_write_before_time<1,  3>;
template class handler_entry_write_before_time<1,  0>;
template class handler_entry_write_before_time<1, -1>;
template class handler_entry_write_before_time<2,  3>;
template class handler_entry_write_before_time<2,  0>;
template class handler_entry_write_before_time<2, -1>;
template class handler_entry_write_before_time<2, -2>;
template class handler_entry_write_before_time<3,  0>;
template class handler_entry_write_before_time<3, -1>;
template class handler_entry_write_before_time<3, -2>;
template class handler_entry_write_before_time<3, -3>;



template<int Width, int AddrShift> emu::detail::handler_entry_size_t<Width> handler_entry_read_before_delay<Width, AddrShift>::read(offs_t offset, uX mem_mask) const
{
	return this->m_next->read(offset, mem_mask);
}

template<int Width, int AddrShift> emu::detail::handler_entry_size_t<Width> handler_entry_read_before_delay<Width, AddrShift>::read_interruptible(offs_t offset, uX mem_mask) const
{
	if(m_cpu.access_before_delay(m_ws(offset), this))
		return 0;

	return this->m_next->read_interruptible(offset, mem_mask);
}

template<int Width, int AddrShift> std::pair<emu::detail::handler_entry_size_t<Width>, u16> handler_entry_read_before_delay<Width, AddrShift>::read_flags(offs_t offset, uX mem_mask) const
{
	return this->m_next->read_flags(offset, mem_mask);
}

template<int Width, int AddrShift> u16 handler_entry_read_before_delay<Width, AddrShift>::lookup_flags(offs_t offset, uX mem_mask) const
{
	return this->m_next->lookup_flags(offset, mem_mask);
}

template<int Width, int AddrShift> std::string handler_entry_read_before_delay<Width, AddrShift>::name() const
{
	return util::string_format("(bd %s) %s", m_ws.name(), this->m_next->name());
}

template<int Width, int AddrShift> handler_entry_read_before_delay<Width, AddrShift> *handler_entry_read_before_delay<Width, AddrShift>::instantiate(handler_entry_read<Width, AddrShift> *next) const
{
	return new handler_entry_read_before_delay<Width, AddrShift>(this->m_space, this->m_mph, next, m_ws);
}


template<int Width, int AddrShift> void handler_entry_write_before_delay<Width, AddrShift>::write(offs_t offset, uX data, uX mem_mask) const
{
	this->m_next->write(offset, data, mem_mask);
}

template<int Width, int AddrShift> void handler_entry_write_before_delay<Width, AddrShift>::write_interruptible(offs_t offset, uX data, uX mem_mask) const
{
	if(m_cpu.access_before_delay(m_ws(offset), this))
		return;
	this->m_next->write_interruptible(offset, data, mem_mask);
}

template<int Width, int AddrShift> u16 handler_entry_write_before_delay<Width, AddrShift>::write_flags(offs_t offset, uX data, uX mem_mask) const
{
	return this->m_next->write_flags(offset, data, mem_mask);
}

template<int Width, int AddrShift> u16 handler_entry_write_before_delay<Width, AddrShift>::lookup_flags(offs_t offset, uX mem_mask) const
{
	return this->m_next->lookup_flags(offset, mem_mask);
}

template<int Width, int AddrShift> std::string handler_entry_write_before_delay<Width, AddrShift>::name() const
{
	return util::string_format("(bd %s) %s", m_ws.name(), this->m_next->name());
}


template<int Width, int AddrShift> handler_entry_write_before_delay<Width, AddrShift> *handler_entry_write_before_delay<Width, AddrShift>::instantiate(handler_entry_write<Width, AddrShift> *next) const
{
	return new handler_entry_write_before_delay<Width, AddrShift>(this->m_space, this->m_mph, next, m_ws);
}



template class handler_entry_read_before_delay<0,  1>;
template class handler_entry_read_before_delay<0,  0>;
template class handler_entry_read_before_delay<1,  3>;
template class handler_entry_read_before_delay<1,  0>;
template class handler_entry_read_before_delay<1, -1>;
template class handler_entry_read_before_delay<2,  3>;
template class handler_entry_read_before_delay<2,  0>;
template class handler_entry_read_before_delay<2, -1>;
template class handler_entry_read_before_delay<2, -2>;
template class handler_entry_read_before_delay<3,  0>;
template class handler_entry_read_before_delay<3, -1>;
template class handler_entry_read_before_delay<3, -2>;
template class handler_entry_read_before_delay<3, -3>;

template class handler_entry_write_before_delay<0,  1>;
template class handler_entry_write_before_delay<0,  0>;
template class handler_entry_write_before_delay<1,  3>;
template class handler_entry_write_before_delay<1,  0>;
template class handler_entry_write_before_delay<1, -1>;
template class handler_entry_write_before_delay<2,  3>;
template class handler_entry_write_before_delay<2,  0>;
template class handler_entry_write_before_delay<2, -1>;
template class handler_entry_write_before_delay<2, -2>;
template class handler_entry_write_before_delay<3,  0>;
template class handler_entry_write_before_delay<3, -1>;
template class handler_entry_write_before_delay<3, -2>;
template class handler_entry_write_before_delay<3, -3>;



template<int Width, int AddrShift> emu::detail::handler_entry_size_t<Width> handler_entry_read_after_delay<Width, AddrShift>::read(offs_t offset, uX mem_mask) const
{
	return this->m_next->read(offset, mem_mask);
}

template<int Width, int AddrShift> emu::detail::handler_entry_size_t<Width> handler_entry_read_after_delay<Width, AddrShift>::read_interruptible(offs_t offset, uX mem_mask) const
{
	auto r = this->m_next->read_interruptible(offset, mem_mask);
	m_cpu.access_after_delay(m_ws(offset));
	return r;
}

template<int Width, int AddrShift> std::pair<emu::detail::handler_entry_size_t<Width>, u16> handler_entry_read_after_delay<Width, AddrShift>::read_flags(offs_t offset, uX mem_mask) const
{
	return this->m_next->read_flags(offset, mem_mask);
}

template<int Width, int AddrShift> u16 handler_entry_read_after_delay<Width, AddrShift>::lookup_flags(offs_t offset, uX mem_mask) const
{
	return this->m_next->lookup_flags(offset, mem_mask);
}

template<int Width, int AddrShift> std::string handler_entry_read_after_delay<Width, AddrShift>::name() const
{
	return util::string_format("(ad %s) %s", m_ws.name(), this->m_next->name());
}

template<int Width, int AddrShift> handler_entry_read_after_delay<Width, AddrShift> *handler_entry_read_after_delay<Width, AddrShift>::instantiate(handler_entry_read<Width, AddrShift> *next) const
{
	return new handler_entry_read_after_delay<Width, AddrShift>(this->m_space, this->m_mph, next, m_ws);
}


template<int Width, int AddrShift> void handler_entry_write_after_delay<Width, AddrShift>::write(offs_t offset, uX data, uX mem_mask) const
{
	this->m_next->write(offset, data, mem_mask);
}

template<int Width, int AddrShift> void handler_entry_write_after_delay<Width, AddrShift>::write_interruptible(offs_t offset, uX data, uX mem_mask) const
{
	this->m_next->write_interruptible(offset, data, mem_mask);
	m_cpu.access_after_delay(m_ws(offset));
}

template<int Width, int AddrShift> u16 handler_entry_write_after_delay<Width, AddrShift>::write_flags(offs_t offset, uX data, uX mem_mask) const
{
	return this->m_next->write_flags(offset, data, mem_mask);
}

template<int Width, int AddrShift> u16 handler_entry_write_after_delay<Width, AddrShift>::lookup_flags(offs_t offset, uX mem_mask) const
{
	return this->m_next->lookup_flags(offset, mem_mask);
}

template<int Width, int AddrShift> std::string handler_entry_write_after_delay<Width, AddrShift>::name() const
{
	return util::string_format("(ad %s) %s", m_ws.name(), this->m_next->name());
}


template<int Width, int AddrShift> handler_entry_write_after_delay<Width, AddrShift> *handler_entry_write_after_delay<Width, AddrShift>::instantiate(handler_entry_write<Width, AddrShift> *next) const
{
	return new handler_entry_write_after_delay<Width, AddrShift>(this->m_space, this->m_mph, next, m_ws);
}



template class handler_entry_read_after_delay<0,  1>;
template class handler_entry_read_after_delay<0,  0>;
template class handler_entry_read_after_delay<1,  3>;
template class handler_entry_read_after_delay<1,  0>;
template class handler_entry_read_after_delay<1, -1>;
template class handler_entry_read_after_delay<2,  3>;
template class handler_entry_read_after_delay<2,  0>;
template class handler_entry_read_after_delay<2, -1>;
template class handler_entry_read_after_delay<2, -2>;
template class handler_entry_read_after_delay<3,  0>;
template class handler_entry_read_after_delay<3, -1>;
template class handler_entry_read_after_delay<3, -2>;
template class handler_entry_read_after_delay<3, -3>;

template class handler_entry_write_after_delay<0,  1>;
template class handler_entry_write_after_delay<0,  0>;
template class handler_entry_write_after_delay<1,  3>;
template class handler_entry_write_after_delay<1,  0>;
template class handler_entry_write_after_delay<1, -1>;
template class handler_entry_write_after_delay<2,  3>;
template class handler_entry_write_after_delay<2,  0>;
template class handler_entry_write_after_delay<2, -1>;
template class handler_entry_write_after_delay<2, -2>;
template class handler_entry_write_after_delay<3,  0>;
template class handler_entry_write_after_delay<3, -1>;
template class handler_entry_write_after_delay<3, -2>;
template class handler_entry_write_after_delay<3, -3>;
