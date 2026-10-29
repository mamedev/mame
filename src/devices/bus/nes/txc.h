// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Brian H. Gilbert
#ifndef MAME_BUS_NES_TXC_H
#define MAME_BUS_NES_TXC_H

#pragma once

#include "nxrom.h"
#include "sound/tms5220.h"


// ======================> nes_txc_22211_device

class nes_txc_22211_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_txc_22211_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_l(offs_t offset) override;
	virtual void write_l(offs_t offset, uint8_t data) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	nes_txc_22211_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	virtual void update_banks();

	// TXC 05-00002-010 / JV001 latch-adder-inverter (NESdev INES Mapper 132, 172, 173)
	uint8_t m_input;    // 'P' staging register ($4102)
	uint8_t m_reg;      // 'R' register, loaded or incremented by $4100
	uint8_t m_output;   // copied from 'R' by any $8000-$FFFF write
	uint8_t m_s;        // 'S' bit (mapper 132/173, read back directly)
	uint8_t m_invert;   // inVert mode ($4101)
	uint8_t m_increment;// inCrement mode ($4103)
};


// ======================> nes_txc_dumarc_device

class nes_txc_dumarc_device : public nes_txc_22211_device
{
public:
	// construction/destruction
	nes_txc_dumarc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_l(offs_t offset) override;
	virtual void write_l(offs_t offset, uint8_t data) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

protected:
	virtual void update_banks() override;
};


// ======================> nes_txc_mjblock_device

class nes_txc_mjblock_device : public nes_txc_22211_device
{
public:
	// construction/destruction
	nes_txc_mjblock_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_l(offs_t offset, uint8_t data) override;

protected:
	virtual void update_banks() override;
};


// ======================> nes_txc_strikew_device

class nes_txc_strikew_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_txc_strikew_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_l(offs_t offset) override;
	virtual void write_l(offs_t offset, u8 data) override;
	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	u8 m_reg[4];
};


// ======================> nes_txc_mxmdhtwo_device

class nes_txc_mxmdhtwo_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_txc_mxmdhtwo_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_l(offs_t offset) override;
	virtual void write_l(offs_t offset, uint8_t data) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void lpc_reset();

	required_device<tms5220c_device> m_lpc;   // TMS5220C-data-compatible LPC speech chip of the educational carts
	uint8_t m_lpc_nibble;                     // low nibble waiting for its high nibble
	uint8_t m_lpc_have_nibble;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_TXC_22211,      nes_txc_22211_device)
DECLARE_DEVICE_TYPE(NES_TXC_DUMARACING, nes_txc_dumarc_device)
DECLARE_DEVICE_TYPE(NES_TXC_MJBLOCK,    nes_txc_mjblock_device)
DECLARE_DEVICE_TYPE(NES_TXC_STRIKEW,    nes_txc_strikew_device)
DECLARE_DEVICE_TYPE(NES_TXC_MXMDHTWO,  nes_txc_mxmdhtwo_device)

#endif // MAME_BUS_NES_TXC_H
