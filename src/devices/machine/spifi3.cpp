// license:BSD-3-Clause
// copyright-holders:Brice Onken,Olivier Galibert

/*
 * HP 1TV3-0302 SPIFI3-SE SCSI controller
 *
 * References:
 * - https://github.com/NetBSD/src/blob/trunk/sys/arch/newsmips/apbus/spifireg.h
 * - https://github.com/NetBSD/src/blob/trunk/sys/arch/newsmips/apbus/spifi.c
 * - https://github.com/mamedev/mame/blob/master/src/devices/machine/ncr5390.cpp
 *
 * TODO:
 *  - NetBSD compatibility
 *  - Reselection, target mode, SDTR
 *  - LUN selection (currently assumes 0)
 *  - Non-chip-reset conditions
 *  - Other SPSTAT and ICOND values
 *  - CMDPAGE details
 *  - Anything the Sony NEWS driver doesn't use
 */

#include "emu.h"
#include "spifi3.h"

#define LOG_GENERAL (1U << 0)
#define LOG_STATE (1U << 1)
#define LOG_INTERRUPT (1U << 2)
#define LOG_DATA (1U << 3)
#define LOG_REGISTER (1U << 4)
#define LOG_CMD (1U << 5)
#define LOG_AUTO (1U << 6)

#define SPIFI3_DEBUG (LOG_GENERAL | LOG_REGISTER | LOG_INTERRUPT | LOG_AUTO)
#define SPIFI3_TRACE (SPIFI3_DEBUG | LOG_STATE | LOG_CMD)
#define SPIFI3_MAX (SPIFI3_TRACE | LOG_DATA)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(SPIFI3, spifi3_device, "spifi3", "HP 1TV3-0302 SPIFI3 SCSI-2 Protocol Controller")

ALLOW_SAVE_TYPE(spifi3_device::scsi_mode)
ALLOW_SAVE_TYPE(spifi3_device::scsi_data_target)
ALLOW_SAVE_TYPE(spifi3_device::dma_direction)

namespace
{
	// AUXCTRL constants and functions
	constexpr uint32_t AUXCTRL_DMAEDGE = 0x04;
	constexpr uint32_t AUXCTRL_SETRST = 0x20;
	constexpr uint32_t AUXCTRL_CRST = 0x40;
	constexpr uint32_t AUXCTRL_SRST = 0x80;

	// spstat - not fully implemented yet
	constexpr uint32_t SPS_IDLE = 0x00;
	constexpr uint32_t SPS_MSGOUT = 0x04;
	constexpr uint32_t SPS_COMMAND = 0x05;
	constexpr uint32_t SPS_INTR = 0x08;
	constexpr uint32_t SPS_STATUS = 0x0c;
	constexpr uint32_t SPS_MSGIN = 0x0d;
	constexpr uint32_t SPS_DATAOUT = 0x0e;
	constexpr uint32_t SPS_DATAIN = 0x0f;

	// prstat - PRS_Z not implemented yet
	constexpr uint32_t PRS_IO = 0x08;
	constexpr uint32_t PRS_CD = 0x10;
	constexpr uint32_t PRS_MSG = 0x20;
	constexpr uint32_t PRS_ATN = 0x40;

	// Interrupt status register - Not all interrupts implemented yet
	constexpr uint32_t INTR_BSRQ = 0x01;
	constexpr uint32_t INTR_TIMEO = 0x08;
	constexpr uint32_t INTR_FCOMP = 0x80;

	// Interrupt condition register - Not all interrupts implemented yet
	constexpr uint32_t ICOND_CNTZERO = 0x40;
	constexpr uint32_t ICOND_UXPHASEZ = 0x80;

	// Select register - SETATN and IRESELEN not implemented yet
	constexpr uint32_t SEL_ISTART = 0x08;
	constexpr uint32_t SEL_WATN = 0x80;
	constexpr uint32_t SEL_TARGET = 0x70;

	// Autodata register
	constexpr uint32_t ADATA_IN = 0x40;
	constexpr uint32_t ADATA_EN = 0x80;
	constexpr uint32_t ADATA_TARGET_ID = 0x07;

	// cmlen register
	constexpr uint32_t CML_LENMASK = 0x0f;
	constexpr uint32_t CML_AMSG_EN = 0x40;
	constexpr uint32_t CML_ACOM_EN = 0x80;

	// init_status register
	constexpr uint32_t INIT_STATUS_ACK = 0x40;

	// FIFOCTRL constants and functions
	// Based on the existence of CLREVEN/ODD, the fact that NetBSD only uses EVEN, and the max is 8
	// even though this is a 4 bit value, it seems likely that there are actually two FIFOs,
	// one in the even slots, and one in the odd slots
	constexpr uint32_t FIFOC_FSLOT = 0x0f; // Free slots in FIFO, max 8. Free slots = 8 - (FIFOCTRL & FIFOC_FSLOT)
	constexpr uint32_t FIFOC_SSTKACT = 0x10;
	constexpr uint32_t FIFOC_RQOVRN = 0x20;
	constexpr uint32_t FIFOC_CLREVEN = 0x00;
	constexpr uint32_t FIFOC_CLRODD = 0x40;
	constexpr uint32_t FIFOC_FLUSH = 0x80;
	constexpr uint32_t FIFOC_LOAD = 0xc0;

	// SPIFI commands
	enum PRCMD_COMMANDS : uint32_t
	{
		PRC_DATAOUT = 0x0,
		PRC_DATAIN = 0x1,
		PRC_COMMAND = 0x2,
		PRC_STATUS = 0x3,
		PRC_TRPAD = 0x4,
		PRC_MSGOUT = 0x6,
		PRC_MSGIN = 0x7,
		PRC_KILLREQ = 0x08,
		PRC_CLRACK = 0x10,
		PRC_NJMP = 0x80
	};

	constexpr uint32_t PRCMD_MASK = 0x1f;
	const std::string_view prcmd_command_names[9] = {"PRC_DATAOUT", "PRC_DATAIN", "PRC_COMMAND", "PRC_STATUS", "PRC_TRPAD", "UNKNOWN", "PRC_MSGOUT", "PRC_MSGIN", "PRC_KILLREQ"};

	enum
	{
		IDLE
	};

	enum
	{
		// Bus initiated sequences
		BUSINIT_SETTLE_DELAY = 1,
		BUSINIT_ASSERT_BUS_SEL,
		BUSINIT_MSG_OUT,
		BUSINIT_RECV_BYTE,
		BUSINIT_ASSERT_BUS_RESEL,
		BUSINIT_WAIT_REQ,
		BUSINIT_RECV_BYTE_NACK,

		// Bus SCSI Reset
		BUSRESET_WAIT_INT,
		BUSRESET_RESET_BOARD,

		// Disconnected state commands
		DISC_SEL_ARBITRATION_INIT,
		DISC_SEL_ARBITRATION,
		DISC_SEL_ATN_WAIT_REQ,
		DISC_SEL_ATN_SEND_BYTE,
		DISC_SEL_WAIT_REQ,
		DISC_SEL_SEND_BYTE,
		DISC_REC_ARBITRATION,
		DISC_REC_MSG_IN,
		DISC_REC_SEND_BYTE,
		DISC_RESET,

		// Command sequence
		CMDSEQ_CMD_PHASE,
		CMDSEQ_RECV_BYTE,

		// Target commands
		TARGET_SEND_BYTE,
		TARGET_CMD_RECV_BYTE,
		TARGET_MSG_RECV_BYTE,
		TARGET_MSG_RECV_PAD,
		TARGET_DISC_SEND_BYTE,
		TARGET_DISC_MSG_IN,
		TARGET_DISC_SEND_BYTE_2,

		// Initiator commands
		INIT_MSG_WAIT_REQ,
		INIT_XFR,
		INIT_XFR_SEND_BYTE,
		INIT_XFR_SEND_PAD_WAIT_REQ,
		INIT_XFR_SEND_PAD,
		INIT_XFR_RECV_PAD_WAIT_REQ,
		INIT_XFR_RECV_PAD,
		INIT_XFR_RECV_BYTE_ACK,
		INIT_XFR_RECV_BYTE_NACK,
		INIT_XFR_FUNCTION_COMPLETE,
		INIT_XFR_BUS_COMPLETE,
		INIT_XFR_WAIT_REQ,
		INIT_CPT_RECV_BYTE_ACK,
		INIT_CPT_RECV_WAIT_REQ,
		INIT_CPT_RECV_BYTE_NACK,
		INIT_XFR_RECV_BYTE_ACK_AUTOMSG
	};

