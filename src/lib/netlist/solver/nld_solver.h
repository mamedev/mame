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
#include "solver/nld_matrix_solver.h"

//#define ATTR_ALIGNED(N) __attribute__((aligned(N)))
#define ATTR_ALIGNED(N) ATTR_ALIGN

// ----------------------------------------------------------------------------------------
// Macros
// ----------------------------------------------------------------------------------------

#define SOLVER(name, freq)                                                 \
		NET_REGISTER_DEV(SOLVER, name)                                      \
		PARAM(name.FREQ, freq)

// ----------------------------------------------------------------------------------------
// solver
// ----------------------------------------------------------------------------------------

namespace netlist
{
	namespace devices
	{
class NETLIB_NAME(solver);


class matrix_solver_t;

NETLIB_OBJECT(solver)
{
	NETLIB_CONSTRUCTOR(solver)
	, m_fb_step(*this, "FB_step")
	, m_Q_step(*this, "Q_step")
	, m_sync_delay(*this, "SYNC_DELAY", NLTIME_FROM_NS(10).as_double())
	, m_freq(*this, "FREQ", 48000.0)

	/* iteration parameters */
	, m_sor(*this, "SOR_FACTOR", 1.059)
	, m_iterative_solver(*this, "ITERATIVE", "SOR")
	, m_accuracy(*this, "ACCURACY", 1e-7)
	, m_gs_threshold(*this, "GS_THRESHOLD", 6)      // below this value, gaussian elimination is used
	, m_gs_loops(*this, "GS_LOOPS",9)              // Gauss-Seidel loops

	/* general parameters */
	, m_gmin(*this, "GMIN", NETLIST_GMIN_DEFAULT)
	, m_pivot(*this, "PIVOT", 0)                    // use pivoting - on supported solvers
	, m_nr_loops(*this, "NR_LOOPS", 250)            // Newton-Raphson loops
	, m_parallel(*this, "PARALLEL", 0)

	/* automatic time step */
	, m_dynamic(*this, "DYNAMIC_TS", 0)
	, m_lte(*this, "DYNAMIC_LTE", 5e-5)                     // diff/timestep
	, m_min_timestep(*this, "MIN_TIMESTEP", 1e-6)   // nl_double timestep resolution

	, m_log_stats(*this, "LOG_STATS", 1)   // nl_double timestep resolution
	{
		// internal staff

		connect_late(m_fb_step, m_Q_step);
	}

	virtual ~NETLIB_NAME(solver)();

	void post_start();
	void stop() override;

	inline nl_double gmin() { return m_gmin.Value(); }

	void create_solver_code(plib::postream &strm);

	NETLIB_UPDATEI();
	NETLIB_RESETI();
	// NETLIB_UPDATE_PARAMI();

protected:
	logic_input_t m_fb_step;
	logic_output_t m_Q_step;

	param_double_t m_sync_delay;
	param_double_t m_freq;
	param_double_t m_sor;
	param_str_t m_iterative_solver;
	param_double_t m_accuracy;
	param_int_t m_gs_threshold;
	param_int_t m_gs_loops;
	param_double_t m_gmin;
	param_logic_t  m_pivot;
	param_int_t m_nr_loops;
	param_int_t m_parallel;
	param_logic_t  m_dynamic;
	param_double_t m_lte;
	param_double_t m_min_timestep;


	param_logic_t  m_log_stats;

	std::vector<std::unique_ptr<matrix_solver_t>> m_mat_solvers;
private:

	solver_parameters_t m_params;

	template <int m_N, int storage_N>
	std::unique_ptr<matrix_solver_t> create_solver(int size, bool use_specific);
};

	} //namespace devices
} // namespace netlist

#endif /* NLD_SOLVER_H_ */
