// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
#include "emu.h"
#include "debugger.h"
#include "mb86233.h"

static std::string COND(unsigned int cond)
{
	std::ostringstream stream;

	switch(cond)
	{
		case 0x16:
			util::stream_format(stream, "always");
			return stream.str();

		case 0x00:
			util::stream_format(stream, "eq");
			return stream.str();

		case 0x01:
			util::stream_format(stream, "ge");
			return stream.str();

		case 0x02:
			util::stream_format(stream, "le");
			return stream.str();

		case 0x06:
			util::stream_format(stream, "never");
			return stream.str();

		case 0x10:
			util::stream_format(stream, "(--r12)!=0");
			return stream.str();

		case 0x11:
			util::stream_format(stream, "(--r13)!=0");
			return stream.str();
	}

	util::stream_format(stream, "unk (%x)",cond);
	return stream.str();
}

static std::string REGS( uint32_t reg, int IsSource )
{
	std::ostringstream stream;
	int         mode = (reg >> 6 ) & 0x07;

	reg &= 0x3f;

	if ( mode == 0 || mode == 1 || mode == 3 )
	{
		if ( reg < 0x10 )
		{
			util::stream_format(stream, "r%d",reg);
			return stream.str();
		}

		switch(reg)
		{
			case 0x10:
				util::stream_format(stream, "a");
			break;

			case 0x11:
				util::stream_format(stream, "a.e");
			break;

			case 0x12:
				util::stream_format(stream, "a.m");
			break;

			case 0x13:
				util::stream_format(stream, "b");
			break;

			case 0x14:
				util::stream_format(stream, "b.e");
			break;

			case 0x15:
				util::stream_format(stream, "b.m");
			break;

			case 0x19:
				util::stream_format(stream, "d");
			break;

			case 0x1a:
				util::stream_format(stream, "d.e");
			break;

			case 0x1b:
				util::stream_format(stream, "d.m");
			break;

			case 0x1c:
				util::stream_format(stream, "p");
			break;

			case 0x1d:
				util::stream_format(stream, "p.e");
			break;

			case 0x1e:
				util::stream_format(stream, "p.m");
			break;

			case 0x1f:
				util::stream_format(stream, "shift");
			break;

			case 0x20:
				util::stream_format(stream, "parport");
			break;

			case 0x21:
				util::stream_format(stream, "FIn");
			break;

			case 0x22:
				util::stream_format(stream, "FOut");
			break;

			case 0x23:
				util::stream_format(stream, "EB");
			break;

			default:
				util::stream_format(stream, "Unkreg (%x)",reg);
			break;
		}
	}
	else if ( mode == 2 )
	{
		util::stream_format(stream, "0x%x+",reg & 0x1f);

		if ( IsSource )
		{
			if ( !( reg & 0x20 ) )
				util::stream_format(stream, "r0+");

			util::stream_format(stream, "r2");
		}
		else
		{
			if ( !( reg & 0x20 ) )
				util::stream_format(stream, "r1+");

			util::stream_format(stream, "r3");
		}
	}
	else if ( mode == 6 )
	{
		if ( IsSource )
		{
			if ( !( reg & 0x20 ) )
				util::stream_format(stream, "r0+");

			util::stream_format(stream, "r2");
		}
		else
		{
			if ( !( reg & 0x20 ) )
				util::stream_format(stream, "r1+");

			util::stream_format(stream, "r3");
		}

		if ( reg & 0x10 )
			util::stream_format(stream, "--%d", 0x20 - ( reg & 0x1f ) );
		else
			util::stream_format(stream, "++%d", reg & 0x1f );
	}
	else
	{
		util::stream_format(stream, "UNKMOD %x (0x%x)", mode, reg);
	}

	return stream.str();
}

