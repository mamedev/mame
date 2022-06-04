// license:BSD-3-Clause
// copyright-holders:Couriersud

#include "plib/palloc.h"
#include "plib/pstonum.h"
#include "plib/pstrutil.h"
#include "plib/putil.h"

#include "nl_convert.h"

#include <algorithm>
#include <unordered_map>
#include <vector>

namespace netlist::convert
{

// FIXME: temporarily defined here - should be in a file
// FIXME: family logic in netlist is convoluted, create
//        define a model param on core device

// Format: external name,netlist device,model

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)
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
	auto reader = plib::putf8_reader(std::make_unique<std::istringstream>(putf8string(lm)));
	reader.stream().imbue(std::locale::classic());
	lib_map_t m;
	putf8string line;
	while (reader.read_line(line))
	{
		std::vector<pstring> split(plib::psplit(pstring(line), ','));
		m[plib::trim(split[0])] = { plib::trim(split[1]), plib::trim(split[2]) };
	}
	return m;
}

// -------------------------------------------------
//    convert - convert a spice netlist
// -------------------------------------------------

nl_convert_base_t::nl_convert_base_t()
	: out(&m_buf)
	, m_number_chars("0123456789-+Ee.")
{
	m_buf.imbue(std::locale::classic());
	m_units = {
			{"T",   "{1}e12",      1.0e12 }, // NOLINT
			{"G",   "{1}e9",       1.0e9  }, // NOLINT
			{"MEG", "RES_M({1})",  1.0e6  }, // NOLINT
			{"k",   "RES_K({1})",  1.0e3  }, // NOLINT: eagle
			{"K",   "RES_K({1})",  1.0e3  }, // NOLINT
			{"",    "{1}",         1.0e0  }, // NOLINT
			{"M",   "{1}e-3",      1.0e-3 }, // NOLINT
			{"u",   "CAP_U({1})",  1.0e-6 }, // NOLINT: eagle
			{"U",   "CAP_U({1})",  1.0e-6 }, // NOLINT
			{"μ",   "CAP_U({1})",  1.0e-6 }, // NOLINT
			{"N",   "CAP_N({1})",  1.0e-9 }, // NOLINT
			{"pF",  "CAP_P({1})",  1.0e-12}, // NOLINT
			{"P",   "CAP_P({1})",  1.0e-12}, // NOLINT
			{"F",   "{1}e-15",     1.0e-15}, // NOLINT

			{"MIL", "{1}",  25.4e-6} // NOLINT
	};

	dev_map =
	{
		{ "VCCS", {"OP", "ON", "IP", "IN"} },
		{ "VCVS", {"OP", "ON", "IP", "IN"} },
		{ "CCCS", {"OP", "ON", "IP", "IN"} },
		{ "CCVS", {"OP", "ON", "IP", "IN"} },
		{ "VS", {"1", "2"} },
		{ "TTL_INPUT", {"Q", "VCC", "GND"} },
		{ "DIODE", {"A", "K"} },
		{ "POT", {"1", "2", "3"} },
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
	m_pins.emplace(pname, plib::make_unique<pin_alias_t, arena>(pname, devname + "." + alias));
}

void nl_convert_base_t::add_ext_alias(const pstring &alias)
{
	m_ext_alias.emplace_back(alias, alias);
}

void nl_convert_base_t::add_ext_alias(const pstring &alias, const pstring &net)
{
	m_ext_alias.emplace_back(alias, net);
}

void nl_convert_base_t::add_device(arena::unique_ptr<dev_t> dev)
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
	add_device(plib::make_unique<dev_t, arena>(atype, aname, amodel));
}
void nl_convert_base_t::add_device(const pstring &atype, const pstring &aname, double aval)
{
	add_device(plib::make_unique<dev_t, arena>(atype, aname, aval));
}
void nl_convert_base_t::add_device(const pstring &atype, const pstring &aname)
{
	add_device(plib::make_unique<dev_t, arena>(atype, aname));
}

