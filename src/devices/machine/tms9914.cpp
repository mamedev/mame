// license:BSD-3-Clause
// copyright-holders:F. Ulivi
/*********************************************************************

    tms9914.cpp

    Texas Instruments TMS9914(A) GPIB Controller

    TODO:
    - A few interface commands
    - A few auxiliary commands

    Main reference for this IC:
    TI, jun 89, TMS9914A GPIB Controller - Data Manual

**********************************************************************/

#include "emu.h"
#include "tms9914.h"

// Debugging
#define LOG_NOISY_MASK  (LOG_GENERAL << 1)
#define LOG_REG_MASK    (LOG_NOISY_MASK << 1)
#define LOG_INT_MASK    (LOG_REG_MASK << 1)

#define LOG_NOISY(...)  LOGMASKED(LOG_NOISY_MASK, __VA_ARGS__)
#define LOG_REG(...)    LOGMASKED(LOG_REG_MASK, __VA_ARGS__)
#define LOG_INT(...)    LOGMASKED(LOG_INT_MASK, __VA_ARGS__)

//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

// Bit manipulation
namespace {
	template<typename T> constexpr T BIT_MASK(unsigned n)
	{
		return (T)1U << n;
	}

	template<typename T> void BIT_CLR(T& w , unsigned n)
	{
		w &= ~BIT_MASK<T>(n);
	}

	template<typename T> void BIT_SET(T& w , unsigned n)
	{
		w |= BIT_MASK<T>(n);
	}
}

// Registers
enum {
	REG_R_INT_STAT0 = 0,    // R 0: Interrupt status 0
	REG_R_INT_STAT1 = 1,    // R 1: Interrupt status 1
	REG_R_ADDR_STAT = 2,    // R 2: Address status
	REG_R_BUS_STAT = 3,     // R 3: Bus status
	REG_R_CMD_PT = 6,       // R 6: Command pass-through
	REG_R_DI = 7,           // R 7: Data input
	REG_W_INT_MASK0 = 0,    // W 0: Interrupt mask 0
	REG_W_INT_MASK1 = 1,    // W 1: Interrupt mask 1
	REG_W_AUX_CMD = 3,      // W 3: Auxiliary command
	REG_W_ADDRESS = 4,      // W 4: Address
	REG_W_SERIAL_P = 5,     // W 5: Serial poll
	REG_W_PARALLEL_P = 6,   // W 6: Parallel poll
	REG_W_DO = 7            // W 7: Data output
};

// Interrupt status/mask 0
constexpr unsigned REG_INT0_MAC_BIT = 0;    // My Address status Changed
constexpr unsigned REG_INT0_RLC_BIT = 1;    // Remote/Local status Changed
constexpr unsigned REG_INT0_SPAS_BIT = 2;   // Polled by serial poll
constexpr unsigned REG_INT0_END_BIT = 3;    // EOI received
constexpr unsigned REG_INT0_BO_BIT = 4;     // Byte Out interrupt
constexpr unsigned REG_INT0_BI_BIT = 5;     // Byte In interrupt
constexpr unsigned REG_INT0_INT1_BIT = 6;   // Interrupt(s) pending from INT1 register
constexpr unsigned REG_INT0_INT0_BIT = 7;   // Interrupt(s) pending from INT0 register
constexpr uint8_t  REG_INT0_INT_MASK = 0x3f;    // Mask of actual interrupt bits

// Interrupt status/mask 1
constexpr unsigned REG_INT1_IFC_BIT = 0;    // IFC received
constexpr unsigned REG_INT1_SRQ_BIT = 1;    // SRQ asserted
constexpr unsigned REG_INT1_MA_BIT = 2;     // My address received
constexpr unsigned REG_INT1_DCAS_BIT = 3;   // DCAS state active
constexpr unsigned REG_INT1_APT_BIT = 4;    // Address Pass-Through
constexpr unsigned REG_INT1_UNC_BIT = 5;    // Unrecognized command
constexpr unsigned REG_INT1_ERR_BIT = 6;    // Source handshake error
constexpr unsigned REG_INT1_GET_BIT = 7;    // Group Execute Trigger

// Address status register
constexpr unsigned REG_AS_ULPA_BIT = 0;     // LSB of last recognized address
constexpr unsigned REG_AS_TADS_BIT = 1;     // Addressed to talk
constexpr unsigned REG_AS_LADS_BIT = 2;     // Addressed to listen
constexpr unsigned REG_AS_TPAS_BIT = 3;     // TPAS state active
constexpr unsigned REG_AS_LPAS_BIT = 4;     // LPAS state active
constexpr unsigned REG_AS_ATN_BIT = 5;      // ATN asserted
constexpr unsigned REG_AS_LLO_BIT = 6;      // Local lockout enabled
constexpr unsigned REG_AS_REM_BIT = 7;      // Remote state enabled

// Bus status register
constexpr unsigned REG_BS_REN_BIT = 0;
constexpr unsigned REG_BS_IFC_BIT = 1;
constexpr unsigned REG_BS_SRQ_BIT = 2;
constexpr unsigned REG_BS_EOI_BIT = 3;
constexpr unsigned REG_BS_NRFD_BIT = 4;
constexpr unsigned REG_BS_NDAC_BIT = 5;
constexpr unsigned REG_BS_DAV_BIT = 6;
constexpr unsigned REG_BS_ATN_BIT = 7;

// Auxiliary command register
constexpr uint8_t  REG_AUXCMD_CMD_MASK = 0x1f;  // Mask of auxiliary command
constexpr unsigned REG_AUXCMD_CS_BIT = 7;       // Clear/set bit

// Auxiliary commands
enum {
	AUXCMD_SWRST = 0x00,
	AUXCMD_DACR = 0x01,
	AUXCMD_RHDF = 0x02,
	AUXCMD_HDFA = 0x03,
	AUXCMD_HDFE = 0x04,
	AUXCMD_NBAF = 0x05,
	AUXCMD_FGET = 0x06,
	AUXCMD_RTL = 0x07,
	AUXCMD_FEOI = 0x08,
	AUXCMD_LON = 0x09,
	AUXCMD_TON = 0x0a,
	AUXCMD_GTS = 0x0b,
	AUXCMD_TCA = 0x0c,
	AUXCMD_TCS = 0x0d,
	AUXCMD_RPP = 0x0e,
	AUXCMD_SIC = 0x0f,
	AUXCMD_SRE = 0x10,
	AUXCMD_RQC = 0x11,
	AUXCMD_RLC = 0x12,
	AUXCMD_DAI = 0x13,
	AUXCMD_PTS = 0x14,
	AUXCMD_STDL = 0x15,
	AUXCMD_SHDW = 0x16,
	AUXCMD_VSTDL = 0x17,
	AUXCMD_RSV2 = 0x18
};

// Address register
constexpr uint8_t  REG_ADDR_ADDR_MASK = 0x1f;   // Address mask
constexpr unsigned REG_ADDR_DAT_BIT = 5;        // Disable talker
constexpr unsigned REG_ADDR_DAL_BIT = 6;        // Disable listener
constexpr unsigned REG_ADDR_EDPA_BIT = 7;       // Dual primary address mode

