// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

/*
 * NCR 5385 SCSI Protocol Controller
 *
 * Sources:
 *  - NCR 5385 SCSI Protocol Controller, 1983, NCR Corporation, Dayton, Ohio, USA
 *  - NCR SCSI Engineering Notebook, 1984, NCR Microelectronics
 *
 * TODO:
 *  - target mode send/receive
 *  - disconnect/reselection
 */

#include "emu.h"
#include "ncr5385.h"

#define LOG_GENERAL  (1U << 0)
#define LOG_REGW     (1U << 1)
#define LOG_REGR     (1U << 2)
#define LOG_STATE    (1U << 3)
#define LOG_DMA      (1U << 4)
#define LOG_COMMAND  (1U << 5)

#define VERBOSE (LOG_GENERAL|LOG_REGW|LOG_REGR|LOG_STATE|LOG_DMA|LOG_COMMAND)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(NCR5385, ncr5385_device, "ncr5385", "NCR 5385 SCSI Protocol Controller")

// FIXME: would be better to reuse from nscsi_full_device
unsigned constexpr SCSI_ARB_DELAY = 2'400;
unsigned constexpr SCSI_BUS_CLEAR = 800;
unsigned constexpr SCSI_BUS_FREE = 800;
unsigned constexpr SCSI_BUS_SETTLE = 400;
unsigned constexpr SCSI_BUS_SKEW = 10;
unsigned constexpr SCSI_RST_HOLD = 25'000;
unsigned constexpr SCSI_SEL_TIMEOUT = 250'000'000;

ncr5385_device::ncr5385_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: nscsi_device(mconfig, NCR5385, tag, owner, clock)
	, nscsi_slot_card_interface(mconfig, *this, DEVICE_SELF)
	, m_int(*this)
	, m_dreq(*this)
	, m_int_state(false)
	, m_dreq_state(false)
{
}

enum state : u32
{
	IDLE,
	DIAGNOSTIC,

	ARB_BUS_FREE,
	ARB_START,
	ARB_EVALUATE,
	SEL_START,
	SEL_DELAY,
	SEL_WAIT_BSY,
	SEL_COMPLETE,
	SEL_WAIT_REQ,

	XFI_START,
	XFI_IN_REQ,
	XFI_IN_DRQ,
	XFI_IN_ACK,
	XFI_OUT_REQ,
	XFI_OUT_DRQ,
	XFI_OUT_ACK,
	XFI_OUT_PAD,
};

enum mode : u8
{
	DISCONNECTED,
	INITIATOR,
	TARGET,
};

enum diag_mask : u8
{
	DIAG_SELF = 0x07, // self-diagnostic status
	DIAG_CMD  = 0x38, // diagnostic command status
	DIAG_DONE = 0x80, // self-diagnostic complete

	DIAG_CMD_GP = 0x18, // diagnostic good parity
	DIAG_CMD_BP = 0x20, // diagnostic bad parity
};

enum int_mask : u8
{
	INT_FUNC_COMPLETE = 0x01,
	INT_BUS_SERVICE   = 0x02,
	INT_DISCONNECTED  = 0x04,
	INT_SELECTED      = 0x08,
	INT_RESELECTED    = 0x10,
	INT_INVALID_CMD   = 0x40,
};

enum aux_status_mask : u8
{
	AUX_STATUS_TC_ZERO    = 0x02,
	AUX_STATUS_PAUSED     = 0x04,
	AUX_STATUS_IO         = 0x08,
	AUX_STATUS_CD         = 0x10,
	AUX_STATUS_MSG        = 0x20,
	AUX_STATUS_PARITY_ERR = 0x40,
	AUX_STATUS_DATA_FULL  = 0x80,
};

enum cmd_mask : u8
{
	CMD_INT = 0x08, // interrupting
	CMD_SBX = 0x40, // single byte transfer
	CMD_DMA = 0x80, // DMA mode
};

