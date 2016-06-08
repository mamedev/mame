// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * pstate.h
 *
 */

#ifndef PSTATE_H_
#define PSTATE_H_

#include <memory>

#include "plists.h"
#include "pstring.h"

// ----------------------------------------------------------------------------------------
// state saving ...
// ----------------------------------------------------------------------------------------

enum pstate_data_type_e {
	NOT_SUPPORTED,
	DT_CUSTOM,
	DT_DOUBLE,
#if (PHAS_INT128)
	DT_INT128,
#endif
	DT_INT64,
	DT_INT16,
	DT_INT8,
	DT_INT,
	DT_BOOLEAN,
	DT_FLOAT
};

template<typename ItemType> struct pstate_datatype
{
	static const pstate_data_type_e type = pstate_data_type_e(NOT_SUPPORTED);
	static const bool is_ptr = false;
};

template<typename ItemType> struct pstate_datatype<ItemType *>
{
	static const pstate_data_type_e type = pstate_data_type_e(NOT_SUPPORTED);
	static const bool is_ptr = true;
};

//template<typename ItemType> struct type_checker<ItemType*> { static const bool is_atom = false; static const bool is_pointer = true; };

#define NETLIST_SAVE_TYPE(TYPE, TYPEDESC) \
		template<> struct pstate_datatype<TYPE>{ static const pstate_data_type_e type = pstate_data_type_e(TYPEDESC); static const bool is_ptr = false;}; \
		template<> struct pstate_datatype<TYPE *>{ static const pstate_data_type_e type = pstate_data_type_e(TYPEDESC); static const bool is_ptr = true;}

NETLIST_SAVE_TYPE(char, DT_INT8);
NETLIST_SAVE_TYPE(double, DT_DOUBLE);
NETLIST_SAVE_TYPE(float, DT_FLOAT);
NETLIST_SAVE_TYPE(INT8, DT_INT8);
NETLIST_SAVE_TYPE(UINT8, DT_INT8);
#if (PHAS_INT128)
NETLIST_SAVE_TYPE(INT128, DT_INT128);
NETLIST_SAVE_TYPE(UINT128, DT_INT128);
#endif
NETLIST_SAVE_TYPE(INT64, DT_INT64);
NETLIST_SAVE_TYPE(UINT64, DT_INT64);
NETLIST_SAVE_TYPE(bool, DT_BOOLEAN);
NETLIST_SAVE_TYPE(UINT32, DT_INT);
NETLIST_SAVE_TYPE(INT32, DT_INT);
NETLIST_SAVE_TYPE(UINT16, DT_INT16);
NETLIST_SAVE_TYPE(INT16, DT_INT16);
//NETLIST_SAVE_TYPE(std::size_t, DT_INT64);

namespace plib {

class pstate_manager_t;

class pstate_callback_t
{
public:
	using list_t = std::vector<pstate_callback_t *>;

	virtual ~pstate_callback_t() { };

	virtual void register_state(pstate_manager_t &manager, const pstring &module) = 0;
	virtual void on_pre_save() = 0;
	virtual void on_post_load() = 0;
protected:
};

struct pstate_entry_t
{
	using list_t = std::vector<std::unique_ptr<pstate_entry_t>>;

	pstate_entry_t(const pstring &stname, const pstate_data_type_e dt, const void *owner,
			const int size, const int count, void *ptr, bool is_ptr)
	: m_name(stname), m_dt(dt), m_owner(owner), m_callback(nullptr), m_size(size), m_count(count), m_ptr(ptr), m_is_ptr(is_ptr) { }

	pstate_entry_t(const pstring &stname, const void *owner, pstate_callback_t *callback)
	: m_name(stname), m_dt(DT_CUSTOM), m_owner(owner), m_callback(callback), m_size(0), m_count(0), m_ptr(nullptr), m_is_ptr(false) { }

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

	template<typename C> void save_item(const void *owner, C &state, const pstring &stname)
	{
		save_state_ptr( owner, stname, pstate_datatype<C>::type, sizeof(C), 1, &state, pstate_datatype<C>::is_ptr);
	}

	template<typename C, std::size_t N> void save_item(const void *owner, C (&state)[N], const pstring &stname)
	{
		save_state_ptr(owner, stname, pstate_datatype<C>::type, sizeof(state[0]), N, &(state[0]), false);
	}

	template<typename C> void save_item(const void *owner, C *state, const pstring &stname, const int count)
	{
		save_state_ptr(owner, stname, pstate_datatype<C>::type, sizeof(C), count, state, false);
	}

	template<typename C>
	void save_item(const void *owner, std::vector<C> &v, const pstring &stname)
	{
		save_state(v.data(), owner, stname, v.size());
	}

	void pre_save();
	void post_load();
	void remove_save_items(const void *owner);

	const pstate_entry_t::list_t &save_list() const { return m_save; }

	void save_state_ptr(const void *owner, const pstring &stname, const pstate_data_type_e, const int size, const int count, void *ptr, bool is_ptr);

protected:

private:
	pstate_entry_t::list_t m_save;
};

template<> void pstate_manager_t::save_item(const void *owner, pstate_callback_t &state, const pstring &stname);

template <typename T>
class pstate_interface_t
{
public:
	pstate_interface_t() { }

	template<typename C> void save(C &state, const pstring &stname)
	{
		pstate_manager_t &manager = static_cast<T*>(this)->state_manager();
		pstring module = static_cast<T*>(this)->name();
		manager.save_item(this, state, module + "." + stname);
	}
	template<typename C, std::size_t N> void save(C (&state)[N], const pstring &stname)
	{
		pstate_manager_t &manager = static_cast<T*>(this)->state_manager();
		pstring module = static_cast<T*>(this)->name();
		manager.save_state_ptr(this, module + "." + stname, pstate_datatype<C>::type, sizeof(state[0]), N, &(state[0]), false);
	}
	template<typename C> void save(C *state, const pstring &stname, const int count)
	{
		pstate_manager_t &manager = static_cast<T*>(this)->state_manager();
		pstring module = static_cast<T*>(this)->name();
		manager.save_state_ptr(this, module + "." + stname, pstate_datatype<C>::type, sizeof(C), count, state, false);
	}
};

}

#endif /* PSTATE_H_ */
