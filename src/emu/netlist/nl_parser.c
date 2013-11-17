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
		astring n;
		skipws();
		if (!*m_p) break;
		n = getname('(');
		NL_VERBOSE_OUT(("Parser: Device: %s\n", n.cstr()));
		if (n == "NET_ALIAS")
			net_alias();
		else if (n == "NETDEV_PARAM")
			netdev_param();
		else if ((n == "NETDEV_TTL_CONST") || (n == "NETDEV_ANALOG_CONST"))
			netdev_const(n);
		else
			netdev_device(n);
	}
}

void netlist_parser::net_alias()
{
	astring alias;
	astring out;
	skipws();
	alias = getname(',');
	skipws();
	out = getname(')');
	NL_VERBOSE_OUT(("Parser: Alias: %s %s\n", alias.cstr(), out.cstr()));
	m_setup.register_alias(alias, out);
}

void netlist_parser::netdev_param()
{
	astring param;
	double val;
	skipws();
	param = getname(',');
	skipws();
	val = eval_param();
	NL_VERBOSE_OUT(("Parser: Param: %s %f\n", param.cstr(), val));
	m_setup.find_param(param).initial(val);
	check_char(')');
}

void netlist_parser::netdev_const(const astring &dev_name)
{
	astring name;
	netlist_device_t *dev;
	astring paramfq;
	double val;

	skipws();
	name = getname(',');
	dev = net_create_device_by_name(dev_name, m_setup, name);
	m_setup.register_dev(dev);
	skipws();
	val = eval_param();
	check_char(')');
	paramfq = name;
	paramfq.cat(".CONST");
	NL_VERBOSE_OUT(("Parser: Const: %s %f\n", name.cstr(), val));
	m_setup.find_param(paramfq).initial(val);
}

void netlist_parser::netdev_device(const astring &dev_type)
{
	astring devname;
	netlist_device_t *dev;
	int cnt;

	skipws();
	devname = getname2(',', ')');
	dev = net_create_device_by_name(dev_type, m_setup, devname);
	m_setup.register_dev(dev);
	skipws();
	NL_VERBOSE_OUT(("Parser: IC: %s\n", devname.cstr()));
	cnt = 0;
	while (*m_p != ')')
	{
		m_p++;
		skipws();
		astring output_name = getname2(',', ')');
		NL_VERBOSE_OUT(("Parser: ID: %s %s\n", output_name.cstr(), dev->m_terminals.item(cnt)->cstr()));
		m_setup.register_link(devname + "." + *dev->m_terminals.item(cnt), output_name);
		skipws();
		cnt++;
	}
	if (cnt != dev->m_terminals.count() && !dev->variable_input_count())
		fatalerror("netlist: input count mismatch for %s - expected %d found %d\n", devname.cstr(), dev->m_terminals.count(), cnt);
	if (dev->variable_input_count())
	{
		NL_VERBOSE_OUT(("variable inputs %s: %d\n", dev->name().cstr(), cnt));
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
				skipeol();
			break;
		default:
			return;
		}
	}
}

astring netlist_parser::getname(char sep)
{
	char buf[300];
	char *p1 = buf;

	while (*m_p != sep)
		*p1++ = *m_p++;
	*p1 = 0;
	m_p++;
	return astring(buf);
}

astring netlist_parser::getname2(char sep1, char sep2)
{
	char buf[300];
	char *p1 = buf;

	while ((*m_p != sep1) && (*m_p != sep2))
		*p1++ = *m_p++;
	*p1 = 0;
	return astring(buf);
}

void netlist_parser::check_char(char ctocheck)
{
	skipws();
	if (*m_p == ctocheck)
	{
		m_p++;
		return;
	}
	fatalerror("Parser: expected '%c' found '%c'\n", ctocheck, *m_p);
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
		fatalerror("Parser: Error with parameter ...\n");
	if (f>0)
		e++;
	m_p = e;
	return ret * facs[f];
}
