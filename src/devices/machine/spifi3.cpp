// license:BSD-3-Clause
// copyright-holders:Brice Onken,Olivier Galibert

/*
 * HP 1TV3-0302 SPIFI3-SE SCSI controller
 *
 * References:
 * - https://github.com/NetBSD/src/blob/trunk/sys/arch/newsmips/apbus/spifireg.h
 * - https://github.com/NetBSD/src/blob/trunk/sys/arch/newsmips/apbus/spifi.c
 * - https://github.com/mamedev/mame/blob/master/src/devices/machine/ncr5390.cpp
 */

#include "emu.h"
#include "spifi3.h"

//#define VERBOSE 1
#include "logmacro.h"

#define DELAY_HACK // TODO:

DEFINE_DEVICE_TYPE(SPIFI3, spifi3_device, "spifi3", "HP 1TV3-0302 SPIFI3 SCSI-2 Protocol Controller")

spifi3_device::spifi3_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
    : nscsi_device(mconfig, SPIFI3, tag, owner, clock),
      nscsi_slot_card_interface(mconfig, *this, DEVICE_SELF),
      m_irq_handler(*this),
      m_drq_handler(*this)
{
}

void spifi3_device::device_start()
{
    nscsi_device::device_start();

    /*
    save_item(NAME(command));
    save_item(NAME(config));
    save_item(NAME(status));
    save_item(NAME(istatus));
    save_item(NAME(clock_conv));
    save_item(NAME(sync_offset));
    save_item(NAME(sync_period));
    save_item(NAME(bus_id));
    save_item(NAME(select_timeout));
    save_item(NAME(seq));
    save_item(NAME(fifo));
    save_item(NAME(tcount));
    save_item(NAME(tcounter));
    save_item(NAME(mode));
    save_item(NAME(fifo_pos));
    save_item(NAME(command_pos));
    save_item(NAME(state));
    save_item(NAME(xfr_phase));
    save_item(NAME(command_length));
    save_item(NAME(dma_dir));
    save_item(NAME(irq));
    save_item(NAME(drq));
    save_item(NAME(test_mode));
    */

    m_irq_handler.resolve_safe();
    m_drq_handler.resolve_safe();

    // config = 0;
    // select_timeout = 0;

    bus_id = 0;
    tm = timer_alloc(0);
}

