// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Magnolia Microsystems 77316 soft-sectored floppy controller

****************************************************************************/

#ifndef MAME_HEATHKIT_MMS77316_FDC_H
#define MAME_HEATHKIT_MMS77316_FDC_H

#pragma once

#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"


class mms77316_fdc_device : public device_t
{
public:

	mms77316_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	void write(offs_t reg, u8 val);
	u8 read(offs_t reg);

	auto irq_cb() { return m_irq_cb.bind(); }
	auto drq_cb() { return m_drq_cb.bind(); }

protected:

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	void ctrl_w(u8 val);
	void data_w(u8 val);
	u8 data_r();

	void set_irq(int state);
	void set_drq(int state);

	// Burst mode was required for a 2 MHz Z80 to handle 8" DD data rates.
	// The typical irq/drq was too slow, this utilizes wait states to read the
	// WD1797 data port once the drq line is high.
	inline bool burstMode() { return !m_drq_allowed; }

private:

	devcb_write_line                           m_irq_cb;
	devcb_write_line                           m_drq_cb;

	required_device<fd1797_device>             m_fdc;
	required_device_array<floppy_connector, 8> m_floppies;

	bool                                       m_irq_allowed;
	bool                                       m_drq_allowed;

	bool                                       m_irq;
	bool                                       m_drq;
	u32                                        m_drq_count;

	/// Bits set in cmd_ControlPort_c
	static constexpr u8 ctrl_525DriveSel_c     = 2;
	static constexpr u8 ctrl_EnableIntReq_c    = 3;
	static constexpr u8 ctrl_EnableDrqInt_c    = 5;
	static constexpr u8 ctrl_SetMFMRecording_c = 6;

	static constexpr XTAL MASTER_CLOCK         = XTAL(8'000'000);
	static constexpr XTAL FIVE_IN_CLOCK        = MASTER_CLOCK / 8;
	static constexpr XTAL EIGHT_IN_CLOCK       = MASTER_CLOCK / 4;

};

DECLARE_DEVICE_TYPE(MMS77316_FDC, mms77316_fdc_device)


#endif // MAME_HEATHKIT_MMS77316_FDC_H
