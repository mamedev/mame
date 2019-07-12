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

#define BODY_CONNECTED_TO_SOURCE    (1)

namespace netlist
{
namespace analog
{

	using constants = plib::constants<nl_double>;

	// -----------------------------------------------------------------------------
	// nld_FET - Base classes
	// -----------------------------------------------------------------------------

	/*! Class representing the nmos/pmos model paramers.
	 *
	 *  This is the model representation of the nmos model.
	 *
	 *  Netlist has an additional parameter caller CAPMOD:
	 *
	 *  CAPMOD=0: Capacitance model disabled
	 *  CAPMOD=2: Meyer capacitance model
	 *
	 *  Typically, SPICE uses the following parameters. A "Y" in the first
	 *  column indicates that the parameter is actually used in netlist.
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
	 * |  Y  |Cgso  | Gate-source overlap capacitance per meter channel width               |F/m|0|0.00000000004|
	 * |  Y  |Cgdo  | Gate-drain overlap capacitance per meter channel width                |F/m|0|0.00000000004|*
	 * |  Y  |Cgbo  | Gate-bulk overlap capacitance per meter channel width                 |F/m|0|0.0000000002|*
	 * |     |Rsh   | Drain and source diffusion sheet resistance                           |W|0|10|*
	 * |     |Cj    | Zero-bias bulk junction bottom capacitance per square meter of junction area|F/m²|0|0.0002|*
	 * |     |Mj    | Bulk junction bottom grading coefficient                              |-|0.5|0.5|*
	 * |     |Cjsw  | Zero-bias bulk junction sidewall capacitance per meter of junction perimeter|F/m|0|1p|*
	 * |     |Mjsw  | Bulk junction sidewall grading coefficient                            |-|.50 level 1  .33 level 2,3||
	 * |     |Js    | Bulk junction saturation current per square-meter of junction area|A/m|0|0.00000001|
	 * |  Y  |Tox   | Oxide thickness                                                       |m|0.0000001|0.0000001|
	 * |  Y  |Nsub  | Substrate doping                                                      |1/cm³|0|4000000000000000|
	 * |     |Nss   | Surface state density                                                 |1/cm²|0|10000000000|
	 * |     |Nfs   | Fast surface state                                                    |1/cm²|0|10000000000|*
	 * |     |TPG   | Type of gate material:  +1 opp. to substrate -1 same as substrate     0 Al gate|-|1|
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
	 * |  Y  |L     | Length scaling                                                        |-|100e-6||
	 * |  Y  |W     | Width scaling                                                         |-|100e-6||
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
		, m_CGSO(*this, "CGSO")
		, m_CGDO(*this, "CGDO")
		, m_CGBO(*this, "CGBO")
		, m_CAPMOD(*this, "CAPMOD")
		{}

		value_t m_VTO;      //!< Threshold voltage [V]
		value_t m_N;        //!< Bulk diode emission coefficient
		value_t m_ISS;      //!< Body diode saturation current
		value_t m_ISD;      //!< Body diode saturation current
		value_t m_LD;       //!< Lateral diffusion [m]
		value_t m_L;        //!< Length scaling
		value_t m_W;        //!< Width scaling
		value_t m_TOX;      //!< Oxide thickness
		value_t m_KP;       //!< Transconductance parameter [A/V²]
		value_t m_UO;       //!< Surface mobility [cm²/V/s]
		value_t m_PHI;      //!< Surface inversion potential [V]
		value_t m_NSUB;     //!< Substrate doping [1/cm³]
		value_t m_GAMMA;    //!< Bulk threshold parameter [V^½]
		value_t m_LAMBDA;   //!< Channel-length modulation [1/V]
		value_t m_RD;       //!< Drain ohmic resistance
		value_t m_RS;       //!< Source ohmic resistance
		value_t m_CGSO;     //!< Gate-source overlap capacitance per meter channel width
		value_t m_CGDO;     //!< Gate-drain overlap capacitance per meter channel width
		value_t m_CGBO;     //!< Gate-bulk overlap capacitance per meter channel width
		value_base_t<int> m_CAPMOD; //!< Capacitance model (0=no model 2=Meyer)
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
		, m_cap_gb(*this, "m_cap_gb")
		, m_cap_gs(*this, "m_cap_gs")
		, m_cap_gd(*this, "m_cap_gd")
		, m_phi(0.0)
		, m_gamma(0.0)
		, m_vto(0.0)
		, m_beta(0.0)
		, m_lambda(0.0)
		, m_Leff(0.0)
		, m_CoxWL(0.0)
		, m_polarity(qtype() == FET_NMOS ? 1.0 : -1.0)
		, m_Cgb(0.0)
		, m_Cgs(0.0)
		, m_Cgd(0.0)
		, m_capmod(2)
		, m_Vgs(*this, "m_Vgs", 0.0)
		, m_Vgd(*this, "m_Vgd", 0.0)
	{
			register_subalias("S", m_SG.m_P);   // Source
			register_subalias("G", m_SG.m_N);   // Gate

			register_subalias("D", m_DG.m_P);   // Drain

			connect(m_SG.m_P, m_SD.m_P);
			connect(m_SG.m_N, m_DG.m_N);
			connect(m_DG.m_P, m_SD.m_N);

			set_qtype((m_model.type() == "NMOS_DEFAULT") ? FET_NMOS : FET_PMOS);
			m_polarity = qtype() == FET_NMOS ? 1.0 : -1.0;

			m_capmod = m_model.m_CAPMOD;
			// printf("capmod %d %g %g\n", m_capmod, (double)m_model.m_VTO, m_polarity);
			nl_assert_always(m_capmod == 0 || m_capmod == 2, "Error: CAPMODEL invalid value for " + m_model.name());

			/*
			 * From http://ltwiki.org/LTspiceHelp/LTspiceHelp/M_MOSFET.htm :
			 *
			 *      VTO, KP, LAMBDA, PHI and GAMMA. These parameters are computed
			 *      if the process parameters(NSUB, TOX,...) are given, but
			 *      user-specified values always override.
			 *
			 *  But couldn't find a formula for lambda anywhere
			 *
			 */

