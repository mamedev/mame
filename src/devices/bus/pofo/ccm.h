// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Atari Portfolio Memory Card port emulation

**********************************************************************

    Pin     COMMON  RAM     OPTROM              Mask ROM
                            32k 64k 128k
    1       A16
    2       A15
    3               VBB     VPP NC  VPP         NC
    4       A12
    5       A7
    6       A6
    7       A5
    8       A4
    9       A3
    10      A2
    11      A1
    12      A0
    13      D0
    14      D1
    15      D2
    16      GND
    17      D3
    18      D4
    19      D5
    20      D6
    21      D7
    22      CE
    23      A10
    24              OE      OE  OE/VPP          OE      OE
    25      A11
    26      A9
    27      A8
    28      A13
    29      A14
    30              WE      NC  NC              PGM     NC
    31      VCC
    32      CDET

**********************************************************************/

#ifndef MAME_BUS_POFO_CCM_H
#define MAME_BUS_POFO_CCM_H

#pragma once

#include "imagedev/memcard.h"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define PORTFOLIO_MEMORY_CARD_SLOT_A_TAG     "ccma"
#define PORTFOLIO_MEMORY_CARD_SLOT_B_TAG     "ccmb"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> device_portfolio_memory_card_slot_interface

class portfolio_memory_card_slot_device;

class device_portfolio_memory_card_slot_interface : public device_interface
{
	friend class portfolio_memory_card_slot_device;

public:
	virtual bool cdet() { return 1; }

	virtual uint8_t nrdi_r(offs_t offset) { return 0xff; }
	virtual void nwri_w(offs_t offset, uint8_t data) { }

protected:
	// construction/destruction
	device_portfolio_memory_card_slot_interface(const machine_config &mconfig, device_t &device);

	std::unique_ptr<uint8_t[]> m_rom;

	portfolio_memory_card_slot_device *m_slot;
};


// ======================> portfolio_memory_card_slot_device

class portfolio_memory_card_slot_device : public device_t,
									 public device_single_card_slot_interface<device_portfolio_memory_card_slot_interface>,
									 public device_memcard_image_interface
{
public:
	// construction/destruction
	template <typename T>
	portfolio_memory_card_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: portfolio_memory_card_slot_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	portfolio_memory_card_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// computer interface
	bool cdet_r() { return (m_card != nullptr) ? m_card->cdet() : 1; }

	uint8_t nrdi_r(offs_t offset) { return (m_card != nullptr) ? m_card->nrdi_r(offset) : 0xff; }
	void nwri_w(offs_t offset, uint8_t data) { if (m_card != nullptr) m_card->nwri_w(offset, data); }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual const software_list_loader &get_software_list_loader() const override;

	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "pofo_card"; }
	virtual const char *file_extensions() const noexcept override { return "rom,bin"; }

	// device_slot_interface implementation
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	device_portfolio_memory_card_slot_interface *m_card;
};


// device type definition
DECLARE_DEVICE_TYPE(PORTFOLIO_MEMORY_CARD_SLOT, portfolio_memory_card_slot_device)


void portfolio_memory_cards(device_slot_interface &device);



#endif // MAME_BUS_POFO_CCM_H
