// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore PET Memory Expansion Port emulation

**********************************************************************

**********************************************************************/

#ifndef MAME_BUS_PET_EXP_H
#define MAME_BUS_PET_EXP_H

#pragma once




//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pet_expansion_slot_device

class device_pet_expansion_card_interface;

class pet_expansion_slot_device : public device_t,
									public device_single_card_slot_interface<device_pet_expansion_card_interface>
{
public:
	template <typename T>
	pet_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&opts, const char *dflt)
		: pet_expansion_slot_device(mconfig, tag, owner, clock)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	pet_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~pet_expansion_slot_device();

	auto dma_read_callback() { return m_read_dma.bind(); }
	auto dma_write_callback() { return m_write_dma.bind(); }

	// computer interface
	int norom_r(offs_t offset, int sel);
	uint8_t read(offs_t offset, uint8_t data, int &sel);
	void write(offs_t offset, uint8_t data, int &sel);
	int diag_r();
	void irq_w(int state);

	// cartridge interface
	uint8_t dma_bd_r(offs_t offset);
	void dma_bd_w(offs_t offset, uint8_t data);
	int phi2();

	enum
	{
		SEL_NONE = -1,
		SEL0 = 0,
		SEL1,
		SEL2,
		SEL3,
		SEL4,
		SEL5,
		SEL6,
		SEL7,
		SEL8,
		SEL9,
		SELA,
		SELB,
		SELC,
		SELD,
		SELE,
		SELF
	};

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	device_pet_expansion_card_interface *m_card;

	devcb_read8  m_read_dma;
	devcb_write8 m_write_dma;
};


// ======================> device_pet_expansion_card_interface

class device_pet_expansion_card_interface : public device_interface
{
	friend class pet_expansion_slot_device;

public:
	// construction/destruction
	virtual ~device_pet_expansion_card_interface();

protected:
	device_pet_expansion_card_interface(const machine_config &mconfig, device_t &device);

	// runtime
	virtual int pet_norom_r(offs_t offset, int sel) { return 1; }
	virtual uint8_t pet_bd_r(offs_t offset, uint8_t data, int &sel) { return data; }
	virtual void pet_bd_w(offs_t offset, uint8_t data, int &sel) { }
	virtual int pet_diag_r() { return 1; }
	virtual void pet_irq_w(int state) { }

	pet_expansion_slot_device *m_slot;
};


// device type definition
DECLARE_DEVICE_TYPE(PET_EXPANSION_SLOT, pet_expansion_slot_device)


void pet_expansion_cards(device_slot_interface &device);

#endif // MAME_BUS_PET_EXP_H
