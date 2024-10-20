// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    Apple SWIM1 floppy disk controller

*********************************************************************/
#ifndef MAME_MACHINE_SWIM1_H
#define MAME_MACHINE_SWIM1_H

#pragma once

#include "applefdintf.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


class swim1_device : public applefdintf_device
{
public:
	// construction/destruction
	swim1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual u8 read(offs_t offset) override;
	virtual void write(offs_t offset, u8 data) override;

	virtual void set_floppy(floppy_image_device *floppy) override;
	virtual floppy_image_device *get_floppy() const override;

	virtual void sync() override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(update);

private:
	enum {
		MODE_IDLE,
		MODE_ACTIVE, MODE_DELAY, // m_iwm_active modes
		MODE_READ, MODE_WRITE    // m_iwm_rw modes
	};

	// iwm state machine states
	enum {
		S_IDLE,
		SR_WINDOW_EDGE_0,
		SR_WINDOW_EDGE_1,
		SW_WINDOW_LOAD,
		SW_WINDOW_MIDDLE,
		SW_WINDOW_END,
		SW_UNDERRUN,
	};

	// ism buffered byte marks
	enum {
		M_MARK = 0x100,
		M_CRC  = 0x200,
		M_CRC0 = 0x400
	};

	// parameter ram addresses
	enum {
		P_MINCT, P_MULT, P_SSL, P_SSS, P_SLL, P_SLS, P_RPT, P_CSLS,
		P_LSL, P_LSS, P_LLL, P_LLS, P_LATE, P_TIME0, P_EARLY, P_TIME1
	};

	// CSM states
	enum {
		CSM_INIT,
		CSM_COUNT_MIN,
		CSM_WAIT_NON_MIN,
		CSM_CHECK_MARK,
		CSM_SYNCHRONIZED
	};

	floppy_image_device *m_floppy;
	emu_timer *m_timer;

	u64 m_flux_write_start;
	std::array<u64, 32> m_flux_write;
	u32 m_flux_write_count;
	u64 m_last_sync;

	u8 m_ism_param[16];
	u8 m_ism_mode, m_ism_setup;
	u8 m_ism_error;
	u8 m_ism_param_idx, m_ism_fifo_pos;
	u8 m_ism_tss_sr, m_ism_tss_output, m_ism_current_bit;
	u16 m_ism_fifo[2];
	u16 m_ism_sr;
	u16 m_ism_crc;
	u32 m_ism_half_cycles_before_change;
	u8 m_ism_correction_factor[2];

	u64 m_ism_latest_edge;
	u8 m_ism_prev_ls;
	u8 m_ism_csm_state;
	u32 m_ism_csm_error_counter[2];
	u8 m_ism_csm_pair_side, m_ism_csm_min_count;
	u8 m_ism_tsm_out, m_ism_tsm_bits;
	bool m_ism_tsm_mark;

	u64 m_iwm_next_state_change, m_iwm_sync_update, m_iwm_async_update;
	int m_iwm_active, m_iwm_rw, m_iwm_rw_state;
	u8 m_iwm_data, m_iwm_whd, m_iwm_mode, m_iwm_status, m_iwm_control, m_iwm_rw_bit_count;
	u8 m_iwm_rsh, m_iwm_wsh;
	u8 m_iwm_to_ism_counter;
	u8 m_iwm_devsel;

	emu_timer *m_sync_timer;
	TIMER_CALLBACK_MEMBER(ism_periodic_sync);

	u64 time_to_cycles(const attotime &tm) const;
	attotime cycles_to_time(u64 cycles) const;
	void flush_write(u64 when = 0);

	u64 iwm_window_size() const;
	u64 iwm_half_window_size() const;
	u64 iwm_read_register_update_delay() const;
	inline bool iwm_is_sync() const;
	void iwm_mode_w(u8 data);
	void iwm_data_w(u8 data);
	void iwm_control(int offset, u8 data);
	void iwm_sync();

	void ism_fifo_clear();
	bool ism_fifo_push(u16 data);
	u16 ism_fifo_pop();
	void ism_show_mode() const;
	void ism_crc_update(int bit);
	void ism_crc_clear();
	u8 ism_read(offs_t offset);
	void ism_write(offs_t offset, u8 data);
	void ism_sync();
	void ism_update_dat1byte();
};

DECLARE_DEVICE_TYPE(SWIM1, swim1_device)

#endif  /* MAME_MACHINE_SWIM1_H */