void spifi3_device::map(address_map &map)
{
    // Ugly address map
    map(0x00, 0x03).lrw32(NAME([this]()
                               {
                                   uint32_t spstat = spifi_reg.spstat << 4 | ((spifi_reg.intr > 0) ? SPS_INTR : 0);
                                   LOG("read spifi_reg.spstat = 0x%x\n", spstat);
                                   return spstat;
                               }),
                          NAME([this](uint32_t data)
                               {
                                   LOG("write spifi_reg.spstat = 0x%x\n", data);
                                   spifi_reg.spstat = data;
                               }));

    map(0x08, 0x0b).lrw32(NAME([this]() { LOG("read spifi_reg.cmdpage = 0x%x\n", spifi_reg.cmdpage); return spifi_reg.cmdpage; }), NAME([this](uint32_t data) { LOG("write spifi_reg.cmdpage = 0x%x\n", data); spifi_reg.cmdpage = data; }));
    map(0x18, 0x1b).lrw32(NAME([this]() { LOG("read spifi_reg.svptr_hi = 0x%x\n", spifi_reg.svptr_hi); return spifi_reg.svptr_hi; }), NAME([this](uint32_t data) { LOG("write spifi_reg.svptr_hi = 0x%x\n", data); spifi_reg.svptr_hi = data; }));
    map(0x1c, 0x1f).lrw32(NAME([this]() { LOG("read spifi_reg.svptr_mid = 0x%x\n", spifi_reg.svptr_mid); return spifi_reg.svptr_mid; }), NAME([this](uint32_t data) { LOG("write spifi_reg.svptr_mid = 0x%x\n", data); spifi_reg.svptr_mid = data; }));
    map(0x20, 0x23).lrw32(NAME([this]() { LOG("read spifi_reg.svptr_low = 0x%x\n", spifi_reg.svptr_low); return spifi_reg.svptr_low; }), NAME([this](uint32_t data) { LOG("write spifi_reg.svptr_low = 0x%x\n", data); spifi_reg.svptr_low = data; }));
    map(0x28, 0x2b).lrw32(NAME([this]() { LOG("read spifi_reg.imask = 0x%x\n", spifi_reg.imask); return spifi_reg.imask; }), NAME([this](uint32_t data) { LOG("write spifi_reg.imask = 0x%x\n", data); spifi_reg.imask = data; }));
    map(0x2c, 0x2f).lrw32(NAME([this]() { LOG("read spifi_reg.prctrl = 0x%x\n", spifi_reg.prctrl); return spifi_reg.prctrl; }), NAME([this](uint32_t data) { LOG("write spifi_reg.prctrl = 0x%x\n", data); spifi_reg.prctrl = data; }));
    map(0x3c, 0x3f).lrw32(NAME([this]() { LOG("read spifi_reg.fifodata = 0x%x\n", spifi_reg.fifodata); return spifi_reg.fifodata; }), NAME([this](uint32_t data) { LOG("write spifi_reg.fifodata = 0x%x\n", data); spifi_reg.fifodata = data; }));
    map(0x44, 0x47).lrw32(NAME([this]() { LOG("read spifi_reg.data_xfer = 0x%x\n", spifi_reg.data_xfer); return spifi_reg.data_xfer; }), NAME([this](uint32_t data) { LOG("write spifi_reg.data_xfer = 0x%x\n", data); spifi_reg.data_xfer = data; }));
    map(0x48, 0x4b).lrw32(NAME([this]() { LOG("read spifi_reg.autocmd = 0x%x\n", spifi_reg.autocmd); return spifi_reg.autocmd; }), NAME([this](uint32_t data) { LOG("write spifi_reg.autocmd = 0x%x\n", data); spifi_reg.autocmd = data; }));
    map(0x50, 0x53).lrw32(NAME([this]() { LOG("read spifi_reg.resel = 0x%x\n", spifi_reg.resel); return spifi_reg.resel; }), NAME([this](uint32_t data) { LOG("write spifi_reg.resel = 0x%x\n", data); spifi_reg.resel = data; }));
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

    // stuff I'm actively working on
    map(0x04, 0x07).lrw32(NAME([this]()
                               {
                                   LOG("read spifi_reg.cmlen = 0x%x\n", spifi_reg.cmlen);
                                   return spifi_reg.cmlen;
                               }),
                          NAME([this](uint32_t data)
                               {
                                   LOG("write spifi_reg.cmlen = 0x%x\n", data);
                                   spifi_reg.cmlen = data;
                                   spifi_reg.icond &= ~ICOND_CNTZERO; // Not sure if this is where this is actually cleared.
                                                                      // Putting it here prevents NEWS-OS from trying to
                                                                      // transition to the DATAOUT phase too early when it sees
                                                                      // the CNTZERO condition flag
                               }));
    map(0x0c, 0x0f).lrw32(NAME([this]()
                               {
                                   uint8_t count_hi = (tcounter >> 16) & 0xff;
                                   spifi_reg.icond &= ~ICOND_CNTZERO; // Does a read clear this?
                                   LOG("read spifi_reg.count_hi = 0x%x\n", count_hi);
                                   return count_hi;
                               }),
                          NAME([this](uint32_t data)
                               {
                                   LOG("write spifi_reg.count_hi = 0x%x\n", data);
                                   spifi_reg.icond &= ~ICOND_CNTZERO;
                                   tcounter &= ~0xff0000;
                                   tcounter |= (data & 0xff) << 16;
                               }));
    map(0x10, 0x13).lrw32(NAME([this]()
                               {
                                   uint8_t count_mid = (tcounter >> 8) & 0xff;
                                   spifi_reg.icond &= ~ICOND_CNTZERO; // Does a read clear this?
                                   LOG("read spifi_reg.count_mid = 0x%x\n", count_mid);
                                   return count_mid;
                               }),
                          NAME([this](uint32_t data)
                               {
                                   LOG("write spifi_reg.count_mid = 0x%x\n", data);
                                   spifi_reg.icond &= ~ICOND_CNTZERO;
                                   tcounter &= ~0xff00;
                                   tcounter |= (data & 0xff) << 8;
                               }));

    map(0x14, 0x17).lrw32(NAME([this]()
                               {
                                   uint8_t count_lo = tcounter & 0xff;
                                   spifi_reg.icond &= ~ICOND_CNTZERO; // Does a read clear this?
                                   LOG("read spifi_reg.count_low = 0x%x\n", count_lo);
                                   return count_lo;
                               }),
                          NAME([this](uint32_t data)
                               {
                                   LOG("write spifi_reg.count_low = 0x%x\n", data);
                                   spifi_reg.icond &= ~ICOND_CNTZERO;
                                   tcounter &= ~0xff;
                                   tcounter |= data & 0xff;
                               }));

    map(0x24, 0x27).lrw32(NAME([this]()
    {
        LOG("read spifi_reg.intr = 0x%x (%s)\n", spifi_reg.intr, machine().describe_context());
        return spifi_reg.intr;
    }),
    NAME([this](uint32_t data)
    {
        LOG("write spifi_reg.intr = 0x%x\n", data);
        spifi_reg.intr &= data;
        check_irq();
    }));

    map(0x34, 0x37).lrw32(NAME([this]()
                               {
                                   // NetBSD only lists this bit, but there is probably more in this register.
                                   spifi_reg.init_status = (scsi_bus->ctrl_r() & S_ACK) > 0 ? 0x40 : 0x0; // todo: make constant
                                   LOG("read spifi_reg.init_status = 0x%x\n", spifi_reg.init_status);
                                   return spifi_reg.init_status;
                               }),
                          NAME([this](uint32_t data)
                               {
                                   LOG("write spifi_reg.init_status = 0x%x\n", data);
                                   spifi_reg.init_status = data;
                               }));
    
    map(0x4c, 0x4f).lrw32(NAME([this]() 
                               { 
                                   LOG("read spifi_reg.autostat = 0x%x\n", spifi_reg.autostat); 
                                   return spifi_reg.autostat;
                               }),
                          NAME([this](uint32_t data)
                               { 
                                   LOG("write spifi_reg.autostat = 0x%x\n", data);
                                   spifi_reg.autostat |= data; // This is a total guess based on what the NEWS-OS kernel does with this register.
                                                               // NetBSD absolutely doesn't use it the same way.
                                }));
    

    // Below this line probably won't need to change
    map(0x30, 0x33).r(FUNC(spifi3_device::prstat_r));
    map(0x38, 0x3b).rw(FUNC(spifi3_device::fifoctrl_r), FUNC(spifi3_device::fifoctrl_w));
    map(0x40, 0x43).lrw32(NAME([this]() { LOG("read spifi_reg.config = 0x%x\n", spifi_reg.config); return spifi_reg.config; }), NAME([this](uint32_t data) { LOG("write spifi_reg.config = 0x%x\n", data); spifi_reg.config = data; }));
    map(0x54, 0x57).w(FUNC(spifi3_device::select_w));
    map(0x54, 0x57).lr32(NAME([this]() { LOG("read spifi_reg.select = 0x%x\n", spifi_reg.select); return spifi_reg.select; }));
    map(0x58, 0x5b).rw(FUNC(spifi3_device::prcmd_r), FUNC(spifi3_device::prcmd_w));
    map(0x5c, 0x5f).rw(FUNC(spifi3_device::auxctrl_r), FUNC(spifi3_device::auxctrl_w));
    map(0x60, 0x63).w(FUNC(spifi3_device::autodata_w));
    map(0x60, 0x63).lr32(NAME([this]() { LOG("read spifi_reg.autodata = 0x%x\n", spifi_reg.autodata); return spifi_reg.autodata; }));

    // Map command buffer
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

    // now, write the appropriate item
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
        spifi_reg = {}; // Reset register values
        dma_command = false;
        dma_dir = DMA_NONE;
        tcounter = 0;
        command_pos = 0;
        command_length = 0;
    }
    if(spifi_reg.auxctrl & AUXCTRL_SETRST)
    {
        // bus reset?
        LOG("SETRST asserted\n");
    }
    if(spifi_reg.auxctrl & AUXCTRL_DMAEDGE)
    {
        // do we need to take action here? might be what enables DMA mode/DRQ?
        LOG("DMAEDGE asserted\n");
    }
}

uint32_t spifi3_device::fifoctrl_r()
{
    LOG("read spifi_reg.fifoctrl = 0x%x\n", spifi_reg.fifoctrl);

    auto evenCount = 8 - m_even_fifo.size(); // How does the count actually work? need to test
    spifi_reg.fifoctrl &= ~FIFOC_FSLOT;
    spifi_reg.fifoctrl |= evenCount & FIFOC_FSLOT;

    return spifi_reg.fifoctrl;
}

