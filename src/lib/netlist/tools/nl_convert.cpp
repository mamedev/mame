// license:GPL-2.0+
// copyright-holders:Couriersud

#include "plib/palloc.h"
#include "plib/pstonum.h"
#include "plib/putil.h"

#include "nl_convert.h"

#include <algorithm>
#include <unordered_map>

// FIXME: temporarily defined here - should be in a file
// FIXME: family logic in netlist is convoluted, create
//        define a model param on core device

// Format: external name,netlist device,model
static constexpr const char s_lib_map[] =
"SN74LS00D,   TTL_7400_DIP,  74LSXX\n"
"SN74LS04D,   TTL_7404_DIP,  74LSXX\n"
"SN74ALS08D,  TTL_7408_DIP,  74ALSXX\n"
"SN74ALS10AD, TTL_7410_DIP,  74ALSXX\n"
"SN74LS30N,   TTL_7430_DIP,  74LSXX\n"
"SN74ALS74AD, TTL_7474_DIP,  74ALSXX\n"
"SN74LS74AD,  TTL_7474_DIP,  74LSXX\n"
"SN74LS86AD,  TTL_7486_DIP,  74LSXX\n"
"SN74F153D,   TTL_74153_DIP, 74FXX\n"
"SN74LS161AD, TTL_74161_DIP, 74LSXX\n"
"SN74LS164D,  TTL_74164_DIP, 74LSXX\n"
"DM74LS366AN, TTL_74366_DIP, 74LSXX\n"
;

struct lib_map_entry
{
	pstring dev;
	pstring model;
};

using lib_map_t = std::unordered_map<pstring, lib_map_entry>;

static lib_map_t read_lib_map(const pstring &lm)
{
	auto reader = plib::putf8_reader(std::istringstream(lm));
	reader.stream().imbue(std::locale::classic());
	lib_map_t m;
	pstring line;
	while (reader.readline(line))
	{
		std::vector<pstring> split(plib::psplit(line, ","));
		m[plib::trim(split[0])] = { plib::trim(split[1]), plib::trim(split[2]) };
	}
	return m;
}

// -------------------------------------------------
//    convert - convert a spice netlist
// -------------------------------------------------

nl_convert_base_t::nl_convert_base_t()
	: out(&m_buf)
	, m_numberchars("0123456789-+e.")
{
	m_buf.imbue(std::locale::classic());
	m_units = {
			{"T",   "",      1.0e12 },
			{"G",   "",      1.0e9  },
			{"MEG", "RES_M({1})", 1.0e6  },
			{"k",   "RES_K({1})", 1.0e3  }, // eagle
			{"K",   "RES_K({1})", 1.0e3  },
			{"",    "{1}",        1.0e0  },
			{"M",   "CAP_M({1})", 1.0e-3 },
			{"u",   "CAP_U({1})", 1.0e-6 }, // eagle
			{"U",   "CAP_U({1})", 1.0e-6 },
			{"μ",   "CAP_U({1})",  1.0e-6 },
			{"µ",   "CAP_U({1})",  1.0e-6 },
			{"N",   "CAP_N({1})", 1.0e-9 },
			{"pF",  "CAP_P({1})", 1.0e-12},
			{"P",   "CAP_P({1})", 1.0e-12},
			{"F",   "{1}e-15",    1.0e-15},

			{"MIL", "{1}",  25.4e-6}
	};

}

nl_convert_base_t::~nl_convert_base_t()
{
	m_nets.clear();
	m_devs.clear();
	m_pins.clear();
}

void nl_convert_base_t::add_pin_alias(const pstring &devname, const pstring &name, const pstring &alias)
{
	pstring pname = devname + "." + name;
	m_pins.emplace(pname, plib::make_unique<pin_alias_t>(pname, devname + "." + alias));
}

void nl_convert_base_t::add_ext_alias(const pstring &alias)
{
	m_ext_alias.push_back(alias);
}

void nl_convert_base_t::add_device(plib::unique_ptr<dev_t> dev)
{
	for (auto & d : m_devs)
		if (d->name() == dev->name())
		{
			out("ERROR: Duplicate device {1} ignored.", dev->name());
			return;
		}
	m_devs.push_back(std::move(dev));
}

void nl_convert_base_t::add_device(const pstring &atype, const pstring &aname, const pstring &amodel)
{
	add_device(plib::make_unique<dev_t>(atype, aname, amodel));
}
void nl_convert_base_t::add_device(const pstring &atype, const pstring &aname, double aval)
{
	add_device(plib::make_unique<dev_t>(atype, aname, aval));
}
void nl_convert_base_t::add_device(const pstring &atype, const pstring &aname)
{
	add_device(plib::make_unique<dev_t>(atype, aname));
}

