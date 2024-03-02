// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Heathkit H-17 Floppy Disk Controller


  Model number: H-88-1

  TODO
   - Mame core must support hard-sectored disk images.
   - used floppy clock bits to clock USRT received clock.
   - Add support for a heath hard-sectored disk support (h17disk).

****************************************************************************/

#ifndef MAME_HEATHKIT_H17_FDC_H
#define MAME_HEATHKIT_H17_FDC_H

#pragma once

#include "imagedev/floppy.h"
#include "machine/s2350.h"


class heath_h17_fdc_device : public device_t
{
public:
	heath_h17_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	auto floppy_ram_wp_cb() { return m_floppy_ram_wp.bind(); }

	void write(offs_t reg, u8 val);
	u8   read(offs_t reg);

	void side_select_w(int state);

protected:
	static constexpr u8 MAX_FLOPPY_DRIVES = 3;

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	void ctrl_w(u8 val);
	u8   floppy_status_r();

	void set_floppy(floppy_image_device *floppy);
	void step_w(int state);
	void dir_w(int state);
	void set_motor(bool motor_on);

	void sync_character_received(int state);

	TIMER_DEVICE_CALLBACK_MEMBER(tx_timer_cb);

	devcb_write_line m_floppy_ram_wp;

	required_device<s2350_device> m_s2350;
	required_device_array<floppy_connector, MAX_FLOPPY_DRIVES> m_floppies;
	required_device<timer_device> m_tx_timer;

	bool m_motor_on;
	bool m_write_gate;
	bool m_sync_char_received;
	u8   m_step_direction;
	u8   m_side;

	floppy_image_device *m_floppy;

	/// write bit control port
	static constexpr u8 CTRL_WRITE_GATE       = 0;
	static constexpr u8 CTRL_DRIVE_SELECT_0   = 1;
	static constexpr u8 CTRL_DRIVE_SELECT_1   = 2;
	static constexpr u8 CTRL_DRIVE_SELECT_2   = 3;
	static constexpr u8 CTRL_MOTOR_ON         = 4; // Controls all the drives
	static constexpr u8 CTRL_DIRECTION        = 5; // (0 = out)
	static constexpr u8 CTRL_STEP_COMMAND     = 6; // (Active high)
	static constexpr u8 CTRL_WRITE_ENABLE_RAM = 7; // 0 - write protected

	// USRT clock
	static constexpr XTAL USRT_BASE_CLOCK = XTAL(12'288'000) / 6 / 16;
	static constexpr u32  USRT_TX_CLOCK   = USRT_BASE_CLOCK.value();
};


DECLARE_DEVICE_TYPE(HEATH_H17_FDC, heath_h17_fdc_device)


#endif // MAME_HEATHKIT_H17_FDC_H
