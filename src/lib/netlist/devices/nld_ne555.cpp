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


/*
 * "Description: The Swiss army knife for timing purposes\n"
 * "    which has a ton of applications.\n"
 * "DipAlias: GND,TRIG,OUT,RESET,VCC,DISCH,THRES,CONT\n"
 * "Package: DIP\n"
 * "NamingConvention: Naming conventions follow Texas Instruments datasheet\n"
 * "Limitations: Internal resistor network currently fixed to 5k\n"
 * "     more limitations\n"
 * "Function Table:\n"
 *
 *  Function table created from truthtable if missing.
 *
 *  For package, refer to:
 *
 *  https://en.wikipedia.org/wiki/List_of_integrated_circuit_packaging_types
 *
 *  Special case: GATE -> use symbolic names
 *
 */

#include "nld_ne555.h"
#include "netlist/analog/nlid_twoterm.h"
#include "netlist/solver/nld_solver.h"

#define R_OFF (1E20)
#define R_ON (25)   // Datasheet states a maximum discharge of 200mA, R = 5V / 0.2

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
		, m_RESET(*this, "RESET")     // Pin 4
		, m_THRES(*this, "THRESH")    // Pin 6
		, m_TRIG(*this, "TRIG")       // Pin 2
		, m_OUT(*this, "_OUT")        // to Pin 3 via ROUT
		, m_last_out(*this, "m_last_out", false)
		, m_ff(*this, "m_ff", false)
		, m_last_reset(*this, "m_last_reset", false)
		{
			register_subalias("GND",  m_R3.m_N);    // Pin 1
			register_subalias("CONT", m_R1.m_N);    // Pin 5
			register_subalias("DISCH", m_RDIS.m_P); // Pin 7
			register_subalias("VCC",  m_R1.m_P);    // Pin 8
			register_subalias("OUT",  m_ROUT.m_P);  // Pin 3

			connect(m_R1.m_N, m_R2.m_P);
			connect(m_R2.m_N, m_R3.m_P);
			connect(m_RDIS.m_N, m_R3.m_N);
			connect(m_OUT, m_ROUT.m_N);
		}

		NETLIB_UPDATEI();
		NETLIB_RESETI();

	private:
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

		nl_fptype clamp(const nl_fptype v, const nl_fptype a, const nl_fptype b)
		{
			nl_fptype ret = v;
			nl_fptype vcc = m_R1.m_P();

			if (ret >  vcc - a)
				ret = vcc - a;
			if (ret < b)
				ret = b;
			return ret;
		}
	};

	NETLIB_OBJECT_DERIVED(NE555_dip, NE555)
	{
		NETLIB_CONSTRUCTOR_DERIVED(NE555_dip, NE555)
		{
			register_subalias("1", "GND");      // Pin 1
			register_subalias("2", "TRIG");     // Pin 2
			register_subalias("3", "OUT");      // Pin 3
			register_subalias("4", "RESET");    // Pin 4
			register_subalias("5", "CONT");     // Pin 5
			register_subalias("6", "THRESH");   // Pin 6
			register_subalias("7", "DISCH");    // Pin 7
			register_subalias("8", "VCC");      // Pin 8
		}
	};

	NETLIB_RESET(NE555)
	{
		m_R1.reset();
		m_R2.reset();
		m_R3.reset();
		m_ROUT.reset();
		m_RDIS.reset();

		/* FIXME make resistances a parameter, properly model other variants */
		m_R1.set_R(nlconst::magic(5000));
		m_R2.set_R(nlconst::magic(5000));
		m_R3.set_R(nlconst::magic(5000));
		m_ROUT.set_R(nlconst::magic(20));
		m_RDIS.set_R(nlconst::magic(R_OFF));

		m_last_out = true;
	}

	NETLIB_UPDATE(NE555)
	{
		// FIXME: assumes GND is connected to 0V.

		const auto reset = m_RESET();

		if (!reset && m_last_reset)
		{
			m_ff = false;
		}
		else
		{
			const nl_fptype vt = clamp(m_R2.m_P(), nlconst::magic(0.7), nlconst::magic(1.4));
			const bool bthresh = (m_THRES() > vt);
			const bool btrig = (m_TRIG() > clamp(m_R2.m_N(), nlconst::magic(0.7), nlconst::magic(1.4)));

			if (!btrig)
				m_ff = true;
			else if (bthresh)
				m_ff = false;
		}

		const bool out = (!reset ? false : m_ff);

		if (m_last_out && !out)
		{
			m_RDIS.update();
			m_OUT.push(m_R3.m_N());
			m_RDIS.set_R(nlconst::magic(R_ON));
		}
		else if (!m_last_out && out)
		{
			m_RDIS.update();
			// FIXME: Should be delayed by 100ns
			m_OUT.push(m_R1.m_P());
			m_RDIS.set_R(nlconst::magic(R_OFF));
		}
		m_last_reset = reset;
		m_last_out = out;
	}

	NETLIB_DEVICE_IMPL(NE555,     "NE555", "")
	NETLIB_DEVICE_IMPL(NE555_dip, "NE555_DIP", "")

	NETLIB_DEVICE_IMPL_ALIAS(MC1455P, NE555,     "MC1455P", "")
	NETLIB_DEVICE_IMPL_ALIAS(MC1455P_dip, NE555_dip, "MC1455P_DIP", "")
	} //namespace devices
} // namespace netlist
