/*
 * nld_solver.c
 *
 */

/* Commented out for now. Relatively low number of terminals / nets make
 * the vectorizations fast-math enables pretty expensive
 */

#if 0
#pragma GCC optimize "-ffast-math"
//#pragma GCC optimize "-funroll-loops"
#pragma GCC optimize "-funswitch-loops"
#pragma GCC optimize "-fvariable-expansion-in-unroller"
#pragma GCC optimize "-funsafe-loop-optimizations"
#pragma GCC optimize "-fvect-cost-model"
#pragma GCC optimize "-fvariable-expansion-in-unroller"
#pragma GCC optimize "-ftree-loop-if-convert-stores"
#pragma GCC optimize "-ftree-loop-distribution"
#pragma GCC optimize "-ftree-loop-im"
#pragma GCC optimize "-ftree-loop-ivcanon"
#pragma GCC optimize "-fivopts"
#pragma GCC optimize "-ftree-parallelize-loops=4"
#endif

#define SOLVER_VERBOSE_OUT(x) do {} while (0)
//#define SOLVER_VERBOSE_OUT(x) printf x

#include <algorithm>
#include "nld_solver.h"
#include "nld_ms_direct.h"
#include "nld_ms_direct1.h"
#include "nld_ms_direct2.h"
#include "nld_ms_gauss_seidel.h"
#include "nld_twoterm.h"
#include "../nl_lists.h"

#if HAS_OPENMP
#include "omp.h"
#endif

vector_ops_t *vector_ops_t::create_ops(const int size)
{
	switch (size)
	{
		case 1:
			return nl_alloc(vector_ops_impl_t<1>);
		case 2:
			return nl_alloc(vector_ops_impl_t<2>);
		case 3:
			return nl_alloc(vector_ops_impl_t<3>);
		case 4:
			return nl_alloc(vector_ops_impl_t<4>);
		case 5:
			return nl_alloc(vector_ops_impl_t<5>);
		case 6:
			return nl_alloc(vector_ops_impl_t<6>);
		case 7:
			return nl_alloc(vector_ops_impl_t<7>);
		case 8:
			return nl_alloc(vector_ops_impl_t<8>);
		case 9:
			return nl_alloc(vector_ops_impl_t<9>);
		case 10:
			return nl_alloc(vector_ops_impl_t<10>);
		case 11:
			return nl_alloc(vector_ops_impl_t<11>);
		case 12:
			return nl_alloc(vector_ops_impl_t<12>);
		default:
			return nl_alloc(vector_ops_impl_t<0>, size);
	}
}

ATTR_COLD void terms_t::add(netlist_terminal_t *term, int net_other)
{
	m_term.add(term);
	m_net_other.add(net_other);
	m_gt.add(0.0);
	m_go.add(0.0);
	m_Idr.add(0.0);
	m_other_curanalog.add(NULL);
}

ATTR_COLD void terms_t::set_pointers()
{
	for (int i = 0; i < count(); i++)
	{
		m_term[i]->m_gt1 = &m_gt[i];
		m_term[i]->m_go1 = &m_go[i];
		m_term[i]->m_Idr1 = &m_Idr[i];
		m_other_curanalog[i] = &m_term[i]->m_otherterm->net().as_analog().m_cur_Analog;
	}
}

// ----------------------------------------------------------------------------------------
// netlist_matrix_solver
// ----------------------------------------------------------------------------------------

ATTR_COLD netlist_matrix_solver_t::netlist_matrix_solver_t(const eSolverType type, const netlist_solver_parameters_t &params)
: m_stat_calculations(0),
	m_stat_newton_raphson(0),
	m_stat_vsolver_calls(0),
	m_params(params),
	m_cur_ts(0),
	m_type(type)
{
}

ATTR_COLD netlist_matrix_solver_t::~netlist_matrix_solver_t()
{
	for (int i = 0; i < m_inps.count(); i++)
		global_free(m_inps[i]);
}

