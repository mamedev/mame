// license:BSD-3-Clause
// copyright-holders:Ville Linde, Barry Rodewald, Carl, Philip Bennett

uint32_t i386_device::i386_load_protected_mode_segment(I386_SREG *seg, uint64_t *desc )
{
	uint32_t v1,v2;
	uint32_t base, limit;
	int entry;

	if(!seg->selector)
	{
		seg->flags = 0;
		seg->base = 0;
		seg->limit = 0;
		seg->d = 0;
		seg->valid = false;
		return 0;
	}

	if ( seg->selector & 0x4 )
	{
		base = m_ldtr.base;
		limit = m_ldtr.limit;
	} else {
		base = m_gdtr.base;
		limit = m_gdtr.limit;
	}

	entry = seg->selector & ~0x7;
	if (limit == 0 || entry + 7 > limit)
		return 0;

	v1 = READ32PL(base + entry, 0);
	v2 = READ32PL(base + entry + 4, 0);

	seg->flags = (v2 >> 8) & 0xf0ff;
	seg->base = (v2 & 0xff000000) | ((v2 & 0xff) << 16) | ((v1 >> 16) & 0xffff);
	seg->limit = (v2 & 0xf0000) | (v1 & 0xffff);
	if (seg->flags & 0x8000)
		seg->limit = (seg->limit << 12) | 0xfff;
	seg->d = (seg->flags & 0x4000) ? 1 : 0;
	seg->valid = true;

	if(desc)
		*desc = ((uint64_t)v2<<32)|v1;
	return 1;
}

void i386_device::i386_load_call_gate(I386_CALL_GATE *gate)
{
	uint32_t v1,v2;
	uint32_t base,limit;
	int entry;

	if ( gate->segment & 0x4 )
	{
		base = m_ldtr.base;
		limit = m_ldtr.limit;
	} else {
		base = m_gdtr.base;
		limit = m_gdtr.limit;
	}

	entry = gate->segment & ~0x7;
	if (limit == 0 || entry + 7 > limit)
		return;

	v1 = READ32PL(base + entry, 0);
	v2 = READ32PL(base + entry + 4, 0);

	/* Note that for task gates, offset and dword_count are not used */
	gate->selector = (v1 >> 16) & 0xffff;
	gate->offset = (v1 & 0x0000ffff) | (v2 & 0xffff0000);
	gate->ar = (v2 >> 8) & 0xff;
	gate->dword_count = v2 & 0x001f;
	gate->present = (gate->ar >> 7) & 0x01;
	gate->dpl = (gate->ar >> 5) & 0x03;
}

void i386_device::i386_set_descriptor_accessed(uint16_t selector)
{
	// assume the selector is valid, we don't need to check it again
	uint32_t base, addr;
	uint8_t rights;
	if(!(selector & ~3))
		return;

	if ( selector & 0x4 )
		base = m_ldtr.base;
	else
		base = m_gdtr.base;

	addr = base + (selector & ~7) + 5;
	i386_translate_address(TR_READ, false, &addr, nullptr);
	rights = m_program->read_byte(addr);
	// Should a fault be thrown if the table is read only?
	m_program->write_byte(addr, rights | 1);
}

void i386_device::i386_load_segment_descriptor(int segment )
{
	if (PROTECTED_MODE)
	{
		uint16_t old_flags = m_sreg[segment].flags;
		if (!V8086_MODE)
		{
			i386_load_protected_mode_segment(&m_sreg[segment], nullptr);
			if (m_sreg[segment].selector)
			{
				i386_set_descriptor_accessed(m_sreg[segment].selector);
				m_sreg[segment].flags |= 0x0001;
			}
		}
		else
		{
			m_sreg[segment].base = m_sreg[segment].selector << 4;
			m_sreg[segment].limit = 0xffff;
			m_sreg[segment].flags = 0x00f3;
			m_sreg[segment].d = 0;
			m_sreg[segment].valid = true;
		}
		if (segment == CS && m_sreg[segment].flags != old_flags)
			debugger_privilege_hook();
	}
	else
	{
		m_sreg[segment].base = m_sreg[segment].selector << 4;
		m_sreg[segment].d = 0;
		m_sreg[segment].valid = true;

		if (segment == CS)
		{
			if (!m_performed_intersegment_jump)
				m_sreg[segment].base |= 0xfff00000;
			if (m_cpu_version < 0x500)
				m_sreg[segment].flags = 0x93;
		}
	}
}

/* Retrieves the stack selector located in the current TSS */
uint32_t i386_device::i386_get_stack_segment(uint8_t privilege)
{
	uint32_t ret;
	if(privilege >= 3)
		return 0;

	if(m_task.flags & 8)
		ret = READ32PL((m_task.base+8) + (8*privilege), 0);
	else
		ret = READ16PL((m_task.base+4) + (4*privilege), 0);

	return ret;
}

/* Retrieves the stack pointer located in the current TSS */
uint32_t i386_device::i386_get_stack_ptr(uint8_t privilege)
{
	uint32_t ret;
	if(privilege >= 3)
		return 0;

	if(m_task.flags & 8)
		ret = READ32PL((m_task.base+4) + (8*privilege), 0);
	else
		ret = READ16PL((m_task.base+2) + (4*privilege), 0);

	return ret;
}

/* Check segment register for validity when changing privilege level after an RETF */
void i386_device::i386_check_sreg_validity(int reg)
{
	uint16_t selector = m_sreg[reg].selector;
	uint8_t CPL = m_CPL;
	uint8_t DPL,RPL;
	I386_SREG desc;
	int invalid;

	memset(&desc, 0, sizeof(desc));
	desc.selector = selector;
	i386_load_protected_mode_segment(&desc,nullptr);
	DPL = (desc.flags >> 5) & 0x03;  // descriptor privilege level
	RPL = selector & 0x03;

	/* Must be within the relevant descriptor table limits */
	if(selector & 0x04)
	{
		if((selector & ~0x07) > m_ldtr.limit)
			invalid = 1;
	}
	else
	{
		if((selector & ~0x07) > m_gdtr.limit)
			invalid = 1;
	}

	/* Must be either a data or readable code segment */
	if(((desc.flags & 0x0018) == 0x0018 && (desc.flags & 0x0002)) || (desc.flags & 0x0018) == 0x0010)
		invalid = 0;
	else
		invalid = 1;

	/* If a data segment or non-conforming code segment, then either DPL >= CPL or DPL >= RPL */
	if(((desc.flags & 0x0018) == 0x0018 && (desc.flags & 0x0004) == 0) || (desc.flags & 0x0018) == 0x0010)
	{
		if((DPL < CPL) || (DPL < RPL))
			invalid = 1;
	}

	/* if segment is invalid, then segment register is nulled */
	if(invalid != 0)
	{
		m_sreg[reg].selector = 0;
		i386_load_segment_descriptor(reg);
	}
}

int i386_device::i386_limit_check(int seg, uint32_t offset, int size)
{
	if(PROTECTED_MODE && !V8086_MODE)
	{
		if((m_sreg[seg].flags & 0x0018) == 0x0010 && m_sreg[seg].flags & 0x0004) // if expand-down data segment
		{
			// compare if greater then 0xffffffff when we're passed the access size
			if((offset <= m_sreg[seg].limit) || ((m_sreg[seg].d)?0:(offset > 0xffff)))
			{
				LOGMASKED(LOG_LIMIT_CHECK, "Limit check at 0x%08x failed. Segment %04x, limit %08x, offset %08x (expand-down)\n",m_pc,m_sreg[seg].selector,m_sreg[seg].limit,offset);
				return 1;
			}
		}
		else
		{
			if((offset + size - 1) > m_sreg[seg].limit)
			{
				LOGMASKED(LOG_LIMIT_CHECK, "Limit check at 0x%08x failed. Segment %04x, limit %08x, offset %08x\n",m_pc,m_sreg[seg].selector,m_sreg[seg].limit,offset);
				//machine().debug_break();
				return 1;
			}
		}
	}
	return 0;
}

void i386_device::i386_sreg_load(uint16_t selector, uint8_t reg, bool *fault)
{
	// Checks done when MOV changes a segment register in protected mode
	uint8_t CPL,RPL,DPL;

	CPL = m_CPL;
	RPL = selector & 0x0003;

	if(!PROTECTED_MODE || V8086_MODE)
	{
		m_sreg[reg].selector = selector;
		i386_load_segment_descriptor(reg);
		if(fault) *fault = false;
		return;
	}

	if(fault) *fault = true;
	if(reg == SS)
	{
		I386_SREG stack;

		memset(&stack, 0, sizeof(stack));
		stack.selector = selector;
		i386_load_protected_mode_segment(&stack,nullptr);
		DPL = (stack.flags >> 5) & 0x03;

		if((selector & ~0x0003) == 0)
		{
			LOGMASKED(LOG_PM_FAULT_GP, "SReg Load (%08x): Selector is null.\n",m_pc);
			FAULT(FAULT_GP,0)
		}
		if(selector & 0x0004)  // LDT
		{
			if((selector & ~0x0007) > m_ldtr.limit)
			{
				LOGMASKED(LOG_PM_FAULT_GP, "SReg Load (%08x): Selector is out of LDT bounds.\n",m_pc);
				FAULT(FAULT_GP,selector & ~0x03)
			}
		}
		else  // GDT
		{
			if((selector & ~0x0007) > m_gdtr.limit)
			{
				LOGMASKED(LOG_PM_FAULT_GP, "SReg Load (%08x): Selector is out of GDT bounds.\n",m_pc);
				FAULT(FAULT_GP,selector & ~0x03)
			}
		}
		if (RPL != CPL)
		{
			LOGMASKED(LOG_PM_FAULT_GP, "SReg Load (%08x): Selector RPL does not equal CPL.\n",m_pc);
			FAULT(FAULT_GP,selector & ~0x03)
		}
		if(((stack.flags & 0x0018) != 0x10) && (stack.flags & 0x0002) != 0)
		{
			LOGMASKED(LOG_PM_FAULT_GP, "SReg Load (%08x): Segment is not a writable data segment.\n",m_pc);
			FAULT(FAULT_GP,selector & ~0x03)
		}
		if(DPL != CPL)
		{
			LOGMASKED(LOG_PM_FAULT_GP, "SReg Load (%08x): Segment DPL does not equal CPL.\n",m_pc);
			FAULT(FAULT_GP,selector & ~0x03)
		}
		if(!(stack.flags & 0x0080))
		{
			LOGMASKED(LOG_PM_FAULT_SS, "SReg Load (%08x): Segment is not present.\n",m_pc);
			FAULT(FAULT_SS,selector & ~0x03)
		}
	}
	if(reg == DS || reg == ES || reg == FS || reg == GS)
	{
		I386_SREG desc;

		if((selector & ~0x0003) == 0)
		{
			m_sreg[reg].selector = selector;
			i386_load_segment_descriptor(reg );
			if(fault) *fault = false;
			return;
		}

		memset(&desc, 0, sizeof(desc));
		desc.selector = selector;
		i386_load_protected_mode_segment(&desc,nullptr);
		DPL = (desc.flags >> 5) & 0x03;

		if(selector & 0x0004)  // LDT
		{
			if((selector & ~0x0007) > m_ldtr.limit)
			{
				LOGMASKED(LOG_PM_FAULT_GP, "SReg Load (%08x): Selector is out of LDT bounds.\n",m_pc);
				FAULT(FAULT_GP,selector & ~0x03)
			}
		}
		else  // GDT
		{
			if((selector & ~0x0007) > m_gdtr.limit)
			{
				LOGMASKED(LOG_PM_FAULT_GP, "SReg Load (%08x): Selector is out of GDT bounds.\n",m_pc);
				FAULT(FAULT_GP,selector & ~0x03)
			}
		}
		if((desc.flags & 0x0018) != 0x10)
		{
			if((((desc.flags & 0x0002) != 0) && ((desc.flags & 0x0018) != 0x18)) || !(desc.flags & 0x10))
			{
				LOGMASKED(LOG_PM_FAULT_GP, "SReg Load (%08x): Segment is not a data segment or readable code segment.\n",m_pc);
				FAULT(FAULT_GP,selector & ~0x03)
			}
		}
		if(((desc.flags & 0x0018) == 0x10) || ((!(desc.flags & 0x0004)) && ((desc.flags & 0x0018) == 0x18)))
		{
			// if data or non-conforming code segment
			if((RPL > DPL) || (CPL > DPL))
			{
				LOGMASKED(LOG_PM_FAULT_GP, "SReg Load (%08x): Selector RPL or CPL is not less or equal to segment DPL.\n",m_pc);
				FAULT(FAULT_GP,selector & ~0x03)
			}
		}
		if(!(desc.flags & 0x0080))
		{
			LOGMASKED(LOG_PM_FAULT_NP, "SReg Load (%08x): Segment is not present.\n",m_pc);
			FAULT(FAULT_NP,selector & ~0x03)
		}
	}

	m_sreg[reg].selector = selector;
	i386_load_segment_descriptor(reg );
	if(fault) *fault = false;
}

