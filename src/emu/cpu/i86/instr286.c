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
#define LIMIT(desc)  (desc[0]&0xffff)
#define BASE(desc)   ((desc[1]&0xffff)|((desc[2]&0xff)<<16))
#define RIGHTS(desc) ((desc[2]>>8)&0xff)

#define ACCESS(r)  (r&1)
#define SET_ACC(desc) (desc[2]|=0x100)
#define RW(r)      ((r>>1)&1)
#define READ(r)    ((r>>1)&1)
#define CONF(r)    ((r>>2)&1)
#define EXPDOWN(r) ((r>>2)&1)
#define CODE(r)    ((r>>3)&1)
#define SEGDESC(r) ((r>>4)&1)
#define DPL(r)     ((r>>5)&3)
#define PRES(r)    ((r>>7)&1)
#define GATE(r)    (r&31)
#define GATESEL(desc) (desc[1])
#define GATEOFF(desc) (desc[0])
#define GATECNT(desc) (desc[2]&31)

#define RPL(s)     (s&3)
#define IDX(s)     (s&(~7))
#define IDXTBL(s)  (s&(~3))
#define TBL(s)     (s&4)

#define TSSDESCIDLE 1
#define LDTDESC    2
#define TSSDESCBUSY 3
#define CALLGATE   4
#define TASKGATE   5
#define INTGATE    6
#define TRAPGATE   7

#define TSS_BACK 0
#define TSS_SP0  2
#define TSS_SS0  4
#define TSS_SP1  6
#define TSS_SS1  8
#define TSS_SP2  10
#define TSS_SS2  12
#define TSS_IP   14
#define TSS_FLAG 16
#define TSS_AX   18
#define TSS_CX   20
#define TSS_DX   22
#define TSS_BX   24
#define TSS_SP   26
#define TSS_BP   28
#define TSS_SI   30
#define TSS_DI   32
#define TSS_ES   34
#define TSS_CS   36
#define TSS_SS   38
#define TSS_DS   40
#define TSS_LDT  42

#define PMAX(a,b) ((a<b)?b:a)

// when a cpu reset happens on a AT the bios checks for 9 in byte 0xf
// of the nvram.  if yes, after init, it sets the stack pointer to the value in 0040:0067
// in the bios data segment then pops es and ds off that stack, does popa then a far ret.

static void i80286_trap2(i80286_state *cpustate,UINT32 error)
{
	int error_code = error & 0xffff;
	UINT16 number = error >> 16;
	if(error_code == 0xffff) error_code = -1;
	cpustate->pc = cpustate->prevpc;
	try {
		switch(number) {
		case DIVIDE_BY_ZERO:
		case INVALID_TSS:
		case SEG_NOT_PRESENT:
		case STACK_FAULT:
		case GENERAL_PROTECTION_FAULT:
			cpustate->trap_level++;
			if(cpustate->trap_level == 2) throw TRAP(DOUBLE_FAULT,0);
			if(cpustate->trap_level == 3) break;
			i80286_interrupt_descriptor(cpustate,number,1,error_code);
			break;
		case DOUBLE_FAULT:
			i80286_interrupt_descriptor(cpustate,number,1,0);
			break;
		default:
			i80286_interrupt_descriptor(cpustate,number,1,-1);
		}
	} catch(UINT32 e) { i80286_trap2(cpustate, e); }
	if(cpustate->trap_level == 3) 
		// this is supposed to triggered by support hardware
		// create a shutdown output line that causes a reset
		// NMI can wake processor without reset
		device_set_input_line(cpustate->device, INPUT_LINE_RESET, PULSE_LINE);
	cpustate->trap_level = 0;
}

static UINT32 i80286_selector_address(i80286_state *cpustate,UINT16 sel)
{
	UINT32 base;
	UINT16 limit;
	if(TBL(sel)) { base = cpustate->ldtr.base; limit = cpustate->ldtr.limit; }
	else { base = cpustate->gdtr.base; limit = cpustate->gdtr.limit; }
	return ((IDX(sel)>=limit)||!IDXTBL(sel)?-1:base+IDX(sel));
}

