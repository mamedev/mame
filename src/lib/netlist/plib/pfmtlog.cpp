// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nl_string.c
 *
 */

#include <cstring>
//FIXME:: pstring should be locale free
#include <cctype>
#include <cstdlib>
#include <cstdio>

#include <algorithm>

#include "pfmtlog.h"
#include "palloc.h"

pfmt::pfmt(const pstring &fmt)
: m_str(m_str_buf), m_allocated(0), m_arg(0)
{
	unsigned l = fmt.blen() + 1;
	if (l>sizeof(m_str_buf))
	{
		m_allocated = 2 * l;
		m_str = palloc_array(char, 2 * l);
	}
	memcpy(m_str, fmt.cstr(), l);
}

pfmt::pfmt(const char *fmt)
: m_str(m_str_buf), m_allocated(0), m_arg(0)
{
	unsigned l = strlen(fmt) + 1;
	if (l>sizeof(m_str_buf))
	{
		m_allocated = 2 * l;
		m_str = palloc_array(char, 2 * l);
	}
	memcpy(m_str, fmt, l);
}

pfmt::~pfmt()
{
	if (m_allocated > 0)
		pfree_array(m_str);
}

#if 0
void pformat::format_element(const char *f, const char *l, const char *fmt_spec,  ...)
{
	va_list ap;
	va_start(ap, fmt_spec);
	char fmt[30] = "%";
	char search[10] = "";
	char buf[1024];
	strcat(fmt, f);
	strcat(fmt, l);
	strcat(fmt, fmt_spec);
	int nl = vsprintf(buf, fmt, ap);
	m_arg++;
	int sl = sprintf(search, "%%%d", m_arg);
	char *p = strstr(m_str, search);
	if (p != NULL)
	{
		// Make room
		memmove(p+nl, p+sl, strlen(p) + 1 - sl);
		memcpy(p, buf, nl);
	}
	va_end(ap);
}
#else
void pfmt::format_element(const char *f, const char *l, const char *fmt_spec,  ...)
{
	va_list ap;
	va_start(ap, fmt_spec);
	char fmt[30] = "%";
	char search[10] = "";
	char buf[2048];
	m_arg++;
	int sl = sprintf(search, "{%d:", m_arg);
	char *p = strstr(m_str, search);
	if (p == NULL)
	{
		sl = sprintf(search, "{%d}", m_arg);
		p = strstr(m_str, search);
		if (p == NULL)
		{
			sl = 2;
			p = strstr(m_str, "{}");
		}
		if (p==NULL)
		{
			sl=1;
			p = strstr(m_str, "{");
			if (p != NULL)
			{
				char *p1 = strstr(p, "}");
				if (p1 != NULL)
				{
					sl = p1 - p + 1;
					strncat(fmt, p+1, p1 - p - 2);
				}
				else
					strcat(fmt, f);
			}
			else
				strcat(fmt, f);
		}
	}
	else
	{
		char *p1 = strstr(p, "}");
		if (p1 != NULL)
		{
			sl = p1 - p + 1;
			if (m_arg>=10)
				strncat(fmt, p+4, p1 - p - 4);
			else
				strncat(fmt, p+3, p1 - p - 3);
		}
		else
			strcat(fmt, f);
	}
	strcat(fmt, l);
	char *pend = fmt + strlen(fmt) - 1;
	if (strchr("fge", *fmt_spec) != NULL)
	{
		if (strchr("fge", *pend) == NULL)
			strcat(fmt, fmt_spec);
	}
	else if (strchr("duxo", *fmt_spec) != NULL)
	{
		if (strchr("duxo", *pend) == NULL)
			strcat(fmt, fmt_spec);
	}
	else
		strcat(fmt, fmt_spec);
	int nl = vsprintf(buf, fmt, ap);
	if (p != NULL)
	{
		// check room
		unsigned new_size = (p - m_str) + nl + strlen(p) + 1 - sl;
		if (new_size > m_allocated)
		{
			unsigned old_alloc = std::max(m_allocated, (unsigned) sizeof(m_str_buf));
			if (m_allocated < old_alloc)
				m_allocated = old_alloc;
			while (new_size > m_allocated)
				m_allocated *= 2;
			char *np = palloc_array(char, m_allocated);
			memcpy(np, m_str, old_alloc);
			p = np + (p - m_str);
			if (m_str != m_str_buf)
				pfree_array(m_str);
			m_str = np;
		}
		// Make room
		memmove(p+nl, p+sl, strlen(p) + 1 - sl);
		memcpy(p, buf, nl);
	}
	va_end(ap);
}
#endif
