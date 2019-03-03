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

	list_t::list_t(log_type &alog)
	: m_log(alog)
	{
	}

	void list_t::register_device(plib::unique_ptr<element_t> &&factory)
	{
		for (auto & e : *this)
			if (e->name() == factory->name())
				m_log.fatal(MF_1_FACTORY_ALREADY_CONTAINS_1, factory->name());
		push_back(std::move(factory));
	}

	factory::element_t * list_t::factory_by_name(const pstring &devname)
	{
		for (auto & e : *this)
		{
			if (e->name() == devname)
				return e.get();
		}

		m_log.fatal(MF_1_CLASS_1_NOT_FOUND, devname);
		return nullptr; // appease code analysis
	}

	// -----------------------------------------------------------------------------
	// factory_lib_entry_t: factory class to wrap macro based chips/elements
	// -----------------------------------------------------------------------------

	poolptr<device_t> library_element_t::Create(netlist_state_t &anetlist, const pstring &name)
	{
		return pool().make_poolptr<NETLIB_NAME(wrapper)>(anetlist, name);
	}

	void library_element_t::macro_actions(nlparse_t &nparser, const pstring &name)
	{
		nparser.namespace_push(name);
		nparser.include(this->name());
		nparser.namespace_pop();
	}


} // namespace factory
 } // namespace netlist
