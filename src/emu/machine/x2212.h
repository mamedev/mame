/***************************************************************************

    x2212.h

    Xicor X2212 256 x 4 bit Nonvolatile Static RAM.

***************************************************************************/

#pragma once

#ifndef __X2212_H__
#define __X2212_H__



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MDRV_X2212_ADD(_tag) \
	MDRV_DEVICE_ADD(_tag, X2212, 0) \

// some systems (like many early Atari games) wire up the /STORE signal
// to fire on power-down, effectively creating an "auto-save" functionality
#define MDRV_X2212_ADD_AUTOSAVE(_tag) \
	MDRV_DEVICE_ADD(_tag, X2212, 0) \
	x2212_device_config::static_set_auto_save(device); \



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> x2212_device_config

class x2212_device_config :	public device_config,
							public device_config_memory_interface,
							public device_config_nvram_interface
{
	friend class x2212_device;

	// construction/destruction
	x2212_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);

public:
	// allocators
	static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
	virtual device_t *alloc_device(running_machine &machine) const;

	// inline configuration helpers
	static void static_set_auto_save(device_config *device);

protected:
	// device_config_memory_interface overrides
	virtual const address_space_config *memory_space_config(int spacenum = 0) const;

	// device-specific configuration
	address_space_config		m_sram_space_config;
	address_space_config		m_e2prom_space_config;

	// internal state
	bool						m_auto_save;
};


// ======================> x2212_device

class x2212_device :	public device_t,
						public device_memory_interface,
						public device_nvram_interface
{
	friend class x2212_device_config;

	// construction/destruction
	x2212_device(running_machine &_machine, const x2212_device_config &config);

public:
	// I/O operations
	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_WRITE_LINE_MEMBER( store );
	DECLARE_WRITE_LINE_MEMBER( recall );

protected:
	// internal helpers
	void store();
	void recall();

	// device-level overrides
	virtual void device_start();

	// device_nvram_interface overrides
	virtual void nvram_default();
	virtual void nvram_read(mame_file &file);
	virtual void nvram_write(mame_file &file);

	static const int SIZE_DATA = 0x100;

	// internal state
	const x2212_device_config &m_config;

	address_space *	m_sram;
	address_space *	m_e2prom;

	bool		m_store;
	bool		m_array_recall;
};


// device type definition
extern const device_type X2212;


#endif
