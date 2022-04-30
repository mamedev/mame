// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    lrucache.h

    Associative LRU cache with map-like behaviour.

***************************************************************************/

#ifndef MAME_UTIL_LRUCACHE_H
#define MAME_UTIL_LRUCACHE_H

#pragma once

#include <cassert>
#include <functional>
#include <iterator>
#include <list>
#include <memory>
#include <set>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>

namespace util {

// LRU cache that behaves like std::map with differences:
// * drops least-recently used items if necessary on insert to prevent size from exceeding max_size
// * operator[], at, insert, emplace and find freshen existing entries
// * iterates from least- to most-recently used rather than in order by key
// * iterators to dropped items are invalidated
// * not all map interfaces implemented
// * copyable and swappable but not movable
// * swap may invalidate past-the-end iterator, other iterators refer to new container
template <typename Key, typename T, typename Compare = std::less<Key>, class Allocator = std::allocator<std::pair<Key const, T> > >
class lru_cache_map
{
private:
	class iterator_compare;
	typedef std::list<std::pair<Key const, T>, Allocator> value_list;
	typedef typename std::allocator_traits<Allocator>::template rebind_alloc<typename value_list::iterator> iterator_allocator_type;
	typedef std::set<typename value_list::iterator, iterator_compare, iterator_allocator_type> iterator_set;

	class iterator_compare
	{
	public:
		typedef std::true_type is_transparent;
		iterator_compare(Compare const &comp) : m_comp(comp) { }
		iterator_compare(iterator_compare const &that) = default;
		iterator_compare(iterator_compare &&that) = default;
		Compare key_comp() const { return m_comp; }
		iterator_compare &operator=(iterator_compare const &that) = default;
		iterator_compare &operator=(iterator_compare &&that) = default;
		bool operator()(typename value_list::iterator const &lhs, typename value_list::iterator const &rhs) const { return m_comp(lhs->first, rhs->first); }
		template <typename K> bool operator()(typename value_list::iterator const &lhs, K const &rhs) const { return m_comp(lhs->first, rhs); }
		template <typename K> bool operator()(K const &lhs, typename value_list::iterator const &rhs) const { return m_comp(lhs, rhs->first); }
	private:
		Compare m_comp;
	};

public:
	typedef Key key_type;
	typedef T mapped_type;
	typedef std::pair<Key const, T> value_type;
	typedef typename value_list::size_type size_type;
	typedef typename value_list::difference_type difference_type;
	typedef Compare key_compare;
	typedef Allocator allocator_type;
	typedef value_type &reference;
	typedef value_type const &const_reference;
	typedef typename std::allocator_traits<Allocator>::pointer pointer;
	typedef typename std::allocator_traits<Allocator>::const_pointer const_pointer;
	typedef typename value_list::iterator iterator;
	typedef typename value_list::const_iterator const_iterator;
	typedef typename value_list::reverse_iterator reverse_iterator;
	typedef typename value_list::const_reverse_iterator const_reverse_iterator;

	explicit lru_cache_map(size_type max_size)
		: lru_cache_map(max_size, key_compare())
	{
	}
	lru_cache_map(size_type max_size, key_compare const &comp, allocator_type const &alloc = allocator_type())
		: m_max_size(max_size)
		, m_size(0U)
		, m_elements(alloc)
		, m_mapping(iterator_compare(comp), iterator_allocator_type(alloc))
	{
		assert(0U < m_max_size);
	}
	lru_cache_map(lru_cache_map const &that)
		: m_max_size(that.m_max_size)
		, m_size(that.m_size)
		, m_elements(that.m_elements)
		, m_mapping(that.m_mapping.key_comp(), that.m_mapping.get_allocator())
	{
		for (iterator it = m_elements.begin(); it != m_elements.end(); ++it)
			m_mapping.insert(it);
		assert(m_elements.size() == m_size);
		assert(m_mapping.size() == m_size);
	}

	allocator_type get_allocator() const { return m_elements.get_allocator(); }

	iterator begin() { return m_elements.begin(); }
	const_iterator begin() const { return m_elements.cbegin(); }
	const_iterator cbegin() const { return m_elements.cbegin(); }
	iterator end() { return m_elements.end(); }
	const_iterator end() const { return m_elements.cend(); }
	const_iterator cend() const { return m_elements.cend(); }
	reverse_iterator rbegin() { return m_elements.rbegin(); }
	const_reverse_iterator rbegin() const { return m_elements.crbegin(); }
	const_reverse_iterator crbegin() const { return m_elements.crbegin(); }
	reverse_iterator rend() { return m_elements.end(); }
	const_reverse_iterator rend() const { return m_elements.crend(); }
	const_reverse_iterator crend() const { return m_elements.crend(); }

