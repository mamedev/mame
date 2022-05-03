// license:BSD-3-Clause
// copyright-holders:Couriersud


#include "nl_factory.h"
#include "core/setup.h"
#include "nl_errstr.h"
#include "nl_setup.h" // FIXME: only needed for splitter code
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
#include "plib/ptimed_queue.h"

#include <algorithm>
#include <type_traits>

namespace netlist
{
namespace devices
{

	// ----------------------------------------------------------------------------------------
	// solver
	// ----------------------------------------------------------------------------------------

	NETLIB_RESET(solver)
	{
		if (exec().stats_enabled())
			m_fb_step.set_delegate(NETLIB_DELEGATE(fb_step<true>));
		for (auto &s : m_mat_solvers)
			s->reset();
		for (auto &s : m_mat_solvers)
			m_queue.push<false>({netlist_time_ext::zero(), s.get()});
	}

	void NETLIB_NAME(solver)::stop()
	{
		for (auto &s : m_mat_solvers)
			s->log_stats();
	}

#if 1

	template<bool KEEP_STATS>
	NETLIB_HANDLER(solver, fb_step)
	{
		const netlist_time_ext now(exec().time());
		const std::size_t nthreads = m_params.m_parallel() < 2 ? 1 : std::min(static_cast<std::size_t>(m_params.m_parallel()), plib::omp::get_max_threads());
		const netlist_time_ext sched(now + (nthreads <= 1 ? netlist_time_ext::zero() : netlist_time_ext::from_nsec(100)));
		plib::uninitialised_array<solver::matrix_solver_t *, config::max_solver_queue_size::value> tmp; //NOLINT
		plib::uninitialised_array<netlist_time, config::max_solver_queue_size::value> nt; //NOLINT
		std::size_t p=0;

		while (!m_queue.empty())
		{
			const auto t = m_queue.top().exec_time();
			auto *o = m_queue.top().object();
			if (t != now)
				if (t > sched)
					break;
			tmp[p++] = o;
			m_queue.pop();
		}

		// FIXME: Disabled for now since parallel processing will decrease performance
		//        for tested applications. More testing required here
		if (true || nthreads < 2)
		{
			if (!KEEP_STATS)
			{
				for (std::size_t i = 0; i < p; i++)
						nt[i] = tmp[i]->solve(now, "no-parallel");
			}
			else
			{
				stats()->m_stat_total_time.stop();
				for (std::size_t i = 0; i < p; i++)
				{
					tmp[i]->stats()->m_stat_call_count.inc();
					auto g(tmp[i]->stats()->m_stat_total_time.guard());
					nt[i] = tmp[i]->solve(now, "no-parallel");
				}
				stats()->m_stat_total_time.start();
			}

			for (std::size_t i = 0; i < p; i++)
			{
				if (nt[i] != netlist_time::zero())
					m_queue.push<false>({now + nt[i], tmp[i]});
				tmp[i]->update_inputs();
			}
		}
		else
		{
			plib::omp::set_num_threads(nthreads);
			plib::omp::for_static(static_cast<std::size_t>(0), p, [&tmp, &nt,now](std::size_t i)
				{
					nt[i] = tmp[i]->solve(now, "parallel");
				});
			for (std::size_t i = 0; i < p; i++)
			{
				if (nt[i] != netlist_time::zero())
					m_queue.push<false>({now + nt[i], tmp[i]});
				tmp[i]->update_inputs();
			}
		}
		if (!m_queue.empty())
			m_Q_step.net().toggle_and_push_to_queue(static_cast<netlist_time>(m_queue.top().exec_time() - now));
	}

