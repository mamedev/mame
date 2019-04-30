// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_matrix_solver.cpp
 *
 */

#include "nld_matrix_solver.h"
#include "plib/putil.h"

#include <cmath>  // <<= needed by windows build

namespace netlist
{
namespace devices
{

	terms_for_net_t::terms_for_net_t()
		: m_railstart(0)
		, m_last_V(0.0)
		, m_DD_n_m_1(0.0)
		, m_h_n_m_1(1e-12)
	{
	}

	void terms_for_net_t::add(terminal_t *term, int net_other, bool sorted)
	{
		if (sorted)
			for (std::size_t i=0; i < m_connected_net_idx.size(); i++)
			{
				if (m_connected_net_idx[i] > net_other)
				{
					plib::container::insert_at(m_terms, i, term);
					plib::container::insert_at(m_connected_net_idx, i, net_other);
					return;
				}
			}
		m_terms.push_back(term);
		m_connected_net_idx.push_back(net_other);
	}

	// ----------------------------------------------------------------------------------------
	// matrix_solver
	// ----------------------------------------------------------------------------------------

	matrix_solver_t::matrix_solver_t(netlist_state_t &anetlist, const pstring &name,
			const eSortType sort, const solver_parameters_t *params)
		: device_t(anetlist, name)
		, m_params(*params)
		, m_stat_calculations(*this, "m_stat_calculations", 0)
		, m_stat_newton_raphson(*this, "m_stat_newton_raphson", 0)
		, m_stat_vsolver_calls(*this, "m_stat_vsolver_calls", 0)
		, m_iterative_fail(*this, "m_iterative_fail", 0)
		, m_iterative_total(*this, "m_iterative_total", 0)
		, m_last_step(*this, "m_last_step", netlist_time::zero())
		, m_fb_sync(*this, "FB_sync")
		, m_Q_sync(*this, "Q_sync")
		, m_ops(0)
		, m_sort(sort)
	{
		connect_post_start(m_fb_sync, m_Q_sync);
	}

	void matrix_solver_t::setup_base(analog_net_t::list_t &nets)
	{

		log().debug("New solver setup\n");

		m_nets.clear();
		m_terms.clear();

		for (auto & net : nets)
		{
			m_nets.push_back(net);
			m_terms.push_back(plib::make_unique<terms_for_net_t>());
			m_rails_temp.push_back(plib::make_unique<terms_for_net_t>());
		}

		for (std::size_t k = 0; k < nets.size(); k++)
		{
			analog_net_t *net = nets[k];

			log().debug("setting up net\n");

			net->set_solver(this);

			for (auto &p : net->core_terms())
			{
				log().debug("{1} {2} {3}\n", p->name(), net->name(), net->isRailNet());
				switch (p->type())
				{
					case detail::terminal_type::TERMINAL:
						if (p->device().is_timestep())
							if (!plib::container::contains(m_step_devices, &p->device()))
								m_step_devices.push_back(&p->device());
						if (p->device().is_dynamic())
							if (!plib::container::contains(m_dynamic_devices, &p->device()))
								m_dynamic_devices.push_back(&p->device());
						{
							auto *pterm = dynamic_cast<terminal_t *>(p);
							add_term(k, pterm);
						}
						log().debug("Added terminal {1}\n", p->name());
						break;
					case detail::terminal_type::INPUT:
						{
							proxied_analog_output_t *net_proxy_output = nullptr;
							for (auto & input : m_inps)
								if (input->proxied_net() == &p->net())
								{
									net_proxy_output = input.get();
									break;
								}

							if (net_proxy_output == nullptr)
							{
								pstring nname = this->name() + "." + pstring(plib::pfmt("m{1}")(m_inps.size()));
								nl_assert(p->net().is_analog());
								auto net_proxy_output_u = pool().make_poolptr<proxied_analog_output_t>(*this, nname, static_cast<analog_net_t *>(&p->net()));
								net_proxy_output = net_proxy_output_u.get();
								m_inps.push_back(std::move(net_proxy_output_u));
							}
							net_proxy_output->net().add_terminal(*p);
							// FIXME: repeated calling - kind of brute force
							net_proxy_output->net().rebuild_list();
							log().debug("Added input\n");
						}
						break;
					case detail::terminal_type::OUTPUT:
						log().fatal(MF_UNHANDLED_ELEMENT_1_FOUND(p->name()));
						break;
				}
			}
			log().debug("added net with {1} populated connections\n", net->core_terms().size());
		}

		/* now setup the matrix */
		setup_matrix();
	}

