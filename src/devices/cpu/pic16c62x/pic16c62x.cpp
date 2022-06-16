// license:BSD-3-Clause
// copyright-holders:Tony La Porta
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

void pic16c62x_device::pic16c620_ram(address_map &map)
{
	map(0x00, 0x06).ram();
	map(0x0a, 0x0c).ram();
	map(0x1f, 0x6f).ram();
	map(0x80, 0x86).ram();
	map(0x8a, 0x8e).ram();
	map(0x9f, 0x9f).ram();
}

void pic16c62x_device::pic16c622_ram(address_map &map)
{
	map(0x00, 0x06).ram();
	map(0x0a, 0x0c).ram();
	map(0x1f, 0x7f).ram();
	map(0x80, 0x86).ram();
	map(0x8a, 0x8e).ram();
	map(0x9f, 0xbf).ram();
}

// pic16c620a, pic16c621a and pic16c622a
void pic16c62x_device::pic16c62xa_ram(address_map &map)
{
	map(0x00, 0x06).ram();
	map(0x0a, 0x0c).ram();
	map(0x1f, 0x6f).ram();
	map(0x70, 0x7f).ram().share(nullptr);
	map(0x80, 0x86).ram();
	map(0x8a, 0x8e).ram();
	map(0x9f, 0xbf).ram();
	map(0xf0, 0xff).ram().share(nullptr);
}


pic16c62x_device::pic16c62x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int program_width, int picmodel)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 16, program_width, -1
					   , ( ( program_width == 9 ) ?  address_map_constructor(FUNC(pic16c62x_device::pic16c62x_rom_9), this) :
						 ( ( program_width == 10 ) ? address_map_constructor(FUNC(pic16c62x_device::pic16c62x_rom_10), this) :
													 address_map_constructor(FUNC(pic16c62x_device::pic16c62x_rom_11), this) )))
	, m_data_config("data", ENDIANNESS_LITTLE, 8, 8, 0
					, ( ( picmodel == 0x16C620 || picmodel == 0x16C621 ) ? address_map_constructor(FUNC(pic16c62x_device::pic16c620_ram), this) :
					  ( ( picmodel == 0x16C622 ) ? address_map_constructor(FUNC(pic16c62x_device::pic16c622_ram), this) :
												   address_map_constructor(FUNC(pic16c62x_device::pic16c62xa_ram), this) ) ) )
	, m_io_config("io", ENDIANNESS_LITTLE, 8, 5, 0)
	, m_CONFIG(0x3fff)
	, m_reset_vector(0x0)
	, m_picmodel(picmodel)
	, m_picRAMmask(0xff)
{
}


pic16c620_device::pic16c620_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pic16c62x_device(mconfig, PIC16C620, tag, owner, clock, 9, 0x16C620)
{
}

pic16c620a_device::pic16c620a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pic16c62x_device(mconfig, PIC16C620A, tag, owner, clock, 9, 0x16C620A)
{
}

pic16c621_device::pic16c621_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pic16c62x_device(mconfig, PIC16C621, tag, owner, clock, 10, 0x16C621)
{
}

pic16c621a_device::pic16c621a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pic16c62x_device(mconfig, PIC16C621A, tag, owner, clock, 10, 0x16C621A)
{
}

pic16c622_device::pic16c622_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pic16c62x_device(mconfig, PIC16C622, tag, owner, clock, 11, 0x16C622)
{
}

pic16c622a_device::pic16c622a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pic16c62x_device(mconfig, PIC16C622A, tag, owner, clock, 11, 0x16C622A)
{
}


device_memory_interface::space_config_vector pic16c62x_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA,    &m_data_config),
		std::make_pair(AS_IO,      &m_io_config)
	};
}

std::unique_ptr<util::disasm_interface> pic16c62x_device::create_disassembler()
{
	return std::make_unique<pic16c62x_disassembler>();
}


void pic16c62x_device::update_internalram_ptr()
{
	m_internalram = (uint8_t *)m_data.space().get_write_ptr(0x00);
}

