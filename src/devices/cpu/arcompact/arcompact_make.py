#!/usr/bin/python

from __future__ import print_function
import sys

def EmitGroup04_Handle_NZ_Flags(f, funcname, opname):
        print("		if (result & 0x80000000) { STATUS32_SET_N; }", file=f)
        print("		else { STATUS32_CLEAR_N; }", file=f)
        print("		if (result == 0x00000000) { STATUS32_SET_Z; }", file=f)
        print("		else { STATUS32_CLEAR_Z; }", file=f)

def EmitGroup04_Handle_NZC_LSR1_Flags(f, funcname, opname):
        print("		if (result & 0x80000000) { STATUS32_SET_N; }", file=f)
        print("		else { STATUS32_CLEAR_N; }", file=f)
        print("		if (result == 0x00000000) { STATUS32_SET_Z; }", file=f)
        print("		else { STATUS32_CLEAR_Z; }", file=f)
        print("		if (c == 0x00000001) { STATUS32_SET_C; }", file=f)
        print("		else { STATUS32_CLEAR_C; }", file=f)

def EmitGroup04_Handle_NZCV_ADD_Flags(f, funcname, opname):
        print("		if (result & 0x80000000) { STATUS32_SET_N; }", file=f)
        print("		else { STATUS32_CLEAR_N; }", file=f)
        print("		if (result == 0x00000000) { STATUS32_SET_Z; }", file=f)
        print("		else { STATUS32_CLEAR_Z; }", file=f)
        print("		if ((b & 0x80000000) == (c & 0x80000000))", file=f)
        print("		{", file=f)
        print("			if ((result & 0x80000000) != (b & 0x80000000))", file=f)
        print("			{", file=f)
        print("				STATUS32_SET_V;", file=f)
        print("			}", file=f)
        print("			else", file=f)
        print("			{", file=f)
        print("				STATUS32_CLEAR_V;", file=f)
        print("			}", file=f)
        print("		}", file=f)
        print("		if (b < c)", file=f)
        print("		{", file=f)
        print("			STATUS32_SET_C;", file=f)
        print("		}", file=f)
        print("		else", file=f)
        print("		{", file=f)
        print("			STATUS32_CLEAR_C;", file=f)
        print("		}", file=f)


def EmitGroup04_no_Flags(f, funcname, opname):
       print("		// no flag changes", file=f)

def EmitGroup04_unsupported_Flags(f, funcname, opname):
        print("		arcompact_fatal(\"arcompact_handle%s (%s) (F set)\\n\"); // not yet supported" % (funcname, opname), file=f)

def EmitGroup04_Flaghandler(f,funcname, opname, flagcondition, flaghandler):
    if flagcondition == -1:
        print("	if (F)", file=f)
        print("	{", file=f)
        flaghandler(f, funcname, opname)
        print("	}", file=f)
    elif flagcondition == 0:
        print("	if (0)", file=f)
        print("	{", file=f)
        flaghandler(f, funcname, opname)
        print("	}", file=f)
    elif flagcondition == 1:
        print("	if (1)", file=f)
        print("	{", file=f)
        flaghandler(f, funcname, opname)
        print("	}", file=f)

def EmitGroup04_u5fragment(f,funcname, opname, opexecute, opwrite, opwrite_alt, ignore_a, breg_is_dst_only, flagcondition, flaghandler):
    print("	int size = 4;", file=f)
    
    if breg_is_dst_only == 0:	
        print("	UINT32 limm = 0;", file=f)
    
    print("/*	int got_limm = 0; */", file=f)
    print("	", file=f)
    print("	COMMON32_GET_breg;", file=f)
    
    if flagcondition == -1:
        print("	COMMON32_GET_F;", file=f)
    
    print("	COMMON32_GET_u6;", file=f)
    
    if ignore_a == 0:
        print("	COMMON32_GET_areg;", file=f)
    elif ignore_a == 1:
        print("     //COMMON32_GET_areg; // areg is reserved / not used", file=f)
    elif ignore_a == 2:
        print("     //COMMON32_GET_areg; // areg bits already used as opcode select", file=f)
    elif ignore_a == 3:
        print("     //COMMON32_GET_areg; // areg bits already used as condition code select", file=f)
    print("	", file=f)
    
    print("	UINT32 c;", file=f)
    if breg_is_dst_only == 0:
        print("	UINT32 b;", file=f)
        print("	", file=f)
        print("	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */", file=f)
        print("	if (breg == LIMM_REG)", file=f)
        print("	{", file=f)
        print("		GET_LIMM_32;", file=f)
        print("		size = 8;", file=f)
        print("/*		got_limm = 1; */", file=f)
        print("		b = limm;", file=f)
        print("	}", file=f)
        print("	else", file=f)
        print("	{", file=f)
        print("		b = m_regs[breg];", file=f)
        print("	}", file=f)
    
    print("    ", file=f)
    print(" 	c = u;", file=f)
    print("	", file=f)
    print("	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */", file=f)

