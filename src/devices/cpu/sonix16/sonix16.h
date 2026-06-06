// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_SONIX16_SONIX16_H
#define MAME_CPU_SONIX16_SONIX16_H

#pragma once

class sonix16_device : public cpu_device
{
	enum {
		SONIX16_X0 = 0, SONIX16_X1, SONIX16_R0, SONIX16_R1, SONIX16_Y0, SONIX16_Y1,
		SONIX16_IX0, SONIX16_IX1, SONIX16_IY0, SONIX16_IY1,
		SONIX16_MR, SONIX16_MR0, SONIX16_MR1, SONIX16_MR2,
		SONIX16_SSF,
		SONIX16_RAMBK,
		SONIX16_IX0BK, SONIX16_IX1BK, SONIX16_IY0BK, SONIX16_IY1BK, SONIX16_IX0BKRAM, SONIX16_IX1BKRAM,
		SONIX16_PC, SONIX16_SP,
		SONIX16_INTEN
	};

public:
	// construction/destruction
	sonix16_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual void execute_run() override;
	virtual u32 execute_min_cycles() const noexcept override { return 1; }
	virtual u32 execute_max_cycles() const noexcept override { return 2; }

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

private:
	u16 get_reg(unsigned r) const noexcept;
	void set_reg(unsigned r, u16 v) noexcept;
	u16 add(u16 xop, u16 yop, bool cin) noexcept;

	u16 ssf_r();
	void ssf_w(u16 data);
	u16 ix_r(offs_t offset);
	void ix_w(offs_t offset, u16 data);
	u16 iy_r(offs_t offset);
	void iy_w(offs_t offset, u16 data);
	u16 rambk_r();
	void rambk_w(u16 data);
	u16 pch_r();
	void pch_w(u16 data);
	u16 pcl_r();
	void pcl_w(u16 data);
	u16 sp_r();
	void sp_w(u16 data);
	u16 mr2_r();
	void mr2_w(u16 data);
	u16 ixbk_r(offs_t offset);
	void ixbk_w(offs_t offset, u16 data);
	u16 iybk_r(offs_t offset);
	void iybk_w(offs_t offset, u16 data);
	u16 ixbkram_r(offs_t offset);
	void ixbkram_w(offs_t offset, u16 data);
	u16 inten_r();
	void inten_w(u16 data);
	u16 iosw_r();
	void iosw_w(u16 data);

	void io_map(address_map &map) ATTR_COLD;

	// address spaces
	address_space_config m_rom_config;
	address_space_config m_ram_config;
	address_space_config m_io_config;
	memory_access<24, 1, -1, ENDIANNESS_LITTLE>::specific m_rom_space;
	memory_access<24, 1, -1, ENDIANNESS_LITTLE>::specific m_ram_space;
	memory_access<7, 1, -1, ENDIANNESS_LITTLE>::specific m_io_space;
	memory_access<24, 1, -1, ENDIANNESS_LITTLE>::cache m_cache;

	// internal registers
	u16 m_r[6];
	u16 m_ir[4];
	u64 m_mr;
	u8 m_ssf;
	u8 m_rambk;
	u8 m_ixbk[4];
	u8 m_ixbkram[2];
	u32 m_pc;
	u16 m_sp;
	u16 m_inten;
	u16 m_iosw;
	s32 m_icount;
};

// device type declaration
DECLARE_DEVICE_TYPE(SONIX16, sonix16_device)

#endif // MAME_CPU_SONIX16_SONIX16_H
