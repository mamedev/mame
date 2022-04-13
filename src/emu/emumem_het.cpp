// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "emumem_hep.h"
#include "emumem_het.h"

template<int Width, int AddrShift> typename emu::detail::handler_entry_size<Width>::uX handler_entry_read_tap<Width, AddrShift>::read(offs_t offset, uX mem_mask) const
{
	this->ref();

	uX data = this->m_next->read(offset, mem_mask);
	m_tap(offset, data, mem_mask);

	this->unref();
	return data;
}

template<int Width, int AddrShift> std::pair<typename emu::detail::handler_entry_size<Width>::uX, u16> handler_entry_read_tap<Width, AddrShift>::read_flags(offs_t offset, uX mem_mask) const
{
	this->ref();

	auto pack = this->m_next->read_flags(offset, mem_mask);
	m_tap(offset, pack.first, mem_mask);

	this->unref();
	return pack;
}

template<int Width, int AddrShift> std::string handler_entry_read_tap<Width, AddrShift>::name() const
{
	return '(' + m_name + ") " + this->m_next->name();
}

template<int Width, int AddrShift> handler_entry_read_tap<Width, AddrShift> *handler_entry_read_tap<Width, AddrShift>::instantiate(handler_entry_read<Width, AddrShift> *next) const
{
	return new handler_entry_read_tap<Width, AddrShift>(this->m_space, this->m_mph, next, m_name, m_tap);
}


template<int Width, int AddrShift> void handler_entry_write_tap<Width, AddrShift>::write(offs_t offset, uX data, uX mem_mask) const
{
	this->ref();

	m_tap(offset, data, mem_mask);
	this->m_next->write(offset, data, mem_mask);

	this->unref();
}

template<int Width, int AddrShift> u16 handler_entry_write_tap<Width, AddrShift>::write_flags(offs_t offset, uX data, uX mem_mask) const
{
	this->ref();

	m_tap(offset, data, mem_mask);
	u16 flags = this->m_next->write_flags(offset, data, mem_mask);

	this->unref();
	return flags;
}

template<int Width, int AddrShift> std::string handler_entry_write_tap<Width, AddrShift>::name() const
{
	return '(' + m_name + ") " + this->m_next->name();
}


template<int Width, int AddrShift> handler_entry_write_tap<Width, AddrShift> *handler_entry_write_tap<Width, AddrShift>::instantiate(handler_entry_write<Width, AddrShift> *next) const
{
	return new handler_entry_write_tap<Width, AddrShift>(this->m_space, this->m_mph, next, m_name, m_tap);
}



template class handler_entry_read_tap<0,  1>;
template class handler_entry_read_tap<0,  0>;
template class handler_entry_read_tap<1,  3>;
template class handler_entry_read_tap<1,  0>;
template class handler_entry_read_tap<1, -1>;
template class handler_entry_read_tap<2,  3>;
template class handler_entry_read_tap<2,  0>;
template class handler_entry_read_tap<2, -1>;
template class handler_entry_read_tap<2, -2>;
template class handler_entry_read_tap<3,  0>;
template class handler_entry_read_tap<3, -1>;
template class handler_entry_read_tap<3, -2>;
template class handler_entry_read_tap<3, -3>;

template class handler_entry_write_tap<0,  1>;
template class handler_entry_write_tap<0,  0>;
template class handler_entry_write_tap<1,  3>;
template class handler_entry_write_tap<1,  0>;
template class handler_entry_write_tap<1, -1>;
template class handler_entry_write_tap<2,  3>;
template class handler_entry_write_tap<2,  0>;
template class handler_entry_write_tap<2, -1>;
template class handler_entry_write_tap<2, -2>;
template class handler_entry_write_tap<3,  0>;
template class handler_entry_write_tap<3, -1>;
template class handler_entry_write_tap<3, -2>;
template class handler_entry_write_tap<3, -3>;
