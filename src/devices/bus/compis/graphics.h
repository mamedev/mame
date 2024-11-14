// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    TeleNova Compis graphics bus emulation

**********************************************************************

**********************************************************************/

#ifndef MAME_BUS_COMPIS_GRAPHICS_H
#define MAME_BUS_COMPIS_GRAPHICS_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> device_compis_graphics_card_interface

class compis_graphics_slot_device;

class device_compis_graphics_card_interface : public device_interface
{
public:
	virtual uint8_t mcs0_r(offs_t offset) { return 0xff; }
	virtual void mcs0_w(offs_t offset, uint8_t data) { }
	virtual uint8_t mcs1_r(offs_t offset) { return 0xff; }
	virtual void mcs1_w(offs_t offset, uint8_t data) { }
	virtual uint16_t pcs3_r(offs_t offset) { return 0xff; }
	virtual void pcs3_w(offs_t offset, uint16_t data) { }
	virtual uint8_t pcs6_6_r(offs_t offset) { return 0xff; }
	virtual void pcs6_6_w(offs_t offset, uint8_t data) { }
	virtual uint8_t dma_ack_r(offs_t offset) { return 0xff; }
	virtual void dma_ack_w(offs_t offset, uint8_t data) { }

protected:
	// construction/destruction
	device_compis_graphics_card_interface(const machine_config &mconfig, device_t &device);

	compis_graphics_slot_device *m_slot;
};


// ======================> compis_graphics_slot_device

class compis_graphics_slot_device : public device_t, public device_single_card_slot_interface<device_compis_graphics_card_interface>
{
public:
	// construction/destruction
	template <typename T>
	compis_graphics_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock, T &&opts, char const *dflt)
		: compis_graphics_slot_device(mconfig, tag, owner, clock)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	compis_graphics_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto drq() { return m_write_dma_request.bind(); }

	// computer interface
	uint8_t mcs0_r(offs_t offset) { return m_card ? m_card->mcs0_r(offset) : 0xff; }
	void mcs0_w(offs_t offset, uint8_t data) { if (m_card) m_card->mcs0_w(offset, data); }
	uint8_t mcs1_r(offs_t offset) { return m_card ? m_card->mcs1_r(offset) : 0xff; }
	void mcs1_w(offs_t offset, uint8_t data) { if (m_card) m_card->mcs1_w(offset, data); }
	uint8_t pcs3_r(offs_t offset) { return m_card ? m_card->pcs3_r(offset) : 0xff; }
	void pcs3_w(offs_t offset, uint8_t data) { if (m_card) m_card->pcs3_w(offset, data); }
	uint8_t pcs6_6_r(offs_t offset) { return m_card ? m_card->pcs6_6_r(offset) : 0xff; }
	void pcs6_6_w(offs_t offset, uint8_t data) { if (m_card) m_card->pcs6_6_w(offset, data); }
	uint8_t dma_ack_r(offs_t offset) { return m_card ? m_card->dma_ack_r(offset) : 0xff; }
	void dma_ack_w(offs_t offset, uint8_t data) { if (m_card) m_card->dma_ack_w(offset, data); }

	// card interface
	void dma_request_w(int state) { m_write_dma_request(state); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	devcb_write_line   m_write_dma_request;

	device_compis_graphics_card_interface *m_card;
};


// device type definition
DECLARE_DEVICE_TYPE(COMPIS_GRAPHICS_SLOT, compis_graphics_slot_device)


void compis_graphics_cards(device_slot_interface &device);


#endif // MAME_BUS_COMPIS_GRAPHICS_H