// Serial poll register
constexpr uint8_t  REG_SERIAL_P_MASK = 0xbf;    // Serial status mask
constexpr unsigned REG_SERIAL_P_RSV1_BIT = 6;   // Request service 1

// Interface commands
constexpr uint8_t IFCMD_MASK       = 0x7f;  // Mask of valid bits in if. commands
constexpr uint8_t IFCMD_ACG_MASK   = 0x70;  // Mask of ACG commands
constexpr uint8_t IFCMD_ACG_VALUE  = 0x00;  // Value of ACG commands
constexpr uint8_t IFCMD_UCG_MASK   = 0x70;  // Mask of UCG commands
constexpr uint8_t IFCMD_UCG_VALUE  = 0x10;  // Value of UCG commands
constexpr uint8_t IFCMD_GROUP_MASK = 0x60;  // Mask of group id
constexpr uint8_t IFCMD_LAG_VALUE  = 0x20;  // Value of LAG commands
constexpr uint8_t IFCMD_TAG_VALUE  = 0x40;  // Value of TAG commands
constexpr uint8_t IFCMD_SCG_VALUE  = 0x60;  // Value of SCG commands
constexpr uint8_t IFCMD_GTL        = 0x01;  // Go to local
constexpr uint8_t IFCMD_SDC        = 0x04;  // Selected device clear
constexpr uint8_t IFCMD_GET        = 0x08;  // Group execute trigger
constexpr uint8_t IFCMD_TCT        = 0x09;  // Take control
constexpr uint8_t IFCMD_LLO        = 0x11;  // Local lock-out
constexpr uint8_t IFCMD_DCL        = 0x14;  // Device clear
constexpr uint8_t IFCMD_SPE        = 0x18;  // Serial poll enable
constexpr uint8_t IFCMD_SPD        = 0x19;  // Serial poll disable
constexpr uint8_t IFCMD_UNL        = 0x3f;  // Unlisten
constexpr uint8_t IFCMD_UNT        = 0x5f;  // Untalk

// Device type definition
DEFINE_DEVICE_TYPE(TMS9914, tms9914_device, "tms9914", "TMS9914 GPIB Controller")

// Constructors
tms9914_device::tms9914_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig , TMS9914 , tag , owner , clock),
	m_dio_read_func(*this, 0xff),
	m_dio_write_func(*this),
	m_signal_wr_fns(*this),
	m_int_write_func(*this),
	m_accrq_write_func(*this),
	m_int_line{false},
	m_accrq_line{false},
	m_dio{0},
	m_signals{false},
	m_ext_signals{false},
	m_no_reflection{false},
	m_ext_state_change{false},
	m_reg_int0_status{0},
	m_reg_int0_mask{0},
	m_reg_int1_status{0},
	m_reg_int1_mask{0},
	m_reg_address{0},
	m_reg_serial_p{0},
	m_reg_2nd_serial_p{0},
	m_reg_parallel_p{0},
	m_reg_2nd_parallel_p{0},
	m_reg_di{0},
	m_reg_do{0},
	m_reg_ulpa{false},
	m_swrst{false},
	m_hdfa{false},
	m_hdfe{false},
	m_rtl{false},
	m_gts{false},
	m_rpp{false},
	m_sic{false},
	m_sre{false},
	m_dai{false},
	m_pts{false},
	m_stdl{false},
	m_shdw{false},
	m_vstdl{false},
	m_rsvd2{false},
	m_ah_state{FSM_AH_AIDS},
	m_ah_adhs{false},
	m_ah_anhs{false},
	m_ah_aehs{false},
	m_sh_state{FSM_SH_SIDS},
	m_sh_shfs{false},
	m_sh_vsts{false},
	m_t_state{FSM_T_TIDS},
	m_t_tpas{false},
	m_t_spms{false},
	m_t_eoi_state{FSM_T_ENIS},
	m_l_state{FSM_L_LIDS},
	m_l_lpas{false},
	m_sr_state{FSM_SR_NPRS},
	m_rl_state{FSM_RL_LOCS},
	m_pp_ppas{false},
	m_c_state{FSM_C_CIDS},
	m_next_eoi{false}
{
	// Silence compiler complaints about unused variables
	(void)REG_INT1_IFC_BIT;
	(void)REG_INT1_GET_BIT;
}

// Signal inputs
void tms9914_device::eoi_w(int state)
{
	set_ext_signal(IEEE_488_EOI , state);
}

void tms9914_device::dav_w(int state)
{
	set_ext_signal(IEEE_488_DAV , state);
}

void tms9914_device::nrfd_w(int state)
{
	set_ext_signal(IEEE_488_NRFD , state);
}

void tms9914_device::ndac_w(int state)
{
	set_ext_signal(IEEE_488_NDAC , state);
}

void tms9914_device::ifc_w(int state)
{
	set_ext_signal(IEEE_488_IFC , state);
}

void tms9914_device::srq_w(int state)
{
	bool prev_srq = get_signal(IEEE_488_SRQ);
	set_ext_signal(IEEE_488_SRQ , state);
	if (cont_r() && !prev_srq && get_signal(IEEE_488_SRQ)) {
		set_int1_bit(REG_INT1_SRQ_BIT);
	}
}

void tms9914_device::atn_w(int state)
{
	set_ext_signal(IEEE_488_ATN , state);
}

void tms9914_device::ren_w(int state)
{
	set_ext_signal(IEEE_488_REN , state);
}

// Register I/O
void tms9914_device::write(offs_t offset, uint8_t data)
{
	LOG_REG("W %u=%02x\n" , offset , data);

	switch (offset) {
	case REG_W_INT_MASK0:
		m_reg_int0_mask = data & REG_INT0_INT_MASK;
		update_int();
		break;

	case REG_W_INT_MASK1:
		m_reg_int1_mask = data;
		update_int();
		break;

	case REG_W_AUX_CMD:
		do_aux_cmd(data & REG_AUXCMD_CMD_MASK , BIT(data , REG_AUXCMD_CS_BIT));
		break;

	case REG_W_ADDRESS:
		{
			uint8_t diff = m_reg_address ^ data;
			m_reg_address = data;
			if (BIT(diff , REG_ADDR_DAT_BIT) || BIT(diff , REG_ADDR_DAL_BIT)) {
				update_fsm();
			}
		}
		break;

	case REG_W_SERIAL_P:
		{
			uint8_t diff = m_reg_2nd_serial_p ^ data;
			m_reg_2nd_serial_p = data;
			if (BIT(diff , REG_SERIAL_P_RSV1_BIT)) {
				update_fsm();
			}
		}
		break;

	case REG_W_PARALLEL_P:
		m_reg_2nd_parallel_p = data;
		break;

	case REG_W_DO:
		m_reg_do = data;

		if (m_next_eoi) {
			m_next_eoi = false;
			if (!m_swrst) {
				if (m_t_eoi_state == FSM_T_ENIS) {
					m_t_eoi_state = FSM_T_ENRS;
				} else if (m_t_eoi_state == FSM_T_ENAS) {
					m_t_eoi_state = FSM_T_ERAS;
				}
			}

		}

		set_accrq(false);
		if (!m_swrst) {
			BIT_CLR(m_reg_int0_status , REG_INT0_BO_BIT);
			update_int();
			if (m_t_eoi_state == FSM_T_ENRS) {
				m_t_eoi_state = FSM_T_ERAS;
			} else if (m_t_eoi_state == FSM_T_ENAS) {
				m_t_eoi_state = FSM_T_ENIS;
			}
			bool update = sh_active();
			if (m_sh_shfs) {
				m_sh_shfs = false;
				update = true;
			}
			if (update) {
				update_fsm();
			}
		}
		break;

	default:
		LOG("Write to unmapped reg %u\n" , offset);
		break;
	}
}

