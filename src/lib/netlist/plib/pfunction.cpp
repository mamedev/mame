// license:GPL-2.0+
// copyright-holders:Couriersud

#include "pfunction.h"
#include "pexception.h"
#include "pfmtlog.h"
#include "pmath.h"
#include "pstonum.h"
#include "pstrutil.h"
#include "putil.h"

#include <array>
#include <map>
#include <stack>
#include <type_traits>
#include <utility>

namespace plib {

	static constexpr const std::size_t MAX_STACK = 32;

	// FIXME: Exa parsing conflicts with e,E parsing
	template<typename F>
	static const std::map<pstring, F> &units_si()
	{
		static std::map<pstring, F> units_si_stat =
		{
			//{ "Y", narrow_cast<F>(1e24) }, // NOLINT: Yotta
			//{ "Z", narrow_cast<F>(1e21) }, // NOLINT: Zetta
			//{ "E", narrow_cast<F>(1e18) }, // NOLINT: Exa
			{ "P", narrow_cast<F>(1e15) }, // NOLINT: Peta
			{ "T", narrow_cast<F>(1e12) }, // NOLINT: Tera
			{ "G", narrow_cast<F>( 1e9) }, // NOLINT: Giga
			{ "M", narrow_cast<F>( 1e6) }, // NOLINT: Mega
			{ "k", narrow_cast<F>( 1e3) }, // NOLINT: Kilo
			{ "h", narrow_cast<F>( 1e2) }, // NOLINT: Hekto
			//{ "da", narrow_cast<F>(1e1) }, // NOLINT: Deka
			{ "d", narrow_cast<F>(1e-1) }, // NOLINT: Dezi
			{ "c", narrow_cast<F>(1e-2) }, // NOLINT: Zenti
			{ "m", narrow_cast<F>(1e-3) }, // NOLINT: Milli
			{ "μ", narrow_cast<F>(1e-6) }, // NOLINT: Mikro
			{ "n", narrow_cast<F>(1e-9) }, // NOLINT: Nano
			{ "p", narrow_cast<F>(1e-12) }, // NOLINT: Piko
			{ "f", narrow_cast<F>(1e-15) }, // NOLINT: Femto
			{ "a", narrow_cast<F>(1e-18) }, // NOLINT: Atto
			{ "z", narrow_cast<F>(1e-21) }, // NOLINT: Zepto
			{ "y", narrow_cast<F>(1e-24) }, // NOLINT: Yokto
		};
		return units_si_stat;
	}


	template <typename NT>
	void pfunction<NT>::compile(const pstring &expr, const inputs_container &inputs) noexcept(false)
	{
		if (plib::startsWith(expr, "rpn:"))
			compile_postfix(expr.substr(4), inputs);
		else
			compile_infix(expr, inputs);
	}

	template <typename NT>
	void pfunction<NT>::compile_postfix(const pstring &expr, const inputs_container &inputs) noexcept(false)
	{
		std::vector<pstring> cmds(plib::psplit(expr, " "));
		compile_postfix(inputs, cmds, expr);
	}

