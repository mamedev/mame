// license:BSD-3-Clause
// copyright-holders:Eric Anderson
#ifndef MAME_BUS_S100_VECTORDUALMODE_H
#define MAME_BUS_S100_VECTORDUALMODE_H

#pragma once

#include "bus/s100/s100.h"
#include "imagedev/floppy.h"
#include "machine/fdc_pll.h"

class s100_vector_dualmode_device :
		public device_t,
		public device_s100_card_interface
{
public:
	s100_vector_dualmode_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;
	void device_add_mconfig(machine_config &config) override;

	uint8_t s100_sinp_r(offs_t offset) override;
	void s100_sout_w(offs_t offset, uint8_t data) override;

private:
	TIMER_CALLBACK_MEMBER(motor_off);
	TIMER_CALLBACK_MEMBER(byte_cb);
	bool hdd_selected();
	bool get_next_bit(attotime &tm, const attotime &limit);
	void floppy_index_cb(floppy_image_device *floppy, int state);
	void start_of_sector();

	required_device_array<floppy_connector, 4> m_floppy;
	uint8_t m_ram[512];
	uint16_t m_cmar;
	uint8_t m_drive;
	uint8_t m_sector;
	uint8_t m_fdd_sector_counter;
	bool m_read;
	bool m_busy;
	emu_timer *m_motor_on_timer;
	attotime m_last_sector_pulse; // fdd

	fdc_pll_t m_pll;
	emu_timer *m_byte_timer;
	uint16_t m_pending_byte;
	uint8_t m_pending_size;
};

DECLARE_DEVICE_TYPE(S100_VECTOR_DUALMODE, s100_vector_dualmode_device)

#endif // MAME_BUS_S100_VECTORDUALMODE_H