#define PIC16C62x_RDOP(A)         (m_cache.read_word(A))
#define PIC16C62x_RAM_RDMEM(A)    ((uint8_t)m_data.read_byte(A))
#define PIC16C62x_RAM_WRMEM(A,V)  (m_data.write_byte(A,V))
#define PIC16C62x_In(Port)        ((uint8_t)m_io.read_byte((Port)))
#define PIC16C62x_Out(Port,Value) (m_io.write_byte((Port),Value))
/************  Read the state of the T0 Clock input signal  ************/
#define PIC16C62x_T0_In           (m_io.read_byte(PIC16C62x_T0) >> 4)

#define M_RDRAM(A)      (((A) == 0) ? m_internalram[0] : PIC16C62x_RAM_RDMEM(A))
#define M_WRTRAM(A,V)   do { if ((A) == 0) m_internalram[0] = (V); else PIC16C62x_RAM_WRMEM(A,V); } while (0)
#define M_RDOP(A)       PIC16C62x_RDOP(A)
#define P_IN(A)         PIC16C62x_In(A)
#define P_OUT(A,V)      PIC16C62x_Out(A,V)
#define S_T0_IN         PIC16C62x_T0_In
#define ADDR_MASK       0x1fff



#define TMR0    m_internalram[1]
#define PCL     m_internalram[2]
#define STATUS  m_internalram[3]
#define FSR     m_internalram[4]
#define PORTA   m_internalram[5]
#define PORTB   m_internalram[6]
#define INDF    M_RDRAM(FSR)

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

#define IRP     (STATUS & IRP_FLAG)
#define RP1     (STATUS & RP1_FLAG)
#define RP0     (STATUS & RP0_FLAG)
#define TO      (STATUS & TO_FLAG)
#define PD      (STATUS & PD_FLAG)
#define ZERO    (STATUS & Z_FLAG)
#define DC      (STATUS & DC_FLAG)
#define CARRY   (STATUS & C_FLAG)

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
#define POS  ((m_opcode.w.l >> 7) & 7)
static const unsigned int bit_clr[8] = { 0xfe, 0xfd, 0xfb, 0xf7, 0xef, 0xdf, 0xbf, 0x7f };
static const unsigned int bit_set[8] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };



void pic16c62x_device::CALCULATE_Z_FLAG()
{
	if (m_ALU == 0) SET(STATUS, Z_FLAG);
	else CLR(STATUS, Z_FLAG);
}

void pic16c62x_device::CALCULATE_ADD_CARRY()
{
	if ((uint8_t)(m_old_data) > (uint8_t)(m_ALU)) {
		SET(STATUS, C_FLAG);
	}
	else {
		CLR(STATUS, C_FLAG);
	}
}

void pic16c62x_device::CALCULATE_ADD_DIGITCARRY()
{
	if (((uint8_t)(m_old_data) & 0x0f) > ((uint8_t)(m_ALU) & 0x0f)) {
		SET(STATUS, DC_FLAG);
	}
	else {
		CLR(STATUS, DC_FLAG);
	}
}

void pic16c62x_device::CALCULATE_SUB_CARRY()
{
	if ((uint8_t)(m_old_data) < (uint8_t)(m_ALU)) {
		CLR(STATUS, C_FLAG);
	}
	else {
		SET(STATUS, C_FLAG);
	}
}

