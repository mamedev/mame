// license:BSD-3-Clause
// copyright-holders:kmg,Brian H. Gilbert
#ifndef MAME_BUS_NES_VRC_CLONES_H
#define MAME_BUS_NES_VRC_CLONES_H

#pragma once

#include "konami.h"

#include "sound/dac.h"


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
	// M2 at the cartridge connector: the IRQ counter is clocked once per CPU cycle, at the M2 rise (before that cycle's access)
	virtual bool wants_m2() override { return true; }
	virtual void m2_w(int state, offs_t address, bool write) override { if (state) irq_timer_tick(0); }

private:
	u16 m_irq_count;
	int m_irq_enable;

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
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<dac_4bit_r2r_device> m_dac;
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
	// M2 at the cartridge connector: the IRQ counter is clocked once per CPU cycle, at the M2 rise (before that cycle's access)
	virtual bool wants_m2() override { return true; }
	virtual void m2_w(int state, offs_t address, bool write) override { if (state) irq_timer_tick(0); }

private:
	u16 m_irq_count;
	u8 m_irq_latch;
	int m_irq_enable;

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


// ======================> nes_waixing_fs306_device

class nes_waixing_fs306_device : public nes_konami_vrc4_device
{
public:
	// construction/destruction
	nes_waixing_fs306_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;
	virtual void chr_w(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	virtual void set_prg() override;
	virtual void set_chr() override;

	// the IRQ reaches the cartridge edge one M2 cycle after the VRC4 core raises it
	virtual void m2_w(int state, offs_t address, bool write) override;
	virtual void vrc_irq_w(int state) override;

private:
	bool gal_chr_is_ram(u16 bank) const;
	void update_ciram();

	u8 m_prg2;              // $9C00
	u8 m_ciram[4];          // $9C0C-$9C0F
	u8 m_gal;               // CHR-RAM configuration
	u8 m_irq_delay;         // the VRC4 core raised /IRQ at the last M2 rise; it reaches the edge at the next one
};


// ======================> nes_jy_yy840820c_device

class nes_jy_yy840820c_device : public nes_konami_vrc4_device
{
public:
	// construction/destruction
	nes_jy_yy840820c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;
	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	virtual void set_prg() override;
	virtual void set_chr() override;

private:
	u8 m_outer;             // the low byte of the last $8000-$FFFF write address
};


// ======================> nes_tmnt2_563_device

class nes_tmnt2_563_device : public nes_konami_vrc2_device
{
public:
	// construction/destruction
	nes_tmnt2_563_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;
	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	virtual bool wants_a12_level() override { return true; }
	virtual void ppu_a12_level(int state) override;

private:
	void update_banks();
	u8 m_prg[2];            // VRC2 $8000 and $A000
	u8 m_outer;             // $F018
	u8 m_irq_enable;
	u16 m_irq_count;        // counts falling edges of PPU A12
	int m_a12;
};


// ======================> nes_sango2_559_device

class nes_sango2_559_device : public nes_konami_vrc4_device
{
public:
	// construction/destruction
	nes_sango2_559_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;
	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	virtual void set_prg() override;

private:
	void update_ciram();
	u8 m_prg2;              // the third PRG bank, from the external select
	u8 m_nt[4];             // CIRAM page of each nametable, from the external select
};


// ======================> nes_akumajo_272_device

class nes_akumajo_272_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_akumajo_272_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

	virtual bool wants_ppu_addr_bus() override { return true; }
	virtual void ppu_addr_bus(u16 addr) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	void update_banks();

	u8 m_prg[2];            // the two switchable 8 KiB PRG banks
	u8 m_chr[8];            // 1 KiB CHR banks, written a nibble at a time
	u8 m_mirr;              // VRC mirroring ($9000)
	u8 m_pal_mirr;          // PAL mirroring ($8004), 2 and 3 select one-screen
	u8 m_irq_enable;
	u8 m_irq_count;
	u8 m_last_a13;
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
DECLARE_DEVICE_TYPE(NES_WAIXING_FS306, nes_waixing_fs306_device)
DECLARE_DEVICE_TYPE(NES_TMNT2_563, nes_tmnt2_563_device)
DECLARE_DEVICE_TYPE(NES_SANGO2_559, nes_sango2_559_device)
DECLARE_DEVICE_TYPE(NES_AKUMAJO_272, nes_akumajo_272_device)
DECLARE_DEVICE_TYPE(NES_JY_YY840820C, nes_jy_yy840820c_device)

#endif // MAME_BUS_NES_VRC_CLONES_H
