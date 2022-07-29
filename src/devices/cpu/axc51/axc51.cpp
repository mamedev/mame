// license:BSD-3-Clause
// copyright-holders:Steve Ellenoff, Manuel Abadia, Couriersud

/*
	This is currently mostly copied from the mcs51 core
	but as the axc51 has many differences, it will eventually
	diverge more greatly.  For now assume a lot of what is here
	is still incorrect.

*/

/*****************************************************************************

    AXC51-CORE / AX208 SoC (AppoTech Inc.)

    AXC51CORE:
    somes sources indicate that the extended opcode encoding may change
    on some CPU models despite all being 'AXC51CORE' however we lack solid
    information on this at present.

    AX208:
    The CPU has 0x2000 bytes of internal ROM mapped at 0x8000-0x9fff providing
    bootcode, operating kernel and many standard library functions

 *****************************************************************************/

#include "emu.h"
#include "axc51.h"
#include "axc51dasm.h"

#define VERBOSE 0

#define LOG(x)  do { if (VERBOSE) logerror x; } while (0)

/***************************************************************************
    CONSTANTS
***************************************************************************/


DEFINE_DEVICE_TYPE(AX208, ax208_cpu_device, "ax208", "AppoTech AX208 (AXC51-CORE)")
DEFINE_DEVICE_TYPE(AX208P, ax208p_cpu_device, "ax208p", "AppoTech AX208 (AXC51-CORE) (prototype?)")

/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

void axc51base_cpu_device::program_internal(address_map &map)
{
}

void axc51base_cpu_device::data_internal(address_map &map)
{
	map(0x0000, 0x00ff).ram().share("scratchpad");
	map(0x0100, 0x01ff).ram().share("sfr_ram"); /* SFR */
}

void ax208_cpu_device::ax208_internal_program_mem(address_map &map)
{
	map(0x8000, 0x9fff).rom().region("rom", 0); // this can only be read from code running within the same region
}


axc51base_cpu_device::axc51base_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor program_map, address_map_constructor data_map, int program_width, int data_width, uint8_t features)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 8, 16, 0, program_map)
	, m_data_config("data", ENDIANNESS_LITTLE, 8, 9, 0, data_map)
	, m_io_config("io", ENDIANNESS_LITTLE, 8, 16, 0)
	, m_pc(0)
	, m_features(features)
	, m_rom_size(program_width > 0 ? 1 << program_width : 0)
	, m_num_interrupts(5)
	, m_sfr_ram(*this, "sfr_ram")
	, m_scratchpad(*this, "scratchpad")
	, m_port_in_cb(*this)
	, m_port_out_cb(*this)
	, m_serial_tx_cb(*this)
	, m_serial_rx_cb(*this)
	, m_rtemp(0)
{
	/* default to standard cmos interfacing */
	for (auto & elem : m_forced_inputs)
		elem = 0;
}


axc51base_cpu_device::axc51base_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int program_width, int data_width, uint8_t features)
	: axc51base_cpu_device(mconfig, type, tag, owner, clock, address_map_constructor(FUNC(axc51base_cpu_device::program_internal), this), address_map_constructor(FUNC(axc51base_cpu_device::data_internal), this), program_width, data_width, features)
{
}


device_memory_interface::space_config_vector axc51base_cpu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA,    &m_data_config),
		std::make_pair(AS_IO,      &m_io_config)
	};
}


/***************************************************************************
    MACROS
***************************************************************************/

/* Read Opcode/Opcode Arguments from Program Code */
#define ROP(pc)         m_program.read_byte(pc)
#define ROP_ARG(pc)     m_program.read_byte(pc)

/* Read a byte from External Code Memory (Usually Program Rom(s) Space) */
#define CODEMEM_R(a)    (uint8_t)m_program.read_byte(a)

/* Read/Write a byte from/to External Data Memory (Usually RAM or other I/O) */
#define DATAMEM_R(a)    (uint8_t)m_io.read_byte(a)
#define DATAMEM_W(a,v)  m_io.write_byte(a, v)

/* Read/Write a byte from/to the Internal RAM */

#define IRAM_R(a)       iram_read(a)
#define IRAM_W(a, d)    iram_write(a, d)

/* Read/Write a byte from/to the Internal RAM indirectly */
/* (called from indirect addressing)                     */
uint8_t axc51base_cpu_device::iram_iread(offs_t a) { return m_data.read_byte(a); }
void axc51base_cpu_device::iram_iwrite(offs_t a, uint8_t d) { m_data.write_byte(a, d); }

#define IRAM_IR(a)      iram_iread(a)
#define IRAM_IW(a, d)   iram_iwrite(a, d)

/* Form an Address to Read/Write to External RAM indirectly */
/* (called from indirect addressing)                        */
#define ERAM_ADDR(a,m)  external_ram_iaddr(a,m)

/* Read/Write a bit from Bit Addressable Memory */
#define BIT_R(a)        bit_address_r(a)
#define BIT_W(a,v)      bit_address_w(a, v)


/***************************************************************************
    SHORTCUTS
***************************************************************************/

#define PPC     m_ppc
#define PC      m_pc

/* SFR Registers - These are accessed directly for speed on read */
/* Read accessors                                                */

#define SFR_A(a)        m_sfr_ram[(a)]
#define SET_SFR_A(a,v)  do { SFR_A(a) = (v); } while (0)

#define ACC         SFR_A(ADDR_ACC)
#define PSW         SFR_A(ADDR_PSW)

#define P0          ((const uint8_t) SFR_A(ADDR_P0))
#define P1          ((const uint8_t) SFR_A(ADDR_P1))
#define P2          ((const uint8_t) SFR_A(ADDR_P2))
#define P3          ((const uint8_t) SFR_A(ADDR_P3))

#define SP          SFR_A(ADDR_SP)
#define DPL         SFR_A(ADDR_DPL)
#define DPH         SFR_A(ADDR_DPH)
#define PCON        SFR_A(ADDR_PCON)
#define IE          SFR_A(ADDR_IE)
#define IP          SFR_A(ADDR_IP)
#define B           SFR_A(ADDR_B)

