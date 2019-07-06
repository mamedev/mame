// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    Fujitsu Micro F2MC-16 series

***************************************************************************/

#ifndef MAME_CPU_F2MC16_F2MC16_H
#define MAME_CPU_F2MC16_F2MC16_H 1

#pragma once

class f2mc16_device : public cpu_device
{
public:
	enum
	{
		F2MC16_PC, F2MC16_PS, F2MC16_USP, F2MC16_SSP, F2MC16_ACC,
		F2MC16_PCB, F2MC16_DTB, F2MC16_USB, F2MC16_SSB, F2MC16_ADB, F2MC16_DPR,
		F2MC16_RW0, F2MC16_RW1, F2MC16_RW2, F2MC16_RW3,
		F2MC16_RW4, F2MC16_RW5, F2MC16_RW6, F2MC16_RW7,
		F2MC16_RL0, F2MC16_RL1, F2MC16_RL2, F2MC16_RL3,
		F2MC16_R0, F2MC16_R1, F2MC16_R2, F2MC16_R3, F2MC16_R4, F2MC16_R5, F2MC16_R6, F2MC16_R7
	};

	enum
	{
		F_I = 0x40,
		F_S = 0x20,
		F_T = 0x10,
		F_N = 0x08,
		F_Z = 0x04,
		F_V = 0x02,
		F_C = 0x01
	};

	// construction/destruction
	f2mc16_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	f2mc16_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_execute_interface overrides
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

private:
	address_space_config m_program_config;
	address_space *m_program;

	u16 m_pc, m_usp, m_ssp, m_ps, m_tmp16;
	u8 m_pcb, m_dtb, m_usb, m_ssb, m_adb, m_dpr, m_tmp8;
	u32 m_acc, m_temp, m_tmp32;
	s32 m_icount;

	inline u8 read_8(u32 addr)
	{
		return m_program->read_byte(addr);
	}
	inline u16 read_16(u32 addr)
	{
		if (addr & 1)
		{
			return m_program->read_byte(addr) | (m_program->read_byte(addr+1)<<8);
		}
		else
		{
			return m_program->read_word(addr);
		}
	}
	inline u32 read_32(u32 addr)
	{
		if (addr & 3)
		{
			return m_program->read_byte(addr) | (m_program->read_byte(addr+1)<<8) | (m_program->read_byte(addr+2)<<16) | (m_program->read_byte(addr+3)<<24);
		}
		else
		{
			return m_program->read_dword(addr);
		}
	}
	inline void write_8(u32 addr, u8 data)
	{
		m_program->write_byte(addr, data);
	}
	inline void write_16(u32 addr, u16 data)
	{
		if (addr & 1)
		{
			m_program->write_byte(addr, data & 0xff);
			m_program->write_byte(addr+1, (data>>8) & 0xff);
		}
		else
		{
			m_program->write_word(addr, data);
		}
	}
	inline void write_32(u32 addr, u32 data)
	{
		if (addr & 3)
		{
			m_program->write_byte(addr, data & 0xff);
			m_program->write_byte(addr+1, (data>>8) & 0xff);
			m_program->write_byte(addr+2, (data>>16) & 0xff);
			m_program->write_byte(addr+3, (data>>24) & 0xff);
		}
		else
		{
			m_program->write_dword(addr, data);
		}
	}
	inline u8 read_rX(int reg)
	{
		reg &= 7;
		return m_temp = read_8(reg + 0x188 + (((m_ps>>8)&0x1f)*0x10));
	}
	inline u16 read_rwX(int reg)
	{
		reg &= 7;
		return m_temp = read_16((reg<<1) + 0x180 + (((m_ps>>8)&0x1f)*0x10));
	}
	inline u32 read_rlX(int reg)
	{
		reg &= 3;
		return m_temp = read_32((reg<<2) + 0x180 + (((m_ps>>8)&0x1f)*0x10));
	}
	inline void write_rX(int reg, u8 val)
	{
		reg &= 7;
		write_8(reg + 0x188 + (((m_ps>>8)&0x1f)*0x10), val);
	}
	inline void write_rwX(int reg, u16 val)
	{
		reg &= 7;
		write_16((reg<<1) + 0x180 + (((m_ps>>8)&0x1f)*0x10), val);
	}
	inline void write_rlX(int reg, u32 val)
	{
		reg &= 3;
		write_16((reg<<2) + 0x180 + (((m_ps>>8)&0x1f)*0x10), val);
	}

	inline void setNZ_8(u8 uVal) { m_ps &= ~(F_N|F_Z); m_ps |= (uVal == 0) ? F_Z : 0; m_ps |= (uVal & 0x80) ? F_N : 0; }
	inline void setNZ_16(u16 uVal) { m_ps &= ~(F_N|F_Z); m_ps |= (uVal == 0) ? F_Z : 0; m_ps |= (uVal & 0x8000) ? F_N : 0; }

	// get the full 24 bit address for an @RWx access
	inline u32 getRWbank(int iReg, u16 uBankAddr)
	{
		iReg &= 7;
		switch (iReg)
		{
			case 0:
			case 1:
			case 4:
			case 5:
				return (m_dtb<<16) | uBankAddr;

			case 2:
			case 6:
				return (m_adb<<16) | uBankAddr;

			case 3:
			case 7:
				return (m_usb<<16) | uBankAddr;
		}

		// this can't happen, but GCC insists
		return (m_dtb<<16) | uBankAddr;
	}

	void opcodes_str6e(u8 operand);
	void opcodes_2b6f(u8 operand);
	void opcodes_ea71(u8 operand);
};

DECLARE_DEVICE_TYPE(F2MC16, f2mc16_device)

#endif // MAME_CPU_F2MC16_F2MC16_H