static int i80286_verify(i80286_state *cpustate, UINT16 selector, i80286_operation operation, UINT8 rights)
{
	if (!IDXTBL(selector)) return GENERAL_PROTECTION_FAULT;
	if (!SEGDESC(rights)) return GENERAL_PROTECTION_FAULT;
	
	switch(operation) {
		case I80286_READ:
			if (CODE(rights) && !READ(rights)) return GENERAL_PROTECTION_FAULT;
			break;
		case I80286_WRITE:
			if (CODE(rights) || !RW(rights)) return GENERAL_PROTECTION_FAULT;
			break;
		case I80286_EXECUTE:
			if (!CODE(rights)) return GENERAL_PROTECTION_FAULT;
			break;

	}
	return 0;
}

static void i80286_pop_seg(i80286_state *cpustate, int reg)
{
	UINT16 sel = ReadWord(cpustate->base[SS]+cpustate->regs.w[SP]);
	i80286_data_descriptor(cpustate, reg, sel);
	cpustate->regs.w[SP] += 2;
}

static void i80286_data_descriptor_full(i80286_state *cpustate,int reg, UINT16 selector, int cpl, UINT32 trap)
{
	if (PM) {
		UINT16 desc[3];
		UINT8 r;
		UINT32 addr;
		/* selector format
           15..3 number/address in descriptor table
           2: 0 global, 1 local descriptor table
           1,0: requested privileg level
           must be higher or same as current privileg level in code selector */
		if ((reg != SS) && !IDXTBL(selector)) {
			cpustate->sregs[reg]=0;
			cpustate->limit[reg]=0;
			cpustate->base[reg]=0;
			cpustate->rights[reg]=0;
			return;
		}

		if ((addr = i80286_selector_address(cpustate,selector)) == -1) throw trap;

		desc[0] = ReadWord(addr);
		desc[1] = ReadWord(addr+2);
		desc[2] = ReadWord(addr+4);
		r = RIGHTS(desc);
		if (!SEGDESC(r)) throw trap;
		if (reg == SS) {
			if (!IDXTBL(selector)) throw trap;
			if (DPL(r)!=cpl) throw trap;
			if (RPL(selector)!=cpl) throw trap;
			if (!RW(r) || CODE(r)) throw trap;
			if (!PRES(r)) throw TRAP(STACK_FAULT,(IDXTBL(selector)+(trap&1)));
		} else {
			if ((DPL(r) < PMAX(cpl,RPL(selector))) && (!CODE(r) || (CODE(r) && !CONF(r)))) throw trap;
			if (CODE(r) && !READ(r)) throw trap;
			if (!PRES(r)) throw TRAP(SEG_NOT_PRESENT,(IDXTBL(selector)+(trap&1)));
		}

		SET_ACC(desc);
		WriteWord(addr+4, desc[2]);
		cpustate->sregs[reg]=selector;
		cpustate->limit[reg]=LIMIT(desc);
		cpustate->base[reg]=BASE(desc);
		cpustate->rights[reg]=RIGHTS(desc);
	} else {
		cpustate->sregs[reg]=selector;
		cpustate->base[reg]=selector<<4;
	}
}

static void i80286_data_descriptor(i80286_state *cpustate, int reg, UINT16 selector)
{
	i80286_data_descriptor_full(cpustate, reg, selector, CPL, TRAP(GENERAL_PROTECTION_FAULT,IDXTBL(selector)));
}

