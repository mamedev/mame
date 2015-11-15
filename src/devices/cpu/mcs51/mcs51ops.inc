/*******************************************************************************************
 NOTE: All registers are accessed directly, instead of using the SFR_R() function for speed
 Direct register access is availabe from the R_(register name) macros.. ex: ACC for the ACC
 with the exception of the PC
********************************************************************************************/

//ACALL code addr                           /* 1: aaa1 0001 */
OPHANDLER( acall )
{
	UINT8 addr = ROP_ARG(PC++);             //Grab code address byte
	PUSH_PC();                              //Save PC to the stack
	//Thanks Gerrit for help with this! :)
	PC = (PC & 0xf800) | ((r & 0xe0) << 3) | addr;
}

//ADD A, #data                              /* 1: 0010 0100 */
OPHANDLER( add_a_byte )
{
	UINT8 data = ROP_ARG(PC++);             //Grab data
	UINT8 result = ACC + data;          //Add data to accumulator
	DO_ADD_FLAGS(ACC,data,0);               //Set Flags
	SET_ACC(result);                        //Store 8 bit result of addtion in ACC
}

//ADD A, data addr                          /* 1: 0010 0101 */
OPHANDLER( add_a_mem )
{
	UINT8 addr = ROP_ARG(PC++);             //Grab data address
	UINT8 data = IRAM_R(addr);              //Grab data from data address
	UINT8 result = ACC + data;          //Add data to accumulator
	DO_ADD_FLAGS(ACC,data,0);               //Set Flags
	SET_ACC(result);                        //Store 8 bit result of addtion in ACC
}

//ADD A, @R0/@R1                            /* 1: 0010 011i */
OPHANDLER( add_a_ir )
{
	UINT8 data = IRAM_IR(R_REG(r));         //Grab data from memory pointed to by R0 or R1
	UINT8 result = ACC + data;          //Add data to accumulator
	DO_ADD_FLAGS(ACC,data,0);               //Set Flags
	SET_ACC(result);                        //Store 8 bit result of addtion in ACC
}

//ADD A, R0 to R7                           /* 1: 0010 1rrr */
OPHANDLER( add_a_r )
{
	UINT8 data = R_REG(r);                  //Grab data from R0 - R7
	UINT8 result = ACC + data;          //Add data to accumulator
	DO_ADD_FLAGS(ACC,data,0);               //Set Flags
	SET_ACC(result);                        //Store 8 bit result of addtion in ACC
}

//ADDC A, #data                             /* 1: 0011 0100 */
OPHANDLER( addc_a_byte )
{
	UINT8 data = ROP_ARG(PC++);             //Grab data
	UINT8 result = ACC + data + GET_CY; //Add data + carry flag to accumulator
	DO_ADD_FLAGS(ACC,data,GET_CY);      //Set Flags
	SET_ACC(result);                        //Store 8 bit result of addtion in ACC
}

//ADDC A, data addr                         /* 1: 0011 0101 */
OPHANDLER( addc_a_mem )
{
	UINT8 addr = ROP_ARG(PC++);             //Grab data address
	UINT8 data = IRAM_R(addr);              //Grab data from data address
	UINT8 result = ACC + data + GET_CY; //Add data + carry flag to accumulator
	DO_ADD_FLAGS(ACC,data,GET_CY);      //Set Flags
	SET_ACC(result);                        //Store 8 bit result of addtion in ACC
}

//ADDC A, @R0/@R1                           /* 1: 0011 011i */
OPHANDLER( addc_a_ir )
{
	UINT8 data = IRAM_IR(R_REG(r));         //Grab data from memory pointed to by R0 or R1
	UINT8 result = ACC + data + GET_CY; //Add data + carry flag to accumulator
	DO_ADD_FLAGS(ACC,data,GET_CY);      //Set Flags
	SET_ACC(result);                        //Store 8 bit result of addtion in ACC
}

//ADDC A, R0 to R7                          /* 1: 0011 1rrr */
OPHANDLER( addc_a_r )
{
	UINT8 data = R_REG(r);                  //Grab data from R0 - R7
	UINT8 result = ACC + data + GET_CY; //Add data + carry flag to accumulator
	DO_ADD_FLAGS(ACC,data,GET_CY);      //Set Flags
	SET_ACC(result);                        //Store 8 bit result of addtion in ACC
}

//AJMP code addr                            /* 1: aaa0 0001 */
OPHANDLER( ajmp )
{
	UINT8 addr = ROP_ARG(PC++);             //Grab code address byte
	//Thanks Gerrit for help with this! :)
	PC = (PC & 0xf800) | ((r & 0xe0) << 3) | addr;
}

//ANL data addr, A                          /* 1: 0101 0010 */
OPHANDLER( anl_mem_a )
{
	UINT8 addr = ROP_ARG(PC++);             //Grab data address
	UINT8 data = IRAM_R(addr);              //Grab data from data address
	IRAM_W(addr,data & ACC);                //Set data address value to it's value Logical AND with ACC
}

