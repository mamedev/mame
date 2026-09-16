// license:BSD-3-Clause
// copyright-holders:Fabio Priuli, R. Belmont
#ifndef MAME_BUS_SNES_SA1_H
#define MAME_BUS_SNES_SA1_H

#pragma once

#include "snes_slot.h"
#include "cpu/g65816/g65816.h"
#include <algorithm>


// ======================> sns_sa1_device

class sns_sa1_device : public device_t,
						public device_sns_cart_interface
{
public:
	// construction/destruction
	sns_sa1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// reading and writing
	virtual u8 read_l(offs_t offset) override;
	virtual u8 read_h(offs_t offset) override;
	virtual void write_l(offs_t offset, u8 data) override;
	virtual void write_h(offs_t offset, u8 data) override;

	// additional reading and writing
	virtual u8 chip_read(offs_t offset) override;
	virtual void chip_write(offs_t offset, u8 data) override;

	TIMER_CALLBACK_MEMBER(timer_tick);

private:
	u8 var_length_read(offs_t offset);
	void dma_transfer();
	void dma_cctype1_transfer();
	void dma_cctype2_transfer();

	u8 host_r(offs_t offset);
	void host_w(offs_t offset, u8 data);
	void shared_regs_w(offs_t offset, u8 data);

	u8 read_regs(offs_t offset);
	u8 read_iram(offs_t offset);
	template<bool SA1Read> u8 read_bwram(offs_t offset, bool bitmap = false);
	u8 read_cconv1_dma(offs_t offset);
	void write_regs(offs_t offset, u8 data);
	void write_iram(offs_t offset, u8 data);
	void write_bwram(offs_t offset, u8 data, bool bitmap = false);
	void recalc_irqs();

	u8 rom_r(offs_t offset);
	u8 sa1_rom_r(offs_t offset);

	u8 sa1_iram_r(offs_t offset);
	void sa1_iram_w(offs_t offset, u8 data);

	u8 sa1_bwram_r(offs_t offset, bool bitmap = false);
	void sa1_bwram_w(offs_t offset, u8 data, bool bitmap = false);

	required_device<g65816_device> m_sa1;
	emu_timer *m_sa1_timer;

	std::unique_ptr<u8[]> m_internal_ram;

	// bus conflict related:
	// ROM: [00-3f][8000-ffff], [80-bf][8000-ffff], [c0-ff][0000-ffff]
	// IRAM: [00-3f][3000-38ff], [80-bf][3000-38ff]
	// BWRAM: [00-3f][6000-7fff], [40-4f][0000-ffff], [80-bf][6000-7fff]
	inline const bool bus_conflict_rom() { return ((address_r() & 0x408000) == 0x008000) || ((address_r() & 0xc00000) == 0xc00000); }
	inline const bool bus_conflict_iram() { return ((address_r() & 0x40f800) == 0x003000); }
	inline const bool bus_conflict_bwram() { return ((address_r() & 0x40e000) == 0x006000) || ((address_r() & 0xf00000) == 0x400000); }

	// register related
	// $2200
	u8 m_sa1_ctrl;
	bool m_sa1_reset_flag;
	// SA-1 CPU Control (CCNT)
	inline const bool CCNT_SA1_CPU_IRQ()  { return BIT(m_sa1_ctrl, 7); }    // SA-1 CPU IRQ (from SNES CPU)
	inline const bool CCNT_SA1_CPU_RDYB() { return BIT(m_sa1_ctrl, 6); }    // SA-1 CPU ready
	inline const bool CCNT_SA1_CPU_RESB() { return BIT(m_sa1_ctrl, 5); }    // SA-1 CPU reset
	inline const bool CCNT_SA1_CPU_NMI()  { return BIT(m_sa1_ctrl, 4); }    // SA-1 CPU NMI (from SNES CPU)
	inline const u8 CCNT_SMEG()           { return BIT(m_sa1_ctrl, 0, 4); } // Message from SNES CPU to SA-1 CPU
	inline const bool sa1_halted()        { return (CCNT_SA1_CPU_RDYB() || CCNT_SA1_CPU_RESB()); } // SA-1 is halted?
	// $2201
	u8 m_scpu_sie;
	// $2203-$2208
	u16 m_sa1_reset_vector, m_sa1_nmi_vector, m_sa1_irq_vector;
	// $2209
	u8 m_scpu_ctrl;
	// SNES CPU Control (SCNT)
	inline const bool SCNT_SNESCPU_IRQ()  { return BIT(m_scpu_ctrl, 7); }    // IRQ from SA-1 CPU to SNES CPU
	inline const bool SCNT_SNESCPU_IVSW() { return BIT(m_scpu_ctrl, 6); }    // SNES CPU IRQ vector selection (ROM, IRQ vector register)
	inline const bool SCNT_SNESCPU_NVSW() { return BIT(m_scpu_ctrl, 4); }    // SNES CPU NMI vector selection (ROM, NMI vector register)
	inline const u8 SCNT_CMEG()           { return BIT(m_scpu_ctrl, 0, 4); } // Message from SA-1 CPU to SNES CPU
	// $220a
	u8 m_sa1_sie;
	// $220c-$220d - S-CPU vectors
	u16 m_irq_vector, m_nmi_vector;
	// $2210-$2211
	u8 m_timer_ctrl;
	// H/V Timer control (TMC)
	inline const bool TMC_HVSELB() { return BIT(m_timer_ctrl, 7); } // Select HV timer (HV timer, Linear timer)
	inline const bool TMC_VEN()    { return BIT(m_timer_ctrl, 1); } // V count enable
	inline const bool TMC_HEN()    { return BIT(m_timer_ctrl, 0); } // H count enable
	// H/V Timer position
	u16 m_hpos, m_vpos;
	// $2212-$2215
	u16 m_hcount, m_vcount;
	// $2220-$2223
	/*
	enum super_mmc_t
	{
	    CXB = 0,
	    DXB = 1,
	    EXB = 2,
	    FXB = 3
	};
	*/
	bool m_bank_hi[4];
	u8 m_bank_rom[4];
	// $2224-$2225 & $223f
	u8 m_bwram_snes, m_bwram_sa1;
	bool m_bwram_sa1_source, m_bwram_sa1_format;
	// $2226-$2227
	bool m_bwram_write_snes, m_bwram_write_sa1;
	// $2228
	u32 m_bwpa_sa1;
	// $2229-$222a
	u8 m_iram_write_snes, m_iram_write_sa1;
	// $2230-$2231
	u8 m_dma_ctrl, m_dma_ccparam;
	u8 m_dma_cconv_size;
	u8 m_dma_cconv_bits;
	// DMA Control (DCNT)
	inline const bool DCNT_DMAEN()   { return BIT(m_dma_ctrl, 7); }                        // DMA Enable control
	//inline const bool DCNT_DPRIO() { return BIT(m_dma_ctrl, 6); }                        // Processing priority between SA-1 CPU and DMA; Not emulated currently
	inline const bool DCNT_CDEN()    { return BIT(m_dma_ctrl, 5); }                        // DMA mode selection (Normal/Character conversion)
	inline const bool DCNT_CDSEL()   { return BIT(m_dma_ctrl, 4); }                        // Character conversion DMA type
	inline const bool DCNT_DD()      { return BIT(m_dma_ctrl, 2); }                        // Destination device (IRAM, BWRAM)
	inline const u8 DCNT_SD()        { return BIT(m_dma_ctrl, 0, 1); }                     // Source device (ROM, IRAM, BWRAM)
	// Character conversion DMA parameters (CDMA)
	inline const bool CDMA_CHDEND()  { return BIT(m_dma_ccparam, 7); }                     // End character conversion 1
	inline const u8 CDMA_SIZE()      { return std::min<u8>(5, BIT(m_dma_ccparam, 2, 3)); }  // Number of virtual VRAM horizontal characters (1 << SIZE)
	inline const u8 CDMA_CB()        { return std::min<u8>(2, BIT(m_dma_ccparam, 0, 2)); }  // Character conversion DMA color mode (8bpp, 4bpp, 2bpp)
	// $2232-$2237
	u32 m_src_addr, m_dst_addr;
	// $2238-$2239
	u16 m_dma_cnt;
	// $2240-$224f
	u8 m_brf_reg[0x10];
	// $2250-$2254
	u8 m_math_ctlr, m_math_overflow;
	u16 m_math_a, m_math_b;
	u64 m_math_res;
	// $2258-$225b
	u32 m_vda;
	u8 m_vbit, m_vlen;
	bool m_drm;
	// $2300-$2301
	u8 m_scpu_flags, m_sa1_flags;
	// $2302-$2305
	u16 m_hcr, m_vcr;

	bool m_cconv1_dma_active;
	u8 m_cconv2_line;

	u8 sa1_lo_r(offs_t offset);
	u8 sa1_hi_r(offs_t offset);
	void sa1_lo_w(offs_t offset, u8 data);
	void sa1_hi_w(offs_t offset, u8 data);

	void sa1_map(address_map &map) ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(SNS_LOROM_SA1, sns_sa1_device)

#endif // MAME_BUS_SNES_SA1_H
