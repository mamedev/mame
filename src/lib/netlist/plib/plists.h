// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * plists.h
 *
 */

#pragma once

#ifndef PLISTS_H_
#define PLISTS_H_

#include <algorithm>
#include <vector>
#include <type_traits>
//#include <cstring>
//#include <array>

#include "palloc.h"
#include "pstring.h"

namespace plib {

/* ----------------------------------------------------------------------------------------
 * uninitialised_array_t:
 * 		fixed size array allowing to override constructor and initialize
 * 		members by placement new.
 *
 * 		Use with care. This template is provided to improve locality of storage
 * 		in high frequency applications. It should not be used for anything else.
 * ---------------------------------------------------------------------------------------- */

template <class C, std::size_t N>
class uninitialised_array_t
{
public:
	uninitialised_array_t()
	{
	}

	~uninitialised_array_t()
	{
		for (std::size_t i=0; i<N; i++)
			(*this)[i].~C();
	}

	size_t size() { return N; }

	C& operator[](const std::size_t &index)
	{
		return *reinterpret_cast<C *>(&m_buf[index]);
	}

	const C& operator[](const std::size_t &index) const
	{
		return *reinterpret_cast<C *>(&m_buf[index]);
	}

	template<typename... Args>
	void emplace(const std::size_t index, Args&&... args)
	{
		new (&m_buf[index]) C(std::forward<Args>(args)...);
	}

protected:

private:

	/* ensure proper alignment */
	typename std::aligned_storage<sizeof(C), alignof(C)>::type m_buf[N];
};

// ----------------------------------------------------------------------------------------
// plist_t: a simple list
// ----------------------------------------------------------------------------------------

template <typename LC>
class pvector_t : public std::vector<LC>
{
public:
	pvector_t() : std::vector<LC>() {}

	bool contains(const LC &elem) const
	{
		return (std::find(this->begin(), this->end(), elem) != this->end());
	}

	void remove(const LC &elem)
	{
		this->erase(std::remove(this->begin(), this->end(), elem), this->end());
	}

	void insert_at(const std::size_t index, const LC &elem)
	{
		this->insert(this->begin() + index, elem);
	}

	 void remove_at(const std::size_t pos)
	{
		this->erase(this->begin() + pos);
	}

	int indexof(const LC &elem) const
	{
		for (std::size_t i = 0; i < this->size(); i++)
		{
			if (this->at(i) == elem)
				return i;
		}
		return -1;
	}

};

// ----------------------------------------------------------------------------------------
// plinkedlist_t: a simple linked list
//                the list allows insertions / deletions if used properly
// ----------------------------------------------------------------------------------------

template <class LC>
class linkedlist_t
{
public:

	struct element_t
	{
	public:

		friend class linkedlist_t<LC>;

		element_t() : m_next(nullptr) {}

		LC *next() const { return m_next; }
	private:
		LC * m_next;
	};

	struct iter_t final : public std::iterator<std::forward_iterator_tag, LC>
	{
		LC* p;
	public:
		constexpr iter_t(LC* x) noexcept : p(x) {}
		iter_t(const iter_t &rhs) noexcept = default;
		iter_t(iter_t &&rhs) noexcept = default;
		iter_t& operator++() noexcept {p = p->next();return *this;}
		iter_t operator++(int) noexcept {iter_t tmp(*this); operator++(); return tmp;}
		bool operator==(const iter_t& rhs) noexcept {return p==rhs.p;}
		bool operator!=(const iter_t& rhs) noexcept {return p!=rhs.p;}
		LC& operator*() noexcept {return *p;}
		LC* operator->() noexcept {return p;}
	};

	linkedlist_t() : m_head(nullptr) {}

	iter_t begin() const { return iter_t(m_head); }
	constexpr iter_t end() const { return iter_t(nullptr); }

	void push_front(LC *elem)
	{
		elem->m_next = m_head;
		m_head = elem;
	}

	void push_back(LC *elem)
	{
		LC **p = &m_head;
		while (*p != nullptr)
		{
			p = &((*p)->m_next);
		}
		*p = elem;
		elem->m_next = nullptr;
	}

	void remove(const LC *elem)
	{
		auto p = &m_head;
		for ( ; *p != elem; p = &((*p)->m_next))
		{
			//nl_assert(*p != nullptr);
		}
		(*p) = elem->m_next;
	}

