// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_MN1880_MN1880_H
#define MAME_CPU_MN1880_MN1880_H

#pragma once

class mn1880_device : public cpu_device
{
public:
	mn1880_device(const machine_config &config, const char *tag, device_t *owner, u32 clock);

	enum {
		MN1880_IP, MN1880_IPA, MN1880_IPB,
		MN1880_IR, MN1880_IRA, MN1880_IRB,
		MN1880_FS, MN1880_FSA, MN1880_FSB,
		MN1880_XP, MN1880_XPA, MN1880_XPB,
		MN1880_YP, MN1880_YPA, MN1880_YPB,
		MN1880_XPL, MN1880_XPH,
		MN1880_YPL, MN1880_YPH,
		MN1880_SP, MN1880_SPA, MN1880_SPB,
		MN1880_LP, MN1880_LPA, MN1880_LPB,
		MN1880_IE, MN1880_IEA, MN1880_IEB,
		MN1880_IEMASK, MN1880_IEMASKA, MN1880_IEMASKB,
		MN1880_DIVIDER1, MN1880_DIVIDER2,
		MN1880_IF,
		MN1880_CPUM
	};

protected:
	mn1880_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, bool has_mmu, address_map_constructor data_map);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual void execute_run() override;
	virtual uint64_t execute_clocks_to_cycles(uint64_t clocks) const noexcept override { return (clocks + 4 - 1) / 4; }
	virtual uint64_t execute_cycles_to_clocks(uint64_t cycles) const noexcept override { return (cycles * 4); }

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;
	virtual bool memory_translate(int spacenum, int intention, offs_t &address, address_space *&target_space) override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	void internal_data_map(address_map &map) ATTR_COLD;

