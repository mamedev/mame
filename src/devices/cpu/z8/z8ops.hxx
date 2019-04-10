// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Zilog Z8 Single-Chip MCU emulation

**********************************************************************/

/***************************************************************************
    MACROS
***************************************************************************/

#define read(_reg)      register_read(_reg)
#define r(_data)        get_working_register(_data)
#define Ir(_data)       get_intermediate_register(get_working_register(_data))
#define R               get_register(fetch())
#define IR              get_intermediate_register(get_register(fetch()))
#define RR              get_intermediate_register(get_register(fetch()))
#define IM              fetch()
#define flag(_flag)     ((m_flags & Z8_FLAGS_##_flag) ? 1 : 0)

#define mode_r1_r2(_func)   \
	uint8_t dst_src = fetch();\
	uint8_t dst = r(dst_src >> 4);\
	uint8_t src = read(r(dst_src & 0x0f));\
	_func(dst, src);

#define mode_r1_Ir2(_func) \
	uint8_t dst_src = fetch();\
	uint8_t dst = r(dst_src >> 4);\
	uint8_t src = read(Ir(dst_src & 0x0f));\
	_func(dst, src);

#define mode_R2_R1(_func) \
	uint8_t src = read(R);\
	uint8_t dst = R;\
	_func(dst, src);

#define mode_IR2_R1(_func) \
	uint8_t src = read(read(R));\
	uint8_t dst = R;\
	_func(dst, src);

#define mode_R1_IM(_func) \
	uint8_t dst = R;\
	uint8_t src = IM;\
	_func(dst, src);

#define mode_IR1_IM(_func) \
	uint8_t dst = IR;\
	uint8_t src = IM;\
	_func(dst, src);

#define mode_r1(_func) \
	uint8_t dst = r(opcode >> 4);\
	_func(dst);

#define mode_R1(_func) \
	uint8_t dst = R;\
	_func(dst);

#define mode_RR1(_func) \
	uint8_t dst = R;\
	_func(dst);

#define mode_IR1(_func) \
	uint8_t dst = IR;\
	_func(dst);

#define mode_r1_IM(_func) \
	uint8_t dst = r(opcode >> 4);\
	uint8_t src = IM;\
	_func(dst, src);

#define mode_r1_R2(_func) \
	uint8_t dst = r(opcode >> 4);\
	uint8_t src = read(R);\
	_func(dst, src);

#define mode_r2_R1(_func) \
	uint8_t src = read(r(opcode >> 4));\
	uint8_t dst = R;\
	_func(dst, src);

#define mode_Ir1_r2(_func) \
	uint8_t dst_src = fetch();\
	uint8_t dst = Ir(dst_src >> 4);\
	uint8_t src = read(r(dst_src & 0x0f));\
	_func(dst, src);

#define mode_R2_IR1(_func) \
	uint8_t src = read(R);\
	uint8_t dst = IR;\
	_func(dst, src);

#define mode_r1_x_R2(_func) \
	uint8_t dst_src = fetch();\
	uint8_t dst = r(dst_src >> 4);\
	uint8_t src = read(read(r(dst_src & 0x0f)) + R);\
	_func(dst, src);

#define mode_r2_x_R1(_func) \
	uint8_t dst_src = fetch();\
	uint8_t dst = R + read(r(dst_src & 0x0f));\
	uint8_t src = read(r(dst_src >> 4));\
	_func(dst, src);

/***************************************************************************
    LOAD INSTRUCTIONS
***************************************************************************/

void z8_device::clear(uint8_t dst)
{
	/* dst <- 0 */
	register_write(dst, 0);
}

INSTRUCTION( clr_R1 )           { mode_R1(clear) }
INSTRUCTION( clr_IR1 )          { mode_IR1(clear) }

void z8_device::load(uint8_t dst, uint8_t src)
{
	/* dst <- src */
	register_write(dst, src);
}

INSTRUCTION( ld_r1_IM )         { mode_r1_IM(load) }
INSTRUCTION( ld_r1_R2 )         { mode_r1_R2(load) }
INSTRUCTION( ld_r2_R1 )         { mode_r2_R1(load) }
INSTRUCTION( ld_Ir1_r2 )        { mode_Ir1_r2(load) }
INSTRUCTION( ld_R2_IR1 )        { mode_R2_IR1(load) }
INSTRUCTION( ld_r1_x_R2 )       { mode_r1_x_R2(load) }
INSTRUCTION( ld_r2_x_R1 )       { mode_r2_x_R1(load) }

