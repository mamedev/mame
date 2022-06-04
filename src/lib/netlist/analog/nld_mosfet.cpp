// license:BSD-3-Clause
// copyright-holders:Couriersud

// Names
// spell-checker: words Farid, Naim, Jahn, Margraf, Habchi, Raimund, Qucs
//
// Specific technical terms
// spell-checker: words Cgso, Cgdo, Cgbo, Cjsw, Mjsw, Ucrit, Uexp, Utra, Neff, Tnom, capval, Udsat, Utst

///
/// \file nld_mosfet.cpp
///
/// Formulas in here based on the following Sources:
///
/// https:///www.imperial.ac.uk/pls/portallive/docs/1/7292573.PDF
/// http://www3.imperial.ac.uk/pls/portallive/docs/1/56133736.PDF
/// https://people.rit.edu/lffeee/SPICE_MOSFET_Model_Intro.pdf
/// https://people.rit.edu/lffeee/SPICE.pdf
/// http://web.mit.edu/course/6/6.012/SPR98/www/lectures/S98_Lecture10.pdf
/// http://homepages.rpi.edu/~sawyes/Models_review.pdf
/// http://jaco.ec.t.kanazawa-u.ac.jp/edu/mix/pdf/3.pdf
///
/// Farid N. Naim, Circuit Simulation (Wiley-IEEE Press, 2010).
/// Stefan Jahn, Michael Margraf, Vincent Habchi and Raimund Jacob, "Qucs Technical Papers" (2007)
///

#include "solver/nld_solver.h"
#include "../nl_setup.h"
#include "nlid_twoterm.h"

#define BODY_CONNECTED_TO_SOURCE    (1)

namespace netlist
{
namespace analog
{

	using constants = plib::constants<nl_fptype>;

	// -----------------------------------------------------------------------------
	// nld_FET - Base classes
	// -----------------------------------------------------------------------------

