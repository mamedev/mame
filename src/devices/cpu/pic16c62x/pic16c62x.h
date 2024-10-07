// license:BSD-3-Clause
// copyright-holders:Tony La Porta, Samuele Zannoli
	/**************************************************************************\
	*                  Microchip PIC16C62X Emulator                            *
	*                                                                          *
	*                          Based On                                        *
	*                  Microchip PIC16C5X Emulator                             *
	*                    Copyright Tony La Porta                               *
	*                 Originally written for the MAME project.                 *
	*                                                                          *
	*                                                                          *
	*      Addressing architecture is based on the Harvard addressing scheme.  *
	*                                                                          *
	\**************************************************************************/

#ifndef MAME_CPU_PIC16C62X_PIC16C62X_H
#define MAME_CPU_PIC16C62X_PIC16C62X_H

#pragma once




/**************************************************************************
 *  Internal Clock divisor
 *
 *  External Clock is divided internally by 4 for the instruction cycle
 *  times. (Each instruction cycle passes through 4 machine states). This
 *  is handled by the cpu execution engine.
 */

enum
{
	PIC16C62x_PC=1, PIC16C62x_STK0, PIC16C62x_STK1, PIC16C62x_STK2,
	PIC16C62x_STK3, PIC16C62x_STK4, PIC16C62x_STK5, PIC16C62x_STK6,
	PIC16C62x_STK7, PIC16C62x_FSR,  PIC16C62x_W,    PIC16C62x_ALU,
	PIC16C62x_STR,  PIC16C62x_OPT,  PIC16C62x_TMR0, PIC16C62x_PRTA,
	PIC16C62x_PRTB, PIC16C62x_WDT,  PIC16C62x_TRSA, PIC16C62x_TRSB,
	PIC16C62x_PSCL, PIC16C62x_PCLATH
};

#define PIC16C62x_T0        0


DECLARE_DEVICE_TYPE(PIC16C620,   pic16c620_device)
DECLARE_DEVICE_TYPE(PIC16C620A,  pic16c620a_device)
//DECLARE_DEVICE_TYPE(PIC16CR620A, pic16cr620a_device)
DECLARE_DEVICE_TYPE(PIC16C621,   pic16c621_device)
DECLARE_DEVICE_TYPE(PIC16C621A,  pic16c621a_device)
DECLARE_DEVICE_TYPE(PIC16C622,   pic16c622_device)
DECLARE_DEVICE_TYPE(PIC16C622A,  pic16c622a_device)


class pic16c62x_device : public cpu_device
{
public:
	auto read_a() { return m_read_port[0].bind(); }
	auto write_a() { return m_write_port[0].bind(); }

	auto read_b() { return m_read_port[1].bind(); }
	auto write_b() { return m_write_port[1].bind(); }

