// license:GPL-2.0+
// copyright-holders:Couriersud
/***************************************************************************

    nltool.c

    Simple tool to debug netlists outside MAME.

****************************************************************************/

#include "netlist/plib/pmain.h"
#include "netlist/devices/net_lib.h"
#include "netlist/nl_parser.h"
#include "netlist/nl_setup.h"
#include "netlist/solver/nld_solver.h"
#include "netlist/tools/nl_convert.h"

#include <cstring>

#define NLTOOL_VERSION  20190420

class tool_app_t : public plib::app
{
public:
	tool_app_t() :
		plib::app(),
		opt_grp1(*this,     "General options",              "The following options apply to all commands."),
		opt_cmd (*this,     "c", "cmd",         0,          std::vector<pstring>({"run","validate","convert","listdevices","static","header","docheader"}), "run|validate|convert|listdevices|static|header|docheader"),
		opt_file(*this,     "f", "file",        "-",        "file to process (default is stdin)"),
		opt_defines(*this,  "D", "define",                  "predefine value as macro, e.g. -Dname=value. If '=value' is omitted predefine it as 1. This option may be specified repeatedly."),
		opt_rfolders(*this, "r", "rom",                     "where to look for data files"),
		opt_verb(*this,     "v", "verbose",                 "be verbose - this produces lots of output"),
		opt_quiet(*this,    "q", "quiet",                   "be quiet - no warnings"),
		opt_version(*this,  "",  "version",                 "display version and exit"),
		opt_help(*this,     "h", "help",                    "display help and exit"),

		opt_grp2(*this,     "Options for run and static commands",   "These options apply to run and static commands."),
		opt_name(*this,     "n", "name",        "",         "the netlist in file specified by ""-f"" option to run; default is first one"),

		opt_grp3(*this,     "Options for run command",      "These options are only used by the run command."),
		opt_ttr (*this,     "t", "time_to_run", 1.0,        "time to run the emulation (seconds)\n\n  abc def\n\n xyz"),
		opt_stats(*this,    "s", "statistics",              "gather runtime statistics"),
		opt_logs(*this,     "l", "log" ,                    "define terminal to log. This option may be specified repeatedly."),
		opt_inp(*this,      "i", "input",       "",         "input file to process (default is none)"),
		opt_loadstate(*this,"",  "loadstate",   "",         "load state from file and continue from there"),
		opt_savestate(*this,"",  "savestate",   "",         "save state to file at end of run"),

		opt_grp4(*this,     "Options for convert command",  "These options are only used by the convert command."),
		opt_type(*this,     "y", "type",        0,          std::vector<pstring>({"spice","eagle","rinf"}), "type of file to be converted: spice,eagle,rinf"),

		opt_grp5(*this,     "Options for header command",  "These options are only used by the header command."),
		opt_tabwidth(*this, "", "tab-width", 4,          "Tab width for output."),
		opt_linewidth(*this,"", "line-width", 72,       "Line width for output."),

		opt_ex1(*this,     "nltool -c run -t 3.5 -f nl_examples/cdelay.c -n cap_delay",
				"Run netlist \"cap_delay\" from file nl_examples/cdelay.c for 3.5 seconds"),
		opt_ex2(*this,     "nltool --cmd=listdevices",
				"List all known devices."),
		opt_ex3(*this,     "nltool --cmd=header --tab-width=8 --line-width=80",
				"Create the header file needed for including netlists as code."),

		m_warnings(0),
		m_errors(0)
		{}

	plib::option_group  opt_grp1;
	plib::option_str_limit<unsigned> opt_cmd;
	plib::option_str    opt_file;
	plib::option_vec    opt_defines;
	plib::option_vec    opt_rfolders;
	plib::option_bool   opt_verb;
	plib::option_bool   opt_quiet;
	plib::option_bool   opt_version;
	plib::option_bool   opt_help;
	plib::option_group  opt_grp2;
	plib::option_str    opt_name;
	plib::option_group  opt_grp3;
	plib::option_num<double> opt_ttr;
	plib::option_bool   opt_stats;
	plib::option_vec    opt_logs;
	plib::option_str    opt_inp;
	plib::option_str    opt_loadstate;
	plib::option_str    opt_savestate;
	plib::option_group  opt_grp4;
	plib::option_str_limit<unsigned> opt_type;
	plib::option_group  opt_grp5;
	plib::option_num<unsigned> opt_tabwidth;
	plib::option_num<unsigned> opt_linewidth;
	plib::option_example opt_ex1;
	plib::option_example opt_ex2;
	plib::option_example opt_ex3;

