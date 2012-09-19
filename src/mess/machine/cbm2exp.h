/**********************************************************************

    Commodore CBM-II Expansion Port emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

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

#pragma once

#ifndef __CBM2_EXPANSION_SLOT__
#define __CBM2_EXPANSION_SLOT__

#include "emu.h"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define CBM2_EXPANSION_SLOT_TAG		"exp"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define CBM2_EXPANSION_INTERFACE(_name) \
	const pet_expansion_slot_interface (_name) =


#define MCFG_CBM2_EXPANSION_SLOT_ADD(_tag, _clock, _slot_intf, _def_slot, _def_inp) \
    MCFG_DEVICE_ADD(_tag, CBM2_EXPANSION_SLOT, _clock) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, _def_inp, false)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cbm2_expansion_slot_device

class device_cbm2_expansion_card_interface;

class cbm2_expansion_slot_device : public device_t,
								   public device_slot_interface,
								   public device_image_interface
{
public:
	// construction/destruction
	cbm2_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~cbm2_expansion_slot_device();

	// computer interface
	UINT8 read(address_space &space, offs_t offset, UINT8 data, int csbank1, int csbank2, int csbank3);
	void write(address_space &space, offs_t offset, UINT8 data, int csbank1, int csbank2, int csbank3);

	// cartridge interface
	int phi2();

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

	// image-level overrides
	virtual bool call_load();
	virtual bool call_softlist_load(char *swlist, char *swname, rom_entry *start_entry);

	virtual iodevice_t image_type() const { return IO_CARTSLOT; }

	virtual bool is_readable()  const { return 1; }
	virtual bool is_writeable() const { return 0; }
	virtual bool is_creatable() const { return 0; }
	virtual bool must_be_loaded() const { return 0; }
	virtual bool is_reset_on_load() const { return 1; }
	virtual const char *image_interface() const { return "cbm2_cart"; }
	virtual const char *file_extensions() const { return "20,40,60"; }
	virtual const option_guide *create_option_guide() const { return NULL; }

	// slot interface overrides
	virtual const char * get_default_card_software(const machine_config &config, emu_options &options);

	device_cbm2_expansion_card_interface *m_cart;
};


// ======================> device_cbm2_expansion_card_interface

class device_cbm2_expansion_card_interface : public device_slot_card_interface
{
	friend class cbm2_expansion_slot_device;

public:
	// construction/destruction
	device_cbm2_expansion_card_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_cbm2_expansion_card_interface();

protected:
	// initialization
	virtual UINT8* cbm2_bank1_pointer(running_machine &machine, size_t size);
	virtual UINT8* cbm2_bank2_pointer(running_machine &machine, size_t size);
	virtual UINT8* cbm2_bank3_pointer(running_machine &machine, size_t size);
	virtual UINT8* cbm2_ram_pointer(running_machine &machine, size_t size);
	virtual UINT8* cbm2_nvram_pointer(running_machine &machine, size_t size);

	// runtime
	virtual UINT8 cbm2_bd_r(address_space &space, offs_t offset, UINT8 data, int csbank1, int csbank2, int csbank3) { return data; };
	virtual void cbm2_bd_w(address_space &space, offs_t offset, UINT8 data, int csbank1, int csbank2, int csbank3) { };

	cbm2_expansion_slot_device *m_slot;

	UINT8 *m_bank1;
	UINT8 *m_bank2;
	UINT8 *m_bank3;
	UINT8 *m_ram;
	UINT8 *m_nvram;

	size_t m_nvram_size;

	size_t m_bank1_mask;
	size_t m_bank2_mask;
	size_t m_bank3_mask;
	size_t m_ram_mask;
	size_t m_nvram_mask;
};


// device type definition
extern const device_type CBM2_EXPANSION_SLOT;



#endif
