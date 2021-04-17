// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "emumem_hea.h"
#include "emumem_hedp.h"

template<int Width, int AddrShift, endianness_t Endian, typename READ> template<typename R>
	std::enable_if_t<std::is_same<R, read8_delegate>::value ||
					 std::is_same<R, read16_delegate>::value ||
					 std::is_same<R, read32_delegate>::value ||
					 std::is_same<R, read64_delegate>::value,
					 typename emu::detail::handler_entry_size<Width>::uX> handler_entry_read_delegate<Width, AddrShift, Endian, READ>::read_impl(offs_t offset, uX mem_mask) const
{
	return m_delegate(*this->m_space, ((offset - this->m_address_base) & this->m_address_mask) >> (Width + AddrShift), mem_mask);
}

template<int Width, int AddrShift, endianness_t Endian, typename READ> template<typename R>
	std::enable_if_t<std::is_same<R, read8m_delegate>::value ||
					 std::is_same<R, read16m_delegate>::value ||
					 std::is_same<R, read32m_delegate>::value ||
					 std::is_same<R, read64m_delegate>::value,
					 typename emu::detail::handler_entry_size<Width>::uX> handler_entry_read_delegate<Width, AddrShift, Endian, READ>::read_impl(offs_t offset, uX mem_mask) const
{
	return m_delegate(*this->m_space, ((offset - this->m_address_base) & this->m_address_mask) >> (Width + AddrShift));
}

template<int Width, int AddrShift, endianness_t Endian, typename READ> template<typename R>
	std::enable_if_t<std::is_same<R, read8s_delegate>::value ||
					 std::is_same<R, read16s_delegate>::value ||
					 std::is_same<R, read32s_delegate>::value ||
					 std::is_same<R, read64s_delegate>::value,
					 typename emu::detail::handler_entry_size<Width>::uX> handler_entry_read_delegate<Width, AddrShift, Endian, READ>::read_impl(offs_t offset, uX mem_mask) const
{
	return m_delegate(((offset - this->m_address_base) & this->m_address_mask) >> (Width + AddrShift), mem_mask);
}

template<int Width, int AddrShift, endianness_t Endian, typename READ> template<typename R>
	std::enable_if_t<std::is_same<R, read8sm_delegate>::value ||
					 std::is_same<R, read16sm_delegate>::value ||
					 std::is_same<R, read32sm_delegate>::value ||
					 std::is_same<R, read64sm_delegate>::value,
					 typename emu::detail::handler_entry_size<Width>::uX> handler_entry_read_delegate<Width, AddrShift, Endian, READ>::read_impl(offs_t offset, uX mem_mask) const
{
	return m_delegate(((offset - this->m_address_base) & this->m_address_mask) >> (Width + AddrShift));
}

template<int Width, int AddrShift, endianness_t Endian, typename READ> template<typename R>
	std::enable_if_t<std::is_same<R, read8mo_delegate>::value ||
					 std::is_same<R, read16mo_delegate>::value ||
					 std::is_same<R, read32mo_delegate>::value ||
					 std::is_same<R, read64mo_delegate>::value,
					 typename emu::detail::handler_entry_size<Width>::uX> handler_entry_read_delegate<Width, AddrShift, Endian, READ>::read_impl(offs_t offset, uX mem_mask) const
{
	return m_delegate(*this->m_space);
}

template<int Width, int AddrShift, endianness_t Endian, typename READ> template<typename R>
	std::enable_if_t<std::is_same<R, read8smo_delegate>::value ||
					 std::is_same<R, read16smo_delegate>::value ||
					 std::is_same<R, read32smo_delegate>::value ||
					 std::is_same<R, read64smo_delegate>::value,
					 typename emu::detail::handler_entry_size<Width>::uX> handler_entry_read_delegate<Width, AddrShift, Endian, READ>::read_impl(offs_t offset, uX mem_mask) const
{
	return m_delegate();
}

template<int Width, int AddrShift, endianness_t Endian, typename READ> typename emu::detail::handler_entry_size<Width>::uX handler_entry_read_delegate<Width, AddrShift, Endian, READ>::read(offs_t offset, uX mem_mask) const
{
	return read_impl<READ>(offset, mem_mask);
}

template<int Width, int AddrShift, endianness_t Endian, typename READ> std::string handler_entry_read_delegate<Width, AddrShift, Endian, READ>::name() const
{
	return m_delegate.name();
}

template<int Width, int AddrShift, endianness_t Endian, typename WRITE> template<typename W>
	std::enable_if_t<std::is_same<W, write8_delegate>::value ||
					 std::is_same<W, write16_delegate>::value ||
					 std::is_same<W, write32_delegate>::value ||
					 std::is_same<W, write64_delegate>::value,
					 void> handler_entry_write_delegate<Width, AddrShift, Endian, WRITE>::write_impl(offs_t offset, uX data, uX mem_mask) const
{
	m_delegate(*this->m_space, ((offset - this->m_address_base) & this->m_address_mask) >> (Width + AddrShift), data, mem_mask);
}

template<int Width, int AddrShift, endianness_t Endian, typename WRITE> template<typename W>
	std::enable_if_t<std::is_same<W, write8m_delegate>::value ||
					 std::is_same<W, write16m_delegate>::value ||
					 std::is_same<W, write32m_delegate>::value ||
					 std::is_same<W, write64m_delegate>::value,
					 void> handler_entry_write_delegate<Width, AddrShift, Endian, WRITE>::write_impl(offs_t offset, uX data, uX mem_mask) const
{
	m_delegate(*this->m_space, ((offset - this->m_address_base) & this->m_address_mask) >> (Width + AddrShift), data);
}