static std::string INDIRECT( uint32_t reg, int IsSource )
{
	std::ostringstream stream;
	int         mode = ( reg >> 6 ) & 0x07;

	if ( mode == 0 || mode == 3 || mode == 1)
	{
		util::stream_format(stream, "0x%x", reg);
	}
	else if ( mode == 2 )
	{
		util::stream_format(stream, "0x%x+",reg&0x1f);

		if ( IsSource )
		{
			if ( !(reg & 0x20) )
				util::stream_format(stream, "r0+");

			util::stream_format(stream, "r2");
		}
		else
		{
			if ( !(reg & 0x20) )
				util::stream_format(stream, "r1+");

			util::stream_format(stream, "r3");
		}
	}
	else if ( mode == 6 || mode == 7 )
	{
		if ( IsSource )
		{
			if ( !( reg & 0x20 ) )
				util::stream_format(stream, "r0+");

			util::stream_format(stream, "r2");
		}
		else
		{
			if( !( reg & 0x20 ) )
				util::stream_format(stream, "r1+");

			util::stream_format(stream, "r3");
		}

		if ( reg & 0x10 )
			util::stream_format(stream, "--%d",0x20 - ( reg & 0x1f ));
		else
			util::stream_format(stream, "++%d",reg & 0x1f);
	}
	else
	{
		util::stream_format(stream, "UNKMOD %x (0x%x)", mode, reg);
	}

	return stream.str();
}


static std::string ALU( uint32_t alu)
{
	std::ostringstream stream;

	switch( alu )
	{
		case 0x0:
		break;

		case 0x1:
			util::stream_format(stream, "d=d&a");
		break;

		case 0x2:
			util::stream_format(stream, "d=d|a");
		break;

		case 0x3:
			util::stream_format(stream, "d=d^a");
		break;

		case 0x5:
			util::stream_format(stream, "cmp d,a");
		break;

		case 0x6:
			util::stream_format(stream, "d=d+a");
		break;

		case 0x7:
			util::stream_format(stream, "d=d-a");
		break;

		case 0x8:
			util::stream_format(stream, "p=a*b");
		break;

		case 0x9:
			util::stream_format(stream, "d=d+p, p=a*b");
		break;

		case 0xa:
			util::stream_format(stream, "d=d-p, p=a*b");
		break;

		case 0xb:
			util::stream_format(stream, "d=fabs d");
		break;

		case 0xc:
			util::stream_format(stream, "d=d+p");
		break;

		case 0xd:
			util::stream_format(stream, "d=p, p=a*b");
		break;

		case 0xe:
			util::stream_format(stream, "d=float(d)");
		break;

		case 0xf:
			util::stream_format(stream, "d=int(d)");
		break;

		case 0x10:
			util::stream_format(stream, "d=d/a");
		break;

		case 0x11:
			util::stream_format(stream, "d=-d");
		break;

		case 0x13:
			util::stream_format(stream, "d=a+b");
		break;

		case 0x14:
			util::stream_format(stream, "d=b-a");
		break;

		case 0x16:
			util::stream_format(stream, "d=(lsr d,shift)");
		break;

		case 0x17:
			util::stream_format(stream, "d=(lsl d,shift)");
		break;

		case 0x18:
			util::stream_format(stream, "d=(asr d,shift)");
		break;

		case 0x1a:
			util::stream_format(stream, "d=d+a (int)");
		break;

		case 0x1b:
			util::stream_format(stream, "d=d-a (int)");
		break;

		default:
			util::stream_format(stream, "ALU UNK(%x)",alu);
		break;
	}

	return stream.str();
}

