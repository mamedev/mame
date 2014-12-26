#!/usr/bin/python

import sys
import re

def EmitGroup04_Handle_NZ_Flags(f, funcname, opname):
        print >>f, "		if (result & 0x80000000) { STATUS32_SET_N; }"
        print >>f, "		else { STATUS32_CLEAR_N; }"
        print >>f, "		if (result == 0x00000000) { STATUS32_SET_Z; }"
        print >>f, "		else { STATUS32_CLEAR_Z; }"

def EmitGroup04_no_Flags(f, funcname, opname):
       print >>f, "		// no flag changes"

def EmitGroup04_unsupported_Flags(f, funcname, opname):
        print >>f, "		arcompact_fatal(\"arcompact_handle%s (%s) (F set)\\n\"); // not yet supported" % (funcname, opname)

def EmitGroup04_Flaghandler(f,funcname, opname, flagcondition, flaghandler):
    if flagcondition == -1:
        print >>f, "	if (F)"
        print >>f, "	{"
        flaghandler(f, funcname, opname)
        print >>f, "	}"
    elif flagcondition == 0:
        print >>f, "	if (0)"
        print >>f, "	{"
        flaghandler(f, funcname, opname)
        print >>f, "	}"
    elif flagcondition == 1:
        print >>f, "	if (1)"
        print >>f, "	{"
        flaghandler(f, funcname, opname)
        print >>f, "	}"

