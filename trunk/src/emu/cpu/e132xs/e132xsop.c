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
	decode->same_srcf_dst = 0; \


static void hyperstone_op00(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_chk(cpustate, decode);
}

static void hyperstone_op01(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_chk(cpustate, decode);
}

static void hyperstone_op02(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_chk(cpustate, decode);
}

static void hyperstone_op03(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_chk(cpustate, decode);
}

static void hyperstone_op04(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_movd(cpustate, decode);
}

static void hyperstone_op05(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_movd(cpustate, decode);
}

static void hyperstone_op06(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_movd(cpustate, decode);
}

static void hyperstone_op07(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_movd(cpustate, decode);
}

static void hyperstone_op08(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_divu(cpustate, decode);
}

static void hyperstone_op09(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_divu(cpustate, decode);
}

static void hyperstone_op0a(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_divu(cpustate, decode);
}

static void hyperstone_op0b(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_divu(cpustate, decode);
}

static void hyperstone_op0c(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_divs(cpustate, decode);
}

static void hyperstone_op0d(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_divs(cpustate, decode);
}

static void hyperstone_op0e(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_divs(cpustate, decode);
}

static void hyperstone_op0f(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_divs(cpustate, decode);
}



static void hyperstone_op10(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRlimdecode(decode, 0, 0);
	hyperstone_xm(cpustate, decode);
}

static void hyperstone_op11(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRlimdecode(decode, 0, 1);
	hyperstone_xm(cpustate, decode);
}

static void hyperstone_op12(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRlimdecode(decode, 1, 0);
	hyperstone_xm(cpustate, decode);
}

static void hyperstone_op13(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRlimdecode(decode, 1, 1);
	hyperstone_xm(cpustate, decode);
}

static void hyperstone_op14(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRconstdecode(decode, 0, 0);
	hyperstone_mask(cpustate, decode);
}

static void hyperstone_op15(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRconstdecode(decode, 0, 1);
	hyperstone_mask(cpustate, decode);
}

static void hyperstone_op16(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRconstdecode(decode, 1, 0);
	hyperstone_mask(cpustate, decode);
}

static void hyperstone_op17(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRconstdecode(decode, 1, 1);
	hyperstone_mask(cpustate, decode);
}

static void hyperstone_op18(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRconstdecode(decode, 0, 0);
	hyperstone_sum(cpustate, decode);
}

static void hyperstone_op19(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRconstdecode(decode, 0, 1);
	hyperstone_sum(cpustate, decode);
}

static void hyperstone_op1a(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRconstdecode(decode, 1, 0);
	hyperstone_sum(cpustate, decode);
}

static void hyperstone_op1b(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRconstdecode(decode, 1, 1);
	hyperstone_sum(cpustate, decode);
}

static void hyperstone_op1c(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRconstdecode(decode, 0, 0);
	hyperstone_sums(cpustate, decode);
}

static void hyperstone_op1d(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRconstdecode(decode, 0, 1);
	hyperstone_sums(cpustate, decode);
}

static void hyperstone_op1e(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRconstdecode(decode, 1, 0);
	hyperstone_sums(cpustate, decode);
}

static void hyperstone_op1f(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRconstdecode(decode, 1, 1);
	hyperstone_sums(cpustate, decode);
}



static void hyperstone_op20(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_cmp(cpustate, decode);
}

static void hyperstone_op21(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_cmp(cpustate, decode);
}

static void hyperstone_op22(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_cmp(cpustate, decode);
}

static void hyperstone_op23(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_cmp(cpustate, decode);
}

static void hyperstone_op24(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecodewithHflag(decode, 0, 0);
	hyperstone_mov(cpustate, decode);
}

static void hyperstone_op25(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecodewithHflag(decode, 0, 1);
	hyperstone_mov(cpustate, decode);
}

static void hyperstone_op26(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecodewithHflag(decode, 1, 0);
	hyperstone_mov(cpustate, decode);
}

static void hyperstone_op27(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecodewithHflag(decode, 1, 1);
	hyperstone_mov(cpustate, decode);
}

static void hyperstone_op28(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_add(cpustate, decode);
}

static void hyperstone_op29(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_add(cpustate, decode);
}

static void hyperstone_op2a(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_add(cpustate, decode);
}

static void hyperstone_op2b(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_add(cpustate, decode);
}

