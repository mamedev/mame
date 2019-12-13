// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef NL_CONVERT_H_
#define NL_CONVERT_H_

///
/// \file nl_convert.h
///

#include "plib/plists.h"
#include "plib/pstring.h"
#include "plib/ptokenizer.h"
#include "plib/ptypes.h"

#include <memory>

// -------------------------------------------------
//  convert - convert a spice netlist
// -------------------------------------------------

class nl_convert_base_t
{
public:

	COPYASSIGNMOVE(nl_convert_base_t, delete)

	virtual ~nl_convert_base_t();

	pstring result() { return pstring(m_buf.str()); }

	virtual void convert(const pstring &contents) = 0;

protected:
	nl_convert_base_t();

	void add_pin_alias(const pstring &devname, const pstring &name, const pstring &alias);

	void add_ext_alias(const pstring &alias);

	void add_device(const pstring &atype, const pstring &aname, const pstring &amodel);
	void add_device(const pstring &atype, const pstring &aname, double aval);
	void add_device(const pstring &atype, const pstring &aname);

	void add_term(const pstring &netname, const pstring &termname);

	void dump_nl();

	const pstring get_nl_val(const double val);
	double get_sp_unit(const pstring &unit);

	double get_sp_val(const pstring &sin);

	plib::putf8_fmt_writer out;
private:

	struct net_t
	{
	public:
		explicit net_t(pstring aname)
		: m_name(std::move(aname)), m_no_export(false) {}

		const pstring &name() const { return m_name;}
		std::vector<pstring> &terminals(){ return m_terminals; }
		void set_no_export() { m_no_export = true; }
		bool is_no_export() const { return m_no_export; }

	private:
		pstring m_name;
		bool m_no_export;
		std::vector<pstring> m_terminals;
	};

	struct dev_t
	{
	public:
		dev_t(pstring atype, pstring aname, pstring amodel)
		: m_type(std::move(atype))
		, m_name(std::move(aname))
		, m_model(std::move(amodel))
		, m_val(0)
		, m_has_val(false)
		{}

		dev_t(pstring atype, pstring aname, double aval)
		: m_type(std::move(atype))
		, m_name(std::move(aname))
		, m_model("")
		, m_val(aval)
		, m_has_val(true)
		{}

		dev_t(pstring atype, pstring aname)
		: m_type(std::move(atype))
		, m_name(std::move(aname))
		, m_model("")
		, m_val(0.0)
		, m_has_val(false)
		{}

		const pstring &name() const { return m_name;}
		const pstring &type() const { return m_type;}
		const pstring &model() const { return m_model;}
		double value() const { return m_val;}

		bool has_model() const { return m_model != ""; }
		bool has_value() const { return m_has_val; }

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
		pin_alias_t(pstring name, pstring alias)
		: m_name(std::move(name)), m_alias(std::move(alias))
		{}
		const pstring &name() const { return m_name; }
		const pstring &alias() const { return m_alias; }
	private:
		pstring m_name;
		pstring m_alias;
	};

private:

	void add_device(plib::unique_ptr<dev_t> dev);

	std::stringstream m_buf;

	std::vector<plib::unique_ptr<dev_t>> m_devs;
	std::unordered_map<pstring, plib::unique_ptr<net_t> > m_nets;
	std::vector<pstring> m_ext_alias;
	std::unordered_map<pstring, plib::unique_ptr<pin_alias_t>> m_pins;

	std::vector<unit_t> m_units;
	pstring m_numberchars;

};

class nl_convert_spice_t : public nl_convert_base_t
{
public:

	nl_convert_spice_t() : nl_convert_base_t() {}

	void convert(const pstring &contents) override;

protected:

	void process_line(const pstring &line);

private:
	pstring m_subckt;
};

class nl_convert_eagle_t : public nl_convert_base_t
{
public:

	nl_convert_eagle_t() : nl_convert_base_t() {}

	class tokenizer : public plib::ptokenizer
	{
	public:
		tokenizer(nl_convert_eagle_t &convert, plib::putf8_reader &&strm);

		token_id_t m_tok_ADD;
		token_id_t m_tok_VALUE;
		token_id_t m_tok_SIGNAL;
		token_id_t m_tok_SEMICOLON;

	protected:

		void verror(const pstring &msg) override;

	private:
		nl_convert_eagle_t &m_convert;
	};

	void convert(const pstring &contents) override;

protected:


private:

};

class nl_convert_rinf_t : public nl_convert_base_t
{
public:

	nl_convert_rinf_t() : nl_convert_base_t() {}

	class tokenizer : public plib::ptokenizer
	{
	public:
		tokenizer(nl_convert_rinf_t &convert, plib::putf8_reader &&strm);

		token_id_t m_tok_HEA;
		token_id_t m_tok_APP;
		token_id_t m_tok_TIM;
		token_id_t m_tok_TYP;
		token_id_t m_tok_ADDC;
		token_id_t m_tok_ATTC;
		token_id_t m_tok_NET;
		token_id_t m_tok_TER;
		token_id_t m_tok_END;

	protected:

		void verror(const pstring &msg) override;

	private:
		nl_convert_rinf_t &m_convert;
	};

	void convert(const pstring &contents) override;

protected:


private:

};

#endif // NL_CONVERT_H_
