// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
#define LOCAL_DECODE_INIT \
	struct regs_decode decode_state; \
	struct regs_decode *decode = &decode_state; \
\
	/* clear 'current regs / flags' */ \
	decode->src = 0; \
	decode->dst = 0; \
	decode->src_value = 0; \
	decode->next_src_value = 0; \
	decode->dst_value = 0; \
	decode->next_dst_value = 0; \
	decode->sub_type = 0; \
	decode->extra.u = 0; \
	decode->src_is_local = 0; \
	decode->dst_is_local = 0; \
	decode->same_src_dst = 0; \
	decode->same_src_dstf = 0; \
	decode->same_srcf_dst = 0;

void hyperstone_device::op00()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_chk(decode);
}

void hyperstone_device::op01()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_chk(decode);
}

void hyperstone_device::op02()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_chk(decode);
}

void hyperstone_device::op03()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_chk(decode);
}

void hyperstone_device::op04()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_movd(decode);
}

void hyperstone_device::op05()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_movd(decode);
}

void hyperstone_device::op06()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_movd(decode);
}

void hyperstone_device::op07()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_movd(decode);
}

void hyperstone_device::op08()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_divu(decode);
}

void hyperstone_device::op09()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_divu(decode);
}

void hyperstone_device::op0a()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_divu(decode);
}

void hyperstone_device::op0b()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_divu(decode);
}

void hyperstone_device::op0c()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_divs(decode);
}

void hyperstone_device::op0d()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_divs(decode);
}

void hyperstone_device::op0e()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_divs(decode);
}

void hyperstone_device::op0f()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_divs(decode);
}



void hyperstone_device::op10()
{
	LOCAL_DECODE_INIT;
	RRlimdecode(decode, 0, 0);
	hyperstone_xm(decode);
}

void hyperstone_device::op11()
{
	LOCAL_DECODE_INIT;
	RRlimdecode(decode, 0, 1);
	hyperstone_xm(decode);
}

void hyperstone_device::op12()
{
	LOCAL_DECODE_INIT;
	RRlimdecode(decode, 1, 0);
	hyperstone_xm(decode);
}

void hyperstone_device::op13()
{
	LOCAL_DECODE_INIT;
	RRlimdecode(decode, 1, 1);
	hyperstone_xm(decode);
}

void hyperstone_device::op14()
{
	LOCAL_DECODE_INIT;
	RRconstdecode(decode, 0, 0);
	hyperstone_mask(decode);
}

void hyperstone_device::op15()
{
	LOCAL_DECODE_INIT;
	RRconstdecode(decode, 0, 1);
	hyperstone_mask(decode);
}

void hyperstone_device::op16()
{
	LOCAL_DECODE_INIT;
	RRconstdecode(decode, 1, 0);
	hyperstone_mask(decode);
}

void hyperstone_device::op17()
{
	LOCAL_DECODE_INIT;
	RRconstdecode(decode, 1, 1);
	hyperstone_mask(decode);
}

void hyperstone_device::op18()
{
	LOCAL_DECODE_INIT;
	RRconstdecode(decode, 0, 0);
	hyperstone_sum(decode);
}

void hyperstone_device::op19()
{
	LOCAL_DECODE_INIT;
	RRconstdecode(decode, 0, 1);
	hyperstone_sum(decode);
}

void hyperstone_device::op1a()
{
	LOCAL_DECODE_INIT;
	RRconstdecode(decode, 1, 0);
	hyperstone_sum(decode);
}

void hyperstone_device::op1b()
{
	LOCAL_DECODE_INIT;
	RRconstdecode(decode, 1, 1);
	hyperstone_sum(decode);
}

void hyperstone_device::op1c()
{
	LOCAL_DECODE_INIT;
	RRconstdecode(decode, 0, 0);
	hyperstone_sums(decode);
}

void hyperstone_device::op1d()
{
	LOCAL_DECODE_INIT;
	RRconstdecode(decode, 0, 1);
	hyperstone_sums(decode);
}

void hyperstone_device::op1e()
{
	LOCAL_DECODE_INIT;
	RRconstdecode(decode, 1, 0);
	hyperstone_sums(decode);
}

void hyperstone_device::op1f()
{
	LOCAL_DECODE_INIT;
	RRconstdecode(decode, 1, 1);
	hyperstone_sums(decode);
}



void hyperstone_device::op20()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_cmp(decode);
}

