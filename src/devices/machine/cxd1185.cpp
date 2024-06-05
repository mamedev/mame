// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Sony CXD1185 SCSI 1 Protocol Controller.
 *
 * Sources:
 *  - https://datasheetspdf.com/pdf-file/199633/SonyCorporation/CXD1185CQ/1
 *
 * TODO:
 *  - target mode
 *  - cq/aq variants
 *  - synchronous mode
 */

#include "emu.h"
#include "cxd1185.h"

#define LOG_CMD     (1U << 1)
#define LOG_REG     (1U << 2)
#define LOG_STATE   (1U << 3)
#define LOG_CONFIG  (1U << 4)
#define LOG_INT     (1U << 5)
#define LOG_SCSI    (1U << 6)
#define LOG_DMA     (1U << 7)

//#define VERBOSE (LOG_GENERAL|LOG_CMD|LOG_REG|LOG_STATE|LOG_CONFIG|LOG_INT|LOG_SCSI|LOG_DMA)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(CXD1185, cxd1185_device, "cxd1185", "Sony CXD1185 SCSI 1 Protocol Controller")

static char const *const nscsi_phase[] = { "DATA OUT", "DATA IN", "COMMAND", "STATUS", "*", "*", "MESSAGE OUT", "MESSAGE IN" };

// FIXME: would be better to reuse from nscsi_full_device
static unsigned const SCSI_ARB_DELAY  =  2'400;
static unsigned const SCSI_BUS_CLEAR  =    800;
static unsigned const SCSI_BUS_FREE   =    800;
static unsigned const SCSI_BUS_SETTLE =    400;
static unsigned const SCSI_BUS_SKEW   =     10;
static unsigned const SCSI_RST_HOLD   = 25'000;

ALLOW_SAVE_TYPE(cxd1185_device::state);

cxd1185_device::cxd1185_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: nscsi_device(mconfig, CXD1185, tag, owner, clock)
	, nscsi_slot_card_interface(mconfig, *this, DEVICE_SELF)
	, m_irq_out_cb(*this)
	, m_drq_out_cb(*this)
	, m_port_out_cb(*this)
{
}

void cxd1185_device::map(address_map &map)
{
	map(0x0, 0x0).rw(FUNC(cxd1185_device::status_r), FUNC(cxd1185_device::command_w));
	map(0x1, 0x1).rw(FUNC(cxd1185_device::scsi_data_r), FUNC(cxd1185_device::scsi_data_w));
	map(0x2, 0x2).r(FUNC(cxd1185_device::int_req_r<0>));
	map(0x3, 0x3).rw(FUNC(cxd1185_device::int_req_r<1>), FUNC(cxd1185_device::environ_w));
	map(0x4, 0x4).rw(FUNC(cxd1185_device::scsi_ctrl_monitor_r), FUNC(cxd1185_device::timer_w));
	map(0x5, 0x5).r(FUNC(cxd1185_device::fifo_status_r));
	map(0x6, 0x6).rw(FUNC(cxd1185_device::scsi_id_r), FUNC(cxd1185_device::scsi_id_w));
	map(0x7, 0x7).rw(FUNC(cxd1185_device::count_r<0>), FUNC(cxd1185_device::count_w<0>));
	map(0x8, 0x8).rw(FUNC(cxd1185_device::count_r<1>), FUNC(cxd1185_device::count_w<1>));
	map(0x9, 0x9).rw(FUNC(cxd1185_device::count_r<2>), FUNC(cxd1185_device::count_w<2>));
	map(0xa, 0xa).rw(FUNC(cxd1185_device::int_auth_r<0>), FUNC(cxd1185_device::int_auth_w<0>));
	map(0xb, 0xb).rw(FUNC(cxd1185_device::int_auth_r<1>), FUNC(cxd1185_device::int_auth_w<1>));
	map(0xc, 0xc).rw(FUNC(cxd1185_device::mode_r), FUNC(cxd1185_device::mode_w));
	map(0xd, 0xd).rw(FUNC(cxd1185_device::sync_ctrl_r), FUNC(cxd1185_device::sync_ctrl_w));
	map(0xe, 0xe).rw(FUNC(cxd1185_device::scsi_ctrl_r), FUNC(cxd1185_device::scsi_ctrl_w));
	map(0xf, 0xf).rw(FUNC(cxd1185_device::ioport_r), FUNC(cxd1185_device::ioport_w));
}