void ncr5385_device::device_start()
{
	save_item(NAME(m_dat));
	save_item(NAME(m_cmd));
	save_item(NAME(m_ctl));
	save_item(NAME(m_dst_id));
	save_item(NAME(m_aux_status));
	save_item(NAME(m_own_id));
	save_item(NAME(m_int_status));
	save_item(NAME(m_src_id));
	save_item(NAME(m_dia_status));
	save_item(NAME(m_cnt));

	save_item(NAME(m_state));
	save_item(NAME(m_phase));
	save_item(NAME(m_mode));
	save_item(NAME(m_sbx));

	save_item(NAME(m_int_state));
	save_item(NAME(m_dreq_state));

	m_state_timer = timer_alloc(timer_expired_delegate(FUNC(ncr5385_device::state_timer), this));

	m_dia_status = DIAG_DONE;
}

void ncr5385_device::device_reset()
{
	m_cmd = 0;
	m_ctl = 0;
	m_dst_id = 0;
	m_aux_status = AUX_STATUS_TC_ZERO;
	m_int_status = 0;
	m_src_id = 0;
	m_dia_status &= (DIAG_DONE | DIAG_SELF);
	m_cnt = 0;

	m_state = IDLE;
	m_mode = DISCONNECTED;
	m_sbx = false;

	// monitor all control lines (device has no RST line)
	scsi_bus->ctrl_wait(scsi_refid, S_ALL & ~S_RST, S_ALL & ~S_RST);

	update_int();
}

void ncr5385_device::scsi_ctrl_changed()
{
	u32 const ctrl = scsi_bus->ctrl_r();

	static char const *const nscsi_phase[] = { "DATA OUT", "DATA IN", "COMMAND", "STATUS", "*", "*", "MESSAGE OUT", "MESSAGE IN" };

	if ((ctrl & S_BSY) && !(ctrl & S_SEL))
	{
		LOGMASKED(LOG_STATE, "scsi_ctrl_changed 0x%03x phase %s%s%s pc(%s)\n", ctrl, nscsi_phase[ctrl & S_PHASE_MASK],
			ctrl & S_REQ ? " REQ" : "", ctrl & S_ACK ? " ACK" : "", machine().describe_context());

		if (m_state != IDLE)
			m_state_timer->adjust(attotime::zero);
	}
	else if (ctrl & S_BSY)
		LOGMASKED(LOG_STATE, "scsi_ctrl_changed 0x%03x arbitration/selection\n", ctrl);
	else
	{
		LOGMASKED(LOG_STATE, "scsi_ctrl_changed 0x%03x BUS FREE\n", ctrl);

		if (m_mode == INITIATOR)
		{
			m_mode = DISCONNECTED;
			m_int_status |= INT_DISCONNECTED;
			update_int();
		}
	}
}

void ncr5385_device::map(address_map &map)
{
	map(0x0, 0x0).rw(FUNC(ncr5385_device::dat_r), FUNC(ncr5385_device::dat_w));
	map(0x1, 0x1).rw(FUNC(ncr5385_device::cmd_r), FUNC(ncr5385_device::cmd_w));
	map(0x2, 0x2).rw(FUNC(ncr5385_device::ctl_r), FUNC(ncr5385_device::ctl_w));
	map(0x3, 0x3).rw(FUNC(ncr5385_device::dst_id_r), FUNC(ncr5385_device::dst_id_w));
	map(0x4, 0x4).r(FUNC(ncr5385_device::aux_status_r));
	map(0x5, 0x5).r(FUNC(ncr5385_device::own_id_r));
	map(0x6, 0x6).r(FUNC(ncr5385_device::int_status_r));
	map(0x7, 0x7).r(FUNC(ncr5385_device::src_id_r));
	map(0x9, 0x9).r(FUNC(ncr5385_device::dia_status_r));
	map(0xc, 0xc).rw(FUNC(ncr5385_device::cnt_r<2>), FUNC(ncr5385_device::cnt_w<2>));
	map(0xd, 0xd).rw(FUNC(ncr5385_device::cnt_r<1>), FUNC(ncr5385_device::cnt_w<1>));
	map(0xe, 0xe).rw(FUNC(ncr5385_device::cnt_r<0>), FUNC(ncr5385_device::cnt_w<0>));
	map(0xf, 0xf).rw(FUNC(ncr5385_device::tst_r), FUNC(ncr5385_device::tst_w));
}