void hyperstone_device::op21()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_cmp(decode);
}

void hyperstone_device::op22()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_cmp(decode);
}

void hyperstone_device::op23()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_cmp(decode);
}

void hyperstone_device::op24()
{
	LOCAL_DECODE_INIT;
	RRdecodewithHflag(decode, 0, 0);
	hyperstone_mov(decode);
}

void hyperstone_device::op25()
{
	LOCAL_DECODE_INIT;
	RRdecodewithHflag(decode, 0, 1);
	hyperstone_mov(decode);
}

void hyperstone_device::op26()
{
	LOCAL_DECODE_INIT;
	RRdecodewithHflag(decode, 1, 0);
	hyperstone_mov(decode);
}

void hyperstone_device::op27()
{
	LOCAL_DECODE_INIT;
	RRdecodewithHflag(decode, 1, 1);
	hyperstone_mov(decode);
}

void hyperstone_device::op28()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_add(decode);
}

void hyperstone_device::op29()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_add(decode);
}

void hyperstone_device::op2a()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_add(decode);
}

void hyperstone_device::op2b()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_add(decode);
}

void hyperstone_device::op2c()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_adds(decode);
}

void hyperstone_device::op2d()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_adds(decode);
}

void hyperstone_device::op2e()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_adds(decode);
}

void hyperstone_device::op2f()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_adds(decode);
}



void hyperstone_device::op30()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_cmpb(decode);
}

void hyperstone_device::op31()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_cmpb(decode);
}

void hyperstone_device::op32()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_cmpb(decode);
}

void hyperstone_device::op33()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_cmpb(decode);
}

void hyperstone_device::op34()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_andn(decode);
}

void hyperstone_device::op35()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_andn(decode);
}

void hyperstone_device::op36()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_andn(decode);
}

void hyperstone_device::op37()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_andn(decode);
}

void hyperstone_device::op38()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_or(decode);
}

void hyperstone_device::op39()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_or(decode);
}

void hyperstone_device::op3a()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_or(decode);
}

void hyperstone_device::op3b()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_or(decode);
}

void hyperstone_device::op3c()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_xor(decode);
}

void hyperstone_device::op3d()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_xor(decode);
}

void hyperstone_device::op3e()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_xor(decode);
}

void hyperstone_device::op3f()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_xor(decode);
}



void hyperstone_device::op40()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_subc(decode);
}

void hyperstone_device::op41()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_subc(decode);
}

void hyperstone_device::op42()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_subc(decode);
}

void hyperstone_device::op43()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_subc(decode);
}

void hyperstone_device::op44()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_not(decode);
}

void hyperstone_device::op45()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_not(decode);
}

void hyperstone_device::op46()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_not(decode);
}

void hyperstone_device::op47()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_not(decode);
}

void hyperstone_device::op48()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_sub(decode);
}

void hyperstone_device::op49()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_sub(decode);
}

void hyperstone_device::op4a()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_sub(decode);
}

void hyperstone_device::op4b()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_sub(decode);
}

void hyperstone_device::op4c()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_subs(decode);
}

void hyperstone_device::op4d()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_subs(decode);
}

void hyperstone_device::op4e()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_subs(decode);
}

void hyperstone_device::op4f()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_subs(decode);
}



void hyperstone_device::op50()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_addc(decode);
}

void hyperstone_device::op51()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_addc(decode);
}

void hyperstone_device::op52()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_addc(decode);
}

void hyperstone_device::op53()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_addc(decode);
}

void hyperstone_device::op54()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_and(decode);
}

void hyperstone_device::op55()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_and(decode);
}

void hyperstone_device::op56()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_and(decode);
}

void hyperstone_device::op57()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_and(decode);
}

void hyperstone_device::op58()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_neg(decode);
}

void hyperstone_device::op59()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_neg(decode);
}

void hyperstone_device::op5a()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_neg(decode);
}

void hyperstone_device::op5b()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_neg(decode);
}

void hyperstone_device::op5c()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_negs(decode);
}

void hyperstone_device::op5d()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_negs(decode);
}

void hyperstone_device::op5e()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_negs(decode);
}

void hyperstone_device::op5f()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_negs(decode);
}



void hyperstone_device::op60()
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 0, 0);
	hyperstone_cmpi(decode);
}

void hyperstone_device::op61()
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 0, 1);
	hyperstone_cmpi(decode);
}

void hyperstone_device::op62()
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 1, 0);
	hyperstone_cmpi(decode);
}

