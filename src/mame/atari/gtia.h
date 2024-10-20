// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller
/***************************************************************************

    Atari 400/800

    GTIA  graphics television interface adapter

    Juergen Buchmueller, June 1998

***************************************************************************/

#ifndef MAME_ATARI_GTIA_H
#define MAME_ATARI_GTIA_H

#pragma once


// ======================> gtia_device

enum gtia_region : unsigned
{
	GTIA_NTSC,
	GTIA_PAL
};

class gtia_device :  public device_t
{
public:
	// construction/destruction
	gtia_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_region(gtia_region region) { m_region = region; }
	auto read_callback() { return m_read_cb.bind(); }
	auto write_callback() { return m_write_cb.bind(); }
	auto trigger_callback() { return m_trigger_cb.bind(); }

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	uint16_t *get_color_lookup() { return m_color_lookup; }
	void set_color_lookup(int i, uint16_t data) { m_color_lookup[i] = data; }

	uint8_t get_w_colbk() { return m_w.colbk; }
	uint8_t get_w_colpf1() { return m_w.colpf1; }
	uint8_t get_w_colpf2() { return m_w.colpf2; }
	uint8_t get_w_prior() { return m_w.prior; }
	void count_hitclr_frames() { m_h.hitclr_frames++; }
	void button_interrupt(int button_count);

	void render(uint8_t *src, uint8_t *dst, uint8_t *pmbits, uint8_t *prio);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

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

	inline void player_render(uint8_t gfx, u8 size_index, uint8_t color, uint8_t *dst);
	inline void missile_render(uint8_t gfx, u8 size_index, uint8_t color, uint8_t *dst);

private:
	/* reading registers */
	struct gtia_readregs
	{
		uint8_t   m0pf;       /* d000 missile 0 playfield collisions */
		uint8_t   m1pf;       /* d001 missile 1 playfield collisions */
		uint8_t   m2pf;       /* d002 missile 2 playfield collisions */
		uint8_t   m3pf;       /* d003 missile 3 playfield collisions */
		uint8_t   p0pf;       /* d004 player 0 playfield collisions */
		uint8_t   p1pf;       /* d005 player 1 playfield collisions */
		uint8_t   p2pf;       /* d006 player 2 playfield collisions */
		uint8_t   p3pf;       /* d007 player 3 playfield collisions */
		uint8_t   m0pl;       /* d008 missile 0 player collisions */
		uint8_t   m1pl;       /* d009 missile 1 player collisions */
		uint8_t   m2pl;       /* d00a missile 2 player collisions */
		uint8_t   m3pl;       /* d00b missile 3 player collisions */
		uint8_t   p0pl;       /* d00c player 0 player collisions */
		uint8_t   p1pl;       /* d00d player 1 player collisions */
		uint8_t   p2pl;       /* d00e player 2 player collisions */
		uint8_t   p3pl;       /* d00f player 3 player collisions */
		uint8_t   but[4];     /* d010-d013 button stick 0-3 */
		uint8_t   pal;        /* d014 PAL/NTSC config (D3,2,1 0=PAL, 1=NTSC */
		uint8_t   gtia15;     /* d015 nothing */
		uint8_t   gtia16;     /* d016 nothing */
		uint8_t   gtia17;     /* d017 nothing */
		uint8_t   gtia18;     /* d018 nothing */
		uint8_t   gtia19;     /* d019 nothing */
		uint8_t   gtia1a;     /* d01a nothing */
		uint8_t   gtia1b;     /* d01b nothing */
		uint8_t   gtia1c;     /* d01c nothing */
		uint8_t   gtia1d;     /* d01d nothing */
		uint8_t   gtia1e;     /* d01e nothing */
		uint8_t   consol;     /* d01f console keys */
	};

