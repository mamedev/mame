// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_CPU_ALPHA_ALPHA_H
#define MAME_CPU_ALPHA_ALPHA_H

#pragma once

#include "alphad.h"

class alpha_device : public cpu_device
{
public:
	void set_dasm_type(alpha_disassembler::dasm_type type) { m_dasm_type = type; }

	// input/output lines
	auto srom_oe_w() { return m_srom_oe_cb.bind(); }
	auto srom_data_r() { return m_srom_data_cb.bind(); }

protected:
	alpha_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual u32 execute_min_cycles() const noexcept override { return 1; }
	virtual u32 execute_max_cycles() const noexcept override { return 1; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;
	virtual bool memory_translate(int spacenum, int intention, offs_t &address, address_space *&target_space) override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual void cpu_execute(u32 const op);
	virtual bool cpu_translate(u64 &address, int intention) { return false; }

	// execution helpers
	static u64 zap_mask(u8 const zap_bits);
	static u64 u64_to_g_floating(u64 const data);
	static u64 u32_to_f_floating(u32 const data);
	static u32 f_floating_to_u32(u64 const data);

	// memory access helpers
	template <typename T, typename U> std::enable_if_t<std::is_convertible<U, std::function<void(T)>>::value, void> load(u64 address, U &&apply);
	template <typename T, typename U> std::enable_if_t<std::is_convertible<U, std::function<void(address_space &, u64, T)>>::value, void> load_l(u64 address, U &&apply);
	template <typename T, typename U> std::enable_if_t<std::is_convertible<U, T>::value, void> store(u64 address, U data, T mem_mask = ~T(0));
	void fetch(u64 address, std::function<void(u32)> &&apply);

	// cache helpers
	u32 read_srom(unsigned const bits);
	virtual u32 icache_fetch(u64 const address) = 0;

	// configuration
	alpha_disassembler::dasm_type m_dasm_type;
	address_space_config m_as_config[4];
	devcb_write_line m_srom_oe_cb;
	devcb_read_line m_srom_data_cb;

	// emulation state
	int m_icount;

	u64 m_pc;
	u64 m_r[32];
	u64 m_f[32];

	bool m_pal_mode;
	memory_passthrough_handler *m_lock_watch;
};

class alpha_ev4_device : public alpha_device
{
public:
	alpha_ev4_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void cpu_execute(u32 const op) override;
	virtual bool cpu_translate(u64 &address, int intention) override;

	enum ibx_reg : unsigned
	{
		IBX_TB_TAG       = 0,  // w, PALmode only
		IBX_ITB_PTE      = 1,  // r/w, PALmode only
		IBX_ICCSR        = 2,  // r/w
		IBX_ITB_PTE_TEMP = 3,  // r, PALmode only
		IBX_EXC_ADDR     = 4,  // r/w
		IBX_SL_RCV       = 5,  // r
		IBX_ITBZAP       = 6,  // w, PALmode only
		IBX_ITBASM       = 7,  // w, PALmode only
		IBX_ITBIS        = 8,  // w, PALmode only
		IBX_PS           = 9,  // r/w
		IBX_EXC_SUM      = 10, // r/w
		IBX_PAL_BASE     = 11, // r/w
		IBX_HIRR         = 12, // r
		IBX_SIRR         = 13, // r/w
		IBX_ASTRR        = 14, // r/w
		IBX_HIER         = 16, // r/w
		IBX_SIER         = 17, // r/w
		IBX_ASTER        = 18, // r/w
		IBX_SL_CLR       = 19, // w
		IBX_SL_XMIT      = 22, // w
	};

	enum ibx_itb_pte_mask : u64
	{
		IBX_ITB_PTE_W_ASM = 0x00000000'00000010, // address space match
		IBX_ITB_PTE_W_KRE = 0x00000000'00000100, // kernel read-enable
		IBX_ITB_PTE_W_ERE = 0x00000000'00000200, // executive read-enable
		IBX_ITB_PTE_W_SRE = 0x00000000'00000400, // supervisor read-enable
		IBX_ITB_PTE_W_URE = 0x00000000'00000800, // user read-enable
		IBX_ITB_PTE_W_PFN = 0x001fffff'00000000, // page frame number