void i386_device::i386_trap(int irq, int irq_gate, int trap_level)
{
	/*  I386 Interrupts/Traps/Faults:
	 *
	 *  0x00    Divide by zero
	 *  0x01    Debug exception
	 *  0x02    NMI
	 *  0x03    Int3
	 *  0x04    Overflow
	 *  0x05    Array bounds check
	 *  0x06    Illegal Opcode
	 *  0x07    FPU not available
	 *  0x08    Double fault
	 *  0x09    Coprocessor segment overrun
	 *  0x0a    Invalid task state
	 *  0x0b    Segment not present
	 *  0x0c    Stack exception
	 *  0x0d    General Protection Fault
	 *  0x0e    Page fault
	 *  0x0f    Reserved
	 *  0x10    Coprocessor error
	 */
	uint32_t v1, v2;
	uint32_t offset, oldflags = get_flags();
	uint16_t segment;
	int entry = irq * (PROTECTED_MODE ? 8 : 4);
	int SetRPL;
	m_lock = false;

	debugger_exception_hook(irq);

	m_cycles -= 4; // TODO: subtract correct number of cycles

	if( !(PROTECTED_MODE) )
	{
		/* 16-bit */
		PUSH16(oldflags & 0xffff );
		PUSH16(m_sreg[CS].selector );
		if(irq == 3 || irq == 4 || irq == 9 || irq_gate == 1)
			PUSH16(m_eip );
		else
			PUSH16(m_prev_eip );

		m_sreg[CS].selector = READ16(m_idtr.base + entry + 2 );
		m_eip = READ16(m_idtr.base + entry );

		m_TF = 0;
		m_IF = 0;
	}
	else
	{
		int type;
		uint16_t flags;
		I386_SREG desc;
		uint8_t CPL = m_CPL, DPL; //, RPL = 0;

		/* 32-bit */
		v1 = READ32PL(m_idtr.base + entry, 0);
		v2 = READ32PL(m_idtr.base + entry + 4, 0);
		offset = (v2 & 0xffff0000) | (v1 & 0xffff);
		segment = (v1 >> 16) & 0xffff;
		type = (v2>>8) & 0x1F;
		flags = (v2>>8) & 0xf0ff;

		/* segment privilege checks */
		if(entry >= m_idtr.limit)
		{
			LOGMASKED(LOG_PM_FAULT_GP, "IRQ (%08x): Vector %02xh is past IDT limit.\n",m_pc,entry);
			FAULT_EXP(FAULT_GP,entry+2)
		}
		/* segment must be interrupt gate, trap gate, or task gate */
		if(type != 0x05 && type != 0x06 && type != 0x07 && type != 0x0e && type != 0x0f)
		{
			LOGMASKED(LOG_PM_FAULT_GP, "IRQ#%02x (%08x): Vector segment %04x is not an interrupt, trap or task gate.\n",irq,m_pc,segment);
			FAULT_EXP(FAULT_GP,entry+2)
		}

		if(m_ext == 0) // if software interrupt (caused by INT/INTO/INT3)
		{
			if(((flags >> 5) & 0x03) < CPL)
			{
				LOGMASKED(LOG_PM_FAULT_GP, "IRQ (%08x): Software IRQ - gate DPL is less than CPL.\n",m_pc);
				FAULT_EXP(FAULT_GP,entry+2)
			}
			if(V8086_MODE)
			{
				if((!m_IOP1 || !m_IOP2) && (m_opcode != 0xcc))
				{
					LOGMASKED(LOG_PM_FAULT_GP, "IRQ (%08x): Is in Virtual 8086 mode and IOPL != 3.\n",m_pc);
					FAULT(FAULT_GP,0)
				}

			}
		}

		if((flags & 0x0080) == 0)
		{
			LOGMASKED(LOG_PM_FAULT_NP, "IRQ: Vector segment is not present.\n");
			FAULT_EXP(FAULT_NP,entry+2)
		}

		if(type == 0x05)
		{
			/* Task gate */
			memset(&desc, 0, sizeof(desc));
			desc.selector = segment;
			i386_load_protected_mode_segment(&desc,nullptr);
			if(segment & 0x04)
			{
				LOGMASKED(LOG_PM_FAULT_TS, "IRQ: Task gate: TSS is not in the GDT.\n");
				FAULT_EXP(FAULT_TS,segment & ~0x03);
			}
			else
			{
				if(segment > m_gdtr.limit)
				{
					LOGMASKED(LOG_PM_FAULT_TS, "IRQ: Task gate: TSS is past GDT limit.\n");
					FAULT_EXP(FAULT_TS,segment & ~0x03);
				}
			}
			if((desc.flags & 0x000f) != 0x09 && (desc.flags & 0x000f) != 0x01)
			{
				LOGMASKED(LOG_PM_FAULT_TS, "IRQ: Task gate: TSS is not an available TSS.\n");
				FAULT_EXP(FAULT_TS,segment & ~0x03);
			}
			if((desc.flags & 0x0080) == 0)
			{
				LOGMASKED(LOG_PM_FAULT_NP, "IRQ: Task gate: TSS is not present.\n");
				FAULT_EXP(FAULT_NP,segment & ~0x03);
			}
			if(!(irq == 3 || irq == 4 || irq == 9 || irq_gate == 1))
				m_eip = m_prev_eip;
			if(desc.flags & 0x08)
				i386_task_switch(desc.selector,1);
			else
				i286_task_switch(desc.selector,1);
			return;
		}
		else
		{
			/* Interrupt or Trap gate */
			memset(&desc, 0, sizeof(desc));
			desc.selector = segment;
			i386_load_protected_mode_segment(&desc,nullptr);
			CPL = m_CPL;  // current privilege level
			DPL = (desc.flags >> 5) & 0x03;  // descriptor privilege level
//          RPL = segment & 0x03;  // requested privilege level

			if((segment & ~0x03) == 0)
			{
				LOGMASKED(LOG_PM_FAULT_GP, "IRQ: Gate segment is null.\n");
				FAULT_EXP(FAULT_GP,m_ext)
			}
			if(segment & 0x04)
			{
				if((segment & ~0x07) > m_ldtr.limit)
				{
					LOGMASKED(LOG_PM_FAULT_GP, "IRQ: Gate segment is past LDT limit.\n");
					FAULT_EXP(FAULT_GP,(segment & 0x03)+m_ext)
				}
			}
			else
			{
				if((segment & ~0x07) > m_gdtr.limit)
				{
					LOGMASKED(LOG_PM_FAULT_GP, "IRQ: Gate segment is past GDT limit.\n");
					FAULT_EXP(FAULT_GP,(segment & 0x03)+m_ext)
				}
			}
			if((desc.flags & 0x0018) != 0x18)
			{
				LOGMASKED(LOG_PM_FAULT_GP, "IRQ: Gate descriptor is not a code segment.\n");
				FAULT_EXP(FAULT_GP,(segment & 0x03)+m_ext)
			}
			if((desc.flags & 0x0080) == 0)
			{
				LOGMASKED(LOG_PM_FAULT_NP, "IRQ: Gate segment is not present.\n");
				FAULT_EXP(FAULT_NP,(segment & 0x03)+m_ext)
			}
			if((desc.flags & 0x0004) == 0 && (DPL < CPL))
			{
				/* IRQ to inner privilege */
				I386_SREG stack;
				uint32_t newESP,oldSS,oldESP;

				if(V8086_MODE && DPL)
				{
					LOGMASKED(LOG_PM_FAULT_GP, "IRQ: Gate to CPL>0 from VM86 mode.\n");
					FAULT_EXP(FAULT_GP,segment & ~0x03);
				}
				/* Check new stack segment in TSS */
				memset(&stack, 0, sizeof(stack));
				stack.selector = i386_get_stack_segment(DPL);
				i386_load_protected_mode_segment(&stack,nullptr);
				oldSS = m_sreg[SS].selector;
				if(flags & 0x0008)
					oldESP = REG32(ESP);
				else
					oldESP = REG16(SP);
				if((stack.selector & ~0x03) == 0)
				{
					LOGMASKED(LOG_PM_FAULT_GP, "IRQ: New stack selector is null.\n");
					FAULT_EXP(FAULT_GP,m_ext)
				}
				if(stack.selector & 0x04)
				{
					if((stack.selector & ~0x07) > m_ldtr.base)
					{
						LOGMASKED(LOG_PM_FAULT_TS, "IRQ: New stack selector is past LDT limit.\n");
						FAULT_EXP(FAULT_TS,(stack.selector & ~0x03)+m_ext)
					}
				}
				else
				{
					if((stack.selector & ~0x07) > m_gdtr.base)
					{
						LOGMASKED(LOG_PM_FAULT_TS, "IRQ: New stack selector is past GDT limit.\n");
						FAULT_EXP(FAULT_TS,(stack.selector & ~0x03)+m_ext)
					}
				}
				if((stack.selector & 0x03) != DPL)
				{
					LOGMASKED(LOG_PM_FAULT_TS, "IRQ: New stack selector RPL is not equal to code segment DPL.\n");
					FAULT_EXP(FAULT_TS,(stack.selector & ~0x03)+m_ext)
				}
				if(((stack.flags >> 5) & 0x03) != DPL)
				{
					LOGMASKED(LOG_PM_FAULT_TS, "IRQ: New stack segment DPL is not equal to code segment DPL.\n");
					FAULT_EXP(FAULT_TS,(stack.selector & ~0x03)+m_ext)
				}
				if(((stack.flags & 0x0018) != 0x10) && (stack.flags & 0x0002) != 0)
				{
					LOGMASKED(LOG_PM_FAULT_TS, "IRQ: New stack segment is not a writable data segment.\n");
					FAULT_EXP(FAULT_TS,(stack.selector & ~0x03)+m_ext) // #TS(stack selector + EXT)
				}
				if((stack.flags & 0x0080) == 0)
				{
					LOGMASKED(LOG_PM_FAULT_SS, "IRQ: New stack segment is not present.\n");
					FAULT_EXP(FAULT_SS,(stack.selector & ~0x03)+m_ext) // #TS(stack selector + EXT)
				}
				newESP = i386_get_stack_ptr(DPL);
				if(type & 0x08) // 32-bit gate
				{
					if(((newESP < (V8086_MODE?36:20)) && !(stack.flags & 0x4)) || ((~stack.limit < (~(newESP - 1) + (V8086_MODE?36:20))) && (stack.flags & 0x4)))
					{
						LOGMASKED(LOG_PM_FAULT_SS, "IRQ: New stack has no space for return addresses.\n");
						FAULT_EXP(FAULT_SS,0)
					}
				}
				else // 16-bit gate
				{
					newESP &= 0xffff;
					if(((newESP < (V8086_MODE?18:10)) && !(stack.flags & 0x4)) || ((~stack.limit < (~(newESP - 1) + (V8086_MODE?18:10))) && (stack.flags & 0x4)))
					{
						LOGMASKED(LOG_PM_FAULT_SS, "IRQ: New stack has no space for return addresses.\n");
						FAULT_EXP(FAULT_SS,0)
					}
				}
				if(offset > desc.limit)
				{
					LOGMASKED(LOG_PM_FAULT_GP, "IRQ: New EIP is past code segment limit.\n");
					FAULT_EXP(FAULT_GP,0)
				}
				/* change CPL before accessing the stack */
				m_CPL = DPL;
				/* check for page fault at new stack TODO: check if stack frame crosses page boundary */
				WRITE_TEST(stack.base+newESP-1);
				/* Load new stack segment descriptor */
				m_sreg[SS].selector = stack.selector;
				i386_load_protected_mode_segment(&m_sreg[SS],nullptr);
				i386_set_descriptor_accessed(stack.selector);
				REG32(ESP) = newESP;
				if(V8086_MODE)
				{
					//LOGMASKED(LOG_PM_EVENTS, "IRQ (%08x): Interrupt during V8086 task\n",m_pc);
					if(type & 0x08)
					{
						PUSH32SEG(m_sreg[GS].selector & 0xffff);
						PUSH32SEG(m_sreg[FS].selector & 0xffff);
						PUSH32SEG(m_sreg[DS].selector & 0xffff);
						PUSH32SEG(m_sreg[ES].selector & 0xffff);
					}
					else
					{
						PUSH16(m_sreg[GS].selector);
						PUSH16(m_sreg[FS].selector);
						PUSH16(m_sreg[DS].selector);
						PUSH16(m_sreg[ES].selector);
					}
					m_sreg[GS].selector = 0;
					m_sreg[FS].selector = 0;
					m_sreg[DS].selector = 0;
					m_sreg[ES].selector = 0;
					m_VM = 0;
					i386_load_segment_descriptor(GS);
					i386_load_segment_descriptor(FS);
					i386_load_segment_descriptor(DS);
					i386_load_segment_descriptor(ES);
				}
				if(type & 0x08)
				{
					// 32-bit gate
					PUSH32SEG(oldSS);
					PUSH32(oldESP);
				}
				else
				{
					// 16-bit gate
					PUSH16(oldSS);
					PUSH16(oldESP);
				}
				SetRPL = 1;
			}
			else
			{
				int stack_limit;
				if((desc.flags & 0x0004) || (DPL == CPL))
				{
					/* IRQ to same privilege */
					if(V8086_MODE && !m_ext)
					{
						LOGMASKED(LOG_PM_FAULT_GP, "IRQ: Gate to same privilege from VM86 mode.\n");
						FAULT_EXP(FAULT_GP,segment & ~0x03);
					}
					if(type == 0x0e || type == 0x0f)  // 32-bit gate
						stack_limit = 10;
					else
						stack_limit = 6;
					// TODO: Add check for error code (2 extra bytes)
					if(REG32(ESP) < stack_limit)
					{
						LOGMASKED(LOG_PM_FAULT_SS, "IRQ: Stack has no space left (needs %i bytes).\n",stack_limit);
						FAULT_EXP(FAULT_SS,0)
					}
					if(offset > desc.limit)
					{
						LOGMASKED(LOG_PM_FAULT_GP, "IRQ: Gate segment offset is past segment limit.\n");
						FAULT_EXP(FAULT_GP,0)
					}
					SetRPL = 1;
				}
				else
				{
					LOGMASKED(LOG_PM_FAULT_GP, "IRQ: Gate descriptor is non-conforming, and DPL does not equal CPL.\n");
					FAULT_EXP(FAULT_GP,segment)
				}
			}
		}
		uint32_t tempSP = REG32(ESP);
		try
		{
			// this is ugly but the alternative is worse
			if(type != 0x0e && type != 0x0f)  // if not 386 interrupt or trap gate
			{
				PUSH16(oldflags & 0xffff );
				PUSH16(m_sreg[CS].selector );
				if(irq == 3 || irq == 4 || irq == 9 || irq_gate == 1)
					PUSH16(m_eip );
				else
					PUSH16(m_prev_eip );
			}
			else
			{
				PUSH32((oldflags & 0x00ffffff) | (1 << 16) ); //386 faults always have the RF bit set in the saved flags register.
				PUSH32SEG(m_sreg[CS].selector );
				if(irq == 3 || irq == 4 || irq == 9 || irq_gate == 1)
					PUSH32(m_eip );
				else
					PUSH32(m_prev_eip );
			}
		}
		catch(uint64_t e)
		{
			REG32(ESP) = tempSP;
			throw e;
		}
		if(SetRPL != 0)
			segment = (segment & ~0x03) | m_CPL;
		m_sreg[CS].selector = segment;
		m_eip = offset;

		if(type == 0x0e || type == 0x06)
			m_IF = 0;
		m_TF = 0;
		m_NT = 0;
	}

	i386_load_segment_descriptor(CS);
	CHANGE_PC(m_eip);

}

