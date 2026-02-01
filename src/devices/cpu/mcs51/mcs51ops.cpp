// license:BSD-3-Clause
// copyright-holders:Steve Ellenoff

#include "emu.h"
#include "i8051.h"

#define VERBOSE (0)

#include "logmacro.h"

// # of oscillations each opcode requires
const u8 mcs51_cpu_device::mcs51_cycles[256] =
{
	1,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,
	2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,
	2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,
	2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,
	2,2,1,2,1,1,1,1,1,1,1,1,1,1,1,1,
	2,2,1,2,1,1,1,1,1,1,1,1,1,1,1,1,
	2,2,1,2,1,1,1,1,1,1,1,1,1,1,1,1,
	2,2,2,2,1,2,1,1,1,1,1,1,1,1,1,1,
	2,2,2,2,4,2,2,2,2,2,2,2,2,2,2,2,
	2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,
	2,2,1,2,4,1,2,2,2,2,2,2,2,2,2,2,
	2,2,1,1,2,2,2,2,2,2,2,2,2,2,2,2,
	2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	2,2,1,1,1,2,1,1,2,2,2,2,2,2,2,2,
	2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,
	2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1
};

const u8 mcs51_cpu_device::parity_value[256] =
{
	0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,
	1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,
	1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,
	0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,
	1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,
	0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,
	0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,
	1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,
	1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,
	0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,
	0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,
	1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,
	0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,
	1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,
	1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,
	0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,
};

u8 mcs51_cpu_device::bit_address_r(u8 offset)
{
	u8 word;
	u8 mask;
	int bit_pos;
	int distance; // distance between bit addressable words (1 for normal bits, 8 for sfr bit addresses)

	m_last_bit = offset;

	// User defined bit addresses 0x20-0x2f (values are 0x0-0x7f)
	if (offset < 0x80)
	{
		distance = 1;
		word = ((offset & 0x78) >> 3) * distance + 0x20;
		bit_pos = offset & 0x7;
		mask = (0x1 << bit_pos);
		return((m_intd.read_byte(word) & mask) >> bit_pos);
	}
	// SFR bit addressable registers
	else
	{
		distance = 8;
		word = ((offset & 0x78) >> 3) * distance + 0x80;
		bit_pos = offset & 0x7;
		mask = (0x1 << bit_pos);
		return ((m_intd.read_byte(word) & mask) >> bit_pos);
	}
}

void mcs51_cpu_device::bit_address_w(u8 offset, u8 bit)
{
	int word;
	u8 mask;
	int bit_pos;
	u8 result;
	int distance;

	// User defined bit addresses 0x20-0x2f (values are 0x0-0x7f)
	if (offset < 0x80)
	{
		distance = 1;
		word = ((offset & 0x78) >> 3) * distance + 0x20;
		bit_pos = offset & 0x7;
		bit = (bit & 0x1) << bit_pos;
		mask = ~(1 << bit_pos) & 0xff;
		result = m_intd.read_byte(word) & mask;
		result = result | bit;
		m_intd.write_byte(word, result);
	}
	// SFR bit addressable registers
	else
	{
		distance = 8;
		word = ((offset & 0x78) >> 3) * distance + 0x80;
		bit_pos = offset & 0x7;
		bit = (bit & 0x1) << bit_pos;
		mask = ~(1 << bit_pos) & 0xff;
		result = m_intd.read_byte(word) & mask;
		result = result | bit;
		m_intd.write_byte(word, result);
	}
}

void mcs51_cpu_device::set_reg(u8 r, u8 v)
{
	m_internal_ram[r | (m_psw & 0x18)] = v;
}

u8 mcs51_cpu_device::r_reg(u8 r)
{
	return m_internal_ram[r | (m_psw & 0x18)];
}

void mcs51_cpu_device::acc_w (u8 data)
{
	m_acc = data;
	m_psw = (m_psw & 0xfe) | parity_value[m_acc];
}

void mcs51_cpu_device::do_add_flags(u8 a, u8 data, u8 c)
{
	u16 result = a + data + c;
	s16 result1 = (s8)a + (s8)data + c;

	m_psw = (m_psw & 0x7f) | ((result & 0x100) >> 1);
	result = (a & 0x0f) + (data & 0x0f) + c;
	set_ac((result & 0x10) >> 4);
	set_ov(result1 < -128 || result1 > 127);
}

void mcs51_cpu_device::do_sub_flags(u8 a, u8 data, u8 c)
{
	u16 result = a - (data + c);
	s16 result1 = (s8)a - (s8)(data + c);

	m_psw = (m_psw & 0x7f) | ((result & 0x100) >> 1);
	result = (a & 0x0f) - ((data & 0x0f) + c);
	set_ac((result & 0x10) >> 4);
	set_ov((result1 < -128 || result1 > 127));
}


/*Push the current m_pc to the stack*/
void mcs51_cpu_device::push_pc()
{
	m_inti.write_byte(++m_sp, m_pc);        //Store low byte of m_pc to Internal Ram
	m_inti.write_byte(++m_sp, m_pc >> 8);   //Store hi byte of m_pc to next address in Internal Ram
}

/*Pop the current m_pc off the stack and into the pc*/
void mcs51_cpu_device::pop_pc()
{
	m_pc = m_inti.read_byte(m_sp--) << 8;    //Store hi byte to m_pc
	m_pc = m_pc | m_inti.read_byte(m_sp--);    //Store lo byte to m_pc
}

//ACALL code addr                           /* 1: aaa1 0001 */
void mcs51_cpu_device::acall(u8 r)
{
	u8 addr = m_program.read_byte(m_pc++);           //Grab code address byte
	push_pc();                              //Save m_pc to the stack
	//Thanks Gerrit for help with this! :)
	m_pc = (m_pc & 0xf800) | ((r & 0xe0) << 3) | addr;
}

//ADD A, #data                              /* 1: 0010 0100 */
void mcs51_cpu_device::add_a_byte(u8 r)
{
	u8 data = m_program.read_byte(m_pc++);           //Grab data
	u8 result = m_acc + data;            //Add data to accumulator
	do_add_flags(m_acc, data, 0);             //Set Flags
	acc_w(result);                            //Store 8 bit result of addition in ACC
}

//ADD A, data addr                          /* 1: 0010 0101 */
void mcs51_cpu_device::add_a_mem(u8 r)
{
	u8 addr = m_program.read_byte(m_pc++);           //Grab data address
	u8 data = m_intd.read_byte(addr);            //Grab data from data address
	u8 result = m_acc + data;            //Add data to accumulator
	do_add_flags(m_acc, data, 0);             //Set Flags
	acc_w(result);                            //Store 8 bit result of addition in ACC
}

//ADD A, @R0/@R1                            /* 1: 0010 011i */
void mcs51_cpu_device::add_a_ir(u8 r)
{
	u8 data = m_inti.read_byte(r_reg(r));       //Grab data from memory pointed to by R0 or R1
	u8 result = m_acc + data;            //Add data to accumulator
	do_add_flags(m_acc, data, 0);             //Set Flags
	acc_w(result);                            //Store 8 bit result of addition in ACC
}

//ADD A, R0 to R7                           /* 1: 0010 1rrr */
void mcs51_cpu_device::add_a_r(u8 r)
{
	u8 data = r_reg(r);                //Grab data from R0 - R7
	u8 result = m_acc + data;            //Add data to accumulator
	do_add_flags(m_acc, data, 0);             //Set Flags
	acc_w(result);                            //Store 8 bit result of addition in ACC
}

