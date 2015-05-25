// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_ms_direct1.h
 *
 */

#ifndef NLD_MS_DIRECT2_H_
#define NLD_MS_DIRECT2_H_

#include "nld_solver.h"
#include "nld_ms_direct.h"



class netlist_matrix_solver_direct2_t: public netlist_matrix_solver_direct_t<2,2>
{
public:

	netlist_matrix_solver_direct2_t(const netlist_solver_parameters_t &params)
		: netlist_matrix_solver_direct_t<2, 2>(params, 2)
		{}
	ATTR_HOT inline int vsolve_non_dynamic(const bool newton_raphson);
protected:
	ATTR_HOT virtual nl_double vsolve();
private:
};

// ----------------------------------------------------------------------------------------
// netlist_matrix_solver - Direct2
// ----------------------------------------------------------------------------------------

ATTR_HOT nl_double netlist_matrix_solver_direct2_t::vsolve()
{
	solve_base<netlist_matrix_solver_direct2_t>(this);
	return this->compute_next_timestep();
}

ATTR_HOT inline int netlist_matrix_solver_direct2_t::vsolve_non_dynamic(ATTR_UNUSED const bool newton_raphson)
{
	build_LE_A();
	build_LE_RHS();

	const nl_double a = m_A[0][0];
	const nl_double b = m_A[0][1];
	const nl_double c = m_A[1][0];
	const nl_double d = m_A[1][1];

	nl_double new_val[2];
	new_val[1] = (a * m_RHS[1] - c * m_RHS[0]) / (a * d - b * c);
	new_val[0] = (m_RHS[0] - b * new_val[1]) / a;

	if (is_dynamic())
	{
		nl_double err = this->delta(new_val);
		store(new_val, true);
		if (err > m_params.m_accuracy )
			return 2;
		else
			return 1;
	}
	store(new_val, false);
	return 1;
}



#endif /* NLD_MS_DIRECT2_H_ */
