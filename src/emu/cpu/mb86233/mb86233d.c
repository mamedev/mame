// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
#include "emu.h"
#include "debugger.h"
#include "mb86233.h"

static char * COND(unsigned int cond)
{
	static char bufs[4][256];
	static int bufindex = 0;
	char *buf = &bufs[bufindex++][0];

	bufindex &= 3;

	switch(cond)
	{
		case 0x16:
			sprintf(buf,"always");
			return buf;

		case 0x00:
			sprintf(buf,"eq");
			return buf;

		case 0x01:
			sprintf(buf,"ge");
			return buf;

		case 0x02:
			sprintf(buf,"le");
			return buf;

		case 0x06:
			sprintf(buf,"never");
			return buf;

		case 0x10:
			sprintf(buf,"(--r12)!=0");
			return buf;

		case 0x11:
			sprintf(buf,"(--r13)!=0");
			return buf;
	}

	sprintf(buf,"unk (%x)",cond);
	return buf;
}

static char * REGS( UINT32 reg, int IsSource )
{
	static char bufs[4][256];
	static int bufindex = 0;
	char *buf = &bufs[bufindex++][0];
	int         mode = (reg >> 6 ) & 0x07;

	bufindex &= 3;

	reg &= 0x3f;

	if ( mode == 0 || mode == 1 || mode == 3 )
	{
		if ( reg < 0x10 )
		{
			sprintf(buf,"r%d",reg);
			return buf;
		}

		switch(reg)
		{
			case 0x10:
				sprintf(buf,"a");
			break;

			case 0x11:
				sprintf(buf,"a.e");
			break;

			case 0x12:
				sprintf(buf,"a.m");
			break;

			case 0x13:
				sprintf(buf,"b");
			break;

			case 0x14:
				sprintf(buf,"b.e");
			break;

			case 0x15:
				sprintf(buf,"b.m");
			break;

			case 0x19:
				sprintf(buf,"d");
			break;

			case 0x1a:
				sprintf(buf,"d.e");
			break;

			case 0x1b:
				sprintf(buf,"d.m");
			break;

			case 0x1c:
				sprintf(buf,"p");
			break;

			case 0x1d:
				sprintf(buf,"p.e");
			break;

			case 0x1e:
				sprintf(buf,"p.m");
			break;

			case 0x1f:
				sprintf(buf,"shift");
			break;

			case 0x20:
				sprintf(buf,"parport");
			break;

			case 0x21:
				sprintf(buf,"FIn");
			break;

			case 0x22:
				sprintf(buf,"FOut");
			break;

			case 0x23:
				sprintf(buf,"EB");
			break;

			default:
				sprintf(buf,"Unkreg (%x)",reg);
			break;
		}
	}
	else if ( mode == 2 )
	{
		char *p = buf;

		p += sprintf(p,"0x%x+",reg & 0x1f);

		if ( IsSource )
		{
			if ( !( reg & 0x20 ) )
				p += sprintf(p,"r0+");

			p += sprintf(p,"r2");
		}
		else
		{
			if ( !( reg & 0x20 ) )
				p += sprintf(p,"r1+");

			p += sprintf(p,"r3");
		}
	}
	else if ( mode == 6 )
	{
		char *p = buf;

		if ( IsSource )
		{
			if ( !( reg & 0x20 ) )
				p += sprintf(p,"r0+");

			p += sprintf(p,"r2");
		}
		else
		{
			if ( !( reg & 0x20 ) )
				p += sprintf(p,"r1+");

			p += sprintf(p,"r3");
		}

		if ( reg & 0x10 )
			p += sprintf(p,"--%d", 0x20 - ( reg & 0x1f ) );
		else
			p += sprintf(p,"++%d", reg & 0x1f );
	}
	else
	{
		sprintf(buf,"UNKMOD %x (0x%x)", mode, reg);
	}

	return buf;
}