void spifi3_device::fifoctrl_w(uint32_t data)
{
    LOG("write spifi_reg.fifoctrl = 0x%x\n", data);
    spifi_reg.fifoctrl = data & ~FIFOC_FSLOT; // TODO: this might not be persisted - read/write might be different. TBD.
    if(spifi_reg.fifoctrl & FIFOC_SSTKACT) { LOG("fifoctrl.SSTKACT: w unimplemented"); } // likely RO guess: NetBSD uses this to know when synchronous data should be loaded into the FIFO?
    if(spifi_reg.fifoctrl & FIFOC_RQOVRN) { LOG("fifoctrl.RQOVRN: w unimplemented"); } // likely RO - Whatever this is, it would cause NetBSD to panic
    if(spifi_reg.fifoctrl & FIFOC_CLREVEN)
    {
        LOG("Clearing even FIFO of %d items\n", m_even_fifo.size());
        while (!m_even_fifo.empty())
        {
            m_even_fifo.pop();
        }
    }
    if(spifi_reg.fifoctrl & FIFOC_CLRODD)
    {
        LOG("Clearing odd FIFO of %d items\n", m_odd_fifo.size());
        while (!m_odd_fifo.empty())
        {
            m_odd_fifo.pop();
        }
    }
    if(spifi_reg.fifoctrl & FIFOC_FLUSH) { LOG("fifoctrl.FLUSH: unimplemented"); } // flush FIFO - kick off DMA regardless of FIFO count, I assume
    if(spifi_reg.fifoctrl & FIFOC_LOAD) { LOG("fifoctrl.LOAD: unimplemented"); } // Load FIFO synchronously (only needed for SDTR mode?)
}

void spifi3_device::clear_fifo()
{
    LOG("Clearing even FIFO of %d items\n", m_even_fifo.size());
    while (!m_even_fifo.empty())
    {
        m_even_fifo.pop();
    }
    LOG("Clearing odd FIFO of %d items\n", m_odd_fifo.size());
    while (!m_odd_fifo.empty())
    {
        m_odd_fifo.pop();
    }
}

void spifi3_device::select_w(uint32_t data)
{
    LOG("write spifi_reg.select = 0x%x\n", data);
    spifi_reg.select = data & ~SEL_ISTART;

    if(data & SEL_ISTART)
    {
        LOG("Select started! Targeting ID %d\n", (data & SEL_TARGET) >> 4);
        state = DISC_SEL_ARBITRATION_INIT;
        // spifi_reg.spstat = SPS_SEL;
        dma_set(DMA_OUT);
        arbitrate();
    }
}

void spifi3_device::autodata_w(uint32_t data)
{
    LOG("write spifi_reg.autodata = 0x%x\n", data);
    spifi_reg.autodata = data;

    if(spifi_reg.autodata & ADATA_EN)
    {
        LOG("autodata enabled! target %d direction %s\n", spifi_reg.autodata & ADATA_TARGET_ID, spifi_reg.autodata & ADATA_IN ? "in" : "out");
    }
}

uint32_t spifi3_device::prstat_r()
{
    auto ctrl = scsi_bus->ctrl_r();
    uint32_t prstat = 0;
    prstat |= (ctrl & S_ATN) ? PRS_ATN : 0;
    prstat |= (ctrl & S_MSG) ? PRS_MSG : 0;
    prstat |= (ctrl & S_CTL) ? PRS_CD : 0;
    prstat |= (ctrl & S_INP) ? PRS_IO : 0;
    // TODO: PRS_Z, which is 1 when the bus is free, and 0 during REQ/ACK handshakes
    spifi_reg.prstat = prstat; // Might be able to get rid of the register copy of this since we can compute it on demand.
    LOG("read spifi_reg.prstat = 0x%x\n", prstat);
    return prstat;
}

uint32_t spifi3_device::prcmd_r()
{
    LOG("read spifi_reg.prcmd = 0x%x\n", spifi_reg.prcmd);
    return spifi_reg.prcmd;
}

void spifi3_device::prcmd_w(uint32_t data)
{
    LOG("write spifi_reg.prcmd = 0x%x\n", data);
    spifi_reg.prcmd = data;

    switch(data)
    {
        // TODO: a lot of these can be consolidated
        case PRC_DATAOUT:
        {
            LOG("Got DATAOUT command! Starting data output phase...\n");
            state = INIT_XFR;
            xfr_phase = scsi_bus->ctrl_r() & S_PHASE_MASK;

            dma_command = true; // TODO: This seems to be triggered by a write to the AUTODATA register. Not sure how to "reset" that, so to speak
            dma_set(DMA_OUT); // TODO: This seems to be triggered by a write to the AUTODATA register. Not sure how to "reset" that, so to speak
            check_drq();
            step(false);
            break;
        }
        case PRC_DATAIN:
        {
            LOG("Got DATAIN command! Starting data input phase...\n");
            state = INIT_XFR;
            xfr_phase = scsi_bus->ctrl_r() & S_PHASE_MASK;

            dma_command = (spifi_reg.autodata & ADATA_IN) > 0; // TODO: ID check
            dma_set(dma_command ? ((xfr_phase & S_INP) ? DMA_IN : DMA_OUT) : DMA_NONE);
            check_drq();
            step(false);
            break;
        }
        case PRC_COMMAND:
        {
            LOG("Got COMMAND command! Starting command...\n");
            state = INIT_XFR;
            xfr_phase = scsi_bus->ctrl_r() & S_PHASE_MASK;

            dma_command = (spifi_reg.autodata & ADATA_EN) > 0; // TODO: ID check
            command_pos = 0;
            dma_set(dma_command ? ((xfr_phase & S_INP) ? DMA_IN : DMA_OUT) : DMA_NONE);
            check_drq();
            step(false);
            break;
        }
        case PRC_STATUS:
        {
            LOG("Got STATUS command! Starting transfer of status information...\n");
            state = INIT_XFR;
            xfr_phase = scsi_bus->ctrl_r() & S_PHASE_MASK;

            // dma_set(dma_command ? ((xfr_phase & S_INP) ? DMA_IN : DMA_OUT) : DMA_NONE); // TODO: proper setting of DMA???
            dma_command = false; // TODO:
            dma_set(DMA_NONE); // TODO:
            check_drq();
            step(false);
            break;
        }
        case PRC_TRPAD:
        {
            LOG("Transfer pad\n");
            xfr_phase = scsi_bus->ctrl_r() & S_PHASE_MASK;
            if(xfr_phase & S_INP)
            {
                state = INIT_XFR_RECV_PAD_WAIT_REQ;
            }
            else
            {
                state = INIT_XFR_SEND_PAD_WAIT_REQ;
            }
            scsi_bus->ctrl_w(scsi_refid, 0, S_ACK);
            step(false);
            break;
        }
        case PRC_MSGOUT:
        {
            LOG("Got MSGOUT command! Starting message output phase...\n");
            state = INIT_XFR;
            xfr_phase = scsi_bus->ctrl_r() & S_PHASE_MASK;

            dma_command = false; // TODO:
            dma_set(DMA_NONE); // TODO:
            check_drq();
            step(false);
            break;
        }
        case PRC_MSGIN:
        {
            LOG("Got MSGIN command! Starting message input phase...\n");
            state = INIT_XFR;
            xfr_phase = scsi_bus->ctrl_r() & S_PHASE_MASK;

            dma_command = false; // TODO:
            dma_set(DMA_NONE); // TODO:
            check_drq();
            step(false);
            break;
        }
        default:
        {
            LOG("Unimplemented command %d!\n", data);
            break;
        }
    }
}