	int execute() override;
	pstring usage() override;

	int m_warnings;
	int m_errors;
private:
	void run();
	void validate();
	void convert();
	void static_compile();

	void mac_out(const pstring &s, const bool cont = true);
	void header_entry(const netlist::factory::element_t *e);
	void mac(const netlist::factory::element_t *e);

	void create_header();
	void create_docheader();

	void listdevices();

	std::vector<pstring> m_options;

};

static NETLIST_START(dummy)
	/* Standard stuff */

	CLOCK(clk, 1000) // 1000 Hz
	SOLVER(Solver, 48000)

NETLIST_END()

/***************************************************************************
    CORE IMPLEMENTATION
***************************************************************************/

class netlist_data_folder_t : public netlist::source_t
{
public:
	netlist_data_folder_t(const pstring &folder)
	: netlist::source_t(netlist::source_t::DATA)
	, m_folder(folder)
	{
	}

	plib::unique_ptr<plib::pistream> stream(const pstring &file) override
	{
		pstring name = m_folder + "/" + file;
		try
		{
			auto strm = plib::make_unique<plib::pifilestream>(name);
			return std::move(strm);
		}
		catch (const plib::pexception &e)
		{
			if (dynamic_cast<const plib::file_open_e *>(&e) == nullptr )
				throw;
		}
		return plib::unique_ptr<plib::pistream>(nullptr);
	}

private:
	pstring m_folder;
};

class netlist_tool_callbacks_t : public netlist::callbacks_t
{
public:
	netlist_tool_callbacks_t(tool_app_t &app)
	: netlist::callbacks_t()
	, m_app(app)
	{ }

	void vlog(const plib::plog_level &l, const pstring &ls) const override;

private:
	tool_app_t &m_app;
};

class netlist_tool_t : public netlist::netlist_t
{
public:

	netlist_tool_t(tool_app_t &app, const pstring &aname)
	: netlist::netlist_t(aname, plib::make_unique<netlist_tool_callbacks_t>(app))
	{
	}

	void init()
	{
	}

	netlist::setup_t &setup() { return nlstate().setup(); }

	void read_netlist(const pstring &filename, const pstring &name,
			const std::vector<pstring> &logs,
			const std::vector<pstring> &defines,
			const std::vector<pstring> &roms)
	{
		// read the netlist ...

		for (auto & d : defines)
			setup().add_define(d);

		for (auto & r : roms)
			setup().register_source(plib::make_unique<netlist_data_folder_t>(r));

		setup().register_source(plib::make_unique<netlist::source_file_t>(filename));
		setup().include(name);
		create_dynamic_logs(logs);

		// start devices
		setup().prepare_to_run();
	}

	void create_dynamic_logs(const std::vector<pstring> &logs)
	{
		log().debug("Creating dynamic logs ...\n");
		for (auto & log : logs)
		{
			pstring name = "log_" + log;
			/*netlist_device_t *nc = */ setup().register_dev("LOG", name);
			setup().register_link(name + ".I", log);
		}
	}

	std::vector<char> save_state()
	{
		run_state_manager().pre_save();
		std::size_t size = 0;
		for (auto const & s : run_state_manager().save_list())
			size += s->m_dt.size * s->m_count;

		std::vector<char> buf(size);
		char *p = buf.data();

		for (auto const & s : run_state_manager().save_list())
		{
			std::size_t sz = s->m_dt.size * s->m_count;
			if (s->m_dt.is_float || s->m_dt.is_integral)
				std::copy(static_cast<char *>(s->m_ptr),
						static_cast<char *>(s->m_ptr) + sz, p);
			else
				log().fatal("found unsupported save element {1}\n", s->m_name);
			p += sz;
		}
		return buf;
	}

	void load_state(std::vector<char> &buf)
	{
		std::size_t size = 0;
		for (auto const & s : run_state_manager().save_list())
			size += s->m_dt.size * s->m_count;

		if (buf.size() != size)
			throw netlist::nl_exception("Size different during load state.");

		char *p = buf.data();

		for (auto const & s : run_state_manager().save_list())
		{
			std::size_t sz = s->m_dt.size * s->m_count;
			if (s->m_dt.is_float || s->m_dt.is_integral)
				std::copy(p, p + sz, static_cast<char *>(s->m_ptr));
			else
				log().fatal("found unsupported save element {1}\n", s->m_name);
			p += sz;
		}
		run_state_manager().post_load();
		nlstate().rebuild_lists();
	}

protected:

private:
};

