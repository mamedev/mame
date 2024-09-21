// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub

#ifndef MAME_CPU_MPK1839_KL1839VM1_H
#define MAME_CPU_MPK1839_KL1839VM1_H

#pragma once


enum
{
	KL1839_MCA = STATE_GENPC, KL1839_VMA, KL1839_IF, KL1839_RSP, KL1839_PCM, KL1839_RC, KL1839_RV, KL1839_SCH,
	VAX_R0, VAX_R1, VAX_R2, VAX_R3, VAX_R4, VAX_R5, VAX_R6, VAX_R7, VAX_R8, VAX_R9, VAX_RA, VAX_RB,
	VAX_AP, VAX_FP, VAX_SP, VAX_PC,
	VAX_AK0, VAX_AK1, VAX_AK2, VAX_AK3, VAX_AK4, VAX_AK5, VAX_AK6, VAX_AK7, VAX_AK8,
	VAX_RNK, VAX_RKA, VAX_PSL, VAX_BO
};


class kl1839vm1_device :  public cpu_device
{
public:
	// construction/destruction
	kl1839vm1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual u32 execute_min_cycles() const noexcept override { return 1; }
	virtual u32 execute_max_cycles() const noexcept override { return 16; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	void flag(u32 op);
	void rest(u32 op);
	void ma(u32 op);
	void mb(u32 op);
	void mc(u32 op);
	void mk(u32 op);
	void yp(u32 op);
	void zsch(u32 op);
	void psch(u32 op);
	void rts(u32 op);
	void acc(u32 op);
	void chka(u32 op);
	void chlk(u32 op);
	void srf(u32 op);
	void invalid(u32 op);

	u32 shr(u32 val, bool va, u8 fo, bool a_c, bool l_r);
	void kob_process(u8 no, u8 fd, u8 kob, u32 data);
	void kop(u8 kop, u8 fd, u32 x, u32 y, u32 &z, u8 ps, bool va, u8 fo);
	void mreg_w();
	void mreg_r();
	void decode_op(u32 op);

	address_space_config m_microcode_config;
	address_space_config m_sysram_config;
	address_space_config m_ram_config;
	address_space_config m_io_config;

	memory_access<14, 2, -2, ENDIANNESS_BIG>::cache m_microcode;
	memory_access<24, 0, 0, ENDIANNESS_BIG>::specific m_sysram;
	memory_access<24, 0, 0, ENDIANNESS_LITTLE>::specific m_ram;
	memory_access<6, 2, -2, ENDIANNESS_LITTLE>::specific m_io;
	PAIR                m_mca;
	PAIR                m_vma;
	PAIR                m_vma_tmp; // does we have int reg for this?
	PAIR                m_rv;
	PAIR                m_sch;
	PAIR                m_rsp;
	PAIR                m_ppc;    // previous program counter
	bool                m_fp;
	u32                 m_consts[0x10] = { 0x4, 0x2, 0x8, 0x1, 0x0, 0, 0, 0x66, 0, 0xc00000, 0xffffffff, 0x1f0000, 0x4000000, 0, 0, 0 };
	PAIR                m_reg[0x20];
	int                 m_icount;
};

DECLARE_DEVICE_TYPE(KL1839VM1, kl1839vm1_device)

#endif // MAME_CPU_MPK1839_KL1839VM1_H
