// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef NLD_MATRIX_SOLVER_H_
#define NLD_MATRIX_SOLVER_H_

///
/// \file nld_matrix_solver.h
///

#include "netlist/nl_base.h"
#include "netlist/nl_errstr.h"
#include "netlist/plib/mat_cr.h"
#include "netlist/plib/palloc.h"
#include "netlist/plib/penum.h"
#include "netlist/plib/pmatrix2d.h"
#include "netlist/plib/putil.h"
#include "netlist/plib/vector_ops.h"

#include <numeric>

namespace netlist
{
namespace solver
{

	enum static_compile_target
	{
		CXX_EXTERNAL_C,
		CXX_STATIC
	};

	PENUM(matrix_sort_type_e,
		NOSORT,
		ASCENDING,
		DESCENDING,
		PREFER_IDENTITY_TOP_LEFT,
		PREFER_BAND_MATRIX
	)

	PENUM(matrix_type_e,
		SOR_MAT,
		MAT_CR,
		MAT,
		SM,
		W,
		SOR,
		GMRES
	)

	PENUM(matrix_fp_type_e,
		  FLOAT
		, DOUBLE
		, LONGDOUBLE
		, FLOATQ128
	)

	using static_compile_container = std::vector<std::pair<pstring, pstring>>;

	struct solver_parameters_t
	{
		solver_parameters_t(device_t &parent)
		: m_freq(parent, "FREQ", nlconst::magic(48000.0))

		// iteration parameters
		, m_gs_sor(parent,   "SOR_FACTOR", nlconst::magic(1.059))
		, m_method(parent,   "METHOD", matrix_type_e::MAT_CR)
		, m_fp_type(parent,  "FPTYPE", matrix_fp_type_e::DOUBLE)
		, m_reltol(parent,   "RELTOL", nlconst::magic(1e-3))            ///< SPICE RELTOL parameter
		, m_vntol(parent,    "VNTOL",  nlconst::magic(1e-7))            ///< SPICE VNTOL parameter
		, m_accuracy(parent, "ACCURACY", nlconst::magic(1e-7))          ///< Iterative solver accuracy
		, m_nr_loops(parent, "NR_LOOPS", 250)           ///< Maximum number of Newton-Raphson loops
		, m_gs_loops(parent, "GS_LOOPS", 9)             ///< Maximum number of Gauss-Seidel loops

		// general parameters
		, m_gmin(parent, "GMIN", nlconst::magic(1e-9))
		, m_pivot(parent, "PIVOT", false)               ///< use pivoting on supported solvers
		, m_nr_recalc_delay(parent, "NR_RECALC_DELAY",
			netlist_time::quantum().as_fp<nl_fptype>()) ///< Delay to next solve attempt if nr loops exceeded
		, m_parallel(parent, "PARALLEL", 0)

		// automatic time step
		, m_dynamic_ts(parent, "DYNAMIC_TS", false)     ///< Use dynamic time stepping
		, m_dynamic_lte(parent, "DYNAMIC_LTE", nlconst::magic(1e-5))    ///< dynamic time stepping slope
		, m_dynamic_min_ts(parent, "DYNAMIC_MIN_TIMESTEP", nlconst::magic(1e-6)) ///< smallest time step allowed

		// matrix sorting
		, m_sort_type(parent, "SORT_TYPE", matrix_sort_type_e::PREFER_IDENTITY_TOP_LEFT)

		// special
		, m_use_gabs(parent, "USE_GABS", true)

		{
			m_min_timestep = m_dynamic_min_ts();
			m_max_timestep = netlist_time::from_fp(plib::reciprocal(m_freq())).as_fp<decltype(m_max_timestep)>();

			if (m_dynamic_ts)
			{
				m_max_timestep *= 1;//NL_FCONST(1000.0);
			}
			else
			{
				m_min_timestep = m_max_timestep;
			}
		}

		param_fp_t m_freq;
		param_fp_t m_gs_sor;
		param_enum_t<matrix_type_e> m_method;
		param_enum_t<matrix_fp_type_e> m_fp_type;
		param_fp_t m_reltol;
		param_fp_t m_vntol;
		param_fp_t m_accuracy;
		param_num_t<std::size_t> m_nr_loops;
		param_num_t<std::size_t> m_gs_loops;
		param_fp_t m_gmin;
		param_logic_t  m_pivot;
		param_fp_t m_nr_recalc_delay;
		param_int_t m_parallel;
		param_logic_t  m_dynamic_ts;
		param_fp_t m_dynamic_lte;
		param_fp_t m_dynamic_min_ts;
		param_enum_t<matrix_sort_type_e> m_sort_type;

