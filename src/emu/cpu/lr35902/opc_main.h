#define	INC_8BIT(x) \
{ \
  register UINT8 r,f; \
  x++; \
  r=(x);  \
  f=(UINT8)(cpustate->b.F&FLAG_C); \
  if( r==0 )       f|=FLAG_Z; \
  if( (r&0xF)==0 ) f|=FLAG_H; \
  cpustate->b.F=f; \
}

#define	DEC_8BIT(x) \
{ \
  register UINT8 r,f; \
  x--; \
  r=(x);  \
  f=(UINT8)((cpustate->b.F&FLAG_C)|FLAG_N); \
  if( r==0 )       f|=FLAG_Z; \
  if( (r&0xF)==0xF ) f|=FLAG_H; \
  cpustate->b.F=f; \
}

#define	ADD_HL_RR(x) \
{ \
  register UINT32 r1,r2; \
  register UINT8 f; \
  r1=cpustate->w.HL+(x); \
  r2=(cpustate->w.HL&0xFFF)+((x)&0xFFF); \
  f=(UINT8)(cpustate->b.F&FLAG_Z); \
  if( r1>0xFFFF ) f|=FLAG_C; \
  if( r2>0x0FFF ) f|=FLAG_H; \
  cpustate->w.HL=(UINT16)r1; \
  cpustate->b.F=f; \
}

#define	ADD_A_X(x) \
{ \
  register UINT16 r1,r2; \
  register UINT8 f; \
  r1=(UINT16)((cpustate->b.A&0xF)+((x)&0xF)); \
  r2=(UINT16)(cpustate->b.A+(x)); \
  cpustate->b.A=(UINT8)r2; \
  if( ((UINT8)r2)==0 ) f=FLAG_Z; \
    else f=0; \
  if( r2>0xFF ) f|=FLAG_C; \
  if( r1>0xF )  f|=FLAG_H; \
  cpustate->b.F=f; \
}

#define	SUB_A_X(x) \
{ \
  register UINT16 r1,r2; \
  register UINT8 f; \
  r1=(UINT16)((cpustate->b.A&0xF)-((x)&0xF)); \
  r2=(UINT16)(cpustate->b.A-(x)); \
  cpustate->b.A=(UINT8)r2; \
  if( ((UINT8)r2)==0 ) f=FLAG_N|FLAG_Z; \
    else f=FLAG_N; \
  if( r2>0xFF ) f|=FLAG_C; \
  if( r1>0xF )  f|=FLAG_H; \
  cpustate->b.F=f; \
}

/*
   #define      CP_A_X(x) \
   { \
   register UINT16 r; \
   register UINT8 f; \
   r=(UINT16)(cpustate->b.A-(x)); \
   if( ((UINT8)r)==0 ) \
   f=FLAG_N|FLAG_Z; \
   else \
   f=FLAG_N; \
   f|=(UINT8)((r>>8)&FLAG_C); \
   if( (r^cpustate->b.A^(x))&0x10 ) \
   f|=FLAG_H; \
   cpustate->b.F=f; \
   }
 */

#define	CP_A_X(x) \
{ \
  register UINT16 r1,r2; \
  register UINT8 f; \
  r1=(UINT16)((cpustate->b.A&0xF)-((x)&0xF)); \
  r2=(UINT16)(cpustate->b.A-(x)); \
  if( ((UINT8)r2)==0 ) f=FLAG_N|FLAG_Z; \
    else f=FLAG_N; \
  if( r2>0xFF ) f|=FLAG_C; \
  if( r1>0xF )  f|=FLAG_H; \
  cpustate->b.F=f; \
}

#define	SBC_A_X(x) \
{ \
  register UINT16 r1,r2; \
  register UINT8 f; \
  r1=(UINT16)((cpustate->b.A&0xF)-((x)&0xF)-((cpustate->b.F&FLAG_C)?1:0)); \
  r2=(UINT16)(cpustate->b.A-(x)-((cpustate->b.F&FLAG_C)?1:0)); \
  cpustate->b.A=(UINT8)r2; \
  if( ((UINT8)r2)==0 ) f=FLAG_N|FLAG_Z; \
    else f=FLAG_N; \
  if( r2>0xFF ) f|=FLAG_C; \
  if( r1>0xF )  f|=FLAG_H; \
  cpustate->b.F=f; \
}

#define	ADC_A_X(x) \
{ \
  register UINT16 r1,r2;  \
  register UINT8 f; \
  r1=(UINT16)((cpustate->b.A&0xF)+((x)&0xF)+((cpustate->b.F&FLAG_C)?1:0));  \
  r2=(UINT16)(cpustate->b.A+(x)+((cpustate->b.F&FLAG_C)?1:0)); \
  if( (cpustate->b.A=(UINT8)r2)==0 ) f=FLAG_Z; \
    else f=0; \
  if( r2>0xFF )	f|=FLAG_C; \
  if( r1>0xF )	f|=FLAG_H; \
  cpustate->b.F=f; \
}

#define	AND_A_X(x) \
  if( (cpustate->b.A&=(x))==0 ) \
    cpustate->b.F=FLAG_H|FLAG_Z; \
  else \
    cpustate->b.F=FLAG_H;

#define XOR_A_X(x) \
  if( (cpustate->b.A^=(x))==0 ) \
    cpustate->b.F=FLAG_Z; \
  else \
    cpustate->b.F=0;

#define	OR_A_X(x) \
  if( (cpustate->b.A|=(x))==0 ) \
    cpustate->b.F=FLAG_Z; \
  else \
    cpustate->b.F=0;


case 0x00: /*      NOP */
  break;
case 0x01: /*      LD BC,n16 */
  cpustate->w.BC = mem_ReadWord (cpustate, cpustate->w.PC);
  cpustate->w.PC += 2;
  break;
case 0x02: /*      LD (BC),A */
  mem_WriteByte (cpustate, cpustate->w.BC, cpustate->b.A);
  break;
