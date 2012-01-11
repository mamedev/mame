#include "emu.h"
#include "arm7core.h"
#include "arm7thmb.h"
#include "arm7help.h"

arm7thumb_ophandler thumb_handler[0x10] =
{
	thumbblock00,
	thumbblock01,
	thumbblock02,
	thumbblock03,
	thumbblock04,
	thumbblock05,
	thumbblock06,
	thumbblock07,
	thumbblock08,
	thumbblock09,
	thumbblock0a,
	thumbblock0b,
	thumbblock0c,
	thumbblock0d,
	thumbblock0e,
	thumbblock0f,
};


const void thumbblock00(arm_state *cpustate, UINT32 pc, UINT32 insn) /* Shift operations */
{
	UINT32 rs, rd, rrs;
	INT32 offs;

    SET_CPSR(GET_CPSR & ~(N_MASK | Z_MASK));
    if (insn & THUMB_SHIFT_R) /* Shift right */
    {
        rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
        rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
        rrs = GET_REGISTER(cpustate, rs);
        offs = (insn & THUMB_SHIFT_AMT) >> THUMB_SHIFT_AMT_SHIFT;
        if (offs != 0)
        {
            SET_REGISTER(cpustate, rd, rrs >> offs);
            if (rrs & (1 << (offs - 1)))
            {
                SET_CPSR(GET_CPSR | C_MASK);
            }
            else
            {
                SET_CPSR(GET_CPSR & ~C_MASK);
            }
        }
        else
        {
            SET_REGISTER(cpustate, rd, 0);
            if (rrs & 0x80000000)
            {
                SET_CPSR(GET_CPSR | C_MASK);
            }
            else
            {
                SET_CPSR(GET_CPSR & ~C_MASK);
            }
        }
        SET_CPSR(GET_CPSR & ~(Z_MASK | N_MASK));
        SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(cpustate, rd)));
        R15 += 2;
    }
    else /* Shift left */
    {
        rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
        rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
        rrs = GET_REGISTER(cpustate, rs);
        offs = (insn & THUMB_SHIFT_AMT) >> THUMB_SHIFT_AMT_SHIFT;
        if (offs != 0)
        {
            SET_REGISTER(cpustate, rd, rrs << offs);
            if (rrs & (1 << (31 - (offs - 1))))
            {
                SET_CPSR(GET_CPSR | C_MASK);
            }
            else
            {
                SET_CPSR(GET_CPSR & ~C_MASK);
            }
        }
        else
        {
            SET_REGISTER(cpustate, rd, rrs);
        }
        SET_CPSR(GET_CPSR & ~(Z_MASK | N_MASK));
        SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(cpustate, rd)));
        R15 += 2;
    }
}

const void thumbblock01(arm_state *cpustate, UINT32 pc, UINT32 insn) /* Arithmetic */
{
	UINT32 rn, rs, rd, imm, rrs;
	INT32 offs;

    if (insn & THUMB_INSN_ADDSUB)
    {
        switch ((insn & THUMB_ADDSUB_TYPE) >> THUMB_ADDSUB_TYPE_SHIFT)
        {
            case 0x0: /* ADD Rd, Rs, Rn */
                rn = GET_REGISTER(cpustate, (insn & THUMB_ADDSUB_RNIMM) >> THUMB_ADDSUB_RNIMM_SHIFT);
                rs = GET_REGISTER(cpustate, (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT);
                rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
                SET_REGISTER(cpustate, rd, rs + rn);
                HandleThumbALUAddFlags(GET_REGISTER(cpustate, rd), rs, rn);
                break;
            case 0x1: /* SUB Rd, Rs, Rn */
                rn = GET_REGISTER(cpustate, (insn & THUMB_ADDSUB_RNIMM) >> THUMB_ADDSUB_RNIMM_SHIFT);
                rs = GET_REGISTER(cpustate, (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT);
                rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
                SET_REGISTER(cpustate, rd, rs - rn);
                HandleThumbALUSubFlags(GET_REGISTER(cpustate, rd), rs, rn);
                break;
            case 0x2: /* ADD Rd, Rs, #imm */
                imm = (insn & THUMB_ADDSUB_RNIMM) >> THUMB_ADDSUB_RNIMM_SHIFT;
                rs = GET_REGISTER(cpustate, (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT);
                rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
                SET_REGISTER(cpustate, rd, rs + imm);
                HandleThumbALUAddFlags(GET_REGISTER(cpustate, rd), rs, imm);
                break;
            case 0x3: /* SUB Rd, Rs, #imm */
                imm = (insn & THUMB_ADDSUB_RNIMM) >> THUMB_ADDSUB_RNIMM_SHIFT;
                rs = GET_REGISTER(cpustate, (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT);
                rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
                SET_REGISTER(cpustate, rd, rs - imm);
                HandleThumbALUSubFlags(GET_REGISTER(cpustate, rd), rs,imm);
                break;
            default:
                fatalerror("%08x: G1 Undefined Thumb instruction: %04x\n", pc, insn);
                R15 += 2;
                break;
        }
    }
    else
    {
        /* ASR.. */
        //if (insn & THUMB_SHIFT_R) /* Shift right */
        {
            rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
            rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
            rrs = GET_REGISTER(cpustate, rs);
            offs = (insn & THUMB_SHIFT_AMT) >> THUMB_SHIFT_AMT_SHIFT;
            if (offs == 0)
            {
                offs = 32;
            }
            if (offs >= 32)
            {
                if (rrs >> 31)
                {
                    SET_CPSR(GET_CPSR | C_MASK);
                }
                else
                {
                    SET_CPSR(GET_CPSR & ~C_MASK);
                }
                SET_REGISTER(cpustate, rd, (rrs & 0x80000000) ? 0xFFFFFFFF : 0x00000000);
            }
            else
            {
                if ((rrs >> (offs - 1)) & 1)
                {
                    SET_CPSR(GET_CPSR | C_MASK);
                }
                else
                {
                    SET_CPSR(GET_CPSR & ~C_MASK);
                }
                SET_REGISTER(cpustate, rd,
                                (rrs & 0x80000000)
                                ? ((0xFFFFFFFF << (32 - offs)) | (rrs >> offs))
                                : (rrs >> offs));
            }
            SET_CPSR(GET_CPSR & ~(N_MASK | Z_MASK));
            SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(cpustate, rd)));
            R15 += 2;
        }
    }
}

const void thumbblock02(arm_state *cpustate, UINT32 pc, UINT32 insn) /* CMP / MOV */
{
	UINT32 rn, rd, op2;

    if (insn & THUMB_INSN_CMP)
    {
        rn = GET_REGISTER(cpustate, (insn & THUMB_INSN_IMM_RD) >> THUMB_INSN_IMM_RD_SHIFT);
        op2 = insn & THUMB_INSN_IMM;
        rd = rn - op2;
        HandleThumbALUSubFlags(rd, rn, op2);
        //mame_printf_debug("%08x: xxx Thumb instruction: CMP R%d (%08x), %02x (N=%d, Z=%d, C=%d, V=%d)\n", pc, (insn & THUMB_INSN_IMM_RD) >> THUMB_INSN_IMM_RD_SHIFT, GET_REGISTER(cpustate, (insn & THUMB_INSN_IMM_RD) >> THUMB_INSN_IMM_RD_SHIFT), op2, N_IS_SET(GET_CPSR) ? 1 : 0, Z_IS_SET(GET_CPSR) ? 1 : 0, C_IS_SET(GET_CPSR) ? 1 : 0, V_IS_SET(GET_CPSR) ? 1 : 0);
    }
    else
    {
        rd = (insn & THUMB_INSN_IMM_RD) >> THUMB_INSN_IMM_RD_SHIFT;
        op2 = (insn & THUMB_INSN_IMM);
        SET_REGISTER(cpustate, rd, op2);
        SET_CPSR(GET_CPSR & ~(Z_MASK | N_MASK));
        SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(cpustate, rd)));
        R15 += 2;
    }
}