	void NETLIB_NAME(solver) :: reschedule(solver::matrix_solver_t *solv, netlist_time ts)
	{
		const netlist_time_ext now(exec().time());
		const netlist_time_ext sched(now + ts);
		m_queue.remove<false>(solv);
		m_queue.push<false>({sched, solv});

		if (m_Q_step.net().is_queued())
		{
			if (m_Q_step.net().next_scheduled_time() > sched)
				m_Q_step.net().toggle_and_push_to_queue(ts);
		}
		else
			m_Q_step.net().toggle_and_push_to_queue(ts);
	}
#else
	NETLIB_HANDLER(solver, fb_step)
	{
		if (m_params.m_dynamic_ts)
			return;

		netlist_time_ext now(exec().time());
		// force solving during start up if there are no time-step devices
		// FIXME: Needs a more elegant solution
		bool force_solve = (now < netlist_time_ext::from_fp<decltype(m_params.m_max_timestep)>(2 * m_params.m_max_timestep));

		std::size_t nthreads = std::min(static_cast<std::size_t>(m_params.m_parallel()), plib::omp::get_max_threads());

		std::vector<solver_entry *> &solvers = (force_solve ? m_mat_solvers_all : m_mat_solvers_timestepping);

		if (nthreads > 1 && solvers.size() > 1)
		{
			plib::omp::set_num_threads(nthreads);
			plib::omp::for_static(static_cast<std::size_t>(0), solvers.size(), [&solvers, now](std::size_t i)
				{
					const netlist_time ts = solvers[i]->ptr->solve(now);
					plib::unused_var(ts);
				});
		}
		else
			for (auto & solver : solvers)
			{
				const netlist_time ts = solver->ptr->solve(now);
				plib::unused_var(ts);
			}

		for (auto & solver : solvers)
			solver->ptr->update_inputs();

		// step circuit
		if (!m_Q_step.net().is_queued())
		{
			m_Q_step.net().toggle_and_push_to_queue(netlist_time::from_fp(m_params.m_max_timestep));
		}
	}
#endif

	// FIXME: should be created in device space
	template <class C>
	NETLIB_NAME(solver)::solver_ptr create_it(NETLIB_NAME(solver) &main_solver, pstring name,
		NETLIB_NAME(solver)::net_list_t &nets,
		const solver::solver_parameters_t *params, std::size_t size)
	{
		return plib::make_unique<C, device_arena>(main_solver, name, nets, params, size);
	}

	template <typename FT, int SIZE>
	NETLIB_NAME(solver)::solver_ptr NETLIB_NAME(solver)::create_solver(std::size_t size,
		const pstring &solvername, const solver::solver_parameters_t *params,
		NETLIB_NAME(solver)::net_list_t &nets)
	{
		switch (params->m_method())
		{
			case solver::matrix_type_e::MAT_CR:
				return create_it<solver::matrix_solver_GCR_t<FT, SIZE>>(*this, solvername, nets, params, size);
			case solver::matrix_type_e::MAT:
				return create_it<solver::matrix_solver_direct_t<FT, SIZE>>(*this, solvername, nets, params, size);
			case solver::matrix_type_e::GMRES:
				return create_it<solver::matrix_solver_GMRES_t<FT, SIZE>>(*this, solvername, nets, params, size);
#if (NL_USE_ACADEMIC_SOLVERS)
			case solver::matrix_type_e::SOR:
				return create_it<solver::matrix_solver_SOR_t<FT, SIZE>>(*this, solvername, nets, params, size);
			case solver::matrix_type_e::SOR_MAT:
				return create_it<solver::matrix_solver_SOR_mat_t<FT, SIZE>>(*this, solvername, nets, params, size);
			case solver::matrix_type_e::SM:
				// Sherman-Morrison Formula
				return create_it<solver::matrix_solver_sm_t<FT, SIZE>>(*this, solvername, nets, params, size);
			case solver::matrix_type_e::W:
				// Woodbury Formula
				return create_it<solver::matrix_solver_w_t<FT, SIZE>>(*this, solvername, nets, params, size);
#else
			//case solver::matrix_type_e::GMRES:
			case solver::matrix_type_e::SOR:
			case solver::matrix_type_e::SOR_MAT:
			case solver::matrix_type_e::SM:
			case solver::matrix_type_e::W:
				state().log().warning(MW_SOLVER_METHOD_NOT_SUPPORTED(params->m_method().name(), "MAT_CR"));
				return create_it<solver::matrix_solver_GCR_t<FT, SIZE>>(*this, solvername, nets, params, size);
#endif
		}
		return solver_ptr();
	}