//ANL data addr, #data                      /* 1: 0101 0011 */
OPHANDLER( anl_mem_byte )
{
	UINT8 addr = ROP_ARG(PC++);             //Grab data address
	UINT8 data = ROP_ARG(PC++);             //Grab data
	UINT8 srcdata = IRAM_R(addr);           //Grab data from data address
	IRAM_W(addr,srcdata & data);            //Set data address value to it's value Logical AND with Data
}

//ANL A, #data                              /* 1: 0101 0100 */
OPHANDLER( anl_a_byte )
{
	UINT8 data = ROP_ARG(PC++);             //Grab data
	SET_ACC(ACC & data);                //Set ACC to value of ACC Logical AND with Data
}

//ANL A, data addr                          /* 1: 0101 0101 */
OPHANDLER( anl_a_mem )
{
	UINT8 addr = ROP_ARG(PC++);             //Grab data address
	UINT8 data = IRAM_R(addr);              //Grab data from data address
	SET_ACC(ACC & data);                //Set ACC to value of ACC Logical AND with Data
}

//ANL A, @RO/@R1                            /* 1: 0101 011i */
OPHANDLER( anl_a_ir )
{
	UINT8 data = IRAM_IR(R_REG(r));         //Grab data from address R0 or R1 points to
	SET_ACC(ACC & data);                //Set ACC to value of ACC Logical AND with Data
}

//ANL A, RO to R7                           /* 1: 0101 1rrr */
OPHANDLER( anl_a_r )
{
	UINT8 data = R_REG(r);                  //Grab data from R0 - R7
	SET_ACC(ACC & data);                //Set ACC to value of ACC Logical AND with Data
}

//ANL C, bit addr                           /* 1: 1000 0010 */
OPHANDLER( anl_c_bitaddr )
{
	int cy = GET_CY;
	UINT8 addr = ROP_ARG(PC++);             //Grab bit address
	UINT8 bit = BIT_R(addr);                //Grab bit data from bit address
	SET_CY( (cy & bit) );                   //Set Carry flag to Carry Flag Value Logical AND with Bit
}

//ANL C,/bit addr                           /* 1: 1011 0000 */
OPHANDLER( anl_c_nbitaddr )
{
	int cy = GET_CY;
	UINT8 addr = ROP_ARG(PC++);             //Grab bit address
	UINT8 bit = BIT_R(addr);                //Grab bit data from bit address
	bit = ((~bit)&1);                       //Complement bit
	SET_CY( (cy & bit) );                   //Set Carry flag to Carry Flag Value Logical AND with Complemented Bit
}

//CJNE A, #data, code addr                  /* 1: 1011 0100 */
OPHANDLER( cjne_a_byte )
{
	UINT8 data = ROP_ARG(PC++);             //Grab data
	INT8 rel_addr = ROP_ARG(PC++);          //Grab relative code address

	if(ACC != data)                     //Jump if values are not equal
	{
		PC = PC + rel_addr;
	}

	//Set carry flag to 1 if 1st compare value is < 2nd compare value
	SET_CY( (ACC < data) );
}

//CJNE A, data addr, code addr              /* 1: 1011 0101 */
OPHANDLER( cjne_a_mem )
{
	UINT8 addr = ROP_ARG(PC++);             //Grab data address
	INT8 rel_addr = ROP_ARG(PC++);          //Grab relative code address
	UINT8 data = IRAM_R(addr);              //Pull value from data address

	if(ACC != data)                     //Jump if values are not equal
	{
		PC = PC + rel_addr;
	}

	//Set carry flag to 1 if 1st compare value is < 2nd compare value
	SET_CY( (ACC < data) );
}

//CJNE @R0/@R1, #data, code addr            /* 1: 1011 011i */
OPHANDLER( cjne_ir_byte )
{
	UINT8 data = ROP_ARG(PC++);             //Grab data
	INT8 rel_addr = ROP_ARG(PC++);          //Grab relative code address
	UINT8 srcdata = IRAM_IR(R_REG(r));      //Grab value pointed to by R0 or R1

	if(srcdata != data)                     //Jump if values are not equal
	{
		PC = PC + rel_addr;
	}

	//Set carry flag to 1 if 1st compare value is < 2nd compare value
	SET_CY( (srcdata < data) );
}

//CJNE R0 to R7, #data, code addr           /* 1: 1011 1rrr */
OPHANDLER( cjne_r_byte )
{
	UINT8 data = ROP_ARG(PC++);             //Grab data
	INT8 rel_addr = ROP_ARG(PC++);          //Grab relative code address
	UINT8 srcdata = R_REG(r);                   //Grab value of R0 - R7

	if(srcdata != data)                     //Jump if values are not equal
	{
		PC = PC + rel_addr;
	}

	//Set carry flag to 1 if 1st compare value is < 2nd compare value
	SET_CY( (srcdata < data) );
}

