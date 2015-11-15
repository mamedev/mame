// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    TI-99/8 main board logic

    This component implements the address decoder and mapper logic from the
    TI-99/8 console.

    See 998board.c for documentation

    Michael Zapf

*****************************************************************************/

#ifndef __MAPPER8__
#define __MAPPER8__

#include "emu.h"
#include "ti99defs.h"
#include "sound/tms5220.h"

extern const device_type MAINBOARD8;
extern const device_type OSO;
extern const device_type SPEECH8;

#define OSO_TAG "oso"
#define SPEECHSYN_TAG "speechsyn"

#define NATIVE 0
#define TI99EM 1
#define PATGEN 2
#define PHYSIC 3
#define CONT 0
#define STOP 1

#define SRAMNAME "SRAM"
#define ROM0NAME "ROM0"
#define ROM1A0NAME "ROM1A"
#define ROM1C0NAME "ROM1C"
#define INTSNAME "INTS"
#define DRAMNAME "DRAM"
#define PCODENAME "PCODE"

#define SRAM_SIZE 2048
#define DRAM_SIZE 65536

// We use these constants in the read/write functions.
enum mapper8_device_kind
{
	MAP8_UNDEF = 0,
	MAP8_SRAM,
	MAP8_ROM0,
	MAP8_ROM1A0,
	MAP8_ROM1C0,
	MAP8_DRAM,
	MAP8_PCODE,
	MAP8_INTS,
	MAP8_DEV        // device by name
};

struct mapper8_list_entry
{
	const char* name;               // Name of the device (used for looking up the device)
	int         mode;               // Mode of the system which applies to this entry
	int         stop;               // Mapper shall stop searching for a matching device when this entry applies
	UINT32      select_pattern;     // State of the address line bits when addressing this device
	UINT32      address_mask;       // Bits of the address bus used to address this device
	UINT32      write_select;       // Additional bits set when doing write accesses to this device
};

#define MAPPER8_CONFIG(name) \
	const mapper8_config(name) =

