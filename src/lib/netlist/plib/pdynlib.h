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

	class dynlib_base
	{
	public:
		explicit dynlib_base() : m_is_loaded(false) { }

		virtual ~dynlib_base() = default;

		PCOPYASSIGN(dynlib_base, delete)
		PMOVEASSIGN(dynlib_base, default)

		bool isLoaded() const { return m_is_loaded; }

		template <typename T>
		T getsym(const pstring &name) const noexcept
		{
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			return reinterpret_cast<T>(getsym_p(name));
		}

	protected:
		void set_loaded(bool v) noexcept { m_is_loaded = v; }
		virtual void *getsym_p(const pstring &name) const noexcept = 0;
	private:
		bool m_is_loaded;
	};

	class dynlib : public dynlib_base
	{
	public:
		explicit dynlib(const pstring &libname);
		dynlib(const pstring &path, const pstring &libname);

		~dynlib() override;

		PCOPYASSIGN(dynlib, delete)
		PMOVEASSIGN(dynlib, default)

	protected:
		void *getsym_p(const pstring &name) const noexcept override;

	private:
		void *m_lib;
	};

	struct dynlib_static_sym
	{
		const char *name;
		void       *addr;
	};

	class dynlib_static : public dynlib_base
	{
	public:
		explicit dynlib_static(const dynlib_static_sym *syms)
		: m_syms(syms)
		{
			if (syms != nullptr)
				set_loaded(true);
		}

	protected:
		void *getsym_p(const pstring &name) const noexcept override
		{
			const dynlib_static_sym *p = m_syms;
			while (p->name[0] != 0)
			{
				if (name == pstring(p->name))
					return p->addr;
				p++;
			}
			return nullptr;
		}

	private:
		const dynlib_static_sym *m_syms;
	};

	template <typename R, typename... Args>
	class dynproc
	{
	public:
		using calltype = R(*) (Args... args);

		dynproc() : m_sym(nullptr) { }

		dynproc(dynlib_base &dl, const pstring &name) noexcept
		: m_sym(dl.getsym<calltype>(name))
		{
		}

		void load(dynlib_base &dl, const pstring &name) noexcept
		{
			m_sym = dl.getsym<calltype>(name);
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

} // namespace plib

#endif // PDYNLIB_H_
