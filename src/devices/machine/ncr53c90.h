// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_MACHINE_NCR53C90_H
#define MAME_MACHINE_NCR53C90_H

#pragma once

#include "machine/nscsi_bus.h"

class ncr53c90_device : public nscsi_device, public nscsi_slot_card_interface
{
public:
	ncr53c90_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	auto irq_handler_cb() { return m_irq_handler.bind(); }
	auto drq_handler_cb() { return m_drq_handler.bind(); }

	virtual void map(address_map &map) ATTR_COLD;

	uint8_t tcounter_lo_r();
	void tcount_lo_w(uint8_t data);
	uint8_t tcounter_hi_r();
	void tcount_hi_w(uint8_t data);
	uint8_t fifo_r();
	void fifo_w(uint8_t data);
	uint8_t command_r();
	void command_w(uint8_t data);
	virtual uint8_t status_r();
	void bus_id_w(uint8_t data);
	uint8_t istatus_r();
	void timeout_w(uint8_t data);
	uint8_t seq_step_r();
	void sync_period_w(uint8_t data);
	uint8_t fifo_flags_r();
	void sync_offset_w(uint8_t data);
	uint8_t conf_r();
	void conf_w(uint8_t data);
	void test_w(uint8_t data);
	void clock_w(uint8_t data);

	virtual uint8_t read(offs_t offset);
	virtual void write(offs_t offset, uint8_t data);

	virtual void scsi_ctrl_changed() override;

	uint8_t dma_r();
	void dma_w(uint8_t val);

protected:
	ncr53c90_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(update_tick);

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
	uint32_t tcount;
	uint32_t tcounter, tcounter_mask;
	int mode, fifo_pos, command_pos;
	int state, xfr_phase;

	int dma_dir;

	bool irq, drq;
	bool dma_command;
	bool test_mode;
	int stepping;

	void dma_set(int dir);
	virtual void check_drq();

	void start_command();
	void step(bool timeout);
	virtual bool check_valid_command(uint8_t cmd);
	void function_complete();
	void function_bus_complete();
	void bus_complete();

	void arbitrate();
	void command_pop_and_chain();
	void check_irq();

	void reset_disconnect();

	uint8_t fifo_pop();
	void fifo_push(uint8_t val);
	void send_byte();
	void recv_byte();

	void delay(int cycles);
	void delay_cycles(int cycles);

	void decrement_tcounter(int count = 1);

	devcb_write_line m_irq_handler;
	devcb_write_line m_drq_handler;
};

class ncr53c90a_device : public ncr53c90_device
{
public:
	ncr53c90a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void map(address_map &map) override ATTR_COLD;

	virtual uint8_t status_r() override;

	uint8_t conf2_r() { return config2; }
	virtual void conf2_w(uint8_t data) { config2 = data; }

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;

protected:
	ncr53c90a_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual bool check_valid_command(uint8_t cmd) override;

	// 53c90a uses a previously reserved bit as an interrupt flag
	enum {
		S_INTERRUPT = 0x80,
	};

	enum conf2_mask : u8
	{
		PGDP  = 0x01, // pass through/generate data parity
		PGRP  = 0x02, // pass through/generate register parity
		ACDPE = 0x04, // abort on command/data parity error
		S2FE  = 0x08, // scsi-2 features enable
		TSDR  = 0x10, // tri-state dma request
		SBO   = 0x20, // select byte order
		LSP   = 0x40, // latch scsi phase
		DAE   = 0x80, // data alignment enable
	};

protected:
	u8 config2;
};

class ncr53c94_device : public ncr53c90a_device
{
public:
	ncr53c94_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	enum busmd_t : u8
	{
		BUSMD_0 = 0, // single bus: 8-bit host, 8 bit dma
		BUSMD_1 = 1, // single bus: 8 bit host, 16 bit dma
		BUSMD_2 = 2, // two buses: 8 bit multiplexed host, 16 bit dma
		BUSMD_3 = 3, // two buses: 8 bit host, 16 bit dma
	};
	void set_busmd(busmd_t const busmd) { m_busmd = busmd; }

	virtual void map(address_map &map) override ATTR_COLD;

	uint8_t conf3_r() { return config3; }
	void conf3_w(uint8_t data) { config3 = data; }

	void fifo_align_w(uint8_t data) { fifo_align = data; }

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;

	u16 dma16_r();
	void dma16_w(u16 data);

	u16 dma16_swap_r() { return swapendian_int16(dma16_r()); }
	void dma16_swap_w(u16 data) { return dma16_w(swapendian_int16(data)); }

protected:
	ncr53c94_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	enum conf3_mask : u8
	{
		BS8  = 0x01, // burst size 8
		MDM  = 0x02, // modify dma mode
		LBTM = 0x04, // last byte transfer mode
	};

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void check_drq() override;

private:
	u8 config3;
	u8 fifo_align;
	busmd_t m_busmd;
};

class ncr53c96_device : public ncr53c94_device
{
public:
	ncr53c96_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class ncr53cf94_device : public ncr53c94_device
{
public:
	ncr53cf94_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void map(address_map &map) override ATTR_COLD;

	virtual void conf2_w(uint8_t data) override;

	uint8_t conf4_r() { return config4; }
	void conf4_w(uint8_t data) { config4 = data; }

	uint8_t tcounter_hi2_r();
	void tcount_hi2_w(uint8_t data);

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;

protected:
	ncr53cf94_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	u8 config4;
	u8 family_id;
	u8 revision_level;
};

class ncr53cf96_device : public ncr53cf94_device
{
public:
	ncr53cf96_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(NCR53C90, ncr53c90_device)
DECLARE_DEVICE_TYPE(NCR53C90A, ncr53c90a_device)
DECLARE_DEVICE_TYPE(NCR53C94, ncr53c94_device)
DECLARE_DEVICE_TYPE(NCR53C96, ncr53c96_device)
DECLARE_DEVICE_TYPE(NCR53CF94, ncr53cf94_device)
DECLARE_DEVICE_TYPE(NCR53CF96, ncr53cf96_device)

#endif // MAME_MACHINE_NCR53C90_H
