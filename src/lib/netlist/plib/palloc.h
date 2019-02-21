// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * palloc.h
 *
 */

#ifndef PALLOC_H_
#define PALLOC_H_

#include "pstring.h"
#include "ptypes.h"

#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

namespace plib {

	//============================================================
	//  Memory allocation
	//============================================================

	template<typename T, typename... Args>
	T *palloc(Args&&... args)
	{
		return new T(std::forward<Args>(args)...);
	}

	template<typename T>
	void pfree(T *ptr)
	{
		delete ptr;
	}

	template<typename T>
	T* palloc_array(const std::size_t num)
	{
		return new T[num]();
	}

	template<typename T>
	void pfree_array(T *ptr)
	{
		delete [] ptr;
	}

	template <typename SC, typename D = std::default_delete<SC>>
	class owned_ptr
	{
	public:
		owned_ptr()
		: m_ptr(nullptr), m_is_owned(true) { }

		template <typename, typename>
		friend class owned_ptr;

		owned_ptr(SC *p, bool owned) noexcept
		: m_ptr(p), m_deleter(), m_is_owned(owned)
		{ }

		owned_ptr(SC *p, bool owned, D deleter) noexcept
		: m_ptr(p), m_deleter(deleter), m_is_owned(owned)
		{ }


		owned_ptr(const owned_ptr &r) = delete;
		owned_ptr & operator =(owned_ptr &r) = delete;

		template<typename DC, typename DC_D>
		owned_ptr & operator =(owned_ptr<DC, DC_D> &&r)
		{
			if (m_is_owned && (m_ptr != nullptr))
				//delete m_ptr;
				m_deleter(m_ptr);
			m_is_owned = r.m_is_owned;
			m_ptr = r.m_ptr;
			m_deleter = r.m_deleter;
			r.m_is_owned = false;
			r.m_ptr = nullptr;
			return *this;
		}

		owned_ptr(owned_ptr &&r) noexcept
		{
			m_is_owned = r.m_is_owned;
			m_ptr = r.m_ptr;
			m_deleter = r.m_deleter;
			r.m_is_owned = false;
			r.m_ptr = nullptr;
		}

		owned_ptr &operator=(owned_ptr &&r) noexcept
		{
			if (m_is_owned && (m_ptr != nullptr))
				//delete m_ptr;
				m_deleter(m_ptr);
			m_is_owned = r.m_is_owned;
			m_ptr = r.m_ptr;
			m_deleter = r.m_deleter;
			r.m_is_owned = false;
			r.m_ptr = nullptr;
			return *this;
		}

		template<typename DC, typename DC_D>
		owned_ptr(owned_ptr<DC, DC_D> &&r) noexcept
		{
			m_ptr = static_cast<SC *>(r.get());
			m_is_owned = r.is_owned();
			m_deleter = r.m_deleter;
			r.release();
		}

		~owned_ptr() noexcept
		{
			if (m_is_owned && (m_ptr != nullptr))
			{
				//delete m_ptr;
				m_deleter(m_ptr);
			}
			m_is_owned = false;
			m_ptr = nullptr;
		}
		SC * release()
		{
			SC *tmp = m_ptr;
			m_is_owned = false;
			m_ptr = nullptr;
			return tmp;
		}

		bool is_owned() const { return m_is_owned; }

		SC * operator ->() const { return m_ptr; }
		SC & operator *() const { return *m_ptr; }
		SC * get() const { return m_ptr; }
	private:
		SC *m_ptr;
		D m_deleter;
		bool m_is_owned;
	};

	template<typename T, typename... Args>
	std::unique_ptr<T> make_unique(Args&&... args)
	{
		return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
	}

	template<typename T, typename... Args>
	static owned_ptr<T> make_owned(Args&&... args)
	{
		owned_ptr<T> a(new T(std::forward<Args>(args)...), true);
		return std::move(a);
	}

} // namespace plib

#endif /* PALLOC_H_ */
