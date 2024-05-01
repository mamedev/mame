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
	*                                                                          *
	*  **** Change Log ****                                                    *
	*  SZ (22-Oct-2009)                                                        *
	*   - Improvements and tests                                               *
	*  SZ (2-Oct-2009)                                                         *
	*   - Internal ram and registers                                           *
	*  SZ (12-Sep-2009)                                                        *
	*   - Started working on it.                                               *
	*                                                                          *
	*                                                                          *
	*  **** TODO ****                                                          *
	*   - Finish checking opcodes/instructions                                 *
	*   - Internal devices                                                     *
	*   - Interrupts                                                           *
	*   - Everything !                                                         *
	*                                                                          *
	*  **** DONE ****                                                          *
	*   - I/O ports                                                            *
	*   - Savestates                                                           *
	*   - Internal memory                                                      *
	*   - New opcodes                                                          *
	*   - Opcode disassembly                                                   *
	*                                                                          *
	*  **** Notes (from PIC16C5X): ****                                        *
	*  PIC WatchDog Timer has a separate internal clock. For the moment, we're *
	*     basing the count on a 4MHz input clock, since 4MHz is the typical    *
	*     input frequency (but by no means always).                            *
	*  A single scaler is available for the Counter/Timer or WatchDog Timer.   *
	*     When connected to the Counter/Timer, it functions as a Prescaler,    *
	*     hence prescale overflows, tick the Counter/Timer.                    *
	*     When connected to the WatchDog Timer, it functions as a Postscaler   *
	*     hence WatchDog Timer overflows, tick the Postscaler. This scenario   *
	*     means that the WatchDog timeout occurs when the Postscaler has       *
	*     reached the scaler rate value, not when the WatchDog reaches zero.   *
	*  CLRWDT should prevent the WatchDog Timer from timing out and generating *
	*     a device reset, but how is not known. The manual also mentions that  *
	*     the WatchDog Timer can only be disabled during ROM programming, and  *
	*     no other means seem to exist???                                      *
	*                                                                          *
	\**************************************************************************/

#include "emu.h"
#include "pic16c62x.h"
#include "16c62xdsm.h"


DEFINE_DEVICE_TYPE(PIC16C620,  pic16c620_device,  "pic16c620",   "Microchip PIC16C620")
DEFINE_DEVICE_TYPE(PIC16C620A, pic16c620a_device, "pic16c620a",  "Microchip PIC16C620A")
DEFINE_DEVICE_TYPE(PIC16C621,  pic16c621_device,  "pic16c621",   "Microchip PIC16C621")
DEFINE_DEVICE_TYPE(PIC16C621A, pic16c621a_device, "pic16c621a",  "Microchip PIC16C621A")
DEFINE_DEVICE_TYPE(PIC16C622,  pic16c622_device,  "pic16c622",   "Microchip PIC16C622")
DEFINE_DEVICE_TYPE(PIC16C622A, pic16c622a_device, "pic16c622a",  "Microchip PIC16C622A")



/****************************************************************************
 *  Internal Memory Map
 ****************************************************************************/

void pic16c62x_device::pic16c62x_rom_9(address_map &map)
{
	map(0x000, 0x1ff).rom();
}

void pic16c62x_device::pic16c62x_rom_10(address_map &map)
{
	map(0x000, 0x3ff).rom();
}

void pic16c62x_device::pic16c62x_rom_11(address_map &map)
{
	map(0x000, 0x7ff).rom();
}

void pic16c62x_device::core_regs(address_map &map)
{
	map(0x01, 0x01).rw(FUNC(pic16c62x_device::tmr0_r), FUNC(pic16c62x_device::tmr0_w));
	map(0x02, 0x02).mirror(0x80).rw(FUNC(pic16c62x_device::pcl_r), FUNC(pic16c62x_device::pcl_w));
	map(0x03, 0x03).mirror(0x80).rw(FUNC(pic16c62x_device::status_r), FUNC(pic16c62x_device::status_w));
	map(0x04, 0x04).mirror(0x80).rw(FUNC(pic16c62x_device::fsr_r), FUNC(pic16c62x_device::fsr_w));
	map(0x05, 0x05).rw(FUNC(pic16c62x_device::porta_r), FUNC(pic16c62x_device::porta_w));
	map(0x06, 0x06).rw(FUNC(pic16c62x_device::portb_r), FUNC(pic16c62x_device::portb_w));
	map(0x0a, 0x0a).mirror(0x80).rw(FUNC(pic16c62x_device::pclath_r), FUNC(pic16c62x_device::pclath_w));
	map(0x0b, 0x0b).mirror(0x80).rw(FUNC(pic16c62x_device::intcon_r), FUNC(pic16c62x_device::intcon_w));
	map(0x0c, 0x0c).rw(FUNC(pic16c62x_device::pir1_r), FUNC(pic16c62x_device::pir1_w));
	map(0x1f, 0x1f).rw(FUNC(pic16c62x_device::cmcon_r), FUNC(pic16c62x_device::cmcon_w));
	map(0x81, 0x81).rw(FUNC(pic16c62x_device::option_r), FUNC(pic16c62x_device::option_w));
	map(0x85, 0x85).rw(FUNC(pic16c62x_device::trisa_r), FUNC(pic16c62x_device::trisa_w));
	map(0x86, 0x86).rw(FUNC(pic16c62x_device::trisb_r), FUNC(pic16c62x_device::trisb_w));
	map(0x8c, 0x8c).rw(FUNC(pic16c62x_device::pie1_r), FUNC(pic16c62x_device::pie1_w));
	map(0x8e, 0x8e).rw(FUNC(pic16c62x_device::pcon_r), FUNC(pic16c62x_device::pcon_w));
	map(0x9f, 0x9f).rw(FUNC(pic16c62x_device::vrcon_r), FUNC(pic16c62x_device::vrcon_w));
}

