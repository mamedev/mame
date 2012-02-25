#define	INC_8BIT(x) \
{ \
  register UINT8 r,f; \
  x++; \
  r=(x);  \
  f=(UINT8)(cpustate->F&FLAG_C); \
  if( r==0 )       f|=FLAG_Z; \
  if( (r&0xF)==0 ) f|=FLAG_H; \
  cpustate->F=f; \
}

#define	DEC_8BIT(x) \
{ \
  register UINT8 r,f; \
  x--; \
  r=(x);  \
  f=(UINT8)((cpustate->F&FLAG_C)|FLAG_N); \
  if( r==0 )       f|=FLAG_Z; \
  if( (r&0xF)==0xF ) f|=FLAG_H; \
  cpustate->F=f; \
}

#define	ADD_HL_RR(x) \
{ \
  register UINT32 r1,r2; \
  register UINT8 f; \
  r1=((cpustate->H<<8)|cpustate->L)+(x); \
  r2=(((cpustate->H<<8)|cpustate->L)&0xFFF)+((x)&0xFFF); \
  f=(UINT8)(cpustate->F&FLAG_Z); \
  if( r1>0xFFFF ) f|=FLAG_C; \
  if( r2>0x0FFF ) f|=FLAG_H; \
  cpustate->L = r1; \
  cpustate->H = r1 >> 8; \
  cpustate->F=f; \
}

#define	ADD_A_X(x) \
{ \
  register UINT16 r1,r2; \
  register UINT8 f; \
  r1=(UINT16)((cpustate->A&0xF)+((x)&0xF)); \
  r2=(UINT16)(cpustate->A+(x)); \
  cpustate->A=(UINT8)r2; \
  if( ((UINT8)r2)==0 ) f=FLAG_Z; \
    else f=0; \
  if( r2>0xFF ) f|=FLAG_C; \
  if( r1>0xF )  f|=FLAG_H; \
  cpustate->F=f; \
}

#define	SUB_A_X(x) \
{ \
  register UINT16 r1,r2; \
  register UINT8 f; \
  r1=(UINT16)((cpustate->A&0xF)-((x)&0xF)); \
  r2=(UINT16)(cpustate->A-(x)); \
  cpustate->A=(UINT8)r2; \
  if( ((UINT8)r2)==0 ) f=FLAG_N|FLAG_Z; \
    else f=FLAG_N; \
  if( r2>0xFF ) f|=FLAG_C; \
  if( r1>0xF )  f|=FLAG_H; \
  cpustate->F=f; \
}

/*
   #define      CP_A_X(x) \
   { \
   register UINT16 r; \
   register UINT8 f; \
   r=(UINT16)(cpustate->A-(x)); \
   if( ((UINT8)r)==0 ) \
   f=FLAG_N|FLAG_Z; \
   else \
   f=FLAG_N; \
   f|=(UINT8)((r>>8)&FLAG_C); \
   if( (r^cpustate->A^(x))&0x10 ) \
   f|=FLAG_H; \
   cpustate->F=f; \
   }
 */

#define	CP_A_X(x) \
{ \
  register UINT16 r1,r2; \
  register UINT8 f; \
  r1=(UINT16)((cpustate->A&0xF)-((x)&0xF)); \
  r2=(UINT16)(cpustate->A-(x)); \
  if( ((UINT8)r2)==0 ) f=FLAG_N|FLAG_Z; \
    else f=FLAG_N; \
  if( r2>0xFF ) f|=FLAG_C; \
  if( r1>0xF )  f|=FLAG_H; \
  cpustate->F=f; \
}

#define	SBC_A_X(x) \
{ \
  register UINT16 r1,r2; \
  register UINT8 f; \
  r1=(UINT16)((cpustate->A&0xF)-((x)&0xF)-((cpustate->F&FLAG_C)?1:0)); \
  r2=(UINT16)(cpustate->A-(x)-((cpustate->F&FLAG_C)?1:0)); \
  cpustate->A=(UINT8)r2; \
  if( ((UINT8)r2)==0 ) f=FLAG_N|FLAG_Z; \
    else f=FLAG_N; \
  if( r2>0xFF ) f|=FLAG_C; \
  if( r1>0xF )  f|=FLAG_H; \
  cpustate->F=f; \
}

#define	ADC_A_X(x) \
{ \
  register UINT16 r1,r2;  \
  register UINT8 f; \
  r1=(UINT16)((cpustate->A&0xF)+((x)&0xF)+((cpustate->F&FLAG_C)?1:0));  \
  r2=(UINT16)(cpustate->A+(x)+((cpustate->F&FLAG_C)?1:0)); \
  if( (cpustate->A=(UINT8)r2)==0 ) f=FLAG_Z; \
    else f=0; \
  if( r2>0xFF )	f|=FLAG_C; \
  if( r1>0xF )	f|=FLAG_H; \
  cpustate->F=f; \
}

#define	AND_A_X(x) \
  if( (cpustate->A&=(x))==0 ) \
    cpustate->F=FLAG_H|FLAG_Z; \
  else \
    cpustate->F=FLAG_H;

#define XOR_A_X(x) \
  if( (cpustate->A^=(x))==0 ) \
    cpustate->F=FLAG_Z; \
  else \
    cpustate->F=0;

#define	OR_A_X(x) \
  if( (cpustate->A|=(x))==0 ) \
    cpustate->F=FLAG_Z; \
  else \
    cpustate->F=0;

#define POP(x,y) \
	y = mem_ReadByte( cpustate, cpustate->SP++ ); \
	x = mem_ReadByte( cpustate, cpustate->SP++ );

#define PUSH(x,y) \
	cpustate->SP--; \
	mem_WriteByte( cpustate, cpustate->SP, x ); \
	cpustate->SP--; \
	mem_WriteByte( cpustate, cpustate->SP, y );

case 0x00: /*      NOP */
  break;
case 0x01: /*      LD BC,n16 */
	cpustate->C = mem_ReadOp (cpustate);
	cpustate->B = mem_ReadOp (cpustate);
	break;
