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


class device_neogeo_control_port_interface : public device_interface
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


class neogeo_control_port_device : public device_t, public device_single_card_slot_interface<device_neogeo_control_port_interface>
{
public:
	// construction/destruction
	template <typename T>
	neogeo_control_port_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt, bool const fixed)
		: neogeo_control_port_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(fixed);
	}
	neogeo_control_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~neogeo_control_port_device();

	uint8_t read_ctrl();
	uint8_t read_start_sel();
	void write_ctrlsel(uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	device_neogeo_control_port_interface *m_device;
};


class device_neogeo_ctrl_edge_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_neogeo_ctrl_edge_interface();

	virtual uint8_t read_start_sel() { return 0xff; }
	virtual uint8_t in0_r() { return 0xff; }
	virtual uint8_t in1_r() { return 0xff; }
	virtual void write_ctrlsel(uint8_t data) { }

protected:
	device_neogeo_ctrl_edge_interface(const machine_config &mconfig, device_t &device);

	neogeo_ctrl_edge_port_device *m_port;
};


class neogeo_ctrl_edge_port_device : public device_t, public device_single_card_slot_interface<device_neogeo_ctrl_edge_interface>
{
public:
	// construction/destruction
	template <typename T>
	neogeo_ctrl_edge_port_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt, bool const fixed)
		: neogeo_ctrl_edge_port_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(fixed);
	}
	neogeo_ctrl_edge_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~neogeo_ctrl_edge_port_device();

	uint8_t read_start_sel();
	uint8_t in0_r();
	uint8_t in1_r();
	void write_ctrlsel(uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	device_neogeo_ctrl_edge_interface *m_device;
};


DECLARE_DEVICE_TYPE(NEOGEO_CONTROL_PORT,        neogeo_control_port_device)
DECLARE_DEVICE_TYPE(NEOGEO_CTRL_EDGE_CONNECTOR, neogeo_ctrl_edge_port_device)


void neogeo_controls(device_slot_interface &device);
void neogeo_arc_edge(device_slot_interface &device);
void neogeo_arc_edge_irrmaze(device_slot_interface &device);
void neogeo_arc_edge_fixed(device_slot_interface &device);
void neogeo_arc_pin15(device_slot_interface &device);

#endif // MAME_BUS_NEOGEO_CTRL_CTRL_H
