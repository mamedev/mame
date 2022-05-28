// license:BSD-3-Clause
// copyright-holders:Couriersud

// ***************************************************************************
//
//    nltool.cpp
//
//    Simple tool to debug netlists outside MAME.
//
// ***************************************************************************

#include "plib/pdynlib.h"
#include "core/setup.h"
#include "devices/net_lib.h"
#include "nl_errstr.h"
#include "nl_parser.h"
#include "nl_setup.h"
#include "plib/pmain.h"
#include "plib/pstrutil.h"
#include "solver/nld_solver.h"
#include "tools/nl_convert.h"

#include "plib/ptests.h"

#include <cstdio> // scanf
#include <iomanip> // scanf
#include <ios>
#include <iostream> // scanf

#ifndef NL_DISABLE_DYNAMIC_LOAD
#define NL_DISABLE_DYNAMIC_LOAD 0
#endif

// FIXME: "NamingConvention", "FunctionTable:", "Pinalias" need to be renamed here
//        and in all source locations.
// spell-checker: words Pinalias, devsyn, csynopsis, subpage

//
// Names
// spell-checker: words Woodbury, Raphson,
//
// FIXME: These are either defined in other parts or have more complex consequences
// spell-checker: words nlconst, rpad, pfmt, pexception, pbinary, dynlib, pstring, psplit,putf
//
// FIXME: nlconst -> nl_const
//
// Specific technical terms
// spell-checker: words rinf

extern const plib::static_library::symbol nl_static_solver_syms[];

// Forward declarations

class netlist_tool_t;

class tool_app_t : public plib::app
{
public:
	tool_app_t() :
		plib::app(),
		m_warnings(0),
		m_errors(0),

		opt_grp1(*this,     "General options",              "The following options apply to all commands."),
		opt_cmd (*this,     "c", "cmd",         0,          std::vector<pstring>({"run","validate","convert","list-devices","list-models","static","header","doc-header","tests"}), "run|validate|convert|list-devices|list-models|static|header|doc-header|tests"),
		opt_includes(*this, "I", "include",                 "Add the directory to the list of directories to be searched for header files. This option may be specified repeatedly."),
		opt_defines(*this,  "D", "define",                  "predefine value as macro, e.g. -Dname=value. If '=value' is omitted predefine it as 1. This option may be specified repeatedly."),
		opt_data_folders(*this, "d", "data",                    "where to look for data files"),
		opt_verb(*this,     "v", "verbose",                 "be verbose - this produces lots of output"),
		opt_quiet(*this,    "q", "quiet",                   "be quiet - no warnings"),
		opt_pre_pro(*this,  "",  "prepro",                  "output preprocessor output to stderr"),
		opt_progress(*this, "",  "progress",                "show progress bar on longer operations"),

		opt_files(*this, "files to process"),

		opt_version(*this,  "",  "version",                 "display version and exit"),
		opt_help(*this,     "h", "help",                    "display help and exit"),

		opt_grp2(*this,     "Options for run and static commands",   "These options apply to run and static commands."),
		opt_name(*this,     "n", "name",        "",         "the netlist in file specified by ""-f"" option to run; default is first one"),

		opt_grp3(*this,     "Options for static command",   "These options apply to static command."),
		opt_dir(*this,      "d", "dir",        "",          "output directory for the generated files."),
		opt_out(*this,      "o", "output",     "",          "single output file for the generated code.\nEither --dir or --output can be specificied"),
//      opt_static_include(*this, "", "static-include",     "write static solvers to individual files\nincluded by file specified with --output.\n--dir must be path to \"generated/static\" path in netlist folder."),
		opt_static_include(*this, "", "static-include",     "write static solvers to individual files included by file specified with --output. --dir must be path to \"generated/static\" path in netlist folder."),

		opt_grp4(*this,     "Options for run command",      "These options are only used by the run command."),
		opt_ttr (*this,     "t", "time_to_run", 1,          "time to run the emulation (seconds)"),
		opt_boost_lib(*this,  "",  "boost-lib", "builtin",   "generic: will use generic solvers.\nbuiltin: Use optimized solvers compiled in.\nsome_lib.so: Use library with precompiled solvers."),
		opt_stats(*this,    "s", "statistics",              "gather runtime statistics"),
		opt_logs(*this,     "l", "log" ,                    "define terminal to log. This option may be specified repeatedly."),
		opt_inp(*this,      "i", "input",       "",         "input file to process (default is none)"),
		opt_load_state(*this,"", "load-state",   "",        "load state from file and continue from there"),
		opt_save_state(*this,"", "save-state",   "",        "save state to file at end of run"),
		opt_fp_error(*this,  "", "fp-error",                "raise exception on floating point errors. This is intended to be used during debugging."),

		opt_grp5(*this,     "Options for convert command",  "These options are only used by the convert command."),
		opt_type(*this,     "y", "type",        0,           std::vector<pstring>({"spice","eagle","rinf"}), "type of file to be converted: spice,eagle,rinf"),

		opt_grp6(*this,     "Options for validate command", "These options are only used by the validate command."),

		opt_grp7(*this,     "Options for header command",   "These options are only used by the header command."),
		opt_tab_width(*this, "", "tab-width", 4,            "Tab width for output."),
		opt_line_width(*this,"", "line-width", 72,          "Line width for output."),
		opt_pattern(*this, "", "pattern",                   "Pattern to match against device names. If the device name contains pattern, the device will be included in the output. Multiple patterns can be specified, if none is given, all devices will be output."),

