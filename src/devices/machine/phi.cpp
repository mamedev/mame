// license:BSD-3-Clause
// copyright-holders:F. Ulivi
/*********************************************************************

    phi.h

    HP PHI (Processor-to-Hpib-Interface) (1AA6-6x04)

    PHI supports these features of HP-IB:
    * SH1
    * AH1
    * T1/TE1
    * L1/LE1
    * SR1
    * RL2
    * PP1
    * DC1
    * DT1
    * C1,C2,C3,C4,C5
    * HP non-standard IDENTIFY sequence

    Fun fact: PHI has no clock input, its FSMs are driven only by
    changes in input signals and by a few internal monostables

    Main reference for this ASIC:
    HP 12009-90001, sep 82, HP12009A HP-IB Interface Reference Manual

*********************************************************************/

#include "emu.h"
#include "phi.h"

// Debugging
#define VERBOSE 1
#define LOG(x)  do { if (VERBOSE) logerror x; } while (0)
#define VERBOSE_0 0
#define LOG_0(x)  do { if (VERBOSE_0) logerror x; } while (0)

// Macros to clear/set single bits
#define BIT_MASK(n) (1U << (n))
#define BIT_CLR(w , n)  ((w) &= ~BIT_MASK(n))
#define BIT_SET(w , n)  ((w) |= BIT_MASK(n))

// Timers
enum {
	SH_DELAY_TMR_ID,
	C_DELAY_TMR_ID
};

// Register addresses
enum {
	REG_R_INT_COND = 0, // R 0: Interrupting conditions
	REG_W_INT_COND = 0, // W 0: Interrupting conditions
	REG_R_INT_MASK = 1, // R 1: Interrupt mask
	REG_W_INT_MASK = 1, // W 1: Interrupt mask
	REG_R_INBOUND_FIFO = 2, // R 2: Inbound FIFO
	REG_W_OUTBOUND_FIFO = 2,// W 2: Outbound FIFO
	REG_R_STATUS = 3,   // R 3: Status
	REG_W_STATUS = 3,   // W 3: Status
	REG_R_CONTROL = 4,  // R 4: Control
	REG_W_CONTROL = 4,  // W 4: Control
	REG_R_ADDRESS = 5,  // R 5: HPIB address
	REG_W_ADDRESS = 5,  // W 5: HPIB address
	REG_R_1ST_ID = 6,   // R 6: 1st ID byte
	REG_W_1ST_ID = 6,   // W 6: 1st ID byte
	REG_R_2ND_ID = 7,   // R 7: 2nd ID byte
	REG_W_2ND_ID = 7    // W 7: 2nd ID byte
};

// All valid bits in registers
#define REG_ALL_MASK    0xc0ff

// D0 & D1 bits
#define REG_D0D1_MASK   0xc000  // Mask of D0/D1 bits
#define REG_D0D1_SHIFT  14      // Position of D0/D1 bits

// D8-D15 bits
#define REG_D08D15_MASK 0xff    // Mask of D8:D15 bits

// Bits in INT_COND & INT_MASK
#define REG_INT_DEV_CLEAR_BIT   0   // Device clear
#define REG_INT_FIFO_IDLE_BIT   1   // FIFO idle
#define REG_INT_FIFO_AV_BIT     2   // FIFO bytes available
#define REG_INT_FIFO_ROOM_BIT   3   // FIFO room available
#define REG_INT_SRQ_BIT         4   // Service request
#define REG_INT_PP_RESPONSE_BIT 5   // PP response
#define REG_INT_PROC_ABORT_BIT  6   // Processor handshake abort
#define REG_INT_STATUS_CH_BIT   7   // Status change
#define REG_INT_PARITY_ERR_BIT  14  // Parity error
#define REG_INT_PENDING_BIT     15  // Interrupt pending
#define REG_INT_CLEARABLE_MASK  0x40c1  // Mask of clearable bits
#define REG_INT_STATE_MASK      0x803e  // Mask of "state" bits

// Bits in inbound FIFO
#define REG_IFIFO_NORMAL_MASK   0x0000  // Mask of D0/D1 bits for "normal" bytes
#define REG_IFIFO_CNT_EXP_MASK  0x8000  // Mask for a byte that caused byte count to expire
#define REG_IFIFO_LAST_MASK     0xc000  // Mask for last byte in a record
#define REG_IFIFO_2_ADDR_MASK   0x4000  // Mask for secondary addresses
#define REG_IFIFO_TALK_BIT      5       // Bit of "talk" flag

// Bits in outbound FIFO
#define REG_OFIFO_SPECIAL_BIT   14      // Bit to discriminate between normal bytes and the rest
#define REG_OFIFO_END_BIT       15      // Bit of EOI
#define REG_OFIFO_IFCMD_MASK    0x4000  // Mask of interface commands
#define REG_OFIFO_UNCNT_MASK    0xc000  // Mask of uncounted transfer enable
#define REG_OFIFO_XFER_EN_MASK  0x0000  // Mask of byte transfer enable
#define REG_OFIFO_LF_INH_BIT    15      // Bit of LF detection inhibit

// Bits in status register
#define REG_STATUS_DATA_FREEZE_BIT  0   // Outbound data freeze
#define REG_STATUS_LISTEN_BIT       1   // Addressed to listen
#define REG_STATUS_TALK_BIT         2   // Addressed to talk or identify
#define REG_STATUS_SYS_CTRL_BIT     3   // System controller
#define REG_STATUS_CONTROLLER_BIT   4   // Current controller
#define REG_STATUS_REMOTE_BIT       5   // Remote state
#define REG_STATUS_D0D1_BIT         6   // D0/D1 bit access
#define REG_STATUS_STATE_MASK       0x3e    // Mask of "state" bits

// Bits in control register
#define REG_CTRL_INIT_OFIFO_BIT     0   // Initialize outbound FIFO
#define REG_CTRL_DMA_FIFO_BIT       1   // DMA FIFO selection
#define REG_CTRL_SERVICE_REQ_BIT    2   // Request service
#define REG_CTRL_PP_RESPONSE_BIT    3   // Respond to PP
#define REG_CTRL_IFC_BIT            4   // IFC value
#define REG_CTRL_REN_BIT            5   // REN value
#define REG_CTRL_PAR_FREEZE_BIT     6   // Parity freeze
#define REG_CTRL_8BIT_PROC_BIT      7   // 8-bit processor