	/// \brief Class representing the nmos/pmos model parameters.
	///
	///  This is the model representation of the nmos model.
	///
	///  Netlist has an additional parameter caller CAPMOD:
	///
	///  CAPMOD=0: Capacitance model disabled
	///  CAPMOD=2: Meyer capacitance model
	///
	///  Typically, SPICE uses the following parameters. A "Y" in the first
	///  column indicates that the parameter is actually used in netlist.
	///
	/// | NL? |Name  |                                                            Description|Units  |Default   |Example          |
	/// |:---:|------|-----------------------------------------------------------------------|-------|---------:|----------------:|
	/// |  Y  |Vto   | Zero-bias threshold voltage                                           | V     | 0        | 1               |
	/// |  Y  |Kp    | Transconductance parameter                                            | A/V²  | 0.00002  | 0.00003         |
	/// |  Y  |Gamma | Bulk threshold parameter                                              | V^½   | 0        | 0.37            |
	/// |  Y  |Phi   | Surface inversion potential                                           | V     | 0.6      | 0.65            |
	/// |  Y  |Lambda| Channel-length modulation (level 1 and 2 only)                        | 1/V   | 0        | 0.02            |
	/// |     |Rd    | Drain ohmic resistance                                                |W|0|1|
	/// |     |Rs    | Source ohmic resistance                                               |W|0|1|
	/// |     |Cbd   | Zero-bias B-D junction capacitance                                    |F|0|20f|
	/// |     |Cbs   | Zero-bias B-S junction capacitance                                    |F|0|20f|
	/// |  Y  |Is    | Bulk junction saturation current                                      |A|0.00000000000001|1E-015|
	/// |  Y  |N     | Bulk diode emission coefficient                                       |-|1|*|
	/// |     |Pb    | Bulk junction potential                                               |V|0.8|0.87|
	/// |  Y  |Cgso  | Gate-source overlap capacitance per meter channel width               |F/m|0|0.00000000004|
	/// |  Y  |Cgdo  | Gate-drain overlap capacitance per meter channel width                |F/m|0|0.00000000004*|
	/// |  Y  |Cgbo  | Gate-bulk overlap capacitance per meter channel width                 |F/m|0|0.0000000002*|
	/// |     |Rsh   | Drain and source diffusion sheet resistance                           |W|0|10*|
	/// |     |Cj    | Zero-bias bulk junction bottom capacitance per square meter of junction area|F/m²|0|0.0002*|
	/// |     |Mj    | Bulk junction bottom grading coefficient                              |-|0.5|0.5*|
	/// |     |Cjsw  | Zero-bias bulk junction sidewall capacitance per meter of junction perimeter|F/m|0|1p*|
	/// |     |Mjsw  | Bulk junction sidewall grading coefficient                            |-|.50 level 1  .33 level 2,3||
	/// |     |Js    | Bulk junction saturation current per square-meter of junction area|A/m|0|0.00000001|
	/// |  Y  |Tox   | Oxide thickness                                                       |m|0.0000001|0.0000001|
	/// |  Y  |Nsub  | Substrate doping                                                      |1/cm³|0|4000000000000000|
	/// |     |Nss   | Surface state density                                                 |1/cm²|0|10000000000|
	/// |     |Nfs   | Fast surface state                                                    |1/cm²|0|10000000000*|
	/// |     |TPG   | Type of gate material:  +1 opp. to substrate -1 same as substrate     |Al gate|-|1|
	/// |     |Xj    | Metallurgical junction depth                                          |m|0|1µ*|
	/// |  Y  |Ld    | Lateral diffusion                                                     |m|0|0.8µ|
	/// |  Y  |Uo    | Surface mobility                                                      |cm²/V/s|600|700|
	/// |     |Ucrit | Critical field for mobility degradation (level 2 only)                |V/cm|10000|10000|
	/// |     |Uexp  | Critical field exponent in mobility degradation (level 2 only)        |-|0|0.1|
	/// |     |Utra  | Transverse field coefficient (level 2 only)                           |-|0|0.3*|
	/// |     |Vmax  | Maximum carrier drift velocity (levels 2 & 3 only)                    |m/s|0|50000|
	/// |     |Neff  | Total channel-charge exponent (level 2 only)                          |-|1|5|
	/// |     |Kf    | Flicker noise coefficient                                             |-|0|1E-026|
	/// |     |Af    | Flicker noise exponent                                                |-|1|1.2|
	/// |     |Fc    | Coefficient for forward-bias depletion capacitance formula            |-|0.5||
	/// |     |Delta | Width effect on threshold voltage(levels 2 and 3)                     |-|0|1|
	/// |     |Theta | Mobility modulation (level 3 only)                                    |-|0|0.1|
	/// |     |Eta   | Static feedback (level 3 only)                                        |-|0|1|
	/// |     |Kappa | Saturation field (level 3 only)                                       |-|0.2|0.5|
	/// |     |Tnom  | Parameter measurement temperature                                     |ºC|27|50|
	/// |  Y  |L     | Length scaling                                                        |-|100e-6||
	/// |  Y  |W     | Width scaling                                                         |-|100e-6||
	///

	class fet_model_t
	{
	public:
		fet_model_t(param_model_t &model)
		: m_VTO(model,  "VTO")
		, m_N(model,  "N")
		, m_ISS(model,  "IS")  // Haven't seen a model using ISS / ISD
		, m_ISD(model,  "IS")
		, m_LD(model,  "LD")
		, m_L(model, "L")
		, m_W(model, "W")
		, m_TOX(model, "TOX")
		, m_KP(model, "KP")
		, m_UO(model, "UO")
		, m_PHI(model, "PHI")
		, m_NSUB(model, "NSUB")
		, m_GAMMA(model, "GAMMA")
		, m_LAMBDA(model, "LAMBDA")
		, m_RD(model, "RD")
		, m_RS(model, "RS")
		, m_CGSO(model, "CGSO")
		, m_CGDO(model, "CGDO")
		, m_CGBO(model, "CGBO")
		, m_CAPMOD(model, "CAPMOD")
		{}

