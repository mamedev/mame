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

#define NETDEV_SOLVER(_name)                                                        \
		NET_REGISTER_DEV(solver, _name)

// ----------------------------------------------------------------------------------------
// solver
// ----------------------------------------------------------------------------------------

class NETLIB_NAME(solver);

class netlist_matrix_solver_t
{
public:
	typedef netlist_list_t<netlist_matrix_solver_t *> list_t;
	typedef netlist_core_device_t::list_t dev_list_t;

	ATTR_COLD void setup(netlist_net_t::list_t &nets, NETLIB_NAME(solver) &owner);

	// return true if a reschedule is needed ...
	ATTR_HOT bool solve();
	ATTR_HOT void step(const netlist_time delta);
	ATTR_HOT void update_inputs();

	ATTR_HOT inline bool is_dynamic() { return m_dynamic.count() > 0; }
    ATTR_HOT inline bool is_timestep() { return m_steps.count() > 0; }

	ATTR_HOT inline const NETLIB_NAME(solver) &owner() const;

	double m_accuracy;
	double m_convergence_factor;
	int m_resched_loops;

private:
	netlist_net_t::list_t m_nets;
	dev_list_t m_dynamic;
	netlist_core_terminal_t::list_t m_inps;
	dev_list_t m_steps;

	NETLIB_NAME(solver) *m_owner;
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
        netlist_param_int_t m_resched_loops;

		netlist_time m_inc;
		netlist_time m_last_step;
		netlist_time m_nt_sync_delay;

		netlist_matrix_solver_t::list_t m_mat_solvers;
public:

		ATTR_COLD ~NETLIB_NAME(solver)();

		ATTR_HOT inline void schedule();

		ATTR_COLD void post_start();
		ATTR_COLD void reset()
		{
		    m_last_step = netlist_time::zero;
		}
);

ATTR_HOT inline void NETLIB_NAME(solver)::schedule()
{
	// FIXME: time should be parameter;
	if (!m_Q_sync.net().is_queued())
		m_Q_sync.net().push_to_queue(m_nt_sync_delay);
}

ATTR_HOT inline const NETLIB_NAME(solver) &netlist_matrix_solver_t::owner() const
{
	return *m_owner;
}


#endif /* NLD_SOLVER_H_ */