void cxd1185_device::device_start()
{
	save_item(NAME(m_state));
	save_item(NAME(m_irq_asserted));
	save_item(NAME(m_drq_asserted));
	save_item(NAME(m_pio_data_mode));
	save_item(NAME(m_pio_ctrl_mode));
	//save_item(NAME(m_fifo));
	save_item(NAME(m_scsi_ctrl_state));

	save_item(NAME(m_status));
	save_item(NAME(m_command));
	save_item(NAME(m_int_req));
	save_item(NAME(m_environ));
	save_item(NAME(m_sel_time));
	save_item(NAME(m_rst_time));
	save_item(NAME(m_scsi_id));
	save_item(NAME(m_int_auth));
	save_item(NAME(m_mode));
	save_item(NAME(m_count));
	save_item(NAME(m_sync_ctrl));
	save_item(NAME(m_scsi_ctrl));
	save_item(NAME(m_ioport));

	m_state_timer = timer_alloc(FUNC(cxd1185_device::state_timer), this);
	m_state = IDLE;

	m_irq_asserted = false;
	m_drq_asserted = false;

	// monitor all scsi bus control lines
	scsi_bus->ctrl_wait(scsi_refid, S_ALL, S_ALL);
}

void cxd1185_device::device_reset()
{
	m_environ = 0;

	reset_chip();
}

void cxd1185_device::reset_chip()
{
	m_pio_data_mode = false;
	m_pio_ctrl_mode = false;

	// clear all except environment register
	m_status = 0;
	m_command = 0;
	m_int_req[0] = 0;
	m_int_req[1] = 0;
	m_sel_time = 0;
	m_rst_time = 0;
	m_scsi_id = 0;
	m_int_auth[0] = 0;
	m_int_auth[1] = 0;
	m_mode = 0;
	m_count = 0;
	m_sync_ctrl = 0;
	m_scsi_ctrl = 0;
	m_ioport = 0;

	// clear drq and irq
	reset_fifo();
	int_check();

	// clear scsi bus
	scsi_bus->data_w(scsi_refid, 0);
	scsi_bus->ctrl_w(scsi_refid, 0, S_ALL);
}

void cxd1185_device::reset_fifo()
{
	m_fifo.clear();
	set_drq(false);
}

u8 cxd1185_device::status_r()
{
	u8 const data = (scsi_bus->ctrl_r() & S_RST) ? MRST : 0;

	LOGMASKED(LOG_REG, "status_r 0x%02x\n", data | m_status);

	return data | m_status;
}

u8 cxd1185_device::scsi_data_r()
{
	u8 data = 0;

	if (!m_pio_data_mode)
	{
		if (!machine().side_effects_disabled())
		{
			data = m_fifo.dequeue();

			if (m_state != IDLE && !m_state_timer->enabled())
				m_state_timer->adjust(attotime::zero);
		}
		else
			data = m_fifo.peek();
	}
	else
		data = scsi_bus->data_r();

	LOGMASKED(LOG_REG, "scsi_data_r 0x%02x (%s)\n", data, machine().describe_context());

	return data;
}

template <unsigned Register> u8 cxd1185_device::int_req_r()
{
	u8 const data = m_int_req[Register];

	if (!machine().side_effects_disabled())
	{
		LOGMASKED(LOG_REG, "int_req_r<%d> 0x%02x\n", Register, data);

		m_int_req[Register] = 0;
		int_check();
	}

	return data;
}

