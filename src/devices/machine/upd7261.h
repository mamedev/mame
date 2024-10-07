// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_MACHINE_UPD7261_H
#define MAME_MACHINE_UPD7261_H

#pragma once

#include "imagedev/harddriv.h"

class upd7261_device
	: public device_t
{
public:
	upd7261_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	// output lines
	auto out_dreq() { return m_dreq.bind(); }
	auto out_int() { return m_int.bind(); }

	// input lines
	void tc_w(int state);
	void head_w(u8 data) { m_head = data; }

	void map(address_map &map) ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void set_dreq(int state);
	void set_int(bool state);

	u8 data_r();
	u8 status_r();
	void data_w(u8 data);
	void command_w(u8 data);

	void state_timer(s32 param);
	attotime state_step(s32 param);

private:
	optional_device_array<harddisk_image_device, 8> m_drive;

	devcb_write_line m_dreq;
	devcb_write_line m_int;

	emu_timer *m_state_timer;

	u32 m_state;
	u8 m_head;    // head number (extended)

	u8 m_status;  // status register
	u8 m_est;     // error status byte
	u8 m_ist;     // interrupt status byte
	u8 m_ua;      // unit address
	u16 m_pcn[4]; // physical cylinder number

	struct specify
	{
		// data transfer length
		u16 dtl() const;

		// step time in ticks
		unsigned stp(unsigned cylinders) const;

		u8 mode; // operation mode
		u8 dtlh; // data transfer length (high byte)
		u8 dtll; // data transfer length (low byte)
		u8 etn;  // ending track number
		u8 esn;  // ending sector number
		u8 gpl2; // gap length 2
		u8 rwch; // reduced write current cylinder (high byte)
		u8 rwcl; // reduced write current cylinder (low byte)
	}
	m_specify;
	struct transfer
	{
		// logical cylinder number
		u16 lcn() const;

		u8 phn;  // physical head number
		u8 lcnh; // logical cylinder number (high byte)
		u8 lcnl; // logical cylinder number (low byte)
		u8 lhn;  // logical head number
		u8 lsn;  // logical sector number
		u8 scnt; // sector count
	}
	m_transfer;

	// data buffer
	u32 m_buf_index;
	u32 m_buf_count;
	std::unique_ptr<u8[]> m_buf;

	// i/o line state
	bool m_dreq_state;
	bool m_int_state;
	bool m_tc_state;
};

DECLARE_DEVICE_TYPE(UPD7261, upd7261_device)

#endif // MAME_MACHINE_UPD7261_H