case 0x03: /*      INC BC */
#if 0				/* FIXME ?? do we want to support this? (bug emulation) */
  if (cpustate->b.B == 0xFE)
  {
    trash_sprites (state);
  }
#endif
	cpustate->w.BC += 1;
	CYCLES_PASSED( 4 );
	break;
case 0x04: /*      INC B */

  INC_8BIT (cpustate->b.B)
  break;
case 0x05: /*      DEC B */

  DEC_8BIT (cpustate->b.B)
  break;
case 0x06: /*      LD B,n8 */

  cpustate->b.B = mem_ReadByte (cpustate, cpustate->w.PC++);
  break;
case 0x07: /*      RLCA */

  cpustate->b.A = (UINT8) ((cpustate->b.A << 1) | (cpustate->b.A >> 7));
  if (cpustate->b.A & 1)
  {
    cpustate->b.F = FLAG_C;
  }
  else
  {
    cpustate->b.F = 0;
  }
  break;
case 0x08: /*      LD (n16),SP */

  mem_WriteWord (cpustate, mem_ReadWord (cpustate, cpustate->w.PC), cpustate->w.SP);
  cpustate->w.PC += 2;
  break;
case 0x09: /*      ADD HL,BC */
	ADD_HL_RR (cpustate->w.BC)
	CYCLES_PASSED( 4 );
	break;
case 0x0A: /*      LD A,(BC) */

  cpustate->b.A = mem_ReadByte (cpustate, cpustate->w.BC);
  break;
case 0x0B: /*      DEC BC */

#if 0				/* FIXME ?? do we want to support this? (bug emulation) */
  if (cpustate->b.B == 0xFE)
  {
    trash_sprites (state);
  }
#endif
	cpustate->w.BC -= 1;
	CYCLES_PASSED( 4 );
	break;
case 0x0C: /*      INC C */

  INC_8BIT (cpustate->b.C)
  break;
case 0x0D: /*      DEC C */

  DEC_8BIT (cpustate->b.C)
  break;
case 0x0E: /*      LD C,n8 */

  cpustate->b.C = mem_ReadByte (cpustate, cpustate->w.PC++);
  break;
case 0x0F: /*      RRCA */

  cpustate->b.A = (UINT8) ((cpustate->b.A >> 1) | (cpustate->b.A << 7));
  cpustate->b.F = 0;
  if (cpustate->b.A & 0x80)
  {
    cpustate->b.F |= FLAG_C;
  }
  break;
case 0x10: /*      STOP */
  if ( cpustate->w.gb_speed_change_pending ) {
    cpustate->w.gb_speed = ( cpustate->w.gb_speed == 1 ) ? 2 : 1;
  }
  cpustate->w.gb_speed_change_pending = 0;
  break;
case 0x11: /*      LD DE,n16 */

  cpustate->w.DE = mem_ReadWord (cpustate, cpustate->w.PC);
  cpustate->w.PC += 2;
  break;
case 0x12: /*      LD (DE),A */
  mem_WriteByte (cpustate, cpustate->w.DE, cpustate->b.A);
  break;
case 0x13: /*      INC DE */

#if 0				/* FIXME ?? do we want to support this? (bug emulation) */
  if (cpustate->b.D == 0xFE)
  {
    trash_sprites (state);
  }
#endif

	cpustate->w.DE += 1;
	CYCLES_PASSED( 4 );
	break;
case 0x14: /*      INC D */

  INC_8BIT (cpustate->b.D)
  break;
case 0x15: /*      DEC D */

  DEC_8BIT (cpustate->b.D)
  break;
case 0x16: /*      LD D,n8 */

  cpustate->b.D = mem_ReadByte (cpustate, cpustate->w.PC++);
  break;
case 0x17: /*      RLA */

  x = (cpustate->b.A & 0x80) ? FLAG_C : 0;

  cpustate->b.A = (UINT8) ((cpustate->b.A << 1) | ((cpustate->b.F & FLAG_C) ? 1 : 0));
  cpustate->b.F = x;
  break;
case 0x18: /*      JR      n8 */
  {
	INT8 offset;

    offset = mem_ReadByte (cpustate, cpustate->w.PC++);
    cpustate->w.PC += offset;
	CYCLES_PASSED( 4 );
  }
  break;
case 0x19: /*      ADD HL,DE */
	ADD_HL_RR (cpustate->w.DE)
	CYCLES_PASSED( 4 );
	break;
case 0x1A: /*      LD A,(DE) */

  cpustate->b.A = mem_ReadByte (cpustate, cpustate->w.DE);
  break;
case 0x1B: /*      DEC DE */

#if 0				/* FIXME ?? do we want to support this? (bug emulation) */
  if (cpustate->b.D == 0xFE)
  {
    trash_sprites (state);
  }
#endif
	cpustate->w.DE -= 1;
	CYCLES_PASSED( 4 );
	break;
case 0x1C: /*      INC E */

  INC_8BIT (cpustate->b.E)
  break;
case 0x1D: /*      DEC E */

  DEC_8BIT (cpustate->b.E)
  break;
case 0x1E: /*      LD E,n8 */

  cpustate->b.E = mem_ReadByte (cpustate, cpustate->w.PC++);
  break;
case 0x1F: /*      RRA */

  x = (cpustate->b.A & 1) ? FLAG_C : 0;

  cpustate->b.A = (UINT8) ((cpustate->b.A >> 1) | ((cpustate->b.F & FLAG_C) ? 0x80 : 0));
  cpustate->b.F = x;
  break;
case 0x20: /*      JR NZ,n8 */
	{
		INT8 offset = mem_ReadByte (cpustate, cpustate->w.PC++);
		if (! (cpustate->b.F & FLAG_Z) )
		{
			cpustate->w.PC += offset;
			CYCLES_PASSED( 4 );
		}
	}
	break;
case 0x21: /*      LD HL,n16 */

  cpustate->w.HL = mem_ReadWord (cpustate, cpustate->w.PC);
  cpustate->w.PC += 2;
  break;
