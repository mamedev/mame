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
#include "palloc.h"
#include "plists.h"
#include "nl_base.h"
#include "pstring.h"

// -----------------------------------------------------------------------------
// net_dev class factory
// -----------------------------------------------------------------------------

class net_device_t_base_factory
{
	NETLIST_PREVENT_COPYING(net_device_t_base_factory)
public:
	ATTR_COLD net_device_t_base_factory(const pstring &name, const pstring &classname,
			const pstring &def_param)
	: m_name(name), m_classname(classname), m_def_param(def_param)
	{}

	/* ATTR_COLD */ virtual ~net_device_t_base_factory() {}

	/* ATTR_COLD */ virtual netlist_device_t *Create() const = 0;

	ATTR_COLD const pstring &name() const { return m_name; }
	ATTR_COLD const pstring &classname() const { return m_classname; }
	ATTR_COLD const pstring &param_desc() const { return m_def_param; }
	ATTR_COLD const nl_util::pstring_list term_param_list();
	ATTR_COLD const nl_util::pstring_list def_params();

protected:
	pstring m_name;                             /* device name */
	pstring m_classname;                        /* device class name */
	pstring m_def_param;                        /* default parameter */
};

template <class C>
class net_device_t_factory : public net_device_t_base_factory
{
	NETLIST_PREVENT_COPYING(net_device_t_factory)
public:
	ATTR_COLD net_device_t_factory(const pstring &name, const pstring &classname,
			const pstring &def_param)
	: net_device_t_base_factory(name, classname, def_param) { }

	ATTR_COLD netlist_device_t *Create() const
	{
		netlist_device_t *r = palloc(C);
		//r->init(setup, name);
		return r;
	}
};

class netlist_factory_t
{
public:
	typedef plist_t<net_device_t_base_factory *> list_t;

	netlist_factory_t();
	~netlist_factory_t();

	template<class _C>
	ATTR_COLD void register_device(const pstring &name, const pstring &classname,
			const pstring &def_param)
	{
		m_list.add(palloc(net_device_t_factory< _C >, name, classname, def_param));
	}

	ATTR_COLD netlist_device_t *new_device_by_classname(const pstring &classname) const;
	ATTR_COLD netlist_device_t *new_device_by_name(const pstring &name, netlist_setup_t &setup) const;
	ATTR_COLD net_device_t_base_factory * factory_by_name(const pstring &name, netlist_setup_t &setup) const;

	const list_t &list() { return m_list; }

private:
	list_t m_list;

};


#endif /* NLFACTORY_H_ */
