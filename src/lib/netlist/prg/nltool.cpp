// license:GPL-2.0+
// copyright-holders:Couriersud

// ***************************************************************************
//
//    nltool.cpp
//
//    Simple tool to debug netlists outside MAME.
//
// ***************************************************************************

#include "netlist/plib/pmain.h"
#include "netlist/devices/net_lib.h"
#include "netlist/nl_errstr.h"
#include "netlist/nl_parser.h"
#include "netlist/nl_setup.h"
#include "netlist/plib/pstrutil.h"
#include "netlist/solver/nld_solver.h"
#include "netlist/tools/nl_convert.h"

#include <cstdio> // scanf
#include <iomanip> // scanf
#include <ios>
#include <iostream> // scanf

#ifndef NL_DISABLE_DYNAMIC_LOAD
#define NL_DISABLE_DYNAMIC_LOAD 0
#endif

extern plib::dynlib_static_sym nl_static_solver_syms[];

class tool_app_t : public plib::app
{
public:
	tool_app_t() :
		plib::app(),
		m_warnings(0),
		m_errors(0),

		opt_grp1(*this,     "General options",              "The following options apply to all commands."),
		opt_cmd (*this,     "c", "cmd",         0,          std::vector<pstring>({"run","validate","convert","listdevices","static","header","docheader"}), "run|validate|convert|listdevices|static|header|docheader"),
		opt_includes(*this, "I", "include",                 "Add the directory to the list of directories to be searched for header files. This option may be specified repeatedly."),
		opt_defines(*this,  "D", "define",                  "predefine value as macro, e.g. -Dname=value. If '=value' is omitted predefine it as 1. This option may be specified repeatedly."),
		opt_rfolders(*this, "r", "rom",                     "where to look for data files"),
		opt_verb(*this,     "v", "verbose",                 "be verbose - this produces lots of output"),
		opt_quiet(*this,    "q", "quiet",                   "be quiet - no warnings"),
		opt_prepro(*this,   "",  "prepro",                  "output preprocessor output to stderr"),

		opt_files(*this, "files to process"),

		opt_version(*this,  "",  "version",                 "display version and exit"),
		opt_help(*this,     "h", "help",                    "display help and exit"),

		opt_grp2(*this,     "Options for run and static commands",   "These options apply to run and static commands."),
		opt_name(*this,     "n", "name",        "",         "the netlist in file specified by ""-f"" option to run; default is first one"),

		opt_grp3(*this,     "Options for static command",   "These options apply to static command."),
		opt_dir(*this,      "d", "dir",        "",          "output directory for the generated files."),
		opt_out(*this,      "o", "output",     "",          "single output file for the generated code.\nEither --dir or --output can be specificied"),

		opt_grp4(*this,     "Options for run command",      "These options are only used by the run command."),
		opt_ttr (*this,     "t", "time_to_run", 1,          "time to run the emulation (seconds)"),
		opt_boostlib(*this,  "",  "boost_lib", "builtin",   "generic: will use generic solvers.\nbuiltin: Use optimized solvers compiled in.\nsomelib.so: Use library with precompiled solvers."),
		opt_stats(*this,    "s", "statistics",              "gather runtime statistics"),
		opt_logs(*this,     "l", "log" ,                    "define terminal to log. This option may be specified repeatedly."),
		opt_inp(*this,      "i", "input",       "",         "input file to process (default is none)"),
		opt_loadstate(*this,"",  "loadstate",   "",         "load state from file and continue from there"),
		opt_savestate(*this,"",  "savestate",   "",         "save state to file at end of run"),
		opt_fperr(*this,    "",  "fperr",
			"raise exception on floating point errors. This is intended to be used during debugging."),

		opt_grp5(*this,     "Options for convert command",  "These options are only used by the convert command."),
		opt_type(*this,     "y", "type",        0,           std::vector<pstring>({"spice","eagle","rinf"}), "type of file to be converted: spice,eagle,rinf"),

		opt_grp6(*this,     "Options for validate command",  "These options are only used by the validate command."),
		opt_extended_validation(*this, "", "extended",       "Identify issues with power terminals."),

		opt_grp7(*this,     "Options for header command",    "These options are only used by the header command."),
		opt_tabwidth(*this, "", "tab-width", 4,              "Tab width for output."),
		opt_linewidth(*this,"", "line-width", 72,            "Line width for output."),
		opt_pattern(*this, "", "pattern",                    "Pattern to match against device names. If the device name contains pattern, the device will be included in the output. Multiple patterns can be specified, if none is given, all devices will be output."),

