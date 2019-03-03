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
#include "netlist/plib/palloc.h"
#include "netlist/plib/putil.h"
#include "netlist/plib/vector_ops.h"

namespace netlist
{
namespace devices
{
	/* FIXME: these should become proper devices */

	struct solver_parameters_t
	{
		bool m_pivot;
		nl_double m_accuracy;
		nl_double m_dynamic_lte;
		nl_double m_min_timestep;
		nl_double m_max_timestep;
		nl_double m_gs_sor;
		bool m_dynamic_ts;
		std::size_t m_gs_loops;
		std::size_t m_nr_loops;
		netlist_time m_nr_recalc_delay;
		bool m_use_gabs;
		bool m_use_linear_prediction;
		bool m_log_stats;
	};


	class terms_for_net_t : plib::nocopyassignmove
	{
	public:
		terms_for_net_t();

		void clear();

		void add(terminal_t *term, int net_other, bool sorted);

		std::size_t count() const { return m_terms.size(); }

		terminal_t **terms() { return m_terms.data(); }
		int *connected_net_idx() { return m_connected_net_idx.data(); }
		nl_double *gt() { return m_gt.data(); }
		nl_double *go() { return m_go.data(); }
		nl_double *Idr() { return m_Idr.data(); }
		nl_double * const *connected_net_V() const { return m_connected_net_V.data(); }

		void set_pointers();

		/* FIXME: this works a bit better for larger matrices */
		template <typename AP, typename FT>
		void fill_matrix/*_larger*/(AP &tcr, FT &RHS)
		{

			const std::size_t term_count = this->count();
			const std::size_t railstart = this->m_railstart;

			for (std::size_t i = 0; i < railstart; i++)
				*tcr[i]       -= m_go[i];

	#if 1
			FT gtot_t = 0.0;
			FT RHS_t = 0.0;

			for (std::size_t i = 0; i < term_count; i++)
			{
				gtot_t        += m_gt[i];
				RHS_t         += m_Idr[i];
			}
			// FIXME: Code above is faster than vec_sum - Check this
	#else
			auto gtot_t = plib::vec_sum<FT>(term_count, m_gt);
			auto RHS_t = plib::vec_sum<FT>(term_count, m_Idr);
	#endif

			for (std::size_t i = railstart; i < term_count; i++)
			{
				RHS_t += (/*m_Idr[i]*/ + m_go[i] * *m_connected_net_V[i]);
			}

			RHS = RHS_t;
			// update diagonal element ...
			*tcr[railstart] += gtot_t; //mat.A[mat.diag[k]] += gtot_t;
		}

		std::size_t m_railstart;

		std::vector<unsigned> m_nz;   /* all non zero for multiplication */
		std::vector<unsigned> m_nzrd; /* non zero right of the diagonal for elimination, may include RHS element */
		std::vector<unsigned> m_nzbd; /* non zero below of the diagonal for elimination */

		/* state */
		nl_double m_last_V;
		nl_double m_DD_n_m_1;
		nl_double m_h_n_m_1;

	private:
		std::vector<int> m_connected_net_idx;
		plib::aligned_vector<nl_double, PALIGN_VECTOROPT> m_go;
		plib::aligned_vector<nl_double, PALIGN_VECTOROPT> m_gt;
		plib::aligned_vector<nl_double, PALIGN_VECTOROPT> m_Idr;
		plib::aligned_vector<nl_double *, PALIGN_VECTOROPT> m_connected_net_V;
		std::vector<terminal_t *> m_terms;

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

		enum eSortType
		{
			NOSORT,
			ASCENDING,
			DESCENDING,
			PREFER_IDENTITY_TOP_LEFT,
			PREFER_BAND_MATRIX
		};

		void setup(analog_net_t::list_t &nets)
		{
			vsetup(nets);
		}

		void solve_base();

		/* after every call to solve, update inputs must be called.
		 * this can be done as well as a batch to ease parallel processing.
		 */
		const netlist_time solve(netlist_time now);
		void update_inputs();

		bool has_dynamic_devices() const { return m_dynamic_devices.size() > 0; }
		bool has_timestep_devices() const { return m_step_devices.size() > 0; }

		void update_forced();
		void update_after(const netlist_time after)
		{
			m_Q_sync.net().toggle_and_push_to_queue(after);
		}

		/* netdevice functions */
		NETLIB_UPDATEI();
		NETLIB_RESETI();

