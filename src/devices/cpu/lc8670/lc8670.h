// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/**********************************************************************

    Sanyo LC8670

**********************************************************************/

#ifndef MAME_CPU_LC8670_LC8670_H
#define MAME_CPU_LC8670_LC8670_H

#pragma once


//**************************************************************************
//  DEFINITION
//**************************************************************************

// input ports
enum
{
	LC8670_PORT1,       // 8-bit I/O port
	LC8670_PORT3,       // 8-bit I/O port
	LC8670_PORT7        // 4-bit I port
};

// external input lines
enum
{
	LC8670_EXT_INT0 = 0,    // P70
	LC8670_EXT_INT1,        // P71
	LC8670_EXT_INT2,        // P72
	LC8670_EXT_INT3         // P73
};

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

#define LC8670_LCD_UPDATE(name) uint32_t name(bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t* vram, bool lcd_enabled, uint8_t stad)

// ======================> lc8670_cpu_device

class lc8670_cpu_device : public cpu_device
{
public:
	enum class clock_source
	{
		SUB = 0,
		RC,
		CF
	};

	typedef device_delegate<uint32_t (bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t* vram, bool lcd_enabled, uint8_t stad)> lcd_update_delegate;

	// construction/destruction
	lc8670_cpu_device(const machine_config &mconfig, const char *_tag, device_t *_owner, uint32_t _clock);

	// public interfaces
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	// internal map handlers
	uint8_t regs_r(offs_t offset);
	void regs_w(offs_t offset, uint8_t data);
	uint8_t mram_r(offs_t offset);
	void mram_w(offs_t offset, uint8_t data);
	uint8_t xram_r(offs_t offset);
	void xram_w(offs_t offset, uint8_t data);

	// configuration helpers
	void set_cpu_clock(clock_source source, uint32_t clock) { m_clocks[unsigned(source)] = clock; }
	void set_cpu_clock(clock_source source, const XTAL &clock) { set_cpu_clock(source, clock.value()); }
	template <typename T, typename U, typename V>
	void set_clock_sources(T &&sub_clock, U &&rc_clock, V &&cf_clock)
	{
		set_cpu_clock(lc8670_cpu_device::clock_source::SUB, std::forward<T>(sub_clock));
		set_cpu_clock(lc8670_cpu_device::clock_source::RC, std::forward<U>(rc_clock));
		set_cpu_clock(lc8670_cpu_device::clock_source::CF, std::forward<V>(cf_clock));
	}

	auto bank_cb() { return m_bankswitch_func.bind(); }

	template <typename... T> void set_lcd_update_cb(T &&... args) { m_lcd_update_func.set(std::forward<T>(args)...); }

	void lc8670_internal_map(address_map &map) ATTR_COLD;
protected:
	enum
	{
		LC8670_PC = 1,
		LC8670_SFR
	};

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override { return 1; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 7; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	TIMER_CALLBACK_MEMBER(base_timer_update);
	TIMER_CALLBACK_MEMBER(clock_timer_update);

private:
	// helpers
	inline uint8_t fetch();
	inline void  push(uint8_t data);
	inline uint8_t pop();
	inline uint8_t read_data(uint16_t offset);
	inline void write_data(uint16_t offset, uint8_t data);
	inline uint8_t read_data_latch(uint16_t offset);
	inline void write_data_latch(uint16_t offset, uint8_t data);
	inline void update_port1(uint8_t data);
	inline void set_pc(uint16_t new_pc);
	inline uint8_t get_data();
	inline uint16_t get_addr();
	inline void change_clock_source();
	inline void check_p_flag();
	inline void check_p3int();
	inline void set_irq_flag(int source);
	int decode_op(uint8_t op);
	void check_irqs();
	void timer0_prescaler_tick();
	void timer0_tick(bool ext_line = false);
	void timer1_tick();
	void base_timer_tick();

	// opcodes handlers
	int op_nop();
	int op_br();
	int op_ld();
	int op_call();
	int op_callr();
	int op_brf();
	int op_st();
	int op_callf();
	int op_jmpf();
	int op_mov();
	int op_jmp();
	int op_mul();
	int op_be();
	int op_be_ri();
	int op_div();
	int op_bne();
	int op_bne_ri();
	int op_ldf();
	int op_stf();
	int op_dbnz();
	int op_bpc();
	int op_push();
	int op_inc();
	int op_bp();
	int op_pop();
	int op_dec();
	int op_bz();
	int op_add();
	int op_bn();
	int op_bnz();
	int op_addc();
	int op_ret();
	int op_sub();
	int op_not1();
	int op_reti();
	int op_subc();
	int op_ror();
	int op_ldc();
	int op_xch();
	int op_clr1();
	int op_rorc();
	int op_or();
	int op_rol();
	int op_and();
	int op_set1();
	int op_rolc();
	int op_xor();

	address_space_config  m_program_config;
	address_space_config  m_data_config;
	address_space_config  m_io_config;

	memory_access<16, 0, 0, ENDIANNESS_BIG>::cache m_cache;
	memory_access<16, 0, 0, ENDIANNESS_BIG>::specific m_program; // program space (ROM or flash)
	memory_access< 9, 0, 0, ENDIANNESS_BIG>::specific m_data;    // internal RAM/register
	memory_access< 8, 0, 0, ENDIANNESS_BIG>::specific m_io;      // I/O ports

	// timers
	emu_timer *           m_basetimer;
	emu_timer *           m_clocktimer;

	// internal state
	int                   m_icount;
	uint16_t              m_pc;
	uint16_t              m_ppc;
	uint8_t               m_op;
	uint8_t               m_sfr[0x80];            // special function registers
	uint8_t               m_mram[0x200];          // main RAM
	uint8_t               m_xram[0xc6];           // XRAM
	uint8_t               m_vtrbf[0x200];         // work RAM
	uint16_t              m_irq_flag;
	uint8_t               m_irq_lev;
	bool                  m_after_reti;
	uint8_t               m_p1_data;
	uint8_t               m_timer0_prescaler;
	uint8_t               m_timer0[2];
	uint8_t               m_timer1[2];
	uint8_t               m_timer1_comparator[2];
	uint8_t               m_base_timer[2];
	bool                  m_clock_changed;
	int                   m_input_lines[4];

	// configuration
	uint32_t              m_clocks[3];       // clock sources
	devcb_write8          m_bankswitch_func; // bankswitch CB
	lcd_update_delegate   m_lcd_update_func; // LCD update CB

	// interrupts vectors
	static const uint16_t s_irq_vectors[16];

	// opcodes table
	typedef int (lc8670_cpu_device::*op_handler)();
	static const op_handler s_opcode_table[80];
};

DECLARE_DEVICE_TYPE(LC8670, lc8670_cpu_device)

#endif // MAME_CPU_LC8670_LC8670_H