void i386_device::i386_trap_with_error(int irq, int irq_gate, int trap_level, uint32_t error)
{
	try
	{
		i386_trap(irq,irq_gate,trap_level);
		if(irq == 8 || irq == 10 || irq == 11 || irq == 12 || irq == 13 || irq == 14)
		{
			// for these exceptions, an error code is pushed onto the stack by the processor.
			// no error code is pushed for software interrupts, either.
			if(PROTECTED_MODE)
			{
				uint32_t entry = irq * 8;
				uint32_t v2,type;
				v2 = READ32PL(m_idtr.base + entry + 4, 0);
				type = (v2>>8) & 0x1F;
				if(type == 5)
				{
					v2 = READ32PL(m_idtr.base + entry, 0);
					v2 = READ32PL(m_gdtr.base + ((v2 >> 16) & 0xfff8) + 4, 0);
					type = (v2>>8) & 0x1F;
				}
				if(type >= 9)
					PUSH32(error);
				else
					PUSH16(error);
			}
			else
				PUSH16(error);
		}
	}
	catch(uint64_t e)
	{
		trap_level++;
		if(trap_level == 1)
		{
			m_ext = 1;
			LOGMASKED(LOG_PM_FAULT_DF, "IRQ: Double fault.\n");
			i386_trap_with_error(FAULT_DF,0,trap_level,0);
			return;
		}
		if(trap_level >= 2)
		{
			LOGMASKED(LOG_PM_EVENTS, "IRQ: Triple fault. CPU reset.\n");
			pulse_input_line(INPUT_LINE_RESET, attotime::zero);
			return;
		}
	}
}


void i386_device::i286_task_switch(uint16_t selector, uint8_t nested)
{
	uint32_t tss;
	I386_SREG seg;
	uint16_t old_task;
	uint8_t ar_byte;  // access rights byte

	/* TODO: Task State Segment privilege checks */

	/* For tasks that aren't nested, clear the busy bit in the task's descriptor */
	if(nested == 0)
	{
		if(m_task.segment & 0x0004)
		{
			ar_byte = READ8(m_ldtr.base + (m_task.segment & ~0x0007) + 5);
			WRITE8(m_ldtr.base + (m_task.segment & ~0x0007) + 5,ar_byte & ~0x02);
		}
		else
		{
			ar_byte = READ8(m_gdtr.base + (m_task.segment & ~0x0007) + 5);
			WRITE8(m_gdtr.base + (m_task.segment & ~0x0007) + 5,ar_byte & ~0x02);
		}
	}

	/* Save the state of the current task in the current TSS (TR register base) */
	tss = m_task.base;
	WRITE16(tss+0x0e,m_eip & 0x0000ffff);
	WRITE16(tss+0x10,get_flags() & 0x0000ffff);
	WRITE16(tss+0x12,REG16(AX));
	WRITE16(tss+0x14,REG16(CX));
	WRITE16(tss+0x16,REG16(DX));
	WRITE16(tss+0x18,REG16(BX));
	WRITE16(tss+0x1a,REG16(SP));
	WRITE16(tss+0x1c,REG16(BP));
	WRITE16(tss+0x1e,REG16(SI));
	WRITE16(tss+0x20,REG16(DI));
	WRITE16(tss+0x22,m_sreg[ES].selector);
	WRITE16(tss+0x24,m_sreg[CS].selector);
	WRITE16(tss+0x26,m_sreg[SS].selector);
	WRITE16(tss+0x28,m_sreg[DS].selector);

	old_task = m_task.segment;

	/* Load task register with the selector of the incoming task */
	m_task.segment = selector;
	memset(&seg, 0, sizeof(seg));
	seg.selector = m_task.segment;
	i386_load_protected_mode_segment(&seg,nullptr);
	m_task.limit = seg.limit;
	m_task.base = seg.base;
	m_task.flags = seg.flags;

	/* Set TS bit in CR0 */
	m_cr[0] |= CR0_TS;

	/* Load incoming task state from the new task's TSS */
	tss = m_task.base;
	m_ldtr.segment = READ16(tss+0x2a) & 0xffff;
	seg.selector = m_ldtr.segment;
	i386_load_protected_mode_segment(&seg,nullptr);
	m_ldtr.limit = seg.limit;
	m_ldtr.base = seg.base;
	m_ldtr.flags = seg.flags;
	m_eip = READ16(tss+0x0e);
	set_flags(READ16(tss+0x10));
	REG16(AX) = READ16(tss+0x12);
	REG16(CX) = READ16(tss+0x14);
	REG16(DX) = READ16(tss+0x16);
	REG16(BX) = READ16(tss+0x18);
	REG16(SP) = READ16(tss+0x1a);
	REG16(BP) = READ16(tss+0x1c);
	REG16(SI) = READ16(tss+0x1e);
	REG16(DI) = READ16(tss+0x20);
	m_sreg[ES].selector = READ16(tss+0x22) & 0xffff;
	i386_load_segment_descriptor(ES);
	m_sreg[CS].selector = READ16(tss+0x24) & 0xffff;
	i386_load_segment_descriptor(CS);
	m_sreg[SS].selector = READ16(tss+0x26) & 0xffff;
	i386_load_segment_descriptor(SS);
	m_sreg[DS].selector = READ16(tss+0x28) & 0xffff;
	i386_load_segment_descriptor(DS);

	/* Set the busy bit in the new task's descriptor */
	if(selector & 0x0004)
	{
		ar_byte = READ8(m_ldtr.base + (selector & ~0x0007) + 5);
		WRITE8(m_ldtr.base + (selector & ~0x0007) + 5,ar_byte | 0x02);
	}
	else
	{
		ar_byte = READ8(m_gdtr.base + (selector & ~0x0007) + 5);
		WRITE8(m_gdtr.base + (selector & ~0x0007) + 5,ar_byte | 0x02);
	}

	/* For nested tasks, we write the outgoing task's selector to the back-link field of the new TSS,
	   and set the NT flag in the EFLAGS register */
	if(nested != 0)
	{
		WRITE16(tss+0,old_task);
		m_NT = 1;
	}
	CHANGE_PC(m_eip);

	m_CPL = (m_sreg[SS].flags >> 5) & 3;

	m_auto_clear_RF = false;
//  printf("286 Task Switch from selector %04x to %04x\n",old_task,selector);
}

