// license:BSD-3-Clause
// copyright-holders:R. Belmont, David Haywood
/***************************************************************************

    sh4fe.c

    Front end for SH-4 recompiler

***************************************************************************/

#include "emu.h"
#include "sh4.h"
#include "sh4comn.h"
#include "cpu/drcfe.h"


/***************************************************************************
    INSTRUCTION PARSERS
***************************************************************************/

sh4_frontend::sh4_frontend(sh_common_execution *device, uint32_t window_start, uint32_t window_end, uint32_t max_sequence)
	: sh_frontend(device, window_start, window_end, max_sequence)
{
}

uint16_t sh4_frontend::read_word(opcode_desc &desc)
{
	if (desc.physpc >= 0xe0000000)
		return m_sh->m_pr16(desc.physpc);

	return m_sh->m_pr16(desc.physpc & SH34_AM);
}

uint16_t sh4be_frontend::read_word(opcode_desc &desc)
{
	if (desc.physpc >= 0xe0000000)
		return m_sh->m_pr16(desc.physpc);

	return m_sh->m_pr16(desc.physpc & SH34_AM);
}


bool sh4_frontend::describe_group_0(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode)
{
	switch (opcode & 0xff)
	{
	default:
		// fall through to SH2 handlers
		return sh_frontend::describe_group_0(desc, prev, opcode);

	case 0x52:
	case 0x62:
	case 0x43:
	case 0x63:
	case 0xe3:
	case 0x68:
	case 0xe8:
	case 0x4a:
	case 0xca:
		return true; // ILLEGAL(); break; // illegal on sh4

	case 0x93:
	case 0xa3:
	case 0xb3:
		return true; // TODO(opcode); break;

	case 0x82:
	case 0x92:
	case 0xa2:
	case 0xb2:
	case 0xc2:
	case 0xd2:
	case 0xe2:
	case 0xf2:
		return true; // STC RBANK(opcode); break; // sh3/4 only

	case 0x32:  return true; // STCSSR(opcode); break; // sh3/4 only
	case 0x42:  return true; // STCSPC(opcode); break; // sh3/4 only
	case 0x83:  return true; // PREFM(opcode); break; // sh3/4 only
	case 0xc3:  return true; // MOVCAL(opcode); break; // sh4 only

	case 0x38:
	case 0xb8:
		return true; // LDTLB(opcode); break; // sh3/4 only

	case 0x48:
	case 0xc8:
		return true; // CLRS(opcode); break; // sh3/4 only

	case 0x58:
	case 0xd8:
		return true; // SETS(opcode); break; // sh3/4 only

	case 0x3a:
	case 0xba:
		return true; // STCSGR(opcode); break; // sh4 only

	case 0x5a:
	case 0xda:
		return true; // STSFPUL(opcode); break; // sh4 only

	case 0x6a:
	case 0xea:
		return true; // STSFPSCR(opcode); break; // sh4 only

	case 0x7a:
	case 0xfa:
		return true; // STCDBR(opcode); break; // sh4 only
	}

	return false;
}


bool sh4_frontend::describe_group_4(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode)
{
	switch (opcode & 0xff)
	{

	default: // LDCMSR (0x0e) has sh2/4 flag difference
		// fall through to SH2 handlers
		return sh_frontend::describe_group_4(desc, prev, opcode);

	case 0x42:
	case 0x46:
	case 0x4a:
	case 0x53:
	case 0x57:
	case 0x5e:
	case 0x63:
	case 0x67:
	case 0x6e:
	case 0x82:
	case 0x86:
	case 0x8a:
	case 0x92:
	case 0x96:
	case 0x9a:
	case 0xa2:
	case 0xa6:
	case 0xaa:
	case 0xc2:
	case 0xc6:
	case 0xca:
	case 0xd2:
	case 0xd6:
	case 0xda:
	case 0xe2:
	case 0xe6:
	case 0xea:
		return true; // ILLEGAL(); break; // defined as illegal on SH4

	case 0x0c:
	case 0x1c:
	case 0x2c:
	case 0x3c:
	case 0x4c:
	case 0x5c:
	case 0x6c:
	case 0x7c:
	case 0x8c:
	case 0x9c:
	case 0xac:
	case 0xbc:
	case 0xcc:
	case 0xdc:
	case 0xec:
	case 0xfc:
		return true; // SHAD(opcode); break; // sh3/4 only  needed

	case 0x0d:
	case 0x1d:
	case 0x2d:
	case 0x3d:
	case 0x4d:
	case 0x5d:
	case 0x6d:
	case 0x7d:
	case 0x8d:
	case 0x9d:
	case 0xad:
	case 0xbd:
	case 0xcd:
	case 0xdd:
	case 0xed:
	case 0xfd:
		return true; // SHLD(opcode); break; // sh3/4 only  needed

	case 0x8e:
	case 0x9e:
	case 0xae:
	case 0xbe:
	case 0xce:
	case 0xde:
	case 0xee:
	case 0xfe:
		return true; // LDCRBANK(opcode); break; // sh3/4 only

	case 0x83:
	case 0x93:
	case 0xa3:
	case 0xb3:
	case 0xc3:
	case 0xd3:
	case 0xe3:
	case 0xf3:
		return true; // STCMRBANK(opcode); break; // sh3/4 only

	case 0x87:
	case 0x97:
	case 0xa7:
	case 0xb7:
	case 0xc7:
	case 0xd7:
	case 0xe7:
	case 0xf7:
		return true; // LDCMRBANK(opcode); break; // sh3/4 only

	case 0x32:  return true; // STCMSGR(opcode); break; // sh4 only
	case 0x33:  return true; // STCMSSR(opcode); break; // sh4 only
	case 0x37:  return true; // LDCMSSR(opcode); break; // sh3/4 only
	case 0x3e:  return true; // LDCSSR(opcode); break; // sh3/4 only
	case 0x43:  return true; // STCMSPC(opcode); break; // sh3/4 only
	case 0x47:  return true; // LDCMSPC(opcode); break; // sh3/4 only
	case 0x4e:  return true; // LDCSPC(opcode); break; // sh3/4 only
	case 0x52:  return true; // STSMFPUL(opcode); break; // sh4 only
	case 0x56:  return true; // LDSMFPUL(opcode); break; // sh4 only
	case 0x5a:  return true; // LDSFPUL(opcode); break; // sh4 only
	case 0x62:  return true; // STSMFPSCR(opcode); break; // sh4 only
	case 0x66:  return true; // LDSMFPSCR(opcode); break; // sh4 only
	case 0x6a:  return true; // LDSFPSCR(opcode); break; // sh4 only
	case 0xf2:  return true; // STCMDBR(opcode); break; // sh4 only
	case 0xf6:  return true; // LDCMDBR(opcode); break; // sh4 only
	case 0xfa:  return true; // LDCDBR(opcode); break; // sh4 only
	}

	return false;
}