		param_model_t::value_t m_VTO;      //!< Threshold voltage [V]
		param_model_t::value_t m_N;        //!< Bulk diode emission coefficient
		param_model_t::value_t m_ISS;      //!< Body diode saturation current
		param_model_t::value_t m_ISD;      //!< Body diode saturation current
		param_model_t::value_t m_LD;       //!< Lateral diffusion [m]
		param_model_t::value_t m_L;        //!< Length scaling
		param_model_t::value_t m_W;        //!< Width scaling
		param_model_t::value_t m_TOX;      //!< Oxide thickness
		param_model_t::value_t m_KP;       //!< Transconductance parameter [A/V²]
		param_model_t::value_t m_UO;       //!< Surface mobility [cm²/V/s]
		param_model_t::value_t m_PHI;      //!< Surface inversion potential [V]
		param_model_t::value_t m_NSUB;     //!< Substrate doping [1/cm³]
		param_model_t::value_t m_GAMMA;    //!< Bulk threshold parameter [V^½]
		param_model_t::value_t m_LAMBDA;   //!< Channel-length modulation [1/V]
		param_model_t::value_t m_RD;       //!< Drain ohmic resistance
		param_model_t::value_t m_RS;       //!< Source ohmic resistance
		param_model_t::value_t m_CGSO;     //!< Gate-source overlap capacitance per meter channel width
		param_model_t::value_t m_CGDO;     //!< Gate-drain overlap capacitance per meter channel width
		param_model_t::value_t m_CGBO;     //!< Gate-bulk overlap capacitance per meter channel width
		param_model_t::value_base_t<int> m_CAPMOD; //!< Capacitance model (0=no model 2=Meyer)
	};

	// Have a common start for mosfets

	NETLIB_BASE_OBJECT(FET)
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

		q_type qtype() const noexcept { return m_qtype; }
		bool is_qtype(q_type atype) const noexcept { return m_qtype == atype; }
		void set_qtype(q_type atype) noexcept { m_qtype = atype; }
	protected:

		param_model_t m_model;
	private:
		q_type m_qtype;
	};

	// -----------------------------------------------------------------------------
	// nld_QBJT_EB
	// -----------------------------------------------------------------------------

