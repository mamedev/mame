#ifndef __A78_SLOT_H
#define __A78_SLOT_H


/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/


/* PCB */
enum
{
	A78_TYPE0 = 0,		// standard 8K/16K/32K games, no bankswitch
	A78_TYPE1,			// as TYPE0 + POKEY chip on the PCB
	A78_TYPE2,			// Atari SuperGame pcb (8x16K banks with bankswitch)
	A78_TYPE3,			// as TYPE1 + POKEY chip on the PCB
	A78_TYPE6,			// as TYPE1 + RAM IC on the PCB
	A78_TYPEA,			// Alien Brigade, Crossbow (9x16K banks with diff bankswitch)
	A78_ABSOLUTE,		// F18 Hornet
	A78_ACTIVISION,		// Double Dragon, Rampage
	A78_HSC,			// Atari HighScore cart
	A78_BANKRAM,		// SuperGame + 32K RAM banked (untested)
	A78_XB_BOARD,		// A7800 Expansion Board (it shall more or less apply to the Expansion Module too, but this is not officially released yet)
	A78_XM_BOARD,		// A7800 XM Expansion Module (theoretical specs only, since this is not officially released yet)
	A78_TYPEB			// Cart exploiting the XB board, but possibly also compatible with non-expanded A7800
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

	void rom_alloc(UINT32 size);
	void ram_alloc(UINT32 size);
	void nvram_alloc(UINT32 size);
	UINT8* get_rom_base() { return m_rom; }
	UINT8* get_ram_base() { return m_ram; }
	UINT8* get_nvram_base() { return m_nvram; }
	UINT32 get_rom_size() { return m_rom.bytes(); }
	UINT32 get_ram_size() { return m_ram.bytes(); }
	UINT32 get_nvram_size() { return m_nvram.bytes(); }

protected:
	// internal state
	dynamic_buffer m_rom;
	dynamic_buffer m_ram;
	dynamic_buffer m_nvram;	// HiScore cart can save scores!
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
	virtual void device_start();
	virtual void device_config_complete();

	// image-level overrides
	virtual bool call_load();
	virtual void call_unload();
	virtual bool call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry);

	int get_cart_type() { return m_type; };
	int identify_cart_type(UINT8 *ROM, UINT32 len);
	bool has_cart() { return m_cart != NULL; }
	
	virtual iodevice_t image_type() const { return IO_CARTSLOT; }
	virtual bool is_readable()  const { return 1; }
	virtual bool is_writeable() const { return 0; }
	virtual bool is_creatable() const { return 0; }
	virtual bool must_be_loaded() const { return 0; }
	virtual bool is_reset_on_load() const { return 1; }
	virtual const option_guide *create_option_guide() const { return NULL; }
	virtual const char *image_interface() const { return "a7800_cart"; }
	virtual const char *file_extensions() const { return "bin,a78"; }
	virtual device_image_partialhash_func get_partial_hash() const { return &a78_partialhash; }

	// slot interface overrides
	virtual void get_default_card_software(astring &result);

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
	int m_stick_type;

	int verify_header(char *header);
	void internal_header_logging(UINT8 *header, UINT32 len);
};


// device type definition
extern const device_type A78_CART_SLOT;


/***************************************************************************
 DEVICE CONFIGURATION MACROS
 ***************************************************************************/

#define MCFG_A78_CARTRIDGE_ADD(_tag,_slot_intf,_def_slot) \
	MCFG_DEVICE_ADD(_tag, A78_CART_SLOT, 0)  \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)


#endif
