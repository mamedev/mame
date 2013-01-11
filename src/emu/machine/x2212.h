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

#define MCFG_X2212_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, X2212, 0)
// some systems (like many early Atari games) wire up the /STORE signal
// to fire on power-down, effectively creating an "auto-save" functionality
#define MCFG_X2212_ADD_AUTOSAVE(_tag) \
	MCFG_DEVICE_ADD(_tag, X2212, 0) \
	x2212_device::static_set_auto_save(*device);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> x2212_device

class x2212_device :    public device_t,
						public device_memory_interface,
						public device_nvram_interface
{
public:
	// construction/destruction
	x2212_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// inline configuration helpers
	static void static_set_auto_save(device_t &device);

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

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const;

	// device_nvram_interface overrides
	virtual void nvram_default();
	virtual void nvram_read(emu_file &file);
	virtual void nvram_write(emu_file &file);

	static const int SIZE_DATA = 0x100;

	// configuration state
	bool                        m_auto_save;

	// device-specific configuration
	address_space_config        m_sram_space_config;
	address_space_config        m_e2prom_space_config;

	// internal state
	address_space * m_sram;
	address_space * m_e2prom;

	bool        m_store;
	bool        m_array_recall;
};


// device type definition
extern const device_type X2212;


#endif