//ADDC A, #data                             /* 1: 0011 0100 */
void mcs51_cpu_device::addc_a_byte(u8 r)
{
	u8 data = m_program.read_byte(m_pc++);           //Grab data
	u8 result = m_acc + data + BIT(m_psw, PSW_CY);   //Add data + carry flag to accumulator
	do_add_flags(m_acc, data, BIT(m_psw, PSW_CY));        //Set Flags
	acc_w(result);                            //Store 8 bit result of addition in ACC
}

//ADDC A, data addr                         /* 1: 0011 0101 */
void mcs51_cpu_device::addc_a_mem(u8 r)
{
	u8 addr = m_program.read_byte(m_pc++);           //Grab data address
	u8 data = m_intd.read_byte(addr);            //Grab data from data address
	u8 result = m_acc + data + BIT(m_psw, PSW_CY);   //Add data + carry flag to accumulator
	do_add_flags(m_acc, data, BIT(m_psw, PSW_CY));        //Set Flags
	acc_w(result);                            //Store 8 bit result of addition in ACC
}

//ADDC A, @R0/@R1                           /* 1: 0011 011i */
void mcs51_cpu_device::addc_a_ir(u8 r)
{
	u8 data = m_inti.read_byte(r_reg(r));       //Grab data from memory pointed to by R0 or R1
	u8 result = m_acc + data + BIT(m_psw, PSW_CY);   //Add data + carry flag to accumulator
	do_add_flags(m_acc, data, BIT(m_psw, PSW_CY));        //Set Flags
	acc_w(result);                            //Store 8 bit result of addition in ACC
}

//ADDC A, R0 to R7                          /* 1: 0011 1rrr */
void mcs51_cpu_device::addc_a_r(u8 r)
{
	u8 data = r_reg(r);                //Grab data from R0 - R7
	u8 result = m_acc + data + BIT(m_psw, PSW_CY);   //Add data + carry flag to accumulator
	do_add_flags(m_acc, data, BIT(m_psw, PSW_CY));        //Set Flags
	acc_w(result);                            //Store 8 bit result of addition in ACC
}

//AJMP code addr                            /* 1: aaa0 0001 */
void mcs51_cpu_device::ajmp(u8 r)
{
	u8 addr = m_program.read_byte(m_pc++);           //Grab code address byte
	//Thanks Gerrit for help with this! :)
	m_pc = (m_pc & 0xf800) | ((r & 0xe0) << 3) | addr;
}

//ANL data addr, A                          /* 1: 0101 0010 */
void mcs51_cpu_device::anl_mem_a(u8 r)
{
	u8 addr = m_program.read_byte(m_pc++);           //Grab data address
	u8 data = m_intd.read_byte(addr);            //Grab data from data address
	m_intd.write_byte(addr, data & m_acc);               //Set data address value to it's value Logical AND with m_acc
}

//ANL data addr, #data                      /* 1: 0101 0011 */
void mcs51_cpu_device::anl_mem_byte(u8 r)
{
	u8 addr = m_program.read_byte(m_pc++);           //Grab data address
	u8 data = m_program.read_byte(m_pc++);           //Grab data
	u8 srcdata = m_intd.read_byte(addr);         //Grab data from data address
	m_intd.write_byte(addr, srcdata & data);           //Set data address value to it's value Logical AND with Data
}

//ANL A, #data                              /* 1: 0101 0100 */
void mcs51_cpu_device::anl_a_byte(u8 r)
{
	u8 data = m_program.read_byte(m_pc++);           //Grab data
	acc_w(m_acc & data);                    //Set ACC to value of ACC Logical AND with Data
}

//ANL A, data addr                          /* 1: 0101 0101 */
void mcs51_cpu_device::anl_a_mem(u8 r)
{
	u8 addr = m_program.read_byte(m_pc++);           //Grab data address
	u8 data = m_intd.read_byte(addr);            //Grab data from data address
	acc_w(m_acc & data);                    //Set ACC to value of ACC Logical AND with Data
}

//ANL A, @RO/@R1                            /* 1: 0101 011i */
void mcs51_cpu_device::anl_a_ir(u8 r)
{
	u8 data = m_inti.read_byte(r_reg(r));       //Grab data from address R0 or R1 points to
	acc_w(m_acc & data);                    //Set ACC to value of ACC Logical AND with Data
}

//ANL A, RO to R7                           /* 1: 0101 1rrr */
void mcs51_cpu_device::anl_a_r(u8 r)
{
	u8 data = r_reg(r);                //Grab data from R0 - R7
	acc_w(m_acc & data);                    //Set ACC to value of ACC Logical AND with Data
}

//ANL C, bit addr                           /* 1: 1000 0010 */
void mcs51_cpu_device::anl_c_bitaddr(u8 r)
{
	u8 addr = m_program.read_byte(m_pc++);           //Grab bit address
	u8 bit = bit_address_r(addr);              //Grab bit data from bit address
	m_psw &= (bit << 7) | 0x7f;        //Set Carry flag to Carry Flag Value Logical AND with Bit
}

//ANL C,/bit addr                           /* 1: 1011 0000 */
void mcs51_cpu_device::anl_c_nbitaddr(u8 r)
{
	u8 addr = m_program.read_byte(m_pc++);           //Grab bit address
	u8 bit = bit_address_r(addr);              //Grab bit data from bit address
	bit = (~bit & 1);                       //Complement bit
	m_psw &= (bit << 7) | 0x7f;                       //Set Carry flag to Carry Flag Value Logical AND with Complemented Bit
}

//CJNE A, #data, code addr                  /* 1: 1011 0100 */
void mcs51_cpu_device::cjne_a_byte(u8 r)
{
	u8 data = m_program.read_byte(m_pc++);           //Grab data
	s8 rel_addr = m_program.read_byte(m_pc++);        //Grab relative code address

	if (m_acc != data)                        //Jump if values are not equal
	{
		m_pc = m_pc + rel_addr;
	}

	//Set carry flag to 1 if 1st compare value is < 2nd compare value
	set_cy(m_acc < data);
}

//CJNE A, data addr, code addr              /* 1: 1011 0101 */
void mcs51_cpu_device::cjne_a_mem(u8 r)
{
	u8 addr = m_program.read_byte(m_pc++);           //Grab data address
	s8 rel_addr = m_program.read_byte(m_pc++);        //Grab relative code address
	u8 data = m_intd.read_byte(addr);            //Pull value from data address

	if (m_acc != data)                        //Jump if values are not equal
	{
		m_pc = m_pc + rel_addr;
	}

	//Set carry flag to 1 if 1st compare value is < 2nd compare value
	set_cy(m_acc < data);
}

//CJNE @R0/@R1, #data, code addr            /* 1: 1011 011i */
void mcs51_cpu_device::cjne_ir_byte(u8 r)
{
	u8 data = m_program.read_byte(m_pc++);           //Grab data
	s8 rel_addr = m_program.read_byte(m_pc++);        //Grab relative code address
	u8 srcdata = m_inti.read_byte(r_reg(r));    //Grab value pointed to by R0 or R1

	if (srcdata != data)                    //Jump if values are not equal
	{
		m_pc = m_pc + rel_addr;
	}

	//Set carry flag to 1 if 1st compare value is < 2nd compare value
	set_cy(srcdata < data);
}

//CJNE R0 to R7, #data, code addr           /* 1: 1011 1rrr */
void mcs51_cpu_device::cjne_r_byte(u8 r)
{
	u8 data = m_program.read_byte(m_pc++);           //Grab data
	s8 rel_addr = m_program.read_byte(m_pc++);        //Grab relative code address
	u8 srcdata = r_reg(r);             //Grab value of R0 - R7

	if (srcdata != data)                    //Jump if values are not equal
	{
		m_pc = m_pc + rel_addr;
	}

	//Set carry flag to 1 if 1st compare value is < 2nd compare value
	set_cy(srcdata < data);
}

