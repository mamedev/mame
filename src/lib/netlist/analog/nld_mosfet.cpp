// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_mosfet.cpp
 *
 * Formulas in here based on the following Sources:
 *
 * https://www.imperial.ac.uk/pls/portallive/docs/1/7292573.PDF
 * http://www3.imperial.ac.uk/pls/portallive/docs/1/56133736.PDF
 * https://people.rit.edu/lffeee/SPICE_MOSFET_Model_Intro.pdf
 * https://people.rit.edu/lffeee/SPICE.pdf
 * http://web.mit.edu/course/6/6.012/SPR98/www/lectures/S98_Lecture10.pdf
 * http://homepages.rpi.edu/~sawyes/Models_review.pdf
 * http://jaco.ec.t.kanazawa-u.ac.jp/edu/mix/pdf/3.pdf
 *
 * Farid N. Naim, Circuit Simulation (Wiley-IEEE Press, 2010).
 * Stefan Jahn, Michael Margraf, Vincent Habchi and Raimund Jacob, "Qucs Technical Papers" (2007)
 *
 */

#include "netlist/solver/nld_solver.h"
#include "netlist/nl_setup.h"
#include "nlid_twoterm.h"

#include <cmath>

#define BODY_CONNECTED_TO_SOURCE	(1)

namespace netlist
{
namespace analog
{

	using constants = plib::constants<nl_double>;

	// -----------------------------------------------------------------------------
	// nld_FET - Base classes
	// -----------------------------------------------------------------------------

	/*! Class representing the nmos model paramers.
	 *
	 *  This is the model representation of the nmos model. Typically, SPICE uses
	 *  the following parameters. A "Y" in the first column indicates that the
	 *  parameter is actually used in netlist.
	 *
	 * | NL? |Name  |                                                            Description|Units  |Default   |Example          |
	 * |:---:|------|-----------------------------------------------------------------------|-------|---------:|----------------:|
	 * |  Y  |Vto   | Zero-bias threshold voltage                                           | V     | 0        | 1               |
	 * |  Y  |Kp    | Transconductance parameter                                            | A/V²  | 0.00002  | 0.00003         |
	 * |  Y  |Gamma | Bulk threshold parameter                                              | V^½   | 0        | 0.37            |
	 * |  Y  |Phi   | Surface inversion potential                                           | V     | 0.6      | 0.65            |
	 * |  Y  |Lambda| Channel-length modulation (level 1 and 2 only)                        | 1/V   | 0        | 0.02            |
	 * |     |Rd    | Drain ohmic resistance                                                |W|0|1|
	 * |     |Rs    | Source ohmic resistance                                               |W|0|1|
	 * |     |Cbd   | Zero-bias B-D junction capacitance                                    |F|0|20f|
	 * |     |Cbs   | Zero-bias B-S junction capacitance                                    |F|0|20f|
	 * |  Y  |Is    | Bulk junction saturation current                                      |A|0.00000000000001|1E-015|
	 * |  Y  |N     | Bulk diode emission coefficient                                       |-|1|*
	 * |     |Pb    | Bulk junction potential                                               |V|0.8|0.87|8|
	 * |     |Cgso  | Gate-source overlap capacitance per meter channel width               |F/m|0|0.00000000004|
	 * |     |Cgdo  | Gate-drain overlap capacitance per meter channel width                |F/m|0|0.00000000004|*
	 * |     |Cgbo  | Gate-bulk overlap capacitance per meter channel width                 |F/m|0|0.0000000002|*
	 * |     |Rsh   | Drain and source diffusion sheet resistance                           |W|0|10|*
	 * |     |Cj    | Zero-bias bulk junction bottom capacitance per square meter of junction area|F/m²|0|0.0002|*
	 * |     |Mj    | Bulk junction bottom grading coefficient                              |-|0.5|0.5|*
	 * |     |Cjsw  | Zero-bias bulk junction sidewall capacitance per meter of junction perimeter|F/m|0|1p|*
	 * |     |Mjsw  | Bulk junction sidewall grading coefficient                            |-|.50 level 1 	.33 level 2,3||
	 * |     |Js    | Bulk junction saturation current per square-meter of junction area|A/m|0|0.00000001|
	 * |  Y  |Tox   | Oxide thickness                                                       |m|0.0000001|0.0000001|
	 * |  Y  |Nsub  | Substrate doping                                                      |1/cm³|0|4000000000000000|
	 * |     |Nss   | Surface state density                                                 |1/cm²|0|10000000000|
	 * |     |Nfs   | Fast surface state                                                    |1/cm²|0|10000000000|*
	 * |     |TPG   | Type of gate material:  +1 opp. to substrate -1 same as substrate 	0 Al gate|-|1|
	 * |     |Xj    | Metallurgical junction depth                                          |m|0|1µ|*
	 * |  Y  |Ld    | Lateral diffusion                                                     |m|0|0.8µ|
	 * |  Y  |Uo    | Surface mobility                                                      |cm²/V/s|600|700|
	 * |     |Ucrit | Critical field for mobility degradation (level 2 only)                |V/cm|10000|10000|
	 * |     |Uexp  | Critical field exponent in mobility degradation (level 2 only)        |-|0|0.1|
	 * |     |Utra  | Transverse field coefficient (level 2 only)                           |-|0|0.3|*
	 * |     |Vmax  | Maximum carrier drift velocity (levels 2 & 3 only)                    |m/s|0|50000|
	 * |     |Neff  | Total channel-charge exponent (level 2 only)                          |-|1|5|
	 * |     |Kf    | Flicker noise coefficient                                             |-|0|1E-026|
	 * |     |Af    | Flicker noise exponent                                                |-|1|1.2|
	 * |     |Fc    | Coefficient for forward-bias depletion capacitance formula            |-|0.5|
	 * |     |Delta | Width effect on threshold voltage(levels 2 and 3)                     |-|0|1|
	 * |     |Theta | Mobility modulation (level 3 only)                                    |-|0|0.1|
	 * |     |Eta   | Static feedback (level 3 only)                                        |-|0|1|
	 * |     |Kappa | Saturation field (level 3 only)                                       |0.2|0.5|
	 * |     |Tnom  | Parameter measurement temperature                                     |ºC|27|50||
	 * |  Y  |L     | Length scaling                                                        |-|1.0||
	 * |  Y  |W     | Width scaling                                                         |-|1.0||
	 * */

