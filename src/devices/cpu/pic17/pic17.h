// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_PIC17_PIC17_H
#define MAME_CPU_PIC17_PIC17_H

#pragma once

class pic17_cpu_device : public cpu_device
{
public:
	enum {
		PIC17_PC,
		PIC17_PCLATH,
		PIC17_STKPTR,
		PIC17_TOS,
		PIC17_WREG,
		PIC17_PROD,
		PIC17_ALUSTA,
		PIC17_CPUSTA,
		PIC17_INTSTA,
		PIC17_FSR0,
		PIC17_FSR1,
		PIC17_BSR,
		PIC17_TBLPTR,
		PIC17_TABLAT,
		PIC17_TLWT,
		PIC17_T0STA,
		PIC17_TMR0,
		PIC17_PS
	};

	enum class mode {
		MICROPROCESSOR,
		MICROCONTROLLER,
		EXTENDED_MICROCONTROLLER
	};

	// misc. configuration
	void set_mode(mode m) { m_mode = m; }

protected:
	pic17_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u16 rom_size, address_map_constructor data_map);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual void execute_run() override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// internal data map
	void core_data_map(address_map &map) ATTR_COLD;

	// helpers for derived classes
	void int_edge(bool rising);
	void t0cki_edge(bool rising);
	void set_peif(bool state);
	virtual void increment_timers();

private:
	enum class exec_phase : u8 { Q1, Q2, Q3, Q4 };

	enum exec_flags : u8 {
		REGWT = 1 << 0U,
		FORCENOP = 1 << 1U,
		SKIP = 1 << 2U,
		INTRPT = 1 << 3U,
		TBLPTRI = 1 << 4U,
		TABLRD = 1 << 5U,
		TABLWT = 1 << 6U,
		SLEEP = 1 << 7U
	};

	// internal register accessors
	u8 pcl_r();
	void pcl_w(u8 data);
	void debug_set_pc(u16 data);
	u8 pclath_r();
	void pclath_w(u8 data);
	u8 wreg_r();
	void wreg_w(u8 data);
	u8 alusta_r();
	void alusta_w(u8 data);
	u8 cpusta_r();
	void cpusta_w(u8 data);
	u8 intsta_r();
	void intsta_w(u8 data);
	u8 t0sta_r();
	void t0sta_w(u8 data);
	u8 tmr0l_r();
	void tmr0l_w(u8 data);
	u8 tmr0h_r();
	void tmr0h_w(u8 data);
	void increment_tmr0();
	u8 fsr0_r();
	void fsr0_w(u8 data);
	u8 fsr1_r();
	void fsr1_w(u8 data);
	u8 bsr_r();
	void bsr_w(u8 data);
	u8 tblptrl_r();
	void tblptrl_w(u8 data);
	u8 tblptrh_r();
	void tblptrh_w(u8 data);
	u8 prodl_r();
	void prodl_w(u8 data);
	u8 prodh_r();
	void prodh_w(u8 data);

	// execution helpers
	void set_skip();
	void clear_watchdog_timer();
	u16 banked_register(u8 r);
	void stack_push(u16 addr);
	u16 stack_pop();
	u16 interrupt_vector();
	void set_zero(bool z);
	void set_carry(bool c);
	void add_with_carry(u8 augend, bool cin);
	void decimal_adjust();
	void q1_decode();
	void q2_read();
	void q3_execute();
	void q4_write();

	// address map constructor
	void program_map(address_map &map) ATTR_COLD;

	// address spaces
	const address_space_config m_program_config;
	const address_space_config m_data_config;
	const u16 m_rom_size;
	memory_access<16, 1, -1, ENDIANNESS_LITTLE>::cache m_cache;
	memory_access<16, 1, -1, ENDIANNESS_LITTLE>::specific m_program;
	memory_access<12, 0,  0, ENDIANNESS_LITTLE>::specific m_data;

	// mode configuration
	mode m_mode;

	// execution state
	u16 m_pc;
	u16 m_ppc;
	u16 m_paddr;
	u16 m_raddr;
	u16 m_ir;
	u8 m_tmp;
	exec_phase m_execphase;
	u8 m_execflags;
	s32 m_icount;

	// internal status and control registers
	u8 m_pclath;
	u8 m_wreg;
	u8 m_alusta;
	u8 m_cpusta;
	u8 m_intsta;
	u8 m_intsample;
	u16 m_tblptr;
	u16 m_tablat;
	u16 m_tlwt;
	u16 m_prod;
	u8 m_fsr[2];
	u8 m_bsr;
	u8 m_stkptr;
	u16 m_stack[16];
	u8 m_t0sta;
	u16 m_tmr0;
	u8 m_ps;
};

#endif // MAME_CPU_PIC17_PIC17_H