void pic16c62x_device::pic16c620_ram(address_map &map)
{
	core_regs(map);
	map(0x20, 0x6f).ram();
}

void pic16c62x_device::pic16c622_ram(address_map &map)
{
	core_regs(map);
	map(0x20, 0x7f).ram();
	map(0xa0, 0xbf).ram();
}

void pic16c62x_device::pic16c620a_ram(address_map &map)
{
	core_regs(map);
	map(0x20, 0x6f).ram();
	map(0x70, 0x7f).mirror(0x80).ram();
}

void pic16c62x_device::pic16c622a_ram(address_map &map)
{
	core_regs(map);
	map(0x20, 0x6f).ram();
	map(0x70, 0x7f).mirror(0x80).ram();
	map(0xa0, 0xbf).ram();
}


pic16c62x_device::pic16c62x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int program_width, address_map_constructor ram_map)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 16, program_width, -1
					   , ( ( program_width == 9 ) ?  address_map_constructor(FUNC(pic16c62x_device::pic16c62x_rom_9), this) :
						 ( ( program_width == 10 ) ? address_map_constructor(FUNC(pic16c62x_device::pic16c62x_rom_10), this) :
													 address_map_constructor(FUNC(pic16c62x_device::pic16c62x_rom_11), this) )))
	, m_data_config("data", ENDIANNESS_LITTLE, 8, 8, 0, ram_map)
	, m_CONFIG(0x3fff)
	, m_reset_vector(0x0)
	, m_read_port(*this, 0)
	, m_write_port(*this)
{
}


pic16c620_device::pic16c620_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pic16c62x_device(mconfig, PIC16C620, tag, owner, clock, 9, address_map_constructor(FUNC(pic16c620_device::pic16c620_ram), this))
{
}

pic16c620a_device::pic16c620a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pic16c62x_device(mconfig, PIC16C620A, tag, owner, clock, 9, address_map_constructor(FUNC(pic16c620a_device::pic16c620a_ram), this))
{
}

pic16c621_device::pic16c621_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pic16c62x_device(mconfig, PIC16C621, tag, owner, clock, 10, address_map_constructor(FUNC(pic16c621_device::pic16c620_ram), this))
{
}

pic16c621a_device::pic16c621a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pic16c62x_device(mconfig, PIC16C621A, tag, owner, clock, 10, address_map_constructor(FUNC(pic16c621a_device::pic16c620a_ram), this))
{
}

pic16c622_device::pic16c622_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pic16c62x_device(mconfig, PIC16C622, tag, owner, clock, 11, address_map_constructor(FUNC(pic16c622_device::pic16c622_ram), this))
{
}

pic16c622a_device::pic16c622a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pic16c62x_device(mconfig, PIC16C622A, tag, owner, clock, 11, address_map_constructor(FUNC(pic16c622a_device::pic16c622a_ram), this))
{
}


device_memory_interface::space_config_vector pic16c62x_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA,    &m_data_config)
	};
}

std::unique_ptr<util::disasm_interface> pic16c62x_device::create_disassembler()
{
	return std::make_unique<pic16c62x_disassembler>();
}


#define PIC16C62x_RDOP(A)         (m_cache.read_word(A))
/************  Read the state of the T0 Clock input signal  ************/
#define PIC16C62x_T0_In           ((porta_r() >> 4) & 1)

#define M_RDOP(A)       PIC16C62x_RDOP(A)
#define S_T0_IN         PIC16C62x_T0_In
#define ADDR_MASK       0x1fff



#define  RISING_EDGE_T0  (( (int)(T0_in - m_old_T0) > 0) ? 1 : 0)
#define FALLING_EDGE_T0  (( (int)(T0_in - m_old_T0) < 0) ? 1 : 0)


/********  The following is the Status Flag register definition.  *********/
			/* | 7 | 6 | 5 |  4 |  3 | 2 |  1 | 0 | */
			/* |IRP|RP1|RP0| TO | PD | Z | DC | C | */
#define IRP_FLAG    0x80    /* IRP  Register Bank Select bit (used for indirect addressing) */
#define RP1_FLAG    0x40    /* RP1  Register Bank Select bits (used for direct addressing) */
#define RP0_FLAG    0x20    /* RP0  Register Bank Select bits (used for direct addressing) */
#define TO_FLAG     0x10    /* TO   Time Out flag (WatchDog) */
#define PD_FLAG     0x08    /* PD   Power Down flag */
#define Z_FLAG      0x04    /* Z    Zero Flag */
#define DC_FLAG     0x02    /* DC   Digit Carry/Borrow flag (Nibble) */
#define C_FLAG      0x01    /* C    Carry/Borrow Flag (Byte) */

#define IRP     (m_STATUS & IRP_FLAG)
#define RP1     (m_STATUS & RP1_FLAG)
#define RP0     (m_STATUS & RP0_FLAG)
#define TO      (m_STATUS & TO_FLAG)
#define PD      (m_STATUS & PD_FLAG)
#define ZERO    (m_STATUS & Z_FLAG)
#define DC      (m_STATUS & DC_FLAG)
#define CARRY   (m_STATUS & C_FLAG)

#define ADDR    ((m_opcode.b.l & 0x7f) | (RP0 << 2))

/********  The following is the Option Flag register definition.  *********/
			/* |   7  |   6    |   5  |   4  |  3  | 2 | 1 | 0 | */
			/* | RBPU | INTEDG | TOCS | TOSE | PSA |    PS     | */