case 0x02: /*      LD (BC),A */
  mem_WriteByte (cpustate, ( cpustate->B << 8 ) | cpustate->C, cpustate->A);
  break;
case 0x03: /*      INC BC */
#if 0				/* FIXME ?? do we want to support this? (bug emulation) */
  if (cpustate->B == 0xFE)
  {
    trash_sprites (state);
  }
#endif
	cpustate->C++;
	if ( cpustate->C == 0 )
	{
		cpustate->B++;
	}
	CYCLES_PASSED( 4 );
	break;
case 0x04: /*      INC B */
	INC_8BIT (cpustate->B)
	break;
case 0x05: /*      DEC B */

  DEC_8BIT (cpustate->B)
  break;
case 0x06: /*      LD B,n8 */

  cpustate->B = mem_ReadByte (cpustate, cpustate->PC++);
  break;
case 0x07: /*      RLCA */

  cpustate->A = (UINT8) ((cpustate->A << 1) | (cpustate->A >> 7));
  if (cpustate->A & 1)
  {
    cpustate->F = FLAG_C;
  }
  else
  {
    cpustate->F = 0;
  }
  break;
case 0x08: /*      LD (n16),SP */

  mem_WriteWord (cpustate, mem_ReadWord (cpustate, cpustate->PC), cpustate->SP);
  cpustate->PC += 2;
  break;
case 0x09: /*      ADD HL,BC */
	ADD_HL_RR ((cpustate->B<<8)|cpustate->C)
	CYCLES_PASSED( 4 );
	break;
case 0x0A: /*      LD A,(BC) */

  cpustate->A = mem_ReadByte (cpustate, (cpustate->B<<8)|cpustate->C);
  break;
case 0x0B: /*      DEC BC */

#if 0				/* FIXME ?? do we want to support this? (bug emulation) */
  if (cpustate->B == 0xFE)
  {
    trash_sprites (state);
  }
#endif
	cpustate->C--;
	if ( cpustate->C == 0xFF )
	{
		cpustate->B--;
	}
	CYCLES_PASSED( 4 );
	break;
case 0x0C: /*      INC C */

  INC_8BIT (cpustate->C)
  break;
case 0x0D: /*      DEC C */

  DEC_8BIT (cpustate->C)
  break;
case 0x0E: /*      LD C,n8 */

  cpustate->C = mem_ReadByte (cpustate, cpustate->PC++);
  break;
case 0x0F: /*      RRCA */

  cpustate->A = (UINT8) ((cpustate->A >> 1) | (cpustate->A << 7));
  cpustate->F = 0;
  if (cpustate->A & 0x80)
  {
    cpustate->F |= FLAG_C;
  }
  break;
case 0x10: /*      STOP */
  if ( cpustate->gb_speed_change_pending ) {
    cpustate->gb_speed = ( cpustate->gb_speed == 1 ) ? 2 : 1;
  }
  cpustate->gb_speed_change_pending = 0;
  break;
case 0x11: /*      LD DE,n16 */
	cpustate->E = mem_ReadOp (cpustate);
	cpustate->D = mem_ReadOp (cpustate);
	break;
case 0x12: /*      LD (DE),A */
  mem_WriteByte (cpustate, ( cpustate->D << 8 ) | cpustate->E, cpustate->A);
  break;
case 0x13: /*      INC DE */

#if 0				/* FIXME ?? do we want to support this? (bug emulation) */
  if (cpustate->D == 0xFE)
  {
    trash_sprites (state);
  }
#endif
	cpustate->E++;
	if ( cpustate->E == 0 )
	{
		cpustate->D++;
	}
	CYCLES_PASSED( 4 );
	break;
case 0x14: /*      INC D */

  INC_8BIT (cpustate->D)
  break;
case 0x15: /*      DEC D */

  DEC_8BIT (cpustate->D)
  break;
case 0x16: /*      LD D,n8 */

  cpustate->D = mem_ReadByte (cpustate, cpustate->PC++);
  break;
case 0x17: /*      RLA */

  x = (cpustate->A & 0x80) ? FLAG_C : 0;

  cpustate->A = (UINT8) ((cpustate->A << 1) | ((cpustate->F & FLAG_C) ? 1 : 0));
  cpustate->F = x;
  break;
case 0x18: /*      JR      n8 */
  {
	INT8 offset;

    offset = mem_ReadByte (cpustate, cpustate->PC++);
    cpustate->PC += offset;
	CYCLES_PASSED( 4 );
  }
  break;
case 0x19: /*      ADD HL,DE */
	ADD_HL_RR (( cpustate->D << 8 ) | cpustate->E)
	CYCLES_PASSED( 4 );
	break;
case 0x1A: /*      LD A,(DE) */

  cpustate->A = mem_ReadByte (cpustate, ( cpustate->D << 8 ) | cpustate->E);
  break;
case 0x1B: /*      DEC DE */

#if 0				/* FIXME ?? do we want to support this? (bug emulation) */
  if (cpustate->D == 0xFE)
  {
    trash_sprites (state);
  }
#endif
	cpustate->E--;
	if ( cpustate->E == 0xFF )
	{
		cpustate->D--;
	}
	CYCLES_PASSED( 4 );
	break;
case 0x1C: /*      INC E */

  INC_8BIT (cpustate->E)
  break;
case 0x1D: /*      DEC E */

  DEC_8BIT (cpustate->E)
  break;
case 0x1E: /*      LD E,n8 */

  cpustate->E = mem_ReadByte (cpustate, cpustate->PC++);
  break;
case 0x1F: /*      RRA */

  x = (cpustate->A & 1) ? FLAG_C : 0;

  cpustate->A = (UINT8) ((cpustate->A >> 1) | ((cpustate->F & FLAG_C) ? 0x80 : 0));
  cpustate->F = x;
  break;