def EmitGroup04(f,funcname, opname, opexecute, ignore_a, breg_is_dst_only, flagcondition, flaghandler):
    # the mode 0x00 handler  
    print >>f, "ARCOMPACT_RETTYPE arcompact_device::arcompact_handle%s_p00(OPS_32)" % (funcname)
    print >>f, "{"
    print >>f, "	int size = 4;"
    
    print >>f, "	UINT32 limm = 0;"
    
    print >>f, "	int got_limm = 0;"
    print >>f, "	"
    print >>f, "	COMMON32_GET_breg;"

    if flagcondition == -1:
        print >>f, "	COMMON32_GET_F;"
    
    print >>f, "	COMMON32_GET_creg;"

    if ignore_a == 0:
        print >>f, "	COMMON32_GET_areg;"
    elif ignore_a == 1:
       print >>f, "     //COMMON32_GET_areg; // areg is reserved / not used"
    
    print >>f, "	"
    
    print >>f, "	UINT32 c;"
    if breg_is_dst_only == 0:
        print >>f, "	UINT32 b;"
        print >>f, "	"
        print >>f, "	if (breg == LIMM_REG)"
        print >>f, "	{"
        print >>f, "		GET_LIMM_32;"
        print >>f, "		size = 8;"
        print >>f, "		got_limm = 1;"
        print >>f, "		b = limm;"
        print >>f, "	}"
        print >>f, "	else"
        print >>f, "	{"
        print >>f, "		b = m_regs[breg];"
        print >>f, "	}"
    
    print >>f, "	"
    print >>f, "	if (creg == LIMM_REG)"
    print >>f, "	{"
    print >>f, "		if (!got_limm)"
    print >>f, "		{"
    print >>f, "			GET_LIMM_32;"
    print >>f, "			size = 8;"
    print >>f, "		}"
    print >>f, "		c = limm;"
    print >>f, "	}"
    print >>f, "	else"
    print >>f, "	{"
    print >>f, "		c = m_regs[creg];"
    print >>f, "	}"
    print >>f, "	/* todo: is the limm, limm syntax valid? (it's pointless.) */"
    print >>f, "	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */"
    print >>f, "	%s" % (opexecute)
    print >>f, "	"
    EmitGroup04_Flaghandler(f,funcname,opname,flagcondition,flaghandler)
    print >>f, "	return m_pc + (size >> 0);"
    print >>f, "}"
    print >>f, ""
    print >>f, ""
    # the mode 0x01 handler    
    print >>f, "ARCOMPACT_RETTYPE arcompact_device::arcompact_handle%s_p01(OPS_32)" % (funcname)
    print >>f, "{"
    print >>f, "	int size = 4;"
    
    if breg_is_dst_only == 0:	
        print >>f, "	UINT32 limm = 0;"
    
    print >>f, "/*	int got_limm = 0; */"
    print >>f, "	"
    print >>f, "	COMMON32_GET_breg;"
    
    if flagcondition == -1:
        print >>f, "	COMMON32_GET_F;"
    
    print >>f, "	COMMON32_GET_u6;"
    
    if ignore_a == 0:
        print >>f, "	COMMON32_GET_areg;"
    elif ignore_a == 1:
       print >>f, "     //COMMON32_GET_areg; // areg is reserved / not used"
    
    print >>f, "	"
    
    print >>f, "	UINT32 c;"
    if breg_is_dst_only == 0:
        print >>f, "	UINT32 b;"
        print >>f, "	"
        print >>f, "	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */"
        print >>f, "	if (breg == LIMM_REG)"
        print >>f, "	{"
        print >>f, "		GET_LIMM_32;"
        print >>f, "		size = 8;"
        print >>f, "/*		got_limm = 1; */"
        print >>f, "		b = limm;"
        print >>f, "	}"
        print >>f, "	else"
        print >>f, "	{"
        print >>f, "		b = m_regs[breg];"
        print >>f, "	}"
    
    print >>f, "    "
    print >>f, " 	c = u;"
    print >>f, "	"
    print >>f, "	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */"
    print >>f, "	%s" % (opexecute)
    print >>f, "	"
    EmitGroup04_Flaghandler(f,funcname,opname,flagcondition,flaghandler)
    print >>f, "	return m_pc + (size >> 0);"
    print >>f, "}"
    print >>f, ""
    print >>f, ""
    # the mode 0x10 handler 
    print >>f, "ARCOMPACT_RETTYPE arcompact_device::arcompact_handle%s_p10(OPS_32)" % (funcname)
    print >>f, "{"
    print >>f, "	int size = 4;"

    if breg_is_dst_only == 0:
        print >>f, "	UINT32 limm = 0;"
    
    print >>f, "/*	int got_limm = 0; */"
    print >>f, "	"
    print >>f, "	COMMON32_GET_breg;"

    if flagcondition == -1:	
        print >>f, "	COMMON32_GET_F;"
    
    print >>f, "	COMMON32_GET_s12;"
    
    if ignore_a == 0:	
        print >>f, "	COMMON32_GET_areg;"
    elif ignore_a == 1:
       print >>f, "     //COMMON32_GET_areg; // areg is reserved / not used"
    
    print >>f, "	"
    print >>f, "	UINT32 c;"
    if breg_is_dst_only == 0:
        print >>f, "	UINT32 b;"
        print >>f, "	"
        print >>f, "	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */"
        print >>f, "	if (breg == LIMM_REG)"
        print >>f, "	{"
        print >>f, "		GET_LIMM_32;"
        print >>f, "		size = 8;"
        print >>f, "/*		got_limm = 1; */"
        print >>f, "		b = limm;"
        print >>f, "	}"
        print >>f, "	else"
        print >>f, "	{"
        print >>f, "		b = m_regs[breg];"
        print >>f, "	}"
    
    print >>f, "    "
    print >>f, " 	c = (UINT32)S;"
    print >>f, "	"
    print >>f, "	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */"
    print >>f, "	%s" % (opexecute)
    print >>f, "	"
    EmitGroup04_Flaghandler(f,funcname,opname,flagcondition,flaghandler)
    print >>f, "	return m_pc + (size >> 0);"
    print >>f, "}"
    print >>f, ""
    print >>f, ""
    # the mode 0x11 m0 handler    
    print >>f, "ARCOMPACT_RETTYPE arcompact_device::arcompact_handle%s_p11_m0(OPS_32)" % (funcname)
    print >>f, "{"
    print >>f, "	int size = 4;"
    print >>f, "	arcompact_fatal(\"arcompact_handle%s_p11_m0 (%s)\\n\");"  % (funcname, opname)
    print >>f, "	return m_pc + (size >> 0);"
    print >>f, "}"
    print >>f, ""
    print >>f, ""	
    # the mode 0x11 m1 handler    
    print >>f, "ARCOMPACT_RETTYPE arcompact_device::arcompact_handle%s_p11_m1(OPS_32)" % (funcname)
    print >>f, "{"
    print >>f, "	int size = 4;"
    print >>f, "	arcompact_fatal(\"arcompact_handle%s_p11_m1 (%s)\\n\");"  % (funcname, opname)
    print >>f, "	return m_pc + (size >> 0);"
    print >>f, "}"
    print >>f, ""
    print >>f, ""