static char * INDIRECT( UINT32 reg, int IsSource )
{
	static char bufs[4][256];
	static int bufindex = 0;
	char *buf = &bufs[bufindex++][0];
	int         mode = ( reg >> 6 ) & 0x07;

	bufindex &= 3;

	if ( mode == 0 || mode == 3 || mode == 1)
	{
		sprintf(buf,"0x%x",reg);
	}
	else if ( mode == 2 )
	{
		char *p = buf;

		p += sprintf(p,"0x%x+",reg&0x1f);

		if ( IsSource )
		{
			if ( !(reg & 0x20) )
				p += sprintf(p,"r0+");

			p += sprintf(p,"r2");
		}
		else
		{
			if ( !(reg & 0x20) )
				p += sprintf(p,"r1+");

			p += sprintf(p,"r3");
		}
	}
	else if ( mode == 6 || mode == 7 )
	{
		char *p = buf;

		if ( IsSource )
		{
			if ( !( reg & 0x20 ) )
				p += sprintf(p,"r0+");

			p += sprintf(p,"r2");
		}
		else
		{
			if( !( reg & 0x20 ) )
				p += sprintf(p,"r1+");

			p += sprintf(p,"r3");
		}

		if ( reg & 0x10 )
			p += sprintf(p,"--%d",0x20 - ( reg & 0x1f ));
		else
			p += sprintf(p,"++%d",reg & 0x1f);
	}
	else
	{
		sprintf(buf,"UNKMOD %x (0x%x)", mode, reg);
	}

	return buf;
}


static char * ALU( UINT32 alu)
{
	static char bufs[4][256];
	static int bufindex = 0;
	char *buf = &bufs[bufindex++][0];

	bufindex &= 3;

	switch( alu )
	{
		case 0x0:
			buf[0] = 0;
		break;

		case 0x1:
			sprintf(buf,"d=d&a");
		break;

		case 0x2:
			sprintf(buf,"d=d|a");
		break;

		case 0x3:
			sprintf(buf,"d=d^a");
		break;

		case 0x5:
			sprintf(buf,"cmp d,a");
		break;

		case 0x6:
			sprintf(buf,"d=d+a");
		break;

		case 0x7:
			sprintf(buf,"d=d-a");
		break;

		case 0x8:
			sprintf(buf,"p=a*b");
		break;

		case 0x9:
			sprintf(buf,"d=d+p, p=a*b");
		break;

		case 0xa:
			sprintf(buf,"d=d-p, p=a*b");
		break;

		case 0xb:
			sprintf(buf,"d=fabs d");
		break;

		case 0xc:
			sprintf(buf,"d=d+p");
		break;

		case 0xd:
			sprintf(buf,"d=p, p=a*b");
		break;

		case 0xe:
			sprintf(buf,"d=float(d)");
		break;

		case 0xf:
			sprintf(buf,"d=int(d)");
		break;

		case 0x10:
			sprintf(buf,"d=d/a");
		break;

		case 0x11:
			sprintf(buf,"d=-d");
		break;

		case 0x13:
			sprintf(buf,"d=a+b");
		break;

		case 0x14:
			sprintf(buf,"d=b-a");
		break;

		case 0x16:
			sprintf(buf,"d=(lsr d,shift)");
		break;

		case 0x17:
			sprintf(buf,"d=(lsl d,shift)");
		break;

		case 0x18:
			sprintf(buf,"d=(asr d,shift)");
		break;

		case 0x1a:
			sprintf(buf,"d=d+a (int)");
		break;

		case 0x1b:
			sprintf(buf,"d=d-a (int)");
		break;

		default:
			sprintf(buf,"ALU UNK(%x)",alu);
		break;
	}

	return buf;
}