void i386_device::i386_task_switch(uint16_t selector, uint8_t nested)
{
	uint32_t tss;
	I386_SREG seg;
	uint16_t old_task;
	uint8_t ar_byte;  // access rights byte
	uint32_t oldcr3 = m_cr[3];

	/* TODO: Task State Segment privilege checks */

	/* For tasks that aren't nested, clear the busy bit in the task's descriptor */
	if(nested == 0)
	{
		if(m_task.segment & 0x0004)
		{
			ar_byte = READ8(m_ldtr.base + (m_task.segment & ~0x0007) + 5);
			WRITE8(m_ldtr.base + (m_task.segment & ~0x0007) + 5,ar_byte & ~0x02);
		}
		else
		{
			ar_byte = READ8(m_gdtr.base + (m_task.segment & ~0x0007) + 5);
			WRITE8(m_gdtr.base + (m_task.segment & ~0x0007) + 5,ar_byte & ~0x02);
		}
	}

	/* Save the state of the current task in the current TSS (TR register base) */
	tss = m_task.base;
	WRITE32(tss+0x1c,m_cr[3]);  // correct?
	WRITE32(tss+0x20,m_eip);
	WRITE32(tss+0x24,get_flags());
	WRITE32(tss+0x28,REG32(EAX));
	WRITE32(tss+0x2c,REG32(ECX));
	WRITE32(tss+0x30,REG32(EDX));
	WRITE32(tss+0x34,REG32(EBX));
	WRITE32(tss+0x38,REG32(ESP));
	WRITE32(tss+0x3c,REG32(EBP));
	WRITE32(tss+0x40,REG32(ESI));
	WRITE32(tss+0x44,REG32(EDI));
	WRITE32(tss+0x48,m_sreg[ES].selector);
	WRITE32(tss+0x4c,m_sreg[CS].selector);
	WRITE32(tss+0x50,m_sreg[SS].selector);
	WRITE32(tss+0x54,m_sreg[DS].selector);
	WRITE32(tss+0x58,m_sreg[FS].selector);
	WRITE32(tss+0x5c,m_sreg[GS].selector);

	old_task = m_task.segment;

	/* Load task register with the selector of the incoming task */
	m_task.segment = selector;
	memset(&seg, 0, sizeof(seg));
	seg.selector = m_task.segment;
	i386_load_protected_mode_segment(&seg,nullptr);
	m_task.limit = seg.limit;
	m_task.base = seg.base;
	m_task.flags = seg.flags;

	/* Set TS bit in CR0 */
	m_cr[0] |= CR0_TS;

	/* Load incoming task state from the new task's TSS */
	tss = m_task.base;
	m_ldtr.segment = READ32(tss+0x60) & 0xffff;
	seg.selector = m_ldtr.segment;
	i386_load_protected_mode_segment(&seg,nullptr);
	m_ldtr.limit = seg.limit;
	m_ldtr.base = seg.base;
	m_ldtr.flags = seg.flags;
	m_eip = READ32(tss+0x20);
	set_flags(READ32(tss+0x24));
	REG32(EAX) = READ32(tss+0x28);
	REG32(ECX) = READ32(tss+0x2c);
	REG32(EDX) = READ32(tss+0x30);
	REG32(EBX) = READ32(tss+0x34);
	REG32(ESP) = READ32(tss+0x38);
	REG32(EBP) = READ32(tss+0x3c);
	REG32(ESI) = READ32(tss+0x40);
	REG32(EDI) = READ32(tss+0x44);
	m_sreg[ES].selector = READ32(tss+0x48) & 0xffff;
	i386_load_segment_descriptor(ES);
	m_sreg[CS].selector = READ32(tss+0x4c) & 0xffff;
	i386_load_segment_descriptor(CS);
	m_sreg[SS].selector = READ32(tss+0x50) & 0xffff;
	i386_load_segment_descriptor(SS);
	m_sreg[DS].selector = READ32(tss+0x54) & 0xffff;
	i386_load_segment_descriptor(DS);
	m_sreg[FS].selector = READ32(tss+0x58) & 0xffff;
	i386_load_segment_descriptor(FS);
	m_sreg[GS].selector = READ32(tss+0x5c) & 0xffff;
	i386_load_segment_descriptor(GS);
	/* For nested tasks, we write the outgoing task's selector to the back-link field of the new TSS,
	   and set the NT flag in the EFLAGS register before setting cr3 as the old tss address might be gone */
	if(nested != 0)
	{
		WRITE32(tss+0,old_task);
		m_NT = 1;
	}
	m_cr[3] = READ32(tss+0x1c);  // CR3 (PDBR)
	if(oldcr3 != m_cr[3])
		vtlb_flush_dynamic();

	/* Set the busy bit in the new task's descriptor */
	if(selector & 0x0004)
	{
		ar_byte = READ8(m_ldtr.base + (selector & ~0x0007) + 5);
		WRITE8(m_ldtr.base + (selector & ~0x0007) + 5,ar_byte | 0x02);
	}
	else
	{
		ar_byte = READ8(m_gdtr.base + (selector & ~0x0007) + 5);
		WRITE8(m_gdtr.base + (selector & ~0x0007) + 5,ar_byte | 0x02);
	}

	CHANGE_PC(m_eip);

	m_CPL = (m_sreg[SS].flags >> 5) & 3;

	int t_bit = READ32(tss+0x64) & 1;
	if(t_bit) m_dr[6] |= (1 << 15); //If the T bit of the new TSS is set, set the BT bit of DR6.

	m_dr[7] &= ~(0x155); //Clear all of the local enable bits from DR7.

	m_auto_clear_RF = false;
//  printf("386 Task Switch from selector %04x to %04x\n",old_task,selector);
}

void i386_device::i386_protected_mode_jump(uint16_t seg, uint32_t off, int indirect, int operand32)
{
	I386_SREG desc;
	I386_CALL_GATE call_gate;
	uint8_t CPL,DPL,RPL;
	uint8_t SetRPL;
	uint16_t segment = seg;
	uint32_t offset = off;

	/* Check selector is not null */
	if((segment & ~0x03) == 0)
	{
		LOGMASKED(LOG_PM_FAULT_GP, "JMP: Segment is null.\n");
		FAULT(FAULT_GP,0)
	}
	/* Selector is within descriptor table limit */
	if((segment & 0x04) == 0)
	{
		/* check GDT limit */
		if((segment & ~0x07) > (m_gdtr.limit))
		{
			LOGMASKED(LOG_PM_FAULT_GP, "JMP: Segment is past GDT limit.\n");
			FAULT(FAULT_GP,segment & 0xfffc)
		}
	}
	else
	{
		/* check LDT limit */
		if((segment & ~0x07) > (m_ldtr.limit))
		{
			LOGMASKED(LOG_PM_FAULT_GP, "JMP: Segment is past LDT limit.\n");
			FAULT(FAULT_GP,segment & 0xfffc)
		}
	}
	/* Determine segment type */
	memset(&desc, 0, sizeof(desc));
	desc.selector = segment;
	i386_load_protected_mode_segment(&desc,nullptr);
	CPL = m_CPL;  // current privilege level
	DPL = (desc.flags >> 5) & 0x03;  // descriptor privilege level
	RPL = segment & 0x03;  // requested privilege level
	if((desc.flags & 0x0018) == 0x0018)
	{
		/* code segment */
		if((desc.flags & 0x0004) == 0)
		{
			/* non-conforming */
			if(RPL > CPL)
			{
				LOGMASKED(LOG_PM_FAULT_GP, "JMP: RPL %i is less than CPL %i\n",RPL,CPL);
				FAULT(FAULT_GP,segment & 0xfffc)
			}
			if(DPL != CPL)
			{
				LOGMASKED(LOG_PM_FAULT_GP, "JMP: DPL %i is not equal CPL %i\n",DPL,CPL);
				FAULT(FAULT_GP,segment & 0xfffc)
			}
		}
		else
		{
			/* conforming */
			if(DPL > CPL)
			{
				LOGMASKED(LOG_PM_FAULT_GP, "JMP: DPL %i is less than CPL %i\n",DPL,CPL);
				FAULT(FAULT_GP,segment & 0xfffc)
			}
		}
		SetRPL = 1;
		if((desc.flags & 0x0080) == 0)
		{
			LOGMASKED(LOG_PM_FAULT_NP, "JMP: Segment is not present\n");
			FAULT(FAULT_NP,segment & 0xfffc)
		}
		if(offset > desc.limit)
		{
			LOGMASKED(LOG_PM_FAULT_GP, "JMP: Offset is past segment limit\n");
			FAULT(FAULT_GP,0)
		}
	}
	else
	{
		if((desc.flags & 0x0010) != 0)
		{
			LOGMASKED(LOG_PM_FAULT_GP, "JMP: Segment is a data segment\n");
			FAULT(FAULT_GP,segment & 0xfffc)  // #GP (cannot execute code in a data segment)
		}
		else
		{
			switch(desc.flags & 0x000f)
			{
			case 0x01:  // 286 Available TSS
			case 0x09:  // 386 Available TSS
				LOGMASKED(LOG_PM_EVENTS, "JMP: Available 386 TSS at %08x\n",m_pc);
				memset(&desc, 0, sizeof(desc));
				desc.selector = segment;
				i386_load_protected_mode_segment(&desc,nullptr);
				DPL = (desc.flags >> 5) & 0x03;  // descriptor privilege level
				if(DPL < CPL)
				{
					LOGMASKED(LOG_PM_FAULT_GP, "JMP: TSS: DPL %i is less than CPL %i\n",DPL,CPL);
					FAULT(FAULT_GP,segment & 0xfffc)
				}
				if(DPL < RPL)
				{
					LOGMASKED(LOG_PM_FAULT_GP, "JMP: TSS: DPL %i is less than TSS RPL %i\n",DPL,RPL);
					FAULT(FAULT_GP,segment & 0xfffc)
				}
				if((desc.flags & 0x0080) == 0)
				{
					LOGMASKED(LOG_PM_FAULT_GP, "JMP: TSS: Segment is not present\n");
					FAULT(FAULT_GP,segment & 0xfffc)
				}
				if(desc.flags & 0x0008)
					i386_task_switch(desc.selector,0);
				else
					i286_task_switch(desc.selector,0);
				return;
			case 0x04:  // 286 Call Gate
			case 0x0c:  // 386 Call Gate
				//LOGMASKED(LOG_PM_EVENTS, "JMP: Call gate at %08x\n",m_pc);
				SetRPL = 1;
				memset(&call_gate, 0, sizeof(call_gate));
				call_gate.segment = segment;
				i386_load_call_gate(&call_gate);
				DPL = call_gate.dpl;
				if(DPL < CPL)
				{
					LOGMASKED(LOG_PM_FAULT_GP, "JMP: Call Gate: DPL %i is less than CPL %i\n",DPL,CPL);
					FAULT(FAULT_GP,segment & 0xfffc)
				}
				if(DPL < RPL)
				{
					LOGMASKED(LOG_PM_FAULT_GP, "JMP: Call Gate: DPL %i is less than RPL %i\n",DPL,RPL);
					FAULT(FAULT_GP,segment & 0xfffc)
				}
				if((desc.flags & 0x0080) == 0)
				{
					LOGMASKED(LOG_PM_FAULT_NP, "JMP: Call Gate: Segment is not present\n");
					FAULT(FAULT_NP,segment & 0xfffc)
				}
				/* Now we examine the segment that the call gate refers to */
				if(call_gate.selector == 0)
				{
					LOGMASKED(LOG_PM_FAULT_GP, "JMP: Call Gate: Gate selector is null\n");
					FAULT(FAULT_GP,0)
				}
				if(call_gate.selector & 0x04)
				{
					if((call_gate.selector & ~0x07) > m_ldtr.limit)
					{
						LOGMASKED(LOG_PM_FAULT_GP, "JMP: Call Gate: Gate Selector is past LDT segment limit\n");
						FAULT(FAULT_GP,call_gate.selector & 0xfffc)
					}
				}
				else
				{
					if((call_gate.selector & ~0x07) > m_gdtr.limit)
					{
						LOGMASKED(LOG_PM_FAULT_GP, "JMP: Call Gate: Gate Selector is past GDT segment limit\n");
						FAULT(FAULT_GP,call_gate.selector & 0xfffc)
					}
				}
				desc.selector = call_gate.selector;
				i386_load_protected_mode_segment(&desc,nullptr);
				DPL = (desc.flags >> 5) & 0x03;
				if((desc.flags & 0x0018) != 0x18)
				{
					LOGMASKED(LOG_PM_FAULT_GP, "JMP: Call Gate: Gate does not point to a code segment\n");
					FAULT(FAULT_GP,call_gate.selector & 0xfffc)
				}
				if((desc.flags & 0x0004) == 0)
				{  // non-conforming
					if(DPL != CPL)
					{
						LOGMASKED(LOG_PM_FAULT_GP, "JMP: Call Gate: Gate DPL does not equal CPL\n");
						FAULT(FAULT_GP,call_gate.selector & 0xfffc)
					}
				}
				else
				{  // conforming
					if(DPL > CPL)
					{
						LOGMASKED(LOG_PM_FAULT_GP, "JMP: Call Gate: Gate DPL is greater than CPL\n");
						FAULT(FAULT_GP,call_gate.selector & 0xfffc)
					}
				}
				if((desc.flags & 0x0080) == 0)
				{
					LOGMASKED(LOG_PM_FAULT_NP, "JMP: Call Gate: Gate Segment is not present\n");
					FAULT(FAULT_NP,call_gate.selector & 0xfffc)
				}
				if(call_gate.offset > desc.limit)
				{
					LOGMASKED(LOG_PM_FAULT_GP, "JMP: Call Gate: Gate offset is past Gate segment limit\n");
					FAULT(FAULT_GP,call_gate.selector & 0xfffc)
				}
				segment = call_gate.selector;
				offset = call_gate.offset;
				break;
			case 0x05:  // Task Gate
				LOGMASKED(LOG_PM_EVENTS, "JMP: Task gate at %08x\n",m_pc);
				memset(&call_gate, 0, sizeof(call_gate));
				call_gate.segment = segment;
				i386_load_call_gate(&call_gate);
				DPL = call_gate.dpl;
				if(DPL < CPL)
				{
					LOGMASKED(LOG_PM_FAULT_GP, "JMP: Task Gate: Gate DPL %i is less than CPL %i\n",DPL,CPL);
					FAULT(FAULT_GP,segment & 0xfffc)
				}
				if(DPL < RPL)
				{
					LOGMASKED(LOG_PM_FAULT_GP, "JMP: Task Gate: Gate DPL %i is less than CPL %i\n",DPL,CPL);
					FAULT(FAULT_GP,segment & 0xfffc)
				}
				if(call_gate.present == 0)
				{
					LOGMASKED(LOG_PM_FAULT_GP, "JMP: Task Gate: Gate is not present.\n");
					FAULT(FAULT_GP,segment & 0xfffc)
				}
				/* Check the TSS that the task gate points to */
				desc.selector = call_gate.selector;
				i386_load_protected_mode_segment(&desc,nullptr);
				DPL = (desc.flags >> 5) & 0x03;  // descriptor privilege level
				RPL = call_gate.selector & 0x03;  // requested privilege level
				if(call_gate.selector & 0x04)
				{
					LOGMASKED(LOG_PM_FAULT_GP, "JMP: Task Gate TSS: TSS must be global.\n");
					FAULT(FAULT_GP,call_gate.selector & 0xfffc)
				}
				else
				{
					if((call_gate.selector & ~0x07) > m_gdtr.limit)
					{
						LOGMASKED(LOG_PM_FAULT_GP, "JMP: Task Gate TSS: TSS is past GDT limit.\n");
						FAULT(FAULT_GP,call_gate.selector & 0xfffc)
					}
				}
				if((call_gate.ar & 0x000f) == 0x0009 || (call_gate.ar & 0x000f) == 0x0001)
				{
					LOGMASKED(LOG_PM_FAULT_GP, "JMP: Task Gate TSS: Segment is not an available TSS.\n");
					FAULT(FAULT_GP,call_gate.selector & 0xfffc)
				}
				if(call_gate.present == 0)
				{
					LOGMASKED(LOG_PM_FAULT_NP, "JMP: Task Gate TSS: TSS is not present.\n");
					FAULT(FAULT_NP,call_gate.selector & 0xfffc)
				}
				if(call_gate.ar & 0x08)
					i386_task_switch(call_gate.selector,0);
				else
					i286_task_switch(call_gate.selector,0);
				return;
			default:  // invalid segment type
				LOGMASKED(LOG_PM_FAULT_GP, "JMP: Invalid segment type (%i) to jump to.\n",desc.flags & 0x000f);
				FAULT(FAULT_GP,segment & 0xfffc)
			}
		}
	}

	if(SetRPL != 0)
		segment = (segment & ~0x03) | m_CPL;
	if(operand32 == 0)
		m_eip = offset & 0x0000ffff;
	else
		m_eip = offset;
	m_sreg[CS].selector = segment;
	m_performed_intersegment_jump = 1;
	i386_load_segment_descriptor(CS);
	CHANGE_PC(m_eip);
}