case 0x20: /*      JR NZ,n8 */
	{
		INT8 offset = mem_ReadByte (cpustate, cpustate->PC++);
		if (! (cpustate->F & FLAG_Z) )
		{
			cpustate->PC += offset;
			CYCLES_PASSED( 4 );
		}
	}
	break;
case 0x21: /*      LD HL,n16 */
	cpustate->L = mem_ReadOp (cpustate);
	cpustate->H = mem_ReadOp (cpustate);
	break;
case 0x22: /*      LD (HL+),A */

#if 0				/* FIXME ?? do we want to support this? (bug emulation) */
  if (cpustate->H == 0xFE)
  {
    trash_sprites (state);
  }
#endif
	mem_WriteByte (cpustate, (cpustate->H << 8 ) | cpustate->L, cpustate->A);
	cpustate->L++;
	if ( cpustate->L == 0 )
	{
		cpustate->H++;
	}
	break;
case 0x23: /*      INC HL */

#if 0				/* FIXME ?? do we want to support this? (bug emulation) */
  if (cpustate->H == 0xFE)
  {
    trash_sprites (state);
  }
#endif
	cpustate->L++;
	if ( cpustate->L == 0 )
	{
		cpustate->H++;
	}
	CYCLES_PASSED( 4 );
	break;
case 0x24: /*      INC H */

  INC_8BIT (cpustate->H);
  break;
case 0x25: /*      DEC H */

  DEC_8BIT (cpustate->H);
  break;
case 0x26: /*      LD H,n8 */

  cpustate->H = mem_ReadByte (cpustate, cpustate->PC++);
  break;
case 0x27: /*      DAA */
	{
		int tmp = cpustate->A;

		if ( ! ( cpustate->F & FLAG_N ) ) {
			if ( ( cpustate->F & FLAG_H ) || ( tmp & 0x0F ) > 9 )
				tmp += 6;
			if ( ( cpustate->F & FLAG_C ) || tmp > 0x9F )
				tmp += 0x60;
		} else {
			if ( cpustate->F & FLAG_H ) {
				tmp -= 6;
				if ( ! ( cpustate->F & FLAG_C ) )
					tmp &= 0xFF;
			}
			if ( cpustate->F & FLAG_C )
					tmp -= 0x60;
		}
		cpustate->F &= ~ ( FLAG_H | FLAG_Z );
		if ( tmp & 0x100 )
			cpustate->F |= FLAG_C;
		cpustate->A = tmp & 0xFF;
		if ( ! cpustate->A )
			cpustate->F |= FLAG_Z;
	}
  break;
case 0x28: /*      JR Z,n8 */
	{
		INT8 offset = mem_ReadByte (cpustate, cpustate->PC++);;

		if (cpustate->F & FLAG_Z)
		{
			cpustate->PC += offset;
			CYCLES_PASSED( 4 );
		}
	}
	break;
case 0x29: /*      ADD HL,HL */
	ADD_HL_RR ((cpustate->H << 8 ) | cpustate->L)
	CYCLES_PASSED( 4 );
	break;
case 0x2A: /*      LD A,(HL+) */
#if 0				/* FIXME ?? do we want to support this? (bug emulation) */
  if (cpustate->H == 0xFE)
  {
    trash_sprites (state);
  }
#endif
	cpustate->A = mem_ReadByte (cpustate, ( cpustate->H << 8 ) | cpustate->L);
	cpustate->L++;
	if ( cpustate->L == 0 )
	{
		cpustate->H++;
	}
	break;
case 0x2B: /*      DEC HL */

#if 0				/* FIXME ?? do we want to support this? (bug emulation) */
  if (cpustate->H == 0xFE)
  {
    trash_sprites (state);
  }
#endif
	cpustate->L--;
	if ( cpustate->L == 0xFF )
	{
		cpustate->H--;
	}
	CYCLES_PASSED( 4 );
	break;
case 0x2C: /*      INC L */

  INC_8BIT (cpustate->L);
  break;
case 0x2D: /*      DEC L */

  DEC_8BIT (cpustate->L);
  break;
case 0x2E: /*      LD L,n8 */

  cpustate->L = mem_ReadByte (cpustate, cpustate->PC++);
  break;
case 0x2F: /*      CPL */

  cpustate->A = ~cpustate->A;
  cpustate->F |= FLAG_N | FLAG_H;
  break;
case 0x30: /*      JR NC,n8 */
	{
		INT8 offset = mem_ReadByte (cpustate, cpustate->PC++);

		if ( ! (cpustate->F & FLAG_C) )
		{
			cpustate->PC += offset;
			CYCLES_PASSED( 4 );
		}
	}
	break;
case 0x31: /*      LD SP,n16 */

  cpustate->SP = mem_ReadWord (cpustate, cpustate->PC);
  cpustate->PC += 2;
  break;
case 0x32: /*      LD (HL-),A */

#if 0				/* FIXME ?? do we want to support this? (bug emulation) */
  if (cpustate->H == 0xFE)
  {
    trash_sprites (state);
  }
#endif
	mem_WriteByte (cpustate, ( cpustate->H << 8 ) | cpustate->L, cpustate->A);
	cpustate->L--;
	if ( cpustate->L == 0xFF )
	{
		cpustate->H--;
	}
	break;
case 0x33: /*      INC SP */
	cpustate->SP += 1;
	CYCLES_PASSED( 4 );
	break;
case 0x34: /*      INC (HL) */
	{
		UINT16 addr = ( cpustate->H << 8 ) | cpustate->L;
		register UINT8 r, f;

		f = (UINT8) (cpustate->F & FLAG_C);
		r = mem_ReadByte (cpustate, addr);
		r += 1;
		mem_WriteByte (cpustate, addr, r);

		if (r == 0)
			f |= FLAG_Z;

		if ((r & 0xF) == 0)
			f |= FLAG_H;

		cpustate->F = f;
	}
	break;
