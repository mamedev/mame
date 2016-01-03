// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __A78_SLOT_H
#define __A78_SLOT_H


/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/


/* PCB */
enum
{
	A78_TYPE0 = 0,      // standard 8K/16K/32K games, no bankswitch
	A78_TYPE1,          // as TYPE0 + POKEY chip on the PCB
	A78_TYPE2,          // Atari SuperGame pcb (8x16K banks with bankswitch)
	A78_TYPE3,          // as TYPE1 + POKEY chip on the PCB
	A78_TYPE6,          // as TYPE1 + RAM IC on the PCB
	A78_TYPEA,          // Alien Brigade, Crossbow (9x16K banks with diff bankswitch)
	A78_ABSOLUTE,       // F18 Hornet
	A78_ACTIVISION,     // Double Dragon, Rampage
	A78_HSC,            // Atari HighScore cart
	A78_XB_BOARD,       // A7800 Expansion Board (it shall more or less apply to the Expansion Module too, but this is not officially released yet)
	A78_XM_BOARD,       // A7800 XM Expansion Module (theoretical specs only, since this is not officially released yet)
	A78_MEGACART,               // Homebrew by CPUWIZ, consists of SuperGame bank up to 512K + 32K RAM banked
	A78_VERSABOARD = 0x10,      // Homebrew by CPUWIZ, consists of SuperGame bank up to 256K + 32K RAM banked
	// VersaBoard variants configured as Type 1/3/A or VersaBoard + POKEY at $0450
	A78_TYPE0_POK450 = 0x20,
	A78_TYPE1_POK450 = 0x21,
	A78_TYPE6_POK450 = 0x24,
	A78_TYPEA_POK450 = 0x25,
	A78_VERSA_POK450 = 0x30
};


// ======================> device_a78_cart_interface

class device_a78_cart_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_a78_cart_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_a78_cart_interface();

	// memory accessor
	virtual DECLARE_READ8_MEMBER(read_04xx) { return 0xff; }
	virtual DECLARE_READ8_MEMBER(read_10xx) { return 0xff; }
	virtual DECLARE_READ8_MEMBER(read_30xx) { return 0xff; }
	virtual DECLARE_READ8_MEMBER(read_40xx) { return 0xff; }
	virtual DECLARE_WRITE8_MEMBER(write_04xx) {}
	virtual DECLARE_WRITE8_MEMBER(write_10xx) {}
	virtual DECLARE_WRITE8_MEMBER(write_30xx) {}
	virtual DECLARE_WRITE8_MEMBER(write_40xx) {}

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
	UINT32 m_base_rom;
	int m_bank_mask;
};


void a78_partialhash(hash_collection &dest, const unsigned char *data, unsigned long length, const char *functions);


// ======================> a78_cart_slot_device

class a78_cart_slot_device : public device_t,
								public device_image_interface,
								public device_slot_interface
{
public:
	// construction/destruction
	a78_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~a78_cart_slot_device();

	// device-level overrides
	virtual void device_start() override;
	virtual void device_config_complete() override;

	// image-level overrides
	virtual bool call_load() override;
	virtual void call_unload() override;
	virtual bool call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry) override;

	int get_cart_type() { return m_type; };
	bool has_cart() { return m_cart != nullptr; }

	virtual iodevice_t image_type() const override { return IO_CARTSLOT; }
	virtual bool is_readable()  const override { return 1; }
	virtual bool is_writeable() const override { return 0; }
	virtual bool is_creatable() const override { return 0; }
	virtual bool must_be_loaded() const override { return 0; }
	virtual bool is_reset_on_load() const override { return 1; }
	virtual const option_guide *create_option_guide() const override { return nullptr; }
	virtual const char *image_interface() const override { return "a7800_cart"; }
	virtual const char *file_extensions() const override { return "bin,a78"; }
	virtual device_image_partialhash_func get_partial_hash() const override { return &a78_partialhash; }

	// slot interface overrides
	virtual void get_default_card_software(std::string &result) override;

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_04xx);
	virtual DECLARE_READ8_MEMBER(read_10xx);
	virtual DECLARE_READ8_MEMBER(read_30xx);
	virtual DECLARE_READ8_MEMBER(read_40xx);
	virtual DECLARE_WRITE8_MEMBER(write_04xx);
	virtual DECLARE_WRITE8_MEMBER(write_10xx);
	virtual DECLARE_WRITE8_MEMBER(write_30xx);
	virtual DECLARE_WRITE8_MEMBER(write_40xx);

private:
	device_a78_cart_interface*       m_cart;
	int m_type;

	int verify_header(char *header);
	int validate_header(int head, bool log);
	void internal_header_logging(UINT8 *header, UINT32 len);
};


// device type definition
extern const device_type A78_CART_SLOT;


/***************************************************************************
 DEVICE CONFIGURATION MACROS
 ***************************************************************************/

#define A78SLOT_ROM_REGION_TAG ":cart:rom"

#define MCFG_A78_CARTRIDGE_ADD(_tag,_slot_intf,_def_slot) \
	MCFG_DEVICE_ADD(_tag, A78_CART_SLOT, 0)  \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)


#endif
