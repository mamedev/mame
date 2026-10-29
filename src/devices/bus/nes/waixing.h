// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Brian H. Gilbert
#ifndef MAME_BUS_NES_WAIXING_H
#define MAME_BUS_NES_WAIXING_H

#pragma once

#include "machine/eepromser.h"
#include "sound/tms5220.h"
#include "mmc3.h"


// ======================> nes_waixing_a_device

class nes_waixing_a_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_waixing_a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_l(offs_t offset) override;
	virtual void write_l(offs_t offset, uint8_t data) override;
	virtual void waixing_write(offs_t offset, uint8_t data);
	virtual void write_h(offs_t offset, uint8_t data) override { waixing_write(offset, data); }
	virtual void chr_cb(int start, int bank, int source) override;

	virtual void pcb_reset() override;

protected:
	nes_waixing_a_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	void set_mirror(uint8_t nt);

	uint8_t mapper_ram[0x400];
};


// ======================> nes_waixing_b_device

class nes_waixing_b_device : public nes_waixing_a_device
{
public:
	// construction/destruction
	nes_waixing_b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void set_waixing_b_submapper(u8 val) override { m_submapper = val; }
	virtual void waixing_write(offs_t offset, uint8_t data) override;
	virtual void prg_cb(int start, int bank) override;
	virtual void chr_cb(int start, int bank, int source) override;
	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	u8 m_submapper;
	u8 m_outer;
};


// ======================> nes_waixing_c_device

class nes_waixing_c_device : public nes_waixing_a_device
{
public:
	// construction/destruction
	nes_waixing_c_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void chr_cb(int start, int bank, int source) override;
};


// ======================> nes_waixing_d_device

class nes_waixing_d_device : public nes_waixing_a_device
{
public:
	// construction/destruction
	nes_waixing_d_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void chr_cb(int start, int bank, int source) override;
};


// ======================> nes_waixing_e_device

class nes_waixing_e_device : public nes_waixing_a_device
{
public:
	// construction/destruction
	nes_waixing_e_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_l(offs_t offset) override;
	virtual void write_l(offs_t offset, uint8_t data) override;
	virtual void chr_cb(int start, int bank, int source) override;
	virtual void chr_w(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	bool gal_chr_is_ram(int bank) const;
	uint8_t chr_bank_for_window(int window) const;

	uint8_t m_ram5000[0x1000];  // 4K of non-battery-backed RAM at $5000-$5fff
	uint8_t m_gal;              // GAL setting: the last CHR bank number with bit 7 set that was written (see waixing.cpp)
};


// ======================> nes_waixing_f_device

class nes_waixing_f_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_waixing_f_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 read_l(offs_t offset) override;
	virtual void write_l(offs_t offset, u8 data) override;
	virtual void prg_cb(int start, int bank) override;

protected:
	u8 *exram_ptr(offs_t offset);

	// construction/destruction
	nes_waixing_f_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);
};


// ======================> nes_waixing_f1_device

class nes_waixing_f1_device : public nes_waixing_f_device
{
public:
	// construction/destruction
	nes_waixing_f1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void set_chr(uint8_t chr, int chr_base, int chr_mask) override;
};


// ======================> nes_waixing_g_device

class nes_waixing_g_device : public nes_waixing_a_device
{
public:
	// construction/destruction
	nes_waixing_g_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;
	virtual void chr_cb(int start, int bank, int source) override;

	virtual void pcb_reset() override;

protected:
	virtual void set_chr(uint8_t chr, int chr_base, int chr_mask) override;
};


// ======================> nes_waixing_h_device

class nes_waixing_h_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_waixing_h_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;
	virtual void chr_cb(int start, int bank, int source) override;

	// This PCB does not have 1K of internal RAM, so it's not derived from nes_waixing_a_device!!

protected:
	nes_waixing_h_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
};


// ======================> nes_waixing_h1_device

class nes_waixing_h1_device : public nes_waixing_h_device
{
public:
	// construction/destruction
	nes_waixing_h1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;

	// This variant does not ignore the wram protect!
};


// ======================> nes_waixing_i_device

class nes_waixing_i_device : public nes_waixing_a_device
{
public:
	// construction/destruction
	nes_waixing_i_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// still to emulate this variant
};


// ======================> nes_waixing_j_device

class nes_waixing_j_device : public nes_waixing_a_device
{
public:
	// construction/destruction
	nes_waixing_j_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;
	virtual void chr_cb(int start, int bank, int source) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	virtual void set_prg(int prg_base, int prg_mask) override;
	uint8_t m_reg[4];
};


// ======================> nes_waixing_sh2_device

class nes_waixing_sh2_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_waixing_sh2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u8 chr_r(offs_t offset) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	virtual void set_chr(u8 chr, int chr_base, int chr_mask) override;

private:
	u8 m_reg[2];
};


// ======================> nes_waixing_sec_device

class nes_waixing_sec_device : public nes_txrom_device
{
public:
	// construction/destruction
	nes_waixing_sec_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_l(offs_t offset, uint8_t data) override;
	virtual void prg_cb(int start, int bank) override;
	virtual void chr_cb(int start, int bank, int source) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	uint8_t m_reg;
};


// ======================> nes_waixing_sgzlz_device

