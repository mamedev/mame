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
	decode->op = opcode; \


static void hyperstone_op00(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_chk(decode);
}

static void hyperstone_op01(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_chk(decode);
}

static void hyperstone_op02(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_chk(decode);
}

static void hyperstone_op03(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_chk(decode);
}

static void hyperstone_op04(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_movd(decode);
}

static void hyperstone_op05(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_movd(decode);
}

static void hyperstone_op06(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_movd(decode);
}

static void hyperstone_op07(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_movd(decode);
}

static void hyperstone_op08(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_divu(decode);
}

static void hyperstone_op09(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_divu(decode);
}

static void hyperstone_op0a(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_divu(decode);
}

static void hyperstone_op0b(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_divu(decode);
}

static void hyperstone_op0c(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_divs(decode);
}

static void hyperstone_op0d(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_divs(decode);
}

static void hyperstone_op0e(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_divs(decode);
}

static void hyperstone_op0f(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_divs(decode);
}



static void hyperstone_op10(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRlimdecode(decode, 0, 0);
	hyperstone_xm(decode);
}

static void hyperstone_op11(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRlimdecode(decode, 0, 1);
	hyperstone_xm(decode);
}

static void hyperstone_op12(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRlimdecode(decode, 1, 0);
	hyperstone_xm(decode);
}

static void hyperstone_op13(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRlimdecode(decode, 1, 1);
	hyperstone_xm(decode);
}

static void hyperstone_op14(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRconstdecode(decode, 0, 0);
	hyperstone_mask(decode);
}

static void hyperstone_op15(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRconstdecode(decode, 0, 1);
	hyperstone_mask(decode);
}

static void hyperstone_op16(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRconstdecode(decode, 1, 0);
	hyperstone_mask(decode);
}

static void hyperstone_op17(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRconstdecode(decode, 1, 1);
	hyperstone_mask(decode);
}

static void hyperstone_op18(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRconstdecode(decode, 0, 0);
	hyperstone_sum(decode);
}

static void hyperstone_op19(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRconstdecode(decode, 0, 1);
	hyperstone_sum(decode);
}

static void hyperstone_op1a(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRconstdecode(decode, 1, 0);
	hyperstone_sum(decode);
}

static void hyperstone_op1b(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRconstdecode(decode, 1, 1);
	hyperstone_sum(decode);
}

static void hyperstone_op1c(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRconstdecode(decode, 0, 0);
	hyperstone_sums(decode);
}

static void hyperstone_op1d(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRconstdecode(decode, 0, 1);
	hyperstone_sums(decode);
}

static void hyperstone_op1e(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRconstdecode(decode, 1, 0);
	hyperstone_sums(decode);
}

static void hyperstone_op1f(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRconstdecode(decode, 1, 1);
	hyperstone_sums(decode);
}



static void hyperstone_op20(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_cmp(decode);
}

static void hyperstone_op21(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_cmp(decode);
}

static void hyperstone_op22(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_cmp(decode);
}

static void hyperstone_op23(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_cmp(decode);
}

static void hyperstone_op24(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecodewithHflag(decode, 0, 0);
	hyperstone_mov(decode);
}

static void hyperstone_op25(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecodewithHflag(decode, 0, 1);
	hyperstone_mov(decode);
}

static void hyperstone_op26(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecodewithHflag(decode, 1, 0);
	hyperstone_mov(decode);
}

static void hyperstone_op27(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecodewithHflag(decode, 1, 1);
	hyperstone_mov(decode);
}

static void hyperstone_op28(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_add(decode);
}

static void hyperstone_op29(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_add(decode);
}

static void hyperstone_op2a(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_add(decode);
}

static void hyperstone_op2b(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_add(decode);
}

static void hyperstone_op2c(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_adds(decode);
}

static void hyperstone_op2d(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_adds(decode);
}

static void hyperstone_op2e(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_adds(decode);
}

static void hyperstone_op2f(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_adds(decode);
}



static void hyperstone_op30(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_cmpb(decode);
}

static void hyperstone_op31(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_cmpb(decode);
}

static void hyperstone_op32(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_cmpb(decode);
}