	NETLIB_OBJECT_DERIVED(MOSFET, FET)
	{
	public:
		NETLIB_CONSTRUCTOR(MOSFET)
		, m_DG(*this, "m_DG", NETLIB_DELEGATE(terminal_handler))
		, m_SG(*this, "m_SG", NETLIB_DELEGATE(terminal_handler))
		, m_SD(*this, "m_SD", NETLIB_DELEGATE(terminal_handler))
		, m_D_BD(*this, "m_D_BD")
#if (!BODY_CONNECTED_TO_SOURCE)
		, m_D_BS(*this, "m_D_BS")
#endif
		, m_cap_gb(*this, "m_cap_gb")
		, m_cap_gs(*this, "m_cap_gs")
		, m_cap_gd(*this, "m_cap_gd")
		, m_phi(nlconst::zero())
		, m_gamma(nlconst::zero())
		, m_vto(nlconst::zero())
		, m_beta(nlconst::zero())
		, m_lambda(nlconst::zero())
		, m_Leff(nlconst::zero())
		, m_CoxWL(nlconst::zero())
		, m_polarity(qtype() == FET_NMOS ? nlconst::one() : -nlconst::one())
		, m_Cgb(nlconst::zero())
		, m_Cgs(nlconst::zero())
		, m_Cgd(nlconst::zero())
		, m_capacitor_model(2)
		, m_Vgs(*this, "m_Vgs", nlconst::zero())
		, m_Vgd(*this, "m_Vgd", nlconst::zero())
		, m_model_acc(m_model)
	{
			register_sub_alias("S", m_SG.P());   // Source
			register_sub_alias("G", m_SG.N());   // Gate

			register_sub_alias("D", m_DG.P());   // Drain

			connect(m_SG.P(), m_SD.P());
			connect(m_SG.N(), m_DG.N());
			connect(m_DG.P(), m_SD.N());

			set_qtype((m_model.type() == "NMOS_DEFAULT") ? FET_NMOS : FET_PMOS);
			m_polarity = (qtype() == FET_NMOS ? nlconst::one() : -nlconst::one());

			m_capacitor_model = m_model_acc.m_CAPMOD;
			//# printf("capmod %d %g %g\n", m_capacitor_model, (nl_fptype)m_model_acc.m_VTO, m_polarity);
			nl_assert_always(m_capacitor_model == 0 || m_capacitor_model == 2, "Error: CAPMODEL invalid value");

			//
			// From http://ltwiki.org/LTspiceHelp/LTspiceHelp/M_MOSFET.htm :
			//
			//      VTO, KP, LAMBDA, PHI and GAMMA. These parameters are computed
			//      if the process parameters(NSUB, TOX,...) are given, but
			//      user-specified values always override.
			//
			//  But couldn't find a formula for lambda anywhere
			//

			m_lambda = m_model_acc.m_LAMBDA; // FIXME: m_lambda only set once

			// calculate effective channel length
			m_Leff = m_model_acc.m_L - 2 * m_model_acc.m_LD;
			nl_assert_always(m_Leff > nlconst::zero(), "Effective Lateral diffusion would be negative for model");

			nl_fptype Cox = (m_model_acc.m_TOX > nlconst::zero()) ? (constants::eps_SiO2() * constants::eps_0() / m_model_acc.m_TOX) : nlconst::zero();

			// calculate DC transconductance coefficient
			if (m_model_acc.m_KP > nlconst::zero())
				m_beta = m_model_acc.m_KP * m_model_acc.m_W / m_Leff;
			else if (Cox > nlconst::zero() && m_model_acc.m_UO > nlconst::zero())
				m_beta = m_model_acc.m_UO * nlconst::magic(1e-4) * Cox * m_model_acc.m_W / m_Leff;
			else
				m_beta = nlconst::magic(2e-5) * m_model_acc.m_W / m_Leff;

			//FIXME::UT can disappear
			const nl_fptype Vt = constants::T0() * constants::k_b() / constants::Q_e();

			// calculate surface potential if not given

			if (m_model_acc.m_PHI > nlconst::zero())
				m_phi = m_model_acc.m_PHI;
			else if (m_model_acc.m_NSUB > nlconst::zero())
			{
				nl_assert_always(m_model_acc.m_NSUB * nlconst::magic(1e6) >= constants::NiSi(), "Error calculating phi for model");
				m_phi = nlconst::two() * Vt * plib::log (m_model_acc.m_NSUB * nlconst::magic(1e6) / constants::NiSi());
			}
			else
				m_phi = nlconst::magic(0.6);

			// calculate bulk threshold if not given
			if (m_model_acc.m_GAMMA > nlconst::zero())
				m_gamma = m_model_acc.m_GAMMA;
			else
			{
				if (Cox > nlconst::zero() && m_model_acc.m_NSUB > nlconst::zero())
					m_gamma = plib::sqrt (nlconst::two()
						* constants::Q_e() * constants::eps_Si() * constants::eps_0()
						* m_model_acc.m_NSUB * nlconst::magic(1e6)) / Cox;
				else
					m_gamma = nlconst::zero();
			}

			m_vto = m_model_acc.m_VTO;
			// FIXME zero conversion
			if(m_vto == nlconst::zero())
				log().warning(MW_MOSFET_THRESHOLD_VOLTAGE(m_model.name()));

			// FIXME: VTO if missing may be calculated from TPG, NSS and temperature. Usually models
			// specify VTO so skip this here.

			m_CoxWL = Cox * m_model_acc.m_W * m_Leff;

			//#printf("Cox: %g\n", m_Cox);
		}

		NETLIB_IS_TIMESTEP(true || m_capacitor_model != 0)

		NETLIB_TIMESTEPI()
		{
			if (m_capacitor_model != 0)
			{
				if (ts_type == time_step_type::FORWARD)
				{
					//#const nl_nl_fptype Ugd = -m_DG.deltaV() * m_polarity; // Gate - Drain
					//#const nl_nl_fptype Ugs = -m_SG.deltaV() * m_polarity; // Gate - Source
					const nl_fptype Ugd = m_Vgd; // Gate - Drain
					const nl_fptype Ugs = m_Vgs; // Gate - Source
					const nl_fptype Ubs = nlconst::zero(); // Bulk - Source == 0 if connected
					const nl_fptype Ugb = Ugs - Ubs;

					m_cap_gb.time_step(m_Cgb, Ugb, step);
					m_cap_gs.time_step(m_Cgs, Ugs, step);
					m_cap_gd.time_step(m_Cgd, Ugd, step);
				}
				else
				{
					m_cap_gb.restore_state();
					m_cap_gs.restore_state();
					m_cap_gd.restore_state();
				}
			}
		}

	protected:

		NETLIB_RESETI()
		{
			NETLIB_NAME(FET)::reset();
			// Bulk diodes

			m_D_BD.set_param(m_model_acc.m_ISD, m_model_acc.m_N, exec().gmin(), constants::T0());
			#if (!BODY_CONNECTED_TO_SOURCE)
				m_D_BS.set_param(m_model_acc.m_ISS, m_model_acc.m_N, exec().gmin(), constants::T0());
			#endif
		}

		NETLIB_HANDLERI(terminal_handler)
		{
			// only called if connected to a rail net ==> notify the solver to recalculate
			auto *solv(m_SG.solver());
			if (solv != nullptr)
				solv->solve_now();
			else
				m_DG.solver()->solve_now();
		}
		NETLIB_UPDATE_PARAMI();
		NETLIB_UPDATE_TERMINALSI();

	private:

		nld_two_terminal m_DG;
		nld_two_terminal m_SG;
		nld_two_terminal m_SD;

		generic_diode<diode_e::MOS> m_D_BD;
#if (!BODY_CONNECTED_TO_SOURCE)
		generic_diode<diode_e::MOS> m_D_BS;
#endif

		generic_capacitor<capacitor_e::VARIABLE_CAPACITY> m_cap_gb;
		generic_capacitor<capacitor_e::VARIABLE_CAPACITY> m_cap_gs;
		generic_capacitor<capacitor_e::VARIABLE_CAPACITY> m_cap_gd;

		nl_fptype m_phi;
		nl_fptype m_gamma;
		nl_fptype m_vto;
		nl_fptype m_beta;
		nl_fptype m_lambda;

		// used in capacitance calculation
		nl_fptype m_Leff;
		nl_fptype m_CoxWL;
		nl_fptype m_polarity;

		// capacitance values

		nl_fptype m_Cgb;
		nl_fptype m_Cgs;
		nl_fptype m_Cgd;

		int m_capacitor_model;
		state_var<nl_fptype> m_Vgs;
		state_var<nl_fptype> m_Vgd;
		fet_model_t m_model_acc;

		void set_cap(generic_capacitor<capacitor_e::VARIABLE_CAPACITY> &cap,
			nl_fptype capval, nl_fptype V,
			nl_fptype &g11, nl_fptype &g12, nl_fptype &g21, nl_fptype &g22,
			nl_fptype &I1, nl_fptype &I2) const
		{
			const nl_fptype I = cap.Ieq(capval, V) * m_polarity;
			const nl_fptype G = cap.G(capval);
			g11 += G; g12 -= G; g21 -= G; g22 += G;
			I1 -= I; I2 += I;
			//printf("Cap: %g\n", capval);
		}

		void calculate_caps(nl_fptype Vgs, nl_fptype Vgd, nl_fptype Vth,
			nl_fptype &Cgs, nl_fptype &Cgd, nl_fptype &Cgb) const
		{
			nl_fptype Vctrl = Vgs - Vth * m_polarity;
			// Cut off - now further differentiated into 3 different formulas
			// Accumulation
			if (Vctrl <= -m_phi)
			{
				Cgb = m_CoxWL;
				Cgs = nlconst::zero();
				Cgd = nlconst::zero();
			}
			else if (Vctrl <= -m_phi / nlconst::two())
			{
				Cgb = -Vctrl * m_CoxWL / m_phi;
				Cgs = nlconst::zero();
				Cgd = nlconst::zero();
			}
			// Depletion
			else if (Vctrl <= 0)
			{
				Cgb = -Vctrl * m_CoxWL / m_phi;
				Cgs = Vctrl * m_CoxWL * nlconst::fraction(4.0, 3.0) / m_phi + nlconst::two_thirds() * m_CoxWL;
				Cgd = nlconst::zero();
			}
			else
			{
				const nl_fptype Vdsat = Vctrl;
				const nl_fptype Vds = Vgs - Vgd;
				// saturation
				if (Vdsat <= Vds)
				{
					Cgb = nlconst::zero();
					Cgs = nlconst::two_thirds() * m_CoxWL;
					Cgd = nlconst::zero();
				}
				else
				{
					// linear
					const auto Sqr1(plib::narrow_cast<nl_fptype>(plib::pow(Vdsat - Vds, 2)));
					const auto Sqr2(plib::narrow_cast<nl_fptype>(plib::pow(nlconst::two() * Vdsat - Vds, 2)));
					Cgb = 0;
					Cgs = m_CoxWL * (nlconst::one() - Sqr1 / Sqr2) * nlconst::two_thirds();
					Cgd = m_CoxWL * (nlconst::one() - Vdsat * Vdsat / Sqr2) * nlconst::two_thirds();
				}
			}
		}
	};