	public:
		int get_net_idx(detail::net_t *net);
		std::pair<int, int> get_left_right_of_diag(std::size_t row, std::size_t diag);
		double get_weight_around_diag(std::size_t row, std::size_t diag);

		virtual void log_stats();

		virtual std::pair<pstring, pstring> create_solver_code()
		{
			return std::pair<pstring, pstring>("", plib::pfmt("/* solver doesn't support static compile */\n\n"));
		}

		/* return number of floating point operations for solve */
		std::size_t ops() { return m_ops; }

	protected:

		matrix_solver_t(netlist_state_t &anetlist, const pstring &name,
				eSortType sort, const solver_parameters_t *params);

		void sort_terms(eSortType sort);

		void setup_base(analog_net_t::list_t &nets);
		void update_dynamic();

		virtual void vsetup(analog_net_t::list_t &nets) = 0;
		virtual unsigned vsolve_non_dynamic(const bool newton_raphson) = 0;

		netlist_time compute_next_timestep(const double cur_ts);
		/* virtual */ void  add_term(std::size_t net_idx, terminal_t *term);

		template <typename T>
		void store(const T & V);

		template <typename T>
		auto delta(const T & V) -> typename std::decay<decltype(V[0])>::type;

		template <typename T>
		void build_LE_A(T &child);
		template <typename T>
		void build_LE_RHS(T &child);

		std::vector<plib::unique_ptr<terms_for_net_t>> m_terms;
		std::vector<analog_net_t *> m_nets;
		std::vector<poolptr<proxied_analog_output_t>> m_inps;

		std::vector<plib::unique_ptr<terms_for_net_t>> m_rails_temp;

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

		/* calculate matrix */
		void setup_matrix();

		void step(const netlist_time &delta);

		std::size_t m_ops;
		const eSortType m_sort;
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
			cerr = std::max(cerr, std::abs(V[i] - this->m_nets[i]->Q_Analog()));
		return cerr;
	}

	template <typename T>
	void matrix_solver_t::store(const T & V)
	{
		const std::size_t iN = this->m_terms.size();
		for (std::size_t i = 0; i < iN; i++)
			this->m_nets[i]->set_Q_Analog(V[i]);
	}

	template <typename T>
	void matrix_solver_t::build_LE_A(T &child)
	{
		using float_type = typename T::float_type;
		static_assert(std::is_base_of<matrix_solver_t, T>::value, "T must derive from matrix_solver_t");

		const std::size_t iN = child.size();
		for (std::size_t k = 0; k < iN; k++)
		{
			terms_for_net_t *terms = m_terms[k].get();
			float_type * Ak = &child.A(k, 0ul);

			for (std::size_t i=0; i < iN; i++)
				Ak[i] = 0.0;

			const std::size_t terms_count = terms->count();
			const std::size_t railstart =  terms->m_railstart;
			const float_type * const gt = terms->gt();

			{
				float_type akk  = 0.0;
				for (std::size_t i = 0; i < terms_count; i++)
					akk += gt[i];

				Ak[k] = akk;
			}

			const float_type * const go = terms->go();
			int * net_other = terms->connected_net_idx();

			for (std::size_t i = 0; i < railstart; i++)
				Ak[net_other[i]] -= go[i];
		}
	}

	template <typename T>
	void matrix_solver_t::build_LE_RHS(T &child)
	{
		static_assert(std::is_base_of<matrix_solver_t, T>::value, "T must derive from matrix_solver_t");
		using float_type = typename T::float_type;

		const std::size_t iN = child.size();
		for (std::size_t k = 0; k < iN; k++)
		{
			float_type rhsk_a = 0.0;
			float_type rhsk_b = 0.0;

			const std::size_t terms_count = m_terms[k]->count();
			const float_type * const go = m_terms[k]->go();
			const float_type * const Idr = m_terms[k]->Idr();
			const float_type * const * other_cur_analog = m_terms[k]->connected_net_V();

			for (std::size_t i = 0; i < terms_count; i++)
				rhsk_a = rhsk_a + Idr[i];

			for (std::size_t i = m_terms[k]->m_railstart; i < terms_count; i++)
				//rhsk = rhsk + go[i] * terms[i]->m_otherterm->net().as_analog().Q_Analog();
				rhsk_b = rhsk_b + go[i] * *other_cur_analog[i];

			child.RHS(k) = rhsk_a + rhsk_b;
		}
	}

} //namespace devices
} // namespace netlist

#endif /* NLD_MS_DIRECT_H_ */