private:
	struct cpu_registers
	{
		cpu_registers() : ip(0), irp(0), ir(0), fs(0), xp(0), yp(0), sp(0), lp(0), wait(0), ie(0), iemask(false) { }

		u8 addcz(u8 data1, u8 data2, bool carry, bool holdz);
		u8 adddcz(u8 data1, u8 data2, bool carry);
		u8 subcz(u8 data1, u8 data2, bool carry, bool holdz);
		u8 subdcz(u8 data1, u8 data2, bool carry);
		u8 rolc(u8 data);
		u8 rorc(u8 data);
		u8 asrc(u8 data);
		void branch(u16 label);

		u16 ip;
		u16 irp;
		u8 ir;
		u8 fs;
		u16 xp;
		u16 yp;
		u16 sp;
		u16 lp;
		u16 wait;
		u16 ie;
		bool iemask;
	};

	enum class microstate : u8 {
		NEXT,
		NOP_1,
		REP_1,
		CLRSET_1,
		T1B_1,
		MOVL31_1, MOVL31_2,
		MOVL34_1, MOVL34_2, MOVL34_3, MOVL34_4,
		MOVL35_1, MOVL35_2, MOVL35_3, MOVL35_4,
		MOV36_1, MOV36_2,
		MOV37_1,
		MOVL38_1, MOVL38_2, MOVL38_3,
		MOVL39_1, MOVL39_2, MOVL39_3,
		ASL_1, ASL_2,
		ASR_1, ASR_2,
		DEC40_1,
		NOT_1,
		CMPM44_1, CMPM44_2, CMPM44_3,
		CMPM45_1, CMPM45_2,
		XCH4_1, XCH4_2,
		INC48_1,
		CLR4A_1,
		ROL_1, ROL_2,
		ROR_1, ROR_2,
		CMPM50_1, CMPM50_2, CMPM50_3,
		CMPM52_1, CMPM52_2,
		DIV51_1, DIV51_2, DIV51_3, DIV51_4, DIV51_5, DIV51_6, DIV51_7, DIV51_8, DIV51_9, DIV51_10,
		MOVDA_1, MOVDA_2, MOVDA_3,
		MOV54_1,
		MOV55_1,
		MOV56_1, MOV56_2,
		XCH58_1, XCH58_2, XCH58_3, XCH58_4,
		MUL59_1, MUL59_2, MUL59_3, MUL59_4, MUL59_5, MUL59_6, MUL59_7, MUL59_8,
		MOVL5C_1, MOVL5C_2, MOVL5C_3,
		MOVL5D_1, MOVL5D_2,
		MOVL5E_1, MOVL5E_2, MOVL5E_3, MOVL5E_4,
		MOVL5F_1, MOVL5F_2,
		CMP_1, CMP_2, CMP_3,
		AND_1, AND_2, AND_3,
		XOR_1, XOR_2, XOR_3,
		OR_1, OR_2, OR_3,
		SUBC_1, SUBC_2, SUBC_3,
		SUBD_1, SUBD_2, SUBD_3, SUBD_4,
		ADDC_1, ADDC_2, ADDC_3,
		ADDD_1, ADDD_2, ADDD_3, ADDD_4,
		BR80_1, BR88_1,
		CMPL_1, CMPL_2, CMPL_3, CMPL_4, CMPL_5,
		CLRFS_1, SETFS_1,
		CALL90_1, CALL90_2,
		BRA0_1,
		POPFS_1, POPFS_2,
		POPB4_1, POPB4_2, POPB4_3,
		POPB5_1, POPB5_2,
		POPB7_1,
		PUSHFS_1,
		PUSHBC_1, PUSHBC_2, PUSHBC_3,
		PUSHBD_1, PUSHBD_2, PUSHBD_3,
		PUSHBF_1,
		SUBCL_1, SUBCL_2, SUBCL_3, SUBCL_4, SUBCL_5, SUBCL_6,
		DIVC1_1, DIVC1_2,
		XCHC4_1, XCHC4_2,
		XCHC5_1, XCHC5_2,
		ADDCL_1, ADDCL_2, ADDCL_3, ADDCL_4, ADDCL_5, ADDCL_6,
		MULC9_1, MULC9_2, MULC9_3,
		MOVCC_1,
		MOVCD_1, MOVCD_2,
		CMPD0_1, CMPD0_2,
		CMPD1_1, CMPD1_2,
		XCHD4_1, XCHD4_2,
		XCHD7_1,
		MOVD8_1,
		MOVD9_1,
		MOVDC_1,
		MOVDD_1,
		LOOP_1, LOOP_2,
		DECE4_1,
		INCE5_1,
		ADDRE8_1, ADDRE8_2, ADDRE8_3,
		ADDRE9_1, ADDRE9_2, ADDRE9_3,
		ADDREC_1,
		ADDRED_1,
		CMPBF0_1,
		CMPBF1_1, CMPBF1_2, CMPBF1_3,
		MOV1_1, MOV1_2, MOV1_3, MOV1_4, MOV1N_4,
		WAIT_1, WAIT_2,
		RET_1, RET_2, RET_3,
		RETI_1, RETI_2, RETI_3, RETI_4, RETI_5,
		BR_1, BR_2,
		CALL_1, CALL_2, CALL_3,
		PUSHFB_1,
		BRFC_1,
		CALLFD_1,
		RDTBL_1, RDTBL_2, RDTBL_3,
		PI_1, PI_2, PI_3, PI_4,
		UNKNOWN
	};

	static void setl(u16 &pr, u8 data) { pr = (pr & 0xff00) | data; }
	static void seth(u16 &pr, u8 data) { pr = (pr & 0x00ff) | (data << 8); }

	cpu_registers &get_active_cpu() { return m_cpu[BIT(m_cpum, 4)]; }
	const cpu_registers &get_active_cpu() const { return m_cpu[BIT(m_cpum, 4)]; }
	bool output_queued() const { return m_output_queue_state != 0xff; }
	void set_output_queued() { m_output_queue_state = BIT(m_cpum, 4); }

	offs_t mmu_psen_translate(u16 addr) const { return BIT(m_mmu_enable, 6) && BIT(addr + 0x4000, 15) ? u32(m_mmu_bank[BIT(addr, 15)]) << 14 | (addr & 0x3fff) : addr; }
	offs_t mmu_data_translate(u16 addr) const { return BIT(m_mmu_enable, 7) && BIT(addr + 0x4000, 15) ? u32(m_mmu_bank[BIT(addr, 15) + 2]) << 14 | (addr & 0x3fff) : addr; }

	u8 ie0_r();
	void ie0_w(u8 data);
	u8 ie1_r();
	void ie1_w(u8 data);
	u8 cpum_r();
	void cpum_w(u8 data);

	u8 mmu_bank_r(offs_t offset);
	void mmu_bank_w(offs_t offset, u8 data);
	u8 mmu_enable_r();
	void mmu_enable_w(u8 data);

	void swap_cpus();
	void next_instruction(u8 input);

	static const microstate s_decode_map[256];
	static const u16 s_input_queue_map[16];
	static const u8 s_branch_fs[4];

	// address spaces
	address_space_config m_program_config;
	address_space_config m_data_config;
	memory_access<16, 0, 0, ENDIANNESS_BIG>::cache m_cache;
	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::specific m_data;
	const bool m_has_mmu;

	// execution state
	cpu_registers m_cpu[2];
	u8 m_cpum;
	microstate m_ustate;
	u16 m_da;
	u16 m_tmp1;
	u16 m_tmp2;
	u8 m_output_queue_state;
	s32 m_icount;

	// interrupt state
	u16 m_if;
	u16 m_irq;

	// MMU state
	u8 m_mmu_bank[4];
	u8 m_mmu_enable;
};

class mn18801a_device : public mn1880_device
{
public:
	mn18801a_device(const machine_config &config, const char *tag, device_t *owner, u32 clock);
};

DECLARE_DEVICE_TYPE(MN1880, mn1880_device)
DECLARE_DEVICE_TYPE(MN18801A, mn18801a_device)

#endif // MAME_CPU_MN1880_MN1880_H
