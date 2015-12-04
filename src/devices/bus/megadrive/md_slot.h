// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __MD_SLOT_H
#define __MD_SLOT_H

/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/

#define MD_ADDR(a)  (rom_bank_map[((a << 1) / 0x10000) & 0x3f] * 0x10000 + ((a << 1) & 0xffff))/2

/* PCB */
enum
{
	SEGA_STD = 0,

	// Cart + Slot Expansion
	SEGA_SK,                     /* Sonic & Knuckles pass-through cart */

	// Cart + SVP
	SEGA_SVP,                    /* Virtua Racing */

	// Cart + NVRAM
	SEGA_SRAM, SEGA_FRAM,
	HARDBALL95,                  /* Hardball 95 uses different sram start address */
	XINQIG,                   /* Xin Qigai Wangzi uses different sram start address and has no valid header */
	BEGGARP,                     /* Beggar Prince uses different sram start address + bankswitch tricks */
	WUKONG,                      /* Legend of Wukong uses different sram start address + bankswitch trick for last 128K of ROM */

	// EEPROM
	SEGA_EEPROM,                 /* Wonder Boy V / Evander Holyfield's Boxing / Greatest Heavyweights of the Ring / Sports Talk Baseball / Megaman */
	NBA_JAM,                     /* NBA Jam */
	NBA_JAM_TE,                  /* NBA Jam TE / NFL Quarterback Club */
	NFL_QB_96,                   /* NFL Quarterback Club '96 */
	C_SLAM,                      /* College Slam / Frank Thomas Big Hurt Baseball */
	EA_NHLPA,                    /* NHLPA Hockey 93 / Rings of Power */
	BRIAN_LARA,                  /* Brian Lara Cricket 96 */
	PSOLAR,                      /* Pier Solar (STM95 EEPROM) */

	// J-Cart
	CM_JCART,                    /* Pete Sampras Tennis */
	CODE_MASTERS,                /* Micro Machines 2 / Military (J-Cart + SEPROM)  */
	CM_MM96,                     /* Micro Machines 96 (J-Cart + SEPROM, diff I2C model)  */

	// Various
	SSF2,                        /* Super Street Fighter 2 */
	CM_2IN1,                     /* CodeMasters 2in1 : Psycho Pinball + Micro Machines */
	GAME_KANDUME,                /* Game no Kandume Otokuyou */
	RADICA,                      /* Radica TV games.. these probably should be a separate driver since they are a separate 'console' */

	TILESMJ2,                    /* 16 Mahjong Tiles II */
	BUGSLIFE,                    /* A Bug's Life */
	CHINFIGHT3,                  /* Chinese Fighters 3 */
	ELFWOR,                      /* Linghuan Daoshi Super Magician */
	KAIJU,                       /* Pokemon Stadium */
	KOF98,                       /* King of Fighters '98 */
	KOF99,                       /* King of Fighters '99 */
	LIONK2,                      /* Lion King 2 */
	LIONK3,                      /* Lion King 3, Super Donkey Kong 99, Super King Kong 99 */
	MC_PIRATE,                   /* Super 19 in 1, Super 15 in 1, 12 in 1 and a few more multicarts */
	MJLOVER,                     /* Mahjong Lover */
	POKEMONA,                    /* Pocket Monster Alt Protection */
	REALTEC,                     /* Whac a Critter/Mallet legend, Defend the Earth, Funnyworld/Ballonboy */
	REDCLIFF,                    /* Romance of the Three Kingdoms - Battle of Red Cliffs, already decoded from .mdx format */
	REDCL_EN,                    /* The encoded version... */
	ROCKMANX3,                   /* Rockman X3 */
	SBUBBOB,                     /* Super Bubble Bobble */
	SMB,                         /* Super Mario Bros. */
	SMB2,                        /* Super Mario Bros. 2 */
	SMW64,                       /* Super Mario World 64 */
	SMOUSE,                      /* Smart Mouse */
	SOULBLAD,                    /* Soul Blade */
	SQUIRRELK,                   /* Squirrel King */
	TEKKENSP,                    /* Tekken Special */
	TOPFIGHTER,                  /* Top Fighter 2000 MK VIII */

	// when loading from fullpath, we need to treat SRAM in custom way
	SEGA_SRAM_FULLPATH,
	SEGA_SRAM_FALLBACK
};


// ======================> device_md_cart_interface