INSTRUCTION( ld_r1_r2 )         { mode_r1_r2(load) }
INSTRUCTION( ld_r1_Ir2 )        { mode_r1_Ir2(load) }
INSTRUCTION( ld_R2_R1 )         { mode_R2_R1(load) }
INSTRUCTION( ld_IR2_R1 )        { mode_IR2_R1(load) }
INSTRUCTION( ld_R1_IM )         { mode_R1_IM(load) }
INSTRUCTION( ld_IR1_IM )        { mode_IR1_IM(load) }

void z8_device::load_from_memory(address_space &space)
{
	uint8_t operands = fetch();
	uint8_t dst = get_working_register(operands >> 4);
	uint8_t src = get_working_register(operands & 0x0f);

	uint16_t address = register_pair_read(src);

	uint8_t data;
	if (&space == m_program && address < m_rom_size)
		data = m_cache->read_byte(address);
	else
		data = space.read_byte(mask_external_address(address));

	register_write(dst, data);
}

void z8_device::load_to_memory(address_space &space)
{
	uint8_t operands = fetch();
	uint8_t src = get_working_register(operands >> 4);
	uint8_t dst = get_working_register(operands & 0x0f);

	uint16_t address = register_pair_read(dst);
	if (&space != m_program || address >= m_rom_size)
		address = mask_external_address(address);

	uint8_t data = register_read(src);
	space.write_byte(address, data);
}

void z8_device::load_from_memory_autoinc(address_space &space)
{
	uint8_t operands = fetch();
	uint8_t dst = get_working_register(operands >> 4);
	uint8_t real_dst = get_intermediate_register(dst);
	uint8_t src = get_working_register(operands & 0x0f);

	uint16_t address = register_pair_read(src);

	uint8_t data;
	if (&space == m_program && address < m_rom_size)
		data = m_cache->read_byte(address);
	else
		data = space.read_byte(mask_external_address(address));
	register_write(real_dst, data);

	register_write(dst, real_dst + 1);
	register_pair_write(src, address + 1);
}

void z8_device::load_to_memory_autoinc(address_space &space)
{
	uint8_t operands = fetch();
	uint8_t src = get_working_register(operands >> 4);
	uint8_t dst = get_working_register(operands & 0x0f);
	uint8_t real_src = get_intermediate_register(src);

	uint16_t address = register_pair_read(dst);
	uint8_t data = register_read(real_src);

	if (&space != m_program || address >= m_rom_size)
		address = mask_external_address(address);
	space.write_byte(address, data);

	register_pair_write(dst, address + 1);
	register_write(src, real_src + 1);
}

INSTRUCTION( ldc_r1_Irr2 )      { load_from_memory(*m_program); }
INSTRUCTION( ldc_r2_Irr1 )      { load_to_memory(*m_program); }
INSTRUCTION( ldci_Ir1_Irr2 )    { load_from_memory_autoinc(*m_program); }
INSTRUCTION( ldci_Ir2_Irr1 )    { load_to_memory_autoinc(*m_program); }
INSTRUCTION( lde_r1_Irr2 )      { load_from_memory(*m_data); }
INSTRUCTION( lde_r2_Irr1 )      { load_to_memory(*m_data); }
INSTRUCTION( ldei_Ir1_Irr2 )    { load_from_memory_autoinc(*m_data); }
INSTRUCTION( ldei_Ir2_Irr1 )    { load_to_memory_autoinc(*m_data); }

void z8_device::pop(uint8_t dst)
{
	/* dst <- @SP
	   SP <- SP + 1 */
	register_write(dst, stack_pop_byte());
}

INSTRUCTION( pop_R1 )           { mode_R1(pop) }
INSTRUCTION( pop_IR1 )          { mode_IR1(pop) }

void z8_device::push(uint8_t src)
{
	/* SP <- SP - 1
	   @SP <- src */
	stack_push_byte(read(src));
}

INSTRUCTION( push_R2 )          { mode_R1(push) }
INSTRUCTION( push_IR2 )         { mode_IR1(push) }