template<int Width, int AddrShift, endianness_t Endian, typename WRITE> template<typename W>
	std::enable_if_t<std::is_same<W, write8s_delegate>::value ||
					 std::is_same<W, write16s_delegate>::value ||
					 std::is_same<W, write32s_delegate>::value ||
					 std::is_same<W, write64s_delegate>::value,
					 void> handler_entry_write_delegate<Width, AddrShift, Endian, WRITE>::write_impl(offs_t offset, uX data, uX mem_mask) const
{
	m_delegate(((offset - this->m_address_base) & this->m_address_mask) >> (Width + AddrShift), data, mem_mask);
}

template<int Width, int AddrShift, endianness_t Endian, typename WRITE> template<typename W>
	std::enable_if_t<std::is_same<W, write8sm_delegate>::value ||
					 std::is_same<W, write16sm_delegate>::value ||
					 std::is_same<W, write32sm_delegate>::value ||
					 std::is_same<W, write64sm_delegate>::value,
					 void> handler_entry_write_delegate<Width, AddrShift, Endian, WRITE>::write_impl(offs_t offset, uX data, uX mem_mask) const
{
	m_delegate(((offset - this->m_address_base) & this->m_address_mask) >> (Width + AddrShift), data);
}

template<int Width, int AddrShift, endianness_t Endian, typename WRITE> template<typename W>
	std::enable_if_t<std::is_same<W, write8mo_delegate>::value ||
					 std::is_same<W, write16mo_delegate>::value ||
					 std::is_same<W, write32mo_delegate>::value ||
					 std::is_same<W, write64mo_delegate>::value,
					 void> handler_entry_write_delegate<Width, AddrShift, Endian, WRITE>::write_impl(offs_t offset, uX data, uX mem_mask) const
{
	m_delegate(*this->m_space, data);
}

template<int Width, int AddrShift, endianness_t Endian, typename WRITE> template<typename W>
	std::enable_if_t<std::is_same<W, write8smo_delegate>::value ||
					 std::is_same<W, write16smo_delegate>::value ||
					 std::is_same<W, write32smo_delegate>::value ||
					 std::is_same<W, write64smo_delegate>::value,
					 void> handler_entry_write_delegate<Width, AddrShift, Endian, WRITE>::write_impl(offs_t offset, uX data, uX mem_mask) const
{
	m_delegate(data);
}

template<int Width, int AddrShift, endianness_t Endian, typename WRITE> void handler_entry_write_delegate<Width, AddrShift, Endian, WRITE>::write(offs_t offset, uX data, uX mem_mask) const
{
	write_impl<WRITE>(offset, data, mem_mask);
}

template<int Width, int AddrShift, endianness_t Endian, typename WRITE> std::string handler_entry_write_delegate<Width, AddrShift, Endian, WRITE>::name() const
{
	return m_delegate.name();
}




template<int Width, int AddrShift, endianness_t Endian> typename emu::detail::handler_entry_size<Width>::uX handler_entry_read_ioport<Width, AddrShift, Endian>::read(offs_t offset, uX mem_mask) const
{
	return m_port->read();
}

template<int Width, int AddrShift, endianness_t Endian> std::string handler_entry_read_ioport<Width, AddrShift, Endian>::name() const
{
	return m_port->tag();
}

template<int Width, int AddrShift, endianness_t Endian> void handler_entry_write_ioport<Width, AddrShift, Endian>::write(offs_t offset, uX data, uX mem_mask) const
{
	m_port->write(data, mem_mask);
}

template<int Width, int AddrShift, endianness_t Endian> std::string handler_entry_write_ioport<Width, AddrShift, Endian>::name() const
{
	return m_port->tag();
}



template class handler_entry_read_delegate<0,  1, ENDIANNESS_LITTLE, read8_delegate>;
template class handler_entry_read_delegate<0,  1, ENDIANNESS_BIG,    read8_delegate>;
template class handler_entry_read_delegate<0,  0, ENDIANNESS_LITTLE, read8_delegate>;
template class handler_entry_read_delegate<0,  0, ENDIANNESS_BIG,    read8_delegate>;
template class handler_entry_read_delegate<1,  3, ENDIANNESS_LITTLE, read16_delegate>;
template class handler_entry_read_delegate<1,  3, ENDIANNESS_BIG,    read16_delegate>;
template class handler_entry_read_delegate<1,  0, ENDIANNESS_LITTLE, read16_delegate>;
template class handler_entry_read_delegate<1,  0, ENDIANNESS_BIG,    read16_delegate>;
template class handler_entry_read_delegate<1, -1, ENDIANNESS_LITTLE, read16_delegate>;
template class handler_entry_read_delegate<1, -1, ENDIANNESS_BIG,    read16_delegate>;
template class handler_entry_read_delegate<2,  3, ENDIANNESS_LITTLE, read32_delegate>;
template class handler_entry_read_delegate<2,  3, ENDIANNESS_BIG,    read32_delegate>;
template class handler_entry_read_delegate<2,  0, ENDIANNESS_LITTLE, read32_delegate>;
template class handler_entry_read_delegate<2,  0, ENDIANNESS_BIG,    read32_delegate>;
template class handler_entry_read_delegate<2, -1, ENDIANNESS_LITTLE, read32_delegate>;
template class handler_entry_read_delegate<2, -1, ENDIANNESS_BIG,    read32_delegate>;
template class handler_entry_read_delegate<2, -2, ENDIANNESS_LITTLE, read32_delegate>;
template class handler_entry_read_delegate<2, -2, ENDIANNESS_BIG,    read32_delegate>;
template class handler_entry_read_delegate<3,  0, ENDIANNESS_LITTLE, read64_delegate>;
template class handler_entry_read_delegate<3,  0, ENDIANNESS_BIG,    read64_delegate>;
template class handler_entry_read_delegate<3, -1, ENDIANNESS_LITTLE, read64_delegate>;
template class handler_entry_read_delegate<3, -1, ENDIANNESS_BIG,    read64_delegate>;
template class handler_entry_read_delegate<3, -2, ENDIANNESS_LITTLE, read64_delegate>;
template class handler_entry_read_delegate<3, -2, ENDIANNESS_BIG,    read64_delegate>;
template class handler_entry_read_delegate<3, -3, ENDIANNESS_LITTLE, read64_delegate>;
template class handler_entry_read_delegate<3, -3, ENDIANNESS_BIG,    read64_delegate>;

