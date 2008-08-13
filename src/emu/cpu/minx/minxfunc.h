INLINE UINT8 ADD8( UINT8 arg1, UINT8 arg2 )
{
	UINT32 res = arg1 + arg2;
	regs.F = ( regs.F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( ( arg2 ^ arg1 ^ 0x80 ) & ( arg2 ^ res ) & 0x80 ) ? FLAG_O : 0 )
		| ( ( res & 0xFF00 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFF;
}


INLINE UINT16 ADD16( UINT16 arg1, UINT16 arg2 )
{
	UINT32 res = arg1 + arg2;
	regs.F = ( regs.F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x8000 ) ? FLAG_S : 0 )
		| ( ( ( arg2 ^ arg1 ^ 0x8000 ) & ( arg2 ^ res ) & 0x8000 ) ? FLAG_O : 0 )
		| ( ( res & 0xFF0000 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFFFF;
}


INLINE UINT8 ADDC8( UINT8 arg1, UINT8 arg2 )
{
        UINT32 res = arg1 + arg2 + ( ( regs.F & FLAG_C ) ? 1 : 0 );
	regs.F = ( regs.F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( ( arg2 ^ arg1 ^ 0x80 ) & ( arg2 ^ res ) & 0x80 ) ? FLAG_O : 0 )
		| ( ( res & 0xFF00 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFF;
}


INLINE UINT16 ADDC16( UINT16 arg1, UINT16 arg2 )
{
	UINT32 res = arg1 + arg2 + ( ( regs.F & FLAG_C ) ? 1 : 0 );
	regs.F = ( regs.F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x8000 ) ? FLAG_S : 0 )
		| ( ( ( arg2 ^ arg1 ^ 0x8000 ) & ( arg2 ^ res ) & 0x8000 ) ? FLAG_O : 0 )
		| ( ( res & 0xFF0000 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFFFF;
}


INLINE UINT8 INC8( UINT8 arg )
{
	UINT8 old_F = regs.F;
	UINT8 res = ADD8( arg, 1 );
	regs.F = ( old_F & ~ ( FLAG_Z ) )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


INLINE UINT16 INC16( UINT16 arg )
{
	UINT8 old_F = regs.F;
	UINT16 res = ADD16( arg, 1 );
	regs.F = ( old_F & ~ ( FLAG_Z ) )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


INLINE UINT8 SUB8( UINT8 arg1, UINT8 arg2 )
{
	UINT32 res = arg1 - arg2;
	regs.F = ( regs.F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( ( arg2 ^ arg1 ) & ( arg1 ^ res ) & 0x80 ) ? FLAG_O : 0 )
		| ( ( res & 0xFF00 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFF;
}


INLINE UINT16 SUB16( UINT16 arg1, UINT16 arg2 )
{
	UINT32 res = arg1 - arg2;
	regs.F = ( regs.F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x8000 ) ? FLAG_S : 0 )
		| ( ( ( arg2 ^ arg1 ) & ( arg1 ^ res ) & 0x8000 ) ? FLAG_O : 0 )
		| ( ( res & 0xFF0000 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFFFF;
}


INLINE UINT8 SUBC8( UINT8 arg1, UINT8 arg2 )
{
	UINT32 res = arg1 - arg2 - ( ( regs.F & FLAG_C ) ? 1 : 0 );
	regs.F = ( regs.F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( ( arg2 ^ arg1 ) & ( arg1 ^ res ) & 0x80 ) ? FLAG_O : 0 )
		| ( ( res & 0xFF00 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFF;
}


INLINE UINT16 SUBC16( UINT16 arg1, UINT16 arg2 )
{
	UINT32 res = arg1 - arg2 - ( ( regs.F & FLAG_C ) ? 1 : 0 );
	regs.F = ( regs.F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x8000 ) ? FLAG_S : 0 )
		| ( ( ( arg2 ^ arg1 ) & ( arg1 ^ res ) & 0x8000 ) ? FLAG_O : 0 )
		| ( ( res & 0xFF0000 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFFFF;
}


INLINE UINT8 DEC8( UINT8 arg )
{
	UINT8 old_F = regs.F;
	UINT8 res = SUB8( arg, 1 );
	regs.F = ( old_F & ~ ( FLAG_Z ) )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


INLINE UINT16 DEC16( UINT16 arg )
{
	UINT8 old_F = regs.F;
	UINT16 res = SUB16( arg, 1 );
	regs.F = ( old_F & ~ ( FLAG_Z ) )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


INLINE UINT8 AND8( UINT8 arg1, UINT8 arg2 )
{
	UINT8 res = arg1 & arg2;
	regs.F = ( regs.F & ~ ( FLAG_S | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


INLINE UINT8 OR8( UINT8 arg1, UINT8 arg2 )
{
	UINT8 res = arg1 | arg2;
	regs.F = ( regs.F & ~ ( FLAG_S | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


INLINE UINT8 XOR8( UINT8 arg1, UINT8 arg2 )
{
	UINT8 res = arg1 ^ arg2;
	regs.F = ( regs.F & ~ ( FLAG_S | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


INLINE UINT8 NOT8( UINT8 arg )
{
	UINT8 res = ~arg;
	regs.F = ( regs.F & ~ ( FLAG_S | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


INLINE UINT8 NEG8( UINT8 arg )
{
	UINT8 res = -arg;
	regs.F = ( regs.F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


INLINE UINT8 SAL8( UINT8 arg )
{
	UINT16 res = arg << 1;
	regs.F = ( regs.F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( arg != 0 && res == 0 ) ? FLAG_O : 0 )
		| ( ( arg & 0x80 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


INLINE UINT8 SAR8( UINT8 arg )
{
	UINT16 res = ( arg >> 1 ) | ( arg & 0x80 );
	regs.F = ( regs.F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( arg != 0x80 && res == 0x80 ) ? FLAG_O : 0 )
		| ( ( arg & 0x01 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFF;
}


INLINE UINT8 SHL8( UINT8 arg )
{
	UINT16 res = arg << 1;
	regs.F = ( regs.F & ~ ( FLAG_S | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( arg & 0x80 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


INLINE UINT8 SHR8( UINT8 arg )
{
	UINT16 res = arg >> 1;
	regs.F = ( regs.F & ~ ( FLAG_S | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( arg & 0x01 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFF;
}


INLINE UINT8 ROLC8( UINT8 arg )
{
	UINT16 res = ( arg << 1 ) | ( ( regs.F & FLAG_C ) ? 1 : 0 );
	regs.F = ( regs.F & ~ ( FLAG_S | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( arg & 0x80 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFF;
}


INLINE UINT8 RORC8( UINT8 arg )
{
	UINT16 res = ( arg >> 1 ) | ( ( regs.F & FLAG_C ) ? 0x80 : 0 );
	regs.F = ( regs.F & ~ ( FLAG_S | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( arg & 0x01 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFF;
}


INLINE UINT8 ROL8( UINT8 arg )
{
	UINT16 res = ( arg << 1 ) | ( ( arg & 0x80 ) ? 1 : 0 );
	regs.F = ( regs.F & ~ ( FLAG_S | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( arg & 0x80 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFF;
}


INLINE UINT8 ROR8( UINT8 arg )
{
	UINT16 res = ( arg >> 1 ) | ( ( arg & 0x01 ) ? 0x80 : 0 );
	regs.F = ( regs.F & ~ ( FLAG_S | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( arg & 0x01 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFF;
}


INLINE void PUSH8( UINT8 arg )
{
	regs.SP = regs.SP - 1;
	WR( regs.SP, arg );
}


INLINE void PUSH16( UINT16 arg )
{
	PUSH8( arg >> 8 );
	PUSH8( arg & 0x00FF );
}


INLINE UINT8 POP8( void )
{
	UINT8 res = RD( regs.SP );
	regs.SP = regs.SP + 1;
	return res;
}


INLINE UINT16 POP16( void )
{
	return POP8() | ( POP8() << 8 );
}


INLINE void JMP( UINT16 arg )
{
	regs.V = regs.U;
	regs.PC = arg;
	change_pc( GET_MINX_PC );
}


INLINE void CALL( UINT16 arg )
{
	PUSH8( regs.V );
	PUSH16( regs.PC );
	JMP( arg );
}


#define AD1_IHL	UINT32 addr1 = ( regs.I << 16 ) | regs.HL
#define AD1_IN8	UINT32 addr1 = ( regs.I << 16 ) | ( regs.N << 8 ) | rdop()
#define AD1_I16	UINT32 addr1 = ( regs.I << 16 ) | rdop16()
#define AD1_XIX	UINT32 addr1 = ( regs.XI << 16 ) | regs.X
#define AD1_YIY	UINT32 addr1 = ( regs.YI << 16 ) | regs.Y
#define AD1_X8	UINT32 addr1 = ( regs.XI << 16 ) | ( regs.X + rdop() )
#define AD1_Y8	UINT32 addr1 = ( regs.YI << 16 ) | ( regs.Y + rdop() )
#define AD1_XL	UINT32 addr1 = ( regs.XI << 16 ) | ( regs.X + ( regs.HL & 0x00FF ) )
#define AD1_YL	UINT32 addr1 = ( regs.YI << 16 ) | ( regs.Y + ( regs.HL & 0x00FF ) )
#define AD2_IHL	UINT32 addr2 = ( regs.I << 16 ) | regs.HL
#define AD2_IN8	UINT32 addr2 = ( regs.I << 16 ) | ( regs.N << 8 ) | rdop()
#define AD2_I16	UINT32 addr2 = ( regs.I << 16 ) | rdop(); addr2 |= ( rdop() << 8 )
#define AD2_XIX	UINT32 addr2 = ( regs.XI << 16 ) | regs.X
#define AD2_YIY	UINT32 addr2 = ( regs.YI << 16 ) | regs.Y
#define AD2_X8	UINT32 addr2 = ( regs.XI << 16 ) | ( regs.X + rdop() )
#define AD2_Y8	UINT32 addr2 = ( regs.YI << 16 ) | ( regs.Y + rdop() )
#define AD2_XL	UINT32 addr2 = ( regs.XI << 16 ) | ( regs.X + ( regs.HL & 0x00FF ) )
#define AD2_YL	UINT32 addr2 = ( regs.YI << 16 ) | ( regs.Y + ( regs.HL & 0x00FF ) )
