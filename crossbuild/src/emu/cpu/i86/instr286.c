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
#define WRITEABLE(a) ((a&0xa)==2)
#define READABLE(a) ( ((a&0xa)==0xa)|| ((a&8)==0) )

static void i80286_trap2(int number)
{
	i80286_interrupt(number);
}

static int i80286_selector_okay(UINT16 selector)
{
	if (selector&4) {
		return (selector&~7)<I.ldtr.limit;
	} else {
		return (selector&~7)<I.gdtr.limit;
	}
}

static offs_t i80286_selector_to_address(UINT16 selector)
{
	if (selector&4) {
		return I.ldtr.base+(selector&~7);
	} else {
		return I.gdtr.base+(selector&~7);
	}
}

static void i80286_data_descriptor(int reg, UINT16 selector)
{
	if (PM) {
		UINT16 help;
		/* selector format
           15..3 number/address in descriptor table
           2: 0 global, 1 local descriptor table
           1,0: requested privileg level
           must be higher or same as current privileg level in code selector */
		if (selector&4) { /* local descriptor table */
			if (selector>I.ldtr.limit) i80286_trap2(GENERAL_PROTECTION_FAULT);
			I.sregs[reg]=selector;
			I.limit[reg]=ReadWord(I.ldtr.base+(selector&~7));
			I.base[reg]=ReadWord(I.ldtr.base+(selector&~7)+2)
				|(ReadWord(I.ldtr.base+(selector&~7)+4)<<16);
			I.rights[reg]=I.base[reg]>>24;
			I.base[reg]&=0xffffff;
		} else { /* global descriptor table */
			if (!(selector&~7)||(selector>I.gdtr.limit)) i80286_trap2(GENERAL_PROTECTION_FAULT);
			I.sregs[reg]=selector;
			I.limit[reg]=ReadWord(I.gdtr.base+(selector&~7));
			I.base[reg]=ReadWord(I.gdtr.base+(selector&~7)+2);
			help=ReadWord(I.gdtr.base+(selector&~7)+4);
			I.rights[reg]=help>>8;
			I.base[reg]|=(help&0xff)<<16;
		}
	} else {
		I.sregs[reg]=selector;
		I.base[reg]=selector<<4;
	}
}

static void i80286_code_descriptor(UINT16 selector, UINT16 offset)
{
	UINT16 word1, word2, word3;
	if (PM) {
		/* selector format
           15..3 number/address in descriptor table
           2: 0 global, 1 local descriptor table
           1,0: requested privileg level
           must be higher or same as current privileg level in code selector */
		if (selector&4) { /* local descriptor table */
			if (selector>I.ldtr.limit) i80286_trap2(GENERAL_PROTECTION_FAULT);
			word1=ReadWord(I.ldtr.base+(selector&~7));
			word2=ReadWord(I.ldtr.base+(selector&~7)+2);
			word3=ReadWord(I.ldtr.base+(selector&~7)+4);
		} else { /* global descriptor table */
			if (!(selector&~7)||(selector>I.gdtr.limit)) i80286_trap2(GENERAL_PROTECTION_FAULT);
			word1=ReadWord(I.gdtr.base+(selector&~7));
			word2=ReadWord(I.gdtr.base+(selector&~7)+2);
			word3=ReadWord(I.gdtr.base+(selector&~7)+4);
		}
		if (word3&0x1000) {
			I.sregs[CS]=selector;
			I.limit[CS]=word1;
			I.base[CS]=word2|((word3&0xff)<<16);
			I.rights[CS]=word3>>8;
			I.pc=I.base[CS]+offset;
		} else { // systemdescriptor
			switch (word3&0xf00) {
			case 0x400: // call gate
				// word3&0x1f words to be copied from stack to stack
				i80286_data_descriptor(CS, word2);
				I.pc=I.base[CS]+word1;
				break;
			case 0x500: // task gate
				i80286_data_descriptor(CS, word2);
				I.pc=I.base[CS]+word1;
				break;
			case 0x600: // interrupt gate
				I.TF = I.IF = 0;
				i80286_data_descriptor(CS, word2);
				I.pc=I.base[CS]+word1;
				break;
			case 0x700: // trap gate
				i80286_data_descriptor(CS, word2);
				I.pc=I.base[CS]+word1;
				break;
			}
		}
	} else {
		I.sregs[CS]=selector;
		I.base[CS]=selector<<4;
		I.pc=I.base[CS]+offset;
	}
}

