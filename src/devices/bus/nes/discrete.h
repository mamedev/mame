// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Brian H. Gilbert
#ifndef MAME_BUS_NES_DISCRETE_H
#define MAME_BUS_NES_DISCRETE_H

#pragma once

#include "nxrom.h"


// ======================> nes_74x161x161x32_device

class nes_74x161x161x32_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_74x161x161x32_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;
};


// ======================> nes_74x139x74_device

class nes_74x139x74_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_74x139x74_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_m(offs_t offset, u8 data) override;
};


// ======================> nes_74x377_device

class nes_74x377_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_74x377_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;
};


// ======================> nes_74x161x138_device

class nes_74x161x138_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_74x161x138_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_m(offs_t offset, u8 data) override;
};


// ======================> nes_gtrom_device

class nes_gtrom_device : public nes_nrom_device, public device_nvram_interface
{
public:
	// construction/destruction
	nes_gtrom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void set_flashable(bool val) override { m_flashable = val; }

	virtual void write_l(offs_t offset, u8 data) override;
	virtual void write_m(offs_t offset, u8 data) override;
	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	virtual void device_start() override ATTR_COLD;

	// flash contents are kept as non-volatile data (self-flashable boards only)
	virtual void nvram_default() override { }
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;
	virtual bool nvram_can_write() const override { return m_flashable; }

private:
	void update_banks();
	void flash_w(offs_t offset, u8 data);

	bool m_flashable = false;
	u8 m_reg = 0;           // GRNC PPPP
	u8 m_flash_step = 0;    // position in the SST39SF040 command sequence
	u8 m_flash_cmd = 0;     // $80 (erase prefix) or $A0 (program)
};


// device type definition
DECLARE_DEVICE_TYPE(NES_74X161X161X32, nes_74x161x161x32_device)
DECLARE_DEVICE_TYPE(NES_74X139X74,     nes_74x139x74_device)
DECLARE_DEVICE_TYPE(NES_74X377,        nes_74x377_device)
DECLARE_DEVICE_TYPE(NES_74X161X138,    nes_74x161x138_device)
DECLARE_DEVICE_TYPE(NES_GTROM,         nes_gtrom_device)

#endif // MAME_BUS_NES_DISCRETE_H
