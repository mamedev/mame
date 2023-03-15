// license:BSD-3-Clause
// copyright-holders:Couriersud

#ifndef PSTATE_H_
#define PSTATE_H_

///
/// \file pstate.h
///

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
		: m_size(bsize), m_is_integral(bintegral), m_is_float(bfloat), m_is_custom(false)
		{}
		explicit datatype_t(bool bcustom)
		: m_size(0), m_is_integral(false), m_is_float(false), m_is_custom(bcustom)
		{}

		std::size_t size() const noexcept { return m_size; }
		bool is_integral() const noexcept { return m_is_integral; }
		bool is_float()    const noexcept { return m_is_float; }
		bool is_custom()   const noexcept { return m_is_custom; }

	private:
		std::size_t m_size;
		bool m_is_integral;
		bool m_is_float;
		bool m_is_custom;
	};

	template<typename T>
	static datatype_t dtype()
	{
		return datatype_t(sizeof(T),
				plib::is_integral<T>::value || std::is_enum<T>::value,
				plib::is_floating_point<T>::value);
	}

	struct callback_t
	{
	public:
		using list_t = std::vector<callback_t *>;

		virtual void register_state(state_manager_t &manager, const pstring &module) = 0;
		virtual void on_pre_save(state_manager_t &manager) = 0;
		virtual void on_post_load(state_manager_t &manager) = 0;
	protected:
		callback_t() = default;
		virtual ~callback_t() = default;
		PCOPYASSIGNMOVE(callback_t, default)
	};

	struct entry_t
	{
		using list_t = std::vector<entry_t>;

		entry_t(const pstring &item_name, const datatype_t &dt, const void *owner,
				const std::size_t count, void *ptr)
		: m_name(item_name), m_dt(dt), m_owner(owner), m_callback(nullptr), m_count(count), m_ptr(ptr) { }

		entry_t(const pstring &item_name, const void *owner, callback_t *callback)
		: m_name(item_name), m_dt(datatype_t(true)), m_owner(owner), m_callback(callback), m_count(0), m_ptr(nullptr) { }

		pstring name() const noexcept { return m_name; }
		datatype_t dt() const noexcept { return m_dt; }
		const void * owner() const noexcept { return m_owner; }
		callback_t * callback() const noexcept { return m_callback; }
		std::size_t count() const noexcept { return m_count; }
		void * ptr() const noexcept { return m_ptr; }

	private:
		pstring             m_name;
		datatype_t          m_dt;
		const void *        m_owner;
		callback_t *        m_callback;
		std::size_t         m_count;
		void *              m_ptr;
	};

	state_manager_t() = default;

	struct saver_t
	{
		saver_t(state_manager_t &sm, const void *owner, const pstring &member_name)
		: m_sm(sm)
		, m_owner(owner)
		, m_member_name(member_name)
		{ }

		template <typename XS>
		void save_item(XS &some_state, const pstring &item_name)
		{
			m_sm.save_item(m_owner, some_state, m_member_name + "." + item_name);
		}

		state_manager_t &m_sm;
		const void * m_owner;
		const pstring m_member_name;
	};

	template<typename C>
	void save_item(const void *owner, C &state, const pstring &item_name)
	{
		save_item_dispatch(owner, state, item_name);
	}

	template<typename C, std::size_t N>
	void save_item(const void *owner, C (&state)[N], const pstring &item_name) // NOLINT(cppcoreguidelines-avoid-c-arrays, modernize-avoid-c-arrays)
	{
		save_state_ptr(owner, item_name, dtype<C>(), N, &(state[0]));
	}

	template<typename C>
	void save_item(const void *owner, C *state, const pstring &item_name, const std::size_t count)
	{
		save_state_ptr(owner, item_name, dtype<C>(), count, state);
	}

	template<typename C, typename A>
	void save_item(const void *owner, std::vector<C, A> &v, const pstring &item_name)
	{
		save_state_ptr(owner, item_name, dtype<C>(), v.size(), v.data());
	}

	template<typename C, std::size_t N>
	void save_item(const void *owner, std::array<C, N> &a, const pstring &item_name)
	{
		save_state_ptr(owner, item_name, dtype<C>(), N, a.data());
	}

	void save_state_ptr(const void *owner, const pstring &item_name, const datatype_t &dt, const std::size_t count, void *ptr)
	{
		m_save.emplace_back(item_name, dt, owner, count, ptr);
	}

	void pre_save()
	{
		for (auto & s : m_custom)
			s.callback()->on_pre_save(*this);
	}

	void post_load()
	{
		for (auto & s : m_custom)
			s.callback()->on_post_load(*this);
	}

	void remove_save_items(const void *owner)
	{
		auto i = m_save.end();
		while (i != m_save.begin())
		{
			i--;
			if (i->owner() == owner)
				i = m_save.erase(i);
		}
		i = m_custom.end();
		while (i > m_custom.begin())
		{
			i--;
			if (i->owner() == owner)
				i = m_custom.erase(i);
		}
	}

	std::vector<const entry_t *> save_list() const
	{
		std::vector<const entry_t *> ret;
		for (const auto &i : m_save)
			ret.push_back(&i);
		return ret;
	}

protected:

private:

	template<typename C>
	std::enable_if_t<plib::is_integral<C>::value || std::is_enum<C>::value
			|| plib::is_floating_point<C>::value>
	save_item_dispatch(const void *owner, C &state, const pstring &item_name)
	{
		save_state_ptr( owner, item_name, dtype<C>(), 1, &state);
	}

	template<typename C>
	std::enable_if_t<!(plib::is_integral<C>::value || std::is_enum<C>::value
			|| plib::is_floating_point<C>::value)>
	save_item_dispatch(const void *owner, C &state, const pstring &item_name)
	{
		saver_t sav(*this, owner, item_name);
		state.save_state(sav);
	}

	entry_t::list_t m_save;
	entry_t::list_t m_custom;

};

template<>
inline void state_manager_t::save_item(const void *owner, callback_t &state, const pstring &item_name)
{
	m_custom.emplace_back(item_name, owner, &state);
	state.register_state(*this, item_name);
}


} // namespace plib

#endif // PSTATE_H_
