// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_EMU_EMUMEM_HEDR_IPP
#define MAME_EMU_EMUMEM_HEDR_IPP

#pragma once

#include "emumem_mud.h"
#include "emumem_hea.h"
#include "emumem_heu.h"
#include "emumem_heun.h"
#include "emumem_hep.h"
#include "emumem_hedr.h"

template<int HighBits, int Width, int AddrShift> const handler_entry_read<Width, AddrShift> *const *handler_entry_read_dispatch<HighBits, Width, AddrShift>::get_dispatch() const
{
	return m_a_dispatch;
}

template<int HighBits, int Width, int AddrShift> handler_entry_read_dispatch<HighBits, Width, AddrShift>::handler_entry_read_dispatch(address_space *space, const handler_entry::range &init, handler_entry_read<Width, AddrShift> *handler) : handler_entry_read<Width, AddrShift>(space, handler_entry::F_DISPATCH), m_view(nullptr)
{
	m_ranges_array.resize(1);
	m_dispatch_array.resize(1);
	m_a_ranges = m_ranges_array[0].data();
	m_a_dispatch = m_dispatch_array[0].data();
	m_u_ranges = m_ranges_array[0].data();
	m_u_dispatch = m_dispatch_array[0].data();

	if (!handler)
		handler = space->get_unmap_r<Width, AddrShift>();
	handler->ref(COUNT);
	for(unsigned int i=0; i != COUNT; i++) {
		m_u_dispatch[i] = handler;
		m_u_ranges[i] = init;
	}
}

template<int HighBits, int Width, int AddrShift> handler_entry_read_dispatch<HighBits, Width, AddrShift>::handler_entry_read_dispatch(address_space *space, memory_view &view) : handler_entry_read<Width, AddrShift>(space, handler_entry::F_VIEW), m_view(&view), m_a_dispatch(nullptr), m_a_ranges(nullptr), m_u_dispatch(nullptr), m_u_ranges(nullptr)
{
	m_ranges_array.resize(1);
	m_dispatch_array.resize(1);
	m_a_ranges = m_ranges_array[0].data();
	m_a_dispatch = m_dispatch_array[0].data();
	m_u_ranges = m_ranges_array[0].data();
	m_u_dispatch = m_dispatch_array[0].data();

	auto handler = space->get_unmap_r<Width, AddrShift>();
	handler->ref(COUNT);
	for(unsigned int i=0; i != COUNT; i++) {
		m_u_dispatch[i] = handler;
		m_u_ranges[i].set(0, 0);
	}
}

template<int HighBits, int Width, int AddrShift> handler_entry_read_dispatch<HighBits, Width, AddrShift>::handler_entry_read_dispatch(handler_entry_read_dispatch<HighBits, Width, AddrShift> *src) : handler_entry_read<Width, AddrShift>(src->m_space, handler_entry::F_DISPATCH), m_view(nullptr)
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


template<int HighBits, int Width, int AddrShift> handler_entry_read_dispatch<HighBits, Width, AddrShift>::~handler_entry_read_dispatch()
{
	for(auto &d : m_dispatch_array)
		for(auto p : d)
			if(p)
				p->unref();
}

template<int HighBits, int Width, int AddrShift> void handler_entry_read_dispatch<HighBits, Width, AddrShift>::enumerate_references(handler_entry::reflist &refs) const
{
	for(auto &d : m_dispatch_array)
		for(auto p : d)
			if(p)
				refs.add(p);
}

template<int HighBits, int Width, int AddrShift> offs_t handler_entry_read_dispatch<HighBits, Width, AddrShift>::dispatch_entry(offs_t address) const
{
	return (address & HIGHMASK) >> LowBits;
}

template<int HighBits, int Width, int AddrShift> void handler_entry_read_dispatch<HighBits, Width, AddrShift>::dump_map(std::vector<memory_entry> &map) const
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

template<int HighBits, int Width, int AddrShift> typename emu::detail::handler_entry_size<Width>::uX handler_entry_read_dispatch<HighBits, Width, AddrShift>::read(offs_t offset, uX mem_mask) const
{
	return dispatch_read<Level, Width, AddrShift>(HIGHMASK, offset, mem_mask, m_a_dispatch);
}