u8 cxd1185_device::scsi_ctrl_monitor_r()
{
	u32 const ctrl = scsi_bus->ctrl_r();

	u8 const data =
		((ctrl & S_BSY) ? MBSY : 0) |
		((ctrl & S_SEL) ? MSEL : 0) |
		((ctrl & S_MSG) ? MMSG : 0) |
		((ctrl & S_CTL) ? MCD : 0) |
		((ctrl & S_INP) ? MIO : 0) |
		((ctrl & S_REQ) ? MREQ : 0) |
		((ctrl & S_ACK) ? MACK : 0) |
		((ctrl & S_ATN) ? MATN : 0);

	LOGMASKED(LOG_REG, "scsi_ctrl_monitor_r 0x%02x\n", data);

	return data;
}

u8 cxd1185_device::fifo_status_r()
{
	u8 const data =
		(m_fifo.empty() ? FIE : 0) |
		(m_fifo.full() ? FIF : 0) |
		(m_fifo.queue_length() & FC);

	LOGMASKED(LOG_REG, "fifo_status_r 0x%02x\n", data);

	return data;
}

void cxd1185_device::command_w(u8 data)
{
	LOGMASKED(LOG_REG, "command_w 0x%02x\n", data);

	// check command in progress
	if ((m_status & CIP) && (data != CMD_RESET))
		return;

	// check command mode/category
	switch (data & CAT)
	{
	case 0x00:
		// commands valid in any state
		break;
	case 0x40:
		// commands valid in disconnected state
		if (m_status & (INIT | TARG))
			return;
		break;
	case 0x80:
		// commands valid in target state
		if ((m_status & (INIT | TARG)) != TARG)
			return;
		fatalerror("cxd1185_device: target mode not implemented\n");
		break;
	case 0xc0:
		// commands valid in initiator state
		if ((m_status & (INIT | TARG)) != INIT)
			return;
		break;
	}

	m_command = data;
	m_status |= CIP;
	m_last_dma_direction = DMA_NONE;

	switch (data & (CAT | CMD))
	{
	case 0x00:
		LOGMASKED(LOG_CMD, "no operation\n");
		break;
	case 0x01:
		LOGMASKED(LOG_CMD, "reset chip\n");
		reset_chip();
		break;
	case 0x02:
		LOGMASKED(LOG_CMD, "assert scsi reset\n");
		m_state = BUS_RESET;
		break;
	case 0x03:
		LOGMASKED(LOG_CMD, "flush fifo\n");
		reset_fifo();
		break;
	case 0x04:
		LOGMASKED(LOG_CMD, "assert scsi control\n");
		m_pio_ctrl_mode = true;
		m_scsi_ctrl = 0;
		if ((m_status & (INIT | TARG)) == TARG)
			scsi_bus->ctrl_w(scsi_refid, 0, S_ALL & ~S_BSY);
		break;
	case 0x05:
		LOGMASKED(LOG_CMD, "deassert scsi control\n");
		m_pio_ctrl_mode = false;
		break;
	case 0x06:
		LOGMASKED(LOG_CMD, "assert scsi data\n");
		m_pio_data_mode = true;
		break;
	case 0x07:
		LOGMASKED(LOG_CMD, "deassert scsi data\n");
		m_pio_data_mode = false;
		break;

	case 0x40: LOGMASKED(LOG_CMD, "reselect\n"); break;
	case 0x41:
		LOGMASKED(LOG_CMD, "select target %d without atn\n", (m_scsi_id & TID) >> 5);
		m_status |= INIT;
		m_state = ARB_BUS_FREE;
		m_last_dma_direction = (m_command & DMA) ? DMA_OUT : DMA_NONE;
		break;
	case 0x42:
		LOGMASKED(LOG_CMD, "select target %d with atn\n", (m_scsi_id & TID) >> 5);
		m_status |= INIT;
		m_state = ARB_BUS_FREE;
		m_last_dma_direction = (m_command & DMA) ? DMA_OUT : DMA_NONE;
		break;
	case 0x43: LOGMASKED(LOG_CMD, "enable selection/reselection\n"); break;
	case 0x44: LOGMASKED(LOG_CMD, "disable selection/reselection\n"); break;

		// TODO: not implemented
	case 0x80: LOGMASKED(LOG_CMD, "send message\n"); break;
	case 0x81: LOGMASKED(LOG_CMD, "send status\n"); break;
	case 0x82: LOGMASKED(LOG_CMD, "send data\n"); break;
	case 0x83: LOGMASKED(LOG_CMD, "disconnect\n"); break;
	case 0x84: LOGMASKED(LOG_CMD, "receive message out\n"); break;
	case 0x85: LOGMASKED(LOG_CMD, "receive command\n"); break;
	case 0x86: LOGMASKED(LOG_CMD, "receive data\n"); break;

	case 0xc0:
		LOGMASKED(LOG_CMD, "transfer information\n");
		m_state = XFR_INFO;
		m_last_dma_direction = (m_command & DMA) ? ((scsi_bus->ctrl_r() & S_INP) ? DMA_IN : DMA_OUT) : DMA_NONE;
		break;
	case 0xc1:
		LOGMASKED(LOG_CMD, "transfer pad\n");
		m_state = XFR_INFO;
		m_last_dma_direction = (m_command & DMA) ? ((scsi_bus->ctrl_r() & S_INP) ? DMA_IN : DMA_OUT) : DMA_NONE;
		break;
	case 0xc2:
		LOGMASKED(LOG_CMD, "deassert ack\n");
		scsi_bus->ctrl_w(scsi_refid, 0, S_ACK);
		break;
	case 0xc3:
		LOGMASKED(LOG_CMD, "assert atn\n");
		scsi_bus->ctrl_w(scsi_refid, S_ATN, S_ATN);
		break;
	case 0xc4:
		LOGMASKED(LOG_CMD, "deassert atn\n");
		scsi_bus->ctrl_w(scsi_refid, 0, S_ATN);
		break;
	}

	if (m_state == IDLE)
	{
		// command completes immediately
		LOGMASKED(LOG_CMD, "command complete immediate\n");

		m_command = 0;
		m_status &= ~CIP;

		if (data != CMD_RESET)
		{
			m_int_req[1] |= FNC;
			int_check();
		}
	}
	else
		m_state_timer->adjust(attotime::zero);
}