u8 ncr5385_device::dat_r()
{
	if (m_aux_status & AUX_STATUS_DATA_FULL)
	{
		m_aux_status &= ~AUX_STATUS_DATA_FULL;

		if (m_state != IDLE)
			m_state_timer->adjust(attotime::zero);
	}
	else
		logerror("data register empty (%s)\n", machine().describe_context());

	return m_dat;
}

u8 ncr5385_device::cmd_r()
{
	return m_cmd;
}

u8 ncr5385_device::ctl_r()
{
	return m_ctl;
}

u8 ncr5385_device::dst_id_r()
{
	return m_dst_id;
}

u8 ncr5385_device::aux_status_r()
{
	u8 data = m_aux_status;

	if (!m_int_status)
	{
		// AB mask out any bits
		data &= ~(AUX_STATUS_MSG | AUX_STATUS_CD | AUX_STATUS_IO);

		// return current phase
		u32 const ctrl = scsi_bus->ctrl_r();
		if (ctrl & S_MSG)
			data |= AUX_STATUS_MSG;
		if (ctrl & S_CTL)
			data |= AUX_STATUS_CD;
		if (ctrl & S_INP)
			data |= AUX_STATUS_IO;
	}
	LOGMASKED(LOG_REGR, "aux_status_r 0x%02x (%s)\n", data, machine().describe_context());

	return data;
}

u8 ncr5385_device::own_id_r()
{
	return m_own_id;
}

u8 ncr5385_device::int_status_r()
{
	u8 const data = m_int_status;
	LOGMASKED(LOG_REGR, "int_status_r 0x%02x (%s)\n", data, machine().describe_context());
	m_aux_status &= ~AUX_STATUS_PARITY_ERR;
	m_int_status = 0;
	update_int();

	if (m_state != IDLE)
		m_state_timer->adjust(attotime::zero);

	return data;
}

u8 ncr5385_device::src_id_r()
{
	return m_src_id;
}

u8 ncr5385_device::dia_status_r()
{
	return m_dia_status;
}

template <unsigned N> u8 ncr5385_device::cnt_r()
{
	return u8(m_cnt >> (N * 8));
}

u8 ncr5385_device::tst_r()
{
	return 0;
}

void ncr5385_device::dat_w(u8 data)
{
	LOGMASKED(LOG_REGW, "dat_w 0x%02x (%s)\n", data, machine().describe_context());

	if (!(m_aux_status & AUX_STATUS_DATA_FULL))
	{
		m_dat = data;
		m_aux_status |= AUX_STATUS_DATA_FULL;

		if (m_state != IDLE)
			m_state_timer->adjust(attotime::zero);
	}
	else
		logerror("data register full\n");
}

