// license:BSD-3-Clause
// copyright-holders:Couriersud

// Names
// spell-checker: words Raphson, Seidel
//
// Specific technical terms
// spell-checker: words vsolver

#include "nld_solver.h"
#include "core/setup.h"
#include "nl_setup.h"
#include "nld_matrix_solver.h"
#include "plib/putil.h"

namespace netlist::solver
{

	terms_for_net_t::terms_for_net_t(arena_type &arena, analog_net_t * net)
		: m_nz(arena)
		, m_nzrd(arena)
		, m_nzbd(arena)
		, m_connected_net_idx(arena)
		, m_terms(arena)
		, m_net(net)
		, m_rail_start(0)
	{
	}

	void terms_for_net_t::add_terminal(terminal_t *term, int net_other, bool sorted)
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

	matrix_solver_t::matrix_solver_t(devices::nld_solver &main_solver, const pstring &name,
		const net_list_t &nets,
		const solver::solver_parameters_t *params)
		: device_t(static_cast<device_t &>(main_solver), name)
		, m_params(*params)
		, m_gonn(m_arena)
		, m_gtn(m_arena)
		, m_Idrn(m_arena)
		, m_connected_net_Vn(m_arena)
		, m_iterative_fail(*this, "m_iterative_fail", 0)
		, m_iterative_total(*this, "m_iterative_total", 0)
		, m_main_solver(main_solver)
		, m_stat_calculations(*this, "m_stat_calculations", 0)
		, m_stat_newton_raphson(*this, "m_stat_newton_raphson", 0)
		, m_stat_newton_raphson_fail(*this, "m_stat_newton_raphson_fail", 0)
		, m_stat_vsolver_calls(*this, "m_stat_vsolver_calls", 0)
		, m_last_step(*this, "m_last_step", netlist_time_ext::zero())
		, m_step_funcs(m_arena)
		, m_dynamic_funcs(m_arena)
		, m_inps(m_arena)
		, m_ops(0)
	{
		setup_base(this->state().setup(), nets);

		// now setup the matrix
		setup_matrix();
		//printf("Freq: %f\n", m_params.m_freq());
	}

	analog_net_t *matrix_solver_t::get_connected_net(terminal_t *term)
	{
		return &state().setup().get_connected_terminal(*term)->net();
	}

	void matrix_solver_t::reschedule(netlist_time ts)
	{
		m_main_solver.reschedule(this, ts);
	}

