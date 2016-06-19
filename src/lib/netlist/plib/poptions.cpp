// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * poptions.cpp
 *
 */

#include "poptions.h"

namespace plib {

/***************************************************************************
    Options
***************************************************************************/

	option::option()
	: m_short(""), m_long(""), m_help(""), m_has_argument(false)
	{}

	option::option(options &parent, pstring ashort, pstring along, pstring help, bool has_argument)
	: m_short(ashort), m_long(along), m_help(help), m_has_argument(has_argument)
	{
		parent.register_option(this);
	}

	option::~option()
	{
	}

	int option_str::parse(pstring argument)
	{
		m_val = argument;
		return 0;
	}

	int option_str_limit::parse(pstring argument)
	{
		if (plib::container::contains(m_limit, argument))
		{
			m_val = argument;
			return 0;
		}
		else
			return 1;
	}

	int option_bool::parse(ATTR_UNUSED pstring argument)
	{
		m_val = true;
		return 0;
	}

	int option_double::parse(pstring argument)
	{
		bool err = false;
		m_val = argument.as_double(&err);
		return (err ? 1 : 0);
	}

	options::options()
	{

	}

	options::options(option *o[])
	{
		int i=0;
		while (o[i] != nullptr)
		{
			m_opts.push_back(o[i]);
			i++;
		}
	}

	options::~options()
	{
		m_opts.clear();
	}

	void options::register_option(option *opt)
	{
		m_opts.push_back(opt);
	}

	int options::parse(int argc, char *argv[])
	{
		m_app = argv[0];

		for (int i=1; i<argc; )
		{
			pstring arg(argv[i]);
			option *opt = nullptr;
			pstring opt_arg;
			bool has_equal_arg = false;

			if (arg.startsWith("--"))
			{
				auto v = pstring_vector_t(arg.substr(2),"=");
				opt = getopt_long(v[0]);
				has_equal_arg = (v.size() > 1);
				if (has_equal_arg) opt_arg = v[1];
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
				if (has_equal_arg)
				{
					if (opt->parse(opt_arg) != 0)
						return i;
				}
				else
				{
					i++; // FIXME: are there more arguments?
					if (opt->parse(argv[i]) != 0)
						return i - 1;
				}
			}
			else
			{
				if (has_equal_arg)
					return i;
				opt->parse("");
			}
			i++;
		}
		return argc;
	}

	pstring options::split_paragraphs(pstring text, unsigned width, unsigned ident,
			unsigned firstline_ident)
	{
		auto paragraphs = pstring_vector_t(text,"\n");
		pstring ret("");

		for (auto &p : paragraphs)
		{
			pstring line = pstring("").rpad(" ", firstline_ident);
			for (auto &s : pstring_vector_t(p, " "))
			{
				if (line.len() + s.len() > width)
				{
					ret += line + "\n";
					line = pstring("").rpad(" ", ident);
				}
				line += s + " ";
			}
			ret += line + "\n";
		}
		return ret;
	}

	pstring options::help(pstring description, pstring usage,
			unsigned width, unsigned ident)
	{
		pstring ret;

		ret = split_paragraphs(description, width, 0, 0) + "\n";
		ret += "Usage:\t" + usage + "\n\nOptions:\n";

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
				ret += split_paragraphs(opt->help(), 72, 21, 21);
			}
			else
				ret += split_paragraphs(line + opt->help(), 72, 21, 0);
		}
		return ret;
	}

	option *options::getopt_short(pstring arg)
	{
		for (auto & opt : m_opts)
		{
			if (opt->short_opt() == arg)
				return opt;
		}
		return nullptr;
	}
	option *options::getopt_long(pstring arg)
	{
		for (auto & opt : m_opts)
		{
			if (opt->long_opt() == arg)
				return opt;
		}
		return nullptr;
	}

} // namespace plib
