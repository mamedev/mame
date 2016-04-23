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

NETLIB_NAMESPACE_DEVICES_START()

//#define nl_ext_double __float128 // slow, very slow
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
//	int _block[29]; /* make it 256 bytes */
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

template <unsigned m_N, unsigned _storage_N>
#if TEST_PARALLEL
class matrix_solver_direct_t: public matrix_solver_t, public thr_intf
#else
class matrix_solver_direct_t: public matrix_solver_t
#endif
{
	friend class matrix_solver_t;
public:

	matrix_solver_direct_t(const solver_parameters_t *params, const int size);
	matrix_solver_direct_t(const eSortType sort, const solver_parameters_t *params, const int size);

	virtual ~matrix_solver_direct_t();

	virtual void vsetup(analog_net_t::list_t &nets) override;
	virtual void reset() override { matrix_solver_t::reset(); }

protected:
	virtual int vsolve_non_dynamic(const bool newton_raphson) override;
	int solve_non_dynamic(const bool newton_raphson);

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
	ATTR_ALIGN nl_double m_last_RHS[_storage_N]; // right hand side - contains currents

private:
	static const std::size_t m_pitch = (((_storage_N + 1) + 7) / 8) * 8;
	//static const std::size_t m_pitch = (((_storage_N + 1) + 15) / 16) * 16;
	//static const std::size_t m_pitch = (((_storage_N + 1) + 31) / 32) * 32;
#if (NL_USE_DYNAMIC_ALLOCATION)
	ATTR_ALIGN nl_ext_double * RESTRICT m_A;
#else
	ATTR_ALIGN nl_ext_double m_A[_storage_N][m_pitch];
#endif
	//ATTR_ALIGN nl_ext_double m_RHSx[_storage_N];

	const unsigned m_dim;

};

// ----------------------------------------------------------------------------------------
// matrix_solver_direct
// ----------------------------------------------------------------------------------------

template <unsigned m_N, unsigned _storage_N>
matrix_solver_direct_t<m_N, _storage_N>::~matrix_solver_direct_t()
{
#if (NL_USE_DYNAMIC_ALLOCATION)
	pfree_array(m_A);
#endif
#if TEST_PARALLEL
	thr_dispose();
#endif
}

template <unsigned m_N, unsigned _storage_N>
ATTR_COLD void matrix_solver_direct_t<m_N, _storage_N>::vsetup(analog_net_t::list_t &nets)
{
	if (m_dim < nets.size())
		log().fatal("Dimension {1} less than {2}", m_dim, nets.size());

	matrix_solver_t::setup_base(nets);

	/* add RHS element */
	for (unsigned k = 0; k < N(); k++)
	{
		terms_t * t = m_terms[k];

		if (!t->m_nzrd.contains(N()))
			t->m_nzrd.push_back(N());
	}

	save(NLNAME(m_last_RHS));

	for (unsigned k = 0; k < N(); k++)
	{
		pstring num = pfmt("{1}")(k);

		save(RHS(k), "RHS." + num);
	}
}



#if TEST_PARALLEL
template <unsigned m_N, unsigned _storage_N>
void matrix_solver_direct_t<m_N, _storage_N>::do_work(const int id, void *param)
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