void nl_convert_base_t::add_term(const pstring &netname, const pstring &termname)
{
	net_t * net = nullptr;
	auto idx = m_nets.find(netname);
	if (idx != m_nets.end())
		net = m_nets[netname].get();
	else
	{
		auto nets = plib::make_unique<net_t>(netname);
		net = nets.get();
		m_nets.emplace(netname, std::move(nets));
	}

	// if there is a pin alias, translate ...
	pin_alias_t *alias = m_pins[termname].get();

	if (alias != nullptr)
		net->terminals().push_back(alias->alias());
	else
		net->terminals().push_back(termname);
}

void nl_convert_base_t::dump_nl()
{
	for (auto & alias : m_ext_alias)
	{
		net_t *net = m_nets[alias].get();
		// use the first terminal ...
		out("ALIAS({}, {})\n", alias.c_str(), net->terminals()[0].c_str());
		// if the aliased net only has this one terminal connected ==> don't dump
		if (net->terminals().size() == 1)
			net->set_no_export();
	}

	std::vector<size_t> sorted;
	for (size_t i=0; i < m_devs.size(); i++)
		sorted.push_back(i);
	std::sort(sorted.begin(), sorted.end(),
			[&](size_t i1, size_t i2) { return m_devs[i1]->name() < m_devs[i2]->name(); });

	for (std::size_t i=0; i<m_devs.size(); i++)
	{
		std::size_t j = sorted[i];

		if (m_devs[j]->has_value())
			out("{}({}, {})\n", m_devs[j]->type().c_str(),
					m_devs[j]->name().c_str(), get_nl_val(m_devs[j]->value()).c_str());
		else if (m_devs[j]->has_model())
			out("{}({}, \"{}\")\n", m_devs[j]->type().c_str(),
					m_devs[j]->name().c_str(), m_devs[j]->model().c_str());
		else
			out("{}({})\n", m_devs[j]->type().c_str(),
					m_devs[j]->name().c_str());
	}
	// print nets
	for (auto & i : m_nets)
	{
		net_t * net = i.second.get();
		if (!net->is_no_export())
		{
			out("NET_C({}", net->terminals()[0].c_str() );
			for (std::size_t j=1; j<net->terminals().size(); j++)
			{
				out(", {}", net->terminals()[j].c_str() );
			}
			out(")\n");
		}
	}
	m_devs.clear();
	m_nets.clear();
	m_pins.clear();
	m_ext_alias.clear();
}

const pstring nl_convert_base_t::get_nl_val(const double val)
{
	for (auto &e : m_units)
	{
		if (e.m_mult <= plib::abs(val))
			return plib::pfmt(e.m_func)(val / e.m_mult);
	}
	return plib::pfmt("{1}")(val);
}

double nl_convert_base_t::get_sp_unit(const pstring &unit)
{
	for (auto &e : m_units)
	{
		if (e.m_unit == unit)
			return e.m_mult;
	}
	plib::perrlogger("Unit {} unknown\n", unit);
	return 0.0;
}

double nl_convert_base_t::get_sp_val(const pstring &sin)
{
	std::size_t p = 0;
	while (p < sin.length() && (m_numberchars.find(sin.substr(p, 1)) != pstring::npos))
		++p;
	pstring val = plib::left(sin, p);
	pstring unit = sin.substr(p);
	double ret = get_sp_unit(unit) * plib::pstonum<double>(val);
	return ret;
}

#if 0
std::vector<nl_convert_base_t::unit_t> nl_convert_base_t::m_units = {
		{"T",   "",      1.0e12 },
		{"G",   "",      1.0e9  },
		{"MEG", "RES_M({1})", 1.0e6  },
		{"k",   "RES_K({1})", 1.0e3  }, // eagle
		{"K",   "RES_K({1})", 1.0e3  },
		{"",    "{1}",        1.0e0  },
		{"M",   "CAP_M({1})", 1.0e-3 },
		{"u",   "CAP_U({1})", 1.0e-6 }, // eagle
		{"U",   "CAP_U({1})", 1.0e-6 },
		{"μ",   "CAP_U({1})",  1.0e-6 },
		{"µ",   "CAP_U({1})",  1.0e-6 },
		{"N",   "CAP_N({1})", 1.0e-9 },
		{"pF",  "CAP_P({1})", 1.0e-12},
		{"P",   "CAP_P({1})", 1.0e-12},
		{"F",   "{1}e-15",    1.0e-15},

		{"MIL", "{1}",  25.4e-6}
};
#endif

