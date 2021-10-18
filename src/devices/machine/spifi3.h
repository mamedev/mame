// license:BSD-3-Clause
// copyright-holders:Brice Onken,Tsubai Masanari,Olivier Galibert
// thanks-to:Patrick Mackinlay

/*
 * HP 1TV3-0302 SPIFI3-SE SCSI controller
 *
 * Datasheets for this seem to be impossible to find - the only avaliable implementation to reference that I have
 * found is the Sony NEWS APBus NetBSD driver. Hopefully a datasheet will turn up eventually.
 * Based on internet research, it seems some HP PA-RISC systems also used the SPIFI3, including the E55.
 *
 * Because this driver was developed to work with NetBSD, NEWS-OS, and the NWS-5000 monitor ROM, only
 * the features and flows that Sony used are implemented. Emulating non-Sony designs using this chip will likely 
 * require similar RE work to determine the exact SPIFI features used and support for them added into this driver.  
 * In its current state, this driver is unlikely to work out of the box with any other machines.
 *
 * Register definitions were derived from the NetBSD source code, copyright (c) 2000 Tsubai Masanari.
 * SCSI state machine code was derived from the MAME NCR5390 driver, copyright (c) Olivier Galibert
 *
 * References:
 * - https://github.com/NetBSD/src/blob/trunk/sys/arch/newsmips/apbus/spifireg.h
 * - https://github.com/NetBSD/src/blob/trunk/sys/arch/newsmips/apbus/spifi.c
 * - https://github.com/mamedev/mame/blob/master/src/devices/machine/ncr5390.cpp
 */

#ifndef MAME_MACHINE_SPIFI3_H
#define MAME_MACHINE_SPIFI3_H

#pragma once

#include <queue>
#include "machine/nscsi_bus.h"

class spifi3_device
    : public nscsi_device,
      public nscsi_slot_card_interface
{
public:
    spifi3_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);
    void map(address_map &map);

    auto irq_handler_cb() { return m_irq_handler.bind(); }
    auto drq_handler_cb() { return m_drq_handler.bind(); }

    uint8_t dma_r();
    void dma_w(uint8_t val);

protected:
    virtual void device_start() override;
    virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void scsi_ctrl_changed() override;

