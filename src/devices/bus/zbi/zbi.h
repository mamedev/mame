// license:BSD-3-Clause
// copyright-holders:A. Lenard
/***************************************************************************

    ZBI bus & slot device

ZBI (Z-Bus Backplane Interconnect) is a multiplexed 32-bit bus.
Each ZBI card has 2 connectors, P1 and P2.
P1 is for connecting the card to the common ZBI bus.
P2 is for connecting it to the peripheral(s) managed by the device.
Both connectors have the same number of pins, but P2 is unique to each device.
This file defines the shared ZBI bus connector (P1).

==================================================
           ZBI Backplane Connector (P1)
==================================================
     Row A            Row B            Row C
 Pin Signal       Pin Signal       Pin Signal
==================================================
 1   _RESET       33  _WAIT        65  CAVAIL
 2   _CAI         34  _CAO         66  _CPUREQ
 3   _BAI         35  _BAO         67  _BUSREQ
 4   _MMAI        36  _MMAO        68  GND
 5   IEI3         37  IEO3         69  _MMREQ
 6   IEI2         38  IEO2         70  Not used
 7   IEI1         39  IEO1         71  GND
 8   _INT1        40  _INT2        72  _INT3
 9   R/_W         41  B/_W         73  W/_LW
 10  S2           42  S3           74  S4
 11  S0           43  S1           75  GND
 12  _ME          44  _AS          76  _DS
 13  Not used     45  _STOP        77  N/_S
 14  Not used     46  Not used     78  Not used
 15  AD31         47  Not used     79  GND
 16  AD28         48  AD29         80  AD30
 17  AD25         49  AD26         81  AD27
 18  AD22         50  AD23         82  AD24
 19  AD20         51  AD21         83  GND
 20  AD17         52  AD18         84  AD19
 21  AD14         53  AD15         85  AD16
 22  AD11         54  AD12         86  AD13
 23  AD9          55  AD10         87  GND
 24  AD6          56  AD7          88  AD8
 25  AD3          57  AD4          89  AD5
 26  AD0          58  AD1          90  AD2
 27  _PWRBAD      59  MCLK         91  BCLK
 28  +5V          60  +5V          92  +5V
 29  -5V          61  -5V          93  -5V
 30  +12V         62  +12V         94  +12V
 31  -12V         63  -12V         95  -12V
 32  GND          64  GND          96  GND
==================================================

***************************************************************************/

#ifndef MAME_BUS_ZBI_ZBI_H
#define MAME_BUS_ZBI_ZBI_H

#pragma once

#include "machine/z80daisy.h"

#include <functional>
#include <vector>

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> zbi_bus_device

class device_zbi_card_interface;

class zbi_bus_device : public device_t, public z80_daisy_chain_interface
{
public:
	// construction/destruction
	zbi_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~zbi_bus_device();

	void set_bus_clock(u32 clock);
	void set_bus_clock(const XTAL &xtal)
		{ set_bus_clock(xtal.value()); }

	void assign_iospace_installer(address_space_installer *installer);
	address_space_installer *iospace() const;

	void add_to_daisy_chain(std::string tag)
		{ m_daisy.push_back(tag); }

	void memerr_w(int state);
	void busreq_w(int state);
	void busack_w(int state);
	void cpureq_w(int state);
	void cpuavail_w(int state);
	void mmreq_w(int state);
	void vi_w(int state);
	void nvi_w(int state);
	void nmi_w(int state);
	void ns_w(int state);
	void wait_w(int state);
	void stop_w(int state);
	void reti_w(uint8_t data)
		{ if (data == 0x4d) daisy_call_reti_device(); }

	uint16_t viack_r();
	uint16_t nviack_r();
	uint16_t nmiack_r();

	void add_card(device_zbi_card_interface &card);

	uint8_t ram8_r(offs_t offset);
	void ram8_w(offs_t offset, uint8_t data);
	uint16_t ram16_r(offs_t offset, uint16_t mask = ~0);
	void ram16_w(offs_t offset, uint16_t data, uint16_t mask = ~0);
	uint32_t ram32_r(offs_t offset, uint32_t mask = ~0);
	void ram32_w(offs_t offset, uint32_t data, uint32_t mask = ~0);

protected:
	// configuration helpers
	void set_daisy_config(const z80_daisy_config *config)
		{ z80_daisy_chain_interface::set_daisy_config(config); }

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// interface-level overrides
	virtual void interface_post_start() override;

private:
	using card_vector = std::vector<std::reference_wrapper<device_zbi_card_interface> >;

	address_space_installer *m_iospace_installer;
	std::vector<std::string> m_daisy;
	z80_daisy_config *m_daisy_chain;
	card_vector m_device_list;
};

// ======================> device_zbi_card_interface

class zbi_slot_device;

class device_zbi_card_interface : public device_z80daisy_interface
{
	friend class zbi_slot_device;
	friend class zbi_bus_device;

public:
	// construction/destruction
	device_zbi_card_interface(const machine_config &mconfig, device_t &device);

protected:
	virtual void card_memerr_w(int state) { }
	virtual void card_busreq_w(int state) { }
	virtual void card_cpureq_w(int state) { }
	virtual void card_cpuavail_w(int state) { }
	virtual void card_mmreq_w(int state) { }
	virtual void card_nvi_w(int state) { }
	virtual void card_vi_w(int state) { }
	virtual void card_nmi_w(int state) { }
	virtual void card_ns_w(int state) { }
	virtual void card_wait_w(int state) { }
	virtual void card_stop_w(int state) { }

	virtual uint8_t card_ram8_r(offs_t offset) { return 0; }
	virtual void card_ram8_w(offs_t offset, uint8_t data) {}
	virtual uint16_t card_ram16_r(offs_t offset, uint16_t mask) { return 0; }
	virtual void card_ram16_w(offs_t offset, uint16_t data, uint16_t mask) {}
	virtual uint32_t card_ram32_r(offs_t offset, uint32_t mask) { return 0; }
	virtual void card_ram32_w(offs_t offset, uint32_t data, uint32_t mask) {}

	// z80daisy_interface overrides
	virtual int z80daisy_irq_state() override;
	virtual int z80daisy_irq_ack() override;
	virtual void z80daisy_irq_reti() override;

	virtual int busdaisy_req_state();
	virtual void busdaisy_req_ack();

	zbi_bus_device *m_bus;
};

// ======================> zbi_slot_device

class zbi_slot_device : public device_t, public device_slot_interface
{
public:
	// construction/destruction
	template <typename T, typename U>
	zbi_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&bus_tag, U &&slot_options, char const *default_option, bool fixed = false)
		: zbi_slot_device(mconfig, tag, owner, DERIVED_CLOCK(1,1))
	{
		m_bus.set_tag(std::forward<T>(bus_tag));
		option_reset();
		slot_options(*this);
		set_default_option(default_option);
		set_fixed(fixed);
	}

	zbi_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_resolve_objects() override ATTR_COLD;

	// configuration
	required_device<zbi_bus_device> m_bus;
};

// device type definition
DECLARE_DEVICE_TYPE(ZBI_BUS, zbi_bus_device)
DECLARE_DEVICE_TYPE(ZBI_SLOT, zbi_slot_device)

#endif // MAME_BUS_ZBI_ZBI_H
