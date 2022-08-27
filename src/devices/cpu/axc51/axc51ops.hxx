// license:BSD-3-Clause
// copyright-holders:Steve Ellenoff
/*******************************************************************************************
 NOTE: All registers are accessed directly, instead of using the SFR_R() function for speed
 Direct register access is availabe from the R_(register name) macros.. ex: ACC for the ACC
 with the exception of the m_pc
********************************************************************************************/

//ACALL code addr                           /* 1: aaa1 0001 */
void axc51base_cpu_device::acall(uint8_t r)
{
	uint8_t addr = m_program.read_byte(m_pc++);             //Grab code address byte
	push_pc();                              //Save m_pc to the stack
	//Thanks Gerrit for help with this! :)
	m_pc = (m_pc & 0xf800) | ((r & 0xe0) << 3) | addr;
}

//ADD A, #data                              /* 1: 0010 0100 */
void axc51base_cpu_device::add_a_byte(uint8_t r)
{
	uint8_t data = m_program.read_byte(m_pc++);             //Grab data
	uint8_t result = ACC + data;          //Add data to accumulator
	do_add_flags(ACC,data,0);               //Set Flags
	SET_ACC(result);                        //Store 8 bit result of addtion in ACC
}

//ADD A, data addr                          /* 1: 0010 0101 */
void axc51base_cpu_device::add_a_mem(uint8_t r)
{
	uint8_t addr = m_program.read_byte(m_pc++);             //Grab data address
	uint8_t data = iram_read(addr);              //Grab data from data address
	uint8_t result = ACC + data;          //Add data to accumulator
	do_add_flags(ACC,data,0);               //Set Flags
	SET_ACC(result);                        //Store 8 bit result of addtion in ACC
}

//ADD A, @R0/@R1                            /* 1: 0010 011i */
void axc51base_cpu_device::add_a_ir(uint8_t r)
{
	uint8_t data = iram_indirect_read(R_REG(r));         //Grab data from memory pointed to by R0 or R1
	uint8_t result = ACC + data;          //Add data to accumulator
	do_add_flags(ACC,data,0);               //Set Flags
	SET_ACC(result);                        //Store 8 bit result of addtion in ACC
}

//ADD A, R0 to R7                           /* 1: 0010 1rrr */
void axc51base_cpu_device::add_a_r(uint8_t r)
{
	uint8_t data = R_REG(r);                  //Grab data from R0 - R7
	uint8_t result = ACC + data;          //Add data to accumulator
	do_add_flags(ACC,data,0);               //Set Flags
	SET_ACC(result);                        //Store 8 bit result of addtion in ACC
}

//ADDC A, #data                             /* 1: 0011 0100 */
void axc51base_cpu_device::addc_a_byte(uint8_t r)
{
	uint8_t data = m_program.read_byte(m_pc++);             //Grab data
	uint8_t result = ACC + data + GET_CY; //Add data + carry flag to accumulator
	do_add_flags(ACC,data,GET_CY);      //Set Flags
	SET_ACC(result);                        //Store 8 bit result of addtion in ACC
}

//ADDC A, data addr                         /* 1: 0011 0101 */
void axc51base_cpu_device::addc_a_mem(uint8_t r)
{
	uint8_t addr = m_program.read_byte(m_pc++);             //Grab data address
	uint8_t data = iram_read(addr);              //Grab data from data address
	uint8_t result = ACC + data + GET_CY; //Add data + carry flag to accumulator
	do_add_flags(ACC,data,GET_CY);      //Set Flags
	SET_ACC(result);                        //Store 8 bit result of addtion in ACC
}

//ADDC A, @R0/@R1                           /* 1: 0011 011i */
void axc51base_cpu_device::addc_a_ir(uint8_t r)
{
	uint8_t data = iram_indirect_read(R_REG(r));         //Grab data from memory pointed to by R0 or R1
	uint8_t result = ACC + data + GET_CY; //Add data + carry flag to accumulator
	do_add_flags(ACC,data,GET_CY);      //Set Flags
	SET_ACC(result);                        //Store 8 bit result of addtion in ACC
}

//ADDC A, R0 to R7                          /* 1: 0011 1rrr */
void axc51base_cpu_device::addc_a_r(uint8_t r)
{
	uint8_t data = R_REG(r);                  //Grab data from R0 - R7
	uint8_t result = ACC + data + GET_CY; //Add data + carry flag to accumulator
	do_add_flags(ACC,data,GET_CY);      //Set Flags
	SET_ACC(result);                        //Store 8 bit result of addtion in ACC
}

//AJMP code addr                            /* 1: aaa0 0001 */
void axc51base_cpu_device::ajmp(uint8_t r)
{
	uint8_t addr = m_program.read_byte(m_pc++);             //Grab code address byte
	//Thanks Gerrit for help with this! :)
	m_pc = (m_pc & 0xf800) | ((r & 0xe0) << 3) | addr;
}

//ANL data addr, A                          /* 1: 0101 0010 */
void axc51base_cpu_device::anl_mem_a(uint8_t r)
{
	uint8_t addr = m_program.read_byte(m_pc++);             //Grab data address
	uint8_t data = iram_read(addr);              //Grab data from data address
	iram_write(addr,data & ACC);                //Set data address value to it's value Logical AND with ACC
}

//ANL data addr, #data                      /* 1: 0101 0011 */
void axc51base_cpu_device::anl_mem_byte(uint8_t r)
{
	uint8_t addr = m_program.read_byte(m_pc++);             //Grab data address
	uint8_t data = m_program.read_byte(m_pc++);             //Grab data
	uint8_t srcdata = iram_read(addr);           //Grab data from data address
	iram_write(addr,srcdata & data);            //Set data address value to it's value Logical AND with Data
}

