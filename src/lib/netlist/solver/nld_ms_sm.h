// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef NLD_MS_SM_H_
#define NLD_MS_SM_H_

///
/// \file nld_ms_sm.h
///
/// Sherman-Morrison Solver
///
/// Computes the updated inverse of A given that the change in A is
///
/// A <- A + (u x v)   u,v vectors
///
/// In this specific implementation, u is a unit vector specifying the row which
/// changed. Thus v contains the changed column.
///
/// Than z = A^-1 u ,  w = transpose(A^-1) v , lambda = v z
///
/// A^-1 <- 1.0 / (1.0 + lambda) * (z x w)
///
/// The approach is iterative and applied for each row changed.
///
/// The performance for a typical circuit like kidniki compared to Gaussian
/// elimination is poor:
///
/// a) The code needs to be run for each row change.
/// b) The inverse of A typically is fully occupied.
///
/// It may have advantages for circuits with a high number of elements and only
/// few dynamic/active components.
///

#include "nld_matrix_solver.h"
#include "nld_solver.h"
#include "plib/vector_ops.h"

#include <algorithm>

namespace netlist
{
namespace solver
{

	template <typename FT, int SIZE>
	class matrix_solver_sm_t: public matrix_solver_ext_t<FT, SIZE>
	{
		friend class matrix_solver_t;

	public:

		using float_ext_type = FT;
		using float_type = FT;
		// FIXME: dirty hack to make this compile
		static constexpr const std::size_t storage_N = 100;

		matrix_solver_sm_t(netlist_state_t &anetlist, const pstring &name,
			const analog_net_t::list_t &nets,
			const solver_parameters_t *params, const std::size_t size)
		: matrix_solver_ext_t<FT, SIZE>(anetlist, name, nets, params, size)
		, m_cnt(0)
		{
			this->build_mat_ptr(m_A);
		}

		void reset() override { matrix_solver_t::reset(); }

	protected:
		unsigned vsolve_non_dynamic(const bool newton_raphson) override;
		unsigned solve_non_dynamic(const bool newton_raphson);

		void LE_invert();

		template <typename T>
		void LE_compute_x(T & x);


		template <typename T1, typename T2>
		float_ext_type &A(const T1 &r, const T2 &c) { return m_A[r][c]; }
		template <typename T1, typename T2>
		float_ext_type &W(const T1 &r, const T2 &c) { return m_W[r][c]; }
		template <typename T1, typename T2>
		float_ext_type &Ainv(const T1 &r, const T2 &c) { return m_Ainv[r][c]; }
		template <typename T1>
		float_ext_type &RHS(const T1 &r) { return this->m_RHS[r]; }


		template <typename T1, typename T2>
		float_ext_type &lA(const T1 &r, const T2 &c) { return m_lA[r][c]; }
		template <typename T1, typename T2>
		float_ext_type &lAinv(const T1 &r, const T2 &c) { return m_lAinv[r][c]; }

	private:
		template <typename T, std::size_t N, std::size_t M>
		using array2D = std::array<std::array<T, M>, N>;
		static constexpr std::size_t m_pitch  = (((  storage_N) + 7) / 8) * 8;
		array2D<float_ext_type, storage_N, m_pitch> m_A;
		array2D<float_ext_type, storage_N, m_pitch> m_Ainv;
		array2D<float_ext_type, storage_N, m_pitch> m_W;

		array2D<float_ext_type, storage_N, m_pitch> m_lA;
		array2D<float_ext_type, storage_N, m_pitch> m_lAinv;

		//float_ext_type m_RHSx[storage_N];

		std::size_t m_cnt;

	};

	// ----------------------------------------------------------------------------------------
	// matrix_solver_direct
	// ----------------------------------------------------------------------------------------