void nl_convert_base_t::add_term(const pstring &netname, const pstring &termname)
{
	// Ignore NC nets!
	if (plib::startsWith(netname,"NC_"))
		return;

	net_t * net = nullptr;
	auto idx = m_nets.find(netname);
	if (idx != m_nets.end())
		net = m_nets[netname].get();
	else
	{
		auto nets = plib::make_unique<net_t, arena>(netname);
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

void nl_convert_base_t::add_term(const pstring &netname, const pstring &devname, unsigned term)
{
	auto e = dev_map.find(get_device(devname)->type());
	if (e == dev_map.end())
		out("// ERROR: No terminals found for device {}\n", devname);
	else
	{
		if (term >= e->second.size())
			out("// ERROR: {} : Term {} exceeds number of terminals {}\n", netname, devname, term);
		else
			add_term(netname, devname + "." + e->second[term]);
	}
}

void nl_convert_base_t::add_device_extra_s(const pstring &devname, const pstring &extra)
{
	auto *dev = get_device(devname);
	if (dev == nullptr)
		out("// ERROR: Device {} not found\n", devname);
	else
	{
		dev->add_extra(extra);
	}
}



void nl_convert_base_t::dump_nl()
{
	// do replacements
	for (auto &r : m_replace)
	{
		// Get the device entry
		auto *d = get_device(r.m_ce);
		if (d == nullptr)
		{
			out("ERROR: Can not find <{}>\n", r.m_ce);
			continue;
		}

		auto e = dev_map.find(d->type());
		if (e == dev_map.end())
		{
			out("ERROR: Can not find type {}\n", d->type());
			continue;
		}
		pstring term1 = r.m_ce + "." + e->second[0];
		// scan all nets
		for (auto &n : m_nets)
		{
			for (auto &t : n.second->terminals())
			{
				if (t == term1)
					t = r.m_repterm;
			}
		}
		add_term(r.m_net, term1);
	}

	for (auto & alias : m_ext_alias)
	{
		net_t *net = m_nets[alias.second].get();
		// use the first terminal ...
		out("ALIAS({}, {})\n", alias.first, net->terminals()[0]);
		// if the aliased net only has this one terminal connected ==> don't dump
		if (net->terminals().size() == 1)
			net->set_no_export();
	}

	std::vector<size_t> sorted;
	sorted.reserve(m_devs.size());
	for (size_t i=0; i < m_devs.size(); i++)
		sorted.push_back(i);
	std::sort(sorted.begin(), sorted.end(),
			[&](size_t i1, size_t i2) { return m_devs[i1]->name() < m_devs[i2]->name(); });

	for (std::size_t i=0; i<m_devs.size(); i++)
	{
		std::size_t j = sorted[i];

		if (m_devs[j]->has_value())
		{
			pstring t = m_devs[j]->type();
			pstring vals = (t == "RES" || t == "CAP") ? get_nl_val(m_devs[j]->value()) : plib::pfmt("{1:g}")(m_devs[j]->value());
			out("{}({}, {})\n", t, m_devs[j]->name(), vals);
		}
		else if (m_devs[j]->has_model())
			out("{}({}, \"{}\")\n", m_devs[j]->type(),
					m_devs[j]->name(), m_devs[j]->model());
		else
			out("{}({})\n", m_devs[j]->type(),
					m_devs[j]->name());
		for (const auto &e : m_devs[j]->extra())
			out("{}\n", e);

	}
	// print nets
	for (auto & i : m_nets)
	{
		net_t * net = i.second.get();
		if (!net->is_no_export() && !(net->terminals().size() == 1 && net->terminals()[0] == "GND" ))
		{
			out("NET_C({}", net->terminals()[0] );
			for (std::size_t j=1; j<net->terminals().size(); j++)
			{
				out(", {}", net->terminals()[j] );
			}
			out(")\n");
		}
	}
	m_replace.clear();
	m_devs.clear();
	m_nets.clear();
	m_pins.clear();
	m_ext_alias.clear();
}

pstring nl_convert_base_t::get_nl_val(double val) const
{
	for (const auto &e : m_units)
	{
		if (e.m_mult <= plib::abs(val))
		{
			double v = val / e.m_mult;
			if (plib::abs(v - std::round(v)) <= 1e-6)
				return plib::pfmt(e.m_func)(static_cast<int>(std::round(v)));
			return plib::pfmt(e.m_func)(v);
		}
	}

	if (plib::abs(val - std::round(val)) <= 1e-6)
		return plib::pfmt("{1}")(static_cast<int>(std::round(val)));
	return plib::pfmt("{1}")(val);
}

double nl_convert_base_t::get_sp_unit(const pstring &unit) const
{
	for (const auto &e : m_units)
	{
		if (e.m_unit == unit)
			return e.m_mult;
	}
	plib::perrlogger("Unit {} unknown\n", unit);
	return 0.0;
}

double nl_convert_base_t::get_sp_val(const pstring &sin) const
{
	std::size_t p = 0;
	while (p < sin.length() && (m_number_chars.find(sin.substr(p, 1)) != pstring::npos))
		++p;
	pstring val = plib::left(sin, p);
	pstring unit = sin.substr(p);
	double ret = get_sp_unit(unit) * plib::pstonum<double>(val);
	return ret;
}

void nl_convert_spice_t::convert_block(const str_list &contents)
{
	for (const auto &line : contents)
		process_line(line);
}


void nl_convert_spice_t::convert(const pstring &contents)
{
	std::vector<pstring> spnl(plib::psplit(contents, '\n'));
	std::vector<pstring> after_line_continuation;

	// Add gnd net

	// FIXME: Parameter

	pstring line = "";

	// process line continuation

	for (const auto &i : spnl)
	{
		// Basic preprocessing
		pstring inl = plib::ucase(plib::trim(i));
		if (plib::startsWith(inl, "+"))
			line += inl.substr(1);
		else
		{
			after_line_continuation.push_back(line);
			line = inl;
		}
	}
	after_line_continuation.push_back(line);
	spnl.clear(); // no longer needed

	// Process sub circuits

	std::vector<std::vector<pstring>> subckts;
	std::vector<pstring> nl;
	auto inp = after_line_continuation.begin();
	while (inp != after_line_continuation.end())
	{
		if (plib::startsWith(*inp, ".SUBCKT"))
		{
			std::vector<pstring> sub;
			while (inp != after_line_continuation.end())
			{
				auto s(*inp);
				sub.push_back(s);
				inp++;
				if (plib::startsWith(s, ".ENDS"))
					break;
			}
			subckts.push_back(sub);
		}
		else
		{
			nl.push_back(*inp);
			inp++;
		}
	}

	for (const auto &sub : subckts)
	{
		add_term("0", "GND");
		add_term("GND", "GND"); // For Kicad
		convert_block(sub);
	}

	out("NETLIST_START(dummy)\n");
	add_term("0", "GND");
	add_term("GND", "GND"); // For Kicad

	convert_block(nl);
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

static int npoly(const pstring &s)
{
	// Brute force
	if (s=="POLY(1)")
		return 1;
	if (s=="POLY(2)")
		return 2;
	if (s=="POLY(3)")
		return 3;
	if (s=="POLY(4)")
		return 4;
	if (s=="POLY(5)")
		return 5;
	return -1;
}

void nl_convert_spice_t::process_line(const pstring &line)
{
	if (!line.empty())
	{
		//printf("// %s\n", line.c_str());
		std::vector<pstring> tt(plib::psplit(line, ' ', true));
		double val = 0.0;
		switch (tt[0].at(0))
		{
			case ';':
			case '*':
				out("// {}\n", line.substr(1));
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
					pstring mod(rem(tt,2));
					// Filter out `ngspice` X=X model declarations
					if (tt[1] != mod)
						out("NET_MODEL(\"{} {}\")\n", m_subckt + tt[1], mod);
				}
				else if (tt[0] == ".TITLE" && tt[1] == "KICAD")
				{
					m_is_kicad = true;
				}
				else
					out("// {}\n", line);
				break;
			case 'Q':
			{
				// check for fourth terminal ... should be numeric net
				// including "0" or start with "N" (`ltspice`)

				pstring model;
				pstring pins ="CBE";
				bool err(false);
				[[maybe_unused]] auto nval = plib::pstonum_ne<long>(tt[4], err);

				if ((!err || plib::startsWith(tt[4], "N")) && tt.size() > 5)
					model = tt[5];
				else
					model = tt[4];
				std::vector<pstring> m(plib::psplit(model, '{'));
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
			case 'B':  // arbitrary behavioural current source - needs manual work afterwards
				add_device("CS", tt[0], "/*" + rem(tt, 3) + "*/");
				add_term(tt[1], tt[0] + ".P");
				add_term(tt[2], tt[0] + ".N");
				break;
			case 'E':
			{
				auto n=npoly(tt[3]);
				if (n<0)
				{
					add_device("VCVS", tt[0], get_sp_val(tt[5]));
					add_term(tt[1], tt[0], 0);
					add_term(tt[2], tt[0], 1);
					add_term(tt[3], tt[0], 2);
					add_term(tt[4], tt[0], 3);
					//add_device_extra(tt[0], "PARAM({}, {})", tt[0] + ".G", tt[5]);
				}
				else
				{
					unsigned sce(4);
					auto scoeff(static_cast<unsigned>(5 + n));
					if ((tt.size() != 5 + 2 * static_cast<unsigned>(n)) || (tt[scoeff-1] != "0"))
					{
						out("// IGNORED {}: {}\n", tt[0], line);
						break;
					}
					pstring lastnet = tt[1];
					for (std::size_t i=0; i < static_cast<std::size_t>(n); i++)
					{
						pstring devname = plib::pfmt("{}{}")(tt[0], i);
						pstring nextnet = (i<static_cast<std::size_t>(n)-1) ? plib::pfmt("{}a{}")(tt[1], i) : tt[2];
						auto net2 = plib::psplit(plib::replace_all(plib::replace_all(tt[sce+i],")",""),"(",""),',');
						add_device("VCVS", devname, get_sp_val(tt[scoeff+i]));
						add_term(lastnet, devname, 0);
						add_term(nextnet, devname, 1);
						add_term(net2[0], devname, 2);
						add_term(net2[1], devname, 3);
						//# add_device_extra(devname, "PARAM({}, {})", devname + ".G", tt[scoeff+i]);
						lastnet = nextnet;
					}
				}
			}
				break;
			case 'F':
				{
					auto n=npoly(tt[3]);
					unsigned sce(4);
					unsigned scoeff(5 + static_cast<unsigned>(n));
					if (n<0)
					{
						sce = 3;
						scoeff = 4;
						n = 1;
					}
					else
					{
						if ((tt.size() != 5 + 2 *  static_cast<unsigned>(n)) || (tt[scoeff-1] != "0"))
						{
							out("// IGNORED {}: {}\n", tt[0], line);
							break;
						}
					}
					for (std::size_t i=0; i < static_cast<std::size_t>(n); i++)
					{
						pstring devname = plib::pfmt("{}{}")(tt[0], i);
						add_device("CCCS", devname, get_sp_val(tt[scoeff+i]));
						add_term(tt[1], devname, 0);
						add_term(tt[2], devname, 1);

						pstring extra_net_name = devname + "net";
						m_replace.push_back({tt[sce+i], devname + ".IP", extra_net_name });
						add_term(extra_net_name, devname + ".IN");
						//# add_device_extra(devname, "PARAM({}, {})", devname + ".G", tt[scoeff+i]);
					}
				}
				break;
			case 'H':
				add_device("CCVS", tt[0], get_sp_val(tt[4]));
				add_term(tt[1], tt[0] + ".OP");
				add_term(tt[2], tt[0] + ".ON");
				m_replace.push_back({tt[3], tt[0] + ".IP", tt[2] + "a" });
				add_term(tt[2] + "a", tt[0] + ".IN");
				//add_device_extra(tt[0], "PARAM({}, {})", tt[0] + ".G", tt[4]);
				break;
			case 'G':
				add_device("VCCS", tt[0], get_sp_val(tt[5]));
				add_term(tt[1], tt[0], 0);
				add_term(tt[2], tt[0], 1);
				add_term(tt[3], tt[0], 2);
				add_term(tt[4], tt[0], 3);
				//add_device_extra(tt[0], "PARAM({}, {})", tt[0] + ".G", tt[5]);
				break;
			case 'V':
				// only DC Voltage sources ....
				val = get_sp_val(tt[3] == "DC" ? tt[4] : tt[3]);
				add_device("VS", tt[0], val);
				add_term(tt[1], tt[0] + ".1");
				add_term(tt[2], tt[0] + ".2");
				break;
			case 'I':
				{
					val = get_sp_val(tt[3] == "DC" ? tt[4] : tt[3]);
					add_device("CS", tt[0], val);
					add_term(tt[1], tt[0] + ".1");
					add_term(tt[2], tt[0] + ".2");
				}
				break;
			case 'D':
				add_device("DIODE", tt[0], m_subckt + tt[3]);
				add_term(tt[1], tt[0], 0);
				add_term(tt[2], tt[0], 1);
				break;
			case 'U':
			case 'X':
			{
				// FIXME: specific code for KICAD exports
				//        last element is component type
				// FIXME: Parameter

				pstring xname = plib::replace_all(tt[0], pstring("."), pstring("_"));
				// Extract parameters of form X=Y
				std::vector<pstring> nets;
				std::unordered_map<pstring, pstring> params;
				for (std::size_t i=1; i < tt.size(); i++)
				{
					auto p = tt[i].find('=');
					if (p != pstring::npos)
						params.emplace(tt[i].substr(0,p), tt[i].substr(p+1));
					else
					{
						nets.push_back(tt[i]);
					}
				}
				pstring modname = nets[nets.size()-1];
				pstring tname = modname;
				if (plib::startsWith(modname, "7"))
					tname = "TTL_" + modname + "_DIP";
				else if (plib::startsWith(modname, "4"))
					tname = "CD" + modname + "_DIP";
				else if (modname == "ANALOG_INPUT" && params.size()== 1 && params.begin()->first == "V")
				{
					auto yname=pstring("I_") + tt[0].substr(1);
					val = get_sp_val(params["V"]);
					add_device(modname, yname, val);
					add_term(nets[0], yname + ".Q");
					break;
				}
				else if (modname == "TTL_INPUT" && params.size()== 1 && params.begin()->first == "L")
				{
					auto yname=pstring("I_") + tt[0].substr(1);
					val = get_sp_val(params["L"]);
					add_device(modname, yname, val);
					add_term(nets[0], yname, 0);
					add_term(nets[1], yname, 1);
					add_term(nets[2], yname, 2);
					break;
				}
				else if (modname == "ALIAS" && nets.size() == 2 && params.empty())
				{
					auto yname=tt[0].substr(1);
					add_ext_alias(yname, nets[0]);
					break;
				}
				else if (modname == "RPOT" && nets.size() == 4 && !params.empty())
				{
					auto yname=tt[0];
					auto R = params.find("R");
					auto V = params.find("V");
					if (R != params.end())
					{
						add_device("POT", yname, get_sp_val(R->second));
						add_term(nets[0], yname, 0);
						add_term(nets[1], yname, 1);
						add_term(nets[2], yname, 2);
						if (V != params.end())
							add_device_extra(yname, "PARAM({}, {})", yname + ".DIAL", get_sp_val(V->second));
					}
					else
						out("// IGNORED {}: {}\n", tt[0], line);
					break;
				}
				else
					tname = modname + "_DIP";

				add_device(tname, xname);
				for (std::size_t i=0; i < nets.size() - 1; i++)
				{
					// FIXME:
					pstring term = plib::pfmt("{1}.{2}")(xname)(i+1);
					add_term(nets[i], term);
				}
				break;
			}
			default:
				out("// IGNORED {}: {}\n", tt[0], line);
		}
	}
}

//-------------------------------------------------
//    Eagle converter
// -------------------------------------------------

nl_convert_eagle_t::tokenizer::tokenizer(nl_convert_eagle_t &convert)
	: plib::tokenizer_t()
	, m_convert(convert)
{
	this->identifier_chars("abcdefghijklmnopqrstuvwvxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890_.-")
		.number_chars(".0123456789", "0123456789eE-.") //FIXME: processing of numbers
		.whitespace(pstring("") + ' ' + static_cast<char>(9) +  static_cast<char>(10) + static_cast<char>(13))
		// FIXME: netlist doesn't print comments
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

	tokenizer tok(*this);

	tokenizer::token_store tokstor;
	plib::putf8_reader u8reader(std::make_unique<std::istringstream>(putf8string(contents)));

	tok.append_to_store(&u8reader, tokstor);
	tok.set_token_source(&tokstor);

	out("NETLIST_START(dummy)\n");
	add_term("GND", "GND");
	add_term("VCC", "VCC");
	tokenizer::token_t token = tok.get_token();
	while (true)
	{
		if (token.is_type(tokenizer::token_type::ENDOFFILE))
		{
			dump_nl();
			// FIXME: Parameter
			out("NETLIST_END()\n");
			return;
		}

		if (token.is(tok.m_tok_SEMICOLON))
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
					tok.error(plib::perrmsg("// IGNORED {1}", name));
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
			out("Unexpected {}\n", token.str());
			return;
		}
	}

}

// -------------------------------------------------
//    RINF converter
// -------------------------------------------------

nl_convert_rinf_t::tokenizer::tokenizer(nl_convert_rinf_t &convert)
	: plib::tokenizer_t()
	, m_convert(convert)
{
	this->identifier_chars(".abcdefghijklmnopqrstuvwvxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890_-")
		.number_chars("0123456789", "0123456789eE-.") //FIXME: processing of numbers
		.whitespace(pstring("") + ' ' + static_cast<char>(9) +  static_cast<char>(10) + static_cast<char>(13))
		// FIXME: netlist doesn't print comments
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
	tokenizer tok(*this);

	tokenizer::token_store tokstor;
	plib::putf8_reader u8reader(std::make_unique<std::istringstream>(putf8string(contents)));

	tok.append_to_store(&u8reader, tokstor);
	tok.set_token_source(&tokstor);

	auto lm = read_lib_map(s_lib_map);

	out("NETLIST_START(dummy)\n");
	add_term("GND", "GND");
	add_term("VCC", "VCC");
	tokenizer::token_t token = tok.get_token();
	while (true)
	{
		if (token.is_type(tokenizer::token_type::ENDOFFILE) || token.is(tok.m_tok_END))
		{
			dump_nl();
			// FIXME: Parameter
			out("NETLIST_END()\n");
			return;
		}

		if (token.is(tok.m_tok_HEA))
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
			if (val.empty())
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
				while (token.is_type(plib::token_reader_t::token_type::IDENTIFIER))
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

} // namespace netlist::convert
