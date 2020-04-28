// license:GPL-2.0+
// copyright-holders:Couriersud

// Commented out for now. Relatively low number of terminals / nets make
// the vectorizations fast-math enables pretty expensive
//
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

#include "netlist/nl_factory.h"
#include "netlist/nl_setup.h" // FIXME: only needed for splitter code
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

		netlist_time_ext now(exec().time());
		// force solving during start up if there are no time-step devices
		// FIXME: Needs a more elegant solution
		bool force_solve = (now < netlist_time_ext::from_fp<decltype(m_params.m_max_timestep)>(2 * m_params.m_max_timestep));

		std::size_t nthreads = std::min(static_cast<std::size_t>(m_params.m_parallel()), plib::omp::get_max_threads());

		std::vector<solver::matrix_solver_t *> &solvers = (force_solve ? m_mat_solvers_all : m_mat_solvers_timestepping);

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

		// step circuit
		if (!m_Q_step.net().is_queued())
		{
			m_Q_step.net().toggle_and_push_to_queue(netlist_time::from_fp(m_params.m_max_timestep));
		}
	}

	template <class C>
	plib::unique_ptr<solver::matrix_solver_t> create_it(netlist_state_t &nl, pstring name,
		analog_net_t::list_t &nets,
		solver::solver_parameters_t &params, std::size_t size)
	{
		return plib::make_unique<C>(nl, name, nets, &params, size);
	}

	template <typename FT, int SIZE>
	plib::unique_ptr<solver::matrix_solver_t> NETLIB_NAME(solver)::create_solver(std::size_t size,
		const pstring &solvername,
		analog_net_t::list_t &nets)
	{
		switch (m_params.m_method())
		{
			case solver::matrix_type_e::MAT_CR:
				return create_it<solver::matrix_solver_GCR_t<FT, SIZE>>(state(), solvername, nets, m_params, size);
			case solver::matrix_type_e::MAT:
				return create_it<solver::matrix_solver_direct_t<FT, SIZE>>(state(), solvername, nets, m_params, size);
#if (NL_USE_ACADEMIC_SOLVERS)
			case solver::matrix_type_e::GMRES:
				return create_it<solver::matrix_solver_GMRES_t<FT, SIZE>>(state(), solvername, nets, m_params, size);
			case solver::matrix_type_e::SOR:
				return create_it<solver::matrix_solver_SOR_t<FT, SIZE>>(state(), solvername, nets, m_params, size);
			case solver::matrix_type_e::SOR_MAT:
				return create_it<solver::matrix_solver_SOR_mat_t<FT, SIZE>>(state(), solvername, nets, m_params, size);
			case solver::matrix_type_e::SM:
				// Sherman-Morrison Formula
				return create_it<solver::matrix_solver_sm_t<FT, SIZE>>(state(), solvername, nets, m_params, size);
			case solver::matrix_type_e::W:
				// Woodbury Formula
				return create_it<solver::matrix_solver_w_t<FT, SIZE>>(state(), solvername, nets, m_params, size);
#else
			default:
				state().log().warning(MW_SOLVER_METHOD_NOT_SUPPORTED(m_params.m_method().name(), "MAT_CR"));
				return create_it<solver::matrix_solver_GCR_t<FT, SIZE>>(state(), solvername, nets, m_params, size);
#endif
		}
		return plib::unique_ptr<solver::matrix_solver_t>();
	}

	template <typename FT>
	plib::unique_ptr<solver::matrix_solver_t> NETLIB_NAME(solver)::create_solvers(
		const pstring &sname,
		analog_net_t::list_t &nets)
	{
		std::size_t net_count = nets.size();
		switch (net_count)
		{
			case 1:
				return plib::make_unique<solver::matrix_solver_direct1_t<FT>>(state(), sname, nets, &m_params);
				break;
			case 2:
				return plib::make_unique<solver::matrix_solver_direct2_t<FT>>(state(), sname, nets, &m_params);
				break;
			case 3:
				return create_solver<FT, 3>(3, sname, nets);
				break;
			case 4:
				return create_solver<FT, 4>(4, sname, nets);
				break;
			case 5:
				return create_solver<FT, 5>(5, sname, nets);
				break;
			case 6:
				return create_solver<FT, 6>(6, sname, nets);
				break;
			case 7:
				return create_solver<FT, 7>(7, sname, nets);
				break;
			case 8:
				return create_solver<FT, 8>(8, sname, nets);
				break;
			default:
				log().info(MI_NO_SPECIFIC_SOLVER(net_count));
				if (net_count <= 16)
				{
					return create_solver<FT, -16>(net_count, sname, nets);
				}
				if (net_count <= 32)
				{
					return create_solver<FT, -32>(net_count, sname, nets);
				}
				if (net_count <= 64)
				{
					return create_solver<FT, -64>(net_count, sname, nets);
				}
				if (net_count <= 128)
				{
					return create_solver<FT, -128>(net_count, sname, nets);
				}
				if (net_count <= 256)
				{
					return create_solver<FT, -256>(net_count, sname, nets);
				}
				if (net_count <= 512)
				{
					return create_solver<FT, -512>(net_count, sname, nets);
				}
				return create_solver<FT, 0>(net_count, sname, nets);
				break;
		}
	}

	struct net_splitter
	{
		void run(netlist_state_t &netlist)
		{
			for (auto & net : netlist.nets())
			{
				netlist.log().verbose("processing {1}", net->name());
				if (!net->is_rail_net() && net->has_connections())
				{
					netlist.log().verbose("   ==> not a rail net");
					// Must be an analog net
					auto &n = *static_cast<analog_net_t *>(net.get());
					if (!already_processed(n))
					{
						groupspre.emplace_back(analog_net_t::list_t());
						process_net(netlist, n);
					}
				}
			}
			for (auto &g : groupspre)
				if (!g.empty())
					groups.push_back(g);
		}

		std::vector<analog_net_t::list_t> groups;

	private:

		bool already_processed(const analog_net_t &n) const
		{
			// no need to process rail nets - these are known variables
			if (n.is_rail_net())
				return true;
			// if it's already processed - no need to continue
			for (const auto & grp : groups)
				if (plib::container::contains(grp, &n))
					return true;
			return false;
		}

		bool check_if_processed_and_join(const analog_net_t &n)
		{
			// no need to process rail nets - these are known variables
			if (n.is_rail_net())
				return true;
			// First check if it is in a previous group.
			// In this case we need to merge this group into the current group
			if (groupspre.size() > 1)
			{
				for (std::size_t i = 0; i<groupspre.size() - 1; i++)
					if (plib::container::contains(groupspre[i], &n))
					{
						// copy all nets
						for (auto & cn : groupspre[i])
							if (!plib::container::contains(groupspre.back(), cn))
								groupspre.back().push_back(cn);
						// clear
						groupspre[i].clear();
						return true;
					}
			}
			// if it's already processed - no need to continue
			if (!groupspre.empty() && plib::container::contains(groupspre.back(), &n))
				return true;
			return false;
		}

		void process_net(netlist_state_t &netlist, analog_net_t &n)
		{
			// ignore empty nets. FIXME: print a warning message
			netlist.log().verbose("Net {}", n.name());
			if (n.has_connections())
			{
				// add the net
				groupspre.back().push_back(&n);
				// process all terminals connected to this net
				for (auto &term : n.core_terms())
				{
					netlist.log().verbose("Term {} {}", term->name(), static_cast<int>(term->type()));
					// only process analog terminals
					if (term->is_type(detail::terminal_type::TERMINAL))
					{
						auto *pt = static_cast<terminal_t *>(term);
						// check the connected terminal
						analog_net_t &connected_net = netlist.setup().get_connected_terminal(*pt)->net();
						netlist.log().verbose("  Connected net {}", connected_net.name());
						if (!check_if_processed_and_join(connected_net))
							process_net(netlist, connected_net);
					}
				}
			}
		}

		std::vector<analog_net_t::list_t> groupspre;
	};

	void NETLIB_NAME(solver)::post_start()
	{
		log().verbose("Scanning net groups ...");
		// determine net groups

		net_splitter splitter;

		splitter.run(state());

		// setup the solvers
		log().verbose("Found {1} net groups in {2} nets\n", splitter.groups.size(), state().nets().size());
		for (auto & grp : splitter.groups)
		{
			plib::unique_ptr<solver::matrix_solver_t> ms;
			pstring sname = plib::pfmt("Solver_{1}")(m_mat_solvers.size());

			switch (m_params.m_fp_type())
			{
				case solver::matrix_fp_type_e::FLOAT:
#if (NL_USE_FLOAT_MATRIX)
					ms = create_solvers<float>(sname, grp);
#else
					log().info("FPTYPE {1} not supported. Using DOUBLE", m_params.m_fp_type().name());
					ms = create_solvers<double>(sname, grp);
#endif
					break;
				case solver::matrix_fp_type_e::DOUBLE:
					ms = create_solvers<double>(sname, grp);
					break;
				case solver::matrix_fp_type_e::LONGDOUBLE:
#if (NL_USE_LONG_DOUBLE_MATRIX)
					ms = create_solvers<long double>(sname, grp);
#else
					log().info("FPTYPE {1} not supported. Using DOUBLE", m_params.m_fp_type().name());
					ms = create_solvers<double>(sname, grp);
#endif
					break;
				case solver::matrix_fp_type_e::FLOATQ128:
#if (NL_USE_FLOAT128)
					ms = create_solvers<FLOAT128>(sname, grp);
#else
					log().info("FPTYPE {1} not supported. Using DOUBLE", m_params.m_fp_type().name());
					ms = create_solvers<double>(sname, grp);
#endif
					break;
			}

			log().verbose("Solver {1}", ms->name());
			log().verbose("       ==> {1} nets", grp.size());
			log().verbose("       has {1} dynamic elements", ms->dynamic_device_count());
			log().verbose("       has {1} timestep elements", ms->timestep_device_count());
			for (auto &n : grp)
			{
				log().verbose("Net {1}", n->name());
				for (const auto &pcore : n->core_terms())
				{
					log().verbose("   {1}", pcore->name());
				}
			}

			m_mat_solvers_all.push_back(ms.get());
			if (ms->timestep_device_count() != 0)
				m_mat_solvers_timestepping.push_back(ms.get());

			m_mat_solvers.emplace_back(std::move(ms));
		}
	}

	solver::static_compile_container NETLIB_NAME(solver)::create_solver_code(solver::static_compile_target target)
	{
		solver::static_compile_container mp;
		for (auto & s : m_mat_solvers)
		{
			auto r = s->create_solver_code(target);
			if (r.first != "") // ignore solvers not supporting static compile
				mp.push_back(r);
		}
		return mp;
	}

	NETLIB_DEVICE_IMPL(solver, "SOLVER", "FREQ")

} // namespace devices
} // namespace netlist