	class fet_model_t : public param_model_t
	{
	public:
		fet_model_t(device_t &device, const pstring &name, const pstring &val)
		: param_model_t(device, name, val)
		, m_VTO(*this,  "VTO")
		, m_N(*this,  "N")
		, m_ISS(*this,  "IS")  // Haven't seen a model using ISS / ISD
		, m_ISD(*this,  "IS")
		, m_LD(*this,  "LD")
		, m_L(*this, "L")
		, m_W(*this, "W")
		, m_TOX(*this, "TOX")
		, m_KP(*this, "KP")
		, m_UO(*this, "UO")
		, m_PHI(*this, "PHI")
		, m_NSUB(*this, "NSUB")
		, m_GAMMA(*this, "GAMMA")
		, m_LAMBDA(*this, "LAMBDA")
		, m_RD(*this, "RD")
		, m_RS(*this, "RS")
		{}

		value_t m_VTO; //!< Threshold voltage [V]
		value_t m_N;   //!< Bulk diode emission coefficient
		value_t m_ISS;  //!< Body diode saturation current
		value_t m_ISD;  //!< Body diode saturation current
		value_t m_LD;  //!< Lateral diffusion [m]
		value_t m_L;   //!< Length scaling
		value_t m_W;   //!< Width scaling
		value_t m_TOX; //!< Oxide thickness
		value_t m_KP;  //!< Transconductance parameter [A/V²]
		value_t m_UO;  //!< Surface mobility [cm²/V/s]
		value_t m_PHI; //!< Surface inversion potential [V]
		value_t m_NSUB;//!< Substrate doping [1/cm³]
		value_t m_GAMMA; //!< Bulk threshold parameter [V^½]
		value_t m_LAMBDA; //!< Channel-length modulation [1/V]
		value_t m_RD;  //!< Drain ohmic resistance
		value_t m_RS;  //!< Source ohmic resistance
	};

	// Have a common start for mosfets

