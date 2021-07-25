// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_MULTIGAME_H
#define MAME_BUS_NES_MULTIGAME_H

#pragma once

#include "nxrom.h"


// ======================> nes_action52_device

class nes_action52_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_action52_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_caltron_device

class nes_caltron_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_caltron_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_m(offs_t offset, uint8_t data) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	uint8_t m_latch;
};


// ======================> nes_rumblestat_device

class nes_rumblestat_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_rumblestat_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_m(offs_t offset, uint8_t data) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	uint8_t m_prg, m_chr;
};


// ======================> nes_svision16_device

class nes_svision16_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_svision16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_m(offs_t offset) override;
	virtual void write_m(offs_t offset, uint8_t data) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	void update_prg();
	uint8_t m_latch1, m_latch2;
};


// ======================> nes_kn42_device

class nes_kn42_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_kn42_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	u8 m_latch;
};


// ======================> nes_n625092_device

class nes_n625092_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_n625092_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	void set_prg(uint8_t reg1, uint8_t reg2);
	uint8_t m_latch1, m_latch2;
};


// ======================> nes_a65as_device

class nes_a65as_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_a65as_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_t262_device

class nes_t262_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_t262_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	uint8_t m_latch1, m_latch2;
};


// ======================> nes_novel1_device

class nes_novel1_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_novel1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_novel2_device

class nes_novel2_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_novel2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_studyngame_device

class nes_studyngame_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_studyngame_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_sgun20in1_device

class nes_sgun20in1_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_sgun20in1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_vt5201_device

class nes_vt5201_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_vt5201_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_h(offs_t offset) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	uint8_t m_latch, m_dipsetting;
};


// ======================> nes_bmc_80013b_device

class nes_bmc_80013b_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_bmc_80013b_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	void update_prg();
	u8 m_latch, m_reg[2];
};


// ======================> nes_bmc_810544c_device

class nes_bmc_810544c_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_bmc_810544c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_ntd03_device

class nes_ntd03_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_ntd03_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_bmc_ctc09_device

class nes_bmc_ctc09_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_bmc_ctc09_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;
};


// ======================> nes_bmc_gb63_device

class nes_bmc_gb63_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_bmc_gb63_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_h(offs_t offset) override;
	virtual void write_h(offs_t offset, uint8_t data) override;
	virtual void chr_w(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	void update_banks();
	uint8_t m_latch, m_dipsetting;
	uint8_t m_reg[2];
	int m_vram_disable;
};


// ======================> nes_bmc_gka_device

class nes_bmc_gka_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_bmc_gka_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	uint8_t m_latch1, m_latch2;
};


// ======================> nes_bmc_gkb_device

class nes_bmc_gkb_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_bmc_gkb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_bmc_k3046_device

class nes_bmc_k3046_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_bmc_k3046_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;
};


// ======================> nes_bmc_sa005a_device

class nes_bmc_sa005a_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_bmc_sa005a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;
};


// ======================> nes_bmc_tj03_device

class nes_bmc_tj03_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_bmc_tj03_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;
};


// ======================> nes_bmc_ws_device

class nes_bmc_ws_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_bmc_ws_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_m(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	uint8_t m_latch;
};


// ======================> nes_bmc_11160_device

class nes_bmc_11160_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_bmc_11160_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_bmc_g146_device

class nes_bmc_g146_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_bmc_g146_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_bmc_8157_device

class nes_bmc_8157_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_bmc_8157_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_h(offs_t offset) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_bmc_hik300_device

class nes_bmc_hik300_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_bmc_hik300_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_bmc_s700_device

class nes_bmc_s700_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_bmc_s700_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_bmc_ball11_device

class nes_bmc_ball11_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_bmc_ball11_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_m(offs_t offset, uint8_t data) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	void set_banks();
	uint8_t m_reg[2];
};


// ======================> nes_bmc_22games_device

class nes_bmc_22games_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_bmc_22games_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_bmc_64y2k_device

class nes_bmc_64y2k_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_bmc_64y2k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_l(offs_t offset, uint8_t data) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	void set_prg();
	uint8_t m_reg[4];
};


