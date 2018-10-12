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
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_COMPIS_GRAPHICS_SLOT_ADD(_tag, _clock, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, COMPIS_GRAPHICS_SLOT, _clock) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)


#define MCFG_COMPIS_GRAPHICS_SLOT_DMA_REQUEST_CALLBACK(_dma_request) \
	downcast<compis_graphics_slot_device *>(device)->set_dma_request_callback(DEVCB_##_dma_request);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> device_compis_graphics_card_interface

class compis_graphics_slot_device;

class device_compis_graphics_card_interface : public device_slot_card_interface
{
public:
	virtual uint8_t mcs0_r(address_space &space, offs_t offset) { return 0xff; }
	virtual void mcs0_w(address_space &space, offs_t offset, uint8_t data) { }
	virtual uint8_t mcs1_r(address_space &space, offs_t offset) { return 0xff; }
	virtual void mcs1_w(address_space &space, offs_t offset, uint8_t data) { }
	virtual uint16_t pcs3_r(address_space &space, offs_t offset) { return 0xff; }
	virtual void pcs3_w(address_space &space, offs_t offset, uint16_t data) { }
	virtual uint8_t pcs6_6_r(address_space &space, offs_t offset) { return 0xff; }
	virtual void pcs6_6_w(address_space &space, offs_t offset, uint8_t data) { }
	virtual uint8_t dma_ack_r(address_space &space, offs_t offset) { return 0xff; }
	virtual void dma_ack_w(address_space &space, offs_t offset, uint8_t data) { }

protected:
	// construction/destruction
	device_compis_graphics_card_interface(const machine_config &mconfig, device_t &device);

	compis_graphics_slot_device *m_slot;
};


// ======================> compis_graphics_slot_device

class compis_graphics_slot_device : public device_t,
							   public device_slot_interface
{
public:
	// construction/destruction
	compis_graphics_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template<class _dma_request> void set_dma_request_callback(_dma_request dma_request) { m_write_dma_request.set_callback(dma_request); }

	// computer interface
	DECLARE_READ8_MEMBER( mcs0_r ) { return m_card ? m_card->mcs0_r(space, offset) : 0xff; }
	DECLARE_WRITE8_MEMBER( mcs0_w ) { if (m_card) m_card->mcs0_w(space, offset, data); }
	DECLARE_READ8_MEMBER( mcs1_r ) { return m_card ? m_card->mcs1_r(space, offset) : 0xff; }
	DECLARE_WRITE8_MEMBER( mcs1_w ) { if (m_card) m_card->mcs1_w(space, offset, data); }
	DECLARE_READ16_MEMBER( pcs3_r ) { return m_card ? m_card->pcs3_r(space, offset) : 0xff; }
	DECLARE_WRITE16_MEMBER( pcs3_w ) { if (m_card) m_card->pcs3_w(space, offset, data); }
	DECLARE_READ8_MEMBER( pcs6_6_r ) { return m_card ? m_card->pcs6_6_r(space, offset) : 0xff; }
	DECLARE_WRITE8_MEMBER( pcs6_6_w ) { if (m_card) m_card->pcs6_6_w(space, offset, data); }
	DECLARE_READ8_MEMBER( dma_ack_r ) { return m_card ? m_card->dma_ack_r(space, offset) : 0xff; }
	DECLARE_WRITE8_MEMBER( dma_ack_w ) { if (m_card) m_card->dma_ack_w(space, offset, data); }

	// card interface
	DECLARE_WRITE_LINE_MEMBER( dma_request_w ) { m_write_dma_request(state); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override { if (m_card) get_card_device()->reset(); }

	devcb_write_line   m_write_dma_request;

	device_compis_graphics_card_interface *m_card;
};


// device type definition
DECLARE_DEVICE_TYPE(COMPIS_GRAPHICS_SLOT, compis_graphics_slot_device)


void compis_graphics_cards(device_slot_interface &device);


#endif // MAME_BUS_COMPIS_GRAPHICS_H
