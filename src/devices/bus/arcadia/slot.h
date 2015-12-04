// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __ARCADIA_SLOT_H
#define __ARCADIA_SLOT_H

/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/


/* PCB */
enum
{
	ARCADIA_STD = 0,
	ARCADIA_GOLF
};


// ======================> device_arcadia_cart_interface

class device_arcadia_cart_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_arcadia_cart_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_arcadia_cart_interface();

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom) { return 0xff; }
	virtual DECLARE_READ8_MEMBER(extra_rom) { return 0xff; }

	void rom_alloc(UINT32 size, const char *tag);
	UINT8* get_rom_base() { return m_rom; }
	UINT32 get_rom_size() { return m_rom_size; }

protected:
	// internal state
	UINT8 *m_rom;
	UINT32 m_rom_size;
};


// ======================> arcadia_cart_slot_device

class arcadia_cart_slot_device : public device_t,
								public device_image_interface,
								public device_slot_interface
{
public:
	// construction/destruction
	arcadia_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~arcadia_cart_slot_device();

	// device-level overrides
	virtual void device_start();
	virtual void device_config_complete();

	// image-level overrides
	virtual bool call_load();
	virtual void call_unload() {}
	virtual bool call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry);

	int get_type() { return m_type; }

	virtual iodevice_t image_type() const { return IO_CARTSLOT; }
	virtual bool is_readable()  const { return 1; }
	virtual bool is_writeable() const { return 0; }
	virtual bool is_creatable() const { return 0; }
	virtual bool must_be_loaded() const { return 1; }
	virtual bool is_reset_on_load() const { return 1; }
	virtual const option_guide *create_option_guide() const { return nullptr; }
	virtual const char *image_interface() const { return "arcadia_cart"; }
	virtual const char *file_extensions() const { return "bin"; }

	// slot interface overrides
	virtual void get_default_card_software(std::string &result);

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom);
	virtual DECLARE_READ8_MEMBER(extra_rom);

protected:

	int m_type;
	device_arcadia_cart_interface*       m_cart;
};



// device type definition
extern const device_type EA2001_CART_SLOT;


/***************************************************************************
 DEVICE CONFIGURATION MACROS
 ***************************************************************************/

#define EA2001SLOT_ROM_REGION_TAG ":cart:rom"

#define MCFG_ARCADIA_CARTRIDGE_ADD(_tag,_slot_intf,_def_slot) \
	MCFG_DEVICE_ADD(_tag, EA2001_CART_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)
#endif