case 0x22: /*      LD (HL+),A */

#if 0				/* FIXME ?? do we want to support this? (bug emulation) */
  if (cpustate->b.H == 0xFE)
  {
    trash_sprites (state);
  }
#endif

  mem_WriteByte (cpustate, cpustate->w.HL, cpustate->b.A);
  cpustate->w.HL += 1;
  break;
case 0x23: /*      INC HL */

#if 0				/* FIXME ?? do we want to support this? (bug emulation) */
  if (cpustate->b.H == 0xFE)
  {
    trash_sprites (state);
  }
#endif

	cpustate->w.HL += 1;
	CYCLES_PASSED( 4 );
	break;
case 0x24: /*      INC H */

  INC_8BIT (cpustate->b.H);
  break;
case 0x25: /*      DEC H */

  DEC_8BIT (cpustate->b.H);
  break;
case 0x26: /*      LD H,n8 */

  cpustate->b.H = mem_ReadByte (cpustate, cpustate->w.PC++);
  break;
case 0x27: /*      DAA */
	{
		int tmp = cpustate->b.A;

		if ( ! ( cpustate->b.F & FLAG_N ) ) {
			if ( ( cpustate->b.F & FLAG_H ) || ( tmp & 0x0F ) > 9 )
				tmp += 6;
			if ( ( cpustate->b.F & FLAG_C ) || tmp > 0x9F )
				tmp += 0x60;
		} else {
			if ( cpustate->b.F & FLAG_H ) {
				tmp -= 6;
				if ( ! ( cpustate->b.F & FLAG_C ) )
					tmp &= 0xFF;
			}
			if ( cpustate->b.F & FLAG_C )
					tmp -= 0x60;
		}
		cpustate->b.F &= ~ ( FLAG_H | FLAG_Z );
		if ( tmp & 0x100 )
			cpustate->b.F |= FLAG_C;
		cpustate->b.A = tmp & 0xFF;
		if ( ! cpustate->b.A )
			cpustate->b.F |= FLAG_Z;
	}
  break;
case 0x28: /*      JR Z,n8 */
	{
		INT8 offset = mem_ReadByte (cpustate, cpustate->w.PC++);;

		if (cpustate->b.F & FLAG_Z)
		{
			cpustate->w.PC += offset;
			CYCLES_PASSED( 4 );
		}
	}
	break;
case 0x29: /*      ADD HL,HL */
	ADD_HL_RR (cpustate->w.HL)
	CYCLES_PASSED( 4 );
	break;
case 0x2A: /*      LD A,(HL+) */
#if 0				/* FIXME ?? do we want to support this? (bug emulation) */
  if (cpustate->b.H == 0xFE)
  {
    trash_sprites (state);
  }
#endif

  cpustate->b.A = mem_ReadByte (cpustate, cpustate->w.HL);
  cpustate->w.HL += 1;
  break;
case 0x2B: /*      DEC HL */

#if 0				/* FIXME ?? do we want to support this? (bug emulation) */
  if (cpustate->b.H == 0xFE)
  {
    trash_sprites (state);
  }
#endif
	cpustate->w.HL -= 1;
	CYCLES_PASSED( 4 );
	break;
case 0x2C: /*      INC L */

  INC_8BIT (cpustate->b.L);
  break;
case 0x2D: /*      DEC L */

  DEC_8BIT (cpustate->b.L);
  break;
case 0x2E: /*      LD L,n8 */

  cpustate->b.L = mem_ReadByte (cpustate, cpustate->w.PC++);
  break;
case 0x2F: /*      CPL */

  cpustate->b.A = ~cpustate->b.A;
  cpustate->b.F |= FLAG_N | FLAG_H;
  break;
case 0x30: /*      JR NC,n8 */
	{
		INT8 offset = mem_ReadByte (cpustate, cpustate->w.PC++);

		if ( ! (cpustate->b.F & FLAG_C) )
		{
			cpustate->w.PC += offset;
			CYCLES_PASSED( 4 );
		}
	}
	break;
case 0x31: /*      LD SP,n16 */

  cpustate->w.SP = mem_ReadWord (cpustate, cpustate->w.PC);
  cpustate->w.PC += 2;
  break;
case 0x32: /*      LD (HL-),A */

#if 0				/* FIXME ?? do we want to support this? (bug emulation) */
  if (cpustate->b.H == 0xFE)
  {
    trash_sprites (state);
  }
#endif

  mem_WriteByte (cpustate, cpustate->w.HL, cpustate->b.A);
  cpustate->w.HL -= 1;
  break;
case 0x33: /*      INC SP */
	cpustate->w.SP += 1;
	CYCLES_PASSED( 4 );
	break;
case 0x34: /*      INC (HL) */

  {
	register UINT8 r, f;

	f = (UINT8) (cpustate->b.F & FLAG_C);
	r = mem_ReadByte (cpustate, cpustate->w.HL);
	r += 1;
    mem_WriteByte (cpustate, cpustate->w.HL, r);

    if (r == 0)
      f |= FLAG_Z;

    if ((r & 0xF) == 0)
      f |= FLAG_H;

    cpustate->b.F = f;
  }
  break;
case 0x35: /*      DEC (HL) */

  {
	register UINT8 r, f;

	f = (UINT8) ((cpustate->b.F & FLAG_C) | FLAG_N);
	r = mem_ReadByte (cpustate, cpustate->w.HL);
	r -= 1;
    mem_WriteByte (cpustate, cpustate->w.HL, r);

    if (r == 0)
      f |= FLAG_Z;

    if ((r & 0xF) == 0xF)
      f |= FLAG_H;

    cpustate->b.F = f;
  }
  break;
case 0x36: /*      LD (HL),n8 */
  /* FIXED / broken ? */
  {
	UINT8 v = mem_ReadByte (cpustate, cpustate->w.PC++);
	mem_WriteByte (cpustate, cpustate->w.HL, v);
  }
  break;
case 0x37: /*      SCF */

  cpustate->b.F = (UINT8) ((cpustate->b.F & FLAG_Z) | FLAG_C);
  break;
