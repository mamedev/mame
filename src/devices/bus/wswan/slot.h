// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_WSWAN_SLOT_H
#define MAME_BUS_WSWAN_SLOT_H

#pragma once

#include "softlist_dev.h"


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
	virtual ~device_ws_cart_interface();

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom20) { return 0xff; }
	virtual DECLARE_READ8_MEMBER(read_rom30) { return 0xff; }
	virtual DECLARE_READ8_MEMBER(read_rom40) { return 0xff; }
	virtual DECLARE_READ8_MEMBER(read_ram) { return 0xff; }
	virtual DECLARE_WRITE8_MEMBER(write_ram) {}
	virtual DECLARE_READ8_MEMBER(read_io) { return 0xff; }
	virtual DECLARE_WRITE8_MEMBER(write_io) { }

	void rom_alloc(uint32_t size, const char *tag);
	void nvram_alloc(uint32_t size);
	uint8_t* get_rom_base() { return m_rom; }
	uint8_t* get_nvram_base() { return &m_nvram[0]; }
	uint32_t get_rom_size() { return m_rom_size; }
	uint32_t get_nvram_size() { return m_nvram.size(); }

	void save_nvram() { device().save_item(NAME(m_nvram)); }
	void set_has_rtc(bool val) { m_has_rtc = val; }
	void set_is_rotated(bool val) { m_is_rotated = val; }
	int get_is_rotated() { return m_is_rotated ? 1 : 0; }

protected:
	device_ws_cart_interface(const machine_config &mconfig, device_t &device);

	// internal state
	uint8_t *m_rom;
	uint32_t m_rom_size;
	std::vector<uint8_t> m_nvram;
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
	template <typename T>
	ws_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&opts, const char *dflt)
		: ws_cart_slot_device(mconfig, tag, owner, clock)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	ws_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~ws_cart_slot_device();

	// image-level overrides
	virtual image_init_result call_load() override;
	virtual void call_unload() override;
	virtual const software_list_loader &get_software_list_loader() const override { return rom_software_list_loader::instance(); }

	int get_type() { return m_type; }
	int get_is_rotated() { return m_cart->get_is_rotated(); }
	int get_cart_type(const uint8_t *ROM, uint32_t len, uint32_t &nvram_len) const;
	void internal_header_logging(uint8_t *ROM, uint32_t offs, uint32_t len);

	void save_nvram()   { if (m_cart && m_cart->get_nvram_size()) m_cart->save_nvram(); }

	virtual iodevice_t image_type() const override { return IO_CARTSLOT; }
	virtual bool is_readable()  const override { return 1; }
	virtual bool is_writeable() const override { return 0; }
	virtual bool is_creatable() const override { return 0; }
	virtual bool must_be_loaded() const override { return 1; }
	virtual bool is_reset_on_load() const override { return 1; }
	virtual const char *image_interface() const override { return "wswan_cart"; }
	virtual const char *file_extensions() const override { return "ws,wsc,bin"; }

	// slot interface overrides
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom20);
	virtual DECLARE_READ8_MEMBER(read_rom30);
	virtual DECLARE_READ8_MEMBER(read_rom40);
	virtual DECLARE_READ8_MEMBER(read_ram);
	virtual DECLARE_WRITE8_MEMBER(write_ram);
	virtual DECLARE_READ8_MEMBER(read_io);
	virtual DECLARE_WRITE8_MEMBER(write_io);

protected:
	// device-level overrides
	virtual void device_start() override;

	int m_type;
	device_ws_cart_interface* m_cart;
};



// device type definition
DECLARE_DEVICE_TYPE(WS_CART_SLOT, ws_cart_slot_device)


/***************************************************************************
 DEVICE CONFIGURATION MACROS
 ***************************************************************************/

#define WSSLOT_ROM_REGION_TAG ":cart:rom"

#endif // MAME_BUS_WSWAN_SLOT_H
