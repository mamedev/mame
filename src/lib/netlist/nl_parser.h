// license:GPL-2.0+
// copyright-holders:Couriersud

///
/// \file nl_parser.h
///

#ifndef NL_PARSER_H_
#define NL_PARSER_H_

#include "nl_setup.h"
#include "plib/ptokenizer.h"

namespace netlist
{
	class parser_t : public plib::ptokenizer
	{
	public:
		template <typename T>
		parser_t(T &&strm, nlparse_t &setup)
			: plib::ptokenizer(std::forward<T>(strm))
			, m_setup(setup)
		{
		}

		bool parse(const pstring &nlname = "");

	protected:
		void parse_netlist(const pstring &nlname);
		void net_alias();
		void dippins();
		void netdev_param();
		void netdev_hint();
		void net_c();
		void frontier();
		void device(const pstring &dev_type);
		void netdev_netlist_end();
		void net_model();
		void net_submodel();
		void net_include();
		void net_local_source();
		void net_truthtable_start(const pstring &nlname);

		void verror(const pstring &msg) override;
	private:

		nl_fptype eval_param(const token_t &tok);

		token_id_t m_tok_paren_left;
		token_id_t m_tok_paren_right;
		token_id_t m_tok_comma;
		token_id_t m_tok_ALIAS;
		token_id_t m_tok_NET_C;
		token_id_t m_tok_DIPPINS;
		token_id_t m_tok_FRONTIER;
		token_id_t m_tok_PARAM;
		token_id_t m_tok_HINT;
		token_id_t m_tok_NET_MODEL;
		token_id_t m_tok_NETLIST_START;
		token_id_t m_tok_NETLIST_END;
		token_id_t m_tok_SUBMODEL;
		token_id_t m_tok_INCLUDE;
		token_id_t m_tok_LOCAL_SOURCE;
		token_id_t m_tok_LOCAL_LIB_ENTRY;
		token_id_t m_tok_TRUTHTABLE_START;
		token_id_t m_tok_TRUTHTABLE_END;
		token_id_t m_tok_TT_HEAD;
		token_id_t m_tok_TT_LINE;
		token_id_t m_tok_TT_FAMILY;

		nlparse_t &m_setup;
};

} // namespace netlist

#endif // NL_PARSER_H_