		param_logic_t m_use_gabs;

		nl_fptype m_min_timestep;
		nl_fptype m_max_timestep;
	};


	class terms_for_net_t
	{
	public:
		terms_for_net_t(analog_net_t * net = nullptr);

		void clear();

		void add_terminal(terminal_t *term, int net_other, bool sorted);

		std::size_t count() const noexcept { return m_terms.size(); }

		std::size_t railstart() const noexcept { return m_railstart; }

		terminal_t **terms() noexcept { return m_terms.data(); }

		nl_fptype getV() const noexcept { return m_net->Q_Analog(); }

		void setV(nl_fptype v) noexcept { m_net->set_Q_Analog(v); }

		bool is_net(const analog_net_t * net) const noexcept { return net == m_net; }

		void set_railstart(std::size_t val) noexcept { m_railstart = val; }

		PALIGNAS_VECTOROPT()

		plib::aligned_vector<unsigned> m_nz;   //!< all non zero for multiplication
		plib::aligned_vector<unsigned> m_nzrd; //!< non zero right of the diagonal for elimination, may include RHS element
		plib::aligned_vector<unsigned> m_nzbd; //!< non zero below of the diagonal for elimination

		plib::aligned_vector<int> m_connected_net_idx;
	private:
		analog_net_t * m_net;
		plib::aligned_vector<terminal_t *> m_terms;
		std::size_t m_railstart;
	};

	class proxied_analog_output_t : public analog_output_t
	{
	public:

		proxied_analog_output_t(core_device_t &dev, const pstring &aname, analog_net_t *pnet)
		: analog_output_t(dev, aname)
		, m_proxied_net(pnet)
		{ }

		analog_net_t *proxied_net() const { return m_proxied_net;}
	private:
		analog_net_t *m_proxied_net; // only for proxy nets in analog input logic
	};

	class matrix_solver_t : public device_t
	{
	public:
		using list_t = std::vector<matrix_solver_t *>;

		// after every call to solve, update inputs must be called.
		// this can be done as well as a batch to ease parallel processing.

		netlist_time solve(netlist_time_ext now);
		void update_inputs();

		/// \brief Checks if solver may alter a net
		///
		/// This checks if a solver will alter a net. Returns true if the
		/// net is either part of the voltage vector or if it belongs to
		/// the analog input nets connected to the solver.

		bool updates_net(const analog_net_t *net) const noexcept;

		std::size_t dynamic_device_count() const noexcept { return m_dynamic_funcs.size(); }
		std::size_t timestep_device_count() const noexcept { return m_step_funcs.size(); }

		/// \brief Immediately solve system at current time
		///
		/// This should only be called from update and update_param events.
		/// It's purpose is to bring voltage values to the current timestep.
		/// This will be called BEFORE updating object properties.
		void solve_now()
		{
			// this should only occur outside of execution and thus
			// using time should be safe.

			const netlist_time new_timestep = solve(exec().time());
			plib::unused_var(new_timestep);

			update_inputs();

			if (m_params.m_dynamic_ts && (timestep_device_count() != 0))
			{
				m_Q_sync.net().toggle_and_push_to_queue(netlist_time::from_fp(m_params.m_min_timestep));
			}
		}

		template <typename F>
		void change_state(F f, netlist_time delay = netlist_time::quantum())
		{
			// We only need to update the net first if this is a time stepping net
			if (timestep_device_count() > 0)
			{
				const netlist_time new_timestep = solve(exec().time());
				plib::unused_var(new_timestep);
				update_inputs();
			}
			f();
			m_Q_sync.net().toggle_and_push_to_queue(delay);
		}

		// netdevice functions
		NETLIB_UPDATEI()
		{
			const netlist_time new_timestep = solve(exec().time());
			update_inputs();

			if (m_params.m_dynamic_ts && (timestep_device_count() != 0) && new_timestep > netlist_time::zero())
			{
				m_Q_sync.net().toggle_and_push_to_queue(new_timestep);
			}
		}

		NETLIB_RESETI();

		virtual void log_stats();

