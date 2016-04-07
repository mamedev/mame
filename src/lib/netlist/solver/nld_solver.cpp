// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_solver.c
 *
 */

/* Commented out for now. Relatively low number of terminals / nets make
 * the vectorizations fast-math enables pretty expensive
 */

#if 0
#pragma GCC optimize "-ffast-math"
#pragma GCC optimize "-fstrict-aliasing"
#pragma GCC optimize "-ftree-vectorizer-verbose=2"
#pragma GCC optimize "-fopt-info-vec"
#pragma GCC optimize "-fopt-info-vec-missed"
//#pragma GCC optimize "-ftree-parallelize-loops=4"
#pragma GCC optimize "-funroll-loops"
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
#endif

#include <iostream>
#include <algorithm>
//#include "nld_twoterm.h"
#include "nl_lists.h"

#if HAS_OPENMP
#include "omp.h"
#endif

#include "nld_solver.h"
#include "nld_matrix_solver.h"

#if 1
#include "nld_ms_direct.h"
#else
#include "nld_ms_direct_lu.h"
#endif
#include "nld_ms_w.h"
#include "nld_ms_sm.h"
#include "nld_ms_direct1.h"
#include "nld_ms_direct2.h"
#include "nld_ms_sor.h"
#include "nld_ms_sor_mat.h"
#include "nld_ms_gmres.h"

NETLIB_NAMESPACE_DEVICES_START()

ATTR_COLD void terms_t::add(terminal_t *term, int net_other, bool sorted)
{
	if (sorted)
		for (unsigned i=0; i < m_net_other.size(); i++)
		{
			if (m_net_other[i] > net_other)
			{
				m_term.insert_at(i, term);
				m_net_other.insert_at(i, net_other);
				m_gt.insert_at(i, 0.0);
				m_go.insert_at(i, 0.0);
				m_Idr.insert_at(i, 0.0);
				m_other_curanalog.insert_at(i, NULL);
				return;
			}
		}
	m_term.push_back(term);
	m_net_other.push_back(net_other);
	m_gt.push_back(0.0);
	m_go.push_back(0.0);
	m_Idr.push_back(0.0);
	m_other_curanalog.push_back(NULL);
}

ATTR_COLD void terms_t::set_pointers()
{
	for (unsigned i = 0; i < count(); i++)
	{
		m_term[i]->set_ptrs(&m_gt[i], &m_go[i], &m_Idr[i]);
		m_other_curanalog[i] = &m_term[i]->m_otherterm->net().m_cur_Analog;
	}
}

// ----------------------------------------------------------------------------------------
// matrix_solver
// ----------------------------------------------------------------------------------------

ATTR_COLD matrix_solver_t::matrix_solver_t(const eSolverType type, const solver_parameters_t *params)
: m_stat_calculations(0),
	m_stat_newton_raphson(0),
	m_stat_vsolver_calls(0),
	m_iterative_fail(0),
	m_iterative_total(0),
	m_params(*params),
	m_cur_ts(0),
	m_type(type)
{
}

ATTR_COLD matrix_solver_t::~matrix_solver_t()
{
	m_inps.clear_and_free();
}

ATTR_COLD void matrix_solver_t::setup_base(analog_net_t::list_t &nets)
{
	log().debug("New solver setup\n");

	m_nets.clear();

	for (auto & net : nets)
		m_nets.push_back(net);

	for (std::size_t k = 0; k < nets.size(); k++)
	{
		log().debug("setting up net\n");

		analog_net_t *net = nets[k];

		net->m_solver = this;

		for (core_terminal_t *p : net->m_core_terms)
		{
			log().debug("{1} {2} {3}\n", p->name(), net->name(), (int) net->isRailNet());
			switch (p->type())
			{
				case terminal_t::TERMINAL:
					switch (p->device().family())
					{
						case device_t::CAPACITOR:
							if (!m_step_devices.contains(&p->device()))
								m_step_devices.push_back(&p->device());
							break;
						case device_t::BJT_EB:
						case device_t::DIODE:
						case device_t::LVCCS:
						case device_t::BJT_SWITCH:
							log().debug("found BJT/Diode/LVCCS\n");
							if (!m_dynamic_devices.contains(&p->device()))
								m_dynamic_devices.push_back(&p->device());
							break;
						default:
							break;
					}
					{
						terminal_t *pterm = dynamic_cast<terminal_t *>(p);
						add_term(k, pterm);
					}
					log().debug("Added terminal\n");
					break;
				case terminal_t::INPUT:
					{
						analog_output_t *net_proxy_output = NULL;
						for (auto & input : m_inps)
							if (input->m_proxied_net == &p->net().as_analog())
							{
								net_proxy_output = input;
								break;
							}

						if (net_proxy_output == NULL)
						{
							net_proxy_output = palloc(analog_output_t);
							net_proxy_output->init_object(*this, this->name() + "." + pfmt("m{1}")(m_inps.size()));
							m_inps.push_back(net_proxy_output);
							net_proxy_output->m_proxied_net = &p->net().as_analog();
						}
						net_proxy_output->net().register_con(*p);
						// FIXME: repeated
						net_proxy_output->net().rebuild_list();
						log().debug("Added input\n");
					}
					break;
				default:
					log().fatal("unhandled element found\n");
					break;
			}
		}
		log().debug("added net with {1} populated connections\n", net->m_core_terms.size());
	}

}