void netlist_tool_callbacks_t::vlog(const plib::plog_level &l, const pstring &ls) const
{
	pstring err = plib::pfmt("{}: {}\n")(l.name())(ls.c_str());
	if (l == plib::plog_level::WARNING)
		m_app.m_warnings++;
	if (l == plib::plog_level::ERROR)
		m_app.m_errors++;
	if (l == plib::plog_level::FATAL)
	{
		m_app.m_errors++;
		throw netlist::nl_exception(err);
	}
	else
		m_app.pout("{}", err);
}

struct input_t
{
	input_t(const netlist::setup_t &setup, const pstring &line)
	: m_value(0.0)
	{
		std::array<char, 400> buf; // NOLINT(cppcoreguidelines-pro-type-member-init)
		double t;
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
		int e = sscanf(line.c_str(), "%lf,%[^,],%lf", &t, buf.data(), &m_value);
		if (e != 3)
			throw netlist::nl_exception(plib::pfmt("error {1} scanning line {2}\n")(e)(line));
		m_time = netlist::netlist_time::from_double(t);
		m_param = setup.find_param(pstring(buf.data()), true);
	}

	void setparam()
	{
		switch (m_param->param_type())
		{
			case netlist::param_t::STRING:
			case netlist::param_t::POINTER:
				throw netlist::nl_exception(plib::pfmt("param {1} is not numeric\n")(m_param->name()));
			case netlist::param_t::DOUBLE:
				static_cast<netlist::param_double_t*>(m_param)->setTo(m_value);
				break;
			case netlist::param_t::INTEGER:
				static_cast<netlist::param_int_t*>(m_param)->setTo(static_cast<int>(m_value));
				break;
			case netlist::param_t::LOGIC:
				static_cast<netlist::param_logic_t*>(m_param)->setTo(static_cast<bool>(m_value));
				break;
		}
	}

	netlist::netlist_time m_time;
	netlist::param_t *m_param;
	double m_value;
};

static std::vector<input_t> read_input(const netlist::setup_t &setup, const pstring &fname)
{
	std::vector<input_t> ret;
	if (fname != "")
	{
		plib::putf8_reader r = plib::putf8_reader(plib::pifilestream(fname));
		pstring l;
		while (r.readline(l))
		{
			if (l != "")
			{
				input_t inp(setup, l);
				ret.push_back(inp);
			}
		}
	}
	return ret;
}

void tool_app_t::run()
{
	plib::chrono::timer<plib::chrono::system_ticks> t;
	std::vector<input_t> inps;
	netlist::netlist_time ttr;
	netlist_tool_t nt(*this, "netlist");

	{
		auto t_guard(t.guard());
		//plib::perftime_t<plib::exact_ticks> t;

		nt.enable_stats(opt_stats());
		nt.init();

		if (!opt_verb())
			nt.log().verbose.set_enabled(false);
		if (opt_quiet())
			nt.log().info.set_enabled(false);

		nt.read_netlist(opt_file(), opt_name(),
				opt_logs(),
				m_options, opt_rfolders());

		nt.reset();

		inps = read_input(nt.setup(), opt_inp());
		ttr = netlist::netlist_time::from_double(opt_ttr());
	}


	pout("startup time ==> {1:5.3f}\n", t.as_seconds() );

	t.reset();

	netlist::netlist_time nlt = nt.time();
	{
		auto t_guard(t.guard());

		// FIXME: error handling
		if (opt_loadstate.was_specified())
		{
			plib::pifilestream strm(opt_loadstate());
			plib::pbinary_reader reader(strm);
			std::vector<char> loadstate;
			reader.read(loadstate);
			nt.load_state(loadstate);
			pout("Loaded state, run will continue at {1:.6f}\n", nt.time().as_double());
		}

		unsigned pos = 0;


		while (pos < inps.size()
				&& inps[pos].m_time < ttr
				&& inps[pos].m_time >= nlt)
		{
			nt.process_queue(inps[pos].m_time - nlt);
			inps[pos].setparam();
			nlt = inps[pos].m_time;
			pos++;
		}

		pout("runnning ...\n");

		if (ttr > nlt)
			nt.process_queue(ttr - nlt);
		else
		{
			pout("end time {1:.6f} less than saved time {2:.6f}\n",
					ttr.as_double(), nlt.as_double());
			ttr = nlt;
		}

		if (opt_savestate.was_specified())
		{
			auto savestate = nt.save_state();
			plib::pofilestream strm(opt_savestate());
			plib::pbinary_writer writer(strm);
			writer.write(savestate);
		}
		nt.stop();
	}

	double emutime = t.as_seconds();
	pout("{1:f} seconds emulation took {2:f} real time ==> {3:5.2f}%\n",
			(ttr - nlt).as_double(), emutime,
			(ttr - nlt).as_double() / emutime * 100.0);
}