void hyperstone_device::op63()
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 1, 1);
	hyperstone_cmpi(decode);
}

void hyperstone_device::op64()
{
	LOCAL_DECODE_INIT;
	RimmdecodewithHflag(decode, 0, 0);
	hyperstone_movi(decode);
}

void hyperstone_device::op65()
{
	LOCAL_DECODE_INIT;
	RimmdecodewithHflag(decode, 0, 1);
	hyperstone_movi(decode);
}

void hyperstone_device::op66()
{
	LOCAL_DECODE_INIT;
	RimmdecodewithHflag(decode, 1, 0);
	hyperstone_movi(decode);
}

void hyperstone_device::op67()
{
	LOCAL_DECODE_INIT;
	RimmdecodewithHflag(decode, 1, 1);
	hyperstone_movi(decode);
}

void hyperstone_device::op68()
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 0, 0);
	hyperstone_addi(decode);
}

void hyperstone_device::op69()
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 0, 1);
	hyperstone_addi(decode);
}

void hyperstone_device::op6a()
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 1, 0);
	hyperstone_addi(decode);
}

void hyperstone_device::op6b()
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 1, 1);
	hyperstone_addi(decode);
}

void hyperstone_device::op6c()
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 0, 0);
	hyperstone_addsi(decode);
}

void hyperstone_device::op6d()
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 0, 1);
	hyperstone_addsi(decode);
}

void hyperstone_device::op6e()
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 1, 0);
	hyperstone_addsi(decode);
}

void hyperstone_device::op6f()
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 1, 1);
	hyperstone_addsi(decode);
}



void hyperstone_device::op70()
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 0, 0);
	hyperstone_cmpbi(decode);
}

void hyperstone_device::op71()
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 0, 1);
	hyperstone_cmpbi(decode);
}

void hyperstone_device::op72()
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 1, 0);
	hyperstone_cmpbi(decode);
}

void hyperstone_device::op73()
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 1, 1);
	hyperstone_cmpbi(decode);
}

void hyperstone_device::op74()
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 0, 0);
	hyperstone_andni(decode);
}

void hyperstone_device::op75()
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 0, 1);
	hyperstone_andni(decode);
}

void hyperstone_device::op76()
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 1, 0);
	hyperstone_andni(decode);
}

void hyperstone_device::op77()
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 1, 1);
	hyperstone_andni(decode);
}

void hyperstone_device::op78()
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 0, 0);
	hyperstone_ori(decode);
}

void hyperstone_device::op79()
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 0, 1);
	hyperstone_ori(decode);
}

void hyperstone_device::op7a()
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 1, 0);
	hyperstone_ori(decode);
}

void hyperstone_device::op7b()
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 1, 1);
	hyperstone_ori(decode);
}

void hyperstone_device::op7c()
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 0, 0);
	hyperstone_xori(decode);
}

void hyperstone_device::op7d()
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 0, 1);
	hyperstone_xori(decode);
}

void hyperstone_device::op7e()
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 1, 0);
	hyperstone_xori(decode);
}

void hyperstone_device::op7f()
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 1, 1);
	hyperstone_xori(decode);
}



void hyperstone_device::op80()
{
	LOCAL_DECODE_INIT;
	Lndecode(decode);
	hyperstone_shrdi(decode);
}

void hyperstone_device::op81()
{
	LOCAL_DECODE_INIT;
	Lndecode(decode);
	hyperstone_shrdi(decode);
}

void hyperstone_device::op82()
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_shrd(decode);
}

void hyperstone_device::op83()
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_shr(decode);
}

void hyperstone_device::op84()
{
	LOCAL_DECODE_INIT;
	Lndecode(decode);
	hyperstone_sardi(decode);
}

void hyperstone_device::op85()
{
	LOCAL_DECODE_INIT;
	Lndecode(decode);
	hyperstone_sardi(decode);
}

void hyperstone_device::op86()
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_sard(decode);
}

void hyperstone_device::op87()
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_sar(decode);
}

void hyperstone_device::op88()
{
	LOCAL_DECODE_INIT;
	Lndecode(decode);
	hyperstone_shldi(decode);
}

void hyperstone_device::op89()
{
	LOCAL_DECODE_INIT;
	Lndecode(decode);
	hyperstone_shldi(decode);
}

void hyperstone_device::op8a()
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_shld(decode);
}

void hyperstone_device::op8b()
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_shl(decode);
}