void ncr5385_device::cmd_w(u8 data)
{
	LOGMASKED(LOG_REGW, "cmd_w 0x%02x (%s)\n", data, machine().describe_context());
	if (!(data & 0x18))
	{
		// immediate commands

		switch (data & 0x1f)
		{
		case 0x00:
			LOGMASKED(LOG_COMMAND, "reset\n");
			reset();
			break;
		case 0x01: // disconnect
			LOGMASKED(LOG_COMMAND, "disconnect\n");
			m_mode = DISCONNECTED;
			break;
		case 0x02: // pause
			LOGMASKED(LOG_COMMAND, "pause\n");
			break;
		case 0x03: // set atn
			LOGMASKED(LOG_COMMAND, "set atn\n");
			scsi_bus->ctrl_w(scsi_refid, S_ATN, S_ATN);
			break;
		case 0x04: // message accepted
			LOGMASKED(LOG_COMMAND, "message accepted\n");
			scsi_bus->ctrl_w(scsi_refid, 0, S_ACK);
			break;
		case 0x05: // chip disabled
			LOGMASKED(LOG_COMMAND, "chip disabled\n");
			break;
		case 0x06: case 0x07:
			// reserved
			break;
		}
	}
	else
	{
		// interrupting commands
		m_aux_status &= ~AUX_STATUS_DATA_FULL;
		m_cmd = data;

		switch (data & 0x1f)
		{
		case 0x08: // select w/atn
			LOGMASKED(LOG_COMMAND, "select %d w/atn (timeout %d)\n", m_dst_id, attotime::from_ticks(m_cnt * 1024, clock()).to_string());
			m_state = ARB_BUS_FREE;
			m_state_timer->adjust(attotime::zero);
			break;
		case 0x09: // select w/o atn
			LOGMASKED(LOG_COMMAND, "select %d w/o atn (timeout %d)\n", m_dst_id, attotime::from_ticks(m_cnt * 1024, clock()).to_string());
			m_state = ARB_BUS_FREE;
			m_state_timer->adjust(attotime::zero);
			break;
		case 0x0a: // reselect
			LOGMASKED(LOG_COMMAND, "reselect\n");
			break;
		case 0x0b: // diagnostic
			LOGMASKED(LOG_COMMAND, "diagnostic (%s parity)\n", BIT(data, 6) ? "bad" : "good");
			m_state = DIAGNOSTIC;
			break;
		case 0x0c: // receive command
			LOGMASKED(LOG_COMMAND, "receive command\n");
			break;
		case 0x0d: // receive data
			LOGMASKED(LOG_COMMAND, "receive data\n");
			break;
		case 0x0e: // receive message out
			LOGMASKED(LOG_COMMAND, "receive message out\n");
			break;
		case 0x0f: // receive unspecified info out
			LOGMASKED(LOG_COMMAND, "receive unspecified info out\n");
			break;
		case 0x10: // send status
			LOGMASKED(LOG_COMMAND, "send status\n");
			break;
		case 0x11: // send data
			LOGMASKED(LOG_COMMAND, "send data\n");
			break;
		case 0x12: // send message in
			LOGMASKED(LOG_COMMAND, "send message in\n");
			break;
		case 0x13: // send unspecified info in
			LOGMASKED(LOG_COMMAND, "send unspecified info in\n");
			break;
		case 0x14: // transfer info
			if (data & CMD_SBX)
				LOGMASKED(LOG_COMMAND, "transfer info (%s, single byte)\n", (data & CMD_DMA) ? "dma" : "pio");
			else
				LOGMASKED(LOG_COMMAND, "transfer info (%s, count=%d)\n", (data & CMD_DMA) ? "dma" : "pio", m_cnt);
			m_state = XFI_START;
			m_sbx = data & CMD_SBX;
			m_state_timer->adjust(attotime::zero);
			break;
		case 0x15: // transfer pad
			if (data & CMD_SBX)
				LOGMASKED(LOG_COMMAND, "transfer pad (%s, single byte)\n", (data & CMD_DMA) ? "dma" : "pio");
			else
				LOGMASKED(LOG_COMMAND, "transfer pad (%s, count=%d)\n", (data & CMD_DMA) ? "dma" : "pio", m_cnt);
			m_state = XFI_START;
			m_sbx = data & CMD_SBX;
			m_state_timer->adjust(attotime::zero);
			break;
		case 0x16: case 0x17:
		case 0x18: case 0x19: case 0x1a: case 0x1b:
		case 0x1c: case 0x1d: case 0x1e: case 0x1f:
			// reserved
			LOGMASKED(LOG_COMMAND, "reserved / invalid cmd\n");
			m_int_status |= INT_INVALID_CMD;
			update_int();
			break;
		}
	}
}

void ncr5385_device::ctl_w(u8 data)
{
	LOGMASKED(LOG_REGW, "ctl_w 0x%02x (%s)\n", data, machine().describe_context());

	m_ctl = data & 7;
}

void ncr5385_device::dst_id_w(u8 data)
{
	LOGMASKED(LOG_REGW, "dst_id_w 0x%02x (%s)\n", data, machine().describe_context());
	m_dst_id = (data & 7);
}

template <unsigned N> void ncr5385_device::cnt_w(u8 data)
{
	m_cnt = (m_cnt & ~(u32(0xff) << (N * 8))) | (u32(data) << (N * 8));

	if (m_cnt)
		m_aux_status &= ~AUX_STATUS_TC_ZERO;
	else
		m_aux_status |= AUX_STATUS_TC_ZERO;
}

