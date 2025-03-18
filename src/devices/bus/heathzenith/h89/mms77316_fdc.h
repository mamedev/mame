// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Magnolia Microsystems 77316 soft-sectored floppy controller

****************************************************************************/

#ifndef MAME_BUS_HEATHZENITH_H89_MMS77316_FDC_H
#define MAME_BUS_HEATHZENITH_H89_MMS77316_FDC_H

#pragma once

#include "h89bus.h"

#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"

#include "bus/heathzenith/h89/intr_cntrl.h"

class mms77316_fdc_device : public device_t, public device_h89bus_right_card_interface
{
public:
	mms77316_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	virtual void write(u8 select_lines, u8 offset, u8 data) override;
	virtual u8 read(u8 select_lines, u8 offset) override;

	template <typename T> void set_intr_cntrl(T &&tag) { m_intr_cntrl.set_tag(std::forward<T>(tag)); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	void ctrl_w(u8 val);
	void data_w(u8 val);
	u8 data_r();

	void set_irq(int state);
	void set_drq(int state);

	// Burst mode was required for a 2 MHz Z80 to handle 8" DD data rates.
	// The typical irq/drq was too slow, this utilizes wait states to read the
	// WD1797 data port once the drq line is high.
	inline bool burst_mode_r() { return !m_drq_allowed; }

private:
	required_device<fd1797_device> m_fdc;
	required_device_array<floppy_connector, 8> m_floppies;
	required_device<heath_intr_socket> m_intr_cntrl;

	bool m_irq_allowed;
	bool m_drq_allowed;

	bool m_irq;
	bool m_drq;
	u32 m_drq_count;

	/// Bits set in cmd_ControlPort_c
	static constexpr u8 ctrl_525DriveSel_c = 2;
	static constexpr u8 ctrl_EnableIntReq_c = 3;
	static constexpr u8 ctrl_EnableDrqInt_c = 5;
	static constexpr u8 ctrl_SetMFMRecording_c = 6;

	static constexpr XTAL MASTER_CLOCK = XTAL(8'000'000);
	static constexpr XTAL FIVE_IN_CLOCK = MASTER_CLOCK / 8;
	static constexpr XTAL EIGHT_IN_CLOCK = MASTER_CLOCK / 4;
};

DECLARE_DEVICE_TYPE(H89BUS_MMS77316, device_h89bus_right_card_interface)

#endif // MAME_BUS_HEATHZENITH_H89_MMS77316_FDC_H
