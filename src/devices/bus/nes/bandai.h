// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __NES_BANDAI_H
#define __NES_BANDAI_H

#include "nxrom.h"
#include "machine/i2cmem.h"


// ======================> nes_oekakids_device

class nes_oekakids_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_oekakids_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_h) override;
	virtual DECLARE_READ8_MEMBER(nt_r) override;
	virtual DECLARE_WRITE8_MEMBER(nt_w) override;

	virtual void pcb_reset() override;

	virtual void ppu_latch(offs_t offset) override;

	// TODO: add oeka kids controller emulation
protected:
	void update_chr();
	UINT8 m_reg, m_latch;
};


// ======================> nes_fcg_device

class nes_fcg_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_fcg_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);
	nes_fcg_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual DECLARE_WRITE8_MEMBER(fcg_write);
	virtual DECLARE_WRITE8_MEMBER(write_m) override;

	virtual void pcb_reset() override;

protected:
	UINT16     m_irq_count;
	int        m_irq_enable;

	static const device_timer_id TIMER_IRQ = 0;
	emu_timer *irq_timer;
};


// ======================> nes_lz93d50_device

class nes_lz93d50_device : public nes_fcg_device
{
public:
	// construction/destruction
	nes_lz93d50_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);
	nes_lz93d50_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual DECLARE_WRITE8_MEMBER(write_h) override { fcg_write(space, offset, data, mem_mask); }
};


// ======================> nes_lz93d50_24c01_device

class nes_lz93d50_24c01_device : public nes_lz93d50_device
{
public:
	// construction/destruction
	nes_lz93d50_24c01_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);
	nes_lz93d50_24c01_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual DECLARE_READ8_MEMBER(read_m) override;
	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	virtual void pcb_reset() override;

	// TODO: fix EEPROM I/O emulation
	required_device<i2cmem_device> m_i2cmem;
	UINT8 m_i2c_dir;
};


// ======================> nes_lz93d50_24c02_device

class nes_lz93d50_24c02_device : public nes_lz93d50_24c01_device
{
public:
	// construction/destruction
	nes_lz93d50_24c02_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
};


// ======================> nes_fjump2_device

class nes_fjump2_device : public nes_lz93d50_device
{
public:
	// construction/destruction
	nes_fjump2_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_READ8_MEMBER(read_m) override;
	virtual DECLARE_WRITE8_MEMBER(write_m) override;
	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	virtual void pcb_reset() override;

protected:
	void set_prg();
	UINT8 m_reg[5];
};


// device type definition
extern const device_type NES_OEKAKIDS;
extern const device_type NES_FCG;
extern const device_type NES_LZ93D50;
extern const device_type NES_LZ93D50_24C01;
extern const device_type NES_LZ93D50_24C02;
extern const device_type NES_FJUMP2;

#endif
