#ifndef __NES_KONAMI_H
#define __NES_KONAMI_H

#include "machine/nes_nxrom.h"


// ======================> nes_konami_vrc1_device

class nes_konami_vrc1_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_konami_vrc1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual DECLARE_WRITE8_MEMBER(write_h);

	virtual void pcb_reset();

private:
	UINT8 m_mmc_vrom_bank[2];
};


// ======================> nes_konami_vrc2_device

class nes_konami_vrc2_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_konami_vrc2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual DECLARE_READ8_MEMBER(read_m);
	virtual DECLARE_WRITE8_MEMBER(write_m);
	virtual DECLARE_WRITE8_MEMBER(write_h);

	virtual void pcb_reset();

private:
	UINT8 m_mmc_vrom_bank[8];
	UINT8 m_latch;
};


// ======================> nes_konami_vrc3_device

class nes_konami_vrc3_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_konami_vrc3_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	nes_konami_vrc3_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual DECLARE_WRITE8_MEMBER(write_h);

	virtual void hblank_irq(int scanline, int vblank, int blanked);
	virtual void pcb_reset();

protected:
	UINT16 m_irq_count, m_irq_count_latch;
	int m_irq_enable, m_irq_enable_latch;
	int m_irq_mode;   // used by VRC-4, VRC-6 & VRC-7. currently not implemented: 0 = prescaler mode / 1 = CPU mode
};


// ======================> nes_konami_vrc4_device

class nes_konami_vrc4_device : public nes_konami_vrc3_device
{
public:
	// construction/destruction
	nes_konami_vrc4_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual DECLARE_WRITE8_MEMBER(write_h);

	virtual void pcb_reset();

private:
	void set_prg();
	UINT8 m_mmc_vrom_bank[8];
	UINT8 m_latch, m_mmc_prg_bank;
};


// ======================> nes_konami_vrc6_device

class nes_konami_vrc6_device : public nes_konami_vrc3_device
{
public:
	// construction/destruction
	nes_konami_vrc6_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual DECLARE_WRITE8_MEMBER(write_h);

	// TODO: emulate sound capabilities!
};


// ======================> nes_konami_vrc7_device

class nes_konami_vrc7_device : public nes_konami_vrc3_device
{
public:
	// construction/destruction
	nes_konami_vrc7_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual DECLARE_WRITE8_MEMBER(write_h);

	virtual void pcb_reset();

private:
	device_t *m_ym2413;
};



// device type definition
extern const device_type NES_VRC1;
extern const device_type NES_VRC2;
extern const device_type NES_VRC3;
extern const device_type NES_VRC4;
extern const device_type NES_VRC6;
extern const device_type NES_VRC7;

#endif
