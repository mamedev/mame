// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************
*
*   Sony Playstation 2 Vector Unit device skeleton
*
*   To Do:
*     Everything
*
*/

#ifndef MAME_CPU_MIPS_PS2VU_H
#define MAME_CPU_MIPS_PS2VU_H

#pragma once

class sonyvu0_device;
class sonyvu1_device;

#include "video/ps2gs.h"
#include "ps2vif1.h"

enum
{
	SONYVU_TPC,
	SONYVU_SF,
	SONYVU_MF,
	SONYVU_CF,
	SONYVU_R,
	SONYVU_I,
	SONYVU_Q,
	SONYVU_ACCx,  SONYVU_ACCy,  SONYVU_ACCz,  SONYVU_ACCw,
	SONYVU_VF00x, SONYVU_VF00y, SONYVU_VF00z, SONYVU_VF00w,
	SONYVU_VF01x, SONYVU_VF01y, SONYVU_VF01z, SONYVU_VF01w,
	SONYVU_VF02x, SONYVU_VF02y, SONYVU_VF02z, SONYVU_VF02w,
	SONYVU_VF03x, SONYVU_VF03y, SONYVU_VF03z, SONYVU_VF03w,
	SONYVU_VF04x, SONYVU_VF04y, SONYVU_VF04z, SONYVU_VF04w,
	SONYVU_VF05x, SONYVU_VF05y, SONYVU_VF05z, SONYVU_VF05w,
	SONYVU_VF06x, SONYVU_VF06y, SONYVU_VF06z, SONYVU_VF06w,
	SONYVU_VF07x, SONYVU_VF07y, SONYVU_VF07z, SONYVU_VF07w,
	SONYVU_VF08x, SONYVU_VF08y, SONYVU_VF08z, SONYVU_VF08w,
	SONYVU_VF09x, SONYVU_VF09y, SONYVU_VF09z, SONYVU_VF09w,
	SONYVU_VF10x, SONYVU_VF10y, SONYVU_VF10z, SONYVU_VF10w,
	SONYVU_VF11x, SONYVU_VF11y, SONYVU_VF11z, SONYVU_VF11w,
	SONYVU_VF12x, SONYVU_VF12y, SONYVU_VF12z, SONYVU_VF12w,
	SONYVU_VF13x, SONYVU_VF13y, SONYVU_VF13z, SONYVU_VF13w,
	SONYVU_VF14x, SONYVU_VF14y, SONYVU_VF14z, SONYVU_VF14w,
	SONYVU_VF15x, SONYVU_VF15y, SONYVU_VF15z, SONYVU_VF15w,
	SONYVU_VF16x, SONYVU_VF16y, SONYVU_VF16z, SONYVU_VF16w,
	SONYVU_VF17x, SONYVU_VF17y, SONYVU_VF17z, SONYVU_VF17w,
	SONYVU_VF18x, SONYVU_VF18y, SONYVU_VF18z, SONYVU_VF18w,
	SONYVU_VF19x, SONYVU_VF19y, SONYVU_VF19z, SONYVU_VF19w,
	SONYVU_VF20x, SONYVU_VF20y, SONYVU_VF20z, SONYVU_VF20w,
	SONYVU_VF21x, SONYVU_VF21y, SONYVU_VF21z, SONYVU_VF21w,
	SONYVU_VF22x, SONYVU_VF22y, SONYVU_VF22z, SONYVU_VF22w,
	SONYVU_VF23x, SONYVU_VF23y, SONYVU_VF23z, SONYVU_VF23w,
	SONYVU_VF24x, SONYVU_VF24y, SONYVU_VF24z, SONYVU_VF24w,
	SONYVU_VF25x, SONYVU_VF25y, SONYVU_VF25z, SONYVU_VF25w,
	SONYVU_VF26x, SONYVU_VF26y, SONYVU_VF26z, SONYVU_VF26w,
	SONYVU_VF27x, SONYVU_VF27y, SONYVU_VF27z, SONYVU_VF27w,
	SONYVU_VF28x, SONYVU_VF28y, SONYVU_VF28z, SONYVU_VF28w,
	SONYVU_VF29x, SONYVU_VF29y, SONYVU_VF29z, SONYVU_VF29w,
	SONYVU_VF30x, SONYVU_VF30y, SONYVU_VF30z, SONYVU_VF30w,
	SONYVU_VF31x, SONYVU_VF31y, SONYVU_VF31z, SONYVU_VF31w,
	SONYVU_VI00,  SONYVU_VI01,  SONYVU_VI02,  SONYVU_VI03,
	SONYVU_VI04,  SONYVU_VI05,  SONYVU_VI06,  SONYVU_VI07,
	SONYVU_VI08,  SONYVU_VI09,  SONYVU_VI10,  SONYVU_VI11,
	SONYVU_VI12,  SONYVU_VI13,  SONYVU_VI14,  SONYVU_VI15,

