// license:BSD-3-Clause
// copyright-holders:Couriersud

#ifndef NLD_SOLVER_H_
#define NLD_SOLVER_H_

///
/// \file nld_solver.h
///

#include "../nl_base.h"
#include "../plib/pstream.h"
#include "nld_matrix_solver.h"

#include <map>
#include <memory>
#include <vector>

// ----------------------------------------------------------------------------------------
// solver
// ----------------------------------------------------------------------------------------

namespace netlist
{
namespace devices
{
	NETLIB_OBJECT(solver)
	{
	public:
		using queue_type = detail::queue_base<solver::matrix_solver_t, false>;
		using solver_arena = device_arena;

		NETLIB_CONSTRUCTOR(solver)
		, m_fb_step(*this, "FB_step", NETLIB_DELEGATE(fb_step<false>))
		, m_Q_step(*this, "Q_step")
		, m_params(*this, "", solver::solver_parameter_defaults::get_instance())
		, m_queue(config::max_solver_queue_size(),
			queue_type::id_delegate(&NETLIB_NAME(solver) :: get_solver_id, this),
			queue_type::obj_delegate(&NETLIB_NAME(solver) :: solver_by_id, this))
		{
			// internal stuff
			state().save(*this, static_cast<plib::state_manager_t::callback_t &>(m_queue), this->name(), "m_queue");

			connect("FB_step", "Q_step");
		}

		void post_start();
		void stop();

		auto gmin() const -> decltype(solver::solver_parameters_t::m_gmin()) { return m_params.m_gmin(); }

		solver::static_compile_container create_solver_code(solver::static_compile_target target);

		NETLIB_RESETI();
		// NETLIB_UPDATE_PARAMI();

		using solver_ptr = solver_arena::unique_ptr<solver::matrix_solver_t>;

		using net_list_t = solver::matrix_solver_t::net_list_t;

		void reschedule(solver::matrix_solver_t *solv, netlist_time ts);

	private:
		using params_uptr = solver_arena::unique_ptr<solver::solver_parameters_t>;

		template<bool KEEP_STATS>
		NETLIB_HANDLERI(fb_step);

		logic_input_t m_fb_step;
		logic_output_t m_Q_step;

		// FIXME: these should be created in device space
		std::vector<params_uptr> m_mat_params;
		std::vector<solver_ptr> m_mat_solvers;

		solver::solver_parameters_t m_params;
		queue_type m_queue;

		template <typename FT, int SIZE>
		solver_ptr create_solver(std::size_t size, const pstring &solvername,
			const solver::solver_parameters_t *params,net_list_t &nets);

		template <typename FT>
		solver_ptr create_solvers(const pstring &sname,
			const solver::solver_parameters_t *params, net_list_t &nets);

		std::size_t get_solver_id(const solver::matrix_solver_t *net) const;
		solver::matrix_solver_t *solver_by_id(std::size_t id) const;

	};

} // namespace devices
} // namespace netlist

#endif // NLD_SOLVER_H_
