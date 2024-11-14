// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_SACHEN_H
#define MAME_BUS_NES_SACHEN_H

#pragma once

#include "nxrom.h"


// ======================> nes_sachen_sa009_device

class nes_sachen_sa009_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_sachen_sa009_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_l(offs_t offset, uint8_t data) override;
};


// ======================> nes_sachen_sa0036_device

class nes_sachen_sa0036_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_sachen_sa0036_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;
};


// ======================> nes_sachen_sa0037_device

class nes_sachen_sa0037_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_sachen_sa0037_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;
};


// ======================> nes_sachen_sa72007_device

class nes_sachen_sa72007_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_sachen_sa72007_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_l(offs_t offset, uint8_t data) override;
};


// ======================> nes_sachen_sa72008_device

class nes_sachen_sa72008_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_sachen_sa72008_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_l(offs_t offset, uint8_t data) override;
};


// ======================> nes_sachen_tca01_device

class nes_sachen_tca01_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_sachen_tca01_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_l(offs_t offset) override;

	virtual void pcb_reset() override;
};


// ======================> nes_sachen_tcu01_device

class nes_sachen_tcu01_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_sachen_tcu01_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_l(offs_t offset, uint8_t data) override;
	virtual void write_m(offs_t offset, uint8_t data) override { write_l((offset + 0x100) & 0xfff, data); }
	virtual void write_h(offs_t offset, uint8_t data) override { write_l((offset + 0x100) & 0xfff, data); }
};


// ======================> nes_sachen_tcu02_device

class nes_sachen_tcu02_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_sachen_tcu02_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_l(offs_t offset) override;
	virtual void write_l(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	uint8_t m_latch;
};


// ======================> nes_sachen_3013_device

class nes_sachen_3013_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_sachen_3013_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_h(offs_t offset) override;
};


// ======================> nes_sachen_3014_device

class nes_sachen_3014_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_sachen_3014_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_h(offs_t offset) override;
	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	u8 m_latch;
};


// ======================> nes_sachen_74x374_device

class nes_sachen_74x374_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_sachen_74x374_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_l(offs_t offset) override;
	virtual void write_l(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	nes_sachen_74x374_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	void set_mirror(uint8_t nt);
	uint8_t m_latch, m_mmc_vrom_bank;
};


// ======================> nes_sachen_74x374_alt_device

class nes_sachen_74x374_alt_device : public nes_sachen_74x374_device
{
public:
	// construction/destruction
	nes_sachen_74x374_alt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_l(offs_t offset) override { return 0xff; }   // no read_l here
	virtual void write_l(offs_t offset, uint8_t data) override;
};


// ======================> nes_sachen_8259a_device

class nes_sachen_8259a_device : public nes_sachen_74x374_device
{
public:
	// construction/destruction
	nes_sachen_8259a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_l(offs_t offset, uint8_t data) override;
	virtual void write_m(offs_t offset, uint8_t data) override { write_l((offset + 0x100) & 0xfff, data); }

	virtual void pcb_reset() override;

protected:
	nes_sachen_8259a_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	virtual void chr_update();
	uint8_t m_reg[8];
};


// ======================> nes_sachen_8259b_device

class nes_sachen_8259b_device : public nes_sachen_8259a_device
{
public:
	// construction/destruction
	nes_sachen_8259b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void chr_update() override;
};


// ======================> nes_sachen_8259c_device

class nes_sachen_8259c_device : public nes_sachen_8259a_device
{
public:
	// construction/destruction
	nes_sachen_8259c_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void chr_update() override;
};


// ======================> nes_sachen_8259d_device

class nes_sachen_8259d_device : public nes_sachen_8259a_device
{
public:
	// construction/destruction
	nes_sachen_8259d_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void pcb_reset() override;

protected:
	virtual void chr_update() override;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_SACHEN_SA009,      nes_sachen_sa009_device)
DECLARE_DEVICE_TYPE(NES_SACHEN_SA0036,     nes_sachen_sa0036_device)
DECLARE_DEVICE_TYPE(NES_SACHEN_SA0037,     nes_sachen_sa0037_device)
DECLARE_DEVICE_TYPE(NES_SACHEN_SA72007,    nes_sachen_sa72007_device)
DECLARE_DEVICE_TYPE(NES_SACHEN_SA72008,    nes_sachen_sa72008_device)
DECLARE_DEVICE_TYPE(NES_SACHEN_TCA01,      nes_sachen_tca01_device)
DECLARE_DEVICE_TYPE(NES_SACHEN_TCU01,      nes_sachen_tcu01_device)
DECLARE_DEVICE_TYPE(NES_SACHEN_TCU02,      nes_sachen_tcu02_device)
DECLARE_DEVICE_TYPE(NES_SACHEN_3013,       nes_sachen_3013_device)
DECLARE_DEVICE_TYPE(NES_SACHEN_3014,       nes_sachen_3014_device)
DECLARE_DEVICE_TYPE(NES_SACHEN_74X374,     nes_sachen_74x374_device)
DECLARE_DEVICE_TYPE(NES_SACHEN_74X374_ALT, nes_sachen_74x374_alt_device)
DECLARE_DEVICE_TYPE(NES_SACHEN_8259A,      nes_sachen_8259a_device)
DECLARE_DEVICE_TYPE(NES_SACHEN_8259B,      nes_sachen_8259b_device)
DECLARE_DEVICE_TYPE(NES_SACHEN_8259C,      nes_sachen_8259c_device)
DECLARE_DEVICE_TYPE(NES_SACHEN_8259D,      nes_sachen_8259d_device)

#endif // MAME_BUS_NES_SACHEN_H
