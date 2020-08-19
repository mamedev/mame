// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "emumem_hea.h"
#include "emumem_hem.h"

template<int Width, int AddrShift, endianness_t Endian> typename emu::detail::handler_entry_size<Width>::uX handler_entry_read_memory<Width, AddrShift, Endian>::read(offs_t offset, uX mem_mask) const
{
	return m_base[((offset - inh::m_address_base) & inh::m_address_mask) >> (Width + AddrShift)];
}

template<int Width, int AddrShift, endianness_t Endian> void *handler_entry_read_memory<Width, AddrShift, Endian>::get_ptr(offs_t offset) const
{
	return m_base + (((offset - inh::m_address_base) & inh::m_address_mask) >> (Width + AddrShift));
}

template<int Width, int AddrShift, endianness_t Endian> std::string handler_entry_read_memory<Width, AddrShift, Endian>::name() const
{
	return util::string_format("memory@%x", inh::m_address_base);
}


template<int Width, int AddrShift, endianness_t Endian> void handler_entry_write_memory<Width, AddrShift, Endian>::write(offs_t offset, uX data, uX mem_mask) const
{
	offs_t off = ((offset - inh::m_address_base) & inh::m_address_mask) >> (Width + AddrShift);
	m_base[off] = (m_base[off] & ~mem_mask) | (data & mem_mask);
}

template<> void handler_entry_write_memory<0, 0, ENDIANNESS_LITTLE>::write(offs_t offset, u8 data, u8 mem_mask) const
{
	m_base[(offset - inh::m_address_base) & inh::m_address_mask] = data;
}

template<> void handler_entry_write_memory<0, 0, ENDIANNESS_BIG>::write(offs_t offset, u8 data, u8 mem_mask) const
{
	m_base[(offset - inh::m_address_base) & inh::m_address_mask] = data;
}

template<int Width, int AddrShift, endianness_t Endian> void *handler_entry_write_memory<Width, AddrShift, Endian>::get_ptr(offs_t offset) const
{
	return m_base + (((offset - inh::m_address_base) & inh::m_address_mask) >> (Width + AddrShift));
}

template<int Width, int AddrShift, endianness_t Endian> std::string handler_entry_write_memory<Width, AddrShift, Endian>::name() const
{
	return util::string_format("memory@%x", inh::m_address_base);
}





template<int Width, int AddrShift, endianness_t Endian> typename emu::detail::handler_entry_size<Width>::uX handler_entry_read_memory_bank<Width, AddrShift, Endian>::read(offs_t offset, uX mem_mask) const
{
	return static_cast<uX *>(m_bank.base())[((offset - inh::m_address_base) & inh::m_address_mask) >> (Width + AddrShift)];
}

template<int Width, int AddrShift, endianness_t Endian> void *handler_entry_read_memory_bank<Width, AddrShift, Endian>::get_ptr(offs_t offset) const
{
	return static_cast<uX *>(m_bank.base()) + (((offset - inh::m_address_base) & inh::m_address_mask) >> (Width + AddrShift));
}

template<int Width, int AddrShift, endianness_t Endian> std::string handler_entry_read_memory_bank<Width, AddrShift, Endian>::name() const
{
	return m_bank.name();
}


template<int Width, int AddrShift, endianness_t Endian> void handler_entry_write_memory_bank<Width, AddrShift, Endian>::write(offs_t offset, uX data, uX mem_mask) const
{
	offs_t off = ((offset - inh::m_address_base) & inh::m_address_mask) >> (Width + AddrShift);
	static_cast<uX *>(m_bank.base())[off] = (static_cast<uX *>(m_bank.base())[off] & ~mem_mask) | (data & mem_mask);
}

template<> void handler_entry_write_memory_bank<0, 0, ENDIANNESS_LITTLE>::write(offs_t offset, u8 data, u8 mem_mask) const
{
	static_cast<uX *>(m_bank.base())[(offset - inh::m_address_base) & inh::m_address_mask] = data;
}

template<> void handler_entry_write_memory_bank<0, 0, ENDIANNESS_BIG>::write(offs_t offset, u8 data, u8 mem_mask) const
{
	static_cast<uX *>(m_bank.base())[(offset - inh::m_address_base) & inh::m_address_mask] = data;
}

template<int Width, int AddrShift, endianness_t Endian> void *handler_entry_write_memory_bank<Width, AddrShift, Endian>::get_ptr(offs_t offset) const
{
	return static_cast<uX *>(m_bank.base()) + (((offset - inh::m_address_base) & inh::m_address_mask) >> (Width + AddrShift));
}

template<int Width, int AddrShift, endianness_t Endian> std::string handler_entry_write_memory_bank<Width, AddrShift, Endian>::name() const
{
	return m_bank.name();
}



