/*
 * nld_solver.h
 *
 */

#ifndef NLD_SOLVER_H_
#define NLD_SOLVER_H_

#include "../nl_setup.h"
#include "../nl_base.h"

// ----------------------------------------------------------------------------------------
// Macros
// ----------------------------------------------------------------------------------------

#define SOLVER(_name, _freq)                                                 \
		NET_REGISTER_DEV(solver, _name)                                      \
		PARAM(_name.FREQ, _freq)

// ----------------------------------------------------------------------------------------
// solver
// ----------------------------------------------------------------------------------------

class NETLIB_NAME(solver);

/* FIXME: these should become proper devices */

struct netlist_solver_parameters_t
{
	double m_accuracy;
	double m_convergence_factor;
	int m_resched_loops;
};

class netlist_matrix_solver_t
{
public:
	typedef netlist_list_t<netlist_matrix_solver_t *> list_t;
	typedef netlist_core_device_t::list_t dev_list_t;

	netlist_matrix_solver_t() : m_owner(NULL) {}
	virtual ~netlist_matrix_solver_t() {}

	ATTR_COLD virtual void setup(netlist_net_t::list_t &nets, NETLIB_NAME(solver) &owner);

	// return true if a reschedule is needed ...
	ATTR_HOT virtual int solve_non_dynamic() = 0;
	ATTR_HOT virtual void step(const netlist_time delta);

	ATTR_HOT bool solve();

	ATTR_HOT void update_inputs();
	ATTR_HOT void update_dynamic();

	ATTR_HOT void schedule();

	ATTR_HOT inline bool is_dynamic() { return m_dynamic.count() > 0; }
	ATTR_HOT inline bool is_timestep() { return m_steps.count() > 0; }

	ATTR_HOT inline const NETLIB_NAME(solver) &owner() const;
	ATTR_COLD virtual void reset();

	netlist_solver_parameters_t m_params;

protected:
	netlist_net_t::list_t m_nets;
	dev_list_t m_dynamic;
	netlist_core_terminal_t::list_t m_inps;
	dev_list_t m_steps;
	netlist_time m_last_step;

	NETLIB_NAME(solver) *m_owner;
};

template <int m_N, int _storage_N>
class netlist_matrix_solver_direct_t: public netlist_matrix_solver_t
{
public:

	netlist_matrix_solver_direct_t() : netlist_matrix_solver_t() {}

	virtual ~netlist_matrix_solver_direct_t() {}

	ATTR_COLD virtual void setup(netlist_net_t::list_t &nets, NETLIB_NAME(solver) &owner);
	ATTR_COLD virtual void reset() { netlist_matrix_solver_t::reset(); }
	ATTR_HOT virtual int solve_non_dynamic();

	ATTR_HOT inline const int N() const { return (m_N == 0) ? m_nets.count() : m_N; }

protected:
	ATTR_HOT inline void build_LE(double (* RESTRICT A)[_storage_N], double (* RESTRICT RHS));
	ATTR_HOT inline void gauss_LE(double (* RESTRICT A)[_storage_N],
			double (* RESTRICT RHS),
			double (* RESTRICT x));
	ATTR_HOT inline double delta(
			const double (* RESTRICT RHS),
			const double (* RESTRICT V));
	ATTR_HOT inline void store(const double (* RESTRICT RHS), const double (* RESTRICT V));

	double m_RHS[_storage_N]; // right hand side - contains currents

private:

	ATTR_COLD int get_net_idx(netlist_net_t *net);

	struct terms_t{
		int net_this;
		int net_other;
		netlist_terminal_t *term;
	};
	int m_term_num;
	int m_rail_start;
	terms_t m_terms[100];
};

template <int m_N, int _storage_N>
class netlist_matrix_solver_gauss_seidel_t: public netlist_matrix_solver_t
{
public:

	netlist_matrix_solver_gauss_seidel_t() : netlist_matrix_solver_t() {}

	virtual ~netlist_matrix_solver_gauss_seidel_t() {}

	ATTR_COLD virtual void setup(netlist_net_t::list_t &nets, NETLIB_NAME(solver) &owner)
	{
		netlist_matrix_solver_t::setup(nets, owner);
		m_fallback.m_params = m_params;
		m_fallback.setup(nets, owner);
	}

	ATTR_HOT int solve_non_dynamic();

	ATTR_HOT inline const int N() const { if (m_N == 0) return m_nets.count(); else return m_N; }

	ATTR_COLD virtual void reset()
	{
		netlist_matrix_solver_t::reset();
		m_fallback.reset();
	}

private:
	netlist_matrix_solver_direct_t<m_N, _storage_N> m_fallback;
};

class netlist_matrix_solver_direct1_t: public netlist_matrix_solver_direct_t<1,1>
{
public:
	ATTR_HOT int solve_non_dynamic();
private:
};

class netlist_matrix_solver_direct2_t: public netlist_matrix_solver_direct_t<2,2>
{
public:
	ATTR_HOT int solve_non_dynamic();
private:
};

NETLIB_DEVICE_WITH_PARAMS(solver,
		typedef netlist_core_device_t::list_t dev_list_t;

		netlist_ttl_input_t m_fb_sync;
		netlist_ttl_output_t m_Q_sync;

		netlist_ttl_input_t m_fb_step;
		netlist_ttl_output_t m_Q_step;

		netlist_param_double_t m_freq;
		netlist_param_double_t m_sync_delay;
		netlist_param_double_t m_accuracy;
		netlist_param_double_t m_convergence;
		netlist_param_double_t m_gmin;
		netlist_param_int_t m_resched_loops;
		netlist_param_int_t m_parallel;

		netlist_time m_inc;
		netlist_time m_last_step;
		netlist_time m_nt_sync_delay;

		netlist_matrix_solver_t::list_t m_mat_solvers;
public:

		ATTR_COLD ~NETLIB_NAME(solver)();

		ATTR_HOT inline void schedule();

		ATTR_COLD void post_start();

		ATTR_HOT inline double gmin() { return m_gmin.Value(); }
);

ATTR_HOT inline void NETLIB_NAME(solver)::schedule()
{
	if (!m_Q_sync.net().is_queued())
		m_Q_sync.net().push_to_queue(m_nt_sync_delay);
}

ATTR_HOT inline const NETLIB_NAME(solver) &netlist_matrix_solver_t::owner() const
{
	return *m_owner;
}


#endif /* NLD_SOLVER_H_ */
