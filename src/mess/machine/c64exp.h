/**********************************************************************

    Commodore 64 Expansion Port emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************

                    GND       1      A       GND
                    +5V       2      B       _ROMH
                    +5V       3      C       _RESET
                   _IRQ       4      D       _NMI
                  _CR/W       5      E       Sphi2
                 DOTCLK       6      F       CA15
                  _I/O1       7      H       CA14
                  _GAME       8      J       CA13
                 _EXROM       9      K       CA12
                  _I/O2      10      L       CA11
                  _ROML      11      M       CA10
                     BA      12      N       CA9
                   _DMA      13      P       CA8
                    CD7      14      R       CA7
                    CD6      15      S       CA6
                    CD5      16      T       CA5
                    CD4      17      U       CA4
                    CD3      18      V       CA3
                    CD2      19      W       CA2
                    CD1      20      X       CA1
                    CD0      21      Y       CA0
                    GND      22      Z       GND

**********************************************************************/

#pragma once

#ifndef __C64_EXPANSION_SLOT__
#define __C64_EXPANSION_SLOT__

#include "emu.h"
#include "formats/cbm_crt.h"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define C64_EXPANSION_SLOT_TAG      "exp"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define C64_EXPANSION_INTERFACE(_name) \
	const c64_expansion_slot_interface (_name) =


#define MCFG_C64_EXPANSION_SLOT_ADD(_tag, _clock, _config, _slot_intf, _def_slot, _def_inp) \
	MCFG_DEVICE_ADD(_tag, C64_EXPANSION_SLOT, _clock) \
	MCFG_DEVICE_CONFIG(_config) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, _def_inp, false)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_expansion_slot_interface

struct c64_expansion_slot_interface
{
	devcb_read8         m_in_dma_cd_cb;
	devcb_write8        m_out_dma_cd_cb;
	devcb_write_line    m_out_irq_cb;
	devcb_write_line    m_out_nmi_cb;
	devcb_write_line    m_out_dma_cb;
	devcb_write_line    m_out_reset_cb;
};


// ======================> c64_expansion_slot_device

class device_c64_expansion_card_interface;

class c64_expansion_slot_device : public device_t,
									public c64_expansion_slot_interface,
									public device_slot_interface,
									public device_image_interface
{
public:
	// construction/destruction
	c64_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~c64_expansion_slot_device();

	// computer interface
	UINT8 cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2);
	void cd_w(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2);
	int game_r(offs_t offset, int sphi2, int ba, int rw, int hiram);
	int exrom_r(offs_t offset, int sphi2, int ba, int rw, int hiram);
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER( port_reset_w );

	// cartridge interface
	UINT8 dma_cd_r(offs_t offset);
	void dma_cd_w(offs_t offset, UINT8 data);
	DECLARE_WRITE_LINE_MEMBER( irq_w );
	DECLARE_WRITE_LINE_MEMBER( nmi_w );
	DECLARE_WRITE_LINE_MEMBER( dma_w );
	DECLARE_WRITE_LINE_MEMBER( reset_w );
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
	virtual const char *image_interface() const { return "c64_cart,vic10_cart"; }
	virtual const char *file_extensions() const { return "80,a0,e0,crt"; }
	virtual const option_guide *create_option_guide() const { return NULL; }

	// slot interface overrides
	virtual const char * get_default_card_software(const machine_config &config, emu_options &options);

	devcb_resolved_read8        m_in_dma_cd_func;
	devcb_resolved_write8       m_out_dma_cd_func;
	devcb_resolved_write_line   m_out_irq_func;
	devcb_resolved_write_line   m_out_nmi_func;
	devcb_resolved_write_line   m_out_dma_func;
	devcb_resolved_write_line   m_out_reset_func;

	device_c64_expansion_card_interface *m_cart;
};


// ======================> device_c64_expansion_card_interface

class device_c64_expansion_card_interface : public device_slot_card_interface
{
	friend class c64_expansion_slot_device;

public:
	// construction/destruction
	device_c64_expansion_card_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_c64_expansion_card_interface();

protected:
	// initialization
	virtual UINT8* c64_roml_pointer(running_machine &machine, size_t size);
	virtual UINT8* c64_romh_pointer(running_machine &machine, size_t size);
	virtual UINT8* c64_ram_pointer(running_machine &machine, size_t size);
	virtual UINT8* c64_nvram_pointer(running_machine &machine, size_t size);

	// runtime
	virtual UINT8 c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2) { return data; };
	virtual void c64_cd_w(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2) { };
	virtual int c64_game_r(offs_t offset, int sphi2, int ba, int rw, int hiram) { return m_game; }
	virtual int c64_exrom_r(offs_t offset, int sphi2, int ba, int rw, int hiram) { return m_exrom; }
	virtual void c64_reset_w(int state) { };

	c64_expansion_slot_device *m_slot;

	UINT8 *m_roml;
	UINT8 *m_romh;
	UINT8 *m_ram;
	UINT8 *m_nvram;

	size_t m_nvram_size;

	size_t m_roml_mask;
	size_t m_romh_mask;
	size_t m_ram_mask;
	size_t m_nvram_mask;

	int m_game;
	int m_exrom;
};


// device type definition
extern const device_type C64_EXPANSION_SLOT;



#endif
