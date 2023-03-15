// license:BSD-3-Clause
// copyright-holders:Couriersud

#ifndef PDYNLIB_H_
#define PDYNLIB_H_

///
/// \file pdynlib.h
///
/// Dynamic loading of libraries
///

#include "pstring.h"
#include "ptypes.h"

namespace plib {

	class dynamic_library_base
	{
	public:

		explicit dynamic_library_base() : m_is_loaded(false) { }

		virtual ~dynamic_library_base() = default;

		dynamic_library_base(const dynamic_library_base &) = delete;
		dynamic_library_base &operator=(const dynamic_library_base &) = delete;

		dynamic_library_base(dynamic_library_base &&) noexcept = default;
		dynamic_library_base &operator=(dynamic_library_base &&) noexcept = default;

		template <typename R, typename... Args>
		class function
		{
		public:
			using calltype = R(*) (Args... args);

			function() : m_sym(nullptr) { }

			function(dynamic_library_base &dl, const pstring &name) noexcept
			: m_sym(dl.get_symbol<calltype>(name))
			{
			}

			void load(dynamic_library_base &dl, const pstring &name) noexcept
			{
				m_sym = dl.get_symbol<calltype>(name);
			}

			R operator ()(Args&&... args) const
			{
				return m_sym(std::forward<Args>(args)...);
				//return m_sym(args...);
			}

			bool resolved() const noexcept { return m_sym != nullptr; }
		private:
			calltype m_sym;
		};

		bool isLoaded() const { return m_is_loaded; }

		template <typename T>
		T get_symbol(const pstring &name) const noexcept
		{
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			return reinterpret_cast<T>(get_symbol_pointer(name));
		}

	protected:
		void set_loaded(bool v) noexcept { m_is_loaded = v; }
		virtual void *get_symbol_pointer(const pstring &name) const noexcept = 0;
	private:
		bool m_is_loaded;
	};

	class dynamic_library : public dynamic_library_base
	{
	public:
		explicit dynamic_library(const pstring &libname);
		dynamic_library(const pstring &path, const pstring &libname);

		~dynamic_library() override;

		PCOPYASSIGN(dynamic_library, delete)
		PMOVEASSIGN(dynamic_library, default)

	protected:
		void *get_symbol_pointer(const pstring &name) const noexcept override;

	private:
		void *m_lib;
	};

	class static_library : public dynamic_library_base
	{
	public:
		struct symbol
		{
			const char *name;
			void       *addr;
		};


		explicit static_library(const symbol *symbols)
		: m_syms(symbols)
		{
			if (symbols != nullptr)
				set_loaded(true);
		}

	protected:
		void *get_symbol_pointer(const pstring &name) const noexcept override
		{
			const symbol *p = m_syms;
			while (p->name[0] != 0)
			{
				if (name == pstring(p->name))
					return p->addr;
				p++;
			}
			return nullptr;
		}

	private:
		const symbol *m_syms;
	};

} // namespace plib

#endif // PDYNLIB_H_
