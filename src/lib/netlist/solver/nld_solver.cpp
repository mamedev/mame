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
#pragma GCC optimize "-ftree-vectorize"
#pragma GCC optimize "-ffast-math"
#pragma GCC optimize "-funsafe-math-optimizations"
#pragma GCC optimize "-funroll-loops"
#pragma GCC optimize "-funswitch-loops"
#pragma GCC optimize "-fstrict-aliasing"
#pragma GCC optimize "tree-vectorizer-verbose=7"
#pragma GCC optimize "opt-info-vec"
#pragma GCC optimize "opt-info-vec-missed"
//#pragma GCC optimize "tree-parallelize-loops=4"
#pragma GCC optimize "variable-expansion-in-unroller"
#pragma GCC optimize "unsafe-loop-optimizations"
#pragma GCC optimize "vect-cost-model"
#pragma GCC optimize "variable-expansion-in-unroller"
#pragma GCC optimize "tree-loop-if-convert-stores"
#pragma GCC optimize "tree-loop-distribution"
#pragma GCC optimize "tree-loop-im"
#pragma GCC optimize "tree-loop-ivcanon"
#pragma GCC optimize "ivopts"
#endif

#include "netlist/nl_lists.h"
#include "netlist/nl_factory.h"
#include "nld_matrix_solver.h"
#include "nld_ms_direct.h"
#include "nld_ms_direct1.h"
#include "nld_ms_direct2.h"
#include "nld_ms_gcr.h"
#include "nld_ms_gmres.h"
#include "nld_ms_sm.h"
#include "nld_ms_sor.h"
#include "nld_ms_sor_mat.h"
#include "nld_ms_w.h"
#include "nld_solver.h"
#include "plib/pomp.h"

#include <algorithm>
#include <cmath>

namespace netlist
{
namespace devices
{

	// ----------------------------------------------------------------------------------------
	// solver
	// ----------------------------------------------------------------------------------------

	NETLIB_RESET(solver)
	{
		for (auto &s : m_mat_solvers)
			s->reset();
	}

	void NETLIB_NAME(solver)::stop()
	{
		for (auto &s : m_mat_solvers)
			s->log_stats();
	}

	NETLIB_UPDATE(solver)
	{
		if (m_params.m_dynamic_ts)
			return;

		netlist_time now(exec().time());
		/* force solving during start up if there are no time-step devices */
		/* FIXME: Needs a more elegant solution */
		bool force_solve = (now < netlist_time::from_double(2 * m_params.m_max_timestep));

		std::size_t nthreads = std::min(static_cast<std::size_t>(m_parallel()), plib::omp::get_max_threads());

		std::vector<matrix_solver_t *> &solvers = (force_solve ? m_mat_solvers_all : m_mat_solvers_timestepping);

		if (nthreads > 1 && solvers.size() > 1)
		{
			plib::omp::set_num_threads(nthreads);
			plib::omp::for_static(static_cast<std::size_t>(0), solvers.size(), [&solvers, now](std::size_t i)
				{
					const netlist_time ts = solvers[i]->solve(now);
					plib::unused_var(ts);
				});
		}
		else
			for (auto & solver : solvers)
			{
				const netlist_time ts = solver->solve(now);
				plib::unused_var(ts);
			}

		for (auto & solver : solvers)
			solver->update_inputs();

		/* step circuit */
		if (!m_Q_step.net().is_queued())
		{
			m_Q_step.net().toggle_and_push_to_queue(netlist_time::from_double(m_params.m_max_timestep));
		}
	}

	template <class C>
	pool_owned_ptr<matrix_solver_t> create_it(netlist_state_t &nl, pstring name, solver_parameters_t &params, std::size_t size)
	{
		return pool().make_poolptr<C>(nl, name, &params, size);
	}

