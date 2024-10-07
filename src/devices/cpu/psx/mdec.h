// license:BSD-3-Clause
// copyright-holders:smf
/*
 * PlayStation DMA emulator
 *
 * Copyright 2003-2011 smf
 *
 */

#ifndef MAME_CPU_PSX_MDEC_H
#define MAME_CPU_PSX_MDEC_H

#pragma once


DECLARE_DEVICE_TYPE(PSX_MDEC, psxmdec_device)

class psxmdec_device : public device_t
{
public:
	psxmdec_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void write(offs_t offset, uint32_t data);
	uint32_t read(offs_t offset);

	void dma_write( uint32_t *ram, uint32_t n_address, int32_t n_size );
	void dma_read( uint32_t *ram, uint32_t n_address, int32_t n_size );

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override;

private:
	static constexpr unsigned DCTSIZE = 8;
	static constexpr unsigned DCTSIZE2 = DCTSIZE * DCTSIZE;

	static constexpr unsigned MDEC_COS_PRECALC_BITS = 21;

	static const uint32_t m_p_n_mdec_zigzag[ DCTSIZE2 ];

	void mdec_cos_precalc();
	void mdec_idct( int32_t *p_n_src, int32_t *p_n_dst );
	uint32_t mdec_unpack( uint32_t *ram, uint32_t n_address );
	uint16_t mdec_clamp_r5( int32_t n_r ) const;
	uint16_t mdec_clamp_g5( int32_t n_g ) const;
	uint16_t mdec_clamp_b5( int32_t n_b ) const;
	uint16_t mdec_clamp8( int32_t n_r ) const;
	void mdec_yuv2_to_rgb15( void );
	void mdec_yuv2_to_rgb24( void );
	void mdec_makergb15( uint32_t n_address, int32_t n_r, int32_t n_g, int32_t n_b, int32_t *p_n_y, uint16_t n_stp );
	void mdec_makergb24( uint32_t n_address, int32_t n_r, int32_t n_g, int32_t n_b, int32_t *p_n_y, uint32_t n_stp );

	uint32_t n_decoded;
	uint32_t n_offset;
	uint16_t p_n_output[ 24 * 16 ];

	int32_t p_n_quantize_y[ DCTSIZE2 ];
	int32_t p_n_quantize_uv[ DCTSIZE2 ];
	int32_t p_n_cos[ DCTSIZE2 ];
	int32_t p_n_cos_precalc[ DCTSIZE2 * DCTSIZE2 ];

	uint32_t n_0_command;
	uint32_t n_0_address;
	uint32_t n_0_size;
	uint32_t n_1_command;
	uint32_t n_1_status;

	uint16_t p_n_clamp8[ 256 * 3 ];
	uint16_t p_n_r5[ 256 * 3 ];
	uint16_t p_n_g5[ 256 * 3 ];
	uint16_t p_n_b5[ 256 * 3 ];

	int32_t m_p_n_unpacked[ DCTSIZE2 * 6 * 2 ];
};

#endif // MAME_CPU_PSX_MDEC_H
