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
	heath_z37_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void write(offs_t reg, uint8_t val);
	uint8_t read(offs_t reg);
	auto irq_cb() { return m_fd_irq_cb.bind(); }
	auto drq_cb() { return m_drq_cb.bind(); }

	auto block_interrupt_cb() { return m_block_interrupt_cb.bind(); }

protected : virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override;

	void ctrl_w(uint8_t val);
	uint8_t ctrl_r();

	void intf_w(uint8_t val);
	uint8_t intf_r();

	void stat_w(uint8_t val);
	uint8_t stat_r();

	void data_w(uint8_t val);
	uint8_t data_r();

  void set_irq(uint8_t data);
  void set_drq(uint8_t data);

private:
	devcb_write_line m_fd_irq_cb;
	devcb_write_line m_drq_cb;
	devcb_write_line m_block_interrupt_cb;

	required_device<fd1797_device> m_fdc;
	required_device_array<floppy_connector, 4> m_floppies;

	u8 m_control_reg;
	u8 m_interface_reg;
	bool m_intrq_allowed;
	bool m_drq_allowed;
	bool m_access_track_sector;

	floppy_image_device *m_floppy;

	/// Bits set in cmd_ControlPort_c - DK.CON
	static constexpr u8 ctrl_EnableIntReq_c = 0;
	static constexpr u8 ctrl_EnableDrqInt_c = 1;
	static constexpr u8 ctrl_SetMFMRecording_c = 2;
	static constexpr u8 ctrl_MotorsOn_c = 3;
	static constexpr u8 ctrl_Drive_0_c = 4;
	static constexpr u8 ctrl_Drive_1_c = 5;
	static constexpr u8 ctrl_Drive_2_c = 6;
	static constexpr u8 ctrl_Drive_3_c = 7;

	/// Bits to set alternate registers on InterfaceControl_c - DK.INT
	static constexpr uint8_t if_SelectSectorTrack_c = 0;

};

DECLARE_DEVICE_TYPE(HEATH_Z37_FDC, heath_z37_fdc_device)


#endif // MAME_HEATHKIT_Z37_FDC_H
