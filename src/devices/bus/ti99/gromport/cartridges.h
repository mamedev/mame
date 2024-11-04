// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/***************************************************************************
    Cartridge emulations
****************************************************************************/
#ifndef MAME_BUS_TI99_GROMPORT_CARTRIDGES_H
#define MAME_BUS_TI99_GROMPORT_CARTRIDGES_H

#pragma once

#include "gromport.h"

#include "imagedev/cartrom.h"
#include "machine/tmc0430.h"

#include "emuopts.h"

#include "utilfwd.h"


// declared in formats/rpk.h
class rpk_socket;

namespace bus::ti99::gromport {

class ti99_cartridge_pcb;

class ti99_cartridge_device : public device_t, public device_cartrom_image_interface
{
	friend class ti99_single_cart_conn_device;
	friend class ti99_multi_cart_conn_device;
	friend class ti99_gkracker_device;

public:
	ti99_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void readz(offs_t offset, uint8_t *value);
	void write(offs_t offset, uint8_t data);
	void crureadz(offs_t offset, uint8_t *value);
	void cruwrite(offs_t offset, uint8_t data);

	void ready_line(int state);
	void romgq_line(int state);

	void set_gromlines(line_state mline, line_state moline, line_state gsq);

	void gclock_in(int state);

	bool    is_available() { return m_pcb != nullptr; }
	bool    is_grom_idle();

protected:
	virtual void device_start() override { }
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry* device_rom_region() const override;

	// Image handling: implementation of methods which are abstract in the parent
	std::pair<std::error_condition, std::string> call_load() override;
	void call_unload() override;

	void prepare_cartridge();

	// device_image_interface
	bool is_reset_on_load() const noexcept override       { return false; }
	const char *image_interface() const noexcept override { return "ti99_cart"; }
	const char *file_extensions() const noexcept override { return "rpk"; }

	void set_connector(cartridge_connector_device* conn) { m_connector = conn; }

private:

	class ti99_rpk_socket;
	class rpk;

	static std::error_condition rpk_open(emu_options &options, std::unique_ptr<util::random_read> &&stream, const char *system_name, std::unique_ptr<rpk> &result);
	static std::error_condition rpk_load_rom_resource(const rpk_socket &socket, std::unique_ptr<ti99_rpk_socket> &result);
	static std::unique_ptr<ti99_rpk_socket> rpk_load_ram_resource(emu_options &options, const rpk_socket &socket, const char *system_name);

	class rpk
	{
		friend class ti99_cartridge_device;
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
		std::unordered_map<std::string,std::unique_ptr<ti99_rpk_socket>> m_sockets;

		void add_socket(const char* id, std::unique_ptr<ti99_rpk_socket> &&newsock);
	};

	class ti99_rpk_socket
	{
	public:
		ti99_rpk_socket(const char *id, int length, std::vector<uint8_t> &&contents);
		ti99_rpk_socket(const char *id, int length, std::vector<uint8_t> &&contents, std::string &&pathname);
		~ti99_rpk_socket() {}

		const char*     id() { return m_id; }
		int             get_content_length() { return m_length; }
		uint8_t*          get_contents() { return &m_contents[0]; }
		bool            persistent_ram() { return !m_pathname.empty(); }
		const char*     get_pathname() { return m_pathname.c_str(); }
		void            cleanup() { m_contents.clear(); }

	private:
		const char*     m_id;
		uint32_t          m_length;
		std::vector<uint8_t> m_contents;
		const std::string m_pathname;
	};

	bool    m_readrom;
	int     m_pcbtype;
	int     get_index_from_tagname();

	std::unique_ptr<ti99_cartridge_pcb> m_pcb;          // inbound
	cartridge_connector_device*    m_connector;    // outbound

	// We use dynamically allocated space instead of memory regions
	// because the required spaces are widely varying (8K to 32M)
	std::unique_ptr<u8[]> m_romspace;

