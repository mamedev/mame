// license:BSD-3-Clause
// copyright-holders:Couriersud

///
/// \file nl_parser.h
///

#ifndef NL_PARSER_H_
#define NL_PARSER_H_

#include "nltypes.h" // for setup_t

#include "core/setup.h"

#include "plib/ptokenizer.h"

#include <unordered_map>

namespace netlist
{
	class parser_t : public plib::token_reader_t
	{
	public:
		using token_t = plib::tokenizer_t::token_t;
		using token_type = plib::tokenizer_t::token_type;
		using token_id_t = plib::tokenizer_t::token_id_t;
		using token_store_t = plib::tokenizer_t::token_store_t;

		parser_t(nlparse_t &setup);

		bool parse(plib::istream_uptr &&strm, const pstring &nlname);
		bool parse(const token_store_t &store, const pstring &nlname);
		void parse_tokens(plib::istream_uptr &&strm, token_store_t &tokstore);

	protected:
		void parse_netlist();
		void net_alias();
		void dip_pins();
		void netdev_param();
		void netdev_default_param();
		void netdev_hint();
		void net_c();
		void frontier();
		void device(const pstring &dev_type);
		void net_model();
		void net_sub_model();
		void net_include();
		void net_local_source();
		void net_external_source();
		void net_lib_entry(bool is_local);
		void net_register_dev();
		void net_truth_table_start(const pstring &nlname);

		void verror(const pstring &msg) override;

	private:
		void register_local_as_source(const pstring &name);

		pstring stringify_expression(token_t &tok);

		token_id_t m_tok_paren_left;
		token_id_t m_tok_paren_right;
		token_id_t m_tok_brace_left;
		token_id_t m_tok_brace_right;
		token_id_t m_tok_comma;
		token_id_t m_tok_static;
		token_id_t m_tok_ALIAS;
		token_id_t m_tok_NET_C;
		token_id_t m_tok_DIPPINS;
		token_id_t m_tok_FRONTIER;
		token_id_t m_tok_PARAM;
		token_id_t m_tok_DEFPARAM;
		token_id_t m_tok_HINT;
		token_id_t m_tok_NET_MODEL;
		token_id_t m_tok_NET_REGISTER_DEV;
		token_id_t m_tok_NETLIST_START;
		token_id_t m_tok_NETLIST_EXTERNAL;
		token_id_t m_tok_SUBMODEL;
		token_id_t m_tok_INCLUDE;
		token_id_t m_tok_EXTERNAL_SOURCE;
		token_id_t m_tok_LOCAL_SOURCE;
		token_id_t m_tok_LOCAL_LIB_ENTRY;
		token_id_t m_tok_EXTERNAL_LIB_ENTRY;
		token_id_t m_tok_TRUTH_TABLE;
		token_id_t m_tok_TRUTHTABLE_ENTRY;
		token_id_t m_tok_TT_HEAD;
		token_id_t m_tok_TT_LINE;
		token_id_t m_tok_TT_FAMILY;

		plib::tokenizer_t m_tokenizer;
		nlparse_t &       m_setup;

		std::unordered_map<pstring, token_store_t> m_local;
		token_store_t *                            m_cur_local;
	};

	class source_token_t : public source_netlist_t
	{
	public:
		source_token_t(const pstring &     name,
			const parser_t::token_store_t &store)
		: m_store(store)
		, m_name(name)
		{
		}

		bool parse(nlparse_t &setup, const pstring &name) override;

	protected:
		plib::istream_uptr stream(const pstring &name) override;

	private:
		parser_t::token_store_t m_store;
		pstring                 m_name;
	};

} // namespace netlist

#endif // NL_PARSER_H_
