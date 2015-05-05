/*
 * nld_solver.h
 *
 */

#ifndef NLD_SOLVER_H_
#define NLD_SOLVER_H_

#include "../nl_setup.h"
#include "../nl_base.h"

//#define ATTR_ALIGNED(N) __attribute__((aligned(N)))
#define ATTR_ALIGNED(N) ATTR_ALIGN

#define USE_PIVOT_SEARCH (0)
#define VECTALT 1
#define USE_GABS 1
#define USE_MATRIX_GS 0
// savings are eaten up by effort
#define USE_LINEAR_PREDICTION (0)

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
	nl_double m_accuracy;
	nl_double m_lte;
	nl_double m_min_timestep;
	nl_double m_max_timestep;
	nl_double m_sor;
	bool m_dynamic;
	int m_gs_loops;
	int m_nr_loops;
	netlist_time m_nt_sync_delay;
};


class ATTR_ALIGNED(64) terms_t
{
	NETLIST_PREVENT_COPYING(terms_t)

	public:
	ATTR_COLD terms_t() : m_railstart(0)
	{}

	ATTR_COLD void clear()
	{
		m_term.clear();
		m_net_other.clear();
		m_gt.clear();
	}

	ATTR_COLD void add(netlist_terminal_t *term, int net_other);

	ATTR_HOT inline int count() { return m_term.count(); }

	ATTR_HOT inline netlist_terminal_t **terms() { return m_term; }
	ATTR_HOT inline int *net_other() { return m_net_other; }
	ATTR_HOT inline nl_double *gt() { return m_gt; }
	ATTR_HOT inline nl_double *go() { return m_go; }
	ATTR_HOT inline nl_double *Idr() { return m_Idr; }
	ATTR_HOT inline nl_double **other_curanalog() { return m_other_curanalog; }

	ATTR_COLD void set_pointers();

	int m_railstart;

private:
	plinearlist_t<netlist_terminal_t *> m_term;
	plinearlist_t<int> m_net_other;
	plinearlist_t<nl_double> m_go;
	plinearlist_t<nl_double> m_gt;
	plinearlist_t<nl_double> m_Idr;
	plinearlist_t<nl_double *> m_other_curanalog;
};

class netlist_matrix_solver_t : public netlist_device_t
{
public:
	typedef plinearlist_t<netlist_matrix_solver_t *> list_t;
	typedef netlist_core_device_t::list_t dev_list_t;

	enum eSolverType
	{
		GAUSSIAN_ELIMINATION,
		GAUSS_SEIDEL
	};

	ATTR_COLD netlist_matrix_solver_t(const eSolverType type, const netlist_solver_parameters_t &params);
	ATTR_COLD virtual ~netlist_matrix_solver_t();

	ATTR_COLD virtual void vsetup(netlist_analog_net_t::list_t &nets) = 0;

	template<class C>
	void solve_base(C *p);

	ATTR_HOT nl_double solve();

	ATTR_HOT inline bool is_dynamic() { return m_dynamic_devices.count() > 0; }
	ATTR_HOT inline bool is_timestep() { return m_step_devices.count() > 0; }

	ATTR_HOT void update_forced();
	ATTR_HOT inline void update_after(const netlist_time after)
	{
		m_Q_sync.net().reschedule_in_queue(after);
	}

	/* netdevice functions */
	ATTR_HOT  virtual void update();
	ATTR_COLD virtual void start();
	ATTR_COLD virtual void reset();

	ATTR_COLD int get_net_idx(netlist_net_t *net);
	ATTR_COLD virtual void log_stats() {};

	inline const eSolverType type() const { return m_type; }

protected:

	ATTR_COLD void setup(netlist_analog_net_t::list_t &nets);
	ATTR_HOT void update_dynamic();

	// should return next time step
	ATTR_HOT virtual nl_double vsolve() = 0;

	ATTR_COLD virtual void  add_term(int net_idx, netlist_terminal_t *term) = 0;

	plinearlist_t<netlist_analog_net_t *> m_nets;
	plinearlist_t<netlist_analog_output_t *> m_inps;

	int m_stat_calculations;
	int m_stat_newton_raphson;
	int m_stat_vsolver_calls;

	const netlist_solver_parameters_t &m_params;

	ATTR_HOT inline const nl_double current_timestep() { return m_cur_ts; }
private:

	netlist_time m_last_step;
	nl_double m_cur_ts;
	dev_list_t m_step_devices;
	dev_list_t m_dynamic_devices;

	netlist_ttl_input_t m_fb_sync;
	netlist_ttl_output_t m_Q_sync;

	ATTR_HOT void step(const netlist_time delta);

	ATTR_HOT void update_inputs();

	const eSolverType m_type;
};



class ATTR_ALIGNED(64) NETLIB_NAME(solver) : public netlist_device_t
{
public:
	NETLIB_NAME(solver)()
	: netlist_device_t()    { }

	ATTR_COLD virtual ~NETLIB_NAME(solver)();

	ATTR_COLD void post_start();

	ATTR_HOT inline nl_double gmin() { return m_gmin.Value(); }

protected:
	ATTR_HOT void update();
	ATTR_HOT void start();
	ATTR_HOT void reset();
	ATTR_HOT void update_param();

	netlist_ttl_input_t m_fb_step;
	netlist_ttl_output_t m_Q_step;

	netlist_param_double_t m_freq;
	netlist_param_double_t m_sync_delay;
	netlist_param_double_t m_accuracy;
	netlist_param_double_t m_gmin;
	netlist_param_double_t m_lte;
	netlist_param_double_t m_sor;
	netlist_param_logic_t  m_dynamic;
	netlist_param_double_t m_min_timestep;

	netlist_param_int_t m_nr_loops;
	netlist_param_int_t m_gs_loops;
	netlist_param_int_t m_gs_threshold;
	netlist_param_int_t m_parallel;

	netlist_matrix_solver_t::list_t m_mat_solvers;
private:

	netlist_solver_parameters_t m_params;

	template <int m_N, int _storage_N>
	netlist_matrix_solver_t *create_solver(int size, int gs_threshold, bool use_specific);
};



#endif /* NLD_SOLVER_H_ */
