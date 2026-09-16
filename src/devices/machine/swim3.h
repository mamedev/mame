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

		S_FORMAT_WAIT_INDEX,
		S_FORMAT,
	};

	enum {
		L_IDLE,

		L_GCR_SEARCH_ID,
		L_GCR_READ_ID,
		L_GCR_READ_ID_BYTE,

		L_GCR_SEARCH_DATA,
		L_GCR_READ_DATA,
		L_GCR_READ_DATA_BYTE,

		L_GCR_COPY_PROTECT,
		L_GCR_COPY_PROTECT_BYTE,

		L_GCR_SKIP_GAP,

		L_MFM_SEARCH_ID,
		L_MFM_SCAN_ID,
		L_MFM_READ_ID,
		L_MFM_READ_ID_BYTE,

		L_MFM_SEARCH_DATA,
		L_MFM_SCAN_DATA,
		L_MFM_READ_DATA,
		L_MFM_READ_DATA_BYTE,

		L_MFM_SKIP_GAP,
		L_MFM_SKIP_GAP_BYTE,

		L_WRITE_START,
		L_WRITE,
		L_WRITE_BYTE,
	};

	struct live_info {
		attotime tm;
		fdc_pll_t pll;
		int state, next_state;
		u16 shift_reg;
		u16 crc;
		int bit_counter;
		bool data_separator_phase, data_bit_context, no_crc;
		uint8_t data_reg;
		uint8_t idbuf[5];
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

	// state of the escaped byte stream the dma channel feeds to the writer
	u16 m_wr_esc_off;                 // bytes still to write with escaping off ($99 $0f)
	u8 m_wr_crc_left;                 // crc bytes still to write ($99 $04)
	bool m_wr_mark_pending;           // last byte was a mark, preset the crc before the next one
	bool m_wr_format;                 // this transaction is a track (format) write
	bool m_wr_gcr;                    // this transaction writes gcr, not mfm

	int m_state;

	// 6-and-2 encoding table, and the decoding table built from it in device_start
	static const u8 gcr6fw_tb[0x40];
	u8 m_gcr_bw[0x100];
	bool m_gcr_valid[0x100];

	void update_irq();
	void update_drq();
	void set_error(u8 error);
	void index_callback(floppy_image_device *floppy, int state);
	void run(bool timeout, bool index);
	void delay(int);
	void checkpoint();
	void rollback();
	void live_abort();
	void live_delay(int state);
	bool read_one_bit(const attotime &limit);
	bool read_one_bit_gcr(const attotime &limit, bool &byte_ready);
	bool write_one_bit(const attotime &limit);
	void live_write_raw(u16 raw);
	void live_write_mfm(u8 mfm);
	void live_write_gcr(u8 gcr);
	void live_start(int state, bool start_writing = false);
	void live_run(attotime limit = attotime::never);

	attotime cell_period(int index) const;
	u8 gcr_encode(u8 data) const;
	u8 gcr_decode(u8 data) const;

	void wr_start(bool format);
	bool wr_next_byte();
	void wr_emit_data(u8 data);
	void wr_emit_mark(u16 raw);
	void wr_finish();

	void xfer_done();
	bool sector_matches() const;

	bool fifo_push(u8 data, u8 error);
	bool fifo_pop(u8 &data, u8 error);

	void show_mode() const;
};

DECLARE_DEVICE_TYPE(SWIM3, swim3_device)

#endif  /* MAME_MACHINE_SWIM3_H */
