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

#include "plib/poptions.h"
#include "plib/pstring.h"
#include "plib/plists.h"
#include "nl_setup.h"
#include "nl_factory.h"
#include "nl_parser.h"
#include "devices/net_lib.h"
#include "tools/nl_convert.h"


#ifdef PSTANDALONE_PROVIDED

#include <ctime>

#define osd_ticks_t clock_t

inline osd_ticks_t osd_ticks_per_second() { return CLOCKS_PER_SEC; }

osd_ticks_t osd_ticks(void) { return clock(); }
#else

#endif

/***************************************************************************
 * MAME COMPATIBILITY ...
 *
 * These are needed if we link without libutil
 ***************************************************************************/

#if 0
void ATTR_PRINTF(1,2) osd_printf_warning(const char *format, ...)
{
	va_list argptr;

	/* do the output */
	va_start(argptr, format);
	vprintf(format, argptr);
	va_end(argptr);
}

void *malloc_file_line(size_t size, const char *file, int line)
{
	// allocate the memory and fail if we can't
	void *ret = osd_malloc(size);
	memset(ret, 0, size);
	return ret;
}

void *malloc_array_file_line(size_t size, const char *file, int line)
{
	// allocate the memory and fail if we can't
	void *ret = osd_malloc_array(size);
	memset(ret, 0, size);
	return ret;
}

void free_file_line( void *memory, const char *file, int line )
{
	osd_free( memory );
}

void CLIB_DECL logerror(const char *format, ...)
{
	va_list arg;
	va_start(arg, format);
	vprintf(format, arg);
	va_end(arg);
}

void report_bad_cast(const std::type_info &src_type, const std::type_info &dst_type)
{
	printf("Error: bad downcast<> or device<>.  Tried to convert a %s to a %s, which are incompatible.\n",
			src_type.name(), dst_type.name());
	throw;
}
#endif

class tool_options_t : public poptions
{
public:
	tool_options_t() :
		poptions(),
		opt_ttr ("t", "time_to_run", 1.0,     "time to run the emulation (seconds)", this),
		opt_name("n", "name",        "",      "netlist in file to run; default is first one", this),
		opt_logs("l", "logs",        "",      "colon separated list of terminals to log", this),
		opt_file("f", "file",        "-",     "file to process (default is stdin)", this),
		opt_type("y", "type",        "spice", "spice:eagle", "type of file to be converted: spice,eagle", this),
		opt_cmd ("c", "cmd",         "run",   "run|convert|listdevices", this),
		opt_inp( "i", "input",       "",      "input file to process (default is none)", this),
		opt_verb("v", "verbose",              "be verbose - this produces lots of output", this),
		opt_quiet("q", "quiet",               "be quiet - no warnings", this),
		opt_help("h", "help",                 "display help", this)
	{}

	poption_double opt_ttr;
	poption_str    opt_name;
	poption_str    opt_logs;
	poption_str    opt_file;
	poption_str_limit opt_type;
	poption_str    opt_cmd;
	poption_str    opt_inp;
	poption_bool   opt_verb;
	poption_bool   opt_quiet;
	poption_bool   opt_help;
};

//Alternative
//static poption *optlist[] = { &opt_ttr, &opt_logs, &opt_file, &opt_cmd, &opt_verb, &opt_help, NULL };

NETLIST_START(dummy)
	/* Standard stuff */

	CLOCK(clk, 1000) // 1000 Hz
	SOLVER(Solver, 48000)

NETLIST_END()

/***************************************************************************
    CORE IMPLEMENTATION
***************************************************************************/

pstring filetobuf(pstring fname)
{
	pstring pbuf = "";

	if (fname == "-")
	{
		char lbuf[1024];
		while (!feof(stdin))
		{
			fgets(lbuf, 1024, stdin);
			pbuf += lbuf;
		}
		printf("%d\n",*(pbuf.right(1).cstr()+1));
		return pbuf;
	}
	else
	{
		FILE *f;
		f = fopen(fname, "rb");
		fseek(f, 0, SEEK_END);
		long fsize = ftell(f);
		fseek(f, 0, SEEK_SET);

		char *buf = (char *) malloc(fsize + 1);
		fread(buf, fsize, 1, f);
		buf[fsize] = 0;
		fclose(f);
		pbuf = buf;
		free(buf);
		return pbuf;
	}
}

class netlist_tool_t : public netlist::netlist_t
{
public:

	netlist_tool_t()
	: netlist::netlist_t(), m_opts(NULL), m_setup(NULL)
	{
	}

	~netlist_tool_t()
	{
		if (m_setup != NULL)
			pfree(m_setup);
	};

	void init()
	{
		m_setup = palloc(netlist::setup_t(this));
		this->init_object(*this, "netlist");
		m_setup->init();
	}

	void read_netlist(const char *buffer, pstring name)
	{
		// read the netlist ...

		m_setup->register_source(palloc(netlist::netlist_source_mem_t(buffer)));
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
		NL_VERBOSE_OUT(("Creating dynamic logs ...\n"));
		pstring_list_t ll(m_opts ? m_opts->opt_logs() : "" , ":");
		for (int i=0; i < ll.size(); i++)
		{
			pstring name = "log_" + ll[i];
			/*netlist_device_t *nc = */ m_setup->register_dev("LOG", name);
			m_setup->register_link(name + ".I", ll[i]);
		}
	}

	tool_options_t *m_opts;

protected:

	void verror(const loglevel_e level, const char *format, va_list ap) const
	{
		switch (level)
		{
			case NL_LOG:
				if (m_opts ? m_opts->opt_verb() : false)
				{
					vprintf(format, ap);
					printf("\n");
				}
				break;
			case NL_WARNING:
				if (!(m_opts ? m_opts->opt_quiet() : false))
				{
					vprintf(format, ap);
					printf("\n");
				}
				break;
			case NL_ERROR:
				vprintf(format, ap);
				printf("\n");
				throw;
		}
	}

private:
	netlist::setup_t *m_setup;
};


void usage(tool_options_t &opts)
{
	fprintf(stderr,
		"Usage:\n"
		"  nltool -help\n"
		"  nltool [options]\n"
		"\n"
		"Where:\n"
	);
	fprintf(stderr, "%s\n", opts.help().cstr());
}

struct input_t
{
	netlist::netlist_time m_time;
	netlist::param_t *m_param;
	double m_value;

	input_t()
	{

	}
	input_t(netlist::netlist_t *netlist, const pstring &line)
	{
		char buf[400];
		double t;
		int e = sscanf(line.cstr(), "%lf,%[^,],%lf", &t, buf, &m_value);
		if ( e!= 3)
			throw netlist::fatalerror_e("error %d scanning line %s\n", e, line.cstr());
		m_time = netlist::netlist_time::from_double(t);
		m_param = netlist->setup().find_param(buf, true);
	}

	void setparam()
	{
		switch (m_param->param_type())
		{
			case netlist::param_t::MODEL:
			case netlist::param_t::STRING:
				throw netlist::fatalerror_e("param %s is not numeric\n", m_param->name().cstr());
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
};

plist_t<input_t> *read_input(netlist::netlist_t *netlist, pstring fname)
{
	plist_t<input_t> *ret = palloc(plist_t<input_t>());
	if (fname != "")
	{
		pstring_list_t lines(filetobuf(fname) , "\n");
		for (unsigned i=0; i<lines.size(); i++)
		{
			pstring l = lines[i].trim();
			if (l != "")
			{
				input_t inp(netlist, l);
				ret->add(inp);
			}
		}
	}
	return ret;
}

static void run(tool_options_t &opts)
{
	netlist_tool_t nt;
	osd_ticks_t t = osd_ticks();

	nt.m_opts = &opts;
	nt.init();
	nt.read_netlist(filetobuf(opts.opt_file()), opts.opt_name());

	plist_t<input_t> *inps = read_input(&nt, opts.opt_inp());

	double ttr = opts.opt_ttr();

	printf("startup time ==> %5.3f\n", (double) (osd_ticks() - t) / (double) osd_ticks_per_second() );
	printf("runnning ...\n");
	t = osd_ticks();

	unsigned pos = 0;
	netlist::netlist_time nlt = netlist::netlist_time::zero;

	while (pos < inps->size() && (*inps)[pos].m_time < netlist::netlist_time::from_double(ttr))
	{
		nt.process_queue((*inps)[pos].m_time - nlt);
		(*inps)[pos].setparam();
		nlt = (*inps)[pos].m_time;
		pos++;
	}
	nt.process_queue(netlist::netlist_time::from_double(ttr) - nlt);
	nt.stop();
	pfree(inps);

	double emutime = (double) (osd_ticks() - t) / (double) osd_ticks_per_second();
	printf("%f seconds emulation took %f real time ==> %5.2f%%\n", ttr, emutime, ttr/emutime*100.0);
}

/*-------------------------------------------------
    listdevices - list all known devices
-------------------------------------------------*/

static void listdevices()
{
	netlist_tool_t nt;
	nt.init();
	const netlist::factory_list_t &list = nt.setup().factory();

	nt.setup().register_source(palloc(netlist::source_proc_t("dummy", &netlist_dummy)));
	nt.setup().include("dummy");

	nt.setup().start_devices();
	nt.setup().resolve_inputs();

	for (int i=0; i < list.size(); i++)
	{
		netlist::base_factory_t *f = list.value_at(i);
		pstring out = pstring::sprintf("%-20s %s(<id>", f->classname().cstr(),
				f->name().cstr() );
		pstring terms("");

		netlist::device_t *d = f->Create();
		d->init(nt, pstring::sprintf("dummy%d", i));
		d->start_dev();

		// get the list of terminals ...
		for (int j=0; j < d->m_terminals.size(); j++)
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

static const char *pmf_verbose[] =
{
	"NL_PMF_TYPE_VIRTUAL",
	"NL_PMF_TYPE_GNUC_PMF",
	"NL_PMF_TYPE_GNUC_PMF_CONV",
	"NL_PMF_TYPE_INTERNAL"
};

int main(int argc, char *argv[])
{
#if (!PSTANDALONE)
	track_memory(true);
	{
#endif
	tool_options_t opts;
	int ret;

	fprintf(stderr, "%s", "WARNING: This is Work In Progress! - It may fail anytime\n");
	fprintf(stderr, "Update dispatching using method %s\n", pmf_verbose[NL_PMF_TYPE]);
	if ((ret = opts.parse(argc, argv)) != argc)
	{
		fprintf(stderr, "Error parsing %s\n", argv[ret]);
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
	else if (cmd == "convert")
	{
		pstring contents = filetobuf(opts.opt_file());
		nl_convert_base_t *converter = NULL;
		if (opts.opt_type().equals("spice"))
			converter = palloc(nl_convert_spice_t);
		else
			converter = palloc(nl_convert_eagle_t);
		converter->convert(contents);
		/* present result */
		printf("%s\n", converter->result().cstr());
		pfree(converter);
	}
	else
	{
		fprintf(stderr, "Unknown command %s\n", cmd.cstr());
		usage(opts);
		return 1;
	}
#if (!PSTANDALONE)
	}
	dump_unfreed_mem();
#endif
	return 0;
}
