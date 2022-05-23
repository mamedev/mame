// license:BSD-3-Clause
// copyright-holders:Couriersud
/*
 * nld_74123.cpp
 *
 *  74123: Dual Retriggerable One-Shot with Clear and Complementary Outputs
 *
 *           +--------------+
 *        A1 |1     ++    16| VCC
 *        B1 |2           15| RC1
 *      CLR1 |3           14| C1
 *       Q1Q |4   74123   13| Q1
 *        Q2 |5           12| Q2Q
 *        C2 |6           11| CLR2
 *       RC2 |7           10| B2
 *       GND |8            9| A2
 *           +--------------+
 *
 *  Naming conventions follow Fairchild Semiconductor datasheet
 *
 *  DM9602: Dual Retriggerable, Resettable One Shots
 *
 *           +--------------+
 *        C1 |1     ++    16| VCC
 *       RC1 |2           15| C2
 *      CLR1 |3           14| RC2
 *        B1 |4    9602   13| CLR2
 *        A1 |5           12| B2
 *        Q1 |6           11| A2
 *       Q1Q |7           10| Q2
 *       GND |8            9| Q2Q
 *           +--------------+
 *
 *  CD4538: Dual Retriggerable, Resettable One Shots
 *
 *           +--------------+
 *        C1 |1     ++    16| VCC
 *       RC1 |2           15| C2
 *      CLR1 |3           14| RC2
 *        A1 |4    4538   13| CLR2
 *        B1 |5           12| A2
 *        Q1 |6           11| B2
 *       Q1Q |7           10| Q2
 *       GND |8            9| Q2Q
 *           +--------------+
 *
 */

#include "analog/nlid_twoterm.h"
#include "nlid_system.h"

namespace netlist::devices {

	/// \brief Base monostable device
	///
	/// The basic operation is the following:
	///
	/// After a trigger signal has been detected, the output changes to high.
	/// The capacitor is quickly discharged until it reaches Vlow.
	/// The device toggles into charging mode until Vhigh is reached. At this
	/// point, the output is set to low. Charging continues until VCC - or
	/// more specific VRC is reached.
	///
	/// Using
	///
	/// - Vlow = alpha * VCC
	/// - Vhigh = (1-alpha) * VCC
	/// - Ignoring Rext during discharge
	///
	/// we have calculate the following time constants:
	///
	/// - tD = - X*C * ln(alpha)
	/// - tC = - R*C * ln(alhpa)
	/// - tL = - R*C * ln(1.0 - alpha)
	///
	/// where tD denotes the time to discharge from VCC to Vlow, tC denotes
	/// the time to charge from 0.0 to Vhigh and tL denotes the time to charge
	/// from 0 to Vlow. X denotes the internal resistance used to discharge the
	/// capacitor. The total impulse duration is thus
	///
	/// tP = tD + tC - tL
	///
	/// Using K = ln(1 - alpha) - ln (alpha) = ln(1/alpha - 1)
	///
	/// we get
	///
	/// tP = R*C*K * (1 + X/R * ln(alpha) / K)
	///
	/// and alpha = 1 / (1 + exp(K))
	///
	/// Datasheets express the right term usually as
	///
	/// (1 + f * 1000 / R) = (1 + X/R * ln(alpha) / K)
	///
	/// and thus
	///
	/// X = f * 1000 * K / ln(alpha)
	///
	/// FIXME: Currently X is given directly (as RI). It would be better to use
	/// f (usually 0.7) and K to calculate X.
	///
	template <typename D>
	NETLIB_OBJECT(74123_base)
	{
		NETLIB_CONSTRUCTOR(74123_base)
		, m_RP(*this, "RP")
		, m_RN(*this, "RN")
		, m_RP_Q(*this, "_RP_Q")
		, m_RN_Q(*this, "_RN_Q")
		, m_I(*this, D::names(), NETLIB_DELEGATE(ab_clear))
		, m_Q(*this, "Q")
		, m_QQ(*this, "QQ")
		, m_CV(*this, "_CV", NETLIB_DELEGATE(cv)) // internal
		, m_last_trig(*this, "m_last_trig", 0)
		, m_state(*this, "m_state", 0)
		, m_KP(plib::reciprocal(nlconst::one() + plib::exp(D::K())))
		//, m_power_pins(*this)
		{

			register_subalias(pstring(D::gnd()), "RN.2");
			register_subalias(pstring(D::vcc()), "RP.1");
			register_subalias("C",   "RN.2");
			register_subalias("RC",  "RN.1");

			connect("_RP_Q", "RP.I");
			connect("_RN_Q", "RN.I");

			connect("RN.1", "RP.2");
			connect("_CV", "RN.1");

			m_RP.m_RON.set(D::RI());
			m_RN.m_RON.set(D::RI());
		}

		NETLIB_HANDLERI(ab_clear)
		{
			netlist_sig_t m_trig(0);

			m_trig = D::trigfunc(m_I);

			if (!D::clear(m_I))
			{
				m_Q.push(0, D::t_C_to_Q::value());
				m_QQ.push(1, D::t_C_to_Q::value());
				/* quick charge until trigger */
				/* FIXME: SGS datasheet shows quick charge to 5V,
				 * though schematics indicate quick charge to Vhigh only.
				 */
				m_RP_Q.push(1, D::t_C_to_Q::value()); // R_ON
				m_RN_Q.push(0, D::t_C_to_Q::value()); // R_OFF
				m_state = 2; //charging (quick)
			}
			else if (!m_last_trig && m_trig)
			{
				// FIXME: Timing!
				m_Q.push(1, D::t_AB_to_Q::value());
				m_QQ.push(0, D::t_AB_to_Q::value());

				m_RN_Q.push(1, D::t_AB_to_Q::value()); // R_ON
				m_RP_Q.push(0, D::t_AB_to_Q::value()); // R_OFF

				m_state = 1; // discharging
			}

			m_last_trig = m_trig;
		}

		NETLIB_HANDLERI(cv)
		{
			if (m_state == 1)
			{
				const nl_fptype vLow = m_KP * m_RP.P()();
				if (m_CV() < vLow)
				{
					m_RN_Q.push(0, NLTIME_FROM_NS(10)); // R_OFF
					m_state = 2; // charging
				}
			}
			if (m_state == 2)
			{
				const nl_fptype vHigh = (nlconst::one() - m_KP) * m_RP.P()();
				if (m_CV() > vHigh)
				{
					m_RP_Q.push(0, NLTIME_FROM_NS(10)); // R_OFF

					m_Q.push(0, NLTIME_FROM_NS(10));
					m_QQ.push(1, NLTIME_FROM_NS(10));
					m_state = 0; // waiting
				}
			}
		}

		NETLIB_RESETI()
		{
			m_RP.reset();
			m_RN.reset();

			//m_RP.set_R(R_OFF);
			//m_RN.set_R(R_OFF);

			m_last_trig = 0;
			m_state = 0;
		}

	private:
		NETLIB_SUB(sys_dsw1) m_RP;
		NETLIB_SUB(sys_dsw1) m_RN;

		logic_output_t m_RP_Q;
		logic_output_t m_RN_Q;

		object_array_t<logic_input_t, 3> m_I;
		logic_output_t m_Q;
		logic_output_t m_QQ;

		analog_input_t m_CV;

		state_var<netlist_sig_t> m_last_trig;
		state_var<unsigned>      m_state;
		nl_fptype                m_KP;
		//nld_power_pins           m_power_pins; // not needed, device exposes VCC and GND
	};

