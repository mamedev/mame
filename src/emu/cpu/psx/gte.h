/*
 * PlayStation Geometry Transformation Engine emulator
 *
 * Copyright 2003-2011 smf
 *
 * divider reverse engineering by pSXAuthor.
 *
 */

#pragma once

#ifndef __PSXGTE_H__
#define __PSXGTE_H__

#include "emu.h"

#define GTE_OP( op ) ( ( op >> 20 ) & 31 )
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
	UINT32 Lm_E( UINT32 result );
};

#endif
