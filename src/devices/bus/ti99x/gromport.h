// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/***************************************************************************
    Gromport of the TI-99 consoles

    For details see gromport.c

    Michael Zapf, July 2012
***************************************************************************/

#ifndef __GROMPORT__
#define __GROMPORT__

#include "emu.h"
#include "ti99defs.h"
#include "grom.h"

extern const device_type GROMPORT;

class ti99_cartridge_connector_device;

class gromport_device : public bus8z_device, public device_slot_interface
{
public:
	gromport_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	DECLARE_READ8Z_MEMBER(readz);
	DECLARE_WRITE8_MEMBER(write);
	DECLARE_READ8Z_MEMBER(crureadz);
	DECLARE_WRITE8_MEMBER(cruwrite);
	DECLARE_WRITE_LINE_MEMBER(ready_line);

	template<class _Object> static devcb_base &static_set_ready_callback(device_t &device, _Object object)  { return downcast<gromport_device &>(device).m_console_ready.set_callback(object); }
	template<class _Object> static devcb_base &static_set_reset_callback(device_t &device, _Object object) { return downcast<gromport_device &>(device).m_console_reset.set_callback(object); }

	void    cartridge_inserted();
	void    set_grom_base(UINT16 grombase, UINT16 grommask);
	UINT16  get_grom_base() { return m_grombase; }
	UINT16  get_grom_mask() { return m_grommask; }

protected:
	virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete();
	virtual ioport_constructor device_input_ports() const;

private:
	ti99_cartridge_connector_device*    m_connector;
	bool                m_reset_on_insert;
	devcb_write_line   m_console_ready;
	devcb_write_line   m_console_reset;
	UINT16              m_grombase;
	UINT16              m_grommask;
};

SLOT_INTERFACE_EXTERN(gromport);

