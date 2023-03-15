// license:BSD-3-Clause
// copyright-holders:Couriersud

#ifndef PSOURCE_H_
#define PSOURCE_H_

///
/// \file putil.h
///

#include "palloc.h"
#include "pexception.h"
#include "pstream.h"
#include "pstring.h"

#include <algorithm>
#include <initializer_list>
#include <sstream>
#include <vector>


#define PSOURCELOC() plib::source_location(__FILE__, __LINE__)

namespace plib
{

	/// \brief Source code locations.
	///
	/// The c++20 draft for source locations is based on const char * strings.
	/// It is thus only suitable for c++ source code and not for programmatic
	/// parsing of files. This class is a replacement for dynamic use cases.
	///
	struct source_location
	{
		source_location() noexcept
		: m_file("unknown"), m_func(m_file), m_line(0), m_col(0)
		{ }

		source_location(pstring file, unsigned line) noexcept
		: m_file(std::move(file)), m_func("unknown"), m_line(line), m_col(0)
		{ }

		source_location(pstring file, pstring func, unsigned line) noexcept
		: m_file(std::move(file)), m_func(std::move(func)), m_line(line), m_col(0)
		{ }

		PCOPYASSIGNMOVE(source_location, default)

		~source_location() = default;

		unsigned line() const noexcept { return m_line; }
		unsigned column() const noexcept { return m_col; }
		pstring file_name() const noexcept { return m_file; }
		pstring function_name() const noexcept { return m_func; }

		source_location &operator ++() noexcept
		{
			++m_line;
			return *this;
		}

	private:
		pstring m_file;
		pstring m_func;
		unsigned m_line;
		unsigned m_col;
	};

	/// \brief Base source class.
	///
	/// Pure virtual class all other source implementations are based on.
	/// Sources provide an abstraction to read input from a variety of
	/// sources, e.g. files, memory, remote locations.
	///
	class psource_t
	{
	public:

		psource_t() noexcept = default;

		PCOPYASSIGNMOVE(psource_t, delete)

		virtual ~psource_t() noexcept = default;

		virtual istream_uptr stream(const pstring &name) = 0;
	private:
	};

	/// \brief Generic string source.
	///
	/// Will return the given string when name matches.
	/// Is used in preprocessor code to eliminate inclusion of certain files.
	///
	class psource_str_t : public psource_t
	{
	public:
		psource_str_t(pstring name, pstring str)
		: m_name(std::move(name)), m_str(std::move(str))
		{}

		PCOPYASSIGNMOVE(psource_str_t, delete)
		~psource_str_t() noexcept override = default;

		istream_uptr stream(const pstring &name) override
		{
			if (name == m_name)
				return {std::make_unique<std::stringstream>(putf8string(m_str)), name };

			return istream_uptr();
		}
	private:
		pstring m_name;
		pstring m_str;
	};

	/// \brief Generic sources collection.
	///
	class psource_collection_t
	{
	public:
		using source_ptr = std::unique_ptr<psource_t>;
		using list_t = std::vector<source_ptr>;

		psource_collection_t() noexcept = default;

		PCOPYASSIGNMOVE(psource_collection_t, delete)
		virtual ~psource_collection_t() noexcept = default;

		template <typename S, typename... Args>
		void add_source(Args&&... args)
		{
			static_assert(std::is_base_of<psource_t, S>::value, "S must inherit from plib::psource_t");

			auto src = std::make_unique<S>(std::forward<Args>(args)...);
			m_collection.push_back(std::move(src));
		}

		template <typename S = psource_t>
		istream_uptr get_stream(pstring name)
		{
			for (auto &s : m_collection)
			{
				if (auto source = plib::dynamic_downcast<S *>(s.get()))
				{
					auto strm = (*source)->stream(name);
					if (!strm.empty())
						return strm;
				}
			}
			return istream_uptr();
		}

		template <typename S, typename F>
		bool for_all(F lambda)
		{
			for (auto &s : m_collection)
			{
				if (auto source = plib::dynamic_downcast<S *>(s.get()))
				{
					if (lambda(*source))
						return true;
				}
			}
			return false;
		}

	private:
		list_t m_collection;
	};

} // namespace plib

#endif // PSOURCE_H_
