// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nl_parser.c
 *
 */

#ifndef NL_PARSER_H_
#define NL_PARSER_H_

#include "nl_setup.h"
#include "nl_util.h"
#include "pparser.h"

class netlist_parser : public ptokenizer
{
	NETLIST_PREVENT_COPYING(netlist_parser)
public:
	netlist_parser(netlist_setup_t &setup)
	: ptokenizer(), m_setup(setup), m_buf(NULL) {}

	bool parse(const char *buf, const pstring nlname = "");

	void parse_netlist(const pstring &nlname);
	void net_alias();
	void netdev_param();
	void net_c();
	void device(const pstring &dev_type);
	void netdev_netlist_start();
	void netdev_netlist_end();
	void net_model();
	void net_submodel();
	void net_include();

protected:
	virtual void verror(pstring msg, int line_num, pstring line);
private:

	nl_double eval_param(const token_t tok);

	token_id_t m_tok_param_left;
	token_id_t m_tok_param_right;
	token_id_t m_tok_comma;
	token_id_t m_tok_ALIAS;
	token_id_t m_tok_NET_C;
	token_id_t m_tok_PARAM;
	token_id_t m_tok_NET_MODEL;
	token_id_t m_tok_NETLIST_START;
	token_id_t m_tok_NETLIST_END;
	token_id_t m_tok_SUBMODEL;
	token_id_t m_tok_INCLUDE;

	netlist_setup_t &m_setup;

	const char *m_buf;
};

class netlist_source_t
{
public:
	typedef plist_t<netlist_source_t> list_t;

	enum source_e
	{
		EMPTY,
		STRING,
		PROC,
		MEMORY
	};

	netlist_source_t()
	: m_type(EMPTY),
		m_setup_func(NULL),
		m_setup_func_name(""),
		m_mem(NULL)
	{
	}

	netlist_source_t(pstring name, void (*setup_func)(netlist_setup_t &))
	: m_type(PROC),
		m_setup_func(setup_func),
		m_setup_func_name(name),
		m_mem(NULL)
	{
	}

	netlist_source_t(const char *mem)
	: m_type(MEMORY),
		m_setup_func(NULL),
		m_setup_func_name(""),
		m_mem(mem)
	{
	}

	~netlist_source_t() { }

	bool parse(netlist_setup_t &setup, const pstring name)
	{
		switch (m_type)
		{
			case PROC:
				if (name == m_setup_func_name)
				{
					m_setup_func(setup);
					return true;
				}
				break;
			case MEMORY:
				{
					netlist_parser p(setup);
					return p.parse(m_mem, name);
				}
				break;
			case STRING:
			case EMPTY:
				break;
		}
		return false;
	}
private:
	source_e m_type;

	void (*m_setup_func)(netlist_setup_t &);
	pstring m_setup_func_name;
	const char *m_mem;

};

class netlist_sources_t
{
public:

	netlist_sources_t() { }

	~netlist_sources_t()
	{
		m_list.clear();
	}

	void add(netlist_source_t src)
	{
		m_list.add(src);
	}

	void parse(netlist_setup_t &setup, const pstring name)
	{
		for (std::size_t i=0; i < m_list.size(); i++)
		{
			if (m_list[i].parse(setup, name))
				return;
		}
		setup.netlist().error("unable to find %s in source collection", name.cstr());
	}

private:
	netlist_source_t::list_t m_list;
};

#endif /* NL_PARSER_H_ */
