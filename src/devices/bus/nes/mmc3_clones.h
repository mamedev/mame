// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __NES_MMC3_CLONES_H
#define __NES_MMC3_CLONES_H

#include "mmc3.h"


// ======================> nes_nitra_device

class nes_nitra_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_nitra_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual DECLARE_WRITE8_MEMBER(write_h);
};


// ======================> nes_ks7057_device

class nes_ks7057_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_ks7057_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual DECLARE_WRITE8_MEMBER(write_h);
};


// ======================> nes_sbros11_device

class nes_sbros11_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_sbros11_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual DECLARE_WRITE8_MEMBER(write_h);
};


// ======================> nes_malisb_device

class nes_malisb_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_malisb_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual DECLARE_WRITE8_MEMBER(write_h);
	virtual void prg_cb(int start, int bank) override;
	virtual void chr_cb(int start, int bank, int source) override;
};


// ======================> nes_family4646_device

class nes_family4646_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_family4646_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual DECLARE_WRITE8_MEMBER(write_m);

	virtual void pcb_reset() override;
};


// ======================> nes_pikay2k_device

class nes_pikay2k_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_pikay2k_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_READ8_MEMBER(read_m);
	virtual DECLARE_WRITE8_MEMBER(write_m);
	virtual DECLARE_WRITE8_MEMBER(write_h);

	virtual void pcb_reset() override;

private:
	UINT8 m_reg[2];
};


// ======================> nes_8237_device

class nes_8237_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_8237_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_l);
	virtual DECLARE_WRITE8_MEMBER(write_h);
	virtual void prg_cb(int start, int bank) override;
	virtual void chr_cb(int start, int bank, int source) override;

	virtual void pcb_reset() override;

private:
	UINT8 m_reg[3];
	int m_cd_enable;
};


// ======================> nes_sglionk_device

class nes_sglionk_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_sglionk_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_m);
	virtual DECLARE_WRITE8_MEMBER(write_h);

	virtual void pcb_reset() override;

private:
	UINT8 m_reg;
	int m_reg_enabled;
};


// ======================> nes_sgboog_device

class nes_sgboog_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_sgboog_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_l);
	virtual DECLARE_WRITE8_MEMBER(write_m);
	virtual DECLARE_WRITE8_MEMBER(write_h);
	virtual void prg_cb(int start, int bank) override;
	virtual void chr_cb(int start, int bank, int source) override;

	virtual void pcb_reset() override;

private:
	virtual void set_prg(int prg_base, int prg_mask) override;
	UINT8 m_reg[3];
	UINT8 m_mode;
};


// ======================> nes_kasing_device

class nes_kasing_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_kasing_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_m);
	virtual void prg_cb(int start, int bank) override;

	virtual void pcb_reset() override;

private:
	UINT8 m_reg;
};


// ======================> nes_kay_device

class nes_kay_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_kay_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_READ8_MEMBER(read_l);
	virtual DECLARE_WRITE8_MEMBER(write_l);
	virtual DECLARE_WRITE8_MEMBER(write_h);
	virtual void prg_cb(int start, int bank) override;
	virtual void chr_cb(int start, int bank, int source) override;

	virtual void pcb_reset() override;

private:
	void update_regs();
	UINT8 m_reg[8];
	UINT8 m_low_reg;
};


// ======================> nes_h2288_device

class nes_h2288_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_h2288_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_READ8_MEMBER(read_l);
	virtual DECLARE_WRITE8_MEMBER(write_l);
	virtual DECLARE_WRITE8_MEMBER(write_h);
	virtual void prg_cb(int start, int bank) override;

	virtual void pcb_reset() override;

private:
	UINT8 m_reg[2]; // reg 1 is unused?
};


// ======================> nes_6035052_device

