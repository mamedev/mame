// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "emumem_mud.h"
#include "emumem_hea.h"
#include "emumem_heu.h"
#include "emumem_heun.h"
#include "emumem_hep.h"
#include "emumem_hedw.h"


template<int HighBits, int Width, int AddrShift, endianness_t Endian> const handler_entry_write<Width, AddrShift, Endian> *const *handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::get_dispatch() const
{
	return m_a_dispatch;
}

template<int HighBits, int Width, int AddrShift, endianness_t Endian> handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::handler_entry_write_dispatch(address_space *space, const handler_entry::range &init, handler_entry_write<Width, AddrShift, Endian> *handler) : handler_entry_write<Width, AddrShift, Endian>(space, handler_entry::F_DISPATCH), m_view(nullptr)
{
	m_ranges_array.resize(1);
	m_dispatch_array.resize(1);
	m_a_ranges = m_ranges_array[0].data();
	m_a_dispatch = m_dispatch_array[0].data();
	m_u_ranges = m_ranges_array[0].data();
	m_u_dispatch = m_dispatch_array[0].data();

	if (!handler)
		handler = space->get_unmap_w<Width, AddrShift, Endian>();
	handler->ref(COUNT);
	for(unsigned int i=0; i != COUNT; i++) {
		m_u_dispatch[i] = handler;
		m_u_ranges[i] = init;
	}
}

template<int HighBits, int Width, int AddrShift, endianness_t Endian> handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::handler_entry_write_dispatch(address_space *space, memory_view &view) : handler_entry_write<Width, AddrShift, Endian>(space, handler_entry::F_VIEW), m_view(&view), m_a_dispatch(nullptr), m_a_ranges(nullptr), m_u_dispatch(nullptr), m_u_ranges(nullptr)
{
}


template<int HighBits, int Width, int AddrShift, endianness_t Endian> handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::handler_entry_write_dispatch(handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian> *src) : handler_entry_write<Width, AddrShift, Endian>(src->m_space, handler_entry::F_DISPATCH), m_view(nullptr)
{
	m_ranges_array.resize(1);
	m_dispatch_array.resize(1);
	m_a_ranges = m_ranges_array[0].data();
	m_a_dispatch = m_dispatch_array[0].data();
	m_u_ranges = m_ranges_array[0].data();
	m_u_dispatch = m_dispatch_array[0].data();

	for(unsigned int i=0; i != COUNT; i++) {
		m_u_dispatch[i] = src->m_u_dispatch[i]->dup();
		m_u_ranges[i] = src->m_u_ranges[i];
	}
}


template<int HighBits, int Width, int AddrShift, endianness_t Endian> handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::~handler_entry_write_dispatch()
{
	for(auto &d : m_dispatch_array)
		for(auto p : d)
			if(p)
				p->unref();
}

template<int HighBits, int Width, int AddrShift, endianness_t Endian> void handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::enumerate_references(handler_entry::reflist &refs) const
{
	for(auto &d : m_dispatch_array)
		for(auto p : d)
			if(p)
				refs.add(p);
}

template<int HighBits, int Width, int AddrShift, endianness_t Endian> void handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::dump_map(std::vector<memory_entry> &map) const
{
	if(m_view) {
		for(u32 i = 0; i != m_dispatch_array.size(); i++) {
			u32 j = map.size();
			offs_t cur = map.empty() ? m_view->m_addrstart & HIGHMASK : map.back().end + 1;
			offs_t end = m_view->m_addrend + 1;
			do {
				offs_t entry = (cur >> LowBits) & BITMASK;
				if(m_dispatch_array[i][entry]->is_dispatch() || m_dispatch_array[i][entry]->is_view())
					m_dispatch_array[i][entry]->dump_map(map);
				else
					map.emplace_back(memory_entry{ m_ranges_array[i][entry].start, m_ranges_array[i][entry].end, m_dispatch_array[i][entry] });
				cur = map.back().end + 1;
			} while(cur != end);
			if(i == 0) {
				for(u32 k = j; k != map.size(); k++)
					map[k].context.emplace(map[k].context.begin(), memory_entry_context{ m_view, true, 0 });
			} else {
				int slot = m_view->id_to_slot(int(i)-1);
				for(u32 k = j; k != map.size(); k++)
					map[k].context.emplace(map[k].context.begin(), memory_entry_context{ m_view, false, slot });
			}
		}
	} else {
		offs_t cur = map.empty() ? 0 : map.back().end + 1;
		offs_t base = cur & UPMASK;
		do {
			offs_t entry = (cur >> LowBits) & BITMASK;
			if(m_a_dispatch[entry]->is_dispatch() || m_a_dispatch[entry]->is_view())
				m_a_dispatch[entry]->dump_map(map);
			else
				map.emplace_back(memory_entry{ m_a_ranges[entry].start, m_a_ranges[entry].end, m_a_dispatch[entry] });
			cur = map.back().end + 1;
		} while(cur && !((cur ^ base) & UPMASK));
	}
}

