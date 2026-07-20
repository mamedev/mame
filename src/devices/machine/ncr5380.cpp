// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * NCR 5380 and 53C80, aka Zilog Z5380, AMD Am5380, Sony CXD1180, National Semiconductor DP8490, Logic Devices L5380 and others.
 *
 * Sources:
 *  - http://bitsavers.org/components/ncr/scsi/SP-1051_NCR_5380-53C80_SCSI_Interface_Chip_Design_Manual_Mar86.pdf
 *
 * TODO:
 *  - target mode
 *  - CXD1180 enhancements
 *  - DP8490 enhancements
 */

#include "emu.h"
#include "ncr5380.h"

#define LOG_REGW     (1U << 1)
#define LOG_REGR     (1U << 2)
#define LOG_SCSI     (1U << 3)
#define LOG_ARB      (1U << 4)
#define LOG_DMA      (1U << 5)

//#define VERBOSE (LOG_GENERAL|LOG_REGW|LOG_REGR|LOG_SCSI|LOG_ARB|LOG_DMA)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(NCR5380,  ncr5380_device,  "ncr5380",  "NCR 5380 SCSI")
DEFINE_DEVICE_TYPE(NCR53C80, ncr53c80_device, "ncr53c80", "NCR 53C80 SCSI")
DEFINE_DEVICE_TYPE(CXD1180,  cxd1180_device,  "cxd1180",  "Sony CXD1180")
DEFINE_DEVICE_TYPE(DP8490,   dp8490_device,   "dp8490",   "National Semiconductor DP8490 EASI")

ALLOW_SAVE_TYPE(ncr5380_device::state);

ncr5380_device::ncr5380_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock, bool has_lbs)
	: device_t(mconfig, type, tag, owner, clock)
	, nscsi_device_interface(mconfig, *this)
	, m_irq_handler(*this)
	, m_drq_handler(*this)
	, m_has_lbs(has_lbs)
	, m_rst_self_irq(true)
{
}

ncr5380_device::ncr5380_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: ncr5380_device(mconfig, NCR5380, tag, owner, clock)
{
}

ncr53c80_device::ncr53c80_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: ncr5380_device(mconfig, NCR53C80, tag, owner, clock, true)
{
}

cxd1180_device::cxd1180_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: ncr5380_device(mconfig, CXD1180, tag, owner, clock, true)
{
}

dp8490_device::dp8490_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: ncr5380_device(mconfig, DP8490, tag, owner, clock, true)
{
	// EASI divergence: no interrupt for a self-issued bus reset (see
	// scsi_ctrl_changed; base-5380 behavior per SP-1051 8.3 is to interrupt)
	m_rst_self_irq = false;
}

void ncr5380_device::map(address_map &map)
{
	map(0x0, 0x0).rw(FUNC(ncr5380_device::csdata_r), FUNC(ncr5380_device::odata_w));
	map(0x1, 0x1).rw(FUNC(ncr5380_device::icmd_r), FUNC(ncr5380_device::icmd_w));
	map(0x2, 0x2).rw(FUNC(ncr5380_device::mode_r), FUNC(ncr5380_device::mode_w));
	map(0x3, 0x3).rw(FUNC(ncr5380_device::tcmd_r), FUNC(ncr5380_device::tcmd_w));
	map(0x4, 0x4).rw(FUNC(ncr5380_device::csstat_r), FUNC(ncr5380_device::selen_w));
	map(0x5, 0x5).rw(FUNC(ncr5380_device::bas_r), FUNC(ncr5380_device::sds_w));
	map(0x6, 0x6).rw(FUNC(ncr5380_device::idata_r), FUNC(ncr5380_device::sdtr_w));
	map(0x7, 0x7).rw(FUNC(ncr5380_device::rpi_r), FUNC(ncr5380_device::sdir_w));
}

void ncr5380_device::device_start()
{
	// Need to be cleared here so that set_irq/drq called from reset
	// does not compare with uninitialized
	m_irq_state = false;
	m_drq_state = false;
	m_rst_out = false;

	m_state_timer = timer_alloc(FUNC(ncr5380_device::state_timer), this);

	save_item(NAME(m_state));

	save_item(NAME(m_odata));
	save_item(NAME(m_icmd));
	save_item(NAME(m_mode));
	save_item(NAME(m_tcmd));
	save_item(NAME(m_bas));
	save_item(NAME(m_idata));
	save_item(NAME(m_selen));

	save_item(NAME(m_scsi_ctrl));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_drq_state));
	save_item(NAME(m_rst_out));
}