void spifi3_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
    step(true);
}

void spifi3_device::check_irq()
{
    // There are various ways interrupts can be triggered by the SPIFI - this method is a work in progress.
    bool irqState = (spifi_reg.intr & ~spifi_reg.imask) > 0; // ICOND plays a role here - need to determine how to make sure it is in sync
    LOG("Checking IRQ state - is %d, going to %d\n", irq, irqState);
    if (irq != irqState)
    {
        LOG("Setting IRQ line to %d\n", irqState);
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
            drq_state = !transfer_count_zero() && m_even_fifo.size() < 8;
            break;
        }
    }

    if (drq_state != drq)
    {
        LOG("DRQ changed to %d!\n", drq_state);
        drq = drq_state;
        m_drq_handler(drq);
    }
}

bool spifi3_device::transfer_count_zero()
{
    return (spifi_reg.icond & ICOND_CNTZERO) > 0;
}

void spifi3_device::reset_disconnect()
{
    scsi_bus->ctrl_w(scsi_refid, 0, ~S_RST);

    command_pos = 0;
    command_length = 0;
    // memset(command, 0, sizeof(command)); TODO: spifi3 equiv
    mode = MODE_D;
}

void spifi3_device::send_cmd_byte()
{
    state = (state & STATE_MASK) | (SEND_WAIT_SETTLE << SUB_SHIFT);

    if((state & STATE_MASK) != INIT_XFR_SEND_PAD && ((state & STATE_MASK) != DISC_SEL_SEND_BYTE || command_length))
    {
        // Send next data from cmbuf.
        if(command_pos > 11)
        {
            fatalerror("Tried to send command past the end of cdb! Command_pos: %d", command_pos);
        }
        LOG("Sending byte from cmbuf[%d].cdb[%d] = 0x%x\n", scsi_id, command_pos, spifi_reg.cmbuf[scsi_id].cdb[command_pos]);
        scsi_bus->data_w(scsi_refid, spifi_reg.cmbuf[scsi_id].cdb[command_pos++]);
    }
    else
    {
        // Send 0
        scsi_bus->data_w(scsi_refid, 0);
    }

    scsi_bus->ctrl_w(scsi_refid, S_ACK, S_ACK); // Send ACK
    scsi_bus->ctrl_wait(scsi_refid, S_REQ, S_REQ); // Wait for REQ
    delay_cycles(sync_period); // Delay till next cycle
}

void spifi3_device::send_byte()
{
    if(m_even_fifo.empty())
    {
        fatalerror("spifi3_device::send_byte - Tried to send data with an empty FIFO!\n");
    }

    state = (state & STATE_MASK) | (SEND_WAIT_SETTLE << SUB_SHIFT);

    if((state & STATE_MASK) != INIT_XFR_SEND_PAD && ((state & STATE_MASK) != DISC_SEL_SEND_BYTE || command_length))
    {
        // Send next data from FIFO.
        LOG("tcounter = %d\n", tcounter);
        scsi_bus->data_w(scsi_refid, m_even_fifo.front());
        m_even_fifo.pop();
        check_drq();
    }
    else
    {
        // Send 0
        scsi_bus->data_w(scsi_refid, 0);
    }

    scsi_bus->ctrl_w(scsi_refid, S_ACK, S_ACK);
    scsi_bus->ctrl_wait(scsi_refid, S_REQ, S_REQ);
    delay_cycles(sync_period);
}

void spifi3_device::recv_byte()
{
    LOG("recv_byte started - state = %d.%d\n", state & STATE_MASK, (state & SUB_MASK) >> SUB_SHIFT);
    // Wait for valid input
    scsi_bus->ctrl_wait(scsi_refid, S_REQ, S_REQ);
    state = (state & STATE_MASK) | (RECV_WAIT_REQ_1 << SUB_SHIFT);
    step(false);
}

void spifi3_device::function_bus_complete()
{
    LOG("function_bus_complete\n");
    state = IDLE;
    spifi_reg.spstat = SPS_IDLE; // TODO:
    // was: istatus |= I_FUNCTION|I_BUS;
    spifi_reg.intr |= INTR_FCOMP | INTR_BSRQ; // TODO: icond? is BSRQ 1:1 w/ bus complete?
    spifi_reg.prcmd = 0; // TODO:
    dma_set(DMA_NONE);
    check_drq();
    check_irq();
}

void spifi3_device::function_complete()
{
    LOG("function_complete\n");
    state = IDLE;
    spifi_reg.spstat = SPS_IDLE; // TODO:
    spifi_reg.intr |= INTR_FCOMP; // TODO: icond?
    spifi_reg.prcmd = 0; // TODO:
    dma_set(DMA_NONE);
    check_drq();
    check_irq();
}

void spifi3_device::bus_complete()
{
    LOG("bus_complete\n");
    state = IDLE;
    // was: istatus |= I_BUS;
    spifi_reg.intr |= INTR_BSRQ; // TODO: icond? is BSRQ 1:1 w/ bus complete?
    spifi_reg.prcmd = 0; // TODO:
    dma_set(DMA_NONE);
    check_drq();
    check_irq();
}

void spifi3_device::dma_set(int dir)
{
    dma_dir = dir;

    // account for data already in the fifo
    if (dir == DMA_OUT && !m_even_fifo.empty())
    {
        decrement_tcounter(m_even_fifo.size()); // TODO: is this needed for SPIFI?
    }
}