template<int HighBits, int Width, int AddrShift, endianness_t Endian> void handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::write(offs_t offset, uX data, uX mem_mask) const
{
	dispatch_write<Level, Width, AddrShift, Endian>(HIGHMASK, offset, data, mem_mask, m_a_dispatch);
}

template<int HighBits, int Width, int AddrShift, endianness_t Endian> void *handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::get_ptr(offs_t offset) const
{
	return m_a_dispatch[(offset & HIGHMASK) >> LowBits]->get_ptr(offset);
}

template<int HighBits, int Width, int AddrShift, endianness_t Endian> std::string handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::name() const
{
	return m_view ? "view" :"dispatch";
}

template<int HighBits, int Width, int AddrShift, endianness_t Endian> void handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::lookup(offs_t address, offs_t &start, offs_t &end, handler_entry_write<Width, AddrShift, Endian> *&handler) const
{
	offs_t slot = (address >> LowBits) & BITMASK;
	auto h = m_a_dispatch[slot];
	if(h->is_dispatch() || h->is_view())
		h->lookup(address, start, end, handler);
	else {
		start = m_a_ranges[slot].start;
		end = m_a_ranges[slot].end;
		handler = h;
	}
}

template<int HighBits, int Width, int AddrShift, endianness_t Endian> void handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::range_cut_before(offs_t address, int start)
{
	while(--start >= 0) {
		if(int(LowBits) > -AddrShift && m_u_dispatch[start]->is_dispatch()) {
			static_cast<handler_entry_write_dispatch<LowBits, Width, AddrShift, Endian> *>(m_u_dispatch[start])->range_cut_before(address);
			break;
		}
		if(m_u_ranges[start].end <= address)
			break;
		m_u_ranges[start].end = address;
	}
}

template<int HighBits, int Width, int AddrShift, endianness_t Endian> void handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::range_cut_after(offs_t address, int start)
{
	while(++start < COUNT) {
		if(int(LowBits) > -AddrShift && m_u_dispatch[start]->is_dispatch()) {
			static_cast<handler_entry_write_dispatch<LowBits, Width, AddrShift, Endian> *>(m_u_dispatch[start])->range_cut_after(address);
			break;
		}
		if(m_u_ranges[start].start >= address)
			break;
		m_u_ranges[start].start = address;
	}
}

template<int HighBits, int Width, int AddrShift, endianness_t Endian> void handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::populate_nomirror_subdispatch(offs_t entry, offs_t start, offs_t end, offs_t ostart, offs_t oend, handler_entry_write<Width, AddrShift, Endian> *handler)
{
	auto cur = m_u_dispatch[entry];
	if(cur->is_dispatch())
		cur->populate_nomirror(start, end, ostart, oend, handler);
	else {
		auto subdispatch = new handler_entry_write_dispatch<LowBits, Width, AddrShift, Endian>(handler_entry::m_space, m_u_ranges[entry], cur);
		cur->unref();
		m_u_dispatch[entry] = subdispatch;
		subdispatch->populate_nomirror(start, end, ostart, oend, handler);
	}
}