#define R_REG(r)    m_scratchpad[(r) | (PSW & 0x18)]
#define DPTR        ((DPH<<8) | DPL)

/* WRITE accessors */

/* Shortcuts */

#define SET_PSW(v)  do { SFR_A(ADDR_PSW) = (v); SET_PARITY(); } while (0)
#define SET_ACC(v)  do { SFR_A(ADDR_ACC) = (v); SET_PARITY(); } while (0)

/* These trigger actions on modification and have to be written through SFR_W */
#define SET_P0(v)   IRAM_W(ADDR_P0, v)
#define SET_P1(v)   IRAM_W(ADDR_P1, v)
#define SET_P2(v)   IRAM_W(ADDR_P2, v)
#define SET_P3(v)   IRAM_W(ADDR_P3, v)



/* No actions triggered on write */
#define SET_REG(r, v)   do { m_scratchpad[(r) | (PSW & 0x18)] = (v); } while (0)

#define SET_DPTR(n)     do { DPH = ((n) >> 8) & 0xff; DPL = (n) & 0xff; } while (0)

/* Macros for Setting Flags */
#define SET_X(R, v) do { R = (v);} while (0)

#define SET_CY(n)       SET_PSW((PSW & 0x7f) | (n<<7))  //Carry Flag
#define SET_AC(n)       SET_PSW((PSW & 0xbf) | (n<<6))  //Aux.Carry Flag
#define SET_FO(n)       SET_PSW((PSW & 0xdf) | (n<<5))  //User Flag
#define SET_RS(n)       SET_PSW((PSW & 0xe7) | (n<<3))  //R Bank Select
#define SET_OV(n)       SET_PSW((PSW & 0xfb) | (n<<2))  //Overflow Flag
#define SET_P(n)        SET_PSW((PSW & 0xfe) | (n<<0))  //Parity Flag

#define SET_BIT(R, n, v) do { R = (R & ~(1<<(n))) | ((v) << (n));} while (0)
#define GET_BIT(R, n) (((R)>>(n)) & 0x01)

#define SET_EA(n)       SET_BIT(IE, 7, n)       //Global Interrupt Enable/Disable
#define SET_ES(n)       SET_BIT(IE, 4, v)       //Serial Interrupt Enable/Disable
#define SET_ET1(n)      SET_BIT(IE, 3, n)       //Timer 1 Interrupt Enable/Disable
#define SET_EX1(n)      SET_BIT(IE, 2, n)       //External Int 1 Interrupt Enable/Disable
#define SET_ET0(n)      SET_BIT(IE, 1, n)       //Timer 0 Interrupt Enable/Disable
#define SET_EX0(n)      SET_BIT(IE, 0, n)       //External Int 0 Interrupt Enable/Disable

/* Macros for accessing flags */

#define GET_CY          GET_BIT(PSW, 7)
#define GET_AC          GET_BIT(PSW, 6)
#define GET_FO          GET_BIT(PSW, 5)
#define GET_RS          GET_BIT(PSW, 3)
#define GET_OV          GET_BIT(PSW, 2)
#define GET_P           GET_BIT(PSW, 0)

#define GET_EA          GET_BIT(IE, 7)
#define GET_ET2         GET_BIT(IE, 5)
#define GET_ES          GET_BIT(IE, 4)
#define GET_ET1         GET_BIT(IE, 3)
#define GET_EX1         GET_BIT(IE, 2)
#define GET_ET0         GET_BIT(IE, 1)
#define GET_EX0         GET_BIT(IE, 0)



/*Add and Subtract Flag settings*/
#define DO_ADD_FLAGS(a,d,c) do_add_flags(a, d, c)
#define DO_SUB_FLAGS(a,d,c) do_sub_flags(a, d, c)

#define SET_PARITY()    do {m_recalc_parity |= 1;} while (0)
#define PUSH_PC()       push_pc()
#define POP_PC()        pop_pc()

/* Clear Current IRQ  */
#define CLEAR_CURRENT_IRQ() clear_current_irq()


/* Hold callback functions so they can be set by caller (before the cpu reset) */

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

void axc51base_cpu_device::clear_current_irq()
{
	if (m_cur_irq_prio >= 0)
		m_irq_active &= ~(1 << m_cur_irq_prio);
	if (m_irq_active & 4)
		m_cur_irq_prio = 2;
	else if (m_irq_active & 2)
		m_cur_irq_prio = 1;
	else if (m_irq_active & 1)
		m_cur_irq_prio = 0;
	else
		m_cur_irq_prio = -1;
	LOG(("New: %d %02x\n", m_cur_irq_prio, m_irq_active));
}

uint8_t axc51base_cpu_device::r_acc() { return SFR_A(ADDR_ACC); }

uint8_t axc51base_cpu_device::r_psw() { return SFR_A(ADDR_PSW); }


offs_t axc51base_cpu_device::external_ram_iaddr(offs_t offset, offs_t mem_mask)
{
	if (mem_mask == 0x00ff)
		return (offset & mem_mask) | (P2 << 8);

	return offset;
}

/* Internal ram read/write */

uint8_t axc51base_cpu_device::iram_read(size_t offset)
{
	return (((offset) < 0x80) ? m_data.read_byte(offset) : sfr_read(offset));
}

void axc51base_cpu_device::iram_write(size_t offset, uint8_t data)
{
	if ((offset) < 0x80)
		m_data.write_byte(offset, data);
	else
		sfr_write(offset, data);
}

/*Push the current PC to the stack*/
void axc51base_cpu_device::push_pc()
{
	uint8_t tmpSP = SP+1;                     //Grab and Increment Stack Pointer
	IRAM_IW(tmpSP, (PC & 0xff));                //Store low byte of PC to Internal Ram (Use IRAM_IW to store stack above 128 bytes)
	tmpSP++;                                    // ""
	SP = tmpSP;                             // ""
	IRAM_IW(tmpSP, ( (PC & 0xff00) >> 8));      //Store hi byte of PC to next address in Internal Ram (Use IRAM_IW to store stack above 128 bytes)
}