#define MCFG_TI99_GROMPORT_ADD( _tag )   \
	MCFG_DEVICE_ADD(_tag, GROMPORT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(gromport, "single", false)

#define MCFG_GROMPORT_READY_HANDLER( _ready ) \
	devcb = &gromport_device::static_set_ready_callback( *device, DEVCB_##_ready );

#define MCFG_GROMPORT_RESET_HANDLER( _reset ) \
	devcb = &gromport_device::static_set_reset_callback( *device, DEVCB_##_reset );

/****************************************************************************/

class rpk;
class ti99_cartridge_pcb;

class ti99_cartridge_device : public bus8z_device, public device_image_interface
{
public:
	ti99_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8Z_MEMBER(readz);
	DECLARE_WRITE8_MEMBER(write);
	DECLARE_READ8Z_MEMBER(crureadz);
	DECLARE_WRITE8_MEMBER(cruwrite);

	DECLARE_WRITE_LINE_MEMBER(ready_line);
	bool    is_available() { return m_pcb != nullptr; }
	bool    has_grom();
	void    set_slot(int i);
	UINT16  grom_base();
	UINT16  grom_mask();

protected:
	virtual void device_start() { };
	virtual void device_config_complete();
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual const rom_entry* device_rom_region() const;

	// Image handling: implementation of methods which are abstract in the parent
	bool call_load();
	void call_unload();
	bool call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry);

	void prepare_cartridge();

	// device_image_interface
	iodevice_t image_type() const { return IO_CARTSLOT; }
	bool is_readable()  const           { return true; }
	bool is_writeable() const           { return false; }
	bool is_creatable() const           { return false; }
	bool must_be_loaded() const         { return false; }
	bool is_reset_on_load() const       { return false; }
	const char *image_interface() const { return "ti99_cart"; }
	const char *file_extensions() const { return "rpk"; }
	const option_guide *create_option_guide() const { return nullptr; }

private:
	bool    m_softlist;
	int     m_pcbtype;
	int     m_slot;
	int     get_index_from_tagname();

	ti99_cartridge_pcb*                 m_pcb;          // inbound
	ti99_cartridge_connector_device*    m_connector;    // outbound

	// RPK which is associated to this cartridge
	// When we close it, the contents are saved to NVRAM if available
	rpk *m_rpk;
};

extern const device_type TI99CART;

/****************************************************************************/

class ti99_cartridge_connector_device : public bus8z_device
{
public:
	virtual DECLARE_READ8Z_MEMBER(readz) =0;
	virtual DECLARE_WRITE8_MEMBER(write) =0;
	virtual DECLARE_READ8Z_MEMBER(crureadz) = 0;
	virtual DECLARE_WRITE8_MEMBER(cruwrite) = 0;

	DECLARE_WRITE_LINE_MEMBER(ready_line);

	virtual void insert(int index, ti99_cartridge_device* cart) { m_gromport->cartridge_inserted(); };
	virtual void remove(int index) { };
	UINT16 grom_base();
	UINT16 grom_mask();

protected:
	ti99_cartridge_connector_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	virtual void device_config_complete();

	gromport_device*    m_gromport;
};

/*
    Single cartridge connector.
*/
class single_conn_device : public ti99_cartridge_connector_device
{
public:
	single_conn_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8Z_MEMBER(readz);
	DECLARE_WRITE8_MEMBER(write);
	DECLARE_READ8Z_MEMBER(crureadz);
	DECLARE_WRITE8_MEMBER(cruwrite);

protected:
	virtual void device_start();
	virtual void device_reset();
	virtual machine_config_constructor device_mconfig_additions() const;

private:
	ti99_cartridge_device *m_cartridge;
};

/*
    Multi cartridge connector.
*/

/* We set the number of slots to 4, although we may have up to 16. From a
   logical point of view we could have 256, but the operating system only checks
   the first 16 banks. */
#define NUMBER_OF_CARTRIDGE_SLOTS 4

class multi_conn_device : public ti99_cartridge_connector_device
{
public:
	multi_conn_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8Z_MEMBER(readz);
	DECLARE_WRITE8_MEMBER(write);
	DECLARE_READ8Z_MEMBER(crureadz);
	DECLARE_WRITE8_MEMBER(cruwrite);

	void insert(int index, ti99_cartridge_device* cart);
	void remove(int index);

protected:
	virtual void device_start();
	virtual void device_reset();
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;

private:
	int     m_active_slot;
	int     m_fixed_slot;
	int     m_next_free_slot;
	ti99_cartridge_device*  m_cartridge[NUMBER_OF_CARTRIDGE_SLOTS];

	void    set_slot(int slotnumber);
	int     get_active_slot(bool changebase, offs_t offset);
	void    change_slot(bool inserted, int index);
};

/*
    GRAM Kracker.
*/
class gkracker_device : public ti99_cartridge_connector_device, public device_nvram_interface
{
public:
	gkracker_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8Z_MEMBER(readz);
	DECLARE_WRITE8_MEMBER(write);
	DECLARE_READ8Z_MEMBER(crureadz);
	DECLARE_WRITE8_MEMBER(cruwrite);

	void insert(int index, ti99_cartridge_device* cart);
	void remove(int index);
	DECLARE_INPUT_CHANGED_MEMBER( gk_changed );

protected:
	virtual void device_start();
	virtual void device_reset();

	virtual machine_config_constructor device_mconfig_additions() const;
	virtual const rom_entry* device_rom_region() const;
	virtual ioport_constructor device_input_ports() const;

	// device_nvram_interface
	void nvram_default();
	void nvram_read(emu_file &file);
	void nvram_write(emu_file &file);

private:
	int     m_gk_switch[6];         // Used to cache the switch settings.

	int     m_ram_page;
	int     m_grom_address;
	UINT8*  m_ram_ptr;
	UINT8*  m_grom_ptr;

	bool    m_waddr_LSB;

	ti99_cartridge_device *m_cartridge;     // guest cartridge

	// Just for proper initialization
	void gk_install_menu(const char* menutext, int len, int ptr, int next, int start);
};

extern const device_type GROMPORT_SINGLE;
extern const device_type GROMPORT_MULTI;
extern const device_type GROMPORT_GK;

/****************************************************************************/

class ti99_cartridge_pcb
{
	friend class ti99_cartridge_device;
public:
	ti99_cartridge_pcb();
	virtual ~ti99_cartridge_pcb() { };

protected:
	virtual DECLARE_READ8Z_MEMBER(readz);
	virtual DECLARE_WRITE8_MEMBER(write);
	virtual DECLARE_READ8Z_MEMBER(crureadz);
	virtual DECLARE_WRITE8_MEMBER(cruwrite);

	DECLARE_READ8Z_MEMBER(gromreadz);
	DECLARE_WRITE8_MEMBER(gromwrite);
	inline void         set_grom_pointer(int number, device_t *dev);
	void                set_cartridge(ti99_cartridge_device *cart);
	UINT16              grom_base();
	UINT16              grom_mask();
	const char*         tag() { return m_tag; }
	void                set_tag(const char* tag) { m_tag = tag; }

	ti99_cartridge_device*  m_cart;
	ti99_grom_device*   m_grom[5];
	int                 m_grom_size;
	int                 m_rom_size;
	int                 m_ram_size;

	UINT8*              m_rom_ptr;
	UINT8*              m_ram_ptr;
	int                 m_rom_page;     // for some cartridge types
	UINT8*              m_grom_ptr;     // for gromemu
	int                 m_grom_address; // for gromemu
	int                 m_ram_page;     // for super
	const char*         m_tag;
};

/******************** Standard cartridge ******************************/

class ti99_standard_cartridge : public ti99_cartridge_pcb
{
public:
	~ti99_standard_cartridge() { };
};

/*********** Paged cartridge (like Extended Basic) ********************/

class ti99_paged_cartridge : public ti99_cartridge_pcb
{
public:
	~ti99_paged_cartridge() { };
	DECLARE_READ8Z_MEMBER(readz);
	DECLARE_WRITE8_MEMBER(write);
};

/********************** Mini Memory ***********************************/

class ti99_minimem_cartridge : public ti99_cartridge_pcb
{
public:
	~ti99_minimem_cartridge() { };
	DECLARE_READ8Z_MEMBER(readz);
	DECLARE_WRITE8_MEMBER(write);
};

/********************* Super Space II *********************************/

class ti99_super_cartridge : public ti99_cartridge_pcb
{
public:
	~ti99_super_cartridge() { };
	DECLARE_READ8Z_MEMBER(readz);
	DECLARE_WRITE8_MEMBER(write);
	DECLARE_READ8Z_MEMBER(crureadz);
	DECLARE_WRITE8_MEMBER(cruwrite);
};

/************************* MBX  ***************************************/

class ti99_mbx_cartridge : public ti99_cartridge_pcb
{
public:
	~ti99_mbx_cartridge() { };
	DECLARE_READ8Z_MEMBER(readz);
	DECLARE_WRITE8_MEMBER(write);
};

/********************** Paged 379i ************************************/

class ti99_paged379i_cartridge : public ti99_cartridge_pcb
{
public:
	~ti99_paged379i_cartridge() { };
	DECLARE_READ8Z_MEMBER(readz);
	DECLARE_WRITE8_MEMBER(write);
private:
	int     get_paged379i_bank(int rompage);
};

/********************** Paged 378 ************************************/

class ti99_paged378_cartridge : public ti99_cartridge_pcb
{
public:
	~ti99_paged378_cartridge() { };
	DECLARE_READ8Z_MEMBER(readz);
	DECLARE_WRITE8_MEMBER(write);
private:
	int     get_paged378_bank(int rompage);
};

/********************** Paged 377 ************************************/

class ti99_paged377_cartridge : public ti99_cartridge_pcb
{
public:
	~ti99_paged377_cartridge() { };
	DECLARE_READ8Z_MEMBER(readz);
	DECLARE_WRITE8_MEMBER(write);
private:
	int     get_paged377_bank(int rompage);
};

/********************** Paged CRU  ************************************/

class ti99_pagedcru_cartridge : public ti99_cartridge_pcb
{
public:
	~ti99_pagedcru_cartridge() { };
	DECLARE_READ8Z_MEMBER(readz);
	DECLARE_WRITE8_MEMBER(write);
	DECLARE_READ8Z_MEMBER(crureadz);
	DECLARE_WRITE8_MEMBER(cruwrite);
};

/********************** GROM emulation cartridge  ************************************/

class ti99_gromemu_cartridge : public ti99_cartridge_pcb
{
public:
	ti99_gromemu_cartridge(): m_waddr_LSB(false)
	{  m_grom_address = 0; }
	~ti99_gromemu_cartridge() { };
	DECLARE_READ8Z_MEMBER(readz);
	DECLARE_WRITE8_MEMBER(write);
	DECLARE_READ8Z_MEMBER(gromemureadz);
	DECLARE_WRITE8_MEMBER(gromemuwrite);
private:
	bool    m_waddr_LSB;
};


struct pcb_type
{
	int id;
	const char* name;
};

/*************************************************************************
    RPK support
*************************************************************************/
class rpk;

class rpk_socket
{
	friend class simple_list<rpk_socket>;
	friend class rpk;

public:
	rpk_socket(const char *id, int length, UINT8 *contents);
	rpk_socket(const char *id, int length, UINT8 *contents, const char *pathname);

	const char*     id() { return m_id; }
	int             get_content_length() { return m_length; }
	UINT8*          get_contents() { return m_contents; }
	bool            persistent_ram() { return m_pathname != nullptr; }
	const char*     get_pathname() { return m_pathname; }
	void            cleanup() { if (m_contents != nullptr) global_free_array(m_contents); }

private:
	const char*     m_id;
	UINT32          m_length;
	rpk_socket*     m_next;
	UINT8*          m_contents;
	const char*     m_pathname;
};

class rpk_reader
{
public:
	rpk_reader(const pcb_type *types)
	: m_types(types) { };

	rpk *open(emu_options &options, const char *filename, const char *system_name);

private:
	const zip_file_header*  find_file(zip_file *zip, const char *filename, UINT32 crc);
	rpk_socket*             load_rom_resource(zip_file* zip, xml_data_node* rom_resource_node, const char* socketname);
	rpk_socket*             load_ram_resource(emu_options &options, xml_data_node* ram_resource_node, const char* socketname, const char* system_name);
	const pcb_type*         m_types;
};

class rpk
{
	friend class rpk_reader;
public:
	rpk(emu_options& options, const char* sysname);
	~rpk();

	int         get_type(void) { return m_type; }
	UINT8*      get_contents_of_socket(const char *socket_name);
	int         get_resource_length(const char *socket_name);
	void        close();

private:
	emu_options&            m_options;      // need this to find the path to the nvram files
	int                     m_type;
	//const char*             m_system_name;  // need this to find the path to the nvram files
	tagged_list<rpk_socket> m_sockets;

	void add_socket(const char* id, rpk_socket *newsock);
};

enum rpk_open_error
{
	RPK_OK,
	RPK_NOT_ZIP_FORMAT,
	RPK_CORRUPT,
	RPK_OUT_OF_MEMORY,
	RPK_XML_ERROR,
	RPK_INVALID_FILE_REF,
	RPK_ZIP_ERROR,
	RPK_ZIP_UNSUPPORTED,
	RPK_MISSING_RAM_LENGTH,
	RPK_INVALID_RAM_SPEC,
	RPK_UNKNOWN_RESOURCE_TYPE,
	RPK_INVALID_RESOURCE_REF,
	RPK_INVALID_LAYOUT,
	RPK_MISSING_LAYOUT,
	RPK_NO_PCB_OR_RESOURCES,
	RPK_UNKNOWN_PCB_TYPE
};

static const char error_text[16][30] =
{
	"No error",
	"Not a RPK (zip) file",
	"Module definition corrupt",
	"Out of memory",
	"XML format error",
	"Invalid file reference",
	"Zip file error",
	"Unsupported zip version",
	"Missing RAM length",
	"Invalid RAM specification",
	"Unknown resource type",
	"Invalid resource reference",
	"layout.xml not valid",
	"Missing layout",
	"No pcb or resource found",
	"Unknown pcb type"
};

class rpk_exception
{
public:
	rpk_exception(rpk_open_error value): m_err(value), m_detail(nullptr) { };
	rpk_exception(rpk_open_error value, const char* detail): m_err(value), m_detail(detail) { };

	const char* to_string()
	{
		if (m_detail==nullptr) return error_text[(int)m_err];
		std::string errormsg = std::string(error_text[(int)m_err]).append(": ").append(m_detail);
		return core_strdup(errormsg.c_str());
	}

private:
	rpk_open_error m_err;
	const char* m_detail;
};

#endif