static unsigned dasm_mb86233(std::ostream &stream, uint32_t opcode )
{
	uint32_t  grp = ( opcode >> 26 ) & 0x3f;

	switch( grp )
	{
		case 0x0:   /* Dual move */
		{
			uint32_t r1=opcode & 0x1ff;
			uint32_t r2=(opcode>>9) & 0x7f;
			uint32_t alu=(opcode>>21) & 0x1f;
			uint32_t op=(opcode>>16) & 0x1f;

			if ( alu != 0 )
				util::stream_format(stream, "%s, ", ALU(alu) );

			switch( op )
			{
				case 0x0c:  /* a = RAM[addr], b = BRAM[addr] */
					util::stream_format(stream, "LAB RAM(0x%x)->a,BRAM(0x%x)->b",r1,r2);
				break;

				case 0x0d:  /* a = RAM[addr], b = BRAM[addr] */
					util::stream_format(stream, "LAB RAM(0x%x)->a,BRAM(%s)->b",r1,INDIRECT(r2|(2<<6),0));
				break;

				case 0x0f:  /* a = RAM[addr], b = BRAM[reg] */
					util::stream_format(stream, "LAB RAM(0x%x)->a,BRAM(%s)->b",r1,INDIRECT(r2|(6<<6),0));
				break;

				case 0x10:  /* a = BRAM[reg], b = RAM[addr] */
					util::stream_format(stream, "LAB BRAM(%s)->a,RAM(0x%x)->b",INDIRECT(r1,1),r2);
				break;

				default:
					util::stream_format(stream, "UNKDUAL (%x)",op);
				break;
			}
		}
		break;

		case 0x7:   /* LD/MOV */
		{
			uint32_t r1=opcode & 0x1ff;
			uint32_t r2=(opcode>>9) & 0x7f;
			uint32_t alu=(opcode>>21) & 0x1f;
			uint32_t op=(opcode>>16) & 0x1f;

			if ( alu != 0 )
			{
				util::stream_format(stream, "%s", ALU(alu) );

				if ( !(op == 0x1f && r1 == 0x10 && r2 == 0x0f) )
					util::stream_format(stream, ", ");
			}

			switch(op)
			{
				case 0x03:  /* RAM->External Indirect */
				{
					util::stream_format(stream, "MOV RAM(0x%x)->E(EB+%s)",r1,INDIRECT(r2|(6<<6),0));
				}
				break;

				case 0x04:  /* MOV RAM->External */
				{
					util::stream_format(stream, "MOV RAM(0x%x)->E(EB+0x%x)",r1,r2);
				}
				break;

				case 0x07:  /* RAMInd->External */
				{
					util::stream_format(stream, "MOV RAM(%s)->E(EB+%s)",INDIRECT(r1,1),INDIRECT(r2|(6<<6),0));
				}
				break;

				case 0x08:  /* External->RAM */
				{
					util::stream_format(stream, "MOV EXT(EB+");
					util::stream_format(stream, "%s",INDIRECT(r1,1));
					util::stream_format(stream, ")->RAM(0x%x)",r2);
				}
				break;

				case 0x0b:  /* External->RAMInd */
				{
					int mode = ( r1 >> 6 ) & 0x07;

					util::stream_format(stream, "MOV EXT(EB+");

					if ( mode == 0 || mode == 3 || mode == 1 )
						util::stream_format(stream, "RAM(");

					util::stream_format(stream, "%s",INDIRECT(r1,1));

					if ( mode == 0 || mode == 3 || mode == 1)
						util::stream_format(stream, ")");

					util::stream_format(stream, ")->RAM(%s)",INDIRECT(r2|(6<<6),0));
				}
				break;

				case 0x0c:  /* MOV RAM->BRAM */
					util::stream_format(stream, "MOV RAM(0x%x)->BRAM(0x%x)",r1,r2);
				break;

				case 0x0f:  /* MOV RAMInd->BRAMInd */
					util::stream_format(stream, "MOV RAM(%s)->BRAM(%s)",INDIRECT(r1,1),INDIRECT(r2|(6<<6),0));
				break;

				case 0x10:  /* MOV BRAMInd->RAM */
					util::stream_format(stream, "MOV BRAM(%s)->RAM(0x%x)",INDIRECT(r1,1),r2);
				break;

				case 0x13:  /* MOV BRAMInd->RAMInd */
					util::stream_format(stream, "MOV BRAM(%s)->RAM(%s)",INDIRECT(r1,1),INDIRECT(r2|(6<<6),0));
				break;

				case 0x1c:  /* MOV Reg->RAMInd */
					if ( ( r2 >> 6 ) & 0x01)
					{
						util::stream_format(stream, "MOV %s->EXT(EB+%s)",REGS(r2,1),INDIRECT(r1,0));
					}
					else
					{
						util::stream_format(stream, "MOV %s->RAM(%s)",REGS(r2,1),INDIRECT(r1,0));
					}
				break;

				case 0x1d:  /* MOV RAM->Reg */
				{
					if ( r1 & 0x180 )
					{
						util::stream_format(stream, "MOV RAM(%s)->%s",REGS(r1,0),REGS(r2,0));
					}
					else
					{
						util::stream_format(stream, "MOV RAM(0x%x)->%s",r1,REGS(r2,0));
					}
				}
				break;

				case 0x1e:  /* External->Reg */
				{
					int     mode2 = (r2 >> 6) & 1;
					util::stream_format(stream, "MOV EXT(EB+%s)->%s",INDIRECT(r1,mode2),REGS(r2,0));
				}
				break;

				case 0x1f:  /* MOV Reg->Reg */
					if ( !(r1 == 0x10 && r2 == 0x0f) )
					{
						util::stream_format(stream, "MOV %s->%s",REGS(r1,1),REGS(r2,0));
					}
				break;

				default:
					util::stream_format(stream, "UNKMV (0x%x)",op);
				break;
			}
		}
		break;

		case 0x0e:  /* Load 24 bit val */
		{
			uint32_t sub=(opcode>>24)&0x3;
			static const char regs[4] = { 'p', 'a', 'b', 'd' };

			util::stream_format(stream, "LDIMM24 0x%X->%c",opcode&0xffffff, regs[sub]);
		}
		break;

		case 0x0f:  /* repeat */
		{
			uint32_t  alu = ( opcode >> 20 ) & 0x1f;
			uint32_t  sub2 = ( opcode >> 16 ) & 0x0f;

			if ( alu != 0 )
				util::stream_format(stream, "%s, ", ALU(alu) );

			if ( sub2 == 0x00 )
			{
				util::stream_format(stream, "CLEAR ");

				switch( opcode & 0x3f )
				{
					case 0x04: util::stream_format(stream, "a" ); break;
					case 0x08: util::stream_format(stream, "b" ); break;
					case 0x10: util::stream_format(stream, "d" ); break;
					default: util::stream_format(stream, "UNKNOWN REG(%x)",opcode&0x3F); break;
				}
			}
			else if ( sub2 == 0x02 )
				util::stream_format(stream, "CLRFLAG 0x%x",opcode&0xffff);
			else if ( sub2==0x4 )
			{
				if ( (opcode & 0xfff) == 0 )
					util::stream_format(stream, "REP 0x100");
				else
					util::stream_format(stream, "REP 0x%x",opcode&0xff);
			}
			else if ( sub2 == 0x06 )
				util::stream_format(stream, "SETFLAG 0x%x",opcode&0xffff);
		}
		break;

		case 0x10:
		{
			uint32_t  dst=(opcode>>24)&0xf;
			uint32_t  imm=(opcode)&0xFFFF;

			if ( dst <= 3 )
				util::stream_format(stream, "LDIMM 0x%x->r%d",imm,dst);
			else
				util::stream_format(stream, "LDIMM 0x%x->UNKDST(0x%x)",imm,dst);
		}
		break;

		case 0x13:
		{
			uint32_t sub = ( opcode >> 24 ) & 0x03;

			util::stream_format(stream, "LDIMM 0x%X->",opcode&0xffffff);

			if ( sub == 0 ) util::stream_format(stream, "r12");
			else if ( sub == 1 ) util::stream_format(stream, "r13");
			else util::stream_format(stream, "UNKREG(%x)", sub);
		}
		break;

		case 0x14:
		{
			uint32_t sub = ( opcode >> 24 ) & 0x03;

			util::stream_format(stream, "LDIMM 0x%X->",opcode&0xffffff);

			if ( sub == 0 ) util::stream_format(stream, "a.exp");
			else if ( sub == 1 ) util::stream_format(stream, "a.e");
			else if ( sub == 2 ) util::stream_format(stream, "a.m");
			else util::stream_format(stream, "UNKREG(%x)", sub);
		}
		break;

		case 0x15:
		{
			uint32_t sub = ( opcode >> 24 ) & 0x03;

			util::stream_format(stream, "LDIMM 0x%X->",opcode&0xffffff);

			if ( sub == 0 ) util::stream_format(stream, "b.exp");
			else if ( sub == 1 ) util::stream_format(stream, "b.e");
			else if ( sub == 2 ) util::stream_format(stream, "b.m");
			else util::stream_format(stream, "UNKREG(%x)", sub);
		}
		break;

		case 0x16:
		{
			uint32_t sub = ( opcode >> 24 ) & 0x03;

			util::stream_format(stream, "LDIMM 0x%X->",opcode&0xffffff);

			if ( sub == 2 ) util::stream_format(stream, "d.e");
			else if ( sub == 3 ) util::stream_format(stream, "d.m");
			else util::stream_format(stream, "UNKREG(%x)", sub);
		}
		break;

		case 0x17:
		{
			uint32_t sub = ( opcode >> 24 ) & 0x03;

			util::stream_format(stream, "LDIMM 0x%X->",opcode&0xffffff);

			if ( sub == 0x03 ) util::stream_format(stream, "shift");
			else util::stream_format(stream, "UNKREG(%x)", sub);
		}
		break;

		case 0x18:
		{
			uint32_t sub = ( opcode >> 24 ) & 0x03;

			util::stream_format(stream, "LDIMM24 0x%X->",opcode&0xffffff);

			if ( sub == 0x03 ) util::stream_format(stream, "EB");
			else util::stream_format(stream, "UNKREG(%x)", sub);
		}
		break;

		case 0x2f:
		{
			uint32_t cond = ( opcode >> 20 ) & 0x1f;
			uint32_t subtype = ( opcode >> 16 ) & 0x0f;
			uint32_t data = opcode & 0xffff;

			switch( subtype )
			{
				case 0:
					util::stream_format(stream, "BRIF %s 0x%X", COND(cond), data);
				break;

				case 2:
					util::stream_format(stream, "BRIF %s ", COND(cond));
					if ( data & 0x4000 )
						util::stream_format(stream, "%s",REGS(data&0x3f,0));
					else
						util::stream_format(stream, "RAM(0x%x)",data);
				break;

				case 4:
					util::stream_format(stream, "BSIF %s 0x%X", COND(cond), data);
				break;

				case 0x6:
					util::stream_format(stream, "BSIF %s ", COND(cond));
					if ( data & 0x4000 )
						util::stream_format(stream, "%s",REGS(data&0x3f,0));
					else
						util::stream_format(stream, "RAM(0x%x)",data);
				break;

				case 0xa:
					util::stream_format(stream, "RTIF %s", COND(cond));
				break;

				case 0xc:
					util::stream_format(stream, "LDIF %s RAM(0x%x)->%s", COND(cond),data&0x1ff,REGS((data>>9)&0x3f,0));
				break;

				case 0xe:
					util::stream_format(stream, "RIIF %s", COND(cond));
				break;

				default:
					util::stream_format(stream, "UNKG5 (%x cond %x)",subtype,cond);
				break;
			}
		}
		break;

		case 0x3f:
		{
			uint32_t cond = ( opcode >> 20 ) & 0x1f;
			uint32_t subtype = ( opcode >> 16 ) & 0x0f;
			uint32_t data = opcode & 0xffff;

			switch( subtype )
			{
				case 0:
					util::stream_format(stream, "BRUL %s 0x%X", COND(cond), data);
				break;

				case 2:
					util::stream_format(stream, "BRUL %s ", COND(cond));
					if ( data & 0x4000 )
						util::stream_format(stream, "%s",REGS(data&0x3f,0));
					else
						util::stream_format(stream, "RAM(0x%x)",data);
				break;

				case 4:
					util::stream_format(stream, "BSUL %s 0x%X", COND(cond), data);
				break;

				case 0x6:
					util::stream_format(stream, "BSUL %s ", COND(cond));
					if ( data & 0x4000 )
						util::stream_format(stream, "%s",REGS(data&0x3f,0));
					else
						util::stream_format(stream, "RAM(0x%x)",data);
				break;

				case 0xa:
					util::stream_format(stream, "RTUL %s", COND(cond));
				break;

				case 0xc:
					util::stream_format(stream, "LDUL %s RAM(0x%x)->%s", COND(cond),data&0x1ff,REGS((data>>9)&0x3f,0));
				break;

				case 0xe:
					util::stream_format(stream, "RIUL %s", COND(cond));
				break;

				default:
					util::stream_format(stream, "UNKG5 (%x cond %x)",subtype,cond);
				break;
			}
		}
		break;

		default:
			util::stream_format(stream, "UNKOP");
		break;
	}

	return (1 | DASMFLAG_SUPPORTED);
}

CPU_DISASSEMBLE(mb86233)
{
	uint32_t op = *(uint32_t *)oprom;
	op = little_endianize_int32(op);
	return dasm_mb86233(stream, op);
}