	template <typename FT>
	NETLIB_NAME(solver)::solver_ptr NETLIB_NAME(solver)::create_solvers(
		const pstring &sname, const solver::solver_parameters_t *params,
		net_list_t &nets)
	{
		std::size_t net_count = nets.size();
		switch (net_count)
		{
#if !defined(__EMSCRIPTEN__)
			case 1:
				return plib::make_unique<solver::matrix_solver_direct1_t<FT>, device_arena>(*this, sname, nets, params);
			case 2:
				return plib::make_unique<solver::matrix_solver_direct2_t<FT>, device_arena>(*this, sname, nets, params);
			case 3:
				return create_solver<FT, 3>(3, sname, params, nets);
			case 4:
				return create_solver<FT, 4>(4, sname, params, nets);
			case 5:
				return create_solver<FT, 5>(5, sname, params, nets);
			case 6:
				return create_solver<FT, 6>(6, sname, params, nets);
			case 7:
				return create_solver<FT, 7>(7, sname, params, nets);
			case 8:
				return create_solver<FT, 8>(8, sname, params, nets);
#endif
			default:
				log().info(MI_NO_SPECIFIC_SOLVER(net_count));
				if (net_count <= 16)
				{
					return create_solver<FT, -16>(net_count, sname, params, nets);
				}
				if (net_count <= 32)
				{
					return create_solver<FT, -32>(net_count, sname, params, nets);
				}
				if (net_count <= 64)
				{
					return create_solver<FT, -64>(net_count, sname, params, nets);
				}
				if (net_count <= 128)
				{
					return create_solver<FT, -128>(net_count, sname, params, nets);
				}
				if (net_count <= 256)
				{
					return create_solver<FT, -256>(net_count, sname, params, nets);
				}
				if (net_count <= 512)
				{
					return create_solver<FT, -512>(net_count, sname, params, nets);
				}
				return create_solver<FT, 0>(net_count, sname, params, nets);
		}
	}

	struct net_splitter
	{
		void run(netlist_state_t &nlstate)
		{
			for (auto & net : nlstate.nets())
			{
				nlstate.log().verbose("processing {1}", net->name());
				if (!net->is_rail_net() && !nlstate.core_terms(*net).empty())
				{
					nlstate.log().verbose("   ==> not a rail net");
					// Must be an analog net
					auto &n = dynamic_cast<analog_net_t &>(*net);
					if (!already_processed(n))
					{
						groupspre.emplace_back(NETLIB_NAME(solver)::net_list_t());
						process_net(nlstate, n);
					}
				}
			}
			for (auto &g : groupspre)
				if (!g.empty())
					groups.push_back(g);
		}

		std::vector<NETLIB_NAME(solver)::net_list_t> groups;

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

		// NOLINTNEXTLINE(misc-no-recursion)
		void process_net(netlist_state_t &nlstate, analog_net_t &n)
		{
			// ignore empty nets. FIXME: print a warning message
			nlstate.log().verbose("Net {}", n.name());
			if (!nlstate.core_terms(n).empty())
			{
				// add the net
				groupspre.back().push_back(&n);
				// process all terminals connected to this net
				for (auto &term : nlstate.core_terms(n))
				{
					nlstate.log().verbose("Term {} {}", term->name(), static_cast<int>(term->type()));
					// only process analog terminals
					if (term->is_type(detail::terminal_type::TERMINAL))
					{
						auto &pt = dynamic_cast<terminal_t &>(*term);
						// check the connected terminal
						const auto *const connected_terminals = nlstate.setup().get_connected_terminals(pt);
						// NOLINTNEXTLINE proposal does not work for VS
						for (auto ct = connected_terminals->begin(); *ct != nullptr; ct++)
						{
							analog_net_t &connected_net = (*ct)->net();
							nlstate.log().verbose("  Connected net {}", connected_net.name());
							if (!check_if_processed_and_join(connected_net))
								process_net(nlstate, connected_net);
						}
					}
				}
			}
		}

		std::vector<NETLIB_NAME(solver)::net_list_t> groupspre;
	};