	template <typename FT, int SIZE>
	pool_owned_ptr<matrix_solver_t> NETLIB_NAME(solver)::create_solver(std::size_t size, const pstring &solvername)
	{
		if (m_method() == "SOR_MAT")
		{
			return create_it<matrix_solver_SOR_mat_t<FT, SIZE>>(state(), solvername, m_params, size);
			//typedef matrix_solver_SOR_mat_t<m_N,storage_N> solver_sor_mat;
			//return plib::make_unique<solver_sor_mat>(state(), solvername, &m_params, size);
		}
		else if (m_method() == "MAT_CR")
		{
			if (size > 0) // GCR always outperforms MAT solver
			{
				return create_it<matrix_solver_GCR_t<FT, SIZE>>(state(), solvername, m_params, size);
			}
			else
			{
				return create_it<matrix_solver_direct_t<FT, SIZE>>(state(), solvername, m_params, size);
			}
		}
		else if (m_method() == "MAT")
		{
			return create_it<matrix_solver_direct_t<FT, SIZE>>(state(), solvername, m_params, size);
		}
		else if (m_method() == "SM")
		{
			/* Sherman-Morrison Formula */
			return create_it<matrix_solver_sm_t<FT, SIZE>>(state(), solvername, m_params, size);
		}
		else if (m_method() == "W")
		{
			/* Woodbury Formula */
			return create_it<matrix_solver_w_t<FT, SIZE>>(state(), solvername, m_params, size);
		}
		else if (m_method() == "SOR")
		{
			return create_it<matrix_solver_SOR_t<FT, SIZE>>(state(), solvername, m_params, size);
		}
		else if (m_method() == "GMRES")
		{
			return create_it<matrix_solver_GMRES_t<FT, SIZE>>(state(), solvername, m_params, size);
		}
		else
		{
			log().fatal(MF_UNKNOWN_SOLVER_TYPE(m_method()));
			return pool_owned_ptr<matrix_solver_t>();
		}
	}

	template <typename FT, int SIZE>
	pool_owned_ptr<matrix_solver_t> NETLIB_NAME(solver)::create_solver_x(std::size_t size, const pstring &solvername)
	{
		if (SIZE > 0)
		{
			if (size == SIZE)
				return create_solver<FT, SIZE>(size, solvername);
			else
				return this->create_solver_x<FT, SIZE-1>(size, solvername);
		}
		else
		{
			if (size * 2 > -SIZE )
				return create_solver<FT, SIZE>(size, solvername);
			else
				return this->create_solver_x<FT, SIZE / 2>(size, solvername);
		}
	}

	struct net_splitter
	{

		bool already_processed(const analog_net_t &n) const
		{
			/* no need to process rail nets - these are known variables */
			if (n.isRailNet())
				return true;
			/* if it's already processed - no need to continue */
			for (auto & grp : groups)
				if (plib::container::contains(grp, &n))
					return true;
			return false;
		}

		void process_net(analog_net_t &n)
		{
			/* ignore empty nets. FIXME: print a warning message */
			if (n.num_cons() == 0)
				return;
			/* add the net */
			groups.back().push_back(&n);
			/* process all terminals connected to this net */
			for (auto &term : n.core_terms())
			{
				/* only process analog terminals */
				if (term->is_type(detail::terminal_type::TERMINAL))
				{
					auto *pt = static_cast<terminal_t *>(term);
					/* check the connected terminal */
					analog_net_t &connected_net = pt->connected_terminal()->net();
					if (!already_processed(connected_net))
						process_net(connected_net);
				}
			}
		}

		void run(netlist_state_t &netlist)
		{
			for (auto & net : netlist.nets())
			{
				netlist.log().debug("processing {1}\n", net->name());
				if (!net->isRailNet() && net->num_cons() > 0)
				{
					netlist.log().debug("   ==> not a rail net\n");
					/* Must be an analog net */
					auto &n = *static_cast<analog_net_t *>(net.get());
					if (!already_processed(n))
					{
						groups.emplace_back(analog_net_t::list_t());
						process_net(n);
					}
				}
			}
		}

		std::vector<analog_net_t::list_t> groups;
	};

