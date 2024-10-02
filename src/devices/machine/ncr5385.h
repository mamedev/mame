// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***********************************************************************

    NCR 5385 SCSI Controller emulation

************************************************************************
                            _____   _____
                    D2   1 |*    \_/     | 48  VCC
                    D1   2 |             | 47  D3
                    D0   3 |             | 46  D4
                 RESET   4 |             | 45  D5
                   ATN   5 |             | 44  D6
                   IGS   6 |             | 43  D7
                   I/O   7 |             | 42  BSYOUT
                   C/D   8 |             | 41  SB7
                   MSG   9 |             | 40  SB6
                   ACK  10 |             | 39  SB5
                   REQ  11 |             | 38  SB4
                  /ID2  12 |  NCR 5385E  | 37  SB3
                  /ID1  13 |             | 36  SB2
                  /ID0  14 |             | 35  SB1
                   ARB  15 |             | 34  SB9
                   CLK  16 |             | 33  SBP
                BSY IN  17 |             | 32  SELOUT
                SEL IN  18 |             | 31  /RD
                   INT  19 |             | 30  /WR
                 /SBEN  20 |             | 29  DREQ
                   /CS  21 |             | 28  TGS
                    A0  22 |             | 27  /DACK
                    A1  23 |             | 26  A3
                   GND  24 |_____________| 25  A2

************************************************************************/

#ifndef MAME_MACHINE_NCR5385_H
#define MAME_MACHINE_NCR5385_H

#pragma once

#include "machine/nscsi_bus.h"

class ncr5385_device
	: public nscsi_device
	, public nscsi_slot_card_interface
{
public:
	auto irq() { return m_int.bind(); }
	auto dreq() { return m_dreq.bind(); }

	void set_own_id(unsigned id) { m_own_id = id; }

	ncr5385_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	void map(address_map &map) ATTR_COLD;

	u8 dma_r();
	void dma_w(u8 data);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// ncsci_device implementation
	virtual void scsi_ctrl_changed() override;

	// read handlers
	u8 dat_r();
	u8 cmd_r();
	u8 ctl_r();
	u8 dst_id_r();
	u8 aux_status_r();
	u8 own_id_r();
	u8 int_status_r();
	u8 src_id_r();
	u8 dia_status_r();
	template <unsigned N> u8 cnt_r();
	u8 tst_r();

	// write handlers
	void dat_w(u8 data);
	void cmd_w(u8 data);
	void ctl_w(u8 data);
	void dst_id_w(u8 data);
	template <unsigned N> void cnt_w(u8 data);
	void tst_w(u8 data);

	// state machine, interrupts and dma
	void state_timer(s32 param);
	int state_step();
	bool remaining(u32 const count = 0) const;
	void update_int();
	void set_dreq(bool dreq);

private:
	devcb_write_line m_int;
	devcb_write_line m_dreq;

	emu_timer *m_state_timer;

	// registers
	u8 m_dat;
	u8 m_cmd;
	u8 m_ctl;
	u8 m_dst_id;
	u8 m_aux_status;
	u8 m_own_id;
	u8 m_int_status;
	u8 m_src_id;
	u8 m_dia_status;
	u32 m_cnt;

	// other state
	u32 m_state;
	u8 m_phase;
	u8 m_mode;
	bool m_sbx;

	bool m_int_state;
	bool m_dreq_state;
};

DECLARE_DEVICE_TYPE(NCR5385, ncr5385_device)

#endif // MAME_MACHINE_NCR5385_H