//CLR bit addr                              /* 1: 1100 0010 */
void mcs51_cpu_device::clr_bitaddr(u8 r)
{
	u8 addr = m_program.read_byte(m_pc++);           //Grab bit address
	bit_address_w(addr, 0);                         //Clear bit at specified bit address
}

//CLR C                                     /* 1: 1100 0011 */
void mcs51_cpu_device::clr_c(u8 r)
{
	m_psw &= 0x7f;                              //Clear Carry Flag
}

//CLR A                                     /* 1: 1110 0100 */
void mcs51_cpu_device::clr_a(u8 r)
{
	acc_w(0);                             //Clear Accumulator
}

//CPL bit addr                              /* 1: 1011 0010 */
void mcs51_cpu_device::cpl_bitaddr(u8 r)
{
	u8 addr = m_program.read_byte(m_pc++);           //Grab bit address
	u8 data = (~bit_address_r(addr)) & 1;
	bit_address_w(addr, data);                      //Complement bit at specified bit address
}

//CPL C                                     /* 1: 1011 0011 */
void mcs51_cpu_device::cpl_c(u8 r)
{
	m_psw ^= 0x80;            //Complement Carry Flag
}

//CPL A                                     /* 1: 1111 0100 */
void mcs51_cpu_device::cpl_a(u8 r)
{
	u8 data = ((~m_acc) & 0xff);
	acc_w(data);                          //Complement Accumulator
}

//DA A                                      /* 1: 1101 0100 */
void mcs51_cpu_device::da_a(u8 r)
{
/*From several sources, since none said the same thing:
 The decimal adjust instruction is associated with the use of the ADD and ADDC instructions.
 The eight-bit value in the accumulator is adjusted to form two BCD digits of four bits each.
 If the accumulator contents bits 0-3 are greater than 9, OR the AC flag is set, then six is added to
 produce a proper BCD digit.
 If the carry is set, OR the four high bits 4-7 exceed nine, six is added to the value of these bits.
 The carry flag will be set if the result is > 0x99, but not cleared otherwise */

	u16 new_acc = m_acc & 0xff;
	if (BIT(m_psw, PSW_AC) || (new_acc & 0x0f) > 0x09)
		new_acc += 0x06;
	if (BIT(m_psw, PSW_CY) || ((new_acc & 0xf0) > 0x90) || (new_acc & ~0xff))
		new_acc += 0x60;
	acc_w(new_acc & 0xff);
	if (new_acc & ~0xff)
		m_psw |= 0x80;
}

//DEC A                                     /* 1: 0001 0100 */
void mcs51_cpu_device::dec_a(u8 r)
{
	acc_w(m_acc - 1);
}

//DEC data addr                             /* 1: 0001 0101 */
void mcs51_cpu_device::dec_mem(u8 r)
{
	u8 addr = m_program.read_byte(m_pc++);           //Grab data address
	u8 data = m_intd.read_byte(addr);
	m_intd.write_byte(addr, data - 1);
}

//DEC @R0/@R1                               /* 1: 0001 011i */
void mcs51_cpu_device::dec_ir(u8 r)
{
	u8 data = m_inti.read_byte(r_reg(r));
	m_inti.write_byte(r_reg(r), data - 1);
}

//DEC R0 to R7                              /* 1: 0001 1rrr */
void mcs51_cpu_device::dec_r(u8 r)
{
	set_reg(r, r_reg(r) - 1);
}

//DIV AB                                    /* 1: 1000 0100 */
void mcs51_cpu_device::div_ab(u8 r)
{
	if (m_b == 0)
	{
		//Overflow flag is set!
		set_ov(1);
		//Really the values are undefined according to the manual, but we'll just leave them as is..
		//acc_w(0xff);
		//SFR_W(B, 0xff);
	}
	else
	{
		u8 a = m_acc / m_b;
		u8 b = m_acc % m_b;
		//A gets quotient byte, B gets remainder byte
		acc_w(a);
		m_b = b;
		//Overflow flag is cleared
		set_ov(0);
	}
	//Carry Flag is always cleared
	m_psw &= 0x7f;
}

//DJNZ data addr, code addr                 /* 1: 1101 0101 */
void mcs51_cpu_device::djnz_mem(u8 r)
{
	u8 addr = m_program.read_byte(m_pc++);           //Grab data address
	s8 rel_addr = m_program.read_byte(m_pc++);        //Grab relative code address
	m_intd.write_byte(addr, m_intd.read_byte(addr) - 1);         //Decrement value contained at data address
	if (m_intd.read_byte(addr) != 0)                  //Branch if contents of data address is not 0
	{
		m_pc = m_pc + rel_addr;
	}
}

//DJNZ R0 to R7,code addr                   /* 1: 1101 1rrr */
void mcs51_cpu_device::djnz_r(u8 r)
{
	s8 rel_addr = m_program.read_byte(m_pc++);        //Grab relative code address
	set_reg(r, r_reg(r) - 1);               //Decrement value
	if (r_reg(r) != 0)                      //Branch if contents of R0 - R7 is not 0
	{
		m_pc = m_pc + rel_addr;
	}
}

//INC A                                     /* 1: 0000 0100 */
void mcs51_cpu_device::inc_a(u8 r)
{
	acc_w(m_acc + 1);
}

//INC data addr                             /* 1: 0000 0101 */
void mcs51_cpu_device::inc_mem(u8 r)
{
	u8 addr = m_program.read_byte(m_pc++);           //Grab data address
	u8 data = m_intd.read_byte(addr);
	m_intd.write_byte(addr, data + 1);
}

//INC @R0/@R1                               /* 1: 0000 011i */
void mcs51_cpu_device::inc_ir(u8 r)
{
	u8 data = m_inti.read_byte(r_reg(r));
	m_inti.write_byte(r_reg(r), data + 1);
}

//INC R0 to R7                              /* 1: 0000 1rrr */
void mcs51_cpu_device::inc_r(u8 r)
{
	u8 data = r_reg(r);
	set_reg(r, data + 1);
}

//INC m_dptr                                  /* 1: 1010 0011 */
void mcs51_cpu_device::inc_dptr(u8 r)
{
	m_dptr++;
}

//JB  bit addr, code addr                   /* 1: 0010 0000 */
void mcs51_cpu_device::jb(u8 r)
{
	u8 addr = m_program.read_byte(m_pc++);           //Grab bit address
	s8 rel_addr = m_program.read_byte(m_pc++);        //Grab relative code address
	if (bit_address_r(addr))                        //If bit set at specified bit address, jump
	{
		m_pc = m_pc + rel_addr;
	}
}

//JBC bit addr, code addr                   /* 1: 0001 0000 */
void mcs51_cpu_device::jbc(u8 r)
{
	u8 addr = m_program.read_byte(m_pc++);           //Grab bit address
	s8 rel_addr = m_program.read_byte(m_pc++);        //Grab relative code address
	if (bit_address_r(addr))                        //If bit set at specified bit address, jump
	{
		m_pc = m_pc + rel_addr;
		bit_address_w(addr, 0);                     //Clear Bit also
	}
}

//JC code addr                              /* 1: 0100 0000 */
void mcs51_cpu_device::jc(u8 r)
{
	s8 rel_addr = m_program.read_byte(m_pc++);        //Grab relative code address
	if (BIT(m_psw, PSW_CY))                             //Jump if Carry Flag Set
	{
		m_pc = m_pc + rel_addr;
	}
}

//JMP @A+m_dptr                               /* 1: 0111 0011 */
void mcs51_cpu_device::jmp_iadptr(u8 r)
{
	m_pc = m_acc + m_dptr;
}