	void matrix_solver_t::sort_terms(eSortType sort)
	{
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
		 * NOTE: Even better would be to sort on elements right of the matrix diagonal.
		 *
		 */

		const std::size_t iN = m_nets.size();

		switch (sort)
		{
			case PREFER_BAND_MATRIX:
				{
					for (std::size_t k = 0; k < iN - 1; k++)
					{
						auto pk = get_weight_around_diag(k,k);
						for (std::size_t i = k+1; i < iN; i++)
						{
							auto pi = get_weight_around_diag(i,k);
							if (pi < pk)
							{
								std::swap(m_terms[i], m_terms[k]);
								std::swap(m_nets[i], m_nets[k]);
								pk = get_weight_around_diag(k,k);
							}
						}
					}
				}
				break;
			case PREFER_IDENTITY_TOP_LEFT:
				{
					for (std::size_t k = 0; k < iN - 1; k++)
					{
						auto pk = get_left_right_of_diag(k,k);
						for (std::size_t i = k+1; i < iN; i++)
						{
							auto pi = get_left_right_of_diag(i,k);
							if (pi.first <= pk.first && pi.second >= pk.second)
							{
								std::swap(m_terms[i], m_terms[k]);
								std::swap(m_nets[i], m_nets[k]);
								pk = get_left_right_of_diag(k,k);
							}
						}
					}
				}
				break;
			case ASCENDING:
			case DESCENDING:
				{
					int sort_order = (m_sort == DESCENDING ? 1 : -1);

					for (std::size_t k = 0; k < iN - 1; k++)
						for (std::size_t i = k+1; i < iN; i++)
						{
							if ((static_cast<int>(m_terms[k]->m_railstart) - static_cast<int>(m_terms[i]->m_railstart)) * sort_order < 0)
							{
								std::swap(m_terms[i], m_terms[k]);
								std::swap(m_nets[i], m_nets[k]);
							}
						}
				}
				break;
			case NOSORT:
				break;
		}
		/* rebuild */
		for (auto &term : m_terms)
		{
			int *other = term->m_connected_net_idx.data();
			for (std::size_t i = 0; i < term->count(); i++)
				//FIXME: this is weird
				if (other[i] != -1)
					other[i] = get_net_idx(&term->terms()[i]->connected_terminal()->net());
		}
	}

