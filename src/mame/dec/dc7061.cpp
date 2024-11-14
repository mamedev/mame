// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * DEC DC7061 SII SCSI gate array.
 *
 * Sources:
 *  - DECstation 3100 Desktop Workstation Functional Specification, Revision 1.3, August 28, 1990, Workstation Systems Engineering, Digital Equipment Corporation
 *
 * TODO:
 *  - everything, stub only
 */
/*
 * WIP notes:
 * boot -f rz(0,4,0)vmunix
 *
    [:scsi:7:sii] csr_w 0x0010 (':cpu' (BFC12F24))    # enable bus arbitration
    [:scsi:7:sii] id_w 0x8006 (':cpu' (BFC12F8C))     # set scsi id
    [:scsi:7:sii] dmctrl_w 0x0000 (':cpu' (BFC12F94)) # clear req/ack offset
    [:scsi:7:sii] cstat_w 0x3080 (':cpu' (BFC12FA0))  # clear rst|ber|sch
    [:scsi:7:sii] dictrl_w 0x0004 (':cpu' (BFC12FAC)) # enable port
    [:scsi:7:sii] comm_w 0x4000 (':cpu' (BFC12FB8))   # scsi reset
    [:scsi:7:sii] cstat_w 0x2080 (':cpu' (BFC12FE0))  # clear rst|sch
    [:scsi:7:sii] csr_w 0x001f (':cpu' (BFC12FF4))    # enable reseletion, selection, parity and interrupts
    [:scsi:7:sii] cstat_w 0x0300 (':cpu' (BFC12304))  # clear rst|ber
    [:scsi:7:sii] slcsr_w 0x0004 (':cpu' (BFC12400))  # target id 4
    [:scsi:7:sii] comm_w 0x0400 (':cpu' (BFC1240C))   # select
    [:scsi:7:sii] comm_w 0x0100 (':cpu' (BFC12AE0))   # disconnect
    [:] control_w 0x0040
    [:scsi:7:sii] comm_w 0x0000 (':cpu' (BFC12B3C))   # no command?
    [:] control_w 0x002c
    [:scsi:7:sii] cstat_w 0x0300 (':cpu' (BFC12304))  # clear rst|ber
    [:scsi:7:sii] slcsr_w 0x0004 (':cpu' (BFC12400))  # target id 4
    [:scsi:7:sii] comm_w 0x0400 (':cpu' (BFC1240C))   # select
    [:scsi:7:sii] comm_w 0x0100 (':cpu' (BFC12AE0))   # disconnect
    [:] control_w 0x0040
    [:scsi:7:sii] comm_w 0x0000 (':cpu' (BFC12B3C))   # no command?
 */

#include "emu.h"
#include "dc7061.h"

#define LOG_REGW     (1U << 1)
#define LOG_REGR     (1U << 2)
#define LOG_SCSI     (1U << 3)
#define LOG_ARB      (1U << 4)
#define LOG_SEL      (1U << 5)
#define LOG_PIO      (1U << 6)
#define LOG_DMA      (1U << 7)

//#define VERBOSE (LOG_GENERAL|LOG_REGW|LOG_REGR|LOG_SCSI|LOG_ARB|LOG_SEL|LOG_PIO|LOG_DMA)
#include "logmacro.h"

