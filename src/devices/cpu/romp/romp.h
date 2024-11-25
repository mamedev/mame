// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_CPU_ROMP_ROMP_H
#define MAME_CPU_ROMP_ROMP_H

#pragma once

#include "rsc.h"

class romp_device : public cpu_device
{
public:
	romp_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	template <typename T> void set_mmu(T &&tag) { m_mmu.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_iou(T &&tag) { m_iou.set_tag(std::forward<T>(tag)); }

	void clk_w(int state);

protected:
	enum registers : unsigned
	{
		ROMP_SCR = 0,
		ROMP_GPR = 16,
	};

	enum scr : unsigned
	{
		COUS =  6, // counter source
		COU  =  7, // counter
		TS   =  8, // timer status
		ECR  =  9, // exception control (advanced/enhanced only)
		MQ   = 10, // multiplier quotient
		MPCS = 11, // machine/program check status
		IRB  = 12, // interrupt request buffer
		IAR  = 13, // instruction address register
		ICS  = 14, // interrupt control status
		CS   = 15, // condition status
	};

	enum ts_mask : u32
	{
		TS_E = 0x0000'0040, // enable
		TS_I = 0x0000'0020, // interrupt status
		TS_O = 0x0000'0010, // overflow
		TS_P = 0x0000'0007, // timer interrupt priority
	};
	enum mpcs_mask : u32
	{
							   // reserved
		PCS_DAE = 0x0000'0002, // data address exception
		PCS_IAE = 0x0000'0004, // instruction address exception
		PCS_IOC = 0x0000'0008, // illegal operation code
		PCS_PIE = 0x0000'0010, // privileged instruction exception
		PCS_PT  = 0x0000'0020, // program trap
		PCS_PCU = 0x0000'0040, // program check with unknown origin
		PCS_PCK = 0x0000'0080, // program check with known origin
							   // reserved
		MCS_IOT = 0x0000'0200, // i/o trap
		MCS_PCT = 0x0000'0400, // processor channel timeout
		MCS_DT  = 0x0000'0800, // data timeout
		MCS_IT  = 0x0000'1000, // instruction timeout
		MCS_PC  = 0x0000'2000, // parity check
							   // reserved
		MCS_PCC = 0x0000'8000, // processor channel check

		MCS_ALL = 0x0000'be00,
		PCS_ALL = 0x0000'00fe,
	};

	enum irb_mask : u16
	{
						  // reserved
		IRB_L6  = 0x0200, // interrupt request level 6
		IRB_L5  = 0x0400, // interrupt request level 5
		IRB_L4  = 0x0800, // interrupt request level 4
		IRB_L3  = 0x1000, // interrupt request level 3
		IRB_L2  = 0x2000, // interrupt request level 2
		IRB_L1  = 0x4000, // interrupt request level 1
		IRB_L0  = 0x8000, // interrupt request level 0

		IRB_ALL = 0xfe00,
	};
	enum ics_mask : u32
	{
		ICS_PP = 0x0000'0007, // processor priority
							  // reserved
		ICS_RS = 0x0000'0070, // register set number
		ICS_CS = 0x0000'0080, // check stop mask
		ICS_IM = 0x0000'0100, // interrupt mask
		ICS_TM = 0x0000'0200, // translate mode
		ICS_US = 0x0000'0400, // unprivileged state
		ICS_MP = 0x0000'0800, // memory protect
		ICS_PE = 0x0000'1000, // parity error retry interrupt enable
		ICS_UA = 0x0000'2000, // unaligned access interrupt enable (advanced/enhanced only)
	};
	enum cs_mask : u32
	{
		CS_T = 0x0000'0001, // test bit
		CS_O = 0x0000'0002, // overflow
		CS_C = 0x0000'0008, // carry
		CS_G = 0x0000'0010, // greater than
		CS_E = 0x0000'0020, // equal
		CS_L = 0x0000'0040, // less than
		CS_Z = 0x0000'0080, // permanent zero
	};

	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual u32 execute_min_cycles() const noexcept override { return 1; }
	virtual u32 execute_max_cycles() const noexcept override { return 40; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;
	virtual bool memory_translate(int spacenum, int intention, offs_t &address, address_space *&target_space) override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	// insruction decode helpers
	u32 r3_0(unsigned reg) const { return reg ? m_gpr[reg] : 0; }
	s32 ji(u16 op) const { return s32(s8(op)) << 1; }
	u32 ba(u16 hi, u16 lo) const { return ((u32(hi) << 16) | lo) & 0x00ff'fffeU; }
	s32 bi(u16 hi, u16 lo) const { return s32((u32(hi) << 16 | lo) << 12) >> 11; }

	void flags_log(u32 const op1);
	void flags_add(u32 const op1, u32 const op2);
	void flags_sub(u32 const op1, u32 const op2);

	void set_scr(unsigned scr, u32 data);

	void interrupt_check();
	void machine_check(u32 mcs);
	void program_check(u32 pcs);
	void interrupt_enter(unsigned vector, u32 iar, u16 svc = 0);

	using rsc_mode = rsc_bus_interface::rsc_mode;

	void fetch(u32 address, std::function<void(u16)> f)
	{
		u16 data = 0;

		if (m_mmu->fetch(address, data, rsc_mode((m_scr[ICS] >> 9) & 7)))
			f(data);
		else
			program_check(PCS_PCK | PCS_IAE);
	}

	template <typename T> void load(u32 address, std::function<void(T)> f, bool mask = true)
	{
		rsc_mode const mode = mask ? rsc_mode((m_scr[ICS] >> 9) & 7) : rsc_mode::RSC_N;

		T data = 0;

		switch (address >> 28)
		{
		default:
			if (m_mmu->mem_load(address, data, mode))
				f(data);
			else
				program_check(PCS_PCK | PCS_DAE);
			break;

		case 15:
			switch (address >> 24)
			{
			case 0xf0:
				if (m_iou->pio_load(address, data, mode))
					f(data);
				else
					program_check(PCS_PCK | PCS_DAE);
				break;
			case 0xf4:
				if (m_iou->mem_load(address, data, mode))
					f(data);
				else
					program_check(PCS_PCK | PCS_DAE);
				break;

			case 0xfc: // mc68881 assist mode
			case 0xfd: // mc68881 non-assist mode
			case 0xfe: // afpa dma
			case 0xff: // fpa
			default: // reserved
				program_check(PCS_PCK | PCS_DAE);
				break;
			}
			break;
		}
	}

	template <typename T> void store(u32 address, T data, bool mask = true)
	{
		rsc_mode const mode = mask ? rsc_mode((m_scr[ICS] >> 9) & 7) : rsc_mode::RSC_N;

		switch (address >> 28)
		{
		default:
			if (!m_mmu->mem_store(address, data, mode))
				program_check(PCS_PCK | PCS_DAE);
			break;

		case 15:
			switch (address >> 24)
			{
			case 0xf0:
				if (!m_iou->pio_store(address, data, mode))
					program_check(PCS_PCK | PCS_DAE);
				break;
			case 0xf4:
				if (!m_iou->mem_store(address, data, mode))
					program_check(PCS_PCK | PCS_DAE);
				break;

			case 0xfc: // mc68881 assist mode
			case 0xfd: // mc68881 non-assist mode
			case 0xfe: // afpa dma
			case 0xff: // fpa
			default: // reserved
				program_check(PCS_PCK | PCS_DAE);
				break;
			}
			break;
		}
	}

	template <typename T> void modify(u32 address, std::function<T(T)> f, bool mask = true)
	{
		rsc_mode const mode = mask ? rsc_mode((m_scr[ICS] >> 9) & 7) : rsc_mode::RSC_N;

		switch (address >> 28)
		{
		default:
			if (!m_mmu->mem_modify(address, f, mode))
				program_check(PCS_PCK | PCS_DAE);
			break;

		case 15:
			switch (address >> 24)
			{
			case 0xf0:
				if (!m_iou->pio_modify(address, f, mode))
					program_check(PCS_PCK | PCS_DAE);
				break;
			case 0xf4:
				if (!m_iou->mem_modify(address, f, mode))
					program_check(PCS_PCK | PCS_DAE);
				break;

			case 0xfc: // mc68881 assist mode
			case 0xfd: // mc68881 non-assist mode
			case 0xfe: // afpa dma
			case 0xff: // fpa
			default: // reserved
				program_check(PCS_PCK | PCS_DAE);
				break;
			}
			break;
		}
	}

	// address spaces
	address_space_config const m_mem_config;

	required_device<rsc_cpu_interface> m_mmu;
	required_device<rsc_bus_interface> m_iou;

	// mame state
	int m_icount;

	// core registers
	u32 m_scr[16];
	u32 m_gpr[16];

	// input line state
	u8 m_reqi;
	bool m_trap;

	// internal state
	enum branch_state : unsigned
	{
		DEFAULT   = 0,
		BRANCH    = 1, // branch subject instruction active
		DELAY     = 2, // delayed branch instruction active
		EXCEPTION = 3,
		WAIT      = 4,
	}
	m_branch_state;
	u32 m_branch_source;
	u32 m_branch_target;
	bool m_defer_int;
};

DECLARE_DEVICE_TYPE(ROMP, romp_device)

#endif // MAME_CPU_ROMP_ROMP_H
