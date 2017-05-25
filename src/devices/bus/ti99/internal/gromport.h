// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/***************************************************************************
    Gromport (Cartridge port) of the TI-99 consoles
    For details see gromport.cpp
    
    Michael Zapf
***************************************************************************/

#ifndef MAME_BUS_TI99_INTERNAL_GROMPORT_H
#define MAME_BUS_TI99_INTERNAL_GROMPORT_H

#pragma once

#include "bus/ti99/ti99defs.h"
#include "machine/tmc0430.h"
#include "softlist_dev.h"

namespace bus { namespace ti99 { namespace internal {

class cartridge_connector_device;

class gromport_device : public bus8z_device, public device_slot_interface
{
public:
	gromport_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	DECLARE_READ8Z_MEMBER(readz) override;
	DECLARE_WRITE8_MEMBER(write) override;
	DECLARE_READ8Z_MEMBER(crureadz);
	DECLARE_WRITE8_MEMBER(cruwrite);
	DECLARE_WRITE_LINE_MEMBER(ready_line);

	DECLARE_WRITE_LINE_MEMBER(romgq_line);

	// Combined GROM select lines
	DECLARE_WRITE8_MEMBER(set_gromlines);

	DECLARE_WRITE_LINE_MEMBER(gclock_in);

	static void set_mask(device_t &device, int mask) { downcast<gromport_device &>(device).m_mask = mask; }

	template <class Object> static devcb_base &static_set_ready_callback(device_t &device, Object &&cb)  { return downcast<gromport_device &>(device).m_console_ready.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &static_set_reset_callback(device_t &device, Object &&cb) { return downcast<gromport_device &>(device).m_console_reset.set_callback(std::forward<Object>(cb)); }

	void    cartridge_inserted();
	bool    is_grom_idle();

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_config_complete() override;
	virtual ioport_constructor device_input_ports() const override;

private:
	cartridge_connector_device*    m_connector;
	bool                m_reset_on_insert;
	devcb_write_line   m_console_ready;
	devcb_write_line   m_console_reset;
	int             m_mask;
	int m_romgq;
};

/****************************************************************************/

class ti99_cartridge_pcb;

class ti99_cartridge_device : public bus8z_device, public device_image_interface
{
public:
	ti99_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ8Z_MEMBER(readz) override;
	DECLARE_WRITE8_MEMBER(write) override;
	DECLARE_READ8Z_MEMBER(crureadz);
	DECLARE_WRITE8_MEMBER(cruwrite);

	DECLARE_WRITE_LINE_MEMBER(ready_line);
	DECLARE_WRITE_LINE_MEMBER(romgq_line);
	DECLARE_WRITE8_MEMBER(set_gromlines);

	DECLARE_WRITE_LINE_MEMBER(gclock_in);

	bool    is_available() { return m_pcb != nullptr; }
	void    set_slot(int i);
	bool    is_grom_idle();

protected:
	virtual void device_start() override { }
	virtual void device_config_complete() override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const tiny_rom_entry* device_rom_region() const override;

	// Image handling: implementation of methods which are abstract in the parent
	image_init_result call_load() override;
	void call_unload() override;
	virtual const software_list_loader &get_software_list_loader() const override { return rom_software_list_loader::instance(); }

	void prepare_cartridge();

	// device_image_interface
	iodevice_t image_type() const override { return IO_CARTSLOT; }
	bool is_readable()  const override           { return true; }
	bool is_writeable() const override           { return false; }
	bool is_creatable() const override           { return false; }
	bool must_be_loaded() const override         { return false; }
	bool is_reset_on_load() const override       { return false; }
	const char *image_interface() const override { return "ti99_cart"; }
	const char *file_extensions() const override { return "rpk"; }

private:
	class rpk_socket;
	class rpk_reader;
	class rpk;

	bool    m_readrom;
	int     m_pcbtype;
	int     m_slot;
	int     get_index_from_tagname();

	std::unique_ptr<ti99_cartridge_pcb> m_pcb;          // inbound
	cartridge_connector_device*    m_connector;    // outbound