static void i80286_code_descriptor(i80286_state *cpustate, UINT16 selector, UINT16 offset, int gate)
{
	if (PM) {
		UINT16 desc[3];
		UINT8 r;
		UINT32 addr;
		/* selector format
           15..3 number/address in descriptor table
           2: 0 global, 1 local descriptor table
           1,0: requested privileg level
           must be higher or same as current privileg level in code selector */
		if ((addr = i80286_selector_address(cpustate,selector)) == -1) throw TRAP(GENERAL_PROTECTION_FAULT,IDXTBL(selector));
		desc[0] = ReadWord(addr);
		desc[1] = ReadWord(addr+2);
		desc[2] = ReadWord(addr+4);
		r = RIGHTS(desc);

		if (SEGDESC(r)) {
			if (!CODE(r)) throw TRAP(GENERAL_PROTECTION_FAULT,IDXTBL(selector));
			if (CONF(r)) { if(DPL(r)>CPL) throw TRAP(GENERAL_PROTECTION_FAULT,IDXTBL(selector)); }
			else if ((RPL(selector)>CPL) || (DPL(r)!=CPL)) throw TRAP(GENERAL_PROTECTION_FAULT,IDXTBL(selector));
			
			SET_ACC(desc);
			WriteWord(addr+4, desc[2]);
			if (!PRES(r)) throw TRAP(SEG_NOT_PRESENT,IDXTBL(selector));  // this order is important
			if (offset > LIMIT(desc)) throw TRAP(GENERAL_PROTECTION_FAULT, 0);
			cpustate->sregs[CS]=IDXTBL(selector) | CPL;
			cpustate->limit[CS]=LIMIT(desc);
			cpustate->base[CS]=BASE(desc);
			cpustate->rights[CS]=RIGHTS(desc);
			cpustate->pc=cpustate->base[CS]+offset;
		} else { // systemdescriptor
			UINT16 gatedesc[3];
			UINT16 gatesel = GATESEL(desc);
			if (!gate) throw TRAP(GENERAL_PROTECTION_FAULT,IDXTBL(selector)); // can't ret through gate
			if (DPL(r) < PMAX(CPL,RPL(selector))) throw TRAP(GENERAL_PROTECTION_FAULT,IDXTBL(selector));
			if (!PRES(r)) throw TRAP(SEG_NOT_PRESENT, IDXTBL(selector));

			switch (GATE(r)) {
			case CALLGATE:
				if ((addr = i80286_selector_address(cpustate,gatesel)) == -1) throw TRAP(GENERAL_PROTECTION_FAULT,IDXTBL(gatesel));
				gatedesc[0] = ReadWord(addr);
				gatedesc[1] = ReadWord(addr+2);
				gatedesc[2] = ReadWord(addr+4);
				r = RIGHTS(gatedesc);
				if (!CODE(r)) throw TRAP(GENERAL_PROTECTION_FAULT,IDXTBL(gatesel));
				if (DPL(r)>CPL) throw TRAP(GENERAL_PROTECTION_FAULT,IDXTBL(gatesel));
				if (!PRES(r)) throw TRAP(SEG_NOT_PRESENT,IDXTBL(gatesel));
				if (GATEOFF(desc) > LIMIT(gatedesc)) throw TRAP(GENERAL_PROTECTION_FAULT,IDXTBL(gatesel));

				if (!CONF(r)&&(DPL(r)<CPL)) {  // inner call
					UINT16 tss_ss, tss_sp, oldss, oldsp;
					UINT32 oldstk;
					int i;
					if(gate == 1) throw TRAP(GENERAL_PROTECTION_FAULT,IDXTBL(gatesel)); // can't jmp to inner
					tss_ss = ReadWord(cpustate->tr.base+TSS_SS0+(DPL(r)*4));
					tss_sp = ReadWord(cpustate->tr.base+TSS_SP0+(DPL(r)*4));

					oldss = cpustate->sregs[SS];
					oldsp = cpustate->regs.w[SP];
					oldstk = cpustate->base[SS] + oldsp;
					i80286_data_descriptor_full(cpustate, SS, tss_ss, DPL(r), TRAP(INVALID_TSS,IDXTBL(tss_ss)));
					cpustate->regs.w[SP] = tss_sp;
					PUSH(oldss);
					PUSH(oldsp);
					for (i = GATECNT(desc)-1; i >= 0; i--)
						PUSH(ReadWord(oldstk+(i*2)));
				}
				cpustate->sregs[CS]=IDXTBL(gatesel) | DPL(r);
				cpustate->limit[CS]=LIMIT(gatedesc);
				cpustate->base[CS]=BASE(gatedesc);
				cpustate->rights[CS]=RIGHTS(gatedesc);
				cpustate->pc=(cpustate->base[CS]+GATEOFF(desc))&AMASK;
				break;
			case TASKGATE:
				i80286_data_descriptor(cpustate, CS, desc[1]);
				cpustate->pc=cpustate->base[CS]+desc[0];
				break;
			default:
				throw TRAP(GENERAL_PROTECTION_FAULT,IDXTBL(selector));
			}
		}
	} else {
		cpustate->sregs[CS]=selector;
		cpustate->base[CS]=selector<<4;
		cpustate->rights[CS]=0x9a;
		cpustate->limit[CS]=0xffff;
		cpustate->pc=(cpustate->base[CS]+offset)&AMASK;
	}
}