template class handler_entry_read_memory<0,  1, ENDIANNESS_LITTLE>;
template class handler_entry_read_memory<0,  1, ENDIANNESS_BIG>;
template class handler_entry_read_memory<0,  0, ENDIANNESS_LITTLE>;
template class handler_entry_read_memory<0,  0, ENDIANNESS_BIG>;
template class handler_entry_read_memory<1,  3, ENDIANNESS_LITTLE>;
template class handler_entry_read_memory<1,  3, ENDIANNESS_BIG>;
template class handler_entry_read_memory<1,  0, ENDIANNESS_LITTLE>;
template class handler_entry_read_memory<1,  0, ENDIANNESS_BIG>;
template class handler_entry_read_memory<1, -1, ENDIANNESS_LITTLE>;
template class handler_entry_read_memory<1, -1, ENDIANNESS_BIG>;
template class handler_entry_read_memory<2,  3, ENDIANNESS_LITTLE>;
template class handler_entry_read_memory<2,  3, ENDIANNESS_BIG>;
template class handler_entry_read_memory<2,  0, ENDIANNESS_LITTLE>;
template class handler_entry_read_memory<2,  0, ENDIANNESS_BIG>;
template class handler_entry_read_memory<2, -1, ENDIANNESS_LITTLE>;
template class handler_entry_read_memory<2, -1, ENDIANNESS_BIG>;
template class handler_entry_read_memory<2, -2, ENDIANNESS_LITTLE>;
template class handler_entry_read_memory<2, -2, ENDIANNESS_BIG>;
template class handler_entry_read_memory<3,  0, ENDIANNESS_LITTLE>;
template class handler_entry_read_memory<3,  0, ENDIANNESS_BIG>;
template class handler_entry_read_memory<3, -1, ENDIANNESS_LITTLE>;
template class handler_entry_read_memory<3, -1, ENDIANNESS_BIG>;
template class handler_entry_read_memory<3, -2, ENDIANNESS_LITTLE>;
template class handler_entry_read_memory<3, -2, ENDIANNESS_BIG>;
template class handler_entry_read_memory<3, -3, ENDIANNESS_LITTLE>;
template class handler_entry_read_memory<3, -3, ENDIANNESS_BIG>;

template class handler_entry_write_memory<0,  1, ENDIANNESS_LITTLE>;
template class handler_entry_write_memory<0,  1, ENDIANNESS_BIG>;
template class handler_entry_write_memory<0,  0, ENDIANNESS_LITTLE>;
template class handler_entry_write_memory<0,  0, ENDIANNESS_BIG>;
template class handler_entry_write_memory<1,  3, ENDIANNESS_LITTLE>;
template class handler_entry_write_memory<1,  3, ENDIANNESS_BIG>;
template class handler_entry_write_memory<1,  0, ENDIANNESS_LITTLE>;
template class handler_entry_write_memory<1,  0, ENDIANNESS_BIG>;
template class handler_entry_write_memory<1, -1, ENDIANNESS_LITTLE>;
template class handler_entry_write_memory<1, -1, ENDIANNESS_BIG>;
template class handler_entry_write_memory<2,  3, ENDIANNESS_LITTLE>;
template class handler_entry_write_memory<2,  3, ENDIANNESS_BIG>;
template class handler_entry_write_memory<2,  0, ENDIANNESS_LITTLE>;
template class handler_entry_write_memory<2,  0, ENDIANNESS_BIG>;
template class handler_entry_write_memory<2, -1, ENDIANNESS_LITTLE>;
template class handler_entry_write_memory<2, -1, ENDIANNESS_BIG>;
template class handler_entry_write_memory<2, -2, ENDIANNESS_LITTLE>;
template class handler_entry_write_memory<2, -2, ENDIANNESS_BIG>;
template class handler_entry_write_memory<3,  0, ENDIANNESS_LITTLE>;
template class handler_entry_write_memory<3,  0, ENDIANNESS_BIG>;
template class handler_entry_write_memory<3, -1, ENDIANNESS_LITTLE>;
template class handler_entry_write_memory<3, -1, ENDIANNESS_BIG>;
template class handler_entry_write_memory<3, -2, ENDIANNESS_LITTLE>;
template class handler_entry_write_memory<3, -2, ENDIANNESS_BIG>;
template class handler_entry_write_memory<3, -3, ENDIANNESS_LITTLE>;
template class handler_entry_write_memory<3, -3, ENDIANNESS_BIG>;


