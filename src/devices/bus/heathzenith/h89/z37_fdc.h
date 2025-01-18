// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Heathkit Z-37 Floppy Disk Controller

****************************************************************************/

#ifndef MAME_BUS_HEATHZENITH_H89_Z37_FDC_H
#define MAME_BUS_HEATHZENITH_H89_Z37_FDC_H

#pragma once

#include "h89bus.h"

#include "bus/heathzenith/h89/intr_cntrl.h"

class h89bus_z37_device : public device_t, public device_h89bus_right_card_interface
{
public:
	h89bus_z37_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	virtual void write(u8 select_lines, u8 offset, u8 data) override;
	virtual u8 read(u8 select_lines, u8 offset) override;

	template <typename T> void set_intr_cntrl(T &&tag) { m_intr_cntrl.set_tag(std::forward<T>(tag)); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	void ctrl_w(u8 val);

	void intf_w(u8 val);

	void cmd_w(u8 val);
	u8 stat_r();

	void data_w(u8 val);
	u8 data_r();

	void set_irq(int state);
	void set_drq(int state);

private:
	required_device<fd1797_device> m_fdc;
	required_device_array<floppy_connector, 4> m_floppies;
	required_device<heath_intr_socket> m_intr_cntrl;

	bool m_irq_allowed;
	bool m_drq_allowed;
	bool m_access_track_sector;

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
	static constexpr u8 if_SelectSectorTrack_c = 0;
};

DECLARE_DEVICE_TYPE(H89BUS_Z37, device_h89bus_right_card_interface)

#endif // MAME_BUS_HEATHZENITH_H89_Z37_FDC_H