// SH4 only (FPU ops)
bool sh4_frontend::describe_group_15(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode)
{
	switch (opcode & 0x0f)
	{
	case 0x00:  return true; // FADD(opcode); break;
	case 0x01:  return true; // FSUB(opcode); break;
	case 0x02:  return true; // FMUL(opcode); break;
	case 0x03:  return true; // FDIV(opcode); break;
	case 0x04:  return true; // FCMP_EQ(opcode); break;
	case 0x05:  return true; // FCMP_GT(opcode); break;
	case 0x06:  return true; // FMOVS0FR(opcode); break;
	case 0x07:  return true; // FMOVFRS0(opcode); break;
	case 0x08:  return true; // FMOVMRFR(opcode); break;
	case 0x09:  return true; // FMOVMRIFR(opcode); break;
	case 0x0a:  return true; // FMOVFRMR(opcode); break;
	case 0x0b:  return true; // FMOVFRMDR(opcode); break;
	case 0x0c:  return true; // FMOVFR(opcode); break;
	case 0x0d:  return describe_op1111_0x13(desc, prev, opcode); // break;
	case 0x0e:  return true; // FMAC(opcode); break;
	case 0x0f:
		return true;
		//if (opcode == 0xffff) return true;    // atomiswave uses ffff as NOP?
		//return false; // dbreak(opcode); break;
	}
	return false;
}

bool sh4_frontend::describe_op1111_0x13(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode)
{
	switch ((opcode >> 4) & 0x0f)
	{
	case 0x00:  return true; // FSTS(opcode); break;
	case 0x01:  return true; // FLDS(opcode); break;
	case 0x02:  return true; // FLOAT(opcode); break;
	case 0x03:  return true; // FTRC(opcode); break;
	case 0x04:  return true; // FNEG(opcode); break;
	case 0x05:  return true; // FABS(opcode); break;
	case 0x06:  return true; // FSQRT(opcode); break;
	case 0x07:  return true; // FSRRA(opcode); break;
	case 0x08:  return true; // FLDI0(opcode); break;
	case 0x09:  return true; // FLDI1(opcode); break;
	case 0x0a:  return true; // FCNVSD(opcode); break;
	case 0x0b:  return true; // FCNVDS(opcode); break;
	case 0x0c:  return false; // dbreak(opcode); break;
	case 0x0d:  return false; // dbreak(opcode); break;
	case 0x0e:  return true; // FIPR(opcode); break;
	case 0x0f:  return describe_op1111_0xf13(desc, prev, opcode); // break;
	}
	return false;
}

bool sh4_frontend::describe_op1111_0xf13(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode)
{
	if (opcode & 0x100) {
		if (opcode & 0x200) {
			switch (opcode & 0xC00)
			{
			case 0x000:
				return true; //FSCHG();
				break;
			case 0x800:
				return true; //FRCHG();
				break;
			default:
				return false; //machine().debug_break();
				break;
			}
		}
		else {
			return true; // FTRV(opcode);
		}
	}
	else {
		return true; // FSSCA(opcode);
	}
	return false;
}