	void NETLIB_NAME(solver)::post_start()
	{
		log().verbose("Scanning net groups ...");
		// determine net groups

		net_splitter splitter;

		splitter.run(state());
		log().verbose("Found {1} net groups in {2} nets\n", splitter.groups.size(), state().nets().size());

		int num_errors = 0;

		log().verbose("checking net consistency  ...");
		for (const auto &grp : splitter.groups)
		{
			int railterms = 0;
			pstring nets_in_grp;
			for (const auto &n : grp)
			{
				nets_in_grp += (n->name() + " ");
				if (!n->is_analog())
				{
					state().log().error(ME_SOLVER_CONSISTENCY_NOT_ANALOG_NET(n->name()));
					num_errors++;
				}
				if (n->is_rail_net())
				{
					state().log().error(ME_SOLVER_CONSISTENCY_RAIL_NET(n->name()));
					num_errors++;
				}
				for (const auto &t : state().core_terms(*n))
				{
					if (!t->has_net())
					{
						state().log().error(ME_SOLVER_TERMINAL_NO_NET(t->name()));
						num_errors++;
					}
					else
					{
						auto *otherterm = dynamic_cast<terminal_t *>(t);
						if (otherterm != nullptr)
							if (state().setup().get_connected_terminal(*otherterm)->net().is_rail_net())
								railterms++;
					}
				}
			}
			if (railterms == 0)
			{
				state().log().error(ME_SOLVER_NO_RAIL_TERMINAL(nets_in_grp));
				num_errors++;
			}
		}
		if (num_errors > 0)
			throw nl_exception(MF_SOLVER_CONSISTENCY_ERRORS(num_errors));


		// setup the solvers
		for (auto & grp : splitter.groups)
		{
			solver_ptr ms;
			pstring sname = plib::pfmt("Solver_{1}")(m_mat_solvers.size());
			params_uptr params = plib::make_unique<solver::solver_parameters_t, solver_arena>(*this, sname + ".", m_params);

			switch (params->m_fp_type())
			{
				case solver::matrix_fp_type_e::FLOAT:
					if (!config::use_float_matrix())
						log().info("FPTYPE {1} not supported. Using DOUBLE", params->m_fp_type().name());
					ms = create_solvers<std::conditional_t<config::use_float_matrix::value, float, double>>(sname, params.get(), grp);
					break;
				case solver::matrix_fp_type_e::DOUBLE:
					ms = create_solvers<double>(sname, params.get(), grp);
					break;
				case solver::matrix_fp_type_e::LONGDOUBLE:
					if (!config::use_long_double_matrix())
						log().info("FPTYPE {1} not supported. Using DOUBLE", params->m_fp_type().name());
					ms = create_solvers<std::conditional_t<config::use_long_double_matrix::value, long double, double>>(sname, params.get(), grp);
					break;
				case solver::matrix_fp_type_e::FLOATQ128:
#if (NL_USE_FLOAT128)
					ms = create_solvers<FLOAT128>(sname, params.get(), grp);
#else
					log().info("FPTYPE {1} not supported. Using DOUBLE", params->m_fp_type().name());
					ms = create_solvers<double>(sname, params.get(), grp);
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
				for (const auto &t : state().core_terms(*n))
				{
					log().verbose("   {1}", t->name());
				}
			}

			m_mat_params.push_back(std::move(params));
			m_mat_solvers.push_back(std::move(ms));
		}

	}

	solver::static_compile_container NETLIB_NAME(solver)::create_solver_code(solver::static_compile_target target)
	{
		solver::static_compile_container mp;
		for (auto & s : m_mat_solvers)
		{
			auto r = s->create_solver_code(target);
			if (!r.first.empty()) // ignore solvers not supporting static compile
				mp.push_back(r);
		}
		return mp;
	}

	std::size_t NETLIB_NAME(solver)::get_solver_id(const solver::matrix_solver_t *net) const
	{
		for (std::size_t i=0; i < m_mat_solvers.size(); i++)
			if (m_mat_solvers[i].get() == net)
				return i;
		return std::numeric_limits<std::size_t>::max();
	}

	solver::matrix_solver_t * NETLIB_NAME(solver)::solver_by_id(std::size_t id) const
	{
		return m_mat_solvers[id].get();
	}


	NETLIB_DEVICE_IMPL(solver, "SOLVER", "FREQ")

} // namespace devices
} // namespace netlist
