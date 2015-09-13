// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __NES_PIR_H
#define __NES_PIR_H

#include "nxrom.h"


// ======================> nes_agci_device

class nes_agci_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_agci_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual DECLARE_WRITE8_MEMBER(write_h);

	virtual void pcb_reset();
};


// ======================> nes_dreamtech_device

class nes_dreamtech_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_dreamtech_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual DECLARE_WRITE8_MEMBER(write_l);

	virtual void pcb_reset();
};


// ======================> nes_fukutake_device

class nes_fukutake_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_fukutake_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual DECLARE_READ8_MEMBER(read_l);
	virtual DECLARE_READ8_MEMBER(read_m);
	virtual DECLARE_WRITE8_MEMBER(write_l);
	virtual DECLARE_WRITE8_MEMBER(write_m);

	virtual void pcb_reset();

private:
	UINT8 m_latch;
	UINT8 m_ram[0xb00];
};


// ======================> nes_futuremedia_device

class nes_futuremedia_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_futuremedia_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual DECLARE_WRITE8_MEMBER(write_h);

	virtual void hblank_irq(int scanline, int vblank, int blanked);
	virtual void pcb_reset();

private:
	UINT16 m_irq_count, m_irq_count_latch;
	UINT8 m_irq_clear;
	int m_irq_enable;
};


// ======================> nes_magseries_device

class nes_magseries_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_magseries_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual DECLARE_WRITE8_MEMBER(write_h);

	virtual void pcb_reset();
};


// ======================> nes_daou306_device

class nes_daou306_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_daou306_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual DECLARE_WRITE8_MEMBER(write_h);

	virtual void pcb_reset();

private:
	UINT8 m_reg[16];
};


// ======================> nes_subor0_device

class nes_subor0_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_subor0_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual DECLARE_WRITE8_MEMBER(write_h);

	virtual void pcb_reset();

private:
	UINT8 m_reg[4];
};


// ======================> nes_subor1_device

class nes_subor1_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_subor1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual DECLARE_WRITE8_MEMBER(write_h);

	virtual void pcb_reset();

private:
	UINT8 m_reg[4];
};


// ======================> nes_cc21_device

class nes_cc21_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_cc21_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual DECLARE_WRITE8_MEMBER(write_h);

	virtual void pcb_reset();
};


// ======================> nes_xiaozy_device

class nes_xiaozy_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_xiaozy_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual DECLARE_WRITE8_MEMBER(write_l);

	virtual void pcb_reset();
};


// ======================> nes_edu2k_device

class nes_edu2k_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_edu2k_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual DECLARE_READ8_MEMBER(read_m);
	virtual DECLARE_WRITE8_MEMBER(write_m);
	virtual DECLARE_WRITE8_MEMBER(write_h);

	virtual void pcb_reset();

private:
	UINT8 m_latch;
};


// ======================> nes_t230_device

class nes_t230_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_t230_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual DECLARE_WRITE8_MEMBER(write_h);

	virtual void hblank_irq(int scanline, int vblank, int blanked);
	virtual void pcb_reset();

private:
	UINT16 m_irq_count, m_irq_count_latch;
	UINT8 m_irq_mode;
	int m_irq_enable, m_irq_enable_latch;

	UINT8 m_mmc_vrom_bank[8];
};


// ======================> nes_mk2_device

class nes_mk2_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_mk2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual DECLARE_WRITE8_MEMBER(write_m);

	virtual void hblank_irq(int scanline, int vblank, int blanked);
	virtual void pcb_reset();

private:
	UINT16 m_irq_count, m_irq_count_latch;
	UINT8 m_irq_clear;
	int m_irq_enable;
};


// ======================> nes_whero_device

class nes_whero_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_whero_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual DECLARE_WRITE8_MEMBER(write_h);

	virtual void hblank_irq(int scanline, int vblank, int blanked);
	virtual void pcb_reset();

private:
	UINT8 m_reg;
	UINT8 m_mmc_vrom_bank[8];

	UINT16 m_irq_count, m_irq_count_latch;
	int m_irq_enable, m_irq_enable_latch;
//  int m_irq_mode;
};


// ======================> nes_43272_device

class nes_43272_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_43272_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual DECLARE_READ8_MEMBER(read_h);
	virtual DECLARE_WRITE8_MEMBER(write_h);

	virtual void pcb_reset();

private:
	UINT16 m_latch;
};


// ======================> nes_tf1201_device

class nes_tf1201_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_tf1201_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual DECLARE_WRITE8_MEMBER(write_h);

	virtual void hblank_irq(int scanline, int vblank, int blanked);
	virtual void pcb_reset();

private:
	void update_prg();
	UINT8 m_prg, m_swap;
	UINT16 m_irq_count;
	int m_irq_enable, m_irq_enable_latch;

	UINT8 m_mmc_vrom_bank[8];
};


// ======================> nes_cityfight_device

class nes_cityfight_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_cityfight_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	virtual DECLARE_WRITE8_MEMBER(write_h);

	virtual void pcb_reset();

private:
	void update_prg();
	UINT8 m_prg_reg, m_prg_mode;
	UINT16 m_irq_count;
	int m_irq_enable;

	static const device_timer_id TIMER_IRQ = 0;
	emu_timer *irq_timer;

	UINT8 m_mmc_vrom_bank[8];
};



#ifdef UNUSED_FUNCTION
// ======================> nes_fujiya_device

class nes_fujiya_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_fujiya_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual DECLARE_WRITE8_MEMBER(read_m);
	virtual DECLARE_WRITE8_MEMBER(write_m);

	virtual void pcb_reset();

private:
	UINT8 m_latch;
};
#endif


// device type definition
extern const device_type NES_AGCI_50282;
extern const device_type NES_DREAMTECH01;
extern const device_type NES_FUKUTAKE;
extern const device_type NES_FUTUREMEDIA;
extern const device_type NES_MAGSERIES;
extern const device_type NES_DAOU306;
extern const device_type NES_SUBOR0;
extern const device_type NES_SUBOR1;
extern const device_type NES_CC21;
extern const device_type NES_XIAOZY;
extern const device_type NES_EDU2K;
extern const device_type NES_T230;
extern const device_type NES_MK2;
extern const device_type NES_WHERO;
extern const device_type NES_43272;
extern const device_type NES_TF1201;
extern const device_type NES_CITYFIGHT;
extern const device_type NES_FUJIYA;

#endif