		IBX_ITB_PTE_R_KRE = 0x00000000'00000200, // kernel read-enable
		IBX_ITB_PTE_R_ERE = 0x00000000'00000400, // executive read-enable
		IBX_ITB_PTE_R_SRE = 0x00000000'00000800, // supervisor read-enable
		IBX_ITB_PTE_R_URE = 0x00000000'00001000, // user read-enable
		IBX_ITB_PTE_R_PFN = 0x00000003'ffffe000, // page frame number
		IBX_ITB_PTE_R_ASM = 0x00000004'00000000, // address space match
	};
	enum ibx_iccsr_mask : u64
	{
		IBX_ICCSR_W_PC1    = 0x00000000'00000001, // performance counter 1 interrupt enable
		IBX_ICCSR_W_PC0    = 0x00000000'00000008, // performance counter 0 interrupt enable
		IBX_ICCSR_W_PCMUX0 = 0x00000000'00000f00,
		IBX_ICCSR_W_PCMUX1 = 0x00000007'00000000,
		IBX_ICCSR_W_PIPE   = 0x00000008'00000000, // pipeline enable
		IBX_ICCSR_W_BPE    = 0x00000010'00000000,
		IBX_ICCSR_W_JSE    = 0x00000020'00000000, // jsr stack enable
		IBX_ICCSR_W_BHE    = 0x00000040'00000000,
		IBX_ICCSR_W_DI     = 0x00000080'00000000, // dual issue enable
		IBX_ICCSR_W_HWE    = 0x00000100'00000000, // hardware mode enable
		IBX_ICCSR_W_MAP    = 0x00000200'00000000, // i-stream superpage enable
		IBX_ICCSR_W_FPE    = 0x00000400'00000000, // floating-point enable
		IBX_ICCSR_W_PCE    = 0x00003000'00000000, // performance counter enable
		IBX_ICCSR_W_ASN    = 0x001f8000'00000000, // address space number

		IBX_ICCSR_R_PC0    = 0x00000000'00000002, // performance counter 0 interrupt enable
		IBX_ICCSR_R_PC1    = 0x00000000'00000004, // performance counter 1 interrupt enable
		IBX_ICCSR_R_PCMUX0 = 0x00000000'00001e00,
		IBX_ICCSR_R_PCMUX1 = 0x00000000'0000e000,
		IBX_ICCSR_R_PIPE   = 0x00000000'00010000, // pipeline enable
		IBX_ICCSR_R_BPE    = 0x00000000'00020000,
		IBX_ICCSR_R_JSE    = 0x00000000'00040000, // jsr stack enable
		IBX_ICCSR_R_BHE    = 0x00000000'00080000,
		IBX_ICCSR_R_DI     = 0x00000000'00100000, // dual issue enable
		IBX_ICCSR_R_HWE    = 0x00000000'00200000, // hardware mode enable
		IBX_ICCSR_R_MAP    = 0x00000000'00400000, // i-stream superpage enable
		IBX_ICCSR_R_FPE    = 0x00000000'00800000, // floating-point enable
		IBX_ICCSR_R_ASN    = 0x00000003'f0000000, // address space number
		IBX_ICCSR_R_PCE    = 0x00003000'00000000, // performance counter enable

		IBX_ICCSR_W_GRP1   = 0x000007ff'00000000,
	};
	enum ibx_ps_mask : u64
	{
		IBX_PS_R_CM0 = 0x00000000'00000001,
		IBX_PS_R_CM1 = 0x00000004'00000000,
		IBX_PS_R_CM  = 0x00000004'00000001,

		IBX_PS_W_CM0 = 0x00000000'00000008,
		IBX_PS_W_CM1 = 0x00000000'00000010,
	};
	enum ibx_pal_base_mask : u64
	{
		IBX_PAL_BASE_W = 0x00000003'ffffc000,
	};

