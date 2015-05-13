// license:BSD-3-Clause
// copyright-holders:Curt Coder
#include "emu.h"
#include "debugger.h"
#include "z8.h"

/***************************************************************************
    CONSTANTS
***************************************************************************/

static const char *const REGISTER_NAME[256] =
{
	"P0", "P1", "P2", "P3", "", "", "", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	"SIO", "TMR", "T1", "PRE1", "T0", "PRE0", "P2M", "P3M", "P01M", "IPR", "IRQ", "IMR", "FLAGS", "RP", "SPH", "SPL"
};

static const char *const CONDITION_CODE[16] =
{
	"F", "LT", "LE", "ULE", "OV", "MI", "Z", "C",
	"", "GE", "GT", "UGT", "NOV", "PL", "NZ", "NC"
};

/***************************************************************************
    MACROS
***************************************************************************/

#define r       "R%u"
#define Ir      "@R%u"
#define R       "%02Xh"
#define RR      "%02Xh"
#define IR      "@%02Xh"
#define Irr     "@RR%u"
#define IRR     "@%02Xh"
#define IM      "#%02Xh"
#define X       "%02Xh(R%u)"
#define DA      "%04Xh"
#define RA      "%04Xh"

#define B0      oprom[0]
#define B1      oprom[1]
#define B0H     (B0 >> 4)
#define B0L     (B0 & 0x0f)
#define OPH     (opcode >> 4)

#define ARG(_formatting, _value)    { if (argc) dst += sprintf(dst, ", "); dst += sprintf(dst, _formatting, _value); argc++; }

#define arg_name(_value)            ARG("%s", REGISTER_NAME[_value])
#define arg_cc                      ARG("%s", CONDITION_CODE[OPH])
#define arg_r(_value)               ARG(r, _value)
#define arg_Ir(_value)              ARG(Ir, _value)
#define arg_Irr(_value)             ARG(Irr, _value & 0x0f)
#define arg_R(_value)               if ((_value & 0xf0) == 0xe0) ARG(r, _value & 0x0f) else if ((_value < 4) || (_value >= 0xf0)) arg_name(_value) else ARG(R, _value)
#define arg_RR(_value)              if ((_value & 0xf0) == 0xe0) ARG(r, _value & 0x0f) else ARG(R, _value)
#define arg_IR(_value)              if ((_value & 0xf0) == 0xe0) ARG(Ir, _value & 0x0f) else ARG(IR, _value)
#define arg_IRR(_value)             if ((_value & 0xf0) == 0xe0) ARG(Irr, _value & 0x0f) else ARG(IRR, _value)
#define arg_IM(_value)              ARG(IM, _value)
#define arg_RA                      ARG(RA, pc + (INT8)B0 + 2)
#define arg_DA                      ARG(DA, B0 << 8 | B1)
#define arg_X(_value1, _value2)     { if (argc) dst += sprintf(dst, ", "); dst += sprintf(dst, X, _value1, _value2); argc++; }

#define illegal                     dst += sprintf(dst, "Illegal")
#define mnemonic(_mnemonic)         dst += sprintf(dst, "%-5s", _mnemonic)
#define bytes(_count)               oprom += (_count - 1)
#define step_over                   flags = DASMFLAG_STEP_OVER
#define step_out                    flags = DASMFLAG_STEP_OUT

/***************************************************************************
    DISASSEMBLER
***************************************************************************/

