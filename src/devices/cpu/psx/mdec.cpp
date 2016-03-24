// license:BSD-3-Clause
// copyright-holders:smf
/*
 * PlayStation Motion Decoder emulator
 *
 * Copyright 2003-2011 smf
 *
 * Thanks to Oliver Galibert for help figuring out IDCT
 *
 */

#include "emu.h"
#include "dma.h"
#include "mdec.h"

#define VERBOSE_LEVEL ( 0 )

static inline void ATTR_PRINTF(3,4) verboselog( device_t& device, int n_level, const char *s_fmt, ... )
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		device.logerror( "%s: %s", device.machine().describe_context(), buf );
	}
}

const device_type PSX_MDEC = &device_creator<psxmdec_device>;

psxmdec_device::psxmdec_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, PSX_MDEC, "Sony PSX MDEC", tag, owner, clock, "psxmdec", __FILE__), n_decoded(0), n_offset(0), n_0_command(0), n_0_address(0), n_0_size(0), n_1_command(0), n_1_status(0)
{
}

void psxmdec_device::device_reset()
{
	n_0_command = 0;
	n_0_address = 0;
	n_0_size = 0;
	n_1_command = 0;
	n_1_status = 0;
	n_offset = 0;
	n_decoded = 0;
}

void psxmdec_device::device_post_load()
{
	mdec_cos_precalc();
}

void psxmdec_device::device_start()
{
	for( int n = 0; n < 256; n++ )
	{
		p_n_clamp8[ n ] = 0;
		p_n_clamp8[ n + 256 ] = n;
		p_n_clamp8[ n + 512 ] = 255;

		p_n_r5[ n ] = 0;
		p_n_r5[ n + 256 ] = ( n >> 3 );
		p_n_r5[ n + 512 ] = ( 255 >> 3 );

		p_n_g5[ n ] = 0;
		p_n_g5[ n + 256 ] = ( n >> 3 ) << 5;
		p_n_g5[ n + 512 ] = ( 255 >> 3 ) << 5;

		p_n_b5[ n ] = 0;
		p_n_b5[ n + 256 ] = ( n >> 3 ) << 10;
		p_n_b5[ n + 512 ] = ( 255 >> 3 ) << 10;
	}

	save_item( NAME( n_0_command ) );
	save_item( NAME( n_0_address ) );
	save_item( NAME( n_0_size ) );
	save_item( NAME( n_1_command ) );
	save_item( NAME( n_1_status ) );
	save_item( NAME( p_n_quantize_y ) );
	save_item( NAME( p_n_quantize_uv ) );
	save_item( NAME( p_n_cos ) );
}

#ifdef UNUSED_FUNCTION
static inline void psxwriteword( UINT32 *p_n_psxram, UINT32 n_address, UINT16 n_data )
{
	*( (UINT16 *)( (UINT8 *)p_n_psxram + WORD_XOR_LE( n_address ) ) ) = n_data;
}
#endif

static inline UINT16 psxreadword( UINT32 *p_n_psxram, UINT32 n_address )
{
	return *( (UINT16 *)( (UINT8 *)p_n_psxram + WORD_XOR_LE( n_address ) ) );
}

static const UINT32 m_p_n_mdec_zigzag[ DCTSIZE2 ] =
{
		0,  1,  8, 16,  9,  2,  3, 10,
	17, 24, 32, 25, 18, 11,  4,  5,
	12, 19, 26, 33, 40, 48, 41, 34,
	27, 20, 13,  6,  7, 14, 21, 28,
	35, 42, 49, 56, 57, 50, 43, 36,
	29, 22, 15, 23, 30, 37, 44, 51,
	58, 59, 52, 45, 38, 31, 39, 46,
	53, 60, 61, 54, 47, 55, 62, 63
};

void psxmdec_device::mdec_cos_precalc()
{
	UINT32 n_x;
	UINT32 n_y;
	UINT32 n_u;
	UINT32 n_v;
	INT32 *p_n_precalc = p_n_cos_precalc;

	for( n_y = 0; n_y < 8; n_y++ )
	{
		for( n_x = 0; n_x < 8; n_x++ )
		{
			for( n_v = 0; n_v < 8; n_v++ )
			{
				for( n_u = 0; n_u < 8; n_u++ )
				{
					*( p_n_precalc++ ) =
						( ( p_n_cos[ ( n_u * 8 ) + n_x ] *
						p_n_cos[ ( n_v * 8 ) + n_y ] ) >> ( 30 - MDEC_COS_PRECALC_BITS ) );
				}
			}
		}
	}
}

