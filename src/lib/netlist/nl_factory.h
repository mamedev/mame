// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nl_factory.h
 *
 *
 */

#ifndef NLFACTORY_H_
#define NLFACTORY_H_

#include "nl_config.h"
#include "plib/palloc.h"
#include "plib/plists.h"
#include "nl_base.h"

namespace netlist
{
	// -----------------------------------------------------------------------------
	// net_dev class factory
	// -----------------------------------------------------------------------------

	class base_factory_t
	{
		P_PREVENT_COPYING(base_factory_t)
	public:
		ATTR_COLD base_factory_t(const pstring &name, const pstring &classname,
				const pstring &def_param)
		: m_name(name), m_classname(classname), m_def_param(def_param)
		{}

		virtual ~base_factory_t() {}

		virtual device_t *Create() = 0;

		ATTR_COLD const pstring &name() const { return m_name; }
		ATTR_COLD const pstring &classname() const { return m_classname; }
		ATTR_COLD const pstring &param_desc() const { return m_def_param; }
		ATTR_COLD const pstring_vector_t term_param_list();
		ATTR_COLD const pstring_vector_t def_params();

	protected:
		pstring m_name;                             /* device name */
		pstring m_classname;                        /* device class name */
		pstring m_def_param;                        /* default parameter */
	};

	template <class _device_class>
	class factory_t : public base_factory_t
	{
		P_PREVENT_COPYING(factory_t)
	public:
		ATTR_COLD factory_t(const pstring &name, const pstring &classname,
				const pstring &def_param)
		: base_factory_t(name, classname, def_param) { }

		ATTR_COLD device_t *Create() override
		{
			device_t *r = palloc(_device_class);
			//r->init(setup, name);
			return r;
		}
	};

	class factory_list_t : public phashmap_t<pstring, base_factory_t *>
	{
	public:
		factory_list_t(setup_t &m_setup);
		~factory_list_t();

		template<class _device_class>
		ATTR_COLD void register_device(const pstring &name, const pstring &classname,
				const pstring &def_param)
		{
			if (!add(name, palloc(factory_t< _device_class >(name, classname, def_param))))
				error("factory already contains " + name);
		}

		ATTR_COLD void register_device(base_factory_t *factory)
		{
			if (!add(factory->name(), factory))
				error("factory already contains " + factory->name());
		}

		//ATTR_COLD device_t *new_device_by_classname(const pstring &classname) const;
		ATTR_COLD device_t *new_device_by_name(const pstring &name);
		ATTR_COLD base_factory_t * factory_by_name(const pstring &name);

	private:
		void error(const pstring &s);

		setup_t &m_setup;
	};

}

#endif /* NLFACTORY_H_ */