	LC *front() const { return m_head; }
	void clear() { m_head = nullptr; }
	bool empty() const { return (m_head == nullptr); }

//private:
	LC *m_head;
};


// ----------------------------------------------------------------------------------------
// hashmap list
// ----------------------------------------------------------------------------------------


template <class C>
struct hash_functor
{
	hash_functor()
	: m_hash(0)
	{}
	hash_functor(const C &v)
	: m_hash(v)
	{
	}
	friend unsigned operator%(const hash_functor &lhs, const unsigned &rhs) { return lhs.m_hash % rhs; }
	bool operator==(const hash_functor &lhs) const { return (m_hash == lhs.m_hash); }
private:
	unsigned m_hash;
};

template <>
struct hash_functor<pstring>
{
	hash_functor()
	: m_hash(0)
	{}
	hash_functor(const pstring &v)
	{
		/* modified djb2 */
		const pstring::mem_t *string = v.cstr();
		unsigned result = 5381;
		for (pstring::mem_t c = *string; c != 0; c = *string++)
			result = ((result << 5) + result ) ^ (result >> (32 - 5)) ^ c;
			//result = (result*33) ^ c;
		m_hash = result;
	}
	friend unsigned operator%(const hash_functor<pstring> &lhs, const unsigned &rhs) { return lhs.m_hash % rhs; }
	unsigned operator()() { return m_hash; }
	bool operator==(const hash_functor<pstring> &lhs) const { return (m_hash == lhs.m_hash); }
private:
	unsigned m_hash;
};

#if 0
#if 0
	unsigned hash(const pstring &v) const
	{
		/* Fowler???Noll???Vo hash - FNV-1 */
		const char *string = v.cstr();
		unsigned result = 2166136261;
		for (UINT8 c = *string++; c != 0; c = *string++)
			result = (result * 16777619) ^ c;
			// result = (result ^ c) * 16777619; FNV 1a
		return result;
	}
#else
	unsigned hash(const pstring &v) const
	{
		/* jenkins one at a time algo */
		unsigned result = 0;
		const char *string = v.cstr();
		while (*string)
		{
			result += *string;
			string++;
			result += (result << 10);
			result ^= (result >> 6);
		}
		result += (result << 3);
		result ^= (result >> 11);
		result += (result << 15);
		return result;
	}
#endif
#endif

template <class K, class V, class H = hash_functor<K> >
class hashmap_t
{
public:
	hashmap_t() : m_hash(37)
	{
		for (unsigned i=0; i<m_hash.size(); i++)
			m_hash[i] = -1;
	}

	~hashmap_t()
	{
	}

	struct element_t
	{
		element_t(const K &key, const H &hash, const V &value)
		: m_key(key), m_hash(hash), m_value(value), m_next(-1)
		{}
		K m_key;
		H m_hash;
		V m_value;
		int m_next;
	};

	void clear()
	{
#if 0
		if (0)
		{
			unsigned cnt = 0;
			for (unsigned i=0; i<m_hash.size(); i++)
				if (m_hash[i] >= 0)
					cnt++;
			const unsigned s = m_values.size();
			if (s>0)
				printf("phashmap: %d elements %d hashsize, percent in overflow: %d\n", s, (unsigned) m_hash.size(),  (s - cnt) * 100 / s);
			else
				printf("phashmap: No elements .. \n");
		}
#endif
		m_values.clear();
		for (unsigned i=0; i<m_hash.size(); i++)
			m_hash[i] = -1;
	}

	bool contains(const K &key) const
	{
		return (get_idx(key) >= 0);
	}

	int index_of(const K &key) const
	{
		return get_idx(key);
	}

	unsigned size() const { return m_values.size(); }

	bool add(const K &key, const V &value)
	{
		/*
		 * we are using the Euler prime function here
		 *
		 * n * n + n + 41 | 40 >= n >=0
		 *
		 * and accept that outside we will not have a prime
		 *
		 */
		if (m_values.size() * 3 / 2 > m_hash.size())
		{
			unsigned n = std::sqrt( 2 * m_hash.size());
			n = n * n + n + 41;
			m_hash.resize(n);
			rebuild();
		}
		const H hash(key);
		const unsigned pos = hash % m_hash.size();
		if (m_hash[pos] == -1)
		{
			unsigned vpos = m_values.size();
			m_values.push_back(element_t(key, hash, value));
			m_hash[pos] = vpos;
		}
		else
		{
			int ep = m_hash[pos];

			for (; ep != -1; ep = m_values[ep].m_next)
			{
				if (m_values[ep].m_hash == hash && m_values[ep].m_key == key )
					return false; /* duplicate */
			}
			unsigned vpos = m_values.size();
			m_values.push_back(element_t(key, hash, value));
			m_values[vpos].m_next = m_hash[pos];
			m_hash[pos] = vpos;
		}
		return true;
	}

	V& operator[](const K &key)
	{
		int p = get_idx(key);
		if (p == -1)
		{
			p = m_values.size();
			add(key, V());
		}
		return m_values[p].m_value;
	}

	V& value_at(const unsigned pos) { return m_values[pos].m_value; }
	const V& value_at(const unsigned pos) const { return m_values[pos].m_value; }

	V& key_at(const unsigned pos) { return m_values[pos].m_key; }
private:

	int get_idx(const K &key) const
	{
		H hash(key);
		const unsigned pos = hash % m_hash.size();

		for (int ep = m_hash[pos]; ep != -1; ep = m_values[ep].m_next)
			if (m_values[ep].m_hash == hash && m_values[ep].m_key == key )
				return ep;
		return -1;
	}

	void rebuild()
	{
		for (unsigned i=0; i<m_hash.size(); i++)
			m_hash[i] = -1;
		for (unsigned i=0; i<m_values.size(); i++)
		{
			unsigned pos = m_values[i].m_hash % m_hash.size();
			m_values[i].m_next = m_hash[pos];
			m_hash[pos] = i;
		}

	}
	pvector_t<element_t> m_values;
	std::vector<int> m_hash;
};

// ----------------------------------------------------------------------------------------
// sort a list ... slow, I am lazy
// elements must support ">" operator.
// ----------------------------------------------------------------------------------------

template<typename T>
static inline void sort_list(T &sl)
{
	for(unsigned i = 0; i < sl.size(); i++)
	{
		for(unsigned j = i + 1; j < sl.size(); j++)
			if(sl[i] > sl[j])
				std::swap(sl[i], sl[j]);

	}
}

}

#endif /* PLISTS_H_ */
