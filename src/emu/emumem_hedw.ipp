// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "emumem_mud.h"
#include "emumem_hea.h"
#include "emumem_heu.h"
#include "emumem_heun.h"
#include "emumem_hep.h"
#include "emumem_hedw.h"


template<int HighBits, int Width, int AddrShift, int Endian> handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::handler_entry_write_dispatch(address_space *space, const handler_entry::range &init, handler_entry_write<Width, AddrShift, Endian> *handler) : handler_entry_write<Width, AddrShift, Endian>(space, handler_entry::F_DISPATCH)
{
	if (!handler)
		handler = space->get_unmap_w<Width, AddrShift, Endian>();
	handler->ref(COUNT);
	for(unsigned int i=0; i != COUNT; i++) {
		m_dispatch[i] = handler;
		m_ranges[i] = init;
	}
}

template<int HighBits, int Width, int AddrShift, int Endian> handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::~handler_entry_write_dispatch()
{
	for(unsigned int i=0; i != COUNT; i++)
		m_dispatch[i]->unref();
}

template<int HighBits, int Width, int AddrShift, int Endian> void handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::enumerate_references(handler_entry::reflist &refs) const
{
	for(unsigned int i=0; i != COUNT; i++)
		refs.add(m_dispatch[i]);
}

template<int HighBits, int Width, int AddrShift, int Endian> void handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::dump_map(std::vector<memory_entry> &map) const
{
	offs_t cur = map.empty() ? 0 : map.back().end + 1;
	offs_t base = cur & UPMASK;
	do {
		offs_t entry = (cur >> LowBits) & BITMASK;
		if(m_dispatch[entry]->is_dispatch())
			m_dispatch[entry]->dump_map(map);
		else
			map.emplace_back(memory_entry{ m_ranges[entry].start, m_ranges[entry].end, m_dispatch[entry] });
		cur = map.back().end + 1;
	} while(cur && !((cur ^ base) & UPMASK));
}

template<int HighBits, int Width, int AddrShift, int Endian> void handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::write(offs_t offset, uX data, uX mem_mask)
{
	m_dispatch[(offset >> LowBits) & BITMASK]->write(offset, data, mem_mask);
}

template<int HighBits, int Width, int AddrShift, int Endian> void *handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::get_ptr(offs_t offset) const
{
	return m_dispatch[(offset >> LowBits) & BITMASK]->get_ptr(offset);
}

template<int HighBits, int Width, int AddrShift, int Endian> std::string handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::name() const
{
	return "dispatch";
}

template<int HighBits, int Width, int AddrShift, int Endian> void handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::lookup(offs_t address, offs_t &start, offs_t &end, handler_entry_write<Width, AddrShift, Endian> *&handler) const
{
	offs_t slot = (address >> LowBits) & BITMASK;
	auto h = m_dispatch[slot];
	if(h->is_dispatch())
		h->lookup(address, start, end, handler);
	else {
		start = m_ranges[slot].start;
		end = m_ranges[slot].end;
		handler = h;
	}
}

template<int HighBits, int Width, int AddrShift, int Endian> void handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::range_cut_before(offs_t address, int start)
{
	while(--start >= 0) {
		if(int(LowBits) > -AddrShift && m_dispatch[start]->is_dispatch()) {
			static_cast<handler_entry_write_dispatch<LowBits, Width, AddrShift, Endian> *>(m_dispatch[start])->range_cut_before(address);
			break;
		}
		if(m_ranges[start].end <= address)
			break;
		m_ranges[start].end = address;
	}
}

template<int HighBits, int Width, int AddrShift, int Endian> void handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::range_cut_after(offs_t address, int start)
{
	while(++start < COUNT) {
		if(int(LowBits) > -AddrShift && m_dispatch[start]->is_dispatch()) {
			static_cast<handler_entry_write_dispatch<LowBits, Width, AddrShift, Endian> *>(m_dispatch[start])->range_cut_after(address);
			break;
		}
		if(m_ranges[start].start >= address)
			break;
		m_ranges[start].start = address;
	}
}

template<int HighBits, int Width, int AddrShift, int Endian> void handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::populate_nomirror_subdispatch(offs_t entry, offs_t start, offs_t end, offs_t ostart, offs_t oend, handler_entry_write<Width, AddrShift, Endian> *handler)
{
	auto cur = m_dispatch[entry];
	if(cur->is_dispatch())
		cur->populate_nomirror(start, end, ostart, oend, handler);
	else {
		auto subdispatch = new handler_entry_write_dispatch<LowBits, Width, AddrShift, Endian>(handler_entry::m_space, m_ranges[entry], cur);
		cur->unref();
		m_dispatch[entry] = subdispatch;
		subdispatch->populate_nomirror(start, end, ostart, oend, handler);
	}
}