void ncr5380_device::device_reset()
{
	m_state = IDLE;

	// clear registers
	m_odata = 0;
	m_icmd = 0;
	m_mode = 0;
	m_tcmd = 0;
	m_bas = 0;
	m_idata = 0;
	m_selen = 0;

	// clear scsi bus
	m_scsi_bus->data_w(m_scsi_refid, 0);
	m_scsi_bus->ctrl_w(m_scsi_refid, 0, S_ALL);

	// monitor all control lines
	m_scsi_ctrl = 0;
	m_scsi_bus->ctrl_wait(m_scsi_refid, S_ALL, S_ALL);

	// clear output lines
	set_irq(false);
	set_drq(false);
}

void ncr5380_device::scsi_ctrl_changed()
{
	u32 const ctrl = m_scsi_bus->ctrl_r();

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

	m_bas &= ~BAS_BUSYERROR;

	// initiator data-bus drivers follow the phase lines: with ICR "assert data
	// bus" set the drivers tri-state as soon as the bus leaves the OUT phase
	// matching the target-command register (e.g. the target switches DATA OUT
	// -> STATUS).  Without this the last data byte stays wire-OR'd over the
	// target's STATUS/DATA-IN bytes.  Release-only: the drivers re-enable via
	// the normal odata_w/ic_write paths, never spontaneously here.
	if (!(m_mode & MODE_TARGET) && (m_icmd & IC_DBUS) && !(ctrl & S_RST)
		&& ((ctrl & S_INP) || (ctrl & S_PHASE_MASK) != (m_tcmd & S_PHASE_MASK)))
		m_scsi_bus->data_w(m_scsi_refid, 0);

	if (ctrl & S_RST)
	{
		LOG("scsi reset received\n");
		bool const self_reset = m_rst_out;
		device_reset();

		// The base 5380 interrupts on ANY R̅S̅T̅ transition, including one it
		// asserted itself, and that interrupt cannot be disabled (SP-1051 8.3:
		// "this interrupt also occurs after setting the ASSERT R̅S̅T̅ bit").  The
		// DP8490 EASI diverges: firmware written against the real part (the
		// NetBSD/pc532 ROM monitor) asserts R̅S̅T̅ and treats a resulting IRQ as
		// a fault, so the EASI evidently suppresses the self-reset interrupt.
		if (!self_reset || m_rst_self_irq)
			set_irq(true);
	}
	else if (!(m_mode & MODE_TARGET) && (m_scsi_ctrl & S_BSY) && !(ctrl & S_BSY))
	{
		LOG("target disconnected\n");
		m_icmd &= (IC_RST | IC_AIP);

		if (m_mode & MODE_DMA)
		{
			// stop dma
			m_mode &= ~MODE_DMA;
			m_bas &= ~BAS_ENDOFDMA;

			set_drq(false);
		}

		if (m_mode & MODE_BSYIRQ)
		{
			m_bas |= BAS_BUSYERROR;

			set_irq(true);
		}

		m_state = IDLE;
		m_state_timer->enable(false);

		// clear scsi bus
		m_scsi_bus->data_w(m_scsi_refid, 0);
		m_scsi_bus->ctrl_w(m_scsi_refid, 0, S_ALL);
	}
	else if (!(m_mode & MODE_TARGET) && !(m_scsi_ctrl & S_REQ) && (ctrl & S_REQ))
	{
		// initiator side: the target asserted REQ
		if (m_mode & MODE_DMA)
		{
			if ((ctrl & S_PHASE_MASK) == (m_tcmd & TC_PHASE))
			{
				// transfer cycle
				if (m_state != IDLE && !m_state_timer->enabled())
					m_state_timer->adjust(attotime::zero);
			}
			else
			{
				LOG("phase mismatch %d != %d\n", (ctrl & S_PHASE_MASK), (m_tcmd & TC_PHASE));

				m_state = IDLE;
				m_state_timer->enable(false);

				// A bus phase mismatch ends the DMA transfer and interrupts
				// (SP-1051 8.5).  Direction matters for DRQ: after a SEND the
				// chip has already requested the next byte, and that request
				// survives the mismatch (6.7: DRQ "does not reset when a phase
				// mismatch interrupt occurs"); the host completes the transfer
				// by cycling DACK once more so ACK can release (10.5.2, 10.3).
				// The Mac Plus SCSI Manager blocks on that final DRQ.  A
				// RECEIVE has no pending request -- the last byte was already
				// DACKed -- so DRQ stays low (a spurious raise feeds pseudo-DMA
				// read loops a garbage byte).
				if (!(m_tcmd & TC_IO))
					set_drq(true);
				set_irq(true);
			}
		}
	}
	else if ((m_mode & MODE_TARGET) && ((m_scsi_ctrl ^ ctrl) & S_ACK))
	{
		// target side: an A̅C̅K̅ edge from the initiator advances the transfer states
		if (m_state != IDLE && !m_state_timer->enabled())
			m_state_timer->adjust(attotime::zero);
	}

	// Selection/reselection detect (armed via the Select Enable Register): another
	// device asserts S̅E̅L̅ with our ID on the data bus while B̅S̅Y̅ is released and we
	// are not driving S̅E̅L̅ ourselves -> we are being selected (target) or reselected
	// (initiator).  Raise an interrupt; firmware accepts by asserting B̅S̅Y̅.  The
	// I̅C̅_S̅E̅L̅ guard avoids a false match when WE select another device (our own ID is
	// on the bus then).
	if (m_selen && !(m_icmd & IC_SEL)
		&& !(m_scsi_ctrl & S_SEL) && (ctrl & S_SEL) && !(ctrl & S_BSY)
		&& (m_scsi_bus->data_r() & m_selen))
	{
		LOG("selected (sel-enable 0x%02x, data 0x%02x)\n", m_selen, m_scsi_bus->data_r());
		set_irq(true);
	}

	m_scsi_ctrl = ctrl;
}

