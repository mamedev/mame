// license:GPL-2.0+
// copyright-holders:Couriersud

//- Identifier:  NE555_DIP
//- Title: NE555 PRECISION TIMERS
//- Description:
//-   These devices are precision timing circuits capable of producing accurate
//-   time delays or oscillation. In the time-delay or monostable mode of
//-   operation, the timed interval is controlled by a single external resistor
//-   and capacitor network. In the astable mode of operation, the frequency and
//-   duty cycle can be controlled independently with two external resistors and
//-   a single external capacitor.
//-
//-   The threshold and trigger levels normally are two-thirds and one-third,
//-   respectively, of V CC. These levels can be altered by use of the
//-   control-voltage terminal. When the trigger input falls below the trigger
//-   level, the flip-flop is set and NC – No internal connection the output
//-   goes high. If the trigger input is above the trigger level and the
//-   threshold input is above the threshold level, the flip-flop is reset and
//-   the output is low. The reset (RESET) input can override all other
//-   inputs and can be used to initiate a new timing cycle. When RESET goes
//-   low, the flip-flop is reset and the output goes low. When the output is low,
//-   a low-impedance path is provided between discharge (DISCH) and ground.
//-
//-   The output circuit is capable of sinking or sourcing current up to 200 mA.
//-   Operation is specified for supplies of 5 V to 15 V. With a 5-V supply,
//-   output levels are compatible with TTL inputs.
//-
//-   The NE555 is characterized for operation from 0°C to 70°C. The SA555 is
//-   characterized for operation from –40°C to 85°C. The SE555 is characterized
//-   for operation over the full military range of –55°C to 125°C.
//-
//- Pinalias: GND,TRIG,OUT,RESET,CONT,THRES,DISCH,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow Texas instrument datasheet
//- Limitations: Internal resistor network currently fixed to 5k
//-   If TRIG and TRESH are connected overshoot compensation will be enabled.
//-   The approach is raw but delivers results (at 5 to 10 steps per discharge/charge)
//-   within a couple of percent. Please take into account that any datasheet
//-   formulas are idealistic. Neither capacitor, resistor, internal resistor
//-   tolerances are taken into account. Nor are ambient temperature and chip
//-   temperature. Thus the result is considered acceptable.
//-   The datasheet states a maximum discharge of 200mA, this is not modelled
//-   Instead an impedance of 1 Ohm is used.
//-
//- Example: ne555_astable.c,ne555_example
//- FunctionTable:
//-
//-    |RESET|TRIGGER VOLTAGE|THRESHOLD VOLTAGE|OUTPUT|DISCHARGE SWITCH|
//-    |:---:|:-------------:|:---------------:|:----:|:--------------:|
//-    |Low  | Irrelevant    | Irrelevant      |  Low |    On          |
//-    |High | <1/3 VDD      | Irrelevant      | High |    Off         |
//-    |High | >1/3 VDD      | >2/3 VDD        | Low  |    On          |
//-    |High | >1/3 VDD      | <2/3 VDD        | As previously established||
//-

#include "nld_ne555.h"
#include "netlist/analog/nlid_twoterm.h"
#include "netlist/solver/nld_solver.h"

#include "plib/pmath.h"

#define R_OFF (1E20)
#define R_ON  (1)

namespace netlist
{
	namespace devices
	{
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
			register_subalias("GND",  m_R3.N());    // Pin 1
			register_subalias("CONT", m_R1.N());    // Pin 5
			register_subalias("DISCH", m_RDIS.P()); // Pin 7
			register_subalias("VCC",  m_R1.P());    // Pin 8
			register_subalias("OUT",  m_ROUT.P());  // Pin 3

			connect(m_R1.N(), m_R2.P());
			connect(m_R2.N(), m_R3.P());
			connect(m_RDIS.N(), m_R3.N());
			connect(m_OUT, m_ROUT.N());
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

	NETLIB_OBJECT(NE555_dip)
	{
		NETLIB_CONSTRUCTOR(NE555_dip)
		, A(*this, "A")
		{
			register_subalias("1", "A.GND");      // Pin 1
			register_subalias("2", "A.TRIG");     // Pin 2
			register_subalias("3", "A.OUT");      // Pin 3
			register_subalias("4", "A.RESET");    // Pin 4
			register_subalias("5", "A.CONT");     // Pin 5
			register_subalias("6", "A.THRESH");   // Pin 6
			register_subalias("7", "A.DISCH");    // Pin 7
			register_subalias("8", "A.VCC");      // Pin 8
		}
		// FIXME: R_base needs to be removed from the code base
		// The reset on R_Base executed after NE555 reset will
		// overwrite values.
		NETLIB_RESETI() { A.reset(); }
	private:
		NETLIB_SUB(NE555) A;
	};

	NETLIB_DEVICE_IMPL(NE555,     "NE555", "")
	NETLIB_DEVICE_IMPL(NE555_dip, "NE555_DIP", "")

	NETLIB_DEVICE_IMPL_ALIAS(MC1455P, NE555,     "MC1455P", "")
	NETLIB_DEVICE_IMPL_ALIAS(MC1455P_dip, NE555_dip, "MC1455P_DIP", "")
	} //namespace devices
} // namespace netlist