	struct desc_74123 : public desc_base
	{
		using t_AB_to_Q = time_ns<10>;
		using t_C_to_Q = time_ns<10>;
		static constexpr nl_fptype K()  { return nlconst::magic(0.4); }
		static constexpr nl_fptype RI() { return nlconst::magic(400.0); }

		static constexpr std::array<const char *, 3> names() { return {"CLRQ", "A", "B"};}
		template<typename T>
		static netlist_sig_t trigfunc(const T &in)
		{
			return ((in[1]() | (in[2]() ^ 1)) ^ 1) & in[0](); // ((m_A() | (m_B() ^ 1)) ^ 1) & m_CLRQ()
		}
		template<typename T> static constexpr netlist_sig_t clear(const T &in) { return in[0]();}

		static constexpr const char *vcc() { return "VCC"; }
		static constexpr const char *gnd() { return "GND"; }
	};

	struct desc_74121 : public desc_74123
	{
		static constexpr nl_fptype K()  { return nlconst::magic(0.7); }

		static constexpr std::array<const char *, 3> names() { return {"A1", "A2", "B"};}
		template<typename T>
		static netlist_sig_t trigfunc(const T &in)
		{
			return ((in[0]() ^ 1) | (in[1]() ^ 1)) & in[2](); // (~A1 | ~A2) & B
		}
		template<typename T> static constexpr netlist_sig_t clear([[maybe_unused]] const T &in) { return 1; }
	};

	struct desc_9602 : public desc_74123
	{
		template<typename T>
		static netlist_sig_t trigfunc(const T &in)
		{
			return ((in[1]() ^ 1) | in[2]()); // (m_A() ^ 1) | m_B()
		}
	};

	struct desc_4538 : public desc_74123
	{
		using t_AB_to_Q = time_ns<300>;
		using t_C_to_Q = time_ns<250>;
		static constexpr nl_fptype K() { return nlconst::one(); } // CD4538 datasheet states PW=RC

		template<typename T>
		static netlist_sig_t trigfunc(const T &in)
		{
			return (in[1]() | (in[2]() ^ 1)); // m_A() | (m_B() ^ 1)
		}
		static constexpr const char *vcc() { return "VDD"; }
		static constexpr const char *gnd() { return "VSS"; }
	};

	using NETLIB_NAME(74123) = NETLIB_NAME(74123_base)<desc_74123>;
	using NETLIB_NAME(74121) = NETLIB_NAME(74123_base)<desc_74121>;
	using NETLIB_NAME(4538) = NETLIB_NAME(74123_base)<desc_4538>;
	using NETLIB_NAME(9602) = NETLIB_NAME(74123_base)<desc_9602>;

	NETLIB_DEVICE_IMPL(74123, "TTL_74123", "")
	NETLIB_DEVICE_IMPL(74121, "TTL_74121", "")
	NETLIB_DEVICE_IMPL(4538,  "CD4538",    "")
	NETLIB_DEVICE_IMPL(9602,  "TTL_9602",  "")

} // namespace netlist::devices