uint8_t tms9914_device::read(offs_t offset)
{
	uint8_t res;

	switch (offset) {
	case REG_R_INT_STAT0:
		res = m_reg_int0_status;
		m_reg_int0_status = 0;
		update_int();
		break;

	case REG_R_INT_STAT1:
		res = m_reg_int1_status;
		m_reg_int1_status = 0;
		update_int();
		break;

	case REG_R_ADDR_STAT:
		res = 0;
		if (m_reg_ulpa) {
			BIT_SET(res , REG_AS_ULPA_BIT);
		}
		if (m_t_state != FSM_T_TIDS) {
			BIT_SET(res , REG_AS_TADS_BIT);
		}
		if (m_l_state != FSM_L_LIDS) {
			BIT_SET(res , REG_AS_LADS_BIT);
		}
		if (m_t_tpas) {
			BIT_SET(res , REG_AS_TPAS_BIT);
		}
		if (m_l_lpas) {
			BIT_SET(res , REG_AS_LPAS_BIT);
		}
		if (get_signal(IEEE_488_ATN)) {
			BIT_SET(res , REG_AS_ATN_BIT);
		}
		if (m_rl_state == FSM_RL_RWLS || m_rl_state == FSM_RL_LWLS) {
			BIT_SET(res , REG_AS_LLO_BIT);
		}
		if (m_rl_state == FSM_RL_REMS || m_rl_state == FSM_RL_RWLS) {
			BIT_SET(res , REG_AS_REM_BIT);
		}
		break;

	case REG_R_BUS_STAT:
		res = 0;
		if (get_signal(IEEE_488_REN)) {
			BIT_SET(res , REG_BS_REN_BIT);
		}
		if (get_ifcin()) {
			BIT_SET(res , REG_BS_IFC_BIT);
		}
		if (get_signal(IEEE_488_SRQ)) {
			BIT_SET(res , REG_BS_SRQ_BIT);
		}
		if (get_signal(IEEE_488_EOI)) {
			BIT_SET(res , REG_BS_EOI_BIT);
		}
		if (get_signal(IEEE_488_NRFD)) {
			BIT_SET(res , REG_BS_NRFD_BIT);
		}
		if (get_signal(IEEE_488_NDAC)) {
			BIT_SET(res , REG_BS_NDAC_BIT);
		}
		if (get_signal(IEEE_488_DAV)) {
			BIT_SET(res , REG_BS_DAV_BIT);
		}
		if (get_signal(IEEE_488_ATN)) {
			BIT_SET(res , REG_BS_ATN_BIT);
		}
		break;

	case REG_R_CMD_PT:
		res = get_dio();
		break;

	case REG_R_DI:
		res = m_reg_di;
		BIT_CLR(m_reg_int0_status , REG_INT0_BI_BIT);
		update_int();
		set_accrq(false);
		if (!m_hdfa && m_ah_anhs) {
			m_ah_anhs = false;
			update_fsm();
		}
		// TODO: ACRS -> ANRS ?
		break;

	default:
		LOG("Read from unmapped reg %u\n" , offset);
		res = 0;
		break;
	}

	LOG_REG("R %u=%02x\n" , offset , res);
	return res;
}

int tms9914_device::cont_r()
{
	return m_c_state != FSM_C_CIDS && m_c_state != FSM_C_CADS;
}

// device-level overrides
void tms9914_device::device_start()
{
	save_item(NAME(m_int_line));
	save_item(NAME(m_accrq_line));
	save_item(NAME(m_dio));
	save_item(NAME(m_signals));
	save_item(NAME(m_ext_signals));
	save_item(NAME(m_no_reflection));
	save_item(NAME(m_ext_state_change));
	save_item(NAME(m_reg_int0_status));
	save_item(NAME(m_reg_int0_mask));
	save_item(NAME(m_reg_int1_status));
	save_item(NAME(m_reg_int1_mask));
	save_item(NAME(m_reg_address));
	save_item(NAME(m_reg_serial_p));
	save_item(NAME(m_reg_2nd_serial_p));
	save_item(NAME(m_reg_parallel_p));
	save_item(NAME(m_reg_2nd_parallel_p));
	save_item(NAME(m_reg_di));
	save_item(NAME(m_reg_do));
	save_item(NAME(m_reg_ulpa));
	save_item(NAME(m_swrst));
	save_item(NAME(m_hdfa));
	save_item(NAME(m_hdfe));
	save_item(NAME(m_rtl));
	save_item(NAME(m_gts));
	save_item(NAME(m_rpp));
	save_item(NAME(m_sic));
	save_item(NAME(m_sre));
	save_item(NAME(m_dai));
	save_item(NAME(m_pts));
	save_item(NAME(m_stdl));
	save_item(NAME(m_shdw));
	save_item(NAME(m_vstdl));
	save_item(NAME(m_rsvd2));
	save_item(NAME(m_ah_state));
	save_item(NAME(m_ah_adhs));
	save_item(NAME(m_ah_anhs));
	save_item(NAME(m_sh_state));
	save_item(NAME(m_sh_shfs));
	save_item(NAME(m_sh_vsts));
	save_item(NAME(m_t_state));
	save_item(NAME(m_t_tpas));
	save_item(NAME(m_t_spms));
	save_item(NAME(m_t_eoi_state));
	save_item(NAME(m_l_state));
	save_item(NAME(m_l_lpas));
	save_item(NAME(m_sr_state));
	save_item(NAME(m_rl_state));
	save_item(NAME(m_pp_ppas));
	save_item(NAME(m_c_state));
	save_item(NAME(m_next_eoi));

	m_sh_dly_timer = timer_alloc(FUNC(tms9914_device::fsm_tick), this);
	m_ah_dly_timer = timer_alloc(FUNC(tms9914_device::fsm_tick), this);
	m_c_dly_timer = timer_alloc(FUNC(tms9914_device::fsm_tick), this);
}

void tms9914_device::device_reset()
{
	m_no_reflection = false;
	m_ext_state_change = false;
	m_swrst = true;
	m_hdfa = false;
	m_hdfe = false;
	m_rtl = false;
	m_rpp = false;
	m_sic = false;
	m_sre = false;
	m_dai = false;
	m_stdl = false;
	m_shdw = false;
	m_vstdl = false;
	m_rsvd2 = false;
	m_int_line = true;
	m_accrq_line = true;    // Ensure change is propagated

	m_reg_int0_status = 0;
	m_reg_int1_status = 0;
	m_reg_serial_p = 0;
	m_reg_2nd_serial_p = 0;
	m_reg_parallel_p = 0;
	m_reg_2nd_parallel_p = 0;

	do_swrst();
	update_fsm();
	update_int();
	update_ifc();
	update_ren();
}