private:
    enum scsi_mode
    {
        MODE_D, // Disconnected
        MODE_T, // Target
        MODE_I	// Initiator
    };

    enum scsi_data_source
    {
        COMMAND_BUFFER,
        FIFO
    };

    // State tracking variables
    scsi_mode mode;				  // Target or initiatior?
    scsi_data_source xfrDataSource; // where are we transferring data from?
    int state;					  // SCSI controller state
    int xfr_phase;
    int command_pos;
    int dma_dir;
    bool irq = false; // IRQ pin state
    bool drq = false; // DRQ pin state
    bool dma_command;
    uint32_t tcounter;
    uint8_t sync_period;
    uint8_t clock_conv = 2; // TODO: appropriate value for SPIFI
    emu_timer *tm;
    int bus_id;

    // I/O ports
    devcb_write_line m_irq_handler;
    devcb_write_line m_drq_handler;

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

    enum dma_direction
    {
        DMA_NONE,
        DMA_IN,
        DMA_OUT
    };

    // State-related functions
    void step(bool timeout);
    void check_irq();
    void check_drq();
    void reset_disconnect();
    void send_byte();
    void send_cmd_byte();
    void recv_byte();
    void function_complete();
    void function_bus_complete();
    void bus_complete();
    void dma_set(int dir);
    void decrement_tcounter(int count = 1);
    bool transfer_count_zero();
    void delay(int cycles);
    void delay_cycles(int cycles);
    void arbitrate();
    void clear_fifo();

    // AUXCTRL constants and functions
    const uint32_t AUXCTRL_DMAEDGE = 0x04;
    const uint32_t AUXCTRL_SETRST = 0x20;
    const uint32_t AUXCTRL_CRST = 0x40;
    const uint32_t AUXCTRL_SRST = 0x80;
    uint32_t auxctrl_r();
    void auxctrl_w(uint32_t data);

    // FIFOCTRL constants and functions
    // Based on the existence of CLREVEN/ODD, the fact that NetBSD only uses EVEN, and the max is 8
    // even though this is a 4 bit value, it seems likely that there are actually two FIFOs,
    // one in the even slots, and one in the odd slots
    const uint32_t FIFOC_FSLOT = 0x0f; // Free slots in FIFO - max 8. Free slots = 8 - (FIFOCTRL & FIFOC_FSLOT)
    const uint32_t FIFOC_SSTKACT = 0x10;
    const uint32_t FIFOC_RQOVRN = 0x20;
    const uint32_t FIFOC_CLREVEN = 0x00;
    const uint32_t FIFOC_CLRODD = 0x40;
    const uint32_t FIFOC_FLUSH = 0x80;
    const uint32_t FIFOC_LOAD = 0xc0;
    uint32_t fifoctrl_r();
    void fifoctrl_w(uint32_t data);
    std::queue<uint32_t> m_even_fifo; // 0, 2, 4, 6, 8, 10, 12, 14
    std::queue<uint32_t> m_odd_fifo;  // 1, 3, 5, 7, 9, 11, 13, 15
    void clear_queue(std::queue<uint32_t> queue)
    {
        while(!queue.empty())
        {
            queue.pop();
        }
    }

    // spstat
    const uint32_t SPS_IDLE = 0x00;
    const uint32_t SPS_SEL = 0x01;
    const uint32_t SPS_ARB = 0x02;
    const uint32_t SPS_RESEL = 0x03;
    const uint32_t SPS_MSGOUT = 0x04;
    const uint32_t SPS_COMMAND = 0x05;
    const uint32_t SPS_DISCON = 0x06;
    const uint32_t SPS_NXIN = 0x07;
    const uint32_t SPS_INTR = 0x08;
    const uint32_t SPS_NXOUT = 0x09;
    const uint32_t SPS_CCOMP = 0x0a;
    const uint32_t SPS_SVPTR = 0x0b;
    const uint32_t SPS_STATUS = 0x0c;
    const uint32_t SPS_MSGIN = 0x0d;
    const uint32_t SPS_DATAOUT = 0x0e;
    const uint32_t SPS_DATAIN = 0x0f;
    uint32_t spstat_r();

    // prstat
    const uint32_t PRS_IO = 0x08;
    const uint32_t PRS_CD = 0x10;
    const uint32_t PRS_MSG = 0x20;
    const uint32_t PRS_ATN = 0x40;
    const uint32_t PRS_Z = 0x80;
    const uint32_t PRS_PHASE = (PRS_MSG | PRS_CD | PRS_IO);
    uint32_t prstat_r();

    // Interrupt status register
    const uint32_t INTR_BSRQ = 0x01;
    const uint32_t INTR_COMRECV = 0x02;
    const uint32_t INTR_PERR = 0x04;
    const uint32_t INTR_TIMEO = 0x08;
    const uint32_t INTR_DERR = 0x10;
    const uint32_t INTR_TGSEL = 0x20;
    const uint32_t INTR_DISCON = 0x40;
    const uint32_t INTR_FCOMP = 0x80;

    // Interrupt condition register
    const uint32_t ICOND_ADATAOFF = 0x02;
    const uint32_t ICOND_AMSGOFF = 0x06;
    const uint32_t ICOND_ACMDOFF = 0x0a;
    const uint32_t ICOND_ASTATOFF = 0x0e;
    const uint32_t ICOND_SVPTEXP = 0x10;
    const uint32_t ICOND_ADATAMIS = 0x20;
    const uint32_t ICOND_CNTZERO = 0x40;
    const uint32_t ICOND_UXPHASEZ = 0x80;
    const uint32_t ICOND_UXPHASENZ = 0x81;
    const uint32_t ICOND_NXTREQ = 0xa0;
    const uint32_t ICOND_UKMSGZ = 0xc0;
    const uint32_t ICOND_UKMSGNZ = 0xc1;
    const uint32_t ICOND_UBF = 0xe0; /* Unexpected bus free */

    // Config register
    const uint32_t CONFIG_INITIATOR_ID = 0x7;
    const uint32_t CONFIG_PGENEN = 0x08;
    const uint32_t CONFIG_PCHKEN = 0x10;
    const uint32_t CONFIG_WORDEN = 0x20;
    const uint32_t CONFIG_AUTOID = 0x40;
    const uint32_t CONFIG_DMABURST = 0x80;

    // Select register
    const uint32_t SEL_SETATN = 0x02;	// Set ATN??? NetBSD doesn't use this...
    const uint32_t SEL_IRESELEN = 0x04; // Enable reselection phase
    const uint32_t SEL_ISTART = 0x08;	// Start selection
    const uint32_t SEL_WATN = 0x80;		// Select with ATN
    const uint32_t SEL_TARGET = 0x70;
    void select_w(uint32_t data);
    int get_target_id()
    {
        return (spifi_reg.select & SEL_TARGET) >> 4;
    }

    // Autodata register
    const uint32_t ADATA_IN = 0x40;
    const uint32_t ADATA_EN = 0x80;
    const uint32_t ADATA_TARGET_ID = 0x07;
    void autodata_w(uint32_t data);
    bool autodata_active(int target_id)
    {
        return (spifi_reg.autodata & ADATA_EN) && ((spifi_reg.autodata & ADATA_TARGET_ID) == target_id);
    }
    bool autodata_in(int target_id)
    {
        return autodata_active(target_id) && (spifi_reg.autodata & ADATA_IN);
    }
    bool autodata_out(int target_id)
    {
        return autodata_active(target_id) && !(spifi_reg.autodata & ADATA_IN);
    }

    // Autostat register
    void autostat_done(int target_id)
    {
        spifi_reg.autostat &= ~(1 << target_id);
    }
    bool autostat_active(int target_id)
    {
        return spifi_reg.autostat & (1 << target_id);
    }
    void start_autostat(int target_id);

    // prcmd
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
    uint32_t prcmd_r();
    void prcmd_w(uint32_t data);

    // Command buffer constants and functions
    uint8_t cmd_buf_r(offs_t offset);
    void cmd_buf_w(offs_t offset, uint8_t data);

    // cmlen register
    const uint32_t CML_LENMASK = 0x0f;
    const uint32_t CML_AMSG_EN = 0x40;
    const uint32_t CML_ACOM_EN = 0x80;
    bool automsg_active()
    {
        return spifi_reg.cmlen & CML_AMSG_EN;
    }
    void start_automsg(int msg_phase);
    bool autocmd_active()
    {
        return spifi_reg.cmlen & CML_ACOM_EN;
    }
    void start_autocmd();

    // init_status
    uint32_t init_status_r();
    const uint32_t INIT_STATUS_ACK = 0x40;

    struct spifi_cmd_entry
    {
        // NetBSD has these mapped as uint32_t to align the accesses and such
        // in reality, these are all 8-bit values that are mapped, in typical NWS-5000 series
        // fashion, to be 32-bit word aligned.
        // the same probably applies to the register file.
        uint8_t cdb[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        uint8_t quecode = 0;
        uint8_t quetag = 0;
        uint8_t idmsg = 0;
        uint8_t status = 0;
    };

    struct register_file
    {
        /*00*/ uint32_t spstat = 0;
        uint32_t cmlen = 0;
        uint32_t cmdpage = 0;
        // uint32_t count_hi = 0;

        /*10*/ // uint32_t count_mid = 0;
        // uint32_t count_low = 0;
        uint32_t svptr_hi = 0;
        uint32_t svptr_mid = 0;

        /*20*/ uint32_t svptr_low = 0;
        uint32_t intr = 0;
        uint32_t imask = 0;
        uint32_t prctrl = 0;

        /*30*/ uint32_t prstat = 0;
        uint32_t init_status = 0;
        uint32_t fifoctrl = 0;
        uint32_t fifodata = 0;

        /*40*/ uint32_t config = 0;
        uint32_t data_xfer = 0;
        uint32_t autocmd = 0;
        uint32_t autostat = 0;

        /*50*/ uint32_t resel = 0;
        uint32_t select = 0;
        uint32_t prcmd = 0;
        uint32_t auxctrl = 0;

        /*60*/ uint32_t autodata = 0;
        uint32_t loopctrl = 0;
        uint32_t loopdata = 0;
        uint32_t identify = 0;

        /*70*/ uint32_t complete = 0;
        uint32_t scsi_status = 0x1; // MROM reads this to check if the SPIFI is alive at system boot, so the WO description from NetBSD might be wrong.
        uint32_t data = 0;
        uint32_t icond = 0;

        /*80*/ uint32_t fastwide = 0;
        uint32_t exctrl = 0;
        uint32_t exstat = 0;
        uint32_t test = 0;

        /*90*/ uint32_t quematch = 0;
        uint32_t quecode = 0;
        uint32_t quetag = 0;
        uint32_t quepage = 0;

        // uint32_t image[88]; // mirror of the previous values
        spifi_cmd_entry cmbuf[8];
    } spifi_reg;
};

DECLARE_DEVICE_TYPE(SPIFI3, spifi3_device)

#endif // MAME_MACHINE_SPIFI3_H
