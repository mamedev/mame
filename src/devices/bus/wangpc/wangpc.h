// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Wang Professional Computer bus emulation

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

#define MCFG_WANGPC_BUS_ADD() \
	MCFG_DEVICE_ADD(WANGPC_BUS_TAG, WANGPC_BUS, 0)

#define MCFG_WANGPC_BUS_SLOT_ADD(_tag, _sid, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, WANGPC_BUS_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false) \
	wangpcbus_slot_device::static_set_wangpcbus_slot(*device, _sid);


#define MCFG_WANGPC_BUS_IRQ2_CALLBACK(_write) \
	devcb = &wangpcbus_device::set_irq2_wr_callback(*device, DEVCB_##_write);

#define MCFG_WANGPC_BUS_IRQ3_CALLBACK(_write) \
	devcb = &wangpcbus_device::set_irq3_wr_callback(*device, DEVCB_##_write);

#define MCFG_WANGPC_BUS_IRQ4_CALLBACK(_write) \
	devcb = &wangpcbus_device::set_irq4_wr_callback(*device, DEVCB_##_write);

#define MCFG_WANGPC_BUS_IRQ5_CALLBACK(_write) \
	devcb = &wangpcbus_device::set_irq5_wr_callback(*device, DEVCB_##_write);

#define MCFG_WANGPC_BUS_IRQ6_CALLBACK(_write) \
	devcb = &wangpcbus_device::set_irq6_wr_callback(*device, DEVCB_##_write);

#define MCFG_WANGPC_BUS_IRQ7_CALLBACK(_write) \
	devcb = &wangpcbus_device::set_irq7_wr_callback(*device, DEVCB_##_write);

#define MCFG_WANGPC_BUS_DRQ1_CALLBACK(_write) \
	devcb = &wangpcbus_device::set_drq1_wr_callback(*device, DEVCB_##_write);

#define MCFG_WANGPC_BUS_DRQ2_CALLBACK(_write) \
	devcb = &wangpcbus_device::set_drq2_wr_callback(*device, DEVCB_##_write);

#define MCFG_WANGPC_BUS_DRQ3_CALLBACK(_write) \
	devcb = &wangpcbus_device::set_drq3_wr_callback(*device, DEVCB_##_write);

#define MCFG_WANGPC_BUS_IOERROR_CALLBACK(_write) \
	devcb = &wangpcbus_device::set_ioerror_wr_callback(*device, DEVCB_##_write);


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
	wangpcbus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;

	// inline configuration
	static void static_set_wangpcbus_slot(device_t &device, int sid);

private:
	// configuration
	wangpcbus_device  *m_bus;
	int m_sid;
};


// device type definition
extern const device_type WANGPC_BUS_SLOT;


class device_wangpcbus_card_interface;


// ======================> wangpcbus_device

class wangpcbus_device : public device_t
{
public:
	// construction/destruction
	wangpcbus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~wangpcbus_device() { m_device_list.detach_all(); }

	template<class _Object> static devcb_base &set_irq2_wr_callback(device_t &device, _Object object) { return downcast<wangpcbus_device &>(device).m_write_irq2.set_callback(object); }
	template<class _Object> static devcb_base &set_irq3_wr_callback(device_t &device, _Object object) { return downcast<wangpcbus_device &>(device).m_write_irq3.set_callback(object); }
	template<class _Object> static devcb_base &set_irq4_wr_callback(device_t &device, _Object object) { return downcast<wangpcbus_device &>(device).m_write_irq4.set_callback(object); }
	template<class _Object> static devcb_base &set_irq5_wr_callback(device_t &device, _Object object) { return downcast<wangpcbus_device &>(device).m_write_irq5.set_callback(object); }
	template<class _Object> static devcb_base &set_irq6_wr_callback(device_t &device, _Object object) { return downcast<wangpcbus_device &>(device).m_write_irq6.set_callback(object); }
	template<class _Object> static devcb_base &set_irq7_wr_callback(device_t &device, _Object object) { return downcast<wangpcbus_device &>(device).m_write_irq7.set_callback(object); }
	template<class _Object> static devcb_base &set_drq1_wr_callback(device_t &device, _Object object) { return downcast<wangpcbus_device &>(device).m_write_drq1.set_callback(object); }
	template<class _Object> static devcb_base &set_drq2_wr_callback(device_t &device, _Object object) { return downcast<wangpcbus_device &>(device).m_write_drq2.set_callback(object); }
	template<class _Object> static devcb_base &set_drq3_wr_callback(device_t &device, _Object object) { return downcast<wangpcbus_device &>(device).m_write_drq3.set_callback(object); }
	template<class _Object> static devcb_base &set_ioerror_wr_callback(device_t &device, _Object object) { return downcast<wangpcbus_device &>(device).m_write_ioerror.set_callback(object); }

	void add_card(device_wangpcbus_card_interface *card, int sid);

	// computer interface
	uint16_t mrdc_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void amwc_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	uint16_t sad_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void sad_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	uint8_t dack_r(address_space &space, int line);
	void dack_w(address_space &space, int line, uint8_t data);

	uint8_t dack0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) { return dack_r(space, 0); }
	void dack0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) { dack_w(space, 0, data); }
	uint8_t dack1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) { return dack_r(space, 1); }
	void dack1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) { dack_w(space, 1, data); }
	uint8_t dack2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) { return dack_r(space, 2); }
	void dack2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) { dack_w(space, 2, data); }
	uint8_t dack3_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) { return dack_r(space, 3); }
	void dack3_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) { dack_w(space, 3, data); }

	void tc_w(int state);

	// peripheral interface
	void irq2_w(int state) { m_write_irq2(state); }
	void irq3_w(int state) { m_write_irq3(state); }
	void irq4_w(int state) { m_write_irq4(state); }
	void irq5_w(int state) { m_write_irq5(state); }
	void irq6_w(int state) { m_write_irq6(state); }
	void irq7_w(int state) { m_write_irq7(state); }
	void drq1_w(int state) { m_write_drq1(state); }
	void drq2_w(int state) { m_write_drq2(state); }
	void drq3_w(int state) { m_write_drq3(state); }
	void ioerror_w(int state) { m_write_ioerror(state); }

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	devcb_write_line   m_write_irq2;
	devcb_write_line   m_write_irq3;
	devcb_write_line   m_write_irq4;
	devcb_write_line   m_write_irq5;
	devcb_write_line   m_write_irq6;
	devcb_write_line   m_write_irq7;
	devcb_write_line   m_write_drq1;
	devcb_write_line   m_write_drq2;
	devcb_write_line   m_write_drq3;
	devcb_write_line   m_write_ioerror;

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
	virtual ~device_wangpcbus_card_interface() { }

	device_wangpcbus_card_interface *next() const { return m_next; }

	// memory access
	virtual uint16_t wangpcbus_mrdc_r(address_space &space, offs_t offset, uint16_t mem_mask) { return 0; };
	virtual void wangpcbus_amwc_w(address_space &space, offs_t offset, uint16_t mem_mask, uint16_t data) { };

	// I/O access
	virtual uint16_t wangpcbus_iorc_r(address_space &space, offs_t offset, uint16_t mem_mask) { return 0; };
	virtual void wangpcbus_aiowc_w(address_space &space, offs_t offset, uint16_t mem_mask, uint16_t data) { };
	bool sad(offs_t offset) { return ((offset & 0xf80) == (0x800 | (m_sid << 7))) ? true : false; }

	// DMA
	virtual uint8_t wangpcbus_dack_r(address_space &space, int line) { return 0; }
	virtual void wangpcbus_dack_w(address_space &space, int line, uint8_t data) { }
	virtual void wangpcbus_tc_w(int state) { }
	virtual bool wangpcbus_have_dack(int line) { return false; }

	wangpcbus_device  *m_bus;
	wangpcbus_slot_device *m_slot;

	int m_sid;
	device_wangpcbus_card_interface *m_next;
};


SLOT_INTERFACE_EXTERN( wangpc_cards );



#endif
