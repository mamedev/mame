// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nl_convert.h
 *
 */

#pragma once

#ifndef NL_CONVERT_H_
#define NL_CONVERT_H_

#include <cstddef>
#include <cstdarg>

#include "plib/pstring.h"
#include "plib/plists.h"

/*-------------------------------------------------
    convert - convert a spice netlist
-------------------------------------------------*/

class nl_convert_base_t
{
public:

	nl_convert_base_t() {};
	virtual ~nl_convert_base_t()
	{
		m_nets.clear_and_free();
		devs.clear_and_free();
		m_pins.clear_and_free();
	}

	const pstringbuffer &result() { return m_buf; }
	virtual void convert(const pstring &contents) = 0;

protected:

	void out(const char *format, ...) ATTR_PRINTF(2,3)
	{
		va_list ap;
		va_start(ap, format);
		m_buf += pstring(format).vprintf(ap);
		va_end(ap);
	}

	void add_pin_alias(const pstring &devname, const pstring &name, const pstring &alias)
	{
		m_pins.add(palloc(sp_pin_alias_t, devname + "." + name, devname + "." + alias), false);
	}

	void add_ext_alias(const pstring &alias)
	{
		m_ext_alias.add(alias);
	}

	void add_device(const pstring &atype, const pstring &aname, const pstring &amodel)
	{
		devs.add(palloc(sp_dev_t, atype, aname, amodel), false);
	}
	void add_device(const pstring &atype, const pstring &aname, double aval)
	{
		devs.add(palloc(sp_dev_t, atype, aname, aval), false);
	}
	void add_device(const pstring &atype, const pstring &aname)
	{
		devs.add(palloc(sp_dev_t, atype, aname), false);
	}

	void add_term(pstring netname, pstring termname)
	{
		sp_net_t * net = m_nets.find_by_name(netname);
		if (net == NULL)
		{
			net = palloc(sp_net_t, netname);
			m_nets.add(net, false);
		}

		/* if there is a pin alias, translate ... */
		sp_pin_alias_t *alias = m_pins.find_by_name(termname);

		if (alias != NULL)
			net->terminals().add(alias->alias());
		else
			net->terminals().add(termname);
	}

	void dump_nl()
	{
		for (int i=0; i<m_ext_alias.size(); i++)
		{
			sp_net_t *net = m_nets.find_by_name(m_ext_alias[i]);
			// use the first terminal ...
			out("ALIAS(%s, %s)\n", m_ext_alias[i].cstr(), net->terminals()[0].cstr());
			// if the aliased net only has this one terminal connected ==> don't dump
			if (net->terminals().size() == 1)
				net->set_no_export();
		}
		for (int i=0; i<devs.size(); i++)
		{
			if (devs[i]->has_value())
				out("%s(%s, %s)\n", devs[i]->type().cstr(),
						devs[i]->name().cstr(), get_nl_val(devs[i]->value()).cstr());
			else if (devs[i]->has_model())
				out("%s(%s, \"%s\")\n", devs[i]->type().cstr(),
						devs[i]->name().cstr(), devs[i]->model().cstr());
			else
				out("%s(%s)\n", devs[i]->type().cstr(),
						devs[i]->name().cstr());
		}
		// print nets
		for (int i=0; i<m_nets.size(); i++)
		{
			sp_net_t * net = m_nets[i];
			if (!net->is_no_export())
			{
				//printf("Net %s\n", net->name().cstr());
				out("NET_C(%s", net->terminals()[0].cstr() );
				for (int j=1; j<net->terminals().size(); j++)
				{
					out(", %s", net->terminals()[j].cstr() );
				}
				out(")\n");
			}
		}
		devs.clear_and_free();
		m_nets.clear_and_free();
		m_pins.clear_and_free();
		m_ext_alias.clear();
	}

	const pstring get_nl_val(const double val)
	{
		{
			int i = 0;
			while (m_sp_units[i].sp_unit != "-" )
			{
				if (m_sp_units[i].mult <= nl_math::abs(val))
					break;
				i++;
			}
			return pstring::sprintf(m_sp_units[i].nl_func.cstr(), val / m_sp_units[i].mult);
		}
	}
	double get_sp_unit(const pstring &unit)
	{
		int i = 0;
		while (m_sp_units[i].sp_unit != "-")
		{
			if (m_sp_units[i].sp_unit == unit)
				return m_sp_units[i].mult;
			i++;
		}
		fprintf(stderr, "Unit %s unknown\n", unit.cstr());
		return 0.0;
	}

