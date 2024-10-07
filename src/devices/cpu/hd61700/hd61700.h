// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/**********************************************************************

    Hitachi HD61700

**********************************************************************/

#ifndef MAME_CPU_HD61700_HD61700_H
#define MAME_CPU_HD61700_HD61700_H

#pragma once

//**************************************************************************
//  DEFINITIONS
//**************************************************************************

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
	hd61700_cpu_device(const machine_config &mconfig, const char *_tag, device_t *_owner, uint32_t _clock);

	auto lcd_ctrl() { return m_lcd_ctrl_cb.bind(); }
	auto lcd_write() { return m_lcd_write_cb.bind(); }
	auto lcd_read() { return m_lcd_read_cb.bind(); }
	auto kb_write() { return m_kb_write_cb.bind(); }
	auto kb_read() { return m_kb_read_cb.bind(); }
	auto port_write() { return m_port_write_cb.bind(); }
	auto port_read() { return m_port_read_cb.bind(); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override { return 1; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 52; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// interrupts
	bool check_irqs();

	// helpers
	void     set_pc(int32_t new_pc);
	uint8_t  read_op();
	uint8_t  mem_readbyte(uint8_t segment, uint16_t offset);
	void     mem_writebyte(uint8_t segment, uint16_t offset, uint8_t data);
	uint32_t make_18bit_addr(uint8_t segment, uint16_t offset);
	int      check_cond( uint32_t op );
	void     push(uint16_t &offset, uint8_t data);
	uint8_t  pop(uint16_t &offset);
	uint8_t  make_logic(uint8_t type, uint8_t d1, uint8_t d2);
	void     check_optional_jr(uint8_t arg);
	uint8_t  get_sir_im8(uint8_t arg);
	uint8_t  get_sir_im8(uint8_t arg, uint8_t arg1);
	int      get_sign_mreg(uint8_t op1);
	int      get_sign_im8(uint8_t op1);
	int      get_im_7(uint8_t data);
	uint16_t make_bcd_sub(uint8_t arg1, uint8_t arg2);
	uint16_t make_bcd_add(uint8_t arg1, uint8_t arg2);

	TIMER_CALLBACK_MEMBER(second_tick);

	// registers
	enum
	{
		HD61700_PC=1, HD61700_F, HD61700_SX, HD61700_SY, HD61700_SZ, HD61700_PE, HD61700_PD,
		HD61700_IB,  HD61700_UA, HD61700_IA, HD61700_IE, HD61700_TM, HD61700_IX,
		HD61700_IY,  HD61700_IZ, HD61700_US, HD61700_SS, HD61700_KY, HD61700_MAINREG
	};

	// internal state
	address_space_config m_program_config;
	emu_timer *m_sec_timer;

	offs_t         m_ppc;
	offs_t         m_curpc;
	uint16_t         m_pc;
	uint8_t          m_flags;
	uint32_t         m_fetch_addr;
	uint8_t          m_regsir[3];                         // 5bit register (sx, sy, sz)
	uint8_t          m_reg8bit[8];                        // 8bit register (pe, pd, ib, ua, ia, ie, tm, tm)
	uint16_t         m_reg16bit[8];                       // 16bit register (ix, iy, iz, us, ss, ky, ky, ky)
	uint8_t          m_regmain[0x20];                     // main registers
	uint8_t          m_irq_status;
	uint8_t          m_state;
	uint8_t          prev_ua;
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
	static constexpr int FLAG_Z     = 0x80;
	static constexpr int FLAG_C     = 0x40;
	static constexpr int FLAG_LZ    = 0x20;
	static constexpr int FLAG_UZ    = 0x10;
	static constexpr int FLAG_SW    = 0x08;
	static constexpr int FLAG_APO   = 0x04;
};

DECLARE_DEVICE_TYPE(HD61700, hd61700_cpu_device)

#endif // MAME_CPU_HD61700_HD61700_H