/*Pop the current PC off the stack and into the pc*/
void axc51base_cpu_device::pop_pc()
{
	uint8_t tmpSP = SP;                           //Grab Stack Pointer
	PC = (IRAM_IR(tmpSP--) & 0xff) << 8;        //Store hi byte to PC (must use IRAM_IR to access stack pointing above 128 bytes)
	PC = PC | IRAM_IR(tmpSP--);                 //Store lo byte to PC (must use IRAM_IR to access stack pointing above 128 bytes)
	SP = tmpSP;                             //Decrement Stack Pointer
}

//Set the PSW Parity Flag
void axc51base_cpu_device::set_parity()
{
	//This flag will be set when the accumulator contains an odd # of bits set..
	uint8_t p = 0;
	int i;
	uint8_t a = ACC;

	for (i=0; i<8; i++) {       //Test for each of the 8 bits in the ACC!
		p ^= (a & 1);
		a = (a >> 1);
	}

	SET_P(p & 1);
}

uint8_t axc51base_cpu_device::bit_address_r(uint8_t offset)
{
	uint8_t   word;
	uint8_t   mask;
	int bit_pos;
	int distance;   /* distance between bit addressable words */
					/* 1 for normal bits, 8 for sfr bit addresses */

	m_last_bit = offset;

	//User defined bit addresses 0x20-0x2f (values are 0x0-0x7f)
	if (offset < 0x80) {
		distance = 1;
		word = ( (offset & 0x78) >> 3) * distance + 0x20;
		bit_pos = offset & 0x7;
		mask = (0x1 << bit_pos);
		return((IRAM_R(word) & mask) >> bit_pos);
	}
	//SFR bit addressable registers
	else {
		distance = 8;
		word = ( (offset & 0x78) >> 3) * distance + 0x80;
		bit_pos = offset & 0x7;
		mask = (0x1 << bit_pos);
		return ((IRAM_R(word) & mask) >> bit_pos);
	}
}


void axc51base_cpu_device::bit_address_w(uint8_t offset, uint8_t bit)
{
	int word;
	uint8_t   mask;
	int bit_pos;
	uint8_t   result;
	int distance;

	/* User defined bit addresses 0x20-0x2f (values are 0x0-0x7f) */
	if (offset < 0x80) {
		distance = 1;
		word = ((offset & 0x78) >> 3) * distance + 0x20;
		bit_pos = offset & 0x7;
		bit = (bit & 0x1) << bit_pos;
		mask = ~(1 << bit_pos) & 0xff;
		result = IRAM_R(word) & mask;
		result = result | bit;
		IRAM_W(word, result);
	}
	/* SFR bit addressable registers */
	else {
		distance = 8;
		word = ((offset & 0x78) >> 3) * distance + 0x80;
		bit_pos = offset & 0x7;
		bit = (bit & 0x1) << bit_pos;
		mask = ~(1 << bit_pos) & 0xff;
		result = IRAM_R(word) & mask;
		result = result | bit;
		IRAM_W(word, result);
	}
}

void axc51base_cpu_device::do_add_flags(uint8_t a, uint8_t data, uint8_t c)
{
	uint16_t result = a+data+c;
	int16_t result1 = (int8_t)a+(int8_t)data+c;

	SET_CY((result & 0x100) >> 8);
	result = (a&0x0f)+(data&0x0f)+c;
	SET_AC((result & 0x10) >> 4);
	SET_OV(result1 < -128 || result1 > 127);
}

void axc51base_cpu_device::do_sub_flags(uint8_t a, uint8_t data, uint8_t c)
{
	uint16_t result = a-(data+c);
	int16_t result1 = (int8_t)a-(int8_t)(data+c);

	SET_CY((result & 0x100) >> 8);
	result = (a&0x0f)-((data&0x0f)+c);
	SET_AC((result & 0x10) >> 4);
	SET_OV((result1 < -128 || result1 > 127));
}

void axc51base_cpu_device::transmit_receive(int source)
{
}

void axc51base_cpu_device::update_timer_t0(int cycles)
{
}



void axc51base_cpu_device::update_timer_t1(int cycles)
{
}

void axc51base_cpu_device::update_timer_t2(int cycles)
{
}

void axc51base_cpu_device::update_timers(int cycles)
{
	while (cycles--)
	{
		update_timer_t0(1);
		update_timer_t1(1);
	}
}

void axc51base_cpu_device::serial_transmit(uint8_t data)
{
}

void axc51base_cpu_device::serial_receive()
{
}

/* Check and update status of serial port */
void axc51base_cpu_device::update_serial(int cycles)
{
	while (--cycles>=0)
		transmit_receive(0);
}

/* Check and update status of serial port */
void axc51base_cpu_device::update_irq_prio(uint8_t ipl, uint8_t iph)
{
	for (int i=0; i<8; i++)
		m_irq_prio[i] = ((ipl >> i) & 1) | (((iph >>i ) & 1) << 1);
}


/***************************************************************************
    OPCODES
***************************************************************************/

#define OPHANDLER( _name ) void axc51base_cpu_device::_name (uint8_t r)

#include "axc51ops.hxx"