	enum
	{
		// Arbitration
		ARB_WAIT_BUS_FREE = 1,
		ARB_COMPLETE,
		ARB_ASSERT_SEL,
		ARB_SET_DEST,
		ARB_RELEASE_BUSY,
		ARB_TIMEOUT_BUSY,
		ARB_TIMEOUT_ABORT,
		ARB_DESKEW_WAIT,

		// Send/receive byte
		SEND_WAIT_SETTLE,
		SEND_WAIT_REQ_0,
		RECV_WAIT_REQ_1,
		RECV_WAIT_SETTLE,
		RECV_WAIT_REQ_0
	};

	enum
	{
		STATE_MASK = 0x00ff,
		SUB_SHIFT = 8,
		SUB_MASK = 0xff00
	};

	enum
	{
		BUS_BUSY,
		BUS_FREE_WAIT,
		BUS_FREE
	};

	// Helper functions
	inline uint32_t prcmd_to_spstat(uint32_t cmd)
	{
		uint32_t spstat_val = 0;
		switch (cmd)
		{
			case PRC_DATAIN:
				spstat_val = SPS_DATAIN;
				break;
			case PRC_DATAOUT:
				spstat_val = SPS_DATAOUT;
				break;
			case PRC_COMMAND:
				spstat_val = SPS_COMMAND;
				break;
			case PRC_STATUS:
				spstat_val = SPS_STATUS;
				break;
			case PRC_MSGOUT:
				spstat_val = SPS_MSGOUT;
				break;
			case PRC_MSGIN:
				spstat_val = SPS_MSGIN;
				break;
		}
		return spstat_val;
	}
}

spifi3_device::spifi3_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
	nscsi_device(mconfig, SPIFI3, tag, owner, clock),
	nscsi_slot_card_interface(mconfig, *this, DEVICE_SELF),
	m_even_fifo(),
	m_odd_fifo(),
	m_irq_handler(*this),
	m_drq_handler(*this)
{
}

void spifi3_device::device_start()
{
	nscsi_device::device_start();

	save_item(NAME(dma_dir));
	save_item(NAME(mode));
	save_item(NAME(xfr_data_source));
	save_item(NAME(state));
	save_item(NAME(xfr_phase));
	save_item(NAME(command_pos));
	save_item(NAME(irq));
	save_item(NAME(drq));
	save_item(NAME(tcounter));
	save_item(NAME(sync_period));
	save_item(NAME(clock_conv));
	save_item(NAME(bus_id));
	save_item(STRUCT_MEMBER(spifi_reg, spstat));
	save_item(STRUCT_MEMBER(spifi_reg, cmlen));
	save_item(STRUCT_MEMBER(spifi_reg, cmdpage));
	save_item(STRUCT_MEMBER(spifi_reg, svptr_hi));
	save_item(STRUCT_MEMBER(spifi_reg, svptr_mid));
	save_item(STRUCT_MEMBER(spifi_reg, svptr_low));
	save_item(STRUCT_MEMBER(spifi_reg, intr));
	save_item(STRUCT_MEMBER(spifi_reg, imask));
	save_item(STRUCT_MEMBER(spifi_reg, prctrl));
	save_item(STRUCT_MEMBER(spifi_reg, prstat));
	save_item(STRUCT_MEMBER(spifi_reg, init_status));
	save_item(STRUCT_MEMBER(spifi_reg, fifoctrl));
	save_item(STRUCT_MEMBER(spifi_reg, fifodata));
	save_item(STRUCT_MEMBER(spifi_reg, config));
	save_item(STRUCT_MEMBER(spifi_reg, data_xfer));
	save_item(STRUCT_MEMBER(spifi_reg, autocmd));
	save_item(STRUCT_MEMBER(spifi_reg, autostat));
	save_item(STRUCT_MEMBER(spifi_reg, resel));
	save_item(STRUCT_MEMBER(spifi_reg, select));
	save_item(STRUCT_MEMBER(spifi_reg, auxctrl));
	save_item(STRUCT_MEMBER(spifi_reg, autodata));
	save_item(STRUCT_MEMBER(spifi_reg, loopctrl));
	save_item(STRUCT_MEMBER(spifi_reg, loopdata));
	save_item(STRUCT_MEMBER(spifi_reg, identify));
	save_item(STRUCT_MEMBER(spifi_reg, complete));
	save_item(STRUCT_MEMBER(spifi_reg, scsi_status));
	save_item(STRUCT_MEMBER(spifi_reg, data));
	save_item(STRUCT_MEMBER(spifi_reg, icond));
	save_item(STRUCT_MEMBER(spifi_reg, fastwide));
	save_item(STRUCT_MEMBER(spifi_reg, exctrl));
	save_item(STRUCT_MEMBER(spifi_reg, exstat));
	save_item(STRUCT_MEMBER(spifi_reg, test));
	save_item(STRUCT_MEMBER(spifi_reg, quematch));
	save_item(STRUCT_MEMBER(spifi_reg, quecode));
	save_item(STRUCT_MEMBER(spifi_reg, quetag));
	save_item(STRUCT_MEMBER(spifi_reg, quepage));
	save_item(STRUCT_MEMBER(spifi_reg.cmbuf, cdb));
	save_item(STRUCT_MEMBER(spifi_reg.cmbuf, quecode));
	save_item(STRUCT_MEMBER(spifi_reg.cmbuf, quetag));
	save_item(STRUCT_MEMBER(spifi_reg.cmbuf, idmsg));
	save_item(STRUCT_MEMBER(spifi_reg.cmbuf, status));
	save_item(STRUCT_MEMBER(m_even_fifo, head));
	save_item(STRUCT_MEMBER(m_even_fifo, tail));
	save_item(STRUCT_MEMBER(m_even_fifo, size));
	save_item(STRUCT_MEMBER(m_even_fifo, fifo));
	save_item(STRUCT_MEMBER(m_odd_fifo, head));
	save_item(STRUCT_MEMBER(m_odd_fifo, tail));
	save_item(STRUCT_MEMBER(m_odd_fifo, size));
	save_item(STRUCT_MEMBER(m_odd_fifo, fifo));

	m_irq_handler.resolve_safe();
	m_drq_handler.resolve_safe();

	bus_id = 0;
	tm = timer_alloc(FUNC(spifi3_device::tick), this);
}

void spifi3_device::map(address_map &map)
{
	map(0x00, 0x03).r(FUNC(spifi3_device::spstat_r));
	map(0x04, 0x07).rw(FUNC(spifi3_device::cmlen_r), FUNC(spifi3_device::cmlen_w));
	map(0x08, 0x0b).rw(FUNC(spifi3_device::cmdpage_r), FUNC(spifi3_device::cmdpage_w));
	map(0x0c, 0x17).rw(FUNC(spifi3_device::count_r), FUNC(spifi3_device::count_w));
	// svptr
	map(0x24, 0x27).rw(FUNC(spifi3_device::intr_r), FUNC(spifi3_device::intr_w));
	map(0x28, 0x2b).rw(FUNC(spifi3_device::imask_r), FUNC(spifi3_device::imask_w));
	// prctrl
	map(0x30, 0x33).r(FUNC(spifi3_device::prstat_r));
	map(0x34, 0x37).r(FUNC(spifi3_device::init_status_r));
	map(0x38, 0x3b).rw(FUNC(spifi3_device::fifoctrl_r), FUNC(spifi3_device::fifoctrl_w));
	// fifodata, config
	map(0x44, 0x47).rw(FUNC(spifi3_device::data_xfer_r), FUNC(spifi3_device::data_xfer_w));
	map(0x48, 0x4b).rw(FUNC(spifi3_device::autocmd_r), FUNC(spifi3_device::autocmd_w));
	map(0x4c, 0x4f).rw(FUNC(spifi3_device::autostat_r), FUNC(spifi3_device::autostat_w));
	// resel
	map(0x54, 0x57).rw(FUNC(spifi3_device::select_r), FUNC(spifi3_device::select_w));
	map(0x58, 0x5b).w(FUNC(spifi3_device::prcmd_w));
	map(0x5c, 0x5f).rw(FUNC(spifi3_device::auxctrl_r), FUNC(spifi3_device::auxctrl_w));
	map(0x60, 0x63).rw(FUNC(spifi3_device::autodata_r), FUNC(spifi3_device::autodata_w));
	// loopctrl, loopdata
	map(0x6c, 0x6f).rw(FUNC(spifi3_device::identify_r), FUNC(spifi3_device::identify_w));
	// complete
	map(0x74, 0x77).rw(FUNC(spifi3_device::scsi_status_r), FUNC(spifi3_device::scsi_status_w));
	// data
	map(0x7c, 0x7f).rw(FUNC(spifi3_device::icond_r), FUNC(spifi3_device::icond_w));
	map(0x80, 0x83).rw(FUNC(spifi3_device::fastwide_r), FUNC(spifi3_device::fastwide_w));
	map(0x84, 0x87).rw(FUNC(spifi3_device::exctrl_r), FUNC(spifi3_device::exctrl_w));
	// exstat, test, quematch, quecode, quetag, quepage

	map(0x200, 0x3ff).rw(FUNC(spifi3_device::cmd_buf_r), FUNC(spifi3_device::cmd_buf_w)).umask32(0xff);
}