template class handler_entry_read_delegate<0,  1, ENDIANNESS_LITTLE, read8m_delegate>;
template class handler_entry_read_delegate<0,  1, ENDIANNESS_BIG,    read8m_delegate>;
template class handler_entry_read_delegate<0,  0, ENDIANNESS_LITTLE, read8m_delegate>;
template class handler_entry_read_delegate<0,  0, ENDIANNESS_BIG,    read8m_delegate>;
template class handler_entry_read_delegate<1,  3, ENDIANNESS_LITTLE, read16m_delegate>;
template class handler_entry_read_delegate<1,  3, ENDIANNESS_BIG,    read16m_delegate>;
template class handler_entry_read_delegate<1,  0, ENDIANNESS_LITTLE, read16m_delegate>;
template class handler_entry_read_delegate<1,  0, ENDIANNESS_BIG,    read16m_delegate>;
template class handler_entry_read_delegate<1, -1, ENDIANNESS_LITTLE, read16m_delegate>;
template class handler_entry_read_delegate<1, -1, ENDIANNESS_BIG,    read16m_delegate>;
template class handler_entry_read_delegate<2,  3, ENDIANNESS_LITTLE, read32m_delegate>;
template class handler_entry_read_delegate<2,  3, ENDIANNESS_BIG,    read32m_delegate>;
template class handler_entry_read_delegate<2,  0, ENDIANNESS_LITTLE, read32m_delegate>;
template class handler_entry_read_delegate<2,  0, ENDIANNESS_BIG,    read32m_delegate>;
template class handler_entry_read_delegate<2, -1, ENDIANNESS_LITTLE, read32m_delegate>;
template class handler_entry_read_delegate<2, -1, ENDIANNESS_BIG,    read32m_delegate>;
template class handler_entry_read_delegate<2, -2, ENDIANNESS_LITTLE, read32m_delegate>;
template class handler_entry_read_delegate<2, -2, ENDIANNESS_BIG,    read32m_delegate>;
template class handler_entry_read_delegate<3,  0, ENDIANNESS_LITTLE, read64m_delegate>;
template class handler_entry_read_delegate<3,  0, ENDIANNESS_BIG,    read64m_delegate>;
template class handler_entry_read_delegate<3, -1, ENDIANNESS_LITTLE, read64m_delegate>;
template class handler_entry_read_delegate<3, -1, ENDIANNESS_BIG,    read64m_delegate>;
template class handler_entry_read_delegate<3, -2, ENDIANNESS_LITTLE, read64m_delegate>;
template class handler_entry_read_delegate<3, -2, ENDIANNESS_BIG,    read64m_delegate>;
template class handler_entry_read_delegate<3, -3, ENDIANNESS_LITTLE, read64m_delegate>;
template class handler_entry_read_delegate<3, -3, ENDIANNESS_BIG,    read64m_delegate>;

template class handler_entry_read_delegate<0,  1, ENDIANNESS_LITTLE, read8s_delegate>;
template class handler_entry_read_delegate<0,  1, ENDIANNESS_BIG,    read8s_delegate>;
template class handler_entry_read_delegate<0,  0, ENDIANNESS_LITTLE, read8s_delegate>;
template class handler_entry_read_delegate<0,  0, ENDIANNESS_BIG,    read8s_delegate>;
template class handler_entry_read_delegate<1,  3, ENDIANNESS_LITTLE, read16s_delegate>;
template class handler_entry_read_delegate<1,  3, ENDIANNESS_BIG,    read16s_delegate>;
template class handler_entry_read_delegate<1,  0, ENDIANNESS_LITTLE, read16s_delegate>;
template class handler_entry_read_delegate<1,  0, ENDIANNESS_BIG,    read16s_delegate>;
template class handler_entry_read_delegate<1, -1, ENDIANNESS_LITTLE, read16s_delegate>;
template class handler_entry_read_delegate<1, -1, ENDIANNESS_BIG,    read16s_delegate>;
template class handler_entry_read_delegate<2,  3, ENDIANNESS_LITTLE, read32s_delegate>;
template class handler_entry_read_delegate<2,  3, ENDIANNESS_BIG,    read32s_delegate>;
template class handler_entry_read_delegate<2,  0, ENDIANNESS_LITTLE, read32s_delegate>;
template class handler_entry_read_delegate<2,  0, ENDIANNESS_BIG,    read32s_delegate>;
template class handler_entry_read_delegate<2, -1, ENDIANNESS_LITTLE, read32s_delegate>;
template class handler_entry_read_delegate<2, -1, ENDIANNESS_BIG,    read32s_delegate>;
template class handler_entry_read_delegate<2, -2, ENDIANNESS_LITTLE, read32s_delegate>;
template class handler_entry_read_delegate<2, -2, ENDIANNESS_BIG,    read32s_delegate>;
template class handler_entry_read_delegate<3,  0, ENDIANNESS_LITTLE, read64s_delegate>;
template class handler_entry_read_delegate<3,  0, ENDIANNESS_BIG,    read64s_delegate>;
template class handler_entry_read_delegate<3, -1, ENDIANNESS_LITTLE, read64s_delegate>;
template class handler_entry_read_delegate<3, -1, ENDIANNESS_BIG,    read64s_delegate>;
template class handler_entry_read_delegate<3, -2, ENDIANNESS_LITTLE, read64s_delegate>;
template class handler_entry_read_delegate<3, -2, ENDIANNESS_BIG,    read64s_delegate>;
template class handler_entry_read_delegate<3, -3, ENDIANNESS_LITTLE, read64s_delegate>;
template class handler_entry_read_delegate<3, -3, ENDIANNESS_BIG,    read64s_delegate>;

