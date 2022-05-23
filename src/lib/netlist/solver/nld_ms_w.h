// license:BSD-3-Clause
// copyright-holders:Couriersud

#ifndef NLD_MS_W_H_
#define NLD_MS_W_H_

// Names
// spell-checker: words Woodbury, Raphson,
//
// Specific technical terms
// spell-checker: words Cgso, Cgdo, Cgbo, Cjsw, Mjsw, Ucrit, Uexp, Utra, Neff, Tnom, capval, Udsat, Utst


///
/// \file nld_ms_direct.h
///
/// Woodbury Solver
///
/// Computes the updated solution of A given that the change in A is
///
/// A <- A + (U x transpose(V))   U,V matrices
///
/// The approach is describes in "Numerical Recipes in C", Second edition, Page 75ff
///
/// Whilst the book proposes to invert the matrix R=(I+transpose(V)*Z) we define
///
///       w = transpose(V)*y
///       a = R^-1 * w
///
/// and consequently
///
///       R * a = w
///
/// And solve for a using Gaussian elimination. This is a lot faster.
///
/// One fact omitted in the book is the fact that actually the matrix Z which contains
/// in it's columns the solutions of
///
///      A * zk = uk
///
/// for uk being unit vectors for full rank (max(k) == n) is identical to the
/// inverse of A.
///
/// The approach performs relatively well for matrices up to n ~ 40 (`kidniki` using frontiers).
/// `Kidniki` without frontiers has n==88. Here, the average number of Newton-Raphson
/// loops increase to 20. It looks like that the approach for larger matrices
/// introduces numerical instability.
///

#include "nld_matrix_solver_ext.h"
#include "plib/vector_ops.h"

#include <algorithm>

namespace netlist::solver
{

	template <typename FT, int SIZE>
	class matrix_solver_w_t: public matrix_solver_ext_t<FT, SIZE>
	{
	public:
		using float_ext_type = FT;
		using float_type = FT;

		// FIXME: dirty hack to make this compile
		static constexpr const std::size_t storage_N = 100;

		matrix_solver_w_t(devices::nld_solver &main_solver, const pstring &name,
			const matrix_solver_t::net_list_t &nets,
			const solver_parameters_t *params, const std::size_t size)
		: matrix_solver_ext_t<FT, SIZE>(main_solver, name, nets, params, size)
		, m_cnt(0)
		{
			this->build_mat_ptr(m_A);
		}

		void reset() override { matrix_solver_t::reset(); }

	protected:
		void vsolve_non_dynamic() override;

		void LE_invert();

		template <typename T>
		void LE_compute_x(T & x);

		template <typename T1, typename T2>
		float_ext_type &A(const T1 &r, const T2 &c) { return m_A[r][c]; }
		template <typename T1, typename T2>
		float_ext_type &W(const T1 &r, const T2 &c) { return m_W[r][c]; }

		// access to the inverted matrix for fixed columns over row, values stored transposed
		template <typename T1, typename T2>
		float_ext_type &Ainv(const T1 &r, const T2 &c) { return m_Ainv[c][r]; }
		template <typename T1>
		float_ext_type &RHS(const T1 &r) { return this->m_RHS[r]; }


		template <typename T1, typename T2>
		float_ext_type &lA(const T1 &r, const T2 &c) { return m_lA[r][c]; }


	private:
		void solve_non_dynamic();

		template <typename T, std::size_t N, std::size_t M>
		using array2D = std::array<std::array<T, M>, N>;
		static constexpr std::size_t m_pitch  = (((  storage_N) + 7) / 8) * 8;
		array2D<float_ext_type, storage_N, m_pitch> m_A;
		array2D<float_ext_type, storage_N, m_pitch> m_Ainv;
		array2D<float_ext_type, storage_N, m_pitch> m_W;

		array2D<float_ext_type, storage_N, m_pitch> m_lA;

		// temporary
		array2D<float_ext_type, storage_N, m_pitch> H;
		std::array<unsigned, storage_N> rows;
		array2D<unsigned, storage_N, m_pitch> cols;
		std::array<unsigned, storage_N> col_count;

		unsigned m_cnt;
	};

	// ----------------------------------------------------------------------------------------
	// matrix_solver_direct
	// ----------------------------------------------------------------------------------------

	template <typename FT, int SIZE>
	void matrix_solver_w_t<FT, SIZE>::LE_invert()
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
			const size_t e = this->m_terms[i].m_nzrd.size();

			// Eliminate column i from row j

