// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
void hyperstone_device::op00()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR00(decode);
	hyperstone_chk(decode);
}

void hyperstone_device::op01()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR01(decode);
	hyperstone_chk(decode);
}

void hyperstone_device::op02()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR10(decode);
	hyperstone_chk(decode);
}

void hyperstone_device::op03()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR11(decode);
	hyperstone_chk(decode);
}

void hyperstone_device::op04()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR00(decode);
	hyperstone_movd(decode);
}

void hyperstone_device::op05()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR01(decode);
	hyperstone_movd(decode);
}

void hyperstone_device::op06()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR10(decode);
	hyperstone_movd(decode);
}

void hyperstone_device::op07()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR11(decode);
	hyperstone_movd(decode);
}

void hyperstone_device::op08()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR00(decode);
	hyperstone_divu(decode);
}

void hyperstone_device::op09()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR01(decode);
	hyperstone_divu(decode);
}

void hyperstone_device::op0a()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR10(decode);
	hyperstone_divu(decode);
}

void hyperstone_device::op0b()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR11(decode);
	hyperstone_divu(decode);
}

void hyperstone_device::op0c()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR00(decode);
	hyperstone_divs(decode);
}

void hyperstone_device::op0d()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR01(decode);
	hyperstone_divs(decode);
}

void hyperstone_device::op0e()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR10(decode);
	hyperstone_divs(decode);
}

void hyperstone_device::op0f()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR11(decode);
	hyperstone_divs(decode);
}



void hyperstone_device::op10()
{
	regs_decode decode;
	decode_lim(decode);
	check_delay_PC();
	decode_RR00(decode);
	hyperstone_xm(decode);
}

void hyperstone_device::op11()
{
	regs_decode decode;
	decode_lim(decode);
	check_delay_PC();
	decode_RR01(decode);
	hyperstone_xm(decode);
}

void hyperstone_device::op12()
{
	regs_decode decode;
	decode_lim(decode);
	check_delay_PC();
	decode_RR10(decode);
	hyperstone_xm(decode);
}

void hyperstone_device::op13()
{
	regs_decode decode;
	decode_lim(decode);
	check_delay_PC();
	decode_RR11(decode);
	hyperstone_xm(decode);
}

void hyperstone_device::op14()
{
	regs_decode decode;
	decode_const(decode);
	check_delay_PC();
	decode_RR00(decode);
	hyperstone_mask(decode);
}

void hyperstone_device::op15()
{
	regs_decode decode;
	decode_const(decode);
	check_delay_PC();
	decode_RR01(decode);
	hyperstone_mask(decode);
}

void hyperstone_device::op16()
{
	regs_decode decode;
	decode_const(decode);
	check_delay_PC();
	decode_RR10(decode);
	hyperstone_mask(decode);
}

void hyperstone_device::op17()
{
	regs_decode decode;
	decode_const(decode);
	check_delay_PC();
	decode_RR11(decode);
	hyperstone_mask(decode);
}

void hyperstone_device::op18()
{
	regs_decode decode;
	decode_const(decode);
	check_delay_PC();
	decode_RR00(decode);
	hyperstone_sum(decode);
}

void hyperstone_device::op19()
{
	regs_decode decode;
	decode_const(decode);
	check_delay_PC();
	decode_RR01(decode);
	hyperstone_sum(decode);
}

void hyperstone_device::op1a()
{
	regs_decode decode;
	decode_const(decode);
	check_delay_PC();
	decode_RR10(decode);
	hyperstone_sum(decode);
}

void hyperstone_device::op1b()
{
	regs_decode decode;
	decode_const(decode);
	check_delay_PC();
	decode_RR11(decode);
	hyperstone_sum(decode);
}

void hyperstone_device::op1c()
{
	regs_decode decode;
	decode_const(decode);
	check_delay_PC();
	decode_RR00(decode);
	hyperstone_sums(decode);
}

void hyperstone_device::op1d()
{
	regs_decode decode;
	decode_const(decode);
	check_delay_PC();
	decode_RR01(decode);
	hyperstone_sums(decode);
}

void hyperstone_device::op1e()
{
	regs_decode decode;
	decode_const(decode);
	check_delay_PC();
	decode_RR10(decode);
	hyperstone_sums(decode);
}

void hyperstone_device::op1f()
{
	regs_decode decode;
	decode_const(decode);
	check_delay_PC();
	decode_RR11(decode);
	hyperstone_sums(decode);
}



void hyperstone_device::op20()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR00(decode);
	hyperstone_cmp(decode);
}

void hyperstone_device::op21()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR01(decode);
	hyperstone_cmp(decode);
}

void hyperstone_device::op22()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR10(decode);
	hyperstone_cmp(decode);
}

void hyperstone_device::op23()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR11(decode);
	hyperstone_cmp(decode);
}

void hyperstone_device::op24()
{
	regs_decode decode;
	check_delay_PC();
	decode.src = SRC_CODE;
	decode.dst = DST_CODE;
	decode_source(decode, GET_H);
	decode_dest(decode, GET_H);
	hyperstone_mov(decode);
}

void hyperstone_device::op25()
{
	regs_decode decode;
	check_delay_PC();
	decode.src = SRC_CODE;
	decode.dst = DST_CODE;
	decode_source_local(decode);
	decode_dest(decode, GET_H);
	hyperstone_mov(decode);
}

void hyperstone_device::op26()
{
	regs_decode decode;
	check_delay_PC();
	decode.src = SRC_CODE;
	decode.dst = DST_CODE;
	decode_source(decode, GET_H);
	decode_dest_local(decode);
	hyperstone_mov(decode);
}