void pic16c62x_device::CALCULATE_SUB_DIGITCARRY()
{
	if (((uint8_t)(m_old_data) & 0x0f) < ((uint8_t)(m_ALU) & 0x0f)) {
		CLR(STATUS, DC_FLAG);
	}
	else {
		SET(STATUS, DC_FLAG);
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



uint8_t pic16c62x_device::GET_REGFILE(offs_t addr)    /* Read from internal memory */
{
	uint8_t data;

	if (addr == 0) {                        /* Indirect addressing  */
		addr = (FSR & m_picRAMmask);
	}

	switch(addr)
	{
		case 0x00:  /* Not an actual register, so return 0 */
		case 0x80:
					data = 0;
					break;
		case 0x02:
		case 0x03:
		case 0x0b:
		case 0x82:
		case 0x83:
		case 0x8b:
					data = M_RDRAM(addr & 0x7f);
					break;
		case 0x84:
		case 0x04:  data = (FSR | (uint8_t)(~m_picRAMmask));
					break;
		case 0x05:  data = P_IN(0);
					data &= m_TRISA;
					data |= ((uint8_t)(~m_TRISA) & PORTA);
					data &= 0x1f;       /* 5-bit port (only lower 5 bits used) */
					break;
		case 0x06:  data = P_IN(1);
					data &= m_TRISB;
					data |= ((uint8_t)(~m_TRISB) & PORTB);
					break;
		case 0x8a:
		case 0x0a:  data = m_PCLATH;
					break;
		case 0x81:  data = m_OPTION;
					break;
		case 0x85:  data = m_TRISA;
					break;
		case 0x86:  data = m_TRISB;
					break;
		default:    data = M_RDRAM(addr);
					break;
	}
	return data;
}

void pic16c62x_device::STORE_REGFILE(offs_t addr, uint8_t data)   /* Write to internal memory */
{
	if (addr == 0) {                        /* Indirect addressing  */
		addr = (FSR & m_picRAMmask);
	}

	switch(addr)
	{
		case 0x80:
		case 0x00:  /* Not an actual register, nothing to save */
					break;
		case 0x01:  m_delay_timer = 2;      /* Timer starts after next two instructions */
					if (PSA == 0) m_prescaler = 0;  /* Must clear the Prescaler */
					TMR0 = data;
					break;
		case 0x82:
		case 0x02:  PCL = data;
					m_PC = (m_PCLATH << 8) | data;
					break;
		case 0x83:
		case 0x03:  STATUS &= (uint8_t)(~(IRP_FLAG|RP1_FLAG|RP0_FLAG)); STATUS |= (data & (IRP_FLAG|RP1_FLAG|RP0_FLAG));
					break;
		case 0x84:
		case 0x04:  FSR = (data | (uint8_t)(~m_picRAMmask));
					break;
		case 0x05:  data &= 0x1f;       /* 5-bit port (only lower 5 bits used) */
					P_OUT(0,data & (uint8_t)(~m_TRISA)); PORTA = data;
					break;
		case 0x06:  P_OUT(1,data & (uint8_t)(~m_TRISB)); PORTB = data;
					break;
		case 0x8a:
		case 0x0a:
					m_PCLATH = data & 0x1f;
					M_WRTRAM(0x0a, m_PCLATH);
					break;
		case 0x8b:
		case 0x0b:  M_WRTRAM(0x0b, data);
					break;
		case 0x81:  m_OPTION = data;
					M_WRTRAM(0x81, data);
					break;
		case 0x85:  if   (m_TRISA != data)
					{
						m_TRISA = data | 0xf0;
						P_OUT(2,m_TRISA);
						P_OUT(0,PORTA & (uint8_t)(~m_TRISA) & 0x0f);
						M_WRTRAM(addr, data);
					}
					break;
		case 0x86:  if   (m_TRISB != data)
					{
						m_TRISB = data;
						P_OUT(3,m_TRISB);
						P_OUT(1,PORTB & (uint8_t)(~m_TRISB));
						M_WRTRAM(addr, data);
					}
					break;
		default:    M_WRTRAM(addr, data);
					break;
	}
}


void pic16c62x_device::STORE_RESULT(offs_t addr, uint8_t data)
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


/************************************************************************
 *  Emulate the Instructions
 ************************************************************************/

/* This following function is here to fill in the void for */
/* the opcode call function. This function is never called. */


void pic16c62x_device::illegal()
{
	logerror("PIC16C62x:  PC=%03x,  Illegal opcode = %04x\n", (m_PC-1), m_opcode.w.l);
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
	m_ALU &= bit_clr[POS];
	STORE_REGFILE(ADDR, m_ALU);
}

void pic16c62x_device::bsf()
{
	m_ALU = GET_REGFILE(ADDR);
	m_ALU |= bit_set[POS];
	STORE_REGFILE(ADDR, m_ALU);
}

void pic16c62x_device::btfss()
{
	if ((GET_REGFILE(ADDR) & bit_set[POS]) == bit_set[POS])
	{
		m_PC++;
		PCL = m_PC & 0xff;
		m_inst_cycles += 1;     /* Add NOP cycles */
	}
}

void pic16c62x_device::btfsc()
{
	if ((GET_REGFILE(ADDR) & bit_set[POS]) == 0)
	{
		m_PC++;
		PCL = m_PC & 0xff;
		m_inst_cycles += 1;     /* Add NOP cycles */
	}
}

void pic16c62x_device::call()
{
	PUSH_STACK(m_PC);
	m_PC = ((m_PCLATH & 0x18) << 8) | (m_opcode.w.l & 0x7ff);
	m_PC &= ADDR_MASK;
	PCL = m_PC & 0xff;
}

void pic16c62x_device::clrw()
{
	m_W = 0;
	SET(STATUS, Z_FLAG);
}

void pic16c62x_device::clrf()
{
	STORE_REGFILE(ADDR, 0);
	SET(STATUS, Z_FLAG);
}

void pic16c62x_device::clrwdt()
{
	m_WDT = 0;
	if (PSA) m_prescaler = 0;
	SET(STATUS, TO_FLAG);
	SET(STATUS, PD_FLAG);
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
		PCL = m_PC & 0xff;
		m_inst_cycles += 1;     /* Add NOP cycles */
	}
}

void pic16c62x_device::goto_op()
{
	m_PC = ((m_PCLATH & 0x18) << 8) | (m_opcode.w.l & 0x7ff);
	m_PC &= ADDR_MASK;
	PCL = m_PC & 0xff;
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
		PCL = m_PC & 0xff;
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
	PCL = m_PC & 0xff;
}

void pic16c62x_device::returns()
{
	m_PC = POP_STACK();
	PCL = m_PC & 0xff;
}

void pic16c62x_device::retfie()
{
	m_PC = POP_STACK();
	PCL = m_PC & 0xff;
	//INTCON(7)=1;
}

void pic16c62x_device::rlf()
{
	m_ALU = GET_REGFILE(ADDR);
	m_ALU <<= 1;
	if (STATUS & C_FLAG) m_ALU |= 1;
	if (GET_REGFILE(ADDR) & 0x80) SET(STATUS, C_FLAG);
	else CLR(STATUS, C_FLAG);
	STORE_RESULT(ADDR, m_ALU);
}

void pic16c62x_device::rrf()
{
	m_ALU = GET_REGFILE(ADDR);
	m_ALU >>= 1;
	if (STATUS & C_FLAG) m_ALU |= 0x80;
	if (GET_REGFILE(ADDR) & 1) SET(STATUS, C_FLAG);
	else CLR(STATUS, C_FLAG);
	STORE_RESULT(ADDR, m_ALU);
}

void pic16c62x_device::sleepic()
{
	if (WDTE) m_WDT = 0;
	if (PSA) m_prescaler = 0;
	SET(STATUS, TO_FLAG);
	CLR(STATUS, PD_FLAG);
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
	m_ALU = (m_opcode.b.l & 0xff) - m_W;
	m_W = m_ALU;
	CALCULATE_Z_FLAG();
	CALCULATE_SUB_CARRY();
	CALCULATE_SUB_DIGITCARRY();
}

void pic16c62x_device::swapf()
{
	m_ALU  = ((GET_REGFILE(ADDR) << 4) & 0xf0);
	m_ALU |= ((GET_REGFILE(ADDR) >> 4) & 0x0f);
	STORE_RESULT(ADDR, m_ALU);
}

void pic16c62x_device::tris()
{
	switch(m_opcode.b.l & 0x7)
	{
		case 5:     STORE_REGFILE(0x85, m_W); break;
		case 6:     STORE_REGFILE(0x86, m_W); break;
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
	{(char *)"000111dfffffff", &pic16c62x_device::addwf, 1},
	{(char *)"000101dfffffff", &pic16c62x_device::andwf, 1},
	{(char *)"0000011fffffff", &pic16c62x_device::clrf, 1},
	{(char *)"00000100000011", &pic16c62x_device::clrw, 1},
	{(char *)"001001dfffffff", &pic16c62x_device::comf, 1},
	{(char *)"000011dfffffff", &pic16c62x_device::decf, 1},
	{(char *)"001011dfffffff", &pic16c62x_device::decfsz, 1},
	{(char *)"001010dfffffff", &pic16c62x_device::incf, 1},
	{(char *)"001111dfffffff", &pic16c62x_device::incfsz, 1},
	{(char *)"000100dfffffff", &pic16c62x_device::iorwf, 1},
	{(char *)"001000dfffffff", &pic16c62x_device::movf, 1},
	{(char *)"0000001fffffff", &pic16c62x_device::movwf, 1},
	{(char *)"0000000xx00000", &pic16c62x_device::nop, 1},
	{(char *)"001101dfffffff", &pic16c62x_device::rlf, 1},
	{(char *)"001100dfffffff", &pic16c62x_device::rrf, 1},
	{(char *)"000010dfffffff", &pic16c62x_device::subwf, 1},
	{(char *)"001110dfffffff", &pic16c62x_device::swapf, 1},
	{(char *)"000110dfffffff", &pic16c62x_device::xorwf, 1},
	{(char *)"0100bbbfffffff", &pic16c62x_device::bcf, 1},
	{(char *)"0101bbbfffffff", &pic16c62x_device::bsf, 1},
	{(char *)"0110bbbfffffff", &pic16c62x_device::btfsc, 1},
	{(char *)"0111bbbfffffff", &pic16c62x_device::btfss, 1},
	{(char *)"11111xkkkkkkkk", &pic16c62x_device::addlw, 1},
	{(char *)"111001kkkkkkkk", &pic16c62x_device::andlw, 1},
	{(char *)"100aaaaaaaaaaa", &pic16c62x_device::call, 2},
	{(char *)"101aaaaaaaaaaa", &pic16c62x_device::goto_op, 2},
	{(char *)"111000kkkkkkkk", &pic16c62x_device::iorlw, 1},
	{(char *)"1100xxkkkkkkkk", &pic16c62x_device::movlw, 1},
	{(char *)"00000000001001", &pic16c62x_device::retfie, 2},
	{(char *)"1101xxkkkkkkkk", &pic16c62x_device::retlw, 2},
	{(char *)"00000000001000", &pic16c62x_device::returns, 2},
	{(char *)"00000001100011", &pic16c62x_device::sleepic, 1},
	{(char *)"11110xkkkkkkkk", &pic16c62x_device::sublw, 1},
	{(char *)"111010kkkkkkkk", &pic16c62x_device::xorlw, 1},
	{(char *)"00000001100100", &pic16c62x_device::clrwdt, 1},
	{(char *)"00000001100010", &pic16c62x_device::option, 1},      // deprecated
	{(char *)"00000001100fff", &pic16c62x_device::tris, 1},        // deprecated
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
	space(AS_PROGRAM).specific(m_program);
	space(AS_DATA).specific(m_data);
	space(AS_IO).specific(m_io);

	/* ensure the internal ram pointers are set before get_info is called */
	update_internalram_ptr();

	build_opcode_table();

	save_item(NAME(m_W));
	save_item(NAME(m_ALU));
	save_item(NAME(m_OPTION));
	save_item(NAME(m_PCLATH));
	save_item(NAME(m_TRISA));
	save_item(NAME(m_TRISB));
	save_item(NAME(m_old_T0));
	save_item(NAME(m_old_data));
	save_item(NAME(m_picRAMmask));
	save_item(NAME(m_WDT));
	save_item(NAME(m_prescaler));
	save_item(NAME(m_STACK));
	save_item(NAME(m_PC));
	save_item(NAME(m_PREVPC));
	save_item(NAME(m_CONFIG));
	save_item(NAME(m_opcode.d));
	save_item(NAME(m_delay_timer));
	save_item(NAME(m_picmodel));
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
	state_add( PIC16C62x_TRSA, "TRSA", m_TRISA).mask(0x1f).formatstr("%02X");
	state_add( PIC16C62x_TRSB, "TRSB", m_TRISB).formatstr("%02X");
	state_add( PIC16C62x_FSR,  "FSR",  m_debugger_temp).mask(0xff).callimport().callexport().formatstr("%02X");
	state_add( PIC16C62x_PSCL, "PSCL", m_debugger_temp).callimport().formatstr("%3s");

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
			STATUS = m_debugger_temp;
			break;
		case PIC16C62x_TMR0:
			TMR0 = m_debugger_temp;
			break;
		case PIC16C62x_PRTA:
			PORTA = m_debugger_temp;
			break;
		case PIC16C62x_PRTB:
			PORTB = m_debugger_temp;
			break;
		case PIC16C62x_FSR:
			FSR = ((m_debugger_temp & m_picRAMmask) | (uint8_t)(~m_picRAMmask));
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
			m_debugger_temp = STATUS;
			break;
		case PIC16C62x_TMR0:
			m_debugger_temp = TMR0;
			break;
		case PIC16C62x_PRTA:
			m_debugger_temp = PORTA & 0x1f;
			break;
		case PIC16C62x_PRTB:
			m_debugger_temp = PORTB;
			break;
		case PIC16C62x_FSR:
			m_debugger_temp = ((FSR) & m_picRAMmask) | (uint8_t)(~m_picRAMmask);
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
				(STATUS & 0xe0) >> 5,
				STATUS & 0x10 ? '.':'O',      /* WDT Overflow */
				STATUS & 0x08 ? 'P':'D',      /* Power/Down */
				STATUS & 0x04 ? 'Z':'.',      /* Zero */
				STATUS & 0x02 ? 'c':'b',      /* Nibble Carry/Borrow */
				STATUS & 0x01 ? 'C':'B',      /* Carry/Borrow */

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
	m_TRISA  = 0x1f;
	m_TRISB  = 0xff;
	m_OPTION = 0xff;
	STATUS = 0x18;
	PCL    = 0;
	FSR   |= (uint8_t)(~m_picRAMmask);
	PORTA  = 0;
	m_prescaler = 0;
	m_delay_timer = 0;
	m_old_T0 = 0;
	m_inst_cycles = 0;
	PIC16C62x_RAM_WRMEM(0x85,m_TRISA);
	PIC16C62x_RAM_WRMEM(0x86,m_TRISB);
	PIC16C62x_RAM_WRMEM(0x81,m_OPTION);
}

void pic16c62x_device::pic16c62x_soft_reset()
{
	SET(STATUS, (TO_FLAG | PD_FLAG | Z_FLAG | DC_FLAG | C_FLAG));
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

	if ((m_opcode.w.l != 0x64) && (m_opcode.w.l != 0x63))
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
					CLR(STATUS, TO_FLAG);
					pic16c62x_soft_reset();
				}
			}
			else {
				CLR(STATUS, TO_FLAG);
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
			TMR0 += (m_prescaler / (2 << PS));
			m_prescaler %= (2 << PS);   /* Overflow prescaler */
		}
	}
	else {
		TMR0 += counts;
	}
}


/****************************************************************************
 *  Execute IPeriod. Return 0 if emulation should be stopped
 ****************************************************************************/

void pic16c62x_device::execute_run()
{
	uint8_t T0_in;

	update_internalram_ptr();

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

			m_opcode.d = M_RDOP(m_PC);
			m_PC++;
			PCL++;

			m_inst_cycles = m_opcode_table[m_opcode.w.l & 0x3fff].cycles;
			(this->*m_opcode_table[m_opcode.w.l & 0x3fff].function)();

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
	update_internalram_ptr();

	pic16c62x_reset_regs();
	SET(STATUS, (TO_FLAG | PD_FLAG));
}
