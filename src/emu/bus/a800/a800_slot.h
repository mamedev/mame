// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __A800_SLOT_H
#define __A800_SLOT_H


/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/


/* PCB */
enum
{
	A800_8K = 0,
	A800_8K_RIGHT,
	A800_16K,
	A800_OSS034M,
	A800_OSS043M,
	A800_OSSM091,
	A800_OSS8K,
	A800_PHOENIX,
	A800_XEGS,
	A800_BBSB,
	A800_DIAMOND,
	A800_WILLIAMS,
	A800_EXPRESS,
	A800_SPARTADOS,
	A800_BLIZZARD,
	A800_TURBO64,
	A800_TURBO128,
	A800_TELELINK2,
	A800_MICROCALC,
	A800_CORINA,
	A5200_4K,
	A5200_8K,
	A5200_16K,
	A5200_32K,
	A5200_16K_2CHIPS,
	A5200_BBSB
};


// ======================> device_a800_cart_interface

class device_a800_cart_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_a800_cart_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_a800_cart_interface();

	// memory accessor
	virtual DECLARE_READ8_MEMBER(read_80xx) { return 0xff; }
	virtual DECLARE_READ8_MEMBER(read_d5xx) { return 0xff; }
	virtual DECLARE_WRITE8_MEMBER(write_80xx) {}
	virtual DECLARE_WRITE8_MEMBER(write_d5xx) {}

	void rom_alloc(UINT32 size, const char *tag);
	void ram_alloc(UINT32 size);
	void nvram_alloc(UINT32 size);
	UINT8* get_rom_base() { return m_rom; }
	UINT8* get_ram_base() { return &m_ram[0]; }
	UINT8* get_nvram_base() { return &m_nvram[0]; }
	UINT32 get_rom_size() { return m_rom_size; }
	UINT32 get_ram_size() { return m_ram.size(); }
	UINT32 get_nvram_size() { return m_nvram.size(); }

protected:
	// internal state
	UINT8 *m_rom;
	UINT32 m_rom_size;
	dynamic_buffer m_ram;
	dynamic_buffer m_nvram; // HiScore cart can save scores!
	// helpers
	int m_bank_mask;
};


// ======================> a800_cart_slot_device

class a800_cart_slot_device : public device_t,
								public device_image_interface,
								public device_slot_interface
{
public:
	// construction/destruction
	a800_cart_slot_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	a800_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~a800_cart_slot_device();

	// device-level overrides
	virtual void device_start();
	virtual void device_config_complete();

	// image-level overrides
	virtual bool call_load();
	virtual void call_unload();
	virtual bool call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry);

	int get_cart_type() { return m_type; };
	int identify_cart_type(UINT8 *header);
	bool has_cart() { return m_cart != NULL; }

	virtual iodevice_t image_type() const { return IO_CARTSLOT; }
	virtual bool is_readable()  const { return 1; }
	virtual bool is_writeable() const { return 0; }
	virtual bool is_creatable() const { return 0; }
	virtual bool must_be_loaded() const { return 0; }
	virtual bool is_reset_on_load() const { return 1; }
	virtual const option_guide *create_option_guide() const { return NULL; }
	virtual const char *image_interface() const { return "a8bit_cart"; }
	virtual const char *file_extensions() const { return "bin,rom,car"; }

	// slot interface overrides
	virtual void get_default_card_software(std::string &result);

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_80xx);
	virtual DECLARE_READ8_MEMBER(read_d5xx);
	virtual DECLARE_WRITE8_MEMBER(write_80xx);
	virtual DECLARE_WRITE8_MEMBER(write_d5xx);

private:
	device_a800_cart_interface*       m_cart;
	int m_type;
};


// The variants below are added to handle the additional formats for a5200, and to give more
// clear error messages if you try to load an A5200 game into an A800 or a XEGS, etc.

// ======================> a5200_cart_slot_device

class a5200_cart_slot_device : public a800_cart_slot_device
{
public:
	// construction/destruction
	a5200_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~a5200_cart_slot_device();

	virtual const char *file_extensions() const { return "bin,rom,car,a52"; }

	// slot interface overrides
	virtual void get_default_card_software(std::string &result);
};

// ======================> xegs_cart_slot_device

class xegs_cart_slot_device : public a800_cart_slot_device
{
public:
	// construction/destruction
	xegs_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~xegs_cart_slot_device();

	virtual const char *file_extensions() const { return "bin,rom,car"; }

	// slot interface overrides
	virtual void get_default_card_software(std::string &result);
};

// device type definition
extern const device_type A800_CART_SLOT;
extern const device_type A5200_CART_SLOT;
extern const device_type XEGS_CART_SLOT;


/***************************************************************************
 DEVICE CONFIGURATION MACROS
 ***************************************************************************/

#define A800SLOT_ROM_REGION_TAG ":cart:rom"

#define MCFG_A800_CARTRIDGE_ADD(_tag,_slot_intf,_def_slot) \
	MCFG_DEVICE_ADD(_tag, A800_CART_SLOT, 0)  \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)

#define MCFG_A5200_CARTRIDGE_ADD(_tag,_slot_intf,_def_slot) \
	MCFG_DEVICE_ADD(_tag, A5200_CART_SLOT, 0)  \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)

#define MCFG_XEGS_CARTRIDGE_ADD(_tag,_slot_intf,_def_slot) \
	MCFG_DEVICE_ADD(_tag, XEGS_CART_SLOT, 0)  \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)


#endif