case 0x38: /*      JR C,n8 */
	{
		INT8 offset = mem_ReadByte (cpustate, cpustate->w.PC++);

		if (cpustate->b.F & FLAG_C)
		{
			cpustate->w.PC += offset;
			CYCLES_PASSED( 4 );
		}
	}
	break;
case 0x39: /*      ADD HL,SP */
	ADD_HL_RR (cpustate->w.SP)
	CYCLES_PASSED( 4 );
	break;
case 0x3A: /*      LD A,(HL-) */
#if 0				/* FIXME ?? do we want to support this? (bug emulation) */
  if (cpustate->b.H == 0xFE)
  {
    trash_sprites (state);
  }
#endif

  cpustate->b.A = mem_ReadByte (cpustate, cpustate->w.HL);
  cpustate->w.HL -= 1;
  break;
case 0x3B: /*      DEC SP */
	cpustate->w.SP -= 1;
	CYCLES_PASSED( 4 );
	break;
case 0x3C: /*      INC     A */

  INC_8BIT (cpustate->b.A);
  break;
case 0x3D: /*      DEC     A */

  DEC_8BIT (cpustate->b.A);
  break;
case 0x3E: /*      LD A,n8 */

  cpustate->b.A = mem_ReadByte (cpustate, cpustate->w.PC++);
  break;
case 0x3F: /*      CCF */

  cpustate->b.F = (UINT8) ((cpustate->b.F & FLAG_Z) | ((cpustate->b.F & FLAG_C) ? 0 : FLAG_C));
  break;
case 0x40: /*      LD B,B */
  break;
case 0x41: /*      LD B,C */

  cpustate->b.B = cpustate->b.C;
  break;
case 0x42: /*      LD B,D */

  cpustate->b.B = cpustate->b.D;
  break;
case 0x43: /*      LD B,E */

  cpustate->b.B = cpustate->b.E;
  break;
case 0x44: /*      LD B,H */

  cpustate->b.B = cpustate->b.H;
  break;
case 0x45: /*      LD B,L */

  cpustate->b.B = cpustate->b.L;
  break;
case 0x46: /*      LD B,(HL) */

  cpustate->b.B = mem_ReadByte (cpustate, cpustate->w.HL);
  break;
case 0x47: /*      LD B,A */

  cpustate->b.B = cpustate->b.A;
  break;
case 0x48: /*      LD C,B */

  cpustate->b.C = cpustate->b.B;
  break;
case 0x49: /*      LD C,C */
  break;
case 0x4A: /*      LD C,D */

  cpustate->b.C = cpustate->b.D;
  break;
case 0x4B: /*      LD C,E */

  cpustate->b.C = cpustate->b.E;
  break;
case 0x4C: /*      LD C,H */

  cpustate->b.C = cpustate->b.H;
  break;
case 0x4D: /*      LD C,L */

  cpustate->b.C = cpustate->b.L;
  break;
case 0x4E: /*      LD C,(HL) */

  cpustate->b.C = mem_ReadByte (cpustate, cpustate->w.HL);
  break;
case 0x4F: /*      LD C,A */

  cpustate->b.C = cpustate->b.A;
  break;
case 0x50: /*      LD D,B */

  cpustate->b.D = cpustate->b.B;
  break;
case 0x51: /*      LD D,C */

  cpustate->b.D = cpustate->b.C;
  break;
case 0x52: /*      LD D,D */
  break;
case 0x53: /*      LD D,E */

  cpustate->b.D = cpustate->b.E;
  break;
case 0x54: /*      LD D,H */

  cpustate->b.D = cpustate->b.H;
  break;
case 0x55: /*      LD D,L */

  cpustate->b.D = cpustate->b.L;
  break;
case 0x56: /*      LD D,(HL) */

  cpustate->b.D = mem_ReadByte (cpustate, cpustate->w.HL);
  break;
case 0x57: /*      LD D,A */

  cpustate->b.D = cpustate->b.A;
  break;
case 0x58: /*      LD E,B */

  cpustate->b.E = cpustate->b.B;
  break;
case 0x59: /*      LD E,C */

  cpustate->b.E = cpustate->b.C;
  break;
case 0x5A: /*      LD E,D */

  cpustate->b.E = cpustate->b.D;
  break;
case 0x5B: /*      LD E,E */
  break;
case 0x5C: /*      LD E,H */

  cpustate->b.E = cpustate->b.H;
  break;
case 0x5D: /*      LD E,L */

  cpustate->b.E = cpustate->b.L;
  break;
case 0x5E: /*      LD E,(HL) */

  cpustate->b.E = mem_ReadByte (cpustate, cpustate->w.HL);
  break;
case 0x5F: /*      LD E,A */

  cpustate->b.E = cpustate->b.A;
  break;
case 0x60: /*      LD H,B */

  cpustate->b.H = cpustate->b.B;
  break;
case 0x61: /*      LD H,C */

  cpustate->b.H = cpustate->b.C;
  break;
case 0x62: /*      LD H,D */

  cpustate->b.H = cpustate->b.D;
  break;
case 0x63: /*      LD H,E */

  cpustate->b.H = cpustate->b.E;
  break;
case 0x64: /*      LD H,H */
  break;
case 0x65: /*      LD H,L */

  cpustate->b.H = cpustate->b.L;
  break;
case 0x66: /*      LD H,(HL) */

  cpustate->b.H = mem_ReadByte (cpustate, cpustate->w.HL);
  break;
case 0x67: /*      LD H,A */

  cpustate->b.H = cpustate->b.A;
  break;
case 0x68: /*      LD L,B */

  cpustate->b.L = cpustate->b.B;
  break;
case 0x69: /*      LD L,C */

  cpustate->b.L = cpustate->b.C;
  break;
case 0x6A: /*      LD L,D */
  cpustate->b.L = cpustate->b.D;
  break;
case 0x6B: /*      LD L,E */

  cpustate->b.L = cpustate->b.E;
  break;