	// ----------------------------------------------------------------------------------------
	// MOSFET
	// ----------------------------------------------------------------------------------------

	NETLIB_UPDATE_TERMINALS(MOSFET)
	{
		nl_fptype Vgd = -m_DG.deltaV() * m_polarity; // Gate - Drain
		nl_fptype Vgs = -m_SG.deltaV() * m_polarity; // Gate - Source

		// limit step sizes

		const nl_fptype k = nlconst::magic(3.5); // see "Circuit Simulation", page 185
		nl_fptype d = (Vgs - m_Vgs);
		Vgs = m_Vgs + plib::reciprocal(k) * plib::signum(d) * plib::log1p(k * plib::abs(d));
		d = (Vgd - m_Vgd);
		Vgd = m_Vgd + plib::reciprocal(k) * plib::signum(d) * plib::log1p(k * plib::abs(d));

		m_Vgs = Vgs;
		m_Vgd = Vgd;

		const nl_fptype Vbs = nlconst::zero(); // Bulk - Source == 0 if connected
		//const nl_nl_fptype Vbd = m_SD.deltaV() * m_polarity;  // Bulk - Drain = Source  - Drain
		const nl_fptype Vds = Vgs - Vgd;
		const nl_fptype Vbd = -Vds;  // Bulk - Drain = Source  - Drain

#if (!BODY_CONNECTED_TO_SOURCE)
		m_D_BS.update_diode(Vbs);
#endif
		m_D_BD.update_diode(Vbd);

		// Are we in forward mode ?
		// in backward mode, just swap source and drain
		const bool is_forward = Vds >= nlconst::zero();

		// calculate Vth
		const nl_fptype Vbulk = is_forward ? Vbs : Vbd;
		const nl_fptype phi_m_Vbulk = (m_phi > Vbulk) ? plib::sqrt(m_phi - Vbulk) : nlconst::zero();
		const nl_fptype Vth = m_vto * m_polarity + m_gamma * (phi_m_Vbulk - plib::sqrt(m_phi));

		const nl_fptype Vctrl = (is_forward ? Vgs : Vgd) - Vth;

		nl_fptype Ids(0);
		nl_fptype gm(0);
		nl_fptype gds(0);
		nl_fptype gmb(0);

		const nl_fptype absVds = plib::abs(Vds);

		if (Vctrl <= nlconst::zero())
		{
			// cutoff region
			Ids = nlconst::zero();
			gm  = nlconst::zero();
			gds = nlconst::zero();
			gmb = nlconst::zero();
		}
		else
		{
			const nl_fptype beta = m_beta * (nlconst::one() + m_lambda * absVds);
			if (Vctrl <= absVds)
			{
				// saturation region
				Ids = beta * Vctrl * Vctrl / nlconst::two();
				gm  = beta * Vctrl;
				gds = m_lambda * m_beta * Vctrl * Vctrl / nlconst::two();
			}
			else
			{
				// linear region
				Ids = beta * absVds * (Vctrl - absVds / nlconst::two());
				gm  = beta * absVds;
				gds = beta * (Vctrl - absVds) + m_lambda * m_beta * absVds * (Vctrl - absVds / nlconst::two());
			}

			// back gate transconductance
			const nl_fptype bgtc = (phi_m_Vbulk != nlconst::zero()) ? (m_gamma / phi_m_Vbulk / nlconst::two()) : nlconst::zero();
			gmb = gm * bgtc;
		}

		// FIXME: these are needed to compute capacitance
		// nl_fptype Udsat = pol * std::max (Utst, 0.0);
		// Uon = pol * Vth;

		// compute bulk diode equivalent currents

		const nl_fptype IeqBD = m_D_BD.Ieq();
		const nl_fptype gbd = m_D_BD.G();

#if (!BODY_CONNECTED_TO_SOURCE)
		const nl_fptype IeqBS = m_D_BS.Ieq();
		const nl_fptype gbs = m_D_BS.G();
#else
		const nl_fptype IeqBS = nlconst::zero();
		const nl_fptype gbs = nlconst::zero();
#endif
		// exchange controlling nodes if necessary
		const nl_fptype gsource = is_forward ? (gm + gmb) : nlconst::zero();
		const nl_fptype gdrain  = is_forward ? nlconst::zero() : (gm + gmb);

		const nl_fptype IeqDS = (is_forward) ?
			   Ids - gm * Vgs - gmb * Vbs - gds * Vds
			: -Ids - gm * Vgd - gmb * Vbd - gds * Vds;

		// IG = 0
		nl_fptype IG = nlconst::zero();
		nl_fptype ID = (+IeqBD - IeqDS) * m_polarity;
		nl_fptype IS = (+IeqBS + IeqDS) * m_polarity;
		nl_fptype IB = (-IeqBD - IeqBS) * m_polarity;

		nl_fptype gGG = nlconst::zero();
		nl_fptype gGD = nlconst::zero();
		nl_fptype gGS = nlconst::zero();
		nl_fptype gGB = nlconst::zero();

		nl_fptype gDG =  gm;
		nl_fptype gDD =  gds + gbd - gdrain;
		const nl_fptype gDS = -gds - gsource;
		const nl_fptype gDB =  gmb - gbd;

		nl_fptype gSG = -gm;
		const nl_fptype gSD = -gds + gdrain;
		nl_fptype gSS =  gbs + gds + gsource;
		const nl_fptype gSB = -gbs - gmb;

		nl_fptype gBG =  nlconst::zero();
		const nl_fptype gBD = -gbd;
		const nl_fptype gBS = -gbs;
		nl_fptype gBB =  gbs + gbd;

		if (m_capacitor_model != 0)
		{
			const nl_fptype Vgb = Vgs - Vbs;

			if (is_forward)
				calculate_caps(Vgs, Vgd, Vth, m_Cgs, m_Cgd, m_Cgb);
			else
				calculate_caps(Vgd, Vgs, Vth, m_Cgd, m_Cgs, m_Cgb);

			set_cap(m_cap_gb, m_Cgb + m_model_acc.m_CGBO * m_Leff, Vgb, gGG, gGB, gBG, gBB, IG, IB);
			set_cap(m_cap_gs, m_Cgs + m_model_acc.m_CGSO * m_model_acc.m_W, Vgs, gGG, gGS, gSG, gSS, IG, IS);
			set_cap(m_cap_gd, m_Cgd + m_model_acc.m_CGDO * m_model_acc.m_W, Vgd, gGG, gGD, gDG, gDD, IG, ID);
		}

		// Source connected to body, Diode S-B shorted!
		const nl_fptype gSSBB = gSS + gBB + gBS + gSB;
		const auto zero(nlconst::zero());
		//                 S          G
		m_SG.set_mat(    gSSBB,   gSG + gBG, +(IS + IB),       // S
					   gGS + gGB,    gGG,      IG       );     // G
		//                 D          G
		m_DG.set_mat(     gDD,       gDG,    +ID,              // D
						  gGD,      zero,   zero        );     // G
		//                 S          D
		m_SD.set_mat(    zero,    gSD + gBD, zero,             // S
					   gDS + gDB,   zero,    zero);            // D
	}

	NETLIB_UPDATE_PARAM(MOSFET)
	{
	}

} // namespace analog

namespace devices {
	NETLIB_DEVICE_IMPL_NS(analog, MOSFET, "MOSFET", "MODEL")
} // namespace devices

} // namespace netlist
