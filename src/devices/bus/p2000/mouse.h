// license:BSD-3-Clause
// copyright-holders:Bart Eversdijk
/**********************************************************************

    P2000 MSX Mouse cartridge (Stichting Computer Creatief)
        44-47       44: PIO A DATA status channel 
                    45: PIO A ctrl status channel 
                    46: PIO B DATA status channel 
                    47: PIO B ctrl status channel 

**********************************************************************/

#ifndef MAME_BUS_P2000_MOUSE_H
#define MAME_BUS_P2000_MOUSE_H

#pragma once

#include "bus/p2000/exp.h"
#include "machine/z80pio.h"
#include "machine/timer.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class p2000_mouse_device :
	public device_t,
	public device_p2000_expansion_slot_card_interface
{

public:
	// construction/destruction
	p2000_mouse_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
    
protected:
	// device-level overrides
	virtual void device_start() override;
	
	// optional information overrides
    virtual ioport_constructor device_input_ports() const override;
	virtual void device_add_mconfig(machine_config &config) override;

    // PIO callbacks
    uint8_t pa_r_cb();
    void pa_w_cb(uint8_t data);
    uint8_t pb_r_cb();
    void pb_w_cb(uint8_t data);

    required_device<z80pio_device> m_mousepio;
    required_ioport m_io_mouse_b;
	required_ioport m_io_mouse_x;
	required_ioport m_io_mouse_y;

private:
    TIMER_DEVICE_CALLBACK_MEMBER(mouse_timer_cb);
    
	uint8_t m_channel_b_data;

    /* mouse */
    uint8_t m_mouse_b;
    uint8_t m_mouse_x;
	uint8_t m_mouse_y;
    
    uint8_t m_mouse_last_x;
    uint8_t m_mouse_last_y;
	int m_mouse_status;
};


// device type definition
DECLARE_DEVICE_TYPE(P2000_MOUSE, p2000_mouse_device)


#endif // MAME_BUS_P2000_MOUSE_H
