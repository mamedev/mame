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
	option();
	option(options &parent, pstring ashort, pstring along, pstring help, bool has_argument);

	virtual ~option();

	/* no_argument options will be called with "" argument */

	virtual int parse(ATTR_UNUSED pstring argument) = 0;

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

	virtual int parse(pstring argument) override;

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

	virtual int parse(pstring argument) override;

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

	virtual int parse(ATTR_UNUSED pstring argument) override;

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

	virtual int parse(pstring argument) override;

	double operator ()() { return m_val; }
private:
	double m_val;
};

class options
{
public:

	options();
	explicit options(option *o[]);

	~options();

	void register_option(option *opt);
	int parse(int argc, char *argv[]);

	pstring help(pstring description, pstring usage,
			unsigned width = 72, unsigned ident = 20);

	pstring app() { return m_app; }

private:
	static pstring split_paragraphs(pstring text, unsigned width, unsigned ident,
			unsigned firstline_ident);

	option *getopt_short(pstring arg);
	option *getopt_long(pstring arg);

	std::vector<option *> m_opts;
	pstring m_app;
};


}

#endif /* POPTIONS_H_ */
