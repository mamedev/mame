// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood
#ifndef __NEOGEO_SLOT_H
#define __NEOGEO_SLOT_H

#include "neogeo_intf.h"
#include "neogeo_helper.h"

/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/

// ======================> neogeo_cart_slot_device

class neogeo_cart_slot_device : public device_t,
								public device_image_interface,
								public device_slot_interface
{
public:
	// construction/destruction
	neogeo_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT16 clock);
	virtual ~neogeo_cart_slot_device();

	// device-level overrides
	virtual void device_start();
	virtual void device_config_complete();

	// image-level overrides
	virtual bool call_load();
	virtual void call_unload();
	virtual bool call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry);

	virtual iodevice_t image_type() const { return IO_CARTSLOT; }
	virtual bool is_readable()  const { return 1; }
	virtual bool is_writeable() const { return 0; }
	virtual bool is_creatable() const { return 0; }
	virtual bool must_be_loaded() const { return 0; }
	virtual bool is_reset_on_load() const { return 1; }
	virtual const option_guide *create_option_guide() const { return nullptr; }
	virtual const char *image_interface() const { return "neo_cart"; }
	virtual const char *file_extensions() const { return "bin"; }

	// slot interface overrides
	virtual void get_default_card_software(std::string &result);

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read_rom);

	UINT16* get_rom_base() { if (m_cart) { return m_cart->get_rom_base(); } else { return nullptr; } }
	UINT32  get_rom_size() { if (m_cart) { return m_cart->get_rom_size(); } else { return 0; } }
	UINT8* get_fixed_base() { if (m_cart) { return m_cart->get_fixed_base(); } else { return nullptr; } }
	UINT32  get_fixed_size() { if (m_cart) { return m_cart->get_fixed_size(); } else { return 0; } }
	UINT8* get_sprites_base() { if (m_cart) { return m_cart->get_sprites_base(); } else { return nullptr; } }
	UINT32  get_sprites_size() { if (m_cart) { return m_cart->get_sprites_size(); } else { return 0; } }
	UINT8* get_sprites_optimized() { if (m_cart) { return m_cart->get_sprites_optimized(); } else { return nullptr; } }
	UINT32 get_sprites_addrmask() { if (m_cart) { return m_cart->get_sprites_addrmask(); } else { return 0; } }
	UINT8* get_audio_base() { if (m_cart) { return m_cart->get_audio_base(); } else { return nullptr; } }
	UINT32  get_audio_size() { if (m_cart) { return m_cart->get_audio_size(); } else { return 0; } }
	UINT8* get_ym_base() { if (m_cart) { return m_cart->get_ym_base(); } else { return nullptr; } }
	UINT32  get_ym_size() { if (m_cart) { return m_cart->get_ym_size(); } else { return 0; } }
	UINT8* get_ymdelta_base() { if (m_cart) { return m_cart->get_ymdelta_base(); } else { return nullptr; } }
	UINT32  get_ymdelta_size() { if (m_cart) { return m_cart->get_ymdelta_size(); } else { return 0; } }
	int get_fixed_bank_type(void) { if (m_cart) { return m_cart->get_fixed_bank_type(); } else { return 0; } }

	void activate_cart(ACTIVATE_CART_PARAMS) { if (m_cart) m_cart->activate_cart(machine, maincpu, cpuregion, cpuregion_size, fixedregion, fixedregion_size);  }
	void setup_memory_banks(running_machine &machine);


private:
	device_neogeo_cart_interface*       m_cart;
};


// device type definition
extern const device_type NEOGEO_CART_SLOT;


/***************************************************************************
 DEVICE CONFIGURATION MACROS
 ***************************************************************************/

#define MCFG_NEOGEO_CARTRIDGE_ADD(_tag,_slot_intf,_def_slot) \
	MCFG_DEVICE_ADD(_tag, NEOGEO_CART_SLOT, 0)  \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)


#endif