void axc51base_cpu_device::execute_op(uint8_t op)
{
	if (m_recalc_parity)
	{
		set_parity();
		m_recalc_parity = 0;
	}

	m_last_op = op;

	switch( op )
	{
		case 0x00:  nop(op);                           break;  //NOP
		case 0x01:  ajmp(op);                      break;  //AJMP code addr
		case 0x02:  ljmp(op);                      break;  //LJMP code addr
		case 0x03:  rr_a(op);                      break;  //RR A
		case 0x04:  inc_a(op);                     break;  //INC A
		case 0x05:  inc_mem(op);                   break;  //INC data addr

		case 0x06:
		case 0x07:  inc_ir(op&1);                       break;  //INC @R0/@R1

		case 0x08:
		case 0x09:
		case 0x0a:
		case 0x0b:
		case 0x0c:
		case 0x0d:
		case 0x0e:
		case 0x0f:  inc_r(op&7);                       break;  //INC R0 to R7

		case 0x10:  jbc(op);                       break;  //JBC bit addr, code addr
		case 0x11:  acall(op);                     break;  //ACALL code addr
		case 0x12:  lcall(op);                         break;  //LCALL code addr
		case 0x13:  rrc_a(op);                     break;  //RRC A
		case 0x14:  dec_a(op);                     break;  //DEC A
		case 0x15:  dec_mem(op);                   break;  //DEC data addr

		case 0x16:
		case 0x17:  dec_ir(op&1);                  break;  //DEC @R0/@R1

		case 0x18:
		case 0x19:
		case 0x1a:
		case 0x1b:
		case 0x1c:
		case 0x1d:
		case 0x1e:
		case 0x1f:  dec_r(op&7);                   break;  //DEC R0 to R7

		case 0x20:  jb(op);                            break;  //JB  bit addr, code addr
		case 0x21:  ajmp(op);                      break;  //AJMP code addr
		case 0x22:  ret(op);                           break;  //RET
		case 0x23:  rl_a(op);                      break;  //RL A
		case 0x24:  add_a_byte(op);                    break;  //ADD A, #data
		case 0x25:  add_a_mem(op);                 break;  //ADD A, data addr

		case 0x26:
		case 0x27:  add_a_ir(op&1);                    break;  //ADD A, @R0/@R1

		case 0x28:
		case 0x29:
		case 0x2a:
		case 0x2b:
		case 0x2c:
		case 0x2d:
		case 0x2e:
		case 0x2f:  add_a_r(op&7);                 break;  //ADD A, R0 to R7

		case 0x30:  jnb(op);                           break;  //JNB bit addr, code addr
		case 0x31:  acall(op);                     break;  //ACALL code addr
		case 0x32:  reti(op);                      break;  //RETI
		case 0x33:  rlc_a(op);                     break;  //RLC A
		case 0x34:  addc_a_byte(op);                   break;  //ADDC A, #data
		case 0x35:  addc_a_mem(op);                    break;  //ADDC A, data addr

		case 0x36:
		case 0x37:  addc_a_ir(op&1);                   break;  //ADDC A, @R0/@R1

		case 0x38:
		case 0x39:
		case 0x3a:
		case 0x3b:
		case 0x3c:
		case 0x3d:
		case 0x3e:
		case 0x3f:  addc_a_r(op&7);                    break;  //ADDC A, R0 to R7

		case 0x40:  jc(op);                            break;  //JC code addr
		case 0x41:  ajmp(op);                      break;  //AJMP code addr
		case 0x42:  orl_mem_a(op);                 break;  //ORL data addr, A
		case 0x43:  orl_mem_byte(op);              break;  //ORL data addr, #data
		case 0x44:  orl_a_byte(op);                    break;
		case 0x45:  orl_a_mem(op);                 break;  //ORL A, data addr

		case 0x46:
		case 0x47:  orl_a_ir(op&1);                    break;  //ORL A, @RO/@R1

		case 0x48:
		case 0x49:
		case 0x4a:
		case 0x4b:
		case 0x4c:
		case 0x4d:
		case 0x4e:
		case 0x4f:  orl_a_r(op&7);                     break;  //ORL A, RO to R7

		case 0x50:  jnc(op);                       break;  //JNC code addr
		case 0x51:  acall(op);                     break;  //ACALL code addr
		case 0x52:  anl_mem_a(op);                 break;  //ANL data addr, A
		case 0x53:  anl_mem_byte(op);              break;  //ANL data addr, #data
		case 0x54:  anl_a_byte(op);                    break;  //ANL A, #data
		case 0x55:  anl_a_mem(op);                 break;  //ANL A, data addr

		case 0x56:
		case 0x57:  anl_a_ir(op&1);                    break;  //ANL A, @RO/@R1

		case 0x58:
		case 0x59:
		case 0x5a:
		case 0x5b:
		case 0x5c:
		case 0x5d:
		case 0x5e:
		case 0x5f:  anl_a_r(op&7);                 break;  //ANL A, RO to R7

		case 0x60:  jz(op);                            break;  //JZ code addr
		case 0x61:  ajmp(op);                      break;  //AJMP code addr
		case 0x62:  xrl_mem_a(op);                 break;  //XRL data addr, A
		case 0x63:  xrl_mem_byte(op);              break;  //XRL data addr, #data
		case 0x64:  xrl_a_byte(op);                    break;  //XRL A, #data
		case 0x65:  xrl_a_mem(op);                 break;  //XRL A, data addr

		case 0x66:
		case 0x67:  xrl_a_ir(op&1);                    break;  //XRL A, @R0/@R1

		case 0x68:
		case 0x69:
		case 0x6a:
		case 0x6b:
		case 0x6c:
		case 0x6d:
		case 0x6e:
		case 0x6f:  xrl_a_r(op&7);                 break;  //XRL A, R0 to R7

		case 0x70:  jnz(op);                           break;  //JNZ code addr
		case 0x71:  acall(op);                     break;  //ACALL code addr
		case 0x72:  orl_c_bitaddr(op);             break;  //ORL C, bit addr
		case 0x73:  jmp_iadptr(op);                    break;  //JMP @A+DPTR
		case 0x74:  mov_a_byte(op);                    break;  //MOV A, #data
		case 0x75:  mov_mem_byte(op);              break;  //MOV data addr, #data

		case 0x76:
		case 0x77:  mov_ir_byte(op&1);             break;  //MOV @R0/@R1, #data

		case 0x78:
		case 0x79:
		case 0x7a:
		case 0x7b:
		case 0x7c:
		case 0x7d:
		case 0x7e:
		case 0x7f:  mov_r_byte(op&7);              break;  //MOV R0 to R7, #data

		case 0x80:  sjmp(op);                      break;  //SJMP code addr
		case 0x81:  ajmp(op);                      break;  //AJMP code addr
		case 0x82:  anl_c_bitaddr(op);             break;  //ANL C, bit addr
		case 0x83:  movc_a_iapc(op);                   break;  //MOVC A, @A + PC
		case 0x84:  div_ab(op);                        break;  //DIV AB
		case 0x85:  mov_mem_mem(op);                   break;  //MOV data addr, data addr

		case 0x86:
		case 0x87:  mov_mem_ir(op&1);              break;  //MOV data addr, @R0/@R1

		case 0x88:
		case 0x89:
		case 0x8a:
		case 0x8b:
		case 0x8c:
		case 0x8d:
		case 0x8e:
		case 0x8f:  mov_mem_r(op&7);                   break;  //MOV data addr,R0 to R7

		case 0x90:  mov_dptr_byte(op);             break;  //MOV DPTR, #data
		case 0x91:  acall(op);                     break;  //ACALL code addr
		case 0x92:  mov_bitaddr_c(op);             break;    //MOV bit addr, C
		case 0x93:  movc_a_iadptr(op);             break;  //MOVC A, @A + DPTR
		case 0x94:  subb_a_byte(op);                   break;  //SUBB A, #data
		case 0x95:  subb_a_mem(op);                    break;  //SUBB A, data addr

		case 0x96:
		case 0x97:  subb_a_ir(op&1);                   break;  //SUBB A, @R0/@R1

		case 0x98:
		case 0x99:
		case 0x9a:
		case 0x9b:
		case 0x9c:
		case 0x9d:
		case 0x9e:
		case 0x9f:  subb_a_r(op&7);                    break;  //SUBB A, R0 to R7

		case 0xa0:  orl_c_nbitaddr(op);                break;  //ORL C, /bit addr
		case 0xa1:  ajmp(op);                      break;  //AJMP code addr
		case 0xa2:  mov_c_bitaddr(op);             break;  //MOV C, bit addr
		case 0xa3:  inc_dptr(op);                  break;  //INC DPTR
		case 0xa4:  mul_ab(op);                        break;  //MUL AB
		case 0xa5:  illegal(op);                       break;  //reserved

		case 0xa6:
		case 0xa7:  mov_ir_mem(op&1);              break;  //MOV @R0/@R1, data addr

		case 0xa8:
		case 0xa9:
		case 0xaa:
		case 0xab:
		case 0xac:
		case 0xad:
		case 0xae:
		case 0xaf:  mov_r_mem(op&7);                   break;  //MOV R0 to R7, data addr

		case 0xb0:  anl_c_nbitaddr(op);                break;  //ANL C,/bit addr
		case 0xb1:  acall(op);                     break;  //ACALL code addr
		case 0xb2:  cpl_bitaddr(op);               break;  //CPL bit addr
		case 0xb3:  cpl_c(op);                     break;  //CPL C
		case 0xb4:  cjne_a_byte(op);                   break;  //CJNE A, #data, code addr
		case 0xb5:  cjne_a_mem(op);                    break;  //CJNE A, data addr, code addr

		case 0xb6:
		case 0xb7:  cjne_ir_byte(op&1);                break;  //CJNE @R0/@R1, #data, code addr

		case 0xb8:
		case 0xb9:
		case 0xba:
		case 0xbb:
		case 0xbc:
		case 0xbd:
		case 0xbe:
		case 0xbf:  cjne_r_byte(op&7);                 break;  //CJNE R0 to R7, #data, code addr

		case 0xc0:  push(op);                      break;  //PUSH data addr
		case 0xc1:  ajmp(op);                      break;  //AJMP code addr
		case 0xc2:  clr_bitaddr(op);               break;  //CLR bit addr
		case 0xc3:  clr_c(op);                         break;  //CLR C
		case 0xc4:  swap_a(op);                        break;  //SWAP A
		case 0xc5:  xch_a_mem(op);                 break;  //XCH A, data addr

		case 0xc6:
		case 0xc7:  xch_a_ir(op&1);                    break;  //XCH A, @RO/@R1

		case 0xc8:
		case 0xc9:
		case 0xca:
		case 0xcb:
		case 0xcc:
		case 0xcd:
		case 0xce:
		case 0xcf:  xch_a_r(op&7);                 break;  //XCH A, RO to R7

		case 0xd0:  pop(op);                           break;  //POP data addr
		case 0xd1:  acall(op);                     break;  //ACALL code addr
		case 0xd2:  setb_bitaddr(op);              break;  //SETB bit addr
		case 0xd3:  setb_c(op);                        break;  //SETB C
		case 0xd4:  da_a(op);                      break;  //DA A
		case 0xd5:  djnz_mem(op);                  break;  //DJNZ data addr, code addr

		case 0xd6:
		case 0xd7:  xchd_a_ir(op&1);                   break;  //XCHD A, @R0/@R1

		case 0xd8:
		case 0xd9:
		case 0xda:
		case 0xdb:
		case 0xdc:
		case 0xdd:
		case 0xde:
		case 0xdf:  djnz_r(op&7);                  break;  //DJNZ R0 to R7,code addr

		case 0xe0:  movx_a_idptr(op);              break;  //MOVX A,@DPTR
		case 0xe1:  ajmp(op);                      break;  //AJMP code addr

		case 0xe2:
		case 0xe3:  movx_a_ir(op&1);                   break;  //MOVX A, @R0/@R1

		case 0xe4:  clr_a(op);                     break;  //CLR A
		case 0xe5:  mov_a_mem(op);                 break;  //MOV A, data addr
		case 0xe6:
		case 0xe7:  mov_a_ir(op&1);                    break;  //MOV A,@RO/@R1

		case 0xe8:
		case 0xe9:
		case 0xea:
		case 0xeb:
		case 0xec:
		case 0xed:
		case 0xee:
		case 0xef:  mov_a_r(op&7);                 break;  //MOV A,R0 to R7

		case 0xf0:  movx_idptr_a(op);              break;  //MOVX @DPTR,A
		case 0xf1:  acall(op);                     break;  //ACALL code addr

		case 0xf2:
		case 0xf3:  movx_ir_a(op&1);                   break;  //MOVX @R0/@R1,A

		case 0xf4:  cpl_a(op);                     break;  //CPL A
		case 0xf5:  mov_mem_a(op);                 break;  //MOV data addr, A

		case 0xf6:
		case 0xf7:  mov_ir_a(op&1);                    break;  //MOV @R0/@R1, A

		case 0xf8:
		case 0xf9:
		case 0xfa:
		case 0xfb:
		case 0xfc:
		case 0xfd:
		case 0xfe:
		case 0xff:  mov_r_a(op&7);                 break;  //MOV R0 to R7, A
		default:
			illegal(op);
	}
}