enum sdb_mask : u16
{
	SDB_DATA = 0x00ff,
	SDB_PTY  = 0x0100,
	SDB_MSK  = 0x01ff,
};
enum sc1_mask : u16
{
	SC1_IO  = 0x0001,
	SC1_CD  = 0x0002,
	SC1_MSG = 0x0004,
	SC1_ATN = 0x0008,
	SC1_REQ = 0x0010,
	SC1_ACK = 0x0020,
	SC1_RST = 0x0040,
	SC1_SEL = 0x0080,
	SC1_BSY = 0x0100,
	SC1_MSK = 0x01ff,
};
enum sc2_mask : u16
{
	SC2_SBE = 0x0001, // drive scsi data bus and parity lines
	SC2_ARB = 0x0002, // enable scsi drivers for arbitration
	SC2_TGS = 0x0004, // steer scsi drivers for target role
	SC2_IGS = 0x0008, // steer scsi drivers for initiator role
	SC2_MSK = 0x000f,
};
enum csr_mask : u16
{
	CSR_IE  = 0x0001, // interrupt enable
	CSR_PCE = 0x0002, // parity check enable
	CSR_SLE = 0x0004, // selection enable
	CSR_RSE = 0x0008, // reselection enable
	CSR_HPM = 0x0010, // host bus arbitration enable
	CSR_MSK = 0x001f,
};
enum id_mask : u16
{
	ID_ID  = 0x0007, // bus id
	ID_IO  = 0x8000, // id output enable
	ID_MSK = 0x8007,
};
enum slcsr_mask : u16
{
	SLCSR_ID  = 0x0007, // select/reselect destination id
	SLCSR_MSK = 0x0007,
};
enum destat_mask : u16
{
	DESTAT_ID  = 0x0007, // select source id
	DESTAT_MSK = 0x0007,
};
enum dmctrl_mask : u16
{
	DMCTRL_RAO = 0x0003, // req/ack offset
	DMCTRL_MSK = 0x0003,
};
enum dmlotc_mask : u16
{
	DMLOTC_CNT = 0x1fff, // transfer count
	DMLOTC_MSK = 0x1fff,
};
enum dmaddrh_mask : u16
{
	DMADDRH_ADR = 0x0003, // dma address high
	DMADDRH_MSK = 0x0003,
};
enum cstat_mask : u16
{
						// * indicates interrupt source
	CSTAT_LST = 0x0002, // lost arbitration (r/o)
	CSTAT_SIP = 0x0004, // selection in progress (r/o)
	CSTAT_SWA = 0x0008, // selected with atn (w1c)
	CSTAT_TGT = 0x0010, // target (r/o)
	CSTAT_DST = 0x0020, // destination (r/o)
	CSTAT_CON = 0x0040, // connected (r/o)
	CSTAT_SCH = 0x0080, // state change (w1c, *)
	CSTAT_LDN = 0x0100, // unused (w1c?, *)
	CSTAT_BUF = 0x0200, // unused (w1c?, *)
	CSTAT_TZ  = 0x0400, // unused
	CSTAT_OBC = 0x0800, // unused (w1c?, *)
	CSTAT_BER = 0x1000, // bus error (w1c, *)
	CSTAT_RST = 0x2000, // rst asserted (w1c, *)
	CSTAT_DI  = 0x4000, // dstat interrupt (r/o)
	CSTAT_CI  = 0x8000, // cstat interrupt (r/o)
	CSTAT_MSK = 0x3f88,
	CSTAT_W1C = 0x3b88,
};
enum dstat_mask : u16
{
						// * indicates interrupt source
	DSTAT_IO  = 0x0001, // i/o asserted (r/o)
	DSTAT_CD  = 0x0002, // c/d asserted (r/o)
	DSTAT_MSG = 0x0004, // msg asserted (r/o)
	DSTAT_ATN = 0x0008, // atn asserted (w1c)
	DSTAT_MIS = 0x0010, // phase mismatch (r/o?, *)
	DSTAT_OBB = 0x0100, // odd byte boundary (r/o?)
	DSTAT_IPE = 0x0200, // incoming parity error (r/o?)
	DSTAT_IBF = 0x0400, // input buffer full (r/o?, *)
	DSTAT_TBE = 0x0800, // transmit buffer empty (r/o?, *)
	DSTAT_TCZ = 0x1000, // transfer counter zero (r/o?)
	DSTAT_DNE = 0x2000, // transfer done (w1c, *)
	DSTAT_DI  = 0x4000, // dstat interrupt (r/o)
	DSTAT_CI  = 0x8000, // cstat interrupt (r/o)
	DSTAT_MSK = 0x2008,
	DSTAT_W1C = 0x2008,
};
enum comm_mask : u16
{
	COMM_IO  = 0x0001, // assert i/o (t)
	COMM_CD  = 0x0002, // assert c/d (t)
	COMM_MSG = 0x0004, // assert msg (t)
	COMM_ATN = 0x0008, // assert atn (i)
	COMM_TGT = 0x0010, // expect target
	COMM_DST = 0x0020, // expect destination
	COMM_CON = 0x0040, // expect connected
	COMM_CMD = 0x0f80, // command
	COMM_RSL = 0x1000, // attempt reselection
	COMM_RST = 0x4000, // assert rst
	COMM_DMA = 0x8000, // dma enable
	COMM_MSK = 0x9fff,
	COMM_PHM = 0x0007,
};
enum comm_cmd_mask : u16
{
	CMD_RST = 0x0080, // chip reset
	CMD_DSC = 0x0100, // disconnect
	CMD_REQ = 0x0200, // request data
	CMD_SEL = 0x0400, // select
	CMD_XFR = 0x0800, // information transfer
};
enum dictrl_mask : u16
{
	DICTRL_TST = 0x0001, // test mode
	DICTRL_DIA = 0x0002, // external loopback
	DICTRL_PRE = 0x0004, // port enable
	DICTRL_LPB = 0x0008, // internal loopback
	DICTRL_MSK = 0x000f,
};