void i386_device::i386_protected_mode_call(uint16_t seg, uint32_t off, int indirect, int operand32)
{
	I386_SREG desc;
	I386_CALL_GATE gate;
	uint8_t SetRPL;
	uint8_t CPL, DPL, RPL;
	uint16_t selector = seg;
	uint32_t offset = off;
	int x;

	if((selector & ~0x03) == 0)
	{
		LOGMASKED(LOG_PM_FAULT_GP, "CALL (%08x): Selector is null.\n",m_pc);
		FAULT(FAULT_GP,0)  // #GP(0)
	}
	if(selector & 0x04)
	{
		if((selector & ~0x07) > m_ldtr.limit)
		{
			LOGMASKED(LOG_PM_FAULT_GP, "CALL: Selector is past LDT limit.\n");
			FAULT(FAULT_GP,selector & ~0x03)  // #GP(selector)
		}
	}
	else
	{
		if((selector & ~0x07) > m_gdtr.limit)
		{
			LOGMASKED(LOG_PM_FAULT_GP, "CALL: Selector is past GDT limit.\n");
			FAULT(FAULT_GP,selector & ~0x03)  // #GP(selector)
		}
	}

	/* Determine segment type */
	memset(&desc, 0, sizeof(desc));
	desc.selector = selector;
	i386_load_protected_mode_segment(&desc,nullptr);
	CPL = m_CPL;  // current privilege level
	DPL = (desc.flags >> 5) & 0x03;  // descriptor privilege level
	RPL = selector & 0x03;  // requested privilege level
	if((desc.flags & 0x0018) == 0x18)  // is a code segment
	{
		if(desc.flags & 0x0004)
		{
			/* conforming */
			if(DPL > CPL)
			{
				LOGMASKED(LOG_PM_FAULT_GP, "CALL: Code segment DPL %i is greater than CPL %i\n",DPL,CPL);
				FAULT(FAULT_GP,selector & ~0x03)  // #GP(selector)
			}
		}
		else
		{
			/* non-conforming */
			if(RPL > CPL)
			{
				LOGMASKED(LOG_PM_FAULT_GP, "CALL: RPL %i is greater than CPL %i\n",RPL,CPL);
				FAULT(FAULT_GP,selector & ~0x03)  // #GP(selector)
			}
			if(DPL != CPL)
			{
				LOGMASKED(LOG_PM_FAULT_GP, "CALL: Code segment DPL %i is not equal to CPL %i\n",DPL,CPL);
				FAULT(FAULT_GP,selector & ~0x03)  // #GP(selector)
			}
		}
		SetRPL = 1;
		if((desc.flags & 0x0080) == 0)
		{
			LOGMASKED(LOG_PM_FAULT_NP, "CALL (%08x): Code segment is not present.\n",m_pc);
			FAULT(FAULT_NP,selector & ~0x03)  // #NP(selector)
		}
		if (operand32 != 0)  // if 32-bit
		{
			uint32_t offset = (STACK_32BIT ? REG32(ESP) - 8 : (REG16(SP) - 8) & 0xffff);
			if(i386_limit_check(SS, offset))
			{
				LOGMASKED(LOG_PM_FAULT_SS, "CALL (%08x): Stack has no room for return address.\n",m_pc);
				FAULT(FAULT_SS,0)  // #SS(0)
			}
		}
		else
		{
			uint32_t offset = (STACK_32BIT ? REG32(ESP) - 4 : (REG16(SP) - 4) & 0xffff);
			if(i386_limit_check(SS, offset))
			{
				LOGMASKED(LOG_PM_FAULT_SS, "CALL (%08x): Stack has no room for return address.\n",m_pc);
				FAULT(FAULT_SS,0)  // #SS(0)
			}
		}
		if(offset > desc.limit)
		{
			LOGMASKED(LOG_PM_FAULT_GP, "CALL: EIP is past segment limit.\n");
			FAULT(FAULT_GP,0)  // #GP(0)
		}
	}
	else
	{
		/* special segment type */
		if(desc.flags & 0x0010)
		{
			LOGMASKED(LOG_PM_FAULT_GP, "CALL: Segment is a data segment.\n");
			FAULT(FAULT_GP,desc.selector & ~0x03)  // #GP(selector)
		}
		else
		{
			switch(desc.flags & 0x000f)
			{
			case 0x01:  // Available 286 TSS
			case 0x09:  // Available 386 TSS
				LOGMASKED(LOG_PM_EVENTS, "CALL: Available TSS at %08x\n",m_pc);
				if(DPL < CPL)
				{
					LOGMASKED(LOG_PM_FAULT_TS, "CALL: TSS: DPL is less than CPL.\n");
					FAULT(FAULT_TS,selector & ~0x03) // #TS(selector)
				}
				if(DPL < RPL)
				{
					LOGMASKED(LOG_PM_FAULT_TS, "CALL: TSS: DPL is less than RPL.\n");
					FAULT(FAULT_TS,selector & ~0x03) // #TS(selector)
				}
				if(desc.flags & 0x0002)
				{
					LOGMASKED(LOG_PM_FAULT_TS, "CALL: TSS: TSS is busy.\n");
					FAULT(FAULT_TS,selector & ~0x03) // #TS(selector)
				}
				if((desc.flags & 0x0080) == 0)
				{
					LOGMASKED(LOG_PM_FAULT_NP, "CALL: TSS: Segment %02x is not present.\n",selector);
					FAULT(FAULT_NP,selector & ~0x03) // #NP(selector)
				}
				if(desc.flags & 0x08)
					i386_task_switch(desc.selector,1);
				else
					i286_task_switch(desc.selector,1);
				return;
			case 0x04:  // 286 call gate
			case 0x0c:  // 386 call gate
				if((desc.flags & 0x000f) == 0x04)
					operand32 = 0;
				else
					operand32 = 1;
				memset(&gate, 0, sizeof(gate));
				gate.segment = selector;
				i386_load_call_gate(&gate);
				DPL = gate.dpl;
				//LOGMASKED(LOG_PM_EVENTS, "CALL: Call gate at %08x (%i parameters)\n",m_pc,gate.dword_count);
				if(DPL < CPL)
				{
					LOGMASKED(LOG_PM_FAULT_GP, "CALL: Call gate DPL %i is less than CPL %i.\n",DPL,CPL);
					FAULT(FAULT_GP,desc.selector & ~0x03)  // #GP(selector)
				}
				if(DPL < RPL)
				{
					LOGMASKED(LOG_PM_FAULT_GP, "CALL: Call gate DPL %i is less than RPL %i.\n",DPL,RPL);
					FAULT(FAULT_GP,desc.selector & ~0x03)  // #GP(selector)
				}
				if(gate.present == 0)
				{
					LOGMASKED(LOG_PM_FAULT_NP, "CALL: Call gate is not present.\n");
					FAULT(FAULT_NP,desc.selector & ~0x03)  // #GP(selector)
				}
				desc.selector = gate.selector;
				if((gate.selector & ~0x03) == 0)
				{
					LOGMASKED(LOG_PM_FAULT_GP, "CALL: Call gate: Segment is null.\n");
					FAULT(FAULT_GP,0)  // #GP(0)
				}
				if(desc.selector & 0x04)
				{
					if((desc.selector & ~0x07) > m_ldtr.limit)
					{
						LOGMASKED(LOG_PM_FAULT_GP, "CALL: Call gate: Segment is past LDT limit\n");
						FAULT(FAULT_GP,desc.selector & ~0x03)  // #GP(selector)
					}
				}
				else
				{
					if((desc.selector & ~0x07) > m_gdtr.limit)
					{
						LOGMASKED(LOG_PM_FAULT_GP, "CALL: Call gate: Segment is past GDT limit\n");
						FAULT(FAULT_GP,desc.selector & ~0x03)  // #GP(selector)
					}
				}
				i386_load_protected_mode_segment(&desc,nullptr);
				if((desc.flags & 0x0018) != 0x18)
				{
					LOGMASKED(LOG_PM_FAULT_GP, "CALL: Call gate: Segment is not a code segment.\n");
					FAULT(FAULT_GP,desc.selector & ~0x03)  // #GP(selector)
				}
				DPL = ((desc.flags >> 5) & 0x03);
				if(DPL > CPL)
				{
					LOGMASKED(LOG_PM_FAULT_GP, "CALL: Call gate: Segment DPL %i is greater than CPL %i.\n",DPL,CPL);
					FAULT(FAULT_GP,desc.selector & ~0x03)  // #GP(selector)
				}
				if((desc.flags & 0x0080) == 0)
				{
					LOGMASKED(LOG_PM_FAULT_NP, "CALL (%08x): Code segment is not present.\n",m_pc);
					FAULT(FAULT_NP,desc.selector & ~0x03)  // #NP(selector)
				}
				if(DPL < CPL && (desc.flags & 0x0004) == 0)
				{
					I386_SREG stack;
					I386_SREG temp;
					uint32_t oldSS,oldESP;
					/* more privilege */
					/* Check new SS segment for privilege level from TSS */
					memset(&stack, 0, sizeof(stack));
					stack.selector = i386_get_stack_segment(DPL);
					i386_load_protected_mode_segment(&stack,nullptr);
					if((stack.selector & ~0x03) == 0)
					{
						LOGMASKED(LOG_PM_FAULT_TS, "CALL: Call gate: TSS selector is null\n");
						FAULT(FAULT_TS,0)  // #TS(0)
					}
					if(stack.selector & 0x04)
					{
						if((stack.selector & ~0x07) > m_ldtr.limit)
						{
							LOGMASKED(LOG_PM_FAULT_TS, "CALL: Call gate: TSS selector is past LDT limit\n");
							FAULT(FAULT_TS,stack.selector)  // #TS(SS selector)
						}
					}
					else
					{
						if((stack.selector & ~0x07) > m_gdtr.limit)
						{
							LOGMASKED(LOG_PM_FAULT_TS, "CALL: Call gate: TSS selector is past GDT limit\n");
							FAULT(FAULT_TS,stack.selector)  // #TS(SS selector)
						}
					}
					if((stack.selector & 0x03) != DPL)
					{
						LOGMASKED(LOG_PM_FAULT_TS, "CALL: Call gate: Stack selector RPL does not equal code segment DPL %i\n",DPL);
						FAULT(FAULT_TS,stack.selector)  // #TS(SS selector)
					}
					if(((stack.flags >> 5) & 0x03) != DPL)
					{
						LOGMASKED(LOG_PM_FAULT_TS, "CALL: Call gate: Stack DPL does not equal code segment DPL %i\n",DPL);
						FAULT(FAULT_TS,stack.selector)  // #TS(SS selector)
					}
					if((stack.flags & 0x0018) != 0x10 && (stack.flags & 0x0002))
					{
						LOGMASKED(LOG_PM_FAULT_TS, "CALL: Call gate: Stack segment is not a writable data segment\n");
						FAULT(FAULT_TS,stack.selector)  // #TS(SS selector)
					}
					if((stack.flags & 0x0080) == 0)
					{
						LOGMASKED(LOG_PM_FAULT_SS, "CALL: Call gate: Stack segment is not present\n");
						FAULT(FAULT_SS,stack.selector)  // #SS(SS selector)
					}
					uint32_t newESP = i386_get_stack_ptr(DPL);
					if(!stack.d)
					{
						newESP &= 0xffff;
					}
					if(operand32 != 0)
					{
						if(newESP < ((gate.dword_count & 0x1f) + 16))
						{
							LOGMASKED(LOG_PM_FAULT_SS, "CALL: Call gate: New stack has no room for 32-bit return address and parameters.\n");
							FAULT(FAULT_SS,0) // #SS(0)
						}
						if(gate.offset > desc.limit)
						{
							LOGMASKED(LOG_PM_FAULT_GP, "CALL: Call gate: EIP is past segment limit.\n");
							FAULT(FAULT_GP,0) // #GP(0)
						}
					}
					else
					{
						if(newESP < ((gate.dword_count & 0x1f) + 8))
						{
							LOGMASKED(LOG_PM_FAULT_SS, "CALL: Call gate: New stack has no room for 16-bit return address and parameters.\n");
							FAULT(FAULT_SS,0) // #SS(0)
						}
						if((gate.offset & 0xffff) > desc.limit)
						{
							LOGMASKED(LOG_PM_FAULT_GP, "CALL: Call gate: IP is past segment limit.\n");
							FAULT(FAULT_GP,0) // #GP(0)
						}
					}
					selector = gate.selector;
					offset = gate.offset;

					m_CPL = (stack.flags >> 5) & 0x03;
					/* check for page fault at new stack */
					WRITE_TEST(stack.base+newESP-1);
					/* switch to new stack */
					oldSS = m_sreg[SS].selector;
					m_sreg[SS].selector = i386_get_stack_segment(m_CPL);
					if(operand32 != 0)
					{
						oldESP = REG32(ESP);
					}
					else
					{
						oldESP = REG16(SP);
					}
					i386_load_segment_descriptor(SS );
					REG32(ESP) = newESP;

					if(operand32 != 0)
					{
						PUSH32SEG(oldSS);
						PUSH32(oldESP);
					}
					else
					{
						PUSH16(oldSS);
						PUSH16(oldESP & 0xffff);
					}

					memset(&temp, 0, sizeof(temp));
					temp.selector = oldSS;
					i386_load_protected_mode_segment(&temp,nullptr);
					/* copy parameters from old stack to new stack */
					for(x=(gate.dword_count & 0x1f)-1;x>=0;x--)
					{
						uint32_t addr = oldESP + (operand32?(x*4):(x*2));
						addr = temp.base + (temp.d?addr:(addr&0xffff));
						if(operand32)
							PUSH32(READ32(addr));
						else
							PUSH16(READ16(addr));
					}
					SetRPL = 1;
				}
				else
				{
					/* same privilege */
					if (operand32 != 0)  // if 32-bit
					{
						uint32_t stkoff = (STACK_32BIT ? REG32(ESP) - 8 : (REG16(SP) - 8) & 0xffff);
						if(i386_limit_check(SS, stkoff))
						{
							LOGMASKED(LOG_PM_FAULT_SS, "CALL: Stack has no room for return address.\n");
							FAULT(FAULT_SS,0) // #SS(0)
						}
						selector = gate.selector;
						offset = gate.offset;
					}
					else
					{
						uint32_t stkoff = (STACK_32BIT ? REG32(ESP) - 4 : (REG16(SP) - 4) & 0xffff);
						if(i386_limit_check(SS, stkoff))
						{
							LOGMASKED(LOG_PM_FAULT_SS, "CALL: Stack has no room for return address.\n");
							FAULT(FAULT_SS,0) // #SS(0)
						}
						selector = gate.selector;
						offset = gate.offset & 0xffff;
					}
					if(offset > desc.limit)
					{
						LOGMASKED(LOG_PM_FAULT_GP, "CALL: EIP is past segment limit.\n");
						FAULT(FAULT_GP,0) // #GP(0)
					}
					SetRPL = 1;
				}
				break;
			case 0x05:  // task gate
				LOGMASKED(LOG_PM_EVENTS, "CALL: Task gate at %08x\n",m_pc);
				memset(&gate, 0, sizeof(gate));
				gate.segment = selector;
				i386_load_call_gate(&gate);
				DPL = gate.dpl;
				if(DPL < CPL)
				{
					LOGMASKED(LOG_PM_FAULT_TS, "CALL: Task Gate: Gate DPL is less than CPL.\n");
					FAULT(FAULT_TS,selector & ~0x03) // #TS(selector)
				}
				if(DPL < RPL)
				{
					LOGMASKED(LOG_PM_FAULT_TS, "CALL: Task Gate: Gate DPL is less than RPL.\n");
					FAULT(FAULT_TS,selector & ~0x03) // #TS(selector)
				}
				if((gate.ar & 0x0080) == 0)
				{
					LOGMASKED(LOG_PM_FAULT_NP, "CALL: Task Gate: Gate is not present.\n");
					FAULT(FAULT_NP,selector & ~0x03) // #NP(selector)
				}
				/* Check the TSS that the task gate points to */
				desc.selector = gate.selector;
				i386_load_protected_mode_segment(&desc,nullptr);
				if(gate.selector & 0x04)
				{
					LOGMASKED(LOG_PM_FAULT_TS, "CALL: Task Gate: TSS is not global.\n");
					FAULT(FAULT_TS,gate.selector & ~0x03) // #TS(selector)
				}
				else
				{
					if((gate.selector & ~0x07) > m_gdtr.limit)
					{
						LOGMASKED(LOG_PM_FAULT_TS, "CALL: Task Gate: TSS is past GDT limit.\n");
						FAULT(FAULT_TS,gate.selector & ~0x03) // #TS(selector)
					}
				}
				if(desc.flags & 0x0002)
				{
					LOGMASKED(LOG_PM_FAULT_TS, "CALL: Task Gate: TSS is busy.\n");
					FAULT(FAULT_TS,gate.selector & ~0x03) // #TS(selector)
				}
				if((desc.flags & 0x0080) == 0)
				{
					LOGMASKED(LOG_PM_FAULT_NP, "CALL: Task Gate: TSS is not present.\n");
					FAULT(FAULT_NP,gate.selector & ~0x03) // #TS(selector)
				}
				if(desc.flags & 0x08)
					i386_task_switch(desc.selector,1);  // with nesting
				else
					i286_task_switch(desc.selector,1);
				return;
			default:
				LOGMASKED(LOG_PM_FAULT_GP, "CALL: Invalid special segment type (%i) to jump to.\n",desc.flags & 0x000f);
				FAULT(FAULT_GP,selector & ~0x07)  // #GP(selector)
			}
		}
	}

	if(SetRPL != 0)
		selector = (selector & ~0x03) | m_CPL;

	uint32_t tempSP = REG32(ESP);
	try
	{
		// this is ugly but the alternative is worse
		if(operand32 == 0)
		{
			/* 16-bit operand size */
			PUSH16(m_sreg[CS].selector );
			PUSH16(m_eip & 0x0000ffff );
			m_sreg[CS].selector = selector;
			m_performed_intersegment_jump = 1;
			m_eip = offset;
			i386_load_segment_descriptor(CS);
		}
		else
		{
			/* 32-bit operand size */
			PUSH32SEG(m_sreg[CS].selector );
			PUSH32(m_eip );
			m_sreg[CS].selector = selector;
			m_performed_intersegment_jump = 1;
			m_eip = offset;
			i386_load_segment_descriptor(CS );
		}
	}
	catch(uint64_t e)
	{
		REG32(ESP) = tempSP;
		throw e;
	}

	CHANGE_PC(m_eip);
}

