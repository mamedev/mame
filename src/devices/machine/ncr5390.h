// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_MACHINE_NCR5390_H
#define MAME_MACHINE_NCR5390_H

#pragma once

#include "machine/nscsi_bus.h"

#define MCFG_NCR5390_IRQ_HANDLER(_devcb) \
	downcast<ncr5390_device &>(*device).set_irq_handler(DEVCB_##_devcb);

#define MCFG_NCR5390_DRQ_HANDLER(_devcb) \
	downcast<ncr5390_device &>(*device).set_drq_handler(DEVCB_##_devcb);

class ncr5390_device : public nscsi_device
{
public:
	ncr5390_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	template <class Object> devcb_base &set_irq_handler(Object &&cb) { return m_irq_handler.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_drq_handler(Object &&cb) { return m_drq_handler.set_callback(std::forward<Object>(cb)); }

	auto irq_handler_cb() { return m_irq_handler.bind(); }
	auto drq_handler_cb() { return m_drq_handler.bind(); }

	virtual void map(address_map &map);

	DECLARE_READ8_MEMBER(tcounter_lo_r);
	DECLARE_WRITE8_MEMBER(tcount_lo_w);
	DECLARE_READ8_MEMBER(tcounter_hi_r);
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
	DECLARE_WRITE8_MEMBER(test_w);
	DECLARE_WRITE8_MEMBER(clock_w);

	virtual DECLARE_READ8_MEMBER(read);
	virtual DECLARE_WRITE8_MEMBER(write);

	virtual void scsi_ctrl_changed() override;

	uint8_t dma_r();
	void dma_w(uint8_t val);

	// memory mapped wrappers for dma read/write
	DECLARE_READ8_MEMBER(mdma_r) { return dma_r(); }
	DECLARE_WRITE8_MEMBER(mdma_w) { dma_w(data); }

protected:
	ncr5390_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

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
		CD_SELECT_ATN3     = 0x46, // 53c90a
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
		CT_ABORT_DMA       = 0x04, // 53c90a
		CI_XFER            = 0x10,
		CI_COMPLETE        = 0x11,
		CI_MSG_ACCEPT      = 0x12,
		CI_PAD             = 0x18,
		CI_SET_ATN         = 0x1a,
		CI_RESET_ATN       = 0x1b, // 53c90a
	};

	enum { DMA_NONE, DMA_IN, DMA_OUT };

	emu_timer *tm;

	uint8_t command[2], config, status, istatus;
	uint8_t clock_conv, sync_offset, sync_period, bus_id, select_timeout, seq;
	uint8_t fifo[16];
	uint16_t tcount;
	uint16_t tcounter;
	int mode, fifo_pos, command_pos;
	int state, xfr_phase;
	int command_length;

	int dma_dir;

	bool irq, drq;
	bool dma_command;
	bool test_mode;

	void dma_set(int dir);
	void drq_set();
	void drq_clear();

	void start_command();
	void step(bool timeout);
	virtual bool check_valid_command(uint8_t cmd);
	int derive_msg_size(uint8_t msg_id);
	void function_complete();
	void function_bus_complete();
	void bus_complete();

	void arbitrate();
	void command_pop_and_chain();
	void check_irq();

protected:
	void reset_soft();

private:
	void reset_disconnect();

	uint8_t fifo_pop();
	void fifo_push(uint8_t val);
	void send_byte();
	void recv_byte();

	void delay(int cycles);
	void delay_cycles(int cycles);

	void decrement_tcounter();

	devcb_write_line m_irq_handler;
	devcb_write_line m_drq_handler;
};

class ncr53c90a_device : public ncr5390_device
{
public:
	ncr53c90a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void map(address_map &map) override;

	DECLARE_READ8_MEMBER(status_r);

	DECLARE_READ8_MEMBER(conf2_r) { return config2; };
	DECLARE_WRITE8_MEMBER(conf2_w) { config2 = data; };

	DECLARE_READ8_MEMBER(read) override;
	DECLARE_WRITE8_MEMBER(write) override;

protected:
	ncr53c90a_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	void reset_soft();

	virtual bool check_valid_command(uint8_t cmd) override;

	// 53c90a uses a previously reserved bit as an interrupt flag
	enum {
		S_INTERRUPT = 0x80,
	};

private:
	u8 config2;
};

class ncr53c94_device : public ncr53c90a_device
{
public:
	ncr53c94_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void map(address_map &map) override;

	DECLARE_READ8_MEMBER(conf3_r) { return config3; };
	DECLARE_WRITE8_MEMBER(conf3_w) { config3 = data; };
	DECLARE_WRITE8_MEMBER(fifo_align_w) { fifo_align = data; };

	DECLARE_READ8_MEMBER(read) override;
	DECLARE_WRITE8_MEMBER(write) override;

protected:
	virtual void device_start() override;
	void reset_soft();

private:
	u8 config3;
	u8 fifo_align;
};

DECLARE_DEVICE_TYPE(NCR5390, ncr5390_device)
DECLARE_DEVICE_TYPE(NCR53C90A, ncr53c90a_device)
DECLARE_DEVICE_TYPE(NCR53C94, ncr53c94_device)

#endif // MAME_MACHINE_NCR5390_H
