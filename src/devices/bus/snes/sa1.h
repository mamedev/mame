// license:BSD-3-Clause
// copyright-holders:Fabio Priuli, R. Belmont
#ifndef __SNS_SA1_H
#define __SNS_SA1_H

#include "snes_slot.h"
#include "cpu/g65816/g65816.h"


// ======================> sns_sa1_device

class sns_sa1_device : public device_t,
						public device_sns_cart_interface
{
public:
	// construction/destruction
	sns_sa1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_reset() override;

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_l) override;
	virtual DECLARE_READ8_MEMBER(read_h) override;
	virtual DECLARE_WRITE8_MEMBER(write_l) override;
	virtual DECLARE_WRITE8_MEMBER(write_h) override;

	// additional reading and writing
	virtual DECLARE_READ8_MEMBER(chip_read) override;
	virtual DECLARE_WRITE8_MEMBER(chip_write) override;

	DECLARE_READ8_MEMBER(sa1_lo_r);
	DECLARE_READ8_MEMBER(sa1_hi_r);
	DECLARE_WRITE8_MEMBER(sa1_lo_w);
	DECLARE_WRITE8_MEMBER(sa1_hi_w);

private:

	UINT8 var_length_read(address_space &space, UINT32 offset);
	void dma_transfer(address_space &space);
	void dma_cctype1_transfer(address_space &space);
	void dma_cctype2_transfer(address_space &space);

	UINT8 read_regs(address_space &space, UINT32 offset);
	UINT8 read_iram(UINT32 offset);
	UINT8 read_bwram(UINT32 offset);
	void write_regs(address_space &space, UINT32 offset, UINT8 data);
	void write_iram(UINT32 offset, UINT8 data);
	void write_bwram(UINT32 offset, UINT8 data);
	void recalc_irqs();

	required_device<g65816_device> m_sa1;

	UINT8 m_internal_ram[0x800];

	// register related
	// $2200
	UINT8 m_sa1_ctrl;
	// $2201
	UINT8 m_scpu_sie;
	// $2203-$2208
	UINT16 m_sa1_reset, m_sa1_nmi, m_sa1_irq;
	// $2209
	UINT8 m_scpu_ctrl;
	// $220a
	UINT8 m_sa1_sie;
	// $200c-$200d - S-CPU vectors
	UINT16 m_irq_vector, m_nmi_vector;
	// $2012-$2015
	UINT16 m_hcount, m_vcount;
	// $2220-$2223
	int m_bank_c_hi, m_bank_c_rom;
	int m_bank_d_hi, m_bank_d_rom;
	int m_bank_e_hi, m_bank_e_rom;
	int m_bank_f_hi, m_bank_f_rom;
	// $2224-$2225 & $223f
	UINT8 m_bwram_snes, m_bwram_sa1;
	int m_bwram_sa1_source, m_bwram_sa1_format;
	// $2226-$2227
	int m_bwram_write_snes, m_bwram_write_sa1;
	// $2228
	UINT32 m_bwpa_sa1;
	// $2229-$222a
	UINT8 m_iram_write_snes, m_iram_write_sa1;
	// $2230-$2231
	UINT8 m_dma_ctrl, m_dma_ccparam;
	// $2232-$2237
	UINT32 m_src_addr, m_dst_addr;
	// $2238-$2239
	UINT16 m_dma_cnt;
	// $2240-$224f
	UINT8 m_brf_reg[0x10];
	// $2250-$2254
	UINT8 m_math_ctlr, m_math_overflow;
	UINT16 m_math_a, m_math_b;
	UINT64 m_math_res;
	// $2258-$225b
	UINT32 m_vda;
	UINT8 m_vbit, m_vlen;
	int m_drm;
	// $2300-$2301
	UINT8 m_scpu_flags, m_sa1_flags;
	// $2302-$2305
	UINT16 m_hcr, m_vcr;
};


// device type definition
extern const device_type SNS_LOROM_SA1;

#endif