	NETLIB_OBJECT(FET)
	{
	public:
		enum q_type {
			FET_NMOS,
			FET_PMOS
		};

		NETLIB_CONSTRUCTOR(FET)
		, m_model(*this, "MODEL", "NMOS")
		, m_qtype(FET_NMOS)
		{
		}

		NETLIB_IS_DYNAMIC(true)

		//NETLIB_RESETI();
		NETLIB_UPDATEI() { }

		q_type qtype() const { return m_qtype; }
		bool is_qtype(q_type atype) const { return m_qtype == atype; }
		void set_qtype(q_type atype) { m_qtype = atype; }
	protected:

		fet_model_t m_model;
	private:
		q_type m_qtype;
	};

	// -----------------------------------------------------------------------------
	// nld_QBJT_EB
	// -----------------------------------------------------------------------------


	NETLIB_OBJECT_DERIVED(MOSFET, FET)
	{
	public:
		NETLIB_CONSTRUCTOR_DERIVED(MOSFET, FET)
		, m_DG(*this, "m_DG", true)
		, m_SG(*this, "m_SG", true)
		, m_SD(*this, "m_SD", true)
		, m_D_BD(*this, "m_D_BD")
#if (!BODY_CONNECTED_TO_SOURCE)
		, m_D_BS(*this, "m_D_BS")
#endif
		, m_phi(0.0)
		, m_gamma(0.0)
		, m_vto(0.0)
		, m_beta(0.0)
		, m_lambda(0.0)
		, m_Leff(0.0)
		, m_Cox(0.0)
	{
			register_subalias("S", m_SG.m_P);   // Source
			register_subalias("G", m_SG.m_N);   // Gate

			register_subalias("D", m_DG.m_P);   // Drain

			connect(m_SG.m_P, m_SD.m_P);
			connect(m_SG.m_N, m_DG.m_N);
			connect(m_DG.m_P, m_SD.m_N);

#if 0
			if (m_model.m_CJE > 0.0)
			{
				create_and_register_subdevice("m_CJE", m_CJE);
				connect("B", "m_CJE.1");
				connect("E", "m_CJE.2");
			}
			if (m_model.m_CJC > 0.0)
			{
				create_and_register_subdevice("m_CJC", m_CJC);
				connect("B", "m_CJC.1");
				connect("C", "m_CJC.2");
			}
#endif
		}

	protected:

		NETLIB_RESETI();
		NETLIB_UPDATEI();
		NETLIB_UPDATE_PARAMI();
		NETLIB_UPDATE_TERMINALSI();

	private:

		nld_twoterm m_DG;
		nld_twoterm m_SG;
		nld_twoterm m_SD;

		generic_diode<diode_e::MOS> m_D_BD;
#if (!BODY_CONNECTED_TO_SOURCE)
		generic_diode<diode_e::MOS> m_D_BS;
#endif

		nl_double m_phi;
		nl_double m_gamma;
		nl_double m_vto;
		nl_double m_beta;
		nl_double m_lambda;

		/* used in capacitance calculation */
		nl_double m_Leff;
		nl_double m_Cox;

		//NETLIB_SUBXX(analog, C) m_CJE;
		//NETLIB_SUBXX(analog, C) m_CJC;
	};



	// ----------------------------------------------------------------------------------------
	// nld_Q - Ebers Moll
	// ----------------------------------------------------------------------------------------


	NETLIB_UPDATE(MOSFET)
	{
		if (!m_SG.m_P.net().isRailNet())
			m_SG.m_P.solve_now();   // Basis
		else if (!m_SG.m_N.net().isRailNet())
			m_SG.m_N.solve_now();   // Emitter
		else
			m_DG.m_N.solve_now();   // Collector
	}

	NETLIB_RESET(MOSFET)
	{
		NETLIB_NAME(FET)::reset();
#if 0
		if (m_CJE)
		{
			m_CJE->reset();
			m_CJE->m_C.setTo(m_model.m_CJE);
		}
		if (m_CJC)
		{
			m_CJC->reset();
			m_CJC->m_C.setTo(m_model.m_CJC);
		}
#endif
	}

