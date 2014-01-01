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
		else if (n == "NETDEV_R")
			netdev_device(n, "R");
		else if (n == "NETDEV_C")
			netdev_device(n, "C");
		else if (n == "NETDEV_POT")
			netdev_device(n, "R");
		else if (n == "NETDEV_D")
			netdev_device(n, "model", true);
        else if (n == "NETDEV_QNPN")
            netdev_device(n, "model", true);
        else if (n == "NETDEV_QPNP")
            netdev_device(n, "model", true);
		else if ((n == "NETDEV_TTL_CONST") || (n == "NETDEV_ANALOG_CONST"))
			netdev_const(n);
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

void netlist_parser::netdev_const(const pstring &dev_name)
{
	pstring name;
	netlist_device_t *dev;
	pstring paramfq;
	double val;

	skipws();
	name = getname(',');
	dev = m_setup.factory().new_device_by_name(dev_name, m_setup);
	m_setup.register_dev(dev, name);
	skipws();
	val = eval_param();
	paramfq = name + ".CONST";
	NL_VERBOSE_OUT(("Parser: Const: %s %f\n", name.cstr(), val));
	check_char(')');
	m_setup.register_param(paramfq, val);
}

void netlist_parser::netdev_device(const pstring &dev_type)
{
	pstring devname;
	netlist_device_t *dev;
	int cnt;

	skipws();
	devname = getname2(',', ')');
	dev = m_setup.factory().new_device_by_name(dev_type, m_setup);
	m_setup.register_dev(dev, devname);
	NL_VERBOSE_OUT(("Parser: IC: %s\n", devname.cstr()));
	cnt = 0;
	while (getc() != ')')
	{
		skipws();
		pstring output_name = getname2(',', ')');
		pstring alias = pstring::sprintf("%s.[%d]", devname.cstr(), cnt);
		NL_VERBOSE_OUT(("Parser: ID: %s %s\n", output_name.cstr(), alias.cstr()));
		m_setup.register_link(alias, output_name);
		skipws();
		cnt++;
	}
/*
    if (cnt != dev->m_terminals.count() && !dev->variable_input_count())
        fatalerror("netlist: input count mismatch for %s - expected %d found %d\n", devname.cstr(), dev->m_terminals.count(), cnt);
    if (dev->variable_input_count())
    {
        NL_VERBOSE_OUT(("variable inputs %s: %d\n", dev->name().cstr(), cnt));
    }
    */
}

void netlist_parser::netdev_device(const pstring &dev_type, const pstring &default_param, bool isString)
{
	netlist_device_t *dev;

	skipws();
	pstring devname = getname2(',', ')');
	pstring defparam = devname + "." + default_param;
	dev = m_setup.factory().new_device_by_name(dev_type, m_setup);
	m_setup.register_dev(dev, devname);
	NL_VERBOSE_OUT(("Parser: IC: %s\n", devname.cstr()));
	if (getc() != ')')
	{
		// have a default param
		skipws();
		if (isString)
		{
			pstring val = getname(')');
			ungetc();
			NL_VERBOSE_OUT(("Parser: Default param: %s %s\n", defparam.cstr(), val.cstr()));
			m_setup.register_param(defparam, val);
		}
		else
		{
			double val = eval_param();
			NL_VERBOSE_OUT(("Parser: Default param: %s %f\n", defparam.cstr(), val));
			m_setup.register_param(defparam, val);
		}
	}
	check_char(')');
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
    char buf[300];
    char *p1 = buf;
    char c=getc();

    while ((c != sep1) && (c != sep2))
    {
        char cU = toupper(c);
        if (strchr(allowed, cU) != NULL)
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
	ret = s.substr(strlen(macs[f])).as_double(&e);
	if ((f>0) && e)
		error("Error with parameter ...\n");
	if (f>0)
		check_char(')');
	return ret * facs[f];
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
