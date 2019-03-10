// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_SNES_SNES_SLOT_H
#define MAME_BUS_SNES_SNES_SLOT_H

#pragma once

#include "softlist_dev.h"


/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/

// offset of add-on dumps inside snes_add/snesp_add bios, to support old dumps missing add-on data
#define SNES_DSP1_OFFSET  (0x00000)
#define SNES_DSP1B_OFFSET (0x03000)
#define SNES_DSP2_OFFSET  (0x06000)
#define SNES_DSP3_OFFSET  (0x09000)
#define SNES_DSP4_OFFSET  (0x0c000)
#define SNES_ST10_OFFSET  (0x0f000)
#define SNES_ST11_OFFSET  (0x20000)
#define SNES_CX4_OFFSET   (0x31000)
#define SNES_ST18_OFFSET1 (0x32000)
#define SNES_ST18_OFFSET2 (0x52000)


/* PCB */
enum
{
	SNES_MODE20 = 0,
	SNES_MODE21,
	SNES_MODE22,    // ExLoROM - not used anymore in emulation (only to log info), will be removed
	SNES_MODE25,    // ExHiROM - not used anymore in emulation (only to log info), will be removed
	SNES_CX4,
	SNES_DSP,
	SNES_DSP_2MB,
	SNES_DSP_MODE21,
	SNES_DSP4,
	SNES_OBC1,
	SNES_SA1,
	SNES_SDD1,
	SNES_SFX,
	SNES_SPC7110,
	SNES_SPC7110_RTC,
	SNES_SRTC,
	SNES_ST010,
	SNES_ST011,
	SNES_ST018,
	SNES_Z80GB,
	SNES_PFEST94,
	SNES_BSX,
	SNES_BSXLO,
	SNES_BSXHI,
	SNES_BSMEMPAK,
	SNES_SUFAMITURBO,
	SNES_STROM,
	// pirate carts
	SNES_POKEMON,
	SNES_TEKKEN2,
	SNES_SOULBLAD,
	SNES_MCPIR1,
	SNES_MCPIR2,
	SNES_20COL,
	SNES_BANANA,    // wip
	SNES_BUGS,   // wip
	// legacy types to support DSPx games from fullpath
	SNES_DSP1_LEG,
	SNES_DSP1B_LEG,
	SNES_DSP2_LEG,
	SNES_DSP3_LEG,
	SNES_DSP4_LEG,
	SNES_DSP1_MODE21_LEG,
	SNES_ST010_LEG,
	SNES_ST011_LEG
};

/* add-ons to handle legacy dumps in snes_add  */
enum
{
	ADDON_NONE = 0,
	ADDON_CX4,
	ADDON_DSP1,
	ADDON_DSP1B,
	ADDON_DSP2,
	ADDON_DSP3,
	ADDON_DSP4,
	ADDON_OBC1,
	ADDON_SA1,
	ADDON_SDD1,
	ADDON_SFX,
	ADDON_SPC7110,
	ADDON_SPC7110_RTC,
	ADDON_ST010,
	ADDON_ST011,
	ADDON_ST018,
	ADDON_SRTC,
	ADDON_Z80GB
};

class base_sns_cart_slot_device;

// ======================> device_sns_cart_interface

class device_sns_cart_interface : public device_slot_card_interface
{
	friend class base_sns_cart_slot_device;

public:
	// construction/destruction
	virtual ~device_sns_cart_interface();

	// reading and writing
	virtual uint8_t read_l(offs_t offset) { return 0xff; }   // ROM access in range [00-7f]
	virtual uint8_t read_h(offs_t offset) { return 0xff; }   // ROM access in range [80-ff]
	virtual uint8_t read_ram(offs_t offset) { if (!m_nvram.empty()) return m_nvram[offset & (m_nvram.size()-1)]; else return 0xff; }   // NVRAM access
	virtual void write_l(offs_t offset, uint8_t data) { }   // used by carts with subslots
	virtual void write_h(offs_t offset, uint8_t data) { }   // used by carts with subslots
	virtual void write_ram(offs_t offset, uint8_t data) { if (!m_nvram.empty()) m_nvram[offset & (m_nvram.size()-1)] = data; } // NVRAM access
	virtual uint8_t chip_read(offs_t offset) { return 0xff; }
	virtual void chip_write(offs_t offset, uint8_t data) { }
	virtual void speedup_addon_bios_access() {}

	void rom_alloc(uint32_t size, const char *tag);
	void nvram_alloc(uint32_t size);
	void rtc_ram_alloc(uint32_t size);
	void addon_bios_alloc(uint32_t size);
	uint8_t* get_rom_base() { return m_rom; }
	uint8_t* get_nvram_base() { return &m_nvram[0]; }
	uint8_t* get_addon_bios_base() { return &m_bios[0]; }
	uint8_t* get_rtc_ram_base() { return &m_rtc_ram[0]; }
	uint32_t get_rom_size() { return m_rom_size; }
	uint32_t get_nvram_size() { return m_nvram.size(); }
	uint32_t get_addon_bios_size() { return m_bios.size(); }
	uint32_t get_rtc_ram_size() { return m_rtc_ram.size(); }

	void rom_map_setup(uint32_t size);
	void save_nvram()   { device().save_item(NAME(m_nvram)); }
	void save_rtc_ram() { device().save_item(NAME(m_rtc_ram)); }

protected:
	device_sns_cart_interface(const machine_config &mconfig, device_t &device);

