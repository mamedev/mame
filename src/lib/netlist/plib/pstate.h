// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * pstate.h
 *
 */

#ifndef PSTATE_H_
#define PSTATE_H_

#include "palloc.h"
#include "pstring.h"
#include "ptypes.h"

#include <array>
#include <memory>
#include <vector>

// ----------------------------------------------------------------------------------------
// state saving ...
// ----------------------------------------------------------------------------------------

namespace plib {
class state_manager_t
{
public:

	struct datatype_t
	{
		datatype_t(std::size_t bsize, bool bintegral, bool bfloat)
		: size(bsize), is_integral(bintegral), is_float(bfloat), is_custom(false)
		{}
		explicit datatype_t(bool bcustom)
		: size(0), is_integral(false), is_float(false), is_custom(bcustom)
		{}

		const std::size_t size;
		const bool is_integral;
		const bool is_float;
		const bool is_custom;
	};

	template<typename T>
	static datatype_t dtype()
	{
		return datatype_t(sizeof(T),
				plib::is_integral<T>::value || std::is_enum<T>::value,
				std::is_floating_point<T>::value);
	}

	class callback_t
	{
	public:
		using list_t = std::vector<callback_t *>;

		virtual void register_state(state_manager_t &manager, const pstring &module) = 0;
		virtual void on_pre_save(state_manager_t &manager) = 0;
		virtual void on_post_load(state_manager_t &manager) = 0;
	protected:
		callback_t() = default;
		~callback_t() = default;
		COPYASSIGNMOVE(callback_t, default)
	};

	struct entry_t
	{
		using list_t = std::vector<plib::unique_ptr<entry_t>>;

		entry_t(const pstring &stname, const datatype_t &dt, const void *owner,
				const std::size_t count, void *ptr)
		: m_name(stname), m_dt(dt), m_owner(owner), m_callback(nullptr), m_count(count), m_ptr(ptr) { }

		entry_t(const pstring &stname, const void *owner, callback_t *callback)
		: m_name(stname), m_dt(datatype_t(true)), m_owner(owner), m_callback(callback), m_count(0), m_ptr(nullptr) { }

		pstring             m_name;
		const datatype_t    m_dt;
		const void *        m_owner;
		callback_t *        m_callback;
		const std::size_t   m_count;
		void *              m_ptr;
	};

	state_manager_t() = default;

	template<typename C>
	void save_item(const void *owner, C &state, const pstring &stname)
	{
		save_state_ptr( owner, stname, dtype<C>(), 1, &state);
	}

	template<typename C, std::size_t N>
	void save_item(const void *owner, C (&state)[N], const pstring &stname) // NOLINT(cppcoreguidelines-avoid-c-arrays, modernize-avoid-c-arrays)
	{
		save_state_ptr(owner, stname, dtype<C>(), N, &(state[0]));
	}

	template<typename C>
	void save_item(const void *owner, C *state, const pstring &stname, const std::size_t count)
	{
		save_state_ptr(owner, stname, dtype<C>(), count, state);
	}

	template<typename C>
	void save_item(const void *owner, std::vector<C> &v, const pstring &stname)
	{
		save_state_ptr(owner, stname, dtype<C>(), v.size(), v.data());
	}

	template<typename C, std::size_t N>
	void save_item(const void *owner, std::array<C, N> &a, const pstring &stname)
	{
		save_state_ptr(owner, stname, dtype<C>(), N, a.data());
	}

	void pre_save();
	void post_load();
	void remove_save_items(const void *owner);

	const std::vector<const entry_t *> save_list() const
	{
		std::vector<const entry_t *> ret;
		for (auto &i : m_save)
			ret.push_back(i.get());
		return ret;
	}

	void save_state_ptr(const void *owner, const pstring &stname, const datatype_t &dt, const std::size_t count, void *ptr);

protected:

private:
	entry_t::list_t m_save;
	entry_t::list_t m_custom;

};

template<> void state_manager_t::save_item(const void *owner, callback_t &state, const pstring &stname);

} // namespace plib

#endif /* PSTATE_H_ */
