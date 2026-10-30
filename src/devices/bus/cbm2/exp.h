// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore CBM-II Expansion Port emulation

**********************************************************************

                     A0       1      A       BD0
                     A1       2      B       BD1
                     A2       3      C       BD2
                     A3       4      D       BD3
                     A4       5      E       BD4
                     A5       6      F       BD5
                     A6       7      H       BD6
                     A7       8      J       BD7
                     A8       9      K       GND
                     A9      10      L       GND
                    A10      11      M       SR/_W
                    A11      12      N       Sphi2
                    A12      13      P       _CSBANK1
                    +5V      14      R       _CSBANK2
                    +5V      15      S       _CSBANK2

**********************************************************************/

#ifndef MAME_BUS_CBM2_EXP_H
#define MAME_BUS_CBM2_EXP_H

#pragma once

#include "imagedev/cartrom.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cbm2_expansion_window

class cbm2_expansion_window
{
public:
	cbm2_expansion_window(device_t &device, const char *name, offs_t start) : m_view(device, name), m_start(start) { }

	void install_view(address_space_installer &program);

	void install_rom(offs_t start, offs_t end, void *baseptr);
	void install_ram(offs_t start, offs_t end, void *baseptr);

	template <typename R> void install_read_handler(offs_t start, offs_t end, R &&rhandler)
	{
		m_view[0].install_read_handler(m_start + start, m_start + end, std::forward<R>(rhandler));
		m_view.select(0);
	}
	template <typename W> void install_write_handler(offs_t start, offs_t end, W &&whandler)
	{
		m_view[0].install_write_handler(m_start + start, m_start + end, std::forward<W>(whandler));
		m_view.select(0);
	}
	template <typename R, typename W> void install_readwrite_handler(offs_t start, offs_t end, R &&rhandler, W &&whandler)
	{
		m_view[0].install_readwrite_handler(m_start + start, m_start + end, std::forward<R>(rhandler), std::forward<W>(whandler));
		m_view.select(0);
	}

private:
	friend class cbm2_expansion_slot_device;

	memory_view m_view;
	offs_t const m_start;
};


// ======================> cbm2_expansion_slot_device

class device_cbm2_expansion_card_interface;

class cbm2_expansion_slot_device : public device_t,
									public device_single_card_slot_interface<device_cbm2_expansion_card_interface>,
									public device_cartrom_image_interface
{
public:
	// construction/destruction
	template <typename T>
	cbm2_expansion_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock, T &&opts, char const *dflt)
		: cbm2_expansion_slot_device(mconfig, tag, owner, clock)
	{
		set_options(std::forward<T>(opts), dflt, false);
	}
	cbm2_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	template <typename T> void set_program_space(T &&tag, int spacenum) { m_program.set_tag(std::forward<T>(tag), spacenum); }
	uint8_t mirror_r(offs_t offset);
	void mirror_w(offs_t offset, uint8_t data);

	// cartridge interface
	cbm2_expansion_window &bank1() { return m_bank1; }
	cbm2_expansion_window &bank2() { return m_bank2; }
	cbm2_expansion_window &bank3() { return m_bank3; }

	int phi2() { return clock(); }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;

	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "cbm2_cart"; }
	virtual const char *file_extensions() const noexcept override { return "20,40,60"; }

	// device_slot_interface implementation
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	uint8_t *alloc_region(const char *tag);

	optional_address_space m_program;
	device_cbm2_expansion_card_interface *m_card;

private:
	void install_program_views(address_space_installer &program);

	cbm2_expansion_window m_bank1;
	cbm2_expansion_window m_bank2;
	cbm2_expansion_window m_bank3;
};


// ======================> device_cbm2_expansion_card_interface

class device_cbm2_expansion_card_interface : public device_interface
{
	friend class cbm2_expansion_slot_device;

public:
	// construction/destruction
	virtual ~device_cbm2_expansion_card_interface();

protected:
	device_cbm2_expansion_card_interface(const machine_config &mconfig, device_t &device);

	cbm2_expansion_slot_device *m_slot;
};


// device type declaration
DECLARE_DEVICE_TYPE(CBM2_EXPANSION_SLOT, cbm2_expansion_slot_device)


void cbm2_expansion_cards(device_slot_interface &device);

#endif // MAME_BUS_CBM2_EXP_H