class nes_6035052_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_6035052_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_READ8_MEMBER(read_ex);
	virtual DECLARE_WRITE8_MEMBER(write_ex);
	virtual DECLARE_READ8_MEMBER(read_l) override { return read_ex(space, offset, mem_mask); }
	virtual DECLARE_READ8_MEMBER(read_m) override { return read_ex(space, offset, mem_mask); }
	virtual DECLARE_WRITE8_MEMBER(write_l) override { write_ex(space, offset, data, mem_mask); }
	virtual DECLARE_WRITE8_MEMBER(write_m) override { write_ex(space, offset, data, mem_mask); }

	virtual void pcb_reset() override;

private:
	UINT8 m_prot;
};


// ======================> nes_txc_tw_device

class nes_txc_tw_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_txc_tw_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual DECLARE_WRITE8_MEMBER(write_l);
	virtual DECLARE_WRITE8_MEMBER(write_m) override { write_l(space, offset & 0xff, data, mem_mask); }   // offset does not really count for this mapper }
	virtual void prg_cb(int start, int bank) override;
};


// ======================> nes_kof97_device

class nes_kof97_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_kof97_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual DECLARE_WRITE8_MEMBER(write_h);
};


// ======================> nes_kof96_device

class nes_kof96_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_kof96_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_READ8_MEMBER(read_l);
	virtual DECLARE_WRITE8_MEMBER(write_l);
	virtual DECLARE_WRITE8_MEMBER(write_h);   // offset does not really count for this mapper
	virtual void prg_cb(int start, int bank) override;
	virtual void chr_cb(int start, int bank, int source) override;

	virtual void pcb_reset() override;

private:
	UINT8 m_reg[4];
};


// ======================> nes_sf3_device

class nes_sf3_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_sf3_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual DECLARE_WRITE8_MEMBER(write_h);

protected:
	virtual void set_chr(UINT8 chr, int chr_base, int chr_mask) override;
};


// ======================> nes_gouder_device

class nes_gouder_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_gouder_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_READ8_MEMBER(read_l);
	virtual DECLARE_WRITE8_MEMBER(write_l);
	virtual void prg_cb(int start, int bank) override;

	virtual void pcb_reset() override;

private:
	UINT8 m_reg[5];
};


// ======================> nes_sa9602b_device

class nes_sa9602b_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_sa9602b_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_h);
	virtual void prg_cb(int start, int bank) override;

	virtual void pcb_reset() override;

private:
	UINT8 m_reg;
	int m_prg_chip;
};


// ======================> nes_sachen_shero_device

class nes_sachen_shero_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_sachen_shero_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_READ8_MEMBER(read_l);
	virtual DECLARE_WRITE8_MEMBER(write_l);
	virtual void chr_cb(int start, int bank, int source) override;

	virtual void pcb_reset() override;

private:
	UINT8 m_reg;
};

#ifdef UNUSED_FUNCTION
// ======================> nes_a9746_device

class nes_a9746_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_a9746_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual DECLARE_WRITE8_MEMBER(write_h);

	virtual void pcb_reset();

private:
	void update_banks(UINT8 value);
	UINT8 m_reg[3];
};
#endif


// ======================> nes_fk23c_device

class nes_fk23c_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_fk23c_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	nes_fk23c_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_l);
	virtual DECLARE_WRITE8_MEMBER(write_h);
	virtual void prg_cb(int start, int bank) override;
	virtual void chr_cb(int start, int bank, int source) override;

	virtual void pcb_reset() override;

protected:
	void fk23c_set_prg();
	void fk23c_set_chr();
	UINT8 m_reg[8];
	UINT8 m_mmc_cmd1;
};


// ======================> nes_fk23ca_device

class nes_fk23ca_device : public nes_fk23c_device
{
public:
	// construction/destruction
	nes_fk23ca_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual void pcb_reset() override;
};


// ======================> nes_s24in1sc03_device

class nes_s24in1sc03_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_s24in1sc03_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_l);
	virtual void prg_cb(int start, int bank) override;
	virtual void chr_cb(int start, int bank, int source) override;

	virtual void pcb_reset() override;

private:
	UINT8 m_reg[3];
};


// ======================> nes_bmc_15in1_device