void cxd1185_device::scsi_data_w(u8 data)
{
	LOGMASKED(LOG_REG, "scsi_data_w 0x%02x (%s)\n", data, machine().describe_context());

	if (!m_pio_data_mode)
	{
		m_fifo.enqueue(data);

		if (m_state != IDLE && !m_state_timer->enabled())
			m_state_timer->adjust(attotime::zero);
	}
	else
	{
		u32 const ctrl = scsi_bus->ctrl_r();

		if (!(m_int_req[1] & PHC) &&
			(((m_status & (INIT | TARG)) == INIT && !(ctrl & S_INP)) ||
			((m_status & (INIT | TARG)) == TARG && (ctrl & S_INP))))
			scsi_bus->data_w(scsi_refid, data);
	}
}

void cxd1185_device::environ_w(u8 data)
{
	bool const polarity = (data ^ m_environ) & SIRM;

	LOGMASKED(LOG_REG, "environ_w 0x%02x\n", data);
	if (data ^ m_environ)
		LOGMASKED(LOG_CONFIG, "%s mode, %s data bus parity, %s irq polarity, divider %d\n",
			(data & DIFE) ? "differential" : "single-ended",
			(data & DPEN) ? ((data & SDPM) ? "even" : "odd") : "no",
			(data & SIRM) ? "negative" : "positive",
			(data & FS) ? ((data & FS1) ? 2 : 3) : 4);

	m_environ = data;

	// update irq line if polarity changed
	if (polarity)
		m_irq_out_cb((m_environ & SIRM) ? !m_irq_asserted : m_irq_asserted);
}