def EmitGroup04(f,funcname, opname, opexecute, opwrite, opwrite_alt, ignore_a, breg_is_dst_only, flagcondition, flaghandler):
    # the mode 0x00 handler  
    print("ARCOMPACT_RETTYPE arcompact_device::arcompact_handle%s_p00(OPS_32)" % funcname, file=f)
    print("{", file=f)
    print("	int size = 4;", file=f)
    
    print("	UINT32 limm = 0;", file=f)
    
    print("	int got_limm = 0;", file=f)
    print("	", file=f)
    print("	COMMON32_GET_breg;", file=f)

    if flagcondition == -1:
        print("	COMMON32_GET_F;", file=f)
        
    print("	COMMON32_GET_creg;", file=f)

    if ignore_a == 0:
        print("	COMMON32_GET_areg;", file=f)
    elif ignore_a == 1:
        print("     //COMMON32_GET_areg; // areg is reserved / not used", file=f)
    elif ignore_a == 2:
        print("     //COMMON32_GET_areg; // areg bits already used as opcode select", file=f)
  
    print("	", file=f)
    
    print("	UINT32 c;", file=f)
    if breg_is_dst_only == 0:
        print("	UINT32 b;", file=f)
        print("	", file=f)
        print("	if (breg == LIMM_REG)", file=f)
        print("	{", file=f)
        print("		GET_LIMM_32;", file=f)
        print("		size = 8;", file=f)
        print("		got_limm = 1;", file=f)
        print("		b = limm;", file=f)
        print("	}", file=f)
        print("	else", file=f)
        print("	{", file=f)
        print("		b = m_regs[breg];", file=f)
        print("	}", file=f)
    
    print("	", file=f)
    print("	if (creg == LIMM_REG)", file=f)
    print("	{", file=f)
    print("		if (!got_limm)", file=f)
    print("		{", file=f)
    print("			GET_LIMM_32;", file=f)
    print("			size = 8;", file=f)
    print("		}", file=f)
    print("		c = limm;", file=f)
    print("	}", file=f)
    print("	else", file=f)
    print("	{", file=f)
    print("		c = m_regs[creg];", file=f)
    print("	}", file=f)
    print("	/* todo: is the limm, limm syntax valid? (it's pointless.) */", file=f)
    print("	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */", file=f)
    print("	%s" % opexecute, file=f)
    print("	%s" % opwrite, file=f)
    print("	", file=f)
    EmitGroup04_Flaghandler(f,funcname,opname,flagcondition,flaghandler)
    print("	return m_pc + (size >> 0);", file=f)
    print("}", file=f)
    print("", file=f)
    print("", file=f)
    # the mode 0x01 handler    
    print("ARCOMPACT_RETTYPE arcompact_device::arcompact_handle%s_p01(OPS_32)" % funcname, file=f)
    print("{", file=f)
    EmitGroup04_u5fragment(f,funcname, opname, opexecute, opwrite, opwrite_alt, ignore_a, breg_is_dst_only, flagcondition, flaghandler)
    print("	%s" % opexecute, file=f)
    print("	%s" % opwrite, file=f)
    print("	", file=f)
    EmitGroup04_Flaghandler(f,funcname,opname,flagcondition,flaghandler)
    print("	return m_pc + (size >> 0);", file=f)
    print("}", file=f)
    print("", file=f)
    print("", file=f)
    # the mode 0x10 handler 
    print("ARCOMPACT_RETTYPE arcompact_device::arcompact_handle%s_p10(OPS_32)" % funcname, file=f)
    if ignore_a == 2:
        print("{", file=f)
        print("	int size = 4;", file=f)
        print("	arcompact_fatal(\"illegal arcompact_handle%s_p10 (ares bits already used as opcode select, can't be used as s12) (%s)\\n\");"  % (funcname, opname), file=f)
        print("	return m_pc + (size >> 0);", file=f)
        print("}", file=f)
    else:
        print("{", file=f)
        print("	int size = 4;", file=f)
        if breg_is_dst_only == 0:
            print("	UINT32 limm = 0;", file=f)
        
        print("/*	int got_limm = 0; */", file=f)
        print("	", file=f)
        print("	COMMON32_GET_breg;", file=f)
    
        if flagcondition == -1:	
            print("	COMMON32_GET_F;", file=f)
        
        print("	COMMON32_GET_s12;", file=f)
        
        # areg can't be used here, it's used for s12 bits
        
        print("	", file=f)
        print("	UINT32 c;", file=f)
        if breg_is_dst_only == 0:
            print("	UINT32 b;", file=f)
            print("	", file=f)
            print("	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */", file=f)
            print("	if (breg == LIMM_REG)", file=f)
            print("	{", file=f)
            print("		GET_LIMM_32;", file=f)
            print("		size = 8;", file=f)
            print("/*		got_limm = 1; */", file=f)
            print("		b = limm;", file=f)
            print("	}", file=f)
            print("	else", file=f)
            print("	{", file=f)
            print("		b = m_regs[breg];", file=f)
            print("	}", file=f)
        
        print("    ", file=f)
        print(" 	c = (UINT32)S;", file=f)
        print("	", file=f)
        print("	/* todo: if areg = LIMM then there is no result (but since that register can never be read, I guess it doesn't matter if we store it there anyway?) */", file=f)
        print("	%s" % opexecute, file=f)
        print("	%s" % opwrite_alt, file=f)
        print("	", file=f)
        EmitGroup04_Flaghandler(f,funcname,opname,flagcondition,flaghandler)
        print("	return m_pc + (size >> 0);", file=f)
        print("}", file=f)
    print("", file=f)
    print("", file=f)
    # the mode 0x11 m0 handler    
    print("ARCOMPACT_RETTYPE arcompact_device::arcompact_handle%s_p11_m0(OPS_32)" % funcname, file=f)
    if ignore_a == 2:
        print("{", file=f)
        print("	int size = 4;", file=f)
        print("	arcompact_fatal(\"illegal arcompact_handle%s_p11_m0 (ares bits already used as opcode select, can't be used as Q condition) (%s)\\n\");"  % (funcname, opname), file=f)
        print("	return m_pc + (size >> 0);", file=f)
        print("}", file=f)
    else:
        print("{", file=f)
        print("	int size = 4;", file=f)
        print("	arcompact_fatal(\"arcompact_handle%s_p11_m0 (%s)\\n\");"  % (funcname, opname), file=f)
        print("	return m_pc + (size >> 0);", file=f)
        print("}", file=f)
        print("", file=f)
        print("", file=f)
    # the mode 0x11 m1 handler    
    print("ARCOMPACT_RETTYPE arcompact_device::arcompact_handle%s_p11_m1(OPS_32)" % funcname, file=f)
    if ignore_a == 2:
        print("{", file=f)
        print("	int size = 4;", file=f)
        print("	arcompact_fatal(\"illegal arcompact_handle%s_p11_m1 (ares bits already used as opcode select, can't be used as Q condition) (%s)\\n\");"  % (funcname, opname), file=f)
        print("	return m_pc + (size >> 0);", file=f)
        print("}", file=f)
    else:
        print("{", file=f)
        EmitGroup04_u5fragment(f,funcname, opname, opexecute, opwrite, opwrite_alt, 3, breg_is_dst_only, flagcondition, flaghandler)
        print("	COMMON32_GET_CONDITION;", file=f)
        print("	if (!check_condition(condition))", file=f)
        print("		return m_pc + (size>>0);", file=f)
        print("", file=f)
        print("	%s" % opexecute, file=f)
        print("	%s" % opwrite_alt, file=f)
        print("	", file=f)
        EmitGroup04_Flaghandler(f,funcname,opname,flagcondition,flaghandler)
        print("	return m_pc + (size >> 0);", file=f)
        print("}", file=f)
    print("", file=f)
    print("", file=f)


