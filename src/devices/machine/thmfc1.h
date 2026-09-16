// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#ifndef MAME_MACHINE_THMFC1_H
#define MAME_MACHINE_THMFC1_H

#pragma once

#include "imagedev/floppy.h"

class thmfc1_device : public device_t
{
public:
	thmfc1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~thmfc1_device() = default;

	void map(address_map &map) ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override;

	TIMER_CALLBACK_MEMBER(motor_off);

private:
	enum {
		S0_BYTE   = 0x80,
		S0_END    = 0x10,
		S0_FREE   = 0x08,
		S0_CRCER  = 0x04,
		S0_DREQ   = 0x02,
		S0_SYNC   = 0x01,

		S1_INDX   = 0x40,
		S1_DKCH   = 0x20,
		S1_MTON   = 0x10,
		S1_TRK0   = 0x08,
		S1_WPRT   = 0x04,
		S1_RDY    = 0x02,

		C0_FM     = 0x20,
		C0_ENSYN  = 0x10,
		C0_NOMCK  = 0x08,
		C0_WGC    = 0x04,

		C1_SIDE   = 0x10,
		C1_DSYRD  = 0x01,

		C2_SISELB = 0x40,
		C2_DIRECB = 0x20,
		C2_STEP   = 0x10,
		C2_MTON   = 0x04,
		C2_DRS1   = 0x02,
		C2_DRS0   = 0x01,
	};

	enum {
		S_IDLE,
		S_WAIT_HEADER_SYNC,
		S_VERIFY_HEADER,
		S_SKIP_GAP,
		S_WAIT_SECTOR_SYNC,
		S_READ_SECTOR,
	};

	required_device_array<floppy_connector, 2> m_floppy;
	floppy_image_device *m_cur_floppy;
	emu_timer *m_timer_motoroff;

	u64 m_last_sync, m_window_start;
	int m_state;

	u16 m_shift_reg, m_crc, m_bit_counter;
	u8 m_data_reg;

	u8 m_cmd0, m_cmd1, m_cmd2, m_stat0;
	u8 m_data, m_clk, m_sect, m_trck, m_cell;

	bool m_data_separator_phase;

	u8 clk_bits() const;

	void cmd0_w(u8 data);
	void cmd1_w(u8 data);
	void cmd2_w(u8 data);
	void wdata_w(u8 data);
	void wclk_w(u8 data);
	void wsect_w(u8 data);
	void wtrck_w(u8 data);
	void wcell_w(u8 data);

	u8 stat0_r();
	u8 stat1_r();
	u8 rdata_r();

	u64 time_to_cycles(const attotime &tm) const;
	attotime cycles_to_time(u64 cycles) const;

	void sync();
	bool read_one_bit(u64 limit, u64 &next_flux_change);
};

DECLARE_DEVICE_TYPE(THMFC1, thmfc1_device)

#endif // MAME_MACHINE_THMFC1_H
