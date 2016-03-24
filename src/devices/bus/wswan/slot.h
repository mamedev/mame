// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef __WS_SLOT_H
#define __WS_SLOT_H

/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/


/* PCB */
enum
{
	WS_STD = 0,
	WS_SRAM,
	WS_EEPROM
};


// ======================> device_ws_cart_interface

class device_ws_cart_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_ws_cart_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_ws_cart_interface();

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom20) { return 0xff; }
	virtual DECLARE_READ8_MEMBER(read_rom30) { return 0xff; }
	virtual DECLARE_READ8_MEMBER(read_rom40) { return 0xff; }
	virtual DECLARE_READ8_MEMBER(read_ram) { return 0xff; }
	virtual DECLARE_WRITE8_MEMBER(write_ram) {}
	virtual DECLARE_READ8_MEMBER(read_io) { return 0xff; }
	virtual DECLARE_WRITE8_MEMBER(write_io) {}

	void rom_alloc(UINT32 size, const char *tag);
	void nvram_alloc(UINT32 size);
	UINT8* get_rom_base() { return m_rom; }
	UINT8* get_nvram_base() { return &m_nvram[0]; }
	UINT32 get_rom_size() { return m_rom_size; }
	UINT32 get_nvram_size() { return m_nvram.size(); }

	void save_nvram()   { device().save_item(NAME(m_nvram)); }
	void set_has_rtc(bool val) { m_has_rtc = val; }
	void set_is_rotated(bool val) { m_is_rotated = val; }
	int get_is_rotated() { return m_is_rotated ? 1 : 0; }

protected:
	// internal state
	UINT8 *m_rom;
	UINT32 m_rom_size;
	dynamic_buffer m_nvram;
	int m_bank_mask;

	bool m_has_rtc, m_is_rotated;
};


// ======================> ws_cart_slot_device

class ws_cart_slot_device : public device_t,
								public device_image_interface,
								public device_slot_interface
{
public:
	// construction/destruction
	ws_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~ws_cart_slot_device();

	// device-level overrides
	virtual void device_start() override;
	virtual void device_config_complete() override;

	// image-level overrides
	virtual bool call_load() override;
	virtual void call_unload() override;
	virtual bool call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry) override;

	int get_type() { return m_type; }
	int get_is_rotated() { return m_cart->get_is_rotated(); }
	int get_cart_type(UINT8 *ROM, UINT32 len, UINT32 &nvram_len);
	void internal_header_logging(UINT8 *ROM, UINT32 offs, UINT32 len);

	void save_nvram()   { if (m_cart && m_cart->get_nvram_size()) m_cart->save_nvram(); }

	virtual iodevice_t image_type() const override { return IO_CARTSLOT; }
	virtual bool is_readable()  const override { return 1; }
	virtual bool is_writeable() const override { return 0; }
	virtual bool is_creatable() const override { return 0; }
	virtual bool must_be_loaded() const override { return 1; }
	virtual bool is_reset_on_load() const override { return 1; }
	virtual const option_guide *create_option_guide() const override { return nullptr; }
	virtual const char *image_interface() const override { return "wswan_cart"; }
	virtual const char *file_extensions() const override { return "ws,wsc,bin"; }

	// slot interface overrides
	virtual std::string get_default_card_software() override;

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom20);
	virtual DECLARE_READ8_MEMBER(read_rom30);
	virtual DECLARE_READ8_MEMBER(read_rom40);
	virtual DECLARE_READ8_MEMBER(read_ram);
	virtual DECLARE_WRITE8_MEMBER(write_ram);
	virtual DECLARE_READ8_MEMBER(read_io);
	virtual DECLARE_WRITE8_MEMBER(write_io);

protected:

	int m_type;
	device_ws_cart_interface*       m_cart;
};



// device type definition
extern const device_type WS_CART_SLOT;


/***************************************************************************
 DEVICE CONFIGURATION MACROS
 ***************************************************************************/

#define WSSLOT_ROM_REGION_TAG ":cart:rom"

#define MCFG_WSWAN_CARTRIDGE_ADD(_tag,_slot_intf,_def_slot) \
	MCFG_DEVICE_ADD(_tag, WS_CART_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)
#endif