		virtual std::pair<pstring, pstring> create_solver_code(solver::static_compile_target target)
		{
			plib::unused_var(target);
			return std::pair<pstring, pstring>("", plib::pfmt("/* solver doesn't support static compile */\n\n"));
		}

		// return number of floating point operations for solve
		std::size_t ops() const { return m_ops; }

	protected:
		template <typename T>
		using aligned_alloc = plib::aligned_allocator<T, PALIGN_VECTOROPT>;

		matrix_solver_t(netlist_state_t &anetlist, const pstring &name,
			const analog_net_t::list_t &nets,
			const solver_parameters_t *params);

		virtual void vsolve_non_dynamic() = 0;
		virtual netlist_time compute_next_timestep(nl_fptype cur_ts, nl_fptype max_ts) = 0;
		virtual bool check_err() = 0;
		virtual void store() = 0;

		plib::pmatrix2d<nl_fptype, aligned_alloc<nl_fptype>>        m_gonn;
		plib::pmatrix2d<nl_fptype, aligned_alloc<nl_fptype>>        m_gtn;
		plib::pmatrix2d<nl_fptype, aligned_alloc<nl_fptype>>        m_Idrn;
		plib::pmatrix2d<nl_fptype *, aligned_alloc<nl_fptype *>>    m_connected_net_Vn;

		plib::aligned_vector<terms_for_net_t> m_terms;

		const solver_parameters_t &m_params;

		state_var<std::size_t> m_iterative_fail;
		state_var<std::size_t> m_iterative_total;

	private:

		plib::aligned_vector<terms_for_net_t> m_rails_temp;
		std::vector<unique_pool_ptr<proxied_analog_output_t>> m_inps;

		state_var<std::size_t> m_stat_calculations;
		state_var<std::size_t> m_stat_newton_raphson;
		state_var<std::size_t> m_stat_vsolver_calls;

		state_var<netlist_time_ext> m_last_step;
		plib::aligned_vector<nldelegate_ts> m_step_funcs;
		plib::aligned_vector<nldelegate_dyn> m_dynamic_funcs;

		logic_input_t m_fb_sync;
		logic_output_t m_Q_sync;

		std::size_t m_ops;

		// base setup - called from constructor
		void setup_base(setup_t &setup, const analog_net_t::list_t &nets) noexcept(false);

		void sort_terms(matrix_sort_type_e sort);

		void update_dynamic() noexcept;
		void step(netlist_time delta) noexcept;

		int get_net_idx(const analog_net_t *net) const noexcept;
		std::pair<int, int> get_left_right_of_diag(std::size_t irow, std::size_t idiag);
		nl_fptype get_weight_around_diag(std::size_t row, std::size_t diag);

		void add_term(std::size_t net_idx, terminal_t *term) noexcept(false);

		// calculate matrix
		void setup_matrix();

		void set_pointers();

		analog_net_t *get_connected_net(terminal_t *term);

	};

	template <typename FT, int SIZE>
	class matrix_solver_ext_t: public matrix_solver_t
	{
	public:

		using float_type = FT;

		matrix_solver_ext_t(netlist_state_t &anetlist, const pstring &name,
			const analog_net_t::list_t &nets,
			const solver_parameters_t *params, const std::size_t size)
		: matrix_solver_t(anetlist, name, nets, params)
		, m_dim(size)
		, m_new_V(size)
		, m_RHS(size)
		, m_mat_ptr(size, this->max_railstart() + 1)
		, m_last_V(size, nlconst::zero())
		, m_DD_n_m_1(size, nlconst::zero())
		, m_h_n_m_1(size, nlconst::magic(1e-6)) // we need a non zero value here
		{
			//
			// save states
			//
			state().save(*this, m_last_V.as_base(), this->name(), "m_last_V");
			state().save(*this, m_DD_n_m_1.as_base(), this->name(), "m_DD_n_m_1");
			state().save(*this, m_h_n_m_1.as_base(), this->name(), "m_h_n_m_1");
		}


	private:
		const std::size_t m_dim;

	protected:
		static constexpr const std::size_t SIZEABS = plib::parray<FT, SIZE>::SIZEABS();
		static constexpr const std::size_t m_pitch_ABS = (((SIZEABS + 0) + 7) / 8) * 8;

		PALIGNAS_VECTOROPT()
		plib::parray<float_type, SIZE> m_new_V;
		PALIGNAS_VECTOROPT()
		plib::parray<float_type, SIZE> m_RHS;