	/* writing registers */
	struct gtia_writeregs
	{
		uint8_t   hposp0;     /* d000 player 0 horz position */
		uint8_t   hposp1;     /* d001 player 1 horz position */
		uint8_t   hposp2;     /* d002 player 2 horz position */
		uint8_t   hposp3;     /* d003 player 3 horz position */
		uint8_t   hposm0;     /* d004 missile 0 horz position */
		uint8_t   hposm1;     /* d005 missile 1 horz position */
		uint8_t   hposm2;     /* d006 missile 2 horz position */
		uint8_t   hposm3;     /* d007 missile 3 horz position */
		uint8_t   sizep0;     /* d008 size player 0 */
		uint8_t   sizep1;     /* d009 size player 1 */
		uint8_t   sizep2;     /* d00a size player 2 */
		uint8_t   sizep3;     /* d00b size player 3 */
		uint8_t   sizem;      /* d00c size missiles */
		uint8_t   grafp0[2];  /* d00d graphics data for player 0 */
		uint8_t   grafp1[2];  /* d00e graphics data for player 1 */
		uint8_t   grafp2[2];  /* d00f graphics data for player 2 */
		uint8_t   grafp3[2];  /* d010 graphics data for player 3 */
		uint8_t   grafm[2];   /* d011 graphics data for missiles */
		uint8_t   colpm0;     /* d012 color for player/missile 0 */
		uint8_t   colpm1;     /* d013 color for player/missile 1 */
		uint8_t   colpm2;     /* d014 color for player/missile 2 */
		uint8_t   colpm3;     /* d015 color for player/missile 3 */
		uint8_t   colpf0;     /* d016 playfield color 0 */
		uint8_t   colpf1;     /* d017 playfield color 1 */
		uint8_t   colpf2;     /* d018 playfield color 2 */
		uint8_t   colpf3;     /* d019 playfield color 3 */
		uint8_t   colbk;      /* d01a background playfield */
		uint8_t   prior;      /* d01b priority select */
		uint8_t   vdelay;     /* d01c delay until vertical retrace */
		uint8_t   gractl;     /* d01d graphics control */
		uint8_t   hitclr;     /* d01e clear collisions */
		uint8_t   consol;     /* d01f write console (speaker) */
	};

	/* helpers */
	struct gtia_helpervars
	{
		uint8_t   grafp0;     /* optimized graphics data player 0 */
		uint8_t   grafp1;     /* optimized graphics data player 1 */
		uint8_t   grafp2;     /* optimized graphics data player 2 */
		uint8_t   grafp3;     /* optimized graphics data player 3 */
		uint8_t   grafm0;     /* optimized graphics data missile 0 */
		uint8_t   grafm1;     /* optimized graphics data missile 1 */
		uint8_t   grafm2;     /* optimized graphics data missile 2 */
		uint8_t   grafm3;     /* optimized graphics data missile 3 */
		uint32_t  hitclr_frames;/* frames gone since last hitclr */
		uint8_t   sizem;      /* optimized size missiles */
		uint8_t   usedp;      /* mask for used player colors */
		uint8_t   usedm0;     /* mask for used missile 0 color */
		uint8_t   usedm1;     /* mask for used missile 1 color */
		uint8_t   usedm2;     /* mask for used missile 2 color */
		uint8_t   usedm3;     /* mask for used missile 3 color */
		uint8_t   vdelay_m0;  /* vertical delay for missile 0 */
		uint8_t   vdelay_m1;  /* vertical delay for missile 1 */
		uint8_t   vdelay_m2;  /* vertical delay for missile 2 */
		uint8_t   vdelay_m3;  /* vertical delay for missile 3 */
		uint8_t   vdelay_p0;  /* vertical delay for player 0 */
		uint8_t   vdelay_p1;  /* vertical delay for player 1 */
		uint8_t   vdelay_p2;  /* vertical delay for player 2 */
		uint8_t   vdelay_p3;  /* vertical delay for player 3 */
	};

	gtia_readregs   m_r;
	gtia_writeregs  m_w;
	gtia_helpervars m_h;
	gtia_region     m_region;

	uint8_t m_lumpf1;
	uint8_t m_huepm0, m_huepm1, m_huepm2, m_huepm3, m_huepm4;
	uint8_t m_huepf2, m_huebk;

	uint16_t m_color_lookup[256]; // probably better fit to ANTIC, but it remains here for the moment...

	devcb_read8 m_read_cb;
	devcb_write8 m_write_cb;
	devcb_read8 m_trigger_cb;
};


// device type definition
DECLARE_DEVICE_TYPE(ATARI_GTIA, gtia_device)

#endif // MAME_ATARI_GTIA_H