void hyperstone_device::op27()
{
	regs_decode decode;
	check_delay_PC();
	decode.src = SRC_CODE;
	decode.dst = DST_CODE;
	decode_source_local(decode);
	decode_dest_local(decode);
	hyperstone_mov(decode);
}

void hyperstone_device::op28()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR00(decode);
	hyperstone_add(decode);
}

void hyperstone_device::op29()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR01(decode);
	hyperstone_add(decode);
}

void hyperstone_device::op2a()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR10(decode);
	hyperstone_add(decode);
}

void hyperstone_device::op2b()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR11(decode);
	hyperstone_add(decode);
}

void hyperstone_device::op2c()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR00(decode);
	hyperstone_adds(decode);
}

void hyperstone_device::op2d()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR01(decode);
	hyperstone_adds(decode);
}

void hyperstone_device::op2e()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR10(decode);
	hyperstone_adds(decode);
}

void hyperstone_device::op2f()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR11(decode);
	hyperstone_adds(decode);
}



void hyperstone_device::op30()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR00(decode);
	hyperstone_cmpb(decode);
}

void hyperstone_device::op31()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR01(decode);
	hyperstone_cmpb(decode);
}

void hyperstone_device::op32()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR10(decode);
	hyperstone_cmpb(decode);
}

void hyperstone_device::op33()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR11(decode);
	hyperstone_cmpb(decode);
}

void hyperstone_device::op34()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR00(decode);
	hyperstone_andn(decode);
}

void hyperstone_device::op35()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR01(decode);
	hyperstone_andn(decode);
}

void hyperstone_device::op36()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR10(decode);
	hyperstone_andn(decode);
}

void hyperstone_device::op37()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR11(decode);
	hyperstone_andn(decode);
}

void hyperstone_device::op38()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR00(decode);
	hyperstone_or(decode);
}

void hyperstone_device::op39()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR01(decode);
	hyperstone_or(decode);
}

void hyperstone_device::op3a()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR10(decode);
	hyperstone_or(decode);
}

void hyperstone_device::op3b()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR11(decode);
	hyperstone_or(decode);
}

void hyperstone_device::op3c()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR00(decode);
	hyperstone_xor(decode);
}

void hyperstone_device::op3d()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR01(decode);
	hyperstone_xor(decode);
}

void hyperstone_device::op3e()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR10(decode);
	hyperstone_xor(decode);
}

void hyperstone_device::op3f()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR11(decode);
	hyperstone_xor(decode);
}



void hyperstone_device::op40()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR00(decode);
	hyperstone_subc(decode);
}

void hyperstone_device::op41()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR01(decode);
	hyperstone_subc(decode);
}

void hyperstone_device::op42()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR10(decode);
	hyperstone_subc(decode);
}

void hyperstone_device::op43()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR11(decode);
	hyperstone_subc(decode);
}

void hyperstone_device::op44()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR00(decode);
	hyperstone_not(decode);
}

void hyperstone_device::op45()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR01(decode);
	hyperstone_not(decode);
}

void hyperstone_device::op46()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR10(decode);
	hyperstone_not(decode);
}

void hyperstone_device::op47()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR11(decode);
	hyperstone_not(decode);
}

void hyperstone_device::op48()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR00(decode);
	hyperstone_sub(decode);
}

void hyperstone_device::op49()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR01(decode);
	hyperstone_sub(decode);
}

void hyperstone_device::op4a()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR10(decode);
	hyperstone_sub(decode);
}

void hyperstone_device::op4b()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR11(decode);
	hyperstone_sub(decode);
}

void hyperstone_device::op4c()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR00(decode);
	hyperstone_subs(decode);
}

void hyperstone_device::op4d()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR01(decode);
	hyperstone_subs(decode);
}

void hyperstone_device::op4e()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR10(decode);
	hyperstone_subs(decode);
}

void hyperstone_device::op4f()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR11(decode);
	hyperstone_subs(decode);
}



void hyperstone_device::op50()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR00(decode);
	hyperstone_addc(decode);
}

void hyperstone_device::op51()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR01(decode);
	hyperstone_addc(decode);
}

void hyperstone_device::op52()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR10(decode);
	hyperstone_addc(decode);
}

void hyperstone_device::op53()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR11(decode);
	hyperstone_addc(decode);
}

void hyperstone_device::op54()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR00(decode);
	hyperstone_and(decode);
}

void hyperstone_device::op55()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR01(decode);
	hyperstone_and(decode);
}

void hyperstone_device::op56()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR10(decode);
	hyperstone_and(decode);
}

void hyperstone_device::op57()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR11(decode);
	hyperstone_and(decode);
}

void hyperstone_device::op58()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR00(decode);
	hyperstone_neg(decode);
}

void hyperstone_device::op59()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR01(decode);
	hyperstone_neg(decode);
}

void hyperstone_device::op5a()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR10(decode);
	hyperstone_neg(decode);
}

void hyperstone_device::op5b()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR11(decode);
	hyperstone_neg(decode);
}

void hyperstone_device::op5c()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR00(decode);
	hyperstone_negs(decode);
}

void hyperstone_device::op5d()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR01(decode);
	hyperstone_negs(decode);
}

void hyperstone_device::op5e()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR10(decode);
	hyperstone_negs(decode);
}

void hyperstone_device::op5f()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR11(decode);
	hyperstone_negs(decode);
}



void hyperstone_device::op60()
{
	regs_decode decode;
	decode_immediate_u(decode);
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest_noh(decode);
	hyperstone_cmpi(decode);
}