/***************************************************************************
    ARITHMETIC INSTRUCTIONS
***************************************************************************/

void z8_device::add_carry(uint8_t dst, uint8_t src)
{
	/* dst <- dst + src + C */
	uint8_t data = register_read(dst);
	uint16_t new_data = data + src + flag(C);

	set_flag_c(new_data & 0x100);
	new_data &= 0xff;
	set_flag_z(new_data == 0);
	set_flag_s(new_data & 0x80);
	set_flag_v(((data & 0x80) == (src & 0x80)) && ((new_data & 0x80) != (src & 0x80)));
	set_flag_d(0);
	set_flag_h(((data & 0x1f) == 0x0f) && ((new_data & 0x1f) == 0x10));

	register_write(dst, new_data);
}

INSTRUCTION( adc_r1_r2 )        { mode_r1_r2(add_carry) }
INSTRUCTION( adc_r1_Ir2 )       { mode_r1_Ir2(add_carry) }
INSTRUCTION( adc_R2_R1 )        { mode_R2_R1(add_carry) }
INSTRUCTION( adc_IR2_R1 )       { mode_IR2_R1(add_carry) }
INSTRUCTION( adc_R1_IM )        { mode_R1_IM(add_carry) }
INSTRUCTION( adc_IR1_IM )       { mode_IR1_IM(add_carry) }

void z8_device::add(uint8_t dst, uint8_t src)
{
	/* dst <- dst + src */
	uint8_t data = register_read(dst);
	uint16_t new_data = data + src;

	set_flag_c(new_data & 0x100);
	new_data &= 0xff;
	set_flag_z(new_data == 0);
	set_flag_s(new_data & 0x80);
	set_flag_v(((data & 0x80) == (src & 0x80)) && ((new_data & 0x80) != (src & 0x80)));
	set_flag_d(0);
	set_flag_h(((data & 0x1f) == 0x0f) && ((new_data & 0x1f) == 0x10));

	register_write(dst, new_data);
}

INSTRUCTION( add_r1_r2 )        { mode_r1_r2(add) }
INSTRUCTION( add_r1_Ir2 )       { mode_r1_Ir2(add) }
INSTRUCTION( add_R2_R1 )        { mode_R2_R1(add) }
INSTRUCTION( add_IR2_R1 )       { mode_IR2_R1(add) }
INSTRUCTION( add_R1_IM )        { mode_R1_IM(add) }
INSTRUCTION( add_IR1_IM )       { mode_IR1_IM(add) }

void z8_device::compare(uint8_t dst, uint8_t src)
{
	/* dst - src */
	uint8_t data = register_read(dst);
	uint16_t new_data = data - src;

	set_flag_c(new_data & 0x100);
	new_data &= 0xff;
	set_flag_z(new_data == 0);
	set_flag_s(new_data & 0x80);
	set_flag_v(((data & 0x80) != (src & 0x80)) && ((new_data & 0x80) == (src & 0x80)));
}

INSTRUCTION( cp_r1_r2 )         { mode_r1_r2(compare) }
INSTRUCTION( cp_r1_Ir2 )        { mode_r1_Ir2(compare) }
INSTRUCTION( cp_R2_R1 )         { mode_R2_R1(compare) }
INSTRUCTION( cp_IR2_R1 )        { mode_IR2_R1(compare) }
INSTRUCTION( cp_R1_IM )         { mode_R1_IM(compare) }
INSTRUCTION( cp_IR1_IM )        { mode_IR1_IM(compare) }

void z8_device::decimal_adjust(uint8_t dst)
{
	uint8_t data = register_read(dst);
	uint16_t new_data = data;
	if (flag(D))
	{
		if (flag(H) | ((data&0xf)>9)) new_data-=6;
		if (flag(C) | (data>0x99)) new_data-=0x60;
	}
	else
	{
		if (flag(H) | ((data&0xf)>9)) new_data+=6;
		if (flag(C) | (data>0x99)) new_data+=0x60;
	}

	set_flag_c(new_data & 0x100);
	set_flag_s(new_data & 0x80);
	new_data &= 0xff;
	set_flag_z(new_data == 0);
	// officially, v is undefined
	register_write(dst, new_data);
}