// Bits in address register
#define REG_ADDR_HPIB_ADDR_BIT      0   // HPIB address
#define REG_ADDR_LA_BIT             5   // Listen always
#define REG_ADDR_TA_BIT             6   // Talk always
#define REG_ADDR_ONLINE_BIT         7   // Online

// Interface commands
#define IFCMD_MASK              0x7f    // Mask of interface commands
#define IFCMD_DCL               0x14    // Device clear
#define IFCMD_GET               0x08    // Group execute trigger
#define IFCMD_GTL               0x01    // Go to local
#define IFCMD_LLO               0x11    // Local lock-out
#define IFCMD_AG_MASK           0x60    // Mask of bits identifying address group commands
#define IFCMD_ADDR_MASK         0x1f    // Mask of address in AG commands
#define IFCMD_LAG_VALUE         0x20    // Value of LAG commands
#define IFCMD_TAG_VALUE         0x40    // Value of TAG commands
#define IFCMD_SCG_VALUE         0x60    // Value of SCG commands
#define IFCMD_PPC               0x05    // Parallel poll configure
#define IFCMD_PPX_MASK          0x70    // Mask of PPE/PPD commands
#define IFCMD_PPE_VALUE         0x60    // Parallel poll enable
#define IFCMD_PPE_S_BIT         3       // Position of "S" bit in PPE
#define IFCMD_PPE_PPR_MASK      7       // Mask in PPE of PPR msg no.
#define IFCMD_PPD_VALUE         0x70    // Parallel poll disable
#define IFCMD_PPU               0x15    // Parallel poll unconfigure
#define IFCMD_SDC               0x04    // Selected device clear
#define IFCMD_SPD               0x19    // Serial poll disable
#define IFCMD_SPE               0x18    // Serial poll enable
#define IFCMD_TCT               0x09    // Take control
#define IFCMD_UNL               0x3f    // Unlisten
#define IFCMD_UNT               0x5f    // Untalk

// Delays
#define DELAY_T1    2000    // T1: 2 us
#define DELAY_T7    500     // T7: 0.5 us
#define DELAY_T9    1500    // T9: 1.5 us
#define DELAY_T10   1500    // T10: 1.5 us

// Controller address
#define CONTROLLER_ADDR         0x1e    // PHI always has this address when it's a controller

// Device type definition
const device_type PHI = device_creator<phi_device>;

// Constructors
phi_device::phi_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, __FILE__),
	  m_dio_read_func(*this),
	  m_dio_write_func(*this),
	  m_signal_wr_fns{
		  devcb_write_line(*this),
		  devcb_write_line(*this),
		  devcb_write_line(*this),
		  devcb_write_line(*this),
		  devcb_write_line(*this),
		  devcb_write_line(*this),
		  devcb_write_line(*this),
		  devcb_write_line(*this) },
	  m_int_write_func(*this),
	  m_dmarq_write_func(*this)
{
}

phi_device::phi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PHI, "HP PHI", tag, owner, clock, "PHI", __FILE__),
	  m_dio_read_func(*this),
	  m_dio_write_func(*this),
	  m_signal_wr_fns{
		  devcb_write_line(*this),
		  devcb_write_line(*this),
		  devcb_write_line(*this),
		  devcb_write_line(*this),
		  devcb_write_line(*this),
		  devcb_write_line(*this),
		  devcb_write_line(*this),
		  devcb_write_line(*this) },
	  m_int_write_func(*this),
	  m_dmarq_write_func(*this)
{
}

WRITE_LINE_MEMBER(phi_device::eoi_w)
{
	set_ext_signal(PHI_488_EOI , state);
}

WRITE_LINE_MEMBER(phi_device::dav_w)
{
	set_ext_signal(PHI_488_DAV , state);
}

WRITE_LINE_MEMBER(phi_device::nrfd_w)
{
	set_ext_signal(PHI_488_NRFD , state);
}

WRITE_LINE_MEMBER(phi_device::ndac_w)
{
	set_ext_signal(PHI_488_NDAC , state);
}

WRITE_LINE_MEMBER(phi_device::ifc_w)
{
	set_ext_signal(PHI_488_IFC , state);
}

WRITE_LINE_MEMBER(phi_device::srq_w)
{
	set_ext_signal(PHI_488_SRQ , state);
}

WRITE_LINE_MEMBER(phi_device::atn_w)
{
	set_ext_signal(PHI_488_ATN , state);
}

WRITE_LINE_MEMBER(phi_device::ren_w)
{
	set_ext_signal(PHI_488_REN , state);
}


void phi_device::set_ext_signal(phi_488_signal_t signal , int state)
{
	state = !state;
	if (m_ext_signals[ signal ] != state) {
		m_ext_signals[ signal ] = state;
		LOG_0(("EXT EOI %d DAV %d NRFD %d NDAC %d IFC %d SRQ %d ATN %d REN %d\n" ,
			 m_ext_signals[ PHI_488_EOI ] ,
			 m_ext_signals[ PHI_488_DAV ] ,
			 m_ext_signals[ PHI_488_NRFD ] ,
			 m_ext_signals[ PHI_488_NDAC ] ,
			 m_ext_signals[ PHI_488_IFC ] ,
			 m_ext_signals[ PHI_488_SRQ ] ,
			 m_ext_signals[ PHI_488_ATN ] ,
			 m_ext_signals[ PHI_488_REN ]));
		update_fsm();
	}
}

WRITE16_MEMBER(phi_device::reg16_w)
{
	int_reg_w(offset , data & REG_ALL_MASK);
}

