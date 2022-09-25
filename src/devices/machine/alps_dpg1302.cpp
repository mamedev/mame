// license:BSD-3-Clause
// copyright-holders: Fabio Dalla Libera

#include "machine/alps_dpg1302.h"

/*
  The pen changing mechanism is explained in this video https://www.youtube.com/watch?v=TuMkl_ftuNM
*/

alps_dpg1302_plotter_device::alps_dpg1302_plotter_device (const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock):
	paper_roll_plotter_device(mconfig, ALPS_DPG1302, tag, owner,
                                  Element(600,450,0xc3b4bb),
 				  Element(500,350,0xFFFFFF),
 				  Position(50,50),
 				  Position(550,175),
				  -0.5, -0.5),
	m_pencarrierphase(0),m_prevx(0)
{
	set_headcolor(PENCOLOR[0]);
}

void alps_dpg1302_plotter_device::update_motors(uint8_t xpattern,uint8_t ypattern)
{
	paper_roll_plotter_device::update_motors(xpattern, ypattern);
	int pos=get_xmotor_pos();
	if (m_prevx<METALTABPOS && pos>=METALTABPOS)
	{
		m_pencarrierphase = (m_pencarrierphase+1)%12;
		if (m_pencarrierphase%3 == 0)
		{
			set_pencolor(PENCOLOR[m_pencarrierphase/3]);
			set_headcolor(PENCOLOR[m_pencarrierphase/3]);
		}
	}
	m_prevx=pos;
}

int alps_dpg1302_plotter_device::get_reedswitch_state()
{
	return (m_pencarrierphase==0) && (get_xmotor_pos()>METALTABPOS) ? 0 : 1;
}

DEFINE_DEVICE_TYPE(ALPS_DPG1302, alps_dpg1302_plotter_device, "alps_dpg1302", "ALPS DPG1302")
