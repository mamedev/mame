// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_ms_direct1.h
 *
 */

#ifndef NLD_MS_DIRECT1_H_
#define NLD_MS_DIRECT1_H_

#include "solver/nld_ms_direct.h"
#include "solver/nld_solver.h"

NETLIB_NAMESPACE_DEVICES_START()

class matrix_solver_direct1_t: public matrix_solver_direct_t<1,1>
{
public:

	matrix_solver_direct1_t(const solver_parameters_t *params)
		: matrix_solver_direct_t<1, 1>(params, 1)
		{}
	ATTR_HOT inline int vsolve_non_dynamic(const bool newton_raphson);
protected:
	ATTR_HOT virtual nl_double vsolve();
private:
};

// ----------------------------------------------------------------------------------------
// matrix_solver - Direct1
// ----------------------------------------------------------------------------------------

ATTR_HOT nl_double matrix_solver_direct1_t::vsolve()
{
	solve_base<matrix_solver_direct1_t>(this);
	return this->compute_next_timestep();
}

ATTR_HOT inline int matrix_solver_direct1_t::vsolve_non_dynamic(ATTR_UNUSED const bool newton_raphson)
{
	analog_net_t *net = m_nets[0];
	this->build_LE_A();
	this->build_LE_RHS(m_RHS);
	//NL_VERBOSE_OUT(("{1} {2}\n", new_val, m_RHS[0] / m_A[0][0]);

	nl_double new_val =  m_RHS[0] / A(0,0);

	nl_double e = (new_val - net->m_cur_Analog);
	nl_double cerr = nl_math::abs(e);

	net->m_cur_Analog = new_val;

	if (is_dynamic() && (cerr  > m_params.m_accuracy))
	{
		return 2;
	}
	else
		return 1;

}

NETLIB_NAMESPACE_DEVICES_END()


#endif /* NLD_MS_DIRECT1_H_ */
