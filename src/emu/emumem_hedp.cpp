// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "emumem_hea.h"
#include "emumem_hedp.h"

template<int Width, int AddrShift, typename READ> template<typename R>
	std::enable_if_t<std::is_same<R, read8_delegate>::value ||
					 std::is_same<R, read16_delegate>::value ||
					 std::is_same<R, read32_delegate>::value ||
					 std::is_same<R, read64_delegate>::value,
					 typename emu::detail::handler_entry_size<Width>::uX> handler_entry_read_delegate<Width, AddrShift, READ>::read_impl(offs_t offset, uX mem_mask) const
{
	return m_delegate(*this->m_space, ((offset - this->m_address_base) & this->m_address_mask) >> (Width + AddrShift), mem_mask);
}

template<int Width, int AddrShift, typename READ> template<typename R>
	std::enable_if_t<std::is_same<R, read8m_delegate>::value ||
					 std::is_same<R, read16m_delegate>::value ||
					 std::is_same<R, read32m_delegate>::value ||
					 std::is_same<R, read64m_delegate>::value,
					 typename emu::detail::handler_entry_size<Width>::uX> handler_entry_read_delegate<Width, AddrShift, READ>::read_impl(offs_t offset, uX mem_mask) const
{
	return m_delegate(*this->m_space, ((offset - this->m_address_base) & this->m_address_mask) >> (Width + AddrShift));
}

template<int Width, int AddrShift, typename READ> template<typename R>
	std::enable_if_t<std::is_same<R, read8s_delegate>::value ||
					 std::is_same<R, read16s_delegate>::value ||
					 std::is_same<R, read32s_delegate>::value ||
					 std::is_same<R, read64s_delegate>::value,
					 typename emu::detail::handler_entry_size<Width>::uX> handler_entry_read_delegate<Width, AddrShift, READ>::read_impl(offs_t offset, uX mem_mask) const
{
	return m_delegate(((offset - this->m_address_base) & this->m_address_mask) >> (Width + AddrShift), mem_mask);
}

template<int Width, int AddrShift, typename READ> template<typename R>
	std::enable_if_t<std::is_same<R, read8sm_delegate>::value ||
					 std::is_same<R, read16sm_delegate>::value ||
					 std::is_same<R, read32sm_delegate>::value ||
					 std::is_same<R, read64sm_delegate>::value,
					 typename emu::detail::handler_entry_size<Width>::uX> handler_entry_read_delegate<Width, AddrShift, READ>::read_impl(offs_t offset, uX mem_mask) const
{
	return m_delegate(((offset - this->m_address_base) & this->m_address_mask) >> (Width + AddrShift));
}

template<int Width, int AddrShift, typename READ> template<typename R>
	std::enable_if_t<std::is_same<R, read8mo_delegate>::value ||
					 std::is_same<R, read16mo_delegate>::value ||
					 std::is_same<R, read32mo_delegate>::value ||
					 std::is_same<R, read64mo_delegate>::value,
					 typename emu::detail::handler_entry_size<Width>::uX> handler_entry_read_delegate<Width, AddrShift, READ>::read_impl(offs_t offset, uX mem_mask) const
{
	return m_delegate(*this->m_space);
}

template<int Width, int AddrShift, typename READ> template<typename R>
	std::enable_if_t<std::is_same<R, read8smo_delegate>::value ||
					 std::is_same<R, read16smo_delegate>::value ||
					 std::is_same<R, read32smo_delegate>::value ||
					 std::is_same<R, read64smo_delegate>::value,
					 typename emu::detail::handler_entry_size<Width>::uX> handler_entry_read_delegate<Width, AddrShift, READ>::read_impl(offs_t offset, uX mem_mask) const
{
	return m_delegate();
}

template<int Width, int AddrShift, typename READ> typename emu::detail::handler_entry_size<Width>::uX handler_entry_read_delegate<Width, AddrShift, READ>::read(offs_t offset, uX mem_mask) const
{
	return read_impl<READ>(offset, mem_mask);
}

template<int Width, int AddrShift, typename READ> std::string handler_entry_read_delegate<Width, AddrShift, READ>::name() const
{
	return m_delegate.name();
}

