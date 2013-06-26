INLINE UINT8 ADD8( minx_state *minx, UINT8 arg1, UINT8 arg2 )
{
	UINT32 res = arg1 + arg2;
	minx->F = ( minx->F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( ( arg2 ^ arg1 ^ 0x80 ) & ( arg2 ^ res ) & 0x80 ) ? FLAG_O : 0 )
		| ( ( res & 0xFF00 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFF;
}


INLINE UINT16 ADD16( minx_state *minx, UINT16 arg1, UINT16 arg2 )
{
	UINT32 res = arg1 + arg2;
	minx->F = ( minx->F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x8000 ) ? FLAG_S : 0 )
		| ( ( ( arg2 ^ arg1 ^ 0x8000 ) & ( arg2 ^ res ) & 0x8000 ) ? FLAG_O : 0 )
		| ( ( res & 0xFF0000 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFFFF;
}


INLINE UINT8 ADDC8( minx_state *minx, UINT8 arg1, UINT8 arg2 )
{
		UINT32 res = arg1 + arg2 + ( ( minx->F & FLAG_C ) ? 1 : 0 );
	minx->F = ( minx->F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( ( arg2 ^ arg1 ^ 0x80 ) & ( arg2 ^ res ) & 0x80 ) ? FLAG_O : 0 )
		| ( ( res & 0xFF00 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFF;
}


INLINE UINT16 ADDC16( minx_state *minx, UINT16 arg1, UINT16 arg2 )
{
	UINT32 res = arg1 + arg2 + ( ( minx->F & FLAG_C ) ? 1 : 0 );
	minx->F = ( minx->F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x8000 ) ? FLAG_S : 0 )
		| ( ( ( arg2 ^ arg1 ^ 0x8000 ) & ( arg2 ^ res ) & 0x8000 ) ? FLAG_O : 0 )
		| ( ( res & 0xFF0000 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFFFF;
}


INLINE UINT8 INC8( minx_state *minx, UINT8 arg )
{
	UINT8 old_F = minx->F;
	UINT8 res = ADD8( minx, arg, 1 );
	minx->F = ( old_F & ~ ( FLAG_Z ) )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


INLINE UINT16 INC16( minx_state *minx, UINT16 arg )
{
	UINT8 old_F = minx->F;
	UINT16 res = ADD16( minx, arg, 1 );
	minx->F = ( old_F & ~ ( FLAG_Z ) )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


INLINE UINT8 SUB8( minx_state *minx, UINT8 arg1, UINT8 arg2 )
{
	UINT32 res = arg1 - arg2;
	minx->F = ( minx->F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( ( arg2 ^ arg1 ) & ( arg1 ^ res ) & 0x80 ) ? FLAG_O : 0 )
		| ( ( res & 0xFF00 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFF;
}


INLINE UINT16 SUB16( minx_state *minx, UINT16 arg1, UINT16 arg2 )
{
	UINT32 res = arg1 - arg2;
	minx->F = ( minx->F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x8000 ) ? FLAG_S : 0 )
		| ( ( ( arg2 ^ arg1 ) & ( arg1 ^ res ) & 0x8000 ) ? FLAG_O : 0 )
		| ( ( res & 0xFF0000 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFFFF;
}


INLINE UINT8 SUBC8( minx_state *minx, UINT8 arg1, UINT8 arg2 )
{
	UINT32 res = arg1 - arg2 - ( ( minx->F & FLAG_C ) ? 1 : 0 );
	minx->F = ( minx->F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( ( arg2 ^ arg1 ) & ( arg1 ^ res ) & 0x80 ) ? FLAG_O : 0 )
		| ( ( res & 0xFF00 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFF;
}


INLINE UINT16 SUBC16( minx_state *minx, UINT16 arg1, UINT16 arg2 )
{
	UINT32 res = arg1 - arg2 - ( ( minx->F & FLAG_C ) ? 1 : 0 );
	minx->F = ( minx->F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x8000 ) ? FLAG_S : 0 )
		| ( ( ( arg2 ^ arg1 ) & ( arg1 ^ res ) & 0x8000 ) ? FLAG_O : 0 )
		| ( ( res & 0xFF0000 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFFFF;
}


INLINE UINT8 DEC8( minx_state *minx, UINT8 arg )
{
	UINT8 old_F = minx->F;
	UINT8 res = SUB8( minx, arg, 1 );
	minx->F = ( old_F & ~ ( FLAG_Z ) )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


INLINE UINT16 DEC16( minx_state *minx, UINT16 arg )
{
	UINT8 old_F = minx->F;
	UINT16 res = SUB16( minx, arg, 1 );
	minx->F = ( old_F & ~ ( FLAG_Z ) )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


INLINE UINT8 AND8( minx_state *minx, UINT8 arg1, UINT8 arg2 )
{
	UINT8 res = arg1 & arg2;
	minx->F = ( minx->F & ~ ( FLAG_S | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


INLINE UINT8 OR8( minx_state *minx, UINT8 arg1, UINT8 arg2 )
{
	UINT8 res = arg1 | arg2;
	minx->F = ( minx->F & ~ ( FLAG_S | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


INLINE UINT8 XOR8( minx_state *minx, UINT8 arg1, UINT8 arg2 )
{
	UINT8 res = arg1 ^ arg2;
	minx->F = ( minx->F & ~ ( FLAG_S | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


INLINE UINT8 NOT8( minx_state *minx, UINT8 arg )
{
	UINT8 res = ~arg;
	minx->F = ( minx->F & ~ ( FLAG_S | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


INLINE UINT8 NEG8( minx_state *minx, UINT8 arg )
{
	UINT8 res = -arg;
	minx->F = ( minx->F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


INLINE UINT8 SAL8( minx_state *minx, UINT8 arg )
{
	UINT16 res = arg << 1;
	minx->F = ( minx->F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( arg != 0 && res == 0 ) ? FLAG_O : 0 )
		| ( ( arg & 0x80 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


INLINE UINT8 SAR8( minx_state *minx, UINT8 arg )
{
	UINT16 res = ( arg >> 1 ) | ( arg & 0x80 );
	minx->F = ( minx->F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( arg != 0x80 && res == 0x80 ) ? FLAG_O : 0 )
		| ( ( arg & 0x01 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFF;
}


INLINE UINT8 SHL8( minx_state *minx, UINT8 arg )
{
	UINT16 res = arg << 1;
	minx->F = ( minx->F & ~ ( FLAG_S | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( arg & 0x80 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


INLINE UINT8 SHR8( minx_state *minx, UINT8 arg )
{
	UINT16 res = arg >> 1;
	minx->F = ( minx->F & ~ ( FLAG_S | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( arg & 0x01 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFF;
}


INLINE UINT8 ROLC8( minx_state *minx, UINT8 arg )
{
	UINT16 res = ( arg << 1 ) | ( ( minx->F & FLAG_C ) ? 1 : 0 );
	minx->F = ( minx->F & ~ ( FLAG_S | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( arg & 0x80 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFF;
}


INLINE UINT8 RORC8( minx_state *minx, UINT8 arg )
{
	UINT16 res = ( arg >> 1 ) | ( ( minx->F & FLAG_C ) ? 0x80 : 0 );
	minx->F = ( minx->F & ~ ( FLAG_S | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( arg & 0x01 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFF;
}


INLINE UINT8 ROL8( minx_state *minx, UINT8 arg )
{
	UINT16 res = ( arg << 1 ) | ( ( arg & 0x80 ) ? 1 : 0 );
	minx->F = ( minx->F & ~ ( FLAG_S | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( arg & 0x80 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFF;
}


INLINE UINT8 ROR8( minx_state *minx, UINT8 arg )
{
	UINT16 res = ( arg >> 1 ) | ( ( arg & 0x01 ) ? 0x80 : 0 );
	minx->F = ( minx->F & ~ ( FLAG_S | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( arg & 0x01 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFF;
}


INLINE void PUSH8( minx_state *minx, UINT8 arg )
{
	minx->SP = minx->SP - 1;
	WR( minx->SP, arg );
}


INLINE void PUSH16( minx_state *minx, UINT16 arg )
{
	PUSH8( minx, arg >> 8 );
	PUSH8( minx, arg & 0x00FF );
}


INLINE UINT8 POP8( minx_state *minx )
{
	UINT8 res = RD( minx->SP );
	minx->SP = minx->SP + 1;
	return res;
}


INLINE UINT16 POP16( minx_state *minx )
{
	return POP8(minx) | ( POP8(minx) << 8 );
}


INLINE void JMP( minx_state *minx, UINT16 arg )
{
	minx->V = minx->U;
	minx->PC = arg;
}


INLINE void CALL( minx_state *minx, UINT16 arg )
{
	PUSH8( minx, minx->V );
	PUSH16( minx, minx->PC );
	JMP( minx, arg );
}


#define AD1_IHL UINT32 addr1 = ( minx->I << 16 ) | minx->HL
#define AD1_IN8 UINT32 addr1 = ( minx->I << 16 ) | ( minx->N << 8 ) | rdop(minx)
#define AD1_I16 UINT32 addr1 = ( minx->I << 16 ) | rdop16(minx)
#define AD1_XIX UINT32 addr1 = ( minx->XI << 16 ) | minx->X
#define AD1_YIY UINT32 addr1 = ( minx->YI << 16 ) | minx->Y
#define AD1_X8  UINT32 addr1 = ( minx->XI << 16 ) | ( minx->X + rdop(minx) )
#define AD1_Y8  UINT32 addr1 = ( minx->YI << 16 ) | ( minx->Y + rdop(minx) )
#define AD1_XL  UINT32 addr1 = ( minx->XI << 16 ) | ( minx->X + ( minx->HL & 0x00FF ) )
#define AD1_YL  UINT32 addr1 = ( minx->YI << 16 ) | ( minx->Y + ( minx->HL & 0x00FF ) )
#define AD2_IHL UINT32 addr2 = ( minx->I << 16 ) | minx->HL
#define AD2_IN8 UINT32 addr2 = ( minx->I << 16 ) | ( minx->N << 8 ) | rdop(minx)
#define AD2_I16 UINT32 addr2 = ( minx->I << 16 ) | rdop(minx); addr2 |= ( rdop(minx) << 8 )
#define AD2_XIX UINT32 addr2 = ( minx->XI << 16 ) | minx->X
#define AD2_YIY UINT32 addr2 = ( minx->YI << 16 ) | minx->Y
#define AD2_X8  UINT32 addr2 = ( minx->XI << 16 ) | ( minx->X + rdop(minx) )
#define AD2_Y8  UINT32 addr2 = ( minx->YI << 16 ) | ( minx->Y + rdop(minx) )
#define AD2_XL  UINT32 addr2 = ( minx->XI << 16 ) | ( minx->X + ( minx->HL & 0x00FF ) )
#define AD2_YL  UINT32 addr2 = ( minx->YI << 16 ) | ( minx->Y + ( minx->HL & 0x00FF ) )
