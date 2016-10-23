// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __NES_SACHEN_H
#define __NES_SACHEN_H

#include "nxrom.h"


// ======================> nes_sachen_sa009_device

class nes_sachen_sa009_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_sachen_sa009_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void write_l(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;
};


// ======================> nes_sachen_sa0036_device

class nes_sachen_sa0036_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_sachen_sa0036_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;
};


// ======================> nes_sachen_sa0037_device

class nes_sachen_sa0037_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_sachen_sa0037_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;
};


// ======================> nes_sachen_sa72007_device

class nes_sachen_sa72007_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_sachen_sa72007_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void write_l(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;
};


// ======================> nes_sachen_sa72008_device

class nes_sachen_sa72008_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_sachen_sa72008_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void write_l(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;
};


// ======================> nes_sachen_tca01_device

class nes_sachen_tca01_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_sachen_tca01_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual uint8_t read_l(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;
};


// ======================> nes_sachen_tcu01_device

class nes_sachen_tcu01_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_sachen_tcu01_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void write_l(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual void write_m(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override { write_l(space, (offset + 0x100) & 0xfff, data, mem_mask); }
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override { write_l(space, (offset + 0x100) & 0xfff, data, mem_mask); }

	virtual void pcb_reset() override;
};


// ======================> nes_sachen_tcu02_device

class nes_sachen_tcu02_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_sachen_tcu02_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual uint8_t read_l(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_l(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;

private:
	uint8_t m_latch;
};


// ======================> nes_sachen_74x374_device

class nes_sachen_74x374_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_sachen_74x374_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
	nes_sachen_74x374_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual uint8_t read_l(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_l(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;

protected:
	void set_mirror(uint8_t nt);
	uint8_t m_latch, m_mmc_vrom_bank;
};


// ======================> nes_sachen_74x374_alt_device

class nes_sachen_74x374_alt_device : public nes_sachen_74x374_device
{
public:
	// construction/destruction
	nes_sachen_74x374_alt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual uint8_t read_l(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override { return 0xff; }   // no read_l here
	virtual void write_l(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
};


// ======================> nes_sachen_8259a_device

class nes_sachen_8259a_device : public nes_sachen_74x374_device
{
public:
	// construction/destruction
	nes_sachen_8259a_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
	nes_sachen_8259a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void write_l(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual void write_m(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override { write_l(space, (offset + 0x100) & 0xfff, data, mem_mask); }

	virtual void pcb_reset() override;

protected:
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
extern const device_type NES_SACHEN_SA009;
extern const device_type NES_SACHEN_SA0036;
extern const device_type NES_SACHEN_SA0037;
extern const device_type NES_SACHEN_SA72007;
extern const device_type NES_SACHEN_SA72008;
extern const device_type NES_SACHEN_TCA01;
extern const device_type NES_SACHEN_TCU01;
extern const device_type NES_SACHEN_TCU02;
extern const device_type NES_SACHEN_74X374;
extern const device_type NES_SACHEN_74X374_ALT;
extern const device_type NES_SACHEN_8259A;
extern const device_type NES_SACHEN_8259B;
extern const device_type NES_SACHEN_8259C;
extern const device_type NES_SACHEN_8259D;

#endif