template<int Width, int AddrShift, typename WRITE> template<typename W>
	std::enable_if_t<std::is_same<W, write8_delegate>::value ||
					 std::is_same<W, write16_delegate>::value ||
					 std::is_same<W, write32_delegate>::value ||
					 std::is_same<W, write64_delegate>::value,
					 void> handler_entry_write_delegate<Width, AddrShift, WRITE>::write_impl(offs_t offset, uX data, uX mem_mask) const
{
	m_delegate(*this->m_space, ((offset - this->m_address_base) & this->m_address_mask) >> (Width + AddrShift), data, mem_mask);
}

template<int Width, int AddrShift, typename WRITE> template<typename W>
	std::enable_if_t<std::is_same<W, write8m_delegate>::value ||
					 std::is_same<W, write16m_delegate>::value ||
					 std::is_same<W, write32m_delegate>::value ||
					 std::is_same<W, write64m_delegate>::value,
					 void> handler_entry_write_delegate<Width, AddrShift, WRITE>::write_impl(offs_t offset, uX data, uX mem_mask) const
{
	m_delegate(*this->m_space, ((offset - this->m_address_base) & this->m_address_mask) >> (Width + AddrShift), data);
}

template<int Width, int AddrShift, typename WRITE> template<typename W>
	std::enable_if_t<std::is_same<W, write8s_delegate>::value ||
					 std::is_same<W, write16s_delegate>::value ||
					 std::is_same<W, write32s_delegate>::value ||
					 std::is_same<W, write64s_delegate>::value,
					 void> handler_entry_write_delegate<Width, AddrShift, WRITE>::write_impl(offs_t offset, uX data, uX mem_mask) const
{
	m_delegate(((offset - this->m_address_base) & this->m_address_mask) >> (Width + AddrShift), data, mem_mask);
}

template<int Width, int AddrShift, typename WRITE> template<typename W>
	std::enable_if_t<std::is_same<W, write8sm_delegate>::value ||
					 std::is_same<W, write16sm_delegate>::value ||
					 std::is_same<W, write32sm_delegate>::value ||
					 std::is_same<W, write64sm_delegate>::value,
					 void> handler_entry_write_delegate<Width, AddrShift, WRITE>::write_impl(offs_t offset, uX data, uX mem_mask) const
{
	m_delegate(((offset - this->m_address_base) & this->m_address_mask) >> (Width + AddrShift), data);
}

template<int Width, int AddrShift, typename WRITE> template<typename W>
	std::enable_if_t<std::is_same<W, write8mo_delegate>::value ||
					 std::is_same<W, write16mo_delegate>::value ||
					 std::is_same<W, write32mo_delegate>::value ||
					 std::is_same<W, write64mo_delegate>::value,
					 void> handler_entry_write_delegate<Width, AddrShift, WRITE>::write_impl(offs_t offset, uX data, uX mem_mask) const
{
	m_delegate(*this->m_space, data);
}

template<int Width, int AddrShift, typename WRITE> template<typename W>
	std::enable_if_t<std::is_same<W, write8smo_delegate>::value ||
					 std::is_same<W, write16smo_delegate>::value ||
					 std::is_same<W, write32smo_delegate>::value ||
					 std::is_same<W, write64smo_delegate>::value,
					 void> handler_entry_write_delegate<Width, AddrShift, WRITE>::write_impl(offs_t offset, uX data, uX mem_mask) const
{
	m_delegate(data);
}

template<int Width, int AddrShift, typename WRITE> void handler_entry_write_delegate<Width, AddrShift, WRITE>::write(offs_t offset, uX data, uX mem_mask) const
{
	write_impl<WRITE>(offset, data, mem_mask);
}

template<int Width, int AddrShift, typename WRITE> std::string handler_entry_write_delegate<Width, AddrShift, WRITE>::name() const
{
	return m_delegate.name();
}




template<int Width, int AddrShift> typename emu::detail::handler_entry_size<Width>::uX handler_entry_read_ioport<Width, AddrShift>::read(offs_t offset, uX mem_mask) const
{
	return m_port->read();
}

template<int Width, int AddrShift> std::string handler_entry_read_ioport<Width, AddrShift>::name() const
{
	return m_port->tag();
}

template<int Width, int AddrShift> void handler_entry_write_ioport<Width, AddrShift>::write(offs_t offset, uX data, uX mem_mask) const
{
	m_port->write(data, mem_mask);
}

