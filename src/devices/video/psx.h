// license:BSD-3-Clause
// copyright-holders:smf
/*
 * PlayStation GPU emulator
 *
 * Copyright 2003-2014 smf
 *
 */

#ifndef MAME_VIDEO_PSX_H
#define MAME_VIDEO_PSX_H

#pragma once

#define PSXGPU_DEBUG_VIEWER ( 0 )

DECLARE_DEVICE_TYPE(CXD8514Q,  cxd8514q_device)
DECLARE_DEVICE_TYPE(CXD8538Q,  cxd8538q_device)
DECLARE_DEVICE_TYPE(CXD8561Q,  cxd8561q_device)
DECLARE_DEVICE_TYPE(CXD8561BQ, cxd8561bq_device)
DECLARE_DEVICE_TYPE(CXD8561CQ, cxd8561cq_device)
DECLARE_DEVICE_TYPE(CXD8654Q,  cxd8654q_device)

class psxcpu_device;

class psxgpu_device : public device_t, public device_video_interface, public device_palette_interface
{
public:
	// configuration helpers
	auto vblank_callback() { return m_vblank_handler.bind(); }
	int vram_size() { return vramSize; }

	void write(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t read(offs_t offset, uint32_t mem_mask = ~0);
	void dma_read( uint32_t *ram, uint32_t n_address, int32_t n_size );
	void dma_write( uint32_t *ram, uint32_t n_address, int32_t n_size );
	void lightgun_set( int, int );

	static constexpr feature_type imperfect_features() { return feature::GRAPHICS; }

protected:
	static constexpr unsigned MAX_LEVEL = 32;
	static constexpr unsigned MID_LEVEL = (MAX_LEVEL / 2) << 8;
	static constexpr unsigned MAX_SHADE = 0x100;
	static constexpr unsigned MID_SHADE = 0x80;

	// construction/destruction
	psxgpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t vram_size, psxcpu_device *cpu_tag);
	psxgpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_post_load() override;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_config_complete() override;

	// device_palette_interface overrides
	virtual uint32_t palette_entries() const noexcept override { return 32*32*32*2; }

	int vramSize;

private:
	static constexpr unsigned DEBUG_COORDS = 10;

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
		uint32_t n_entry[ 16 ];

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

