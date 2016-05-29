// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nl_factory.h
 *
 *
 */

#ifndef NLFACTORY_H_
#define NLFACTORY_H_

#include <type_traits>

#include "nl_config.h"
#include "plib/plists.h"
#include "nl_base.h"

#define NETLIB_DEVICE_IMPL(chip) factory_creator_ptr_t decl_ ## chip = factory_creator_t< NETLIB_NAME(chip) >;

namespace netlist
{

	// -----------------------------------------------------------------------------
	// net_dev class factory
	// -----------------------------------------------------------------------------

	class base_factory_t
	{
		P_PREVENT_COPYING(base_factory_t)
	public:
		base_factory_t(const pstring &name, const pstring &classname,
				const pstring &def_param)
		: m_name(name), m_classname(classname), m_def_param(def_param)
		{}

		virtual ~base_factory_t() {}

		virtual plib::owned_ptr<device_t> Create(netlist_t &anetlist, const pstring &name) = 0;

		const pstring &name() const { return m_name; }
		const pstring &classname() const { return m_classname; }
		const pstring &param_desc() const { return m_def_param; }
		const plib::pstring_vector_t term_param_list();
		const plib::pstring_vector_t def_params();

	protected:
		pstring m_name;                             /* device name */
		pstring m_classname;                        /* device class name */
		pstring m_def_param;                        /* default parameter */
	};

	template <class C>
	class factory_t : public base_factory_t
	{
		P_PREVENT_COPYING(factory_t)
	public:
		factory_t(const pstring &name, const pstring &classname,
				const pstring &def_param)
		: base_factory_t(name, classname, def_param) { }

		plib::owned_ptr<device_t> Create(netlist_t &anetlist, const pstring &name) override
		{
			return plib::owned_ptr<device_t>::Create<C>(anetlist, name);
		}
	};

	class factory_list_t : public plib::pvector_t<plib::owned_ptr<base_factory_t>>
	{
	public:
		factory_list_t(setup_t &m_setup);
		~factory_list_t();

		template<class device_class>
		void register_device(const pstring &name, const pstring &classname,
				const pstring &def_param)
		{
			register_device(plib::owned_ptr<base_factory_t>::Create<factory_t<device_class>>(name, classname, def_param));
		}

		void register_device(plib::owned_ptr<base_factory_t> factory)
		{
			for (auto & e : *this)
				if (e->name() == factory->name())
					error("factory already contains " + factory->name());
			push_back(std::move(factory));
		}

		base_factory_t * factory_by_name(const pstring &devname);

		template <class C>
		bool is_class(base_factory_t *f)
		{
			return dynamic_cast<factory_t<C> *>(f) != nullptr;
		}

	private:
		void error(const pstring &s);

		setup_t &m_setup;
	};

	// -----------------------------------------------------------------------------
	// factory_creator_ptr_t
	// -----------------------------------------------------------------------------

	using factory_creator_ptr_t = plib::owned_ptr<base_factory_t> (*)(const pstring &name, const pstring &classname,
			const pstring &def_param);

	template <typename T>
	plib::owned_ptr<base_factory_t> factory_creator_t(const pstring &name, const pstring &classname,
			const pstring &def_param)
	{
		return plib::owned_ptr<base_factory_t>::Create<factory_t<T>>(name, classname, def_param);
	}

}

#endif /* NLFACTORY_H_ */