enum state : u32
{
	IDLE,

	// arbitration
	ARB_BUS_FREE,
	ARB_START,
	ARB_EVALUATE,

	// selection
	SEL_START,
	SEL_DELAY,
	SEL_WAIT_BSY,
	SEL_COMPLETE,

	XFR_DMA,
	XFR_DMA_IN_REQ,
	XFR_DMA_IN_ACK,
	XFR_DMA_OUT,

	XFR_PIO,
	XFR_PIO_IN,
	XFR_PIO_OUT,
	XFR_PIO_OUT_ACK,

	// dma transfer
	DMA_IN_REQ,
	DMA_IN_ACK,
	DMA_OUT_REQ,
	DMA_OUT_DRQ,
	DMA_OUT_ACK,
};

static unsigned const SCSI_ARB_DELAY  =  2'400;
static unsigned const SCSI_BUS_CLEAR  =    800;
static unsigned const SCSI_BUS_FREE   =    800;
static unsigned const SCSI_BUS_SETTLE =    400;
static unsigned const SCSI_BUS_SKEW   =     10;
static unsigned const SCSI_RST_HOLD   = 25'000;

DEFINE_DEVICE_TYPE(DC7061, dc7061_device, "dc7061", "DEC DC7061 SII")

dc7061_device::dc7061_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: nscsi_device(mconfig, DC7061, tag, owner, clock)
	, nscsi_slot_card_interface(mconfig, *this, DEVICE_SELF)
	, m_sys_int(*this)
	, m_dma_r(*this, 0)
	, m_dma_w(*this)
{
}

void dc7061_device::map(address_map &map)
{
	map(0x00, 0x01).rw(FUNC(dc7061_device::sdb_r), FUNC(dc7061_device::sdb_w));
	map(0x02, 0x03).rw(FUNC(dc7061_device::sc1_r), FUNC(dc7061_device::sc1_w));
	map(0x04, 0x05).rw(FUNC(dc7061_device::sc2_r), FUNC(dc7061_device::sc2_w));
	map(0x06, 0x07).rw(FUNC(dc7061_device::csr_r), FUNC(dc7061_device::csr_w));
	map(0x08, 0x09).rw(FUNC(dc7061_device::id_r), FUNC(dc7061_device::id_w));
	map(0x0a, 0x0b).rw(FUNC(dc7061_device::slcsr_r), FUNC(dc7061_device::slcsr_w));
	map(0x0c, 0x0d).r(FUNC(dc7061_device::destat_r));
	//map(0x0e, 0x0f).rw(FUNC(dc7061_device::dstmo_r), FUNC(dc7061_device::dstmo_w));
	map(0x10, 0x11).rw(FUNC(dc7061_device::data_r), FUNC(dc7061_device::data_w));
	map(0x12, 0x13).rw(FUNC(dc7061_device::dmctrl_r), FUNC(dc7061_device::dmctrl_w));
	map(0x14, 0x15).rw(FUNC(dc7061_device::dmlotc_r), FUNC(dc7061_device::dmlotc_w));
	map(0x16, 0x17).rw(FUNC(dc7061_device::dmaddrl_r), FUNC(dc7061_device::dmaddrl_w));
	map(0x18, 0x19).rw(FUNC(dc7061_device::dmaddrh_r), FUNC(dc7061_device::dmaddrh_w));
	map(0x1a, 0x1b).rw(FUNC(dc7061_device::dmabyte_r), FUNC(dc7061_device::dmabyte_w));
	//map(0x1c, 0x1d).rw(FUNC(dc7061_device::stlp_r), FUNC(dc7061_device::stlp_w));
	//map(0x1e, 0x1f).rw(FUNC(dc7061_device::ltlp_r), FUNC(dc7061_device::ltlp_w));
	//map(0x20, 0x21).rw(FUNC(dc7061_device::ilp_r), FUNC(dc7061_device::ilp_w));
	//map(0x22, 0x23).rw(FUNC(dc7061_device::dsctrl_r), FUNC(dc7061_device::dsctrl_w));
	map(0x24, 0x25).rw(FUNC(dc7061_device::cstat_r), FUNC(dc7061_device::cstat_w));
	map(0x26, 0x27).rw(FUNC(dc7061_device::dstat_r), FUNC(dc7061_device::dstat_w));
	map(0x28, 0x29).rw(FUNC(dc7061_device::comm_r), FUNC(dc7061_device::comm_w));
	map(0x2a, 0x2b).rw(FUNC(dc7061_device::dictrl_r), FUNC(dc7061_device::dictrl_w));
}

