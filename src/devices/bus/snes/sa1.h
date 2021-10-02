// license:BSD-3-Clause
// copyright-holders:Fabio Priuli, R. Belmont
#ifndef MAME_BUS_SNES_SA1_H
#define MAME_BUS_SNES_SA1_H

#pragma once

#include "snes_slot.h"
#include "cpu/g65816/g65816.h"


// ======================> sns_sa1_device

class sns_sa1_device : public device_t,
						public device_sns_cart_interface
{
public:
	// construction/destruction
	sns_sa1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void device_add_mconfig(machine_config &config) override;

	// reading and writing
	virtual uint8_t read_l(offs_t offset) override;
	virtual uint8_t read_h(offs_t offset) override;
	virtual void write_l(offs_t offset, uint8_t data) override;
	virtual void write_h(offs_t offset, uint8_t data) override;

	// additional reading and writing
	virtual uint8_t chip_read(offs_t offset) override;
	virtual void chip_write(offs_t offset, uint8_t data) override;

private:
	template<bool SA1Read> uint8_t var_length_read(uint32_t offset);
	void dma_transfer();
	void dma_cctype1_transfer();
	void dma_cctype2_transfer();

	template<bool SA1Read> uint8_t read_regs(uint32_t offset);
	uint8_t read_iram(uint32_t offset);
	template<bool SA1Read> uint8_t read_bwram(uint32_t offset, bool bitmap = false);
	uint8_t read_cconv1_dma(uint32_t offset);
	void write_regs(uint32_t offset, uint8_t data);
	void write_iram(uint32_t offset, uint8_t data);
	void write_bwram(uint32_t offset, uint8_t data, bool bitmap = false);
	void recalc_irqs();

	required_device<g65816_device> m_sa1;

	uint8_t m_internal_ram[0x800];

	// register related
	// $2200
	uint8_t m_sa1_ctrl;
	// $2201
	uint8_t m_scpu_sie;
	// $2203-$2208
	uint16_t m_sa1_reset, m_sa1_nmi, m_sa1_irq;
	// $2209
	uint8_t m_scpu_ctrl;
	// $220a
	uint8_t m_sa1_sie;
	// $200c-$200d - S-CPU vectors
	uint16_t m_irq_vector, m_nmi_vector;
	// $2012-$2015
	uint16_t m_hcount, m_vcount;
	// $2220-$2223
	int m_bank_c_hi, m_bank_c_rom;
	int m_bank_d_hi, m_bank_d_rom;
	int m_bank_e_hi, m_bank_e_rom;
	int m_bank_f_hi, m_bank_f_rom;
	// $2224-$2225 & $223f
	uint8_t m_bwram_snes, m_bwram_sa1;
	int m_bwram_sa1_source, m_bwram_sa1_format;
	// $2226-$2227
	int m_bwram_write_snes, m_bwram_write_sa1;
	// $2228
	uint32_t m_bwpa_sa1;
	// $2229-$222a
	uint8_t m_iram_write_snes, m_iram_write_sa1;
	// $2230-$2231
	uint8_t m_dma_ctrl, m_dma_ccparam;
	bool m_dma_cconv_end;
	uint8_t m_dma_cconv_size;
	uint8_t m_dma_cconv_bits;
	// $2232-$2237
	uint32_t m_src_addr, m_dst_addr;
	// $2238-$2239
	uint16_t m_dma_cnt;
	// $2240-$224f
	uint8_t m_brf_reg[0x10];
	// $2250-$2254
	uint8_t m_math_ctlr, m_math_overflow;
	uint16_t m_math_a, m_math_b;
	uint64_t m_math_res;
	// $2258-$225b
	uint32_t m_vda;
	uint8_t m_vbit, m_vlen;
	int m_drm;
	// $2300-$2301
	uint8_t m_scpu_flags, m_sa1_flags;
	// $2302-$2305
	uint16_t m_hcr, m_vcr;

	bool m_cconv1_dma_active;
	uint32_t m_cconv_bits;

	uint8_t sa1_lo_r(offs_t offset);
	uint8_t sa1_hi_r(offs_t offset);
	void sa1_lo_w(offs_t offset, uint8_t data);
	void sa1_hi_w(offs_t offset, uint8_t data);

	void sa1_map(address_map &map);

	uint8_t dma_cctype1_read(offs_t offset);
};


// device type definition
DECLARE_DEVICE_TYPE(SNS_LOROM_SA1, sns_sa1_device)

#endif // MAME_BUS_SNES_SA1_H