	double get_sp_val(const pstring &sin)
	{
		int p = sin.len() - 1;
		while (p>=0 && (sin.substr(p,1) < "0" || sin.substr(p,1) > "9"))
			p--;
		pstring val = sin.substr(0,p + 1);
		pstring unit = sin.substr(p + 1);

		double ret = get_sp_unit(unit) * val.as_double();
		//printf("<%s> %s %d ==> %f\n", sin.cstr(), unit.cstr(), p, ret);
		return ret;
	}

private:
	struct sp_net_t
	{
	public:
		sp_net_t(const pstring &aname)
		: m_name(aname), m_no_export(false) {}

		const pstring &name() { return m_name;}
		pstring_list_t &terminals() { return m_terminals; }
		void set_no_export() { m_no_export = true; }
		bool is_no_export() { return m_no_export; }

	private:
		pstring m_name;
		bool m_no_export;
		pstring_list_t m_terminals;
	};

	struct sp_dev_t
	{
	public:
		sp_dev_t(const pstring &atype, const pstring &aname, const pstring &amodel)
		: m_type(atype), m_name(aname), m_model(amodel), m_val(0), m_has_val(false)
		{}

		sp_dev_t(const pstring &atype, const pstring &aname, double aval)
		: m_type(atype), m_name(aname), m_model(""), m_val(aval), m_has_val(true)
		{}

		sp_dev_t(const pstring &atype, const pstring &aname)
		: m_type(atype), m_name(aname), m_model(""), m_val(0.0), m_has_val(false)
		{}

		const pstring &name() { return m_name;}
		const pstring &type() { return m_type;}
		const pstring &model() { return m_model;}
		const double &value() { return m_val;}

		bool has_model() { return m_model != ""; }
		bool has_value() { return m_has_val; }

	private:
		pstring m_type;
		pstring m_name;
		pstring m_model;
		double m_val;
		bool m_has_val;
	};

	struct sp_unit {
		pstring sp_unit;
		pstring nl_func;
		double mult;
	};

	struct sp_pin_alias_t
	{
	public:
		sp_pin_alias_t(const pstring &name, const pstring &alias)
		: m_name(name), m_alias(alias)
		{}
		const pstring &name() { return m_name; }
		const pstring &alias() { return m_alias; }
	private:
		pstring m_name;
		pstring m_alias;
	};




private:

	pstringbuffer m_buf;

	pnamedlist_t<sp_dev_t *> devs;
	pnamedlist_t<sp_net_t *> m_nets;
	plist_t<pstring> m_ext_alias;
	pnamedlist_t<sp_pin_alias_t *> m_pins;

	static sp_unit m_sp_units[];

};

nl_convert_base_t::sp_unit nl_convert_base_t::m_sp_units[] = {
		{"T",   "",      1.0e12 },
		{"G",   "",      1.0e9  },
		{"MEG", "RES_M(%g)", 1.0e6  },
		{"k",   "RES_K(%g)", 1.0e3  }, /* eagle */
		{"K",   "RES_K(%g)", 1.0e3  },
		{"",    "%g",        1.0e0  },
		{"M",   "CAP_M(%g)", 1.0e-3 },
		{"u",   "CAP_U(%g)", 1.0e-6 }, /* eagle */
		{"U",   "CAP_U(%g)", 1.0e-6 },
		{"µ",   "CAP_U(%g)", 1.0e-6	},
		{"N",   "CAP_N(%g)", 1.0e-9 },
		{"P",   "CAP_P(%g)", 1.0e-12},
		{"F",   "%ge-15",    1.0e-15},

		{"MIL", "%e",  25.4e-6},

		{"-",   "%g",  1.0  }
};

class nl_convert_spice_t : public nl_convert_base_t
{
public:

	nl_convert_spice_t() : nl_convert_base_t() {};
	~nl_convert_spice_t()
	{
	}