READ16_MEMBER(phi_device::reg16_r)
{
	uint16_t res;

	switch (offset) {
	case REG_R_INT_COND:
		res = m_reg_int_cond & m_reg_int_mask;
		break;

	case REG_R_INT_MASK:
		res = m_reg_int_mask;
		break;

	case REG_R_INBOUND_FIFO:
		// TODO: read from PP response
		if (m_fifo_in.empty()) {
			BIT_SET(m_reg_int_cond, REG_INT_PROC_ABORT_BIT);
			res = 0;
		} else {
			res = m_fifo_in.dequeue();
		}
		update_fsm();
		break;

	case REG_R_STATUS:
		res = m_reg_status;
		break;

	case REG_R_CONTROL:
		res = m_reg_control;
		break;

	case REG_R_ADDRESS:
		res = m_reg_address;
		break;

	case REG_R_1ST_ID:
		res = m_reg_1st_id;
		break;

	case REG_R_2ND_ID:
		res = m_reg_2nd_id;
		break;

	default:
		res = 0;
		LOG(("Reading from unmapped address (%u)\n", offset));
		break;
	};

	if (offset != REG_R_STATUS) {
		// Store D0/D1 in top bits of status register
		m_reg_status = (m_reg_status & ~(3U << REG_STATUS_D0D1_BIT)) |
			((res & REG_D0D1_MASK) >> (REG_D0D1_SHIFT - REG_STATUS_D0D1_BIT));
	}

	//LOG(("R %u=%04x\n" , offset , res));
	return res;
}

WRITE8_MEMBER(phi_device::reg8_w)
{
	int_reg_w(offset , data);
}

READ8_MEMBER(phi_device::reg8_r)
{
	return (uint8_t)reg16_r(space , offset , mem_mask);
}

void phi_device::device_start()
{
	save_item(NAME(m_dio));
	save_item(NAME(m_signals));
	save_item(NAME(m_ext_signals));
	save_item(NAME(m_sys_controller));
	save_item(NAME(m_loopback));
	save_item(NAME(m_id_enabled));
	save_item(NAME(m_sh_state));
	save_item(NAME(m_ah_state));
	save_item(NAME(m_t_state));
	save_item(NAME(m_t_spms));
	save_item(NAME(m_l_state));
	save_item(NAME(m_sr_state));
	save_item(NAME(m_pp_state));
	save_item(NAME(m_pp_pacs));
	save_item(NAME(m_ppr_msg));
	save_item(NAME(m_s_sense));
	save_item(NAME(m_c_state));
	save_item(NAME(m_be_counter));
	save_item(NAME(m_reg_status));
	save_item(NAME(m_reg_int_cond));
	save_item(NAME(m_reg_int_mask));
	save_item(NAME(m_reg_1st_id));
	save_item(NAME(m_reg_2nd_id));
	save_item(NAME(m_reg_control));
	save_item(NAME(m_reg_address));
	save_item(NAME(m_nba_origin));

	m_dio_read_func.resolve_safe(0xff);
	m_dio_write_func.resolve_safe();
	for (auto& f : m_signal_wr_fns) {
		f.resolve_safe();
	}
	m_int_write_func.resolve_safe();
	m_dmarq_write_func.resolve_safe();

	m_sh_dly_timer = timer_alloc(SH_DELAY_TMR_ID);
	m_c_dly_timer = timer_alloc(C_DELAY_TMR_ID);
}

void phi_device::device_reset()
{
	m_dio = 0;
	for (auto& s : m_signals) {
		s = false;
	}
	for (auto& s : m_ext_signals) {
		s = false;
	}
	m_no_recursion = false;
	// The following variables are set "true" because m_reg_address is set to 0
	m_sys_controller = true;
	m_loopback = true;
	m_id_enabled = false;
	m_reg_status = 0;
	m_reg_int_cond = 0;
	m_reg_int_mask = 0;
	m_reg_1st_id = 0;
	m_reg_2nd_id = 0;
	m_reg_control = 0;
	m_reg_address = 0;
	m_fifo_in.clear();
	m_fifo_out.clear();

	pon_msg();
	update_488();
}

void phi_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	LOG_0(("tmr %d enabled %d\n" , id , timer.enabled()));
	update_fsm();
}

void phi_device::int_reg_w(offs_t offset , uint16_t data)
{
	if (BIT(m_reg_control , REG_CTRL_8BIT_PROC_BIT)) {
		// In 8-bit mode, D0/D1 come from status register
		data = (data & REG_D08D15_MASK) | ((m_reg_status << (REG_D0D1_SHIFT - REG_STATUS_D0D1_BIT)) & REG_D0D1_MASK);
	}

	//LOG(("W %u=%04x\n" , offset , data));

	switch (offset) {
	case REG_W_INT_COND:
		// Bits D1/D8/D9/D15 only are clearable when written to 1
		m_reg_int_cond &= ~(data & REG_INT_CLEARABLE_MASK);
		update_fsm();
		break;

	case REG_W_INT_MASK:
		m_reg_int_mask = data;
		update_fsm();
		break;

	case REG_W_OUTBOUND_FIFO:
		if (m_fifo_out.full()) {
			BIT_SET(m_reg_int_cond, REG_INT_PROC_ABORT_BIT);
		} else {
			m_fifo_out.enqueue(data);
		}
		update_fsm();
		break;

	case REG_W_STATUS:
		// Copy D0/D1 access bits into status register
		m_reg_status = (m_reg_status & ~(3U << REG_STATUS_D0D1_BIT)) |
			(data & (3U << REG_STATUS_D0D1_BIT));
		if (BIT(data , REG_STATUS_DATA_FREEZE_BIT) && m_fifo_in.empty()) {
			BIT_CLR(m_reg_status, REG_STATUS_DATA_FREEZE_BIT);
		}
		update_fsm();
		break;

	case REG_W_CONTROL:
		// D0/D1/D15 are not mapped into register
		m_reg_control = data & 0xfe;
		if (BIT(data , REG_CTRL_INIT_OFIFO_BIT)) {
			m_fifo_out.clear();
		}
		if (m_loopback) {
			// TODO: better?
			m_id_enabled = BIT(m_reg_control , REG_CTRL_PP_RESPONSE_BIT);
		}
		update_fsm();
		break;

	case REG_W_ADDRESS:
		{
			// No D0/D1 in register
			data &= REG_D08D15_MASK;
			bool prev_ol = BIT(m_reg_address , REG_ADDR_ONLINE_BIT);
			m_reg_address = data;
			bool current_ol = BIT(m_reg_address , REG_ADDR_ONLINE_BIT);
			// TODO: consider SYS CNTRL input, too
			m_sys_controller = !current_ol;
			m_loopback = !current_ol;
			if (!current_ol) {
				// IDENTIFY is enabled by PP_RESPONSE bit in control register
				m_id_enabled = BIT(m_reg_control , REG_CTRL_PP_RESPONSE_BIT);
			} else if (!prev_ol) {
				// Going on-line
				pon_msg();
				m_id_enabled = BIT(m_reg_control , REG_CTRL_PP_RESPONSE_BIT);
			}
			if (prev_ol != current_ol) {
				update_488();
			}
			update_fsm();
		}
		break;

	case REG_W_1ST_ID:
		// No D0/D1 in register
		m_reg_1st_id = data & REG_D08D15_MASK;
		update_fsm();
		break;

	case REG_W_2ND_ID:
		// No D0/D1 in register
		m_reg_2nd_id = data & REG_D08D15_MASK;
		update_fsm();
		break;

	default:
		LOG(("Writing to unmapped address (%u)\n", offset));
		break;
	}
}

