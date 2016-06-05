// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * poptions.h
 *
 */

#pragma once

#ifndef POPTIONS_H_
#define POPTIONS_H_

#include <cstddef>

#include "pstring.h"
#include "plists.h"

namespace plib {

/***************************************************************************
    Options
***************************************************************************/

class options;

class option
{
public:
	option()
	: m_short(""), m_long(""), m_help(""), m_has_argument(false)
	{}

	option(pstring ashort, pstring along, pstring help, bool has_argument, options *parent = nullptr);

	virtual ~option()
	{
	}

	/* no_argument options will be called with "" argument */

	virtual int parse(ATTR_UNUSED pstring argument) { return 0; }

	pstring m_short;
	pstring m_long;
	pstring m_help;
	bool m_has_argument;
private:
};

class option_str : public option
{
public:
	option_str(pstring ashort, pstring along, pstring defval, pstring help, options *parent = nullptr)
	: option(ashort, along, help, true, parent), m_val(defval)
	{}

	virtual int parse(pstring argument) override { m_val = argument; return 0; }

	pstring operator ()() { return m_val; }
private:
	pstring m_val;
};

class option_str_limit : public option
{
public:
	option_str_limit(pstring ashort, pstring along, pstring defval, pstring limit, pstring help, options *parent = nullptr)
	: option(ashort, along, help, true, parent), m_val(defval), m_limit(limit, ":")
	{}

	virtual int parse(pstring argument) override
	{
		if (m_limit.contains(argument))
		{
			m_val = argument;
			return 0;
		}
		else
			return 1;
	}

	pstring operator ()() { return m_val; }
private:
	pstring m_val;
	pstring_vector_t m_limit;
};

class option_bool : public option
{
public:
	option_bool(pstring ashort, pstring along, pstring help, options *parent = nullptr)
	: option(ashort, along, help, false, parent), m_val(false)
	{}

	virtual int parse(ATTR_UNUSED pstring argument) override { m_val = true; return 0; }

	bool operator ()() { return m_val; }
private:
	bool m_val;
};

class option_double : public option
{
public:
	option_double(pstring ashort, pstring along, double defval, pstring help, options *parent = nullptr)
	: option(ashort, along, help, true, parent), m_val(defval)
	{}

	virtual int parse(pstring argument) override
	{
		bool err = false;
		m_val = argument.as_double(&err);
		return (err ? 1 : 0);
	}

	double operator ()() { return m_val; }
private:
	double m_val;
};

class options
{
public:

	options() {}

	options(option *o[])
	{
		int i=0;
		while (o[i] != nullptr)
		{
			m_opts.push_back(o[i]);
			i++;
		}
	}

	~options()
	{
		m_opts.clear();
	}

	void register_option(option *opt)
	{
		m_opts.push_back(opt);
	}

	int parse(int argc, char *argv[])
	{
		m_app = argv[0];

		for (int i=1; i<argc; )
		{
			pstring arg(argv[i]);
			option *opt = nullptr;

			if (arg.startsWith("--"))
			{
				opt = getopt_long(arg.substr(2));
			}
			else if (arg.startsWith("-"))
			{
				opt = getopt_short(arg.substr(1));
			}
			else
				return i;
			if (opt == nullptr)
				return i;
			if (opt->m_has_argument)
			{
				i++; // FIXME: are there more arguments?
				if (opt->parse(argv[i]) != 0)
					return i - 1;
			}
			else
				opt->parse("");
			i++;
		}
		return argc;
	}

	pstring help()
	{
		pstring ret;

		for (auto & opt : m_opts )
		{
			pstring line = "";
			if (opt->m_short != "")
				line += "  -" + opt->m_short;
			if (opt->m_long != "")
			{
				if (line != "")
					line += ", ";
				else
					line = "     ";
				line += "--" + opt->m_long;
			}
			line = line.rpad(" ", 20).cat(opt->m_help);
			ret = ret + line + "\n";
		}
		return ret;
	}
	pstring app() { return m_app; }

private:

	option *getopt_short(pstring arg)
	{
		for (auto & opt : m_opts)
		{
			if (opt->m_short == arg)
				return opt;
		}
		return nullptr;
	}
	option *getopt_long(pstring arg)
	{
		for (auto & opt : m_opts)
		{
			if (opt->m_long == arg)
				return opt;
		}
		return nullptr;
	}

	pvector_t<option *> m_opts;
	pstring m_app;
};

option::option(pstring ashort, pstring along, pstring help, bool has_argument, options *parent)
: m_short(ashort), m_long(along), m_help(help), m_has_argument(has_argument)
{
	if (parent != nullptr)
		parent->register_option(this);
}

}

#endif /* POPTIONS_H_ */
