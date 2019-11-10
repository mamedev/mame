// license:GPL-2.0+
// copyright-holders:Couriersud

#include "pfunction.h"
#include "pexception.h"
#include "pfmtlog.h"
#include "pmath.h"
#include "pstonum.h"
#include "pstrutil.h"
#include "putil.h"

#include <stack>
#include <type_traits>

namespace plib {

	template <typename NT>
	void pfunction<NT>::compile(const pstring &expr, const std::vector<pstring> &inputs)
	{
		if (plib::startsWith(expr, "rpn:"))
			compile_postfix(expr.substr(4), inputs);
		else
			compile_infix(expr, inputs);
	}

	template <typename NT>
	void pfunction<NT>::compile_postfix(const pstring &expr, const std::vector<pstring> &inputs)
	{
		std::vector<pstring> cmds(plib::psplit(expr, " "));
		compile_postfix(inputs, cmds, expr);
	}

	template <typename NT>
	void pfunction<NT>::compile_postfix(const std::vector<pstring> &inputs,
			const std::vector<pstring> &cmds, const pstring &expr)
	{
		m_precompiled.clear();
		int stk = 0;

		for (const pstring &cmd : cmds)
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
			else if (cmd == "trunc")
				{ rc.m_cmd = TRUNC; stk -= 0; }
			else if (cmd == "rand")
				{ rc.m_cmd = RAND; stk += 1; }
			else
			{
				for (std::size_t i = 0; i < inputs.size(); i++)
				{
					if (inputs[i] == cmd)
					{
						rc.m_cmd = PUSH_INPUT;
						rc.m_param = static_cast<NT>(i);
						stk += 1;
						break;
					}
				}
				if (rc.m_cmd != PUSH_INPUT)
				{
					rc.m_cmd = PUSH_CONST;
					bool err(false);
					rc.m_param = plib::pstonum_ne<decltype(rc.m_param)>(cmd, err);
					if (err)
						pthrow<pexception>(plib::pfmt("pfunction: unknown/misformatted token <{1}> in <{2}>")(cmd)(expr));
					stk += 1;
				}
			}
			if (stk < 1)
				pthrow<pexception>(plib::pfmt("pfunction: stack underflow on token <{1}> in <{2}>")(cmd)(expr));
			m_precompiled.push_back(rc);
		}
		if (stk != 1)
			pthrow<pexception>(plib::pfmt("pfunction: stack count different to one on <{2}>")(expr));
	}

	static int get_prio(const pstring &v)
	{
		if (v == "(" || v == ")")
			return 1;
		else if (plib::left(v, 1) >= "a" && plib::left(v, 1) <= "z")
			return 0;
		else if (v == "*" || v == "/")
			return 20;
		else if (v == "+" || v == "-")
			return 10;
		else if (v == "^")
			return 30;
		else
			return -1;
	}

	static pstring pop_check(std::stack<pstring> &stk, const pstring &expr)
	{
		if (stk.size() == 0)
			pthrow<pexception>(plib::pfmt("pfunction: stack underflow during infix parsing of: <{1}>")(expr));
		pstring res = stk.top();
		stk.pop();
		return res;
	}

	template <typename NT>
	void pfunction<NT>::compile_infix(const pstring &expr, const std::vector<pstring> &inputs)
	{
		// Shunting-yard infix parsing
		std::vector<pstring> sep = {"(", ")", ",", "*", "/", "+", "-", "^"};
		std::vector<pstring> sexpr1(plib::psplit(plib::replace_all(expr, " ", ""), sep));
		std::stack<pstring> opstk;
		std::vector<pstring> postfix;
		std::vector<pstring> sexpr;

		// FIXME: We really need to switch to ptokenizer and fix negative number
		//        handling in ptokenizer.

		// Fix numbers
		for (std::size_t i = 0; i < sexpr1.size(); )
		{
			if ((i == 0) && (sexpr1.size() > 1) && (sexpr1[0] == "-")
				&& (plib::left(sexpr1[1],1) >= "0") && (plib::left(sexpr1[1],1) <= "9"))
			{
				if (sexpr1.size() < 4)
				{
					sexpr.push_back(sexpr1[0] + sexpr1[1]);
					i+=2;
				}
				else
				{
					auto r(plib::right(sexpr1[1], 1));
					auto ne(sexpr1[2]);
					if ((r == "e" || r == "E") && (ne == "-" || ne == "+"))
					{
						sexpr.push_back(sexpr1[0] + sexpr1[1] + ne + sexpr1[3]);
						i+=4;
					}
					else
					{
						sexpr.push_back(sexpr1[0] + sexpr1[1]);
						i+=2;
					}
				}
			}
			else if (i + 2 < sexpr1.size() && sexpr1[i].length() > 1)
			{
				auto l(plib::left(sexpr1[i], 1));
				auto r(plib::right(sexpr1[i], 1));
				auto ne(sexpr1[i+1]);
				if ((l >= "0") && (l <= "9") && (r == "e" || r == "E") && (ne == "-" || ne == "+"))
				{
					sexpr.push_back(sexpr1[i] + ne + sexpr1[i+2]);
					i+=3;
				}
				else
					sexpr.push_back(sexpr1[i++]);
			}
			else
				sexpr.push_back(sexpr1[i++]);
		}

		for (std::size_t i = 0; i < sexpr.size(); i++)
		{
			pstring &s = sexpr[i];
			if (s=="(")
				opstk.push(s);
			else if (s==")")
			{
				pstring x = pop_check(opstk, expr);
				while (x != "(")
				{
					postfix.push_back(x);
					x = pop_check(opstk, expr);
				}
				if (opstk.size() > 0 && get_prio(opstk.top()) == 0)
					postfix.push_back(pop_check(opstk, expr));
			}
			else if (s==",")
			{
				pstring x = pop_check(opstk, expr);
				while (x != "(")
				{
					postfix.push_back(x);
					x = pop_check(opstk, expr);
				}
				opstk.push(x);
			}
			else {
				int p = get_prio(s);
				if (p>0)
				{
					if (opstk.size() == 0)
						opstk.push(s);
					else
					{
						if (get_prio(opstk.top()) >= get_prio(s))
							postfix.push_back(pop_check(opstk, expr));
						opstk.push(s);
					}
				}
				else if (p == 0) // Function or variable
				{
					if (sexpr[i+1] == "(")
						opstk.push(s);
					else
						postfix.push_back(s);
				}
				else
					postfix.push_back(s);
			}
		}
		while (opstk.size() > 0)
		{
			postfix.push_back(opstk.top());
			opstk.pop();
		}
		//printf("e : %s\n", expr.c_str());
		//for (auto &s : postfix)
		//	printf("x : %s\n", s.c_str());
		compile_postfix(inputs, postfix, expr);
	}

	template <typename NT>
	static inline typename std::enable_if<std::is_floating_point<NT>::value, NT>::type
	lfsr_random(std::uint16_t &lfsr) noexcept
	{
		std::uint16_t lsb = lfsr & 1;
		lfsr >>= 1;
		if (lsb)
			lfsr ^= 0xB400u; // taps 15, 13, 12, 10
		return static_cast<NT>(lfsr) / static_cast<NT>(0xffffu);
	}

	template <typename NT>
	static inline typename std::enable_if<std::is_integral<NT>::value, NT>::type
	lfsr_random(std::uint16_t &lfsr) noexcept
	{
		std::uint16_t lsb = lfsr & 1;
		lfsr >>= 1;
		if (lsb)
			lfsr ^= 0xB400u; // taps 15, 13, 12, 10
		return static_cast<NT>(lfsr);
	}

	#define ST1 stack[ptr]
	#define ST2 stack[ptr-1]

	#define OP(OP, ADJ, EXPR) \
	case OP: \
		ptr-= (ADJ); \
		stack[ptr-1] = (EXPR); \
		break;

	template <typename NT>
	NT pfunction<NT>::evaluate(const std::vector<NT> &values) noexcept
	{
		std::array<NT, 20> stack = { plib::constants<NT>::zero() };
		unsigned ptr = 0;
		stack[0] = plib::constants<NT>::zero();
		for (auto &rc : m_precompiled)
		{
			switch (rc.m_cmd)
			{
				OP(ADD,  1, ST2 + ST1)
				OP(MULT, 1, ST2 * ST1)
				OP(SUB,  1, ST2 - ST1)
				OP(DIV,  1, ST2 / ST1)
				OP(POW,  1, plib::pow(ST2, ST1))
				OP(SIN,  0, plib::sin(ST2))
				OP(COS,  0, plib::cos(ST2))
				OP(TRUNC,  0, plib::trunc(ST2))
				case RAND:
					stack[ptr++] = lfsr_random<NT>(m_lfsr);
					break;
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

	template class pfunction<float>;
	template class pfunction<double>;
	template class pfunction<long double>;
#if (PUSE_FLOAT128)
	template class pfunction<__float128>;
#endif

} // namespace plib
