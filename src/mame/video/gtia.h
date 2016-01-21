// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller
/***************************************************************************

    Atari 400/800

    GTIA  graphics television interface adapter

    Juergen Buchmueller, June 1998

***************************************************************************/

#ifndef __GTIA_H__
#define __GTIA_H__

#include "emu.h"


/* reading registers */
struct gtia_readregs
{
	UINT8   m0pf;       /* d000 missile 0 playfield collisions */
	UINT8   m1pf;       /* d001 missile 1 playfield collisions */
	UINT8   m2pf;       /* d002 missile 2 playfield collisions */
	UINT8   m3pf;       /* d003 missile 3 playfield collisions */
	UINT8   p0pf;       /* d004 player 0 playfield collisions */
	UINT8   p1pf;       /* d005 player 1 playfield collisions */
	UINT8   p2pf;       /* d006 player 2 playfield collisions */
	UINT8   p3pf;       /* d007 player 3 playfield collisions */
	UINT8   m0pl;       /* d008 missile 0 player collisions */
	UINT8   m1pl;       /* d009 missile 1 player collisions */
	UINT8   m2pl;       /* d00a missile 2 player collisions */
	UINT8   m3pl;       /* d00b missile 3 player collisions */
	UINT8   p0pl;       /* d00c player 0 player collisions */
	UINT8   p1pl;       /* d00d player 1 player collisions */
	UINT8   p2pl;       /* d00e player 2 player collisions */
	UINT8   p3pl;       /* d00f player 3 player collisions */
	UINT8   but[4];     /* d010-d013 button stick 0-3 */
	UINT8   pal;        /* d014 PAL/NTSC config (D3,2,1 0=PAL, 1=NTSC */
	UINT8   gtia15;     /* d015 nothing */
	UINT8   gtia16;     /* d016 nothing */
	UINT8   gtia17;     /* d017 nothing */
	UINT8   gtia18;     /* d018 nothing */
	UINT8   gtia19;     /* d019 nothing */
	UINT8   gtia1a;     /* d01a nothing */
	UINT8   gtia1b;     /* d01b nothing */
	UINT8   gtia1c;     /* d01c nothing */
	UINT8   gtia1d;     /* d01d nothing */
	UINT8   gtia1e;     /* d01e nothing */
	UINT8   cons;       /* d01f console keys */
};

/* writing registers */
struct gtia_writeregs
{
	UINT8   hposp0;     /* d000 player 0 horz position */
	UINT8   hposp1;     /* d001 player 1 horz position */
	UINT8   hposp2;     /* d002 player 2 horz position */
	UINT8   hposp3;     /* d003 player 3 horz position */
	UINT8   hposm0;     /* d004 missile 0 horz position */
	UINT8   hposm1;     /* d005 missile 1 horz position */
	UINT8   hposm2;     /* d006 missile 2 horz position */
	UINT8   hposm3;     /* d007 missile 3 horz position */
	UINT8   sizep0;     /* d008 size player 0 */
	UINT8   sizep1;     /* d009 size player 1 */
	UINT8   sizep2;     /* d00a size player 2 */
	UINT8   sizep3;     /* d00b size player 3 */
	UINT8   sizem;      /* d00c size missiles */
	UINT8   grafp0[2];  /* d00d graphics data for player 0 */
	UINT8   grafp1[2];  /* d00e graphics data for player 1 */
	UINT8   grafp2[2];  /* d00f graphics data for player 2 */
	UINT8   grafp3[2];  /* d010 graphics data for player 3 */
	UINT8   grafm[2];   /* d011 graphics data for missiles */
	UINT8   colpm0;     /* d012 color for player/missile 0 */
	UINT8   colpm1;     /* d013 color for player/missile 1 */
	UINT8   colpm2;     /* d014 color for player/missile 2 */
	UINT8   colpm3;     /* d015 color for player/missile 3 */
	UINT8   colpf0;     /* d016 playfield color 0 */
	UINT8   colpf1;     /* d017 playfield color 1 */
	UINT8   colpf2;     /* d018 playfield color 2 */
	UINT8   colpf3;     /* d019 playfield color 3 */
	UINT8   colbk;      /* d01a background playfield */
	UINT8   prior;      /* d01b priority select */
	UINT8   vdelay;     /* d01c delay until vertical retrace */
	UINT8   gractl;     /* d01d graphics control */
	UINT8   hitclr;     /* d01e clear collisions */
	UINT8   cons;       /* d01f write console (speaker) */
};