template<int HighBits, int Width, int AddrShift> std::pair<typename emu::detail::handler_entry_size<Width>::uX, u16> handler_entry_read_dispatch<HighBits, Width, AddrShift>::read_flags(offs_t offset, uX mem_mask) const
{
	return dispatch_read_flags<Level, Width, AddrShift>(HIGHMASK, offset, mem_mask, m_a_dispatch);
}

template<int HighBits, int Width, int AddrShift> void *handler_entry_read_dispatch<HighBits, Width, AddrShift>::get_ptr(offs_t offset) const
{
	return m_a_dispatch[(offset & HIGHMASK) >> LowBits]->get_ptr(offset);
}

template<int HighBits, int Width, int AddrShift> std::string handler_entry_read_dispatch<HighBits, Width, AddrShift>::name() const
{
	return m_view ? "view" : "dispatch";
}

template<int HighBits, int Width, int AddrShift> void handler_entry_read_dispatch<HighBits, Width, AddrShift>::lookup(offs_t address, offs_t &start, offs_t &end, handler_entry_read<Width, AddrShift> *&handler) const
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

template<int HighBits, int Width, int AddrShift> void handler_entry_read_dispatch<HighBits, Width, AddrShift>::range_cut_before(offs_t address, int start)
{
	while(--start >= 0 && m_u_dispatch[start]) {
		if(int(LowBits) > -AddrShift && m_u_dispatch[start]->is_dispatch()) {
			static_cast<handler_entry_read_dispatch<LowBits, Width, AddrShift> *>(m_u_dispatch[start])->range_cut_before(address);
			break;
		}
		if(m_u_ranges[start].end <= address)
			break;
		m_u_ranges[start].end = address;
	}
}

template<int HighBits, int Width, int AddrShift> void handler_entry_read_dispatch<HighBits, Width, AddrShift>::range_cut_after(offs_t address, int start)
{
	while(++start < COUNT && m_u_dispatch[start]) {
		if(int(LowBits) > -AddrShift && m_u_dispatch[start]->is_dispatch()) {
			static_cast<handler_entry_read_dispatch<LowBits, Width, AddrShift> *>(m_u_dispatch[start])->range_cut_after(address);
			break;
		}
		if(m_u_ranges[start].start >= address)
			break;
		m_u_ranges[start].start = address;
	}
}

template<int HighBits, int Width, int AddrShift> void handler_entry_read_dispatch<HighBits, Width, AddrShift>::populate_nomirror_subdispatch(offs_t entry, offs_t start, offs_t end, offs_t ostart, offs_t oend, handler_entry_read<Width, AddrShift> *handler)
{
	auto cur = m_u_dispatch[entry];
	if(cur->is_dispatch())
		cur->populate_nomirror(start, end, ostart, oend, handler);
	else {
		auto subdispatch = new handler_entry_read_dispatch<LowBits, Width, AddrShift>(this->m_space, m_u_ranges[entry], cur);
		cur->unref();
		m_u_dispatch[entry] = subdispatch;
		subdispatch->populate_nomirror(start, end, ostart, oend, handler);
	}
}