//JNB bit addr, code addr                   /* 1: 0011 0000 */
void mcs51_cpu_device::jnb(u8 r)
{
	u8 addr = m_program.read_byte(m_pc++);           //Grab bit address
	s8 rel_addr = m_program.read_byte(m_pc++);        //Grab relative code address
	if (!bit_address_r(addr))                       //If bit NOT set at specified bit address, jump
	{
		m_pc = m_pc + rel_addr;
	}
}

//JNC code addr                             /* 1: 0101 0000 */
void mcs51_cpu_device::jnc(u8 r)
{
	s8 rel_addr = m_program.read_byte(m_pc++);        //Grab relative code address
	if (!BIT(m_psw, PSW_CY))                            //Jump if Carry Flag not set
	{
		m_pc = m_pc + rel_addr;
	}
}

//JNZ code addr                             /* 1: 0111 0000 */
void mcs51_cpu_device::jnz(u8 r)
{
	s8 rel_addr = m_program.read_byte(m_pc++);        //Grab relative code address
	if (m_acc != 0)                           //Branch if m_acc is not 0
	{
		m_pc = m_pc + rel_addr;
	}
}

//JZ code addr                              /* 1: 0110 0000 */
void mcs51_cpu_device::jz(u8 r)
{
	s8 rel_addr = m_program.read_byte(m_pc++);        //Grab relative code address
	if (m_acc == 0)                           //Branch if m_acc is 0
	{
		m_pc = m_pc + rel_addr;
	}
}

//LCALL code addr                           /* 1: 0001 0010 */
void mcs51_cpu_device::lcall(u8 r)
{
	u8 addr_hi, addr_lo;
	addr_hi = m_program.read_byte(m_pc++);
	addr_lo = m_program.read_byte(m_pc++);
	push_pc();
	m_pc = (u16)((addr_hi << 8) | addr_lo);
}

//LJMP code addr                            /* 1: 0000 0010 */
void mcs51_cpu_device::ljmp(u8 r)
{
	u8 addr_hi, addr_lo;
	addr_hi = m_program.read_byte(m_pc++);
	addr_lo = m_program.read_byte(m_pc++);
	m_pc = (u16)((addr_hi << 8) | addr_lo);
}

//MOV A, #data                              /* 1: 0111 0100 */
void mcs51_cpu_device::mov_a_byte(u8 r)
{
	u8 data = m_program.read_byte(m_pc++);           //Grab data
	acc_w(data);                          //Store data to ACC
}

//MOV A, data addr                          /* 1: 1110 0101 */
void mcs51_cpu_device::mov_a_mem(u8 r)
{
	u8 addr = m_program.read_byte(m_pc++);           //Grab data address
	acc_w(m_intd.read_byte(addr));                  //Store contents of data address to ACC
}

//MOV A,@RO/@R1                             /* 1: 1110 011i */
void mcs51_cpu_device::mov_a_ir(u8 r)
{
	acc_w(m_inti.read_byte(r_reg(r)));             //Store contents of address pointed by R0 or R1 to ACC
}

//MOV A,R0 to R7                            /* 1: 1110 1rrr */
void mcs51_cpu_device::mov_a_r(u8 r)
{
	acc_w(r_reg(r));                      //Store contents of R0 - R7 to ACC
}

//MOV data addr, #data                      /* 1: 0111 0101 */
void mcs51_cpu_device::mov_mem_byte(u8 r)
{
	u8 addr = m_program.read_byte(m_pc++);           //Grab data address
	u8 data = m_program.read_byte(m_pc++);           //Grab data
	m_intd.write_byte(addr, data);                     //Store data to data address location
}

//MOV data addr, data addr                  /* 1: 1000 0101 */
void mcs51_cpu_device::mov_mem_mem(u8 r)
{
	//1st address is src, 2nd is dst, but the mov command works as mov dst,src)
	u8 src,dst;
	src = m_program.read_byte(m_pc++);                    //Grab source data address
	dst = m_program.read_byte(m_pc++);                    //Grab destination data address
	m_intd.write_byte(dst, m_intd.read_byte(src));               //Read source address contents and store to destination address
}

//MOV @R0/@R1, #data                        /* 1: 0111 011i */
void mcs51_cpu_device::mov_ir_byte(u8 r)
{
	u8 data = m_program.read_byte(m_pc++);           //Grab data
	m_inti.write_byte(r_reg(r), data);                //Store data to address pointed by R0 or R1
}

//MOV R0 to R7, #data                       /* 1: 0111 1rrr */
void mcs51_cpu_device::mov_r_byte(u8 r)
{
	u8 data = m_program.read_byte(m_pc++);           //Grab data
	set_reg(r, data);                       //Store to R0 - R7
}

//MOV data addr, @R0/@R1                    /* 1: 1000 011i */
void mcs51_cpu_device::mov_mem_ir(u8 r)
{
	u8 addr = m_program.read_byte(m_pc++);           //Grab data address
	m_intd.write_byte(addr, m_inti.read_byte(r_reg(r)));        //Store contents pointed to by R0 or R1 to data address
}

//MOV data addr,R0 to R7                    /* 1: 1000 1rrr */
void mcs51_cpu_device::mov_mem_r(u8 r)
{
	u8 addr = m_program.read_byte(m_pc++);           //Grab data address
	m_intd.write_byte(addr, r_reg(r));                 //Store contents of R0 - R7 to data address
}

//MOV m_dptr, #data16                         /* 1: 1001 0000 */
void mcs51_cpu_device::mov_dptr_byte(u8 r)
{
	u8 data_hi, data_lo;
	data_hi = m_program.read_byte(m_pc++);                //Grab hi byte
	data_lo = m_program.read_byte(m_pc++);                //Grab lo byte
	m_dptr = (data_hi << 8) | data_lo;      //Store to DPTR
}

//MOV bit addr, C                           /* 1: 1001 0010 */
void mcs51_cpu_device::mov_bitaddr_c(u8 r)
{
	u8 addr = m_program.read_byte(m_pc++);           //Grab bit address
	bit_address_w(addr, BIT(m_psw, PSW_CY));                    //Store Carry Flag to Bit Address
}

//MOV @R0/@R1, data addr                    /* 1: 1010 011i */
void mcs51_cpu_device::mov_ir_mem(u8 r)
{
	u8 addr = m_program.read_byte(m_pc++);           //Grab data address
	m_inti.write_byte(r_reg(r), m_intd.read_byte(addr));        //Store data from data address to address pointed to by R0 or R1
}

//MOV R0 to R7, data addr                   /* 1: 1010 1rrr */
void mcs51_cpu_device::mov_r_mem(u8 r)
{
	u8 addr = m_program.read_byte(m_pc++);           //Grab data address
	set_reg(r, m_intd.read_byte(addr));               //Store to R0 - R7
}

//MOV data addr, A                          /* 1: 1111 0101 */
void mcs51_cpu_device::mov_mem_a(u8 r)
{
	u8 addr = m_program.read_byte(m_pc++);           //Grab data address
	m_intd.write_byte(addr, m_acc);                      //Store A to data address
}

//MOV @R0/@R1, A                            /* 1: 1111 011i */
void mcs51_cpu_device::mov_ir_a(u8 r)
{
	m_inti.write_byte(r_reg(r), m_acc);                 //Store A to location pointed to by R0 or R1
}

//MOV R0 to R7, A                           /* 1: 1111 1rrr */
void mcs51_cpu_device::mov_r_a(u8 r)
{
	set_reg(r, m_acc);                        //Store A to R0-R7
}

