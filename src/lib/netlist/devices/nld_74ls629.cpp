// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_SN74LS629.c
 *
 */

/*
 * The 74LS624 series are constant current based VCOs.  The Freq Control voltage
 * modulates the current source.  The current is created from Rext, which is
 * internally fixed at 600 ohms for all devices except the 74LS628 which has
 * external connections.  The current source linearly discharges the cap voltage.
 * The cap starts with 0V charge across it.  One side is connected to a fixed voltage
 * bias circuit.  The other side is charged negatively from the current source until
 * a certain low threshold is reached.  Once this threshold is reached, the output
 * toggles state and the pins on the cap reverse in respect to the charge/bias hookup.
 * This starts the one side of the cap to be at bias, and the other side of the cap is
 * now at bias + the charge on the cap which is bias - threshold.
 * Y = 0;  CX1 = bias;    CX2 = charge
 * Y = 1;  CX1 = charge;  CX2 = bias
 * The Range voltage adjusts the threshold voltage.  The higher the Range voltage,
 * the lower the threshold voltage, the longer the cap can charge, the lower the frequency.
 *
 * In a perfect world it would work like this:
 * The current is based on the mysterious Rext mentioned in the data sheet.
 * I = (VfreqControl  * 20k/90k) / Rext
 * where Rext = 600 ohms or external Rext on a 74LS628
 * The Freq Control has an input impedance of approximately 90k, so any input resistance
 * connected to the Freq Control pin works as a voltage divider.
 * I = (VfreqControl * 20k/(90k + RfreqControlIn)) / Rext
 * That gives us a change in voltage on the cap of
 * dV = I / sampleRate / C_inFarads
 *
 * Unfortunately the chip does not behave linearly do to internal interactions,
 * so I have just worked out the formula (using zunzun.com) of FreqControl and
 * range to frequency out for a fixed cap value of 0.1uf.  Other cap values can just
 * scale from that.  From the freq, we calculate the time of 1/2 cycle using 1/Freq/2.
 * Then just use that to toggle a waveform.
 */


#include "nld_74ls629.h"
#include "../analog/nlid_twoterm.h"

namespace netlist
{
	namespace devices
	{
	NETLIB_OBJECT(SN74LS629clk)
	{
		NETLIB_CONSTRUCTOR(SN74LS629clk)
		, m_FB(*this, "FB")
		, m_Y(*this, "Y")
		, m_enableq(*this, "m_enableq", 1)
		, m_out(*this, "m_out", 0)
		, m_inc(*this, "m_inc", netlist_time::zero())
		{
			connect(m_FB, m_Y);
		}

		NETLIB_RESETI()
		{
			m_enableq = 0;
			m_out = 0;
			m_inc = netlist_time::zero();
		}

		NETLIB_UPDATEI();

	public:
		logic_input_t m_FB;
		logic_output_t m_Y;

		state_var<netlist_sig_t> m_enableq;
		state_var<netlist_sig_t> m_out;
		state_var<netlist_time> m_inc;
	};

	NETLIB_OBJECT(SN74LS629)
	{
		NETLIB_CONSTRUCTOR(SN74LS629)
		, m_clock(*this, "OSC")
		, m_R_FC(*this, "R_FC")
		, m_R_RNG(*this, "R_RNG")
		, m_ENQ(*this, "ENQ")
		, m_RNG(*this, "RNG")
		, m_FC(*this, "FC")
		, m_CAP(*this, "CAP", 1e-6)
		{
			register_subalias("GND",    m_R_FC.m_N);

			connect(m_FC, m_R_FC.m_P);
			connect(m_RNG, m_R_RNG.m_P);
			connect(m_R_FC.m_N, m_R_RNG.m_N);

			register_subalias("Y", m_clock.m_Y);
		}

		NETLIB_RESETI()
		{
			m_R_FC.set_R(90000.0);
			m_R_RNG.set_R(90000.0);
			m_clock.reset();
		}
		NETLIB_UPDATEI();

		NETLIB_UPDATE_PARAMI()
		{
			/* update param may be called from anywhere, update_dev(time) is not a good idea */
		}

	public:
		NETLIB_SUB(SN74LS629clk) m_clock;
		analog::NETLIB_SUB(R_base) m_R_FC;
		analog::NETLIB_SUB(R_base) m_R_RNG;

		logic_input_t m_ENQ;
		analog_input_t m_RNG;
		analog_input_t m_FC;

		param_double_t m_CAP;
	};