	NETLIB_UPDATE_TERMINALS(MOSFET)
	{
		const nl_double polarity = (qtype() == FET_NMOS ? 1.0 : -1.0);

		const nl_double Ugd = -m_DG.deltaV() * polarity; // Gate - Drain
		const nl_double Ugs = -m_SG.deltaV() * polarity; // Gate - Source
		const nl_double Ubs = 0.0;                       // Bulk - Source == 0 if connected
		const nl_double Ubd = m_SD.deltaV() * polarity;  // Bulk - Drain = Source  - Drain
		const nl_double Uds = Ugs - Ugd;

#if (!BODY_CONNECTED_TO_SOURCE)
		m_D_BS.update_diode(Ubs);
#endif
		m_D_BD.update_diode(Ubd);

		// Are we in forward mode ?
		const bool is_forward = Uds >= 0;

		// calculate Vth
		const nl_double Vbulk = is_forward ? Ubs : Ubd;
		const nl_double phi_m_Vbulk = (m_phi > Vbulk) ? std::sqrt(m_phi - Vbulk) : 0.0;
		const nl_double Vth = m_vto * polarity + m_gamma * (phi_m_Vbulk - std::sqrt(m_phi));

		const nl_double Vctrl = (is_forward ? Ugs : Ugd) - Vth;

		nl_double Ids, gm, gds, gmb;

		if (Vctrl <= 0.0)
		{
			// cutoff region
		    Ids = 0.0;
		    gm  = 0.0;
		    gds = 0.0;
		    gmb = 0.0;
		}
		else
		{
			const nl_double Vds = std::abs(Uds);
		    const nl_double b   = m_beta * (1.0 + m_lambda * Vds);
		    if (Vctrl <= Vds)
		    {
			    // saturation region
		    	Ids = b * Vctrl * Vctrl / 2.0;
		    	gm  = b * Vctrl;
		    	gds = m_lambda * m_beta * Vctrl * Vctrl / 2.0;
		    }
		    else
		    {
			    // linear region
		    	Ids = b * Vds * (Vctrl - Vds / 2);
		    	gm  = b * Vds;
		    	gds = b * (Vctrl - Vds) + m_lambda * m_beta * Vds * (Vctrl - Vds / 2.0);
		    }

			// backgate transconductance
			const nl_double bgtc = (phi_m_Vbulk != 0.0) ? (m_gamma / phi_m_Vbulk / 2.0) : 0.0;
		    gmb = gm * bgtc;
		}

		// FIXME: these are needed to compute capacitance
		// nl_double Udsat = pol * std::max (Utst, 0.0);
		// Uon = pol * Vth;

		// compute bulk diode equivalent currents

		const nl_double IeqBD = m_D_BD.Ieq();
		const nl_double gbd = m_D_BD.G();
#if 0
		const nl_double IeqBS = m_D_BS.Ieq();
		const nl_double gbs = m_D_BS.G();
#else
		const nl_double IeqBS = 0.0;
		const nl_double gbs = 0.0;
#endif
		// exchange controlling nodes if necessary
		const nl_double gsource = is_forward ? (gm + gmb) : 0;
		const nl_double gdrain  = is_forward ?   0.0 : (gm + gmb);

		const nl_double IeqDS = (is_forward) ?
			   Ids - gm * Ugs - gmb * Ubs - gds * Uds
			: -Ids - gm * Ugd - gmb * Ubd - gds * Uds;

		// IG = 0
		const nl_double IG = 0.0;
		const nl_double ID = (+IeqBD - IeqDS) * polarity;
		const nl_double IS = (+IeqBS + IeqDS) * polarity;
		const nl_double IB = (-IeqBD - IeqBS) * polarity;

		const nl_double gGG = 0.0; // ok
		const nl_double gGD = 0.0; // ok
		const nl_double gGS = 0.0; // ok
		const nl_double gGB = 0.0; // ok

		const nl_double gDG =  gm; // ok
		const nl_double gDD =  gds + gbd - gdrain; // ok
		const nl_double gDS = -gds - gsource; // ok
		const nl_double gDB =  gmb - gbd; // ok

		const nl_double gSG = -gm; // ok
		const nl_double gSD = -gds + gdrain; // ok
		const nl_double gSS =  gbs + gds + gsource;  // ok
		const nl_double gSB = -gbs - gmb;

		const nl_double gBG =  0.0; // ok
		const nl_double gBD = -gbd; // ok
		const nl_double gBS = -gbs;
		const nl_double gBB =  gbs + gbd; // ok

		// Source connected to body, Diode S-B shorted!
		const nl_double gSSBB = gSS + gBB + gBS + gSB;

		//                 S          G
		m_SG.set_mat(    gSSBB,   gSG + gBG, +(IS + IB),       // S
					   gGS + gGB,    gGG,      IG       );     // G
		//                 D          G
		m_DG.set_mat(     gDD,       gDG,    +ID,              // D
					      gGD,       0.0,    0.0        );     // G
		//                 S          D
		m_SD.set_mat(     0.0,    gSD + gBD,  0.0,             // S
					   gDS + gDB,    0.0,     0.0);            // D

	}