template<int HighBits, int Width, int AddrShift, endianness_t Endian> void handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::populate_nomirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, handler_entry_write<Width, AddrShift, Endian> *handler)
{
	offs_t start_entry = (start & HIGHMASK) >> LowBits;
	offs_t end_entry = (end & HIGHMASK) >> LowBits;
	range_cut_before(ostart-1, start_entry);
	range_cut_after(oend+1, end_entry);

	if(LowBits <= Width + AddrShift) {
		if(handler->is_view())
			handler->init_handlers(start_entry, end_entry, LowBits, m_u_dispatch, m_u_ranges);
		handler->ref(end_entry - start_entry);
		for(offs_t ent = start_entry; ent <= end_entry; ent++) {
			m_u_dispatch[ent]->unref();
			m_u_dispatch[ent] = handler;
			m_u_ranges[ent].set(ostart, oend);
		}

	} else if(start_entry == end_entry) {
		if(!(start & LOWMASK) && (end & LOWMASK) == LOWMASK) {
			if(handler->is_view())
				handler->init_handlers(start_entry, end_entry, LowBits, m_u_dispatch, m_u_ranges);
			m_u_dispatch[start_entry]->unref();
			m_u_dispatch[start_entry] = handler;
			m_u_ranges[start_entry].set(ostart, oend);
		} else
			populate_nomirror_subdispatch(start_entry, start & LOWMASK, end & LOWMASK, ostart, oend, handler);

	} else {
		if(start & LOWMASK) {
			populate_nomirror_subdispatch(start_entry, start & LOWMASK, LOWMASK, ostart, oend, handler);
			start_entry++;
			if(start_entry <= end_entry)
				handler->ref();
		}
		if((end & LOWMASK) != LOWMASK) {
			populate_nomirror_subdispatch(end_entry, 0, end & LOWMASK, ostart, oend, handler);
			end_entry--;
			if(start_entry <= end_entry)
				handler->ref();
		}

		if(start_entry <= end_entry) {
			if(handler->is_view())
				handler->init_handlers(start_entry, end_entry, LowBits, m_u_dispatch, m_u_ranges);
			handler->ref(end_entry - start_entry);
			for(offs_t ent = start_entry; ent <= end_entry; ent++) {
				m_u_dispatch[ent]->unref();
				m_u_dispatch[ent] = handler;
				m_u_ranges[ent].set(ostart, oend);
			}
		}
	}
}

template<int HighBits, int Width, int AddrShift, endianness_t Endian> void handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::populate_mirror_subdispatch(offs_t entry, offs_t start, offs_t end, offs_t ostart, offs_t oend, offs_t mirror, handler_entry_write<Width, AddrShift, Endian> *handler)
{
	auto cur = m_u_dispatch[entry];
	if(cur->is_dispatch())
		cur->populate_mirror(start, end, ostart, oend, mirror, handler);
	else {
		auto subdispatch = new handler_entry_write_dispatch<LowBits, Width, AddrShift, Endian>(handler_entry::m_space, m_u_ranges[entry], cur);
		cur->unref();
		m_u_dispatch[entry] = subdispatch;
		subdispatch->populate_mirror(start, end, ostart, oend, mirror, handler);
	}
}

template<int HighBits, int Width, int AddrShift, endianness_t Endian> void handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::populate_mirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, offs_t mirror, handler_entry_write<Width, AddrShift, Endian> *handler)
{
	offs_t hmirror = mirror & HIGHMASK;
	offs_t lmirror = mirror & LOWMASK;

	if(lmirror) {
		// If lmirror is non-zero, then each mirror instance is a single entry
		offs_t add = 1 + ~hmirror;
		offs_t offset = 0;
		offs_t base_entry = start >> LowBits;
		start &= LOWMASK;
		end &= LOWMASK;
		do {
			if(offset)
				handler->ref();
			populate_mirror_subdispatch(base_entry | (offset >> LowBits), start, end, ostart | offset, oend | offset, lmirror, handler);
			offset = (offset + add) & hmirror;
		} while(offset);
	} else {
		// If lmirror is zero, call the nomirror version as needed
		offs_t add = 1 + ~hmirror;
		offs_t offset = 0;
		do {
			if(offset)
				handler->ref();
			populate_nomirror(start | offset, end | offset, ostart | offset, oend | offset, handler);
			offset = (offset + add) & hmirror;
		} while(offset);
	}
}

