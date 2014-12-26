#!/usr/bin/python

import sys
import re


def EmitGroup04(f,funcname, opname, opexecute):
    # the mode 0x00 handler  
    print >>f, "ARCOMPACT_RETTYPE arcompact_device::arcompact_handle%s_p00(OPS_32)" % (funcname)
    print >>f, "{"
    print >>f, "	int size = 4;"
    print >>f, "	UINT32 limm = 0;"
    print >>f, "	int got_limm = 0;"
    print >>f, "	"
    print >>f, "	COMMON32_GET_breg;"
    print >>f, "	COMMON32_GET_F;"
    print >>f, "	COMMON32_GET_creg;"
    print >>f, "	COMMON32_GET_areg;"
    print >>f, "	"
    print >>f, "	UINT32 b, c;"
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
    print >>f, "	if (F)"
    print >>f, "	{"
    print >>f, "		arcompact_fatal(\"arcompact_handle%s_p00 (%s) (F set)\\n\"); // not yet supported" % (funcname, opname)
    print >>f, "	}"
    print >>f, "	return m_pc + (size >> 0);"
    print >>f, "}"
    print >>f, ""
    print >>f, ""
    # the mode 0x01 handler    
    print >>f, "ARCOMPACT_RETTYPE arcompact_device::arcompact_handle%s_p01(OPS_32)" % (funcname)
    print >>f, "{"
    print >>f, "	int size = 4;"
    print >>f, "	UINT32 limm = 0;"
    print >>f, "/*	int got_limm = 0; */"
    print >>f, "	"
    print >>f, "	COMMON32_GET_breg;"
    print >>f, "	COMMON32_GET_F;"
    print >>f, "	COMMON32_GET_u6;"
    print >>f, "	COMMON32_GET_areg;"
    print >>f, "	"
    print >>f, "	UINT32 b, c;"
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
    print >>f, "	if (F)"
    print >>f, "	{"
    print >>f, "		arcompact_fatal(\"arcompact_handle%s_p01 (%s) (F set)\\n\"); // not yet supported" % (funcname, opname)
    print >>f, "	}"
    print >>f, "	return m_pc + (size >> 0);"
    print >>f, "}"
    print >>f, ""
    print >>f, ""
    # the mode 0x10 handler 
    print >>f, "ARCOMPACT_RETTYPE arcompact_device::arcompact_handle%s_p10(OPS_32)" % (funcname)
    print >>f, "{"
    print >>f, "	int size = 4;"
    print >>f, "	UINT32 limm = 0;"
    print >>f, "/*	int got_limm = 0; */"
    print >>f, "	"
    print >>f, "	COMMON32_GET_breg;"
    print >>f, "	COMMON32_GET_F;"
    print >>f, "	COMMON32_GET_s12;"
    print >>f, "	COMMON32_GET_areg;"
    print >>f, "	"
    print >>f, "	UINT32 b, c;"
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
    print >>f, "	if (F)"
    print >>f, "	{"
    print >>f, "		arcompact_fatal(\"arcompact_handle%s_p01 (%s) (F set)\\n\"); // not yet supported" % (funcname, opname)
    print >>f, "	}"
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

try:
    f = open(sys.argv[1], "w")
except Exception, err:
    logging.error("cannot write file %s [%s]", fname, err)
    sys.exit(1)


EmitGroup04(f, "04_00", "ADD", "m_regs[areg] = b + c;" )

EmitGroup04(f, "04_04", "AND", "m_regs[areg] = b & c;" )
EmitGroup04(f, "04_05", "OR",  "m_regs[areg] = b | c;" )
EmitGroup04(f, "04_06", "BIC", "m_regs[areg] = b & (~c);" )
EmitGroup04(f, "04_07", "XOR", "m_regs[areg] = b ^ c;" )

EmitGroup04(f, "04_0f", "BSET", "m_regs[areg] = b | (1 << (c & 0x1f));" )

EmitGroup04(f, "04_16", "ADD3", "m_regs[areg] = b + (c << 3);" )


EmitGroup04(f, "05_00", "ASL", "m_regs[areg] = b << (c&0x1f);" )
EmitGroup04(f, "05_01", "LSR", "m_regs[areg] = b >> (c&0x1f);" )


