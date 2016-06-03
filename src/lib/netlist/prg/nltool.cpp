// license:GPL-2.0+
// copyright-holders:Couriersud
/***************************************************************************

    nltool.c

    Simple tool to debug netlists outside MAME.

****************************************************************************/

#include <cstdio>

#ifdef PSTANDALONE
#if (PSTANDALONE)
#define PSTANDALONE_PROVIDED
#endif
#endif

#include <plib/poptions.h>
#include "plib/pstring.h"
#include "plib/plists.h"
#include "plib/ptypes.h"
#include "nl_setup.h"
#include "nl_factory.h"
#include "nl_parser.h"
#include "devices/net_lib.h"
#include "tools/nl_convert.h"

class tool_options_t : public plib::options
{
public:
	tool_options_t() :
		plib::options(),
		opt_ttr ("t", "time_to_run", 1.0,   "time to run the emulation (seconds)", this),
		opt_name("n", "name",        "",      "netlist in file to run; default is first one", this),
		opt_logs("l", "logs",        "",      "colon separated list of terminals to log", this),
		opt_file("f", "file",        "-",     "file to process (default is stdin)", this),
		opt_type("y", "type",        "spice", "spice:eagle", "type of file to be converted: spice,eagle", this),
		opt_cmd ("c", "cmd",         "run",   "run|convert|listdevices|static", this),
		opt_inp( "i", "input",       "",      "input file to process (default is none)", this),
		opt_verb("v", "verbose",              "be verbose - this produces lots of output", this),
		opt_quiet("q", "quiet",               "be quiet - no warnings", this),
		opt_help("h", "help",                 "display help", this)
	{}

	plib::option_double opt_ttr;
	plib::option_str    opt_name;
	plib::option_str    opt_logs;
	plib::option_str    opt_file;
	plib::option_str_limit opt_type;
	plib::option_str    opt_cmd;
	plib::option_str    opt_inp;
	plib::option_bool   opt_verb;
	plib::option_bool   opt_quiet;
	plib::option_bool   opt_help;
};

plib::pstdout pout_strm;
plib::pstderr perr_strm;

plib::pstream_fmt_writer_t pout(pout_strm);
plib::pstream_fmt_writer_t perr(perr_strm);

NETLIST_START(dummy)
	/* Standard stuff */

	CLOCK(clk, 1000) // 1000 Hz
	SOLVER(Solver, 48000)

NETLIST_END()

/***************************************************************************
    CORE IMPLEMENTATION
***************************************************************************/

class netlist_tool_t : public netlist::netlist_t
{
public:

	netlist_tool_t(const pstring &aname)
	: netlist::netlist_t(aname), m_opts(nullptr), m_setup(nullptr)
	{
	}

	~netlist_tool_t()
	{
		if (m_setup != nullptr)
			plib::pfree(m_setup);
	};

	void init()
	{
		m_setup = plib::palloc<netlist::setup_t>(*this);
	}

	void read_netlist(const pstring &filename, const pstring &name)
	{
		// read the netlist ...

		m_setup->register_source(std::make_shared<netlist::source_file_t>(filename));
		m_setup->include(name);
		log_setup();

		// start devices
		m_setup->start_devices();
		m_setup->resolve_inputs();
		// reset
		this->reset();
	}

	void log_setup()
	{
		log().debug("Creating dynamic logs ...\n");
		plib::pstring_vector_t ll(m_opts ? m_opts->opt_logs() : "" , ":");
		for (unsigned i=0; i < ll.size(); i++)
		{
			pstring name = "log_" + ll[i];
			/*netlist_device_t *nc = */ m_setup->register_dev("LOG", name);
			m_setup->register_link(name + ".I", ll[i]);
		}
	}

	tool_options_t *m_opts;

protected:

	void vlog(const plib::plog_level &l, const pstring &ls) const override
	{
		pout("{}: {}\n", l.name().cstr(), ls.cstr());
		if (l == plib::plog_level::FATAL)
			throw;
	}

private:
	netlist::setup_t *m_setup;
};


void usage(tool_options_t &opts)
{
	perr("{}",
		"Usage:\n"
		"  nltool --help\n"
		"  nltool [options]\n"
		"\n"
		"Where:\n"
	);
	perr("{}\n", opts.help().cstr());
}

struct input_t
{
	input_t()
	: m_param(nullptr), m_value(0.0)
	{
	}
	input_t(netlist::netlist_t *netlist, const pstring &line)
	{
		char buf[400];
		double t;
		int e = sscanf(line.cstr(), "%lf,%[^,],%lf", &t, buf, &m_value);
		if ( e!= 3)
			throw netlist::fatalerror_e(plib::pfmt("error {1} scanning line {2}\n")(e)(line));
		m_time = netlist::netlist_time(t);
		m_param = netlist->setup().find_param(buf, true);
	}

	void setparam()
	{
		switch (m_param->param_type())
		{
			case netlist::param_t::MODEL:
			case netlist::param_t::STRING:
				throw netlist::fatalerror_e(plib::pfmt("param {1} is not numeric\n")(m_param->name()));
			case netlist::param_t::DOUBLE:
				static_cast<netlist::param_double_t*>(m_param)->setTo(m_value);
				break;
			case netlist::param_t::INTEGER:
				static_cast<netlist::param_int_t*>(m_param)->setTo((int)m_value);
				break;
			case netlist::param_t::LOGIC:
				static_cast<netlist::param_logic_t*>(m_param)->setTo((int) m_value);
				break;
		}
	}

	netlist::netlist_time m_time;
	netlist::param_t *m_param;
	double m_value;

};

