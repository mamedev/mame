#ifndef __SNS_SLOT_H
#define __SNS_SLOT_H

/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/


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
	SNES_BUGS   // wip
};


// ======================> sns_cart_interface

struct sns_cart_interface
{
};


// ======================> device_sns_cart_interface

class device_sns_cart_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_sns_cart_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_sns_cart_interface();

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_l) { return 0xff; }	// ROM access in range [00-7f]
	virtual DECLARE_READ8_MEMBER(read_h) { return 0xff; }	// ROM access in range [80-ff]
	virtual DECLARE_READ8_MEMBER(read_ram) { UINT32 mask = m_nvram_size - 1; return m_nvram[offset & mask]; }	// NVRAM access
	virtual DECLARE_WRITE8_MEMBER(write_l) {}	// used by carts with subslots
	virtual DECLARE_WRITE8_MEMBER(write_h) {}	// used by carts with subslots
	virtual DECLARE_WRITE8_MEMBER(write_ram) { UINT32 mask = m_nvram_size - 1; m_nvram[offset & mask] = data; return; }	// NVRAM access
	virtual DECLARE_READ8_MEMBER(chip_read) { return 0xff; }
	virtual DECLARE_WRITE8_MEMBER(chip_write) {}

	void rom_alloc(running_machine &machine, UINT32 size);
	void nvram_alloc(running_machine &machine, UINT32 size);
	void rtc_ram_alloc(running_machine &machine, UINT32 size);
	void addon_bios_alloc(running_machine &machine, UINT32 size);
	UINT8* get_rom_base() { return m_rom; };
	UINT8* get_nvram_base() { return m_nvram; };
	UINT8* get_addon_bios_base() { return m_bios; };
	UINT8* get_rtc_ram_base() { return m_rtc_ram; };
	UINT32 get_rom_size() { return m_rom_size; };
	UINT32 get_nvram_size() { return m_nvram_size; };
	UINT32 get_addon_bios_size() { return m_bios_size; };
	UINT32 get_rtc_ram_size() { return m_rtc_ram_size; };

	void rom_map_setup(UINT32 size);

	// internal state
	UINT8  *m_rom;
	UINT8  *m_nvram;
	UINT8  *m_bios;
	UINT8  *m_rtc_ram;  // temp pointer to save RTC ram to nvram (will disappear when RTCs become devices)
	UINT32 m_rom_size;
	UINT32 m_nvram_size;
	UINT32 m_bios_size;
	UINT32 m_rtc_ram_size;  // temp

	UINT8 rom_bank_map[256];    // 32K chunks of rom
};


// ======================> base_sns_cart_slot_device

class base_sns_cart_slot_device : public device_t,
								public sns_cart_interface,
								public device_image_interface,
								public device_slot_interface
{
public:
	// construction/destruction
	base_sns_cart_slot_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);
	virtual ~base_sns_cart_slot_device();

	// device-level overrides
	virtual void device_start();
	virtual void device_config_complete();

	// image-level overrides
	virtual bool call_load();
	virtual void call_unload();
	virtual bool call_softlist_load(char *swlist, char *swname, rom_entry *start_entry);

	int get_cart_type(UINT8 *ROM, UINT32 len);
	UINT32 snes_skip_header(UINT8 *ROM, UINT32 snes_rom_size);
	int get_type() { return m_type; }

	void setup_custom_mappers();
	void setup_nvram();
	void internal_header_logging(UINT8 *ROM, UINT32 len);

	virtual iodevice_t image_type() const { return IO_CARTSLOT; }
	virtual bool is_readable()  const { return 1; }
	virtual bool is_writeable() const { return 0; }
	virtual bool is_creatable() const { return 0; }
	virtual bool must_be_loaded() const { return 1; }
	virtual bool is_reset_on_load() const { return 1; }
	virtual const option_guide *create_option_guide() const { return NULL; }

	// slot interface overrides
	virtual const char * get_default_card_software(const machine_config &config, emu_options &options);

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_l);
	virtual DECLARE_READ8_MEMBER(read_h);
	virtual DECLARE_READ8_MEMBER(read_ram);
	virtual DECLARE_WRITE8_MEMBER(write_l);
	virtual DECLARE_WRITE8_MEMBER(write_h);
	virtual DECLARE_WRITE8_MEMBER(write_ram);
	virtual DECLARE_READ8_MEMBER(chip_read);
	virtual DECLARE_WRITE8_MEMBER(chip_write);

// m_cart cannot be made private yet, because we need to check nvram_size from the driver...
// more work needed
//private:

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

#define MCFG_SNS_CARTRIDGE_ADD(_tag,_slot_intf,_def_slot,_def_inp) \
	MCFG_DEVICE_ADD(_tag, SNS_CART_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, _def_inp, false)

#define MCFG_SNS_SUFAMI_CARTRIDGE_ADD(_tag,_slot_intf,_def_slot,_def_inp) \
	MCFG_DEVICE_ADD(_tag, SNS_SUFAMI_CART_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, _def_inp, false)

#define MCFG_SNS_BSX_CARTRIDGE_ADD(_tag,_slot_intf,_def_slot,_def_inp) \
	MCFG_DEVICE_ADD(_tag, SNS_BSX_CART_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, _def_inp, false)


#endif