		opt_ex1(*this,     "nltool -c run -t 3.5 -n cap_delay nl_examples/cdelay.c",
				"Run netlist \"cap_delay\" from file nl_examples/cdelay.c for 3.5 seconds"),
		opt_ex2(*this,     "nltool --cmd=list-devices",
				"List all known devices."),
		opt_ex3(*this,     "nltool --cmd=header --tab-width=8 --line-width=80",
				"Create the header file needed for including netlists as code."),
		opt_ex4(*this,     "nltool --cmd static --output src/lib/netlist/generated/static_solvers.cpp src/mame/audio/nl_*.cpp src/mame/machine/nl_*.cpp",
				"Create static solvers for the MAME project using one single source file."),
		opt_ex5(*this,     "nltool --cmd static --output src/lib/netlist/generated/static_solvers.cpp --dir src/lib/netlist/generated/static --static_include src/mame/audio/nl_*.cpp src/mame/machine/nl_*.cpp",
				"Create static solvers for the MAME project using a single source file which includes generated solvers written to --dir folder."),
		opt_ex6(*this,     "nltool --cmd tests",
			"Run unit tests. In case the unit tests are not linked in, this will do nothing.")
		{}

	int execute() override;
	pstring usage() override;

	template<typename... ARGS>
	void output_with_prefix(const pstring &prefix, const pstring &fmt, ARGS&&... args)
	{
		pstring res = plib::pfmt(fmt)(std::forward<ARGS>(args)...);
		auto lines(plib::psplit(res, '\n', false));
		if (lines.empty())
			std_out(prefix + "\n");
		else
			for (auto &l : lines)
				std_out(prefix + l + "\n");
	}

	int m_warnings;
	int m_errors;
private:
	plib::option_group  opt_grp1;
	plib::option_str_limit<unsigned> opt_cmd;
	plib::option_vec    opt_includes;
	plib::option_vec    opt_defines;
	plib::option_vec    opt_data_folders;
	plib::option_bool   opt_verb;
	plib::option_bool   opt_quiet;
	plib::option_bool   opt_pre_pro;
	plib::option_bool   opt_progress;
	plib::option_args   opt_files;
	plib::option_bool   opt_version;
	plib::option_bool   opt_help;

	plib::option_group  opt_grp2;
	plib::option_str    opt_name;

	plib::option_group  opt_grp3;
	plib::option_str    opt_dir;
	plib::option_str    opt_out;
	plib::option_bool   opt_static_include;

	plib::option_group  opt_grp4;
	plib::option_num<netlist::nl_fptype> opt_ttr;
	plib::option_str    opt_boost_lib;
	plib::option_bool   opt_stats;
	plib::option_vec    opt_logs;
	plib::option_str    opt_inp;
	plib::option_str    opt_load_state;
	plib::option_str    opt_save_state;
	plib::option_bool   opt_fp_error;

	plib::option_group  opt_grp5;
	plib::option_str_limit<unsigned> opt_type;

	plib::option_group  opt_grp6;
	plib::option_group  opt_grp7;
	plib::option_num<unsigned> opt_tab_width;
	plib::option_num<unsigned> opt_line_width;
	plib::option_vec     opt_pattern;
	plib::option_example opt_ex1;
	plib::option_example opt_ex2;
	plib::option_example opt_ex3;
	plib::option_example opt_ex4;
	plib::option_example opt_ex5;
	plib::option_example opt_ex6;

	struct compile_map_entry
	{
		compile_map_entry(const pstring &mod, const pstring &code)
		: m_code(code)
		{
			m_modules.push_back(mod);
		}
		pstring m_code;
		std::vector<pstring> m_modules;
	};

	using compile_map = std::map<pstring, compile_map_entry>;

	void logger(plib::plog_level l, const pstring &ls);

	void run_with_progress(netlist_tool_t &nt, netlist::netlist_time_ext start, netlist::netlist_time_ext duration);

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
	void create_doc_header();

	void list_models();
	void list_devices();

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

	plib::istream_uptr stream(const pstring &file) override
	{
		pstring name = m_folder + "/" + file;
		plib::istream_uptr strm(std::make_unique<plib::ifstream>(plib::filesystem::u8path(name)), plib::filesystem::u8path(name));
		if (strm->fail())
			return plib::istream_uptr();

		strm->imbue(std::locale::classic());
		return strm;
	}

private:
	pstring m_folder;
};

class netlist_tool_t : public netlist::netlist_state_t
{
public:

