// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nl_parser.c
 *
 */

#include "nl_parser.h"

//#undef NL_VERBOSE_OUT
//#define NL_VERBOSE_OUT(x) printf x

// ----------------------------------------------------------------------------------------
// A netlist parser
// ----------------------------------------------------------------------------------------

ATTR_COLD void netlist_parser::error(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);

    pstring errmsg1 =pstring(format).vprintf(ap);
    va_end(ap);

    char buf[300];
    int bufp = 0;
    const char *p = m_line_ptr;
    while (*p && *p != 10)
        buf[bufp++] = *p++;
    buf[bufp] = 0;

    m_setup.netlist().error("line %d: error: %s\n\t\t%s\n", m_line, errmsg1.cstr(), buf);

    //throw error;
}


void netlist_parser::parse(const char *buf)
{
	m_px = buf;
	m_line_ptr = buf;
	m_line = 1;

	while (!eof())
	{
		pstring n;
		skipws();
		if (eof()) break;
		n = getname('(');
		NL_VERBOSE_OUT(("Parser: Device: %s\n", n.cstr()));
		if (n == "NET_ALIAS")
			net_alias();
		else if (n == "NET_C")
			net_c();
		else if (n == "NETDEV_PARAM")
			netdev_param();
		else if ((n == "NET_MODEL"))
		    net_model();
		else if (n == "NETLIST_START")
		    netdev_netlist_start();
        else if (n == "NETLIST_END")
            netdev_netlist_end();
		else
			netdev_device(n);
	}
}

void netlist_parser::netdev_netlist_start()
{
    // don't do much
    skipws();
    /*pstring dummyname = */ getname(')');
    //check_char(')');
}

void netlist_parser::netdev_netlist_end()
{
    // don't do much
    skipws();
    check_char(')');
}

void netlist_parser::net_model()
{
    // don't do much
    pstring model = getstring();
    m_setup.register_model(model);
    check_char(')');
}

void netlist_parser::net_alias()
{
	pstring alias;
	pstring out;
	skipws();
	alias = getname(',');
	skipws();
	out = getname(')');
	NL_VERBOSE_OUT(("Parser: Alias: %s %s\n", alias.cstr(), out.cstr()));
	m_setup.register_alias(alias, out);
}

void netlist_parser::net_c()
{
	pstring t1;
	pstring t2;
	skipws();
	t1 = getname(',');
	skipws();
	t2 = getname(')');
	NL_VERBOSE_OUT(("Parser: Connect: %s %s\n", t1.cstr(), t2.cstr()));
	m_setup.register_link(t1 , t2);
}

void netlist_parser::netdev_param()
{
	pstring param;
	double val;
	skipws();
	param = getname(',');
	skipws();
	val = eval_param();
	NL_VERBOSE_OUT(("Parser: Param: %s %f\n", param.cstr(), val));
	m_setup.register_param(param, val);
	check_char(')');
}

void netlist_parser::netdev_device(const pstring &dev_type)
{
	pstring devname;
	net_device_t_base_factory *f = m_setup.factory().factory_by_name(dev_type, m_setup);
	netlist_device_t *dev;
	nl_util::pstring_list termlist = f->term_param_list();
	pstring def_param = f->def_param();

	int cnt;

	skipws();
	devname = getname2(',', ')');
	dev = f->Create();
	m_setup.register_dev(dev, devname);

	NL_VERBOSE_OUT(("Parser: IC: %s\n", devname.cstr()));

	if (def_param != "")
	{
        pstring paramfq = devname + "." + def_param;
	    NL_VERBOSE_OUT(("Defparam: %s\n", def_param.cstr()));
        check_char(',');
	    skipws();
	    if (peekc() == '"')
	    {
            pstring val = getstring();
            m_setup.register_param(paramfq, val);
	    }
	    else
	    {
	        double val = eval_param();
	        m_setup.register_param(paramfq, val);
	    }
	    if (termlist.count() > 0)
	        check_char(',');
	}

	cnt = 0;
	while (getc() != ')' && cnt < termlist.count())
	{
		skipws();
		pstring output_name = getname2(',', ')');

		m_setup.register_link(devname + "." + termlist[cnt], output_name);

		skipws();
		cnt++;
	}
    if (cnt != termlist.count())
        fatalerror("netlist: input count mismatch for %s - expected %d found %d\n", devname.cstr(), termlist.count(), cnt);
}