void hyperstone_device::op8c()
{
	LOCAL_DECODE_INIT;
	no_decode(decode);
	reserved(decode);
}

void hyperstone_device::op8d()
{
	LOCAL_DECODE_INIT;
	no_decode(decode);
	reserved(decode);
}

void hyperstone_device::op8e()
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_testlz(decode);
}

void hyperstone_device::op8f()
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_rol(decode);
}



void hyperstone_device::op90()
{
	LOCAL_DECODE_INIT;
	RRdisdecode(decode, 0, 0);
	hyperstone_ldxx1(decode);
}

void hyperstone_device::op91()
{
	LOCAL_DECODE_INIT;
	RRdisdecode(decode, 0, 1);
	hyperstone_ldxx1(decode);
}

void hyperstone_device::op92()
{
	LOCAL_DECODE_INIT;
	RRdisdecode(decode, 1, 0);
	hyperstone_ldxx1(decode);
}

void hyperstone_device::op93()
{
	LOCAL_DECODE_INIT;
	RRdisdecode(decode, 1, 1);
	hyperstone_ldxx1(decode);
}

void hyperstone_device::op94()
{
	LOCAL_DECODE_INIT;
	RRdisdecode(decode, 0, 0);
	hyperstone_ldxx2(decode);
}

void hyperstone_device::op95()
{
	LOCAL_DECODE_INIT;
	RRdisdecode(decode, 0, 1);
	hyperstone_ldxx2(decode);
}

void hyperstone_device::op96()
{
	LOCAL_DECODE_INIT;
	RRdisdecode(decode, 1, 0);
	hyperstone_ldxx2(decode);
}

void hyperstone_device::op97()
{
	LOCAL_DECODE_INIT;
	RRdisdecode(decode, 1, 1);
	hyperstone_ldxx2(decode);
}

void hyperstone_device::op98()
{
	LOCAL_DECODE_INIT;
	RRdisdecode(decode, 0, 0);
	hyperstone_stxx1(decode);
}

void hyperstone_device::op99()
{
	LOCAL_DECODE_INIT;
	RRdisdecode(decode, 0, 1);
	hyperstone_stxx1(decode);
}

void hyperstone_device::op9a()
{
	LOCAL_DECODE_INIT;
	RRdisdecode(decode, 1, 0);
	hyperstone_stxx1(decode);
}

void hyperstone_device::op9b()
{
	LOCAL_DECODE_INIT;
	RRdisdecode(decode, 1, 1);
	hyperstone_stxx1(decode);
}

void hyperstone_device::op9c()
{
	LOCAL_DECODE_INIT;
	RRdisdecode(decode, 0, 0);
	hyperstone_stxx2(decode);
}

void hyperstone_device::op9d()
{
	LOCAL_DECODE_INIT;
	RRdisdecode(decode, 0, 1);
	hyperstone_stxx2(decode);
}

void hyperstone_device::op9e()
{
	LOCAL_DECODE_INIT;
	RRdisdecode(decode, 1, 0);
	hyperstone_stxx2(decode);
}

void hyperstone_device::op9f()
{
	LOCAL_DECODE_INIT;
	RRdisdecode(decode, 1, 1);
	hyperstone_stxx2(decode);
}



void hyperstone_device::opa0()
{
	LOCAL_DECODE_INIT;
	Rndecode(decode, 0);
	hyperstone_shri(decode);
}

void hyperstone_device::opa1()
{
	LOCAL_DECODE_INIT;
	Rndecode(decode, 0);
	hyperstone_shri(decode);
}

void hyperstone_device::opa2()
{
	LOCAL_DECODE_INIT;
	Rndecode(decode, 1);
	hyperstone_shri(decode);
}

void hyperstone_device::opa3()
{
	LOCAL_DECODE_INIT;
	Rndecode(decode, 1);
	hyperstone_shri(decode);
}

void hyperstone_device::opa4()
{
	LOCAL_DECODE_INIT;
	Rndecode(decode, 0);
	hyperstone_sari(decode);
}

void hyperstone_device::opa5()
{
	LOCAL_DECODE_INIT;
	Rndecode(decode, 0);
	hyperstone_sari(decode);
}

void hyperstone_device::opa6()
{
	LOCAL_DECODE_INIT;
	Rndecode(decode, 1);
	hyperstone_sari(decode);
}

void hyperstone_device::opa7()
{
	LOCAL_DECODE_INIT;
	Rndecode(decode, 1);
	hyperstone_sari(decode);
}