class device_md_cart_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_md_cart_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_md_cart_interface();

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read) { return 0xffff; }
	virtual DECLARE_WRITE16_MEMBER(write) {}
	virtual DECLARE_READ16_MEMBER(read_a13) { return 0xffff; }
	virtual DECLARE_WRITE16_MEMBER(write_a13) {}
	virtual DECLARE_READ16_MEMBER(read_a15) { return 0xffff; }
	virtual DECLARE_WRITE16_MEMBER(write_a15) {}

	virtual int read_test() { return 0; }   // used by Virtua Racing test

	/* this probably should do more, like make Genesis V2 'die' if the SEGA string is not written promptly */
	virtual DECLARE_WRITE16_MEMBER(write_tmss_bank) { m_device.logerror("Write to TMSS bank: offset %x data %x\n", 0xa14000 + (offset << 1), data); };

	virtual void rom_alloc(size_t size, const char *tag);
	virtual void nvram_alloc(size_t size);
	virtual UINT16* get_rom_base() { return m_rom; };
	virtual UINT16* get_nvram_base() { return &m_nvram[0]; };
	virtual UINT32 get_rom_size() { return m_rom_size; };
	virtual UINT32 get_nvram_size() { return m_nvram.size()*sizeof(UINT16); };
	virtual void set_bank_to_rom(const char *banktag, UINT32 offset) {};

	void save_nvram() { device().save_item(NAME(m_nvram)); }

	void rom_map_setup(UINT32 size);
	UINT32 get_padded_size(UINT32 size);

	int m_nvram_start, m_nvram_end;
	int m_nvram_active, m_nvram_readonly;

	// when loading from fullpath, we create NVRAM even if not set in the header
	// however in this case we access it only if the game turn it on
	// the variable below is basically needed to track this...
	int m_nvram_handlers_installed;

	// internal state
	UINT16  *m_rom;
	UINT32  m_rom_size;
	std::vector<UINT16> m_nvram;

	UINT8 rom_bank_map[128];    // 64K chunks of rom
};


// ======================> base_md_cart_slot_device

class base_md_cart_slot_device : public device_t,
								public device_image_interface,
								public device_slot_interface
{
public:
	// construction/destruction
	base_md_cart_slot_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	virtual ~base_md_cart_slot_device();

	// device-level overrides
	virtual void device_start();
	virtual void device_config_complete();

	// image-level overrides
	virtual bool call_load();
	virtual void call_unload();
	virtual bool call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry);

	virtual iodevice_t image_type() const { return IO_CARTSLOT; }
	virtual bool is_readable()  const { return 1; }
	virtual bool is_writeable() const { return 0; }
	virtual bool is_creatable() const { return 0; }
	virtual bool must_be_loaded() const { return m_must_be_loaded; }
	virtual bool is_reset_on_load() const { return 1; }
	virtual const option_guide *create_option_guide() const { return nullptr; }

	// slot interface overrides
	virtual void get_default_card_software(std::string &result);

	int get_type() { return m_type; }

	int load_list();
	int load_nonlist();
	int get_cart_type(UINT8 *ROM, UINT32 len);

	void setup_custom_mappers();
	void setup_nvram();
	void set_must_be_loaded(bool _must_be_loaded) { m_must_be_loaded = _must_be_loaded; }
	void file_logging(UINT8 *ROM, UINT32 rom_len, UINT32 nvram_len);

	void save_nvram() { if (m_cart && m_cart->get_nvram_size()) m_cart->save_nvram(); }

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read);
	virtual DECLARE_WRITE16_MEMBER(write);
	virtual DECLARE_READ16_MEMBER(read_a13);
	virtual DECLARE_WRITE16_MEMBER(write_a13);
	virtual DECLARE_READ16_MEMBER(read_a15);
	virtual DECLARE_WRITE16_MEMBER(write_a15);
	virtual DECLARE_WRITE16_MEMBER(write_tmss_bank) { if (m_cart) m_cart->write_tmss_bank(space, offset, data, mem_mask); };

	virtual int read_test() { if (m_cart) return m_cart->read_test(); else return 0; }  // used by Virtua Racing test

// TODO: this only needs to be public because megasvp copies rom into memory region, so we need to rework that code...
//private:

	int m_type;
	device_md_cart_interface*       m_cart;
	bool                            m_must_be_loaded;
};

// ======================> md_cart_slot_device

class md_cart_slot_device :  public base_md_cart_slot_device
{
public:
	// construction/destruction
	md_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual const char *image_interface() const { return "megadriv_cart"; }
	virtual const char *file_extensions() const { return "smd,bin,md,gen"; }
};

// ======================> pico_cart_slot_device

class pico_cart_slot_device :  public base_md_cart_slot_device
{
public:
	// construction/destruction
	pico_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual const char *image_interface() const { return "pico_cart"; }
	virtual const char *file_extensions() const { return "bin,md"; }
};

// ======================> copera_cart_slot_device

class copera_cart_slot_device :  public base_md_cart_slot_device
{
public:
	// construction/destruction
	copera_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual const char *image_interface() const { return "copera_cart"; }
	virtual const char *file_extensions() const { return "bin,md"; }
};


// device type definition
extern const device_type MD_CART_SLOT;
extern const device_type PICO_CART_SLOT;
extern const device_type COPERA_CART_SLOT;


/***************************************************************************
 DEVICE CONFIGURATION MACROS
 ***************************************************************************/

#define MDSLOT_ROM_REGION_TAG ":cart:rom"

#define MCFG_MD_CARTRIDGE_ADD(_tag,_slot_intf,_def_slot) \
	MCFG_DEVICE_ADD(_tag, MD_CART_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)

#define MCFG_PICO_CARTRIDGE_ADD(_tag,_slot_intf,_def_slot) \
	MCFG_DEVICE_ADD(_tag, PICO_CART_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)

#define MCFG_COPERA_CARTRIDGE_ADD(_tag,_slot_intf,_def_slot) \
	MCFG_DEVICE_ADD(_tag, COPERA_CART_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)



#define MCFG_MD_CARTRIDGE_NOT_MANDATORY                                     \
	static_cast<md_cart_slot_device *>(device)->set_must_be_loaded(FALSE);


#endif