case 0x35: /*      DEC (HL) */
	{
		UINT16 addr = ( cpustate->H << 8 ) | cpustate->L;
		register UINT8 r, f;

		f = (UINT8) ((cpustate->F & FLAG_C) | FLAG_N);
		r = mem_ReadByte (cpustate, addr);
		r -= 1;
		mem_WriteByte (cpustate, addr, r);

		if (r == 0)
			f |= FLAG_Z;

		if ((r & 0xF) == 0xF)
			f |= FLAG_H;

		cpustate->F = f;
	}
	break;
case 0x36: /*      LD (HL),n8 */
	{
		UINT8 v = mem_ReadByte (cpustate, cpustate->PC++);
		mem_WriteByte (cpustate, ( cpustate->H << 8 ) | cpustate->L, v);
	}
	break;
case 0x37: /*      SCF */

  cpustate->F = (UINT8) ((cpustate->F & FLAG_Z) | FLAG_C);
  break;
case 0x38: /*      JR C,n8 */
	{
		INT8 offset = mem_ReadByte (cpustate, cpustate->PC++);

		if (cpustate->F & FLAG_C)
		{
			cpustate->PC += offset;
			CYCLES_PASSED( 4 );
		}
	}
	break;
case 0x39: /*      ADD HL,SP */
	ADD_HL_RR (cpustate->SP)
	CYCLES_PASSED( 4 );
	break;
case 0x3A: /*      LD A,(HL-) */
#if 0				/* FIXME ?? do we want to support this? (bug emulation) */
  if (cpustate->H == 0xFE)
  {
    trash_sprites (state);
  }
#endif
	cpustate->A = mem_ReadByte (cpustate, ( cpustate->H << 8 ) | cpustate->L);
	cpustate->L--;
	if ( cpustate->L == 0xFF )
	{
		cpustate->H--;
	}
	break;
case 0x3B: /*      DEC SP */
	cpustate->SP -= 1;
	CYCLES_PASSED( 4 );
	break;
case 0x3C: /*      INC     A */

  INC_8BIT (cpustate->A);
  break;
case 0x3D: /*      DEC     A */

  DEC_8BIT (cpustate->A);
  break;
case 0x3E: /*      LD A,n8 */

  cpustate->A = mem_ReadByte (cpustate, cpustate->PC++);
  break;
case 0x3F: /*      CCF */

  cpustate->F = (UINT8) ((cpustate->F & FLAG_Z) | ((cpustate->F & FLAG_C) ? 0 : FLAG_C));
  break;
case 0x40: /*      LD B,B */
  break;
case 0x41: /*      LD B,C */

  cpustate->B = cpustate->C;
  break;
case 0x42: /*      LD B,D */

  cpustate->B = cpustate->D;
  break;
case 0x43: /*      LD B,E */

  cpustate->B = cpustate->E;
  break;
case 0x44: /*      LD B,H */

  cpustate->B = cpustate->H;
  break;
case 0x45: /*      LD B,L */

  cpustate->B = cpustate->L;
  break;
case 0x46: /*      LD B,(HL) */

  cpustate->B = mem_ReadByte (cpustate, ( cpustate->H << 8 ) | cpustate->L);
  break;
case 0x47: /*      LD B,A */

  cpustate->B = cpustate->A;
  break;
case 0x48: /*      LD C,B */

  cpustate->C = cpustate->B;
  break;
case 0x49: /*      LD C,C */
  break;
case 0x4A: /*      LD C,D */

  cpustate->C = cpustate->D;
  break;
case 0x4B: /*      LD C,E */

  cpustate->C = cpustate->E;
  break;
case 0x4C: /*      LD C,H */

  cpustate->C = cpustate->H;
  break;
case 0x4D: /*      LD C,L */

  cpustate->C = cpustate->L;
  break;
case 0x4E: /*      LD C,(HL) */

  cpustate->C = mem_ReadByte (cpustate, ( cpustate->H << 8 ) | cpustate->L);
  break;
case 0x4F: /*      LD C,A */

  cpustate->C = cpustate->A;
  break;
case 0x50: /*      LD D,B */

  cpustate->D = cpustate->B;
  break;
case 0x51: /*      LD D,C */

  cpustate->D = cpustate->C;
  break;
case 0x52: /*      LD D,D */
  break;
case 0x53: /*      LD D,E */

  cpustate->D = cpustate->E;
  break;
case 0x54: /*      LD D,H */

  cpustate->D = cpustate->H;
  break;
case 0x55: /*      LD D,L */

  cpustate->D = cpustate->L;
  break;
case 0x56: /*      LD D,(HL) */

  cpustate->D = mem_ReadByte (cpustate, ( cpustate->H << 8 ) | cpustate->L);
  break;
case 0x57: /*      LD D,A */

  cpustate->D = cpustate->A;
  break;
case 0x58: /*      LD E,B */

  cpustate->E = cpustate->B;
  break;
case 0x59: /*      LD E,C */

  cpustate->E = cpustate->C;
  break;
case 0x5A: /*      LD E,D */

  cpustate->E = cpustate->D;
  break;
case 0x5B: /*      LD E,E */
  break;
case 0x5C: /*      LD E,H */

  cpustate->E = cpustate->H;
  break;
case 0x5D: /*      LD E,L */

  cpustate->E = cpustate->L;
  break;
case 0x5E: /*      LD E,(HL) */

  cpustate->E = mem_ReadByte (cpustate, ( cpustate->H << 8 ) | cpustate->L);
  break;
case 0x5F: /*      LD E,A */

  cpustate->E = cpustate->A;
  break;
case 0x60: /*      LD H,B */

  cpustate->H = cpustate->B;
  break;
case 0x61: /*      LD H,C */

  cpustate->H = cpustate->C;
  break;
case 0x62: /*      LD H,D */

  cpustate->H = cpustate->D;
  break;
case 0x63: /*      LD H,E */

  cpustate->H = cpustate->E;
  break;
case 0x64: /*      LD H,H */
  break;
case 0x65: /*      LD H,L */

  cpustate->H = cpustate->L;
  break;