template<int Width, int AddrShift> std::string handler_entry_write_ioport<Width, AddrShift>::name() const
{
	return m_port->tag();
}



template class handler_entry_read_delegate<0,  1, read8_delegate>;
template class handler_entry_read_delegate<0,  0, read8_delegate>;
template class handler_entry_read_delegate<1,  3, read16_delegate>;
template class handler_entry_read_delegate<1,  0, read16_delegate>;
template class handler_entry_read_delegate<1, -1, read16_delegate>;
template class handler_entry_read_delegate<2,  3, read32_delegate>;
template class handler_entry_read_delegate<2,  0, read32_delegate>;
template class handler_entry_read_delegate<2, -1, read32_delegate>;
template class handler_entry_read_delegate<2, -2, read32_delegate>;
template class handler_entry_read_delegate<3,  0, read64_delegate>;
template class handler_entry_read_delegate<3, -1, read64_delegate>;
template class handler_entry_read_delegate<3, -2, read64_delegate>;
template class handler_entry_read_delegate<3, -3, read64_delegate>;

template class handler_entry_read_delegate<0,  1, read8m_delegate>;
template class handler_entry_read_delegate<0,  0, read8m_delegate>;
template class handler_entry_read_delegate<1,  3, read16m_delegate>;
template class handler_entry_read_delegate<1,  0, read16m_delegate>;
template class handler_entry_read_delegate<1, -1, read16m_delegate>;
template class handler_entry_read_delegate<2,  3, read32m_delegate>;
template class handler_entry_read_delegate<2,  0, read32m_delegate>;
template class handler_entry_read_delegate<2, -1, read32m_delegate>;
template class handler_entry_read_delegate<2, -2, read32m_delegate>;
template class handler_entry_read_delegate<3,  0, read64m_delegate>;
template class handler_entry_read_delegate<3, -1, read64m_delegate>;
template class handler_entry_read_delegate<3, -2, read64m_delegate>;
template class handler_entry_read_delegate<3, -3, read64m_delegate>;

template class handler_entry_read_delegate<0,  1, read8s_delegate>;
template class handler_entry_read_delegate<0,  0, read8s_delegate>;
template class handler_entry_read_delegate<1,  3, read16s_delegate>;
template class handler_entry_read_delegate<1,  0, read16s_delegate>;
template class handler_entry_read_delegate<1, -1, read16s_delegate>;
template class handler_entry_read_delegate<2,  3, read32s_delegate>;
template class handler_entry_read_delegate<2,  0, read32s_delegate>;
template class handler_entry_read_delegate<2, -1, read32s_delegate>;
template class handler_entry_read_delegate<2, -2, read32s_delegate>;
template class handler_entry_read_delegate<3,  0, read64s_delegate>;
template class handler_entry_read_delegate<3, -1, read64s_delegate>;
template class handler_entry_read_delegate<3, -2, read64s_delegate>;
template class handler_entry_read_delegate<3, -3, read64s_delegate>;

template class handler_entry_read_delegate<0,  1, read8sm_delegate>;
template class handler_entry_read_delegate<0,  0, read8sm_delegate>;
template class handler_entry_read_delegate<1,  3, read16sm_delegate>;
template class handler_entry_read_delegate<1,  0, read16sm_delegate>;
template class handler_entry_read_delegate<1, -1, read16sm_delegate>;
template class handler_entry_read_delegate<2,  3, read32sm_delegate>;
template class handler_entry_read_delegate<2,  0, read32sm_delegate>;
template class handler_entry_read_delegate<2, -1, read32sm_delegate>;
template class handler_entry_read_delegate<2, -2, read32sm_delegate>;
template class handler_entry_read_delegate<3,  0, read64sm_delegate>;
template class handler_entry_read_delegate<3, -1, read64sm_delegate>;
template class handler_entry_read_delegate<3, -2, read64sm_delegate>;
template class handler_entry_read_delegate<3, -3, read64sm_delegate>;

