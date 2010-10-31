/* descriptor table format in memory
   UINT16 limit
   UINT24 addr
   0..3 type
    system segment
     3 execute:
      0 data segment: readable,
       1: write
       2: expand down
      1 code segment
       1: readable
       2: conforming (can be used with higher privilege level)
     0: access (set when processor accesses segment)

   4 dt 0 system segment, 1 application segment (code, data)
   5,6 dpl descriptor privileg level
   7 p present 0 gives trap when accessed
   UINT16 reserved (should be zero)
*/
#define IS_PRESENT(a)		( ( (a) & 0x80 ) == 0x80 )
#define IS_WRITEABLE(a)		( ( (a) & 0xa ) == 2 )
#define IS_READABLE(a)		( ( ( (a) & 0xa ) == 0xa ) || ( ( (a) & 8 ) == 0 ) )

static void i80286_trap2(i80286_state *cpustate,int number)
{
	i80286_interrupt(cpustate,number);
}

static int i80286_selector_okay(i80286_state *cpustate,UINT16 selector)
{
	if (selector&4) {
		return (selector&~7)<cpustate->ldtr.limit;
	} else {
		return (selector&~7)<cpustate->gdtr.limit;
	}
}

static offs_t i80286_selector_to_address(i80286_state *cpustate,UINT16 selector)
{
	if (selector&4) {
		return cpustate->ldtr.base+(selector&~7);
	} else {
		return cpustate->gdtr.base+(selector&~7);
	}
}

static void i80286_data_descriptor(i80286_state *cpustate,int reg, UINT16 selector)
{
	if (PM) {
		UINT16 help;
		/* selector format
           15..3 number/address in descriptor table
           2: 0 global, 1 local descriptor table
           1,0: requested privileg level
           must be higher or same as current privileg level in code selector */
		if (selector&4) { /* local descriptor table */
			if (selector>cpustate->ldtr.limit) i80286_trap2(cpustate,GENERAL_PROTECTION_FAULT);
			cpustate->sregs[reg]=selector;
			cpustate->limit[reg]=ReadWord(cpustate->ldtr.base+(selector&~7));
			cpustate->base[reg]=ReadWord(cpustate->ldtr.base+(selector&~7)+2)
				|(ReadWord(cpustate->ldtr.base+(selector&~7)+4)<<16);
			cpustate->rights[reg]=cpustate->base[reg]>>24;
			cpustate->base[reg]&=0xffffff;
		} else { /* global descriptor table */
			if (!(selector&~7)||(selector>cpustate->gdtr.limit)) i80286_trap2(cpustate,GENERAL_PROTECTION_FAULT);
			cpustate->sregs[reg]=selector;
			cpustate->limit[reg]=ReadWord(cpustate->gdtr.base+(selector&~7));
			cpustate->base[reg]=ReadWord(cpustate->gdtr.base+(selector&~7)+2);
			help=ReadWord(cpustate->gdtr.base+(selector&~7)+4);
			cpustate->rights[reg]=help>>8;
			cpustate->base[reg]|=(help&0xff)<<16;
		}
	} else {
		cpustate->sregs[reg]=selector;
		cpustate->base[reg]=selector<<4;
	}
}

