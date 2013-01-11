/**********************************************************************

    Wang Professional Computer bus emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************


**********************************************************************/

#pragma once

#ifndef __WANGPC_BUS__
#define __WANGPC_BUS__

#include "emu.h"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define WANGPC_BUS_TAG      "wangpcbus"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_WANGPC_BUS_ADD(_config) \
	MCFG_DEVICE_ADD(WANGPC_BUS_TAG, WANGPC_BUS, 0) \
	MCFG_DEVICE_CONFIG(_config)


#define WANGPC_BUS_INTERFACE(_name) \
	const wangpcbus_interface (_name) =


#define MCFG_WANGPC_BUS_SLOT_ADD(_tag, _sid, _slot_intf, _def_slot, _def_inp) \
	MCFG_DEVICE_ADD(_tag, WANGPC_BUS_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, _def_inp, false) \
	wangpcbus_slot_device::static_set_wangpcbus_slot(*device, _sid);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> wangpcbus_slot_device

class wangpcbus_device;

class wangpcbus_slot_device : public device_t,
								public device_slot_interface
{
public:
	// construction/destruction
	wangpcbus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();

	// inline configuration
	static void static_set_wangpcbus_slot(device_t &device, int sid);

private:
	// configuration
	wangpcbus_device  *m_bus;
	int m_sid;
};


// device type definition
extern const device_type WANGPC_BUS_SLOT;


// ======================> wangpcbus_interface

struct wangpcbus_interface
{
	devcb_write_line    m_out_irq2_cb;
	devcb_write_line    m_out_irq3_cb;
	devcb_write_line    m_out_irq4_cb;
	devcb_write_line    m_out_irq5_cb;
	devcb_write_line    m_out_irq6_cb;
	devcb_write_line    m_out_irq7_cb;
	devcb_write_line    m_out_drq1_cb;
	devcb_write_line    m_out_drq2_cb;
	devcb_write_line    m_out_drq3_cb;
	devcb_write_line    m_out_ioerror_cb;
};

class device_wangpcbus_card_interface;


// ======================> wangpcbus_device

class wangpcbus_device : public device_t,
							public wangpcbus_interface
{
public:
	// construction/destruction
	wangpcbus_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	void add_wangpcbus_card(device_wangpcbus_card_interface *card, int sid);

	// computer interface
	DECLARE_READ16_MEMBER( mrdc_r );
	DECLARE_WRITE16_MEMBER( amwc_w );

	DECLARE_READ16_MEMBER( sad_r );
	DECLARE_WRITE16_MEMBER( sad_w );

	UINT8 dack_r(address_space &space, int line);
	void dack_w(address_space &space, int line, UINT8 data);

	DECLARE_READ8_MEMBER( dack0_r );
	DECLARE_WRITE8_MEMBER( dack0_w );
	DECLARE_READ8_MEMBER( dack1_r );
	DECLARE_WRITE8_MEMBER( dack1_w );
	DECLARE_READ8_MEMBER( dack2_r );
	DECLARE_WRITE8_MEMBER( dack2_w );
	DECLARE_READ8_MEMBER( dack3_r );
	DECLARE_WRITE8_MEMBER( dack3_w );

	DECLARE_WRITE_LINE_MEMBER( tc_w );

	// peripheral interface
	DECLARE_WRITE_LINE_MEMBER( irq2_w );
	DECLARE_WRITE_LINE_MEMBER( irq3_w );
	DECLARE_WRITE_LINE_MEMBER( irq4_w );
	DECLARE_WRITE_LINE_MEMBER( irq5_w );
	DECLARE_WRITE_LINE_MEMBER( irq6_w );
	DECLARE_WRITE_LINE_MEMBER( irq7_w );
	DECLARE_WRITE_LINE_MEMBER( drq1_w );
	DECLARE_WRITE_LINE_MEMBER( drq2_w );
	DECLARE_WRITE_LINE_MEMBER( drq3_w );
	DECLARE_WRITE_LINE_MEMBER( ioerror_w );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete();

private:
	devcb_resolved_write_line   m_out_irq2_func;
	devcb_resolved_write_line   m_out_irq3_func;
	devcb_resolved_write_line   m_out_irq4_func;
	devcb_resolved_write_line   m_out_irq5_func;
	devcb_resolved_write_line   m_out_irq6_func;
	devcb_resolved_write_line   m_out_irq7_func;
	devcb_resolved_write_line   m_out_drq1_func;
	devcb_resolved_write_line   m_out_drq2_func;
	devcb_resolved_write_line   m_out_drq3_func;
	devcb_resolved_write_line   m_out_ioerror_func;

	simple_list<device_wangpcbus_card_interface> m_device_list;
};


// device type definition
extern const device_type WANGPC_BUS;


// ======================> device_wangpcbus_card_interface

// class representing interface-specific live wangpcbus card
class device_wangpcbus_card_interface : public device_slot_card_interface
{
	friend class wangpcbus_device;

public:
	// construction/destruction
	device_wangpcbus_card_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_wangpcbus_card_interface();

	device_wangpcbus_card_interface *next() const { return m_next; }

	// memory access
	virtual UINT16 wangpcbus_mrdc_r(address_space &space, offs_t offset, UINT16 mem_mask) { return 0; };
	virtual void wangpcbus_amwc_w(address_space &space, offs_t offset, UINT16 mem_mask, UINT16 data) { };

	// I/O access
	virtual UINT16 wangpcbus_iorc_r(address_space &space, offs_t offset, UINT16 mem_mask) { return 0; };
	virtual void wangpcbus_aiowc_w(address_space &space, offs_t offset, UINT16 mem_mask, UINT16 data) { };
	bool sad(offs_t offset) { return ((offset & 0xf80) == (0x800 | (m_sid << 7))) ? true : false; }

	// DMA
	virtual UINT8 wangpcbus_dack_r(address_space &space, int line) { return 0; }
	virtual void wangpcbus_dack_w(address_space &space, int line, UINT8 data) { }
	virtual void wangpcbus_tc_w(int state) { }
	virtual bool wangpcbus_have_dack(int line) { return false; }

	wangpcbus_device  *m_bus;
	wangpcbus_slot_device *m_slot;

	int m_sid;
	device_wangpcbus_card_interface *m_next;
};

#endif