//MOVC A, @A + m_pc                           /* 1: 1000 0011 */
void mcs51_cpu_device::movc_a_iapc(u8 r)
{
	u8 data;
	data = m_program.read_byte(m_acc + m_pc);             //Move a byte from CODE(Program) Memory and store to ACC
	acc_w(data);
}

//MOV C, bit addr                           /* 1: 1010 0010 */
void mcs51_cpu_device::mov_c_bitaddr(u8 r)
{
	u8 addr = m_program.read_byte(m_pc++);           //Grab bit address
	set_cy(bit_address_r(addr));                    //Store Bit from Bit Address to Carry Flag
}

//MOVC A, @A + m_dptr                         /* 1: 1001 0011 */
void mcs51_cpu_device::movc_a_iadptr(u8 r)
{
	u8 data = m_program.read_byte(m_acc + m_dptr);           //Move a byte from CODE(Program) Memory and store to ACC
	acc_w(data);
}

//MOVX A,@m_dptr                              /* 1: 1110 0000 */
//(Move External Ram 16 bit address to A)
void mcs51_cpu_device::movx_a_idptr(u8 r)
{
//  u8 byte = m_data.read_byte(R_m_dptr);       //Grab 1 byte from External DATA memory pointed to by dptr
	u32 addr = external_ram_iaddr(m_dptr, 0xffff);
	u8 byte = m_data.read_byte(addr);         //Grab 1 byte from External DATA memory pointed to by dptr
	acc_w(byte);                          //Store to ACC
}

//MOVX A, @R0/@R1                           /* 1: 1110 001i */
//(Move External Ram 8 bit address to A)
void mcs51_cpu_device::movx_a_ir(u8 r)
{
	u32 addr = external_ram_iaddr(r_reg(r), 0xff); //Grab address by reading location pointed to by R0 or R1
	u8 byte = m_data.read_byte(addr);         //Grab 1 byte from External DATA memory pointed to by address
	acc_w(byte);                          //Store to ACC
}

//MOVX @m_dptr,A                              /* 1: 1111 0000 */
//(Move A to External Ram 16 bit address)
void mcs51_cpu_device::movx_idptr_a(u8 r)
{
//  m_data.write_byte(R_m_dptr, m_acc);                 //Store m_acc to External DATA memory address pointed to by DPTR
	u32 addr = external_ram_iaddr(m_dptr, 0xffff);
	m_data.write_byte(addr, m_acc);                   //Store m_acc to External DATA memory address pointed to by DPTR
}

//MOVX @R0/@R1,A                            /* 1: 1111 001i */
//(Move A to External Ram 8 bit address)
void mcs51_cpu_device::movx_ir_a(u8 r)
{
	u32 addr = external_ram_iaddr(r_reg(r), 0xff); //Grab address by reading location pointed to by R0 or R1
	m_data.write_byte(addr, m_acc);                   //Store m_acc to External DATA memory address
}

//MUL AB                                    /* 1: 1010 0100 */
void mcs51_cpu_device::mul_ab(u8 r)
{
	u16 result = m_acc * m_b;
	//A gets lo bits, B gets hi bits of result
	m_b = (u8)((result & 0xff00) >> 8);
	acc_w((u8)(result & 0x00ff));
	//Set flags
	set_ov((result & 0x100) >> 8);          //Set/Clear Overflow Flag if result > 255
	m_psw &= 0x7f;                          //Carry Flag always cleared
}

//NOP                                       /* 1: 0000 0000 */
void mcs51_cpu_device::nop(u8 r)
{
}

//ORL data addr, A                          /* 1: 0100 0010 */
void mcs51_cpu_device::orl_mem_a(u8 r)
{
	u8 addr = m_program.read_byte(m_pc++);           //Grab data address
	u8 data = m_intd.read_byte(addr);            //Grab data from data address
	m_intd.write_byte(addr, data | m_acc);               //Set data address value to it's value Logical OR with ACC
}

//ORL data addr, #data                      /* 1: 0100 0011 */
void mcs51_cpu_device::orl_mem_byte(u8 r)
{
	u8 addr = m_program.read_byte(m_pc++);           //Grab data address
	u8 data = m_program.read_byte(m_pc++);           //Grab data
	u8 srcdata = m_intd.read_byte(addr);         //Grab data from data address
	m_intd.write_byte(addr, srcdata | data);           //Set data address value to it's value Logical OR with Data
}

//ORL A, #data                              /* 1: 0100 0100 */
void mcs51_cpu_device::orl_a_byte(u8 r)
{
	u8 data = m_program.read_byte(m_pc++);           //Grab data
	acc_w(m_acc | data);                    //Set ACC to value of ACC Logical OR with Data
}

//ORL A, data addr                          /* 1: 0100 0101 */
void mcs51_cpu_device::orl_a_mem(u8 r)
{
	u8 addr = m_program.read_byte(m_pc++);           //Grab data address
	u8 data = m_intd.read_byte(addr);            //Grab data from data address
	acc_w(m_acc | data);                    //Set ACC to value of ACC Logical OR with Data
}

//ORL A, @RO/@R1                            /* 1: 0100 011i */
void mcs51_cpu_device::orl_a_ir(u8 r)
{
	u8 data = m_inti.read_byte(r_reg(r));       //Grab data from address R0 or R1 points to
	acc_w(m_acc | data);                    //Set ACC to value of ACC Logical OR with Data
}

//ORL A, RO to R7                           /* 1: 0100 1rrr */
void mcs51_cpu_device::orl_a_r(u8 r)
{
	u8 data = r_reg(r);                //Grab data from R0 - R7
	acc_w(m_acc | data);                    //Set ACC to value of ACC Logical OR with Data
}

//ORL C, bit addr                           /* 1: 0111 0010 */
void mcs51_cpu_device::orl_c_bitaddr(u8 r)
{
	u8 addr = m_program.read_byte(m_pc++);           //Grab bit address
	u8 bit = bit_address_r(addr);              //Grab bit data from bit address
	m_psw |= bit << 7;                 //Set Carry flag to Carry Flag Value Logical OR with Bit
}

//ORL C, /bit addr                          /* 1: 1010 0000 */
void mcs51_cpu_device::orl_c_nbitaddr(u8 r)
{
	u8 addr = m_program.read_byte(m_pc++);           //Grab bit address
	u8 bit = bit_address_r(addr);              //Grab bit data from bit address
	bit = (~bit & 1);                       //Complement bit
	m_psw |= bit << 7;                      //Set Carry flag to Carry Flag Value Logical OR with Complemented Bit
}

//POP data addr                             /* 1: 1101 0000 */
void mcs51_cpu_device::pop(u8 r)
{
	u8 addr = m_program.read_byte(m_pc++);           //Grab data address
	m_intd.write_byte(addr, m_inti.read_byte(m_sp));              //Store to contents of data addr, data pointed to by Stack - m_inti.read_byte needed to access upper 128 bytes of stack
	//m_inti.write_byte(addr, m_inti.read_byte(R_m_sp));         //Store to contents of data addr, data pointed to by Stack - doesn't work, sfr's are not restored this way and it's not an indirect access anyway
	m_sp --;                                  //Decrement m_sp
}

//PUSH data addr                            /* 1: 1100 0000 */
void mcs51_cpu_device::push(u8 r)
{
	u8 addr = m_program.read_byte(m_pc++);           //Grab data address
	m_sp ++;                 //Grab and Increment Stack Pointer
	m_inti.write_byte(m_sp, m_intd.read_byte(addr));           //Store to stack contents of data address - m_inti.write_byte needed to store to upper 128 bytes of stack, however, can't use m_inti.read_byte because that won't store the sfrs and it's not an indirect access anyway
}

