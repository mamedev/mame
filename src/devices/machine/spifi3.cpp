// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay, Brice Onken

/*
 * HP 1TV3-0302 SPIFI3-SE SCSI controller
 * Datasheets for this seem to be impossible to find - as such, this implementation is tightly coupled
 * with the Sony DMAC3 and Sony NEWS platform because the only avaliable implementation to reference that I have
 * found is the APBus NetBSD one. Hopefully a datasheet will turn up eventually.
 * - https://github.com/NetBSD/src/blob/trunk/sys/arch/newsmips/apbus/spifireg.h
 * - https://github.com/NetBSD/src/blob/trunk/sys/arch/newsmips/apbus/spifi.c
 *
 * TODO:
 * - Everything
 *
 * MROM SCSI probe example:
<basic init>
[:dmac] dmac0 ictl_w: 0x202 <- Enable DMAC0 interrupt and EOPIE interrupt
[:dmac] dmac0 cnf_w: 0x20 <- SLOWACCESS for SPIFI register rw
[:scsi0:7:spifi3] write spifi_reg.auxctrl = 0x40 <- CRST
[:scsi0:7:spifi3] read spifi_reg.scsi_status = 0x1 <- read scsistatus and check if 0x1 - if so, SPIFI is alive. If this is 0, the SCSI buses, DMACs, and SPIFIs will not enumerate
[:dmac] dmac1 ictl_w: 0x202 < Enable DMAC1 interrupt and EOPIE interrupt
[:dmac] dmac1 cnf_w: 0x20 <- SLOWACCESS for SPIFI register rw
[:scsi1:7:spifi3] write spifi_reg.auxctrl = 0x40 <- CRST
[:scsi1:7:spifi3] read spifi_reg.scsi_status = 0x1 <- scsi status check

<repeat previous section - stability/bugs? NetBSD code mentions a few issues>
[:dmac] dmac0 ictl_w: 0x202
[:dmac] dmac0 cnf_w: 0x20
[:scsi0:7:spifi3] write spifi_reg.auxctrl = 0x40
[:scsi0:7:spifi3] read spifi_reg.scsi_status = 0x1
[:dmac] dmac1 ictl_w: 0x202
[:dmac] dmac1 cnf_w: 0x20
[:scsi1:7:spifi3] write spifi_reg.auxctrl = 0x40
[:scsi1:7:spifi3] read spifi_reg.scsi_status = 0x1

<restore default mode>
[:dmac] dmac0 ictl_w: 0x202
[:dmac] dmac0 cnf_w: 0x1
[:dmac] dmac1 ictl_w: 0x202
[:dmac] dmac1 cnf_w: 0x1
 *
 beginning of example DMAC+SPIFI transaction (running dl from the MROM)
 [:dmac] dmac0 cnf_w: 0x20 <- Set DMAC to SLOWACCESS (SPIFI register mode)
[:scsi0:7:spifi3] write spifi_reg.auxctrl = 0x80 <- SRST (does it automatically clear these bits?)
[:dmac] dmac0 cnf_w: 0x1 <-Set DMAC to FASTACCESS (normal mode)
[:dmac] dmac0 cnf_w: 0x20
[:scsi0:7:spifi3] write spifi_reg.auxctrl = 0x40 <- CRST (does it automatically clear these bits?)
[:dmac] dmac0 cnf_w: 0x1
[:dmac] dmac0 cnf_w: 0x20
[:scsi0:7:spifi3] write spifi_reg.auxctrl = 0x20 <- SETRST (does it automatically clear these bits?) 
[:dmac] dmac0 cnf_w: 0x1
[:dmac] dmac0 cnf_w: 0x20
[:scsi0:7:spifi3] write spifi_reg.auxctrl = 0x0 <- clear AUXCTRL register
[:dmac] dmac0 cnf_w: 0x1
[:dmac] dmac0 cnf_w: 0x20
[:scsi0:7:spifi3] write spifi_reg.auxctrl = 0x80 <- set SRST (does it automatically clear these bits?)
[:dmac] dmac0 cnf_w: 0x1
[:dmac] dmac0 cnf_w: 0x20
[:scsi0:7:spifi3] write spifi_reg.auxctrl = 0x40 <- set CRST (does it automatically clear these bits?)
[:dmac] dmac0 cnf_w: 0x1
[:dmac] dmac0 cnf_w: 0x20
[:scsi0:7:spifi3] write spifi_reg.auxctrl = 0x4 <- set DMAEDGE
[:dmac] dmac0 cnf_w: 0x1
[:dmac] dmac0 cnf_w: 0x20
[:scsi0:7:spifi3] write spifi_reg.imask = 0x22 <- mask target mode interrupts (COMRECV and TGSEL) (NetBSD masks these too)
[:dmac] dmac0 cnf_w: 0x1
[:dmac] dmac0 cnf_w: 0x20
[:scsi0:7:spifi3] write spifi_reg.config = 0xf <- set config to PGENEN [3] (parity generation enable), and IID (initiator SCSI ID) [2-0]. NetBSD also enables PCHKEN (parity checking)
[:dmac] dmac0 cnf_w: 0x1
[:dmac] dmac0 cnf_w: 0x20
[:scsi0:7:spifi3] write spifi_reg.fastwide = 0x0 <- fastwide mode disable (netbsd enables this)
[:dmac] dmac0 cnf_w: 0x1
[:dmac] dmac0 cnf_w: 0x20
[:scsi0:7:spifi3] write spifi_reg.prctrl = 0x0 <-no extra processor options (matches netbsd)
[:dmac] dmac0 cnf_w: 0x1
[:dmac] dmac0 cnf_w: 0x20
[:scsi0:7:spifi3] write spifi_reg.loopctrl = 0x0 <- no loopback (matches netbsd)
[:dmac] dmac0 cnf_w: 0x1
[:] LED_DISK: ON
[:dmac] dmac0 cnf_w: 0x20
[:scsi0:7:spifi3] read spifi_reg.spstat = 0x380 <- need to determine what the value of this register should be
[:dmac] dmac0 cnf_w: 0x1
[:dmac] dmac0 ictl_r: 0x202 <- waiting for interrupt (watching bit 0 of ictl)
<hangs here>
 */

