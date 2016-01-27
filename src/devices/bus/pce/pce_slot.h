// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __PCE_SLOT_H
#define __PCE_SLOT_H

/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/


/* PCB */
enum
{
	PCE_STD = 0,
	PCE_CDSYS3J,
	PCE_CDSYS3U,
	PCE_POPULOUS,
	PCE_SF2
};


// ======================> device_pce_cart_interface

class device_pce_cart_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_pce_cart_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_pce_cart_interface();

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_cart) { return 0xff; }
	virtual DECLARE_WRITE8_MEMBER(write_cart) {};

	void rom_alloc(UINT32 size, const char *tag);
	void ram_alloc(UINT32 size);
	UINT8* get_rom_base() { return m_rom; }
	UINT8* get_ram_base() { return &m_ram[0]; }
	UINT32 get_rom_size() { return m_rom_size; }
	UINT32 get_ram_size() { return m_ram.size(); }

	// internal state
	UINT8 *m_rom;
	UINT32 m_rom_size;
	dynamic_buffer m_ram;

	void rom_map_setup(UINT32 size);

	UINT8 rom_bank_map[8];    // 128K chunks of rom
};


// ======================> pce_cart_slot_device

class pce_cart_slot_device : public device_t,
								public device_image_interface,
								public device_slot_interface
{
public:
	// construction/destruction
	pce_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~pce_cart_slot_device();

	// device-level overrides
	virtual void device_start() override;
	virtual void device_config_complete() override;

	// image-level overrides
	virtual bool call_load() override;
	virtual void call_unload() override;
	virtual bool call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry) override;

	int get_type() { return m_type; }
	int get_cart_type(UINT8 *ROM, UINT32 len);

	void internal_header_logging(UINT8 *ROM, UINT32 len);

	void set_intf(const char * interface) { m_interface = interface; }

	virtual iodevice_t image_type() const override { return IO_CARTSLOT; }
	virtual bool is_readable()  const override { return 1; }
	virtual bool is_writeable() const override { return 0; }
	virtual bool is_creatable() const override { return 0; }
	virtual bool must_be_loaded() const override { return 1; }
	virtual bool is_reset_on_load() const override { return 1; }
	virtual const option_guide *create_option_guide() const override { return nullptr; }
	virtual const char *image_interface() const override { return m_interface; }
	virtual const char *file_extensions() const override { return "pce,bin"; }

	// slot interface overrides
	virtual std::string get_default_card_software() override;

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_cart);
	virtual DECLARE_WRITE8_MEMBER(write_cart);


protected:

	const char *m_interface;
	int m_type;
	device_pce_cart_interface*       m_cart;
};



// device type definition
extern const device_type PCE_CART_SLOT;


/***************************************************************************
 DEVICE CONFIGURATION MACROS
 ***************************************************************************/

#define PCESLOT_ROM_REGION_TAG ":cart:rom"

#define MCFG_PCE_CARTRIDGE_ADD(_tag,_slot_intf,_def_slot) \
	MCFG_DEVICE_ADD(_tag, PCE_CART_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false) \
	static_cast<pce_cart_slot_device *>(device)->set_intf("pce_cart");

#define MCFG_TG16_CARTRIDGE_ADD(_tag,_slot_intf,_def_slot) \
	MCFG_DEVICE_ADD(_tag, PCE_CART_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false) \
	static_cast<pce_cart_slot_device *>(device)->set_intf("tg16_cart");


#endif