//CLR bit addr                              /* 1: 1100 0010 */
OPHANDLER( clr_bitaddr )
{
	UINT8 addr = ROP_ARG(PC++);             //Grab bit address
	BIT_W(addr,0);                          //Clear bit at specified bit address
}

//CLR C                                     /* 1: 1100 0011 */
OPHANDLER( clr_c )
{
	SET_CY(0);                              //Clear Carry Flag
}

//CLR A                                     /* 1: 1110 0100 */
OPHANDLER( clr_a )
{
	SET_ACC(0);                         //Clear Accumulator
}

//CPL bit addr                              /* 1: 1011 0010 */
OPHANDLER( cpl_bitaddr )
{
	UINT8 addr = ROP_ARG(PC++);             //Grab bit address
	UINT8 data = (~BIT_R(addr))&1;
	BIT_W(addr,data);                       //Complement bit at specified bit address
}

//CPL C                                     /* 1: 1011 0011 */
OPHANDLER( cpl_c )
{
	UINT8 bit = (~GET_CY)&1;                //Complement Carry Flag
	SET_CY(bit);
}

//CPL A                                     /* 1: 1111 0100 */
OPHANDLER( cpl_a )
{
	UINT8 data = ((~ACC)&0xff);
	SET_ACC(data);                      //Complement Accumulator
}

//DA A                                      /* 1: 1101 0100 */
OPHANDLER( da_a )
{
/*From several sources, since none said the same thing:
 The decimal adjust instruction is associated with the use of the ADD and ADDC instructions.
 The eight-bit value in the accumulator is adjusted to form two BCD digits of four bits each.
 If the accumulator contents bits 0-3 are greater than 9, OR the AC flag is set, then six is added to
 produce a proper BCD digit.
 If the carry is set, OR the four high bits 4-7 exceed nine, six is added to the value of these bits.
 The carry flag will be set if the result is > 0x99, but not cleared otherwise */

	UINT16 new_acc = ACC & 0xff;
	if(GET_AC || (new_acc & 0x0f) > 0x09)
		new_acc += 0x06;
	if(GET_CY || ((new_acc & 0xf0) > 0x90) || (new_acc & ~0xff))
		new_acc += 0x60;
	SET_ACC(new_acc&0xff);
	if(new_acc & ~0xff)
	SET_CY(1);
}

//DEC A                                     /* 1: 0001 0100 */
OPHANDLER( dec_a )
{
	SET_ACC(ACC-1);
}

//DEC data addr                             /* 1: 0001 0101 */
OPHANDLER( dec_mem )
{
	UINT8 addr = ROP_ARG(PC++);             //Grab data address
	UINT8 data = IRAM_R(addr);
	IRAM_W(addr,data-1);
}

//DEC @R0/@R1                               /* 1: 0001 011i */
OPHANDLER( dec_ir )
{
	UINT8 data = IRAM_IR(R_REG(r));
	IRAM_W(R_REG(r),data-1);
}

//DEC R0 to R7                              /* 1: 0001 1rrr */
OPHANDLER( dec_r )
{
	SET_REG(r, R_REG(r) - 1);
}

//DIV AB                                    /* 1: 1000 0100 */
OPHANDLER( div_ab )
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
OPHANDLER( djnz_mem )
{
	UINT8 addr = ROP_ARG(PC++);             //Grab data address
	INT8 rel_addr = ROP_ARG(PC++);          //Grab relative code address
	IRAM_W(addr,IRAM_R(addr) - 1);          //Decrement value contained at data address
	if(IRAM_R(addr) != 0)                   //Branch if contents of data address is not 0
	{
		PC = PC + rel_addr;
	}
}

//DJNZ R0 to R7,code addr                   /* 1: 1101 1rrr */
OPHANDLER( djnz_r )
{
	INT8 rel_addr = ROP_ARG(PC++);          //Grab relative code address
	SET_REG(r ,R_REG(r) - 1);                   //Decrement value
	if(R_REG(r) != 0)                           //Branch if contents of R0 - R7 is not 0
	{
		PC = PC + rel_addr;
	}
}

//INC A                                     /* 1: 0000 0100 */
OPHANDLER( inc_a )
{
	SET_ACC(ACC+1);
}

//INC data addr                             /* 1: 0000 0101 */
OPHANDLER( inc_mem )
{
	UINT8 addr = ROP_ARG(PC++);             //Grab data address
	UINT8 data = IRAM_R(addr);
	IRAM_W(addr,data+1);
}

//INC @R0/@R1                               /* 1: 0000 011i */
OPHANDLER( inc_ir )
{
	UINT8 data = IRAM_IR(R_REG(r));
	IRAM_W(R_REG(r),data+1);
}

//INC R0 to R7                              /* 1: 0000 1rrr */
OPHANDLER( inc_r )
{
	UINT8 data = R_REG(r);
	SET_REG(r, data + 1);
}