template<int HighBits, int Width, int AddrShift, endianness_t Endian> void handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::mismatched_patch(const memory_units_descriptor<Width, AddrShift, Endian> &descriptor, u8 rkey, std::vector<mapping> &mappings, handler_entry_write<Width, AddrShift, Endian> *&target)
{
	u8 ukey = descriptor.rkey_to_ukey(rkey);
	handler_entry_write<Width, AddrShift, Endian> *original = target->is_units() ? target : nullptr;
	handler_entry_write<Width, AddrShift, Endian> *replacement = nullptr;
	for(const auto &p : mappings)
		if(p.ukey == ukey && p.original == original) {
			replacement = p.patched;
			break;
		}
	if(!replacement) {
		if(original)
			replacement = new handler_entry_write_units<Width, AddrShift, Endian>(descriptor, ukey, static_cast<handler_entry_write_units<Width, AddrShift, Endian> *>(original));
		else
			replacement = new handler_entry_write_units<Width, AddrShift, Endian>(descriptor, ukey, inh::m_space);

		mappings.emplace_back(mapping{ original, replacement, ukey });
	} else
		replacement->ref();
	target->unref();
	target = replacement;
}

template<int HighBits, int Width, int AddrShift, endianness_t Endian> void handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::populate_mismatched_nomirror_subdispatch(offs_t entry, offs_t start, offs_t end, offs_t ostart, offs_t oend, const memory_units_descriptor<Width, AddrShift, Endian> &descriptor, u8 rkey, std::vector<mapping> &mappings)
{
	auto cur = m_u_dispatch[entry];
	if(cur->is_dispatch())
		cur->populate_mismatched_nomirror(start, end, ostart, oend, descriptor, rkey, mappings);
	else {
		auto subdispatch = new handler_entry_write_dispatch<LowBits, Width, AddrShift, Endian>(handler_entry::m_space, m_u_ranges[entry], cur);
		cur->unref();
		m_u_dispatch[entry] = subdispatch;
		subdispatch->populate_mismatched_nomirror(start, end, ostart, oend, descriptor, rkey, mappings);
	}
}

template<int HighBits, int Width, int AddrShift, endianness_t Endian> void handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::populate_mismatched_nomirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, const memory_units_descriptor<Width, AddrShift, Endian> &descriptor, u8 rkey, std::vector<mapping> &mappings)
{
	offs_t start_entry = (start & HIGHMASK) >> LowBits;
	offs_t end_entry = (end & HIGHMASK) >> LowBits;
	range_cut_before(ostart-1, start_entry);
	range_cut_after(oend+1, end_entry);

	if(LowBits <= Width + AddrShift) {
		for(offs_t ent = start_entry; ent <= end_entry; ent++) {
			u8 rkey1 = rkey;
			if(ent != start_entry)
				rkey1 &= ~handler_entry::START;
			if(ent != end_entry)
				rkey1 &= ~handler_entry::END;
			mismatched_patch(descriptor, rkey1, mappings, m_u_dispatch[ent]);
			m_u_ranges[ent].intersect(ostart, oend);
		}

	} else if(start_entry == end_entry) {
		if(!(start & LOWMASK) && (end & LOWMASK) == LOWMASK) {
			if(m_u_dispatch[start_entry]->is_dispatch())
				m_u_dispatch[start_entry]->populate_mismatched_nomirror(start & LOWMASK, end & LOWMASK, ostart, oend, descriptor, rkey, mappings);
			else {
				mismatched_patch(descriptor, rkey, mappings, m_u_dispatch[start_entry]);
				m_u_ranges[start_entry].intersect(ostart, oend);
			}
		} else
			populate_mismatched_nomirror_subdispatch(start_entry, start & LOWMASK, end & LOWMASK, ostart, oend, descriptor, rkey, mappings);

	} else {
		if(start & LOWMASK) {
			populate_mismatched_nomirror_subdispatch(start_entry, start & LOWMASK, LOWMASK, ostart, oend, descriptor, rkey & ~handler_entry::END, mappings);
			start_entry++;
			rkey &= ~handler_entry::START;
		}
		if((end & LOWMASK) != LOWMASK) {
			populate_mismatched_nomirror_subdispatch(end_entry, 0, end & LOWMASK, ostart, oend, descriptor, rkey & ~handler_entry::START, mappings);
			end_entry--;
			rkey &= ~handler_entry::END;
		}

		if(start_entry <= end_entry) {
			for(offs_t ent = start_entry; ent <= end_entry; ent++) {
				u8 rkey1 = rkey;
				if(ent != start_entry)
					rkey1 &= ~handler_entry::START;
				if(ent != end_entry)
					rkey1 &= ~handler_entry::END;
				if(m_u_dispatch[ent]->is_dispatch())
					m_u_dispatch[ent]->populate_mismatched_nomirror(start & LOWMASK, end & LOWMASK, ostart, oend, descriptor, rkey1, mappings);
				else {
					mismatched_patch(descriptor, rkey1, mappings, m_u_dispatch[ent]);
					m_u_ranges[ent].intersect(ostart, oend);
				}
			}
		}
	}
}

