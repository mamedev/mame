// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Brian H. Gilbert
#ifndef MAME_BUS_NES_MMC3_CLONES_H
#define MAME_BUS_NES_MMC3_CLONES_H

#pragma once

#include "mmc3.h"
#include "sound/msm5205.h"


// ======================> nes_nitra_device

class nes_nitra_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_nitra_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;
	virtual void prg_cb(int start, int bank) override;
};


// ======================> nes_bmw8544_device

class nes_bmw8544_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_bmw8544_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_m(offs_t offset) override;
	virtual void write_m(offs_t offset, u8 data) override;
	virtual void prg_cb(int start, int bank) override;
	virtual void chr_cb(int start, int bank, int source) override;
	virtual void cpu_write_latch(u8 data) override { m_bus_latch = data; }

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	u8 m_reg[3];
	u8 m_bus_latch;
};


// ======================> nes_fs6_device

class nes_fs6_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_fs6_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;
	virtual u8 read_m(offs_t offset) override;
	virtual void write_m(offs_t offset, u8 data) override;
	virtual void prg_cb(int start, int bank) override;
	virtual void set_fs6_submapper(u8 val) override { m_prg189 = (val == 3); }

	virtual void pcb_reset() override;

private:
	bool m_prg189;      // submapper 3: the 32 KiB PRG bank comes from a register at $6000-$7FFF, as on mapper 189
};


// ======================> nes_sbros11_device

class nes_sbros11_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_sbros11_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;
};


// ======================> nes_malisb_device

class nes_malisb_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_malisb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;
	virtual void prg_cb(int start, int bank) override;
	virtual void chr_cb(int start, int bank, int source) override;
};


// ======================> nes_family4646_device

class nes_family4646_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_family4646_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_h(offs_t offset) override;
	virtual void write_m(offs_t offset, u8 data) override;
	virtual void prg_cb(int start, int bank) override;
	virtual void chr_cb(int start, int bank, int source) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	optional_ioport m_config;
	u8 m_reg[4];
};


// ======================> nes_hengg412_device

class nes_hengg412_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_hengg412_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_m(offs_t offset, u8 data) override;
	virtual void prg_cb(int start, int bank) override;
	virtual void chr_cb(int start, int bank, int source) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	u8 m_reg[3];
};


// ======================> nes_pikay2k_device

class nes_pikay2k_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_pikay2k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_m(offs_t offset) override;
	virtual void write_m(offs_t offset, uint8_t data) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	uint8_t m_reg[2];
};


// ======================> nes_8237_device

class nes_8237_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_8237_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_l(offs_t offset, u8 data) override;
	virtual void write_h(offs_t offset, u8 data) override;
	virtual void prg_cb(int start, int bank) override;

	virtual void pcb_reset() override;

protected:
	// construction/destruction
	nes_8237_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 board);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	void update_banks();
	u8 m_reg[3];
	const u8 m_board;
};


// ======================> nes_8237a_device

class nes_8237a_device : public nes_8237_device
{
public:
	// construction/destruction
	nes_8237a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


// ======================> nes_158b_device

class nes_158b_device : public nes_8237_device
{
public:
	// construction/destruction
	nes_158b_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_l(offs_t offset) override;
	virtual void write_l(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	int m_prot;
};


// ======================> nes_kasing_device

class nes_kasing_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_kasing_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_m(offs_t offset) override;
	virtual void write_m(offs_t offset, u8 data) override;
	virtual void prg_cb(int start, int bank) override;

	virtual void pcb_reset() override;

protected:
	// construction/destruction
	nes_kasing_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	bool m_mapper115 = true;    // mapper 115 has PRG A18 in $6000 bit 6, decodes A1-A0 and has a solder pad at $6002; mapper 114 does not
	int m_irq_type = 0;     // 0 = MMC3C-like IRQ (mapper 115), 1 = MMC3A-like, a latch value of zero disables it (mapper 114)

private:
	optional_ioport m_pad;
	bool m_mmc3_mode;
};


// ======================> nes_sglionk_device

class nes_sglionk_device : public nes_kasing_device
{
public:
	// construction/destruction
	nes_sglionk_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

protected:
	// construction/destruction
	nes_sglionk_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 board);

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	const u8 m_board;
};


// ======================> nes_sgboog_device

class nes_sgboog_device : public nes_sglionk_device
{
public:
	// construction/destruction
	nes_sgboog_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


// ======================> nes_kay_device

class nes_kay_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_kay_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_l(offs_t offset) override;
	virtual void write_l(offs_t offset, u8 data) override;
	virtual void write_h(offs_t offset, u8 data) override;
	virtual void chr_cb(int start, int bank, int source) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	virtual void set_prg(int prg_base, int prg_mask) override;

private:
	void prot_sync();
	void update_outer();

