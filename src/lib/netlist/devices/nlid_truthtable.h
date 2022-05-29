// license:BSD-3-Clause
// copyright-holders:Couriersud

///
/// \file nld_truthtable.h
///
///

#ifndef NLID_TRUTHTABLE_H_
#define NLID_TRUTHTABLE_H_

#include "../nl_setup.h"
#include "nl_factory.h"

#define USE_TT_ALTERNATIVE (0)

namespace netlist::factory {

	class truthtable_base_element_t : public factory::element_t
	{
	public:
		truthtable_base_element_t(const pstring &name,properties &&props);

		std::vector<pstring> m_desc;
		pstring m_family_name;
	};

	host_arena::unique_ptr<truthtable_base_element_t> truthtable_create(tt_desc &desc,
		properties &&props);

} // namespace netlist::factory

#endif // NLID_TRUTHTABLE_H_
