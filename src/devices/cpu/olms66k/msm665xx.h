// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Oki MSM665xx 16-bit microcontroller family (nX-8/500S core)

**********************************************************************/

#ifndef MAME_CPU_OLMS66K_MSM665XX_H
#define MAME_CPU_OLMS66K_MSM665XX_H

#pragma once


class msm665xx_device : public cpu_device
{
public:
	enum {
		MSM665XX_ACC, MSM665XX_PC, MSM665XX_PSW, MSM665XX_LRB, MSM665XX_SSP,
		MSM665XX_PR0, MSM665XX_X1 = MSM665XX_PR0,
		MSM665XX_PR1, MSM665XX_X2 = MSM665XX_PR1,
		MSM665XX_PR2, MSM665XX_DP = MSM665XX_PR2,
		MSM665XX_PR3, MSM665XX_USP = MSM665XX_PR3,
		MSM665XX_ER0, MSM665XX_ER1, MSM665XX_ER2, MSM665XX_ER3,
		MSM665XX_R0, MSM665XX_R1, MSM665XX_R2, MSM665XX_R3,
		MSM665XX_R4, MSM665XX_R5, MSM665XX_R6, MSM665XX_R7,
		MSM665XX_CSR, MSM665XX_DSR, MSM665XX_TSR,
		MSM665XX_ROMWIN, MSM665XX_MEMSCON
	};

	// TODO: port callbacks

protected:
	msm665xx_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor mem_map, address_map_constructor data_map);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual void execute_run() override;
	virtual u32 execute_min_cycles() const noexcept override { return 2; }
	virtual u32 execute_max_cycles() const noexcept override { return 42; }

	virtual space_config_vector memory_space_config() const override;

	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	u16 ssp_r();
	void ssp_w(u16 data);
	u16 lrb_r();
	void lrb_w(offs_t offset, u16 data, u16 mem_mask);
	u16 psw_r();
	void psw_w(offs_t offset, u16 data, u16 mem_mask);
	u16 acc_r();
	void acc_w(offs_t offset, u16 data, u16 mem_mask);
	u8 dsr_r();
	void dsr_w(u8 data);
	u8 tsr_r();
	void tsr_w(u8 data);
	u8 romwin_r();
	void romwin_w(u8 data);
	void memsacp_w(u8 data);
	u8 memscon_r();
	void memscon_w(u8 data);