void hyperstone_device::op61()
{
	regs_decode decode;
	decode_immediate_s(decode);
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest_noh(decode);
	hyperstone_cmpi(decode);
}

void hyperstone_device::op62()
{
	regs_decode decode;
	decode_immediate_u(decode);
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest_local(decode);
	hyperstone_cmpi(decode);
}

void hyperstone_device::op63()
{
	regs_decode decode;
	decode_immediate_s(decode);
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest_local(decode);
	hyperstone_cmpi(decode);
}

void hyperstone_device::op64()
{
	regs_decode decode;
	decode_immediate_u(decode);
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest(decode, GET_H);
	hyperstone_movi(decode);
}

void hyperstone_device::op65()
{
	regs_decode decode;
	decode_immediate_s(decode);
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest(decode, GET_H);
	hyperstone_movi(decode);
}

void hyperstone_device::op66()
{
	regs_decode decode;
	decode_immediate_u(decode);
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest_local(decode);
	hyperstone_movi(decode);
}

void hyperstone_device::op67()
{
	regs_decode decode;
	decode_immediate_s(decode);
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest_local(decode);
	hyperstone_movi(decode);
}

void hyperstone_device::op68()
{
	regs_decode decode;
	decode_immediate_u(decode);
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest_noh(decode);
	hyperstone_addi(decode);
}

void hyperstone_device::op69()
{
	regs_decode decode;
	decode_immediate_s(decode);
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest_noh(decode);
	hyperstone_addi(decode);
}

void hyperstone_device::op6a()
{
	regs_decode decode;
	decode_immediate_u(decode);
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest_local(decode);
	hyperstone_addi(decode);
}

void hyperstone_device::op6b()
{
	regs_decode decode;
	decode_immediate_s(decode);
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest_local(decode);
	hyperstone_addi(decode);
}

void hyperstone_device::op6c()
{
	regs_decode decode;
	decode_immediate_u(decode);
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest_noh(decode);
	hyperstone_addsi(decode);
}

void hyperstone_device::op6d()
{
	regs_decode decode;
	decode_immediate_s(decode);
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest_noh(decode);
	hyperstone_addsi(decode);
}

void hyperstone_device::op6e()
{
	regs_decode decode;
	decode_immediate_u(decode);
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest_local(decode);
	hyperstone_addsi(decode);
}

void hyperstone_device::op6f()
{
	regs_decode decode;
	decode_immediate_s(decode);
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest_local(decode);
	hyperstone_addsi(decode);
}



void hyperstone_device::op70()
{
	regs_decode decode;
	decode_immediate_u(decode);
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest_noh(decode);
	hyperstone_cmpbi(decode);
}

void hyperstone_device::op71()
{
	regs_decode decode;
	decode_immediate_s(decode);
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest_noh(decode);
	hyperstone_cmpbi(decode);
}

void hyperstone_device::op72()
{
	regs_decode decode;
	decode_immediate_u(decode);
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest_local(decode);
	hyperstone_cmpbi(decode);
}

void hyperstone_device::op73()
{
	regs_decode decode;
	decode_immediate_s(decode);
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest_local(decode);
	hyperstone_cmpbi(decode);
}

void hyperstone_device::op74()
{
	regs_decode decode;
	decode_immediate_u(decode);
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest_noh(decode);
	hyperstone_andni(decode);
}

void hyperstone_device::op75()
{
	regs_decode decode;
	decode_immediate_s(decode);
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest_noh(decode);
	hyperstone_andni(decode);
}

void hyperstone_device::op76()
{
	regs_decode decode;
	decode_immediate_u(decode);
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest_local(decode);
	hyperstone_andni(decode);
}

void hyperstone_device::op77()
{
	regs_decode decode;
	decode_immediate_s(decode);
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest_local(decode);
	hyperstone_andni(decode);
}

void hyperstone_device::op78()
{
	regs_decode decode;
	decode_immediate_u(decode);
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest_noh(decode);
	hyperstone_ori(decode);
}

void hyperstone_device::op79()
{
	regs_decode decode;
	decode_immediate_s(decode);
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest_noh(decode);
	hyperstone_ori(decode);
}

void hyperstone_device::op7a()
{
	regs_decode decode;
	decode_immediate_u(decode);
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest_local(decode);
	hyperstone_ori(decode);
}

void hyperstone_device::op7b()
{
	regs_decode decode;
	decode_immediate_s(decode);
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest_local(decode);
	hyperstone_ori(decode);
}

void hyperstone_device::op7c()
{
	regs_decode decode;
	decode_immediate_u(decode);
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest_noh(decode);
	hyperstone_xori(decode);
}

void hyperstone_device::op7d()
{
	regs_decode decode;
	decode_immediate_s(decode);
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest_noh(decode);
	hyperstone_xori(decode);
}

void hyperstone_device::op7e()
{
	regs_decode decode;
	decode_immediate_u(decode);
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest_local(decode);
	hyperstone_xori(decode);
}

void hyperstone_device::op7f()
{
	regs_decode decode;
	decode_immediate_s(decode);
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest_local(decode);
	hyperstone_xori(decode);
}



void hyperstone_device::op80()
{
	regs_decode decode;
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest_local(decode);
	hyperstone_shrdi(decode);
}

void hyperstone_device::op81()
{
	regs_decode decode;
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest_local(decode);
	hyperstone_shrdi(decode);
}

void hyperstone_device::op82()
{
	regs_decode decode;
	check_delay_PC();
	decode_LL(decode);
	hyperstone_shrd(decode);
}

void hyperstone_device::op83()
{
	regs_decode decode;
	check_delay_PC();
	decode_LL(decode);
	hyperstone_shr(decode);
}

