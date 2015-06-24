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
		NETLIST_PREVENT_COPYING(base_factory_t)
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
		ATTR_COLD const pstring_list_t term_param_list();
		ATTR_COLD const pstring_list_t def_params();

	protected:
		pstring m_name;                             /* device name */
		pstring m_classname;                        /* device class name */
		pstring m_def_param;                        /* default parameter */
	};

	template <class C>
	class factory_t : public base_factory_t
	{
		NETLIST_PREVENT_COPYING(factory_t)
	public:
		ATTR_COLD factory_t(const pstring &name, const pstring &classname,
				const pstring &def_param)
		: base_factory_t(name, classname, def_param) { }

		ATTR_COLD device_t *Create()
		{
			device_t *r = palloc(C);
			//r->init(setup, name);
			return r;
		}
	};

	class factory_list_t
	{
	public:
		typedef plist_t<base_factory_t *> list_t;

		factory_list_t();
		~factory_list_t();

		template<class _C>
		ATTR_COLD void register_device(const pstring &name, const pstring &classname,
				const pstring &def_param)
		{
			m_list.add(palloc(factory_t< _C >, name, classname, def_param));
		}

		ATTR_COLD void register_device(base_factory_t *factory)
		{
			m_list.add(factory);
		}

		ATTR_COLD device_t *new_device_by_classname(const pstring &classname) const;
		ATTR_COLD device_t *new_device_by_name(const pstring &name, setup_t &setup) const;
		ATTR_COLD base_factory_t * factory_by_name(const pstring &name, setup_t &setup) const;

		const list_t &list() { return m_list; }

	private:
		list_t m_list;

	};

}

#endif /* NLFACTORY_H_ */