	u8 m_ovr[3];        // PRG banks selected through the protection index: $E000, $C000, $A000
	u8 m_outer;         // outer bank register ($5180 bit 7)
	u8 m_prot_out;      // protection array value returned by reads of $5000-$5FFF
	u8 m_prot_index;    // last value written to $8003
	u8 m_prot_latch;    // last value written to $8001, with the lower six bits reversed
	u8 m_prot_lock;     // set when the selected index does not apply further $8001 writes
};


// ======================> nes_h2288_device

class nes_h2288_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_h2288_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_l(offs_t offset, u8 data) override;
	virtual void write_h(offs_t offset, u8 data) override;
	virtual void prg_cb(int start, int bank) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	bool m_mmc3_mode;
};


// ======================> nes_6035052_device

class nes_6035052_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_6035052_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_ex(offs_t offset) override;
	virtual void write_ex(offs_t offset, uint8_t data) override;
	virtual uint8_t read_l(offs_t offset) override { return read_ex(offset); }
	virtual uint8_t read_m(offs_t offset) override { return read_ex(offset); }
	virtual void write_l(offs_t offset, uint8_t data) override { write_ex(offset, data); }
	virtual void write_m(offs_t offset, uint8_t data) override { write_ex(offset, data); }

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	uint8_t m_prot;
};


// ======================> nes_txc_tw_device

class nes_txc_tw_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_txc_tw_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_l(offs_t offset, uint8_t data) override;
	virtual uint8_t read_m(offs_t offset) override { return get_open_bus(); }   // no WRAM: $6000-$7FFF is the PRG register
	virtual void write_m(offs_t offset, uint8_t data) override;
	virtual void prg_cb(int start, int bank) override;
};


// ======================> nes_kof97_device

class nes_kof97_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_kof97_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;
};


// ======================> nes_kof96_device

class nes_kof96_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_kof96_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_l(offs_t offset) override;
	virtual void write_l(offs_t offset, u8 data) override;
	virtual void prg_cb(int start, int bank) override;
	virtual void chr_cb(int start, int bank, int source) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	bool m_mmc3_mode;
};


// ======================> nes_sf3_device

class nes_sf3_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_sf3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_m(offs_t offset, uint8_t data) override;
	virtual void write_h(offs_t offset, uint8_t data) override;
	virtual void prg_cb(int start, int bank) override;
	virtual void set_sf3_submapper(u8 val) override { m_submapper = val; }

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	virtual void set_chr(uint8_t chr, int chr_base, int chr_mask) override;

private:
	u8 m_submapper;
	u8 m_outer;     // submapper 3: outer PRG A17 register at $6000-$7FFF
};


// ======================> nes_cocoma_device

class nes_cocoma_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_cocoma_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;
};


// ======================> nes_gouder_device

class nes_gouder_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_gouder_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_l(offs_t offset) override;
	virtual void write_l(offs_t offset, uint8_t data) override;
	virtual void write_m(offs_t offset, uint8_t data) override;
	virtual void write_h(offs_t offset, uint8_t data) override;
	virtual void prg_cb(int start, int bank) override;
	virtual void set_gouder_submapper(u8 val) override { m_submapper = val; }

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	void update_mirroring();

	uint8_t m_reg[6];   // 0-3: protection registers, 4: protection table index, 5: PRG/mirroring register
	u8 m_submapper;
};


// ======================> nes_sa9602b_device

class nes_sa9602b_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_sa9602b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;
	virtual void prg_cb(int start, int bank) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	uint8_t m_reg;
	int m_prg_chip;
};


// ======================> nes_sachen_shero_device

class nes_sachen_shero_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_sachen_shero_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_l(offs_t offset) override;
	virtual void write_l(offs_t offset, u8 data) override;
	virtual void chr_cb(int start, int bank, int source) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	required_ioport m_jumper;
	u8 m_reg;
};

// ======================> nes_sachen_zgdh_device

class nes_sachen_zgdh_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_sachen_zgdh_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_l(offs_t offset, u8 data) override;
	virtual u8 nt_r(offs_t offset) override;
	virtual void nt_w(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	virtual void set_chr(u8 chr, int chr_base, int chr_mask) override;

private:
	u8 m_reg;
};

// ======================> nes_a9746_device

class nes_a9746_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_a9746_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_l(offs_t offset, u8 data) override;
	virtual void write_h(offs_t offset, uint8_t data) override;
	virtual void prg_cb(int start, int bank) override;
	virtual void chr_cb(int start, int bank, int source) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	void update_banks(uint8_t value);
	void update_outer();
	uint8_t m_reg[4];   // 0: register selected by the last write to $8000/$8002, 1: unused, 2: CHR bank latch, 3: 128 KiB outer PRG/CHR bank
	uint8_t m_ext;      // $8002 bit 5: 0 = MMC3 mode, 1 = extended mode
};