static void i80286_interrupt_descriptor(i80286_state *cpustate,UINT16 number, int hwint, int error)
{
	UINT16 desc[3], gatedesc[3], gatesel;
	UINT8 r;
	UINT32 addr;

	if(!PM) return PREFIX86(_interrupt)(cpustate, number);

	if ((number<<3)>=cpustate->idtr.limit) 
		throw TRAP(GENERAL_PROTECTION_FAULT,(number*8+2+(hwint&&1)));

	desc[0] = ReadWord(cpustate->idtr.base+(number<<3));
	desc[1] = ReadWord(cpustate->idtr.base+(number<<3)+2);
	desc[2] = ReadWord(cpustate->idtr.base+(number<<3)+4);
	r = RIGHTS(desc);
	if (!hwint && (DPL(r)<CPL)) throw TRAP(GENERAL_PROTECTION_FAULT,(number*8+2+(hwint&&1)));
	if (!PRES(r)) throw TRAP(SEG_NOT_PRESENT,(number*8+2+(hwint&&1)));

	switch (GATE(r)) {
	case TASKGATE:
		i80286_data_descriptor(cpustate, CS, desc[1]);
		cpustate->pc=cpustate->base[CS]+desc[0];
		break;
	case INTGATE:
	case TRAPGATE:
		gatesel = GATESEL(desc);
		if ((addr = i80286_selector_address(cpustate,gatesel)) == -1) throw TRAP(GENERAL_PROTECTION_FAULT,IDXTBL(gatesel));
	 	gatedesc[0] = ReadWord(addr);
		gatedesc[1] = ReadWord(addr+2);
		gatedesc[2] = ReadWord(addr+4);
		r = RIGHTS(gatedesc);
		if (!CODE(r)) throw TRAP(GENERAL_PROTECTION_FAULT,(IDXTBL(gatesel)+(hwint&&1)));
		if (DPL(r)>CPL) throw TRAP(GENERAL_PROTECTION_FAULT,(IDXTBL(gatesel)+(hwint&&1)));
		if (!PRES(r)) throw TRAP(SEG_NOT_PRESENT,(IDXTBL(gatesel)+(hwint&&1)));
		if (GATEOFF(desc) > LIMIT(gatedesc)) throw TRAP(GENERAL_PROTECTION_FAULT,(IDXTBL(gatesel)+(hwint&&1)));

		if (!CONF(r)&&(DPL(r)<CPL)) {  // inner call
			UINT16 tss_ss, tss_sp, oldss, oldsp;
			tss_ss = ReadWord(cpustate->tr.base+TSS_SS0+(DPL(r)*4));
			tss_sp = ReadWord(cpustate->tr.base+TSS_SP0+(DPL(r)*4));

			oldss = cpustate->sregs[SS];
			oldsp = cpustate->regs.w[SP];
			i80286_data_descriptor_full(cpustate, SS, tss_ss, DPL(r), TRAP(INVALID_TSS,(IDXTBL(tss_ss)+(hwint&&1))));
			cpustate->regs.w[SP] = tss_sp;
			PUSH(oldss);
			PUSH(oldsp);
		} 
		PREFIX(_pushf(cpustate));
		PUSH(cpustate->sregs[CS]);
		PUSH(cpustate->pc-cpustate->base[CS]);
		if((hwint == 1) && (error != -1)) PUSH(error);
		cpustate->sregs[CS]=IDXTBL(gatesel) | DPL(r);
		cpustate->limit[CS]=LIMIT(gatedesc);
		cpustate->base[CS]=BASE(gatedesc);
		cpustate->rights[CS]=RIGHTS(gatedesc);
		cpustate->pc=(cpustate->base[CS]+GATEOFF(desc))&AMASK;
		cpustate->TF = 0;
		if (GATE(RIGHTS(desc)) == INTGATE) cpustate->IF = 0;
		CHANGE_PC(cpustate->pc);
		break;
	default:
		throw TRAP(GENERAL_PROTECTION_FAULT,(number*8+2+(hwint&&1)));
	}
}