	netlist_tool_t(plib::plog_delegate logger, const pstring &name, const pstring &boost_lib)
	: netlist::netlist_state_t(name, logger)
	{
		if (boost_lib == "builtin")
			set_static_solver_lib(std::make_unique<plib::static_library>(nl_static_solver_syms));
		else if (boost_lib == "generic")
			set_static_solver_lib(std::make_unique<plib::static_library>(nullptr));
		else if (NL_DISABLE_DYNAMIC_LOAD)
			throw netlist::nl_exception("Dynamic library loading not supported due to project security concerns.");
		else
			//#pstring libpath = plib::util::environment("NL_BOOSTLIB", plib::util::buildpath({".", "nlboost.so"}));
			set_static_solver_lib(std::make_unique<plib::dynamic_library>(boost_lib));
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


struct input_t
{
	input_t(const netlist::setup_t &setup, const putf8string &line)
	: m_value(netlist::nlconst::zero())
	{
		std::array<char, 400> buf; // NOLINT(cppcoreguidelines-pro-type-member-init)
		double t(0);
		double val(0);

		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
		int e = std::sscanf(line.c_str(), "%lf,%[^,],%lf", &t, buf.data(), &val);
		if (e != 3)
			throw netlist::nl_exception(plib::pfmt("error {1} scanning line {2}\n")(e)(line));
		m_value = static_cast<netlist::nl_fptype>(val);
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
				plib::downcast<netlist::param_fp_t &>(m_param.param()).set(m_value);
				break;
			case netlist::param_t::INTEGER:
				plib::downcast<netlist::param_int_t &>(m_param.param()).set(static_cast<int>(m_value));
				break;
			case netlist::param_t::LOGIC:
				plib::downcast<netlist::param_logic_t &>(m_param.param()).set(static_cast<bool>(m_value));
				break;
		}
	}

	netlist::netlist_time_ext m_time;
	netlist::param_ref_t m_param;
	netlist::nl_fptype m_value;
};

static std::vector<input_t> read_input(const netlist::setup_t &setup, const pstring &fname)
{
	std::vector<input_t> ret;
	if (!fname.empty())
	{
		plib::putf8_reader r = plib::putf8_reader(std::make_unique<plib::ifstream>(plib::filesystem::u8path(fname)));
		if (r.stream().fail())
			throw netlist::nl_exception(netlist::MF_FILE_OPEN_ERROR(fname));
		r.stream().imbue(std::locale::classic());
		putf8string l;
		while (r.read_line(l))
		{
			if (!l.empty())
			{
				input_t inp(setup, l);
				ret.push_back(inp);
			}
		}
	}
	return ret;
}

void tool_app_t::run_with_progress(netlist_tool_t &nt, netlist::netlist_time_ext start, netlist::netlist_time_ext duration)
{
	if (!opt_progress())
		nt.exec().process_queue(duration);
	else
	{
		auto now = nt.exec().time();
		auto end = now + duration;
		// run to next_sec
		while (now < end)
		{
			auto elapsed = now - start;
			auto elapsed_sec = elapsed.in_sec() + 1;

			auto next_sec = start + netlist::netlist_time_ext::from_sec(elapsed_sec);
			if (end < next_sec)
			{
				nt.exec().process_queue(end - now);
			}
			else
			{
				nt.exec().process_queue(next_sec - now);
				std_out("progress {1:4}s : {2}\r", elapsed_sec, pstring(gsl::narrow_cast<std::size_t>(elapsed_sec), '*'));
				std_out.flush();
			}
			now = nt.exec().time();
		}
	}
}

void tool_app_t::logger(plib::plog_level l, const pstring &ls)
{
	pstring err = plib::pfmt("{}: {}\n")(l.name())(ls.c_str());
	if (l == plib::plog_level::WARNING)
		m_warnings++;
	if (l == plib::plog_level::ERROR)
		m_errors++;
	if (l == plib::plog_level::FATAL)
		m_errors++;
	std_out("{}", err);
}

void tool_app_t::run()
{
	plib::chrono::timer<plib::chrono::system_ticks> t;
	std::vector<input_t> inputs;
	netlist::netlist_time_ext duration;

	if (opt_files().size() != 1)
		throw netlist::nl_exception("nltool: run needs exactly one file");

	if (!plib::util::exists(opt_files()[0]))
		throw netlist::nl_exception("nltool: file doesn't exists: {}", opt_files()[0]);

	t.start();

	netlist_tool_t nt(plib::plog_delegate(&tool_app_t::logger, this), "netlist", opt_boost_lib());

	nt.exec().enable_stats(opt_stats());

	if (!opt_verb())
		nt.log().verbose.set_enabled(false);
	if (opt_quiet())
		nt.log().info.set_enabled(false);

	nt.read_netlist(opt_files()[0], opt_name(),
			opt_logs(),
			m_defines, opt_data_folders(), opt_includes());

	// Inputs must be read before reset -> will clear setup and parser
	inputs = read_input(nt.setup(), opt_inp());
	nt.free_setup_resources();
	nt.exec().reset();

	duration = netlist::netlist_time_ext::from_fp(opt_ttr());
	t.stop();
	std_out("startup time ==> {1:5.3f}\n", t.as_seconds<netlist::nl_fptype>() );

	// FIXME: error handling
	if (opt_load_state.was_specified())
	{
		plib::ifstream strm(plib::filesystem::u8path(opt_load_state()));
		if (strm.fail())
			throw netlist::nl_exception(netlist::MF_FILE_OPEN_ERROR(opt_load_state()));
		strm.imbue(std::locale::classic());
		plib::pbinary_reader reader(strm);
		std::vector<char> load_state;
		reader.read(load_state);
		nt.load_state(load_state);
		std_out("Loaded state, run will continue at {1:.6f}\n", nt.exec().time().as_double());
	}

	t.reset();

	netlist::netlist_time_ext start = nt.exec().time();
	{
		std_out("runnning ...\n");
		unsigned pos = 0;
		netlist::netlist_time_ext nlt = start;
		auto t_guard(t.guard());

		while (pos < inputs.size()
				&& inputs[pos].m_time < duration
				&& inputs[pos].m_time >= nlt)
		{
			run_with_progress(nt, start, inputs[pos].m_time - nlt);
			inputs[pos].setparam();
			nlt = inputs[pos].m_time;
			pos++;
		}

		if (duration > nlt)
			run_with_progress(nt, start, duration - nlt);
		else
		{
			std_out("end time {1:.6f} less than saved time {2:.6f}\n",
					duration.as_double(), nlt.as_double());
			duration = nlt;
		}
	}

	if (opt_save_state.was_specified())
	{
		auto save_state = nt.save_state();
		plib::ofstream strm(plib::filesystem::u8path(opt_save_state()), std::ios_base::binary);
		if (strm.fail())
			throw plib::file_open_e(opt_save_state());
		strm.imbue(std::locale::classic());

		plib::pbinary_writer writer(strm);
		writer.write(save_state);
	}
	nt.exec().stop();

	if (opt_progress())
		std_out("\n");
	auto emulation_time(t.as_seconds<netlist::nl_fptype>());
	std_out("{1:f} seconds emulation took {2:f} real time ==> {3:5.2f}%\n",
			(duration - start).as_fp<netlist::nl_fptype>(), emulation_time,
			(duration - start).as_fp<netlist::nl_fptype>() / emulation_time * netlist::nlconst::hundred());
}

void tool_app_t::validate()
{
	netlist_tool_t nt(plib::plog_delegate(&tool_app_t::logger, this), "netlist", opt_boost_lib());

	if (opt_files().size() != 1)
		throw netlist::nl_exception("nltool: validate needs exactly one file");

	if (!opt_verb())
		nt.log().verbose.set_enabled(false);
	if (opt_quiet())
		nt.log().info.set_enabled(false);

	m_errors = 0;
	m_warnings = 0;

	try
	{
		nt.read_netlist(opt_files()[0], opt_name(),
				opt_logs(),
				m_defines, opt_data_folders(), opt_includes());
	}
	catch (netlist::nl_exception &e)
	{
		std_out("Netlist exception caught: {}\n", e.text());
	}
	catch (plib::pexception &e)
	{
		std_out("plib exception caught: {}\n", e.text());
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
		netlist_tool_t nt(plib::plog_delegate(&tool_app_t::logger, this), "netlist", opt_boost_lib());

		nt.log().verbose.set_enabled(false);
		nt.log().info.set_enabled(false);
		nt.log().warning.set_enabled(false);

		nt.read_netlist(file, name,
				opt_logs(),
				m_defines, opt_data_folders(), opt_includes());

		// need to reset ...

		nt.free_setup_resources();
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
				if (!plib::container::contains(it->second.m_modules, name))
					it->second.m_modules.push_back(name);
			}
		}

