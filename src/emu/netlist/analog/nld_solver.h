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
//#undef RESTRICT
//#define RESTRICT


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

class vector_ops_t
{
public:

    vector_ops_t(int size)
    : m_dim(size)
    {
    }

    virtual ~vector_ops_t() {}

    ATTR_ALIGNED(64) double * RESTRICT  m_V;

    virtual const double sum(const double * v) = 0;
    virtual void sum2(const double * RESTRICT v1, const double * RESTRICT v2, double & RESTRICT  s1, double & RESTRICT s2) = 0;
    virtual void addmult(double * RESTRICT v1, const double * RESTRICT v2, const double &mult) = 0;
    virtual void sum2a(const double * RESTRICT v1, const double * RESTRICT v2, const double * RESTRICT v3abs, double & RESTRICT s1, double & RESTRICT s2, double & RESTRICT s3abs) = 0;

    virtual const double sumabs(const double * v) = 0;

protected:
    int m_dim;

private:

};

template <int m_N>
class vector_ops_impl_t : public vector_ops_t
{
public:

    vector_ops_impl_t()
    : vector_ops_t(m_N)
    {
    }

    vector_ops_impl_t(int size)
    : vector_ops_t(size)
    {
        assert(m_N == 0);
    }

    virtual ~vector_ops_impl_t() {}

    ATTR_HOT inline const int N() const { if (m_N == 0) return m_dim; else return m_N; }

    const double sum(const double * v)
    {
        const double *  RESTRICT vl = v;
        double tmp = 0.0;
        for (int i=0; i < N(); i++)
            tmp += vl[i];
        return tmp;
    }

    void sum2(const double * RESTRICT v1, const double * RESTRICT v2, double & RESTRICT s1, double & RESTRICT s2)
    {
        const double * RESTRICT v1l = v1;
        const double * RESTRICT v2l = v2;
        for (int i=0; i < N(); i++)
        {
            s1 += v1l[i];
            s2 += v2l[i];
        }
    }

    void addmult(double * RESTRICT v1, const double * RESTRICT v2, const double &mult)
    {
        double * RESTRICT v1l = v1;
        const double * RESTRICT v2l = v2;
        for (int i=0; i < N(); i++)
        {
            v1l[i] += v2l[i] * mult;
        }
    }

    void sum2a(const double * RESTRICT v1, const double * RESTRICT v2, const double * RESTRICT v3abs, double & RESTRICT s1, double & RESTRICT s2, double & RESTRICT s3abs)
    {
        const double * RESTRICT v1l = v1;
        const double * RESTRICT v2l = v2;
        const double * RESTRICT v3l = v3abs;
        for (int i=0; i < N(); i++)
        {
            s1 += v1l[i];
            s2 += v2l[i];
            s3abs += fabs(v3l[i]);
        }
    }

    const double sumabs(const double * v)
    {
        const double * RESTRICT vl = v;
        double tmp = 0.0;
        for (int i=0; i < N(); i++)
            tmp += fabs(vl[i]);
        return tmp;
    }

private:
};

class ATTR_ALIGNED(64) terms_t
{
    NETLIST_PREVENT_COPYING(terms_t)

    public:
    ATTR_COLD terms_t() {}

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
    ATTR_HOT inline double *gt() { return m_gt; }
    ATTR_HOT inline double *go() { return m_go; }
    ATTR_HOT inline double *Idr() { return m_Idr; }
    ATTR_HOT inline double **other_curanalog() { return m_other_curanalog; }
    ATTR_HOT vector_ops_t *ops() { return m_ops; }

    ATTR_COLD void set_pointers();

    int m_railstart;

private:
    plinearlist_t<netlist_terminal_t *> m_term;
    plinearlist_t<int> m_net_other;
    plinearlist_t<double> m_gt;
    plinearlist_t<double> m_go;
    plinearlist_t<double> m_Idr;
    plinearlist_t<double *> m_other_curanalog;
    vector_ops_t * m_ops;
};

class netlist_matrix_solver_t : public netlist_device_t
{
public:
	typedef plinearlist_t<netlist_matrix_solver_t *> list_t;
	typedef netlist_core_device_t::list_t dev_list_t;

	ATTR_COLD netlist_matrix_solver_t();
	ATTR_COLD virtual ~netlist_matrix_solver_t();

    ATTR_COLD virtual void vsetup(netlist_analog_net_t::list_t &nets) = 0;

    template<class C>
    void solve_base(C *p);

	ATTR_HOT double solve();

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

	netlist_solver_parameters_t m_params;

