// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nl_convert.c
 *
 */

#include <algorithm>
#include <cstdio>
#include <cmath>
#include "nl_convert.h"


template<typename Class>
static pvector_t<int> bubble(const pvector_t<Class *> &sl)
{
	pvector_t<int> ret;
	for (unsigned i=0; i<sl.size(); i++)
		ret.push_back(i);

	for(unsigned i=0; i < sl.size(); i++)
	{
		for(unsigned j=i+1; j < sl.size(); j++)
		{
			if(sl[ret[i]]->name() > sl[ret[j]]->name())
			{
				std::swap(ret[i], ret[j]);
			}
		}
	}
	return ret;
}

/*-------------------------------------------------
    convert - convert a spice netlist
-------------------------------------------------*/

void nl_convert_base_t::add_pin_alias(const pstring &devname, const pstring &name, const pstring &alias)
{
	pstring pname = devname + "." + name;
	m_pins.add(pname, palloc(pin_alias_t(pname, devname + "." + alias)));
}

void nl_convert_base_t::add_ext_alias(const pstring &alias)
{
	m_ext_alias.push_back(alias);
}

void nl_convert_base_t::add_device(dev_t *dev)
{
	for (auto & d : m_devs)
		if (d->name() == dev->name())
		{
			out("ERROR: Duplicate device {1} ignored.", dev->name());
			return;
		}
	m_devs.push_back(dev);
}

void nl_convert_base_t::add_device(const pstring &atype, const pstring &aname, const pstring &amodel)
{
	add_device(palloc(dev_t(atype, aname, amodel)));
}
void nl_convert_base_t::add_device(const pstring &atype, const pstring &aname, double aval)
{
	add_device(palloc(dev_t(atype, aname, aval)));
}
void nl_convert_base_t::add_device(const pstring &atype, const pstring &aname)
{
	add_device(palloc(dev_t(atype, aname)));
}

void nl_convert_base_t::add_term(pstring netname, pstring termname)
{
	net_t * net = NULL;
	if (m_nets.contains(netname))
		net = m_nets[netname];
	else
	{
		net = palloc(net_t(netname));
		m_nets.add(netname, net);
	}

	/* if there is a pin alias, translate ... */
	pin_alias_t *alias = m_pins[termname];

	if (alias != NULL)
		net->terminals().push_back(alias->alias());
	else
		net->terminals().push_back(termname);
}

void nl_convert_base_t::dump_nl()
{
	for (std::size_t i=0; i<m_ext_alias.size(); i++)
	{
		net_t *net = m_nets[m_ext_alias[i]];
		// use the first terminal ...
		out("ALIAS({}, {})\n", m_ext_alias[i].cstr(), net->terminals()[0].cstr());
		// if the aliased net only has this one terminal connected ==> don't dump
		if (net->terminals().size() == 1)
			net->set_no_export();
	}
	pvector_t<int> sorted = bubble(m_devs);
	for (std::size_t i=0; i<m_devs.size(); i++)
	{
		std::size_t j = sorted[i];

		if (m_devs[j]->has_value())
			out("{}({}, {})\n", m_devs[j]->type().cstr(),
					m_devs[j]->name().cstr(), get_nl_val(m_devs[j]->value()).cstr());
		else if (m_devs[j]->has_model())
			out("{}({}, \"{}\")\n", m_devs[j]->type().cstr(),
					m_devs[j]->name().cstr(), m_devs[j]->model().cstr());
		else
			out("{}({})\n", m_devs[j]->type().cstr(),
					m_devs[j]->name().cstr());
	}
	// print nets
	for (std::size_t i=0; i<m_nets.size(); i++)
	{
		net_t * net = m_nets.value_at(i);
		if (!net->is_no_export())
		{
			//printf("Net {}\n", net->name().cstr());
			out("NET_C({}", net->terminals()[0].cstr() );
			for (std::size_t j=1; j<net->terminals().size(); j++)
			{
				out(", {}", net->terminals()[j].cstr() );
			}
			out(")\n");
		}
	}
	m_devs.clear_and_free();
	for (std::size_t i = 0; i < m_nets.size(); i++)
		pfree(m_nets.value_at(i));
	m_nets.clear();
	for (std::size_t i = 0; i < m_pins.size(); i++)
		pfree(m_pins.value_at(i));
	m_pins.clear();
	m_ext_alias.clear();
}

const pstring nl_convert_base_t::get_nl_val(const double val)
{
	{
		int i = 0;
		while (m_units[i].m_unit != "-" )
		{
			if (m_units[i].m_mult <= std::abs(val))
				break;
			i++;
		}
		return pfmt(m_units[i].m_func.cstr())(val / m_units[i].m_mult);
	}
}
double nl_convert_base_t::get_sp_unit(const pstring &unit)
{
	int i = 0;
	while (m_units[i].m_unit != "-")
	{
		if (m_units[i].m_unit == unit)
			return m_units[i].m_mult;
		i++;
	}
	fprintf(stderr, "Unit %s unknown\n", unit.cstr());
	return 0.0;
}