	enum abx_reg : unsigned
	{
		ABX_TB_CTL        = 0,  // w
		ABX_DTB_PTE       = 2,  // r/w
		ABX_DTB_PTE_TEMP  = 3,  // r
		ABX_MM_CSR        = 4,  // r
		ABX_VA            = 5,  // r
		ABX_DTBZAP        = 6,  // w
		ABX_DTBASM        = 7,  // w
		ABX_DTBIS         = 8,  // w
		ABX_BIU_ADDR      = 9,  // r
		ABX_BIU_STAT      = 10, // r
		ABX_DC_STAT       = 12, // r
		ABX_FILL_ADDR     = 13, // r
		ABX_ABOX_CTL      = 14, // w
		ABX_ALT_MODE      = 15, // w
		ABX_CC            = 16, // w
		ABX_CC_CTL        = 17, // w
		ABX_BIU_CTL       = 18, // w
		ABX_FILL_SYNDROME = 19, // w
		ABX_BC_TAG        = 20, // w
		ABX_FLUSH_IC      = 21, // w
		ABX_FLUSH_IC_ASM  = 23, // w
	};

	enum abx_abox_ctl_mask : u64
	{
		ABX_ABOX_CTL_WB_DIS          = 0x0001, // write buffer unload disable
		ABX_ABOX_CTL_MCHK_EN         = 0x0002, // machine check enable
		ABX_ABOX_CTL_CRD_EN          = 0x0004, // corrected read data interrupt enable
		ABX_ABOX_CTL_IC_SBUF_EN      = 0x0008, // icache stream buffer enable
		ABX_ABOX_CTL_SPE_1           = 0x0010, // super page enable 1
		ABX_ABOX_CTL_SPE_2           = 0x0020, // super page enable 2
		ABX_ABOX_CTL_EMD_EN          = 0x0040, // big-endian mode enable
		ABX_ABOX_CTL_STC_NORESULT    = 0x0080, //
		ABX_ABOX_CTL_NCACHE_NDISTURB = 0x0100, //
		ABX_ABOX_CTL_DTB_RR          = 0x0200, // dtb round-robin replacement
		ABX_ABOX_CTL_DC_ENA          = 0x0400, // dcache enable
		ABX_ABOX_CTL_DC_FHIT         = 0x0800, // dcache force hit
		ABX_ABOX_CTL_DC_16K          = 0x1000, // select 16K dcache (21064A only)
		ABX_ABOX_CTL_F_TAG_ERR       = 0x2000, // generate bad dcache tag parity (21064A only)
		ABX_ABOX_CTL_NOCHK_PAR       = 0x4000, // disable cache parity checks (21064A only)
		ABX_ABOX_CTL_DOUBLE_INVAL    = 0x8000, // (21064A only)
	};

	enum palcode_entry : u16
	{
		RESET           = 0x0000,
		MCHK            = 0x0020,
		ARITH           = 0x0060,
		INTERRUPT       = 0x00e0,
		D_FAULT         = 0x01e0,
		ITB_MISS        = 0x03e0,
		ITB_ACV         = 0x07e0,
		DTB_MISS_NATIVE = 0x08e0,
		DTB_MISS_PAL    = 0x09e0,
		UNALIGN         = 0x11e0,
		OPCDEC          = 0x13e0,
		FEN             = 0x17e0,
		CALL_PAL        = 0x2000,
	};
	static constexpr u32 CALL_PAL_MASK = 0x03ffff40;

	u64 ibx_get(u8 reg);
	void ibx_set(u8 reg, u64 data);
	u64 abx_get(u8 reg);
	void abx_set(u8 reg, u64 data);

	u64 m_ibx[32];
	u64 m_abx[32];
	u64 m_pt[32];
};

class dec_21064_device : public alpha_ev4_device
{
public:
	dec_21064_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_reset() override ATTR_COLD;

	virtual u32 icache_fetch(u64 const address) override;

private:
	struct icache_block
	{
		u32 lw[8]; // instruction longwords
		u32 tag;   // cache tag
		u8 aav;    // asn, asm and v fields
		u8 bht;    // branch history table
	};

	enum aav_mask : u8
	{
		AAV_ASN = 0x3f, // address space number
		AAV_ASM = 0x40, // address space match
		AAV_V   = 0x80, // valid
	};

	icache_block m_icache[256];
};

DECLARE_DEVICE_TYPE(DEC_21064, dec_21064_device)

#endif // MAME_CPU_ALPHA_ALPHA_H
