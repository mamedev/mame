// license:GPL-2.0+
// copyright-holders:Couriersud
/***************************************************************************

    nl_factory.c

    Discrete netlist implementation.

****************************************************************************/

#include "nl_factory.h"
#include "nl_base.h"
#include "nl_errstr.h"
#include "nl_setup.h"
#include "plib/putil.h"

namespace netlist { namespace factory
{

	class NETLIB_NAME(wrapper) : public device_t
	{
	public:
		NETLIB_NAME(wrapper)(netlist_state_t &anetlist, const pstring &name)
		: device_t(anetlist, name)
		{
		}
	protected:
		NETLIB_RESETI() { }
		NETLIB_UPDATEI() { }
	};

	element_t::element_t(const pstring &name, const pstring &classname,
			const pstring &def_param, const pstring &sourcefile)
		: m_name(name), m_classname(classname), m_def_param(def_param),
		  m_sourcefile(sourcefile)
	{
	}

	element_t::element_t(const pstring &name, const pstring &classname,
			const pstring &def_param)
		: m_name(name), m_classname(classname), m_def_param(def_param),
		  m_sourcefile("<unknown>")
	{
	}

	// ----------------------------------------------------------------------------------------
	// net_device_t_base_factory
	// ----------------------------------------------------------------------------------------

	list_t::list_t( setup_t &setup)
	: m_setup(setup)
	{
	}

	list_t::~list_t()
	{
		clear();
	}

	void list_t::register_device(std::unique_ptr<element_t> &&factory)
	{
		for (auto & e : *this)
			if (e->name() == factory->name())
				m_setup.log().fatal(MF_1_FACTORY_ALREADY_CONTAINS_1, factory->name());
		push_back(std::move(factory));
	}

	factory::element_t * list_t::factory_by_name(const pstring &devname)
	{
		for (auto & e : *this)
		{
			if (e->name() == devname)
				return e.get();
		}

		m_setup.log().fatal(MF_1_CLASS_1_NOT_FOUND, devname);
		return nullptr; // appease code analysis
	}

	// -----------------------------------------------------------------------------
	// factory_lib_entry_t: factory class to wrap macro based chips/elements
	// -----------------------------------------------------------------------------

	plib::owned_ptr<device_t> library_element_t::Create(netlist_state_t &anetlist, const pstring &name)
	{
		return plib::owned_ptr<device_t>::Create<NETLIB_NAME(wrapper)>(anetlist, name);
	}

	void library_element_t::macro_actions(netlist_state_t &anetlist, const pstring &name)
	{
		anetlist.setup().namespace_push(name);
		anetlist.setup().include(this->name());
		anetlist.setup().namespace_pop();
	}


} // namespace factory
 } // namespace netlist