static void hyperstone_op2c(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_adds(cpustate, decode);
}

static void hyperstone_op2d(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_adds(cpustate, decode);
}

static void hyperstone_op2e(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_adds(cpustate, decode);
}

static void hyperstone_op2f(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_adds(cpustate, decode);
}



static void hyperstone_op30(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_cmpb(cpustate, decode);
}

static void hyperstone_op31(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_cmpb(cpustate, decode);
}

static void hyperstone_op32(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_cmpb(cpustate, decode);
}

static void hyperstone_op33(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_cmpb(cpustate, decode);
}

static void hyperstone_op34(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_andn(cpustate, decode);
}

static void hyperstone_op35(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_andn(cpustate, decode);
}

static void hyperstone_op36(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_andn(cpustate, decode);
}

static void hyperstone_op37(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_andn(cpustate, decode);
}

static void hyperstone_op38(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_or(cpustate, decode);
}

static void hyperstone_op39(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_or(cpustate, decode);
}

static void hyperstone_op3a(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_or(cpustate, decode);
}

static void hyperstone_op3b(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_or(cpustate, decode);
}

static void hyperstone_op3c(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_xor(cpustate, decode);
}

static void hyperstone_op3d(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_xor(cpustate, decode);
}

static void hyperstone_op3e(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_xor(cpustate, decode);
}

static void hyperstone_op3f(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_xor(cpustate, decode);
}



static void hyperstone_op40(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_subc(cpustate, decode);
}

static void hyperstone_op41(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_subc(cpustate, decode);
}

static void hyperstone_op42(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_subc(cpustate, decode);
}

static void hyperstone_op43(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_subc(cpustate, decode);
}

static void hyperstone_op44(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_not(cpustate, decode);
}

static void hyperstone_op45(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_not(cpustate, decode);
}

static void hyperstone_op46(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_not(cpustate, decode);
}

static void hyperstone_op47(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_not(cpustate, decode);
}

static void hyperstone_op48(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_sub(cpustate, decode);
}

static void hyperstone_op49(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_sub(cpustate, decode);
}

static void hyperstone_op4a(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_sub(cpustate, decode);
}

static void hyperstone_op4b(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_sub(cpustate, decode);
}

static void hyperstone_op4c(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_subs(cpustate, decode);
}

static void hyperstone_op4d(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_subs(cpustate, decode);
}

static void hyperstone_op4e(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_subs(cpustate, decode);
}

static void hyperstone_op4f(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_subs(cpustate, decode);
}



static void hyperstone_op50(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_addc(cpustate, decode);
}

static void hyperstone_op51(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_addc(cpustate, decode);
}

static void hyperstone_op52(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_addc(cpustate, decode);
}

static void hyperstone_op53(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_addc(cpustate, decode);
}

static void hyperstone_op54(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_and(cpustate, decode);
}

static void hyperstone_op55(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_and(cpustate, decode);
}

static void hyperstone_op56(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_and(cpustate, decode);
}

static void hyperstone_op57(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_and(cpustate, decode);
}

static void hyperstone_op58(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_neg(cpustate, decode);
}

static void hyperstone_op59(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_neg(cpustate, decode);
}

static void hyperstone_op5a(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_neg(cpustate, decode);
}

static void hyperstone_op5b(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_neg(cpustate, decode);
}

static void hyperstone_op5c(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_negs(cpustate, decode);
}

static void hyperstone_op5d(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_negs(cpustate, decode);
}

static void hyperstone_op5e(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_negs(cpustate, decode);
}

static void hyperstone_op5f(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_negs(cpustate, decode);
}



static void hyperstone_op60(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 0, 0);
	hyperstone_cmpi(cpustate, decode);
}

static void hyperstone_op61(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 0, 1);
	hyperstone_cmpi(cpustate, decode);
}

static void hyperstone_op62(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 1, 0);
	hyperstone_cmpi(cpustate, decode);
}

static void hyperstone_op63(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 1, 1);
	hyperstone_cmpi(cpustate, decode);
}

static void hyperstone_op64(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RimmdecodewithHflag(decode, 0, 0);
	hyperstone_movi(cpustate, decode);
}

static void hyperstone_op65(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RimmdecodewithHflag(decode, 0, 1);
	hyperstone_movi(cpustate, decode);
}