	SONYVU0_VPU_STAT,
	SONYVU0_FBRST,
	SONYVU0_CMSAR0,
	SONYVU0_CMSAR1,

	SONYVU1_P = SONYVU0_VPU_STAT
};

class sonyvu_device : public cpu_device
{
public:
	// construction/destruction
	virtual ~sonyvu_device() {}

	void write_vu_mem(uint32_t address, uint32_t data);
	void write_micro_mem(uint32_t address, uint64_t data);
	float* vector_regs() { return m_v; }
	uint64_t *micro_mem() { return &m_micro_mem[0]; }
	uint32_t *vu_mem() { return &m_vu_mem[0]; }
	uint32_t mem_mask() const { return m_mem_mask; }

	bool running() const { return m_running; }
	void start(uint32_t address);

protected:
	enum chip_type
	{
		CHIP_TYPE_VU0,
		CHIP_TYPE_VU1,
	};

	sonyvu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor micro_cons, address_map_constructor vu_cons, chip_type chiptype, uint32_t mem_size);

	enum : uint64_t
	{
		OP_UPPER_I = 0x8000000000000000ULL,
		OP_UPPER_E = 0x4000000000000000ULL,
		OP_UPPER_M = 0x2000000000000000ULL,
		OP_UPPER_D = 0x1000000000000000ULL,
		OP_UPPER_T = 0x0800000000000000ULL
	};

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override { return 1; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 1; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override { }

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	void execute_upper(const uint32_t op);
	void execute_lower(const uint32_t op);

	virtual void execute_xgkick(uint32_t rs) = 0;

	static int16_t immediate_s11(const uint32_t op);

	// address spaces
	const address_space_config m_micro_config;
	const address_space_config m_vu_config;
	address_space   *m_micro_space;
	address_space   *m_vu_space;

	// core registers
	uint32_t        m_mem_size;
	uint32_t        m_mem_mask;
	required_shared_ptr<uint64_t> m_micro_mem;
	required_shared_ptr<uint32_t> m_vu_mem;
	float*          m_vfmem;
	uint32_t*       m_vimem;

	float           m_vfr[32][4]; // 0..3 = x..w
	uint32_t        m_vcr[32];
	float           m_acc[4];

	float*          m_v;

	uint32_t        m_status_flag;
	uint32_t        m_mac_flag;
	uint32_t        m_clip_flag;
	uint32_t        m_r;
	float           m_i;
	float           m_q;

	uint32_t        m_pc;
	uint32_t        m_delay_pc;
	uint32_t        m_start_pc;

	bool            m_running;

	int             m_icount;
};

class sonyvu1_device : public sonyvu_device
{
public:
	template <typename T>
	sonyvu1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&gs_tag)
		: sonyvu1_device(mconfig, tag, owner, clock)
	{
		m_gs.set_tag(std::forward<T>(gs_tag));
	}

	sonyvu1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	ps2_vif1_device* interface();

	uint64_t vif_r(offs_t offset);
	void vif_w(offs_t offset, uint64_t data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	void micro_map(address_map &map) ATTR_COLD;
	void vu_map(address_map &map) ATTR_COLD;

	void execute_xgkick(uint32_t rs) override;

	required_device<ps2_gs_device> m_gs;
	required_device<ps2_vif1_device> m_vif;

	float m_p;
};

class sonyvu0_device : public sonyvu_device
{
public:
	template <typename T>
	sonyvu0_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&vu1_tag)
		: sonyvu0_device(mconfig, tag, owner, clock)
	{
		m_vu1.set_tag(std::forward<T>(vu1_tag));
	}

	sonyvu0_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void micro_map(address_map &map) ATTR_COLD;
	void vu_map(address_map &map) ATTR_COLD;

	void execute_xgkick(uint32_t rs) override;

	uint32_t vu1_reg_r(offs_t offset);
	void vu1_reg_w(offs_t offset, uint32_t data);

	required_device<sonyvu1_device> m_vu1;

	float*          m_vu1_regs;
	uint32_t        m_control;
	uint32_t        m_vpu_stat;
	uint32_t        m_cmsar0;
	uint32_t        m_cmsar1;
};

DECLARE_DEVICE_TYPE(SONYPS2_VU1, sonyvu1_device)
DECLARE_DEVICE_TYPE(SONYPS2_VU0, sonyvu0_device)

#endif // MAME_CPU_MIPS_PS2VU_H
