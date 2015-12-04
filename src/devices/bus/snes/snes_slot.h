// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __SNS_SLOT_H
#define __SNS_SLOT_H

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

// ======================> device_sns_cart_interface

class device_sns_cart_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_sns_cart_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_sns_cart_interface();

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_l) { return 0xff; }   // ROM access in range [00-7f]
	virtual DECLARE_READ8_MEMBER(read_h) { return 0xff; }   // ROM access in range [80-ff]
	virtual DECLARE_READ8_MEMBER(read_ram) { if (!m_nvram.empty()) return m_nvram[offset & (m_nvram.size()-1)]; else return 0xff; }   // NVRAM access
	virtual DECLARE_WRITE8_MEMBER(write_l) {}   // used by carts with subslots
	virtual DECLARE_WRITE8_MEMBER(write_h) {}   // used by carts with subslots
	virtual DECLARE_WRITE8_MEMBER(write_ram) { if (!m_nvram.empty()) m_nvram[offset & (m_nvram.size()-1)] = data; } // NVRAM access
	virtual DECLARE_READ8_MEMBER(chip_read) { return 0xff; }
	virtual DECLARE_WRITE8_MEMBER(chip_write) {}
	virtual void speedup_addon_bios_access() {};

	void rom_alloc(UINT32 size, const char *tag);
	void nvram_alloc(UINT32 size);
	void rtc_ram_alloc(UINT32 size);
	void addon_bios_alloc(UINT32 size);
	UINT8* get_rom_base() { return m_rom; };
	UINT8* get_nvram_base() { return &m_nvram[0]; };
	UINT8* get_addon_bios_base() { return &m_bios[0]; };
	UINT8* get_rtc_ram_base() { return &m_rtc_ram[0]; };
	UINT32 get_rom_size() { return m_rom_size; };
	UINT32 get_nvram_size() { return m_nvram.size(); };
	UINT32 get_addon_bios_size() { return m_bios.size(); };
	UINT32 get_rtc_ram_size() { return m_rtc_ram.size(); };

	void rom_map_setup(UINT32 size);
	void save_nvram()   { device().save_item(NAME(m_nvram)); }
	void save_rtc_ram() { device().save_item(NAME(m_rtc_ram)); }

	// internal state
	UINT8 *m_rom;
	UINT32 m_rom_size;
	dynamic_buffer m_nvram;
	dynamic_buffer m_bios;
	dynamic_buffer m_rtc_ram;  // temp pointer to save RTC ram to nvram (will disappear when RTCs become devices)

	UINT8 rom_bank_map[256];    // 32K chunks of rom
};


// ======================> base_sns_cart_slot_device

class base_sns_cart_slot_device : public device_t,
								public device_image_interface,
								public device_slot_interface
{
public:
	// construction/destruction
	base_sns_cart_slot_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	virtual ~base_sns_cart_slot_device();

	// device-level overrides
	virtual void device_start();
	virtual void device_config_complete();

	// image-level overrides
	virtual bool call_load();
	virtual void call_unload();
	virtual bool call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry);

	void get_cart_type_addon(UINT8 *ROM, UINT32 len, int &type, int &addon);
	UINT32 snes_skip_header(UINT8 *ROM, UINT32 snes_rom_size);
	int get_type() { return m_type; }

	void setup_nvram();
	void internal_header_logging(UINT8 *ROM, UINT32 len);

	void save_ram() { if (m_cart && m_cart->get_nvram_size()) m_cart->save_nvram();
					if (m_cart && m_cart->get_rtc_ram_size()) m_cart->save_rtc_ram(); }

	virtual iodevice_t image_type() const { return IO_CARTSLOT; }
	virtual bool is_readable()  const { return 1; }
	virtual bool is_writeable() const { return 0; }
	virtual bool is_creatable() const { return 0; }
	virtual bool must_be_loaded() const { return 1; }
	virtual bool is_reset_on_load() const { return 1; }
	virtual const option_guide *create_option_guide() const { return nullptr; }

	// slot interface overrides
	virtual void get_default_card_software(std::string &result);

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_l);
	virtual DECLARE_READ8_MEMBER(read_h);
	virtual DECLARE_READ8_MEMBER(read_ram);
	virtual DECLARE_WRITE8_MEMBER(write_l);
	virtual DECLARE_WRITE8_MEMBER(write_h);
	virtual DECLARE_WRITE8_MEMBER(write_ram);
	virtual DECLARE_READ8_MEMBER(chip_read);
	virtual DECLARE_WRITE8_MEMBER(chip_write);

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
};

// ======================> sns_cart_slot_device

class sns_cart_slot_device :  public base_sns_cart_slot_device
{
public:
	// construction/destruction
	sns_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual const char *image_interface() const { return "snes_cart"; }
	virtual const char *file_extensions() const { return "sfc"; }
};

// ======================> sns_sufami_cart_slot_device

class sns_sufami_cart_slot_device :  public base_sns_cart_slot_device
{
public:
	// construction/destruction
	sns_sufami_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual const char *image_interface() const { return "st_cart"; }
	virtual const char *file_extensions() const { return "st"; }
	virtual bool must_be_loaded() const { return 0; }
};

// ======================> sns_sufami_cart_slot_device

class sns_bsx_cart_slot_device :  public base_sns_cart_slot_device
{
public:
	// construction/destruction
	sns_bsx_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual const char *image_interface() const { return "bspack"; }
	virtual const char *file_extensions() const { return "bs"; }
	virtual bool must_be_loaded() const { return 0; }
};


// device type definition
extern const device_type SNS_CART_SLOT;
extern const device_type SNS_SUFAMI_CART_SLOT;
extern const device_type SNS_BSX_CART_SLOT;


/***************************************************************************
 DEVICE CONFIGURATION MACROS
 ***************************************************************************/

#define SNSSLOT_ROM_REGION_TAG ":cart:rom"


#define MCFG_SNS_CARTRIDGE_ADD(_tag,_slot_intf,_def_slot) \
	MCFG_DEVICE_ADD(_tag, SNS_CART_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)

#define MCFG_SNS_SUFAMI_CARTRIDGE_ADD(_tag,_slot_intf,_def_slot) \
	MCFG_DEVICE_ADD(_tag, SNS_SUFAMI_CART_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)

#define MCFG_SNS_BSX_CARTRIDGE_ADD(_tag,_slot_intf,_def_slot) \
	MCFG_DEVICE_ADD(_tag, SNS_BSX_CART_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)


#endif