void hyperstone_device::op84()
{
	regs_decode decode;
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest_local(decode);
	hyperstone_sardi(decode);
}

void hyperstone_device::op85()
{
	regs_decode decode;
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest_local(decode);
	hyperstone_sardi(decode);
}

void hyperstone_device::op86()
{
	regs_decode decode;
	check_delay_PC();
	decode_LL(decode);
	hyperstone_sard(decode);
}

void hyperstone_device::op87()
{
	regs_decode decode;
	check_delay_PC();
	decode_LL(decode);
	hyperstone_sar(decode);
}

void hyperstone_device::op88()
{
	regs_decode decode;
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest_local(decode);
	hyperstone_shldi(decode);
}

void hyperstone_device::op89()
{
	regs_decode decode;
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest_local(decode);
	hyperstone_shldi(decode);
}

void hyperstone_device::op8a()
{
	regs_decode decode;
	check_delay_PC();
	decode_LL(decode);
	hyperstone_shld(decode);
}

void hyperstone_device::op8b()
{
	regs_decode decode;
	check_delay_PC();
	decode_LL(decode);
	hyperstone_shl(decode);
}

void hyperstone_device::op8c()
{
	DEBUG_PRINTF(("Executed Reserved opcode. PC = %08X OP = %04X\n", PC, OP));
}

void hyperstone_device::op8d()
{
	DEBUG_PRINTF(("Executed Reserved opcode. PC = %08X OP = %04X\n", PC, OP));
}

void hyperstone_device::op8e()
{
	regs_decode decode;
	check_delay_PC();
	decode_LL(decode);
	hyperstone_testlz(decode);
}

void hyperstone_device::op8f()
{
	regs_decode decode;
	check_delay_PC();
	decode_LL(decode);
	hyperstone_rol(decode);
}



void hyperstone_device::op90()
{
	regs_decode decode;
	decode_dis(decode);
	check_delay_PC();
	decode_RR00(decode);
	hyperstone_ldxx1(decode);
}

void hyperstone_device::op91()
{
	regs_decode decode;
	decode_dis(decode);
	check_delay_PC();
	decode_RR01(decode);
	hyperstone_ldxx1(decode);
}

void hyperstone_device::op92()
{
	regs_decode decode;
	decode_dis(decode);
	check_delay_PC();
	decode_RR10(decode);
	hyperstone_ldxx1(decode);
}

void hyperstone_device::op93()
{
	regs_decode decode;
	decode_dis(decode);
	check_delay_PC();
	decode_RR11(decode);
	hyperstone_ldxx1(decode);
}

void hyperstone_device::op94()
{
	regs_decode decode;
	decode_dis(decode);
	check_delay_PC();
	decode_RR00(decode);
	hyperstone_ldxx2(decode);
}

void hyperstone_device::op95()
{
	regs_decode decode;
	decode_dis(decode);
	check_delay_PC();
	decode_RR01(decode);
	hyperstone_ldxx2(decode);
}

void hyperstone_device::op96()
{
	regs_decode decode;
	decode_dis(decode);
	check_delay_PC();
	decode_RR10(decode);
	hyperstone_ldxx2(decode);
}

void hyperstone_device::op97()
{
	regs_decode decode;
	decode_dis(decode);
	check_delay_PC();
	decode_RR11(decode);
	hyperstone_ldxx2(decode);
}

void hyperstone_device::op98()
{
	regs_decode decode;
	decode_dis(decode);
	check_delay_PC();
	decode_RR00(decode);
	hyperstone_stxx1(decode);
}

void hyperstone_device::op99()
{
	regs_decode decode;
	decode_dis(decode);
	check_delay_PC();
	decode_RR01(decode);
	hyperstone_stxx1(decode);
}

void hyperstone_device::op9a()
{
	regs_decode decode;
	decode_dis(decode);
	check_delay_PC();
	decode_RR10(decode);
	hyperstone_stxx1(decode);
}

void hyperstone_device::op9b()
{
	regs_decode decode;
	decode_dis(decode);
	check_delay_PC();
	decode_RR11(decode);
	hyperstone_stxx1(decode);
}

void hyperstone_device::op9c()
{
	regs_decode decode;
	decode_dis(decode);
	check_delay_PC();
	decode_RR00(decode);
	hyperstone_stxx2(decode);
}

void hyperstone_device::op9d()
{
	regs_decode decode;
	decode_dis(decode);
	check_delay_PC();
	decode_RR01(decode);
	hyperstone_stxx2(decode);
}

void hyperstone_device::op9e()
{
	regs_decode decode;
	decode_dis(decode);
	check_delay_PC();
	decode_RR10(decode);
	hyperstone_stxx2(decode);
}

void hyperstone_device::op9f()
{
	regs_decode decode;
	decode_dis(decode);
	check_delay_PC();
	decode_RR11(decode);
	hyperstone_stxx2(decode);
}



void hyperstone_device::opa0()
{
	regs_decode decode;
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest_noh(decode);
	hyperstone_shri(decode);
}

void hyperstone_device::opa1()
{
	regs_decode decode;
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest_noh(decode);
	hyperstone_shri(decode);
}

void hyperstone_device::opa2()
{
	regs_decode decode;
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest_local(decode);
	hyperstone_shri(decode);
}

void hyperstone_device::opa3()
{
	regs_decode decode;
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest_local(decode);
	hyperstone_shri(decode);
}

void hyperstone_device::opa4()
{
	regs_decode decode;
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest_noh(decode);
	hyperstone_sari(decode);
}

