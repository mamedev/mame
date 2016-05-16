// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_solver.h
 *
 */

#ifndef NLD_SOLVER_H_
#define NLD_SOLVER_H_

#include "nl_setup.h"
#include "nl_base.h"
#include "plib/pstream.h"

//#define ATTR_ALIGNED(N) __attribute__((aligned(N)))
#define ATTR_ALIGNED(N) ATTR_ALIGN

// ----------------------------------------------------------------------------------------
// Macros
// ----------------------------------------------------------------------------------------

#define SOLVER(_name, _freq)                                                 \
		NET_REGISTER_DEV(SOLVER, _name)                                      \
		PARAM(_name.FREQ, _freq)

// ----------------------------------------------------------------------------------------
// solver
// ----------------------------------------------------------------------------------------

NETLIB_NAMESPACE_DEVICES_START()

class NETLIB_NAME(solver);

/* FIXME: these should become proper devices */

struct solver_parameters_t
{
	int m_pivot;
	nl_double m_accuracy;
	nl_double m_lte;
	nl_double m_min_timestep;
	nl_double m_max_timestep;
	nl_double m_sor;
	bool m_dynamic;
	int m_gs_loops;
	int m_nr_loops;
	netlist_time m_nt_sync_delay;
	bool m_log_stats;
};


class matrix_solver_t;

NETLIB_OBJECT(solver)
{
	NETLIB_CONSTRUCTOR(solver)
	{
		enregister("Q_step", m_Q_step);

		register_param("SYNC_DELAY", m_sync_delay, NLTIME_FROM_NS(10).as_double());

		register_param("FREQ", m_freq, 48000.0);


		/* iteration parameters */
		register_param("SOR_FACTOR", m_sor, 1.059);
		register_param("ITERATIVE", m_iterative_solver, "SOR");
		register_param("ACCURACY", m_accuracy, 1e-7);
		register_param("GS_THRESHOLD", m_gs_threshold, 6);      // below this value, gaussian elimination is used
		register_param("GS_LOOPS", m_gs_loops, 9);              // Gauss-Seidel loops

		/* general parameters */
		register_param("GMIN", m_gmin, NETLIST_GMIN_DEFAULT);
		register_param("PIVOT", m_pivot, 0);                    // use pivoting - on supported solvers
		register_param("NR_LOOPS", m_nr_loops, 250);            // Newton-Raphson loops
		register_param("PARALLEL", m_parallel, 0);

		/* automatic time step */
		register_param("DYNAMIC_TS", m_dynamic, 0);
		register_param("DYNAMIC_LTE", m_lte, 5e-5);                     // diff/timestep
		register_param("MIN_TIMESTEP", m_min_timestep, 1e-6);   // nl_double timestep resolution

		register_param("LOG_STATS", m_log_stats, 1);   // nl_double timestep resolution

		// internal staff

		enregister("FB_step", m_fb_step);
		connect_late(m_fb_step, m_Q_step);
	}

	virtual ~NETLIB_NAME(solver)();

	void post_start();
	void stop() override;

	inline nl_double gmin() { return m_gmin.Value(); }

	void create_solver_code(postream &strm);

	NETLIB_UPDATEI();
	NETLIB_RESETI();
	NETLIB_UPDATE_PARAMI();

protected:
	logic_input_t m_fb_step;
	logic_output_t m_Q_step;

	param_logic_t  m_pivot;
	param_double_t m_freq;
	param_double_t m_sync_delay;
	param_double_t m_accuracy;
	param_double_t m_gmin;
	param_double_t m_lte;
	param_double_t m_sor;
	param_logic_t  m_dynamic;
	param_double_t m_min_timestep;

	param_str_t m_iterative_solver;
	param_int_t m_nr_loops;
	param_int_t m_gs_loops;
	param_int_t m_gs_threshold;
	param_int_t m_parallel;

	param_logic_t  m_log_stats;

	pvector_t<matrix_solver_t *> m_mat_solvers;
private:

	solver_parameters_t m_params;

	template <int m_N, int _storage_N>
	matrix_solver_t *create_solver(int size, bool use_specific);
};

NETLIB_NAMESPACE_DEVICES_END()

#endif /* NLD_SOLVER_H_ */
