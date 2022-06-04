// license:BSD-3-Clause
// copyright-holders:Couriersud

#ifndef NLD_GENERIC_MODELS_H_
#define NLD_GENERIC_MODELS_H_

///
/// \file nld_generic_models.h
///

#include "core/state_var.h"
#include "nl_base.h"

//
// Set to 0 to use a linearized diode model in the range exceeding
// maximum dissipation. The intention is to have a faster
// convergence. On selected circuits (LM3900 trapezoidal) this is
// observable and has a 10% impact.
// FIXME: More research needed
//

#define USE_TEXTBOOK_DIODE  (1)

namespace netlist::analog
{

	// -----------------------------------------------------------------------------
	// A generic capacitor model
	// -----------------------------------------------------------------------------

	enum class capacitor_e
	{
		VARIABLE_CAPACITY,
		CONSTANT_CAPACITY
	};

	template <capacitor_e TYPE>
	class generic_capacitor
	{
	};

	template <>
	class generic_capacitor<capacitor_e::VARIABLE_CAPACITY>
	{
	public:
		generic_capacitor(core_device_t &dev, const pstring &name)
		: m_h(dev, name + ".m_h", nlconst::zero())
		, m_c(dev, name + ".m_c", nlconst::zero())
		, m_v(dev, name + ".m_v", nlconst::zero())
		, m_gmin(nlconst::zero())
		{
		}

		static capacitor_e type() noexcept { return capacitor_e::VARIABLE_CAPACITY; }

		// Circuit Simulation, page 284, 5.360
		// q(un+1) - q(un) = int(un, un+1, C(U)) = (C0+C1)/2 * (un+1-un)
		// The direct application of formulas 5.359 and 5.360 has
		// issues with pulses. Therefore G and Ieq are expressed differently
		// so that G depends on un+1 only and Ieq on un only.
		// In both cases, i = G * un+1 + Ieq

		nl_fptype G(nl_fptype cap) const noexcept
		{
			//return m_h * cap +  m_gmin;
			return m_h * nlconst::half() * (cap + m_c) +  m_gmin;
			//return m_h * cap +  m_gmin;
		}

		nl_fptype Ieq(nl_fptype cap, [[maybe_unused]] nl_fptype v) const noexcept
		{
			//return -m_h * 0.5 * ((cap + m_c) * m_v + (cap - m_c) * v) ;
			return -m_h * nlconst::half() * (cap + m_c) * m_v;
			//return -m_h * cap * m_v;
		}

		void time_step(nl_fptype cap, nl_fptype v, nl_fptype step) noexcept
		{
			m_h = plib::reciprocal(step);
			m_c = cap;
			m_v = v;
		}

		void restore_state() noexcept
		{
			// no state used
		}

		void set_params(nl_fptype gmin) noexcept { m_gmin = gmin; }

	private:
		state_var<nl_fptype> m_h;
		state_var<nl_fptype> m_c;
		state_var<nl_fptype> m_v;
		nl_fptype m_gmin;
	};

	// "Circuit simulation", page 274
	template <>
	class generic_capacitor<capacitor_e::CONSTANT_CAPACITY>
	{
	public:
		generic_capacitor(device_t &dev, const pstring &name)
		: m_h(dev, name + ".m_h", nlconst::zero())
		, m_v(dev, name + ".m_v", nlconst::zero())
		, m_gmin(nlconst::zero())
		{
		}

		static capacitor_e type() noexcept { return capacitor_e::CONSTANT_CAPACITY; }
		nl_fptype G(nl_fptype cap) const noexcept { return cap * m_h +  m_gmin; }
		nl_fptype Ieq(nl_fptype cap, [[maybe_unused]] nl_fptype v) const noexcept
		{
			return - G(cap) * m_v;
		}

		void time_step([[maybe_unused]] nl_fptype cap, nl_fptype v, nl_fptype step) noexcept
		{
			m_h = plib::reciprocal(step);
			m_v = v;
		}
		void setparams(nl_fptype gmin) noexcept { m_gmin = gmin; }
	private:
		state_var<nl_fptype> m_h;
		state_var<nl_fptype> m_v;
		nl_fptype m_gmin;
	};

#if (NL_USE_BACKWARD_EULER)
	// Constant model for constant capacitor model
	// Backward Euler
	// "Circuit simulation", page 274
	struct generic_capacitor_const
	{
	public:
		generic_capacitor_const( /*[[maybe_unused]]*/ core_device_t &dev, /*[[maybe_unused]]*/ const pstring &name)
		: m_gmin(nlconst::zero())
		{
			// gcc 7.2 (mingw) and 7.5 (ubuntu) don't accept maybe_unused here
			plib::unused_var(dev, name);
		}