void matrix_solver_t::update_inputs()
{
	// avoid recursive calls. Inputs are updated outside this call
	for (std::size_t i=0; i<m_inps.size(); i++)
		m_inps[i]->set_Q(m_inps[i]->m_proxied_net->Q_Analog());
}

void matrix_solver_t::update_dynamic()
{
	/* update all non-linear devices  */
	for (std::size_t i=0; i < m_dynamic_devices.size(); i++)
		m_dynamic_devices[i]->update_terminals();
}

ATTR_COLD void matrix_solver_t::start()
{
	register_output("Q_sync", m_Q_sync);
	register_input("FB_sync", m_fb_sync);
	connect_direct(m_fb_sync, m_Q_sync);

	save(NLNAME(m_last_step));
	save(NLNAME(m_cur_ts));
	save(NLNAME(m_stat_calculations));
	save(NLNAME(m_stat_newton_raphson));
	save(NLNAME(m_stat_vsolver_calls));
	save(NLNAME(m_iterative_fail));
	save(NLNAME(m_iterative_total));

}

ATTR_COLD void matrix_solver_t::reset()
{
	m_last_step = netlist_time::zero;
}

ATTR_COLD void matrix_solver_t::update()
{
	const netlist_time new_timestep = solve();

	if (m_params.m_dynamic && is_timestep() && new_timestep > netlist_time::zero)
		m_Q_sync.net().reschedule_in_queue(new_timestep);
}

ATTR_COLD void matrix_solver_t::update_forced()
{
	ATTR_UNUSED const netlist_time new_timestep = solve();

	if (m_params.m_dynamic && is_timestep())
		m_Q_sync.net().reschedule_in_queue(netlist_time::from_double(m_params.m_min_timestep));
}

void matrix_solver_t::step(const netlist_time &delta)
{
	const nl_double dd = delta.as_double();
	for (std::size_t k=0; k < m_step_devices.size(); k++)
		m_step_devices[k]->step_time(dd);
}

netlist_time matrix_solver_t::solve_base()
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
			this_resched = this->vsolve_non_dynamic(true);
			newton_loops++;
		} while (this_resched > 1 && newton_loops < m_params.m_nr_loops);

		m_stat_newton_raphson += newton_loops;
		// reschedule ....
		if (this_resched > 1 && !m_Q_sync.net().is_queued())
		{
			log().warning("NEWTON_LOOPS exceeded on net {1}... reschedule", this->name());
			m_Q_sync.net().reschedule_in_queue(m_params.m_nt_sync_delay);
		}
	}
	else
	{
		this->vsolve_non_dynamic(false);
	}
	return this->compute_next_timestep();
}

netlist_time matrix_solver_t::solve()
{
	const netlist_time now = netlist().time();
	const netlist_time delta = now - m_last_step;

	// We are already up to date. Avoid oscillations.
	// FIXME: Make this a parameter!
	if (delta < netlist_time::from_nsec(1)) // 20000
		return netlist_time::from_nsec(0);

	/* update all terminals for new time step */
	m_last_step = now;
	m_cur_ts = delta.as_double();

	step(delta);

	const netlist_time next_time_step = solve_base();

	update_inputs();
	return next_time_step;
}

ATTR_COLD int matrix_solver_t::get_net_idx(net_t *net)
{
	for (std::size_t k = 0; k < m_nets.size(); k++)
		if (m_nets[k] == net)
			return k;
	return -1;
}

void matrix_solver_t::log_stats()
{
	if (this->m_stat_calculations != 0 && this->m_stat_vsolver_calls && this->m_params.m_log_stats)
	{
		log().verbose("==============================================");
		log().verbose("Solver {1}", this->name());
		log().verbose("       ==> {1} nets", this->m_nets.size()); //, (*(*groups[i].first())->m_core_terms.first())->name());
		log().verbose("       has {1} elements", this->is_dynamic() ? "dynamic" : "no dynamic");
		log().verbose("       has {1} elements", this->is_timestep() ? "timestep" : "no timestep");
		log().verbose("       {1:6.3} average newton raphson loops", (double) this->m_stat_newton_raphson / (double) this->m_stat_vsolver_calls);
		log().verbose("       {1:10} invocations ({2:6} Hz)  {3:10} gs fails ({4:6.2} %) {5:6.3} average",
				this->m_stat_calculations,
				this->m_stat_calculations * 10 / (int) (this->netlist().time().as_double() * 10.0),
				this->m_iterative_fail,
				100.0 * (double) this->m_iterative_fail / (double) this->m_stat_calculations,
				(double) this->m_iterative_total / (double) this->m_stat_calculations);
	}
}







