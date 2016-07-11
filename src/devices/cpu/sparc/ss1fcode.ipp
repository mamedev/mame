// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//================================================================
//
//  ss1fcode.ipp - Logging support for execution of the Forth VM
//                 used in the Sun SPARCstation 1.
//
//================================================================

void mb86901_device::log_fcodes()
{
	if (PC != 0xffef0000 && PC != m_ss1_next_entry_point)
		return;
	
	if (PC == 0xffef0000)
	{
		UINT32 opcode = read_sized_word(11, REG(5), 2);
		if (!(REG(5) & 2))
		{
			opcode >>= 16;
		}

		if (opcode == 0x4627)
		{
			m_log_fcodes = true;
			//machine().debugger().debug_break();
		}
		else if (!m_log_fcodes)
		{
			return;
		}
		else if (opcode == 0x0cb8)
		{
			m_log_fcodes = false;
		}
		UINT32 handler_base = opcode << 2;
		handler_base += REG(2); // l1 = r2 + opcode << 2

		UINT32 entry_point = read_sized_word(11, handler_base, 2);
		if (!(handler_base & 2))
		{
			entry_point >>= 16;
		}
		entry_point <<= 2;
		entry_point += REG(2); // l0 = r2 + entry_point << 2

		disassemble_ss1_fcode(REG(5), opcode, handler_base, entry_point, REG(7));
	}
	else if (PC == m_ss1_next_entry_point && m_log_fcodes)
	{
		disassemble_ss1_fcode(m_ss1_next_pc, m_ss1_next_opcode, m_ss1_next_handler_base, m_ss1_next_entry_point, m_ss1_next_stack);
		m_ss1_next_pc = ~0;
		m_ss1_next_opcode = ~0;
		m_ss1_next_handler_base = ~0;
		m_ss1_next_entry_point = ~0;
		m_ss1_next_stack = ~0;
	}
}

void mb86901_device::indent()
{
	UINT32 program_depth = (0xffeff000 - (REG(6) - 4)) / 4;

	if (program_depth < 15)
		return;
	program_depth -= 15;
	
	for (int i = 0; i < program_depth; i++)
	{
		printf("    ");
	}
}