static unsigned dasm_mb86233(char *buffer, UINT32 opcode )
{
	char *p = buffer;
	UINT32  grp = ( opcode >> 26 ) & 0x3f;

	switch( grp )
	{
		case 0x0:   /* Dual move */
		{
			UINT32 r1=opcode & 0x1ff;
			UINT32 r2=(opcode>>9) & 0x7f;
			UINT32 alu=(opcode>>21) & 0x1f;
			UINT32 op=(opcode>>16) & 0x1f;

			if ( alu != 0 )
				p += sprintf(p, "%s, ", ALU(alu) );

			switch( op )
			{
				case 0x0c:  /* a = RAM[addr], b = BRAM[addr] */
					p += sprintf(p,"LAB RAM(0x%x)->a,BRAM(0x%x)->b",r1,r2);
				break;

				case 0x0d:  /* a = RAM[addr], b = BRAM[addr] */
					p += sprintf(p,"LAB RAM(0x%x)->a,BRAM(%s)->b",r1,INDIRECT(r2|(2<<6),0));
				break;

				case 0x0f:  /* a = RAM[addr], b = BRAM[reg] */
					p += sprintf(p,"LAB RAM(0x%x)->a,BRAM(%s)->b",r1,INDIRECT(r2|(6<<6),0));
				break;

				case 0x10:  /* a = BRAM[reg], b = RAM[addr] */
					p += sprintf(p,"LAB BRAM(%s)->a,RAM(0x%x)->b",INDIRECT(r1,1),r2);
				break;

				default:
					p += sprintf(p,"UNKDUAL (%x)",op);
				break;
			}
		}
		break;

		case 0x7:   /* LD/MOV */
		{
			UINT32 r1=opcode & 0x1ff;
			UINT32 r2=(opcode>>9) & 0x7f;
			UINT32 alu=(opcode>>21) & 0x1f;
			UINT32 op=(opcode>>16) & 0x1f;

			if ( alu != 0 )
			{
				p += sprintf(p, "%s", ALU(alu) );

				if ( !(op == 0x1f && r1 == 0x10 && r2 == 0x0f) )
					p += sprintf(p, ", ");
			}

			switch(op)
			{
				case 0x03:  /* RAM->External Indirect */
				{
					p += sprintf(p,"MOV RAM(0x%x)->E(EB+%s)",r1,INDIRECT(r2|(6<<6),0));
				}
				break;

				case 0x04:  /* MOV RAM->External */
				{
					p += sprintf(p,"MOV RAM(0x%x)->E(EB+0x%x)",r1,r2);
				}
				break;

				case 0x07:  /* RAMInd->External */
				{
					p += sprintf(p,"MOV RAM(%s)->E(EB+%s)",INDIRECT(r1,1),INDIRECT(r2|(6<<6),0));
				}
				break;

				case 0x08:  /* External->RAM */
				{
					p += sprintf(p,"MOV EXT(EB+");
					p += sprintf(p,"%s",INDIRECT(r1,1));
					p += sprintf(p,")->RAM(0x%x)",r2);
				}
				break;

				case 0x0b:  /* External->RAMInd */
				{
					int mode = ( r1 >> 6 ) & 0x07;

					p += sprintf(p,"MOV EXT(EB+");

					if ( mode == 0 || mode == 3 || mode == 1 )
						p += sprintf(p,"RAM(");

					p += sprintf(p,"%s",INDIRECT(r1,1));

					if ( mode == 0 || mode == 3 || mode == 1)
						p += sprintf(p,")");

					p += sprintf(p,")->RAM(%s)",INDIRECT(r2|(6<<6),0));
				}
				break;

				case 0x0c:  /* MOV RAM->BRAM */
					p += sprintf(p,"MOV RAM(0x%x)->BRAM(0x%x)",r1,r2);
				break;

				case 0x0f:  /* MOV RAMInd->BRAMInd */
					p += sprintf(p,"MOV RAM(%s)->BRAM(%s)",INDIRECT(r1,1),INDIRECT(r2|(6<<6),0));
				break;

				case 0x10:  /* MOV BRAMInd->RAM */
					p += sprintf(p,"MOV BRAM(%s)->RAM(0x%x)",INDIRECT(r1,1),r2);
				break;

				case 0x13:  /* MOV BRAMInd->RAMInd */
					p += sprintf(p,"MOV BRAM(%s)->RAM(%s)",INDIRECT(r1,1),INDIRECT(r2|(6<<6),0));
				break;

				case 0x1c:  /* MOV Reg->RAMInd */
					if ( ( r2 >> 6 ) & 0x01)
					{
						p += sprintf(p,"MOV %s->EXT(EB+%s)",REGS(r2,1),INDIRECT(r1,0));
					}
					else
					{
						p += sprintf(p,"MOV %s->RAM(%s)",REGS(r2,1),INDIRECT(r1,0));
					}
				break;

				case 0x1d:  /* MOV RAM->Reg */
				{
					if ( r1 & 0x180 )
					{
						p += sprintf(p,"MOV RAM(%s)->%s",REGS(r1,0),REGS(r2,0));
					}
					else
					{
						p += sprintf(p,"MOV RAM(0x%x)->%s",r1,REGS(r2,0));
					}
				}
				break;

				case 0x1e:  /* External->Reg */
				{
					int     mode2 = (r2 >> 6) & 1;
					p += sprintf(p,"MOV EXT(EB+%s)->%s",INDIRECT(r1,mode2),REGS(r2,0));
				}
				break;

				case 0x1f:  /* MOV Reg->Reg */
					if ( !(r1 == 0x10 && r2 == 0x0f) )
					{
						p += sprintf(p,"MOV %s->%s",REGS(r1,1),REGS(r2,0));
					}
				break;

				default:
					p += sprintf(p,"UNKMV (0x%x)",op);
				break;
			}
		}
		break;

		case 0x0e:  /* Load 24 bit val */
		{
			UINT32 sub=(opcode>>24)&0x3;
			static const char regs[4] = { 'p', 'a', 'b', 'd' };

			p += sprintf(p,"LDIMM24 0x%X->%c",opcode&0xffffff, regs[sub]);
		}
		break;

		case 0x0f:  /* repeat */
		{
			UINT32  alu = ( opcode >> 20 ) & 0x1f;
			UINT32  sub2 = ( opcode >> 16 ) & 0x0f;

			if ( alu != 0 )
				p += sprintf(p, "%s, ", ALU(alu) );

			if ( sub2 == 0x00 )
			{
				p += sprintf(p,"CLEAR ");

				switch( opcode & 0x3f )
				{
					case 0x04: p += sprintf(p, "a" ); break;
					case 0x08: p += sprintf(p, "b" ); break;
					case 0x10: p += sprintf(p, "d" ); break;
					default: p += sprintf(p, "UNKNOWN REG(%x)",opcode&0x3F); break;
				}
			}
			else if ( sub2 == 0x02 )
				p += sprintf(p,"CLRFLAG 0x%x",opcode&0xffff);
			else if ( sub2==0x4 )
			{
				if ( (opcode & 0xfff) == 0 )
					p += sprintf(p,"REP 0x100");
				else
					p += sprintf(p,"REP 0x%x",opcode&0xff);
			}
			else if ( sub2 == 0x06 )
				p += sprintf(p,"SETFLAG 0x%x",opcode&0xffff);
		}
		break;

		case 0x10:
		{
			UINT32  dst=(opcode>>24)&0xf;
			UINT32  imm=(opcode)&0xFFFF;

			if ( dst <= 3 )
				p += sprintf(p,"LDIMM 0x%x->r%d",imm,dst);
			else
				p += sprintf(p,"LDIMM 0x%x->UNKDST(0x%x)",imm,dst);
		}
		break;

		case 0x13:
		{
			UINT32 sub = ( opcode >> 24 ) & 0x03;

			p += sprintf(p,"LDIMM 0x%X->",opcode&0xffffff);

			if ( sub == 0 ) p += sprintf(p,"r12");
			else if ( sub == 1 ) p += sprintf(p,"r13");
			else p += sprintf(p,"UNKREG(%x)", sub);
		}
		break;

		case 0x14:
		{
			UINT32 sub = ( opcode >> 24 ) & 0x03;

			p += sprintf(p,"LDIMM 0x%X->",opcode&0xffffff);

			if ( sub == 0 ) p += sprintf(p,"a.exp");
			else if ( sub == 1 ) p += sprintf(p,"a.e");
			else if ( sub == 2 ) p += sprintf(p,"a.m");
			else p += sprintf(p,"UNKREG(%x)", sub);
		}
		break;

		case 0x15:
		{
			UINT32 sub = ( opcode >> 24 ) & 0x03;

			p += sprintf(p,"LDIMM 0x%X->",opcode&0xffffff);

			if ( sub == 0 ) p += sprintf(p,"b.exp");
			else if ( sub == 1 ) p += sprintf(p,"b.e");
			else if ( sub == 2 ) p += sprintf(p,"b.m");
			else p += sprintf(p,"UNKREG(%x)", sub);
		}
		break;

		case 0x16:
		{
			UINT32 sub = ( opcode >> 24 ) & 0x03;

			p += sprintf(p,"LDIMM 0x%X->",opcode&0xffffff);

			if ( sub == 2 ) p += sprintf(p,"d.e");
			else if ( sub == 3 ) p += sprintf(p,"d.m");
			else p += sprintf(p,"UNKREG(%x)", sub);
		}
		break;

		case 0x17:
		{
			UINT32 sub = ( opcode >> 24 ) & 0x03;

			p += sprintf(p,"LDIMM 0x%X->",opcode&0xffffff);

			if ( sub == 0x03 ) p += sprintf(p,"shift");
			else p += sprintf(p,"UNKREG(%x)", sub);
		}
		break;

		case 0x18:
		{
			UINT32 sub = ( opcode >> 24 ) & 0x03;

			p += sprintf(p,"LDIMM24 0x%X->",opcode&0xffffff);

			if ( sub == 0x03 ) p += sprintf(p,"EB");
			else p += sprintf(p,"UNKREG(%x)", sub);
		}
		break;

		case 0x2f:
		{
			UINT32 cond = ( opcode >> 20 ) & 0x1f;
			UINT32 subtype = ( opcode >> 16 ) & 0x0f;
			UINT32 data = opcode & 0xffff;

			switch( subtype )
			{
				case 0:
					p += sprintf(p,"BRIF %s 0x%X", COND(cond), data);
				break;

				case 2:
					p += sprintf(p,"BRIF %s ", COND(cond));
					if ( data & 0x4000 )
						p += sprintf(p,"%s",REGS(data&0x3f,0));
					else
						p += sprintf(p,"RAM(0x%x)",data);
				break;

				case 4:
					p += sprintf(p,"BSIF %s 0x%X", COND(cond), data);
				break;

				case 0x6:
					p += sprintf(p,"BSIF %s ", COND(cond));
					if ( data & 0x4000 )
						p += sprintf(p,"%s",REGS(data&0x3f,0));
					else
						p += sprintf(p,"RAM(0x%x)",data);
				break;

				case 0xa:
					p += sprintf(p,"RTIF %s", COND(cond));
				break;

				case 0xc:
					p += sprintf(p,"LDIF %s RAM(0x%x)->%s", COND(cond),data&0x1ff,REGS((data>>9)&0x3f,0));
				break;

				case 0xe:
					p += sprintf(p,"RIIF %s", COND(cond));
				break;

				default:
					p += sprintf(p,"UNKG5 (%x cond %x)",subtype,cond);
				break;
			}
		}
		break;

		case 0x3f:
		{
			UINT32 cond = ( opcode >> 20 ) & 0x1f;
			UINT32 subtype = ( opcode >> 16 ) & 0x0f;
			UINT32 data = opcode & 0xffff;

			switch( subtype )
			{
				case 0:
					p += sprintf(p,"BRUL %s 0x%X", COND(cond), data);
				break;

				case 2:
					p += sprintf(p,"BRUL %s ", COND(cond));
					if ( data & 0x4000 )
						p += sprintf(p,"%s",REGS(data&0x3f,0));
					else
						p += sprintf(p,"RAM(0x%x)",data);
				break;

				case 4:
					p += sprintf(p,"BSUL %s 0x%X", COND(cond), data);
				break;

				case 0x6:
					p += sprintf(p,"BSUL %s ", COND(cond));
					if ( data & 0x4000 )
						p += sprintf(p,"%s",REGS(data&0x3f,0));
					else
						p += sprintf(p,"RAM(0x%x)",data);
				break;

				case 0xa:
					p += sprintf(p,"RTUL %s", COND(cond));
				break;

				case 0xc:
					p += sprintf(p,"LDUL %s RAM(0x%x)->%s", COND(cond),data&0x1ff,REGS((data>>9)&0x3f,0));
				break;

				case 0xe:
					p += sprintf(p,"RIUL %s", COND(cond));
				break;

				default:
					p += sprintf(p,"UNKG5 (%x cond %x)",subtype,cond);
				break;
			}
		}
		break;

		default:
			p += sprintf(p,"UNKOP");
		break;
	}

	return (1 | DASMFLAG_SUPPORTED);
}

CPU_DISASSEMBLE( mb86233 )
{
	UINT32 op = *(UINT32 *)oprom;
	op = LITTLE_ENDIANIZE_INT32(op);
	return dasm_mb86233(buffer, op);
}
