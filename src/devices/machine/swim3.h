// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    Apple SWIM3 floppy disk controller

*********************************************************************/
#ifndef MAME_MACHINE_SWIM3_H
#define MAME_MACHINE_SWIM3_H

#pragma once

#include "applefdintf.h"
#include "machine/fdc_pll.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


class swim3_device : public applefdintf_device
{
public:
	// construction/destruction
	swim3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto irq_cb() { return m_irq_cb.bind(); }
	auto drq_cb() { return m_drq_cb.bind(); }

	virtual u8 read(offs_t offset) override;
	virtual void write(offs_t offset, u8 data) override;

	// For direct dma access
	u8 dma_r();
	void dma_w(u8 data);

	virtual void set_floppy(floppy_image_device *floppy) override;
	virtual floppy_image_device *get_floppy() const override;

	virtual void sync() override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(update);

private:
	enum {
		S_IDLE,

		S_STEP,
		S_STEP_1,
		S_STEP_0,
		S_STEP_DONE,

		S_ID,
	};

	enum {
		L_IDLE,

		L_GCR_SEARCH_ID,

		L_MFM_SEARCH_ID,
		L_MFM_SCAN_ID,
		L_MFM_READ_ID,
		L_MFM_READ_ID_BYTE,

		L_MFM_SEARCH_DATA,
		L_MFM_SCAN_DATA,
		L_MFM_READ_DATA,
		L_MFM_READ_DATA_BYTE,
	};

	struct live_info {
		attotime tm;
		fdc_pll_t pll;
		int state, next_state;
		u16 shift_reg;
		u16 crc;
		int bit_counter;
		bool data_separator_phase, data_bit_context;
		uint8_t data_reg;
	};

	devcb_write_line m_irq_cb, m_drq_cb;
	emu_timer *m_timer;
	floppy_image_device *m_floppy;

	live_info m_cur_live, m_checkpoint_live;

	u8 m_param;
	u8 m_mode, m_setup, m_irq, m_imask, m_step, m_error;
	u8 m_cur_track, m_cur_sector, m_cur_format;
	u8 m_gap, m_sect1, m_xfer, m_fifo[2], m_fifo_pos;

	bool m_drq_write;

	int m_state;

	void update_irq();
	void update_drq();
	void index_callback(floppy_image_device *floppy, int state);
	void run(bool timeout, bool index);
	void delay(int);
	void checkpoint();
	void rollback();
	void live_abort();
	void live_delay(int state);
	bool read_one_bit(const attotime &limit);
	bool write_one_bit(const attotime &limit);
	void live_start(int state, bool start_writing = false);
	void live_run(attotime limit = attotime::never);

	void fifo_push(u8 data);
	u8 fifo_pop();

	void show_mode() const;
};

DECLARE_DEVICE_TYPE(SWIM3, swim3_device)

#endif  /* MAME_MACHINE_SWIM3_H */