//INC DPTR                                  /* 1: 1010 0011 */
OPHANDLER( inc_dptr )
{
	UINT16 dptr = (DPTR)+1;
	SET_DPTR(dptr);
}

//JB  bit addr, code addr                   /* 1: 0010 0000 */
OPHANDLER( jb )
{
	UINT8 addr = ROP_ARG(PC++);             //Grab bit address
	INT8 rel_addr = ROP_ARG(PC++);          //Grab relative code address
	if(BIT_R(addr))                         //If bit set at specified bit address, jump
	{
		PC = PC + rel_addr;
	}
}

//JBC bit addr, code addr                   /* 1: 0001 0000 */
OPHANDLER( jbc )
{
	UINT8 addr = ROP_ARG(PC++);             //Grab bit address
	INT8 rel_addr = ROP_ARG(PC++);          //Grab relative code address
	if(BIT_R(addr)) {                       //If bit set at specified bit address, jump
		PC = PC + rel_addr;
		BIT_W(addr,0);                      //Clear Bit also
	}
}

//JC code addr                              /* 1: 0100 0000 */
OPHANDLER( jc )
{
	INT8 rel_addr = ROP_ARG(PC++);          //Grab relative code address
	if(GET_CY)                              //Jump if Carry Flag Set
	{
		PC = PC + rel_addr;
	}
}

//JMP @A+DPTR                               /* 1: 0111 0011 */
OPHANDLER( jmp_iadptr )
{
	PC = ACC + DPTR;
}

//JNB bit addr, code addr                   /* 1: 0011 0000 */
OPHANDLER( jnb )
{
	UINT8 addr = ROP_ARG(PC++);             //Grab bit address
	INT8 rel_addr = ROP_ARG(PC++);          //Grab relative code address
	if(!BIT_R(addr))                        //If bit NOT set at specified bit address, jump
	{
		PC = PC + rel_addr;
	}
}

//JNC code addr                             /* 1: 0101 0000 */
OPHANDLER( jnc )
{
	INT8 rel_addr = ROP_ARG(PC++);          //Grab relative code address
	if(!GET_CY)                             //Jump if Carry Flag not set
	{
		PC = PC + rel_addr;
	}
}

//JNZ code addr                             /* 1: 0111 0000 */
OPHANDLER( jnz )
{
	INT8 rel_addr = ROP_ARG(PC++);          //Grab relative code address
	if(ACC != 0)                            //Branch if ACC is not 0
	{
		PC = PC+rel_addr;
	}
}

//JZ code addr                              /* 1: 0110 0000 */
OPHANDLER( jz )
{
	INT8 rel_addr = ROP_ARG(PC++);          //Grab relative code address
	if(ACC == 0)                            //Branch if ACC is 0
	{
		PC = PC+rel_addr;
	}
}

//LCALL code addr                           /* 1: 0001 0010 */
OPHANDLER( lcall )
{
	UINT8 addr_hi, addr_lo;
	addr_hi = ROP_ARG(PC++);
	addr_lo = ROP_ARG(PC++);
	PUSH_PC();
	PC = (UINT16)((addr_hi<<8) | addr_lo);
}

//LJMP code addr                            /* 1: 0000 0010 */
OPHANDLER( ljmp )
{
	UINT8 addr_hi, addr_lo;
	addr_hi = ROP_ARG(PC++);
	addr_lo = ROP_ARG(PC++);
	PC = (UINT16)((addr_hi<<8) | addr_lo);
}

//MOV A, #data                              /* 1: 0111 0100 */
OPHANDLER( mov_a_byte )
{
	UINT8 data = ROP_ARG(PC++);             //Grab data
	SET_ACC(data);                      //Store data to ACC
}

//MOV A, data addr                          /* 1: 1110 0101 */
OPHANDLER( mov_a_mem )
{
	UINT8 addr = ROP_ARG(PC++);             //Grab data address
	SET_ACC(IRAM_R(addr));              //Store contents of data address to ACC
}

//MOV A,@RO/@R1                             /* 1: 1110 011i */
OPHANDLER( mov_a_ir )
{
	SET_ACC(IRAM_IR(R_REG(r)));             //Store contents of address pointed by R0 or R1 to ACC
}

//MOV A,R0 to R7                            /* 1: 1110 1rrr */
OPHANDLER( mov_a_r )
{
	SET_ACC(R_REG(r));                      //Store contents of R0 - R7 to ACC
}

//MOV data addr, #data                      /* 1: 0111 0101 */
OPHANDLER( mov_mem_byte )
{
	UINT8 addr = ROP_ARG(PC++);             //Grab data address
	UINT8 data = ROP_ARG(PC++);             //Grab data
	IRAM_W(addr,data);                      //Store data to data address location
}

