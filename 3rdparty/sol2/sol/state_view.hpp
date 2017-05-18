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

#ifndef SOL_STATE_VIEW_HPP
#define SOL_STATE_VIEW_HPP

#include "error.hpp"
#include "table.hpp"
#include "load_result.hpp"
#include <memory>

namespace sol {
	enum class lib : char {
		base,
		package,
		coroutine,
		string,
		os,
		math,
		table,
		debug,
		bit32,
		io,
		ffi,
		jit,
		utf8,
		count
	};

	inline std::size_t total_memory_used(lua_State* L) {
		std::size_t kb = lua_gc(L, LUA_GCCOUNT, 0);
		kb *= 1024;
		kb += lua_gc(L, LUA_GCCOUNTB, 0);
		return kb;
	}

	class state_view {
	private:
		lua_State* L;
		table reg;
		global_table global;

		optional<object> is_loaded_package(const std::string& key) {
			auto loaded = reg.traverse_get<optional<object>>("_LOADED", key);
			bool is53mod = loaded && !(loaded->is<bool>() && !loaded->as<bool>());
			if (is53mod)
				return loaded;
#if SOL_LUA_VERSION <= 501
			auto loaded51 = global.traverse_get<optional<object>>("package", "loaded", key);
			bool is51mod = loaded51 && !(loaded51->is<bool>() && !loaded51->as<bool>());
			if (is51mod)
				return loaded51;
#endif
			return nullopt;
		}

		template <typename T>
		void ensure_package(const std::string& key, T&& sr) {
#if SOL_LUA_VERSION <= 501
			auto pkg = global["package"];
			if (!pkg.valid()) {
				pkg = create_table_with("loaded", create_table_with(key, sr));
			}
			else {
				auto ld = pkg["loaded"];
				if (!ld.valid()) {
					ld = create_table_with(key, sr);
				}
				else {
					ld[key] = sr;
				}
			}
#endif
			auto loaded = reg["_LOADED"];
			if (!loaded.valid()) {
				loaded = create_table_with(key, sr);
			}
			else {
				loaded[key] = sr;
			}
		}

		template <typename Fx>
		object require_core(const std::string& key, Fx&& action, bool create_global = true) {
			optional<object> loaded = is_loaded_package(key);
			if (loaded && loaded->valid())
				return std::move(*loaded);
			action();
			auto sr = stack::get<stack_reference>(L);
			if (create_global)
				set(key, sr);
			ensure_package(key, sr);
			return stack::pop<object>(L);
		}

	public:
		typedef global_table::iterator iterator;
		typedef global_table::const_iterator const_iterator;

		state_view(lua_State* Ls) :
			L(Ls),
			reg(Ls, LUA_REGISTRYINDEX),
			global(Ls, detail::global_) {

		}

		state_view(this_state Ls) : state_view(Ls.L){

		}

		lua_State* lua_state() const {
			return L;
		}

		template<typename... Args>
		void open_libraries(Args&&... args) {
			static_assert(meta::all_same<lib, Args...>::value, "all types must be libraries");
			if (sizeof...(args) == 0) {
				luaL_openlibs(L);
				return;
			}

			lib libraries[1 + sizeof...(args)] = { lib::count, std::forward<Args>(args)... };

			for (auto&& library : libraries) {
				switch (library) {
#if SOL_LUA_VERSION <= 501 && defined(SOL_LUAJIT)
				case lib::coroutine:
#endif // luajit opens coroutine base stuff
				case lib::base:
					luaL_requiref(L, "base", luaopen_base, 1);
					lua_pop(L, 1);
					break;
				case lib::package:
					luaL_requiref(L, "package", luaopen_package, 1);
					lua_pop(L, 1);
					break;
#if !defined(SOL_LUAJIT)
				case lib::coroutine:
#if SOL_LUA_VERSION > 501
					luaL_requiref(L, "coroutine", luaopen_coroutine, 1);
					lua_pop(L, 1);
#endif // Lua 5.2+ only
					break;
#endif // Not LuaJIT - comes builtin
				case lib::string:
					luaL_requiref(L, "string", luaopen_string, 1);
					lua_pop(L, 1);
					break;
				case lib::table:
					luaL_requiref(L, "table", luaopen_table, 1);
					lua_pop(L, 1);
					break;
				case lib::math:
					luaL_requiref(L, "math", luaopen_math, 1);
					lua_pop(L, 1);
					break;
				case lib::bit32:
#ifdef SOL_LUAJIT
					luaL_requiref(L, "bit32", luaopen_bit, 1);
					lua_pop(L, 1);
#elif (SOL_LUA_VERSION == 502) || defined(LUA_COMPAT_BITLIB)  || defined(LUA_COMPAT_5_2)
					luaL_requiref(L, "bit32", luaopen_bit32, 1);
					lua_pop(L, 1);
#else
#endif // Lua 5.2 only (deprecated in 5.3 (503)) (Can be turned on with Compat flags)
					break;
				case lib::io:
					luaL_requiref(L, "io", luaopen_io, 1);
					lua_pop(L, 1);
					break;
				case lib::os:
					luaL_requiref(L, "os", luaopen_os, 1);
					lua_pop(L, 1);
					break;
				case lib::debug:
					luaL_requiref(L, "debug", luaopen_debug, 1);
					lua_pop(L, 1);
					break;
				case lib::utf8:
#if SOL_LUA_VERSION > 502 && !defined(SOL_LUAJIT)
					luaL_requiref(L, "utf8", luaopen_utf8, 1);
					lua_pop(L, 1);
#endif // Lua 5.3+ only
					break;
				case lib::ffi:
#ifdef SOL_LUAJIT
					luaL_requiref(L, "ffi", luaopen_ffi, 1);
					lua_pop(L, 1);
#endif // LuaJIT only
					break;
				case lib::jit:
#ifdef SOL_LUAJIT
					luaL_requiref(L, "jit", luaopen_jit, 1);
					lua_pop(L, 1);
#endif // LuaJIT Only
					break;
				case lib::count:
				default:
					break;
				}
			}
		}