// ======================> nes_a88s1_device

class nes_a88s1_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_a88s1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_l(offs_t offset) override;
	virtual void write_l(offs_t offset, u8 data) override;
	virtual void prg_cb(int start, int bank) override;
	virtual void chr_cb(int start, int bank, int source) override;
	virtual void set_a88s1_submapper(u8 val) override { m_submapper = val; }

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	void update_banks();

	u8 m_reg[3];    // 0-1: outer registers, 2: DIP switch value (advances on every reset)
	u8 m_submapper;
};


// ======================> nes_bmc_el86xc_device

class nes_bmc_el86xc_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_bmc_el86xc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_m(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;
};


// ======================> nes_fk23c_device

class nes_fk23c_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_fk23c_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_l(offs_t offset, uint8_t data) override;
	virtual void write_h(offs_t offset, uint8_t data) override;
	virtual void prg_cb(int start, int bank) override;
	virtual void chr_cb(int start, int bank, int source) override;

	virtual void pcb_reset() override;

protected:
	nes_fk23c_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	void fk23c_set_prg();
	void fk23c_set_chr();
	uint8_t m_reg[8];
	uint8_t m_mmc_cmd1;
};


// ======================> nes_fk23ca_device

class nes_fk23ca_device : public nes_fk23c_device
{
public:
	// construction/destruction
	nes_fk23ca_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void pcb_reset() override;
};


// ======================> nes_nt639_device

class nes_nt639_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_nt639_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_m(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;
};


// ======================> nes_resettxrom_device

class nes_resettxrom_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_resettxrom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void pcb_reset() override;
	virtual void set_resettxrom_submapper(u8 val) override { m_submapper = val; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	int m_count;
	u8 m_submapper = 0;
};


// ======================> nes_s24in1sc03_device

class nes_s24in1sc03_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_s24in1sc03_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_l(offs_t offset, uint8_t data) override;
	virtual void prg_cb(int start, int bank) override;
	virtual void chr_cb(int start, int bank, int source) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	uint8_t m_reg[3];
};


// ======================> nes_tech9in1_device

class nes_tech9in1_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_tech9in1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_h(offs_t offset) override;
	virtual void write_h(offs_t offset, u8 data) override;
	virtual void write_l(offs_t offset, u8 data) override;
	virtual void write_ex(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	virtual void set_prg(int prg_base, int prg_mask) override;
	virtual void set_chr(u8 chr, int chr_base, int chr_mask) override;
	virtual void prg_cb(int start, int bank) override;

	// the VRC4 IRQ counter is clocked by M2
	virtual bool wants_m2() override { return true; }
	virtual void m2_w(int state, offs_t address, bool write) override { if (state) vrc_irq_clock(); }

private:
	void update_outer();
	void update_all();
	void set_mirror(u8 data);

	void mmc1_write(offs_t offset, u8 data);
	void vrc_write(offs_t offset, u8 data);
	void vrc_irq_tick();
	void vrc_irq_clock();

	u8 m_reg[3];                // $5000, $5001, $5002
	u16 m_prg_map[4];           // 8 KiB bank (PRG-ROM followed by CHR-ROM) seen at each CPU window
	bool m_nrom_8k;
	bool m_chr_8k;

	u8 m_mmc1_reg[4];
	u8 m_mmc1_shift;
	u8 m_mmc1_count;

	u8 m_vrc_prg[2];
	u16 m_vrc_chr[8];
	u8 m_vrc_flip;
	u8 m_vrc_irq_count, m_vrc_irq_latch;
	int m_vrc_irq_enable, m_vrc_irq_enable_latch;
	int m_vrc_irq_mode;
	int m_vrc_irq_prescale;
};


// ======================> nes_bmc_5in1_device

class nes_bmc_5in1_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_bmc_5in1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_m(offs_t offset) override;
	virtual void write_m(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_ioport m_jumper;
};


// ======================> nes_bmc_8in1_device

class nes_bmc_8in1_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_bmc_8in1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;
};


// ======================> nes_bmc_15in1_device

class nes_bmc_15in1_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_bmc_15in1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_m(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_ioport m_jumper;
};


// ======================> nes_bmc_sbig7_device

class nes_bmc_sbig7_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_bmc_sbig7_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;
};


// ======================> nes_bmc_hik8_device