# xxx_S  c, b, u3  format opcodes (note c is destination)
def EmitGroup0d(f,funcname, opname, opexecute, opwrite):
    print("ARCOMPACT_RETTYPE arcompact_device::arcompact_handle%s(OPS_16)"  % funcname, file=f)
    print("{", file=f)
    print("	int u, breg, creg;", file=f)
    print("", file=f)
    print("	COMMON16_GET_u3;", file=f)
    print("	COMMON16_GET_breg;", file=f)
    print("	COMMON16_GET_creg;", file=f)
    print("", file=f)
    print("	REG_16BIT_RANGE(breg);", file=f)
    print("	REG_16BIT_RANGE(creg);", file=f)
    print("", file=f)
    print("	%s" % opexecute, file=f)
    print("	%s" % opwrite, file=f)
    print("", file=f)
    print("	return m_pc + (2 >> 0);", file=f)
    print("}", file=f)
    print("", file=f)
    print("", file=f)


# xxx_S b <- b,c format opcodes
def EmitGroup0f(f,funcname, opname, opexecute, opwrite):
    print("ARCOMPACT_RETTYPE arcompact_device::arcompact_handle%s(OPS_16)"% funcname, file=f)
    print("{", file=f)
    print("	int breg, creg;", file=f)
    print("", file=f)
    print("	COMMON16_GET_breg;", file=f)
    print("	COMMON16_GET_creg;", file=f)
    print("", file=f)
    print("	REG_16BIT_RANGE(breg);", file=f)
    print("	REG_16BIT_RANGE(creg);", file=f)
    print("", file=f)
    print("	%s" % opexecute, file=f)
    print("	%s" % opwrite, file=f)
    print("", file=f)
    print("	return m_pc + (2 >> 0);", file=f)
    print("}", file=f)
    print("", file=f)
    print("", file=f)


