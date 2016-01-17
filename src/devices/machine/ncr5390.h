// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef NCR5390_H
#define NCR5390_H

#include "machine/nscsi_bus.h"

#define MCFG_NCR5390_IRQ_HANDLER(_devcb) \
	devcb = &ncr5390_device::set_irq_handler(*device, DEVCB_##_devcb);

#define MCFG_NCR5390_DRQ_HANDLER(_devcb) \
	devcb = &ncr5390_device::set_drq_handler(*device, DEVCB_##_devcb);

class ncr5390_device : public nscsi_device
{
public:
	ncr5390_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	template<class _Object> static devcb_base &set_irq_handler(device_t &device, _Object object) { return downcast<ncr5390_device &>(device).m_irq_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_drq_handler(device_t &device, _Object object) { return downcast<ncr5390_device &>(device).m_drq_handler.set_callback(object); }

	DECLARE_ADDRESS_MAP(map, 8);

	DECLARE_READ8_MEMBER(tcount_lo_r);
	DECLARE_WRITE8_MEMBER(tcount_lo_w);
	DECLARE_READ8_MEMBER(tcount_hi_r);
	DECLARE_WRITE8_MEMBER(tcount_hi_w);
	DECLARE_READ8_MEMBER(fifo_r);
	DECLARE_WRITE8_MEMBER(fifo_w);
	DECLARE_READ8_MEMBER(command_r);
	DECLARE_WRITE8_MEMBER(command_w);
	DECLARE_READ8_MEMBER(status_r);
	DECLARE_WRITE8_MEMBER(bus_id_w);
	DECLARE_READ8_MEMBER(istatus_r);
	DECLARE_WRITE8_MEMBER(timeout_w);
	DECLARE_READ8_MEMBER(seq_step_r);
	DECLARE_WRITE8_MEMBER(sync_period_w);
	DECLARE_READ8_MEMBER(fifo_flags_r);
	DECLARE_WRITE8_MEMBER(sync_offset_w);
	DECLARE_READ8_MEMBER(conf_r);
	DECLARE_WRITE8_MEMBER(conf_w);
	DECLARE_WRITE8_MEMBER(clock_w);

	virtual void scsi_ctrl_changed() override;

	UINT8 dma_r();
	void dma_w(UINT8 val);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	enum { MODE_D, MODE_T, MODE_I };
	enum { IDLE };

	enum {
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
		INIT_XFR_WAIT_REQ,
		INIT_CPT_RECV_BYTE_ACK,
		INIT_CPT_RECV_WAIT_REQ,
		INIT_CPT_RECV_BYTE_NACK
	};

	enum {
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

	enum {
		STATE_MASK = 0x00ff,
		SUB_SHIFT  = 8,
		SUB_MASK   = 0xff00
	};

	enum { BUS_BUSY, BUS_FREE_WAIT, BUS_FREE };

	enum {
		S_GROSS_ERROR     = 0x40,
		S_PARITY          = 0x20,
		S_TC0             = 0x10,
		S_TCC             = 0x08,

		I_SCSI_RESET      = 0x80,
		I_ILLEGAL         = 0x40,
		I_DISCONNECT      = 0x20,
		I_BUS             = 0x10,
		I_FUNCTION        = 0x08,
		I_RESELECTED      = 0x04,
		I_SELECT_ATN      = 0x02,
		I_SELECTED        = 0x01,

		CM_NOP             = 0x00,
		CM_FLUSH_FIFO      = 0x01,
		CM_RESET           = 0x02,
		CM_RESET_BUS       = 0x03,
		CD_RESELECT        = 0x40,
		CD_SELECT          = 0x41,
		CD_SELECT_ATN      = 0x42,
		CD_SELECT_ATN_STOP = 0x43,
		CD_ENABLE_SEL      = 0x44,
		CD_DISABLE_SEL     = 0x45,
		CT_SEND_MSG        = 0x20,
		CT_SEND_STATUS     = 0x21,
		CT_SEND_DATA       = 0x22,
		CT_DISCONNECT_SEQ  = 0x23,
		CT_TERMINATE       = 0x24,
		CT_COMPLETE        = 0x25,
		CT_DISCONNECT      = 0x27,
		CT_RECV_MSG        = 0x28,
		CT_RECV_CMD        = 0x29,
		CT_RECV_DATA       = 0x2a,
		CT_RECV_CMD_SEQ    = 0x2b,
		CI_XFER            = 0x10,
		CI_COMPLETE        = 0x11,
		CI_MSG_ACCEPT      = 0x12,
		CI_PAD             = 0x18,
		CI_SET_ATN         = 0x1a
	};

	enum { DMA_NONE, DMA_IN, DMA_OUT };

	emu_timer *tm;

	UINT8 command[2], config, status, istatus;
	UINT8 clock_conv, sync_offset, sync_period, bus_id, select_timeout, seq;
	UINT8 fifo[16];
	UINT16 tcount;
	int mode, fifo_pos, command_pos;
	int state, xfr_phase;
	int command_length;

	int dma_dir;

	bool irq, drq;

	void dma_set(int dir);
	void drq_set();
	void drq_clear();

	void start_command();
	void step(bool timeout);
	bool check_valid_command(UINT8 cmd);
	int derive_msg_size(UINT8 msg_id);
	void function_complete();
	void function_bus_complete();
	void bus_complete();

	void arbitrate();
	void command_pop_and_chain();
	void check_irq();

	void reset_soft();
	void reset_disconnect();

	UINT8 fifo_pop();
	void fifo_push(UINT8 val);
	void send_byte();
	void recv_byte();

	void delay(int cycles);
	void delay_cycles(int cycles);

	devcb_write_line m_irq_handler;
	devcb_write_line m_drq_handler;
};

extern const device_type NCR5390;

#endif
