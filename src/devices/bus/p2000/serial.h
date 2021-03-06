// license:BSD-3-Clause
// copyright-holders:Bart Eversdijk
/**********************************************************************

    P2000 V.24 Serial cartridge(s)

        PTC   - V24 Interface

            Port    
              44   USART 8251 DATA port 
              45   USART 8251 command port

              60   freq. divider 0   (2.5 MHz / 2 / div 0 - RX clock)
              61   freq. divider 1   (2.5 MHz / 2 / div 1 - TX clock)
              62   freq. divider 2   N/A
              63   program divider 2

        M2001 - Miniware RS-232 Interface
        
            Port    
              40   freq. divider 0   global div.
              41   freq. divider 1   (2.5 MHz / div 0 / div 1 - RX clock)
              42   freq. divider 2   (2.5 MHz / div 0 / div 2 - TX clock)
              43   program divider 2
              44   USART 8251 DATA port 
              45   USART 8251 command port
        
        P2174 - Philips V.24/RS-232 Interface
        
            Port    
              40   USART 8251 DATA port 
              41   USART 8251 command port

              61   Input address port - not used
              62   Switch S2 port 

            SI-1     75
            SI-2    150
            SI-3    300
            SI-4    600
            SI-5   1200
            SI-6   2400
            SI-7   4800
            SI-8   9600

        P2171-1 Viewdadata Communication Interface Cartridge
        
            Port    
              40   USART 8251 DATA port 
              41   USART 8251 command port

              60-6f   Set status  

**********************************************************************/

#ifndef MAME_BUS_P2000_V24SERIAL_H
#define MAME_BUS_P2000_V24SERIAL_H

#pragma once

#include "bus/p2000/exp.h"
#include "machine/clock.h"
#include "machine/i8251.h"
#include "machine/pit8253.h"
#include "bus/rs232/rs232.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

//**************************************************************************
//  PTC V.24 Serial Interface Cartridge
//**************************************************************************

class p2000_v24serial_device : 
    public device_t,
	public device_p2000_expansion_slot_card_interface
{

public:
	// construction/destruction
	p2000_v24serial_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
    
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override;

    required_device<i8251_device> m_usart;
    required_device<pit8253_device> m_clkdivider;
};

//**************************************************************************
//  M2001 V.24 Serial Interface Cartridge
//**************************************************************************

class p2000_m2001_serial_device : 
    public device_t,
	public device_p2000_expansion_slot_card_interface
{

public:
	// construction/destruction
	p2000_m2001_serial_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override;

    required_device<i8251_device> m_usart;
    required_device<pit8253_device> m_clkdivider;
};

//**************************************************************************
//  P2174 V.24 Serial Interface Cartridge
//**************************************************************************

class p2000_p2174_serial_device : 
    public device_t,
	public device_p2000_expansion_slot_card_interface
{

public:
	// construction/destruction
	p2000_p2174_serial_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

    uint8_t port_60_r();
    uint8_t port_62_r();
    
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override;
    virtual ioport_constructor device_input_ports() const override;

    DECLARE_WRITE_LINE_MEMBER( usart_clock_tick );

	required_device<i8251_device> m_usart;
    required_device<rs232_port_device> m_rs232;
    required_ioport m_dsw1;
    required_ioport m_dsw2;
    uint8_t m_usart_divide_counter;
	uint8_t m_usart_clock_state;

};

//**************************************************************************
//  P2171-1 Viewdadata V.24 Serial Interface Cartridge
//**************************************************************************

class p2000_p2171_viewdata_serial_device : 
    public device_t,
	public device_p2000_expansion_slot_card_interface
{

public:
	// construction/destruction
	p2000_p2171_viewdata_serial_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

    void port_606f_w(uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override;

	required_device<i8251_device> m_usart;
    required_device<rs232_port_device> m_rs232;
    uint8_t m_usart_divide_counter;
    uint8_t m_usart_clock_state;

};


// device type definition
DECLARE_DEVICE_TYPE(P2000_P2174V24, p2000_p2174_serial_device)
DECLARE_DEVICE_TYPE(P2000_PTCV24,   p2000_v24serial_device)
DECLARE_DEVICE_TYPE(P2000_M2001V24, p2000_m2001_serial_device)
DECLARE_DEVICE_TYPE(P2000_VIEWDATA, p2000_p2171_viewdata_serial_device)

#endif // MAME_BUS_P2000_V24SERIAL_H
