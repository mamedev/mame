// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    tms57002.h

    TMS57002 "DASP" emulator.

***************************************************************************/
#ifndef MAME_CPU_TMS57002_TMS57002_H
#define MAME_CPU_TMS57002_TMS57002_H

#pragma once

class tms57002_device : public cpu_device, public device_sound_interface
{
public:
	tms57002_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto dready_callback() { return m_dready_callback.bind(); }
	auto pc0_callback() { return m_pc0_callback.bind(); }
	auto empty_callback() { return m_empty_callback.bind(); }

	u8 data_r();
	void data_w(u8 data);

	DECLARE_WRITE_LINE_MEMBER(pload_w);
	DECLARE_WRITE_LINE_MEMBER(cload_w);
	DECLARE_READ_LINE_MEMBER(empty_r);
	DECLARE_READ_LINE_MEMBER(dready_r);
	DECLARE_READ_LINE_MEMBER(pc0_r);
	DECLARE_WRITE_LINE_MEMBER(sync_w);

	void internal_pgm(address_map &map);
protected:
	virtual void device_resolve_objects() override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;
	virtual space_config_vector memory_space_config() const override;
	virtual u32 execute_min_cycles() const noexcept override;
	virtual u32 execute_max_cycles() const noexcept override;
	virtual u32 execute_input_lines() const noexcept override;
	virtual void execute_run() override;
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	enum {
		IN_PLOAD = 0x00000001,
		IN_CLOAD = 0x00000002,
		SU_CVAL  = 0x00000004,
		SU_MASK  = 0x00000018, SU_ST0 = 0x00, SU_ST1 = 0x08, SU_PRG = 0x10,
		S_IDLE   = 0x00000020,
		S_READ   = 0x00000040,
		S_WRITE  = 0x00000080,
		S_BRANCH = 0x00000100,
		S_HOST   = 0x00000200,
		S_UPDATE = 0x00000400,
	};

	enum {
		ST0_INCS = 0x000001,
		ST0_DIRI = 0x000002,
		ST0_FI   = 0x000004,
		ST0_SIM  = 0x000008,
		ST0_PLRI = 0x000020,
		ST0_PBCI = 0x000040,
		ST0_DIRO = 0x000080,
		ST0_FO   = 0x000100,
		ST0_SOM  = 0x000600,
		ST0_PLRO = 0x000800,
		ST0_PBCO = 0x001000,
		ST0_CNS  = 0x002000,
		ST0_WORD = 0x004000,
		ST0_SEL  = 0x008000,
		ST0_M    = 0x030000, ST0_M_64K = 0x000000, ST0_M_256K = 0x010000, ST0_M_1M = 0x020000,
		ST0_SRAM = 0x200000,

		ST1_AOV  = 0x000001,
		ST1_SFAI = 0x000002,
		ST1_SFAO = 0x000004,
		ST1_AOVM = 0x000008, // undocumented!
		ST1_MOVM = 0x000020,
		ST1_MOV  = 0x000040,
		ST1_SFMA = 0x000180, ST1_SFMA_SHIFT = 7,
		ST1_SFMO = 0x001800, ST1_SFMO_SHIFT = 11,
		ST1_RND  = 0x038000, ST1_RND_SHIFT = 15,
		ST1_CRM  = 0x0C0000, ST1_CRM_SHIFT = 18, ST1_CRM_32 = 0x000000, ST1_CRM_16H = 0x040000, ST1_CRM_16L = 0x080000,
		ST1_DBP  = 0x100000,
		ST1_CAS  = 0x200000,

		ST1_CACHE = ST1_SFAI|ST1_SFAO|ST1_MOVM|ST1_SFMA|ST1_SFMO|ST1_RND|ST1_CRM|ST1_DBP
	};


	enum { BR_UB, BR_CB, BR_IDLE };

	enum { IBS = 8192, HBS = 4096 };

	enum { INC_CA = 1, INC_ID = 2 };

	struct icd {
		u16 op;
		s16 next;
		u8 param;
	};

	struct hcd {
		u32 st1;
		s16 ipc;
		s16 next;
	};

	struct cd {
		s16 hashbase[256];
		hcd hashnode[HBS];
		icd inst[IBS];
		int hused, iused;
	};