class nes_bmc_hik8_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_bmc_hik8_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_l(offs_t offset) override;
	virtual u8 read_h(offs_t offset) override;
	virtual void write_m(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// construction/destruction
	nes_bmc_hik8_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	u8 m_reg[4];
	u8 m_count;
	optional_ioport m_dip;      // DIP switch read at $5000-$5FFF
	optional_ioport m_menu;     // address line connected to PRG /CE

private:
	bool prg_ce_disabled();
};


// ======================> nes_bmc_jy208_device

class nes_bmc_jy208_device : public nes_bmc_hik8_device
{
public:
	// construction/destruction
	nes_bmc_jy208_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_m(offs_t offset, u8 data) override;

protected:
	virtual void set_chr(u8 chr, int chr_base, int chr_mask) override;
};


// ======================> nes_bmc_jy302_device

class nes_bmc_jy302_device : public nes_bmc_hik8_device
{
public:
	// construction/destruction
	nes_bmc_jy302_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void set_chr(u8 chr, int chr_base, int chr_mask) override;
};


// ======================> nes_bmc_kc885_device

class nes_bmc_kc885_device : public nes_bmc_hik8_device
{
public:
	// construction/destruction
	nes_bmc_kc885_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_h(offs_t offset) override;
	virtual void write_m(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_ioport m_jumper;
};


// ======================> nes_bmc_sfc12_device

class nes_bmc_sfc12_device : public nes_bmc_hik8_device
{
public:
	// construction/destruction
	nes_bmc_sfc12_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void set_chr(u8 chr, int chr_base, int chr_mask) override;
};


// ======================> nes_bmc_hik4_device

class nes_bmc_hik4_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_bmc_hik4_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_m(offs_t offset, u8 data) override;
	virtual void prg_cb(int start, int bank) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	bool m_mmc3_mode;
};


// ======================> nes_bmc_f15_device

class nes_bmc_f15_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_bmc_f15_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_m(offs_t offset, u8 data) override;
	virtual void prg_cb(int start, int bank) override;

	virtual void pcb_reset() override;
};


// ======================> nes_bmc_f600_device

class nes_bmc_f600_device : public nes_txsrom_device
{
public:
	// construction/destruction
	nes_bmc_f600_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_l(offs_t offset) override;
	virtual void write_l(offs_t offset, u8 data) override;
	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	virtual void set_chr(u8 chr, int chr_base, int chr_mask) override;

private:
	required_ioport m_jumper;
	u8 m_reg;
};


// ======================> nes_bmc_gn45_device

class nes_bmc_gn45_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_bmc_gn45_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_m(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	bool m_lock;
};


// ======================> nes_bmc_gold7in1_device

class nes_bmc_gold7in1_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_bmc_gold7in1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_m(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	bool m_lock;
};


// ======================> nes_bmc_k3006_device

class nes_bmc_k3006_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_bmc_k3006_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_m(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;
};


// ======================> nes_bmc_k3033_device

class nes_bmc_k3033_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_bmc_k3033_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_m(offs_t offset, u8 data) override;
	virtual void prg_cb(int start, int bank) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	bool m_mmc3_mode;
};


// ======================> nes_bmc_l6in1_device

class nes_bmc_l6in1_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_bmc_l6in1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_m(offs_t offset, u8 data) override;
	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	virtual void set_prg(int prg_base, int prg_mask) override;

private:
	void set_mirror();
	u8 m_reg;
};


// ======================> nes_bmc_00202650_device

class nes_bmc_00202650_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_bmc_00202650_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_m(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	virtual void set_prg(int prg_base, int prg_mask) override;
	virtual void set_chr(u8 chr, int chr_base, int chr_mask) override;

private:
	bool m_mmc3_mode;
};


// ======================> nes_bmc_411120c_device

class nes_bmc_411120c_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_bmc_411120c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_m(offs_t offset, u8 data) override;
	virtual void prg_cb(int start, int bank) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	bool prg32_mode() { return BIT(m_reg, 3) || (m_dip.read_safe(0) && BIT(m_reg, 2)); }

	u8 m_reg;
	optional_ioport m_dip;
};


// ======================> nes_bmc_810305c_device

class nes_bmc_810305c_device : public nes_txsrom_device
{
public:
	// construction/destruction
	nes_bmc_810305c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;
	virtual u8 chr_r(offs_t offset) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	virtual void prg_cb(int start, int bank) override;
	virtual void chr_cb(int start, int bank, int source) override;
	virtual void set_chr(u8 chr, int chr_base, int chr_mask) override;

private:
	void update_live_a17(bool a17);

	u8 m_outer;
	u8 m_slot_a17;      // one bit per 1 KiB CHR window: CHR A17 (bit 7 of the MMC3 bank number serving that window)
	u8 m_last_slot;     // CHR window of the last pattern fetch
	bool m_live_a17;    // CHR A17 as of the last pattern fetch
};