//ANL A, #data                              /* 1: 0101 0100 */
void axc51base_cpu_device::anl_a_byte(uint8_t r)
{
	uint8_t data = m_program.read_byte(m_pc++);             //Grab data
	SET_ACC(ACC & data);                //Set ACC to value of ACC Logical AND with Data
}

//ANL A, data addr                          /* 1: 0101 0101 */
void axc51base_cpu_device::anl_a_mem(uint8_t r)
{
	uint8_t addr = m_program.read_byte(m_pc++);             //Grab data address
	uint8_t data = iram_read(addr);              //Grab data from data address
	SET_ACC(ACC & data);                //Set ACC to value of ACC Logical AND with Data
}

//ANL A, @RO/@R1                            /* 1: 0101 011i */
void axc51base_cpu_device::anl_a_ir(uint8_t r)
{
	uint8_t data = iram_indirect_read(R_REG(r));         //Grab data from address R0 or R1 points to
	SET_ACC(ACC & data);                //Set ACC to value of ACC Logical AND with Data
}

//ANL A, RO to R7                           /* 1: 0101 1rrr */
void axc51base_cpu_device::anl_a_r(uint8_t r)
{
	uint8_t data = R_REG(r);                  //Grab data from R0 - R7
	SET_ACC(ACC & data);                //Set ACC to value of ACC Logical AND with Data
}

//ANL C, bit addr                           /* 1: 1000 0010 */
void axc51base_cpu_device::anl_c_bitaddr(uint8_t r)
{
	int cy = GET_CY;
	uint8_t addr = m_program.read_byte(m_pc++);             //Grab bit address
	uint8_t bit = bit_address_r(addr);                //Grab bit data from bit address
	SET_CY( (cy & bit) );                   //Set Carry flag to Carry Flag Value Logical AND with Bit
}

//ANL C,/bit addr                           /* 1: 1011 0000 */
void axc51base_cpu_device::anl_c_nbitaddr(uint8_t r)
{
	int cy = GET_CY;
	uint8_t addr = m_program.read_byte(m_pc++);             //Grab bit address
	uint8_t bit = bit_address_r(addr);                //Grab bit data from bit address
	bit = ((~bit)&1);                       //Complement bit
	SET_CY( (cy & bit) );                   //Set Carry flag to Carry Flag Value Logical AND with Complemented Bit
}

//CJNE A, #data, code addr                  /* 1: 1011 0100 */
void axc51base_cpu_device::cjne_a_byte(uint8_t r)
{
	uint8_t data = m_program.read_byte(m_pc++);             //Grab data
	int8_t rel_addr = m_program.read_byte(m_pc++);          //Grab relative code address

	if(ACC != data)                     //Jump if values are not equal
	{
		m_pc = m_pc + rel_addr;
	}

	//Set carry flag to 1 if 1st compare value is < 2nd compare value
	SET_CY( (ACC < data) );
}

//CJNE A, data addr, code addr              /* 1: 1011 0101 */
void axc51base_cpu_device::cjne_a_mem(uint8_t r)
{
	uint8_t addr = m_program.read_byte(m_pc++);             //Grab data address
	int8_t rel_addr = m_program.read_byte(m_pc++);          //Grab relative code address
	uint8_t data = iram_read(addr);              //Pull value from data address

	if(ACC != data)                     //Jump if values are not equal
	{
		m_pc = m_pc + rel_addr;
	}

	//Set carry flag to 1 if 1st compare value is < 2nd compare value
	SET_CY( (ACC < data) );
}

//CJNE @R0/@R1, #data, code addr            /* 1: 1011 011i */
void axc51base_cpu_device::cjne_ir_byte(uint8_t r)
{
	uint8_t data = m_program.read_byte(m_pc++);             //Grab data
	int8_t rel_addr = m_program.read_byte(m_pc++);          //Grab relative code address
	uint8_t srcdata = iram_indirect_read(R_REG(r));      //Grab value pointed to by R0 or R1

	if(srcdata != data)                     //Jump if values are not equal
	{
		m_pc = m_pc + rel_addr;
	}

	//Set carry flag to 1 if 1st compare value is < 2nd compare value
	SET_CY( (srcdata < data) );
}

//CJNE R0 to R7, #data, code addr           /* 1: 1011 1rrr */
void axc51base_cpu_device::cjne_r_byte(uint8_t r)
{
	uint8_t data = m_program.read_byte(m_pc++);             //Grab data
	int8_t rel_addr = m_program.read_byte(m_pc++);          //Grab relative code address
	uint8_t srcdata = R_REG(r);                   //Grab value of R0 - R7

	if(srcdata != data)                     //Jump if values are not equal
	{
		m_pc = m_pc + rel_addr;
	}

	//Set carry flag to 1 if 1st compare value is < 2nd compare value
	SET_CY( (srcdata < data) );
}

//CLR bit addr                              /* 1: 1100 0010 */
void axc51base_cpu_device::clr_bitaddr(uint8_t r)
{
	uint8_t addr = m_program.read_byte(m_pc++);             //Grab bit address
	bit_address_w(addr,0);                          //Clear bit at specified bit address
}

//CLR C                                     /* 1: 1100 0011 */
void axc51base_cpu_device::clr_c(uint8_t r)
{
	SET_CY(0);                              //Clear Carry Flag
}

//CLR A                                     /* 1: 1110 0100 */
void axc51base_cpu_device::clr_a(uint8_t r)
{
	SET_ACC(0);                         //Clear Accumulator
}