void psxmdec_device::mdec_idct( INT32 *p_n_src, INT32 *p_n_dst )
{
	INT32 *p_n_precalc = p_n_cos_precalc;

	for( UINT32 n_yx = 0; n_yx < DCTSIZE2; n_yx++ )
	{
		INT32 p_n_z[ 8 ];
		INT32 *p_n_data = p_n_src;

		memset( p_n_z, 0, sizeof( p_n_z ) );

		for( UINT32 n_vu = 0; n_vu < DCTSIZE2 / 8; n_vu++ )
		{
			p_n_z[ 0 ] += p_n_data[ 0 ] * p_n_precalc[ 0 ];
			p_n_z[ 1 ] += p_n_data[ 1 ] * p_n_precalc[ 1 ];
			p_n_z[ 2 ] += p_n_data[ 2 ] * p_n_precalc[ 2 ];
			p_n_z[ 3 ] += p_n_data[ 3 ] * p_n_precalc[ 3 ];
			p_n_z[ 4 ] += p_n_data[ 4 ] * p_n_precalc[ 4 ];
			p_n_z[ 5 ] += p_n_data[ 5 ] * p_n_precalc[ 5 ];
			p_n_z[ 6 ] += p_n_data[ 6 ] * p_n_precalc[ 6 ];
			p_n_z[ 7 ] += p_n_data[ 7 ] * p_n_precalc[ 7 ];
			p_n_data += 8;
			p_n_precalc += 8;
		}

		*( p_n_dst++ ) = ( p_n_z[ 0 ] + p_n_z[ 1 ] + p_n_z[ 2 ] + p_n_z[ 3 ] +
			p_n_z[ 4 ] + p_n_z[ 5 ] + p_n_z[ 6 ] + p_n_z[ 7 ] ) >> ( MDEC_COS_PRECALC_BITS + 2 );
	}
}

static inline UINT16 mdec_unpack_run( UINT16 n_packed )
{
	return n_packed >> 10;
}

static inline INT32 mdec_unpack_val( UINT16 n_packed )
{
	return ( ( (INT32)n_packed ) << 22 ) >> 22;
}

UINT32 psxmdec_device::mdec_unpack( UINT32 *p_n_psxram, UINT32 n_address )
{
	UINT8 n_z;
	INT32 n_qscale;
	UINT16 n_packed;
	INT32 *p_n_block;
	INT32 p_n_unpacked[ 64 ];
	INT32 *p_n_q;

	p_n_q = p_n_quantize_uv;
	p_n_block = m_p_n_unpacked;

	for( UINT32 n_block = 0; n_block < 6; n_block++ )
	{
		memset( p_n_unpacked, 0, sizeof( p_n_unpacked ) );

		if( n_block == 2 )
		{
			p_n_q = p_n_quantize_y;
		}
		n_packed = psxreadword( p_n_psxram, n_address );
		n_address += 2;
		if( n_packed == 0xfe00 )
		{
			break;
		}

		n_qscale = mdec_unpack_run( n_packed );
		p_n_unpacked[ 0 ] = mdec_unpack_val( n_packed ) * p_n_q[ 0 ];

		n_z = 0;
		for( ;; )
		{
			n_packed = psxreadword( p_n_psxram, n_address );
			n_address += 2;

			if( n_packed == 0xfe00 )
			{
				break;
			}
			n_z += mdec_unpack_run( n_packed ) + 1;
			if( n_z > 63 )
			{
				break;
			}
			p_n_unpacked[ m_p_n_mdec_zigzag[ n_z ] ] = ( mdec_unpack_val( n_packed ) * p_n_q[ n_z ] * n_qscale ) / 8;
		}
		mdec_idct( p_n_unpacked, p_n_block );
		p_n_block += DCTSIZE2;
	}
	return n_address;
}

static inline INT32 mdec_cr_to_r( INT32 n_cr )
{
	return ( 1435 * n_cr ) >> 10;
}

static inline INT32 mdec_cr_to_g( INT32 n_cr )
{
	return ( -731 * n_cr ) >> 10;
}

static inline INT32 mdec_cb_to_g( INT32 n_cb )
{
	return ( -351 * n_cb ) >> 10;
}

static inline INT32 mdec_cb_to_b( INT32 n_cb )
{
	return ( 1814 * n_cb ) >> 10;
}

