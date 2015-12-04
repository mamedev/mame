// license:BSD-3-Clause
// copyright-holders:Fabio Priuli, Wilbert Pol
#ifndef __GB_SLOT_H
#define __GB_SLOT_H

/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/


/* PCB */
enum
{
	GB_MBC_NONE = 0,     /*  32KB ROM - No memory bank controller         */
	GB_MBC_MBC1,         /*  ~2MB ROM,   8KB RAM -or- 512KB ROM, 32KB RAM */
	GB_MBC_MBC2,         /* 256KB ROM,  32KB RAM                          */
	GB_MBC_MBC3,         /*   2MB ROM,  32KB RAM, RTC                     */
	GB_MBC_MBC4,         /*    ?? ROM,    ?? RAM                          */
	GB_MBC_MBC5,         /*   8MB ROM, 128KB RAM (32KB w/ Rumble)         */
	GB_MBC_TAMA5,        /*    ?? ROM     ?? RAM - What is this?          */
	GB_MBC_HUC1,         /*    ?? ROM,    ?? RAM - Hudson Soft Controller */
	GB_MBC_HUC3,         /*    ?? ROM,    ?? RAM - Hudson Soft Controller */
	GB_MBC_MBC6,         /*    ?? ROM,  32KB SRAM                         */
	GB_MBC_MBC7,         /*    ?? ROM,    ?? RAM                          */
	GB_MBC_M161,         /*    ?? ROM,    ?? RAM                          */
	GB_MBC_MMM01,        /*    ?? ROM,    ?? RAM                          */
	GB_MBC_WISDOM,       /*    ?? ROM,    ?? RAM - Wisdom tree controller */
	GB_MBC_MBC1_COL,     /*   1MB ROM,  32KB RAM - workaround for MBC1 on PCB that maps rom address lines differently */
	GB_MBC_SACHEN1,      /*    ?? ROM,    ?? RAM - Sachen MMC-1 variant */
	GB_MBC_SACHEN2,      /*    ?? ROM,    ?? RAM - Sachen MMC-2 variant */
	GB_MBC_YONGYONG,     /*    ?? ROM,    ?? RAM - Appears in Sonic 3D Blast 5 pirate */
	GB_MBC_LASAMA,       /*    ?? ROM,    ?? RAM - Appears in La Sa Ma */
	GB_MBC_ATVRACIN,
	GB_MBC_CAMERA,
	GB_MBC_188IN1,
	GB_MBC_SINTAX,
	GB_MBC_CHONGWU,
	GB_MBC_LICHENG,
	GB_MBC_DIGIMON,
	GB_MBC_ROCKMAN8,
	GB_MBC_SM3SP,
	GB_MBC_DKONG5,
	GB_MBC_UNK01,
	GB_MBC_MEGADUCK,     /* MEGADUCK style banking                        */
	GB_MBC_UNKNOWN       /* Unknown mapper                                */
};


// ======================> device_gb_cart_interface

class device_gb_cart_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_gb_cart_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_gb_cart_interface();

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom) { return 0xff; }
	virtual DECLARE_WRITE8_MEMBER(write_bank) {}
	virtual DECLARE_READ8_MEMBER(read_ram) { return 0xff; }
	virtual DECLARE_WRITE8_MEMBER(write_ram) {}

	void rom_alloc(UINT32 size, const char *tag);
	void ram_alloc(UINT32 size);
	UINT8* get_rom_base() { return m_rom; }
	UINT8* get_ram_base() { return &m_ram[0]; }
	UINT32 get_rom_size() { return m_rom_size; }
	UINT32 get_ram_size() { return m_ram.size(); }

	void rom_map_setup(UINT32 size);
	void ram_map_setup(UINT8 banks);

	virtual void set_additional_wirings(UINT8 mask, int shift) { }  // MBC-1 will then overwrite this!
	void set_has_timer(bool val) { has_timer = val; }
	void set_has_rumble(bool val) { has_rumble = val; }
	void set_has_battery(bool val) { has_battery = val; }
	bool get_has_battery() { return has_battery; }

	void save_ram() { device().save_item(NAME(m_ram)); }

	// internal state
	UINT8 *m_rom;
	UINT32 m_rom_size;
	dynamic_buffer m_ram;

	// bankswitch variables
	// we access ROM/RAM banks through these bank maps
	// default accesses are:
	// 0x0000-0x3fff = rom_bank_map[m_latch_bank]   (generally defaults to m_latch_bank = 0)
	// 0x4000-0x7fff = rom_bank_map[m_latch_bank2]  (generally defaults to m_latch_bank2 = 1)
	// 0xa000-0xbfff = ram_bank_map[m_ram_bank]   (generally defaults to m_ram_bank = 0)
	// suitable writes to 0x0000-0x7fff can then modify m_latch_bank/m_latch_bank2
	UINT8 rom_bank_map[512];    // 16K chunks of ROM
	UINT8 ram_bank_map[256];    // 16K chunks of RAM
	UINT8 m_ram_bank;
	UINT16 m_latch_bank, m_latch_bank2;

	bool has_rumble, has_timer, has_battery;
};