//CPL bit addr                              /* 1: 1011 0010 */
void axc51base_cpu_device::cpl_bitaddr(uint8_t r)
{
	uint8_t addr = m_program.read_byte(m_pc++);             //Grab bit address
	uint8_t data = (~bit_address_r(addr))&1;
	bit_address_w(addr,data);                       //Complement bit at specified bit address
}

//CPL C                                     /* 1: 1011 0011 */
void axc51base_cpu_device::cpl_c(uint8_t r)
{
	uint8_t bit = (~GET_CY)&1;                //Complement Carry Flag
	SET_CY(bit);
}

//CPL A                                     /* 1: 1111 0100 */
void axc51base_cpu_device::cpl_a(uint8_t r)
{
	uint8_t data = ((~ACC)&0xff);
	SET_ACC(data);                      //Complement Accumulator
}

//DA A                                      /* 1: 1101 0100 */
void axc51base_cpu_device::da_a(uint8_t r)
{
/*From several sources, since none said the same thing:
 The decimal adjust instruction is associated with the use of the ADD and ADDC instructions.
 The eight-bit value in the accumulator is adjusted to form two BCD digits of four bits each.
 If the accumulator contents bits 0-3 are greater than 9, OR the AC flag is set, then six is added to
 produce a proper BCD digit.
 If the carry is set, OR the four high bits 4-7 exceed nine, six is added to the value of these bits.
 The carry flag will be set if the result is > 0x99, but not cleared otherwise */

	uint16_t new_acc = ACC & 0xff;
	if(GET_AC || (new_acc & 0x0f) > 0x09)
		new_acc += 0x06;
	if(GET_CY || ((new_acc & 0xf0) > 0x90) || (new_acc & ~0xff))
		new_acc += 0x60;
	SET_ACC(new_acc&0xff);
	if(new_acc & ~0xff)
	SET_CY(1);
}

//DEC A                                     /* 1: 0001 0100 */
void axc51base_cpu_device::dec_a(uint8_t r)
{
	SET_ACC(ACC-1);
}

//DEC data addr                             /* 1: 0001 0101 */
void axc51base_cpu_device::dec_mem(uint8_t r)
{
	uint8_t addr = m_program.read_byte(m_pc++);             //Grab data address
	uint8_t data = iram_read(addr);
	iram_write(addr,data-1);
}

//DEC @R0/@R1                               /* 1: 0001 011i */
void axc51base_cpu_device::dec_ir(uint8_t r)
{
	uint8_t data = iram_indirect_read(R_REG(r));
	iram_indirect_write(R_REG(r),data-1);
}

//DEC R0 to R7                              /* 1: 0001 1rrr */
void axc51base_cpu_device::dec_r(uint8_t r)
{
	SET_REG(r, R_REG(r) - 1);
}

//DIV AB                                    /* 1: 1000 0100 */
void axc51base_cpu_device::div_ab(uint8_t r)
{
	if( B == 0 ) {
		//Overflow flag is set!
		SET_OV(1);
		//Really the values are undefined according to the manual, but we'll just leave them as is..
		//SET_ACC(0xff);
		//SFR_W(B,0xff);
	}
	else {
		int a = (int)ACC / B;
		int b = (int)ACC % B;
		//A gets quotient byte, B gets remainder byte
		SET_ACC(a);
		B = b;
		//Overflow flag is cleared
		SET_OV(0);
	}
	//Carry Flag is always cleared
	SET_CY(0);
}

//DJNZ data addr, code addr                 /* 1: 1101 0101 */
void axc51base_cpu_device::djnz_mem(uint8_t r)
{
	uint8_t addr = m_program.read_byte(m_pc++);             //Grab data address
	int8_t rel_addr = m_program.read_byte(m_pc++);          //Grab relative code address
	iram_write(addr,iram_read(addr) - 1);          //Decrement value contained at data address
	if(iram_read(addr) != 0)                   //Branch if contents of data address is not 0
	{
		m_pc = m_pc + rel_addr;
	}
}

//DJNZ R0 to R7,code addr                   /* 1: 1101 1rrr */
void axc51base_cpu_device::djnz_r(uint8_t r)
{
	int8_t rel_addr = m_program.read_byte(m_pc++);          //Grab relative code address
	SET_REG(r ,R_REG(r) - 1);                   //Decrement value
	if(R_REG(r) != 0)                           //Branch if contents of R0 - R7 is not 0
	{
		m_pc = m_pc + rel_addr;
	}
}

//INC A                                     /* 1: 0000 0100 */
void axc51base_cpu_device::inc_a(uint8_t r)
{
	SET_ACC(ACC+1);
}

//INC data addr                             /* 1: 0000 0101 */
void axc51base_cpu_device::inc_mem(uint8_t r)
{
	uint8_t addr = m_program.read_byte(m_pc++);             //Grab data address
	uint8_t data = iram_read(addr);
	iram_write(addr,data+1);
}

//INC @R0/@R1                               /* 1: 0000 011i */
void axc51base_cpu_device::inc_ir(uint8_t r)
{
	uint8_t data = iram_indirect_read(R_REG(r));
	iram_indirect_write(R_REG(r),data+1);
}

//INC R0 to R7                              /* 1: 0000 1rrr */
void axc51base_cpu_device::inc_r(uint8_t r)
{
	uint8_t data = R_REG(r);
	SET_REG(r, data + 1);
}

//INC DPTR                                  /* 1: 1010 0011 */
void axc51base_cpu_device::inc_dptr(uint8_t r)
{
	//if (m_sfr_regs[SFR_DPCON] & 0x08) // auto-increment enabled (not used here)
	//{
	//	fatalerror("inc_dptr with auto-inc");
	//}

	if (m_sfr_regs[SFR_DPCON] & 0x04) // auto-toggle enabled
	{
		fatalerror("inc_dptr with auto-toggle");
	}

	if (m_sfr_regs[SFR_DPCON] & 0x01) // DPTR1 enabled
	{
		uint16_t dptr = (DPTR1)+1;
		SET_DPTR1(dptr);
	}
	else
	{
		uint16_t dptr = (DPTR0)+1;
		SET_DPTR0(dptr);
	}

}

