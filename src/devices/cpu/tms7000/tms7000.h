// license:BSD-3-Clause
// copyright-holders:hap, Tim Lindner
/*

  Texas Instruments TMS7000

*/

#pragma once

#ifndef __TMS7000_H__
#define __TMS7000_H__

#include "emu.h"
#include "debugger.h"


enum { TMS7000_PC=1, TMS7000_SP, TMS7000_ST };

enum
{
	/* note: INT2,4,5 are generated internally */
	TMS7000_INT1_LINE = 0,
	TMS7000_INT3_LINE
};

enum
{
	TMS7000_PORTA = 0,      /* read-only on 70x0 */
	TMS7000_PORTB,          /* write-only */
	TMS7000_PORTC,
	TMS7000_PORTD,
	TMS7000_PORTE           /* TMS70C46 only */
};

// chip info flags
#define TMS7000_CHIP_IS_CMOS        0x01
#define TMS7000_CHIP_FAMILY_70X0    0x00
#define TMS7000_CHIP_FAMILY_70X2    0x02
#define TMS7000_CHIP_FAMILY_70CX2   0x04
#define TMS7000_CHIP_FAMILY_MASK    0x06


class tms7000_device : public cpu_device
{
public:
	// construction/destruction
	tms7000_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	tms7000_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, address_map_constructor internal, UINT32 info_flags, std::string shortname, std::string source);

	DECLARE_READ8_MEMBER(tms7000_unmapped_rf_r) { if (!space.debugger_access()) logerror("'%s' (%04X): unmapped_rf_r @ $%04x\n", tag().c_str(), m_pc, offset + 0x80); return 0; };
	DECLARE_WRITE8_MEMBER(tms7000_unmapped_rf_w) { logerror("'%s' (%04X): unmapped_rf_w @ $%04x = $%02x\n", tag().c_str(), m_pc, offset + 0x80, data); };

	DECLARE_READ8_MEMBER(tms7000_pf_r);
	DECLARE_WRITE8_MEMBER(tms7000_pf_w);
	DECLARE_READ8_MEMBER(tms7002_pf_r) { return tms7000_pf_r(space, offset + 0x10); }
	DECLARE_WRITE8_MEMBER(tms7002_pf_w) { tms7000_pf_w(space, offset + 0x10, data); }

	bool chip_is_cmos() { return (m_info_flags & TMS7000_CHIP_IS_CMOS) ? true : false; }
	UINT32 chip_get_family() { return m_info_flags & TMS7000_CHIP_FAMILY_MASK; }
	bool chip_is_family_70x0() { return chip_get_family() == TMS7000_CHIP_FAMILY_70X0; }
	bool chip_is_family_70x2() { return chip_get_family() == TMS7000_CHIP_FAMILY_70X2; }
	bool chip_is_family_70cx2() { return chip_get_family() == TMS7000_CHIP_FAMILY_70CX2; }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual UINT64 execute_clocks_to_cycles(UINT64 clocks) const override { return (clocks + 2 - 1) / 2; } // internal /2 divider
	virtual UINT64 execute_cycles_to_clocks(UINT64 cycles) const override { return (cycles * 2); } // internal /2 divider
	virtual UINT32 execute_min_cycles() const override { return 5; }
	virtual UINT32 execute_max_cycles() const override { return 49; }
	virtual UINT32 execute_input_lines() const override { return 2; }
	virtual void execute_run() override;
	virtual void execute_set_input(int extline, int state) override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override { return (spacenum == AS_PROGRAM) ? &m_program_config : ( (spacenum == AS_IO) ? &m_io_config : nullptr ); }

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override { return 1; }
	virtual UINT32 disasm_max_opcode_bytes() const override { return 4; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	virtual void execute_one(UINT8 op);

	address_space_config m_program_config;
	address_space_config m_io_config;

	UINT32 m_info_flags;

	address_space *m_program;
	direct_read_data *m_direct;
	address_space *m_io;
	int m_icount;

	bool m_irq_state[2];
	bool m_idle_state;
	bool m_idle_halt;
	UINT16 m_pc;
	UINT8 m_sp;
	UINT8 m_sr;
	UINT8 m_op;

	UINT8 m_io_control[3];

	emu_timer *m_timer_handle[2];
	UINT8 m_timer_data[2];
	UINT8 m_timer_control[2];
	int m_timer_decrementer[2];
	int m_timer_prescaler[2];
	UINT16 m_timer_capture_latch[2];

	UINT8 m_port_latch[4];
	UINT8 m_port_ddr[4];

	void flag_ext_interrupt(int extline);
	void check_interrupts();
	void do_interrupt(int irqline);

	TIMER_CALLBACK_MEMBER(simple_timer_cb);
	void timer_run(int tmr);
	void timer_reload(int tmr);
	void timer_tick_pre(int tmr);
	void timer_tick_low(int tmr);

	// internal read/write
	inline UINT8 read_r8(UINT8 address) { return m_program->read_byte(address); }
	inline void write_r8(UINT8 address, UINT8 data) { m_program->write_byte(address, data); }
	inline UINT16 read_r16(UINT8 address) { return m_program->read_byte((address - 1) & 0xff) << 8 | m_program->read_byte(address); }
	inline void write_r16(UINT8 address, UINT16 data) { m_program->write_byte((address - 1) & 0xff, data >> 8 & 0xff); m_program->write_byte(address, data & 0xff); }

	inline UINT8 read_p(UINT8 address) { return m_program->read_byte(0x100 + address); }
	inline void write_p(UINT8 address, UINT8 data) { m_program->write_byte(0x100 + address, data); }

	inline UINT8 read_mem8(UINT16 address) { return m_program->read_byte(address); }
	inline void write_mem8(UINT16 address, UINT8 data) { m_program->write_byte(address, data); }
	inline UINT16 read_mem16(UINT16 address) { return m_program->read_byte(address) << 8 | m_program->read_byte((address + 1) & 0xffff); }
	inline void write_mem16(UINT16 address, UINT16 data) { m_program->write_byte(address, data >> 8 & 0xff); m_program->write_byte((address + 1) & 0xffff, data & 0xff); }

	inline UINT8 imm8() { return m_direct->read_byte(m_pc++); }
	inline UINT16 imm16() { UINT16 ret = m_direct->read_byte(m_pc++) << 8; return ret | m_direct->read_byte(m_pc++); }

	inline UINT8 pull8() { return m_program->read_byte(m_sp--); }
	inline void push8(UINT8 data) { m_program->write_byte(++m_sp, data); }
	inline UINT16 pull16() { UINT16 ret = m_program->read_byte(m_sp--); return ret | m_program->read_byte(m_sp--) << 8; }
	inline void push16(UINT16 data) { m_program->write_byte(++m_sp, data >> 8 & 0xff); m_program->write_byte(++m_sp, data & 0xff); }

	// opcode handlers
	void br_dir();
	void br_inx();
	void br_ind();
	void call_dir();
	void call_inx();
	void call_ind();
	void cmpa_dir();
	void cmpa_inx();
	void cmpa_ind();
	void decd_a();
	void decd_b();
	void decd_r();
	void dint();
	void eint();
	void idle();
	void lda_dir();
	void lda_inx();
	void lda_ind();
	void ldsp();
	void movd_dir();
	void movd_inx();
	void movd_ind();
	void nop();
	void pop_a();
	void pop_b();
	void pop_r();
	void pop_st();
	void push_a();
	void push_b();
	void push_r();
	void push_st();
	void reti();
	void rets();
	void setc();
	void sta_dir();
	void sta_inx();
	void sta_ind();
	void stsp();
	void trap(UINT8 address);
	void illegal(UINT8 op);

	typedef int (tms7000_device::*op_func)(UINT8, UINT8);
	int op_clr(UINT8 param1, UINT8 param2);
	int op_dec(UINT8 param1, UINT8 param2);
	int op_inc(UINT8 param1, UINT8 param2);
	int op_inv(UINT8 param1, UINT8 param2);
	int op_rl(UINT8 param1, UINT8 param2);
	int op_rlc(UINT8 param1, UINT8 param2);
	int op_rr(UINT8 param1, UINT8 param2);
	int op_rrc(UINT8 param1, UINT8 param2);
	int op_swap(UINT8 param1, UINT8 param2);
	int op_xchb(UINT8 param1, UINT8 param2);

	int op_adc(UINT8 param1, UINT8 param2);
	int op_add(UINT8 param1, UINT8 param2);
	int op_and(UINT8 param1, UINT8 param2);
	int op_cmp(UINT8 param1, UINT8 param2);
	int op_dac(UINT8 param1, UINT8 param2);
	int op_dsb(UINT8 param1, UINT8 param2);
	int op_mpy(UINT8 param1, UINT8 param2);
	int op_mov(UINT8 param1, UINT8 param2);
	int op_or(UINT8 param1, UINT8 param2);
	int op_sbb(UINT8 param1, UINT8 param2);
	int op_sub(UINT8 param1, UINT8 param2);
	int op_xor(UINT8 param1, UINT8 param2);

	inline void shortbranch(bool check);
	inline void jmp(bool check);
	int op_djnz(UINT8 param1, UINT8 param2);
	int op_btjo(UINT8 param1, UINT8 param2);
	int op_btjz(UINT8 param1, UINT8 param2);

	void am_a(op_func op);
	void am_b(op_func op);
	void am_r(op_func op);
	void am_a2a(op_func op);
	void am_a2b(op_func op);
	void am_a2r(op_func op);
	void am_a2p(op_func op);
	void am_b2a(op_func op);
	void am_b2b(op_func op);
	void am_b2r(op_func op);
	void am_b2p(op_func op);
	void am_r2a(op_func op);
	void am_r2b(op_func op);
	void am_r2r(op_func op);
	void am_i2a(op_func op);
	void am_i2b(op_func op);
	void am_i2r(op_func op);
	void am_i2p(op_func op);
	void am_p2a(op_func op);
	void am_p2b(op_func op);
};