TIMER_CALLBACK_MEMBER(tms9914_device::fsm_tick)
{
	update_fsm();
}

uint8_t tms9914_device::get_dio()
{
	return ~m_dio_read_func();
}

void tms9914_device::set_dio(uint8_t data)
{
	if (data != m_dio) {
		LOG_NOISY("DIO=%02x\n" , data);
		m_dio = data;
		m_dio_write_func(~data);
	}
}

bool tms9914_device::get_signal(ieee_488_signal_t signal) const
{
	return m_ext_signals[ signal ];
}

bool tms9914_device::get_ifcin() const
{
	return get_signal(IEEE_488_IFC) && !m_sic;
}

void tms9914_device::set_ext_signal(ieee_488_signal_t signal , int state)
{
	state = !state;
	if (m_ext_signals[ signal ] != state) {
		m_ext_signals[ signal ] = state;
		LOG_NOISY("EXT EOI %d DAV %d NRFD %d NDAC %d IFC %d SRQ %d ATN %d REN %d\n" ,
				  m_ext_signals[ IEEE_488_EOI ] ,
				  m_ext_signals[ IEEE_488_DAV ] ,
				  m_ext_signals[ IEEE_488_NRFD ] ,
				  m_ext_signals[ IEEE_488_NDAC ] ,
				  m_ext_signals[ IEEE_488_IFC ] ,
				  m_ext_signals[ IEEE_488_SRQ ] ,
				  m_ext_signals[ IEEE_488_ATN ] ,
				  m_ext_signals[ IEEE_488_REN ]);
		update_fsm();
	}
}

void tms9914_device::set_signal(ieee_488_signal_t signal , bool state)
{
	if (state != m_signals[ signal ]) {
		m_signals[ signal ] = state;
		LOG_NOISY("INT EOI %d DAV %d NRFD %d NDAC %d IFC %d SRQ %d ATN %d REN %d\n" ,
				  m_signals[ IEEE_488_EOI ] ,
				  m_signals[ IEEE_488_DAV ] ,
				  m_signals[ IEEE_488_NRFD ] ,
				  m_signals[ IEEE_488_NDAC ] ,
				  m_signals[ IEEE_488_IFC ] ,
				  m_signals[ IEEE_488_SRQ ] ,
				  m_signals[ IEEE_488_ATN ] ,
				  m_signals[ IEEE_488_REN ]);
		m_signal_wr_fns[ signal ](!state);
	}
}

void tms9914_device::do_swrst()
{
	m_reg_int0_status = 0;
	m_reg_int1_status = 0;

	m_ah_state = FSM_AH_AIDS;
	m_ah_adhs = false;
	m_ah_anhs = false;
	m_ah_aehs = false;
	m_sh_state = FSM_SH_SIDS;
	m_sh_shfs = true;
	m_sh_vsts = false;
	m_t_state = FSM_T_TIDS;
	m_t_tpas = false;
	m_t_spms = false;
	m_t_eoi_state = FSM_T_ENIS;
	m_l_state = FSM_L_LIDS;
	m_l_lpas = false;
	m_sr_state = FSM_SR_NPRS;
	m_rl_state = FSM_RL_LOCS;
	m_pp_ppas = false;
	m_c_state = FSM_C_CIDS;
	m_gts = false;

	update_int();
	set_accrq(false);
}

bool tms9914_device::listener_reset() const
{
	return m_swrst || BIT(m_reg_address , REG_ADDR_DAL_BIT) || m_sic || get_ifcin();
}

bool tms9914_device::talker_reset() const
{
	return m_swrst || BIT(m_reg_address , REG_ADDR_DAT_BIT) || m_sic || get_ifcin();
}

bool tms9914_device::controller_reset() const
{
	return m_swrst || get_ifcin();
}

bool tms9914_device::sh_active() const
{
	return m_sh_state == FSM_SH_SDYS || m_sh_state == FSM_SH_STRS || m_sh_state == FSM_SH_SERS;
}

