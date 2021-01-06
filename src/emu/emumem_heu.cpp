// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "emumem_hea.h"
#include "emumem_mud.h"
#include "emumem_heu.h"


template<int Width, int AddrShift, endianness_t Endian> handler_entry_read_units<Width, AddrShift, Endian>::handler_entry_read_units(const memory_units_descriptor<Width, AddrShift, Endian> &descriptor, u8 ukey, address_space *space) :
	handler_entry_read<Width, AddrShift, Endian>(space, inh::F_UNITS),
	m_subunits(0)
{
	const auto &entries = descriptor.get_entries_for_key(ukey);
	fill(descriptor, entries);
	std::sort(m_subunit_infos, m_subunit_infos + m_subunits, [](const subunit_info &a, const subunit_info &b) { return a.m_offset < b.m_offset; });
}

template<int Width, int AddrShift, endianness_t Endian> handler_entry_read_units<Width, AddrShift, Endian>::handler_entry_read_units(const memory_units_descriptor<Width, AddrShift, Endian> &descriptor, u8 ukey, const handler_entry_read_units *src) :
	handler_entry_read<Width, AddrShift, Endian>(src->m_space, inh::F_UNITS),
	m_subunits(0)
{
	uX fullmask = 0;
	const auto &entries = descriptor.get_entries_for_key(ukey);
	for(const auto &e : entries)
		fullmask |= e.m_dmask;

	for(u32 i=0; i != src->m_subunits; i++)
		if(!(src->m_subunit_infos[i].m_dmask & fullmask)) {
			m_subunit_infos[m_subunits] = src->m_subunit_infos[i];
			m_subunit_infos[m_subunits].m_handler->ref();
			m_subunits++;
		}

	fill(descriptor, entries);
	std::sort(m_subunit_infos, m_subunit_infos + m_subunits, [](const subunit_info &a, const subunit_info &b) { return a.m_offset < b.m_offset; });
}

template<int Width, int AddrShift, endianness_t Endian> handler_entry_read_units<Width, AddrShift, Endian>::handler_entry_read_units(const handler_entry_read_units *src) :
	handler_entry_read<Width, AddrShift, Endian>(src->m_space, inh::F_UNITS),
	m_subunits(src->m_subunits)
{
	for(u32 i=0; i != src->m_subunits; i++) {
		m_subunit_infos[i] = src->m_subunit_infos[i];
		m_subunit_infos[i].m_handler = static_cast<handler_entry_write<Width, AddrShift, Endian> *>(m_subunit_infos[i].m_handler)->dup();
	}
}

template<int Width, int AddrShift, endianness_t Endian> handler_entry_read_units<Width, AddrShift, Endian>::~handler_entry_read_units()
{
	for(u32 i=0; i != m_subunits; i++)
		m_subunit_infos[i].m_handler->unref();
}

template<int Width, int AddrShift, endianness_t Endian> handler_entry_read<Width, AddrShift, Endian> *handler_entry_read_units<Width, AddrShift, Endian>::dup()
{
	return new handler_entry_read_units<Width, AddrShift, Endian>(this);
}

template<int Width, int AddrShift, endianness_t Endian> void handler_entry_read_units<Width, AddrShift, Endian>::enumerate_references(handler_entry::reflist &refs) const
{
	for(u32 i=0; i != m_subunits; i++)
		refs.add(m_subunit_infos[i].m_handler);
}

template<int Width, int AddrShift, endianness_t Endian> void handler_entry_read_units<Width, AddrShift, Endian>::fill(const memory_units_descriptor<Width, AddrShift, Endian> &descriptor, const std::vector<typename memory_units_descriptor<Width, AddrShift, Endian>::entry> &entries)
{
	handler_entry *handler = descriptor.get_subunit_handler();
	handler->ref(entries.size());
	for(const auto &e : entries)
		m_subunit_infos[m_subunits++] = subunit_info{ handler, e.m_amask, e.m_dmask, e.m_ashift, e.m_offset, e.m_dshift, descriptor.get_subunit_width(), descriptor.get_subunit_endian() };
	m_unmap = inh::m_space->unmap();
	for(int i = 0; i < m_subunits; i++)
		m_unmap &= ~m_subunit_infos[i].m_dmask;
}