		nt.exec().stop();
	}
	catch (netlist::nl_exception &e)
	{
		std_err("{} : Netlist exception : {}\n", file, e.text());
	}
	catch (plib::pexception &e)
	{
		std_err("{} : Netlist exception : {}\n", file, e.text());
	}
}

void tool_app_t::static_compile()
{

	netlist::solver::static_compile_target target = netlist::solver::CXX_STATIC;

	if (!opt_static_include() && ((opt_dir.was_specified() ^ opt_out.was_specified()) == 0))
		throw netlist::nl_exception("either --dir or --output option needed");

	if (opt_static_include() && (!opt_dir.was_specified() || !opt_out.was_specified()))
		throw netlist::nl_exception("--split requires both --dir and --output option");

	if (!opt_static_include() && opt_dir.was_specified())
	{
		if (opt_files().size() != 1)
			throw netlist::nl_exception("nltool: static_compile needs exactly one file");

		compile_map mp;

		compile_one_and_add_to_map(opt_files()[0], opt_name(), target, mp);

		for (auto &e : mp)
		{
			plib::ofstream output_stream(opt_dir() + "/" + e.first + ".c" );
			output_stream << putf8string(e.second.m_code);
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
				plib::putf8_reader r = plib::putf8_reader(std::make_unique<plib::ifstream>(plib::filesystem::u8path(f)));
				if (r.stream().fail())
					throw netlist::nl_exception(netlist::MF_FILE_OPEN_ERROR(f));
				r.stream().imbue(std::locale::classic());
				putf8string line;
				while (r.read_line(line))
				{
					if (plib::startsWith(line, "//NL_CONTAINS "))
					{
						auto sp = plib::psplit(pstring(plib::trim(line.substr(13))), ' ', true);
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
			std::sort(names.begin(), names.end());
			for (auto &name : names)
			{
				if (!opt_quiet())
					std_out("Processing {}({}) ... \n", name, f);

				compile_one_and_add_to_map(f, name, target, map);
			}
		}
		plib::ofstream output_stream(opt_out());
		if (output_stream.fail())
			throw netlist::nl_exception(netlist::MF_FILE_OPEN_ERROR(opt_out()));

		output_stream << "// spell-checker: disable\n\n";
		output_stream << "#include \"plib/pdynlib.h\"\n\n";
		output_stream << "#if !defined(__EMSCRIPTEN__)\n\n";

		std::vector<std::pair<pstring,pstring>> sort_map;
		for (auto &e : map)
			sort_map.emplace_back(e.second.m_modules[0], e.first);

		std::sort(sort_map.begin(), sort_map.end(), [](const auto &a, const auto &b) {
			return a.first < b.first ? true : (a.first == b.first && a.second < b.second);
		});

		pstring last;
		plib::ofstream output_splitted;
		for (auto &se : sort_map)
		{
			const auto &e = map.find(se.second);
			pstring mods;
			for (auto &s : e->second.m_modules)
				mods += s + ",";
			mods = plib::left(mods, mods.length()-1);
			if (opt_static_include())
			{
				auto fname = pstring("sc_") + e->second.m_modules[0] + ".hpp";
				auto fpath(plib::util::build_path({opt_dir(), fname}));
				if (last != fpath)
				{
					last = fpath;
					if (output_splitted.is_open())
						output_splitted.close();
					output_splitted.open(fpath);
					if (output_stream.fail())
						throw netlist::nl_exception(netlist::MF_FILE_OPEN_ERROR(fpath));
					output_stream << "// spell-checker: disable\n\n";
					output_stream << "// " << putf8string(mods) << "\n";
					output_stream << "#include \"generated/static/" << fname << "\"\n";
				}
				output_splitted << "// " << putf8string(mods) << "\n";
				output_splitted << putf8string(e->second.m_code);
			}
			else
			{
				output_stream << "// " << putf8string(mods) << "\n";
				output_stream << putf8string(e->second.m_code);
			}
		}

		output_stream << "#endif\n\n";
		output_stream << "extern const plib::static_library::symbol nl_static_solver_syms[];\n";
		output_stream << "const plib::static_library::symbol nl_static_solver_syms[] = {\n";
		output_stream << "#if !defined(__EMSCRIPTEN__)\n\n";
		for (auto &se : sort_map)
		{
			const auto &e = map.find(se.second);
			pstring mods;
			for (auto &s : e->second.m_modules)
				mods += s + ",";
			output_stream << "// " << putf8string(plib::left(mods, mods.length()-1)) << "\n";
			output_stream << "\t{\"" << putf8string(e->first) << "\", reinterpret_cast<void *>(&" << putf8string(e->first) << ")}, // NOLINT\n";
		}
		output_stream << "#endif\n\n";
		output_stream << "{\"\", nullptr}\n";
		output_stream << "};\n";

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
	std::vector<pstring> pin_alias;
	pstring package;
	std::vector<std::pair<pstring, pstring>> params;
	pstring naming_conventions;
	pstring limitations;
	pstring function_table;
	std::vector<pstring> example;
};

static doc_ext read_documentation_from_source(const pstring &fname, const pstring &id)
{
	plib::putf8_reader r = plib::putf8_reader(std::make_unique<plib::ifstream>(plib::filesystem::u8path(fname)));
	if (r.stream().fail())
		throw netlist::nl_exception(netlist::MF_FILE_OPEN_ERROR(fname));
	r.stream().imbue(std::locale::classic());
	doc_ext ret;

	putf8string l;
	if (!r.read_line(l))
		return ret;
	do
	{
		l = plib::trim(l);
		if (plib::startsWith(l, "//-"))
		{
			l = plib::trim(l.substr(3));
			if (!l.empty())
			{
				auto a(plib::psplit(pstring(l), ':', true));
				if (a.empty() || (a.size() > 2))
					throw netlist::nl_exception(pstring(l) + " size mismatch");
				pstring n(plib::trim(a[0]));
				pstring v(a.size() < 2 ? "" : plib::trim(a[1]));
				pstring v2(v);
				if (n == "Identifier")
				{
					if (!r.read_line(l) || ret.id == id)
						return (ret.id == id ? ret : doc_ext());
					ret = doc_ext();
					ret.id = v;
				}
				else
				{
					while (r.read_line(l))
					{
						l = plib::ltrim(l);
						if (!(plib::startsWith(l, "//-  ") || plib::startsWith(l, "//-\t"))
							&& !(plib::rtrim(l) == "//-"))
							break;
						v = v + "\n" + pstring(l.substr(3));
					}
					if (n == "Title")
						ret.title = plib::trim(v);
					else if (n == "Pinalias")
						ret.pin_alias = plib::psplit(plib::trim(v),',',true);
					else if (n == "Description")
						ret.description = v;
					else if (n == "Package")
						ret.package = plib::trim(v);
					else if (n == "NamingConvention")
						ret.naming_conventions = v;
					else if (n == "Limitations")
						ret.limitations = v;
					else if (n == "FunctionTable")
						ret.function_table = v;
					else if (n == "Param")
						ret.params.emplace_back(v2, plib::trim(v.substr(v2.length())));
					else if (n == "Example")
					{
						ret.example = plib::psplit(plib::trim(v),',',true);
						if (ret.example.size() != 2 && !ret.example.empty())
							throw netlist::nl_exception("Example requires 2 parameters, but found {1}", ret.example.size());
					}
					else
						throw netlist::nl_exception(n);
				}
			}
		}
		else if (!r.read_line(l))
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
				auto pos_mod_4 = pos % opt_tab_width();
				auto tab_adj = opt_tab_width() - pos_mod_4;
				r += plib::rpad(pstring(""), pstring(" "), tab_adj);
				pos += tab_adj;
			}
			else
			{
				r += x;
				pos++;
			}
		}
		std_out("{1}\\\n", plib::rpad(r, pstring(" "), opt_line_width()-1));
	}
	else
		std_out("{1}\n", s);
}

void tool_app_t::header_entry(const netlist::factory::element_t *e)
{
	auto v = plib::psplit(e->param_desc(), ',');
	pstring vs;
	pstring avs;
	for (const auto &s : v)
		if (!plib::startsWith(s, "@"))
			vs += ", p" + plib::replace_all(plib::replace_all(s, "+", ""), ".", "_");
		else
			avs += ", " + s.substr(1);

	mac_out("// usage       : " + e->name() + "(name" + vs + ")", false);
	if (!avs.empty())
		mac_out("// auto connect: " + avs.substr(2), false);

	mac_out("#define " + e->name() + "(...)");
	mac_out("\tNET_REGISTER_DEVEXT(" + e->name() +", __VA_ARGS__)", false);
	mac_out("", false);
}

void tool_app_t::mac(const netlist::factory::element_t *e)
{
	auto v = plib::psplit(e->param_desc(), ',');
	pstring vs;
	for (const auto &s : v)
		if (!plib::startsWith(s, "@"))
			vs += ", " + plib::replace_all(plib::replace_all(s, "+", ""), ".", "_");

	std_out("{1}(name{2})\n", e->name(), vs);
	if (!v.empty())
	{
		std_out("/*\n");
		for (const auto &s : v)
		{
			pstring r(plib::replace_all(plib::replace_all(plib::replace_all(s, "+", ""), ".", "_"), "@",""));
			if (plib::startsWith(s, "+"))
				std_out("{1:10}: Terminal\n",r);
			else if (plib::startsWith(s, "@"))
				std_out("{1:10}: Power terminal - automatically connected\n", r);
			else
				std_out("{1:10}: Parameter\n", r);
		}
		std_out("*/\n");
	}
}

void tool_app_t::create_header()
{
	if (!opt_files().empty())
		throw netlist::nl_exception("Header doesn't support input files, but {1} where given", opt_files().size());

	netlist_tool_t nt(plib::plog_delegate(&tool_app_t::logger, this), "netlist", opt_boost_lib());

	nt.log().verbose.set_enabled(false);
	nt.log().info.set_enabled(false);

	nt.parser().register_source<netlist::source_proc_t>("dummy", &netlist_dummy);
	nt.parser().include("dummy");

	std_out("// license:BSD-3-Clause\n");
	std_out("// copyright-holders:Couriersud\n");
	std_out("#ifndef NLD_DEVINC_H\n");
	std_out("#define NLD_DEVINC_H\n");
	std_out("\n");
	std_out("#ifndef __PLIB_PREPROCESSOR__\n");
	std_out("\n");
	std_out("#include ""../nl_setup.h""\n");
	std_out("\n");

	std_out("// ----------------------------------------------------------------------------\n");
	std_out("//  Netlist Macros\n");
	std_out("// ---------------------------------------------------------------------------\n");
	std_out("\n");

	pstring last_source("");

	for (auto &e : nt.parser().factory())
	{
		bool found(opt_pattern().empty());

		for (const auto &p : opt_pattern())
			found |= (e->name().find(p) != pstring::npos);

		if (found)
		{
			if (last_source != e->source().file_name())
			{
				last_source = e->source().file_name();
				std_out("{1}\n", plib::rpad(pstring("// "), pstring("-"), opt_line_width()));
				std_out("{1}{2}\n", "// Source: ", plib::replace_all(e->source().file_name(), "../", ""));
				std_out("{1}\n", plib::rpad(pstring("// "), pstring("-"), opt_line_width()));
			}
			header_entry(e.get());
		}
	}
	std_out("#endif // __PLIB_PREPROCESSOR__\n");
	std_out("#endif\n");
	nt.exec().stop();

}

void tool_app_t::create_doc_header()
{
	netlist_tool_t nt(plib::plog_delegate(&tool_app_t::logger, this), "netlist", opt_boost_lib());

	nt.log().verbose.set_enabled(false);
	nt.log().info.set_enabled(false);

	nt.parser().register_source<netlist::source_proc_t>("dummy", &netlist_dummy);
	nt.parser().include("dummy");

	std::vector<pstring> devices;
	for (auto &e : nt.parser().factory())
		devices.push_back(e->name());
	std::sort(devices.begin(), devices.end(), [&](pstring &a, pstring &b) { return a < b; });

	std_out("// license:BSD-3-Clause\n");
	std_out("// copyright-holders:Couriersud\n");
	std_out("\n");
	std_out("// ----------------------------------------------------------------------------\n");
	std_out("//  Automatically created file. DO NOT MODIFY.\n");
	std_out("// ---------------------------------------------------------------------------\n");
	std_out("///\n");
	std_out("/// \\page devices Devices\n");
	std_out("///\n");
	std_out("/// Below is a list of all the devices currently known to the system ...\n");
	std_out("///\n");

	for (auto &s : devices)
		std_out("/// - @subpage {1}\n", s);

	std_out("\n");

	std::vector<doc_ext> de_cache;

	for (auto &e : nt.parser().factory())
	{
		auto d(read_documentation_from_source(e->source().file_name(), e->name()));

		if (!d.id.empty())
		{
			std_out("//! [{1} csynopsis]\n", e->name());
			header_entry(e.get());
			std_out("//! [{1} csynopsis]\n", e->name());
			std_out("//! [{1} synopsis]\n", e->name());
			mac(e.get());
			std_out("//! [{1} synopsis]\n", e->name());
		}
		de_cache.push_back(std::move(d));
	}

	output_with_prefix("", "");
	output_with_prefix("///", "");
	//output_with_prefix("///", " @file ");
	output_with_prefix("///", " @page ''"); // FIXME: snippets and pages need to be separate files
	output_with_prefix("", "");

	for (auto &d : de_cache)
	{
		//auto d(read_documentation_from_source(e->source().file_name(), e->name()));

		if (!d.id.empty())
		{

			output_with_prefix("///", "");
			//output_with_prefix("///", "  @file {}", e->sourcefile());
			output_with_prefix("///", "");
			output_with_prefix("///", "  @page {} {}", d.id, d.title);
			output_with_prefix("///", "");
			output_with_prefix("///", "  {}", d.description);
			output_with_prefix("///", "");
			output_with_prefix("///", "  @section {}_1 Synopsis", d.id);
			output_with_prefix("///", "");
			output_with_prefix("///", "  @snippet devsyn.dox.h {} synopsis", d.id);
			output_with_prefix("///", "");
			output_with_prefix("///", "  @section {}_11 C Synopsis", d.id);
			output_with_prefix("///", "");
			output_with_prefix("///", "  @snippet devsyn.dox.h {} csynopsis", d.id);
			output_with_prefix("///", "");

			output_with_prefix("///", "  @section {}_2 Parameters", d.id);
			output_with_prefix("///", "");
			if (!d.params.empty())
			{
				output_with_prefix("///", "  <table>");
				output_with_prefix("///", "  <tr><th>Name</th><th>Description</th></tr>");
				for (auto &e : d.params)
					output_with_prefix("///", "  <tr><td>{1}</td><td>{2}</td></tr>", e.first, e.second);
				output_with_prefix("///", "  </table>");
			}
			else
				output_with_prefix("///", "  This device has no parameters.");
			output_with_prefix("///", "");
			output_with_prefix("///", "  @section {}_3 Connection Diagram", d.id);
			output_with_prefix("///", "");

			if (!d.pin_alias.empty())
			{
				output_with_prefix("///", "  <pre>");
				if (d.package == "DIP")
				{
					auto & pins = d.pin_alias;
					//const int w = 8;
					output_with_prefix("///", " {1:10} +--------+", " ");
					for (std::size_t i=0; i < pins.size() / 2; i++)
					{
						output_with_prefix("///", " {1:10} |{2:-2}    {3:2}| {4:-10}",
							pins[i], i+1, pins.size()-i, pins[pins.size()-i-1]);
					}
					output_with_prefix("///", " {1:10} +--------+", " ");
				}
				else if (d.package == "SIL")
				{
					auto & pins = d.pin_alias;
					//const int w = 8;
					output_with_prefix("///", " {1:10} +--------+", " ");
					for (std::size_t i=0; i < pins.size(); i++)
					{
						output_with_prefix("///", " {1:10} |{2:-2}      |",
							pins[i], i+1);
					}
					output_with_prefix("///", " {1:10} +--------+", " ");
				}
				output_with_prefix("///", "  </pre>");
			}
			output_with_prefix("///", "");
			output_with_prefix("///", "  {}", d.naming_conventions);
			output_with_prefix("///", "");
			output_with_prefix("///", "  @section {}_4 Function Table", d.id);
			output_with_prefix("///", "");
			if (d.function_table.empty())
				output_with_prefix("///", "  Please refer to the datasheet.");
			else
				output_with_prefix("///", "  {}", d.function_table);
			output_with_prefix("///", "");
			output_with_prefix("///", "  @section {}_5 Limitations", d.id);
			output_with_prefix("///", "");
			output_with_prefix("///", "  {}", d.limitations);
			if (!d.example.empty())
			{
				output_with_prefix("///", "");
				output_with_prefix("///", "  @section {}_6 Example", d.id);
				output_with_prefix("///", "  @snippet {1} {2}", d.example[0], d.example[1]);
				output_with_prefix("///", "");
				output_with_prefix("", "");
			}
		}
	}
	nt.exec().stop();
}


// -------------------------------------------------
//    list_devices - list all known devices
// -------------------------------------------------

void tool_app_t::list_devices()
{
	netlist_tool_t nt(plib::plog_delegate(&tool_app_t::logger, this), "netlist", opt_boost_lib());

	nt.log().verbose.set_enabled(false);
	nt.log().info.set_enabled(false);
	nt.log().warning.set_enabled(false);

	netlist::factory::list_t &list = nt.parser().factory();

	nt.parser().register_source<netlist::source_proc_t>("dummy", &netlist_dummy);
	nt.parser().include("dummy");
	nt.setup().prepare_to_run();

	std::vector<netlist::device_arena::unique_ptr<netlist::core_device_t>> devices;

	for (auto & fl : list)
	{
		pstring out = plib::pfmt("{1:-20} {2}(<id>")(fl->name())(fl->name());

		netlist::factory::element_t *f = nullptr;
		nt.parser().register_dev(fl->name(), fl->name() + "_lc",
			std::vector<pstring>(), &f);

		auto d = f->make_device(nt.pool(), nt, f->name() + "_lc");
		// get the list of terminals ...

		std::vector<pstring> terms(nt.setup().get_terminals_for_device_name(d->name()));

		out += "," + f->param_desc();
		for (const auto &p : plib::psplit(f->param_desc(),',') )
		{
			if (plib::startsWith(p, "+"))
			{
				plib::container::remove(terms, p.substr(1));
			}
		}
		out += ")";
		std_out("{}\n", out);
		if (!terms.empty())
		{
			pstring t = "";
			for (auto & j : terms)
				t += "," + j;
			std_out("\tTerminals: {}\n", t.substr(1));
		}
		devices.emplace_back(std::move(d));
	}
}

void tool_app_t::list_models()
{
	netlist_tool_t nt(plib::plog_delegate(&tool_app_t::logger, this), "netlist", opt_boost_lib());

	nt.log().verbose.set_enabled(false);
	nt.log().info.set_enabled(false);
	nt.log().warning.set_enabled(false);

	nt.parser().register_source<netlist::source_proc_t>("dummy", &netlist_dummy);
	nt.parser().include("dummy");
	nt.setup().prepare_to_run();

	using string_pair = std::pair<pstring, pstring>;

	struct comp_s {
	  bool operator() (const string_pair &i, const string_pair &j)
	  {
		  if (i.first < j.first)
			  return true;
		  if (i.first == j.first)
			  return (i.second < j.second);
		  return false;
	  }
	} comp;

	std::vector<string_pair> elems;

	for (auto & e : nt.setup().models().known_models())
	{
		auto model = nt.setup().models().get_model(e);

		elems.emplace_back(model.type(), e);
	}

	std::sort(elems.begin(), elems.end(), comp);

	for (auto & e : elems)
	{
		pstring out = plib::pfmt("{1:-15} {2}")(e.first, e.second);
		std_out("{}\n", out);
	}
}

// -------------------------------------------------
//    convert - convert spice et al to netlist
// -------------------------------------------------

void tool_app_t::convert()
{
	std::stringstream out_stream;
	out_stream.imbue(std::locale::classic());

	if (opt_files().size() > 1)
		throw netlist::nl_exception("nltool: convert needs exactly one file");

	if (opt_files().empty() || opt_files()[0] == "-")
	{
		plib::copy_stream(out_stream, std::cin);
	}
	else
	{
		plib::ifstream strm(plib::filesystem::u8path(opt_files()[0]));
		if (strm.fail())
			throw netlist::nl_exception(netlist::MF_FILE_OPEN_ERROR(opt_files()[0]));
		strm.imbue(std::locale::classic());
		plib::copy_stream(out_stream, strm);
	}

	pstring contents(putf8string(out_stream.str()));

	pstring result;
	if (opt_type.as_string() == "spice")
	{
		netlist::convert::nl_convert_spice_t c;
		c.convert(contents);
		result = c.result();
	}
	else if (opt_type.as_string() == "eagle")
	{
		netlist::convert::nl_convert_eagle_t c;
		c.convert(contents);
		result = c.result();
	}
	else if (opt_type.as_string() == "rinf")
	{
		netlist::convert::nl_convert_rinf_t c;
		c.convert(contents);
		result = c.result();
	}
	// present result
	std_out.write(result);
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
		std_out(usage());
		return 0;
	}

	if (opt_version())
	{
		std_out(
			"nltool (netlist) {1}\n"
			"Copyright (C) 2021 Couriersud\n"
			"License BSD-3-Clause\n"
			"This is free software: you are free to change and redistribute it.\n"
			"There is NO WARRANTY, to the extent permitted by law.\n\n"
			"Written by Couriersud.\n", netlist::netlist_state_t::version());
		if (opt_verb())
		{
			std::vector<std::pair<pstring, pstring>> defs;
			netlist::netlist_state_t::compile_defines(defs);
			std_out("\nCompile defines:\n");
			for (auto &x : defs)
				std_out("{1:-30} = {2}\n", x.first, x.second);

		}
		return 0;
	}

	m_defines = opt_defines();
	m_defines.emplace_back("NLTOOL_VERSION=" + netlist::netlist_state_t::version());
	if (opt_pre_pro())
		m_defines.emplace_back("__PREPROCESSOR_DEBUG__=1");

	try
	{
		plib::fp_signal_enabler::global_enable(opt_fp_error());
		plib::fp_signal_enabler fp_protect(plib::FP_DIVBYZERO | plib::FP_UNDERFLOW | plib::FP_OVERFLOW | plib::FP_INVALID);

		pstring cmd = opt_cmd.as_string();
		if (cmd == "list-devices")
			list_devices();
		else if (cmd == "list-models")
			list_models();
		else if (cmd == "run")
			run();
		else if (cmd == "validate")
			validate();
		else if (cmd == "static")
			static_compile();
		else if (cmd == "header")
			create_header();
		else if (cmd == "doc-header")
			create_doc_header();
		else if (cmd == "convert")
			convert();
		else if (cmd == "tests")
		{
			return PRUN_ALL_TESTS(opt_verb() ? ::plib::testing::loglevel::INFO
				: opt_quiet() ? ::plib::testing::loglevel::ERROR
				: ::plib::testing::loglevel::WARNING);
		}
		else
		{
			std_err("Unknown command {}\n", cmd.c_str());
			//FIXME: usage_short
			std_err(usage());
			return 1;
		}
	}
	catch (plib::pexception &e)
	{
		std_err("Exception caught: {}\n", e.text());
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