    ATTR_COLD int get_net_idx(netlist_net_t *net);
	ATTR_COLD virtual void log_stats() {};

protected:

    ATTR_COLD void setup(netlist_analog_net_t::list_t &nets);
    ATTR_HOT void update_dynamic();

    // should return next time step
    ATTR_HOT virtual double vsolve() = 0;

    ATTR_COLD virtual void  add_term(int net_idx, netlist_terminal_t *term) = 0;

    plinearlist_t<netlist_analog_net_t *> m_nets;
    plinearlist_t<netlist_analog_output_t *> m_inps;

    int m_calculations;

    ATTR_HOT inline const double current_timestep() { return m_cur_ts; }
private:

    netlist_time m_last_step;
    double m_cur_ts;
    dev_list_t m_step_devices;
    dev_list_t m_dynamic_devices;

    netlist_ttl_input_t m_fb_sync;
    netlist_ttl_output_t m_Q_sync;

    ATTR_HOT void step(const netlist_time delta);

    ATTR_HOT void update_inputs();

};

template <int m_N, int _storage_N>
class netlist_matrix_solver_direct_t: public netlist_matrix_solver_t
{
public:

	netlist_matrix_solver_direct_t(int size);

	virtual ~netlist_matrix_solver_direct_t();

	ATTR_COLD virtual void vsetup(netlist_analog_net_t::list_t &nets);
	ATTR_COLD virtual void reset() { netlist_matrix_solver_t::reset(); }

	ATTR_HOT inline const int N() const { if (m_N == 0) return m_dim; else return m_N; }

    ATTR_HOT inline int vsolve_non_dynamic();

protected:
    ATTR_COLD virtual void add_term(int net_idx, netlist_terminal_t *term);

    ATTR_HOT virtual double vsolve();

    ATTR_HOT int solve_non_dynamic();
	ATTR_HOT void build_LE();
	ATTR_HOT void gauss_LE(double (* RESTRICT x));
	ATTR_HOT double delta(const double (* RESTRICT V));
	ATTR_HOT void store(const double (* RESTRICT V), const bool store_RHS);

    /* bring the whole system to the current time
     * Don't schedule a new calculation time. The recalculation has to be
     * triggered by the caller after the netlist element was changed.
     */
    ATTR_HOT double compute_next_timestep();

    double m_A[_storage_N][((_storage_N + 7) / 8) * 8];
    double m_RHS[_storage_N];
    double m_last_RHS[_storage_N]; // right hand side - contains currents
    double m_Vdelta[_storage_N];
    double m_last_V[_storage_N];

    terms_t **m_terms;

    terms_t *m_rails_temp;

private:
    vector_ops_t *m_row_ops[_storage_N + 1];

	int m_dim;
	double m_lp_fact;
};

template <int m_N, int _storage_N>
class ATTR_ALIGNED(64) netlist_matrix_solver_gauss_seidel_t: public netlist_matrix_solver_direct_t<m_N, _storage_N>
{
public:

	netlist_matrix_solver_gauss_seidel_t(int size)
      : netlist_matrix_solver_direct_t<m_N, _storage_N>(size)
      , m_lp_fact(0)
      , m_gs_fail(0)
      , m_gs_total(0)
      {}

	virtual ~netlist_matrix_solver_gauss_seidel_t() {}

    ATTR_COLD virtual void log_stats();

    ATTR_HOT inline int vsolve_non_dynamic();
protected:
    ATTR_HOT virtual double vsolve();

private:
    double m_lp_fact;
    int m_gs_fail;
    int m_gs_total;

};

class ATTR_ALIGNED(64) netlist_matrix_solver_direct1_t: public netlist_matrix_solver_direct_t<1,1>
{
public:

    netlist_matrix_solver_direct1_t()
      : netlist_matrix_solver_direct_t<1, 1>(1)
      {}
    ATTR_HOT inline int vsolve_non_dynamic();
protected:
    ATTR_HOT virtual double vsolve();
private:
};

class ATTR_ALIGNED(64) netlist_matrix_solver_direct2_t: public netlist_matrix_solver_direct_t<2,2>
{
public:

    netlist_matrix_solver_direct2_t()
      : netlist_matrix_solver_direct_t<2, 2>(2)
      {}
    ATTR_HOT inline int vsolve_non_dynamic();
protected:
    ATTR_HOT virtual double vsolve();
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
    netlist_matrix_solver_t *create_solver(int size, int gs_threshold, bool use_specific);
};



#endif /* NLD_SOLVER_H_ */
