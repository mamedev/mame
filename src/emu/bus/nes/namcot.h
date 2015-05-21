// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __NES_NAMCOT_H
#define __NES_NAMCOT_H

#include "nxrom.h"


// ======================> nes_namcot3433_device

class nes_namcot3433_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_namcot3433_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	nes_namcot3433_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual DECLARE_WRITE8_MEMBER(dxrom_write);
	virtual DECLARE_WRITE8_MEMBER(write_h) { dxrom_write(space, offset, data, mem_mask); }

	virtual void pcb_reset();

private:
	UINT8 m_latch;
};


// ======================> nes_namcot3446_device

class nes_namcot3446_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_namcot3446_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual DECLARE_WRITE8_MEMBER(write_h);

	virtual void pcb_reset();

private:
	UINT8 m_latch;
};


// ======================> nes_namcot3425_device

class nes_namcot3425_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_namcot3425_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual DECLARE_WRITE8_MEMBER(write_h);

	virtual void pcb_reset();

private:
	UINT8 m_latch;
	UINT8 m_reg[4];
};


// ======================> nes_namcot340_device

class nes_namcot340_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_namcot340_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	nes_namcot340_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	virtual DECLARE_READ8_MEMBER(n340_loread);
	virtual DECLARE_WRITE8_MEMBER(n340_lowrite);
	virtual DECLARE_WRITE8_MEMBER(n340_hiwrite);
	virtual DECLARE_READ8_MEMBER(read_l) { return n340_loread(space, offset, mem_mask); }
	virtual DECLARE_WRITE8_MEMBER(write_l) { n340_lowrite(space, offset, data, mem_mask); }
	virtual DECLARE_WRITE8_MEMBER(write_h) { n340_hiwrite(space, offset, data, mem_mask); }

	virtual void pcb_reset();

protected:
	UINT16 m_irq_count;
	int m_irq_enable;

	static const device_timer_id TIMER_IRQ = 0;
	emu_timer *irq_timer;

	// Namcot-163 chip contains 8K of internal ram, possibly battery backed up (not emulated yet)
	// was it also present in 175 & 340 chips?
	UINT8 m_n163_ram[0x2000];
};


// ======================> nes_namcot175_device

class nes_namcot175_device : public nes_namcot340_device
{
public:
	// construction/destruction
	nes_namcot175_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual DECLARE_READ8_MEMBER(read_m);
	virtual DECLARE_WRITE8_MEMBER(write_m);
	virtual DECLARE_WRITE8_MEMBER(write_h);

	virtual void pcb_reset();

private:
	UINT8 m_wram_protect;
};


// ======================> nes_namcot163_device

class nes_namcot163_device : public nes_namcot340_device
{
public:
	// construction/destruction
	nes_namcot163_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual DECLARE_READ8_MEMBER(read_l);
	virtual DECLARE_READ8_MEMBER(read_m);
	virtual DECLARE_WRITE8_MEMBER(write_l);
	virtual DECLARE_WRITE8_MEMBER(write_m);
	virtual DECLARE_WRITE8_MEMBER(write_h);

	// we have to overwrite these to allow CIRAM to be used for VRAM, even if it's not clear which game(s) use this
	virtual DECLARE_READ8_MEMBER(chr_r);
	virtual DECLARE_WRITE8_MEMBER(chr_w);

	virtual void pcb_reset();

private:
	void set_mirror(UINT8 page, UINT8 data);

	UINT8 m_wram_protect, m_latch, m_chr_bank;
	// TODO: add emulation of the sound part of the chip
};



// device type definition
extern const device_type NES_NAMCOT3433;
extern const device_type NES_NAMCOT3446;
extern const device_type NES_NAMCOT3425;
extern const device_type NES_NAMCOT175;
extern const device_type NES_NAMCOT340;
extern const device_type NES_NAMCOT163;

#endif
