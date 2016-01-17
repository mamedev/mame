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

	void rom_alloc(UINT32 size, std::string tag);
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
	arcadia_cart_slot_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	virtual ~arcadia_cart_slot_device();

	// device-level overrides
	virtual void device_start() override;
	virtual void device_config_complete() override;

	// image-level overrides
	virtual bool call_load() override;
	virtual void call_unload() override {}
	virtual bool call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry) override;

	int get_type() { return m_type; }

	virtual iodevice_t image_type() const override { return IO_CARTSLOT; }
	virtual bool is_readable()  const override { return 1; }
	virtual bool is_writeable() const override { return 0; }
	virtual bool is_creatable() const override { return 0; }
	virtual bool must_be_loaded() const override { return 1; }
	virtual bool is_reset_on_load() const override { return 1; }
	virtual const option_guide *create_option_guide() const override { return nullptr; }
	virtual const char *image_interface() const override { return "arcadia_cart"; }
	virtual const char *file_extensions() const override { return "bin"; }

	// slot interface overrides
	virtual std::string get_default_card_software() override;

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