static void i80286_code_descriptor(i80286_state *cpustate,UINT16 selector, UINT16 offset)
{
	UINT16 word1, word2, word3;
	if (PM) {
		/* selector format
           15..3 number/address in descriptor table
           2: 0 global, 1 local descriptor table
           1,0: requested privileg level
           must be higher or same as current privileg level in code selector */
		if (selector&4) { /* local descriptor table */
			if (selector>cpustate->ldtr.limit) i80286_trap2(cpustate,GENERAL_PROTECTION_FAULT);
			word1=ReadWord(cpustate->ldtr.base+(selector&~7));
			word2=ReadWord(cpustate->ldtr.base+(selector&~7)+2);
			word3=ReadWord(cpustate->ldtr.base+(selector&~7)+4);
		} else { /* global descriptor table */
			if (!(selector&~7)||(selector>cpustate->gdtr.limit)) i80286_trap2(cpustate,GENERAL_PROTECTION_FAULT);
			word1=ReadWord(cpustate->gdtr.base+(selector&~7));
			word2=ReadWord(cpustate->gdtr.base+(selector&~7)+2);
			word3=ReadWord(cpustate->gdtr.base+(selector&~7)+4);
		}
		if (word3&0x1000) {
			cpustate->sregs[CS]=selector;
			cpustate->limit[CS]=word1;
			cpustate->base[CS]=word2|((word3&0xff)<<16);
			cpustate->rights[CS]=word3>>8;
			cpustate->pc=cpustate->base[CS]+offset;
		} else { // systemdescriptor
			switch (word3&0xf00) {
			case 0x400: // call gate
				// word3&0x1f words to be copied from stack to stack
				i80286_data_descriptor(cpustate, CS, word2);
				cpustate->pc=cpustate->base[CS]+word1;
				break;
			case 0x500: // task gate
				i80286_data_descriptor(cpustate, CS, word2);
				cpustate->pc=cpustate->base[CS]+word1;
				break;
			case 0x600: // interrupt gate
				cpustate->TF = cpustate->IF = 0;
				i80286_data_descriptor(cpustate, CS, word2);
				cpustate->pc=cpustate->base[CS]+word1;
				break;
			case 0x700: // trap gate
				i80286_data_descriptor(cpustate, CS, word2);
				cpustate->pc=cpustate->base[CS]+word1;
				break;
			}
		}
	} else {
		cpustate->sregs[CS]=selector;
		cpustate->base[CS]=selector<<4;
		cpustate->pc=cpustate->base[CS]+offset;
	}
}

static void i80286_interrupt_descriptor(i80286_state *cpustate,UINT16 number)
{
	UINT16 word1,word2,word3;
	if ((number<<3)>=cpustate->idtr.limit) {
		;// go into shutdown mode
		return;
	}
	PREFIX(_pushf(cpustate));
	PUSH(cpustate->sregs[CS]);
	PUSH(cpustate->pc - cpustate->base[CS]);
	word1=ReadWord(cpustate->idtr.base+(number<<3));
	word2=ReadWord(cpustate->idtr.base+(number<<3)+2);
	word3=ReadWord(cpustate->idtr.base+(number<<3)+4);
	switch (word3&0xf00) {
	case 0x500: // task gate
		i80286_data_descriptor(cpustate, CS, word2);
		cpustate->pc=cpustate->base[CS]+word1;
		break;
	case 0x600: // interrupt gate
		cpustate->TF = cpustate->IF = 0;
		i80286_data_descriptor(cpustate, CS, word2);
		cpustate->pc=cpustate->base[CS]+word1;
		break;
	case 0x700: // trap gate
		i80286_data_descriptor(cpustate, CS, word2);
		cpustate->pc=cpustate->base[CS]+word1;
		break;
	}
}

