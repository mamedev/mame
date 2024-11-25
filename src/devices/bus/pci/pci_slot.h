// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Generic PCI card port

#ifndef MAME_BUS_PCI_PCI_SLOT_H
#define MAME_BUS_PCI_PCI_SLOT_H

#include "machine/pci.h"

#include <array>


class pci_card_interface;

class pci_slot_device: public device_t, public device_single_card_slot_interface<pci_card_interface>
{
public:
	friend class pci_card_interface;

	template <typename T>
	pci_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, u8 slot, u8 irqa, u8 irqb, u8 irqc, u8 irqd, const char *dflt)
		: pci_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
		m_slot = slot;
		m_irq[0] = irqa;
		m_irq[1] = irqb;
		m_irq[2] = irqc;
		m_irq[3] = irqd;
	}

	pci_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	virtual ~pci_slot_device();

	u8 get_slot() const;
	class pci_card_device *get_card() const;

	void get_irq_map(std::array<u8, 4> &map) const { map = m_irq; }

protected:
	virtual void device_start() override ATTR_COLD;

private:
	std::array<u8, 4> m_irq;
	u8 m_slot;
};


class pci_card_interface : public device_interface
{
protected:
	pci_slot_device *const m_pci_slot;

	pci_card_interface(const machine_config &mconfig, device_t &device);
};


class pci_card_device : public pci_device, public pci_card_interface
{
public:
	virtual ~pci_card_device();

	void set_irq_map(u8 irqa, u8 irqb = 0xff, u8 irqc = 0xff, u8 irqd = 0xff) {
		m_irq_map[0] = irqa;
		m_irq_map[1] = irqb;
		m_irq_map[2] = irqc;
		m_irq_map[3] = irqd;
	}

protected:
	u8 m_pin_state;
	std::array<u8, 4> m_irq_map;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	pci_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	void irq_pin_w(offs_t line, int state);
};

DECLARE_DEVICE_TYPE(PCI_SLOT, pci_slot_device)

void pci_cards(device_slot_interface &device);
void agp_cards(device_slot_interface &device);

#endif
