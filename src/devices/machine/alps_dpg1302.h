// license:BSD-3-Clause
// copyright-holders: Fabio Dalla Libera
/*
  ALPS DPG1302 plotter.

*/

#include "machine/plotter.h"

#ifndef MAME_MACHINE_ALPS_DPG1302_H
#define MAME_MACHINE_ALPS_DPG1302_H

#pragma once

class alps_dpg1302_plotter_device:public paper_roll_plotter_device
{

public:

	alps_dpg1302_plotter_device (const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock=0);
	int get_reedswitch_state();
	void update_motors(uint8_t xpattern,uint8_t ypattern) override;

protected:

	uint8_t m_pencarrierphase;
	int m_prevx;
	static constexpr int METALTABPOS = 1100;
	static constexpr uint32_t PENCOLOR[4] = {0x00000, 0x0000FF, 0x008000, 0x800000};
};

DECLARE_DEVICE_TYPE(ALPS_DPG1302, alps_dpg1302_plotter_device)

#endif
