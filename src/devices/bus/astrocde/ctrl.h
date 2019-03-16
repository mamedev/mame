// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
#ifndef MAME_BUS_ASTROCDE_CTRL_H
#define MAME_BUS_ASTROCDE_CTRL_H

#pragma once


/***************************************************************************
 FORWARD DECLARATIONS
 ***************************************************************************/

class astrocade_ctrl_port_device;


/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/

// ======================> device_astrocade_ctrl_interface

class device_astrocade_ctrl_interface : public device_slot_card_interface
{
public:
	virtual ~device_astrocade_ctrl_interface();

	virtual uint8_t read_handle() { return 0; }
	virtual uint8_t read_knob() { return 0; }

protected:
	device_astrocade_ctrl_interface(machine_config const &mconfig, device_t &device);

	// device_interface implementation
	virtual void interface_validity_check(validity_checker &valid) const override ATTR_COLD;
	virtual void interface_pre_start() override;

private:
	astrocade_ctrl_port_device *const m_port;

	friend class astrocade_ctrl_port_device;
};


// ======================> astrocade_ctrl_port_device

class astrocade_ctrl_port_device : public device_t, public device_slot_interface
{
public:
	// construction/destruction
	template <typename T>
	astrocade_ctrl_port_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: astrocade_ctrl_port_device(mconfig, tag, owner, 0U)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	astrocade_ctrl_port_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock = 0U);
	virtual ~astrocade_ctrl_port_device();

	uint8_t read_handle();
	uint8_t read_knob();

protected:
	// device_t implementation
	virtual void device_validity_check(validity_checker &valid) const override ATTR_COLD;
	virtual void device_resolve_objects() override;
	virtual void device_start() override;

private:
	device_astrocade_ctrl_interface *m_device;

	friend class device_astrocade_ctrl_interface;
};


/***************************************************************************
 FUNCTIONS
 ***************************************************************************/

void astrocade_controllers(device_slot_interface &device);


/***************************************************************************
 DEVICE TYPES
 ***************************************************************************/

DECLARE_DEVICE_TYPE(ASTROCADE_CTRL_PORT, astrocade_ctrl_port_device)

#endif // MAME_BUS_ASTROCDE_CTRL_H
