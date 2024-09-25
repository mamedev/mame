// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Microtan Bus emulation

**********************************************************************/

#ifndef MAME_BUS_TANBUS_TANBUS_H
#define MAME_BUS_TANBUS_TANBUS_H

#pragma once

#include <functional>
#include <vector>


//**************************************************************************
//  FORWARD DECLARATIONS
//**************************************************************************

class tanbus_device;
class device_tanbus_interface;


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class tanbus_slot_device : public device_t, public device_single_card_slot_interface<device_tanbus_interface>
{
public:
	// construction/destruction
	template <typename T>
	tanbus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, int num, T &&opts, const char *dflt)
		: tanbus_slot_device(mconfig, tag, owner, DERIVED_CLOCK(1, 1))
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
		set_tanbus_slot(num);
	}
	tanbus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// inline configuration
	void set_tanbus_slot(int num) { m_bus_num = num; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	required_device<tanbus_device> m_tanbus;

	int m_bus_num;
};

// device type definition
DECLARE_DEVICE_TYPE(TANBUS_SLOT, tanbus_slot_device)



// ======================> tanbus_device

class tanbus_device : public device_t
{
public:
	// construction/destruction
	tanbus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~tanbus_device();

	// inline configuration
	auto out_irq_callback() { return m_out_irq_cb.bind(); }
	auto out_nmi_callback() { return m_out_nmi_cb.bind(); }
	auto out_so_callback() { return m_out_so_cb.bind(); }
	auto out_pgm_callback() { return m_out_pgm_cb.bind(); }

	void add_card(device_tanbus_interface *card, int num);

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	void set_inhibit_lines(offs_t offset);

	void irq_w(int state) { m_out_irq_cb(state); }
	void nmi_w(int state) { m_out_nmi_cb(state); }
	void so_w(int state) { m_out_so_cb(state); }

	// pgm board has additional cable to fully decode the character generator
	void pgm_w(offs_t offset, uint8_t data) { m_out_pgm_cb(offset, data); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// internal state
	devcb_write_line m_out_irq_cb;
	devcb_write_line m_out_nmi_cb;
	devcb_write_line m_out_so_cb;
	devcb_write8 m_out_pgm_cb;

	uint8_t m_block_register;

	int m_inhrom;
	int m_inhram;
	int m_block_enable;

	std::vector<std::reference_wrapper<device_tanbus_interface> > m_device_list;
};


// device type definition
DECLARE_DEVICE_TYPE(TANBUS, tanbus_device)

// ======================> device_tanbus_interface

class device_tanbus_interface : public device_interface
{
	friend class tanbus_device;

public:
	// bus access
	virtual uint8_t read(offs_t offset, int inhrom, int inhram, int be) { return 0xff; }
	virtual void write(offs_t offset, uint8_t data, int inhrom, int inhram, int be) { }
	virtual void set_inhibit_lines(offs_t offset, int &inhram, int &inhrom) { }

protected:
	device_tanbus_interface(const machine_config &mconfig, device_t &device);

	tanbus_device *m_tanbus;
	int m_page;
};


void tanex_devices(device_slot_interface &device);
void tanbus_devices(device_slot_interface &device);
void tanbus6809_devices(device_slot_interface &device);


typedef device_type_enumerator<tanbus_slot_device> tanbus_slot_device_enumerator;

#endif // MAME_BUS_TANBUS_TANBUS_H
