// license:BSD-3-Clause
// copyright-holders:Brice Onken

/*
 * HP 1TV3-0302 SPIFI3-SE SCSI controller
 *
 * References:
 * - https://github.com/NetBSD/src/blob/trunk/sys/arch/newsmips/apbus/spifireg.h
 * - https://github.com/NetBSD/src/blob/trunk/sys/arch/newsmips/apbus/spifi.c
 *
 * TODO: Everything. This is a placeholder that only logs read/writes.
 */

#include "emu.h"
#include "spifi3.h"

#define VERBOSE 1
#include "logmacro.h"

DEFINE_DEVICE_TYPE(SPIFI3, spifi3_device, "spifi3", "HP 1TV3-0302 SPIFI3 SCSI 1 Protocol Controller")

spifi3_device::spifi3_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: nscsi_device(mconfig, SPIFI3, tag, owner, clock)
	, nscsi_slot_card_interface(mconfig, *this, DEVICE_SELF)
{
}

void spifi3_device::map(address_map &map)
{
	// Ugly address map
	map(0x00, 0x03).lrw32(NAME([this]() { LOG("read spifi_reg.spstat = 0x%x\n", spifi_reg.spstat); return spifi_reg.spstat; }), NAME([this](uint32_t data) { LOG("write spifi_reg.spstat = 0x%x\n", data); spifi_reg.spstat = data; }));
	map(0x04, 0x07).lrw32(NAME([this]() { LOG("read spifi_reg.cmlen = 0x%x\n", spifi_reg.cmlen); return spifi_reg.cmlen; }), NAME([this](uint32_t data) { LOG("write spifi_reg.cmlen = 0x%x\n", data); spifi_reg.cmlen = data; }));
	map(0x08, 0x0b).lrw32(NAME([this]() { LOG("read spifi_reg.cmdpage = 0x%x\n", spifi_reg.cmdpage); return spifi_reg.cmdpage; }), NAME([this](uint32_t data) { LOG("write spifi_reg.cmdpage = 0x%x\n", data); spifi_reg.cmdpage = data; }));
	map(0x0c, 0x0f).lrw32(NAME([this]() { LOG("read spifi_reg.count_hi = 0x%x\n", spifi_reg.count_hi); return spifi_reg.count_hi; }), NAME([this](uint32_t data) { LOG("write spifi_reg.count_hi = 0x%x\n", data); spifi_reg.count_hi = data; }));
	map(0x10, 0x13).lrw32(NAME([this]() { LOG("read spifi_reg.count_mid = 0x%x\n", spifi_reg.count_mid); return spifi_reg.count_mid; }), NAME([this](uint32_t data) { LOG("write spifi_reg.count_mid = 0x%x\n", data); spifi_reg.count_mid = data; }));
	map(0x14, 0x17).lrw32(NAME([this]() { LOG("read spifi_reg.count_low = 0x%x\n", spifi_reg.count_low); return spifi_reg.count_low; }), NAME([this](uint32_t data) { LOG("write spifi_reg.count_low = 0x%x\n", data); spifi_reg.count_low = data; }));
	map(0x18, 0x1b).lrw32(NAME([this]() { LOG("read spifi_reg.svptr_hi = 0x%x\n", spifi_reg.svptr_hi); return spifi_reg.svptr_hi; }), NAME([this](uint32_t data) { LOG("write spifi_reg.svptr_hi = 0x%x\n", data); spifi_reg.svptr_hi = data; }));
	map(0x1c, 0x1f).lrw32(NAME([this]() { LOG("read spifi_reg.svptr_mid = 0x%x\n", spifi_reg.svptr_mid); return spifi_reg.svptr_mid; }), NAME([this](uint32_t data) { LOG("write spifi_reg.svptr_mid = 0x%x\n", data); spifi_reg.svptr_mid = data; }));
	map(0x20, 0x23).lrw32(NAME([this]() { LOG("read spifi_reg.svptr_low = 0x%x\n", spifi_reg.svptr_low); return spifi_reg.svptr_low; }), NAME([this](uint32_t data) { LOG("write spifi_reg.svptr_low = 0x%x\n", data); spifi_reg.svptr_low = data; }));
	map(0x24, 0x27).lrw32(NAME([this]() { LOG("read spifi_reg.intr = 0x%x (%s)\n", spifi_reg.intr, machine().describe_context()); return spifi_reg.intr; }), NAME([this](uint32_t data) { LOG("write spifi_reg.intr = 0x%x\n", data); spifi_reg.intr = data; }));
	map(0x28, 0x2b).lrw32(NAME([this]() { LOG("read spifi_reg.imask = 0x%x\n", spifi_reg.imask); return spifi_reg.imask; }), NAME([this](uint32_t data) { LOG("write spifi_reg.imask = 0x%x\n", data); spifi_reg.imask = data; }));
	map(0x2c, 0x2f).lrw32(NAME([this]() { LOG("read spifi_reg.prctrl = 0x%x\n", spifi_reg.prctrl); return spifi_reg.prctrl; }), NAME([this](uint32_t data) { LOG("write spifi_reg.prctrl = 0x%x\n", data); spifi_reg.prctrl = data; }));
	map(0x30, 0x33).lrw32(NAME([this]() { LOG("read spifi_reg.prstat = 0x%x\n", spifi_reg.prstat); return spifi_reg.prstat; }), NAME([this](uint32_t data) { LOG("write spifi_reg.prstat = 0x%x\n", data); spifi_reg.prstat = data; }));
	map(0x34, 0x37).lrw32(NAME([this]() { LOG("read spifi_reg.init_status = 0x%x\n", spifi_reg.init_status); return spifi_reg.init_status; }), NAME([this](uint32_t data) { LOG("write spifi_reg.init_status = 0x%x\n", data); spifi_reg.init_status = data; }));
	map(0x38, 0x3b).rw(FUNC(spifi3_device::fifoctrl_r), FUNC(spifi3_device::fifoctrl_w));
	map(0x3c, 0x3f).lrw32(NAME([this]() { LOG("read spifi_reg.fifodata = 0x%x\n", spifi_reg.fifodata); return spifi_reg.fifodata; }), NAME([this](uint32_t data) { LOG("write spifi_reg.fifodata = 0x%x\n", data); spifi_reg.fifodata = data; }));
	map(0x40, 0x43).lrw32(NAME([this]() { LOG("read spifi_reg.config = 0x%x\n", spifi_reg.config); return spifi_reg.config; }), NAME([this](uint32_t data) { LOG("write spifi_reg.config = 0x%x\n", data); spifi_reg.config = data; }));
	map(0x44, 0x47).lrw32(NAME([this]() { LOG("read spifi_reg.data_xfer = 0x%x\n", spifi_reg.data_xfer); return spifi_reg.data_xfer; }), NAME([this](uint32_t data) { LOG("write spifi_reg.data_xfer = 0x%x\n", data); spifi_reg.data_xfer = data; }));
	map(0x48, 0x4b).lrw32(NAME([this]() { LOG("read spifi_reg.autocmd = 0x%x\n", spifi_reg.autocmd); return spifi_reg.autocmd; }), NAME([this](uint32_t data) { LOG("write spifi_reg.autocmd = 0x%x\n", data); spifi_reg.autocmd = data; }));
	map(0x4c, 0x4f).lrw32(NAME([this]() { LOG("read spifi_reg.autostat = 0x%x\n", spifi_reg.autostat); return spifi_reg.autostat; }), NAME([this](uint32_t data) { LOG("write spifi_reg.autostat = 0x%x\n", data); spifi_reg.autostat = data; }));
	map(0x50, 0x53).lrw32(NAME([this]() { LOG("read spifi_reg.resel = 0x%x\n", spifi_reg.resel); return spifi_reg.resel; }), NAME([this](uint32_t data) { LOG("write spifi_reg.resel = 0x%x\n", data); spifi_reg.resel = data; }));
	map(0x54, 0x57).lrw32(NAME([this]() { LOG("read spifi_reg.select = 0x%x\n", spifi_reg.select); return spifi_reg.select; }), NAME([this](uint32_t data) { LOG("write spifi_reg.select = 0x%x\n", data); spifi_reg.select = data; }));
	map(0x58, 0x5b).lrw32(NAME([this]() { LOG("read spifi_reg.prcmd = 0x%x\n", spifi_reg.prcmd); return spifi_reg.prcmd; }), NAME([this](uint32_t data) { LOG("write spifi_reg.prcmd = 0x%x\n", data); spifi_reg.prcmd = data; }));
	map(0x5c, 0x5f).rw(FUNC(spifi3_device::auxctrl_r), FUNC(spifi3_device::auxctrl_w));
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
	// mirror of above values goes here
	map(0x200, 0x3ff).rw(FUNC(spifi3_device::cmd_buf_r), FUNC(spifi3_device::cmd_buf_w)).umask32(0xff);
}

