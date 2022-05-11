// license:BSD-3-Clause
// copyright-holders:Couriersud

#ifndef NLD_MATRIX_SOLVER_EXT_H_
#define NLD_MATRIX_SOLVER_EXT_H_

///
/// \file nld_matrix_solver.h
///

#include "nld_matrix_solver.h"

#include <numeric>

namespace netlist::solver
{

	template <typename FT, int SIZE>
	class matrix_solver_ext_t: public matrix_solver_t
	{
	public:

		using float_type = FT;

		matrix_solver_ext_t(devices::nld_solver &main_solver, const pstring &name,
			const net_list_t &nets,
			const solver::solver_parameters_t *params, const std::size_t size)
		: matrix_solver_t(main_solver, name, nets, params)
		, m_new_V(size)
		, m_RHS(size)
		, m_mat_ptr(size, this->max_railstart() + 1)
		, m_last_V(size, nlconst::zero())
		, m_DD_n_m_1(size, nlconst::zero())
		, m_h_n_m_1(size, nlconst::magic(1e-6)) // we need a non zero value here
		, m_dim(size)
		{
			//
			// save states
			//
			state().save(*this, m_last_V.as_base(), this->name(), "m_last_V");
			state().save(*this, m_DD_n_m_1.as_base(), this->name(), "m_DD_n_m_1");
			state().save(*this, m_h_n_m_1.as_base(), this->name(), "m_h_n_m_1");
		}

	protected:
		static constexpr const std::size_t SIZEABS = plib::parray<FT, SIZE>::SIZEABS();
		static constexpr const std::size_t m_pitch_ABS = (((SIZEABS + 0) + 7) / 8) * 8;

		//PALIGNAS_VECTOROPT() parrays define alignment already
		plib::parray<float_type, SIZE> m_new_V;
		//PALIGNAS_VECTOROPT() parrays define alignment already
		plib::parray<float_type, SIZE> m_RHS;

		//PALIGNAS_VECTOROPT() parrays define alignment already
		plib::pmatrix2d<float_type *> m_mat_ptr;

		template <typename T, typename M>
		void log_fill(const T &fill, M &mat)
		{
			const std::size_t iN = fill.size();

			// FIXME: Not yet working, mat_cr.h needs some more work
#if 0
			auto mat_GE = dynamic_cast<plib::pGEmatrix_cr_t<typename M::base> *>(&mat);
#else
			plib::unused_var(mat);
#endif
			std::vector<unsigned> levL(iN, 0);
			std::vector<unsigned> levU(iN, 0);

			// parallel scheme for L x = y
			for (std::size_t k = 0; k < iN; k++)
			{
				unsigned lm=0;
				for (std::size_t j = 0; j<k; j++)
					if (fill[k][j] < M::FILL_INFINITY)
						lm = std::max(lm, levL[j]);
				levL[k] = 1+lm;
			}

			// parallel scheme for U x = y
			for (std::size_t k = iN; k-- > 0; )
			{
				unsigned lm=0;
				for (std::size_t j = iN; --j > k; )
					if (fill[k][j] < M::FILL_INFINITY)
						lm = std::max(lm, levU[j]);
				levU[k] = 1+lm;
			}
			for (std::size_t k = 0; k < iN; k++)
			{
				unsigned fm = 0;
				pstring ml = "";
				for (std::size_t j = 0; j < iN; j++)
				{
					ml += fill[k][j] == 0 ? 'X' : fill[k][j] < M::FILL_INFINITY ? '+' : '.';
					if (fill[k][j] < M::FILL_INFINITY)
						if (fill[k][j] > fm)
							fm = fill[k][j];
				}
#if 0
				this->log().verbose("{1:4} {2} {3:4} {4:4} {5:4} {6:4}", k, ml,
					levL[k], levU[k], mat_GE ? mat_GE->get_parallel_level(k) : 0, fm);
#else
				this->log().verbose("{1:4} {2} {3:4} {4:4} {5:4} {6:4}", k, ml,
					levL[k], levU[k], 0, fm);
#endif
			}
		}

		constexpr std::size_t size() const noexcept
		{
			return (SIZE > 0) ? static_cast<std::size_t>(SIZE) : m_dim;
		}

#if 1
		void store() override
		{
			const std::size_t iN = size();
			for (std::size_t i = 0; i < iN; i++)
				this->m_terms[i].setV(static_cast<fptype>(m_new_V[i]));
		}
#else
		// global tanh damping (4.197)
		// partially cures the symptoms but not the cause
		void store() override
		{
			const std::size_t iN = size();
			for (std::size_t i = 0; i < iN; i++)
			{
				auto oldV = this->m_terms[i].template getV<fptype>();
				this->m_terms[i].setV(oldV + 0.02 * plib::tanh((m_new_V[i]-oldV)*50.0));
			}
		}
#endif
		bool check_err() const override
		{
			// NOTE: Ideally we should also include currents (RHS) here. This would
			// need a reevaluation of the right hand side after voltages have been updated
			// and thus belong into a different calculation. This applies to all solvers.

			const std::size_t iN = size();
			const float_type reltol(static_cast<float_type>(m_params.m_reltol));
			const float_type vntol(static_cast<float_type>(m_params.m_vntol));
			for (std::size_t i = 0; i < iN; i++)
			{
				const float_type vold(static_cast<float_type>(this->m_terms[i].getV()));
				const float_type vnew(m_new_V[i]);
				const float_type tol(vntol + reltol * std::max(plib::abs(vnew),plib::abs(vold)));
				if (plib::abs(vnew - vold) > tol)
					return true;
			}
			return false;
		}