case 0x66: /*      LD H,(HL) */

  cpustate->H = mem_ReadByte (cpustate, ( cpustate->H << 8 ) | cpustate->L);
  break;
case 0x67: /*      LD H,A */

  cpustate->H = cpustate->A;
  break;
case 0x68: /*      LD L,B */

  cpustate->L = cpustate->B;
  break;
case 0x69: /*      LD L,C */

  cpustate->L = cpustate->C;
  break;
case 0x6A: /*      LD L,D */
  cpustate->L = cpustate->D;
  break;
case 0x6B: /*      LD L,E */

  cpustate->L = cpustate->E;
  break;
case 0x6C: /*      LD L,H */

  cpustate->L = cpustate->H;
  break;
case 0x6D: /*      LD L,L */
  break;
case 0x6E: /*      LD L,(HL) */

  cpustate->L = mem_ReadByte (cpustate, ( cpustate->H << 8 ) | cpustate->L);
  break;
case 0x6F: /*      LD L,A */

  cpustate->L = cpustate->A;
  break;
case 0x70: /*      LD (HL),B */

  mem_WriteByte (cpustate, ( cpustate->H << 8 ) | cpustate->L, cpustate->B);
  break;
case 0x71: /*      LD (HL),C */

  mem_WriteByte (cpustate, ( cpustate->H << 8 ) | cpustate->L, cpustate->C);
  break;
case 0x72: /*      LD (HL),D */

  mem_WriteByte (cpustate, ( cpustate->H << 8 ) | cpustate->L, cpustate->D);
  break;
case 0x73: /*      LD (HL),E */

  mem_WriteByte (cpustate, ( cpustate->H << 8 ) | cpustate->L, cpustate->E);
  break;
case 0x74: /*      LD (HL),H */

  mem_WriteByte (cpustate, ( cpustate->H << 8 ) | cpustate->L, cpustate->H);
  break;
case 0x75: /*      LD (HL),L */

  mem_WriteByte (cpustate, ( cpustate->H << 8 ) | cpustate->L, cpustate->L);
  break;
case 0x76: /*      HALT */
	cpustate->enable |= HALTED;
	cpustate->PC--;
  break;
case 0x77: /*      LD (HL),A */

  mem_WriteByte (cpustate, ( cpustate->H << 8 ) | cpustate->L, cpustate->A);
  break;
case 0x78: /*      LD A,B */

  cpustate->A = cpustate->B;
  break;
case 0x79: /*      LD A,C */

  cpustate->A = cpustate->C;
  break;
case 0x7A: /*      LD A,D */

  cpustate->A = cpustate->D;
  break;
case 0x7B: /*      LD A,E */

  cpustate->A = cpustate->E;
  break;
case 0x7C: /*      LD A,H */

  cpustate->A = cpustate->H;
  break;
case 0x7D: /*      LD A,L */

  cpustate->A = cpustate->L;
  break;
case 0x7E: /*      LD A,(HL) */

  cpustate->A = mem_ReadByte (cpustate, ( cpustate->H << 8 ) | cpustate->L);
  break;
case 0x7F: /*      LD A,A */
  break;
case 0x80: /*      ADD A,B */

  ADD_A_X (cpustate->B)
  break;
case 0x81: /*      ADD A,C */

  ADD_A_X (cpustate->C)
  break;
case 0x82: /*      ADD A,D */

  ADD_A_X (cpustate->D)
  break;
case 0x83: /*      ADD A,E */

  ADD_A_X (cpustate->E)
  break;
case 0x84: /*      ADD A,H */

  ADD_A_X (cpustate->H)
  break;
case 0x85: /*      ADD A,L */

  ADD_A_X (cpustate->L)
  break;
case 0x86: /*      ADD A,(HL) */

  x = mem_ReadByte (cpustate, ( cpustate->H << 8 ) | cpustate->L);

  ADD_A_X (x)
  break;
case 0x87: /*      ADD A,A */

  ADD_A_X (cpustate->A)
  break;
case 0x88: /*      ADC A,B */

  ADC_A_X (cpustate->B)
  break;
case 0x89: /*      ADC A,C */

  ADC_A_X (cpustate->C)
  break;
case 0x8A: /*      ADC A,D */

  ADC_A_X (cpustate->D)
  break;
case 0x8B: /*      ADC A,E */

  ADC_A_X (cpustate->E)
  break;
case 0x8C: /*      ADC A,H */

  ADC_A_X (cpustate->H)
  break;
case 0x8D: /*      ADC A,L */

  ADC_A_X (cpustate->L)
  break;
case 0x8E: /*      ADC A,(HL) */

  x = mem_ReadByte (cpustate, ( cpustate->H << 8 ) | cpustate->L);

  ADC_A_X (x)
  break;
case 0x8F: /*      ADC A,A */

  ADC_A_X (cpustate->A)
  break;
case 0x90: /*      SUB A,B */

  SUB_A_X (cpustate->B)
  break;
case 0x91: /*      SUB A,C */

  SUB_A_X (cpustate->C)
  break;
case 0x92: /*      SUB A,D */

  SUB_A_X (cpustate->D)
  break;
case 0x93: /*      SUB A,E */

  SUB_A_X (cpustate->E)
  break;
case 0x94: /*      SUB A,H */

  SUB_A_X (cpustate->H)
  break;
case 0x95: /*      SUB A,L */

  SUB_A_X (cpustate->L)
  break;
case 0x96: /*      SUB A,(HL) */


  x = mem_ReadByte (cpustate, ( cpustate->H << 8 ) | cpustate->L);

  SUB_A_X (x)
  break;
case 0x97: /*      SUB A,A */

  SUB_A_X (cpustate->A)
  break;
case 0x98: /*      SBC A,B */

  SBC_A_X (cpustate->B)
  break;
case 0x99: /*      SBC A,C */

  SBC_A_X (cpustate->C)
  break;
case 0x9A: /*      SBC A,D */

  SBC_A_X (cpustate->D)
  break;
case 0x9B: /*      SBC A,E */

  SBC_A_X (cpustate->E)
  break;