template class handler_entry_read_delegate<0,  1, ENDIANNESS_LITTLE, read8sm_delegate>;
template class handler_entry_read_delegate<0,  1, ENDIANNESS_BIG,    read8sm_delegate>;
template class handler_entry_read_delegate<0,  0, ENDIANNESS_LITTLE, read8sm_delegate>;
template class handler_entry_read_delegate<0,  0, ENDIANNESS_BIG,    read8sm_delegate>;
template class handler_entry_read_delegate<1,  3, ENDIANNESS_LITTLE, read16sm_delegate>;
template class handler_entry_read_delegate<1,  3, ENDIANNESS_BIG,    read16sm_delegate>;
template class handler_entry_read_delegate<1,  0, ENDIANNESS_LITTLE, read16sm_delegate>;
template class handler_entry_read_delegate<1,  0, ENDIANNESS_BIG,    read16sm_delegate>;
template class handler_entry_read_delegate<1, -1, ENDIANNESS_LITTLE, read16sm_delegate>;
template class handler_entry_read_delegate<1, -1, ENDIANNESS_BIG,    read16sm_delegate>;
template class handler_entry_read_delegate<2,  3, ENDIANNESS_LITTLE, read32sm_delegate>;
template class handler_entry_read_delegate<2,  3, ENDIANNESS_BIG,    read32sm_delegate>;
template class handler_entry_read_delegate<2,  0, ENDIANNESS_LITTLE, read32sm_delegate>;
template class handler_entry_read_delegate<2,  0, ENDIANNESS_BIG,    read32sm_delegate>;
template class handler_entry_read_delegate<2, -1, ENDIANNESS_LITTLE, read32sm_delegate>;
template class handler_entry_read_delegate<2, -1, ENDIANNESS_BIG,    read32sm_delegate>;
template class handler_entry_read_delegate<2, -2, ENDIANNESS_LITTLE, read32sm_delegate>;
template class handler_entry_read_delegate<2, -2, ENDIANNESS_BIG,    read32sm_delegate>;
template class handler_entry_read_delegate<3,  0, ENDIANNESS_LITTLE, read64sm_delegate>;
template class handler_entry_read_delegate<3,  0, ENDIANNESS_BIG,    read64sm_delegate>;
template class handler_entry_read_delegate<3, -1, ENDIANNESS_LITTLE, read64sm_delegate>;
template class handler_entry_read_delegate<3, -1, ENDIANNESS_BIG,    read64sm_delegate>;
template class handler_entry_read_delegate<3, -2, ENDIANNESS_LITTLE, read64sm_delegate>;
template class handler_entry_read_delegate<3, -2, ENDIANNESS_BIG,    read64sm_delegate>;
template class handler_entry_read_delegate<3, -3, ENDIANNESS_LITTLE, read64sm_delegate>;
template class handler_entry_read_delegate<3, -3, ENDIANNESS_BIG,    read64sm_delegate>;

template class handler_entry_read_delegate<0,  1, ENDIANNESS_LITTLE, read8mo_delegate>;
template class handler_entry_read_delegate<0,  1, ENDIANNESS_BIG,    read8mo_delegate>;
template class handler_entry_read_delegate<0,  0, ENDIANNESS_LITTLE, read8mo_delegate>;
template class handler_entry_read_delegate<0,  0, ENDIANNESS_BIG,    read8mo_delegate>;
template class handler_entry_read_delegate<1,  3, ENDIANNESS_LITTLE, read16mo_delegate>;
template class handler_entry_read_delegate<1,  3, ENDIANNESS_BIG,    read16mo_delegate>;
template class handler_entry_read_delegate<1,  0, ENDIANNESS_LITTLE, read16mo_delegate>;
template class handler_entry_read_delegate<1,  0, ENDIANNESS_BIG,    read16mo_delegate>;
template class handler_entry_read_delegate<1, -1, ENDIANNESS_LITTLE, read16mo_delegate>;
template class handler_entry_read_delegate<1, -1, ENDIANNESS_BIG,    read16mo_delegate>;
template class handler_entry_read_delegate<2,  3, ENDIANNESS_LITTLE, read32mo_delegate>;
template class handler_entry_read_delegate<2,  3, ENDIANNESS_BIG,    read32mo_delegate>;
template class handler_entry_read_delegate<2,  0, ENDIANNESS_LITTLE, read32mo_delegate>;
template class handler_entry_read_delegate<2,  0, ENDIANNESS_BIG,    read32mo_delegate>;
template class handler_entry_read_delegate<2, -1, ENDIANNESS_LITTLE, read32mo_delegate>;
template class handler_entry_read_delegate<2, -1, ENDIANNESS_BIG,    read32mo_delegate>;
template class handler_entry_read_delegate<2, -2, ENDIANNESS_LITTLE, read32mo_delegate>;
template class handler_entry_read_delegate<2, -2, ENDIANNESS_BIG,    read32mo_delegate>;
template class handler_entry_read_delegate<3,  0, ENDIANNESS_LITTLE, read64mo_delegate>;
template class handler_entry_read_delegate<3,  0, ENDIANNESS_BIG,    read64mo_delegate>;
template class handler_entry_read_delegate<3, -1, ENDIANNESS_LITTLE, read64mo_delegate>;
template class handler_entry_read_delegate<3, -1, ENDIANNESS_BIG,    read64mo_delegate>;
template class handler_entry_read_delegate<3, -2, ENDIANNESS_LITTLE, read64mo_delegate>;
template class handler_entry_read_delegate<3, -2, ENDIANNESS_BIG,    read64mo_delegate>;
template class handler_entry_read_delegate<3, -3, ENDIANNESS_LITTLE, read64mo_delegate>;
template class handler_entry_read_delegate<3, -3, ENDIANNESS_BIG,    read64mo_delegate>;

