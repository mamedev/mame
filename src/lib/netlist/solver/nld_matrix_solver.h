// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_matrix_solver.h
 *
 */

#ifndef NLD_MATRIX_SOLVER_H_
#define NLD_MATRIX_SOLVER_H_

#include "netlist/nl_base.h"
#include "netlist/nl_errstr.h"
#include "plib/mat_cr.h"
#include "plib/palloc.h"
#include "plib/pmatrix2d.h"
#include "plib/putil.h"
#include "plib/vector_ops.h"

#include <cmath>

namespace netlist
{
namespace solver
{
	P_ENUM(matrix_sort_type_e,
		NOSORT,
		ASCENDING,
		DESCENDING,
		PREFER_IDENTITY_TOP_LEFT,
		PREFER_BAND_MATRIX
	)

	P_ENUM(matrix_type_e,
		SOR_MAT,
		MAT_CR,
		MAT,
		SM,
		W,
		SOR,
		GMRES
	)

	struct solver_parameters_t
	{
		solver_parameters_t(device_t &parent)
		: m_freq(parent, "FREQ", 48000.0)

		/* iteration parameters */
		, m_gs_sor(parent, "SOR_FACTOR", 1.059)
		, m_method(parent, "METHOD", matrix_type_e::MAT_CR)
		, m_accuracy(parent, "ACCURACY", 1e-7)
		, m_gs_loops(parent, "GS_LOOPS", 9)              // Gauss-Seidel loops

		/* general parameters */
		, m_gmin(parent, "GMIN", 1e-9)
		, m_pivot(parent, "PIVOT", false)                    // use pivoting - on supported solvers
		, m_nr_loops(parent, "NR_LOOPS", 250)            // Newton-Raphson loops
		, m_nr_recalc_delay(parent, "NR_RECALC_DELAY", netlist_time::quantum().as_double()) // Delay to next solve attempt if nr loops exceeded
		, m_parallel(parent, "PARALLEL", 0)

		/* automatic time step */
		, m_dynamic_ts(parent, "DYNAMIC_TS", false)
		, m_dynamic_lte(parent, "DYNAMIC_LTE", 1e-5)                     // diff/timestep
		, m_dynamic_min_ts(parent, "DYNAMIC_MIN_TIMESTEP", 1e-6)   // nl_fptype timestep resolution

		/* matrix sorting */
		, m_sort_type(parent, "SORT_TYPE", matrix_sort_type_e::PREFER_IDENTITY_TOP_LEFT)

		/* special */
		, m_use_gabs(parent, "USE_GABS", true)
		, m_use_linear_prediction(parent, "USE_LINEAR_PREDICTION", false) // // savings are eaten up by effort