#define RBPU_FLAG   0x80    /* RBPU     Pull-up Enable */
#define INTEDG_FLAG 0x40    /* INTEDG   Interrupt Edge Select */
#define T0CS_FLAG   0x20    /* TOCS     Timer 0 clock source select */
#define T0SE_FLAG   0x10    /* TOSE     Timer 0 clock source edge select */
#define PSA_FLAG    0x08    /* PSA      Prescaler Assignment bit */
#define PS_REG      0x07    /* PS       Prescaler Rate select */

#define T0CS    (m_OPTION & T0CS_FLAG)
#define T0SE    (m_OPTION & T0SE_FLAG)
#define PSA     (m_OPTION & PSA_FLAG)
#define PS      (m_OPTION & PS_REG)

/********  The following is the Config Flag register definition.  *********/
	/* | 13 | 12 | 11 | 10 | 9 | 8 | 7 |   6   | 5 | 4 |   3   |   2  | 1 | 0 | */
	/* |           CP              |   | BODEN |  CP   | PWRTE | WDTE |  FOSC | */
	/* CP       Code Protect (ROM read protect) */
#define BODEN_FLAG  0x40    /* BODEN    Brown-out Reset Enable */
#define PWRTE_FLAG  0x08    /* PWRTE    Power-up Timer Enable */
#define WDTE_FLAG   0x04    /* WDTE     WatchDog Timer enable */
#define FOSC_FLAG   0x03    /* FOSC     Oscillator source select */

#define WDTE    (m_CONFIG & WDTE_FLAG)
#define FOSC    (m_CONFIG & FOSC_FLAG)


/************************************************************************
 *  Shortcuts
 ************************************************************************/

#define CLR(flagreg, flag) ( flagreg &= (uint8_t)(~flag) )
#define SET(flagreg, flag) ( flagreg |=  flag )


/* Easy bit position selectors */
#define POS  ((m_opcode.w >> 7) & 7)



void pic16c62x_device::CALCULATE_Z_FLAG()
{
	if (m_ALU == 0) SET(m_STATUS, Z_FLAG);
	else CLR(m_STATUS, Z_FLAG);
}

void pic16c62x_device::CALCULATE_ADD_CARRY()
{
	if ((uint8_t)(m_old_data) > (uint8_t)(m_ALU)) {
		SET(m_STATUS, C_FLAG);
	}
	else {
		CLR(m_STATUS, C_FLAG);
	}
}

void pic16c62x_device::CALCULATE_ADD_DIGITCARRY()
{
	if (((uint8_t)(m_old_data) & 0x0f) > ((uint8_t)(m_ALU) & 0x0f)) {
		SET(m_STATUS, DC_FLAG);
	}
	else {
		CLR(m_STATUS, DC_FLAG);
	}
}

void pic16c62x_device::CALCULATE_SUB_CARRY()
{
	if ((uint8_t)(m_old_data) < (uint8_t)(m_ALU)) {
		CLR(m_STATUS, C_FLAG);
	}
	else {
		SET(m_STATUS, C_FLAG);
	}
}

void pic16c62x_device::CALCULATE_SUB_DIGITCARRY()
{
	if (((uint8_t)(m_old_data) & 0x0f) < ((uint8_t)(m_ALU) & 0x0f)) {
		CLR(m_STATUS, DC_FLAG);
	}
	else {
		SET(m_STATUS, DC_FLAG);
	}
}


uint16_t pic16c62x_device::POP_STACK()
{
	uint16_t data = m_STACK[7];
	m_STACK[7] = m_STACK[6];
	m_STACK[6] = m_STACK[5];
	m_STACK[5] = m_STACK[4];
	m_STACK[4] = m_STACK[3];
	m_STACK[3] = m_STACK[2];
	m_STACK[2] = m_STACK[1];
	m_STACK[1] = m_STACK[0];
	return (data & ADDR_MASK);
}

void pic16c62x_device::PUSH_STACK(uint16_t data)
{
	m_STACK[0] = m_STACK[1];
	m_STACK[1] = m_STACK[2];
	m_STACK[2] = m_STACK[3];
	m_STACK[3] = m_STACK[4];
	m_STACK[4] = m_STACK[5];
	m_STACK[5] = m_STACK[6];
	m_STACK[6] = m_STACK[7];
	m_STACK[7] = (data & ADDR_MASK);
}



uint8_t pic16c62x_device::GET_REGFILE(uint8_t addr)    /* Read from internal memory */
{
	if ((addr & 0x7f) == 0) {               /* Indirect addressing  */
		addr = m_FSR;
	}

	return m_data.read_byte(addr);
}

void pic16c62x_device::STORE_REGFILE(uint8_t addr, uint8_t data)   /* Write to internal memory */
{
	if ((addr & 0x7f) == 0) {               /* Indirect addressing  */
		addr = m_FSR;
	}

	m_data.write_byte(addr, data);
}


void pic16c62x_device::STORE_RESULT(uint8_t addr, uint8_t data)
{
	if (m_opcode.b.l & 0x80)
	{
		STORE_REGFILE(addr, data);
	}
	else
	{
		m_W = data;
	}
}


uint8_t pic16c62x_device::tmr0_r()
{
	return m_TMR0;
}

void pic16c62x_device::tmr0_w(uint8_t data)
{
	m_delay_timer = 2;      /* Timer starts after next two instructions */
	if (PSA == 0) m_prescaler = 0;  /* Must clear the Prescaler */
	m_TMR0 = data;
}

uint8_t pic16c62x_device::pcl_r()
{
	return m_PC & 0xff;
}

void pic16c62x_device::pcl_w(uint8_t data)
{
	m_PC = (m_PCLATH << 8) | data;
}

uint8_t pic16c62x_device::status_r()
{
	return m_STATUS;
}

