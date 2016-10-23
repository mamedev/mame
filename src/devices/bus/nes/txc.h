// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __NES_TXC_H
#define __NES_TXC_H

#include "nxrom.h"


// ======================> nes_txc_22211_device

class nes_txc_22211_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_txc_22211_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
	nes_txc_22211_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual uint8_t read_l(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_l(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;

protected:
	uint8_t m_reg[4];
};


// ======================> nes_txc_dumarac_device

class nes_txc_dumarc_device : public nes_txc_22211_device
{
public:
	// construction/destruction
	nes_txc_dumarc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
};


// ======================> nes_txc_mjblock_device

class nes_txc_mjblock_device : public nes_txc_22211_device
{
public:
	// construction/destruction
	nes_txc_mjblock_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual uint8_t read_l(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
};


// ======================> nes_txc_strikew_device

class nes_txc_strikew_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_txc_strikew_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;
};


// ======================> nes_txc_commandos_device

class nes_txc_commandos_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_txc_commandos_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual uint8_t read_l(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_h(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void pcb_reset() override;
};




// device type definition
extern const device_type NES_TXC_22211;
extern const device_type NES_TXC_DUMARACING;
extern const device_type NES_TXC_MJBLOCK;
extern const device_type NES_TXC_STRIKEW;
extern const device_type NES_TXC_COMMANDOS;

#endif