uint32_t spifi3_device::spstat_r()
{
	const uint32_t spstat = (spifi_reg.spstat << 4) | ((spifi_reg.intr > 0) ? SPS_INTR : 0);
	LOGMASKED(LOG_REGISTER, "read spifi_reg.spstat = 0x%x\n", spstat);
	return spstat;
}

uint32_t spifi3_device::cmlen_r()
{
	LOGMASKED(LOG_REGISTER, "read spifi_reg.cmlen = 0x%x\n", spifi_reg.cmlen);
	return spifi_reg.cmlen;
}

void spifi3_device::cmlen_w(uint32_t data)
{
	LOGMASKED(LOG_REGISTER, "write spifi_reg.cmlen = 0x%x\n", data);
	spifi_reg.cmlen = data;

	// Not sure if this is where this is actually cleared.
	// Putting it here prevents NEWS-OS from trying to
	// transition to the DATAOUT phase too early when it sees
	// the CNTZERO condition flag
	spifi_reg.icond &= ~ICOND_CNTZERO;
}

uint32_t spifi3_device::cmdpage_r()
{
	LOGMASKED(LOG_REGISTER, "read spifi_reg.cmdpage = 0x%x\n", spifi_reg.cmdpage);
	return spifi_reg.cmdpage;
}

void spifi3_device::cmdpage_w(uint32_t data)
{
	LOGMASKED(LOG_REGISTER, "write spifi_reg.cmdpage = 0x%x\n", data);
	spifi_reg.cmdpage = data;
}

uint32_t spifi3_device::count_r(offs_t offset)
{
	const uint8_t count = (tcounter >> (8 * (2 - offset))) & 0xff;
	LOGMASKED(LOG_REGISTER, "read spifi_reg.count%d = 0x%x -> 0x%x\n", offset, tcounter, count);
	return count;
}

void spifi3_device::count_w(offs_t offset, uint32_t data)
{
	spifi_reg.icond &= ~ICOND_CNTZERO;
	tcounter &= ~(0xff << (8 * (2 - offset)));
	tcounter |= (data & 0xff) << (8 * (2 - offset));
	LOGMASKED(LOG_REGISTER, "write spifi_reg.count%d = 0x%x -> 0x%x\n", offset, data, tcounter);
}

uint32_t spifi3_device::intr_r()
{
	LOGMASKED(LOG_REGISTER, "read spifi_reg.intr = 0x%x (%s)\n", spifi_reg.intr, machine().describe_context());
	return spifi_reg.intr;
}

void spifi3_device::intr_w(uint32_t data)
{
	LOGMASKED(LOG_REGISTER, "write spifi_reg.intr = 0x%x\n", data);
	spifi_reg.intr &= data;
	spifi_reg.icond = 0;
	check_irq();
}

uint32_t spifi3_device::imask_r()
{
	LOGMASKED(LOG_REGISTER, "read spifi_reg.imask = 0x%x\n", spifi_reg.imask);
	return spifi_reg.imask;
}

void spifi3_device::imask_w(uint32_t data)
{
	LOGMASKED(LOG_REGISTER, "write spifi_reg.imask = 0x%x\n", data);
	spifi_reg.imask = data;
}

uint32_t spifi3_device::prstat_r()
{
	auto ctrl = scsi_bus->ctrl_r();

	// TODO: PRS_Z (disconnect state?)
	uint32_t prstat = 0;
	prstat |= (ctrl & S_ATN) ? PRS_ATN : 0;
	prstat |= (ctrl & S_MSG) ? PRS_MSG : 0;
	prstat |= (ctrl & S_CTL) ? PRS_CD : 0;
	prstat |= (ctrl & S_INP) ? PRS_IO : 0;
	spifi_reg.prstat = prstat; // Might be able to get rid of the register copy of this since we can compute it on demand.
	LOGMASKED(LOG_REGISTER, "read spifi_reg.prstat = 0x%x\n", prstat);
	return prstat;
}

uint32_t spifi3_device::init_status_r()
{
	// NetBSD only lists this bit, but there is probably more in this register.
	const auto init_status = (scsi_bus->ctrl_r() & S_ACK) > 0 ? INIT_STATUS_ACK : 0x0;
	LOGMASKED(LOG_REGISTER, "read spifi_reg.init_status = 0x%x\n", init_status);
	return init_status;
}

uint32_t spifi3_device::fifoctrl_r()
{
	LOGMASKED(LOG_REGISTER, "read spifi_reg.fifoctrl = 0x%x\n", spifi_reg.fifoctrl);

	const auto evenCount = FIFO_SIZE - m_even_fifo.get_size();
	spifi_reg.fifoctrl &= ~FIFOC_FSLOT;
	spifi_reg.fifoctrl |= evenCount & FIFOC_FSLOT;

	return spifi_reg.fifoctrl;
}

void spifi3_device::fifoctrl_w(uint32_t data)
{
	LOGMASKED(LOG_REGISTER, "write spifi_reg.fifoctrl = 0x%x\n", data);
	spifi_reg.fifoctrl = data & ~FIFOC_FSLOT; // TODO: this might not be persisted - read/write might be different. TBD.
	if (spifi_reg.fifoctrl & FIFOC_SSTKACT)
	{
		LOG("fifoctrl.SSTKACT: w unimplemented");
	} // likely RO guess: NetBSD uses this to know when synchronous data should be loaded into the FIFO?
	if (spifi_reg.fifoctrl & FIFOC_RQOVRN)
	{
		LOG("fifoctrl.RQOVRN: w unimplemented");
	} // likely RO - probably fifo overrun. Whatever this is, it would cause NetBSD to panic
	if (spifi_reg.fifoctrl & FIFOC_CLREVEN)
	{
		LOG("Clearing even FIFO of %d items\n", m_even_fifo.get_size());
		m_even_fifo.clear_queue();
	}
	if (spifi_reg.fifoctrl & FIFOC_CLRODD)
	{
		LOG("Clearing odd FIFO of %d items\n", m_odd_fifo.get_size());
		m_odd_fifo.clear_queue();
	}
	if (spifi_reg.fifoctrl & FIFOC_FLUSH)
	{
		LOG("fifoctrl.FLUSH: unimplemented");
	} // flush FIFO - kick off DMA regardless of FIFO count, I assume
	if (spifi_reg.fifoctrl & FIFOC_LOAD)
	{
		LOG("fifoctrl.LOAD: unimplemented");
	} // Load FIFO synchronously (only needed for SDTR mode?)
}

uint32_t spifi3_device::data_xfer_r()
{
	LOGMASKED(LOG_REGISTER, "read spifi_reg.data_xfer = 0x%x\n", spifi_reg.data_xfer);
	return spifi_reg.data_xfer;
}

void spifi3_device::data_xfer_w(uint32_t data)
{
	LOGMASKED(LOG_REGISTER, "write spifi_reg.data_xfer = 0x%x\n", data);
	spifi_reg.data_xfer = data;
}

uint32_t spifi3_device::autocmd_r()
{
	LOGMASKED(LOG_REGISTER, "read spifi_reg.autocmd = 0x%x\n", spifi_reg.autocmd);
	return spifi_reg.autocmd;
}

void spifi3_device::autocmd_w(uint32_t data)
{
	LOGMASKED(LOG_REGISTER, "write spifi_reg.autocmd = 0x%x\n", data);
	spifi_reg.autocmd = data;
}

uint32_t spifi3_device::autodata_r()
{
	LOGMASKED(LOG_REGISTER, "read spifi_reg.autodata = 0x%x\n", spifi_reg.autodata);
	return spifi_reg.autodata;
}