case 0x6C: /*      LD L,H */

  cpustate->b.L = cpustate->b.H;
  break;
case 0x6D: /*      LD L,L */
  break;
case 0x6E: /*      LD L,(HL) */

  cpustate->b.L = mem_ReadByte (cpustate, cpustate->w.HL);
  break;
case 0x6F: /*      LD L,A */

  cpustate->b.L = cpustate->b.A;
  break;
case 0x70: /*      LD (HL),B */

  mem_WriteByte (cpustate, cpustate->w.HL, cpustate->b.B);
  break;
case 0x71: /*      LD (HL),C */

  mem_WriteByte (cpustate, cpustate->w.HL, cpustate->b.C);
  break;
case 0x72: /*      LD (HL),D */

  mem_WriteByte (cpustate, cpustate->w.HL, cpustate->b.D);
  break;
case 0x73: /*      LD (HL),E */

  mem_WriteByte (cpustate, cpustate->w.HL, cpustate->b.E);
  break;
case 0x74: /*      LD (HL),H */

  mem_WriteByte (cpustate, cpustate->w.HL, cpustate->b.H);
  break;
case 0x75: /*      LD (HL),L */

  mem_WriteByte (cpustate, cpustate->w.HL, cpustate->b.L);
  break;
case 0x76: /*      HALT */
	cpustate->w.enable |= HALTED;
	cpustate->w.PC--;
  break;
case 0x77: /*      LD (HL),A */

  mem_WriteByte (cpustate, cpustate->w.HL, cpustate->b.A);
  break;
case 0x78: /*      LD A,B */

  cpustate->b.A = cpustate->b.B;
  break;
case 0x79: /*      LD A,C */

  cpustate->b.A = cpustate->b.C;
  break;
case 0x7A: /*      LD A,D */

  cpustate->b.A = cpustate->b.D;
  break;
case 0x7B: /*      LD A,E */

  cpustate->b.A = cpustate->b.E;
  break;
case 0x7C: /*      LD A,H */

  cpustate->b.A = cpustate->b.H;
  break;
case 0x7D: /*      LD A,L */

  cpustate->b.A = cpustate->b.L;
  break;
case 0x7E: /*      LD A,(HL) */

  cpustate->b.A = mem_ReadByte (cpustate, cpustate->w.HL);
  break;
case 0x7F: /*      LD A,A */
  break;
case 0x80: /*      ADD A,B */

  ADD_A_X (cpustate->b.B)
  break;
case 0x81: /*      ADD A,C */

  ADD_A_X (cpustate->b.C)
  break;
case 0x82: /*      ADD A,D */

  ADD_A_X (cpustate->b.D)
  break;
case 0x83: /*      ADD A,E */

  ADD_A_X (cpustate->b.E)
  break;
case 0x84: /*      ADD A,H */

  ADD_A_X (cpustate->b.H)
  break;
case 0x85: /*      ADD A,L */

  ADD_A_X (cpustate->b.L)
  break;
case 0x86: /*      ADD A,(HL) */

  x = mem_ReadByte (cpustate, cpustate->w.HL);

  ADD_A_X (x)
  break;
case 0x87: /*      ADD A,A */

  ADD_A_X (cpustate->b.A)
  break;
case 0x88: /*      ADC A,B */

  ADC_A_X (cpustate->b.B)
  break;
case 0x89: /*      ADC A,C */

  ADC_A_X (cpustate->b.C)
  break;
case 0x8A: /*      ADC A,D */

  ADC_A_X (cpustate->b.D)
  break;
case 0x8B: /*      ADC A,E */

  ADC_A_X (cpustate->b.E)
  break;
case 0x8C: /*      ADC A,H */

  ADC_A_X (cpustate->b.H)
  break;
case 0x8D: /*      ADC A,L */

  ADC_A_X (cpustate->b.L)
  break;
case 0x8E: /*      ADC A,(HL) */

  x = mem_ReadByte (cpustate, cpustate->w.HL);

  ADC_A_X (x)
  break;
case 0x8F: /*      ADC A,A */

  ADC_A_X (cpustate->b.A)
  break;
case 0x90: /*      SUB A,B */

  SUB_A_X (cpustate->b.B)
  break;
case 0x91: /*      SUB A,C */

  SUB_A_X (cpustate->b.C)
  break;
case 0x92: /*      SUB A,D */

  SUB_A_X (cpustate->b.D)
  break;
case 0x93: /*      SUB A,E */

  SUB_A_X (cpustate->b.E)
  break;
case 0x94: /*      SUB A,H */

  SUB_A_X (cpustate->b.H)
  break;
case 0x95: /*      SUB A,L */

  SUB_A_X (cpustate->b.L)
  break;
case 0x96: /*      SUB A,(HL) */


  x = mem_ReadByte (cpustate, cpustate->w.HL);

  SUB_A_X (x)
  break;
case 0x97: /*      SUB A,A */

  SUB_A_X (cpustate->b.A)
  break;
case 0x98: /*      SBC A,B */

  SBC_A_X (cpustate->b.B)
  break;
case 0x99: /*      SBC A,C */

  SBC_A_X (cpustate->b.C)
  break;
case 0x9A: /*      SBC A,D */

  SBC_A_X (cpustate->b.D)
  break;
case 0x9B: /*      SBC A,E */

  SBC_A_X (cpustate->b.E)
  break;
case 0x9C: /*      SBC A,H */

  SBC_A_X (cpustate->b.H)
  break;
case 0x9D: /*      SBC A,L */

  SBC_A_X (cpustate->b.L)
  break;
case 0x9E: /*      SBC A,(HL) */

  x = mem_ReadByte (cpustate, cpustate->w.HL);

  SBC_A_X (x)
  break;
case 0x9F: /*      SBC A,A */

  SBC_A_X (cpustate->b.A)
  break;
case 0xA0: /*      AND A,B */

  AND_A_X (cpustate->b.B)
  break;