static void hyperstone_op33(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_cmpb(decode);
}

static void hyperstone_op34(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_andn(decode);
}

static void hyperstone_op35(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_andn(decode);
}

static void hyperstone_op36(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_andn(decode);
}

static void hyperstone_op37(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_andn(decode);
}

static void hyperstone_op38(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_or(decode);
}

static void hyperstone_op39(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_or(decode);
}

static void hyperstone_op3a(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_or(decode);
}

static void hyperstone_op3b(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_or(decode);
}

static void hyperstone_op3c(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_xor(decode);
}

static void hyperstone_op3d(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_xor(decode);
}

static void hyperstone_op3e(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_xor(decode);
}

static void hyperstone_op3f(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_xor(decode);
}



static void hyperstone_op40(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_subc(decode);
}

static void hyperstone_op41(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_subc(decode);
}

static void hyperstone_op42(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_subc(decode);
}

static void hyperstone_op43(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_subc(decode);
}

static void hyperstone_op44(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_not(decode);
}

static void hyperstone_op45(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_not(decode);
}

static void hyperstone_op46(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_not(decode);
}

static void hyperstone_op47(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_not(decode);
}

static void hyperstone_op48(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_sub(decode);
}

static void hyperstone_op49(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_sub(decode);
}

static void hyperstone_op4a(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_sub(decode);
}

static void hyperstone_op4b(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_sub(decode);
}

static void hyperstone_op4c(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_subs(decode);
}

static void hyperstone_op4d(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_subs(decode);
}

static void hyperstone_op4e(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_subs(decode);
}

static void hyperstone_op4f(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_subs(decode);
}



static void hyperstone_op50(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_addc(decode);
}

static void hyperstone_op51(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_addc(decode);
}

static void hyperstone_op52(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_addc(decode);
}

static void hyperstone_op53(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_addc(decode);
}

static void hyperstone_op54(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_and(decode);
}

static void hyperstone_op55(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_and(decode);
}

static void hyperstone_op56(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_and(decode);
}

static void hyperstone_op57(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_and(decode);
}

static void hyperstone_op58(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_neg(decode);
}

static void hyperstone_op59(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_neg(decode);
}

static void hyperstone_op5a(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_neg(decode);
}

static void hyperstone_op5b(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_neg(decode);
}

static void hyperstone_op5c(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_negs(decode);
}

static void hyperstone_op5d(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_negs(decode);
}

static void hyperstone_op5e(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_negs(decode);
}

static void hyperstone_op5f(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_negs(decode);
}



static void hyperstone_op60(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 0, 0);
	hyperstone_cmpi(decode);
}

static void hyperstone_op61(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 0, 1);
	hyperstone_cmpi(decode);
}

static void hyperstone_op62(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 1, 0);
	hyperstone_cmpi(decode);
}

static void hyperstone_op63(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 1, 1);
	hyperstone_cmpi(decode);
}

static void hyperstone_op64(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RimmdecodewithHflag(decode, 0, 0);
	hyperstone_movi(decode);
}

static void hyperstone_op65(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RimmdecodewithHflag(decode, 0, 1);
	hyperstone_movi(decode);
}

static void hyperstone_op66(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RimmdecodewithHflag(decode, 1, 0);
	hyperstone_movi(decode);
}

static void hyperstone_op67(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RimmdecodewithHflag(decode, 1, 1);
	hyperstone_movi(decode);
}

static void hyperstone_op68(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 0, 0);
	hyperstone_addi(decode);
}

static void hyperstone_op69(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 0, 1);
	hyperstone_addi(decode);
}

static void hyperstone_op6a(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 1, 0);
	hyperstone_addi(decode);
}

static void hyperstone_op6b(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 1, 1);
	hyperstone_addi(decode);
}

static void hyperstone_op6c(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 0, 0);
	hyperstone_addsi(decode);
}

static void hyperstone_op6d(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 0, 1);
	hyperstone_addsi(decode);
}

static void hyperstone_op6e(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 1, 0);
	hyperstone_addsi(decode);
}

static void hyperstone_op6f(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 1, 1);
	hyperstone_addsi(decode);
}



static void hyperstone_op70(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 0, 0);
	hyperstone_cmpbi(decode);
}

