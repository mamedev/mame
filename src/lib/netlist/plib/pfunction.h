// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * pfunction.h
 *
 */

#ifndef PFUNCTION_H_
#define PFUNCTION_H_

#include "pstate.h"
#include "pstring.h"

#include <vector>

namespace plib {

	//============================================================
	//  function evaluation
	//============================================================

	/*! Class providing support for evaluating expressions
	 *
	 */
	class pfunction
	{
		enum rpn_cmd
		{
			ADD,
			MULT,
			SUB,
			DIV,
			POW,
			SIN,
			COS,
			RAND, /* random number between 0 and 1 */
			PUSH_CONST,
			PUSH_INPUT
		};
		struct rpn_inst
		{
			rpn_inst() : m_cmd(ADD), m_param(0.0) { }
			rpn_cmd m_cmd;
			double m_param;
		};
	public:
		/*! Constructor with state saving support
		 *
		 * @param name Name of this object
		 * @param owner Owner of this object
		 * @param state_manager State manager to handle saving object state
		 *
		 */
		pfunction(const pstring &name, const void *owner, state_manager_t &state_manager)
		: m_lfsr(0xACE1u)
		{
			state_manager.save_item(owner, m_lfsr, name + ".lfsr");
		}

		/*! Constructor without state saving support
		 *
		 */
		pfunction()
		: m_lfsr(0xACE1u)
		{
		}

		/*! Compile an expression
		 *
		 * @param inputs Vector of input variables, e.g. {"A","B"}
		 * @param expr infix or postfix expression. default is infix, postrix
		 *          to be prefixed with rpn, e.g. "rpn:A B + 1.3 /"
		 */
		void compile(const std::vector<pstring> &inputs, const pstring &expr);

		/*! Compile a rpn expression
		 *
		 * @param inputs Vector of input variables, e.g. {"A","B"}
		 * @param expr Reverse polish notation expression, e.g. "A B + 1.3 /"
		 */
		void compile_postfix(const std::vector<pstring> &inputs, const pstring &expr);
		/*! Compile an infix expression
		 *
		 * @param inputs Vector of input variables, e.g. {"A","B"}
		 * @param expr Infix expression, e.g. "(A+B)/1.3"
		 */
		void compile_infix(const std::vector<pstring> &inputs, const pstring &expr);
		/*! Evaluate the expression
		 *
		 * @param values for input variables, e.g. {1.1, 2.2}
		 * @return value of expression
		 */
		double evaluate(const std::vector<double> &values);

	private:

		void compile_postfix(const std::vector<pstring> &inputs,
				const std::vector<pstring> &cmds, const pstring &expr);

		double lfsr_random()
		{
			std::uint16_t lsb = m_lfsr & 1;
			m_lfsr >>= 1;
			if (lsb)
				m_lfsr ^= 0xB400u; // taps 15, 13, 12, 10
			return static_cast<double>(m_lfsr) / static_cast<double>(0xffffu);
		}

		std::vector<rpn_inst> m_precompiled; //!< precompiled expression

		std::uint16_t m_lfsr; //!< lfsr used for generating random numbers
	};


} // namespace plib

#endif /* PEXCEPTION_H_ */