	template <typename NT>
	void pfunction<NT>::compile_postfix(const inputs_container &inputs,
			const std::vector<pstring> &cmds, const pstring &expr) noexcept(false)
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
			else if (cmd == "==")
				{ rc.m_cmd = EQ; stk -= 1; }
			else if (cmd == "!=")
				{ rc.m_cmd = NE; stk -= 1; }
			else if (cmd == "<")
				{ rc.m_cmd = LT; stk -= 1; }
			else if (cmd == ">")
				{ rc.m_cmd = GT; stk -= 1; }
			else if (cmd == "<=")
				{ rc.m_cmd = LE; stk -= 1; }
			else if (cmd == ">=")
				{ rc.m_cmd = GE; stk -= 1; }
			else if (cmd == "if")
				{ rc.m_cmd = IF; stk -= 2; }
			else if (cmd == "pow")
				{ rc.m_cmd = POW; stk -= 1; }
			else if (cmd == "log")
				{ rc.m_cmd = LOG; stk -= 0; }
			else if (cmd == "sin")
				{ rc.m_cmd = SIN; stk -= 0; }
			else if (cmd == "cos")
				{ rc.m_cmd = COS; stk -= 0; }
			else if (cmd == "max")
				{ rc.m_cmd = MAX; stk -= 1; }
			else if (cmd == "min")
				{ rc.m_cmd = MIN; stk -= 1; }
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
						rc.m_param = narrow_cast<NT>(i);
						stk += 1;
						break;
					}
				}
				if (rc.m_cmd != PUSH_INPUT)
				{
					using fl_t = decltype(rc.m_param);
					rc.m_cmd = PUSH_CONST;
					bool err(false);
					auto rs(plib::right(cmd,1));
					auto r=units_si<fl_t>().find(rs);
					if (r == units_si<fl_t>().end())
						rc.m_param = plib::pstonum_ne<fl_t>(cmd, err);
					else
						rc.m_param = plib::pstonum_ne<fl_t>(plib::left(cmd, cmd.size()-1), err) * r->second;
					if (err)
						throw pexception(plib::pfmt("pfunction: unknown/misformatted token <{1}> in <{2}>")(cmd)(expr));
					stk += 1;
				}
			}
			if (stk < 1)
				throw pexception(plib::pfmt("pfunction: stack underflow on token <{1}> in <{2}>")(cmd)(expr));
			if (stk >= narrow_cast<int>(MAX_STACK))
				throw pexception(plib::pfmt("pfunction: stack overflow on token <{1}> in <{2}>")(cmd)(expr));
			m_precompiled.push_back(rc);
		}
		if (stk != 1)
			throw pexception(plib::pfmt("pfunction: stack count different to one on <{2}>")(expr));
	}

	static int get_prio(const pstring &v)
	{
		if (v == "(" || v == ")")
			return 1;
		if (plib::left(v, 1) >= "a" && plib::left(v, 1) <= "z")
			return 0;
		if (v == "^")
			return 30;
		if (v == "*" || v == "/")
			return 20;
		if (v == "+" || v == "-")
			return 10;
		if (v == "<" || v == ">" || v == "<=" || v == ">=")
			return 9;
		if (v == "==" || v == "!=")
			return 8;

		return -1;
	}

	static pstring pop_check(std::stack<pstring> &stk, const pstring &expr) noexcept(false)
	{
		if (stk.empty())
			throw pexception(plib::pfmt("pfunction: stack underflow during infix parsing of: <{1}>")(expr));
		pstring res = stk.top();
		stk.pop();
		return res;
	}

	template <typename NT>
	void pfunction<NT>::compile_infix(const pstring &expr, const inputs_container &inputs)
	{
		// Shunting-yard infix parsing
		std::vector<pstring> sep = {"(", ")", ",", "*", "/", "+", "-", "^", "<=", ">=", "==", "!=", "<", ">"};
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
				if ((l >= "0") && (l <= "9")
					&& (r == "e" || r == "E")
					&& (ne == "-" || ne == "+"))
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
				if (!opstk.empty() && get_prio(opstk.top()) == 0)
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
					if (opstk.empty())
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
					if ((i+1<sexpr.size()) && sexpr[i+1] == "(")
						opstk.push(s);
					else
						postfix.push_back(s);
				}
				else
					postfix.push_back(s);
			}
		}
		while (!opstk.empty())
		{
			postfix.push_back(opstk.top());
			opstk.pop();
		}
		//for (auto &e : postfix)
		//	printf("\t %s\n", e.c_str());
		compile_postfix(inputs, postfix, expr);
	}

	template <typename NT>
	static inline std::enable_if_t<plib::is_floating_point<NT>::value, NT>
	lfsr_random(std::uint16_t &lfsr) noexcept
	{
		std::uint16_t lsb = lfsr & 1;
		lfsr >>= 1;
		if (lsb)
			lfsr ^= 0xB400U; // NOLINT: taps 15, 13, 12, 10
		return narrow_cast<NT>(lfsr) / narrow_cast<NT>(0xffffU); // NOLINT
	}

	template <typename NT>
	static inline std::enable_if_t<plib::is_integral<NT>::value, NT>
	lfsr_random(std::uint16_t &lfsr) noexcept
	{
		std::uint16_t lsb = lfsr & 1;
		lfsr >>= 1;
		if (lsb)
			lfsr ^= 0xB400U; // NOLINT: taps 15, 13, 12, 10
		return narrow_cast<NT>(lfsr);
	}

	#define ST0 stack[ptr+1]
	#define ST1 stack[ptr]
	#define ST2 stack[ptr-1]

	#define OP(OP, ADJ, EXPR) \
	case OP: \
		ptr-= (ADJ); \
		stack[ptr-1] = (EXPR); \
		break;

	template <typename NT>
	NT pfunction<NT>::evaluate(const values_container &values) noexcept
	{
		std::array<value_type, MAX_STACK> stack = { plib::constants<value_type>::zero() };
		unsigned ptr = 0;
		stack[0] = plib::constants<value_type>::zero();
		for (auto &rc : m_precompiled)
		{
			switch (rc.m_cmd)
			{
				OP(ADD,  1, ST2 + ST1)
				OP(MULT, 1, ST2 * ST1)
				OP(SUB,  1, ST2 - ST1)
				OP(DIV,  1, ST2 / ST1)
				OP(EQ,   1, ST2 == ST1 ? 1.0 : 0.0)
				OP(NE,   1, ST2 != ST1 ? 1.0 : 0.0)
				OP(GT,   1, ST2 > ST1 ? 1.0 : 0.0)
				OP(LT,   1, ST2 < ST1 ? 1.0 : 0.0)
				OP(LE,   1, ST2 <= ST1 ? 1.0 : 0.0)
				OP(GE,   1, ST2 >= ST1 ? 1.0 : 0.0)
				OP(IF,   2, (ST2 != 0.0) ? ST1 : ST0)
				OP(POW,  1, plib::pow(ST2, ST1))
				OP(LOG,  0, plib::log(ST2))
				OP(SIN,  0, plib::sin(ST2))
				OP(COS,  0, plib::cos(ST2))
				OP(MAX,  1, std::max(ST2, ST1))
				OP(MIN,  1, std::min(ST2, ST1))
				OP(TRUNC,  0, plib::trunc(ST2))
				case RAND:
					stack[ptr++] = lfsr_random<value_type>(m_lfsr);
					break;
				case PUSH_INPUT:
					stack[ptr++] = values[narrow_cast<unsigned>(rc.m_param)];
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
	template class pfunction<FLOAT128>;
#endif

} // namespace plib
