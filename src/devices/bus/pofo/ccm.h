// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Atari Portfolio Memory Card port emulation

**********************************************************************

**********************************************************************/

#pragma once

#ifndef __PORTFOLIO_MEMORY_CARD_SLOT__
#define __PORTFOLIO_MEMORY_CARD_SLOT__

#include "emu.h"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define PORTFOLIO_MEMORY_CARD_SLOT_A_TAG     "ccma"
#define PORTFOLIO_MEMORY_CARD_SLOT_B_TAG     "ccmb"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_PORTFOLIO_MEMORY_CARD_SLOT_ADD(_tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, PORTFOLIO_MEMORY_CARD_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> device_portfolio_memory_card_slot_interface

class portfolio_memory_card_slot_t;

class device_portfolio_memory_card_slot_interface : public device_slot_card_interface
{
	friend class portfolio_memory_card_slot_t;

public:
	// construction/destruction
	device_portfolio_memory_card_slot_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_portfolio_memory_card_slot_interface() { }

	virtual bool cdet() { return 1; }

	virtual UINT8 nrdi_r(address_space &space, offs_t offset) { return 0xff; };
	virtual void nwri_w(address_space &space, offs_t offset, UINT8 data) { };

protected:
	optional_shared_ptr<UINT8> m_rom;
	optional_shared_ptr<UINT8> m_nvram;

	portfolio_memory_card_slot_t *m_slot;
};


// ======================> portfolio_memory_card_slot_t

class portfolio_memory_card_slot_t : public device_t,
									 public device_slot_interface,
								     public device_image_interface
{
public:
	// construction/destruction
	portfolio_memory_card_slot_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~portfolio_memory_card_slot_t() { }

	// computer interface
	bool cdet_r() { return (m_card != nullptr) ? m_card->cdet() : 1; }

	DECLARE_READ8_MEMBER( nrdi_r ) { return (m_card != nullptr) ? m_card->nrdi_r(space, offset) : 0xff; }
	DECLARE_WRITE8_MEMBER( nwri_w ) { if (m_card != nullptr) m_card->nwri_w(space, offset, data); }

protected:
	// device-level overrides
	virtual void device_config_complete() override { update_names(); }
	virtual void device_start() override;

	// image-level overrides
	virtual image_init_result call_load() override;
	virtual const software_list_loader &get_software_list_loader() const override { return rom_software_list_loader::instance(); }

	virtual iodevice_t image_type() const override { return IO_CARTSLOT; }

	virtual bool is_readable()  const override { return 1; }
	virtual bool is_writeable() const override { return 1; }
	virtual bool is_creatable() const override { return 1; }
	virtual bool must_be_loaded() const override { return 0; }
	virtual bool is_reset_on_load() const override { return 1; }
	virtual const char *image_interface() const override { return "pofo_card"; }
	virtual const char *file_extensions() const override { return "rom,bin"; }

	// slot interface overrides
	virtual std::string get_default_card_software() override;

	device_portfolio_memory_card_slot_interface *m_card;
};


// device type definition
extern const device_type PORTFOLIO_MEMORY_CARD_SLOT;


SLOT_INTERFACE_EXTERN( portfolio_memory_cards );



#endif