void pic16c62x_device::status_w(uint8_t data)
{
	m_STATUS &= (uint8_t)(~(IRP_FLAG|RP1_FLAG|RP0_FLAG));
	m_STATUS |= (data & (IRP_FLAG|RP1_FLAG|RP0_FLAG));
}

uint8_t pic16c62x_device::fsr_r()
{
	return m_FSR;
}

void pic16c62x_device::fsr_w(uint8_t data)
{
	m_FSR = data;
}

uint8_t pic16c62x_device::porta_r()
{
	uint8_t data = m_read_port[0](0, ~m_port_tris[0]);
	data &= m_port_tris[0];
	data |= ((uint8_t)(~m_port_tris[0]) & m_port_data[0]);
	return data & 0x1f;       /* 5-bit port (only lower 5 bits used) */
}

void pic16c62x_device::porta_w(uint8_t data)
{
	data &= 0x1f;       /* 5-bit port (only lower 5 bits used) */
	m_write_port[0](0, data & (~m_port_tris[0]), ~m_port_tris[0]);
	m_port_data[0] = data;
}

uint8_t pic16c62x_device::portb_r()
{
	uint8_t data = m_read_port[1](0, ~m_port_tris[1]);
	data &= m_port_tris[1];
	data |= ((uint8_t)(~m_port_tris[1]) & m_port_data[1]);
	return data;
}

void pic16c62x_device::portb_w(uint8_t data)
{
	m_write_port[1](0, data & (~m_port_tris[1]), ~m_port_tris[1]);
	m_port_data[1] = data;
}

uint8_t pic16c62x_device::pclath_r()
{
	return m_PCLATH;
}

void pic16c62x_device::pclath_w(uint8_t data)
{
	m_PCLATH = data & 0x1f;
}

uint8_t pic16c62x_device::intcon_r()
{
	return m_INTCON;
}

void pic16c62x_device::intcon_w(uint8_t data)
{
	m_INTCON = data;
}

uint8_t pic16c62x_device::option_r()
{
	return m_OPTION;
}

void pic16c62x_device::option_w(uint8_t data)
{
	m_OPTION = data;
}

uint8_t pic16c62x_device::trisa_r()
{
	return m_port_tris[0];
}

void pic16c62x_device::trisa_w(uint8_t data)
{
	data &= 0x1f;
	if (m_port_tris[0] != data)
	{
		m_port_tris[0] = data;
		m_write_port[0](0, m_port_data[0] & (~m_port_tris[0]), ~m_port_tris[0] & 0x1f);
	}
}

uint8_t pic16c62x_device::trisb_r()
{
	return m_port_tris[1];
}

void pic16c62x_device::trisb_w(uint8_t data)
{
	if (m_port_tris[1] != data)
	{
		m_port_tris[1] = data;
		m_write_port[1](0, m_port_data[1] & (~m_port_tris[1]), ~m_port_tris[1]);
	}
}

uint8_t pic16c62x_device::pir1_r()
{
	return m_PIR1;
}

void pic16c62x_device::pir1_w(uint8_t data)
{
	// Only CMIF is implemented
	m_PIR1 = data & 0x40;
}

uint8_t pic16c62x_device::pie1_r()
{
	return m_PIE1;
}

void pic16c62x_device::pie1_w(uint8_t data)
{
	// Only CMIE is implemented
	m_PIE1 = data & 0x40;
}

uint8_t pic16c62x_device::pcon_r()
{
	return m_PCON;
}

void pic16c62x_device::pcon_w(uint8_t data)
{
	m_PCON = data & 0x03;
}

uint8_t pic16c62x_device::cmcon_r()
{
	// TODO: C1OUT, C2OUT
	return m_CMCON;
}

void pic16c62x_device::cmcon_w(uint8_t data)
{
	m_CMCON = data & 0x0f;
}

uint8_t pic16c62x_device::vrcon_r()
{
	return m_VRCON;
}

void pic16c62x_device::vrcon_w(uint8_t data)
{
	m_VRCON = data & 0xef;
}

/************************************************************************
 *  Emulate the Instructions
 ************************************************************************/

/* This following function is here to fill in the void for */
/* the opcode call function. This function is never called. */


void pic16c62x_device::illegal()
{
	logerror("PIC16C62x:  PC=%03x,  Illegal opcode = %04x\n", (m_PC-1), m_opcode.w);
}


void pic16c62x_device::addwf()
{
	m_old_data = GET_REGFILE(ADDR);
	m_ALU = m_old_data + m_W;
	STORE_RESULT(ADDR, m_ALU);
	CALCULATE_Z_FLAG();
	CALCULATE_ADD_CARRY();
	CALCULATE_ADD_DIGITCARRY();
}

void pic16c62x_device::addlw()
{
	m_ALU = (m_opcode.b.l & 0xff) + m_W;
	m_W = m_ALU;
	CALCULATE_Z_FLAG();
	CALCULATE_ADD_CARRY();
	CALCULATE_ADD_DIGITCARRY();
}

void pic16c62x_device::andwf()
{
	m_ALU = GET_REGFILE(ADDR) & m_W;
	STORE_RESULT(ADDR, m_ALU);
	CALCULATE_Z_FLAG();
}

void pic16c62x_device::andlw()
{
	m_ALU = m_opcode.b.l & m_W;
	m_W = m_ALU;
	CALCULATE_Z_FLAG();
}

void pic16c62x_device::bcf()
{
	m_ALU = GET_REGFILE(ADDR);
	m_ALU &= ~(1 << POS);
	STORE_REGFILE(ADDR, m_ALU);
}

void pic16c62x_device::bsf()
{
	m_ALU = GET_REGFILE(ADDR);
	m_ALU |= 1 << POS;
	STORE_REGFILE(ADDR, m_ALU);
}