ATTR_COLD void netlist_matrix_solver_t::setup(netlist_analog_net_t::list_t &nets)
{
	NL_VERBOSE_OUT(("New solver setup\n"));

	m_nets.clear();

	for (int k = 0; k < nets.count(); k++)
	{
		m_nets.add(nets[k]);
	}

	for (int k = 0; k < nets.count(); k++)
	{
		NL_VERBOSE_OUT(("setting up net\n"));

		netlist_analog_net_t *net = nets[k];

		net->m_solver = this;

		for (int i = 0; i < net->m_core_terms.count(); i++)
		{
			netlist_core_terminal_t *p = net->m_core_terms[i];
			NL_VERBOSE_OUT(("%s %s %d\n", p->name().cstr(), net->name().cstr(), (int) net->isRailNet()));
			switch (p->type())
			{
				case netlist_terminal_t::TERMINAL:
					switch (p->netdev().family())
					{
						case netlist_device_t::CAPACITOR:
							if (!m_step_devices.contains(&p->netdev()))
								m_step_devices.add(&p->netdev());
							break;
						case netlist_device_t::BJT_EB:
						case netlist_device_t::DIODE:
						//case netlist_device_t::VCVS:
						case netlist_device_t::BJT_SWITCH:
							NL_VERBOSE_OUT(("found BJT/Diode\n"));
							if (!m_dynamic_devices.contains(&p->netdev()))
								m_dynamic_devices.add(&p->netdev());
							break;
						default:
							break;
					}
					{
						netlist_terminal_t *pterm = dynamic_cast<netlist_terminal_t *>(p);
						add_term(k, pterm);
					}
					NL_VERBOSE_OUT(("Added terminal\n"));
					break;
				case netlist_terminal_t::INPUT:
					{
						netlist_analog_output_t *net_proxy_output = NULL;
						for (int i = 0; i < m_inps.count(); i++)
							if (m_inps[i]->m_proxied_net == &p->net().as_analog())
							{
								net_proxy_output = m_inps[i];
								break;
							}

						if (net_proxy_output == NULL)
						{
							net_proxy_output = nl_alloc(netlist_analog_output_t);
							net_proxy_output->init_object(*this, this->name() + "." + pstring::sprintf("m%d", m_inps.count()));
							m_inps.add(net_proxy_output);
							net_proxy_output->m_proxied_net = &p->net().as_analog();
						}
						net_proxy_output->net().register_con(*p);
						// FIXME: repeated
						net_proxy_output->net().rebuild_list();
						NL_VERBOSE_OUT(("Added input\n"));
					}
					break;
				default:
					netlist().error("unhandled element found\n");
					break;
			}
		}
		NL_VERBOSE_OUT(("added net with %d populated connections\n", net->m_core_terms.count()));
	}
}


ATTR_HOT void netlist_matrix_solver_t::update_inputs()
{
	// avoid recursive calls. Inputs are updated outside this call
	for (netlist_analog_output_t * const *p = m_inps.first(); p != NULL; p = m_inps.next(p))
		(*p)->set_Q((*p)->m_proxied_net->m_cur_Analog);

}


ATTR_HOT void netlist_matrix_solver_t::update_dynamic()
{
	/* update all non-linear devices  */
	for (netlist_core_device_t * const *p = m_dynamic_devices.first(); p != NULL; p = m_dynamic_devices.next(p))
		switch ((*p)->family())
		{
			case netlist_device_t::DIODE:
				static_cast<NETLIB_NAME(D) *>((*p))->update_terminals();
				break;
			default:
				(*p)->update_terminals();
				break;
		}
}

ATTR_COLD void netlist_matrix_solver_t::start()
{
	register_output("Q_sync", m_Q_sync);
	register_input("FB_sync", m_fb_sync);
	connect(m_fb_sync, m_Q_sync);
}

ATTR_COLD void netlist_matrix_solver_t::reset()
{
	m_last_step = netlist_time::zero;
}

ATTR_COLD void netlist_matrix_solver_t::update()
{
	const nl_double new_timestep = solve();

	if (m_params.m_dynamic && is_timestep() && new_timestep > 0)
		m_Q_sync.net().reschedule_in_queue(netlist_time::from_double(new_timestep));
}

ATTR_COLD void netlist_matrix_solver_t::update_forced()
{
	ATTR_UNUSED const nl_double new_timestep = solve();

	if (m_params.m_dynamic && is_timestep())
		m_Q_sync.net().reschedule_in_queue(netlist_time::from_double(m_params.m_min_timestep));
}

ATTR_HOT void netlist_matrix_solver_t::step(const netlist_time delta)
{
	const nl_double dd = delta.as_double();
	for (int k=0; k < m_step_devices.count(); k++)
		m_step_devices[k]->step_time(dd);
}

template<class C >
void netlist_matrix_solver_t::solve_base(C *p)
{
	m_stat_vsolver_calls++;
	if (is_dynamic())
	{
		int this_resched;
		int newton_loops = 0;
		do
		{
			update_dynamic();
			// Gauss-Seidel will revert to Gaussian elemination if steps exceeded.
			this_resched = p->vsolve_non_dynamic();
			newton_loops++;
		} while (this_resched > 1 && newton_loops < m_params.m_nr_loops);

		m_stat_newton_raphson += newton_loops;
		// reschedule ....
		if (this_resched > 1 && !m_Q_sync.net().is_queued())
		{
			netlist().warning("NEWTON_LOOPS exceeded ... reschedule");
			m_Q_sync.net().reschedule_in_queue(m_params.m_nt_sync_delay);
		}
	}
	else
	{
		p->vsolve_non_dynamic();
	}
}