static void i80286_interrupt_descriptor(UINT16 number)
{
	UINT16 word1,word2,word3;
	if ((number<<3)>=I.idtr.limit) {
		;// go into shutdown mode
		return;
	}
	PREFIX(_pushf());
	PUSH(I.sregs[CS]);
	PUSH(I.pc - I.base[CS]);
	word1=ReadWord(I.idtr.base+(number<<3));
	word2=ReadWord(I.idtr.base+(number<<3)+2);
	word3=ReadWord(I.idtr.base+(number<<3)+4);
	switch (word3&0xf00) {
	case 0x500: // task gate
		i80286_data_descriptor(CS, word2);
		I.pc=I.base[CS]+word1;
		break;
	case 0x600: // interrupt gate
		I.TF = I.IF = 0;
		i80286_data_descriptor(CS, word2);
		I.pc=I.base[CS]+word1;
		break;
	case 0x700: // trap gate
		i80286_data_descriptor(CS, word2);
		I.pc=I.base[CS]+word1;
		break;
	}
}

static void PREFIX286(_0fpre)(void)
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
			if (!PM) i80286_trap2(ILLEGAL_INSTRUCTION);
			PutRMWord(ModRM, I.ldtr.sel);
			break;
		case 8: /* str */
			if (!PM) i80286_trap2(ILLEGAL_INSTRUCTION);
			PutRMWord(ModRM, I.tr.sel);
			break;
		case 0x10: /* lldt */
			if (!PM) i80286_trap2(ILLEGAL_INSTRUCTION);
			if (PM&&(CPL!=0)) i80286_trap2(GENERAL_PROTECTION_FAULT);
			I.ldtr.sel=GetRMWord(ModRM);
			if ((I.ldtr.sel&~7)>=I.gdtr.limit) i80286_trap2(GENERAL_PROTECTION_FAULT);
			I.ldtr.limit=ReadWord(I.gdtr.base+(I.ldtr.sel&~7));
			I.ldtr.base=ReadWord(I.gdtr.base+(I.ldtr.sel&~7)+2)
				|(ReadWord(I.gdtr.base+(I.ldtr.sel&~7)+4)<<16);
			I.ldtr.rights=I.ldtr.base>>24;
			I.ldtr.base&=0xffffff;
			break;
		case 0x18: /* ltr */
			if (!PM) i80286_trap2(ILLEGAL_INSTRUCTION);
			if (CPL!=0) i80286_trap2(GENERAL_PROTECTION_FAULT);
			I.tr.sel=GetRMWord(ModRM);
			if ((I.tr.sel&~7)>=I.gdtr.limit) i80286_trap2(GENERAL_PROTECTION_FAULT);
			I.tr.limit=ReadWord(I.gdtr.base+(I.tr.sel&~7));
			I.tr.base=ReadWord(I.gdtr.base+(I.tr.sel&~7)+2)
				|(ReadWord(I.gdtr.base+(I.tr.sel&~7)+4)<<16);
			I.tr.rights=I.tr.base>>24;
			I.tr.base&=0xffffff;
			break;
		case 0x20: /* verr */
			if (!PM) i80286_trap2(ILLEGAL_INSTRUCTION);
			tmp=GetRMWord(ModRM);
			if (tmp&4) {
				I.ZeroVal=( ((tmp&~7)<I.ldtr.limit)
							&& READABLE( ReadByte(I.ldtr.base+(tmp&~7)+5)) );
			} else {
				I.ZeroVal=( ((tmp&~7)<I.gdtr.limit)
							&& READABLE( ReadByte(I.gdtr.base+(tmp&~7)+5)) );
			}
			break;
		case 0x28: /* verw */
			if (!PM) i80286_trap2(ILLEGAL_INSTRUCTION);
			tmp=GetRMWord(ModRM);
			if (tmp&4) {
				I.ZeroVal=( ((tmp&~7)<I.ldtr.limit)
							&& WRITEABLE( ReadByte(I.ldtr.base+(tmp&~7)+5)) );
			} else {
				I.ZeroVal=( ((tmp&~7)<I.gdtr.limit)
							&& WRITEABLE( ReadByte(I.gdtr.base+(tmp&~7)+5)) );
			}
			break;
		default:
			i80286_trap2(ILLEGAL_INSTRUCTION);
			break;
		}
		break;
	case 1:
		/* lgdt, lldt in protected mode privilege level 0 required else common protection
           failure 0xd */
		ModRM = FETCHOP;
		switch (ModRM&0x38) {
		case 0: /* sgdt */
			PutRMWord(ModRM,I.gdtr.limit);
			PutRMWordOffset(2,I.gdtr.base&0xffff);
			PutRMByteOffset(4,I.gdtr.base>>16);
			break;
		case 8: /* sidt */
			PutRMWord(ModRM,I.idtr.limit);
			PutRMWordOffset(2,I.idtr.base&0xffff);
			PutRMByteOffset(4,I.idtr.base>>16);
			break;
		case 0x10: /* lgdt */
			if (PM&&(CPL!=0)) i80286_trap2(GENERAL_PROTECTION_FAULT);
			I.gdtr.limit=GetRMWord(ModRM);
			I.gdtr.base=GetRMWordOffset(2)|(GetRMByteOffset(4)<<16);
			break;
		case 0x18: /* lidt */
			if (PM&&(CPL!=0)) i80286_trap2(GENERAL_PROTECTION_FAULT);
			I.idtr.limit=GetRMWord(ModRM);
			I.idtr.base=GetRMWordOffset(2)|(GetRMByteOffset(4)<<16);
			break;
		case 0x20: /* smsw */
			PutRMWord(ModRM, I.msw);
			break;
		case 0x30: /* lmsw */
			if (PM&&(CPL!=0)) i80286_trap2(GENERAL_PROTECTION_FAULT);
			I.msw=(I.msw&1)|GetRMWord(ModRM);
			break;
		default:
			i80286_trap2(ILLEGAL_INSTRUCTION);
			break;
		}
		break;
	case 2: /* LAR */
		ModRM = FETCHOP;
		tmp=GetRMWord(ModRM);
		I.ZeroVal=i80286_selector_okay(tmp);
		if (I.ZeroVal) {
			RegWord(ModRM)=tmp;
		}
		break;
	case 3: /* LSL */
		if (!PM) i80286_trap2(ILLEGAL_INSTRUCTION);
		ModRM = FETCHOP;
		tmp=GetRMWord(ModRM);
		I.ZeroVal=i80286_selector_okay(tmp);
		if (I.ZeroVal) {
			addr=i80286_selector_to_address(tmp);
			RegWord(ModRM)=ReadWord(addr);
		}
		break;
	case 6: /* clts */
		if (PM&&(CPL!=0)) i80286_trap2(GENERAL_PROTECTION_FAULT);
		I.msw=~8;
		break;
	default:
		i80286_trap2(ILLEGAL_INSTRUCTION);
		break;
	}
}

static void PREFIX286(_arpl)(void) /* 0x63 */
{
	if (PM) {
		UINT16 ModRM=FETCHOP, tmp=GetRMWord(ModRM);

		I.ZeroVal=i80286_selector_okay(RegWord(ModRM))
			  &&i80286_selector_okay(RegWord(ModRM))
			  &&((tmp&3)<(RegWord(ModRM)&3));
		if (I.ZeroVal) PutbackRMWord(ModRM, (tmp&~3)|(RegWord(ModRM)&3));
	} else {
		i80286_trap2(ILLEGAL_INSTRUCTION);
	}
}
