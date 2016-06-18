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
#include "putil.h"

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

	option(options &parent, pstring ashort, pstring along, pstring help, bool has_argument);

	virtual ~option()
	{
	}

	/* no_argument options will be called with "" argument */

	virtual int parse(ATTR_UNUSED pstring argument) { return 0; }

	pstring short_opt() { return m_short; }
	pstring long_opt() { return m_long; }
	pstring help() { return m_help; }
	bool has_argument() { return m_has_argument ; }
private:
	pstring m_short;
	pstring m_long;
	pstring m_help;
	bool m_has_argument;
};

class option_str : public option
{
public:
	option_str(options &parent, pstring ashort, pstring along, pstring defval, pstring help)
	: option(parent, ashort, along, help, true), m_val(defval)
	{}

	virtual int parse(pstring argument) override { m_val = argument; return 0; }

	pstring operator ()() { return m_val; }
private:
	pstring m_val;
};

class option_str_limit : public option
{
public:
	option_str_limit(options &parent, pstring ashort, pstring along, pstring defval, pstring limit, pstring help)
	: option(parent, ashort, along, help, true), m_val(defval), m_limit(limit, ":")
	{}

	virtual int parse(pstring argument) override
	{
		if (plib::container::contains(m_limit, argument))
		{
			m_val = argument;
			return 0;
		}
		else
			return 1;
	}

	pstring operator ()() { return m_val; }
	const plib::pstring_vector_t &limit() { return m_limit; }

private:
	pstring m_val;
	plib::pstring_vector_t m_limit;
};

class option_bool : public option
{
public:
	option_bool(options &parent, pstring ashort, pstring along, pstring help)
	: option(parent, ashort, along, help, false), m_val(false)
	{}

	virtual int parse(ATTR_UNUSED pstring argument) override { m_val = true; return 0; }

	bool operator ()() { return m_val; }
private:
	bool m_val;
};

class option_double : public option
{
public:
	option_double(options &parent, pstring ashort, pstring along, double defval, pstring help)
	: option(parent, ashort, along, help, true), m_val(defval)
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
			if (opt->has_argument())
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
			if (opt->short_opt() != "")
				line += "  -" + opt->short_opt();
			if (opt->long_opt() != "")
			{
				if (line != "")
					line += ", ";
				else
					line = "      ";
				line += "--" + opt->long_opt();
				if (opt->has_argument())
				{
					line += "=";
					option_str_limit *ol = dynamic_cast<option_str_limit *>(opt);
					if (ol)
					{
						for (auto &v : ol->limit())
						{
							line += v + "|";
						}
						line = line.left(line.len() - 1);
					}
					else
						line += "Value";
				}
			}
			line = line.rpad(" ", 20) + " ";
			if (line.len() > 21)
			{
				ret += line + "\n";
				line = pstring("").rpad(" ", 21);
			}
			for (auto &s : pstring_vector_t(opt->help(), " "))
			{
				if (line.len() + s.len() > 72)
				{
					ret += line + "\n";
					line = pstring("").rpad(" ", 21);
				}
				line += s + " ";
			}
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
			if (opt->short_opt() == arg)
				return opt;
		}
		return nullptr;
	}
	option *getopt_long(pstring arg)
	{
		for (auto & opt : m_opts)
		{
			if (opt->long_opt() == arg)
				return opt;
		}
		return nullptr;
	}

	std::vector<option *> m_opts;
	pstring m_app;
};

option::option(options &parent, pstring ashort, pstring along, pstring help, bool has_argument)
: m_short(ashort), m_long(along), m_help(help), m_has_argument(has_argument)
{
	parent.register_option(this);
}

}

#endif /* POPTIONS_H_ */
