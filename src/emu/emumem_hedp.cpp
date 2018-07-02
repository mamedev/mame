// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "emumem_hea.h"
#include "emumem_hedp.h"

template<int Width, int AddrShift, int Endian> typename handler_entry_size<Width>::uX handler_entry_read_delegate<Width, AddrShift, Endian>::read(offs_t offset, uX mem_mask)
{
	return m_delegate(*inh::m_space, ((offset - inh::m_address_base) & inh::m_address_mask) >> (Width + AddrShift), mem_mask);
}

template<int Width, int AddrShift, int Endian> std::string handler_entry_read_delegate<Width, AddrShift, Endian>::name() const
{
	return m_delegate.name();
}

template<int Width, int AddrShift, int Endian> void handler_entry_write_delegate<Width, AddrShift, Endian>::write(offs_t offset, uX data, uX mem_mask)
{
	m_delegate(*inh::m_space, ((offset - inh::m_address_base) & inh::m_address_mask) >> (Width + AddrShift), data, mem_mask);
}

template<int Width, int AddrShift, int Endian> std::string handler_entry_write_delegate<Width, AddrShift, Endian>::name() const
{
	return m_delegate.name();
}




template<int Width, int AddrShift, int Endian> typename handler_entry_size<Width>::uX handler_entry_read_ioport<Width, AddrShift, Endian>::read(offs_t offset, uX mem_mask)
{
	return m_port->read();
}

template<int Width, int AddrShift, int Endian> std::string handler_entry_read_ioport<Width, AddrShift, Endian>::name() const
{
	return m_port->tag();
}

template<int Width, int AddrShift, int Endian> void handler_entry_write_ioport<Width, AddrShift, Endian>::write(offs_t offset, uX data, uX mem_mask)
{
	m_port->write(data, mem_mask);
}

template<int Width, int AddrShift, int Endian> std::string handler_entry_write_ioport<Width, AddrShift, Endian>::name() const
{
	return m_port->tag();
}



template class handler_entry_read_delegate<0,  0, ENDIANNESS_LITTLE>;
template class handler_entry_read_delegate<0,  0, ENDIANNESS_BIG>;
template class handler_entry_read_delegate<1,  3, ENDIANNESS_LITTLE>;
template class handler_entry_read_delegate<1,  3, ENDIANNESS_BIG>;
template class handler_entry_read_delegate<1,  0, ENDIANNESS_LITTLE>;
template class handler_entry_read_delegate<1,  0, ENDIANNESS_BIG>;
template class handler_entry_read_delegate<1, -1, ENDIANNESS_LITTLE>;
template class handler_entry_read_delegate<1, -1, ENDIANNESS_BIG>;
template class handler_entry_read_delegate<2,  0, ENDIANNESS_LITTLE>;
template class handler_entry_read_delegate<2,  0, ENDIANNESS_BIG>;
template class handler_entry_read_delegate<2, -1, ENDIANNESS_LITTLE>;
template class handler_entry_read_delegate<2, -1, ENDIANNESS_BIG>;
template class handler_entry_read_delegate<2, -2, ENDIANNESS_LITTLE>;
template class handler_entry_read_delegate<2, -2, ENDIANNESS_BIG>;
template class handler_entry_read_delegate<3,  0, ENDIANNESS_LITTLE>;
template class handler_entry_read_delegate<3,  0, ENDIANNESS_BIG>;
template class handler_entry_read_delegate<3, -1, ENDIANNESS_LITTLE>;
template class handler_entry_read_delegate<3, -1, ENDIANNESS_BIG>;
template class handler_entry_read_delegate<3, -2, ENDIANNESS_LITTLE>;
template class handler_entry_read_delegate<3, -2, ENDIANNESS_BIG>;
template class handler_entry_read_delegate<3, -3, ENDIANNESS_LITTLE>;
template class handler_entry_read_delegate<3, -3, ENDIANNESS_BIG>;