/***************************************************************************
    OPCODE CYCLES
***************************************************************************/

/* # of oscilations each opcode requires*/
const uint8_t axc51base_cpu_device::axc51_cycles[256] = {
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};


void axc51base_cpu_device::check_irqs()
{
}

void axc51base_cpu_device::burn_cycles(int cycles)
{
	/* Update Timer (if any timers are running) */
	update_timers(cycles);

	/* Update Serial (only for mode 0) */
	update_serial(cycles);

	/* check_irqs */
	check_irqs();
}

void axc51base_cpu_device::execute_set_input(int irqline, int state)
{
	uint32_t new_state = (m_last_line_state & ~(1 << irqline)) | ((state != CLEAR_LINE) << irqline);
	/* detect 0->1 transitions */
	//uint32_t tr_state = (~m_last_line_state) & new_state;

	// TODO

	m_last_line_state = new_state;
}

/* Execute cycles - returns number of cycles actually run */
void axc51base_cpu_device::execute_run()
{
	uint8_t op;

	/* external interrupts may have been set since we last checked */
	m_inst_cycles = 0;
	check_irqs();

	m_icount -= m_inst_cycles;
	burn_cycles(m_inst_cycles);

	do
	{
		/* Read next opcode */
		PPC = PC;
		debugger_instruction_hook(PC);
		op = m_program.read_byte(PC++);

		/* process opcode and count cycles */
		m_inst_cycles = axc51_cycles[op];
		execute_op(op);

		/* burn the cycles */
		m_icount -= m_inst_cycles;

		burn_cycles(m_inst_cycles);

	} while( m_icount > 0 );
}