void spifi3_device::decrement_tcounter(int count)
{
    if (!dma_command)
    {
        return;
    }

    tcounter -= count;
    if(tcounter < 0)
    {
        tcounter = 0;
    }
    if (tcounter == 0)
    {
        spifi_reg.icond |= ICOND_CNTZERO; // TODO: does this immediately trigger an interrupt? or is this just a status thing?
    }
}

void spifi3_device::delay(int cycles)
{
    // TODO: What is clock_conv?
    if(!clock_conv)
    {
        return;
    }
    cycles *= clock_conv;
    tm->adjust(clocks_to_attotime(cycles));
}

void spifi3_device::delay_cycles(int cycles)
{
    tm->adjust(clocks_to_attotime(cycles));
}

void spifi3_device::arbitrate()
{
    //spifi_reg.spstat = SPS_ARB;
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
    LOG("dma_r called! Fifo count = %d, state = %d.%d, tcounter = %d\n", m_even_fifo.size(), state & STATE_MASK, (state & SUB_MASK) >> SUB_SHIFT, tcounter);
    uint8_t val = m_even_fifo.front();
    m_even_fifo.pop();
    decrement_tcounter();
    check_drq();
    step(false);
    return val;
}

void spifi3_device::scsi_ctrl_changed()
{
	uint32_t ctrl = scsi_bus->ctrl_r();
	if(ctrl & S_RST) {
		LOG("scsi bus reset\n");
		return;
	}

	step(false);
}