template class handler_entry_read_memory_bank<0,  1, ENDIANNESS_LITTLE>;
template class handler_entry_read_memory_bank<0,  1, ENDIANNESS_BIG>;
template class handler_entry_read_memory_bank<0,  0, ENDIANNESS_LITTLE>;
template class handler_entry_read_memory_bank<0,  0, ENDIANNESS_BIG>;
template class handler_entry_read_memory_bank<1,  3, ENDIANNESS_LITTLE>;
template class handler_entry_read_memory_bank<1,  3, ENDIANNESS_BIG>;
template class handler_entry_read_memory_bank<1,  0, ENDIANNESS_LITTLE>;
template class handler_entry_read_memory_bank<1,  0, ENDIANNESS_BIG>;
template class handler_entry_read_memory_bank<1, -1, ENDIANNESS_LITTLE>;
template class handler_entry_read_memory_bank<1, -1, ENDIANNESS_BIG>;
template class handler_entry_read_memory_bank<2,  3, ENDIANNESS_LITTLE>;
template class handler_entry_read_memory_bank<2,  3, ENDIANNESS_BIG>;
template class handler_entry_read_memory_bank<2,  0, ENDIANNESS_LITTLE>;
template class handler_entry_read_memory_bank<2,  0, ENDIANNESS_BIG>;
template class handler_entry_read_memory_bank<2, -1, ENDIANNESS_LITTLE>;
template class handler_entry_read_memory_bank<2, -1, ENDIANNESS_BIG>;
template class handler_entry_read_memory_bank<2, -2, ENDIANNESS_LITTLE>;
template class handler_entry_read_memory_bank<2, -2, ENDIANNESS_BIG>;
template class handler_entry_read_memory_bank<3,  0, ENDIANNESS_LITTLE>;
template class handler_entry_read_memory_bank<3,  0, ENDIANNESS_BIG>;
template class handler_entry_read_memory_bank<3, -1, ENDIANNESS_LITTLE>;
template class handler_entry_read_memory_bank<3, -1, ENDIANNESS_BIG>;
template class handler_entry_read_memory_bank<3, -2, ENDIANNESS_LITTLE>;
template class handler_entry_read_memory_bank<3, -2, ENDIANNESS_BIG>;
template class handler_entry_read_memory_bank<3, -3, ENDIANNESS_LITTLE>;
template class handler_entry_read_memory_bank<3, -3, ENDIANNESS_BIG>;

template class handler_entry_write_memory_bank<0,  1, ENDIANNESS_LITTLE>;
template class handler_entry_write_memory_bank<0,  1, ENDIANNESS_BIG>;
template class handler_entry_write_memory_bank<0,  0, ENDIANNESS_LITTLE>;
template class handler_entry_write_memory_bank<0,  0, ENDIANNESS_BIG>;
template class handler_entry_write_memory_bank<1,  3, ENDIANNESS_LITTLE>;
template class handler_entry_write_memory_bank<1,  3, ENDIANNESS_BIG>;
template class handler_entry_write_memory_bank<1,  0, ENDIANNESS_LITTLE>;
template class handler_entry_write_memory_bank<1,  0, ENDIANNESS_BIG>;
template class handler_entry_write_memory_bank<1, -1, ENDIANNESS_LITTLE>;
template class handler_entry_write_memory_bank<1, -1, ENDIANNESS_BIG>;
template class handler_entry_write_memory_bank<2,  3, ENDIANNESS_LITTLE>;
template class handler_entry_write_memory_bank<2,  3, ENDIANNESS_BIG>;
template class handler_entry_write_memory_bank<2,  0, ENDIANNESS_LITTLE>;
template class handler_entry_write_memory_bank<2,  0, ENDIANNESS_BIG>;
template class handler_entry_write_memory_bank<2, -1, ENDIANNESS_LITTLE>;
template class handler_entry_write_memory_bank<2, -1, ENDIANNESS_BIG>;
template class handler_entry_write_memory_bank<2, -2, ENDIANNESS_LITTLE>;
template class handler_entry_write_memory_bank<2, -2, ENDIANNESS_BIG>;
template class handler_entry_write_memory_bank<3,  0, ENDIANNESS_LITTLE>;
template class handler_entry_write_memory_bank<3,  0, ENDIANNESS_BIG>;
template class handler_entry_write_memory_bank<3, -1, ENDIANNESS_LITTLE>;
template class handler_entry_write_memory_bank<3, -1, ENDIANNESS_BIG>;
template class handler_entry_write_memory_bank<3, -2, ENDIANNESS_LITTLE>;
template class handler_entry_write_memory_bank<3, -2, ENDIANNESS_BIG>;
template class handler_entry_write_memory_bank<3, -3, ENDIANNESS_LITTLE>;
template class handler_entry_write_memory_bank<3, -3, ENDIANNESS_BIG>;