void axc51base_cpu_device::sfr_write(size_t offset, uint8_t data)
{
	/* update register */
	assert(offset >= 0x80 && offset <= 0xff);

	switch (offset)
	{
		case ADDR_P0:   m_port_out_cb[0](data);             break;
		case ADDR_P1:   m_port_out_cb[1](data);             break;
		case ADDR_P2:   m_port_out_cb[2](data);             break;
		case ADDR_P3:   m_port_out_cb[3](data);             break;
		case ADDR_PSW:  SET_PARITY();                       break;
		case ADDR_ACC:  SET_PARITY();                       break;
		case ADDR_IP:   update_irq_prio(data, 0);  break;

		case ADDR_B:
		case ADDR_SP:
		case ADDR_DPL:
		case ADDR_DPH:
		case ADDR_PCON:
		case ADDR_IE:
			break;
		default:
			LOG(("axc51 '%s': attemping to write to an invalid/non-implemented SFR address: %x at 0x%04x, data=%x\n", tag(), (uint32_t)offset,PC,data));
			/* no write in this case according to manual */
			return;
	}
	m_data.write_byte((size_t)offset | 0x100, data);
}

uint8_t axc51base_cpu_device::sfr_read(size_t offset)
{
	assert(offset >= 0x80 && offset <= 0xff);

	switch (offset)
	{
		/* Move to memory map */
		case ADDR_P0:   return (P0 | m_forced_inputs[0]) & m_port_in_cb[0]();
		case ADDR_P1:   return (P1 | m_forced_inputs[1]) & m_port_in_cb[1]();
		case ADDR_P2:   return (P2 | m_forced_inputs[2]) & m_port_in_cb[2]();
		case ADDR_P3:   return (P3 | m_forced_inputs[3]) & m_port_in_cb[3]()
							& ~(GET_BIT(m_last_line_state, AXC51_INT0_LINE) ? 4 : 0)
							& ~(GET_BIT(m_last_line_state, AXC51_INT1_LINE) ? 8 : 0);

		case ADDR_PSW:
		case ADDR_ACC:
		case ADDR_B:
		case ADDR_SP:
		case ADDR_DPL:
		case ADDR_DPH:
		case ADDR_PCON:
		case ADDR_IE:
		case ADDR_IP:
			return m_data.read_byte((size_t) offset | 0x100);
		/* Illegal or non-implemented sfr */
		default:
			LOG(("axc51 '%s': attemping to read an invalid/non-implemented SFR address: %x at 0x%04x\n", tag(), (uint32_t)offset,PC));
			/* according to the manual, the read may return random bits */
			return 0xff;
	}
}