template<int Width, int AddrShift, endianness_t Endian> typename emu::detail::handler_entry_size<Width>::uX handler_entry_read_units<Width, AddrShift, Endian>::read(offs_t offset, uX mem_mask) const
{
	this->ref();

	uX result = m_unmap;
	for (int index = 0; index < m_subunits; index++) {
		const subunit_info &si = m_subunit_infos[index];
		if (mem_mask & si.m_amask) {
			offs_t aoffset = (si.m_ashift >= 0 ? offset >> si.m_ashift : offset << si.m_ashift) + si.m_offset;
			switch(si.m_width) {
			case 0:
				if(si.m_endian == ENDIANNESS_LITTLE)
					result |= uX(static_cast<handler_entry_read<0,  0, ENDIANNESS_LITTLE> *>(si.m_handler)->read(aoffset, mem_mask >> si.m_dshift)) << si.m_dshift;
				else
					result |= uX(static_cast<handler_entry_read<0,  0, ENDIANNESS_BIG   > *>(si.m_handler)->read(aoffset, mem_mask >> si.m_dshift)) << si.m_dshift;
				break;
			case 1:
				if(si.m_endian == ENDIANNESS_LITTLE)
					result |= uX(static_cast<handler_entry_read<1, -1, ENDIANNESS_LITTLE> *>(si.m_handler)->read(aoffset, mem_mask >> si.m_dshift)) << si.m_dshift;
				else
					result |= uX(static_cast<handler_entry_read<1, -1, ENDIANNESS_BIG   > *>(si.m_handler)->read(aoffset, mem_mask >> si.m_dshift)) << si.m_dshift;
				break;
			case 2:
				if(si.m_endian == ENDIANNESS_LITTLE)
					result |= uX(static_cast<handler_entry_read<2, -2, ENDIANNESS_LITTLE> *>(si.m_handler)->read(aoffset, mem_mask >> si.m_dshift)) << si.m_dshift;
				else
					result |= uX(static_cast<handler_entry_read<2, -2, ENDIANNESS_BIG   > *>(si.m_handler)->read(aoffset, mem_mask >> si.m_dshift)) << si.m_dshift;
				break;
			default:
				abort();
			}
		}
	}

	this->unref();
	return result;
}

template<int Width, int AddrShift, endianness_t Endian> std::string handler_entry_read_units<Width, AddrShift, Endian>::m2r(typename emu::detail::handler_entry_size<Width>::uX mask)
{
	constexpr u32 mbits = 8*sizeof(uX);
	u32 start, end;
	for(start = 0; start < mbits && !(mask & (uX(1)<<start)); start += 8);
	for(end = 8*sizeof(uX) - 1; end < mbits && !(mask & (uX(1)<<end)); end -= 8);
	if(start >= mbits || end >= mbits)
		return "???";
	return util::string_format("%d-%d", end, start);
}

template<int Width, int AddrShift, endianness_t Endian> std::string handler_entry_read_units<Width, AddrShift, Endian>::name() const
{
	std::string result;

	for (int index = 0; index < m_subunits; index++) {
		const subunit_info &si = m_subunit_infos[index];
		if(!result.empty())
			result += ' ';
		result += util::string_format("%s:%s:%d:%d:%d:%s", m2r(si.m_amask), m2r(si.m_dmask), si.m_ashift, si.m_offset, si.m_dshift, si.m_handler->name());
	}

	return result;
}




template<int Width, int AddrShift, endianness_t Endian> handler_entry_write_units<Width, AddrShift, Endian>::handler_entry_write_units(const memory_units_descriptor<Width, AddrShift, Endian> &descriptor, u8 ukey, address_space *space) :
	handler_entry_write<Width, AddrShift, Endian>(space, inh::F_UNITS),
	m_subunits(0)
{
	const auto &entries = descriptor.get_entries_for_key(ukey);
	fill(descriptor, entries);
	std::sort(m_subunit_infos, m_subunit_infos + m_subunits, [](const subunit_info &a, const subunit_info &b) { return a.m_offset < b.m_offset; });
}

template<int Width, int AddrShift, endianness_t Endian> handler_entry_write_units<Width, AddrShift, Endian>::handler_entry_write_units(const memory_units_descriptor<Width, AddrShift, Endian> &descriptor, u8 ukey, const handler_entry_write_units<Width, AddrShift, Endian> *src) :
	handler_entry_write<Width, AddrShift, Endian>(src->m_space, inh::F_UNITS),
	m_subunits(0)
{
	uX fullmask = 0;
	const auto &entries = descriptor.get_entries_for_key(ukey);
	for(const auto &e : entries)
		fullmask |= e.m_dmask;

	for(u32 i=0; i != src->m_subunits; i++)
		if(!(src->m_subunit_infos[i].m_dmask & fullmask)) {
			m_subunit_infos[m_subunits] = src->m_subunit_infos[i];
			m_subunit_infos[m_subunits].m_handler->ref();
			m_subunits++;
		}

	fill(descriptor, entries);
	std::sort(m_subunit_infos, m_subunit_infos + m_subunits, [](const subunit_info &a, const subunit_info &b) { return a.m_offset < b.m_offset; });
}