static void hyperstone_op66(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RimmdecodewithHflag(decode, 1, 0);
	hyperstone_movi(cpustate, decode);
}

static void hyperstone_op67(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RimmdecodewithHflag(decode, 1, 1);
	hyperstone_movi(cpustate, decode);
}

static void hyperstone_op68(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 0, 0);
	hyperstone_addi(cpustate, decode);
}

static void hyperstone_op69(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 0, 1);
	hyperstone_addi(cpustate, decode);
}

static void hyperstone_op6a(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 1, 0);
	hyperstone_addi(cpustate, decode);
}

static void hyperstone_op6b(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 1, 1);
	hyperstone_addi(cpustate, decode);
}

static void hyperstone_op6c(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 0, 0);
	hyperstone_addsi(cpustate, decode);
}

static void hyperstone_op6d(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 0, 1);
	hyperstone_addsi(cpustate, decode);
}

static void hyperstone_op6e(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 1, 0);
	hyperstone_addsi(cpustate, decode);
}

static void hyperstone_op6f(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 1, 1);
	hyperstone_addsi(cpustate, decode);
}



static void hyperstone_op70(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 0, 0);
	hyperstone_cmpbi(cpustate, decode);
}

static void hyperstone_op71(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 0, 1);
	hyperstone_cmpbi(cpustate, decode);
}

static void hyperstone_op72(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 1, 0);
	hyperstone_cmpbi(cpustate, decode);
}

static void hyperstone_op73(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 1, 1);
	hyperstone_cmpbi(cpustate, decode);
}

static void hyperstone_op74(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 0, 0);
	hyperstone_andni(cpustate, decode);
}

static void hyperstone_op75(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 0, 1);
	hyperstone_andni(cpustate, decode);
}

static void hyperstone_op76(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 1, 0);
	hyperstone_andni(cpustate, decode);
}

static void hyperstone_op77(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 1, 1);
	hyperstone_andni(cpustate, decode);
}

static void hyperstone_op78(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 0, 0);
	hyperstone_ori(cpustate, decode);
}

static void hyperstone_op79(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 0, 1);
	hyperstone_ori(cpustate, decode);
}

static void hyperstone_op7a(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 1, 0);
	hyperstone_ori(cpustate, decode);
}

static void hyperstone_op7b(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 1, 1);
	hyperstone_ori(cpustate, decode);
}

static void hyperstone_op7c(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 0, 0);
	hyperstone_xori(cpustate, decode);
}

static void hyperstone_op7d(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 0, 1);
	hyperstone_xori(cpustate, decode);
}

static void hyperstone_op7e(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 1, 0);
	hyperstone_xori(cpustate, decode);
}

static void hyperstone_op7f(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 1, 1);
	hyperstone_xori(cpustate, decode);
}



static void hyperstone_op80(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	Lndecode(decode);
	hyperstone_shrdi(cpustate, decode);
}

static void hyperstone_op81(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	Lndecode(decode);
	hyperstone_shrdi(cpustate, decode);
}

static void hyperstone_op82(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_shrd(cpustate, decode);
}

static void hyperstone_op83(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_shr(cpustate, decode);
}

static void hyperstone_op84(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	Lndecode(decode);
	hyperstone_sardi(cpustate, decode);
}

static void hyperstone_op85(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	Lndecode(decode);
	hyperstone_sardi(cpustate, decode);
}

static void hyperstone_op86(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_sard(cpustate, decode);
}

static void hyperstone_op87(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_sar(cpustate, decode);
}

static void hyperstone_op88(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	Lndecode(decode);
	hyperstone_shldi(cpustate, decode);
}

static void hyperstone_op89(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	Lndecode(decode);
	hyperstone_shldi(cpustate, decode);
}

static void hyperstone_op8a(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_shld(cpustate, decode);
}

static void hyperstone_op8b(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_shl(cpustate, decode);
}

static void hyperstone_op8c(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	no_decode(decode);
	reserved(cpustate, decode);
}

static void hyperstone_op8d(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	no_decode(decode);
	reserved(cpustate, decode);
}

static void hyperstone_op8e(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_testlz(cpustate, decode);
}

static void hyperstone_op8f(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_rol(cpustate, decode);
}



static void hyperstone_op90(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdisdecode(decode, 0, 0);
	hyperstone_ldxx1(cpustate, decode);
}

static void hyperstone_op91(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdisdecode(decode, 0, 1);
	hyperstone_ldxx1(cpustate, decode);
}