	// RPK which is associated to this cartridge
	// When we close it, the contents are saved to NVRAM if available
	rpk *m_rpk;
};

/****************************************************************************/

class cartridge_connector_device : public bus8z_device
{
public:
	virtual DECLARE_READ8Z_MEMBER(crureadz) = 0;
	virtual DECLARE_WRITE8_MEMBER(cruwrite) = 0;

	virtual DECLARE_WRITE_LINE_MEMBER(romgq_line) = 0;
	virtual DECLARE_WRITE8_MEMBER(set_gromlines) = 0;

	virtual DECLARE_WRITE_LINE_MEMBER(gclock_in) = 0;

	DECLARE_WRITE_LINE_MEMBER(ready_line);

	virtual void insert(int index, ti99_cartridge_device* cart) { m_gromport->cartridge_inserted(); }
	virtual void remove(int index) { }
	virtual bool is_grom_idle() = 0;

protected:
	cartridge_connector_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	virtual void device_config_complete() override;

	gromport_device*    m_gromport;
	bool     m_grom_selected;
};

/*
    Single cartridge connector.
*/
class ti99_single_cart_conn_device : public cartridge_connector_device
{
public:
	ti99_single_cart_conn_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ8Z_MEMBER(readz) override;
	DECLARE_WRITE8_MEMBER(write) override;
	DECLARE_READ8Z_MEMBER(crureadz) override;
	DECLARE_WRITE8_MEMBER(cruwrite) override;
	DECLARE_WRITE_LINE_MEMBER(romgq_line) override;
	DECLARE_WRITE8_MEMBER(set_gromlines) override;
	DECLARE_WRITE_LINE_MEMBER(gclock_in) override;

	bool is_grom_idle() override;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

private:
	ti99_cartridge_device *m_cartridge;
};

/*
    Multi cartridge connector.
*/

/* We set the number of slots to 4, although we may have up to 16. From a
   logical point of view we could have 256, but the operating system only checks
   the first 16 banks. */

class ti99_multi_cart_conn_device : public cartridge_connector_device
{
public:
	ti99_multi_cart_conn_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ8Z_MEMBER(readz) override;
	DECLARE_WRITE8_MEMBER(write) override;
	DECLARE_READ8Z_MEMBER(crureadz) override;
	DECLARE_WRITE8_MEMBER(cruwrite) override;
	DECLARE_WRITE_LINE_MEMBER(romgq_line) override;
	DECLARE_WRITE8_MEMBER(set_gromlines) override;
	DECLARE_WRITE_LINE_MEMBER(gclock_in) override;

	void insert(int index, ti99_cartridge_device* cart) override;
	void remove(int index) override;
	DECLARE_INPUT_CHANGED_MEMBER( switch_changed );

	bool is_grom_idle() override;

protected:
	static constexpr unsigned NUMBER_OF_CARTRIDGE_SLOTS = 4;

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

private:
	bool m_readrom;
	int     m_active_slot;
	int     m_fixed_slot;
	int     m_next_free_slot;
	ti99_cartridge_device*  m_cartridge[NUMBER_OF_CARTRIDGE_SLOTS];

	void    set_slot(int slotnumber);
	int     get_active_slot(bool changebase, offs_t offset);
};

/*
    GRAM Kracker.
*/
class ti99_gkracker_device : public cartridge_connector_device, public device_nvram_interface
{
public:
	ti99_gkracker_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ8Z_MEMBER(readz) override;
	DECLARE_WRITE8_MEMBER(write) override;
	DECLARE_READ8Z_MEMBER(crureadz) override;
	DECLARE_WRITE8_MEMBER(cruwrite) override;
	DECLARE_WRITE_LINE_MEMBER(romgq_line) override;
	DECLARE_WRITE8_MEMBER(set_gromlines) override;
	DECLARE_WRITE_LINE_MEMBER(gclock_in) override;

	void insert(int index, ti99_cartridge_device* cart) override;
	void remove(int index) override;
	DECLARE_INPUT_CHANGED_MEMBER( gk_changed );