// ======================> nes_bmc_820720c_device

class nes_bmc_820720c_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_bmc_820720c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_m(offs_t offset, u8 data) override;
	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	virtual void set_prg(int prg_base, int prg_mask) override;
	virtual void set_chr(u8 chr, int chr_base, int chr_mask) override;

private:
	u8 m_reg;
};


// ======================> nes_bmc_830118c_device

class nes_bmc_830118c_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_bmc_830118c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_m(offs_t offset, u8 data) override;
	virtual void prg_cb(int start, int bank) override;

	virtual void pcb_reset() override;
};


// ======================> nes_bmc_830832c_device

class nes_bmc_830832c_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_bmc_830832c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_m(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;
};


// ======================> nes_bmc_yy841101c_device

class nes_bmc_yy841101c_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_bmc_yy841101c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_m(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;
};


// ======================> nes_bmc_yy841155c_device

class nes_bmc_yy841155c_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_bmc_yy841155c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_m(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	u8 m_reg[2];
};


// ======================> nes_pjoy84_device

// TEC9719 (mapper 126), ING003C/BS-300 family (mapper 422) and ING-022 (mapper 534) enhanced MMC3 ASICs
class nes_pjoy84_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_pjoy84_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual u8 read_m(offs_t offset) override;
	virtual u8 read_h(offs_t offset) override;
	virtual void write_m(offs_t offset, uint8_t data) override;
	virtual void write_h(offs_t offset, u8 data) override;
	virtual void prg_cb(int start, int bank) override;
	virtual void chr_cb(int start, int bank, int source) override;
	virtual void set_pjoy_submapper(u8 val) override { m_submapper = val; }

	virtual void pcb_reset() override;

protected:
	// construction/destruction
	nes_pjoy84_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, u16 mapper);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	u8 get_mmc3_bank(int bank) const;
	void update_prg();
	void update_chr();
	void update_mirroring();
	void update_banks();

	u8 m_reg[4];
	u8 m_submapper;
	u8 m_sl0;
	const u16 m_mapper;
};


// ======================> nes_bs300_device

class nes_bs300_device : public nes_pjoy84_device
{
public:
	// construction/destruction
	nes_bs300_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// ======================> nes_ing022_device

class nes_ing022_device : public nes_pjoy84_device
{
public:
	// construction/destruction
	nes_ing022_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// ======================> nes_smd133_device

class nes_smd133_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_smd133_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_m(offs_t offset) override;
	virtual void write_l(offs_t offset, u8 data) override;
	virtual void write_m(offs_t offset, u8 data) override;
	virtual void chr_w(offs_t offset, u8 data) override;
	virtual void write_h(offs_t offset, u8 data) override;
	virtual void prg_cb(int start, int bank) override;
	virtual void chr_cb(int start, int bank, int source) override;
	virtual void set_smd133_submapper(u8 val) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	void smd133_write(offs_t offset, u8 data);
	void update_banks();
	void update_mirroring();

	u8 m_reg[7];    // 0-5: outer registers, 6: latched register 1 value for the submapper 8/9 PRG upper bits
	u8 m_submapper;
};


// ======================> nes_8025_device

// 8025 enhanced MMC3 ASIC (NES 2.0 mapper 176 submappers 2-5: WAIXING-FS005, JX9003B, GameStar Smart Genius Deluxe, HST-162)
class nes_8025_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_8025_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_l(offs_t offset) override;
	virtual void write_l(offs_t offset, u8 data) override;
	virtual u8 read_m(offs_t offset) override;
	virtual void write_m(offs_t offset, u8 data) override;
	virtual void write_h(offs_t offset, u8 data) override;
	virtual void set_8025_submapper(u8 val) override { m_submapper = val; }

	virtual void pcb_start(running_machine &machine, u8 *ciram_ptr, bool cart_mounted) override;
	virtual void pcb_reset() override;

protected:
	nes_8025_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void set_prg(int prg_base, int prg_mask) override;
	virtual void set_chr(u8 chr, int chr_base, int chr_mask) override;

private:
	bool ext_mode() const { return (m_submapper == 1 || m_submapper == 2) && BIT(m_ext, 1); }
	bool ram_cfg() const { return m_submapper == 2 && BIT(m_wram_protect, 5); }
	bool regs_enabled() const { return !ram_cfg() || BIT(m_wram_protect, 6); }
	std::vector<u8> &wram() { return m_battery.empty() ? m_prgram : m_battery; }    // the cart's single SRAM, battery-backed when the cart has a battery
	int mmc3_prg_bank(int page, bool ext) const;
	int prg_bank(int page) const;
	void update_mirroring();
	void update_banks();