static void hyperstone_op92(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdisdecode(decode, 1, 0);
	hyperstone_ldxx1(cpustate, decode);
}

static void hyperstone_op93(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdisdecode(decode, 1, 1);
	hyperstone_ldxx1(cpustate, decode);
}

static void hyperstone_op94(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdisdecode(decode, 0, 0);
	hyperstone_ldxx2(cpustate, decode);
}

static void hyperstone_op95(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdisdecode(decode, 0, 1);
	hyperstone_ldxx2(cpustate, decode);
}

static void hyperstone_op96(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdisdecode(decode, 1, 0);
	hyperstone_ldxx2(cpustate, decode);
}

static void hyperstone_op97(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdisdecode(decode, 1, 1);
	hyperstone_ldxx2(cpustate, decode);
}

static void hyperstone_op98(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdisdecode(decode, 0, 0);
	hyperstone_stxx1(cpustate, decode);
}

static void hyperstone_op99(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdisdecode(decode, 0, 1);
	hyperstone_stxx1(cpustate, decode);
}

static void hyperstone_op9a(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdisdecode(decode, 1, 0);
	hyperstone_stxx1(cpustate, decode);
}

static void hyperstone_op9b(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdisdecode(decode, 1, 1);
	hyperstone_stxx1(cpustate, decode);
}

static void hyperstone_op9c(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdisdecode(decode, 0, 0);
	hyperstone_stxx2(cpustate, decode);
}

static void hyperstone_op9d(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdisdecode(decode, 0, 1);
	hyperstone_stxx2(cpustate, decode);
}

static void hyperstone_op9e(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdisdecode(decode, 1, 0);
	hyperstone_stxx2(cpustate, decode);
}

static void hyperstone_op9f(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdisdecode(decode, 1, 1);
	hyperstone_stxx2(cpustate, decode);
}



static void hyperstone_opa0(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	Rndecode(decode, 0);
	hyperstone_shri(cpustate, decode);
}

static void hyperstone_opa1(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	Rndecode(decode, 0);
	hyperstone_shri(cpustate, decode);
}

static void hyperstone_opa2(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	Rndecode(decode, 1);
	hyperstone_shri(cpustate, decode);
}

static void hyperstone_opa3(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	Rndecode(decode, 1);
	hyperstone_shri(cpustate, decode);
}

static void hyperstone_opa4(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	Rndecode(decode, 0);
	hyperstone_sari(cpustate, decode);
}

static void hyperstone_opa5(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	Rndecode(decode, 0);
	hyperstone_sari(cpustate, decode);
}

static void hyperstone_opa6(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	Rndecode(decode, 1);
	hyperstone_sari(cpustate, decode);
}

static void hyperstone_opa7(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	Rndecode(decode, 1);
	hyperstone_sari(cpustate, decode);
}

static void hyperstone_opa8(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	Rndecode(decode, 0);
	hyperstone_shli(cpustate, decode);
}

static void hyperstone_opa9(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	Rndecode(decode, 0);
	hyperstone_shli(cpustate, decode);
}

static void hyperstone_opaa(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	Rndecode(decode, 1);
	hyperstone_shli(cpustate, decode);
}

static void hyperstone_opab(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	Rndecode(decode, 1);
	hyperstone_shli(cpustate, decode);
}

static void hyperstone_opac(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	no_decode(decode);
	reserved(cpustate, decode);
}

static void hyperstone_opad(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	no_decode(decode);
	reserved(cpustate, decode);
}

static void hyperstone_opae(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	no_decode(decode);
	reserved(cpustate, decode);
}

static void hyperstone_opaf(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	no_decode(decode);
	reserved(cpustate, decode);
}



static void hyperstone_opb0(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_mulu(cpustate, decode);
}

static void hyperstone_opb1(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_mulu(cpustate, decode);
}

static void hyperstone_opb2(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_mulu(cpustate, decode);
}

static void hyperstone_opb3(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_mulu(cpustate, decode);
}

static void hyperstone_opb4(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_muls(cpustate, decode);
}

static void hyperstone_opb5(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_muls(cpustate, decode);
}

static void hyperstone_opb6(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_muls(cpustate, decode);
}

static void hyperstone_opb7(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_muls(cpustate, decode);
}

static void hyperstone_opb8(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	Rndecode(decode, 0);
	hyperstone_set(cpustate, decode);
}

