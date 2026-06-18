// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_EDSP_EDSP_H
#define MAME_CPU_EDSP_EDSP_H

#pragma once


class edsp_device : public cpu_device
{
protected:
	enum {
		EDSP_PC, EDSP_SP,
		EDSP_RC, EDSP_LC, EDSP_LSA, EDSP_LEA,
		EDSP_SR,
		EDSP_R0, EDSP_R1, EDSP_R2, EDSP_R3, EDSP_R4, EDSP_R5, EDSP_R6, EDSP_R7
	};

	edsp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor prog_map, address_map_constructor data_map, address_map_constructor io_map);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual space_config_vector memory_space_config() const override;

	virtual void execute_run() override;

private:
	const address_space_config m_program_config;
	const address_space_config m_data_config;
	const address_space_config m_io_config;

	s32 m_icount;

	u16 m_pc;
	u16 m_sp;
	u16 m_rcr;
	u16 m_lcr;
	u16 m_lsa;
	u16 m_lea;
	u16 m_sr;
	u16 m_r[8];
};

class emg2000a_device : public edsp_device
{
public:
	emg2000a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

private:
	void program_map(address_map &map);
	void data_map(address_map &map);
	void io_map(address_map &map);
};


DECLARE_DEVICE_TYPE(EMG2000A, emg2000a_device)

#endif // MAME_CPU_EDSP_EDSP_H