void nl_convert_spice_t::convert(const pstring &contents)
{
	std::vector<pstring> spnl(plib::psplit(contents, "\n"));

	// Add gnd net

	// FIXME: Parameter
	out("NETLIST_START(dummy)\n");
	add_term("0", "GND");

	pstring line = "";

	for (const auto &i : spnl)
	{
		// Basic preprocessing
		pstring inl = plib::ucase(plib::trim(i));
		if (plib::startsWith(inl, "+"))
			line += inl.substr(1);
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

static pstring rem(const std::vector<pstring> &vps, std::size_t start)
{
	pstring r(vps[start]);
	for (std::size_t i=start + 1; i<vps.size(); i++)
		r += " " + vps[i];
	return r;
}

void nl_convert_spice_t::process_line(const pstring &line)
{
	if (line != "")
	{
		//printf("// %s\n", line.c_str());
		std::vector<pstring> tt(plib::psplit(line, " ", true));
		double val = 0.0;
		switch (tt[0].at(0))
		{
			case ';':
				out("// {}\n", line.substr(1));
				break;
			case '*':
				out("// {}\n", line.substr(1).c_str());
				break;
			case '.':
				if (tt[0] == ".SUBCKT")
				{
					m_subckt = tt[1] + "_";
					out("NETLIST_START({})\n", tt[1]);
					for (std::size_t i=2; i<tt.size(); i++)
						add_ext_alias(tt[i]);
				}
				else if (tt[0] == ".ENDS")
				{
					dump_nl();
					out("NETLIST_END()\n");
					m_subckt = "";
				}
				else if (tt[0] == ".MODEL")
				{
					out("NET_MODEL(\"{} {}\")\n", m_subckt + tt[1], rem(tt,2));
				}
				else
					out("// {}\n", line.c_str());
				break;
			case 'Q':
			{
				// check for fourth terminal ... should be numeric net
				// including "0" or start with "N" (ltspice)

				pstring model;
				pstring pins ="CBE";
				bool err(false);
				auto nval = plib::pstonum_ne<long>(tt[4], err);
				plib::unused_var(nval);

				if ((!err || plib::startsWith(tt[4], "N")) && tt.size() > 5)
					model = tt[5];
				else
					model = tt[4];
				std::vector<pstring> m(plib::psplit(model,"{"));
				if (m.size() == 2)
				{
					if (m[1].length() != 4)
						plib::perrlogger("error with model desc {}\n", model);
					pins = plib::left(m[1], 3);
				}
				add_device("QBJT_EB", tt[0], m_subckt + m[0]);
				add_term(tt[1], tt[0] + "." + pins.at(0));
				add_term(tt[2], tt[0] + "." + pins.at(1));
				add_term(tt[3], tt[0] + "." + pins.at(2));
			}
				break;
			case 'R':
				if (plib::startsWith(tt[0], "RV"))
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
			case 'B':  // arbitrary behavioural current source - needs manual work afterwords
				add_device("CS", tt[0], "/*" + rem(tt, 3) + "*/");
				add_term(tt[1], tt[0] + ".P");
				add_term(tt[2], tt[0] + ".N");
				break;
			case 'E':
				add_device("VCVS", tt[0]);
				add_term(tt[1], tt[0] + ".OP");
				add_term(tt[2], tt[0] + ".ON");
				add_term(tt[3], tt[0] + ".IP");
				add_term(tt[4], tt[0] + ".IN");
				out("PARAM({}, {})\n", tt[0] + ".G", tt[5]);
				break;
			case 'V':
				// just simple Voltage sources ....
				if (tt[2] == "0")
				{
					val = get_sp_val(tt[3]);
					add_device("ANALOG_INPUT", tt[0], val);
					add_term(tt[1], tt[0] + ".Q");
					//add_term(tt[2], tt[0] + ".2");
				}
				else
					plib::perrlogger("Voltage Source {} not connected to GND\n", tt[0]);
				break;
#if 0
			// This is wrong ... Need to use something else for inputs!
			case 'I': // Input pin special notation
				{
					val = get_sp_val(tt[2]);
					add_device("ANALOG_INPUT", tt[0], val);
					add_term(tt[1], tt[0] + ".Q");
				}
				break;
#else
			case 'I':
				{
					val = get_sp_val(tt[3]);
					add_device("CS", tt[0], val);
					add_term(tt[1], tt[0] + ".1");
					add_term(tt[2], tt[0] + ".2");
				}
				break;
#endif
			case 'D':
				add_device("DIODE", tt[0], tt[3]);
				// FIXME ==> does Kicad use different notation from LTSPICE
				add_term(tt[1], tt[0] + ".K");
				add_term(tt[2], tt[0] + ".A");
				break;
			case 'U':
			case 'X':
			{
				// FIXME: specific code for KICAD exports
				//        last element is component type
				// FIXME: Parameter

				pstring xname = plib::replace_all(tt[0], pstring("."), pstring("_"));
				pstring tname = "TTL_" + tt[tt.size()-1] + "_DIP";
				add_device(tname, xname);
				for (std::size_t i=1; i < tt.size() - 1; i++)
				{
					pstring term = plib::pfmt("{1}.{2}")(xname)(i);
					add_term(tt[i], term);
				}
				break;
			}
			default:
				out("// IGNORED {}: {}\n", tt[0].c_str(), line.c_str());
		}
	}
}

//-------------------------------------------------
//    Eagle converter
// -------------------------------------------------

nl_convert_eagle_t::tokenizer::tokenizer(nl_convert_eagle_t &convert, plib::putf8_reader &&strm)
	: plib::ptokenizer(std::move(strm))
	, m_convert(convert)
{
	this->identifier_chars("abcdefghijklmnopqrstuvwvxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890_.-")
		.number_chars(".0123456789", "0123456789eE-.") //FIXME: processing of numbers
		.whitespace(pstring("") + ' ' + static_cast<char>(9) +  static_cast<char>(10) + static_cast<char>(13))
		// FIXME: gnetlist doesn't print comments
		.comment("/*", "*/", "//")
		.string_char('\'');
	m_tok_ADD = register_token("ADD");
	m_tok_VALUE = register_token("VALUE");
	m_tok_SIGNAL = register_token("SIGNAL");
	m_tok_SEMICOLON = register_token(";");
	// currently not used, but required for parsing
	register_token(")");
	register_token("(");
}

void nl_convert_eagle_t::tokenizer::verror(const pstring &msg)
{
	m_convert.out("{}\n", msg);
}

//FIXME: should accept a stream as well
void nl_convert_eagle_t::convert(const pstring &contents)
{

	tokenizer tok(*this, plib::putf8_reader(std::istringstream(contents)));
	tok.stream().stream().imbue(std::locale::classic());

	out("NETLIST_START(dummy)\n");
	add_term("GND", "GND");
	add_term("VCC", "VCC");
	tokenizer::token_t token = tok.get_token();
	while (true)
	{
		if (token.is_type(tokenizer::ENDOFFILE))
		{
			dump_nl();
			// FIXME: Parameter
			out("NETLIST_END()\n");
			return;
		}
		else if (token.is(tok.m_tok_SEMICOLON))
		{
			// ignore empty statements
			token = tok.get_token();
		}
		else if (token.is(tok.m_tok_ADD))
		{
			pstring name = tok.get_string();
			// skip to semicolon
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
			switch (name.at(0))
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
					if (plib::ucase(sval) == "HIGH")
						add_device("TTL_INPUT", name, 1);
					else if (plib::ucase(sval) == "LOW")
						add_device("TTL_INPUT", name, 0);
					else
						add_device("ANALOG_INPUT", name, plib::pstonum<double>(sval));
					add_pin_alias(name, "1", "Q");
					break;
				case 'D':
					// Pin 1 = Anode, Pin 2 = Cathode
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
				// fixme: should check for string
				pstring devname = token.str();
				pstring pin = tok.get_string();
				add_term(netname, devname + "." + pin);
				token = tok.get_token();                }
		}
		else
		{
			out("Unexpected {}\n", token.str().c_str());
			return;
		}
	}

}

// -------------------------------------------------
//    RINF converter
// -------------------------------------------------

nl_convert_rinf_t::tokenizer::tokenizer(nl_convert_rinf_t &convert, plib::putf8_reader &&strm)
	: plib::ptokenizer(std::move(strm))
	, m_convert(convert)
{
	this->identifier_chars(".abcdefghijklmnopqrstuvwvxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890_-")
		.number_chars("0123456789", "0123456789eE-.") //FIXME: processing of numbers
		.whitespace(pstring("") + ' ' + static_cast<char>(9) +  static_cast<char>(10) + static_cast<char>(13))
		// FIXME: gnetlist doesn't print comments
		.comment("","","//") // FIXME:needs to be confirmed
		.string_char('"');
	m_tok_HEA = register_token(".HEA");
	m_tok_APP = register_token(".APP");
	m_tok_TIM = register_token(".TIM");
	m_tok_TYP = register_token(".TYP");
	m_tok_ADDC = register_token(".ADD_COM");
	m_tok_ATTC = register_token(".ATT_COM");
	m_tok_NET = register_token(".ADD_TER");
	m_tok_TER = register_token(".TER");
	m_tok_END = register_token(".END");
}

void nl_convert_rinf_t::tokenizer::verror(const pstring &msg)
{
	m_convert.out("{}\n", msg);
}

//        token_id_t m_tok_HFA;
//        token_id_t m_tok_APP;
//        token_id_t m_tok_TIM;
//        token_id_t m_tok_TYP;
//        token_id_t m_tok_ADDC;
//        token_id_t m_tok_ATTC;
//        token_id_t m_tok_NET;
//        token_id_t m_tok_TER;

void nl_convert_rinf_t::convert(const pstring &contents)
{
	tokenizer tok(*this, plib::putf8_reader(std::istringstream(contents)));
	tok.stream().stream().imbue(std::locale::classic());
	auto lm = read_lib_map(s_lib_map);

	out("NETLIST_START(dummy)\n");
	add_term("GND", "GND");
	add_term("VCC", "VCC");
	tokenizer::token_t token = tok.get_token();
	while (true)
	{
		if (token.is_type(tokenizer::ENDOFFILE) || token.is(tok.m_tok_END))
		{
			dump_nl();
			// FIXME: Parameter
			out("NETLIST_END()\n");
			return;
		}
		else if (token.is(tok.m_tok_HEA))
		{
			// seems to be start token - ignore
			token = tok.get_token();
		}
		else if (token.is(tok.m_tok_APP))
		{
			// version string
			pstring app = tok.get_string();
			out("// APP: {}\n", app);
			token = tok.get_token();
		}
		else if (token.is(tok.m_tok_TIM))
		{
			// time
			out("// TIM:");
			for (int i=0; i<6; i++)
			{
				long x = tok.get_number_long();
				out(" {}", x);
			}
			out("\n");
			token = tok.get_token();
		}
		else if (token.is(tok.m_tok_TYP))
		{
			pstring id(tok.get_identifier());
			out("// TYP: {}\n", id);
			token = tok.get_token();
		}
		else if (token.is(tok.m_tok_ADDC))
		{
			std::unordered_map<pstring, pstring> attr;
			pstring id = tok.get_identifier();
			pstring s1 = tok.get_string();
			pstring s2 = tok.get_string();

			token = tok.get_token();
			while (token.is(tok.m_tok_ATTC))
			{
				pstring tid = tok.get_identifier();
				if (tid != id)
				{
					out("Error: found {} expected {} in {}\n", tid, id, token.str());
					return;
				}
				pstring at = tok.get_string();
				pstring val = tok.get_string();
				attr[at] = val;
				token = tok.get_token();
			}
			pstring sim = attr["Simulation"];
			pstring val = attr["Value"];
			pstring com = attr["Comment"];
			if (val == "")
				val = com;

			if (sim == "CAP")
			{
				add_device("CAP", id, get_sp_val(val));
			}
			else if (sim == "RESISTOR")
			{
				add_device("RES", id, get_sp_val(val));
			}
			else
			{
				pstring lib = attr["Library Reference"];
				auto f = lm.find(lib);
				if (f != lm.end())
					add_device(f->second.dev, id);
				else
					add_device(lib, id);
			}
		}
		else if (token.is(tok.m_tok_NET))
		{
			pstring dev = tok.get_identifier();
			pstring pin = tok.get_identifier_or_number();
			pstring net = tok.get_string();
			add_term(net, dev + "." + pin);
			token = tok.get_token();
			if (token.is(tok.m_tok_TER))
			{
				token = tok.get_token();
				while (token.is_type(plib::ptokenizer::IDENTIFIER))
				{
					pin = tok.get_identifier_or_number();
					add_term(net, token.str() + "." + pin);
					token = tok.get_token();
				}
			}
		}
#if 0
			token = tok.get_token();
			// skip to semicolon
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
					// Pin 1 = Anode, Pin 2 = Cathode
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
				// fixme: should check for string
				pstring devname = token.str();
				pstring pin = tok.get_string();
				add_term(netname, devname + "." + pin);
				token = tok.get_token();                }
		}
#endif
		else
		{
			out("Unexpected {}\n", token.str());
			return;
		}
	}

}
