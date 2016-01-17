// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/**********************************************************************

    Sanyo LC8670

**********************************************************************/

#pragma once

#ifndef __LC8670_H__
#define __LC8670_H__


//**************************************************************************
//  DEFINITION
//**************************************************************************

enum
{
	LC8670_PC = 1,
	LC8670_SFR
};

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

// clock sources
enum
{
	LC8670_SUB_CLOCK = 0,
	LC8670_RC_CLOCK,
	LC8670_CF_CLOCK
};


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

typedef UINT32 (*lc8670_lcd_update)(device_t &device, bitmap_ind16 &bitmap, const rectangle &cliprect, UINT8* vram, bool lcd_enabled, UINT8 stad);
#define LC8670_LCD_UPDATE(name) UINT32 name(device_t &device, bitmap_ind16 &bitmap, const rectangle &cliprect, UINT8* vram, bool lcd_enabled, UINT8 stad)


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_LC8670_SET_CLOCK_SOURCES(_sub_clock, _rc_clock, _cf_clock) \
	lc8670_cpu_device::static_set_cpu_clock(*device, LC8670_SUB_CLOCK, _sub_clock); \
	lc8670_cpu_device::static_set_cpu_clock(*device, LC8670_RC_CLOCK, _rc_clock); \
	lc8670_cpu_device::static_set_cpu_clock(*device, LC8670_CF_CLOCK, _cf_clock);
#define MCFG_LC8670_BANKSWITCH_CB(_devcb) \
	devcb = &lc8670_cpu_device::static_set_bankswitch_cb(*device, DEVCB_##_devcb);

#define MCFG_LC8670_LCD_UPDATE_CB(_cb) \
	lc8670_cpu_device::static_set_lcd_update_cb(*device, _cb);


// ======================> lc8670_cpu_device

class lc8670_cpu_device : public cpu_device
{
public:
	// construction/destruction
	lc8670_cpu_device(const machine_config &mconfig, std::string _tag, device_t *_owner, UINT32 _clock);

	// public interfaces
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	// internal map handlers
	DECLARE_READ8_MEMBER(regs_r);
	DECLARE_WRITE8_MEMBER(regs_w);
	DECLARE_READ8_MEMBER(mram_r);
	DECLARE_WRITE8_MEMBER(mram_w);
	DECLARE_READ8_MEMBER(xram_r);
	DECLARE_WRITE8_MEMBER(xram_w);

	// static configuration helpers
	static void static_set_cpu_clock(device_t &device, int _source, UINT32 _clock) { downcast<lc8670_cpu_device &>(device).m_clocks[_source] = _clock; }
	static void static_set_lcd_update_cb(device_t &device, lc8670_lcd_update _cb) { downcast<lc8670_cpu_device &>(device).m_lcd_update_func = _cb; }
	template<class _Object> static devcb_base & static_set_bankswitch_cb(device_t &device, _Object object) { return downcast<lc8670_cpu_device &>(device).m_bankswitch_func.set_callback(object); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override { return 1; }
	virtual UINT32 execute_max_cycles() const override { return 7; }
	virtual UINT32 execute_input_lines() const override { return 4; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override { return 1; }
	virtual UINT32 disasm_max_opcode_bytes() const override { return 4; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

private:
	// helpers
	inline UINT8 fetch();
	inline void  push(UINT8 data);
	inline UINT8 pop();
	inline UINT8 read_data(UINT16 offset);
	inline void write_data(UINT16 offset, UINT8 data);
	inline UINT8 read_data_latch(UINT16 offset);
	inline void write_data_latch(UINT16 offset, UINT8 data);
	inline void update_port1(UINT8 data);
	inline void set_pc(UINT16 new_pc);
	inline UINT8 get_data();
	inline UINT16 get_addr();
	inline void change_clock_source();
	inline void check_p_flag();
	inline void check_p3int();
	inline void set_irq_flag(int source);
	int decode_op(UINT8 op);
	void check_irqs();
	void timer0_prescaler_tick();
	void timer0_tick(bool ext_line = false);
	void timer1_tick();
	void base_timer_tick();
	static void dasm_arg(UINT8 op, char *buffer, offs_t pc, int arg, const UINT8 *oprom, int &pos);

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

private:
	address_space_config m_program_config;
	address_space_config m_data_config;
	address_space_config m_io_config;

	address_space *     m_program;              // program space (ROM or flash)
	address_space *     m_data;                 // internal RAM/register
	address_space *     m_io;                   // I/O ports
	direct_read_data *  m_direct;

	// timers
	static const device_timer_id BASE_TIMER = 1;
	static const device_timer_id CLOCK_TIMER = 2;
	emu_timer *         m_basetimer;
	emu_timer *         m_clocktimer;

	// internal state
	int                 m_icount;
	UINT16              m_pc;
	UINT16              m_ppc;
	UINT8               m_op;
	UINT8               m_sfr[0x80];            // special function registers
	UINT8               m_mram[0x200];          // main RAM
	UINT8               m_xram[0xc6];           // XRAM
	UINT8               m_vtrbf[0x200];         // work RAM
	UINT16              m_irq_flag;
	UINT8               m_irq_lev;
	bool                m_after_reti;
	UINT8               m_p1_data;
	UINT8               m_timer0_prescaler;
	UINT8               m_timer0[2];
	UINT8               m_timer1[2];
	UINT8               m_timer1_comparator[2];
	UINT8               m_base_timer[2];
	bool                m_clock_changed;
	int                 m_input_lines[4];

	// configuration
	UINT32              m_clocks[3];            // clock sources
	devcb_write8       m_bankswitch_func;      // bankswitch CB
	lc8670_lcd_update   m_lcd_update_func;      // LCD update CB

	// interrupts vectors
	static const UINT16 s_irq_vectors[16];

	// opcodes table
	typedef int (lc8670_cpu_device::*op_handler)();
	static const op_handler s_opcode_table[80];

	// disassembler
	enum
	{
		OP_NULL,
		OP_R8,
		OP_R8RI,
		OP_R16,
		OP_RI,
		OP_A12,
		OP_A16,
		OP_I8,
		OP_B3,
		OP_D9,
		OP_D9B3,
		OP_RII8
	};

	// disasm table
	struct dasm_entry
	{
		const char *str;
		UINT8       arg1;
		UINT8       arg2;
		bool        inv;
	};
	static const dasm_entry s_dasm_table[80];
};

extern const device_type LC8670;

#endif /* __LC8670_H__ */