u8 ncr5380_device::csdata_r()
{
	u8 const data = m_scsi_bus->data_r();
	LOGMASKED(LOG_REGR, "csdata_r 0x%02x (%s)\n", data, machine().describe_context());

	return data;
}

void ncr5380_device::odata_w(u8 data)
{
	LOGMASKED(LOG_REGW, "odata_w 0x%02x (%s)\n", data, machine().describe_context());

	// drive scsi data
	if (m_icmd & IC_DBUS)
		scsi_data_w(data);

	m_odata = data;
}

u8 ncr5380_device::icmd_r()
{
	LOGMASKED(LOG_REGR, "icmd_r 0x%02x (%s)\n", m_icmd, machine().describe_context());

	return m_icmd;
}

void ncr5380_device::icmd_w(u8 data)
{
	LOGMASKED(LOG_REGW, "icmd_w 0x%02x (%s)\n", data, machine().describe_context());

	if (!(data & IC_RST))
	{
		m_rst_out = false;

		// drive scsi data
		if ((data ^ m_icmd) & IC_DBUS)
			scsi_data_w((data & IC_DBUS) ? m_odata : 0);

		// check for control line changes
		if ((data & IC_PHASE) ^ (m_icmd & IC_PHASE))
		{
			u32 const mask = (m_mode & MODE_TARGET) ?
				(S_RST | S_BSY | S_SEL) :
				(S_RST | S_ACK | S_BSY | S_SEL | S_ATN);

			// translate to nscsi
			u32 const ctrl =
				(data & IC_RST ? S_RST : 0) |
				(data & IC_ACK ? S_ACK : 0) |
				(data & IC_BSY ? S_BSY : 0) |
				(data & IC_SEL ? S_SEL : 0) |
				(data & IC_ATN ? S_ATN : 0);

			LOGMASKED(LOG_SCSI, "changing control lines 0x%04x\n", ctrl);
			m_scsi_bus->ctrl_w(m_scsi_refid, ctrl, mask);
		}
	}
	else
	{
		LOG("scsi reset issued\n");
		device_reset();
		// the chip is now driving R̅S̅T̅ itself; flag it so the resulting bus
		// reset notification is not mistaken for an externally-initiated reset
		// and does not raise a (self-)interrupt the host driver never expects
		// (NetBSD/pc532 monitor asserts R̅S̅T̅, then aborts on the spurious IRQ).
		m_rst_out = true;
		m_scsi_bus->ctrl_w(m_scsi_refid, S_RST, S_RST);
	}

	m_icmd = (m_icmd & ~IC_WRITE) | (data & IC_WRITE);
}