void cxd1185_device::timer_w(u8 data)
{
	LOGMASKED(LOG_REG, "timer_w 0x%02x\n", data);

	unsigned const divisor = (m_environ & FS) ? ((m_environ & FS1) ? 2 : 3) : 4;
	if (m_mode & TMSL)
	{
		m_rst_time = double(divisor * (32 * data + 38)) / clock() * 1E+9;

		LOGMASKED(LOG_CONFIG, "reset timer %d ns\n", m_rst_time);
	}
	else
	{
		m_sel_time = double(divisor * (data + 1) * 8192) / clock() * 1E+9;

		LOGMASKED(LOG_CONFIG, "selection timer %d ns\n", m_sel_time);
	}
}

template <unsigned Register> void cxd1185_device::int_auth_w(u8 data)
{
	LOGMASKED(LOG_REG, "int_auth_w<%d> 0x%02x\n", Register, data);

	m_int_auth[Register] = data;

	int_check();
}

void cxd1185_device::scsi_ctrl_w(u8 data)
{
	LOGMASKED(LOG_REG, "scsi_ctrl_w 0x%02x (%s)\n", data, machine().describe_context());

	if (m_pio_ctrl_mode)
	{
		u32 nscsi_mask = S_BSY | S_SEL;

		if ((m_status & (INIT | TARG)) == TARG)
			nscsi_mask |= S_MSG | S_CTL | S_INP | S_REQ;
		else if ((m_status & (INIT | TARG)) == INIT)
			nscsi_mask |= S_ACK | S_ATN;

		u32 const nscsi_data =
			((data & ABSY) ? S_BSY : 0) |
			((data & ASEL) ? S_SEL : 0) |
			((data & AMSG) ? S_MSG : 0) |
			((data & ACD) ? S_CTL : 0) |
			((data & AIO) ? S_INP : 0) |
			((data & AREQ) ? S_REQ : 0) |
			((data & AACK) ? S_ACK : 0) |
			((data & AATN) ? S_ATN : 0);

		scsi_bus->ctrl_w(scsi_refid, nscsi_data, nscsi_mask);
	}
	else
		m_scsi_ctrl = data;
}

void cxd1185_device::ioport_w(u8 data)
{
	LOGMASKED(LOG_REG, "ioport_w 0x%02x\n", data);

	// update direction bits first
	m_ioport &= ~PCN;
	m_ioport |= data & PCN;

	u8 const mask = (m_ioport & PCN) >> 4;

	// update output bits
	m_ioport &= ~mask;
	m_ioport |= data & mask;

	m_port_out_cb(0, m_ioport & mask, mask);
}

void cxd1185_device::state_timer(s32 param)
{
	// step state machine
	int delay = state_step();

	// check for interrupts
	int_check();

	// check for data stall
	if (delay < 0)
		return;

	if (m_state == IDLE)
	{
		LOGMASKED(LOG_CMD, "command complete\n");
		m_status &= ~CIP;
		m_command = 0;
	}
	else
		m_state_timer->adjust(attotime::from_nsec(delay));
}