void tool_app_t::validate()
{
	std::vector<input_t> inps;
	netlist_tool_t nt(*this, "netlist");

	if (!opt_verb())
		nt.log().verbose.set_enabled(false);
	if (opt_quiet())
		nt.log().info.set_enabled(false);

	m_errors = 0;
	m_warnings = 0;

	nt.setup().enable_validation();

	try
	{
		nt.init();

		nt.read_netlist(opt_file(), opt_name(),
				opt_logs(),
				m_options, opt_rfolders());
	}
	catch (netlist::nl_exception &e)
	{
		pout("Netlist exception caught: {}\n", e.text());
	}
	catch (plib::pexception &e)
	{
		pout("plib exception caught: {}\n", e.text());
	}

	//pout("Validation warnings: {}\n", m_warnings);
	//pout("Validation errors: {}\n",   m_errors);

	if (m_warnings + m_errors > 0)
		throw netlist::nl_exception("validation: {1} errors {2} warnings", m_errors, m_warnings);

}

void tool_app_t::static_compile()
{
	netlist_tool_t nt(*this, "netlist");

	nt.init();

	nt.log().verbose.set_enabled(false);
	nt.log().info.set_enabled(false);

	nt.read_netlist(opt_file(), opt_name(),
			opt_logs(),
			m_options, opt_rfolders());

	// no reset needed ...

	plib::putf8_writer w(&pout_strm);
	std::map<pstring, pstring> mp;

	nt.solver()->create_solver_code(mp);

	for (auto &e : mp)
	{
		w.write(e.second);
	}

	nt.stop();

}

void tool_app_t::mac_out(const pstring &s, const bool cont)
{
	if (cont)
	{
		unsigned pos = 0;
		pstring r;
		for (const auto &x : s)
		{
			if (x == '\t')
			{
				auto pos_mod_4 = pos % opt_tabwidth();
				auto tab_adj = opt_tabwidth() - pos_mod_4;
				r += plib::rpad(pstring(""), pstring(" "), tab_adj);
				pos += tab_adj;
			}
			else
			{
				r += x;
				pos++;
			}
		}
		pout("{1}\\\n", plib::rpad(r, pstring(" "), opt_linewidth()-1));
	}
	else
		pout("{1}\n", s);
}

void tool_app_t::header_entry(const netlist::factory::element_t *e)
{
	auto v = plib::psplit(e->param_desc(), ",");
	pstring vs;
	for (const auto &s : v)
		if (!plib::startsWith(s, "@"))
			vs += ", p" + plib::replace_all(plib::replace_all(s, "+", ""), ".", "_");
	mac_out("#define " + e->name() + "(name" + vs + ")");
	mac_out("\tNET_REGISTER_DEV(" + e->name() +", name)");

	for (const auto &s : v)
	{
		pstring r(plib::replace_all(plib::replace_all(plib::replace_all(s, "+", ""), ".", "_"), "@",""));
		if (plib::startsWith(s, "+"))
			mac_out("\tNET_CONNECT(name, " + r + ", p" + r + ")");
		else if (plib::startsWith(s, "@"))
			mac_out("\tNET_CONNECT(name, " + r + ", " + r + ")");
		else
			mac_out("\tNETDEV_PARAMI(name, " + r + ", p" + r + ")");
	}
	mac_out("", false);
}

void tool_app_t::mac(const netlist::factory::element_t *e)
{
	auto v = plib::psplit(e->param_desc(), ",");
	pstring vs;
	for (const auto &s : v)
		if (!plib::startsWith(s, "@"))
			vs += ", " + plib::replace_all(plib::replace_all(s, "+", ""), ".", "_");

	pout("{1}(name{2})\n", e->name(), vs);
	if (v.size() > 0)
	{
		pout("/*\n");
		for (const auto &s : v)
		{
			pstring r(plib::replace_all(plib::replace_all(plib::replace_all(s, "+", ""), ".", "_"), "@",""));
			if (plib::startsWith(s, "+"))
				pout("{1:10}: Terminal\n",r);
			else if (plib::startsWith(s, "@"))
				pout("{1:10}: Power terminal - automatically connected\n", r);
			else
				pout("{1:10}: Parameter\n", r);
		}
		pout("*/\n");
	}
}

