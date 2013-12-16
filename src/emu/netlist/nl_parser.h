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
    void net_c();
	void netdev_const(const pstring &dev_name);
	void netdev_device(const pstring &dev_type);
    void netdev_device(const pstring &dev_type, const pstring &default_param, bool isString = false);

private:

	void skipeol();
	void skipws();
	pstring getname(char sep);
	pstring getname2(char sep1, char sep2);
	void check_char(char ctocheck);
	double eval_param();

	char * m_p;
	netlist_setup_t &m_setup;
};


#endif /* NL_PARSER_H_ */