	bool empty() const { return !m_size; }
	size_type size() const { return m_size; }
	size_type max_size() const { return m_max_size; }

	mapped_type &operator[](key_type const &key)
	{
		typename iterator_set::iterator existing(m_mapping.lower_bound(key));
		if ((m_mapping.end() != existing) && !m_mapping.key_comp()(key, *existing))
		{
			m_elements.splice(m_elements.cend(), m_elements, *existing);
			return (*existing)->second;
		}
		make_space(existing);
		iterator const inserted(m_elements.emplace(m_elements.end(), std::piecewise_construct, std::forward_as_tuple(key), std::tuple<>()));
		m_mapping.insert(existing, inserted);
		++m_size;
		assert(m_elements.size() == m_size);
		assert(m_mapping.size() == m_size);
		return inserted->second;
	}
	mapped_type &operator[](key_type &&key)
	{
		typename iterator_set::iterator existing(m_mapping.lower_bound(key));
		if ((m_mapping.end() != existing) && !m_mapping.key_comp()(key, *existing))
		{
			m_elements.splice(m_elements.cend(), m_elements, *existing);
			return (*existing)->second;
		}
		make_space(existing);
		iterator const inserted(m_elements.emplace(m_elements.end(), std::piecewise_construct, std::forward_as_tuple(std::move(key)), std::tuple<>()));
		m_mapping.insert(existing, inserted);
		++m_size;
		assert(m_elements.size() == m_size);
		assert(m_mapping.size() == m_size);
		return inserted->second;
	}
	mapped_type &at(key_type const &key)
	{
		typename iterator_set::iterator existing(m_mapping.find(key));
		if (m_mapping.end() != existing)
		{
			m_elements.splice(m_elements.cend(), m_elements, *existing);
			return (*existing)->second;
		}
		else
		{
			throw std::out_of_range("lru_cache_map::at");
		}
	}
	mapped_type const &at(key_type const &key) const
	{
		typename iterator_set::iterator existing(m_mapping.find(key));
		if (m_mapping.end() != existing)
		{
			m_elements.splice(m_elements.cend(), m_elements, *existing);
			return (*existing)->second;
		}
		else
		{
			throw std::out_of_range("lru_cache_map::at");
		}
	}

	void clear()
	{
		m_size = 0U;
		m_elements.clear();
		m_mapping.clear();
	}
	std::pair<iterator, bool> insert(value_type const &value)
	{
		typename iterator_set::iterator existing(m_mapping.lower_bound(value.first));
		if ((m_mapping.end() != existing) && !m_mapping.key_comp()(value.first, *existing))
		{
			m_elements.splice(m_elements.cend(), m_elements, *existing);
			return std::pair<iterator, bool>(*existing, false);
		}
		make_space(existing);
		iterator const inserted(m_elements.emplace(m_elements.end(), value));
		m_mapping.insert(existing, inserted);
		++m_size;
		assert(m_elements.size() == m_size);
		assert(m_mapping.size() == m_size);
		return std::pair<iterator, bool>(inserted, true);
	}
	std::pair<iterator, bool> insert(value_type &&value)
	{
		typename iterator_set::iterator existing(m_mapping.lower_bound(value.first));
		if ((m_mapping.end() != existing) && !m_mapping.key_comp()(value.first, *existing))
		{
			m_elements.splice(m_elements.cend(), m_elements, *existing);
			return std::pair<iterator, bool>(*existing, false);
		}
		make_space(existing);
		iterator const inserted(m_elements.emplace(m_elements.end(), std::move(value)));
		m_mapping.insert(existing, inserted);
		++m_size;
		assert(m_elements.size() == m_size);
		assert(m_mapping.size() == m_size);
		return std::pair<iterator, bool>(inserted, true);
	}
	template <typename P>
	std::enable_if_t<std::is_constructible<value_type, P>::value, std::pair<iterator, bool> > insert(P &&value)
	{
		return emplace(std::forward<P>(value));
	}
	template <typename InputIt>
	void insert(InputIt first, InputIt last)
	{
		while (first != last)
		{
			insert(*first);
			++first;
		}
	}
	void insert(std::initializer_list<value_type> ilist)
	{
		for (value_type const &value : ilist)
			insert(value);
	}
	template <typename... Params>
	std::pair<iterator, bool> emplace(Params &&... args)
	{
		// TODO: is there a more efficient way than depending on value_type being efficiently movable?
		return insert(value_type(std::forward<Params>(args)...));
	}
	iterator erase(const_iterator pos)
	{
		m_mapping.erase(m_elements.erase(pos, pos));
		iterator const result(m_elements.erase(pos));
		--m_size;
		assert(m_elements.size() == m_size);
		assert(m_mapping.size() == m_size);
		return result;
	}
	iterator erase(const_iterator first, const_iterator last)
	{
		iterator pos(m_elements.erase(first, first));
		while (pos != last)
		{
			m_mapping.erase(pos);
			pos = m_elements.erase(pos);
			--m_size;
		}
		assert(m_elements.size() == m_size);
		assert(m_mapping.size() == m_size);
		return pos;
	}
	size_type erase(key_type const &key)
	{
		typename iterator_set::iterator const found(m_mapping.find(key));
		if (m_mapping.end() == found)
		{
			return 0U;
		}
		else
		{
			m_elements.erase(*found);
			m_mapping.erase(found);
			--m_size;
			assert(m_elements.size() == m_size);
			assert(m_mapping.size() == m_size);
			return 1U;
		}
	}
	void swap(lru_cache_map &that)
	{
		using std::swap;
		swap(m_max_size, that.m_max_size);
		swap(m_size, that.m_size);
		swap(m_elements, that.m_elements);
		swap(m_mapping, that.m_mapping);
	}