		// Returns { G, Ieq }
		std::pair<nl_fptype, nl_fptype> time_step(nl_fptype cap, nl_fptype v, nl_fptype step) const noexcept
		{
			const nl_fptype h(plib::reciprocal(step));
			const nl_fptype G(cap * h + m_gmin);
			return { G, - G * v };
		}
		void restore_state() noexcept
		{
			// this one has no state
		}
		void set_parameters(nl_fptype gmin) noexcept { m_gmin = gmin; }
	private:
		nl_fptype m_gmin;
	};
#else
	// Constant model for constant capacitor model
	// Trapezoidal
	// "Circuit simulation", page 278
	struct generic_capacitor_const
	{
	public:
		generic_capacitor_const([[maybe_unused]] core_device_t &dev, [[maybe_unused]] const pstring &name)
		: m_gmin(nlconst::zero())
		, m_vn(0)
		, m_in(0)
		, m_trn(0.0)
		{
		}

		// Returns { G, Ieq }
		std::pair<nl_fptype, nl_fptype> time_step(nl_fptype cap, nl_fptype v, nl_fptype step) noexcept
		{
			const nl_fptype h(plib::reciprocal(step));
			if (m_trn == 0.0)
			{
				const nl_fptype G(cap * h + m_gmin);
				m_vn = v;
				m_trn = h;
				return { G, - G * v };
			}
			const nl_fptype Gn = nlconst::two() * cap * m_trn;
			const nl_fptype inp1 = Gn * v - (m_in + Gn * m_vn);
			const nl_fptype G(nlconst::two() * cap * h);
			const nl_fptype Ieq(inp1 + G * v);
			m_in = inp1;
			m_vn = v;
			m_trn = h;
			return { G + m_gmin, -Ieq };
		}
		void restore_state() noexcept
		{
			// this one has no state
		}
		void set_parameters(nl_fptype gmin) noexcept { m_gmin = gmin; }
	private:
		nl_fptype m_gmin;
		nl_fptype m_vn;
		nl_fptype m_in;
		nl_fptype m_trn;
	};
#endif
	// -----------------------------------------------------------------------------
	// A generic diode model to be used in other devices (Diode, BJT ...)
	// -----------------------------------------------------------------------------

	enum class diode_e
	{
		BIPOLAR,
		MOS
	};

	template <diode_e TYPE>
	class generic_diode
	{
	public:
		generic_diode(core_device_t &dev, const pstring &name)
		: m_Vd(dev, name + ".m_Vd", nlconst::diode_start_voltage())
		, m_Id(dev, name + ".m_Id", nlconst::zero())
		, m_G(dev,  name + ".m_G", nlconst::cgminalt())
		, m_Vt(nlconst::zero())
		, m_Vmin(nlconst::zero()) // not used in MOS model
		, m_Is(nlconst::zero())
		, m_logIs(nlconst::zero())
		, m_gmin(nlconst::cgminalt())
		, m_VtInv(nlconst::zero())
		, m_Vcrit(nlconst::zero())
		{
			set_param(
				nlconst::np_Is()
			  , nlconst::one()
			  , nlconst::cgminalt()
			  , nlconst::T0());
		}
		// Basic math
		//
		// I(V) = f(V)
		//
		// G(V) = df/dV(V)
		//
		// Ieq(V) = I(V) - V * G(V)
		//
		//
		void update_diode(nl_fptype nVd) noexcept
		{
			if (TYPE == diode_e::BIPOLAR)
			{
#if USE_TEXTBOOK_DIODE
				if (nVd > m_Vcrit)
				{
					// if the old voltage is less than zero and new is above
					// make sure we move enough so that matrix and current
					// changes.
					const nl_fptype old = std::max(nlconst::zero(), m_Vd());
					const nl_fptype d = std::min(+fp_constants<nl_fptype>::DIODE_MAXDIFF(), nVd - old);
					const nl_fptype a = plib::abs(d) * m_VtInv;
					m_Vd = old + plib::signum(d) * plib::log1p(a) * m_Vt;
				}
				else
					m_Vd = std::max(-fp_constants<nl_fptype>::DIODE_MAXDIFF(), nVd);

				if (m_Vd < m_Vmin)
				{
					m_G = m_gmin;
					m_Id = - m_Is;
				}
				else
				{
					const auto IseVDVt = plib::exp(m_logIs + m_Vd * m_VtInv);
					m_Id = IseVDVt - m_Is;
					m_G = IseVDVt * m_VtInv + m_gmin;
				}
#else
				//printf("%s: %g %g\n", m_name.c_str(), nVd, (nl_fptype) m_Vd);
				m_Vd = nVd;
				if (nVd > m_Vcrit)
				{
					m_Id = m_Icrit_p_Is - m_Is + (m_Vd - m_Vcrit) * m_Icrit_p_Is * m_VtInv;
					m_G = m_Icrit_p_Is * m_VtInv + m_gmin;
				}
				else if (m_Vd < m_Vmin)
				{
					m_G = m_gmin;
					//m_Id = m_Imin + (m_Vd - m_Vmin) * m_gmin;
					//m_Imin = m_gmin * m_Vt - m_Is;
					m_Id = (m_Vd - m_Vmin + m_Vt) * m_gmin - m_Is;
				}
				else
				{
					const auto IseVDVt = plib::exp(m_logIs + m_Vd * m_VtInv);
					m_Id = IseVDVt - m_Is;
					m_G = IseVDVt * m_VtInv + m_gmin;
				}
#endif
			}
			else if (TYPE == diode_e::MOS)
			{
				m_Vd = nVd;
				if (nVd < nlconst::zero())
				{
					m_G = m_Is * m_VtInv + m_gmin;
					m_Id = m_G * m_Vd;
				}
				else // log stepping should already be done in mosfet
				{
					const auto IseVDVt = plib::exp(std::min(+fp_constants<nl_fptype>::DIODE_MAXVOLT(), m_logIs + m_Vd * m_VtInv));
					m_Id = IseVDVt - m_Is;
					m_G = IseVDVt * m_VtInv + m_gmin;
				}
			}
		}