static void hyperstone_op71(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 0, 1);
	hyperstone_cmpbi(decode);
}

static void hyperstone_op72(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 1, 0);
	hyperstone_cmpbi(decode);
}

static void hyperstone_op73(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 1, 1);
	hyperstone_cmpbi(decode);
}

static void hyperstone_op74(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 0, 0);
	hyperstone_andni(decode);
}

static void hyperstone_op75(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 0, 1);
	hyperstone_andni(decode);
}

static void hyperstone_op76(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 1, 0);
	hyperstone_andni(decode);
}

static void hyperstone_op77(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 1, 1);
	hyperstone_andni(decode);
}

static void hyperstone_op78(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 0, 0);
	hyperstone_ori(decode);
}

static void hyperstone_op79(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 0, 1);
	hyperstone_ori(decode);
}

static void hyperstone_op7a(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 1, 0);
	hyperstone_ori(decode);
}

static void hyperstone_op7b(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 1, 1);
	hyperstone_ori(decode);
}

static void hyperstone_op7c(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 0, 0);
	hyperstone_xori(decode);
}

static void hyperstone_op7d(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 0, 1);
	hyperstone_xori(decode);
}

static void hyperstone_op7e(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 1, 0);
	hyperstone_xori(decode);
}

static void hyperstone_op7f(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	Rimmdecode(decode, 1, 1);
	hyperstone_xori(decode);
}



static void hyperstone_op80(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	Lndecode(decode);
	hyperstone_shrdi(decode);
}

static void hyperstone_op81(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	Lndecode(decode);
	hyperstone_shrdi(decode);
}

static void hyperstone_op82(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_shrd(decode);
}

static void hyperstone_op83(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_shr(decode);
}

static void hyperstone_op84(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	Lndecode(decode);
	hyperstone_sardi(decode);
}

static void hyperstone_op85(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	Lndecode(decode);
	hyperstone_sardi(decode);
}

static void hyperstone_op86(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_sard(decode);
}

static void hyperstone_op87(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_sar(decode);
}

static void hyperstone_op88(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	Lndecode(decode);
	hyperstone_shldi(decode);
}

static void hyperstone_op89(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	Lndecode(decode);
	hyperstone_shldi(decode);
}

static void hyperstone_op8a(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_shld(decode);
}

static void hyperstone_op8b(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_shl(decode);
}

static void hyperstone_op8c(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	no_decode(decode);
	reserved(decode);
}

static void hyperstone_op8d(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	no_decode(decode);
	reserved(decode);
}

static void hyperstone_op8e(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_testlz(decode);
}

static void hyperstone_op8f(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_rol(decode);
}



static void hyperstone_op90(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdisdecode(decode, 0, 0);
	hyperstone_ldxx1(decode);
}

static void hyperstone_op91(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdisdecode(decode, 0, 1);
	hyperstone_ldxx1(decode);
}

static void hyperstone_op92(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdisdecode(decode, 1, 0);
	hyperstone_ldxx1(decode);
}

static void hyperstone_op93(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdisdecode(decode, 1, 1);
	hyperstone_ldxx1(decode);
}

static void hyperstone_op94(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdisdecode(decode, 0, 0);
	hyperstone_ldxx2(decode);
}

static void hyperstone_op95(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdisdecode(decode, 0, 1);
	hyperstone_ldxx2(decode);
}

static void hyperstone_op96(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdisdecode(decode, 1, 0);
	hyperstone_ldxx2(decode);
}

static void hyperstone_op97(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdisdecode(decode, 1, 1);
	hyperstone_ldxx2(decode);
}

static void hyperstone_op98(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdisdecode(decode, 0, 0);
	hyperstone_stxx1(decode);
}

static void hyperstone_op99(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdisdecode(decode, 0, 1);
	hyperstone_stxx1(decode);
}

static void hyperstone_op9a(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdisdecode(decode, 1, 0);
	hyperstone_stxx1(decode);
}

static void hyperstone_op9b(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdisdecode(decode, 1, 1);
	hyperstone_stxx1(decode);
}

static void hyperstone_op9c(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdisdecode(decode, 0, 0);
	hyperstone_stxx2(decode);
}

