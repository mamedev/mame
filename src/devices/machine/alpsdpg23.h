// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    ALPS DPG23 4-pen X/Y plotter mechanism

    A ball-point pen carried on X/Y lead-screw steppers, lifted by a
    self-holding solenoid, with a 4-pen carousel changed by jogging the
    X axis into a bay to the left of the normal drawing area. Each
    reversal of X travel while in the bay advances a 12-tooth ratchet
    by one tooth (3 teeth per pen); a magnet at tooth 0 trips a reed
    switch read back as the colour sensor.

**********************************************************************/

#ifndef MAME_MACHINE_ALPSDPG23_H
#define MAME_MACHINE_ALPSDPG23_H

#pragma once

#include "machine/bitmap_printer.h"
#include "machine/steppers.h"


class alps_dpg23_device : public device_t
{
public:
	alps_dpg23_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	void motor_w(u8 data);
	void pen_w(u8 data);
	int sensor_r() const { return m_sensor; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	static constexpr int TEETH_PER_REV = 12;
	static constexpr int TEETH_PER_PEN = 3;
	static constexpr int BAY_SLOP = 2;

	void step_x(int delta);
	void set_sensor(bool found);
	void draw();

	required_device<bitmap_printer_device> m_bitmap_printer;
	required_device<stepper_device> m_cr_stepper;
	required_device<stepper_device> m_pf_stepper;

	u8 m_pd_data;
	u8 m_pc_data;
	bool m_pen_down;
	int m_x_dir;
	bool m_y_homed;
	int m_ratchet;
	int m_sensor;
};

DECLARE_DEVICE_TYPE(ALPS_DPG23, alps_dpg23_device)

#endif // MAME_MACHINE_ALPSDPG23_H
