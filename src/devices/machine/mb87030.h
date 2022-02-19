// license:BSD-3-Clause
// copyright-holders:Sven Schnelle
#ifndef MAME_MACHINE_MB87030_H
#define MAME_MACHINE_MB87030_H

#pragma once

#include "machine/nscsi_bus.h"
#include <queue>

class mb87030_device : public nscsi_device, public nscsi_slot_card_interface
{
public:
	mb87030_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void map(address_map &map);

	auto out_irq_callback() { return m_irq_handler.bind(); }
	auto out_dreq_callback() { return m_dreq_handler.bind(); }

	uint8_t bdid_r();
	uint8_t sctl_r();
	uint8_t scmd_r();
	uint8_t tmod_r();
	uint8_t ints_r();
	uint8_t psns_r();
	uint8_t ssts_r();
	uint8_t serr_r();
	uint8_t pctl_r();
	uint8_t mbc_r();
	uint8_t dreg_r();
	uint8_t temp_r();
	uint8_t tch_r();
	uint8_t tcm_r();
	uint8_t tcl_r();
	uint8_t exbf_r();

	void bdid_w(uint8_t data);
	void sctl_w(uint8_t data);
	void scmd_w(uint8_t data);
	void tmod_w(uint8_t data);
	void ints_w(uint8_t data);
	void sdgc_w(uint8_t data);
	void pctl_w(uint8_t data);
	void dreg_w(uint8_t data);
	void temp_w(uint8_t data);
	void tch_w(uint8_t data);
	void tcm_w(uint8_t data);
	void tcl_w(uint8_t data);
	void exbf_w(uint8_t data);

	void reset_w(int state);
	virtual void scsi_ctrl_changed() override;

	uint8_t dma_r();
	void dma_w(uint8_t val);

	void ctrl_write(uint32_t value, uint32_t mask) { scsi_bus->ctrl_w(scsi_refid, value, mask); scsi_ctrl_changed(); }
	uint32_t data_read() { return scsi_bus->data_r(); }
protected:
	mb87030_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;
private:

	constexpr static uint8_t SCTL_INT_ENABLE = 0x01;
	constexpr static uint8_t SCTL_RESELECT_ENABLE = 0x02;
	constexpr static uint8_t SCTL_SELECT_ENABLE = 0x04;
	constexpr static uint8_t SCTL_PARITY_ENABLE = 0x08;
	constexpr static uint8_t SCTL_ARBITRATION_ENABLE = 0x10;
	constexpr static uint8_t SCTL_DIAG_MODE = 0x20;
	constexpr static uint8_t SCTL_CONTROL_RESET = 0x40;
	constexpr static uint8_t SCTL_RESET_AND_DISABLE = 0x80;

	constexpr static uint8_t SCMD_TERM_MODE = 0x01;
	constexpr static uint8_t SCMD_PRG_XFER = 0x02;
	constexpr static uint8_t SCMD_INTERCEPT_XFER = 0x04;
	constexpr static uint8_t SCMD_RST_OUT = 0x08;
	constexpr static uint8_t SCMD_CMD_BUS_RELEASE = 0x00;
	constexpr static uint8_t SCMD_CMD_SELECT = 0x20;
	constexpr static uint8_t SCMD_CMD_RESET_ATN = 0x40;
	constexpr static uint8_t SCMD_CMD_SET_ATN = 0x60;
	constexpr static uint8_t SCMD_CMD_TRANSFER = 0x80;
	constexpr static uint8_t SCMD_CMD_TRANSFER_PAUSE = 0xa0;
	constexpr static uint8_t SCMD_CMD_RESET_ACK_REQ = 0xc0;
	constexpr static uint8_t SCMD_CMD_SET_ACK_REQ = 0xe0;
	constexpr static uint8_t SCMD_CMD_MASK = 0xe0;

	constexpr static uint8_t TMOD_MIN_TRANSFER_PERIOD_SHIFT = 0x02;
	constexpr static uint8_t TMOD_MIN_TRANSFER_PERIOD_MASK = 0x0c;
	constexpr static uint8_t TMOD_MAX_TRANSFER_OFFSET_SHIFT = 0x04;
	constexpr static uint8_t TMOD_MIN_TRANSFER_OFFSET_MASK = 0x70;
	constexpr static uint8_t TMOD_SYNC_XFER = 0x80;

	constexpr static uint8_t PCTL_PHASE_SHIFT = 0x00;
	constexpr static uint8_t PCTL_PHASE_MASK = 0x07;
	constexpr static uint8_t PCTL_BUS_FREE_IE = 0x80;

	constexpr static uint8_t PSNS_IO = 0x01;
	constexpr static uint8_t PSNS_CD = 0x02;
	constexpr static uint8_t PSNS_MSG = 0x04;
	constexpr static uint8_t PSNS_BSY = 0x08;
	constexpr static uint8_t PSNS_SEL = 0x10;
	constexpr static uint8_t PSNS_ATN = 0x20;
	constexpr static uint8_t PSNS_ACK = 0x40;
	constexpr static uint8_t PSNS_REQ = 0x80;