		{
			m_min_timestep = m_dynamic_min_ts();
			m_max_timestep = netlist_time::from_double(1.0 / m_freq()).as_double();

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
		param_fp_t m_accuracy;
		param_num_t<std::size_t> m_gs_loops;
		param_fp_t m_gmin;
		param_logic_t  m_pivot;
		param_num_t<std::size_t> m_nr_loops;
		param_fp_t m_nr_recalc_delay;
		param_int_t m_parallel;
		param_logic_t  m_dynamic_ts;
		param_fp_t m_dynamic_lte;
		param_fp_t m_dynamic_min_ts;
		param_enum_t<matrix_sort_type_e> m_sort_type;

		param_logic_t m_use_gabs;
		param_logic_t m_use_linear_prediction;

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

		bool isNet(const analog_net_t * net) const noexcept { return net == m_net; }

		void set_railstart(std::size_t val) noexcept { m_railstart = val; }

		PALIGNAS_VECTOROPT()

		plib::aligned_vector<unsigned> m_nz;   /* all non zero for multiplication */
		plib::aligned_vector<unsigned> m_nzrd; /* non zero right of the diagonal for elimination, may include RHS element */
		plib::aligned_vector<unsigned> m_nzbd; /* non zero below of the diagonal for elimination */

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

		/* after every call to solve, update inputs must be called.
		 * this can be done as well as a batch to ease parallel processing.
		 */
		const netlist_time solve(netlist_time now);
		void update_inputs();

		bool has_dynamic_devices() const noexcept { return m_dynamic_devices.size() > 0; }
		bool has_timestep_devices() const noexcept { return m_step_devices.size() > 0; }

		void update_forced();
		void update_after(const netlist_time after)
		{
			m_Q_sync.net().toggle_and_push_to_queue(after);
		}

		/* netdevice functions */
		NETLIB_UPDATEI();
		NETLIB_RESETI();

	public:
		int get_net_idx(const analog_net_t *net) const noexcept;
		std::pair<int, int> get_left_right_of_diag(std::size_t row, std::size_t diag);
		nl_fptype get_weight_around_diag(std::size_t row, std::size_t diag);

		virtual void log_stats();

		virtual std::pair<pstring, pstring> create_solver_code()
		{
			return std::pair<pstring, pstring>("", plib::pfmt("/* solver doesn't support static compile */\n\n"));
		}

		/* return number of floating point operations for solve */
		std::size_t ops() { return m_ops; }

	protected:

		matrix_solver_t(netlist_state_t &anetlist, const pstring &name,
			const analog_net_t::list_t &nets,
			const solver_parameters_t *params);

		void sort_terms(matrix_sort_type_e sort);

		void update_dynamic();

		virtual unsigned vsolve_non_dynamic(const bool newton_raphson) = 0;

		netlist_time compute_next_timestep(const nl_fptype cur_ts);
		/* virtual */ void  add_term(std::size_t net_idx, terminal_t *term);

		template <typename T>
		void store(const T & V);

		template <typename T>
		auto delta(const T & V) -> typename std::decay<decltype(V[0])>::type;

		void set_pointers()
		{
			const std::size_t iN = this->m_terms.size();

			std::size_t max_count = 0;
			std::size_t max_rail = 0;
			for (std::size_t k = 0; k < iN; k++)
			{
				max_count = std::max(max_count, m_terms[k].count());
				max_rail = std::max(max_rail, m_terms[k].railstart());
			}

			m_mat_ptr.resize(iN, max_rail+1);
			m_gtn.resize(iN, max_count);
			m_gonn.resize(iN, max_count);
			m_Idrn.resize(iN, max_count);
			m_connected_net_Vn.resize(iN, max_count);

			for (std::size_t k = 0; k < iN; k++)
			{
				auto count = m_terms[k].count();

				for (std::size_t i = 0; i < count; i++)
				{
					m_terms[k].terms()[i]->set_ptrs(&m_gtn[k][i], &m_gonn[k][i], &m_Idrn[k][i]);
					m_connected_net_Vn[k][i] = m_terms[k].terms()[i]->connected_terminal()->net().Q_Analog_state_ptr();
				}
			}
		}

		template <typename FT>
		void fill_matrix(std::size_t N, FT &RHS)
		{
			for (std::size_t k = 0; k < N; k++)
			{
				auto &net = m_terms[k];
				auto **tcr_r = &(m_mat_ptr[k][0]);

				const std::size_t term_count = net.count();
				const std::size_t railstart = net.railstart();
				const auto &go = m_gonn[k];
				const auto &gt = m_gtn[k];
				const auto &Idr = m_Idrn[k];
				const auto &cnV = m_connected_net_Vn[k];

				for (std::size_t i = 0; i < railstart; i++)
					*tcr_r[i]       += go[i];

				typename FT::value_type gtot_t = 0.0;
				typename FT::value_type RHS_t = 0.0;

				for (std::size_t i = 0; i < term_count; i++)
				{
					gtot_t        += gt[i];
					RHS_t         += Idr[i];
				}
				// FIXME: Code above is faster than vec_sum - Check this
		#if 0
				auto gtot_t = plib::vec_sum<FT>(term_count, m_gt);
				auto RHS_t = plib::vec_sum<FT>(term_count, m_Idr);
		#endif

				for (std::size_t i = railstart; i < term_count; i++)
				{
					RHS_t += (/*m_Idr[i]*/ (- go[i]) * *cnV[i]);
				}

				RHS[k] = RHS_t;
				// update diagonal element ...
				*tcr_r[railstart] += gtot_t; //mat.A[mat.diag[k]] += gtot_t;
			}

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

		template <typename M>
		void clear_square_mat(std::size_t n, M &m)
		{
			for (std::size_t k=0; k < n; k++)
			{
				auto *p = &(m[k][0]);
				for (std::size_t i=0; i < n; i++)
					p[i] = 0.0;
			}
		}

		template <typename M>
		void build_mat_ptr(std::size_t iN, M &mat)
		{
			for (std::size_t k=0; k<iN; k++)
			{
				std::size_t cnt(0);
				/* build pointers into the compressed row format matrix for each terminal */
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

		template <typename T>
		using aligned_alloc = plib::aligned_allocator<T, PALIGN_VECTOROPT>;

		plib::pmatrix2d<nl_fptype, aligned_alloc<nl_fptype>>        m_gonn;
		plib::pmatrix2d<nl_fptype, aligned_alloc<nl_fptype>>        m_gtn;
		plib::pmatrix2d<nl_fptype, aligned_alloc<nl_fptype>>        m_Idrn;
		plib::pmatrix2d<nl_fptype *, aligned_alloc<nl_fptype *>>    m_mat_ptr;
		plib::pmatrix2d<nl_fptype *, aligned_alloc<nl_fptype *>>    m_connected_net_Vn;

		plib::aligned_vector<terms_for_net_t> m_terms;
		plib::aligned_vector<terms_for_net_t> m_rails_temp;

		/* state - variable time_stepping */
		plib::aligned_vector<nl_fptype> m_last_V;
		plib::aligned_vector<nl_fptype> m_DD_n_m_1;
		plib::aligned_vector<nl_fptype> m_h_n_m_1;

		// FIXME: it should be like this, however dimensions are determined
		//        in vsetup.
		//state_container<std::vector<nl_fptype>> m_last_V;
		//state_container<std::vector<nl_fptype>> m_DD_n_m_1;
		//state_container<std::vector<nl_fptype>> m_h_n_m_1;

		std::vector<unique_pool_ptr<proxied_analog_output_t>> m_inps;

		const solver_parameters_t &m_params;

		state_var<int> m_stat_calculations;
		state_var<int> m_stat_newton_raphson;
		state_var<int> m_stat_vsolver_calls;
		state_var<int> m_iterative_fail;
		state_var<int> m_iterative_total;

	private:

		state_var<netlist_time> m_last_step;
		std::vector<core_device_t *> m_step_devices;
		std::vector<core_device_t *> m_dynamic_devices;

		logic_input_t m_fb_sync;
		logic_output_t m_Q_sync;

		/* base setup - called from constructor */
		void setup_base(const analog_net_t::list_t &nets);

		/* calculate matrix */
		void setup_matrix();

		void step(const netlist_time &delta);

		std::size_t m_ops;
	};

	template <typename T>
	auto matrix_solver_t::delta(const T & V) -> typename std::decay<decltype(V[0])>::type
	{
		/* NOTE: Ideally we should also include currents (RHS) here. This would
		 * need a reevaluation of the right hand side after voltages have been updated
		 * and thus belong into a different calculation. This applies to all solvers.
		 */

		const std::size_t iN = this->m_terms.size();
		typename std::decay<decltype(V[0])>::type cerr = 0;
		for (std::size_t i = 0; i < iN; i++)
			cerr = std::max(cerr, std::abs(V[i] - this->m_terms[i].getV()));
		return cerr;
	}

	template <typename T>
	void matrix_solver_t::store(const T & V)
	{
		const std::size_t iN = this->m_terms.size();
		for (std::size_t i = 0; i < iN; i++)
			this->m_terms[i].setV(V[i]);
	}

} // namespace solver
} // namespace netlist

#endif /* NLD_MS_DIRECT_H_ */
