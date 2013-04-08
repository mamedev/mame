/*
 * PlayStation Geometry Transformation Engine emulator
 *
 * Copyright 2003-2013 smf
 *
 * divider reverse engineering by pSXAuthor.
 *
 */

#pragma once

#ifndef __PSXGTE_H__
#define __PSXGTE_H__

#include "emu.h"

#define GTE_OP( op ) ( ( op >> 20 ) & 31 ) /* not used */
#define GTE_SF( op ) ( ( op >> 19 ) & 1 )
#define GTE_MX( op ) ( ( op >> 17 ) & 3 )
#define GTE_V( op ) ( ( op >> 15 ) & 3 )
#define GTE_CV( op ) ( ( op >> 13 ) & 3 )
#define GTE_CD( op ) ( ( op >> 11 ) & 3 ) /* not used */
#define GTE_LM( op ) ( ( op >> 10 ) & 1 )
#define GTE_CT( op ) ( ( op >> 6 ) & 15 ) /* not used */
#define GTE_FUNCT( op ) ( op & 63 )

class gte
{
public:
	PAIR m_cp2cr[ 32 ];
	PAIR m_cp2dr[ 32 ];

	UINT32 getcp2dr( UINT32 pc, int reg );
	void setcp2dr( UINT32 pc, int reg, UINT32 value );
	UINT32 getcp2cr( UINT32 pc, int reg );
	void setcp2cr( UINT32 pc, int reg, UINT32 value );
	int docop2( UINT32 pc, int gteop );

protected:
	INT32 LIM( INT32 value, INT32 max, INT32 min, UINT32 flag );
	INT64 BOUNDS( INT64 n_value, INT64 n_max, int n_maxflag, INT64 n_min, int n_minflag );
	INT64 A1( INT64 a );
	INT64 A2( INT64 a );
	INT64 A3( INT64 a );
	INT32 Lm_B1( INT32 a, int lm );
	INT32 Lm_B2( INT32 a, int lm );
	INT32 Lm_B3( INT32 a, int lm );
	INT32 Lm_B3_sf( INT64 value, int sf, int lm );
	INT32 Lm_C1( INT32 a );
	INT32 Lm_C2( INT32 a );
	INT32 Lm_C3( INT32 a );
	INT32 Lm_D( INT64 a, int sf );
	UINT32 Lm_E( UINT32 result );
	INT64 F( INT64 a );
	INT32 Lm_G1( INT32 a );
	INT32 Lm_G2( INT32 a );
	INT32 Lm_H( INT64 value, int sf );

	int m_sf;
	INT64 m_mac0;
	INT64 m_mac1;
	INT64 m_mac2;
	INT64 m_mac3;
};

#endif