	void convert(const pstring &contents)
	{
		pstring_list_t spnl(contents, "\n");

		// Add gnd net

		// FIXME: Parameter
		out("NETLIST_START(dummy)\n");
		add_term("0", "GND");

		pstring line = "";

		for (std::size_t i=0; i < spnl.size(); i++)
		{
			// Basic preprocessing
			pstring inl = spnl[i].trim().ucase();
			if (inl.startsWith("+"))
				line = line + inl.substr(1);
			else
			{
				process_line(line);
				line = inl;
			}
		}
		process_line(line);
		dump_nl();
		// FIXME: Parameter
		out("NETLIST_END()\n");
	}

protected:

	void process_line(const pstring &line)
	{
		if (line != "")
		{
			pstring_list_t tt(line, " ", true);
			double val = 0.0;
			switch (tt[0].cstr()[0])
			{
				case ';':
					out("// %s\n", line.substr(1).cstr());
					break;
				case '*':
					out("// %s\n", line.substr(1).cstr());
					break;
				case '.':
					if (tt[0].equals(".SUBCKT"))
					{
						out("NETLIST_START(%s)\n", tt[1].cstr());
						for (int i=2; i<tt.size(); i++)
							add_ext_alias(tt[i]);
					}
					else if (tt[0].equals(".ENDS"))
					{
						dump_nl();
						out("NETLIST_END()\n");
					}
					else
						out("// %s\n", line.cstr());
					break;
				case 'Q':
				{
					bool cerr = false;
					/* check for fourth terminal ... should be numeric net
					 * including "0" or start with "N" (ltspice)
					 */
					// FIXME: we need a is_long method ..
					ATTR_UNUSED int nval =tt[4].as_long(&cerr);
					if ((!cerr || tt[4].startsWith("N")) && tt.size() > 5)
						add_device("QBJT", tt[0], tt[5]);
					else
						add_device("QBJT", tt[0], tt[4]);
					add_term(tt[1], tt[0] + ".C");
					add_term(tt[2], tt[0] + ".B");
					add_term(tt[3], tt[0] + ".E");
				}
					break;
				case 'R':
					val = get_sp_val(tt[3]);
					add_device("RES", tt[0], val);
					add_term(tt[1], tt[0] + ".1");
					add_term(tt[2], tt[0] + ".2");
					break;
				case 'C':
					val = get_sp_val(tt[3]);
					add_device("CAP", tt[0], val);
					add_term(tt[1], tt[0] + ".1");
					add_term(tt[2], tt[0] + ".2");
					break;
				case 'V':
					// just simple Voltage sources ....
					if (tt[2].equals("0"))
					{
						val = get_sp_val(tt[3]);
						add_device("ANALOG_INPUT", tt[0], val);
						add_term(tt[1], tt[0] + ".Q");
						//add_term(tt[2], tt[0] + ".2");
					}
					else
						fprintf(stderr, "Voltage Source %s not connected to GND\n", tt[0].cstr());
					break;
				case 'D':
					// FIXME: Rewrite resistor value
					add_device("DIODE", tt[0], tt[3]);
					add_term(tt[1], tt[0] + ".A");
					add_term(tt[2], tt[0] + ".K");
					break;
				case 'U':
				case 'X':
				{
					// FIXME: specific code for KICAD exports
					//        last element is component type
					// FIXME: Parameter

					pstring xname = tt[0].replace(".", "_");
					pstring tname = "TTL_" + tt[tt.size()-1] + "_DIP";
					add_device(tname, xname);
					for (int i=1; i < tt.size() - 1; i++)
					{
						pstring term = pstring::sprintf("%s.%d", xname.cstr(), i);
						add_term(tt[i], term);
					}
					break;
				}
				default:
					out("// IGNORED %s: %s\n", tt[0].cstr(), line.cstr());
			}
		}
	}

private:

};

class nl_convert_eagle_t : public nl_convert_base_t
{
public:

	nl_convert_eagle_t() : nl_convert_base_t() {};
	~nl_convert_eagle_t()
	{
	}

	class eagle_tokenizer : public ptokenizer
	{