		object require(const std::string& key, lua_CFunction open_function, bool create_global = true) {
			luaL_requiref(L, key.c_str(), open_function, create_global ? 1 : 0);
			return stack::pop<object>(L);
		}

		object require_script(const std::string& key, const std::string& code, bool create_global = true) {
			return require_core(key, [this, &code]() {stack::script(L, code); }, create_global);
		}

		object require_file(const std::string& key, const std::string& filename, bool create_global = true) {
			return require_core(key, [this, &filename]() {stack::script_file(L, filename); }, create_global);
		}

		protected_function_result do_string(const std::string& code) {
			sol::protected_function pf = load(code);
			return pf();
		}

		protected_function_result do_file(const std::string& filename) {
			sol::protected_function pf = load_file(filename);
			return pf();
		}

		function_result script(const std::string& code) {
			int index = lua_gettop(L);
			stack::script(L, code);
			int postindex = lua_gettop(L);
			int returns = postindex - index;
			return function_result(L, (std::max)(postindex - (returns - 1), 1), returns);
		}

		function_result script_file(const std::string& filename) {
			int index = lua_gettop(L);
			stack::script_file(L, filename);
			int postindex = lua_gettop(L);
			int returns = postindex - index;
			return function_result(L, (std::max)(postindex - (returns - 1), 1), returns);
		}

		load_result load(const std::string& code) {
			load_status x = static_cast<load_status>(luaL_loadstring(L, code.c_str()));
			return load_result(L, lua_absindex(L, -1), 1, 1, x);
		}

		load_result load_file(const std::string& filename) {
			load_status x = static_cast<load_status>(luaL_loadfile(L, filename.c_str()));
			return load_result(L, lua_absindex(L, -1), 1, 1, x);
		}

		load_result load_buffer(const char *buff, size_t size, const char *name, const char* mode = nullptr) {
			load_status x = static_cast<load_status>(luaL_loadbufferx(L, buff, size, name, mode));
			return load_result(L, lua_absindex(L, -1), 1, 1, x);
		}

		iterator begin() const {
			return global.begin();
		}

		iterator end() const {
			return global.end();
		}

		const_iterator cbegin() const {
			return global.cbegin();
		}

		const_iterator cend() const {
			return global.cend();
		}

		global_table globals() const {
			return global;
		}

		table registry() const {
			return reg;
		}

		std::size_t memory_used() const {
			return total_memory_used(lua_state());
		}

		void collect_garbage() {
			lua_gc(lua_state(), LUA_GCCOLLECT, 0);
		}

		operator lua_State* () const {
			return lua_state();
		}

		void set_panic(lua_CFunction panic) {
			lua_atpanic(L, panic);
		}

		template<typename... Args, typename... Keys>
		decltype(auto) get(Keys&&... keys) const {
			return global.get<Args...>(std::forward<Keys>(keys)...);
		}

		template<typename T, typename Key>
		decltype(auto) get_or(Key&& key, T&& otherwise) const {
			return global.get_or(std::forward<Key>(key), std::forward<T>(otherwise));
		}

		template<typename T, typename Key, typename D>
		decltype(auto) get_or(Key&& key, D&& otherwise) const {
			return global.get_or<T>(std::forward<Key>(key), std::forward<D>(otherwise));
		}

		template<typename... Args>
		state_view& set(Args&&... args) {
			global.set(std::forward<Args>(args)...);
			return *this;
		}

		template<typename T, typename... Keys>
		decltype(auto) traverse_get(Keys&&... keys) const {
			return global.traverse_get<T>(std::forward<Keys>(keys)...);
		}

		template<typename... Args>
		state_view& traverse_set(Args&&... args) {
			global.traverse_set(std::forward<Args>(args)...);
			return *this;
		}

		template<typename T>
		state_view& set_usertype(usertype<T>& user) {
			return set_usertype(usertype_traits<T>::name(), user);
		}

		template<typename Key, typename T>
		state_view& set_usertype(Key&& key, usertype<T>& user) {
			global.set_usertype(std::forward<Key>(key), user);
			return *this;
		}