	// We may have a cartridge plugged into the GK
	bool is_grom_idle() override;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const tiny_rom_entry* device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;

	// device_nvram_interface
	void nvram_default() override;
	void nvram_read(emu_file &file) override;
	void nvram_write(emu_file &file) override;

private:
	int     m_gk_switch[6];         // Used to cache the switch settings.

	bool    m_romspace_selected;
	int     m_ram_page;
	int     m_grom_address;
	uint8_t*  m_ram_ptr;
	uint8_t*  m_grom_ptr;

	bool    m_waddr_LSB;

	ti99_cartridge_device *m_cartridge;     // guest cartridge

	// Just for proper initialization
	void gk_install_menu(const char* menutext, int len, int ptr, int next, int start);
};

/****************************************************************************/

class ti99_cartridge_pcb
{
	friend class ti99_cartridge_device;
public:
	ti99_cartridge_pcb();
	virtual ~ti99_cartridge_pcb() { }

protected:
	virtual DECLARE_READ8Z_MEMBER(readz);
	virtual DECLARE_WRITE8_MEMBER(write);
	virtual DECLARE_READ8Z_MEMBER(crureadz);
	virtual DECLARE_WRITE8_MEMBER(cruwrite);

	DECLARE_WRITE_LINE_MEMBER(romgq_line);
	virtual DECLARE_WRITE8_MEMBER(set_gromlines);
	DECLARE_WRITE_LINE_MEMBER(gclock_in);

	DECLARE_READ8Z_MEMBER(gromreadz);
	DECLARE_WRITE8_MEMBER(gromwrite);
	inline void         set_grom_pointer(int number, device_t *dev);
	void                set_cartridge(ti99_cartridge_device *cart);
	const char*         tag() { return m_tag; }
	void                set_tag(const char* tag) { m_tag = tag; }
	bool                is_grom_idle() { return m_grom_idle; }

	ti99_cartridge_device*  m_cart;
	tmc0430_device*     m_grom[5];
	bool                m_grom_idle;
	int                 m_grom_size;
	int                 m_rom_size;
	int                 m_ram_size;