UINT16 psxmdec_device::mdec_clamp_r5( INT32 n_r ) const
{
	return p_n_r5[ n_r + 128 + 256 ];
}

UINT16 psxmdec_device::mdec_clamp_g5( INT32 n_g ) const
{
	return p_n_g5[ n_g + 128 + 256 ];
}

UINT16 psxmdec_device::mdec_clamp_b5( INT32 n_b ) const
{
	return p_n_b5[ n_b + 128 + 256 ];
}

void psxmdec_device::mdec_makergb15( UINT32 n_address, INT32 n_r, INT32 n_g, INT32 n_b, INT32 *p_n_y, UINT16 n_stp )
{
	p_n_output[ WORD_XOR_LE( n_address + 0 ) / 2 ] = n_stp |
		mdec_clamp_r5( p_n_y[ 0 ] + n_r ) |
		mdec_clamp_g5( p_n_y[ 0 ] + n_g ) |
		mdec_clamp_b5( p_n_y[ 0 ] + n_b );

	p_n_output[ WORD_XOR_LE( n_address + 2 ) / 2 ] = n_stp |
		mdec_clamp_r5( p_n_y[ 1 ] + n_r ) |
		mdec_clamp_g5( p_n_y[ 1 ] + n_g ) |
		mdec_clamp_b5( p_n_y[ 1 ] + n_b );
}

void psxmdec_device::mdec_yuv2_to_rgb15( void )
{
	INT32 n_r;
	INT32 n_g;
	INT32 n_b;
	INT32 n_cb;
	INT32 n_cr;
	INT32 *p_n_cb;
	INT32 *p_n_cr;
	INT32 *p_n_y;
	UINT32 n_x;
	UINT32 n_y;
	UINT32 n_z;
	UINT16 n_stp;
	int n_address = 0;

	if( ( n_0_command & ( 1L << 25 ) ) != 0 )
	{
		n_stp = 0x8000;
	}
	else
	{
		n_stp = 0x0000;
	}

	p_n_cr = &m_p_n_unpacked[ 0 ];
	p_n_cb = &m_p_n_unpacked[ DCTSIZE2 ];
	p_n_y = &m_p_n_unpacked[ DCTSIZE2 * 2 ];

	for( n_z = 0; n_z < 2; n_z++ )
	{
		for( n_y = 0; n_y < 4; n_y++ )
		{
			for( n_x = 0; n_x < 4; n_x++ )
			{
				n_cr = *( p_n_cr );
				n_cb = *( p_n_cb );
				n_r = mdec_cr_to_r( n_cr );
				n_g = mdec_cr_to_g( n_cr ) + mdec_cb_to_g( n_cb );
				n_b = mdec_cb_to_b( n_cb );

				mdec_makergb15( ( n_address +  0 ), n_r, n_g, n_b, p_n_y, n_stp );
				mdec_makergb15( ( n_address + 32 ), n_r, n_g, n_b, p_n_y + 8, n_stp );

				n_cr = *( p_n_cr + 4 );
				n_cb = *( p_n_cb + 4 );
				n_r = mdec_cr_to_r( n_cr );
				n_g = mdec_cr_to_g( n_cr ) + mdec_cb_to_g( n_cb );
				n_b = mdec_cb_to_b( n_cb );

				mdec_makergb15( ( n_address + 16 ), n_r, n_g, n_b, p_n_y + DCTSIZE2, n_stp );
				mdec_makergb15( ( n_address + 48 ), n_r, n_g, n_b, p_n_y + DCTSIZE2 + 8, n_stp );

				p_n_cr++;
				p_n_cb++;
				p_n_y += 2;
				n_address += 4;
			}
			p_n_cr += 4;
			p_n_cb += 4;
			p_n_y += 8;
			n_address += 48;
		}
		p_n_y += DCTSIZE2;
	}
	n_decoded = ( 16 * 16 ) / 2;
}

UINT16 psxmdec_device::mdec_clamp8( INT32 n_r ) const
{
	return p_n_clamp8[ n_r + 128 + 256 ];
}