static void hyperstone_opb9(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	Rndecode(decode, 0);
	hyperstone_set(cpustate, decode);
}

static void hyperstone_opba(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	Rndecode(decode, 1);
	hyperstone_set(cpustate, decode);
}

static void hyperstone_opbb(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	Rndecode(decode, 1);
	hyperstone_set(cpustate, decode);
}

static void hyperstone_opbc(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_mul(cpustate, decode);
}

static void hyperstone_opbd(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_mul(cpustate, decode);
}

static void hyperstone_opbe(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_mul(cpustate, decode);
}

static void hyperstone_opbf(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_mul(cpustate, decode);
}



static void hyperstone_opc0(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_fadd(cpustate, decode);
}

static void hyperstone_opc1(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_faddd(cpustate, decode);
}

static void hyperstone_opc2(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_fsub(cpustate, decode);
}

static void hyperstone_opc3(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_fsubd(cpustate, decode);
}

static void hyperstone_opc4(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_fmul(cpustate, decode);
}

static void hyperstone_opc5(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_fmuld(cpustate, decode);
}

static void hyperstone_opc6(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_fdiv(cpustate, decode);
}

static void hyperstone_opc7(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_fdivd(cpustate, decode);
}

static void hyperstone_opc8(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_fcmp(cpustate, decode);
}

static void hyperstone_opc9(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_fcmpd(cpustate, decode);
}

static void hyperstone_opca(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_fcmpu(cpustate, decode);
}

static void hyperstone_opcb(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_fcmpud(cpustate, decode);
}

static void hyperstone_opcc(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_fcvt(cpustate, decode);
}

static void hyperstone_opcd(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_fcvtd(cpustate, decode);
}

static void hyperstone_opce(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	LLextdecode(decode);
	hyperstone_extend(cpustate, decode);
}

static void hyperstone_opcf(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_do(cpustate, decode);
}



static void hyperstone_opd0(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	LRdecode(decode, 0);
	hyperstone_ldwr(cpustate, decode);
}

static void hyperstone_opd1(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	LRdecode(decode, 1);
	hyperstone_ldwr(cpustate, decode);
}

static void hyperstone_opd2(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	LRdecode(decode, 0);
	hyperstone_lddr(cpustate, decode);
}

static void hyperstone_opd3(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	LRdecode(decode, 1);
	hyperstone_lddr(cpustate, decode);
}

static void hyperstone_opd4(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	LRdecode(decode, 0);
	hyperstone_ldwp(cpustate, decode);
}

static void hyperstone_opd5(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	LRdecode(decode, 1);
	hyperstone_ldwp(cpustate, decode);
}

static void hyperstone_opd6(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	LRdecode(decode, 0);
	hyperstone_lddp(cpustate, decode);
}

static void hyperstone_opd7(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	LRdecode(decode, 1);
	hyperstone_lddp(cpustate, decode);
}

static void hyperstone_opd8(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	LRdecode(decode, 0);
	hyperstone_stwr(cpustate, decode);
}

static void hyperstone_opd9(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	LRdecode(decode, 1);
	hyperstone_stwr(cpustate, decode);
}

static void hyperstone_opda(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	LRdecode(decode, 0);
	hyperstone_stdr(cpustate, decode);
}

static void hyperstone_opdb(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	LRdecode(decode, 1);
	hyperstone_stdr(cpustate, decode);
}

static void hyperstone_opdc(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	LRdecode(decode, 0);
	hyperstone_stwp(cpustate, decode);
}

static void hyperstone_opdd(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	LRdecode(decode, 1);
	hyperstone_stwp(cpustate, decode);
}

static void hyperstone_opde(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	LRdecode(decode, 0);
	hyperstone_stdp(cpustate, decode);
}

static void hyperstone_opdf(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	LRdecode(decode, 1);
	hyperstone_stdp(cpustate, decode);
}



static void hyperstone_ope0(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_dbv(cpustate, decode);
}

static void hyperstone_ope1(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_dbnv(cpustate, decode);
}

static void hyperstone_ope2(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_dbe(cpustate, decode);
}

static void hyperstone_ope3(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_dbne(cpustate, decode);
}

static void hyperstone_ope4(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_dbc(cpustate, decode);
}

static void hyperstone_ope5(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_dbnc(cpustate, decode);
}

static void hyperstone_ope6(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_dbse(cpustate, decode);
}