template class handler_entry_read_delegate<0,  1, ENDIANNESS_LITTLE, read8smo_delegate>;
template class handler_entry_read_delegate<0,  1, ENDIANNESS_BIG,    read8smo_delegate>;
template class handler_entry_read_delegate<0,  0, ENDIANNESS_LITTLE, read8smo_delegate>;
template class handler_entry_read_delegate<0,  0, ENDIANNESS_BIG,    read8smo_delegate>;
template class handler_entry_read_delegate<1,  3, ENDIANNESS_LITTLE, read16smo_delegate>;
template class handler_entry_read_delegate<1,  3, ENDIANNESS_BIG,    read16smo_delegate>;
template class handler_entry_read_delegate<1,  0, ENDIANNESS_LITTLE, read16smo_delegate>;
template class handler_entry_read_delegate<1,  0, ENDIANNESS_BIG,    read16smo_delegate>;
template class handler_entry_read_delegate<1, -1, ENDIANNESS_LITTLE, read16smo_delegate>;
template class handler_entry_read_delegate<1, -1, ENDIANNESS_BIG,    read16smo_delegate>;
template class handler_entry_read_delegate<2,  3, ENDIANNESS_LITTLE, read32smo_delegate>;
template class handler_entry_read_delegate<2,  3, ENDIANNESS_BIG,    read32smo_delegate>;
template class handler_entry_read_delegate<2,  0, ENDIANNESS_LITTLE, read32smo_delegate>;
template class handler_entry_read_delegate<2,  0, ENDIANNESS_BIG,    read32smo_delegate>;
template class handler_entry_read_delegate<2, -1, ENDIANNESS_LITTLE, read32smo_delegate>;
template class handler_entry_read_delegate<2, -1, ENDIANNESS_BIG,    read32smo_delegate>;
template class handler_entry_read_delegate<2, -2, ENDIANNESS_LITTLE, read32smo_delegate>;
template class handler_entry_read_delegate<2, -2, ENDIANNESS_BIG,    read32smo_delegate>;
template class handler_entry_read_delegate<3,  0, ENDIANNESS_LITTLE, read64smo_delegate>;
template class handler_entry_read_delegate<3,  0, ENDIANNESS_BIG,    read64smo_delegate>;
template class handler_entry_read_delegate<3, -1, ENDIANNESS_LITTLE, read64smo_delegate>;
template class handler_entry_read_delegate<3, -1, ENDIANNESS_BIG,    read64smo_delegate>;
template class handler_entry_read_delegate<3, -2, ENDIANNESS_LITTLE, read64smo_delegate>;
template class handler_entry_read_delegate<3, -2, ENDIANNESS_BIG,    read64smo_delegate>;
template class handler_entry_read_delegate<3, -3, ENDIANNESS_LITTLE, read64smo_delegate>;
template class handler_entry_read_delegate<3, -3, ENDIANNESS_BIG,    read64smo_delegate>;

template class handler_entry_write_delegate<0,  1, ENDIANNESS_LITTLE, write8_delegate>;
template class handler_entry_write_delegate<0,  1, ENDIANNESS_BIG,    write8_delegate>;
template class handler_entry_write_delegate<0,  0, ENDIANNESS_LITTLE, write8_delegate>;
template class handler_entry_write_delegate<0,  0, ENDIANNESS_BIG,    write8_delegate>;
template class handler_entry_write_delegate<1,  3, ENDIANNESS_LITTLE, write16_delegate>;
template class handler_entry_write_delegate<1,  3, ENDIANNESS_BIG,    write16_delegate>;
template class handler_entry_write_delegate<1,  0, ENDIANNESS_LITTLE, write16_delegate>;
template class handler_entry_write_delegate<1,  0, ENDIANNESS_BIG,    write16_delegate>;
template class handler_entry_write_delegate<1, -1, ENDIANNESS_LITTLE, write16_delegate>;
template class handler_entry_write_delegate<1, -1, ENDIANNESS_BIG,    write16_delegate>;
template class handler_entry_write_delegate<2,  3, ENDIANNESS_LITTLE, write32_delegate>;
template class handler_entry_write_delegate<2,  3, ENDIANNESS_BIG,    write32_delegate>;
template class handler_entry_write_delegate<2,  0, ENDIANNESS_LITTLE, write32_delegate>;
template class handler_entry_write_delegate<2,  0, ENDIANNESS_BIG,    write32_delegate>;
template class handler_entry_write_delegate<2, -1, ENDIANNESS_LITTLE, write32_delegate>;
template class handler_entry_write_delegate<2, -1, ENDIANNESS_BIG,    write32_delegate>;
template class handler_entry_write_delegate<2, -2, ENDIANNESS_LITTLE, write32_delegate>;
template class handler_entry_write_delegate<2, -2, ENDIANNESS_BIG,    write32_delegate>;
template class handler_entry_write_delegate<3,  0, ENDIANNESS_LITTLE, write64_delegate>;
template class handler_entry_write_delegate<3,  0, ENDIANNESS_BIG,    write64_delegate>;
template class handler_entry_write_delegate<3, -1, ENDIANNESS_LITTLE, write64_delegate>;
template class handler_entry_write_delegate<3, -1, ENDIANNESS_BIG,    write64_delegate>;
template class handler_entry_write_delegate<3, -2, ENDIANNESS_LITTLE, write64_delegate>;
template class handler_entry_write_delegate<3, -2, ENDIANNESS_BIG,    write64_delegate>;
template class handler_entry_write_delegate<3, -3, ENDIANNESS_LITTLE, write64_delegate>;
template class handler_entry_write_delegate<3, -3, ENDIANNESS_BIG,    write64_delegate>;