static void PREFIX286(_0fpre)(i8086_state *cpustate)
{
	unsigned next = FETCHOP;
	UINT16 ModRM, desc[3];
	UINT16 tmp, msw, sel;
	UINT8 r;
	UINT32 addr;

	switch (next) {
	case 0:
		if (!PM) throw TRAP(ILLEGAL_INSTRUCTION,-1);
		ModRM=FETCHOP;
		switch (ModRM&0x38) {
		case 0: /* sldt */
			PutRMWord(ModRM, cpustate->ldtr.sel);
			break;
		case 8: /* str */
			PutRMWord(ModRM, cpustate->tr.sel);
			break;
		case 0x10: /* lldt */
			sel=GetRMWord(ModRM);
			if (TBL(sel)) throw TRAP(GENERAL_PROTECTION_FAULT,IDXTBL(sel));
			if (IDXTBL(sel)) {
				if (IDX(sel)>=cpustate->gdtr.limit) throw TRAP(GENERAL_PROTECTION_FAULT,IDXTBL(sel));
				addr = cpustate->gdtr.base + IDX(sel);
				desc[0] = ReadWord(addr);
				desc[1] = ReadWord(addr+2);
				desc[2] = ReadWord(addr+4);
				r = RIGHTS(desc);
				if (GATE(r) != LDTDESC) throw TRAP(GENERAL_PROTECTION_FAULT,IDXTBL(sel));
				if (!PRES(r)) TRAP(SEG_NOT_PRESENT,IDXTBL(sel));
			} else {
				desc[0] = 0;
				desc[1] = 0;
				desc[2] = 0;
			}
			cpustate->ldtr.sel=sel;
			cpustate->ldtr.limit=LIMIT(desc);
			cpustate->ldtr.base=BASE(desc);
			cpustate->ldtr.rights=RIGHTS(desc);
			break;
		case 0x18: /* ltr */
			sel=GetRMWord(ModRM);
			if ((addr = i80286_selector_address(cpustate,sel)) == -1) throw TRAP(GENERAL_PROTECTION_FAULT,IDXTBL(sel));
			desc[0] = ReadWord(addr);
			desc[1] = ReadWord(addr+2);
			desc[2] = ReadWord(addr+4);
			r = RIGHTS(desc);
			if (GATE(r) != TSSDESCIDLE) throw TRAP(GENERAL_PROTECTION_FAULT,IDXTBL(sel));
			if (!PRES(r)) TRAP(SEG_NOT_PRESENT,IDXTBL(sel));
			cpustate->tr.sel=sel;
			cpustate->tr.limit=LIMIT(desc);
			cpustate->tr.base=BASE(desc);
			cpustate->tr.rights=RIGHTS(desc);
			break;
		case 0x20: /* verr */
			tmp=GetRMWord(ModRM);
			if ((addr = i80286_selector_address(cpustate,tmp)) == -1) cpustate->ZeroVal = 1;
			else {
				desc[2] = ReadWord(addr+4);
				r = RIGHTS(desc);
				cpustate->ZeroVal = i80286_verify(cpustate, tmp, I80286_READ, RIGHTS(desc));
				cpustate->ZeroVal = cpustate->ZeroVal || (CODE(r) && CONF(r) ? 0 : (DPL(r)<PMAX(RPL(tmp),CPL)));
			}
			break;
		case 0x28: /* verw */
			tmp=GetRMWord(ModRM);
			if ((addr = i80286_selector_address(cpustate,tmp)) == -1) cpustate->ZeroVal = 1;
			else {
				desc[2] = ReadWord(addr+4);
				r = RIGHTS(desc);
				cpustate->ZeroVal = i80286_verify(cpustate, tmp, I80286_WRITE, RIGHTS(desc));
			}
			break;
		default:
			throw TRAP(ILLEGAL_INSTRUCTION,-1);
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
			PutRMWordOffset(4,0xff00|cpustate->gdtr.base>>16);
			break;
		case 8: /* sidt */
			PutRMWord(ModRM,cpustate->idtr.limit);
			PutRMWordOffset(2,cpustate->idtr.base&0xffff);
			PutRMWordOffset(4,0xff00|cpustate->idtr.base>>16);
			break;
		case 0x10: /* lgdt */
			if (PM&&(CPL!=0)) throw TRAP(GENERAL_PROTECTION_FAULT,0);
			cpustate->gdtr.limit=GetRMWord(ModRM);
			cpustate->gdtr.base=GetRMWordOffset(2)|(GetRMByteOffset(4)<<16);
			break;
		case 0x18: /* lidt */
			if (PM&&(CPL!=0)) throw TRAP(GENERAL_PROTECTION_FAULT,0);
			cpustate->idtr.limit=GetRMWord(ModRM);
			cpustate->idtr.base=GetRMWordOffset(2)|(GetRMByteOffset(4)<<16);
			break;
		case 0x20: /* smsw */
			PutRMWord(ModRM, cpustate->msw);
			break;
		case 0x30: /* lmsw */
			if (PM&&(CPL!=0)) throw TRAP(GENERAL_PROTECTION_FAULT,0);
			msw = GetRMWord(ModRM);
			if (!PM&&(msw&1)) cpustate->sregs[CS] = IDX(cpustate->sregs[CS]); // cheat and set cpl to 0
			cpustate->msw=(cpustate->msw&1)|msw;
			break;
		default:
			throw TRAP(ILLEGAL_INSTRUCTION,-1);
			break;
		}
		break;
	case 2: /* LAR */
		if (!PM) throw TRAP(ILLEGAL_INSTRUCTION,-1);
		ModRM = FETCHOP;
		tmp=GetRMWord(ModRM);
		if ((addr = i80286_selector_address(cpustate,tmp)) == -1) cpustate->ZeroVal = 1;
		else {
			desc[2] = ReadWord(addr+4);
			r = RIGHTS(desc);
			if (DPL(r)>=PMAX(RPL(tmp),CPL)) {
				cpustate->ZeroVal = 0;
				// rights are expected to be in upper byte
				RegWord(ModRM) = r << 8;
			}
			else
				cpustate->ZeroVal = 1;
		}
		break;
	case 3: /* LSL */
		if (!PM) throw TRAP(ILLEGAL_INSTRUCTION,-1);
		ModRM = FETCHOP;
		tmp=GetRMWord(ModRM);
		if ((addr = i80286_selector_address(cpustate,tmp)) == -1) cpustate->ZeroVal = 1;
		else {
			desc[2] = ReadWord(addr+4);
			r = RIGHTS(desc);
			if (DPL(r)>=PMAX(RPL(tmp),CPL)) {
				cpustate->ZeroVal = 0;
				RegWord(ModRM) = ReadWord(addr);
			}
			else
				cpustate->ZeroVal = 1;
		}
		break;
	case 5: /* loadall */
		if (PM&&(CPL!=0)) throw TRAP(GENERAL_PROTECTION_FAULT,0);
		cpustate->msw =        (cpustate->msw&1)|ReadWord(0x806);
		cpustate->tr.sel =     ReadWord(0x816);
		tmp = 	               ReadWord(0x818);
		ExpandFlags(tmp);
		cpustate->flags = tmp;
		cpustate->flags = CompressFlags();
		cpustate->pc =         ReadWord(0x81a);
		cpustate->ldtr.sel =   ReadWord(0x81c);
		cpustate->sregs[DS] =  ReadWord(0x81e);
		cpustate->sregs[SS] =  ReadWord(0x820);
		cpustate->sregs[CS] =  ReadWord(0x822);
		cpustate->sregs[ES] =  ReadWord(0x824);
		cpustate->regs.w[DI] = ReadWord(0x826);
		cpustate->regs.w[SI] = ReadWord(0x828);
		cpustate->regs.w[BP] = ReadWord(0x82a);
		cpustate->regs.w[SP] = ReadWord(0x82c);
		cpustate->regs.w[BX] = ReadWord(0x82e);
		cpustate->regs.w[DX] = ReadWord(0x830);
		cpustate->regs.w[CX] = ReadWord(0x832);
		cpustate->regs.w[AX] = ReadWord(0x834);
		UINT16 desc[3];
// loadall uses base-rights-limit order
#define LOADDESC(addr, sreg) {  desc[1] = ReadWord(addr); desc[2] = ReadWord(addr+2); desc[0] = ReadWord(addr+4); \
				cpustate->base[sreg] = BASE(desc); cpustate->rights[sreg] = RIGHTS(desc); \
				cpustate->limit[sreg] = LIMIT(desc); }
		LOADDESC(0x836, ES);
		LOADDESC(0x83C, CS);
		LOADDESC(0x842, SS);
		LOADDESC(0x848, DS);
#undef LOADDESC
// void cast supresses warning
#define LOADDESC(addr, reg, r) { desc[1] = ReadWord(addr); desc[2] = ReadWord(addr+2); desc[0] = ReadWord(addr+4); \
				cpustate->reg.base = BASE(desc); (void)(r); cpustate->reg.limit = LIMIT(desc); }
		LOADDESC(0x84e, gdtr, 1);
		LOADDESC(0x854, ldtr, cpustate->ldtr.rights = RIGHTS(desc));
		LOADDESC(0x85a, idtr, 1);
		LOADDESC(0x860, tr,  cpustate->tr.rights = RIGHTS(desc));
