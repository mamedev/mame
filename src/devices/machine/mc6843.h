// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Motorola 6843 floppy drive controller
//
// The Hitachi HD46503S, HD6843 and HD68A43 seem identical

#ifndef MAME_MACHINE_MC6843_H
#define MAME_MACHINE_MC6843_H

#pragma once

#include "imagedev/floppy.h"
#include "fdc_pll.h"

class mc6843_device : public device_t
{
public:
	mc6843_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void force_ready() { m_force_ready = true; }

	auto irq() { return m_irq.bind(); }

	void set_floppy(floppy_image_device *floppy);

	void map(address_map &map) ATTR_COLD;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(update_tick);

private:
	// Status flags
	enum {
		SA_BUSY  = 0x80,
		SA_IDX   = 0x40,
		SA_TNEQ  = 0x20,
		SA_WPT   = 0x10,
		SA_TRK0  = 0x08,
		SA_RDY   = 0x04,
		SA_DDM   = 0x02,
		SA_DTR   = 0x01,

		SB_HERR  = 0x80,
		SB_WERR  = 0x40,
		SB_FI    = 0x20,
		SB_SERR  = 0x10,
		SB_SAERR = 0x08,
		SB_DMERR = 0x04,
		SB_CRC   = 0x02,
		SB_DTERR = 0x01,

		I_STRB   = 0x08,
		I_SSR    = 0x04,
		I_SCE    = 0x02,
		I_RWCE   = 0x01,

		C_STZ    = 0x2,
		C_SEK    = 0x3,
		C_SSR    = 0x4,
		C_SSW    = 0x5,
		C_RCR    = 0x6,
		C_SWD    = 0x7,
		C_FFR    = 0xa,
		C_FFW    = 0xb,
		C_MSR    = 0xc,
		C_MSW    = 0xd,
	};

	enum {
		S_IDLE,

		S_STZ_STEP,
		S_STZ_STEP_WAIT,
		S_STZ_HEAD_SETTLING,

		S_SEEK_STEP,
		S_SEEK_STEP_WAIT,
		S_SEEK_HEAD_SETTLING,

		S_SRW_WAIT_READY,
		S_SRW_HEAD_SETTLING,
		S_SRW_START,

		S_FFW_WAIT_READY,
		S_FFW_HEAD_SETTLING,
		S_FFW_START,

		S_IDAM_BAD_TRACK,
		S_IDAM_BAD_CRC,
		S_IDAM_FOUND,
		S_IDAM_NOT_FOUND,

		S_DAM_NOT_FOUND,
		S_DAM_BAD_CRC,
		S_DAM_DONE,
	};

	enum {
		L_IDLE,

		L_IDAM_SEARCH,
		L_IDAM_CHECK_TRACK,
		L_IDAM_CHECK_SECTOR,
		L_IDAM_CHECK_CRC,

		L_DAM_SEARCH,
		L_DAM_DELETED,
		L_DAM_READ,
		L_DAM_READ_BYTE,

		L_DAM_WAIT,
		L_DAM_WRITE,
		L_DAM_WRITE_BYTE,

		L_FFW_BYTE,
		L_FFW_WRITE,
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

	devcb_write_line m_irq;
	bool m_force_ready;

	emu_timer *m_timer;
	floppy_image_device *m_floppy;

	live_info m_cur_live, m_checkpoint_live;

	int m_state;

	bool m_head_loaded, m_dir_loaded, m_dor_loaded, m_dor_needed;

	u8 m_dir;
	u8 m_dor;
	u8 m_ctar;
	u8 m_cmr;
	u8 m_isr;
	u8 m_sur;
	u8 m_stra;
	u8 m_sar;
	u8 m_strb;
	u8 m_gcr;
	u8 m_ccr;
	u8 m_ltar;

	u8 m_step_count;
	u8 m_idam_turns;

	u8 dir_r();
	void dor_w(u8 data);
	u8 ctar_r();
	void ctar_w(u8 data);
	u8 isr_r();
	void cmr_w(u8 data);
	u8 stra_r();
	void sur_w(u8 data);
	u8 strb_r();
	void sar_w(u8 data);
	void gcr_w(u8 data);
	void ccr_w(u8 data);
	void ltar_w(u8 data);

	void index_callback(floppy_image_device *floppy, int state);
	void ready_callback(floppy_image_device *floppy, int state);

	void command_start();
	void run(bool timeout, bool ready, bool index);
	void isr_raise(u8 flag);
	void delay(int);
	void live_start(int state, bool start_writing = false);
	void checkpoint();
	void rollback();
	void live_delay(int state);
	void live_sync();
	void live_abort();
	bool read_one_bit(const attotime &limit);
	bool write_one_bit(const attotime &limit);
	void live_run(attotime limit = attotime::never);
	bool is_ready() const;
};

DECLARE_DEVICE_TYPE(MC6843, mc6843_device)

#endif // MAME_MACHINE_MC6843_H