template<int HighBits, int Width, int AddrShift, int Endian> void handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::populate_nomirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, handler_entry_write<Width, AddrShift, Endian> *handler)
{
	offs_t start_entry = (start & HIGHMASK) >> LowBits;
	offs_t end_entry = (end & HIGHMASK) >> LowBits;
	range_cut_before(ostart-1, start_entry);
	range_cut_after(oend+1, end_entry);

	if(LowBits <= Width + AddrShift) {
		handler->ref(end_entry - start_entry);
		for(offs_t ent = start_entry; ent <= end_entry; ent++) {
			m_dispatch[ent]->unref();
			m_dispatch[ent] = handler;
			m_ranges[ent].set(ostart, oend);
		}

	} else if(start_entry == end_entry) {
		if(!(start & LOWMASK) && (end & LOWMASK) == LOWMASK) {
			m_dispatch[start_entry]->unref();
			m_dispatch[start_entry] = handler;
			m_ranges[start_entry].set(ostart, oend);
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
			handler->ref(end_entry - start_entry);
			for(offs_t ent = start_entry; ent <= end_entry; ent++) {
				m_dispatch[ent]->unref();
				m_dispatch[ent] = handler;
				m_ranges[ent].set(ostart, oend);
			}
		}
	}
}

template<int HighBits, int Width, int AddrShift, int Endian> void handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::populate_mirror_subdispatch(offs_t entry, offs_t start, offs_t end, offs_t ostart, offs_t oend, offs_t mirror, handler_entry_write<Width, AddrShift, Endian> *handler)
{
	auto cur = m_dispatch[entry];
	if(cur->is_dispatch())
		cur->populate_mirror(start, end, ostart, oend, mirror, handler);
	else {
		auto subdispatch = new handler_entry_write_dispatch<LowBits, Width, AddrShift, Endian>(handler_entry::m_space, m_ranges[entry], cur);
		cur->unref();
		m_dispatch[entry] = subdispatch;
		subdispatch->populate_mirror(start, end, ostart, oend, mirror, handler);
	}
}

template<int HighBits, int Width, int AddrShift, int Endian> void handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::populate_mirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, offs_t mirror, handler_entry_write<Width, AddrShift, Endian> *handler)
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

template<int HighBits, int Width, int AddrShift, int Endian> void handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::mismatched_patch(const memory_units_descriptor<Width, AddrShift, Endian> &descriptor, u8 rkey, std::vector<mapping> &mappings, handler_entry_write<Width, AddrShift, Endian> *&target)
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

template<int HighBits, int Width, int AddrShift, int Endian> void handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::populate_mismatched_nomirror_subdispatch(offs_t entry, offs_t start, offs_t end, offs_t ostart, offs_t oend, const memory_units_descriptor<Width, AddrShift, Endian> &descriptor, u8 rkey, std::vector<mapping> &mappings)
{
	auto cur = m_dispatch[entry];
	if(cur->is_dispatch())
		cur->populate_mismatched_nomirror(start, end, ostart, oend, descriptor, rkey, mappings);
	else {
		auto subdispatch = new handler_entry_write_dispatch<LowBits, Width, AddrShift, Endian>(handler_entry::m_space, m_ranges[entry], cur);
		cur->unref();
		m_dispatch[entry] = subdispatch;
		subdispatch->populate_mismatched_nomirror(start, end, ostart, oend, descriptor, rkey, mappings);
	}
}

template<int HighBits, int Width, int AddrShift, int Endian> void handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::populate_mismatched_nomirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, const memory_units_descriptor<Width, AddrShift, Endian> &descriptor, u8 rkey, std::vector<mapping> &mappings)
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
			mismatched_patch(descriptor, rkey1, mappings, m_dispatch[ent]);
			m_ranges[ent].intersect(ostart, oend);
		}

	} else if(start_entry == end_entry) {
		if(!(start & LOWMASK) && (end & LOWMASK) == LOWMASK) {
			if(m_dispatch[start_entry]->is_dispatch())
				m_dispatch[start_entry]->populate_mismatched_nomirror(start & LOWMASK, end & LOWMASK, ostart, oend, descriptor, rkey, mappings);
			else {
				mismatched_patch(descriptor, rkey, mappings, m_dispatch[start_entry]);
				m_ranges[start_entry].intersect(ostart, oend);
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
				if(m_dispatch[ent]->is_dispatch())
					m_dispatch[ent]->populate_mismatched_nomirror(start & LOWMASK, end & LOWMASK, ostart, oend, descriptor, rkey1, mappings);
				else {
					mismatched_patch(descriptor, rkey1, mappings, m_dispatch[ent]);
					m_ranges[ent].intersect(ostart, oend);
				}
			}
		}
	}
}

template<int HighBits, int Width, int AddrShift, int Endian> void handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::populate_mismatched_mirror_subdispatch(offs_t entry, offs_t start, offs_t end, offs_t ostart, offs_t oend, offs_t mirror, const memory_units_descriptor<Width, AddrShift, Endian> &descriptor, std::vector<mapping> &mappings)
{
	auto cur = m_dispatch[entry];
	if(cur->is_dispatch())
		cur->populate_mismatched_mirror(start, end, ostart, oend, mirror, descriptor, mappings);
	else {
		auto subdispatch = new handler_entry_write_dispatch<LowBits, Width, AddrShift, Endian>(handler_entry::m_space, m_ranges[entry], cur);
		cur->unref();
		m_dispatch[entry] = subdispatch;
		subdispatch->populate_mismatched_mirror(start, end, ostart, oend, mirror, descriptor, mappings);
	}
}

