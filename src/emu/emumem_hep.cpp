// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "emumem_hep.h"

template<int Width, int AddrShift> handler_entry_read_passthrough<Width, AddrShift>::~handler_entry_read_passthrough()
{
	if(m_next) {
		m_mph.remove_handler(this);
		m_next->unref();
	}
}

template<int Width, int AddrShift> void handler_entry_read_passthrough<Width, AddrShift>::detach(const std::unordered_set<handler_entry *> &handlers)
{
	if(!m_next->is_passthrough())
		return;
	auto np = static_cast<handler_entry_read_passthrough<Width, AddrShift> *>(m_next);

	if(handlers.find(np) != handlers.end()) {
		m_next = np->get_subhandler();
		m_next->ref();
		np->unref();

	} else
		np->detach(handlers);
}

template<int Width, int AddrShift> handler_entry_write_passthrough<Width, AddrShift>::~handler_entry_write_passthrough()
{
	if(m_next) {
		m_mph.remove_handler(this);
		m_next->unref();
	}
}

template<int Width, int AddrShift> void handler_entry_write_passthrough<Width, AddrShift>::detach(const std::unordered_set<handler_entry *> &handlers)
{
	if(!m_next->is_passthrough())
		return;
	auto np = static_cast<handler_entry_write_passthrough<Width, AddrShift> *>(m_next);

	if(handlers.find(np) != handlers.end()) {
		m_next = np->get_subhandler();
		m_next->ref();
		np->unref();

	} else
		np->detach(handlers);
}

template class handler_entry_read_passthrough<0,  1>;
template class handler_entry_read_passthrough<0,  0>;
template class handler_entry_read_passthrough<1,  3>;
template class handler_entry_read_passthrough<1,  0>;
template class handler_entry_read_passthrough<1, -1>;
template class handler_entry_read_passthrough<2,  3>;
template class handler_entry_read_passthrough<2,  0>;
template class handler_entry_read_passthrough<2, -1>;
template class handler_entry_read_passthrough<2, -2>;
template class handler_entry_read_passthrough<3,  0>;
template class handler_entry_read_passthrough<3, -1>;
template class handler_entry_read_passthrough<3, -2>;
template class handler_entry_read_passthrough<3, -3>;

template class handler_entry_write_passthrough<0,  1>;
template class handler_entry_write_passthrough<0,  0>;
template class handler_entry_write_passthrough<1,  3>;
template class handler_entry_write_passthrough<1,  0>;
template class handler_entry_write_passthrough<1, -1>;
template class handler_entry_write_passthrough<2,  3>;
template class handler_entry_write_passthrough<2,  0>;
template class handler_entry_write_passthrough<2, -1>;
template class handler_entry_write_passthrough<2, -2>;
template class handler_entry_write_passthrough<3,  0>;
template class handler_entry_write_passthrough<3, -1>;
template class handler_entry_write_passthrough<3, -2>;
template class handler_entry_write_passthrough<3, -3>;