template<int HighBits, int Width, int AddrShift, endianness_t Endian> void handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::populate_mismatched_mirror_subdispatch(offs_t entry, offs_t start, offs_t end, offs_t ostart, offs_t oend, offs_t mirror, const memory_units_descriptor<Width, AddrShift, Endian> &descriptor, std::vector<mapping> &mappings)
{
	auto cur = m_u_dispatch[entry];
	if(cur->is_dispatch())
		cur->populate_mismatched_mirror(start, end, ostart, oend, mirror, descriptor, mappings);
	else {
		auto subdispatch = new handler_entry_write_dispatch<LowBits, Width, AddrShift, Endian>(handler_entry::m_space, m_u_ranges[entry], cur);
		cur->unref();
		m_u_dispatch[entry] = subdispatch;
		subdispatch->populate_mismatched_mirror(start, end, ostart, oend, mirror, descriptor, mappings);
	}
}

template<int HighBits, int Width, int AddrShift, endianness_t Endian> void handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::populate_mismatched_mirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, offs_t mirror, const memory_units_descriptor<Width, AddrShift, Endian> &descriptor, std::vector<mapping> &mappings)
{
	offs_t hmirror = mirror & HIGHMASK;
	offs_t lmirror = mirror & LOWMASK;

	if(lmirror) {
		// If lmirror is non-zero, then each mirror instance is a single entry
		offs_t add = 1 + ~hmirror;
		offs_t offset = 0;
		offs_t base_entry = start >> LowBits;
		start &= LOWMASK;
		end &= LOWMASK;
		do {
			populate_mismatched_mirror_subdispatch(base_entry | (offset >> LowBits), start, end, ostart | offset, oend | offset, lmirror, descriptor, mappings);
			offset = (offset + add) & hmirror;
		} while(offset);
	} else {
		// If lmirror is zero, call the nomirror version as needed
		offs_t add = 1 + ~hmirror;
		offs_t offset = 0;
		do {
			populate_mismatched_nomirror(start | offset, end | offset, ostart | offset, oend | offset, descriptor, handler_entry::START|handler_entry::END, mappings);
			offset = (offset + add) & hmirror;
		} while(offset);
	}
}


template<int HighBits, int Width, int AddrShift, endianness_t Endian> void handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::passthrough_patch(handler_entry_write_passthrough<Width, AddrShift, Endian> *handler, std::vector<mapping> &mappings, handler_entry_write<Width, AddrShift, Endian> *&target)
{
	handler_entry_write<Width, AddrShift, Endian> *original = target;
	handler_entry_write<Width, AddrShift, Endian> *replacement = nullptr;
	for(const auto &p : mappings)
		if(p.original == original) {
			replacement = p.patched;
			break;
		}
	if(!replacement) {
		replacement = handler->instantiate(original);
		mappings.emplace_back(mapping{ original, replacement });
	} else
		replacement->ref();
	target->unref();
	target = replacement;
}

template<int HighBits, int Width, int AddrShift, endianness_t Endian> void handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::populate_passthrough_nomirror_subdispatch(offs_t entry, offs_t start, offs_t end, offs_t ostart, offs_t oend, handler_entry_write_passthrough<Width, AddrShift, Endian> *handler, std::vector<mapping> &mappings)
{
	auto cur = m_u_dispatch[entry];
	if(cur->is_dispatch())
		cur->populate_passthrough_nomirror(start, end, ostart, oend, handler, mappings);
	else {
		auto subdispatch = new handler_entry_write_dispatch<LowBits, Width, AddrShift, Endian>(handler_entry::m_space, m_u_ranges[entry], cur);
		cur->unref();
		m_u_dispatch[entry] = subdispatch;
		subdispatch->populate_passthrough_nomirror(start, end, ostart, oend, handler, mappings);
	}
}