void pic16c62x_device::btfss()
{
	if ((GET_REGFILE(ADDR) & (1 << POS)) != 0)
	{
		m_PC++;
		m_inst_cycles += 1;     /* Add NOP cycles */
	}
}

void pic16c62x_device::btfsc()
{
	if ((GET_REGFILE(ADDR) & (1 << POS)) == 0)
	{
		m_PC++;
		m_inst_cycles += 1;     /* Add NOP cycles */
	}
}

void pic16c62x_device::call()
{
	PUSH_STACK(m_PC);
	m_PC = ((m_PCLATH & 0x18) << 8) | (m_opcode.w & 0x7ff);
	m_PC &= ADDR_MASK;
}

void pic16c62x_device::clrw()
{
	m_W = 0;
	SET(m_STATUS, Z_FLAG);
}

void pic16c62x_device::clrf()
{
	STORE_REGFILE(ADDR, 0);
	SET(m_STATUS, Z_FLAG);
}

void pic16c62x_device::clrwdt()
{
	m_WDT = 0;
	if (PSA) m_prescaler = 0;
	SET(m_STATUS, TO_FLAG);
	SET(m_STATUS, PD_FLAG);
}

void pic16c62x_device::comf()
{
	m_ALU = (uint8_t)(~(GET_REGFILE(ADDR)));
	STORE_RESULT(ADDR, m_ALU);
	CALCULATE_Z_FLAG();
}

void pic16c62x_device::decf()
{
	m_ALU = GET_REGFILE(ADDR) - 1;
	STORE_RESULT(ADDR, m_ALU);
	CALCULATE_Z_FLAG();
}

void pic16c62x_device::decfsz()
{
	m_ALU = GET_REGFILE(ADDR) - 1;
	STORE_RESULT(ADDR, m_ALU);
	if (m_ALU == 0)
	{
		m_PC++;
		m_inst_cycles += 1;     /* Add NOP cycles */
	}
}

void pic16c62x_device::goto_op()
{
	m_PC = ((m_PCLATH & 0x18) << 8) | (m_opcode.w & 0x7ff);
	m_PC &= ADDR_MASK;
}

void pic16c62x_device::incf()
{
	m_ALU = GET_REGFILE(ADDR) + 1;
	STORE_RESULT(ADDR, m_ALU);
	CALCULATE_Z_FLAG();
}

void pic16c62x_device::incfsz()
{
	m_ALU = GET_REGFILE(ADDR) + 1;
	STORE_RESULT(ADDR, m_ALU);
	if (m_ALU == 0)
	{
		m_PC++;
		m_inst_cycles += 1;     /* Add NOP cycles */
	}
}

void pic16c62x_device::iorlw()
{
	m_ALU = m_opcode.b.l | m_W;
	m_W = m_ALU;
	CALCULATE_Z_FLAG();
}

void pic16c62x_device::iorwf()
{
	m_ALU = GET_REGFILE(ADDR) | m_W;
	STORE_RESULT(ADDR, m_ALU);
	CALCULATE_Z_FLAG();
}

void pic16c62x_device::movf()
{
	m_ALU = GET_REGFILE(ADDR);
	STORE_RESULT(ADDR, m_ALU);
	CALCULATE_Z_FLAG();
}

void pic16c62x_device::movlw()
{
	m_W = m_opcode.b.l;
}

void pic16c62x_device::movwf()
{
	STORE_REGFILE(ADDR, m_W);
}

void pic16c62x_device::nop()
{
	/* Do nothing */
}

void pic16c62x_device::option()
{
	m_OPTION = m_W;
}

void pic16c62x_device::retlw()
{
	m_W = m_opcode.b.l;
	m_PC = POP_STACK();
}

void pic16c62x_device::returns()
{
	m_PC = POP_STACK();
}

void pic16c62x_device::retfie()
{
	m_PC = POP_STACK();
	//INTCON(7)=1;
}

void pic16c62x_device::rlf()
{
	m_old_data = GET_REGFILE(ADDR);
	m_ALU = m_old_data << 1;
	if (m_STATUS & C_FLAG) m_ALU |= 1;
	if (m_old_data & 0x80) SET(m_STATUS, C_FLAG);
	else CLR(m_STATUS, C_FLAG);
	STORE_RESULT(ADDR, m_ALU);
}

void pic16c62x_device::rrf()
{
	m_old_data = GET_REGFILE(ADDR);
	m_ALU = m_old_data >> 1;
	if (m_STATUS & C_FLAG) m_ALU |= 0x80;
	if (m_old_data & 1) SET(m_STATUS, C_FLAG);
	else CLR(m_STATUS, C_FLAG);
	STORE_RESULT(ADDR, m_ALU);
}

void pic16c62x_device::sleepic()
{
	if (WDTE) m_WDT = 0;
	if (PSA) m_prescaler = 0;
	SET(m_STATUS, TO_FLAG);
	CLR(m_STATUS, PD_FLAG);
}

void pic16c62x_device::subwf()
{
	m_old_data = GET_REGFILE(ADDR);
	m_ALU = m_old_data - m_W;
	STORE_RESULT(ADDR, m_ALU);
	CALCULATE_Z_FLAG();
	CALCULATE_SUB_CARRY();
	CALCULATE_SUB_DIGITCARRY();
}

void pic16c62x_device::sublw()
{
	m_old_data = m_opcode.b.l;
	m_ALU = m_old_data - m_W;
	m_W = m_ALU;
	CALCULATE_Z_FLAG();
	CALCULATE_SUB_CARRY();
	CALCULATE_SUB_DIGITCARRY();
}