		void backup() override
		{
			const std::size_t iN = size();
			for (std::size_t i = 0; i < iN; i++)
				m_last_V[i] = gsl::narrow_cast<fptype>(this->m_terms[i].getV());
		}

		void restore() override
		{
			const std::size_t iN = size();
			for (std::size_t i = 0; i < iN; i++)
				this->m_terms[i].setV(static_cast<nl_fptype>(m_last_V[i]));
		}

		netlist_time compute_next_timestep(fptype cur_ts, fptype min_ts, fptype max_ts) override
		{
			fptype new_solver_timestep_sq(max_ts * max_ts);

			for (std::size_t k = 0; k < size(); k++)
			{
				const auto &t = m_terms[k];
				const auto v(static_cast<fptype>(t.getV()));
				// avoid floating point exceptions
				const fptype DD_n = std::max(-fp_constants<fptype>::TIMESTEP_MAXDIFF(),
					std::min(+fp_constants<fptype>::TIMESTEP_MAXDIFF(),(v - m_last_V[k])));

				//m_last_V[k] = v;
				const fptype hn = cur_ts;

				fptype DD2 = (DD_n / hn - m_DD_n_m_1[k] / m_h_n_m_1[k]) / (hn + m_h_n_m_1[k]);

				m_h_n_m_1[k] = hn;
				m_DD_n_m_1[k] = DD_n;
				if (plib::abs(DD2) > fp_constants<fptype>::TIMESTEP_MINDIV()) // avoid div-by-zero
				{
					// save the sqrt for the end
					const fptype new_net_timestep_sq = m_params.m_dynamic_lte / plib::abs(nlconst::half()*DD2);
					new_solver_timestep_sq = std::min(new_net_timestep_sq, new_solver_timestep_sq);
				}
			}

			new_solver_timestep_sq = std::max(plib::sqrt(new_solver_timestep_sq), min_ts);

			// FIXME: Factor 2 below is important. Without, we get timing issues. This must be a bug elsewhere.
			return std::max(netlist_time::from_fp(new_solver_timestep_sq), netlist_time::quantum() * 2);
		}

		template <typename M>
		void build_mat_ptr(M &mat)
		{
			const std::size_t iN = size();

			for (std::size_t k=0; k<iN; k++)
			{
				std::size_t cnt(0);
				// build pointers into the compressed row format matrix for each terminal
				for (std::size_t j=0; j< this->m_terms[k].railstart();j++)
				{
					int other = this->m_terms[k].m_connected_net_idx[j];
					if (other >= 0)
					{
						m_mat_ptr[k][j] = &(mat[k][static_cast<std::size_t>(other)]);
						cnt++;
					}
				}
				nl_assert_always(cnt == this->m_terms[k].railstart(), "Count and railstart mismatch");
				m_mat_ptr[k][this->m_terms[k].railstart()] = &(mat[k][k]);
			}
		}

		template <typename M>
		void clear_square_mat(M &m)
		{
			const std::size_t n = size();
			for (std::size_t k=0; k < n; k++)
			{
				auto *p = &(m[k][0]);
				using mat_elem_type = typename std::decay<decltype(*p)>::type;
				for (std::size_t i=0; i < n; i++)
					p[i] = plib::constants<mat_elem_type>::zero();
			}
		}

		void fill_matrix_and_rhs()
		{
			const std::size_t N = size();

			for (std::size_t k = 0; k < N; k++)
			{
				auto &net = m_terms[k];
				auto **tcr_r = &(m_mat_ptr[k][0]);

				using source_type = typename decltype(m_gtn)::value_type;
				const std::size_t term_count = net.count();
				const std::size_t railstart = net.railstart();
				const auto &go = m_gonn[k];
				const auto &gt = m_gtn[k];
				const auto &Idr = m_Idrn[k];
				const auto &cnV = m_connected_net_Vn[k];

				// FIXME: gonn, gtn and Idr - which float types should they have?

				auto gtot_t = std::accumulate(gt, gt + term_count, plib::constants<source_type>::zero());

				// update diagonal element ...
				*tcr_r[railstart] = static_cast<FT>(gtot_t); //mat.A[mat.diag[k]] += gtot_t;

				for (std::size_t i = 0; i < railstart; i++)
					*tcr_r[i]       += static_cast<FT>(go[i]);

				auto RHS_t = std::accumulate(Idr, Idr + term_count, plib::constants<source_type>::zero());

				for (std::size_t i = railstart; i < term_count; i++)
					RHS_t +=  (- go[i]) * *cnV[i];

				m_RHS[k] = static_cast<FT>(RHS_t);
			}
		}

	private:
		// state - variable time_stepping
		//PALIGNAS_VECTOROPT() parrays define alignment already
		plib::parray<fptype, SIZE> m_last_V;
		// PALIGNAS_VECTOROPT() parrays define alignment already
		plib::parray<fptype, SIZE> m_DD_n_m_1;
		// PALIGNAS_VECTOROPT() parrays define alignment already
		plib::parray<fptype, SIZE> m_h_n_m_1;

		const std::size_t m_dim;

	};

} // namespace netlist::solver

#endif // NLD_MATRIX_SOLVER_EXT_H_
