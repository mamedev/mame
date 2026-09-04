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
		EDSP_R0, EDSP_R1, EDSP_R2, EDSP_R3, EDSP_R4, EDSP_R5, EDSP_R6, EDSP_R7,
		EDSP_INTE, EDSP_INTF
	};

	edsp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor prog_map, address_map_constructor data_map, address_map_constructor io_map);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_disasm_interface implementation
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override;

	// device_execute_interface implementation
	virtual void execute_run() override;
	virtual u64 execute_clocks_to_cycles(u64 clocks) const noexcept override { return (clocks + 2 - 1) / 2; }
	virtual u64 execute_cycles_to_clocks(u64 cycles) const noexcept override { return (cycles * 2); }

	// register access
	u16 sr_r();
	void sr_w(u16 data);
	u16 bank_r();
	void bank_w(u16 data);
	u16 inte_r(offs_t offset);
	void inte_w(offs_t offset, u16 data);
	u16 intf_r(offs_t offset);
	void intf_w(offs_t offset, u16 data);
	u16 spa_r();
	void spa_w(u16 data);
	u16 timer01_r(offs_t offset);
	void timer01_w(offs_t offset, u16 data);

private:
	TIMER_CALLBACK_MEMBER(timer01_interrupt);

	u16 add(u16 s, u16 t, bool c) noexcept;
	bool test_condition(u8 cond) const noexcept;
	u16 read_program_word(u16 addr);

	const address_space_config m_program_config;
	const address_space_config m_data_config;
	const address_space_config m_io_config;

	memory_access<24, 1, -1, ENDIANNESS_LITTLE>::cache m_cache;
	memory_access<24, 1, -1, ENDIANNESS_LITTLE>::specific m_program;
	memory_access<16, 1, -1, ENDIANNESS_LITTLE>::specific m_data;
	memory_access<7, 1, -1, ENDIANNESS_LITTLE>::specific m_io;

	s32 m_icount;

	u16 m_pc;
	u16 m_ppc;
	u16 m_sp;
	u16 m_rcr;
	u16 m_lcr;
	u16 m_lsa;
	u16 m_lea;
	u16 m_sr;
	u16 m_r[8];
	u32 m_inte;
	u32 m_intf;
	u16 m_bank;
	u8 m_trl[2];
	u16 m_tcon[2];

	emu_timer *m_timer01[2];
};

class emg2000a_device : public edsp_device
{
public:
	emg2000a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration
	auto in_pa_callback() { return m_in_pa_cb.bind(); }
	auto out_pa_callback() { return m_out_pa_cb.bind(); }
	auto in_pb_callback() { return m_in_pb_cb.bind(); }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// internal address maps
	void program_map(address_map &map);
	void data_map(address_map &map);
	void io_map(address_map &map);

	u16 porta_r();
	void porta_w(u16 data);
	u16 portb_r();
	u16 pdira_r();
	void pdira_w(u16 data);
	u16 pcona_r();
	void pcona_w(u16 data);

	// device callbacks
	devcb_read16 m_in_pa_cb;
	devcb_write16 m_out_pa_cb;
	devcb_read8 m_in_pb_cb;

	u16 m_pdata;
	u16 m_pdira;
	u16 m_pcona;
};


DECLARE_DEVICE_TYPE(EMG2000A, emg2000a_device)

#endif // MAME_CPU_EDSP_EDSP_H
