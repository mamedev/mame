// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nl_convert.h
 *
 */

#pragma once

#ifndef NL_CONVERT_H_
#define NL_CONVERT_H_

#include "plib/pstring.h"
#include "plib/plists.h"
#include "plib/pparser.h"

/*-------------------------------------------------
    convert - convert a spice netlist
-------------------------------------------------*/

class nl_convert_base_t
{
public:

	nl_convert_base_t() : out(m_buf) {};
	virtual ~nl_convert_base_t()
	{
		m_nets.clear_and_free();
		m_devs.clear_and_free();
		m_pins.clear_and_free();
	}

	const pstringbuffer &result() { return m_buf.str(); }

	virtual void convert(const pstring &contents) = 0;

protected:

	void add_pin_alias(const pstring &devname, const pstring &name, const pstring &alias);

	void add_ext_alias(const pstring &alias);

	void add_device(const pstring &atype, const pstring &aname, const pstring &amodel);
	void add_device(const pstring &atype, const pstring &aname, double aval);
	void add_device(const pstring &atype, const pstring &aname);

	void add_term(pstring netname, pstring termname);

	void dump_nl();

	const pstring get_nl_val(const double val);
	double get_sp_unit(const pstring &unit);

	double get_sp_val(const pstring &sin);

	pstream_fmt_writer_t out;
private:
	struct net_t
	{
	public:
		net_t(const pstring &aname)
		: m_name(aname), m_no_export(false) {}

		const pstring &name() { return m_name;}
		pstring_list_t &terminals() { return m_terminals; }
		void set_no_export() { m_no_export = true; }
		bool is_no_export() { return m_no_export; }

	private:
		pstring m_name;
		bool m_no_export;
		pstring_list_t m_terminals;
	};

	struct dev_t
	{
	public:
		dev_t(const pstring &atype, const pstring &aname, const pstring &amodel)
		: m_type(atype), m_name(aname), m_model(amodel), m_val(0), m_has_val(false)
		{}

		dev_t(const pstring &atype, const pstring &aname, double aval)
		: m_type(atype), m_name(aname), m_model(""), m_val(aval), m_has_val(true)
		{}

		dev_t(const pstring &atype, const pstring &aname)
		: m_type(atype), m_name(aname), m_model(""), m_val(0.0), m_has_val(false)
		{}

		const pstring &name() { return m_name;}
		const pstring &type() { return m_type;}
		const pstring &model() { return m_model;}
		const double &value() { return m_val;}

		bool has_model() { return m_model != ""; }
		bool has_value() { return m_has_val; }

	private:
		pstring m_type;
		pstring m_name;
		pstring m_model;
		double m_val;
		bool m_has_val;
	};

	struct unit_t {
		pstring m_unit;
		pstring m_func;
		double m_mult;
	};

	struct pin_alias_t
	{
	public:
		pin_alias_t(const pstring &name, const pstring &alias)
		: m_name(name), m_alias(alias)
		{}
		const pstring &name() { return m_name; }
		const pstring &alias() { return m_alias; }
	private:
		pstring m_name;
		pstring m_alias;
	};

private:

	postringstream m_buf;

	pnamedlist_t<dev_t *> m_devs;
	pnamedlist_t<net_t *> m_nets;
	plist_t<pstring> m_ext_alias;
	pnamedlist_t<pin_alias_t *> m_pins;

	static unit_t m_units[];

};

class nl_convert_spice_t : public nl_convert_base_t
{
public:

	nl_convert_spice_t() : nl_convert_base_t() {};
	~nl_convert_spice_t()
	{
	}

	void convert(const pstring &contents) override;

protected:

	void process_line(const pstring &line);

private:

};

class nl_convert_eagle_t : public nl_convert_base_t
{
public:

	nl_convert_eagle_t() : nl_convert_base_t() {};
	~nl_convert_eagle_t()
	{
	}

	class eagle_tokenizer : public ptokenizer
	{
	public:
		eagle_tokenizer(nl_convert_eagle_t &convert, pistream &strm)
		: ptokenizer(strm), m_convert(convert)
		{
			set_identifier_chars("abcdefghijklmnopqrstuvwvxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890_.-");
			set_number_chars(".0123456789", "0123456789eE-."); //FIXME: processing of numbers
			char ws[5];
			ws[0] = ' ';
			ws[1] = 9;
			ws[2] = 10;
			ws[3] = 13;
			ws[4] = 0;
			set_whitespace(ws);
			/* FIXME: gnetlist doesn't print comments */
			set_comment("/*", "*/", "//");
			set_string_char('\'');
			m_tok_ADD = register_token("ADD");
			m_tok_VALUE = register_token("VALUE");
			m_tok_SIGNAL = register_token("SIGNAL");
			m_tok_SEMICOLON = register_token(";");
			/* currently not used, but required for parsing */
			register_token(")");
			register_token("(");
		}

		token_id_t m_tok_ADD;
		token_id_t m_tok_VALUE;
		token_id_t m_tok_SIGNAL;
		token_id_t m_tok_SEMICOLON;

	protected:

		void verror(const pstring &msg, int line_num, const pstring &line) override
		{
			m_convert.out("{} (line {}): {}\n", msg.cstr(), line_num, line.cstr());
		}


	private:
		nl_convert_eagle_t &m_convert;
	};

	void convert(const pstring &contents) override;

protected:


private:

};

#endif /* NL_CONVERT_H_ */