// ======================> base_gb_cart_slot_device

class base_gb_cart_slot_device : public device_t,
								public device_image_interface,
								public device_slot_interface
{
public:
	// construction/destruction
	base_gb_cart_slot_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	virtual ~base_gb_cart_slot_device();

	// device-level overrides
	virtual void device_start();
	virtual void device_config_complete();

	// image-level overrides
	virtual bool call_load();
	virtual void call_unload();
	virtual bool call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry);

	int get_type() { return m_type; }
	int get_cart_type(UINT8 *ROM, UINT32 len);
	bool get_mmm01_candidate(UINT8 *ROM, UINT32 len);
	// remove me when SGB is properly emulated
	int get_sgb_hack() { return m_sgb_hack; }

	void setup_ram(UINT8 banks);
	void internal_header_logging(UINT8 *ROM, UINT32 len);
	void save_ram() { if (m_cart && m_cart->get_ram_size()) m_cart->save_ram(); }

	virtual iodevice_t image_type() const { return IO_CARTSLOT; }
	virtual bool is_readable()  const { return 1; }
	virtual bool is_writeable() const { return 0; }
	virtual bool is_creatable() const { return 0; }
	virtual bool must_be_loaded() const { return 0; }
	virtual bool is_reset_on_load() const { return 1; }
	virtual const option_guide *create_option_guide() const { return nullptr; }
	virtual const char *image_interface() const { return "gameboy_cart"; }
	virtual const char *file_extensions() const { return "bin,gb,gbc"; }

	// slot interface overrides
	virtual void get_default_card_software(std::string &result);

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom);
	virtual DECLARE_WRITE8_MEMBER(write_bank);
	virtual DECLARE_READ8_MEMBER(read_ram);
	virtual DECLARE_WRITE8_MEMBER(write_ram);


protected:
	// Donkey Kong Land 2 + 3 store SGB border tiles differently... this will be hopefully be removed when SGB is properly emulated!
	int m_sgb_hack;

	int m_type;
	device_gb_cart_interface*       m_cart;
};

// ======================> gb_cart_slot_device

class gb_cart_slot_device :  public base_gb_cart_slot_device
{
public:
	// construction/destruction
	gb_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


// ======================> megaduck_cart_slot_device

class megaduck_cart_slot_device :  public base_gb_cart_slot_device
{
public:
	// construction/destruction
	megaduck_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// image-level overrides
	virtual bool call_load();
	virtual const char *image_interface() const { return "megaduck_cart"; }
	virtual const char *file_extensions() const { return "bin"; }

	// slot interface overrides
	virtual void get_default_card_software(std::string &result);
};




// device type definition
extern const device_type GB_CART_SLOT;
extern const device_type MEGADUCK_CART_SLOT;


/***************************************************************************
 DEVICE CONFIGURATION MACROS
 ***************************************************************************/

#define GBSLOT_ROM_REGION_TAG ":cart:rom"

#define MCFG_GB_CARTRIDGE_ADD(_tag,_slot_intf,_def_slot) \
	MCFG_DEVICE_ADD(_tag, GB_CART_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)

#define MCFG_MEGADUCK_CARTRIDGE_ADD(_tag,_slot_intf,_def_slot) \
	MCFG_DEVICE_ADD(_tag, MEGADUCK_CART_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)


#endif
