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

/***************************************************************************
    Options
***************************************************************************/

class poptions;

class poption
{
public:
	poption()
	: m_short(""), m_long(""), m_help(""), m_has_argument(false)
	{}

	poption(pstring ashort, pstring along, pstring help, bool has_argument, poptions *parent = NULL);

	virtual ~poption()
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

class poption_str : public poption
{
public:
	poption_str(pstring ashort, pstring along, pstring defval, pstring help, poptions *parent = NULL)
	: poption(ashort, along, help, true, parent), m_val(defval)
	{}

	virtual int parse(pstring argument) { m_val = argument; return 0; }

	pstring operator ()() { return m_val; }
private:
	pstring m_val;
};

class poption_str_limit : public poption
{
public:
	poption_str_limit(pstring ashort, pstring along, pstring defval, pstring limit, pstring help, poptions *parent = NULL)
	: poption(ashort, along, help, true, parent), m_val(defval), m_limit(limit, ":")
	{}

	virtual int parse(pstring argument)
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
	pstring_list_t m_limit;
};

class poption_bool : public poption
{
public:
	poption_bool(pstring ashort, pstring along, pstring help, poptions *parent = NULL)
	: poption(ashort, along, help, false, parent), m_val(false)
	{}

	virtual int parse(ATTR_UNUSED pstring argument) { m_val = true; return 0; }

	bool operator ()() { return m_val; }
private:
	bool m_val;
};

class poption_double : public poption
{
public:
	poption_double(pstring ashort, pstring along, double defval, pstring help, poptions *parent = NULL)
	: poption(ashort, along, help, true, parent), m_val(defval)
	{}

	virtual int parse(pstring argument)
	{
		bool err = false;
		m_val = argument.as_double(&err);
		return (err ? 1 : 0);
	}

	double operator ()() { return m_val; }
private:
	double m_val;
};

class poptions
{
public:

	poptions() {}

	poptions(poption *o[])
	{
		int i=0;
		while (o[i] != NULL)
		{
			m_opts.add(o[i]);
			i++;
		}
	}

	~poptions()
	{
		m_opts.clear();
	}

	void register_option(poption *opt)
	{
		m_opts.add(opt);
	}

	int parse(int argc, char *argv[])
	{
		m_app = argv[0];

		for (int i=1; i<argc; )
		{
			pstring arg(argv[i]);
			poption *opt = NULL;

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
			if (opt == NULL)
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

		for (std::size_t i=0; i<m_opts.size(); i++ )
		{
			poption *opt = m_opts[i];
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

	poption *getopt_short(pstring arg)
	{
		for (std::size_t i=0; i < m_opts.size(); i++)
		{
			if (m_opts[i]->m_short == arg)
				return m_opts[i];
		}
		return NULL;
	}
	poption *getopt_long(pstring arg)
	{
		for (std::size_t i=0; i < m_opts.size(); i++)
		{
			if (m_opts[i]->m_long == arg)
				return m_opts[i];
		}
		return NULL;
	}

	plist_t<poption *> m_opts;
	pstring m_app;
};

poption::poption(pstring ashort, pstring along, pstring help, bool has_argument, poptions *parent)
: m_short(ashort), m_long(along), m_help(help), m_has_argument(has_argument)
{
	if (parent != NULL)
		parent->register_option(this);
}


#endif /* POPTIONS_H_ */