int cxd1185_device::state_step()
{
	int delay = 0;

	u8 const oid = 1 << ((m_scsi_id & OID) >> 0);
	u8 const tid = 1 << ((m_scsi_id & TID) >> 5);

	switch (m_state)
	{
	case IDLE:
		break;

	case ARB_BUS_FREE:
		LOGMASKED(LOG_STATE, "arbitration: waiting for bus free\n");
		if (!(scsi_bus->ctrl_r() & (S_SEL | S_BSY | S_RST)))
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
		if ((scsi_bus->ctrl_r() & S_SEL) || (scsi_bus->data_r() & ~((oid - 1) | oid)))
		{
			LOGMASKED(LOG_STATE, "arbitration: lost\n");
			m_status &= ~INIT;
			m_int_req[0] |= ARBF;

			m_state = COMPLETE;

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
		delay = std::max(m_sel_time, SCSI_BUS_SETTLE);

		// clear BSY, optionally assert ATN
		if (m_command == CMD_SEL_ATN)
			scsi_bus->ctrl_w(scsi_refid, S_ATN, S_BSY | S_ATN);
		else
			scsi_bus->ctrl_w(scsi_refid, 0, S_BSY);
		break;
	case SEL_WAIT_BSY:
		if (scsi_bus->ctrl_r() & S_BSY)
		{
			LOGMASKED(LOG_STATE, "selection: BSY asserted by target\n");
			m_state = SEL_COMPLETE;
			delay = SCSI_BUS_SKEW * 2;
		}
		else
		{
			LOGMASKED(LOG_STATE, "selection: timed out\n");
			m_status &= ~INIT;
			m_int_req[0] |= STO;
			m_state = COMPLETE;

			scsi_bus->ctrl_w(scsi_refid, 0, S_ATN | S_SEL);
		}
		break;
	case SEL_COMPLETE:
		LOGMASKED(LOG_STATE, "selection: complete\n");
		m_state = COMPLETE;

		// clear data and SEL
		scsi_bus->data_w(scsi_refid, 0);
		scsi_bus->ctrl_w(scsi_refid, 0, S_SEL);
		break;

	case XFR_INFO:
		LOGMASKED(LOG_STATE, "transfer: count %d waiting for REQ\n", (m_command & TRBE) ? m_count : 1);
		if (scsi_bus->ctrl_r() & S_REQ)
			m_state = scsi_bus->ctrl_r() & S_INP ? XFR_IN : XFR_OUT;
		break;
	case XFR_IN:
		// FIXME: datasheet says ACK should be asserted when TRBE & FIF
		if (!m_fifo.full())
		{
			u8 const data = ((m_command & CMD) == (CMD_XFR_PAD & CMD)) ? 0 : scsi_bus->data_r();
			LOGMASKED(LOG_STATE, "transfer in: data 0x%02x\n", data);

			m_fifo.enqueue(data);
			if (m_command & TRBE)
				m_count--;

			m_state = XFR_IN_NEXT;

			// assert ACK
			scsi_bus->ctrl_w(scsi_refid, S_ACK, S_ACK);

			// If this is a DMA command and we have data now, assert DRQ so host can start transferring
			// (also handles transfers in the case where the counter is not aligned with the expected bytes read)
			if (m_command & DMA)
				set_drq(true);
		}
		else
		{
			delay = -1;
			if (m_command & DMA)
				set_drq(true);
		}
		break;
	case XFR_IN_NEXT:
		if (!(scsi_bus->ctrl_r() & S_REQ))
		{
			LOGMASKED(LOG_STATE, "transfer in: count %d\n", (m_command & TRBE) ? m_count : 0);
			if (!(m_command & TRBE) || !m_count)
			{
				if (m_command & TRBE)
					m_status |= TRBZ;

				m_state = XFR_IN_DRAIN;
			}
			else
				m_state = XFR_IN_REQ;

			// clear ACK except for single-byte message-in
			if (!((scsi_bus->ctrl_r() & S_PHASE_MASK) == S_PHASE_MSG_IN && !(m_command & TRBE)))
				scsi_bus->ctrl_w(scsi_refid, 0, S_ACK);
		}
		break;
	case XFR_IN_REQ:
		if (scsi_bus->ctrl_r() & S_REQ)
		{
			// check if target changed phase
			if (m_int_req[1] & PHC)
			{
				// Lower DRQ unless FIFO still has valid data that the host machine can read out
				if ((m_command & DMA) && m_fifo.empty())
					set_drq(false);

				m_state = XFR_INFO_DONE;
			}
			else
				m_state = XFR_IN;
		}
		break;
	case XFR_IN_DRAIN:
		if (!m_fifo.empty() && (m_command & DMA))
			set_drq(true);
		m_state = XFR_INFO_DONE;
		break;
	case XFR_OUT:
		if (!m_fifo.empty() || (m_command & CMD) == (CMD_XFR_PAD & CMD))
		{
			u8 const data = ((m_command & CMD) == (CMD_XFR_PAD & CMD)) ? 0 : m_fifo.dequeue();

			LOGMASKED(LOG_STATE, "transfer out: data 0x%02x\n", data);
			m_state = XFR_OUT_NEXT;

			// assert data and ACK
			scsi_bus->data_w(scsi_refid, data);
			scsi_bus->ctrl_w(scsi_refid, S_ACK, S_ACK);
		}
		else
		{
			delay = -1;
			if (m_command & DMA)
				set_drq(true);
		}
		break;
	case XFR_OUT_NEXT:
		if (!(scsi_bus->ctrl_r() & S_REQ))
		{
			LOGMASKED(LOG_STATE, "transfer out: data ACK\n");
			if (m_command & TRBE)
			{
				if (!--m_count)
				{
					m_status |= TRBZ;
					m_state = XFR_INFO_DONE;
				}
				else
					m_state = XFR_OUT_REQ;
			}
			else
				m_state = XFR_INFO_DONE;

			// clear data and ACK
			scsi_bus->data_w(scsi_refid, 0);
			scsi_bus->ctrl_w(scsi_refid, 0, S_ACK);
		}
		break;
	case XFR_OUT_REQ:
		LOGMASKED(LOG_STATE, "transfer out: count %d waiting for REQ\n", m_count);
		if (scsi_bus->ctrl_r() & S_REQ)
		{
			// check if target changed phase
			if (m_int_req[1] & PHC)
			{
				if (m_command & DMA)
					set_drq(false);

				m_state = XFR_INFO_DONE;
			}
			else
				m_state = XFR_OUT;
		}
		break;
	case XFR_INFO_DONE:
		LOGMASKED(LOG_STATE, "transfer: complete\n");
		m_state = COMPLETE;
		break;

	case BUS_RESET:
		LOGMASKED(LOG_STATE, "bus reset: asserted\n");
		m_status &= ~(INIT | TARG);
		m_int_req[1] |= SRST;

		m_state = BUS_RESET_DONE;
		delay = (m_mode & TMSL) ? m_rst_time : SCSI_RST_HOLD;

		// clear data and assert RST
		scsi_bus->data_w(scsi_refid, 0);
		scsi_bus->ctrl_w(scsi_refid, S_RST, S_ALL);
		break;
	case BUS_RESET_DONE:
		LOGMASKED(LOG_STATE, "bus reset: complete\n");
		if (m_mode & TMSL)
			m_int_req[0] |= STO;
		m_state = COMPLETE;

		// clear RST
		scsi_bus->ctrl_w(scsi_refid, 0, S_RST);
		break;

	case COMPLETE:
		LOGMASKED(LOG_STATE, "function complete\n");
		m_int_req[1] |= FNC;
		m_state = IDLE;
		break;
	}

	return delay;
}

void cxd1185_device::scsi_ctrl_changed()
{
	u32 const ctrl = scsi_bus->ctrl_r();

	if ((ctrl & S_BSY) && !(ctrl & S_SEL))
		LOGMASKED(LOG_SCSI, "scsi_ctrl_changed 0x%x phase %s%s%s\n", ctrl, nscsi_phase[ctrl & S_PHASE_MASK],
			ctrl & S_REQ ? " REQ" : "", ctrl & S_ACK ? " ACK" : "");
	else if (ctrl & S_BSY)
		LOGMASKED(LOG_SCSI, "scsi_ctrl_changed 0x%x arbitration/selection\n", ctrl);
	else if (ctrl & S_RST)
		LOGMASKED(LOG_SCSI, "scsi_ctrl_changed 0x%x BUS RESET\n", ctrl);
	else
		LOGMASKED(LOG_SCSI, "scsi_ctrl_changed 0x%x BUS FREE\n", ctrl);

	if (ctrl & S_RST)
	{
		m_status &= ~(INIT | TARG);
		m_int_req[1] |= SRST;

		// clear data and ctrl
		scsi_bus->data_w(scsi_refid, 0);
		scsi_bus->ctrl_w(scsi_refid, 0, S_ALL);
	}
	else if ((m_status & (TARG | INIT)) == INIT)
	{
		if ((ctrl & S_SEL) && !(m_scsi_ctrl_state & S_BSY) && (ctrl & S_BSY))
		{
			LOGMASKED(LOG_SCSI, "target selected\n");

			// truncate selection delay
			m_state_timer->adjust(attotime::zero);
		}
		else if ((m_scsi_ctrl_state & S_BSY) && !(ctrl & S_BSY))
		{
			LOGMASKED(LOG_SCSI, "target disconnected\n");

			m_status &= ~INIT;
			m_int_req[1] |= DCNT;
		}
		else if ((ctrl ^ m_scsi_ctrl_state) & S_PHASE_MASK)
		{
			if (ctrl & S_REQ)
			{
				LOGMASKED(LOG_SCSI, "target changed phase\n");

				m_int_req[1] |= PHC;
				if (ctrl & S_MSG)
					m_int_req[1] |= RMSG;
			}
			else
				// ignore until req asserted
				return;
		}
	}
	else if ((m_status & (TARG | INIT)) == TARG)
	{
		if (!(m_scsi_ctrl_state & S_ATN) && (ctrl & S_ATN))
		{
			LOGMASKED(LOG_SCSI, "initiator asserted attention\n");

			m_int_req[1] |= DATN;
		}
	}

	// record state
	m_scsi_ctrl_state = ctrl;

	int_check();
}

void cxd1185_device::int_check()
{
	bool irq_asserted = false;

	// When SPHI is set during DMA data in, phase change interrupts are deferred until the DMA transfer is complete
	// This will be recalculated at that time once the FIFO is empty
	uint8_t int_req1 = m_int_req[1];
	if ((m_mode & SPHI) && !m_fifo.empty() && (m_last_dma_direction == DMA_IN) && (int_req1 & PHC))
	{
		LOGMASKED(LOG_INT, "SPHI active, suppressing PHC interrupt\n");
		int_req1 &= ~PHC;
	}

	// update mirq
	if (m_int_req[0] || int_req1)
	{
		m_status |= MIRQ;

		irq_asserted = (m_int_req[0] & m_int_auth[0]) || (int_req1 & m_int_auth[1]);
	}
	else
		m_status &= ~MIRQ;

	// update irq line
	if (m_irq_asserted != irq_asserted)
	{
		LOGMASKED(LOG_INT, "irq_check interrupt %s\n", irq_asserted ? "asserted" : "cleared");

		m_irq_asserted = irq_asserted;
		m_irq_out_cb((m_environ & SIRM) ? !m_irq_asserted : m_irq_asserted);
	}
}

void cxd1185_device::set_drq(bool asserted)
{
	if (m_drq_asserted != asserted)
	{
		LOGMASKED(LOG_DMA, "set_drq %s\n", asserted ? "asserted" : "deasserted");
		m_drq_asserted = asserted;
		m_drq_out_cb(m_drq_asserted);
	}
}

u8 cxd1185_device::dma_r()
{
	u8 const data = m_fifo.dequeue();

	LOGMASKED(LOG_DMA, "dma_r 0x%02x\n", data);

	if (m_fifo.empty())
	{
		set_drq(false);

		if (m_count)
			m_state_timer->adjust(attotime::zero);
	}

	return data;
}

void cxd1185_device::dma_w(u8 data)
{
	LOGMASKED(LOG_DMA, "dma_w 0x%02x\n", data);

	m_fifo.enqueue(data);

	if (m_fifo.full() || m_fifo.queue_length() >= m_count)
	{
		set_drq(false);

		if (m_count)
			m_state_timer->adjust(attotime::zero);
	}
}

void cxd1185_device::port_w(u8 data)
{
	u8 const mask = ~(PCN | ((m_ioport & PCN) >> 4));

	LOG("port_w 0x%02x mask 0x%02x\n", data, mask);

	m_ioport &= ~mask;
	m_ioport |= data & mask;
}
