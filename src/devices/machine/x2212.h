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
	MCFG_DEVICE_ADD(_tag, X2212)
// some systems (like many early Atari games) wire up the /STORE signal
// to fire on power-down, effectively creating an "auto-save" functionality
#define MCFG_X2212_ADD_AUTOSAVE(_tag) \
	MCFG_DEVICE_ADD(_tag, X2212) \
	downcast<x2212_device &>(*device).set_auto_save(true);

#define MCFG_X2210_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, X2210)

#define MCFG_X2210_ADD_AUTOSAVE(_tag) \
	MCFG_DEVICE_ADD(_tag, X2210) \
	downcast<x2212_device &>(*device).set_auto_save(true);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> x2212_device

class x2212_device : public device_t, public device_nvram_interface
{
public:
	// construction/destruction
	x2212_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// inline configuration helpers
	void set_auto_save(bool auto_save) { m_auto_save = auto_save; }

	// I/O operations
	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_WRITE_LINE_MEMBER( store );
	DECLARE_WRITE_LINE_MEMBER( recall );

protected:
	x2212_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int size_data);

	// device-level overrides
	virtual void device_start() override;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual void nvram_read(emu_file &file) override;
	virtual void nvram_write(emu_file &file) override;

private:
	// configuration state
	bool                        m_auto_save;

	// internal state
	std::unique_ptr<u8[]> m_sram;
	std::unique_ptr<u8[]> m_e2prom;

	bool        m_store;
	bool        m_array_recall;

	int const m_size_data;
	optional_region_ptr<u8> m_default_data;

	// internal helpers
	void do_store();
	void do_recall();
};

class x2210_device : public x2212_device
{
public:
	x2210_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
};


// device type definition
DECLARE_DEVICE_TYPE(X2212, x2212_device)
DECLARE_DEVICE_TYPE(X2210, x2210_device)

#endif // MAME_MACHINE_X2212_H