#undef LOADDESC
		cpustate->pc = (cpustate->pc + cpustate->base[CS]) & AMASK;
		CHANGE_PC(cpustate->pc);
		break;

	case 6: /* clts */
		if (PM&&(CPL!=0)) throw TRAP(GENERAL_PROTECTION_FAULT,0);
		cpustate->msw&=~8;
		break;
	default:
		throw TRAP(ILLEGAL_INSTRUCTION,-1);
		break;
	}
}

static void PREFIX286(_arpl)(i8086_state *cpustate) /* 0x63 */
{
	UINT16 ModRM, tmp, source;
	if (!PM) throw TRAP(ILLEGAL_INSTRUCTION,-1);

	ModRM=FETCHOP;
	tmp=GetRMWord(ModRM);
	source=RegWord(ModRM);

	if (RPL(tmp)<RPL(source))
	{
		cpustate->ZeroVal = 0;
		PutbackRMWord(ModRM, IDXTBL(tmp)|RPL(source));
	}
	else
		cpustate->ZeroVal = 1;
}

static void i80286_load_flags(i8086_state *cpustate, int flags, int cpl)
{
	if(PM && cpl) {
		int mask = 0xf000;
		if(cpl>IOPL) mask |= 0x200;
		flags &= ~mask;
		flags |= (cpustate->flags & mask);
	}
	else if(!PM) (flags &= ~0xf000);
	ExpandFlags(flags);
	cpustate->flags = flags;
	cpustate->flags = CompressFlags();
	
	if (cpustate->TF) PREFIX(_trap)(cpustate);
	/* if the IF is set, and an interrupt is pending, signal an interrupt */
	if (cpustate->IF && cpustate->irq_state)
		i80286_interrupt_descriptor(cpustate, (*cpustate->irq_callback)(cpustate->device, 0), 2, -1);
}

