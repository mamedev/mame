// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "sound/okim6295.h"

class shangha3_state : public driver_device
{
public:
	shangha3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_ram(*this, "ram") { }

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	required_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT16> m_ram;

	// driver init configuration
	int m_do_shadows;
	UINT8 m_drawmode_table[16];

	int m_prot_count;
	UINT16 m_gfxlist_addr;
	bitmap_ind16 m_rawbitmap;

	// common
	DECLARE_WRITE16_MEMBER(flipscreen_w);
	DECLARE_WRITE16_MEMBER(gfxlist_addr_w);
	DECLARE_WRITE16_MEMBER(blitter_go_w);
	DECLARE_WRITE16_MEMBER(irq_ack_w);

	// shangha3 specific
	DECLARE_READ16_MEMBER(shangha3_prot_r);
	DECLARE_WRITE16_MEMBER(shangha3_prot_w);
	DECLARE_WRITE16_MEMBER(shangha3_coinctrl_w);

	// heberpop specific
	DECLARE_WRITE16_MEMBER(heberpop_coinctrl_w);
	DECLARE_WRITE16_MEMBER(heberpop_sound_command_w); // used by blocken too

	// blocken specific
	DECLARE_WRITE16_MEMBER(blocken_coinctrl_w);

	DECLARE_DRIVER_INIT(shangha3);
	DECLARE_DRIVER_INIT(heberpop);
	virtual void video_start() override;

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