		opt_ex1(*this,     "nltool -c run -t 3.5 -n cap_delay nl_examples/cdelay.c",
				"Run netlist \"cap_delay\" from file nl_examples/cdelay.c for 3.5 seconds"),
		opt_ex2(*this,     "nltool --cmd=listdevices",
				"List all known devices."),
		opt_ex3(*this,     "nltool --cmd=header --tab-width=8 --line-width=80",
				"Create the header file needed for including netlists as code."),
		opt_ex4(*this,     "nltool --cmd static --output src/lib/netlist/generated/static_solvers.cpp src/mame/audio/nl_*.cpp src/mame/machine/nl_*.cpp",
				"Create static solvers for the MAME project.")
		{}
public:
	int execute() override;
	pstring usage() override;

	template<typename... ARGS>
	void poutprefix(const pstring &prefix, const pstring &fmt, ARGS&&... args)
	{
		pstring res = plib::pfmt(fmt)(std::forward<ARGS>(args)...);
		auto lines(plib::psplit(res, "\n", false));
		if (lines.empty())
			pout(prefix + "\n");
		else
			for (auto &l : lines)
				pout(prefix + l + "\n");
	}

	int m_warnings;
	int m_errors;
private:
	plib::option_group  opt_grp1;
	plib::option_str_limit<unsigned> opt_cmd;
	plib::option_vec    opt_includes;
	plib::option_vec    opt_defines;
	plib::option_vec    opt_rfolders;
	plib::option_bool   opt_verb;
	plib::option_bool   opt_quiet;
	plib::option_bool   opt_prepro;
	plib::option_args   opt_files;
	plib::option_bool   opt_version;
	plib::option_bool   opt_help;

	plib::option_group  opt_grp2;
	plib::option_str    opt_name;

	plib::option_group  opt_grp3;
	plib::option_str    opt_dir;
	plib::option_str    opt_out;

	plib::option_group  opt_grp4;
	plib::option_num<nl_fptype> opt_ttr;
	plib::option_str    opt_boostlib;
	plib::option_bool   opt_stats;
	plib::option_vec    opt_logs;
	plib::option_str    opt_inp;
	plib::option_str    opt_loadstate;
	plib::option_str    opt_savestate;
	plib::option_bool   opt_fperr;

	plib::option_group  opt_grp5;
	plib::option_str_limit<unsigned> opt_type;

	plib::option_group  opt_grp6;
	plib::option_bool   opt_extended_validation;
	plib::option_group  opt_grp7;
	plib::option_num<unsigned> opt_tabwidth;
	plib::option_num<unsigned> opt_linewidth;
	plib::option_vec     opt_pattern;
	plib::option_example opt_ex1;
	plib::option_example opt_ex2;
	plib::option_example opt_ex3;
	plib::option_example opt_ex4;

	struct compile_map_entry
	{
		compile_map_entry(pstring mod, pstring code)
		: m_module(mod), m_code(code) { }
		pstring m_module;
		pstring m_code;
	};

	using compile_map = std::map<pstring, compile_map_entry>;

	void run();
	void validate();
	void convert();

	void compile_one_and_add_to_map(const pstring &file,
		const pstring &name, netlist::solver::static_compile_target target,
		compile_map &map);
	void static_compile();

	void mac_out(const pstring &s, bool cont = true);
	void header_entry(const netlist::factory::element_t *e);
	void mac(const netlist::factory::element_t *e);

	void create_header();
	void create_docheader();

	void listdevices();

	std::vector<pstring> m_defines;

};

static NETLIST_START(dummy)
NETLIST_END()

// **************************************************************************
//    CORE IMPLEMENTATION
// **************************************************************************

class netlist_data_folder_t : public netlist::source_data_t
{
public:
	explicit netlist_data_folder_t(const pstring &folder)
	: m_folder(folder)
	{
	}

	stream_ptr stream(const pstring &file) override
	{
		pstring name = m_folder + "/" + file;
		auto strm(plib::make_unique<std::ifstream>(plib::filesystem::u8path(name)));
		if (strm->fail())
			return stream_ptr(nullptr);

		strm->imbue(std::locale::classic());
		return std::move(strm); // FIXME: for c++11 clang builds;
	}

private:
	pstring m_folder;
};

class netlist_tool_callbacks_t : public netlist::callbacks_t
{
public:
	explicit netlist_tool_callbacks_t(tool_app_t &app, pstring boostlib)
	: m_app(app), m_boostlib(boostlib)
	{ }

	void vlog(const plib::plog_level &l, const pstring &ls) const noexcept override;

