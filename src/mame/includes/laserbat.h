// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*************************************************************************

    Laser Battle / Lazarian - Cat and Mouse

*************************************************************************/

#include "audio/zaccaria.h"

#include "machine/6821pia.h"
#include "machine/pla.h"
#include "machine/s2636.h"

#include "sound/ay8910.h"
#include "sound/sn76477.h"
#include "sound/tms3615.h"


class laserbat_state_base : public driver_device
{
public:

	laserbat_state_base(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_row0(*this, "ROW0")
		, m_row1(*this, "ROW1")
		, m_row2(*this, "ROW2")
		, m_sw1(*this, "SW1")
		, m_sw2(*this, "SW2")
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_gfxmix(*this, "gfxmix")
		, m_pvi1(*this, "pvi1")
		, m_pvi2(*this, "pvi2")
		, m_pvi3(*this, "pvi3")
		, m_gfxdecode(*this, "gfxdecode")
		, m_scanline_timer(nullptr)
		, m_gfx1(nullptr)
		, m_gfx2(nullptr)
		, m_input_mux(0)
		, m_mpx_p_1_2(false)
		, m_mpx_bkeff(false)
		, m_nave(false)
		, m_clr_lum(0)
		, m_shp(0)
		, m_wcoh(0)
		, m_wcov(0)
		, m_abeff1(false)
		, m_abeff2(false)
		, m_mpx_eff2_sh(false)
		, m_coleff(0)
		, m_neg1(false)
		, m_neg2(false)
		, m_rhsc(0)
		, m_whsc(0)
		, m_csound1(0)
		, m_csound2(0)
	{
	}

	// control ports
	DECLARE_WRITE8_MEMBER(ct_io_w);
	DECLARE_READ8_MEMBER(rrowx_r);

	DECLARE_DRIVER_INIT(laserbat);
	INTERRUPT_GEN_MEMBER(laserbat_interrupt);

	// video memory and control ports
	DECLARE_WRITE8_MEMBER(videoram_w);
	DECLARE_WRITE8_MEMBER(wcoh_w);
	DECLARE_WRITE8_MEMBER(wcov_w);
	DECLARE_WRITE8_MEMBER(cnt_eff_w);
	DECLARE_WRITE8_MEMBER(cnt_nav_w);

	// sound control ports
	virtual DECLARE_READ8_MEMBER(rhsc_r);
	virtual DECLARE_WRITE8_MEMBER(whsc_w);
	virtual DECLARE_WRITE8_MEMBER(csound1_w);
	virtual DECLARE_WRITE8_MEMBER(csound2_w);

	// running the video
	virtual void video_start() override;
	UINT32 screen_update_laserbat(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	enum { TIMER_SCANLINE };

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// video functions
	TIMER_CALLBACK_MEMBER(video_line);

	// input lines
	required_ioport m_row0;
	required_ioport m_row1;
	required_ioport m_row2;
	required_ioport m_sw1;
	required_ioport m_sw2;

	// main CPU device
	required_device<cpu_device> m_maincpu;

	// video devices
	required_device<screen_device>      m_screen;
	required_device<palette_device>     m_palette;
	required_device<pla_device>         m_gfxmix;
	required_device<s2636_device>       m_pvi1;
	required_device<s2636_device>       m_pvi2;
	required_device<s2636_device>       m_pvi3;
	required_device<gfxdecode_device>   m_gfxdecode;

	// stuff for rendering video
	emu_timer       *m_scanline_timer;
	bitmap_ind16    m_bitmap;
	UINT8 const     *m_gfx1;
	UINT8 const     *m_gfx2;

	// control lines
	unsigned        m_input_mux;
	bool            m_mpx_p_1_2;

	// RAM used by TTL video hardware, writable by CPU
	UINT8           m_bg_ram[0x400];    // background tilemap
	UINT8           m_eff_ram[0x400];   // per-scanline effects (A8 not wired meaning only half is usable)
	bool            m_mpx_bkeff;        // select between writing background and effects memory

	// signals affecting the TTL-generated 32x32 sprite
	bool            m_nave;             // 1-bit enable
	unsigned        m_clr_lum;          // 3-bit colour/luminance
	unsigned        m_shp;              // 3-bit shape
	unsigned        m_wcoh;             // 8-bit offset horizontal
	unsigned        m_wcov;             // 8-bit offset vertical

	// video effects signals
	bool            m_abeff1;           // 1-bit effect enable
	bool            m_abeff2;           // 1-bit effect enable
	bool            m_mpx_eff2_sh;      // 1-bit effect selection
	unsigned        m_coleff;           // 2-bit colour effect
	bool            m_neg1;             // 1-bit area selection
	bool            m_neg2;             // 1-bit area selection

	// sound board I/O signals
	unsigned        m_rhsc;             // 8-bit input from J7
	unsigned        m_whsc;             // 8-bit output to J7
	unsigned        m_csound1;          // bits 1-8 on J3
	unsigned        m_csound2;          // bits 9-16 on J3
};


class laserbat_state : public laserbat_state_base
{
public:
	laserbat_state(const machine_config &mconfig, device_type type, const char *tag)
		: laserbat_state_base(mconfig, type, tag)
		, m_csg(*this, "csg")
		, m_synth_low(*this, "synth_low")
		, m_synth_high(*this, "synth_high")
		, m_keys(0)
	{
	}

	// video initialisation
	DECLARE_PALETTE_INIT(laserbat);

	// sound control ports
	virtual DECLARE_WRITE8_MEMBER(csound2_w) override;

protected:

	// initialisation/startup
	virtual void machine_start() override;

	// sound board devices
	required_device<sn76477_device> m_csg;
	required_device<tms3615_device> m_synth_low;
	required_device<tms3615_device> m_synth_high;

	// register state
	unsigned    m_keys;     // low octave keys 1-13 and high octave keys 2-12 (24 bits)
};


class catnmous_state : public laserbat_state_base
{
public:
	catnmous_state(const machine_config &mconfig, device_type type, const char *tag)
		: laserbat_state_base(mconfig, type, tag)
		, m_audiopcb(*this, "audiopcb")
	{
	}

	// video initialisation
	DECLARE_PALETTE_INIT(catnmous);

	// sound control ports
	virtual DECLARE_WRITE8_MEMBER(csound1_w) override;
	virtual DECLARE_WRITE8_MEMBER(csound2_w) override;

protected:
	required_device<zac1b11107_audio_device>    m_audiopcb;
};
