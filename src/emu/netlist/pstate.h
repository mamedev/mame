/*
 * pstate.h
 *
 */

#ifndef PSTATE_H_
#define PSTATE_H_

#include "nl_config.h"
#include "nl_time.h"
#include "nl_lists.h"
#include "pstring.h"

// ----------------------------------------------------------------------------------------
// state saving ...
// ----------------------------------------------------------------------------------------

#define PSTATE_INTERFACE_DECL()               \
	template<typename C> ATTR_COLD void save(C &state, const pstring &stname);

#define PSTATE_INTERFACE(obj, manager, module)               \
	template<typename C> ATTR_COLD void obj::save(C &state, const pstring &stname) \
	{                                                                       \
		manager->save_item(state, this, module + "." + stname);  \
	}

enum pstate_data_type_e {
	NOT_SUPPORTED,
	DT_CUSTOM,
	DT_DOUBLE,
	DT_INT64,
	DT_INT16,
	DT_INT8,
	DT_INT,
	DT_BOOLEAN
};

template<typename _ItemType> struct nl_datatype { static const pstate_data_type_e type = pstate_data_type_e(NOT_SUPPORTED); };
//template<typename _ItemType> struct type_checker<_ItemType*> { static const bool is_atom = false; static const bool is_pointer = true; };

#define NETLIST_SAVE_TYPE(TYPE, TYPEDESC) template<> struct nl_datatype<TYPE>{ static const pstate_data_type_e type = pstate_data_type_e(TYPEDESC); }

NETLIST_SAVE_TYPE(char, DT_INT8);
NETLIST_SAVE_TYPE(double, DT_DOUBLE);
NETLIST_SAVE_TYPE(INT8, DT_INT8);
NETLIST_SAVE_TYPE(UINT8, DT_INT8);
NETLIST_SAVE_TYPE(INT64, DT_INT64);
NETLIST_SAVE_TYPE(UINT64, DT_INT64);
NETLIST_SAVE_TYPE(bool, DT_BOOLEAN);
NETLIST_SAVE_TYPE(UINT32, DT_INT);
NETLIST_SAVE_TYPE(INT32, DT_INT);
NETLIST_SAVE_TYPE(UINT16, DT_INT16);
NETLIST_SAVE_TYPE(INT16, DT_INT16);
//NETLIST_SAVE_TYPE(netlist_time::INTERNALTYPE, DT_INT64);

class pstate_manager_t;

class pstate_callback_t
{
public:
	typedef netlist_list_t<pstate_callback_t *> list_t;

	virtual ~pstate_callback_t() { };

	virtual void register_state(pstate_manager_t &manager, const pstring &module) = 0;
	virtual void on_pre_save() = 0;
	virtual void on_post_load() = 0;
protected:
};

struct pstate_entry_t
{
	typedef netlist_list_t<pstate_entry_t *> list_t;

	pstate_entry_t(const pstring &stname, const pstate_data_type_e dt, const void *owner,
			const int size, const int count, void *ptr)
	: m_name(stname), m_dt(dt), m_owner(owner), m_callback(NULL), m_size(size), m_count(count), m_ptr(ptr) { }

	pstate_entry_t(const pstring &stname, const void *owner, pstate_callback_t *callback)
	: m_name(stname), m_dt(DT_CUSTOM), m_owner(owner), m_callback(callback), m_size(0), m_count(0), m_ptr(NULL) { }

	pstring m_name;
	const pstate_data_type_e m_dt;
	const void *m_owner;
	pstate_callback_t *m_callback;
	const int m_size;
	const int m_count;
	void *m_ptr;
};

class pstate_manager_t
{
public:

	ATTR_COLD ~pstate_manager_t();

	template<typename C> ATTR_COLD void save_item(C &state, const void *owner, const pstring &stname)
	{
		save_state_ptr(stname, nl_datatype<C>::type, owner, sizeof(C), 1, &state);
	}

	template<typename C, std::size_t N> ATTR_COLD void save_item(C (&state)[N], const void *owner, const pstring &stname)
	{
		save_state_ptr(stname, nl_datatype<C>::type, owner, sizeof(state[0]), N, &(state[0]));
	}

	template<typename C> ATTR_COLD void save_item(C *state, const void *owner, const pstring &stname, const int count)
	{
		save_state_ptr(stname, nl_datatype<C>::type, owner, sizeof(C), count, state);
	}

	ATTR_COLD void pre_save();
	ATTR_COLD void post_load();
	ATTR_COLD void remove_save_items(const void *owner);

	inline const pstate_entry_t::list_t &save_list() const { return m_save; }

protected:
	ATTR_COLD void save_state_ptr(const pstring &stname, const pstate_data_type_e, const void *owner, const int size, const int count, void *ptr);

private:
	pstate_entry_t::list_t m_save;
};

template<> ATTR_COLD inline void pstate_manager_t::save_item(pstate_callback_t &state, const void *owner, const pstring &stname)
{
	//save_state_ptr(stname, DT_CUSTOM, 0, 1, &state);
	pstate_entry_t *p = new pstate_entry_t(stname, owner, &state);
	m_save.add(p);
	state.register_state(*this, stname);
}

template<> ATTR_COLD inline void pstate_manager_t::save_item(netlist_time &nlt, const void *owner, const pstring &stname)
{
	save_state_ptr(stname, DT_INT64, owner, sizeof(netlist_time::INTERNALTYPE), 1, nlt.get_internaltype_ptr());
}



#endif /* PSTATE_H_ */
