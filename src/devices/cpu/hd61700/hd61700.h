// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/**********************************************************************

    Hitachi HD61700

**********************************************************************/

#pragma once

#ifndef __HD61700_H__
#define __HD61700_H__

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_HD61700_LCD_CTRL_CB(_devcb) \
	devcb = &hd61700_cpu_device::set_lcd_ctrl_callback(*device, DEVCB_##_devcb);

#define MCFG_HD61700_LCD_WRITE_CB(_devcb) \
	devcb = &hd61700_cpu_device::set_lcd_write_callback(*device, DEVCB_##_devcb);

#define MCFG_HD61700_LCD_READ_CB(_devcb) \
	devcb = &hd61700_cpu_device::set_lcd_read_callback(*device, DEVCB_##_devcb);

#define MCFG_HD61700_KB_WRITE_CB(_devcb) \
	devcb = &hd61700_cpu_device::set_kb_write_callback(*device, DEVCB_##_devcb);

#define MCFG_HD61700_KB_READ_CB(_devcb) \
	devcb = &hd61700_cpu_device::set_kb_read_callback(*device, DEVCB_##_devcb);

#define MCFG_HD61700_PORT_WRITE_CB(_devcb) \
	devcb = &hd61700_cpu_device::set_port_write_callback(*device, DEVCB_##_devcb);

#define MCFG_HD61700_PORT_READ_CB(_devcb) \
	devcb = &hd61700_cpu_device::set_port_read_callback(*device, DEVCB_##_devcb);


//**************************************************************************
//  DEFINITIONS
//**************************************************************************

// registers
enum
{
	HD61700_PC=1, HD61700_F, HD61700_SX, HD61700_SY, HD61700_SZ, HD61700_PE, HD61700_PD,
	HD61700_IB,  HD61700_UA, HD61700_IA, HD61700_IE, HD61700_TM, HD61700_IX,
	HD61700_IY,  HD61700_IZ, HD61700_US, HD61700_SS, HD61700_KY, HD61700_MAINREG
};

// input lines
enum
{
	HD61700_ON_INT,
	HD61700_TIMER_INT,
	HD61700_INT2,
	HD61700_KEY_INT,
	HD61700_INT1,
	HD61700_SW
};


// ======================> hd61700_cpu_device

class hd61700_cpu_device : public cpu_device
{
public:
	// construction/destruction
	hd61700_cpu_device(const machine_config &mconfig, const char *_tag, device_t *_owner, UINT32 _clock);

	template<class _Object> static devcb_base &set_lcd_ctrl_callback(device_t &device, _Object object) { return downcast<hd61700_cpu_device &>(device).m_lcd_ctrl_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_lcd_write_callback(device_t &device, _Object object) { return downcast<hd61700_cpu_device &>(device).m_lcd_write_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_lcd_read_callback(device_t &device, _Object object) { return downcast<hd61700_cpu_device &>(device).m_lcd_read_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_kb_write_callback(device_t &device, _Object object) { return downcast<hd61700_cpu_device &>(device).m_kb_write_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_kb_read_callback(device_t &device, _Object object) { return downcast<hd61700_cpu_device &>(device).m_kb_read_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_port_write_callback(device_t &device, _Object object) { return downcast<hd61700_cpu_device &>(device).m_port_write_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_port_read_callback(device_t &device, _Object object) { return downcast<hd61700_cpu_device &>(device).m_port_read_cb.set_callback(object); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override { return 1; }
	virtual UINT32 execute_max_cycles() const override { return 52; }
	virtual UINT32 execute_input_lines() const override { return 6; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override { return (spacenum == AS_PROGRAM) ? &m_program_config : nullptr; }

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override { return 1; }
	virtual UINT32 disasm_max_opcode_bytes() const override { return 16; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	// interrupts
	bool check_irqs(void);

	// inline helpers
	inline void  set_pc(INT32 new_pc);
	inline UINT8 read_op();
	inline UINT8 mem_readbyte(UINT8 segment, UINT16 offset);
	inline void  mem_writebyte(UINT8 segment, UINT16 offset, UINT8 data);
	inline UINT32 make_18bit_addr(UINT8 segment, UINT16 offset);
	inline int   check_cond( UINT32 op );
	inline void  push(UINT16 &offset, UINT8 data);
	inline UINT8 pop(UINT16 &offset);
	inline UINT8 make_logic(UINT8 type, UINT8 d1, UINT8 d2);
	inline void  check_optional_jr(UINT8 arg);
	inline UINT8 get_sir_im8(UINT8 arg);
	inline UINT8 get_sir_im8(UINT8 arg, UINT8 arg1);
	inline int   get_sign_mreg(UINT8 op1);
	inline int   get_sign_im8(UINT8 op1);
	inline int   get_im_7(UINT8 data);
	inline UINT16 make_bcd_sub(UINT8 arg1, UINT8 arg2);
	inline UINT16 make_bcd_add(UINT8 arg1, UINT8 arg2);

protected:

	// internal state
	address_space_config m_program_config;
	static const device_timer_id SEC_TIMER = 1;
	emu_timer *m_sec_timer;

	offs_t         m_ppc;
	offs_t         m_curpc;
	UINT16         m_pc;
	UINT8          m_flags;
	UINT32         m_fetch_addr;
	UINT8          m_regsir[3];                         // 5bit register (sx, sy, sz)
	UINT8          m_reg8bit[8];                        // 8bit register (pe, pd, ib, ua, ia, ie, tm, tm)
	UINT16         m_reg16bit[8];                       // 16bit register (ix, iy, iz, us, ss, ky, ky, ky)
	UINT8          m_regmain[0x20];                     // main registers
	UINT8          m_irq_status;
	UINT8          m_state;
	UINT8          prev_ua;
	int            m_lines_status[6];
	int            m_icount;

	address_space *m_program;

	devcb_write8    m_lcd_ctrl_cb;      //lcd control
	devcb_read8     m_lcd_read_cb;      //lcd data read
	devcb_write8    m_lcd_write_cb;     //lcd data write
	devcb_read16    m_kb_read_cb;       //keyboard matrix read
	devcb_write8    m_kb_write_cb;      //keyboard matrix write
	devcb_read8     m_port_read_cb;     //8 bit port read
	devcb_write8    m_port_write_cb;    //8 bit port write

	// flag definitions
	static const int FLAG_Z     = 0x80;
	static const int FLAG_C     = 0x40;
	static const int FLAG_LZ    = 0x20;
	static const int FLAG_UZ    = 0x10;
	static const int FLAG_SW    = 0x08;
	static const int FLAG_APO   = 0x04;
};

extern const device_type HD61700;


#endif /* __HD61700_H__ */