u8 ncr5380_device::mode_r()
{
	LOGMASKED(LOG_REGR, "mode_r 0x%02x (%s)\n", m_mode, machine().describe_context());

	return m_mode;
}

void ncr5380_device::mode_w(u8 data)
{
	LOGMASKED(LOG_REGW, "mode_w 0x%02x (%s)\n", data, machine().describe_context());

	if (!(data & MODE_BSYIRQ))
		m_bas &= ~BAS_BUSYERROR;

	// disable dma
	if ((m_mode & MODE_DMA) && !(data & MODE_DMA))
	{
		m_state = IDLE;
		m_state_timer->enable(false);

		m_bas &= ~BAS_ENDOFDMA;

		if (m_has_lbs)
			m_tcmd &= ~TC_LBS;

		set_drq(false);

		// clear ACK
		m_scsi_bus->ctrl_w(m_scsi_refid, 0, S_ACK);
	}

	// start/stop arbitration
	if ((m_mode ^ data) & MODE_ARBITRATE)
	{
		if (data & MODE_ARBITRATE)
		{
			// start arbitration
			m_icmd &= ~IC_LA;
			m_state = ARB_BUS_FREE;
			m_state_timer->adjust(attotime::zero);
		}
		else
		{
			// stop arbitration
			m_state = IDLE;
			m_icmd &= ~(IC_AIP | IC_LA);
		}
	}

	m_mode = data;
}

u8 ncr5380_device::tcmd_r()
{
	LOGMASKED(LOG_REGR, "tcmd_r 0x%02x (%s)\n", m_tcmd, machine().describe_context());

	return m_tcmd;
}

void ncr5380_device::tcmd_w(u8 data)
{
	LOGMASKED(LOG_REGW, "tcmd_w 0x%02x (%s)\n", data, machine().describe_context());

	if (m_has_lbs)
		m_tcmd = (m_tcmd & TC_LBS) | (data & ~TC_LBS);
	else
		m_tcmd = data;

	// In target mode the Target Command Register drives the phase lines (C̅/D, I̅/O,
	// M̅S̅G̅) and R̅E̅Q̅ onto the bus; an initiator's TCR only records the expected phase
	// for phase-match comparison and drives nothing.
	if (m_mode & MODE_TARGET)
	{
		u32 mask = S_MSG | S_CTL | S_INP;
		u32 ctrl =
			(m_tcmd & TC_MSG ? S_MSG : 0) |
			(m_tcmd & TC_CD  ? S_CTL : 0) |
			(m_tcmd & TC_IO  ? S_INP : 0);

		// R̅E̅Q̅ is driven from the TCR only in programmed (non-DMA) target transfers;
		// during MODE_DMA the target DMA state machine owns R̅E̅Q̅, so leave it alone.
		if (!(m_mode & MODE_DMA))
		{
			mask |= S_REQ;
			ctrl |= (m_tcmd & TC_REQ ? S_REQ : 0);
		}

		m_scsi_bus->ctrl_w(m_scsi_refid, ctrl, mask);
	}
}

u8 ncr5380_device::csstat_r()
{
	u32 const ctrl = m_scsi_bus->ctrl_r();
	u8 const data =
		(ctrl & S_RST ? ST_RST : 0) |
		(ctrl & S_BSY ? ST_BSY : 0) |
		(ctrl & S_REQ ? ST_REQ : 0) |
		(ctrl & S_MSG ? ST_MSG : 0) |
		(ctrl & S_CTL ? ST_CD  : 0) |
		(ctrl & S_INP ? ST_IO  : 0) |
		(ctrl & S_SEL ? ST_SEL : 0);

	LOGMASKED(LOG_REGR, "csstat_r 0x%02x (%s)\n", data, machine().describe_context());
	return data;
}

void ncr5380_device::selen_w(u8 data)
{
	LOGMASKED(LOG_REGW, "selen_w 0x%02x (%s)\n", data, machine().describe_context());

	// Select Enable Register: bit mask of the SCSI ID(s) this chip answers to.
	// When S̅E̅L̅ is asserted with a matching ID on the data bus (and B̅S̅Y̅ released),
	// the chip raises a selection (target) / reselection (initiator) interrupt.
	m_selen = data;
}