uint8_t phi_device::get_dio(void)
{
	if (m_loopback) {
		return m_dio;
	} else {
		return ~m_dio_read_func();
	}
}

void phi_device::set_dio(uint8_t data)
{
	if (data != m_dio) {
		LOG_0(("DIO=%02x\n" , data));
		m_dio = data;
		if (!m_loopback) {
			m_dio_write_func(~data);
		}
	}
}

bool phi_device::get_signal(phi_488_signal_t signal)
{
	if (m_loopback) {
		return m_signals[ signal ];
	} else {
		return m_ext_signals[ signal ];
	}
}

void phi_device::set_signal(phi_488_signal_t signal , bool state)
{
	if (state != m_signals[ signal ]) {
		m_signals[ signal ] = state;
		LOG_0(("INT EOI %d DAV %d NRFD %d NDAC %d IFC %d SRQ %d ATN %d REN %d\n" ,
			 m_signals[ PHI_488_EOI ] ,
			 m_signals[ PHI_488_DAV ] ,
			 m_signals[ PHI_488_NRFD ] ,
			 m_signals[ PHI_488_NDAC ] ,
			 m_signals[ PHI_488_IFC ] ,
			 m_signals[ PHI_488_SRQ ] ,
			 m_signals[ PHI_488_ATN ] ,
			 m_signals[ PHI_488_REN ]));
		if (!m_loopback) {
			m_signal_wr_fns[ signal ](!state);
		}
	}
}

void phi_device::pon_msg(void)
{
	m_sh_state = PHI_SH_SIDS;
	m_ah_state = PHI_AH_AIDS;
	m_t_state = PHI_T_TIDS;
	m_t_spms = false;
	m_l_state = PHI_L_LIDS;
	m_sr_state = PHI_SR_NPRS;
	m_pp_pacs = false;
	uint8_t addr = my_address();
	if (addr <= 7) {
		// If address <= 7, PP is automatically enabled and configured for PPR = ~address
		m_ppr_msg = addr ^ 7;
		m_pp_state = PHI_PP_PPSS;
	} else {
		m_ppr_msg = 0;
		m_pp_state = PHI_PP_PPIS;
	}
	m_s_sense = true;
	m_c_state = PHI_C_CIDS;
	m_be_counter = 0;
	m_nba_origin = NBA_NONE;
}

void phi_device::update_488(void)
{
	if (m_loopback) {
		m_dio_write_func(~0);
		for (auto& f : m_signal_wr_fns) {
			f(1);
		}
	} else {
		m_dio_write_func(~m_dio);
		for (unsigned i = 0; i < PHI_488_SIGNAL_COUNT; i++) {
			m_signal_wr_fns[ i ](!m_signals[ i ]);
		}
	}
}