//RET                                       /* 1: 0010 0010 */
void mcs51_cpu_device::ret(u8 r)
{
	pop_pc();
}

//RETI                                      /* 1: 0011 0010 */
void mcs51_cpu_device::reti(u8 r)
{
	pop_pc();
	clear_current_irq();
}

//RL A                                      /* 1: 0010 0011 */
void mcs51_cpu_device::rl_a(u8 r)
{
	//Left Shift A, Bit 7 carries to Bit 0
	u8 carry = ((m_acc & 0x80) >> 7);
	u8 data = (m_acc << 1) & 0xfe;
	acc_w(data | carry);
}

//RLC A                                     /* 1: 0011 0011 */
void mcs51_cpu_device::rlc_a(u8 r)
{
	//Left Shift A, Bit 7 goes to Carry Flag, Original Carry Flag goes to Bit 0 of ACC
	u8 carry = ((m_acc & 0x80) >> 7);
	u8 data = ((m_acc << 1) & 0xfe) | BIT(m_psw, PSW_CY);
	acc_w(data);
	set_cy(carry);
}

//RR A                                      /* 1: 0000 0011 */
void mcs51_cpu_device::rr_a(u8 r)
{
	//Right Shift A, Bit 0 carries to Bit 7
	u8 carry = ((m_acc & 1) << 7);
	u8 data = (m_acc >> 1) & 0x7f;
	acc_w(data | carry);
}

//RRC A                                     /* 1: 0001 0011 */
void mcs51_cpu_device::rrc_a(u8 r)
{
	//Right Shift A, Bit 0 goes to Carry Flag, Bit 7 of ACC gets set to original Carry Flag
	u8 carry = (m_acc & 1);
	u8 data = ((m_acc >> 1) & 0x7f) | (BIT(m_psw, PSW_CY) << 7);
	acc_w(data);
	set_cy(carry);
}

//SETB C                                    /* 1: 1101 0011 */
void mcs51_cpu_device::setb_c(u8 r)
{
	//Set Carry Flag
	m_psw |= 0x80;
}

//SETB bit addr                             /* 1: 1101 0010 */
void mcs51_cpu_device::setb_bitaddr(u8 r)
{
	u8 addr = m_program.read_byte(m_pc++);           //Grab bit address
	bit_address_w(addr, 1);                         //Set Bit at Bit Address
}

//SJMP code addr                            /* 1: 1000 0000 */
void mcs51_cpu_device::sjmp(u8 r)
{
	s8 rel_addr = m_program.read_byte(m_pc++);        //Grab relative code address
	m_pc = m_pc + rel_addr;                     //Update m_pc
}

//SUBB A, #data                             /* 1: 1001 0100 */
void mcs51_cpu_device::subb_a_byte(u8 r)
{
	u8 data = m_program.read_byte(m_pc++);           //Grab data
	u8 result = m_acc - data - BIT(m_psw, PSW_CY);   //Subtract data & carry flag from accumulator
	do_sub_flags(m_acc, data, BIT(m_psw, PSW_CY));        //Set Flags
	acc_w(result);                            //Store 8 bit result of addition in ACC

}

//SUBB A, data addr                         /* 1: 1001 0101 */
void mcs51_cpu_device::subb_a_mem(u8 r)
{
	u8 addr = m_program.read_byte(m_pc++);           //Grab data address
	u8 data = m_intd.read_byte(addr);            //Grab data from data address
	u8 result = m_acc - data - BIT(m_psw, PSW_CY);   //Subtract data & carry flag from accumulator
	do_sub_flags(m_acc, data, BIT(m_psw, PSW_CY));        //Set Flags
	acc_w(result);                            //Store 8 bit result of addition in ACC
}

//SUBB A, @R0/@R1                           /* 1: 1001 011i */
void mcs51_cpu_device::subb_a_ir(u8 r)
{
	u8 data = m_inti.read_byte(r_reg(r));       //Grab data from memory pointed to by R0 or R1
	u8 result = m_acc - data - BIT(m_psw, PSW_CY);   //Subtract data & carry flag from accumulator
	do_sub_flags(m_acc, data, BIT(m_psw, PSW_CY));        //Set Flags
	acc_w(result);                            //Store 8 bit result of addition in ACC
}

//SUBB A, R0 to R7                          /* 1: 1001 1rrr */
void mcs51_cpu_device::subb_a_r(u8 r)
{
	u8 data = r_reg(r);                //Grab data from R0 - R7
	u8 result = m_acc - data - BIT(m_psw, PSW_CY);   //Subtract data & carry flag from accumulator
	do_sub_flags(m_acc, data, BIT(m_psw, PSW_CY));        //Set Flags
	acc_w(result);                            //Store 8 bit result of addition in ACC
}

//SWAP A                                    /* 1: 1100 0100 */
void mcs51_cpu_device::swap_a(u8 r)
{
	u8 a_nib_lo, a_nib_hi;
	a_nib_hi = (m_acc & 0x0f) << 4;           //Grab lo byte of ACC and move to hi
	a_nib_lo = (m_acc & 0xf0) >> 4;           //Grab hi byte of ACC and move to lo
	acc_w(a_nib_hi | a_nib_lo);
}

//XCH A, data addr                          /* 1: 1100 0101 */
void mcs51_cpu_device::xch_a_mem(u8 r)
{
	u8 addr = m_program.read_byte(m_pc++);           //Grab data address
	u8 data = m_intd.read_byte(addr);            //Grab data
	u8 oldacc = m_acc;                   //Hold value of ACC
	acc_w(data);                              //Sets m_acc to data
	m_intd.write_byte(addr, oldacc);                     //Sets data address to old value of ACC
}

//XCH A, @RO/@R1                            /* 1: 1100 011i */
void mcs51_cpu_device::xch_a_ir(u8 r)
{
	u8 data = m_inti.read_byte(r_reg(r));       //Grab data pointed to by R0 or R1
	u8 oldacc = m_acc;                   //Hold value of ACC
	acc_w(data);                              //Sets m_acc to data
	m_inti.write_byte(r_reg(r), oldacc);                //Sets data address to old value of ACC
}

//XCH A, RO to R7                           /* 1: 1100 1rrr */
void mcs51_cpu_device::xch_a_r(u8 r)
{
	u8 data = r_reg(r);                //Grab data from R0-R7
	u8 oldacc = m_acc;                   //Hold value of ACC
	acc_w(data);                              //Sets m_acc to data
	set_reg(r, oldacc);                       //Sets data address to old value of ACC
}

//XCHD A, @R0/@R1                           /* 1: 1101 011i */
void mcs51_cpu_device::xchd_a_ir(u8 r)
{
	u8 acc = m_acc;
	u8 ir_data = m_inti.read_byte(r_reg(r));            //Grab data pointed to by R0 or R1
	acc_w((acc & 0xf0) | (ir_data & 0x0f)); //Set ACC to lower nibble of data pointed to by R0 or R1
	m_inti.write_byte(r_reg(r), (ir_data & 0xf0) | (acc & 0x0f)); //Set data pointed to by R0 or R1 to lower nibble of ACC
}

//XRL data addr, A                          /* 1: 0110 0010 */
void mcs51_cpu_device::xrl_mem_a(u8 r)
{
	u8 addr = m_program.read_byte(m_pc++);           //Grab data address
	u8 data = m_intd.read_byte(addr);            //Grab data from data address
	m_intd.write_byte(addr, data ^ m_acc);               //Set data address value to it's value Logical XOR with m_acc
}