//MOV data addr, data addr                  /* 1: 1000 0101 */
OPHANDLER( mov_mem_mem )
{
	//1st address is src, 2nd is dst, but the mov command works as mov dst,src)
	UINT8 src,dst;
	src = ROP_ARG(PC++);                    //Grab source data address
	dst = ROP_ARG(PC++);                    //Grab destination data address
	IRAM_W(dst,IRAM_R(src));                //Read source address contents and store to destination address
}

//MOV @R0/@R1, #data                        /* 1: 0111 011i */
OPHANDLER( mov_ir_byte )
{
	UINT8 data = ROP_ARG(PC++);             //Grab data
	IRAM_IW(R_REG(r),data);                 //Store data to address pointed by R0 or R1
}

//MOV R0 to R7, #data                       /* 1: 0111 1rrr */
OPHANDLER( mov_r_byte )
{
	UINT8 data = ROP_ARG(PC++);             //Grab data
	SET_REG(r, data);                           //Store to R0 - R7
}

//MOV data addr, @R0/@R1                    /* 1: 1000 011i */
OPHANDLER( mov_mem_ir )
{
	UINT8 addr = ROP_ARG(PC++);             //Grab data address
	IRAM_W(addr,IRAM_IR(R_REG(r)));         //Store contents pointed to by R0 or R1 to data address
}

//MOV data addr,R0 to R7                    /* 1: 1000 1rrr */
OPHANDLER( mov_mem_r )
{
	UINT8 addr = ROP_ARG(PC++);             //Grab data address
	IRAM_W(addr,R_REG(r));                  //Store contents of R0 - R7 to data address
}

//MOV DPTR, #data16                         /* 1: 1001 0000 */
OPHANDLER( mov_dptr_byte )
{
	UINT8 data_hi, data_lo;
	data_hi = ROP_ARG(PC++);                //Grab hi byte
	data_lo = ROP_ARG(PC++);                //Grab lo byte
	SET_DPTR((UINT16)((data_hi<<8)|data_lo));   //Store to DPTR
}

//MOV bit addr, C                           /* 1: 1001 0010 */
OPHANDLER( mov_bitaddr_c )
{
	UINT8 addr = ROP_ARG(PC++);             //Grab bit address
	BIT_W(addr,GET_CY);                     //Store Carry Flag to Bit Address
}

//MOV @R0/@R1, data addr                    /* 1: 1010 011i */
OPHANDLER( mov_ir_mem )
{
	UINT8 addr = ROP_ARG(PC++);             //Grab data address
	IRAM_IW(R_REG(r),IRAM_R(addr));         //Store data from data address to address pointed to by R0 or R1
}

//MOV R0 to R7, data addr                   /* 1: 1010 1rrr */
OPHANDLER( mov_r_mem )
{
	UINT8 addr = ROP_ARG(PC++);             //Grab data address
	SET_REG(r, IRAM_R(addr));               //Store to R0 - R7
}

//MOV data addr, A                          /* 1: 1111 0101 */
OPHANDLER( mov_mem_a )
{
	UINT8 addr = ROP_ARG(PC++);             //Grab data address
	IRAM_W(addr,ACC);                       //Store A to data address
}

//MOV @R0/@R1, A                            /* 1: 1111 011i */
OPHANDLER( mov_ir_a )
{
	IRAM_IW(R_REG(r),ACC);                  //Store A to location pointed to by R0 or R1
}

//MOV R0 to R7, A                           /* 1: 1111 1rrr */
OPHANDLER( mov_r_a )
{
	SET_REG(r, ACC);                        //Store A to R0-R7
}

//MOVC A, @A + PC                           /* 1: 1000 0011 */
OPHANDLER( movc_a_iapc )
{
	UINT8 data;
	data = CODEMEM_R(ACC+PC);               //Move a byte from CODE(Program) Memory and store to ACC
	SET_ACC(data);
}

//MOV C, bit addr                           /* 1: 1010 0010 */
OPHANDLER( mov_c_bitaddr )
{
	UINT8 addr = ROP_ARG(PC++);             //Grab bit address
	SET_CY( (BIT_R(addr)) );                //Store Bit from Bit Address to Carry Flag
}

//MOVC A, @A + DPTR                         /* 1: 1001 0011 */
OPHANDLER( movc_a_iadptr )
{
	UINT8 data;
	data = CODEMEM_R(ACC + DPTR);           //Move a byte from CODE(Program) Memory and store to ACC
	SET_ACC(data);
}

//MOVX A,@DPTR                              /* 1: 1110 0000 */
//(Move External Ram 16 bit address to A)
OPHANDLER( movx_a_idptr )
{
//  UINT8 byte = DATAMEM_R(R_DPTR);         //Grab 1 byte from External DATA memory pointed to by dptr
	UINT32 addr = ERAM_ADDR(DPTR, 0xFFFF);
	UINT8 byte = DATAMEM_R(addr);           //Grab 1 byte from External DATA memory pointed to by dptr
	SET_ACC(byte);                      //Store to ACC
}

