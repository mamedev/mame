// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __NES_JALECO_H
#define __NES_JALECO_H

#include "nxrom.h"
#include "sound/samples.h"


// ======================> nes_jf11_device

class nes_jf11_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_jf11_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_m);

	virtual void pcb_reset() override;
};


// ======================> nes_jf13_device

class nes_jf13_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_jf13_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual DECLARE_WRITE8_MEMBER(write_m);

	virtual void pcb_reset() override;

private:
	required_device<samples_device> m_samples;
};


// ======================> nes_jf16_device

class nes_jf16_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_jf16_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_h);

	virtual void pcb_reset() override;
};


// ======================> nes_jf17_device

class nes_jf17_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_jf17_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	nes_jf17_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_h);

	virtual void pcb_reset() override;

protected:
	UINT8 m_latch;
};


// ======================> nes_jf17_adpcm_device

class nes_jf17_adpcm_device : public nes_jf17_device
{
public:
	// construction/destruction
	nes_jf17_adpcm_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual DECLARE_WRITE8_MEMBER(write_h);

private:
	required_device<samples_device> m_samples;
};


// ======================> nes_jf19_device

class nes_jf19_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_jf19_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	nes_jf19_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_h);

	virtual void pcb_reset() override;
};


// ======================> nes_jf19_adpcm_device

class nes_jf19_adpcm_device : public nes_jf19_device
{
public:
	// construction/destruction
	nes_jf19_adpcm_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual DECLARE_WRITE8_MEMBER(write_h);

private:
	required_device<samples_device> m_samples;
};


// ======================> nes_ss88006_device

class nes_ss88006_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ss88006_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	nes_ss88006_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual DECLARE_WRITE8_MEMBER(ss88006_write);
	virtual DECLARE_WRITE8_MEMBER(write_h) override { ss88006_write(space, offset, data, mem_mask); }

	virtual void pcb_reset() override;

protected:
	UINT16 m_irq_count, m_irq_count_latch;
	UINT8 m_irq_mode;
	int m_irq_enable;

	static const device_timer_id TIMER_IRQ = 0;
	emu_timer *irq_timer;

	UINT8 m_mmc_prg_bank[3];
	UINT8 m_mmc_vrom_bank[8];

	UINT8 m_latch; // used for samples, in derived classes
};


// ======================> nes_ss88006_adpcm_device

class nes_ss88006_adpcm_device : public nes_ss88006_device
{
public:
	// construction/destruction
	nes_ss88006_adpcm_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	// device-level overrides
	void ss88006_adpcm_write(address_space &space, offs_t offset, UINT8 data, samples_device *dev);
};


// ======================> nes_jf23_device

class nes_jf23_device : public nes_ss88006_adpcm_device
{
public:
	// construction/destruction
	nes_jf23_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual machine_config_constructor device_mconfig_additions() const override;

private:
	required_device<samples_device> m_samples;
	virtual DECLARE_WRITE8_MEMBER(write_h) override { ss88006_adpcm_write(space, offset, data, m_samples); }
};


// ======================> nes_jf24_device

class nes_jf24_device : public nes_ss88006_adpcm_device
{
public:
	// construction/destruction
	nes_jf24_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual machine_config_constructor device_mconfig_additions() const override;

private:
	required_device<samples_device> m_samples;
	virtual DECLARE_WRITE8_MEMBER(write_h) override { ss88006_adpcm_write(space, offset, data, m_samples); }
};


// ======================> nes_jf29_device

class nes_jf29_device : public nes_ss88006_adpcm_device
{
public:
	// construction/destruction
	nes_jf29_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual machine_config_constructor device_mconfig_additions() const override;

private:
	required_device<samples_device> m_samples;
	virtual DECLARE_WRITE8_MEMBER(write_h) override { ss88006_adpcm_write(space, offset, data, m_samples); }
};


// ======================> nes_jf33_device

class nes_jf33_device : public nes_ss88006_adpcm_device
{
public:
	// construction/destruction
	nes_jf33_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual machine_config_constructor device_mconfig_additions() const override;

private:
	required_device<samples_device> m_samples;
	virtual DECLARE_WRITE8_MEMBER(write_h) override { ss88006_adpcm_write(space, offset, data, m_samples); }
};




// device type definition
extern const device_type NES_JF11;
extern const device_type NES_JF13;
extern const device_type NES_JF16;
extern const device_type NES_JF17;
extern const device_type NES_JF17_ADPCM;
extern const device_type NES_JF19;
extern const device_type NES_JF19_ADPCM;
extern const device_type NES_SS88006;
extern const device_type NES_JF23;
extern const device_type NES_JF24;
extern const device_type NES_JF29;
extern const device_type NES_JF33;

#endif
