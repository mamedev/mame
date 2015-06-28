// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * pstate.h
 *
 */

#ifndef PSTATE_H_
#define PSTATE_H_

#include "plists.h"
#include "pstring.h"

// ----------------------------------------------------------------------------------------
// state saving ...
// ----------------------------------------------------------------------------------------

#define PSTATE_INTERFACE_DECL()               \
	template<typename C> ATTR_COLD void save(C &state, const pstring &stname); \
	template<typename C, std::size_t N> ATTR_COLD void save(C (&state)[N], const pstring &stname); \
	template<typename C> ATTR_COLD void save(C *state, const pstring &stname, const int count);

#define PSTATE_INTERFACE(obj, manager, module)               \
	template<typename C> ATTR_COLD void obj::save(C &state, const pstring &stname) \
	{ manager->save_item(state, this, module + "." + stname); } \
	template<typename C, std::size_t N> ATTR_COLD void obj::save(C (&state)[N], const pstring &stname) \
	{ manager->save_state_ptr(module + "." + stname, pstate_datatype<C>::type, this, sizeof(state[0]), N, &(state[0]), false); } \
	template<typename C> ATTR_COLD void obj::save(C *state, const pstring &stname, const int count) \
	{ manager->save_state_ptr(module + "." + stname, pstate_datatype<C>::type, this, sizeof(C), count, state, false);   }

enum pstate_data_type_e {
	NOT_SUPPORTED,
	DT_CUSTOM,
	DT_DOUBLE,
	DT_INT64,
	DT_INT16,
	DT_INT8,
	DT_INT,
	DT_BOOLEAN,
	DT_FLOAT
};

template<typename _ItemType> struct pstate_datatype
{
	static const pstate_data_type_e type = pstate_data_type_e(NOT_SUPPORTED);
	static const bool is_ptr = false;
};

template<typename _ItemType> struct pstate_datatype<_ItemType *>
{
	static const pstate_data_type_e type = pstate_data_type_e(NOT_SUPPORTED);
	static const bool is_ptr = true;
};

//template<typename _ItemType> struct type_checker<_ItemType*> { static const bool is_atom = false; static const bool is_pointer = true; };

#define NETLIST_SAVE_TYPE(TYPE, TYPEDESC) \
		template<> struct pstate_datatype<TYPE>{ static const pstate_data_type_e type = pstate_data_type_e(TYPEDESC); static const bool is_ptr = false;}; \
		template<> struct pstate_datatype<TYPE *>{ static const pstate_data_type_e type = pstate_data_type_e(TYPEDESC); static const bool is_ptr = true;}

NETLIST_SAVE_TYPE(char, DT_INT8);
NETLIST_SAVE_TYPE(double, DT_DOUBLE);
NETLIST_SAVE_TYPE(float, DT_FLOAT);
NETLIST_SAVE_TYPE(INT8, DT_INT8);
NETLIST_SAVE_TYPE(UINT8, DT_INT8);
NETLIST_SAVE_TYPE(INT64, DT_INT64);
NETLIST_SAVE_TYPE(UINT64, DT_INT64);
NETLIST_SAVE_TYPE(bool, DT_BOOLEAN);
NETLIST_SAVE_TYPE(UINT32, DT_INT);
NETLIST_SAVE_TYPE(INT32, DT_INT);
NETLIST_SAVE_TYPE(UINT16, DT_INT16);
NETLIST_SAVE_TYPE(INT16, DT_INT16);
//NETLIST_SAVE_TYPE(std::size_t, DT_INT64);

class pstate_manager_t;

class pstate_callback_t
{
public:
	typedef plist_t<pstate_callback_t *> list_t;

	virtual ~pstate_callback_t() { };

	virtual void register_state(pstate_manager_t &manager, const pstring &module) = 0;
	virtual void on_pre_save() = 0;
	virtual void on_post_load() = 0;
protected:
};

struct pstate_entry_t
{
	typedef plist_t<pstate_entry_t *> list_t;

	pstate_entry_t(const pstring &stname, const pstate_data_type_e dt, const void *owner,
			const int size, const int count, void *ptr, bool is_ptr)
	: m_name(stname), m_dt(dt), m_owner(owner), m_callback(NULL), m_size(size), m_count(count), m_ptr(ptr), m_is_ptr(is_ptr) { }

	pstate_entry_t(const pstring &stname, const void *owner, pstate_callback_t *callback)
	: m_name(stname), m_dt(DT_CUSTOM), m_owner(owner), m_callback(callback), m_size(0), m_count(0), m_ptr(NULL), m_is_ptr(false) { }

	~pstate_entry_t() { }

	pstring m_name;
	const pstate_data_type_e m_dt;
	const void *m_owner;
	pstate_callback_t *m_callback;
	const int m_size;
	const int m_count;
	void *m_ptr;
	bool m_is_ptr;

	template<typename T>
	T *resolved()
	{
		if (m_is_ptr)
			return *static_cast<T **>(m_ptr);
		else
			return static_cast<T *>(m_ptr);
	}
};

class pstate_manager_t
{
public:

	pstate_manager_t();
	~pstate_manager_t();

	template<typename C> ATTR_COLD void save_item(C &state, const void *owner, const pstring &stname)
	{
		save_state_ptr(stname, pstate_datatype<C>::type, owner, sizeof(C), 1, &state, pstate_datatype<C>::is_ptr);
	}

	template<typename C, std::size_t N> ATTR_COLD void save_item(C (&state)[N], const void *owner, const pstring &stname)
	{
		save_state_ptr(stname, pstate_datatype<C>::type, owner, sizeof(state[0]), N, &(state[0]), false);
	}

	template<typename C> ATTR_COLD void save_item(C *state, const void *owner, const pstring &stname, const int count)
	{
		save_state_ptr(stname, pstate_datatype<C>::type, owner, sizeof(C), count, state, false);
	}

	ATTR_COLD void pre_save();
	ATTR_COLD void post_load();
	ATTR_COLD void remove_save_items(const void *owner);

	const pstate_entry_t::list_t &save_list() const { return m_save; }

	ATTR_COLD void save_state_ptr(const pstring &stname, const pstate_data_type_e, const void *owner, const int size, const int count, void *ptr, bool is_ptr);

protected:

private:
	pstate_entry_t::list_t m_save;
};

template<> ATTR_COLD void pstate_manager_t::save_item(pstate_callback_t &state, const void *owner, const pstring &stname);


#endif /* PSTATE_H_ */