static void hyperstone_ope7(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_dbht(cpustate, decode);
}

static void hyperstone_ope8(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_dbn(cpustate, decode);
}

static void hyperstone_ope9(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_dbnn(cpustate, decode);
}

static void hyperstone_opea(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_dble(cpustate, decode);
}

static void hyperstone_opeb(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_dbgt(cpustate, decode);
}

static void hyperstone_opec(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_dbr(cpustate, decode);
}

static void hyperstone_oped(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_frame(cpustate, decode);
}

static void hyperstone_opee(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	LRconstdecode(decode, 0);
	hyperstone_call(cpustate, decode);
}

static void hyperstone_opef(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	LRconstdecode(decode, 1);
	hyperstone_call(cpustate, decode);
}



static void hyperstone_opf0(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_bv(cpustate, decode);
}

static void hyperstone_opf1(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_bnv(cpustate, decode);
}

static void hyperstone_opf2(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_be(cpustate, decode);
}

static void hyperstone_opf3(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_bne(cpustate, decode);
}

static void hyperstone_opf4(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_bc(cpustate, decode);
}

static void hyperstone_opf5(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_bnc(cpustate, decode);
}

static void hyperstone_opf6(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_bse(cpustate, decode);
}

static void hyperstone_opf7(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_bht(cpustate, decode);
}

static void hyperstone_opf8(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_bn(cpustate, decode);
}

static void hyperstone_opf9(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_bnn(cpustate, decode);
}

static void hyperstone_opfa(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_ble(cpustate, decode);
}

static void hyperstone_opfb(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_bgt(cpustate, decode);
}

static void hyperstone_opfc(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_br(cpustate, decode);
}

static void hyperstone_opfd(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	PCadrdecode(decode);
	hyperstone_trap(cpustate, decode);
}

static void hyperstone_opfe(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	PCadrdecode(decode);
	hyperstone_trap(cpustate, decode);
}

static void hyperstone_opff(hyperstone_state *cpustate)
{
	LOCAL_DECODE_INIT;
	PCadrdecode(decode);
	hyperstone_trap(cpustate, decode);
}


