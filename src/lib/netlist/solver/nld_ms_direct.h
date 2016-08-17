// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_ms_direct.h
 *
 */

#ifndef NLD_MS_DIRECT_H_
#define NLD_MS_DIRECT_H_

#include <algorithm>

#include "solver/nld_solver.h"
#include "solver/nld_matrix_solver.h"
#include "solver/vector_base.h"

/* Disabling dynamic allocation gives a ~10% boost in performance
 * This flag has been added to support continuous storage for arrays
 * going forward in case we implement cuda solvers in the future.
 */
#define NL_USE_DYNAMIC_ALLOCATION (0)
#define TEST_PARALLEL (0)

#if TEST_PARALLEL
#include <thread>
#include <atomic>
#endif

namespace netlist
{
	namespace devices
	{
//#define nl_ext_double _float128 // slow, very slow
//#define nl_ext_double long double // slightly slower
#define nl_ext_double nl_double

#if TEST_PARALLEL
#define MAXTHR 10
static const int num_thr = 1;

struct thr_intf
{
	virtual void do_work(const int id, void *param) = 0;
};

struct ti_t
{
	volatile std::atomic<int> lo;
	thr_intf *intf;
	void *params;
//  int block[29]; /* make it 256 bytes */
};

static ti_t ti[MAXTHR];
static std::thread thr[MAXTHR];

int thr_init = 0;

static void thr_process_proc(int id)
{
	while (true)
	{
		while (ti[id].lo.load() == 0)
			;
		if (ti[id].lo.load() == 2)
			return;
		ti[id].intf->do_work(id, ti[id].params);
		ti[id].lo.store(0);
	}
}

static void thr_process(int id, thr_intf *intf, void *params)
{
	ti[id].intf = intf;
	ti[id].params = params;
	ti[id].lo.store(1);
}

static void thr_wait()
{
	int c=1;
	while (c > 0)
	{
		c=0;
		for (int i=0; i<num_thr; i++)
			c += ti[i].lo.load();
	}
}

static void thr_initialize()
{
	thr_init++;
	if (thr_init == 1)
	{
		for (int i=0; i<num_thr; i++)
		{
			ti[i].lo = 0;
			thr[i] = std::thread(thr_process_proc, i);
		}
	}
}

static void thr_dispose()
{
	thr_init--;
	if (thr_init == 0)
	{
		for (int i=0; i<num_thr; i++)
			ti[i].lo = 2;
		for (int i=0; i<num_thr; i++)
			thr[i].join();
	}
}
#endif

template <unsigned m_N, unsigned storage_N>
#if TEST_PARALLEL
class matrix_solver_direct_t: public matrix_solver_t, public thr_intf
#else
class matrix_solver_direct_t: public matrix_solver_t
#endif
{
	friend class matrix_solver_t;
public:

	matrix_solver_direct_t(netlist_t &anetlist, const pstring &name, const solver_parameters_t *params, const unsigned size);
	matrix_solver_direct_t(netlist_t &anetlist, const pstring &name, const eSortType sort, const solver_parameters_t *params, const unsigned size);

	virtual ~matrix_solver_direct_t();

	virtual void vsetup(analog_net_t::list_t &nets) override;
	virtual void reset() override { matrix_solver_t::reset(); }

protected:
	virtual unsigned vsolve_non_dynamic(const bool newton_raphson) override;
	unsigned solve_non_dynamic(const bool newton_raphson);

	inline unsigned N() const { if (m_N == 0) return m_dim; else return m_N; }

	void LE_solve();

	template <typename T>
	void LE_back_subst(T * RESTRICT x);

#if TEST_PARALLEL
	int x_i[10];
	int x_start[10];
	int x_stop[10];
	virtual void do_work(const int id, void *param) override;
#endif

#if (NL_USE_DYNAMIC_ALLOCATION)
	template <typename T1, typename T2>
	inline nl_ext_double &A(const T1 &r, const T2 &c) { return m_A[r * m_pitch + c]; }
	template <typename T1>
	inline nl_ext_double &RHS(const T1 &r) { return m_A[r * m_pitch + N()]; }
#else
	template <typename T1, typename T2>
	inline nl_ext_double &A(const T1 &r, const T2 &c) { return m_A[r][c]; }
	template <typename T1>
	inline nl_ext_double &RHS(const T1 &r) { return m_A[r][N()]; }
#endif
	nl_double m_last_RHS[storage_N]; // right hand side - contains currents

private:
	static const std::size_t m_pitch = (((storage_N + 1) + 7) / 8) * 8;
	//static const std::size_t m_pitch = (((storage_N + 1) + 15) / 16) * 16;
	//static const std::size_t m_pitch = (((storage_N + 1) + 31) / 32) * 32;
#if (NL_USE_DYNAMIC_ALLOCATION)
	nl_ext_double * RESTRICT m_A;
#else
	nl_ext_double m_A[storage_N][m_pitch];
#endif
	//nl_ext_double m_RHSx[storage_N];

	const unsigned m_dim;

};

// ----------------------------------------------------------------------------------------
// matrix_solver_direct
// ----------------------------------------------------------------------------------------

template <unsigned m_N, unsigned storage_N>
matrix_solver_direct_t<m_N, storage_N>::~matrix_solver_direct_t()
{
#if (NL_USE_DYNAMIC_ALLOCATION)
	pfree_array(m_A);
#endif
#if TEST_PARALLEL
	thr_dispose();
#endif
}

template <unsigned m_N, unsigned storage_N>
void matrix_solver_direct_t<m_N, storage_N>::vsetup(analog_net_t::list_t &nets)
{
	if (m_dim < nets.size())
		log().fatal("Dimension {1} less than {2}", m_dim, nets.size());

	matrix_solver_t::setup_base(nets);

	/* add RHS element */
	for (unsigned k = 0; k < N(); k++)
	{
		terms_t * t = m_terms[k];

		if (!plib::container::contains(t->m_nzrd, N()))
			t->m_nzrd.push_back(N());
	}

	netlist().save(*this, m_last_RHS, "m_last_RHS");

	for (unsigned k = 0; k < N(); k++)
		netlist().save(*this, RHS(k), plib::pfmt("RHS.{1}")(k));
}



#if TEST_PARALLEL
template <unsigned m_N, unsigned storage_N>
void matrix_solver_direct_t<m_N, storage_N>::do_work(const int id, void *param)
{
	const int i = x_i[id];
	/* FIXME: Singular matrix? */
	const nl_double f = 1.0 / A(i,i);
	const unsigned * RESTRICT const p = m_terms[i]->m_nzrd.data();
	const unsigned e = m_terms[i]->m_nzrd.size();

	/* Eliminate column i from row j */

	const unsigned * RESTRICT const pb = m_terms[i]->m_nzbd.data();
	const unsigned sj = x_start[id];
	const unsigned se = x_stop[id];
	for (unsigned jb = sj; jb < se; jb++)
	{
		const unsigned j = pb[jb];
		const nl_double f1 = - A(j,i) * f;
		for (unsigned k = 0; k < e; k++)
			A(j,p[k]) += A(i,p[k]) * f1;
	}
}
#endif

template <unsigned m_N, unsigned storage_N>
void matrix_solver_direct_t<m_N, storage_N>::LE_solve()
{
	const unsigned kN = N();

	for (unsigned i = 0; i < kN; i++) {
		// FIXME: use a parameter to enable pivoting? m_pivot
		if (!TEST_PARALLEL && m_params.m_pivot)
		{
			/* Find the row with the largest first value */
			unsigned maxrow = i;
			for (unsigned j = i + 1; j < kN; j++)
			{
				//if (std::abs(m_A[j][i]) > std::abs(m_A[maxrow][i]))
				if (A(j,i) * A(j,i) > A(maxrow,i) * A(maxrow,i))
					maxrow = j;
			}

			if (maxrow != i)
			{
				/* Swap the maxrow and ith row */
				for (unsigned k = 0; k < kN + 1; k++) {
					std::swap(A(i,k), A(maxrow,k));
				}
				//std::swap(RHS(i), RHS(maxrow));
			}
			/* FIXME: Singular matrix? */
			const nl_double f = 1.0 / A(i,i);

			/* Eliminate column i from row j */

			for (unsigned j = i + 1; j < kN; j++)
			{
				const nl_double f1 = - A(j,i) * f;
				if (f1 != NL_FCONST(0.0))
				{
					const nl_double * RESTRICT pi = &A(i,i+1);
					nl_double * RESTRICT pj = &A(j,i+1);
#if 1
					vec_add_mult_scalar(kN-i,pi,f1,pj);
#else
					vec_add_mult_scalar(kN-i-1,pj,f1,pi);
					//for (unsigned k = i+1; k < kN; k++)
					//  pj[k] = pj[k] + pi[k] * f1;
					//for (unsigned k = i+1; k < kN; k++)
						//A(j,k) += A(i,k) * f1;
					RHS(j) += RHS(i) * f1;
#endif
				}
			}
		}
		else
		{
#if TEST_PARALLEL
			const unsigned eb = m_terms[i]->m_nzbd.size();
			if (eb > 0)
			{
				//printf("here %d\n", eb);
				unsigned chunks = (eb + num_thr) / (num_thr + 1);
				for (int p=0; p < num_thr + 1; p++)
				{
					x_i[p] = i;
					x_start[p] = chunks * p;
					x_stop[p] = std::min(chunks*(p+1), eb);
					if (p<num_thr && x_start[p] < x_stop[p]) thr_process(p, this, nullptr);
				}
				if (x_start[num_thr] < x_stop[num_thr])
					do_work(num_thr, nullptr);
				thr_wait();
			}
			else if (eb > 0)
			{
				x_i[0] = i;
				x_start[0] = 0;
				x_stop[0] = eb;
				do_work(0, nullptr);
			}
#else

			/* FIXME: Singular matrix? */
			const nl_double f = 1.0 / A(i,i);
			const auto &nzrd = m_terms[i]->m_nzrd;
			const auto &nzbd = m_terms[i]->m_nzbd;

			for (auto & j : nzbd)
			{
				const nl_double f1 = -f * A(j,i);
				for (auto & k : nzrd)
					A(j,k) += A(i,k) * f1;
				//RHS(j) += RHS(i) * f1;
#endif
			}
		}
	}
}

template <unsigned m_N, unsigned storage_N>
template <typename T>
void matrix_solver_direct_t<m_N, storage_N>::LE_back_subst(
		T * RESTRICT x)
{
	const unsigned kN = N();

	/* back substitution */
	if (m_params.m_pivot)
	{
		for (unsigned j = kN; j-- > 0; )
		{
			T tmp = 0;
			for (std::size_t k = j+1; k < kN; k++)
				tmp += A(j,k) * x[k];
			x[j] = (RHS(j) - tmp) / A(j,j);
		}
	}
	else
	{
		for (unsigned j = kN; j-- > 0; )
		{
			T tmp = 0;

			const auto *p = m_terms[j]->m_nzrd.data();
			const auto e = m_terms[j]->m_nzrd.size() - 1; /* exclude RHS element */

			for (unsigned k = 0; k < e; k++)
			{
				const auto pk = p[k];
				tmp += A(j,pk) * x[pk];
			}
			x[j] = (RHS(j) - tmp) / A(j,j);
		}
	}
}


template <unsigned m_N, unsigned storage_N>
unsigned matrix_solver_direct_t<m_N, storage_N>::solve_non_dynamic(const bool newton_raphson)
{
	nl_double new_V[storage_N]; // = { 0.0 };

	this->LE_solve();
	this->LE_back_subst(new_V);

	if (newton_raphson)
	{
		nl_double err = delta(new_V);

		store(new_V);

		return (err > this->m_params.m_accuracy) ? 2 : 1;
	}
	else
	{
		store(new_V);
		return 1;
	}
}

template <unsigned m_N, unsigned storage_N>
inline unsigned matrix_solver_direct_t<m_N, storage_N>::vsolve_non_dynamic(const bool newton_raphson)
{
	build_LE_A<matrix_solver_direct_t>();
	build_LE_RHS<matrix_solver_direct_t>();

	for (unsigned i=0, iN=N(); i < iN; i++)
		m_last_RHS[i] = RHS(i);

	this->m_stat_calculations++;
	return this->solve_non_dynamic(newton_raphson);
}

template <unsigned m_N, unsigned storage_N>
matrix_solver_direct_t<m_N, storage_N>::matrix_solver_direct_t(netlist_t &anetlist, const pstring &name,
		const solver_parameters_t *params, const unsigned size)
: matrix_solver_t(anetlist, name, ASCENDING, params)
, m_dim(size)
{
#if (NL_USE_DYNAMIC_ALLOCATION)
	m_A = palloc_array(nl_ext_double, N() * m_pitch);
#endif
	for (unsigned k = 0; k < N(); k++)
	{
		m_last_RHS[k] = 0.0;
	}
#if TEST_PARALLEL
	thr_initialize();
#endif
}

template <unsigned m_N, unsigned storage_N>
matrix_solver_direct_t<m_N, storage_N>::matrix_solver_direct_t(netlist_t &anetlist, const pstring &name,
		const eSortType sort, const solver_parameters_t *params, const unsigned size)
: matrix_solver_t(anetlist, name, sort, params)
, m_dim(size)
{
#if (NL_USE_DYNAMIC_ALLOCATION)
	m_A = palloc_array(nl_ext_double, N() * m_pitch);
#endif
	for (unsigned k = 0; k < N(); k++)
	{
		m_last_RHS[k] = 0.0;
	}
#if TEST_PARALLEL
	thr_initialize();
#endif
}

	} //namespace devices
} // namespace netlist

#endif /* NLD_MS_DIRECT_H_ */