	optional_ioport m_config;
	u8 m_submapper;
	u8 m_mode, m_prg_lsb, m_chr_lsb, m_ext, m_prg_msb, m_chr_msb, m_latch_data;
};


// ======================> nes_rinco_fsg2_device

class nes_rinco_fsg2_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_rinco_fsg2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_l(offs_t offset) override;
	virtual void write_l(offs_t offset, u8 data) override;
	virtual u8 read_m(offs_t offset) override;
	virtual void write_m(offs_t offset, u8 data) override;
	virtual void write_h(offs_t offset, u8 data) override;

	virtual void prg_cb(int start, int bank) override;
	virtual void chr_cb(int start, int bank, int source) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void adpcm_vck(int state);
	int prg_or() const;
	void fifo_clear();

	required_device<msm6585_device> m_adpcm;
	u8 m_reg[4];            // $9000, $9001, $B000, $B001
	u8 m_fifo[1024];        // Am7202A
	u16 m_fifo_read, m_fifo_count;
	u8 m_adpcm_byte;
	bool m_adpcm_low;       // the next sample comes from the low nibble
};


// ======================> nes_bmc_n49c300_device

class nes_bmc_n49c300_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_bmc_n49c300_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_m(offs_t offset) override;
	virtual void write_m(offs_t offset, u8 data) override;
	virtual void write_l(offs_t offset, u8 data) override;
	virtual void write_h(offs_t offset, u8 data) override;
	virtual void ppu_a12_rise() override;

	virtual void prg_cb(int start, int bank) override;
	virtual void chr_cb(int start, int bank, int source) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// the mapper 40 mode's IRQ counter counts M2 cycles
	virtual bool wants_m2() override { return true; }
	virtual void m2_w(int state, offs_t address, bool write) override;

private:
	void update_banks();

	u8 m_mode;              // $4100
	u8 m_smb2j_bank;        // mapper 40 mode: $E000
	u8 m_smb2j_irq_enable;
	u16 m_smb2j_irq_count;
};


// ======================> nes_bmc_620_device

class nes_bmc_620_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_bmc_620_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_l(offs_t offset, u8 data) override;
	virtual void write_m(offs_t offset, u8 data) override;

	virtual void prg_cb(int start, int bank) override;
	virtual void chr_cb(int start, int bank, int source) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	void outer_w(u8 data);
	u8 m_outer;
};


// ======================> nes_bmc_hpxx_device

class nes_bmc_hpxx_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_bmc_hpxx_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_l(offs_t offset) override;
	virtual void write_l(offs_t offset, u8 data) override;
	virtual void write_h(offs_t offset, u8 data) override;

	virtual void prg_cb(int start, int bank) override;
	virtual void chr_cb(int start, int bank, int source) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	void update_banks();

	required_ioport m_dsw;
	u8 m_mode;              // $5000
	u8 m_prg_outer;         // $5001
	u8 m_chr_outer;         // $5002
	u8 m_cn_latch;          // $8000-$FFFF in modes 4-7
};


// ======================> nes_realtec432_device

class nes_realtec432_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_realtec432_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_h(offs_t offset) override;
	virtual void write_m(offs_t offset, u8 data) override;

	virtual void prg_cb(int start, int bank) override;
	virtual void chr_cb(int start, int bank, int source) override;

	virtual void pcb_reset() override;
	virtual void set_realtec432_submapper(u8 val) override { m_submapper = val; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void set_chr(u8 chr, int chr_base, int chr_mask) override;

private:
	bool q_is_a19() const { return (m_submapper == 0) || (m_submapper == 4); }
	bool chr512() const { return (m_submapper == 3) && BIT(m_reg[1], 5); }

	required_ioport m_pad_port;
	u8 m_reg[2];            // $6000 (even), $6001 (odd)
	int m_mmc_prg[2];       // the MMC3's banks for $8000 and $A000, used by the NROM modes
	u8 m_submapper;
	u8 m_pad;               // solder pad value
};


// ======================> nes_jncota_fs005_device

class nes_jncota_fs005_device : public nes_8025_device
{
public:
	// construction/destruction
	nes_jncota_fs005_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_h(offs_t offset, u8 data) override;
	virtual void pcb_reset() override;

protected:
	virtual void set_chr(u8 chr, int chr_base, int chr_mask) override;

private:
	// the header's mirroring bit (flags 6 bit 0) says how the nametables are wired
	int hardwired_mirroring() const { return (m_mirroring == PPU_MIRROR_VERT) ? PPU_MIRROR_VERT : PPU_MIRROR_HORZ; }
};


// ======================> nes_bmc_gx121_device