template class handler_entry_write_delegate<0,  0, ENDIANNESS_LITTLE>;
template class handler_entry_write_delegate<0,  0, ENDIANNESS_BIG>;
template class handler_entry_write_delegate<1,  3, ENDIANNESS_LITTLE>;
template class handler_entry_write_delegate<1,  3, ENDIANNESS_BIG>;
template class handler_entry_write_delegate<1,  0, ENDIANNESS_LITTLE>;
template class handler_entry_write_delegate<1,  0, ENDIANNESS_BIG>;
template class handler_entry_write_delegate<1, -1, ENDIANNESS_LITTLE>;
template class handler_entry_write_delegate<1, -1, ENDIANNESS_BIG>;
template class handler_entry_write_delegate<2,  0, ENDIANNESS_LITTLE>;
template class handler_entry_write_delegate<2,  0, ENDIANNESS_BIG>;
template class handler_entry_write_delegate<2, -1, ENDIANNESS_LITTLE>;
template class handler_entry_write_delegate<2, -1, ENDIANNESS_BIG>;
template class handler_entry_write_delegate<2, -2, ENDIANNESS_LITTLE>;
template class handler_entry_write_delegate<2, -2, ENDIANNESS_BIG>;
template class handler_entry_write_delegate<3,  0, ENDIANNESS_LITTLE>;
template class handler_entry_write_delegate<3,  0, ENDIANNESS_BIG>;
template class handler_entry_write_delegate<3, -1, ENDIANNESS_LITTLE>;
template class handler_entry_write_delegate<3, -1, ENDIANNESS_BIG>;
template class handler_entry_write_delegate<3, -2, ENDIANNESS_LITTLE>;
template class handler_entry_write_delegate<3, -2, ENDIANNESS_BIG>;
template class handler_entry_write_delegate<3, -3, ENDIANNESS_LITTLE>;
template class handler_entry_write_delegate<3, -3, ENDIANNESS_BIG>;


template class handler_entry_read_ioport<0,  0, ENDIANNESS_LITTLE>;
template class handler_entry_read_ioport<0,  0, ENDIANNESS_BIG>;
template class handler_entry_read_ioport<1,  3, ENDIANNESS_LITTLE>;
template class handler_entry_read_ioport<1,  3, ENDIANNESS_BIG>;
template class handler_entry_read_ioport<1,  0, ENDIANNESS_LITTLE>;
template class handler_entry_read_ioport<1,  0, ENDIANNESS_BIG>;
template class handler_entry_read_ioport<1, -1, ENDIANNESS_LITTLE>;
template class handler_entry_read_ioport<1, -1, ENDIANNESS_BIG>;
template class handler_entry_read_ioport<2,  0, ENDIANNESS_LITTLE>;
template class handler_entry_read_ioport<2,  0, ENDIANNESS_BIG>;
template class handler_entry_read_ioport<2, -1, ENDIANNESS_LITTLE>;
template class handler_entry_read_ioport<2, -1, ENDIANNESS_BIG>;
template class handler_entry_read_ioport<2, -2, ENDIANNESS_LITTLE>;
template class handler_entry_read_ioport<2, -2, ENDIANNESS_BIG>;
template class handler_entry_read_ioport<3,  0, ENDIANNESS_LITTLE>;
template class handler_entry_read_ioport<3,  0, ENDIANNESS_BIG>;
template class handler_entry_read_ioport<3, -1, ENDIANNESS_LITTLE>;
template class handler_entry_read_ioport<3, -1, ENDIANNESS_BIG>;
template class handler_entry_read_ioport<3, -2, ENDIANNESS_LITTLE>;
template class handler_entry_read_ioport<3, -2, ENDIANNESS_BIG>;
template class handler_entry_read_ioport<3, -3, ENDIANNESS_LITTLE>;
template class handler_entry_read_ioport<3, -3, ENDIANNESS_BIG>;

template class handler_entry_write_ioport<0,  0, ENDIANNESS_LITTLE>;
template class handler_entry_write_ioport<0,  0, ENDIANNESS_BIG>;
template class handler_entry_write_ioport<1,  3, ENDIANNESS_LITTLE>;
template class handler_entry_write_ioport<1,  3, ENDIANNESS_BIG>;
template class handler_entry_write_ioport<1,  0, ENDIANNESS_LITTLE>;
template class handler_entry_write_ioport<1,  0, ENDIANNESS_BIG>;
template class handler_entry_write_ioport<1, -1, ENDIANNESS_LITTLE>;
template class handler_entry_write_ioport<1, -1, ENDIANNESS_BIG>;
template class handler_entry_write_ioport<2,  0, ENDIANNESS_LITTLE>;
template class handler_entry_write_ioport<2,  0, ENDIANNESS_BIG>;
template class handler_entry_write_ioport<2, -1, ENDIANNESS_LITTLE>;
template class handler_entry_write_ioport<2, -1, ENDIANNESS_BIG>;
template class handler_entry_write_ioport<2, -2, ENDIANNESS_LITTLE>;
template class handler_entry_write_ioport<2, -2, ENDIANNESS_BIG>;
template class handler_entry_write_ioport<3,  0, ENDIANNESS_LITTLE>;
template class handler_entry_write_ioport<3,  0, ENDIANNESS_BIG>;
template class handler_entry_write_ioport<3, -1, ENDIANNESS_LITTLE>;
template class handler_entry_write_ioport<3, -1, ENDIANNESS_BIG>;
template class handler_entry_write_ioport<3, -2, ENDIANNESS_LITTLE>;
template class handler_entry_write_ioport<3, -2, ENDIANNESS_BIG>;
template class handler_entry_write_ioport<3, -3, ENDIANNESS_LITTLE>;
template class handler_entry_write_ioport<3, -3, ENDIANNESS_BIG>;