void i386_device::i386_protected_mode_retf(uint8_t count, uint8_t operand32)
{
	uint32_t newCS, newEIP;
	I386_SREG desc;
	uint8_t CPL, RPL, DPL;

	uint32_t ea = i386_translate(SS, (STACK_32BIT)?REG32(ESP):REG16(SP), 0);

	if(operand32 == 0)
	{
		newEIP = READ16(ea) & 0xffff;
		newCS = READ16(ea+2) & 0xffff;
	}
	else
	{
		newEIP = READ32(ea);
		newCS = READ32(ea+4) & 0xffff;
	}

	memset(&desc, 0, sizeof(desc));
	desc.selector = newCS;
	i386_load_protected_mode_segment(&desc,nullptr);
	CPL = m_CPL;  // current privilege level
	DPL = (desc.flags >> 5) & 0x03;  // descriptor privilege level
	RPL = newCS & 0x03;

	if(RPL < CPL)
	{
		LOGMASKED(LOG_PM_FAULT_GP, "RETF (%08x): Return segment RPL is less than CPL.\n",m_pc);
		FAULT(FAULT_GP,newCS & ~0x03)
	}

	if(RPL == CPL)
	{
		/* same privilege level */
		if((newCS & ~0x03) == 0)
		{
			LOGMASKED(LOG_PM_FAULT_GP, "RETF: Return segment is null.\n");
			FAULT(FAULT_GP,0)
		}
		if(newCS & 0x04)
		{
			if((newCS & ~0x07) >= m_ldtr.limit)
			{
				LOGMASKED(LOG_PM_FAULT_GP, "RETF: Return segment is past LDT limit.\n");
				FAULT(FAULT_GP,newCS & ~0x03)
			}
		}
		else
		{
			if((newCS & ~0x07) >= m_gdtr.limit)
			{
				LOGMASKED(LOG_PM_FAULT_GP, "RETF: Return segment is past GDT limit.\n");
				FAULT(FAULT_GP,newCS & ~0x03)
			}
		}
		if((desc.flags & 0x0018) != 0x0018)
		{
			LOGMASKED(LOG_PM_FAULT_GP, "RETF: Return segment is not a code segment.\n");
			FAULT(FAULT_GP,newCS & ~0x03)
		}
		if(desc.flags & 0x0004)
		{
			if(DPL > RPL)
			{
				LOGMASKED(LOG_PM_FAULT_GP, "RETF: Conforming code segment DPL is greater than CS RPL.\n");
				FAULT(FAULT_GP,newCS & ~0x03)
			}
		}
		else
		{
			if(DPL != RPL)
			{
				LOGMASKED(LOG_PM_FAULT_GP, "RETF: Non-conforming code segment DPL does not equal CS RPL.\n");
				FAULT(FAULT_GP,newCS & ~0x03)
			}
		}
		if((desc.flags & 0x0080) == 0)
		{
			LOGMASKED(LOG_PM_FAULT_NP, "RETF (%08x): Code segment is not present.\n",m_pc);
			FAULT(FAULT_NP,newCS & ~0x03)
		}
		if(newEIP > desc.limit)
		{
			LOGMASKED(LOG_PM_FAULT_GP, "RETF: EIP is past code segment limit.\n");
			FAULT(FAULT_GP,0)
		}
		if(operand32 == 0)
		{
			uint32_t offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
			if(i386_limit_check(SS,offset+count+3) != 0)
			{
				LOGMASKED(LOG_PM_FAULT_SS, "RETF (%08x): SP is past stack segment limit.\n",m_pc);
				FAULT(FAULT_SS,0)
			}
		}
		else
		{
			uint32_t offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
			if(i386_limit_check(SS,offset+count+7) != 0)
			{
				LOGMASKED(LOG_PM_FAULT_SS, "RETF: ESP is past stack segment limit.\n");
				FAULT(FAULT_SS,0)
			}
		}
		if(STACK_32BIT)
			REG32(ESP) += (operand32 ? 8 : 4) + count;
		else
			REG16(SP) +=  (operand32 ? 8 : 4) + count;
	}
	else if(RPL > CPL)
	{
		uint32_t newSS, newESP;  // when changing privilege
		/* outer privilege level */
		if(operand32 == 0)
		{
			uint32_t offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
			if(i386_limit_check(SS,offset+count+7) != 0)
			{
				LOGMASKED(LOG_PM_FAULT_SS, "RETF (%08x): SP is past stack segment limit.\n",m_pc);
				FAULT(FAULT_SS,0)
			}
		}
		else
		{
			uint32_t offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
			if(i386_limit_check(SS,offset+count+15) != 0)
			{
				LOGMASKED(LOG_PM_FAULT_SS, "RETF: ESP is past stack segment limit.\n");
				FAULT(FAULT_SS,0)
			}
		}
		/* Check CS selector and descriptor */
		if((newCS & ~0x03) == 0)
		{
			LOGMASKED(LOG_PM_FAULT_GP, "RETF: CS segment is null.\n");
			FAULT(FAULT_GP,0)
		}
		if(newCS & 0x04)
		{
			if((newCS & ~0x07) >= m_ldtr.limit)
			{
				LOGMASKED(LOG_PM_FAULT_GP, "RETF: CS segment selector is past LDT limit.\n");
				FAULT(FAULT_GP,newCS & ~0x03)
			}
		}
		else
		{
			if((newCS & ~0x07) >= m_gdtr.limit)
			{
				LOGMASKED(LOG_PM_FAULT_GP, "RETF: CS segment selector is past GDT limit.\n");
				FAULT(FAULT_GP,newCS & ~0x03)
			}
		}
		if((desc.flags & 0x0018) != 0x0018)
		{
			LOGMASKED(LOG_PM_FAULT_GP, "RETF: CS segment is not a code segment.\n");
			FAULT(FAULT_GP,newCS & ~0x03)
		}
		if(desc.flags & 0x0004)
		{
			if(DPL > RPL)
			{
				LOGMASKED(LOG_PM_FAULT_GP, "RETF: Conforming CS segment DPL is greater than return selector RPL.\n");
				FAULT(FAULT_GP,newCS & ~0x03)
			}
		}
		else
		{
			if(DPL != RPL)
			{
				LOGMASKED(LOG_PM_FAULT_GP, "RETF: Non-conforming CS segment DPL is not equal to return selector RPL.\n");
				FAULT(FAULT_GP,newCS & ~0x03)
			}
		}
		if((desc.flags & 0x0080) == 0)
		{
			LOGMASKED(LOG_PM_FAULT_NP, "RETF: CS segment is not present.\n");
			FAULT(FAULT_NP,newCS & ~0x03)
		}
		if(newEIP > desc.limit)
		{
			LOGMASKED(LOG_PM_FAULT_GP, "RETF: EIP is past return CS segment limit.\n");
			FAULT(FAULT_GP,0)
		}

		if(operand32 == 0)
		{
			ea += count+4;
			newESP = READ16(ea) & 0xffff;
			newSS = READ16(ea+2) & 0xffff;
		}
		else
		{
			ea += count+8;
			newESP = READ32(ea);
			newSS = READ32(ea+4) & 0xffff;
		}

		/* Check SS selector and descriptor */
		desc.selector = newSS;
		i386_load_protected_mode_segment(&desc,nullptr);
		DPL = (desc.flags >> 5) & 0x03;  // descriptor privilege level
		if((newSS & ~0x07) == 0)
		{
			LOGMASKED(LOG_PM_FAULT_GP, "RETF: SS segment is null.\n");
			FAULT(FAULT_GP,0)
		}
		if(newSS & 0x04)
		{
			if((newSS & ~0x07) > m_ldtr.limit)
			{
				LOGMASKED(LOG_PM_FAULT_GP, "RETF (%08x): SS segment selector is past LDT limit.\n",m_pc);
				FAULT(FAULT_GP,newSS & ~0x03)
			}
		}
		else
		{
			if((newSS & ~0x07) > m_gdtr.limit)
			{
				LOGMASKED(LOG_PM_FAULT_GP, "RETF (%08x): SS segment selector is past GDT limit.\n",m_pc);
				FAULT(FAULT_GP,newSS & ~0x03)
			}
		}
		if((newSS & 0x03) != RPL)
		{
			LOGMASKED(LOG_PM_FAULT_GP, "RETF: SS segment RPL is not equal to CS segment RPL.\n");
			FAULT(FAULT_GP,newSS & ~0x03)
		}
		if((desc.flags & 0x0018) != 0x0010 || (desc.flags & 0x0002) == 0)
		{
			LOGMASKED(LOG_PM_FAULT_GP, "RETF: SS segment is not a writable data segment.\n");
			FAULT(FAULT_GP,newSS & ~0x03)
		}
		if(((desc.flags >> 5) & 0x03) != RPL)
		{
			LOGMASKED(LOG_PM_FAULT_GP, "RETF: SS DPL is not equal to CS segment RPL.\n");
			FAULT(FAULT_GP,newSS & ~0x03)
		}
		if((desc.flags & 0x0080) == 0)
		{
			LOGMASKED(LOG_PM_FAULT_GP, "RETF: SS segment is not present.\n");
			FAULT(FAULT_GP,newSS & ~0x03)
		}
		m_CPL = newCS & 0x03;

		/* Load new SS:(E)SP */
		if(operand32 == 0)
			REG16(SP) = (newESP+count) & 0xffff;
		else
			REG32(ESP) = newESP+count;
		m_sreg[SS].selector = newSS;
		i386_load_segment_descriptor(SS );

		/* Check that DS, ES, FS and GS are valid for the new privilege level */
		i386_check_sreg_validity(DS);
		i386_check_sreg_validity(ES);
		i386_check_sreg_validity(FS);
		i386_check_sreg_validity(GS);
	}

	/* Load new CS:(E)IP */
	if(operand32 == 0)
		m_eip = newEIP & 0xffff;
	else
		m_eip = newEIP;
	m_sreg[CS].selector = newCS;
	i386_load_segment_descriptor(CS );
	CHANGE_PC(m_eip);
}

