// license:BSD-3-Clause
// copyright-holders:Bart Eversdijk
/**********************************************************************

    P2000 PTCC Universal I/O Cartridge (UNIFACE)

            Port    
              60   Databus I/O
              61   Address / status bus
                    Status bits
                    0  0 = analog input print selected 
                       1 = digital I/Ointerface print present 
                    1  0 = digital OR analog input print selected 
                       1 = digital output print selected 
                    6  0 = interface print present
                    7  0 = interface print present

**********************************************************************/

#ifndef MAME_BUS_P2000_UNIFACE_H
#define MAME_BUS_P2000_UNIFACE_H

#pragma once

#include "bus/p2000/exp.h"
#include "machine/output_latch.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

//**************************************************************************
//  P2000 Universal I/O Cartridge (UNIFACE)
//**************************************************************************

class p2000_uniface_device :
	public device_t,
	public device_p2000_expansion_slot_card_interface
{

public:
	// construction/destruction
	p2000_uniface_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
    
protected:
	// device-level overrides
	virtual void device_start() override;
    virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
    virtual ioport_constructor device_input_ports() const override;

    void port_60_w(uint8_t data);
    uint8_t port_60_r();
    
    void port_61_w(uint8_t data);
    uint8_t port_61_r();

    required_device<output_latch_device> m_address_bus;
    required_device<output_latch_device> m_data_out;
    required_ioport m_data_in;
    required_ioport m_status_bus;
};


// device type definition
DECLARE_DEVICE_TYPE(P2000_UNIFACE, p2000_uniface_device)

#endif // MAME_BUS_P2000_UNIFACE_H