private:
	u16 addr_lr(u8 n) const noexcept { return 0x0200 + (n & 7) + (m_lrb & 0x00ff) * 8; }
	u16 addr_er(u8 n) const noexcept { return 0x0200 + (n & 3) * 2 + (m_lrb & 0x00ff) * 8; }
	u16 addr_pr(u8 n) const noexcept { return 0x0200 + (n & 3) * 2 + (m_psw & 0x0007) * 8; }

	void set_adr_fix8(u8 fix8) noexcept { m_adr = 0x0200 | fix8; m_seg = 0; }
	void set_adr_off8(u8 off8) noexcept { m_adr = (m_lrb & 0xff00) | off8; m_seg = get_seg(m_adr); }
	void set_adr_sfr8(u8 sfr8) noexcept { m_adr = sfr8; m_seg = 0; }

	u16 get_seg(u16 addr) const noexcept;

	u8 data_read_byte(u16 seg, u16 adr) { return s16(seg) < 0 ? m_program_space.read_byte(u32(seg) << 16 | adr) : m_data_space.read_byte(u32(seg) << 16 | adr); }
	u16 data_read_word(u16 seg, u16 adr) { return s16(seg) < 0 ? m_program_space.read_word_unaligned(u32(seg) << 16 | adr) : m_data_space.read_word(u32(seg) << 16 | adr); }
	void data_write_byte(u16 seg, u16 adr, u8 data) { m_data_space.write_byte(u32(seg) << 16 | adr, data); }
	void data_write_word(u16 seg, u16 adr, u16 data) { m_data_space.write_word(u32(seg) << 16 | adr, data); }

	void next_inst() { m_inst = m_fetch_byte; m_state = s_inst_decode[BIT(m_psw, 12)][m_inst]; m_ppc = u32(m_csr) << 16 | u16(m_pc - 1); debugger_instruction_hook(m_ppc); }
	void next_inst_and_store_byte(u8 data) { m_inst = m_fetch_byte; m_state = s_inst_decode[BIT(m_psw, 12)][m_inst]; data_write_byte(m_seg, m_adr, data); m_ppc = u32(m_csr) << 16 | u16(m_pc - 1); debugger_instruction_hook(m_ppc); }
	void next_inst_and_store_word(u16 data) { m_inst = m_fetch_byte; m_state = s_inst_decode[BIT(m_psw, 12)][m_inst]; data_write_word(m_seg, m_adr, data); m_ppc = u32(m_csr) << 16 | u16(m_pc - 1); debugger_instruction_hook(m_ppc); }
	void prefetch() { m_fetch_byte = m_program_cache.read_byte(u32(m_csr) << 16 | m_pc); ++m_pc; }

	void acc_load_byte(u8 data) noexcept
	{
		m_acc = (m_acc & 0xff00) | data;
		if (data == 0)
			m_psw |= 0x4000;
		else
			m_psw &= 0xbfff;
		m_psw &= 0xefff;
	}

	void acc_load_word(u16 data) noexcept
	{
		m_acc = data;
		if (data == 0)
			m_psw |= 0x4000;
		else
			m_psw &= 0xbfff;
		m_psw |= 0x1000;
	}

	void setzs_byte(u8 data) noexcept
	{
		if (data == 0)
			m_psw |= 0x4000;
		else
			m_psw &= 0xbfff;
		if (s8(data) < 0)
			m_psw |= 0x0800;
		else
			m_psw &= 0xf7ff;
	}

	void setzs_word(u16 data) noexcept
	{
		if (data == 0)
			m_psw |= 0x4000;
		else
			m_psw &= 0xbfff;
		if (s16(data) < 0)
			m_psw |= 0x0800;
		else
			m_psw &= 0xf7ff;
	}

	u16 do_add(u16 n1, u16 n2, bool c = false) noexcept;
	u8 do_addb(u8 n1, u8 n2, bool c = false) noexcept;
	u16 do_sub(u16 n1, u16 n2, bool c = false) noexcept;
	u8 do_subb(u8 n1, u8 n2, bool c = false) noexcept;
	u16 do_aluop_word(u16 n1, u16 n2) noexcept;
	u8 do_aluop_byte(u8 n1, u8 n2) noexcept;
	u16 do_inc(u16 n) noexcept;
	u8 do_incb(u8 n) noexcept;
	u16 do_dec(u16 n) noexcept;
	u8 do_decb(u8 n) noexcept;
	u16 do_rol(u16 n) noexcept;
	u8 do_rolb(u8 n) noexcept;
	u16 do_ror(u16 n) noexcept;
	u8 do_rorb(u8 n) noexcept;
	void do_mul(u16 n) noexcept;
	void do_div(u16 n) noexcept;
	void do_divb(u8 n) noexcept;
	void do_divq(u16 n) noexcept;

	enum class inst_state : u8;
	static const inst_state s_inst_decode[2][0x100];
	static const inst_state s_prefixed_inst_decode[2][0x100];

	address_space_config m_program_config;
	address_space_config m_data_config;
	memory_access<20, 0, 0, ENDIANNESS_LITTLE>::cache m_program_cache;
	memory_access<20, 0, 0, ENDIANNESS_LITTLE>::specific m_program_space;
	memory_access<20, 1, 0, ENDIANNESS_LITTLE>::cache m_data_cache;
	memory_access<20, 1, 0, ENDIANNESS_LITTLE>::specific m_data_space;

	u16 m_acc;
	u16 m_pc;
	u32 m_ppc;
	u16 m_psw;
	u16 m_lrb;
	u16 m_ssp;
	u8 m_csr;
	u8 m_dsr;
	u8 m_tsr;
	u8 m_romwin;
	u8 m_memscon;

	inst_state m_state;
	u8 m_inst;
	u8 m_fetch_byte;
	u16 m_adr;
	u16 m_seg;
	u16 m_tmp[2];
	s32 m_icount;
};


class msm66573_device : public msm665xx_device
{
public:
	// device type constructor
	msm66573_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

private:
	void data_map(address_map &map) ATTR_COLD;
};


// device type declaration
DECLARE_DEVICE_TYPE(MSM66573, msm66573_device)

#endif // MAME_CPU_OLMS66K_MSM665XX_H