static void hyperstone_op9d(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdisdecode(decode, 0, 1);
	hyperstone_stxx2(decode);
}

static void hyperstone_op9e(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdisdecode(decode, 1, 0);
	hyperstone_stxx2(decode);
}

static void hyperstone_op9f(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdisdecode(decode, 1, 1);
	hyperstone_stxx2(decode);
}



static void hyperstone_opa0(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	Rndecode(decode, 0);
	hyperstone_shri(decode);
}

static void hyperstone_opa1(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	Rndecode(decode, 0);
	hyperstone_shri(decode);
}

static void hyperstone_opa2(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	Rndecode(decode, 1);
	hyperstone_shri(decode);
}

static void hyperstone_opa3(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	Rndecode(decode, 1);
	hyperstone_shri(decode);
}

static void hyperstone_opa4(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	Rndecode(decode, 0);
	hyperstone_sari(decode);
}

static void hyperstone_opa5(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	Rndecode(decode, 0);
	hyperstone_sari(decode);
}

static void hyperstone_opa6(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	Rndecode(decode, 1);
	hyperstone_sari(decode);
}

static void hyperstone_opa7(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	Rndecode(decode, 1);
	hyperstone_sari(decode);
}

static void hyperstone_opa8(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	Rndecode(decode, 0);
	hyperstone_shli(decode);
}

static void hyperstone_opa9(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	Rndecode(decode, 0);
	hyperstone_shli(decode);
}

static void hyperstone_opaa(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	Rndecode(decode, 1);
	hyperstone_shli(decode);
}

static void hyperstone_opab(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	Rndecode(decode, 1);
	hyperstone_shli(decode);
}

static void hyperstone_opac(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	no_decode(decode);
	reserved(decode);
}

static void hyperstone_opad(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	no_decode(decode);
	reserved(decode);
}

static void hyperstone_opae(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	no_decode(decode);
	reserved(decode);
}

static void hyperstone_opaf(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	no_decode(decode);
	reserved(decode);
}



static void hyperstone_opb0(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_mulu(decode);
}

static void hyperstone_opb1(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_mulu(decode);
}

static void hyperstone_opb2(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_mulu(decode);
}

static void hyperstone_opb3(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_mulu(decode);
}

static void hyperstone_opb4(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_muls(decode);
}

static void hyperstone_opb5(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_muls(decode);
}

static void hyperstone_opb6(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_muls(decode);
}

static void hyperstone_opb7(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_muls(decode);
}

static void hyperstone_opb8(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	Rndecode(decode, 0);
	hyperstone_set(decode);
}

static void hyperstone_opb9(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	Rndecode(decode, 0);
	hyperstone_set(decode);
}

static void hyperstone_opba(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	Rndecode(decode, 1);
	hyperstone_set(decode);
}

static void hyperstone_opbb(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	Rndecode(decode, 1);
	hyperstone_set(decode);
}

static void hyperstone_opbc(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 0);
	hyperstone_mul(decode);
}

static void hyperstone_opbd(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 0, 1);
	hyperstone_mul(decode);
}

static void hyperstone_opbe(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 0);
	hyperstone_mul(decode);
}

static void hyperstone_opbf(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	RRdecode(decode, 1, 1);
	hyperstone_mul(decode);
}



static void hyperstone_opc0(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_fadd(decode);
}

static void hyperstone_opc1(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_faddd(decode);
}

static void hyperstone_opc2(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_fsub(decode);
}

static void hyperstone_opc3(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_fsubd(decode);
}

static void hyperstone_opc4(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_fmul(decode);
}

static void hyperstone_opc5(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_fmuld(decode);
}

static void hyperstone_opc6(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_fdiv(decode);
}

static void hyperstone_opc7(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_fdivd(decode);
}

static void hyperstone_opc8(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_fcmp(decode);
}

static void hyperstone_opc9(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_fcmpd(decode);
}

static void hyperstone_opca(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_fcmpu(decode);
}

static void hyperstone_opcb(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_fcmpud(decode);
}

static void hyperstone_opcc(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_fcvt(decode);
}

static void hyperstone_opcd(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_fcvtd(decode);
}

static void hyperstone_opce(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	LLextdecode(decode);
	hyperstone_extend(decode);
}