static void PREFIX286(_0fpre)(i8086_state *cpustate)
{
	unsigned next = FETCHOP;
	UINT16 ModRM;
	UINT16 tmp;
	offs_t addr;

	switch (next) {
	case 0:
		ModRM=FETCHOP;
		switch (ModRM&0x38) {
		case 0: /* sldt */
			if (!PM) i80286_trap2(cpustate,ILLEGAL_INSTRUCTION);
			PutRMWord(ModRM, cpustate->ldtr.sel);
			break;
		case 8: /* str */
			if (!PM) i80286_trap2(cpustate,ILLEGAL_INSTRUCTION);
			PutRMWord(ModRM, cpustate->tr.sel);
			break;
		case 0x10: /* lldt */
			if (!PM) i80286_trap2(cpustate,ILLEGAL_INSTRUCTION);
			if (PM&&(CPL!=0)) i80286_trap2(cpustate,GENERAL_PROTECTION_FAULT);
			cpustate->ldtr.sel=GetRMWord(ModRM);
			if ((cpustate->ldtr.sel&~7)>=cpustate->gdtr.limit) i80286_trap2(cpustate,GENERAL_PROTECTION_FAULT);
			cpustate->ldtr.limit=ReadWord(cpustate->gdtr.base+(cpustate->ldtr.sel&~7));
			cpustate->ldtr.base=ReadWord(cpustate->gdtr.base+(cpustate->ldtr.sel&~7)+2)
				|(ReadWord(cpustate->gdtr.base+(cpustate->ldtr.sel&~7)+4)<<16);
			cpustate->ldtr.rights=cpustate->ldtr.base>>24;
			cpustate->ldtr.base&=0xffffff;
			break;
		case 0x18: /* ltr */
			if (!PM) i80286_trap2(cpustate,ILLEGAL_INSTRUCTION);
			if (CPL!=0) i80286_trap2(cpustate,GENERAL_PROTECTION_FAULT);
			cpustate->tr.sel=GetRMWord(ModRM);
			if ((cpustate->tr.sel&~7)>=cpustate->gdtr.limit) i80286_trap2(cpustate,GENERAL_PROTECTION_FAULT);
			cpustate->tr.limit=ReadWord(cpustate->gdtr.base+(cpustate->tr.sel&~7));
			cpustate->tr.base=ReadWord(cpustate->gdtr.base+(cpustate->tr.sel&~7)+2)
				|(ReadWord(cpustate->gdtr.base+(cpustate->tr.sel&~7)+4)<<16);
			cpustate->tr.rights=cpustate->tr.base>>24;
			cpustate->tr.base&=0xffffff;
			break;
		case 0x20: /* verr */
			if (!PM) i80286_trap2(cpustate,ILLEGAL_INSTRUCTION);
			tmp=GetRMWord(ModRM);
			if (tmp&4) {
				cpustate->ZeroVal=! ( ((tmp&~7)<cpustate->ldtr.limit)
							&& IS_READABLE( ReadByte(cpustate->ldtr.base+(tmp&~7)+5)) );
			} else {
				cpustate->ZeroVal=! ( ((tmp&~7)<cpustate->gdtr.limit)
							&& IS_READABLE( ReadByte(cpustate->gdtr.base+(tmp&~7)+5)) );
			}
			break;
		case 0x28: /* verw */
			if (!PM) i80286_trap2(cpustate,ILLEGAL_INSTRUCTION);
			tmp=GetRMWord(ModRM);
			if (tmp&4) {
				cpustate->ZeroVal=! ( ((tmp&~7)<cpustate->ldtr.limit)
							&& IS_WRITEABLE( ReadByte(cpustate->ldtr.base+(tmp&~7)+5)) );
			} else {
				cpustate->ZeroVal=! ( ((tmp&~7)<cpustate->gdtr.limit)
							&& IS_WRITEABLE( ReadByte(cpustate->gdtr.base+(tmp&~7)+5)) );
			}
			break;
		default:
			i80286_trap2(cpustate,ILLEGAL_INSTRUCTION);
			break;
		}
		break;
	case 1:
		/* lgdt, lldt in protected mode privilege level 0 required else common protection
           failure 0xd */
		ModRM = FETCHOP;
		switch (ModRM&0x38) {
		case 0: /* sgdt */
			PutRMWord(ModRM,cpustate->gdtr.limit);
			PutRMWordOffset(2,cpustate->gdtr.base&0xffff);
			PutRMByteOffset(4,cpustate->gdtr.base>>16);
			break;
		case 8: /* sidt */
			PutRMWord(ModRM,cpustate->idtr.limit);
			PutRMWordOffset(2,cpustate->idtr.base&0xffff);
			PutRMByteOffset(4,cpustate->idtr.base>>16);
			break;
		case 0x10: /* lgdt */
			if (PM&&(CPL!=0)) i80286_trap2(cpustate,GENERAL_PROTECTION_FAULT);
			cpustate->gdtr.limit=GetRMWord(ModRM);
			cpustate->gdtr.base=GetRMWordOffset(2)|(GetRMByteOffset(4)<<16);
			break;
		case 0x18: /* lidt */
			if (PM&&(CPL!=0)) i80286_trap2(cpustate,GENERAL_PROTECTION_FAULT);
			cpustate->idtr.limit=GetRMWord(ModRM);
			cpustate->idtr.base=GetRMWordOffset(2)|(GetRMByteOffset(4)<<16);
			break;
		case 0x20: /* smsw */
			PutRMWord(ModRM, cpustate->msw);
			break;
		case 0x30: /* lmsw */
			if (PM&&(CPL!=0)) i80286_trap2(cpustate,GENERAL_PROTECTION_FAULT);
			cpustate->msw=(cpustate->msw&1)|GetRMWord(ModRM);
			break;
		default:
			i80286_trap2(cpustate,ILLEGAL_INSTRUCTION);
			break;
		}
		break;
	case 2: /* LAR */
		ModRM = FETCHOP;
		tmp=GetRMWord(ModRM);
		if ( i80286_selector_okay(cpustate,tmp) )
		{
			cpustate->ZeroVal = 0;
			RegWord(ModRM) = ReadByte( i80286_selector_to_address(cpustate,tmp) + 5 ) << 8;
		}
		else
		{
			cpustate->ZeroVal = 1;
		}
		break;
	case 3: /* LSL */
		if (!PM) i80286_trap2(cpustate,ILLEGAL_INSTRUCTION);
		ModRM = FETCHOP;
		tmp=GetRMWord(ModRM);
		if ( i80286_selector_okay(cpustate,tmp) )
		{
			cpustate->ZeroVal = 0;
			addr=i80286_selector_to_address(cpustate,tmp);
			RegWord(ModRM)=ReadWord(addr);
		}
		else
		{
			cpustate->ZeroVal = 1;
		}
		break;
	case 6: /* clts */
		if (PM&&(CPL!=0)) i80286_trap2(cpustate,GENERAL_PROTECTION_FAULT);
		cpustate->msw=~8;
		break;
	default:
		i80286_trap2(cpustate,ILLEGAL_INSTRUCTION);
		break;
	}
}

