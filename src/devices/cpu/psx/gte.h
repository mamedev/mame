// license:BSD-3-Clause
// copyright-holders:smf
/*
 * PlayStation Geometry Transformation Engine emulator
 *
 * Copyright 2003-2013 smf
 *
 */

#pragma once

#ifndef __PSXGTE_H__
#define __PSXGTE_H__

#include "emu.h"

#define GTE_SF( op ) ( ( op >> 19 ) & 1 )
#define GTE_MX( op ) ( ( op >> 17 ) & 3 )
#define GTE_V( op ) ( ( op >> 15 ) & 3 )
#define GTE_CV( op ) ( ( op >> 13 ) & 3 )
#define GTE_LM( op ) ( ( op >> 10 ) & 1 )
#define GTE_FUNCT( op ) ( op & 63 )

class gte
{
public:
	gte() : m_sf(0), m_mac0(0), m_mac1(0), m_mac2(0), m_mac3(0)
	{
	}

	PAIR m_cp2cr[ 32 ];
	PAIR m_cp2dr[ 32 ];

	UINT32 getcp2dr( UINT32 pc, int reg );
	void setcp2dr( UINT32 pc, int reg, UINT32 value );
	UINT32 getcp2cr( UINT32 pc, int reg );
	void setcp2cr( UINT32 pc, int reg, UINT32 value );
	int docop2( UINT32 pc, int gteop );

protected:
	class int44
	{
	public:
		int44( INT64 value ) :
			m_value( value ),
			m_positive_overflow( value > S64( 0x7ffffffffff ) ),
			m_negative_overflow( value < S64( -0x80000000000 ) )
		{
		}

		int44( INT64 value, bool positive_overflow, bool negative_overflow ) :
			m_value( value ),
			m_positive_overflow( positive_overflow ),
			m_negative_overflow( negative_overflow )
		{
		}

		int44 operator+( INT64 add )
		{
			INT64 value = ( ( m_value + add ) << 20 ) >> 20;

			return int44( value,
				m_positive_overflow || ( value < 0 && m_value >= 0 && add >= 0 ),
				m_negative_overflow || ( value >= 0 && m_value < 0 && add < 0 ) );
		}

		bool positive_overflow()
		{
			return m_positive_overflow;
		}

		bool negative_overflow()
		{
			return m_negative_overflow;
		}

		INT64 value()
		{
			return m_value;
		}

	private:
		INT64 m_value;
		bool m_positive_overflow;
		bool m_negative_overflow;
	};

	INT32 LIM( INT32 value, INT32 max, INT32 min, UINT32 flag );
	INT32 BOUNDS( int44 a, int max_flag, int min_flag );
	INT32 A1( int44 a );
	INT32 A2( int44 a );
	INT32 A3( int44 a );
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
	INT32 Lm_G1( INT64 a );
	INT32 Lm_G2( INT64 a );
	INT32 Lm_H( INT64 value, int sf );

	int m_sf;
	INT64 m_mac0;
	INT64 m_mac1;
	INT64 m_mac2;
	INT64 m_mac3;
};

#endif