//JB  bit addr, code addr                   /* 1: 0010 0000 */
void axc51base_cpu_device::jb(uint8_t r)
{
	uint8_t addr = m_program.read_byte(m_pc++);             //Grab bit address
	int8_t rel_addr = m_program.read_byte(m_pc++);          //Grab relative code address
	if(bit_address_r(addr))                         //If bit set at specified bit address, jump
	{
		m_pc = m_pc + rel_addr;
	}
}

//JBC bit addr, code addr                   /* 1: 0001 0000 */
void axc51base_cpu_device::jbc(uint8_t r)
{
	uint8_t addr = m_program.read_byte(m_pc++);             //Grab bit address
	int8_t rel_addr = m_program.read_byte(m_pc++);          //Grab relative code address
	if(bit_address_r(addr)) {                       //If bit set at specified bit address, jump
		m_pc = m_pc + rel_addr;
		bit_address_w(addr,0);                      //Clear Bit also
	}
}

//JC code addr                              /* 1: 0100 0000 */
void axc51base_cpu_device::jc(uint8_t r)
{
	int8_t rel_addr = m_program.read_byte(m_pc++);          //Grab relative code address
	if(GET_CY)                              //Jump if Carry Flag Set
	{
		m_pc = m_pc + rel_addr;
	}
}

//JMP @A+DPTR                               /* 1: 0111 0011 */
void axc51base_cpu_device::jmp_iadptr(uint8_t r)
{
	// not listed as being affected by auto-inc or auto-toggle?
	if (m_sfr_regs[SFR_DPCON] & 0x08) // auto-increment enabled
	{
		fatalerror("jmp_iadptr with auto-inc");
	}

	if (m_sfr_regs[SFR_DPCON] & 0x04) // auto-toggle enabled
	{
		fatalerror("jmp_iadptr with auto-toggle");
	}

	if (m_sfr_regs[SFR_DPCON] & 0x01) // DPTR0 enabled
	{
		fatalerror("jmp_iadptr with DPTR1");
	}

	m_pc = ACC + DPTR0;
}

//JNB bit addr, code addr                   /* 1: 0011 0000 */
void axc51base_cpu_device::jnb(uint8_t r)
{
	uint8_t addr = m_program.read_byte(m_pc++);             //Grab bit address
	int8_t rel_addr = m_program.read_byte(m_pc++);          //Grab relative code address
	if(!bit_address_r(addr))                        //If bit NOT set at specified bit address, jump
	{
		m_pc = m_pc + rel_addr;
	}
}

//JNC code addr                             /* 1: 0101 0000 */
void axc51base_cpu_device::jnc(uint8_t r)
{
	int8_t rel_addr = m_program.read_byte(m_pc++);          //Grab relative code address
	if(!GET_CY)                             //Jump if Carry Flag not set
	{
		m_pc = m_pc + rel_addr;
	}
}

//JNZ code addr                             /* 1: 0111 0000 */
void axc51base_cpu_device::jnz(uint8_t r)
{
	int8_t rel_addr = m_program.read_byte(m_pc++);          //Grab relative code address
	if(ACC != 0)                            //Branch if ACC is not 0
	{
		m_pc = m_pc+rel_addr;
	}
}

//JZ code addr                              /* 1: 0110 0000 */
void axc51base_cpu_device::jz(uint8_t r)
{
	int8_t rel_addr = m_program.read_byte(m_pc++);          //Grab relative code address
	if(ACC == 0)                            //Branch if ACC is 0
	{
		m_pc = m_pc+rel_addr;
	}
}

//LCALL code addr                           /* 1: 0001 0010 */
void axc51base_cpu_device::lcall(uint8_t r)
{
	uint8_t addr_hi, addr_lo;
	addr_hi = m_program.read_byte(m_pc++);
	addr_lo = m_program.read_byte(m_pc++);
	push_pc();
	m_pc = (uint16_t)((addr_hi<<8) | addr_lo);
}

//LJMP code addr                            /* 1: 0000 0010 */
void axc51base_cpu_device::ljmp(uint8_t r)
{
	uint8_t addr_hi, addr_lo;
	addr_hi = m_program.read_byte(m_pc++);
	addr_lo = m_program.read_byte(m_pc++);
	m_pc = (uint16_t)((addr_hi<<8) | addr_lo);
}

//MOV A, #data                              /* 1: 0111 0100 */
void axc51base_cpu_device::mov_a_byte(uint8_t r)
{
	uint8_t data = m_program.read_byte(m_pc++);             //Grab data
	SET_ACC(data);                      //Store data to ACC
}

//MOV A, data addr                          /* 1: 1110 0101 */
void axc51base_cpu_device::mov_a_mem(uint8_t r)
{
	uint8_t addr = m_program.read_byte(m_pc++);             //Grab data address
	SET_ACC(iram_read(addr));              //Store contents of data address to ACC
}

//MOV A,@RO/@R1                             /* 1: 1110 011i */
void axc51base_cpu_device::mov_a_ir(uint8_t r)
{
	SET_ACC(iram_indirect_read(R_REG(r)));             //Store contents of address pointed by R0 or R1 to ACC
}

//MOV A,R0 to R7                            /* 1: 1110 1rrr */
void axc51base_cpu_device::mov_a_r(uint8_t r)
{
	SET_ACC(R_REG(r));                      //Store contents of R0 - R7 to ACC
}

