// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

   Mattel Intellivision ECS hack for controller port emulation

**********************************************************************/


#pragma once

#ifndef __INTVECS_CONTROL_PORT__
#define __INTVECS_CONTROL_PORT__

#include "emu.h"
#include "bus/intv_ctrl/ctrl.h"
#include "bus/intv_ctrl/handctrl.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class intvecs_control_port_device;

// ======================> device_intvecs_control_port_interface

class device_intvecs_control_port_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_intvecs_control_port_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_intvecs_control_port_interface();

	virtual UINT8 read_portA() { return 0xff; };
    virtual UINT8 read_portB() { return 0xff; };
    virtual void write_portA(UINT8 data) { };

protected:
	intvecs_control_port_device *m_port;
};

// ======================> intvecs_control_port_device

class intvecs_control_port_device : public device_t,
								public device_slot_interface
{
public:
	// construction/destruction
	intvecs_control_port_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~intvecs_control_port_device();

    DECLARE_READ8_MEMBER( portA_r ) { return read_portA(); }
    DECLARE_READ8_MEMBER( portB_r ) { return read_portB(); }
    DECLARE_WRITE8_MEMBER( portA_w ) { return write_portA(data); }

protected:
	// device-level overrides
	virtual void device_start() override;
    UINT8 read_portA();
    UINT8 read_portB();
    void write_portA(UINT8 data);

	device_intvecs_control_port_interface *m_device;
};


// device type definition
extern const device_type INTVECS_CONTROL_PORT;


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_INTVECS_CONTROL_PORT_ADD(_tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, INTVECS_CONTROL_PORT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)



SLOT_INTERFACE_EXTERN( intvecs_control_port_devices );


//**************************************************************************
//  ACTUAL SLOT DEVICES - included here until core issues are solved...
//**************************************************************************

// ======================> intvecs_ctrls_device

class intvecs_ctrls_device : public device_t,
                        public device_intvecs_control_port_interface
{
public:
    // construction/destruction
    intvecs_ctrls_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

    // optional information overrides
    virtual machine_config_constructor device_mconfig_additions() const override;

protected:
    // device-level overrides
    virtual void device_start() override;
    virtual void device_reset() override;
    
    virtual UINT8 read_portA() override;
    virtual UINT8 read_portB() override;
    
private:
    required_device<intv_control_port_device> m_hand1;
    required_device<intv_control_port_device> m_hand2;
};

// ======================> intvecs_keybd_device

class intvecs_keybd_device : public device_t,
                        public device_intvecs_control_port_interface
{
public:
    // construction/destruction
    intvecs_keybd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
    
    // optional information overrides
    virtual ioport_constructor device_input_ports() const override;
    
protected:
    // device-level overrides
    virtual void device_start() override;
    virtual void device_reset() override;
    
    virtual UINT8 read_portB() override;
    virtual void write_portA(UINT8 data) override;
    
private:
    UINT8 m_psg_portA;
    required_ioport_array<7> m_keybd;
};

// ======================> intvecs_synth_device

class intvecs_synth_device : public device_t,
                        public device_intvecs_control_port_interface
{
public:
    // construction/destruction
    intvecs_synth_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
    
    // optional information overrides
    virtual ioport_constructor device_input_ports() const override;
    
protected:
    // device-level overrides
    virtual void device_start() override;
    virtual void device_reset() override;
    
    virtual UINT8 read_portB() override;
    virtual void write_portA(UINT8 data) override;
    
private:
    UINT8 m_psg_portA;
    required_ioport_array<7> m_synth;
};


// device type definition
extern const device_type ECS_CTRLS;
extern const device_type ECS_KEYBD;
extern const device_type ECS_SYNTH;



#endif