void pic16c62x_device::swapf()
{
	m_ALU = GET_REGFILE(ADDR);
	m_ALU = (m_ALU << 4) | (m_ALU >> 4);
	STORE_RESULT(ADDR, m_ALU);
}

void pic16c62x_device::tris()
{
	switch(m_opcode.b.l & 0x7)
	{
		case 5:     m_data.write_byte(0x85, m_W); break;
		case 6:     m_data.write_byte(0x86, m_W); break;
		default:    illegal(); break;
	}
}

void pic16c62x_device::xorlw()
{
	m_ALU = m_W ^ m_opcode.b.l;
	m_W = m_ALU;
	CALCULATE_Z_FLAG();
}

void pic16c62x_device::xorwf()
{
	m_ALU = GET_REGFILE(ADDR) ^ m_W;
	STORE_RESULT(ADDR, m_ALU);
	CALCULATE_Z_FLAG();
}


/***********************************************************************
 *  Instruction Table (Format, Instruction, Cycles)
 ***********************************************************************/

const pic16c62x_device::pic16c62x_instruction pic16c62x_device::s_instructiontable[]=
{
	{"000111dfffffff", &pic16c62x_device::addwf, 1},
	{"000101dfffffff", &pic16c62x_device::andwf, 1},
	{"0000011fffffff", &pic16c62x_device::clrf, 1},
	{"00000100000011", &pic16c62x_device::clrw, 1},
	{"001001dfffffff", &pic16c62x_device::comf, 1},
	{"000011dfffffff", &pic16c62x_device::decf, 1},
	{"001011dfffffff", &pic16c62x_device::decfsz, 1},
	{"001010dfffffff", &pic16c62x_device::incf, 1},
	{"001111dfffffff", &pic16c62x_device::incfsz, 1},
	{"000100dfffffff", &pic16c62x_device::iorwf, 1},
	{"001000dfffffff", &pic16c62x_device::movf, 1},
	{"0000001fffffff", &pic16c62x_device::movwf, 1},
	{"0000000xx00000", &pic16c62x_device::nop, 1},
	{"001101dfffffff", &pic16c62x_device::rlf, 1},
	{"001100dfffffff", &pic16c62x_device::rrf, 1},
	{"000010dfffffff", &pic16c62x_device::subwf, 1},
	{"001110dfffffff", &pic16c62x_device::swapf, 1},
	{"000110dfffffff", &pic16c62x_device::xorwf, 1},
	{"0100bbbfffffff", &pic16c62x_device::bcf, 1},
	{"0101bbbfffffff", &pic16c62x_device::bsf, 1},
	{"0110bbbfffffff", &pic16c62x_device::btfsc, 1},
	{"0111bbbfffffff", &pic16c62x_device::btfss, 1},
	{"11111xkkkkkkkk", &pic16c62x_device::addlw, 1},
	{"111001kkkkkkkk", &pic16c62x_device::andlw, 1},
	{"100aaaaaaaaaaa", &pic16c62x_device::call, 2},
	{"101aaaaaaaaaaa", &pic16c62x_device::goto_op, 2},
	{"111000kkkkkkkk", &pic16c62x_device::iorlw, 1},
	{"1100xxkkkkkkkk", &pic16c62x_device::movlw, 1},
	{"00000000001001", &pic16c62x_device::retfie, 2},
	{"1101xxkkkkkkkk", &pic16c62x_device::retlw, 2},
	{"00000000001000", &pic16c62x_device::returns, 2},
	{"00000001100011", &pic16c62x_device::sleepic, 1},
	{"11110xkkkkkkkk", &pic16c62x_device::sublw, 1},
	{"111010kkkkkkkk", &pic16c62x_device::xorlw, 1},
	{"00000001100100", &pic16c62x_device::clrwdt, 1},
	{"00000001100010", &pic16c62x_device::option, 1},      // deprecated
	{"00000001100fff", &pic16c62x_device::tris, 1},        // deprecated
	{nullptr, nullptr, 0}
};

/***********************************************************************
 *  Opcode Table build function
 ***********************************************************************/

void pic16c62x_device::build_opcode_table(void)
{
	int instr,mask,bits;
	int a;

	// defaults
	for ( a = 0; a < 0x4000; a++)
	{
		m_opcode_table[a].cycles = 0;
		m_opcode_table[a].function = &pic16c62x_device::illegal;
	}
	// build table
	for( instr = 0; s_instructiontable[instr].cycles != 0; instr++)
	{
		bits=0;
		mask=0;
		for ( a = 0; a < 14; a++)
		{
			switch (s_instructiontable[instr].format[a])
			{
				case '0':
					bits = bits << 1;
					mask = (mask << 1) | 1;
					break;
				case '1':
					bits = (bits << 1) | 1;
					mask = (mask << 1) | 1;
					break;
				default:
					bits = bits << 1;
					mask = mask << 1;
					break;
			}
		}
		for ( a = 0; a < 0x4000; a++)
		{
			if (((a & mask) == bits) && (m_opcode_table[a].cycles == 0))
			{
				m_opcode_table[a].cycles = s_instructiontable[instr].cycles;
				m_opcode_table[a].function = s_instructiontable[instr].function;
			}
		}
	}
}

/****************************************************************************
 *  Inits CPU emulation
 ****************************************************************************/