template<int HighBits, int Width, int AddrShift> void handler_entry_read_dispatch<HighBits, Width, AddrShift>::populate_nomirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, handler_entry_read<Width, AddrShift> *handler)
{
	offs_t start_entry = (start & HIGHMASK) >> LowBits;
	offs_t end_entry = (end & HIGHMASK) >> LowBits;
	range_cut_before(ostart-1, start_entry);
	range_cut_after(oend+1, end_entry);

	if constexpr(LowBits <= Width + AddrShift) {
		if(handler->is_view()) {
			int delta = dispatch_entry(ostart) - handler->dispatch_entry(ostart);
			handler->init_handlers(start >> LowBits, end >> LowBits, LowBits, ostart, oend, m_u_dispatch + delta, m_u_ranges + delta);
		}
		handler->ref(end_entry - start_entry);
		for(offs_t ent = start_entry; ent <= end_entry; ent++) {
			m_u_dispatch[ent]->unref();
			m_u_dispatch[ent] = handler;
			m_u_ranges[ent].set(ostart, oend);
		}

	} else if(start_entry == end_entry) {
		if(!(start & LOWMASK) && (end & LOWMASK) == LOWMASK) {
			if(handler->is_view()) {
				int delta = dispatch_entry(ostart) - handler->dispatch_entry(ostart);
				handler->init_handlers(start >> LowBits, end >> LowBits, LowBits, ostart, oend, m_u_dispatch + delta, m_u_ranges + delta);
			}
			m_u_dispatch[start_entry]->unref();
			m_u_dispatch[start_entry] = handler;
			m_u_ranges[start_entry].set(ostart, oend);
		} else
			populate_nomirror_subdispatch(start_entry, start, end, ostart, oend, handler);

	} else {
		if(start & LOWMASK) {
			populate_nomirror_subdispatch(start_entry, start, start | LOWMASK, ostart, oend, handler);
			start_entry++;
			start = (start | LOWMASK) + 1;
			if(start_entry <= end_entry)
				handler->ref();
		}
		if((end & LOWMASK) != LOWMASK) {
			populate_nomirror_subdispatch(end_entry, end & ~LOWMASK, end, ostart, oend, handler);
			end_entry--;
			end = (end & ~LOWMASK) - 1;
			if(start_entry <= end_entry)
				handler->ref();
		}

		if(start_entry <= end_entry) {
			if(handler->is_view()) {
				int delta = dispatch_entry(ostart) - handler->dispatch_entry(ostart);
				handler->init_handlers(start >> LowBits, end >> LowBits, LowBits, ostart, oend, m_u_dispatch + delta, m_u_ranges + delta);
			}
			handler->ref(end_entry - start_entry);
			for(offs_t ent = start_entry; ent <= end_entry; ent++) {
				m_u_dispatch[ent]->unref();
				m_u_dispatch[ent] = handler;
				m_u_ranges[ent].set(ostart, oend);
			}
		}
	}
}

template<int HighBits, int Width, int AddrShift> void handler_entry_read_dispatch<HighBits, Width, AddrShift>::populate_mirror_subdispatch(offs_t entry, offs_t start, offs_t end, offs_t ostart, offs_t oend, offs_t mirror, handler_entry_read<Width, AddrShift> *handler)
{
	auto cur = m_u_dispatch[entry];
	if(cur->is_dispatch())
		cur->populate_mirror(start, end, ostart, oend, mirror, handler);
	else {
		auto subdispatch = new handler_entry_read_dispatch<LowBits, Width, AddrShift>(this->m_space, m_u_ranges[entry], cur);
		cur->unref();
		m_u_dispatch[entry] = subdispatch;
		subdispatch->populate_mirror(start, end, ostart, oend, mirror, handler);
	}
}