template <unsigned m_N, unsigned _storage_N>
void matrix_solver_direct_t<m_N, _storage_N>::LE_solve()
{
#if 0
	// Static matrix compilation
	const double fd0 = 1.0 / A(0,0);
	  const double f1 = -fd0 * A(23,0);
	    A(23,23) += f1 * A(0,23);
	    RHS(23) += f1 * RHS(0);
	const double fd1 = 1.0 / A(1,1);
	  const double f2 = -fd1 * A(39,1);
	    A(39,39) += f2 * A(1,39);
	    RHS(39) += f2 * RHS(1);
	const double fd2 = 1.0 / A(2,2);
	  const double f3 = -fd2 * A(31,2);
	    A(31,31) += f3 * A(2,31);
	    RHS(31) += f3 * RHS(2);
	const double fd3 = 1.0 / A(3,3);
	  const double f4 = -fd3 * A(19,3);
	    A(19,19) += f4 * A(3,19);
	    RHS(19) += f4 * RHS(3);
	const double fd4 = 1.0 / A(4,4);
	  const double f5 = -fd4 * A(29,4);
	    A(29,29) += f5 * A(4,29);
	    RHS(29) += f5 * RHS(4);
	const double fd5 = 1.0 / A(5,5);
	  const double f6 = -fd5 * A(32,5);
	    A(32,32) += f6 * A(5,32);
	    RHS(32) += f6 * RHS(5);
	const double fd6 = 1.0 / A(6,6);
	  const double f7 = -fd6 * A(69,6);
	    A(69,69) += f7 * A(6,69);
	    RHS(69) += f7 * RHS(6);
	const double fd7 = 1.0 / A(7,7);
	  const double f8 = -fd7 * A(22,7);
	    A(22,22) += f8 * A(7,22);
	    RHS(22) += f8 * RHS(7);
	const double fd8 = 1.0 / A(8,8);
	  const double f9 = -fd8 * A(28,8);
	    A(28,28) += f9 * A(8,28);
	    RHS(28) += f9 * RHS(8);
	const double fd9 = 1.0 / A(9,9);
	  const double f10 = -fd9 * A(82,9);
	    A(82,82) += f10 * A(9,82);
	    RHS(82) += f10 * RHS(9);
	const double fd10 = 1.0 / A(10,10);
	  const double f11 = -fd10 * A(82,10);
	    A(82,82) += f11 * A(10,82);
	    RHS(82) += f11 * RHS(10);
	const double fd11 = 1.0 / A(11,11);
	  const double f12 = -fd11 * A(38,11);
	    A(38,38) += f12 * A(11,38);
	    RHS(38) += f12 * RHS(11);
	const double fd12 = 1.0 / A(12,12);
	  const double f13 = -fd12 * A(43,12);
	    A(43,84) += f13 * A(12,84);
	    RHS(43) += f13 * RHS(12);
	  const double f14 = -fd12 * A(84,12);
	    A(84,84) += f14 * A(12,84);
	    RHS(84) += f14 * RHS(12);
	const double fd13 = 1.0 / A(13,13);
	  const double f15 = -fd13 * A(48,13);
	    A(48,48) += f15 * A(13,48);
	    RHS(48) += f15 * RHS(13);
	const double fd14 = 1.0 / A(14,14);
	  const double f16 = -fd14 * A(56,14);
	    A(56,56) += f16 * A(14,56);
	    RHS(56) += f16 * RHS(14);
	const double fd15 = 1.0 / A(15,15);
	  const double f17 = -fd15 * A(60,15);
	    A(60,60) += f17 * A(15,60);
	    RHS(60) += f17 * RHS(15);
	const double fd16 = 1.0 / A(16,16);
	  const double f18 = -fd16 * A(81,16);
	    A(81,81) += f18 * A(16,81);
	    A(81,85) = f18 * A(16,85);
	    RHS(81) += f18 * RHS(16);
	  const double f19 = -fd16 * A(85,16);
	    A(85,81) = f19 * A(16,81);
	    A(85,85) += f19 * A(16,85);
	    RHS(85) += f19 * RHS(16);
	const double fd17 = 1.0 / A(17,17);
	  const double f20 = -fd17 * A(72,17);
	    A(72,72) += f20 * A(17,72);
	    A(72,85) = f20 * A(17,85);
	    RHS(72) += f20 * RHS(17);
	  const double f21 = -fd17 * A(85,17);
	    A(85,72) = f21 * A(17,72);
	    A(85,85) += f21 * A(17,85);
	    RHS(85) += f21 * RHS(17);
	const double fd18 = 1.0 / A(18,18);
	  const double f22 = -fd18 * A(72,18);
	    A(72,72) += f22 * A(18,72);
	    A(72,78) += f22 * A(18,78);
	    RHS(72) += f22 * RHS(18);
	  const double f23 = -fd18 * A(78,18);
	    A(78,72) += f23 * A(18,72);
	    A(78,78) += f23 * A(18,78);
	    RHS(78) += f23 * RHS(18);
	const double fd19 = 1.0 / A(19,19);
	  const double f24 = -fd19 * A(87,19);
	    A(87,87) += f24 * A(19,87);
	    RHS(87) += f24 * RHS(19);
	const double fd20 = 1.0 / A(20,20);
	  const double f25 = -fd20 * A(71,20);
	    A(71,69) += f25 * A(20,69);
	    A(71,85) = f25 * A(20,85);
	    RHS(71) += f25 * RHS(20);
	const double fd21 = 1.0 / A(21,21);
	  const double f26 = -fd21 * A(24,21);
	    A(24,63) = f26 * A(21,63);
	    A(24,74) += f26 * A(21,74);
	    RHS(24) += f26 * RHS(21);
	  const double f27 = -fd21 * A(63,21);
	    A(63,63) += f27 * A(21,63);
	    A(63,74) += f27 * A(21,74);
	    RHS(63) += f27 * RHS(21);
	  const double f28 = -fd21 * A(74,21);
	    A(74,63) += f28 * A(21,63);
	    A(74,74) += f28 * A(21,74);
	    RHS(74) += f28 * RHS(21);
	const double fd22 = 1.0 / A(22,22);
	  const double f29 = -fd22 * A(63,22);
	    A(63,63) += f29 * A(22,63);
	    RHS(63) += f29 * RHS(22);
	const double fd23 = 1.0 / A(23,23);
	  const double f30 = -fd23 * A(87,23);
	    A(87,87) += f30 * A(23,87);
	    RHS(87) += f30 * RHS(23);
	const double fd24 = 1.0 / A(24,24);
	  const double f31 = -fd24 * A(74,24);
	    A(74,63) += f31 * A(24,63);
	    A(74,74) += f31 * A(24,74);
	    RHS(74) += f31 * RHS(24);
	const double fd25 = 1.0 / A(25,25);
	  const double f32 = -fd25 * A(77,25);
	    A(77,26) += f32 * A(25,26);
	    A(77,77) += f32 * A(25,77);
	    RHS(77) += f32 * RHS(25);
	const double fd26 = 1.0 / A(26,26);
	  const double f33 = -fd26 * A(62,26);
	    A(62,62) += f33 * A(26,62);
	    A(62,77) += f33 * A(26,77);
	    RHS(62) += f33 * RHS(26);
	  const double f34 = -fd26 * A(77,26);
	    A(77,62) += f34 * A(26,62);
	    A(77,77) += f34 * A(26,77);
	    RHS(77) += f34 * RHS(26);
	const double fd27 = 1.0 / A(27,27);
	  const double f35 = -fd27 * A(62,27);
	    A(62,62) += f35 * A(27,62);
	    A(62,68) = f35 * A(27,68);
	    RHS(62) += f35 * RHS(27);
	  const double f36 = -fd27 * A(68,27);
	    A(68,62) = f36 * A(27,62);
	    A(68,68) += f36 * A(27,68);
	    RHS(68) += f36 * RHS(27);
	const double fd28 = 1.0 / A(28,28);
	  const double f37 = -fd28 * A(30,28);
	    A(30,68) += f37 * A(28,68);
	    RHS(30) += f37 * RHS(28);
	  const double f38 = -fd28 * A(68,28);
	    A(68,68) += f38 * A(28,68);
	    RHS(68) += f38 * RHS(28);
	const double fd29 = 1.0 / A(29,29);
	  const double f39 = -fd29 * A(87,29);
	    A(87,87) += f39 * A(29,87);
	    RHS(87) += f39 * RHS(29);
	const double fd30 = 1.0 / A(30,30);
	  const double f40 = -fd30 * A(68,30);
	    A(68,68) += f40 * A(30,68);
	    RHS(68) += f40 * RHS(30);
	const double fd31 = 1.0 / A(31,31);
	  const double f41 = -fd31 * A(87,31);
	    A(87,87) += f41 * A(31,87);
	    RHS(87) += f41 * RHS(31);
	const double fd32 = 1.0 / A(32,32);
	  const double f42 = -fd32 * A(87,32);
	    A(87,87) += f42 * A(32,87);
	    RHS(87) += f42 * RHS(32);
	const double fd33 = 1.0 / A(33,33);
	  const double f43 = -fd33 * A(79,33);
	    A(79,79) += f43 * A(33,79);
	    A(79,85) = f43 * A(33,85);
	    RHS(79) += f43 * RHS(33);
	  const double f44 = -fd33 * A(85,33);
	    A(85,79) = f44 * A(33,79);
	    A(85,85) += f44 * A(33,85);
	    RHS(85) += f44 * RHS(33);
	const double fd34 = 1.0 / A(34,34);
	  const double f45 = -fd34 * A(50,34);
	    A(50,65) = f45 * A(34,65);
	    A(50,79) += f45 * A(34,79);
	    RHS(50) += f45 * RHS(34);
	  const double f46 = -fd34 * A(65,34);
	    A(65,65) += f46 * A(34,65);
	    A(65,79) += f46 * A(34,79);
	    RHS(65) += f46 * RHS(34);
	  const double f47 = -fd34 * A(79,34);
	    A(79,65) += f47 * A(34,65);
	    A(79,79) += f47 * A(34,79);
	    RHS(79) += f47 * RHS(34);
	const double fd35 = 1.0 / A(35,35);
	  const double f48 = -fd35 * A(36,35);
	    A(36,36) += f48 * A(35,36);
	    A(36,66) = f48 * A(35,66);
	    RHS(36) += f48 * RHS(35);
	  const double f49 = -fd35 * A(66,35);
	    A(66,36) = f49 * A(35,36);
	    A(66,66) += f49 * A(35,66);
	    RHS(66) += f49 * RHS(35);
	const double fd36 = 1.0 / A(36,36);
	  const double f50 = -fd36 * A(37,36);
	    A(37,37) += f50 * A(36,37);
	    A(37,66) = f50 * A(36,66);
	    RHS(37) += f50 * RHS(36);
	  const double f51 = -fd36 * A(66,36);
	    A(66,37) = f51 * A(36,37);
	    A(66,66) += f51 * A(36,66);
	    RHS(66) += f51 * RHS(36);
	const double fd37 = 1.0 / A(37,37);
	  const double f52 = -fd37 * A(38,37);
	    A(38,38) += f52 * A(37,38);
	    A(38,66) = f52 * A(37,66);
	    RHS(38) += f52 * RHS(37);
	  const double f53 = -fd37 * A(66,37);
	    A(66,38) = f53 * A(37,38);
	    A(66,66) += f53 * A(37,66);
	    RHS(66) += f53 * RHS(37);
	const double fd38 = 1.0 / A(38,38);
	  const double f54 = -fd38 * A(66,38);
	    A(66,66) += f54 * A(38,66);
	    RHS(66) += f54 * RHS(38);
	const double fd39 = 1.0 / A(39,39);
	  const double f55 = -fd39 * A(87,39);
	    A(87,87) += f55 * A(39,87);
	    RHS(87) += f55 * RHS(39);
	const double fd40 = 1.0 / A(40,40);
	  const double f56 = -fd40 * A(41,40);
	    A(41,41) += f56 * A(40,41);
	    A(41,86) = f56 * A(40,86);
	    RHS(41) += f56 * RHS(40);
	  const double f57 = -fd40 * A(86,40);
	    A(86,41) = f57 * A(40,41);
	    A(86,86) += f57 * A(40,86);
	    RHS(86) += f57 * RHS(40);
	const double fd41 = 1.0 / A(41,41);
	  const double f58 = -fd41 * A(42,41);
	    A(42,42) += f58 * A(41,42);
	    A(42,86) = f58 * A(41,86);
	    RHS(42) += f58 * RHS(41);
	  const double f59 = -fd41 * A(86,41);
	    A(86,42) = f59 * A(41,42);
	    A(86,86) += f59 * A(41,86);
	    RHS(86) += f59 * RHS(41);
	const double fd42 = 1.0 / A(42,42);
	  const double f60 = -fd42 * A(86,42);
	    A(86,43) = f60 * A(42,43);
	    A(86,86) += f60 * A(42,86);
	    RHS(86) += f60 * RHS(42);
	const double fd43 = 1.0 / A(43,43);
	  const double f61 = -fd43 * A(86,43);
	    A(86,84) = f61 * A(43,84);
	    RHS(86) += f61 * RHS(43);
	const double fd44 = 1.0 / A(44,44);
	  const double f62 = -fd44 * A(84,44);
	    A(84,84) += f62 * A(44,84);
	    A(84,87) = f62 * A(44,87);
	    RHS(84) += f62 * RHS(44);
	  const double f63 = -fd44 * A(87,44);
	    A(87,84) = f63 * A(44,84);
	    A(87,87) += f63 * A(44,87);
	    RHS(87) += f63 * RHS(44);
	const double fd45 = 1.0 / A(45,45);
	  const double f64 = -fd45 * A(46,45);
	    A(46,46) += f64 * A(45,46);
	    A(46,67) = f64 * A(45,67);
	    RHS(46) += f64 * RHS(45);
	  const double f65 = -fd45 * A(67,45);
	    A(67,46) = f65 * A(45,46);
	    A(67,67) += f65 * A(45,67);
	    RHS(67) += f65 * RHS(45);
	const double fd46 = 1.0 / A(46,46);
	  const double f66 = -fd46 * A(47,46);
	    A(47,47) += f66 * A(46,47);
	    A(47,67) = f66 * A(46,67);
	    RHS(47) += f66 * RHS(46);
	  const double f67 = -fd46 * A(67,46);
	    A(67,47) = f67 * A(46,47);
	    A(67,67) += f67 * A(46,67);
	    RHS(67) += f67 * RHS(46);
	const double fd47 = 1.0 / A(47,47);
	  const double f68 = -fd47 * A(48,47);
	    A(48,48) += f68 * A(47,48);
	    A(48,67) = f68 * A(47,67);
	    RHS(48) += f68 * RHS(47);
	  const double f69 = -fd47 * A(67,47);
	    A(67,48) = f69 * A(47,48);
	    A(67,67) += f69 * A(47,67);
	    RHS(67) += f69 * RHS(47);
	const double fd48 = 1.0 / A(48,48);
	  const double f70 = -fd48 * A(67,48);
	    A(67,67) += f70 * A(48,67);
	    RHS(67) += f70 * RHS(48);
	const double fd49 = 1.0 / A(49,49);
	  const double f71 = -fd49 * A(80,49);
	    A(80,80) += f71 * A(49,80);
	    A(80,84) += f71 * A(49,84);
	    RHS(80) += f71 * RHS(49);
	  const double f72 = -fd49 * A(84,49);
	    A(84,80) += f72 * A(49,80);
	    A(84,84) += f72 * A(49,84);
	    RHS(84) += f72 * RHS(49);
	const double fd50 = 1.0 / A(50,50);
	  const double f73 = -fd50 * A(79,50);
	    A(79,65) += f73 * A(50,65);
	    A(79,79) += f73 * A(50,79);
	    RHS(79) += f73 * RHS(50);
	const double fd51 = 1.0 / A(51,51);
	  const double f74 = -fd51 * A(83,51);
	    A(83,83) += f74 * A(51,83);
	    A(83,85) = f74 * A(51,85);
	    RHS(83) += f74 * RHS(51);
	  const double f75 = -fd51 * A(85,51);
	    A(85,83) = f75 * A(51,83);
	    A(85,85) += f75 * A(51,85);
	    RHS(85) += f75 * RHS(51);
	const double fd52 = 1.0 / A(52,52);
	  const double f76 = -fd52 * A(76,52);
	    A(76,76) += f76 * A(52,76);
	    A(76,83) += f76 * A(52,83);
	    RHS(76) += f76 * RHS(52);
	  const double f77 = -fd52 * A(83,52);
	    A(83,76) += f77 * A(52,76);
	    A(83,83) += f77 * A(52,83);
	    RHS(83) += f77 * RHS(52);
	const double fd53 = 1.0 / A(53,53);
	  const double f78 = -fd53 * A(54,53);
	    A(54,54) += f78 * A(53,54);
	    A(54,64) = f78 * A(53,64);
	    RHS(54) += f78 * RHS(53);
	  const double f79 = -fd53 * A(64,53);
	    A(64,54) = f79 * A(53,54);
	    A(64,64) += f79 * A(53,64);
	    RHS(64) += f79 * RHS(53);
	const double fd54 = 1.0 / A(54,54);
	  const double f80 = -fd54 * A(64,54);
	    A(64,64) += f80 * A(54,64);
	    A(64,83) = f80 * A(54,83);
	    RHS(64) += f80 * RHS(54);
	  const double f81 = -fd54 * A(83,54);
	    A(83,64) = f81 * A(54,64);
	    A(83,83) += f81 * A(54,83);
	    RHS(83) += f81 * RHS(54);
	const double fd55 = 1.0 / A(55,55);
	  const double f82 = -fd55 * A(56,55);
	    A(56,56) += f82 * A(55,56);
	    A(56,64) = f82 * A(55,64);
	    RHS(56) += f82 * RHS(55);
	  const double f83 = -fd55 * A(64,55);
	    A(64,56) = f83 * A(55,56);
	    A(64,64) += f83 * A(55,64);
	    RHS(64) += f83 * RHS(55);
	const double fd56 = 1.0 / A(56,56);
	  const double f84 = -fd56 * A(64,56);
	    A(64,64) += f84 * A(56,64);
	    RHS(64) += f84 * RHS(56);
	const double fd57 = 1.0 / A(57,57);
	  const double f85 = -fd57 * A(78,57);
	    A(78,78) += f85 * A(57,78);
	    A(78,80) = f85 * A(57,80);
	    RHS(78) += f85 * RHS(57);
	  const double f86 = -fd57 * A(80,57);
	    A(80,78) = f86 * A(57,78);
	    A(80,80) += f86 * A(57,80);
	    RHS(80) += f86 * RHS(57);
	const double fd58 = 1.0 / A(58,58);
	  const double f87 = -fd58 * A(75,58);
	    A(75,75) += f87 * A(58,75);
	    A(75,81) += f87 * A(58,81);
	    RHS(75) += f87 * RHS(58);
	  const double f88 = -fd58 * A(81,58);
	    A(81,75) += f88 * A(58,75);
	    A(81,81) += f88 * A(58,81);
	    RHS(81) += f88 * RHS(58);
	const double fd59 = 1.0 / A(59,59);
	  const double f89 = -fd59 * A(60,59);
	    A(60,60) += f89 * A(59,60);
	    A(60,70) = f89 * A(59,70);
	    RHS(60) += f89 * RHS(59);
	  const double f90 = -fd59 * A(70,59);
	    A(70,60) = f90 * A(59,60);
	    A(70,70) += f90 * A(59,70);
	    RHS(70) += f90 * RHS(59);
	const double fd60 = 1.0 / A(60,60);
	  const double f91 = -fd60 * A(70,60);
	    A(70,70) += f91 * A(60,70);
	    RHS(70) += f91 * RHS(60);
	const double fd61 = 1.0 / A(61,61);
	  const double f92 = -fd61 * A(73,61);
	    A(73,73) += f92 * A(61,73);
	    A(73,75) = f92 * A(61,75);
	    RHS(73) += f92 * RHS(61);
	  const double f93 = -fd61 * A(75,61);
	    A(75,73) = f93 * A(61,73);
	    A(75,75) += f93 * A(61,75);
	    RHS(75) += f93 * RHS(61);
	const double fd62 = 1.0 / A(62,62);
	  const double f94 = -fd62 * A(68,62);
	    A(68,68) += f94 * A(62,68);
	    A(68,77) = f94 * A(62,77);
	    RHS(68) += f94 * RHS(62);
	  const double f95 = -fd62 * A(77,62);
	    A(77,68) = f95 * A(62,68);
	    A(77,77) += f95 * A(62,77);
	    RHS(77) += f95 * RHS(62);
	const double fd63 = 1.0 / A(63,63);
	  const double f96 = -fd63 * A(74,63);
	    A(74,74) += f96 * A(63,74);
	    RHS(74) += f96 * RHS(63);
	const double fd64 = 1.0 / A(64,64);
	  const double f97 = -fd64 * A(76,64);
	    A(76,76) += f97 * A(64,76);
	    A(76,83) += f97 * A(64,83);
	    RHS(76) += f97 * RHS(64);
	  const double f98 = -fd64 * A(83,64);
	    A(83,76) += f98 * A(64,76);
	    A(83,83) += f98 * A(64,83);
	    RHS(83) += f98 * RHS(64);
	const double fd65 = 1.0 / A(65,65);
	  const double f99 = -fd65 * A(79,65);
	    A(79,79) += f99 * A(65,79);
	    A(79,86) = f99 * A(65,86);
	    RHS(79) += f99 * RHS(65);
	  const double f100 = -fd65 * A(86,65);
	    A(86,79) = f100 * A(65,79);
	    A(86,86) += f100 * A(65,86);
	    RHS(86) += f100 * RHS(65);
	const double fd66 = 1.0 / A(66,66);
	  const double f101 = -fd66 * A(86,66);
	    A(86,86) += f101 * A(66,86);
	    RHS(86) += f101 * RHS(66);
	const double fd67 = 1.0 / A(67,67);
	  const double f102 = -fd67 * A(86,67);
	    A(86,86) += f102 * A(67,86);
	    RHS(86) += f102 * RHS(67);
	const double fd68 = 1.0 / A(68,68);
	  const double f103 = -fd68 * A(77,68);
	    A(77,77) += f103 * A(68,77);
	    RHS(77) += f103 * RHS(68);
	const double fd69 = 1.0 / A(69,69);
	  const double f104 = -fd69 * A(71,69);
	    A(71,71) += f104 * A(69,71);
	    A(71,85) += f104 * A(69,85);
	    RHS(71) += f104 * RHS(69);
	  const double f105 = -fd69 * A(85,69);
	    A(85,71) = f105 * A(69,71);
	    A(85,85) += f105 * A(69,85);
	    RHS(85) += f105 * RHS(69);
	const double fd70 = 1.0 / A(70,70);
	  const double f106 = -fd70 * A(73,70);
	    A(73,73) += f106 * A(70,73);
	    A(73,78) = f106 * A(70,78);
	    RHS(73) += f106 * RHS(70);
	  const double f107 = -fd70 * A(78,70);
	    A(78,73) = f107 * A(70,73);
	    A(78,78) += f107 * A(70,78);
	    RHS(78) += f107 * RHS(70);
	const double fd71 = 1.0 / A(71,71);
	  const double f108 = -fd71 * A(82,71);
	    A(82,82) += f108 * A(71,82);
	    A(82,85) = f108 * A(71,85);
	    RHS(82) += f108 * RHS(71);
	  const double f109 = -fd71 * A(85,71);
	    A(85,82) = f109 * A(71,82);
	    A(85,85) += f109 * A(71,85);
	    RHS(85) += f109 * RHS(71);
	const double fd72 = 1.0 / A(72,72);
	  const double f110 = -fd72 * A(78,72);
	    A(78,78) += f110 * A(72,78);
	    A(78,85) = f110 * A(72,85);
	    RHS(78) += f110 * RHS(72);
	  const double f111 = -fd72 * A(85,72);
	    A(85,78) = f111 * A(72,78);
	    A(85,85) += f111 * A(72,85);
	    RHS(85) += f111 * RHS(72);
	const double fd73 = 1.0 / A(73,73);
	  const double f112 = -fd73 * A(75,73);
	    A(75,75) += f112 * A(73,75);
	    A(75,78) = f112 * A(73,78);
	    A(75,81) += f112 * A(73,81);
	    RHS(75) += f112 * RHS(73);
	  const double f113 = -fd73 * A(78,73);
	    A(78,75) = f113 * A(73,75);
	    A(78,78) += f113 * A(73,78);
	    A(78,81) = f113 * A(73,81);
	    RHS(78) += f113 * RHS(73);
	  const double f114 = -fd73 * A(81,73);
	    A(81,75) += f114 * A(73,75);
	    A(81,78) = f114 * A(73,78);
	    A(81,81) += f114 * A(73,81);
	    RHS(81) += f114 * RHS(73);
	const double fd74 = 1.0 / A(74,74);
	  const double f115 = -fd74 * A(82,74);
	    A(82,82) += f115 * A(74,82);
	    RHS(82) += f115 * RHS(74);
	const double fd75 = 1.0 / A(75,75);
	  const double f116 = -fd75 * A(78,75);
	    A(78,78) += f116 * A(75,78);
	    A(78,81) += f116 * A(75,81);
	    RHS(78) += f116 * RHS(75);
	  const double f117 = -fd75 * A(81,75);
	    A(81,78) += f117 * A(75,78);
	    A(81,81) += f117 * A(75,81);
	    RHS(81) += f117 * RHS(75);
	const double fd76 = 1.0 / A(76,76);
	  const double f118 = -fd76 * A(83,76);
	    A(83,83) += f118 * A(76,83);
	    RHS(83) += f118 * RHS(76);
	const double fd77 = 1.0 / A(77,77);
	  const double f119 = -fd77 * A(82,77);
	    A(82,82) += f119 * A(77,82);
	    RHS(82) += f119 * RHS(77);
	const double fd78 = 1.0 / A(78,78);
	  const double f120 = -fd78 * A(80,78);
	    A(80,80) += f120 * A(78,80);
	    A(80,81) = f120 * A(78,81);
	    A(80,85) = f120 * A(78,85);
	    RHS(80) += f120 * RHS(78);
	  const double f121 = -fd78 * A(81,78);
	    A(81,80) = f121 * A(78,80);
	    A(81,81) += f121 * A(78,81);
	    A(81,85) += f121 * A(78,85);
	    RHS(81) += f121 * RHS(78);
	  const double f122 = -fd78 * A(85,78);
	    A(85,80) = f122 * A(78,80);
	    A(85,81) += f122 * A(78,81);
	    A(85,85) += f122 * A(78,85);
	    RHS(85) += f122 * RHS(78);
	const double fd79 = 1.0 / A(79,79);
	  const double f123 = -fd79 * A(85,79);
	    A(85,85) += f123 * A(79,85);
	    A(85,86) = f123 * A(79,86);
	    RHS(85) += f123 * RHS(79);
	  const double f124 = -fd79 * A(86,79);
	    A(86,85) = f124 * A(79,85);
	    A(86,86) += f124 * A(79,86);
	    RHS(86) += f124 * RHS(79);
	const double fd80 = 1.0 / A(80,80);
	  const double f125 = -fd80 * A(81,80);
	    A(81,81) += f125 * A(80,81);
	    A(81,84) = f125 * A(80,84);
	    A(81,85) += f125 * A(80,85);
	    RHS(81) += f125 * RHS(80);
	  const double f126 = -fd80 * A(84,80);
	    A(84,81) = f126 * A(80,81);
	    A(84,84) += f126 * A(80,84);
	    A(84,85) = f126 * A(80,85);
	    RHS(84) += f126 * RHS(80);
	  const double f127 = -fd80 * A(85,80);
	    A(85,81) += f127 * A(80,81);
	    A(85,84) = f127 * A(80,84);
	    A(85,85) += f127 * A(80,85);
	    RHS(85) += f127 * RHS(80);
	const double fd81 = 1.0 / A(81,81);
	  const double f128 = -fd81 * A(84,81);
	    A(84,84) += f128 * A(81,84);
	    A(84,85) += f128 * A(81,85);
	    RHS(84) += f128 * RHS(81);
	  const double f129 = -fd81 * A(85,81);
	    A(85,84) += f129 * A(81,84);
	    A(85,85) += f129 * A(81,85);
	    RHS(85) += f129 * RHS(81);
	const double fd82 = 1.0 / A(82,82);
	  const double f130 = -fd82 * A(85,82);
	    A(85,85) += f130 * A(82,85);
	    RHS(85) += f130 * RHS(82);
	const double fd83 = 1.0 / A(83,83);
	  const double f131 = -fd83 * A(85,83);
	    A(85,85) += f131 * A(83,85);
	    RHS(85) += f131 * RHS(83);
	const double fd84 = 1.0 / A(84,84);
	  const double f132 = -fd84 * A(85,84);
	    A(85,85) += f132 * A(84,85);
	    A(85,87) = f132 * A(84,87);
	    RHS(85) += f132 * RHS(84);
	  const double f133 = -fd84 * A(86,84);
	    A(86,85) += f133 * A(84,85);
	    A(86,87) = f133 * A(84,87);
	    RHS(86) += f133 * RHS(84);
	  const double f134 = -fd84 * A(87,84);
	    A(87,85) = f134 * A(84,85);
	    A(87,87) += f134 * A(84,87);
	    RHS(87) += f134 * RHS(84);
	const double fd85 = 1.0 / A(85,85);
	  const double f135 = -fd85 * A(86,85);
	    A(86,86) += f135 * A(85,86);
	    A(86,87) += f135 * A(85,87);
	    RHS(86) += f135 * RHS(85);
	  const double f136 = -fd85 * A(87,85);
	    A(87,86) = f136 * A(85,86);
	    A(87,87) += f136 * A(85,87);
	    RHS(87) += f136 * RHS(85);
	const double fd86 = 1.0 / A(86,86);
	  const double f137 = -fd86 * A(87,86);
	    A(87,87) += f137 * A(86,87);
	    RHS(87) += f137 * RHS(86);
	return;
#endif
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
					//	pj[k] = pj[k] + pi[k] * f1;
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
					x_stop[p] = nl_math::min(chunks*(p+1), eb);
					if (p<num_thr && x_start[p] < x_stop[p]) thr_process(p, this, NULL);
				}
				if (x_start[num_thr] < x_stop[num_thr])
					do_work(num_thr, NULL);
				thr_wait();
			}
			else if (eb > 0)
			{
				x_i[0] = i;
				x_start[0] = 0;
				x_stop[0] = eb;
				do_work(0, NULL);
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

template <unsigned m_N, unsigned _storage_N>
template <typename T>
void matrix_solver_direct_t<m_N, _storage_N>::LE_back_subst(
		T * RESTRICT x)
{
	const unsigned kN = N();

	/* back substitution */
	if (m_params.m_pivot)
	{
		for (int j = kN - 1; j >= 0; j--)
		{
			T tmp = 0;
			for (unsigned k = j+1; k < kN; k++)
				tmp += A(j,k) * x[k];
			x[j] = (RHS(j) - tmp) / A(j,j);
		}
	}
	else
	{
		for (int j = kN - 1; j >= 0; j--)
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


template <unsigned m_N, unsigned _storage_N>
int matrix_solver_direct_t<m_N, _storage_N>::solve_non_dynamic(ATTR_UNUSED const bool newton_raphson)
{
	nl_double new_V[_storage_N]; // = { 0.0 };

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

template <unsigned m_N, unsigned _storage_N>
inline int matrix_solver_direct_t<m_N, _storage_N>::vsolve_non_dynamic(const bool newton_raphson)
{
	build_LE_A<matrix_solver_direct_t>();
	build_LE_RHS<matrix_solver_direct_t>();

	for (unsigned i=0, iN=N(); i < iN; i++)
		m_last_RHS[i] = RHS(i);

	this->m_stat_calculations++;
	return this->solve_non_dynamic(newton_raphson);
}

template <unsigned m_N, unsigned _storage_N>
matrix_solver_direct_t<m_N, _storage_N>::matrix_solver_direct_t(const solver_parameters_t *params, const int size)
: matrix_solver_t(ASCENDING, params)
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

template <unsigned m_N, unsigned _storage_N>
matrix_solver_direct_t<m_N, _storage_N>::matrix_solver_direct_t(const eSortType sort, const solver_parameters_t *params, const int size)
: matrix_solver_t(sort, params)
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

NETLIB_NAMESPACE_DEVICES_END()

#endif /* NLD_MS_DIRECT_H_ */
