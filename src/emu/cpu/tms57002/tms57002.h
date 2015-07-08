// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    tms57002.h

    TMS57002 "DASP" emulator.

***************************************************************************/
#pragma once

#ifndef __TMS57002_H__
#define __TMS57002_H__

class tms57002_device : public cpu_device, public device_sound_interface {
public:
	tms57002_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER(data_r);
	DECLARE_WRITE8_MEMBER(data_w);

	DECLARE_WRITE_LINE_MEMBER(pload_w);
	DECLARE_WRITE_LINE_MEMBER(cload_w);
	DECLARE_READ_LINE_MEMBER(empty_r);
	DECLARE_READ_LINE_MEMBER(dready_r);
	DECLARE_READ_LINE_MEMBER(pc0_r);
	DECLARE_WRITE_LINE_MEMBER(sync_w);

protected:
	virtual void device_start();
	virtual void device_reset();
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const;
	virtual UINT32 execute_min_cycles() const;
	virtual UINT32 execute_max_cycles() const;
	virtual UINT32 execute_input_lines() const;
	virtual void execute_run();
	virtual UINT32 disasm_min_opcode_bytes() const;
	virtual UINT32 disasm_max_opcode_bytes() const;
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);

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
		S_HOST   = 0x00000200
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
		unsigned short op;
		short next;
		unsigned char param;
	};

	struct hcd {
		unsigned int st1;
		short ipc;
		short next;
	};

	struct cd {
		short hashbase[256];
		hcd hashnode[HBS];
		icd inst[IBS];
		int hused, iused;
	};

	struct cstate {
		int branch;
		int inc;
		short hnode;
		short ipc;
	};

	INT64 macc;

	UINT32 cmem[256];
	UINT32 dmem0[256];
	UINT32 dmem1[32];

	UINT32 si[4], so[4];

	UINT32 st0, st1, sti;
	UINT32 aacc, xoa, xba, xwr, xrd, txrd, creg;

	UINT8 pc, hpc, ca, id, ba0, ba1, rptc, rptc_next, sa;

	UINT32 xm_adr;

	UINT8 host[4], hidx, allow_update;

	cd cache;

	const address_space_config program_config, data_config;

	address_space *program, *data;
	int icount;
	int unsupported_inst_warning;

	void decode_error(UINT32 opcode);
	void decode_cat1(UINT32 opcode, unsigned short *op, cstate *cs);
	void decode_cat2_pre(UINT32 opcode, unsigned short *op, cstate *cs);
	void decode_cat3(UINT32 opcode, unsigned short *op, cstate *cs);
	void decode_cat2_post(UINT32 opcode, unsigned short *op, cstate *cs);

	inline int xmode(UINT32 opcode, char type, cstate *cs);
	inline int sfao(UINT32 st1);
	inline int dbp(UINT32 st1);
	inline int crm(UINT32 st1);
	inline int sfai(UINT32 st1);
	inline int sfmo(UINT32 st1);
	inline int rnd(UINT32 st1);
	inline int movm(UINT32 st1);
	inline int sfma(UINT32 st1);

	const char *get_memadr(UINT32 opcode, char type);

	void xm_init();
	void xm_step_read();
	void xm_step_write();
	INT64 macc_to_output_0(INT64 rounding, UINT64 rmask);
	INT64 macc_to_output_1(INT64 rounding, UINT64 rmask);
	INT64 macc_to_output_2(INT64 rounding, UINT64 rmask);
	INT64 macc_to_output_3(INT64 rounding, UINT64 rmask);
	INT64 macc_to_output_0s(INT64 rounding, UINT64 rmask);
	INT64 macc_to_output_1s(INT64 rounding, UINT64 rmask);
	INT64 macc_to_output_2s(INT64 rounding, UINT64 rmask);
	INT64 macc_to_output_3s(INT64 rounding, UINT64 rmask);
	INT64 check_macc_overflow_0();
	INT64 check_macc_overflow_1();
	INT64 check_macc_overflow_2();
	INT64 check_macc_overflow_3();
	INT64 check_macc_overflow_0s();
	INT64 check_macc_overflow_1s();
	INT64 check_macc_overflow_2s();
	INT64 check_macc_overflow_3s();
	void cache_flush();
	void add_one(cstate *cs, unsigned short op, UINT8 param);
	void decode_one(UINT32 opcode, cstate *cs, void (tms57002_device::*dec)(UINT32 opcode, unsigned short *op, cstate *cs));
	short get_hash(unsigned char adr, UINT32 st1, short *pnode);
	short get_hashnode(unsigned char adr, UINT32 st1, short pnode);
	int decode_get_pc();
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

extern const device_type TMS57002;

#endif