case 0xA1: /*      AND A,C */

  AND_A_X (cpustate->b.C)
  break;
case 0xA2: /*      AND A,D */

  AND_A_X (cpustate->b.D)
  break;
case 0xA3: /*      AND A,E */

  AND_A_X (cpustate->b.E)
  break;
case 0xA4: /*      AND A,H */

  AND_A_X (cpustate->b.H)
  break;
case 0xA5: /*      AND A,L */

  AND_A_X (cpustate->b.L)
  break;
case 0xA6: /*      AND A,(HL) */

  x = mem_ReadByte (cpustate, cpustate->w.HL);

  AND_A_X (x)
  break;
case 0xA7: /*      AND A,A */

  cpustate->b.F = (cpustate->b.A == 0) ? (FLAG_H | FLAG_Z) : FLAG_H;
  break;
case 0xA8: /*      XOR A,B */

  XOR_A_X (cpustate->b.B)
  break;
case 0xA9: /*      XOR A,C */

  XOR_A_X (cpustate->b.C)
  break;
case 0xAA: /*      XOR A,D */

  XOR_A_X (cpustate->b.D)
  break;
case 0xAB: /*      XOR A,E */

  XOR_A_X (cpustate->b.E)
  break;
case 0xAC: /*      XOR A,H */

  XOR_A_X (cpustate->b.H)
  break;
case 0xAD: /*      XOR A,L */

  XOR_A_X (cpustate->b.L)
  break;
case 0xAE: /*      XOR A,(HL) */

  x = mem_ReadByte (cpustate, cpustate->w.HL);

  XOR_A_X (x)
  break;
case 0xAF: /*      XOR A,A */

  XOR_A_X (cpustate->b.A)
  break;
case 0xB0: /*      OR A,B */

  OR_A_X (cpustate->b.B)
  break;
case 0xB1: /*      OR A,C */

  OR_A_X (cpustate->b.C)
  break;
case 0xB2: /*      OR A,D */

  OR_A_X (cpustate->b.D)
  break;
case 0xB3: /*      OR A,E */

  OR_A_X (cpustate->b.E)
  break;
case 0xB4: /*      OR A,H */

  OR_A_X (cpustate->b.H)
  break;
case 0xB5: /*      OR A,L */

  OR_A_X (cpustate->b.L)
  break;
case 0xB6: /*      OR A,(HL) */

  x = mem_ReadByte (cpustate, cpustate->w.HL);

  OR_A_X (x)
  break;
case 0xB7: /*      OR A,A */

  OR_A_X (cpustate->b.A)
  break;
case 0xB8: /*      CP A,B */

  CP_A_X (cpustate->b.B)
  break;
case 0xB9: /*      CP A,C */

  CP_A_X (cpustate->b.C)
  break;
case 0xBA: /*      CP A,D */

  CP_A_X (cpustate->b.D)
  break;
case 0xBB: /*      CP A,E */

  CP_A_X (cpustate->b.E)
  break;
case 0xBC: /*      CP A,H */

  CP_A_X (cpustate->b.H)
  break;
case 0xBD: /*      CP A,L */

  CP_A_X (cpustate->b.L)
  break;
case 0xBE: /*      CP A,(HL) */

  x = mem_ReadByte (cpustate, cpustate->w.HL);

  CP_A_X (x)
  break;
case 0xBF: /*      CP A,A */

  CP_A_X (cpustate->b.A)
  break;
case 0xC0: /*      RET NZ */
	CYCLES_PASSED( 4 );
	if (!(cpustate->b.F & FLAG_Z))
	{
		cpustate->w.PC = mem_ReadWord (cpustate, cpustate->w.SP);
		cpustate->w.SP += 2;
		CYCLES_PASSED( 4 );
	}
	break;
case 0xC1: /*      POP BC */

  cpustate->w.BC = mem_ReadWord (cpustate, cpustate->w.SP);
  cpustate->w.SP += 2;
  break;
case 0xC2: /*      JP NZ,n16 */
	{
		UINT16 addr = mem_ReadWord (cpustate, cpustate->w.PC);
		cpustate->w.PC += 2;

		if ( ! (cpustate->b.F & FLAG_Z) )
		{
			cpustate->w.PC = addr;
			CYCLES_PASSED( 4 );
		}
	}
	break;
case 0xC3: /*      JP n16 */
	cpustate->w.PC = mem_ReadWord (cpustate, cpustate->w.PC);
	CYCLES_PASSED( 4 );
	break;
case 0xC4: /*      CALL NZ,n16 */
	{
		UINT16 addr = mem_ReadWord (cpustate, cpustate->w.PC);
		cpustate->w.PC += 2;

		if ( ! (cpustate->b.F & FLAG_Z) )
		{
			cpustate->w.SP -= 2;
			mem_WriteWord (cpustate, cpustate->w.SP, cpustate->w.PC);
			cpustate->w.PC = addr;
			CYCLES_PASSED( 4 );
		}
	}
	break;
case 0xC5: /*      PUSH BC */
	cpustate->w.SP -= 2;
	mem_WriteWord (cpustate, cpustate->w.SP, cpustate->w.BC);
	CYCLES_PASSED( 4 );
	break;
case 0xC6: /*      ADD A,n8 */

  x = mem_ReadByte (cpustate, cpustate->w.PC++);
  ADD_A_X (x)
  break;
case 0xC7: /*      RST 0 */
	cpustate->w.SP -= 2;
	mem_WriteWord (cpustate, cpustate->w.SP, cpustate->w.PC);
	cpustate->w.PC = 0;
	CYCLES_PASSED( 4 );
	break;
case 0xC8: /*      RET Z */
	CYCLES_PASSED( 4 );
	if (cpustate->b.F & FLAG_Z)
	{
		cpustate->w.PC = mem_ReadWord (cpustate, cpustate->w.SP);
		cpustate->w.SP += 2;
		CYCLES_PASSED( 4 );
	}
	break;