		void set_param(nl_fptype Is, nl_fptype n, nl_fptype gmin, nl_fptype temp) noexcept
		{
			m_Is = Is;
			m_logIs = plib::log(Is);
			m_gmin = gmin;

			m_Vt = nlconst::np_VT(n, temp);
			m_VtInv = plib::reciprocal(m_Vt);

#if USE_TEXTBOOK_DIODE
			m_Vmin = nlconst::diode_min_cutoff_mult() * m_Vt;
			// Vcrit : f(V) has smallest radius of curvature rho(V) == min(rho(v))
			m_Vcrit = m_Vt * plib::log(m_Vt / m_Is / nlconst::sqrt2());
#else
			m_Vmin = plib::log(m_gmin * m_Vt / m_Is) * m_Vt;
			//m_Imin = plib::exp(m_logIs + m_Vmin * m_VtInv) - m_Is;
			//m_Imin = m_gmin * m_Vt - m_Is;
			// Fixme: calculate max dissipation voltage - use use 0.5 (500mW) here for typical diode
			// P = V * I = V * (Is*exp(V/Vt) - Is)
			// P ~= V * I = V * Is*exp(V/Vt)
			// ln(P/Is) = ln(V)+V/Vt ~= V - 1 + V/vt
			// V = (1+ln(P/Is))/(1 + 1/Vt)

			m_Vcrit = (nlconst::one() + plib::log(nlconst::half() / m_Is)) / (nlconst::one() + m_VtInv);
			//printf("Vcrit: %f\n", m_Vcrit);
			m_Icrit_p_Is = plib::exp(m_logIs + m_Vcrit * m_VtInv);
			//m_Icrit = plib::exp(m_logIs + m_Vcrit * m_VtInv) - m_Is;
#endif

		}

		nl_fptype I() const noexcept { return m_Id; }
		nl_fptype G() const noexcept  { return m_G; }
		nl_fptype Ieq() const noexcept  { return (m_Id - m_Vd * m_G); }
		nl_fptype Vd() const noexcept  { return m_Vd; }

		// owning object must save those ...

	private:
		state_var<nl_fptype> m_Vd;
		state_var<nl_fptype> m_Id;
		state_var<nl_fptype> m_G;

		nl_fptype m_Vt;
		nl_fptype m_Vmin;
		nl_fptype m_Is;
		nl_fptype m_logIs;
		nl_fptype m_gmin;

		nl_fptype m_VtInv;
		nl_fptype m_Vcrit;
#if !USE_TEXTBOOK_DIODE
		//nl_fptype m_Imin;
		nl_fptype m_Icrit_p_Is;
#endif
	};


} // namespace netlist::analog

#endif // NLD_GENERIC_MODELS_H_