double nl_convert_base_t::get_sp_val(const pstring &sin)
{
	int p = sin.len() - 1;
	while (p>=0 && (sin.substr(p,1) < "0" || sin.substr(p,1) > "9"))
		p--;
	pstring val = sin.substr(0,p + 1);
	pstring unit = sin.substr(p + 1);

	double ret = get_sp_unit(unit) * val.as_double();
	return ret;
}

nl_convert_base_t::unit_t nl_convert_base_t::m_units[] = {
		{"T",   "",      1.0e12 },
		{"G",   "",      1.0e9  },
		{"MEG", "RES_M({1})", 1.0e6  },
		{"k",   "RES_K({1})", 1.0e3  }, /* eagle */
		{"K",   "RES_K({1})", 1.0e3  },
		{"",    "{1}",        1.0e0  },
		{"M",   "CAP_M({1})", 1.0e-3 },
		{"u",   "CAP_U({1})", 1.0e-6 }, /* eagle */
		{"U",   "CAP_U({1})", 1.0e-6 },
		{"??",   "CAP_U({1})", 1.0e-6    },
		{"N",   "CAP_N({1})", 1.0e-9 },
		{"P",   "CAP_P({1})", 1.0e-12},
		{"F",   "{1}e-15",    1.0e-15},

		{"MIL", "{1}",  25.4e-6},

		{"-",   "{1}",  1.0  }
};


void nl_convert_spice_t::convert(const pstring &contents)
{
	pstring_vector_t spnl(contents, "\n");

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

void nl_convert_spice_t::process_line(const pstring &line)
{
	if (line != "")
	{
		pstring_vector_t tt(line, " ", true);
		double val = 0.0;
		switch (tt[0].code_at(0))
		{
			case ';':
				out("// {}\n", line.substr(1).cstr());
				break;
			case '*':
				out("// {}\n", line.substr(1).cstr());
				break;
			case '.':
				if (tt[0].equals(".SUBCKT"))
				{
					out("NETLIST_START({})\n", tt[1].cstr());
					for (std::size_t i=2; i<tt.size(); i++)
						add_ext_alias(tt[i]);
				}
				else if (tt[0].equals(".ENDS"))
				{
					dump_nl();
					out("NETLIST_END()\n");
				}
				else
					out("// {}\n", line.cstr());
				break;
			case 'Q':
			{
				bool cerr = false;
				/* check for fourth terminal ... should be numeric net
				 * including "0" or start with "N" (ltspice)
				 */
				ATTR_UNUSED int nval =tt[4].as_long(&cerr);
				pstring model;
				pstring pins ="CBE";

				if ((!cerr || tt[4].startsWith("N")) && tt.size() > 5)
					model = tt[5];
				else
					model = tt[4];
				pstring_vector_t m(model,"{");
				if (m.size() == 2)
				{
					if (m[1].len() != 4)
						fprintf(stderr, "error with model desc %s\n", model.cstr());
					pins = m[1].left(3);
				}
				add_device("QBJT_EB", tt[0], m[0]);
				add_term(tt[1], tt[0] + "." + pins.code_at(0));
				add_term(tt[2], tt[0] + "." + pins.code_at(1));
				add_term(tt[3], tt[0] + "." + pins.code_at(2));
			}
				break;
			case 'R':
				if (tt[0].startsWith("RV"))
				{
					val = get_sp_val(tt[4]);
					add_device("POT", tt[0], val);
					add_term(tt[1], tt[0] + ".1");
					add_term(tt[2], tt[0] + ".2");
					add_term(tt[3], tt[0] + ".3");
				}
				else
				{
					val = get_sp_val(tt[3]);
					add_device("RES", tt[0], val);
					add_term(tt[1], tt[0] + ".1");
					add_term(tt[2], tt[0] + ".2");
				}
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
			case 'I': // Input pin special notation
				{
					val = get_sp_val(tt[2]);
					add_device("ANALOG_INPUT", tt[0], val);
					add_term(tt[1], tt[0] + ".Q");
				}
				break;
			case 'D':
				add_device("DIODE", tt[0], tt[3]);
				/* FIXME ==> does Kicad use different notation from LTSPICE */
				add_term(tt[1], tt[0] + ".K");
				add_term(tt[2], tt[0] + ".A");
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
				for (std::size_t i=1; i < tt.size() - 1; i++)
				{
					pstring term = pfmt("{1}.{2}")(xname)(i);
					add_term(tt[i], term);
				}
				break;
			}
			default:
				out("// IGNORED {}: {}\n", tt[0].cstr(), line.cstr());
		}
	}
}

//FIXME: should accept a stream as well
void nl_convert_eagle_t::convert(const pstring &contents)
{
	pistringstream istrm(contents);
	eagle_tokenizer tok(*this, istrm);

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
			switch (name.code_at(0))
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
					tok.error("// IGNORED " + name);
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
				token = tok.get_token();                }
		}
		else
		{
			out("Unexpected {}\n", token.str().cstr());
			return;
		}
	}

}
