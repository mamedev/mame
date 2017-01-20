// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * pfunction.h
 *
 */

#ifndef PFUNCTION_H_
#define PFUNCTION_H_

#include <vector>

#include "pconfig.h"
#include "pstring.h"

namespace plib {

	//============================================================
	//  function evaluation - reverse polish notation
	//============================================================

	/*! Class providing support for precompiled rpn expressions
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
		pfunction()
		{
		}

		/*! Compile a rpn expression
		 *
		 * @param inputs Vector of input variables, e.g. {"A","B"}
		 * @param expr Reverse polish notation expression, e.g. "A B + 1.3 /"
		 */
		void compile_postfix(const std::vector<pstring> &inputs, const pstring expr);
		/*! Evaluate the expression
		 *
		 * @param values for input variables, e.g. {1.1, 2.2}
		 * @return value of expression
		 */
		double evaluate(const std::vector<double> &values);

	private:
		std::vector<rpn_inst> m_precompiled; //!< precompiled expression
	};


}

#endif /* PEXCEPTION_H_ */