uint8_t spifi3_device::cmd_buf_r(offs_t offset)
{
	// find which cmd entry
	// 8 slots in the buffer, 16 bytes each
	// so, divide the offset by 16 (truncated) to get the cmd entry
	int cmd_entry = offset / 16;

	// now, return the right item
	// this is ugly, I need to improve this
	uint8_t result = 0;
	int register_offset = offset % 16;
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

	LOG("SPIFI3: cmd_buf_r(0x%x) -> 0x%x\n", offset, result);

	return result;
}

void spifi3_device::cmd_buf_w(offs_t offset, uint8_t data)
{
	LOG("SPIFI3: cmd_buf_w(0x%x, 0x%x)\n", offset, data);
	// find which cmd entry
	// 8 slots in the buffer, 16 bytes each
	// so, divide the offset by 16 (truncated) to get the cmd entry
	int cmd_entry = offset / 16;

	// now, return the right item
	// this is ugly, I need to improve this
	int register_offset = offset % 16;
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

uint32_t spifi3_device::auxctrl_r()
{
	LOG("read spifi_reg.auxctrl = 0x%x\n", spifi_reg.auxctrl);
	return spifi_reg.auxctrl;
}

void spifi3_device::auxctrl_w(uint32_t data)
{
	LOG("write spifi_reg.auxctrl = 0x%x\n", data);
	spifi_reg.auxctrl = data;
	if(spifi_reg.auxctrl & AUXCTRL_SRST)
	{
		// reset of some kind
		LOG("SRST asserted\n");
	}
	if(spifi_reg.auxctrl & AUXCTRL_CRST)
	{
		// chip reset?
		LOG("CRST asserted\n");
	}
	if(spifi_reg.auxctrl & AUXCTRL_SETRST)
	{
		// bus reset?
		LOG("SETRST asserted\n");
	}
	if(spifi_reg.auxctrl & AUXCTRL_DMAEDGE)
	{
		// do we need to take action here?
		LOG("DMAEDGE asserted\n");
	}
}

uint32_t spifi3_device::fifoctrl_r()
{
	LOG("read spifi_reg.fifoctrl = 0x%x\n", spifi_reg.fifoctrl);
	// TODO: calculate free FIFO slots to fill FIFOC_FSLOT
	return spifi_reg.fifoctrl;
}

void spifi3_device::fifoctrl_w(uint32_t data)
{
	LOG("write spifi_reg.fifoctrl = 0x%x\n", data);
	spifi_reg.fifoctrl = data; // TODO: this might not be persisted - read/write might be different. TBD.
	if(spifi_reg.fifoctrl & FIFOC_SSTKACT) { LOG("fifoctrl.SSTKACT: unimplemented"); } // likely RO guess: NetBSD uses this to know when synchronous data should be loaded into the FIFO?
	if(spifi_reg.fifoctrl & FIFOC_RQOVRN) { LOG("fifoctrl.RQOVRN: unimplemented"); } // likely RO - Whatever this is, it would cause NetBSD to panic
	if(spifi_reg.fifoctrl & FIFOC_CLREVEN) { LOG("fifoctrl.CLREVEN: unimplemented"); } // clear FIFO
	if(spifi_reg.fifoctrl & FIFOC_CLRODD) { LOG("fifoctrl.CLRODD: unimplemented"); } // ??? clear of some sort
	if(spifi_reg.fifoctrl & FIFOC_FLUSH) { LOG("fifoctrl.FLUSH: unimplemented"); } // flush FIFO
	if(spifi_reg.fifoctrl & FIFOC_LOAD) { LOG("fifoctrl.LOAD: unimplemented"); } // Load FIFO synchronously
}
