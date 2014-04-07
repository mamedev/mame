// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nl_parser.c
 *
 */

#ifndef NL_PARSER_H_
#define NL_PARSER_H_

#include "nl_setup.h"

class ptokenizer
{
	NETLIST_PREVENT_COPYING(ptokenizer)
public:
	virtual ~ptokenizer() {}

	ptokenizer()
	: m_line(1), m_line_ptr(NULL), m_px(NULL)
	{}

	enum token_type
	{
		IDENTIFIER,
		NUMBER,
		TOKEN,
		STRING,
		COMMENT,
		UNKNOWN,
		ENDOFFILE,
	};

	struct token_id_t
	{
	public:
		token_id_t() : m_id(-2) {}
		token_id_t(const int id) : m_id(id) {}
		const int id() const { return m_id; }
	private:
		int m_id;
	};

	struct token_t
	{
		token_t() {};
		token_t(token_type type)
		{
			m_type = type;
			m_id = token_id_t(-1);
			m_token ="";
		}
		token_t(token_type type, const pstring str)
		{
			m_type = type;
			m_id = token_id_t(-1);
			m_token = str;
		}
		token_t(const token_id_t id, const pstring str)
		{
			m_type = TOKEN;
			m_id = id;
			m_token = str;
		}

		bool is(const token_id_t &tok_id) const { return m_id.id() == tok_id.id(); }
		bool is_not(const token_id_t &tok_id) const { return !is(tok_id); }

		bool is_type(const token_type type) const { return m_type == type; }

		pstring str() const { return m_token; }

	private:
		token_type m_type;
		token_id_t m_id;
		pstring m_token;
	};


	int currentline_no() { return m_line; }
	pstring currentline_str();

	/* tokenizer stuff follows ... */

	token_t get_token();
	pstring get_string();
	pstring get_identifier();

	void require_token(const token_id_t &token_num);
	void require_token(const token_t tok, const token_id_t &token_num);

	token_id_t register_token(pstring token)
	{
		m_tokens.add(token);
		return token_id_t(m_tokens.count() - 1);
	}

	void set_identifier_chars(pstring s) { m_identifier_chars = s; }
	void set_number_chars(pstring s) { m_number_chars = s; }
	void set_whitespace(pstring s) { m_whitespace = s; }
	void set_comment(pstring start, pstring end, pstring line)
	{
		m_tok_comment_start = register_token(start);
		m_tok_comment_end = register_token(end);
		m_tok_line_comment = register_token(line);
		m_string = '"';
	}

	token_t get_token_internal();
	void error(const char *format, ...) ATTR_PRINTF(2,3);

protected:
	void reset(const char *p) { m_px = p; m_line = 1; m_line_ptr = p; }
	virtual void verror(pstring msg, int line_num, pstring line) = 0;

private:
	void skipeol();

	unsigned char getc();
	void ungetc();
	bool eof() { return *m_px == 0; }

	int m_line;
	const char * m_line_ptr;
	const char * m_px;

	/* tokenizer stuff follows ... */

	pstring m_identifier_chars;
	pstring m_number_chars;
	netlist_list_t<pstring> m_tokens;
	pstring m_whitespace;
	char  m_string;

	token_id_t m_tok_comment_start;
	token_id_t m_tok_comment_end;
	token_id_t m_tok_line_comment;
};

class netlist_parser : public ptokenizer
{
	NETLIST_PREVENT_COPYING(netlist_parser)
public:
	netlist_parser(netlist_setup_t &setup)
	: ptokenizer(), m_setup(setup) {}

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

	double eval_param(const token_t tok);

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
	typedef netlist_list_t<netlist_source_t> list_t;

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
	void add(netlist_source_t src)
	{
		m_list.add(src);
	}

	void parse(netlist_setup_t &setup, const pstring name)
	{
		for (int i=0; i < m_list.count(); i++)
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