INSTRUCTION( da_R1 )            { mode_R1(decimal_adjust) }
INSTRUCTION( da_IR1 )           { mode_IR1(decimal_adjust) }

void z8_device::decrement(uint8_t dst)
{
	/* dst <- dst - 1 */
	uint8_t data = register_read(dst) - 1;

	set_flag_z(data == 0);
	set_flag_s(data & 0x80);
	set_flag_v(data == 0x7f);

	register_write(dst, data);
}

INSTRUCTION( dec_R1 )           { mode_R1(decrement) }
INSTRUCTION( dec_IR1 )          { mode_IR1(decrement) }

void z8_device::decrement_word(uint8_t dst)
{
	/* dst <- dst - 1 */
	uint16_t data = register_pair_read(dst) - 1;

	set_flag_z(data == 0);
	set_flag_s(data & 0x8000);
	set_flag_v(data == 0x7fff);

	register_pair_write(dst, data);
}

INSTRUCTION( decw_RR1 )         { mode_RR1(decrement_word) }
INSTRUCTION( decw_IR1 )         { mode_IR1(decrement_word) }

void z8_device::increment(uint8_t dst)
{
	/* dst <- dst + 1 */
	uint8_t data = register_read(dst) + 1;

	set_flag_z(data == 0);
	set_flag_s(data & 0x80);
	set_flag_v(data == 0x80);

	register_write(dst, data);
}

INSTRUCTION( inc_r1 )           { mode_r1(increment) }
INSTRUCTION( inc_R1 )           { mode_R1(increment) }
INSTRUCTION( inc_IR1 )          { mode_IR1(increment) }

void z8_device::increment_word(uint8_t dst)
{
	/* dst <- dst + 1 */
	uint16_t data = register_pair_read(dst) + 1;

	set_flag_z(data == 0);
	set_flag_s(data & 0x8000);
	set_flag_v(data == 0x8000);

	register_pair_write(dst, data);
}

INSTRUCTION( incw_RR1 )         { mode_RR1(increment_word) }
INSTRUCTION( incw_IR1 )         { mode_IR1(increment_word) }

void z8_device::subtract_carry(uint8_t dst, uint8_t src)
{
	/* dst <- dst - src - C */
	uint8_t data = register_read(dst);
	uint16_t new_data = data - src - flag(C);

	set_flag_c(new_data & 0x100);
	new_data &= 0xff;
	set_flag_z(new_data == 0);
	set_flag_s(new_data & 0x80);
	set_flag_v(((data & 0x80) != (src & 0x80)) && ((new_data & 0x80) == (src & 0x80)));
	set_flag_d(1);
	set_flag_h(!(((data & 0x1f) == 0x0f) && ((new_data & 0x1f) == 0x10)));

	register_write(dst, new_data);
}

INSTRUCTION( sbc_r1_r2 )        { mode_r1_r2(subtract_carry) }
INSTRUCTION( sbc_r1_Ir2 )       { mode_r1_Ir2(subtract_carry) }
INSTRUCTION( sbc_R2_R1 )        { mode_R2_R1(subtract_carry) }
INSTRUCTION( sbc_IR2_R1 )       { mode_IR2_R1(subtract_carry) }
INSTRUCTION( sbc_R1_IM )        { mode_R1_IM(subtract_carry) }
INSTRUCTION( sbc_IR1_IM )       { mode_IR1_IM(subtract_carry) }

void z8_device::subtract(uint8_t dst, uint8_t src)
{
	/* dst <- dst - src */
	uint8_t data = register_read(dst);
	uint16_t new_data = data - src;

	set_flag_c(new_data & 0x100);
	new_data &= 0xff;
	set_flag_z(new_data == 0);
	set_flag_s(new_data & 0x80);
	set_flag_v(((data & 0x80) != (src & 0x80)) && ((new_data & 0x80) == (src & 0x80)));
	set_flag_d(1);
	set_flag_h(!(((data & 0x1f) == 0x0f) && ((new_data & 0x1f) == 0x10)));

	register_write(dst, new_data);
}