u8 ncr5380_device::bas_r()
{
	u32 const ctrl = m_scsi_bus->ctrl_r();
	u8 const data = m_bas |
		(((ctrl & S_PHASE_MASK) == (m_tcmd & TC_PHASE)) ? BAS_PHASEMATCH : 0) |
		(ctrl & S_ATN ? BAS_ATN : 0) |
		(ctrl & S_ACK ? BAS_ACK : 0);

	LOGMASKED(LOG_REGR, "bas_r 0x%02x (%s)\n", data, machine().describe_context());

	return data;
}

void ncr5380_device::sds_w(u8 data)
{
	LOGMASKED(LOG_REGW, "sds_w 0x%02x (%s)\n", data, machine().describe_context());

	if (m_mode & MODE_DMA)
	{
		// Start DMA Send: in initiator mode this sends data OUT to a target; in
		// target mode it sends data IN to the initiator (we drive data + R̅E̅Q̅).
		m_state = (m_mode & MODE_TARGET) ? TSEND_DRQ : DMA_OUT_DRQ;
		m_state_timer->adjust(attotime::zero);
	}
}

u8 ncr5380_device::idata_r()
{
	LOGMASKED(LOG_REGR, "idata_r 0x%02x (%s)\n", m_idata, machine().describe_context());

	return m_idata;
}

void ncr5380_device::sdtr_w(u8 data)
{
	LOGMASKED(LOG_REGW, "sdtr_w 0x%02x (%s)\n", data, machine().describe_context());

	// Start DMA Target Receive: target-mode only.  We drive R̅E̅Q̅ and latch the byte
	// the initiator presents with A̅C̅K̅.
	if ((m_mode & MODE_DMA) && (m_mode & MODE_TARGET))
	{
		m_state = TRECV_REQ;
		m_state_timer->adjust(attotime::zero);
	}
}

u8 ncr5380_device::rpi_r()
{
	LOGMASKED(LOG_REGR, "rpi_r (%s)\n", machine().describe_context());

	m_bas &= ~(BAS_PARITYERROR | BAS_BUSYERROR);
	set_irq(false);

	return 0;
}

void ncr5380_device::sdir_w(u8 data)
{
	LOGMASKED(LOG_REGW, "sdir_w 0x%02x (%s)\n", data, machine().describe_context());

	if ((m_mode & MODE_DMA) && !(m_mode & MODE_TARGET))
	{
		m_state = DMA_IN_REQ;
		m_state_timer->adjust(attotime::zero);
	}
}

void ncr5380_device::state_timer(s32 param)
{
	// step state machine
	int const prev_state = m_state;
	int const delay = state_step();

	// check for data stall
	if (delay < 0)
		return;

	// repeat until idle
	if (m_state != IDLE)
	{
		// A DMA wait-state that made no progress and asked for no delay is
		// polling the SCSI bus for a REQ/phase change.  Re-arming at zero would
		// spin with emulated time frozen, so the target could never change the
		// bus -- hanging the whole emulation (observed booting Minix 1.3/pc532,
		// whose root-mount read polls REQ in a window NetBSD's timing skipped).
		// Poll at the SCSI REQ/ACK period instead so time advances and the
		// target can respond.  (An event-driven resume via scsi_ctrl_changed
		// would be faster but mis-steps the monitor's SCSI read -> busy error.)
		unsigned const ns = (m_state == prev_state && delay == 0) ? 100 : unsigned(delay);
		m_state_timer->adjust(attotime::from_nsec(ns));
	}
}

