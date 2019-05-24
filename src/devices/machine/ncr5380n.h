// license:BSD-3-Clause
// copyright-holders:R. Belmont, Olivier Galibert
/*********************************************************************

    ncr5380n.c

    Implementation of the NCR 5380

*********************************************************************/

#ifndef MAME_MACHINE_NCR5380N_H
#define MAME_MACHINE_NCR5380N_H

#pragma once

#include "machine/nscsi_bus.h"


class ncr5380n_device : public nscsi_device
{
public:
	ncr5380n_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	auto irq_handler() { return m_irq_handler.bind(); }
	auto drq_handler() { return m_drq_handler.bind(); }

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	uint8_t dma_r();
	void dma_w(uint8_t val);

protected:
	ncr5380n_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	virtual void scsi_ctrl_changed() override;

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
		ST_RST            = 0x80,
		ST_BSY            = 0x40,
		ST_REQ            = 0x20,
		ST_MSG            = 0x10,
		ST_CD             = 0x08,
		ST_IO             = 0x04,
		ST_SEL            = 0x02,
		ST_DBP            = 0x01,

		BAS_ENDOFDMA      = 0x80,
		BAS_DMAREQUEST    = 0x40,
		BAS_PARITYERROR   = 0x20,
		BAS_IRQACTIVE     = 0x10,
		BAS_PHASEMATCH    = 0x08,
		BAS_BUSYERROR     = 0x04,
		BAS_ATN           = 0x02,
		BAS_ACK           = 0x01,

		IC_RST            = 0x80,
		IC_ARBITRATION    = 0x40,
		IC_ARBLOST        = 0x20,
		IC_ACK            = 0x10,
		IC_BSY            = 0x08,
		IC_SEL            = 0x04,
		IC_ATN            = 0x02,
		IC_DBUS           = 0x01,
		IC_PHASEMASK      = 0x9e,
		IC_WRITEMASK      = 0x9f,

		MODE_BLOCKDMA     = 0x80,
		MODE_TARGET       = 0x40,
		MODE_PARITYCHK    = 0x20,
		MODE_PARITYIRQ    = 0x10,
		MODE_EOPIRQ       = 0x08,
		MODE_BSYIRQ       = 0x04,
		MODE_DMA          = 0x02,
		MODE_ARBITRATE    = 0x01
	};

	enum { DMA_NONE, DMA_IN, DMA_OUT };

	uint32_t m_fake_clock;

	emu_timer *tm;

	uint8_t status, istatus, m_mode, m_outdata, m_busstatus, m_dmalatch;
	uint8_t m_icommand, m_tcommand;
	uint8_t clock_conv, sync_offset, sync_period, bus_id, select_timeout, seq;
	uint16_t tcount;
	int mode;
	int state/*, xfr_phase*/;

	bool irq, drq;

	void drq_set();
	void drq_clear();

	void step(bool timeout);
	void function_complete();
	void function_bus_complete();
	void bus_complete();

	void arbitrate();
	void check_irq();

	void reset_soft();
	void reset_disconnect();

	void send_byte();
	void recv_byte();

	void delay(int cycles);
	void delay_cycles(int cycles);

	void map(address_map &map);

	uint8_t scsidata_r();
	void outdata_w(uint8_t data);
	uint8_t icmd_r();
	void icmd_w(uint8_t data);
	uint8_t mode_r();
	void mode_w(uint8_t data);
	uint8_t command_r();
	void command_w(uint8_t data);
	uint8_t status_r();
	void selenable_w(uint8_t data);
	uint8_t busandstatus_r();
	void startdmasend_w(uint8_t data);
	uint8_t indata_r();
	void startdmatargetrx_w(uint8_t data);
	uint8_t resetparityirq_r();
	void startdmainitrx_w(uint8_t data);

	devcb_write_line m_irq_handler;
	devcb_write_line m_drq_handler;
};

class ncr53c80_device : public ncr5380n_device
{
public:
	ncr53c80_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(NCR5380N, ncr5380n_device)
DECLARE_DEVICE_TYPE(NCR53C80, ncr53c80_device)

#endif // MAME_MACHINE_NCR5380N_H