		PALIGNAS_VECTOROPT()
		plib::pmatrix2d<float_type *> m_mat_ptr;

		// FIXME: below should be private
		// state - variable time_stepping
		PALIGNAS_VECTOROPT()
		plib::parray<nl_fptype, SIZE> m_last_V;
		PALIGNAS_VECTOROPT()
		plib::parray<nl_fptype, SIZE> m_DD_n_m_1;
		PALIGNAS_VECTOROPT()
		plib::parray<nl_fptype, SIZE> m_h_n_m_1;

		std::size_t max_railstart() const noexcept
		{
			std::size_t max_rail = 0;
			for (std::size_t k = 0; k < m_terms.size(); k++)
				max_rail = std::max(max_rail, m_terms[k].railstart());
			return max_rail;
		}


		template <typename T, typename M>
		void log_fill(const T &fill, M &mat)
		{
			const std::size_t iN = fill.size();

			// FIXME: Not yet working, mat_cr.h needs some more work
#if 0
			auto mat_GE = dynamic_cast<plib::pGEmatrix_cr_t<typename M::base> *>(&mat);
#else
			plib::unused_var(mat);
#endif
			std::vector<unsigned> levL(iN, 0);
			std::vector<unsigned> levU(iN, 0);

			// parallel scheme for L x = y
			for (std::size_t k = 0; k < iN; k++)
			{
				unsigned lm=0;
				for (std::size_t j = 0; j<k; j++)
					if (fill[k][j] < M::FILL_INFINITY)
						lm = std::max(lm, levL[j]);
				levL[k] = 1+lm;
			}

			// parallel scheme for U x = y
			for (std::size_t k = iN; k-- > 0; )
			{
				unsigned lm=0;
				for (std::size_t j = iN; --j > k; )
					if (fill[k][j] < M::FILL_INFINITY)
						lm = std::max(lm, levU[j]);
				levU[k] = 1+lm;
			}
			for (std::size_t k = 0; k < iN; k++)
			{
				unsigned fm = 0;
				pstring ml = "";
				for (std::size_t j = 0; j < iN; j++)
				{
					ml += fill[k][j] == 0 ? 'X' : fill[k][j] < M::FILL_INFINITY ? '+' : '.';
					if (fill[k][j] < M::FILL_INFINITY)
						if (fill[k][j] > fm)
							fm = fill[k][j];
				}
#if 0
				this->log().verbose("{1:4} {2} {3:4} {4:4} {5:4} {6:4}", k, ml,
					levL[k], levU[k], mat_GE ? mat_GE->get_parallel_level(k) : 0, fm);
#else
				this->log().verbose("{1:4} {2} {3:4} {4:4} {5:4} {6:4}", k, ml,
					levL[k], levU[k], 0, fm);
#endif
			}
		}

		constexpr std::size_t size() const noexcept
		{
			return (SIZE > 0) ? static_cast<std::size_t>(SIZE) : m_dim;
		}

#if 1
		void store() override
		{
			const std::size_t iN = size();
			for (std::size_t i = 0; i < iN; i++)
				this->m_terms[i].setV(static_cast<nl_fptype>(m_new_V[i]));
		}
#else
		// global tanh damping (4.197)
		// partially cures the symptoms but not the cause
		void store() override
		{
			const std::size_t iN = size();
			for (std::size_t i = 0; i < iN; i++)
			{
				auto oldV = this->m_terms[i].template getV<nl_fptype>();
				this->m_terms[i].setV(oldV + 0.02 * plib::tanh((m_new_V[i]-oldV)*50.0));
			}
		}
#endif
		bool check_err() override
		{
			// NOTE: Ideally we should also include currents (RHS) here. This would
			// need a reevaluation of the right hand side after voltages have been updated
			// and thus belong into a different calculation. This applies to all solvers.

			const std::size_t iN = size();
			const auto reltol(static_cast<float_type>(m_params.m_reltol));
			const auto vntol(static_cast<float_type>(m_params.m_vntol));
			for (std::size_t i = 0; i < iN; i++)
			{
				const auto vold(static_cast<float_type>(this->m_terms[i].getV()));
				const auto vnew(m_new_V[i]);
				const auto tol(vntol + reltol * std::max(plib::abs(vnew),plib::abs(vold)));
				if (plib::abs(vnew - vold) > tol)
					return true;
			}
			return false;
		}