void dc7061_device::device_start()
{
	m_state_timer = timer_alloc(timer_expired_delegate(FUNC(dc7061_device::state_timer), this));

	save_item(NAME(m_state));

	save_item(NAME(m_sdb));
	save_item(NAME(m_sc1));
	save_item(NAME(m_sc2));
	save_item(NAME(m_csr));
	save_item(NAME(m_id));
	save_item(NAME(m_slcsr));
	save_item(NAME(m_destat));
	save_item(NAME(m_data));
	save_item(NAME(m_dmctrl));
	save_item(NAME(m_dmlotc));
	save_item(NAME(m_dmaddrl));
	save_item(NAME(m_dmaddrh));
	save_item(NAME(m_dmabyte));
	save_item(NAME(m_cstat));
	save_item(NAME(m_dstat));
	save_item(NAME(m_comm));
	save_item(NAME(m_dictrl));

	save_item(NAME(m_scsi_ctrl));
	save_item(NAME(m_sys_int_state));
}

void dc7061_device::device_reset()
{
	m_state = IDLE;

	// clear scsi bus
	scsi_bus->data_w(scsi_refid, 0);
	scsi_bus->ctrl_w(scsi_refid, 0, S_ALL);

	// monitor all control lines
	m_scsi_ctrl = 0;
	scsi_bus->ctrl_wait(scsi_refid, S_ALL, S_ALL);

	// clear output lines
	set_irq(false);
}

void dc7061_device::scsi_ctrl_changed()
{
	u32 const ctrl = scsi_bus->ctrl_r();

	if (VERBOSE & LOG_SCSI)
	{
		static char const *const nscsi_phase[] = { "DATA OUT", "DATA IN", "COMMAND", "STATUS", "*", "*", "MESSAGE OUT", "MESSAGE IN" };

		if (ctrl & S_RST)
			LOGMASKED(LOG_SCSI, "scsi_ctrl_changed 0x%x BUS RESET\n", ctrl);
		else if ((ctrl & S_BSY) && !(ctrl & S_SEL))
			LOGMASKED(LOG_SCSI, "scsi_ctrl_changed 0x%x phase %s%s%s\n", ctrl, nscsi_phase[ctrl & S_PHASE_MASK],
				ctrl & S_REQ ? " REQ" : "", ctrl & S_ACK ? " ACK" : "");
		else if (ctrl & S_BSY)
			LOGMASKED(LOG_SCSI, "scsi_ctrl_changed 0x%x arbitration/selection\n", ctrl);
		else
			LOGMASKED(LOG_SCSI, "scsi_ctrl_changed 0x%x BUS FREE\n", ctrl);
	}

	if (ctrl & S_RST)
	{
		LOG("scsi reset received\n");
		device_reset();

		set_irq(true);
	}
	else if (!(m_cstat & CSTAT_TGT) && (m_scsi_ctrl & S_BSY) && !(ctrl & S_BSY))
	{
		LOG("target disconnected\n");
		m_state = IDLE;
		m_state_timer->enable(false);

		// clear scsi bus
		scsi_bus->data_w(scsi_refid, 0);
		scsi_bus->ctrl_w(scsi_refid, 0, S_ALL);
	}
	else if (!(m_scsi_ctrl & S_REQ) && (ctrl & S_REQ))
	{
		// target asserted REQ
		if ((ctrl & S_PHASE_MASK) == (m_comm & COMM_PHM))
		{
			if (m_comm & COMM_DMA)
			{
				// transfer cycle
				if (m_state != IDLE && !m_state_timer->enabled())
					m_state_timer->adjust(attotime::zero);
			}
			else
				set_dstat(DSTAT_TBE, DSTAT_TBE);
		}
		else
		{
			LOG("phase mismatch %d != %d\n", ctrl & S_PHASE_MASK, m_comm & COMM_PHM);

			m_state = IDLE;
			m_state_timer->enable(false);

			if (ctrl & S_INP)
				set_dstat(DSTAT_MIS | DSTAT_TBE, DSTAT_MIS | DSTAT_TBE);
			else
				set_dstat(DSTAT_MIS, DSTAT_MIS);
		}
	}

	m_scsi_ctrl = ctrl;
}