void tool_app_t::create_header()
{
	netlist_tool_t nt(*this, "netlist");

	nt.init();

	nt.log().verbose.set_enabled(false);
	nt.log().info.set_enabled(false);

	nt.setup().register_source(plib::make_unique<netlist::source_proc_t>("dummy", &netlist_dummy));
	nt.setup().include("dummy");

	pout("// license:GPL-2.0+\n");
	pout("// copyright-holders:Couriersud\n");
	pout("#ifndef NLD_DEVINC_H\n");
	pout("#define NLD_DEVINC_H\n");
	pout("\n");
	pout("#include \"nl_setup.h\"\n");
	pout("#ifndef __PLIB_PREPROCESSOR__\n");
	pout("\n");
	pout("/* ----------------------------------------------------------------------------\n");
	pout(" *  Netlist Macros\n");
	pout(" * ---------------------------------------------------------------------------*/\n");
	pout("\n");

	pstring last_source("");

	for (auto &e : nt.setup().factory())
	{
		if (last_source != e->sourcefile())
		{
			last_source = e->sourcefile();
			pout("{1}\n", plib::rpad(pstring("// "), pstring("-"), opt_linewidth()));
			pout("{1}{2}\n", pstring("// Source: "), plib::replace_all(e->sourcefile(), "../", ""));
			pout("{1}\n", plib::rpad(pstring("// "), pstring("-"), opt_linewidth()));
		}
		header_entry(e.get());
	}
	pout("#endif // __PLIB_PREPROCESSOR__\n");
	pout("#endif\n");
	nt.stop();

}

void tool_app_t::create_docheader()
{
	netlist_tool_t nt(*this, "netlist");

	nt.init();

	nt.log().verbose.set_enabled(false);
	nt.log().info.set_enabled(false);

	nt.setup().register_source(plib::make_unique<netlist::source_proc_t>("dummy", &netlist_dummy));
	nt.setup().include("dummy");

	std::vector<pstring> devs;
	for (auto &e : nt.setup().factory())
		devs.push_back(e->name());
	std::sort(devs.begin(), devs.end(), [&](pstring &a, pstring &b) { return a < b; });

	pout("// license:GPL-2.0+\n");
	pout("// copyright-holders:Couriersud\n");
	pout("/* ----------------------------------------------------------------------------\n");
	pout(" *  Automatically created file. DO NOT MODIFY.\n");
	pout(" * ---------------------------------------------------------------------------*/\n");
	pout("/*!\n");
	pout(" * \\page devices Devices\n");
	pout(" *\n");
	pout(" * Below is a list of all the devices currently known to the system ...\n");
	pout(" *\n");

	for (auto &s : devs)
		pout(" *         - \\subpage {1}\n", s);

	pout(" *\n");

	for (auto &e : nt.setup().factory())
	{
		pout("//! [{1} csynopsis]\n", e->name());
		header_entry(e.get());
		pout("//! [{1} csynopsis]\n", e->name());
		pout("//! [{1} synopsis]\n", e->name());
		mac(e.get());
		pout("//! [{1} synopsis]\n", e->name());
	}
	nt.stop();
}


/*-------------------------------------------------
    listdevices - list all known devices
-------------------------------------------------*/

void tool_app_t::listdevices()
{
	netlist_tool_t nt(*this, "netlist");
	nt.init();

	nt.log().verbose.set_enabled(false);
	nt.log().info.set_enabled(false);
	nt.log().warning.set_enabled(false);

	netlist::factory::list_t &list = nt.setup().factory();

	nt.setup().register_source(plib::make_unique<netlist::source_proc_t>("dummy", &netlist_dummy));
	nt.setup().include("dummy");


	nt.setup().prepare_to_run();

	std::vector<netlist::pool_owned_ptr<netlist::core_device_t>> devs;

	for (auto & f : list)
	{
		pstring out = plib::pfmt("{1:-20} {2}(<id>")(f->classname())(f->name());

		f->macro_actions(nt.setup(), f->name() + "_lc");
		auto d = f->Create(nt.nlstate(), f->name() + "_lc");
		// get the list of terminals ...

		std::vector<pstring> terms(nt.setup().get_terminals_for_device_name(d->name()));

		out += "," + f->param_desc();
		for (const auto &p : plib::psplit(f->param_desc(),",") )
		{
			if (plib::startsWith(p, "+"))
			{
				plib::container::remove(terms, p.substr(1));
			}
		}
		out += ")";
		pout("{}\n", out);
		if (terms.size() > 0)
		{
			pstring t = "";
			for (auto & j : terms)
				t += "," + j;
			pout("\tTerminals: {}\n", t.substr(1));
		}
		devs.emplace_back(std::move(d));
	}
}