		netlist_time compute_next_timestep(nl_fptype cur_ts, nl_fptype max_ts) override
		{
			nl_fptype new_solver_timestep(max_ts);

			for (std::size_t k = 0; k < size(); k++)
			{
				const auto &t = m_terms[k];
				const auto v(static_cast<nl_fptype>(t.getV()));
				// avoid floating point exceptions
				const nl_fptype DD_n = std::max(-fp_constants<nl_fptype>::TIMESTEP_MAXDIFF(),
					std::min(+fp_constants<nl_fptype>::TIMESTEP_MAXDIFF(),(v - m_last_V[k])));

				m_last_V[k] = v;
				const nl_fptype hn = cur_ts;

				nl_fptype DD2 = (DD_n / hn - m_DD_n_m_1[k] / m_h_n_m_1[k]) / (hn + m_h_n_m_1[k]);
				nl_fptype new_net_timestep(0);

				m_h_n_m_1[k] = hn;
				m_DD_n_m_1[k] = DD_n;
				if (plib::abs(DD2) > fp_constants<nl_fptype>::TIMESTEP_MINDIV()) // avoid div-by-zero
					new_net_timestep = plib::sqrt(m_params.m_dynamic_lte / plib::abs(nlconst::half()*DD2));
				else
					new_net_timestep = m_params.m_max_timestep;

				new_solver_timestep = std::min(new_net_timestep, new_solver_timestep);
			}
			new_solver_timestep = std::max(new_solver_timestep, m_params.m_min_timestep);

			// FIXME: Factor 2 below is important. Without, we get timing issues. This must be a bug elsewhere.
			return std::max(netlist_time::from_fp(new_solver_timestep), netlist_time::quantum() * 2);
		}

		template <typename M>
		void build_mat_ptr(M &mat)
		{
			const std::size_t iN = size();

			for (std::size_t k=0; k<iN; k++)
			{
				std::size_t cnt(0);
				// build pointers into the compressed row format matrix for each terminal
				for (std::size_t j=0; j< this->m_terms[k].railstart();j++)
				{
					int other = this->m_terms[k].m_connected_net_idx[j];
					if (other >= 0)
					{
						m_mat_ptr[k][j] = &(mat[k][static_cast<std::size_t>(other)]);
						cnt++;
					}
				}
				nl_assert_always(cnt == this->m_terms[k].railstart(), "Count and railstart mismatch");
				m_mat_ptr[k][this->m_terms[k].railstart()] = &(mat[k][k]);
			}
		}

		template <typename M>
		void clear_square_mat(M &m)
		{
			const std::size_t n = size();
			for (std::size_t k=0; k < n; k++)
			{
				auto *p = &(m[k][0]);
				using mat_elem_type = typename std::decay<decltype(*p)>::type;
				for (std::size_t i=0; i < n; i++)
					p[i] = plib::constants<mat_elem_type>::zero();
			}
		}

		void fill_matrix_and_rhs()
		{
			const std::size_t N = size();

			for (std::size_t k = 0; k < N; k++)
			{
				auto &net = m_terms[k];
				auto **tcr_r = &(m_mat_ptr[k][0]);

				using source_type = typename decltype(m_gtn)::value_type;
				const std::size_t term_count = net.count();
				const std::size_t railstart = net.railstart();
				const auto &go = m_gonn[k];
				const auto &gt = m_gtn[k];
				const auto &Idr = m_Idrn[k];
				const auto &cnV = m_connected_net_Vn[k];

				// FIXME: gonn, gtn and Idr - which float types should they have?

				auto gtot_t = std::accumulate(gt, gt + term_count, plib::constants<source_type>::zero());

				// update diagonal element ...
				*tcr_r[railstart] = static_cast<FT>(gtot_t); //mat.A[mat.diag[k]] += gtot_t;

				for (std::size_t i = 0; i < railstart; i++)
					*tcr_r[i]       += static_cast<FT>(go[i]);

				auto RHS_t(std::accumulate(Idr, Idr + term_count, plib::constants<source_type>::zero()));

				for (std::size_t i = railstart; i < term_count; i++)
					RHS_t +=  (- go[i]) * *cnV[i];

				m_RHS[k] = static_cast<FT>(RHS_t);
			}
		}

	};

} // namespace solver
} // namespace netlist

#endif // NLD_MS_DIRECT_H_
