// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_ms_sor.h
 *
 * Generic successive over relaxation solver.
 *
 * Fow w==1 we will do the classic Gauss-Seidel approach
 *
 */

#ifndef NLD_MS_SOR_MAT_H_
#define NLD_MS_SOR_MAT_H_

#include <algorithm>

#include "nld_matrix_solver.h"
#include "nld_ms_direct.h"
#include "nld_solver.h"

namespace netlist
{
	namespace devices
	{

template <typename FT, int SIZE>
class matrix_solver_SOR_mat_t: public matrix_solver_direct_t<FT, SIZE>
{
	friend class matrix_solver_t;

public:

	using float_type = FT;

	matrix_solver_SOR_mat_t(netlist_state_t &anetlist, const pstring &name, const solver_parameters_t *params, std::size_t size)
		: matrix_solver_direct_t<FT, SIZE>(anetlist, name, matrix_solver_t::ASCENDING, params, size)
		, m_Vdelta(*this, "m_Vdelta", std::vector<float_type>(size))
		, m_omega(*this, "m_omega", params->m_gs_sor)
		, m_lp_fact(*this, "m_lp_fact", 0)
		{
		}

	void vsetup(analog_net_t::list_t &nets) override;

	unsigned vsolve_non_dynamic(const bool newton_raphson) override;

private:
	//state_var<float_type[storage_N]> m_Vdelta;
	state_var<std::vector<float_type>> m_Vdelta;

	state_var<float_type> m_omega;
	state_var<float_type> m_lp_fact;

};

// ----------------------------------------------------------------------------------------
// matrix_solver - Gauss - Seidel
// ----------------------------------------------------------------------------------------

template <typename FT, int SIZE>
void matrix_solver_SOR_mat_t<FT, SIZE>::vsetup(analog_net_t::list_t &nets)
{
	matrix_solver_direct_t<FT, SIZE>::vsetup(nets);
}

#if 0
//FIXME: move to solve_base
template <unsigned m_N, unsigned storage_N>
float_type matrix_solver_SOR_mat_t<m_N, storage_N>::vsolve()
{
	/*
	 * enable linear prediction on first newton pass
	 */

	if (USE_LINEAR_PREDICTION)
		for (unsigned k = 0; k < this->size(); k++)
		{
			this->m_last_V[k] = this->m_nets[k]->m_cur_Analog;
			this->m_nets[k]->m_cur_Analog = this->m_nets[k]->m_cur_Analog + this->m_Vdelta[k] * this->current_timestep() * m_lp_fact;
		}
	else
		for (unsigned k = 0; k < this->size(); k++)
		{
			this->m_last_V[k] = this->m_nets[k]->m_cur_Analog;
		}

	this->solve_base(this);

	if (USE_LINEAR_PREDICTION)
	{
		float_type sq = 0;
		float_type sqo = 0;
		const float_type rez_cts = 1.0 / this->current_timestep();
		for (unsigned k = 0; k < this->size(); k++)
		{
			const analog_net_t *n = this->m_nets[k];
			const float_type nv = (n->Q_Analog() - this->m_last_V[k]) * rez_cts ;
			sq += nv * nv;
			sqo += this->m_Vdelta[k] * this->m_Vdelta[k];
			this->m_Vdelta[k] = nv;
		}

		// FIXME: used to be 1e90, but this would not be compatible with float
		if (sqo > NL_FCONST(1e-20))
			m_lp_fact = std::min(std::sqrt(sq/sqo), (float_type) 2.0);
		else
			m_lp_fact = NL_FCONST(0.0);
	}


	return this->compute_next_timestep();
}
#endif

template <typename FT, int SIZE>
unsigned matrix_solver_SOR_mat_t<FT, SIZE>::vsolve_non_dynamic(const bool newton_raphson)
{
	/* The matrix based code looks a lot nicer but actually is 30% slower than
	 * the optimized code which works directly on the data structures.
	 * Need something like that for gaussian elimination as well.
	 */


	const std::size_t iN = this->size();

	this->build_LE_A(*this);
	this->build_LE_RHS(*this);

	bool resched = false;

	unsigned resched_cnt = 0;


#if 0
	static int ws_cnt = 0;
	ws_cnt++;
	if (1 && ws_cnt % 200 == 0)
	{
		// update omega
		float_type lambdaN = 0;
		float_type lambda1 = 1e9;
		for (int k = 0; k < iN; k++)
		{
	#if 0
			float_type akk = std::abs(this->m_A[k][k]);
			if ( akk > lambdaN)
				lambdaN = akk;
			if (akk < lambda1)
				lambda1 = akk;
	#else
			float_type akk = std::abs(this->m_A[k][k]);
			float_type s = 0.0;
			for (int i=0; i<iN; i++)
				s = s + std::abs(this->m_A[k][i]);
			akk = s / akk - 1.0;
			if ( akk > lambdaN)
				lambdaN = akk;
			if (akk < lambda1)
				lambda1 = akk;
	#endif
		}

		//ws = 2.0 / (2.0 - lambdaN - lambda1);
		m_omega = 2.0 / (2.0 - lambda1);
	}
#endif

	for (std::size_t k = 0; k < iN; k++)
		this->m_new_V[k] = this->m_nets[k]->Q_Analog();

	do {
		resched = false;
		float_type cerr = 0.0;

		for (std::size_t k = 0; k < iN; k++)
		{
			float_type Idrive = 0;

			const auto *p = this->m_terms[k]->m_nz.data();
			const std::size_t e = this->m_terms[k]->m_nz.size();

			for (std::size_t i = 0; i < e; i++)
				Idrive = Idrive + this->A(k,p[i]) * this->m_new_V[p[i]];

			FT w = m_omega / this->A(k,k);
			if (USE_GABS)
			{
				FT gabs_t = 0.0;
				for (std::size_t i = 0; i < e; i++)
					if (p[i] != k)
						gabs_t = gabs_t + std::abs(this->A(k,p[i]));

				gabs_t *= plib::constants<FT>::one(); // derived by try and error
				if (gabs_t > this->A(k,k))
				{
					w = plib::constants<FT>::one() / (this->A(k,k) + gabs_t);
				}
			}

			const float_type delta = w * (this->RHS(k) - Idrive) ;
			cerr = std::max(cerr, std::abs(delta));
			this->m_new_V[k] += delta;
		}

		if (cerr > this->m_params.m_accuracy)
		{
			resched = true;
		}
		resched_cnt++;
	} while (resched && (resched_cnt < this->m_params.m_gs_loops));

	this->m_stat_calculations++;
	this->m_iterative_total += resched_cnt;

	if (resched)
	{
		this->m_iterative_fail++;
		//this->netlist().warning("Falling back to direct solver .. Consider increasing RESCHED_LOOPS");
		return matrix_solver_direct_t<FT, SIZE>::solve_non_dynamic(newton_raphson);
	}

	const float_type err = (newton_raphson ? this->delta(this->m_new_V) : 0.0);
	this->store(this->m_new_V);
	return (err > this->m_params.m_accuracy) ? 2 : 1;

}

	} //namespace devices
} // namespace netlist

#endif /* NLD_MS_GAUSS_SEIDEL_H_ */