case 0xC9: /*      RET */
	cpustate->w.PC = mem_ReadWord (cpustate, cpustate->w.SP);
	cpustate->w.SP += 2;
	CYCLES_PASSED( 4 );
	break;
case 0xCA: /*      JP Z,n16 */
	{
		UINT16 addr = mem_ReadWord (cpustate, cpustate->w.PC);
		cpustate->w.PC += 2;

		if (cpustate->b.F & FLAG_Z)
		{
			cpustate->w.PC = addr;
			CYCLES_PASSED( 4 );
		}
	}
	break;
case 0xCB: /*      PREFIX! */
  x = mem_ReadByte (cpustate, cpustate->w.PC++);
  switch (x)
  {
    #include "opc_cb.h"
  }
  break;
case 0xCC: /*      CALL Z,n16 */
	{
		UINT16 addr = mem_ReadWord (cpustate, cpustate->w.PC);
		cpustate->w.PC += 2;

		if (cpustate->b.F & FLAG_Z)
		{
			cpustate->w.SP -= 2;
			mem_WriteWord (cpustate, cpustate->w.SP, cpustate->w.PC);
			cpustate->w.PC = addr;
			CYCLES_PASSED( 4 );
		}
	}
	break;
case 0xCD: /*      CALL n16 */
	{
		UINT16 addr = mem_ReadWord (cpustate, cpustate->w.PC);
		cpustate->w.PC += 2;

		cpustate->w.SP -= 2;
		mem_WriteWord (cpustate, cpustate->w.SP, cpustate->w.PC);
		cpustate->w.PC = addr;
		CYCLES_PASSED( 4 );
	}
	break;
case 0xCE: /*      ADC A,n8 */

  x = mem_ReadByte (cpustate, cpustate->w.PC++);
  ADC_A_X (x)
  break;
case 0xCF: /*      RST 8 */
	cpustate->w.SP -= 2;
	mem_WriteWord (cpustate, cpustate->w.SP, cpustate->w.PC);
	cpustate->w.PC = 8;
	CYCLES_PASSED( 4 );
	break;
case 0xD0: /*      RET NC */
	CYCLES_PASSED( 4 );
	if (!(cpustate->b.F & FLAG_C))
	{
		cpustate->w.PC = mem_ReadWord (cpustate, cpustate->w.SP);
		cpustate->w.SP += 2;
		CYCLES_PASSED( 4 );
	}
	break;
case 0xD1: /*      POP DE */

  cpustate->w.DE = mem_ReadWord (cpustate, cpustate->w.SP);
  cpustate->w.SP += 2;
  break;
case 0xD2: /*      JP NC,n16 */
	{
		UINT16 addr = mem_ReadWord (cpustate, cpustate->w.PC);
		cpustate->w.PC += 2;

		if ( ! (cpustate->b.F & FLAG_C) )
		{
			cpustate->w.PC = addr;
			CYCLES_PASSED( 4 );
		}
	}
	break;
case 0xD3: /*      EH? */
  break;
case 0xD4: /*      CALL NC,n16 */
	{
		UINT16 addr = mem_ReadWord (cpustate, cpustate->w.PC);
		cpustate->w.PC += 2;

		if ( ! (cpustate->b.F & FLAG_C) )
		{
			cpustate->w.SP -= 2;
			mem_WriteWord (cpustate, cpustate->w.SP, cpustate->w.PC);
			cpustate->w.PC = addr;
			CYCLES_PASSED( 4 );
		}
	}
	break;
case 0xD5: /*      PUSH DE */
	cpustate->w.SP -= 2;
	mem_WriteWord (cpustate, cpustate->w.SP, cpustate->w.DE);
	CYCLES_PASSED( 4 );
	break;
case 0xD6: /*      SUB A,n8 */

  x = mem_ReadByte (cpustate, cpustate->w.PC++);
  SUB_A_X (x)
  break;
case 0xD7: /*      RST     $10 */
	cpustate->w.SP -= 2;
	mem_WriteWord (cpustate, cpustate->w.SP, cpustate->w.PC);
	cpustate->w.PC = 0x10;
	CYCLES_PASSED( 4 );
	break;
case 0xD8: /*      RET C */
	CYCLES_PASSED( 4 );
	if (cpustate->b.F & FLAG_C)
	{
		cpustate->w.PC = mem_ReadWord (cpustate, cpustate->w.SP);
		cpustate->w.SP += 2;
		CYCLES_PASSED( 4 );
	}
	break;
case 0xD9: /*      RETI */
	cpustate->w.PC = mem_ReadWord (cpustate, cpustate->w.SP);
	cpustate->w.SP += 2;
	cpustate->w.enable |= IME;
	CYCLES_PASSED( 4 );
	break;
case 0xDA: /*      JP C,n16 */
	{
		UINT16 addr = mem_ReadWord (cpustate, cpustate->w.PC);
		cpustate->w.PC += 2;

		if (cpustate->b.F & FLAG_C)
		{
			cpustate->w.PC = addr;
			CYCLES_PASSED( 4 );
		}
	}
	break;
case 0xDB: /*      EH? */
  break;
case 0xDC: /*      CALL C,n16 */
	{
		UINT16 addr = mem_ReadWord (cpustate, cpustate->w.PC);
		cpustate->w.PC += 2;

		if (cpustate->b.F & FLAG_C)
		{
			cpustate->w.SP -= 2;
			mem_WriteWord (cpustate, cpustate->w.SP, cpustate->w.PC);
			cpustate->w.PC = addr;
			CYCLES_PASSED( 4 );
		}
	}
	break;
case 0xDD: /*      EH? */
  break;
case 0xDE: /*      SBC A,n8 */

  x = mem_ReadByte (cpustate, cpustate->w.PC++);
  SBC_A_X (x)
  break;
case 0xDF: /*      RST     $18 */
	cpustate->w.SP -= 2;
	mem_WriteWord (cpustate, cpustate->w.SP, cpustate->w.PC);
	cpustate->w.PC = 0x18;
	CYCLES_PASSED( 4 );
	break;