void axc51base_cpu_device::device_start()
{
	space(AS_PROGRAM).cache(m_program);
	space(AS_DATA).specific(m_data);
	space(AS_IO).specific(m_io);

	m_port_in_cb.resolve_all_safe(0xff);
	m_port_out_cb.resolve_all_safe();
	m_serial_rx_cb.resolve_safe(0);
	m_serial_tx_cb.resolve_safe();

	/* Save states */
	save_item(NAME(m_ppc));
	save_item(NAME(m_pc));
	save_item(NAME(m_last_op));
	save_item(NAME(m_last_bit));
	save_item(NAME(m_cur_irq_prio) );
	save_item(NAME(m_last_line_state) );
	save_item(NAME(m_recalc_parity) );
	save_item(NAME(m_irq_prio) );
	save_item(NAME(m_irq_active) );
	save_item(NAME(m_uart.data_out));
	save_item(NAME(m_uart.bits_to_send));
	save_item(NAME(m_uart.smod_div));
	save_item(NAME(m_uart.rx_clk));
	save_item(NAME(m_uart.tx_clk));
	save_item(NAME(m_uart.delay_cycles));

	state_add( AXC51_PC,  "PC", m_pc).formatstr("%04X");
	state_add( AXC51_SP,  "SP", SP).formatstr("%02X");
	state_add( AXC51_PSW, "PSW", PSW).formatstr("%02X");
	state_add( AXC51_ACC, "A", ACC).formatstr("%02X");
	state_add( AXC51_B,   "B", B).formatstr("%02X");
	state_add<uint16_t>( AXC51_DPTR, "DPTR", [this](){ return DPTR; }, [this](uint16_t dp){ SET_DPTR(dp); }).formatstr("%04X");
	state_add( AXC51_DPH, "DPH", DPH).noshow();
	state_add( AXC51_DPL, "DPL", DPL).noshow();
	state_add( AXC51_IE,  "IE", IE).formatstr("%02X");
	state_add( AXC51_IP,  "IP", IP).formatstr("%02X");
	if (m_rom_size > 0)
		state_add<uint8_t>( AXC51_P0,  "P0", [this](){ return P0; }, [this](uint8_t p){ SET_P0(p); }).formatstr("%02X");
	state_add<uint8_t>( AXC51_P1,  "P1", [this](){ return P1; }, [this](uint8_t p){ SET_P1(p); }).formatstr("%02X");
	state_add<uint8_t>( AXC51_P2,  "P2", [this](){ return P2; }, [this](uint8_t p){ SET_P2(p); }).formatstr("%02X");
	state_add<uint8_t>( AXC51_P3,  "P3", [this](){ return P3; }, [this](uint8_t p){ SET_P3(p); }).formatstr("%02X");
	state_add<uint8_t>( AXC51_R0,  "R0", [this](){ return R_REG(0); }, [this](uint8_t r){ SET_REG(0, r); }).formatstr("%02X");
	state_add<uint8_t>( AXC51_R1,  "R1", [this](){ return R_REG(1); }, [this](uint8_t r){ SET_REG(1, r); }).formatstr("%02X");
	state_add<uint8_t>( AXC51_R2,  "R2", [this](){ return R_REG(2); }, [this](uint8_t r){ SET_REG(2, r); }).formatstr("%02X");
	state_add<uint8_t>( AXC51_R3,  "R3", [this](){ return R_REG(3); }, [this](uint8_t r){ SET_REG(3, r); }).formatstr("%02X");
	state_add<uint8_t>( AXC51_R4,  "R4", [this](){ return R_REG(4); }, [this](uint8_t r){ SET_REG(4, r); }).formatstr("%02X");
	state_add<uint8_t>( AXC51_R5,  "R5", [this](){ return R_REG(5); }, [this](uint8_t r){ SET_REG(5, r); }).formatstr("%02X");
	state_add<uint8_t>( AXC51_R6,  "R6", [this](){ return R_REG(6); }, [this](uint8_t r){ SET_REG(6, r); }).formatstr("%02X");
	state_add<uint8_t>( AXC51_R7,  "R7", [this](){ return R_REG(7); }, [this](uint8_t r){ SET_REG(7, r); }).formatstr("%02X");
	state_add<uint8_t>( AXC51_RB,  "RB", [this](){ return (PSW & 0x18)>>3; }, [this](uint8_t rb){ SET_RS(rb); }).mask(0x03).formatstr("%02X");


	state_add( STATE_GENPC, "GENPC", m_pc ).noshow();
	state_add( STATE_GENPCBASE, "CURPC", m_pc ).noshow();
	state_add( STATE_GENFLAGS, "GENFLAGS", m_rtemp).formatstr("%8s").noshow();

	set_icountptr(m_icount);
}

void axc51base_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c%c%c%c%c%c%c",
				PSW & 0x80 ? 'C':'.',
				PSW & 0x40 ? 'A':'.',
				PSW & 0x20 ? 'F':'.',
				PSW & 0x10 ? '0':'.',
				PSW & 0x08 ? '1':'.',
				PSW & 0x04 ? 'V':'.',
				PSW & 0x02 ? '?':'.',
				PSW & 0x01 ? 'P':'.');
			break;
	}
}

/* Reset registers to the initial values */
void axc51base_cpu_device::device_reset()
{
	m_last_line_state = 0;

	/* Flag as NO IRQ in Progress */
	m_irq_active = 0;
	m_cur_irq_prio = -1;
	m_last_op = 0;
	m_last_bit = 0;

	/* these are all defined reset states */
	PPC = PC;
	PC = 0;
	SP = 0x7;
	SET_PSW(0);
	SET_ACC(0);
	DPH = 0;
	DPL = 0;
	B = 0;
	IP = 0;
	update_irq_prio(IP, 0);
	IE = 0;
	PCON = 0;


	/* set the port configurations to all 1's */
	SET_P3(0xff);
	SET_P2(0xff);
	SET_P1(0xff);
	SET_P0(0xff);

	m_uart.data_out = 0;
	m_uart.rx_clk = 0;
	m_uart.tx_clk = 0;
	m_uart.bits_to_send = 0;
	m_uart.delay_cycles = 0;
	m_uart.smod_div = 0;

	m_recalc_parity = 0;
}


std::unique_ptr<util::disasm_interface> axc51base_cpu_device::create_disassembler()
{
	return std::make_unique<axc51core_disassembler>();
}



// AX208 (specific CPU)

ax208_cpu_device::ax208_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: axc51base_cpu_device(mconfig, type, tag, owner, clock, address_map_constructor(FUNC(ax208_cpu_device::ax208_internal_program_mem), this), address_map_constructor(FUNC(ax208_cpu_device::data_internal), this), 0, 8)
{
}