//MOV data addr, #data                      /* 1: 0111 0101 */
void axc51base_cpu_device::mov_mem_byte(uint8_t r)
{
	uint8_t addr = m_program.read_byte(m_pc++);             //Grab data address
	uint8_t data = m_program.read_byte(m_pc++);             //Grab data
	iram_write(addr,data);                      //Store data to data address location
}

//MOV data addr, data addr                  /* 1: 1000 0101 */
void axc51base_cpu_device::mov_mem_mem(uint8_t r)
{
	//1st address is src, 2nd is dst, but the mov command works as mov dst,src)
	uint8_t src,dst;
	src = m_program.read_byte(m_pc++);                    //Grab source data address
	dst = m_program.read_byte(m_pc++);                    //Grab destination data address
	iram_write(dst,iram_read(src));                //Read source address contents and store to destination address
}

//MOV @R0/@R1, #data                        /* 1: 0111 011i */
void axc51base_cpu_device::mov_ir_byte(uint8_t r)
{
	uint8_t data = m_program.read_byte(m_pc++);             //Grab data
	iram_indirect_write(R_REG(r),data);                 //Store data to address pointed by R0 or R1
}

//MOV R0 to R7, #data                       /* 1: 0111 1rrr */
void axc51base_cpu_device::mov_r_byte(uint8_t r)
{
	uint8_t data = m_program.read_byte(m_pc++);             //Grab data
	SET_REG(r, data);                           //Store to R0 - R7
}

//MOV data addr, @R0/@R1                    /* 1: 1000 011i */
void axc51base_cpu_device::mov_mem_ir(uint8_t r)
{
	uint8_t addr = m_program.read_byte(m_pc++);             //Grab data address
	iram_write(addr,iram_indirect_read(R_REG(r)));         //Store contents pointed to by R0 or R1 to data address
}

//MOV data addr,R0 to R7                    /* 1: 1000 1rrr */
void axc51base_cpu_device::mov_mem_r(uint8_t r)
{
	uint8_t addr = m_program.read_byte(m_pc++);             //Grab data address
	iram_write(addr,R_REG(r));                  //Store contents of R0 - R7 to data address
}

//MOV DPTR, #data16                         /* 1: 1001 0000 */
void axc51base_cpu_device::mov_dptr_byte(uint8_t r)
{
	//if (m_sfr_regs[SFR_DPCON] & 0x08) // auto-increment enabled (not used here)
	//{
	//	fatalerror("mov_dptr_byte with auto-inc");
	//}

	if (m_sfr_regs[SFR_DPCON] & 0x04) // auto-toggle enabled
	{
		fatalerror("mov_dptr_byte with auto-toggle");
	}

	uint8_t data_hi, data_lo;
	data_hi = m_program.read_byte(m_pc++);                //Grab hi byte
	data_lo = m_program.read_byte(m_pc++);                //Grab lo byte

	if (m_sfr_regs[SFR_DPCON] & 0x01) // DPTR1 enabled
	{
		SET_DPTR1((uint16_t)((data_hi << 8) | data_lo));   //Store to DPTR1
	}
	else
	{
		SET_DPTR0((uint16_t)((data_hi << 8) | data_lo));   //Store to DPTR0
	}
}

//MOV bit addr, C                           /* 1: 1001 0010 */
void axc51base_cpu_device::mov_bitaddr_c(uint8_t r)
{
	uint8_t addr = m_program.read_byte(m_pc++);             //Grab bit address
	bit_address_w(addr,GET_CY);                     //Store Carry Flag to Bit Address
}

//MOV @R0/@R1, data addr                    /* 1: 1010 011i */
void axc51base_cpu_device::mov_ir_mem(uint8_t r)
{
	uint8_t addr = m_program.read_byte(m_pc++);             //Grab data address
	iram_indirect_write(R_REG(r),iram_read(addr));         //Store data from data address to address pointed to by R0 or R1
}

//MOV R0 to R7, data addr                   /* 1: 1010 1rrr */
void axc51base_cpu_device::mov_r_mem(uint8_t r)
{
	uint8_t addr = m_program.read_byte(m_pc++);             //Grab data address
	SET_REG(r, iram_read(addr));               //Store to R0 - R7
}

//MOV data addr, A                          /* 1: 1111 0101 */
void axc51base_cpu_device::mov_mem_a(uint8_t r)
{
	uint8_t addr = m_program.read_byte(m_pc++);             //Grab data address
	iram_write(addr,ACC);                       //Store A to data address
}

//MOV @R0/@R1, A                            /* 1: 1111 011i */
void axc51base_cpu_device::mov_ir_a(uint8_t r)
{
	iram_indirect_write(R_REG(r),ACC);                  //Store A to location pointed to by R0 or R1
}

//MOV R0 to R7, A                           /* 1: 1111 1rrr */
void axc51base_cpu_device::mov_r_a(uint8_t r)
{
	SET_REG(r, ACC);                        //Store A to R0-R7
}

//MOVC A, @A + m_pc                           /* 1: 1000 0011 */
void axc51base_cpu_device::movc_a_iapc(uint8_t r)
{
	uint8_t data;
	data = (uint8_t)m_program.read_byte(ACC+m_pc);               //Move a byte from CODE(Program) Memory and store to ACC
	SET_ACC(data);
}

//MOV C, bit addr                           /* 1: 1010 0010 */
void axc51base_cpu_device::mov_c_bitaddr(uint8_t r)
{
	uint8_t addr = m_program.read_byte(m_pc++);             //Grab bit address
	SET_CY( (bit_address_r(addr)) );                //Store Bit from Bit Address to Carry Flag
}