case 0xE0: /*      LD      ($FF00+n8),A */
  {
	UINT8 v = mem_ReadByte (cpustate, cpustate->w.PC++);
	mem_WriteByte (cpustate, 0xFF00 + v, cpustate->b.A);
  }
  break;
case 0xE1: /*      POP HL */

  cpustate->w.HL = mem_ReadWord (cpustate, cpustate->w.SP);
  cpustate->w.SP += 2;
  break;
case 0xE2: /*      LD ($FF00+C),A */

  mem_WriteByte (cpustate, (UINT16) (0xFF00 + cpustate->b.C), cpustate->b.A);
  break;
case 0xE3: /*      EH? */
  break;
case 0xE4: /*      EH? */
  break;
case 0xE5: /*      PUSH HL */
	cpustate->w.SP -= 2;
	mem_WriteWord (cpustate, cpustate->w.SP, cpustate->w.HL);
	CYCLES_PASSED( 4 );
	break;
case 0xE6: /*      AND A,n8 */

  x = mem_ReadByte (cpustate, cpustate->w.PC++);
  AND_A_X (x)
  break;
case 0xE7: /*      RST $20 */
	cpustate->w.SP -= 2;
	mem_WriteWord (cpustate, cpustate->w.SP, cpustate->w.PC);
	cpustate->w.PC = 0x20;
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

	n = (INT8) mem_ReadByte (cpustate, cpustate->w.PC++);

	if ( ( cpustate->w.SP & 0xFF ) + (UINT8)(n & 0xFF) > 0xFF )
    {
      cpustate->b.F = FLAG_C;
    }
    else
    {
      cpustate->b.F = 0;
    }

    if ( ( cpustate->w.SP & 0x0F ) + ( n & 0x0F ) > 0x0F )
    {
      cpustate->b.F |= FLAG_H;
    }

	cpustate->w.SP = (UINT16) ( cpustate->w.SP + n );
  }
  CYCLES_PASSED( 8 );
  break;
case 0xE9: /*      JP (HL) */

  cpustate->w.PC = cpustate->w.HL;
  break;
case 0xEA: /*      LD (n16),A */

  mem_WriteByte (cpustate, mem_ReadWord (cpustate, cpustate->w.PC), cpustate->b.A);
  cpustate->w.PC += 2;
  break;
case 0xEB: /*      EH? */
  break;
case 0xEC: /*      EH? */
  break;
case 0xED: /*      EH? */
  break;
case 0xEE: /*      XOR A,n8 */

  x = mem_ReadByte (cpustate, cpustate->w.PC++);
  XOR_A_X (x)
  break;
case 0xEF: /*      RST $28 */
	cpustate->w.SP -= 2;
	mem_WriteWord (cpustate, cpustate->w.SP, cpustate->w.PC);
	cpustate->w.PC = 0x28;
	CYCLES_PASSED( 4 );
	break;
case 0xF0: /*      LD A,($FF00+n8) */
  {
	UINT8 v = mem_ReadByte (cpustate, cpustate->w.PC++);
	cpustate->b.A = mem_ReadByte (cpustate, 0xFF00 + v);
  }
  break;
case 0xF1: /*      POP AF */

  cpustate->w.AF = (UINT16) (mem_ReadWord (cpustate, cpustate->w.SP) & 0xFFF0);
  cpustate->w.SP += 2;
  break;
case 0xF2: /*      LD A,($FF00+C) */

  cpustate->b.A = mem_ReadByte (cpustate, (UINT16) (0xFF00 + cpustate->b.C));
  break;
case 0xF3: /*      DI */
  cpustate->w.ei_delay = 0;
  cpustate->w.enable &= ~IME;
  break;
case 0xF4: /*      EH? */
  break;
case 0xF5: /*      PUSH AF */
	cpustate->w.SP -= 2;
	mem_WriteWord (cpustate, cpustate->w.SP, (UINT16) (cpustate->w.AF & 0xFFF0));
	CYCLES_PASSED( 4 );
	break;
case 0xF6: /*      OR A,n8 */

  x = mem_ReadByte (cpustate, cpustate->w.PC++);
  OR_A_X (x)
  break;
case 0xF7: /*      RST $30 */
	cpustate->w.SP -= 2;
	mem_WriteWord (cpustate, cpustate->w.SP, cpustate->w.PC);
	cpustate->w.PC = 0x30;
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

	n = (INT8) mem_ReadByte (cpustate, cpustate->w.PC++);

	if ( ( cpustate->w.SP & 0xFF ) + (UINT8)(n & 0xFF) > 0xFF )
    {
      cpustate->b.F = FLAG_C;
    }
    else
    {
      cpustate->b.F = 0;
    }

	if ( ( cpustate->w.SP & 0x0F ) + ( n & 0x0F ) > 0x0F )
    {
      cpustate->b.F |= FLAG_H;
    }

	cpustate->w.HL = (UINT16) ( cpustate->w.SP + n );
  }
  CYCLES_PASSED( 4 );
  break;
case 0xF9: /*      LD SP,HL */
	cpustate->w.SP = cpustate->w.HL;
	CYCLES_PASSED( 4 );
	break;
case 0xFA: /*      LD A,(n16) */
	cpustate->b.A = mem_ReadByte (cpustate, mem_ReadWord (cpustate, cpustate->w.PC));
	cpustate->w.PC += 2;
	break;
case 0xFB: /*      EI */
	cpustate->w.enable |= IME;
	cpustate->w.ei_delay = 1;
	break;
case 0xFC: /*      EH? */
	break;
case 0xFD: /*      EH? */
	break;
case 0xFE: /*      CP A,n8 */
	x = mem_ReadByte (cpustate, cpustate->w.PC++);
	CP_A_X (x)
	break;
case 0xFF: /*      RST $38 */
	cpustate->w.SP -= 2;
	mem_WriteWord (cpustate, cpustate->w.SP, cpustate->w.PC);
	cpustate->w.PC = 0x38;
	CYCLES_PASSED( 4 );
	break;
