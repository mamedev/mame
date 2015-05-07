// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    ColecoVision cartridge port emulation

**********************************************************************

                     D2       1      2       /C000
                     D1       3      4       D3
                     D0       5      6       D4
                     A0       7      8       D5
                     A1       9      10      D6
                     A2      11      12      D7
                   SHLD      13      14      A11
                     A3      15      16      A10
                     A4      17      18      /8000
                    A13      19      20      A14
                     A5      21      22      /A000
                     A6      23      24      A12
                     A7      25      26      A9
                  /E000      27      28      A8
                    GND      29      30      +5V


**********************************************************************/

#pragma once

#ifndef __COLECOVISION_CARTRIDGE_SLOT__
#define __COLECOVISION_CARTRIDGE_SLOT__

#include "emu.h"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define COLECOVISION_CARTRIDGE_SLOT_TAG      "cartslot"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_COLECOVISION_CARTRIDGE_SLOT_ADD(_tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, COLECOVISION_CARTRIDGE_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> colecovision_cartridge_slot_device

class device_colecovision_cartridge_interface;

class colecovision_cartridge_slot_device : public device_t,
											public device_slot_interface,
											public device_image_interface
{
public:
	// construction/destruction
	colecovision_cartridge_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~colecovision_cartridge_slot_device() { }

	// computer interface
	UINT8 bd_r(address_space &space, offs_t offset, UINT8 data, int _8000, int _a000, int _c000, int _e000);

protected:
	// device-level overrides
	virtual void device_config_complete() { update_names(); }
	virtual void device_start();

	// image-level overrides
	virtual bool call_load();
	virtual bool call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry);

	virtual iodevice_t image_type() const { return IO_CARTSLOT; }

	virtual bool is_readable()  const { return 1; }
	virtual bool is_writeable() const { return 0; }
	virtual bool is_creatable() const { return 0; }
	virtual bool must_be_loaded() const { return 0; }
	virtual bool is_reset_on_load() const { return 1; }
	virtual const char *image_interface() const { return "coleco_cart"; }
	virtual const char *file_extensions() const { return "rom,col,bin"; }
	virtual const option_guide *create_option_guide() const { return NULL; }

	// slot interface overrides
	virtual void get_default_card_software(std::string &result);

	device_colecovision_cartridge_interface *m_card;
};


// ======================> device_colecovision_cartridge_interface

class device_colecovision_cartridge_interface : public device_slot_card_interface
{
	friend class colecovision_cartridge_slot_device;

public:
	// construction/destruction
	device_colecovision_cartridge_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_colecovision_cartridge_interface() { }

	virtual UINT8 bd_r(address_space &space, offs_t offset, UINT8 data, int _8000, int _a000, int _c000, int _e000) { return 0xff; }

	void rom_alloc(size_t size);

protected:
	UINT8 *m_rom;
	size_t m_rom_size;

	colecovision_cartridge_slot_device *m_slot;
};


// device type definition
extern const device_type COLECOVISION_CARTRIDGE_SLOT;

SLOT_INTERFACE_EXTERN( colecovision_cartridges );



#endif