// ----------------------------------------------------------------------------------------
// solver
// ----------------------------------------------------------------------------------------



NETLIB_START(solver)
{
	register_output("Q_step", m_Q_step);

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

	register_input("FB_step", m_fb_step);
	connect_late(m_fb_step, m_Q_step);

}

NETLIB_RESET(solver)
{
	for (std::size_t i = 0; i < m_mat_solvers.size(); i++)
		m_mat_solvers[i]->reset();
}


NETLIB_UPDATE_PARAM(solver)
{
	//m_inc = time::from_hz(m_freq.Value());
}

NETLIB_STOP(solver)
{
	for (std::size_t i = 0; i < m_mat_solvers.size(); i++)
		m_mat_solvers[i]->log_stats();
}

NETLIB_NAME(solver)::~NETLIB_NAME(solver)()
{
	m_mat_solvers.clear_and_free();
}

NETLIB_UPDATE(solver)
{
	if (m_params.m_dynamic)
		return;


#if HAS_OPENMP && USE_OPENMP
	const std::size_t t_cnt = m_mat_solvers.size();
	if (m_parallel.Value())
	{
		omp_set_num_threads(3);
		//omp_set_dynamic(0);
		#pragma omp parallel
		{
			#pragma omp for
			for (int i = 0; i <  t_cnt; i++)
				if (m_mat_solvers[i]->is_timestep())
				{
					// Ignore return value
					ATTR_UNUSED const nl_double ts = m_mat_solvers[i]->solve();
				}
		}
	}
	else
		for (int i = 0; i < t_cnt; i++)
			if (m_mat_solvers[i]->is_timestep())
			{
				// Ignore return value
				ATTR_UNUSED const nl_double ts = m_mat_solvers[i]->solve();
			}
#else
	for (auto & solver : m_mat_solvers)
		if (solver->is_timestep())
			// Ignore return value
			ATTR_UNUSED const netlist_time ts = solver->solve();
#endif

	/* step circuit */
	if (!m_Q_step.net().is_queued())
	{
		m_Q_step.net().push_to_queue(netlist_time::from_double(m_params.m_max_timestep));
	}
}

template <int m_N, int _storage_N>
matrix_solver_t * NETLIB_NAME(solver)::create_solver(int size, const bool use_specific)
{
	if (use_specific && m_N == 1)
		return palloc(matrix_solver_direct1_t(&m_params));
	else if (use_specific && m_N == 2)
		return palloc(matrix_solver_direct2_t(&m_params));
	else
	{
		if (size >= m_gs_threshold)
		{
			if (pstring("SOR_MAT").equals(m_iterative_solver))
			{
				typedef matrix_solver_SOR_mat_t<m_N,_storage_N> solver_sor_mat;
				return palloc(solver_sor_mat(&m_params, size));
			}
			else if (pstring("MAT").equals(m_iterative_solver))
			{
				typedef matrix_solver_direct_t<m_N,_storage_N> solver_mat;
				return palloc(solver_mat(&m_params, size));
			}
			else if (pstring("SM").equals(m_iterative_solver))
			{
				/* Sherman-Morrison Formula */
				typedef matrix_solver_sm_t<m_N,_storage_N> solver_mat;
				return palloc(solver_mat(&m_params, size));
			}
			else if (pstring("W").equals(m_iterative_solver))
			{
				/* Woodbury Formula */
				typedef matrix_solver_w_t<m_N,_storage_N> solver_mat;
				return palloc(solver_mat(&m_params, size));
			}
			else if (pstring("SOR").equals(m_iterative_solver))
			{
				typedef matrix_solver_SOR_t<m_N,_storage_N> solver_GS;
				return palloc(solver_GS(&m_params, size));
			}
			else if (pstring("GMRES").equals(m_iterative_solver))
			{
				typedef matrix_solver_GMRES_t<m_N,_storage_N> solver_GMRES;
				return palloc(solver_GMRES(&m_params, size));
			}
			else
			{
				netlist().log().fatal("Unknown solver type: {1}\n", m_iterative_solver.Value());
				return NULL;
			}
		}
		else
		{
			typedef matrix_solver_direct_t<m_N,_storage_N> solver_D;
			return palloc(solver_D(&m_params, size));
		}
	}
}