class tms7020_device : public tms7000_device
{
public:
	tms7020_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};


class tms7020_exl_device : public tms7000_device
{
public:
	tms7020_exl_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

protected:
	virtual void execute_one(UINT8 op) override;

private:
	void lvdp();
};


class tms7040_device : public tms7000_device
{
public:
	tms7040_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};


class tms70c00_device : public tms7000_device
{
public:
	tms70c00_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};


class tms70c20_device : public tms7000_device
{
public:
	tms70c20_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};


class tms70c40_device : public tms7000_device
{
public:
	tms70c40_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};


class tms70c46_device : public tms7000_device
{
public:
	tms70c46_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER(control_r);
	DECLARE_WRITE8_MEMBER(control_w);

	DECLARE_READ8_MEMBER(dockbus_status_r);
	DECLARE_WRITE8_MEMBER(dockbus_status_w);
	DECLARE_READ8_MEMBER(dockbus_data_r);
	DECLARE_WRITE8_MEMBER(dockbus_data_w);

	// access I/O port E if databus is disabled
	DECLARE_READ8_MEMBER(e_bus_data_r) { return (space.debugger_access()) ? 0xff : ((m_control & 0x20) ? 0xff : m_io->read_byte(TMS7000_PORTE)); }
	DECLARE_WRITE8_MEMBER(e_bus_data_w) { if (~m_control & 0x20) m_io->write_byte(TMS7000_PORTE, data); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	UINT8 m_control;
};


class tms7001_device : public tms7000_device
{
public:
	tms7001_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};


class tms7041_device : public tms7000_device
{
public:
	tms7041_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};


class tms7002_device : public tms7000_device
{
public:
	tms7002_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};


class tms7042_device : public tms7000_device
{
public:
	tms7042_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};


extern const device_type TMS7000;
extern const device_type TMS7020;
extern const device_type TMS7020_EXL;
extern const device_type TMS7040;
extern const device_type TMS70C00;
extern const device_type TMS70C20;
extern const device_type TMS70C40;
extern const device_type TMS70C46;
extern const device_type TMS7001;
extern const device_type TMS7041;
extern const device_type TMS7002;
extern const device_type TMS7042;

#endif /* __TMS7000_H__ */
