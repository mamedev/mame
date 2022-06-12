// license:BSD-3-Clause
// copyright-holders:Couriersud

#ifndef POPTIONS_H_
#define POPTIONS_H_

///
/// \file poptions.h
///

#include "pfmtlog.h"
#include "pstonum.h"
#include "pstring.h"
#include "putil.h"

namespace plib {

	/// \brief options base class
	///
	class options;

	class option_base
	{
	public:
		option_base(options &parent, const pstring &help);
		virtual ~option_base() = default;

		PCOPYASSIGNMOVE(option_base, delete)

		virtual pstring help() const { return m_help; }
	private:
		pstring m_help;
	};

	class option_group : public option_base
	{
	public:
		option_group(options &parent, const pstring &group, const pstring &help)
		: option_base(parent, help), m_group(group) { }

		pstring group() const { return m_group; }
	private:
		pstring m_group;
	};

	class option_example : public option_base
	{
	public:
		option_example(options &parent, const pstring &group, const pstring &help)
		: option_base(parent, help), m_example(group) { }

		pstring example() const { return m_example; }
	private:
		pstring m_example;
	};


	class option : public option_base
	{
	public:
		option(options &parent, const pstring &short_opt, const pstring &long_opt, const pstring &help, bool has_argument);

		// no_argument options will be called with "" argument

		pstring short_opt() const { return m_short; }
		pstring long_opt() const { return m_long; }
		bool has_argument() const { return m_has_argument ; }
		bool was_specified() const { return m_specified; }

		int do_parse(const pstring &argument)
		{
			m_specified = true;
			return parse(argument);
		}

	protected:
		virtual int parse(const pstring &argument) = 0;

	private:
		pstring m_short;
		pstring m_long;
		bool m_has_argument;
		bool m_specified;
	};

	class option_str : public option
	{
	public:
		option_str(options &parent, const pstring &short_opt, const pstring &long_opt, const pstring &default_value, const pstring &help)
		: option(parent, short_opt, long_opt, help, true), m_val(default_value)
		{}

		pstring operator ()() const { return m_val; }

	protected:
		int parse(const pstring &argument) override;

	private:
		pstring m_val;
	};

	class option_str_limit_base : public option
	{
	public:
		option_str_limit_base(options &parent, const pstring &short_opt, const pstring &long_opt, std::vector<pstring> &&limit, const pstring &help)
		: option(parent, short_opt, long_opt, help, true)
		, m_limit(limit)
		{
		}
		const std::vector<pstring> &limit() const { return m_limit; }

	protected:

	private:
		std::vector<pstring> m_limit;
	};


	template <typename T>
	class option_str_limit : public option_str_limit_base
	{
	public:
		option_str_limit(options &parent, const pstring &short_opt, const pstring &long_opt, const T &default_value, std::vector<pstring> &&limit, const pstring &help)
		: option_str_limit_base(parent, short_opt, long_opt, std::move(limit), help), m_val(default_value)
		{
		}

		T operator ()() const { return m_val; }

		pstring as_string() const { return limit()[m_val]; }

	protected:
		int parse(const pstring &argument) override
		{
			auto raw = plib::container::indexof(limit(), argument);

			if (raw != plib::container::npos)
			{
				m_val = narrow_cast<T>(raw);
				return 0;
			}

			return 1;
		}

	private:
		T m_val;
	};

	class option_bool : public option
	{
	public:
		option_bool(options &parent, const pstring &short_opt, const pstring &long_opt, const pstring &help)
		: option(parent, short_opt, long_opt, help, false), m_val(false)
		{}

		bool operator ()() const { return m_val; }

	protected:
		int parse(const pstring &argument) override;

	private:
		bool m_val;
	};

	template <typename T>
	class option_num : public option
	{
	public:
		option_num(options &parent, const pstring &short_opt, const pstring &long_opt, T default_value,
				const pstring &help,
				T min_val = std::numeric_limits<T>::lowest(),
				T max_val = std::numeric_limits<T>::max() )
		: option(parent, short_opt, long_opt, help, true)
		, m_val(default_value)
		, m_min(min_val)
		, m_max(max_val)
		, m_def(default_value)
		{}

		T operator ()() const { return m_val; }

		pstring help() const override
		{
			auto hs(option::help());
			return plib::pfmt(hs)(m_def, m_min, m_max);
		}

	protected:
		int parse(const pstring &argument) override
		{
			bool err(false);
			m_val = pstonum_ne<T>(argument, err);
			return (err ? 1 : (m_val < m_min || m_val > m_max));
		}

	private:
		T m_val;
		T m_min;
		T m_max;
		T m_def;
	};

	class option_vec : public option
	{
	public:
		option_vec(options &parent, const pstring &short_opt, const pstring &long_opt, const pstring &help)
		: option(parent, short_opt, long_opt, help, true)
		{}

		const std::vector<pstring> &operator ()() const { return m_val; }

	protected:
		int parse(const pstring &argument) override;

	private:
		std::vector<pstring> m_val;
	};

	class option_args : public option_vec
	{
	public:
		option_args(options &parent, const pstring &help)
		: option_vec(parent, "", "", help)
		{}
	};

	class options
	{
	public:

		options();
		explicit options(option **o);

		~options() = default;

		PCOPYASSIGNMOVE(options, delete)

		void register_option(option_base *opt);
		std::size_t parse(const std::vector<putf8string> &argv);

		pstring help(const pstring &description, const pstring &usage,
				unsigned width = 72, unsigned indent = 20) const;

		pstring app() const { return m_app; }

	private:
		static pstring split_paragraphs(const pstring &text, unsigned width, unsigned indent,
				unsigned first_line_indent, const pstring &line_end = "\n");

		void check_consistency() noexcept(false);

		template <typename T>
		T *getopt_type() const
		{
			for (const auto & base_class : m_options )
			{
				if (auto opt = dynamic_cast<T *>(base_class))
					return opt;
			}
		return nullptr;
	}

		option *getopt_short(const pstring &arg) const;
		option *getopt_long(const pstring &arg) const;

		std::vector<option_base *> m_options;
		pstring m_app;
		option_args * m_other_args;
};

} // namespace plib

#endif // POPTIONS_H_
