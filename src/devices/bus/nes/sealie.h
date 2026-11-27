// license:BSD-3-Clause
// copyright-holders:kmg,Brian H. Gilbert
#ifndef MAME_BUS_NES_SEALIE_H
#define MAME_BUS_NES_SEALIE_H

#pragma once

#include "nxrom.h"


// ======================> nes_cufrom_device

class nes_cufrom_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_cufrom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;
};


// ======================> nes_dpcmcart_device

class nes_dpcmcart_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_dpcmcart_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;
};


// ======================> nes_unrom512_device

class nes_unrom512_device : public nes_nrom_device, public device_nvram_interface
{
public:
	virtual void set_unrom512_flashable(bool val) override { m_flashable = val; }
	virtual void set_unrom512_submapper(u8 val) override { m_submapper = val; }
	// construction/destruction
	nes_unrom512_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	virtual void device_start() override ATTR_COLD;

	// flash contents are kept as non-volatile data (self-flashable boards only)
	virtual void nvram_default() override { }
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;
	virtual bool nvram_can_write() const override { return m_flashable; }

	// construction/destruction
	nes_unrom512_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

private:
	void flash_w(offs_t offset, u8 data);

	bool m_flashable = false;
	u8 m_submapper = 0;
	u8 m_latch = 0;
	u8 m_flash_step = 0;    // position in the SST39SF0x0 command sequence
	u8 m_flash_cmd = 0;     // $80 (erase prefix) or $A0 (program)
};


// ======================> nes_8bitxmas_device

class nes_8bitxmas_device : public nes_unrom512_device
{
public:
	// construction/destruction
	nes_8bitxmas_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	void update_led();
	u8 m_led;
};



// ======================> nes_8bitxmas17_device

class nes_8bitxmas17_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_8bitxmas17_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_m(offs_t offset, u8 data) override;
	virtual void write_h(offs_t offset, u8 data) override;
	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	void update_banks();

	u8 m_outer_lo;      // 128 KiB outer bank of the $8000-$BFFF window
	u8 m_outer_hi;      // 128 KiB outer bank of the $C000-$FFFF window
	u8 m_inner;         // 16 KiB inner bank of the $8000-$BFFF window
	u8 m_chr;           // 8 KiB CHR-RAM bank
	u8 m_led;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_8BITXMAS, nes_8bitxmas_device)
DECLARE_DEVICE_TYPE(NES_8BITXMAS17, nes_8bitxmas17_device)
DECLARE_DEVICE_TYPE(NES_CUFROM,   nes_cufrom_device)
DECLARE_DEVICE_TYPE(NES_DPCMCART, nes_dpcmcart_device)
DECLARE_DEVICE_TYPE(NES_UNROM512, nes_unrom512_device)

#endif // MAME_BUS_NES_SEALIE_H