	public:
		eagle_tokenizer(nl_convert_eagle_t &convert)
		: ptokenizer(), m_convert(convert)
		{
			set_identifier_chars("abcdefghijklmnopqrstuvwvxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890_.-");
			set_number_chars(".0123456789", "0123456789eE-."); //FIXME: processing of numbers
			char ws[5];
			ws[0] = ' ';
			ws[1] = 9;
			ws[2] = 10;
			ws[3] = 13;
			ws[4] = 0;
			set_whitespace(ws);
			/* FIXME: gnetlist doesn't print comments */
			set_comment("/*", "*/", "//");
			set_string_char('\'');
			m_tok_ADD = register_token("ADD");
			m_tok_VALUE = register_token("VALUE");
			m_tok_SIGNAL = register_token("SIGNAL");
			m_tok_SEMICOLON = register_token(";");
			/* currently not used, but required for parsing */
			register_token(")");
			register_token("(");
		}

		token_id_t m_tok_ADD;
		token_id_t m_tok_VALUE;
		token_id_t m_tok_SIGNAL;
		token_id_t m_tok_SEMICOLON;

	protected:

		void verror(pstring msg, int line_num, pstring line)
		{
			m_convert.out("%s (line %d): %s\n", msg.cstr(), line_num, line.cstr());
		}


	private:
		nl_convert_eagle_t &m_convert;
	};

	void convert(const pstring &contents)
	{
		eagle_tokenizer tok(*this);
		tok.reset(contents.cstr());

		out("NETLIST_START(dummy)\n");
		add_term("GND", "GND");
		add_term("VCC", "VCC");
		eagle_tokenizer::token_t token = tok.get_token();
		while (true)
		{
			if (token.is_type(eagle_tokenizer::ENDOFFILE))
			{
				dump_nl();
				// FIXME: Parameter
				out("NETLIST_END()\n");
				return;
			}
			else if (token.is(tok.m_tok_SEMICOLON))
			{
				/* ignore empty statements */
				token = tok.get_token();
			}
			else if (token.is(tok.m_tok_ADD))
			{
				pstring name = tok.get_string();
				/* skip to semicolon */
				do
				{
					token = tok.get_token();
				} while (!token.is(tok.m_tok_SEMICOLON));
				token = tok.get_token();
				pstring sval = "";
				if (token.is(tok.m_tok_VALUE))
				{
					pstring vname = tok.get_string();
					sval = tok.get_string();
					tok.require_token(tok.m_tok_SEMICOLON);
					token = tok.get_token();
				}
				switch (name.cstr()[0])
				{
					case 'Q':
					{
						add_device("QBJT", name, sval);
					}
						break;
					case 'R':
						{
							double val = get_sp_val(sval);
							add_device("RES", name, val);
						}
						break;
					case 'C':
						{
							double val = get_sp_val(sval);
							add_device("CAP", name, val);
						}
						break;
					case 'P':
						if (sval.ucase() == "HIGH")
							add_device("TTL_INPUT", name, 1);
						else if (sval.ucase() == "LOW")
							add_device("TTL_INPUT", name, 0);
						else
							add_device("ANALOG_INPUT", name, sval.as_double());
						add_pin_alias(name, "1", "Q");
						break;
					case 'D':
						/* Pin 1 = Anode, Pin 2 = Cathode */
						add_device("DIODE", name, sval);
						add_pin_alias(name, "1", "A");
						add_pin_alias(name, "2", "K");
						break;
					case 'U':
					case 'X':
					{
						pstring tname = "TTL_" + sval + "_DIP";
						add_device(tname, name);
						break;
					}
					default:
						tok.error("// IGNORED %s\n", name.cstr());
				}

			}
			else if (token.is(tok.m_tok_SIGNAL))
			{
				pstring netname = tok.get_string();
				token = tok.get_token();
				while (!token.is(tok.m_tok_SEMICOLON))
				{
					/* fixme: should check for string */
					pstring devname = token.str();
					pstring pin = tok.get_string();
					add_term(netname, devname + "." + pin);
					token = tok.get_token();				}
			}
			else
			{
				out("Unexpected %s\n", token.str().cstr());
				return;
			}
		}

	}

protected:


private:

};

#endif /* NL_CONVERT_H_ */