const void thumbblock03(arm_state *cpustate, UINT32 pc, UINT32 insn) /* ADD/SUB immediate */
{
	UINT32 rn, rd, op2;

    if (insn & THUMB_INSN_SUB) /* SUB Rd, #Offset8 */
    {
        rn = GET_REGISTER(cpustate, (insn & THUMB_INSN_IMM_RD) >> THUMB_INSN_IMM_RD_SHIFT);
        op2 = insn & THUMB_INSN_IMM;
        //mame_printf_debug("%08x:  Thumb instruction: SUB R%d, %02x\n", pc, (insn & THUMB_INSN_IMM_RD) >> THUMB_INSN_IMM_RD_SHIFT, op2);
        rd = rn - op2;
        SET_REGISTER(cpustate, (insn & THUMB_INSN_IMM_RD) >> THUMB_INSN_IMM_RD_SHIFT, rd);
        HandleThumbALUSubFlags(rd, rn, op2);
    }
    else /* ADD Rd, #Offset8 */
    {
        rn = GET_REGISTER(cpustate, (insn & THUMB_INSN_IMM_RD) >> THUMB_INSN_IMM_RD_SHIFT);
        op2 = insn & THUMB_INSN_IMM;
        rd = rn + op2;
        //mame_printf_debug("%08x:  Thumb instruction: ADD R%d, %02x\n", pc, (insn & THUMB_INSN_IMM_RD) >> THUMB_INSN_IMM_RD_SHIFT, op2);
        SET_REGISTER(cpustate, (insn & THUMB_INSN_IMM_RD) >> THUMB_INSN_IMM_RD_SHIFT, rd);
        HandleThumbALUAddFlags(rd, rn, op2);
    }
}