#include "emu.h"
#include "spifi3.h"

#define VERBOSE 1
#include "logmacro.h"

DEFINE_DEVICE_TYPE(SPIFI3, spifi3_device, "spifi3", "HP SPIFI3 SCSI 1 Protocol Controller")

/*
static char const *const nscsi_phase[] = { "DATA OUT", "DATA IN", "COMMAND", "STATUS", "*", "*", "MESSAGE OUT", "MESSAGE IN" };

// FIXME: would be better to reuse from nscsi_full_device
static unsigned const SCSI_ARB_DELAY  =  2'400;
static unsigned const SCSI_BUS_CLEAR  =    800;
static unsigned const SCSI_BUS_FREE   =    800;
static unsigned const SCSI_BUS_SETTLE =    400;
static unsigned const SCSI_BUS_SKEW   =     10;
static unsigned const SCSI_RST_HOLD   = 25'000;
*/

// ALLOW_SAVE_TYPE(spifi3_device::state);


spifi3_device::spifi3_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: nscsi_device(mconfig, SPIFI3, tag, owner, clock)
	, nscsi_slot_card_interface(mconfig, *this, DEVICE_SELF)
	, m_irq_out_cb(*this)
	, m_drq_out_cb(*this)
	, m_port_out_cb(*this)
{
}
void spifi3_device::map(address_map &map)
{
	// please forgive me for this
	map(0x00, 0x03).lrw32(NAME([this]() { LOG("read spifi_reg.spstat = 0x%x\n", spifi_reg.spstat); return spifi_reg.spstat; }), NAME([this](uint32_t data) { LOG("write spifi_reg.spstat = 0x%x\n", data); spifi_reg.spstat = data; }));
	map(0x04, 0x07).lrw32(NAME([this]() { LOG("read spifi_reg.cmlen = 0x%x\n", spifi_reg.cmlen); return spifi_reg.cmlen; }), NAME([this](uint32_t data) { LOG("write spifi_reg.cmlen = 0x%x\n", data); spifi_reg.cmlen = data; }));
	map(0x08, 0x0b).lrw32(NAME([this]() { LOG("read spifi_reg.cmdpage = 0x%x\n", spifi_reg.cmdpage); return spifi_reg.cmdpage; }), NAME([this](uint32_t data) { LOG("write spifi_reg.cmdpage = 0x%x\n", data); spifi_reg.cmdpage = data; }));
	map(0x0c, 0x0f).lrw32(NAME([this]() { LOG("read spifi_reg.count_hi = 0x%x\n", spifi_reg.count_hi); return spifi_reg.count_hi; }), NAME([this](uint32_t data) { LOG("write spifi_reg.count_hi = 0x%x\n", data); spifi_reg.count_hi = data; }));
	map(0x10, 0x13).lrw32(NAME([this]() { LOG("read spifi_reg.count_mid = 0x%x\n", spifi_reg.count_mid); return spifi_reg.count_mid; }), NAME([this](uint32_t data) { LOG("write spifi_reg.count_mid = 0x%x\n", data); spifi_reg.count_mid = data; }));
	map(0x14, 0x17).lrw32(NAME([this]() { LOG("read spifi_reg.count_low = 0x%x\n", spifi_reg.count_low); return spifi_reg.count_low; }), NAME([this](uint32_t data) { LOG("write spifi_reg.count_low = 0x%x\n", data); spifi_reg.count_low = data; }));
	map(0x18, 0x1b).lrw32(NAME([this]() { LOG("read spifi_reg.svptr_hi = 0x%x\n", spifi_reg.svptr_hi); return spifi_reg.svptr_hi; }), NAME([this](uint32_t data) { LOG("write spifi_reg.svptr_hi = 0x%x\n", data); spifi_reg.svptr_hi = data; }));
	map(0x1c, 0x1f).lrw32(NAME([this]() { LOG("read spifi_reg.svptr_mid = 0x%x\n", spifi_reg.svptr_mid); return spifi_reg.svptr_mid; }), NAME([this](uint32_t data) { LOG("write spifi_reg.svptr_mid = 0x%x\n", data); spifi_reg.svptr_mid = data; }));
	map(0x20, 0x23).lrw32(NAME([this]() { LOG("read spifi_reg.svptr_low = 0x%x\n", spifi_reg.svptr_low); return spifi_reg.svptr_low; }), NAME([this](uint32_t data) { LOG("write spifi_reg.svptr_low = 0x%x\n", data); spifi_reg.svptr_low = data; }));
	map(0x24, 0x27).lrw32(NAME([this]() { LOG("read spifi_reg.intr = 0x%x\n", spifi_reg.intr); return spifi_reg.intr; }), NAME([this](uint32_t data) { LOG("write spifi_reg.intr = 0x%x\n", data); spifi_reg.intr = data; }));
	map(0x28, 0x2b).lrw32(NAME([this]() { LOG("read spifi_reg.imask = 0x%x\n", spifi_reg.imask); return spifi_reg.imask; }), NAME([this](uint32_t data) { LOG("write spifi_reg.imask = 0x%x\n", data); spifi_reg.imask = data; }));
	map(0x2c, 0x2f).lrw32(NAME([this]() { LOG("read spifi_reg.prctrl = 0x%x\n", spifi_reg.prctrl); return spifi_reg.prctrl; }), NAME([this](uint32_t data) { LOG("write spifi_reg.prctrl = 0x%x\n", data); spifi_reg.prctrl = data; }));
	map(0x30, 0x33).lrw32(NAME([this]() { LOG("read spifi_reg.prstat = 0x%x\n", spifi_reg.prstat); return spifi_reg.prstat; }), NAME([this](uint32_t data) { LOG("write spifi_reg.prstat = 0x%x\n", data); spifi_reg.prstat = data; }));
	map(0x34, 0x37).lrw32(NAME([this]() { LOG("read spifi_reg.init_status = 0x%x\n", spifi_reg.init_status); return spifi_reg.init_status; }), NAME([this](uint32_t data) { LOG("write spifi_reg.init_status = 0x%x\n", data); spifi_reg.init_status = data; }));
	map(0x38, 0x3b).lrw32(NAME([this]() { LOG("read spifi_reg.fifoctrl = 0x%x\n", spifi_reg.fifoctrl); return spifi_reg.fifoctrl; }), NAME([this](uint32_t data) { LOG("write spifi_reg.fifoctrl = 0x%x\n", data); spifi_reg.fifoctrl = data; }));
	map(0x3c, 0x3f).lrw32(NAME([this]() { LOG("read spifi_reg.fifodata = 0x%x\n", spifi_reg.fifodata); return spifi_reg.fifodata; }), NAME([this](uint32_t data) { LOG("write spifi_reg.fifodata = 0x%x\n", data); spifi_reg.fifodata = data; }));
	map(0x40, 0x43).lrw32(NAME([this]() { LOG("read spifi_reg.config = 0x%x\n", spifi_reg.config); return spifi_reg.config; }), NAME([this](uint32_t data) { LOG("write spifi_reg.config = 0x%x\n", data); spifi_reg.config = data; }));
	map(0x44, 0x47).lrw32(NAME([this]() { LOG("read spifi_reg.data_xfer = 0x%x\n", spifi_reg.data_xfer); return spifi_reg.data_xfer; }), NAME([this](uint32_t data) { LOG("write spifi_reg.data_xfer = 0x%x\n", data); spifi_reg.data_xfer = data; }));
	map(0x48, 0x4b).lrw32(NAME([this]() { LOG("read spifi_reg.autocmd = 0x%x\n", spifi_reg.autocmd); return spifi_reg.autocmd; }), NAME([this](uint32_t data) { LOG("write spifi_reg.autocmd = 0x%x\n", data); spifi_reg.autocmd = data; }));
	map(0x4c, 0x4f).lrw32(NAME([this]() { LOG("read spifi_reg.autostat = 0x%x\n", spifi_reg.autostat); return spifi_reg.autostat; }), NAME([this](uint32_t data) { LOG("write spifi_reg.autostat = 0x%x\n", data); spifi_reg.autostat = data; }));
	map(0x50, 0x53).lrw32(NAME([this]() { LOG("read spifi_reg.resel = 0x%x\n", spifi_reg.resel); return spifi_reg.resel; }), NAME([this](uint32_t data) { LOG("write spifi_reg.resel = 0x%x\n", data); spifi_reg.resel = data; }));
	map(0x54, 0x57).lrw32(NAME([this]() { LOG("read spifi_reg.select = 0x%x\n", spifi_reg.select); return spifi_reg.select; }), NAME([this](uint32_t data) { LOG("write spifi_reg.select = 0x%x\n", data); spifi_reg.select = data; }));
	map(0x58, 0x5b).lrw32(NAME([this]() { LOG("read spifi_reg.prcmd = 0x%x\n", spifi_reg.prcmd); return spifi_reg.prcmd; }), NAME([this](uint32_t data) { LOG("write spifi_reg.prcmd = 0x%x\n", data); spifi_reg.prcmd = data; }));
	map(0x5c, 0x5f).lrw32(NAME([this]() { LOG("read spifi_reg.auxctrl = 0x%x\n", spifi_reg.auxctrl); return spifi_reg.auxctrl; }), NAME([this](uint32_t data) { LOG("write spifi_reg.auxctrl = 0x%x\n", data); spifi_reg.auxctrl = data; }));
	map(0x60, 0x63).lrw32(NAME([this]() { LOG("read spifi_reg.autodata = 0x%x\n", spifi_reg.autodata); return spifi_reg.autodata; }), NAME([this](uint32_t data) { LOG("write spifi_reg.autodata = 0x%x\n", data); spifi_reg.autodata = data; }));
	map(0x64, 0x67).lrw32(NAME([this]() { LOG("read spifi_reg.loopctrl = 0x%x\n", spifi_reg.loopctrl); return spifi_reg.loopctrl; }), NAME([this](uint32_t data) { LOG("write spifi_reg.loopctrl = 0x%x\n", data); spifi_reg.loopctrl = data; }));
	map(0x68, 0x6b).lrw32(NAME([this]() { LOG("read spifi_reg.loopdata = 0x%x\n", spifi_reg.loopdata); return spifi_reg.loopdata; }), NAME([this](uint32_t data) { LOG("write spifi_reg.loopdata = 0x%x\n", data); spifi_reg.loopdata = data; }));
	map(0x6c, 0x6f).lrw32(NAME([this]() { LOG("read spifi_reg.identify = 0x%x\n", spifi_reg.identify); return spifi_reg.identify; }), NAME([this](uint32_t data) { LOG("write spifi_reg.identify = 0x%x\n", data); spifi_reg.identify = data; }));
	map(0x70, 0x73).lrw32(NAME([this]() { LOG("read spifi_reg.complete = 0x%x\n", spifi_reg.complete); return spifi_reg.complete; }), NAME([this](uint32_t data) { LOG("write spifi_reg.complete = 0x%x\n", data); spifi_reg.complete = data; }));
	map(0x74, 0x77).lrw32(NAME([this]() { LOG("read spifi_reg.scsi_status = 0x%x\n", spifi_reg.scsi_status); return spifi_reg.scsi_status; }), NAME([this](uint32_t data) { LOG("write spifi_reg.scsi_status = 0x%x\n", data); spifi_reg.scsi_status = data; }));
	map(0x78, 0x7b).lrw32(NAME([this]() { LOG("read spifi_reg.data = 0x%x\n", spifi_reg.data); return spifi_reg.data; }), NAME([this](uint32_t data) { LOG("write spifi_reg.data = 0x%x\n", data); spifi_reg.data = data; }));
	map(0x7c, 0x7f).lrw32(NAME([this]() { LOG("read spifi_reg.icond = 0x%x\n", spifi_reg.icond); return spifi_reg.icond; }), NAME([this](uint32_t data) { LOG("write spifi_reg.icond = 0x%x\n", data); spifi_reg.icond = data; }));
	map(0x80, 0x83).lrw32(NAME([this]() { LOG("read spifi_reg.fastwide = 0x%x\n", spifi_reg.fastwide); return spifi_reg.fastwide; }), NAME([this](uint32_t data) { LOG("write spifi_reg.fastwide = 0x%x\n", data); spifi_reg.fastwide = data; }));
	map(0x84, 0x87).lrw32(NAME([this]() { LOG("read spifi_reg.exctrl = 0x%x\n", spifi_reg.exctrl); return spifi_reg.exctrl; }), NAME([this](uint32_t data) { LOG("write spifi_reg.exctrl = 0x%x\n", data); spifi_reg.exctrl = data; }));
	map(0x88, 0x8b).lrw32(NAME([this]() { LOG("read spifi_reg.exstat = 0x%x\n", spifi_reg.exstat); return spifi_reg.exstat; }), NAME([this](uint32_t data) { LOG("write spifi_reg.exstat = 0x%x\n", data); spifi_reg.exstat = data; }));
	map(0x8c, 0x8f).lrw32(NAME([this]() { LOG("read spifi_reg.test = 0x%x\n", spifi_reg.test); return spifi_reg.test; }), NAME([this](uint32_t data) { LOG("write spifi_reg.test = 0x%x\n", data); spifi_reg.test = data; }));
	map(0x90, 0x93).lrw32(NAME([this]() { LOG("read spifi_reg.quematch = 0x%x\n", spifi_reg.quematch); return spifi_reg.quematch; }), NAME([this](uint32_t data) { LOG("write spifi_reg.quematch = 0x%x\n", data); spifi_reg.quematch = data; }));
	map(0x94, 0x97).lrw32(NAME([this]() { LOG("read spifi_reg.quecode = 0x%x\n", spifi_reg.quecode); return spifi_reg.quecode; }), NAME([this](uint32_t data) { LOG("write spifi_reg.quecode = 0x%x\n", data); spifi_reg.quecode = data; }));
	map(0x98, 0x9b).lrw32(NAME([this]() { LOG("read spifi_reg.quetag = 0x%x\n", spifi_reg.quetag); return spifi_reg.quetag; }), NAME([this](uint32_t data) { LOG("write spifi_reg.quetag = 0x%x\n", data); spifi_reg.quetag = data; }));
	map(0x9c, 0x9f).lrw32(NAME([this]() { LOG("read spifi_reg.quepage = 0x%x\n", spifi_reg.quepage); return spifi_reg.quepage; }), NAME([this](uint32_t data) { LOG("write spifi_reg.quepage = 0x%x\n", data); spifi_reg.quepage = data; }));

	// TODO: command buffer
}

