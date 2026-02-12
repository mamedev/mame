// license:BSD-3-Clause
// copyright-holders:Ville Linde
#ifndef MAME_CPU_SHARC_SHARCINTERNAL_IPP
#define MAME_CPU_SHARC_SHARCINTERNAL_IPP

#include "sharc.h"


constexpr uint32_t OP_USERFLAG_LOOP                 = 0x00000001;
constexpr uint32_t OP_USERFLAG_ASTAT_DELAY_COPY_AZ  = 0x00001000;
constexpr uint32_t OP_USERFLAG_ASTAT_DELAY_COPY_AN  = 0x00002000;
constexpr uint32_t OP_USERFLAG_ASTAT_DELAY_COPY_AC  = 0x00004000;
constexpr uint32_t OP_USERFLAG_ASTAT_DELAY_COPY_AV  = 0x00008000;
constexpr uint32_t OP_USERFLAG_ASTAT_DELAY_COPY_MV  = 0x00010000;
constexpr uint32_t OP_USERFLAG_ASTAT_DELAY_COPY_MN  = 0x00020000;
constexpr uint32_t OP_USERFLAG_ASTAT_DELAY_COPY_SV  = 0x00040000;
constexpr uint32_t OP_USERFLAG_ASTAT_DELAY_COPY_SZ  = 0x00080000;
constexpr uint32_t OP_USERFLAG_ASTAT_DELAY_COPY_BTF = 0x00100000;
constexpr uint32_t OP_USERFLAG_ASTAT_DELAY_COPY     = 0x001ff000;
constexpr uint32_t OP_USERFLAG_CALL                 = 0x10000000;


struct alignas(16) adsp21062_device::sharc_internal_state
{
	struct alignas(16) SHARC_DAG
	{
		uint32_t i[8];
		uint32_t m[8];
		uint32_t b[8];
		uint32_t l[8];
	};

	struct SHARC_LADDR
	{
		uint32_t addr;
		uint32_t code;
		uint32_t loop_type;
	};

	struct ASTAT_DRC
	{
		union
		{
			struct
			{
				uint32_t az;
				uint32_t av;
				uint32_t an;
				uint32_t ac;
				uint32_t as;
				uint32_t ai;
				uint32_t mn;
				uint32_t mv;
				uint32_t mu;
				uint32_t mi;
				uint32_t sv;
				uint32_t sz;
				uint32_t ss;
				uint32_t btf;
				uint32_t af;
				uint32_t cacc;
			};
			uint64_t flags64[8];
		};

		uint32_t pack() const
		{
			return
					((az << AZ_SHIFT) & AZ) |
					((av << AV_SHIFT) & AV) |
					((an << AN_SHIFT) & AN) |
					((ac << AC_SHIFT) & AC) |
					((as << AS_SHIFT) & AS) |
					((ai << AI_SHIFT) & AI) |
					((mn << MN_SHIFT) & MN) |
					((mv << MV_SHIFT) & MV) |
					((mu << MU_SHIFT) & MU) |
					((mi << MI_SHIFT) & MI) |
					((sv << SV_SHIFT) & SV) |
					((sz << SZ_SHIFT) & SZ) |
					((ss << SS_SHIFT) & SS) |
					((btf << BTF_SHIFT) & BTF) |
					((af << AF_SHIFT) & AF) |
					((cacc << 24) & 0xff00'0000);
		}

		void unpack(uint32_t in)
		{
			az = BIT(in, AZ_SHIFT);
			av = BIT(in, AV_SHIFT);
			an = BIT(in, AN_SHIFT);
			ac = BIT(in, AC_SHIFT);
			as = BIT(in, AS_SHIFT);
			ai = BIT(in, AI_SHIFT);
			mn = BIT(in, MN_SHIFT);
			mv = BIT(in, MV_SHIFT);
			mu = BIT(in, MU_SHIFT);
			mi = BIT(in, MI_SHIFT);
			sv = BIT(in, SV_SHIFT);
			sz = BIT(in, SZ_SHIFT);
			ss = BIT(in, SS_SHIFT);
			btf = BIT(in, BTF_SHIFT);
			af = BIT(in, AF_SHIFT);
			cacc = BIT(in, 24, 8);
		}
	};