class nes_bmc_gx121_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_bmc_gx121_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void write_l(offs_t offset, u8 data) override;

	virtual void prg_cb(int start, int bank) override;
	virtual void chr_cb(int start, int bank, int source) override;

	virtual void pcb_start(running_machine &machine, u8 *ciram_ptr, bool cart_mounted) override;
	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	u8 m_reg[4];            // the four outer bank registers at $5000
	u8 m_reg_index;         // the register the next $5000 write goes to
};


// ======================> nes_bmc_sb5013_device

class nes_bmc_sb5013_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_bmc_sb5013_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_m(offs_t offset) override;
	virtual void write_m(offs_t offset, u8 data) override;
	virtual void write_h(offs_t offset, u8 data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	virtual void set_prg(int prg_base, int prg_mask) override;
	virtual void set_chr(u8 chr, int chr_base, int chr_mask) override;

	// the M2 mode of the IRQ counts CPU cycles
	virtual bool wants_m2() override { return true; }
	virtual void m2_w(int state, offs_t address, bool write) override;

	nes_bmc_sb5013_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	void update_irq_enable();

	u8 m_prg_reg[4];        // $8000-$8003: $8000, $A000, $C000, $6000
	u8 m_chr[8];            // $A000-$B003
	u8 m_prg_outer;         // $9000
	u8 m_size;              // $9001
	u8 m_chr_outer;         // $9003
	u8 m_irq_mode;          // $C002: .AME
	bool m_irq_on;          // the IRQ enable of the current mode
	u16 m_m2_count;
};


// ======================> nes_unl_82112c_device

class nes_unl_82112c_device : public nes_bmc_sb5013_device
{
public:
	// construction/destruction
	nes_unl_82112c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void set_chr(u8 chr, int chr_base, int chr_mask) override;
};


// device type definition
DECLARE_DEVICE_TYPE(NES_NITRA,         nes_nitra_device)
DECLARE_DEVICE_TYPE(NES_BMW8544,       nes_bmw8544_device)
DECLARE_DEVICE_TYPE(NES_FS6,           nes_fs6_device)
DECLARE_DEVICE_TYPE(NES_SBROS11,       nes_sbros11_device)
DECLARE_DEVICE_TYPE(NES_MALISB,        nes_malisb_device)
DECLARE_DEVICE_TYPE(NES_FAMILY4646,    nes_family4646_device)
DECLARE_DEVICE_TYPE(NES_HENGG_412,     nes_hengg412_device)
DECLARE_DEVICE_TYPE(NES_PIKAY2K,       nes_pikay2k_device)
DECLARE_DEVICE_TYPE(NES_8237,          nes_8237_device)
DECLARE_DEVICE_TYPE(NES_8237A,         nes_8237a_device)
DECLARE_DEVICE_TYPE(NES_158B,          nes_158b_device)
DECLARE_DEVICE_TYPE(NES_SG_LIONK,      nes_sglionk_device)
DECLARE_DEVICE_TYPE(NES_SG_BOOG,       nes_sgboog_device)
DECLARE_DEVICE_TYPE(NES_KASING,        nes_kasing_device)
DECLARE_DEVICE_TYPE(NES_KAY,           nes_kay_device)
DECLARE_DEVICE_TYPE(NES_H2288,         nes_h2288_device)
DECLARE_DEVICE_TYPE(NES_6035052,       nes_6035052_device)
DECLARE_DEVICE_TYPE(NES_TXC_TW,        nes_txc_tw_device)
DECLARE_DEVICE_TYPE(NES_KOF97,         nes_kof97_device)
DECLARE_DEVICE_TYPE(NES_KOF96,         nes_kof96_device)
DECLARE_DEVICE_TYPE(NES_SF3,           nes_sf3_device)
DECLARE_DEVICE_TYPE(NES_COCOMA,        nes_cocoma_device)
DECLARE_DEVICE_TYPE(NES_GOUDER,        nes_gouder_device)
DECLARE_DEVICE_TYPE(NES_SA9602B,       nes_sa9602b_device)
DECLARE_DEVICE_TYPE(NES_SACHEN_SHERO,  nes_sachen_shero_device)
DECLARE_DEVICE_TYPE(NES_SACHEN_ZGDH,   nes_sachen_zgdh_device)
DECLARE_DEVICE_TYPE(NES_A9746,         nes_a9746_device)

