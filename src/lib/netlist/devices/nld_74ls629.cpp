// license:BSD-3-Clause
// copyright-holders:Couriersud
/*
 * nld_SN74LS629.cpp
 *
 *  SN74LS629: VOLTAGE-CONTROLLED OSCILLATORS
 *
 *          +--------------+
 *      2FC |1     ++    16| VCC
 *      1FC |2           15| QSC VCC
 *     1RNG |3           14| 2RNG
 *     1CX1 |4  74LS629  13| 2CX1
 *     1CX2 |5           12| 2CX2
 *     1ENQ |6           11| 2ENQ
 *       1Y |7           10| 2Y
 *  OSC GND |8            9| GND
 *          +--------------+
 *
 *  Naming conventions follow Texas Instruments datasheet
 *
 *  NOTE: The CX1 and CX2 pins are not connected!
 *        The capacitor value has to be specified as a parameter.
 *        There are more comments on the challenges of emulating this
 *        chip in the *.c file
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

#include "analog/nlid_twoterm.h"

namespace netlist::devices {

	struct SN74LS629clk
	{
		SN74LS629clk(device_t &owner)
		: m_FB(owner, "FB", nl_delegate(&SN74LS629clk::fb, this))
		, m_Y(owner, "Y")
		, m_enableq(owner, "m_enableq", 0)
		, m_out(owner, "m_out", 0)
		, m_inc(owner, "m_inc", netlist_time::zero())
		{
			owner.connect("FB", "Y");
		}

	public:
		logic_input_t m_FB;
		logic_output_t m_Y;

		state_var<netlist_sig_t> m_enableq;
		state_var<netlist_sig_t> m_out;
		state_var<netlist_time> m_inc;

	private:
		NETLIB_HANDLERI(fb)
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

	};

	NETLIB_OBJECT(SN74LS629)
	{
		NETLIB_CONSTRUCTOR(SN74LS629)
		, m_clock(*this)
		, m_R_FC(*this, "R_FC")
		, m_R_RNG(*this, "R_RNG")
		, m_ENQ(*this, "ENQ", NETLIB_DELEGATE(inputs))
		, m_RNG(*this, "RNG", NETLIB_DELEGATE(inputs))
		, m_FC(*this, "FC", NETLIB_DELEGATE(inputs))
		, m_CAP(*this, "CAP", nlconst::magic(1e-6))
		, m_power_pins(*this)
		, m_power_pins_osc(*this, "OSCVCC", "OSCGND")
		{
			connect("OSCGND", "R_FC.2");

			connect("FC", "R_FC.1");
			connect("RNG", "R_RNG.1");
			connect("R_FC.2", "R_RNG.2");
		}

	private:
		NETLIB_RESETI()
		{
			m_R_FC().set_R( nlconst::magic(90000.0));
			m_R_RNG().set_R(nlconst::magic(90000.0));
		}

		NETLIB_UPDATE_PARAMI()
		{
			/* update param may be called from anywhere, update_dev(time) is not a good idea */
		}

		SN74LS629clk m_clock;
		NETLIB_SUB_NS(analog, R_base) m_R_FC;
		NETLIB_SUB_NS(analog, R_base) m_R_RNG;

		logic_input_t m_ENQ;
		analog_input_t m_RNG;
		analog_input_t m_FC;

		param_fp_t m_CAP;
		nld_power_pins m_power_pins;
		nld_power_pins m_power_pins_osc;

		NETLIB_HANDLERI(inputs)
		{
			{
				// recompute
				nl_fptype  v_freq = m_FC();
				nl_fptype  v_rng = m_RNG();

				/* coefficients */
				const nl_fptype k1 =  nlconst::magic( 1.9904769024796283E+03);
				const nl_fptype k2 =  nlconst::magic( 1.2070059213983407E+03);
				const nl_fptype k3 =  nlconst::magic( 1.3266985579561108E+03);
				const nl_fptype k4 =  nlconst::magic(-1.5500979825922698E+02);
				const nl_fptype k5 =  nlconst::magic( 2.8184536266938172E+00);
				const nl_fptype k6 =  nlconst::magic(-2.3503421582744556E+02);
				const nl_fptype k7 =  nlconst::magic(-3.3836786704527788E+02);
				const nl_fptype k8 =  nlconst::magic(-1.3569136703258670E+02);
				const nl_fptype k9 =  nlconst::magic( 2.9914575453819188E+00);
				const nl_fptype k10 = nlconst::magic( 1.6855569086173170E+00);

				/* scale due to input resistance */

				/* Polyfunctional3D_model created by zunzun.com using sum of squared absolute error */

				nl_fptype v_freq_2 = v_freq * v_freq;
				nl_fptype v_freq_3 = v_freq_2 * v_freq;
				nl_fptype v_freq_4 = v_freq_3 * v_freq;
				nl_fptype freq = k1;
				freq += k2 * v_freq;
				freq += k3 * v_freq_2;
				freq += k4 * v_freq_3;
				freq += k5 * v_freq_4;
				freq += k6 * v_rng;
				freq += k7 * v_rng * v_freq;
				freq += k8 * v_rng * v_freq_2;
				freq += k9 * v_rng * v_freq_3;
				freq += k10 * v_rng * v_freq_4;

				freq *= nlconst::magic(0.1e-6) / m_CAP();

				// FIXME: we need a possibility to remove entries from queue ...
				//        or an exact model ...
				m_clock.m_inc = netlist_time::from_fp(nlconst::half() / freq);
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

	};

	NETLIB_DEVICE_IMPL(SN74LS629,     "SN74LS629",     "CAP,@VCC,@GND")

} // namespace netlist::devices
