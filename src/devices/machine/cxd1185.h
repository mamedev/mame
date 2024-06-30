// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_MACHINE_CXD1185_H
#define MAME_MACHINE_CXD1185_H

#pragma once

#include "machine/nscsi_bus.h"

class cxd1185_device
	: public nscsi_device
	, public nscsi_slot_card_interface
{
public:
	cxd1185_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	// device configuration
	auto irq_out_cb() { return m_irq_out_cb.bind(); }
	auto drq_out_cb() { return m_drq_out_cb.bind(); }
	auto port_out_cb() { return m_port_out_cb.bind(); }

	void map(address_map &map);

	// dma and port handlers
	u8 dma_r();
	void dma_w(u8 data);
	void port_w(u8 data);

protected:
	// device_t overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// ncsci_device overrides
	virtual void scsi_ctrl_changed() override;

	// register handlers
	u8 status_r();
	u8 scsi_data_r();
	template <unsigned Register> u8 int_req_r();
	u8 scsi_ctrl_monitor_r();
	u8 fifo_status_r();
	u8 scsi_id_r() { return m_scsi_id; }
	template <unsigned Byte> u8 count_r() { return u8(m_count >> (Byte * 8)); }
	template <unsigned Register> u8 int_auth_r() { return m_int_auth[Register]; }
	u8 mode_r() { return m_mode; }
	u8 sync_ctrl_r() { return m_sync_ctrl; }
	u8 scsi_ctrl_r() { return m_scsi_ctrl; }
	u8 ioport_r() { return m_ioport; }

	void command_w(u8 data);
	void scsi_data_w(u8 data);
	void environ_w(u8 data);
	void timer_w(u8 data);
	void scsi_id_w(u8 data) { m_scsi_id = data; }
	template <unsigned Byte> void count_w(u8 data) { m_count &= ~(0xffU << (Byte * 8)); m_count |= u32(data) << (Byte * 8); }
	template <unsigned Register> void int_auth_w(u8 data);
	void mode_w(u8 data) { m_mode = data; }
	void sync_ctrl_w(u8 data) { m_sync_ctrl = data; }
	void scsi_ctrl_w(u8 data);
	void ioport_w(u8 data);

	// state machine
	void state_timer(s32 param);
	int state_step();

	// other logic
	void reset_chip();
	void reset_fifo();
	void int_check();
	void set_drq(bool asserted);

private:
	devcb_write_line m_irq_out_cb;
	devcb_write_line m_drq_out_cb;
	devcb_write8 m_port_out_cb;

	emu_timer *m_state_timer;
	enum state : unsigned
	{
		IDLE,

		// arbitration
		ARB_BUS_FREE,
		ARB_START,
		ARB_EVALUATE,

		// selection
		SEL_START,
		SEL_DELAY,
		SEL_WAIT_BSY,
		SEL_COMPLETE,

		// information transfer
		XFR_INFO,
		XFR_IN,
		XFR_IN_NEXT,
		XFR_IN_REQ,
		XFR_IN_DRAIN,
		XFR_OUT,
		XFR_OUT_NEXT,
		XFR_OUT_REQ,
		XFR_INFO_DONE,

		// scsi bus reset
		BUS_RESET,
		BUS_RESET_DONE,

		COMPLETE,
	}
	m_state;

	enum dma_direction : u8
	{
		DMA_NONE,
		DMA_IN,
		DMA_OUT
	};

	// internal state
	bool m_irq_asserted;
	bool m_drq_asserted;
	util::fifo <u8, 16> m_fifo;
	bool m_pio_data_mode;
	bool m_pio_ctrl_mode;
	u32 m_scsi_ctrl_state;
	dma_direction m_last_dma_direction;

	enum status_mask : u8
	{
		CIP  = 0x01, // command in progress
		MIRQ = 0x02, // monitor irq
		TRBZ = 0x04, // transfer byte count zero
		TARG = 0x08, // target
		INIT = 0x10, // initiator

		MDBP = 0x40, // monitor scsi bus DBP
		MRST = 0x80, // monitor scsi bus RST
	};
	enum command_mask : u8
	{
		CMD  = 0x0f, // command code
		TRBE = 0x10, // transfer byte counter enable
		DMA  = 0x20, // dma mode
		CAT  = 0xc0, // command category

		CMD_RESET   = 0x01,
		CMD_SEL_ATN = 0x42,
		CMD_XFR_PAD = 0xc1,
	};
	enum int_req1 : u8
	{
		ARBF = 0x01, // arbitration fail
		SWOA = 0x02, // selection without atn
		SWA  = 0x04, // selection with atn
		RSL  = 0x08, // reselected
		STO  = 0x10, // selection time over
	};
	enum int_req2 : u8
	{
		RMSG = 0x01, // req in message phase
		SPE  = 0x02, // scsi bus parity error
		DPE  = 0x04, // data bus parity error
		DATN = 0x08, // drive atn
		PHC  = 0x10, // phase change
		SRST = 0x20, // scsi reset
		DCNT = 0x40, // disconnected
		FNC  = 0x80, // function complete
	};
	enum fifo_status_mask : u8
	{
		FC  = 0x0f, // fifo count
		FIF = 0x10, // fifo is full

		FIE = 0x80, // fifo is empty
	};
	enum environ_mask : u8
	{
		FS0  = 0x01,
		FS1  = 0x02,
		FS   = 0x03, // clock division ratio

		SIRM = 0x10, // irq active low
		DPEN = 0x20, // data bus parity enable
		SDPM = 0x40, // data bus parity even
		DIFE = 0x80, // differential mode
	};
	enum scsi_ctrl_monitor_mask : u8
	{
		MATN = 0x01,
		MACK = 0x02,
		MREQ = 0x04,
		MIO  = 0x08,
		MCD  = 0x10,
		MMSG = 0x20,
		MSEL = 0x40,
		MBSY = 0x80,
	};
	enum scsi_id_mask : u8
	{
		OID  = 0x07, // owner id
		SMOD = 0x10, // single initiator mode
		SID  = 0xe0, // selecting id
		TID  = 0xe0, // target id
	};
	enum mode_mask : u8
	{
		BDMA = 0x01, // burst dma mode
		SSPE = 0x04, // ignore selection scsi parity error
		SPHI = 0x08, // scsi phase change ignore
		TMSL = 0x10, // scsi reset timer enable
		HATN = 0x20, // halt on atn
		HSPE = 0x40, // halt on scsi parity error
		HDPE = 0x80, // halt on data bus parity error
	};
	enum sync_ctrl_mask : u8
	{
		TOF = 0x0f, // transfer offset
		TPD = 0xf0, // transfer cycle
	};
	enum io_port_mask : u8
	{
		PRT = 0x0f, // i/o port data
		PCN = 0xf0, // i/o port control
	};
	enum scsi_ctrl_mask : u8
	{
		AATN = 0x01,
		AACK = 0x02,
		AREQ = 0x04,
		AIO  = 0x08,
		ACD  = 0x10,
		AMSG = 0x20,
		ASEL = 0x40,
		ABSY = 0x80,
	};

	// registers
	u8 m_status;
	u8 m_command;
	u8 m_int_req[2];
	u8 m_environ;
	unsigned m_sel_time;
	unsigned m_rst_time;
	u8 m_scsi_id;
	u8 m_int_auth[2];
	u8 m_mode;
	u32 m_count;
	u8 m_sync_ctrl;
	u8 m_scsi_ctrl;
	u8 m_ioport;
};

DECLARE_DEVICE_TYPE(CXD1185, cxd1185_device)

#endif // MAME_MACHINE_CXD1185_H
