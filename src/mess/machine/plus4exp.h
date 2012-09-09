/**********************************************************************

    Commodore Plus/4 Expansion Port emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************

                    GND       1      A       GND
                    +5V       2      B       C1 LOW
                    +5V       3      C       _BRESET
                   _IRQ       4      D       _RAS
                   R/_W       5      E       phi0
                C1 HIGH       6      F       A15
                 C2 LOW       7      H       A14
                C2 HIGH       8      J       A13
                   _CS1       9      K       A12
                   _CS0      10      L       A11
                   _CAS      11      M       A10
                    MUX      12      N       A9
                     BA      13      P       A8
                     D7      14      R       A7
                     D6      15      S       A6
                     D5      16      T       A5
                     D4      17      U       A4
                     D3      18      V       A3
                     D2      19      W       A2
                     D1      20      X       A1
                     D0      21      Y       A0
                    AEC      22      Z       N.C. (RAMEN)
              EXT AUDIO      23      AA      N.C.
                   phi2      24      BB      N.C.
                    GND      25      CC      GND

**********************************************************************/

#pragma once

#ifndef __PLUS4_EXPANSION_SLOT__
#define __PLUS4_EXPANSION_SLOT__

#include "emu.h"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define PLUS4_EXPANSION_SLOT_TAG		"exp"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define PLUS4_EXPANSION_INTERFACE(_name) \
	const plus4_expansion_slot_interface (_name) =


#define MCFG_PLUS4_EXPANSION_SLOT_ADD(_tag, _clock, _config, _slot_intf, _def_slot, _def_inp) \
    MCFG_DEVICE_ADD(_tag, PLUS4_EXPANSION_SLOT, _clock) \
    MCFG_DEVICE_CONFIG(_config) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, _def_inp, false)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> plus4_expansion_slot_interface

struct plus4_expansion_slot_interface
{
	devcb_read8			m_in_dma_cd_cb;
	devcb_write8		m_out_dma_cd_cb;
    devcb_write_line	m_out_irq_cb;
    devcb_write_line	m_out_aec_cb;
};


// ======================> plus4_expansion_slot_device

class device_plus4_expansion_card_interface;

class plus4_expansion_slot_device : public device_t,
									public plus4_expansion_slot_interface,
									public device_slot_interface,
									public device_image_interface
{
public:
	// construction/destruction
	plus4_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~plus4_expansion_slot_device();

	// computer interface
	UINT8 cd_r(address_space &space, offs_t offset, UINT8 data, int ba, int cs0, int c1l, int c2l, int cs1, int c1h, int c2h);
	void cd_w(address_space &space, offs_t offset, UINT8 data, int ba, int cs0, int c1l, int c2l, int cs1, int c1h, int c2h);
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER( breset_w );

	// cartridge interface
	UINT8 dma_cd_r(offs_t offset);
	void dma_cd_w(offs_t offset, UINT8 data);
	DECLARE_WRITE_LINE_MEMBER( irq_w );
	DECLARE_WRITE_LINE_MEMBER( aec_w );
	int phi2();
	int dotclock();

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
	virtual const char *image_interface() const { return "plus4_cart"; }
	virtual const char *file_extensions() const { return "rom,bin"; }
	virtual const option_guide *create_option_guide() const { return NULL; }

	// slot interface overrides
	virtual const char * get_default_card_software(const machine_config &config, emu_options &options);

	devcb_resolved_read8		m_in_dma_cd_func;
	devcb_resolved_write8		m_out_dma_cd_func;
	devcb_resolved_write_line	m_out_irq_func;
	devcb_resolved_write_line	m_out_aec_func;

	device_plus4_expansion_card_interface *m_cart;
};


// ======================> device_plus4_expansion_card_interface

class device_plus4_expansion_card_interface : public device_slot_card_interface
{
	friend class plus4_expansion_slot_device;

public:
	// construction/destruction
	device_plus4_expansion_card_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_plus4_expansion_card_interface();

	// initialization
	virtual UINT8* plus4_c1l_pointer(running_machine &machine, size_t size);
	virtual UINT8* plus4_c1h_pointer(running_machine &machine, size_t size);
	virtual UINT8* plus4_c2l_pointer(running_machine &machine, size_t size);
	virtual UINT8* plus4_c2h_pointer(running_machine &machine, size_t size);
	virtual UINT8* plus4_ram_pointer(running_machine &machine, size_t size);
	virtual UINT8* plus4_nvram_pointer(running_machine &machine, size_t size);

	// runtime
	virtual UINT8 plus4_cd_r(address_space &space, offs_t offset, UINT8 data, int ba, int cs0, int c1l, int c2l, int cs1, int c1h, int c2h) { return data; };
	virtual void plus4_cd_w(address_space &space, offs_t offset, UINT8 data, int ba, int cs0, int c1l, int c2l, int cs1, int c1h, int c2h) { };
	virtual void plus4_breset_w(int state) { };

protected:
	plus4_expansion_slot_device *m_slot;

	UINT8 *m_c1l;
	UINT8 *m_c1h;
	UINT8 *m_c2l;
	UINT8 *m_c2h;
	UINT8 *m_ram;
	UINT8 *m_nvram;

	size_t m_nvram_size;

	size_t m_c1l_mask;
	size_t m_c1h_mask;
	size_t m_c2l_mask;
	size_t m_c2h_mask;
	size_t m_ram_mask;
	size_t m_nvram_mask;
};


// device type definition
extern const device_type PLUS4_EXPANSION_SLOT;



#endif
