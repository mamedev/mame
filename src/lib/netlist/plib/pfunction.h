// license:BSD-3-Clause
// copyright-holders:Couriersud

#ifndef PFUNCTION_H_
#define PFUNCTION_H_

///
/// \file pfunction.h
///

#include "pmath.h"
#include "pstring.h"

#include <vector>

namespace plib {

	//============================================================
	//  function evaluation
	//============================================================

	enum rpn_cmd
	{
		ADD,
		MULT,
		SUB,
		DIV,
		EQ,
		NE,
		LT,
		GT,
		LE,
		GE,
		IF,
		NEG,    // unary minus
		POW,
		LOG,
		SIN,
		COS,
		MIN,
		MAX,
		RAND, /// random number between 0 and 1
		TRUNC,

		PUSH_CONST,
		PUSH_INPUT,

		LP, // Left parenthesis - for infix parsing
		RP  // right parenthesis - for infix parsing
	};

	/// \brief Class providing support for evaluating expressions
	///
	///  \tparam NT Number type, should be float or double
	///
	template <typename NT>
	class pfunction
	{
		struct rpn_inst
		{
			constexpr rpn_inst() : m_cmd(ADD)
			{
				m_param.val = plib::constants<NT>::zero();
			}
			constexpr rpn_inst(rpn_cmd cmd, std::size_t index = 0)
			: m_cmd(cmd)
			{
				m_param.index = index;
			}
			constexpr rpn_inst(NT v) : m_cmd(PUSH_CONST)
			{
				m_param.val = v;
			}
			constexpr const rpn_cmd &cmd() const noexcept
			{
				return m_cmd;
			}
			constexpr const NT &value() const noexcept
			{
				return m_param.val; // NOLINT
			}
			constexpr const std::size_t &index() const noexcept
			{
				return m_param.index; // NOLINT
			}
		private:
			rpn_cmd m_cmd;
			union
			{
				NT          val;
				std::size_t index;
			} m_param;
		};
	public:

		using value_type = NT;

		using inputs_container = std::vector<pstring>;
		using values_container = std::vector<value_type>;

		/// \brief Constructor
		///
		pfunction()
		: m_lfsr(0xace1U) // NOLINT
		{
		}

		/// \brief Constructor with compile
		///
		pfunction(const pstring &expr, const inputs_container &inputs = inputs_container())
		: m_lfsr(0xace1U) // NOLINT
		{
			compile(expr, inputs);
		}

		/// \brief Evaluate the expression
		///
		/// \param values for input variables, e.g. {1.1, 2.2}
		/// \return value of expression
		///
		value_type operator()(const values_container &values = values_container()) noexcept
		{
			return evaluate(values);
		}

		/// \brief Compile an expression
		///
		/// \param expr infix or postfix expression. default is infix, postfix
		///          to be prefixed with rpn, e.g. "rpn:A B + 1.3 /"
		/// \param inputs Vector of input variables, e.g. {"A","B"}
		///
		void compile(const pstring &expr, const inputs_container &inputs = inputs_container()) noexcept(false);

		/// \brief Compile a rpn expression
		///
		/// \param expr Reverse polish notation expression, e.g. "A B + 1.3 /"
		/// \param inputs Vector of input variables, e.g. {"A","B"}
		///
		void compile_postfix(const pstring &expr, const inputs_container &inputs = inputs_container()) noexcept(false);

		/// \brief Compile an infix expression
		///
		/// \param expr Infix expression, e.g. "(A+B)/1.3"
		/// \param inputs Vector of input variables, e.g. {"A","B"}
		///
		void compile_infix(const pstring &expr, const inputs_container &inputs = inputs_container()) noexcept(false);

		/// \brief Evaluate the expression
		///
		/// \param values for input variables, e.g. {1.1, 2.2}
		/// \return value of expression
		///
		value_type evaluate(const values_container &values = values_container()) noexcept;

		template <typename ST>
		void save_state(ST &st)
		{
			st.save_item(m_lfsr, "m_lfsr");
		}

	private:

		void compress();
		void compile_postfix(const inputs_container &inputs,
				const std::vector<pstring> &cmds, const pstring &expr);

		std::vector<rpn_inst> m_precompiled; //!< precompiled expression

		std::uint16_t m_lfsr; //!< lfsr used for generating random numbers
	};

	extern template class pfunction<float>;
	extern template class pfunction<double>;
	extern template class pfunction<long double>;
#if (PUSE_FLOAT128)
	extern template class pfunction<FLOAT128>;
#endif

} // namespace plib

#endif // PEXCEPTION_H_