	DECLARE_WRITE_LINE_MEMBER(write_irq);
	uint8_t read_open_bus();

	// internal state
	uint8_t *m_rom;
	uint32_t m_rom_size;
	std::vector<uint8_t> m_nvram;
	std::vector<uint8_t> m_bios;
	std::vector<uint8_t> m_rtc_ram;  // temp pointer to save RTC ram to nvram (will disappear when RTCs become devices)

	uint8_t rom_bank_map[256];    // 32K chunks of rom

	base_sns_cart_slot_device *m_slot;
};


// ======================> base_sns_cart_slot_device

class base_sns_cart_slot_device : public device_t,
								public device_image_interface,
								public device_slot_interface
{
public:
	// construction/destruction
	virtual ~base_sns_cart_slot_device();

	// configuration
	auto irq_callback() { return m_irq_callback.bind(); }
	auto open_bus_callback() { return m_open_bus_callback.bind(); }

	// device-level overrides
	virtual void device_start() override;

	// image-level overrides
	virtual image_init_result call_load() override;
	virtual void call_unload() override;
	virtual const software_list_loader &get_software_list_loader() const override { return rom_software_list_loader::instance(); }

	void get_cart_type_addon(const uint8_t *ROM, uint32_t len, int &type, int &addon) const;
	uint32_t snes_skip_header(const uint8_t *ROM, uint32_t snes_rom_size) const;
	int get_type() { return m_type; }

	void setup_nvram();
	void internal_header_logging(uint8_t *ROM, uint32_t len);

	void save_ram() { if (m_cart && m_cart->get_nvram_size()) m_cart->save_nvram();
					if (m_cart && m_cart->get_rtc_ram_size()) m_cart->save_rtc_ram(); }

	virtual iodevice_t image_type() const override { return IO_CARTSLOT; }
	virtual bool is_readable()  const override { return 1; }
	virtual bool is_writeable() const override { return 0; }
	virtual bool is_creatable() const override { return 0; }
	virtual bool must_be_loaded() const override { return 1; }
	virtual bool is_reset_on_load() const override { return 1; }

	// slot interface overrides
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	// reading and writing
	uint8_t read_l(offs_t offset);
	uint8_t read_h(offs_t offset);
	uint8_t read_ram(offs_t offset);
	void write_l(offs_t offset, uint8_t data);
	void write_h(offs_t offset, uint8_t data);
	void write_ram(offs_t offset, uint8_t data);
	uint8_t chip_read(offs_t offset);
	void chip_write(offs_t offset, uint8_t data);

	DECLARE_WRITE_LINE_MEMBER(write_irq) { m_irq_callback(state); }
	uint8_t read_open_bus() { return m_open_bus_callback(); }

	// in order to support legacy dumps + add-on CPU dump appended at the end of the file, we
	// check if the required data is present and update bank map accordingly
	void setup_addon_from_fullpath();


// m_cart cannot be made private yet, because we need to check nvram_size from the driver...
// more work needed
//private:

	// this is used to support legacy DSPx/ST0xx/CX4 dumps not including the CPU data...
	// i.e. it's only used for snes_add/snesp_add
	int m_addon;

	int m_type;
	device_sns_cart_interface*      m_cart;

protected:
	base_sns_cart_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

private:
	devcb_write_line m_irq_callback;
	devcb_read8 m_open_bus_callback;
};

// ======================> sns_cart_slot_device

class sns_cart_slot_device : public base_sns_cart_slot_device
{
public:
	// construction/destruction
	template <typename T>
	sns_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&opts, const char *dflt)
		: sns_cart_slot_device(mconfig, tag, owner, clock)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	sns_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual const char *image_interface() const override { return "snes_cart"; }
	virtual const char *file_extensions() const override { return "sfc"; }
};

// ======================> sns_sufami_cart_slot_device

class sns_sufami_cart_slot_device : public base_sns_cart_slot_device
{
public:
	// construction/destruction
	template <typename T>
	sns_sufami_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: sns_sufami_cart_slot_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	sns_sufami_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual const char *image_interface() const override { return "st_cart"; }
	virtual const char *file_extensions() const override { return "st"; }
	virtual bool must_be_loaded() const override { return 0; }
};

// ======================> sns_sufami_cart_slot_device

class sns_bsx_cart_slot_device :  public base_sns_cart_slot_device
{
public:
	// construction/destruction
	template <typename T>
	sns_bsx_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: sns_bsx_cart_slot_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	sns_bsx_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual const char *image_interface() const override { return "bspack"; }
	virtual const char *file_extensions() const override { return "bs"; }
	virtual bool must_be_loaded() const override { return 0; }
};


// device type definition
DECLARE_DEVICE_TYPE(SNS_CART_SLOT,        sns_cart_slot_device)
DECLARE_DEVICE_TYPE(SNS_SUFAMI_CART_SLOT, sns_sufami_cart_slot_device)
DECLARE_DEVICE_TYPE(SNS_BSX_CART_SLOT,    sns_bsx_cart_slot_device)


/***************************************************************************
 DEVICE CONFIGURATION MACROS
 ***************************************************************************/

#define SNSSLOT_ROM_REGION_TAG ":cart:rom"


#endif // MAME_BUS_SNES_SNES_SLOT_H
