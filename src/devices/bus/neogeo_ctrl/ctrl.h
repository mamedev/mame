// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

   SNK Neo Geo controller port emulation

**********************************************************************/
#ifndef MAME_BUS_NEOGEO_CTRL_CTRL_H
#define MAME_BUS_NEOGEO_CTRL_CTRL_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class neogeo_control_port_device;
class neogeo_ctrl_edge_port_device;

// ======================> device_neogeo_control_port_interface

class device_neogeo_control_port_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	virtual ~device_neogeo_control_port_interface();

	virtual uint8_t read_ctrl() { return 0xff; }
	virtual uint8_t read_start_sel() { return 0xff; }
	virtual void write_ctrlsel(uint8_t data) { }

protected:
	device_neogeo_control_port_interface(const machine_config &mconfig, device_t &device);

	neogeo_control_port_device *m_port;
};

// ======================> neogeo_control_port_device

class neogeo_control_port_device : public device_t, public device_slot_interface
{
public:
	// construction/destruction
	neogeo_control_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~neogeo_control_port_device();

	uint8_t read_ctrl();
	uint8_t read_start_sel();
	void write_ctrlsel(uint8_t data);
	DECLARE_READ8_MEMBER( ctrl_r ) { return read_ctrl(); }

protected:
	// device-level overrides
	virtual void device_start() override;

	device_neogeo_control_port_interface *m_device;
};


// ======================> device_neogeo_ctrl_edge_interface

class device_neogeo_ctrl_edge_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	virtual ~device_neogeo_ctrl_edge_interface();

	virtual uint8_t read_start_sel() { return 0xff; }
	virtual DECLARE_READ8_MEMBER( in0_r ) { return 0xff; }
	virtual DECLARE_READ8_MEMBER( in1_r ) { return 0xff; }
	virtual void write_ctrlsel(uint8_t data) { }

protected:
	device_neogeo_ctrl_edge_interface(const machine_config &mconfig, device_t &device);

	neogeo_ctrl_edge_port_device *m_port;
};

// ======================> neogeo_ctrl_edge_port_device

class neogeo_ctrl_edge_port_device : public device_t, public device_slot_interface
{
public:
	// construction/destruction
	neogeo_ctrl_edge_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~neogeo_ctrl_edge_port_device();

	uint8_t read_start_sel();
	DECLARE_READ8_MEMBER( in0_r );
	DECLARE_READ8_MEMBER( in1_r );
	void write_ctrlsel(uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override;

	device_neogeo_ctrl_edge_interface *m_device;
};


// device type definition
DECLARE_DEVICE_TYPE(NEOGEO_CONTROL_PORT,        neogeo_control_port_device)
DECLARE_DEVICE_TYPE(NEOGEO_CTRL_EDGE_CONNECTOR, neogeo_ctrl_edge_port_device)


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_NEOGEO_CONTROL_PORT_ADD(_tag, _slot_intf, _def_slot, _fixed) \
	MCFG_DEVICE_ADD(_tag, NEOGEO_CONTROL_PORT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, _fixed)

#define MCFG_NEOGEO_CONTROL_EDGE_CONNECTOR_ADD(_tag, _slot_intf, _def_slot, _fixed) \
	MCFG_DEVICE_ADD(_tag, NEOGEO_CTRL_EDGE_CONNECTOR, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, _fixed)



void neogeo_controls(device_slot_interface &device);
void neogeo_arc_edge(device_slot_interface &device);
void neogeo_arc_edge_fixed(device_slot_interface &device);
void neogeo_arc_pin15(device_slot_interface &device);


#endif // MAME_BUS_NEOGEO_CTRL_CTRL_H