	void set_config(int data);

protected:
	// construction/destruction
	pic16c62x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int program_width, address_map_constructor ram_map);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint64_t execute_clocks_to_cycles(uint64_t clocks) const noexcept override { return (clocks + 4 - 1) / 4; }
	virtual uint64_t execute_cycles_to_clocks(uint64_t cycles) const noexcept override { return (cycles * 4); }
	virtual uint32_t execute_min_cycles() const noexcept override { return 1; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 2; }
	virtual void execute_run() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	address_space_config m_program_config;
	address_space_config m_data_config;

	/******************** CPU Internal Registers *******************/
	uint16_t  m_PC;
	uint16_t  m_PREVPC;     /* previous program counter */
	uint8_t   m_W;
	uint8_t   m_PCLATH;     /* 0a,8a */
	uint8_t   m_OPTION;     /* 81 */
	uint16_t  m_CONFIG;
	uint8_t   m_ALU;
	uint16_t  m_WDT;
	uint8_t   m_TMR0;
	uint8_t   m_STATUS;
	uint8_t   m_FSR;
	uint8_t   m_port_data[2];
	uint8_t   m_port_tris[2];
	uint8_t   m_INTCON;
	uint8_t   m_PIR1;
	uint8_t   m_PIE1;
	uint8_t   m_PCON;
	uint8_t   m_CMCON;
	uint8_t   m_VRCON;
	uint16_t  m_STACK[8];
	uint16_t  m_prescaler;  /* Note: this is really an 8-bit register */
	PAIR16    m_opcode;

	int     m_icount;
	int     m_reset_vector;
	int     m_delay_timer;
	uint16_t  m_temp_config;
	uint8_t   m_old_T0;
	int8_t    m_old_data;
	int     m_inst_cycles;

	memory_access<11, 1, -1, ENDIANNESS_LITTLE>::cache m_cache;
	memory_access< 8, 0,  0, ENDIANNESS_LITTLE>::specific m_data;

	devcb_read8::array<2> m_read_port;
	devcb_write8::array<2> m_write_port;

	// For debugger
	int m_debugger_temp;

	/* opcode table entry */
	typedef void (pic16c62x_device::*pic16c62x_ophandler)();
	struct pic16c62x_opcode
	{
		uint8_t   cycles;
		pic16c62x_ophandler function;
	};
	pic16c62x_opcode m_opcode_table[16384];

	/* instruction list entry */
	struct pic16c62x_instruction
	{
		const char    *format;
		pic16c62x_ophandler function;
		uint8_t   cycles;
	};
	static const pic16c62x_instruction s_instructiontable[];

	void CALCULATE_Z_FLAG();
	void CALCULATE_ADD_CARRY();
	void CALCULATE_ADD_DIGITCARRY();
	void CALCULATE_SUB_CARRY();
	void CALCULATE_SUB_DIGITCARRY();
	uint16_t POP_STACK();
	void PUSH_STACK(uint16_t data);
	uint8_t GET_REGFILE(uint8_t addr);
	void STORE_REGFILE(uint8_t addr, uint8_t data);
	void STORE_RESULT(uint8_t addr, uint8_t data);

	uint8_t tmr0_r();
	void tmr0_w(uint8_t data);
	uint8_t pcl_r();
	void pcl_w(uint8_t data);
	uint8_t status_r();
	void status_w(uint8_t data);
	uint8_t fsr_r();
	void fsr_w(uint8_t data);
	uint8_t porta_r();
	void porta_w(uint8_t data);
	uint8_t portb_r();
	void portb_w(uint8_t data);
	uint8_t pclath_r();
	void pclath_w(uint8_t data);
	uint8_t intcon_r();
	void intcon_w(uint8_t data);
	uint8_t option_r();
	void option_w(uint8_t data);
	uint8_t trisa_r();
	void trisa_w(uint8_t data);
	uint8_t trisb_r();
	void trisb_w(uint8_t data);
	uint8_t pir1_r();
	void pir1_w(uint8_t data);
	uint8_t pie1_r();
	void pie1_w(uint8_t data);
	uint8_t pcon_r();
	void pcon_w(uint8_t data);
	uint8_t cmcon_r();
	void cmcon_w(uint8_t data);
	uint8_t vrcon_r();
	void vrcon_w(uint8_t data);

	void illegal();
	void addwf();
	void addlw();
	void andwf();
	void andlw();
	void bcf();
	void bsf();
	void btfss();
	void btfsc();
	void call();
	void clrw();
	void clrf();
	void clrwdt();
	void comf();
	void decf();
	void decfsz();
	void goto_op();
	void incf();
	void incfsz();
	void iorlw();
	void iorwf();
	void movf();
	void movlw();
	void movwf();
	void nop();
	void option();
	void retlw();
	void returns();
	void retfie();
	void rlf();
	void rrf();
	void sleepic();
	void subwf();
	void sublw();
	void swapf();
	void tris();
	void xorlw();
	void xorwf();
	void build_opcode_table(void);
	void pic16c62x_reset_regs();
	void pic16c62x_soft_reset();
	void pic16c62x_update_watchdog(int counts);
	void pic16c62x_update_timer(int counts);

protected:
	void core_regs(address_map &map) ATTR_COLD;
	void pic16c620_ram(address_map &map) ATTR_COLD;
	void pic16c622_ram(address_map &map) ATTR_COLD;
	void pic16c62x_rom_10(address_map &map) ATTR_COLD;
	void pic16c62x_rom_11(address_map &map) ATTR_COLD;
	void pic16c62x_rom_9(address_map &map) ATTR_COLD;
	void pic16c620a_ram(address_map &map) ATTR_COLD;
	void pic16c622a_ram(address_map &map) ATTR_COLD;
};


class pic16c620_device : public pic16c62x_device
{
public:
	// construction/destruction
	pic16c620_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class pic16c620a_device : public pic16c62x_device
{
public:
	// construction/destruction
	pic16c620a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

/*
class pic16cr620a_device : public pic16c62x_device
{
public:
    // construction/destruction
    pic16cr620a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
}*/

class pic16c621_device : public pic16c62x_device
{
public:
	// construction/destruction
	pic16c621_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class pic16c621a_device : public pic16c62x_device
{
public:
	// construction/destruction
	pic16c621a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class pic16c622_device : public pic16c62x_device
{
public:
	// construction/destruction
	pic16c622_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class pic16c622a_device : public pic16c62x_device
{
public:
	// construction/destruction
	pic16c622a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


#endif  // MAME_CPU_PIC16C62X_PIC16C62X_H