INSTRUCTION( sub_r1_r2 )        { mode_r1_r2(subtract) }
INSTRUCTION( sub_r1_Ir2 )       { mode_r1_Ir2(subtract) }
INSTRUCTION( sub_R2_R1 )        { mode_R2_R1(subtract) }
INSTRUCTION( sub_IR2_R1 )       { mode_IR2_R1(subtract) }
INSTRUCTION( sub_R1_IM )        { mode_R1_IM(subtract) }
INSTRUCTION( sub_IR1_IM )       { mode_IR1_IM(subtract) }

/***************************************************************************
    LOGICAL INSTRUCTIONS
***************************************************************************/

void z8_device::_and(uint8_t dst, uint8_t src)
{
	/* dst <- dst AND src */
	uint8_t data = register_read(dst) & src;
	register_write(dst, data);

	set_flag_z(data == 0);
	set_flag_s(data & 0x80);
	set_flag_v(0);
}

INSTRUCTION( and_r1_r2 )        { mode_r1_r2(_and) }
INSTRUCTION( and_r1_Ir2 )       { mode_r1_Ir2(_and) }
INSTRUCTION( and_R2_R1 )        { mode_R2_R1(_and) }
INSTRUCTION( and_IR2_R1 )       { mode_IR2_R1(_and) }
INSTRUCTION( and_R1_IM )        { mode_R1_IM(_and) }
INSTRUCTION( and_IR1_IM )       { mode_IR1_IM(_and) }

void z8_device::complement(uint8_t dst)
{
	/* dst <- NOT dst */
	uint8_t data = register_read(dst) ^ 0xff;
	register_write(dst, data);

	set_flag_z(data == 0);
	set_flag_s(data & 0x80);
	set_flag_v(0);
}

INSTRUCTION( com_R1 )           { mode_R1(complement) }
INSTRUCTION( com_IR1 )          { mode_IR1(complement) }

void z8_device::_or(uint8_t dst, uint8_t src)
{
	/* dst <- dst OR src */
	uint8_t data = register_read(dst) | src;
	register_write(dst, data);

	set_flag_z(data == 0);
	set_flag_s(data & 0x80);
	set_flag_v(0);
}

INSTRUCTION( or_r1_r2 )         { mode_r1_r2(_or) }
INSTRUCTION( or_r1_Ir2 )        { mode_r1_Ir2(_or) }
INSTRUCTION( or_R2_R1 )         { mode_R2_R1(_or) }
INSTRUCTION( or_IR2_R1 )        { mode_IR2_R1(_or) }
INSTRUCTION( or_R1_IM )         { mode_R1_IM(_or) }
INSTRUCTION( or_IR1_IM )        { mode_IR1_IM(_or) }

void z8_device::_xor(uint8_t dst, uint8_t src)
{
	/* dst <- dst XOR src */
	uint8_t data = register_read(dst) ^ src;
	register_write(dst, data);

	set_flag_z(data == 0);
	set_flag_s(data & 0x80);
	set_flag_v(0);
}

INSTRUCTION( xor_r1_r2 )        { mode_r1_r2(_xor) }
INSTRUCTION( xor_r1_Ir2 )       { mode_r1_Ir2(_xor) }
INSTRUCTION( xor_R2_R1 )        { mode_R2_R1(_xor) }
INSTRUCTION( xor_IR2_R1 )       { mode_IR2_R1(_xor) }
INSTRUCTION( xor_R1_IM )        { mode_R1_IM(_xor) }
INSTRUCTION( xor_IR1_IM )       { mode_IR1_IM(_xor) }

/***************************************************************************
    PROGRAM CONTROL INSTRUCTIONS
***************************************************************************/

void z8_device::call(uint16_t dst)
{
	stack_push_word(m_pc);
	m_pc = dst;
}

INSTRUCTION( call_IRR1 )        { uint16_t dst = register_pair_read(get_register(fetch())); call(dst); }
INSTRUCTION( call_DA )          { uint16_t dst = fetch_word(); call(dst); }

INSTRUCTION( djnz_r1_RA )
{
	int8_t ra = (int8_t)fetch();

	/* r <- r - 1 */
	int r = get_working_register(opcode >> 4);
	uint8_t data = register_read(r) - 1;
	register_write(r, data);

	/* if r<>0, PC <- PC + dst */
	if (data != 0)
	{
		m_pc += ra;
		*cycles += 2;
	}
}