//void spifi3_device::device_start()
//{
	/*
	m_irq_out_cb.resolve_safe();
	m_drq_out_cb.resolve_safe();
	m_port_out_cb.resolve_safe();

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

	m_state_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(cxd1185_device::state_timer), this));
	m_state = IDLE;

	m_irq_asserted = false;
	m_drq_asserted = false;

	// monitor all scsi bus control lines
	scsi_bus->ctrl_wait(scsi_refid, S_ALL, S_ALL);
	*/
//}

//void spifi3_device::device_reset()
//{
	/*
	m_environ = 0;

	reset_chip();
	*/
//}

/*
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
		break;
	case 0x42:
		LOGMASKED(LOG_CMD, "select target %d with atn\n", (m_scsi_id & TID) >> 5);
		m_status |= INIT;
		m_state = ARB_BUS_FREE;
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
		break;
	case 0xc1:
		LOGMASKED(LOG_CMD, "transfer pad\n");
		m_state = XFR_INFO;
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

void cxd1185_device::state_timer(void *ptr, s32 param)
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
				if (m_command & DMA)
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
*/

//void spifi3_device::scsi_ctrl_changed()
//{
	/*
	u32 const ctrl = scsi_bus->ctrl_r();

	if ((ctrl & S_BSY) && !(ctrl & S_SEL))
		LOGMASKED(LOG_SCSI, "scsi_ctrl_changed 0x%x\n phase %s%s%s\n", ctrl, nscsi_phase[ctrl & S_PHASE_MASK],
			ctrl & S_REQ ? " REQ" : "", ctrl & S_ACK ? " ACK" : "");
	else if (ctrl & S_BSY)
		LOGMASKED(LOG_SCSI, "scsi_ctrl_changed 0x%x\n arbitration/selection\n", ctrl);
	else if (ctrl & S_RST)
		LOGMASKED(LOG_SCSI, "scsi_ctrl_changed 0x%x\n BUS RESET\n", ctrl);
	else
		LOGMASKED(LOG_SCSI, "scsi_ctrl_changed 0x%x\n BUS FREE\n", ctrl);

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
	*/
//}

/*
void cxd1185_device::int_check()
{
	bool irq_asserted = false;

	// update mirq
	if (m_int_req[0] || m_int_req[1])
	{
		m_status |= MIRQ;

		irq_asserted = (m_int_req[0] & m_int_auth[0]) || (m_int_req[1] & m_int_auth[1]);
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

	LOGMASKED(LOG_GENERAL, "port_w 0x%02x mask 0x%02x\n", data, mask);

	m_ioport &= ~mask;
	m_ioport |= data & mask;
}
*/