void tms9914_device::update_fsm()
{
	if (m_no_reflection) {
		return;
	}

	m_no_reflection = true;

	bool changed = true;
	int prev_state;
	// Loop until all changes settle
	while (changed) {
		LOG_NOISY("SH %d SHFS %d AH %d T %d TPAS %d L %d LPAS %d PP %d C %d\n" ,
				  m_sh_state , m_sh_shfs , m_ah_state , m_t_state , m_t_tpas ,
				  m_l_state , m_l_lpas , m_pp_ppas , m_c_state);
		changed = m_ext_state_change;
		m_ext_state_change = false;

		// SH FSM
		prev_state = m_sh_state;
		bool sh_reset = m_swrst ||
			(get_signal(IEEE_488_ATN) && m_c_state != FSM_C_CACS) ||
			(!get_signal(IEEE_488_ATN) && !(m_t_state == FSM_T_TACS || m_t_state == FSM_T_SPAS));

		if (get_signal(IEEE_488_ATN) || !m_vstdl) {
			m_sh_vsts = false;
		}
		if (sh_reset) {
			m_sh_state = FSM_SH_SIDS;
			m_sh_dly_timer->reset();
		} else {
			switch (m_sh_state) {
			case FSM_SH_SIDS:
				if (m_t_state == FSM_T_TACS ||
					m_t_state == FSM_T_SPAS ||
					m_c_state == FSM_C_CACS) {
					m_sh_state = FSM_SH_SGNS;
				}
				break;

			case FSM_SH_SGNS:
				if (!m_sh_shfs || m_t_state == FSM_T_SPAS) {
					m_sh_state = FSM_SH_SDYS;
					unsigned clocks = m_sh_vsts ? 4 : (m_stdl ? 8 : 12);
					m_sh_dly_timer->adjust(clocks_to_attotime(clocks));
					LOG_NOISY("SH DLY %u\n" , clocks);
				}
				break;

			case FSM_SH_SDYS:
				if (!m_t_spms) {
					if (m_t_eoi_state == FSM_T_ENRS) {
						m_t_eoi_state = FSM_T_ENIS;
					} else if (m_t_eoi_state == FSM_T_ERAS) {
						m_t_eoi_state = FSM_T_ENAS;
					}
				}
				if (!m_sh_dly_timer->enabled() && !get_signal(IEEE_488_NRFD)) {
					if (get_signal(IEEE_488_NDAC)) {
						m_sh_state = FSM_SH_STRS;
					} else {
						m_sh_state = FSM_SH_SERS;
						set_int1_bit(REG_INT1_ERR_BIT);
					}
				}
				break;

			case FSM_SH_SERS:
				if (get_signal(IEEE_488_NDAC)) {
					m_sh_state = FSM_SH_STRS;
				}
				break;

			case FSM_SH_STRS:
				if (m_t_state != FSM_T_SPAS) {
					m_sh_shfs = true;
				}
				if (!get_signal(IEEE_488_ATN) && m_vstdl) {
					m_sh_vsts = true;
				}
				if (!get_signal(IEEE_488_NDAC)) {
					if (VERBOSE & LOG_GENERAL) {
						bool const iscmd = m_signals[IEEE_488_ATN];
						char cmd[16] = "";
						if (iscmd) {
							uint8_t tmp = m_dio & 0x7f;
							if (tmp >= 0x20 && tmp <= 0x3f)
								snprintf(cmd, 16, "MLA%d", tmp & 0x1f);
							else if (tmp >= 0x40 && tmp <= 0x5f)
								snprintf(cmd, 16, "MTA%d", tmp & 0x1f);
							else if (tmp >= 0x60 && tmp <= 0x7f)
								snprintf(cmd, 16, "MSA%d", tmp & 0x1f);
						}
						LOG("%.6f TX %s %02X/%d %s\n" , machine().time().as_double() , m_signals[IEEE_488_ATN] ? "C" : "D", m_dio , m_signals[ IEEE_488_EOI ], cmd);
					}
					m_sh_state = FSM_SH_SGNS;
				}
				break;

			default:
				LOG("Invalid SH state %d\n" , m_sh_state);
				m_sh_state = FSM_SH_SIDS;
			}
		}
		if (m_sh_state != prev_state) {
			changed = true;
			if (m_sh_state == FSM_SH_SGNS && m_sh_shfs &&
				m_t_state != FSM_T_SPAS) {
				// BO interrupt is raised when SGNS state is entered
				set_int0_bit(REG_INT0_BO_BIT);
			}
			if (prev_state == FSM_SH_STRS && m_t_state == FSM_T_SPAS &&
				(m_sr_state == FSM_SR_APRS1 || m_sr_state == FSM_SR_APRS2)) {
				set_int0_bit(REG_INT0_SPAS_BIT);
			}
		}
		// SH outputs
		// EOI is controlled by SH & C FSMs
		// DIO is controlled by SH & PP FSMs
		bool eoi_signal = false;
		uint8_t dio_byte = 0;
		set_signal(IEEE_488_DAV , m_sh_state == FSM_SH_STRS);
		if (sh_active()) {
			if (m_t_state == FSM_T_SPAS) {
				dio_byte = m_reg_serial_p & REG_SERIAL_P_MASK;
				if (m_sr_state == FSM_SR_APRS1 || m_sr_state == FSM_SR_APRS2) {
					// Set RQS
					BIT_SET(dio_byte , 6);
				}
			} else {
				dio_byte = m_reg_do;
			}
			eoi_signal = m_t_eoi_state == FSM_T_ERAS || m_t_eoi_state == FSM_T_ENAS;
		}

		// AH FSM
		prev_state = m_ah_state;
		bool ah_reset = m_swrst ||
			(get_signal(IEEE_488_ATN) && cont_r()) ||
			(!get_signal(IEEE_488_ATN) && m_l_state != FSM_L_LADS && m_l_state != FSM_L_LACS);
		if (ah_reset) {
			m_ah_state = FSM_AH_AIDS;
			m_ah_dly_timer->reset();
		} else {
			switch (m_ah_state) {
			case FSM_AH_AIDS:
				m_ah_state = FSM_AH_ANRS;
				break;

			case FSM_AH_ANRS:
				// See also the reading of DI register & RHDF command
				if (m_c_state != FSM_C_CWAS &&
					!get_signal(IEEE_488_DAV) &&
					(get_signal(IEEE_488_ATN) ||
					 (!m_ah_anhs && !m_ah_aehs))) {
					m_ah_state = FSM_AH_ACRS;
					m_ah_dly_timer->adjust(clocks_to_attotime(1));
				} else if (get_signal(IEEE_488_DAV)) {
					m_ah_state = FSM_AH_AWNS;
				}
				break;

			case FSM_AH_ACRS:
				if (!get_signal(IEEE_488_ATN) &&
					(m_ah_anhs || m_ah_aehs)) {
					m_ah_state = FSM_AH_ANRS;
					m_ah_dly_timer->reset();
				} else if (!m_ah_dly_timer->enabled() && get_signal(IEEE_488_DAV)) {
					m_ah_state = FSM_AH_ACDS1;
					m_ah_dly_timer->adjust(clocks_to_attotime(get_signal(IEEE_488_ATN) ? 5 : 1));
				}
				break;

			case FSM_AH_ACDS1:
				if (!get_signal(IEEE_488_DAV)) {
					m_ah_state = FSM_AH_ACRS;
					m_ah_dly_timer->adjust(clocks_to_attotime(1));
				} else if (!m_ah_dly_timer->enabled()) {
					m_ah_state = FSM_AH_ACDS2;
					if (get_signal(IEEE_488_ATN)) {
						// Got a command
						uint8_t if_cmd = get_dio();
						if_cmd_received(if_cmd & IFCMD_MASK);
					} else {
						// Got a DAB
						dab_received(get_dio() , get_signal(IEEE_488_EOI));
					}
				}
				break;

			case FSM_AH_ACDS2:
				if (!m_ah_adhs || !get_signal(IEEE_488_ATN)) {
					m_ah_state = FSM_AH_AWNS;
				} else if (!get_signal(IEEE_488_DAV)) {
					m_ah_state = FSM_AH_ANRS;
				}
				break;

			case FSM_AH_AWNS:
				if (!get_signal(IEEE_488_DAV)) {
					m_ah_state = FSM_AH_ANRS;
				}
				break;

			default:
				LOG("Invalid AH state %d\n" , m_ah_state);
				m_ah_state = FSM_AH_AIDS;
			}
		}
		if (m_ah_state != prev_state) {
			changed = true;
		}
		// AH outputs
		set_signal(IEEE_488_NRFD , m_ah_state == FSM_AH_ANRS || m_ah_state == FSM_AH_ACDS1 || m_ah_state == FSM_AH_ACDS2 || m_ah_state == FSM_AH_AWNS);
		set_signal(IEEE_488_NDAC , m_ah_state == FSM_AH_ANRS || m_ah_state == FSM_AH_ACRS || m_ah_state == FSM_AH_ACDS1 || m_ah_state == FSM_AH_ACDS2);

		// T FSM
		prev_state = m_t_state;
		if (talker_reset()) {
			m_t_state = FSM_T_TIDS;
		} else {
			switch (m_t_state) {
			case FSM_T_TIDS:
				break;

			case FSM_T_TADS:
				if (!get_signal(IEEE_488_ATN)) {
					if (m_t_spms) {
						m_t_state = FSM_T_SPAS;
						// When entering SPAS, serial poll register is copied into the
						// register that is actually output (as it's double buffered)
						m_reg_serial_p = m_reg_2nd_serial_p;
					} else {
						m_t_state = FSM_T_TACS;
					}
				}
				break;

			case FSM_T_TACS:
			case FSM_T_SPAS:
				if (get_signal(IEEE_488_ATN)) {
					m_t_state = FSM_T_TADS;
				}
				break;

			default:
				LOG("Invalid T state %d\n" , m_t_state);
				m_t_state = FSM_T_TIDS;
			}
		}
		if (m_t_state != prev_state) {
			changed = true;
		}
		if (m_t_spms && (m_swrst || get_ifcin() || cont_r())) {
			m_t_spms = false;
			changed = true;
		}
		// No direct T outputs

		// L FSM
		prev_state = m_l_state;
		if (listener_reset()) {
			m_l_state = FSM_L_LIDS;
		} else {
			switch (m_l_state) {
			case FSM_L_LIDS:
				break;

			case FSM_L_LADS:
				if (!get_signal(IEEE_488_ATN)) {
					m_l_state = FSM_L_LACS;
				}
				break;

			case FSM_L_LACS:
				if (get_signal(IEEE_488_ATN)) {
					m_l_state = FSM_L_LADS;
				}
				break;

			default:
				LOG("Invalid L state %d\n" , m_l_state);
				m_l_state = FSM_L_LIDS;
			}
		}
		if (m_l_state != prev_state) {
			changed = true;
		}
		// No direct L outputs

		// PP FSM
		if (!m_pp_ppas) {
			// PPSS
			if (!m_swrst && get_signal(IEEE_488_ATN) && get_signal(IEEE_488_EOI) && !cont_r()) {
				m_pp_ppas = true;
				changed = true;
				// Copy m_reg_2nd_parallel_p when entering PPAS
				m_reg_parallel_p = m_reg_2nd_parallel_p;
			}
		} else {
			// PPAS
			if (m_swrst || !get_signal(IEEE_488_ATN) || !get_signal(IEEE_488_EOI) || cont_r()) {
				m_pp_ppas = false;
				changed = true;
			}
		}
		// PP output
		if (m_pp_ppas) {
			dio_byte |= m_reg_parallel_p;
		}

		// SR FSM
		prev_state = m_sr_state;
		if (m_swrst) {
			m_sr_state = FSM_SR_NPRS;
		} else {
			switch (m_sr_state) {
			case FSM_SR_NPRS:
				if (m_t_state != FSM_T_SPAS &&
					(BIT(m_reg_2nd_serial_p , REG_SERIAL_P_RSV1_BIT) || m_rsvd2)) {
					m_sr_state = FSM_SR_SRQS;
				}
				break;

			case FSM_SR_SRQS:
				if (m_t_state == FSM_T_SPAS) {
					m_sr_state = FSM_SR_APRS1;
				} else if (!BIT(m_reg_2nd_serial_p , REG_SERIAL_P_RSV1_BIT) && !m_rsvd2) {
					m_sr_state = FSM_SR_NPRS;
				}
				break;

			case FSM_SR_APRS1:
				if (m_t_state == FSM_T_SPAS && m_sh_state == FSM_SH_STRS) {
					m_rsvd2 = false;
				}
				if (!BIT(m_reg_2nd_serial_p , REG_SERIAL_P_RSV1_BIT) && !m_rsvd2) {
					m_sr_state = FSM_SR_APRS2;
				}
				break;

			case FSM_SR_APRS2:
				if (m_t_state == FSM_T_SPAS) {
					if (m_sh_state == FSM_SH_STRS) {
						m_rsvd2 = false;
					}
				} else {
					m_sr_state = FSM_SR_NPRS;
				}
				break;

			default:
				LOG("Invalid SR state %d\n" , m_sr_state);
				m_sr_state = FSM_SR_NPRS;
			}
		}
		if (m_sr_state != prev_state) {
			changed = true;
		}
		// SR outputs
		set_signal(IEEE_488_SRQ , m_sr_state == FSM_SR_SRQS);

		// RL FSM
		if (m_rl_state != FSM_RL_LOCS && (m_swrst || !get_signal(IEEE_488_REN))) {
			m_rl_state = FSM_RL_LOCS;
			changed = true;
		}
		// No direct RL outputs

		// C outputs
		prev_state = m_c_state;
		if (controller_reset()) {
			m_c_state = FSM_C_CIDS;
			m_gts = false;
			m_c_dly_timer->reset();
		} else {
			switch (m_c_state) {
			case FSM_C_CIDS:
				// See also sic & rqc aux commands
				if (m_sic) {
					m_c_state = FSM_C_CADS;
				}
				break;

			case FSM_C_CADS:
				if (!get_signal(IEEE_488_ATN)) {
					m_c_state = FSM_C_CACS;
				}
				break;

			case FSM_C_CACS:
				if (m_rpp) {
					m_c_state = FSM_C_CPWS;
					m_gts = false;
				} else if (m_gts && !sh_active()) {
					m_c_state = FSM_C_CSBS;
					m_gts = false;
					// This ensures a BO interrupt is generated if TACS is active
					m_sh_state = FSM_SH_SIDS;
				}
				break;

			case FSM_C_CSBS:
				// tcs -> CWAS
				// tca -> CSHS
				break;

			case FSM_C_CWAS:
				if (m_ah_state == FSM_AH_ANRS) {
					m_c_state = FSM_C_CSHS;
					m_c_dly_timer->adjust(clocks_to_attotime(8));
				}
				break;

			case FSM_C_CSHS:
				if (!m_c_dly_timer->enabled()) {
					m_c_state = FSM_C_CSWS;
					m_c_dly_timer->adjust(clocks_to_attotime(2));
				}
				break;

			case FSM_C_CSWS:
				if (!m_c_dly_timer->enabled()) {
					m_c_state = FSM_C_CAWS;
					m_c_dly_timer->adjust(clocks_to_attotime(8));
				}
				break;

			case FSM_C_CAWS:
				if (!m_c_dly_timer->enabled()) {
					m_c_state = FSM_C_CACS;
				}
				break;

			case FSM_C_CPWS:
				if (!m_rpp) {
					m_c_state = FSM_C_CAWS;
					m_c_dly_timer->adjust(clocks_to_attotime(8));
				}
				break;

			default:
				LOG("Invalid C state %d\n" , m_c_state);
				m_c_state = FSM_C_CIDS;
			}
		}
		if (m_c_state != prev_state) {
			changed = true;
		}
		set_signal(IEEE_488_ATN , m_c_state == FSM_C_CACS ||
				   m_c_state == FSM_C_CSWS || m_c_state == FSM_C_CAWS ||
				   m_c_state == FSM_C_CPWS);
		eoi_signal = eoi_signal || m_c_state == FSM_C_CPWS;
		set_signal(IEEE_488_EOI , eoi_signal);
		set_dio(dio_byte);
	}

	m_no_reflection = false;
}