void phi_device::update_fsm(void)
{
	if (m_no_recursion) {
		// Prevent recursion into this function whenever a signal change propagates through
		// set_ext_signal
		return;
	}
	m_no_recursion = true;

	set_signal(PHI_488_IFC , m_sys_controller && BIT(m_reg_control , REG_CTRL_IFC_BIT));
	set_signal(PHI_488_REN , m_sys_controller && BIT(m_reg_control , REG_CTRL_REN_BIT));

	bool changed = true;
	int prev_state;
	uint8_t new_byte = 0;
	bool new_eoi = false;
	bool prev_cic = controller_in_charge();
	// TODO: SR FSM
	// TODO: RL FSM
	// Loop until all changes settle
	while (changed) {
		LOG_0(("SH %d AH %d T %d SPMS %d L %d SR %d PP %d PACS %d PPR %u S %d C %d\n" ,
			 m_sh_state , m_ah_state , m_t_state , m_t_spms , m_l_state , m_sr_state ,
			 m_pp_state , m_pp_pacs , m_ppr_msg , m_s_sense , m_c_state));
		LOG_0(("O E/F=%d/%d I E/F=%d/%d\n" , m_fifo_out.empty() , m_fifo_out.full() , m_fifo_in.empty() , m_fifo_in.full()));
		changed = false;

		// SH FSM
		prev_state = m_sh_state;
		bool sh_reset =
			(get_signal(PHI_488_ATN) && !(m_c_state == PHI_C_CACS || m_c_state == PHI_C_CTRS)) ||
			(!get_signal(PHI_488_ATN) && !(m_t_state == PHI_T_TACS || m_t_state == PHI_T_SPAS || m_t_state == PHI_T_ID3 || m_t_state == PHI_T_ID5));

		if (sh_reset) {
			m_sh_state = PHI_SH_SIDS;
			m_sh_dly_timer->reset();
		} else {
			switch (m_sh_state) {
			case PHI_SH_SIDS:
				if (m_t_state == PHI_T_TACS ||
					m_t_state == PHI_T_SPAS ||
					m_t_state == PHI_T_ID3 ||
					m_t_state == PHI_T_ID5 ||
					m_c_state == PHI_C_CACS) {
					m_sh_state = PHI_SH_SGNS;
				}
				break;

			case PHI_SH_SGNS:
				if ((m_nba_origin = nba_msg(new_byte , new_eoi)) != NBA_NONE) {
					m_sh_state = PHI_SH_SDYS;
					m_sh_dly_timer->adjust(attotime::from_nsec(DELAY_T1));
					LOG_0(("SH DLY enabled %d\n" , m_sh_dly_timer->enabled()));
				}
				break;

			case PHI_SH_SDYS:
				if (!get_signal(PHI_488_NRFD) && !m_sh_dly_timer->enabled()) {
					m_sh_state = PHI_SH_STRS;
				}
				break;

			case PHI_SH_STRS:
				if (!get_signal(PHI_488_NDAC)) {
					LOG(("TX %02x/%d\n" , m_dio , m_signals[ PHI_488_EOI ]));
					m_sh_state = PHI_SH_SGNS;
					clear_nba((nba_origin_t)m_nba_origin);
				}
				break;

			default:
				logerror("Invalid SH state %d\n" , m_sh_state);
				m_sh_state = PHI_SH_SIDS;
			}
		}
		if (m_sh_state != prev_state) {
			changed = true;
		}

		// SH outputs
		// EOI is controlled by SH & C FSMs
		bool eoi_signal = false;
		uint8_t dio_byte = 0;
		set_signal(PHI_488_DAV , m_sh_state == PHI_SH_STRS);
		if (m_sh_state == PHI_SH_SDYS || m_sh_state == PHI_SH_STRS) {
			nba_msg(new_byte , new_eoi);
			dio_byte = new_byte;
			eoi_signal = new_eoi;
		}

		// AH FSM
		prev_state = m_ah_state;
		bool ah_reset = !(get_signal(PHI_488_ATN) || m_l_state == PHI_L_LADS || m_l_state == PHI_L_LACS || m_c_state == PHI_C_CSBS);
		if (ah_reset) {
			m_ah_state = PHI_AH_AIDS;
		} else {
			switch (m_ah_state) {
			case PHI_AH_AIDS:
				m_ah_state = PHI_AH_ANRS;
				break;

			case PHI_AH_ANRS:
				//if (!tcs_msg() && (get_signal(PHI_488_ATN) || rdy_msg())) {
				// According to standard either ATN or rdy should also be true, but rdy is always true in PHI
				if (!tcs_msg()) {
					m_ah_state = PHI_AH_ACRS;
				}
				break;

			case PHI_AH_ACRS:
				if (get_signal(PHI_488_DAV)) {
					m_ah_state = PHI_AH_ACDS;
				}
				// rdy is always true
				// } else if (!get_signal(PHI_488_ATN) && !rdy_msg()) {
				//  m_ah_state = PHI_AH_ANRS;
				// }
				break;

			case PHI_AH_ACDS:
				// FSM stays in this state until the acceptor has
				// accepted the data byte or the interface command.
				if (get_signal(PHI_488_ATN)) {
					uint8_t if_cmd = get_dio();
					bool parity_ok = odd_parity(if_cmd);
					if (!parity_ok) {
						BIT_SET(m_reg_int_cond , REG_INT_PARITY_ERR_BIT);
					}
					if (BIT(m_reg_control , REG_CTRL_PAR_FREEZE_BIT) && !parity_ok) {
						// With even parity and PARITY FREEZE set, command is ignored and
						// AH FSM freezes in ACDS
						m_ah_state = PHI_AH_ACDS_FROZEN;
					} else {
						// Clear parity bit & process command
						if_cmd &= IFCMD_MASK;
						if (if_cmd_received(if_cmd)) {
							m_ah_state = PHI_AH_AWNS;
						}
					}
				} else if (byte_received(get_dio() , get_signal(PHI_488_EOI))) {
					m_ah_state = PHI_AH_AWNS;
				}
				break;

			case PHI_AH_ACDS_FROZEN:
			case PHI_AH_AWNS:
				if (!get_signal(PHI_488_DAV)) {
					m_ah_state = PHI_AH_ANRS;
				}
				break;

			default:
				logerror("Invalid AH state %d\n" , m_ah_state);
				m_ah_state = PHI_AH_AIDS;
			}
		}
		if (m_ah_state != prev_state) {
			changed = true;
		}
		// AH outputs
		set_signal(PHI_488_NRFD , m_ah_state == PHI_AH_ANRS || m_ah_state == PHI_AH_ACDS || m_ah_state == PHI_AH_ACDS_FROZEN || m_ah_state == PHI_AH_AWNS);
		set_signal(PHI_488_NDAC , m_ah_state == PHI_AH_ANRS || m_ah_state == PHI_AH_ACRS || m_ah_state == PHI_AH_ACDS || m_ah_state == PHI_AH_ACDS_FROZEN);

		// T FSM
		prev_state = m_t_state;
		if (get_signal(PHI_488_IFC)) {
			m_t_state = PHI_T_TIDS;
			m_t_spms = false;
		} else {
			switch (m_t_state) {
			case PHI_T_TIDS:
				if (ton_msg()) {
					m_t_state = PHI_T_TADS;
				}
				break;

			case PHI_T_TADS:
				if (!get_signal(PHI_488_ATN)) {
					if (m_t_spms) {
						m_t_state = PHI_T_SPAS;
					} else {
						m_t_state = PHI_T_TACS;
					}
				}
				break;

			case PHI_T_SPAS:
			case PHI_T_TACS:
				if (get_signal(PHI_488_ATN)) {
					m_t_state = PHI_T_TADS;
				}
				break;

			case PHI_T_ID1:
				break;

			case PHI_T_ID2:
				if (!get_signal(PHI_488_ATN)) {
					m_t_state = PHI_T_ID3;
				}
				break;

			case PHI_T_ID3:
				if (get_signal(PHI_488_ATN)) {
					m_t_state = PHI_T_ID2;
				}
				break;

			case PHI_T_ID4:
				if (!get_signal(PHI_488_ATN)) {
					m_t_state = PHI_T_ID5;
				}
				break;

			case PHI_T_ID5:
				if (get_signal(PHI_488_ATN)) {
					m_t_state = PHI_T_ID4;
				}
				break;

			case PHI_T_ID6:
				break;

			default:
				logerror("Invalid T state %d\n" , m_t_state);
				m_t_state = PHI_T_TIDS;
			}
		}
		if (m_t_state != prev_state) {
			changed = true;
		}
		// No direct T outputs

		// L FSM
		prev_state = m_l_state;
		if (get_signal(PHI_488_IFC)) {
			m_l_state = PHI_L_LIDS;
		} else {
			switch (m_l_state) {
			case PHI_L_LIDS:
				if (lon_msg()) {
					m_l_state = PHI_L_LADS;
				}
				break;

			case PHI_L_LADS:
				if (!get_signal(PHI_488_ATN)) {
					m_l_state = PHI_L_LACS;
				}
				break;

			case PHI_L_LACS:
				if (get_signal(PHI_488_ATN)) {
					m_l_state = PHI_L_LADS;
				}
				break;

			default:
				logerror("Invalid L state %d\n" , m_l_state);
				m_l_state = PHI_L_LIDS;
			}
		}
		if (m_l_state != prev_state) {
			changed = true;
		}
		// No direct L outputs

		// PP FSM
		prev_state = m_pp_state;
		switch (m_pp_state) {
		case PHI_PP_PPIS:
			break;

		case PHI_PP_PPSS:
			if (get_signal(PHI_488_ATN) && get_signal(PHI_488_EOI)) {
				m_pp_state = PHI_PP_PPAS;
			}
			break;

		case PHI_PP_PPAS:
			if (!get_signal(PHI_488_ATN) || !get_signal(PHI_488_EOI)) {
				m_pp_state = PHI_PP_PPSS;
			}
			break;

		default:
			logerror("Invalid PP state %d\n" , m_pp_state);
			m_pp_state = PHI_PP_PPIS;
		}
		if (m_pp_state != prev_state) {
			changed = true;
		}
		// PP outputs
		if (m_pp_state == PHI_PP_PPAS && m_s_sense == !!BIT(m_reg_control , REG_CTRL_PP_RESPONSE_BIT)) {
			LOG(("PP %u\n" , m_ppr_msg));
			dio_byte |= (1U << m_ppr_msg);
		}

		// C FSM
		prev_state = m_c_state;
		if (!m_sys_controller && get_signal(PHI_488_IFC)) {
			m_c_state = PHI_C_CIDS;
			m_c_dly_timer->reset();
		} else {
			switch (m_c_state) {
			case PHI_C_CIDS:
				if (m_sys_controller && get_signal(PHI_488_IFC)) {
					m_c_state = PHI_C_CADS;
				}
				break;

			case PHI_C_CADS:
				if (!get_signal(PHI_488_ATN)) {
					m_c_state = PHI_C_CACS;
				}
				break;

			case PHI_C_CACS:
				// If there are ifcmds to send, just stay in CACS
				// else wait for SH to finish its current transfer then decide what to do
				if (nba_msg(new_byte , new_eoi) != NBA_CMD_FROM_OFIFO &&
					m_sh_state != PHI_SH_STRS && m_sh_state != PHI_SH_SDYS) {
					if (!m_fifo_out.empty()) {
						// Possible cases
						// D0/D1    Meaning of 1st word of OFIFO
						// =====================================
						// x0       Counted transfer enable or byte to be sent
						// 11       Uncounted transfer enable
						// 01       Send interface command (already caught by nba_msg)
						m_c_state = PHI_C_CSBS;
						m_be_counter = 0;
					} else if (rpp_msg()) {
						// Start parallel polling
						m_c_state = PHI_C_CPWS;
					}
					// It's not clear what PHI does when it's controller-in-charge
					// and it's nothing to do. Here we park it in CACS.
				}
				break;

			case PHI_C_CPWS:
				// TODO:
				break;

			case PHI_C_CPPS:
				// TODO:
				break;

			case PHI_C_CSBS:
				if (tcs_msg() && m_ah_state == PHI_AH_ANRS) {
					m_c_state = PHI_C_CSHS;
					m_c_dly_timer->adjust(attotime::from_nsec(DELAY_T10));
				}
				break;

			case PHI_C_CSHS:
				// tcs_msg cannot go false here
				if (!m_c_dly_timer->enabled()) {
					m_c_state = PHI_C_CSWS;
					m_c_dly_timer->adjust(attotime::from_nsec(DELAY_T7));
				}
				break;

			case PHI_C_CAWS:
				if (rpp_msg()) {
					m_c_state = PHI_C_CPWS;
					m_c_dly_timer->reset();
				} else if (!m_c_dly_timer->enabled()) {
					m_c_state = PHI_C_CACS;
				}
				break;

			case PHI_C_CTRS:
				if (m_sh_state != PHI_SH_STRS) {
					m_c_state = PHI_C_CIDS;
				}
				break;

			case PHI_C_CSWS:
				if (m_t_state == PHI_T_TADS || !m_c_dly_timer->enabled()) {
					m_c_state = PHI_C_CAWS;
					m_c_dly_timer->adjust(attotime::from_nsec(DELAY_T9));
				}
				break;

			default:
				logerror("Invalid C state %d\n" , m_c_state);
				m_c_state = PHI_C_CIDS;
			}
		}
		if (m_c_state != prev_state) {
			changed = true;
		}
		// C outputs
		set_signal(PHI_488_ATN , m_c_state == PHI_C_CACS || m_c_state == PHI_C_CPWS ||
				   m_c_state == PHI_C_CPPS || m_c_state == PHI_C_CSWS ||
				   m_c_state == PHI_C_CAWS || m_c_state == PHI_C_CTRS);
		eoi_signal = eoi_signal || m_c_state == PHI_C_CPWS || m_c_state == PHI_C_CPPS;
		set_signal(PHI_488_EOI , eoi_signal);
		set_dio(dio_byte);
	}

	// Update status register
	m_reg_status &= ~REG_STATUS_STATE_MASK;
	if (m_l_state != PHI_L_LIDS) {
		BIT_SET(m_reg_status, REG_STATUS_LISTEN_BIT);
	}
	if (m_t_state != PHI_T_TIDS && m_t_state != PHI_T_ID1) {
		BIT_SET(m_reg_status, REG_STATUS_TALK_BIT);
	}
	if (m_sys_controller) {
		BIT_SET(m_reg_status, REG_STATUS_SYS_CTRL_BIT);
	}
	if (controller_in_charge()) {
		BIT_SET(m_reg_status, REG_STATUS_CONTROLLER_BIT);
	}
	// TODO: REG_STATUS_REMOTE_BIT

	// Update interrupting condition register and INT signal
	if (prev_cic != controller_in_charge()) {
		BIT_SET(m_reg_int_cond, REG_INT_STATUS_CH_BIT);
	}
	m_reg_int_cond &= ~REG_INT_STATE_MASK;
	if (m_fifo_out.empty()) {
		BIT_SET(m_reg_int_cond , REG_INT_FIFO_IDLE_BIT);
	}
	if (!m_fifo_in.empty()) {
		BIT_SET(m_reg_int_cond, REG_INT_FIFO_AV_BIT);
	}
	if (!m_fifo_out.full()) {
		BIT_SET(m_reg_int_cond, REG_INT_FIFO_ROOM_BIT);
	}
	if (controller_in_charge() && get_signal(PHI_488_SRQ)) {
		BIT_SET(m_reg_int_cond, REG_INT_SRQ_BIT);
	}
	// TODO: REG_INT_PP_RESPONSE_BIT
	bool int_pending = (m_reg_int_cond & m_reg_int_mask) != 0;
	bool int_line = false;
	if (int_pending) {
		BIT_SET(m_reg_int_cond, REG_INT_PENDING_BIT);
		if (BIT(m_reg_int_mask , REG_INT_PENDING_BIT)) {
			int_line = true;
		}
	}
	if (int_line != m_int_line) {
		m_int_line = int_line;
		m_int_write_func(m_int_line);
	}

	(void)m_dmarq_line; // TODO: update DMARQ
	m_no_recursion = false;
}