//XRL data addr, #data                      /* 1: 0110 0011 */
void mcs51_cpu_device::xrl_mem_byte(u8 r)
{
	u8 addr = m_program.read_byte(m_pc++);           //Grab data address
	u8 data = m_program.read_byte(m_pc++);           //Grab data
	u8 srcdata = m_intd.read_byte(addr);         //Grab data from data address
	m_intd.write_byte(addr, srcdata ^ data);           //Set data address value to it's value Logical XOR with Data
}

//XRL A, #data                              /* 1: 0110 0100 */
void mcs51_cpu_device::xrl_a_byte(u8 r)
{
	u8 data = m_program.read_byte(m_pc++);           //Grab data
	acc_w(m_acc ^ data);                    //Set ACC to value of ACC Logical XOR with Data
}

//XRL A, data addr                          /* 1: 0110 0101 */
void mcs51_cpu_device::xrl_a_mem(u8 r)
{
	u8 addr = m_program.read_byte(m_pc++);           //Grab data address
	u8 data = m_intd.read_byte(addr);            //Grab data from data address
	acc_w(m_acc ^ data);                    //Set ACC to value of ACC Logical XOR with Data
}

//XRL A, @R0/@R1                            /* 1: 0110 011i */
void mcs51_cpu_device::xrl_a_ir(u8 r)
{
	u8 data = m_inti.read_byte(r_reg(r));       //Grab data from address R0 or R1 points to
	acc_w(m_acc ^ data);                    //Set ACC to value of ACC Logical XOR with Data
}

//XRL A, R0 to R7                           /* 1: 0110 1rrr */
void mcs51_cpu_device::xrl_a_r(u8 r)
{
	u8 data = r_reg(r);                //Grab data from R0 - R7
	acc_w(m_acc ^ data);                    //Set ACC to value of ACC Logical XOR with Data
}

//illegal opcodes
void mcs51_cpu_device::illegal(u8 r)
{
	LOG("illegal opcode at 0x%03x: %02x\n", m_pc-1, r);
}