			m_lambda = m_model.m_LAMBDA; // FIXME: m_lambda only set once

			// calculate effective channel length
			m_Leff = m_model.m_L - 2 * m_model.m_LD;
			nl_assert_always(m_Leff > 0.0, "Effective Lateral diffusion would be negative for model " + m_model.name());

			nl_double Cox = (m_model.m_TOX > 0.0) ? (constants::eps_SiO2() * constants::eps_0() / m_model.m_TOX) : 0.0;

			// calculate DC transconductance coefficient
			if (m_model.m_KP > 0)
				m_beta = m_model.m_KP * m_model.m_W / m_Leff;
			else if (Cox > 0 && m_model.m_UO > 0)
				m_beta = m_model.m_UO * 1e-4 * Cox * m_model.m_W / m_Leff;
			else
				m_beta = 2e-5 * m_model.m_W / m_Leff;

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
				if (Cox > 0.0 && m_model.m_NSUB > 0)
					m_gamma = std::sqrt (2.0 * constants::Q_e() * constants::eps_Si() * constants::eps_0() * m_model.m_NSUB * 1e6) / Cox;
				else
					m_gamma = 0.0;
			}

			m_vto = m_model.m_VTO;
			if(m_vto != 0.0)
				log().warning(MW_MOSFET_THRESHOLD_VOLTAGE(m_model.name()));

			/* FIXME: VTO if missing may be calculated from TPG, NSS and temperature. Usually models
			 * specify VTO so skip this here.
			 */

			m_CoxWL = Cox * m_model.m_W * m_Leff;