void hyperstone_device::opa5()
{
	regs_decode decode;
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest_noh(decode);
	hyperstone_sari(decode);
}

void hyperstone_device::opa6()
{
	regs_decode decode;
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest_local(decode);
	hyperstone_sari(decode);
}

void hyperstone_device::opa7()
{
	regs_decode decode;
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest_local(decode);
	hyperstone_sari(decode);
}

void hyperstone_device::opa8()
{
	regs_decode decode;
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest_noh(decode);
	hyperstone_shli(decode);
}

void hyperstone_device::opa9()
{
	regs_decode decode;
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest_noh(decode);
	hyperstone_shli(decode);
}

void hyperstone_device::opaa()
{
	regs_decode decode;
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest_local(decode);
	hyperstone_shli(decode);
}

void hyperstone_device::opab()
{
	regs_decode decode;
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest_local(decode);
	hyperstone_shli(decode);
}

void hyperstone_device::opac()
{
	DEBUG_PRINTF(("Executed Reserved opcode. PC = %08X OP = %04X\n", PC, OP));
}

void hyperstone_device::opad()
{
	DEBUG_PRINTF(("Executed Reserved opcode. PC = %08X OP = %04X\n", PC, OP));
}

void hyperstone_device::opae()
{
	DEBUG_PRINTF(("Executed Reserved opcode. PC = %08X OP = %04X\n", PC, OP));
}

void hyperstone_device::opaf()
{
	DEBUG_PRINTF(("Executed Reserved opcode. PC = %08X OP = %04X\n", PC, OP));
}



void hyperstone_device::opb0()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR00(decode);
	hyperstone_mulu(decode);
}

void hyperstone_device::opb1()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR01(decode);
	hyperstone_mulu(decode);
}

void hyperstone_device::opb2()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR10(decode);
	hyperstone_mulu(decode);
}

void hyperstone_device::opb3()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR11(decode);
	hyperstone_mulu(decode);
}

void hyperstone_device::opb4()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR00(decode);
	hyperstone_muls(decode);
}

void hyperstone_device::opb5()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR01(decode);
	hyperstone_muls(decode);
}

void hyperstone_device::opb6()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR10(decode);
	hyperstone_muls(decode);
}

void hyperstone_device::opb7()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR11(decode);
	hyperstone_muls(decode);
}

void hyperstone_device::opb8()
{
	regs_decode decode;
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest_noh(decode);
	hyperstone_set(decode);
}

void hyperstone_device::opb9()
{
	regs_decode decode;
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest_noh(decode);
	hyperstone_set(decode);
}

void hyperstone_device::opba()
{
	regs_decode decode;
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest_local(decode);
	hyperstone_set(decode);
}

void hyperstone_device::opbb()
{
	regs_decode decode;
	check_delay_PC();
	decode.dst = DST_CODE;
	decode_dest_local(decode);
	hyperstone_set(decode);
}

void hyperstone_device::opbc()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR00(decode);
	hyperstone_mul(decode);
}

void hyperstone_device::opbd()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR01(decode);
	hyperstone_mul(decode);
}

void hyperstone_device::opbe()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR10(decode);
	hyperstone_mul(decode);
}

void hyperstone_device::opbf()
{
	regs_decode decode;
	check_delay_PC();
	decode_RR11(decode);
	hyperstone_mul(decode);
}



void hyperstone_device::opc0()
{
	regs_decode decode;
	check_delay_PC();
	decode_LL(decode);
	hyperstone_fadd(decode);
}

void hyperstone_device::opc1()
{
	regs_decode decode;
	check_delay_PC();
	decode_LL(decode);
	hyperstone_faddd(decode);
}

void hyperstone_device::opc2()
{
	regs_decode decode;
	check_delay_PC();
	decode_LL(decode);
	hyperstone_fsub(decode);
}

void hyperstone_device::opc3()
{
	regs_decode decode;
	check_delay_PC();
	decode_LL(decode);
	hyperstone_fsubd(decode);
}

void hyperstone_device::opc4()
{
	regs_decode decode;
	check_delay_PC();
	decode_LL(decode);
	hyperstone_fmul(decode);
}

void hyperstone_device::opc5()
{
	regs_decode decode;
	check_delay_PC();
	decode_LL(decode);
	hyperstone_fmuld(decode);
}

void hyperstone_device::opc6()
{
	regs_decode decode;
	check_delay_PC();
	decode_LL(decode);
	hyperstone_fdiv(decode);
}

void hyperstone_device::opc7()
{
	regs_decode decode;
	check_delay_PC();
	decode_LL(decode);
	hyperstone_fdivd(decode);
}

void hyperstone_device::opc8()
{
	regs_decode decode;
	check_delay_PC();
	decode_LL(decode);
	hyperstone_fcmp(decode);
}

void hyperstone_device::opc9()
{
	regs_decode decode;
	check_delay_PC();
	decode_LL(decode);
	hyperstone_fcmpd(decode);
}

void hyperstone_device::opca()
{
	regs_decode decode;
	check_delay_PC();
	decode_LL(decode);
	hyperstone_fcmpu(decode);
}

void hyperstone_device::opcb()
{
	regs_decode decode;
	check_delay_PC();
	decode_LL(decode);
	hyperstone_fcmpud(decode);
}

void hyperstone_device::opcc()
{
	regs_decode decode;
	check_delay_PC();
	decode_LL(decode);
	hyperstone_fcvt(decode);
}

void hyperstone_device::opcd()
{
	regs_decode decode;
	check_delay_PC();
	decode_LL(decode);
	hyperstone_fcvtd(decode);
}