static void PREFIX286(_popf)(i8086_state *cpustate)
{
	UINT16 flags;
	POP(flags);
	ICOUNT -= timing.popf;
	i80286_load_flags(cpustate, flags, CPL);
}

static UINT16 i80286_far_return(i8086_state *cpustate, int iret, int bytes)
{
	UINT16 sel, off, flags = 0;
	int spaddr;
	// must be restartable
	spaddr = (cpustate->base[SS] + cpustate->regs.w[SP]) & AMASK;
	off = ReadWord(spaddr);
	sel = ReadWord(spaddr+2);
	if(iret) flags = ReadWord(spaddr+4);

	if(PM) {
		UINT16 desc[3], newsp, newss;
		int addr, r;

		if (TBL(sel)) { /* local descriptor table */
			if (sel>cpustate->ldtr.limit) throw TRAP(GENERAL_PROTECTION_FAULT,IDXTBL(sel));
			addr = cpustate->ldtr.base+IDX(sel);
		} else { /* global descriptor table */
			if (!IDX(sel)||(sel>cpustate->gdtr.limit)) throw TRAP(GENERAL_PROTECTION_FAULT,IDXTBL(sel));
			addr = cpustate->gdtr.base+IDX(sel);
		}
		if (RPL(sel)<CPL) throw TRAP(GENERAL_PROTECTION_FAULT,IDXTBL(sel));
		desc[0] = ReadWord(addr);
		desc[1] = ReadWord(addr+2);
		desc[2] = ReadWord(addr+4);
		r = RIGHTS(desc);
		if (!CODE(r) || !SEGDESC(r)) throw TRAP(GENERAL_PROTECTION_FAULT,IDXTBL(sel));
		if (CONF(r)) { if(DPL(r)>RPL(sel)) throw TRAP(GENERAL_PROTECTION_FAULT,IDXTBL(sel)); }
		else if (DPL(r)!=RPL(sel)) throw TRAP(GENERAL_PROTECTION_FAULT,IDXTBL(sel));

		SET_ACC(desc);
		WriteWord(addr+4, desc[2]);
		if (!PRES(r)) throw TRAP(SEG_NOT_PRESENT,IDXTBL(sel));
		if (off > LIMIT(desc)) throw TRAP(GENERAL_PROTECTION_FAULT, IDXTBL(sel));
		if (CPL<RPL(sel)) {
			newsp = ReadWord(spaddr+((iret?6:4)+bytes));
			newss = ReadWord(spaddr+((iret?8:6)+bytes));
			i80286_data_descriptor_full(cpustate, SS, newss, RPL(sel), TRAP(GENERAL_PROTECTION_FAULT,IDXTBL(newss)));
			cpustate->regs.w[SP] = newsp + bytes;
		} else cpustate->regs.w[SP] += (iret?6:4) + bytes;
		cpustate->sregs[CS]=sel;
		cpustate->limit[CS]=LIMIT(desc);
		cpustate->base[CS]=BASE(desc);
		cpustate->rights[CS]=RIGHTS(desc);
		cpustate->pc=(cpustate->base[CS]+off)&AMASK ;

		// docs say check rpl but windows doesn't like it
		if (i80286_verify(cpustate, cpustate->sregs[DS], I80286_READ, cpustate->rights[DS]) && (DPL(cpustate->rights[DS]) < CPL))
			i80286_data_descriptor(cpustate, DS, 0);

		if (i80286_verify(cpustate, cpustate->sregs[ES], I80286_READ, cpustate->rights[ES]) && (DPL(cpustate->rights[ES]) < CPL))
			i80286_data_descriptor(cpustate, ES, 0);
	} else {
		cpustate->regs.w[SP] += (iret?6:4) + bytes;
		cpustate->sregs[CS]=sel;
		cpustate->base[CS]=sel<<4;
		cpustate->rights[CS]=0x93;
		cpustate->limit[CS]=0xffff;
		cpustate->pc=(cpustate->base[CS]+off)&AMASK;
	}
	CHANGE_PC(cpustate->pc);

	return flags;
}

