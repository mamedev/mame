// The MIT License (MIT) 

// Copyright (c) 2013-2016 Rapptz, ThePhD and contributors

// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef SOL_VARIADIC_ARGS_HPP
#define SOL_VARIADIC_ARGS_HPP

#include "stack.hpp"
#include "stack_proxy.hpp"
#include <limits>
#include <iterator>

namespace sol {
	template <bool is_const>
	struct va_iterator : std::iterator<std::random_access_iterator_tag, std::conditional_t<is_const, const stack_proxy, stack_proxy>, std::ptrdiff_t, std::conditional_t<is_const, const stack_proxy*, stack_proxy*>, std::conditional_t<is_const, const stack_proxy, stack_proxy>> {
		typedef std::iterator<std::random_access_iterator_tag, std::conditional_t<is_const, const stack_proxy, stack_proxy>, std::ptrdiff_t, std::conditional_t<is_const, const stack_proxy*, stack_proxy*>, std::conditional_t<is_const, const stack_proxy, stack_proxy>> base_t;
		typedef typename base_t::reference reference;
		typedef typename base_t::pointer pointer;
		typedef typename base_t::value_type value_type;
		typedef typename base_t::difference_type difference_type;
		typedef typename base_t::iterator_category iterator_category;
		lua_State* L;
		int index;
		int stacktop;
		stack_proxy sp;

		va_iterator() : L(nullptr), index((std::numeric_limits<int>::max)()), stacktop((std::numeric_limits<int>::max)()) {}
		va_iterator(lua_State* L, int index, int stacktop) : L(L), index(index), stacktop(stacktop), sp(L, index) {}

		reference operator*() {
			return stack_proxy(L, index);
		}

		pointer operator->() {
			sp = stack_proxy(L, index);
			return &sp;
		}

		va_iterator& operator++ () {
			++index;
			return *this;
		}

		va_iterator operator++ (int) {
			auto r = *this;
			this->operator ++();
			return r;
		}

		va_iterator& operator-- () {
			--index;
			return *this;
		}

		va_iterator operator-- (int) {
			auto r = *this;
			this->operator --();
			return r;
		}

		va_iterator& operator+= (difference_type idx) {
			index += static_cast<int>(idx);
			return *this;
		}

		va_iterator& operator-= (difference_type idx) {
			index -= static_cast<int>(idx);
			return *this;
		}

		difference_type operator- (const va_iterator& r) const {
			return index - r.index;
		}

		va_iterator operator+ (difference_type idx) const {
			va_iterator r = *this;
			r += idx;
			return r;
		}

		reference operator[](difference_type idx) {
			return stack_proxy(L, index + static_cast<int>(idx));
		}

		bool operator==(const va_iterator& r) const {
			if (stacktop == (std::numeric_limits<int>::max)()) {
				return r.index == r.stacktop;
			}
			else if (r.stacktop == (std::numeric_limits<int>::max)()) {
				return index == stacktop;
			}
			return index == r.index;
		}

		bool operator != (const va_iterator& r) const {
			return !(this->operator==(r));
		}

		bool operator < (const va_iterator& r) const {
			return index < r.index;
		}

		bool operator > (const va_iterator& r) const {
			return index > r.index;
		}

		bool operator <= (const va_iterator& r) const {
			return index <= r.index;
		}

		bool operator >= (const va_iterator& r) const {
			return index >= r.index;
		}
	};

	template <bool is_const>
	inline va_iterator<is_const> operator+(typename va_iterator<is_const>::difference_type n, const va_iterator<is_const>& r) {
		return r + n;
	}

	struct variadic_args {
	private:
		lua_State* L;
		int index;
		int stacktop;

	public:
		typedef stack_proxy reference_type;
		typedef stack_proxy value_type;
		typedef stack_proxy* pointer;
		typedef std::ptrdiff_t difference_type;
		typedef std::size_t size_type;
		typedef va_iterator<false> iterator;
		typedef va_iterator<true> const_iterator;
		typedef std::reverse_iterator<iterator> reverse_iterator;
		typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

		variadic_args() = default;
		variadic_args(lua_State* L, int index = -1) : L(L), index(lua_absindex(L, index)), stacktop(lua_gettop(L)) {}
		variadic_args(const variadic_args&) = default;
		variadic_args& operator=(const variadic_args&) = default;
		variadic_args(variadic_args&& o) : L(o.L), index(o.index), stacktop(o.stacktop) {
			// Must be manual, otherwise destructor will screw us
			// return count being 0 is enough to keep things clean
			// but will be thorough
			o.L = nullptr;
			o.index = 0;
			o.stacktop = 0;
		}
		variadic_args& operator=(variadic_args&& o) {
			L = o.L;
			index = o.index;
			stacktop = o.stacktop;
			// Must be manual, otherwise destructor will screw us
			// return count being 0 is enough to keep things clean
			// but will be thorough
			o.L = nullptr;
			o.index = 0;
			o.stacktop = 0;
			return *this;
		}

		iterator begin() { return iterator(L, index, stacktop + 1); }
		iterator end() { return iterator(L, stacktop + 1, stacktop + 1); }
		const_iterator begin() const { return const_iterator(L, index, stacktop + 1); }
		const_iterator end() const { return const_iterator(L, stacktop + 1, stacktop + 1); }
		const_iterator cbegin() const { return begin(); }
		const_iterator cend() const { return end(); }

		reverse_iterator rbegin() { return std::reverse_iterator<iterator>(begin()); }
		reverse_iterator rend() { return std::reverse_iterator<iterator>(end()); }
		const_reverse_iterator rbegin() const { return std::reverse_iterator<const_iterator>(begin()); }
		const_reverse_iterator rend() const { return std::reverse_iterator<const_iterator>(end()); }
		const_reverse_iterator crbegin() const { return std::reverse_iterator<const_iterator>(cbegin()); }
		const_reverse_iterator crend() const { return std::reverse_iterator<const_iterator>(cend()); }

		int push() const {
			int pushcount = 0;
			for (int i = index; i <= stacktop; ++i) {
				lua_pushvalue(L, i);
				pushcount += 1;
			}
			return pushcount;
		}

		template<typename T>
		decltype(auto) get(difference_type start = 0) const {
			return stack::get<T>(L, index + static_cast<int>(start));
		}

		stack_proxy operator[](difference_type start) const {
			return stack_proxy(L, index + static_cast<int>(start));
		}

		lua_State* lua_state() const { return L; };
		int stack_index() const { return index; };
		int leftover_count() const { return stacktop - (index - 1); }
		int top() const { return stacktop; }
	};

	namespace stack {
		template <>
		struct getter<variadic_args> {
			static variadic_args get(lua_State* L, int index, record& tracking) {
				tracking.last = 0;
				return variadic_args(L, index);
			}
		};

		template <>
		struct pusher<variadic_args> {
			static int push(lua_State*, const variadic_args& ref) {
				return ref.push();
			}
		};
	} // stack
} // sol

#endif // SOL_VARIADIC_ARGS_HPP