	uint8_t*              m_rom_ptr;
	uint8_t*              m_ram_ptr;
	bool                m_romspace_selected;
	int                 m_rom_page;     // for some cartridge types
	uint8_t*              m_grom_ptr;     // for gromemu
	int                 m_grom_address; // for gromemu
	int                 m_ram_page;     // for super
	const char*         m_tag;
	std::vector<uint8_t>      m_nvram;    // for MiniMemory
	std::vector<uint8_t>      m_ram;  // for MBX
};

/******************** Standard cartridge ******************************/

class ti99_standard_cartridge : public ti99_cartridge_pcb
{
public:
};

/*********** Paged cartridge (like Extended Basic) ********************/

class ti99_paged12k_cartridge : public ti99_cartridge_pcb
{
public:
	DECLARE_READ8Z_MEMBER(readz) override;
	DECLARE_WRITE8_MEMBER(write) override;
};

/*********** Paged cartridge (others) ********************/

class ti99_paged16k_cartridge : public ti99_cartridge_pcb
{
public:
	DECLARE_READ8Z_MEMBER(readz) override;
	DECLARE_WRITE8_MEMBER(write) override;
};

/********************** Mini Memory ***********************************/

class ti99_minimem_cartridge : public ti99_cartridge_pcb
{
public:
	DECLARE_READ8Z_MEMBER(readz) override;
	DECLARE_WRITE8_MEMBER(write) override;
};

/********************* Super Space II *********************************/

class ti99_super_cartridge : public ti99_cartridge_pcb
{
public:
	DECLARE_READ8Z_MEMBER(readz) override;
	DECLARE_WRITE8_MEMBER(write) override;
	DECLARE_READ8Z_MEMBER(crureadz) override;
	DECLARE_WRITE8_MEMBER(cruwrite) override;
};

/************************* MBX  ***************************************/

class ti99_mbx_cartridge : public ti99_cartridge_pcb
{
public:
	DECLARE_READ8Z_MEMBER(readz) override;
	DECLARE_WRITE8_MEMBER(write) override;
};

/********************** Paged 379i ************************************/

class ti99_paged379i_cartridge : public ti99_cartridge_pcb
{
public:
	DECLARE_READ8Z_MEMBER(readz) override;
	DECLARE_WRITE8_MEMBER(write) override;
private:
	int     get_paged379i_bank(int rompage);
};

/********************** Paged 378 ************************************/

class ti99_paged378_cartridge : public ti99_cartridge_pcb
{
public:
	DECLARE_READ8Z_MEMBER(readz) override;
	DECLARE_WRITE8_MEMBER(write) override;
};

/********************** Paged 377 ************************************/

class ti99_paged377_cartridge : public ti99_cartridge_pcb
{
public:
	DECLARE_READ8Z_MEMBER(readz) override;
	DECLARE_WRITE8_MEMBER(write) override;
};

/********************** Paged CRU  ************************************/

class ti99_pagedcru_cartridge : public ti99_cartridge_pcb
{
public:
	DECLARE_READ8Z_MEMBER(readz) override;
	DECLARE_WRITE8_MEMBER(write) override;
	DECLARE_READ8Z_MEMBER(crureadz) override;
	DECLARE_WRITE8_MEMBER(cruwrite) override;
};

/********************** GROM emulation cartridge  ************************************/

class ti99_gromemu_cartridge : public ti99_cartridge_pcb
{
public:
	ti99_gromemu_cartridge(): m_waddr_LSB(false), m_grom_selected(false), m_grom_read_mode(false), m_grom_address_mode(false)
	{  m_grom_address = 0; }
	DECLARE_READ8Z_MEMBER(readz) override;
	DECLARE_WRITE8_MEMBER(write) override;
	DECLARE_READ8Z_MEMBER(gromemureadz);
	DECLARE_WRITE8_MEMBER(gromemuwrite);
	DECLARE_WRITE8_MEMBER(set_gromlines) override;

private:
	bool    m_waddr_LSB;
	bool    m_grom_selected;
	bool    m_grom_read_mode;
	bool    m_grom_address_mode;
};

} } } // end namespace bus::ti99::internal

SLOT_INTERFACE_EXTERN(gromport4);
SLOT_INTERFACE_EXTERN(gromport8);

#define MCFG_GROMPORT4_ADD( _tag )   \
	MCFG_DEVICE_ADD(_tag, TI99_GROMPORT, 0) \
	bus::ti99::internal::gromport_device::set_mask(*device, 0x1fff); \
	MCFG_DEVICE_SLOT_INTERFACE(gromport4, "single", false)

#define MCFG_GROMPORT8_ADD( _tag )   \
	MCFG_DEVICE_ADD(_tag, TI99_GROMPORT, 0) \
	bus::ti99::internal::gromport_device::set_mask(*device, 0x3fff); \
	MCFG_DEVICE_SLOT_INTERFACE(gromport8, "single", false)

#define MCFG_GROMPORT_READY_HANDLER( _ready ) \
	devcb = &bus::ti99::internal::gromport_device::static_set_ready_callback( *device, DEVCB_##_ready );

#define MCFG_GROMPORT_RESET_HANDLER( _reset ) \
	devcb = &bus::ti99::internal::gromport_device::static_set_reset_callback( *device, DEVCB_##_reset );

DECLARE_DEVICE_TYPE_NS(TI99_GROMPORT, bus::ti99::internal, gromport_device)
DECLARE_DEVICE_TYPE_NS(TI99_CART, bus::ti99::internal, ti99_cartridge_device)
DECLARE_DEVICE_TYPE_NS(TI99_GROMPORT_SINGLE, bus::ti99::internal, ti99_single_cart_conn_device)
DECLARE_DEVICE_TYPE_NS(TI99_GROMPORT_MULTI,  bus::ti99::internal, ti99_multi_cart_conn_device)
DECLARE_DEVICE_TYPE_NS(TI99_GROMPORT_GK,     bus::ti99::internal, ti99_gkracker_device)

#endif // MAME_BUS_TI99_INTERNAL_GROMPORT_H
