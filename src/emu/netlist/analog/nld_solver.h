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
	double m_lte;
	double m_min_timestep;
	double m_max_timestep;
	bool m_dynamic;
	int m_gs_loops;
	int m_nr_loops;
    netlist_time m_nt_sync_delay;
};

class ATTR_ALIGNED(64) netlist_matrix_solver_t : public netlist_device_t
{
public:
	typedef plinearlist_t<netlist_matrix_solver_t *> list_t;
	typedef netlist_core_device_t::list_t dev_list_t;

	ATTR_COLD netlist_matrix_solver_t();
	ATTR_COLD virtual ~netlist_matrix_solver_t();

    ATTR_COLD virtual void vsetup(netlist_analog_net_t::list_t &nets) = 0;

	ATTR_HOT double solve();

	ATTR_HOT inline bool is_dynamic() { return m_dynamic.count() > 0; }
	ATTR_HOT inline bool is_timestep() { return m_steps.count() > 0; }

    ATTR_HOT void update_forced();

	/* netdevice functions */
	ATTR_HOT  virtual void update();
    ATTR_COLD virtual void start();
    ATTR_COLD virtual void reset();

	netlist_solver_parameters_t m_params;

    ATTR_COLD int get_net_idx(netlist_net_t *net);
	ATTR_COLD virtual void log_stats() {};

protected:

    class ATTR_ALIGNED(64) terms_t{

    public:
        terms_t() {}

        void clear()
        {
            m_term.clear();
            m_net_other.clear();
            m_gt.clear();
        }

        void add(netlist_terminal_t *term, int net_other)
        {
            m_term.add(term);
            m_net_other.add(net_other);
            m_gt.add(0.0);
            m_go.add(0.0);
            m_Idr.add(0.0);
        }

        inline int count() { return m_term.count(); }

        inline netlist_terminal_t **terms() { return m_term; }
        inline int *net_other() { return m_net_other; }
        inline double *gt() { return m_gt; }
        inline double *go() { return m_go; }
        inline double *Idr() { return m_Idr; }

        void set_pointers()
        {
            for (int i = 0; i < count(); i++)
            {
                m_term[i]->m_gt1 = &m_gt[i];
                m_term[i]->m_go1 = &m_go[i];
                m_term[i]->m_Idr1 = &m_Idr[i];
            }
        }

    private:
        plinearlist_t<netlist_terminal_t *> m_term;
        plinearlist_t<int> m_net_other;
        plinearlist_t<double> m_gt;
        plinearlist_t<double> m_go;
        plinearlist_t<double> m_Idr;

    };

    ATTR_COLD void setup(netlist_analog_net_t::list_t &nets);

    // return true if a reschedule is needed ...
    ATTR_HOT virtual int vsolve_non_dynamic() = 0;

    ATTR_COLD virtual void  add_term(int net_idx, netlist_terminal_t *term) = 0;
    int m_calculations;

    plinearlist_t<netlist_analog_net_t *> m_nets;
    plinearlist_t<netlist_analog_output_t *> m_inps;

private:

    netlist_time m_last_step;
    dev_list_t m_steps;
    dev_list_t m_dynamic;

    netlist_ttl_input_t m_fb_sync;
    netlist_ttl_output_t m_Q_sync;

    ATTR_HOT void step(const netlist_time delta);

    /* bring the whole system to the current time
     * Don't schedule a new calculation time. The recalculation has to be
     * triggered by the caller after the netlist element was changed.
     */
    ATTR_HOT virtual double compute_next_timestep(const double) = 0;

    ATTR_HOT void update_inputs();
    ATTR_HOT void update_dynamic();

};

template <int m_N, int _storage_N>
class ATTR_ALIGNED(64) netlist_matrix_solver_direct_t: public netlist_matrix_solver_t
{
public:

	netlist_matrix_solver_direct_t()
    : netlist_matrix_solver_t()
    , m_dim(0)
    {}

	virtual ~netlist_matrix_solver_direct_t() {}

	ATTR_COLD virtual void vsetup(netlist_analog_net_t::list_t &nets);
	ATTR_COLD virtual void reset() { netlist_matrix_solver_t::reset(); }

	ATTR_HOT inline const int N() const { if (m_N == 0) return m_dim; else return m_N; }

protected:
    ATTR_COLD virtual void add_term(int net_idx, netlist_terminal_t *term);

    ATTR_HOT virtual int vsolve_non_dynamic();
    ATTR_HOT int solve_non_dynamic();
	ATTR_HOT inline void build_LE();
	ATTR_HOT inline void gauss_LE(double (* RESTRICT x));
	ATTR_HOT inline double delta(
			const double (* RESTRICT V));
	ATTR_HOT inline void store(const double (* RESTRICT V), const bool store_RHS);

    ATTR_HOT virtual double compute_next_timestep(const double);

    ATTR_ALIGNED(64) double m_A[_storage_N][_storage_N];
    ATTR_ALIGNED(64) double m_RHS[_storage_N];
    ATTR_ALIGNED(64) double m_last_RHS[_storage_N]; // right hand side - contains currents

    terms_t m_terms[_storage_N];
    terms_t m_rails[_storage_N];
    plinearlist_t<double> xx[_storage_N];

private:

	int m_dim;
};

template <int m_N, int _storage_N>
class ATTR_ALIGNED(64) netlist_matrix_solver_gauss_seidel_t: public netlist_matrix_solver_direct_t<m_N, _storage_N>
{
public:

	netlist_matrix_solver_gauss_seidel_t()
      : netlist_matrix_solver_direct_t<m_N, _storage_N>()
      , m_gs_fail(0)
      , m_gs_total(0)
      {}

	virtual ~netlist_matrix_solver_gauss_seidel_t() {}

    ATTR_COLD virtual void log_stats();

protected:
	ATTR_HOT int vsolve_non_dynamic();

private:
    int m_gs_fail;
    int m_gs_total;

};

class ATTR_ALIGNED(64) netlist_matrix_solver_direct1_t: public netlist_matrix_solver_direct_t<1,1>
{
protected:
    ATTR_HOT int vsolve_non_dynamic();
private:
};

class ATTR_ALIGNED(64) netlist_matrix_solver_direct2_t: public netlist_matrix_solver_direct_t<2,2>
{
protected:
    ATTR_HOT int vsolve_non_dynamic();
private:
};

class ATTR_ALIGNED(64) NETLIB_NAME(solver) : public netlist_device_t
{
public:
    NETLIB_NAME(solver)()
    : netlist_device_t()    { }

    ATTR_COLD virtual ~NETLIB_NAME(solver)();

    ATTR_COLD void post_start();

    ATTR_HOT inline double gmin() { return m_gmin.Value(); }

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
    netlist_matrix_solver_t *create_solver(int gs_threshold, bool use_specific);
};



#endif /* NLD_SOLVER_H_ */