case 0x9C: /*      SBC A,H */

  SBC_A_X (cpustate->H)
  break;
case 0x9D: /*      SBC A,L */

  SBC_A_X (cpustate->L)
  break;
case 0x9E: /*      SBC A,(HL) */

  x = mem_ReadByte (cpustate, ( cpustate->H << 8 ) | cpustate->L);

  SBC_A_X (x)
  break;
case 0x9F: /*      SBC A,A */

  SBC_A_X (cpustate->A)
  break;
case 0xA0: /*      AND A,B */

  AND_A_X (cpustate->B)
  break;
case 0xA1: /*      AND A,C */

  AND_A_X (cpustate->C)
  break;
case 0xA2: /*      AND A,D */

  AND_A_X (cpustate->D)
  break;
case 0xA3: /*      AND A,E */

  AND_A_X (cpustate->E)
  break;
case 0xA4: /*      AND A,H */

  AND_A_X (cpustate->H)
  break;
case 0xA5: /*      AND A,L */

  AND_A_X (cpustate->L)
  break;
case 0xA6: /*      AND A,(HL) */

  x = mem_ReadByte (cpustate, ( cpustate->H << 8 ) | cpustate->L);

  AND_A_X (x)
  break;
case 0xA7: /*      AND A,A */

  cpustate->F = (cpustate->A == 0) ? (FLAG_H | FLAG_Z) : FLAG_H;
  break;
case 0xA8: /*      XOR A,B */

  XOR_A_X (cpustate->B)
  break;
case 0xA9: /*      XOR A,C */

  XOR_A_X (cpustate->C)
  break;
case 0xAA: /*      XOR A,D */

  XOR_A_X (cpustate->D)
  break;
case 0xAB: /*      XOR A,E */

  XOR_A_X (cpustate->E)
  break;
case 0xAC: /*      XOR A,H */

  XOR_A_X (cpustate->H)
  break;
case 0xAD: /*      XOR A,L */

  XOR_A_X (cpustate->L)
  break;
case 0xAE: /*      XOR A,(HL) */

  x = mem_ReadByte (cpustate, ( cpustate->H << 8 ) | cpustate->L);

  XOR_A_X (x)
  break;
case 0xAF: /*      XOR A,A */

  XOR_A_X (cpustate->A)
  break;
case 0xB0: /*      OR A,B */

  OR_A_X (cpustate->B)
  break;
case 0xB1: /*      OR A,C */

  OR_A_X (cpustate->C)
  break;
case 0xB2: /*      OR A,D */

  OR_A_X (cpustate->D)
  break;
case 0xB3: /*      OR A,E */

  OR_A_X (cpustate->E)
  break;
case 0xB4: /*      OR A,H */

  OR_A_X (cpustate->H)
  break;
case 0xB5: /*      OR A,L */

  OR_A_X (cpustate->L)
  break;
case 0xB6: /*      OR A,(HL) */

  x = mem_ReadByte (cpustate, ( cpustate->H << 8 ) | cpustate->L);

  OR_A_X (x)
  break;
case 0xB7: /*      OR A,A */

  OR_A_X (cpustate->A)
  break;
case 0xB8: /*      CP A,B */

  CP_A_X (cpustate->B)
  break;
case 0xB9: /*      CP A,C */

  CP_A_X (cpustate->C)
  break;
case 0xBA: /*      CP A,D */

  CP_A_X (cpustate->D)
  break;
case 0xBB: /*      CP A,E */

  CP_A_X (cpustate->E)
  break;
case 0xBC: /*      CP A,H */

  CP_A_X (cpustate->H)
  break;
case 0xBD: /*      CP A,L */

  CP_A_X (cpustate->L)
  break;
case 0xBE: /*      CP A,(HL) */

  x = mem_ReadByte (cpustate, ( cpustate->H << 8 ) | cpustate->L);

  CP_A_X (x)
  break;
case 0xBF: /*      CP A,A */

  CP_A_X (cpustate->A)
  break;
case 0xC0: /*      RET NZ */
	CYCLES_PASSED( 4 );
	if (!(cpustate->F & FLAG_Z))
	{
		cpustate->PC = mem_ReadWord (cpustate, cpustate->SP);
		cpustate->SP += 2;
		CYCLES_PASSED( 4 );
	}
	break;
case 0xC1: /*      POP BC */
	POP( cpustate->B, cpustate->C );
	break;
case 0xC2: /*      JP NZ,n16 */
	{
		UINT16 addr = mem_ReadWord (cpustate, cpustate->PC);
		cpustate->PC += 2;

		if ( ! (cpustate->F & FLAG_Z) )
		{
			cpustate->PC = addr;
			CYCLES_PASSED( 4 );
		}
	}
	break;
case 0xC3: /*      JP n16 */
	cpustate->PC = mem_ReadWord (cpustate, cpustate->PC);
	CYCLES_PASSED( 4 );
	break;
case 0xC4: /*      CALL NZ,n16 */
	{
		UINT16 addr = mem_ReadWord (cpustate, cpustate->PC);
		cpustate->PC += 2;

		if ( ! (cpustate->F & FLAG_Z) )
		{
			cpustate->SP -= 2;
			mem_WriteWord (cpustate, cpustate->SP, cpustate->PC);
			cpustate->PC = addr;
			CYCLES_PASSED( 4 );
		}
	}
	break;
case 0xC5: /*      PUSH BC */
	PUSH( cpustate->B, cpustate->C );
	CYCLES_PASSED( 4 );
	break;
case 0xC6: /*      ADD A,n8 */

  x = mem_ReadByte (cpustate, cpustate->PC++);
  ADD_A_X (x)
  break;
case 0xC7: /*      RST 0 */
	cpustate->SP -= 2;
	mem_WriteWord (cpustate, cpustate->SP, cpustate->PC);
	cpustate->PC = 0;
	CYCLES_PASSED( 4 );
	break;