template<int HighBits, int Width, int AddrShift, endianness_t Endian> void handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::populate_passthrough_nomirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, handler_entry_write_passthrough<Width, AddrShift, Endian> *handler, std::vector<mapping> &mappings)
{
	offs_t start_entry = (start & HIGHMASK) >> LowBits;
	offs_t end_entry = (end & HIGHMASK) >> LowBits;
	range_cut_before(ostart-1, start_entry);
	range_cut_after(oend+1, end_entry);

	if(LowBits <= Width + AddrShift) {
		for(offs_t ent = start_entry; ent <= end_entry; ent++) {
			passthrough_patch(handler, mappings, m_u_dispatch[ent]);
			m_u_ranges[ent].intersect(ostart, oend);
		}

	} else if(start_entry == end_entry) {
		if(!(start & LOWMASK) && (end & LOWMASK) == LOWMASK) {
			if(m_u_dispatch[start_entry]->is_dispatch())
				m_u_dispatch[start_entry]->populate_passthrough_nomirror(start & LOWMASK, end & LOWMASK, ostart, oend, handler, mappings);
			else {
				passthrough_patch(handler, mappings, m_u_dispatch[start_entry]);
				m_u_ranges[start_entry].intersect(ostart, oend);
			}
		} else
			populate_passthrough_nomirror_subdispatch(start_entry, start & LOWMASK, end & LOWMASK, ostart, oend, handler, mappings);

	} else {
		if(start & LOWMASK) {
			populate_passthrough_nomirror_subdispatch(start_entry, start & LOWMASK, LOWMASK, ostart, oend, handler, mappings);
			start_entry++;
		}
		if((end & LOWMASK) != LOWMASK) {
			populate_passthrough_nomirror_subdispatch(end_entry, 0, end & LOWMASK, ostart, oend, handler, mappings);
			end_entry--;
		}

		if(start_entry <= end_entry) {
			for(offs_t ent = start_entry; ent <= end_entry; ent++) {
				if(m_u_dispatch[ent]->is_dispatch())
					m_u_dispatch[ent]->populate_passthrough_nomirror(start & LOWMASK, end & LOWMASK, ostart, oend, handler, mappings);
				else {
					passthrough_patch(handler, mappings, m_u_dispatch[ent]);
					m_u_ranges[ent].intersect(ostart, oend);
				}
			}
		}
	}
}

template<int HighBits, int Width, int AddrShift, endianness_t Endian> void handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::populate_passthrough_mirror_subdispatch(offs_t entry, offs_t start, offs_t end, offs_t ostart, offs_t oend, offs_t mirror, handler_entry_write_passthrough<Width, AddrShift, Endian> *handler, std::vector<mapping> &mappings)
{
	auto cur = m_u_dispatch[entry];
	if(cur->is_dispatch())
		cur->populate_passthrough_mirror(start, end, ostart, oend, mirror, handler, mappings);
	else {
		auto subdispatch = new handler_entry_write_dispatch<LowBits, Width, AddrShift, Endian>(handler_entry::m_space, m_u_ranges[entry], cur);
		cur->unref();
		m_u_dispatch[entry] = subdispatch;
		subdispatch->populate_passthrough_mirror(start, end, ostart, oend, mirror, handler, mappings);
	}
}

template<int HighBits, int Width, int AddrShift, endianness_t Endian> void handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::populate_passthrough_mirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, offs_t mirror, handler_entry_write_passthrough<Width, AddrShift, Endian> *handler, std::vector<mapping> &mappings)
{
	offs_t hmirror = mirror & HIGHMASK;
	offs_t lmirror = mirror & LOWMASK;

	if(lmirror) {
		// If lmirror is non-zero, then each mirror instance is a single entry
		offs_t add = 1 + ~hmirror;
		offs_t offset = 0;
		offs_t base_entry = start >> LowBits;
		start &= LOWMASK;
		end &= LOWMASK;
		do {
			populate_passthrough_mirror_subdispatch(base_entry | (offset >> LowBits), start, end, ostart | offset, oend | offset, lmirror, handler, mappings);
			offset = (offset + add) & hmirror;
		} while(offset);
	} else {
		// If lmirror is zero, call the nomirror version as needed
		offs_t add = 1 + ~hmirror;
		offs_t offset = 0;
		do {
			populate_passthrough_nomirror(start | offset, end | offset, ostart | offset, oend | offset, handler, mappings);
			offset = (offset + add) & hmirror;
		} while(offset);
	}
}