void mb86901_device::disassemble_ss1_fcode(UINT32 r5, UINT32 opcode, UINT32 handler_base, UINT32 entry_point, UINT32 stack)
{
	std::string opdesc = m_ss1_fcode_table[opcode];
	if (opdesc.length() == 0)
		opdesc = "[unknown]";
	
	indent(); printf("\n");
	indent(); printf("Stacks before this forth code:\n");
	indent(); printf("Data                    Return\n");
	indent(); printf("--------                --------\n");

	UINT32 data_stack[8];
	UINT32 return_stack[8];

	UINT32 data_entries = (0xffefebe4 - (stack - 4)) / 4;
	UINT32 return_entries = (0xffeff000 - (REG(6) - 4)) / 4;

	data_stack[0] = REG(4);
	for (int i = 0; i < data_entries && i < 7; i++)
	{
		data_stack[i + 1] = read_sized_word(11, REG(7) + i * 4, 4);
	}
	data_entries++;

	for (int i = 0; i < return_entries && i < 8; i++)
	{
		return_stack[i] = read_sized_word(11, REG(6) + i * 4, 4);
	}

	UINT32 total_lines = 0;
	if (data_entries > total_lines)
		total_lines = data_entries;

	if (return_entries > total_lines)
		total_lines = return_entries;

	if (total_lines > 8)
		total_lines = 8;

	for (int i = 0; i < total_lines; i++)
	{
		indent();
		if (i < data_entries)
			printf("%08x", data_stack[i]);
		else
			printf("        ");

		if (i < return_entries)
			printf("                %08x\n", return_stack[i]);
		else
			printf("\n");
	}

	UINT32 base = 0xffe87954;
	UINT32 exact_op = (r5 - base) / 4;
	UINT32 base_op = exact_op;
	while (m_ss1_fcode_table[base_op].length() == 0)
		base_op--;
	UINT32 dist = (exact_op - base_op) * 4;
	
	if (entry_point == 0xffe87964)
	{
		indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; call %08x\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), handler_base + 2);
		indent(); printf("                                               // program stack now %08x (%d words deep)\n", REG(6) - 4, (0xffeff000 - (REG(6) - 4)) / 4);
	}
	else if (entry_point == 0xffe87974)
	{
		indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; push handler_base+2 (%08x)\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), handler_base + 2);
		indent(); printf("                                               // data stack now %08x (%d words deep)\n", stack - 4, (0xffefebe4 - (stack - 4)) / 4);
	}
	else if (entry_point == 0xffe8799c)
	{
		UINT32 address = handler_base + 2;
		UINT32 half = read_sized_word(11, address, 2);
		if (!(address & 2)) half >>= 16;

		indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; add halfword at handler_base+2 (%04x) to VM base pointer (%08x) and push onto stack (%08x)\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), half, REG(3), REG(3) + half);
		indent(); printf("                                               // data stack now %08x (%d words deep)\n", stack - 4, (0xffefebe4 - (stack - 4)) / 4);
	}
	else if (entry_point == 0xffe879e4)
	{
		UINT32 address = handler_base + 2;
		UINT32 half0 = read_sized_word(11, address, 2);
		if (address & 2) half0 <<= 16;

		address = handler_base + 4;
		UINT32 half1 = read_sized_word(11, address, 2);
		if (!(address & 2)) half1 >>= 16;

		UINT32 value = half0 | half1;

		indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; push immediate word from handler table (%08x)\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), value);
		indent(); printf("                                               // data stack now %08x (%d words deep)\n", stack - 4, (0xffefebe4 - (stack - 4)) / 4);
	}
	else if (entry_point == 0xffe879c4)
	{
		UINT32 address = handler_base + 2;
		UINT32 l0 = read_sized_word(11, address, 2);
		if (!(address & 2)) l0 >>= 16;

		address = REG(3) + l0;
		UINT32 handler_base_2 = read_sized_word(11, address, 2);
		if (!(address & 2)) handler_base_2 >>= 16;

		address = REG(2) + (handler_base_2 << 2);
		UINT32 l0_2 = read_sized_word(11, address, 2);
		if (!(address & 2)) l0_2 >>= 16;

		UINT32 dest = REG(2) + (l0_2 << 2);

		indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; SPARC branch to %08x, calcs: g2(%08x) + halfword[g2(%04x) + (halfword[g3(%08x) + halfword[entry_point(%04x) + 2](%04x)](%04x) << 2)](%08x)\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), dest, REG(2), REG(2), REG(3), handler_base, l0, handler_base_2, l0_2);
		indent(); printf("                                               // target func: %08x\n", l0_2 << 2);
		switch (l0_2 << 2)
		{
			case 0x10: // call
				indent(); printf("                                               // call %08x\n", (REG(2) + (handler_base_2 << 2)) + 2);
				indent(); printf("                                               // program stack now %08x (%d words deep)\n", REG(6) - 4, (0xffeff000 - (REG(6) - 4)) / 4);
				break;
			default:
				indent(); printf("                                               // unknown handler address: %08x\n", REG(2) + (l0_2 << 2));
				break;
		}
	}
	else if (entry_point == 0xffe8c838)
	{
		UINT32 address = handler_base + 2;
		UINT32 half0 = read_sized_word(11, address, 2);
		if (address & 2) half0 <<= 16;

		address = handler_base + 4;
		UINT32 half1 = read_sized_word(11, address, 2);
		if (!(address & 2)) half1 >>= 16;

		UINT32 value = half0 | half1;

		indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; add 32-bit word (%08x) from handler table to top of stack (%08x + %08x = %08x)\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), value, REG(4), value, REG(4) + value);
	}
	else
	{
		switch(opcode)
		{
			case 0x003f:
			{
				UINT32 address = r5 + 2;
				UINT32 half0 = read_sized_word(11, address, 2);
				if (address & 2) half0 <<= 16;

				address = r5 + 4;
				UINT32 half1 = read_sized_word(11, address, 2);
				if (!(address & 2)) half1 >>= 16;

				UINT32 value = half0 | half1;

				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; push immediate word from instructions (%08x)\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), value);
				indent(); printf("                                               // data stack now %08x (%d words deep)\n", stack - 4, (0xffefebe4 - (stack - 4)) / 4);
				break;
			}

			case 0x004b:
			{
				UINT32 address = r5 + 2;
				UINT32 value = read_sized_word(11, address, 2);
				if (!(address & 2)) value >>= 16;

				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; push immediate halfword from instructions (%04x)\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), value);
				indent(); printf("                                               // data stack now %08x (%d words deep)\n", stack - 4, (0xffefebe4 - (stack - 4)) / 4);
				break;
			}

			case 0x0055:
			{
				UINT32 address = REG(4);
				UINT32 new_opcode = read_sized_word(11, address, 2);
				if (!(address & 2)) new_opcode >>= 16;

				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; pop stack top (%08x) as an opcode to execute\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), REG(4));
				indent(); printf("                                               // data stack now %08x (%d words deep)\n", stack + 4, (0xffefebe4 - (stack + 4)) / 4);
				indent(); printf("                                               // inserted opcode:\n");

				m_ss1_next_pc = REG(4);
				m_ss1_next_opcode = new_opcode;
				m_ss1_next_handler_base = REG(4);

				m_ss1_next_entry_point = read_sized_word(11, m_ss1_next_handler_base, 2);
				if (!(m_ss1_next_handler_base & 2))
				{
					m_ss1_next_entry_point >>= 16;
				}
				m_ss1_next_entry_point <<= 2;
				m_ss1_next_entry_point += REG(2);
				m_ss1_next_stack = stack + 4;
				break;
			}

			case 0x05f:
			{
				UINT32 address = r5 + 2;
				UINT32 pc_offset = read_sized_word(11, address, 2);
				if (!(address & 2)) pc_offset >>= 16;

				// advance program counter by amount specified as parameter
				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; advance program counter by amount specified as parameter (%08x = %08x + %04x)\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), r5 + 2 + pc_offset, r5 + 2, pc_offset);
				break;
			}

			case 0x0066:
			{
				UINT32 address = r5 + 2;
				UINT32 offset = read_sized_word(11, address, 2);
				if (!(address & 2)) offset >>= 16;

				UINT32 target = r5 + 2 + offset;

				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; pop data stack top (%08x) and if zero, jump to %08x\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), read_sized_word(11, REG(7), 4), target);
				indent(); printf("                                               // data stack now %08x (%d words deep)\n", stack + 4, (0xffefebe4 - (stack + 4)) / 4);
				break;
			}

			case 0x007b:
			{
				UINT32 r4 = REG(4);
				UINT32 value = read_sized_word(11, REG(6), 4);
				UINT32 result = value + r4;
				bool arithmetic_overflow = ((BIT31(value) && BIT31(r4) && !BIT31(result)) || (!BIT31(value) && !BIT31(r4) && BIT31(result)));

				UINT32 address = r5 + 2;
				UINT32 offset = read_sized_word(11, address, 2);
				if (!(address & 2)) offset >>= 16;
				UINT32 target = r5 + 2 + offset;

				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; pop data stack top and add to program stack top (%08x = %08x + %08x)\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), result, value, r4);
				indent(); printf("                                               // if no addition overflow, jump to %08x\n", target);
				indent(); printf("                                               // if addition overflow, pop 3 words off program stack\n");
				if (arithmetic_overflow)
				{
					indent(); printf("                                               // program stack now %08x (%d words deep)\n", REG(6) + 12, (0xffeff000 - (REG(6) + 12)) / 4);
				}
				break;
			}

			case 0x0099:
			{
				UINT32 handler_base_2 = REG(4);

				UINT32 address = stack;
				UINT32 l0_2 = read_sized_word(11, address, 4);

				address = stack + 4;
				UINT32 popped_g4 = read_sized_word(11, address, 4);

				address = r5 + 2;
				UINT32 offset = read_sized_word(11, address, 2);
				if (!(address & 2)) offset >>= 16;

				UINT32 target = r5 + 2 + offset;

				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; branch relative to %08x if data stack second (%08x) == data stack top (%08x), pop_data result (%08x)\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), target, l0_2, handler_base_2, popped_g4);
				if (handler_base_2 == l0_2)
				{
					indent(); printf("                                               // branch will be taken\n");
				}
				else
				{
					indent(); printf("                                               // branch will not be taken\n");
					indent(); printf("                                               // push pc (%08x) onto program stack\n", r5 + 2);
					indent(); printf("                                               // push previous data stack top + 0x80000000 (%08x) onto program stack\n", l0_2 + 0x80000000);
					indent(); printf("                                               // push diff of (result - stack top) (%08x = %08x - %08x) onto program stack\n", handler_base_2, l0_2 + 0x80000000, handler_base_2 - (l0_2 + 0x80000000));
					indent(); printf("                                               // program stack now %08x (%d words deep)\n", REG(6) - 12, (0xffeff000 - (REG(6) - 12)) / 4);
				}
				indent(); printf("                                               // data stack now %08x (%d words deep)\n", stack + 8, (0xffefebe4 - (stack + 8)) / 4);

				break;
			}

			case 0x00a4:
			{
				UINT32 word0 = read_sized_word(11, REG(6), 4);
				UINT32 word1 = read_sized_word(11, REG(6) + 4, 4);
				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; push result (%08x) to data stack, add the top two values on the program stack, store in result (%08x = %08x + %08x)\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), REG(4), word0 + word1, word0, word1);
				indent(); printf("                                               // data stack now %08x (%d words deep)\n", stack - 4, (0xffefebe4 - (stack - 4)) / 4);
				break;
			}

			case 0x0121:
			{
				UINT32 address = stack;
				UINT32 word = read_sized_word(11, address, 4);
				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; logical-AND result with data stack pop, store in result (%08x = %08x & %08x)\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), word & REG(4), word, REG(4));
				indent(); printf("                                               // data stack now %08x (%d words deep)\n", stack + 4, (0xffefebe4 - (stack + 4)) / 4);
				break;
			}

			case 0x0128:
			{
				UINT32 address = stack;
				UINT32 word = read_sized_word(11, address, 4);
				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; logical-OR result with data stack pop, store in result: %08x = %08x & %08x\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), word | REG(4), word, REG(4));
				indent(); printf("                                               // data stack now %08x (%d words deep)\n", stack + 4, (0xffefebe4 - (stack + 4)) / 4);
				break;
			}

			case 0x0136:
				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; invert result (%08x -> %08x)\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), REG(4), REG(4) ^ 0xffffffff);
				break;

			case 0x014f:
			{
				UINT32 address = stack;
				UINT32 word = read_sized_word(11, address, 4);
				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; add result to data stack pop, store in result: %08x = %08x + %08x\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), word + REG(4), word, REG(4));
				indent(); printf("                                               // data stack now %08x (%d words deep)\n", stack + 4, (0xffefebe4 - (stack + 4)) / 4);
				break;
			}

			case 0x0155:
			{
				UINT32 address = stack;
				UINT32 word = read_sized_word(11, address, 4);
				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; subtract result from data stack pop, store in result: %08x = %08x - %08x\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), word - REG(4), word, REG(4));
				indent(); printf("                                               // data stack now %08x (%d words deep)\n", stack + 4, (0xffefebe4 - (stack + 4)) / 4);
				break;
			}

			case 0x017f:
				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; push result (%08x) onto data stack, set result to address of opcode dispatcher\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), REG(4));
				indent(); printf("                                               // data stack now %08x (%d words deep)\n", stack - 4, (0xffefebe4 - (stack - 4)) / 4);
				break;

			case 0x01a9:
				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; push result (%08x) onto program stack, pop result from data stack (%08x)\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), REG(4), read_sized_word(11, stack, 4));
				indent(); printf("                                               // program stack now %08x (%d words deep)\n", REG(6) - 4, (0xffeff000 - (REG(6) - 4)) / 4);
				indent(); printf("                                               // data stack now %08x (%d words deep)\n", stack + 4, (0xffefebe4 - (stack + 4)) / 4);
				break;

			case 0x01b1:
				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; push result (%08x) onto data stack, pop result from program stack (%08x)\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), REG(4), read_sized_word(11, REG(6), 4));
				indent(); printf("                                               // program stack now %08x (%d words deep)\n", REG(6) + 4, (0xffeff000 - (REG(6) + 4)) / 4);
				indent(); printf("                                               // data stack now %08x (%d words deep)\n", stack - 4, (0xffefebe4 - (stack - 4)) / 4);
				break;

			case 0x01b9:
				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; push result (%08x) onto data stack, assign program stack top to result (%08x)\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), REG(4), read_sized_word(11, REG(6), 4));
				indent(); printf("                                               // data stack now %08x (%d words deep)\n", stack - 4, (0xffefebe4 - (stack - 4)) / 4);
				break;

			case 0x01c0:
			case 0x01c7:
			{
				UINT32 address = REG(6);
				UINT32 half0 = read_sized_word(11, address, 2);
				if (address & 2) half0 <<= 16;

				address = REG(6) + 2;
				UINT32 half1 = read_sized_word(11, address, 2);
				if (!(address & 2)) half1 >>= 16;

				UINT32 value = half0 | half1;

				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; return (%08x)\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), value);
				indent(); printf("                                               // program stack now %08x (%d words deep)\n", REG(6) + 4, (0xffeff000 - (REG(6) + 4)) / 4);
				break;
			}

			case 0x01cd:
				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; insert result (%08x) between data stack top (%08x) and next data stack entry\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), REG(4), read_sized_word(11, stack, 4));
				indent(); printf("                                               // data stack now %08x (%d words deep)\n", stack - 4, (0xffefebe4 - (stack - 4)) / 4);
				break;

			case 0x01d5:
				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; throw away the word at the top of the data stack\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str());
				indent(); printf("                                               // data stack now %08x (%d words deep)\n", stack + 4, (0xffefebe4 - (stack + 4)) / 4);
				break;

			case 0x01f4:
				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; if result (%08x) >= 0, set result to 0, otherwise -1 (%08x)\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), REG(4), (REG(4) >= 0) ? 0 : ~0);
				break;

			case 0x0217:
			{
				UINT32 value = read_sized_word(11, stack, 4);
				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; if pop_data (%08x) >= result (%08x), set result to 0, otherwise -1 (%08x)\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), value, REG(4), (value >= REG(4)) ? 0 : ~0);
				indent(); printf("                                               // data stack now %08x (%d words deep)\n", stack + 4, (0xffefebe4 - (stack + 4)) / 4);
				break;
			}

			case 0x022b:
			{
				UINT32 value = read_sized_word(11, stack, 4);
				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; if pop_data (%08x) != result (%08x), set result to 0, otherwise -1 (%08x)\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), value, REG(4), (value != REG(4)) ? 0 : ~0);
				indent(); printf("                                               // data stack now %08x (%d words deep)\n", stack + 4, (0xffefebe4 - (stack + 4)) / 4);
				break;
			}

			case 0x0236:
			{
				UINT32 value = read_sized_word(11, stack, 4);
				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; if pop_data (%08x) == result (%08x), set result to 0, otherwise -1 (%08x)\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), value, REG(4), (value == REG(4)) ? 0 : ~0);
				indent(); printf("                                               // data stack now %08x (%d words deep)\n", stack + 4, (0xffefebe4 - (stack + 4)) / 4);
				break;
			}

			case 0x026d:
			{
				UINT32 value = read_sized_word(11, stack, 4);
				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; if pop_data (%08x) < result (%08x), set result to 0, otherwise -1 (%08x)\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), value, REG(4), (value < REG(4)) ? 0 : ~0);
				indent(); printf("                                               // data stack now %08x (%d words deep)\n", stack + 4, (0xffefebe4 - (stack + 4)) / 4);
				break;
			}

			case 0x0278:
			{
				UINT32 value = read_sized_word(11, stack, 4);
				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; if pop_data (%08x) > result (%08x), set result to 0, otherwise -1 (%08x)\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), value, REG(4), (value > REG(4)) ? 0 : ~0);
				indent(); printf("                                               // data stack now %08x (%d words deep)\n", stack + 4, (0xffefebe4 - (stack + 4)) / 4);
				break;
			}

			case 0x0289:
				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; push result (%08x) to data stack\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), REG(4));
				indent(); printf("                                               // data stack now %08x (%d words deep)\n", stack - 4, (0xffefebe4 - (stack - 4)) / 4);
				break;

			case 0x0283:
				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; pop result from data stack (%08x)\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), read_sized_word(11, stack, 4));
				break;

			case 0x028f:
				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; push result (%08x) onto data stack, assign previous top (%08x) to result\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), REG(4), read_sized_word(11, stack, 4));
				indent(); printf("                                               // data stack now %08x (%d words deep)\n", stack - 4, (0xffefebe4 - (stack - 4)) / 4);
				break;

			case 0x0296:
				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; swap result (%08x) with top of data stack (%08x)\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), REG(4), read_sized_word(11, stack, 4));
				break;

			case 0x029d:
			{
				UINT32 top = read_sized_word(11, stack, 4);
				UINT32 next = read_sized_word(11, stack + 4, 4);
				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; swap the top two values of the data stack (%08x <-> %08x), exchange second value with result (%08x <-> %08x)\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), top, next, REG(4), next);
				break;
			}

			case 0x02af:
				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; pop_data to throw away data_top, then pop_data into result (%08x)\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), read_sized_word(11, stack + 4, 4));
				indent(); printf("                                               // data stack now %08x (%d words deep)\n", stack + 8, (0xffefebe4 - (stack + 8)) / 4);
				break;

			case 0x02b6:
				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; duplicate top of data stack (%08x) and push the result (%08x) in between\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), read_sized_word(11, stack, 4), REG(4));
				indent(); printf("                                               // data stack now %08x (%d words deep)\n", stack - 8, (0xffefebe4 - (REG(7) - 8)) / 4);
				break;

			case 0x02e2:
				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; load result with value on data stack (%08x) indexed by result (%08x)\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), read_sized_word(11, stack + (REG(4) << 2), 4), REG(4));
				break;

			case 0x02f2:
				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; decrement result (%08x = %08x - 1)\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), REG(4) - 1, REG(4));
				break;

			case 0x0306:
				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; shift result left by 1 bit (%08x = %08x << 1)\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), REG(4) << 1, REG(4));
				break;

			case 0x031e:
				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; store 0 at address contained by result (%08x), pop word from data stack into result (%08x)\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), REG(4), read_sized_word(11, stack, 4));
				indent(); printf("                                               // data stack now %08x (%d words deep)\n", stack + 4, (0xffefebe4 - (REG(7) + 4)) / 4);
				break;

			case 0x0334:
			{
				UINT32 address = REG(4);
				UINT32 half0 = read_sized_word(11, address, 2);
				if (address & 2) half0 <<= 16;

				address = REG(4) + 2;
				UINT32 half1 = read_sized_word(11, address, 2);
				if (!(address & 2)) half1 >>= 16;

				UINT32 value = half0 | half1;

				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; load result with word (%08x) at result address (%08x)\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), value, REG(4));
				break;
			}

			case 0x0349:
			{
				UINT32 address = REG(4);
				UINT32 value = read_sized_word(11, address, 2);
				if (!(address & 2)) value >>= 16;
				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; load result with unsigned halfword (%04x) at result address (%08x)\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), value, REG(4));
				break;
			}

			case 0x0353:
			{
				UINT32 address = REG(4);
				UINT32 value = read_sized_word(11, address, 1);
				value >>= (3 - (address & 3)) * 8;
				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; load result with unsigned byte (%02x) at result address (%08x)\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), value, REG(4));
				break;
			}

			case 0x0381:
			case 0x0382:
				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; pop_data (%08x) into address contained by result (%08x), pop_data (%08x) into result\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), read_sized_word(11, stack, 4), REG(4), read_sized_word(11, stack + 4, 4));
				indent(); printf("                                               // data stack now %08x (%d words deep)\n", stack + 8, (0xffefebe4 - (stack + 8)) / 4);
				break;

			case 0x045a:
				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; pop word from data stack and add it to result << 2, (%08x = %08x + (%08x << 2))\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), (REG(4) << 2) + read_sized_word(11, stack, 4), read_sized_word(11, stack, 4), REG(4));
				indent(); printf("                                               // data stack now %08x (%d words deep)\n", stack - 4, (0xffefebe4 - (stack - 4)) / 4);
				break;

			case 0x0462:
				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; pop word from data stack and add it to result << 1, (%08x = %08x + (%08x << 1))\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), (REG(4) << 1) + read_sized_word(11, stack, 4), read_sized_word(11, stack, 4), REG(4));
				indent(); printf("                                               // data stack now %08x (%d words deep)\n", stack - 4, (0xffefebe4 - (stack - 4)) / 4);
				break;

			case 0x046a:
				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; increment result (%08x = %08x + 1\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), REG(4) + 1, REG(4));
				break;

			case 0x047e:
				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; add 2 to result (%08x = %08x + 2\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), REG(4) + 2, REG(4));
				break;

			case 0x04ff:
			{
				UINT32 address = r5 + 2;
				UINT32 next_op = read_sized_word(11, address, 2);
				if (!(address & 2)) next_op >>= 16;

				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; push_data result (%08x), load result with entry_point for next opcode (%08x = %08x + %08x, op %04x)\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), REG(4), REG(2) + (next_op << 2), REG(2), next_op << 2, next_op);
				indent(); printf("                                               // data stack now %08x (%d words deep)\n", stack - 4, (0xffefebe4 - (stack - 4)) / 4);
				break;
			}

			case 0x050a:
			{
				UINT32 byte_addr = read_sized_word(11, REG(6), 4);
				UINT32 address_shift = (3 - (byte_addr & 3)) * 8;
				UINT8 value = read_sized_word(11, byte_addr, 1) >> address_shift;
				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; push result (%08x) onto data stack\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), REG(4));
				indent(); printf("                                               // load result with byte pointed to by top of program stack (%02x)\n", value);
				indent(); printf("                                               // push address of next byte (%08x) onto data stack\n", byte_addr + 1);
				indent(); printf("                                               // add (result + 3) to program_top and clear the low bit (now %08x)\n", (byte_addr + value + 3) & ~1);
				indent(); printf("                                               // data stack now %08x (%d words deep)\n", stack - 8, (0xffefebe4 - (stack - 8)) / 4);
				break;
			}

			case 0x05ca:
			{
				//
				UINT32 address = REG(4);
				UINT32 g2_offset = read_sized_word(11, address, 2);
				if (!(address & 2)) g2_offset >>= 16;

				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; add halfword at address in result (%08x) to forth table base pointer (%08x) and store in result (%08x = %08x + %08x)\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), address, REG(2), REG(2) + g2_offset, REG(2), g2_offset);
				break;
			}

			case 0x05e6:
			{
				UINT32 address = REG(4);
				UINT32 next_op = read_sized_word(11, address, 2);
				if (!(address & 2)) next_op >>= 16;

				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; load result with entry_point for opcode (%08x = %08x + %08x, op %04x) pointed to by result (%08x)\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), REG(2) + (next_op << 2), REG(2), next_op << 2, next_op, REG(4));
				break;
			}

			case 0x05ee:
			case 0x05ef:
			{
				UINT32 stack_top = read_sized_word(11, stack, 4);
				UINT32 new_opcode = (stack_top - REG(2)) >> 2;
				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; pop word (%08x) from the data stack, turn it into a 16-bit opcode (%04x)\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), stack_top, new_opcode);
				indent(); printf("                                               // store the opcode at the address contained in the result (%08x)\n", REG(4));
				indent(); printf("                                               // pop word from the data stack into to the result (%08x)\n", read_sized_word(11, stack + 4, 4));
				indent(); printf("                                               // data stack now %08x (%d words deep)\n", stack + 8, (0xffefebe4 - (stack + 8)) / 4);
				break;
			}

			case 0x05fe:
				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; push result (%08x) onto data stack, load base pointer of Fcode table (%08x) into result\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), REG(4), REG(2));
				indent(); printf("                                               // data stack now %08x (%d words deep)\n", stack - 4, (0xffefebe4 - (stack - 4)) / 4);
				break;

			case 0x0613:
				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; if result (%08x) is equal to g2 (%08x), push result to data stack and load with 0, else push nothing and load with ~0\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), REG(4), REG(2));
				if (REG(4) == REG(2))
				{
					indent(); printf("                                               // data stack now %08x (%d words deep)\n", stack - 4, (0xffefebe4 - (stack - 4)) / 4);
				}
				break;

			case 0x0668:
			{
				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; some complex junk I don't understand, the amount of asm in this function is a PITA.\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str());
				indent(); printf("                                               // basically it involves a bunch of comparisons between whatever's on the top of the stack, the\n");
				indent(); printf("                                               // current result, and some other weird junk like 0x20000000\n");
				indent(); printf("                                               // like seriously, this function is a couple hundred lines of asm long\n");
				break;
			}

			case 0x3d38:
				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; call ffe8fe90\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str());
				indent(); printf("                                               // program stack now %08x (%d words deep)\n", REG(6) - 4, (0xffeff000 - (REG(6) - 4)) / 4);
				indent(); printf("                                               // data stack now %08x (%d words deep)\n", stack - 4, (0xffefebe4 - (stack - 4)) / 4);
				break;

			case 0x3fd2:
				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; pop address from data stack (%08x), pop data from data stack (%08x)\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), read_sized_word(11, stack, 4), read_sized_word(11, stack + 4, 4));
				indent(); printf("                                               // address at ASI specified by result (%d), then pop word from data stack (%08x) into result\n", REG(4), read_sized_word(11, stack + 8, 4));
				indent(); printf("                                               // data stack now %08x (%d words deep)\n", stack + 12, (0xffefebe4 - (stack + 12)) / 4);
				break;

			case 0x4181:
			{
				UINT32 address = handler_base + 2;
				UINT32 g3_offset = read_sized_word(11, address, 2);
				if (!(address & 2)) g3_offset >>= 16;

				address = REG(3) + g3_offset;
				UINT32 new_result = read_sized_word(11, address, 2);
				if (!(address & 2)) new_result >>= 16;

				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s ; push result (%08x) to data stack, load halfword (%04x) from handler table\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str(), REG(4), g3_offset);
				indent(); printf("                                               // use g3 (%08x) plus first halfword as the address (%08x) from which to load a halfword (%04x) into result\n", REG(3), address, new_result);
				indent(); printf("                                               // data stack now %08x (%d words deep)\n", stack - 4, (0xffefebe4 - (stack - 4)) / 4);
				break;
			}

			default:
				indent(); printf("Opcode at %08x (%04x + %x): %04x, entry is at %08x // %s: unknown\n", r5, base_op, dist, opcode, entry_point, opdesc.c_str());
				break;
		}
	}
}