bool tms9914_device::is_my_address(uint8_t addr)
{
	uint8_t diff = (addr ^ m_reg_address) & REG_ADDR_ADDR_MASK;
	if (BIT(m_reg_address , REG_ADDR_EDPA_BIT)) {
		// If dual-address mode is enabled, difference in LSB of address is ignored
		BIT_CLR(diff , 0);
	}
	if (diff == 0) {
		m_reg_ulpa = BIT(addr , 0);
	}
	return diff == 0;
}

void tms9914_device::do_LAF()
{
	if (m_l_state == FSM_L_LIDS) {
		m_l_state = FSM_L_LADS;
		m_ext_state_change = true;
	}
	if (m_t_state != FSM_T_TIDS) {
		m_t_state = FSM_T_TIDS;
		m_ext_state_change = true;
	}
	if (m_rl_state == FSM_RL_LWLS) {
		m_rl_state = FSM_RL_RWLS;
		set_int0_bit(REG_INT0_RLC_BIT);
	} else if (m_rl_state == FSM_RL_LOCS && !m_rtl && get_signal(IEEE_488_REN)) {
		m_rl_state = FSM_RL_REMS;
		set_int0_bit(REG_INT0_RLC_BIT);
	}
}

void tms9914_device::do_TAF()
{
	if (m_t_state == FSM_T_TIDS) {
		m_t_state = FSM_T_TADS;
		m_ext_state_change = true;
	}
	if (m_l_state != FSM_L_LIDS) {
		m_l_state = FSM_L_LIDS;
		m_ext_state_change = true;
	}
}