/* helpers */
struct gtia_helpervars
{
	UINT8   grafp0;     /* optimized graphics data player 0 */
	UINT8   grafp1;     /* optimized graphics data player 1 */
	UINT8   grafp2;     /* optimized graphics data player 2 */
	UINT8   grafp3;     /* optimized graphics data player 3 */
	UINT8   grafm0;     /* optimized graphics data missile 0 */
	UINT8   grafm1;     /* optimized graphics data missile 1 */
	UINT8   grafm2;     /* optimized graphics data missile 2 */
	UINT8   grafm3;     /* optimized graphics data missile 3 */
	UINT32  hitclr_frames;/* frames gone since last hitclr */
	UINT8   sizem;      /* optimized size missiles */
	UINT8   usedp;      /* mask for used player colors */
	UINT8   usedm0;     /* mask for used missile 0 color */
	UINT8   usedm1;     /* mask for used missile 1 color */
	UINT8   usedm2;     /* mask for used missile 2 color */
	UINT8   usedm3;     /* mask for used missile 3 color */
	UINT8   vdelay_m0;  /* vertical delay for missile 0 */
	UINT8   vdelay_m1;  /* vertical delay for missile 1 */
	UINT8   vdelay_m2;  /* vertical delay for missile 2 */
	UINT8   vdelay_m3;  /* vertical delay for missile 3 */
	UINT8   vdelay_p0;  /* vertical delay for player 0 */
	UINT8   vdelay_p1;  /* vertical delay for player 1 */
	UINT8   vdelay_p2;  /* vertical delay for player 2 */
	UINT8   vdelay_p3;  /* vertical delay for player 3 */
};

#define MCFG_GTIA_READ_CB(_devcb) \
	devcb = &gtia_device::set_read_callback(*device, DEVCB_##_devcb);

#define MCFG_GTIA_WRITE_CB(_devcb) \
	devcb = &gtia_device::set_write_callback(*device, DEVCB_##_devcb);


// ======================> gtia_device

class gtia_device :  public device_t
{
public:
	// construction/destruction
	gtia_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_read_callback(device_t &device, _Object object) { return downcast<gtia_device &>(device).m_read_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_write_callback(device_t &device, _Object object) { return downcast<gtia_device &>(device).m_write_cb.set_callback(object); }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	UINT16 *get_color_lookup() { return m_color_lookup; }
	void set_color_lookup(int i, UINT16 data) { m_color_lookup[i] = data; }

	UINT8 get_w_colbk() { return m_w.colbk; }
	UINT8 get_w_colpf1() { return m_w.colpf1; }
	UINT8 get_w_colpf2() { return m_w.colpf2; }
	UINT8 get_w_prior() { return m_w.prior; }
	void count_hitclr_frames() { m_h.hitclr_frames++; }
	void button_interrupt(int button_count, UINT8 button_port);

	void render(UINT8 *src, UINT8 *dst, UINT8 *pmbits, UINT8 *prio);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	void gtia_postload();

	int is_ntsc();
	void recalc_p0();
	void recalc_p1();
	void recalc_p2();
	void recalc_p3();
	void recalc_m0();
	void recalc_m1();
	void recalc_m2();
	void recalc_m3();

	inline void player_render(UINT8 gfx, int size, UINT8 color, UINT8 *dst);
	inline void missile_render(UINT8 gfx, int size, UINT8 color, UINT8 *dst);

private:
	gtia_readregs   m_r;          /* read registers */
	gtia_writeregs  m_w;          /* write registers */
	gtia_helpervars m_h;          /* helper variables */

	UINT8 m_lumpf1;
	UINT8 m_huepm0, m_huepm1, m_huepm2, m_huepm3, m_huepm4;
	UINT8 m_huepf2, m_huebk;

	UINT16 m_color_lookup[256];  /* color lookup table */   // probably better fit to ANTIC, but it remains here for the moment...

	devcb_read8 m_read_cb;
	devcb_write8 m_write_cb;
};


// device type definition
extern const device_type ATARI_GTIA;

#endif /* __GTIA_H__ */