		struct
		{
			PAIR n_bgr;
			struct FLATTEXTUREDVERTEX vertex;
		} TexturedDot;
	};

	void updatevisiblearea();
	void decode_tpage( uint32_t tpage );
	void FlatPolygon( int n_points );
	void FlatTexturedPolygon( int n_points );
	void GouraudPolygon( int n_points );
	void GouraudTexturedPolygon( int n_points );
	void MonochromeLine();
	void GouraudLine();
	void FrameBufferRectangleDraw();
	void FlatRectangle();
	void FlatRectangle8x8();
	void FlatRectangle16x16();
	void FlatTexturedRectangle();
	void Sprite8x8();
	void Sprite16x16();
	void Dot();
	void TexturedDot();
	void MoveImage();
	void psx_gpu_init( int n_gputype );
	void gpu_reset();
	void gpu_read( uint32_t *p_ram, int32_t n_size );
	void gpu_write( uint32_t *p_ram, int32_t n_size );

	int32_t m_n_tx;
	int32_t m_n_ty;
	int32_t n_abr;
	int32_t n_tp;
	int32_t n_ix;
	int32_t n_iy;
	int32_t n_ti;

	std::unique_ptr<uint16_t[]> p_vram;
	uint32_t n_vramx;
	uint32_t n_vramy;
	uint32_t n_twy;
	uint32_t n_twx;
	uint32_t n_twh;
	uint32_t n_tww;
	uint32_t n_drawarea_x1;
	uint32_t n_drawarea_y1;
	uint32_t n_drawarea_x2;
	uint32_t n_drawarea_y2;
	uint32_t n_horiz_disstart;
	uint32_t n_horiz_disend;
	uint32_t n_vert_disstart;
	uint32_t n_vert_disend;
	uint32_t b_reverseflag;
	int32_t n_drawoffset_x;
	int32_t n_drawoffset_y;
	uint32_t m_n_displaystartx;
	uint32_t n_displaystarty;
	int m_n_gputype;
	uint32_t n_gpustatus;
	uint32_t n_gpuinfo;
	uint32_t n_gpu_buffer_offset;
	uint32_t n_lightgun_x;
	uint32_t n_lightgun_y;
	uint32_t n_screenwidth;
	uint32_t n_screenheight;
	bool m_draw_stp;
	bool m_check_stp;

	PACKET m_packet;

	uint16_t *p_p_vram[ 1024 ];

	uint16_t p_n_redshade[ MAX_LEVEL * MAX_SHADE ];
	uint16_t p_n_greenshade[ MAX_LEVEL * MAX_SHADE ];
	uint16_t p_n_blueshade[ MAX_LEVEL * MAX_SHADE ];
	uint16_t p_n_redlevel[ 0x10000 ];
	uint16_t p_n_greenlevel[ 0x10000 ];
	uint16_t p_n_bluelevel[ 0x10000 ];

	uint16_t p_n_f025[ MAX_LEVEL * MAX_SHADE ];
	uint16_t p_n_f05[ MAX_LEVEL * MAX_SHADE ];
	uint16_t p_n_f1[ MAX_LEVEL * MAX_SHADE ];
	uint16_t p_n_redb05[ 0x10000 ];
	uint16_t p_n_greenb05[ 0x10000 ];
	uint16_t p_n_blueb05[ 0x10000 ];
	uint16_t p_n_redb1[ 0x10000 ];
	uint16_t p_n_greenb1[ 0x10000 ];
	uint16_t p_n_blueb1[ 0x10000 ];
	uint16_t p_n_redaddtrans[ MAX_LEVEL * MAX_LEVEL ];
	uint16_t p_n_greenaddtrans[ MAX_LEVEL * MAX_LEVEL ];
	uint16_t p_n_blueaddtrans[ MAX_LEVEL * MAX_LEVEL ];
	uint16_t p_n_redsubtrans[ MAX_LEVEL * MAX_LEVEL ];
	uint16_t p_n_greensubtrans[ MAX_LEVEL * MAX_LEVEL ];
	uint16_t p_n_bluesubtrans[ MAX_LEVEL * MAX_LEVEL ];

	uint32_t p_n_g0r0[ 0x10000 ];
	uint32_t p_n_b0[ 0x10000 ];
	uint32_t p_n_r1[ 0x10000 ];
	uint32_t p_n_b1g1[ 0x10000 ];

	devcb_write_line m_vblank_handler;

	void vblank(screen_device &screen, bool vblank_state);
	uint32_t update_screen(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

#if defined(PSXGPU_DEBUG_VIEWER) && PSXGPU_DEBUG_VIEWER
	void DebugMeshInit();
	void DebugMesh( int n_coordx, int n_coordy );
	void DebugMeshEnd();
	void DebugCheckKeys();
	int DebugMeshDisplay( bitmap_rgb32 &bitmap, const rectangle &cliprect );
	int DebugTextureDisplay( bitmap_rgb32 &bitmap );

	psx_gpu_debug m_debug;
#endif
};

class cxd8514q_device : public psxgpu_device
{
public:
	// construction/destruction
	cxd8514q_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint32_t vram_size, psxcpu_device *cpu);
	cxd8514q_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class cxd8538q_device : public psxgpu_device
{
public:
	// construction/destruction
	cxd8538q_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint32_t vram_size, psxcpu_device *cpu);
	cxd8538q_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class cxd8561q_device : public psxgpu_device
{
public:
	// construction/destruction
	cxd8561q_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint32_t vram_size, psxcpu_device *cpu);
	cxd8561q_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class cxd8561bq_device : public psxgpu_device
{
public:
	// construction/destruction
	cxd8561bq_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint32_t vram_size, psxcpu_device *cpu);
	cxd8561bq_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class cxd8561cq_device : public psxgpu_device
{
public:
	// construction/destruction
	cxd8561cq_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint32_t vram_size, psxcpu_device *cpu);
	cxd8561cq_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class cxd8654q_device : public psxgpu_device
{
public:
	// construction/destruction
	cxd8654q_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint32_t vram_size, psxcpu_device *cpu);
	cxd8654q_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

#endif // MAME_VIDEO_PSX_H