	plib::unique_ptr<plib::dynlib_base> static_solver_lib() const override
	{
		if (m_boostlib == "builtin")
			return plib::make_unique<plib::dynlib_static>(nl_static_solver_syms);
		if (m_boostlib == "generic")
			return plib::make_unique<plib::dynlib_static>(nullptr);
		if (NL_DISABLE_DYNAMIC_LOAD)
			throw netlist::nl_exception("Dynamic library loading not supported due to project security concerns.");

		//pstring libpath = plib::util::environment("NL_BOOSTLIB", plib::util::buildpath({".", "nlboost.so"}));
		return plib::make_unique<plib::dynlib>(m_boostlib);
	}

private:
	tool_app_t &m_app;
	pstring    m_boostlib;
};

class netlist_tool_t : public netlist::netlist_state_t
{
public:

	netlist_tool_t(tool_app_t &app, const pstring &name, const pstring &boostlib)
	: netlist::netlist_state_t(name, plib::make_unique<netlist_tool_callbacks_t>(app, boostlib))
	{
	}

	void read_netlist(const pstring &filename, const pstring &name,
			const std::vector<pstring> &logs,
			const std::vector<pstring> &defines,
			const std::vector<pstring> &roms,
			const std::vector<pstring> &includes)
	{
		// read the netlist ...

		for (const auto & d : defines)
			parser().add_define(d);

		for (const auto & r : roms)
			parser().register_source<netlist_data_folder_t>(r);

		for (const auto & i : includes)
			parser().add_include<netlist_data_folder_t>(i);

		parser().register_source<netlist::source_file_t>(filename);
		parser().include(name);
		parser().register_dynamic_log_devices(logs);

		// start devices
		setup().prepare_to_run();
	}

	std::vector<char> save_state()
	{
		run_state_manager().pre_save();
		std::size_t size = 0;
		for (auto const & s : run_state_manager().save_list())
			size += s->dt().size() * s->count();

		std::vector<char> buf(size);
		char *p = buf.data();

		for (auto const & s : run_state_manager().save_list())
		{
			std::size_t sz = s->dt().size() * s->count();
			if (s->dt().is_float() || s->dt().is_integral())
				std::copy(static_cast<char *>(s->ptr()),
						static_cast<char *>(s->ptr()) + sz, p);
			else
				log().fatal("found unsupported save element {1}\n", s->name());
			p += sz;
		}
		return buf;
	}

	void load_state(std::vector<char> &buf)
	{
		std::size_t size = 0;
		for (auto const & s : run_state_manager().save_list())
			size += s->dt().size() * s->count();

		if (buf.size() != size)
			throw netlist::nl_exception("Size different during load state.");

		char *p = buf.data();

		for (auto const & s : run_state_manager().save_list())
		{
			std::size_t sz = s->dt().size() * s->count();
			if (s->dt().is_float() || s->dt().is_integral())
				std::copy(p, p + sz, static_cast<char *>(s->ptr()));
			else
				log().fatal("found unsupported save element {1}\n", s->name());
			p += sz;
		}
		run_state_manager().post_load();
		this->rebuild_lists();
	}

protected:

private:
};

void netlist_tool_callbacks_t::vlog(const plib::plog_level &l, const pstring &ls) const noexcept
{
	pstring err = plib::pfmt("{}: {}\n")(l.name())(ls.c_str());
	if (l == plib::plog_level::WARNING)
		m_app.m_warnings++;
	if (l == plib::plog_level::ERROR)
		m_app.m_errors++;
	if (l == plib::plog_level::FATAL)
		m_app.m_errors++;
	m_app.pout("{}", err);
}

struct input_t
{
	input_t(const netlist::setup_t &setup, const pstring &line)
	: m_value(netlist::nlconst::zero())
	{
		std::array<char, 400> buf; // NOLINT(cppcoreguidelines-pro-type-member-init)
		double t(0);
		double val(0);

		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
		int e = std::sscanf(line.c_str(), "%lf,%[^,],%lf", &t, buf.data(), &val);
		if (e != 3)
			throw netlist::nl_exception(plib::pfmt("error {1} scanning line {2}\n")(e)(line));
		m_value = static_cast<nl_fptype>(val);
		m_time = netlist::netlist_time_ext::from_fp(t);
		m_param = setup.find_param(pstring(buf.data()));
	}