CPU_DISASSEMBLE( z8 )
{
	const UINT8 *startrom = oprom;
	UINT32 flags = 0;
	UINT8 opcode = *oprom++;
	char *dst = buffer;
	int argc = 0;

	switch (pc)
	{
	case 0x0000:
	case 0x0002:
	case 0x0004:
	case 0x0006:
	case 0x0008:
	case 0x000a:
		sprintf(buffer, "IRQ%u Vector %04Xh", pc / 2, opcode << 8 | *oprom++); break;
	default:
		switch (opcode)
		{
			case 0x00:      mnemonic("DEC"); arg_R(B0); bytes(2);                   break;
			case 0x01:      mnemonic("DEC"); arg_IR(B0); bytes(2);                  break;
			case 0x02:      mnemonic("ADD"); arg_r(B0H); arg_r(B0L); bytes(2);      break;
			case 0x03:      mnemonic("ADD"); arg_r(B0H); arg_Ir(B0L); bytes(2);     break;
			case 0x04:      mnemonic("ADD"); arg_R(B1); arg_R(B0); bytes(3);        break;
			case 0x05:      mnemonic("ADD"); arg_R(B1); arg_IR(B0); bytes(3);       break;
			case 0x06:      mnemonic("ADD"); arg_R(B0); arg_IM(B1); bytes(3);       break;
			case 0x07:      mnemonic("ADD"); arg_IR(B0); arg_IM(B1); bytes(3);      break;
			case 0x08:      mnemonic("LD"); arg_r(OPH); arg_R(B0); bytes(2);        break;
			case 0x09:      mnemonic("LD"); arg_R(B0); arg_r(OPH); bytes(2);        break;
			case 0x0a:      mnemonic("DJNZ"); arg_r(OPH); arg_RA; bytes(2);         break;
			case 0x0b:      mnemonic("JR"); arg_cc; arg_RA; bytes(2);               break;
			case 0x0c:      mnemonic("LD"); arg_r(OPH); arg_IM(B0); bytes(2);       break;
			case 0x0d:      mnemonic("JP"); arg_cc; arg_DA; bytes(3);               break;
			case 0x0e:      mnemonic("INC"); arg_r(OPH);                            break;
			case 0x0f:      illegal;                                                break;

			case 0x10:      mnemonic("RLC"); arg_R(B0); bytes(2);                   break;
			case 0x11:      mnemonic("RLC"); arg_IR(B0); bytes(2);                  break;
			case 0x12:      mnemonic("ADC"); arg_r(B0H); arg_r(B0L); bytes(2);      break;
			case 0x13:      mnemonic("ADC"); arg_r(B0H); arg_Ir(B0L); bytes(2);     break;
			case 0x14:      mnemonic("ADC"); arg_R(B1); arg_R(B0); bytes(3);        break;
			case 0x15:      mnemonic("ADC"); arg_R(B1); arg_IR(B0); bytes(3);       break;
			case 0x16:      mnemonic("ADC"); arg_R(B0); arg_IM(B1); bytes(3);       break;
			case 0x17:      mnemonic("ADC"); arg_IR(B0); arg_IM(B1); bytes(3);      break;
			case 0x18:      mnemonic("LD"); arg_r(OPH); arg_R(B0); bytes(2);        break;
			case 0x19:      mnemonic("LD"); arg_R(B0); arg_r(OPH); bytes(2);        break;
			case 0x1a:      mnemonic("DJNZ"); arg_r(OPH); arg_RA; bytes(2);         break;
			case 0x1b:      mnemonic("JR"); arg_cc; arg_RA; bytes(2);               break;
			case 0x1c:      mnemonic("LD"); arg_r(OPH); arg_IM(B0); bytes(2);       break;
			case 0x1d:      mnemonic("JP"); arg_cc; arg_DA; bytes(3);               break;
			case 0x1e:      mnemonic("INC"); arg_r(OPH);                            break;
			case 0x1f:      illegal;                                                break;

			case 0x20:      mnemonic("INC"); arg_R(B0); bytes(2);                   break;
			case 0x21:      mnemonic("INC"); arg_IR(B0); bytes(2);                  break;
			case 0x22:      mnemonic("SUB"); arg_r(B0H); arg_r(B0L); bytes(2);      break;
			case 0x23:      mnemonic("SUB"); arg_r(B0H); arg_Ir(B0L); bytes(2);     break;
			case 0x24:      mnemonic("SUB"); arg_R(B1); arg_R(B0); bytes(3);        break;
			case 0x25:      mnemonic("SUB"); arg_R(B1); arg_IR(B0); bytes(3);       break;
			case 0x26:      mnemonic("SUB"); arg_R(B0); arg_IM(B1); bytes(3);       break;
			case 0x27:      mnemonic("SUB"); arg_IR(B0); arg_IM(B1); bytes(3);      break;
			case 0x28:      mnemonic("LD"); arg_r(OPH); arg_R(B0); bytes(2);        break;
			case 0x29:      mnemonic("LD"); arg_R(B0); arg_r(OPH); bytes(2);        break;
			case 0x2a:      mnemonic("DJNZ"); arg_r(OPH); arg_RA; bytes(2);         break;
			case 0x2b:      mnemonic("JR"); arg_cc; arg_RA; bytes(2);               break;
			case 0x2c:      mnemonic("LD"); arg_r(OPH); arg_IM(B0); bytes(2);       break;
			case 0x2d:      mnemonic("JP"); arg_cc; arg_DA; bytes(3);               break;
			case 0x2e:      mnemonic("INC"); arg_r(OPH);                            break;
			case 0x2f:      illegal;                                                break;

			case 0x30:      mnemonic("JP"); arg_IRR(B0); bytes(2);                  break;
			case 0x31:      mnemonic("SRP"); arg_IM(*oprom++);                      break;
			case 0x32:      mnemonic("SBC"); arg_r(B0H); arg_r(B0L); bytes(2);      break;
			case 0x33:      mnemonic("SBC"); arg_r(B0H); arg_Ir(B0L); bytes(2);     break;
			case 0x34:      mnemonic("SBC"); arg_R(B1); arg_R(B0); bytes(3);        break;
			case 0x35:      mnemonic("SBC"); arg_R(B1); arg_IR(B0); bytes(3);       break;
			case 0x36:      mnemonic("SBC"); arg_R(B0); arg_IM(B1); bytes(3);       break;
			case 0x37:      mnemonic("SBC"); arg_IR(B0); arg_IM(B1); bytes(3);      break;
			case 0x38:      mnemonic("LD"); arg_r(OPH); arg_R(B0); bytes(2);        break;
			case 0x39:      mnemonic("LD"); arg_R(B0); arg_r(OPH); bytes(2);        break;
			case 0x3a:      mnemonic("DJNZ"); arg_r(OPH); arg_RA; bytes(2);         break;
			case 0x3b:      mnemonic("JR"); arg_cc; arg_RA; bytes(2);               break;
			case 0x3c:      mnemonic("LD"); arg_r(OPH); arg_IM(B0); bytes(2);       break;
			case 0x3d:      mnemonic("JP"); arg_cc; arg_DA; bytes(3);               break;
			case 0x3e:      mnemonic("INC"); arg_r(OPH);                            break;
			case 0x3f:      illegal;                                                break;

			case 0x40:      mnemonic("DA"); arg_R(B0); bytes(2);                    break;
			case 0x41:      mnemonic("DA"); arg_IR(B0); bytes(2);                   break;
			case 0x42:      mnemonic("OR"); arg_r(B0H); arg_r(B0L); bytes(2);       break;
			case 0x43:      mnemonic("OR"); arg_r(B0H); arg_Ir(B0L); bytes(2);      break;
			case 0x44:      mnemonic("OR"); arg_R(B1); arg_R(B0); bytes(3);         break;
			case 0x45:      mnemonic("OR"); arg_R(B1); arg_IR(B0); bytes(3);        break;
			case 0x46:      mnemonic("OR"); arg_R(B0); arg_IM(B1); bytes(3);        break;
			case 0x47:      mnemonic("OR"); arg_IR(B0); arg_IM(B1); bytes(3);       break;
			case 0x48:      mnemonic("LD"); arg_r(OPH); arg_R(B0); bytes(2);        break;
			case 0x49:      mnemonic("LD"); arg_R(B0); arg_r(OPH); bytes(2);        break;
			case 0x4a:      mnemonic("DJNZ"); arg_r(OPH); arg_RA; bytes(2);         break;
			case 0x4b:      mnemonic("JR"); arg_cc; arg_RA; bytes(2);               break;
			case 0x4c:      mnemonic("LD"); arg_r(OPH); arg_IM(B0); bytes(2);       break;
			case 0x4d:      mnemonic("JP"); arg_cc; arg_DA; bytes(3);               break;
			case 0x4e:      mnemonic("INC"); arg_r(OPH);                            break;
			case 0x4f:      illegal; /* mnemonic("WDH"); */                         break;

			case 0x50:      mnemonic("POP"); arg_R(B0); bytes(2);                   break;
			case 0x51:      mnemonic("POP"); arg_IR(B0); bytes(2);                  break;
			case 0x52:      mnemonic("AND"); arg_r(B0H); arg_r(B0L); bytes(2);      break;
			case 0x53:      mnemonic("AND"); arg_r(B0H); arg_Ir(B0L); bytes(2);     break;
			case 0x54:      mnemonic("AND"); arg_R(B1); arg_R(B0); bytes(3);        break;
			case 0x55:      mnemonic("AND"); arg_R(B1); arg_IR(B0); bytes(3);       break;
			case 0x56:      mnemonic("AND"); arg_R(B0); arg_IM(B1); bytes(3);       break;
			case 0x57:      mnemonic("AND"); arg_IR(B0); arg_IM(B1); bytes(3);      break;
			case 0x58:      mnemonic("LD"); arg_r(OPH); arg_R(B0); bytes(2);        break;
			case 0x59:      mnemonic("LD"); arg_R(B0); arg_r(OPH); bytes(2);        break;
			case 0x5a:      mnemonic("DJNZ"); arg_r(OPH); arg_RA; bytes(2);         break;
			case 0x5b:      mnemonic("JR"); arg_cc; arg_RA; bytes(2);               break;
			case 0x5c:      mnemonic("LD"); arg_r(OPH); arg_IM(B0); bytes(2);       break;
			case 0x5d:      mnemonic("JP"); arg_cc; arg_DA; bytes(3);               break;
			case 0x5e:      mnemonic("INC"); arg_r(OPH);                            break;
			case 0x5f:      illegal; /* mnemonic("WDT"); */                         break;

			case 0x60:      mnemonic("COM"); arg_R(B0); bytes(2);                   break;
			case 0x61:      mnemonic("COM"); arg_IR(B0); bytes(2);                  break;
			case 0x62:      mnemonic("TCM"); arg_r(B0H); arg_r(B0L); bytes(2);      break;
			case 0x63:      mnemonic("TCM"); arg_r(B0H); arg_Ir(B0L); bytes(2);     break;
			case 0x64:      mnemonic("TCM"); arg_R(B1); arg_R(B0); bytes(3);        break;
			case 0x65:      mnemonic("TCM"); arg_R(B1); arg_IR(B0); bytes(3);       break;
			case 0x66:      mnemonic("TCM"); arg_R(B0); arg_IM(B1); bytes(3);       break;
			case 0x67:      mnemonic("TCM"); arg_IR(B0); arg_IM(B1); bytes(3);      break;
			case 0x68:      mnemonic("LD"); arg_r(OPH); arg_R(B0); bytes(2);        break;
			case 0x69:      mnemonic("LD"); arg_R(B0); arg_r(OPH); bytes(2);        break;
			case 0x6a:      mnemonic("DJNZ"); arg_r(OPH); arg_RA; bytes(2);         break;
			case 0x6b:      mnemonic("JR"); arg_cc; arg_RA; bytes(2);               break;
			case 0x6c:      mnemonic("LD"); arg_r(OPH); arg_IM(B0); bytes(2);       break;
			case 0x6d:      mnemonic("JP"); arg_cc; arg_DA; bytes(3);               break;
			case 0x6e:      mnemonic("INC"); arg_r(OPH);                            break;
			case 0x6f:      illegal; /* mnemonic("STOP"); */                        break;

			case 0x70:      mnemonic("PUSH"); arg_R(B0); bytes(2);                  break;
			case 0x71:      mnemonic("PUSH"); arg_IR(B0); bytes(2);                 break;
			case 0x72:      mnemonic("TM"); arg_r(B0H); arg_r(B0L); bytes(2);       break;
			case 0x73:      mnemonic("TM"); arg_r(B0H); arg_Ir(B0L); bytes(2);      break;
			case 0x74:      mnemonic("TM"); arg_R(B1); arg_R(B0); bytes(3);         break;
			case 0x75:      mnemonic("TM"); arg_R(B1); arg_IR(B0); bytes(3);        break;
			case 0x76:      mnemonic("TM"); arg_R(B0); arg_IM(B1); bytes(3);        break;
			case 0x77:      mnemonic("TM"); arg_IR(B0); arg_IM(B1); bytes(3);       break;
			case 0x78:      mnemonic("LD"); arg_r(OPH); arg_R(B0); bytes(2);        break;
			case 0x79:      mnemonic("LD"); arg_R(B0); arg_r(OPH); bytes(2);        break;
			case 0x7a:      mnemonic("DJNZ"); arg_r(OPH); arg_RA; bytes(2);         break;
			case 0x7b:      mnemonic("JR"); arg_cc; arg_RA; bytes(2);               break;
			case 0x7c:      mnemonic("LD"); arg_r(OPH); arg_IM(B0); bytes(2);       break;
			case 0x7d:      mnemonic("JP"); arg_cc; arg_DA; bytes(3);               break;
			case 0x7e:      mnemonic("INC"); arg_r(OPH);                            break;
			case 0x7f:      illegal; /* mnemonic("HALT"); */                        break;

			case 0x80:      mnemonic("DECW"); arg_RR(*oprom++);                     break;
			case 0x81:      mnemonic("DECW"); arg_IR(B0); bytes(2);                 break;
			case 0x82:      mnemonic("LDE"); arg_r(B0H); arg_Irr(B0L); bytes(2);    break;
			case 0x83:      mnemonic("LDEI"); arg_Ir(B0H); arg_Irr(B0L); bytes(2);  break;
			case 0x84:      illegal;                                                break;
			case 0x85:      illegal;                                                break;
			case 0x86:      illegal;                                                break;
			case 0x87:      illegal;                                                break;
			case 0x88:      mnemonic("LD"); arg_r(OPH); arg_R(B0); bytes(2);        break;
			case 0x89:      mnemonic("LD"); arg_R(B0); arg_r(OPH); bytes(2);        break;
			case 0x8a:      mnemonic("DJNZ"); arg_r(OPH); arg_RA; bytes(2);         break;
			case 0x8b:      mnemonic("JR"); arg_RA; bytes(2);                       break;
			case 0x8c:      mnemonic("LD"); arg_r(OPH); arg_IM(B0); bytes(2);       break;
			case 0x8d:      mnemonic("JP"); arg_DA; bytes(3);                       break;
			case 0x8e:      mnemonic("INC"); arg_r(OPH);                            break;
			case 0x8f:      mnemonic("DI");                                         break;

			case 0x90:      mnemonic("RL"); arg_R(B0); bytes(2);                    break;
			case 0x91:      mnemonic("RL"); arg_IR(B0); bytes(2);                   break;
			case 0x92:      mnemonic("LDE"); arg_r(B0L); arg_Irr(B0H); bytes(2);    break;
			case 0x93:      mnemonic("LDEI"); arg_Ir(B0L); arg_Irr(B0H); bytes(2);  break;
			case 0x94:      illegal;                                                break;
			case 0x95:      illegal;                                                break;
			case 0x96:      illegal;                                                break;
			case 0x97:      illegal;                                                break;
			case 0x98:      mnemonic("LD"); arg_r(OPH); arg_R(B0); bytes(2);        break;
			case 0x99:      mnemonic("LD"); arg_R(B0); arg_r(OPH); bytes(2);        break;
			case 0x9a:      mnemonic("DJNZ"); arg_r(OPH); arg_RA; bytes(2);         break;
			case 0x9b:      mnemonic("JR"); arg_cc; arg_RA; bytes(2);               break;
			case 0x9c:      mnemonic("LD"); arg_r(OPH); arg_IM(B0); bytes(2);       break;
			case 0x9d:      mnemonic("JP"); arg_cc; arg_DA; bytes(3);               break;
			case 0x9e:      mnemonic("INC"); arg_r(OPH);                            break;
			case 0x9f:      mnemonic("EI");                                         break;

			case 0xa0:      mnemonic("INCW"); arg_RR(B0); bytes(2);                 break;
			case 0xa1:      mnemonic("INCW"); arg_IR(B0); bytes(2);                 break;
			case 0xa2:      mnemonic("CP"); arg_r(B0H); arg_r(B0L); bytes(2);       break;
			case 0xa3:      mnemonic("CP"); arg_r(B0H); arg_Ir(B0L); bytes(2);      break;
			case 0xa4:      mnemonic("CP"); arg_R(B1); arg_R(B0); bytes(3);         break;
			case 0xa5:      mnemonic("CP"); arg_R(B1); arg_IR(B0); bytes(3);        break;
			case 0xa6:      mnemonic("CP"); arg_R(B0); arg_IM(B1); bytes(3);        break;
			case 0xa7:      mnemonic("CP"); arg_IR(B0); arg_IM(B1); bytes(3);       break;
			case 0xa8:      mnemonic("LD"); arg_r(OPH); arg_R(B0); bytes(2);        break;
			case 0xa9:      mnemonic("LD"); arg_R(B0); arg_r(OPH); bytes(2);        break;
			case 0xaa:      mnemonic("DJNZ"); arg_r(OPH); arg_RA; bytes(2);         break;
			case 0xab:      mnemonic("JR"); arg_cc; arg_RA; bytes(2);               break;
			case 0xac:      mnemonic("LD"); arg_r(OPH); arg_IM(B0); bytes(2);       break;
			case 0xad:      mnemonic("JP"); arg_cc; arg_DA; bytes(3);               break;
			case 0xae:      mnemonic("INC"); arg_r(OPH);                            break;
			case 0xaf:      mnemonic("RET"); step_out;                              break;

			case 0xb0:      mnemonic("CLR"); arg_R(B0); bytes(2);                   break;
			case 0xb1:      mnemonic("CLR"); arg_IR(B0); bytes(2);                  break;
			case 0xb2:      mnemonic("XOR"); arg_r(B0H); arg_r(B0L); bytes(2);      break;
			case 0xb3:      mnemonic("XOR"); arg_r(B0H); arg_Ir(B0L); bytes(2);     break;
			case 0xb4:      mnemonic("XOR"); arg_R(B1); arg_R(B0); bytes(3);        break;
			case 0xb5:      mnemonic("XOR"); arg_R(B1); arg_IR(B0); bytes(3);       break;
			case 0xb6:      mnemonic("XOR"); arg_R(B0); arg_IM(B1); bytes(3);       break;
			case 0xb7:      mnemonic("XOR"); arg_IR(B0); arg_IM(B1); bytes(3);      break;
			case 0xb8:      mnemonic("LD"); arg_r(OPH); arg_R(B0); bytes(2);        break;
			case 0xb9:      mnemonic("LD"); arg_R(B0); arg_r(OPH); bytes(2);        break;
			case 0xba:      mnemonic("DJNZ"); arg_r(OPH); arg_RA; bytes(2);         break;
			case 0xbb:      mnemonic("JR"); arg_cc; arg_RA; bytes(2);               break;
			case 0xbc:      mnemonic("LD"); arg_r(OPH); arg_IM(B0); bytes(2);       break;
			case 0xbd:      mnemonic("JP"); arg_cc; arg_DA; bytes(3);               break;
			case 0xbe:      mnemonic("INC"); arg_r(OPH);                            break;
			case 0xbf:      mnemonic("IRET"); step_out;                             break;

			case 0xc0:      mnemonic("RRC"); arg_R(B0); bytes(2);                   break;
			case 0xc1:      mnemonic("RRC"); arg_IR(B0); bytes(2);                  break;
			case 0xc2:      mnemonic("LDC"); arg_r(B0H); arg_Irr(B0L); bytes(2);    break;
			case 0xc3:      mnemonic("LDCI"); arg_Ir(B0H); arg_Irr(B0L); bytes(2);  break;
			case 0xc4:      illegal;                                                break;
			case 0xc5:      illegal;                                                break;
			case 0xc6:      illegal;                                                break;
			case 0xc7:      mnemonic("LD"); arg_r(B0H); arg_X(B1, B0L); bytes(3);   break;
			case 0xc8:      mnemonic("LD"); arg_r(OPH); arg_R(B0); bytes(2);        break;
			case 0xc9:      mnemonic("LD"); arg_R(B0); arg_r(OPH); bytes(2);        break;
			case 0xca:      mnemonic("DJNZ"); arg_r(OPH); arg_RA; bytes(2);         break;
			case 0xcb:      mnemonic("JR"); arg_cc; arg_RA; bytes(2);               break;
			case 0xcc:      mnemonic("LD"); arg_r(OPH); arg_IM(B0); bytes(2);       break;
			case 0xcd:      mnemonic("JP"); arg_cc; arg_DA; bytes(3);               break;
			case 0xce:      mnemonic("INC"); arg_r(OPH);                            break;
			case 0xcf:      mnemonic("RCF");                                        break;

			case 0xd0:      mnemonic("SRA"); arg_R(B0); bytes(2);                   break;
			case 0xd1:      mnemonic("SRA"); arg_IR(B0); bytes(2);                  break;
			case 0xd2:      mnemonic("LDC"); arg_Irr(B0L); arg_r(B0H); bytes(2);    break;
			case 0xd3:      mnemonic("LDCI"); arg_Irr(B0L); arg_Ir(B0H); bytes(2);  break;
			case 0xd4:      mnemonic("CALL"); arg_IRR(B0); bytes(2); step_over;     break;
			case 0xd5:      illegal;                                                break;
			case 0xd6:      mnemonic("CALL"); arg_DA; bytes(3); step_over;          break;
			case 0xd7:      mnemonic("LD"); arg_r(B0L); arg_X(B1, B0H); bytes(3);   break;
			case 0xd8:      mnemonic("LD"); arg_r(OPH); arg_R(B0); bytes(2);        break;
			case 0xd9:      mnemonic("LD"); arg_R(B0); arg_r(OPH); bytes(2);        break;
			case 0xda:      mnemonic("DJNZ"); arg_r(OPH); arg_RA; bytes(2);         break;
			case 0xdb:      mnemonic("JR"); arg_cc; arg_RA; bytes(2);               break;
			case 0xdc:      mnemonic("LD"); arg_r(OPH); arg_IM(B0); bytes(2);       break;
			case 0xdd:      mnemonic("JP"); arg_cc; arg_DA; bytes(3);               break;
			case 0xde:      mnemonic("INC"); arg_r(OPH);                            break;
			case 0xdf:      mnemonic("SCF");                                        break;

			case 0xe0:      mnemonic("RR"); arg_R(B0); bytes(2);                    break;
			case 0xe1:      mnemonic("RR"); arg_IR(B0); bytes(2);                   break;
			case 0xe2:      illegal;                                                break;
			case 0xe3:      mnemonic("LD"); arg_r(B0H); arg_Ir(B0L); bytes(2);      break;
			case 0xe4:      mnemonic("LD"); arg_R(B1); arg_R(B0); bytes(3);         break;
			case 0xe5:      mnemonic("LD"); arg_R(B1); arg_IR(B0); bytes(3);        break;
			case 0xe6:      mnemonic("LD"); arg_R(B0); arg_IM(B1); bytes(3);        break;
			case 0xe7:      mnemonic("LD"); arg_IR(B0); arg_IM(B1); bytes(3);       break;
			case 0xe8:      mnemonic("LD"); arg_r(OPH); arg_R(B0); bytes(2);        break;
			case 0xe9:      mnemonic("LD"); arg_R(B0); arg_r(OPH); bytes(2);        break;
			case 0xea:      mnemonic("DJNZ"); arg_r(OPH); arg_RA; bytes(2);         break;
			case 0xeb:      mnemonic("JR"); arg_cc; arg_RA; bytes(2);               break;
			case 0xec:      mnemonic("LD"); arg_r(OPH); arg_IM(B0); bytes(2);       break;
			case 0xed:      mnemonic("JP"); arg_cc; arg_DA; bytes(3);               break;
			case 0xee:      mnemonic("INC"); arg_r(OPH);                            break;
			case 0xef:      mnemonic("CCF");                                        break;

			case 0xf0:      mnemonic("SWAP"); arg_R(B0); bytes(2);                  break;
			case 0xf1:      mnemonic("SWAP"); arg_IR(B0); bytes(2);                 break;
			case 0xf2:      illegal;                                                break;
			case 0xf3:      mnemonic("LD"); arg_Ir(B0H); arg_r(B0L); bytes(2);      break;
			case 0xf4:      illegal;                                                break;
			case 0xf5:      mnemonic("LD"); arg_IR(B0); arg_R(B1); bytes(3);        break;
			case 0xf6:      illegal;                                                break;
			case 0xf7:      illegal;                                                break;
			case 0xf8:      mnemonic("LD"); arg_r(OPH); arg_R(B0); bytes(2);        break;
			case 0xf9:      mnemonic("LD"); arg_R(B0); arg_r(OPH); bytes(2);        break;
			case 0xfa:      mnemonic("DJNZ"); arg_r(OPH); arg_RA; bytes(2);         break;
			case 0xfb:      mnemonic("JR"); arg_cc; arg_RA; bytes(2);               break;
			case 0xfc:      mnemonic("LD"); arg_r(OPH); arg_IM(B0); bytes(2);       break;
			case 0xfd:      mnemonic("JP"); arg_cc; arg_DA; bytes(3);               break;
			case 0xfe:      mnemonic("INC"); arg_r(OPH);                            break;
			case 0xff:      mnemonic("NOP");                                        break;
		}
	}

	return (oprom - startrom) | flags | DASMFLAG_SUPPORTED;
}
