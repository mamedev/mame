/**********************************************************************

    Hitachi HD61700

**********************************************************************/

#pragma once

#ifndef __HD61700_H__
#define __HD61700_H__

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_HD61700_CONFIG(_config) \
	hd61700_cpu_device::static_set_config(*device, _config);
//**************************************************************************
//  DEFINITIONS
//**************************************************************************

// class definition
class hd61700_cpu_device;

// cpu port callbacks types
typedef void   (*hd61700_lcd_control_func)(hd61700_cpu_device &device, UINT8 data);
typedef UINT8  (*hd61700_lcd_data_r_func)(hd61700_cpu_device &device);
typedef void   (*hd61700_lcd_data_w_func)(hd61700_cpu_device &device, UINT8 data);
typedef UINT16 (*hd61700_kb_r_func)(hd61700_cpu_device &device);
typedef void   (*hd61700_kb_w_func)(hd61700_cpu_device &device, UINT8 matrix);
typedef UINT8  (*hd61700_port_r_func)(hd61700_cpu_device &device);
typedef void   (*hd61700_port_w_func)(hd61700_cpu_device &device, UINT8 data);

// device config
struct hd61700_config
{
	hd61700_lcd_control_func    m_lcd_control;      //lcd control
	hd61700_lcd_data_r_func     m_lcd_data_r;       //lcd data read
	hd61700_lcd_data_w_func     m_lcd_data_w;       //lcd data write
	hd61700_kb_r_func           m_kb_r;             //keyboard matrix read
	hd61700_kb_w_func           m_kb_w;             //keyboard matrix write
	hd61700_port_r_func         m_port_r;           //8 bit port read
	hd61700_port_w_func         m_port_w;           //8 bit port write
};


// registers
enum
{
	HD61700_PC=1, HD61700_F, HD61700_SX, HD61700_SY, HD61700_SZ, HD61700_PE, HD61700_PD,
	HD61700_IB,  HD61700_UA, HD61700_IA, HD61700_IE, HD61700_TM, HD61700_IX,
	HD61700_IY,  HD61700_IZ, HD61700_US, HD61700_SS, HD61700_KY, HD61700_MAINREG,
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

class hd61700_cpu_device : public cpu_device,
							public hd61700_config
{
public:
	// construction/destruction
	hd61700_cpu_device(const machine_config &mconfig, const char *_tag, device_t *_owner, UINT32 _clock);

	static void static_set_config(device_t &device, const hd61700_config &config);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const { return 1; }
	virtual UINT32 execute_max_cycles() const { return 52; }
	virtual UINT32 execute_input_lines() const { return 6; }
	virtual void execute_run();
	virtual void execute_set_input(int inputnum, int state);

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry);
	void state_string_export(const device_state_entry &entry, astring &string);

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const { return (spacenum == AS_PROGRAM) ? &m_program_config : NULL; }

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const { return 1; }
	virtual UINT32 disasm_max_opcode_bytes() const { return 16; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);

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
