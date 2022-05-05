// license:BSD-3-Clause
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

	PERRMSGV(MF_FUNCTION_UNKNOWN_TOKEN,          2, "pfunction: unknown/misformatted token <{1}> in <{2}>")
	PERRMSGV(MF_FUNCTION_STACK_UNDERFLOW,        2, "pfunction: stack underflow on token <{1}> in <{2}>")
	PERRMSGV(MF_FUNCTION_STACK_OVERFLOW,         2, "pfunction: stack overflow on token <{1}> in <{2}>")
	PERRMSGV(MF_FUNCTION_PARENTHESIS_INEQUALITY, 2, "pfunction: parenthesis inequality on token <{1}> in <{2}>")
	PERRMSGV(MF_FUNCTION_STACK_UNEQUAL_ONE,      2, "pfunction: stack count {1} different to one on <{2}>")
	PERRMSGV(MF_FUNCTION_STACK_UNDERFLOW_INFIX,  1, "pfunction: stack underflow during infix parsing of: <{1}>")

	static constexpr const std::size_t MAX_STACK = 32;

	struct pcmd_t
	{
		rpn_cmd cmd;
		int adj;
		int prio;
	};

	static const std::map<pstring, pcmd_t> &pcmds()
	{
		static const std::map<pstring, pcmd_t> lpcmds =
		{
			{ "^",     { POW,   1, 30 } },
			{ "neg",   { NEG,   0, 25 } },
			{ "+",     { ADD,   1, 10 } },
			{ "-",     { SUB,   1, 10 } },
			{ "*",     { MULT,  1, 20 } },
			{ "/",     { DIV,   1, 20 } },
			{ "<",     { LT,    1,  9 } },
			{ ">",     { GT,    1,  9 } },
			{ "<=",    { LE,    1,  9 } },
			{ ">=",    { GE,    1,  9 } },
			{ "==",    { EQ,    1,  8 } },
			{ "!=",    { NE,    1,  8 } },
			{ "if",    { IF,    2,  0 } },
			{ "pow",   { POW,   1,  0 } },
			{ "log",   { LOG,   0,  0 } },
			{ "sin",   { SIN,   0,  0 } },
			{ "cos",   { COS,   0,  0 } },
			{ "max",   { MAX,   1,  0 } },
			{ "min",   { MIN,   1,  0 } },
			{ "trunc", { TRUNC, 0,  0 } },
			{ "rand",  { RAND, -1,  0 } },

			{ "(",     { LP,    0,  1 } },
			{ ")",     { RP,    0,  1 } },
		};
		return lpcmds;
	}
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
		std::vector<pstring> cmds(plib::psplit(expr, ' '));
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
			auto p = pcmds().find(cmd);
			if (p != pcmds().end())
			{
				rc = rpn_inst(p->second.cmd);
				stk -= p->second.adj;
			}
			else
			{
				for (std::size_t i = 0; i < inputs.size(); i++)
				{
					if (inputs[i] == cmd)
					{
						rc = rpn_inst(PUSH_INPUT, i);
						stk += 1;
						break;
					}
				}
				if (rc.cmd() != PUSH_INPUT)
				{
					bool err(false);
					auto rs(plib::right(cmd,1));
					auto r=units_si<NT>().find(rs);
					if (r == units_si<NT>().end())
						rc = rpn_inst(plib::pstonum_ne<NT>(cmd, err));
					else
						rc = rpn_inst(plib::pstonum_ne<NT>(plib::left(cmd, cmd.length()-1), err) * r->second);
					if (err)
						throw pexception(MF_FUNCTION_UNKNOWN_TOKEN(cmd, expr));
					stk += 1;
				}
			}
			if (stk < 1)
				throw pexception(MF_FUNCTION_STACK_UNDERFLOW(cmd, expr));
			if (stk >= narrow_cast<int>(MAX_STACK))
				throw pexception(MF_FUNCTION_STACK_OVERFLOW(cmd, expr));
			if (rc.cmd() == LP || rc.cmd() == RP)
				throw pexception(MF_FUNCTION_PARENTHESIS_INEQUALITY(cmd, expr));
			m_precompiled.push_back(rc);
		}
		if (stk != 1)
			throw pexception(MF_FUNCTION_STACK_UNEQUAL_ONE(stk, expr));
		compress();
	}

	static bool is_number(const pstring &n)
	{
		if (n.empty())
			return false;
		const auto l = n.substr(0,1);
		return (l >= "0" && l <= "9");
	}

	static bool is_id(const pstring &n)
	{
		if (n.empty())
			return false;
		const auto l = n.substr(0,1);
		return ((l >= "a" && l <= "z") || (l >= "A" && l <= "Z"));
	}

	static int get_prio(const pstring &v)
	{
		auto p = pcmds().find(v);
		if (p != pcmds().end())
			return p->second.prio;
		if (plib::left(v, 1) >= "a" && plib::left(v, 1) <= "z")
			return 0;
		return -1;
	}

	static pstring pop_check(std::stack<pstring> &stk, const pstring &expr) noexcept(false)
	{
		if (stk.empty())
			throw pexception(MF_FUNCTION_STACK_UNDERFLOW_INFIX(expr));
		pstring res = stk.top();
		stk.pop();
		return res;
	}

	template <typename NT>
	void pfunction<NT>::compile_infix(const pstring &expr, const inputs_container &inputs)
	{
		// Shunting-yard infix parsing
		std::vector<pstring> sep = {"(", ")", ",", "*", "/", "+", "-", "^", "<=", ">=", "==", "!=", "<", ">"};
		std::vector<pstring> sexpr2(plib::psplit(plib::replace_all(expr, " ", ""), sep));
		std::stack<pstring> opstk;
		std::vector<pstring> postfix;
		std::vector<pstring> sexpr1;
		std::vector<pstring> sexpr;

		// FIXME: We really need to switch to ptokenizer and fix negative number
		//        handling in ptokenizer.

		// Fix numbers exponential numbers
		for (std::size_t i = 0; i < sexpr2.size(); )
		{
			if (i + 2 < sexpr2.size() && sexpr2[i].length() > 1)
			{
				auto r(plib::right(sexpr2[i], 1));
				auto ne(sexpr2[i+1]);
				if ((is_number(sexpr2[i]))
					&& (r == "e" || r == "E")
					&& (ne == "-" || ne == "+"))
				{
					sexpr1.push_back(sexpr2[i] + ne + sexpr2[i+2]);
					i+=3;
				}
				else
					sexpr1.push_back(sexpr2[i++]);
			}
			else
				sexpr1.push_back(sexpr2[i++]);
		}
		// Fix numbers with unary minus/plus
		for (std::size_t i = 0; i < sexpr1.size(); )
		{
			if (sexpr1[i]=="-" && (i+1 < sexpr1.size()) && is_number(sexpr1[i+1]))
			{
				if (i==0 || !(is_number(sexpr1[i-1]) || sexpr1[i-1] == ")" || is_id(sexpr1[i-1])))
				{
					sexpr.push_back("-" + sexpr1[i+1]);
					i+=2;
				}
				else
					sexpr.push_back(sexpr1[i++]);
			}
			else if (sexpr1[i]=="-" && (i+1 < sexpr1.size()) && (is_id(sexpr1[i+1]) || sexpr1[i+1] == "("))
			{
				if (i==0 || !(is_number(sexpr1[i-1]) || sexpr1[i-1] == ")" || is_id(sexpr1[i-1])))
				{
					sexpr.emplace_back("neg");
					sexpr.push_back(sexpr1[i+1]);
					i+=2;
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
				int prio = get_prio(s);
				if (prio>0)
				{
					if (opstk.empty())
						opstk.push(s);
					else
					{
						if (get_prio(opstk.top()) >= prio)
							postfix.push_back(pop_check(opstk, expr));
						opstk.push(s);
					}
				}
				else if (prio == 0) // Function or variable
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
		//  printf("\t%s\n", e.c_str());
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
#if 0
	// Currently unused since no integral type pfunction defined
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
#endif
	#define ST0 *(ptr+1)
	#define ST1 *ptr
	#define ST2 *(ptr-1)

	#define OP(OP, ADJ, EXPR) \
	case OP: \
		ptr-= (ADJ); \
		*(ptr-1) = (EXPR); \
		break;

	#define OP0(OP, EXPR) \
	case OP: \
		*(ptr++) = (EXPR); \
		break;

	template <typename NT>
	NT pfunction<NT>::evaluate(const values_container &values) noexcept
	{
		std::array<value_type, MAX_STACK> stack; // NOLINT
		value_type *ptr = stack.data();
		constexpr const auto zero = plib::constants<value_type>::zero();
		constexpr const auto one = plib::constants<value_type>::one();
		for (const auto &rc : m_precompiled)
		{
			switch (rc.cmd())
			{
				OP(ADD,  1, ST2 + ST1)
				OP(MULT, 1, ST2 * ST1)
				OP(SUB,  1, ST2 - ST1)
				OP(DIV,  1, ST2 / ST1)
				OP(EQ,   1, ST2 == ST1 ? one : zero)
				OP(NE,   1, ST2 != ST1 ? one : zero)
				OP(GT,   1, ST2 > ST1 ? one : zero)
				OP(LT,   1, ST2 < ST1 ? one : zero)
				OP(LE,   1, ST2 <= ST1 ? one : zero)
				OP(GE,   1, ST2 >= ST1 ? one : zero)
				OP(IF,   2, (ST2 != zero) ? ST1 : ST0)
				OP(NEG,  0, -ST2)
				OP(POW,  1, plib::pow(ST2, ST1))
				OP(LOG,  0, plib::log(ST2))
				OP(SIN,  0, plib::sin(ST2))
				OP(COS,  0, plib::cos(ST2))
				OP(MAX,  1, std::max(ST2, ST1))
				OP(MIN,  1, std::min(ST2, ST1))
				OP(TRUNC, 0, plib::trunc(ST2))
				OP0(RAND, lfsr_random<value_type>(m_lfsr))
				OP0(PUSH_INPUT, values[rc.index()])
				OP0(PUSH_CONST, rc.value())
				// please compiler
				case LP:
				case RP:
					break;
			}
		}
		return *(ptr-1);
	}

#undef ST0
#undef ST1
#undef ST2
#undef OP
#undef OP0

#define ST0 m_precompiled[ptr+0].value()
#define ST1 m_precompiled[ptr-1].value()
#define ST2 m_precompiled[ptr-2].value()

#define OP(OP, ADJ, EXPR) \
	case OP: \
		if (ADJ == 2) {\
			if (m_precompiled[ptr-3].cmd() == PUSH_CONST && m_precompiled[ptr-2].cmd() == PUSH_CONST && m_precompiled[ptr-1].cmd() == PUSH_CONST) \
			{   ptr--; m_precompiled[ptr-2] = rpn_inst(EXPR); n -= 3; ptr++; std::copy(m_precompiled.begin()+(ptr+1), m_precompiled.end(), m_precompiled.begin()+(ptr-2)); ptr-=2;} \
			else { ptr++; } \
		} else if (ADJ == 1) {\
			if (m_precompiled[ptr-2].cmd() == PUSH_CONST && m_precompiled[ptr-1].cmd() == PUSH_CONST) \
			{   m_precompiled[ptr-2] = rpn_inst(EXPR); n -= 2; std::copy(m_precompiled.begin()+(ptr+1), m_precompiled.end(), m_precompiled.begin()+(ptr-1)); ptr--;} \
			else { ptr++; } \
		} else if (ADJ == 0) {\
			if (m_precompiled[ptr-1].cmd() == PUSH_CONST) \
			{ ptr++; m_precompiled[ptr-2] = rpn_inst(EXPR); n -= 1; ptr--; std::copy(m_precompiled.begin()+(ptr+1), m_precompiled.end(), m_precompiled.begin()+(ptr)); } \
			else { ptr++; } \
		} else ptr++; \
		break;

#define OP0(OP, EXPR) \
	case OP: \
		ptr++; \
		break;

	template <typename NT>
	void pfunction<NT>::compress()
	{
		constexpr const auto zero = plib::constants<value_type>::zero();
		constexpr const auto one = plib::constants<value_type>::one();
		unsigned ptr = 0;
		auto n = m_precompiled.size();
		for (; ptr < n; )
		{
			switch (m_precompiled[ptr].cmd())
			{
				OP(ADD,  1, ST2 + ST1)
				OP(MULT, 1, ST2 * ST1)
				OP(SUB,  1, ST2 - ST1)
				OP(DIV,  1, ST2 / ST1)
				OP(EQ,   1, ST2 == ST1 ? one : zero)
				OP(NE,   1, ST2 != ST1 ? one : zero)
				OP(GT,   1, ST2 > ST1 ? one : zero)
				OP(LT,   1, ST2 < ST1 ? one : zero)
				OP(LE,   1, ST2 <= ST1 ? one : zero)
				OP(GE,   1, ST2 >= ST1 ? one : zero)
				OP(IF,   2, (ST2 != zero) ? ST1 : ST0)
				OP(NEG,  0, -ST2)
				OP(POW,  1, plib::pow(ST2, ST1))
				OP(LOG,  0, plib::log(ST2))
				OP(SIN,  0, plib::sin(ST2))
				OP(COS,  0, plib::cos(ST2))
				OP(MAX,  1, std::max(ST2, ST1))
				OP(MIN,  1, std::min(ST2, ST1))
				OP(TRUNC,  0, plib::trunc(ST2))
				OP0(RAND, lfsr_random<value_type>(m_lfsr))
				OP0(PUSH_INPUT, values[rc.index()])
				OP0(PUSH_CONST, rc.value())
				// please compiler
				case LP:
				case RP:
					break;
			}
		}
		//printf("func %lld %lld\n", m_precompiled.size(), n);
		m_precompiled.resize(n);
	}

	template class pfunction<float>;
	template class pfunction<double>;
	template class pfunction<long double>;
#if (PUSE_FLOAT128)
	template class pfunction<FLOAT128>;
#endif

} // namespace plib