//MOVX A, @R0/@R1                           /* 1: 1110 001i */
//(Move External Ram 8 bit address to A)
OPHANDLER( movx_a_ir )
{
	UINT32 addr = ERAM_ADDR(R_REG(r),0xFF); //Grab address by reading location pointed to by R0 or R1
	UINT8 byte = DATAMEM_R(addr);           //Grab 1 byte from External DATA memory pointed to by address
	SET_ACC(byte);                      //Store to ACC
}

//MOVX @DPTR,A                              /* 1: 1111 0000 */
//(Move A to External Ram 16 bit address)
OPHANDLER( movx_idptr_a )
{
//  DATAMEM_W(R_DPTR, ACC);               //Store ACC to External DATA memory address pointed to by DPTR
	UINT32 addr = ERAM_ADDR(DPTR, 0xFFFF);
	DATAMEM_W(addr, ACC);               //Store ACC to External DATA memory address pointed to by DPTR
}

//MOVX @R0/@R1,A                            /* 1: 1111 001i */
//(Move A to External Ram 8 bit address)
OPHANDLER( movx_ir_a )
{
	UINT32 addr = ERAM_ADDR(R_REG(r),0xFF);   //Grab address by reading location pointed to by R0 or R1
	DATAMEM_W(addr, ACC);                   //Store ACC to External DATA memory address
}

//MUL AB                                    /* 1: 1010 0100 */
OPHANDLER( mul_ab )
{
	UINT16 result = ACC * B;
	//A gets lo bits, B gets hi bits of result
	B = (UINT8) ((result & 0xFF00) >> 8);
	SET_ACC((UINT8)(result & 0x00FF));
	//Set flags
	SET_OV( ((result & 0x100) >> 8) );      //Set/Clear Overflow Flag if result > 255
	SET_CY(0);                              //Carry Flag always cleared
}

//NOP                                       /* 1: 0000 0000 */
OPHANDLER( nop )
{
}

//ORL data addr, A                          /* 1: 0100 0010 */
OPHANDLER( orl_mem_a )
{
	UINT8 addr = ROP_ARG(PC++);             //Grab data address
	UINT8 data = IRAM_R(addr);              //Grab data from data address
	IRAM_W(addr,data | ACC);                //Set data address value to it's value Logical OR with ACC
}

//ORL data addr, #data                      /* 1: 0100 0011 */
OPHANDLER( orl_mem_byte )
{
	UINT8 addr = ROP_ARG(PC++);             //Grab data address
	UINT8 data = ROP_ARG(PC++);             //Grab data
	UINT8 srcdata = IRAM_R(addr);           //Grab data from data address
	IRAM_W(addr,srcdata | data);            //Set data address value to it's value Logical OR with Data
}

//ORL A, #data                              /* 1: 0100 0100 */
OPHANDLER( orl_a_byte )
{
	UINT8 data = ROP_ARG(PC++);             //Grab data
	SET_ACC(ACC | data);                //Set ACC to value of ACC Logical OR with Data
}

//ORL A, data addr                          /* 1: 0100 0101 */
OPHANDLER( orl_a_mem )
{
	UINT8 addr = ROP_ARG(PC++);             //Grab data address
	UINT8 data = IRAM_R(addr);              //Grab data from data address
	SET_ACC(ACC | data);                //Set ACC to value of ACC Logical OR with Data
}

//ORL A, @RO/@R1                            /* 1: 0100 011i */
OPHANDLER( orl_a_ir )
{
	UINT8 data = IRAM_IR(R_REG(r));         //Grab data from address R0 or R1 points to
	SET_ACC(ACC | data);                //Set ACC to value of ACC Logical OR with Data
}

//ORL A, RO to R7                           /* 1: 0100 1rrr */
OPHANDLER( orl_a_r )
{
	UINT8 data = R_REG(r);                  //Grab data from R0 - R7
	SET_ACC(ACC | data);                //Set ACC to value of ACC Logical OR with Data
}

//ORL C, bit addr                           /* 1: 0111 0010 */
OPHANDLER( orl_c_bitaddr )
{
	int cy = GET_CY;
	UINT8 addr = ROP_ARG(PC++);             //Grab bit address
	UINT8 bit = BIT_R(addr);                //Grab bit data from bit address
	SET_CY( (cy | bit) );                   //Set Carry flag to Carry Flag Value Logical OR with Bit
}

//ORL C, /bit addr                          /* 1: 1010 0000 */
OPHANDLER( orl_c_nbitaddr )
{
	int cy = GET_CY;
	UINT8 addr = ROP_ARG(PC++);             //Grab bit address
	UINT8 bit = BIT_R(addr);                //Grab bit data from bit address
	bit = ((~bit)&1);                       //Complement bit
	SET_CY( (cy | bit) );                   //Set Carry flag to Carry Flag Value Logical OR with Complemented Bit
}