DECLARE_DEVICE_TYPE(NES_A88S1,         nes_a88s1_device)
DECLARE_DEVICE_TYPE(NES_BMC_EL86XC,    nes_bmc_el86xc_device)
DECLARE_DEVICE_TYPE(NES_FK23C,         nes_fk23c_device)
DECLARE_DEVICE_TYPE(NES_FK23CA,        nes_fk23ca_device)
DECLARE_DEVICE_TYPE(NES_NT639,         nes_nt639_device)
DECLARE_DEVICE_TYPE(NES_RESETTXROM,    nes_resettxrom_device)
DECLARE_DEVICE_TYPE(NES_S24IN1SC03,    nes_s24in1sc03_device)
DECLARE_DEVICE_TYPE(NES_TECHLINE9IN1,  nes_tech9in1_device)
DECLARE_DEVICE_TYPE(NES_BMC_5IN1,      nes_bmc_5in1_device)
DECLARE_DEVICE_TYPE(NES_BMC_8IN1,      nes_bmc_8in1_device)
DECLARE_DEVICE_TYPE(NES_BMC_15IN1,     nes_bmc_15in1_device)
DECLARE_DEVICE_TYPE(NES_BMC_SBIG7,     nes_bmc_sbig7_device)
DECLARE_DEVICE_TYPE(NES_BMC_HIK8,      nes_bmc_hik8_device)
DECLARE_DEVICE_TYPE(NES_BMC_JY208,     nes_bmc_jy208_device)
DECLARE_DEVICE_TYPE(NES_BMC_JY302,     nes_bmc_jy302_device)
DECLARE_DEVICE_TYPE(NES_BMC_KC885,     nes_bmc_kc885_device)
DECLARE_DEVICE_TYPE(NES_BMC_SFC12,     nes_bmc_sfc12_device)
DECLARE_DEVICE_TYPE(NES_BMC_HIK4,      nes_bmc_hik4_device)
DECLARE_DEVICE_TYPE(NES_BMC_F15,       nes_bmc_f15_device)
DECLARE_DEVICE_TYPE(NES_BMC_F600,      nes_bmc_f600_device)
DECLARE_DEVICE_TYPE(NES_BMC_GN45,      nes_bmc_gn45_device)
DECLARE_DEVICE_TYPE(NES_BMC_GOLD7IN1,  nes_bmc_gold7in1_device)
DECLARE_DEVICE_TYPE(NES_BMC_K3006,     nes_bmc_k3006_device)
DECLARE_DEVICE_TYPE(NES_BMC_K3033,     nes_bmc_k3033_device)
DECLARE_DEVICE_TYPE(NES_BMC_L6IN1,     nes_bmc_l6in1_device)
DECLARE_DEVICE_TYPE(NES_BMC_00202650,  nes_bmc_00202650_device)
DECLARE_DEVICE_TYPE(NES_BMC_411120C,   nes_bmc_411120c_device)
DECLARE_DEVICE_TYPE(NES_BMC_810305C,   nes_bmc_810305c_device)
DECLARE_DEVICE_TYPE(NES_BMC_820720C,   nes_bmc_820720c_device)
DECLARE_DEVICE_TYPE(NES_BMC_830118C,   nes_bmc_830118c_device)
DECLARE_DEVICE_TYPE(NES_BMC_830832C,   nes_bmc_830832c_device)
DECLARE_DEVICE_TYPE(NES_BMC_YY841101C, nes_bmc_yy841101c_device)
DECLARE_DEVICE_TYPE(NES_BMC_YY841155C, nes_bmc_yy841155c_device)
DECLARE_DEVICE_TYPE(NES_PJOY84,        nes_pjoy84_device)
DECLARE_DEVICE_TYPE(NES_BS300,         nes_bs300_device)
DECLARE_DEVICE_TYPE(NES_ING022,        nes_ing022_device)
DECLARE_DEVICE_TYPE(NES_8025,          nes_8025_device)
DECLARE_DEVICE_TYPE(NES_SMD133,        nes_smd133_device)
DECLARE_DEVICE_TYPE(NES_RINCO_FSG2,    nes_rinco_fsg2_device)
DECLARE_DEVICE_TYPE(NES_BMC_N49C300,  nes_bmc_n49c300_device)
DECLARE_DEVICE_TYPE(NES_BMC_620,      nes_bmc_620_device)
DECLARE_DEVICE_TYPE(NES_BMC_HPXX,      nes_bmc_hpxx_device)
DECLARE_DEVICE_TYPE(NES_REALTEC432,    nes_realtec432_device)
DECLARE_DEVICE_TYPE(NES_JNCOTA_FS005,  nes_jncota_fs005_device)
DECLARE_DEVICE_TYPE(NES_BMC_GX121,     nes_bmc_gx121_device)
DECLARE_DEVICE_TYPE(NES_BMC_SB5013,    nes_bmc_sb5013_device)
DECLARE_DEVICE_TYPE(NES_UNL_82112C,    nes_unl_82112c_device)

#endif // MAME_BUS_NES_MMC3_CLONES_H