case 0xC8: /*      RET Z */
	CYCLES_PASSED( 4 );
	if (cpustate->F & FLAG_Z)
	{
		cpustate->PC = mem_ReadWord (cpustate, cpustate->SP);
		cpustate->SP += 2;
		CYCLES_PASSED( 4 );
	}
	break;
case 0xC9: /*      RET */
	cpustate->PC = mem_ReadWord (cpustate, cpustate->SP);
	cpustate->SP += 2;
	CYCLES_PASSED( 4 );
	break;
case 0xCA: /*      JP Z,n16 */
	{
		UINT16 addr = mem_ReadWord (cpustate, cpustate->PC);
		cpustate->PC += 2;

		if (cpustate->F & FLAG_Z)
		{
			cpustate->PC = addr;
			CYCLES_PASSED( 4 );
		}
	}
	break;
case 0xCB: /*      PREFIX! */
  x = mem_ReadByte (cpustate, cpustate->PC++);
  switch (x)
  {
    #include "opc_cb.h"
  }
  break;
case 0xCC: /*      CALL Z,n16 */
	{
		UINT16 addr = mem_ReadWord (cpustate, cpustate->PC);
		cpustate->PC += 2;

		if (cpustate->F & FLAG_Z)
		{
			cpustate->SP -= 2;
			mem_WriteWord (cpustate, cpustate->SP, cpustate->PC);
			cpustate->PC = addr;
			CYCLES_PASSED( 4 );
		}
	}
	break;
case 0xCD: /*      CALL n16 */
	{
		UINT16 addr = mem_ReadWord (cpustate, cpustate->PC);
		cpustate->PC += 2;

		cpustate->SP -= 2;
		mem_WriteWord (cpustate, cpustate->SP, cpustate->PC);
		cpustate->PC = addr;
		CYCLES_PASSED( 4 );
	}
	break;
case 0xCE: /*      ADC A,n8 */

  x = mem_ReadByte (cpustate, cpustate->PC++);
  ADC_A_X (x)
  break;
case 0xCF: /*      RST 8 */
	cpustate->SP -= 2;
	mem_WriteWord (cpustate, cpustate->SP, cpustate->PC);
	cpustate->PC = 8;
	CYCLES_PASSED( 4 );
	break;
case 0xD0: /*      RET NC */
	CYCLES_PASSED( 4 );
	if (!(cpustate->F & FLAG_C))
	{
		cpustate->PC = mem_ReadWord (cpustate, cpustate->SP);
		cpustate->SP += 2;
		CYCLES_PASSED( 4 );
	}
	break;
case 0xD1: /*      POP DE */
	POP( cpustate->D, cpustate->E );
	break;
case 0xD2: /*      JP NC,n16 */
	{
		UINT16 addr = mem_ReadWord (cpustate, cpustate->PC);
		cpustate->PC += 2;

		if ( ! (cpustate->F & FLAG_C) )
		{
			cpustate->PC = addr;
			CYCLES_PASSED( 4 );
		}
	}
	break;
case 0xD3: /*      EH? */
  break;
case 0xD4: /*      CALL NC,n16 */
	{
		UINT16 addr = mem_ReadWord (cpustate, cpustate->PC);
		cpustate->PC += 2;

		if ( ! (cpustate->F & FLAG_C) )
		{
			cpustate->SP -= 2;
			mem_WriteWord (cpustate, cpustate->SP, cpustate->PC);
			cpustate->PC = addr;
			CYCLES_PASSED( 4 );
		}
	}
	break;
case 0xD5: /*      PUSH DE */
	PUSH( cpustate->D, cpustate->E );
	CYCLES_PASSED( 4 );
	break;
case 0xD6: /*      SUB A,n8 */

  x = mem_ReadByte (cpustate, cpustate->PC++);
  SUB_A_X (x)
  break;
case 0xD7: /*      RST     $10 */
	cpustate->SP -= 2;
	mem_WriteWord (cpustate, cpustate->SP, cpustate->PC);
	cpustate->PC = 0x10;
	CYCLES_PASSED( 4 );
	break;
case 0xD8: /*      RET C */
	CYCLES_PASSED( 4 );
	if (cpustate->F & FLAG_C)
	{
		cpustate->PC = mem_ReadWord (cpustate, cpustate->SP);
		cpustate->SP += 2;
		CYCLES_PASSED( 4 );
	}
	break;
case 0xD9: /*      RETI */
	cpustate->PC = mem_ReadWord (cpustate, cpustate->SP);
	cpustate->SP += 2;
	cpustate->enable |= IME;
	CYCLES_PASSED( 4 );
	break;
case 0xDA: /*      JP C,n16 */
	{
		UINT16 addr = mem_ReadWord (cpustate, cpustate->PC);
		cpustate->PC += 2;

		if (cpustate->F & FLAG_C)
		{
			cpustate->PC = addr;
			CYCLES_PASSED( 4 );
		}
	}
	break;
case 0xDB: /*      EH? */
  break;
case 0xDC: /*      CALL C,n16 */
	{
		UINT16 addr = mem_ReadWord (cpustate, cpustate->PC);
		cpustate->PC += 2;

		if (cpustate->F & FLAG_C)
		{
			cpustate->SP -= 2;
			mem_WriteWord (cpustate, cpustate->SP, cpustate->PC);
			cpustate->PC = addr;
			CYCLES_PASSED( 4 );
		}
	}
	break;
case 0xDD: /*      EH? */
  break;
case 0xDE: /*      SBC A,n8 */

  x = mem_ReadByte (cpustate, cpustate->PC++);
  SBC_A_X (x)
  break;
case 0xDF: /*      RST     $18 */
	cpustate->SP -= 2;
	mem_WriteWord (cpustate, cpustate->SP, cpustate->PC);
	cpustate->PC = 0x18;
	CYCLES_PASSED( 4 );
	break;
case 0xE0: /*      LD      ($FF00+n8),A */
  {
	UINT8 v = mem_ReadByte (cpustate, cpustate->PC++);
	mem_WriteByte (cpustate, 0xFF00 + v, cpustate->A);
  }
  break;