void i386_device::i386_protected_mode_iret(int operand32)
{
	uint32_t newCS, newEIP;
	uint32_t newSS, newESP;  // when changing privilege
	I386_SREG desc,stack;
	uint8_t CPL, RPL, DPL;
	uint32_t newflags;
	uint8_t IOPL = m_IOP1 | (m_IOP2 << 1);

	CPL = m_CPL;
	uint32_t ea = i386_translate(SS, (STACK_32BIT)?REG32(ESP):REG16(SP), 0);
	if(operand32 == 0)
	{
		newEIP = READ16(ea) & 0xffff;
		newCS = READ16(ea+2) & 0xffff;
		newflags = READ16(ea+4) & 0xffff;
	}
	else
	{
		newEIP = READ32(ea);
		newCS = READ32(ea+4) & 0xffff;
		newflags = READ32(ea+8);
	}

	if(V8086_MODE)
	{
		uint32_t oldflags = get_flags();
		if(IOPL != 3)
		{
			LOGMASKED(LOG_PM_FAULT_GP, "IRET (%08x): Is in Virtual 8086 mode and IOPL != 3.\n",m_pc);
			FAULT(FAULT_GP,0)
		}
		if(operand32 == 0)
		{
			m_eip = newEIP & 0xffff;
			m_sreg[CS].selector = newCS & 0xffff;
			newflags &= ~(3<<12);
			newflags |= (((oldflags>>12)&3)<<12);  // IOPL cannot be changed in V86 mode
			set_flags((newflags & 0xffff) | (oldflags & ~0xffff));
			REG16(SP) += 6;
		}
		else
		{
			m_eip = newEIP;
			m_sreg[CS].selector = newCS & 0xffff;
			newflags &= ~(3<<12);
			newflags |= 0x20000 | (((oldflags>>12)&3)<<12);  // IOPL and VM cannot be changed in V86 mode
			set_flags(newflags);
			REG32(ESP) += 12;
		}
	}
	else if(NESTED_TASK)
	{
		uint32_t task = READ32(m_task.base);
		/* Task Return */
		LOGMASKED(LOG_PM_EVENTS, "IRET (%08x): Nested task return.\n",m_pc);
		/* Check back-link selector in TSS */
		if(task & 0x04)
		{
			LOGMASKED(LOG_PM_FAULT_TS, "IRET: Task return: Back-linked TSS is not in GDT.\n");
			FAULT(FAULT_TS,task & ~0x03)
		}
		if((task & ~0x07) >= m_gdtr.limit)
		{
			LOGMASKED(LOG_PM_FAULT_TS, "IRET: Task return: Back-linked TSS is not in GDT.\n");
			FAULT(FAULT_TS,task & ~0x03)
		}
		memset(&desc, 0, sizeof(desc));
		desc.selector = task;
		i386_load_protected_mode_segment(&desc,nullptr);
		if((desc.flags & 0x001f) != 0x000b)
		{
			LOGMASKED(LOG_PM_FAULT_TS, "IRET (%08x): Task return: Back-linked TSS is not a busy TSS.\n",m_pc);
			FAULT(FAULT_TS,task & ~0x03)
		}
		if((desc.flags & 0x0080) == 0)
		{
			LOGMASKED(LOG_PM_FAULT_NP, "IRET: Task return: Back-linked TSS is not present.\n");
			FAULT(FAULT_NP,task & ~0x03)
		}
		if(desc.flags & 0x08)
			i386_task_switch(desc.selector,0);
		else
			i286_task_switch(desc.selector,0);
		return;
	}
	else
	{
		if(newflags & 0x00020000) // if returning to virtual 8086 mode
		{
			// 16-bit iret can't reach here
			newESP = READ32(ea+12);
			newSS = READ32(ea+16) & 0xffff;
			/* Return to v86 mode */
			LOGMASKED(LOG_PM_EVENTS, "IRET (%08x): Returning to Virtual 8086 mode.\n",m_pc);
			if(CPL != 0)
			{
				uint32_t oldflags = get_flags();
				newflags = (newflags & ~0x00003000) | (oldflags & 0x00003000);
				if(CPL > IOPL)
					newflags = (newflags & ~0x200 ) | (oldflags & 0x200);
			}
			set_flags(newflags);
			m_eip = POP32() & 0xffff;  // high 16 bits are ignored
			m_sreg[CS].selector = POP32() & 0xffff;
			POP32();  // already set flags
			newESP = POP32();
			newSS = POP32() & 0xffff;
			m_sreg[ES].selector = POP32() & 0xffff;
			m_sreg[DS].selector = POP32() & 0xffff;
			m_sreg[FS].selector = POP32() & 0xffff;
			m_sreg[GS].selector = POP32() & 0xffff;
			REG32(ESP) = newESP;  // all 32 bits are loaded
			m_sreg[SS].selector = newSS;
			i386_load_segment_descriptor(ES);
			i386_load_segment_descriptor(DS);
			i386_load_segment_descriptor(FS);
			i386_load_segment_descriptor(GS);
			i386_load_segment_descriptor(SS);
			m_CPL = 3;  // Virtual 8086 tasks are always run at CPL 3
		}
		else
		{
			if(operand32 == 0)
			{
				uint32_t offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
				if(i386_limit_check(SS,offset+3) != 0)
				{
					LOGMASKED(LOG_PM_FAULT_SS, "IRET: Data on stack is past SS limit.\n");
					FAULT(FAULT_SS,0)
				}
			}
			else
			{
				uint32_t offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
				if(i386_limit_check(SS,offset+7) != 0)
				{
					LOGMASKED(LOG_PM_FAULT_SS, "IRET: Data on stack is past SS limit.\n");
					FAULT(FAULT_SS,0)
				}
			}
			RPL = newCS & 0x03;
			if(RPL < CPL)
			{
				LOGMASKED(LOG_PM_FAULT_GP, "IRET (%08x): Return CS RPL is less than CPL.\n",m_pc);
				FAULT(FAULT_GP,newCS & ~0x03)
			}
			if(RPL == CPL)
			{
				/* return to same privilege level */
				if(operand32 == 0)
				{
					uint32_t offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
					if(i386_limit_check(SS,offset+5) != 0)
					{
						LOGMASKED(LOG_PM_FAULT_SS, "IRET (%08x): Data on stack is past SS limit.\n",m_pc);
						FAULT(FAULT_SS,0)
					}
				}
				else
				{
					uint32_t offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
					if(i386_limit_check(SS,offset+11) != 0)
					{
						LOGMASKED(LOG_PM_FAULT_SS, "IRET (%08x): Data on stack is past SS limit.\n",m_pc);
						FAULT(FAULT_SS,0)
					}
				}
				if((newCS & ~0x03) == 0)
				{
					LOGMASKED(LOG_PM_FAULT_GP, "IRET: Return CS selector is null.\n");
					FAULT(FAULT_GP,0)
				}
				if(newCS & 0x04)
				{
					if((newCS & ~0x07) >= m_ldtr.limit)
					{
						LOGMASKED(LOG_PM_FAULT_GP, "IRET: Return CS selector (%04x) is past LDT limit.\n",newCS);
						FAULT(FAULT_GP,newCS & ~0x03)
					}
				}
				else
				{
					if((newCS & ~0x07) >= m_gdtr.limit)
					{
						LOGMASKED(LOG_PM_FAULT_GP, "IRET: Return CS selector is past GDT limit.\n");
						FAULT(FAULT_GP,newCS & ~0x03)
					}
				}
				memset(&desc, 0, sizeof(desc));
				desc.selector = newCS;
				i386_load_protected_mode_segment(&desc,nullptr);
				DPL = (desc.flags >> 5) & 0x03;  // descriptor privilege level
				RPL = newCS & 0x03;
				if((desc.flags & 0x0018) != 0x0018)
				{
					LOGMASKED(LOG_PM_FAULT_GP, "IRET (%08x): Return CS segment is not a code segment.\n",m_pc);
					FAULT(FAULT_GP,newCS & ~0x07)
				}
				if(desc.flags & 0x0004)
				{
					if(DPL > RPL)
					{
						LOGMASKED(LOG_PM_FAULT_GP, "IRET: Conforming return CS DPL is greater than CS RPL.\n");
						FAULT(FAULT_GP,newCS & ~0x03)
					}
				}
				else
				{
					if(DPL != RPL)
					{
						LOGMASKED(LOG_PM_FAULT_GP, "IRET: Non-conforming return CS DPL is not equal to CS RPL.\n");
						FAULT(FAULT_GP,newCS & ~0x03)
					}
				}
				if((desc.flags & 0x0080) == 0)
				{
					LOGMASKED(LOG_PM_FAULT_NP, "IRET: (%08x) Return CS segment is not present.\n", m_pc);
					FAULT(FAULT_NP,newCS & ~0x03)
				}
				if(newEIP > desc.limit)
				{
					LOGMASKED(LOG_PM_FAULT_GP, "IRET: Return EIP is past return CS limit.\n");
					FAULT(FAULT_GP,0)
				}

				if(CPL != 0)
				{
					uint32_t oldflags = get_flags();
					newflags = (newflags & ~0x00003000) | (oldflags & 0x00003000);
					if(CPL > IOPL)
						newflags = (newflags & ~0x200 ) | (oldflags & 0x200);
				}

				if(operand32 == 0)
				{
					m_eip = newEIP;
					m_sreg[CS].selector = newCS;
					set_flags(newflags);
					REG16(SP) += 6;
				}
				else
				{
					m_eip = newEIP;
					m_sreg[CS].selector = newCS & 0xffff;
					set_flags(newflags);
					REG32(ESP) += 12;
				}
			}
			else if(RPL > CPL)
			{
				/* return to outer privilege level */
				memset(&desc, 0, sizeof(desc));
				desc.selector = newCS;
				i386_load_protected_mode_segment(&desc,nullptr);
				DPL = (desc.flags >> 5) & 0x03;  // descriptor privilege level
				RPL = newCS & 0x03;
				if(operand32 == 0)
				{
					uint32_t offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
					if(i386_limit_check(SS,offset+9) != 0)
					{
						LOGMASKED(LOG_PM_FAULT_SS, "IRET: SP is past SS limit.\n");
						FAULT(FAULT_SS,0)
					}
				}
				else
				{
					uint32_t offset = (STACK_32BIT ? REG32(ESP) : REG16(SP));
					if(i386_limit_check(SS,offset+19) != 0)
					{
						LOGMASKED(LOG_PM_FAULT_SS, "IRET: ESP is past SS limit.\n");
						FAULT(FAULT_SS,0)
					}
				}
				/* Check CS selector and descriptor */
				if((newCS & ~0x03) == 0)
				{
					LOGMASKED(LOG_PM_FAULT_GP, "IRET: Return CS selector is null.\n");
					FAULT(FAULT_GP,0)
				}
				if(newCS & 0x04)
				{
					if((newCS & ~0x07) >= m_ldtr.limit)
					{
						LOGMASKED(LOG_PM_FAULT_GP, "IRET: Return CS selector is past LDT limit.\n");
						FAULT(FAULT_GP,newCS & ~0x03);
					}
				}
				else
				{
					if((newCS & ~0x07) >= m_gdtr.limit)
					{
						LOGMASKED(LOG_PM_FAULT_GP, "IRET: Return CS selector is past GDT limit.\n");
						FAULT(FAULT_GP,newCS & ~0x03);
					}
				}
				if((desc.flags & 0x0018) != 0x0018)
				{
					LOGMASKED(LOG_PM_FAULT_GP, "IRET: Return CS segment is not a code segment.\n");
					FAULT(FAULT_GP,newCS & ~0x03)
				}
				if(desc.flags & 0x0004)
				{
					if(DPL > RPL)
					{
						LOGMASKED(LOG_PM_FAULT_GP, "IRET: Conforming return CS DPL is greater than CS RPL.\n");
						FAULT(FAULT_GP,newCS & ~0x03)
					}
				}
				else
				{
					if(DPL != RPL)
					{
						LOGMASKED(LOG_PM_FAULT_GP, "IRET: Non-conforming return CS DPL does not equal CS RPL.\n");
						FAULT(FAULT_GP,newCS & ~0x03)
					}
				}
				if((desc.flags & 0x0080) == 0)
				{
					LOGMASKED(LOG_PM_FAULT_NP, "IRET: Return CS segment is not present.\n");
					FAULT(FAULT_NP,newCS & ~0x03)
				}

				/* Check SS selector and descriptor */
				if(operand32 == 0)
				{
					newESP = READ16(ea+6) & 0xffff;
					newSS = READ16(ea+8) & 0xffff;
				}
				else
				{
					newESP = READ32(ea+12);
					newSS = READ32(ea+16) & 0xffff;
				}
				memset(&stack, 0, sizeof(stack));
				stack.selector = newSS;
				i386_load_protected_mode_segment(&stack,nullptr);
				DPL = (stack.flags >> 5) & 0x03;
				if((newSS & ~0x03) == 0)
				{
					LOGMASKED(LOG_PM_FAULT_GP, "IRET: Return SS selector is null.\n");
					FAULT(FAULT_GP,0)
				}
				if(newSS & 0x04)
				{
					if((newSS & ~0x07) >= m_ldtr.limit)
					{
						LOGMASKED(LOG_PM_FAULT_GP, "IRET: Return SS selector is past LDT limit.\n");
						FAULT(FAULT_GP,newSS & ~0x03);
					}
				}
				else
				{
					if((newSS & ~0x07) >= m_gdtr.limit)
					{
						LOGMASKED(LOG_PM_FAULT_GP, "IRET: Return SS selector is past GDT limit.\n");
						FAULT(FAULT_GP,newSS & ~0x03);
					}
				}
				if((newSS & 0x03) != RPL)
				{
					LOGMASKED(LOG_PM_FAULT_GP, "IRET: Return SS RPL is not equal to return CS RPL.\n");
					FAULT(FAULT_GP,newSS & ~0x03)
				}
				if((stack.flags & 0x0018) != 0x0010)
				{
					LOGMASKED(LOG_PM_FAULT_GP, "IRET: Return SS segment is not a data segment.\n");
					FAULT(FAULT_GP,newSS & ~0x03)
				}
				if((stack.flags & 0x0002) == 0)
				{
					LOGMASKED(LOG_PM_FAULT_GP, "IRET: Return SS segment is not writable.\n");
					FAULT(FAULT_GP,newSS & ~0x03)
				}
				if(DPL != RPL)
				{
					LOGMASKED(LOG_PM_FAULT_GP, "IRET: Return SS DPL does not equal SS RPL.\n");
					FAULT(FAULT_GP,newSS & ~0x03)
				}
				if((stack.flags & 0x0080) == 0)
				{
					LOGMASKED(LOG_PM_FAULT_NP, "IRET: Return SS segment is not present.\n");
					FAULT(FAULT_NP,newSS & ~0x03)
				}
				if(newEIP > desc.limit)
				{
					LOGMASKED(LOG_PM_FAULT_GP, "IRET: EIP is past return CS limit.\n");
					FAULT(FAULT_GP,0)
				}

//              if(operand32 == 0)
//                  REG16(SP) += 10;
//              else
//                  REG32(ESP) += 20;

				// IOPL can only change if CPL is zero
				if(CPL != 0)
				{
					uint32_t oldflags = get_flags();
					newflags = (newflags & ~0x00003000) | (oldflags & 0x00003000);
					if(CPL > IOPL)
						newflags = (newflags & ~0x200 ) | (oldflags & 0x200);
				}

				if(operand32 == 0)
				{
					m_eip = newEIP & 0xffff;
					m_sreg[CS].selector = newCS;
					set_flags(newflags);
					REG16(SP) = newESP & 0xffff;
					m_sreg[SS].selector = newSS;
				}
				else
				{
					m_eip = newEIP;
					m_sreg[CS].selector = newCS & 0xffff;
					set_flags(newflags);
					REG32(ESP) = newESP;
					m_sreg[SS].selector = newSS & 0xffff;
				}
				m_CPL = newCS & 0x03;
				i386_load_segment_descriptor(SS);

				/* Check that DS, ES, FS and GS are valid for the new privilege level */
				i386_check_sreg_validity(DS);
				i386_check_sreg_validity(ES);
				i386_check_sreg_validity(FS);
				i386_check_sreg_validity(GS);
			}
		}
	}

	i386_load_segment_descriptor(CS);
	CHANGE_PC(m_eip);
}

