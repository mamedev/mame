// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __SAT_BRAM_H
#define __SAT_BRAM_H

#include "sat_slot.h"


// ======================> saturn_bram_device

class saturn_bram_device : public device_t,
							public device_sat_cart_interface,
							public device_nvram_interface
{
public:
	// construction/destruction
	saturn_bram_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual void nvram_read(emu_file &file) override { if (!m_ext_bram.empty()) file.read(&m_ext_bram[0], m_ext_bram.size()); }
	virtual void nvram_write(emu_file &file) override { if (!m_ext_bram.empty()) file.write(&m_ext_bram[0], m_ext_bram.size()); }

	// reading and writing
	virtual uint32_t read_ext_bram(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff) override;
	virtual void write_ext_bram(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff) override;
};

class saturn_bram4mb_device : public saturn_bram_device
{
public:
	// construction/destruction
	saturn_bram4mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class saturn_bram8mb_device : public saturn_bram_device
{
public:
	// construction/destruction
	saturn_bram8mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class saturn_bram16mb_device : public saturn_bram_device
{
public:
	// construction/destruction
	saturn_bram16mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class saturn_bram32mb_device : public saturn_bram_device
{
public:
	// construction/destruction
	saturn_bram32mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};



// device type definition
extern const device_type SATURN_BRAM_4MB;
extern const device_type SATURN_BRAM_8MB;
extern const device_type SATURN_BRAM_16MB;
extern const device_type SATURN_BRAM_32MB;

#endif