//MOVC A, @A + DPTR                         /* 1: 1001 0011 */
void axc51base_cpu_device::movc_a_iadptr(uint8_t r)
{
	if (m_sfr_regs[SFR_DPCON] & 0x08) // auto-increment enabled
	{
		fatalerror("movc_a_iadptr with auto-inc");
	}

	if (m_sfr_regs[SFR_DPCON] & 0x04) // auto-toggle enabled
	{
		fatalerror("movc_a_iadptr with auto-toggle");
	}

	if (m_sfr_regs[SFR_DPCON] & 0x01) // DPTR1 enabled
	{
		fatalerror("movc_a_iadptr with DPTR1");
	}

	uint8_t data;
	data = (uint8_t)m_program.read_byte(ACC + DPTR0);           //Move a byte from CODE(Program) Memory and store to ACC
	SET_ACC(data);
}

//MOVX A,@DPTR                              /* 1: 1110 0000 */
//(Move External Ram 16 bit address to A)
void axc51base_cpu_device::movx_a_idptr(uint8_t r)
{
	uint32_t addr = process_dptr_access();

	uint8_t byte = (uint8_t)m_io.read_byte(addr);           //Grab 1 byte from External DATA memory pointed to by dptr

	SET_ACC(byte);                      //Store to ACC
}

//MOVX A, @R0/@R1                           /* 1: 1110 001i */
//(Move External Ram 8 bit address to A)
void axc51base_cpu_device::movx_a_ir(uint8_t r)
{
	uint32_t addr = external_ram_iaddr(R_REG(r),0xff); //Grab address by reading location pointed to by R0 or R1
	uint8_t byte = (uint8_t)m_io.read_byte(addr);           //Grab 1 byte from External DATA memory pointed to by address
	SET_ACC(byte);                      //Store to ACC
}

//MOVX @DPTR,A                              /* 1: 1111 0000 */
//(Move A to External Ram 16 bit address)
void axc51base_cpu_device::movx_idptr_a(uint8_t r)
{
	uint32_t addr = process_dptr_access();
	m_io.write_byte(addr, ACC);               //Store ACC to External DATA memory address pointed to by DPTR0
}

//MOVX @R0/@R1,A                            /* 1: 1111 001i */
//(Move A to External Ram 8 bit address)
void axc51base_cpu_device::movx_ir_a(uint8_t r)
{
	uint32_t addr = external_ram_iaddr(R_REG(r),0xff);   //Grab address by reading location pointed to by R0 or R1
	m_io.write_byte(addr, ACC);                   //Store ACC to External DATA memory address
}

//MUL AB                                    /* 1: 1010 0100 */
void axc51base_cpu_device::mul_ab(uint8_t r)
{
	uint16_t result = ACC * B;
	//A gets lo bits, B gets hi bits of result
	B = (uint8_t) ((result & 0xff00) >> 8);
	SET_ACC((uint8_t)(result & 0x00ff));
	//Set flags
	SET_OV( ((result & 0x100) >> 8) );      //Set/Clear Overflow Flag if result > 255
	SET_CY(0);                              //Carry Flag always cleared
}

//NOP                                       /* 1: 0000 0000 */
void axc51base_cpu_device::nop(uint8_t r)
{
}

//ORL data addr, A                          /* 1: 0100 0010 */
void axc51base_cpu_device::orl_mem_a(uint8_t r)
{
	uint8_t addr = m_program.read_byte(m_pc++);             //Grab data address
	uint8_t data = iram_read(addr);              //Grab data from data address
	iram_write(addr,data | ACC);                //Set data address value to it's value Logical OR with ACC
}

//ORL data addr, #data                      /* 1: 0100 0011 */
void axc51base_cpu_device::orl_mem_byte(uint8_t r)
{
	uint8_t addr = m_program.read_byte(m_pc++);             //Grab data address
	uint8_t data = m_program.read_byte(m_pc++);             //Grab data
	uint8_t srcdata = iram_read(addr);           //Grab data from data address
	iram_write(addr,srcdata | data);            //Set data address value to it's value Logical OR with Data
}

//ORL A, #data                              /* 1: 0100 0100 */
void axc51base_cpu_device::orl_a_byte(uint8_t r)
{
	uint8_t data = m_program.read_byte(m_pc++);             //Grab data
	SET_ACC(ACC | data);                //Set ACC to value of ACC Logical OR with Data
}

//ORL A, data addr                          /* 1: 0100 0101 */
void axc51base_cpu_device::orl_a_mem(uint8_t r)
{
	uint8_t addr = m_program.read_byte(m_pc++);             //Grab data address
	uint8_t data = iram_read(addr);              //Grab data from data address
	SET_ACC(ACC | data);                //Set ACC to value of ACC Logical OR with Data
}

//ORL A, @RO/@R1                            /* 1: 0100 011i */
void axc51base_cpu_device::orl_a_ir(uint8_t r)
{
	uint8_t data = iram_indirect_read(R_REG(r));         //Grab data from address R0 or R1 points to
	SET_ACC(ACC | data);                //Set ACC to value of ACC Logical OR with Data
}

//ORL A, RO to R7                           /* 1: 0100 1rrr */
void axc51base_cpu_device::orl_a_r(uint8_t r)
{
	uint8_t data = R_REG(r);                  //Grab data from R0 - R7
	SET_ACC(ACC | data);                //Set ACC to value of ACC Logical OR with Data
}

//ORL C, bit addr                           /* 1: 0111 0010 */
void axc51base_cpu_device::orl_c_bitaddr(uint8_t r)
{
	int cy = GET_CY;
	uint8_t addr = m_program.read_byte(m_pc++);             //Grab bit address
	uint8_t bit = bit_address_r(addr);                //Grab bit data from bit address
	SET_CY( (cy | bit) );                   //Set Carry flag to Carry Flag Value Logical OR with Bit
}