phi_device::nba_origin_t phi_device::nba_msg(uint8_t& new_byte , bool& new_eoi) const
{
	// TODO: consider CIC
	if (!m_fifo_out.empty()) {
		uint16_t word = m_fifo_out.peek();
		if ((word & REG_D0D1_MASK) == REG_OFIFO_IFCMD_MASK) {
			// Controller sends an interface command
			new_byte = (uint8_t)(word & IFCMD_MASK);
			if (!odd_parity(new_byte)) {
				BIT_SET(new_byte, 7);
			}
			new_eoi = false;
			return NBA_CMD_FROM_OFIFO;
		}
	}

	switch (m_t_state) {
	case PHI_T_TACS:
		if (!BIT(m_reg_status , REG_STATUS_DATA_FREEZE_BIT) &&
			!BIT(m_reg_int_cond , REG_INT_DEV_CLEAR_BIT) &&
			!m_fifo_out.empty()) {
			uint16_t word = m_fifo_out.peek();
			if (!BIT(word , REG_OFIFO_SPECIAL_BIT)) {
				// Talker sends a data byte
				new_byte = (uint8_t)word;
				new_eoi = BIT(word , REG_OFIFO_END_BIT);
				return NBA_BYTE_FROM_OFIFO;
			}
		}
		break;

	case PHI_T_SPAS:
		// Reply to serial poll: STB & RQS
		new_byte = m_sr_state == PHI_SR_NPRS ? 0x80 : 0x40;
		new_eoi = false;
		return NBA_FROM_SPAS;

	case PHI_T_ID3:
		// 1st byte of ID
		new_byte = (uint8_t)m_reg_1st_id;
		new_eoi = false;
		return NBA_FROM_ID3;

	case PHI_T_ID5:
		// 2nd byte of ID
		new_byte = (uint8_t)m_reg_2nd_id;
		new_eoi = true;
		return NBA_FROM_ID5;

	default:
		break;
	}
	return NBA_NONE;
}