template class handler_entry_read_delegate<0,  1, read8mo_delegate>;
template class handler_entry_read_delegate<0,  0, read8mo_delegate>;
template class handler_entry_read_delegate<1,  3, read16mo_delegate>;
template class handler_entry_read_delegate<1,  0, read16mo_delegate>;
template class handler_entry_read_delegate<1, -1, read16mo_delegate>;
template class handler_entry_read_delegate<2,  3, read32mo_delegate>;
template class handler_entry_read_delegate<2,  0, read32mo_delegate>;
template class handler_entry_read_delegate<2, -1, read32mo_delegate>;
template class handler_entry_read_delegate<2, -2, read32mo_delegate>;
template class handler_entry_read_delegate<3,  0, read64mo_delegate>;
template class handler_entry_read_delegate<3, -1, read64mo_delegate>;
template class handler_entry_read_delegate<3, -2, read64mo_delegate>;
template class handler_entry_read_delegate<3, -3, read64mo_delegate>;

template class handler_entry_read_delegate<0,  1, read8smo_delegate>;
template class handler_entry_read_delegate<0,  0, read8smo_delegate>;
template class handler_entry_read_delegate<1,  3, read16smo_delegate>;
template class handler_entry_read_delegate<1,  0, read16smo_delegate>;
template class handler_entry_read_delegate<1, -1, read16smo_delegate>;
template class handler_entry_read_delegate<2,  3, read32smo_delegate>;
template class handler_entry_read_delegate<2,  0, read32smo_delegate>;
template class handler_entry_read_delegate<2, -1, read32smo_delegate>;
template class handler_entry_read_delegate<2, -2, read32smo_delegate>;
template class handler_entry_read_delegate<3,  0, read64smo_delegate>;
template class handler_entry_read_delegate<3, -1, read64smo_delegate>;
template class handler_entry_read_delegate<3, -2, read64smo_delegate>;
template class handler_entry_read_delegate<3, -3, read64smo_delegate>;

template class handler_entry_write_delegate<0,  1, write8_delegate>;
template class handler_entry_write_delegate<0,  0, write8_delegate>;
template class handler_entry_write_delegate<1,  3, write16_delegate>;
template class handler_entry_write_delegate<1,  0, write16_delegate>;
template class handler_entry_write_delegate<1, -1, write16_delegate>;
template class handler_entry_write_delegate<2,  3, write32_delegate>;
template class handler_entry_write_delegate<2,  0, write32_delegate>;
template class handler_entry_write_delegate<2, -1, write32_delegate>;
template class handler_entry_write_delegate<2, -2, write32_delegate>;
template class handler_entry_write_delegate<3,  0, write64_delegate>;
template class handler_entry_write_delegate<3, -1, write64_delegate>;
template class handler_entry_write_delegate<3, -2, write64_delegate>;
template class handler_entry_write_delegate<3, -3, write64_delegate>;

template class handler_entry_write_delegate<0,  1, write8m_delegate>;
template class handler_entry_write_delegate<0,  0, write8m_delegate>;
template class handler_entry_write_delegate<1,  3, write16m_delegate>;
template class handler_entry_write_delegate<1,  0, write16m_delegate>;
template class handler_entry_write_delegate<1, -1, write16m_delegate>;
template class handler_entry_write_delegate<2,  3, write32m_delegate>;
template class handler_entry_write_delegate<2,  0, write32m_delegate>;
template class handler_entry_write_delegate<2, -1, write32m_delegate>;
template class handler_entry_write_delegate<2, -2, write32m_delegate>;
template class handler_entry_write_delegate<3,  0, write64m_delegate>;
template class handler_entry_write_delegate<3, -1, write64m_delegate>;
template class handler_entry_write_delegate<3, -2, write64m_delegate>;
template class handler_entry_write_delegate<3, -3, write64m_delegate>;

template class handler_entry_write_delegate<0,  1, write8s_delegate>;
template class handler_entry_write_delegate<0,  0, write8s_delegate>;
template class handler_entry_write_delegate<1,  3, write16s_delegate>;
template class handler_entry_write_delegate<1,  0, write16s_delegate>;
template class handler_entry_write_delegate<1, -1, write16s_delegate>;
template class handler_entry_write_delegate<2,  3, write32s_delegate>;
template class handler_entry_write_delegate<2,  0, write32s_delegate>;
template class handler_entry_write_delegate<2, -1, write32s_delegate>;
template class handler_entry_write_delegate<2, -2, write32s_delegate>;
template class handler_entry_write_delegate<3,  0, write64s_delegate>;
template class handler_entry_write_delegate<3, -1, write64s_delegate>;
template class handler_entry_write_delegate<3, -2, write64s_delegate>;
template class handler_entry_write_delegate<3, -3, write64s_delegate>;