INSTRUCTION( iret )
{
	/* FLAGS <- @SP
	   SP <- SP + 1 */
	flags_write(stack_pop_byte());

	/* PC <- @SP
	   SP <- SP + 2 */
	m_pc = stack_pop_word();

	/* IMR (7) <- 1 */
	m_imr |= Z8_IMR_ENABLE;
}

INSTRUCTION( ret )
{
	/* PC <- @SP
	   SP <- SP + 2 */
	m_pc = stack_pop_word();
}

void z8_device::jump(uint16_t dst)
{
	/* PC <- dst */
	m_pc = dst;
}

INSTRUCTION( jp_IRR1 )          { jump(register_pair_read(get_register(IM))); }

bool z8_device::check_condition_code(int cc)
{
	bool truth = 0;

	switch (cc)
	{
	case CC_F:      truth = 0; break;
	case CC_LT:     truth = flag(S) ^ flag(V); break;
	case CC_LE:     truth = (flag(Z) | (flag(S) ^ flag(V))); break;
	case CC_ULE:    truth = flag(C) | flag(Z); break;
	case CC_OV:     truth = flag(V); break;
	case CC_MI:     truth = flag(S); break;
	case CC_Z:      truth = flag(Z); break;
	case CC_C:      truth = flag(C); break;
	case CC_T:      truth = 1; break;
	case CC_GE:     truth = !(flag(S) ^ flag(V)); break;
	case CC_GT:     truth = !(flag(Z) | (flag(S) ^ flag(V))); break;
	case CC_UGT:    truth = ((!flag(C)) & (!flag(Z))); break;
	case CC_NOV:    truth = !flag(V); break;
	case CC_PL:     truth = !flag(S); break;
	case CC_NZ:     truth = !flag(Z); break;
	case CC_NC:     truth = !flag(C); break;
	}

	return truth;
}

INSTRUCTION( jp_cc_DA )
{
	uint16_t dst = fetch_word();

	/* if cc is true, then PC <- dst */
	if (check_condition_code(opcode >> 4))
	{
		jump(dst);
		*cycles += 2;
	}
}

INSTRUCTION( jr_cc_RA )
{
	int8_t ra = (int8_t)fetch();
	uint16_t dst = m_pc + ra;

	/* if cc is true, then PC <- dst */
	if (check_condition_code(opcode >> 4))
	{
		jump(dst);
		*cycles += 2;
	}
}

/***************************************************************************
    BIT MANIPULATION INSTRUCTIONS
***************************************************************************/

void z8_device::test_complement_under_mask(uint8_t dst, uint8_t src)
{
	/* NOT(dst) AND src */
	uint8_t data = (register_read(dst) ^ 0xff) & src;

	set_flag_z(data == 0);
	set_flag_s(data & 0x80);
	set_flag_v(0);
}

INSTRUCTION( tcm_r1_r2 )        { mode_r1_r2(test_complement_under_mask) }
INSTRUCTION( tcm_r1_Ir2 )       { mode_r1_Ir2(test_complement_under_mask) }
INSTRUCTION( tcm_R2_R1 )        { mode_R2_R1(test_complement_under_mask) }
INSTRUCTION( tcm_IR2_R1 )       { mode_IR2_R1(test_complement_under_mask) }
INSTRUCTION( tcm_R1_IM )        { mode_R1_IM(test_complement_under_mask) }
INSTRUCTION( tcm_IR1_IM )       { mode_IR1_IM(test_complement_under_mask) }

void z8_device::test_under_mask(uint8_t dst, uint8_t src)
{
	/* dst AND src */
	uint8_t data = register_read(dst) & src;

	set_flag_z(data == 0);
	set_flag_s(data & 0x80);
	set_flag_v(0);
}

INSTRUCTION( tm_r1_r2 )         { mode_r1_r2(test_under_mask) }
INSTRUCTION( tm_r1_Ir2 )        { mode_r1_Ir2(test_under_mask) }
INSTRUCTION( tm_R2_R1 )         { mode_R2_R1(test_under_mask) }
INSTRUCTION( tm_IR2_R1 )        { mode_IR2_R1(test_under_mask) }
INSTRUCTION( tm_R1_IM )         { mode_R1_IM(test_under_mask) }
INSTRUCTION( tm_IR1_IM )        { mode_IR1_IM(test_under_mask) }