static void hyperstone_opcf(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_do(decode);
}



static void hyperstone_opd0(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	LRdecode(decode, 0);
	hyperstone_ldwr(decode);
}

static void hyperstone_opd1(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	LRdecode(decode, 1);
	hyperstone_ldwr(decode);
}

static void hyperstone_opd2(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	LRdecode(decode, 0);
	hyperstone_lddr(decode);
}

static void hyperstone_opd3(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	LRdecode(decode, 1);
	hyperstone_lddr(decode);
}

static void hyperstone_opd4(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	LRdecode(decode, 0);
	hyperstone_ldwp(decode);
}

static void hyperstone_opd5(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	LRdecode(decode, 1);
	hyperstone_ldwp(decode);
}

static void hyperstone_opd6(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	LRdecode(decode, 0);
	hyperstone_lddp(decode);
}

static void hyperstone_opd7(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	LRdecode(decode, 1);
	hyperstone_lddp(decode);
}

static void hyperstone_opd8(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	LRdecode(decode, 0);
	hyperstone_stwr(decode);
}

static void hyperstone_opd9(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	LRdecode(decode, 1);
	hyperstone_stwr(decode);
}

static void hyperstone_opda(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	LRdecode(decode, 0);
	hyperstone_stdr(decode);
}

static void hyperstone_opdb(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	LRdecode(decode, 1);
	hyperstone_stdr(decode);
}

static void hyperstone_opdc(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	LRdecode(decode, 0);
	hyperstone_stwp(decode);
}

static void hyperstone_opdd(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	LRdecode(decode, 1);
	hyperstone_stwp(decode);
}

static void hyperstone_opde(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	LRdecode(decode, 0);
	hyperstone_stdp(decode);
}

static void hyperstone_opdf(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	LRdecode(decode, 1);
	hyperstone_stdp(decode);
}



static void hyperstone_ope0(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_dbv(decode);
}

static void hyperstone_ope1(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_dbnv(decode);
}

static void hyperstone_ope2(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_dbe(decode);
}

static void hyperstone_ope3(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_dbne(decode);
}

static void hyperstone_ope4(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_dbc(decode);
}

static void hyperstone_ope5(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_dbnc(decode);
}

static void hyperstone_ope6(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_dbse(decode);
}

static void hyperstone_ope7(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_dbht(decode);
}

static void hyperstone_ope8(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_dbn(decode);
}

static void hyperstone_ope9(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_dbnn(decode);
}

static void hyperstone_opea(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_dble(decode);
}

static void hyperstone_opeb(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_dbgt(decode);
}

static void hyperstone_opec(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_dbr(decode);
}

static void hyperstone_oped(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	LLdecode(decode);
	hyperstone_frame(decode);
}

static void hyperstone_opee(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	LRconstdecode(decode, 0);
	hyperstone_call(decode);
}

static void hyperstone_opef(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	LRconstdecode(decode, 1);
	hyperstone_call(decode);
}



static void hyperstone_opf0(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_bv(decode);
}

static void hyperstone_opf1(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_bnv(decode);
}

static void hyperstone_opf2(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_be(decode);
}

static void hyperstone_opf3(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_bne(decode);
}

static void hyperstone_opf4(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_bc(decode);
}

static void hyperstone_opf5(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_bnc(decode);
}

static void hyperstone_opf6(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_bse(decode);
}

static void hyperstone_opf7(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_bht(decode);
}

static void hyperstone_opf8(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_bn(decode);
}

static void hyperstone_opf9(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_bnn(decode);
}

static void hyperstone_opfa(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_ble(decode);
}

static void hyperstone_opfb(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_bgt(decode);
}

static void hyperstone_opfc(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	PCreldecode(decode);
	hyperstone_br(decode);
}

static void hyperstone_opfd(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	PCadrdecode(decode);
	hyperstone_trap(decode);
}

static void hyperstone_opfe(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	PCadrdecode(decode);
	hyperstone_trap(decode);
}

static void hyperstone_opff(UINT16 opcode)
{
	LOCAL_DECODE_INIT;
	PCadrdecode(decode);
	hyperstone_trap(decode);
}


static void (*hyperstone_op[0x100])(UINT16 opcode) =
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