	// RPK which is associated to this cartridge
	// When we close it, the contents are saved to NVRAM if available
	std::unique_ptr<rpk> m_rpk;
};

/****************************************************************************/

class ti99_cartridge_pcb
{
	friend class ti99_cartridge_device;
public:
	ti99_cartridge_pcb();
	virtual ~ti99_cartridge_pcb() { }

protected:
	virtual void readz(offs_t offset, uint8_t *value);
	virtual void write(offs_t offset, uint8_t data);
	virtual void crureadz(offs_t offset, uint8_t *value);
	virtual void cruwrite(offs_t offset, uint8_t data);

	void romgq_line(int state);
	virtual void set_gromlines(line_state mline, line_state moline, line_state gsq);
	void gclock_in(int state);

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
	int                 m_bank_mask;

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
	void readz(offs_t offset, uint8_t *value) override;
	void write(offs_t offset, uint8_t data) override;
};

/*********** Paged cartridge (others) ********************/

class ti99_paged16k_cartridge : public ti99_cartridge_pcb
{
public:
	void readz(offs_t offset, uint8_t *value) override;
	void write(offs_t offset, uint8_t data) override;
};

/*********** Paged7 cartridge (late carts) ********************/

class ti99_paged7_cartridge : public ti99_cartridge_pcb
{
public:
	void readz(offs_t offset, uint8_t *value) override;
	void write(offs_t offset, uint8_t data) override;
};

/********************** Mini Memory ***********************************/

class ti99_minimem_cartridge : public ti99_cartridge_pcb
{
public:
	void readz(offs_t offset, uint8_t *value) override;
	void write(offs_t offset, uint8_t data) override;
};

/********************* Super Space II *********************************/

class ti99_super_cartridge : public ti99_cartridge_pcb
{
public:
	void readz(offs_t offset, uint8_t *value) override;
	void write(offs_t offset, uint8_t data) override;
	void crureadz(offs_t offset, uint8_t *value) override;
	void cruwrite(offs_t offset, uint8_t data) override;
};

/************************* MBX  ***************************************/

class ti99_mbx_cartridge : public ti99_cartridge_pcb
{
public:
	void readz(offs_t offset, uint8_t *value) override;
	void write(offs_t offset, uint8_t data) override;
};

/********************** Paged 379i ************************************/

class ti99_paged379i_cartridge : public ti99_cartridge_pcb
{
public:
	void readz(offs_t offset, uint8_t *value) override;
	void write(offs_t offset, uint8_t data) override;
private:
	int     get_paged379i_bank(int rompage);
};

/********************** Paged 378 ************************************/

class ti99_paged378_cartridge : public ti99_cartridge_pcb
{
public:
	void readz(offs_t offset, uint8_t *value) override;
	void write(offs_t offset, uint8_t data) override;
};

/********************** Paged 377 ************************************/

class ti99_paged377_cartridge : public ti99_cartridge_pcb
{
public:
	void readz(offs_t offset, uint8_t *value) override;
	void write(offs_t offset, uint8_t data) override;
};

/********************** Paged CRU  ************************************/

class ti99_pagedcru_cartridge : public ti99_cartridge_pcb
{
public:
	void readz(offs_t offset, uint8_t *value) override;
	void write(offs_t offset, uint8_t data) override;
	void crureadz(offs_t offset, uint8_t *value) override;
	void cruwrite(offs_t offset, uint8_t data) override;
};

/********************** GROM emulation cartridge  ************************************/

class ti99_gromemu_cartridge : public ti99_cartridge_pcb
{
public:
	ti99_gromemu_cartridge(): m_waddr_LSB(false), m_grom_selected(false), m_grom_read_mode(false), m_grom_address_mode(false)
	{  m_grom_address = 0; }
	void readz(offs_t offset, uint8_t *value) override;
	void write(offs_t offset, uint8_t data) override;
	void gromemureadz(offs_t offset, uint8_t *value);
	void gromemuwrite(offs_t offset, uint8_t data);
	void set_gromlines(line_state mline, line_state moline, line_state gsq) override;

private:
	bool    m_waddr_LSB;
	bool    m_grom_selected;
	bool    m_grom_read_mode;
	bool    m_grom_address_mode;
};

} // end namespace bus::ti99::gromport

DECLARE_DEVICE_TYPE_NS(TI99_CART, bus::ti99::gromport, ti99_cartridge_device)

#endif // MAME_BUS_TI99_GROMPORT_CARTRIDGES_H