void spifi3_device::step(bool timeout)
{
    uint32_t ctrl = scsi_bus->ctrl_r();
    uint32_t data = scsi_bus->data_r();

    // SPIFI3 has an 8-slot buffer for commands. NetBSD source code seems to show that the initiator commands
    // are written into cmbuf[id] (7), with the ability to pull info about other IDs from other cmbuf slots.
    // auto command = spifi_reg.cmbuf[scsi_id]; // refid??
    // uint8_t commandStart = command.cdb[0];

    /*LOGMASKED(LOG_STATE, */ LOG("state=%d.%d %s\n", state & STATE_MASK, (state & SUB_MASK) >> SUB_SHIFT, timeout ? "timeout" : "change");

    if(mode == MODE_I && !(ctrl & S_BSY)) // Not busy and we are the initiator. We can disconnect.
    {
        state = IDLE; // Set idle state
        spifi_reg.spstat = SPS_IDLE;
        // TODO: Set Z state flag? Any interrupts needed?
        reset_disconnect();
        check_irq();
    }

    switch(state & SUB_MASK ? state & SUB_MASK : state & STATE_MASK)
    {
        case IDLE:
        {
            // Don't need to do anything
            spifi_reg.spstat = SPS_IDLE;
            break;
        }

        case BUSRESET_WAIT_INT:
        {
            // Bus was reset by a command, go to idle state and clear reset signal
            state = IDLE;
            scsi_bus->ctrl_w(scsi_refid, 0, S_RST);
            reset_disconnect();

            // TODO: Is there a spifi3 equiv of the below?
            /*if (!(config & 0x40)) {
                istatus |= I_SCSI_RESET;
                check_irq();
            }*/
            break;
        }

        case ARB_COMPLETE << SUB_SHIFT: // Arbitration process done, check results and assert SEL if we won
        {
            if(!timeout) // Synchronize state to clock
            {
                break;
            }

            // Scan to see if we won arbitration
            int arbitrationWinner;
            for(arbitrationWinner = 7; arbitrationWinner >= 0 && !(data & (1<<arbitrationWinner)); arbitrationWinner--) {};
            if(arbitrationWinner != scsi_id) {
                scsi_bus->data_w(scsi_refid, 0);
                scsi_bus->ctrl_w(scsi_refid, 0, S_ALL);
                fatalerror("spifi3_device::step need to wait for bus free (lost arbitration)\n");
            }

            // Now that we won arbitration, we need to assert SEL and wait for the bus to settle.
            state = (state & STATE_MASK) | (ARB_ASSERT_SEL << SUB_SHIFT);
            scsi_bus->ctrl_w(scsi_refid, S_SEL, S_SEL);
            delay(6);
            break;
        }

        case ARB_ASSERT_SEL << SUB_SHIFT: // Won arbitration and asserted SEL, time to write target to data bus
        {
            if(!timeout) // Synchronize state to clock
            {
                break;
            }

            bus_id = (spifi_reg.select & SEL_TARGET) >> 4; // TODO: this is ugly and temporary
            scsi_bus->data_w(scsi_refid, (1 << scsi_id) | (1 << bus_id));
            state = (state & STATE_MASK) | (ARB_SET_DEST << SUB_SHIFT);
            delay_cycles(4);
            break;
        }

        case ARB_SET_DEST << SUB_SHIFT: // Set target, time to release BSY
        {
            if(!timeout) // Synchronize state to clock
            {
                break;
            }

            state = (state & STATE_MASK) | (ARB_RELEASE_BUSY << SUB_SHIFT);
            scsi_bus->ctrl_w(scsi_refid, spifi_reg.select & SEL_WATN ? S_ATN : 0, S_ATN | S_BSY); // TODO: is this right? was: scsi_bus->ctrl_w(scsi_refid, c == CD_SELECT_ATN || c == CD_SELECT_ATN_STOP ? S_ATN : 0, S_ATN|S_BSY);
                                                                                                  // Does SPIFI3 have a "STOP" equivalent?
            delay(2);
            break;
        }

        case ARB_RELEASE_BUSY << SUB_SHIFT: // BSY released, if target responds, we need to do the deskew wait
        {
            if(!timeout) // Synchronize state to clock
            {
                break;
            }

            if(ctrl & S_BSY) // Check if target responded
            {
                state = (state & STATE_MASK) | (ARB_DESKEW_WAIT << SUB_SHIFT);
                /* TODO: spifi3 equiv
                if(c == CD_RESELECT)
                {
                    scsi_bus->ctrl_w(scsi_refid, S_BSY, S_BSY);
                }
                */
                delay_cycles(2);
            }
            else // If not, we ran out of time - wait until the next timeout and check again
            {
                state = (state & STATE_MASK) | (ARB_TIMEOUT_BUSY << SUB_SHIFT);

            // TODO: is DELAY_HACK needed?
    #ifdef DELAY_HACK
                delay(1);
    #else
                delay(8192*select_timeout);
    #endif
            }
            break;
        }

        case ARB_DESKEW_WAIT << SUB_SHIFT: // Waited for deskew, now we can proceed to the next state.
        {
            if(!timeout)
            {
                break;
            }

            scsi_bus->data_w(scsi_refid, 0);
            scsi_bus->ctrl_w(scsi_refid, 0, S_SEL); // Clear SEL - target may now assert REQ

            if(false) //c == CD_RESELECT) // TODO: spifi3 equiv for target mode
            {
                LOG("mode switch to Target\n");
                mode = MODE_T; // Target mode not supported for now
            }
            else
            {
                LOG("mode switch to Initiator\n");
                mode = MODE_I;
            }

            state &= STATE_MASK; // Clear sub step
            step(true);
            break;
        }

        case ARB_TIMEOUT_BUSY << SUB_SHIFT: // Timed out during selection, try again
        {
            if(timeout) // No response from target
            {
                scsi_bus->data_w(scsi_refid, 0);
                LOG("select timeout\n");
                state = (state & STATE_MASK) | (ARB_TIMEOUT_ABORT << SUB_SHIFT); // handle timeout
                delay(1000);
            }
            else if(ctrl & S_BSY) // Got response from target, wait before allowing transaction
            {
                state = (state & STATE_MASK) | (ARB_DESKEW_WAIT << SUB_SHIFT);
                /*if(c == CD_RESELECT) TODO: spifi3 equiv
                {
                    scsi_bus->ctrl_w(scsi_refid, S_BSY, S_BSY);
                }*/
                delay_cycles(2);
            }
            break;
        }

        case ARB_TIMEOUT_ABORT << SUB_SHIFT: // Selection timed out - need to abort
        {
            if(!timeout)
            {
                break;
            }

            if(ctrl & S_BSY) // Last chance for target to respond
            {
                state = (state & STATE_MASK) | (ARB_DESKEW_WAIT << SUB_SHIFT);
                /* TODO: spifi3 equiv
                if(c == CD_RESELECT)
                {
                    scsi_bus->ctrl_w(scsi_refid, S_BSY, S_BSY);
                }
                */
                delay_cycles(2);

            }
            else // If not, force bus free
            {
                scsi_bus->ctrl_w(scsi_refid, 0, S_ALL);
                state = IDLE;
                //spifi_reg.spstat = SPS_DISCON << 4; // Set disconnected flag since we don't have a connection. Bit shift to match NetBSD, but that needs to be tested/investigated
                spifi_reg.intr = INTR_TIMEO; // signal timeout
                // TODO: Set Z state flag?
                reset_disconnect();
                check_irq();
            }
            break;
        }

        case SEND_WAIT_SETTLE << SUB_SHIFT:
        {
            if(!timeout)
            {
                break;
            }

            state = (state & STATE_MASK) | (SEND_WAIT_REQ_0 << SUB_SHIFT);
            step(false);
            break;
        }

        case SEND_WAIT_REQ_0 << SUB_SHIFT:
        {
            if(ctrl & S_REQ)
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
            if(!(ctrl & S_REQ))
                break;

            state = (state & STATE_MASK) | (RECV_WAIT_SETTLE << SUB_SHIFT);
            delay_cycles(sync_period);
            break;
        }

        case RECV_WAIT_SETTLE << SUB_SHIFT:
        {
            if(!timeout)
            {
                break;
            }

            if((state & STATE_MASK) != INIT_XFR_RECV_PAD)
            {
                auto data = scsi_bus->data_r();
                LOG("Got 0x%x! FIFO count: %d\n", data, m_even_fifo.size() + 1);
                m_even_fifo.push(data);
                check_drq();
            }
            scsi_bus->ctrl_w(scsi_refid, S_ACK, S_ACK);
            state = (state & STATE_MASK) | (RECV_WAIT_REQ_0 << SUB_SHIFT);
            step(false);
            break;
        }

        case RECV_WAIT_REQ_0 << SUB_SHIFT:
        {
            if(ctrl & S_REQ)
            {
                break;
            }

            state = state & STATE_MASK;
            step(false);
            break;
        }

        case DISC_SEL_ARBITRATION_INIT: // Arbitration and selection complete, time to execute the queued command
        {
            // Wait for a command, or if autoidentify was enabled, just do it
            // This is reverse engineered from what the NWS-5000 MROM does - NetBSD doesn't use this at all
            // so this may not be fully accurate.
            /*if (((spifi_reg.prcmd & PRC_NJMP) || !(spifi_reg.prcmd & PRC_COMMAND)) && !(spifi_reg.identify & 0x80)) // had && !(spifi_reg.cmlen & CML_ACOM_EN) here, but it needs to move
            {
                // dma starts after bus arbitration/selection is complete
                check_drq();
                break;
            } */

            // Extract the command length from the cmlen register and reset our index into the command buffer if a command was given
            /*if(!(spifi_reg.prcmd & PRC_NJMP) && (spifi_reg.prcmd & PRC_COMMAND))
            {
                LOG("Starting command - length = %d\n", spifi_reg.cmlen & CML_LENMASK);
                command_length = spifi_reg.cmlen & CML_LENMASK;
                command_pos = 0;
            }
            
            else if((spifi_reg.cmlen & CML_AMSG_EN) && ((scsi_bus->ctrl_r() & S_PHASE_MASK) == S_PHASE_MSG_OUT))
            {
                LOG("Starting message - len = %d\n", spifi_reg.cmlen & CML_LENMASK);
                command_length = spifi_reg.cmlen & CML_LENMASK;
                command_pos = 0;
            }

            // Otherwise, we are here because the host enabled autoidentify, so CDB isn't the source of the command. Instead, we'll send identify
            else
            {
                */
                LOG("Starting autoidentify...\n");
                command_pos = -1; // TODO: THIS IS TEMPORARY
            // }
            state = DISC_SEL_ARBITRATION;
            step(false);
            break;
        }

        case DISC_SEL_ARBITRATION:
        {
            if(!(spifi_reg.select & SEL_WATN)) //c == CD_SELECT) TODO: is this right? SEL wait with no ATN?
            {
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
            if(!(ctrl & S_REQ)) // Wait for REQ
            {
                break;
            }
            if((ctrl & S_PHASE_MASK) != S_PHASE_MSG_OUT) // No longer in MSG_OUT, we're done
            {
                function_complete();
                break;
            }
            if(spifi_reg.select & SEL_WATN) // c == CD_SELECT_ATN) Deassert ATN now if we asserted it before
            {
                scsi_bus->ctrl_w(scsi_refid, 0, S_ATN);
            }
            state = DISC_SEL_ATN_SEND_BYTE;
            if(command_pos >= 0)
            {
                send_cmd_byte(); // Send the next CDB byte
            }
            else // autoidentify
            {
                // TODO: refactor me
                scsi_bus->data_w(scsi_refid, 0x80);         // TODO: calc DiscPriv and LUNTAR from register
                scsi_bus->ctrl_w(scsi_refid, S_ACK, S_ACK); // Send ACK
                scsi_bus->ctrl_wait(scsi_refid, S_REQ, S_REQ); // Wait for REQ
            }
            break;
        }

        case DISC_SEL_ATN_SEND_BYTE:
        {
            command_length--;
            if (command_pos < 0)
            {
                // TODO: RE ALERT - MIGHT BE WAY OFF
                // autoidentified target, now we need to see if autocmd is enabled. If so, we can just proceed to the XFR phase automatically.
                command_pos = 0;
                auto newPhase = (ctrl & S_PHASE_MASK);
                if((spifi_reg.cmlen & CML_ACOM_EN) && newPhase == S_PHASE_COMMAND)
                {
                    LOG("Select complete, autocmd enabled so moving on to XFR phase!\n");
                    scsi_bus->ctrl_w(scsi_refid, 0, S_ACK); // TODO: Deassert ACK - just trying this out
                    state = INIT_XFR;
                    xfr_phase = scsi_bus->ctrl_r() & S_PHASE_MASK;
                    step(false); // TODO: delay needed?
                }
                else if ((spifi_reg.cmlen & CML_AMSG_EN) && (newPhase == S_PHASE_MSG_OUT || newPhase == S_PHASE_MSG_IN))
                {
                    LOG("Select complete, automsg enabled so moving on to XFR phase!\n");
                    state = INIT_XFR;
                    dma_command = true;
                    dma_dir = DMA_OUT; // fake
                    xfr_phase = scsi_bus->ctrl_r() & S_PHASE_MASK;
                    step(false); // TODO: delay needed?
                }
                else
                {
                    function_bus_complete();
                }
            }
            else if(false) //c == CD_SELECT_ATN_STOP) // How to determine if we need to stop here?
            {
                function_bus_complete();
            }
            else
            {
                state = DISC_SEL_WAIT_REQ;
            }
            break;
        }

        case DISC_SEL_WAIT_REQ:
        {
            if(!(ctrl & S_REQ))
            {
                break;
            }
            if((ctrl & S_PHASE_MASK) != S_PHASE_COMMAND)
            {
                scsi_bus->ctrl_wait(scsi_refid, 0, S_REQ);
                function_bus_complete();
                break;
            }

            state = DISC_SEL_SEND_BYTE;
            send_cmd_byte();
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
            if(!(ctrl & S_REQ))
            {
                break;
            }

            if((ctrl & S_PHASE_MASK) != S_PHASE_MSG_IN)
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
            if((ctrl & (S_REQ|S_BSY)) == S_BSY)
            {
                break;
            }
            bus_complete();
            break;
        }

        case INIT_XFR:
        {
            LOG("INIT_XFR: %d\n", xfr_phase);
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
                        LOG("INIT_XFR: no data! Deferring until we have some.\n");
                        xfrDataSource = FIFO;
                        break;
                    }

                    // if it's the last message byte, deassert ATN before sending
                    if (xfr_phase == S_PHASE_MSG_OUT && ((!dma_command && m_even_fifo.size() == 1) || (dma_command && tcounter == 1))) // TODO: command equiv
                    {
                        scsi_bus->ctrl_w(scsi_refid, 0, S_ATN);
                    }

                    if(xfr_phase == S_PHASE_DATA_OUT)
                    {
                        LOG("Data out phase, using FIFO.\n");
                        xfrDataSource = FIFO;
                        send_byte();
                    }
                    else // send from cdb buffer on command or message
                    {
                        LOG("Message or command out phase, using cdb.\n");
                        xfrDataSource = COMMAND_BUFFER;
                        send_cmd_byte();
                    }
                    break;
                }

                case S_PHASE_DATA_IN:
                case S_PHASE_STATUS:
                case S_PHASE_MSG_IN:
                {
                    // can't receive if the fifo is full
                    if (m_even_fifo.size() == 8 && !(xfr_phase == S_PHASE_STATUS && spifi_reg.autostat)) // TODO: - no idea if status goes to the fifo or not
                    {
                        // check_drq(); // in case data should be transferred now
                        break;
                    }

                    // if it's the last message byte, ACK remains asserted, terminate with function_complete()
                    // However, if AUTOMSG is enabled, automatically accept the message by lowering ACK before continuing.
                    if((xfr_phase == S_PHASE_MSG_IN && (!dma_command || tcounter == 1)))
                    {
                        state = (spifi_reg.cmlen & CML_AMSG_EN) ? INIT_XFR_RECV_BYTE_ACK_AUTOMSG : INIT_XFR_RECV_BYTE_NACK;
                    }
                    else
                    {
                        state = INIT_XFR_RECV_BYTE_ACK;
                    }

                    xfrDataSource = FIFO; // TODO: redirect status to appropriate place. What to do with automsg?
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
            if(!(ctrl & S_REQ))
            {
                break;
            }

            // check for command complete
            if (xfrDataSource == FIFO && ((dma_command && transfer_count_zero() && (dma_dir == DMA_IN || m_even_fifo.empty()))))
            {
                LOG("DMA Data transfer complete\n");
                state = INIT_XFR_BUS_COMPLETE;

                auto newPhase = (ctrl & S_PHASE_MASK);
                if(newPhase == S_PHASE_STATUS && spifi_reg.autostat > 0)
                {
                    // TODO: ID enforcement for autostat
                    LOG("Autostat enabled, proceeding to status input automatically\n");
                    state = INIT_XFR;
                    xfr_phase = newPhase;

                    // Below this is a guess
                    dma_command = false;
                    dma_dir = DMA_NONE;
                    spifi_reg.autostat &= ~0x1; // TODO: should only be target ID of this transfer
                }
            }
            else if (xfrDataSource == FIFO && (!dma_command && (xfr_phase & S_INP) == 0 && m_even_fifo.empty()))
            {
                LOG("Non-DMA transfer out complete, FIFO drained!\n");
                state = INIT_XFR_BUS_COMPLETE;
            }
            else if (xfrDataSource == FIFO && (!dma_command && ((xfr_phase & S_INP) == S_INP) && m_even_fifo.size() == 1))
            {
                LOG("Non-DMA transfer in complete!\n");
                state = INIT_XFR_BUS_COMPLETE;
                auto newPhase = (ctrl & S_PHASE_MASK);
                if ((newPhase == S_PHASE_MSG_IN) && ((spifi_reg.cmlen & CML_AMSG_EN) > 0)) // Automsg enabled, do that instead
                {
                    LOG("AUTOMSG enabled, proceeding to message input!\n");
                    state = INIT_XFR;
                    xfr_phase = newPhase;
                    // TODO: anything else? set ICOND AMSGOFF or something?
                }
            }
            else if(xfrDataSource == COMMAND_BUFFER && (command_pos >= (spifi_reg.cmlen & CML_LENMASK))) // Done transferring message or command
            {
                LOG("Command transfer complete, new phase = %d\n", ctrl & S_PHASE_MASK);
                state = INIT_XFR_BUS_COMPLETE;
                // spifi_reg.icond |= ICOND_ACMDOFF; TODO: ???

                // If autodata is enabled for this target, then we don't need to notify the host, we just keep going with the transfer instead.
                // TODO: is this the right interpretation??
                // If the target gives us a non-data phase, though, we will notify the host. Is this the correct behavior??
                auto newPhase = (ctrl & S_PHASE_MASK);
                if ( (newPhase == S_PHASE_DATA_IN && (((spifi_reg.autodata & (ADATA_EN | ADATA_IN)) == (ADATA_EN | ADATA_IN))
                     || (newPhase == S_PHASE_DATA_OUT && ((spifi_reg.autodata & (ADATA_EN | ADATA_IN)) == ADATA_EN))))) // TODO: ID enforcement
                {
                    state = INIT_XFR;
                    xfr_phase = newPhase;
                    dma_command = true;
                    if (newPhase == S_PHASE_DATA_IN)
                    {
                        dma_dir = DMA_IN;
                    }
                    else
                    {
                        dma_dir = DMA_OUT;
                    }
                    step(false);
                }
                else if(newPhase == S_PHASE_STATUS && spifi_reg.autostat > 0)
                {
                    // TODO: ID enforcement for autostat
                    LOG("Autostat enabled, proceeding to status input automatically\n");
                    state = INIT_XFR;
                    xfr_phase = newPhase;

                    // Below this is a guess
                    dma_command = false;
                    dma_dir = DMA_NONE;
                    spifi_reg.autostat &= ~0x1; // TODO: should only be target ID of this transfer
                }
                else if ((newPhase == S_PHASE_MSG_IN) && ((spifi_reg.cmlen & CML_AMSG_EN) > 0))
                {
                    LOG("AUTOMSG enabled, proceeding to message input!\n");
                    state = INIT_XFR;
                    xfr_phase = newPhase;
                    // TODO: anything else? set ICOND AMSGOFF or something?
                }
            }
            else
            {
                // check for phase change
                auto newPhase = ctrl & S_PHASE_MASK;
                if (newPhase != xfr_phase)
                {
                    LOG("Phase changed to %d\n", newPhase);
                    command_pos = 0;

                    // If initiator autostatus is enabled, we can automatically do the status phase
                    // TODO: ID checking and actually transferring the status byte to the appropriate SPIFI register
                    if(newPhase == S_PHASE_STATUS && spifi_reg.autostat > 0)
                    {
                        LOG("Autostat enabled, proceeding to status input automatically\n");
                        state = INIT_XFR;
                        xfr_phase = newPhase;

                        // Below this is a guess
                        dma_command = false;
                        dma_dir = DMA_NONE;
                        spifi_reg.autostat &= ~0x1; // TODO: should only be target ID of this transfer
                    }
                    else if (newPhase == S_PHASE_COMMAND && (spifi_reg.cmlen & CML_ACOM_EN))
                    {
                        LOG("Autocmd enabled, proceeding to command automatically\n");
                        state = INIT_XFR;
                        xfr_phase = newPhase;

                        step(false); // TODO: delay needed?
                    }
                    else
                    {
                        LOG("Finished phase, marking bus complete\n");
                        state = INIT_XFR_BUS_COMPLETE;
                    }
                }
                else
                {
                    LOG("Continuing transfer\n");
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
            if (dma_command && !transfer_count_zero() && !m_even_fifo.empty())
            {
                LOG("Data left to transfer, not marking function complete.\n");
                break;
            }
            LOG("Clearing ACK and completing function because automsg is enabled!\n");
            scsi_bus->ctrl_w(scsi_refid, 0, S_ACK);
            function_complete();

            // Dump message info out of the fifo
            clear_fifo();
            step(false);
            break;
        }

        case INIT_XFR_FUNCTION_COMPLETE:
        {
            // wait for dma transfer to complete or fifo to drain
            if (dma_command && !transfer_count_zero() && !m_even_fifo.empty())
            {
                LOG("Data left to transfer, not marking function complete.\n");
                break;
            }

            function_complete();
            break;
        }

        case INIT_XFR_BUS_COMPLETE:
        {
            // wait for dma transfer to complete or fifo to drain
            if (dma_command && !transfer_count_zero() && !m_even_fifo.empty())
            {
                break;
            }

            bus_complete();
            break;
        }

        case INIT_XFR_SEND_PAD_WAIT_REQ:
        {
            if(!(ctrl & S_REQ))
            {
                break;
            }

            if((ctrl & S_PHASE_MASK) != xfr_phase)
            {
                command_pos = 0;
                bus_complete();
            }
            else
            {
                state = INIT_XFR_SEND_PAD;
                send_byte();
            }
            break;
        }

        case INIT_XFR_SEND_PAD:
        {
            decrement_tcounter();
            if(!transfer_count_zero())
            {
                state = INIT_XFR_SEND_PAD_WAIT_REQ;
                step(false);
            }
            else
            {
                function_complete();
            }
            break;
        }

        case INIT_XFR_RECV_PAD_WAIT_REQ:
        {
            if(!(ctrl & S_REQ))
            {
                break;
            }

            if((ctrl & S_PHASE_MASK) != xfr_phase)
            {
                command_pos = 0;
                bus_complete();
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
            decrement_tcounter();
            if(!transfer_count_zero())
            {
                state = INIT_XFR_RECV_PAD_WAIT_REQ;
                scsi_bus->ctrl_w(scsi_refid, 0, S_ACK);
                step(false);
            }
            else
            {
                function_complete();
            }
            break;
        }

        default:
        {
            LOG("step() unexpected state %d.%d\n", state & STATE_MASK, (state & SUB_MASK) >> SUB_SHIFT);
            exit(0);
        }
    }
}
