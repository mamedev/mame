// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
#ifndef __SUPERFX_H__
#define __SUPERFX_H__


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


#define MCFG_SUPERFX_OUT_IRQ(_devcb) \
	superfx_device::set_out_irq_func(*device, DEVCB_##_devcb);


class superfx_device :  public cpu_device
{
public:
	// construction/destruction
	superfx_device(const machine_config &mconfig, std::string _tag, device_t *_owner, UINT32 _clock);

	// static configuration helpers
	template<class _Object> static devcb_base &set_out_irq_func(device_t &device, _Object object) { return downcast<superfx_device &>(device).m_out_irq_func.set_callback(object); }

	UINT8 mmio_read(UINT32 addr);
	void mmio_write(UINT32 addr, UINT8 data);
	void add_clocks(INT32 clocks);
	int access_ram();
	int access_rom();

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override { return 1; }
	virtual UINT32 execute_max_cycles() const override { return 1; }
	virtual UINT32 execute_input_lines() const override { return 0; }
	virtual void execute_run() override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override { return (spacenum == AS_PROGRAM) ? &m_program_config : nullptr; }

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override { return 1; }
	virtual UINT32 disasm_max_opcode_bytes() const override { return 3; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

private:
	address_space_config m_program_config;

	devcb_write_line   m_out_irq_func;

	UINT8  m_pipeline;
	UINT16 m_ramaddr; // RAM Address

	UINT16 m_r[16];   // GPRs
	UINT16 m_sfr;     // Status Flag Register
	UINT8  m_pbr;     // Program Bank Register
	UINT8  m_rombr;   // Game Pack ROM Bank Register
	UINT8  m_rambr;   // Game Pack RAM Bank Register
	UINT16 m_cbr;     // Cache Base Register
	UINT8  m_scbr;    // Screen Base Register
	UINT8  m_scmr;    // Screen Mode Register
	UINT8  m_colr;    // Color Register
	UINT8  m_por;     // Plot Option Register
	UINT8  m_bramr;   // Back-Up RAM Register
	UINT8  m_vcr;     // Version Code Register
	UINT8  m_cfgr;    // Config Register
	UINT8  m_clsr;    // Clock Select Register

	UINT32 m_romcl;   // Clock ticks until ROMDR is valid
	UINT8  m_romdr;   // ROM Buffer Data Register

	UINT32 m_ramcl;   // Clock ticks until RAMDR is valid;
	UINT16 m_ramar;   // RAM Buffer Address Register
	UINT8  m_ramdr;   // RAM Buffer Data Register

	UINT16 *m_sreg;   // Source Register (From)
	UINT8  m_sreg_idx;// Source Register (To), index
	UINT16 *m_dreg;   // Destination Register (To)
	UINT8  m_dreg_idx;// Destination Register (To), index
	UINT8  m_r15_modified;

	UINT8  m_irq;     // IRQ Pending

	UINT32 m_cache_access_speed;
	UINT32 m_memory_access_speed;

	struct {
		UINT8 buffer[0x200];
		UINT8 valid[0x20];
	} m_cache;
	struct {
		UINT16 offset;
		UINT8 bitpend;
		UINT8 data[8];
	} m_pixelcache[2];

	address_space *m_program;
	int m_icount;

	UINT32 m_debugger_temp;

	inline void superfx_regs_reset();
	void superfx_update_speed();
	void superfx_cache_flush();
	UINT8 superfx_cache_mmio_read(UINT32 addr);
	void superfx_cache_mmio_write(UINT32 addr, UINT8 data);
	void superfx_memory_reset();
	inline UINT8 superfx_bus_read(UINT32 addr);
	inline void superfx_bus_write(UINT32 addr, UINT8 data);
	inline void superfx_pixelcache_flush(INT32 line);
	inline void superfx_plot(UINT8 x, UINT8 y);
	UINT8 superfx_rpix(UINT8 x, UINT8 y);
	inline UINT8 superfx_color(UINT8 source);
	inline void superfx_rambuffer_sync();
	inline UINT8 superfx_rambuffer_read(UINT16 addr);
	inline void superfx_rambuffer_write(UINT16 addr, UINT8 data);
	inline void superfx_rombuffer_sync();
	inline void superfx_rombuffer_update();
	inline UINT8 superfx_rombuffer_read();
	inline void superfx_gpr_write(UINT8 r, UINT16 data);
	inline UINT8 superfx_op_read(UINT16 addr);
	inline UINT8 superfx_peekpipe();
	inline UINT8 superfx_pipe();
	inline void superfx_add_clocks_internal(UINT32 clocks);
	void superfx_timing_reset();
	inline void superfx_dreg_sfr_sz_update();
};


extern const device_type SUPERFX;

#endif /* __SUPERFX_H__ */