//POP data addr                             /* 1: 1101 0000 */
OPHANDLER( pop )
{
	UINT8 addr = ROP_ARG(PC++);             //Grab data address
	IRAM_W(addr, IRAM_IR(SP));              //Store to contents of data addr, data pointed to by Stack - IRAM_IR needed to access upper 128 bytes of stack
	//IRAM_IW(addr, IRAM_IR(R_SP));         //Store to contents of data addr, data pointed to by Stack - doesn't work, sfr's are not restored this way and it's not an indirect access anyway
	SP = SP-1;                              //Decrement SP
}

//PUSH data addr                            /* 1: 1100 0000 */
OPHANDLER( push )
{
	UINT8 addr = ROP_ARG(PC++);             //Grab data address
	UINT8 tmpSP = SP+1;                 //Grab and Increment Stack Pointer
	SP = tmpSP;                         // ""
	IRAM_IW(tmpSP, IRAM_R(addr));           //Store to stack contents of data address - IRAM_IW needed to store to upper 128 bytes of stack, however, can't use IRAM_IR because that won't store the sfrs and it's not an indirect access anyway
}

//RET                                       /* 1: 0010 0010 */
OPHANDLER( ret )
{
	POP_PC();
}

//RETI                                      /* 1: 0011 0010 */
OPHANDLER( reti )
{
	POP_PC();
	CLEAR_CURRENT_IRQ();
}

//RL A                                      /* 1: 0010 0011 */
OPHANDLER( rl_a )
{
	//Left Shift A, Bit 7 carries to Bit 0
	int carry = ((ACC & 0x80) >> 7);
	int data = (ACC<<1) & 0xfe;
	SET_ACC( data | carry);
}

//RLC A                                     /* 1: 0011 0011 */
OPHANDLER( rlc_a )
{
	//Left Shift A, Bit 7 goes to Carry Flag, Original Carry Flag goes to Bit 0 of ACC
	int carry = ((ACC & 0x80) >> 7);
	int data = ((ACC<<1) & 0xfe) | GET_CY;
	SET_ACC( data);
	SET_CY(carry);
}

//RR A                                      /* 1: 0000 0011 */
OPHANDLER( rr_a )
{
	//Right Shift A, Bit 0 carries to Bit 7
	int carry = ((ACC & 1) << 7);
	int data = (ACC>>1) & 0x7f;
	SET_ACC( data | carry);
}

//RRC A                                     /* 1: 0001 0011 */
OPHANDLER( rrc_a )
{
	//Right Shift A, Bit 0 goes to Carry Flag, Bit 7 of ACC gets set to original Carry Flag
	int carry = (ACC & 1);
	int data = ((ACC>>1) & 0x7f) | (GET_CY<<7);
	SET_ACC( data);
	SET_CY(carry);
}

//SETB C                                    /* 1: 1101 0011 */
OPHANDLER( setb_c )
{
	SET_CY(1);      //Set Carry Flag
}

//SETB bit addr                             /* 1: 1101 0010 */
OPHANDLER( setb_bitaddr )
{
	UINT8 addr = ROP_ARG(PC++);             //Grab bit address
	BIT_W(addr,1);                          //Set Bit at Bit Address
}

//SJMP code addr                            /* 1: 1000 0000 */
OPHANDLER( sjmp )
{
	INT8 rel_addr = ROP_ARG(PC++);          //Grab relative code address
	PC = PC + rel_addr;                     //Update PC
}

//SUBB A, #data                             /* 1: 1001 0100 */
OPHANDLER( subb_a_byte )
{
	UINT8 data = ROP_ARG(PC++);             //Grab data
	UINT8 result = ACC - data - GET_CY; //Subtract data & carry flag from accumulator
	DO_SUB_FLAGS(ACC,data,GET_CY);      //Set Flags
	SET_ACC(result);                        //Store 8 bit result of addtion in ACC

}

//SUBB A, data addr                         /* 1: 1001 0101 */
OPHANDLER( subb_a_mem )
{
	UINT8 addr = ROP_ARG(PC++);             //Grab data address
	UINT8 data = IRAM_R(addr);              //Grab data from data address
	UINT8 result = ACC - data - GET_CY; //Subtract data & carry flag from accumulator
	DO_SUB_FLAGS(ACC,data,GET_CY);      //Set Flags
	SET_ACC(result);                        //Store 8 bit result of addtion in ACC
}

//SUBB A, @R0/@R1                           /* 1: 1001 011i */
OPHANDLER( subb_a_ir )
{
	UINT8 data = IRAM_IR(R_REG(r));         //Grab data from memory pointed to by R0 or R1
	UINT8 result = ACC - data - GET_CY; //Subtract data & carry flag from accumulator
	DO_SUB_FLAGS(ACC,data,GET_CY);      //Set Flags
	SET_ACC(result);                        //Store 8 bit result of addtion in ACC
}

