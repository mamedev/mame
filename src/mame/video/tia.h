// license:BSD-3-Clause
// copyright-holders:Wilbert Pol,Stefan Jokisch
#ifndef _VIDEO_TIA_H_
#define _VIDEO_TIA_H_

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
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_TIA_READ_INPUT_PORT_CB(_devcb) \
	devcb = &tia_video_device::set_read_input_port_callback(*device, DEVCB_##_devcb);

#define MCFG_TIA_DATABUS_CONTENTS_CB(_devcb) \
	devcb = &tia_video_device::set_databus_contents_callback(*device, DEVCB_##_devcb);

#define MCFG_TIA_VSYNC_CB(_devcb) \
	devcb = &tia_video_device::set_vsync_callback(*device, DEVCB_##_devcb);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> tia_video_device

class tia_video_device :    public device_t,
							public device_video_interface
{
public:
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	template<class _Object> static devcb_base &set_read_input_port_callback(device_t &device, _Object object) { return downcast<tia_video_device &>(device).m_read_input_port_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_databus_contents_callback(device_t &device, _Object object) { return downcast<tia_video_device &>(device).m_databus_contents_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_vsync_callback(device_t &device, _Object object) { return downcast<tia_video_device &>(device).m_vsync_cb.set_callback(object); }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

protected:
	// construction/destruction
	tia_video_device(const machine_config &mconfig, device_type type, const char *name, const char *shortname, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	void draw_sprite_helper(UINT8* p, UINT8 *col, struct player_gfx *gfx, UINT8 GRP, UINT8 COLUP, UINT8 REFP);
	void draw_missile_helper(UINT8* p, UINT8* col, int horz, int skipdelay, int latch, int start, UINT8 RESMP, UINT8 ENAM, UINT8 NUSIZ, UINT8 COLUM);
	void draw_playfield_helper(UINT8* p, UINT8* col, int horz, UINT8 COLU, UINT8 REFPF);
	void draw_ball_helper(UINT8* p, UINT8* col, int horz, UINT8 ENAB);
	void drawS0(UINT8* p, UINT8* col);
	void drawS1(UINT8* p, UINT8* col);
	void drawM0(UINT8* p, UINT8* col);
	void drawM1(UINT8* p, UINT8* col);
	void drawBL(UINT8* p, UINT8* col);
	void drawPF(UINT8* p, UINT8 *col);
	int collision_check(UINT8* p1, UINT8* p2, int x1, int x2);
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

	struct player_gfx p0gfx;
	struct player_gfx p1gfx;

	UINT64 frame_cycles;
	UINT64 paddle_start;

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

	UINT8 VSYNC;
	UINT8 VBLANK;
	UINT8 COLUP0;
	UINT8 COLUP1;
	UINT8 COLUBK;
	UINT8 COLUPF;
	UINT8 CTRLPF;
	UINT8 GRP0;
	UINT8 GRP1;
	UINT8 REFP0;
	UINT8 REFP1;
	UINT8 HMP0;
	UINT8 HMP1;
	UINT8 HMM0;
	UINT8 HMM1;
	UINT8 HMBL;
	UINT8 VDELP0;
	UINT8 VDELP1;
	UINT8 VDELBL;
	UINT8 NUSIZ0;
	UINT8 NUSIZ1;
	UINT8 ENAM0;
	UINT8 ENAM1;
	UINT8 ENABL;
	UINT8 CXM0P;
	UINT8 CXM1P;
	UINT8 CXP0FB;
	UINT8 CXP1FB;
	UINT8 CXM0FB;
	UINT8 CXM1FB;
	UINT8 CXBLPF;
	UINT8 CXPPMM;
	UINT8 RESMP0;
	UINT8 RESMP1;
	UINT8 PF0;
	UINT8 PF1;
	UINT8 PF2;
	UINT8 INPT4;
	UINT8 INPT5;

	UINT8 prevGRP0;
	UINT8 prevGRP1;
	UINT8 prevENABL;

	int HMOVE_started;
	int HMOVE_started_previous;
	UINT8 HMP0_latch;
	UINT8 HMP1_latch;
	UINT8 HMM0_latch;
	UINT8 HMM1_latch;
	UINT8 HMBL_latch;
	UINT8 REFLECT;      /* Should playfield be reflected or not */
	UINT8 NUSIZx_changed;

	std::unique_ptr<bitmap_ind16> helper[3];

	UINT16 screen_height;

	void register_save_state();
};

class tia_pal_video_device : public tia_video_device
{
public:
	tia_pal_video_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_PALETTE_INIT(tia_pal);

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
};

class tia_ntsc_video_device : public tia_video_device
{
public:
	tia_ntsc_video_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_PALETTE_INIT(tia_ntsc);

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
};


// device type definition
extern const device_type TIA_PAL_VIDEO;
extern const device_type TIA_NTSC_VIDEO;

#endif /* _VIDEO_TIA_H_ */