void hyperstone_device::opce()
{
	regs_decode decode;
	m_instruction_length = (2<<19);
	EXTRA_U = READ_OP(PC);
	PC += 2;
	check_delay_PC();
	decode_LL(decode);
	hyperstone_extend(decode);
}

void hyperstone_device::opcf()
{
	regs_decode decode;
	check_delay_PC();
	decode_LL(decode);
	hyperstone_do(decode);
}



void hyperstone_device::opd0()
{
	regs_decode decode;
	check_delay_PC();
	decode_LR(decode, 0);
	hyperstone_ldwr(decode);
}

void hyperstone_device::opd1()
{
	regs_decode decode;
	check_delay_PC();
	decode_LR(decode, 1);
	hyperstone_ldwr(decode);
}

void hyperstone_device::opd2()
{
	regs_decode decode;
	check_delay_PC();
	decode_LR(decode, 0);
	hyperstone_lddr(decode);
}

void hyperstone_device::opd3()
{
	regs_decode decode;
	check_delay_PC();
	decode_LR(decode, 1);
	hyperstone_lddr(decode);
}

void hyperstone_device::opd4()
{
	regs_decode decode;
	check_delay_PC();
	decode_LR(decode, 0);
	hyperstone_ldwp(decode);
}

void hyperstone_device::opd5()
{
	regs_decode decode;
	check_delay_PC();
	decode_LR(decode, 1);
	hyperstone_ldwp(decode);
}

void hyperstone_device::opd6()
{
	regs_decode decode;
	check_delay_PC();
	decode_LR(decode, 0);
	hyperstone_lddp(decode);
}

void hyperstone_device::opd7()
{
	regs_decode decode;
	check_delay_PC();
	decode_LR(decode, 1);
	hyperstone_lddp(decode);
}

void hyperstone_device::opd8()
{
	regs_decode decode;
	check_delay_PC();
	decode_LR(decode, 0);
	hyperstone_stwr(decode);
}

void hyperstone_device::opd9()
{
	regs_decode decode;
	check_delay_PC();
	decode_LR(decode, 1);
	hyperstone_stwr(decode);
}

void hyperstone_device::opda()
{
	regs_decode decode;
	check_delay_PC();
	decode_LR(decode, 0);
	hyperstone_stdr(decode);
}

void hyperstone_device::opdb()
{
	regs_decode decode;
	check_delay_PC();
	decode_LR(decode, 1);
	hyperstone_stdr(decode);
}

void hyperstone_device::opdc()
{
	regs_decode decode;
	check_delay_PC();
	decode_LR(decode, 0);
	hyperstone_stwp(decode);
}

void hyperstone_device::opdd()
{
	regs_decode decode;
	check_delay_PC();
	decode_LR(decode, 1);
	hyperstone_stwp(decode);
}

void hyperstone_device::opde()
{
	regs_decode decode;
	check_delay_PC();
	decode_LR(decode, 0);
	hyperstone_stdp(decode);
}

void hyperstone_device::opdf()
{
	regs_decode decode;
	check_delay_PC();
	decode_LR(decode, 1);
	hyperstone_stdp(decode);
}



void hyperstone_device::ope0()
{
	regs_decode decode;
	decode_pcrel(decode);
	check_delay_PC();
	hyperstone_dbv(decode);
}

void hyperstone_device::ope1()
{
	regs_decode decode;
	decode_pcrel(decode);
	check_delay_PC();
	hyperstone_dbnv(decode);
}

void hyperstone_device::ope2()
{
	regs_decode decode;
	decode_pcrel(decode);
	check_delay_PC();
	hyperstone_dbe(decode);
}

void hyperstone_device::ope3()
{
	regs_decode decode;
	decode_pcrel(decode);
	check_delay_PC();
	hyperstone_dbne(decode);
}

void hyperstone_device::ope4()
{
	regs_decode decode;
	decode_pcrel(decode);
	check_delay_PC();
	hyperstone_dbc(decode);
}

void hyperstone_device::ope5()
{
	regs_decode decode;
	decode_pcrel(decode);
	check_delay_PC();
	hyperstone_dbnc(decode);
}

void hyperstone_device::ope6()
{
	regs_decode decode;
	decode_pcrel(decode);
	check_delay_PC();
	hyperstone_dbse(decode);
}

void hyperstone_device::ope7()
{
	regs_decode decode;
	decode_pcrel(decode);
	check_delay_PC();
	hyperstone_dbht(decode);
}

void hyperstone_device::ope8()
{
	regs_decode decode;
	decode_pcrel(decode);
	check_delay_PC();
	hyperstone_dbn(decode);
}

void hyperstone_device::ope9()
{
	regs_decode decode;
	decode_pcrel(decode);
	check_delay_PC();
	hyperstone_dbnn(decode);
}

void hyperstone_device::opea()
{
	regs_decode decode;
	decode_pcrel(decode);
	check_delay_PC();
	hyperstone_dble(decode);
}

void hyperstone_device::opeb()
{
	regs_decode decode;
	decode_pcrel(decode);
	check_delay_PC();
	hyperstone_dbgt(decode);
}

void hyperstone_device::opec()
{
	regs_decode decode;
	decode_pcrel(decode);
	check_delay_PC();
	hyperstone_dbr(decode);
}

void hyperstone_device::oped()
{
	regs_decode decode;
	check_delay_PC();
	decode_LL(decode);
	hyperstone_frame(decode);
}

void hyperstone_device::opee()
{
	regs_decode decode;
	decode_const(decode);
	check_delay_PC();
	decode_LR(decode, 0);
	hyperstone_call(decode);
}