static void PREFIX286(_arpl)(i8086_state *cpustate) /* 0x63 */
{
	if (PM)
	{
		UINT16 ModRM=FETCHOP, tmp=GetRMWord(ModRM), source=RegWord(ModRM);

		if ( i80286_selector_okay(cpustate,tmp) &&i80286_selector_okay(cpustate,source) &&((tmp&3)<(source&3)) )
		{
			cpustate->ZeroVal = 0;
			PutbackRMWord(ModRM, (tmp&~3)|(source&3));
		}
		else
		{
			cpustate->ZeroVal = 1;
		}
	}
	else
	{
		i80286_trap2(cpustate,ILLEGAL_INSTRUCTION);
	}
}

static void i80286_check_permission(i8086_state *cpustate, UINT8 check_seg, UINT16 offset, i80286_size size, i80286_operation operation)
{
	if (PM)
	{
		/* Is the segment physically present? */
		if ( ! IS_PRESENT( cpustate->rights[check_seg] ) )
			throw GENERAL_PROTECTION_FAULT;

		/* Would we go past the segment boundary? */
		if ( offset + size > cpustate->limit[check_seg] )
		{
			throw GENERAL_PROTECTION_FAULT;
		}

		switch(operation)
		{
		case I80286_READ:
			/* Is the segment readable? */
			if ( ! IS_READABLE( cpustate->rights[check_seg] ) )
				throw GENERAL_PROTECTION_FAULT;
			break;

		case I80286_WRITE:
			/* Is the segment writeable?  */
			if ( ! IS_WRITEABLE( cpustate->rights[check_seg] ) )
				throw GENERAL_PROTECTION_FAULT;
			break;

		case I80286_EXECUTE:
			/* TODO */
			break;
		}
		/* TODO: Mark segment as accessed? */
	}
}