	void matrix_solver_t::setup_base(setup_t &setup, const net_list_t &nets)
	{
		log().debug("New solver setup\n");
		std::vector<core_device_t *> step_devices;
		std::vector<core_device_t *> dynamic_devices;

		m_terms.clear();

		for (const auto & net : nets)
		{
			m_terms.emplace_back(m_arena, net);
			m_rails_temp.emplace_back(m_arena);
		}

		for (std::size_t k = 0; k < nets.size(); k++)
		{
			analog_net_t &net = *nets[k];

			log().debug("adding net with {1} populated connections\n", setup.nlstate().core_terms(net).size());

			net.set_solver(this);

			for (auto &p : setup.nlstate().core_terms(net))
			{
				log().debug("{1} {2} {3}\n", p->name(), net.name(), net.is_rail_net());
				switch (p->type())
				{
					case detail::terminal_type::TERMINAL:
						if (p->device().is_timestep())
							if (!plib::container::contains(step_devices, &p->device()))
								step_devices.push_back(&p->device());
						if (p->device().is_dynamic())
							if (!plib::container::contains(dynamic_devices, &p->device()))
								dynamic_devices.push_back(&p->device());
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
								pstring nname(this->name() + "." + pstring(plib::pfmt("m{1}")(m_inps.size())));
								nl_assert(p->net().is_analog());
								auto net_proxy_output_u = state().make_pool_object<proxied_analog_output_t>(*this, nname, &dynamic_cast<analog_net_t &>(p->net()));
								net_proxy_output = net_proxy_output_u.get();
								m_inps.emplace_back(std::move(net_proxy_output_u));
							}
							setup.add_terminal(net_proxy_output->net(), *p);
							// FIXME: repeated calling - kind of brute force
							net_proxy_output->net().rebuild_list();
							log().debug("Added input {1}", net_proxy_output->name());
						}
						break;
					case detail::terminal_type::OUTPUT:
						log().fatal(MF_UNHANDLED_ELEMENT_1_FOUND(p->name()));
						throw nl_exception(MF_UNHANDLED_ELEMENT_1_FOUND(p->name()));
				}
			}
		}
		for (auto &d : step_devices)
			m_step_funcs.emplace_back(nldelegate_ts(&core_device_t::timestep, d));
		for (auto &d : dynamic_devices)
			m_dynamic_funcs.emplace_back(nldelegate_dyn(&core_device_t::update_terminals, d));
	}

	/// \brief Sort terminals
	///
	/// @param sort Sort algorithm to use.
	///
	/// Sort in descending order by number of connected matrix voltages.
	///The idea is, that for Gauss-Seidel algorithm the first voltage computed
	/// depends on the greatest number of previous voltages thus taking into
	/// account the maximum amount of information.
	///
	/// This actually improves performance on popeye slightly. Average
	/// GS computations reduce from 2.509 to 2.370
	///
	/// Smallest to largest : 2.613
	/// Unsorted            : 2.509
	/// Largest to smallest : 2.370
	//
	/// Sorting as a general matrix pre-conditioning is mentioned in
	/// literature but I have found no articles about Gauss Seidel.
	///
	/// For Gaussian Elimination however increasing order is better suited.
	/// NOTE: Even better would be to sort on elements right of the matrix diagonal.
	/// FIXME: This entry needs an update.
	///
	void matrix_solver_t::sort_terms(matrix_sort_type_e sort)
	{
		const std::size_t iN = m_terms.size();

		switch (sort)
		{
			case matrix_sort_type_e::PREFER_BAND_MATRIX:
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
								pk = get_weight_around_diag(k,k);
							}
						}
					}
				}
				break;
			case matrix_sort_type_e::PREFER_IDENTITY_TOP_LEFT:
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
								pk = get_left_right_of_diag(k,k);
							}
						}
					}
				}
				break;
			case matrix_sort_type_e::ASCENDING:
			case matrix_sort_type_e::DESCENDING:
				{
					int sort_order = (sort == matrix_sort_type_e::DESCENDING ? 1 : -1);

					for (std::size_t k = 0; k < iN - 1; k++)
						for (std::size_t i = k+1; i < iN; i++)
						{
							if ((static_cast<int>(m_terms[k].rail_start()) - static_cast<int>(m_terms[i].rail_start())) * sort_order < 0)
							{
								std::swap(m_terms[i], m_terms[k]);
							}
						}
				}
				break;
			case matrix_sort_type_e::NOSORT:
				break;
		}
		// rebuild
		for (auto &term : m_terms)
		{
			//int *other = term.m_connected_net_idx.data();
			for (std::size_t i = 0; i < term.count(); i++)
				//FIXME: this is weird
				if (term.m_connected_net_idx[i] != -1)
					term.m_connected_net_idx[i] = get_net_idx(get_connected_net(term.terms()[i]));
		}
	}

	void matrix_solver_t::setup_matrix()
	{
		const std::size_t iN = m_terms.size();

		for (std::size_t k = 0; k < iN; k++)
		{
			m_terms[k].set_rail_start(m_terms[k].count());
			for (std::size_t i = 0; i < m_rails_temp[k].count(); i++)
				this->m_terms[k].add_terminal(m_rails_temp[k].terms()[i], m_rails_temp[k].m_connected_net_idx.data()[i], false);
		}

		// free all - no longer needed
		m_rails_temp.clear();

		sort_terms(m_params.m_sort_type);

		this->set_pointers();

		// create a list of non zero elements.
		for (unsigned k = 0; k < iN; k++)
		{
			terms_for_net_t & t = m_terms[k];
			// pretty brutal
			int *other = t.m_connected_net_idx.data();

			t.m_nz.clear();

			for (std::size_t i = 0; i < t.rail_start(); i++)
				if (!plib::container::contains(t.m_nz, static_cast<unsigned>(other[i])))
					t.m_nz.push_back(static_cast<unsigned>(other[i]));

			t.m_nz.push_back(k);     // add diagonal

			// and sort
			std::sort(t.m_nz.begin(), t.m_nz.end());
		}

		// create a list of non zero elements right of the diagonal
		// These list anticipate the population of array elements by
		// Gaussian elimination.

		for (std::size_t k = 0; k < iN; k++)
		{
			terms_for_net_t & t = m_terms[k];
			// pretty brutal
			int *other = t.m_connected_net_idx.data();

			if (k==0)
				t.m_nzrd.clear();
			else
			{
				t.m_nzrd = m_terms[k-1].m_nzrd;
				for (auto j = t.m_nzrd.begin(); j != t.m_nzrd.end(); )
				{
					if (*j < k + 1)
						j = t.m_nzrd.erase(j);
					else
						++j;
				}
			}

			for (std::size_t i = 0; i < t.rail_start(); i++)
				if (!plib::container::contains(t.m_nzrd, static_cast<unsigned>(other[i])) && other[i] >= static_cast<int>(k + 1))
					t.m_nzrd.push_back(static_cast<unsigned>(other[i]));

			// and sort
			std::sort(t.m_nzrd.begin(), t.m_nzrd.end());
		}

		// create a list of non zero elements below diagonal k
		// This should reduce cache misses ...

		std::vector<std::vector<bool>> touched(iN, std::vector<bool>(iN));

		for (std::size_t k = 0; k < iN; k++)
		{
			for (std::size_t j = 0; j < iN; j++)
				touched[k][j] = false;
			for (std::size_t j = 0; j < m_terms[k].m_nz.size(); j++)
				touched[k][m_terms[k].m_nz[j]] = true;
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
					if (!plib::container::contains(m_terms[k].m_nzbd, row))
						m_terms[k].m_nzbd.push_back(row);
					for (std::size_t col = k + 1; col < iN; col++)
						if (touched[k][col])
						{
							touched[row][col] = true;
							m_ops += 2;
						}
				}
			}
		}
		log().verbose("Number of multiplications/additions for {1}: {2}", name(), m_ops);

		if ((false))
			for (std::size_t k = 0; k < iN; k++)
			{
				pstring line = plib::pfmt("{1:3}")(k);
				for (const auto & nzrd : m_terms[k].m_nzrd)
					line += plib::pfmt(" {1:3}")(nzrd);
				log().verbose("{1}", line);
			}

		//
		// save states
		//

		for (std::size_t k = 0; k < iN; k++)
		{
			pstring num = plib::pfmt("{1}")(k);

			state().save(*this, m_gonn[k],"GO" + num, this->name(), m_terms[k].count());
			state().save(*this, m_gtn[k],"GT" + num, this->name(), m_terms[k].count());
			state().save(*this, m_Idrn[k],"IDR" + num, this->name(), m_terms[k].count());
		}
	}

	void matrix_solver_t::set_pointers()
	{
		const std::size_t iN = this->m_terms.size();

		std::size_t max_count = 0;
		std::size_t max_rail = 0;
		for (std::size_t k = 0; k < iN; k++)
		{
			max_count = std::max(max_count, m_terms[k].count());
			max_rail = std::max(max_rail, m_terms[k].rail_start());
		}

		m_gtn.resize(iN, max_count);
		m_gonn.resize(iN, max_count);
		m_Idrn.resize(iN, max_count);
		m_connected_net_Vn.resize(iN, max_count);

		// Initialize arrays to 0 (in case the vrl one is used
		for (std::size_t k = 0; k < iN; k++)
			for (std::size_t j = 0; j < m_terms[k].count(); j++)
			{
				m_gtn.set(k,j, nlconst::zero());
				m_gonn.set(k,j, nlconst::zero());
				m_Idrn.set(k,j, nlconst::zero());
				m_connected_net_Vn.set(k, j, nullptr);
			}


		for (std::size_t k = 0; k < iN; k++)
		{
			auto count = m_terms[k].count();
			for (std::size_t i = 0; i < count; i++)
			{
				m_terms[k].terms()[i]->set_ptrs(&m_gtn[k][i], &m_gonn[k][i], &m_Idrn[k][i]);
				m_connected_net_Vn[k][i] = get_connected_net(m_terms[k].terms()[i])->Q_Analog_state_ptr();
			}
		}
	}

	void matrix_solver_t::update_inputs()
	{
		// avoid recursive calls. Inputs are updated outside this call
		for (auto &inp : m_inps)
			inp->push(inp->proxied_net()->Q_Analog());
	}

	bool matrix_solver_t::updates_net(const analog_net_t *net) const noexcept
	{
		if (net != nullptr)
		{
			for (const auto &t : m_terms )
				if (t.is_net(net))
					return true;
			for (const auto &inp : m_inps)
				if (&inp->net() == net)
					return true;
		}
		return false;
	}

	void matrix_solver_t::update_dynamic() noexcept
	{
		// update all non-linear devices
		for (auto &dyn : m_dynamic_funcs)
			dyn();
	}

	void matrix_solver_t::reset()
	{
		//m_last_step = netlist_time_ext::zero();
	}

	void matrix_solver_t::step(timestep_type ts_type, netlist_time delta) noexcept
	{
		const auto dd(delta.as_fp<fptype>());
		for (auto &d : m_step_funcs)
			d(ts_type, dd);
	}

	bool matrix_solver_t::solve_nr_base()
	{
		bool this_resched(false);
		std::size_t newton_loops = 0;
		do
		{
			update_dynamic();
			// Gauss-Seidel will revert to Gaussian elimination if steps exceeded.
			this->m_stat_calculations++;
			this->vsolve_non_dynamic();
			this_resched = this->check_err();
			this->store();
			newton_loops++;
		} while (this_resched && newton_loops < m_params.m_nr_loops);

		m_stat_newton_raphson += newton_loops;
		if (this_resched)
			m_stat_newton_raphson_fail++;
		return this_resched;
	}

	netlist_time matrix_solver_t::newton_loops_exceeded(netlist_time delta)
	{
		netlist_time next_time_step;
		bool resched(false);

		restore();
		step(timestep_type::RESTORE, delta);

		for (std::size_t i=0; i< 10; i++)
		{
			backup();
			step(timestep_type::FORWARD, netlist_time::from_fp(m_params.m_min_ts_ts()));
			resched = solve_nr_base();
			// update time step calculation
			next_time_step = compute_next_timestep(m_params.m_min_ts_ts(), m_params.m_min_ts_ts(), m_params.m_max_timestep);
			delta -= netlist_time::from_fp(m_params.m_min_ts_ts());
		}
		// try remaining time using compute_next_time step
		while (delta > netlist_time::zero())
		{
			if (next_time_step > delta)
				next_time_step = delta;
			backup();
			step(timestep_type::FORWARD, next_time_step);
			delta -= next_time_step;
			resched = solve_nr_base();
			next_time_step = compute_next_timestep(next_time_step.as_fp<nl_fptype>(), m_params.m_min_ts_ts(), m_params.m_max_timestep);
		}

		if (m_stat_newton_raphson % 100 == 0)
			log().warning(MW_NEWTON_LOOPS_EXCEEDED_INVOCATION_3(100, this->name(), exec().time().as_double() * 1e6));

		if (resched)
		{
			// reschedule ....
			log().warning(MW_NEWTON_LOOPS_EXCEEDED_ON_NET_2(this->name(), exec().time().as_double() * 1e6));
			return netlist_time::from_fp(m_params.m_nr_recalc_delay());
		}
		if (m_params.m_dynamic_ts)
			return next_time_step;

		return netlist_time::from_fp(m_params.m_max_timestep);
	}

	netlist_time matrix_solver_t::solve(netlist_time_ext now, [[maybe_unused]] const char *source)
	{
		auto delta = static_cast<netlist_time>(now - m_last_step());
		PFDEBUG(printf("solve %.10f\n", delta.as_double());)

		// We are already up to date. Avoid oscillations.
		// FIXME: Make this a parameter!
		if (delta < netlist_time::quantum())
		{
			//printf("solve return %s at %f\n", source, now.as_double());
			return timestep_device_count() > 0 ? netlist_time::from_fp(m_params.m_min_timestep) : netlist_time::zero();
		}

		backup(); // save voltages for backup and time step calculation
		// update all terminals for new time step
		m_last_step = now;

		++m_stat_vsolver_calls;
		if (dynamic_device_count() != 0)
		{
			step(timestep_type::FORWARD, delta);
			const auto resched = solve_nr_base();

			if (resched)
				return newton_loops_exceeded(delta);
		}
		else
		{
			step(timestep_type::FORWARD, delta);
			this->m_stat_calculations++;
			this->vsolve_non_dynamic();
			this->store();
		}

		if (m_params.m_dynamic_ts)
		{
			if (timestep_device_count() > 0)
				return compute_next_timestep(delta.as_fp<nl_fptype>(), m_params.m_min_timestep, m_params.m_max_timestep);
		}

		if (timestep_device_count() > 0)
			return netlist_time::from_fp(m_params.m_max_timestep);

		return netlist_time::zero();

	}

	int matrix_solver_t::get_net_idx(const analog_net_t *net) const noexcept
	{
		for (std::size_t k = 0; k < m_terms.size(); k++)
			if (m_terms[k].is_net(net))
				return static_cast<int>(k);
		return -1;
	}

	std::pair<int, int> matrix_solver_t::get_left_right_of_diag(std::size_t irow, std::size_t idiag)
	{
		//
		// return the maximum column left of the diagonal (-1 if no cols found)
		// return the minimum column right of the diagonal (999999 if no cols found)
		//

		const auto row = static_cast<int>(irow);
		const auto diag = static_cast<int>(idiag);

		int colmax = -1;
		int colmin = 999999;

		auto &term = m_terms[irow];

		for (std::size_t i = 0; i < term.count(); i++)
		{
			auto col = get_net_idx(get_connected_net(term.terms()[i]));
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

	matrix_solver_t::fptype matrix_solver_t::get_weight_around_diag(std::size_t row, std::size_t diag)
	{
		{
			//
			// return average absolute distance
			//

			std::vector<bool> touched(1024, false); // FIXME!

			fptype weight = nlconst::zero();
			auto &term = m_terms[row];
			for (std::size_t i = 0; i < term.count(); i++)
			{
				auto col = get_net_idx(get_connected_net(term.terms()[i]));
				if (col >= 0)
				{
					auto colu = static_cast<std::size_t>(col);
					if (!touched[colu])
					{
						if (colu==row) colu = static_cast<unsigned>(diag);
						else if (colu==diag) colu = static_cast<unsigned>(row);

						weight = weight + plib::abs(static_cast<fptype>(colu) - static_cast<fptype>(diag));
						touched[colu] = true;
					}
				}
			}
			return weight;
		}
	}

	void matrix_solver_t::add_term(std::size_t net_idx, terminal_t *term)
	{
		if (get_connected_net(term)->is_rail_net())
		{
			m_rails_temp[net_idx].add_terminal(term, -1, false);
		}
		else
		{
			int ot = get_net_idx(get_connected_net(term));
			if (ot>=0)
			{
				m_terms[net_idx].add_terminal(term, ot, true);
			}
			else
			{
				log().fatal(MF_FOUND_TERM_WITH_MISSING_OTHERNET(term->name()));
				throw nl_exception(MF_FOUND_TERM_WITH_MISSING_OTHERNET(term->name()));
			}
		}
	}

	void matrix_solver_t::log_stats()
	{
		if (this->m_stat_calculations != 0 && this->m_stat_vsolver_calls && log().verbose.is_enabled())
		{
			log().verbose("==============================================");
			log().verbose("Solver {1}", this->name());
			log().verbose("       ==> {1} nets", this->m_terms.size());
			log().verbose("       has {1} dynamic elements", this->dynamic_device_count());
			log().verbose("       has {1} time step elements", this->timestep_device_count());
			log().verbose("       {1:6.3} average newton raphson loops",
						static_cast<fptype>(this->m_stat_newton_raphson) / static_cast<fptype>(this->m_stat_vsolver_calls));
			log().verbose("       {1:10} invocations ({2:6.0} Hz)  {3:10} gs fails ({4:6.2} %) {5:6.3} average",
					this->m_stat_calculations,
					static_cast<fptype>(this->m_stat_calculations) / this->exec().time().as_fp<fptype>(),
					this->m_iterative_fail,
					nlconst::hundred() * static_cast<fptype>(this->m_iterative_fail)
						/ static_cast<fptype>(this->m_stat_calculations),
					static_cast<fptype>(this->m_iterative_total) / static_cast<fptype>(this->m_stat_calculations));
		}
	}

} // namespace netlist::solver