// ======================> nes_bmc_12in1_device

class nes_bmc_12in1_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_bmc_12in1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	void update_banks();
	uint8_t m_reg[3];
};


// ======================> nes_bmc_20in1_device

class nes_bmc_20in1_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_bmc_20in1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_bmc_21in1_device

class nes_bmc_21in1_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_bmc_21in1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_bmc_31in1_device

class nes_bmc_31in1_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_bmc_31in1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_bmc_35in1_device

class nes_bmc_35in1_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_bmc_35in1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_bmc_36in1_device

class nes_bmc_36in1_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_bmc_36in1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_bmc_64in1_device

class nes_bmc_64in1_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_bmc_64in1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_bmc_70in1_device

class nes_bmc_70in1_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_bmc_70in1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_h(offs_t offset) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	uint8_t m_mode;
	uint8_t m_reg[2];
};


// ======================> nes_bmc_72in1_device

class nes_bmc_72in1_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_bmc_72in1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_bmc_76in1_device

class nes_bmc_76in1_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_bmc_76in1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	uint8_t m_reg[2];
};


// ======================> nes_bmc_110in1_device

class nes_bmc_110in1_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_bmc_110in1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_bmc_150in1_device

class nes_bmc_150in1_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_bmc_150in1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_bmc_190in1_device

class nes_bmc_190in1_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_bmc_190in1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_bmc_800in1_device

class nes_bmc_800in1_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_bmc_800in1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_h(offs_t offset) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	uint8_t m_mode;
	uint8_t m_reg[2];
};


// ======================> nes_bmc_1200in1_device

class nes_bmc_1200in1_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_bmc_1200in1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;
	virtual void chr_w(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	uint8_t m_vram_protect;
};


// ======================> nes_bmc_gold150_device

class nes_bmc_gold150_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_bmc_gold150_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_h(offs_t offset) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	uint8_t m_latch;
};


// ======================> nes_bmc_gold260_device

class nes_bmc_gold260_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_bmc_gold260_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_bmc_ch001_device

class nes_bmc_ch001_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_bmc_ch001_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_h(offs_t offset) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	uint8_t m_latch;
};


// ======================> nes_bmc_super22_device

class nes_bmc_super22_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_bmc_super22_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// ======================> nes_bmc_4in1reset_device

class nes_bmc_4in1reset_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_bmc_4in1reset_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	int m_latch;
};

// ======================> nes_bmc_42in1reset_device

class nes_bmc_42in1reset_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_bmc_42in1reset_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	int m_latch;
	uint8_t m_reg[2];
};