case 0xE1: /*      POP HL */
	POP( cpustate->H, cpustate->L );
	break;
case 0xE2: /*      LD ($FF00+C),A */

  mem_WriteByte (cpustate, (UINT16) (0xFF00 + cpustate->C), cpustate->A);
  break;
case 0xE3: /*      EH? */
  break;
case 0xE4: /*      EH? */
  break;
case 0xE5: /*      PUSH HL */
	PUSH( cpustate->H, cpustate->L );
	CYCLES_PASSED( 4 );
	break;
case 0xE6: /*      AND A,n8 */

  x = mem_ReadByte (cpustate, cpustate->PC++);
  AND_A_X (x)
  break;
case 0xE7: /*      RST $20 */
	cpustate->SP -= 2;
	mem_WriteWord (cpustate, cpustate->SP, cpustate->PC);
	cpustate->PC = 0x20;
	CYCLES_PASSED( 4 );
	break;
case 0xE8: /*      ADD SP,n8 */
/*
 *   Z - Reset.
 *   N - Reset.
 *   H - Set or reset according to operation.
 *   C - Set or reset according to operation.
 */

  {
	register INT32 n;

	n = (INT8) mem_ReadByte (cpustate, cpustate->PC++);

	if ( ( cpustate->SP & 0xFF ) + (UINT8)(n & 0xFF) > 0xFF )
    {
      cpustate->F = FLAG_C;
    }
    else
    {
      cpustate->F = 0;
    }

    if ( ( cpustate->SP & 0x0F ) + ( n & 0x0F ) > 0x0F )
    {
      cpustate->F |= FLAG_H;
    }

	cpustate->SP = (UINT16) ( cpustate->SP + n );
  }
  CYCLES_PASSED( 8 );
  break;
case 0xE9: /*      JP (HL) */
	cpustate->PC = ( cpustate->H << 8 ) | cpustate->L;
	break;
case 0xEA: /*      LD (n16),A */

  mem_WriteByte (cpustate, mem_ReadWord (cpustate, cpustate->PC), cpustate->A);
  cpustate->PC += 2;
  break;
case 0xEB: /*      EH? */
  break;
case 0xEC: /*      EH? */
  break;
case 0xED: /*      EH? */
  break;
case 0xEE: /*      XOR A,n8 */

  x = mem_ReadByte (cpustate, cpustate->PC++);
  XOR_A_X (x)
  break;
case 0xEF: /*      RST $28 */
	cpustate->SP -= 2;
	mem_WriteWord (cpustate, cpustate->SP, cpustate->PC);
	cpustate->PC = 0x28;
	CYCLES_PASSED( 4 );
	break;
case 0xF0: /*      LD A,($FF00+n8) */
  {
	UINT8 v = mem_ReadByte (cpustate, cpustate->PC++);
	cpustate->A = mem_ReadByte (cpustate, 0xFF00 + v);
  }
  break;
case 0xF1: /*      POP AF */
	POP( cpustate->A, cpustate->F );
	cpustate->F &= 0xF0;
	break;
case 0xF2: /*      LD A,($FF00+C) */

  cpustate->A = mem_ReadByte (cpustate, (UINT16) (0xFF00 + cpustate->C));
  break;
case 0xF3: /*      DI */
  cpustate->ei_delay = 0;
  cpustate->enable &= ~IME;
  break;
case 0xF4: /*      EH? */
  break;
case 0xF5: /*      PUSH AF */
	cpustate->F &= 0xF0;
	PUSH( cpustate->A, cpustate->F );
	CYCLES_PASSED( 4 );
	break;
case 0xF6: /*      OR A,n8 */

  x = mem_ReadByte (cpustate, cpustate->PC++);
  OR_A_X (x)
  break;
case 0xF7: /*      RST $30 */
	cpustate->SP -= 2;
	mem_WriteWord (cpustate, cpustate->SP, cpustate->PC);
	cpustate->PC = 0x30;
	CYCLES_PASSED( 4 );
	break;
case 0xF8: /*      LD HL,SP+n8 */
/*
 *   n = one UINT8 signed immediate value.
 * Flags affected:
 *   Z - Reset.
 *   N - Reset.
 *   H - Set or reset according to operation.
 *   C - Set or reset according to operation.
 *
 */

  {
	register INT32 n;

	n = (INT8) mem_ReadByte (cpustate, cpustate->PC++);

	if ( ( cpustate->SP & 0xFF ) + (UINT8)(n & 0xFF) > 0xFF )
    {
      cpustate->F = FLAG_C;
    }
    else
    {
      cpustate->F = 0;
    }

	if ( ( cpustate->SP & 0x0F ) + ( n & 0x0F ) > 0x0F )
    {
      cpustate->F |= FLAG_H;
    }

	UINT16 res = cpustate->SP + n;

	cpustate->L = res & 0xFF;
	cpustate->H = res >> 8;
  }
  CYCLES_PASSED( 4 );
  break;
case 0xF9: /*      LD SP,HL */
	cpustate->SP = ( cpustate->H << 8 ) | cpustate->L;
	CYCLES_PASSED( 4 );
	break;
case 0xFA: /*      LD A,(n16) */
	cpustate->A = mem_ReadByte (cpustate, mem_ReadWord (cpustate, cpustate->PC));
	cpustate->PC += 2;
	break;
case 0xFB: /*      EI */
	cpustate->enable |= IME;
	cpustate->ei_delay = 1;
	break;
case 0xFC: /*      EH? */
	break;
case 0xFD: /*      EH? */
	break;
case 0xFE: /*      CP A,n8 */
	x = mem_ReadByte (cpustate, cpustate->PC++);
	CP_A_X (x)
	break;
case 0xFF: /*      RST $38 */
	cpustate->SP -= 2;
	mem_WriteWord (cpustate, cpustate->SP, cpustate->PC);
	cpustate->PC = 0x38;
	CYCLES_PASSED( 4 );
	break;
