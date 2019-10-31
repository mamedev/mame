// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nl_generic_models.h
 *
 */

#ifndef NLD_GENERIC_MODELS_H_
#define NLD_GENERIC_MODELS_H_

#include "netlist/nl_base.h"
#include "netlist/nl_setup.h"

#include <cmath>

namespace netlist
{
namespace analog
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
		generic_capacitor(device_t &dev, const pstring &name)
		: m_h(dev, name + ".m_h", 0.0)
		, m_c(dev, name + ".m_c", 0.0)
		, m_v(dev, name + ".m_v", 0.0)
		, m_gmin(0.0)
		{
		}

		capacitor_e type() const { return capacitor_e::VARIABLE_CAPACITY; }

		// Circuit Simulation, page 284, 5.360
		// q(un+1) - q(un) = int(un, un+1, C(U)) = (C0+C1)/2 * (un+1-un)
		// The direct application of formulas 5.359 and 5.360 has
		// issues with pulses. Therefore G and Ieq are expressed differently
		// so that G depends on un+1 only and Ieq on un only.
		// In both cases, i = G * un+1 + Ieq

		nl_fptype G(nl_fptype cap) const
		{
			//return m_h * cap +  m_gmin;
			return m_h * 0.5 * (cap + m_c) +  m_gmin;
			//return m_h * cap +  m_gmin;
		}

		nl_fptype Ieq(nl_fptype cap, nl_fptype v) const
		{
			plib::unused_var(v);
			//return -m_h * 0.5 * ((cap + m_c) * m_v + (cap - m_c) * v) ;
			return -m_h * 0.5 * (cap + m_c) * m_v;
			//return -m_h * cap * m_v;
		}

		void timestep(nl_fptype cap, nl_fptype v, nl_fptype step)
		{
			m_h = 1.0 / step;
			m_c = cap;
			m_v = v;
		}

		void setparams(nl_fptype gmin) { m_gmin = gmin; }

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
		: m_h(dev, name + ".m_h", 0.0)
		, m_v(dev, name + ".m_v", 0.0)
		, m_gmin(0.0)
		{
		}

		capacitor_e type() const { return capacitor_e::CONSTANT_CAPACITY; }
		nl_fptype G(nl_fptype cap) const { return cap * m_h +  m_gmin; }
		nl_fptype Ieq(nl_fptype cap, nl_fptype v) const
		{
			plib::unused_var(v);
			return - G(cap) * m_v;
		}

		void timestep(nl_fptype cap, nl_fptype v, nl_fptype step)
		{
			plib::unused_var(cap);
			m_h = 1.0 / step;
			m_v = v;
		}
		void setparams(nl_fptype gmin) { m_gmin = gmin; }
	private:
		state_var<nl_fptype> m_h;
		state_var<nl_fptype> m_v;
		nl_fptype m_gmin;
	};

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
		generic_diode(device_t &dev, const pstring &name)
		: m_Vd(dev, name + ".m_Vd", 0.7)
		, m_Id(dev, name + ".m_Id", 0.0)
		, m_G(dev,  name + ".m_G", 1e-15)
		, m_Vt(0.0)
		, m_Vmin(0.0) // not used in MOS model
		, m_Is(0.0)
		, m_logIs(0.0)
		, m_n(0.0)
		, m_gmin(1e-15)
		, m_VtInv(0.0)
		, m_Vcrit(0.0)
		, m_name(name)
		{
			set_param(1e-15, 1, 1e-15, 300.0);
		}

		void update_diode(const nl_fptype nVd)
		{
			nl_fptype IseVDVt(0.0);

			if (TYPE == diode_e::BIPOLAR)
			{
				//printf("%s: %g %g\n", m_name.c_str(), nVd, (nl_fptype) m_Vd);
				if (nVd > m_Vcrit)
				{
					const nl_fptype d = std::min(+fp_constants<nl_fptype>::DIODE_MAXDIFF, nVd - m_Vd);
					const nl_fptype a = std::abs(d) * m_VtInv;
					m_Vd = m_Vd + (d < 0 ? -1.0 : 1.0) * std::log1p(a) * m_Vt;
				}
				else
					m_Vd = std::max(-fp_constants<nl_fptype>::DIODE_MAXDIFF, nVd);
					//m_Vd = nVd;

				if (m_Vd < m_Vmin)
				{
					m_G = m_gmin;
					m_Id = - m_Is;
				}
				else
				{
					IseVDVt = std::exp(m_logIs + m_Vd * m_VtInv);
					m_Id = IseVDVt - m_Is;
					m_G = IseVDVt * m_VtInv + m_gmin;
				}
			}
			else if (TYPE == diode_e::MOS)
			{
				if (nVd < constants::zero())
				{
					m_Vd = nVd;
					m_G = m_Is * m_VtInv + m_gmin;
					m_Id = m_G * m_Vd;
				}
				else /* log stepping should already be done in mosfet */
				{
					m_Vd = nVd;
					IseVDVt = std::exp(std::min(+fp_constants<nl_fptype>::DIODE_MAXVOLT, m_logIs + m_Vd * m_VtInv));
					m_Id = IseVDVt - m_Is;
					m_G = IseVDVt * m_VtInv + m_gmin;
				}
			}
		}

		void set_param(const nl_fptype Is, const nl_fptype n, nl_fptype gmin, nl_fptype temp)
		{
			m_Is = Is;
			m_logIs = std::log(Is);
			m_n = n;
			m_gmin = gmin;

			m_Vt = m_n * temp * constants::k_b() / constants::Q_e();

			m_Vmin = -5.0 * m_Vt;

			m_Vcrit = m_Vt * std::log(m_Vt / m_Is / constants::sqrt2());
			m_VtInv = constants::one() / m_Vt;
			//printf("%g %g\n", m_Vmin, m_Vcrit);
		}


		nl_fptype I() const { return m_Id; }
		nl_fptype G() const { return m_G; }
		nl_fptype Ieq() const { return (m_Id - m_Vd * m_G); }
		nl_fptype Vd() const { return m_Vd; }

		/* owning object must save those ... */

	private:
		state_var<nl_fptype> m_Vd;
		state_var<nl_fptype> m_Id;
		state_var<nl_fptype> m_G;

		nl_fptype m_Vt;
		nl_fptype m_Vmin;
		nl_fptype m_Is;
		nl_fptype m_logIs;
		nl_fptype m_n;
		nl_fptype m_gmin;

		nl_fptype m_VtInv;
		nl_fptype m_Vcrit;

		pstring m_name;
	};


} // namespace analog
} // namespace netlist

#endif /* NLD_GENERIC_MODELS_H_ */
