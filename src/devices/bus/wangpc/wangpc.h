// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Wang Professional Computer bus emulation

**********************************************************************


**********************************************************************/

#ifndef MAME_BUS_WANGPC_WANGPC_H
#define MAME_BUS_WANGPC_WANGPC_H

#pragma once




//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define WANGPC_BUS_TAG      "wangpcbus"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> wangpcbus_slot_device

class wangpcbus_device;

class wangpcbus_slot_device : public device_t, public device_slot_interface
{
public:
	// construction/destruction
	template <typename T>
	wangpcbus_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt, int sid)
		: wangpcbus_slot_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
		set_wangpcbus_slot(sid);
	}
	wangpcbus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// inline configuration
	void set_wangpcbus_slot(int sid) { m_sid = sid; }

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	// configuration
	wangpcbus_device  *m_bus;
	int m_sid;
};


// device type definition
DECLARE_DEVICE_TYPE(WANGPC_BUS_SLOT, wangpcbus_slot_device)


class device_wangpcbus_card_interface;


// ======================> wangpcbus_device

class wangpcbus_device : public device_t
{
public:
	// construction/destruction
	wangpcbus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~wangpcbus_device() { m_device_list.detach_all(); }

	auto irq2_wr_callback() { return m_write_irq2.bind(); }
	auto irq3_wr_callback() { return m_write_irq3.bind(); }
	auto irq4_wr_callback() { return m_write_irq4.bind(); }
	auto irq5_wr_callback() { return m_write_irq5.bind(); }
	auto irq6_wr_callback() { return m_write_irq6.bind(); }
	auto irq7_wr_callback() { return m_write_irq7.bind(); }
	auto drq1_wr_callback() { return m_write_drq1.bind(); }
	auto drq2_wr_callback() { return m_write_drq2.bind(); }
	auto drq3_wr_callback() { return m_write_drq3.bind(); }
	auto ioerror_wr_callback() { return m_write_ioerror.bind(); }

	void add_card(device_wangpcbus_card_interface *card, int sid);

	// computer interface
	uint16_t mrdc_r(offs_t offset, uint16_t mem_mask = 0xffff);
	void amwc_w(offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	uint16_t sad_r(offs_t offset, uint16_t mem_mask = 0xffff);
	void sad_w(offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	uint8_t dack_r(int line);
	void dack_w(int line, uint8_t data);

	uint8_t dack0_r() { return dack_r(0); }
	void dack0_w(uint8_t data) { dack_w(0, data); }
	uint8_t dack1_r() { return dack_r(1); }
	void dack1_w(uint8_t data) { dack_w(1, data); }
	uint8_t dack2_r() { return dack_r(2); }
	void dack2_w(uint8_t data) { dack_w(2, data); }
	uint8_t dack3_r() { return dack_r(3); }
	void dack3_w(uint8_t data) { dack_w(3, data); }

	DECLARE_WRITE_LINE_MEMBER( tc_w );

	// peripheral interface
	DECLARE_WRITE_LINE_MEMBER( irq2_w ) { m_write_irq2(state); }
	DECLARE_WRITE_LINE_MEMBER( irq3_w ) { m_write_irq3(state); }
	DECLARE_WRITE_LINE_MEMBER( irq4_w ) { m_write_irq4(state); }
	DECLARE_WRITE_LINE_MEMBER( irq5_w ) { m_write_irq5(state); }
	DECLARE_WRITE_LINE_MEMBER( irq6_w ) { m_write_irq6(state); }
	DECLARE_WRITE_LINE_MEMBER( irq7_w ) { m_write_irq7(state); }
	DECLARE_WRITE_LINE_MEMBER( drq1_w ) { m_write_drq1(state); }
	DECLARE_WRITE_LINE_MEMBER( drq2_w ) { m_write_drq2(state); }
	DECLARE_WRITE_LINE_MEMBER( drq3_w ) { m_write_drq3(state); }
	DECLARE_WRITE_LINE_MEMBER( ioerror_w ) { m_write_ioerror(state); }

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
DECLARE_DEVICE_TYPE(WANGPC_BUS, wangpcbus_device)


// ======================> device_wangpcbus_card_interface

// class representing interface-specific live wangpcbus card
class device_wangpcbus_card_interface : public device_slot_card_interface
{
	friend class wangpcbus_device;
	template <class ElementType> friend class simple_list;

public:
	device_wangpcbus_card_interface *next() const { return m_next; }

	// memory access
	virtual uint16_t wangpcbus_mrdc_r(offs_t offset, uint16_t mem_mask) { return 0; }
	virtual void wangpcbus_amwc_w(offs_t offset, uint16_t mem_mask, uint16_t data) { }

	// I/O access
	virtual uint16_t wangpcbus_iorc_r(offs_t offset, uint16_t mem_mask) { return 0; }
	virtual void wangpcbus_aiowc_w(offs_t offset, uint16_t mem_mask, uint16_t data) { }
	bool sad(offs_t offset) const { return (offset & 0xf80) == (0x800 | (m_sid << 7)); }

	// DMA
	virtual uint8_t wangpcbus_dack_r(int line) { return 0; }
	virtual void wangpcbus_dack_w(int line, uint8_t data) { }
	virtual void wangpcbus_tc_w(int state) { }
	virtual bool wangpcbus_have_dack(int line) { return false; }

protected:
	// construction/destruction
	device_wangpcbus_card_interface(const machine_config &mconfig, device_t &device);

	wangpcbus_device *m_bus;
	wangpcbus_slot_device *m_slot;

	int m_sid;

private:
	device_wangpcbus_card_interface *m_next;
};


void wangpc_cards(device_slot_interface &device);

#endif // MAME_BUS_WANGPC_WANGPC_H
