// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

   Mattel Intellivision ECS hack for controller port emulation

**********************************************************************/

#ifndef MAME_BUS_INTV_CTRL_ECS_CTRL_H
#define MAME_BUS_INTV_CTRL_ECS_CTRL_H

#pragma once


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
	virtual ~device_intvecs_control_port_interface();

	virtual uint8_t read_portA() { return 0xff; }
	virtual uint8_t read_portB() { return 0xff; }
	virtual void write_portA(uint8_t data) { }

protected:
	device_intvecs_control_port_interface(const machine_config &mconfig, device_t &device);

	intvecs_control_port_device *m_port;
};

// ======================> intvecs_control_port_device

class intvecs_control_port_device : public device_t,
								public device_slot_interface
{
public:
	// construction/destruction
	template <typename T>
	intvecs_control_port_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: intvecs_control_port_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	intvecs_control_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~intvecs_control_port_device();

	DECLARE_READ8_MEMBER( portA_r ) { return read_portA(); }
	DECLARE_READ8_MEMBER( portB_r ) { return read_portB(); }
	DECLARE_WRITE8_MEMBER( portA_w ) { return write_portA(data); }

protected:
	// device-level overrides
	virtual void device_start() override;
	uint8_t read_portA();
	uint8_t read_portB();
	void write_portA(uint8_t data);

	device_intvecs_control_port_interface *m_device;
};


// device type definition
DECLARE_DEVICE_TYPE(INTVECS_CONTROL_PORT, intvecs_control_port_device)


void intvecs_control_port_devices(device_slot_interface &device);


//**************************************************************************
//  ACTUAL SLOT DEVICES - included here until core issues are solved...
//**************************************************************************

// ======================> intvecs_ctrls_device

class intvecs_ctrls_device : public device_t,
						public device_intvecs_control_port_interface
{
public:
	// construction/destruction
	intvecs_ctrls_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;

	virtual uint8_t read_portA() override;
	virtual uint8_t read_portB() override;

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
	intvecs_keybd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual uint8_t read_portB() override;
	virtual void write_portA(uint8_t data) override;

private:
	uint8_t m_psg_portA;
	required_ioport_array<7> m_keybd;
};

// ======================> intvecs_synth_device

class intvecs_synth_device : public device_t,
						public device_intvecs_control_port_interface
{
public:
	// construction/destruction
	intvecs_synth_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual uint8_t read_portB() override;
	virtual void write_portA(uint8_t data) override;

private:
	uint8_t m_psg_portA;
	required_ioport_array<7> m_synth;
};


// device type definition
DECLARE_DEVICE_TYPE(ECS_CTRLS, intvecs_ctrls_device)
DECLARE_DEVICE_TYPE(ECS_KEYBD, intvecs_keybd_device)
DECLARE_DEVICE_TYPE(ECS_SYNTH, intvecs_synth_device)


#endif // MAME_BUS_INTV_CTRL_ECS_CTRL_H