/*-------------------------------------------------
    convert - convert spice et al to netlist
-------------------------------------------------*/

void tool_app_t::convert()
{
	pstring contents;
	plib::postringstream ostrm;
	if (opt_file() == "-")
	{
		plib::pstdin f;
		plib::copystream(ostrm, f);
	}
	else
	{
		plib::pifilestream f(opt_file());
		plib::copystream(ostrm, f);
	}
	contents = ostrm.str();

	pstring result;
	if (opt_type.as_string() == "spice")
	{
		nl_convert_spice_t c;
		c.convert(contents);
		result = c.result();
	}
	else if (opt_type.as_string() == "eagle")
	{
		nl_convert_eagle_t c;
		c.convert(contents);
		result = c.result();
	}
	else if (opt_type.as_string() == "rinf")
	{
		nl_convert_rinf_t c;
		c.convert(contents);
		result = c.result();
	}
	/* present result */
	pout.write(result);
}

/*-------------------------------------------------
    main - primary entry point
-------------------------------------------------*/

#if 0
static const pstring pmf_verbose[] =
{
	"NL_PMF_TYPE_VIRTUAL",
	"NL_PMF_TYPE_GNUC_PMF",
	"NL_PMF_TYPE_GNUC_PMF_CONV",
	"NL_PMF_TYPE_INTERNAL"
};
#endif

pstring tool_app_t::usage()
{
	return help(
			"nltool serves as the Swiss Army knife to run, test and convert netlists.",
			"nltool [options]");
}

int tool_app_t::execute()
{
	tool_app_t opts;

	/* make SIGFPE actually deliver signals on supoorted platforms */
	plib::fpsignalenabler::global_enable(true);
	plib::fpsignalenabler sigen(plib::FP_ALL & ~plib::FP_INEXACT & ~plib::FP_UNDERFLOW);

	if (opt_help())
	{
		pout(usage());
		return 0;
	}

	if (opt_version())
	{
		pout(
			"nltool (netlist) " PSTRINGIFY(NLTOOL_VERSION) "\n"
			"Copyright (C) 2019 Couriersud\n"
			"License GPLv2+: GNU GPL version 2 or later <http://gnu.org/licenses/gpl.html>.\n"
			"This is free software: you are free to change and redistribute it.\n"
			"There is NO WARRANTY, to the extent permitted by law.\n\n"
			"Written by Couriersud.\n");
		if (opt_verb())
		{
			std::vector<std::pair<pstring, pstring>> defs;
			netlist::netlist_state_t::compile_defines(defs);
			pout("\nCompile defines:\n");
			for (auto &x : defs)
				pout("{1:-30} = {2}\n", x.first, x.second);

		}
		return 0;
	}

	m_options = opt_defines();
	m_options.emplace_back("NLTOOL_VERSION=" PSTRINGIFY(NLTOOL_VERSION));

	try
	{
		pstring cmd = opt_cmd.as_string();
		if (cmd == "listdevices")
			listdevices();
		else if (cmd == "run")
			run();
		else if (cmd == "validate")
			validate();
		else if (cmd == "static")
			static_compile();
		else if (cmd == "header")
			create_header();
		else if (cmd == "docheader")
			create_docheader();
		else if (cmd == "convert")
			convert();
		else
		{
			perr("Unknown command {}\n", cmd.c_str());
			//FIXME: usage_short
			perr(usage());
			return 1;
		}
	}
	catch (netlist::nl_exception &e)
	{
		perr("Netlist exception caught: {}\n", e.text());
		return 2;
	}
	catch (plib::pexception &e)
	{
		perr("plib exception caught: {}\n", e.text());
		return 2;
	}

	return 0;
}

PMAIN(tool_app_t)