void phi_device::clear_nba(nba_origin_t origin)
{
	switch (origin) {
	case NBA_CMD_FROM_OFIFO:
	case NBA_BYTE_FROM_OFIFO:
		m_fifo_out.dequeue();
		break;

	case NBA_FROM_ID3:
		m_t_state = PHI_T_ID4;
		break;

	case NBA_FROM_ID5:
		m_t_state = PHI_T_ID6;
		break;

	default:
		break;
	}
}

bool phi_device::if_cmd_received(uint8_t byte)
{
	LOG(("RX cmd: %02x\n" , byte));

	bool accepted = true;

	if ((byte & IFCMD_AG_MASK) != IFCMD_SCG_VALUE && byte != IFCMD_PPC) {
		// PACS -> PUCS when receiving a PCG command that is not PPC
		m_pp_pacs = false;
	}

	// TODO: IFC & non-reflection into controller
	switch (byte) {
	case IFCMD_GTL:
		// Go to local
		// TODO:
		break;

	case IFCMD_SDC:
		// Selected device clear
		if (m_l_state == PHI_L_LADS && !controller_in_charge()) {
			BIT_SET(m_reg_int_cond, REG_INT_DEV_CLEAR_BIT);
		}
		break;

	case IFCMD_PPC:
		// Parallel poll configure
		if (m_l_state == PHI_L_LADS) {
			m_pp_pacs = true;
		}
		break;

	case IFCMD_GET:
		// Group execute trigger
		// TODO:
		break;

	case IFCMD_TCT:
		// Take control
		if (m_c_state == PHI_C_CIDS && m_t_state == PHI_T_TADS) {
			// Take control
			m_c_state = PHI_C_CADS;
		}
		if (m_c_state == PHI_C_CACS && m_t_state != PHI_T_TADS) {
			// Give control to someone else
			m_c_state = PHI_C_CTRS;
		}
		break;

	case IFCMD_LLO:
		// Local lock-out
		// TODO:
		break;

	case IFCMD_DCL:
		// Device clear
		if (!controller_in_charge()) {
			BIT_SET(m_reg_int_cond, REG_INT_DEV_CLEAR_BIT);
		}
		break;

	case IFCMD_PPU:
		// Parallel poll unconfigure
		if (m_pp_state == PHI_PP_PPSS) {
			m_pp_state = PHI_PP_PPIS;
		}
		break;

	case IFCMD_SPE:
		// Serial poll enable
		m_t_spms = true;
		break;

	case IFCMD_SPD:
		// Serial poll disable
		m_t_spms = false;
		break;

	case IFCMD_UNL:
		// Unlisten
		if (m_l_state == PHI_L_LADS && !lon_msg()) {
			m_l_state = PHI_L_LIDS;
		}
		break;

	case IFCMD_UNT:
		// Untalk
		if (!ton_msg()) {
			if (m_id_enabled) {
				m_t_state = PHI_T_ID1;
			} else {
				m_t_state = PHI_T_TIDS;
			}
		}
		break;

	default:
		{
			uint8_t address = byte & IFCMD_ADDR_MASK;
			uint8_t ag = byte & IFCMD_AG_MASK;
			bool my_addr = address == my_address();

			if (ag == IFCMD_LAG_VALUE) {
				// LAG
				if (!lon_msg() && m_l_state == PHI_L_LIDS && my_addr) {
					m_l_state = PHI_L_LADS;
				}
			} else if (ag == IFCMD_TAG_VALUE) {
				// TAG
				if (!ton_msg()) {
					if (my_addr) {
						// MTA
						m_t_state = PHI_T_TADS;
					} else {
						// OTA
						m_t_state = PHI_T_TIDS;
					}
				}
			} else if (ag == IFCMD_SCG_VALUE) {
				// SCG
				if (m_pp_pacs) {
					if ((byte & IFCMD_PPX_MASK) == IFCMD_PPE_VALUE && m_pp_state == PHI_PP_PPIS) {
						// PPE
						m_s_sense = BIT(byte , IFCMD_PPE_S_BIT);
						m_ppr_msg = byte & IFCMD_PPE_PPR_MASK;
						m_pp_state = PHI_PP_PPSS;
					} else if ((byte & IFCMD_PPX_MASK) == IFCMD_PPD_VALUE && m_pp_state == PHI_PP_PPSS) {
						// PPD
						m_pp_state = PHI_PP_PPIS;
					}
				} else {
					// command is a secondary address
					if (m_t_state == PHI_T_ID1 && my_addr) {
						// Start IDENTIFY sequence
						m_t_state = PHI_T_ID2;
					} else if (m_t_state >= PHI_T_ID2 && m_t_state <= PHI_T_ID6 && !my_addr) {
						// Unaddressed by OSA
						m_t_state = PHI_T_ID1;
					} else if (m_t_state == PHI_T_TADS || m_l_state == PHI_L_LADS) {
						if (m_fifo_in.full() || BIT(m_reg_int_cond , REG_INT_DEV_CLEAR_BIT)) {
							// No room for secondary address in FIFO, stall handshake
							accepted = false;
						} else {
							uint16_t word = REG_IFIFO_2_ADDR_MASK | address;
							if (m_t_state == PHI_T_TADS) {
								BIT_SET(word, REG_IFIFO_TALK_BIT);
							}
							rx_n_data_freeze(word);
						}
					}
				}
			}
		}
	}
	return accepted;
}