template<int HighBits, int Width, int AddrShift, int Endian> void handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::populate_mismatched_mirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, offs_t mirror, const memory_units_descriptor<Width, AddrShift, Endian> &descriptor, std::vector<mapping> &mappings)
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


template<int HighBits, int Width, int AddrShift, int Endian> void handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::passthrough_patch(handler_entry_write_passthrough<Width, AddrShift, Endian> *handler, std::vector<mapping> &mappings, handler_entry_write<Width, AddrShift, Endian> *&target)
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

template<int HighBits, int Width, int AddrShift, int Endian> void handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::populate_passthrough_nomirror_subdispatch(offs_t entry, offs_t start, offs_t end, offs_t ostart, offs_t oend, handler_entry_write_passthrough<Width, AddrShift, Endian> *handler, std::vector<mapping> &mappings)
{
	auto cur = m_dispatch[entry];
	if(cur->is_dispatch())
		cur->populate_passthrough_nomirror(start, end, ostart, oend, handler, mappings);
	else {
		auto subdispatch = new handler_entry_write_dispatch<LowBits, Width, AddrShift, Endian>(handler_entry::m_space, m_ranges[entry], cur);
		cur->unref();
		m_dispatch[entry] = subdispatch;
		subdispatch->populate_passthrough_nomirror(start, end, ostart, oend, handler, mappings);
	}
}

template<int HighBits, int Width, int AddrShift, int Endian> void handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::populate_passthrough_nomirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, handler_entry_write_passthrough<Width, AddrShift, Endian> *handler, std::vector<mapping> &mappings)
{
	offs_t start_entry = (start & HIGHMASK) >> LowBits;
	offs_t end_entry = (end & HIGHMASK) >> LowBits;
	range_cut_before(ostart-1, start_entry);
	range_cut_after(oend+1, end_entry);

	if(LowBits <= Width + AddrShift) {
		for(offs_t ent = start_entry; ent <= end_entry; ent++) {
			passthrough_patch(handler, mappings, m_dispatch[ent]);
			m_ranges[ent].intersect(ostart, oend);
		}

	} else if(start_entry == end_entry) {
		if(!(start & LOWMASK) && (end & LOWMASK) == LOWMASK) {
			if(m_dispatch[start_entry]->is_dispatch())
				m_dispatch[start_entry]->populate_passthrough_nomirror(start & LOWMASK, end & LOWMASK, ostart, oend, handler, mappings);
			else {
				passthrough_patch(handler, mappings, m_dispatch[start_entry]);
				m_ranges[start_entry].intersect(ostart, oend);
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
				if(m_dispatch[ent]->is_dispatch())
					m_dispatch[ent]->populate_passthrough_nomirror(start & LOWMASK, end & LOWMASK, ostart, oend, handler, mappings);
				else {
					passthrough_patch(handler, mappings, m_dispatch[ent]);
					m_ranges[ent].intersect(ostart, oend);
				}
			}
		}
	}
}

template<int HighBits, int Width, int AddrShift, int Endian> void handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::populate_passthrough_mirror_subdispatch(offs_t entry, offs_t start, offs_t end, offs_t ostart, offs_t oend, offs_t mirror, handler_entry_write_passthrough<Width, AddrShift, Endian> *handler, std::vector<mapping> &mappings)
{
	auto cur = m_dispatch[entry];
	if(cur->is_dispatch())
		cur->populate_passthrough_mirror(start, end, ostart, oend, mirror, handler, mappings);
	else {
		auto subdispatch = new handler_entry_write_dispatch<LowBits, Width, AddrShift, Endian>(handler_entry::m_space, m_ranges[entry], cur);
		cur->unref();
		m_dispatch[entry] = subdispatch;
		subdispatch->populate_passthrough_mirror(start, end, ostart, oend, mirror, handler, mappings);
	}
}

template<int HighBits, int Width, int AddrShift, int Endian> void handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::populate_passthrough_mirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, offs_t mirror, handler_entry_write_passthrough<Width, AddrShift, Endian> *handler, std::vector<mapping> &mappings)
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

template<int HighBits, int Width, int AddrShift, int Endian> void handler_entry_write_dispatch<HighBits, Width, AddrShift, Endian>::detach(const std::unordered_set<handler_entry *> &handlers)
{
	for(unsigned int i=0; i != COUNT; i++) {
		if(m_dispatch[i]->is_dispatch()) {
			m_dispatch[i]->detach(handlers);
			continue;
		}

		if(!m_dispatch[i]->is_passthrough())
			continue;

		auto np = static_cast<handler_entry_write_passthrough<Width, AddrShift, Endian> *>(m_dispatch[i]);

		if(handlers.find(np) != handlers.end()) {
			m_dispatch[i] = np->get_subhandler();
			m_dispatch[i]->ref();
			np->unref();

		} else
			np->detach(handlers);
	}
}