template class handler_entry_write_delegate<0,  1, write8sm_delegate>;
template class handler_entry_write_delegate<0,  0, write8sm_delegate>;
template class handler_entry_write_delegate<1,  3, write16sm_delegate>;
template class handler_entry_write_delegate<1,  0, write16sm_delegate>;
template class handler_entry_write_delegate<1, -1, write16sm_delegate>;
template class handler_entry_write_delegate<2,  3, write32sm_delegate>;
template class handler_entry_write_delegate<2,  0, write32sm_delegate>;
template class handler_entry_write_delegate<2, -1, write32sm_delegate>;
template class handler_entry_write_delegate<2, -2, write32sm_delegate>;
template class handler_entry_write_delegate<3,  0, write64sm_delegate>;
template class handler_entry_write_delegate<3, -1, write64sm_delegate>;
template class handler_entry_write_delegate<3, -2, write64sm_delegate>;
template class handler_entry_write_delegate<3, -3, write64sm_delegate>;

template class handler_entry_write_delegate<0,  1, write8mo_delegate>;
template class handler_entry_write_delegate<0,  0, write8mo_delegate>;
template class handler_entry_write_delegate<1,  3, write16mo_delegate>;
template class handler_entry_write_delegate<1,  0, write16mo_delegate>;
template class handler_entry_write_delegate<1, -1, write16mo_delegate>;
template class handler_entry_write_delegate<2,  3, write32mo_delegate>;
template class handler_entry_write_delegate<2,  0, write32mo_delegate>;
template class handler_entry_write_delegate<2, -1, write32mo_delegate>;
template class handler_entry_write_delegate<2, -2, write32mo_delegate>;
template class handler_entry_write_delegate<3,  0, write64mo_delegate>;
template class handler_entry_write_delegate<3, -1, write64mo_delegate>;
template class handler_entry_write_delegate<3, -2, write64mo_delegate>;
template class handler_entry_write_delegate<3, -3, write64mo_delegate>;

template class handler_entry_write_delegate<0,  1, write8smo_delegate>;
template class handler_entry_write_delegate<0,  0, write8smo_delegate>;
template class handler_entry_write_delegate<1,  3, write16smo_delegate>;
template class handler_entry_write_delegate<1,  0, write16smo_delegate>;
template class handler_entry_write_delegate<1, -1, write16smo_delegate>;
template class handler_entry_write_delegate<2,  3, write32smo_delegate>;
template class handler_entry_write_delegate<2,  0, write32smo_delegate>;
template class handler_entry_write_delegate<2, -1, write32smo_delegate>;
template class handler_entry_write_delegate<2, -2, write32smo_delegate>;
template class handler_entry_write_delegate<3,  0, write64smo_delegate>;
template class handler_entry_write_delegate<3, -1, write64smo_delegate>;
template class handler_entry_write_delegate<3, -2, write64smo_delegate>;
template class handler_entry_write_delegate<3, -3, write64smo_delegate>;


template class handler_entry_read_ioport<0,  1>;
template class handler_entry_read_ioport<0,  0>;
template class handler_entry_read_ioport<1,  3>;
template class handler_entry_read_ioport<1,  0>;
template class handler_entry_read_ioport<1, -1>;
template class handler_entry_read_ioport<2,  3>;
template class handler_entry_read_ioport<2,  0>;
template class handler_entry_read_ioport<2, -1>;
template class handler_entry_read_ioport<2, -2>;
template class handler_entry_read_ioport<3,  0>;
template class handler_entry_read_ioport<3, -1>;
template class handler_entry_read_ioport<3, -2>;
template class handler_entry_read_ioport<3, -3>;

template class handler_entry_write_ioport<0,  1>;
template class handler_entry_write_ioport<0,  0>;
template class handler_entry_write_ioport<1,  3>;
template class handler_entry_write_ioport<1,  0>;
template class handler_entry_write_ioport<1, -1>;
template class handler_entry_write_ioport<2,  3>;
template class handler_entry_write_ioport<2,  0>;
template class handler_entry_write_ioport<2, -1>;
template class handler_entry_write_ioport<2, -2>;
template class handler_entry_write_ioport<3,  0>;
template class handler_entry_write_ioport<3, -1>;
template class handler_entry_write_ioport<3, -2>;
template class handler_entry_write_ioport<3, -3>;
