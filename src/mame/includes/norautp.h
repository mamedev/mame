// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Roberto Fresca
#include "machine/i8255.h"
#include "sound/discrete.h"
#include "screen.h"


/* Discrete Sound Input Nodes */
#define NORAUTP_SND_EN                  NODE_01
#define NORAUTP_FREQ_DATA               NODE_02


class norautp_state : public driver_device
{
public:
	norautp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu"),
		m_ppi8255(*this, "ppi8255_%u", 0),
		m_discrete(*this, "discrete"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")  { }

	std::unique_ptr<uint16_t[]> m_np_vram;
	DECLARE_WRITE_LINE_MEMBER(ppi2_obf_w);
	TIMER_CALLBACK_MEMBER(ppi2_ack);
	DECLARE_READ8_MEMBER(test2_r);
	DECLARE_WRITE8_MEMBER(mainlamps_w);
	DECLARE_WRITE8_MEMBER(soundlamps_w);
	DECLARE_WRITE8_MEMBER(counterlamps_w);
	DECLARE_DRIVER_INIT(ssa);
	DECLARE_DRIVER_INIT(enc);
	DECLARE_DRIVER_INIT(deb);
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(norautp);
	uint32_t screen_update_norautp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device_array<i8255_device, 3> m_ppi8255;
	required_device<discrete_device> m_discrete;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
};

/*----------- defined in audio/norautp.c -----------*/
DISCRETE_SOUND_EXTERN( norautp );
DISCRETE_SOUND_EXTERN( dphl );
DISCRETE_SOUND_EXTERN( kimble );