#define MCFG_MAINBOARD8_READY_CALLBACK(_write) \
	devcb = &mainboard8_device::set_ready_wr_callback(*device, DEVCB_##_write);

struct mapper8_config
{
	const mapper8_list_entry        *devlist;
};

/*
    Device list of the mapper.
*/
class logically_addressed_device
{
	friend class simple_list<logically_addressed_device>;
	friend class mainboard8_device;

public:
	logically_addressed_device(mapper8_device_kind kind, device_t *busdevice, const mapper8_list_entry &entry)
	: m_next(nullptr), m_kind(kind), m_device(busdevice), m_config(&entry) { };

private:
	logically_addressed_device     *m_next;        // needed for simple_list
	mapper8_device_kind       m_kind;         // named device or predefined
	device_t                 *m_device;      // the actual device
	const mapper8_list_entry *m_config;
};

/*
    Device list of the mapper.
*/
class physically_addressed_device
{
	friend class simple_list<physically_addressed_device>;
	friend class mainboard8_device;

public:
	physically_addressed_device(mapper8_device_kind kind, device_t *busdevice, const mapper8_list_entry &entry)
	: m_next(nullptr), m_kind(kind), m_device(busdevice), m_config(&entry) { };

private:
	physically_addressed_device       *m_next;        // needed for simple_list
	mapper8_device_kind          m_kind;          // named device or predefined
	device_t                    *m_device;      // the actual device
	const mapper8_list_entry    *m_config;
};

/*
    Custom chip: OSO
*/
class ti998_oso_device : public device_t
{
public:
	ti998_oso_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	void device_start();

private:
	UINT8 m_data;
	UINT8 m_status;
	UINT8 m_control;
	UINT8 m_xmit;
};

/*
    Speech support
*/
#define MCFG_SPEECH8_READY_CALLBACK(_write) \
	devcb = &ti998_spsyn_device::set_ready_wr_callback(*device, DEVCB_##_write);

class ti998_spsyn_device : public bus8z_device
{
public:
	ti998_spsyn_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_ready_wr_callback(device_t &device, _Object object) { return downcast<ti998_spsyn_device &>(device).m_ready.set_callback(object); }

	DECLARE_READ8Z_MEMBER(readz);
	DECLARE_WRITE8_MEMBER(write);

	DECLARE_READ8Z_MEMBER(crureadz) { };
	DECLARE_WRITE8_MEMBER(cruwrite) { };

	DECLARE_WRITE_LINE_MEMBER( speech8_ready );

	DECLARE_READ8_MEMBER( spchrom_read );
	DECLARE_WRITE8_MEMBER( spchrom_load_address );
	DECLARE_WRITE8_MEMBER( spchrom_read_and_branch );

protected:
	virtual void    device_start();
	virtual void    device_reset(void);
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;

private:
	tms5220_device  *m_vsp;
//  UINT8           *m_speechrom;           // pointer to speech ROM data
//  int             m_load_pointer;         // which 4-bit nibble will be affected by load address
//  int             m_rombits_count;        // current bit position in ROM
//  UINT32          m_sprom_address;        // 18 bit pointer in ROM
//  UINT32          m_sprom_length;         // length of data pointed by speechrom_data, from 0 to 2^18

	// Ready line to the CPU
	devcb_write_line m_ready;
};

#define MCFG_TISPEECH8_ADD(_tag, _conf)     \
	MCFG_DEVICE_ADD(_tag, TI99_SPEECH8, 0)  \
	MCFG_DEVICE_CONFIG(_conf)


/*
    Main class
*/
class mainboard8_device : public bus8z_device
{
public:
	mainboard8_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_ready_wr_callback(device_t &device, _Object object) { return downcast<mainboard8_device &>(device).m_ready.set_callback(object); }

	DECLARE_READ8_MEMBER( readm);       // used from address map
	DECLARE_WRITE8_MEMBER( writem );    // used from address map

	DECLARE_READ8Z_MEMBER( readz );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_READ8Z_MEMBER(crureadz);
	DECLARE_WRITE8_MEMBER(cruwrite);

	void CRUS_set(bool state);
	void PTGE_set(bool state);

	void clock_in(int state);

protected:
	void device_start(void);
	void device_reset(void);
	machine_config_constructor device_mconfig_additions() const;

private:
	bool access_logical_r(address_space& space, offs_t offset, UINT8 *value, UINT8 mem_mask );
	bool access_logical_w(address_space& space, offs_t offset, UINT8 data, UINT8 mem_mask );
	void access_physical_r(address_space& space, offs_t offset, UINT8 *value, UINT8 mem_mask );
	void access_physical_w(address_space& space, offs_t offset, UINT8 data, UINT8 mem_mask );
	void mapwrite(int offset, UINT8 data);

	// Ready line to the CPU
	devcb_write_line m_ready;

	// All devices that are attached to the 16-bit address bus.
	simple_list<logically_addressed_device> m_logcomp;

	// All devices that are attached to the 24-bit mapped address bus.
	simple_list<physically_addressed_device> m_physcomp;

	// Select bit for the internal DSR.
	bool    m_dsr_selected;

	// Select bit for the Hexbus DSR.
	bool    m_hexbus_selected;

	// 99/4A compatibility mode. Name is taken from the spec. If 1, 99/4A compatibility is active.
	bool    m_CRUS;

	// P-Code mode. Name is taken from the spec. If 0, P-Code libraries are available.
	// May be read as "Pascal and Text-to-speech GROM libraries Enable (Negative)"
	// Note: this is negative logic. GROMs are present only for PTGEN=0
	// We use PTGE as the inverse signal.
	bool    m_PTGE;

	// Counter for the wait states.
	int   m_waitcount;

	// Address mapper registers. Each offset is selected by the first 4 bits
	// of the logical address.
	UINT32  m_pas_offset[16];

	// SRAM area of the system. Directly connected to the address decoder.
	UINT8   *m_sram;

	// DRAM area of the system. Connected to the mapped address bus.
	UINT8   *m_dram;

	// ROM area of the system. Directly connected to the logical address decoder.
	UINT8   *m_rom0;

	// ROM area of the system. Directly connected to the physical address decoder.
	UINT8   *m_rom1;

	// P-Code ROM area of the system. Directly connected to the physical address decoder.
	UINT8   *m_pcode;

	// Custom chips
	required_device<ti998_oso_device> m_oso;
};


#define MCFG_MAINBOARD8_ADD(_tag, _devices)            \
	MCFG_DEVICE_ADD(_tag, MAINBOARD8, 0) \
	MCFG_DEVICE_CONFIG( _devices )

#endif
