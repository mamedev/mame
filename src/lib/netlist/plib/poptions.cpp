// license:BSD-3-Clause
// copyright-holders:Couriersud

#include "poptions.h"
#include "pexception.h"
#include "pstrutil.h"
#include "ptypes.h"

namespace plib {

	option_base::option_base(options &parent, const pstring &help)
	: m_help(help)
	{
		parent.register_option(this);
	}

	option::option(options &parent, const pstring &ashort, const pstring &along, const pstring &help, bool has_argument)
	: option_base(parent, help), m_short(ashort), m_long(along),
	  m_has_argument(has_argument), m_specified(false)
	{
	}

	int option_str::parse(const pstring &argument)
	{
		m_val = argument;
		return 0;
	}

	int option_bool::parse([[maybe_unused]] const pstring &argument)
	{
		m_val = true;
		return 0;
	}

	int option_vec::parse(const pstring &argument)
	{
		bool err = false;
		m_val.push_back(argument);
		return (err ? 1 : 0);
	}

	options::options()
	: m_other_args(nullptr)
	{
	}

	options::options(option **o)
	: m_other_args(nullptr)
	{
		int i=0;
		while (o[i] != nullptr)
		{
			register_option(o[i]);
			i++;
		}
	}

	void options::register_option(option_base *opt)
	{
		m_opts.push_back(opt);
	}

	void options::check_consistency()
	{
		for (auto &opt : m_opts)
		{
			auto *o = dynamic_cast<option *>(opt);
			if (o != nullptr)
			{
				if (o->short_opt().empty() && o->long_opt().empty())
				{
					auto *ov = dynamic_cast<option_args *>(o);
					if (ov != nullptr)
					{
						if (m_other_args != nullptr)
						{
							throw pexception("other args can only be specified once!");
						}

						m_other_args = ov;
					}
					else
						throw pexception("found option with neither short or long tag!" );
				}
			}
		}
	}

	std::size_t options::parse(const std::vector<putf8string> &argv)
	{
		check_consistency();
		m_app = argv[0];
		bool seen_other_args = false;

		for (std::size_t i=1; i < argv.size(); )
		{
			pstring arg(argv[i]);
			option *opt = nullptr;
			pstring opt_arg;
			bool has_equal_arg = false;

			if (!seen_other_args && plib::startsWith(arg, "--"))
			{
				auto v = psplit(arg.substr(2),'=');
				if (!v.empty() && !v[0].empty())
				{
					opt = getopt_long(v[0]);
					has_equal_arg = (v.size() > 1);
					if (has_equal_arg)
					{
						for (std::size_t j = 1; j < v.size() - 1; j++)
							opt_arg += (v[j] + "=");
						opt_arg += v[v.size()-1];
					}
				}
				else
				{
					opt = m_other_args;
					seen_other_args = true;
				}
			}
			else if (!seen_other_args && plib::startsWith(arg, "-"))
			{
				std::size_t p = 1;
				opt = getopt_short(arg.substr(p, 1));
				++p;
				if (p < arg.length())
				{
					has_equal_arg = true;
					opt_arg = arg.substr(p);
				}
			}
			else
			{
				seen_other_args = true;
				if (m_other_args == nullptr)
					return i;
				opt = m_other_args;
				i--; // we haven't had an option specifier;
			}
			if (opt == nullptr)
				return i;
			if (opt->has_argument())
			{
				if (has_equal_arg)
				{
					if (opt->do_parse(opt_arg) != 0)
						return i;
				}
				else
				{
					i++;
					if (i >= argv.size())
						return i - 1;
					if (opt->do_parse(pstring(argv[i])) != 0)
						return i - 1;
				}
			}
			else
			{
				if (has_equal_arg)
					return i;
				opt->do_parse("");
			}
			i++;
		}
		return argv.size();
	}

	pstring options::split_paragraphs(const pstring &text, unsigned width, unsigned indent,
			unsigned firstline_indent, const pstring &line_end)
	{
		auto paragraphs = psplit(text,'\n');
		pstring ret("");

		for (auto &p : paragraphs)
		{
			pstring line = plib::rpad(pstring(""), pstring(" "), firstline_indent);
			for (auto &s : psplit(p, ' '))
			{
				if (line.length() + s.length() > width)
				{
					ret += line + line_end;
					line = plib::rpad(pstring(""), pstring(" "), indent);
				}
				line += s + " ";
			}
			ret += line;
			if (p != paragraphs.back())
				ret += line_end;
		}
		return ret;
	}

	pstring options::help(const pstring &description, const pstring &usage,
			unsigned width, unsigned indent) const
	{
		pstring ret;

		ret = split_paragraphs(description, width, 0, 0) + "\n\n";
		ret += "Usage:\t" + usage + "\n\nOptions:\n\n";

		for (const auto & optbase : m_opts )
		{
			// Skip anonymous inputs which are collected in option_args
			if (dynamic_cast<option_args *>(optbase) != nullptr)
				continue;

			if (auto * const opt = dynamic_cast<option *>(optbase))
			{
				pstring line = "";
				if (!opt->short_opt().empty())
					line += "  -" + opt->short_opt();
				if (!opt->long_opt().empty())
				{
					if (!line.empty())
						line += ", ";
					else
						line = "      ";
					line += "--" + opt->long_opt();
					if (opt->has_argument())
					{
						line += "=";
						auto *ol = dynamic_cast<option_str_limit_base *>(opt);
						if (ol != nullptr)
						{
							for (const auto &v : ol->limit())
							{
								line += v + "|";
							}
							line = plib::left(line, line.length() - 1);
						}
						else
							line += "Value";
					}
				}
				line = plib::rpad(line, pstring(" "), indent - 2) + "  ";
				if (line.length() > indent)
				{
					//ret += "TestGroup abc\n  def gef\nxyz\n\n" ;
					ret += line + "\n";
					ret += split_paragraphs(opt->help(), width, indent, indent) + "\n";
				}
				else
					ret += split_paragraphs(line + opt->help(), width, indent, 0) + "\n";
			}
			else if (auto *grp = dynamic_cast<option_group *>(optbase))
			{
				ret += "\n" + grp->group() + ":\n";
				if (!grp->help().empty())
					ret += split_paragraphs(grp->help(), width, 4, 4) + "\n\n";
			}
		}
		// FIXME: other help ...
		pstring ex("");
		for (const auto & optbase : m_opts )
		{
			if (auto *example = dynamic_cast<option_example *>(optbase))
			{
				// help2man doesn't like \\ line continuation in output
				ex += split_paragraphs(example->example(), width, 8, 0, "\n") + "\n\n";
				ex += split_paragraphs(example->help(), width, 4, 4) + "\n\n";
			}
		}
		if (!ex.empty())
		{
			ret += "\n\nExamples:\n\n" + ex;
		}
		return ret;
	}

	option *options::getopt_short(const pstring &arg) const
	{
		for (const auto & optbase : m_opts)
		{
			auto *opt = dynamic_cast<option *>(optbase);
			if (opt != nullptr && !arg.empty() && opt->short_opt() == arg)
				return opt;
		}
		return nullptr;
	}
	option *options::getopt_long(const pstring &arg) const
	{
		for (const auto & optbase : m_opts)
		{
			auto *opt = dynamic_cast<option *>(optbase);
			if (opt != nullptr && !arg.empty() && opt->long_opt() == arg)
				return opt;
		}
		return nullptr;
	}

} // namespace plib