class nes_waixing_sgzlz_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_waixing_sgzlz_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_l(offs_t offset) override;
	virtual void write_l(offs_t offset, uint8_t data) override;
	virtual uint8_t read_m(offs_t offset) override;
	virtual void write_m(offs_t offset, uint8_t data) override;
	virtual uint8_t read_h(offs_t offset) override;

	virtual void pcb_reset() override;
	virtual void set_sgzlz_submapper(u8 val) override { m_submapper = val; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	uint8_t m_reg[4];

private:
	void lpc_reset();
	uint32_t ram_index(offs_t offset, size_t size) const;

	required_device<tms5220c_device> m_lpc;   // submapper 2: TMS5220-compatible LPC speech chip
	required_ioport m_pad;                    // submapper 3: solder pad
	uint8_t m_submapper;
	uint8_t m_pad_read;                       // submapper 3: PRG-ROM reads return the solder pad
	uint8_t m_lpc_nibble;                     // low nibble waiting for its high nibble
	uint8_t m_lpc_have_nibble;
};


// ======================> nes_waixing_ffv_device

class nes_waixing_ffv_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_waixing_ffv_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_l(offs_t offset) override;
	virtual void write_l(offs_t offset, uint8_t data) override;
	virtual uint8_t chr_r(offs_t offset) override;
	virtual void chr_w(offs_t offset, uint8_t data) override;
	virtual uint8_t nt_r(offs_t offset) override;
	virtual void nt_w(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void update_banks();

	required_device<eeprom_serial_93cxx_device> m_eeprom;
	uint8_t m_reg[4];      // $5000, $5100, $5200, $5300
	bool m_last_nt;        // last PPU access was a nametable access (PPU A13=1)
	uint8_t m_latch_a0;    // PPU A0 latched on the last rise of PPU A13 (1 bpp mode CHR A3)
	uint8_t m_latch_a9;    // PPU A9 latched on the last rise of PPU A13 (1 bpp mode CHR A12)
};


// ======================> nes_waixing_wxzs_device

class nes_waixing_wxzs_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_waixing_wxzs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;
};


// ======================> nes_waixing_dq8_device

class nes_waixing_dq8_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_waixing_dq8_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;
};


// ======================> nes_waixing_wxzs2_device

class nes_waixing_wxzs2_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_waixing_wxzs2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_h(offs_t offset, uint8_t data) override;
};


// ======================> nes_waixing_fs304_device

class nes_waixing_fs304_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_waixing_fs304_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t chr_r(offs_t offset) override;
	virtual void chr_w(offs_t offset, uint8_t data) override;
	virtual uint8_t nt_r(offs_t offset) override;
	virtual void nt_w(offs_t offset, uint8_t data) override;
	virtual void write_l(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	void update_prg();

	uint8_t m_reg[4];
	bool m_last_nt;        // last PPU access was a nametable access (PPU A13=1)
	uint8_t m_chr_a12;      // PPU A9 latched on the last rise of PPU A13
};


// ======================> nes_waixing_yc030309_device

class nes_waixing_yc030309_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_waixing_yc030309_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_l(offs_t offset) override;
	virtual void write_l(offs_t offset, uint8_t data) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void update_banks();

	required_device<eeprom_serial_93cxx_device> m_eeprom;
	uint8_t m_reg[4];      // $5000, $5100, $5200 as latched after the D0/D1 swap, $5300 as written
};


// device type definition
DECLARE_DEVICE_TYPE(NES_WAIXING_A,     nes_waixing_a_device)
DECLARE_DEVICE_TYPE(NES_WAIXING_B,     nes_waixing_b_device)
DECLARE_DEVICE_TYPE(NES_WAIXING_C,     nes_waixing_c_device)
DECLARE_DEVICE_TYPE(NES_WAIXING_D,     nes_waixing_d_device)
DECLARE_DEVICE_TYPE(NES_WAIXING_E,     nes_waixing_e_device)
DECLARE_DEVICE_TYPE(NES_WAIXING_F,     nes_waixing_f_device)
DECLARE_DEVICE_TYPE(NES_WAIXING_F1,    nes_waixing_f1_device)
DECLARE_DEVICE_TYPE(NES_WAIXING_G,     nes_waixing_g_device)
DECLARE_DEVICE_TYPE(NES_WAIXING_H,     nes_waixing_h_device)
DECLARE_DEVICE_TYPE(NES_WAIXING_H1,    nes_waixing_h1_device)
DECLARE_DEVICE_TYPE(NES_WAIXING_I,     nes_waixing_i_device)
DECLARE_DEVICE_TYPE(NES_WAIXING_J,     nes_waixing_j_device)
DECLARE_DEVICE_TYPE(NES_WAIXING_SH2,   nes_waixing_sh2_device)
DECLARE_DEVICE_TYPE(NES_WAIXING_SEC,   nes_waixing_sec_device)
DECLARE_DEVICE_TYPE(NES_WAIXING_SGZLZ, nes_waixing_sgzlz_device)
DECLARE_DEVICE_TYPE(NES_WAIXING_FFV,   nes_waixing_ffv_device)
DECLARE_DEVICE_TYPE(NES_WAIXING_YC030309, nes_waixing_yc030309_device)
DECLARE_DEVICE_TYPE(NES_WAIXING_WXZS,  nes_waixing_wxzs_device)
DECLARE_DEVICE_TYPE(NES_WAIXING_DQ8,   nes_waixing_dq8_device)
DECLARE_DEVICE_TYPE(NES_WAIXING_WXZS2, nes_waixing_wxzs2_device)
DECLARE_DEVICE_TYPE(NES_WAIXING_FS304, nes_waixing_fs304_device)

#endif // MAME_BUS_NES_WAIXING_H
