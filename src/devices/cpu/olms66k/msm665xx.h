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
		MSM665XX_ROMWIN
	};

	// TODO: port callbacks

protected:
	msm665xx_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor mem_map, address_map_constructor data_map);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual void execute_run() override;

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

private:
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
