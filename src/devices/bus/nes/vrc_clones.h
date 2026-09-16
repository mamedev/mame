// license:BSD-3-Clause
// copyright-holders:kmg
#ifndef MAME_BUS_NES_VRC_CLONES_H
#define MAME_BUS_NES_VRC_CLONES_H

#pragma once

#include "konami.h"


// ======================> nes_2yudb_device

class nes_2yudb_device : public nes_konami_vrc4_device
{
public:
	// construction/destruction
	nes_2yudb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	virtual void set_prg() override { nes_konami_vrc4_device::set_prg(m_outer, 0x1f); }

private:
	u8 m_outer;
};


// ======================> nes_900218_device

class nes_900218_device : public nes_konami_vrc2_device
{
public:
	// construction/destruction
	nes_900218_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(irq_timer_tick);

private:
	u16 m_irq_count;
	int m_irq_enable;

	emu_timer *irq_timer;
};


// ======================> nes_ax40g_device

class nes_ax40g_device : public nes_konami_vrc2_device
{
public:
	// construction/destruction
	nes_ax40g_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
};


// ======================> nes_ax5705_device

class nes_ax5705_device : public nes_konami_vrc4_device
{
public:
	// construction/destruction
	nes_ax5705_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
};


// ======================> nes_bmc_830506c_device

class nes_bmc_830506c_device : public nes_konami_vrc4_device
{
public:
	// construction/destruction
	nes_bmc_830506c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	virtual void irq_ack_w() override;
	virtual void set_prg() override { nes_konami_vrc4_device::set_prg(m_outer, 0x0f); }

private:
	u8 m_outer;
};


// ======================> nes_bmc_831128c_device

class nes_bmc_831128c_device : public nes_konami_vrc4_device
{
public:
	// construction/destruction
	nes_bmc_831128c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_m(offs_t offset) override;
	virtual void write_m(offs_t offset, u8 data) override;
	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	u8 m_reg;
};


// ======================> nes_bmc_kl06_device

class nes_bmc_kl06_device : public nes_konami_vrc4_device
{
public:
	// construction/destruction
	nes_bmc_kl06_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_m(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	virtual void set_prg() override;
	virtual void set_chr() override { nes_konami_vrc4_device::set_chr((m_reg & 0x03) << 7, 0x7f); }

private:
	u8 m_reg;
};


// ======================> nes_cityfight_device

class nes_cityfight_device : public nes_konami_vrc4_device
{
public:
	// construction/destruction
	nes_cityfight_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
};


// ======================> nes_shuiguan_device

class nes_shuiguan_device : public nes_konami_vrc4_device
{
public:
	// construction/destruction
	nes_shuiguan_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_m(offs_t offset) override;
	virtual void write_m(offs_t offset, u8 data) override;
	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	u8 m_reg;
};


// ======================> nes_t230_device

class nes_t230_device : public nes_konami_vrc4_device
{
public:
	// construction/destruction
	nes_t230_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
};


// ======================> nes_tf1201_device

class nes_tf1201_device : public nes_konami_vrc4_device
{
public:
	// construction/destruction
	nes_tf1201_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	virtual void irq_ack_w() override;
};


// ======================> nes_th21311_device

class nes_th21311_device : public nes_konami_vrc2_device
{
public:
	// construction/destruction
	nes_th21311_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(irq_timer_tick);

private:
	u16 m_irq_count;
	u8 m_irq_latch;
	int m_irq_enable;

	emu_timer *irq_timer;
};


// ======================> nes_waixing_sgz_device

class nes_waixing_sgz_device : public nes_konami_vrc4_device
{
public:
	// construction/destruction
	nes_waixing_sgz_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;
	virtual void chr_w(offs_t offset, u8 data) override;

protected:
	// construction/destruction
	nes_waixing_sgz_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 chr_match);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	u8 m_chr_mask, m_chr_match;
};

// ======================> nes_hengg_shjy3_device

class nes_hengg_shjy3_device : public nes_waixing_sgz_device
{
public:
	// construction/destruction
	nes_hengg_shjy3_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


// device type definition
DECLARE_DEVICE_TYPE(NES_2YUDB,       nes_2yudb_device)
DECLARE_DEVICE_TYPE(NES_900218,      nes_900218_device)
DECLARE_DEVICE_TYPE(NES_AX40G,       nes_ax40g_device)
DECLARE_DEVICE_TYPE(NES_AX5705,      nes_ax5705_device)
DECLARE_DEVICE_TYPE(NES_BMC_830506C, nes_bmc_830506c_device)
DECLARE_DEVICE_TYPE(NES_BMC_831128C, nes_bmc_831128c_device)
DECLARE_DEVICE_TYPE(NES_BMC_KL06,    nes_bmc_kl06_device)
DECLARE_DEVICE_TYPE(NES_CITYFIGHT,   nes_cityfight_device)
DECLARE_DEVICE_TYPE(NES_HENGG_SHJY3, nes_hengg_shjy3_device)
DECLARE_DEVICE_TYPE(NES_SHUIGUAN,    nes_shuiguan_device)
DECLARE_DEVICE_TYPE(NES_T230,        nes_t230_device)
DECLARE_DEVICE_TYPE(NES_TF1201,      nes_tf1201_device)
DECLARE_DEVICE_TYPE(NES_TH21311,     nes_th21311_device)
DECLARE_DEVICE_TYPE(NES_WAIXING_SGZ, nes_waixing_sgz_device)

#endif // MAME_BUS_NES_VRC_CLONES_H