//SUBB A, R0 to R7                          /* 1: 1001 1rrr */
OPHANDLER( subb_a_r )
{
	UINT8 data = R_REG(r);                  //Grab data from R0 - R7
	UINT8 result = ACC - data - GET_CY; //Subtract data & carry flag from accumulator
	DO_SUB_FLAGS(ACC,data,GET_CY);      //Set Flags
	SET_ACC(result);                        //Store 8 bit result of addtion in ACC
}

//SWAP A                                    /* 1: 1100 0100 */
OPHANDLER( swap_a )
{
	UINT8 a_nib_lo, a_nib_hi;
	a_nib_hi = (ACC & 0x0f) << 4;           //Grab lo byte of ACC and move to hi
	a_nib_lo = (ACC & 0xf0) >> 4;           //Grab hi byte of ACC and move to lo
	SET_ACC( a_nib_hi | a_nib_lo);
}

//XCH A, data addr                          /* 1: 1100 0101 */
OPHANDLER( xch_a_mem )
{
	UINT8 addr = ROP_ARG(PC++);             //Grab data address
	UINT8 data = IRAM_R(addr);              //Grab data
	UINT8 oldACC = ACC;                 //Hold value of ACC
	SET_ACC(data);                      //Sets ACC to data
	IRAM_W(addr,oldACC);                    //Sets data address to old value of ACC
}

//XCH A, @RO/@R1                            /* 1: 1100 011i */
OPHANDLER( xch_a_ir )
{
	UINT8 data = IRAM_IR(R_REG(r));         //Grab data pointed to by R0 or R1
	UINT8 oldACC = ACC;                 //Hold value of ACC
	SET_ACC(data);                      //Sets ACC to data
	IRAM_W(R_REG(r),oldACC);                    //Sets data address to old value of ACC
}

//XCH A, RO to R7                           /* 1: 1100 1rrr */
OPHANDLER( xch_a_r )
{
	UINT8 data = R_REG(r);                  //Grab data from R0-R7
	UINT8 oldACC = ACC;                 //Hold value of ACC
	SET_ACC(data);                      //Sets ACC to data
	SET_REG(r, oldACC);                     //Sets data address to old value of ACC
}

//XCHD A, @R0/@R1                           /* 1: 1101 011i */
OPHANDLER( xchd_a_ir )
{
	UINT8 acc, ir_data;
	ir_data = IRAM_IR(R_REG(r));                //Grab data pointed to by R0 or R1
	acc = ACC;                          //Grab ACC value
	SET_ACC( (acc & 0xf0) | (ir_data & 0x0f) );     //Set ACC to lower nibble of data pointed to by R0 or R1
	IRAM_W(R_REG(r), (ir_data & 0xf0) | (acc & 0x0f) ); //Set data pointed to by R0 or R1 to lower nibble of ACC
}

//XRL data addr, A                          /* 1: 0110 0010 */
OPHANDLER( xrl_mem_a )
{
	UINT8 addr = ROP_ARG(PC++);             //Grab data address
	UINT8 data = IRAM_R(addr);              //Grab data from data address
	IRAM_W(addr,data ^ ACC);                //Set data address value to it's value Logical XOR with ACC
}

//XRL data addr, #data                      /* 1: 0110 0011 */
OPHANDLER( xrl_mem_byte )
{
	UINT8 addr = ROP_ARG(PC++);             //Grab data address
	UINT8 data = ROP_ARG(PC++);             //Grab data
	UINT8 srcdata = IRAM_R(addr);           //Grab data from data address
	IRAM_W(addr,srcdata ^ data);            //Set data address value to it's value Logical XOR with Data
}

//XRL A, #data                              /* 1: 0110 0100 */
OPHANDLER( xrl_a_byte )
{
	UINT8 data = ROP_ARG(PC++);             //Grab data
	SET_ACC(ACC ^ data);                //Set ACC to value of ACC Logical XOR with Data
}

//XRL A, data addr                          /* 1: 0110 0101 */
OPHANDLER( xrl_a_mem )
{
	UINT8 addr = ROP_ARG(PC++);             //Grab data address
	UINT8 data = IRAM_R(addr);              //Grab data from data address
	SET_ACC(ACC ^ data);                //Set ACC to value of ACC Logical XOR with Data
}

//XRL A, @R0/@R1                            /* 1: 0110 011i */
OPHANDLER( xrl_a_ir )
{
	UINT8 data = IRAM_IR(R_REG(r));         //Grab data from address R0 or R1 points to
	SET_ACC(ACC ^ data);                //Set ACC to value of ACC Logical XOR with Data
}

//XRL A, R0 to R7                           /* 1: 0110 1rrr */
OPHANDLER( xrl_a_r )
{
	UINT8 data = R_REG(r);                  //Grab data from R0 - R7
	SET_ACC(ACC ^ data);                //Set ACC to value of ACC Logical XOR with Data
}

//illegal opcodes
OPHANDLER( illegal )
{
	LOG(("i8051 '%s': illegal opcode at 0x%03x: %02x\n", tag(), PC-1, r));
}