		template<typename Class, typename... Args>
		state_view& new_usertype(const std::string& name, Args&&... args) {
			global.new_usertype<Class>(name, std::forward<Args>(args)...);
			return *this;
		}

		template<typename Class, typename CTor0, typename... CTor, typename... Args>
		state_view& new_usertype(const std::string& name, Args&&... args) {
			global.new_usertype<Class, CTor0, CTor...>(name, std::forward<Args>(args)...);
			return *this;
		}

		template<typename Class, typename... CArgs, typename... Args>
		state_view& new_usertype(const std::string& name, constructors<CArgs...> ctor, Args&&... args) {
			global.new_usertype<Class>(name, ctor, std::forward<Args>(args)...);
			return *this;
		}

		template<typename Class, typename... Args>
		state_view& new_simple_usertype(const std::string& name, Args&&... args) {
			global.new_simple_usertype<Class>(name, std::forward<Args>(args)...);
			return *this;
		}

		template<typename Class, typename CTor0, typename... CTor, typename... Args>
		state_view& new_simple_usertype(const std::string& name, Args&&... args) {
			global.new_simple_usertype<Class, CTor0, CTor...>(name, std::forward<Args>(args)...);
			return *this;
		}
		
		template<typename Class, typename... CArgs, typename... Args>
		state_view& new_simple_usertype(const std::string& name, constructors<CArgs...> ctor, Args&&... args) {
			global.new_simple_usertype<Class>(name, ctor, std::forward<Args>(args)...);
			return *this;
		}

		template<typename Class, typename... Args>
		simple_usertype<Class> create_simple_usertype(Args&&... args) {
			return global.create_simple_usertype<Class>(std::forward<Args>(args)...);
		}

		template<typename Class, typename CTor0, typename... CTor, typename... Args>
		simple_usertype<Class> create_simple_usertype(Args&&... args) {
			return global.create_simple_usertype<Class, CTor0, CTor...>(std::forward<Args>(args)...);
		}

		template<typename Class, typename... CArgs, typename... Args>
		simple_usertype<Class> create_simple_usertype(constructors<CArgs...> ctor, Args&&... args) {
			return global.create_simple_usertype<Class>(ctor, std::forward<Args>(args)...);
		}

		template<bool read_only = true, typename... Args>
		state_view& new_enum(const std::string& name, Args&&... args) {
			global.new_enum<read_only>(name, std::forward<Args>(args)...);
			return *this;
		}

		template <typename Fx>
		void for_each(Fx&& fx) {
			global.for_each(std::forward<Fx>(fx));
		}

		template<typename T>
		proxy<global_table&, T> operator[](T&& key) {
			return global[std::forward<T>(key)];
		}

		template<typename T>
		proxy<const global_table&, T> operator[](T&& key) const {
			return global[std::forward<T>(key)];
		}

		template<typename Sig, typename... Args, typename Key>
		state_view& set_function(Key&& key, Args&&... args) {
			global.set_function<Sig>(std::forward<Key>(key), std::forward<Args>(args)...);
			return *this;
		}

		template<typename... Args, typename Key>
		state_view& set_function(Key&& key, Args&&... args) {
			global.set_function(std::forward<Key>(key), std::forward<Args>(args)...);
			return *this;
		}

		template <typename Name>
		table create_table(Name&& name, int narr = 0, int nrec = 0) {
			return global.create(std::forward<Name>(name), narr, nrec);
		}

		template <typename Name, typename Key, typename Value, typename... Args>
		table create_table(Name&& name, int narr, int nrec, Key&& key, Value&& value, Args&&... args) {
			return global.create(std::forward<Name>(name), narr, nrec, std::forward<Key>(key), std::forward<Value>(value), std::forward<Args>(args)...);
		}

		template <typename Name, typename... Args>
		table create_named_table(Name&& name, Args&&... args) {
			table x = global.create_with(std::forward<Args>(args)...);
			global.set(std::forward<Name>(name), x);
			return x;
		}

		table create_table(int narr = 0, int nrec = 0) {
			return create_table(lua_state(), narr, nrec);
		}

		template <typename Key, typename Value, typename... Args>
		table create_table(int narr, int nrec, Key&& key, Value&& value, Args&&... args) {
			return create_table(lua_state(), narr, nrec, std::forward<Key>(key), std::forward<Value>(value), std::forward<Args>(args)...);
		}

		template <typename... Args>
		table create_table_with(Args&&... args) {
			return create_table_with(lua_state(), std::forward<Args>(args)...);
		}

		static inline table create_table(lua_State* L, int narr = 0, int nrec = 0) {
			return global_table::create(L, narr, nrec);
		}

		template <typename Key, typename Value, typename... Args>
		static inline table create_table(lua_State* L, int narr, int nrec, Key&& key, Value&& value, Args&&... args) {
			return global_table::create(L, narr, nrec, std::forward<Key>(key), std::forward<Value>(value), std::forward<Args>(args)...);
		}

		template <typename... Args>
		static inline table create_table_with(lua_State* L, Args&&... args) {
			return global_table::create_with(L, std::forward<Args>(args)...);
		}
	};
} // sol

#endif // SOL_STATE_VIEW_HPP
