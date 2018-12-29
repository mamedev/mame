// license:BSD-3-Clause
// copyright-holders:Wilbert Pol,Stefan Jokisch
#ifndef MAME_VIDEO_VIDEO_TIA_H
#define MAME_VIDEO_VIDEO_TIA_H

#pragma once

#include "sound/tiaintf.h"
#include "emupal.h"

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************


#define TIA_PALETTE_LENGTH              128 + 128 * 128
#define TIA_INPUT_PORT_ALWAYS_ON        0
#define TIA_INPUT_PORT_ALWAYS_OFF       0xff
#define TIA_MAX_SCREEN_HEIGHT           342

#define HMOVE_INACTIVE      -200
#define PLAYER_GFX_SLOTS    4
// Per player graphic
// - pixel number to start drawing from (0-7, from GRPx) / number of pixels drawn from GRPx
// - display position to start drawing
// - size to use
struct player_gfx {
	int start_pixel[PLAYER_GFX_SLOTS];
	int start_drawing[PLAYER_GFX_SLOTS];
	int size[PLAYER_GFX_SLOTS];
	int skipclip[PLAYER_GFX_SLOTS];
};


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> tia_video_device

class tia_video_device :    public device_t,
							public device_video_interface
{
public:
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	auto read_input_port_callback() { return m_read_input_port_cb.bind(); }
	auto databus_contents_callback() { return m_databus_contents_cb.bind(); }
	auto vsync_callback() { return m_vsync_cb.bind(); }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

protected:
	// construction/destruction
	tia_video_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_tia_tag(T &&tag) { m_tia.set_tag(std::forward<T>(tag)); }

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	void draw_sprite_helper(uint8_t* p, uint8_t *col, struct player_gfx *gfx, uint8_t GRP, uint8_t COLUP, uint8_t REFP);
	void draw_missile_helper(uint8_t* p, uint8_t* col, int horz, int skipdelay, int latch, int start, uint8_t RESMP, uint8_t ENAM, uint8_t NUSIZ, uint8_t COLUM);
	void draw_playfield_helper(uint8_t* p, uint8_t* col, int horz, uint8_t COLU, uint8_t REFPF);
	void draw_ball_helper(uint8_t* p, uint8_t* col, int horz, uint8_t ENAB);
	void drawS0(uint8_t* p, uint8_t* col);
	void drawS1(uint8_t* p, uint8_t* col);
	void drawM0(uint8_t* p, uint8_t* col);
	void drawM1(uint8_t* p, uint8_t* col);
	void drawBL(uint8_t* p, uint8_t* col);
	void drawPF(uint8_t* p, uint8_t *col);
	int collision_check(uint8_t* p1, uint8_t* p2, int x1, int x2);
	int current_x();
	int current_y();
	void setup_pXgfx(void);
	void update_bitmap(int next_x, int next_y);
	DECLARE_WRITE8_MEMBER( WSYNC_w );
	DECLARE_WRITE8_MEMBER( VSYNC_w );
	DECLARE_WRITE8_MEMBER( VBLANK_w );
	DECLARE_WRITE8_MEMBER( CTRLPF_w );
	DECLARE_WRITE8_MEMBER( HMP0_w );
	DECLARE_WRITE8_MEMBER( HMP1_w );
	DECLARE_WRITE8_MEMBER( HMM0_w );
	DECLARE_WRITE8_MEMBER( HMM1_w );
	DECLARE_WRITE8_MEMBER( HMBL_w );
	DECLARE_WRITE8_MEMBER( HMOVE_w );
	DECLARE_WRITE8_MEMBER( RSYNC_w );
	DECLARE_WRITE8_MEMBER( NUSIZ0_w );
	DECLARE_WRITE8_MEMBER( NUSIZ1_w );
	DECLARE_WRITE8_MEMBER( HMCLR_w );
	DECLARE_WRITE8_MEMBER( CXCLR_w );
	DECLARE_WRITE8_MEMBER( RESP0_w );
	DECLARE_WRITE8_MEMBER( RESP1_w );
	DECLARE_WRITE8_MEMBER( RESM0_w );
	DECLARE_WRITE8_MEMBER( RESM1_w );
	DECLARE_WRITE8_MEMBER( RESBL_w );
	DECLARE_WRITE8_MEMBER( RESMP0_w );
	DECLARE_WRITE8_MEMBER( RESMP1_w );
	DECLARE_WRITE8_MEMBER( GRP0_w );
	DECLARE_WRITE8_MEMBER( GRP1_w );
	DECLARE_READ8_MEMBER( INPT_r );


private:
	devcb_read16    m_read_input_port_cb;
	devcb_read8     m_databus_contents_cb;
	devcb_write16   m_vsync_cb;

	required_device<cpu_device> m_maincpu;
	required_device<tia_device> m_tia;

	struct player_gfx p0gfx;
	struct player_gfx p1gfx;

	uint64_t frame_cycles;
	uint64_t paddle_start;

	int horzP0;
	int horzP1;
	int horzM0;
	int horzM1;
	int horzBL;
	int motclkP0;
	int motclkP1;
	int motclkM0;
	int motclkM1;
	int motclkBL;
	int startP0;
	int startP1;
	int startM0;
	int startM1;
	int skipclipP0;
	int skipclipP1;
	int skipM0delay;
	int skipM1delay;

	int current_bitmap;

	int prev_x;
	int prev_y;

	uint8_t VSYNC;
	uint8_t VBLANK;
	uint8_t COLUP0;
	uint8_t COLUP1;
	uint8_t COLUBK;
	uint8_t COLUPF;
	uint8_t CTRLPF;
	uint8_t GRP0;
	uint8_t GRP1;
	uint8_t REFP0;
	uint8_t REFP1;
	uint8_t HMP0;
	uint8_t HMP1;
	uint8_t HMM0;
	uint8_t HMM1;
	uint8_t HMBL;
	uint8_t VDELP0;
	uint8_t VDELP1;
	uint8_t VDELBL;
	uint8_t NUSIZ0;
	uint8_t NUSIZ1;
	uint8_t ENAM0;
	uint8_t ENAM1;
	uint8_t ENABL;
	uint8_t CXM0P;
	uint8_t CXM1P;
	uint8_t CXP0FB;
	uint8_t CXP1FB;
	uint8_t CXM0FB;
	uint8_t CXM1FB;
	uint8_t CXBLPF;
	uint8_t CXPPMM;
	uint8_t RESMP0;
	uint8_t RESMP1;
	uint8_t PF0;
	uint8_t PF1;
	uint8_t PF2;
	uint8_t INPT4;
	uint8_t INPT5;

	uint8_t prevGRP0;
	uint8_t prevGRP1;
	uint8_t prevENABL;

	int HMOVE_started;
	int HMOVE_started_previous;
	uint8_t HMP0_latch;
	uint8_t HMP1_latch;
	uint8_t HMM0_latch;
	uint8_t HMM1_latch;
	uint8_t HMBL_latch;
	uint8_t REFLECT;      /* Should playfield be reflected or not */
	uint8_t NUSIZx_changed;

	std::unique_ptr<bitmap_ind16> helper[3];

	uint16_t screen_height;

	void register_save_state();
};

class tia_pal_video_device : public tia_video_device
{
public:
	template <typename T> tia_pal_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&tia_tag)
		: tia_pal_video_device(mconfig, tag, owner, clock)
	{
		set_tia_tag(std::forward<T>(tia_tag));
	}

	tia_pal_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;

private:
	void tia_pal_palette(palette_device &palette) const;
};

class tia_ntsc_video_device : public tia_video_device
{
public:
	template <typename T> tia_ntsc_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&tia_tag)
		: tia_ntsc_video_device(mconfig, tag, owner, clock)
	{
		set_tia_tag(std::forward<T>(tia_tag));
	}

	tia_ntsc_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;

private:
	void tia_ntsc_palette(palette_device &palette) const;
};


// device type definition
DECLARE_DEVICE_TYPE(TIA_PAL_VIDEO, tia_pal_video_device)
DECLARE_DEVICE_TYPE(TIA_NTSC_VIDEO, tia_ntsc_video_device)

#endif // MAME_VIDEO_TIA_H