void mcs51_cpu_device::execute_op(u8 op)
{
	m_last_op = op;

	switch (op)
	{
		case 0x00: nop(op);                             break; //NOP
		case 0x01: ajmp(op);                            break; //AJMP code addr
		case 0x02: ljmp(op);                            break; //LJMP code addr
		case 0x03: rr_a(op);                            break; //RR A
		case 0x04: inc_a(op);                           break; //INC A
		case 0x05: m_rwm = 1; inc_mem(op); m_rwm = 0;       break; //INC data addr

		case 0x06:
		case 0x07: inc_ir(op & 1);                      break; //INC @R0/@R1

		case 0x08:
		case 0x09:
		case 0x0a:
		case 0x0b:
		case 0x0c:
		case 0x0d:
		case 0x0e:
		case 0x0f: inc_r(op & 7);                       break; //INC R0 to R7

		case 0x10: m_rwm = 1; jbc(op); m_rwm = 0;           break; //JBC bit addr, code addr
		case 0x11: acall(op);                           break; //ACALL code addr
		case 0x12: lcall(op);                           break; //LCALL code addr
		case 0x13: rrc_a(op);                           break; //RRC A
		case 0x14: dec_a(op);                           break; //DEC A
		case 0x15: m_rwm = 1; dec_mem(op); m_rwm = 0;       break; //DEC data addr

		case 0x16:
		case 0x17: dec_ir(op & 1);                      break; //DEC @R0/@R1

		case 0x18:
		case 0x19:
		case 0x1a:
		case 0x1b:
		case 0x1c:
		case 0x1d:
		case 0x1e:
		case 0x1f: dec_r(op & 7);                       break; //DEC R0 to R7

		case 0x20: jb(op);                              break; //JB  bit addr, code addr
		case 0x21: ajmp(op);                            break; //AJMP code addr
		case 0x22: ret(op);                             break; //RET
		case 0x23: rl_a(op);                            break; //RL A
		case 0x24: add_a_byte(op);                      break; //ADD A, #data
		case 0x25: add_a_mem(op);                       break; //ADD A, data addr

		case 0x26:
		case 0x27: add_a_ir(op & 1);                    break; //ADD A, @R0/@R1

		case 0x28:
		case 0x29:
		case 0x2a:
		case 0x2b:
		case 0x2c:
		case 0x2d:
		case 0x2e:
		case 0x2f: add_a_r(op & 7);                     break; //ADD A, R0 to R7

		case 0x30: jnb(op);                             break; //JNB bit addr, code addr
		case 0x31: acall(op);                           break; //ACALL code addr
		case 0x32: reti(op);                            break; //RETI
		case 0x33: rlc_a(op);                           break; //RLC A
		case 0x34: addc_a_byte(op);                     break; //ADDC A, #data
		case 0x35: addc_a_mem(op);                      break; //ADDC A, data addr

		case 0x36:
		case 0x37: addc_a_ir(op & 1);                   break; //ADDC A, @R0/@R1

		case 0x38:
		case 0x39:
		case 0x3a:
		case 0x3b:
		case 0x3c:
		case 0x3d:
		case 0x3e:
		case 0x3f: addc_a_r(op & 7);                    break; //ADDC A, R0 to R7

		case 0x40: jc(op);                              break; //JC code addr
		case 0x41: ajmp(op);                            break; //AJMP code addr
		case 0x42: m_rwm = 1; orl_mem_a(op); m_rwm = 0;     break; //ORL data addr, A
		case 0x43: m_rwm = 1; orl_mem_byte(op); m_rwm = 0;  break; //ORL data addr, #data
		case 0x44: orl_a_byte(op);                      break;
		case 0x45: orl_a_mem(op);                       break; //ORL A, data addr

		case 0x46:
		case 0x47: orl_a_ir(op & 1);                    break; //ORL A, @RO/@R1

		case 0x48:
		case 0x49:
		case 0x4a:
		case 0x4b:
		case 0x4c:
		case 0x4d:
		case 0x4e:
		case 0x4f: orl_a_r(op & 7);                     break; //ORL A, RO to R7

		case 0x50: jnc(op);                             break; //JNC code addr
		case 0x51: acall(op);                           break; //ACALL code addr
		case 0x52: m_rwm = 1; anl_mem_a(op); m_rwm = 0;     break; //ANL data addr, A
		case 0x53: m_rwm = 1; anl_mem_byte(op); m_rwm = 0;  break; //ANL data addr, #data
		case 0x54: anl_a_byte(op);                      break; //ANL A, #data
		case 0x55: anl_a_mem(op);                       break; //ANL A, data addr

		case 0x56:
		case 0x57: anl_a_ir(op & 1);                    break; //ANL A, @RO/@R1

		case 0x58:
		case 0x59:
		case 0x5a:
		case 0x5b:
		case 0x5c:
		case 0x5d:
		case 0x5e:
		case 0x5f: anl_a_r(op & 7);                     break; //ANL A, RO to R7

		case 0x60: jz(op);                              break; //JZ code addr
		case 0x61: ajmp(op);                            break; //AJMP code addr
		case 0x62: m_rwm = 1; xrl_mem_a(op); m_rwm = 0;     break; //XRL data addr, A
		case 0x63: m_rwm = 1; xrl_mem_byte(op); m_rwm = 0;  break; //XRL data addr, #data
		case 0x64: xrl_a_byte(op);                      break; //XRL A, #data
		case 0x65: xrl_a_mem(op);                       break; //XRL A, data addr

		case 0x66:
		case 0x67: xrl_a_ir(op & 1);                    break; //XRL A, @R0/@R1

		case 0x68:
		case 0x69:
		case 0x6a:
		case 0x6b:
		case 0x6c:
		case 0x6d:
		case 0x6e:
		case 0x6f: xrl_a_r(op & 7);                     break; //XRL A, R0 to R7

		case 0x70: jnz(op);                             break; //JNZ code addr
		case 0x71: acall(op);                           break; //ACALL code addr
		case 0x72: orl_c_bitaddr(op);                   break; //ORL C, bit addr
		case 0x73: jmp_iadptr(op);                      break; //JMP @A+DPTR
		case 0x74: mov_a_byte(op);                      break; //MOV A, #data
		case 0x75: mov_mem_byte(op);                    break; //MOV data addr, #data

		case 0x76:
		case 0x77: mov_ir_byte(op & 1);                 break; //MOV @R0/@R1, #data

		case 0x78:
		case 0x79:
		case 0x7a:
		case 0x7b:
		case 0x7c:
		case 0x7d:
		case 0x7e:
		case 0x7f: mov_r_byte(op & 7);                  break; //MOV R0 to R7, #data

		case 0x80: sjmp(op);                            break; //SJMP code addr
		case 0x81: ajmp(op);                            break; //AJMP code addr
		case 0x82: anl_c_bitaddr(op);                   break; //ANL C, bit addr
		case 0x83: movc_a_iapc(op);                     break; //MOVC A, @A + m_pc
		case 0x84: div_ab(op);                          break; //DIV AB
		case 0x85: mov_mem_mem(op);                     break; //MOV data addr, data addr

		case 0x86:
		case 0x87: mov_mem_ir(op & 1);                  break; //MOV data addr, @R0/@R1

		case 0x88:
		case 0x89:
		case 0x8a:
		case 0x8b:
		case 0x8c:
		case 0x8d:
		case 0x8e:
		case 0x8f: mov_mem_r(op & 7);                   break; //MOV data addr,R0 to R7

		case 0x90: mov_dptr_byte(op);                   break; //MOV DPTR, #data
		case 0x91: acall(op);                           break; //ACALL code addr
		case 0x92: m_rwm = 1; mov_bitaddr_c(op); m_rwm = 0; break; //MOV bit addr, C
		case 0x93: movc_a_iadptr(op);                   break; //MOVC A, @A + DPTR
		case 0x94: subb_a_byte(op);                     break; //SUBB A, #data
		case 0x95: subb_a_mem(op);                      break; //SUBB A, data addr

		case 0x96:
		case 0x97: subb_a_ir(op & 1);                   break; //SUBB A, @R0/@R1

		case 0x98:
		case 0x99:
		case 0x9a:
		case 0x9b:
		case 0x9c:
		case 0x9d:
		case 0x9e:
		case 0x9f: subb_a_r(op & 7);                    break; //SUBB A, R0 to R7

		case 0xa0: orl_c_nbitaddr(op);                  break; //ORL C, /bit addr
		case 0xa1: ajmp(op);                            break; //AJMP code addr
		case 0xa2: mov_c_bitaddr(op);                   break; //MOV C, bit addr
		case 0xa3: inc_dptr(op);                        break; //INC DPTR
		case 0xa4: mul_ab(op);                          break; //MUL AB
		case 0xa5: illegal(op);                         break; //reserved

		case 0xa6:
		case 0xa7: mov_ir_mem(op & 1);                  break; //MOV @R0/@R1, data addr

		case 0xa8:
		case 0xa9:
		case 0xaa:
		case 0xab:
		case 0xac:
		case 0xad:
		case 0xae:
		case 0xaf: mov_r_mem(op & 7);                   break; //MOV R0 to R7, data addr

		case 0xb0: anl_c_nbitaddr(op);                  break; //ANL C,/bit addr
		case 0xb1: acall(op);                           break; //ACALL code addr
		case 0xb2: m_rwm = 1; cpl_bitaddr(op); m_rwm = 0;   break; //CPL bit addr
		case 0xb3: cpl_c(op);                           break; //CPL C
		case 0xb4: cjne_a_byte(op);                     break; //CJNE A, #data, code addr
		case 0xb5: cjne_a_mem(op);                      break; //CJNE A, data addr, code addr

		case 0xb6:
		case 0xb7: cjne_ir_byte(op & 1);                break; //CJNE @R0/@R1, #data, code addr

		case 0xb8:
		case 0xb9:
		case 0xba:
		case 0xbb:
		case 0xbc:
		case 0xbd:
		case 0xbe:
		case 0xbf: cjne_r_byte(op & 7);                 break; //CJNE R0 to R7, #data, code addr

		case 0xc0: push(op);                            break; //PUSH data addr
		case 0xc1: ajmp(op);                            break; //AJMP code addr
		case 0xc2: m_rwm = 1; clr_bitaddr(op); m_rwm = 0;   break; //CLR bit addr
		case 0xc3: clr_c(op);                           break; //CLR C
		case 0xc4: swap_a(op);                          break; //SWAP A
		case 0xc5: xch_a_mem(op);                       break; //XCH A, data addr

		case 0xc6:
		case 0xc7: xch_a_ir(op & 1);                    break; //XCH A, @RO/@R1

		case 0xc8:
		case 0xc9:
		case 0xca:
		case 0xcb:
		case 0xcc:
		case 0xcd:
		case 0xce:
		case 0xcf: xch_a_r(op & 7);                     break; //XCH A, RO to R7

		case 0xd0: pop(op);                             break; //POP data addr
		case 0xd1: acall(op);                           break; //ACALL code addr
		case 0xd2: m_rwm = 1; setb_bitaddr(op); m_rwm = 0;  break; //SETB bit addr
		case 0xd3: setb_c(op);                          break; //SETB C
		case 0xd4: da_a(op);                            break; //DA A
		case 0xd5: m_rwm = 1; djnz_mem(op); m_rwm = 0;      break; //DJNZ data addr, code addr

		case 0xd6:
		case 0xd7: xchd_a_ir(op & 1);                   break; //XCHD A, @R0/@R1

		case 0xd8:
		case 0xd9:
		case 0xda:
		case 0xdb:
		case 0xdc:
		case 0xdd:
		case 0xde:
		case 0xdf: djnz_r(op & 7);                      break; //DJNZ R0 to R7,code addr

		case 0xe0: movx_a_idptr(op);                    break; //MOVX A,@DPTR
		case 0xe1: ajmp(op);                            break; //AJMP code addr

		case 0xe2:
		case 0xe3: movx_a_ir(op & 1);                   break; //MOVX A, @R0/@R1

		case 0xe4: clr_a(op);                           break; //CLR A
		case 0xe5: mov_a_mem(op);                       break; //MOV A, data addr
		case 0xe6:
		case 0xe7: mov_a_ir(op & 1);                    break; //MOV A,@RO/@R1

		case 0xe8:
		case 0xe9:
		case 0xea:
		case 0xeb:
		case 0xec:
		case 0xed:
		case 0xee:
		case 0xef: mov_a_r(op & 7);                     break; //MOV A,R0 to R7

		case 0xf0: movx_idptr_a(op);                    break; //MOVX @DPTR,A
		case 0xf1: acall(op);                           break; //ACALL code addr

		case 0xf2:
		case 0xf3: movx_ir_a(op & 1);                   break; //MOVX @R0/@R1,A

		case 0xf4: cpl_a(op);                           break; //CPL A
		case 0xf5: mov_mem_a(op);                       break; //MOV data addr, A

		case 0xf6:
		case 0xf7: mov_ir_a(op & 1);                    break; //MOV @R0/@R1, A

		case 0xf8:
		case 0xf9:
		case 0xfa:
		case 0xfb:
		case 0xfc:
		case 0xfd:
		case 0xfe:
		case 0xff: mov_r_a(op & 7);                     break; //MOV R0 to R7, A
		default:
			illegal(op);
	}
}
