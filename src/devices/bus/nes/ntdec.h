// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_NTDEC_H
#define MAME_BUS_NES_NTDEC_H

#pragma once

#include "nxrom.h"


// ======================> nes_ntdec_asder_device

class nes_ntdec_asder_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ntdec_asder_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	u8 m_latch, m_chr_outer;
};


// ======================> nes_ntdec_fh_device

class nes_ntdec_fh_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ntdec_fh_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_m(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;
};


// ======================> nes_ntdec_n715021_device

class nes_ntdec_n715021_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ntdec_n715021_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_NTDEC_ASDER,   nes_ntdec_asder_device)
DECLARE_DEVICE_TYPE(NES_NTDEC_FH,      nes_ntdec_fh_device)
DECLARE_DEVICE_TYPE(NES_NTDEC_N715021, nes_ntdec_n715021_device)

#endif // MAME_BUS_NES_NTDEC_H