	SHARC_REG r[16];
	SHARC_REG reg_alt[16];

	uint32_t pc;
	uint64_t mrf;
	uint64_t mrb;

	uint32_t pcstack[32];
	uint32_t lcstack[6];
	uint32_t lastack[6];
	uint32_t lstkp;

	uint32_t faddr;
	uint32_t daddr;
	uint32_t pcstk;
	uint32_t pcstkp;
	SHARC_LADDR laddr;
	uint32_t curlcntr;
	uint32_t lcntr;
	uint8_t extdma_shift;
	uint32_t iop_write_num;
	uint32_t iop_data;

	/* Data Address Generator (DAG) */
	SHARC_DAG dag1;     // (DM bus)
	SHARC_DAG dag2;     // (PM bus)
	SHARC_DAG dag1_alt;
	SHARC_DAG dag2_alt;

	SHARC_DMA_REGS dma[12];

	/* System registers */
	uint32_t mode1;
	uint32_t mode2;
	uint32_t astat;
	uint32_t stky;
	uint32_t irptl;
	uint32_t imask;
	uint32_t imaskp;
	uint32_t ustat1;
	uint32_t ustat2;

	uint32_t flag[4];

	uint32_t syscon;
	uint32_t sysstat;

	struct
	{
		uint32_t mode1;
		uint32_t astat;
	} status_stack[5];
	int32_t status_stkp;

	uint64_t px;

	int icount;
	uint64_t opcode;

	uint32_t nfaddr;

	int32_t idle;
	int32_t irq_pending;
	int32_t active_irq_num;

	SHARC_DMA_OP dma_op[12];
	uint32_t dma_status;
	bool write_stalled;

	int32_t interrupt_active;

	uint32_t iop_delayed_reg;
	uint32_t iop_delayed_data;
	emu_timer *delayed_iop_timer;

	uint32_t delay_slot1, delay_slot2;

	int32_t systemreg_latency_cycles;
	int32_t systemreg_latency_reg;
	uint32_t systemreg_latency_data;
	uint32_t systemreg_previous_data;

	uint32_t astat_old;
	uint32_t astat_old_old;
	uint32_t astat_old_old_old;

	uint32_t arg0;
	uint32_t arg1;
	uint32_t arg2;
	uint32_t arg3;

	uint64_t arg64;
	uint32_t mode1_delay_data;

	ASTAT_DRC astat_drc;
	ASTAT_DRC astat_drc_copy;
	ASTAT_DRC astat_delay_copy;
	uint32_t dreg_temp;
	uint32_t dreg_temp2;
	uint32_t jmpdest;
	uint32_t temp_return;

	struct
	{
		float k0_0;
		float k0_5;
		float k1_0;
		float k2_0;
	} fp_const;

	uint32_t m_max_sram_pc[2];
	uint32_t force_recompile;
	uint32_t cache_dirty;
};


// SHARC memory operations

// When PM bus is used to transfer 32-bit data, it is aligned to the upper 32 bits of the bus
inline uint32_t adsp21062_device::pm_read32(uint32_t address)
{
	return uint32_t(m_program.read_qword(address) >> 16);
}

inline void adsp21062_device::pm_write32(uint32_t address, uint32_t data)
{
	// TODO: mask should probably be set to all ones
	m_program.write_qword(address, uint64_t(data) << 16, 0x0000ffff'ffff0000U);
}

inline uint64_t adsp21062_device::pm_read48(uint32_t address)
{
	return m_program.read_qword(address);
}

inline void adsp21062_device::pm_write48(uint32_t address, uint64_t data)
{
	m_program.write_qword(address, data);
}

inline uint32_t adsp21062_device::dm_read32(uint32_t address)
{
	return m_data.read_dword(address);
}

inline void adsp21062_device::dm_write32(uint32_t address, uint32_t data)
{
	m_data.write_dword(address, data);
}

#endif // MAME_CPU_SHARC_SHARCINTERNAL_IPP