	void setparam() const
	{
		switch (m_param.param().param_type())
		{
			case netlist::param_t::STRING:
			case netlist::param_t::POINTER:
				throw netlist::nl_exception(plib::pfmt("param {1} is not numeric\n")(m_param.param().name()));
			case netlist::param_t::DOUBLE:
				static_cast<netlist::param_fp_t*>(&m_param.param())->set(m_value);
				break;
			case netlist::param_t::INTEGER:
				static_cast<netlist::param_int_t*>(&m_param.param())->set(static_cast<int>(m_value));
				break;
			case netlist::param_t::LOGIC:
				static_cast<netlist::param_logic_t*>(&m_param.param())->set(static_cast<bool>(m_value));
				break;
		}
	}

	netlist::netlist_time_ext m_time;
	netlist::param_ref_t m_param;
	nl_fptype m_value;
};

static std::vector<input_t> read_input(const netlist::setup_t &setup, const pstring &fname)
{
	std::vector<input_t> ret;
	if (fname != "")
	{
		plib::putf8_reader r = plib::putf8_reader(plib::make_unique<std::ifstream>(plib::filesystem::u8path(fname)));
		if (r.stream().fail())
			throw netlist::nl_exception(netlist::MF_FILE_OPEN_ERROR(fname));
		r.stream().imbue(std::locale::classic());
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
	netlist::netlist_time_ext ttr;

	if (opt_files().size() != 1)
		throw netlist::nl_exception("nltool: run needs exactly one file");

	netlist_tool_t nt(*this, "netlist", opt_boostlib());

	{
		auto t_guard(t.guard());
		//plib::perftime_t<plib::exact_ticks> t;

		nt.exec().enable_stats(opt_stats());

		if (!opt_verb())
			nt.log().verbose.set_enabled(false);
		if (opt_quiet())
			nt.log().info.set_enabled(false);

		nt.read_netlist(opt_files()[0], opt_name(),
				opt_logs(),
				m_defines, opt_rfolders(), opt_includes());

		// Inputs must be read before reset -> will clear setup and parser
		inps = read_input(nt.setup(), opt_inp());
		nt.exec().reset();

		ttr = netlist::netlist_time_ext::from_fp(opt_ttr());
	}


	pout("startup time ==> {1:5.3f}\n", t.as_seconds<nl_fptype>() );

	// FIXME: error handling
	if (opt_loadstate.was_specified())
	{
		std::ifstream strm(plib::filesystem::u8path(opt_loadstate()));
		if (strm.fail())
			throw netlist::nl_exception(netlist::MF_FILE_OPEN_ERROR(opt_loadstate()));
		strm.imbue(std::locale::classic());
		plib::pbinary_reader reader(strm);
		std::vector<char> loadstate;
		reader.read(loadstate);
		nt.load_state(loadstate);
		pout("Loaded state, run will continue at {1:.6f}\n", nt.exec().time().as_double());
	}

	t.reset();

	netlist::netlist_time_ext nlstart = nt.exec().time();
	{
		pout("runnning ...\n");
		unsigned pos = 0;
		netlist::netlist_time_ext nlt = nlstart;
		auto t_guard(t.guard());

		while (pos < inps.size()
				&& inps[pos].m_time < ttr
				&& inps[pos].m_time >= nlt)
		{
			nt.exec().process_queue(inps[pos].m_time - nlt);
			inps[pos].setparam();
			nlt = inps[pos].m_time;
			pos++;
		}

		if (ttr > nlt)
			nt.exec().process_queue(ttr - nlt);
		else
		{
			pout("end time {1:.6f} less than saved time {2:.6f}\n",
					ttr.as_double(), nlt.as_double());
			ttr = nlt;
		}
	}

	if (opt_savestate.was_specified())
	{
		auto savestate = nt.save_state();
		std::ofstream strm(plib::filesystem::u8path(opt_savestate()), std::ios_base::binary);
		if (strm.fail())
			throw plib::file_open_e(opt_savestate());
		strm.imbue(std::locale::classic());

		plib::pbinary_writer writer(strm);
		writer.write(savestate);
	}
	nt.exec().stop();

	auto emutime(t.as_seconds<nl_fptype>());
	pout("{1:f} seconds emulation took {2:f} real time ==> {3:5.2f}%\n",
			(ttr - nlstart).as_fp<nl_fptype>(), emutime,
			(ttr - nlstart).as_fp<nl_fptype>() / emutime * netlist::nlconst::hundred());
}

void tool_app_t::validate()
{
	netlist_tool_t nt(*this, "netlist", opt_boostlib());

	if (opt_files().size() != 1)
		throw netlist::nl_exception("nltool: validate needs exactly one file");

	if (!opt_verb())
		nt.log().verbose.set_enabled(false);
	if (opt_quiet())
		nt.log().info.set_enabled(false);

	m_errors = 0;
	m_warnings = 0;

	nt.set_extended_validation(opt_extended_validation());

	try
	{
		nt.read_netlist(opt_files()[0], opt_name(),
				opt_logs(),
				m_defines, opt_rfolders(), opt_includes());
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

void tool_app_t::compile_one_and_add_to_map(const pstring &file,
	const pstring &name, netlist::solver::static_compile_target target,
	compile_map &map)
{
	try
	{
		netlist_tool_t nt(*this, "netlist", opt_boostlib());

		nt.log().verbose.set_enabled(false);
		nt.log().info.set_enabled(false);
		nt.log().warning.set_enabled(false);

		nt.read_netlist(file, name,
				opt_logs(),
				m_defines, opt_rfolders(), opt_includes());

		// need to reset ...

		nt.exec().reset();

		auto mp(nt.exec().solver()->create_solver_code(target));

		for (auto &e : mp)
		{
			auto it = map.find(e.first);
			if (it == map.end())
				map.insert({e.first, compile_map_entry(name, e.second)});
			else
			{
				if (it->second.m_code != e.second)
				{
					pstring msg = plib::pfmt("nltool: found hash conflict in {1}, netlist {2}")(file, name);
					throw netlist::nl_exception(msg);
				}
			}
		}

		nt.exec().stop();
	}
	catch (netlist::nl_exception &e)
	{
		perr("{} : Netlist exception : {}\n", file, e.text());
	}
	catch (plib::pexception &e)
	{
		perr("{} : Netlist exception : {}\n", file, e.text());
	}
}

void tool_app_t::static_compile()
{

	netlist::solver::static_compile_target target = netlist::solver::CXX_STATIC;

	if (!(opt_dir.was_specified() ^ opt_out.was_specified()))
		throw netlist::nl_exception("either --dir or --output option needed");

	if (opt_dir.was_specified())
	{
		if (opt_files().size() != 1)
			throw netlist::nl_exception("nltool: static_compile needs exactly one file");

		compile_map mp;

		compile_one_and_add_to_map(opt_files()[0], opt_name(), target, mp);

		for (auto &e : mp)
		{
			auto sout(std::ofstream(opt_dir() + "/" + e.first + ".c" ));
			sout << e.second.m_code;
		}
	}
	else
	{
		compile_map map;

		for (const auto &f : opt_files())
		{
			std::vector<pstring> names;
			if (opt_name.was_specified())
				names.push_back(opt_name());
			else
			{
				plib::putf8_reader r = plib::putf8_reader(plib::make_unique<std::ifstream>(plib::filesystem::u8path(f)));
				if (r.stream().fail())
					throw netlist::nl_exception(netlist::MF_FILE_OPEN_ERROR(f));
				r.stream().imbue(std::locale::classic());
				pstring line;
				while (r.readline(line))
				{
					if (plib::startsWith(line, "//NL_CONTAINS "))
					{
						auto sp = plib::psplit(plib::trim(line.substr(13)), " ", true);
						for (auto &e : sp)
							names.push_back(e);
					}
				}

				if (names.empty())
				{
					pstring name = plib::util::basename(f, ".cpp");
					if (plib::startsWith(name, "nl_"))
						name = name.substr(3);
					names.push_back(name);
				}

			}
			for (auto &name : names)
			{
				if (!opt_quiet())
					pout("Processing {}({}) ... \n", name, f);

				compile_one_and_add_to_map(f, name, target, map);
			}
		}
		std::ofstream sout = std::ofstream(opt_out());

		sout << "#include \"plib/pdynlib.h\"\n\n";
		for (auto &e : map)
		{
			sout << "// " << e.second.m_module << "\n";
			sout << e.second.m_code;
		}
		sout << "plib::dynlib_static_sym nl_static_solver_syms[] = {\n";
		for (auto &e : map)
		{
			sout << "// " << e.second.m_module << "\n";
			sout << "\t{\"" << e.first << "\", reinterpret_cast<void *>(&" << e.first << ")},\n";
		}
		sout << "{\"\", nullptr}\n";
		sout << "};\n";

	}
}



// "Description: The Swiss army knife for timing purposes\n"
// "    which has a ton of applications.\n"
// "DipAlias: GND,TRIG,OUT,RESET,VCC,DISCH,THRES,CONT\n"
// "Package: DIP\n"
// "NamingConvention: Naming conventions follow Texas Instruments datasheet\n"
// "Limitations: Internal resistor network currently fixed to 5k\n"
// "     more limitations\n"
// "FunctionTable:\n"
//

struct doc_ext
{
	pstring id;
	pstring title;
	pstring description;
	std::vector<pstring> pinalias;
	pstring package;
	pstring namingconventions;
	pstring limitations;
	pstring functiontable;
	std::vector<pstring> example;
};

static doc_ext read_docsrc(const pstring &fname, const pstring &id)
{
	//printf("file %s\n", fname.c_str());
	plib::putf8_reader r = plib::putf8_reader(plib::make_unique<std::ifstream>(plib::filesystem::u8path(fname)));
	if (r.stream().fail())
		throw netlist::nl_exception(netlist::MF_FILE_OPEN_ERROR(fname));
	r.stream().imbue(std::locale::classic());
	doc_ext ret;

	pstring l;
	if (!r.readline(l))
		return ret;
	do
	{
		l = plib::trim(l);
		if (plib::startsWith(l, "//-"))
		{
			l = plib::trim(l.substr(3));
			if (l != "")
			{
				auto a(plib::psplit(l, ":", true));
				if (a.empty() || (a.size() > 2))
					throw netlist::nl_exception(l+" size mismatch");
				pstring n(plib::trim(a[0]));
				pstring v(a.size() < 2 ? "" : plib::trim(a[1]));
				if (n == "Identifier")
				{
					if (!r.readline(l) || ret.id == id)
						return (ret.id == id ? ret : doc_ext());
					ret.id = v;
				}
				else
				{
					while (r.readline(l))
					{
						l = plib::ltrim(l);
						if (!(plib::startsWith(l, "//-  ") || plib::startsWith(l, "//-\t"))
							&& !(plib::rtrim(l) == "//-"))
							break;
						v = v + "\n" + l.substr(3);
					}
					if (n == "Title")
						ret.title = plib::trim(v);
					else if (n == "Pinalias")
						ret.pinalias = plib::psplit(plib::trim(v),",",true);
					else if (n == "Description")
						ret.description = v;
					else if (n == "Package")
						ret.package = plib::trim(v);
					else if (n == "NamingConvention")
						ret.namingconventions = v;
					else if (n == "Limitations")
						ret.limitations = v;
					else if (n == "FunctionTable")
						ret.functiontable = v;
					else if (n == "Example")
					{
						ret.example = plib::psplit(plib::trim(v),",",true);
						if (ret.example.size() != 2 && !ret.example.empty())
							throw netlist::nl_exception("Example requires 2 parameters, but found {1}", ret.example.size());
					}
					else
						throw netlist::nl_exception(n);
				}
			}
		}
		else if (!r.readline(l))
			return (ret.id == id ? ret : doc_ext());
	} while (true);
	//return ret;
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
	pstring avs;
	for (const auto &s : v)
		if (!plib::startsWith(s, "@"))
			vs += ", p" + plib::replace_all(plib::replace_all(s, "+", ""), ".", "_");
		else
			avs += ", " + s.substr(1);

	mac_out("// usage       : " + e->name() + "(name" + vs + ")", false);
	if (avs != "")
		mac_out("// auto connect: " + avs.substr(2), false);

	mac_out("#define " + e->name() + "(...)");
	mac_out("\tNET_REGISTER_DEVEXT(" + e->name() +", __VA_ARGS__)", false);
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
	if (!v.empty())
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
	if (opt_files().size() > 0)
		throw netlist::nl_exception("Header doesn't support input files, but {1} where given", opt_files().size());

	netlist_tool_t nt(*this, "netlist", opt_boostlib());

	nt.log().verbose.set_enabled(false);
	nt.log().info.set_enabled(false);

	nt.parser().register_source<netlist::source_proc_t>("dummy", &netlist_dummy);
	nt.parser().include("dummy");

	pout("// license:GPL-2.0+\n");
	pout("// copyright-holders:Couriersud\n");
	pout("#ifndef NLD_DEVINC_H\n");
	pout("#define NLD_DEVINC_H\n");
	pout("\n");
	pout("#ifndef __PLIB_PREPROCESSOR__\n");
	pout("\n");
	pout("// ----------------------------------------------------------------------------\n");
	pout("//  Netlist Macros\n");
	pout("// ---------------------------------------------------------------------------\n");
	pout("\n");

	pstring last_source("");

	for (auto &e : nt.parser().factory())
	{
		bool found(opt_pattern().size() == 0);

		for (auto &p : opt_pattern())
			found |= (e->name().find(p) != pstring::npos);

		if (found)
		{
			if (last_source != e->source().file_name())
			{
				last_source = e->source().file_name();
				pout("{1}\n", plib::rpad(pstring("// "), pstring("-"), opt_linewidth()));
				pout("{1}{2}\n", "// Source: ", plib::replace_all(e->source().file_name(), "../", ""));
				pout("{1}\n", plib::rpad(pstring("// "), pstring("-"), opt_linewidth()));
			}
			header_entry(e.get());
		}
	}
	pout("#endif // __PLIB_PREPROCESSOR__\n");
	pout("#endif\n");
	nt.exec().stop();

}

void tool_app_t::create_docheader()
{
	netlist_tool_t nt(*this, "netlist", opt_boostlib());

	nt.log().verbose.set_enabled(false);
	nt.log().info.set_enabled(false);

	nt.parser().register_source<netlist::source_proc_t>("dummy", &netlist_dummy);
	nt.parser().include("dummy");

	std::vector<pstring> devs;
	for (auto &e : nt.parser().factory())
		devs.push_back(e->name());
	std::sort(devs.begin(), devs.end(), [&](pstring &a, pstring &b) { return a < b; });

	pout("// license:GPL-2.0+\n");
	pout("// copyright-holders:Couriersud\n");
	pout("\n");
	pout("// ----------------------------------------------------------------------------\n");
	pout("//  Automatically created file. DO NOT MODIFY.\n");
	pout("// ---------------------------------------------------------------------------\n");
	pout("///\n");
	pout("/// \\page devices Devices\n");
	pout("///\n");
	pout("/// Below is a list of all the devices currently known to the system ...\n");
	pout("///\n");

	for (auto &s : devs)
		pout("/// - @subpage {1}\n", s);

	pout("\n");

	for (auto &e : nt.parser().factory())
	{
		pout("//! [{1} csynopsis]\n", e->name());
		header_entry(e.get());
		pout("//! [{1} csynopsis]\n", e->name());
		pout("//! [{1} synopsis]\n", e->name());
		mac(e.get());
		pout("//! [{1} synopsis]\n", e->name());
	}

	poutprefix("", "");
	poutprefix("///", "");
	//poutprefix("///", " @file ");
	poutprefix("///", " @page '' "); // FIXME: snippets and pages need to be separate files
	poutprefix("", "");

	for (auto &e : nt.parser().factory())
	{
		auto d(read_docsrc(e->source().file_name(), e->name()));

		if (d.id != "")
		{

			poutprefix("///", "");
			//poutprefix("///", "  @file {}", e->sourcefile());
			poutprefix("///", "");
			poutprefix("///", "  @page {} {}", d.id, d.title);
			poutprefix("///", "");
			poutprefix("///", "  {}", d.description);
			poutprefix("///", "");
			poutprefix("///", "  @section {}_1 Synopsis", d.id);
			poutprefix("///", "");
			poutprefix("///", "  @snippet devsyn.dox.h {} synopsis", d.id);
			poutprefix("///", "");
			poutprefix("///", "  @section {}_11 C Synopsis", d.id);
			poutprefix("///", "");
			poutprefix("///", "  @snippet devsyn.dox.h {} csynopsis", d.id);
			poutprefix("///", "");
			poutprefix("///", "  @section {}_2 Connection Diagram", d.id);
			poutprefix("///", "");

			if (!d.pinalias.empty())
			{
				poutprefix("///", "  <pre>");
				if (d.package == "DIP")
				{
					auto & pins = d.pinalias;
					//const int w = 8;
					poutprefix("///", " {1:10} +--------+", " ");
					for (std::size_t i=0; i<pins.size()/2; i++)
					{
						poutprefix("///", " {1:10} |{2:-2}    {3:2}| {4:-10}",
							pins[i], i+1, pins.size()-i, pins[pins.size()-i-1]);
					}
					poutprefix("///", " {1:10} +--------+", " ");
				}
				poutprefix("///", "  </pre>");
			}
			poutprefix("///", "");
			poutprefix("///", "  {}", d.namingconventions);
			poutprefix("///", "");
			poutprefix("///", "  @section {}_3 Function Table", d.id);
			poutprefix("///", "");
			if (d.functiontable == "")
				poutprefix("///", "  Please refer to the datasheet.");
			else
				poutprefix("///", "  {}", d.functiontable);
			poutprefix("///", "");
			poutprefix("///", "  @section {}_4 Limitations", d.id);
			poutprefix("///", "");
			poutprefix("///", "  {}", d.limitations);
			if (!d.example.empty())
			{
				poutprefix("///", "");
				poutprefix("///", "  @section {}_5 Example", d.id);
				poutprefix("///", "  @snippet {1} {2}", d.example[0], d.example[1]);
				poutprefix("///", "");
				poutprefix("", "");
			}
		}
	}
	nt.exec().stop();
}


// -------------------------------------------------
//    listdevices - list all known devices
// -------------------------------------------------

void tool_app_t::listdevices()
{
	netlist_tool_t nt(*this, "netlist", opt_boostlib());

	nt.log().verbose.set_enabled(false);
	nt.log().info.set_enabled(false);
	nt.log().warning.set_enabled(false);

	netlist::factory::list_t &list = nt.parser().factory();

	nt.parser().register_source<netlist::source_proc_t>("dummy", &netlist_dummy);
	nt.parser().include("dummy");
	nt.setup().prepare_to_run();

	std::vector<netlist::unique_pool_ptr<netlist::core_device_t>> devs;

	for (auto & fl : list)
	{
		pstring out = plib::pfmt("{1:-20} {2}(<id>")(fl->name())(fl->name());

		netlist::factory::element_t *f;
		nt.parser().register_dev(fl->name(), fl->name() + "_lc",
			std::vector<pstring>(), &f);

		auto d = f->make_device(nt.pool(), nt, f->name() + "_lc");
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
		if (!terms.empty())
		{
			pstring t = "";
			for (auto & j : terms)
				t += "," + j;
			pout("\tTerminals: {}\n", t.substr(1));
		}
		devs.emplace_back(std::move(d));
	}
}

// -------------------------------------------------
//    convert - convert spice et al to netlist
// -------------------------------------------------

void tool_app_t::convert()
{
	pstring contents;
	std::stringstream ostrm;
	ostrm.imbue(std::locale::classic());

	if (opt_files().size() > 1)
		throw netlist::nl_exception("nltool: convert needs exactly one file");

	if (opt_files().empty() || opt_files()[0] == "-")
	{
		plib::copystream(ostrm, std::cin);
	}
	else
	{
		std::ifstream strm(plib::filesystem::u8path(opt_files()[0]));
		if (strm.fail())
			throw netlist::nl_exception(netlist::MF_FILE_OPEN_ERROR(opt_files()[0]));
		strm.imbue(std::locale::classic());
		plib::copystream(ostrm, strm);
	}
	contents = pstring(ostrm.str());

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
	// present result
	pout.write(result);
}

// -------------------------------------------------
//    main - primary entry point
// -------------------------------------------------

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
		"nltool serves as the Swiss Army knife to run, test and convert netlists.\n\n"
		"Commands may accept one or more files depending on the functionality.\n"
		"If no file is provided, standard input is used.",
		"nltool [option]... [files]...");
}

int tool_app_t::execute()
{
	tool_app_t opts;

	if (opt_help())
	{
		pout(usage());
		return 0;
	}

	if (opt_version())
	{
		pout(
			"nltool (netlist) {1}\n"
			"Copyright (C) 2020 Couriersud\n"
			"License GPLv2+: GNU GPL version 2 or later <http://gnu.org/licenses/gpl.html>.\n"
			"This is free software: you are free to change and redistribute it.\n"
			"There is NO WARRANTY, to the extent permitted by law.\n\n"
			"Written by Couriersud.\n", netlist::netlist_state_t::version());
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

	m_defines = opt_defines();
	m_defines.emplace_back("NLTOOL_VERSION=" + netlist::netlist_state_t::version());
	if (opt_prepro())
		m_defines.emplace_back("__PREPROCESSOR_DEBUG__=1");

	try
	{
		plib::fpsignalenabler::global_enable(opt_fperr());
		plib::fpsignalenabler fpprotect(plib::FP_DIVBYZERO | plib::FP_UNDERFLOW | plib::FP_OVERFLOW | plib::FP_INVALID);

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
	catch (plib::pexception &e)
	{
		perr("Exception caught: {}\n", e.text());
		return 2;
	}

#if 0
	std::cout.imbue(std::locale("de_DE.utf8"));
	std::cout.imbue(std::locale("C.UTF-8"));
	std::cout << std::fixed << 20.003 << "\n";
	std::cout << std::setw(20) << std::left << "01234567890" << "|" << "\n";
	std::cout << std::setw(20) << "Общая ком" << "|" << "\n";
	std::cout << "Общая ком" << pstring(20 - pstring("Общая ком").length(), ' ') << "|" << "\n";
	std::cout << plib::pfmt("{:20}")("Общая ком") << "|" << "\n";

	//char x = 'a';
	//auto b= U'щ';

	auto b= U'\U00000449';
	std::cout << "b: <" << b << ">";
#endif
	return 0;
}

PMAIN(tool_app_t)