void psxmdec_device::mdec_makergb24( UINT32 n_address, INT32 n_r, INT32 n_g, INT32 n_b, INT32 *p_n_y, UINT32 n_stp )
{
	p_n_output[ WORD_XOR_LE( n_address + 0 ) / 2 ] = ( mdec_clamp8( p_n_y[ 0 ] + n_g ) << 8 ) | mdec_clamp8( p_n_y[ 0 ] + n_r );
	p_n_output[ WORD_XOR_LE( n_address + 2 ) / 2 ] = ( mdec_clamp8( p_n_y[ 1 ] + n_r ) << 8 ) | mdec_clamp8( p_n_y[ 0 ] + n_b );
	p_n_output[ WORD_XOR_LE( n_address + 4 ) / 2 ] = ( mdec_clamp8( p_n_y[ 1 ] + n_b ) << 8 ) | mdec_clamp8( p_n_y[ 1 ] + n_g );
}

void psxmdec_device::mdec_yuv2_to_rgb24( void )
{
	INT32 n_r;
	INT32 n_g;
	INT32 n_b;
	INT32 n_cb;
	INT32 n_cr;
	INT32 *p_n_cb;
	INT32 *p_n_cr;
	INT32 *p_n_y;
	UINT32 n_x;
	UINT32 n_y;
	UINT32 n_z;
	UINT32 n_stp;
	int n_address = 0;

	if( ( n_0_command & ( 1L << 25 ) ) != 0 )
	{
		n_stp = 0x80008000;
	}
	else
	{
		n_stp = 0x00000000;
	}

	p_n_cr = &m_p_n_unpacked[ 0 ];
	p_n_cb = &m_p_n_unpacked[ DCTSIZE2 ];
	p_n_y = &m_p_n_unpacked[ DCTSIZE2 * 2 ];

	for( n_z = 0; n_z < 2; n_z++ )
	{
		for( n_y = 0; n_y < 4; n_y++ )
		{
			for( n_x = 0; n_x < 4; n_x++ )
			{
				n_cr = *( p_n_cr );
				n_cb = *( p_n_cb );
				n_r = mdec_cr_to_r( n_cr );
				n_g = mdec_cr_to_g( n_cr ) + mdec_cb_to_g( n_cb );
				n_b = mdec_cb_to_b( n_cb );

				mdec_makergb24( ( n_address +  0 ), n_r, n_g, n_b, p_n_y, n_stp );
				mdec_makergb24( ( n_address + 48 ), n_r, n_g, n_b, p_n_y + 8, n_stp );

				n_cr = *( p_n_cr + 4 );
				n_cb = *( p_n_cb + 4 );
				n_r = mdec_cr_to_r( n_cr );
				n_g = mdec_cr_to_g( n_cr ) + mdec_cb_to_g( n_cb );
				n_b = mdec_cb_to_b( n_cb );

				mdec_makergb24( ( n_address + 24 ), n_r, n_g, n_b, p_n_y + DCTSIZE2, n_stp );
				mdec_makergb24( ( n_address + 72 ), n_r, n_g, n_b, p_n_y + DCTSIZE2 + 8, n_stp );

				p_n_cr++;
				p_n_cb++;
				p_n_y += 2;
				n_address += 6;
			}
			p_n_cr += 4;
			p_n_cb += 4;
			p_n_y += 8;
			n_address += 72;
		}
		p_n_y += DCTSIZE2;
	}
	n_decoded = ( 24 * 16 ) / 2;
}

