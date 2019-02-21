// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_SEGA8_SLOT_H
#define MAME_BUS_SEGA8_SLOT_H

#pragma once

#include "softlist_dev.h"


/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/


/* PCB */
enum
{
	SEGA8_BASE_ROM = 0,
	SEGA8_EEPROM,
	SEGA8_TEREBIOEKAKI,
	SEGA8_4PAK,
	SEGA8_CODEMASTERS,
	SEGA8_ZEMINA,
	SEGA8_NEMESIS,
	SEGA8_JANGGUN,
	SEGA8_KOREAN,
	SEGA8_KOREAN_NOBANK,
	SEGA8_OTHELLO,
	SEGA8_CASTLE,
	SEGA8_BASIC_L3,
	SEGA8_MUSIC_EDITOR,
	SEGA8_DAHJEE_TYPEA,
	SEGA8_DAHJEE_TYPEB,
	SEGA8_SEOJIN,
	SEGA8_MULTICART,
	SEGA8_MEGACART
};


DECLARE_DEVICE_TYPE(SEGA8_CART_SLOT, sega8_cart_slot_device)
DECLARE_DEVICE_TYPE(SEGA8_CARD_SLOT, sega8_card_slot_device)


// ======================> device_sega8_cart_interface

class device_sega8_cart_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	virtual ~device_sega8_cart_interface();

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_cart) { return 0xff; }
	virtual DECLARE_WRITE8_MEMBER(write_cart) { }
	virtual DECLARE_WRITE8_MEMBER(write_mapper) { }
	virtual int get_lphaser_xoffs() { return m_lphaser_xoffs; }
	// a few carts (for SG1000) acts as a RAM expansion, taking control of the system RAM in 0xc000-0xffff
	virtual DECLARE_READ8_MEMBER(read_ram) { return 0xff; }
	virtual DECLARE_WRITE8_MEMBER(write_ram) { }
	// the SC3000 has I/OR, I/OW lines connected
	virtual DECLARE_READ8_MEMBER(read_io) { return 0xff; }
	virtual DECLARE_WRITE8_MEMBER(write_io) { }

	void rom_alloc(uint32_t size, const char *tag);
	void ram_alloc(uint32_t size);

	virtual void late_bank_setup() { }

	void set_has_battery(bool val) { has_battery = val; }
	bool get_has_battery() { return has_battery; }
	void set_late_battery(bool val) { m_late_battery_enable = val; }
	bool get_late_battery() { return m_late_battery_enable; }
	void set_lphaser_xoffs(int val) { m_lphaser_xoffs = val; }
	void set_sms_mode(int val) { m_sms_mode = val; }
	int get_sms_mode() { return m_sms_mode; }

//protected:
	uint8_t* get_rom_base() { return m_rom; }
	uint8_t* get_ram_base() { return &m_ram[0]; }
	uint32_t get_rom_size() { return m_rom_size; }
	uint32_t get_ram_size() { return m_ram.size(); }

	void save_ram() { device().save_item(NAME(m_ram)); }

protected:
	device_sega8_cart_interface(const machine_config &mconfig, device_t &device);

	// internal state
	uint8_t *m_rom;
	uint32_t m_rom_size;
	std::vector<uint8_t> m_ram;
	int m_rom_page_count;

	bool has_battery;

	// we use this variable for fullpath loading only: in this case, RAM is always allocated,
	// but we set has_battery only if RAM is actually enabled during game...
	bool m_late_battery_enable;

	int m_lphaser_xoffs;
	int m_sms_mode;
};


// ======================> sega8_cart_slot_device

class sega8_cart_slot_device : public device_t,
								public device_image_interface,
								public device_slot_interface
{
public:
	// construction/destruction
	sega8_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~sega8_cart_slot_device();

	// image-level overrides
	virtual image_init_result call_load() override;
	virtual void call_unload() override;
	virtual const software_list_loader &get_software_list_loader() const override { return rom_software_list_loader::instance(); }
	virtual const char *custom_instance_name() const override { return "cartridge"; }
	virtual const char *custom_brief_instance_name() const override { return "cart"; }

	int get_type() { return m_type; }
	int get_cart_type(const uint8_t *ROM, uint32_t len) const;

	void setup_ram();
	void internal_header_logging(uint8_t *ROM, uint32_t len, uint32_t nvram_len);
	image_verify_result verify_cart(uint8_t *magic, int size);
	void set_lphaser_xoffset(uint8_t *rom, int size);

	void save_ram() { if (m_cart && m_cart->get_ram_size()) m_cart->save_ram(); }

	virtual iodevice_t image_type() const override { return IO_CARTSLOT; }
	virtual bool is_readable()  const override { return 1; }
	virtual bool is_writeable() const override { return 0; }
	virtual bool is_creatable() const override { return 0; }
	virtual bool is_reset_on_load() const override { return 1; }
	virtual bool must_be_loaded() const override { return false; }
	virtual const char *image_interface() const override { return "sms_cart"; }
	virtual const char *file_extensions() const override { return "bin"; }

	// slot interface overrides
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_cart);
	virtual DECLARE_WRITE8_MEMBER(write_cart);
	virtual DECLARE_WRITE8_MEMBER(write_mapper);
	virtual DECLARE_READ8_MEMBER(read_ram);
	virtual DECLARE_WRITE8_MEMBER(write_ram);
	virtual DECLARE_READ8_MEMBER(read_io);
	virtual DECLARE_WRITE8_MEMBER(write_io);

	int get_lphaser_xoffs() { return m_cart ? m_cart->get_lphaser_xoffs() : -1; }
	int get_sms_mode() { return m_cart->get_sms_mode(); }