static void PREFIX286(_iret)(i8086_state *cpustate)
{
	int oldcpl = (PM)?CPL:0;
	UINT16 flags = i80286_far_return(cpustate, 1, 0);
	ICOUNT -= timing.iret;
	i80286_load_flags(cpustate, flags, oldcpl);
}

static void PREFIX286(_retf_d16)(i8086_state *cpustate)
{
	unsigned count = FETCH;
	count += FETCH << 8;
	i80286_far_return(cpustate, 0, count);
	ICOUNT -= timing.ret_far_imm;
}

static void PREFIX286(_retf)(i8086_state *cpustate)
{
	i80286_far_return(cpustate, 0, 0);
	ICOUNT -= timing.ret_far;
}

static void i80286_check_permission(i8086_state *cpustate, UINT8 check_seg, UINT16 offset, i80286_size size, i80286_operation operation)
{
	int trap = 0;
	if (PM) {
		trap = i80286_verify(cpustate, cpustate->sregs[check_seg], operation, cpustate->rights[check_seg]);
		if (!EXPDOWN(cpustate->rights[check_seg]) && ((offset+size-1) > cpustate->limit[check_seg])) trap = GENERAL_PROTECTION_FAULT;
		if (EXPDOWN(cpustate->rights[check_seg]) && (offset < cpustate->limit[check_seg])) trap = GENERAL_PROTECTION_FAULT;

		if (trap) throw TRAP(trap, IDXTBL(cpustate->sregs[check_seg]));
	}
}