	constexpr static uint8_t INTS_RESET_CONDITION = 0x01;
	constexpr static uint8_t INTS_SPC_HARD_ERR = 0x02;
	constexpr static uint8_t INTS_SPC_TIMEOUT = 0x04;
	constexpr static uint8_t INTS_SERVICE_REQUIRED = 0x08;
	constexpr static uint8_t INTS_COMMAND_COMPLETE = 0x10;
	constexpr static uint8_t INTS_DISCONNECTED = 0x20;
	constexpr static uint8_t INTS_RESELECTED = 0x40;
	constexpr static uint8_t INTS_SELECTED = 0x80;

	constexpr static uint8_t SSTS_DREQ_EMPTY = 0x01;
	constexpr static uint8_t SSTS_DREQ_FULL = 0x02;
	constexpr static uint8_t SSTS_TC_ZERO = 0x04;
	constexpr static uint8_t SSTS_SCSI_RST = 0x08;
	constexpr static uint8_t SSTS_XFER_IN_PROGRESS = 0x10;
	constexpr static uint8_t SSTS_SPC_BUSY = 0x20;
	constexpr static uint8_t SSTS_TARG_CONNECTED = 0x40;
	constexpr static uint8_t SSTS_INIT_CONNECTED = 0x80;

	constexpr static uint8_t SERR_OFFSET_ERROR = 0x01;
	constexpr static uint8_t SERR_SHORT_PERIOD = 0x02;
	constexpr static uint8_t SERR_PHASE_ERROR = 0x04;
	constexpr static uint8_t SERR_TC_P_ERROR = 0x08;
	constexpr static uint8_t SERR_DATA_ERROR_SPC = 0x40;
	constexpr static uint8_t SERR_DATA_ERROR_SCSI = 0x80;

	constexpr static uint8_t SDGC_DIAG_IO = 0x01;
	constexpr static uint8_t SDGC_DIAG_CD = 0x02;
	constexpr static uint8_t SDGC_DIAG_MSG = 0x04;
	constexpr static uint8_t SDGC_DIAG_BSY = 0x08;
	constexpr static uint8_t SDGC_DIAG_ACK = 0x40;
	constexpr static uint8_t SDGC_DIAG_REQ = 0x80;

	emu_timer *m_timer;
	emu_timer *m_delay_timer;

	enum TimerId {
		Delay,
		Timeout,
	};
	enum class State: uint8_t {
		Idle,
		ArbitrationWaitBusFree,
		ArbitrationAssertBSY,
		ArbitrationWait,
		ArbitrationAssertSEL,
		ArbitrationDeAssertBSY,
		SelectionWaitBusFree,
		SelectionAssertSEL,
		SelectionWaitBSY,
		Selection,
		TransferWaitReq,
		TransferSendData,
		TransferSendDataDMAReq,
		TransferSendDataDMAResp,
		TransferRecvData,
		TransferRecvDataDMAReq,
		TransferRecvDataDMAResp,
		TransferSendAck,
		TransferWaitDeassertREQ,
		TransferDeassertACK
		//TransferCommand,
	} m_state;

	void update_ssts(void);
	void update_ints(void);

	void scsi_disconnect_timeout(void);
	void scsi_command_complete(void);
	void scsi_disconnect(void);
	void update_state(mb87030_device::State new_state, int delay = 0, int timeout = 0);
	auto get_state_name(State state) const;
	void scsi_set_ctrl(uint32_t val, uint32_t mask);
	uint32_t scsi_get_ctrl();
	void step(bool timeout);

	devcb_write_line m_irq_handler;
	devcb_write_line m_dreq_handler;

	TIMER_CALLBACK_MEMBER(delay_timeout);
	TIMER_CALLBACK_MEMBER(timeout);

	// registers
	uint8_t m_bdid;
	uint8_t m_sctl;
	uint8_t m_scmd;
	uint8_t m_tmod;
	uint8_t m_ints;
	uint8_t m_sdgc;
	uint8_t m_ssts;
	uint8_t m_serr;
	uint8_t m_pctl;
	uint8_t m_mbc;
	uint8_t m_dreg;
	uint8_t m_temp;
	uint8_t m_tch;
	uint8_t m_tcm;
	uint32_t m_tc;
	uint8_t m_exbf;

	uint8_t m_bus_data;
	uint8_t m_hdb;
	bool m_hdb_loaded;
	bool m_send_atn_during_selection;
	util::fifo <uint8_t, 8> m_fifo;

	uint8_t m_scsi_phase;
	uint32_t m_scsi_ctrl;

	bool m_dma_transfer;
};


DECLARE_DEVICE_TYPE(MB87030, mb87030_device)

#endif // MAME_MACHINE_MB87030_H