	void matrix_solver_t::setup_matrix()
	{
		const std::size_t iN = m_nets.size();

		for (std::size_t k = 0; k < iN; k++)
		{
			m_terms[k]->m_railstart = m_terms[k]->count();
			for (std::size_t i = 0; i < m_rails_temp[k]->count(); i++)
				this->m_terms[k]->add(m_rails_temp[k]->terms()[i], m_rails_temp[k]->m_connected_net_idx.data()[i], false);
		}

		// free all - no longer needed
		m_rails_temp.clear();

		sort_terms(m_sort);

		this->set_pointers();

		/* create a list of non zero elements. */
		for (unsigned k = 0; k < iN; k++)
		{
			terms_for_net_t * t = m_terms[k].get();
			/* pretty brutal */
			int *other = t->m_connected_net_idx.data();

			t->m_nz.clear();

			for (std::size_t i = 0; i < t->m_railstart; i++)
				if (!plib::container::contains(t->m_nz, static_cast<unsigned>(other[i])))
					t->m_nz.push_back(static_cast<unsigned>(other[i]));

			t->m_nz.push_back(k);     // add diagonal

			/* and sort */
			std::sort(t->m_nz.begin(), t->m_nz.end());
		}

		/* create a list of non zero elements right of the diagonal
		 * These list anticipate the population of array elements by
		 * Gaussian elimination.
		 */
		for (std::size_t k = 0; k < iN; k++)
		{
			terms_for_net_t * t = m_terms[k].get();
			/* pretty brutal */
			int *other = t->m_connected_net_idx.data();

			if (k==0)
				t->m_nzrd.clear();
			else
			{
				t->m_nzrd = m_terms[k-1]->m_nzrd;
				for (auto j = t->m_nzrd.begin(); j != t->m_nzrd.end(); )
				{
					if (*j < k + 1)
						j = t->m_nzrd.erase(j);
					else
						++j;
				}
			}

			for (std::size_t i = 0; i < t->m_railstart; i++)
				if (!plib::container::contains(t->m_nzrd, static_cast<unsigned>(other[i])) && other[i] >= static_cast<int>(k + 1))
					t->m_nzrd.push_back(static_cast<unsigned>(other[i]));

			/* and sort */
			std::sort(t->m_nzrd.begin(), t->m_nzrd.end());
		}

		/* create a list of non zero elements below diagonal k
		 * This should reduce cache misses ...
		 */

		std::vector<std::vector<bool>> touched(iN, std::vector<bool>(iN));

		for (std::size_t k = 0; k < iN; k++)
		{
			for (std::size_t j = 0; j < iN; j++)
				touched[k][j] = false;
			for (std::size_t j = 0; j < m_terms[k]->m_nz.size(); j++)
				touched[k][m_terms[k]->m_nz[j]] = true;
		}

		m_ops = 0;
		for (unsigned k = 0; k < iN; k++)
		{
			m_ops++; // 1/A(k,k)
			for (unsigned row = k + 1; row < iN; row++)
			{
				if (touched[row][k])
				{
					m_ops++;
					if (!plib::container::contains(m_terms[k]->m_nzbd, row))
						m_terms[k]->m_nzbd.push_back(row);
					for (std::size_t col = k + 1; col < iN; col++)
						if (touched[k][col])
						{
							touched[row][col] = true;
							m_ops += 2;
						}
				}
			}
		}
		log().verbose("Number of mults/adds for {1}: {2}", name(), m_ops);

		if ((false))
			for (std::size_t k = 0; k < iN; k++)
			{
				pstring line = plib::pfmt("{1:3}")(k);
				for (const auto & nzrd : m_terms[k]->m_nzrd)
					line += plib::pfmt(" {1:3}")(nzrd);
				log().verbose("{1}", line);
			}

		/*
		 * save states
		 */
		for (std::size_t k = 0; k < iN; k++)
		{
			pstring num = plib::pfmt("{1}")(k);

			state().save(*this, m_terms[k]->m_last_V, this->name(), "lastV." + num);
			state().save(*this, m_terms[k]->m_DD_n_m_1, this->name(), "m_DD_n_m_1." + num);
			state().save(*this, m_terms[k]->m_h_n_m_1, this->name(), "m_h_n_m_1." + num);

			// FIXME: This shouldn't be necessary, recalculate on each entry ...
			state().save(*this, m_gonn[k],"GO" + num, this->name(), m_terms[k]->count());
			state().save(*this, m_gtn[k],"GT" + num, this->name(), m_terms[k]->count());
			state().save(*this, m_Idrn[k],"IDR" + num, this->name(), m_terms[k]->count());
		}
	}

	void matrix_solver_t::update_inputs()
	{
		// avoid recursive calls. Inputs are updated outside this call
		for (auto &inp : m_inps)
			inp->push(inp->proxied_net()->Q_Analog());
	}

	void matrix_solver_t::update_dynamic()
	{
		/* update all non-linear devices  */
		for (auto &dyn : m_dynamic_devices)
			dyn->update_terminals();
	}

