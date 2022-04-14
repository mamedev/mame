// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "emumem_hea.h"
#include "emumem_hem.h"

template<int Width, int AddrShift> typename emu::detail::handler_entry_size<Width>::uX handler_entry_read_memory<Width, AddrShift>::read(offs_t offset, uX mem_mask) const
{
	return m_base[((offset - this->m_address_base) & this->m_address_mask) >> (Width + AddrShift)];
}

template<int Width, int AddrShift> std::pair<typename emu::detail::handler_entry_size<Width>::uX, u16> handler_entry_read_memory<Width, AddrShift>::read_flags(offs_t offset, uX mem_mask) const
{
	return std::pair<uX, u16>(m_base[((offset - this->m_address_base) & this->m_address_mask) >> (Width + AddrShift)], this->m_flags);
}

template<int Width, int AddrShift> void *handler_entry_read_memory<Width, AddrShift>::get_ptr(offs_t offset) const
{
	return m_base + (((offset - this->m_address_base) & this->m_address_mask) >> (Width + AddrShift));
}

template<int Width, int AddrShift> std::string handler_entry_read_memory<Width, AddrShift>::name() const
{
	return util::string_format("memory@%x", this->m_address_base);
}


template<int Width, int AddrShift> void handler_entry_write_memory<Width, AddrShift>::write(offs_t offset, uX data, uX mem_mask) const
{
	offs_t off = ((offset - this->m_address_base) & this->m_address_mask) >> (Width + AddrShift);
	m_base[off] = (m_base[off] & ~mem_mask) | (data & mem_mask);
}

template<int Width, int AddrShift> u16 handler_entry_write_memory<Width, AddrShift>::write_flags(offs_t offset, uX data, uX mem_mask) const
{
	offs_t off = ((offset - this->m_address_base) & this->m_address_mask) >> (Width + AddrShift);
	m_base[off] = (m_base[off] & ~mem_mask) | (data & mem_mask);
	return this->m_flags;
}

template<> void handler_entry_write_memory<0, 0>::write(offs_t offset, u8 data, u8 mem_mask) const
{
	m_base[(offset - this->m_address_base) & this->m_address_mask] = data;
}

template<> u16 handler_entry_write_memory<0, 0>::write_flags(offs_t offset, u8 data, u8 mem_mask) const
{
	m_base[(offset - this->m_address_base) & this->m_address_mask] = data;
	return this->m_flags;
}

template<int Width, int AddrShift> void *handler_entry_write_memory<Width, AddrShift>::get_ptr(offs_t offset) const
{
	return m_base + (((offset - this->m_address_base) & this->m_address_mask) >> (Width + AddrShift));
}

template<int Width, int AddrShift> std::string handler_entry_write_memory<Width, AddrShift>::name() const
{
	return util::string_format("memory@%x", this->m_address_base);
}





template<int Width, int AddrShift> typename emu::detail::handler_entry_size<Width>::uX handler_entry_read_memory_bank<Width, AddrShift>::read(offs_t offset, uX mem_mask) const
{
	return static_cast<uX *>(m_bank.base())[((offset - this->m_address_base) & this->m_address_mask) >> (Width + AddrShift)];
}

template<int Width, int AddrShift> std::pair<typename emu::detail::handler_entry_size<Width>::uX, u16> handler_entry_read_memory_bank<Width, AddrShift>::read_flags(offs_t offset, uX mem_mask) const
{
	return std::pair<uX, u16>(static_cast<uX *>(m_bank.base())[((offset - this->m_address_base) & this->m_address_mask) >> (Width + AddrShift)], this->m_flags);
}

template<int Width, int AddrShift> void *handler_entry_read_memory_bank<Width, AddrShift>::get_ptr(offs_t offset) const
{
	return static_cast<uX *>(m_bank.base()) + (((offset - this->m_address_base) & this->m_address_mask) >> (Width + AddrShift));
}

template<int Width, int AddrShift> std::string handler_entry_read_memory_bank<Width, AddrShift>::name() const
{
	return m_bank.name();
}


template<int Width, int AddrShift> void handler_entry_write_memory_bank<Width, AddrShift>::write(offs_t offset, uX data, uX mem_mask) const
{
	offs_t off = ((offset - this->m_address_base) & this->m_address_mask) >> (Width + AddrShift);
	static_cast<uX *>(m_bank.base())[off] = (static_cast<uX *>(m_bank.base())[off] & ~mem_mask) | (data & mem_mask);
}