/***************************************************************************
    ROTATE AND SHIFT INSTRUCTIONS
***************************************************************************/

void z8_device::rotate_left(uint8_t dst)
{
	/* << */
	uint8_t data = register_read(dst);
	uint8_t new_data = (data << 1) | BIT(data, 7);

	set_flag_c(data & 0x80);
	set_flag_z(new_data == 0);
	set_flag_s(new_data & 0x80);
	set_flag_v((data & 0x80) != (new_data & 0x80));

	register_write(dst, new_data);
}

INSTRUCTION( rl_R1 )            { mode_R1(rotate_left) }
INSTRUCTION( rl_IR1 )           { mode_IR1(rotate_left) }

void z8_device::rotate_left_carry(uint8_t dst)
{
	/* << C */
	uint8_t data = register_read(dst);
	uint8_t new_data = (data << 1) | flag(C);

	set_flag_c(data & 0x80);
	set_flag_z(new_data == 0);
	set_flag_s(new_data & 0x80);
	set_flag_v((data & 0x80) != (new_data & 0x80));

	register_write(dst, new_data);
}

INSTRUCTION( rlc_R1 )           { mode_R1(rotate_left_carry) }
INSTRUCTION( rlc_IR1 )          { mode_IR1(rotate_left_carry) }

void z8_device::rotate_right(uint8_t dst)
{
	/* >> */
	uint8_t data = register_read(dst);
	uint8_t new_data = ((data & 0x01) << 7) | (data >> 1);

	set_flag_c(data & 0x01);
	set_flag_z(new_data == 0);
	set_flag_s(new_data & 0x80);
	set_flag_v((data & 0x80) != (new_data & 0x80));

	register_write(dst, new_data);
}

INSTRUCTION( rr_R1 )            { mode_R1(rotate_right) }
INSTRUCTION( rr_IR1 )           { mode_IR1(rotate_right) }

void z8_device::rotate_right_carry(uint8_t dst)
{
	/* >> C */
	uint8_t data = register_read(dst);
	uint8_t new_data = (flag(C) << 7) | (data >> 1);

	set_flag_c(data & 0x01);
	set_flag_z(new_data == 0);
	set_flag_s(new_data & 0x80);
	set_flag_v((data & 0x80) != (new_data & 0x80));

	register_write(dst, new_data);
}

INSTRUCTION( rrc_R1 )           { mode_R1(rotate_right_carry) }
INSTRUCTION( rrc_IR1 )          { mode_IR1(rotate_right_carry) }

void z8_device::shift_right_arithmetic(uint8_t dst)
{
	/* */
	uint8_t data = register_read(dst);
	uint8_t new_data = (data & 0x80) | ((data >> 1) & 0x7f);

	set_flag_c(data & 0x01);
	set_flag_z(new_data == 0);
	set_flag_s(new_data & 0x80);
	set_flag_v(0);

	register_write(dst, new_data);
}

INSTRUCTION( sra_R1 )           { mode_R1(shift_right_arithmetic) }
INSTRUCTION( sra_IR1 )          { mode_IR1(shift_right_arithmetic) }

void z8_device::swap(uint8_t dst)
{
	/* dst(7-4) <-> dst(3-0) */
	uint8_t data = register_read(dst);
	data = (data << 4) | (data >> 4);
	register_write(dst, data);

	set_flag_z(data == 0);
	set_flag_s(data & 0x80);
//  set_flag_v(0); undefined
}

INSTRUCTION( swap_R1 )          { mode_R1(swap) }
INSTRUCTION( swap_IR1 )         { mode_IR1(swap) }

/***************************************************************************
    CPU CONTROL INSTRUCTIONS
***************************************************************************/

INSTRUCTION( ccf )              { m_flags ^= Z8_FLAGS_C; }
INSTRUCTION( di )               { m_imr &= ~Z8_IMR_ENABLE; }
INSTRUCTION( ei )               { m_imr |= Z8_IMR_ENABLE; m_irq_initialized = true; }
INSTRUCTION( nop )              { /* no operation */ }
INSTRUCTION( rcf )              { set_flag_c(0); }
INSTRUCTION( scf )              { set_flag_c(1); }
INSTRUCTION( srp_IM )           { rp_write(fetch()); }