uint32_t spifi3_device::autostat_r()
{
	LOGMASKED(LOG_REGISTER, "read spifi_reg.autostat = 0x%x\n", spifi_reg.autostat);
	return spifi_reg.autostat;
}

void spifi3_device::autostat_w(uint32_t data)
{
	// This is based on what the NEWS-OS kernel does with this register.
	// NetBSD doesn't use this register the same way based on its source code.
	LOGMASKED(LOG_REGISTER, "write spifi_reg.autostat = 0x%x\n", data);
	spifi_reg.autostat |= data;
}

uint32_t spifi3_device::select_r()
{
	LOGMASKED(LOG_REGISTER, "read spifi_reg.select = 0x%x\n", spifi_reg.select);
	return spifi_reg.select;
}

void spifi3_device::select_w(uint32_t data)
{
	LOGMASKED(LOG_REGISTER, "write spifi_reg.select = 0x%x\n", data);
	spifi_reg.select = data & ~SEL_ISTART;

	if (data & SEL_ISTART)
	{
		const auto target_id = (data & SEL_TARGET) >> 4;
		LOGMASKED(LOG_AUTO, "Select started! Targeting ID %d\n", target_id);

		// Selects cmbuf entry, maybe? - can be manually set before a command based on NetBSD source, not supported yet
		spifi_reg.cmdpage = target_id;
		state = DISC_SEL_ARBITRATION_INIT;
		arbitrate();
	}
}

uint32_t spifi3_device::auxctrl_r()
{
	LOGMASKED(LOG_REGISTER, "read spifi_reg.auxctrl = 0x%x\n", spifi_reg.auxctrl);
	return spifi_reg.auxctrl;
}

void spifi3_device::auxctrl_w(uint32_t data)
{
	LOGMASKED(LOG_REGISTER, "write spifi_reg.auxctrl = 0x%x\n", data);
	const auto prev_auxctrl = spifi_reg.auxctrl;
	spifi_reg.auxctrl = data;
	if (spifi_reg.auxctrl & AUXCTRL_SRST)
	{
		// TODO: reset of some kind
		LOG("SRST asserted\n");
	}
	if (spifi_reg.auxctrl & AUXCTRL_CRST)
	{
		LOG("chip reset\n");
		spifi_reg = {};
		dma_dir = DMA_NONE;
		tcounter = 0;
		command_pos = 0;
	}
	if ((spifi_reg.auxctrl & AUXCTRL_SETRST) && !(prev_auxctrl & AUXCTRL_SETRST))
	{
		LOG("SETRST asserted - resetting SCSI bus\n");
		state = BUSRESET_WAIT_INT;
		scsi_bus->ctrl_w(scsi_refid, S_RST, S_RST);
		delay(130);
	}
	if (spifi_reg.auxctrl & AUXCTRL_DMAEDGE)
	{
		// TODO: do we need to take action here?
		LOG("DMAEDGE asserted\n");
	}
}

void spifi3_device::autodata_w(uint32_t data)
{
	LOGMASKED(LOG_REGISTER, "write spifi_reg.autodata = 0x%x\n", data);
	spifi_reg.autodata = data;

	if (spifi_reg.autodata & ADATA_EN)
	{
		LOGMASKED(LOG_AUTO, "autodata enabled! target %d direction %s\n", spifi_reg.autodata & ADATA_TARGET_ID, spifi_reg.autodata & ADATA_IN ? "in" : "out");
	}
}

uint32_t spifi3_device::identify_r()
{
	LOGMASKED(LOG_REGISTER, "read spifi_reg.identify = 0x%x\n", spifi_reg.identify);
	return spifi_reg.identify;
}

void spifi3_device::identify_w(uint32_t data)
{
	LOGMASKED(LOG_REGISTER, "write spifi_reg.identify = 0x%x\n", data);
	spifi_reg.identify = data;
}

uint32_t spifi3_device::scsi_status_r()
{
	LOGMASKED(LOG_REGISTER, "read spifi_reg.scsi_status = 0x%x\n", spifi_reg.scsi_status);
	return spifi_reg.scsi_status;
}

void spifi3_device::scsi_status_w(uint32_t data)
{
	LOGMASKED(LOG_REGISTER, "write spifi_reg.scsi_status = 0x%x\n", data);
	spifi_reg.scsi_status = data;
}

uint32_t spifi3_device::icond_r()
{
	LOGMASKED(LOG_REGISTER, "read spifi_reg.icond = 0x%x\n", spifi_reg.icond);
	return spifi_reg.icond;
}

void spifi3_device::icond_w(uint32_t data)
{
	LOGMASKED(LOG_REGISTER, "write spifi_reg.icond = 0x%x\n", data);
	spifi_reg.icond = data;
}

uint32_t spifi3_device::fastwide_r()
{
	LOGMASKED(LOG_REGISTER, "read spifi_reg.fastwide = 0x%x\n", spifi_reg.fastwide);
	return spifi_reg.fastwide;
}

void spifi3_device::fastwide_w(uint32_t data)
{
	LOGMASKED(LOG_REGISTER, "write spifi_reg.fastwide = 0x%x\n", data);
	spifi_reg.fastwide = data;
}

uint32_t spifi3_device::exctrl_r()
{
	LOGMASKED(LOG_REGISTER, "read spifi_reg.exctrl = 0x%x\n", spifi_reg.exctrl);
	return spifi_reg.exctrl;
}

void spifi3_device::exctrl_w(uint32_t data)
{
	LOGMASKED(LOG_REGISTER, "write spifi_reg.exctrl = 0x%x\n", data);
	spifi_reg.exctrl = data;
}

void spifi3_device::clear_fifo()
{
	m_even_fifo.clear_queue();
	m_odd_fifo.clear_queue();
}

void spifi3_device::prcmd_w(uint32_t data)
{
	LOGMASKED(LOG_REGISTER, "write spifi_reg.prcmd = 0x%x (%s)\n", data, machine().describe_context());

	// TODO: NJMP and other commands
	const auto cmd = data & PRCMD_MASK;
	switch (cmd)
	{
		case PRC_DATAOUT:
		{
			state = INIT_XFR;
			xfr_phase = scsi_bus->ctrl_r() & S_PHASE_MASK;

			const dma_direction luntar_dma_setting = dma_setting(bus_id) == DMA_OUT ? DMA_OUT : DMA_NONE;
			dma_set(luntar_dma_setting);
			LOGMASKED(LOG_CMD, "start command DATAOUT, DMA = %d\n", luntar_dma_setting);
			spifi_reg.spstat = SPS_DATAOUT;
			break;
		}
		case PRC_DATAIN:
		{
			state = INIT_XFR;
			xfr_phase = scsi_bus->ctrl_r() & S_PHASE_MASK;

			const dma_direction luntar_dma_setting = dma_setting(bus_id) == DMA_IN ? DMA_IN : DMA_NONE;
			dma_set(luntar_dma_setting);
			LOGMASKED(LOG_CMD, "start command DATAIN, DMA = %d\n", luntar_dma_setting);
			spifi_reg.spstat = SPS_DATAIN;
			break;
		}
		case PRC_MSGOUT:
		case PRC_MSGIN:
		case PRC_COMMAND:
		case PRC_STATUS:
		{
			LOGMASKED(LOG_CMD, "start command %s\n", prcmd_command_names[cmd]);
			state = INIT_XFR;
			xfr_phase = scsi_bus->ctrl_r() & S_PHASE_MASK;

			command_pos = 0;
			dma_set(DMA_NONE);
			spifi_reg.spstat = prcmd_to_spstat(cmd);
			break;
		}
		case PRC_TRPAD:
		{
			LOGMASKED(LOG_CMD, "start command TRPAD\n");
			xfr_phase = scsi_bus->ctrl_r() & S_PHASE_MASK;
			if (xfr_phase & S_INP)
			{
				state = INIT_XFR_RECV_PAD_WAIT_REQ;
			}
			else
			{
				state = INIT_XFR_SEND_PAD_WAIT_REQ;
			}
			scsi_bus->ctrl_w(scsi_refid, 0, S_ACK);
			break;
		}
		default:
		{
			LOG("Unimplemented command %d!\n", data);
			return;
		}
	}

	if (data != PRC_TRPAD)
	{
		check_drq();
	}

	step(false);
}