ATTR_COLD void NETLIB_NAME(solver)::post_start()
{
	pvector_t<analog_net_t::list_t> groups;
	const bool use_specific = true;

	m_params.m_pivot = m_pivot.Value();
	m_params.m_accuracy = m_accuracy.Value();
	m_params.m_gs_loops = m_gs_loops.Value();
	m_params.m_nr_loops = m_nr_loops.Value();
	m_params.m_nt_sync_delay = netlist_time::from_double(m_sync_delay.Value());
	m_params.m_lte = m_lte.Value();
	m_params.m_sor = m_sor.Value();

	m_params.m_min_timestep = m_min_timestep.Value();
	m_params.m_dynamic = (m_dynamic.Value() == 1 ? true : false);
	m_params.m_max_timestep = netlist_time::from_hz(m_freq.Value()).as_double();

	if (m_params.m_dynamic)
	{
		m_params.m_max_timestep *= NL_FCONST(1000.0);
	}
	else
	{
		m_params.m_min_timestep = m_params.m_max_timestep;
	}

	// Override log statistics
	pstring p = nl_util::environment("NL_STATS");
	if (p != "")
		m_params.m_log_stats = (bool) p.as_long();
	else
		m_params.m_log_stats = (bool) m_log_stats.Value();

	netlist().log().verbose("Scanning net groups ...");
	// determine net groups
	for (auto & net : netlist().m_nets)
	{
		netlist().log().debug("processing {1}\n", net->name());
		if (!net->isRailNet())
		{
			netlist().log().debug("   ==> not a rail net\n");
			analog_net_t *n = &net->as_analog();
			if (!n->already_processed(groups))
			{
				groups.push_back(analog_net_t::list_t());
				n->process_net(groups);
			}
		}
	}

	// setup the solvers
	netlist().log().verbose("Found {1} net groups in {2} nets\n", groups.size(), netlist().m_nets.size());
	for (auto & grp : groups)
	{
		matrix_solver_t *ms;
		std::size_t net_count = grp.size();

		switch (net_count)
		{
			case 1:
				ms = create_solver<1,1>(1, use_specific);
				break;
			case 2:
				ms = create_solver<2,2>(2, use_specific);
				break;
			case 3:
				ms = create_solver<3,3>(3, use_specific);
				break;
			case 4:
				ms = create_solver<4,4>(4, use_specific);
				break;
			case 5:
				ms = create_solver<5,5>(5, use_specific);
				break;
			case 6:
				ms = create_solver<6,6>(6, use_specific);
				break;
			case 7:
				ms = create_solver<7,7>(7, use_specific);
				break;
			case 8:
				ms = create_solver<8,8>(8, use_specific);
				break;
			case 10:
				ms = create_solver<10,10>(10, use_specific);
				break;
			case 11:
				ms = create_solver<11,11>(11, use_specific);
				break;
			case 12:
				ms = create_solver<12,12>(12, use_specific);
				break;
			case 15:
				ms = create_solver<15,15>(15, use_specific);
				break;
			case 31:
				ms = create_solver<31,31>(31, use_specific);
				break;
			case 49:
				ms = create_solver<49,49>(49, use_specific);
				break;
#if 0
			case 87:
				ms = create_solver<87,87>(87, use_specific);
				break;
#endif
			default:
				netlist().log().warning("No specific solver found for netlist of size {1}", (unsigned) net_count);
				if (net_count <= 16)
				{
					ms = create_solver<0,16>(net_count, use_specific);
				}
				else if (net_count <= 32)
				{
					ms = create_solver<0,32>(net_count, use_specific);
				}
				else if (net_count <= 64)
				{
					ms = create_solver<0,64>(net_count, use_specific);
				}
				else
					if (net_count <= 128)
				{
					ms = create_solver<0,128>(net_count, use_specific);
				}
				else
				{
					netlist().log().fatal("Encountered netgroup with > 128 nets");
					ms = NULL; /* tease compilers */
				}

				break;
		}

		register_sub(pfmt("Solver_{1}")(m_mat_solvers.size()), *ms);

		ms->setup(grp);

		m_mat_solvers.push_back(ms);

		netlist().log().verbose("Solver {1}", ms->name());
		netlist().log().verbose("       ==> {2} nets", grp.size());
		netlist().log().verbose("       has {1} elements", ms->is_dynamic() ? "dynamic" : "no dynamic");
		netlist().log().verbose("       has {1} elements", ms->is_timestep() ? "timestep" : "no timestep");
		for (net_t *n : grp)
		{
			netlist().log().verbose("Net {1}", n->name());
			for (const core_terminal_t *pcore : n->m_core_terms)
			{
				netlist().log().verbose("   {1}", pcore->name());
			}
		}
	}
}

NETLIB_NAMESPACE_DEVICES_END()