template<int HighBits, int Width, int AddrShift> void handler_entry_read_dispatch<HighBits, Width, AddrShift>::populate_mirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, offs_t mirror, handler_entry_read<Width, AddrShift> *handler)
{
	offs_t hmirror = mirror & HIGHMASK;
	offs_t lmirror = mirror & LOWMASK;

	if(lmirror) {
		// If lmirror is non-zero, then each mirror instance is a single entry
		offs_t add = 1 + ~hmirror;
		offs_t offset = 0;
		offs_t base_entry = start >> LowBits;
		do {
			if(offset)
				handler->ref();
			populate_mirror_subdispatch(base_entry | (offset >> LowBits), start | offset, end | offset, ostart | offset, oend | offset, lmirror, handler);
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

template<int HighBits, int Width, int AddrShift> void handler_entry_read_dispatch<HighBits, Width, AddrShift>::mismatched_patch(const memory_units_descriptor<Width, AddrShift> &descriptor, u8 rkey, std::vector<mapping> &mappings, handler_entry_read<Width, AddrShift> *&target)
{
	u8 ukey = descriptor.rkey_to_ukey(rkey);
	handler_entry_read<Width, AddrShift> *original = target->is_units() ? target : nullptr;
	handler_entry_read<Width, AddrShift> *replacement = nullptr;
	for(const auto &p : mappings)
		if(p.ukey == ukey && p.original == original) {
			replacement = p.patched;
			break;
		}
	if(!replacement) {
		if(original)
			replacement = new handler_entry_read_units<Width, AddrShift>(descriptor, ukey, static_cast<handler_entry_read_units<Width, AddrShift> *>(original));
		else
			replacement = new handler_entry_read_units<Width, AddrShift>(descriptor, ukey, this->m_space);

		mappings.emplace_back(mapping{ original, replacement, ukey });
	} else
		replacement->ref();
	target->unref();
	target = replacement;
}

template<int HighBits, int Width, int AddrShift> void handler_entry_read_dispatch<HighBits, Width, AddrShift>::populate_mismatched_nomirror_subdispatch(offs_t entry, offs_t start, offs_t end, offs_t ostart, offs_t oend, const memory_units_descriptor<Width, AddrShift> &descriptor, u8 rkey, std::vector<mapping> &mappings)
{
	auto cur = m_u_dispatch[entry];
	if(cur->is_dispatch())
		cur->populate_mismatched_nomirror(start, end, ostart, oend, descriptor, rkey, mappings);
	else {
		auto subdispatch = new handler_entry_read_dispatch<LowBits, Width, AddrShift>(this->m_space, m_u_ranges[entry], cur);
		cur->unref();
		m_u_dispatch[entry] = subdispatch;
		subdispatch->populate_mismatched_nomirror(start, end, ostart, oend, descriptor, rkey, mappings);
	}
}

template<int HighBits, int Width, int AddrShift> void handler_entry_read_dispatch<HighBits, Width, AddrShift>::populate_mismatched_nomirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, const memory_units_descriptor<Width, AddrShift> &descriptor, u8 rkey, std::vector<mapping> &mappings)
{
	offs_t start_entry = (start & HIGHMASK) >> LowBits;
	offs_t end_entry = (end & HIGHMASK) >> LowBits;
	range_cut_before(ostart-1, start_entry);
	range_cut_after(oend+1, end_entry);

	if constexpr(LowBits <= Width + AddrShift) {
		for(offs_t ent = start_entry; ent <= end_entry; ent++) {
			u8 rkey1 = rkey;
			if(ent != start_entry)
				rkey1 &= ~handler_entry::START;
			if(ent != end_entry)
				rkey1 &= ~handler_entry::END;
			mismatched_patch(descriptor, rkey1, mappings, m_u_dispatch[ent]);
			m_u_ranges[ent].set(ostart, oend);
		}

	} else if(start_entry == end_entry) {
		if(!(start & LOWMASK) && (end & LOWMASK) == LOWMASK) {
			if(m_u_dispatch[start_entry]->is_dispatch())
				m_u_dispatch[start_entry]->populate_mismatched_nomirror(start, end, ostart, oend, descriptor, rkey, mappings);
			else {
				mismatched_patch(descriptor, rkey, mappings, m_u_dispatch[start_entry]);
				m_u_ranges[start_entry].set(ostart, oend);
			}
		} else
			populate_mismatched_nomirror_subdispatch(start_entry, start, end, ostart, oend, descriptor, rkey, mappings);

	} else {
		if(start & LOWMASK) {
			populate_mismatched_nomirror_subdispatch(start_entry, start, start | LOWMASK, ostart, oend, descriptor, rkey & ~handler_entry::END, mappings);
			start_entry++;
			rkey &= ~handler_entry::START;
		}
		if((end & LOWMASK) != LOWMASK) {
			populate_mismatched_nomirror_subdispatch(end_entry, end & ~LOWMASK, end, ostart, oend, descriptor, rkey & ~handler_entry::START, mappings);
			end_entry--;
			rkey &= ~handler_entry::END;
		}

		offs_t base = start & ~LOWMASK;
		for(offs_t ent = start_entry; ent <= end_entry; ent++) {
			u8 rkey1 = rkey;
			if(ent != start_entry)
				rkey1 &= ~handler_entry::START;
			if(ent != end_entry)
				rkey1 &= ~handler_entry::END;
			if(m_u_dispatch[ent]->is_dispatch())
				m_u_dispatch[ent]->populate_mismatched_nomirror(base | (ent << LowBits), base | (ent << LowBits) | LOWMASK, ostart, oend, descriptor, rkey1, mappings);
			else {
				mismatched_patch(descriptor, rkey1, mappings, m_u_dispatch[ent]);
				m_u_ranges[ent].set(ostart, oend);
			}
		}
	}
}

template<int HighBits, int Width, int AddrShift> void handler_entry_read_dispatch<HighBits, Width, AddrShift>::populate_mismatched_mirror_subdispatch(offs_t entry, offs_t start, offs_t end, offs_t ostart, offs_t oend, offs_t mirror, const memory_units_descriptor<Width, AddrShift> &descriptor, std::vector<mapping> &mappings)
{
	auto cur = m_u_dispatch[entry];
	if(cur->is_dispatch())
		cur->populate_mismatched_mirror(start, end, ostart, oend, mirror, descriptor, mappings);
	else {
		auto subdispatch = new handler_entry_read_dispatch<LowBits, Width, AddrShift>(this->m_space, m_u_ranges[entry], cur);
		cur->unref();
		m_u_dispatch[entry] = subdispatch;
		subdispatch->populate_mismatched_mirror(start, end, ostart, oend, mirror, descriptor, mappings);
	}
}

template<int HighBits, int Width, int AddrShift> void handler_entry_read_dispatch<HighBits, Width, AddrShift>::populate_mismatched_mirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, offs_t mirror, const memory_units_descriptor<Width, AddrShift> &descriptor, std::vector<mapping> &mappings)
{
	offs_t hmirror = mirror & HIGHMASK;
	offs_t lmirror = mirror & LOWMASK;

	if(lmirror) {
		// If lmirror is non-zero, then each mirror instance is a single entry
		offs_t add = 1 + ~hmirror;
		offs_t offset = 0;
		offs_t base_entry = start >> LowBits;
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


template<int HighBits, int Width, int AddrShift> void handler_entry_read_dispatch<HighBits, Width, AddrShift>::passthrough_patch(handler_entry_read_passthrough<Width, AddrShift> *handler, std::vector<mapping> &mappings, handler_entry_read<Width, AddrShift> *&target)
{
	handler_entry_read<Width, AddrShift> *original = target;
	handler_entry_read<Width, AddrShift> *replacement = nullptr;
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

template<int HighBits, int Width, int AddrShift> void handler_entry_read_dispatch<HighBits, Width, AddrShift>::populate_passthrough_nomirror_subdispatch(offs_t entry, offs_t start, offs_t end, offs_t ostart, offs_t oend, handler_entry_read_passthrough<Width, AddrShift> *handler, std::vector<mapping> &mappings)
{
	auto cur = m_u_dispatch[entry];
	if(cur->is_dispatch())
		cur->populate_passthrough_nomirror(start, end, ostart, oend, handler, mappings);
	else {
		auto subdispatch = new handler_entry_read_dispatch<LowBits, Width, AddrShift>(this->m_space, m_u_ranges[entry], cur);
		cur->unref();
		m_u_dispatch[entry] = subdispatch;
		subdispatch->populate_passthrough_nomirror(start, end, ostart, oend, handler, mappings);
	}
}

template<int HighBits, int Width, int AddrShift> void handler_entry_read_dispatch<HighBits, Width, AddrShift>::populate_passthrough_nomirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, handler_entry_read_passthrough<Width, AddrShift> *handler, std::vector<mapping> &mappings)
{
	offs_t start_entry = (start & HIGHMASK) >> LowBits;
	offs_t end_entry = (end & HIGHMASK) >> LowBits;
	range_cut_before(ostart-1, start_entry);
	range_cut_after(oend+1, end_entry);

	if constexpr(LowBits <= Width + AddrShift) {
		for(offs_t ent = start_entry; ent <= end_entry; ent++) {
			passthrough_patch(handler, mappings, m_u_dispatch[ent]);
			m_u_ranges[ent].intersect(ostart, oend);
		}

	} else if(start_entry == end_entry) {
		if(!(start & LOWMASK) && (end & LOWMASK) == LOWMASK) {
			if(m_u_dispatch[start_entry]->is_dispatch())
				m_u_dispatch[start_entry]->populate_passthrough_nomirror(start, end, ostart, oend, handler, mappings);
			else {
				passthrough_patch(handler, mappings, m_u_dispatch[start_entry]);
				m_u_ranges[start_entry].intersect(ostart, oend);
			}
		} else
			populate_passthrough_nomirror_subdispatch(start_entry, start, end, ostart, oend, handler, mappings);

	} else {
		if(start & LOWMASK) {
			populate_passthrough_nomirror_subdispatch(start_entry, start, start | LOWMASK, ostart, oend, handler, mappings);
			start_entry++;
		}
		if((end & LOWMASK) != LOWMASK) {
			populate_passthrough_nomirror_subdispatch(end_entry, end & ~LOWMASK, end, ostart, oend, handler, mappings);
			end_entry--;
		}

		offs_t base = start & ~LOWMASK;
		for(offs_t ent = start_entry; ent <= end_entry; ent++) {
			if(m_u_dispatch[ent]->is_dispatch())
				m_u_dispatch[ent]->populate_passthrough_nomirror(base | (ent << LowBits), base | (ent << LowBits) | LOWMASK, ostart, oend, handler, mappings);
			else {
				passthrough_patch(handler, mappings, m_u_dispatch[ent]);
				m_u_ranges[ent].intersect(ostart, oend);
			}
		}
	}
}

template<int HighBits, int Width, int AddrShift> void handler_entry_read_dispatch<HighBits, Width, AddrShift>::populate_passthrough_mirror_subdispatch(offs_t entry, offs_t start, offs_t end, offs_t ostart, offs_t oend, offs_t mirror, handler_entry_read_passthrough<Width, AddrShift> *handler, std::vector<mapping> &mappings)
{
	auto cur = m_u_dispatch[entry];
	if(cur->is_dispatch())
		cur->populate_passthrough_mirror(start, end, ostart, oend, mirror, handler, mappings);
	else {
		auto subdispatch = new handler_entry_read_dispatch<LowBits, Width, AddrShift>(this->m_space, m_u_ranges[entry], cur);
		cur->unref();
		m_u_dispatch[entry] = subdispatch;
		subdispatch->populate_passthrough_mirror(start, end, ostart, oend, mirror, handler, mappings);
	}
}

template<int HighBits, int Width, int AddrShift> void handler_entry_read_dispatch<HighBits, Width, AddrShift>::populate_passthrough_mirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, offs_t mirror, handler_entry_read_passthrough<Width, AddrShift> *handler, std::vector<mapping> &mappings)
{
	offs_t hmirror = mirror & HIGHMASK;
	offs_t lmirror = mirror & LOWMASK;

	if(lmirror) {
		// If lmirror is non-zero, then each mirror instance is a single entry
		offs_t add = 1 + ~hmirror;
		offs_t offset = 0;
		offs_t base_entry = start >> LowBits;
		do {
			populate_passthrough_mirror_subdispatch(base_entry | (offset >> LowBits), start | offset, end | offset, ostart | offset, oend | offset, lmirror, handler, mappings);
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

template<int HighBits, int Width, int AddrShift> void handler_entry_read_dispatch<HighBits, Width, AddrShift>::detach(const std::unordered_set<handler_entry *> &handlers)
{
	for(unsigned int i=0; i != COUNT; i++) {
		if(m_u_dispatch[i]->is_dispatch()) {
			m_u_dispatch[i]->detach(handlers);
			continue;
		}

		if(!m_u_dispatch[i]->is_passthrough())
			continue;

		auto np = static_cast<handler_entry_read_passthrough<Width, AddrShift> *>(m_u_dispatch[i]);

		if(handlers.find(np) != handlers.end()) {
			m_u_dispatch[i] = np->get_subhandler();
			m_u_dispatch[i]->ref();
			np->unref();

		} else
			np->detach(handlers);
	}
}

template<int HighBits, int Width, int AddrShift> void handler_entry_read_dispatch<HighBits, Width, AddrShift>::init_handlers(offs_t start_entry, offs_t end_entry, u32 lowbits, offs_t ostart, offs_t oend, handler_entry_read<Width, AddrShift> **dispatch, handler_entry::range *ranges)
{
	if(lowbits < LowBits) {
		offs_t entry = start_entry >> LowBits;
		if(entry != (end_entry >> LowBits))
		   fatalerror("Recursive init_handlers spanning multiple entries.\n");
		entry &= BITMASK;
		handler_entry_read_dispatch<LowBits, Width, AddrShift> *subdispatch;
		if(m_u_dispatch[entry]->flags() & handler_entry::F_DISPATCH)
			subdispatch = static_cast<handler_entry_read_dispatch<LowBits, Width, AddrShift> *>(m_u_dispatch[entry]);

		else if(!(m_u_dispatch[entry]->flags() & handler_entry::F_UNMAP))
			fatalerror("Collision on multiple init_handlers calls");

		else {
			m_u_dispatch[entry]->unref();
			m_u_dispatch[entry] = subdispatch = new handler_entry_read_dispatch<LowBits, Width, AddrShift>(this->m_space, m_u_ranges[entry], nullptr);
		}
		int delta = dispatch_entry(ostart) - subdispatch->dispatch_entry(ostart);
		subdispatch->init_handlers(start_entry, end_entry, lowbits, ostart, oend, dispatch + delta, ranges + delta);

	} else if(lowbits != LowBits) {
		u32 dt = lowbits - LowBits;
		u32 ne = 1 << dt;
		u32 ee = end_entry - start_entry;
		if(m_view) {
			auto filter = [s = m_view->m_addrstart, e = m_view->m_addrend] (handler_entry::range r) { r.intersect(s, e); return r; };

			for(offs_t entry = 0; entry <= ee; entry++) {
				dispatch[entry]->ref(ne);
				u32 e0 = (entry << dt) & BITMASK;
				for(offs_t e = 0; e != ne; e++) {
					offs_t e1 = e0 | e;
					if(!(m_u_dispatch[e1]->flags() & handler_entry::F_UNMAP))
						fatalerror("Collision on multiple init_handlers calls");
					m_u_dispatch[e1]->unref();
					m_u_dispatch[e1] = dispatch[entry];
					m_u_ranges[e1] = filter(ranges[entry]);
				}
			}
		} else {
			for(offs_t entry = 0; entry <= ee; entry++) {
				dispatch[entry]->ref(ne);
				u32 e0 = (entry << dt) & BITMASK;
				for(offs_t e = 0; e != ne; e++) {
					offs_t e1 = e0 | e;
					if(!(m_u_dispatch[e1]->flags() & handler_entry::F_UNMAP))
						fatalerror("Collision on multiple init_handlers calls");
					m_u_dispatch[e1]->unref();
					m_u_dispatch[e1] = dispatch[entry];
					m_u_ranges[e1] = ranges[entry];
				}
			}
		}

	} else {
		if(m_view) {
			auto filter = [s = m_view->m_addrstart, e = m_view->m_addrend] (handler_entry::range r) { r.intersect(s, e); return r; };

			for(offs_t entry = start_entry & BITMASK; entry <= (end_entry & BITMASK); entry++) {
				if(!(m_u_dispatch[entry]->flags() & handler_entry::F_UNMAP))
					fatalerror("Collision on multiple init_handlers calls");
				m_u_dispatch[entry]->unref();
				m_u_dispatch[entry] = dispatch[entry];
				m_u_ranges[entry] = filter(ranges[entry]);
				dispatch[entry]->ref();
			}
		} else {
			for(offs_t entry = start_entry & BITMASK; entry <= (end_entry & BITMASK); entry++) {
				if(!(m_u_dispatch[entry]->flags() & handler_entry::F_UNMAP))
					fatalerror("Collision on multiple init_handlers calls");
				m_u_dispatch[entry]->unref();
				m_u_dispatch[entry] = dispatch[entry];
				m_u_ranges[entry] = ranges[entry];
				dispatch[entry]->ref();
			}
		}
	}
}

template<int HighBits, int Width, int AddrShift> void handler_entry_read_dispatch<HighBits, Width, AddrShift>::select_a(int id)
{
	u32 i = id+1;
	if(i >= m_dispatch_array.size())
		fatalerror("out-of-range view selection.");

	m_a_ranges = m_ranges_array[i].data();
	m_a_dispatch = m_dispatch_array[i].data();
}

template<int HighBits, int Width, int AddrShift> void handler_entry_read_dispatch<HighBits, Width, AddrShift>::select_u(int id)
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

		for(u32 entry = 0; entry != COUNT; entry++)
			if(m_dispatch_array[0][entry]) {
				m_u_dispatch[entry] = m_dispatch_array[0][entry]->dup();
				m_u_ranges[entry] = m_ranges_array[0][entry];
			}

	} else {
		m_u_ranges = m_ranges_array[i].data();
		m_u_dispatch = m_dispatch_array[i].data();
	}
}

template<int HighBits, int Width, int AddrShift> handler_entry_read<Width, AddrShift> *handler_entry_read_dispatch<HighBits, Width, AddrShift>::dup()
{
	if(m_view) {
		handler_entry::ref();
		return this;
	}

	return new handler_entry_read_dispatch<HighBits, Width, AddrShift>(this);
}

#endif // MAME_EMU_EMUMEM_HEDR_IPP
