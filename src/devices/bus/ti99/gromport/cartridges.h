// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/***************************************************************************
    Cartridge emulations
****************************************************************************/
#ifndef MAME_BUS_TI99_GROMPORT_CARTRIDGES_H
#define MAME_BUS_TI99_GROMPORT_CARTRIDGES_H

#pragma once

#include "emuopts.h"
#include "bus/ti99/ti99defs.h"
#include "machine/tmc0430.h"
#include "softlist_dev.h"
#include "gromport.h"
#include "unzip.h"
#include "xmlfile.h"

namespace bus { namespace ti99 { namespace gromport {

class ti99_cartridge_pcb;

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

const char *const error_text[16] =
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

class ti99_cartridge_device : public device_t, public device_image_interface
{
public:
	ti99_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ8Z_MEMBER(readz);
	void write(offs_t offset, uint8_t data);
	DECLARE_READ8Z_MEMBER(crureadz);
	void cruwrite(offs_t offset, uint8_t data);

	DECLARE_WRITE_LINE_MEMBER(ready_line);
	DECLARE_WRITE_LINE_MEMBER(romgq_line);

	void set_gromlines(line_state mline, line_state moline, line_state gsq);

	DECLARE_WRITE_LINE_MEMBER(gclock_in);

	bool    is_available() { return m_pcb != nullptr; }
	void    set_slot(int i);
	bool    is_grom_idle();

protected:
	virtual void device_start() override { }
	virtual void device_config_complete() override;
	virtual void device_add_mconfig(machine_config &config) override;
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

	/***************** RPK support ********************
	  Actually deprecated, and to be removed as soon as
	  softlists allow for homebrew cartridges
	***************************************************/

	class rpk_socket;
	class rpk;

	class rpk_exception
	{
	public:
		rpk_exception(rpk_open_error value): m_err(value), m_detail(nullptr) { }
		rpk_exception(rpk_open_error value, const char* detail) : m_err(value), m_detail(detail) { }

		std::string to_string()
		{
			std::string errmsg(error_text[(int)m_err]);
			if (m_detail==nullptr)
				return errmsg;
			return errmsg.append(": ").append(m_detail);
		}

	private:
		rpk_open_error m_err;
		const char* m_detail;
	};

	class rpk_reader
	{
	public:
		rpk_reader(const pcb_type *types) : m_types(types) { }

		rpk *open(emu_options &options, const char *filename, const char *system_name);

	private:
		int find_file(util::archive_file &zip, const char *filename, uint32_t crc);
		std::unique_ptr<rpk_socket> load_rom_resource(util::archive_file &zip, util::xml::data_node const* rom_resource_node, const char* socketname);
		std::unique_ptr<rpk_socket> load_ram_resource(emu_options &options, util::xml::data_node const* ram_resource_node, const char* socketname, const char* system_name);
		const pcb_type* m_types;
	};

	class rpk
	{
		friend class rpk_reader;
	public:
		rpk(emu_options& options, const char* sysname);
		~rpk();

		int         get_type(void) { return m_type; }
		uint8_t*      get_contents_of_socket(const char *socket_name);
		int         get_resource_length(const char *socket_name);
		void        close();

	private:
		emu_options&            m_options;      // need this to find the path to the nvram files
		int                     m_type;
		//const char*             m_system_name;  // need this to find the path to the nvram files
		std::unordered_map<std::string,std::unique_ptr<rpk_socket>> m_sockets;

		void add_socket(const char* id, std::unique_ptr<rpk_socket> newsock);
	};

	class rpk_socket
	{
	public:
		rpk_socket(const char *id, int length, uint8_t *contents);
		rpk_socket(const char *id, int length, uint8_t *contents, std::string &&pathname);
		~rpk_socket() {}

		const char*     id() { return m_id; }
		int             get_content_length() { return m_length; }
		uint8_t*          get_contents() { return m_contents; }
		bool            persistent_ram() { return !m_pathname.empty(); }
		const char*     get_pathname() { return m_pathname.c_str(); }
		void            cleanup() { if (m_contents != nullptr) global_free_array(m_contents); }

	private:
		const char*     m_id;
		uint32_t          m_length;
		uint8_t*          m_contents;
		const std::string m_pathname;
	};

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

class ti99_cartridge_pcb
{
	friend class ti99_cartridge_device;
public:
	ti99_cartridge_pcb();
	virtual ~ti99_cartridge_pcb() { }

protected:
	virtual DECLARE_READ8Z_MEMBER(readz);
	virtual void write(offs_t offset, uint8_t data);
	virtual DECLARE_READ8Z_MEMBER(crureadz);
	virtual void cruwrite(offs_t offset, uint8_t data);