uint8_t spifi3_device::cmd_buf_r(offs_t offset)
{
	// 8 slots in the buffer, 16 bytes each
	// so, divide the offset by 16 (truncated) to get the cmd entry
	const int cmd_entry = offset / 16;
	uint8_t result = 0;
	const int register_offset = offset % 16;
	if (register_offset < 12)
	{
		result = spifi_reg.cmbuf[cmd_entry].cdb[register_offset];
	}
	else if (register_offset == 12)
	{
		result = spifi_reg.cmbuf[cmd_entry].quecode;
	}
	else if (register_offset == 13)
	{
		result = spifi_reg.cmbuf[cmd_entry].quetag;
	}
	else if (register_offset == 14)
	{
		result = spifi_reg.cmbuf[cmd_entry].idmsg;
	}
	else if (register_offset == 15)
	{
		result = spifi_reg.cmbuf[cmd_entry].status;
	}

	LOGMASKED(LOG_CMD, "cmd_buf_r(0x%x) -> 0x%x\n", offset, result);

	return result;
}

void spifi3_device::cmd_buf_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_CMD, "cmd_buf_w(0x%x, 0x%x)\n", offset, data);

	// 8 slots in the buffer, 16 bytes each
	// so, divide the offset by 16 (truncated) to get the cmd entry
	const int cmd_entry = offset / 16;
	const int register_offset = offset % 16;
	if (register_offset < 12)
	{
		spifi_reg.cmbuf[cmd_entry].cdb[register_offset] = data;
	}
	else if (register_offset == 12)
	{
		spifi_reg.cmbuf[cmd_entry].quecode = data;
	}
	else if (register_offset == 13)
	{
		spifi_reg.cmbuf[cmd_entry].quetag = data;
	}
	else if (register_offset == 14)
	{
		spifi_reg.cmbuf[cmd_entry].idmsg = data;
	}
	else if (register_offset == 15)
	{
		spifi_reg.cmbuf[cmd_entry].status = data;
	}
}

TIMER_CALLBACK_MEMBER(spifi3_device::tick)
{
	step(true);
}

void spifi3_device::check_irq()
{
	// There are various ways interrupts can be triggered by the SPIFI - this method is a work in progress.
	// TODO: ICOND, which doesn't seem to be needed much on the "happy path" (no errors)
	const bool irqState = spifi_reg.intr & ~spifi_reg.imask;
	if (irq != irqState)
	{
		LOGMASKED(LOG_INTERRUPT, "Setting IRQ line to %d\n", irqState);
		irq = irqState;
		m_irq_handler(irq);
	}
}

void spifi3_device::check_drq()
{
	bool drq_state = drq;

	switch (dma_dir)
	{
		case DMA_NONE:
		{
			drq_state = false;
			break;
		}

		case DMA_IN: // device to memory
		{
			drq_state = !transfer_count_zero() && !m_even_fifo.empty();
			break;
		}

		case DMA_OUT: // memory to device
		{
			drq_state = !transfer_count_zero() && !m_even_fifo.full();
			break;
		}
	}

	if (drq_state != drq)
	{
		LOGMASKED(LOG_DATA, "DRQ changed to %d!\n", drq_state);
		drq = drq_state;
		m_drq_handler(drq);
	}
}

bool spifi3_device::transfer_count_zero()
{
	return spifi_reg.icond & ICOND_CNTZERO;
}

void spifi3_device::reset_disconnect()
{
	scsi_bus->ctrl_w(scsi_refid, 0, ~S_RST);

	command_pos = 0;
	mode = MODE_D;
}

void spifi3_device::send_byte(scsi_data_target data_source)
{
	state = (state & STATE_MASK) | (SEND_WAIT_SETTLE << SUB_SHIFT);

	if (data_source == COMMAND_BUFFER)
	{
		// Send next data from cmbuf
		if (command_pos > 11)
		{
			fatalerror("%s: Tried to send command past the end of cdb! Command_pos: %d", tag(), command_pos);
		}
		LOGMASKED(LOG_CMD, "Sending byte from cmbuf[%d].cdb[%d] = 0x%x\n", scsi_id, command_pos, spifi_reg.cmbuf[scsi_id].cdb[command_pos]);
		scsi_bus->data_w(scsi_refid, spifi_reg.cmbuf[scsi_id].cdb[command_pos++]);
	}
	else if (data_source == FIFO && (state & STATE_MASK) != INIT_XFR_SEND_PAD)
	{
		// Send next data from FIFO
		scsi_bus->data_w(scsi_refid, m_even_fifo.pop());
		check_drq();
	}
	else
	{
		scsi_bus->data_w(scsi_refid, 0);
	}

	scsi_bus->ctrl_w(scsi_refid, S_ACK, S_ACK);
	scsi_bus->ctrl_wait(scsi_refid, S_REQ, S_REQ);
	delay_cycles(sync_period);
}

void spifi3_device::recv_byte()
{
	// Wait for valid input
	scsi_bus->ctrl_wait(scsi_refid, S_REQ, S_REQ);
	state = (state & STATE_MASK) | (RECV_WAIT_REQ_1 << SUB_SHIFT);
	step(false);
}

void spifi3_device::function_bus_complete()
{
	LOG("function_bus_complete\n");
	state = IDLE;
	spifi_reg.spstat = SPS_IDLE;

	// TODO: Any ICOND changes needed here?
	spifi_reg.intr |= INTR_FCOMP | INTR_BSRQ;
	dma_set(DMA_NONE);
	check_drq();
	check_irq();
}

void spifi3_device::function_complete()
{
	LOG("function_complete\n");
	state = IDLE;
	spifi_reg.spstat = SPS_IDLE;

	// TODO: Any ICOND changes needed here?
	spifi_reg.intr |= INTR_FCOMP;
	dma_set(DMA_NONE);
	check_drq();
	check_irq();
}

void spifi3_device::bus_complete()
{
	LOG("bus_complete\n");
	state = IDLE;

	spifi_reg.intr |= INTR_BSRQ;
	dma_set(DMA_NONE);
	check_drq();
	check_irq();
}

void spifi3_device::dma_set(dma_direction dir)
{
	dma_dir = dir;

	// account for data already in the fifo
	if (dir == DMA_OUT && !m_even_fifo.empty())
	{
		decrement_tcounter(m_even_fifo.get_size());
	}
}

void spifi3_device::decrement_tcounter(uint32_t count)
{
	if (!dma_command(dma_dir))
	{
		return;
	}
	else if (count > tcounter)
	{
		fatalerror("%s: tcounter ran out of bytes!", tag());
	}

	tcounter -= count;
	if (tcounter == 0)
	{
		// TODO: does this immediately trigger an interrupt? or is this just a status thing?
		spifi_reg.icond |= ICOND_CNTZERO;
	}
}

void spifi3_device::delay(uint32_t cycles)
{
	if (!clock_conv)
	{
		return;
	}
	cycles *= clock_conv;
	tm->adjust(clocks_to_attotime(cycles));
}

void spifi3_device::delay_cycles(uint32_t cycles)
{
	tm->adjust(clocks_to_attotime(cycles));
}

void spifi3_device::arbitrate()
{
	state = (state & STATE_MASK) | (ARB_COMPLETE << SUB_SHIFT);
	scsi_bus->data_w(scsi_refid, 1 << scsi_id);
	scsi_bus->ctrl_w(scsi_refid, S_BSY, S_BSY);
	delay(11);
}

void spifi3_device::dma_w(uint8_t val)
{
	m_even_fifo.push(val);
	decrement_tcounter();
	check_drq();
	step(false);
}

uint8_t spifi3_device::dma_r()
{
	LOGMASKED(LOG_DATA, "dma_r called! Fifo count = %d, state = %d.%d, tcounter = %d\n", m_even_fifo.get_size(), state & STATE_MASK, (state & SUB_MASK) >> SUB_SHIFT, tcounter);
	uint8_t val = m_even_fifo.pop();
	decrement_tcounter();
	check_drq();
	step(false);
	return val;
}

void spifi3_device::scsi_ctrl_changed()
{
	uint32_t ctrl = scsi_bus->ctrl_r();
	if (ctrl & S_RST)
	{
		LOG("scsi bus reset\n");
		return;
	}

	step(false);
}

spifi3_device::dma_direction spifi3_device::dma_setting(uint32_t target_id)
{
	// TODO: LUN? That is also written to this register in NetBSD
	//       (and probably NEWS-OS). Need to figure out how to get
	//       a device with multiple LUNs to test to nail down this
	//       logic.
	dma_direction result = DMA_NONE;
	if ((spifi_reg.autodata & ADATA_TARGET_ID) == target_id)
	{
		result = (spifi_reg.autodata & ADATA_IN) ? DMA_IN :
			DMA_OUT;
	}
	return result;
}