template<int Width, int AddrShift> u16 handler_entry_write_memory_bank<Width, AddrShift>::write_flags(offs_t offset, uX data, uX mem_mask) const
{
	offs_t off = ((offset - this->m_address_base) & this->m_address_mask) >> (Width + AddrShift);
	static_cast<uX *>(m_bank.base())[off] = (static_cast<uX *>(m_bank.base())[off] & ~mem_mask) | (data & mem_mask);
	return this->m_flags;
}

template<> void handler_entry_write_memory_bank<0, 0>::write(offs_t offset, u8 data, u8 mem_mask) const
{
	static_cast<uX *>(m_bank.base())[(offset - this->m_address_base) & this->m_address_mask] = data;
}

template<> u16 handler_entry_write_memory_bank<0, 0>::write_flags(offs_t offset, u8 data, u8 mem_mask) const
{
	static_cast<uX *>(m_bank.base())[(offset - this->m_address_base) & this->m_address_mask] = data;
	return this->m_flags;
}

template<int Width, int AddrShift> void *handler_entry_write_memory_bank<Width, AddrShift>::get_ptr(offs_t offset) const
{
	return static_cast<uX *>(m_bank.base()) + (((offset - this->m_address_base) & this->m_address_mask) >> (Width + AddrShift));
}

template<int Width, int AddrShift> std::string handler_entry_write_memory_bank<Width, AddrShift>::name() const
{
	return m_bank.name();
}



template class handler_entry_read_memory<0,  1>;
template class handler_entry_read_memory<0,  0>;
template class handler_entry_read_memory<1,  3>;
template class handler_entry_read_memory<1,  0>;
template class handler_entry_read_memory<1, -1>;
template class handler_entry_read_memory<2,  3>;
template class handler_entry_read_memory<2,  0>;
template class handler_entry_read_memory<2, -1>;
template class handler_entry_read_memory<2, -2>;
template class handler_entry_read_memory<3,  0>;
template class handler_entry_read_memory<3, -1>;
template class handler_entry_read_memory<3, -2>;
template class handler_entry_read_memory<3, -3>;

template class handler_entry_write_memory<0,  1>;
template class handler_entry_write_memory<0,  0>;
template class handler_entry_write_memory<1,  3>;
template class handler_entry_write_memory<1,  0>;
template class handler_entry_write_memory<1, -1>;
template class handler_entry_write_memory<2,  3>;
template class handler_entry_write_memory<2,  0>;
template class handler_entry_write_memory<2, -1>;
template class handler_entry_write_memory<2, -2>;
template class handler_entry_write_memory<3,  0>;
template class handler_entry_write_memory<3, -1>;
template class handler_entry_write_memory<3, -2>;
template class handler_entry_write_memory<3, -3>;


template class handler_entry_read_memory_bank<0,  1>;
template class handler_entry_read_memory_bank<0,  0>;
template class handler_entry_read_memory_bank<1,  3>;
template class handler_entry_read_memory_bank<1,  0>;
template class handler_entry_read_memory_bank<1, -1>;
template class handler_entry_read_memory_bank<2,  3>;
template class handler_entry_read_memory_bank<2,  0>;
template class handler_entry_read_memory_bank<2, -1>;
template class handler_entry_read_memory_bank<2, -2>;
template class handler_entry_read_memory_bank<3,  0>;
template class handler_entry_read_memory_bank<3, -1>;
template class handler_entry_read_memory_bank<3, -2>;
template class handler_entry_read_memory_bank<3, -3>;

template class handler_entry_write_memory_bank<0,  1>;
template class handler_entry_write_memory_bank<0,  0>;
template class handler_entry_write_memory_bank<1,  3>;
template class handler_entry_write_memory_bank<1,  0>;
template class handler_entry_write_memory_bank<1, -1>;
template class handler_entry_write_memory_bank<2,  3>;
template class handler_entry_write_memory_bank<2,  0>;
template class handler_entry_write_memory_bank<2, -1>;
template class handler_entry_write_memory_bank<2, -2>;
template class handler_entry_write_memory_bank<3,  0>;
template class handler_entry_write_memory_bank<3, -1>;
template class handler_entry_write_memory_bank<3, -2>;
template class handler_entry_write_memory_bank<3, -3>;
