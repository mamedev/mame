// license:GPL-2.0+
// copyright-holders:byuu
#ifndef MAME_CPU_SUPERFX_SUPERFX_H
#define MAME_CPU_SUPERFX_SUPERFX_H

#pragma once

#include "sfx_dasm.h"

enum
{
	SUPERFX_PC = 1,

	SUPERFX_DREG,
	SUPERFX_SREG,

	SUPERFX_R0,
	SUPERFX_R1,
	SUPERFX_R2,
	SUPERFX_R3,
	SUPERFX_R4,
	SUPERFX_R5,
	SUPERFX_R6,
	SUPERFX_R7,
	SUPERFX_R8,
	SUPERFX_R9,
	SUPERFX_R10,
	SUPERFX_R11,
	SUPERFX_R12,
	SUPERFX_R13,
	SUPERFX_R14,
	SUPERFX_R15,

	SUPERFX_PBR,
	SUPERFX_SFR,
	SUPERFX_ROMBR,
	SUPERFX_RAMBR,
	SUPERFX_CBR,
	SUPERFX_SCBR,
	SUPERFX_SCMR,
	SUPERFX_COLR,
	SUPERFX_POR,
	SUPERFX_BRAMR,
	SUPERFX_VCR,
	SUPERFX_CFGR,
	SUPERFX_CLSR,

	SUPERFX_ROMCL,
	SUPERFX_ROMDR,

	SUPERFX_RAMCL,
	SUPERFX_RAMAR,
	SUPERFX_RAMDR,
	SUPERFX_RAMADDR
};

#define SUPERFX_SFR_IRQ     0x8000  // Interrupt Flag
#define SUPERFX_SFR_B       0x1000  // WITH Flag
#define SUPERFX_SFR_IH      0x0800  // Immediate Higher 8-bit Flag
#define SUPERFX_SFR_IL      0x0400  // Immediate Lower 8-bit Flag
#define SUPERFX_SFR_ALT     0x0300  // ALT Mode, both bits
#define SUPERFX_SFR_ALT0    0x0000  // ALT Mode, no bits
#define SUPERFX_SFR_ALT1    0x0100  // ALT Mode, bit 0
#define SUPERFX_SFR_ALT2    0x0200  // ALT Mode, bit 1
#define SUPERFX_SFR_ALT3    0x0300  // ALT Mode, both bits (convenience dupe)
#define SUPERFX_SFR_R       0x0040  // ROM R14 Read Flag
#define SUPERFX_SFR_G       0x0020  // GO Flag
#define SUPERFX_SFR_OV      0x0010  // Overflow Flag
#define SUPERFX_SFR_S       0x0008  // Sign Flag
#define SUPERFX_SFR_CY      0x0004  // Carry Flag
#define SUPERFX_SFR_Z       0x0002  // Zero Flag

#define SUPERFX_POR_OBJ         0x10
#define SUPERFX_POR_FREEZEHIGH  0x08
#define SUPERFX_POR_HIGHNIBBLE  0x04
#define SUPERFX_POR_DITHER      0x02
#define SUPERFX_POR_TRANSPARENT 0x01

#define SUPERFX_SCMR_HT_MASK    0x24
#define SUPERFX_SCMR_HT0        0x00
#define SUPERFX_SCMR_HT1        0x04
#define SUPERFX_SCMR_HT2        0x20
#define SUPERFX_SCMR_HT3        0x24
#define SUPERFX_SCMR_RON        0x10
#define SUPERFX_SCMR_RAN        0x08
#define SUPERFX_SCMR_MD         0x03

#define SUPERFX_CFGR_IRQ    0x80    // IRQ
#define SUPERFX_CFGR_MS0    0x20    // MS0


class superfx_device :  public cpu_device, public superfx_disassembler::config
{
public:
	// configuration helpers
	auto irq() { return m_out_irq_func.bind(); }

	uint8_t mmio_read(uint32_t addr);
	void mmio_write(uint32_t addr, uint8_t data);
	void add_clocks(int32_t clocks);
	int access_ram();
	int access_rom();

	virtual u16 get_alt() const override;

protected:
	// construction/destruction
	superfx_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual void execute_run() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	address_space_config m_program_config;