uint32_t spifi3_device::get_target_id()
{
	return (spifi_reg.select & SEL_TARGET) >> 4;
}

bool spifi3_device::autodata_active(uint32_t target_id)
{
	return (spifi_reg.autodata & ADATA_EN) && ((spifi_reg.autodata & ADATA_TARGET_ID) == target_id);
}

bool spifi3_device::autodata_in(uint32_t target_id)
{
	return autodata_active(target_id) && (spifi_reg.autodata & ADATA_IN);
}

bool spifi3_device::autodata_out(uint32_t target_id)
{
	return autodata_active(target_id) && !(spifi_reg.autodata & ADATA_IN);
}

void spifi3_device::autostat_done(uint32_t target_id)
{
	spifi_reg.autostat &= ~(1 << target_id);
}

bool spifi3_device::autostat_active(uint32_t target_id)
{
	return spifi_reg.autostat & (1 << target_id);
}

bool spifi3_device::automsg_active()
{
	return spifi_reg.cmlen & CML_AMSG_EN;
}

bool spifi3_device::autocmd_active()
{
	return spifi_reg.cmlen & CML_ACOM_EN;
}

void spifi3_device::start_autostat()
{
	LOGMASKED(LOG_AUTO, "start AUTOSTAT\n");
	state = INIT_XFR;
	xfr_phase = S_PHASE_STATUS;
	spifi_reg.spstat = SPS_STATUS;
	dma_set(DMA_NONE);
}

void spifi3_device::start_automsg(uint32_t msg_phase)
{
	LOGMASKED(LOG_AUTO, "start AUTOMSG\n");
	state = INIT_XFR;
	xfr_phase = msg_phase;
	spifi_reg.spstat = msg_phase == S_PHASE_MSG_IN ? SPS_MSGIN : SPS_MSGOUT;
	dma_set(DMA_NONE);
}

void spifi3_device::start_autocmd()
{
	LOGMASKED(LOG_AUTO, "start AUTOCMD\n");
	scsi_bus->ctrl_w(scsi_refid, 0, S_ACK); // Deassert ACK since we are automatically moving to the command phase
	state = INIT_XFR;
	xfr_phase = S_PHASE_COMMAND;
	spifi_reg.spstat = SPS_COMMAND;
	dma_set(DMA_NONE);
}

void spifi3_device::start_autodata(uint32_t data_phase)
{
	LOGMASKED(LOG_AUTO, "start AUTODATA\n");
	state = INIT_XFR;
	xfr_phase = data_phase;
	dma_set(data_phase == S_PHASE_DATA_IN ? DMA_IN : DMA_OUT);
	spifi_reg.spstat = data_phase == S_PHASE_DATA_IN ? SPS_DATAIN : SPS_DATAOUT;
	check_drq();
}

/*
 * auto_phase_transfer
 *
 * This will override the current xfr_phase if needed, otherwise will proceed to BSRQ
 * This is called when a new xfr_phase is detected and there is the possibility of an
 * auto-progression to a new phase
 */
void spifi3_device::auto_phase_transfer(uint32_t new_phase)
{
	if (xfr_phase == new_phase)
	{
		fatalerror("%s: auto_phase_transfer called without phase transition!", tag());
	}
	LOGMASKED(LOG_STATE, "Phase changed to %d\n", new_phase);
	state = INIT_XFR_BUS_COMPLETE;
	command_pos = 0;

	if ((new_phase == S_PHASE_DATA_IN && autodata_in(bus_id)) || (new_phase == S_PHASE_DATA_OUT && autodata_out(bus_id)))
	{
		start_autodata(new_phase);
	}
	else if (new_phase == S_PHASE_STATUS && autostat_active(bus_id))
	{
		start_autostat();
	}
	else if ((new_phase == S_PHASE_MSG_IN || new_phase == S_PHASE_MSG_OUT) && automsg_active())
	{
		start_automsg(new_phase);
	}
	else if (new_phase == S_PHASE_COMMAND && autocmd_active())
	{
		start_autocmd();
	}
}

