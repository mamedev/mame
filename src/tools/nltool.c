/***************************************************************************

    nltool.c

    Simple tool to debug netlists outside MAME.

****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sstream>
#include "astring.h"
#include "corefile.h"
#include "corestr.h"
#include "sha1.h"
#include "netlist/nl_base.h"
#include "netlist/nl_setup.h"
#include "netlist/nl_parser.h"
#include "netlist/nl_factory.h"
#include "netlist/nl_util.h"
#include "netlist/devices/net_lib.h"
#include "options.h"

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

struct options_entry oplist[] =
{
	{ "time_to_run;t",   "1.0", OPTION_FLOAT,   "time to run the emulation (seconds)" },
	{ "logs;l",          "",    OPTION_STRING,  "colon separated list of terminals to log" },
	{ "f",               "-",   OPTION_STRING,  "file to process (default is stdin)" },
	{ "listdevices;ld",  "",    OPTION_BOOLEAN, "list all devices available for use" },
	{ "help;h",          "0",   OPTION_BOOLEAN, "display help" },
	{ NULL }
};

NETLIST_START(dummy)
    /* Standard stuff */

    CLOCK(clk, 1000) // 1000 Hz
    SOLVER(Solver, 48000)

NETLIST_END()

/***************************************************************************
    CORE IMPLEMENTATION
***************************************************************************/

const char *filetobuf(pstring fname)
{
	static pstring pbuf = "";

	if (fname == "-")
	{
		char lbuf[1024];
		while (!feof(stdin))
		{
			fgets(lbuf, 1024, stdin);
			pbuf += lbuf;
		}
		printf("%d\n",*(pbuf.right(1).cstr()+1));
		return pbuf.cstr();
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
		return buf;
	}
}

class netlist_tool_t : public netlist_base_t
{
public:

	netlist_tool_t()
	: netlist_base_t(), m_logs(""), m_setup(NULL)
	{
	}

	virtual ~netlist_tool_t() { };

	void init()
	{
		m_setup = new netlist_setup_t(*this);
		this->init_object(*this, "netlist");
		m_setup->init();
	}

	void read_netlist(const char *buffer)
	{
		// read the netlist ...

		netlist_sources_t sources;

		sources.add(netlist_source_t(buffer));
		sources.parse(*m_setup,"");
		//m_setup->parse(buffer);
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
		nl_util::pstring_list ll = nl_util::split(m_logs, ":");
		for (int i=0; i < ll.count(); i++)
		{
            pstring name = "log_" + ll[i];
			/*netlist_device_t *nc = */ m_setup->register_dev("nld_log", name);
			m_setup->register_link(name + ".I", ll[i]);
		}
	}

	pstring m_logs;
protected:

	void verror(const loglevel_e level, const char *format, va_list ap) const
	{
		switch (level)
		{
			case NL_LOG:
			case NL_WARNING:
				vprintf(format, ap);
				printf("\n");
				break;
			case NL_ERROR:
				vprintf(format, ap);
				printf("\n");
				throw;
		}
	}

private:
	netlist_setup_t *m_setup;
};


void usage(core_options &opts)
{
	astring buffer;
	fprintf(stderr,
		"Usage:\n"
		"  nltool -help\n"
		"  nltool [options]\n"
		"\n"
		"Where:\n"
	);
	fprintf(stderr, "%s\n", opts.output_help(buffer));
}

static void run(core_options &opts)
{
	netlist_tool_t nt;
	osd_ticks_t t = osd_ticks();

	nt.init();
	nt.m_logs = opts.value("l");
	nt.read_netlist(filetobuf(opts.value("f")));
	double ttr = opts.float_value("t");

	printf("startup time ==> %5.3f\n", (double) (osd_ticks() - t) / (double) osd_ticks_per_second() );
	printf("runnning ...\n");
	t = osd_ticks();

	nt.process_queue(netlist_time::from_double(ttr));

	double emutime = (double) (osd_ticks() - t) / (double) osd_ticks_per_second();
	printf("%f seconds emulation took %f real time ==> %5.2f%%\n", ttr, emutime, ttr/emutime*100.0);
}

static void listdevices()
{
	netlist_tool_t nt;
	nt.init();
	const netlist_factory_t::list_t &list = nt.setup().factory().list();

    netlist_sources_t sources;

    sources.add(netlist_source_t("dummy", &netlist_dummy));
    sources.parse(nt.setup(),"dummy");

	nt.setup().start_devices();
	nt.setup().resolve_inputs();

	for (int i=0; i < list.count(); i++)
	{
		pstring out = pstring::sprintf("%-20s %s(<id>", list[i]->classname().cstr(),
				list[i]->name().cstr() );
		pstring terms("");

		net_device_t_base_factory *f = list[i];
		netlist_device_t *d = f->Create();
		d->init(nt, pstring::sprintf("dummy%d", i));
		d->start_dev();

		// get the list of terminals ...
		for (int j=0; j < d->m_terminals.count(); j++)
		{
			pstring inp = d->m_terminals[j];
			if (inp.startsWith(d->name() + "."))
				inp = inp.substr(d->name().len() + 1);
			terms += "," + inp;
		}

		if (list[i]->param_desc().startsWith("+"))
		{
			out += "," + list[i]->param_desc().substr(1);
			terms = "";
		}
		else if (list[i]->param_desc() == "-")
		{
			/* no params at all */
		}
		else
		{
			out += "," + list[i]->param_desc();
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

int main(int argc, char *argv[])
{
	//int result;
	core_options opts(oplist);
	astring aerror("");

	fprintf(stderr, "%s", "WARNING: This is Work In Progress! - It may fail anytime\n");
	if (!opts.parse_command_line(argc, argv, OPTION_PRIORITY_DEFAULT, aerror))
	{
		fprintf(stderr, "%s\n", aerror.cstr());
		usage(opts);
		return 1;
	}

	if (opts.bool_value("h"))
	{
		usage(opts);
		return 1;
	}

	if (opts.bool_value("ld"))
	{
		listdevices();
	}
	else
	{
		run(opts);
	}

	return 0;
}