static void (*const hyperstone_op[0x100])(hyperstone_state *cpustate) =
{
	hyperstone_op00, hyperstone_op01, hyperstone_op02, hyperstone_op03,
	hyperstone_op04, hyperstone_op05, hyperstone_op06, hyperstone_op07,
	hyperstone_op08, hyperstone_op09, hyperstone_op0a, hyperstone_op0b,
	hyperstone_op0c, hyperstone_op0d, hyperstone_op0e, hyperstone_op0f,

	hyperstone_op10, hyperstone_op11, hyperstone_op12, hyperstone_op13,
	hyperstone_op14, hyperstone_op15, hyperstone_op16, hyperstone_op17,
	hyperstone_op18, hyperstone_op19, hyperstone_op1a, hyperstone_op1b,
	hyperstone_op1c, hyperstone_op1d, hyperstone_op1e, hyperstone_op1f,

	hyperstone_op20, hyperstone_op21, hyperstone_op22, hyperstone_op23,
	hyperstone_op24, hyperstone_op25, hyperstone_op26, hyperstone_op27,
	hyperstone_op28, hyperstone_op29, hyperstone_op2a, hyperstone_op2b,
	hyperstone_op2c, hyperstone_op2d, hyperstone_op2e, hyperstone_op2f,

	hyperstone_op30, hyperstone_op31, hyperstone_op32, hyperstone_op33,
	hyperstone_op34, hyperstone_op35, hyperstone_op36, hyperstone_op37,
	hyperstone_op38, hyperstone_op39, hyperstone_op3a, hyperstone_op3b,
	hyperstone_op3c, hyperstone_op3d, hyperstone_op3e, hyperstone_op3f,

	hyperstone_op40, hyperstone_op41, hyperstone_op42, hyperstone_op43,
	hyperstone_op44, hyperstone_op45, hyperstone_op46, hyperstone_op47,
	hyperstone_op48, hyperstone_op49, hyperstone_op4a, hyperstone_op4b,
	hyperstone_op4c, hyperstone_op4d, hyperstone_op4e, hyperstone_op4f,

	hyperstone_op50, hyperstone_op51, hyperstone_op52, hyperstone_op53,
	hyperstone_op54, hyperstone_op55, hyperstone_op56, hyperstone_op57,
	hyperstone_op58, hyperstone_op59, hyperstone_op5a, hyperstone_op5b,
	hyperstone_op5c, hyperstone_op5d, hyperstone_op5e, hyperstone_op5f,

	hyperstone_op60, hyperstone_op61, hyperstone_op62, hyperstone_op63,
	hyperstone_op64, hyperstone_op65, hyperstone_op66, hyperstone_op67,
	hyperstone_op68, hyperstone_op69, hyperstone_op6a, hyperstone_op6b,
	hyperstone_op6c, hyperstone_op6d, hyperstone_op6e, hyperstone_op6f,

	hyperstone_op70, hyperstone_op71, hyperstone_op72, hyperstone_op73,
	hyperstone_op74, hyperstone_op75, hyperstone_op76, hyperstone_op77,
	hyperstone_op78, hyperstone_op79, hyperstone_op7a, hyperstone_op7b,
	hyperstone_op7c, hyperstone_op7d, hyperstone_op7e, hyperstone_op7f,

	hyperstone_op80, hyperstone_op81, hyperstone_op82, hyperstone_op83,
	hyperstone_op84, hyperstone_op85, hyperstone_op86, hyperstone_op87,
	hyperstone_op88, hyperstone_op89, hyperstone_op8a, hyperstone_op8b,
	hyperstone_op8c, hyperstone_op8d, hyperstone_op8e, hyperstone_op8f,

	hyperstone_op90, hyperstone_op91, hyperstone_op92, hyperstone_op93,
	hyperstone_op94, hyperstone_op95, hyperstone_op96, hyperstone_op97,
	hyperstone_op98, hyperstone_op99, hyperstone_op9a, hyperstone_op9b,
	hyperstone_op9c, hyperstone_op9d, hyperstone_op9e, hyperstone_op9f,

	hyperstone_opa0, hyperstone_opa1, hyperstone_opa2, hyperstone_opa3,
	hyperstone_opa4, hyperstone_opa5, hyperstone_opa6, hyperstone_opa7,
	hyperstone_opa8, hyperstone_opa9, hyperstone_opaa, hyperstone_opab,
	hyperstone_opac, hyperstone_opad, hyperstone_opae, hyperstone_opaf,

	hyperstone_opb0, hyperstone_opb1, hyperstone_opb2, hyperstone_opb3,
	hyperstone_opb4, hyperstone_opb5, hyperstone_opb6, hyperstone_opb7,
	hyperstone_opb8, hyperstone_opb9, hyperstone_opba, hyperstone_opbb,
	hyperstone_opbc, hyperstone_opbd, hyperstone_opbe, hyperstone_opbf,

	hyperstone_opc0, hyperstone_opc1, hyperstone_opc2, hyperstone_opc3,
	hyperstone_opc4, hyperstone_opc5, hyperstone_opc6, hyperstone_opc7,
	hyperstone_opc8, hyperstone_opc9, hyperstone_opca, hyperstone_opcb,
	hyperstone_opcc, hyperstone_opcd, hyperstone_opce, hyperstone_opcf,

	hyperstone_opd0, hyperstone_opd1, hyperstone_opd2, hyperstone_opd3,
	hyperstone_opd4, hyperstone_opd5, hyperstone_opd6, hyperstone_opd7,
	hyperstone_opd8, hyperstone_opd9, hyperstone_opda, hyperstone_opdb,
	hyperstone_opdc, hyperstone_opdd, hyperstone_opde, hyperstone_opdf,

	hyperstone_ope0, hyperstone_ope1, hyperstone_ope2, hyperstone_ope3,
	hyperstone_ope4, hyperstone_ope5, hyperstone_ope6, hyperstone_ope7,
	hyperstone_ope8, hyperstone_ope9, hyperstone_opea, hyperstone_opeb,
	hyperstone_opec, hyperstone_oped, hyperstone_opee, hyperstone_opef,

	hyperstone_opf0, hyperstone_opf1, hyperstone_opf2, hyperstone_opf3,
	hyperstone_opf4, hyperstone_opf5, hyperstone_opf6, hyperstone_opf7,
	hyperstone_opf8, hyperstone_opf9, hyperstone_opfa, hyperstone_opfb,
	hyperstone_opfc, hyperstone_opfd, hyperstone_opfe, hyperstone_opff
};