void dc7061_device::set_cstat(u16 data, u16 mask)
{
	m_cstat = (m_cstat & ~mask) | (data & mask);

	if (m_cstat & (CSTAT_RST | CSTAT_BER | CSTAT_OBC | CSTAT_BUF | CSTAT_LDN | CSTAT_SCH))
	{
		m_dstat |= DSTAT_CI;
		m_cstat |= CSTAT_CI;
	}
	else
	{
		m_dstat &= ~DSTAT_CI;
		m_cstat &= ~CSTAT_CI;
	}

	set_irq(m_cstat & (CSTAT_CI | CSTAT_DI));
}

void dc7061_device::set_dstat(u16 data, u16 mask)
{
	m_dstat = (m_dstat & ~mask) | (data & mask);

	if (m_dstat & (DSTAT_DNE | DSTAT_TBE | DSTAT_IBF | DSTAT_MIS))
	{
		m_dstat |= DSTAT_DI;
		m_cstat |= CSTAT_DI;
	}
	else
	{
		m_dstat &= ~DSTAT_DI;
		m_cstat &= ~CSTAT_DI;
	}

	set_irq(m_dstat & (DSTAT_CI | DSTAT_DI));
}

void dc7061_device::set_irq(bool irq_state)
{
	if (irq_state != m_sys_int_state)
	{
		LOG("set_irq %d cstat 0x%04x dstat 0x%04x\n", irq_state, m_cstat, m_dstat);

		m_sys_int_state = irq_state;
		m_sys_int(m_sys_int_state);
	}
}

u16 dc7061_device::sdb_r() { LOGMASKED(LOG_REGR, "sdb_r 0x%04x (%s)\n", m_sdb, machine().describe_context()); return m_sdb; }
u16 dc7061_device::sc1_r() { LOGMASKED(LOG_REGR, "sc1_r 0x%04x (%s)\n", m_sc1, machine().describe_context()); return m_sc1; }
u16 dc7061_device::sc2_r() { LOGMASKED(LOG_REGR, "sc2_r 0x%04x (%s)\n", m_sc2, machine().describe_context()); return m_sc2; }
u16 dc7061_device::csr_r() { LOGMASKED(LOG_REGR, "csr_r 0x%04x (%s)\n", m_csr, machine().describe_context()); return m_csr; }
u16 dc7061_device::id_r() { LOGMASKED(LOG_REGR, "id_r 0x%04x (%s)\n", m_id, machine().describe_context()); return m_id; }
u16 dc7061_device::slcsr_r() { LOGMASKED(LOG_REGR, "slcsr_r 0x%04x (%s)\n", m_slcsr, machine().describe_context()); return m_slcsr; }
u16 dc7061_device::destat_r() { LOGMASKED(LOG_REGR, "destat_r 0x%04x (%s)\n", m_destat, machine().describe_context()); return m_destat; }
u16 dc7061_device::data_r() { LOGMASKED(LOG_REGR, "data_r 0x%04x (%s)\n", m_data, machine().describe_context()); return m_data; }
u16 dc7061_device::dmctrl_r() { LOGMASKED(LOG_REGR, "dmctrl_r 0x%04x (%s)\n", m_dmctrl, machine().describe_context()); return m_dmctrl; }
u16 dc7061_device::dmlotc_r() { LOGMASKED(LOG_REGR, "dmlotc_r 0x%04x (%s)\n", m_dmlotc, machine().describe_context()); return m_dmlotc; }
u16 dc7061_device::dmaddrl_r() { LOGMASKED(LOG_REGR, "dmaddrl_r 0x%04x (%s)\n", m_dmaddrl, machine().describe_context()); return m_dmaddrl; }
u16 dc7061_device::dmaddrh_r() { LOGMASKED(LOG_REGR, "dmaddrh_r 0x%04x (%s)\n", m_dmaddrh, machine().describe_context()); return m_dmaddrh; }
u16 dc7061_device::dmabyte_r() { LOGMASKED(LOG_REGR, "dmabyte_r 0x%04x (%s)\n", m_dmabyte, machine().describe_context()); return m_dmabyte; }
u16 dc7061_device::cstat_r() { LOGMASKED(LOG_REGR, "cstat_r 0x%04x (%s)\n", m_cstat, machine().describe_context()); return m_cstat; }

