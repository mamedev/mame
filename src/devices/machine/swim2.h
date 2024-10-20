// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    Apple SWIM2 floppy disk controller

*********************************************************************/
#ifndef MAME_MACHINE_SWIM2_H
#define MAME_MACHINE_SWIM2_H

#pragma once

#include "applefdintf.h"
#include "fdc_pll.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


class swim2_device : public applefdintf_device
{
public:
	// construction/destruction
	swim2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual u8 read(offs_t offset) override;
	virtual void write(offs_t offset, u8 data) override;

	virtual void set_floppy(floppy_image_device *floppy) override;
	virtual floppy_image_device *get_floppy() const override;

	virtual void sync() override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	enum {
		M_MARK = 0x100,
		M_CRC  = 0x200,
		M_CRC0 = 0x400
	};

	floppy_image_device *m_floppy;
	u8 m_param[4];
	u8 m_mode, m_setup, m_error, m_param_idx, m_fifo_pos, m_tss_sr, m_tss_output, m_current_bit;
	u16 m_fifo[2], m_sr;
	u16 m_crc, m_mfm_sync_counter;
	u32 m_half_cycles_before_change;

	u64 m_last_sync;
	u64 m_flux_write_start;
	std::array<u64, 32> m_flux_write;
	u32 m_flux_write_count;

	fdc_pll_t m_pll;

	u64 time_to_cycles(const attotime &tm) const;
	attotime cycles_to_time(u64 cycles) const;

	void fifo_clear();
	bool fifo_push(u16 data);
	u16 fifo_pop();
	void flush_write(u64 when = 0);
	void show_mode() const;

	void crc_update(int bit);
	void crc_clear();

	void update_dat1byte();
};

DECLARE_DEVICE_TYPE(SWIM2, swim2_device)

#endif  /* MAME_MACHINE_SWIM2_H */