	struct cstate {
		int branch;
		int inc;
		s16 hnode;
		s16 ipc;
	};

	// macc_read and macc_write are used by non-pipelined instructions
	s64 macc, macc_read, macc_write;

	u32 cmem[256];
	u32 dmem0[256];
	u32 dmem1[32];

	u32 si[4], so[4];

	u32 st0, st1, sti;
	u32 aacc, xoa, xba, xwr, xrd, txrd, creg;

	u8 pc, hpc, ca, id, ba0, ba1, rptc, rptc_next, sa;

	u32 xm_adr;

	u8 host[4], hidx, allow_update;

	u32 update[16];
	u8 update_counter_head, update_counter_tail;

	cd cache;

	devcb_write_line m_dready_callback;
	devcb_write_line m_pc0_callback;
	devcb_write_line m_empty_callback;

	const address_space_config program_config, data_config;

	address_space *program, *data;
	int icount;
	int unsupported_inst_warning;

	void decode_error(u32 opcode);
	void decode_cat1(u32 opcode, u16 *op, cstate *cs);
	void decode_cat2_pre(u32 opcode, u16 *op, cstate *cs);
	void decode_cat3(u32 opcode, u16 *op, cstate *cs);
	void decode_cat2_post(u32 opcode, u16 *op, cstate *cs);

	inline int xmode(u32 opcode, char type, cstate *cs);
	inline int sfao(u32 st1);
	inline int dbp(u32 st1);
	inline int crm(u32 st1);
	inline int sfai(u32 st1);
	inline int sfmo(u32 st1);
	inline int rnd(u32 st1);
	inline int movm(u32 st1);
	inline int sfma(u32 st1);

	void update_dready();
	void update_pc0();
	void update_empty();

	void xm_init();
	void xm_step_read();
	void xm_step_write();
	s64 macc_to_output_0(s64 rounding, u64 rmask);
	s64 macc_to_output_1(s64 rounding, u64 rmask);
	s64 macc_to_output_2(s64 rounding, u64 rmask);
	s64 macc_to_output_3(s64 rounding, u64 rmask);
	s64 macc_to_output_0s(s64 rounding, u64 rmask);
	s64 macc_to_output_1s(s64 rounding, u64 rmask);
	s64 macc_to_output_2s(s64 rounding, u64 rmask);
	s64 macc_to_output_3s(s64 rounding, u64 rmask);
	s64 check_macc_overflow_0();
	s64 check_macc_overflow_1();
	s64 check_macc_overflow_2();
	s64 check_macc_overflow_3();
	s64 check_macc_overflow_0s();
	s64 check_macc_overflow_1s();
	s64 check_macc_overflow_2s();
	s64 check_macc_overflow_3s();
	void cache_flush();
	void add_one(cstate *cs, u16 op, u8 param);
	void decode_one(u32 opcode, cstate *cs, void (tms57002_device::*dec)(u32 opcode, u16 *op, cstate *cs));
	s16 get_hash(u8 adr, u32 st1, s16 *pnode);
	s16 get_hashnode(u8 adr, u32 st1, s16 pnode);
	int decode_get_pc();
	u32 get_cmem(u8 addr);

#define CINTRPDECL
#include "../../emu/cpu/tms57002/tms57002.hxx"
#undef CINTRPDECL
};

enum {
	TMS57002_PC=1,
	TMS57002_AACC,
	TMS57002_BA0,
	TMS57002_BA1,
	TMS57002_CREG,
	TMS57002_CA,
	TMS57002_DREG,
	TMS57002_ID,
	TMS57002_MACC,
	TMS57002_HIDX,
	TMS57002_HOST0,
	TMS57002_HOST1,
	TMS57002_HOST2,
	TMS57002_HOST3,
	TMS57002_RPTC,
	TMS57002_SA,
	TMS57002_ST0,
	TMS57002_ST1,
	TMS57002_TREG,
	TMS57002_XBA,
	TMS57002_XOA,
	TMS57002_XRD,
	TMS57002_XWR
};

DECLARE_DEVICE_TYPE(TMS57002, tms57002_device)

#endif // MAME_CPU_TMS57002_TMS57002_H
