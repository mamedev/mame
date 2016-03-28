// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_matrix_solver.h
 *
 */

#ifndef NLD_MATRIX_SOLVER_H_
#define NLD_MATRIX_SOLVER_H_

#include "solver/nld_solver.h"

NETLIB_NAMESPACE_DEVICES_START()

class terms_t
{
	P_PREVENT_COPYING(terms_t)

	public:
	ATTR_COLD terms_t() : m_railstart(0)
	{}

	ATTR_COLD void clear()
	{
		m_term.clear();
		m_net_other.clear();
		m_gt.clear();
		m_go.clear();
		m_Idr.clear();
		m_other_curanalog.clear();
	}

	ATTR_COLD void add(terminal_t *term, int net_other, bool sorted);

	inline unsigned count() { return m_term.size(); }

	inline terminal_t **terms() { return m_term.data(); }
	inline int *net_other() { return m_net_other.data(); }
	inline nl_double *gt() { return m_gt.data(); }
	inline nl_double *go() { return m_go.data(); }
	inline nl_double *Idr() { return m_Idr.data(); }
	inline nl_double **other_curanalog() { return m_other_curanalog.data(); }

	ATTR_COLD void set_pointers();

	unsigned m_railstart;

	pvector_t<unsigned> m_nz;   /* all non zero for multiplication */
	pvector_t<unsigned> m_nzrd; /* non zero right of the diagonal for elimination, may include RHS element */
	pvector_t<unsigned> m_nzbd; /* non zero below of the diagonal for elimination */
private:
	pvector_t<terminal_t *> m_term;
	pvector_t<int> m_net_other;
	pvector_t<nl_double> m_go;
	pvector_t<nl_double> m_gt;
	pvector_t<nl_double> m_Idr;
	pvector_t<nl_double *> m_other_curanalog;
};

class matrix_solver_t : public device_t
{
public:
	typedef pvector_t<matrix_solver_t *> list_t;
	typedef core_device_t::list_t dev_list_t;

	enum eSolverType
	{
		GAUSSIAN_ELIMINATION,
		GAUSS_SEIDEL
	};

	matrix_solver_t(const eSolverType type, const solver_parameters_t *params);
	virtual ~matrix_solver_t();

	void setup(analog_net_t::list_t &nets) { vsetup(nets); }

	netlist_time solve_base();

	netlist_time solve();

	inline bool is_dynamic() const { return m_dynamic_devices.size() > 0; }
	inline bool is_timestep() const { return m_step_devices.size() > 0; }

	void update_forced();
	void update_after(const netlist_time after)
	{
		m_Q_sync.net().reschedule_in_queue(after);
	}

	/* netdevice functions */
	virtual void update() override;
	virtual void start() override;
	virtual void reset() override;

	ATTR_COLD int get_net_idx(net_t *net);

	eSolverType type() const { return m_type; }
	plog_base<NL_DEBUG> &log() { return netlist().log(); }

	virtual void log_stats();

protected:

	ATTR_COLD void setup_base(analog_net_t::list_t &nets);
	void update_dynamic();

	virtual void  add_term(int net_idx, terminal_t *term) = 0;
	virtual void vsetup(analog_net_t::list_t &nets) = 0;
	virtual int vsolve_non_dynamic(const bool newton_raphson) = 0;
	virtual netlist_time compute_next_timestep() = 0;

	pvector_t<analog_net_t *> m_nets;
	pvector_t<analog_output_t *> m_inps;

	int m_stat_calculations;
	int m_stat_newton_raphson;
	int m_stat_vsolver_calls;
	int m_iterative_fail;
	int m_iterative_total;

	const solver_parameters_t &m_params;

	inline nl_double current_timestep() { return m_cur_ts; }
private:

	netlist_time m_last_step;
	nl_double m_cur_ts;
	dev_list_t m_step_devices;
	dev_list_t m_dynamic_devices;

	logic_input_t m_fb_sync;
	logic_output_t m_Q_sync;

	void step(const netlist_time &delta);

	void update_inputs();

	const eSolverType m_type;
};


NETLIB_NAMESPACE_DEVICES_END()

#endif /* NLD_MS_DIRECT_H_ */