u16 dc7061_device::dstat_r()
{
	u16 const data = m_dstat | (scsi_bus->ctrl_r() & S_PHASE_MASK);

	LOGMASKED(LOG_REGR, "dstat_r 0x%04x (%s)\n", data, machine().describe_context());

	return data;
}

u16 dc7061_device::comm_r() { logerror("comm_r 0x%04x (%s)\n", m_comm, machine().describe_context()); return m_comm; }
u16 dc7061_device::dictrl_r() { logerror("dictrl_r 0x%04x (%s)\n", m_dictrl, machine().describe_context()); return m_dictrl; }

void dc7061_device::sdb_w(u16 data) { LOGMASKED(LOG_REGW, "sdb_w 0x%04x (%s)\n", data, machine().describe_context()); m_sdb = data & SDB_MSK; }
void dc7061_device::sc1_w(u16 data) { LOGMASKED(LOG_REGW, "sc1_w 0x%04x (%s)\n", data, machine().describe_context()); m_sc1 = data & SC1_MSK; }
void dc7061_device::sc2_w(u16 data) { LOGMASKED(LOG_REGW, "sc2_w 0x%04x (%s)\n", data, machine().describe_context()); m_sc2 = data & SC2_MSK; }
void dc7061_device::csr_w(u16 data) { LOGMASKED(LOG_REGW, "csr_w 0x%04x (%s)\n", data, machine().describe_context()); m_csr = data & CSR_MSK; }
void dc7061_device::id_w(u16 data) { LOGMASKED(LOG_REGW, "src id %d (%s)\n", data & ID_ID, machine().describe_context()); m_id = data & ID_MSK; }
void dc7061_device::slcsr_w(u16 data) { LOGMASKED(LOG_REGW, "dst id %d (%s)\n", data & SLCSR_ID, machine().describe_context()); m_slcsr = data & SLCSR_MSK; }

void dc7061_device::data_w(u16 data)
{
	LOGMASKED(LOG_REGW, "data_w 0x%04x (%s)\n", data, machine().describe_context());
	m_data = u8(data);
}

void dc7061_device::dmctrl_w(u16 data) { LOGMASKED(LOG_REGW, "dmctrl_w 0x%04x (%s)\n", data, machine().describe_context()); m_dmctrl = data & DMCTRL_MSK; }
void dc7061_device::dmlotc_w(u16 data) { LOGMASKED(LOG_REGW, "dmlotc_w 0x%04x (%s)\n", data, machine().describe_context()); m_dmlotc = data & DMLOTC_MSK; }
void dc7061_device::dmaddrl_w(u16 data) { LOGMASKED(LOG_REGW, "dmaddrl_w 0x%04x (%s)\n", data, machine().describe_context()); m_dmaddrl = data; }
void dc7061_device::dmaddrh_w(u16 data) { LOGMASKED(LOG_REGW, "dmaddrh_w 0x%04x (%s)\n", data, machine().describe_context()); m_dmaddrh = data & DMADDRH_MSK; }
void dc7061_device::dmabyte_w(u16 data) { LOGMASKED(LOG_REGW, "dmabyte_w 0x%04x (%s)\n", data, machine().describe_context()); m_dmabyte = u8(data); }

