// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    Standard bus connector:

	Pin # Signal Name
		1	A15
		2	A14
		3	A13
		4	A12
		5	A11
		6	A10
		7	A9
		8	A8
		9	A7
		10	A6
		11	A5
		12	A4
		13	A3
		14	A2
		15	A1
		16	A0
		17	GND
		18	5V
		19	/M1
		20	/RESET
		21	CLK
		22	/INT
		23	/MREQ
		24	/WR
		25	/RD
		26	/IORQ
		27	D0
		28	D1
		29	D2
		30	D3
		31	D4
		32	D5
		33	D6
		34	D7
		35	TX
		36	RX
		37	USER1
		38	USER2
		39	USER3
		40	USER4

***************************************************************************/

#ifndef MAME_BUS_RC2014_RC2014_H
#define MAME_BUS_RC2014_RC2014_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> rc2014_bus_device

class rc2014_bus_device : public device_t
						, public device_memory_interface
{
public:
	// construction/destruction
	rc2014_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
/*
	auto irq() { return m_write_irq.bind(); }
	auto nmi() { return m_write_nmi.bind(); }
*/
	void set_bus_clock(u32 clock);
	void set_bus_clock(const XTAL &xtal) { set_bus_clock(xtal.value()); }
/*
	DECLARE_WRITE_LINE_MEMBER( irq_w ) { m_write_irq(state); }
	DECLARE_WRITE_LINE_MEMBER( nmi_w ) { m_write_nmi(state); }
*/
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

private:
/*	devcb_write_line   m_write_irq;
	devcb_write_line   m_write_nmi;
*/
	address_space_config const m_mem_config;
	address_space_config const m_io_config;
};

// ======================> device_rc2014_card_interface
class rc2014_slot_device;

class device_rc2014_card_interface : public device_interface
{
	friend class rc2014_slot_device;

protected:
	// construction/destruction
	device_rc2014_card_interface(const machine_config &mconfig, device_t &device);

	void set_bus_device(rc2014_bus_device &bus_device);

	rc2014_bus_device  *m_bus;
};

// ======================> rc2014_slot_device

class rc2014_slot_device : public device_t, public device_single_card_slot_interface<device_rc2014_card_interface>
{
public:
	rc2014_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	template <typename T, typename U>
	rc2014_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&bus_tag, U &&slot_options, char const *default_option)
		: rc2014_slot_device(mconfig, tag, owner, DERIVED_CLOCK(1,1))
	{
		m_bus.set_tag(std::forward<T>(bus_tag));
		option_reset();
		slot_options(*this);
		set_default_option(default_option);
		set_fixed(false);
	}

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_resolve_objects() override;

private:
	required_device<rc2014_bus_device> m_bus;
};

// device type definition
DECLARE_DEVICE_TYPE(RC2014_BUS,  rc2014_bus_device)
DECLARE_DEVICE_TYPE(RC2014_SLOT, rc2014_slot_device)


#endif // MAME_BUS_RC2014_RC2014_H