template<int Width, int AddrShift, endianness_t Endian> handler_entry_write_units<Width, AddrShift, Endian>::handler_entry_write_units(const handler_entry_write_units *src) :
	handler_entry_write<Width, AddrShift, Endian>(src->m_space, inh::F_UNITS),
	m_subunits(src->m_subunits)
{
	for(u32 i=0; i != src->m_subunits; i++) {
		m_subunit_infos[i] = src->m_subunit_infos[i];
		m_subunit_infos[i].m_handler = static_cast<handler_entry_write<Width, AddrShift, Endian> *>(m_subunit_infos[i].m_handler)->dup();
	}
}

template<int Width, int AddrShift, endianness_t Endian> handler_entry_write_units<Width, AddrShift, Endian>::~handler_entry_write_units()
{
	for(u32 i=0; i != m_subunits; i++)
		m_subunit_infos[i].m_handler->unref();
}

template<int Width, int AddrShift, endianness_t Endian> handler_entry_write<Width, AddrShift, Endian> *handler_entry_write_units<Width, AddrShift, Endian>::dup()
{
	return new handler_entry_write_units<Width, AddrShift, Endian>(this);
}

template<int Width, int AddrShift, endianness_t Endian> void handler_entry_write_units<Width, AddrShift, Endian>::enumerate_references(handler_entry::reflist &refs) const
{
	for(u32 i=0; i != m_subunits; i++)
		refs.add(m_subunit_infos[i].m_handler);
}

template<int Width, int AddrShift, endianness_t Endian> void handler_entry_write_units<Width, AddrShift, Endian>::fill(const memory_units_descriptor<Width, AddrShift, Endian> &descriptor, const std::vector<typename memory_units_descriptor<Width, AddrShift, Endian>::entry> &entries)
{
	handler_entry *handler = descriptor.get_subunit_handler();
	handler->ref(entries.size());
	for(const auto &e : entries)
		m_subunit_infos[m_subunits++] = subunit_info{ handler, e.m_amask, e.m_dmask, e.m_ashift, e.m_offset, e.m_dshift, descriptor.get_subunit_width(), descriptor.get_subunit_endian() };
}

template<int Width, int AddrShift, endianness_t Endian> void handler_entry_write_units<Width, AddrShift, Endian>::write(offs_t offset, uX data, uX mem_mask) const
{
	this->ref();

	for (int index = 0; index < m_subunits; index++) {
		const subunit_info &si = m_subunit_infos[index];
		if (mem_mask & si.m_amask) {
			offs_t aoffset = (si.m_ashift >= 0 ? offset >> si.m_ashift : offset << si.m_ashift) + si.m_offset;
			switch(si.m_width) {
			case 0:
				if(si.m_endian == ENDIANNESS_LITTLE)
					static_cast<handler_entry_write<0,  0, ENDIANNESS_LITTLE> *>(si.m_handler)->write(aoffset, data >> si.m_dshift, mem_mask >> si.m_dshift);
				else
					static_cast<handler_entry_write<0,  0, ENDIANNESS_BIG   > *>(si.m_handler)->write(aoffset, data >> si.m_dshift, mem_mask >> si.m_dshift);
				break;
			case 1:
				if(si.m_endian == ENDIANNESS_LITTLE)
					static_cast<handler_entry_write<1, -1, ENDIANNESS_LITTLE> *>(si.m_handler)->write(aoffset, data >> si.m_dshift, mem_mask >> si.m_dshift);
				else
					static_cast<handler_entry_write<1, -1, ENDIANNESS_BIG   > *>(si.m_handler)->write(aoffset, data >> si.m_dshift, mem_mask >> si.m_dshift);
				break;
			case 2:
				if(si.m_endian == ENDIANNESS_LITTLE)
					static_cast<handler_entry_write<2, -2, ENDIANNESS_LITTLE> *>(si.m_handler)->write(aoffset, data >> si.m_dshift, mem_mask >> si.m_dshift);
				else
					static_cast<handler_entry_write<2, -2, ENDIANNESS_BIG   > *>(si.m_handler)->write(aoffset, data >> si.m_dshift, mem_mask >> si.m_dshift);
				break;
			default:
				abort();
			}
		}
	}

	this->unref();
}


template<int Width, int AddrShift, endianness_t Endian> std::string handler_entry_write_units<Width, AddrShift, Endian>::m2r(typename emu::detail::handler_entry_size<Width>::uX mask)
{
	constexpr u32 mbits = 8*sizeof(uX);
	u32 start, end;
	for(start = 0; start < mbits && !(mask & (uX(1)<<start)); start += 8);
	for(end = 8*sizeof(uX) - 1; end < mbits && !(mask & (uX(1)<<end)); end -= 8);
	if(start >= mbits || end >= mbits)
		return "???";
	return util::string_format("%d-%d", end, start);
}