void psxmdec_device::dma_write( UINT32 *p_n_psxram, UINT32 n_address, INT32 n_size )
{
	int n_index;

	verboselog( *this, 2, "mdec0_write( %08x, %08x )\n", n_address, n_size );

	switch( n_0_command >> 28 )
	{
	case 0x3:
		verboselog( *this, 1, "mdec decode %08x %08x %08x\n", n_0_command, n_address, n_size );
		n_0_address = n_address;
		n_0_size = n_size * 4;
		n_1_status |= ( 1L << 29 );
		break;
	case 0x4:
		verboselog( *this, 1, "mdec quantize table %08x %08x %08x\n", n_0_command, n_address, n_size );
		n_index = 0;
		while( n_size > 0 )
		{
			if( n_index < DCTSIZE2 )
			{
				p_n_quantize_y[ n_index + 0 ] = ( p_n_psxram[ n_address / 4 ] >> 0 ) & 0xff;
				p_n_quantize_y[ n_index + 1 ] = ( p_n_psxram[ n_address / 4 ] >> 8 ) & 0xff;
				p_n_quantize_y[ n_index + 2 ] = ( p_n_psxram[ n_address / 4 ] >> 16 ) & 0xff;
				p_n_quantize_y[ n_index + 3 ] = ( p_n_psxram[ n_address / 4 ] >> 24 ) & 0xff;
			}
			else if( n_index < DCTSIZE2 * 2 )
			{
				p_n_quantize_uv[ n_index + 0 - DCTSIZE2 ] = ( p_n_psxram[ n_address / 4 ] >> 0 ) & 0xff;
				p_n_quantize_uv[ n_index + 1 - DCTSIZE2 ] = ( p_n_psxram[ n_address / 4 ] >> 8 ) & 0xff;
				p_n_quantize_uv[ n_index + 2 - DCTSIZE2 ] = ( p_n_psxram[ n_address / 4 ] >> 16 ) & 0xff;
				p_n_quantize_uv[ n_index + 3 - DCTSIZE2 ] = ( p_n_psxram[ n_address / 4 ] >> 24 ) & 0xff;
			}
			n_index += 4;
			n_address += 4;
			n_size--;
		}
		break;
	case 0x6:
		verboselog( *this, 1, "mdec cosine table %08x %08x %08x\n", n_0_command, n_address, n_size );
		n_index = 0;
		while( n_size > 0 )
		{
			p_n_cos[ n_index + 0 ] = (INT16)( ( p_n_psxram[ n_address / 4 ] >> 0 ) & 0xffff );
			p_n_cos[ n_index + 1 ] = (INT16)( ( p_n_psxram[ n_address / 4 ] >> 16 ) & 0xffff );
			n_index += 2;
			n_address += 4;
			n_size--;
		}
		mdec_cos_precalc();
		break;
	default:
		verboselog( *this, 0, "mdec unknown command %08x %08x %08x\n", n_0_command, n_address, n_size );
		break;
	}
}

void psxmdec_device::dma_read( UINT32 *p_n_psxram, UINT32 n_address, INT32 n_size )
{
	UINT32 n_this;
	UINT32 n_nextaddress;

	verboselog( *this, 2, "mdec1_read( %08x, %08x )\n", n_address, n_size );
	if( ( n_0_command & ( 1L << 29 ) ) != 0 && n_0_size != 0 )
	{
		while( n_size > 0 )
		{
			if( n_decoded == 0 )
			{
				if( (int)n_0_size <= 0 )
				{
					osd_printf_debug( "ran out of data %08x\n", n_size );
					n_0_size = 0;
					break;
				}

				n_nextaddress = mdec_unpack( p_n_psxram, n_0_address );
				n_0_size -= n_nextaddress - n_0_address;
				n_0_address = n_nextaddress;

				if( ( n_0_command & ( 1L << 27 ) ) != 0 )
				{
					mdec_yuv2_to_rgb15();
				}
				else
				{
					mdec_yuv2_to_rgb24();
				}
				n_offset = 0;
				while((psxreadword( p_n_psxram, n_0_address ) == 0xfe00) && n_0_size)
				{
					n_0_address += 2;  // eat up 0xfe00
					n_0_size -= 2;
				}
			}

			n_this = n_decoded;
			if( n_this > n_size )
			{
				n_this = n_size;
			}
			n_decoded -= n_this;

			memcpy( (UINT8 *)p_n_psxram + n_address, (UINT8 *)p_n_output + n_offset, n_this * 4 );
			n_offset += n_this * 4;
			n_address += n_this * 4;
			n_size -= n_this;
		}

		if( (int)n_0_size < 0 )
		{
			osd_printf_debug( "ran out of data %d\n", n_0_size );
		}
	}
	else
	{
		osd_printf_debug( "mdec1_read no conversion :%08x:%08x:\n", n_0_command, n_0_size );
	}
	if((int)n_0_size <= 0)
		n_1_status &= ~( 1L << 29 );
}

WRITE32_MEMBER( psxmdec_device::write )
{
	switch( offset )
	{
	case 0:
		verboselog( *this, 2, "mdec 0 command %08x\n", data );
		n_0_command = data;
		break;
	case 1:
		verboselog( *this, 2, "mdec 1 command %08x\n", data );
		n_1_command = data;
		break;
	}
}

READ32_MEMBER( psxmdec_device::read )
{
	switch( offset )
	{
	case 0:
		verboselog( *this, 2, "mdec 0 status %08x\n", 0 );
		return 0;
	case 1:
		verboselog( *this, 2, "mdec 1 status %08x\n", n_1_status );
		return n_1_status;
	}
	return 0;
}
