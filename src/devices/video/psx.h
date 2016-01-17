// license:BSD-3-Clause
// copyright-holders:smf
/*
 * PlayStation GPU emulator
 *
 * Copyright 2003-2014 smf
 *
 */

#pragma once

#ifndef __PSXGPU_H__
#define __PSXGPU_H__

#include "emu.h"

#define MCFG_PSX_GPU_VBLANK_HANDLER(_devcb) \
	devcb = &psxgpu_device::set_vblank_handler(*device, DEVCB_##_devcb);

#define MCFG_PSXGPU_ADD( cputag, tag, type, _vramSize, clock ) \
	MCFG_DEVICE_MODIFY( cputag ) \
	MCFG_PSX_GPU_READ_HANDLER(DEVREAD32(tag, psxgpu_device, read)) \
	MCFG_PSX_GPU_WRITE_HANDLER(DEVWRITE32(tag, psxgpu_device, write)) \
	MCFG_DEVICE_ADD( tag, type, clock ) \
	((psxgpu_device *) device)->vramSize = _vramSize; \
	MCFG_PSX_GPU_VBLANK_HANDLER(DEVWRITELINE(cputag ":irq", psxirq_device, intin0)) \
	MCFG_PSX_DMA_CHANNEL_READ( cputag, 2, psx_dma_write_delegate( FUNC( psxgpu_device::dma_read ), (psxgpu_device *) device ) ) \
	MCFG_PSX_DMA_CHANNEL_WRITE( cputag, 2, psx_dma_read_delegate( FUNC( psxgpu_device::dma_write ), (psxgpu_device *) device ) )

#define MCFG_PSXGPU_REPLACE( cputag, tag, type, _vramSize, clock ) \
	MCFG_DEVICE_MODIFY( cputag ) \
	MCFG_PSX_GPU_READ_HANDLER(DEVREAD32(tag, psxgpu_device, read)) \
	MCFG_PSX_GPU_WRITE_HANDLER(DEVWRITE32(tag, psxgpu_device, write)) \
	MCFG_DEVICE_REPLACE( tag, type, clock ) \
	((psxgpu_device *) device)->vramSize = _vramSize; \
	MCFG_PSX_GPU_VBLANK_HANDLER(DEVWRITELINE(cputag ":irq", psxirq_device, intin0)) \
	MCFG_PSX_DMA_CHANNEL_READ( cputag, 2, psx_dma_write_delegate( FUNC( psxgpu_device::dma_read ), (psxgpu_device *) device ) ) \
	MCFG_PSX_DMA_CHANNEL_WRITE( cputag, 2, psx_dma_read_delegate( FUNC( psxgpu_device::dma_write ), (psxgpu_device *) device ) )

#define MCFG_PSXGPU_VBLANK_CALLBACK( _delegate ) \
	((screen_device *) config.device_find( device, "screen" ))->register_vblank_callback( _delegate );

extern const device_type CXD8514Q;
extern const device_type CXD8538Q;
extern const device_type CXD8561Q;
extern const device_type CXD8561BQ;
extern const device_type CXD8561CQ;
extern const device_type CXD8654Q;

#define STOP_ON_ERROR ( 0 )

#define MAX_LEVEL ( 32 )
#define MID_LEVEL ( ( MAX_LEVEL / 2 ) << 8 )
#define MAX_SHADE ( 0x100 )
#define MID_SHADE ( 0x80 )

#define DEBUG_COORDS ( 10 )

struct psx_gpu_debug
{
	std::unique_ptr<bitmap_ind16> mesh;
	int b_clear;
	int b_mesh;
	int n_skip;
	int b_texture;
	int n_interleave;
	int n_coord;
	int n_coordx[ DEBUG_COORDS ];
	int n_coordy[ DEBUG_COORDS ];
};

struct FLATVERTEX
{
	PAIR n_coord;
};

struct GOURAUDVERTEX
{
	PAIR n_bgr;
	PAIR n_coord;
};

struct FLATTEXTUREDVERTEX
{
	PAIR n_coord;
	PAIR n_texture;
};

struct GOURAUDTEXTUREDVERTEX
{
	PAIR n_bgr;
	PAIR n_coord;
	PAIR n_texture;
};

union PACKET
{
	UINT32 n_entry[ 16 ];

	struct
	{
		PAIR n_cmd;
		struct FLATVERTEX vertex[ 2 ];
		PAIR n_size;
	} MoveImage;

	struct
	{
		PAIR n_bgr;
		PAIR n_coord;
		PAIR n_size;
	} FlatRectangle;

	struct
	{
		PAIR n_bgr;
		PAIR n_coord;
	} FlatRectangle8x8;

	struct
	{
		PAIR n_bgr;
		PAIR n_coord;
	} FlatRectangle16x16;

	struct
	{
		PAIR n_bgr;
		PAIR n_coord;
		PAIR n_texture;
	} Sprite8x8;

	struct
	{
		PAIR n_bgr;
		PAIR n_coord;
		PAIR n_texture;
	} Sprite16x16;

	struct
	{
		PAIR n_bgr;
		PAIR n_coord;
		PAIR n_texture;
		PAIR n_size;
	} FlatTexturedRectangle;

	struct
	{
		PAIR n_bgr;
		struct FLATVERTEX vertex[ 4 ];
	} FlatPolygon;

	struct
	{
		struct GOURAUDVERTEX vertex[ 4 ];
	} GouraudPolygon;

	struct
	{
		PAIR n_bgr;
		struct FLATVERTEX vertex[ 2 ];
	} MonochromeLine;

	struct
	{
		struct GOURAUDVERTEX vertex[ 2 ];
	} GouraudLine;

	struct
	{
		PAIR n_bgr;
		struct FLATTEXTUREDVERTEX vertex[ 4 ];
	} FlatTexturedPolygon;

	struct
	{
		struct GOURAUDTEXTUREDVERTEX vertex[ 4 ];
	} GouraudTexturedPolygon;

	struct
	{
		PAIR n_bgr;
		struct FLATVERTEX vertex;
	} Dot;
};

class psxgpu_device : public device_t
{
public:
	// construction/destruction
	psxgpu_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);
	virtual machine_config_constructor device_mconfig_additions() const override;

	// static configuration helpers
	template<class _Object> static devcb_base &set_vblank_handler(device_t &device, _Object object) { return downcast<psxgpu_device &>(device).m_vblank_handler.set_callback(object); }

	UINT32 update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE32_MEMBER( write );
	DECLARE_READ32_MEMBER( read );
	void dma_read( UINT32 *ram, UINT32 n_address, INT32 n_size );
	void dma_write( UINT32 *ram, UINT32 n_address, INT32 n_size );
	void lightgun_set( int, int );
	int vramSize;
	void vblank(screen_device &screen, bool vblank_state);
	DECLARE_PALETTE_INIT( psx );

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	void updatevisiblearea();
	void decode_tpage( UINT32 tpage );
	void FlatPolygon( int n_points );
	void FlatTexturedPolygon( int n_points );
	void GouraudPolygon( int n_points );
	void GouraudTexturedPolygon( int n_points );
	void MonochromeLine( void );
	void GouraudLine( void );
	void FrameBufferRectangleDraw( void );
	void FlatRectangle( void );
	void FlatRectangle8x8( void );
	void FlatRectangle16x16( void );
	void FlatTexturedRectangle( void );
	void Sprite8x8( void );
	void Sprite16x16( void );
	void Dot( void );
	void MoveImage( void );
	void psx_gpu_init( int n_gputype );
	void gpu_reset();
	void gpu_read( UINT32 *p_ram, INT32 n_size );
	void gpu_write( UINT32 *p_ram, INT32 n_size );

	INT32 m_n_tx;
	INT32 m_n_ty;
	INT32 n_abr;
	INT32 n_tp;
	INT32 n_ix;
	INT32 n_iy;
	INT32 n_ti;

	std::unique_ptr<UINT16[]> p_vram;
	UINT32 n_vramx;
	UINT32 n_vramy;
	UINT32 n_twy;
	UINT32 n_twx;
	UINT32 n_twh;
	UINT32 n_tww;
	UINT32 n_drawarea_x1;
	UINT32 n_drawarea_y1;
	UINT32 n_drawarea_x2;
	UINT32 n_drawarea_y2;
	UINT32 n_horiz_disstart;
	UINT32 n_horiz_disend;
	UINT32 n_vert_disstart;
	UINT32 n_vert_disend;
	UINT32 b_reverseflag;
	INT32 n_drawoffset_x;
	INT32 n_drawoffset_y;
	UINT32 m_n_displaystartx;
	UINT32 n_displaystarty;
	int m_n_gputype;
	UINT32 n_gpustatus;
	UINT32 n_gpuinfo;
	UINT32 n_gpu_buffer_offset;
	UINT32 n_lightgun_x;
	UINT32 n_lightgun_y;
	UINT32 n_screenwidth;
	UINT32 n_screenheight;

	PACKET m_packet;

	UINT16 *p_p_vram[ 1024 ];

	UINT16 p_n_redshade[ MAX_LEVEL * MAX_SHADE ];
	UINT16 p_n_greenshade[ MAX_LEVEL * MAX_SHADE ];
	UINT16 p_n_blueshade[ MAX_LEVEL * MAX_SHADE ];
	UINT16 p_n_redlevel[ 0x10000 ];
	UINT16 p_n_greenlevel[ 0x10000 ];
	UINT16 p_n_bluelevel[ 0x10000 ];

	UINT16 p_n_f025[ MAX_LEVEL * MAX_SHADE ];
	UINT16 p_n_f05[ MAX_LEVEL * MAX_SHADE ];
	UINT16 p_n_f1[ MAX_LEVEL * MAX_SHADE ];
	UINT16 p_n_redb05[ 0x10000 ];
	UINT16 p_n_greenb05[ 0x10000 ];
	UINT16 p_n_blueb05[ 0x10000 ];
	UINT16 p_n_redb1[ 0x10000 ];
	UINT16 p_n_greenb1[ 0x10000 ];
	UINT16 p_n_blueb1[ 0x10000 ];
	UINT16 p_n_redaddtrans[ MAX_LEVEL * MAX_LEVEL ];
	UINT16 p_n_greenaddtrans[ MAX_LEVEL * MAX_LEVEL ];
	UINT16 p_n_blueaddtrans[ MAX_LEVEL * MAX_LEVEL ];
	UINT16 p_n_redsubtrans[ MAX_LEVEL * MAX_LEVEL ];
	UINT16 p_n_greensubtrans[ MAX_LEVEL * MAX_LEVEL ];
	UINT16 p_n_bluesubtrans[ MAX_LEVEL * MAX_LEVEL ];

	UINT16 p_n_g0r0[ 0x10000 ];
	UINT16 p_n_b0[ 0x10000 ];
	UINT16 p_n_r1[ 0x10000 ];
	UINT16 p_n_b1g1[ 0x10000 ];

	devcb_write_line m_vblank_handler;

#if defined(DEBUG_VIEWER) && DEBUG_VIEWER
	required_device<screen_device> m_screen;
	void DebugMeshInit( void );
	void DebugMesh( int n_coordx, int n_coordy );
	void DebugMeshEnd( void );
	void DebugCheckKeys( void );
	int DebugMeshDisplay( bitmap_ind16 &bitmap, const rectangle &cliprect );
	int DebugTextureDisplay( bitmap_ind16 &bitmap );

	psx_gpu_debug m_debug;
#endif
};

class cxd8514q_device : public psxgpu_device
{
public:
	// construction/destruction
	cxd8514q_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};

class cxd8538q_device : public psxgpu_device
{
public:
	// construction/destruction
	cxd8538q_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};

class cxd8561q_device : public psxgpu_device
{
public:
	// construction/destruction
	cxd8561q_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};

class cxd8561bq_device : public psxgpu_device
{
public:
	// construction/destruction
	cxd8561bq_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};

class cxd8561cq_device : public psxgpu_device
{
public:
	// construction/destruction
	cxd8561cq_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};

class cxd8654q_device : public psxgpu_device
{
public:
	// construction/destruction
	cxd8654q_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};

#endif
