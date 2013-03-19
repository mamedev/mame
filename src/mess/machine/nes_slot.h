#ifndef __NES_SLOT_H__
#define __NES_SLOT_H__

#include "includes/nes_mmc.h"


/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/

// ======================> nes_cart_interface

struct nes_cart_interface
{
};


// ======================> device_nes_cart_interface

class device_nes_cart_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_nes_cart_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_nes_cart_interface();

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_l) { return 0xff; }
	virtual DECLARE_READ8_MEMBER(read_m) { return 0xff; }
	virtual DECLARE_READ8_MEMBER(read_h) { return 0xff; }
	virtual DECLARE_WRITE8_MEMBER(write_l) { }
	virtual DECLARE_WRITE8_MEMBER(write_m) { }
	virtual DECLARE_WRITE8_MEMBER(write_h) { }

	virtual void prg_alloc(running_machine &machine, size_t size);
	virtual void prgram_alloc(running_machine &machine, size_t size);
	virtual void vrom_alloc(running_machine &machine, size_t size);
	virtual void vram_alloc(running_machine &machine, size_t size);
	virtual void battery_alloc(running_machine &machine, size_t size);
	virtual void mapper_ram_alloc(running_machine &machine, size_t size);
	virtual void mapper_bram_alloc(running_machine &machine, size_t size);

	virtual int get_mirroring() { return m_mirroring; }
	virtual void set_mirroring(int val) { m_mirroring = val; }
	virtual int get_four_screen_vram() { return m_four_screen_vram; }
	virtual void set_four_screen_vram(int val) { m_four_screen_vram = val; }

	virtual UINT8* get_prg_base() { return m_prg; }
	virtual UINT8* get_prgram_base() { return m_prgram; }
	virtual UINT8* get_vrom_base() { return m_vrom; }
	virtual UINT8* get_vram_base() { return m_vram; }
	virtual UINT8* get_battery_base() { return m_battery; }
	virtual UINT8* get_mapper_ram_base() { return m_mapper_ram; }
	virtual UINT8* get_mapper_bram_base() { return m_mapper_bram; }

	virtual UINT32 get_prg_size() { return m_prg_size; }
	virtual UINT32 get_prgram_size() { return m_prgram_size; }
	virtual UINT32 get_vrom_size() { return m_vrom_size; }
	virtual UINT32 get_vram_size() { return m_vram_size; }
	virtual UINT32 get_battery_size() { return m_battery_size; }
	virtual UINT32 get_mapper_ram_size() { return m_mapper_ram_size; }
	virtual UINT32 get_mapper_bram_size() { return m_mapper_bram_size; }

//private:

	// internal state
	UINT8      *m_prg;
	UINT8      *m_prgram;
	UINT8      *m_vrom;
	UINT8      *m_vram;
	UINT8      *m_battery;
	UINT8      *m_mapper_ram;
	UINT8      *m_mapper_bram;

	UINT32 m_prg_size;
	UINT32 m_prgram_size;
	UINT32 m_vrom_size;
	UINT32 m_vram_size;
	UINT32 m_battery_size;
	UINT32 m_mapper_ram_size;
	UINT32 m_mapper_bram_size;

	int m_mirroring, m_four_screen_vram;
	bool m_has_battery, m_has_prgram;
};

extern void nes_partialhash(hash_collection &dest, const unsigned char *data, unsigned long length, const char *functions);


// ======================> nes_cart_slot_device

class nes_cart_slot_device : public device_t,
								public nes_cart_interface,
								public device_image_interface,
								public device_slot_interface
{
public:
	// construction/destruction
	nes_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~nes_cart_slot_device();

	// device-level overrides
	virtual void device_start();
	virtual void device_config_complete();

	// image-level overrides
	virtual bool call_load();
	virtual void call_unload();
	virtual bool call_softlist_load(char *swlist, char *swname, rom_entry *start_entry);

	virtual iodevice_t image_type() const { return IO_CARTSLOT; }
	virtual bool is_readable()  const { return 1; }
	virtual bool is_writeable() const { return 0; }
	virtual bool is_creatable() const { return 0; }
	virtual bool must_be_loaded() const { return m_must_be_loaded; }
	virtual bool is_reset_on_load() const { return 1; }
	virtual const char *image_interface() const { return "nes_cart"; }
	virtual const char *file_extensions() const { return "nes,unf,unif"; }
	virtual const option_guide *create_option_guide() const { return NULL; }
	virtual device_image_partialhash_func get_partial_hash() const { return &nes_partialhash; }

	// slot interface overrides
	virtual const char * get_default_card_software(const machine_config &config, emu_options &options);

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_l);
	virtual DECLARE_READ8_MEMBER(read_m);
	virtual DECLARE_READ8_MEMBER(read_h);
	virtual DECLARE_WRITE8_MEMBER(write_l);
	virtual DECLARE_WRITE8_MEMBER(write_m);
	virtual DECLARE_WRITE8_MEMBER(write_h);

	int get_pcb_id() { return m_pcb_id; };

	// temporarily here
	int m_chr_open_bus;
	int m_ce_mask;
	int m_ce_state;
	int m_vrc_ls_prg_a;
	int m_vrc_ls_prg_b;
	int m_vrc_ls_chr;
	int m_crc_hack;

	int get_chr_open_bus() { return m_chr_open_bus; };
	int get_ce_mask() { return m_ce_mask; };
	int get_ce_state() { return m_ce_state; };
	int get_vrc_ls_prg_a() { return m_vrc_ls_prg_a; };
	int get_vrc_ls_prg_b() { return m_vrc_ls_prg_b; };
	int get_vrc_ls_chr() { return m_vrc_ls_chr; };
	int get_crc_hack() { return m_crc_hack; };

	void set_must_be_loaded(bool _must_be_loaded) { m_must_be_loaded = _must_be_loaded; }

	//private:

	device_nes_cart_interface*      m_cart;
	int m_pcb_id;
	bool                            m_must_be_loaded;
};

// device type definition
extern const device_type NES_CART_SLOT;


/***************************************************************************
 DEVICE CONFIGURATION MACROS
 ***************************************************************************/

#define MCFG_NES_CARTRIDGE_ADD(_tag,_config,_slot_intf,_def_slot,_def_inp) \
	MCFG_DEVICE_ADD(_tag, NES_CART_SLOT, 0) \
	MCFG_DEVICE_CONFIG(_config) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, _def_inp, false)

#define MCFG_NES_CARTRIDGE_NOT_MANDATORY                                     \
	static_cast<nes_cart_slot_device *>(device)->set_must_be_loaded(FALSE);


// CART DEVICE [TO BE MOVED TO SEPARATE SOURCE LATER]

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nes_rom_device

class nes_rom_device : public device_t,
				public device_nes_cart_interface
{
public:
	// construction/destruction
	nes_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	nes_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);

	//protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_config_complete() { m_shortname = "nes_rom"; }

	// nescart_interface overrides
//  virtual DECLARE_READ8_MEMBER(read_l);
//  virtual DECLARE_READ8_MEMBER(read_m);
//  virtual DECLARE_READ8_MEMBER(read_h);
//  virtual DECLARE_WRITE8_MEMBER(write_l);
//  virtual DECLARE_WRITE8_MEMBER(write_m);
//  virtual DECLARE_WRITE8_MEMBER(write_h);
};

// device type definition
extern const device_type NES_ROM;

#endif
