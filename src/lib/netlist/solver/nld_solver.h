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

class NETLIB_NAME(solver) : public device_t
{
public:
	NETLIB_NAME(solver)()
	: device_t()    { }

	virtual ~NETLIB_NAME(solver)();

	ATTR_COLD void post_start();
	ATTR_COLD void stop() override;

	inline nl_double gmin() { return m_gmin.Value(); }

protected:
	void update() override;
	void start() override;
	void reset() override;
	void update_param() override;

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