	NETLIB_UPDATE_PARAM(MOSFET)
	{
		set_qtype((m_model.model_type() == "NMOS") ? FET_NMOS : FET_PMOS);

		/*
		 * From http://ltwiki.org/LTspiceHelp/LTspiceHelp/M_MOSFET.htm :
		 *
		 *		VTO, KP, LAMBDA, PHI and GAMMA. These parameters are computed
		 *		if the process parameters(NSUB, TOX,...) are given, but
		 *		user-specified values always override.
		 *
		 *	But couldn't find a formula for lambda anywhere
		 *
		 */

		m_lambda = m_model.m_LAMBDA; // FIXME: m_lambda only set once

		// calculate effective channel length
		m_Leff = m_model.m_L - 2 * m_model.m_LD;
		nl_assert_always(m_Leff > 0.0, "Effective Lateral diffusion would be negative for model " + m_model.name());
		if (m_model.m_TOX > 0.0)
			m_Cox = (constants::eps_SiO2() * constants::eps_0() / m_model.m_TOX);
		else
			m_Cox = 0.0;

		// calculate DC transconductance coefficient
		if (m_model.m_KP > 0)
			m_beta = m_model.m_KP * m_model.m_W / m_Leff;
		else if (m_Cox > 0 && m_model.m_UO > 0)
			m_beta = m_model.m_UO * 1e-4 * m_Cox * m_model.m_W / m_Leff;
		else
			m_beta = 2e-5 * m_model.m_W / m_Leff;

		// Bulk diodes

		m_D_BD.set_param(m_model.m_ISD, m_model.m_N, exec().gmin(), constants::T0());
#if (!BODY_CONNECTED_TO_SOURCE)
		m_D_BS.set_param(m_model.m_ISS, m_model.m_N, exec().gmin(), constants::T0());
#endif

		//FIXME::UT can disappear
		const double Vt = constants::T0() * constants::k_b() / constants::Q_e();

		// calculate surface potential if not given

		if (m_model.m_PHI > 0.0)
			m_phi = m_model.m_PHI;
		else if (m_model.m_NSUB > 0.0)
		{
			nl_assert_always(m_model.m_NSUB * 1e6 >= constants::NiSi(), "Error calculating phi for model " + m_model.name());
			m_phi = 2 * Vt * std::log (m_model.m_NSUB * 1e6 / constants::NiSi());
		}
		else
			m_phi = 0.6;

		// calculate bulk threshold if not given
		if (m_model.m_GAMMA > 0.0)
			m_gamma = m_model.m_GAMMA;
		else
		{
			if (m_Cox > 0 && m_model.m_NSUB > 0)
				m_gamma = std::sqrt (2.0 * constants::Q_e() * constants::eps_Si() * constants::eps_0() * m_model.m_NSUB * 1e6) / m_Cox;
			else
				m_gamma = 0.0;
		}

		m_vto = m_model.m_VTO;
		nl_assert_always(m_vto != 0.0, "Threshold voltage not specified for " + m_model.name());

		/* FIXME: VTO if missing may be calculated from TPG, NSS and temperature. Usually models
		 * specify VTO so skip this here.
		 */

		m_Cox = m_Cox * m_model.m_W * m_Leff;

	}

} // namespace analog

namespace devices {
	NETLIB_DEVICE_IMPL_NS(analog, MOSFET, "MOSFET", "MODEL")
} // namespace devices

} // namespace netlist
