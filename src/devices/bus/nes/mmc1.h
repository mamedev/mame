// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __NES_MMC1_H
#define __NES_MMC1_H

#include "nxrom.h"


// ======================> nes_sxrom_device

class nes_sxrom_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_sxrom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
	nes_sxrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual uint8_t read_m(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_m(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual void update_regs(int reg);      // this is needed to simplify NES-EVENT pcb implementation, which handle differently some regs!

	virtual void pcb_reset() override;

protected:
	void resync_callback(void *ptr, int32_t param);
	virtual void set_prg();
	virtual void set_chr();

	uint8_t m_reg[4];
	int m_reg_write_enable;
	int m_latch;
	int m_count;
};

class nes_sorom_device : public nes_sxrom_device
{
public:
	// construction/destruction
	nes_sorom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_m(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_m(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;
};

class nes_sxrom_a_device : public nes_sxrom_device
{
public:
	// construction/destruction
	nes_sxrom_a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_m(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_m(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
};

class nes_sorom_a_device : public nes_sxrom_device
{
public:
	// construction/destruction
	nes_sorom_a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_m(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_m(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;
};




// device type definition
extern const device_type NES_SXROM;
extern const device_type NES_SOROM;
extern const device_type NES_SXROM_A;
extern const device_type NES_SOROM_A;


#endif