	NETLIB_OBJECT(SN74LS629_dip)
	{
		NETLIB_CONSTRUCTOR(SN74LS629_dip)
		, m_1(*this, "1")
		, m_2(*this, "2")
		{
			register_subalias("1",  m_2.m_FC);
			register_subalias("2",  m_1.m_FC);
			register_subalias("3",  m_1.m_RNG);

			register_subalias("6",  m_1.m_ENQ);
			register_subalias("7",  m_1.m_clock.m_Y);

			register_subalias("8",  m_1.m_R_FC.m_N);
			register_subalias("9",  m_1.m_R_FC.m_N);
			connect(m_1.m_R_FC.m_N, m_2.m_R_FC.m_N);

			register_subalias("10",  m_2.m_clock.m_Y);

			register_subalias("11",  m_2.m_ENQ);
			register_subalias("14",  m_2.m_RNG);
		}

		NETLIB_UPDATEI() { }

		NETLIB_RESETI()
		{
			m_1.reset();
			m_2.reset();
		}

	private:
		NETLIB_SUB(SN74LS629) m_1;
		NETLIB_SUB(SN74LS629) m_2;
	};


	NETLIB_UPDATE(SN74LS629clk)
	{
		if (!m_enableq)
		{
			m_out = m_out ^ 1;
			m_Y.push(m_out, m_inc);
		}
		else
		{
			m_Y.push(1, m_inc);
		}
	}

	NETLIB_UPDATE(SN74LS629)
	{
		{
			// recompute
			nl_double  freq;
			nl_double  v_freq_2, v_freq_3, v_freq_4;
			nl_double  v_freq = m_FC();
			nl_double  v_rng = m_RNG();

			/* coefficients */
			const nl_double k1 = 1.9904769024796283E+03;
			const nl_double k2 = 1.2070059213983407E+03;
			const nl_double k3 = 1.3266985579561108E+03;
			const nl_double k4 = -1.5500979825922698E+02;
			const nl_double k5 = 2.8184536266938172E+00;
			const nl_double k6 = -2.3503421582744556E+02;
			const nl_double k7 = -3.3836786704527788E+02;
			const nl_double k8 = -1.3569136703258670E+02;
			const nl_double k9 = 2.9914575453819188E+00;
			const nl_double k10 = 1.6855569086173170E+00;

			/* scale due to input resistance */

			/* Polyfunctional3D_model created by zunzun.com using sum of squared absolute error */

			v_freq_2 = v_freq * v_freq;
			v_freq_3 = v_freq_2 * v_freq;
			v_freq_4 = v_freq_3 * v_freq;
			freq = k1;
			freq += k2 * v_freq;
			freq += k3 * v_freq_2;
			freq += k4 * v_freq_3;
			freq += k5 * v_freq_4;
			freq += k6 * v_rng;
			freq += k7 * v_rng * v_freq;
			freq += k8 * v_rng * v_freq_2;
			freq += k9 * v_rng * v_freq_3;
			freq += k10 * v_rng * v_freq_4;

			freq *= plib::constants<nl_double>::cast(0.1e-6) / m_CAP();

			// FIXME: we need a possibility to remove entries from queue ...
			//        or an exact model ...
			m_clock.m_inc = netlist_time::from_double(0.5 / freq);
			//m_clock.update();

			//NL_VERBOSE_OUT(("{1} {2} {3} {4}\n", name(), v_freq, v_rng, freq));
		}

		if (!m_clock.m_enableq && m_ENQ())
		{
			m_clock.m_enableq = 1;
			m_clock.m_out = m_clock.m_out ^ 1;
			m_clock.m_Y.push(m_clock.m_out, netlist_time::from_nsec(1));
		}
		else if (m_clock.m_enableq && !m_ENQ())
		{
			m_clock.m_enableq = 0;
			m_clock.m_out = m_clock.m_out ^ 1;
			m_clock.m_Y.push(m_clock.m_out, netlist_time::from_nsec(1));
		}
	}

	NETLIB_DEVICE_IMPL(SN74LS629,     "SN74LS629",     "CAP")
	NETLIB_DEVICE_IMPL(SN74LS629_dip, "SN74LS629_DIP", "1.CAP1,2.CAP2")

	} //namespace devices
} // namespace netlist