ax208_cpu_device::ax208_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ax208_cpu_device(mconfig, AX208, tag, owner, clock)
{
}


std::unique_ptr<util::disasm_interface> ax208_cpu_device::create_disassembler()
{
	return std::make_unique<ax208_disassembler>();
}




offs_t ax208_cpu_device::external_ram_iaddr(offs_t offset, offs_t mem_mask)
{
	if (mem_mask == 0x00ff)
		return (offset & mem_mask);

	return offset;
}

uint8_t ax208_cpu_device::spicon_r()
{
	logerror("%s: sfr_read AXC51_SPICON\n", machine().describe_context());
	return axc51base_cpu_device::sfr_read(AXC51_SPICON);
}

uint8_t ax208_cpu_device::dpcon_r()
{
	logerror("%s: sfr_read AXC51_DPCON\n", machine().describe_context());
	return axc51base_cpu_device::sfr_read(AXC51_DPCON);
}

uint8_t ax208_cpu_device::spibuf_r()
{
	// HACK while we figure things out

	if (state_int(AXC51_PC) == 0x8910)
	{
		logerror("%s: sfr_read AXC51_SPIBUF (reading from %08x)\n", machine().describe_context(), m_spiaddr);

		return m_spiptr[m_spiaddr++];
	}

	logerror("%s: sfr_read AXC51_SPIBUF\n", machine().describe_context());

	return machine().rand();
	//return axc51base_cpu_device::sfr_read(AXC51_SPIBUF);
}

uint8_t ax208_cpu_device::sfr_read(size_t offset)
{
	switch (offset)
	{
	case 0x82:
	case 0x83:
	case 0xe0: // ACC
		return axc51base_cpu_device::sfr_read(offset);

	case AXC51_DPCON: // 0x86
		return dpcon_r();

	case AXC51_IRTCON: // 0x9f
		return machine().rand();

	case AXC51_SPICON: // 0xd8
		return spicon_r();

	case AXC51_SPIBUF: // 0xd9
		return spibuf_r();

	}
	logerror("%s: sfr_read (%02x)\n", machine().describe_context(), offset);
	return axc51base_cpu_device::sfr_read(offset);
}

void ax208_cpu_device::spicon_w(uint8_t data)
{
	logerror("%s: sfr_write AXC51_SPICON %02x\n", machine().describe_context(), data);
	axc51base_cpu_device::sfr_write(AXC51_SPICON, data);
}


void ax208_cpu_device::spibuf_w(uint8_t data)
{
	logerror("%s: sfr_write AXC51_SPIBUF %02x\n", machine().describe_context(), data);
	axc51base_cpu_device::sfr_write(AXC51_SPIBUF, data);
}

void ax208_cpu_device::spibaud_w(uint8_t data)
{
	logerror("%s: sfr_write AXC51_SPIBAUD %02x\n", machine().describe_context(), data);
	axc51base_cpu_device::sfr_write(AXC51_SPIBAUD, data);
}

void ax208_cpu_device::dpcon_w(uint8_t data)
{
	logerror("%s: sfr_write AXC51_DPCON %02x\n", machine().describe_context(), data);
	axc51base_cpu_device::sfr_write(AXC51_DPCON, data);
}


void ax208_cpu_device::spidmaadr_w(uint8_t data)
{
	logerror("%s: sfr_write AXC51_SPIDMAADR %02x\n", machine().describe_context(), data);
	axc51base_cpu_device::sfr_write(AXC51_SPIDMAADR, data);
}

void ax208_cpu_device::spidmacnt_w(uint8_t data)
{
	logerror("%s: sfr_write AXC51_SPIDMACNT %02x\n", machine().describe_context(), data);
	axc51base_cpu_device::sfr_write(AXC51_SPIDMACNT, data);
}

void ax208_cpu_device::sfr_write(size_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0x82:
	case 0x83:
	case 0xe0:
		axc51base_cpu_device::sfr_write(offset, data);
		return;
		

	case AXC51_DPCON: dpcon_w(data); return; // 0x86

	case AXC51_SPIDMAADR: spidmaadr_w(data); return; // 0xd6
	case AXC51_SPIDMACNT: spidmacnt_w(data); return; // 0xd7
	case AXC51_SPICON: spicon_w(data); return; // 0xd8
	case AXC51_SPIBUF: spibuf_w(data); return; // 0xd9
	case AXC51_SPIBAUD: spibaud_w(data); return; // 0xda

	}

	logerror("%s: sfr_write (%02x) %02x\n", machine().describe_context(), offset, data);
	axc51base_cpu_device::sfr_write(offset, data);
}

ROM_START( ax208 ) // assume all production ax208 chips use this internal ROM
	ROM_REGION( 0x2000, "rom", 0 )
	ROM_LOAD("ax208.bin", 0x0000, 0x2000, CRC(b85f954a) SHA1(0dc7ab9bdaf73231d4d6627fe6308fe8103e1bbc) )
ROM_END

const tiny_rom_entry *ax208_cpu_device::device_rom_region() const
{
	return ROM_NAME( ax208 );
}

void ax208_cpu_device::device_reset()
{
	axc51base_cpu_device::device_reset();
	set_state_int(AXC51_PC, 0x8000);

	m_spiaddr = 0;
}


ax208p_cpu_device::ax208p_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ax208_cpu_device(mconfig, AX208P, tag, owner, clock)
{
}

ROM_START( ax208p ) // this is an early revision of the internal AX208 code, some functions are moved around so it isn't entirely compatible
	ROM_REGION( 0x2000, "rom", 0 )
	ROM_LOAD("mask208.bin", 0x0000, 0x2000, CRC(52396183) SHA1(b119000f93251894a352ecf675ee42f2e5c347bd) )
ROM_END

const tiny_rom_entry *ax208p_cpu_device::device_rom_region() const
{
	return ROM_NAME( ax208p );
}