void pic16c62x_device::device_start()
{
	space(AS_PROGRAM).cache(m_cache);
	space(AS_DATA).specific(m_data);

	build_opcode_table();

	// Set largely arbitrary initial values for special registers
	m_W = 0;
	m_ALU = 0;
	m_WDT = 0;
	m_TMR0 = 0;
	m_STATUS = 0x18;
	m_FSR = 0;
	m_port_data[0] = 0;
	m_port_data[1] = 0;
	m_PCON = 0;
	m_INTCON = 0;
	std::fill(std::begin(m_STACK), std::end(m_STACK), 0);

	save_item(NAME(m_W));
	save_item(NAME(m_ALU));
	save_item(NAME(m_OPTION));
	save_item(NAME(m_PCLATH));
	save_item(NAME(m_TMR0));
	save_item(NAME(m_STATUS));
	save_item(NAME(m_FSR));
	save_item(NAME(m_port_data));
	save_item(NAME(m_port_tris));
	save_item(NAME(m_INTCON));
	save_item(NAME(m_PIR1));
	save_item(NAME(m_PIE1));
	save_item(NAME(m_PCON));
	save_item(NAME(m_CMCON));
	save_item(NAME(m_VRCON));
	save_item(NAME(m_old_T0));
	save_item(NAME(m_old_data));
	save_item(NAME(m_WDT));
	save_item(NAME(m_prescaler));
	save_item(NAME(m_STACK));
	save_item(NAME(m_PC));
	save_item(NAME(m_PREVPC));
	save_item(NAME(m_CONFIG));
	save_item(NAME(m_opcode.w));
	save_item(NAME(m_delay_timer));
	save_item(NAME(m_reset_vector));

	save_item(NAME(m_temp_config));
	save_item(NAME(m_inst_cycles));

	state_add( PIC16C62x_PC,   "PC",   m_PC).mask(0xfff).formatstr("%03X");
	state_add( PIC16C62x_W,    "W",    m_W).formatstr("%02X");
	state_add( PIC16C62x_ALU,  "ALU",  m_ALU).formatstr("%02X");
	state_add( PIC16C62x_STR,  "STR",  m_debugger_temp).mask(0xff).callimport().callexport().formatstr("%02X");
	state_add( PIC16C62x_TMR0, "TMR",  m_debugger_temp).mask(0xff).callimport().callexport().formatstr("%02X");
	state_add( PIC16C62x_WDT,  "WDT",  m_WDT).formatstr("%04X");
	state_add( PIC16C62x_OPT,  "OPT",  m_OPTION).formatstr("%02X");
	state_add( PIC16C62x_STK0, "STK0", m_STACK[0]).mask(0xfff).formatstr("%03X");
	state_add( PIC16C62x_STK1, "STK1", m_STACK[1]).mask(0xfff).formatstr("%03X");
	state_add( PIC16C62x_STK2, "STK2", m_STACK[2]).mask(0xfff).formatstr("%03X");
	state_add( PIC16C62x_STK3, "STK3", m_STACK[3]).mask(0xfff).formatstr("%03X");
	state_add( PIC16C62x_STK4, "STK4", m_STACK[4]).mask(0xfff).formatstr("%03X");
	state_add( PIC16C62x_STK5, "STK5", m_STACK[5]).mask(0xfff).formatstr("%03X");
	state_add( PIC16C62x_STK6, "STK6", m_STACK[6]).mask(0xfff).formatstr("%03X");
	state_add( PIC16C62x_STK7, "STK7", m_STACK[7]).mask(0xfff).formatstr("%03X");
	state_add( PIC16C62x_PRTA, "PRTA", m_debugger_temp).mask(0x1f).callimport().callexport().formatstr("%02X");
	state_add( PIC16C62x_PRTB, "PRTB", m_debugger_temp).mask(0xff).callimport().callexport().formatstr("%02X");
	state_add( PIC16C62x_TRSA, "TRSA", m_port_tris[0]).mask(0x1f).formatstr("%02X");
	state_add( PIC16C62x_TRSB, "TRSB", m_port_tris[1]).formatstr("%02X");
	state_add( PIC16C62x_FSR,  "FSR",  m_debugger_temp).mask(0xff).callimport().callexport().formatstr("%02X");
	state_add( PIC16C62x_PSCL, "PSCL", m_debugger_temp).callimport().formatstr("%3s");
	state_add( PIC16C62x_PCLATH, "PCLATH", m_PCLATH).mask(0x1f);

	state_add( STATE_GENPC, "GENPC", m_PC).noshow();
	state_add( STATE_GENPCBASE, "CURPC", m_PREVPC).noshow();
	state_add( STATE_GENFLAGS, "GENFLAGS", m_OPTION).formatstr("%13s").noshow();

	set_icountptr(m_icount);
}

void pic16c62x_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case PIC16C62x_STR:
			m_STATUS = m_debugger_temp;
			break;
		case PIC16C62x_TMR0:
			m_TMR0 = m_debugger_temp;
			break;
		case PIC16C62x_PRTA:
			m_port_data[0] = m_debugger_temp;
			break;
		case PIC16C62x_PRTB:
			m_port_data[1] = m_debugger_temp;
			break;
		case PIC16C62x_FSR:
			m_FSR = m_debugger_temp;
			break;
		case PIC16C62x_PSCL:
			m_prescaler = m_debugger_temp;
			break;
	}
}

void pic16c62x_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case PIC16C62x_STR:
			m_debugger_temp = m_STATUS;
			break;
		case PIC16C62x_TMR0:
			m_debugger_temp = m_TMR0;
			break;
		case PIC16C62x_PRTA:
			m_debugger_temp = m_port_data[0] & 0x1f;
			break;
		case PIC16C62x_PRTB:
			m_debugger_temp = m_port_data[1];
			break;
		case PIC16C62x_FSR:
			m_debugger_temp = m_FSR;
			break;
	}
}

