// license:BSD-3-Clause
// copyright-holders:Bart Eversdijk
/**********************************************************************

    P2000 MW102 Centronics Interface Cartridge
        Port    
           4e   Centronics DATA port 
           4f   Centronics status port

**********************************************************************/

#ifndef MAME_BUS_P2000_CENTRONICS_H
#define MAME_BUS_P2000_CENTRONICS_H

#pragma once

#include "bus/p2000/exp.h"
#include "bus/centronics/ctronics.h"

//**************************************************************************
//  P2000 MW102 Centronics Interface Cartridge
//**************************************************************************

class p2000_mw102_centronics_device :
	public device_t,
	public device_p2000_expansion_slot_card_interface
{

public:
	// construction/destruction
	p2000_mw102_centronics_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
    
protected:
	// device-level overrides
	virtual void device_start() override;
    virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

    void port_4e_w(uint8_t data);
    uint8_t port_4f_r();
    void port_4f_w(uint8_t data);

    required_device<centronics_device> m_centronics; 
    DECLARE_WRITE_LINE_MEMBER(centronics_busy_w);
    DECLARE_WRITE_LINE_MEMBER(centronics_paper_empty_w);

private:
    uint8_t m_centronics_status = 0;

};


//**************************************************************************
//  P2000 P2000gg Centronics Interface Cartridge
//**************************************************************************

class p2000_p2gg_centronics_device :
	public device_t,
	public device_p2000_expansion_slot_card_interface
{

public:
	// construction/destruction
	p2000_p2gg_centronics_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
    
protected:
	// device-level overrides
	virtual void device_start() override;
    virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

    void port_46_w(uint8_t data);
    uint8_t port_47_r();
    
    required_device<centronics_device> m_centronics; 
    DECLARE_WRITE_LINE_MEMBER(centronics_error_w);
    DECLARE_WRITE_LINE_MEMBER(centronics_select_w);
    DECLARE_WRITE_LINE_MEMBER(centronics_paper_empty_w);
    DECLARE_WRITE_LINE_MEMBER(centronics_busy_w);
    DECLARE_WRITE_LINE_MEMBER(centronics_ack_w);

private:
    uint8_t m_centronics_status = 0;

};

//**************************************************************************
//  P2000 P2000gg Centronics Interface Cartridge
//**************************************************************************

class p2000_m2003_centronics_device : public p2000_p2gg_centronics_device
{

public:
	// construction/destruction
	p2000_m2003_centronics_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:  
	// device-level overrides
	virtual void device_start() override;

    void port_46_w(uint8_t data);
    void port_48_w(uint8_t data);
    uint8_t port_48_r();
    void port_49_w(uint8_t data);
    uint8_t port_49_r();
};

// device type definition

DECLARE_DEVICE_TYPE(P2000_M2003,      p2000_m2003_centronics_device)
DECLARE_DEVICE_TYPE(P2000_CENTRONICS, p2000_mw102_centronics_device)
DECLARE_DEVICE_TYPE(P2000_P2GGCENT,   p2000_p2gg_centronics_device)

#endif // MAME_BUS_P2000_CENTRONICS_H