ATTR_HOT nl_double netlist_matrix_solver_t::solve()
{
	netlist_time now = netlist().time();
	netlist_time delta = now - m_last_step;

	// We are already up to date. Avoid oscillations.
	// FIXME: Make this a parameter!
	if (delta < netlist_time::from_nsec(1))
		return -1.0;

	/* update all terminals for new time step */
	m_last_step = now;
	m_cur_ts = delta.as_double();

	step(delta);

	const nl_double next_time_step = vsolve();

	update_inputs();
	return next_time_step;
}


// ----------------------------------------------------------------------------------------
// netlist_matrix_solver - Direct base
// ----------------------------------------------------------------------------------------

ATTR_COLD int netlist_matrix_solver_t::get_net_idx(netlist_net_t *net)
{
	for (int k = 0; k < m_nets.count(); k++)
		if (m_nets[k] == net)
			return k;
	return -1;
}







// ----------------------------------------------------------------------------------------
// solver
// ----------------------------------------------------------------------------------------



NETLIB_START(solver)
{
	register_output("Q_step", m_Q_step);

	register_param("SYNC_DELAY", m_sync_delay, NLTIME_FROM_NS(10).as_double());

	register_param("FREQ", m_freq, 48000.0);

	register_param("ACCURACY", m_accuracy, 1e-7);
	register_param("GS_LOOPS", m_gs_loops, 9);              // Gauss-Seidel loops
	register_param("GS_THRESHOLD", m_gs_threshold, 5);      // below this value, gaussian elimination is used
	register_param("NR_LOOPS", m_nr_loops, 25);             // Newton-Raphson loops
	register_param("PARALLEL", m_parallel, 0);
	register_param("SOR_FACTOR", m_sor, 1.059);
	register_param("GMIN", m_gmin, NETLIST_GMIN_DEFAULT);
	register_param("DYNAMIC_TS", m_dynamic, 0);
	register_param("LTE", m_lte, 5e-5);                     // diff/timestep
	register_param("MIN_TIMESTEP", m_min_timestep, 1e-6);   // nl_double timestep resolution

	// internal staff

	register_input("FB_step", m_fb_step);
	connect(m_fb_step, m_Q_step);

}

NETLIB_RESET(solver)
{
	for (int i = 0; i < m_mat_solvers.count(); i++)
		m_mat_solvers[i]->reset();
}


NETLIB_UPDATE_PARAM(solver)
{
	//m_inc = netlist_time::from_hz(m_freq.Value());
}

NETLIB_NAME(solver)::~NETLIB_NAME(solver)()
{
	for (int i = 0; i < m_mat_solvers.count(); i++)
		m_mat_solvers[i]->log_stats();

	netlist_matrix_solver_t * const *e = m_mat_solvers.first();
	while (e != NULL)
	{
		netlist_matrix_solver_t * const *en = m_mat_solvers.next(e);
		global_free(*e);
		e = en;
	}

}

NETLIB_UPDATE(solver)
{
	if (m_params.m_dynamic)
		return;

	const int t_cnt = m_mat_solvers.count();

#if HAS_OPENMP && USE_OPENMP
	if (m_parallel.Value())
	{
		omp_set_num_threads(4);
		omp_set_dynamic(0);
		#pragma omp parallel
		{
			#pragma omp for nowait
			for (int i = 0; i <  t_cnt; i++)
			{
				this_resched[i] = m_mat_solvers[i]->solve();
			}
		}
	}
	else
		for (int i = 0; i < t_cnt; i++)
		{
			if (do_full || (m_mat_solvers[i]->is_timestep()))
				this_resched[i] = m_mat_solvers[i]->solve();
		}
#else
	for (int i = 0; i < t_cnt; i++)
	{
		if (m_mat_solvers[i]->is_timestep())
			{
				// Ignore return value
				ATTR_UNUSED const nl_double ts = m_mat_solvers[i]->solve();
			}
	}
#endif

	/* step circuit */
	if (!m_Q_step.net().is_queued())
	{
		m_Q_step.net().push_to_queue(netlist_time::from_double(m_params.m_max_timestep));
	}
}

template <int m_N, int _storage_N>
netlist_matrix_solver_t * NETLIB_NAME(solver)::create_solver(int size, const int gs_threshold, const bool use_specific)
{
	if (use_specific && m_N == 1)
		return nl_alloc(netlist_matrix_solver_direct1_t, m_params);
	else if (use_specific && m_N == 2)
		return nl_alloc(netlist_matrix_solver_direct2_t, m_params);
	else
	{
		typedef netlist_matrix_solver_gauss_seidel_t<m_N,_storage_N> solver_N;
		if (size >= gs_threshold)
			return nl_alloc(solver_N, m_params, size);
		else
			return nl_alloc(solver_N, m_params, size);
	}
}