	size_type count(key_type const &key) const
	{
		// TODO: perhaps this should freshen an element
		return m_mapping.count(key);
	}
	template <typename K>
	size_type count(K const &x) const
	{
		// FIXME: should only enable this overload if Compare::is_transparent
		// TODO: perhaps this should freshen an element
		return m_mapping.count(x);
	}
	iterator find(key_type const &key)
	{
		typename iterator_set::const_iterator const found(m_mapping.find(key));
		if (m_mapping.end() == found)
		{
			return m_elements.end();
		}
		else
		{
			m_elements.splice(m_elements.cend(), m_elements, *found);
			return *found;
		}
	}
	iterator find(key_type const &key) const
	{
		typename iterator_set::const_iterator const found(m_mapping.find(key));
		if (m_mapping.end() == found)
		{
			return m_elements.end();
		}
		else
		{
			m_elements.splice(m_elements.cend(), m_elements, *found);
			return *found;
		}
	}
	template <typename K>
	iterator find(K const &x)
	{
		// FIXME: should only enable this overload if Compare::is_transparent
		typename iterator_set::const_iterator const found(m_mapping.find(x));
		if (m_mapping.end() == found)
		{
			return m_elements.end();
		}
		else
		{
			m_elements.splice(m_elements.cend(), m_elements, *found);
			return *found;
		}
	}
	template <typename K>
	iterator find(K const &x) const
	{
		// FIXME: should only enable this overload if Compare::is_transparent
		typename iterator_set::const_iterator const found(m_mapping.find(x));
		if (m_mapping.end() == found)
		{
			return m_elements.end();
		}
		else
		{
			m_elements.splice(m_elements.cend(), m_elements, *found);
			return *found;
		}
	}

	key_compare key_comp() const
	{
		return m_mapping.key_comp().key_comp();
	}

	lru_cache_map &operator=(lru_cache_map const &that)
	{
		m_max_size = that.m_max_size;
		m_size = that.m_size;
		m_elements = that.m_elements;
		m_mapping.clear();
		for (iterator it = m_elements.begin(); it != m_elements.end(); ++it)
			m_mapping.insert(it);
		assert(m_elements.size() == m_size);
		assert(m_mapping.size() == m_size);
		return *this;
	}

private:
	void make_space(typename iterator_set::iterator &existing)
	{
		while (m_max_size <= m_size)
		{
			if ((m_mapping.end() != existing) && (m_elements.begin() == *existing))
				existing = m_mapping.erase(existing);
			else
				m_mapping.erase(m_elements.begin());
			m_elements.erase(m_elements.begin());
			--m_size;
		}
	}

	size_type           m_max_size;
	size_type           m_size;
	mutable value_list  m_elements;
	iterator_set        m_mapping;
};

template <typename Key, typename T, typename Compare, class Allocator>
void swap(lru_cache_map<Key, T, Compare, Allocator> &lhs, lru_cache_map<Key, T, Compare, Allocator> &rhs)
{
	lhs.swap(rhs);
}

} // namespace util

#endif // MAME_UTIL_LRUCACHE_H