#  xxx_S b, b, u5 format opcodes
def EmitGroup17(f,funcname, opname, opexecute):
    print >>f, "ARCOMPACT_RETTYPE arcompact_device::arcompact_handle%s(OPS_16)" % (funcname)
    print >>f, "{"
    print >>f, "	int breg, u;"
    print >>f, "	"
    print >>f, "	COMMON16_GET_breg;"
    print >>f, "	COMMON16_GET_u5;"
    print >>f, "	"
    print >>f, "	REG_16BIT_RANGE(breg);"
    print >>f, "	"
    print >>f, "	%s" % (opexecute) 
    print >>f, "	"
    print >>f, "	return m_pc + (2 >> 0);"
    print >>f, "}"
    print >>f, ""
    print >>f, ""



try:
    f = open(sys.argv[1], "w")
except Exception, err:
    logging.error("cannot write file %s [%s]", fname, err)
    sys.exit(1)


EmitGroup04(f, "04_00", "ADD", "UINT32 result = b + c; m_regs[areg] = result;", 0,0, -1, EmitGroup04_unsupported_Flags )

EmitGroup04(f, "04_02", "SUB", "UINT32 result = b - c; m_regs[areg] = result;", 0,0, -1, EmitGroup04_unsupported_Flags  )

EmitGroup04(f, "04_04", "AND", "UINT32 result = b & c; m_regs[areg] = result;", 0,0, -1, EmitGroup04_unsupported_Flags  )
EmitGroup04(f, "04_05", "OR",  "UINT32 result = b | c; m_regs[areg] = result;", 0,0, -1, EmitGroup04_unsupported_Flags  )
EmitGroup04(f, "04_06", "BIC", "UINT32 result = b & (~c); m_regs[areg] = result;",  0,0, -1, EmitGroup04_unsupported_Flags  )
EmitGroup04(f, "04_07", "XOR", "UINT32 result = b ^ c; m_regs[areg] = result;", 0,0, -1, EmitGroup04_unsupported_Flags  )

EmitGroup04(f, "04_0a", "MOV", "UINT32 result = c; m_regs[breg] = result;", 1,1, -1, EmitGroup04_Handle_NZ_Flags  )

EmitGroup04(f, "04_0f", "BSET", "UINT32 result = b | (1 << (c & 0x1f)); m_regs[areg] = result;", 0,0, -1, EmitGroup04_unsupported_Flags  )

EmitGroup04(f, "04_15", "ADD2", "UINT32 result = b + (c << 2); m_regs[areg] = result;", 0,0, -1, EmitGroup04_unsupported_Flags  )
EmitGroup04(f, "04_16", "ADD3", "UINT32 result = b + (c << 3); m_regs[areg] = result;", 0,0, -1, EmitGroup04_unsupported_Flags  )


EmitGroup04(f, "05_00", "ASL", "UINT32 result = b << (c&0x1f); m_regs[areg] = result;", 0,0, -1, EmitGroup04_unsupported_Flags  )
EmitGroup04(f, "05_01", "LSR", "UINT32 result = b >> (c&0x1f); m_regs[areg] = result;", 0,0, -1, EmitGroup04_unsupported_Flags  )

#  xxx_S b, b, u5 format opcodes
EmitGroup17(f, "17_00", "ASL_S",  "m_regs[breg] = m_regs[breg] << (u&0x1f);" )
EmitGroup17(f, "17_01", "LSR_S",  "m_regs[breg] = m_regs[breg] >> (u&0x1f);" )
EmitGroup17(f, "17_02", "ASR_S",  "INT32 temp = (INT32)m_regs[breg]; m_regs[breg] = temp >> (u&0x1f); // treat it as a signed value, so sign extension occurs during shift" )
EmitGroup17(f, "17_03", "SUB_S",  "m_regs[breg] = m_regs[breg] - u;" )
EmitGroup17(f, "17_04", "BSET_S", "m_regs[breg] = m_regs[breg] | (1 << (u & 0x1f));" )

EmitGroup17(f, "17_06", "BMSK_S", "m_regs[breg] = m_regs[breg] | ((1 << (u + 1)) - 1);" )