//ORL C, /bit addr                          /* 1: 1010 0000 */
void axc51base_cpu_device::orl_c_nbitaddr(uint8_t r)
{
	int cy = GET_CY;
	uint8_t addr = m_program.read_byte(m_pc++);             //Grab bit address
	uint8_t bit = bit_address_r(addr);                //Grab bit data from bit address
	bit = ((~bit)&1);                       //Complement bit
	SET_CY( (cy | bit) );                   //Set Carry flag to Carry Flag Value Logical OR with Complemented Bit
}

//POP data addr                             /* 1: 1101 0000 */
void axc51base_cpu_device::pop(uint8_t r)
{
	uint8_t addr = m_program.read_byte(m_pc++);             //Grab data address
	iram_write(addr, iram_indirect_read(SP));              //Store to contents of data addr, data pointed to by Stack - iram_indirect_read needed to access upper 128 bytes of stack
	//iram_indirect_write(addr, iram_indirect_read(R_SP));         //Store to contents of data addr, data pointed to by Stack - doesn't work, sfr's are not restored this way and it's not an indirect access anyway
	SP = SP-1;                              //Decrement SP
}

//PUSH data addr                            /* 1: 1100 0000 */
void axc51base_cpu_device::push(uint8_t r)
{
	uint8_t addr = m_program.read_byte(m_pc++);             //Grab data address
	uint8_t tmpSP = SP+1;                 //Grab and Increment Stack Pointer
	SP = tmpSP;                         // ""
	iram_indirect_write(tmpSP, iram_read(addr));           //Store to stack contents of data address - iram_indirect_write needed to store to upper 128 bytes of stack, however, can't use iram_indirect_read because that won't store the sfrs and it's not an indirect access anyway
}

//RET                                       /* 1: 0010 0010 */
void axc51base_cpu_device::ret(uint8_t r)
{
	pop_pc();
}

//RETI                                      /* 1: 0011 0010 */
void axc51base_cpu_device::reti(uint8_t r)
{
	pop_pc();
	clear_current_irq();
}

//RL A                                      /* 1: 0010 0011 */
void axc51base_cpu_device::rl_a(uint8_t r)
{
	//Left Shift A, Bit 7 carries to Bit 0
	int carry = ((ACC & 0x80) >> 7);
	int data = (ACC<<1) & 0xfe;
	SET_ACC( data | carry);
}

//RLC A                                     /* 1: 0011 0011 */
void axc51base_cpu_device::rlc_a(uint8_t r)
{
	//Left Shift A, Bit 7 goes to Carry Flag, Original Carry Flag goes to Bit 0 of ACC
	int carry = ((ACC & 0x80) >> 7);
	int data = ((ACC<<1) & 0xfe) | GET_CY;
	SET_ACC( data);
	SET_CY(carry);
}

//RR A                                      /* 1: 0000 0011 */
void axc51base_cpu_device::rr_a(uint8_t r)
{
	//Right Shift A, Bit 0 carries to Bit 7
	int carry = ((ACC & 1) << 7);
	int data = (ACC>>1) & 0x7f;
	SET_ACC( data | carry);
}

//RRC A                                     /* 1: 0001 0011 */
void axc51base_cpu_device::rrc_a(uint8_t r)
{
	//Right Shift A, Bit 0 goes to Carry Flag, Bit 7 of ACC gets set to original Carry Flag
	int carry = (ACC & 1);
	int data = ((ACC>>1) & 0x7f) | (GET_CY<<7);
	SET_ACC( data);
	SET_CY(carry);
}

//SETB C                                    /* 1: 1101 0011 */
void axc51base_cpu_device::setb_c(uint8_t r)
{
	SET_CY(1);      //Set Carry Flag
}

//SETB bit addr                             /* 1: 1101 0010 */
void axc51base_cpu_device::setb_bitaddr(uint8_t r)
{
	uint8_t addr = m_program.read_byte(m_pc++);             //Grab bit address
	bit_address_w(addr,1);                          //Set Bit at Bit Address
}

//SJMP code addr                            /* 1: 1000 0000 */
void axc51base_cpu_device::sjmp(uint8_t r)
{
	int8_t rel_addr = m_program.read_byte(m_pc++);          //Grab relative code address
	m_pc = m_pc + rel_addr;                     //Update m_pc
}

//SUBB A, #data                             /* 1: 1001 0100 */
void axc51base_cpu_device::subb_a_byte(uint8_t r)
{
	uint8_t data = m_program.read_byte(m_pc++);             //Grab data
	uint8_t result = ACC - data - GET_CY; //Subtract data & carry flag from accumulator
	do_sub_flags(ACC,data,GET_CY);      //Set Flags
	SET_ACC(result);                        //Store 8 bit result of addtion in ACC

}

//SUBB A, data addr                         /* 1: 1001 0101 */
void axc51base_cpu_device::subb_a_mem(uint8_t r)
{
	uint8_t addr = m_program.read_byte(m_pc++);             //Grab data address
	uint8_t data = iram_read(addr);              //Grab data from data address
	uint8_t result = ACC - data - GET_CY; //Subtract data & carry flag from accumulator
	do_sub_flags(ACC,data,GET_CY);      //Set Flags
	SET_ACC(result);                        //Store 8 bit result of addtion in ACC
}

//SUBB A, @R0/@R1                           /* 1: 1001 011i */
void axc51base_cpu_device::subb_a_ir(uint8_t r)
{
	uint8_t data = iram_indirect_read(R_REG(r));         //Grab data from memory pointed to by R0 or R1
	uint8_t result = ACC - data - GET_CY; //Subtract data & carry flag from accumulator
	do_sub_flags(ACC,data,GET_CY);      //Set Flags
	SET_ACC(result);                        //Store 8 bit result of addtion in ACC
}

