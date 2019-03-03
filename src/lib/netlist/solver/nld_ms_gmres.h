// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_ms_gmres.h
 *
 */

#ifndef NLD_MS_GMRES_H_
#define NLD_MS_GMRES_H_

#include "plib/gmres.h"
#include "plib/mat_cr.h"
#include "plib/parray.h"
#include "plib/vector_ops.h"
#include "nld_ms_direct.h"
#include "nld_solver.h"

#include <algorithm>
#include <cmath>


namespace netlist
{
namespace devices
{

	template <typename FT, int SIZE>
	class matrix_solver_GMRES_t: public matrix_solver_direct_t<FT, SIZE>
	{
	public:

		using float_type = FT;

		/* Sort rows in ascending order. This should minimize fill-in and thus
		 * maximize the efficiency of the incomplete LUT.
		 * This is already preconditioning.
		 */
		matrix_solver_GMRES_t(netlist_state_t &anetlist, const pstring &name, const solver_parameters_t *params, const std::size_t size)
			: matrix_solver_direct_t<FT, SIZE>(anetlist, name, matrix_solver_t::PREFER_BAND_MATRIX, params, size)
			, m_term_cr(size)
			//, m_ops(size, 2)
			, m_ops(size, 4)
			, m_gmres(size)
			{
			}

		void vsetup(analog_net_t::list_t &nets) override;
		unsigned vsolve_non_dynamic(const bool newton_raphson) override;

	private:

		using mattype = typename plib::matrix_compressed_rows_t<FT, SIZE>::index_type;

		plib::parray<plib::aligned_vector<FT *, PALIGN_VECTOROPT>, SIZE> m_term_cr;
		plib::mat_precondition_ILU<FT, SIZE> m_ops;
		//plib::mat_precondition_diag<FT, SIZE> m_ops;
		plib::gmres_t<FT, SIZE> m_gmres;
	};

	// ----------------------------------------------------------------------------------------
	// matrix_solver - GMRES
	// ----------------------------------------------------------------------------------------

	template <typename FT, int SIZE>
	void matrix_solver_GMRES_t<FT, SIZE>::vsetup(analog_net_t::list_t &nets)
	{
		matrix_solver_direct_t<FT, SIZE>::vsetup(nets);

		const std::size_t iN = this->size();

		std::vector<std::vector<unsigned>> fill(iN);

		for (std::size_t k=0; k<iN; k++)
		{
			fill[k].resize(iN, decltype(m_ops.m_mat)::FILL_INFINITY);
			terms_for_net_t * row = this->m_terms[k].get();
			for (const auto &nz_j : row->m_nz)
			{
				fill[k][static_cast<mattype>(nz_j)] = 0;
			}
		}

		m_ops.build(fill);

		/* build pointers into the compressed row format matrix for each terminal */

		for (std::size_t k=0; k<iN; k++)
		{
			for (std::size_t j=0; j< this->m_terms[k]->m_railstart;j++)
			{
				for (std::size_t i = m_ops.m_mat.row_idx[k]; i<m_ops.m_mat.row_idx[k+1]; i++)
					if (this->m_terms[k]->connected_net_idx()[j] == static_cast<int>(m_ops.m_mat.col_idx[i]))
					{
						m_term_cr[k].push_back(&m_ops.m_mat.A[i]);
						break;
					}
			}
			nl_assert(m_term_cr[k].size() == this->m_terms[k]->m_railstart);
			m_term_cr[k].push_back(&m_ops.m_mat.A[m_ops.m_mat.diag[k]]);
		}
	}

	template <typename FT, int SIZE>
	unsigned matrix_solver_GMRES_t<FT, SIZE>::vsolve_non_dynamic(const bool newton_raphson)
	{
		const std::size_t iN = this->size();

		plib::parray<FT, SIZE> RHS(iN);
		//float_type new_V[storage_N];

		m_ops.m_mat.set_scalar(0.0);

		/* populate matrix and V for first estimate */
		for (std::size_t k = 0; k < iN; k++)
		{
			this->m_terms[k]->fill_matrix(m_term_cr[k], RHS[k]);
			this->m_new_V[k] = this->m_nets[k]->Q_Analog();
		}

		const float_type accuracy = this->m_params.m_accuracy;

		auto iter = std::max(plib::constants<std::size_t>::one(), this->m_params.m_gs_loops);
		auto gsl = m_gmres.solve(m_ops, this->m_new_V, RHS, iter, accuracy);

		this->m_iterative_total += gsl;
		this->m_stat_calculations++;

		if (gsl > iter)
		{
			this->m_iterative_fail++;
			return matrix_solver_direct_t<FT, SIZE>::vsolve_non_dynamic(newton_raphson);
		}

		const float_type err = (newton_raphson ? this->delta(this->m_new_V) : 0.0);
		this->store(this->m_new_V);
		return (err > this->m_params.m_accuracy) ? 2 : 1;
	}




} // namespace devices
} // namespace netlist

#endif /* NLD_MS_GMRES_H_ */