int ncr5380_device::state_step()
{
	u32 const ctrl = m_scsi_bus->ctrl_r();
	int delay = 0;

	switch (m_state)
	{
	case IDLE:
		break;

	case ARB_BUS_FREE:
		if (!(ctrl & (S_SEL | S_BSY | S_RST)))
		{
			LOGMASKED(LOG_ARB, "arbitration: bus free\n");
			// FIXME: AIP should only be set when arbitration begins
			m_icmd |= IC_AIP;
			m_state = ARB_START;
			delay = 1700;
		}
		else
		{
			LOGMASKED(LOG_ARB, "arbitration: bus not free\n");
			m_state = IDLE;
		}
		break;
	case ARB_START:
		LOGMASKED(LOG_ARB, "arbitration: started\n");
		m_icmd |= IC_BSY;
		m_state = ARB_EVALUATE;
		delay = 2200;

		// assert own ID and BSY
		m_scsi_bus->data_w(m_scsi_refid, m_odata);
		m_scsi_bus->ctrl_w(m_scsi_refid, S_BSY, S_BSY);
		break;
	case ARB_EVALUATE:
		// check if SEL asserted, or if there's a higher ID on the bus
		if ((ctrl & S_SEL) || (m_scsi_bus->data_r() & ~((m_odata - 1) | m_odata)))
		{
			LOGMASKED(LOG_ARB, "arbitration: lost\n");
			m_icmd &= ~IC_BSY;
			m_icmd |= IC_LA;

			m_state = IDLE;

			// clear data and BSY
			m_scsi_bus->data_w(m_scsi_refid, 0);
			m_scsi_bus->ctrl_w(m_scsi_refid, 0, S_BSY);
		}
		else
		{
			LOGMASKED(LOG_ARB, "arbitration: won\n");
			m_state = IDLE;
		}
		break;

	case DMA_IN_REQ:
		if (ctrl & S_REQ)
		{
			if ((ctrl & S_PHASE_MASK) == (m_tcmd & TC_PHASE))
			{
				m_idata = m_scsi_bus->data_r();
				LOGMASKED(LOG_DMA, "dma in: 0x%02x\n", m_idata);

				m_state = DMA_IN_ACK;
				set_drq(true);

				// assert ACK
				m_scsi_bus->ctrl_w(m_scsi_refid, S_ACK, S_ACK);
			}

			delay = -1;
		}
		break;
	case DMA_IN_ACK:
		if (!(ctrl & S_REQ))
		{
			m_state = (m_bas & BAS_ENDOFDMA) ? IDLE : DMA_IN_REQ;

			// clear ACK
			m_scsi_bus->ctrl_w(m_scsi_refid, 0, S_ACK);
		}
		break;

	case DMA_OUT_DRQ:
		m_state = DMA_OUT_REQ;
		set_drq(true);

		delay = -1;
		break;
	case DMA_OUT_REQ:
		if (ctrl & S_REQ)
		{
			if ((ctrl & S_PHASE_MASK) == (m_tcmd & TC_PHASE))
			{
				LOGMASKED(LOG_DMA, "dma out: 0x%02x\n", m_odata);

				m_state = DMA_OUT_ACK;

				// assert data and ACK
				m_scsi_bus->data_w(m_scsi_refid, m_odata);
				m_scsi_bus->ctrl_w(m_scsi_refid, S_ACK, S_ACK);
			}
		}
		break;
	case DMA_OUT_ACK:
		if (!(ctrl & S_REQ))
		{
			if (m_bas & BAS_ENDOFDMA)
			{
				m_state = IDLE;

				if (m_has_lbs)
					m_tcmd |= TC_LBS;
			}
			else
				m_state = DMA_OUT_DRQ;

			// clear data and ACK
			m_scsi_bus->data_w(m_scsi_refid, 0);
			m_scsi_bus->ctrl_w(m_scsi_refid, 0, S_ACK);
		}
		break;

	// ---- target-mode DMA: we drive R̅E̅Q̅, the initiator answers with A̅C̅K̅ ----
	case TSEND_DRQ:
		// request a byte from the host DMA; stall until dma_w supplies it
		m_state = TSEND_REQ;
		set_drq(true);
		delay = -1;
		break;
	case TSEND_REQ:
		// drive the supplied data and assert R̅E̅Q̅
		LOGMASKED(LOG_DMA, "target send: 0x%02x\n", m_odata);
		m_scsi_bus->data_w(m_scsi_refid, m_odata);
		m_scsi_bus->ctrl_w(m_scsi_refid, S_REQ, S_REQ);
		m_state = TSEND_ACK;
		delay = -1;
		break;
	case TSEND_ACK:
		// initiator latched the byte (A̅C̅K̅) -> deassert R̅E̅Q̅
		if (ctrl & S_ACK)
		{
			m_scsi_bus->ctrl_w(m_scsi_refid, 0, S_REQ);
			m_state = TSEND_END;
		}
		delay = -1;
		break;
	case TSEND_END:
		// initiator released A̅C̅K̅ -> release data; next byte or done
		if (!(ctrl & S_ACK))
		{
			m_scsi_bus->data_w(m_scsi_refid, 0);
			if (m_bas & BAS_ENDOFDMA)
			{
				m_state = IDLE;
				if (m_has_lbs)
					m_tcmd |= TC_LBS;
			}
			else
				m_state = TSEND_DRQ;
		}
		delay = -1;
		break;

	case TRECV_REQ:
		// assert R̅E̅Q̅; the initiator will present data + A̅C̅K̅
		m_scsi_bus->ctrl_w(m_scsi_refid, S_REQ, S_REQ);
		m_state = TRECV_ACK;
		delay = -1;
		break;
	case TRECV_ACK:
		// initiator drove data and asserted A̅C̅K̅ -> latch it, deassert R̅E̅Q̅, DRQ host
		if (ctrl & S_ACK)
		{
			m_idata = m_scsi_bus->data_r();
			LOGMASKED(LOG_DMA, "target recv: 0x%02x\n", m_idata);
			m_scsi_bus->ctrl_w(m_scsi_refid, 0, S_REQ);
			set_drq(true);
			m_state = TRECV_END;
		}
		delay = -1;
		break;
	case TRECV_END:
		// initiator released A̅C̅K̅ -> next byte or done
		if (!(ctrl & S_ACK))
			m_state = (m_bas & BAS_ENDOFDMA) ? IDLE : TRECV_REQ;
		delay = -1;
		break;
	}

	return delay;
}