template<int Width, int AddrShift, endianness_t Endian> std::string handler_entry_write_units<Width, AddrShift, Endian>::name() const
{
	std::string result;

	for (int index = 0; index < m_subunits; index++) {
		const subunit_info &si = m_subunit_infos[index];
		if(!result.empty())
			result += ' ';
		result += util::string_format("%s:%s:%d:%d:%d:%s", m2r(si.m_amask), m2r(si.m_dmask), si.m_ashift, si.m_offset, si.m_dshift, si.m_handler->name());
	}

	return result;
}


template class handler_entry_read_units<0,  1, ENDIANNESS_LITTLE>;
template class handler_entry_read_units<0,  1, ENDIANNESS_BIG>;
template class handler_entry_read_units<0,  0, ENDIANNESS_LITTLE>;
template class handler_entry_read_units<0,  0, ENDIANNESS_BIG>;
template class handler_entry_read_units<1,  3, ENDIANNESS_LITTLE>;
template class handler_entry_read_units<1,  3, ENDIANNESS_BIG>;
template class handler_entry_read_units<1,  0, ENDIANNESS_LITTLE>;
template class handler_entry_read_units<1,  0, ENDIANNESS_BIG>;
template class handler_entry_read_units<1, -1, ENDIANNESS_LITTLE>;
template class handler_entry_read_units<1, -1, ENDIANNESS_BIG>;
template class handler_entry_read_units<2,  3, ENDIANNESS_LITTLE>;
template class handler_entry_read_units<2,  3, ENDIANNESS_BIG>;
template class handler_entry_read_units<2,  0, ENDIANNESS_LITTLE>;
template class handler_entry_read_units<2,  0, ENDIANNESS_BIG>;
template class handler_entry_read_units<2, -1, ENDIANNESS_LITTLE>;
template class handler_entry_read_units<2, -1, ENDIANNESS_BIG>;
template class handler_entry_read_units<2, -2, ENDIANNESS_LITTLE>;
template class handler_entry_read_units<2, -2, ENDIANNESS_BIG>;
template class handler_entry_read_units<3,  0, ENDIANNESS_LITTLE>;
template class handler_entry_read_units<3,  0, ENDIANNESS_BIG>;
template class handler_entry_read_units<3, -1, ENDIANNESS_LITTLE>;
template class handler_entry_read_units<3, -1, ENDIANNESS_BIG>;
template class handler_entry_read_units<3, -2, ENDIANNESS_LITTLE>;
template class handler_entry_read_units<3, -2, ENDIANNESS_BIG>;
template class handler_entry_read_units<3, -3, ENDIANNESS_LITTLE>;
template class handler_entry_read_units<3, -3, ENDIANNESS_BIG>;

template class handler_entry_write_units<0,  1, ENDIANNESS_LITTLE>;
template class handler_entry_write_units<0,  1, ENDIANNESS_BIG>;
template class handler_entry_write_units<0,  0, ENDIANNESS_LITTLE>;
template class handler_entry_write_units<0,  0, ENDIANNESS_BIG>;
template class handler_entry_write_units<1,  3, ENDIANNESS_LITTLE>;
template class handler_entry_write_units<1,  3, ENDIANNESS_BIG>;
template class handler_entry_write_units<1,  0, ENDIANNESS_LITTLE>;
template class handler_entry_write_units<1,  0, ENDIANNESS_BIG>;
template class handler_entry_write_units<1, -1, ENDIANNESS_LITTLE>;
template class handler_entry_write_units<1, -1, ENDIANNESS_BIG>;
template class handler_entry_write_units<2,  3, ENDIANNESS_LITTLE>;
template class handler_entry_write_units<2,  3, ENDIANNESS_BIG>;
template class handler_entry_write_units<2,  0, ENDIANNESS_LITTLE>;
template class handler_entry_write_units<2,  0, ENDIANNESS_BIG>;
template class handler_entry_write_units<2, -1, ENDIANNESS_LITTLE>;
template class handler_entry_write_units<2, -1, ENDIANNESS_BIG>;
template class handler_entry_write_units<2, -2, ENDIANNESS_LITTLE>;
template class handler_entry_write_units<2, -2, ENDIANNESS_BIG>;
template class handler_entry_write_units<3,  0, ENDIANNESS_LITTLE>;
template class handler_entry_write_units<3,  0, ENDIANNESS_BIG>;
template class handler_entry_write_units<3, -1, ENDIANNESS_LITTLE>;
template class handler_entry_write_units<3, -1, ENDIANNESS_BIG>;
template class handler_entry_write_units<3, -2, ENDIANNESS_LITTLE>;
template class handler_entry_write_units<3, -2, ENDIANNESS_BIG>;
template class handler_entry_write_units<3, -3, ENDIANNESS_LITTLE>;
template class handler_entry_write_units<3, -3, ENDIANNESS_BIG>;
