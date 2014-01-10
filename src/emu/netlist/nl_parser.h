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
	NETLIST_PREVENT_COPYING(netlist_parser)
public:
	netlist_parser(netlist_setup_t &setup)
	: m_line(1), m_line_ptr(NULL), m_px(NULL), m_setup(setup) {}

	void parse(const char *buf);
	void net_alias();
	void netdev_param();
	void net_c();
	void netdev_device(const pstring &dev_type);
    void netdev_netlist_start();
    void netdev_netlist_end();
    void net_model();

    void error(const char *format, ...);
private:

	void skipeol();
	void skipws();
	pstring getname(char sep);
	pstring getname2(char sep1, char sep2);
    pstring getname2_ext(char sep1, char sep2, const char *allowed);
	void check_char(char ctocheck);
	double eval_param();
    pstring getstring();

    unsigned char peekc();
	unsigned char getc();
	void ungetc();
	bool eof() { return *m_px == 0; }

	int m_line;
	const char * m_line_ptr;
	const char * m_px;
	netlist_setup_t &m_setup;
};


#endif /* NL_PARSER_H_ */