#  xxx_S b, b, u5 format opcodes
def EmitGroup17(f,funcname, opname, opexecute):
    print("ARCOMPACT_RETTYPE arcompact_device::arcompact_handle%s(OPS_16)" % funcname, file=f)
    print("{", file=f)
    print("	int breg, u;", file=f)
    print("	", file=f)
    print("	COMMON16_GET_breg;", file=f)
    print("	COMMON16_GET_u5;", file=f)
    print("	", file=f)
    print("	REG_16BIT_RANGE(breg);", file=f)
    print("	", file=f)
    print("	%s" % opexecute, file=f)
    print("	", file=f)
    print("	return m_pc + (2 >> 0);", file=f)
    print("}", file=f)
    print("", file=f)
    print("", file=f)



try:
    f = open(sys.argv[1], "w")
except Exception:
    err = sys.exc_info()[1]
    sys.stderr.write("cannot write file %s [%s]\n" % (sys.argv[1], err))
    sys.exit(1)


EmitGroup04(f, "04_00", "ADD",  "UINT32 result = b + c;",                 "m_regs[areg] = result;", "m_regs[breg] = result;", 0,0, -1, EmitGroup04_Handle_NZCV_ADD_Flags )

EmitGroup04(f, "04_02", "SUB",  "UINT32 result = b - c;",                 "m_regs[areg] = result;", "m_regs[breg] = result;", 0,0, -1, EmitGroup04_unsupported_Flags  )

EmitGroup04(f, "04_04", "AND",  "UINT32 result = b & c;",                 "if (areg != LIMM_REG) { m_regs[areg] = result; }", "if (breg != LIMM_REG) { m_regs[breg] = result; }", 0,0, -1, EmitGroup04_Handle_NZ_Flags  )
EmitGroup04(f, "04_05", "OR",   "UINT32 result = b | c;",                 "m_regs[areg] = result;", "m_regs[breg] = result;", 0,0, -1, EmitGroup04_unsupported_Flags  )
EmitGroup04(f, "04_06", "BIC",  "UINT32 result = b & (~c);",              "m_regs[areg] = result;", "m_regs[breg] = result;",  0,0, -1, EmitGroup04_unsupported_Flags  )
EmitGroup04(f, "04_07", "XOR",  "UINT32 result = b ^ c;",                 "m_regs[areg] = result;", "m_regs[breg] = result;", 0,0, -1, EmitGroup04_unsupported_Flags  )