// ----------------------------------------------------------------------------------------
// private
// ----------------------------------------------------------------------------------------

void netlist_parser::skipeol()
{
	char c = getc();
	while (c)
	{
		if (c == 10)
		{
			c = getc();
			if (c != 13)
				ungetc();
			return;
		}
		c = getc();
	}
}

void netlist_parser::skipws()
{
	while (unsigned char c = getc())
	{
		switch (c)
		{
		case ' ':
		case 9:
		case 10:
		case 13:
			break;
		case '#':
		    skipeol(); // treat preprocessor defines as comments
		    break;
		case '/':
			c = getc();
			if (c == '/')
			{
				skipeol();
			}
			else if (c == '*')
			{
				int f=0;
				while (!eof() )
				{
				    c = getc();
					if (f == 0 && c == '*')
						f=1;
					else if (f == 1 && c== '/' )
						break;
					else
						f=0;
				}
			}
			break;
		default:
			ungetc();
			return;
		}
	}
}

pstring netlist_parser::getstring()
{
    skipws();
    check_char('"');
    pstring ret = getname2_ext('"', 0, NULL);
    check_char('"');
    skipws();
    return ret;
}


pstring netlist_parser::getname(char sep)
{
	pstring ret = getname2(sep, 0);
	getc(); // undo the undo ...
	return ret;
}

pstring netlist_parser::getname2(char sep1, char sep2)
{
    static const char *allowed = "0123456789_.ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    return getname2_ext(sep1, sep2, allowed);
}

pstring netlist_parser::getname2_ext(char sep1, char sep2, const char *allowed)
{
    char buf[1024];
    char *p1 = buf;
    char c=getc();

    while ((c != sep1) && (c != sep2))
    {
        char cU = toupper(c);
        if ((allowed == NULL) || strchr(allowed, cU) != NULL)
            *p1++ = c;
        else
        {
            *p1 = 0;
            error("illegal character <%c> in name ...\n", c);
        }
        c = getc();
    }
    *p1 = 0;
    ungetc();
    return pstring(buf);
}
void netlist_parser::check_char(char ctocheck)
{
	skipws();
	char c = getc();
	if ( c == ctocheck)
	{
		return;
	}
	error("expected '%c' found '%c'\n", ctocheck, c);
}

double netlist_parser::eval_param()
{
	static const char *macs[6] = {"", "RES_K(", "RES_M(", "CAP_U(", "CAP_N(", "CAP_P("};
	static const char *allowed = "RESKMUNPAC_0123456789E(+-.";
	static double facs[6] = {1, 1e3, 1e6, 1e-6, 1e-9, 1e-12};
	int i;
	int f=0;
	bool e;
	double ret;

	pstring s = getname2_ext(')',',', allowed);

	for (i=1; i<6;i++)
		if (strncmp(s.cstr(), macs[i], strlen(macs[i])) == 0)
			f = i;
    if (f>0)
        check_char(')');
	s = s.substr(strlen(macs[f]));
	ret = s.as_double(&e);
//    if ((f>0) && e)
	if (e)
		error("Error with parameter ...\n");
	return ret * facs[f];
}

unsigned char netlist_parser::peekc()
{
    unsigned char c = getc();
    ungetc();
    return c;
}

unsigned char netlist_parser::getc()
{
    if (*m_px == 10)
    {
        m_line++;
        m_line_ptr = m_px + 1;
    }
	if (*m_px)
		return *(m_px++);
	else
		return *m_px;
}

void netlist_parser::ungetc()
{
	m_px--;
}