	template <typename FT, int SIZE>
	void matrix_solver_sm_t<FT, SIZE>::LE_invert()
	{
		const std::size_t kN = this->size();

		for (std::size_t i = 0; i < kN; i++)
		{
			for (std::size_t j = 0; j < kN; j++)
			{
				W(i,j) = lA(i,j) = A(i,j);
				Ainv(i,j) = plib::constants<FT>::zero();
			}
			Ainv(i,i) = plib::constants<FT>::one();
		}
		// down
		for (std::size_t i = 0; i < kN; i++)
		{
			// FIXME: Singular matrix?
			const float_type f = plib::reciprocal(W(i,i));
			const auto * const p = this->m_terms[i].m_nzrd.data();
			const std::size_t e = this->m_terms[i].m_nzrd.size();

			// Eliminate column i from row j

			const auto * const pb = this->m_terms[i].m_nzbd.data();
			const std::size_t eb = this->m_terms[i].m_nzbd.size();
			for (std::size_t jb = 0; jb < eb; jb++)
			{
				const unsigned j = pb[jb];
				const float_type f1 = - W(j,i) * f;
				// FIXME: comparison to zero
				if (f1 != plib::constants<float_type>::zero())
				{
					for (std::size_t k = 0; k < e; k++)
						W(j,p[k]) += W(i,p[k]) * f1;
					for (std::size_t k = 0; k <= i; k ++)
						Ainv(j,k) += Ainv(i,k) * f1;
				}
			}
		}
		// up
		for (std::size_t i = kN; i-- > 0; )
		{
			// FIXME: Singular matrix?
			const float_type f = plib::reciprocal(W(i,i));
			for (std::size_t j = i; j-- > 0; )
			{
				const float_type f1 = - W(j,i) * f;
				// FIXME: comparison to zero
				if (f1 != plib::constants<float_type>::zero())
				{
					for (std::size_t k = i; k < kN; k++)
						W(j,k) += W(i,k) * f1;
					for (std::size_t k = 0; k < kN; k++)
						Ainv(j,k) += Ainv(i,k) * f1;
				}
			}
			for (std::size_t k = 0; k < kN; k++)
			{
				Ainv(i,k) *= f;
				lAinv(i,k) = Ainv(i,k);
			}
		}
	}

	template <typename FT, int SIZE>
	template <typename T>
	void matrix_solver_sm_t<FT, SIZE>::LE_compute_x(
			T & x)
	{
		const std::size_t kN = this->size();

		for (std::size_t i=0; i<kN; i++)
			x[i] = plib::constants<FT>::zero();

		for (std::size_t k=0; k<kN; k++)
		{
			const float_type f = RHS(k);

			for (std::size_t i=0; i<kN; i++)
				x[i] += Ainv(i,k) * f;
		}
	}

	template <typename FT, int SIZE>
	unsigned matrix_solver_sm_t<FT, SIZE>::solve_non_dynamic(const bool newton_raphson)
	{
		static constexpr const bool incremental = true;
		const std::size_t iN = this->size();

		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
		std::array<float_type, m_pitch> v;
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
		std::array<std::size_t, m_pitch> cols;
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
		std::array<float_type, m_pitch> z;

		if ((m_cnt % 50) == 0)
		{
			// complete calculation
			this->LE_invert();
		}
		else
		{
			if (!incremental)
			{
				for (std::size_t row = 0; row < iN; row ++)
					for (std::size_t k = 0; k < iN; k++)
						Ainv(row,k) = lAinv(row, k);
			}
			for (std::size_t row = 0; row < iN; row ++)
			{
				std::size_t colcount = 0;

				auto &nz = this->m_terms[row].m_nz;
				for (unsigned & col : nz)
				{
					v[col] = A(row,col) - lA(row,col);
					if (incremental)
						lA(row,col) = A(row,col);
					// FIXME: comparison to zero
					if (v[col] != plib::constants<float_type>::zero())
						cols[colcount++] = col;
				}

				if (colcount > 0)
				{
					auto lamba(plib::constants<FT>::zero());
					std::array<float_type, m_pitch> w = {0};

					// compute w and lamba
					for (std::size_t i = 0; i < iN; i++)
						z[i] = Ainv(i, row); // u is row'th column

					for (std::size_t j = 0; j < colcount; j++)
						lamba += v[cols[j]] * z[cols[j]];

					for (std::size_t j=0; j<colcount; j++)
					{
						std::size_t col = cols[j];
						float_type f = v[col];
						for (std::size_t k = 0; k < iN; k++)
							w[k] += Ainv(col,k) * f; // Transpose(Ainv) * v
					}

					lamba = -plib::reciprocal(plib::constants<float_type>::one() + lamba);
					for (std::size_t i=0; i<iN; i++)
					{
						const float_type f = lamba * z[i];
						// FIXME: comparison to zero
						if (f != plib::constants<float_type>::zero())
							for (std::size_t k = 0; k < iN; k++)
								Ainv(i,k) += f * w[k];
					}
				}

			}
		}

		m_cnt++;

		this->LE_compute_x(this->m_new_V);

		bool err(false);
		if (newton_raphson)
			err = this->check_err();
		this->store();
		return (err) ? 2 : 1;
	}

	template <typename FT, int SIZE>
	unsigned matrix_solver_sm_t<FT, SIZE>::vsolve_non_dynamic(const bool newton_raphson)
	{

		this->clear_square_mat(this->m_A);
		this->fill_matrix_and_rhs();

		this->m_stat_calculations++;
		return this->solve_non_dynamic(newton_raphson);
	}


} // namespace solver
} // namespace netlist

#endif // NLD_MS_SM_H_