void spifi3_device::step(bool timeout)
{
	uint32_t ctrl = scsi_bus->ctrl_r();
	uint32_t data = scsi_bus->data_r();

	LOGMASKED(LOG_STATE, "state=%d.%d %s\n", state & STATE_MASK, (state & SUB_MASK) >> SUB_SHIFT, timeout ? "timeout" : "change");

	if (mode == MODE_I && !(ctrl & S_BSY)) // Not busy and we are the initiator. We can disconnect.
	{
		// TODO: Set Z state flag? Any interrupts needed?
		state = IDLE;
		spifi_reg.spstat = SPS_IDLE;
		reset_disconnect();
		check_irq();
	}

	switch (state & SUB_MASK ? state & SUB_MASK : state & STATE_MASK)
	{
		case IDLE:
		{
			break;
		}

		case BUSRESET_WAIT_INT: // Bus was reset by a command, go to idle state and clear reset signal
		{
			state = IDLE;
			scsi_bus->ctrl_w(scsi_refid, 0, S_RST);
			reset_disconnect();
			break;
		}

		case ARB_COMPLETE << SUB_SHIFT: // Arbitration process done, check results and assert SEL if we won
		{
			if (!timeout) // Synchronize state to clock
			{
				break;
			}

			// Scan to see if we won arbitration
			int arbitrationWinner;
			for (arbitrationWinner = 7; arbitrationWinner >= 0 && !(data & (1 << arbitrationWinner)); arbitrationWinner--)
			{
			}

			if (arbitrationWinner != scsi_id)
			{
				scsi_bus->data_w(scsi_refid, 0);
				scsi_bus->ctrl_w(scsi_refid, 0, S_ALL);
				fatalerror("%s: need to wait for bus free (lost arbitration)\n", tag());
			}

			// Now that we won arbitration, we need to assert SEL and wait for the bus to settle.
			state = (state & STATE_MASK) | (ARB_ASSERT_SEL << SUB_SHIFT);
			scsi_bus->ctrl_w(scsi_refid, S_SEL, S_SEL);
			delay(6);
			break;
		}

		case ARB_ASSERT_SEL << SUB_SHIFT: // Won arbitration and asserted SEL, time to write target to data bus
		{
			if (!timeout) // Synchronize state to clock
			{
				break;
			}

			bus_id = get_target_id();
			scsi_bus->data_w(scsi_refid, (1 << scsi_id) | (1 << bus_id));
			state = (state & STATE_MASK) | (ARB_SET_DEST << SUB_SHIFT);
			delay_cycles(4);
			break;
		}

		case ARB_SET_DEST << SUB_SHIFT: // Set target, time to release BSY
		{
			if (!timeout) // Synchronize state to clock
			{
				break;
			}

			state = (state & STATE_MASK) | (ARB_RELEASE_BUSY << SUB_SHIFT);
			scsi_bus->ctrl_w(scsi_refid, spifi_reg.select & SEL_WATN ? S_ATN : 0, S_ATN | S_BSY);
			delay(2);
			break;
		}

		case ARB_RELEASE_BUSY << SUB_SHIFT: // BSY released, if target responds, we need to do the deskew wait
		{
			if (!timeout) // Synchronize state to clock
			{
				break;
			}

			if (ctrl & S_BSY) // Check if target responded
			{
				state = (state & STATE_MASK) | (ARB_DESKEW_WAIT << SUB_SHIFT);
				// TODO: reselection logic for this step
				delay_cycles(2);
			}
			else // If not, we ran out of time - wait until the next timeout and check again
			{
				state = (state & STATE_MASK) | (ARB_TIMEOUT_BUSY << SUB_SHIFT);

				// TODO: What is the correct delay time for the SPIFI?
				delay(1);
			}
			break;
		}

		case ARB_DESKEW_WAIT << SUB_SHIFT: // Waited for deskew, now we can proceed to the next state.
		{
			if (!timeout)
			{
				break;
			}

			scsi_bus->data_w(scsi_refid, 0);
			scsi_bus->ctrl_w(scsi_refid, 0, S_SEL); // Clear SEL - target may now assert REQ

			// TODO: reselection logic for this step
			// Target mode not supported for now
			if (false)
			{
				LOG("mode switch to Target\n");
				mode = MODE_T;
			}
			else
			{
				LOG("mode switch to Initiator\n");
				mode = MODE_I;
			}

			state &= STATE_MASK;
			step(true);
			break;
		}

		case ARB_TIMEOUT_BUSY << SUB_SHIFT: // Timed out during selection, try again
		{
			if (timeout) // No response from target
			{
				scsi_bus->data_w(scsi_refid, 0);
				LOG("select timeout\n");
				state = (state & STATE_MASK) | (ARB_TIMEOUT_ABORT << SUB_SHIFT); // handle timeout
				delay(1000);
			}
			else if (ctrl & S_BSY) // Got response from target, wait before allowing transaction
			{
				state = (state & STATE_MASK) | (ARB_DESKEW_WAIT << SUB_SHIFT);
				// TODO: reselection logic for this step
				delay_cycles(2);
			}
			break;
		}

		case ARB_TIMEOUT_ABORT << SUB_SHIFT: // Selection timed out - need to abort
		{
			if (!timeout)
			{
				break;
			}

			if (ctrl & S_BSY) // Last chance for target to respond
			{
				state = (state & STATE_MASK) | (ARB_DESKEW_WAIT << SUB_SHIFT);
				// TODO: reselection logic for this step
				delay_cycles(2);
			}
			else // If not, force bus free
			{
				scsi_bus->ctrl_w(scsi_refid, 0, S_ALL);
				state = IDLE;
				spifi_reg.intr = INTR_TIMEO;
				reset_disconnect();
				check_irq();
			}
			break;
		}

		case SEND_WAIT_SETTLE << SUB_SHIFT:
		{
			if (!timeout)
			{
				break;
			}

			state = (state & STATE_MASK) | (SEND_WAIT_REQ_0 << SUB_SHIFT);
			step(false);
			break;
		}

		case SEND_WAIT_REQ_0 << SUB_SHIFT:
		{
			if (ctrl & S_REQ)
			{
				break;
			}

			state = state & STATE_MASK;
			scsi_bus->data_w(scsi_refid, 0);
			scsi_bus->ctrl_w(scsi_refid, 0, S_ACK);
			step(false);
			break;
		}

		case RECV_WAIT_REQ_1 << SUB_SHIFT:
		{
			if (!(ctrl & S_REQ))
				break;

			state = (state & STATE_MASK) | (RECV_WAIT_SETTLE << SUB_SHIFT);
			delay_cycles(sync_period);
			break;
		}

		case RECV_WAIT_SETTLE << SUB_SHIFT:
		{
			if (!timeout)
			{
				break;
			}

			const auto masked_state = state & STATE_MASK;
			if (masked_state != INIT_XFR_RECV_PAD)
			{
				const auto data = scsi_bus->data_r();
				const auto xfr_masked = xfr_phase & S_PHASE_MASK;
				if (xfr_masked == S_PHASE_STATUS && masked_state == INIT_XFR_RECV_BYTE_ACK && autostat_active(bus_id))
				{
					LOGMASKED(LOG_AUTO, "AUTOSTAT setting cmbuf[%d].status = 0x%x\n", data, bus_id);
					spifi_reg.cmbuf[bus_id].status = data;
					autostat_done(bus_id);
				}
				else if (xfr_masked == S_PHASE_MSG_IN && (masked_state == INIT_XFR_RECV_BYTE_ACK_AUTOMSG || masked_state == INIT_XFR_RECV_BYTE_ACK) && automsg_active())
				{
					// TODO: determine where AUTOMSG byte goes - probably in the matching ID's cdb
					LOGMASKED(LOG_AUTO, "AUTOMSG accepted byte 0x%x\n", data);
				}
				else
				{
					m_even_fifo.push(data);
				}
				check_drq();
			}
			scsi_bus->ctrl_w(scsi_refid, S_ACK, S_ACK);
			state = masked_state | (RECV_WAIT_REQ_0 << SUB_SHIFT);
			step(false);
			break;
		}

		case RECV_WAIT_REQ_0 << SUB_SHIFT:
		{
			if (ctrl & S_REQ)
			{
				break;
			}

			state = state & STATE_MASK;
			step(false);
			break;
		}

		case DISC_SEL_ARBITRATION_INIT: // Arbitration and selection complete, time to execute the queued command
		{
			if (automsg_active())
			{
				state = DISC_SEL_ARBITRATION;
				step(false);
			}
			else
			{
				// TODO: It isn't clear what the correct behavior here should be.
				// The NWS-5000 APmonitor, NEWS-OS, and NetBSD all set AUTOMSG, so this code path is never taken.
				// For now, kick it up to the firmware or software to handle, hang, or panic.
				bus_complete();
			}
			break;
		}

		case DISC_SEL_ARBITRATION:
		{
			if (!(spifi_reg.select & SEL_WATN))
			{
				// The NWS-5000 APmonitor, NEWS-OS, and NetBSD all set SEL_WATN, so this code path is never taken.
				fatalerror("%s: SEL_WATN was not asserted - this is not yet implemented!", tag());
				state = DISC_SEL_WAIT_REQ;
			}
			else
			{
				state = DISC_SEL_ATN_WAIT_REQ;
			}

			scsi_bus->ctrl_wait(scsi_refid, S_REQ, S_REQ); // wait for REQ
			if (ctrl & S_REQ)
			{
				step(false);
			}
			break;
		}

		case DISC_SEL_ATN_WAIT_REQ: // REQ asserted, either get read to send a byte, or complete the command.
		{
			if (!(ctrl & S_REQ))
			{
				break;
			}

			// If we're no longer in MSG_OUT, we're done
			if ((ctrl & S_PHASE_MASK) != S_PHASE_MSG_OUT)
			{
				function_complete();
				break;
			}

			// Deassert ATN now if we asserted it before
			if (spifi_reg.select & SEL_WATN)
			{
				scsi_bus->ctrl_w(scsi_refid, 0, S_ATN);
			}

			state = DISC_SEL_ATN_SEND_BYTE;
			if (spifi_reg.identify & 0x80)
			{
				// Identify register has an identify packet - send it.
				scsi_bus->data_w(scsi_refid, spifi_reg.identify);
				scsi_bus->ctrl_w(scsi_refid, S_ACK, S_ACK);
				scsi_bus->ctrl_wait(scsi_refid, S_REQ, S_REQ);
			}
			else
			{
				// Send the next byte from the CDB
				send_byte(COMMAND_BUFFER);
			}
			break;
		}

		case DISC_SEL_ATN_SEND_BYTE:
		{
			if ((spifi_reg.identify & 0x80) || command_pos >= (spifi_reg.cmlen & CML_LENMASK))
			{
				// autoidentified target, now we need to see if autocmd is enabled. If so, we can just proceed to the XFR phase automatically.
				spifi_reg.identify = 0x0; // in case we just sent an ID packet
				command_pos = 0;
				const auto new_phase = (ctrl & S_PHASE_MASK);
				auto_phase_transfer(new_phase);
				if (state == INIT_XFR_BUS_COMPLETE) // auto_phase_transfer fell through
				{
					function_bus_complete();
				}
				else
				{
					step(false);
				}
			}
			else
			{
				state = DISC_SEL_WAIT_REQ;
			}
			break;
		}

		case DISC_SEL_WAIT_REQ:
		{
			if (!(ctrl & S_REQ))
			{
				break;
			}
			if ((ctrl & S_PHASE_MASK) != S_PHASE_COMMAND)
			{
				scsi_bus->ctrl_wait(scsi_refid, 0, S_REQ);
				function_bus_complete();
				break;
			}

			state = DISC_SEL_SEND_BYTE;
			send_byte(COMMAND_BUFFER);
			break;
		}

		case DISC_SEL_SEND_BYTE:
		{
			state = DISC_SEL_WAIT_REQ;
			break;
		}

		case INIT_CPT_RECV_BYTE_ACK:
		{
			state = INIT_CPT_RECV_WAIT_REQ;
			scsi_bus->ctrl_w(scsi_refid, 0, S_ACK);
			break;
		}

		case INIT_CPT_RECV_WAIT_REQ:
		{
			if (!(ctrl & S_REQ))
			{
				break;
			}

			if ((ctrl & S_PHASE_MASK) != S_PHASE_MSG_IN)
			{
				command_pos = 0;
				bus_complete();
			}
			else
			{
				state = INIT_CPT_RECV_BYTE_NACK;
				recv_byte();
			}
			break;
		}

		case INIT_CPT_RECV_BYTE_NACK:
		{
			function_complete();
			break;
		}

		case INIT_MSG_WAIT_REQ:
		{
			if ((ctrl & (S_REQ | S_BSY)) == S_BSY)
			{
				break;
			}
			bus_complete();
			break;
		}

		case INIT_XFR:
		{
			LOGMASKED(LOG_STATE, "INIT_XFR: %d\n", xfr_phase);
			switch (xfr_phase)
			{
				case S_PHASE_DATA_OUT:
				case S_PHASE_COMMAND:
				case S_PHASE_MSG_OUT:
				{
					state = INIT_XFR_SEND_BYTE;

					// can't send if the fifo is empty and we are sending data
					if (m_even_fifo.empty() && xfr_phase == S_PHASE_DATA_OUT)
					{
						xfr_data_source = FIFO;
						break;
					}

					// if it's the last message byte, ensure ATN is low before sending
					if ((xfr_phase == S_PHASE_MSG_OUT) && (command_pos == (spifi_reg.cmlen & CML_LENMASK) - 1))
					{
						scsi_bus->ctrl_w(scsi_refid, 0, S_ATN);
					}

					if (xfr_phase == S_PHASE_DATA_OUT)
					{
						xfr_data_source = FIFO;
						send_byte(FIFO);
					}
					else
					{
						// Both commands and messages come from the CDB
						xfr_data_source = COMMAND_BUFFER;
						send_byte(COMMAND_BUFFER);
					}
					break;
				}

				case S_PHASE_DATA_IN:
				case S_PHASE_STATUS:
				case S_PHASE_MSG_IN:
				{
					// can't receive if the fifo is full
					if (m_even_fifo.full() && !(xfr_phase == S_PHASE_STATUS && autostat_active(bus_id) && !(xfr_phase == S_PHASE_MSG_IN && automsg_active())))
					{
						break;
					}

					// if it's the last message byte, ACK remains asserted.
					// However, if AUTOMSG is enabled, automatically accept the message by lowering ACK before continuing.
					if ((xfr_phase == S_PHASE_MSG_IN && (!dma_command(dma_dir) || tcounter == 1)))
					{
						state = automsg_active() ? INIT_XFR_RECV_BYTE_ACK_AUTOMSG : INIT_XFR_RECV_BYTE_NACK;
					}
					else
					{
						state = INIT_XFR_RECV_BYTE_ACK;
					}

					xfr_data_source = FIFO;
					recv_byte();
					break;
				}

				default:
				{
					LOG("xfer on phase %d\n", scsi_bus->ctrl_r() & S_PHASE_MASK);
					function_complete();
					break;
				}
			}
			break;
		}

		case INIT_XFR_WAIT_REQ:
		{
			if (!(ctrl & S_REQ))
			{
				break;
			}

			// check for command complete
			const auto new_phase = (ctrl & S_PHASE_MASK);
			if (xfr_data_source == FIFO && (dma_command(dma_dir) && transfer_count_zero() && (dma_dir == DMA_IN || m_even_fifo.empty())))
			{
				LOGMASKED(LOG_DATA, "DMA transfer complete\n");
				if (xfr_phase == S_PHASE_DATA_OUT && new_phase == S_PHASE_DATA_OUT)
				{
					// Set ICOND so that NEWS-OS knows that SPIFI is ready to send pad bytes.
					// NEWS-OS will sometimes set tcounter to less than one block size, then sends TR_PAD in response to this ICOND value.
					spifi_reg.icond = ICOND_UXPHASEZ;
					state = INIT_XFR_BUS_COMPLETE;
				}
				else if (xfr_phase == S_PHASE_DATA_IN && new_phase == S_PHASE_DATA_IN)
				{
					// Dump the remaining contents of the FIFO - at this point, the transfer counter is exhausted so whatever
					// is left in the queue is pad byte junk read after the real data was received but before the DMA transfer completed
					m_even_fifo.clear_queue();

					// See above
					spifi_reg.icond = ICOND_UXPHASEZ;
					state = INIT_XFR_BUS_COMPLETE;
				}
				else
				{
					auto_phase_transfer(new_phase);
				}
			}
			else if (xfr_data_source == FIFO && (!dma_command(dma_dir) && (xfr_phase & S_INP) == 0 && m_even_fifo.empty()))
			{
				LOGMASKED(LOG_DATA, "Non-DMA transfer out complete\n");
				auto_phase_transfer(new_phase);
			}
			else if (xfr_data_source == FIFO && (!dma_command(dma_dir) && ((xfr_phase & S_INP) == S_INP) && m_even_fifo.get_size() == 1))
			{
				LOGMASKED(LOG_DATA, "Non-DMA transfer in complete\n");
				auto_phase_transfer(new_phase);
			}
			else if (xfr_data_source == COMMAND_BUFFER && (command_pos >= (spifi_reg.cmlen & CML_LENMASK))) // Done transferring message or command
			{
				if (new_phase != xfr_phase)
				{
					auto_phase_transfer(new_phase);
				}
				else
				{
					fatalerror("%s: ran out of CDB bytes to transfer!", tag());
				}
			}
			else
			{
				const auto new_phase = ctrl & S_PHASE_MASK;
				if (new_phase != xfr_phase)
				{
					auto_phase_transfer(new_phase);
				}
				else
				{
					state = INIT_XFR;
				}
			}
			step(false);
			break;
		}

		case INIT_XFR_SEND_BYTE:
		{
			state = INIT_XFR_WAIT_REQ;
			step(false);
			break;
		}

		case INIT_XFR_RECV_BYTE_ACK:
		{
			state = INIT_XFR_WAIT_REQ;
			scsi_bus->ctrl_w(scsi_refid, 0, S_ACK);
			step(false);
			break;
		}

		case INIT_XFR_RECV_BYTE_NACK:
		{
			state = INIT_XFR_FUNCTION_COMPLETE;
			step(false);
			break;
		}

		case INIT_XFR_RECV_BYTE_ACK_AUTOMSG:
		{
			// Bypass the rest of the state machine, because if we allow this to do another cycle,
			// the bus will be free and the interrupts won't be set correctly.
			// This would have gone to INIT_XFR_FUNCTION_COMPLETE otherwise.
			if (dma_command(dma_dir) && !transfer_count_zero() && !m_even_fifo.empty())
			{
				break;
			}
			LOGMASKED(LOG_AUTO, "AUTOMSG cleared ACK\n");
			scsi_bus->ctrl_w(scsi_refid, 0, S_ACK);
			function_complete();

			// Since we auto-accepted the message, we step again here to complete the disconnect
			step(false);
			break;
		}

		case INIT_XFR_FUNCTION_COMPLETE:
		{
			// wait for dma transfer to complete or fifo to drain
			if (dma_command(dma_dir) && !transfer_count_zero() && !m_even_fifo.empty())
			{
				break;
			}

			function_complete();
			break;
		}

		case INIT_XFR_BUS_COMPLETE:
		{
			// wait for dma transfer to complete or fifo to drain
			if (dma_command(dma_dir) && !transfer_count_zero() && !m_even_fifo.empty())
			{
				break;
			}

			bus_complete();
			break;
		}

		case INIT_XFR_SEND_PAD_WAIT_REQ:
		{
			if (!(ctrl & S_REQ))
			{
				break;
			}

			const auto new_phase = (ctrl & S_PHASE_MASK);
			if (new_phase != xfr_phase)
			{
				auto_phase_transfer(new_phase);
				step(false);
			}
			else
			{
				state = INIT_XFR_SEND_PAD;
				send_byte(FIFO);
			}
			break;
		}

		case INIT_XFR_SEND_PAD:
		{
			state = INIT_XFR_SEND_PAD_WAIT_REQ;
			step(false);
			break;
		}

		case INIT_XFR_RECV_PAD_WAIT_REQ:
		{
			if (!(ctrl & S_REQ))
			{
				break;
			}

			const auto new_phase = (ctrl & S_PHASE_MASK);
			if (new_phase != xfr_phase)
			{
				auto_phase_transfer(new_phase);
				step(false);
			}
			else
			{
				state = INIT_XFR_RECV_PAD;
				recv_byte();
			}
			break;
		}

		case INIT_XFR_RECV_PAD:
		{
			state = INIT_XFR_RECV_PAD_WAIT_REQ;
			scsi_bus->ctrl_w(scsi_refid, 0, S_ACK);
			step(false);
			break;
		}

		default:
		{
			fatalerror("%s: step() unexpected state %d.%d\n", tag(), state & STATE_MASK, (state & SUB_MASK) >> SUB_SHIFT);
		}
	}
}
