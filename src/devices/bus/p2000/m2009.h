// license:BSD-3-Clause
// copyright-holders:Bart Eversdijk
/**********************************************************************

    P2000 Miniware M2009 Auto Dial/Answer modem Cartridge
        Port    
           40   freq. divider 0   global div.
           41   freq. divider 1   (2.5 MHz / div 0 / div 1 - RX clock)
           42   freq. divider 2   (2.5 MHz / div 0 / div 2 - TX clock)
           43   program divider 2
           44   USART 8251 DATA port 
           45   USART 8251 command port
        
**********************************************************************/

#ifndef MAME_BUS_P2000_M2009_H
#define MAME_BUS_P2000_M2009_H

#pragma once

#include "bus/p2000/exp.h"
#include "machine/z80scc.h"
#include "bus/rs232/rs232.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

//**************************************************************************
//  Miniware M2009 Autodial/Auto answer modem Cartridge
//**************************************************************************
/**********************************************************************
    Ports:
        40-43       Z8630 - Z80SCC (Serial Communication Controller)
                        40: cmd/status channlel B
                        41: data in/out channlel B
                        42: cmd/status channlel A
                        43: data in/out channlel B
        44-47       Phone Line control
                      bit 5:  Phone off hook (Line open)
                      bit 6:  Dial active (line short with resistor)  
                      bit 7:  Dial pulse  (line short)
        
    TODO: inplement AM7910(m_modem)
**********************************************************************/
class p2000_m2009_modem_device :
	public device_t,
	public device_p2000_expansion_slot_card_interface
{

public:
	// construction/destruction
	p2000_m2009_modem_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
    
protected:
	// device-level overrides
	virtual void device_start() override;
    virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

    TIMER_CALLBACK_MEMBER(dial_pulse_timer_cb);
    
    required_device<scc8530_device> m_scc;
    
    void port_40_w(uint8_t data);
    uint8_t port_40_r();
    void port_41_w(uint8_t data);
    uint8_t port_41_r();
    void port_42_w(uint8_t data);
    uint8_t port_42_r();
    void port_43_w(uint8_t data);
    uint8_t port_43_r();

    void port_44_w(uint8_t data);
    uint8_t port_44_r();

    void phone_on_hook();

private:

    bool m_sync_toggle = true;
    uint8_t m_port_44 = 0;
    uint8_t m_cha_reg = 0;
    
    const static unsigned int m_dialing_ready_delay = 2000;
    const static unsigned int m_number_size = 20;
    bool m_dial_in_progress = false;
    char m_phone_number[m_number_size+1] = "\0";
    uint8_t m_number_cnt = 0;
    emu_timer *m_dial_pulse_timer;

};

// device type definition
DECLARE_DEVICE_TYPE(P2000_M2009, p2000_m2009_modem_device)

#endif // MAME_BUS_P2000_M2009_H