void dc7061_device::cstat_w(u16 data)
{
	LOGMASKED(LOG_REGW, "cstat_w 0x%04x (%s)\n", data, machine().describe_context());

	m_cstat = (m_cstat & (~CSTAT_MSK & ~(data & CSTAT_W1C))) | (data & (CSTAT_MSK & ~CSTAT_W1C));

	set_cstat(0, 0);
}

void dc7061_device::dstat_w(u16 data)
{
	LOGMASKED(LOG_REGW, "dstat_w 0x%04x (%s)\n", data, machine().describe_context());

	m_dstat = (m_dstat & (~DSTAT_MSK & ~(data & DSTAT_W1C))) | (data & (DSTAT_MSK & ~DSTAT_W1C));

	set_dstat(0, 0);
}

void dc7061_device::comm_w(u16 data)
{
	LOGMASKED(LOG_REGW, "comm_w 0x%04x (%s)\n", data, machine().describe_context());
	m_comm = data & COMM_MSK;

	set_dstat((scsi_bus->ctrl_r() & S_PHASE_MASK) == (m_comm & COMM_PHM) ? 0 : DSTAT_MIS, DSTAT_MIS);

	switch (data & COMM_CMD)
	{
	case CMD_RST:
		LOG("chip reset\n");
		reset();
		break;
	case CMD_DSC:
		LOG("disconnect\n");
		set_cstat(0, CSTAT_CON | CSTAT_SIP | CSTAT_LST); // CHECK:

		scsi_bus->data_w(scsi_refid, 0);
		scsi_bus->ctrl_w(scsi_refid, 0, S_ATN | S_SEL | S_BSY); // CHECK:
		break;
	case CMD_REQ:
		LOG("request data\n");
		break;
	case CMD_SEL:
		LOG("select id %d\n", m_slcsr & SLCSR_ID);
		m_state = ARB_BUS_FREE;
		m_state_timer->adjust(attotime::zero);
		break;
	case CMD_XFR:
		LOG("information transfer\n");
		m_state = (m_comm & COMM_DMA) ? XFR_DMA : XFR_PIO;
		m_state_timer->adjust(attotime::zero);
		break;
	}
}

void dc7061_device::dictrl_w(u16 data) { LOGMASKED(LOG_REGW, "dictrl_w 0x%04x (%s)\n", data, machine().describe_context()); m_dictrl = data & DICTRL_MSK; }

void dc7061_device::state_timer(s32 param)
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

