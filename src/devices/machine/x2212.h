// license:BSD-3-Clause
// copyright-holders:smf,Barry Rodewald
/***************************************************************************

    x2212.h

    Xicor X2212 256 x 4 bit Nonvolatile Static RAM.

***************************************************************************/

#ifndef MAME_MACHINE_X2212_H
#define MAME_MACHINE_X2212_H

#pragma once



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_X2212_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, X2212, 0)
// some systems (like many early Atari games) wire up the /STORE signal
// to fire on power-down, effectively creating an "auto-save" functionality
#define MCFG_X2212_ADD_AUTOSAVE(_tag) \
	MCFG_DEVICE_ADD(_tag, X2212, 0) \
	downcast<x2212_device &>(*device).set_auto_save(true);

#define MCFG_X2210_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, X2210, 0)

#define MCFG_X2210_ADD_AUTOSAVE(_tag) \
	MCFG_DEVICE_ADD(_tag, X2210, 0) \
	downcast<x2212_device &>(*device).set_auto_save(true);


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
	x2212_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// inline configuration helpers
	void set_auto_save(bool auto_save) { m_auto_save = auto_save; }

	// I/O operations
	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_WRITE_LINE_MEMBER( store );
	DECLARE_WRITE_LINE_MEMBER( recall );

protected:
	x2212_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int size_data);

	// device-level overrides
	virtual void device_start() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual void nvram_read(emu_file &file) override;
	virtual void nvram_write(emu_file &file) override;

private:
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

	int const m_size_data;
	optional_region_ptr<uint8_t> m_default_data;

	// internal helpers
	void store();
	void recall();

	void x2212_e2prom_map(address_map &map);
	void x2212_sram_map(address_map &map);
};

class x2210_device : public x2212_device
{
public:
	x2210_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// device type definition
DECLARE_DEVICE_TYPE(X2212, x2212_device)
DECLARE_DEVICE_TYPE(X2210, x2210_device)

#endif // MAME_MACHINE_X2212_H