template class handler_entry_write_delegate<0,  1, ENDIANNESS_LITTLE, write8m_delegate>;
template class handler_entry_write_delegate<0,  1, ENDIANNESS_BIG,    write8m_delegate>;
template class handler_entry_write_delegate<0,  0, ENDIANNESS_LITTLE, write8m_delegate>;
template class handler_entry_write_delegate<0,  0, ENDIANNESS_BIG,    write8m_delegate>;
template class handler_entry_write_delegate<1,  3, ENDIANNESS_LITTLE, write16m_delegate>;
template class handler_entry_write_delegate<1,  3, ENDIANNESS_BIG,    write16m_delegate>;
template class handler_entry_write_delegate<1,  0, ENDIANNESS_LITTLE, write16m_delegate>;
template class handler_entry_write_delegate<1,  0, ENDIANNESS_BIG,    write16m_delegate>;
template class handler_entry_write_delegate<1, -1, ENDIANNESS_LITTLE, write16m_delegate>;
template class handler_entry_write_delegate<1, -1, ENDIANNESS_BIG,    write16m_delegate>;
template class handler_entry_write_delegate<2,  3, ENDIANNESS_LITTLE, write32m_delegate>;
template class handler_entry_write_delegate<2,  3, ENDIANNESS_BIG,    write32m_delegate>;
template class handler_entry_write_delegate<2,  0, ENDIANNESS_LITTLE, write32m_delegate>;
template class handler_entry_write_delegate<2,  0, ENDIANNESS_BIG,    write32m_delegate>;
template class handler_entry_write_delegate<2, -1, ENDIANNESS_LITTLE, write32m_delegate>;
template class handler_entry_write_delegate<2, -1, ENDIANNESS_BIG,    write32m_delegate>;
template class handler_entry_write_delegate<2, -2, ENDIANNESS_LITTLE, write32m_delegate>;
template class handler_entry_write_delegate<2, -2, ENDIANNESS_BIG,    write32m_delegate>;
template class handler_entry_write_delegate<3,  0, ENDIANNESS_LITTLE, write64m_delegate>;
template class handler_entry_write_delegate<3,  0, ENDIANNESS_BIG,    write64m_delegate>;
template class handler_entry_write_delegate<3, -1, ENDIANNESS_LITTLE, write64m_delegate>;
template class handler_entry_write_delegate<3, -1, ENDIANNESS_BIG,    write64m_delegate>;
template class handler_entry_write_delegate<3, -2, ENDIANNESS_LITTLE, write64m_delegate>;
template class handler_entry_write_delegate<3, -2, ENDIANNESS_BIG,    write64m_delegate>;
template class handler_entry_write_delegate<3, -3, ENDIANNESS_LITTLE, write64m_delegate>;
template class handler_entry_write_delegate<3, -3, ENDIANNESS_BIG,    write64m_delegate>;

template class handler_entry_write_delegate<0,  1, ENDIANNESS_LITTLE, write8s_delegate>;
template class handler_entry_write_delegate<0,  1, ENDIANNESS_BIG,    write8s_delegate>;
template class handler_entry_write_delegate<0,  0, ENDIANNESS_LITTLE, write8s_delegate>;
template class handler_entry_write_delegate<0,  0, ENDIANNESS_BIG,    write8s_delegate>;
template class handler_entry_write_delegate<1,  3, ENDIANNESS_LITTLE, write16s_delegate>;
template class handler_entry_write_delegate<1,  3, ENDIANNESS_BIG,    write16s_delegate>;
template class handler_entry_write_delegate<1,  0, ENDIANNESS_LITTLE, write16s_delegate>;
template class handler_entry_write_delegate<1,  0, ENDIANNESS_BIG,    write16s_delegate>;
template class handler_entry_write_delegate<1, -1, ENDIANNESS_LITTLE, write16s_delegate>;
template class handler_entry_write_delegate<1, -1, ENDIANNESS_BIG,    write16s_delegate>;
template class handler_entry_write_delegate<2,  3, ENDIANNESS_LITTLE, write32s_delegate>;
template class handler_entry_write_delegate<2,  3, ENDIANNESS_BIG,    write32s_delegate>;
template class handler_entry_write_delegate<2,  0, ENDIANNESS_LITTLE, write32s_delegate>;
template class handler_entry_write_delegate<2,  0, ENDIANNESS_BIG,    write32s_delegate>;
template class handler_entry_write_delegate<2, -1, ENDIANNESS_LITTLE, write32s_delegate>;
template class handler_entry_write_delegate<2, -1, ENDIANNESS_BIG,    write32s_delegate>;
template class handler_entry_write_delegate<2, -2, ENDIANNESS_LITTLE, write32s_delegate>;
template class handler_entry_write_delegate<2, -2, ENDIANNESS_BIG,    write32s_delegate>;
template class handler_entry_write_delegate<3,  0, ENDIANNESS_LITTLE, write64s_delegate>;
template class handler_entry_write_delegate<3,  0, ENDIANNESS_BIG,    write64s_delegate>;
template class handler_entry_write_delegate<3, -1, ENDIANNESS_LITTLE, write64s_delegate>;
template class handler_entry_write_delegate<3, -1, ENDIANNESS_BIG,    write64s_delegate>;
template class handler_entry_write_delegate<3, -2, ENDIANNESS_LITTLE, write64s_delegate>;
template class handler_entry_write_delegate<3, -2, ENDIANNESS_BIG,    write64s_delegate>;
template class handler_entry_write_delegate<3, -3, ENDIANNESS_LITTLE, write64s_delegate>;
template class handler_entry_write_delegate<3, -3, ENDIANNESS_BIG,    write64s_delegate>;