void tms9914_device::if_cmd_received(uint8_t if_cmd)
{
	LOG("%.6f RX cmd:%02x\n" , machine().time().as_double() , if_cmd);

	bool sahf = false;

	// Any PCG command that is not MLA nor MTA clears LPAS & TPAS
	if ((if_cmd & IFCMD_GROUP_MASK) != IFCMD_SCG_VALUE) {
		m_l_lpas = false;
		m_t_tpas = false;
	}

	switch (if_cmd) {
	case IFCMD_GTL:
		if (m_l_state == FSM_L_LADS) {
			if (m_rl_state == FSM_RL_REMS) {
				m_rl_state = FSM_RL_LOCS;
				set_int0_bit(REG_INT0_RLC_BIT);
			} else if (m_rl_state == FSM_RL_RWLS) {
				m_rl_state = FSM_RL_LWLS;
				set_int0_bit(REG_INT0_RLC_BIT);
			}
		}
		break;

	case IFCMD_SDC:
		if (m_l_state == FSM_L_LADS) {
			set_int1_bit(REG_INT1_DCAS_BIT);
			sahf = BIT(m_reg_int1_mask , REG_INT1_DCAS_BIT);
		}
		break;

	case IFCMD_GET:
		// TODO:
		break;

	case IFCMD_TCT:
		if (m_t_state == FSM_T_TADS) {
			set_int1_bit(REG_INT1_UNC_BIT);
			sahf = BIT(m_reg_int1_mask , REG_INT1_UNC_BIT);
		}
		break;

	case IFCMD_LLO:
		if (m_rl_state == FSM_RL_LOCS && get_signal(IEEE_488_REN)) {
			m_rl_state = FSM_RL_LWLS;
		} else if (m_rl_state == FSM_RL_REMS) {
			m_rl_state = FSM_RL_RWLS;
		}
		break;

	case IFCMD_DCL:
		set_int1_bit(REG_INT1_DCAS_BIT);
		sahf = BIT(m_reg_int1_mask , REG_INT1_DCAS_BIT);
		break;

	case IFCMD_SPE:
		if (!get_ifcin() && !m_t_spms) {
			m_t_spms = true;
			m_ext_state_change = true;
		}
		break;

	case IFCMD_SPD:
		if (m_t_spms) {
			m_t_spms = false;
			m_ext_state_change = true;
		}
		break;

	case IFCMD_UNL:
		if (m_l_state != FSM_L_LIDS) {
			m_l_state = FSM_L_LIDS;
			set_int0_bit(REG_INT0_MAC_BIT);
		}
		break;

	case IFCMD_UNT:
		if (m_t_state != FSM_T_TIDS) {
			m_t_state = FSM_T_TIDS;
			set_int0_bit(REG_INT0_MAC_BIT);
		}
		break;

	default:
		if ((if_cmd & IFCMD_ACG_MASK) == IFCMD_ACG_VALUE) {
			// ACG
			if (m_l_state == FSM_L_LADS) {
				set_int1_bit(REG_INT1_UNC_BIT);
				sahf = BIT(m_reg_int1_mask , REG_INT1_UNC_BIT);
			}
		} else if ((if_cmd & IFCMD_UCG_MASK) == IFCMD_UCG_VALUE) {
			// UCG
			set_int1_bit(REG_INT1_UNC_BIT);
			sahf = BIT(m_reg_int1_mask , REG_INT1_UNC_BIT);
		} else if ((if_cmd & IFCMD_GROUP_MASK) == IFCMD_LAG_VALUE) {
			// LAG
			if (is_my_address(if_cmd)) {
				// MLA
				m_l_lpas = true;
				if (!BIT(m_reg_int1_mask , REG_INT1_APT_BIT)) {
					// Not using secondary addressing
					if (!listener_reset()) {
						if (m_l_state != FSM_L_LADS) {
							set_int0_bit(REG_INT0_MAC_BIT);
						}
						do_LAF();
					}
					if (!m_t_spms) {
						set_int1_bit(REG_INT1_MA_BIT);
						sahf = BIT(m_reg_int1_mask , REG_INT1_MA_BIT);
					}
				}
			}
		} else if ((if_cmd & IFCMD_GROUP_MASK) == IFCMD_TAG_VALUE) {
			// TAG
			if (is_my_address(if_cmd)) {
				// MTA
				m_t_tpas = true;
				if (!BIT(m_reg_int1_mask , REG_INT1_APT_BIT)) {
					// Not using secondary addressing
					if (!talker_reset()) {
						if (m_t_state != FSM_T_TADS) {
							set_int0_bit(REG_INT0_MAC_BIT);
						}
						do_TAF();
					}
					if (!m_t_spms) {
						set_int1_bit(REG_INT1_MA_BIT);
						sahf = BIT(m_reg_int1_mask , REG_INT1_MA_BIT);
					}
				}
			} else {
				// OTA
				if (m_t_state != FSM_T_TIDS) {
					set_int0_bit(REG_INT0_MAC_BIT);
				}
				m_t_state = FSM_T_TIDS;
			}
		} else {
			// SCG
			if (m_pts) {
				set_int1_bit(REG_INT1_UNC_BIT);
				sahf = BIT(m_reg_int1_mask , REG_INT1_UNC_BIT);
				m_pts = false;
			} else if (m_l_lpas || m_t_tpas) {
				set_int1_bit(REG_INT1_APT_BIT);
				sahf = BIT(m_reg_int1_mask , REG_INT1_APT_BIT);
			}
		}
		break;
	}

	if (sahf) {
		m_ah_adhs = true;
	}
}

void tms9914_device::dab_received(uint8_t dab , bool eoi)
{
	LOG("%.6f RX DAB:%02x/%d\n" , machine().time().as_double() , dab , eoi);
	m_reg_di = dab;
	if (!m_shdw) {
		m_ah_anhs = true;
		set_int0_bit(REG_INT0_BI_BIT);
		if (eoi) {
			set_int0_bit(REG_INT0_END_BIT);
		}
	}
	if (m_hdfe && eoi) {
		m_ah_aehs = true;
	}
}

