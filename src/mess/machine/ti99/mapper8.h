/****************************************************************************

    TI-99/8 Address decoder and mapper

    See mapper8.c for documentation

    Michael Zapf

    February 2012: Rewritten as class

*****************************************************************************/

#ifndef __MAPPER8__
#define __MAPPER8__

#include "emu.h"
#include "peribox.h"
#include "ti99defs.h"

extern const device_type MAPPER8;

#define NATIVE 0
#define TI99EM 1
#define PATGEN 2
#define PHYSIC 3
#define CONT 0
#define STOP 1

#define SRAMNAME "SRAM"
#define ROM0NAME "ROM0"
#define ROM1NAME "ROM1"
#define ROM1ANAME "ROM1A"
#define INTSNAME "INTS"
#define DRAMNAME "DRAM"

#define SRAM_SIZE 2048
#define DRAM_SIZE 65536

struct mapper8_list_entry 
{
	const char*	name;				// Name of the device (used for looking up the device)
	int			mode;				// Mode of the system which applies to this entry
	int			stop;				// Mapper shall stop searching for a matching device when this entry applies
	UINT32		select_pattern;		// State of the address line bits when addressing this device
	UINT32		address_mask;		// Bits of the address bus used to address this device
	UINT32		write_select;		// Additional bits set when doing write accesses to this device
};

#define MAPPER8_CONFIG(name) \
	const mapper8_config(name) =

struct mapper8_config 
{
	devcb_write_line				ready;
	const mapper8_list_entry		*devlist;
};

/*
    Device list of the mapper.
*/
class log_addressed_device
{
	friend class simple_list<log_addressed_device>;
	friend class ti998_mapper_device;

public:
	log_addressed_device(device_t *busdevice, const mapper8_list_entry &entry)
	:	m_device(busdevice), m_config(&entry) { };

private:
	log_addressed_device		*m_next;		// needed for simple_list
	device_t					*m_device;		// the actual device
	const mapper8_list_entry	*m_config;
};

/*
    Device list of the mapper.
*/
class phys_addressed_device
{
	friend class simple_list<phys_addressed_device>;
	friend class ti998_mapper_device;

public:
	phys_addressed_device(device_t *busdevice, const mapper8_list_entry &entry)
	:	m_device(busdevice), m_config(&entry) { };

private:
	phys_addressed_device		*m_next;		// needed for simple_list
	device_t					*m_device;		// the actual device
	const mapper8_list_entry	*m_config;
};

/*
    Main class
*/
class ti998_mapper_device : public bus8z_device
{
public:
	ti998_mapper_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	DECLARE_READ8_MEMBER( readm);		// used from address map
	DECLARE_WRITE8_MEMBER( writem );	// used from address map

	DECLARE_READ8Z_MEMBER( readz );
	DECLARE_WRITE8_MEMBER( write );

	void crureadz(offs_t offset, UINT8 *value);
	void cruwrite(offs_t offset, UINT8 data);

	void CRUS_set(bool state);
	void PTGE_set(bool state);

	void clock_in(int state);

protected:
	void device_start(void);
	void device_reset(void);

private:
	bool search_logically_addressed_r(address_space& space, offs_t offset, UINT8 *value, UINT8 mem_mask );
	bool search_logically_addressed_w(address_space& space, offs_t offset, UINT8 data, UINT8 mem_mask );
	void search_physically_addressed_r(address_space& space, offs_t offset, UINT8 *value, UINT8 mem_mask );
	void search_physically_addressed_w(address_space& space, offs_t offset, UINT8 data, UINT8 mem_mask );
	void mapwrite(int offset, UINT8 data);

	// Ready line to the CPU
	devcb_resolved_write_line m_ready;

	// All devices that are attached to the 16-bit address bus.
	simple_list<log_addressed_device> m_logcomp;

	// All devices that are attached to the 24-bit mapped address bus.
	simple_list<phys_addressed_device> m_physcomp;

	// Select bit for the internal DSR.
	bool	m_dsr_selected;

	// 99/4A compatibility mode. Name is taken from the spec. If 1, 99/4A compatibility is active.
	bool	m_CRUS;

	// P-Code mode. Name is taken from the spec. If 0, P-Code libraries are available.
	// May be read as "Pascal and Text-to-speech GROM libraries Enable (Negative)"
	// Note: this is negative logic. GROMs are present only for PTGEN=0
	// We use PTGE as the inverse signal.
	bool	m_PTGE;

	// Counter for the wait states.
	int   m_waitcount;

	// Address mapper registers. Each offset is selected by the first 4 bits
	// of the logical address.
	UINT32	m_pas_offset[16];

	// SRAM area of the system. Directly connected to the address decoder.
	UINT8	*m_sram;

	// DRAM area of the system. Connected to the mapped address bus.
	UINT8	*m_dram;

	// ROM area of the system. Directly connected to the address decoder.
	UINT8	*m_rom;
};


#define MCFG_MAPPER8_ADD(_tag, _devices)			\
	MCFG_DEVICE_ADD(_tag, MAPPER8, 0) \
	MCFG_DEVICE_CONFIG( _devices )

#endif
