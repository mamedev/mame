// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __NES_WAIXING_H
#define __NES_WAIXING_H

#include "mmc3.h"


// ======================> nes_waixing_a_device

class nes_waixing_a_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_waixing_a_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);
	nes_waixing_a_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_READ8_MEMBER(read_l) override;
	virtual DECLARE_WRITE8_MEMBER(write_l) override;
	virtual DECLARE_WRITE8_MEMBER(waixing_write);
	virtual DECLARE_WRITE8_MEMBER(write_h) override { waixing_write(space, offset, data, mem_mask); }
	virtual void chr_cb(int start, int bank, int source) override;

	virtual void pcb_reset() override;

protected:
	void set_mirror(UINT8 nt);
	UINT8 mapper_ram[0x400];
};


// ======================> nes_waixing_a1_device

class nes_waixing_a1_device : public nes_waixing_a_device
{
public:
	// construction/destruction
	nes_waixing_a1_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	virtual void chr_cb(int start, int bank, int source) override;
};


// ======================> nes_waixing_b_device

class nes_waixing_b_device : public nes_waixing_a_device
{
public:
	// construction/destruction
	nes_waixing_b_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void chr_cb(int start, int bank, int source) override;
};


// ======================> nes_waixing_c_device

class nes_waixing_c_device : public nes_waixing_a_device
{
public:
	// construction/destruction
	nes_waixing_c_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void chr_cb(int start, int bank, int source) override;
};


// ======================> nes_waixing_d_device

class nes_waixing_d_device : public nes_waixing_a_device
{
public:
	// construction/destruction
	nes_waixing_d_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void chr_cb(int start, int bank, int source) override;
};


// ======================> nes_waixing_e_device

class nes_waixing_e_device : public nes_waixing_a_device
{
public:
	// construction/destruction
	nes_waixing_e_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void chr_cb(int start, int bank, int source) override;
};


// ======================> nes_waixing_f_device

class nes_waixing_f_device : public nes_waixing_a_device
{
public:
	// construction/destruction
	nes_waixing_f_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual DECLARE_WRITE8_MEMBER(write_h) override;
	virtual void prg_cb(int start, int bank) override;
	virtual void chr_cb(int start, int bank, int source) override;

	virtual void pcb_reset() override;
private:
	virtual void set_prg(int prg_base, int prg_mask) override;
};


// ======================> nes_waixing_g_device

class nes_waixing_g_device : public nes_waixing_a_device
{
public:
	// construction/destruction
	nes_waixing_g_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual DECLARE_WRITE8_MEMBER(write_h) override;
	virtual void chr_cb(int start, int bank, int source) override;

	virtual void pcb_reset() override;

protected:
	virtual void set_chr(UINT8 chr, int chr_base, int chr_mask) override;
};


// ======================> nes_waixing_h_device

class nes_waixing_h_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_waixing_h_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);
	nes_waixing_h_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual DECLARE_WRITE8_MEMBER(write_h) override;
	virtual void chr_cb(int start, int bank, int source) override;

	// This PCB does not have 1K of internal RAM, so it's not derived from nes_waixing_a_device!!
};


// ======================> nes_waixing_h1_device

class nes_waixing_h1_device : public nes_waixing_h_device
{
public:
	// construction/destruction
	nes_waixing_h1_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	// This variant does not ignore the wram protect!
};


// ======================> nes_waixing_i_device

class nes_waixing_i_device : public nes_waixing_a_device
{
public:
	// construction/destruction
	nes_waixing_i_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// still to emulate this variant
};


// ======================> nes_waixing_j_device

class nes_waixing_j_device : public nes_waixing_a_device
{
public:
	// construction/destruction
	nes_waixing_j_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	virtual void pcb_reset() override;

protected:
	virtual void set_prg(int prg_base, int prg_mask) override;
	UINT8 m_reg[4];
};


// ======================> nes_waixing_sh2_device

class nes_waixing_sh2_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_waixing_sh2_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_READ8_MEMBER(chr_r) override;
	virtual void chr_cb(int start, int bank, int source) override;

	virtual void pcb_reset() override;

protected:
	UINT8 m_reg[2];
};


// ======================> nes_waixing_sec_device

class nes_waixing_sec_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_waixing_sec_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_l) override;
	virtual void prg_cb(int start, int bank) override;
	virtual void chr_cb(int start, int bank, int source) override;

	virtual void pcb_reset() override;

protected:
	UINT8 m_reg;
};


// ======================> nes_waixing_sgz_device

class nes_waixing_sgz_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_waixing_sgz_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	virtual void hblank_irq(int scanline, int vblank, int blanked) override;
	virtual void pcb_reset() override;

protected:
	UINT16 m_irq_count, m_irq_count_latch;
	int m_irq_enable, m_irq_enable_latch;

	UINT8 m_mmc_vrom_bank[8];
};


// ======================> nes_waixing_sgzlz_device

class nes_waixing_sgzlz_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_waixing_sgzlz_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_l) override;

	virtual void pcb_reset() override;

protected:
	UINT8 m_latch;
};


// ======================> nes_waixing_ffv_device

class nes_waixing_ffv_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_waixing_ffv_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_l) override;

	virtual void pcb_reset() override;

protected:
	UINT8 m_reg[2];
};


// ======================> nes_waixing_wxzs_device

class nes_waixing_wxzs_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_waixing_wxzs_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	virtual void pcb_reset() override;
};


// ======================> nes_waixing_dq8_device

class nes_waixing_dq8_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_waixing_dq8_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	virtual void pcb_reset() override;
};


// ======================> nes_waixing_wxzs2_device

class nes_waixing_wxzs2_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_waixing_wxzs2_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	virtual void pcb_reset() override;
};


// ======================> nes_waixing_fs304_device

class nes_waixing_fs304_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_waixing_fs304_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual DECLARE_WRITE8_MEMBER(write_l) override;

	virtual void pcb_reset() override;

protected:
	UINT8 m_reg[4];
};





// device type definition
extern const device_type NES_WAIXING_A;
extern const device_type NES_WAIXING_A1;
extern const device_type NES_WAIXING_B;
extern const device_type NES_WAIXING_C;
extern const device_type NES_WAIXING_D;
extern const device_type NES_WAIXING_E;
extern const device_type NES_WAIXING_F;
extern const device_type NES_WAIXING_G;
extern const device_type NES_WAIXING_H;
extern const device_type NES_WAIXING_H1;
extern const device_type NES_WAIXING_I;
extern const device_type NES_WAIXING_J;
extern const device_type NES_WAIXING_SH2;
extern const device_type NES_WAIXING_SEC;
extern const device_type NES_WAIXING_SGZ;
extern const device_type NES_WAIXING_SGZLZ;
extern const device_type NES_WAIXING_FFV;
extern const device_type NES_WAIXING_WXZS;
extern const device_type NES_WAIXING_DQ8;
extern const device_type NES_WAIXING_WXZS2;
extern const device_type NES_WAIXING_FS304;


#endif