	DECLARE_WRITE_LINE_MEMBER(romgq_line);
	virtual void set_gromlines(line_state mline, line_state moline, line_state gsq);
	DECLARE_WRITE_LINE_MEMBER(gclock_in);

	void gromreadz(uint8_t* value);
	void gromwrite(uint8_t data);

	inline void         set_grom_pointer(int number, device_t *dev);
	void                set_cartridge(ti99_cartridge_device *cart);
	const char*         tag() { return m_tag; }
	void                set_tag(const char* tag) { m_tag = tag; }
	bool                is_grom_idle() { return m_grom_idle; }
	template <typename Format, typename... Params> void logerror(Format &&fmt, Params &&... args) const { m_cart->logerror(fmt, args...); }

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
	void write(offs_t offset, uint8_t data) override;
};

/*********** Paged cartridge (others) ********************/

class ti99_paged16k_cartridge : public ti99_cartridge_pcb
{
public:
	DECLARE_READ8Z_MEMBER(readz) override;
	void write(offs_t offset, uint8_t data) override;
};

/*********** Paged7 cartridge (late carts) ********************/

class ti99_paged7_cartridge : public ti99_cartridge_pcb
{
public:
	DECLARE_READ8Z_MEMBER(readz) override;
	void write(offs_t offset, uint8_t data) override;
};

/********************** Mini Memory ***********************************/

class ti99_minimem_cartridge : public ti99_cartridge_pcb
{
public:
	DECLARE_READ8Z_MEMBER(readz) override;
	void write(offs_t offset, uint8_t data) override;
};

/********************* Super Space II *********************************/

class ti99_super_cartridge : public ti99_cartridge_pcb
{
public:
	DECLARE_READ8Z_MEMBER(readz) override;
	void write(offs_t offset, uint8_t data) override;
	DECLARE_READ8Z_MEMBER(crureadz) override;
	void cruwrite(offs_t offset, uint8_t data) override;
};

/************************* MBX  ***************************************/

class ti99_mbx_cartridge : public ti99_cartridge_pcb
{
public:
	DECLARE_READ8Z_MEMBER(readz) override;
	void write(offs_t offset, uint8_t data) override;
};

/********************** Paged 379i ************************************/

class ti99_paged379i_cartridge : public ti99_cartridge_pcb
{
public:
	DECLARE_READ8Z_MEMBER(readz) override;
	void write(offs_t offset, uint8_t data) override;
private:
	int     get_paged379i_bank(int rompage);
};

/********************** Paged 378 ************************************/

class ti99_paged378_cartridge : public ti99_cartridge_pcb
{
public:
	DECLARE_READ8Z_MEMBER(readz) override;
	void write(offs_t offset, uint8_t data) override;
};

/********************** Paged 377 ************************************/

class ti99_paged377_cartridge : public ti99_cartridge_pcb
{
public:
	DECLARE_READ8Z_MEMBER(readz) override;
	void write(offs_t offset, uint8_t data) override;
};

/********************** Paged CRU  ************************************/

class ti99_pagedcru_cartridge : public ti99_cartridge_pcb
{
public:
	DECLARE_READ8Z_MEMBER(readz) override;
	void write(offs_t offset, uint8_t data) override;
	DECLARE_READ8Z_MEMBER(crureadz) override;
	void cruwrite(offs_t offset, uint8_t data) override;
};

/********************** GROM emulation cartridge  ************************************/

class ti99_gromemu_cartridge : public ti99_cartridge_pcb
{
public:
	ti99_gromemu_cartridge(): m_waddr_LSB(false), m_grom_selected(false), m_grom_read_mode(false), m_grom_address_mode(false)
	{  m_grom_address = 0; }
	DECLARE_READ8Z_MEMBER(readz) override;
	void write(offs_t offset, uint8_t data) override;
	DECLARE_READ8Z_MEMBER(gromemureadz);
	void gromemuwrite(offs_t offset, uint8_t data);
	void set_gromlines(line_state mline, line_state moline, line_state gsq) override;

private:
	bool    m_waddr_LSB;
	bool    m_grom_selected;
	bool    m_grom_read_mode;
	bool    m_grom_address_mode;
};
} } } // end namespace bus::ti99::gromport

DECLARE_DEVICE_TYPE_NS(TI99_CART, bus::ti99::gromport, ti99_cartridge_device)

#endif // MAME_BUS_TI99_GROMPORT_CARTRIDGES_H