	devcb_write_line   m_out_irq_func;

	uint8_t  m_pipeline;
	uint16_t m_ramaddr; // RAM Address

	uint16_t m_r[16];   // GPRs
	uint16_t m_sfr;     // Status Flag Register
	uint8_t  m_pbr;     // Program Bank Register
	uint8_t  m_rombr;   // Game Pack ROM Bank Register
	uint8_t  m_rambr;   // Game Pack RAM Bank Register
	uint16_t m_cbr;     // Cache Base Register
	uint8_t  m_scbr;    // Screen Base Register
	uint8_t  m_scmr;    // Screen Mode Register
	uint8_t  m_colr;    // Color Register
	uint8_t  m_por;     // Plot Option Register
	uint8_t  m_bramr;   // Back-Up RAM Register
	uint8_t  m_vcr;     // Version Code Register
	uint8_t  m_cfgr;    // Config Register
	uint8_t  m_clsr;    // Clock Select Register

	uint32_t m_romcl;   // Clock ticks until ROMDR is valid
	uint8_t  m_romdr;   // ROM Buffer Data Register

	uint32_t m_ramcl;   // Clock ticks until RAMDR is valid;
	uint16_t m_ramar;   // RAM Buffer Address Register
	uint8_t  m_ramdr;   // RAM Buffer Data Register

	uint16_t *m_sreg;   // Source Register (From)
	uint8_t  m_sreg_idx;// Source Register (To), index
	uint16_t *m_dreg;   // Destination Register (To)
	uint8_t  m_dreg_idx;// Destination Register (To), index
	uint8_t  m_r15_modified;

	uint8_t  m_irq;     // IRQ Pending

	uint32_t m_cache_access_speed;
	uint32_t m_memory_access_speed;

	struct {
		uint8_t buffer[0x200];
		uint8_t valid[0x20];
	} m_cache;
	struct {
		uint16_t offset;
		uint8_t bitpend;
		uint8_t data[8];
	} m_pixelcache[2];

	address_space *m_program;
	int m_icount;

	uint32_t m_debugger_temp;

	inline void superfx_regs_reset();
	void superfx_update_speed();
	void superfx_cache_flush();
	uint8_t superfx_cache_mmio_read(uint32_t addr);
	void superfx_cache_mmio_write(uint32_t addr, uint8_t data);
	void superfx_memory_reset();
	inline uint8_t superfx_bus_read(uint32_t addr);
	inline void superfx_bus_write(uint32_t addr, uint8_t data);
	inline void superfx_pixelcache_flush(int32_t line);
	inline void superfx_plot(uint8_t x, uint8_t y);
	uint8_t superfx_rpix(uint8_t x, uint8_t y);
	inline uint8_t superfx_color(uint8_t source);
	inline void superfx_rambuffer_sync();
	inline uint8_t superfx_rambuffer_read(uint16_t addr);
	inline void superfx_rambuffer_write(uint16_t addr, uint8_t data);
	inline void superfx_rombuffer_sync();
	inline void superfx_rombuffer_update();
	inline uint8_t superfx_rombuffer_read();
	inline void superfx_gpr_write(uint8_t r, uint16_t data);
	inline uint8_t superfx_op_read(uint16_t addr);
	inline uint8_t superfx_peekpipe();
	inline uint8_t superfx_pipe();
	inline void superfx_add_clocks_internal(uint32_t clocks);
	void superfx_timing_reset();
	inline void superfx_dreg_sfr_sz_update();
};

class superfx1_device :  public superfx_device
{
public:
	// construction/destruction
	superfx1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_execute_interface overrides
	virtual uint64_t execute_clocks_to_cycles(uint64_t clocks) const noexcept override { return (clocks + 2 - 1) / 2; }
	virtual uint64_t execute_cycles_to_clocks(uint64_t cycles) const noexcept override { return (cycles * 2); }
};

class superfx2_device :  public superfx_device
{
public:
	// construction/destruction
	superfx2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(SUPERFX1, superfx1_device)
DECLARE_DEVICE_TYPE(SUPERFX2, superfx2_device)

#endif // MAME_CPU_SUPERFX_SUPERFX_H
