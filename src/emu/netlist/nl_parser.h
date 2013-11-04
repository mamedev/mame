// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nl_parser.c
 *
 */

#ifndef NL_PARSER_H_
#define NL_PARSER_H_

#include "nl_setup.h"

class netlist_parser
{
public:
	netlist_parser(netlist_setup_t &setup)
	: m_setup(setup) {}

	void parse(char *buf);
	void net_alias();
	void netdev_param();
	void netdev_const(const astring &dev_name);
	void netdev_device(const astring &dev_type);

private:

	void skipeol();
	void skipws();
	astring getname(char sep);
	astring getname2(char sep1, char sep2);
	void check_char(char ctocheck);
	double eval_param();

	char * m_p;
	netlist_setup_t &m_setup;
};


#endif /* NL_PARSER_H_ */
