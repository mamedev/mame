// license:GPL-2.0+
// copyright-holders:Couriersud
/*!
 *  \file nld_NE555.cpp
 *
 *  \page NE555 NE555: PRECISION TIMERS
 *
 *  The Swiss army knife for timing purposes
 *
 *  \section ne555_1 Synopsis
 *
 *  \snippet devsyn.dox.h NE555 synopsis
 *  \snippet devsyn.dox.h NE555_DIP synopsis

 *  \section ne555_11 "C" Synopsis
 *
 *  \snippet devsyn.dox.h NE555 csynopsis
 *  \snippet devsyn.dox.h NE555_DIP csynopsis
 *
 *  For the \c NE555 use verbose pin assignments like \c name.TRIG or \c name.OUT.
 *  For the \c NE555_DIP use pin numbers like \c name.1.
 *
 *  \section ne555_2 Connection Diagram
 *
 *  <pre>
 *          +--------+
 *      GND |1  ++  8| VCC
 *     TRIG |2      7| DISCH
 *      OUT |3      6| THRES
 *    RESET |4      5| CONT
 *          +--------+
 *  </pre>
 *
 *  Naming conventions follow Texas Instruments datasheet
 *
 *  \section ne555_3 Function Table
 *
 *  Please refer to the datasheet.
 *
 *  \section ne555_4 Limitations
 *
 *  Internal resistor network currently fixed to 5k.
 *
 *  \section ne555_5 Example
 *  \snippet ne555_astable.c ne555 example
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
		, m_RDIS(*this, "RDIS")
		, m_RESET(*this, "RESET")     // Pin 4
		, m_THRES(*this, "THRESH")    // Pin 6
		, m_TRIG(*this, "TRIG")       // Pin 2
		, m_OUT(*this, "OUT")         // Pin 3
		, m_last_out(*this, "m_last_out", false)
		, m_ff(*this, "m_ff", false)
		{
			register_subalias("GND",  m_R3.m_N);    // Pin 1
			register_subalias("CONT", m_R1.m_N);    // Pin 5
			register_subalias("DISCH", m_RDIS.m_P); // Pin 7
			register_subalias("VCC",  m_R1.m_P);    // Pin 8

			connect(m_R1.m_N, m_R2.m_P);
			connect(m_R2.m_N, m_R3.m_P);
			connect(m_RDIS.m_N, m_R3.m_N);
		}

		NETLIB_UPDATEI();
		NETLIB_RESETI();

	protected:
		analog::NETLIB_SUB(R_base) m_R1;
		analog::NETLIB_SUB(R_base) m_R2;
		analog::NETLIB_SUB(R_base) m_R3;
		analog::NETLIB_SUB(R_base) m_RDIS;

		logic_input_t m_RESET;
		analog_input_t m_THRES;
		analog_input_t m_TRIG;
		analog_output_t m_OUT;

	private:
		state_var<bool> m_last_out;
		state_var<bool> m_ff;

		nl_double clamp(const nl_double v, const nl_double a, const nl_double b)
		{
			nl_double ret = v;
			nl_double vcc = m_R1.m_P();

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
			register_subalias("1",  m_R3.m_N);      // Pin 1
			register_subalias("2",    m_TRIG);      // Pin 2
			register_subalias("3",    m_OUT);       // Pin 3
			register_subalias("4",   m_RESET);      // Pin 4
			register_subalias("5", m_R1.m_N);       // Pin 5
			register_subalias("6",  m_THRES);       // Pin 6
			register_subalias("7", m_RDIS.m_P);     // Pin 7
			register_subalias("8",  m_R1.m_P);      // Pin 8
		}
	};

	NETLIB_RESET(NE555)
	{
		m_R1.reset();
		m_R2.reset();
		m_R3.reset();
		m_RDIS.reset();

		/* FIXME make resistance a parameter, properly model other variants */
		m_R1.set_R(5000);
		m_R2.set_R(5000);
		m_R3.set_R(5000);
		m_RDIS.set_R(R_OFF);

		m_last_out = true;
	}

	NETLIB_UPDATE(NE555)
	{
		// FIXME: assumes GND is connected to 0V.

		nl_double vt = clamp(m_R2.m_P(), 0.7, 1.4);
		bool bthresh = (m_THRES() > vt);
		bool btrig = (m_TRIG() > clamp(m_R2.m_N(), 0.7, 1.4));

		if (!btrig)
		{
			m_ff = true;
		}
		else if (bthresh)
		{
			m_ff = false;
		}

		bool out = (!m_RESET() ? false : m_ff);

		if (m_last_out && !out)
		{
			m_RDIS.update();
			m_OUT.push(m_R3.m_N());
			m_RDIS.set_R(R_ON);
		}
		else if (!m_last_out && out)
		{
			m_RDIS.update();
			// FIXME: Should be delayed by 100ns
			m_OUT.push(m_R1.m_P());
			m_RDIS.set_R(R_OFF);
		}
		m_last_out = out;
	}

	NETLIB_DEVICE_IMPL(NE555,     "NE555", "")
	NETLIB_DEVICE_IMPL(NE555_dip, "NE555_DIP", "")

	} //namespace devices
} // namespace netlist