	void NETLIB_NAME(solver)::post_start()
	{
		m_params.m_pivot = m_pivot();
		m_params.m_accuracy = m_accuracy();
		/* FIXME: Throw when negative */
		m_params.m_gs_loops = static_cast<unsigned>(m_gs_loops());
		m_params.m_nr_loops = static_cast<unsigned>(m_nr_loops());
		m_params.m_nr_recalc_delay = netlist_time::from_double(m_nr_recalc_delay());
		m_params.m_dynamic_lte = m_dynamic_lte();
		m_params.m_gs_sor = m_gs_sor();

		m_params.m_min_timestep = m_dynamic_min_ts();
		m_params.m_dynamic_ts = (m_dynamic_ts() == 1 ? true : false);
		m_params.m_max_timestep = netlist_time::from_double(1.0 / m_freq()).as_double();

		m_params.m_use_gabs = m_use_gabs();
		m_params.m_use_linear_prediction = m_use_linear_prediction();


		if (m_params.m_dynamic_ts)
		{
			m_params.m_max_timestep *= 1;//NL_FCONST(1000.0);
		}
		else
		{
			m_params.m_min_timestep = m_params.m_max_timestep;
		}

		//m_params.m_max_timestep = std::max(m_params.m_max_timestep, m_params.m_max_timestep::)

		// Override log statistics
		pstring p = plib::util::environment("NL_STATS", "");
		if (p != "")
			m_params.m_log_stats = plib::pstonum<decltype(m_params.m_log_stats)>(p);
		else
			m_params.m_log_stats = m_log_stats();

		log().verbose("Scanning net groups ...");
		// determine net groups

		net_splitter splitter;

		splitter.run(state());

		// setup the solvers
		log().verbose("Found {1} net groups in {2} nets\n", splitter.groups.size(), state().nets().size());
		for (auto & grp : splitter.groups)
		{
			pool_owned_ptr<matrix_solver_t> ms;
			std::size_t net_count = grp.size();
			pstring sname = plib::pfmt("Solver_{1}")(m_mat_solvers.size());

			switch (net_count)
			{
	#if 1
				case 1:
					ms = pool().make_poolptr<matrix_solver_direct1_t<double>>(state(), sname, &m_params);
					break;
				case 2:
					ms = pool().make_poolptr<matrix_solver_direct2_t<double>>(state(), sname, &m_params);
					break;
				case 3:
					ms = create_solver<double, 3>(3, sname);
					break;
				case 4:
					ms = create_solver<double, 4>(4, sname);
					break;
				case 5:
					ms = create_solver<double, 5>(5, sname);
					break;
				case 6:
					ms = create_solver<double, 6>(6, sname);
					break;
				case 7:
					ms = create_solver<double, 7>(7, sname);
					break;
				case 8:
					ms = create_solver<double, 8>(8, sname);
					break;
				case 9:
					ms = create_solver<double, 9>(9, sname);
					break;
				case 10:
					ms = create_solver<double, 10>(10, sname);
					break;
	#if 0
				case 11:
					ms = create_solver<double, 11>(11, sname);
					break;
				case 12:
					ms = create_solver<double, 12>(12, sname);
					break;
				case 15:
					ms = create_solver<double, 15>(15, sname);
					break;
				case 31:
					ms = create_solver<double, 31>(31, sname);
					break;
				case 35:
					ms = create_solver<double, 35>(35, sname);
					break;
				case 43:
					ms = create_solver<double, 43>(43, sname);
					break;
				case 49:
					ms = create_solver<double, 49>(49, sname);
					break;
	#endif
	#if 1
				case 86:
					ms = create_solver<double,86>(86, sname);
					break;
	#endif
	#endif
				default:
					log().info(MI_NO_SPECIFIC_SOLVER(net_count));
					if (net_count <= 8)
					{
						ms = create_solver<double, -8>(net_count, sname);
					}
					else if (net_count <= 16)
					{
						ms = create_solver<double, -16>(net_count, sname);
					}
					else if (net_count <= 32)
					{
						ms = create_solver<double, -32>(net_count, sname);
					}
					else
						if (net_count <= 64)
					{
						ms = create_solver<double, -64>(net_count, sname);
					}
					else
						if (net_count <= 128)
					{
						ms = create_solver<double, -128>(net_count, sname);
					}
					else
					{
						log().fatal(MF_NETGROUP_SIZE_EXCEEDED_1(128));
						return; /* tease compilers */
					}
					break;
			}

			// FIXME ...
			ms->setup(grp);

			log().verbose("Solver {1}", ms->name());
			log().verbose("       ==> {2} nets", grp.size());
			log().verbose("       has {1} elements", ms->has_dynamic_devices() ? "dynamic" : "no dynamic");
			log().verbose("       has {1} elements", ms->has_timestep_devices() ? "timestep" : "no timestep");
			for (auto &n : grp)
			{
				log().verbose("Net {1}", n->name());
				for (const auto &pcore : n->core_terms())
				{
					log().verbose("   {1}", pcore->name());
				}
			}

			m_mat_solvers_all.push_back(ms.get());
			if (ms->has_timestep_devices())
				m_mat_solvers_timestepping.push_back(ms.get());

			m_mat_solvers.emplace_back(std::move(ms));
		}
	}

	void NETLIB_NAME(solver)::create_solver_code(std::map<pstring, pstring> &mp)
	{
		for (auto & s : m_mat_solvers)
		{
			auto r = s->create_solver_code();
			mp[r.first] = r.second; // automatically overwrites identical names
		}
	}

	NETLIB_DEVICE_IMPL(solver, "SOLVER", "FREQ")

} // namespace devices
} // namespace netlist
