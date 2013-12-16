// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nl_parser.c
 *
 */

#include "nl_parser.h"

// ----------------------------------------------------------------------------------------
// A netlist parser
// ----------------------------------------------------------------------------------------

void netlist_parser::parse(char *buf)
{
	m_p = buf;
	while (*m_p)
	{
		pstring n;
		skipws();
		if (!*m_p) break;
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
		else if ((n == "NETDEV_TTL_CONST") || (n == "NETDEV_ANALOG_CONST"))
			netdev_const(n);
		else
			netdev_device(n);
	}
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
	//m_setup.find_param(param).initial(val);
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
	check_char(')');
	paramfq = name + ".CONST";
	NL_VERBOSE_OUT(("Parser: Const: %s %f\n", name.cstr(), val));
	//m_setup.find_param(paramfq).initial(val);
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
	skipws();
	NL_VERBOSE_OUT(("Parser: IC: %s\n", devname.cstr()));
	cnt = 0;
	while (*m_p != ')')
	{
		m_p++;
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
	check_char(')');
}

void netlist_parser::netdev_device(const pstring &dev_type, const pstring &default_param, bool isString)
{
    netlist_device_t *dev;

    skipws();
    pstring devname = getname2(',', ')');
    pstring defparam = devname + "." + default_param;
    dev = m_setup.factory().new_device_by_name(dev_type, m_setup);
    m_setup.register_dev(dev, devname);
    skipws();
    NL_VERBOSE_OUT(("Parser: IC: %s\n", devname.cstr()));
    if (*m_p != ')')
    {
        // have a default param
        m_p++;
        skipws();
        if (isString)
        {
            pstring val = getname(')');
            m_p--;
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
	while (*m_p)
	{
		if (*m_p == 10)
		{
			m_p++;
			if (*m_p && *m_p == 13)
				m_p++;
			return;
		}
		m_p++;
	}
}

void netlist_parser::skipws()
{
	while (*m_p)
	{
		switch (*m_p)
		{
		case ' ':
		case 9:
		case 10:
		case 13:
			m_p++;
			break;
		case '/':
			if (*(m_p+1) == '/')
			{
                skipeol();
			}
			else if (*(m_p+1) == '*')
			{
			    m_p+=2;
			    while (*m_p && !(*m_p == '*' && *(m_p + 1) == '/' ))
			        m_p++;
			    if (*m_p)
			        m_p += 2;
			}
			break;
		default:
			return;
		}
	}
}

pstring netlist_parser::getname(char sep)
{
	char buf[300];
	char *p1 = buf;

	while (*m_p != sep)
		*p1++ = *m_p++;
	*p1 = 0;
	m_p++;
	return pstring(buf);
}

pstring netlist_parser::getname2(char sep1, char sep2)
{
	char buf[300];
	char *p1 = buf;

	while ((*m_p != sep1) && (*m_p != sep2))
		*p1++ = *m_p++;
	*p1 = 0;
	return pstring(buf);
}

void netlist_parser::check_char(char ctocheck)
{
	skipws();
	if (*m_p == ctocheck)
	{
		m_p++;
		return;
	}
	m_setup.netlist().xfatalerror("Parser: expected '%c' found '%c'\n", ctocheck, *m_p);
}

double netlist_parser::eval_param()
{
	static const char *macs[6] = {"", "RES_K(", "RES_M(", "CAP_U(", "CAP_N(", "CAP_P("};
	static double facs[6] = {1, 1e3, 1e6, 1e-6, 1e-9, 1e-12};
	int i;
	int f=0;
	char *e;
	double ret;
	char *s = m_p;

	for (i=1; i<6;i++)
		if (strncmp(s, macs[i], strlen(macs[i])) == 0)
			f = i;
	ret = strtod(s+strlen(macs[f]), &e);
	if ((f>0) && (*e != ')'))
	    m_setup.netlist().xfatalerror("Parser: Error with parameter ...\n");
	if (f>0)
		e++;
	m_p = e;
	return ret * facs[f];
}
