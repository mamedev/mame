// license:BSD-3-Clause
// copyright-holders:Couriersud

///
/// \file nl_factory.cpp
///

#include "nl_errstr.h"
#include "nl_factory.h"
#include "nl_setup.h"

#include "core/core_device.h"

#include "plib/putil.h"

namespace netlist::factory {

	// FIXME: this doesn't do anything, check how to remove
	class NETLIB_NAME(wrapper) : public base_device_t
	{
	public:
		NETLIB_NAME(wrapper)(netlist_state_t &anetlist, const pstring &name)
		: base_device_t(anetlist, name)
		{
		}
	protected:
		//NETLIB_RESETI() {}
	};

	element_t::element_t(const pstring &name, properties &&props)
	: m_name(name)
	, m_properties(props)
	{
	}

	// ----------------------------------------------------------------------------------------
	// net_device_t_base_factory
	// ----------------------------------------------------------------------------------------

	list_t::list_t(log_type &alog)
	: m_log(alog)
	{
	}

	bool exists(const pstring &name);

	bool list_t::exists(const pstring &name) const noexcept
	{
		for (const auto & e : *this)
			if (e->name() == name)
				return true;
		return false;
	}

	void list_t::add(host_arena::unique_ptr<element_t> &&factory)
	{
		if (exists(factory->name()))
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
	// library_element_t: factory class to wrap macro based chips/elements
	// -----------------------------------------------------------------------------

	library_element_t::library_element_t(const pstring &name, properties &&props)
	: element_t(name, std::move(properties(props).set_type(element_type::MACRO)))
	{
	}

	device_arena::unique_ptr<core_device_t> library_element_t::make_device(device_arena &pool, netlist_state_t &anetlist, const pstring &name)
	{
		return plib::make_unique<NETLIB_NAME(wrapper)>(pool, anetlist, name);
	}


} // namespace netlist::factory