void hyperstone_device::opef()
{
	regs_decode decode;
	decode_const(decode);
	check_delay_PC();
	decode_LR(decode, 1);
	hyperstone_call(decode);
}



void hyperstone_device::opf0()
{
	regs_decode decode;
	decode_pcrel(decode);
	check_delay_PC();
	hyperstone_bv(decode);
}

void hyperstone_device::opf1()
{
	regs_decode decode;
	decode_pcrel(decode);
	check_delay_PC();
	hyperstone_bnv(decode);
}

void hyperstone_device::opf2()
{
	regs_decode decode;
	decode_pcrel(decode);
	check_delay_PC();
	hyperstone_be(decode);
}

void hyperstone_device::opf3()
{
	regs_decode decode;
	decode_pcrel(decode);
	check_delay_PC();
	hyperstone_bne(decode);
}

void hyperstone_device::opf4()
{
	regs_decode decode;
	decode_pcrel(decode);
	check_delay_PC();
	hyperstone_bc(decode);
}

void hyperstone_device::opf5()
{
	regs_decode decode;
	decode_pcrel(decode);
	check_delay_PC();
	hyperstone_bnc(decode);
}

void hyperstone_device::opf6()
{
	regs_decode decode;
	decode_pcrel(decode);
	check_delay_PC();
	hyperstone_bse(decode);
}

void hyperstone_device::opf7()
{
	regs_decode decode;
	decode_pcrel(decode);
	check_delay_PC();
	hyperstone_bht(decode);
}

void hyperstone_device::opf8()
{
	regs_decode decode;
	decode_pcrel(decode);
	check_delay_PC();
	hyperstone_bn(decode);
}

void hyperstone_device::opf9()
{
	regs_decode decode;
	decode_pcrel(decode);
	check_delay_PC();
	hyperstone_bnn(decode);
}

void hyperstone_device::opfa()
{
	regs_decode decode;
	decode_pcrel(decode);
	check_delay_PC();
	hyperstone_ble(decode);
}

void hyperstone_device::opfb()
{
	regs_decode decode;
	decode_pcrel(decode);
	check_delay_PC();
	hyperstone_bgt(decode);
}

void hyperstone_device::opfc()
{
	regs_decode decode;
	decode_pcrel(decode);
	check_delay_PC();
	hyperstone_br(decode);
}

void hyperstone_device::opfd()
{
	regs_decode decode;
	check_delay_PC();
	hyperstone_trap(decode);
}

void hyperstone_device::opfe()
{
	regs_decode decode;
	check_delay_PC();
	hyperstone_trap(decode);
}

void hyperstone_device::opff()
{
	regs_decode decode;
	check_delay_PC();
	hyperstone_trap(decode);
}