ATTR_COLD void NETLIB_NAME(solver)::post_start()
{
	netlist_analog_net_t::list_t groups[100];
	int cur_group = -1;
	const int gs_threshold = m_gs_threshold.Value();
	const bool use_specific = true;

	m_params.m_accuracy = m_accuracy.Value();
	m_params.m_gs_loops = m_gs_loops.Value();
	m_params.m_nr_loops = m_nr_loops.Value();
	m_params.m_nt_sync_delay = m_sync_delay.Value();
	m_params.m_lte = m_lte.Value();
	m_params.m_sor = m_sor.Value();

	m_params.m_min_timestep = m_min_timestep.Value();
	m_params.m_dynamic = (m_dynamic.Value() == 1 ? true : false);
	m_params.m_max_timestep = netlist_time::from_hz(m_freq.Value()).as_double();

	if (m_params.m_dynamic)
	{
		m_params.m_max_timestep *= 1000.0;
	}
	else
	{
		m_params.m_min_timestep = m_params.m_max_timestep;
	}

	netlist().log("Scanning net groups ...");
	// determine net groups
	for (netlist_net_t * const *pn = netlist().m_nets.first(); pn != NULL; pn = netlist().m_nets.next(pn))
	{
		SOLVER_VERBOSE_OUT(("processing %s\n", (*pn)->name().cstr()));
		if (!(*pn)->isRailNet())
		{
			SOLVER_VERBOSE_OUT(("   ==> not a rail net\n"));
			netlist_analog_net_t *n = &(*pn)->as_analog();
			if (!n->already_processed(groups, cur_group))
			{
				cur_group++;
				n->process_net(groups, cur_group);
			}
		}
	}

	// setup the solvers
	netlist().log("Found %d net groups in %d nets\n", cur_group + 1, netlist().m_nets.count());
	for (int i = 0; i <= cur_group; i++)
	{
		netlist_matrix_solver_t *ms;
		int net_count = groups[i].count();

		switch (net_count)
		{
			case 1:
				ms = create_solver<1,1>(1, gs_threshold, use_specific);
				break;
			case 2:
				ms = create_solver<2,2>(2, gs_threshold, use_specific);
				break;
			case 3:
				ms = create_solver<3,3>(3, gs_threshold, use_specific);
				break;
			case 4:
				ms = create_solver<4,4>(4, gs_threshold, use_specific);
				break;
			case 5:
				ms = create_solver<5,5>(5, gs_threshold, use_specific);
				break;
			case 6:
				ms = create_solver<6,6>(6, gs_threshold, use_specific);
				break;
			case 7:
				ms = create_solver<7,7>(7, gs_threshold, use_specific);
				break;
			case 8:
				ms = create_solver<8,8>(8, gs_threshold, use_specific);
				break;
			case 12:
				ms = create_solver<12,12>(12, gs_threshold, use_specific);
				break;
			default:
				if (net_count <= 16)
				{
					ms = create_solver<0,16>(net_count, gs_threshold, use_specific);
				}
				else if (net_count <= 32)
				{
					ms = create_solver<0,32>(net_count, gs_threshold, use_specific);
				}
				else if (net_count <= 64)
				{
					ms = create_solver<0,64>(net_count, gs_threshold, use_specific);
				}
				else
				{
					netlist().error("Encountered netgroup with > 64 nets");
					ms = NULL; /* tease compilers */
				}

				break;
		}

		register_sub(*ms, pstring::sprintf("Solver %d",m_mat_solvers.count()));

		ms->vsetup(groups[i]);

		m_mat_solvers.add(ms);

		netlist().log("Solver %s", ms->name().cstr());
		netlist().log("       # %d ==> %d nets", i, groups[i].count()); //, (*(*groups[i].first())->m_core_terms.first())->name().cstr());
		netlist().log("       has %s elements", ms->is_dynamic() ? "dynamic" : "no dynamic");
		netlist().log("       has %s elements", ms->is_timestep() ? "timestep" : "no timestep");
		for (int j=0; j<groups[i].count(); j++)
		{
			netlist().log("Net %d: %s", j, groups[i][j]->name().cstr());
			netlist_net_t *n = groups[i][j];
			for (int k = 0; k < n->m_core_terms.count(); k++)
			{
				const netlist_core_terminal_t *p = n->m_core_terms[k];
				netlist().log("   %s", p->name().cstr());
			}
		}
	}
}
