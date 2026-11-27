// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Brian H. Gilbert
#ifndef MAME_BUS_NES_AVE_H
#define MAME_BUS_NES_AVE_H

#pragma once

#include "nxrom.h"


// ======================> nes_nina001_device

class nes_nina001_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_nina001_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_m(offs_t offset, u8 data) override;
};


// ======================> nes_nina006_device

class nes_nina006_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_nina006_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_l(offs_t offset, u8 data) override;
};


// ======================> nes_maxi15_device

class nes_maxi15_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_maxi15_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_h(offs_t offset) override;
	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	void latch(offs_t offset, u8 value);

	u8 m_reg[2];
};


// ======================> nes_nina08_device

class nes_nina08_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_nina08_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_l(offs_t offset, u8 data) override;
	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	void update_banks();

	u8 m_outer;     // $4180
	u8 m_nina;      // $4100, NINA-03-compatible inner bank
	u8 m_cd;        // $8000, Color-Dreams-compatible inner bank
};


// ======================> nes_maxi30p_device

class nes_maxi30p_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_maxi30p_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_m(offs_t offset, u8 data) override;
	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	void update_banks();

	u8 m_outer;     // $6000-$7FFF
	u8 m_inner;     // $8000-$FFFF
};


// device type definition
DECLARE_DEVICE_TYPE(NES_NINA001, nes_nina001_device)
DECLARE_DEVICE_TYPE(NES_NINA006, nes_nina006_device)
DECLARE_DEVICE_TYPE(NES_MAXI15,  nes_maxi15_device)
DECLARE_DEVICE_TYPE(NES_NINA08,  nes_nina08_device)
DECLARE_DEVICE_TYPE(NES_MAXI30P, nes_maxi30p_device)

#endif // MAME_BUS_NES_AVE_H