inline void i386_device::dri_changed()
{
	int dr;

	if(m_dri_changed_active)
		return;
	m_dri_changed_active = true;

	for(dr = 0; dr < 4; dr++)
	{
		m_dr_breakpoints[dr].remove();
		int dr_enabled = (m_dr[7] & (1 << (dr << 1))) || (m_dr[7] & (1 << ((dr << 1) + 1))); // Check both local enable AND global enable bits for this breakpoint.
		if(dr_enabled)
		{
			int breakpoint_type = (m_dr[7] >> ((dr << 2) + 16)) & 3;
			int breakpoint_length = (m_dr[7] >> ((dr << 2) + 16 + 2)) & 3;
			uint32_t phys_addr = m_dr[dr];
			uint32_t error;
			if(translate_address(m_CPL, TR_READ, &phys_addr, &error))
			{
				phys_addr &= ~3; // According to CUP386, data breakpoints are only reliable on dword-aligned addresses, so align this to a dword.
				uint32_t true_mask = 0;
				switch(breakpoint_length)
				{
					case 0: true_mask = 0xff; break;
					case 1: true_mask = 0xffff; break;
					// Case 2 is invalid on a real 386.
					case 3: true_mask = 0xffffffff; break;
				}
				if(true_mask == 0)
				{
					logerror("i386: Unknown breakpoint length value\n");
				}
				else if(breakpoint_type == 1)
				{
					m_dr_breakpoints[dr] = m_program->install_write_tap(
							phys_addr,
							phys_addr + 3,
							"i386_debug_write_breakpoint",
							[this, dr, true_mask](offs_t offset, u32& data, u32 mem_mask)
							{
								if(true_mask & mem_mask)
								{
									m_dr[6] |= 1 << dr;
									i386_trap(1,1,0);
								}
							},
							&m_dr_breakpoints[dr]);
				}
				else if(breakpoint_type == 3)
				{
					m_dr_breakpoints[dr] = m_program->install_readwrite_tap(
							phys_addr,
							phys_addr + 3,
							"i386_debug_readwrite_breakpoint",
							[this, dr, true_mask](offs_t offset, u32& data, u32 mem_mask)
							{
								if(true_mask & mem_mask)
								{
									m_dr[6] |= 1 << dr;
									i386_trap(1,1,0);
								}
							},
							[this, dr, true_mask](offs_t offset, u32& data, u32 mem_mask)
							{
								if(true_mask & mem_mask)
								{
									m_dr[6] |= 1 << dr;
									i386_trap(1,1,0);
								}
							},
							&m_dr_breakpoints[dr]);
				}
			}
		}
	}
	m_dri_changed_active = false;
}

inline void i386_device::dr7_changed(uint32_t old_val, uint32_t new_val)
{
	//Check that none of the breakpoint types or lengths have changed.
	int old_breakpoint_info = old_val >> 16;
	int new_breakpoint_info = new_val >> 16;
	if(old_breakpoint_info != new_breakpoint_info) dri_changed();
}