			const auto * const pb = this->m_terms[i].m_nzbd.data();
			const size_t eb = this->m_terms[i].m_nzbd.size();
			for (std::size_t jb = 0; jb < eb; jb++)
			{
				const auto j = pb[jb];
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
			}
		}
	}

	template <typename FT, int SIZE>
	template <typename T>
	void matrix_solver_w_t<FT, SIZE>::LE_compute_x(
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
	void matrix_solver_w_t<FT, SIZE>::solve_non_dynamic()
	{
		const auto iN = this->size();

		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
		std::array<float_type, storage_N> t;  // FIXME: convert to member
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
		std::array<float_type, storage_N> w;

		if ((m_cnt % 50) == 0)
		{
			// complete calculation
			this->LE_invert();
			this->LE_compute_x(this->m_new_V);
		}
		else
		{
			// Solve Ay = b for y
			this->LE_compute_x(this->m_new_V);

			// determine changed rows

			unsigned row_count=0;
			#define VT(r,c) (A(r,c) - lA(r,c))

			for (unsigned row = 0; row < iN; row ++)
			{
				unsigned cc=0;
				auto &nz = this->m_terms[row].m_nz;
				for (auto & col : nz)
				{
					if (A(row,col) != lA(row,col))
						cols[row_count][cc++] = col;
				}
				if (cc > 0)
				{
					col_count[row_count] = cc;
					rows[row_count++] = row;
				}
			}
			if (row_count > 0)
			{
				// construct w = transform(V) * y
				// dim: row_count x iN
				//
				for (unsigned i = 0; i < row_count; i++)
				{
					const unsigned r = rows[i];
					FT tmp = plib::constants<FT>::zero();
					for (unsigned k = 0; k < iN; k++)
						tmp += VT(r,k) * this->m_new_V[k];
					w[i] = tmp;
				}

				for (unsigned i = 0; i < row_count; i++)
					for (unsigned k=0; k< row_count; k++)
						H[i][k] = plib::constants<FT>::zero();

				for (unsigned i = 0; i < row_count; i++)
					H[i][i] = plib::constants<FT>::one();
				// Construct H = (I + VT*Z)
				for (unsigned i = 0; i < row_count; i++)
					for (unsigned k=0; k< col_count[i]; k++)
					{
						const unsigned col = cols[i][k];
						float_type f = VT(rows[i],col);
						// FIXME: comparison to zero
						if (f != plib::constants<float_type>::zero())
							for (unsigned j= 0; j < row_count; j++)
								H[i][j] += f * Ainv(col,rows[j]);
					}

				// Gaussian elimination of H
				for (unsigned i = 0; i < row_count; i++)
				{
					// FIXME: comparison to zero
					if (H[i][i] == plib::constants<float_type>::zero())
						plib::perrlogger("{} H singular\n", this->name());
					const float_type f = plib::reciprocal(H[i][i]);
					for (unsigned j = i+1; j < row_count; j++)
					{
						const float_type f1 = - f * H[j][i];

						// FIXME: comparison to zero
						if (f1 != plib::constants<float_type>::zero())
						{
							float_type *pj = &H[j][i+1];
							const float_type *pi = &H[i][i+1];
							for (unsigned k = 0; k < row_count-i-1; k++)
								pj[k] += f1 * pi[k];
								//H[j][k] += f1 * H[i][k];
							w[j] += f1 * w[i];
						}
					}
				}
				// Back substitution
				//inv(H) w = t     w = H t
				for (unsigned j = row_count; j-- > 0; )
				{
					float_type tmp = 0;
					const float_type *pj = &H[j][j+1];
					const float_type *tj = &t[j+1];
					for (unsigned k = 0; k < row_count-j-1; k++)
						tmp += pj[k] * tj[k];
						//tmp += H[j][k] * t[k];
					t[j] = (w[j] - tmp) / H[j][j];
				}

				// x = y - Zt
				for (unsigned i=0; i<iN; i++)
				{
					float_type tmp = plib::constants<FT>::zero();
					for (unsigned j=0; j<row_count;j++)
					{
						const unsigned row = rows[j];
						tmp += Ainv(i,row) * t[j];
					}
					this->m_new_V[i] -= tmp;
				}
			}
		}
		m_cnt++;

		if (false) // NOLINT
			for (unsigned i=0; i<iN; i++)
			{
				float_type tmp = plib::constants<FT>::zero();
				for (unsigned j=0; j<iN; j++)
				{
					tmp += A(i,j) * this->m_new_V[j];
				}
				if (plib::abs(tmp-RHS(i)) > static_cast<float_type>(1e-6))
					plib::perrlogger("{} failed on row {}: {} RHS: {}\n", this->name(), i, plib::abs(tmp-RHS(i)), RHS(i));
			}
	}

	template <typename FT, int SIZE>
	void matrix_solver_w_t<FT, SIZE>::vsolve_non_dynamic()
	{
		this->clear_square_mat(this->m_A);
		this->fill_matrix_and_rhs();

		this->solve_non_dynamic();
	}

} // namespace netlist::solver

#endif // NLD_MS_DIRECT_H_