void ncr5385_device::tst_w(u8 data)
{
	LOGMASKED(LOG_REGW, "tst_w 0x%02x (%s)\n", data, machine().describe_context());
}

u8 ncr5385_device::dma_r()
{
	u8 const data = m_dat;
	m_aux_status &= ~AUX_STATUS_DATA_FULL;

	set_dreq(false);
	m_state_timer->adjust(attotime::zero);

	return data;
}

void ncr5385_device::dma_w(u8 data)
{
	m_dat = data;
	m_aux_status |= AUX_STATUS_DATA_FULL;

	set_dreq(false);
	m_state_timer->adjust(attotime::zero);
}

void ncr5385_device::state_timer(s32 param)
{
	// step state machine
	int const delay = state_step();

	// check for data stall
	if (delay < 0)
		return;

	// repeat until idle
	if (m_state != IDLE)
		m_state_timer->adjust(attotime::from_nsec(delay));
}

int ncr5385_device::state_step()
{
	u32 const ctrl = scsi_bus->ctrl_r();
	int delay = 0;

	u8 const oid = 1 << m_own_id;
	u8 const tid = 1 << m_dst_id;

	switch (m_state)
	{
	case DIAGNOSTIC:
		m_dia_status &= DIAG_DONE | DIAG_SELF;
		if (BIT(m_cmd, 6))
		{
			m_aux_status |= AUX_STATUS_PARITY_ERR;
			m_dia_status |= DIAG_CMD_BP;
		}
		else
		{
			m_aux_status &= ~AUX_STATUS_PARITY_ERR;
			m_dia_status |= DIAG_CMD_GP;
		}

		m_int_status |= INT_FUNC_COMPLETE;
		m_state = IDLE;

		update_int();
		break;

	case ARB_BUS_FREE:
		LOGMASKED(LOG_STATE, "arbitration: waiting for bus free\n");
		if (!(ctrl & (S_SEL | S_BSY | S_RST)))
		{
			m_state = ARB_START;
			delay = SCSI_BUS_FREE;
		}
		break;
	case ARB_START:
		LOGMASKED(LOG_STATE, "arbitration: started\n");
		m_state = ARB_EVALUATE;
		delay = SCSI_ARB_DELAY;

		// assert own ID and BSY
		scsi_bus->data_w(scsi_refid, oid);
		scsi_bus->ctrl_w(scsi_refid, S_BSY, S_BSY);
		break;
	case ARB_EVALUATE:
		// check if SEL asserted, or if there's a higher ID on the bus
		if ((ctrl & S_SEL) || (scsi_bus->data_r() & ~((oid - 1) | oid)))
		{
			LOGMASKED(LOG_STATE, "arbitration: lost\n");
			m_state = ARB_BUS_FREE;

			// clear data and BSY
			scsi_bus->data_w(scsi_refid, 0);
			scsi_bus->ctrl_w(scsi_refid, 0, S_BSY);
		}
		else
		{
			LOGMASKED(LOG_STATE, "arbitration: won\n");
			m_state = SEL_START;
			delay = SCSI_BUS_CLEAR + SCSI_BUS_SETTLE;
		}
		break;

	case SEL_START:
		LOGMASKED(LOG_STATE, "selection: SEL asserted\n");
		m_state = SEL_DELAY;
		delay = SCSI_BUS_SKEW * 2;

		// assert own and target ID and SEL
		scsi_bus->data_w(scsi_refid, oid | tid);
		scsi_bus->ctrl_w(scsi_refid, S_SEL, S_SEL);
		break;
	case SEL_DELAY:
		LOGMASKED(LOG_STATE, "selection: BSY cleared\n");
		m_state = SEL_WAIT_BSY;
		delay = SCSI_SEL_TIMEOUT;

		// clear BSY, optionally assert ATN
		if (!BIT(m_cmd, 0))
			scsi_bus->ctrl_w(scsi_refid, S_ATN, S_BSY | S_ATN);
		else
			scsi_bus->ctrl_w(scsi_refid, 0, S_BSY);
		break;
	case SEL_WAIT_BSY:
		if (ctrl & S_BSY)
		{
			LOGMASKED(LOG_STATE, "selection: BSY asserted by target\n");
			m_state = SEL_COMPLETE;
			delay = SCSI_BUS_SKEW;
		}
		else
		{
			LOGMASKED(LOG_STATE, "selection: timed out\n");
			m_int_status |= INT_DISCONNECTED;
			m_state = IDLE;

			scsi_bus->ctrl_w(scsi_refid, 0, S_ATN | S_SEL);

			update_int();
		}
		break;
	case SEL_COMPLETE:
		LOGMASKED(LOG_STATE, "selection: complete\n");
		m_int_status |= INT_FUNC_COMPLETE;
		m_mode = INITIATOR;
		m_state = SEL_WAIT_REQ;
		delay = -1;

		update_int();

		// clear data and SEL
		scsi_bus->data_w(scsi_refid, 0);
		scsi_bus->ctrl_w(scsi_refid, 0, S_SEL);
		break;
	case SEL_WAIT_REQ:
		// don't generate bus service interrupt until the function complete is cleared
		if ((ctrl & S_REQ) && !m_int_state)
		{
			LOGMASKED(LOG_STATE, "selection: REQ asserted by target\n");
			m_int_status |= INT_BUS_SERVICE;
			m_state = IDLE;

			update_int();
		}
		else
			delay = -1;
		break;

	case XFI_START:
		m_phase = ctrl & S_PHASE_MASK;
		m_state = (ctrl & S_INP) ? XFI_IN_REQ : XFI_OUT_REQ;
		break;

	case XFI_IN_REQ:
		// TODO: disconnect
		if (ctrl & S_REQ)
		{
			if (remaining() && (ctrl & S_PHASE_MASK) == m_phase)
			{
				m_state = XFI_IN_DRQ;

				// transfer pad in doesn't transfer any data
				if (!BIT(m_cmd, 0))
				{
					m_aux_status |= AUX_STATUS_DATA_FULL;
					m_dat = scsi_bus->data_r();

					if (m_cmd & CMD_DMA)
						set_dreq(true);

					delay = -1;
				}
			}
			else
			{
				LOGMASKED(LOG_STATE, "xfi_in: %s\n", remaining() ? "phase change" : "transfer complete");

				m_int_status |= INT_BUS_SERVICE;
				m_state = IDLE;

				update_int();
			}
		}
		else
			delay = -1;
		break;
	case XFI_IN_DRQ:
		m_state = XFI_IN_ACK;

		LOGMASKED(LOG_STATE, "xfi_in: data 0x%02x\n", m_dat);

		// assert ACK
		scsi_bus->ctrl_w(scsi_refid, S_ACK, S_ACK);
		break;
	case XFI_IN_ACK:
		if (!(ctrl & S_REQ))
		{
			m_state = XFI_IN_REQ;

			if (!(m_cmd & CMD_SBX))
			{
				m_cnt--;

				LOGMASKED(LOG_STATE, "xfi_in: %d remaining\n", m_cnt);

				if (!m_cnt)
					m_aux_status |= AUX_STATUS_TC_ZERO;
			}
			else
				m_sbx = false;

			delay = 5'000;	// AB 5ns, does this help??
			
			// clear ACK except after last byte of message input phase
			if (!remaining() && (ctrl & S_PHASE_MASK) == S_PHASE_MSG_IN)
			{
				LOGMASKED(LOG_STATE, "xfi_in: INT_FUNC_COMPLETE\n" );
			
				m_int_status |= INT_FUNC_COMPLETE;
				m_state = IDLE;

				update_int();
			}
			else
				scsi_bus->ctrl_w(scsi_refid, 0, S_ACK);
		}
		else
			delay = -1;
		break;

	case XFI_OUT_REQ:
		if (ctrl & S_REQ)
		{
			// TODO: disconnect
			if (remaining() && (ctrl & S_PHASE_MASK) == m_phase)
			{
				m_state = XFI_OUT_DRQ;

				// FIXME: only one byte dma for transfer pad
				if (m_cmd & CMD_DMA)
					set_dreq(true);

				if (!(m_aux_status & AUX_STATUS_DATA_FULL))
					delay = -1;
			}
			else
			{
				LOGMASKED(LOG_STATE, "xfi_out: %s pc(%s)\n", remaining() ? "phase change" : "transfer complete", machine().describe_context());

				m_int_status |= INT_BUS_SERVICE;
				m_state = IDLE;

				update_int();
			}
		}
		else
			delay = -1;
		break;

	case XFI_OUT_DRQ:
		m_state = XFI_OUT_ACK;
		m_aux_status &= ~AUX_STATUS_DATA_FULL;

		LOGMASKED(LOG_STATE, "xfi_out: data 0x%02x\n", m_dat);

		// assert data and ACK
		scsi_bus->data_w(scsi_refid, m_dat);
		if (remaining(1) && (ctrl & S_PHASE_MASK) == S_PHASE_MSG_OUT)
			scsi_bus->ctrl_w(scsi_refid, S_ACK, S_ACK | S_ATN);
		else
			scsi_bus->ctrl_w(scsi_refid, S_ACK, S_ACK);
		break;
	case XFI_OUT_ACK:
		if (!(ctrl & S_REQ))
		{
			if (BIT(m_cmd, 0))
				m_state = XFI_OUT_PAD;
			else
				m_state = XFI_OUT_REQ;

			if (!(m_cmd & CMD_SBX))
			{
				m_cnt--;

				LOGMASKED(LOG_STATE, "xfi_out: %d remaining\n", m_cnt);

				if (!m_cnt)
					m_aux_status |= AUX_STATUS_TC_ZERO;
			}
			else
				m_sbx = false;

			delay = 3'500;		// >=3.5us delay works, < 3.5us fails
			
			// clear data and ACK
			scsi_bus->data_w(scsi_refid, 0);
			scsi_bus->ctrl_w(scsi_refid, 0, S_ACK);

		}
		else
			delay = -1;
		break;
	case XFI_OUT_PAD:
		if (ctrl & S_REQ)
		{
			// TODO: disconnect
			if (remaining() && (ctrl & S_PHASE_MASK) == m_phase)
				m_state = XFI_OUT_DRQ;
			else
			{
				LOGMASKED(LOG_STATE, "xfi_out: %s\n", remaining() ? "phase change" : "transfer complete");
				m_int_status |= INT_BUS_SERVICE;
				m_state = IDLE;

				update_int();
			}
		}
		break;
	}

	return delay;
}

