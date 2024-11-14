// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    RCA Cosmac VIP Byte Input/Output port emulation

**********************************************************************

                            A   IN 0
                            B   IN 1
                            C   IN 2
                            D   IN 3
                            E   IN 4
                            F   IN 5
                            H   IN 6
                            J   IN 7
                            K   INST
                            L   _EF4
                            M   OUT 0
                            N   OUT 1
                            P   OUT 2
                            R   OUT 3
                            S   OUT 4
                            T   OUT 5
                            U   OUT 6
                            V   OUT 7
                            W   Q
                            X   _EF3
                            Y   +5 V
                            Z   GND

**********************************************************************/

#ifndef MAME_BUS_VIP_BYTEIO_H
#define MAME_BUS_VIP_BYTEIO_H

#pragma once



//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define VIP_BYTEIO_PORT_TAG     "byteio"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vip_byteio_port_device

class device_vip_byteio_port_interface;

class vip_byteio_port_device : public device_t, public device_single_card_slot_interface<device_vip_byteio_port_interface>
{
public:
	// construction/destruction
	template <typename T>
	vip_byteio_port_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: vip_byteio_port_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	vip_byteio_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto inst_callback() { return m_write_inst.bind(); }

	// computer interface
	uint8_t in_r();
	void out_w(uint8_t data);
	int ef3_r();
	int ef4_r();
	void q_w(int state);

	// cartridge interface
	void inst_w(int state) { m_write_inst(state); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	devcb_write_line m_write_inst;

	device_vip_byteio_port_interface *m_cart;
};


// ======================> device_vip_byteio_port_interface

// class representing interface-specific live c64_expansion card
class device_vip_byteio_port_interface : public device_interface
{
public:
	virtual uint8_t vip_in_r() { return 0xff; }
	virtual void vip_out_w(uint8_t data) { }

	virtual int vip_ef3_r() { return CLEAR_LINE; }
	virtual int vip_ef4_r() { return CLEAR_LINE; }

	virtual void vip_q_w(int state) { }

protected:
	// construction/destruction
	device_vip_byteio_port_interface(const machine_config &mconfig, device_t &device);

	vip_byteio_port_device *m_slot;
};


// device type definition
DECLARE_DEVICE_TYPE(VIP_BYTEIO_PORT, vip_byteio_port_device)


// slot devices
#include "vp620.h"

void vip_byteio_cards(device_slot_interface &device);

#endif // MAME_BUS_VIP_BYTEIO_H