const hyperstone_device::ophandler hyperstone_device::s_opcodetable[256] =
{
	&hyperstone_device::op00, &hyperstone_device::op01, &hyperstone_device::op02, &hyperstone_device::op03,
	&hyperstone_device::op04, &hyperstone_device::op05, &hyperstone_device::op06, &hyperstone_device::op07,
	&hyperstone_device::op08, &hyperstone_device::op09, &hyperstone_device::op0a, &hyperstone_device::op0b,
	&hyperstone_device::op0c, &hyperstone_device::op0d, &hyperstone_device::op0e, &hyperstone_device::op0f,

	&hyperstone_device::op10, &hyperstone_device::op11, &hyperstone_device::op12, &hyperstone_device::op13,
	&hyperstone_device::op14, &hyperstone_device::op15, &hyperstone_device::op16, &hyperstone_device::op17,
	&hyperstone_device::op18, &hyperstone_device::op19, &hyperstone_device::op1a, &hyperstone_device::op1b,
	&hyperstone_device::op1c, &hyperstone_device::op1d, &hyperstone_device::op1e, &hyperstone_device::op1f,

	&hyperstone_device::op20, &hyperstone_device::op21, &hyperstone_device::op22, &hyperstone_device::op23,
	&hyperstone_device::op24, &hyperstone_device::op25, &hyperstone_device::op26, &hyperstone_device::op27,
	&hyperstone_device::op28, &hyperstone_device::op29, &hyperstone_device::op2a, &hyperstone_device::op2b,
	&hyperstone_device::op2c, &hyperstone_device::op2d, &hyperstone_device::op2e, &hyperstone_device::op2f,

	&hyperstone_device::op30, &hyperstone_device::op31, &hyperstone_device::op32, &hyperstone_device::op33,
	&hyperstone_device::op34, &hyperstone_device::op35, &hyperstone_device::op36, &hyperstone_device::op37,
	&hyperstone_device::op38, &hyperstone_device::op39, &hyperstone_device::op3a, &hyperstone_device::op3b,
	&hyperstone_device::op3c, &hyperstone_device::op3d, &hyperstone_device::op3e, &hyperstone_device::op3f,

	&hyperstone_device::op40, &hyperstone_device::op41, &hyperstone_device::op42, &hyperstone_device::op43,
	&hyperstone_device::op44, &hyperstone_device::op45, &hyperstone_device::op46, &hyperstone_device::op47,
	&hyperstone_device::op48, &hyperstone_device::op49, &hyperstone_device::op4a, &hyperstone_device::op4b,
	&hyperstone_device::op4c, &hyperstone_device::op4d, &hyperstone_device::op4e, &hyperstone_device::op4f,

	&hyperstone_device::op50, &hyperstone_device::op51, &hyperstone_device::op52, &hyperstone_device::op53,
	&hyperstone_device::op54, &hyperstone_device::op55, &hyperstone_device::op56, &hyperstone_device::op57,
	&hyperstone_device::op58, &hyperstone_device::op59, &hyperstone_device::op5a, &hyperstone_device::op5b,
	&hyperstone_device::op5c, &hyperstone_device::op5d, &hyperstone_device::op5e, &hyperstone_device::op5f,

	&hyperstone_device::op60, &hyperstone_device::op61, &hyperstone_device::op62, &hyperstone_device::op63,
	&hyperstone_device::op64, &hyperstone_device::op65, &hyperstone_device::op66, &hyperstone_device::op67,
	&hyperstone_device::op68, &hyperstone_device::op69, &hyperstone_device::op6a, &hyperstone_device::op6b,
	&hyperstone_device::op6c, &hyperstone_device::op6d, &hyperstone_device::op6e, &hyperstone_device::op6f,

	&hyperstone_device::op70, &hyperstone_device::op71, &hyperstone_device::op72, &hyperstone_device::op73,
	&hyperstone_device::op74, &hyperstone_device::op75, &hyperstone_device::op76, &hyperstone_device::op77,
	&hyperstone_device::op78, &hyperstone_device::op79, &hyperstone_device::op7a, &hyperstone_device::op7b,
	&hyperstone_device::op7c, &hyperstone_device::op7d, &hyperstone_device::op7e, &hyperstone_device::op7f,

	&hyperstone_device::op80, &hyperstone_device::op81, &hyperstone_device::op82, &hyperstone_device::op83,
	&hyperstone_device::op84, &hyperstone_device::op85, &hyperstone_device::op86, &hyperstone_device::op87,
	&hyperstone_device::op88, &hyperstone_device::op89, &hyperstone_device::op8a, &hyperstone_device::op8b,
	&hyperstone_device::op8c, &hyperstone_device::op8d, &hyperstone_device::op8e, &hyperstone_device::op8f,

	&hyperstone_device::op90, &hyperstone_device::op91, &hyperstone_device::op92, &hyperstone_device::op93,
	&hyperstone_device::op94, &hyperstone_device::op95, &hyperstone_device::op96, &hyperstone_device::op97,
	&hyperstone_device::op98, &hyperstone_device::op99, &hyperstone_device::op9a, &hyperstone_device::op9b,
	&hyperstone_device::op9c, &hyperstone_device::op9d, &hyperstone_device::op9e, &hyperstone_device::op9f,

	&hyperstone_device::opa0, &hyperstone_device::opa1, &hyperstone_device::opa2, &hyperstone_device::opa3,
	&hyperstone_device::opa4, &hyperstone_device::opa5, &hyperstone_device::opa6, &hyperstone_device::opa7,
	&hyperstone_device::opa8, &hyperstone_device::opa9, &hyperstone_device::opaa, &hyperstone_device::opab,
	&hyperstone_device::opac, &hyperstone_device::opad, &hyperstone_device::opae, &hyperstone_device::opaf,

	&hyperstone_device::opb0, &hyperstone_device::opb1, &hyperstone_device::opb2, &hyperstone_device::opb3,
	&hyperstone_device::opb4, &hyperstone_device::opb5, &hyperstone_device::opb6, &hyperstone_device::opb7,
	&hyperstone_device::opb8, &hyperstone_device::opb9, &hyperstone_device::opba, &hyperstone_device::opbb,
	&hyperstone_device::opbc, &hyperstone_device::opbd, &hyperstone_device::opbe, &hyperstone_device::opbf,

	&hyperstone_device::opc0, &hyperstone_device::opc1, &hyperstone_device::opc2, &hyperstone_device::opc3,
	&hyperstone_device::opc4, &hyperstone_device::opc5, &hyperstone_device::opc6, &hyperstone_device::opc7,
	&hyperstone_device::opc8, &hyperstone_device::opc9, &hyperstone_device::opca, &hyperstone_device::opcb,
	&hyperstone_device::opcc, &hyperstone_device::opcd, &hyperstone_device::opce, &hyperstone_device::opcf,

	&hyperstone_device::opd0, &hyperstone_device::opd1, &hyperstone_device::opd2, &hyperstone_device::opd3,
	&hyperstone_device::opd4, &hyperstone_device::opd5, &hyperstone_device::opd6, &hyperstone_device::opd7,
	&hyperstone_device::opd8, &hyperstone_device::opd9, &hyperstone_device::opda, &hyperstone_device::opdb,
	&hyperstone_device::opdc, &hyperstone_device::opdd, &hyperstone_device::opde, &hyperstone_device::opdf,

	&hyperstone_device::ope0, &hyperstone_device::ope1, &hyperstone_device::ope2, &hyperstone_device::ope3,
	&hyperstone_device::ope4, &hyperstone_device::ope5, &hyperstone_device::ope6, &hyperstone_device::ope7,
	&hyperstone_device::ope8, &hyperstone_device::ope9, &hyperstone_device::opea, &hyperstone_device::opeb,
	&hyperstone_device::opec, &hyperstone_device::oped, &hyperstone_device::opee, &hyperstone_device::opef,

	&hyperstone_device::opf0, &hyperstone_device::opf1, &hyperstone_device::opf2, &hyperstone_device::opf3,
	&hyperstone_device::opf4, &hyperstone_device::opf5, &hyperstone_device::opf6, &hyperstone_device::opf7,
	&hyperstone_device::opf8, &hyperstone_device::opf9, &hyperstone_device::opfa, &hyperstone_device::opfb,
	&hyperstone_device::opfc, &hyperstone_device::opfd, &hyperstone_device::opfe, &hyperstone_device::opff
};