EmitGroup04(f, "04_0a", "MOV",  "UINT32 result = c;",                     "m_regs[breg] = result;", "m_regs[breg] = result;", 1,1, -1, EmitGroup04_Handle_NZ_Flags  ) # special case, result always goes to breg

EmitGroup04(f, "04_0e", "RSUB", "UINT32 result = c - b;",                 "m_regs[areg] = result;", "m_regs[breg] = result;", 0,0, -1, EmitGroup04_unsupported_Flags  )
EmitGroup04(f, "04_0f", "BSET", "UINT32 result = b | (1 << (c & 0x1f));", "m_regs[areg] = result;", "m_regs[breg] = result;", 0,0, -1, EmitGroup04_unsupported_Flags  )

EmitGroup04(f, "04_13", "BMSK", "UINT32 result = b & ((1<<(c+1))-1);",    "m_regs[areg] = result;", "m_regs[breg] = result;", 0,0, -1, EmitGroup04_unsupported_Flags  )


EmitGroup04(f, "04_14", "ADD1", "UINT32 result = b + (c << 1);",          "m_regs[areg] = result;", "m_regs[breg] = result;", 0,0, -1, EmitGroup04_unsupported_Flags  )
EmitGroup04(f, "04_15", "ADD2", "UINT32 result = b + (c << 2);",          "m_regs[areg] = result;", "m_regs[breg] = result;", 0,0, -1, EmitGroup04_unsupported_Flags  )
EmitGroup04(f, "04_16", "ADD3", "UINT32 result = b + (c << 3);",          "m_regs[areg] = result;", "m_regs[breg] = result;", 0,0, -1, EmitGroup04_unsupported_Flags  )
EmitGroup04(f, "04_17", "SUB1", "UINT32 result = b - (c << 1);",          "m_regs[areg] = result;", "m_regs[breg] = result;", 0,0, -1, EmitGroup04_unsupported_Flags  )
EmitGroup04(f, "04_18", "SUB2", "UINT32 result = b - (c << 2);",          "m_regs[areg] = result;", "m_regs[breg] = result;", 0,0, -1, EmitGroup04_unsupported_Flags  )
EmitGroup04(f, "04_19", "SUB3", "UINT32 result = b - (c << 3);",          "m_regs[areg] = result;", "m_regs[breg] = result;", 0,0, -1, EmitGroup04_unsupported_Flags  )

EmitGroup04(f, "04_2a", "LR", "m_regs[breg] = READAUX(c);", "", "", 1,1, -1, EmitGroup04_no_Flags  ) # this can't be conditional (todo)
EmitGroup04(f, "04_2b", "SR", "WRITEAUX(c,b);", "", "", 1,0, -1, EmitGroup04_no_Flags  ) # this can't be conditional (todo)



EmitGroup04(f, "05_00", "ASL", "UINT32 result = b << (c&0x1f);", "m_regs[areg] = result;", "m_regs[breg] = result;", 0,0, -1, EmitGroup04_unsupported_Flags  )
EmitGroup04(f, "05_01", "LSR", "UINT32 result = b >> (c&0x1f);", "m_regs[areg] = result;", "m_regs[breg] = result;", 0,0, -1, EmitGroup04_unsupported_Flags  )

# the 04_2f subgroup uses the same encoding, but the areg is already used as sub-opcode select, so any modes relying on areg bits for other reasons (sign, condition) (modes 10, 11m0, 11m1) are illegal.  the destination is also breg not areg
EmitGroup04(f, "04_2f_02", "LSR1", "UINT32 result = c >> 1;",                                                                                                                             "m_regs[breg] = result;","", 2,1, -1, EmitGroup04_Handle_NZC_LSR1_Flags  ) # no alt handler (invalid path)
EmitGroup04(f, "04_2f_03", "ROR", "int shift = 1; UINT32 mask = (1 << (shift)) - 1; mask <<= (32-shift); UINT32 result = ((c >> shift) & ~mask) | ((c << (32-shift)) & mask);",          "m_regs[breg] = result;","", 2,1, -1, EmitGroup04_Handle_NZC_LSR1_Flags  )