class nes_bmc_15in1_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_bmc_15in1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual DECLARE_WRITE8_MEMBER(write_m);

	virtual void pcb_reset() override;
};


// ======================> nes_bmc_sbig7_device

class nes_bmc_sbig7_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_bmc_sbig7_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual DECLARE_WRITE8_MEMBER(write_h);

	virtual void pcb_reset() override;
};


// ======================> nes_bmc_hik8_device

class nes_bmc_hik8_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_bmc_hik8_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_m);

	virtual void pcb_reset() override;

private:
	UINT8 m_reg[4];
	UINT8 m_count;
};


// ======================> nes_bmc_hik4_device

class nes_bmc_hik4_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_bmc_hik4_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual DECLARE_WRITE8_MEMBER(write_m);

	virtual void pcb_reset() override;
};


// ======================> nes_bmc_mario7in1_device

class nes_bmc_mario7in1_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_bmc_mario7in1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_m);

	virtual void pcb_reset() override;

private:
	UINT8 m_reg_written;
};


// ======================> nes_bmc_gold7in1_device

class nes_bmc_gold7in1_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_bmc_gold7in1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_m);

	virtual void pcb_reset() override;

private:
	UINT8 m_reg_written;
};


// ======================> nes_bmc_gc6in1_device

class nes_bmc_gc6in1_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_bmc_gc6in1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_l);
	virtual DECLARE_WRITE8_MEMBER(write_h);
	virtual void prg_cb(int start, int bank) override;
	virtual void chr_cb(int start, int bank, int source) override;

	virtual void pcb_reset() override;

private:
	UINT8 m_reg[4];
};


// ======================> nes_bmc_411120c_device

class nes_bmc_411120c_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_bmc_411120c_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_m);
	virtual void prg_cb(int start, int bank) override;
	virtual void chr_cb(int start, int bank, int source) override;

	virtual void pcb_reset() override;

private:
	UINT8 m_reg;
};


// ======================> nes_bmc_830118c_device

class nes_bmc_830118c_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_bmc_830118c_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_m);
	virtual void prg_cb(int start, int bank) override;
	virtual void chr_cb(int start, int bank, int source) override;

	virtual void pcb_reset() override;

private:
	UINT8 m_reg;
};


// ======================> nes_pjoy84_device

class nes_pjoy84_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_pjoy84_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_m);
	virtual void prg_cb(int start, int bank) override;
	virtual void chr_cb(int start, int bank, int source) override;

	virtual void pcb_reset() override;

private:
	inline void set_base_mask();
	UINT8 m_reg[4];
};




// device type definition
extern const device_type NES_NITRA;
extern const device_type NES_KS7057;
extern const device_type NES_SBROS11;
extern const device_type NES_MALISB;
extern const device_type NES_FAMILY4646;
extern const device_type NES_PIKAY2K;
extern const device_type NES_8237;
extern const device_type NES_SG_LIONK;
extern const device_type NES_SG_BOOG;
extern const device_type NES_KASING;
extern const device_type NES_KAY;
extern const device_type NES_H2288;
extern const device_type NES_6035052;
extern const device_type NES_TXC_TW;
extern const device_type NES_KOF97;
extern const device_type NES_KOF96;
extern const device_type NES_SF3;
extern const device_type NES_GOUDER;
extern const device_type NES_SA9602B;
extern const device_type NES_SACHEN_SHERO;
extern const device_type NES_A9746;

extern const device_type NES_FK23C;
extern const device_type NES_FK23CA;
extern const device_type NES_S24IN1SC03;
extern const device_type NES_BMC_15IN1;
extern const device_type NES_BMC_SBIG7;
extern const device_type NES_BMC_HIK8;
extern const device_type NES_BMC_HIK4;
extern const device_type NES_BMC_MARIO7IN1;
extern const device_type NES_BMC_GOLD7IN1;
extern const device_type NES_BMC_GC6IN1;
extern const device_type NES_BMC_411120C;
extern const device_type NES_BMC_830118C;
extern const device_type NES_PJOY84;

#endif