void tms9914_device::do_aux_cmd(unsigned cmd , bool set_bit)
{
	switch (cmd) {
	case AUXCMD_SWRST:
		if (set_bit && !m_swrst) {
			do_swrst();
		}
		if (m_swrst != set_bit) {
			m_swrst = set_bit;
			update_fsm();
		}
		break;

	case AUXCMD_DACR:
		if (m_ah_adhs) {
			m_ah_adhs = false;
			if (BIT(m_reg_int1_mask , REG_INT1_APT_BIT)) {
				// Using secondary addressing
				if (set_bit && !listener_reset() && m_l_lpas) {
					do_LAF();
				}
				if (!set_bit && m_t_state == FSM_T_TADS && m_t_tpas) {
					m_t_state = FSM_T_TIDS;
					m_ext_state_change = true;
				}
				if (set_bit && !talker_reset() && m_t_tpas) {
					do_TAF();
				}
			}
			update_fsm();
		}
		break;

	case AUXCMD_RHDF:
		// TODO: ACRS -> ANRS
		if (m_ah_anhs || m_ah_aehs) {
			m_ah_anhs = false;
			m_ah_aehs = false;
			update_fsm();
		}
		break;

	case AUXCMD_HDFA:
		if (!m_swrst) {
			m_hdfa = set_bit;
		}
		break;

	case AUXCMD_HDFE:
		if (!m_swrst) {
			m_hdfe = set_bit;
		}
		break;

	case AUXCMD_NBAF:
		m_t_eoi_state = FSM_T_ENIS;
		if (!m_sh_shfs) {
			m_sh_shfs = true;
			update_fsm();
		}
		break;

	case AUXCMD_FGET:
		LOG("Unimplemented FGET cmd\n");
		break;

	case AUXCMD_RTL:
		m_rtl = set_bit;
		if (m_rtl && m_rl_state == FSM_RL_REMS) {
			m_rl_state = FSM_RL_LOCS;
			set_int0_bit(REG_INT0_RLC_BIT);
		}
		break;

	case AUXCMD_FEOI:
		m_next_eoi = true;
		break;

	case AUXCMD_LON:
		if (set_bit) {
			if (!listener_reset()) {
				do_LAF();
			}
		} else {
			m_l_state = FSM_L_LIDS;
			m_ext_state_change = true;
		}
		update_fsm();
		break;

	case AUXCMD_TON:
		if (set_bit) {
			if (!talker_reset()) {
				do_TAF();
			}
		} else {
			m_t_state = FSM_T_TIDS;
			m_ext_state_change = true;
		}
		update_fsm();
		break;

	case AUXCMD_GTS:
		if (m_c_state == FSM_C_CACS) {
			m_gts = true;
			update_fsm();
		}
		break;

	case AUXCMD_TCA:
		if (m_c_state == FSM_C_CSBS) {
			m_c_state = FSM_C_CSHS;
			m_ext_state_change = true;
			// Manual says delay is 8 clock cycles, but 10 at least are needed
			// to pass diagb hpib diagnostic
			//m_c_dly_timer->adjust(clocks_to_attotime(8));
			m_c_dly_timer->adjust(clocks_to_attotime(10));
			update_fsm();
		}
		break;

	case AUXCMD_TCS:
		if (m_c_state == FSM_C_CSBS) {
			m_c_state = FSM_C_CWAS;
			m_ext_state_change = true;
			update_fsm();
		}
		break;

	case AUXCMD_RPP:
		if (!m_swrst && m_rpp != set_bit) {
			m_rpp = set_bit;
			update_fsm();
		}
		break;

	case AUXCMD_SIC:
		if (m_sic != set_bit) {
			m_sic = set_bit;
			update_ifc();
			if (!controller_reset() && m_sic && m_c_state == FSM_C_CIDS) {
				m_c_state = FSM_C_CADS;
				m_ext_state_change = true;
			}
			update_fsm();
		}
		break;

	case AUXCMD_SRE:
		m_sre = set_bit;
		update_ren();
		break;

	case AUXCMD_RQC:
		if (!controller_reset() && m_c_state == FSM_C_CIDS) {
			m_c_state = FSM_C_CADS;
			m_ext_state_change = true;
			update_fsm();
		}
		break;

	case AUXCMD_RLC:
		if (m_c_state != FSM_C_CIDS) {
			m_c_state = FSM_C_CIDS;
			m_ext_state_change = true;
			update_fsm();
		}
		break;

	case AUXCMD_DAI:
		m_dai = set_bit;
		update_int();
		break;

	case AUXCMD_PTS:
		m_pts = true;
		break;

	case AUXCMD_STDL:
		LOG("Unimplemented STDL=%d cmd\n" , set_bit);
		break;

	case AUXCMD_SHDW:
		if (!m_swrst) {
			m_shdw = set_bit;
			if (m_shdw && m_ah_anhs) {
				m_ah_anhs = false;
				update_fsm();
			}
		}
		break;

	case AUXCMD_VSTDL:
		LOG("Unimplemented VSTDL=%d cmd\n" , set_bit);
		break;

	case AUXCMD_RSV2:
		if (set_bit != m_rsvd2) {
			m_rsvd2 = set_bit;
			update_fsm();
		}
		break;

	default:
		LOG("Unrecognized aux cmd %u\n" , cmd);
		break;
	}
}

void tms9914_device::set_int0_bit(unsigned bit_no)
{
	BIT_SET(m_reg_int0_status , bit_no);
	update_int();
	if (bit_no == REG_INT0_BI_BIT || (bit_no == REG_INT0_BO_BIT && m_c_state != FSM_C_CACS)) {
		set_accrq(true);
	}
}

void tms9914_device::set_int1_bit(unsigned bit_no)
{
	BIT_SET(m_reg_int1_status , bit_no);
	update_int();
}

void tms9914_device::update_int()
{
	bool new_int_line = false;
	m_reg_int0_status &= REG_INT0_INT_MASK;
	if (m_reg_int0_status & m_reg_int0_mask) {
		BIT_SET(m_reg_int0_status , REG_INT0_INT0_BIT);
		new_int_line = true;
	}
	if (m_reg_int1_status & m_reg_int1_mask) {
		BIT_SET(m_reg_int0_status , REG_INT0_INT1_BIT);
		new_int_line = true;
	}
	if (m_dai) {
		new_int_line = false;
	}
	if (new_int_line != m_int_line) {
		LOG_INT("INT=%d\n" , new_int_line);
		m_int_line = new_int_line;
		m_int_write_func(m_int_line);
	}
}

void tms9914_device::update_ifc()
{
	set_signal(IEEE_488_IFC , m_sic);
}

void tms9914_device::update_ren()
{
	set_signal(IEEE_488_REN , m_sre);
}

void tms9914_device::set_accrq(bool state)
{
	if (state != m_accrq_line) {
		LOG_INT("ACCRQ=%d\n" , state);
		m_accrq_line = state;
		m_accrq_write_func(m_accrq_line);
	}
}