int dc7061_device::state_step()
{
	int delay = 0;

	u8 const oid = 1 << m_id;
	u8 const tid = 1 << m_slcsr;

	switch (m_state)
	{
	case IDLE:
		break;

	case ARB_BUS_FREE:
		LOGMASKED(LOG_ARB, "arbitration: waiting for bus free\n");
		if (!(scsi_bus->ctrl_r() & (S_SEL | S_BSY | S_RST)))
		{
			m_state = ARB_START;
			delay = SCSI_BUS_FREE;
		}
		break;
	case ARB_START:
		LOGMASKED(LOG_ARB, "arbitration: started\n");
		m_state = ARB_EVALUATE;
		delay = SCSI_ARB_DELAY;

		// assert own ID and BSY
		scsi_bus->data_w(scsi_refid, oid);
		scsi_bus->ctrl_w(scsi_refid, S_BSY, S_BSY);
		break;
	case ARB_EVALUATE:
		// check if SEL asserted, or if there's a higher ID on the bus
		if ((scsi_bus->ctrl_r() & S_SEL) || (scsi_bus->data_r() & ~((oid - 1) | oid)))
		{
			LOGMASKED(LOG_ARB, "arbitration: lost\n");
			set_cstat(CSTAT_LST, CSTAT_LST);

			m_state = ARB_BUS_FREE;
			delay = SCSI_BUS_FREE; // FIXME: how long?

			// clear data and BSY
			scsi_bus->data_w(scsi_refid, 0);
			scsi_bus->ctrl_w(scsi_refid, 0, S_BSY);
		}
		else
		{
			LOGMASKED(LOG_ARB, "arbitration: won\n");
			set_cstat(CSTAT_SIP, CSTAT_SIP | CSTAT_LST);

			m_state = SEL_START;
			delay = SCSI_BUS_CLEAR + SCSI_BUS_SETTLE;
		}
		break;

	case SEL_START:
		LOGMASKED(LOG_SEL, "selection: SEL asserted\n");
		m_state = SEL_DELAY;
		delay = SCSI_BUS_SKEW * 2;

		// assert own and target ID and SEL
		scsi_bus->data_w(scsi_refid, oid | tid);
		scsi_bus->ctrl_w(scsi_refid, S_SEL, S_SEL);
		break;
	case SEL_DELAY:
		LOGMASKED(LOG_SEL, "selection: BSY cleared\n");
		m_state = SEL_WAIT_BSY;
		delay = SCSI_BUS_SETTLE;

		// clear BSY, optionally assert ATN
		if (m_comm & COMM_ATN)
			scsi_bus->ctrl_w(scsi_refid, S_ATN, S_BSY | S_ATN);
		else
			scsi_bus->ctrl_w(scsi_refid, 0, S_BSY);
		break;
	case SEL_WAIT_BSY:
		if (scsi_bus->ctrl_r() & S_BSY)
		{
			LOGMASKED(LOG_SEL, "selection: BSY asserted by target\n");
			m_state = SEL_COMPLETE;
			delay = SCSI_BUS_SKEW * 2;
		}
		else
		{
			LOGMASKED(LOG_SEL, "selection: timed out\n");
			m_state = IDLE;

			//scsi_bus->ctrl_w(scsi_refid, 0, S_ATN | S_SEL);
		}
		break;
	case SEL_COMPLETE:
		LOGMASKED(LOG_SEL, "selection: complete\n");
		m_state = IDLE;
		set_cstat(CSTAT_SCH | CSTAT_CON, CSTAT_SCH | CSTAT_CON | CSTAT_SIP);

		// clear data and SEL
		scsi_bus->data_w(scsi_refid, 0);
		scsi_bus->ctrl_w(scsi_refid, 0, S_SEL);
		break;

	case XFR_PIO:
		// TODO: phase match
		set_dstat(0, DSTAT_TBE);
		if (scsi_bus->ctrl_r() & S_REQ)
			m_state = (scsi_bus->ctrl_r() & S_INP) ? XFR_PIO_IN : XFR_PIO_OUT;
		break;
	case XFR_PIO_OUT:
		LOGMASKED(LOG_PIO, "xfr pio out: data 0x%02x\n", m_data);
		m_state = XFR_PIO_OUT_ACK;

		// assert data and ACK
		scsi_bus->data_w(scsi_refid, m_data);
		scsi_bus->ctrl_w(scsi_refid, S_ACK, S_ACK);
		break;
	case XFR_PIO_OUT_ACK:
		if (!(scsi_bus->ctrl_r() & S_REQ))
		{
			LOGMASKED(LOG_PIO, "xfr pio out: data ACK\n");

			m_state = IDLE;
			set_dstat(DSTAT_DNE, DSTAT_DNE);

			// clear data and ACK
			scsi_bus->data_w(scsi_refid, 0);
			scsi_bus->ctrl_w(scsi_refid, 0, S_ACK);
		}
		break;

	case XFR_DMA:
		// TODO: phase match
		if (scsi_bus->ctrl_r() & S_REQ)
			m_state = (scsi_bus->ctrl_r() & S_INP) ? XFR_DMA_IN_REQ : XFR_DMA_OUT;
		break;
	case XFR_DMA_IN_REQ:
		{
			u8 const data = scsi_bus->data_r();
			LOGMASKED(LOG_DMA, "xfr dma in: data 0x%02x\n", data);

			if (m_dstat & DSTAT_OBB)
			{
				m_dma_w((u32(m_dmaddrh) << 16) | m_dmaddrl, (u16(data) << 8) | m_dmabyte);
			}
			else
			{
				m_dmabyte = data;
			}

			m_dstat ^= DSTAT_OBB;

			// assert ACK
			scsi_bus->ctrl_w(scsi_refid, S_ACK, S_ACK);
		}
		break;
	default:
		break;
	}

	return delay;
}