template class handler_entry_write_delegate<0,  1, ENDIANNESS_LITTLE, write8sm_delegate>;
template class handler_entry_write_delegate<0,  1, ENDIANNESS_BIG,    write8sm_delegate>;
template class handler_entry_write_delegate<0,  0, ENDIANNESS_LITTLE, write8sm_delegate>;
template class handler_entry_write_delegate<0,  0, ENDIANNESS_BIG,    write8sm_delegate>;
template class handler_entry_write_delegate<1,  3, ENDIANNESS_LITTLE, write16sm_delegate>;
template class handler_entry_write_delegate<1,  3, ENDIANNESS_BIG,    write16sm_delegate>;
template class handler_entry_write_delegate<1,  0, ENDIANNESS_LITTLE, write16sm_delegate>;
template class handler_entry_write_delegate<1,  0, ENDIANNESS_BIG,    write16sm_delegate>;
template class handler_entry_write_delegate<1, -1, ENDIANNESS_LITTLE, write16sm_delegate>;
template class handler_entry_write_delegate<1, -1, ENDIANNESS_BIG,    write16sm_delegate>;
template class handler_entry_write_delegate<2,  3, ENDIANNESS_LITTLE, write32sm_delegate>;
template class handler_entry_write_delegate<2,  3, ENDIANNESS_BIG,    write32sm_delegate>;
template class handler_entry_write_delegate<2,  0, ENDIANNESS_LITTLE, write32sm_delegate>;
template class handler_entry_write_delegate<2,  0, ENDIANNESS_BIG,    write32sm_delegate>;
template class handler_entry_write_delegate<2, -1, ENDIANNESS_LITTLE, write32sm_delegate>;
template class handler_entry_write_delegate<2, -1, ENDIANNESS_BIG,    write32sm_delegate>;
template class handler_entry_write_delegate<2, -2, ENDIANNESS_LITTLE, write32sm_delegate>;
template class handler_entry_write_delegate<2, -2, ENDIANNESS_BIG,    write32sm_delegate>;
template class handler_entry_write_delegate<3,  0, ENDIANNESS_LITTLE, write64sm_delegate>;
template class handler_entry_write_delegate<3,  0, ENDIANNESS_BIG,    write64sm_delegate>;
template class handler_entry_write_delegate<3, -1, ENDIANNESS_LITTLE, write64sm_delegate>;
template class handler_entry_write_delegate<3, -1, ENDIANNESS_BIG,    write64sm_delegate>;
template class handler_entry_write_delegate<3, -2, ENDIANNESS_LITTLE, write64sm_delegate>;
template class handler_entry_write_delegate<3, -2, ENDIANNESS_BIG,    write64sm_delegate>;
template class handler_entry_write_delegate<3, -3, ENDIANNESS_LITTLE, write64sm_delegate>;
template class handler_entry_write_delegate<3, -3, ENDIANNESS_BIG,    write64sm_delegate>;

template class handler_entry_write_delegate<0,  1, ENDIANNESS_LITTLE, write8mo_delegate>;
template class handler_entry_write_delegate<0,  1, ENDIANNESS_BIG,    write8mo_delegate>;
template class handler_entry_write_delegate<0,  0, ENDIANNESS_LITTLE, write8mo_delegate>;
template class handler_entry_write_delegate<0,  0, ENDIANNESS_BIG,    write8mo_delegate>;
template class handler_entry_write_delegate<1,  3, ENDIANNESS_LITTLE, write16mo_delegate>;
template class handler_entry_write_delegate<1,  3, ENDIANNESS_BIG,    write16mo_delegate>;
template class handler_entry_write_delegate<1,  0, ENDIANNESS_LITTLE, write16mo_delegate>;
template class handler_entry_write_delegate<1,  0, ENDIANNESS_BIG,    write16mo_delegate>;
template class handler_entry_write_delegate<1, -1, ENDIANNESS_LITTLE, write16mo_delegate>;
template class handler_entry_write_delegate<1, -1, ENDIANNESS_BIG,    write16mo_delegate>;
template class handler_entry_write_delegate<2,  3, ENDIANNESS_LITTLE, write32mo_delegate>;
template class handler_entry_write_delegate<2,  3, ENDIANNESS_BIG,    write32mo_delegate>;
template class handler_entry_write_delegate<2,  0, ENDIANNESS_LITTLE, write32mo_delegate>;
template class handler_entry_write_delegate<2,  0, ENDIANNESS_BIG,    write32mo_delegate>;
template class handler_entry_write_delegate<2, -1, ENDIANNESS_LITTLE, write32mo_delegate>;
template class handler_entry_write_delegate<2, -1, ENDIANNESS_BIG,    write32mo_delegate>;
template class handler_entry_write_delegate<2, -2, ENDIANNESS_LITTLE, write32mo_delegate>;
template class handler_entry_write_delegate<2, -2, ENDIANNESS_BIG,    write32mo_delegate>;
template class handler_entry_write_delegate<3,  0, ENDIANNESS_LITTLE, write64mo_delegate>;
template class handler_entry_write_delegate<3,  0, ENDIANNESS_BIG,    write64mo_delegate>;
template class handler_entry_write_delegate<3, -1, ENDIANNESS_LITTLE, write64mo_delegate>;
template class handler_entry_write_delegate<3, -1, ENDIANNESS_BIG,    write64mo_delegate>;
template class handler_entry_write_delegate<3, -2, ENDIANNESS_LITTLE, write64mo_delegate>;
template class handler_entry_write_delegate<3, -2, ENDIANNESS_BIG,    write64mo_delegate>;
template class handler_entry_write_delegate<3, -3, ENDIANNESS_LITTLE, write64mo_delegate>;
template class handler_entry_write_delegate<3, -3, ENDIANNESS_BIG,    write64mo_delegate>;