void ncr5380_device::eop_w(int state)
{
	LOGMASKED(LOG_DMA, "eop_w %d\n", state);
	if (state && (m_mode & MODE_DMA))
	{
		m_bas |= BAS_ENDOFDMA;

		if (m_mode & MODE_EOPIRQ)
		{
			// FIXME: should only trigger when combined with dma_r/dma_w
			LOG("eop irq asserted\n");

			set_irq(true);
		}
	}
}

void ncr5380_device::dma_w(u8 data)
{
	set_drq(false);

	if (m_mode & MODE_DMA)
	{
		m_odata = data;

		m_state_timer->adjust(attotime::zero);
	}
}

u8 ncr5380_device::dma_r()
{
	set_drq(false);

	if (m_mode & MODE_DMA)
		m_state_timer->adjust(attotime::zero);

	return m_idata;
}

void ncr5380_device::scsi_data_w(u8 data)
{
	// TODO: release data bus when any of the prerequisite conditions expire
	u32 const ctrl = m_scsi_bus->ctrl_r();

	if ((m_mode & MODE_TARGET) || (!(ctrl & S_INP) && (ctrl & S_PHASE_MASK) == (m_tcmd & S_PHASE_MASK)))
	{
		LOGMASKED(LOG_SCSI, "scsi data 0x%02x\n", data);
		m_scsi_bus->data_w(m_scsi_refid, data);
	}
}

void ncr5380_device::set_irq(bool irq_state)
{
	if (irq_state != m_irq_state)
	{
		LOG("set_irq %d\n", irq_state);

		if (irq_state)
			m_bas |= BAS_IRQACTIVE;
		else
			m_bas &= ~BAS_IRQACTIVE;

		m_irq_state = irq_state;
		m_irq_handler(m_irq_state);
	}
}

void ncr5380_device::set_drq(bool drq_state)
{
	if (drq_state != m_drq_state)
	{
		LOGMASKED(LOG_DMA, "set_drq %d\n", drq_state);

		if (drq_state)
			m_bas |= BAS_DMAREQUEST;
		else
			m_bas &= ~BAS_DMAREQUEST;

		m_drq_state = drq_state;
		m_drq_handler(m_drq_state);
	}
}

u8 ncr5380_device::read(offs_t offset)
{
	switch (offset & 7)
	{
	case 0: return csdata_r();
	case 1: return icmd_r();
	case 2: return mode_r();
	case 3: return tcmd_r();
	case 4: return csstat_r();
	case 5: return bas_r();
	case 6: return idata_r();
	case 7: return rpi_r();
	}

	// can't happen
	return 0;
}

void ncr5380_device::write(offs_t offset, u8 data)
{
	switch (offset & 7)
	{
	case 0: odata_w(data); break;
	case 1: icmd_w(data); break;
	case 2: mode_w(data); break;
	case 3: tcmd_w(data); break;
	case 4: selen_w(data); break;
	case 5: sds_w(data); break;
	case 6: sdtr_w(data); break;
	case 7: sdir_w(data); break;
	}
}
