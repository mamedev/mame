/**********************************************************************

    VideoBrain Expansion Port emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************

                    GND       1      2       BO2
                    BO1       3      4       BO0
                    BA0       5      6       BA1
                    BA2       7      8       BA3
                    BA4       9      10      BA5
                    BA6      11      12      BA7
                    +5V      13      14      BA8
                    BA9      15      16      _CS1
                   BA10      17      18      BO7
                    BO6      19      20      BO5
                    BO4      21      22      BO3
                   BA11      23      24      BA12
                    BRC      25      26      ?
                 EXTRES      27      28      _CS2
                      ?      29      30      UV202/35
                 HBLANK      31      32      UV201/5
                UV201/2      33      34      UV201/4
                UV201/3      35      36      BISTROBE
                UV202/1      37      38      BA13
               UV202/18      39      40      ?
                    R/W      41      42      +5V
                    GND      43      44      GND
                    GND      45      46      N/C
                      ?      47      48      N/C
                    N/C      49      50      N/C

**********************************************************************/

#pragma once

#ifndef __VIDEOBRAIN_EXPANSION_SLOT__
#define __VIDEOBRAIN_EXPANSION_SLOT__

#include "emu.h"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define VIDEOBRAIN_EXPANSION_SLOT_TAG       "exp"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define VIDEOBRAIN_EXPANSION_INTERFACE(_name) \
	const videobrain_expansion_slot_interface (_name) =


#define MCFG_VIDEOBRAIN_EXPANSION_SLOT_ADD(_tag, _config, _slot_intf, _def_slot, _def_inp) \
	MCFG_DEVICE_ADD(_tag, VIDEOBRAIN_EXPANSION_SLOT, 0) \
	MCFG_DEVICE_CONFIG(_config) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, _def_inp, false)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> videobrain_expansion_slot_interface

struct videobrain_expansion_slot_interface
{
	devcb_write_line    m_out_extres_cb;
};


// ======================> videobrain_expansion_slot_device

class device_videobrain_expansion_card_interface;

class videobrain_expansion_slot_device : public device_t,
											public videobrain_expansion_slot_interface,
											public device_slot_interface,
											public device_image_interface
{
public:
	// construction/destruction
	videobrain_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~videobrain_expansion_slot_device();

	// computer interface
	UINT8 bo_r(address_space &space, offs_t offset, int cs1, int cs2);
	void bo_w(address_space &space, offs_t offset, UINT8 data, int cs1, int cs2);

	DECLARE_READ8_MEMBER( cs1_r );
	DECLARE_WRITE8_MEMBER( cs1_w );
	DECLARE_READ8_MEMBER( cs2_r );
	DECLARE_WRITE8_MEMBER( cs2_w );
	DECLARE_READ8_MEMBER( unmap_r );
	DECLARE_WRITE8_MEMBER( unmap_w );

	// cartridge interface
	DECLARE_WRITE_LINE_MEMBER( extres_w );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();

	// image-level overrides
	virtual bool call_load();
	virtual bool call_softlist_load(char *swlist, char *swname, rom_entry *start_entry);

	virtual iodevice_t image_type() const { return IO_CARTSLOT; }

	virtual bool is_readable()  const { return 1; }
	virtual bool is_writeable() const { return 0; }
	virtual bool is_creatable() const { return 0; }
	virtual bool must_be_loaded() const { return 0; }
	virtual bool is_reset_on_load() const { return 1; }
	virtual const char *image_interface() const { return "vidbrain_cart"; }
	virtual const char *file_extensions() const { return "bin"; }
	virtual const option_guide *create_option_guide() const { return NULL; }

	// slot interface overrides
	virtual const char * get_default_card_software(const machine_config &config, emu_options &options);

	devcb_resolved_write_line   m_out_extres_func;

	device_videobrain_expansion_card_interface *m_cart;
};


// ======================> device_videobrain_expansion_card_interface

class device_videobrain_expansion_card_interface : public device_slot_card_interface
{
	friend class videobrain_expansion_slot_device;

public:
	// construction/destruction
	device_videobrain_expansion_card_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_videobrain_expansion_card_interface();

protected:
	// initialization
	virtual UINT8* videobrain_rom_pointer(running_machine &machine, size_t size);
	virtual UINT8* videobrain_ram_pointer(running_machine &machine, size_t size);

	// runtime
	virtual UINT8 videobrain_bo_r(address_space &space, offs_t offset, int cs1, int cs2) { return 0; };
	virtual void videobrain_bo_w(address_space &space, offs_t offset, UINT8 data, int cs1, int cs2) { };
	virtual void videobrain_extres_w() { };

	videobrain_expansion_slot_device *m_slot;

	UINT8 *m_rom;
	UINT8 *m_ram;

	size_t m_rom_mask;
	size_t m_ram_mask;
};


// device type definition
extern const device_type VIDEOBRAIN_EXPANSION_SLOT;



#endif