template class handler_entry_write_delegate<0,  1, ENDIANNESS_LITTLE, write8smo_delegate>;
template class handler_entry_write_delegate<0,  1, ENDIANNESS_BIG,    write8smo_delegate>;
template class handler_entry_write_delegate<0,  0, ENDIANNESS_LITTLE, write8smo_delegate>;
template class handler_entry_write_delegate<0,  0, ENDIANNESS_BIG,    write8smo_delegate>;
template class handler_entry_write_delegate<1,  3, ENDIANNESS_LITTLE, write16smo_delegate>;
template class handler_entry_write_delegate<1,  3, ENDIANNESS_BIG,    write16smo_delegate>;
template class handler_entry_write_delegate<1,  0, ENDIANNESS_LITTLE, write16smo_delegate>;
template class handler_entry_write_delegate<1,  0, ENDIANNESS_BIG,    write16smo_delegate>;
template class handler_entry_write_delegate<1, -1, ENDIANNESS_LITTLE, write16smo_delegate>;
template class handler_entry_write_delegate<1, -1, ENDIANNESS_BIG,    write16smo_delegate>;
template class handler_entry_write_delegate<2,  3, ENDIANNESS_LITTLE, write32smo_delegate>;
template class handler_entry_write_delegate<2,  3, ENDIANNESS_BIG,    write32smo_delegate>;
template class handler_entry_write_delegate<2,  0, ENDIANNESS_LITTLE, write32smo_delegate>;
template class handler_entry_write_delegate<2,  0, ENDIANNESS_BIG,    write32smo_delegate>;
template class handler_entry_write_delegate<2, -1, ENDIANNESS_LITTLE, write32smo_delegate>;
template class handler_entry_write_delegate<2, -1, ENDIANNESS_BIG,    write32smo_delegate>;
template class handler_entry_write_delegate<2, -2, ENDIANNESS_LITTLE, write32smo_delegate>;
template class handler_entry_write_delegate<2, -2, ENDIANNESS_BIG,    write32smo_delegate>;
template class handler_entry_write_delegate<3,  0, ENDIANNESS_LITTLE, write64smo_delegate>;
template class handler_entry_write_delegate<3,  0, ENDIANNESS_BIG,    write64smo_delegate>;
template class handler_entry_write_delegate<3, -1, ENDIANNESS_LITTLE, write64smo_delegate>;
template class handler_entry_write_delegate<3, -1, ENDIANNESS_BIG,    write64smo_delegate>;
template class handler_entry_write_delegate<3, -2, ENDIANNESS_LITTLE, write64smo_delegate>;
template class handler_entry_write_delegate<3, -2, ENDIANNESS_BIG,    write64smo_delegate>;
template class handler_entry_write_delegate<3, -3, ENDIANNESS_LITTLE, write64smo_delegate>;
template class handler_entry_write_delegate<3, -3, ENDIANNESS_BIG,    write64smo_delegate>;


template class handler_entry_read_ioport<0,  1, ENDIANNESS_LITTLE>;
template class handler_entry_read_ioport<0,  1, ENDIANNESS_BIG>;
template class handler_entry_read_ioport<0,  0, ENDIANNESS_LITTLE>;
template class handler_entry_read_ioport<0,  0, ENDIANNESS_BIG>;
template class handler_entry_read_ioport<1,  3, ENDIANNESS_LITTLE>;
template class handler_entry_read_ioport<1,  3, ENDIANNESS_BIG>;
template class handler_entry_read_ioport<1,  0, ENDIANNESS_LITTLE>;
template class handler_entry_read_ioport<1,  0, ENDIANNESS_BIG>;
template class handler_entry_read_ioport<1, -1, ENDIANNESS_LITTLE>;
template class handler_entry_read_ioport<1, -1, ENDIANNESS_BIG>;
template class handler_entry_read_ioport<2,  3, ENDIANNESS_LITTLE>;
template class handler_entry_read_ioport<2,  3, ENDIANNESS_BIG>;
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

template class handler_entry_write_ioport<0,  1, ENDIANNESS_LITTLE>;
template class handler_entry_write_ioport<0,  1, ENDIANNESS_BIG>;
template class handler_entry_write_ioport<0,  0, ENDIANNESS_LITTLE>;
template class handler_entry_write_ioport<0,  0, ENDIANNESS_BIG>;
template class handler_entry_write_ioport<1,  3, ENDIANNESS_LITTLE>;
template class handler_entry_write_ioport<1,  3, ENDIANNESS_BIG>;
template class handler_entry_write_ioport<1,  0, ENDIANNESS_LITTLE>;
template class handler_entry_write_ioport<1,  0, ENDIANNESS_BIG>;
template class handler_entry_write_ioport<1, -1, ENDIANNESS_LITTLE>;
template class handler_entry_write_ioport<1, -1, ENDIANNESS_BIG>;
template class handler_entry_write_ioport<2,  3, ENDIANNESS_LITTLE>;
template class handler_entry_write_ioport<2,  3, ENDIANNESS_BIG>;
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