// device type definition
DECLARE_DEVICE_TYPE(NES_ACTION52,       nes_action52_device)
DECLARE_DEVICE_TYPE(NES_CALTRON6IN1,    nes_caltron_device)
DECLARE_DEVICE_TYPE(NES_RUMBLESTATION,  nes_rumblestat_device)
DECLARE_DEVICE_TYPE(NES_SVISION16,      nes_svision16_device)
DECLARE_DEVICE_TYPE(NES_KN42,           nes_kn42_device)
DECLARE_DEVICE_TYPE(NES_N625092,        nes_n625092_device)
DECLARE_DEVICE_TYPE(NES_A65AS,          nes_a65as_device)
DECLARE_DEVICE_TYPE(NES_T262,           nes_t262_device)
DECLARE_DEVICE_TYPE(NES_NOVEL1,         nes_novel1_device)
DECLARE_DEVICE_TYPE(NES_NOVEL2,         nes_novel2_device)
DECLARE_DEVICE_TYPE(NES_STUDYNGAME,     nes_studyngame_device)
DECLARE_DEVICE_TYPE(NES_SUPERGUN20IN1,  nes_sgun20in1_device)
DECLARE_DEVICE_TYPE(NES_VT5201,         nes_vt5201_device)
DECLARE_DEVICE_TYPE(NES_BMC_80013B,     nes_bmc_80013b_device)
DECLARE_DEVICE_TYPE(NES_BMC_810544C,    nes_bmc_810544c_device)
DECLARE_DEVICE_TYPE(NES_NTD03,          nes_ntd03_device)
DECLARE_DEVICE_TYPE(NES_BMC_CTC09,      nes_bmc_ctc09_device)
DECLARE_DEVICE_TYPE(NES_BMC_GB63,       nes_bmc_gb63_device)
DECLARE_DEVICE_TYPE(NES_BMC_GKA,        nes_bmc_gka_device)
DECLARE_DEVICE_TYPE(NES_BMC_GKB,        nes_bmc_gkb_device)
DECLARE_DEVICE_TYPE(NES_BMC_K3046,      nes_bmc_k3046_device)
DECLARE_DEVICE_TYPE(NES_BMC_SA005A,     nes_bmc_sa005a_device)
DECLARE_DEVICE_TYPE(NES_BMC_TJ03,       nes_bmc_tj03_device)
DECLARE_DEVICE_TYPE(NES_BMC_WS,         nes_bmc_ws_device)
DECLARE_DEVICE_TYPE(NES_BMC_11160,      nes_bmc_11160_device)
DECLARE_DEVICE_TYPE(NES_BMC_G146,       nes_bmc_g146_device)
DECLARE_DEVICE_TYPE(NES_BMC_8157,       nes_bmc_8157_device)
DECLARE_DEVICE_TYPE(NES_BMC_HIK300,     nes_bmc_hik300_device)
DECLARE_DEVICE_TYPE(NES_BMC_S700,       nes_bmc_s700_device)
DECLARE_DEVICE_TYPE(NES_BMC_BALL11,     nes_bmc_ball11_device)
DECLARE_DEVICE_TYPE(NES_BMC_22GAMES,    nes_bmc_22games_device)
DECLARE_DEVICE_TYPE(NES_BMC_64Y2K,      nes_bmc_64y2k_device)
DECLARE_DEVICE_TYPE(NES_BMC_12IN1,      nes_bmc_12in1_device)
DECLARE_DEVICE_TYPE(NES_BMC_20IN1,      nes_bmc_20in1_device)
DECLARE_DEVICE_TYPE(NES_BMC_21IN1,      nes_bmc_21in1_device)
DECLARE_DEVICE_TYPE(NES_BMC_31IN1,      nes_bmc_31in1_device)
DECLARE_DEVICE_TYPE(NES_BMC_35IN1,      nes_bmc_35in1_device)
DECLARE_DEVICE_TYPE(NES_BMC_36IN1,      nes_bmc_36in1_device)
DECLARE_DEVICE_TYPE(NES_BMC_64IN1,      nes_bmc_64in1_device)
DECLARE_DEVICE_TYPE(NES_BMC_70IN1,      nes_bmc_70in1_device)
DECLARE_DEVICE_TYPE(NES_BMC_72IN1,      nes_bmc_72in1_device)
DECLARE_DEVICE_TYPE(NES_BMC_76IN1,      nes_bmc_76in1_device)
DECLARE_DEVICE_TYPE(NES_BMC_110IN1,     nes_bmc_110in1_device)
DECLARE_DEVICE_TYPE(NES_BMC_150IN1,     nes_bmc_150in1_device)
DECLARE_DEVICE_TYPE(NES_BMC_190IN1,     nes_bmc_190in1_device)
DECLARE_DEVICE_TYPE(NES_BMC_800IN1,     nes_bmc_800in1_device)
DECLARE_DEVICE_TYPE(NES_BMC_1200IN1,    nes_bmc_1200in1_device)
DECLARE_DEVICE_TYPE(NES_BMC_GOLD150,    nes_bmc_gold150_device)
DECLARE_DEVICE_TYPE(NES_BMC_GOLD260,    nes_bmc_gold260_device)
DECLARE_DEVICE_TYPE(NES_BMC_CH001,      nes_bmc_ch001_device)
DECLARE_DEVICE_TYPE(NES_BMC_SUPER22,    nes_bmc_super22_device)
DECLARE_DEVICE_TYPE(NES_BMC_4IN1RESET,  nes_bmc_4in1reset_device)
DECLARE_DEVICE_TYPE(NES_BMC_42IN1RESET, nes_bmc_42in1reset_device)

#endif // MAME_BUS_NES_MULTIGAME_H