bool phi_device::byte_received(uint8_t byte , bool eoi)
{
	// Start with D0/D1 = 00
	uint16_t word = byte;

	if (eoi) {
		// EOI -> D0/D1 = 11
		word |= REG_IFIFO_LAST_MASK;
	}

	bool end_of_transfer = false;

	if (!m_fifo_out.empty() && m_c_state == PHI_C_CSBS && m_t_state != PHI_T_TACS) {
		uint16_t be_word = m_fifo_out.peek();
		// Monitoring bytes being transferred on the bus
		if (eoi) {
			end_of_transfer = true;
		} else if (!BIT(be_word , REG_OFIFO_LF_INH_BIT) && byte == 0x0a) {
			// LF received -> D0/D1 = 11
			word |= REG_IFIFO_LAST_MASK;
			end_of_transfer = true;
		} else if (be_word != REG_OFIFO_UNCNT_MASK && ((m_be_counter + 1) & 0xff) == (be_word & 0xff)) {
			// Byte count expired -> D0/D1 = 10
			word |= REG_IFIFO_CNT_EXP_MASK;
			end_of_transfer = true;
		}
	}

	LOG(("RX word:%04x\n" , word));

	if (m_l_state == PHI_L_LACS) {
		if (m_fifo_in.full() || BIT(m_reg_int_cond , REG_INT_DEV_CLEAR_BIT)) {
			// No room for received byte, stall handshake
			LOG_0(("..stalled\n"));
			return false;
		} else {
			LOG_0(("..OK\n"));
			rx_n_data_freeze(word);
		}
	}
	if (end_of_transfer) {
		LOG_0(("End of byte transfer enable\n"));
		m_fifo_out.dequeue();
		m_be_counter = 0;
	} else {
		m_be_counter++;
	}

	return true;
}

void phi_device::rx_n_data_freeze(uint16_t word)
{
	m_fifo_in.enqueue(word);
	if (m_sh_state != PHI_SH_STRS) {
		// If PHI didn't send this byte to itself, set data freeze
		BIT_SET(m_reg_status, REG_STATUS_DATA_FREEZE_BIT);
	}
}

bool phi_device::ton_msg(void) const
{
	return BIT(m_reg_address , REG_ADDR_TA_BIT);
}

bool phi_device::lon_msg(void) const
{
	return BIT(m_reg_address , REG_ADDR_LA_BIT);
}

bool phi_device::odd_parity(uint8_t byte) const
{
	byte = (byte >> 4) ^ byte;
	byte = (byte >> 2) ^ byte;
	byte = (byte >> 1) ^ byte;
	return (byte & 1) != 0;
}

uint8_t phi_device::my_address(void) const
{
	if (m_sys_controller) {
		return CONTROLLER_ADDR;
	} else {
		return (m_reg_address >> REG_ADDR_HPIB_ADDR_BIT) & 0x1f;
	}
}

bool phi_device::tcs_msg(void) const
{
	uint8_t new_byte;
	bool new_eoi;

	// When the CIC takes back control synchronously:
	// * Nothing in OFIFO
	// * Request to start parallel poll is pending
	// * There's an interface command to be sent at head of OFIFO
	return (m_c_state == PHI_C_CSBS || m_c_state == PHI_C_CSHS || m_c_state == PHI_C_CSWS) &&
		(m_fifo_out.empty() ||
		 rpp_msg() ||
		 nba_msg(new_byte , new_eoi) == NBA_CMD_FROM_OFIFO);
}

bool phi_device::rpp_msg(void) const
{
	// TODO:
	return false;
}

bool phi_device::controller_in_charge(void) const
{
	return m_c_state != PHI_C_CIDS;
}