void hyperstone_device::opa8()
{
	LOCAL_DECODE_INIT;
	Rndecode(decode, 0);
	hyperstone_shli(decode);
}

void hyperstone_device::opa9()
{
	LOCAL_DECODE_INIT;
	Rndecode(decode, 0);
	hyperstone_shli(decode);
}

void hyperstone_device::opaa()
{
	LOCAL_DECODE_INIT;
	Rndecode(decode, 1);
	hyperstone_shli(decode);
}

void hyperstone_device::opab()
{
	LOCAL_DECODE_INIT;
	Rndecode(decode, 1);
	hyperstone_shli(decode);
}

void hyperstone_device::opac()
{
	LOCAL_DECODE_INIT;
	no_decode(decode);
	reserved(decode);
}

void hyperstone_device::opad()
{
	LOCAL_DECODE_INIT;
	no_decode(decode);
	reserved(decode);
}

void hyperstone_device::opae()
{
	LOCAL_DECODE_INIT;
	no_decode(decode);
	reserved(decode);
}

void hyperstone_device::opaf()
{
	LOCAL_DECODE_INIT;
	no_decode(decode);
	reserved(decode);
}



void hyperstone_device::opb0()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_mulu(decode);
}

void hyperstone_device::opb1()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_mulu(decode);
}

void hyperstone_device::opb2()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_mulu(decode);
}

void hyperstone_device::opb3()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_mulu(decode);
}

void hyperstone_device::opb4()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_muls(decode);
}

void hyperstone_device::opb5()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_muls(decode);
}

void hyperstone_device::opb6()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_muls(decode);
}

void hyperstone_device::opb7()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_muls(decode);
}

void hyperstone_device::opb8()
{
	LOCAL_DECODE_INIT;
	Rndecode(decode, 0);
	hyperstone_set(decode);
}

void hyperstone_device::opb9()
{
	LOCAL_DECODE_INIT;
	Rndecode(decode, 0);
	hyperstone_set(decode);
}

void hyperstone_device::opba()
{
	LOCAL_DECODE_INIT;
	Rndecode(decode, 1);
	hyperstone_set(decode);
}

void hyperstone_device::opbb()
{
	LOCAL_DECODE_INIT;
	Rndecode(decode, 1);
	hyperstone_set(decode);
}

void hyperstone_device::opbc()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_mul(decode);
}

void hyperstone_device::opbd()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_mul(decode);
}

void hyperstone_device::opbe()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_mul(decode);
}

void hyperstone_device::opbf()
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_mul(decode);
}



void hyperstone_device::opc0()
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_fadd(decode);
}

void hyperstone_device::opc1()
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_faddd(decode);
}

void hyperstone_device::opc2()
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_fsub(decode);
}

void hyperstone_device::opc3()
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_fsubd(decode);
}

void hyperstone_device::opc4()
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_fmul(decode);
}

void hyperstone_device::opc5()
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_fmuld(decode);
}

void hyperstone_device::opc6()
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_fdiv(decode);
}

void hyperstone_device::opc7()
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_fdivd(decode);
}

void hyperstone_device::opc8()
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_fcmp(decode);
}

void hyperstone_device::opc9()
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_fcmpd(decode);
}

void hyperstone_device::opca()
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_fcmpu(decode);
}

void hyperstone_device::opcb()
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_fcmpud(decode);
}

void hyperstone_device::opcc()
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_fcvt(decode);
}

void hyperstone_device::opcd()
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_fcvtd(decode);
}

void hyperstone_device::opce()
{
	LOCAL_DECODE_INIT;
	LLextdecode(decode);
	hyperstone_extend(decode);
}

void hyperstone_device::opcf()
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_do(decode);
}



void hyperstone_device::opd0()
{
	LOCAL_DECODE_INIT;
	LRdecode(decode, 0);
	hyperstone_ldwr(decode);
}

void hyperstone_device::opd1()
{
	LOCAL_DECODE_INIT;
	LRdecode(decode, 1);
	hyperstone_ldwr(decode);
}

void hyperstone_device::opd2()
{
	LOCAL_DECODE_INIT;
	LRdecode(decode, 0);
	hyperstone_lddr(decode);
}

void hyperstone_device::opd3()
{
	LOCAL_DECODE_INIT;
	LRdecode(decode, 1);
	hyperstone_lddr(decode);
}

