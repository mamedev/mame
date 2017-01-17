// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * palloc.c
 *
 */

#include <cmath>
#include "pfunction.h"
#include "pfmtlog.h"
#include "putil.h"
#include "pexception.h"

namespace plib {

void pfunction::compile_postfix(const std::vector<pstring> &inputs, const pstring expr)
{
	plib::pstring_vector_t cmds(expr, " ");
	m_precompiled.clear();
	int stk = 0;

	for (pstring &cmd : cmds)
	{
		rpn_inst rc;
		if (cmd == "+")
			{ rc.m_cmd = ADD; stk -= 1; }
		else if (cmd == "-")
			{ rc.m_cmd = SUB; stk -= 1; }
		else if (cmd == "*")
			{ rc.m_cmd = MULT; stk -= 1; }
		else if (cmd == "/")
			{ rc.m_cmd = DIV; stk -= 1; }
		else if (cmd == "pow")
			{ rc.m_cmd = POW; stk -= 1; }
		else if (cmd == "sin")
			{ rc.m_cmd = SIN; stk -= 0; }
		else if (cmd == "cos")
			{ rc.m_cmd = COS; stk -= 0; }
		else
		{
			for (unsigned i = 0; i < inputs.size(); i++)
			{
				if (inputs[i] == cmd)
				{
					rc.m_cmd = PUSH_INPUT;
					rc.m_param = i;
					stk += 1;
					break;
				}
			}
			if (rc.m_cmd != PUSH_INPUT)
			{
				bool err = false;
				rc.m_cmd = PUSH_CONST;
				rc.m_param = cmd.as_double(&err);
				if (err)
					throw plib::pexception(plib::pfmt("nld_function: unknown/misformatted token <{1}> in <{2}>")(cmd)(expr));
				stk += 1;
			}
		}
		if (stk < 1)
			throw plib::pexception(plib::pfmt("nld_function: stack underflow on token <{1}> in <{2}>")(cmd)(expr));
		m_precompiled.push_back(rc);
	}
	if (stk != 1)
		throw plib::pexception(plib::pfmt("nld_function: stack count different to one on <{2}>")(expr));
}

#define ST1 stack[ptr]
#define ST2 stack[ptr-1]

#define OP(OP, ADJ, EXPR) \
case OP: \
	ptr-=ADJ; \
	stack[ptr-1] = EXPR; \
	break;

double pfunction::evaluate(const std::vector<double> &values)
{
	double stack[20];
	unsigned ptr = 0;
	for (auto &rc : m_precompiled)
	{
		switch (rc.m_cmd)
		{
			OP(ADD,  1, ST2 + ST1)
			OP(MULT, 1, ST2 * ST1)
			OP(SUB,  1, ST2 - ST1)
			OP(DIV,  1, ST2 / ST1)
			OP(POW,  1, std::pow(ST2, ST1))
			OP(SIN,  0, std::sin(ST2));
			OP(COS,  0, std::cos(ST2));
			case PUSH_INPUT:
				stack[ptr++] = values[static_cast<unsigned>(rc.m_param)];
				break;
			case PUSH_CONST:
				stack[ptr++] = rc.m_param;
				break;
		}
	}
	return stack[ptr-1];
}

}
