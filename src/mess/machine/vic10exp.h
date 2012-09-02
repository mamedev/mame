/**********************************************************************

    Commodore VIC-10 Expansion Port emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************

                    GND       1      A       GND
                    +5V       2      B       _UPROM
                    +5V       3      C       _RESET
                   _IRQ       4      D       _NMI
                  _CR/W       5      E       Sphi2
                     SP       6      F       CA15
                 _EXRAM       7      H       CA14
                    CNT       8      J       CA13
                   _CIA       9      K       CA12
               _CIA PLA      10      L       CA11
                 _LOROM      11      M       CA10
                     BA      12      N       CA9
               R/_W PLA      13      P       CA8
                    CD7      14      R       CA7
                    CD6      15      S       CA6
                    CD5      16      T       CA5
                    CD4      17      U       CA4
                    CD3      18      V       CA3
                    CD2      19      W       CA2
                    CD1      20      X       CA1
                    CD0      21      Y       CA0
                     P2      22      Z       GND

**********************************************************************/

#pragma once

#ifndef __VIC10_EXPANSION_SLOT__
#define __VIC10_EXPANSION_SLOT__

#include "emu.h"
#include "formats/cbm_crt.h"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define VIC10_EXPANSION_SLOT_TAG		"exp"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define VIC10_EXPANSION_INTERFACE(_name) \
	const vic10_expansion_slot_interface (_name) =


#define MCFG_VIC10_EXPANSION_SLOT_ADD(_tag, _clock, _config, _slot_intf, _def_slot, _def_inp) \
    MCFG_DEVICE_ADD(_tag, VIC10_EXPANSION_SLOT, _clock) \
    MCFG_DEVICE_CONFIG(_config) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, _def_inp, false)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vic10_expansion_slot_interface

struct vic10_expansion_slot_interface
{
    devcb_write_line	m_out_irq_cb;
    devcb_write_line	m_out_sp_cb;
    devcb_write_line	m_out_cnt_cb;
    devcb_write_line	m_out_res_cb;
};


// ======================> vic10_expansion_slot_device

class device_vic10_expansion_card_interface;

class vic10_expansion_slot_device : public device_t,
								    public vic10_expansion_slot_interface,
								    public device_slot_interface,
								    public device_image_interface
{
public:
	// construction/destruction
	vic10_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~vic10_expansion_slot_device();

	// computer interface
	UINT8 cd_r(address_space &space, offs_t offset, UINT8 data, int lorom, int uprom, int exram);
	void cd_w(address_space &space, offs_t offset, UINT8 data, int lorom, int uprom, int exram);
	DECLARE_READ_LINE_MEMBER( p0_r );
	DECLARE_WRITE_LINE_MEMBER( p0_w );
	DECLARE_WRITE_LINE_MEMBER( port_res_w );

	// cartridge interface
	DECLARE_WRITE_LINE_MEMBER( irq_w );
	DECLARE_WRITE_LINE_MEMBER( sp_w );
	DECLARE_WRITE_LINE_MEMBER( cnt_w );
	DECLARE_WRITE_LINE_MEMBER( res_w );

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

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
	virtual bool must_be_loaded() const { return 1; }
	virtual bool is_reset_on_load() const { return 1; }
	virtual const char *image_interface() const { return "vic10_cart"; }
	virtual const char *file_extensions() const { return "80,e0"; }
	virtual const option_guide *create_option_guide() const { return NULL; }

	// slot interface overrides
	virtual const char * get_default_card_software(const machine_config &config, emu_options &options);

	devcb_resolved_write_line	m_out_irq_func;
	devcb_resolved_write_line	m_out_sp_func;
	devcb_resolved_write_line	m_out_cnt_func;
	devcb_resolved_write_line	m_out_res_func;

	device_vic10_expansion_card_interface *m_cart;
};


// ======================> device_vic10_expansion_card_interface

// class representing interface-specific live vic10_expansion card
class device_vic10_expansion_card_interface : public device_slot_card_interface
{
	friend class vic10_expansion_slot_device;

public:
	// construction/destruction
	device_vic10_expansion_card_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_vic10_expansion_card_interface();

protected:
	// initialization
	virtual UINT8* vic10_exram_pointer(running_machine &machine, size_t size);
	virtual UINT8* vic10_lorom_pointer(running_machine &machine, size_t size);
	virtual UINT8* vic10_uprom_pointer(running_machine &machine, size_t size);

	// runtime
	virtual UINT8 vic10_cd_r(address_space &space, offs_t offset, UINT8 data, int lorom, int uprom, int exram) { return data; };
	virtual void vic10_cd_w(address_space &space, offs_t offset, UINT8 data, int lorom, int uprom, int exram) { };
	virtual int vic10_p0_r() { return 0; };
	virtual void vic10_p0_w(int state) { };
	virtual void vic10_sp_w(int state) { };
	virtual void vic10_cnt_w(int state) { };
	virtual UINT32 vic10_screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect) { return 0; }
	virtual void vic10_res_w(int state) { };

	vic10_expansion_slot_device *m_slot;

	UINT8 *m_exram;
	UINT8 *m_lorom;
	UINT8 *m_uprom;
};


// device type definition
extern const device_type VIC10_EXPANSION_SLOT;



#endif