/*
 * Returns a boolean indicating whether any data remains to be transferred:
 *
 *  - for single byte transfer commands, m_sbx indicates data remaining
 *  - alternatively, return if there's a specific amount of data to transfer
 *  - otherwise, check if there's any data to transfer
 */
bool ncr5385_device::remaining(u32 const count) const
{
	if (m_cmd & CMD_SBX)
		return m_sbx;
	else if (count)
		return m_cnt == count;
	else
		return m_cnt;
}

void ncr5385_device::set_dreq(bool dreq)
{
	if (m_dreq_state != dreq)
	{
		LOGMASKED(LOG_DMA, "set_dreq %d\n", dreq);

		m_dreq_state = dreq;
		m_dreq(m_dreq_state);
	}
}

void ncr5385_device::update_int()
{
	bool const int_state = m_int_status & (INT_FUNC_COMPLETE | INT_BUS_SERVICE |
			INT_DISCONNECTED | INT_SELECTED | INT_RESELECTED | INT_INVALID_CMD);

	LOGMASKED(LOG_COMMAND, "update_int 0x%02x (0x%02x)  pc(%s)\n", m_int_status, m_int_state, machine().describe_context());
	if (m_int_state != int_state)
	{

		if (int_state)
		{
			m_cmd = 0;

		// AB should we not clear out existing?
		m_aux_status &= ~(AUX_STATUS_MSG | AUX_STATUS_CD | AUX_STATUS_IO);

			// latch current phase
			u32 const ctrl = scsi_bus->ctrl_r();
			if (ctrl & S_MSG)
				m_aux_status |= AUX_STATUS_MSG;
			if (ctrl & S_CTL)
				m_aux_status |= AUX_STATUS_CD;
			if (ctrl & S_INP)
				m_aux_status |= AUX_STATUS_IO;
		}

		m_int_state = int_state;
		m_int(m_int_state);
	}
}
