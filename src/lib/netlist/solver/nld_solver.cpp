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
#include "nld_ms_gcr.h"
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
				m_other_curanalog.insert_at(i, nullptr);
				return;
			}
		}
	m_term.push_back(term);
	m_net_other.push_back(net_other);
	m_gt.push_back(0.0);
	m_go.push_back(0.0);
	m_Idr.push_back(0.0);
	m_other_curanalog.push_back(nullptr);
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

ATTR_COLD matrix_solver_t::~matrix_solver_t()
{
	m_inps.clear_and_free();
	for (unsigned k = 0; k < m_terms.size(); k++)
	{
		pfree(m_terms[k]);
	}

}

ATTR_COLD void matrix_solver_t::setup_base(analog_net_t::list_t &nets)
{
	log().debug("New solver setup\n");

	m_nets.clear();
	m_terms.clear();

	for (auto & net : nets)
	{
		m_nets.push_back(net);
		m_terms.push_back(palloc<terms_t>());
		m_rails_temp.push_back(palloc<terms_t>());
	}

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
					if (p->device().is_timestep())
						if (!m_step_devices.contains(&p->device()))
							m_step_devices.push_back(&p->device());
					if (p->device().is_dynamic1())
						if (!m_dynamic_devices.contains(&p->device()))
							m_dynamic_devices.push_back(&p->device());
					{
						terminal_t *pterm = dynamic_cast<terminal_t *>(p);
						add_term(k, pterm);
					}
					log().debug("Added terminal {1}\n", p->name());
					break;
				case terminal_t::INPUT:
					{
						analog_output_t *net_proxy_output = nullptr;
						for (auto & input : m_inps)
							if (input->m_proxied_net == &p->net().as_analog())
							{
								net_proxy_output = input;
								break;
							}

						if (net_proxy_output == nullptr)
						{
							//net_proxy_output = palloc(analog_output_t(*this,
							//      this->name() + "." + pfmt("m{1}")(m_inps.size())));

							net_proxy_output = palloc<analog_output_t>(*this, this->name() + "." + pfmt("m{1}")(m_inps.size()));
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

	/* now setup the matrix */
	setup_matrix();
}

ATTR_COLD void matrix_solver_t::setup_matrix()
{
	const unsigned iN = m_nets.size();

	for (unsigned k = 0; k < iN; k++)
	{
		m_terms[k]->m_railstart = m_terms[k]->count();
		for (unsigned i = 0; i < m_rails_temp[k]->count(); i++)
			this->m_terms[k]->add(m_rails_temp[k]->terms()[i], m_rails_temp[k]->net_other()[i], false);

		m_rails_temp[k]->clear(); // no longer needed
		m_terms[k]->set_pointers();
	}

	for (unsigned k = 0; k < iN; k++)
		pfree(m_rails_temp[k]); // no longer needed

	m_rails_temp.clear();

	/* Sort in descending order by number of connected matrix voltages.
	 * The idea is, that for Gauss-Seidel algo the first voltage computed
	 * depends on the greatest number of previous voltages thus taking into
	 * account the maximum amout of information.
	 *
	 * This actually improves performance on popeye slightly. Average
	 * GS computations reduce from 2.509 to 2.370
	 *
	 * Smallest to largest : 2.613
	 * Unsorted            : 2.509
	 * Largest to smallest : 2.370
	 *
	 * Sorting as a general matrix pre-conditioning is mentioned in
	 * literature but I have found no articles about Gauss Seidel.
	 *
	 * For Gaussian Elimination however increasing order is better suited.
	 * FIXME: Even better would be to sort on elements right of the matrix diagonal.
	 *
	 */

	if (m_sort != NOSORT)
	{
		int sort_order = (m_sort == DESCENDING ? 1 : -1);

		for (unsigned k = 0; k < iN - 1; k++)
			for (unsigned i = k+1; i < iN; i++)
			{
				if (((int) m_terms[k]->m_railstart - (int) m_terms[i]->m_railstart) * sort_order < 0)
				{
					std::swap(m_terms[i], m_terms[k]);
					std::swap(m_nets[i], m_nets[k]);
				}
			}

		for (unsigned k = 0; k < iN; k++)
		{
			int *other = m_terms[k]->net_other();
			for (unsigned i = 0; i < m_terms[k]->count(); i++)
				if (other[i] != -1)
					other[i] = get_net_idx(&m_terms[k]->terms()[i]->m_otherterm->net());
		}
	}

	/* create a list of non zero elements. */
	for (unsigned k = 0; k < iN; k++)
	{
		terms_t * t = m_terms[k];
		/* pretty brutal */
		int *other = t->net_other();

		t->m_nz.clear();

		for (unsigned i = 0; i < t->m_railstart; i++)
			if (!t->m_nz.contains(other[i]))
				t->m_nz.push_back(other[i]);

		t->m_nz.push_back(k);     // add diagonal

		/* and sort */
		psort_list(t->m_nz);
	}

	/* create a list of non zero elements right of the diagonal
	 * These list anticipate the population of array elements by
	 * Gaussian elimination.
	 */
	for (unsigned k = 0; k < iN; k++)
	{
		terms_t * t = m_terms[k];
		/* pretty brutal */
		int *other = t->net_other();

		if (k==0)
			t->m_nzrd.clear();
		else
		{
			t->m_nzrd = m_terms[k-1]->m_nzrd;
			unsigned j=0;
			while(j < t->m_nzrd.size())
			{
				if (t->m_nzrd[j] < k + 1)
					t->m_nzrd.remove_at(j);
				else
					j++;
			}
		}

		for (unsigned i = 0; i < t->m_railstart; i++)
			if (!t->m_nzrd.contains(other[i]) && other[i] >= (int) (k + 1))
				t->m_nzrd.push_back(other[i]);

		/* and sort */
		psort_list(t->m_nzrd);
	}

	/* create a list of non zero elements below diagonal k
	 * This should reduce cache misses ...
	 */

	bool **touched = new bool*[iN];
	for (unsigned k=0; k<iN; k++)
		touched[k] = new bool[iN];

	for (unsigned k = 0; k < iN; k++)
	{
		for (unsigned j = 0; j < iN; j++)
			touched[k][j] = false;
		for (unsigned j = 0; j < m_terms[k]->m_nz.size(); j++)
			touched[k][m_terms[k]->m_nz[j]] = true;
	}

	unsigned ops = 0;
	for (unsigned k = 0; k < iN; k++)
	{
		ops++; // 1/A(k,k)
		for (unsigned row = k + 1; row < iN; row++)
		{
			if (touched[row][k])
			{
				ops++;
				if (!m_terms[k]->m_nzbd.contains(row))
					m_terms[k]->m_nzbd.push_back(row);
				for (unsigned col = k + 1; col < iN; col++)
					if (touched[k][col])
					{
						touched[row][col] = true;
						ops += 2;
					}
			}
		}
	}
	log().verbose("Number of mults/adds for {1}: {2}", name(), ops);

	if (0)
		for (unsigned k = 0; k < iN; k++)
		{
			pstring line = pfmt("{1}")(k, "3");
			for (unsigned j = 0; j < m_terms[k]->m_nzrd.size(); j++)
				line += pfmt(" {1}")(m_terms[k]->m_nzrd[j], "3");
			log().verbose("{1}", line);
		}

	/*
	 * save states
	 */
	for (unsigned k = 0; k < iN; k++)
	{
		pstring num = pfmt("{1}")(k);

		save(m_terms[k]->m_last_V, "lastV." + num);
		save(m_terms[k]->m_DD_n_m_1, "m_DD_n_m_1." + num);
		save(m_terms[k]->m_h_n_m_1, "m_h_n_m_1." + num);

		save(m_terms[k]->go(),"GO" + num, m_terms[k]->count());
		save(m_terms[k]->gt(),"GT" + num, m_terms[k]->count());
		save(m_terms[k]->Idr(),"IDR" + num , m_terms[k]->count());
	}

	for (unsigned k=0; k<iN; k++)
		delete [] touched[k];
	delete [] touched;
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

ATTR_COLD void matrix_solver_t::reset()
{
	m_last_step = netlist_time::zero;
}

ATTR_COLD void matrix_solver_t::update() NOEXCEPT
{
	const netlist_time new_timestep = solve();

	if (m_params.m_dynamic && has_timestep_devices() && new_timestep > netlist_time::zero)
		m_Q_sync.net().reschedule_in_queue(new_timestep);
}

ATTR_COLD void matrix_solver_t::update_forced()
{
	ATTR_UNUSED const netlist_time new_timestep = solve();

	if (m_params.m_dynamic && has_timestep_devices())
		m_Q_sync.net().reschedule_in_queue(netlist_time::from_double(m_params.m_min_timestep));
}

void matrix_solver_t::step(const netlist_time &delta)
{
	const nl_double dd = delta.as_double();
	for (std::size_t k=0; k < m_step_devices.size(); k++)
		m_step_devices[k]->step_time(dd);
}

const netlist_time matrix_solver_t::solve_base()
{
	m_stat_vsolver_calls++;
	if (has_dynamic_devices())
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

const netlist_time matrix_solver_t::solve()
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

ATTR_COLD void matrix_solver_t::add_term(int k, terminal_t *term)
{
	if (term->m_otherterm->net().isRailNet())
	{
		m_rails_temp[k]->add(term, -1, false);
	}
	else
	{
		int ot = get_net_idx(&term->m_otherterm->net());
		if (ot>=0)
		{
			m_terms[k]->add(term, ot, true);
		}
		/* Should this be allowed ? */
		else // if (ot<0)
		{
			m_rails_temp[k]->add(term, ot, true);
			log().fatal("found term with missing othernet {1}\n", term->name());
		}
	}
}

netlist_time matrix_solver_t::compute_next_timestep()
{
	nl_double new_solver_timestep = m_params.m_max_timestep;

	if (m_params.m_dynamic)
	{
		/*
		 * FIXME: We should extend the logic to use either all nets or
		 *        only output nets.
		 */
		for (unsigned k = 0, iN=m_terms.size(); k < iN; k++)
		{
			analog_net_t *n = m_nets[k];
			terms_t *t = m_terms[k];

			const nl_double DD_n = (n->Q_Analog() - t->m_last_V);
			const nl_double hn = current_timestep();

			nl_double DD2 = (DD_n / hn - t->m_DD_n_m_1 / t->m_h_n_m_1) / (hn + t->m_h_n_m_1);
			nl_double new_net_timestep;

			t->m_h_n_m_1 = hn;
			t->m_DD_n_m_1 = DD_n;
			if (nl_math::abs(DD2) > NL_FCONST(1e-30)) // avoid div-by-zero
				new_net_timestep = nl_math::sqrt(m_params.m_lte / nl_math::abs(NL_FCONST(0.5)*DD2));
			else
				new_net_timestep = m_params.m_max_timestep;

			if (new_net_timestep < new_solver_timestep)
				new_solver_timestep = new_net_timestep;

			t->m_last_V = n->Q_Analog();
		}
		if (new_solver_timestep < m_params.m_min_timestep)
			new_solver_timestep = m_params.m_min_timestep;
	}
	//if (new_solver_timestep > 10.0 * hn)
	//    new_solver_timestep = 10.0 * hn;
	return netlist_time::from_double(new_solver_timestep);
}



void matrix_solver_t::log_stats()
{
	if (this->m_stat_calculations != 0 && this->m_stat_vsolver_calls && this->m_params.m_log_stats)
	{
		log().verbose("==============================================");
		log().verbose("Solver {1}", this->name());
		log().verbose("       ==> {1} nets", this->m_nets.size()); //, (*(*groups[i].first())->m_core_terms.first())->name());
		log().verbose("       has {1} elements", this->has_dynamic_devices() ? "dynamic" : "no dynamic");
		log().verbose("       has {1} elements", this->has_timestep_devices() ? "timestep" : "no timestep");
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



NETLIB_RESET(solver)
{
	for (std::size_t i = 0; i < m_mat_solvers.size(); i++)
		m_mat_solvers[i]->do_reset();
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
				if (m_mat_solvers[i]->has_timestep_devices())
				{
					// Ignore return value
					ATTR_UNUSED const netlist_time ts = m_mat_solvers[i]->solve();
				}
		}
	}
	else
		for (int i = 0; i < t_cnt; i++)
			if (m_mat_solvers[i]->has_timestep_devices())
			{
				// Ignore return value
				ATTR_UNUSED const netlist_time ts = m_mat_solvers[i]->solve();
			}
#else
	for (auto & solver : m_mat_solvers)
		if (solver->has_timestep_devices())
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
	pstring solvername = pfmt("Solver_{1}")(m_mat_solvers.size());
	if (use_specific && m_N == 1)
		return palloc<matrix_solver_direct1_t>(netlist(), solvername, &m_params);
	else if (use_specific && m_N == 2)
		return palloc<matrix_solver_direct2_t>(netlist(), solvername, &m_params);
	else
	{
		if (size >= m_gs_threshold)
		{
			if (pstring("SOR_MAT").equals(m_iterative_solver))
			{
				typedef matrix_solver_SOR_mat_t<m_N,_storage_N> solver_sor_mat;
				return palloc<solver_sor_mat>(netlist(), solvername, &m_params, size);
			}
			else if (pstring("MAT_CR").equals(m_iterative_solver))
			{
				typedef matrix_solver_GCR_t<m_N,_storage_N> solver_mat;
				return palloc<solver_mat>(netlist(), solvername, &m_params, size);
			}
			else if (pstring("MAT").equals(m_iterative_solver))
			{
				typedef matrix_solver_direct_t<m_N,_storage_N> solver_mat;
				return palloc<solver_mat>(netlist(), solvername, &m_params, size);
			}
			else if (pstring("SM").equals(m_iterative_solver))
			{
				/* Sherman-Morrison Formula */
				typedef matrix_solver_sm_t<m_N,_storage_N> solver_mat;
				return palloc<solver_mat>(netlist(), solvername, &m_params, size);
			}
			else if (pstring("W").equals(m_iterative_solver))
			{
				/* Woodbury Formula */
				typedef matrix_solver_w_t<m_N,_storage_N> solver_mat;
				return palloc<solver_mat>(netlist(), solvername, &m_params, size);
			}
			else if (pstring("SOR").equals(m_iterative_solver))
			{
				typedef matrix_solver_SOR_t<m_N,_storage_N> solver_GS;
				return palloc<solver_GS>(netlist(), solvername, &m_params, size);
			}
			else if (pstring("GMRES").equals(m_iterative_solver))
			{
				typedef matrix_solver_GMRES_t<m_N,_storage_N> solver_GMRES;
				return palloc<solver_GMRES>(netlist(), solvername, &m_params, size);
			}
			else
			{
				netlist().log().fatal("Unknown solver type: {1}\n", m_iterative_solver.Value());
				return nullptr;
			}
		}
		else
		{
			typedef matrix_solver_direct_t<m_N,_storage_N> solver_D;
			return palloc<solver_D>(netlist(), solvername, &m_params, size);
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
					ms = nullptr; /* tease compilers */
				}

				break;
		}

		// FIXME ...
		ms->set_delegate_pointer();
		ms->setup(grp);

		m_mat_solvers.push_back(ms);

		netlist().log().verbose("Solver {1}", ms->name());
		netlist().log().verbose("       ==> {2} nets", grp.size());
		netlist().log().verbose("       has {1} elements", ms->has_dynamic_devices() ? "dynamic" : "no dynamic");
		netlist().log().verbose("       has {1} elements", ms->has_timestep_devices() ? "timestep" : "no timestep");
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

void NETLIB_NAME(solver)::create_solver_code(postream &strm)
{
	for (auto & s : m_mat_solvers)
		s->create_solver_code(strm);
}


NETLIB_NAMESPACE_DEVICES_END()