//SUBB A, R0 to R7                          /* 1: 1001 1rrr */
void axc51base_cpu_device::subb_a_r(uint8_t r)
{
	uint8_t data = R_REG(r);                  //Grab data from R0 - R7
	uint8_t result = ACC - data - GET_CY; //Subtract data & carry flag from accumulator
	do_sub_flags(ACC,data,GET_CY);      //Set Flags
	SET_ACC(result);                        //Store 8 bit result of addtion in ACC
}

//SWAP A                                    /* 1: 1100 0100 */
void axc51base_cpu_device::swap_a(uint8_t r)
{
	uint8_t a_nib_lo, a_nib_hi;
	a_nib_hi = (ACC & 0x0f) << 4;           //Grab lo byte of ACC and move to hi
	a_nib_lo = (ACC & 0xf0) >> 4;           //Grab hi byte of ACC and move to lo
	SET_ACC( a_nib_hi | a_nib_lo);
}

//XCH A, data addr                          /* 1: 1100 0101 */
void axc51base_cpu_device::xch_a_mem(uint8_t r)
{
	uint8_t addr = m_program.read_byte(m_pc++);             //Grab data address
	uint8_t data = iram_read(addr);              //Grab data
	uint8_t oldACC = ACC;                 //Hold value of ACC
	SET_ACC(data);                      //Sets ACC to data
	iram_write(addr,oldACC);                    //Sets data address to old value of ACC
}

//XCH A, @RO/@R1                            /* 1: 1100 011i */
void axc51base_cpu_device::xch_a_ir(uint8_t r)
{
	uint8_t data = iram_indirect_read(R_REG(r));         //Grab data pointed to by R0 or R1
	uint8_t oldACC = ACC;                 //Hold value of ACC
	SET_ACC(data);                      //Sets ACC to data
	iram_indirect_write(R_REG(r),oldACC);                    //Sets data address to old value of ACC
}

//XCH A, RO to R7                           /* 1: 1100 1rrr */
void axc51base_cpu_device::xch_a_r(uint8_t r)
{
	uint8_t data = R_REG(r);                  //Grab data from R0-R7
	uint8_t oldACC = ACC;                 //Hold value of ACC
	SET_ACC(data);                      //Sets ACC to data
	SET_REG(r, oldACC);                     //Sets data address to old value of ACC
}

//XCHD A, @R0/@R1                           /* 1: 1101 011i */
void axc51base_cpu_device::xchd_a_ir(uint8_t r)
{
	uint8_t acc, ir_data;
	ir_data = iram_indirect_read(R_REG(r));                //Grab data pointed to by R0 or R1
	acc = ACC;                          //Grab ACC value
	SET_ACC( (acc & 0xf0) | (ir_data & 0x0f) );     //Set ACC to lower nibble of data pointed to by R0 or R1
	iram_write(R_REG(r), (ir_data & 0xf0) | (acc & 0x0f) ); //Set data pointed to by R0 or R1 to lower nibble of ACC
}

//XRL data addr, A                          /* 1: 0110 0010 */
void axc51base_cpu_device::xrl_mem_a(uint8_t r)
{
	uint8_t addr = m_program.read_byte(m_pc++);             //Grab data address
	uint8_t data = iram_read(addr);              //Grab data from data address
	iram_write(addr,data ^ ACC);                //Set data address value to it's value Logical XOR with ACC
}

//XRL data addr, #data                      /* 1: 0110 0011 */
void axc51base_cpu_device::xrl_mem_byte(uint8_t r)
{
	uint8_t addr = m_program.read_byte(m_pc++);             //Grab data address
	uint8_t data = m_program.read_byte(m_pc++);             //Grab data
	uint8_t srcdata = iram_read(addr);           //Grab data from data address
	iram_write(addr,srcdata ^ data);            //Set data address value to it's value Logical XOR with Data
}

//XRL A, #data                              /* 1: 0110 0100 */
void axc51base_cpu_device::xrl_a_byte(uint8_t r)
{
	uint8_t data = m_program.read_byte(m_pc++);             //Grab data
	SET_ACC(ACC ^ data);                //Set ACC to value of ACC Logical XOR with Data
}

//XRL A, data addr                          /* 1: 0110 0101 */
void axc51base_cpu_device::xrl_a_mem(uint8_t r)
{
	uint8_t addr = m_program.read_byte(m_pc++);             //Grab data address
	uint8_t data = iram_read(addr);              //Grab data from data address
	SET_ACC(ACC ^ data);                //Set ACC to value of ACC Logical XOR with Data
}

//XRL A, @R0/@R1                            /* 1: 0110 011i */
void axc51base_cpu_device::xrl_a_ir(uint8_t r)
{
	uint8_t data = iram_indirect_read(R_REG(r));         //Grab data from address R0 or R1 points to
	SET_ACC(ACC ^ data);                //Set ACC to value of ACC Logical XOR with Data
}

//XRL A, R0 to R7                           /* 1: 0110 1rrr */
void axc51base_cpu_device::xrl_a_r(uint8_t r)
{
	uint8_t data = R_REG(r);                  //Grab data from R0 - R7
	SET_ACC(ACC ^ data);                //Set ACC to value of ACC Logical XOR with Data
}

//illegal opcodes
void axc51base_cpu_device::illegal(uint8_t r)
{
	LOGMASKED(LOG_GENERAL,"i8051 '%s': illegal opcode at 0x%03x: %02x\n", tag(), m_pc-1, r);
}
