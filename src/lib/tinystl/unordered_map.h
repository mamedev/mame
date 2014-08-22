/*-
 * Copyright 2012 Matthew Endsley
 * All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted providing that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef TINYSTL_UNORDERED_MAP_H
#define TINYSTL_UNORDERED_MAP_H

#include "buffer.h"
#include "hash.h"
#include "hash_base.h"

namespace tinystl {

	template<typename Key, typename Value, typename Alloc = TINYSTL_ALLOCATOR>
	class unordered_map {
	public:
		unordered_map();
		unordered_map(const unordered_map& other);
		~unordered_map();

		unordered_map& operator=(const unordered_map& other);


		typedef pair<Key, Value> value_type;
		
		typedef unordered_hash_iterator<const unordered_hash_node<Key, Value> > const_iterator;
		typedef unordered_hash_iterator<unordered_hash_node<Key, Value> > iterator;

		iterator begin();
		iterator end();

		const_iterator begin() const;
		const_iterator end() const;

		void clear();
		bool empty() const;
		size_t size() const;

		const_iterator find(const Key& key) const;
		iterator find(const Key& key);
		pair<iterator, bool> insert(const pair<Key, Value>& p);
		void erase(const_iterator where);

		Value& operator[](const Key& key);

		void swap(unordered_map& other);

	private:

		typedef unordered_hash_node<Key, Value>* pointer;

		size_t m_size;
		buffer<pointer, Alloc> m_buckets;
	};

	template<typename Key, typename Value, typename Alloc>
	unordered_map<Key, Value, Alloc>::unordered_map()
		: m_size(0)
	{
		buffer_init<pointer, Alloc>(&m_buckets);
		buffer_resize<pointer, Alloc>(&m_buckets, 9, 0);
	}

	template<typename Key, typename Value, typename Alloc>
	unordered_map<Key, Value, Alloc>::unordered_map(const unordered_map& other)
		: m_size(other.m_size)
	{
		const size_t nbuckets = (size_t)(other.m_buckets.last - other.m_buckets.first);
		buffer_init<pointer, Alloc>(&m_buckets);
		buffer_resize<pointer, Alloc>(&m_buckets, nbuckets, 0);

		for (pointer it = *other.m_buckets.first; it; it = it->next) {
			unordered_hash_node<Key, Value>* newnode = new(placeholder(), Alloc::static_allocate(sizeof(unordered_hash_node<Key, Value>))) unordered_hash_node<Key, Value>(it->first, it->second);
			newnode->next = newnode->prev = 0;

			unordered_hash_node_insert(newnode, hash(it->first), m_buckets.first, nbuckets - 1);
		}
	}

	template<typename Key, typename Value, typename Alloc>
	unordered_map<Key, Value, Alloc>::~unordered_map() {
		clear();
		buffer_destroy<pointer, Alloc>(&m_buckets);
	}

	template<typename Key, typename Value, typename Alloc>
	unordered_map<Key, Value, Alloc>& unordered_map<Key, Value, Alloc>::operator=(const unordered_map<Key, Value, Alloc>& other) {
		unordered_map<Key, Value, Alloc>(other).swap(*this);
		return *this;
	}

	template<typename Key, typename Value, typename Alloc>
	inline typename unordered_map<Key, Value, Alloc>::iterator unordered_map<Key, Value, Alloc>::begin() {
		iterator it;
		it.node = *m_buckets.first;
		return it;
	}

	template<typename Key, typename Value, typename Alloc>
	inline typename unordered_map<Key, Value, Alloc>::iterator unordered_map<Key, Value, Alloc>::end() {
		iterator it;
		it.node = 0;
		return it;
	}

	template<typename Key, typename Value, typename Alloc>
	inline typename unordered_map<Key, Value, Alloc>::const_iterator unordered_map<Key, Value, Alloc>::begin() const {
		const_iterator cit;
		cit.node = *m_buckets.first;
		return cit;
	}

	template<typename Key, typename Value, typename Alloc>
	inline typename unordered_map<Key, Value, Alloc>::const_iterator unordered_map<Key, Value, Alloc>::end() const {
		const_iterator cit;
		cit.node = 0;
		return cit;
	}

	template<typename Key, typename Value, typename Alloc>
	inline bool unordered_map<Key, Value, Alloc>::empty() const {
		return m_size == 0;
	}

	template<typename Key, typename Value, typename Alloc>
	inline size_t unordered_map<Key, Value, Alloc>::size() const {
		return m_size;
	}

	template<typename Key, typename Value, typename Alloc>
	inline void unordered_map<Key, Value, Alloc>::clear() {
		pointer it = *m_buckets.first;
		while (it) {
			const pointer next = it->next;
			it->~unordered_hash_node<Key, Value>();
			Alloc::static_deallocate(it, sizeof(unordered_hash_node<Key, Value>));

			it = next;
		}

		m_buckets.last = m_buckets.first;
		buffer_resize<pointer, Alloc>(&m_buckets, 9, 0);
		m_size = 0;
	}

	template<typename Key, typename Value, typename Alloc>
	inline typename unordered_map<Key, Value, Alloc>::iterator unordered_map<Key, Value, Alloc>::find(const Key& key) {
		iterator result;
		result.node = unordered_hash_find(key, m_buckets.first, (size_t)(m_buckets.last - m_buckets.first));
		return result;
	}

	template<typename Key, typename Value, typename Alloc>
	inline typename unordered_map<Key, Value, Alloc>::const_iterator unordered_map<Key, Value, Alloc>::find(const Key& key) const {
		iterator result;
		result.node = unordered_hash_find(key, m_buckets.first, (size_t)(m_buckets.last - m_buckets.first));
		return result;
	}

	template<typename Key, typename Value, typename Alloc>
	inline pair<typename unordered_map<Key, Value, Alloc>::iterator, bool> unordered_map<Key, Value, Alloc>::insert(const pair<Key, Value>& p) {
		pair<iterator, bool> result;
		result.second = false;

		result.first = find(p.first);
		if (result.first.node != 0)
			return result;
		
		unordered_hash_node<Key, Value>* newnode = new(placeholder(), Alloc::static_allocate(sizeof(unordered_hash_node<Key, Value>))) unordered_hash_node<Key, Value>(p.first, p.second);
		newnode->next = newnode->prev = 0;

		const size_t nbuckets = (size_t)(m_buckets.last - m_buckets.first);
		unordered_hash_node_insert(newnode, hash(p.first), m_buckets.first, nbuckets - 1);

		++m_size;
		if (m_size + 1 > 4 * nbuckets) {
			pointer root = *m_buckets.first;
			
			const size_t newnbuckets = ((size_t)(m_buckets.last - m_buckets.first) - 1) * 8;
			m_buckets.last = m_buckets.first;
			buffer_resize<pointer, Alloc>(&m_buckets, newnbuckets + 1, 0);
			unordered_hash_node<Key, Value>** buckets = m_buckets.first;

			while (root) {
				const pointer next = root->next;
				root->next = root->prev = 0;
				unordered_hash_node_insert(root, hash(root->first), buckets, newnbuckets);
				root = next;
			}
		}

		result.first.node = newnode;
		result.second = true;
		return result;
	}

	template<typename Key, typename Value, typename Alloc>
	void unordered_map<Key, Value, Alloc>::erase(const_iterator where) {
		unordered_hash_node_erase(where.node, hash(where->first), m_buckets.first, (size_t)(m_buckets.last - m_buckets.first) - 1);

		where->~unordered_hash_node<Key, Value>();
		Alloc::static_deallocate((void*)where.node, sizeof(unordered_hash_node<Key, Value>));
		--m_size;
	}

	template<typename Key, typename Value, typename Alloc>
	Value& unordered_map<Key, Value, Alloc>::operator[](const Key& key) {
		return insert(pair<Key, Value>(key, Value())).first->second;
	}

	template<typename Key, typename Value, typename Alloc>
	void unordered_map<Key, Value, Alloc>::swap(unordered_map& other) {
		size_t tsize = other.m_size;
		other.m_size = m_size, m_size = tsize;
		buffer_swap(&m_buckets, &other.m_buckets);
	}
}
#endif
