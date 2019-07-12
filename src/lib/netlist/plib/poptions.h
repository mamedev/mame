// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * poptions.h
 *
 */

#pragma once

#ifndef POPTIONS_H_
#define POPTIONS_H_

#include "plists.h"
#include "pstring.h"
#include "putil.h"

namespace plib {
/***************************************************************************
    Options
***************************************************************************/

class options;

class option_base
{
public:
	option_base(options &parent, const pstring &help);
	virtual ~option_base() = default;

	COPYASSIGNMOVE(option_base, delete)

	pstring help() const { return m_help; }
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
	option(options &parent, const pstring &ashort, const pstring &along, const pstring &help, bool has_argument);

	/* no_argument options will be called with "" argument */

	pstring short_opt() { return m_short; }
	pstring long_opt() { return m_long; }
	bool has_argument() { return m_has_argument ; }
	bool was_specified() { return m_specified; }

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
	option_str(options &parent, const pstring &ashort, const pstring &along, const pstring &defval, const pstring &help)
	: option(parent, ashort, along, help, true), m_val(defval)
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
	option_str_limit_base(options &parent, const pstring &ashort, const pstring &along, std::vector<pstring> &&limit, const pstring &help)
	: option(parent, ashort, along, help, true)
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
	option_str_limit(options &parent, const pstring &ashort, const pstring &along, const T &defval, std::vector<pstring> &&limit, const pstring &help)
	: option_str_limit_base(parent, ashort, along, std::move(limit), help), m_val(defval)
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
			m_val = static_cast<T>(raw);
			return 0;
		}
		else
			return 1;
	}

private:
	T m_val;
};

class option_bool : public option
{
public:
	option_bool(options &parent, const pstring &ashort, const pstring &along, const pstring &help)
	: option(parent, ashort, along, help, false), m_val(false)
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
	option_num(options &parent, const pstring &ashort, const pstring &along, T defval,
			const pstring &help,
			T minval = std::numeric_limits<T>::min(),
			T maxval = std::numeric_limits<T>::max() )
	: option(parent, ashort, along, help, true)
	, m_val(defval)
	, m_min(minval)
	, m_max(maxval)
	{}

	T operator ()() const { return m_val; }

protected:
	int parse(const pstring &argument) override
	{
		bool err;
		m_val = pstonum_ne<T, true>(argument, err);
		return (err ? 1 : (m_val < m_min || m_val > m_max));
	}

private:
	T m_val;
	T m_min;
	T m_max;
};

class option_vec : public option
{
public:
	option_vec(options &parent, const pstring &ashort, const pstring &along, const pstring &help)
	: option(parent, ashort, along, help, true)
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

class options : public nocopyassignmove
{
public:

	options();
	explicit options(option **o);

	void register_option(option_base *opt);
	int parse(int argc, char **argv);

	pstring help(const pstring &description, const pstring &usage,
			unsigned width = 72, unsigned indent = 20) const;

	pstring app() const { return m_app; }

private:
	static pstring split_paragraphs(const pstring &text, unsigned width, unsigned indent,
			unsigned firstline_indent);

	void check_consistency();

	template <typename T>
	T *getopt_type() const
	{
		for (auto & optbase : m_opts )
		{
			if (auto opt = dynamic_cast<T *>(optbase))
				return opt;
		}
		return nullptr;
	}

	option *getopt_short(const pstring &arg) const;
	option *getopt_long(const pstring &arg) const;

	std::vector<option_base *> m_opts;
	pstring m_app;
	option_args * m_other_args;
};

} // namespace plib

#endif /* POPTIONS_H_ */