	void matrix_solver_t::reset()
	{
		m_last_step = netlist_time::zero();
	}

	void matrix_solver_t::update() NL_NOEXCEPT
	{
		const netlist_time new_timestep = solve(exec().time());
		update_inputs();

		if (m_params.m_dynamic_ts && has_timestep_devices() && new_timestep > netlist_time::zero())
		{
			m_Q_sync.net().toggle_and_push_to_queue(new_timestep);
		}
	}

	/* update_forced is called from within param_update
	 *
	 * this should only occur outside of execution and thus
	 * using time should be safe.
	 *
	 */
	void matrix_solver_t::update_forced()
	{
		const netlist_time new_timestep = solve(exec().time());
		plib::unused_var(new_timestep);

		update_inputs();

		if (m_params.m_dynamic_ts && has_timestep_devices())
		{
			m_Q_sync.net().toggle_and_push_to_queue(netlist_time::from_double(m_params.m_min_timestep));
		}
	}

	void matrix_solver_t::step(const netlist_time &delta)
	{
		const nl_double dd = delta.as_double();
		for (auto &d : m_step_devices)
			d->timestep(dd);
	}

	void matrix_solver_t::solve_base()
	{
		++m_stat_vsolver_calls;
		if (has_dynamic_devices())
		{
			std::size_t this_resched;
			std::size_t newton_loops = 0;
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
				log().warning(MW_NEWTON_LOOPS_EXCEEDED_ON_NET_1(this->name()));
				m_Q_sync.net().toggle_and_push_to_queue(m_params.m_nr_recalc_delay);
			}
		}
		else
		{
			this->vsolve_non_dynamic(false);
		}
	}

	const netlist_time matrix_solver_t::solve(netlist_time now)
	{
		const netlist_time delta = now - m_last_step;

		// We are already up to date. Avoid oscillations.
		// FIXME: Make this a parameter!
		if (delta < netlist_time::quantum())
			return netlist_time::zero();

		/* update all terminals for new time step */
		m_last_step = now;
		step(delta);
		solve_base();
		const netlist_time next_time_step = compute_next_timestep(delta.as_double());

		return next_time_step;
	}

	int matrix_solver_t::get_net_idx(detail::net_t *net)
	{
		for (std::size_t k = 0; k < m_nets.size(); k++)
			if (m_nets[k] == net)
				return static_cast<int>(k);
		return -1;
	}

	std::pair<int, int> matrix_solver_t::get_left_right_of_diag(std::size_t irow, std::size_t idiag)
	{
		/*
		 * return the maximum column left of the diagonal (-1 if no cols found)
		 * return the minimum column right of the diagonal (999999 if no cols found)
		 */

		const auto row = static_cast<int>(irow);
		const auto diag = static_cast<int>(idiag);

		int colmax = -1;
		int colmin = 999999;

		auto &term = m_terms[irow];

		for (std::size_t i = 0; i < term->count(); i++)
		{
			auto col = get_net_idx(&term->terms()[i]->connected_terminal()->net());
			if (col != -1)
			{
				if (col==row) col = diag;
				else if (col==diag) col = row;

				if (col > diag && col < colmin)
					colmin = col;
				else if (col < diag && col > colmax)
					colmax = col;
			}
		}
		return {colmax, colmin};
	}

	double matrix_solver_t::get_weight_around_diag(std::size_t row, std::size_t diag)
	{
		{
			/*
			 * return average absolute distance
			 */

			std::vector<bool> touched(1024, false); // FIXME!

			double weight = 0.0;
			auto &term = m_terms[row];
			for (std::size_t i = 0; i < term->count(); i++)
			{
				auto col = get_net_idx(&term->terms()[i]->connected_terminal()->net());
				if (col >= 0)
				{
					auto colu = static_cast<std::size_t>(col);
					if (!touched[colu])
					{
						if (colu==row) colu = static_cast<unsigned>(diag);
						else if (colu==diag) colu = static_cast<unsigned>(row);

						weight = weight + std::abs(static_cast<double>(colu) - static_cast<double>(diag));
						touched[colu] = true;
					}
				}
			}
			return weight; // / static_cast<double>(term->m_railstart);
		}
	}

