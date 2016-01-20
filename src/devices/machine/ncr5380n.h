// license:BSD-3-Clause
// copyright-holders:R. Belmont, Olivier Galibert
/*********************************************************************

    ncr5380n.c

    Implementation of the NCR 5380

*********************************************************************/

#ifndef NCR5380N_H
#define NCR5380N_H

#include "machine/nscsi_bus.h"

#define MCFG_NCR5380N_IRQ_HANDLER(_devcb) \
	devcb = &ncr5380n_device::set_irq_handler(*device, DEVCB_##_devcb);

#define MCFG_NCR5380N_DRQ_HANDLER(_devcb) \
	devcb = &ncr5380n_device::set_drq_handler(*device, DEVCB_##_devcb);

class ncr5380n_device : public nscsi_device
{
public:
	ncr5380n_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	template<class _Object> static devcb_base &set_irq_handler(device_t &device, _Object object) { return downcast<ncr5380n_device &>(device).m_irq_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_drq_handler(device_t &device, _Object object) { return downcast<ncr5380n_device &>(device).m_drq_handler.set_callback(object); }

	DECLARE_ADDRESS_MAP(map, 8);

	DECLARE_READ8_MEMBER(scsidata_r);
	DECLARE_WRITE8_MEMBER(outdata_w);
	DECLARE_READ8_MEMBER(icmd_r);
	DECLARE_WRITE8_MEMBER(icmd_w);
	DECLARE_READ8_MEMBER(mode_r);
	DECLARE_WRITE8_MEMBER(mode_w);
	DECLARE_READ8_MEMBER(command_r);
	DECLARE_WRITE8_MEMBER(command_w);
	DECLARE_READ8_MEMBER(status_r);
	DECLARE_WRITE8_MEMBER(selenable_w);
	DECLARE_READ8_MEMBER(busandstatus_r);
	DECLARE_WRITE8_MEMBER(startdmasend_w);
	DECLARE_READ8_MEMBER(indata_r);
	DECLARE_WRITE8_MEMBER(startdmatargetrx_w);
	DECLARE_READ8_MEMBER(resetparityirq_r);
	DECLARE_WRITE8_MEMBER(startdmainitrx_w);

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

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

	emu_timer *tm;

	UINT8 status, istatus, m_mode, m_outdata, m_busstatus, m_dmalatch;
	UINT8 m_icommand, m_tcommand;
	UINT8 clock_conv, sync_offset, sync_period, bus_id, select_timeout, seq;
	UINT16 tcount;
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

	devcb_write_line m_irq_handler;
	devcb_write_line m_drq_handler;
};

extern const device_type NCR5380N;

#endif
