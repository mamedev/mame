// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    esrip.h
    Interface file for the Entertainment Sciences RIP
    Written by Phil Bennett

***************************************************************************/

#ifndef _ESRIP_H
#define _ESRIP_H

#include "emu.h"

/***************************************************************************
    COMPILE-TIME DEFINITIONS
***************************************************************************/


/***************************************************************************
    GLOBAL CONSTANTS
***************************************************************************/

/***************************************************************************
    INTERFACE CONFIGURATION MACROS
***************************************************************************/
#define ESRIP_DRAW(name) int name(int l, int r, int fig, int attr, int addr, int col, int x_scale, int bank)

#define MCFG_ESRIP_FDT_R_CALLBACK(_read) \
	devcb = &esrip_device::static_set_fdt_r_callback(*device, DEVCB_##_read);

#define MCFG_ESRIP_FDT_W_CALLBACK(_write) \
	devcb = &esrip_device::static_set_fdt_w_callback(*device, DEVCB_##_write);

#define MCFG_ESRIP_STATUS_IN_CALLBACK(_read) \
	devcb = &esrip_device::static_set_status_in_callback(*device, DEVCB_##_read);

#define MCFG_ESRIP_DRAW_CALLBACK_OWNER(_class, _method) \
	esrip_device::static_set_draw_callback(*device, esrip_draw_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

#define MCFG_ESRIP_LBRM_PROM(_tag) \
	esrip_device::static_lbrm_prom(*device, _tag);


/***************************************************************************
    REGISTER ENUMERATION
***************************************************************************/

enum
{
	ESRIP_PC = 1,
	ESRIP_ACC,
	ESRIP_DLATCH,
	ESRIP_ILATCH,
	ESRIP_RAM00,
	ESRIP_RAM01,
	ESRIP_RAM02,
	ESRIP_RAM03,
	ESRIP_RAM04,
	ESRIP_RAM05,
	ESRIP_RAM06,
	ESRIP_RAM07,
	ESRIP_RAM08,
	ESRIP_RAM09,
	ESRIP_RAM0A,
	ESRIP_RAM0B,
	ESRIP_RAM0C,
	ESRIP_RAM0D,
	ESRIP_RAM0E,
	ESRIP_RAM0F,
	ESRIP_RAM10,
	ESRIP_RAM11,
	ESRIP_RAM12,
	ESRIP_RAM13,
	ESRIP_RAM14,
	ESRIP_RAM15,
	ESRIP_RAM16,
	ESRIP_RAM17,
	ESRIP_RAM18,
	ESRIP_RAM19,
	ESRIP_RAM1A,
	ESRIP_RAM1B,
	ESRIP_RAM1C,
	ESRIP_RAM1D,
	ESRIP_RAM1E,
	ESRIP_RAM1F,
	ESRIP_STATW,
	ESRIP_FDTC,
	ESRIP_IPTC,
	ESRIP_XSCALE,
	ESRIP_YSCALE,
	ESRIP_BANK,
	ESRIP_LINE,
	ESRIP_FIG,
	ESRIP_ATTR,
	ESRIP_ADRL,
	ESRIP_ADRR,
	ESRIP_COLR,
	ESRIP_IADDR
};


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************
typedef device_delegate<int (int l, int r, int fig, int attr, int addr, int col, int x_scale, int bank)> esrip_draw_delegate;

// device type definition
extern const device_type ESRIP;

// ======================> esrip_device

// Used by core CPU interface
class esrip_device : public cpu_device
{
public:
	// construction/destruction
	esrip_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// inline configuration helpers
	template<class _Object> static devcb_base &static_set_fdt_r_callback(device_t &device, _Object object) { return downcast<esrip_device &>(device).m_fdt_r.set_callback(object); }
	template<class _Object> static devcb_base &static_set_fdt_w_callback(device_t &device, _Object object) { return downcast<esrip_device &>(device).m_fdt_w.set_callback(object); }
	template<class _Object> static devcb_base &static_set_status_in_callback(device_t &device, _Object object) { return downcast<esrip_device &>(device).m_status_in.set_callback(object); }
	static void static_set_draw_callback(device_t &device, esrip_draw_delegate func) { downcast<esrip_device &>(device).m_draw = func; }
	static void static_lbrm_prom(device_t &device, const char *name) { downcast<esrip_device &>(device).m_lbrm_prom = name; }

	// public interfaces
	UINT8 get_rip_status();

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_stop() override;

	void make_ops();

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override;
	virtual UINT32 execute_max_cycles() const override;
	virtual UINT32 execute_input_lines() const override;
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override;
	virtual UINT32 disasm_max_opcode_bytes() const override;
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) override;

	// address spaces
	const address_space_config m_program_config;

	// CPU registers
	UINT16  m_ram[32];
	UINT16  m_acc;
	UINT16  m_d_latch;
	UINT16  m_i_latch;
	UINT16  m_result;
	UINT8   m_new_status;
	UINT8   m_status;
	UINT16  m_inst;
	UINT8   m_immflag;
	UINT8   m_ct;
	UINT8   m_t;

	/* Instruction latches - current and previous values */
	UINT8   m_l1, m_pl1;
	UINT8   m_l2, m_pl2;
	UINT8   m_l3, m_pl3;
	UINT8   m_l4, m_pl4;
	UINT8   m_l5, m_pl5;
	UINT8   m_l6, m_pl6;
	UINT8   m_l7, m_pl7;

	UINT8   m_pc;
	UINT16  m_rip_pc;
	UINT8   m_status_out;

	UINT8   m_x_scale;
	UINT8   m_y_scale;
	UINT8   m_img_bank;
	UINT8   m_line_latch;
	UINT16  m_fig_latch;
	UINT16  m_attr_latch;
	UINT16  m_adl_latch;
	UINT16  m_adr_latch;
	UINT16  m_iaddr_latch;
	UINT8   m_c_latch;

	UINT16  m_fdt_cnt;
	UINT16  m_ipt_cnt;

	UINT8   m_fig;
	UINT16  m_fig_cycles;

	UINT8   m_optable[65536];

	std::vector<UINT16> m_ipt_ram;
	UINT8   *m_lbrm;

	address_space *m_program;
	direct_read_data *m_direct;

	int     m_icount;

	devcb_read16  m_fdt_r;
	devcb_write16 m_fdt_w;
	devcb_read8 m_status_in;
	esrip_draw_delegate m_draw;
	const char *m_lbrm_prom;

	typedef void (esrip_device::*ophandler)(UINT16 inst);

	ophandler m_opcode[24];

	static const ophandler s_opcodetable[24];

private:
	int get_hblank() const;
	int get_lbrm() const;
	int check_jmp(UINT8 jmp_ctrl) const;

	// flags
	void calc_z_flag(UINT16 res);
	void calc_c_flag_add(UINT16 a, UINT16 b);
	void calc_c_flag_sub(UINT16 a, UINT16 b);
	void calc_n_flag(UINT16 res);
	void calc_v_flag_add(UINT16 a, UINT16 b, UINT32 r);
	void calc_v_flag_sub(UINT16 a, UINT16 b, UINT32 r);

	// opcodes
	UINT16 sor_op(UINT16 r, UINT16 opcode);
	void sor(UINT16 inst);
	void sonr(UINT16 inst);

	UINT16 tor_op(UINT16 r, UINT16 s, int opcode);
	void tonr(UINT16 inst);
	void tor1(UINT16 inst);
	void tor2(UINT16 inst);

	void bonr(UINT16 inst);
	void bor1(UINT16 inst);
	void bor2(UINT16 inst);

	void rotr1(UINT16 inst);
	void rotr2(UINT16 inst);
	void rotnr(UINT16 inst);
	void rotc(UINT16 inst);
	void rotm(UINT16 inst);

	void prt(UINT16 inst);
	void prtnr(UINT16 inst);

	void crcf(UINT16 inst);
	void crcr(UINT16 inst);

	UINT16 shift_op(UINT16 u, int opcode);
	void shftr(UINT16 inst);
	void shftnr(UINT16 inst);

	void svstr(UINT16 inst);

	void rstst(UINT16 inst);
	void setst(UINT16 inst);

	void test(UINT16 inst);

	void nop(UINT16 inst);

	void am29116_execute(UINT16 inst, int _sre);
};


CPU_DISASSEMBLE( esrip );

#endif /* _ESRIP_H */
