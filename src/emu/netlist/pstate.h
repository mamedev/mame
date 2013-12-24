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

#define PSTATE_INTERFACE(manager, module)               \
	template<class C> ATTR_COLD void save(C &state, const pstring &stname) \
	{                                                                       \
		dynamic_cast<pstate_manager_t &>(manager).save_manager(state, module + "." + stname);  \
	}

enum netlist_data_type_e {
	NOT_SUPPORTED,
	DT_DOUBLE,
	DT_INT64,
	DT_INT8,
	DT_INT,
	DT_BOOLEAN
};

template<typename _ItemType> struct nl_datatype { static const netlist_data_type_e type = netlist_data_type_e(NOT_SUPPORTED); };
//template<typename _ItemType> struct type_checker<_ItemType*> { static const bool is_atom = false; static const bool is_pointer = true; };

#define NETLIST_SAVE_TYPE(TYPE, TYPEDESC) template<> struct nl_datatype<TYPE>{ static const netlist_data_type_e type = netlist_data_type_e(TYPEDESC); }

NETLIST_SAVE_TYPE(double, DT_DOUBLE);
NETLIST_SAVE_TYPE(INT8, DT_INT8);
NETLIST_SAVE_TYPE(UINT8, DT_INT8);
NETLIST_SAVE_TYPE(INT64, DT_INT64);
NETLIST_SAVE_TYPE(UINT64, DT_INT64);
NETLIST_SAVE_TYPE(bool, DT_BOOLEAN);
NETLIST_SAVE_TYPE(UINT32, DT_INT);
NETLIST_SAVE_TYPE(INT32, DT_INT);

struct pstate_entry_t
{
	typedef netlist_list_t<pstate_entry_t *> list_t;

	pstate_entry_t(const pstring &stname, const netlist_data_type_e dt, const int size, const int count, void *ptr) :
		m_name(stname), m_dt(dt), m_size(size), m_count(count), m_ptr(ptr) { }
	pstring m_name;
	netlist_data_type_e m_dt;
	int m_size;
	int m_count;
	void *m_ptr;
};

class pstate_manager_t
{
public:

	ATTR_COLD ~pstate_manager_t();

	template<class C> ATTR_COLD void save_manager(C &state, const pstring &stname)
	{
		save_state_ptr(stname, nl_datatype<C>::type, sizeof(C), 1, &state);
	}

	template<class C> ATTR_COLD void save_manager(C *state, const pstring &stname, const int count)
	{
		save_state_ptr(stname, nl_datatype<C>::type, sizeof(C), count, state);
	}

	template<class C, std::size_t N> ATTR_COLD void save_manager(C (&state)[N], const pstring &stname)
	{
		save_state_ptr(stname, nl_datatype<C>::type, sizeof(C), N, &(state[0]));
	}

	inline const pstate_entry_t::list_t &save_list() const { return m_save; }

protected:
	ATTR_COLD void save_state_ptr(const pstring &stname, const netlist_data_type_e, const int size, const int count, void *ptr);

private:
	pstate_entry_t::list_t m_save;
};

template<> ATTR_COLD inline void pstate_manager_t::save_manager(netlist_time &nlt, const pstring &stname)
{
	save_state_ptr(stname, DT_INT64, sizeof(netlist_time::INTERNALTYPE), 1, nlt.get_internaltype_ptr());
}



#endif /* PSTATE_H_ */
