// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Heathkit Z-37 Floppy Disk Controller

****************************************************************************/

#ifndef MAME_HEATHKIT_Z37_FDC_H
#define MAME_HEATHKIT_Z37_FDC_H

#pragma once

#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"


class heath_z37_fdc_device : public device_t
{
public:
	heath_z37_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	void write(offs_t reg, u8 val);
	u8 read(offs_t reg);
	auto irq_cb() { return m_irq_cb.bind(); }
	auto drq_cb() { return m_drq_cb.bind(); }

	auto block_interrupt_cb() { return m_block_interrupt_cb.bind(); }

protected:

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	void ctrl_w(u8 val);

	void intf_w(u8 val);

	void cmd_w(u8 val);
	u8 stat_r();

	void data_w(u8 val);
	u8 data_r();

	void set_irq(int state);
	void set_drq(int state);

private:
	devcb_write_line                           m_irq_cb;
	devcb_write_line                           m_drq_cb;
	devcb_write_line                           m_block_interrupt_cb;

	required_device<fd1797_device>             m_fdc;
	required_device_array<floppy_connector, 4> m_floppies;

	bool                                       m_irq_allowed;
	bool                                       m_drq_allowed;
	bool                                       m_access_track_sector;

	/// Bits set in cmd_ControlPort_c - DK.CON
	static constexpr u8 ctrl_EnableIntReq_c    = 0;
	static constexpr u8 ctrl_EnableDrqInt_c    = 1;
	static constexpr u8 ctrl_SetMFMRecording_c = 2;
	static constexpr u8 ctrl_MotorsOn_c        = 3;
	static constexpr u8 ctrl_Drive_0_c         = 4;
	static constexpr u8 ctrl_Drive_1_c         = 5;
	static constexpr u8 ctrl_Drive_2_c         = 6;
	static constexpr u8 ctrl_Drive_3_c         = 7;

	/// Bits to set alternate registers on InterfaceControl_c - DK.INT
	static constexpr u8 if_SelectSectorTrack_c = 0;
};

DECLARE_DEVICE_TYPE(HEATH_Z37_FDC, heath_z37_fdc_device)


#endif // MAME_HEATHKIT_Z37_FDC_H
