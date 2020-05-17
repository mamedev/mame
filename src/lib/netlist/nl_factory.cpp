// license:GPL-2.0+
// copyright-holders:Couriersud

//
// nl_factory.cpp
//

#include "nl_factory.h"
#include "nl_base.h"
#include "nl_errstr.h"
#include "nl_setup.h"
#include "plib/putil.h"

namespace netlist {
namespace factory {

	// FIXME: this doesn't do anything, check how to remove
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

	element_t::element_t(const pstring &name, const pstring &def_param,
		plib::source_location &&sourceloc)
		: m_name(name), m_def_param(def_param),
		  m_sourceloc(sourceloc)
	{
	}

	element_t::element_t(const pstring &name, const pstring &def_param)
		: m_name(name), m_def_param(def_param),
		  m_sourceloc("<unknown>", 1)
	{
	}

	// ----------------------------------------------------------------------------------------
	// net_device_t_base_factory
	// ----------------------------------------------------------------------------------------

	list_t::list_t(log_type &alog)
	: m_log(alog)
	{
	}

	void list_t::add(plib::unique_ptr<element_t> &&factory)
	{
		for (auto & e : *this)
			if (e->name() == factory->name())
			{
				m_log.fatal(MF_FACTORY_ALREADY_CONTAINS_1(factory->name()));
				throw nl_exception(MF_FACTORY_ALREADY_CONTAINS_1(factory->name()));
			}
		push_back(std::move(factory));
	}

	factory::element_t * list_t::factory_by_name(const pstring &devname)
	{
		for (auto & e : *this)
		{
			if (e->name() == devname)
				return e.get();
		}

		m_log.fatal(MF_CLASS_1_NOT_FOUND(devname));
		throw nl_exception(MF_CLASS_1_NOT_FOUND(devname));
	}

	// -----------------------------------------------------------------------------
	// factory_lib_entry_t: factory class to wrap macro based chips/elements
	// -----------------------------------------------------------------------------

	unique_pool_ptr<core_device_t> library_element_t::make_device(nlmempool &pool, netlist_state_t &anetlist, const pstring &name)
	{
		return pool.make_unique<NETLIB_NAME(wrapper)>(anetlist, name);
	}

	void library_element_t::macro_actions(nlparse_t &nparser, const pstring &name)
	{
		nparser.namespace_push(name);
		nparser.include(this->name());
		nparser.namespace_pop();
	}


} // namespace factory
 } // namespace netlist
