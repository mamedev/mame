// license:BSD-3-Clause
// copyright-holders:Couriersud

#ifndef NL_CONVERT_H_
#define NL_CONVERT_H_

///
/// \file nl_convert.h
///

#include "plib/palloc.h"
#include "plib/pstring.h"
#include "plib/ptypes.h"

#include <memory>

#include "../plib/ptokenizer.h"

// -------------------------------------------------
//  convert - convert a spice netlist
// -------------------------------------------------

namespace netlist
{

namespace convert
{

using arena = plib::aligned_arena;

class nl_convert_base_t
{
public:
	using str_list = std::vector<pstring>;

	PCOPYASSIGNMOVE(nl_convert_base_t, delete)

	virtual ~nl_convert_base_t();

	pstring result() { return pstring(putf8string(m_buf.str())); }

	virtual void convert(const pstring &contents) = 0;

protected:
	nl_convert_base_t();

	void add_pin_alias(const pstring &devname, const pstring &name, const pstring &alias);

	void add_ext_alias(const pstring &alias);
	void add_ext_alias(const pstring &alias, const pstring &net);

	void add_device(const pstring &atype, const pstring &aname, const pstring &amodel);
	void add_device(const pstring &atype, const pstring &aname, double aval);
	void add_device(const pstring &atype, const pstring &aname);

	void add_device_extra_s(const pstring &devname, const pstring &extra);

	template<typename... Args>
	void add_device_extra(const pstring &devname, const pstring &fmt, Args&&... args)
	{
		add_device_extra_s(devname, plib::pfmt(fmt)(std::forward<Args>(args)...));
	}

	void add_term(const pstring &netname, const pstring &termname);
	void add_term(const pstring &netname, const pstring &devname, unsigned term);

	void dump_nl();

	pstring get_nl_val(double val) const;
	double get_sp_unit(const pstring &unit) const;

	double get_sp_val(const pstring &sin) const;

	plib::putf8_fmt_writer out;

	struct replace_t
	{
		pstring m_ce; // controlling element - must be twoterm
		pstring m_repterm; // replace with terminal
		pstring m_net; // connect to net
	};
	std::vector<replace_t> m_replace;

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
		const str_list &extra() const { return m_extra;}

		bool has_model() const { return !m_model.empty(); }
		bool has_value() const { return m_has_val; }

		void add_extra(const pstring &s) { m_extra.push_back(s); }
	private:
		pstring m_type;
		pstring m_name;
		pstring m_model;
		double m_val;
		bool m_has_val;
		str_list m_extra;
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

	void add_device(arena::unique_ptr<dev_t> dev);
	dev_t *get_device(const pstring &name)
	{
		for (auto &e : m_devs)
			if (e->name() == name)
				return e.get();
		return nullptr;
	}

	std::stringstream m_buf;

	std::vector<arena::unique_ptr<dev_t>> m_devs;
	std::unordered_map<pstring, arena::unique_ptr<net_t> > m_nets;
	std::vector<std::pair<pstring, pstring>> m_ext_alias;
	std::unordered_map<pstring, arena::unique_ptr<pin_alias_t>> m_pins;

	std::vector<unit_t> m_units;
	pstring m_numberchars;

	std::unordered_map<pstring, str_list> dev_map;

};

class nl_convert_spice_t : public nl_convert_base_t
{
public:

	nl_convert_spice_t() : m_is_kicad(false) { }

	void convert(const pstring &contents) override;

protected:

	bool is_kicad() const { return m_is_kicad; }
	void convert_block(const str_list &contents);
	void process_line(const pstring &line);

private:
	pstring m_subckt;
	bool m_is_kicad;
};

class nl_convert_eagle_t : public nl_convert_base_t
{
public:

	nl_convert_eagle_t() = default;

	class tokenizer : public plib::ptokenizer, public plib::ptoken_reader
	{
	public:
		using token_t = ptokenizer::token_t;
		using token_type = ptokenizer::token_type;
		using token_id_t = ptokenizer::token_id_t;
		using token_store = ptokenizer::token_store;

		tokenizer(nl_convert_eagle_t &convert);

		token_id_t m_tok_ADD;       // NOLINT
		token_id_t m_tok_VALUE;     // NOLINT
		token_id_t m_tok_SIGNAL;    // NOLINT
		token_id_t m_tok_SEMICOLON; // NOLINT
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

	nl_convert_rinf_t() = default;

	class tokenizer : public plib::ptokenizer, public plib::ptoken_reader
	{
	public:
		using token_t = ptokenizer::token_t;
		using token_type = ptokenizer::token_type;
		using token_id_t = ptokenizer::token_id_t;
		using token_store = ptokenizer::token_store;
		tokenizer(nl_convert_rinf_t &convert);

		token_id_t m_tok_HEA; // NOLINT
		token_id_t m_tok_APP; // NOLINT
		token_id_t m_tok_TIM; // NOLINT
		token_id_t m_tok_TYP; // NOLINT
		token_id_t m_tok_ADDC; // NOLINT
		token_id_t m_tok_ATTC; // NOLINT
		token_id_t m_tok_NET; // NOLINT
		token_id_t m_tok_TER; // NOLINT
		token_id_t m_tok_END; // NOLINT
	protected:

		void verror(const pstring &msg) override;

	private:
		nl_convert_rinf_t &m_convert;
	};

	void convert(const pstring &contents) override;

protected:


private:

};

} // namespace convert
} // namespace netlist

#endif // NL_CONVERT_H_