void hyperstone_device::opd4()
{
	LOCAL_DECODE_INIT;
	LRdecode(decode, 0);
	hyperstone_ldwp(decode);
}

void hyperstone_device::opd5()
{
	LOCAL_DECODE_INIT;
	LRdecode(decode, 1);
	hyperstone_ldwp(decode);
}

void hyperstone_device::opd6()
{
	LOCAL_DECODE_INIT;
	LRdecode(decode, 0);
	hyperstone_lddp(decode);
}

void hyperstone_device::opd7()
{
	LOCAL_DECODE_INIT;
	LRdecode(decode, 1);
	hyperstone_lddp(decode);
}

void hyperstone_device::opd8()
{
	LOCAL_DECODE_INIT;
	LRdecode(decode, 0);
	hyperstone_stwr(decode);
}

void hyperstone_device::opd9()
{
	LOCAL_DECODE_INIT;
	LRdecode(decode, 1);
	hyperstone_stwr(decode);
}

void hyperstone_device::opda()
{
	LOCAL_DECODE_INIT;
	LRdecode(decode, 0);
	hyperstone_stdr(decode);
}

void hyperstone_device::opdb()
{
	LOCAL_DECODE_INIT;
	LRdecode(decode, 1);
	hyperstone_stdr(decode);
}

void hyperstone_device::opdc()
{
	LOCAL_DECODE_INIT;
	LRdecode(decode, 0);
	hyperstone_stwp(decode);
}

void hyperstone_device::opdd()
{
	LOCAL_DECODE_INIT;
	LRdecode(decode, 1);
	hyperstone_stwp(decode);
}

void hyperstone_device::opde()
{
	LOCAL_DECODE_INIT;
	LRdecode(decode, 0);
	hyperstone_stdp(decode);
}

void hyperstone_device::opdf()
{
	LOCAL_DECODE_INIT;
	LRdecode(decode, 1);
	hyperstone_stdp(decode);
}



void hyperstone_device::ope0()
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_dbv(decode);
}

void hyperstone_device::ope1()
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_dbnv(decode);
}

void hyperstone_device::ope2()
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_dbe(decode);
}

void hyperstone_device::ope3()
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_dbne(decode);
}

void hyperstone_device::ope4()
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_dbc(decode);
}

void hyperstone_device::ope5()
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_dbnc(decode);
}

void hyperstone_device::ope6()
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_dbse(decode);
}

void hyperstone_device::ope7()
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_dbht(decode);
}

void hyperstone_device::ope8()
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_dbn(decode);
}

void hyperstone_device::ope9()
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_dbnn(decode);
}

void hyperstone_device::opea()
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_dble(decode);
}

void hyperstone_device::opeb()
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_dbgt(decode);
}

void hyperstone_device::opec()
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_dbr(decode);
}

void hyperstone_device::oped()
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_frame(decode);
}

void hyperstone_device::opee()
{
	LOCAL_DECODE_INIT;
	LRconstdecode(decode, 0);
	hyperstone_call(decode);
}

void hyperstone_device::opef()
{
	LOCAL_DECODE_INIT;
	LRconstdecode(decode, 1);
	hyperstone_call(decode);
}



void hyperstone_device::opf0()
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_bv(decode);
}

void hyperstone_device::opf1()
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_bnv(decode);
}

void hyperstone_device::opf2()
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_be(decode);
}

void hyperstone_device::opf3()
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_bne(decode);
}

void hyperstone_device::opf4()
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_bc(decode);
}

void hyperstone_device::opf5()
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_bnc(decode);
}

void hyperstone_device::opf6()
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_bse(decode);
}

void hyperstone_device::opf7()
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_bht(decode);
}

void hyperstone_device::opf8()
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_bn(decode);
}

void hyperstone_device::opf9()
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_bnn(decode);
}

void hyperstone_device::opfa()
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_ble(decode);
}

void hyperstone_device::opfb()
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_bgt(decode);
}

void hyperstone_device::opfc()
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_br(decode);
}

void hyperstone_device::opfd()
{
	LOCAL_DECODE_INIT;
	PCadrdecode(decode);
	hyperstone_trap(decode);
}

void hyperstone_device::opfe()
{
	LOCAL_DECODE_INIT;
	PCadrdecode(decode);
	hyperstone_trap(decode);
}

void hyperstone_device::opff()
{
	LOCAL_DECODE_INIT;
	PCadrdecode(decode);
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