	void matrix_solver_t::add_term(std::size_t k, terminal_t *term)
	{
		if (term->connected_terminal()->net().isRailNet())
		{
			m_rails_temp[k]->add(term, -1, false);
		}
		else
		{
			int ot = get_net_idx(&term->connected_terminal()->net());
			if (ot>=0)
			{
				m_terms[k]->add(term, ot, true);
			}
			/* Should this be allowed ? */
			else // if (ot<0)
			{
				m_rails_temp[k]->add(term, ot, true);
				log().fatal(MF_FOUND_TERM_WITH_MISSING_OTHERNET(term->name()));
			}
		}
	}

	netlist_time matrix_solver_t::compute_next_timestep(const double cur_ts)
	{
		nl_double new_solver_timestep = m_params.m_max_timestep;

		if (m_params.m_dynamic_ts)
		{
			for (std::size_t k = 0, iN=m_terms.size(); k < iN; k++)
			{
				analog_net_t *n = m_nets[k];
				terms_for_net_t *t = m_terms[k].get();

				//const nl_double DD_n = (n->Q_Analog() - t->m_last_V);
				// avoid floating point exceptions
				const nl_double DD_n = std::max(-1e100, std::min(1e100,(n->Q_Analog() - t->m_last_V)));
				const nl_double hn = cur_ts;

				//printf("%g %g %g %g\n", DD_n, hn, t->m_DD_n_m_1, t->m_h_n_m_1);
				nl_double DD2 = (DD_n / hn - t->m_DD_n_m_1 / t->m_h_n_m_1) / (hn + t->m_h_n_m_1);
				nl_double new_net_timestep;

				t->m_h_n_m_1 = hn;
				t->m_DD_n_m_1 = DD_n;
				if (std::fabs(DD2) > plib::constants<nl_double>::cast(1e-60)) // avoid div-by-zero
					new_net_timestep = std::sqrt(m_params.m_dynamic_lte / std::fabs(plib::constants<nl_double>::cast(0.5)*DD2));
				else
					new_net_timestep = m_params.m_max_timestep;

				if (new_net_timestep < new_solver_timestep)
					new_solver_timestep = new_net_timestep;

				t->m_last_V = n->Q_Analog();
			}
			if (new_solver_timestep < m_params.m_min_timestep)
			{
				//log().warning("Dynamic timestep below min timestep. Consider decreasing MIN_TIMESTEP: {1} us", new_solver_timestep*1.0e6);
				new_solver_timestep = m_params.m_min_timestep;
			}
		}
		//if (new_solver_timestep > 10.0 * hn)
		//    new_solver_timestep = 10.0 * hn;
		/*
		 * FIXME: Factor 2 below is important. Without, we get timing issues. This must be a bug elsewhere.
		 */
		return std::max(netlist_time::from_double(new_solver_timestep), netlist_time::quantum() * 2);
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
			log().verbose("       {1:6.3} average newton raphson loops",
						static_cast<double>(this->m_stat_newton_raphson) / static_cast<double>(this->m_stat_vsolver_calls));
			log().verbose("       {1:10} invocations ({2:6.0} Hz)  {3:10} gs fails ({4:6.2} %) {5:6.3} average",
					this->m_stat_calculations,
					static_cast<double>(this->m_stat_calculations) / this->exec().time().as_double(),
					this->m_iterative_fail,
					100.0 * static_cast<double>(this->m_iterative_fail)
						/ static_cast<double>(this->m_stat_calculations),
					static_cast<double>(this->m_iterative_total) / static_cast<double>(this->m_stat_calculations));
		}
	}

} // namespace devices
} // namespace netlist