const void thumbblock04(arm_state *cpustate, UINT32 pc, UINT32 insn) /* Rd & Rm instructions */
{
	UINT32 readword, addr;
	UINT32 rn, rs, rd, op2, imm, rrs, rrd;
	INT32 offs;

    switch ((insn & THUMB_GROUP4_TYPE) >> THUMB_GROUP4_TYPE_SHIFT)
    {
        case 0x0:
            switch ((insn & THUMB_ALUOP_TYPE) >> THUMB_ALUOP_TYPE_SHIFT)
            {
                case 0x0: /* AND Rd, Rs */
                    rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
                    rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
                    SET_REGISTER(cpustate, rd, GET_REGISTER(cpustate, rd) & GET_REGISTER(cpustate, rs));
                    SET_CPSR(GET_CPSR & ~(Z_MASK | N_MASK));
                    SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(cpustate, rd)));
                    R15 += 2;
                    break;
                case 0x1: /* EOR Rd, Rs */
                    rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
                    rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
                    SET_REGISTER(cpustate, rd, GET_REGISTER(cpustate, rd) ^ GET_REGISTER(cpustate, rs));
                    SET_CPSR(GET_CPSR & ~(Z_MASK | N_MASK));
                    SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(cpustate, rd)));
                    R15 += 2;
                    break;
                case 0x2: /* LSL Rd, Rs */
                    rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
                    rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
                    rrd = GET_REGISTER(cpustate, rd);
                    offs = GET_REGISTER(cpustate, rs) & 0x000000ff;
                    if (offs > 0)
                    {
                        if (offs < 32)
                        {
                            SET_REGISTER(cpustate, rd, rrd << offs);
                            if (rrd & (1 << (31 - (offs - 1))))
                            {
                                SET_CPSR(GET_CPSR | C_MASK);
                            }
                            else
                            {
                                SET_CPSR(GET_CPSR & ~C_MASK);
                            }
                        }
                        else if (offs == 32)
                        {
                            SET_REGISTER(cpustate, rd, 0);
                            if (rrd & 1)
                            {
                                SET_CPSR(GET_CPSR | C_MASK);
                            }
                            else
                            {
                                SET_CPSR(GET_CPSR & ~C_MASK);
                            }
                        }
                        else
                        {
                            SET_REGISTER(cpustate, rd, 0);
                            SET_CPSR(GET_CPSR & ~C_MASK);
                        }
                    }
                    SET_CPSR(GET_CPSR & ~(Z_MASK | N_MASK));
                    SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(cpustate, rd)));
                    R15 += 2;
                    break;
                case 0x3: /* LSR Rd, Rs */
                    rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
                    rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
                    rrd = GET_REGISTER(cpustate, rd);
                    offs = GET_REGISTER(cpustate, rs) & 0x000000ff;
                    if (offs >  0)
                    {
                        if (offs < 32)
                        {
                            SET_REGISTER(cpustate, rd, rrd >> offs);
                            if (rrd & (1 << (offs - 1)))
                            {
                                SET_CPSR(GET_CPSR | C_MASK);
                            }
                            else
                            {
                                SET_CPSR(GET_CPSR & ~C_MASK);
                            }
                        }
                        else if (offs == 32)
                        {
                            SET_REGISTER(cpustate, rd, 0);
                            if (rrd & 0x80000000)
                            {
                                SET_CPSR(GET_CPSR | C_MASK);
                            }
                            else
                            {
                                SET_CPSR(GET_CPSR & ~C_MASK);
                            }
                        }
                        else
                        {
                            SET_REGISTER(cpustate, rd, 0);
                            SET_CPSR(GET_CPSR & ~C_MASK);
                        }
                    }
                    SET_CPSR(GET_CPSR & ~(Z_MASK | N_MASK));
                    SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(cpustate, rd)));
                    R15 += 2;
                    break;
                case 0x4: /* ASR Rd, Rs */
                    rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
                    rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
                    rrs = GET_REGISTER(cpustate, rs)&0xff;
                    rrd = GET_REGISTER(cpustate, rd);
                    if (rrs != 0)
                    {
                        if (rrs >= 32)
                        {
                            if (rrd >> 31)
                            {
                                SET_CPSR(GET_CPSR | C_MASK);
                            }
                            else
                            {
                                SET_CPSR(GET_CPSR & ~C_MASK);
                            }
                            SET_REGISTER(cpustate, rd, (GET_REGISTER(cpustate, rd) & 0x80000000) ? 0xFFFFFFFF : 0x00000000);
                        }
                        else
                        {
                            if ((rrd >> (rrs-1)) & 1)
                            {
                                SET_CPSR(GET_CPSR | C_MASK);
                            }
                            else
                            {
                                SET_CPSR(GET_CPSR & ~C_MASK);
                            }
                            SET_REGISTER(cpustate, rd, (rrd & 0x80000000)
                                            ? ((0xFFFFFFFF << (32 - rrs)) | (rrd >> rrs))
                                            : (rrd >> rrs));
                        }
                    }
                    SET_CPSR(GET_CPSR & ~(N_MASK | Z_MASK));
                    SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(cpustate, rd)));
                    R15 += 2;
                    break;
                case 0x5: /* ADC Rd, Rs */
                    rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
                    rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
                    op2=(GET_CPSR & C_MASK) ? 1 : 0;
                    rn=GET_REGISTER(cpustate, rd) + GET_REGISTER(cpustate, rs) + op2;
                    HandleThumbALUAddFlags(rn, GET_REGISTER(cpustate, rd), (GET_REGISTER(cpustate, rs))); // ?
                    SET_REGISTER(cpustate, rd, rn);
                    break;
                case 0x6: /* SBC Rd, Rs */
                    rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
                    rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
                    op2=(GET_CPSR & C_MASK) ? 0 : 1;
                    rn=GET_REGISTER(cpustate, rd) - GET_REGISTER(cpustate, rs) - op2;
                    HandleThumbALUSubFlags(rn, GET_REGISTER(cpustate, rd), (GET_REGISTER(cpustate, rs))); //?
                    SET_REGISTER(cpustate, rd, rn);
                    break;
                case 0x7: /* ROR Rd, Rs */
                    rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
                    rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
                    rrd = GET_REGISTER(cpustate, rd);
                    imm = GET_REGISTER(cpustate, rs) & 0x0000001f;
                    SET_REGISTER(cpustate, rd, (rrd >> imm) | (rrd << (32 - imm)));
                    if (rrd & (1 << (imm - 1)))
                    {
                        SET_CPSR(GET_CPSR | C_MASK);
                    }
                    else
                    {
                        SET_CPSR(GET_CPSR & ~C_MASK);
                    }
                    SET_CPSR(GET_CPSR & ~(Z_MASK | N_MASK));
                    SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(cpustate, rd)));
                    R15 += 2;
                    break;
                case 0x8: /* TST Rd, Rs */
                    rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
                    rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
                    SET_CPSR(GET_CPSR & ~(Z_MASK | N_MASK));
                    SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(cpustate, rd) & GET_REGISTER(cpustate, rs)));
                    R15 += 2;
                    break;
                case 0x9: /* NEG Rd, Rs */
                    rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
                    rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
                    rrs = GET_REGISTER(cpustate, rs);
                    rn = 0 - rrs;
                    SET_REGISTER(cpustate, rd, rn);
                    HandleThumbALUSubFlags(GET_REGISTER(cpustate, rd), 0, rrs);
                    break;
                case 0xa: /* CMP Rd, Rs */
                    rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
                    rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
                    rn = GET_REGISTER(cpustate, rd) - GET_REGISTER(cpustate, rs);
                    HandleThumbALUSubFlags(rn, GET_REGISTER(cpustate, rd), GET_REGISTER(cpustate, rs));
                    break;
                case 0xb: /* CMN Rd, Rs - check flags, add dasm */
                    rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
                    rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
                    rn = GET_REGISTER(cpustate, rd) + GET_REGISTER(cpustate, rs);
                    HandleThumbALUAddFlags(rn, GET_REGISTER(cpustate, rd), GET_REGISTER(cpustate, rs));
                    break;
                case 0xc: /* ORR Rd, Rs */
                    rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
                    rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
                    SET_REGISTER(cpustate, rd, GET_REGISTER(cpustate, rd) | GET_REGISTER(cpustate, rs));
                    SET_CPSR(GET_CPSR & ~(Z_MASK | N_MASK));
                    SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(cpustate, rd)));
                    R15 += 2;
                    break;
                case 0xd: /* MUL Rd, Rs */
                    rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
                    rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
                    rn = GET_REGISTER(cpustate, rd) * GET_REGISTER(cpustate, rs);
                    SET_CPSR(GET_CPSR & ~(Z_MASK | N_MASK));
                    SET_REGISTER(cpustate, rd, rn);
                    SET_CPSR(GET_CPSR | HandleALUNZFlags(rn));
                    R15 += 2;
                    break;
                case 0xe: /* BIC Rd, Rs */
                    rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
                    rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
                    SET_REGISTER(cpustate, rd, GET_REGISTER(cpustate, rd) & (~GET_REGISTER(cpustate, rs)));
                    SET_CPSR(GET_CPSR & ~(Z_MASK | N_MASK));
                    SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(cpustate, rd)));
                    R15 += 2;
                    break;
                case 0xf: /* MVN Rd, Rs */
                    rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
                    rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
                    op2 = GET_REGISTER(cpustate, rs);
                    SET_REGISTER(cpustate, rd, ~op2);
                    SET_CPSR(GET_CPSR & ~(Z_MASK | N_MASK));
                    SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(cpustate, rd)));
                    R15 += 2;
                    break;
                default:
                    fatalerror("%08x: G4-0 Undefined Thumb instruction: %04x %x\n", pc, insn, (insn & THUMB_ALUOP_TYPE) >> THUMB_ALUOP_TYPE_SHIFT);
                    R15 += 2;
                    break;
            }
            break;
        case 0x1:
            switch ((insn & THUMB_HIREG_OP) >> THUMB_HIREG_OP_SHIFT)
            {
                case 0x0: /* ADD Rd, Rs */
                    rs = (insn & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT;
                    rd = insn & THUMB_HIREG_RD;
                    switch ((insn & THUMB_HIREG_H) >> THUMB_HIREG_H_SHIFT)
                    {
                        case 0x1: /* ADD Rd, HRs */
                            SET_REGISTER(cpustate, rd, GET_REGISTER(cpustate, rd) + GET_REGISTER(cpustate, rs+8));
                            // emulate the effects of pre-fetch
                            if (rs == 7)
                            {
                                SET_REGISTER(cpustate, rd, GET_REGISTER(cpustate, rd) + 4);
                            }
                            break;
                        case 0x2: /* ADD HRd, Rs */
                            SET_REGISTER(cpustate, rd+8, GET_REGISTER(cpustate, rd+8) + GET_REGISTER(cpustate, rs));
                            if (rd == 7)
                            {
                                R15 += 2;
                            }
                            break;
                        case 0x3: /* Add HRd, HRs */
                            SET_REGISTER(cpustate, rd+8, GET_REGISTER(cpustate, rd+8) + GET_REGISTER(cpustate, rs+8));
                            // emulate the effects of pre-fetch
                            if (rs == 7)
                            {
                                SET_REGISTER(cpustate, rd+8, GET_REGISTER(cpustate, rd+8) + 4);
                            }
                            if (rd == 7)
                            {
                                R15 += 2;
                            }
                            break;
                        default:
                            fatalerror("%08x: G4-1-0 Undefined Thumb instruction: %04x %x\n", pc, insn, (insn & THUMB_HIREG_H) >> THUMB_HIREG_H_SHIFT);
                            break;
                    }
                    R15 += 2;
                    break;
                case 0x1: /* CMP */
                    switch ((insn & THUMB_HIREG_H) >> THUMB_HIREG_H_SHIFT)
                    {
                        case 0x0: /* CMP Rd, Rs */
                            rs = GET_REGISTER(cpustate, ((insn & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT));
                            rd = GET_REGISTER(cpustate, insn & THUMB_HIREG_RD);
                            rn = rd - rs;
                            HandleThumbALUSubFlags(rn, rd, rs);
                            break;
                        case 0x1: /* CMP Rd, Hs */
                            rs = GET_REGISTER(cpustate, ((insn & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT) + 8);
                            rd = GET_REGISTER(cpustate, insn & THUMB_HIREG_RD);
                            rn = rd - rs;
                            HandleThumbALUSubFlags(rn, rd, rs);
                            break;
                        case 0x2: /* CMP Hd, Rs */
                            rs = GET_REGISTER(cpustate, ((insn & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT));
                            rd = GET_REGISTER(cpustate, (insn & THUMB_HIREG_RD) + 8);
                            rn = rd - rs;
                            HandleThumbALUSubFlags(rn, rd, rs);
                            break;
                        case 0x3: /* CMP Hd, Hs */
                            rs = GET_REGISTER(cpustate, ((insn & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT) + 8);
                            rd = GET_REGISTER(cpustate, (insn & THUMB_HIREG_RD) + 8);
                            rn = rd - rs;
                            HandleThumbALUSubFlags(rn, rd, rs);
                            break;
                        default:
                            fatalerror("%08x: G4-1 Undefined Thumb instruction: %04x %x\n", pc, insn, (insn & THUMB_HIREG_H) >> THUMB_HIREG_H_SHIFT);
                            R15 += 2;
                            break;
                    }
                    break;
                case 0x2: /* MOV */
                    switch ((insn & THUMB_HIREG_H) >> THUMB_HIREG_H_SHIFT)
                    {
                        case 0x0:       // MOV Rd, Rs (undefined)
                            // "The action of H1 = 0, H2 = 0 for Op = 00 (ADD), Op = 01 (CMP) and Op = 10 (MOV) is undefined, and should not be used."
                            rs = (insn & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT;
                            rd = insn & THUMB_HIREG_RD;
                            SET_REGISTER(cpustate, rd, GET_REGISTER(cpustate, rs));
                            R15 += 2;
                            break;
                        case 0x1:       // MOV Rd, Hs
                            rs = (insn & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT;
                            rd = insn & THUMB_HIREG_RD;
                            if (rs == 7)
                            {
                                SET_REGISTER(cpustate, rd, GET_REGISTER(cpustate, rs + 8) + 4);
                            }
                            else
                            {
                                SET_REGISTER(cpustate, rd, GET_REGISTER(cpustate, rs + 8));
                            }
                            R15 += 2;
                            break;
                        case 0x2:       // MOV Hd, Rs
                            rs = (insn & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT;
                            rd = insn & THUMB_HIREG_RD;
                            SET_REGISTER(cpustate, rd + 8, GET_REGISTER(cpustate, rs));
                            if (rd != 7)
                            {
                                R15 += 2;
                            }
                            else
                            {
                                R15 &= ~1;
                            }
                            break;
                        case 0x3:       // MOV Hd, Hs
                            rs = (insn & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT;
                            rd = insn & THUMB_HIREG_RD;
                            if (rs == 7)
                            {
                                SET_REGISTER(cpustate, rd + 8, GET_REGISTER(cpustate, rs+8)+4);
                            }
                            else
                            {
                                SET_REGISTER(cpustate, rd + 8, GET_REGISTER(cpustate, rs+8));
                            }
                            if (rd != 7)
                            {
                                R15 += 2;
                            }
                            if (rd == 7)
                            {
                                R15 &= ~1;
                            }
                            break;
                        default:
                            fatalerror("%08x: G4-2 Undefined Thumb instruction: %04x (%x)\n", pc, insn, (insn & THUMB_HIREG_H) >> THUMB_HIREG_H_SHIFT);
                            R15 += 2;
                            break;
                    }
                    break;
                case 0x3:
                    switch ((insn & THUMB_HIREG_H) >> THUMB_HIREG_H_SHIFT)
                    {
                        case 0x0:
                            rd = (insn & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT;
                            addr = GET_REGISTER(cpustate, rd);
                            if (addr & 1)
                            {
                                addr &= ~1;
                            }
                            else
                            {
                                SET_CPSR(GET_CPSR & ~T_MASK);
                                if (addr & 2)
                                {
                                    addr += 2;
                                }
                            }
                            R15 = addr;
                            break;
                        case 0x1:
                            addr = GET_REGISTER(cpustate, ((insn & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT) + 8);
                            if ((((insn & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT) + 8) == 15)
                            {
                                addr += 2;
                            }
                            if (addr & 1)
                            {
                                addr &= ~1;
                            }
                            else
                            {
                                SET_CPSR(GET_CPSR & ~T_MASK);
                                if (addr & 2)
                                {
                                    addr += 2;
                                }
                            }
                            R15 = addr;
                            break;
                        default:
                            fatalerror("%08x: G4-3 Undefined Thumb instruction: %04x\n", pc, insn);
                            R15 += 2;
                            break;
                    }
                    break;
                default:
                    fatalerror("%08x: G4-x Undefined Thumb instruction: %04x\n", pc, insn);
                    R15 += 2;
                    break;
            }
            break;
        case 0x2:
        case 0x3:
            readword = READ32((R15 & ~2) + 4 + ((insn & THUMB_INSN_IMM) << 2));
            SET_REGISTER(cpustate, (insn & THUMB_INSN_IMM_RD) >> THUMB_INSN_IMM_RD_SHIFT, readword);
            R15 += 2;
            break;
        default:
            fatalerror("%08x: G4-y Undefined Thumb instruction: %04x\n", pc, insn);
            R15 += 2;
            break;
    }
}

const void thumbblock05(arm_state *cpustate, UINT32 pc, UINT32 insn) /* LDR* STR* */
{
	UINT32 addr;
	UINT32 rm, rn, rd, op2;

    switch ((insn & THUMB_GROUP5_TYPE) >> THUMB_GROUP5_TYPE_SHIFT)
    {
        case 0x0: /* STR Rd, [Rn, Rm] */
            rm = (insn & THUMB_GROUP5_RM) >> THUMB_GROUP5_RM_SHIFT;
            rn = (insn & THUMB_GROUP5_RN) >> THUMB_GROUP5_RN_SHIFT;
            rd = (insn & THUMB_GROUP5_RD) >> THUMB_GROUP5_RD_SHIFT;
            addr = GET_REGISTER(cpustate, rn) + GET_REGISTER(cpustate, rm);
            WRITE32(addr, GET_REGISTER(cpustate, rd));
            R15 += 2;
            break;
        case 0x1: /* STRH Rd, [Rn, Rm] */
            rm = (insn & THUMB_GROUP5_RM) >> THUMB_GROUP5_RM_SHIFT;
            rn = (insn & THUMB_GROUP5_RN) >> THUMB_GROUP5_RN_SHIFT;
            rd = (insn & THUMB_GROUP5_RD) >> THUMB_GROUP5_RD_SHIFT;
            addr = GET_REGISTER(cpustate, rn) + GET_REGISTER(cpustate, rm);
            WRITE16(addr, GET_REGISTER(cpustate, rd));
            R15 += 2;
            break;
        case 0x2: /* STRB Rd, [Rn, Rm] */
            rm = (insn & THUMB_GROUP5_RM) >> THUMB_GROUP5_RM_SHIFT;
            rn = (insn & THUMB_GROUP5_RN) >> THUMB_GROUP5_RN_SHIFT;
            rd = (insn & THUMB_GROUP5_RD) >> THUMB_GROUP5_RD_SHIFT;
            addr = GET_REGISTER(cpustate, rn) + GET_REGISTER(cpustate, rm);
            WRITE8(addr, GET_REGISTER(cpustate, rd));
            R15 += 2;
            break;
        case 0x3: /* LDSB Rd, [Rn, Rm] todo, add dasm */
            rm = (insn & THUMB_GROUP5_RM) >> THUMB_GROUP5_RM_SHIFT;
            rn = (insn & THUMB_GROUP5_RN) >> THUMB_GROUP5_RN_SHIFT;
            rd = (insn & THUMB_GROUP5_RD) >> THUMB_GROUP5_RD_SHIFT;
            addr = GET_REGISTER(cpustate, rn) + GET_REGISTER(cpustate, rm);
            op2 = READ8(addr);
            if (op2 & 0x00000080)
            {
                op2 |= 0xffffff00;
            }
            SET_REGISTER(cpustate, rd, op2);
            R15 += 2;
            break;
        case 0x4: /* LDR Rd, [Rn, Rm] */
            rm = (insn & THUMB_GROUP5_RM) >> THUMB_GROUP5_RM_SHIFT;
            rn = (insn & THUMB_GROUP5_RN) >> THUMB_GROUP5_RN_SHIFT;
            rd = (insn & THUMB_GROUP5_RD) >> THUMB_GROUP5_RD_SHIFT;
            addr = GET_REGISTER(cpustate, rn) + GET_REGISTER(cpustate, rm);
            op2 = READ32(addr);
            SET_REGISTER(cpustate, rd, op2);
            R15 += 2;
            break;
        case 0x5: /* LDRH Rd, [Rn, Rm] */
            rm = (insn & THUMB_GROUP5_RM) >> THUMB_GROUP5_RM_SHIFT;
            rn = (insn & THUMB_GROUP5_RN) >> THUMB_GROUP5_RN_SHIFT;
            rd = (insn & THUMB_GROUP5_RD) >> THUMB_GROUP5_RD_SHIFT;
            addr = GET_REGISTER(cpustate, rn) + GET_REGISTER(cpustate, rm);
            op2 = READ16(addr);
            SET_REGISTER(cpustate, rd, op2);
            R15 += 2;
            break;
        case 0x6: /* LDRB Rd, [Rn, Rm] */
            rm = (insn & THUMB_GROUP5_RM) >> THUMB_GROUP5_RM_SHIFT;
            rn = (insn & THUMB_GROUP5_RN) >> THUMB_GROUP5_RN_SHIFT;
            rd = (insn & THUMB_GROUP5_RD) >> THUMB_GROUP5_RD_SHIFT;
            addr = GET_REGISTER(cpustate, rn) + GET_REGISTER(cpustate, rm);
            op2 = READ8(addr);
            SET_REGISTER(cpustate, rd, op2);
            R15 += 2;
            break;
        case 0x7: /* LDSH Rd, [Rn, Rm] */
            rm = (insn & THUMB_GROUP5_RM) >> THUMB_GROUP5_RM_SHIFT;
            rn = (insn & THUMB_GROUP5_RN) >> THUMB_GROUP5_RN_SHIFT;
            rd = (insn & THUMB_GROUP5_RD) >> THUMB_GROUP5_RD_SHIFT;
            addr = GET_REGISTER(cpustate, rn) + GET_REGISTER(cpustate, rm);
            op2 = READ16(addr);
            if (op2 & 0x00008000)
            {
                op2 |= 0xffff0000;
            }
            SET_REGISTER(cpustate, rd, op2);
            R15 += 2;
            break;
        default:
            fatalerror("%08x: G5 Undefined Thumb instruction: %04x\n", pc, insn);
            R15 += 2;
            break;
    }
}

const void thumbblock06(arm_state *cpustate, UINT32 pc, UINT32 insn) /* Word Store w/ Immediate Offset */
{
	UINT32 rn, rd;
	INT32 offs;

    if (insn & THUMB_LSOP_L) /* Load */
    {
        rn = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
        rd = insn & THUMB_ADDSUB_RD;
        offs = ((insn & THUMB_LSOP_OFFS) >> THUMB_LSOP_OFFS_SHIFT) << 2;
        SET_REGISTER(cpustate, rd, READ32(GET_REGISTER(cpustate, rn) + offs)); // fix
        R15 += 2;
    }
    else /* Store */
    {
        rn = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
        rd = insn & THUMB_ADDSUB_RD;
        offs = ((insn & THUMB_LSOP_OFFS) >> THUMB_LSOP_OFFS_SHIFT) << 2;
        WRITE32(GET_REGISTER(cpustate, rn) + offs, GET_REGISTER(cpustate, rd));
        R15 += 2;
    }
}

const void thumbblock07(arm_state *cpustate, UINT32 pc, UINT32 insn) /* Byte Store w/ Immeidate Offset */
{
	UINT32 rn, rd;
	INT32 offs;

    if (insn & THUMB_LSOP_L) /* Load */
    {
        rn = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
        rd = insn & THUMB_ADDSUB_RD;
        offs = (insn & THUMB_LSOP_OFFS) >> THUMB_LSOP_OFFS_SHIFT;
        SET_REGISTER(cpustate, rd, READ8(GET_REGISTER(cpustate, rn) + offs));
        R15 += 2;
    }
    else /* Store */
    {
        rn = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
        rd = insn & THUMB_ADDSUB_RD;
        offs = (insn & THUMB_LSOP_OFFS) >> THUMB_LSOP_OFFS_SHIFT;
        WRITE8(GET_REGISTER(cpustate, rn) + offs, GET_REGISTER(cpustate, rd));
        R15 += 2;
    }
}

const void thumbblock08(arm_state *cpustate, UINT32 pc, UINT32 insn) /* Load/Store Halfword */
{
	UINT32 rs, rd, imm;
	
    if (insn & THUMB_HALFOP_L) /* Load */
    {
        imm = (insn & THUMB_HALFOP_OFFS) >> THUMB_HALFOP_OFFS_SHIFT;
        rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
        rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
        SET_REGISTER(cpustate, rd, READ16(GET_REGISTER(cpustate, rs) + (imm << 1)));
        R15 += 2;
    }
    else /* Store */
    {
        imm = (insn & THUMB_HALFOP_OFFS) >> THUMB_HALFOP_OFFS_SHIFT;
        rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
        rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
        WRITE16(GET_REGISTER(cpustate, rs) + (imm << 1), GET_REGISTER(cpustate, rd));
        R15 += 2;
    }
}

const void thumbblock09(arm_state *cpustate, UINT32 pc, UINT32 insn) /* Stack-Relative Load/Store */
{
	UINT32 readword;
	UINT32 rd;
	INT32 offs;

    if (insn & THUMB_STACKOP_L)
    {
        rd = (insn & THUMB_STACKOP_RD) >> THUMB_STACKOP_RD_SHIFT;
        offs = (UINT8)(insn & THUMB_INSN_IMM);
        readword = READ32(GET_REGISTER(cpustate, 13) + ((UINT32)offs << 2));
        SET_REGISTER(cpustate, rd, readword);
        R15 += 2;
    }
    else
    {
        rd = (insn & THUMB_STACKOP_RD) >> THUMB_STACKOP_RD_SHIFT;
        offs = (UINT8)(insn & THUMB_INSN_IMM);
        WRITE32(GET_REGISTER(cpustate, 13) + ((UINT32)offs << 2), GET_REGISTER(cpustate, rd));
        R15 += 2;
    }
}

const void thumbblock0a(arm_state *cpustate, UINT32 pc, UINT32 insn) /* Get relative address */
{
	UINT32 rd;
	INT32 offs;

    if (insn & THUMB_RELADDR_SP) /* ADD Rd, SP, #nn */
    {
        rd = (insn & THUMB_RELADDR_RD) >> THUMB_RELADDR_RD_SHIFT;
        offs = (UINT8)(insn & THUMB_INSN_IMM) << 2;
        SET_REGISTER(cpustate, rd, GET_REGISTER(cpustate, 13) + offs);
        R15 += 2;
    }
    else /* ADD Rd, PC, #nn */
    {
        rd = (insn & THUMB_RELADDR_RD) >> THUMB_RELADDR_RD_SHIFT;
        offs = (UINT8)(insn & THUMB_INSN_IMM) << 2;
        SET_REGISTER(cpustate, rd, ((R15 + 4) & ~2) + offs);
        R15 += 2;
    }
}

const void thumbblock0b(arm_state *cpustate, UINT32 pc, UINT32 insn) /* Stack-Related Opcodes */
{
	UINT32 addr;
	INT32 offs;

    switch ((insn & THUMB_STACKOP_TYPE) >> THUMB_STACKOP_TYPE_SHIFT)
    {
        case 0x0: /* ADD SP, #imm */
            addr = (insn & THUMB_INSN_IMM);
            addr &= ~THUMB_INSN_IMM_S;
            SET_REGISTER(cpustate, 13, GET_REGISTER(cpustate, 13) + ((insn & THUMB_INSN_IMM_S) ? -(addr << 2) : (addr << 2)));
            R15 += 2;
            break;
        case 0x4: /* PUSH {Rlist} */
            for (offs = 7; offs >= 0; offs--)
            {
                if (insn & (1 << offs))
                {
                    SET_REGISTER(cpustate, 13, GET_REGISTER(cpustate, 13) - 4);
                    WRITE32(GET_REGISTER(cpustate, 13), GET_REGISTER(cpustate, offs));
                }
            }
            R15 += 2;
            break;
        case 0x5: /* PUSH {Rlist}{LR} */
            SET_REGISTER(cpustate, 13, GET_REGISTER(cpustate, 13) - 4);
            WRITE32(GET_REGISTER(cpustate, 13), GET_REGISTER(cpustate, 14));
            for (offs = 7; offs >= 0; offs--)
            {
                if (insn & (1 << offs))
                {
                    SET_REGISTER(cpustate, 13, GET_REGISTER(cpustate, 13) - 4);
                    WRITE32(GET_REGISTER(cpustate, 13), GET_REGISTER(cpustate, offs));
                }
            }
            R15 += 2;
            break;
        case 0xc: /* POP {Rlist} */
            for (offs = 0; offs < 8; offs++)
            {
                if (insn & (1 << offs))
                {
                    SET_REGISTER(cpustate, offs, READ32(GET_REGISTER(cpustate, 13)));
                    SET_REGISTER(cpustate, 13, GET_REGISTER(cpustate, 13) + 4);
                }
            }
            R15 += 2;
            break;
        case 0xd: /* POP {Rlist}{PC} */
            for (offs = 0; offs < 8; offs++)
            {
                if (insn & (1 << offs))
                {
                    SET_REGISTER(cpustate, offs, READ32(GET_REGISTER(cpustate, 13)));
                    SET_REGISTER(cpustate, 13, GET_REGISTER(cpustate, 13) + 4);
                }
            }
            R15 = READ32(GET_REGISTER(cpustate, 13)) & ~1;
            SET_REGISTER(cpustate, 13, GET_REGISTER(cpustate, 13) + 4);
            break;
        default:
            fatalerror("%08x: Gb Undefined Thumb instruction: %04x\n", pc, insn);
            R15 += 2;
            break;
    }
}

const void thumbblock0c(arm_state *cpustate, UINT32 pc, UINT32 insn) /* Multiple Load/Store */
{
	UINT32 rd;
	INT32 offs;

    UINT32 ld_st_address;

    rd = (insn & THUMB_MULTLS_BASE) >> THUMB_MULTLS_BASE_SHIFT;

    // "The address should normally be a word aligned quantity and non-word aligned addresses do not affect the instruction."
    // "However, the bottom 2 bits of the address will appear on A[1:0] and might be interpreted by the memory system."

    // GBA "BB Ball" performs an unaligned read with A[1:0] = 2 and expects A[1] not to be ignored [BP 800B90A,(R4&3)!=0]
    // GBA "Gadget Racers" performs an unaligned read with A[1:0] = 1 and expects A[0] to be ignored [BP B72,(R0&3)!=0]

    ld_st_address = GET_REGISTER(cpustate, rd);

    if (insn & THUMB_MULTLS) /* Load */
    {
        int rd_in_list;

        rd_in_list = insn & (1 << rd);
        for (offs = 0; offs < 8; offs++)
        {
            if (insn & (1 << offs))
            {
                SET_REGISTER(cpustate, offs, READ32(ld_st_address & ~1));
                ld_st_address += 4;
            }
        }
        if (!rd_in_list)
            SET_REGISTER(cpustate, rd, ld_st_address);
        R15 += 2;
    }
    else /* Store */
    {
        for (offs = 0; offs < 8; offs++)
        {
            if (insn & (1 << offs))
            {
                WRITE32(ld_st_address & ~3, GET_REGISTER(cpustate, offs));
                ld_st_address += 4;
            }
        }
        SET_REGISTER(cpustate, rd, ld_st_address);
        R15 += 2;
    }
}

const void thumbblock0d(arm_state *cpustate, UINT32 pc, UINT32 insn) /* Conditional Branch */
{
	INT32 offs;

    offs = (INT8)(insn & THUMB_INSN_IMM);
    switch ((insn & THUMB_COND_TYPE) >> THUMB_COND_TYPE_SHIFT)
    {
        case COND_EQ:
            if (Z_IS_SET(GET_CPSR))
            {
                R15 += 4 + (offs << 1);
            }
            else
            {
                R15 += 2;
            }
            break;
        case COND_NE:
            if (Z_IS_CLEAR(GET_CPSR))
            {
                R15 += 4 + (offs << 1);
            }
            else
            {
                R15 += 2;
            }
            break;
        case COND_CS:
            if (C_IS_SET(GET_CPSR))
            {
                R15 += 4 + (offs << 1);
            }
            else
            {
                R15 += 2;
            }
            break;
        case COND_CC:
            if (C_IS_CLEAR(GET_CPSR))
            {
                R15 += 4 + (offs << 1);
            }
            else
            {
                R15 += 2;
            }
            break;
        case COND_MI:
            if (N_IS_SET(GET_CPSR))
            {
                R15 += 4 + (offs << 1);
            }
            else
            {
                R15 += 2;
            }
            break;
        case COND_PL:
            if (N_IS_CLEAR(GET_CPSR))
            {
                R15 += 4 + (offs << 1);
            }
            else
            {
                R15 += 2;
            }
            break;
        case COND_VS:
            if (V_IS_SET(GET_CPSR))
            {
                R15 += 4 + (offs << 1);
            }
            else
            {
                R15 += 2;
            }
            break;
        case COND_VC:
            if (V_IS_CLEAR(GET_CPSR))
            {
                R15 += 4 + (offs << 1);
            }
            else
            {
                R15 += 2;
            }
            break;
        case COND_HI:
            if (C_IS_SET(GET_CPSR) && Z_IS_CLEAR(GET_CPSR))
            {
                R15 += 4 + (offs << 1);
            }
            else
            {
                R15 += 2;
            }
            break;
        case COND_LS:
            if (C_IS_CLEAR(GET_CPSR) || Z_IS_SET(GET_CPSR))
            {
                R15 += 4 + (offs << 1);
            }
            else
            {
                R15 += 2;
            }
            break;
        case COND_GE:
            if (!(GET_CPSR & N_MASK) == !(GET_CPSR & V_MASK))
            {
                R15 += 4 + (offs << 1);
            }
            else
            {
                R15 += 2;
            }
            break;
        case COND_LT:
            if (!(GET_CPSR & N_MASK) != !(GET_CPSR & V_MASK))
            {
                R15 += 4 + (offs << 1);
            }
            else
            {
                R15 += 2;
            }
            break;
        case COND_GT:
            if (Z_IS_CLEAR(GET_CPSR) && !(GET_CPSR & N_MASK) == !(GET_CPSR & V_MASK))
            {
                R15 += 4 + (offs << 1);
            }
            else
            {
                R15 += 2;
            }
            break;
        case COND_LE:
            if (Z_IS_SET(GET_CPSR) || !(GET_CPSR & N_MASK) != !(GET_CPSR & V_MASK))
            {
                R15 += 4 + (offs << 1);
            }
            else
            {
                R15 += 2;
            }
            break;
        case COND_AL:
            fatalerror("%08x: Undefined Thumb instruction: %04x (ARM9 reserved)\n", pc, insn);
            R15 += 2;
            break;
        case COND_NV:   // SWI (this is sort of a "hole" in the opcode encoding)
            cpustate->pendingSwi = 1;
            ARM7_CHECKIRQ;
            break;
    }
}

const void thumbblock0e(arm_state *cpustate, UINT32 pc, UINT32 insn)  /* B #offs */
{
	UINT32 addr;
	INT32 offs;

    if (insn & THUMB_BLOP_LO)
    {
        addr = GET_REGISTER(cpustate, 14);
        addr += (insn & THUMB_BLOP_OFFS) << 1;
        addr &= 0xfffffffc;
        SET_REGISTER(cpustate, 14, (R15 + 4) | 1);
        R15 = addr;
    }
    else
    {
        offs = (insn & THUMB_BRANCH_OFFS) << 1;
        if (offs & 0x00000800)
        {
            offs |= 0xfffff800;
        }
        R15 += 4 + offs;
    }
}

const void thumbblock0f(arm_state *cpustate, UINT32 pc, UINT32 insn) /* BL */
{
	UINT32 addr;

    if (insn & THUMB_BLOP_LO)
    {
        addr = GET_REGISTER(cpustate, 14) & ~1;
        addr += (insn & THUMB_BLOP_OFFS) << 1;
        SET_REGISTER(cpustate, 14, (R15 + 2) | 1);
        R15 = addr;
        //R15 += 2;
    }
    else
    {
        addr = (insn & THUMB_BLOP_OFFS) << 12;
        if (addr & (1 << 22))
        {
            addr |= 0xff800000;
        }
        addr += R15 + 4;
        SET_REGISTER(cpustate, 14, addr);
        R15 += 2;
    }
}