			//printf("Cox: %g\n", m_Cox);
		}

		NETLIB_IS_TIMESTEP(true || m_capmod != 0)

		NETLIB_TIMESTEPI()
		{
			if (m_capmod != 0)
			{
				//const nl_double Ugd = -m_DG.deltaV() * m_polarity; // Gate - Drain
				//const nl_double Ugs = -m_SG.deltaV() * m_polarity; // Gate - Source
				const nl_double Ugd = m_Vgd; // Gate - Drain
				const nl_double Ugs = m_Vgs; // Gate - Source
				const nl_double Ubs = 0.0;                         // Bulk - Source == 0 if connected
				const nl_double Ugb = Ugs - Ubs;

				m_cap_gb.timestep(m_Cgb, Ugb, step);
				m_cap_gs.timestep(m_Cgs, Ugs, step);
				m_cap_gd.timestep(m_Cgd, Ugd, step);
			}
		}

	protected:

		NETLIB_RESETI()
		{
			NETLIB_NAME(FET)::reset();
			// Bulk diodes

			m_D_BD.set_param(m_model.m_ISD, m_model.m_N, exec().gmin(), constants::T0());
			#if (!BODY_CONNECTED_TO_SOURCE)
				m_D_BS.set_param(m_model.m_ISS, m_model.m_N, exec().gmin(), constants::T0());
			#endif
		}

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

		generic_capacitor<capacitor_e::VARIABLE_CAPACITY> m_cap_gb;
		generic_capacitor<capacitor_e::VARIABLE_CAPACITY> m_cap_gs;
		generic_capacitor<capacitor_e::VARIABLE_CAPACITY> m_cap_gd;

		nl_double m_phi;
		nl_double m_gamma;
		nl_double m_vto;
		nl_double m_beta;
		nl_double m_lambda;

		/* used in capacitance calculation */
		nl_double m_Leff;
		nl_double m_CoxWL;
		nl_double m_polarity;

		/* capacitance values */

		nl_double m_Cgb;
		nl_double m_Cgs;
		nl_double m_Cgd;

		int m_capmod;
		state_var<nl_double> m_Vgs;
		state_var<nl_double> m_Vgd;

		void set_cap(generic_capacitor<capacitor_e::VARIABLE_CAPACITY> cap,
			nl_double capval, nl_double V,
			nl_double &g11, nl_double &g12, nl_double &g21, nl_double &g22,
			nl_double &I1, nl_double &I2)
		{
			const nl_double I = cap.Ieq(capval, V) * m_polarity;
			const nl_double G = cap.G(capval);
			g11 += G; g12 -= G; g21 -= G; g22 += G;
			I1 -= I; I2 += I;
			//printf("Cap: %g\n", capval);
		}

		void calculate_caps(nl_double Vgs, nl_double Vgd, nl_double Vth,
			nl_double &Cgs, nl_double &Cgd, nl_double &Cgb)
		{
			nl_double Vctrl = Vgs - Vth * m_polarity;
			// Cut off - now further differentiated into 3 different formulas
			// Accumulation
			if (Vctrl <= -m_phi)
			{
				Cgb = m_CoxWL;
				Cgs = 0.0;
				Cgd = 0.0;
			}
			else if (Vctrl <= -m_phi / 2.0)
			{
				Cgb = -Vctrl * m_CoxWL / m_phi;
				Cgs = 0.0;
				Cgd = 0.0;
			}
			// Depletion
			else if (Vctrl <= 0)
			{
				Cgb = -Vctrl * m_CoxWL / m_phi;
				Cgs = Vctrl * m_CoxWL * (4.0 / 3.0) / m_phi + (2.0 / 3.0) * m_CoxWL;
				Cgd = 0.0;
			}
			else
			{
				const nl_double Vdsat = Vctrl;
				const nl_double Vds = Vgs - Vgd;
				// saturation
				if (Vdsat <= Vds)
				{
					Cgb = 0;
					Cgs = (2.0 / 3.0) * m_CoxWL;
					Cgd = 0;
				}
				else
				{
					// linear
					const nl_double Sqr1 = std::pow(Vdsat - Vds, 2);
					const nl_double Sqr2 = std::pow(2.0 * Vdsat - Vds, 2);
					Cgb = 0;
					Cgs = m_CoxWL * (1.0 - Sqr1 / Sqr2) * (2.0 / 3.0);
					Cgd = m_CoxWL * (1.0 - Vdsat * Vdsat / Sqr2) * (2.0 / 3.0);
				}
			}
		}
	};

	// ----------------------------------------------------------------------------------------
	// MOSFET
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

	NETLIB_UPDATE_TERMINALS(MOSFET)
	{
		nl_double Vgd = -m_DG.deltaV() * m_polarity; // Gate - Drain
		nl_double Vgs = -m_SG.deltaV() * m_polarity; // Gate - Source

		// limit step sizes

		const nl_double k = 3.5; // see "Circuit Simulation", page 185
		nl_double d = (Vgs - m_Vgs);
		Vgs = m_Vgs + 1.0/k * (d < 0 ? -1.0 : 1.0) * std::log1p(k * std::abs(d));
		d = (Vgd - m_Vgd);
		Vgd = m_Vgd + 1.0/k * (d < 0 ? -1.0 : 1.0) * std::log1p(k * std::abs(d));

		m_Vgs = Vgs;
		m_Vgd = Vgd;

		const nl_double Vbs = 0.0;                       // Bulk - Source == 0 if connected
		//const nl_double Vbd = m_SD.deltaV() * m_polarity;  // Bulk - Drain = Source  - Drain
		const nl_double Vds = Vgs - Vgd;
		const nl_double Vbd = -Vds;  // Bulk - Drain = Source  - Drain

#if (!BODY_CONNECTED_TO_SOURCE)
		m_D_BS.update_diode(Vbs);
#endif
		m_D_BD.update_diode(Vbd);

		// Are we in forward mode ?
		// in backward mode, just swap source and drain
		const bool is_forward = Vds >= 0;

		// calculate Vth
		const nl_double Vbulk = is_forward ? Vbs : Vbd;
		const nl_double phi_m_Vbulk = (m_phi > Vbulk) ? std::sqrt(m_phi - Vbulk) : 0.0;
		const nl_double Vth = m_vto * m_polarity + m_gamma * (phi_m_Vbulk - std::sqrt(m_phi));

		const nl_double Vctrl = (is_forward ? Vgs : Vgd) - Vth;

		nl_double Ids, gm, gds, gmb;
		const nl_double absVds = std::abs(Vds);

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
			const nl_double beta = m_beta * (1.0 + m_lambda * absVds);
			if (Vctrl <= absVds)
			{
				// saturation region
				Ids = beta * Vctrl * Vctrl / 2.0;
				gm  = beta * Vctrl;
				gds = m_lambda * m_beta * Vctrl * Vctrl / 2.0;
			}
			else
			{
				// linear region
				Ids = beta * absVds * (Vctrl - absVds / 2);
				gm  = beta * absVds;
				gds = beta * (Vctrl - absVds) + m_lambda * m_beta * absVds * (Vctrl - absVds / 2.0);
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

#if (!BODY_CONNECTED_TO_SOURCE)
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
			   Ids - gm * Vgs - gmb * Vbs - gds * Vds
			: -Ids - gm * Vgd - gmb * Vbd - gds * Vds;

		// IG = 0
		nl_double IG = 0.0;
		nl_double ID = (+IeqBD - IeqDS) * m_polarity;
		nl_double IS = (+IeqBS + IeqDS) * m_polarity;
		nl_double IB = (-IeqBD - IeqBS) * m_polarity;

		nl_double gGG = 0.0;
		nl_double gGD = 0.0;
		nl_double gGS = 0.0;
		nl_double gGB = 0.0;

		nl_double gDG =  gm;
		nl_double gDD =  gds + gbd - gdrain;
		const nl_double gDS = -gds - gsource;
		const nl_double gDB =  gmb - gbd;

		nl_double gSG = -gm;
		const nl_double gSD = -gds + gdrain;
		nl_double gSS =  gbs + gds + gsource;
		const nl_double gSB = -gbs - gmb;

		nl_double gBG =  0.0;
		const nl_double gBD = -gbd;
		const nl_double gBS = -gbs;
		nl_double gBB =  gbs + gbd;

		if (m_capmod != 0)
		{
			const nl_double Vgb = Vgs - Vbs;

			if (is_forward)
				calculate_caps(Vgs, Vgd, Vth, m_Cgs, m_Cgd, m_Cgb);
			else
				calculate_caps(Vgd, Vgs, Vth, m_Cgd, m_Cgs, m_Cgb);

			set_cap(m_cap_gb, m_Cgb + m_model.m_CGBO * m_Leff, Vgb, gGG, gGB, gBG, gBB, IG, IB);
			set_cap(m_cap_gs, m_Cgs + m_model.m_CGSO * m_model.m_W, Vgs, gGG, gGS, gSG, gSS, IG, IS);
			set_cap(m_cap_gd, m_Cgd + m_model.m_CGDO * m_model.m_W, Vgd, gGG, gGD, gDG, gDD, IG, ID);
		}

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
	}

} // namespace analog

namespace devices {
	NETLIB_DEVICE_IMPL_NS(analog, MOSFET, "MOSFET", "MODEL")
} // namespace devices

} // namespace netlist