protected:
	sega8_cart_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, bool is_card = false);

	// device-level overrides
	virtual void device_start() override;

	int m_type;
	bool m_is_card;
	device_sega8_cart_interface*       m_cart;
};

// ======================> sega8_card_slot_device

class sega8_card_slot_device : public sega8_cart_slot_device
{
public:
	// construction/destruction
	sega8_card_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual const char *custom_instance_name() const override { return "card"; }
	virtual const char *custom_brief_instance_name() const override { return "card"; }

protected:
	// construction/destruction
	sega8_card_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
};

// ======================> sg1000_cart_slot_device

class sg1000_cart_slot_device : public sega8_cart_slot_device
{
public:
	// construction/destruction
	template <typename T>
	sg1000_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: sg1000_cart_slot_device(mconfig, tag, owner, u32(0))
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	sg1000_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual bool must_be_loaded() const override { return true; }
	virtual const char *image_interface() const override { return "sg1000_cart"; }
	virtual const char *file_extensions() const override { return "bin,sg"; }
};

// ======================> omv_cart_slot_device

class omv_cart_slot_device : public sega8_cart_slot_device
{
public:
	// construction/destruction
	template <typename T>
	omv_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: omv_cart_slot_device(mconfig, tag, owner, u32(0))
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	omv_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual bool must_be_loaded() const override { return false; }
	virtual const char *image_interface() const override { return "sg1000_cart"; }
	virtual const char *file_extensions() const override { return "bin,sg"; }
};

// ======================> sc3000_cart_slot_device

class sc3000_cart_slot_device : public sega8_cart_slot_device
{
public:
	// construction/destruction
	template <typename T>
	sc3000_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: sc3000_cart_slot_device(mconfig, tag, owner, u32(0))
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	sc3000_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual bool must_be_loaded() const override { return true; }
	virtual const char *image_interface() const override { return "sg1000_cart"; }
	virtual const char *file_extensions() const override { return "bin,sg,sc"; }
};

// ======================> sg1000mk3_cart_slot_device

class sg1000mk3_cart_slot_device : public sega8_cart_slot_device
{
public:
	// construction/destruction
	template <typename T>
	sg1000mk3_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: sg1000mk3_cart_slot_device(mconfig, tag, owner, u32(0))
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	sg1000mk3_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual bool must_be_loaded() const override { return false; }
	virtual const char *image_interface() const override { return "sms_cart,sg1000_cart"; }
	virtual const char *file_extensions() const override { return "bin,sms,sg"; }
};

// ======================> sms_cart_slot_device

class sms_cart_slot_device : public sega8_cart_slot_device
{
public:
	// construction/destruction
	template <typename T>
	sms_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: sms_cart_slot_device(mconfig, tag, owner, u32(0))
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	sms_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual bool must_be_loaded() const override { return false; }
	virtual const char *image_interface() const override { return "sms_cart"; }
	virtual const char *file_extensions() const override { return "bin,sms"; }
};

// ======================> gamegear_cart_slot_device

class gamegear_cart_slot_device : public sega8_cart_slot_device
{
public:
	// construction/destruction
	template <typename T>
	gamegear_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: gamegear_cart_slot_device(mconfig, tag, owner, u32(0))
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	gamegear_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual bool must_be_loaded() const override { return true; }
	virtual const char *image_interface() const override { return "gamegear_cart"; }
	virtual const char *file_extensions() const override { return "bin,gg"; }
};


// ======================> sms_card_slot_device

class sms_card_slot_device : public sega8_card_slot_device
{
public:
	// construction/destruction
	template <typename T>
	sms_card_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: sms_card_slot_device(mconfig, tag, owner, u32(0))
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	sms_card_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual bool must_be_loaded() const override { return false; }
	virtual const char *image_interface() const override { return "sms_card"; }
	virtual const char *file_extensions() const override { return "bin"; }
};

// ======================> sg1000_card_slot_device

class sg1000_card_slot_device : public sega8_card_slot_device
{
public:
	// construction/destruction
	template <typename T>
	sg1000_card_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: sg1000_card_slot_device(mconfig, tag, owner, u32(0))
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	sg1000_card_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual const char *image_interface() const override { return "sg1000_cart"; }
	virtual const char *file_extensions() const override { return "bin,sg"; }
};

DECLARE_DEVICE_TYPE(SG1000_CART_SLOT,    sg1000_cart_slot_device)
DECLARE_DEVICE_TYPE(OMV_CART_SLOT,       omv_cart_slot_device)
DECLARE_DEVICE_TYPE(SC3000_CART_SLOT,    sc3000_cart_slot_device)
DECLARE_DEVICE_TYPE(SG1000MK3_CART_SLOT, sg1000mk3_cart_slot_device)
DECLARE_DEVICE_TYPE(SMS_CART_SLOT,       sms_cart_slot_device)
DECLARE_DEVICE_TYPE(GAMEGEAR_CART_SLOT,  gamegear_cart_slot_device)
DECLARE_DEVICE_TYPE(SMS_CARD_SLOT,       sms_card_slot_device)
DECLARE_DEVICE_TYPE(SG1000_CARD_SLOT,    sg1000_card_slot_device)

/***************************************************************************
 DEVICE CONFIGURATION MACROS
 ***************************************************************************/

#define S8SLOT_ROM_REGION_TAG ":cart:rom"


// slot interfaces
void sg1000_cart(device_slot_interface &device);
void sg1000mk3_cart(device_slot_interface &device);
void sms_cart(device_slot_interface &device);
void gg_cart(device_slot_interface &device);

#endif // MAME_BUS_SEGA8_SLOT_H
