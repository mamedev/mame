// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_PIRATE_H
#define MAME_BUS_NES_PIRATE_H

#pragma once

#include "nxrom.h"


// ======================> nes_agci_device

class nes_agci_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_agci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_dreamtech_device

class nes_dreamtech_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_dreamtech_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_l(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_fukutake_device

class nes_fukutake_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_fukutake_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_l(offs_t offset) override;
	virtual uint8_t read_m(offs_t offset) override;
	virtual void write_l(offs_t offset, uint8_t data) override;
	virtual void write_m(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	uint8_t m_latch;
	uint8_t m_ram[0xb00];
};


// ======================> nes_futuremedia_device

class nes_futuremedia_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_futuremedia_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void hblank_irq(int scanline, int vblank, int blanked) override;
	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	u8 m_irq_count, m_irq_count_latch;
	int m_irq_enable;
};


// ======================> nes_magseries_device

class nes_magseries_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_magseries_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;
};


// ======================> nes_daou306_device

class nes_daou306_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_daou306_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	uint8_t m_reg[16];
};


// ======================> nes_cc21_device

class nes_cc21_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_cc21_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;
};


// ======================> nes_xiaozy_device

class nes_xiaozy_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_xiaozy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_l(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_edu2k_device

class nes_edu2k_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_edu2k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_m(offs_t offset) override;
	virtual void write_m(offs_t offset, uint8_t data) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	uint8_t m_latch;
};


// ======================> nes_jy830623c_device

class nes_jy830623c_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_jy830623c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_m(offs_t offset, u8 data) override;
	virtual void write_h(offs_t offset, u8 data) override;

	virtual void hblank_irq(int scanline, int vblank, int blanked) override;
	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	void update_banks();
	u8 m_latch;
	u8 m_reg[6];

	u16 m_irq_count, m_irq_count_latch;
	int m_irq_enable;
};


// ======================> nes_43272_device

class nes_43272_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_43272_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_h(offs_t offset) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	uint16_t m_latch;
};


// ======================> nes_eh8813a_device

class nes_eh8813a_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_eh8813a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_h(offs_t offset) override;
	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	u8 m_jumper;
	u16 m_latch;
	u8 m_reg;
};



#ifdef UNUSED_FUNCTION
// ======================> nes_fujiya_device

class nes_fujiya_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_fujiya_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_m(offs_t offset) override;
	virtual void write_m(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	uint8_t m_latch;
};
#endif


// device type definition
DECLARE_DEVICE_TYPE(NES_AGCI_50282,  nes_agci_device)
DECLARE_DEVICE_TYPE(NES_DREAMTECH01, nes_dreamtech_device)
DECLARE_DEVICE_TYPE(NES_FUKUTAKE,    nes_fukutake_device)
DECLARE_DEVICE_TYPE(NES_FUTUREMEDIA, nes_futuremedia_device)
DECLARE_DEVICE_TYPE(NES_MAGSERIES,   nes_magseries_device)
DECLARE_DEVICE_TYPE(NES_DAOU306,     nes_daou306_device)
DECLARE_DEVICE_TYPE(NES_CC21,        nes_cc21_device)
DECLARE_DEVICE_TYPE(NES_XIAOZY,      nes_xiaozy_device)
DECLARE_DEVICE_TYPE(NES_EDU2K,       nes_edu2k_device)
DECLARE_DEVICE_TYPE(NES_JY830623C,   nes_jy830623c_device)
DECLARE_DEVICE_TYPE(NES_43272,       nes_43272_device)
DECLARE_DEVICE_TYPE(NES_EH8813A,     nes_eh8813a_device)
//DECLARE_DEVICE_TYPE(NES_FUJIYA,      nes_fujiya_device)

#endif // MAME_BUS_NES_PIRATE_H
