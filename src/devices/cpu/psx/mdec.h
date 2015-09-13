// license:BSD-3-Clause
// copyright-holders:smf
/*
 * PlayStation DMA emulator
 *
 * Copyright 2003-2011 smf
 *
 */

#pragma once

#ifndef __PSXMDEC_H__
#define __PSXMDEC_H__

#include "emu.h"

extern const device_type PSX_MDEC;

#define DCTSIZE ( 8 )
#define DCTSIZE2 ( DCTSIZE * DCTSIZE )

#define MDEC_COS_PRECALC_BITS ( 21 )

class psxmdec_device : public device_t
{
public:
	psxmdec_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_WRITE32_MEMBER( write );
	DECLARE_READ32_MEMBER( read );

	void dma_write( UINT32 *ram, UINT32 n_address, INT32 n_size );
	void dma_read( UINT32 *ram, UINT32 n_address, INT32 n_size );

protected:
	virtual void device_start();
	virtual void device_reset();
	virtual void device_post_load();

private:
	void mdec_cos_precalc();
	void mdec_idct( INT32 *p_n_src, INT32 *p_n_dst );
	UINT32 mdec_unpack( UINT32 *ram, UINT32 n_address );
	UINT16 mdec_clamp_r5( INT32 n_r ) const;
	UINT16 mdec_clamp_g5( INT32 n_g ) const;
	UINT16 mdec_clamp_b5( INT32 n_b ) const;
	UINT16 mdec_clamp8( INT32 n_r ) const;
	void mdec_yuv2_to_rgb15( void );
	void mdec_yuv2_to_rgb24( void );
	void mdec_makergb15( UINT32 n_address, INT32 n_r, INT32 n_g, INT32 n_b, INT32 *p_n_y, UINT16 n_stp );
	void mdec_makergb24( UINT32 n_address, INT32 n_r, INT32 n_g, INT32 n_b, INT32 *p_n_y, UINT32 n_stp );

	UINT32 n_decoded;
	UINT32 n_offset;
	UINT16 p_n_output[ 24 * 16 ];

	INT32 p_n_quantize_y[ DCTSIZE2 ];
	INT32 p_n_quantize_uv[ DCTSIZE2 ];
	INT32 p_n_cos[ DCTSIZE2 ];
	INT32 p_n_cos_precalc[ DCTSIZE2 * DCTSIZE2 ];

	UINT32 n_0_command;
	UINT32 n_0_address;
	UINT32 n_0_size;
	UINT32 n_1_command;
	UINT32 n_1_status;

	UINT16 p_n_clamp8[ 256 * 3 ];
	UINT16 p_n_r5[ 256 * 3 ];
	UINT16 p_n_g5[ 256 * 3 ];
	UINT16 p_n_b5[ 256 * 3 ];

	INT32 m_p_n_unpacked[ DCTSIZE2 * 6 * 2 ];
};

#endif