void pic16c62x_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case PIC16C62x_PSCL:
			str = string_format("%c%02X", ((m_OPTION & 0x08) ? 'W' : 'T'), m_prescaler);
			break;

		case STATE_GENFLAGS:
			str = string_format("%01x%c%c%c%c%c %c%c%c%03x",
				(m_STATUS & 0xe0) >> 5,
				m_STATUS & 0x10 ? '.':'O',      /* WDT Overflow */
				m_STATUS & 0x08 ? 'P':'D',      /* Power/Down */
				m_STATUS & 0x04 ? 'Z':'.',      /* Zero */
				m_STATUS & 0x02 ? 'c':'b',      /* Nibble Carry/Borrow */
				m_STATUS & 0x01 ? 'C':'B',      /* Carry/Borrow */

				m_OPTION & 0x20 ? 'C':'T',      /* Counter/Timer */
				m_OPTION & 0x10 ? 'N':'P',      /* Negative/Positive */
				m_OPTION & 0x08 ? 'W':'T',      /* WatchDog/Timer */
				m_OPTION & 0x08 ? (1<<(m_OPTION&7)) : (2<<(m_OPTION&7)) );
			break;
	}
}

/****************************************************************************
 *  Reset registers to their initial values
 ****************************************************************************/

void pic16c62x_device::pic16c62x_reset_regs()
{
	m_PC     = m_reset_vector;
	m_port_tris[0] = 0x1f;
	m_port_tris[1] = 0xff;
	m_OPTION = 0xff;
	m_STATUS &= 0x1f;
	m_port_data[0] &= 0x10;
	m_prescaler = 0;
	m_delay_timer = 0;
	m_old_T0 = 0;
	m_inst_cycles = 0;
	m_PCLATH = 0;
	m_INTCON &= 0x01;
	m_PIR1 = 0;
	m_PIE1 = 0;
	m_CMCON = 0;
	m_VRCON = 0;
}

void pic16c62x_device::pic16c62x_soft_reset()
{
	SET(m_STATUS, (TO_FLAG | PD_FLAG | Z_FLAG | DC_FLAG | C_FLAG));
	pic16c62x_reset_regs();
}

void pic16c62x_device::set_config(int data)
{
	logerror("Writing %04x to the PIC16C62x configuration bits\n",data);
	m_CONFIG = (data & 0x3fff);
}


/****************************************************************************
 *  WatchDog
 ****************************************************************************/

void pic16c62x_device::pic16c62x_update_watchdog(int counts)
{
	/* TODO: needs updating */
	/* WatchDog is set up to count 18,000 (0x464f hex) ticks to provide */
	/* the timeout period of 0.018ms based on a 4MHz input clock. */
	/* Note: the 4MHz clock should be divided by the PIC16C5x_CLOCK_DIVIDER */
	/* which effectively makes the PIC run at 1MHz internally. */

	/* If the current instruction is CLRWDT or SLEEP, don't update the WDT */

	if ((m_opcode.w != 0x64) && (m_opcode.w != 0x63))
	{
		uint16_t old_WDT = m_WDT;

		m_WDT -= counts;

		if (m_WDT > 0x464f) {
			m_WDT = 0x464f - (0xffff - m_WDT);
		}

		if (((old_WDT != 0) && (old_WDT < m_WDT)) || (m_WDT == 0))
		{
			if (PSA) {
				m_prescaler++;
				if (m_prescaler >= (1 << PS)) { /* Prescale values from 1 to 128 */
					m_prescaler = 0;
					CLR(m_STATUS, TO_FLAG);
					pic16c62x_soft_reset();
				}
			}
			else {
				CLR(m_STATUS, TO_FLAG);
				pic16c62x_soft_reset();
			}
		}
	}
}


/****************************************************************************
 *  Update Timer
 ****************************************************************************/

void pic16c62x_device::pic16c62x_update_timer(int counts)
{
	if (PSA == 0) {
		m_prescaler += counts;
		if (m_prescaler >= (2 << PS)) { /* Prescale values from 2 to 256 */
			m_TMR0 += (m_prescaler / (2 << PS));
			m_prescaler %= (2 << PS);   /* Overflow prescaler */
		}
	}
	else {
		m_TMR0 += counts;
	}
}


/****************************************************************************
 *  Execute IPeriod. Return 0 if emulation should be stopped
 ****************************************************************************/

void pic16c62x_device::execute_run()
{
	uint8_t T0_in;

	do
	{
		if (PD == 0)                        /* Sleep Mode */
		{
			m_inst_cycles = 1;
			debugger_instruction_hook(m_PC);
			if (WDTE) {
				pic16c62x_update_watchdog(1);
			}
		}
		else
		{
			m_PREVPC = m_PC;

			debugger_instruction_hook(m_PC);

			m_opcode.w = M_RDOP(m_PC);
			m_PC++;

			m_inst_cycles = m_opcode_table[m_opcode.w & 0x3fff].cycles;
			(this->*m_opcode_table[m_opcode.w & 0x3fff].function)();

			if (T0CS) {                     /* Count mode */
				T0_in = S_T0_IN;
				if (T0_in) T0_in = 1;
				if (T0SE) {                 /* Count falling edge T0 input */
					if (FALLING_EDGE_T0) {
						pic16c62x_update_timer(1);
					}
				}
				else {                      /* Count rising edge T0 input */
					if (RISING_EDGE_T0) {
						pic16c62x_update_timer(1);
					}
				}
				m_old_T0 = T0_in;
			}
			else {                          /* Timer mode */
				if (m_delay_timer) {
					m_delay_timer--;
				}
				else {
					pic16c62x_update_timer(m_inst_cycles);
				}
			}
			if (WDTE) {
				pic16c62x_update_watchdog(m_inst_cycles);
			}
		}

		m_icount -= m_inst_cycles;

	} while (m_icount > 0);
}


void pic16c62x_device::device_reset()
{
	pic16c62x_reset_regs();
	SET(m_STATUS, (TO_FLAG | PD_FLAG));
}