template<int HighBits, int Width, int AddrShift, endianness_t Endian> void handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::detach(const std::unordered_set<handler_entry *> &handlers)
{
	for(unsigned int i=0; i != COUNT; i++) {
		if(m_u_dispatch[i]->is_dispatch()) {
			m_u_dispatch[i]->detach(handlers);
			continue;
		}

		if(!m_u_dispatch[i]->is_passthrough())
			continue;

		auto np = static_cast<handler_entry_write_passthrough<Width, AddrShift, Endian> *>(m_u_dispatch[i]);

		if(handlers.find(np) != handlers.end()) {
			m_u_dispatch[i] = np->get_subhandler();
			m_u_dispatch[i]->ref();
			np->unref();

		} else
			np->detach(handlers);
	}
}

template<int HighBits, int Width, int AddrShift, endianness_t Endian> void handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::init_handlers(offs_t start_entry, offs_t end_entry, u32 lowbits, handler_entry_write<Width, AddrShift, Endian> **dispatch, handler_entry::range *ranges)
{
	if(!m_view)
		fatalerror("init_handlers called on non-view handler_entry_write_dispatch.");
	if(!m_dispatch_array.empty())
		fatalerror("init_handlers called twice on handler_entry_write_dispatch.");

	m_ranges_array.resize(1);
	m_dispatch_array.resize(1);
	m_a_ranges = m_ranges_array[0].data();
	m_a_dispatch = m_dispatch_array[0].data();
	m_u_ranges = m_ranges_array[0].data();
	m_u_dispatch = m_dispatch_array[0].data();

	auto filter = [s = m_view->m_addrstart, e = m_view->m_addrend] (handler_entry::range r) {
		r.intersect(s, e);
		return r;
	};

	if(lowbits != LowBits) {
		u32 dt = lowbits - LowBits;
		u32 ne = 1 << dt;
		for(offs_t entry = start_entry; entry <= end_entry; entry++) {
			m_u_dispatch[entry]->ref(ne);
			u32 e0 = (entry << dt) & BITMASK;
			for(offs_t e = 0; e != ne; e++) {
				m_u_dispatch[e0 | e] = dispatch[entry];
				m_u_ranges[e0 | e] = filter(ranges[entry]);
			}
		}

	} else {
		for(offs_t entry = start_entry; entry <= end_entry; entry++) {
			m_u_dispatch[entry & BITMASK] = dispatch[entry];
			m_u_ranges[entry & BITMASK] = filter(ranges[entry]);
			dispatch[entry]->ref();
		}
	}
}

template<int HighBits, int Width, int AddrShift, endianness_t Endian> void handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::select_a(int id)
{
	u32 i = id+1;
	if(i >= m_dispatch_array.size())
		fatalerror("out-of-range view selection.");

	m_a_ranges = m_ranges_array[i].data();
	m_a_dispatch = m_dispatch_array[i].data();
}

template<int HighBits, int Width, int AddrShift, endianness_t Endian> void handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::select_u(int id)
{
	u32 i = id+1;
	if(i > m_dispatch_array.size())
		fatalerror("out-of-range view update selection.");
	else if(i == m_dispatch_array.size()) {
		u32 aid = (handler_array *)(m_a_dispatch) - m_dispatch_array.data();

		m_dispatch_array.resize(i+1);
		m_ranges_array.resize(i+1);
		m_a_ranges = m_ranges_array[aid].data();
		m_a_dispatch = m_dispatch_array[aid].data();
		m_u_ranges = m_ranges_array[i].data();
		m_u_dispatch = m_dispatch_array[i].data();

		for(u32 entry = 0; entry != COUNT; entry++) {
			m_u_dispatch[entry] = m_dispatch_array[0][entry]->dup();
			m_u_ranges[entry] = m_ranges_array[0][entry];
		}

	} else {
		m_u_ranges = m_ranges_array[i].data();
		m_u_dispatch = m_dispatch_array[i].data();
	}
}

template<int HighBits, int Width, int AddrShift, endianness_t Endian> handler_entry_write<Width, AddrShift, Endian> *handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::dup()
{
	if(m_view) {
		handler_entry::ref();
		return this;
	}

	return new handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>(this);
}
