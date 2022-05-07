// license:BSD-3-Clause
// copyright-holders:Couriersud


#include "analog/nlid_twoterm.h"
#include "solver/nld_solver.h"

#include "plib/pmath.h"

#define R_OFF (1E20)
#define R_ON  (1)

namespace netlist::devices {

	NETLIB_OBJECT(NE555)
	{
		NETLIB_CONSTRUCTOR(NE555)
		, m_R1(*this, "R1")
		, m_R2(*this, "R2")
		, m_R3(*this, "R3")
		, m_ROUT(*this, "ROUT")
		, m_RDIS(*this, "RDIS")
		, m_RESET(*this, "RESET", NETLIB_DELEGATE(inputs))     // Pin 4
		, m_THRES(*this, "THRESH", NETLIB_DELEGATE(inputs))    // Pin 6
		, m_TRIG(*this, "TRIG", NETLIB_DELEGATE(inputs))       // Pin 2
		, m_OUT(*this, "_OUT")        // to Pin 3 via ROUT
		, m_last_out(*this, "m_last_out", false)
		, m_ff(*this, "m_ff", false)
		, m_last_reset(*this, "m_last_reset", false)
		, m_overshoot(*this, "m_overshoot", 0.0)
		, m_undershoot(*this, "m_undershoot", 0.0)
		, m_ovlimit(0.0)
		{
			register_sub_alias("GND",  "R3.2");    // Pin 1
			register_sub_alias("CONT", "R1.2");    // Pin 5
			register_sub_alias("DISCH", "RDIS.1"); // Pin 7
			register_sub_alias("VCC",  "R1.1");    // Pin 8
			register_sub_alias("OUT",  "ROUT.1");  // Pin 3

			connect("R1.2", "R2.1");
			connect("R2.2", "R3.1");
			connect("RDIS.2", "R3.2");
			connect("_OUT", "ROUT.2");
		}

		NETLIB_RESETI()
		{
			/* FIXME make resistances a parameter, properly model other variants */
			m_R1.set_R(nlconst::magic(5000));
			m_R2.set_R(nlconst::magic(5000));
			m_R3.set_R(nlconst::magic(5000));
			m_ROUT.set_R(nlconst::magic(20));
			m_RDIS.set_R(nlconst::magic(R_OFF));

			m_last_out = true;
			// Check for astable setup, usually TRIG AND THRES connected. Enable
			// overshoot compensation in this case.
			if (m_TRIG.net() == m_THRES.net())
				m_ovlimit = nlconst::magic(0.5);
		}

	private:
		NETLIB_HANDLERI(inputs)
		{
			// FIXME: assumes GND is connected to 0V.

			const auto reset = m_RESET();

			const nl_fptype vthresh = clamp_hl(m_R2.P()(), nlconst::magic(0.7), nlconst::magic(1.4));
			const nl_fptype vtrig = clamp_hl(m_R2.N()(), nlconst::magic(0.7), nlconst::magic(1.4));

			// avoid artificial oscillation due to overshoot compensation when
			// the control input is used.
			const auto ovlimit = std::min(m_ovlimit, std::max(0.0, (vthresh - vtrig) / 3.0));

			if (!reset && m_last_reset)
			{
				m_ff = false;
			}
			else
			{
#if (NL_USE_BACKWARD_EULER)
				const bool bthresh = (m_THRES() + m_overshoot > vthresh);
				const bool btrig = (m_TRIG() - m_overshoot > vtrig);
#else
				const bool bthresh = (m_THRES() + m_overshoot > vthresh);
				const bool btrig = (m_TRIG() - m_undershoot > vtrig);
#endif
				if (!btrig)
					m_ff = true;
				else if (bthresh)
				{
					m_ff = false;
				}
			}

			const bool out = (!reset ? false : m_ff);

			if (m_last_out && !out)
			{
#if (NL_USE_BACKWARD_EULER)
				m_overshoot += ((m_THRES() - vthresh)) * 2.0;
#else
				m_overshoot += ((m_THRES() - vthresh));
#endif
				m_overshoot = plib::clamp(m_overshoot(), nlconst::zero(), ovlimit);
				//if (this->name() == "IC6_2")
				//  printf("%f %s %f %f %f\n", exec().time().as_double(), this->name().c_str(), m_overshoot(), m_R2.P()(), m_THRES());
				m_RDIS.change_state([this]()
					{
						m_RDIS.set_R(nlconst::magic(R_ON));
					});
				m_OUT.push(m_R3.N()());
			}
			else if (!m_last_out && out)
			{
#if (NL_USE_BACKWARD_EULER)
				m_overshoot += (vtrig - m_TRIG()) * 2.0;
				m_overshoot = plib::clamp(m_overshoot(), nlconst::zero(), ovlimit);
#else
				m_undershoot += (vtrig - m_TRIG());
				m_undershoot = plib::clamp(m_undershoot(), nlconst::zero(), ovlimit);
#endif
				m_RDIS.change_state([this]()
					{
						m_RDIS.set_R(nlconst::magic(R_OFF));
					});
				// FIXME: Should be delayed by 100ns
				m_OUT.push(m_R1.P()());
			}
			m_last_reset = reset;
			m_last_out = out;
		}
		analog::NETLIB_SUB(R_base) m_R1;
		analog::NETLIB_SUB(R_base) m_R2;
		analog::NETLIB_SUB(R_base) m_R3;
		analog::NETLIB_SUB(R_base) m_ROUT;
		analog::NETLIB_SUB(R_base) m_RDIS;

		logic_input_t m_RESET;
		analog_input_t m_THRES;
		analog_input_t m_TRIG;
		analog_output_t m_OUT;

		state_var<bool> m_last_out;
		state_var<bool> m_ff;
		state_var<bool> m_last_reset;
		state_var<nl_fptype> m_overshoot;
		state_var<nl_fptype> m_undershoot;
		nl_fptype m_ovlimit;

		nl_fptype clamp_hl(const nl_fptype v, const nl_fptype a, const nl_fptype b) noexcept
		{
			const nl_fptype vcc = m_R1.P()();
			return plib::clamp(v, b, vcc - a);
		}
	};

	NETLIB_DEVICE_IMPL(NE555,     "NE555", "")

	NETLIB_DEVICE_IMPL_ALIAS(MC1455P, NE555,     "MC1455P", "")

} // namespace netlist::devices