EmitGroup04(f, "04_2f_07", "EXTB", "UINT32 result = c & 0x000000ff;",  "m_regs[breg] = result;","", 2,1, -1, EmitGroup04_unsupported_Flags  ) # ^
EmitGroup04(f, "04_2f_08", "EXTW", "UINT32 result = c & 0x0000ffff;",  "m_regs[breg] = result;","", 2,1, -1, EmitGroup04_unsupported_Flags  ) # ^

# xxx_S  c, b, u3  format opcodes (note c is destination)
EmitGroup0d(f, "0d_00", "ADD_S", "UINT32 result = m_regs[breg] + u;",         "m_regs[creg] = result;" )
EmitGroup0d(f, "0d_01", "SUB_S", "UINT32 result = m_regs[breg] - u;",         "m_regs[creg] = result;" )
EmitGroup0d(f, "0d_02", "ASL_S", "UINT32 result = m_regs[breg] << u;",        "m_regs[creg] = result;" )

# xxx_S b <- b,c format opcodes  (or in some cases xxx_S b,c)
EmitGroup0f(f, "0f_02", "SUB_S", "UINT32 result = m_regs[breg] - m_regs[creg];",        "m_regs[breg] = result;" )
EmitGroup0f(f, "0f_04", "AND_S", "UINT32 result = m_regs[breg] & m_regs[creg];",        "m_regs[breg] = result;" )
EmitGroup0f(f, "0f_05", "OR_S",  "UINT32 result = m_regs[breg] | m_regs[creg];",        "m_regs[breg] = result;" )
EmitGroup0f(f, "0f_07", "XOR_S", "UINT32 result = m_regs[breg] ^ m_regs[creg];",        "m_regs[breg] = result;" )
EmitGroup0f(f, "0f_0f", "EXTB_S","UINT32 result = m_regs[creg] & 0x000000ff;",          "m_regs[breg] = result;" )
EmitGroup0f(f, "0f_10", "EXTW_S","UINT32 result = m_regs[creg] & 0x0000ffff;",          "m_regs[breg] = result;" )
EmitGroup0f(f, "0f_13", "NEG_S"," UINT32 result = 0 - m_regs[creg];",                   "m_regs[breg] = result;" )

EmitGroup0f(f, "0f_14", "ADD1_S"," UINT32 result = m_regs[breg] + (m_regs[creg] <<1);", "m_regs[breg] = result;" )
EmitGroup0f(f, "0f_15", "ADD2_S"," UINT32 result = m_regs[breg] + (m_regs[creg] <<2);", "m_regs[breg] = result;" )
EmitGroup0f(f, "0f_16", "ADD3_S"," UINT32 result = m_regs[breg] + (m_regs[creg] <<3);", "m_regs[breg] = result;" )

EmitGroup0f(f, "0f_19", "LSR_S", "UINT32 result = m_regs[breg] >> (m_regs[creg]&0x1f);","m_regs[breg] = result;" )
EmitGroup0f(f, "0f_1b", "ASL1_S","UINT32 result = m_regs[creg] << 1;",                  "m_regs[breg] = result;" )


#  xxx_S b, b, u5 format opcodes
EmitGroup17(f, "17_00", "ASL_S",  "m_regs[breg] = m_regs[breg] << (u&0x1f);" )
EmitGroup17(f, "17_01", "LSR_S",  "m_regs[breg] = m_regs[breg] >> (u&0x1f);" )
EmitGroup17(f, "17_02", "ASR_S",  "INT32 temp = (INT32)m_regs[breg]; m_regs[breg] = temp >> (u&0x1f); // treat it as a signed value, so sign extension occurs during shift" )
EmitGroup17(f, "17_03", "SUB_S",  "m_regs[breg] = m_regs[breg] - u;" )
EmitGroup17(f, "17_04", "BSET_S", "m_regs[breg] = m_regs[breg] | (1 << (u & 0x1f));" )

EmitGroup17(f, "17_06", "BMSK_S", "m_regs[breg] = m_regs[breg] | ((1 << (u + 1)) - 1);" )