plib::pvector_t<input_t> *read_input(netlist::netlist_t *netlist, pstring fname)
{
	plib::pvector_t<input_t> *ret = plib::palloc<plib::pvector_t<input_t>>();
	if (fname != "")
	{
		plib::pifilestream f(fname);
		pstring l;
		while (f.readline(l))
		{
			if (l != "")
			{
				input_t inp(netlist, l);
				ret->push_back(inp);
			}
		}
	}
	return ret;
}

static void run(tool_options_t &opts)
{
	netlist_tool_t nt("netlist");
	plib::ticks_t t = plib::ticks();

	nt.m_opts = &opts;
	nt.init();

	if (!opts.opt_verb())
		nt.log().verbose.set_enabled(false);
	if (opts.opt_quiet())
		nt.log().warning.set_enabled(false);

	nt.read_netlist(opts.opt_file(), opts.opt_name());

	plib::pvector_t<input_t> *inps = read_input(&nt, opts.opt_inp());

	double ttr = opts.opt_ttr();

	pout("startup time ==> {1:5.3f}\n", (double) (plib::ticks() - t) / (double) plib::ticks_per_second() );
	pout("runnning ...\n");
	t = plib::ticks();

	unsigned pos = 0;
	netlist::netlist_time nlt = netlist::netlist_time::zero;

	while (pos < inps->size() && (*inps)[pos].m_time < netlist::netlist_time(ttr))
	{
		nt.process_queue((*inps)[pos].m_time - nlt);
		(*inps)[pos].setparam();
		nlt = (*inps)[pos].m_time;
		pos++;
	}
	nt.process_queue(netlist::netlist_time(ttr) - nlt);
	nt.stop();
	pfree(inps);

	double emutime = (double) (plib::ticks() - t) / (double) plib::ticks_per_second();
	pout("{1:f} seconds emulation took {2:f} real time ==> {3:5.2f}%\n", ttr, emutime, ttr/emutime*100.0);
}

static void static_compile(tool_options_t &opts)
{
	netlist_tool_t nt("netlist");

	nt.m_opts = &opts;
	nt.init();

	nt.log().verbose.set_enabled(false);
	nt.log().warning.set_enabled(false);

	nt.read_netlist(opts.opt_file(), opts.opt_name());

	nt.solver()->create_solver_code(pout_strm);

	nt.stop();

}

/*-------------------------------------------------
    listdevices - list all known devices
-------------------------------------------------*/

static void listdevices()
{
	netlist_tool_t nt("netlist");
	nt.init();
	netlist::factory_list_t &list = nt.setup().factory();

	nt.setup().register_source(std::make_shared<netlist::source_proc_t>("dummy", &netlist_dummy));
	nt.setup().include("dummy");

	nt.setup().start_devices();
	nt.setup().resolve_inputs();

	for (unsigned i=0; i < list.size(); i++)
	{
		auto &f = list[i];
		pstring out = plib::pfmt("{1} {2}(<id>")(f->classname(),"-20")(f->name());
		pstring terms("");

		auto d = f->Create(nt.setup().netlist(), plib::pfmt("dummy{1}")(i));

		// get the list of terminals ...
		for (unsigned j=0; j < d->m_terminals.size(); j++)
		{
			pstring inp = d->m_terminals[j];
			if (inp.startsWith(d->name() + "."))
				inp = inp.substr(d->name().len() + 1);
			terms += "," + inp;
		}

		if (f->param_desc().startsWith("+"))
		{
			out += "," + f->param_desc().substr(1);
			terms = "";
		}
		else if (f->param_desc() == "-")
		{
			/* no params at all */
		}
		else
		{
			out += "," + f->param_desc();
		}
		out += ")";
		printf("%s\n", out.cstr());
		if (terms != "")
			printf("Terminals: %s\n", terms.substr(1).cstr());
	}
}



/*-------------------------------------------------
    main - primary entry point
-------------------------------------------------*/

#if (!PSTANDALONE)
#include "corealloc.h"
#endif

#if 0
static const char *pmf_verbose[] =
{
	"NL_PMF_TYPE_VIRTUAL",
	"NL_PMF_TYPE_GNUC_PMF",
	"NL_PMF_TYPE_GNUC_PMF_CONV",
	"NL_PMF_TYPE_INTERNAL"
};
#endif

int main(int argc, char *argv[])
{
	tool_options_t opts;
	int ret;

	perr("{}", "WARNING: This is Work In Progress! - It may fail anytime\n");
	//perr("Update dispatching using method {}\n", pmf_verbose[NL_PMF_TYPE]);
	if ((ret = opts.parse(argc, argv)) != argc)
	{
		perr("Error parsing {}\n", argv[ret]);
		usage(opts);
		return 1;
	}

	if (opts.opt_help())
	{
		usage(opts);
		return 1;
	}

	pstring cmd = opts.opt_cmd();
	if (cmd == "listdevices")
		listdevices();
	else if (cmd == "run")
		run(opts);
	else if (cmd == "static")
		static_compile(opts);
	else if (cmd == "convert")
	{
		pstring contents;
		plib::postringstream ostrm;
		if (opts.opt_file() == "-")
		{
			plib::pstdin f;
			ostrm.write(f);
		}
		else
		{
			plib::pifilestream f(opts.opt_file());
			ostrm.write(f);
		}
		contents = ostrm.str();

		pstring result;
		if (opts.opt_type().equals("spice"))
		{
			nl_convert_spice_t c;
			c.convert(contents);
			result = c.result();
		}
		else
		{
			nl_convert_eagle_t c;
			c.convert(contents);
			result = c.result();
		}
		/* present result */
		pout_strm.write(result.cstr());
	}
	else
	{
		perr("Unknown command {}\n", cmd.cstr());
		usage(opts);
		return 1;
	}

	pstring::resetmem();
	return 0;
}
