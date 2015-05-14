// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2cffa.h

    Implementation of Rich Dreher's IDE/CompactFlash board for
    the Apple II

*********************************************************************/

#ifndef __A2BUS_CFFA2__
#define __A2BUS_CFFA2__

#include "emu.h"
#include "a2bus.h"
#include "machine/ataintf.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_cffa2000_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_cffa2000_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual const rom_entry *device_rom_region() const;

protected:
	virtual void device_start();
	virtual void device_reset();

	// overrides of standard a2bus slot functions
	virtual UINT8 read_c0nx(address_space &space, UINT8 offset);
	virtual void write_c0nx(address_space &space, UINT8 offset, UINT8 data);
	virtual UINT8 read_cnxx(address_space &space, UINT8 offset);
	virtual UINT8 read_c800(address_space &space, UINT16 offset);
	virtual void write_c800(address_space &space, UINT16 offset, UINT8 data);

	required_device<ata_interface_device> m_ata;

	UINT8 *m_rom;
	UINT8 m_eeprom[0x1000];

private:
	UINT16 m_lastdata, m_lastreaddata;
	bool m_writeprotect;
	bool m_inwritecycle;
};

class a2bus_cffa2_device : public a2bus_cffa2000_device, public device_nvram_interface
{
public:
	a2bus_cffa2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device_config_nvram_interface overrides
	virtual void nvram_default();
	virtual void nvram_read(emu_file &file);
	virtual void nvram_write(emu_file &file);
};

class a2bus_cffa2_6502_device : public a2bus_cffa2000_device, public device_nvram_interface
{
public:
	a2bus_cffa2_6502_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual const rom_entry *device_rom_region() const;

protected:
	// device_config_nvram_interface overrides
	virtual void nvram_default();
	virtual void nvram_read(emu_file &file);
	virtual void nvram_write(emu_file &file);
};

// device type definition
extern const device_type A2BUS_CFFA2;
extern const device_type A2BUS_CFFA2_6502;

#endif /* __A2BUS_CFFA2__ */
